/* gic.c — ARM GICv2 interrupt controller driver.
 *
 * QEMU virt machine uses GICv2. Initializes distributor and CPU interface,
 * enables the arch timer interrupt (IRQ 27 on aarch64 virt).
 */

#include <kernel/gic.h>
#include <kernel/printf.h>
#include <kernel/types.h>

/* GICD registers */
#define GICD_CTLR      (*(volatile u32 *)(GICD_BASE + 0x000))
#define GICD_ISENABLER (*(volatile u32 *)(GICD_BASE + 0x100))
#define GICD_ICENABLER (*(volatile u32 *)(GICD_BASE + 0x180))
#define GICD_ISPENDR   (*(volatile u32 *)(GICD_BASE + 0x200))

/* GICC registers */
#define GICC_CTLR      (*(volatile u32 *)(GICC_BASE + 0x000))
#define GICC_PMR       (*(volatile u32 *)(GICC_BASE + 0x004))
#define GICC_IAR       (*(volatile u32 *)(GICC_BASE + 0x00C))
#define GICC_EOIR      (*(volatile u32 *)(GICC_BASE + 0x010))
#define GICC_PRM       (*(volatile u32 *)(GICC_BASE + 0x004))

void gic_init(void)
{
    /* Disable distributor during config */
    GICD_CTLR = 0;

    /* Set all interrupts to level-sensitive, group 1 (secure) or group 0 */
    /* For simplicity: use group 0, route to CPU interface */

    /* Enable distributor */
    GICD_CTLR = 1;

    /* Configure CPU interface */
    GICC_CTLR = 1;
    GICC_PMR  = 0xFF;  /* accept all interrupt priorities */

    kprintf("[gic] GICv2 initialized\n");
}

void gic_enable_irq(u32 intid)
{
    u32 reg = intid / 32;
    u32 bit = intid % 32;
    volatile u32 *reg_addr = (volatile u32 *)(u64)(GICD_BASE + 0x100 + reg * 4);
    *reg_addr |= (1 << bit);
}

void gic_irq_ack(void)
{
    u32 intid = GICC_IAR & 0x3FF;
    (void)intid;
}

void gic_irq_eoi(u32 intid)
{
    GICC_EOIR = intid;
}
