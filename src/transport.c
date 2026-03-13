#include "transport.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

struct tls_ctx_s {
    int sock;
    SSL_CTX *ssl_ctx;
    SSL *ssl;
};

static int tls_initialized = 0;

static void tls_init(void) {
    if (!tls_initialized) {
        tls_initialized = 1;
    }
}

int socket_connect(const char *host, int port) {
    return socket_connect_timeout(host, port, 0);
}

int socket_connect_timeout(const char *host, int port, int timeout_sec) {
    struct addrinfo hints, *res, *rp;
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &res) != 0) return -1;

    int sock = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) continue;

        if (timeout_sec > 0) {
            struct timeval tv;
            tv.tv_sec = timeout_sec;
            tv.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }

    freeaddrinfo(res);
    return sock;
}

tls_ctx_t* tls_connect(const char *host, int port) {
    return tls_connect_timeout(host, port, 0);
}

tls_ctx_t* tls_connect_timeout(const char *host, int port, int timeout_sec) {
    tls_init();

    int sock = socket_connect_timeout(host, port, timeout_sec);
    if (sock < 0) return NULL;

    tls_ctx_t *ctx = malloc(sizeof(tls_ctx_t));
    ctx->sock = sock;
    ctx->ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx->ssl_ctx) {
        close(sock);
        free(ctx);
        return NULL;
    }

    SSL_CTX_set_default_verify_paths(ctx->ssl_ctx);
    SSL_CTX_set_verify(ctx->ssl_ctx, SSL_VERIFY_PEER, NULL);

    ctx->ssl = SSL_new(ctx->ssl_ctx);
    SSL_set_fd(ctx->ssl, sock);
    SSL_set_tlsext_host_name(ctx->ssl, host);

    if (SSL_connect(ctx->ssl) <= 0) {
        SSL_free(ctx->ssl);
        SSL_CTX_free(ctx->ssl_ctx);
        close(sock);
        free(ctx);
        return NULL;
    }

    return ctx;
}

ssize_t socket_read(int sock, char *buf, size_t len) {
    return read(sock, buf, len);
}

ssize_t socket_write(int sock, const char *buf, size_t len) {
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(sock, buf + written, len - written);
        if (n <= 0) return n;
        written += n;
    }
    return written;
}

ssize_t tls_read(tls_ctx_t *ctx, char *buf, size_t len) {
    return SSL_read(ctx->ssl, buf, len);
}

ssize_t tls_write(tls_ctx_t *ctx, const char *buf, size_t len) {
    size_t written = 0;
    while (written < len) {
        int n = SSL_write(ctx->ssl, buf + written, len - written);
        if (n <= 0) return n;
        written += n;
    }
    return written;
}

void socket_close(int sock) {
    close(sock);
}

void tls_close(tls_ctx_t *ctx) {
    SSL_shutdown(ctx->ssl);
    SSL_free(ctx->ssl);
    SSL_CTX_free(ctx->ssl_ctx);
    close(ctx->sock);
    free(ctx);
}
