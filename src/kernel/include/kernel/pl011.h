#ifndef KLIKOS_PL011_H
#define KLIKOS_PL011_H

#include <kernel/types.h>

/* PrimeCell PL011 UART base address on the ARM Versatile / QEMU virt board. */
#define PL011_BASE 0x09000000

void pl011_init(void);
void pl011_putc(char c);
void pl011_puts(const char *s);

#endif /* KLIKOS_PL011_H */
