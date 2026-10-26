#ifndef KLIKOS_PRINTF_H
#define KLIKOS_PRINTF_H

#include <kernel/types.h>

/* Minimal kernel printf: %c %s %d %i %u %x %X %p %%. Output goes to PL011 UART. */
void kprintf(const char *fmt, ...);

#endif /* KLIKOS_PRINTF_H */
