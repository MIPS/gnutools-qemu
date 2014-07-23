        .text
        .globl _reset
        .set    noreorder
        # s0 reg is used as a RI counter
        # t8 reg is used as an expected number of RI
_reset:
        lui    $sp, %hi(stack_top)
        ori    $sp, $sp, %lo(stack_top)
        and    $s0, $s0, $0
        and    $t8, $t8, $0

        mfc0  $t3, $12, 0
        lui   $t4, 0x2400 # Status.CU1, Status.FR 
        or    $t3, $t3, $t4
        mtc0  $t3, $12, 0
        
test0:
        mtc0    $0, $16, 5 # clear Config5.FRE

        # no exception
        mfc1    $t3, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        mtc1    $t3, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        abs.s   $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        add.s   $f3, $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        ceil.w.d  $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

test1:
        mfc0  $t3, $16, 5
        li    $t4, 0x100 # set Config5.FRE
        or    $t3, $t3, $t4
        mtc0  $t3, $16, 5
        
        # ---------- MFC1 ----------
        # no exception
        mfc1    $t3, $f2
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        mfc1    $t3, $f28
        ehb
        bne     $s0, $t8, test_failed
        nop
        
        addiu   $t8, $t8, 1        
        mfc1    $t3, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # ---------- MTC1 ----------
        addiu   $t8, $t8, 1
        mtc1    $t3, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        addiu   $t8, $t8, 1
        mtc1    $t4, $f2
        ehb
        bne     $s0, $t8, test_failed
        nop

        addiu   $t8, $t8, 1
        mtc1    $t3, $f14
        ehb
        bne     $s0, $t8, test_failed
        nop
#ifndef REL6
        # ---------- MOVZ.fmt ----------
        addiu   $t8, $t8, 1
        andi    $t1, $t1, 0
        movz.s  $f11, $f9, $t1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no except
        ori     $t1, $t1, 1
        movz.s  $f11, $f9, $t1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no except
        ori     $t1, $t1, 1
        movz.s  $f12, $f11, $t1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no except
        andi    $t1, $t1, 0
        movz.d  $f11, $f9, $t1
        ehb
        bne     $s0, $t8, test_failed
        nop
#endif
        # ---------- ADD.fmt ----------
        
        addiu   $t8, $t8, 1
        add.s   $f3, $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        add.d   $f3, $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # ---------- SUB.fmt ----------

        addiu   $t8, $t8, 1
        sub.s   $f23, $f22, $f21
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        sub.d   $f3, $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        addiu   $t8, $t8, 1
        sub.s   $f3, $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # ---------- ABS.fmt ----------
        
        addiu   $t8, $t8, 1
        abs.s   $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        abs.d   $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # ---------- MOV.fmt ----------

        addiu   $t8, $t8, 1
        mov.s   $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        mov.d   $f2, $f1
        ehb
        bne     $s0, $t8, test_failed
        nop

        # ---------- TRUNC / CEIL /fmt ----------
        
        addiu   $t8, $t8, 1
        trunc.l.s  $f2, $f5
        ehb
        bne     $s0, $t8, test_failed
        nop

        addiu   $t8, $t8, 1
        ceil.w.d  $f2, $f7
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        ceil.l.d  $f2, $f3
        ehb
        bne     $s0, $t8, test_failed
        nop

test2:  
        mtc0    $0, $16, 5 # clear Config5.FRE

        # no exception
        mfc1    $t5, $f8
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        mtc1    $t3, $f12
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        abs.s   $f12, $f21
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        add.s   $f12, $f26, $f31
        ehb
        bne     $s0, $t8, test_failed
        nop

        # no exception
        ceil.w.d  $f22, $f9
        ehb
        bne     $s0, $t8, test_failed
        nop
        
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
