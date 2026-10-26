#ifndef KLIKOS_KERNEL_PIT_H
#define KLIKOS_KERNEL_PIT_H

#include <kernel/types.h>

#define PIT_PORT_DATA0   0x40
#define PIT_PORT_CMD     0x43
#define PIT_FREQUENCY    1193182
#define PIT_HZ           100

void pit_init(u32 hz);
void pit_irq_handler(void);

#endif /* KLIKOS_KERNEL_PIT_H */
