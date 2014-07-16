        .text
        .globl exc_handler
        .set noreorder
exc_handler:
        mfc0  $t3, $13, 0
        andi  $t3, $t3, 0xff
        li   $t4, 0x28
        bne   $t3, $t4, exc_return
        nop
increment:
        addiu $s0, $s0, 1
exc_return:
        mfc0  $t3, $14, 0
        addiu $t3, $t3, 4
        mtc0  $t3, $14, 0
        eret
        nop
