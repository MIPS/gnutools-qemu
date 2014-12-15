ENTRY(_start)
SECTIONS
{
    . = 0x80000180;
    .exception_handler : { exceptions.o(*text) }
    . = 0x80100000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    . = 0x81000000;
    stack_addr = .;
}
