#ifndef WHIPL_TRANSPORT_H
#define WHIPL_TRANSPORT_H

#include <sys/types.h>

typedef struct tls_ctx_s tls_ctx_t;

int socket_connect(const char *host, int port);
int socket_connect_timeout(const char *host, int port, int timeout_sec);
tls_ctx_t* tls_connect(const char *host, int port);
tls_ctx_t* tls_connect_timeout(const char *host, int port, int timeout_sec);
ssize_t socket_read(int sock, char *buf, size_t len);
ssize_t socket_write(int sock, const char *buf, size_t len);
ssize_t tls_read(tls_ctx_t *ctx, char *buf, size_t len);
ssize_t tls_write(tls_ctx_t *ctx, const char *buf, size_t len);
void socket_close(int sock);
void tls_close(tls_ctx_t *ctx);

#endif
