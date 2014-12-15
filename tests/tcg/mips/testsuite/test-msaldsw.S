#include "macros.inc"

/* test if EPC is as expected (using s0 for keeping the value) */
EXCEPTION_HANDLER(custom_exc_handler)
        /* Check Cause */
        mfc0    $t0, $13, 0
        ext     $t1, $t0, 2, 5
        xori    $t0, $t1, 0x2 /* TLBL */
        ASSERT_ZERO($t0)

        /* Check EPC */
        mfc0    $t0, $14, 0
        ASSERT_EQ($s0, $t0)

        /* EPC += 4 and carry on */
        addiu   $t0, $t0, 4
        mtc0    $t0, $14, 0
        eret
EXCEPTION_HANDLER_END

TEST_ENTRY(simple)
        SET_EXCEPTION_HANDLER(custom_exc_handler)
        LOAD_ADDR($s0, gen_exception)

        /* Enable MSA */
        mfc0    $t0, $16, 5
        lui     $t1,  %hi(0x08000000)
        or      $t0, $t1, $t0
        mtc0    $t0, $16, 5
        
        nop
        nop
        nop
gen_exception:
        ld.d $w0, 0($0)
        nop
TEST_END
