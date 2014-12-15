#include "macros.inc"

/* test if EPC is as expected (using s0 for keeping the value) */
TEST_ENTRY(simple)
        /* Assert CP0.Config3.LPA=1 */
        mfc0    $t0, $16, 3
        lui     $t1, 0x0
        ori     $t1, 0x00080
        and     $t0, $t0, $t1
        ASSERT_NOTZERO($t0)

        /* Assert CP0.PageGrain.ELPA=0 */
        mfc0    $t0, $5, 1
        lui     $t1, 0x2000
        and     $t0, $t0, $t1
        ASSERT_ZERO($t0)

        /* Test1: ELPA = 0, Assert PABITS = 32/36 (MIPS32/MIPS64) */
        lui     $t0, 0x3fff
        ori     $t0, 0xffc0
        mtc0    $t0, $2, 0 /* 0x3fffffc0 -> EntryLo (low) */
        lui     $t0, 0xffff
        ori     $t0, 0xffff
        mthc0   $t0, $2, 0 /* 0xffffffff -> EntryLo (high) */
        
        mfc0    $t0, $2, 0
        mfhc0   $t1, $2, 0
#if defined(MIPS32)
        lui     $t2, 0x03ff /* Expect EntryLo (low) = 0x03ffffc0 */
#else
        lui     $t2, 0x3fff /* Expect EntryLo (low) = 0x3fffffc0 */
#endif
        ori     $t2, 0xffc0
        ASSERT_EQ($t0, $t2)
        ASSERT_ZERO($t1)
        
        /* Test2: ELPA = 1, Assert PABITS = 40/48 */
        mfc0    $t0, $5, 1
        lui     $t1, 0x2000
        or      $t0, $t0, $t1
        mtc0    $t0, $5, 1
        /* Assert CP0.PageGrain.ELPA=1 */
        mfc0    $t0, $5, 1
        and     $t0, $t0, $t1
        ASSERT_NOTZERO($t0)

        lui     $t0, 0x3fff
        ori     $t0, 0xffc0
        mtc0    $t0, $2, 0
        lui     $t0, 0xffff
        ori     $t0, 0xffff
        mthc0   $t0, $2, 0
        
        mfc0    $t0, $2, 0
        mfhc0   $t1, $2, 0
        lui     $t2, 0x3fff
        ori     $t2, 0xffc0
        lui     $t3, 0x0
#if defined(MIPS32)
        ori     $t3, 0x000f
#else
        ori     $t3, 0x0fff
#endif
        ASSERT_EQ($t0, $t2)
        ASSERT_EQ($t1, $t3)
        
        /* TODO: Test3: Test with RI/XI bits */
        nop
        nop
        nop
TEST_END
