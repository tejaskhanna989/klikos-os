#include <kernel/types.h>

/* Minimal string functions for kernel GUI (no libc with -nostdlib) */

size_t k_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

char *k_strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n-- > 0 && *src) *d++ = *src++;
    while (n-- > 0) *d++ = '\0';
    return dest;
}

int k_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return (unsigned char)*s1 - (unsigned char)*s2;
}
