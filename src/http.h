#ifndef WHIPL_HTTP_H
#define WHIPL_HTTP_H

#include "buffer.h"

typedef struct {
    int status;
    buffer_t *headers;
    buffer_t *body;
    char *error;
} http_response_t;

typedef struct {
    const char **headers;
    int header_count;
    const char *body;
    size_t body_len;
    int follow_redirects;
    int max_redirects;
    int verbose;
} http_options_t;

http_response_t* http_request(const char *method, const char *url, http_options_t *opts);
void http_response_free(http_response_t *resp);

#endif
