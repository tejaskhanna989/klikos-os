/* bootloader/main.c — KlikOS bootloader (for real hardware).
 *
 * Under QEMU, the kernel is loaded directly by QEMU (-kernel), so this
 * bootloader is a stub placeholder. On real ARM64 hardware it will:
 *   1. Initialize RAM and UART
 *   2. Load the kernel image from storage
 *   3. Jump to the kernel entry point
 */

#include <klikos/types.h>   /* userspace-style types for bootloader */

void putc(char c);

void main(void) {
    putc('K');
    putc('l');
    putc('i');
    putc('k');
    putc('O');
    putc('S');
    putc(' ');
    putc('b');
    putc('o');
    putc('o');
    putc('t');
    putc('l');
    putc('o');
    putc('a');
    putc('d');
    putc('e');
    putc('r');
    putc('\n');

    /* Stub: hang. Real bootloader loads kernel and jumps to it. */
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
