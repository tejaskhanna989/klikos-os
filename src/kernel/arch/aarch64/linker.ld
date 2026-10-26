ENTRY(_start)

SECTIONS
{
    /* QEMU virt loads the kernel at 0x40080000 via -kernel. */
    . = 0x40080000;

    _stext = .;

    .text.boot : {
        *(.text.boot)
    }

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        _sdata = .;
        *(.data)
        *(.data.*)
        _edata = .;
    }

    .bss : {
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _ebss = .;
    }

    _end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note*)
    }
}
