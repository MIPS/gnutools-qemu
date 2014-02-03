        .text
        .globl _reset
        .set    noreorder
        # t7 reg is used as a RI counter
        # t8 reg is used as an expected number of RI
       
_reset:
        lui    $sp, %hi(stack_top)
        ori    $sp, $sp, %lo(stack_top)
        and    $s0, $s0, $0
        and    $t8, $t8, $0

test:

        
        beq     $s0, $t8, test_passed
        nop
test_failed:
        lui    $t9, %hi(fail)
        ori    $t9, %lo(fail)
        jalr   $t9
        nop
test_passed:
        lui    $t9, %hi(pass)
        ori    $t9, %lo(pass)
        jalr   $t9
        nop
