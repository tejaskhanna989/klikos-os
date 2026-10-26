/* sched.c — KlikOS preemptive scheduler.
 *
 * Round-robin scheduler with time-slice switching.
 * Context switch saves/restores x19-x30, sp, lr, pc.
 */

#include <kernel/sched.h>
#include <kernel/printf.h>
#include <kernel/types.h>
#include <kernel/string.h>
#include <kernel/mm.h>

static task_t tasks[MAX_TASKS];
static int task_count = 0;
static int current = -1;

/* Assembly context switch */
extern void context_switch(u32 *old_sp_ptr, u32 new_sp);
extern void sched_start(u32 initial_sp);

void sched_init(void)
{
    task_count = 0;
    current = -1;
    kprintf("[sched] Scheduler initialized\n");
}

int sched_add(void (*entry)(void), const char *name)
{
    if (task_count >= MAX_TASKS) return -1;

    /* Allocate stack from kernel heap */
    u32 stack = (u32)kmalloc(TASK_STACK_SIZE);
    if (!stack) return -1;

    task_t *t = &tasks[task_count];
    t->pid = task_count;
    t->state = TASK_READY;
    t->stack_top = stack + TASK_STACK_SIZE;
    t->entry = (u32)entry;
    k_strncpy(t->name, name, sizeof(t->name) - 1);
    t->name[sizeof(t->name) - 1] = '\0';

    /* Initial stack frame: from high to low:
     * sp[0..1]   = x29(fp), x30(lr=entry)   <- popped first, x30 = return addr
     * sp[2..3]   = x27, x28
     * sp[4..5]   = x25, x26
     * sp[6..7]   = x23, x24
     * sp[8..9]   = x21, x22
     * sp[10..11] = x19, x20                  <- popped last
     * t->sp points to sp[0]
     */
    u32 *sp = (u32 *)t->stack_top;
    sp -= 12;  /* 12 u32 values = 96 bytes */
    sp[0]  = (u32)entry;  /* x29 (fp) */
    sp[1]  = (u32)entry;  /* x30 (lr) — return to entry */
    sp[2]  = 0;   /* x27 */
    sp[3]  = 0;   /* x28 */
    sp[4]  = 0;   /* x25 */
    sp[5]  = 0;   /* x26 */
    sp[6]  = 0;   /* x23 */
    sp[7]  = 0;   /* x24 */
    sp[8]  = 0;   /* x21 */
    sp[9]  = 0;   /* x22 */
    sp[10] = 0;   /* x19 */
    sp[11] = 0;   /* x20 */

    t->sp = (u32)sp;

    kprintf("[sched] Added task %d: %s\n", t->pid, name);
    return task_count++;
}

void sched_yield(void)
{
    sched_switch();
}

void sched_switch(void)
{
    if (task_count == 0) return;

    int next = (current + 1) % task_count;
    int attempts = 0;

    while (tasks[next].state != TASK_READY && attempts < task_count) {
        next = (next + 1) % task_count;
        attempts++;
    }

    if (attempts >= task_count) return;

    int prev = current;
    current = next;
    tasks[current].state = TASK_RUNNING;

    if (prev >= 0) {
        tasks[prev].state = TASK_READY;
        context_switch(&tasks[prev].sp, tasks[current].sp);
    } else {
        /* First task — just start it */
        sched_start(tasks[current].sp);
    }
}

int sched_get_pid(void)
{
    return current >= 0 ? tasks[current].pid : -1;
}

task_t *sched_get_current(void)
{
    return current >= 0 ? &tasks[current] : NULL;
}

/* Called from IRQ handler — saves sp, picks next task, returns new sp */
u32 sched_switch_from_irq(u32 current_sp)
{
    if (task_count == 0)
        return current_sp;

    /* Save current sp */
    if (current >= 0) {
        tasks[current].sp = current_sp;
        tasks[current].state = TASK_READY;
    }

    /* Find next runnable task */
    int next = (current + 1) % task_count;
    int attempts = 0;
    while (tasks[next].state != TASK_READY && attempts < task_count) {
        next = (next + 1) % task_count;
        attempts++;
    }

    if (attempts >= task_count) {
        if (current >= 0)
            tasks[current].state = TASK_RUNNING;
        return current_sp;
    }

    current = next;
    tasks[current].state = TASK_RUNNING;
    return tasks[current].sp;
}
