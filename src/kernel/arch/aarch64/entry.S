/* entry.S — KlikOS kernel entry, exception vectors, context switch.
 *
 * Sets up:
 *  - Stack for boot CPU
 *  - Vector table (VBAR_EL1) for exceptions
 *  - SVC handler that calls syscall_handler() from C
 *  - IRQ handler for preemptive scheduler
 *  - context_switch() and sched_start() assembly functions
 */

.section .text.boot
.global _start
.align 4

_start:
    /* Read MPIDR_EL1 to identify the current CPU */
    mrs     x0, mpidr_el1
    and     x0, x0, #0xff
    cbnz    x0, 2f                  /* non-boot CPUs park */

    /* Boot CPU: set up stack at 0x40100000 */
    movz    x0, #0x4010, lsl #16
    mov     sp, x0

    /* Set up vector table address */
    adr     x0, vector_table
    msr     vbar_el1, x0

    /* SCTLR_EL1: stack alignment check */
    mrs     x0, sctlr_el1
    orr     x0, x0, #(1 << 2)
    msr     sctlr_el1, x0

    /* Branch to C kernel */
    bl      kernel_main

1:
    wfi
    b       1b

2:
    wfe
    b       2b

/* ========================================================================
 * Exception vector table
 * ======================================================================== */
.align 12
vector_table:
    b       sync_el0_sp0
    .fill   120, 1, 0
    b       irq_el0_sp0
    .fill   120, 1, 0
    b       fiq_el0_sp0
    .fill   120, 1, 0
    b       serror_el0_sp0
    .fill   120, 1, 0
    b       . + 0x1c0
    .fill   0x1c0 - 8, 1, 0
    b       . + 0x140
    .fill   0x140 - 8, 1, 0
    b       . + 0xC0
    .fill   0xC0 - 8, 1, 0

/* ------------------------------------------------------------------
 * Synchronous exception handler (EL0) — handles svc (syscall)
 * ------------------------------------------------------------------ */
sync_el0_sp0:
    stp     x0, x1, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x6, x7, [sp, #-16]!
    stp     x8, x9, [sp, #-16]!
    stp     x10, x11, [sp, #-16]!
    stp     x12, x13, [sp, #-16]!
    stp     x14, x15, [sp, #-16]!
    stp     x16, x17, [sp, #-16]!
    stp     x18, x19, [sp, #-16]!
    stp     x20, x21, [sp, #-16]!
    stp     x22, x23, [sp, #-16]!
    stp     x24, x25, [sp, #-16]!
    stp     x26, x27, [sp, #-16]!
    stp     x28, x29, [sp, #-16]!
    stp     x30, x30, [sp, #-16]!

    mrs     x0, esr_el1
    and     x1, x0, #0x3F
    cmp     x1, #0x15
    b.ne    sync_unknown

    /* SVC: x0=result will be set by syscall_handler.
     * x8 = syscall number, x0-x3 = args */
    bl      syscall_handler

    ldp     x28, x29, [sp], #16
    ldp     x26, x27, [sp], #16
    ldp     x24, x25, [sp], #16
    ldp     x22, x23, [sp], #16
    ldp     x20, x21, [sp], #16
    ldp     x18, x19, [sp], #16
    ldp     x16, x17, [sp], #16
    ldp     x14, x15, [sp], #16
    ldp     x12, x13, [sp], #16
    ldp     x10, x11, [sp], #16
    ldp     x8, x9,  [sp], #16
    ldp     x6, x7,  [sp], #16
    ldp     x4, x5,  [sp], #16
    ldp     x2, x3,  [sp], #16
    ldp     x0, x1,  [sp], #16

    eret

sync_unknown:
    b       sync_unknown

/* ------------------------------------------------------------------
 * IRQ handler (EL0) — timer interrupt for preemptive scheduling
 * ------------------------------------------------------------------ */
irq_el0_sp0:
    stp     x0, x1, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x6, x7, [sp, #-16]!
    stp     x8, x9, [sp, #-16]!
    stp     x10, x11, [sp, #-16]!
    stp     x12, x13, [sp, #-16]!
    stp     x14, x15, [sp, #-16]!
    stp     x16, x17, [sp, #-16]!
    stp     x18, x19, [sp, #-16]!
    stp     x20, x21, [sp, #-16]!
    stp     x22, x23, [sp, #-16]!
    stp     x24, x25, [sp, #-16]!
    stp     x26, x27, [sp, #-16]!
    stp     x28, x29, [sp, #-16]!
    stp     x30, x30, [sp, #-16]!

    /* Save current sp to current task struct */
    mov     x0, sp
    bl      sched_switch_from_irq   /* x0 = current_sp, returns new sp in x0 */
    mov     sp, x0                  /* switch to new task's stack */

    ldp     x28, x29, [sp], #16
    ldp     x26, x27, [sp], #16
    ldp     x24, x25, [sp], #16
    ldp     x22, x23, [sp], #16
    ldp     x20, x21, [sp], #16
    ldp     x18, x19, [sp], #16
    ldp     x16, x17, [sp], #16
    ldp     x14, x15, [sp], #16
    ldp     x12, x13, [sp], #16
    ldp     x10, x11, [sp], #16
    ldp     x8, x9,  [sp], #16
    ldp     x6, x7,  [sp], #16
    ldp     x4, x5,  [sp], #16
    ldp     x2, x3,  [sp], #16
    ldp     x0, x1,  [sp], #16

    eret

fiq_el0_sp0:
    eret

serror_el0_sp0:
    b       serror_el0_sp0

/* ------------------------------------------------------------------
 * context_switch(u32 *old_sp_ptr, u32 new_sp)
 * Save all callee-saved regs, switch stack, restore regs.
 * ------------------------------------------------------------------ */
.section .text
.global context_switch
.align 4
context_switch:
    /* Save current sp to *old_sp_ptr first */
    mov     x2, sp
    str     x2, [x0]

    /* Push callee-saved regs onto current (old) stack */
    stp     x19, x20, [sp, #-16]!
    stp     x21, x22, [sp, #-16]!
    stp     x23, x24, [sp, #-16]!
    stp     x25, x26, [sp, #-16]!
    stp     x27, x28, [sp, #-16]!
    stp     x29, x30, [sp, #-16]!

    /* Store updated sp (after pushes) to *old_sp_ptr */
    mov     x2, sp
    str     x2, [x0]

    /* Switch to new stack */
    mov     sp, x1

    /* Pop callee-saved regs from new stack */
    ldp     x29, x30, [sp], #16
    ldp     x27, x28, [sp], #16
    ldp     x25, x26, [sp], #16
    ldp     x23, x24, [sp], #16
    ldp     x21, x22, [sp], #16
    ldp     x19, x20, [sp], #16

    ret

/* ------------------------------------------------------------------
 * sched_start(u32 initial_sp)
 * Start the first task: set sp, pop regs, return to entry point.
 * ------------------------------------------------------------------ */
.global sched_start
.align 4
sched_start:
    mov     sp, x0
    ldp     x29, x30, [sp], #16
    ldp     x27, x28, [sp], #16
    ldp     x25, x26, [sp], #16
    ldp     x23, x24, [sp], #16
    ldp     x21, x22, [sp], #16
    ldp     x19, x20, [sp], #16
    ret

/* ------------------------------------------------------------------
 * Extern declarations
 * ------------------------------------------------------------------ */
.extern syscall_handler
.extern sched_switch_from_irq
.extern kernel_main

.section .bss.stack, "aw", @nobits
.align 12
_stack:
    .skip   32768
