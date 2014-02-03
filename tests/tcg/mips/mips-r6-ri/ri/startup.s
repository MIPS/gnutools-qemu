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
        .word 0b00000000000000000000000001000000 # ssnop
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100001100010000000000000000 # bbgezal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110001000000000000000000000 # bc1any2
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110010000000000000000000000 # bc1any4
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000000000000000000000000 # bc1f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000000100000000000000000 # bc1fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000000010000000000000000 # bc1t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000000110000000000000000 # bc1tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000000000000000000000000 # bc2f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000000100000000000000000 # bc2fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000000010000000000000000 # bc2t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000000110000000000000000 # bc2tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010000000000000000000000000000 # beql
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000100110000000000000000 # bgezall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000000110000000000000000 # bgezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011100000000000000000000000000 # bgtzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011000000000000000000000000000 # blezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100001100000000000000000000 # bltzal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000100100000000000000000 # bltzall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000000100000000000000000 # bltzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010100000000000000000000000000 # bnel
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111100000000000000000000000000 # cache
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000100000000000000000000110000 # ccondfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000100001 # clo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000100000 # clz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110110000000000000000000000 # cop1ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110100000000000000000100110 # cvtpspw
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110000000000000000000100110 # cvtpss
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000100101 # dclo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000100100 # dclz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011110 # ddiv
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011111 # ddivu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011010 # div
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011011 # divu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011100 # dmult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011101 # dmultu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b0000000000000000000000000001000 # jr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101000000000000000000000000000 # ldl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101100000000000000000000000000 # ldr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11000000000000000000000000000000 # ll
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11010000000000000000000000000000 # lld
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10001000000000000000000000000000 # lwl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100000000000000000000011001 # lwle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10011000000000000000000000000000 # lwr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100000000000000000000011010 # lwre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000000000 # madd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001100000000000000000000100000 # maddfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000000001 # maddu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000010000 # mfhi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000010010 # mflo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000000001 # movf
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000100000000000000000000010001 # movffmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000001011 # movn
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000100000000000000000000010011 # movnfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000010000000000000001 # movt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000100000000010000000000010001 # movtfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000001010 # movz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000100000000000000000000010010 # movzfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000000100 # msub
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001100000000000000000000101000 # msubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000000101 # msubu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000010001 # mthi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000010011 # mtlo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000000010 # mul
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011000 # mult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000000000000011001 # multu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001100000000000000000000111000 # nmsubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11001100000000000000000000000000 # pref
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001100000000000000000000000000 # ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11100000000000000000000000000000 # sc
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11110000000000000000000000000000 # scd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000000000000000000000111111 # sdbbp
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110000000000000000000000000000 # sdl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110100000000000000000000000000 # sdr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10101000000000000000000000000000 # swl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100000000000000000000100001 # swle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111000000000000000000000000000 # swr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100000000000000000000100010 # swre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000011000000000000000000 # teqi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000010000000000000000000 # tgei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000010010000000000000000 # tgeiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000010100000000000000000 # tlti
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000010110000000000000000 # tltiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100000011100000000000000000 # tnei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11101100000111000000000000000000 # undef
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100100010000100100010000 # bbgezal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110001001000000100100010000 # bc1any2
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110010001000000100100010000 # bc1any4
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001000000100100010000 # bc1f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001100000100100010000 # bc1fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001010000100100010000 # bc1t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001110000100100010000 # bc1tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000001000000100100010000 # bc2f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000001100000100100010000 # bc2fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000001010000100100010000 # bc2t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000001110000100100010000 # bc2tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010000100010100000100100010000 # beql
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100100110000100100010000 # bgezall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100000110000100100010000 # bgezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011100100000000000100100010000 # bgtzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011000100000000000100100010000 # blezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100100000000100100010000 # bltzal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100100100000100100010000 # bltzall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100000100000100100010000 # bltzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010100100010100000100100010000 # bnel
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111100100010100000100100010000 # cache
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000010000100000100110100 # ccondfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000000100001 # clo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000000100000 # clz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110110001000000100100010000 # cop1ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110100000000000000000100110 # cvtpspw
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110000000000000000000100110 # cvtpss
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000000100101 # dclo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000000100100 # dclz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011110 # ddiv
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011111 # ddivu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011010 # div
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011011 # divu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011100 # dmult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011101 # dmultu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b0000000010000000000000000001000 # jr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101000100010100000100100010000 # ldl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101100100010100000100100010000 # ldr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11000000100010100000100100010000 # ll
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11010000100010100000100100010000 # lld
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10001000100010100000100100010000 # lwl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100100010100010001000011001 # lwle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10011000100010100000100100010000 # lwr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100100010100010001000011010 # lwre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100000000000000000 # madd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010000100100000100100000 # maddfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100000000000000001 # maddu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000001000000010000 # mfhi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000000001000000010010 # mflo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100001000001000000000001 # movf
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001000100000100010001 # movffmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100001000000001011 # movn
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000010100100000100010011 # movnfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100001010001000000000001 # movt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000001010100000100010001 # movtfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100001000000001010 # movz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000010100100000100010010 # movzfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100000000000000100 # msub
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010000100100000100101000 # msubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100000000000000101 # msubu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100000000000000000010001 # mthi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100000000000000000010011 # mtlo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000000000010 # mul
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011000 # mult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000100010100000000000011001 # multu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010000100100000100111000 # nmsubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11001100100010100000100100010000 # pref
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001100100001000000100100010000 # ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11100000100010100000100100010000 # sc
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11110000100010100000100100010000 # scd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110000100010100001000100111111 # sdbbp
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110000100010100000100100010000 # sdl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110100100010100000100100010000 # sdr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10101000100010100000100100010000 # swl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100100010100010001000100001 # swle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111000100010100000100100010000 # swr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111100100010100010001000100010 # swre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100011000000100100010000 # teqi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100010000000100100010000 # tgei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100010010000100100010000 # tgeiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100010100000100100010000 # tlti
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100010110000100100010000 # tltiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000100100011100000100100010000 # tnei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11101100100111000100001000100001 # undef
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101100011011100101110101 # bbgezal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110001011011011100101110101 # bc1any2
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110010011011011100101110101 # bc1any4
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000101001011100101110101 # bc1f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000101101011100101110101 # bc1fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000101011011100101110101 # bc1t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000101111011100101110101 # bc1tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000101001011100101110101 # bc2f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000101101011100101110101 # bc2fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000101011011100101110101 # bc2t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000101111011100101110101 # bc2tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010011101010101011100101110101 # beql
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101100111011100101110101 # bgezall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101000111011100101110101 # bgezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011111101000001011100101110101 # bgtzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011011101000001011100101110101 # blezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101100001011100101110101 # bltzal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101100101011100101110101 # bltzall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101000101011100101110101 # bltzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010111101010101011100101110101 # bnel
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111101101010101011100101110101 # cache
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101010011100111010100110110 # ccondfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011000000100001 # clo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011000000100000 # clz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110110011011011100101110101 # cop1ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110100000000000000000100110 # cvtpspw
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110000000000000000000100110 # cvtpss
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011000000100101 # dclo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011000000100100 # dclz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011110 # ddiv
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011111 # ddivu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011010 # div
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011011 # divu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011100 # dmult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011101 # dmultu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b0000001110100000000000000001000 # jr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101001101010101011100101110101 # ldl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101101101010101011100101110101 # ldr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11000001101010101011100101110101 # ll
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11010001101010101011100101110101 # lld
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10001001101010101011100101110101 # lwl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111101101010100110101100011001 # lwle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10011001101010101011100101110101 # lwr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111101101010100110101100011010 # lwre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010100000000000000000 # madd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010101100111011110100000 # maddfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010100000000000000001 # maddu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000001011000000010000 # mfhi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000001011000000010010 # mflo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101101001011000000000001 # movf
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101010101000111011110010001 # movffmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010101011000000001011 # movn
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101010010100111011110010011 # movnfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101101011011000000000001 # movt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101010101010111011110010001 # movtfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010101011000000001010 # movz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101010010100111011110010010 # movzfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010100000000000000100 # msub
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010101100111011110101000 # msubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010100000000000000101 # msubu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101000000000000000010001 # mthi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101000000000000000010011 # mtlo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011000000000010 # mul
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011000 # mult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011101010100000000000011001 # multu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101010101100111011110111000 # nmsubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11001101101010101011100101110101 # pref
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001101101011011011100101110101 # ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11100001101010101011100101110101 # sc
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11110001101010101011100101110101 # scd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011101010101011001101111111 # sdbbp
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110001101010101011100101110101 # sdl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110101101010101011100101110101 # sdr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10101001101010101011100101110101 # swl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111101101010100110101100100001 # swle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111001101010101011100101110101 # swr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111101101010100110101100100010 # swre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101011001011100101110101 # teqi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101010001011100101110101 # tgei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101010011011100101110101 # tgeiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101010101011100101110101 # tlti
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101010111011100101110101 # tltiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111101011101011100101110101 # tnei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11101111101111001101011010110101 # undef
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111100011111111111111111 # bbgezal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110001111111111111111111111 # bc1any2
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110010111111111111111111111 # bc1any4
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000111001111111111111111 # bc1f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000111101111111111111111 # bc1fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000111011111111111111111 # bc1t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000101000111111111111111111111 # bc1tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000111001111111111111111 # bc2f
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000111101111111111111111 # bc2fl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000111011111111111111111 # bc2t
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001001000111111111111111111111 # bc2tl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010011111111111111111111111111 # beql
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111100111111111111111111 # bgezall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111000111111111111111111 # bgezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011111111000001111111111111111 # bgtzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01011011111000001111111111111111 # blezl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111100001111111111111111 # bltzal
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111100101111111111111111 # bltzall
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111000101111111111111111 # bltzl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01010111111111111111111111111111 # bnel
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111111111111111111111111111111 # cache
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000111111111111111111100111111 # ccondfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111100000100001 # clo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111100000100000 # clz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110110111111111111111111111 # cop1ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110100000000000000000100110 # cvtpspw
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000110000000000000000000100110 # cvtpss
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111100000100101 # dclo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111100000100100 # dclz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011110 # ddiv
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011111 # ddivu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011010 # div
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011011 # divu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011100 # dmult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011101 # dmultu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b0000001111100000000000000001000 # jr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101011111111111111111111111111 # ldl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01101111111111111111111111111111 # ldr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11000011111111111111111111111111 # ll
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11010011111111111111111111111111 # lld
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10001011111111111111111111111111 # lwl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111111111111111111111110011001 # lwle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10011011111111111111111111111111 # lwr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111111111111111111111110011010 # lwre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111110000000000000000 # madd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001111111111111111111111100000 # maddfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111110000000000000001 # maddu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000001111100000010000 # mfhi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000000000000001111100000010010 # mflo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111001111100000000001 # movf
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000111111111001111111111010001 # movffmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111111111100000001011 # movn
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000111111111111111111111010011 # movnfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111011111100000000001 # movt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000111111111011111111111010001 # movtfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111111111100000001010 # movz
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01000111111111111111111111010010 # movzfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111110000000000000100 # msub
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001111111111111111111111101000 # msubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111110000000000000101 # msubu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111000000000000000010001 # mthi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111000000000000000010011 # mtlo
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111100000000010 # mul
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011000 # mult
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000011111111110000000000011001 # multu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001111111111111111111111111000 # nmsubfmt
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11001111111111111111111111111111 # pref
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01001111111111111111111111111111 # ps
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11100011111111111111111111111111 # sc
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11110011111111111111111111111111 # scd
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01110011111111111111111111111111 # sdbbp
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110011111111111111111111111111 # sdl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10110111111111111111111111111111 # sdr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10101011111111111111111111111111 # swl
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111111111111111111111110100001 # swle
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b10111011111111111111111111111111 # swr
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b01111111111111111111111110100010 # swre
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111011001111111111111111 # teqi
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111010001111111111111111 # tgei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111010011111111111111111 # tgeiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111010101111111111111111 # tlti
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111010111111111111111111 # tltiu
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b00000111111011101111111111111111 # tnei
        ehb
        bne     $s0, $t8, test_failed
        nop
        addiu $t8, $t8, 1
        .word 0b11101111111111011111111111111111 # undef
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
