#ifndef KLIKOS_KERNEL_SCHED_H
#define KLIKOS_KERNEL_SCHED_H

#include <kernel/types.h>

#define MAX_TASKS  16
#define TASK_STACK_SIZE 16384

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_t;

typedef struct {
    int   pid;
    task_state_t state;
    u32   sp;                    /* saved stack pointer */
    u32   stack_top;
    u32   entry;                 /* entry point */
    char  name[32];
} task_t;

void sched_init(void);
int sched_add(void (*entry)(void), const char *name);
void sched_yield(void);
void sched_switch(void);         /* called from timer interrupt */
int  sched_get_pid(void);
task_t *sched_get_current(void);

/* Called from IRQ handler — returns new sp */
u32 sched_switch_from_irq(u32 current_sp);

/* Context switch assembly */
void context_switch(u32 *old_sp_ptr, u32 new_sp);
void sched_start(u32 initial_sp);

/* x86_64 IDT initialization */
#ifdef __x86_64__
void idt_init(void);
#endif

#endif /* KLIKOS_KERNEL_SCHED_H */
