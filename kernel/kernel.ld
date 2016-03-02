OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap)

kernel_pbase = 0x1000000; /* 16MB mark */
kernel_vbase = 0xffffffff80000000; /* 2**64 - 2**31, 2GB of space */

PHDRS {
    header PT_LOAD;
    code PT_LOAD;
    data PT_LOAD;
}

SECTIONS {
    . = kernel_pbase;

    .mb_header : { 
        multiboot.o (.mb_header)
    } :header

    .bootstrap : { 
        multiboot.o (.mb_text)
        multiboot.o (.mb_rodata)
        /* ensure page tables are aligned to 4KB boundary */
        . = ALIGN(4096);
        multiboot.o (.mb_bss)
    } :header
    
    . += kernel_vbase - kernel_pbase;
    _start = .;

    /* 4KB alignment */
    . = ALIGN(0x1000);
    
    .text : AT(ADDR(.text) - kernel_vbase + kernel_pbase) {
        _code = .;
        _code_phy_begin = . + kernel_pbase - kernel_vbase;
        *(EXCLUDE_FILE(*multiboot.o) .text)
        *(EXCLUDE_FILE(*multiboot.o) .rodata*)

        _eh_frame = .;
        *(.eh_frame)
        _code_end = .;
        _code_phy_end = . + kernel_pbase - kernel_vbase;
    } :code

    /* 4KB alignment */
    . = ALIGN(0x1000);
    
    .data : {
        _data = .;
        _data_phy_begin = . + kernel_pbase - kernel_vbase;
        *(.data)
    } :data
    
    .bss : {
        _bss_start = .;
        *(.bss)

        *(COMMON)
        _bss_end = .;
        _data_end = .;
        _data_phy_end = . + kernel_pbase - kernel_vbase;
    } :data
    /* 4KB alignment */
    . = ALIGN(0x1000);

    _end = .;

    /DISCARD/ : {
        *(.comment)
    }
}
