#ifndef WHIPL_BUFFER_H
#define WHIPL_BUFFER_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} buffer_t;

buffer_t* buffer_new(size_t cap);
void buffer_append(buffer_t *buf, const char *data, size_t len);
void buffer_reset(buffer_t *buf);
void buffer_null_terminate(buffer_t *buf);
void buffer_free(buffer_t *buf);

#endif
