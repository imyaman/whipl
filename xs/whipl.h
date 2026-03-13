#ifndef WHIPL_H
#define WHIPL_H

typedef struct {
    int status;
    char *headers;
    char *body;
    size_t body_len;
    char *error;
} WhiplResponse;

typedef struct {
    const char **headers;
    int header_count;
    const char *body;
    size_t body_len;
    int follow_redirects;
    int max_redirects;
    int verbose;
} WhiplOptions;

WhiplResponse* whipl_request(const char *method, const char *url, WhiplOptions *opts);
void whipl_response_free(WhiplResponse *resp);

#endif
