#include "http.h"
#include "transport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static inline void parse_url(const char *url, char *host, char *path, int *port, int *use_tls) {
    const char *p = url;
    *use_tls = 0;

    if (strncmp(p, "https://", 8) == 0) {
        p += 8;
        *use_tls = 1;
    } else if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    }

    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');

    if (colon && (!slash || colon < slash)) {
        size_t host_len = colon - p;
        if (host_len > 255) host_len = 255;
        memcpy(host, p, host_len);
        host[host_len] = '\0';
        *port = atoi(colon + 1);
        p = slash ? slash : p + strlen(p);
    } else {
        size_t host_len = slash ? (slash - p) : strlen(p);
        if (host_len > 255) host_len = 255;
        memcpy(host, p, host_len);
        host[host_len] = '\0';
        *port = *use_tls ? 443 : 80;
        p = slash ? slash : p + strlen(p);
    }

    size_t path_len = strlen(*p ? p : "/");
    if (path_len > 1023) path_len = 1023;
    memcpy(path, *p ? p : "/", path_len);
    path[path_len] = '\0';
}

static char* resolve_url(const char *base_url, const char *location) {
    if (strncmp(location, "http://", 7) == 0 || strncmp(location, "https://", 8) == 0) {
        return strdup(location);
    }

    char base_host[256], base_path[1024];
    int base_port, base_tls;
    parse_url(base_url, base_host, base_path, &base_port, &base_tls);

    char *result = malloc(2048);
    int non_default_port = (base_tls && base_port != 443) || (!base_tls && base_port != 80);

    if (non_default_port) {
        snprintf(result, 2048, "%s://%s:%d%s",
            base_tls ? "https" : "http",
            base_host, base_port,
            location[0] == '/' ? location : "");
    } else {
        snprintf(result, 2048, "%s://%s%s",
            base_tls ? "https" : "http",
            base_host,
            location[0] == '/' ? location : "");
    }

    if (location[0] != '/') {
        strncat(result, location, 2048 - strlen(result) - 1);
    }

    return result;
}

static char* get_header_value(const char *headers, const char *name) {
    char *line = strcasestr(headers, name);
    if (!line) return NULL;

    char *colon = strchr(line, ':');
    if (!colon) return NULL;

    char *value = colon + 1;
    while (*value == ' ') value++;

    char *end = strchr(value, '\r');
    if (!end) end = strchr(value, '\n');
    if (!end) return NULL;

    size_t len = end - value;
    char *result = malloc(len + 1);
    memcpy(result, value, len);
    result[len] = '\0';
    return result;
}

http_response_t* http_request(const char *method, const char *url, http_options_t *opts) {
    char host[256], path[1024];
    int port, use_tls;
    parse_url(url, host, path, &port, &use_tls);

    if (opts && opts->verbose) {
        fprintf(stderr, "* Connecting to %s:%d\n", host, port);
        fprintf(stderr, "* Using %s\n", use_tls ? "HTTPS" : "HTTP");
    }

    void *conn = NULL;
    int sock = -1;

    if (use_tls) {
        conn = tls_connect(host, port);
        if (!conn) {
            if (opts && opts->verbose) fprintf(stderr, "* TLS connection failed\n");
            http_response_t *resp = malloc(sizeof(http_response_t));
            resp->status = 0;
            resp->headers = NULL;
            resp->body = NULL;
            resp->error = strdup("TLS connection failed");
            return resp;
        }
        if (opts && opts->verbose) fprintf(stderr, "* TLS handshake complete\n");
    } else {
        sock = socket_connect(host, port);
        if (sock < 0) {
            if (opts && opts->verbose) fprintf(stderr, "* Connection failed\n");
            http_response_t *resp = malloc(sizeof(http_response_t));
            resp->status = 0;
            resp->headers = NULL;
            resp->body = NULL;
            resp->error = strdup("Connection failed");
            return resp;
        }
        if (opts && opts->verbose) fprintf(stderr, "* Connected\n");
    }

    buffer_t *req = buffer_new(512);
    char req_line[2048];

    snprintf(req_line, sizeof(req_line), "%s %s HTTP/1.1\r\nHost: %s\r\n", method, path, host);
    buffer_append(req, req_line, strlen(req_line));

    if (opts && opts->headers) {
        for (int i = 0; i < opts->header_count; i++) {
            buffer_append(req, opts->headers[i], strlen(opts->headers[i]));
            buffer_append(req, "\r\n", 2);
        }
    }

    if (opts && opts->body) {
        snprintf(req_line, sizeof(req_line), "Content-Length: %zu\r\n", opts->body_len);
        buffer_append(req, req_line, strlen(req_line));
    }

    buffer_append(req, "Connection: close\r\n\r\n", 21);

    if (opts && opts->body) {
        buffer_append(req, opts->body, opts->body_len);
    }

    if (opts && opts->verbose) {
        fprintf(stderr, "> %s %s HTTP/1.1\n", method, path);
        fprintf(stderr, "> Host: %s\n", host);
        if (opts->headers) {
            for (int i = 0; i < opts->header_count; i++) {
                fprintf(stderr, "> %s\n", opts->headers[i]);
            }
        }
        fprintf(stderr, ">\n");
    }

    if (use_tls) {
        tls_write(conn, req->data, req->len);
    } else {
        socket_write(sock, req->data, req->len);
    }
    buffer_free(req);

    buffer_t *resp_buf = buffer_new(4096);
    char chunk[4096];
    ssize_t n;

    if (use_tls) {
        while ((n = tls_read(conn, chunk, sizeof(chunk))) > 0) {
            buffer_append(resp_buf, chunk, n);
        }
        tls_close(conn);
    } else {
        while ((n = socket_read(sock, chunk, sizeof(chunk))) > 0) {
            buffer_append(resp_buf, chunk, n);
        }
        socket_close(sock);
    }

    buffer_null_terminate(resp_buf);

    http_response_t *resp = malloc(sizeof(http_response_t));
    resp->headers = buffer_new(1024);
    resp->body = buffer_new(4096);
    resp->error = NULL;

    char *header_end = strstr(resp_buf->data, "\r\n\r\n");
    if (header_end) {
        size_t header_len = header_end - resp_buf->data;
        buffer_append(resp->headers, resp_buf->data, header_len);

        if (opts && opts->verbose) {
            fprintf(stderr, "< %.*s\n", (int)header_len, resp_buf->data);
        }

        char *status_line = resp_buf->data;
        char *space = strchr(status_line, ' ');
        resp->status = space ? atoi(space + 1) : 0;

        char *body_start = header_end + 4;
        size_t body_len = resp_buf->len - header_len - 4;

        char *chunked_header = strcasestr(resp_buf->data, "transfer-encoding: chunked");
        if (chunked_header && chunked_header < header_end) {
            char *p = body_start;
            char *end = body_start + body_len;
            while (p < end) {
                char *line_end = strstr(p, "\r\n");
                if (!line_end) break;
                int chunk_size = strtol(p, NULL, 16);
                if (chunk_size == 0) break;
                p = line_end + 2;
                if (p + chunk_size <= end) {
                    buffer_append(resp->body, p, chunk_size);
                    p += chunk_size + 2;
                }
            }
        } else {
            buffer_append(resp->body, body_start, body_len);
        }
    } else {
        resp->status = 0;
    }

    buffer_free(resp_buf);

    // Handle redirects
    if (opts && opts->follow_redirects && resp->status >= 300 && resp->status < 400) {
        int max_redir = opts->max_redirects > 0 ? opts->max_redirects : 10;
        if (max_redir > 0) {
            char *location = get_header_value(resp->headers->data, "location:");
            if (location) {
                char *full_url = resolve_url(url, location);
                if (opts->verbose) {
                    fprintf(stderr, "* Following redirect to: %s\n", full_url);
                }
                free(location);
                http_response_free(resp);

                http_options_t new_opts = *opts;
                new_opts.max_redirects = max_redir - 1;
                new_opts.body = NULL;
                new_opts.body_len = 0;

                http_response_t *redirect_resp = http_request("GET", full_url, &new_opts);
                free(full_url);
                return redirect_resp;
            }
        }
    }

    return resp;
}

void http_response_free(http_response_t *resp) {
    if (resp->headers) buffer_free(resp->headers);
    if (resp->body) buffer_free(resp->body);
    if (resp->error) free(resp->error);
    free(resp);
}
