ENTRY(_reset)
SECTIONS
{
    . = 0x80000000;
    .filltlb . : { filltlb.o(.text) }
    . = 0x80100000;
    .startup . : { startup.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    . = 0x80200000;
    stack_top = .;
}
