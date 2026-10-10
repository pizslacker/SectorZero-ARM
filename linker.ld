ENTRY(_start)
SECTIONS
{
    /* QEMU virt machine base RAM address */
    . = 0x40000000;
    
    /* Ensure our boot assembly is at the very beginning of the binary */
    .text.boot : { *(.text.boot) }
    
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    
    .bss : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
}