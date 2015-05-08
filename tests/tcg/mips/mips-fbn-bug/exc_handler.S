        .text
        .globl exc_handler
        .set noreorder
exc_handler:
        mfc0  $t9, $14, 0
        addiu $t9, $t9, 4
        mtc0  $t9, $14, 0
        addiu $s0, $s0, 1
        eret
        nop
