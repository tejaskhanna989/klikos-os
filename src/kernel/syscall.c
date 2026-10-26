/* syscall.c — KlikOS syscall dispatcher.
 *
 * Handles svc #0 from userspace. Arguments in x0-x3, number in x8.
 */

#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/printf.h>
#include <kernel/mm.h>
#include <kernel/types.h>

#ifdef __aarch64__
#include <kernel/pl011.h>
#define K_SYS_PUTCH(c) pl011_putc(c)
#else
#include <kernel/vga.h>
#define K_SYS_PUTCH(c) vga_putc(c)
#endif

static u32 sys_write(u32 fd, const char *buf, u32 count);
static u32 sys_read(u32 fd, char *buf, u32 count);
static u32 sys_exit(u32 code);
static u32 sys_yield(void);
static u32 sys_getpid(void);
static u32 sys_kmalloc(u32 size);
static u32 sys_kfree(u32 ptr);

void syscall_init(void)
{
    kprintf("[syscall] Syscall interface initialized\n");
}

u32 syscall_handler(u32 n, u32 a0, u32 a1, u32 a2, u32 a3)
{
    switch (n) {
    case SYS_EXIT:
        return sys_exit(a0);
    case SYS_WRITE:
        return sys_write(a0, (const char *)a1, a2);
    case SYS_READ:
        return sys_read(a0, (char *)a1, a2);
    case SYS_OPEN:
        return -1; /* not implemented */
    case SYS_CLOSE:
        return 0;
    case SYS_YIELD:
        return sys_yield();
    case SYS_SLEEP:
        return 0;
    case SYS_PUTCHAR:
        K_SYS_PUTCH((char)a0);
        return 0;
    case SYS_GETPID:
        return sys_getpid();
    case SYS_MALLOC:
        return sys_kmalloc(a0);
    case SYS_FREE:
        sys_kfree(a0);
        return 0;
    default:
        return -1;
    }
}

static u32 sys_write(u32 fd, const char *buf, u32 count)
{
    (void)fd;
    for (u32 i = 0; i < count; i++)
        K_SYS_PUTCH(buf[i]);
    return count;
}

static u32 sys_read(u32 fd, char *buf, u32 count)
{
    (void)fd; (void)buf; (void)count;
    return 0;
}

static u32 sys_exit(u32 code)
{
    (void)code;
    kprintf("[syscall] Process exited\n");
    /* Mark current task as zombie */
    if (sched_get_current())
        sched_get_current()->state = TASK_ZOMBIE;
    sched_yield();
    return 0;
}

static u32 sys_yield(void)
{
    sched_yield();
    return 0;
}

static u32 sys_getpid(void)
{
    return sched_get_pid();
}

static u32 sys_kmalloc(u32 size)
{
    return (u32)kmalloc(size);
}

static u32 sys_kfree(u32 ptr)
{
    kfree((void *)ptr);
    return 0;
}

void __stack_chk_fail(void)
{
    for (;;) {
#ifdef __aarch64__
        __asm__ volatile ("wfi");
#else
        __asm__ volatile ("hlt");
#endif
    }
}
