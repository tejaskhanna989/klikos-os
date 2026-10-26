/* context_switch.S — x86_64 context switch.
 *
 * Saves/restores callee-saved registers and switches stacks.
 * Used by the preemptive scheduler.
 */

.section .text
.global context_switch
.global sched_start
.align 16

/* context_switch(u64 *old_rsp_ptr, u64 new_rsp)
 * Save callee-saved regs on old stack, store rsp, switch to new stack,
 * restore callee-saved regs from new stack, return.
 */
context_switch:
    /* Save old rsp */
    movq    %rsp, (%rdi)

    /* Push callee-saved registers */
    pushq   %rbx
    pushq   %rbp
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    /* Store updated rsp (after pushes) to *old_rsp_ptr */
    movq    %rsp, (%rdi)

    /* Switch to new stack */
    movq    %rsi, %rsp

    /* Pop callee-saved registers from new stack */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbp
    popq    %rbx

    ret

/* sched_start(u64 initial_rsp)
 * Start the first task: set rsp, pop callee-saved regs, return.
 */
sched_start:
    movq    %rdi, %rsp
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbp
    popq    %rbx
    ret
