#include <kernel/types.h>
#include <kernel/pl011.h>

/* PL011 register offsets */
#define UART_DR   (*(volatile u32 *)(PL011_BASE + 0x00))
#define UART_FR   (*(volatile u32 *)(PL011_BASE + 0x18))
#define UART_IBRD (*(volatile u32 *)(PL011_BASE + 0x24))
#define UART_FBRD (*(volatile u32 *)(PL011_BASE + 0x28))
#define UART_LCRH (*(volatile u32 *)(PL011_BASE + 0x2C))
#define UART_CR   (*(volatile u32 *)(PL011_BASE + 0x30))

#define FR_TXFF   (1 << 5)   /* transmit FIFO full */
#define CR_UARTEN (1 << 0)   /* UART enable */
#define CR_TXE    (1 << 8)   /* transmit enable */
#define CR_RXE    (1 << 9)   /* receive enable */

#define LCRH_WLEN_8 (3 << 5) /* 8-bit word length */
#define LCRH_FEN    (1 << 4) /* enable FIFOs */

/* QEMU virt PL011 clock = 24 MHz. Target baud = 115200.
 * bauddiv = 24000000 / (16 * 115200) = 13.0208
 *   IBRD = 13, FBRD = int(0.0208 * 64 + 0.5) = 1 */
void pl011_init(void)
{
    UART_CR = 0;                         /* disable during config */
    UART_IBRD = 13;
    UART_FBRD = 1;
    UART_LCRH = LCRH_WLEN_8 | LCRH_FEN;
    UART_CR = CR_UARTEN | CR_TXE | CR_RXE;
}

void pl011_putc(char c)
{
    while (UART_FR & FR_TXFF) {          /* wait until FIFO has room */
        ;
    }
    UART_DR = (u32)c;
}

void pl011_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            pl011_putc('\r');
        }
        pl011_putc(*s++);
    }
}
