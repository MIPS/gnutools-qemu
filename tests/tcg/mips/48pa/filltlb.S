    .text
    .globl tlbfill
tlbfill:
    dmfc0 $t0, $10, 0
    dsrl  $t0, $t0, 12
    andi  $t0, $t0, 0xffff
    dsll  $t0, $t0, 26
    ori   $t0, $t0, 7
    dmtc0 $t0, $2, 0
    dmtc0 $t0, $3, 0
    tlbwi
    eret
    nop
