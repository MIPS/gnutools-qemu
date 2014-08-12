        .text
        .globl exc_handler
        .set noreorder
exc_handler:
        mfc0  $t9, $14, 0
        addiu $t9, $t9, 4
        mtc0  $t9, $14, 0
        beq   $s0, $s1, do_eretnc
        and   $s0, $t0, $t0
        eret
        nop
do_eretnc:
        eretnc
        nop
