ENTRY(_start)

SECTIONS
{
    /* x86_64 kernel loads at 0x7C00 (boot sector address) for real mode */
    . = 0x7C00;

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
