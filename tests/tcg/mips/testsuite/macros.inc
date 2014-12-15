/* k0 and k1 registers are reserved */

#define TEST_ENTRY(name) \
        .globl main;     \
        .align 4;        \
main:

#define ASSERT_EQ(r1, r2)    \
        bne r1, r2, do_fail; \
        nop;

#define ASSERT_NE(r1, r2)    \
        beq r1, r2, do_fail; \
        nop;

#define ASSERT_ZERO(r) ASSERT_EQ(r, $0)

#define ASSERT_NOTZERO(r) ASSERT_NE(r, $0)

#define TEST_FAIL       \
        j _test_failed; \
        nop

#define TEST_END        \
        j _test_passed; \
        nop
do_fail:                 \
        TEST_FAIL

#define LOAD_ADDR(r, label)         \
        lui    r, %hi(label);       \
        ori    r, r, %lo(label);

#define EXCEPTION_HANDLER(name) \
.text;                          \
.align 4;                       \
name:

#define EXCEPTION_HANDLER_END

#define SET_EXCEPTION_HANDLER(label)     \
        LOAD_ADDR($k0, label);           \
        sw     $k0, _actual_handler

#define FREE_EXCEPTION_HANDLER(label)    \
        SET_EXCEPTION_HANDLER(_default_handler)

#define PUSH_REG(r)               \
        addiu   $sp, $sp, -4;     \
        sw      r, 0($sp)

#define POP_REG(r)              \
        lw      r, 0($sp);      \
        addiu   $sp, $sp, 4
