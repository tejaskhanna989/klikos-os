#ifndef KLIKOS_KERNEL_SYSCALL_H
#define KLIKOS_KERNEL_SYSCALL_H

#include <kernel/types.h>

void syscall_init(void);
u32  syscall_handler(u32 n, u32 a0, u32 a1, u32 a2, u32 a3);

/* Syscall numbers */
#define SYS_EXIT        0
#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_OPEN        3
#define SYS_CLOSE       4
#define SYS_YIELD       5
#define SYS_SLEEP       6
#define SYS_PUTCHAR     7
#define SYS_GETPID      8
#define SYS_MALLOC      9
#define SYS_FREE        10

#endif /* KLIKOS_KERNEL_SYSCALL_H */
