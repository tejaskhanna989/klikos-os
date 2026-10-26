/* pic.c — 8259 Programmable Interrupt Controller driver for x86_64.
 *
 * Remaps IRQs to avoid collision with CPU exceptions (0x00-0x1F).
 * IRQ0-7 -> 0x20-0x27, IRQ8-15 -> 0x28-0x2F.
 */

#include <kernel/pic.h>
#include <kernel/types.h>

static inline void io_outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 io_inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static u16 irq_mask = 0xFFFF;

void pic_init(void) {
    /* ICW1: init, edge triggered, expect ICW4 */
    io_outb(PIC1_PORT, ICW1_INIT | ICW1_ICW4);
    io_outb(PIC2_PORT, ICW1_INIT | ICW1_ICW4);

    /* ICW2: vector offsets */
    io_outb(PIC1_DATA, PIC1_OFFSET);
    io_outb(PIC2_DATA, PIC2_OFFSET);

    /* ICW3: cascade configuration */
    io_outb(PIC1_DATA, 0x04);   /* IRQ2 connects to slave PIC */
    io_outb(PIC2_DATA, 0x02);   /* slave identity */

    /* ICW4: 8086 mode */
    io_outb(PIC1_DATA, ICW4_8086);
    io_outb(PIC2_DATA, ICW4_8086);

    /* Mask all IRQs initially */
    io_outb(PIC1_DATA, 0xFF);
    io_outb(PIC2_DATA, 0xFF);
    irq_mask = 0xFFFF;
}

void pic_irq_mask(u32 irq) {
    u16 port;
    u8 value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = io_inb(port) | (1 << irq);
    io_outb(port, value);
    irq_mask |= (1 << (irq < 8 ? irq : irq + 8));
}

void pic_irq_unmask(u32 irq) {
    u16 port;
    u8 value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = io_inb(port) & ~(1 << irq);
    io_outb(port, value);
    irq_mask &= ~(1 << (irq < 8 ? irq : irq + 8));
}

void pic_eoi(u32 irq) {
    if (irq >= 8) {
        io_outb(PIC2_PORT, 0x20);
    }
    io_outb(PIC1_PORT, 0x20);
}
