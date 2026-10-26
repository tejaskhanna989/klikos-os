/* pit.c — Programmable Interval Timer driver for x86_64.
 *
 * Channel 0 in mode 3 (square wave) generates IRQ0 for preemptive
 * scheduling at 100 Hz.
 */

#include <kernel/pit.h>
#include <kernel/pic.h>
#include <kernel/sched.h>
#include <kernel/types.h>

static inline void io_outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 io_inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pit_init(u32 hz) {
    u32 divisor = PIT_FREQUENCY / hz;
    u8 low = (u8)(divisor & 0xFF);
    u8 high = (u8)((divisor >> 8) & 0xFF);

    /* Channel 0, mode 3 (square wave), binary */
    io_outb(PIT_PORT_CMD, 0x36);
    io_outb(PIT_PORT_DATA0, low);
    io_outb(PIT_PORT_DATA0, high);

    /* Unmask IRQ0 in PIC */
    pic_irq_unmask(0);
}

void pit_irq_handler(void) {
    /* Call scheduler switch from timer interrupt */
    sched_switch_from_irq(0);
}
