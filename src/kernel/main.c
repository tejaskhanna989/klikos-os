/* main.c — KlikOS kernel entry point.
 *
 * Works on both aarch64 and x86_64. Architecture-specific drivers
 * are selected via #ifdef.
 */

#include <kernel/types.h>
#include <kernel/printf.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <gui/gui.h>
#include <gui/springboard.h>

#ifdef __aarch64__
#include <kernel/pl011.h>
#include <kernel/gic.h>
#include <kernel/timer.h>
#define CONSOLE_INIT() pl011_init()
#define INTERRUPT_INIT() do { gic_init(); timer_init(); } while (0)
#define CONSOLE_NAME "PL011 UART"
#define CPU_IDLE() __asm__ volatile ("wfi")
#else
#include <kernel/vga.h>
#include <kernel/pic.h>
#include <kernel/pit.h>
#define CONSOLE_INIT() vga_init()
#define INTERRUPT_INIT() do { pic_init(); pit_init(100); idt_init(); } while (0)
#define CONSOLE_NAME "VGA text mode"
#define CPU_IDLE() __asm__ volatile ("hlt")
#endif

/* GUI task — draws the springboard each frame */
static void gui_task(void)
{
    for (;;) {
        ui_draw_frame();
        CPU_IDLE();
    }
}

/* Idle task */
static void idle_task(void)
{
    for (;;) { CPU_IDLE(); }
}

void kernel_main(void)
{
    CONSOLE_INIT();

    kprintf("KlikOS kernel v0.2\n");
    kprintf("Booting from scratch mobile OS...\n");
    kprintf("Console: %s\n", CONSOLE_NAME);

    /* 1. Memory manager */
    mm_init();

    /* 2. Scheduler */
    sched_init();

    /* 3. Interrupt controller + timer (preemptive multitasking) */
    INTERRUPT_INIT();

    /* 4. Syscall interface */
    syscall_init();

    /* 5. GUI */
    gui_subsystem_init();

    /* 6. Create tasks */
    sched_add(gui_task, "gui");
    sched_add(idle_task, "idle");

    /* 7. Unmask interrupts */
#ifdef __aarch64__
    __asm__ volatile ("msr daifclr, #0xf");
#else
    __asm__ volatile ("sti");
#endif

    /* 8. Start first task */
    kprintf("[kernel] Starting multitasking...\n");
    sched_switch();

    /* Should never return */
    for (;;) { CPU_IDLE(); }
}
