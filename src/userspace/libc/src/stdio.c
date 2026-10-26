#include <klikos/unistd.h>
#include <klikos/stdio.h>

extern ssize_t write(int fd, const void *buf, size_t count);

extern ssize_t write(int fd, const void *buf, size_t count);

static int stdout_fd = STDOUT_FILENO;

int puts(const char *s) {
    int n = 0;
    while (*s) {
        write(STDOUT_FILENO, s, 1);
        s++;
        n++;
    }
    write(STDOUT_FILENO, "\n", 1);
    return n + 1;
}

int isatty(int fd) {
    /* Stub: treat fds 0,1,2 as ttys */
    return (fd >= 0 && fd <= 2);
}
