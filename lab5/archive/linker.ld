ENTRY(_start)

SECTIONS
{
    . = 0x70000;
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = .;
}