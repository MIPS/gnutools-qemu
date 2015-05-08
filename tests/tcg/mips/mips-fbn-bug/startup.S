        .text
        .globl _reset
        .set    noreorder
_reset:
        lui    $sp, %hi(stack_top)
        ori    $sp, $sp, %lo(stack_top)

        /* Initialize counter */
        lui    $s0, 0

        /* Clear Status.BEV */
        mfc0  $t9, $12, 0
        lui   $t3, 0x40
        xor   $t3, $t3, $t9
        mtc0  $t3, $12, 0

        /* Set Status.CU1 in delay slot */
        mfc0  $t9, $12, 0
        lui   $t3, 0x2000
        or    $t3, $t3, $t9
        b     check_cu1
        mtc0  $t3, $12, 0
        
check_cu1:
        mfc1  $t9, $1 /* Test if Status.CU1 is set. */
        ehb
        bne   $s0, $t9, test_failed

test_passed:
        lui    $t9, %hi(pass)
        ori    $t9, %lo(pass)
        jalr   $t9
        nop
test_failed:
        lui    $t9, %hi(fail)
        ori    $t9, %lo(fail)
        jalr   $t9
        nop
