ENTRY(_reset)
SECTIONS
{
    . = 0x80000180;
    .exc_handler . : { exc_handler.o(.text) }
    . = 0x80100000;
    .startup . : { startup.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    . = 0x80200000;
    stack_top = .;
}
