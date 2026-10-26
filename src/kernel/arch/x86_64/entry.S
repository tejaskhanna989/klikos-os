/* entry.S — KlikOS x86_64 boot code.
 *
 * QEMU loads the kernel at 0x7C00 in real mode.
 * We: enable A20, set up GDT, enter protected mode, enable PAE,
 * set up 4-level paging, enable long mode, jump to 64-bit C kernel.
 */

.section .text.boot
.code16
.global _start
.align 4

/* Multiboot1 header - must be in first 8KB, 4-byte aligned */
.set ALIGN,  0x1
.set INFO,   0x2
.set FLAGS,  ALIGN | INFO
.set MAGIC,  0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.multiboot_header:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long _start          /* header_addr */
    .long _start          /* load_addr */
    .long _end            /* load_end_addr */
    .long _end            /* bss_end_addr */
    .long _start          /* entry_addr */

_start:
    cli

    /* Set up a temporary stack */
    movw    $0x0000, %ax
    movw    %ax, %ss
    movw    $0xFFFC, %sp

    /* Enable A20 gate */
    call    enable_a20

    /* Load GDT */
    lgdt    gdt_desc

    /* Enter protected mode (CR0.PE) */
    movl    %cr0, %eax
    orl     $0x1, %eax
    movl    %eax, %cr0

    /* Far jump to 32-bit protected mode */
    ljmp    $0x08, $pm32

/* ---- A20 enable via keyboard controller ---- */
enable_a20:
    call    .wait_kbd
    movb    $0xAD, %al
    outb    %al, $0x64
    call    .wait_kbd
    movb    $0xD0, %al
    outb    %al, $0x64
    call    .wait_kbd
    inb     $0x60, %al
    call    .wait_kbd
    movb    $0xD1, %al
    outb    %al, $0x64
    call    .wait_kbd
    movb    $0xDF, %al
    outb    %al, $0x60
    call    .wait_kbd
    movb    $0xAE, %al
    outb    %al, $0x64
    ret

.wait_kbd:
    call    .wait_kbd_2
.wait_kbd_2:
    inb     $0x64, %al
    testb   $0x02, %al
    jnz     .wait_kbd_2
    ret

/* ---- GDT (Global Descriptor Table) ---- */
gdt_start:
    .quad   0x0000000000000000          /* null descriptor */
    .quad   0x00AF9A000000FFFF          /* code: present, ring0, executable, readable */
    .quad   0x00AF92000000FFFF          /* data: present, ring0, writable */
gdt_end:
gdt_desc:
    .word   gdt_end - gdt_start - 1
    .quad   gdt_start

/* ---- 32-bit protected mode ---- */
.section .text
.code32
.align 4
pm32:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs

    /* Set up 32-bit stack */
    movl    $0x90000, %esp

    /* Patch page table base addresses (runtime, since symbols are relocatable) */
    movl    $pdpte_table, %eax
    orl     $0x7, %eax
    movl    %eax, pml4_table

    movl    $page_dir, %eax
    orl     $0x7, %eax
    movl    %eax, pdpte_table

    /* Enable PAE (CR4.PAE) */
    movl    %cr4, %eax
    orl     $0x20, %eax
    movl    %eax, %cr4

    /* Load CR3 with PML4 address */
    movl    $pml4_table, %eax
    movl    %eax, %cr3

    /* Enable IA32_EFER.LME (long mode enable) */
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $0x100, %eax
    wrmsr

    /* Enable paging (CR0.PG) */
    movl    %cr0, %eax
    orl     $0x80000000, %eax
    movl    %eax, %cr0

    /* Far jump into 64-bit long mode */
    ljmp    $0x08, $lm64

/* ---- Page tables (4KB aligned) ---- */
.section .data
.align 4096
pml4_table:
    .quad   0x7           /* flags only; base patched at runtime */
    .fill   511*8, 1, 0

pdpte_table:
    .quad   0x7           /* flags only; base patched at runtime */
    .fill   511*8, 1, 0

page_dir:
    /* PDE[0]: map first 2MB as a 2MB huge page (identity map) */
    .quad   0x83           /* present, writable, huge page */
    .fill   511*8, 1, 0

/* ---- 64-bit long mode ---- */
.section .text
.code64
.align 4
.global kernel_entry
kernel_entry:
lm64:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs

    /* Set up 64-bit stack */
    movq    $0x2000000, %rsp

    /* Clear upper 32 bits of registers */
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi

    /* Call the C kernel main */
    call    kernel_main

1:
    hlt
    jmp     1b

/* ---- IDT (Interrupt Descriptor Table) ---- */
.section .rodata
.align 8
idt:
    .fill   256 * 16, 1, 0          /* 256 entries × 16 bytes */

idt_ptr:
    .word   256 * 16 - 1            /* limit */
    .quad   idt                      /* base */

/* ---- IRQ0 handler (timer interrupt) ---- */
.section .text
.code64
.align 16
.global irq0_handler
.type   irq0_handler, @function
irq0_handler:
    /* Save all general-purpose registers */
    pushq   %rax
    pushq   %rbx
    pushq   %rcx
    pushq   %rdx
    pushq   %rsi
    pushq   %rdi
    pushq   %rbp
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %r11
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    /* Call sched_switch_from_irq(current_rsp) -> new_rsp in rax */
    movq    %rsp, %rdi
    call    sched_switch_from_irq
    movq    %rax, %rsp

    /* Restore all general-purpose registers */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rdi
    popq    %rsi
    popq    %rdx
    popq    %rcx
    popq    %rbx
    popq    %rax

    /* Stack layout after CPU interrupt entry (ring0->ring0):
     *   [rsp]   = RFLAGS
     *   [rsp+8] = CS
     *   [rsp+16] = RIP
     * iretq expects: [rsp]=RIP, [rsp+8]=CS, [rsp+16]=RFLAGS
     * So pop and re-push in the correct order.
     */
    popq    %rax          /* rax = RFLAGS */
    popq    %rcx          /* rcx = CS */
    popq    %rdx          /* rdx = RIP */
    pushq   %rdx          /* push RIP */
    pushq   %rcx          /* push CS */
    pushq   %rax          /* push RFLAGS */
    iretq

.size   irq0_handler, .-irq0_handler

/* ---- idt_init: called from C to set up IDT and enable interrupts ----
   Declared in sched.h, defined here so assembly can access idt symbol. */
.global idt_init
.type   idt_init, @function
idt_init:
    /* Set up IDT timer gate: irq0_handler at int 0x20 */
    leaq    idt(%rip), %rax          /* rax = &idt */
    leaq    0x200(%rax), %rax        /* rax = &idt[0x20] (entry 0x20) */
    movq    $irq0_handler, %rdx      /* rdx = handler address */

    /* offset_low (bytes 0-1) */
    movw    %dx, (%rax)
    shrq    $16, %rdx                /* shift to next 16 bits */

    /* selector (bytes 2-3) */
    movw    $0x0008, 2(%rax)         /* kernel code segment */

    /* ist + type_attr (byte 4) = 0x0E */
    movb    $0x0E, 4(%rax)

    /* attributes (byte 5) = 0x80 (present, ring0) */
    movb    $0x80, 5(%rax)

    /* offset_mid (bytes 6-7) */
    movw    %dx, 6(%rax)
    shrq    $16, %rdx                /* shift to high 32 bits */

    /* offset_high (bytes 8-15) */
    movl    %edx, 8(%rax)

    /* Load IDT */
    lidt    idt_ptr

    /* Enable interrupts */
    sti

    ret
