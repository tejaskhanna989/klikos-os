#ifndef KLIKOS_KERNEL_STRING_H
#define KLIKOS_KERNEL_STRING_H

#include <kernel/types.h>

size_t k_strlen(const char *s);
char  *k_strncpy(char *dest, const char *src, size_t n);
int    k_strcmp(const char *s1, const char *s2);

#endif /* KLIKOS_KERNEL_STRING_H */
