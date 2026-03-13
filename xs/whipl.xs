#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "../src/http.h"
#include "whipl.h"
#include <string.h>

WhiplResponse* whipl_request(const char *method, const char *url, WhiplOptions *opts) {
    http_options_t http_opts = {0};
    if (opts) {
        http_opts.headers = opts->headers;
        http_opts.header_count = opts->header_count;
        http_opts.body = opts->body;
        http_opts.body_len = opts->body_len;
        http_opts.follow_redirects = opts->follow_redirects;
        http_opts.max_redirects = opts->max_redirects;
        http_opts.verbose = opts->verbose;
    }

    http_response_t *http_resp = http_request(method, url, opts ? &http_opts : NULL);
    if (!http_resp) return NULL;

    WhiplResponse *resp = malloc(sizeof(WhiplResponse));
    resp->status = http_resp->status;
    resp->error = http_resp->error ? strdup(http_resp->error) : NULL;

    if (http_resp->headers) {
        resp->headers = malloc(http_resp->headers->len + 1);
        memcpy(resp->headers, http_resp->headers->data, http_resp->headers->len);
        resp->headers[http_resp->headers->len] = '\0';
    } else {
        resp->headers = NULL;
    }

    if (http_resp->body) {
        resp->body = malloc(http_resp->body->len);
        memcpy(resp->body, http_resp->body->data, http_resp->body->len);
        resp->body_len = http_resp->body->len;
    } else {
        resp->body = NULL;
        resp->body_len = 0;
    }

    http_response_free(http_resp);
    return resp;
}

void whipl_response_free(WhiplResponse *resp) {
    if (resp->headers) free(resp->headers);
    if (resp->body) free(resp->body);
    if (resp->error) free(resp->error);
    free(resp);
}

static SV* whipl_response_to_sv(pTHX_ WhiplResponse *resp) {
    HV *hv = newHV();
    hv_store(hv, "status", 6, newSViv(resp->status), 0);
    if (resp->headers) hv_store(hv, "headers", 7, newSVpv(resp->headers, 0), 0);
    if (resp->body) hv_store(hv, "body", 4, newSVpvn(resp->body, resp->body_len), 0);
    if (resp->error) hv_store(hv, "error", 5, newSVpv(resp->error, 0), 0);
    whipl_response_free(resp);
    return newRV_noinc((SV*)hv);
}

MODULE = Whipl    PACKAGE = Whipl

PROTOTYPES: DISABLE

SV*
get(url)
    char *url
  CODE:
    WhiplResponse *resp = whipl_request("GET", url, NULL);
    if (!resp) XSRETURN_UNDEF;
    RETVAL = whipl_response_to_sv(aTHX_ resp);
  OUTPUT:
    RETVAL

SV*
request(method, url, ...)
    char *method
    char *url
  CODE:
    WhiplOptions opts = {0};

    if (items > 2) {
        HV *opt_hash = (HV*)SvRV(ST(2));

        SV **headers_sv = hv_fetch(opt_hash, "headers", 7, 0);
        if (headers_sv && SvROK(*headers_sv) && SvTYPE(SvRV(*headers_sv)) == SVt_PVAV) {
            AV *headers_av = (AV*)SvRV(*headers_sv);
            opts.header_count = av_len(headers_av) + 1;
            opts.headers = malloc(opts.header_count * sizeof(char*));
            for (int i = 0; i < opts.header_count; i++) {
                SV **sv = av_fetch(headers_av, i, 0);
                opts.headers[i] = sv ? SvPV_nolen(*sv) : "";
            }
        }

        SV **body_sv = hv_fetch(opt_hash, "body", 4, 0);
        if (body_sv) {
            STRLEN len;
            opts.body = SvPV(*body_sv, len);
            opts.body_len = len;
        }

        SV **follow_sv = hv_fetch(opt_hash, "follow_redirects", 16, 0);
        if (follow_sv && SvTRUE(*follow_sv)) {
            opts.follow_redirects = 1;
            opts.max_redirects = 10;
        }

        SV **max_redir_sv = hv_fetch(opt_hash, "max_redirects", 13, 0);
        if (max_redir_sv) {
            opts.max_redirects = SvIV(*max_redir_sv);
        }

        SV **verbose_sv = hv_fetch(opt_hash, "verbose", 7, 0);
        if (verbose_sv && SvTRUE(*verbose_sv)) {
            opts.verbose = 1;
        }
    }

    WhiplResponse *resp = whipl_request(method, url, items > 2 ? &opts : NULL);

    if (opts.headers) free((void*)opts.headers);

    if (!resp) XSRETURN_UNDEF;
    RETVAL = whipl_response_to_sv(aTHX_ resp);
  OUTPUT:
    RETVAL
