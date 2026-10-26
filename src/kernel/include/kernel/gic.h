#ifndef KLIKOS_KERNEL_GIC_H
#define KLIKOS_KERNEL_GIC_H

#include <kernel/types.h>

#define GICD_BASE 0x08000000  /* GIC Distributor */
#define GICC_BASE 0x08010000  /* GIC CPU Interface */

void gic_init(void);
void gic_enable_irq(u32 intid);
void gic_irq_ack(void);
void gic_irq_eoi(u32 intid);

#endif /* KLIKOS_KERNEL_GIC_H */
