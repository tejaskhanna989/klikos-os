/* timer.c — ARM arch timer driver for preemptive scheduling.
 *
 * Programs the EL1 physical timer to fire periodically,
 * triggering IRQ 27 on GIC which calls sched_switch_from_irq.
 */

#include <kernel/timer.h>
#include <kernel/gic.h>
#include <kernel/printf.h>
#include <kernel/sched.h>
#include <kernel/types.h>

#define CNTFRQ_EL0  (*(volatile u32 *)0x0)  /* read via mrs */
#define CNTP_CTL_EL0 (*(volatile u32 *)0x0) /* via mrs */
#define CNTP_CVAL_EL0 (*(volatile u32 *)0x0)/* via mrs */

static u32 timer_freq = 100000000; /* 100 MHz default */

static inline u32 read_cntfrq(void)
{
    u32 val;
    __asm__ volatile ("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}

static inline void write_cntp_ctl(u32 val)
{
    __asm__ volatile ("msr cntp_ctl_el0, %0" :: "r"(val));
}

static inline u32 read_cntp_ctl(void)
{
    u32 val;
    __asm__ volatile ("mrs %0, cntp_ctl_el0" : "=r"(val));
    return val;
}

static inline void write_cntp_cval(u64 val)
{
    __asm__ volatile ("msr cntp_cval_el0, %0" :: "r"(val));
}

static inline u64 read_cntp_tval(void)
{
    u64 val;
    __asm__ volatile ("mrs %0, cntp_tval_el0" : "=r"(val));
    return val;
}

static inline void write_cntp_tval(u32 val)
{
    __asm__ volatile ("msr cntp_tval_el0, %0" :: "r"(val));
}

void timer_init(void)
{
    timer_freq = read_cntfrq();
    kprintf("[timer] Frequency: %u Hz\n", timer_freq);

    /* Set timer to fire every 10 ms */
    u32 tick = timer_freq / 100;

    /* Clear timer */
    write_cntp_tval(tick);

    /* Enable timer: bit 0 = enable, bit 1 = imask (0 = not masked) */
    write_cntp_ctl(0x1);

    /* Enable IRQ 27 (arch timer) in GIC */
    gic_enable_irq(27);

    kprintf("[timer] Periodic timer enabled (100 Hz)\n");
}

void timer_irq_handler(void)
{
    /* Acknowledge timer interrupt by reading CNTP_CTL (auto-clears on some) */
    /* For arch physical timer, the interrupt clears when CNTP_TVAL reaches 0
     * and we rewrite CNTP_TVAL */
    write_cntp_tval(timer_freq / 100);

    /* Call scheduler switch */
    sched_switch_from_irq(0); /* sp will be set by IRQ handler from task struct */
}
