#include <stdarg.h>
#include <kernel/printf.h>

#ifdef __aarch64__
#include <kernel/pl011.h>
#define K_PUTCH(c) pl011_putc(c)
#define K_PUTS(s) pl011_puts(s)
#else
#include <kernel/vga.h>
#define K_PUTCH(c) vga_putc(c)
#define K_PUTS(s) vga_puts(s)
#endif

/* Print an unsigned long in a given base. */
static void print_ulong(unsigned long val, int base, int uppercase)
{
    char buf[64];
    int i = 0;

    if (val == 0) {
        K_PUTCH('0');
        return;
    }
    while (val > 0) {
        int d = (int)(val % base);
        buf[i++] = (char)((d < 10) ? ('0' + d)
                                   : (uppercase ? ('A' + d - 10)
                                                : ('a' + d - 10)));
        val /= base;
    }
    while (i > 0) {
        K_PUTCH(buf[--i]);
    }
}

/* Print a signed long in base 10. */
static void print_long(long val)
{
    if (val < 0) {
        K_PUTCH('-');
        val = -val;
    }
    print_ulong((unsigned long)val, 10, 0);
}

void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'c':
                K_PUTCH((char)va_arg(args, int));
                break;
            case 's':
                K_PUTS(va_arg(args, char *));
                break;
            case 'd':
            case 'i':
                print_long(va_arg(args, int));
                break;
            case 'u':
                print_ulong(va_arg(args, unsigned int), 10, 0);
                break;
            case 'x':
                print_ulong(va_arg(args, unsigned int), 16, 0);
                break;
            case 'X':
                print_ulong(va_arg(args, unsigned int), 16, 1);
                break;
            case 'p':
                K_PUTS("0x");
                print_ulong(va_arg(args, uintptr_t), 16, 0);
                break;
            case '%':
                K_PUTCH('%');
                break;
            default:
                K_PUTCH(*fmt);
                break;
            }
        } else {
            K_PUTCH(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
