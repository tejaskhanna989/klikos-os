#ifndef KLIKOS_LIBC_UNISTD_H
#define KLIKOS_LIBC_UNISTD_H

#include <stddef.h>
#include <sys/types.h>

ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int     close(int fd);
int     open(const char *pathname, int flags);
int     isatty(int fd);

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_CREAT   0x0040
#define O_TRUNC   0x0200
#define O_APPEND  0x0400

#endif /* KLIKOS_LIBC_UNISTD_H */
