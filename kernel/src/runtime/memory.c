#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t length) {
    unsigned char *out = destination;
    const unsigned char *in = source;
    for (size_t index = 0; index < length; ++index) {
        out[index] = in[index];
    }
    return destination;
}

void *memset(void *destination, int value, size_t length) {
    unsigned char *out = destination;
    for (size_t index = 0; index < length; ++index) {
        out[index] = (unsigned char)value;
    }
    return destination;
}

void *memmove(void *destination, const void *source, size_t length) {
    unsigned char *out = destination;
    const unsigned char *in = source;
    if (out < in) {
        for (size_t index = 0; index < length; ++index) {
            out[index] = in[index];
        }
    } else {
        for (size_t index = length; index != 0; --index) {
            out[index - 1] = in[index - 1];
        }
    }
    return destination;
}

int memcmp(const void *left, const void *right, size_t length) {
    const unsigned char *a = left;
    const unsigned char *b = right;
    for (size_t index = 0; index < length; ++index) {
        if (a[index] != b[index]) {
            return a[index] < b[index] ? -1 : 1;
        }
    }
    return 0;
}
