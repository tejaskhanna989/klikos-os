#ifndef KLIKOS_LIBC_STDIO_H
#define KLIKOS_LIBC_STDIO_H

#include <stddef.h>

int  putchar(int c);
int  puts(const char *s);
int  printf(const char *fmt, ...);
int  sprintf(char *buf, const char *fmt, ...);

#define EOF (-1)
#define BUFSIZ 256

#endif /* KLIKOS_LIBC_STDIO_H */
