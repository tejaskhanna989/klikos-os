/* syscalls.S — userspace syscall interface for aarch64-klikos-elf.
 *
 * Syscall numbers (KlikOS):
 *   0 = sys_exit
 *   1 = sys_write
 *   2 = sys_read
 *   3 = sys_open
 *   4 = sys_close
 *   5 = sys_yield
 *   6 = sys_sleep
 *   7 = sys_putchar
 *
 * Arguments passed in x0-x5, number in x8, result in x0.
 */

.section .text
.global write
.global read
.global open
.global close
.global exit
.global yield
.global sleep
.global putchar

write:
    mov     x8, #1
    svc     #0
    ret

read:
    mov     x8, #2
    svc     #0
    ret

open:
    mov     x8, #3
    svc     #0
    ret

close:
    mov     x8, #4
    svc     #0
    ret

exit:
    mov     x8, #0
    svc     #0
    ret

yield:
    mov     x8, #5
    svc     #0
    ret

sleep:
    mov     x8, #6
    svc     #0
    ret

putchar:
    mov     x8, #7
    svc     #0
    ret
