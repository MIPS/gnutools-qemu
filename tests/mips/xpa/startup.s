        .text
        .globl _reset
        
        .set    noreorder
_reset:
        lui     $sp, %hi(stack_top)
        ori     $sp, $sp, %lo(stack_top)
        lui     $t9, %hi(entry)
        ori     $t9, %lo(entry)

        lui     $a0, 0x8000
        ori     $a0, 0x20
        bal     patch_mtc0
        nop

        lui     $a0, 0x8000
        ori     $a0, 0x28
        bal     patch_mtc0
        nop
        
        jalr    $t9
        nop

patch_mtc0: # expecting a0 to contain address to be patched
        lw      $t4, 0($a0)
        lui     $t5, 0x40c0
        or      $t4, $t5
        sw      $t4, 0($a0)
        jr      $ra
        nop
        