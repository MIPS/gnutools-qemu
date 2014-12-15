.text
.globl _start
.set   noreorder
.align 4
_start:
        lui    $sp, %hi(stack_addr)
        ori    $sp, $sp, %lo(stack_addr)
        /* Clear BEV and ERL */
        mfc0   $t0, $12, 0
        lui    $t1, %hi(0x00400002)
        ori    $t1, %lo(0x00400002)
        or     $t0, $t0, $t1
        xor    $t0, $t0, $t1
        mtc0   $t0, $12, 0
        
        jal    main
        nop

#if !defined(MIPS32) && !defined(MIPS64)
#error missing target
#endif
        
