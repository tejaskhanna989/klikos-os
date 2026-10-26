#ifndef KLIKOS_KERNEL_TIMER_H
#define KLIKOS_KERNEL_TIMER_H

#include <kernel/types.h>

void timer_init(void);
void timer_irq_handler(void);

#endif /* KLIKOS_KERNEL_TIMER_H */
