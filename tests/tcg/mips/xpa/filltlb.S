    .text
    .globl tlbfill
tlbfill:
#ifdef TARGET64
    dmfc0 $t0, $10, 0
    dsrl  $t0, $t0, 12
    dsrl  $t1, $t0, 12
    andi  $t1, $t1, 0xf   # t1 contains PFNX for hi(entrylo)
    andi  $t0, $t0, 0xfff
    dsll  $t0, $t0, 18
    ori   $t0, $t0, 7
    dsll  $t1, $t1, 30
    or    $t1, $t1, $t0
    dmtc0 $t1, $2, 0
    dmtc0 $t1, $3, 0
#else
    mfc0  $t0, $10, 0
    srl   $t0, $t0, 12
    srl   $t1, $t0, 12
    andi  $t1, $t1, 0xf   # t1 contains PFNX for hi(entrylo)
    andi  $t0, $t0, 0xfff
    sll   $t0, $t0, 18
    ori   $t0, $t0, 7
    mtc0  $t0, $2, 0
    mthc0 $t1, $2, 0
    mtc0  $t0, $3, 0
    mthc0 $t1, $3, 0
#endif
    tlbwi
    eret
    nop
