    .text
    .globl tlbfill
tlbfill:
    mfc0  $t0, $10, 0
    srl   $t0, $t0, 12
    srl   $t1, $t0, 12
    andi  $t1, $t1, 0xf   # t1 contains PFNX for hi(entrylo)
    andi  $t0, $t0, 0xfff
    sll   $t0, $t0, 18
    ori   $t0, $t0, 7
    mtc0  $t0, $2, 0
    mtc0  $t1, $2, 0 # will be patched -> MTHC0
    mtc0  $t0, $3, 0
    mtc0  $t1, $3, 0 # will be patched -> MTHC0
    tlbwi
    eret
    nop
