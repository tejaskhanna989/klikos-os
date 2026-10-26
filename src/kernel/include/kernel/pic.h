#ifndef KLIKOS_KERNEL_PIC_H
#define KLIKOS_KERNEL_PIC_H

#include <kernel/types.h>

#define PIC1_PORT      0x20
#define PIC1_DATA      0x21
#define PIC2_PORT      0xA0
#define PIC2_DATA      0xA1

#define ICW1_INIT      0x10
#define ICW1_ICW4      0x01
#define ICW4_8086      0x01

#define PIC1_OFFSET    0x20   /* IRQ0-7 -> int 0x20-0x27 */
#define PIC2_OFFSET    0x28   /* IRQ8-15 -> int 0x28-0x2F */

void pic_init(void);
void pic_irq_mask(u32 irq);
void pic_irq_unmask(u32 irq);
void pic_eoi(u32 irq);

#endif /* KLIKOS_KERNEL_PIC_H */
