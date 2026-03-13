#include "buffer.h"
#include <stdlib.h>
#include <string.h>

buffer_t* buffer_new(size_t cap) {
    buffer_t *buf = malloc(sizeof(buffer_t));
    buf->data = malloc(cap);
    buf->len = 0;
    buf->cap = cap;
    return buf;
}

void buffer_append(buffer_t *buf, const char *data, size_t len) {
    if (buf->len + len > buf->cap) {
        size_t new_cap = (buf->len + len) * 2;
        if (new_cap < buf->cap * 2) new_cap = buf->cap * 2;
        buf->data = realloc(buf->data, new_cap);
        buf->cap = new_cap;
    }
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
}

void buffer_reset(buffer_t *buf) {
    buf->len = 0;
}

void buffer_null_terminate(buffer_t *buf) {
    if (buf->len >= buf->cap) {
        buf->data = realloc(buf->data, buf->cap + 1);
        buf->cap += 1;
    }
    buf->data[buf->len] = '\0';
}

void buffer_free(buffer_t *buf) {
    free(buf->data);
    free(buf);
}
