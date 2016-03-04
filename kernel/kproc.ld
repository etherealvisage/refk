OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS {
    code PT_LOAD;
    data PT_LOAD;
}

SECTIONS {
    . = 0x400000;
    .text : {
        *(.text)
        *(.text*)
    } :code

    . = 0x600000;

    .data : {
        *(.rodata)
        *(.data)
    } :data

    .bss : {
        *(.bss)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
    }
}
