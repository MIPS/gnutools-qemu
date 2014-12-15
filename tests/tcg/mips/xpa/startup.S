        .text
        .globl _reset
        
        .set    noreorder
_reset:
        mfc0    $t0, $12, 0
        lui     $t9, 0x40
        xor     $t0, $t0, $t9
        mtc0    $t0, $12, 0
        
        lui     $sp, %hi(stack_top)
        ori     $sp, $sp, %lo(stack_top)
        lui     $t9, %hi(entry)
        ori     $t9, %lo(entry)

        jalr    $t9
        nop
