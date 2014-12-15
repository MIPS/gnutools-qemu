#include "macros.inc"

.text
.global _test_failed, _test_passed, _default_handler, _do_print

.align 4
_default_handler:
        LOAD_ADDR($t0, msg_nohandler)
        PUSH_REG($t0)
        jal    _do_print
        nop
        j      _exit
        nop

_test_failed:
        LOAD_ADDR($t0, msg_fail)
        PUSH_REG($t0)
        jal    _do_print
        nop
        j      _exit
        nop

_test_passed:
        LOAD_ADDR($t0, msg_pass)
        PUSH_REG($t0)
        jal    _do_print
        nop
        j      _exit
        nop

_exit:
        lui     $t0,0x9f00
        ori     $t0,$t0,0x500
        li      $t1,0x42
        sw      $t1,0($t0)
        wait

_do_print:
        POP_REG($t0)
        LOAD_ADDR($t2, 0xb80003f8)
        
        lb     $t1, 0($t0)
        beq    $t1, $0, finish
        nop
loop:
        sw     $t1, 0($t2)
        addiu  $t0, $t0, 1
        lb     $t1, 0($t0)
        bne    $t1, $0, loop
        nop
finish:
        j      $ra
        nop

.data
msg_pass: .asciiz "test passed\n"
msg_fail: .asciiz "test failed\n"
msg_nohandler:  .asciiz "exception handler undeclared\n"
