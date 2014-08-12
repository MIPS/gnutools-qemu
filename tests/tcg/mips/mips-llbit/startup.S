        .text
        .globl _reset
        .set    noreorder
_reset:
        lui    $sp, %hi(stack_top)
        ori    $sp, $sp, %lo(stack_top)
        and    $s0, $s0, $0

        /* Prepare registers */
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        
        lui    $t2, 0x8030
        ori    $t2, 0x1000

        lui    $s1, 0
        ori    $s1, 1
        
        lui    $k0, 0xABCD
        ori    $k0, 0x1234
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        sw     $k0, 0($t3)
        
        ehb
        /* Test 1 - these stores should not clear llbit */
        /* $k1: value stored by sc, $t3: ll/sc addr */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)
        
        ll    $a0, 0($t3)
        sw    $t3, 16($t3)
        sw    $t3, 8($t3)
        sb    $t3, 8($t3)
        sh    $t3, -2($t3)
        sb    $t3, -1($t3)        
        sc    $k1, 0($t3)
        
        bne   $k1, $s1, test_failed
        nop

        ehb
        /* Test 2 - this store should clear the llbit */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)

        ll    $a0, 0($t3)
        sb    $t2, 7($t3)
        sc    $k1, 0($t3)
        
        bne   $k1, $0, test_failed
        nop

        ll    $a0, 0($t3)
        sw    $t2, 0($t3)
        sc    $k1, 0($t3)
        
        bne     $k1, $0, test_failed
        nop

        /* Test 3 - sc to different address */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)
        
        ll    $a0, 0($t3)
        sc    $k1, 8($t3)
        
        bne   $k1, $0, test_failed
        nop

        ehb

        /* Test 3 - eret should clear make sc to fail */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)
        
        ll    $a0, 0($t3)
        ori   $s0, $0, 0 /* exc_handler will use eretnc if s0 == 1 */
        break
        sc    $k1, 0($t3)
        
        bne   $k1, $0, test_failed
        nop

        /* Test 4 - eretnc shouldn't affect llbit */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)
        
        ll    $a0, 0($t3)
        ori   $s0, $0, 1 /* exc_handler will use eretnc if s0 == 1 */
        break
        sc    $k1, 0($t3)
        
        bne   $k1, $s1, test_failed
        nop

        ehb

        /* Test 4 - test mtc0 / mfc0 */
        lui    $k1, 0x1122
        ori    $k1, 0xAABB
        lui    $t3, 0x8030
        ori    $t3, 0xff30
        sw     $k0, 0($t3)
        
        ll    $a0, 0($t3)
        /* A - clear the llbit */
        mfc0  $s2, $17, 0 /* read llbit */
        andi  $s3, $s2, 1 /* s3 contains llbit */
        bne   $s3, $s1, test_failed
        nop
        
        mtc0  $0, $17, 0 /* clear llbit */
        
        mfc0  $s2, $17, 0 /* read llbit */
        andi  $s3, $s2, 1 /* s3 contains llbit */
        bne   $s3, $0, test_failed
        nop

        /* B - try to set the llbit (should fail) */
        ori   $s2, $0, 0xffff
        mtc0  $s2, $17, 0 /* try to set llbit */
        
        mfc0  $s2, $17, 0 /* read llbit */
        andi  $s3, $s2, 1 /* s3 contains llbit */
        bne   $s3, $0, test_failed
        nop
        
        sc    $k1, 0($t3)
        
        bne   $k1, $0, test_failed
        nop

        ehb

        /* End */

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
