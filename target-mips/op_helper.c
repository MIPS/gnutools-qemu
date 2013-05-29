/*
 *  MIPS emulation helpers for qemu.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
 *  Copyright (c) 2011 Reed Kotler/MIPS Technologies
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include "cpu.h"
#include "dyngen-exec.h"

#include "host-utils.h"

#include "helper.h"

#if !defined(CONFIG_USER_ONLY)
#include "softmmu_exec.h"
#endif /* !defined(CONFIG_USER_ONLY) */

#ifndef CONFIG_USER_ONLY
static inline void cpu_mips_tlb_flush (CPUState *env, int flush_global);

#ifdef MIPSSIM_COMPAT
#include "sysemu.h"

void helper_avp_ok(void)
{
    puts("ok");
    qemu_system_shutdown_request();
}

void helper_avp_fail(void)
{
    puts("fail");
    qemu_system_shutdown_request();
}
#endif
#endif

static inline void compute_hflags(CPUState *env)
{
    env->hflags &= ~(MIPS_HFLAG_COP1X | MIPS_HFLAG_64 | MIPS_HFLAG_CP0 |
                     MIPS_HFLAG_F64 | MIPS_HFLAG_FPU | MIPS_HFLAG_KSU |
                     MIPS_HFLAG_UX | MIPS_HFLAG_DSP);
    if (!(env->CP0_Status & (1 << CP0St_EXL)) &&
        !(env->CP0_Status & (1 << CP0St_ERL)) &&
        !(env->hflags & MIPS_HFLAG_DM)) {
        env->hflags |= (env->CP0_Status >> CP0St_KSU) & MIPS_HFLAG_KSU;
    }
#if defined(TARGET_MIPS64)
    if (((env->hflags & MIPS_HFLAG_KSU) != MIPS_HFLAG_UM) ||
        (env->CP0_Status & (1 << CP0St_PX)) ||
        (env->CP0_Status & (1 << CP0St_UX))) {
        env->hflags |= MIPS_HFLAG_64;
    }
    if (env->CP0_Status & (1 << CP0St_UX)) {
        env->hflags |= MIPS_HFLAG_UX;
    }
#endif
    if ((env->CP0_Status & (1 << CP0St_CU0)) ||
        !(env->hflags & MIPS_HFLAG_KSU)) {
        env->hflags |= MIPS_HFLAG_CP0;
    }
    if (env->CP0_Status & (1 << CP0St_CU1)) {
        env->hflags |= MIPS_HFLAG_FPU;
    }
    if (env->CP0_Status & (1 << CP0St_FR)) {
        env->hflags |= MIPS_HFLAG_F64;
    }
    if (env->CP0_Status & (1 << CP0St_MX)) {
        env->hflags |= MIPS_HFLAG_DSP;
    }
    if (env->insn_flags & ISA_MIPS32R2) {
        if (env->active_fpu.fcr0 & (1 << FCR0_F64)) {
            env->hflags |= MIPS_HFLAG_COP1X;
        }
    } else if (env->insn_flags & ISA_MIPS32) {
        if (env->hflags & MIPS_HFLAG_64) {
            env->hflags |= MIPS_HFLAG_COP1X;
        }
    } else if (env->insn_flags & ISA_MIPS4) {
        /* All supported MIPS IV CPUs use the XX (CU3) to enable
           and disable the MIPS IV extensions to the MIPS III ISA.
           Some other MIPS IV CPUs ignore the bit, so the check here
           would be too restrictive for them.  */
        if (env->CP0_Status & (1 << CP0St_CU3)) {
            env->hflags |= MIPS_HFLAG_COP1X;
        }
    }
}

/*****************************************************************************/
/* Exceptions processing helpers */

void helper_raise_exception_err (uint32_t exception, int error_code)
{
#if 0
    if (exception < 0x100)
        qemu_log("%s: %d %d\n", __func__, exception, error_code);
#endif
    env->exception_index = exception;
    env->error_code = error_code;
    cpu_loop_exit(env);
}

void helper_raise_exception (uint32_t exception)
{
    helper_raise_exception_err(exception, 0);
}

#if !defined(CONFIG_USER_ONLY)
static void do_restore_state (void *pc_ptr)
{
    TranslationBlock *tb;
    unsigned long pc = (unsigned long) pc_ptr;

    tb = tb_find_pc (pc);
    if (tb) {
        cpu_restore_state(tb, env, pc);
    }
}
#endif

#if defined(CONFIG_USER_ONLY)
#define HELPER_LD(name, insn, type)                                     \
static inline type do_##name(target_ulong addr, int mem_idx)            \
{                                                                       \
    return (type) insn##_raw(addr);                                     \
}
#else
#define HELPER_LD(name, insn, type)                                     \
static inline type do_##name(target_ulong addr, int mem_idx)            \
{                                                                       \
    switch (mem_idx)                                                    \
    {                                                                   \
    case 0: return (type) insn##_kernel(addr); break;                   \
    case 1: return (type) insn##_super(addr); break;                    \
    default:                                                            \
    case 2: return (type) insn##_user(addr); break;                     \
    }                                                                   \
}
#endif
HELPER_LD(lbu, ldub, uint8_t)
HELPER_LD(lw, ldl, int32_t)
#ifdef TARGET_MIPS64
HELPER_LD(ld, ldq, int64_t)
#endif
#undef HELPER_LD

#if defined(CONFIG_USER_ONLY)
#define HELPER_ST(name, insn, type)                                     \
static inline void do_##name(target_ulong addr, type val, int mem_idx)  \
{                                                                       \
    insn##_raw(addr, val);                                              \
}
#else
#define HELPER_ST(name, insn, type)                                     \
static inline void do_##name(target_ulong addr, type val, int mem_idx)  \
{                                                                       \
    switch (mem_idx)                                                    \
    {                                                                   \
    case 0: insn##_kernel(addr, val); break;                            \
    case 1: insn##_super(addr, val); break;                             \
    default:                                                            \
    case 2: insn##_user(addr, val); break;                              \
    }                                                                   \
}
#endif
HELPER_ST(sb, stb, uint8_t)
HELPER_ST(sw, stl, uint32_t)
#ifdef TARGET_MIPS64
HELPER_ST(sd, stq, uint64_t)
#endif
#undef HELPER_ST

target_ulong helper_clo (target_ulong arg1)
{
    return clo32(arg1);
}

target_ulong helper_clz (target_ulong arg1)
{
    return clz32(arg1);
}

#if defined(TARGET_MIPS64)
target_ulong helper_dclo (target_ulong arg1)
{
    return clo64(arg1);
}

target_ulong helper_dclz (target_ulong arg1)
{
    return clz64(arg1);
}
#endif /* TARGET_MIPS64 */

/* 64 bits arithmetic for 32 bits hosts */
static inline uint64_t get_HILO (void)
{
    return ((uint64_t)(env->active_tc.HI[0]) << 32) | (uint32_t)env->active_tc.LO[0];
}

static inline void set_HILO (uint64_t HILO)
{
    env->active_tc.LO[0] = (int32_t)HILO;
    env->active_tc.HI[0] = (int32_t)(HILO >> 32);
}

static inline void set_HIT0_LO (target_ulong arg1, uint64_t HILO)
{
    env->active_tc.LO[0] = (int32_t)(HILO & 0xFFFFFFFF);
    arg1 = env->active_tc.HI[0] = (int32_t)(HILO >> 32);
}

static inline void set_HI_LOT0 (target_ulong arg1, uint64_t HILO)
{
    arg1 = env->active_tc.LO[0] = (int32_t)(HILO & 0xFFFFFFFF);
    env->active_tc.HI[0] = (int32_t)(HILO >> 32);
}

/* utility functions for DSP */

static uint32_t mask32[33] = {
    0x00000000,
    0x00000001,
    0x00000003,
    0x00000007,
    0x0000000f,
    0x0000001f,
    0x0000003f,
    0x0000007f,
    0x000000ff,
    0x000001ff,
    0x000003ff,
    0x000007ff,
    0x00000fff,
    0x00001fff,
    0x00003fff,
    0x00007fff,
    0x0000ffff,
    0x0001ffff,
    0x0003ffff,
    0x0007ffff,
    0x000fffff,
    0x001fffff,
    0x003fffff,
    0x007fffff,
    0x00ffffff,
    0x01ffffff,
    0x03ffffff,
    0x07ffffff,
    0x0fffffff,
    0x1fffffff,
    0x3fffffff,
    0x7fffffff,
    0xffffffff
};

static uint32_t lmask32[32] = {
    0xfffffffe,
    0xfffffffc,
    0xfffffff8,
    0xfffffff0,
    0xffffffe0,
    0xffffffc0,
    0xffffff80,
    0xffffff00,
    0xfffffe00,
    0xfffffc00,
    0xfffff800,
    0xfffff000,
    0xffffe000,
    0xffffc000,
    0xffff8000,
    0xffff0000,
    0xfffe0000,
    0xfffc0000,
    0xfff80000,
    0xfff00000,
    0xffe00000,
    0xffc00000,
    0xff800000,
    0xff000000,
    0xfe000000,
    0xfc000000,
    0xf8000000,
    0xf0000000,
    0xe0000000,
    0xc0000000,
    0x80000000,
    0x00000000
};

static int reverse8[256] = {
    0x00, 0x80, 0x40, 0xc0,
    0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0,
    0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8,
    0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8,
    0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4,
    0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4,
    0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc,
    0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc,
    0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2,
    0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2,
    0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca,
    0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda,
    0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6,
    0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6,
    0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce,
    0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde,
    0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1,
    0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1,
    0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9,
    0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9,
    0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5,
    0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5,
    0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd,
    0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd,
    0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3,
    0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3,
    0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb,
    0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb,
    0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7,
    0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7,
    0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf,
    0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf,
    0x3f, 0xbf, 0x7f, 0xff
};

static uint32_t dsp_bit_mask[32] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000
};

static uint32_t dsp_invert_bit_mask[32] = {
    0xfffffffe, 0xfffffffd, 0xfffffffb, 0xfffffff7,
    0xffffffef, 0xffffffdf, 0xffffffbf, 0xffffff7f,
    0xfffffeff, 0xfffffdff, 0xfffffbff, 0xfffff7ff,
    0xffffefff, 0xffffdfff, 0xffffbfff, 0xffff7fff,
    0xfffeffff, 0xfffdffff, 0xfffbffff, 0xfff7ffff,
    0xffefffff, 0xffdfffff, 0xffbfffff, 0xff7fffff,
    0xfeffffff, 0xfdffffff, 0xfbffffff, 0xf7ffffff,
    0xefffffff, 0xdfffffff, 0xbfffffff, 0x7fffffff
};


#define DSP_SET_BIT(x, i, v) \
    ((x) = ((x) & dsp_invert_bit_mask[(i)]) | ((v) << (i)))

#define DSP_BIT_SET(x, i) (((x) & dsp_bit_mask[(i)]) >> (i))

union split_16 {
    uint16_t temp16;
    uint8_t temp8[2];
};

static inline uint16_t reverse16(uint16_t arg)
{
    union split_16 t1, t2;
    t1.temp16 = arg;

    t2.temp8[DSP_PAIR_LO] = reverse8[t1.temp8[DSP_PAIR_HI]];
    t2.temp8[DSP_PAIR_HI] = reverse8[t1.temp8[DSP_PAIR_LO]];

    return t2.temp16;
}

#define DSPCONTROL (env->active_tc.DSPControl)

/* should check ranges. on this pair */
static inline uint32_t getBitRange(uint32_t x, uint8_t l, uint8_t r)
{
    uint32_t m = lmask32[l] | mask32[r];
    return (x & ~m) >> r;
}


static inline uint32_t setBitRange(uint32_t x, uint32_t y, uint8_t l,
    uint8_t r)
{
    uint32_t m = lmask32[l] | mask32[r];
    return (x & m) | ((y << r) & ~m);
}

static inline uint8_t DSPCONTROL_getBitRange(uint8_t l, uint8_t r)
{
    return getBitRange(DSPCONTROL, l, r);
}

static inline void DSPCONTROL_setBitRange(uint32_t x, uint8_t l, uint8_t r)
{
    DSPCONTROL = setBitRange(DSPCONTROL, x, l, r);
}


#define DSPCONTROL_SET_GET_NAME(x, b) \
static inline uint8_t DSPCONTROL_##x(void) \
{ \
    return DSP_BIT_SET(DSPCONTROL, b); \
} \
\
static inline void set_DSPCONTROL_##x(uint8_t v) \
{ \
    DSP_SET_BIT(DSPCONTROL, b, v); \
}

#define DSPCONTROL_SET_GET(x) DSPCONTROL_SET_GET_NAME(x, x)

#define DSPCONTROL_SET_GET_RANGE(x, l, r) \
static inline uint8_t DSPCONTROL_##x(void) \
{ \
    return DSPCONTROL_getBitRange(l, r); \
} \
\
static inline void set_DSPCONTROL_##x(uint8_t i) \
{ \
    DSPCONTROL_setBitRange(i, l, r);\
}


DSPCONTROL_SET_GET_RANGE(ccond, 27, 24)
DSPCONTROL_SET_GET(27)
DSPCONTROL_SET_GET(26)
DSPCONTROL_SET_GET(25)
DSPCONTROL_SET_GET(24)
DSPCONTROL_SET_GET_RANGE(outflag, 23, 16)
DSPCONTROL_SET_GET(16)
DSPCONTROL_SET_GET(17)
DSPCONTROL_SET_GET(18)
DSPCONTROL_SET_GET(19)
DSPCONTROL_SET_GET(20)
DSPCONTROL_SET_GET(21)
DSPCONTROL_SET_GET(22)
DSPCONTROL_SET_GET(23)
DSPCONTROL_SET_GET(14)
DSPCONTROL_SET_GET(13)
DSPCONTROL_SET_GET_RANGE(scount, 12, 7)
DSPCONTROL_SET_GET_RANGE(pos, 5, 0)

#define set_DSPCONTROL_20_ON set_DSPCONTROL_20(1)


static inline uint8_t DSPCONTROL_c_13(void)
{
    return DSP_BIT_SET(DSPCONTROL, 13);
}

static inline void set_DSPCONTROL_outflagBit(uint8_t acc, uint8_t value)
{
    DSP_SET_BIT(DSPCONTROL, (16+acc), value);
}

union target_split {
    uint32_t temp;
    int32_t tempi;
    uint16_t temp16[2];
    int16_t tempi16[2];
    uint8_t temp8[4];
};

typedef union target_split split_32;

#define BIT8_7 ((uint8_t)1 << 7)
#define BIT16_8 ((uint16_t)1 << 8)
#define BIT8_TRIM(x) (x & 0xFF)
#define BIT8_UMAX 0xFF
#define BIT8_MAX 0x7F
#define BIT8_MIN 0x80

static inline uint8_t dsp_abs8(uint8_t x)
{
    if (x == BIT8_MIN) {
        set_DSPCONTROL_20_ON;
        return BIT8_MAX;
    }

    if (x & BIT8_7) {
        return -((int8_t)x);
    }

    return x;
}

static inline uint16_t multiplyU8U8(uint8_t a, uint8_t b)
{
    return (uint16_t)a * (uint16_t)b;
}

/****************************************************
 * DSP ASE R3 */

static inline int16_t multiplyS8S8(uint8_t a, uint8_t b)
{
    return (int16_t)a * (int16_t)b;
}

/* DSP ASE R3
 ***************************************************/

static inline uint8_t dsp_addU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if (result & BIT16_8) {
        set_DSPCONTROL_20_ON;
    }

    return BIT8_TRIM(result);
}

static inline uint8_t dsp_satAddU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if (result & BIT16_8) {
        set_DSPCONTROL_20_ON;
        return BIT8_UMAX;
    }

    return result;
}

static inline uint8_t dsp_rightShift1AddU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;
    return result >> 1;
}

static inline uint8_t dsp_roundRightShift1AddU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;
    result++;
    return result >> 1;
}

static inline uint8_t dsp_compareUnsignedBytesEq(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a == b;
    return result;
}

static inline uint8_t dsp_compareUnsignedBytesLt(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a < b;
    return result;
}

static inline uint8_t dsp_compareUnsignedBytesLe(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a <= b;
    return result;
}

static inline uint8_t dsp_subU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if (result & BIT16_8) {
        set_DSPCONTROL_20_ON;
    }

    return result;
}

static inline uint8_t dsp_satSubU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if (result & BIT16_8) {
        set_DSPCONTROL_20_ON;
        return 0;
    }

    return result;
}

static inline uint8_t dsp_rightShift1SubU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;
    return result >> 1;
}

static inline uint8_t dsp_roundRightShift1SubU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;
    result++;
    return result >> 1;
}

/****************************************************
 * DSP ASE R3 */

#define BIT16_7 ((uint16_t)1 << 7)

#define DSP8_BINARY_HEADER_SIGNED(name) \
    static inline uint8_t name(int8_t a_not_wide_enough, \
                               int8_t b_not_wide_enough) { \
    int16_t result; \
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

static inline uint8_t dsp_add8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if ((result & BIT16_8) != (result & BIT16_7)) {
        set_DSPCONTROL_20_ON;
    }

    return BIT8_TRIM(result);
}

static inline uint8_t dsp_satSAdd8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if ((result & BIT16_8) != (result & BIT16_7)) {
        set_DSPCONTROL_20_ON;
        return (result & BIT16_8) ? BIT8_MIN : BIT8_MAX;
    }

    return result;
}

static inline uint8_t dsp_absSub8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if ((result & BIT16_8)) {
        result = -result;
    }

    return result;
}

static inline uint8_t dsp_absSubU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if ((result & BIT16_8)) {
        result = -result;
    }

    return result;
}

static inline uint8_t dsp_sub8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if ((result & BIT16_8) != (result & BIT16_7)) {
        set_DSPCONTROL_20_ON;
    }

    return result;
}

static inline uint8_t dsp_satSSub8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if ((result & BIT16_8) != (result & BIT16_7)) {
        set_DSPCONTROL_20_ON;
        return (result & BIT16_8) ? BIT8_MIN : BIT8_MAX;
    }

    return result;
}

static inline uint8_t dsp_satSSubU8(uint8_t a_not_wide_enough,
    uint8_t b_not_wide_enough)
{
    uint16_t result;
    uint8_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if ((result & BIT16_8) != (result & BIT16_7)) {
        set_DSPCONTROL_20_ON;
        return (result & BIT16_8) ? BIT8_MIN : BIT8_MAX;
    }

    return result;
}

static inline uint8_t dsp_mul8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a * b;

    if (result > (int16_t)0x007F) {
        set_DSPCONTROL_21(1);
    } else if (result < (int16_t)0xFF80) {
        set_DSPCONTROL_21(1);
    }

    return (int8_t)result;
}

static inline uint8_t dsp_satSMul8(int8_t a_not_wide_enough,
    int8_t b_not_wide_enough)
{
    int16_t result;
    int16_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a * b;

    if (result > (int16_t)0x007F) {
        result = (int16_t)0x007F;
        set_DSPCONTROL_21(1);
    } else if (result < (int16_t)0xFF80) {
        result = (int16_t)0xFF80;
        set_DSPCONTROL_21(1);
    }

    return (int8_t)result;
}

/* DSP ASE R3
 ***************************************************/

#define do_dsp8_unary(func)\
    union target_split s, t;\
    s.temp = arg1;\
    t.temp8[DSP_QUAD_HI] = func(s.temp8[DSP_QUAD_HI]);\
    t.temp8[DSP_QUAD_HIMID] = func(s.temp8[DSP_QUAD_HIMID]);\
    t.temp8[DSP_QUAD_LOMID] = func(s.temp8[DSP_QUAD_LOMID]);\
    t.temp8[DSP_QUAD_LO] = func(s.temp8[DSP_QUAD_LO]);\
    return t.temp;

#define do_dsp8_binary(func) \
    union target_split s1, s2, t;\
    s1.temp = arg1;\
    s2.temp = arg2;\
    t.temp8[DSP_QUAD_HI] = func(s1.temp8[DSP_QUAD_HI],\
                                   s2.temp8[DSP_QUAD_HI]);\
    t.temp8[DSP_QUAD_HIMID] = func(s1.temp8[DSP_QUAD_HIMID],\
                                   s2.temp8[DSP_QUAD_HIMID]);\
    t.temp8[DSP_QUAD_LOMID] = func(s1.temp8[DSP_QUAD_LOMID],\
                                   s2.temp8[DSP_QUAD_LOMID]);\
    t.temp8[DSP_QUAD_LO] = func(s1.temp8[DSP_QUAD_LO],\
                                   s2.temp8[DSP_QUAD_LO]);\
    return t.temp;

#define do_dsp8_binary_cc(func) \
    union target_split s1, s2; \
    s1.temp = arg1; \
    s2.temp = arg2; \
    set_DSPCONTROL_27(func(s1.temp8[DSP_QUAD_HI],\
                           s2.temp8[DSP_QUAD_HI]));\
    set_DSPCONTROL_26(func(s1.temp8[DSP_QUAD_HIMID],\
                           s2.temp8[DSP_QUAD_HIMID]));\
    set_DSPCONTROL_25(func(s1.temp8[DSP_QUAD_LOMID],\
                           s2.temp8[DSP_QUAD_LOMID]));\
    set_DSPCONTROL_24(func(s1.temp8[DSP_QUAD_LO],\
                           s2.temp8[DSP_QUAD_LO]));

#define do_dsp8_binary_cc_resonly(func)\
    do {\
        union target_split s1, s2;\
        uint32_t foo;\
        s1.temp = arg1;\
        s2.temp = arg2;\
        foo = (func(s1.temp8[DSP_QUAD_HI], s2.temp8[DSP_QUAD_HI]) << 3) |\
            (func(s1.temp8[DSP_QUAD_HIMID], s2.temp8[DSP_QUAD_HIMID]) << 2) |\
            (func(s1.temp8[DSP_QUAD_LOMID], s2.temp8[DSP_QUAD_LOMID]) << 1) |\
            func(s1.temp8[DSP_QUAD_LO], s2.temp8[DSP_QUAD_LO]);\
        return foo;\
    } while (0)

#define do_dsp8_binary_cc_res(func)\
    do {\
        do_dsp8_binary_cc(func);\
        return DSPCONTROL_ccond();\
    } while (0)

#define do_dsp8_unary_helper(func, action)\
target_ulong helper_##func(target_ulong arg1)\
{\
    do_dsp8_unary(action);\
}

#define do_dsp8_binary_helper(func, action)\
target_ulong helper_##func(target_ulong arg1, target_ulong arg2)\
{\
    do_dsp8_binary(action);\
}

#define do_dsp8_binary_cc_helper(func, action)\
void helper_##func(target_ulong arg1, target_ulong arg2)\
{\
    do_dsp8_binary_cc(action);\
}

#define do_dsp8_binary_cc_resonly_helper(func, action)\
target_ulong helper_##func(target_ulong arg1, target_ulong arg2)\
{\
    do_dsp8_binary_cc_resonly(action);\
}

#define do_dsp8_binary_cc_res_helper(func, action)\
target_ulong helper_##func(target_ulong arg1, target_ulong arg2)\
{\
    do_dsp8_binary_cc_res(action);\
}

do_dsp8_unary_helper(absq_s_qb, dsp_abs8)
do_dsp8_binary_helper(addu_qb, dsp_addU8)
do_dsp8_binary_helper(addu_s_qb, dsp_satAddU8)
do_dsp8_binary_helper(adduh_qb, dsp_rightShift1AddU8)
do_dsp8_binary_helper(adduh_r_qb, dsp_roundRightShift1AddU8)
do_dsp8_binary_cc_resonly_helper(cmpgu_eq_qb, dsp_compareUnsignedBytesEq)
do_dsp8_binary_cc_resonly_helper(cmpgu_lt_qb, dsp_compareUnsignedBytesLt)
do_dsp8_binary_cc_resonly_helper(cmpgu_le_qb, dsp_compareUnsignedBytesLe)
do_dsp8_binary_cc_res_helper(cmpgdu_eq_qb, dsp_compareUnsignedBytesEq)
do_dsp8_binary_cc_res_helper(cmpgdu_lt_qb, dsp_compareUnsignedBytesLt)
do_dsp8_binary_cc_res_helper(cmpgdu_le_qb, dsp_compareUnsignedBytesLe)
do_dsp8_binary_cc_helper(cmpu_eq_qb, dsp_compareUnsignedBytesEq)
do_dsp8_binary_cc_helper(cmpu_lt_qb, dsp_compareUnsignedBytesLt)
do_dsp8_binary_cc_helper(cmpu_le_qb, dsp_compareUnsignedBytesLe)
do_dsp8_binary_helper(subuh_qb, dsp_rightShift1SubU8)
do_dsp8_binary_helper(subuh_r_qb, dsp_roundRightShift1SubU8)
do_dsp8_binary_helper(subu_qb, dsp_subU8)
do_dsp8_binary_helper(subu_s_qb, dsp_satSubU8)

/****************************************************
 * DSP ASE R3 */

do_dsp8_binary_helper(add_qb, dsp_add8)
do_dsp8_binary_helper(add_s_qb, dsp_satSAdd8)

do_dsp8_binary_helper(asub_qb, dsp_absSub8)
do_dsp8_binary_helper(asubu_qb, dsp_absSubU8)

do_dsp8_binary_helper(sub_qb, dsp_sub8)
do_dsp8_binary_helper(sub_s_qb, dsp_satSSub8)

do_dsp8_binary_helper(mul_qb, dsp_mul8)
do_dsp8_binary_helper(mul_s_qb, dsp_satSMul8)

do_dsp8_binary_helper(subus_s_qb, dsp_satSSubU8)

/* DSP ASE R3
 ***************************************************/

#define do_dsp16_unary(func) \
    do {\
        union target_split s, t; \
        s.temp = arg1; \
        t.temp16[DSP_PAIR_HI] = func(s.temp16[DSP_PAIR_HI]); \
        t.temp16[DSP_PAIR_LO] = func(s.temp16[DSP_PAIR_LO]); \
        return t.temp;\
    } while (0)

#define do_dsp16_binary(func) \
    do {\
        union target_split s1, s2, t;\
        s1.temp = arg1;\
        s2.temp = arg2;\
        t.temp16[DSP_PAIR_HI] = func(s1.temp16[DSP_PAIR_HI],\
            s2.temp16[DSP_PAIR_HI]);\
        t.temp16[DSP_PAIR_LO] = func(s1.temp16[DSP_PAIR_LO],\
            s2.temp16[DSP_PAIR_LO]);\
        return t.temp;\
    } while (0)

#define do_dsp16_binary_cc(func) \
    do {\
        union target_split s1, s2;\
        s1.temp = arg1;\
        s2.temp = arg2;\
        set_DSPCONTROL_25(func(s1.temp16[DSP_PAIR_HI],\
            s2.temp16[DSP_PAIR_HI]));\
        set_DSPCONTROL_24(func(s1.temp16[DSP_PAIR_LO],\
            s2.temp16[DSP_PAIR_LO]));\
        return;\
    } while (0)

static inline uint16_t dsp_abs16(uint16_t x)
{
    if (x == 0x8000) {
        set_DSPCONTROL_20_ON;
        return 0x7FFF;
    }

    if (x & (1 << 15)) {
        return -((int16_t)x);
    }

    return x;
}

static inline uint32_t dsp_multiplyQ15Q15_noacc(uint16_t a, uint16_t b)
{
    uint32_t result;

    if ((a == 0x8000) && (b == 0x8000)) {
        set_DSPCONTROL_21(1);
        result = 0x7FFFFFFF;
     }
    else {
        result =  ((int32_t)(int16_t)a * (int32_t)(int16_t)b) << 1;
    }

    return result;
}

static inline uint32_t dsp_multiplyQ15Q15(uint8_t acc, uint16_t a, uint16_t b)
{
    uint32_t result;

    if ((a == 0x8000) && (b == 0x8000)) {
        result = 0x7FFFFFFF;
        set_DSPCONTROL_outflagBit(acc, 1);
    } else {
        result = ((int32_t)(int16_t)a * (int32_t)(int16_t)b) << 1;
    }

    return result;
}


#define BIT32_15 ((uint32_t)1 << 15)
#define BIT32_16 ((uint32_t)1 << 16)
#define BIT16_TRIM(x) (x & 0xFFFF)
#define BIT16_UMAX 0xFFFF
#define BIT16_MAX 0x7FFF
#define BIT16_MIN 0x8000

#define EXTEND_15(x) (((x & BIT32_15) << 1) | x)
#define EXTEND_15_NO_MATCH(x) (((x >> 1) & BIT32_15) != (x & BIT32_15))

static inline uint16_t dsp_add16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) + EXTEND_15(b);

    if (EXTEND_15_NO_MATCH(result)) {
        set_DSPCONTROL_20_ON;
    }

    return BIT16_TRIM(result);
}


static inline uint16_t dsp_satAdd16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) +  EXTEND_15(b);

    if (EXTEND_15_NO_MATCH(result)) {
        if (result & BIT32_16) {
            result = BIT16_MIN;
        } else {
            result = BIT16_MAX;
        }

        set_DSPCONTROL_20_ON;
        return result;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_rightShift1Addq16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) + EXTEND_15(b);
    return BIT16_TRIM(result >> 1);
}

static inline uint16_t dsp_roundRightShift1Addq16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) + EXTEND_15(b);
    result++;
    return BIT16_TRIM(result >> 1);
}

static inline uint16_t dsp_unsignedAddIntegerHalfwords(
    uint16_t a_not_wide_enough, uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if (result & ~BIT16_UMAX) {
        set_DSPCONTROL_20_ON;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_unsignedAddIntegerHalfwordsSaturate(
    uint16_t a_not_wide_enough, uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if (result & ~BIT16_UMAX) {
        set_DSPCONTROL_20_ON;
        result = BIT16_UMAX;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_compareSignedHalfwordsEq(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = (int16_t)a == (int16_t)b;
    return result;
}

static inline uint16_t dsp_compareSignedHalfwordsLt(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = (int16_t)a < (int16_t)b;
    return result;
}

static inline uint16_t dsp_compareSignedHalfwordsLe(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = (int16_t)a <= (int16_t)b;
    return result;
}

static inline uint16_t dsp_sub16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) - EXTEND_15(b);

    if (EXTEND_15_NO_MATCH(result)) {
        set_DSPCONTROL_20_ON;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_satSub16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) -  EXTEND_15(b);

    if (EXTEND_15_NO_MATCH(result)) {
        if ((result & BIT32_16) == 0) {
            result = BIT16_MAX;
        } else {
            result = BIT16_MIN;
        }

        set_DSPCONTROL_20_ON;
        return result;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_rightShift1Subq16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) - EXTEND_15(b);
    return BIT16_TRIM(result >> 1);
}

static inline uint16_t dsp_roundRightShift1Subq16(uint16_t a_not_wide_enough,
    uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_15(a) - EXTEND_15(b);
    result++;

    return BIT16_TRIM(result >> 1);
}

static inline uint16_t dsp_unsignedSubIntegerHalfwords(
    uint16_t a_not_wide_enough, uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if (result & BIT32_16) {
        set_DSPCONTROL_20_ON;
    }

    return BIT16_TRIM(result);
}

static inline uint16_t dsp_unsignedSubIntegerHalfwordsSaturate(
    uint16_t a_not_wide_enough, uint16_t b_not_wide_enough)
{
    uint32_t result;
    uint32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a - b;

    if (result & BIT32_16) {
        set_DSPCONTROL_20_ON;
        return 0;
    }

    return BIT16_TRIM(result);
}

static inline int16_t dsp_multiply1616(int16_t a_not_wide_enough,
    int16_t b_not_wide_enough)
{
    int32_t result;
    int32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a*b;

    if (result > (int32_t)(0x7FFF)) {
        set_DSPCONTROL_21(1);
    } else if (result < (int32_t)(0xFFFF8000)) {
        set_DSPCONTROL_21(1);
    }

    return (int16_t)result;
}

static inline int16_t dsp_satMultiplyI16I16(int16_t a_not_wide_enough,
    int16_t b_not_wide_enough)
{
    int32_t result;
    int32_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = (int32_t)a * (int32_t)b;

    if (result > (int32_t)(0x7FFF)) {
        result = (int32_t)0x7FFF;
        set_DSPCONTROL_21(1);
    } else if (result < (int32_t)(0xFFFF8000)) {
        result = (int32_t)0xFFFF8000;
        set_DSPCONTROL_21(1);
    }

    return result;
}

static inline int16_t dsp_satMultiplyQ15Q15(int16_t a_not_wide_enough,
    int16_t b_not_wide_enough)
{
    int32_t result;
    int32_t a = a_not_wide_enough, b = b_not_wide_enough;

    if (((a & 0xFFFF) == 0x8000) & ((b & 0xFFFF) == 0x8000)) {
        result = 0x7FFF0000;
        set_DSPCONTROL_21(1);
    } else {
        result = a * b;
        result = result << 1;
    }

    return result >> 16;
}

#define do_dsp16_unary_helper(func, action) \
target_ulong helper_##func(target_ulong arg1) \
{ \
    do_dsp16_unary(action);\
}

#define do_dsp16_binary_helper(func, action) \
target_ulong helper_##func(target_ulong arg1, target_ulong arg2) \
{ \
    do_dsp16_binary(action);\
}

#define do_dsp16_binary_cc_helper(func, action) \
void helper_##func(target_ulong arg1, target_ulong arg2) \
{ \
    do_dsp16_binary_cc(action);\
}

do_dsp16_unary_helper(absq_s_ph, dsp_abs16)
do_dsp16_binary_helper(addq_ph, dsp_add16)
do_dsp16_binary_helper(addq_s_ph, dsp_satAdd16)
do_dsp16_binary_helper(addqh_ph, dsp_rightShift1Addq16)
do_dsp16_binary_helper(addqh_r_ph, dsp_roundRightShift1Addq16)
do_dsp16_binary_helper(addu_ph, dsp_unsignedAddIntegerHalfwords)
do_dsp16_binary_helper(addu_s_ph, dsp_unsignedAddIntegerHalfwordsSaturate)
do_dsp16_binary_cc_helper(cmp_eq_ph, dsp_compareSignedHalfwordsEq)
do_dsp16_binary_cc_helper(cmp_lt_ph, dsp_compareSignedHalfwordsLt)
do_dsp16_binary_cc_helper(cmp_le_ph, dsp_compareSignedHalfwordsLe)
do_dsp16_binary_helper(mul_ph, dsp_multiply1616)
do_dsp16_binary_helper(mul_s_ph, dsp_satMultiplyI16I16)
do_dsp16_binary_helper(mulq_s_ph, dsp_satMultiplyQ15Q15)
do_dsp16_binary_helper(subqh_ph, dsp_rightShift1Subq16)
do_dsp16_binary_helper(subqh_r_ph, dsp_roundRightShift1Subq16)
do_dsp16_binary_helper(subq_ph, dsp_sub16)
do_dsp16_binary_helper(subq_s_ph, dsp_satSub16)
do_dsp16_binary_helper(subu_ph, dsp_unsignedSubIntegerHalfwords)
do_dsp16_binary_helper(subu_s_ph, dsp_unsignedSubIntegerHalfwordsSaturate)

#define BIT64_31 ((uint64_t)(1) << 31)
#define BIT64_32 ((uint64_t)(1) << 32)
#define BIT32_TRIM(x) (x & 0xFFFFFFFF)
#define BIT32_MAX 0x7FFFFFFF
#define BIT32_MIN 0x80000000

#define EXTEND_31(x) (((x & BIT64_31) << 1) | x)
#define EXTEND_31_NO_MATCH(x) (((x >> 1) & BIT64_31) != (x & BIT64_31))

static inline uint32_t dsp_abs32(uint32_t x)
{
    if (x == 0x80000000) {
        set_DSPCONTROL_20_ON;
        return 0x7FFFFFFF;
    }

    if (x & (1 << 31)) {
        return -((int32_t)x);
    }

    return x;
}

static inline uint32_t dsp_bitReverse(uint32_t x)
{
    return reverse16(BIT16_TRIM(x));
}

static inline uint64_t dsp_multiplyQ31Q31(uint8_t acc, uint32_t a, uint32_t b)
{
    uint64_t result;

    if ((a == 0x80000000) && (b == 0x80000000)) {
        result = 0x7FFFFFFFFFFFFFFFULL;
        set_DSPCONTROL_outflagBit(acc, 1);
    } else {
        result = ((int64_t)(int32_t)a * (int64_t)(int32_t)b) << 1;
    }

    return result;
}

typedef struct sat_64_result {
    int64_t value;
    int saturated;
} sat_64_result;

static inline sat_64_result dsp_satAdd64(uint64_t a, uint64_t b)
{
    sat_64_result r;

    r.value = a + b;
    r.saturated = false;

    /* must have same sign to overflow */
    if (!((a ^ b) >> 63)) {
        /* if result sign differs from either operand then we overflowed */
        if ((r.value ^ a) >> 63) {
            r.value = (a & 0x8000000000000000ULL) ? 0x8000000000000000ULL
                                                  : 0x7fffffffffffffffULL;
            r.saturated = true;
        }
    }

    return r;
}

static inline sat_64_result dsp_satSub64(uint64_t a, uint64_t b)
{
    sat_64_result r;

    r.value = a - b;
    r.saturated = false;
    a = (a >> 63) + 0x7FFFFFFFFFFFFFFFULL;

    if ((int64_t)((a ^ b) & (a ^ r.value)) < 0) {
        r.value = a;
        r.saturated = true;
    }

    return r;
}

static inline uint32_t dsp_sat32AccumulateQ31(uint32_t acc, uint64_t acc_value,
    uint32_t b_)
{
    int64_t b = (int64_t)(int32_t)b_;
    uint64_t result = (int64_t)acc_value + (int64_t)b;

    if (((result >> 32) & 1) != ((result >> 31) & 1)) {
        if (((result >> 32) & 1) != 0) {
            result = 0x80000000;
        } else {
            result = 0x7FFFFFFF;
        }

        set_DSPCONTROL_outflagBit(acc, 1);
    }

    return result;
}


static inline uint32_t dsp_satAdd32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) + EXTEND_31(b);

    if (EXTEND_31_NO_MATCH(result)) {
        if (result & BIT64_32) {
            result = BIT32_MIN;
        } else {
            result = BIT32_MAX;
        }

        set_DSPCONTROL_20_ON;

        return result;
    }

    return BIT32_TRIM(result);
}

static inline uint32_t dsp_rightShift1Addq32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) + EXTEND_31(b);
    return BIT32_TRIM(result >> 1);
}

static inline uint32_t dsp_roundRightShift1Addq32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) + EXTEND_31(b);
    result++;

    return BIT32_TRIM(result >> 1);
}

static inline uint32_t dsp_addSignedWordSetCarryBit(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = a + b;

    if (result >> 32) {
        set_DSPCONTROL_13(1);
    } else {
        set_DSPCONTROL_13(0);
    }

    return BIT32_TRIM(result);
}

static inline uint32_t dsp_addWordWithCarryBit(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) + EXTEND_31(b) + DSPCONTROL_c_13();

    if (EXTEND_31_NO_MATCH(result)) {
        set_DSPCONTROL_20_ON;
    }

    return BIT32_TRIM(result);
}

static inline uint32_t dsp_leftShiftAndAppendBitsToLSB(uint32_t a,
    uint32_t b, uint32_t c)
{
    uint64_t result;

    if (c == 0) {
        result = b;
    } else {
        result = (b << c) | (a & mask32[c]);
    }

    return BIT32_TRIM(result);
}

static inline uint32_t dsp_byteAlignContents(uint32_t a, uint32_t b,
    uint32_t c)
{
    uint64_t result;

    if ((c == 0) | (c == 2)) {
        result = 0;
    } else {
        result = (b << (8 * c)) | (a >> (8 * (4 - c)));
    }

    return BIT32_TRIM(result);
}


static inline uint32_t dsp_satSub32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) - EXTEND_31(b);

    if (EXTEND_31_NO_MATCH(result)) {
        if (result & BIT64_32) {
            result = BIT32_MIN;
        } else {
            result = BIT32_MAX;
        }

        set_DSPCONTROL_20_ON;
        return result;
    }

    return BIT32_TRIM(result);
}

static inline uint32_t dsp_rightShift1Subq32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) - EXTEND_31(b);
    return BIT32_TRIM(result >> 1);
}

static inline uint32_t dsp_roundRightShift1Subq32(uint32_t a_not_wide_enough,
    uint32_t b_not_wide_enough)
{
    uint64_t result;
    uint64_t a = a_not_wide_enough, b = b_not_wide_enough;

    result = EXTEND_31(a) - EXTEND_31(b);
    result++;
    return BIT32_TRIM(result >> 1);
}

#define do_dsp32_unary_helper(func, action) \
target_ulong helper_##func(target_ulong arg1) \
{ \
    return action(arg1); \
}

#define do_dsp32_binary_helper(func, action) \
target_ulong helper_##func(target_ulong arg1, target_ulong arg2) \
{ \
    return action(arg1, arg2); \
}

#define do_dsp32_ternary_helper(func, action) \
target_ulong helper_##func(target_ulong arg1, target_ulong arg2, \
    uint32_t arg3) \
{ \
    return action(arg1, arg2, arg3); \
}

do_dsp32_unary_helper(absq_s_w, dsp_abs32)
do_dsp32_binary_helper(addq_s_w, dsp_satAdd32)
do_dsp32_binary_helper(addqh_w, dsp_rightShift1Addq32)
do_dsp32_binary_helper(addqh_r_w, dsp_roundRightShift1Addq32)
do_dsp32_binary_helper(addsc, dsp_addSignedWordSetCarryBit)
do_dsp32_binary_helper(addwc, dsp_addWordWithCarryBit)
do_dsp32_ternary_helper(append, dsp_leftShiftAndAppendBitsToLSB)
do_dsp32_ternary_helper(balign, dsp_byteAlignContents)
do_dsp32_unary_helper(bitrev, dsp_bitReverse)
do_dsp32_binary_helper(subqh_r_w, dsp_roundRightShift1Subq32)
do_dsp32_binary_helper(subqh_w, dsp_rightShift1Subq32)
do_dsp32_binary_helper(subq_s_w, dsp_satSub32)

/* pure dsp helper without arguments */

target_ulong helper_posge32(void)
{
    return DSPCONTROL_pos() >= 32;
}

uint64_t helper_dpa_w_ph(uint64_t ac, target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;
    int64_t tempA, tempB;

    a1.temp = arg1;
    a2.temp = arg2;

    tempA = (int64_t)((int32_t)a1.tempi16[DSP_PAIR_HI] *
                      (int32_t)a2.tempi16[DSP_PAIR_HI]);

    tempB = (int64_t)((int32_t)a1.tempi16[DSP_PAIR_LO] *
                      (int32_t)a2.tempi16[DSP_PAIR_LO]);

    result = ((int64_t)ac + (tempA + tempB));

    return result;
}

uint64_t helper_dps_w_ph(uint64_t ac, target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;
    int64_t tempA, tempB;

    a1.temp = arg1;
    a2.temp = arg2;

    tempA = (int64_t)((int32_t)a1.tempi16[DSP_PAIR_HI] *
                      (int32_t)a2.tempi16[DSP_PAIR_HI]);

    tempB = (int64_t)((int32_t)a1.tempi16[DSP_PAIR_LO] *
                      (int32_t)a2.tempi16[DSP_PAIR_LO]);

    result = ((int64_t)ac - (tempA + tempB));

    return result;
}

uint64_t helper_dpaq_s_w_ph(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_HI],
                                           a2.temp16[DSP_PAIR_HI]);

    int32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_LO],
                                           a2.temp16[DSP_PAIR_LO]);

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    uint64_t retval = (int64_t)((int64_t)ac_value + dotp);

    return retval;
}

uint64_t helper_dpsq_s_w_ph(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_HI],
                                           a2.temp16[DSP_PAIR_HI]);

    int32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_LO],
                                           a2.temp16[DSP_PAIR_LO]);

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    uint64_t retval = (int64_t)((int64_t)ac_value - dotp);

    return retval;
}

uint64_t helper_dpaqx_sa_w_ph(uint32_t acc, uint64_t acc_value,
    target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) ,
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO])));

    int32_t tempA = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])),
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI])));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)(dotp + (int64_t)acc_value);

    if ((((result >> 63) & 1) == 0) && (((result >> 31) & 0xFFFFFFFF) != 0)) {
        result = 0x7FFFFFFF;
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    if ((((result >> 63) & 1) == 1) &&
        (((result >> 31) & 0xFFFFFFFF) != 0xFFFFFFFF)) {
        result = 0xFFFFFFFF80000000ULL;
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    return result;
}

uint64_t helper_dpsqx_sa_w_ph(uint32_t acc, uint64_t acc_value,
    target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) ,
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO])));

    int32_t tempA = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])),
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI])));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)((int64_t)acc_value - dotp);

    if ((((result >> 63) & 1) == 0) && (((result >> 31) & 0xFFFFFFFF) != 0)) {
        result = 0x7FFFFFFF;
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    if ((((result >> 63) & 1) == 1) &&
        (((result >> 31) & 0xFFFFFFFF) != 0xFFFFFFFF)) {
        result = 0xFFFFFFFF80000000ULL;
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    return result;
}

uint64_t helper_dpaq_sa_l_w(uint32_t acc, uint64_t acc_value_u,
    target_ulong arg1, target_ulong arg2)
{
    int64_t dotp = dsp_multiplyQ31Q31(acc, arg1, arg2);
    int64_t acc_value = acc_value_u;
    sat_64_result result = dsp_satAdd64(dotp , acc_value);

    if (result.saturated) {
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    return result.value;
}

uint64_t helper_dpsq_sa_l_w(uint32_t acc, uint64_t acc_value_u,
    target_ulong arg1, target_ulong arg2)
{
    int64_t dotp = dsp_multiplyQ31Q31(acc, arg1, arg2);
    int64_t acc_value = acc_value_u;
    sat_64_result result = dsp_satSub64(acc_value, dotp);

    if (result.saturated) {
        set_DSPCONTROL_outflagBit(acc, 1);
    }

    return result.value;
}

uint64_t helper_dpau_h_qbl(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempB = multiplyU8U8(a1.temp8[DSP_QUAD_HI],
                                  a2.temp8[DSP_QUAD_HI]);

    uint16_t tempA = multiplyU8U8(a1.temp8[DSP_QUAD_HIMID],
                                  a2.temp8[DSP_QUAD_HIMID]);

    uint64_t dotp = tempA + tempB;
    result = acc_value + dotp;

    return result;
}

uint64_t helper_dpsu_h_qbl(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempB = multiplyU8U8(a1.temp8[DSP_QUAD_HI],
                                  a2.temp8[DSP_QUAD_HI]);

    uint16_t tempA = multiplyU8U8(a1.temp8[DSP_QUAD_HIMID],
                                  a2.temp8[DSP_QUAD_HIMID]);

    uint64_t dotp = tempA + tempB;
    result = acc_value - dotp;

    return result;
}

uint64_t helper_dpau_h_qbr(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempB = multiplyU8U8(a1.temp8[DSP_QUAD_LOMID],
                                  a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA = multiplyU8U8(a1.temp8[DSP_QUAD_LO],
                                  a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB;
    result = acc_value + dotp;

    return result;
}

uint64_t helper_dpsu_h_qbr(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempB = multiplyU8U8(a1.temp8[DSP_QUAD_LOMID],
                                  a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA = multiplyU8U8(a1.temp8[DSP_QUAD_LO],
                                  a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB;
    result = acc_value - dotp;

    return result;
}

uint64_t helper_dpax_w_ph(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) *
                    (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO]));

    int32_t tempA = (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])) *
                    (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI]));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)(dotp + (int64_t)acc_value);

    return result;
}

uint64_t helper_dpsx_w_ph(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) *
                    (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO]));

    int32_t tempA = (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])) *
                    (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI]));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)((int64_t)acc_value - dotp);

    return result;
}

uint64_t helper_dpaqx_s_w_ph(uint32_t acc, uint64_t acc_value,
    target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) ,
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO])));

    int32_t tempA = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])),
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI])));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)(dotp + (int64_t)acc_value);

    return result;
}

uint64_t helper_dpsqx_s_w_ph(uint32_t acc, uint64_t acc_value,
    target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_HI])) ,
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_LO])));

    int32_t tempA = dsp_multiplyQ15Q15(acc,
            (int32_t)((int16_t)(a1.temp16[DSP_PAIR_LO])),
            (int32_t)((int16_t)(a2.temp16[DSP_PAIR_HI])));

    int64_t dotp = (int64_t)tempB + (int64_t)tempA;
    result = (int64_t)((int64_t)acc_value - dotp);

    return result;
}

/****************************************************
 * DSP ASE R3 */

uint64_t helper_dpa_h_qb(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempD =
        multiplyS8S8(a1.temp8[DSP_QUAD_HI], a2.temp8[DSP_QUAD_HI]);

    uint16_t tempC =
        multiplyS8S8(a1.temp8[DSP_QUAD_HIMID], a2.temp8[DSP_QUAD_HIMID]);

    uint16_t tempB =
        multiplyS8S8(a1.temp8[DSP_QUAD_LOMID], a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA =
        multiplyS8S8(a1.temp8[DSP_QUAD_LO], a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB + tempC + tempD;
    result = acc_value + dotp;

    return result;
}

uint64_t helper_dpau_h_qb(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempD =
        multiplyU8U8(a1.temp8[DSP_QUAD_HI], a2.temp8[DSP_QUAD_HI]);

    uint16_t tempC =
        multiplyU8U8(a1.temp8[DSP_QUAD_HIMID], a2.temp8[DSP_QUAD_HIMID]);

    uint16_t tempB =
        multiplyU8U8(a1.temp8[DSP_QUAD_LOMID], a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA =
        multiplyU8U8(a1.temp8[DSP_QUAD_LO], a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB + tempC + tempD;
    result = acc_value + dotp;

    return result;
}

uint64_t helper_dps_h_qb(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempD =
        multiplyS8S8(a1.temp8[DSP_QUAD_HI], a2.temp8[DSP_QUAD_HI]);

    uint16_t tempC =
        multiplyS8S8(a1.temp8[DSP_QUAD_HIMID], a2.temp8[DSP_QUAD_HIMID]);

    uint16_t tempB =
        multiplyS8S8(a1.temp8[DSP_QUAD_LOMID], a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA =
        multiplyS8S8(a1.temp8[DSP_QUAD_LO], a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB + tempC + tempD;
    result = acc_value - dotp;

    return result;
}

uint64_t helper_dpsu_h_qb(uint64_t acc_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    uint16_t tempD =
        multiplyU8U8(a1.temp8[DSP_QUAD_HI], a2.temp8[DSP_QUAD_HI]);

    uint16_t tempC =
        multiplyU8U8(a1.temp8[DSP_QUAD_HIMID], a2.temp8[DSP_QUAD_HIMID]);

    uint16_t tempB =
        multiplyU8U8(a1.temp8[DSP_QUAD_LOMID], a2.temp8[DSP_QUAD_LOMID]);

    uint16_t tempA =
        multiplyU8U8(a1.temp8[DSP_QUAD_LO], a2.temp8[DSP_QUAD_LO]);

    uint64_t dotp = tempA + tempB + tempC + tempD;
    result = acc_value - dotp;

    return result;
}

/* DSP ASE R3
 ***************************************************/

/* not really a size, size + 1 bits are extracted */
static inline target_ulong dsp_extp(uint64_t acc_value, uint32_t size,
    uint32_t do_dp)
{
    uint8_t pos = DSPCONTROL_pos();

    acc_value = acc_value << (63 - pos);
    acc_value = acc_value >> (63 - size);

    if (pos < size) {
        set_DSPCONTROL_14(1);
    } else {
        set_DSPCONTROL_14(0);
        if (do_dp) {
            pos -= (size+1);
            set_DSPCONTROL_pos(pos);
        }
    }

    return acc_value;
}

target_ulong helper_extp(uint64_t acc_value, uint32_t size)
{
    return dsp_extp(acc_value, size, 0);
}

target_ulong helper_extpdp(uint64_t acc_value, uint32_t size)
{
    return dsp_extp(acc_value, size, 1);
}

target_ulong helper_extpv(uint64_t acc_value, target_ulong size)
{
    size = size & 0x1f;
    return dsp_extp(acc_value, size, 0);
}

target_ulong helper_extpdpv(uint64_t acc_value, target_ulong size)
{
    size = size & 0x1f;
    return dsp_extp(acc_value, size, 1);
}

static inline uint64_t dsp_shiftShortAccRightArithmetic(uint64_t acc_value,
    uint32_t shift)
{
    uint64_t x = ((int64_t)acc_value >> (shift & 0x1f));

    if ((acc_value >> 63) != (x >> 63)) {
        set_DSPCONTROL_23(1);
    }

    return x;
}

typedef struct dsp_uint65_t {
    uint8_t bit64;
    uint64_t bits0_63;
} dsp_uint65_t;

static inline dsp_uint65_t dsp__shiftShortAccRightArithmetic
    (uint64_t acc_value, uint32_t shift)
{
    dsp_uint65_t result;

    if (shift == 0) {
        result.bit64 = acc_value >> 63;
        result.bits0_63 = acc_value << 1;
    } else {
        int sign = acc_value >> 63;
        shift = shift & 0x1f;
        result.bits0_63 = ((int64_t)acc_value >> (shift - 1));
        result.bit64 = sign;
    }

    return result;
}

target_ulong helper_extr_s_h(uint64_t acc_value, uint32_t shift)
{
    uint64_t x = dsp_shiftShortAccRightArithmetic(acc_value, shift);

    if ((int64_t)x > 0x7FFF) {
        x = 0x7FFF;
        set_DSPCONTROL_23(1);
    } else if ((int64_t)x < (int64_t)(uint64_t)0xFFFFFFFFFFFF8000ULL) {
        x = (x & 0xFFFFFFFF00000000ULL) | 0xFFFF8000;
        set_DSPCONTROL_23(1);
    }

    return x & 0xFFFFFFFF;
}

target_ulong helper_extrv_s_h(uint64_t acc_value, target_ulong shift)
{
    return helper_extr_s_h(acc_value, shift);
}

inline target_ulong helper_extr_w(uint64_t acc_value, uint32_t shift)
{
    dsp_uint65_t temp = dsp__shiftShortAccRightArithmetic(acc_value, shift);
    uint32_t result;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        set_DSPCONTROL_23(1);
    }

    result = (temp.bits0_63 >> 1);

    if (temp.bits0_63 == 0xFFFFFFFFFFFFFFFFULL) {
        temp.bit64 = (temp.bit64 == 0) ? 1 : 0;
    }

    temp.bits0_63++;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        set_DSPCONTROL_23(1);
    }

    return result;
}

inline target_ulong helper_extr_r_w(uint64_t acc_value, uint32_t shift)
{
    dsp_uint65_t temp = dsp__shiftShortAccRightArithmetic(acc_value, shift);
    uint32_t result;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        set_DSPCONTROL_23(1);
    }

    if (temp.bits0_63 == 0xFFFFFFFFFFFFFFFFULL) {
        temp.bit64 = (temp.bit64 == 0) ? 1 : 0;
    }

    temp.bits0_63++;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        set_DSPCONTROL_23(1);
    }

    result = (temp.bits0_63 >> 1);

    return result;
}

inline target_ulong helper_extr_rs_w(uint64_t acc_value, uint32_t shift)
{
    dsp_uint65_t temp = dsp__shiftShortAccRightArithmetic(acc_value, shift);
    uint32_t result;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        set_DSPCONTROL_23(1);
    }

    if (temp.bits0_63 == 0xFFFFFFFFFFFFFFFFULL) {
        temp.bit64 = (temp.bit64 == 0) ? 1 : 0;
    }

    temp.bits0_63++;

    if (!(((temp.bits0_63 >> 32) == 0) && (temp.bit64 == 0)) &&
        !(((temp.bits0_63 >> 32) == 0xFFFFFFFF) && (temp.bit64 == 1))) {
        if (temp.bit64 == 0) {
            result = 0x7FFFFFFF;
        } else {
            result = 0x80000000;
        }

        set_DSPCONTROL_23(1);
    } else {
        result = (temp.bits0_63 >> 1);
    }

    return result;
}


inline target_ulong helper_extrv_w(uint64_t acc_value, target_ulong shift)
{
    return helper_extr_w(acc_value, shift & 0x1f);
}

inline target_ulong helper_extrv_r_w(uint64_t acc_value, target_ulong shift)
{
    return helper_extr_r_w(acc_value, shift & 0x1f);
}

inline target_ulong helper_extrv_rs_w(uint64_t acc_value, target_ulong shift)
{
    return helper_extr_rs_w(acc_value, shift & 0x1f);
}

inline target_ulong helper_insv(target_ulong rs, target_ulong rt)
{
    uint32_t size = DSPCONTROL_scount(); (void)size;
    uint32_t pos = DSPCONTROL_pos(); (void)pos;

    if ((pos > 32) || (size > 32) || ((pos + size) > 32)) {
        return rt;
    }

    rs = rs & mask32[size];
    rt = setBitRange(rt, rs, pos + size - 1, pos);

    return rt;
}

uint64_t helper_maq_s_w_phl(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_HI],
                                            a2.temp16[DSP_PAIR_HI]);

    uint64_t tempB = ac_value + (int64_t)(int32_t)tempA;

    return tempB;
}

uint64_t helper_maq_s_w_phr(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_LO],
                                            a2.temp16[DSP_PAIR_LO]);

    uint64_t tempB = ac_value + (int64_t)(int32_t)tempA;

    return tempB;
}

uint64_t helper_maq_sa_w_phl(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_HI],
                                            a2.temp16[DSP_PAIR_HI]);

    tempA = dsp_sat32AccumulateQ31(ac, ac_value, tempA);

    uint64_t tempB = (int64_t)(int32_t)tempA;

    return tempB;
}

uint64_t helper_maq_sa_w_phr(uint32_t ac, uint64_t ac_value, target_ulong arg1,
    target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t tempA = dsp_multiplyQ15Q15(ac, a1.temp16[DSP_PAIR_LO],
                                            a2.temp16[DSP_PAIR_LO]);

    tempA = dsp_sat32AccumulateQ31(ac, ac_value, tempA);

    uint64_t tempB = (int64_t)(int32_t)tempA;

    return tempB;
}

target_ulong helper_modsub(target_ulong rs, target_ulong rt)
{
    uint32_t result;
    uint16_t decr = rt & 0xFF;
    uint16_t lastindex = (rt >> 8) & 0xFFFF;

    if (rs == 0) {
        result = lastindex;
    } else {
        result = rs - decr;
    }

    return result;
}

void helper_incPosBy32(void)
{
    if (DSPCONTROL_pos() <= 32) {
        set_DSPCONTROL_pos(DSPCONTROL_pos() + 32);
    }
}


target_ulong helper_muleq_s_w_phl(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t result = dsp_multiplyQ15Q15_noacc(a1.temp16[DSP_PAIR_HI],
                                               a2.temp16[DSP_PAIR_HI]);

    return result;

}

target_ulong helper_muleq_s_w_phr(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;

    a1.temp = arg1;
    a2.temp = arg2;

    uint32_t result = dsp_multiplyQ15Q15_noacc(a1.temp16[DSP_PAIR_LO],
                                               a2.temp16[DSP_PAIR_LO]);

    return result;
}

static uint16_t dsp_multiplyU8U16(uint8_t a, uint16_t b)
{
    uint32_t result = a * b;

    if ((result >> 16) > 0) {
        result = 0xFFFF;
        set_DSPCONTROL_21(1);
    }

    return result;
}

target_ulong helper_muleu_s_ph_qbl(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[1] =
        dsp_multiplyU8U16(a1.temp8[DSP_QUAD_HI], a2.temp16[DSP_PAIR_HI]);

    result.temp16[0] =
        dsp_multiplyU8U16(a1.temp8[DSP_QUAD_HIMID], a2.temp16[DSP_PAIR_LO]);

    return result.temp;
}

target_ulong helper_muleu_s_ph_qbr(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[1] =
        dsp_multiplyU8U16(a1.temp8[DSP_QUAD_LOMID], a2.temp16[DSP_PAIR_HI]);

    result.temp16[0] =
        dsp_multiplyU8U16(a1.temp8[DSP_QUAD_LO], a2.temp16[DSP_PAIR_LO]);

    return result.temp;
}

static inline uint16_t dsp_rndQ15MuliplyQ15Q15(uint16_t a, uint16_t b)
{
    uint32_t result;

    if ((a == 0x8000) && (b == 0x8000)) {
        result = 0x7FFF0000;
        set_DSPCONTROL_21(1);
    } else {
        result = (((int32_t)(int16_t)a * (int32_t)(int16_t) b) << 1);
        result = result + 0x00008000;
    }

    return result >> 16;
}

target_ulong helper_mulq_rs_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[DSP_PAIR_HI] =
        dsp_rndQ15MuliplyQ15Q15(a1.temp16[DSP_PAIR_HI],
                                a2.temp16[DSP_PAIR_HI]);
    result.temp16[DSP_PAIR_LO] =
        dsp_rndQ15MuliplyQ15Q15(a1.temp16[DSP_PAIR_LO],
                                a2.temp16[DSP_PAIR_LO]);

    return result.temp;
}

target_ulong helper_mulq_rs_w(target_ulong arg1, target_ulong arg2)
{
    uint64_t result;

    if ((arg1 == 0x80000000) && (arg2 == 0x80000000)) {
        result = (uint64_t)0x7FFFFFFF << 32;
        set_DSPCONTROL_21(1);
    } else {
        result = (((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2) << 1);
        result = result + 0x80000000;
    }

    return result >> 32;
}

target_ulong helper_mulq_s_w(target_ulong arg1, target_ulong arg2)
{
    uint64_t result;

    if ((arg1 == 0x80000000) && (arg2 == 0x80000000)) {
        result = (uint64_t)0x7FFFFFFF << 32;
        set_DSPCONTROL_21(1);
    } else {
        result = (((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2) << 1);
    }

    return result >> 32;
}

uint64_t helper_mulsa_w_ph(uint64_t acc , target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = a1.tempi16[DSP_PAIR_HI] * a2.tempi16[DSP_PAIR_HI];
    int32_t tempA = a1.tempi16[DSP_PAIR_LO] * a2.tempi16[DSP_PAIR_LO];

    int64_t dotp = (int64_t)tempB - (int64_t)tempA;

    result = acc + (uint64_t)dotp;

    return result;
}

uint64_t helper_mulsaq_s_w_ph(uint32_t acc, uint64_t acc_value ,
    target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2;
    uint64_t result;

    a1.temp = arg1;
    a2.temp = arg2;

    int32_t tempB = dsp_multiplyQ15Q15(acc, a1.tempi16[DSP_PAIR_HI],
                                            a2.tempi16[DSP_PAIR_HI]);
    int32_t tempA = dsp_multiplyQ15Q15(acc, a1.tempi16[DSP_PAIR_LO],
                                            a2.tempi16[DSP_PAIR_LO]);

    int64_t dotp = (int64_t)tempB - (int64_t)tempA;

    result = acc_value + (uint64_t)dotp;

    return result;
}

target_ulong helper_packrl_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[DSP_PAIR_HI] = a1.temp16[DSP_PAIR_LO];
    result.temp16[DSP_PAIR_LO] = a2.temp16[DSP_PAIR_HI];

    return result.temp;
}

target_ulong helper_pick_qb(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] =
        DSPCONTROL_27() ? a1.temp8[DSP_QUAD_HI] : a2.temp8[DSP_QUAD_HI];

    result.temp8[DSP_QUAD_HIMID] =
        DSPCONTROL_26() ? a1.temp8[DSP_QUAD_HIMID] : a2.temp8[DSP_QUAD_HIMID];

    result.temp8[DSP_QUAD_LOMID] =
        DSPCONTROL_25() ? a1.temp8[DSP_QUAD_LOMID] : a2.temp8[DSP_QUAD_LOMID];

    result.temp8[DSP_QUAD_LO] =
        DSPCONTROL_24() ? a1.temp8[DSP_QUAD_LO] : a2.temp8[DSP_QUAD_LO];

    return result.temp;
}

target_ulong helper_pick_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[DSP_PAIR_HI] =
        DSPCONTROL_25() ? a1.temp16[DSP_PAIR_HI] : a2.temp16[DSP_PAIR_HI];

    result.temp16[DSP_PAIR_LO] =
        DSPCONTROL_24() ? a1.temp16[DSP_PAIR_LO] : a2.temp16[DSP_PAIR_LO];

    return result.temp;
}

target_ulong helper_preceq_w_phr(target_ulong arg)
{
    union target_split a; uint32_t result;

    a.temp = arg;
    result = a.temp16[DSP_PAIR_LO] << 16;

    return result;
}

target_ulong helper_preceq_w_phl(target_ulong arg)
{
    union target_split a; uint32_t result;

    a.temp = arg;
    result = a.temp16[DSP_PAIR_HI] << 16;

    return result;
}

target_ulong helper_precequ_ph_qbl(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HI] << 7;
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_HIMID] << 7;

    return result.temp;
}

target_ulong helper_preceu_ph_qbl(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HI];
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_HIMID];

    return result.temp;
}

target_ulong helper_preceu_ph_qbr(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_LOMID];
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LO];

    return result.temp;
}

target_ulong helper_preceu_ph_qbla(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HI];
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LOMID];

    return result.temp;
}

target_ulong helper_preceu_ph_qbra(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HIMID];
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LO];

    return result.temp;
}


target_ulong helper_precequ_ph_qbr(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_LOMID] << 7;
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LO] << 7;

    return result.temp;
}

target_ulong helper_precequ_ph_qbla(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HI] << 7;
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LOMID] << 7;

    return result.temp;
}

target_ulong helper_precequ_ph_qbra(target_ulong arg)
{
    union target_split a, result;

    a.temp = arg;
    result.temp16[DSP_PAIR_HI] = a.temp8[DSP_QUAD_HIMID] << 7;
    result.temp16[DSP_PAIR_LO] = a.temp8[DSP_QUAD_LO] << 7;

    return result.temp;
}

target_ulong helper_precr_qb_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_HIMID];
    result.temp8[DSP_QUAD_HIMID] = a1.temp8[DSP_QUAD_LO];
    result.temp8[DSP_QUAD_LOMID] = a2.temp8[DSP_QUAD_HIMID];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_LO];

    return result.temp;
}

target_ulong helper_precr_sra_ph_w(target_ulong arg1, target_ulong arg2,
    uint32_t sa)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;
    sa = sa & 0x1f;

    if (sa == 0) {
        result.temp16[DSP_PAIR_HI] = a2.temp16[DSP_PAIR_LO];
        result.temp16[DSP_PAIR_LO] = a1.temp16[DSP_PAIR_LO];
    } else {
        result.tempi16[DSP_PAIR_HI] = (a2.tempi >> sa);
        result.tempi16[DSP_PAIR_LO] = (a1.tempi >> sa);
    }

    return result.temp;
}

target_ulong helper_precr_sra_r_ph_w(target_ulong arg1, target_ulong arg2,
    uint32_t sa)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;
    sa = sa & 0x1f;

    if (sa == 0) {
        result.temp16[DSP_PAIR_HI] = a2.temp16[DSP_PAIR_LO];
        result.temp16[DSP_PAIR_LO] = a1.temp16[DSP_PAIR_LO];
    } else {
        result.tempi16[DSP_PAIR_HI] = (((a2.tempi >> (sa - 1)) + 1) >> 1);
        result.tempi16[DSP_PAIR_LO] = (((a1.tempi >> (sa - 1)) + 1) >> 1);
    }

    return result.temp;
}

target_ulong helper_precrq_qb_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_HIMID] = a1.temp8[DSP_QUAD_LOMID];
    result.temp8[DSP_QUAD_LOMID] = a2.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_LOMID];

    return result.temp;
}

target_ulong helper_precrq_ph_w(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[DSP_PAIR_HI] = a1.temp16[DSP_PAIR_HI];
    result.temp16[DSP_PAIR_LO] = a2.temp16[DSP_PAIR_HI];

    return result.temp;
}

static inline uint16_t dsp_trunc16SatRound(uint32_t a)
{
    int sign = a >> 31;
    uint64_t temp = (((uint64_t)sign << 32) | (uint64_t)a) + 0x8000;

    if (((temp >> 32) & 1) != ((temp >> 31) & 1)) {
        temp = 0x7FFFFFFF;
        set_DSPCONTROL_22(1);
    }

    return (temp >> 16) & 0xFFFF;
}

target_ulong helper_precrq_rs_ph_w(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp16[DSP_PAIR_HI] = dsp_trunc16SatRound(a1.temp);
    result.temp16[DSP_PAIR_LO] = dsp_trunc16SatRound(a2.temp);

    return result.temp;
}

static inline uint8_t dsp_sat8ReducePrecision(uint16_t a)
{
    uint8_t sign = a >> 15;
    uint16_t mag = a & 0x7FFF;
    uint8_t result;

    if (sign == 0) {
        if (mag > 0x7F80) {
            result = 0xFF;
            set_DSPCONTROL_22(1);
        } else {
            result = mag >> 7;
        }
    } else {
        result = 0;
        set_DSPCONTROL_22(1);
    }

    return result;
}

target_ulong helper_precrqu_s_qb_ph(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI]
        = dsp_sat8ReducePrecision(a1.temp16[DSP_PAIR_HI]);

    result.temp8[DSP_QUAD_HIMID]
        = dsp_sat8ReducePrecision(a1.temp16[DSP_PAIR_LO]);

    result.temp8[DSP_QUAD_LOMID]
        = dsp_sat8ReducePrecision(a2.temp16[DSP_PAIR_HI]);

    result.temp8[DSP_QUAD_LO]
        = dsp_sat8ReducePrecision(a2.temp16[DSP_PAIR_LO]);

    return result.temp;
}

/****************************************************
 * DSP ASE R3 */

target_ulong helper_ilvev_qb(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_HIMID];
    result.temp8[DSP_QUAD_HIMID] = a2.temp8[DSP_QUAD_HIMID];
    result.temp8[DSP_QUAD_LOMID] = a1.temp8[DSP_QUAD_LO];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_LO];

    return result.temp;
}

target_ulong helper_ilvod_qb(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_HIMID] = a2.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_LOMID] = a1.temp8[DSP_QUAD_LOMID];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_LOMID];

    return result.temp;
}

target_ulong helper_ilvl_qb(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_HIMID] = a2.temp8[DSP_QUAD_HI];
    result.temp8[DSP_QUAD_LOMID] = a1.temp8[DSP_QUAD_HIMID];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_HIMID];

    return result.temp;
}

target_ulong helper_ilvr_qb(target_ulong arg1, target_ulong arg2)
{
    union target_split a1, a2, result;

    a1.temp = arg1;
    a2.temp = arg2;

    result.temp8[DSP_QUAD_HI] = a1.temp8[DSP_QUAD_LOMID];
    result.temp8[DSP_QUAD_HIMID] = a2.temp8[DSP_QUAD_LOMID];
    result.temp8[DSP_QUAD_LOMID] = a1.temp8[DSP_QUAD_LO];
    result.temp8[DSP_QUAD_LO] = a2.temp8[DSP_QUAD_LO];

    return result.temp;
}

/* DSP ASE R3
 ***************************************************/

target_ulong helper_prepend(target_ulong rs, target_ulong rt, uint32_t sa)
{
    uint32_t result;

    sa = sa & 0x1F;

    if (sa == 0) {
        result = rt;
    } else {
        result = (rt >> sa) | ((rs & mask32[sa]) << (32 - sa));
    }

    return result;
}

target_ulong helper_raddu_w_qb(target_ulong arg)
{
    uint32_t result;
    union target_split a;

    a.temp = arg;

    result = (uint16_t)a.temp8[DSP_QUAD_HI]
           + (uint16_t)a.temp8[DSP_QUAD_HIMID]
           + (uint16_t)a.temp8[DSP_QUAD_LOMID]
           + (uint16_t)a.temp8[DSP_QUAD_LO];

    return result;
}

target_ulong helper_repl_ph(uint32_t immed)
{
    union target_split result;
    uint8_t sign = (immed >> 9) & 1;
    int16_t val = immed & 0x1FF;

    if (sign) {
        val = val | 0xFE00;  /* sign extend to 16 bits */
    }

    result.tempi16[DSP_PAIR_LO] = val;
    result.tempi16[DSP_PAIR_HI] = val;

    return result.temp;
}

target_ulong helper_repl_qb(uint32_t immed)
{
    union target_split result;

    immed = immed & 0xFF;

    result.temp8[DSP_QUAD_LO] = immed;
    result.temp8[DSP_QUAD_LOMID] = immed;
    result.temp8[DSP_QUAD_HIMID] = immed;
    result.temp8[DSP_QUAD_HI] = immed;

    return result.temp;
}

target_ulong helper_replv_ph(target_ulong val)
{
    union target_split result;

    result.temp16[DSP_PAIR_LO] = val;
    result.temp16[DSP_PAIR_HI] = val;

    return result.temp;
}

target_ulong helper_replv_qb(target_ulong val)
{
    union target_split result;

    result.temp8[DSP_QUAD_LO] = val;
    result.temp8[DSP_QUAD_LOMID] = val;
    result.temp8[DSP_QUAD_HIMID] = val;
    result.temp8[DSP_QUAD_HI] = val;

    return result.temp;
}

uint64_t helper_shilo(uint64_t acc, uint32_t shift)
{
    uint64_t result;

    if (shift == 0) {
        return acc;
    }

    uint8_t sign = (shift >> 5) & 1;
    shift = shift & 0x3f;

    int8_t ishift = (sign == 0) ? shift : -((~shift + 1) & 0x3f);

    if (ishift > 0) {
        result = acc >> ishift;
    } else {
        result = acc << -ishift;
    }

    return result;
}

uint64_t helper_shilov(uint64_t acc, target_ulong shift)
{
    return helper_shilo(acc, shift);
}

static inline uint16_t dsp_shift16Left(uint16_t a, uint8_t s)
{
    s = s & 0xF;

    if (s == 0) {
        return a;
    }

    union target_split result, test;

    result.temp = (int32_t)(int16_t)a;
    result.temp = result.temp << s;
    test.temp = result.temp << 1;

    if ((test.temp16[DSP_PAIR_HI] != 0) &&
        (test.temp16[DSP_PAIR_HI] != 0xFFFF)) {
        set_DSPCONTROL_22(1);
    }

    return result.temp16[DSP_PAIR_LO];
}


target_ulong helper_shll_ph(target_ulong arg, uint32_t shift)
{
    union target_split result, a;

    a.temp = arg;

    result.temp16[DSP_PAIR_LO] = dsp_shift16Left(a.temp16[DSP_PAIR_LO], shift);
    result.temp16[DSP_PAIR_HI] = dsp_shift16Left(a.temp16[DSP_PAIR_HI], shift);

    return result.temp;
}

static inline uint16_t dsp_sat16ShiftLeft(uint16_t a, uint8_t s)
{
    s = s & 0xF;

    if (s == 0) {
        return a;
    }

    union target_split result, test;
    uint8_t sign = (a >> 15) & 1;

    result.temp = (int32_t)(int16_t)a;
    result.temp = result.temp << s;
    test.temp = result.temp << 1;

    if ((test.temp16[DSP_PAIR_HI] != 0) &&
        (test.temp16[DSP_PAIR_HI] != 0xFFFF)) {
        if (sign == 0) {
            result.temp16[DSP_PAIR_LO] = 0x7FFF;
        } else {
            result.temp16[DSP_PAIR_LO] = 0x8000;
        }

        set_DSPCONTROL_22(1);
    }

    return result.temp16[DSP_PAIR_LO];
}

target_ulong helper_shll_s_ph(target_ulong arg, uint32_t shift)
{
    union target_split result, a;

    a.temp = arg;

    result.temp16[DSP_PAIR_LO]
        = dsp_sat16ShiftLeft(a.temp16[DSP_PAIR_LO], shift);

    result.temp16[DSP_PAIR_HI]
        = dsp_sat16ShiftLeft(a.temp16[DSP_PAIR_HI], shift);

    return result.temp;
}

static inline uint8_t dsp_shift8Left(uint8_t a, uint8_t s)
{
    s = s & 0x7;

    if (s == 0) {
        return a;
    }

    int16_t res = (int16_t)(int8_t)a << s;

    if (res > BIT8_UMAX || res < 0) {
        set_DSPCONTROL_22(1);
    }

    return res & 0xff;
}

target_ulong helper_shll_qb(target_ulong arg, uint32_t shift)
{
    union target_split result, a;

    a.temp = arg;

    result.temp8[DSP_QUAD_LO]
        = dsp_shift8Left(a.temp8[DSP_QUAD_LO], shift);

    result.temp8[DSP_QUAD_LOMID]
        = dsp_shift8Left(a.temp8[DSP_QUAD_LOMID], shift);

    result.temp8[DSP_QUAD_HIMID]
        = dsp_shift8Left(a.temp8[DSP_QUAD_HIMID], shift);

    result.temp8[DSP_QUAD_HI]
        = dsp_shift8Left(a.temp8[DSP_QUAD_HI], shift);

    return result.temp;
}

typedef union uint64_split {
    uint64_t temp;
    uint32_t temp32[2];
} uint64_split;

static inline uint32_t dsp_sat32ShiftLeft(uint32_t a, uint8_t s)
{
    s = s & 0x1F;

    if (s == 0) {
        return a;
    }

    union uint64_split result, test;
    uint8_t sign = (a >> 31) & 1;

    result.temp = (int64_t)(int32_t)a;
    result.temp = result.temp << s;
    test.temp = result.temp << 1;

    if ((test.temp32[DSP_PAIR_HI] != 0) &&
        (test.temp32[DSP_PAIR_HI] != 0xFFFFFFFF)) {
        if (sign == 0) {
            result.temp32[DSP_PAIR_LO] = 0x7FFFFFFF;
        } else {
            result.temp32[DSP_PAIR_LO] = 0x80000000;
        }

        set_DSPCONTROL_22(1);
    }

    return result.temp32[DSP_PAIR_LO];
}

target_ulong helper_shll_s_w(target_ulong arg, uint32_t shift)
{
    return dsp_sat32ShiftLeft(arg, shift);
}

target_ulong helper_shllv_ph(target_ulong shift, target_ulong arg)
{
    return helper_shll_ph(arg, shift);
}

target_ulong helper_shllv_s_ph(target_ulong shift, target_ulong arg)
{
    return helper_shll_s_ph(arg, shift);
}

target_ulong helper_shllv_qb(target_ulong shift, target_ulong arg)
{
    return helper_shll_qb(arg, shift);
}

target_ulong helper_shllv_s_w(target_ulong shift, target_ulong arg)
{
    return helper_shll_s_w(arg, shift);
}

static inline uint16_t dsp_shift16RightArithmetic(uint16_t a, uint8_t s)
{
    s = s & 0xF;

    if (s == 0) {
        return a;
    }

    int32_t result = (int32_t)(int16_t)a >> s;

    return result;
}

target_ulong helper_shra_ph(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    a.temp = arg;

    result.temp16[DSP_PAIR_LO]
        = dsp_shift16RightArithmetic(a.temp16[DSP_PAIR_LO], shift);

    result.temp16[DSP_PAIR_HI]
        = dsp_shift16RightArithmetic(a.temp16[DSP_PAIR_HI], shift);

    return result.temp;
}

target_ulong helper_shrav_ph(target_ulong shift, target_ulong arg)
{
    return helper_shra_ph(arg, shift);
}

static inline uint16_t dsp_rnd16shiftRightArithmetic(uint16_t a, uint8_t s)
{
    s = s & 0xF;

    if (s == 0) {
        return a;
    }

    int32_t result = (int32_t)(int16_t)a >> (s - 1);
    result++;
    result = result >> 1;

    return result & 0xFFFF;
}

target_ulong helper_shra_r_ph(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    a.temp = arg;

    result.temp16[DSP_PAIR_LO]
        = dsp_rnd16shiftRightArithmetic(a.temp16[DSP_PAIR_LO], shift);

    result.temp16[DSP_PAIR_HI]
        = dsp_rnd16shiftRightArithmetic(a.temp16[DSP_PAIR_HI], shift);

    return result.temp;
}

target_ulong helper_shrav_r_ph(target_ulong shift, target_ulong arg)
{
    return helper_shra_r_ph(arg, shift);
}

static inline uint8_t dsp_shift8RightArithmetic(uint8_t a, uint8_t s)
{
    s = s & 0x7;

    if (s == 0) {
        return a;
    }

    int16_t result = (int16_t)(int8_t)a >> s;

    return result;
}

target_ulong helper_shra_qb(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    a.temp = arg;

    result.temp8[DSP_QUAD_LO]
        = dsp_shift8RightArithmetic(a.temp8[DSP_QUAD_LO], shift);

    result.temp8[DSP_QUAD_LOMID]
        = dsp_shift8RightArithmetic(a.temp8[DSP_QUAD_LOMID], shift);

    result.temp8[DSP_QUAD_HIMID]
        = dsp_shift8RightArithmetic(a.temp8[DSP_QUAD_HIMID], shift);

    result.temp8[DSP_QUAD_HI]
        = dsp_shift8RightArithmetic(a.temp8[DSP_QUAD_HI], shift);

    return result.temp;
}

target_ulong helper_shrav_qb(target_ulong shift, target_ulong arg)
{
    return helper_shra_qb(arg, shift);
}

static inline uint8_t dsp_rnd8shiftRightArithmetic(uint8_t a, uint8_t s)
{
    s = s & 0x7;

    if (s == 0) {
        return a;
    }

    int16_t result = (int16_t)(int8_t)a >> (s - 1);
    result++;
    result = result >> 1;

    return result;
}

target_ulong helper_shra_r_qb(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    a.temp = arg;

    result.temp8[DSP_QUAD_LO]
        = dsp_rnd8shiftRightArithmetic(a.temp8[DSP_QUAD_LO], shift);

    result.temp8[DSP_QUAD_LOMID]
        = dsp_rnd8shiftRightArithmetic(a.temp8[DSP_QUAD_LOMID], shift);

    result.temp8[DSP_QUAD_HIMID]
        = dsp_rnd8shiftRightArithmetic(a.temp8[DSP_QUAD_HIMID], shift);

    result.temp8[DSP_QUAD_HI]
        = dsp_rnd8shiftRightArithmetic(a.temp8[DSP_QUAD_HI], shift);

    return result.temp;
}

target_ulong helper_shrav_r_qb(target_ulong shift, target_ulong arg)
{
    return helper_shra_r_qb(arg, shift);
}

target_ulong helper_shra_r_w(target_ulong arg, uint32_t shift)
{
    shift = shift & 0x1F;
    int64_t result;

    if (shift == 0) {
        return arg;
    } else {
        result = (int64_t)(int32_t)arg >> (shift - 1);
        result++;
    }

    result = (result >> 1) & 0xFFFFFFFF;

    return result;
}

target_ulong helper_shrav_r_w(target_ulong shift, target_ulong arg)
{
    return helper_shra_r_w(arg, shift);
}

target_ulong helper_shrl_ph(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    shift = shift & 0xF;
    a.temp = arg;

    result.temp16[DSP_PAIR_LO] = a.temp16[DSP_PAIR_LO] >> shift;
    result.temp16[DSP_PAIR_HI] = a.temp16[DSP_PAIR_HI] >> shift;

    return result.temp;
}

target_ulong helper_shrlv_ph(target_ulong shift, target_ulong arg)
{
    return helper_shrl_ph(arg, shift);
}

target_ulong helper_shrl_qb(target_ulong arg, uint32_t shift)
{
    union target_split a, result;

    shift = shift & 0x7;
    a.temp = arg;

    result.temp8[DSP_QUAD_LO] = a.temp8[DSP_QUAD_LO] >> shift;
    result.temp8[DSP_QUAD_LOMID] = a.temp8[DSP_QUAD_LOMID] >> shift;
    result.temp8[DSP_QUAD_HIMID] = a.temp8[DSP_QUAD_HIMID] >> shift;
    result.temp8[DSP_QUAD_HI] = a.temp8[DSP_QUAD_HI] >> shift;

    return result.temp;
}

target_ulong helper_shrlv_qb(target_ulong shift, target_ulong arg)
{
    return helper_shrl_qb(arg, shift);
}

/* Multiplication variants of the vr54xx. */
target_ulong helper_muls (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, 0 - ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_mulsu (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, 0 - ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

target_ulong helper_macc (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, ((int64_t)get_HILO()) + ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_macchi (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, ((int64_t)get_HILO()) + ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_maccu (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, ((uint64_t)get_HILO()) + ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

target_ulong helper_macchiu (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, ((uint64_t)get_HILO()) + ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

target_ulong helper_msac (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, ((int64_t)get_HILO()) - ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_msachi (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, ((int64_t)get_HILO()) - ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_msacu (target_ulong arg1, target_ulong arg2)
{
    set_HI_LOT0(arg1, ((uint64_t)get_HILO()) - ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

target_ulong helper_msachiu (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, ((uint64_t)get_HILO()) - ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

target_ulong helper_mulhi (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, (int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2);

    return arg1;
}

target_ulong helper_mulhiu (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, (uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2);

    return arg1;
}

target_ulong helper_mulshi (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, 0 - ((int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2));

    return arg1;
}

target_ulong helper_mulshiu (target_ulong arg1, target_ulong arg2)
{
    set_HIT0_LO(arg1, 0 - ((uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2));

    return arg1;
}

#ifdef TARGET_MIPS64
void helper_dmult (target_ulong arg1, target_ulong arg2)
{
    muls64(&(env->active_tc.LO[0]), &(env->active_tc.HI[0]), arg1, arg2);
}

void helper_dmultu (target_ulong arg1, target_ulong arg2)
{
    mulu64(&(env->active_tc.LO[0]), &(env->active_tc.HI[0]), arg1, arg2);
}
#endif

#ifndef CONFIG_USER_ONLY

static inline target_phys_addr_t do_translate_address(target_ulong address, int rw)
{
    target_phys_addr_t lladdr;

    lladdr = cpu_mips_translate_address(env, address, rw);

    if (lladdr == -1LL) {
        cpu_loop_exit(env);
    } else {
        return lladdr;
    }
}

#define HELPER_LD_ATOMIC(name, insn)                                          \
target_ulong helper_##name(target_ulong arg, int mem_idx)                     \
{                                                                             \
    env->lladdr = do_translate_address(arg, 0);                               \
    env->llval = do_##insn(arg, mem_idx);                                     \
    return env->llval;                                                        \
}
HELPER_LD_ATOMIC(ll, lw)
#ifdef TARGET_MIPS64
HELPER_LD_ATOMIC(lld, ld)
#endif
#undef HELPER_LD_ATOMIC

#define HELPER_ST_ATOMIC(name, ld_insn, st_insn, almask)                      \
target_ulong helper_##name(target_ulong arg1, target_ulong arg2, int mem_idx) \
{                                                                             \
    target_long tmp;                                                          \
                                                                              \
    if (arg2 & almask) {                                                      \
        env->CP0_BadVAddr = arg2;                                             \
        helper_raise_exception(EXCP_AdES);                                    \
    }                                                                         \
    if (do_translate_address(arg2, 1) == env->lladdr) {                       \
        tmp = do_##ld_insn(arg2, mem_idx);                                    \
        if (tmp == env->llval) {                                              \
            do_##st_insn(arg2, arg1, mem_idx);                                \
            return 1;                                                         \
        }                                                                     \
    }                                                                         \
    return 0;                                                                 \
}
HELPER_ST_ATOMIC(sc, lw, sw, 0x3)
#ifdef TARGET_MIPS64
HELPER_ST_ATOMIC(scd, ld, sd, 0x7)
#endif
#undef HELPER_ST_ATOMIC
#endif

#ifdef TARGET_WORDS_BIGENDIAN
#define GET_LMASK(v) ((v) & 3)
#define GET_OFFSET(addr, offset) (addr + (offset))
#else
#define GET_LMASK(v) (((v) & 3) ^ 3)
#define GET_OFFSET(addr, offset) (addr - (offset))
#endif

target_ulong helper_lwl(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    target_ulong tmp;

    tmp = do_lbu(arg2, mem_idx);
    arg1 = (arg1 & 0x00FFFFFF) | (tmp << 24);

    if (GET_LMASK(arg2) <= 2) {
        tmp = do_lbu(GET_OFFSET(arg2, 1), mem_idx);
        arg1 = (arg1 & 0xFF00FFFF) | (tmp << 16);
    }

    if (GET_LMASK(arg2) <= 1) {
        tmp = do_lbu(GET_OFFSET(arg2, 2), mem_idx);
        arg1 = (arg1 & 0xFFFF00FF) | (tmp << 8);
    }

    if (GET_LMASK(arg2) == 0) {
        tmp = do_lbu(GET_OFFSET(arg2, 3), mem_idx);
        arg1 = (arg1 & 0xFFFFFF00) | tmp;
    }
    return (int32_t)arg1;
}

target_ulong helper_lwr(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    target_ulong tmp;

    tmp = do_lbu(arg2, mem_idx);
    arg1 = (arg1 & 0xFFFFFF00) | tmp;

    if (GET_LMASK(arg2) >= 1) {
        tmp = do_lbu(GET_OFFSET(arg2, -1), mem_idx);
        arg1 = (arg1 & 0xFFFF00FF) | (tmp << 8);
    }

    if (GET_LMASK(arg2) >= 2) {
        tmp = do_lbu(GET_OFFSET(arg2, -2), mem_idx);
        arg1 = (arg1 & 0xFF00FFFF) | (tmp << 16);
    }

    if (GET_LMASK(arg2) == 3) {
        tmp = do_lbu(GET_OFFSET(arg2, -3), mem_idx);
        arg1 = (arg1 & 0x00FFFFFF) | (tmp << 24);
    }
    return (int32_t)arg1;
}

void helper_swl(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    do_sb(arg2, (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK(arg2) <= 2)
        do_sb(GET_OFFSET(arg2, 1), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK(arg2) <= 1)
        do_sb(GET_OFFSET(arg2, 2), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK(arg2) == 0)
        do_sb(GET_OFFSET(arg2, 3), (uint8_t)arg1, mem_idx);
}

void helper_swr(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    do_sb(arg2, (uint8_t)arg1, mem_idx);

    if (GET_LMASK(arg2) >= 1)
        do_sb(GET_OFFSET(arg2, -1), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK(arg2) >= 2)
        do_sb(GET_OFFSET(arg2, -2), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK(arg2) == 3)
        do_sb(GET_OFFSET(arg2, -3), (uint8_t)(arg1 >> 24), mem_idx);
}

#if defined(TARGET_MIPS64)
/* "half" load and stores.  We must do the memory access inline,
   or fault handling won't work.  */

#ifdef TARGET_WORDS_BIGENDIAN
#define GET_LMASK64(v) ((v) & 7)
#else
#define GET_LMASK64(v) (((v) & 7) ^ 7)
#endif

target_ulong helper_ldl(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    uint64_t tmp;

    tmp = do_lbu(arg2, mem_idx);
    arg1 = (arg1 & 0x00FFFFFFFFFFFFFFULL) | (tmp << 56);

    if (GET_LMASK64(arg2) <= 6) {
        tmp = do_lbu(GET_OFFSET(arg2, 1), mem_idx);
        arg1 = (arg1 & 0xFF00FFFFFFFFFFFFULL) | (tmp << 48);
    }

    if (GET_LMASK64(arg2) <= 5) {
        tmp = do_lbu(GET_OFFSET(arg2, 2), mem_idx);
        arg1 = (arg1 & 0xFFFF00FFFFFFFFFFULL) | (tmp << 40);
    }

    if (GET_LMASK64(arg2) <= 4) {
        tmp = do_lbu(GET_OFFSET(arg2, 3), mem_idx);
        arg1 = (arg1 & 0xFFFFFF00FFFFFFFFULL) | (tmp << 32);
    }

    if (GET_LMASK64(arg2) <= 3) {
        tmp = do_lbu(GET_OFFSET(arg2, 4), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFF00FFFFFFULL) | (tmp << 24);
    }

    if (GET_LMASK64(arg2) <= 2) {
        tmp = do_lbu(GET_OFFSET(arg2, 5), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFFFF00FFFFULL) | (tmp << 16);
    }

    if (GET_LMASK64(arg2) <= 1) {
        tmp = do_lbu(GET_OFFSET(arg2, 6), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFFFFFF00FFULL) | (tmp << 8);
    }

    if (GET_LMASK64(arg2) == 0) {
        tmp = do_lbu(GET_OFFSET(arg2, 7), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFFFFFFFF00ULL) | tmp;
    }

    return arg1;
}

target_ulong helper_ldr(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    uint64_t tmp;

    tmp = do_lbu(arg2, mem_idx);
    arg1 = (arg1 & 0xFFFFFFFFFFFFFF00ULL) | tmp;

    if (GET_LMASK64(arg2) >= 1) {
        tmp = do_lbu(GET_OFFSET(arg2, -1), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFFFFFF00FFULL) | (tmp  << 8);
    }

    if (GET_LMASK64(arg2) >= 2) {
        tmp = do_lbu(GET_OFFSET(arg2, -2), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFFFF00FFFFULL) | (tmp << 16);
    }

    if (GET_LMASK64(arg2) >= 3) {
        tmp = do_lbu(GET_OFFSET(arg2, -3), mem_idx);
        arg1 = (arg1 & 0xFFFFFFFF00FFFFFFULL) | (tmp << 24);
    }

    if (GET_LMASK64(arg2) >= 4) {
        tmp = do_lbu(GET_OFFSET(arg2, -4), mem_idx);
        arg1 = (arg1 & 0xFFFFFF00FFFFFFFFULL) | (tmp << 32);
    }

    if (GET_LMASK64(arg2) >= 5) {
        tmp = do_lbu(GET_OFFSET(arg2, -5), mem_idx);
        arg1 = (arg1 & 0xFFFF00FFFFFFFFFFULL) | (tmp << 40);
    }

    if (GET_LMASK64(arg2) >= 6) {
        tmp = do_lbu(GET_OFFSET(arg2, -6), mem_idx);
        arg1 = (arg1 & 0xFF00FFFFFFFFFFFFULL) | (tmp << 48);
    }

    if (GET_LMASK64(arg2) == 7) {
        tmp = do_lbu(GET_OFFSET(arg2, -7), mem_idx);
        arg1 = (arg1 & 0x00FFFFFFFFFFFFFFULL) | (tmp << 56);
    }

    return arg1;
}

void helper_sdl(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    do_sb(arg2, (uint8_t)(arg1 >> 56), mem_idx);

    if (GET_LMASK64(arg2) <= 6)
        do_sb(GET_OFFSET(arg2, 1), (uint8_t)(arg1 >> 48), mem_idx);

    if (GET_LMASK64(arg2) <= 5)
        do_sb(GET_OFFSET(arg2, 2), (uint8_t)(arg1 >> 40), mem_idx);

    if (GET_LMASK64(arg2) <= 4)
        do_sb(GET_OFFSET(arg2, 3), (uint8_t)(arg1 >> 32), mem_idx);

    if (GET_LMASK64(arg2) <= 3)
        do_sb(GET_OFFSET(arg2, 4), (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK64(arg2) <= 2)
        do_sb(GET_OFFSET(arg2, 5), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK64(arg2) <= 1)
        do_sb(GET_OFFSET(arg2, 6), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK64(arg2) <= 0)
        do_sb(GET_OFFSET(arg2, 7), (uint8_t)arg1, mem_idx);
}

void helper_sdr(target_ulong arg1, target_ulong arg2, int mem_idx)
{
    do_sb(arg2, (uint8_t)arg1, mem_idx);

    if (GET_LMASK64(arg2) >= 1)
        do_sb(GET_OFFSET(arg2, -1), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK64(arg2) >= 2)
        do_sb(GET_OFFSET(arg2, -2), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK64(arg2) >= 3)
        do_sb(GET_OFFSET(arg2, -3), (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK64(arg2) >= 4)
        do_sb(GET_OFFSET(arg2, -4), (uint8_t)(arg1 >> 32), mem_idx);

    if (GET_LMASK64(arg2) >= 5)
        do_sb(GET_OFFSET(arg2, -5), (uint8_t)(arg1 >> 40), mem_idx);

    if (GET_LMASK64(arg2) >= 6)
        do_sb(GET_OFFSET(arg2, -6), (uint8_t)(arg1 >> 48), mem_idx);

    if (GET_LMASK64(arg2) == 7)
        do_sb(GET_OFFSET(arg2, -7), (uint8_t)(arg1 >> 56), mem_idx);
}
#endif /* TARGET_MIPS64 */

static const int multiple_regs[] = { 16, 17, 18, 19, 20, 21, 22, 23, 30 };

void helper_lwm (target_ulong addr, target_ulong reglist, uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;
#ifdef CONFIG_USER_ONLY
#undef ldfun
#define ldfun ldl_raw
#else
    uint32_t (*ldfun)(target_ulong);

    switch (mem_idx)
    {
    case 0: ldfun = ldl_kernel; break;
    case 1: ldfun = ldl_super; break;
    default:
    case 2: ldfun = ldl_user; break;
    }
#endif

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            env->active_tc.gpr[multiple_regs[i]] = (target_long) ldfun(addr);
            addr += 4;
        }
    }

    if (do_r31) {
        env->active_tc.gpr[31] = (target_long) ldfun(addr);
    }
}

void helper_swm (target_ulong addr, target_ulong reglist, uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;
#ifdef CONFIG_USER_ONLY
#undef stfun
#define stfun stl_raw
#else
    void (*stfun)(target_ulong, uint32_t);

    switch (mem_idx)
    {
    case 0: stfun = stl_kernel; break;
    case 1: stfun = stl_super; break;
     default:
    case 2: stfun = stl_user; break;
    }
#endif

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            stfun(addr, env->active_tc.gpr[multiple_regs[i]]);
            addr += 4;
        }
    }

    if (do_r31) {
        stfun(addr, env->active_tc.gpr[31]);
    }
}

#if defined(TARGET_MIPS64)
void helper_ldm (target_ulong addr, target_ulong reglist, uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;
#ifdef CONFIG_USER_ONLY
#undef ldfun
#define ldfun ldq_raw
#else
    uint64_t (*ldfun)(target_ulong);

    switch (mem_idx)
    {
    case 0: ldfun = ldq_kernel; break;
    case 1: ldfun = ldq_super; break;
    default:
    case 2: ldfun = ldq_user; break;
    }
#endif

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            env->active_tc.gpr[multiple_regs[i]] = ldfun(addr);
            addr += 8;
        }
    }

    if (do_r31) {
        env->active_tc.gpr[31] = ldfun(addr);
    }
}

void helper_sdm (target_ulong addr, target_ulong reglist, uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;
#ifdef CONFIG_USER_ONLY
#undef stfun
#define stfun stq_raw
#else
    void (*stfun)(target_ulong, uint64_t);

    switch (mem_idx)
    {
    case 0: stfun = stq_kernel; break;
    case 1: stfun = stq_super; break;
     default:
    case 2: stfun = stq_user; break;
    }
#endif

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            stfun(addr, env->active_tc.gpr[multiple_regs[i]]);
            addr += 8;
        }
    }

    if (do_r31) {
        stfun(addr, env->active_tc.gpr[31]);
    }
}
#endif

#ifndef CONFIG_USER_ONLY
/* SMP helpers.  */
static int mips_vpe_is_wfi(CPUState *c)
{
    /* If the VPE is halted but otherwise active, it means it's waiting for
       an interrupt.  */
    return c->halted && mips_vpe_active(c);
}

static inline void mips_vpe_wake(CPUState *c)
{
    /* Dont set ->halted = 0 directly, let it be done via cpu_has_work
       because there might be other conditions that state that c should
       be sleeping.  */
    cpu_interrupt(c, CPU_INTERRUPT_WAKE);
}

static inline void mips_vpe_sleep(CPUState *c)
{
    /* The VPE was shut off, really go to bed.
       Reset any old _WAKE requests.  */
    c->halted = 1;
    cpu_reset_interrupt(c, CPU_INTERRUPT_WAKE);
}

static inline void mips_tc_wake(CPUState *c, int tc)
{
    /* FIXME: TC reschedule.  */
    if (mips_vpe_active(c) && !mips_vpe_is_wfi(c)) {
        mips_vpe_wake(c);
    }
}

static inline void mips_tc_sleep(CPUState *c, int tc)
{
    /* FIXME: TC reschedule.  */
    if (!mips_vpe_active(c)) {
        mips_vpe_sleep(c);
    }
}

/* tc should point to an int with the value of the global TC index.
   This function will transform it into a local index within the
   returned CPUState.

   FIXME: This code assumes that all VPEs have the same number of TCs,
          which depends on runtime setup. Can probably be fixed by
          walking the list of CPUStates.  */
static CPUState *mips_cpu_map_tc(int *tc)
{
    CPUState *other;
    int vpe_idx, nr_threads = env->nr_threads;
    int tc_idx = *tc;

    if (!(env->CP0_VPEConf0 & (1 << CP0VPEC0_MVP))) {
        /* Not allowed to address other CPUs.  */
        *tc = env->current_tc;
        return env;
    }

    vpe_idx = tc_idx / nr_threads;
    *tc = tc_idx % nr_threads;
    other = qemu_get_cpu(vpe_idx);
    return other ? other : env;
}

/* The per VPE CP0_Status register shares some fields with the per TC
   CP0_TCStatus registers. These fields are wired to the same registers,
   so changes to either of them should be reflected on both registers.

   Also, EntryHi shares the bottom 8 bit ASID with TCStauts.

   These helper call synchronizes the regs for a given cpu.  */

/* Called for updates to CP0_Status.  */
static void sync_c0_status(CPUState *cpu, int tc)
{
    int32_t tcstatus, *tcst;
    uint32_t v = cpu->CP0_Status;
    uint32_t cu, mx, asid, ksu;
    uint32_t mask = ((1 << CP0TCSt_TCU3)
                       | (1 << CP0TCSt_TCU2)
                       | (1 << CP0TCSt_TCU1)
                       | (1 << CP0TCSt_TCU0)
                       | (1 << CP0TCSt_TMX)
                       | (3 << CP0TCSt_TKSU)
                       | (0xff << CP0TCSt_TASID));

    cu = (v >> CP0St_CU0) & 0xf;
    mx = (v >> CP0St_MX) & 0x1;
    ksu = (v >> CP0St_KSU) & 0x3;
    asid = env->CP0_EntryHi & 0xff;

    tcstatus = cu << CP0TCSt_TCU0;
    tcstatus |= mx << CP0TCSt_TMX;
    tcstatus |= ksu << CP0TCSt_TKSU;
    tcstatus |= asid;

    if (tc == cpu->current_tc) {
        tcst = &cpu->active_tc.CP0_TCStatus;
    } else {
        tcst = &cpu->tcs[tc].CP0_TCStatus;
    }

    *tcst &= ~mask;
    *tcst |= tcstatus;
    compute_hflags(cpu);
}

/* Called for updates to CP0_TCStatus.  */
static void sync_c0_tcstatus(CPUState *cpu, int tc, target_ulong v)
{
    uint32_t status;
    uint32_t tcu, tmx, tasid, tksu;
    uint32_t mask = ((1 << CP0St_CU3)
                       | (1 << CP0St_CU2)
                       | (1 << CP0St_CU1)
                       | (1 << CP0St_CU0)
                       | (1 << CP0St_MX)
                       | (3 << CP0St_KSU));

    tcu = (v >> CP0TCSt_TCU0) & 0xf;
    tmx = (v >> CP0TCSt_TMX) & 0x1;
    tasid = v & 0xff;
    tksu = (v >> CP0TCSt_TKSU) & 0x3;

    status = tcu << CP0St_CU0;
    status |= tmx << CP0St_MX;
    status |= tksu << CP0St_KSU;

    cpu->CP0_Status &= ~mask;
    cpu->CP0_Status |= status;

    /* Sync the TASID with EntryHi.  */
    cpu->CP0_EntryHi &= ~0xff;
    cpu->CP0_EntryHi = tasid;

    compute_hflags(cpu);
}

/* Called for updates to CP0_EntryHi.  */
static void sync_c0_entryhi(CPUState *cpu, int tc)
{
    int32_t *tcst;
    uint32_t asid, v = cpu->CP0_EntryHi;

    asid = v & 0xff;

    if (tc == cpu->current_tc) {
        tcst = &cpu->active_tc.CP0_TCStatus;
    } else {
        tcst = &cpu->tcs[tc].CP0_TCStatus;
    }

    *tcst &= ~0xff;
    *tcst |= asid;
}

/* CP0 helpers */
target_ulong helper_mfc0_mvpcontrol (void)
{
    return env->mvp->CP0_MVPControl;
}

target_ulong helper_mfc0_mvpconf0 (void)
{
    return env->mvp->CP0_MVPConf0;
}

target_ulong helper_mfc0_mvpconf1 (void)
{
    return env->mvp->CP0_MVPConf1;
}

target_ulong helper_mfc0_random (void)
{
    return (int32_t)cpu_mips_get_random(env);
}

target_ulong helper_mfc0_tcstatus (void)
{
    return env->active_tc.CP0_TCStatus;
}

target_ulong helper_mftc0_tcstatus(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCStatus;
    else
        return other->tcs[other_tc].CP0_TCStatus;
}

target_ulong helper_mfc0_tcbind (void)
{
    return env->active_tc.CP0_TCBind;
}

target_ulong helper_mftc0_tcbind(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCBind;
    else
        return other->tcs[other_tc].CP0_TCBind;
}

target_ulong helper_mfc0_tcrestart (void)
{
    return env->active_tc.PC;
}

target_ulong helper_mftc0_tcrestart(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.PC;
    else
        return other->tcs[other_tc].PC;
}

target_ulong helper_mfc0_tchalt (void)
{
    return env->active_tc.CP0_TCHalt;
}

target_ulong helper_mftc0_tchalt(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCHalt;
    else
        return other->tcs[other_tc].CP0_TCHalt;
}

target_ulong helper_mfc0_tccontext (void)
{
    return env->active_tc.CP0_TCContext;
}

target_ulong helper_mftc0_tccontext(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCContext;
    else
        return other->tcs[other_tc].CP0_TCContext;
}

target_ulong helper_mfc0_tcschedule (void)
{
    return env->active_tc.CP0_TCSchedule;
}

target_ulong helper_mftc0_tcschedule(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCSchedule;
    else
        return other->tcs[other_tc].CP0_TCSchedule;
}

target_ulong helper_mfc0_tcschefback (void)
{
    return env->active_tc.CP0_TCScheFBack;
}

target_ulong helper_mftc0_tcschefback(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCScheFBack;
    else
        return other->tcs[other_tc].CP0_TCScheFBack;
}

target_ulong helper_mfc0_count (void)
{
    return (int32_t)cpu_mips_get_count(env);
}

target_ulong helper_mftc0_entryhi(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    return other->CP0_EntryHi;
}

target_ulong helper_mftc0_cause(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    int32_t tccause;
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc) {
        tccause = other->CP0_Cause;
    } else {
        tccause = other->CP0_Cause;
    }

    return tccause;
}

target_ulong helper_mftc0_status(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    return other->CP0_Status;
}

target_ulong helper_mfc0_lladdr (void)
{
    return (int32_t)(env->lladdr >> env->CP0_LLAddr_shift);
}

target_ulong helper_mfc0_watchlo (uint32_t sel)
{
    return (int32_t)env->CP0_WatchLo[sel];
}

target_ulong helper_mfc0_watchhi (uint32_t sel)
{
    return env->CP0_WatchHi[sel];
}

target_ulong helper_mfc0_debug (void)
{
    target_ulong t0 = env->CP0_Debug;
    if (env->hflags & MIPS_HFLAG_DM)
        t0 |= 1 << CP0DB_DM;

    return t0;
}

target_ulong helper_mftc0_debug(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    int32_t tcstatus;
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        tcstatus = other->active_tc.CP0_Debug_tcstatus;
    else
        tcstatus = other->tcs[other_tc].CP0_Debug_tcstatus;

    /* XXX: Might be wrong, check with EJTAG spec. */
    return (other->CP0_Debug & ~((1 << CP0DB_SSt) | (1 << CP0DB_Halt))) |
            (tcstatus & ((1 << CP0DB_SSt) | (1 << CP0DB_Halt)));
}

#if defined(TARGET_MIPS64)
target_ulong helper_dmfc0_tcrestart (void)
{
    return env->active_tc.PC;
}

target_ulong helper_dmfc0_tchalt (void)
{
    return env->active_tc.CP0_TCHalt;
}

target_ulong helper_dmfc0_tccontext (void)
{
    return env->active_tc.CP0_TCContext;
}

target_ulong helper_dmfc0_tcschedule (void)
{
    return env->active_tc.CP0_TCSchedule;
}

target_ulong helper_dmfc0_tcschefback (void)
{
    return env->active_tc.CP0_TCScheFBack;
}

target_ulong helper_dmfc0_lladdr (void)
{
    return env->lladdr >> env->CP0_LLAddr_shift;
}

target_ulong helper_dmfc0_watchlo (uint32_t sel)
{
    return env->CP0_WatchLo[sel];
}
#endif /* TARGET_MIPS64 */

void helper_mtc0_index (target_ulong arg1)
{
    int num = 1;
    unsigned int tmp = env->tlb->nb_tlb;

    do {
        tmp >>= 1;
        num <<= 1;
    } while (tmp);
    env->CP0_Index = (env->CP0_Index & 0x80000000) | (arg1 & (num - 1));
}

void helper_mtc0_mvpcontrol (target_ulong arg1)
{
    uint32_t mask = 0;
    uint32_t newval;

    if (env->CP0_VPEConf0 & (1 << CP0VPEC0_MVP))
        mask |= (1 << CP0MVPCo_CPA) | (1 << CP0MVPCo_VPC) |
                (1 << CP0MVPCo_EVP);
    if (env->mvp->CP0_MVPControl & (1 << CP0MVPCo_VPC))
        mask |= (1 << CP0MVPCo_STLB);
    newval = (env->mvp->CP0_MVPControl & ~mask) | (arg1 & mask);

    // TODO: Enable/disable shared TLB, enable/disable VPEs.

    env->mvp->CP0_MVPControl = newval;
}

void helper_mtc0_vpecontrol (target_ulong arg1)
{
    uint32_t mask;
    uint32_t newval;

    mask = (1 << CP0VPECo_YSI) | (1 << CP0VPECo_GSI) |
           (1 << CP0VPECo_TE) | (0xff << CP0VPECo_TargTC);
    newval = (env->CP0_VPEControl & ~mask) | (arg1 & mask);

    /* Yield scheduler intercept not implemented. */
    /* Gating storage scheduler intercept not implemented. */

    // TODO: Enable/disable TCs.

    env->CP0_VPEControl = newval;
}

void helper_mttc0_vpecontrol(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);
    uint32_t mask;
    uint32_t newval;

    mask = (1 << CP0VPECo_YSI) | (1 << CP0VPECo_GSI) |
           (1 << CP0VPECo_TE) | (0xff << CP0VPECo_TargTC);
    newval = (other->CP0_VPEControl & ~mask) | (arg1 & mask);

    /* TODO: Enable/disable TCs.  */

    other->CP0_VPEControl = newval;
}

target_ulong helper_mftc0_vpecontrol(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);
    /* FIXME: Mask away return zero on read bits.  */
    return other->CP0_VPEControl;
}

target_ulong helper_mftc0_vpeconf0(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    return other->CP0_VPEConf0;
}

void helper_mtc0_vpeconf0 (target_ulong arg1)
{
    uint32_t mask = 0;
    uint32_t newval;

    if (env->CP0_VPEConf0 & (1 << CP0VPEC0_MVP)) {
        if (env->CP0_VPEConf0 & (1 << CP0VPEC0_VPA))
            mask |= (0xff << CP0VPEC0_XTC);
        mask |= (1 << CP0VPEC0_MVP) | (1 << CP0VPEC0_VPA);
    }
    newval = (env->CP0_VPEConf0 & ~mask) | (arg1 & mask);

    // TODO: TC exclusive handling due to ERL/EXL.

    env->CP0_VPEConf0 = newval;
}

void helper_mttc0_vpeconf0(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);
    uint32_t mask = 0;
    uint32_t newval;

    mask |= (1 << CP0VPEC0_MVP) | (1 << CP0VPEC0_VPA);
    newval = (other->CP0_VPEConf0 & ~mask) | (arg1 & mask);

    /* TODO: TC exclusive handling due to ERL/EXL.  */
    other->CP0_VPEConf0 = newval;
}

void helper_mtc0_vpeconf1 (target_ulong arg1)
{
    uint32_t mask = 0;
    uint32_t newval;

    if (env->mvp->CP0_MVPControl & (1 << CP0MVPCo_VPC))
        mask |= (0xff << CP0VPEC1_NCX) | (0xff << CP0VPEC1_NCP2) |
                (0xff << CP0VPEC1_NCP1);
    newval = (env->CP0_VPEConf1 & ~mask) | (arg1 & mask);

    /* UDI not implemented. */
    /* CP2 not implemented. */

    // TODO: Handle FPU (CP1) binding.

    env->CP0_VPEConf1 = newval;
}

void helper_mtc0_yqmask (target_ulong arg1)
{
    /* Yield qualifier inputs not implemented. */
    env->CP0_YQMask = 0x00000000;
}

void helper_mtc0_vpeopt (target_ulong arg1)
{
    env->CP0_VPEOpt = arg1 & 0x0000ffff;
}

void helper_mtc0_entrylo0 (target_ulong arg1)
{
    /* Large physaddr (PABITS) not implemented */
    /* 1k pages not implemented */
    env->CP0_EntryLo0 = arg1 & 0x3FFFFFFF;
}

void helper_mtc0_tcstatus (target_ulong arg1)
{
    uint32_t mask = env->CP0_TCStatus_rw_bitmask;
    uint32_t newval;

    newval = (env->active_tc.CP0_TCStatus & ~mask) | (arg1 & mask);

    env->active_tc.CP0_TCStatus = newval;
    sync_c0_tcstatus(env, env->current_tc, newval);
}

void helper_mttc0_tcstatus (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCStatus = arg1;
    else
        other->tcs[other_tc].CP0_TCStatus = arg1;
    sync_c0_tcstatus(other, other_tc, arg1);
}

void helper_mtc0_tcbind (target_ulong arg1)
{
    uint32_t mask = (1 << CP0TCBd_TBE);
    uint32_t newval;

    if (env->mvp->CP0_MVPControl & (1 << CP0MVPCo_VPC))
        mask |= (1 << CP0TCBd_CurVPE);
    newval = (env->active_tc.CP0_TCBind & ~mask) | (arg1 & mask);
    env->active_tc.CP0_TCBind = newval;
}

void helper_mttc0_tcbind (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    uint32_t mask = (1 << CP0TCBd_TBE);
    uint32_t newval;
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other->mvp->CP0_MVPControl & (1 << CP0MVPCo_VPC))
        mask |= (1 << CP0TCBd_CurVPE);
    if (other_tc == other->current_tc) {
        newval = (other->active_tc.CP0_TCBind & ~mask) | (arg1 & mask);
        other->active_tc.CP0_TCBind = newval;
    } else {
        newval = (other->tcs[other_tc].CP0_TCBind & ~mask) | (arg1 & mask);
        other->tcs[other_tc].CP0_TCBind = newval;
    }
}

void helper_mtc0_tcrestart (target_ulong arg1)
{
    env->active_tc.PC = arg1;
    env->active_tc.CP0_TCStatus &= ~(1 << CP0TCSt_TDS);
    env->lladdr = 0ULL;
    /* MIPS16 not implemented. */
}

void helper_mttc0_tcrestart (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc) {
        other->active_tc.PC = arg1;
        other->active_tc.CP0_TCStatus &= ~(1 << CP0TCSt_TDS);
        other->lladdr = 0ULL;
        /* MIPS16 not implemented. */
    } else {
        other->tcs[other_tc].PC = arg1;
        other->tcs[other_tc].CP0_TCStatus &= ~(1 << CP0TCSt_TDS);
        other->lladdr = 0ULL;
        /* MIPS16 not implemented. */
    }
}

void helper_mtc0_tchalt (target_ulong arg1)
{
    env->active_tc.CP0_TCHalt = arg1 & 0x1;

    // TODO: Halt TC / Restart (if allocated+active) TC.
    if (env->active_tc.CP0_TCHalt & 1) {
        mips_tc_sleep(env, env->current_tc);
    } else {
        mips_tc_wake(env, env->current_tc);
    }
}

void helper_mttc0_tchalt (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    // TODO: Halt TC / Restart (if allocated+active) TC.

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCHalt = arg1;
    else
        other->tcs[other_tc].CP0_TCHalt = arg1;

    if (arg1 & 1) {
        mips_tc_sleep(other, other_tc);
    } else {
        mips_tc_wake(other, other_tc);
    }
}

void helper_mtc0_tccontext (target_ulong arg1)
{
    env->active_tc.CP0_TCContext = arg1;
}

void helper_mttc0_tccontext (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCContext = arg1;
    else
        other->tcs[other_tc].CP0_TCContext = arg1;
}

void helper_mtc0_tcschedule (target_ulong arg1)
{
    env->active_tc.CP0_TCSchedule = arg1;
}

void helper_mttc0_tcschedule (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCSchedule = arg1;
    else
        other->tcs[other_tc].CP0_TCSchedule = arg1;
}

void helper_mtc0_tcschefback (target_ulong arg1)
{
    env->active_tc.CP0_TCScheFBack = arg1;
}

void helper_mttc0_tcschefback (target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCScheFBack = arg1;
    else
        other->tcs[other_tc].CP0_TCScheFBack = arg1;
}

void helper_mtc0_entrylo1 (target_ulong arg1)
{
    /* Large physaddr (PABITS) not implemented */
    /* 1k pages not implemented */
    env->CP0_EntryLo1 = arg1 & 0x3FFFFFFF;
}

void helper_mtc0_context (target_ulong arg1)
{
    env->CP0_Context = (env->CP0_Context & 0x007FFFFF) | (arg1 & ~0x007FFFFF);
}

void helper_mtc0_pagemask (target_ulong arg1)
{
    /* 1k pages not implemented */
    env->CP0_PageMask = arg1 & (0x1FFFFFFF & (TARGET_PAGE_MASK << 1));
}

void helper_mtc0_pagegrain (target_ulong arg1)
{
    /* SmartMIPS not implemented */
    /* Large physaddr (PABITS) not implemented */
    /* 1k pages not implemented */
    env->CP0_PageGrain = 0;
}

void helper_mtc0_wired (target_ulong arg1)
{
    env->CP0_Wired = arg1 % env->tlb->nb_tlb;
}

void helper_mtc0_srsconf0 (target_ulong arg1)
{
    env->CP0_SRSConf0 |= arg1 & env->CP0_SRSConf0_rw_bitmask;
}

void helper_mtc0_srsconf1 (target_ulong arg1)
{
    env->CP0_SRSConf1 |= arg1 & env->CP0_SRSConf1_rw_bitmask;
}

void helper_mtc0_srsconf2 (target_ulong arg1)
{
    env->CP0_SRSConf2 |= arg1 & env->CP0_SRSConf2_rw_bitmask;
}

void helper_mtc0_srsconf3 (target_ulong arg1)
{
    env->CP0_SRSConf3 |= arg1 & env->CP0_SRSConf3_rw_bitmask;
}

void helper_mtc0_srsconf4 (target_ulong arg1)
{
    env->CP0_SRSConf4 |= arg1 & env->CP0_SRSConf4_rw_bitmask;
}

void helper_mtc0_hwrena (target_ulong arg1)
{
    env->CP0_HWREna = arg1 & 0x0000000F;
}

void helper_mtc0_count (target_ulong arg1)
{
    cpu_mips_store_count(env, arg1);
}

void helper_mtc0_entryhi (target_ulong arg1)
{
    target_ulong old, val;

    /* 1k pages not implemented */
    val = arg1 & ((TARGET_PAGE_MASK << 1) | 0xFF);
#if defined(TARGET_MIPS64)
    val &= env->SEGMask;
#endif
    old = env->CP0_EntryHi;
    env->CP0_EntryHi = val;
    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        sync_c0_entryhi(env, env->current_tc);
    }
    /* If the ASID changes, flush qemu's TLB.  */
    if ((old & 0xFF) != (val & 0xFF))
        cpu_mips_tlb_flush(env, 1);
}

void helper_mttc0_entryhi(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    other->CP0_EntryHi = arg1;
    sync_c0_entryhi(other, other_tc);
}

void helper_mtc0_compare (target_ulong arg1)
{
    cpu_mips_store_compare(env, arg1);
}

void helper_mtc0_status (target_ulong arg1)
{
    uint32_t val, old;
    uint32_t mask = env->CP0_Status_rw_bitmask;

    val = arg1 & mask;
    old = env->CP0_Status;
    env->CP0_Status = (env->CP0_Status & ~mask) | val;
    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        sync_c0_status(env, env->current_tc);
    } else {
        compute_hflags(env);
    }

    if (qemu_loglevel_mask(CPU_LOG_EXEC)) {
        qemu_log("Status %08x (%08x) => %08x (%08x) Cause %08x",
                old, old & env->CP0_Cause & CP0Ca_IP_mask,
                val, val & env->CP0_Cause & CP0Ca_IP_mask,
                env->CP0_Cause);
        switch (env->hflags & MIPS_HFLAG_KSU) {
        case MIPS_HFLAG_UM: qemu_log(", UM\n"); break;
        case MIPS_HFLAG_SM: qemu_log(", SM\n"); break;
        case MIPS_HFLAG_KM: qemu_log("\n"); break;
        default: cpu_abort(env, "Invalid MMU mode!\n"); break;
        }
    }
}

void helper_mttc0_status(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    other->CP0_Status = arg1 & ~0xf1000018;
    sync_c0_status(other, other_tc);
}

void helper_mtc0_intctl (target_ulong arg1)
{
    /* vectored interrupts not implemented, no performance counters. */
    env->CP0_IntCtl = (env->CP0_IntCtl & ~0x000003e0) | (arg1 & 0x000003e0);
}

void helper_mtc0_srsctl (target_ulong arg1)
{
    uint32_t mask = (0xf << CP0SRSCtl_ESS) | (0xf << CP0SRSCtl_PSS);
    env->CP0_SRSCtl = (env->CP0_SRSCtl & ~mask) | (arg1 & mask);
}

static void mtc0_cause(CPUState *cpu, target_ulong arg1)
{
    uint32_t mask = 0x00C00300;
    uint32_t old = cpu->CP0_Cause;
    int i;

    if (cpu->insn_flags & ISA_MIPS32R2) {
        mask |= 1 << CP0Ca_DC;
    }

    cpu->CP0_Cause = (cpu->CP0_Cause & ~mask) | (arg1 & mask);

    if ((old ^ cpu->CP0_Cause) & (1 << CP0Ca_DC)) {
        if (cpu->CP0_Cause & (1 << CP0Ca_DC)) {
            cpu_mips_stop_count(cpu);
        } else {
            cpu_mips_start_count(cpu);
        }
    }

    /* Set/reset software interrupts */
    for (i = 0 ; i < 2 ; i++) {
        if ((old ^ cpu->CP0_Cause) & (1 << (CP0Ca_IP + i))) {
            cpu_mips_soft_irq(cpu, i, cpu->CP0_Cause & (1 << (CP0Ca_IP + i)));
        }
    }
}

void helper_mtc0_cause(target_ulong arg1)
{
    mtc0_cause(env, arg1);
}

void helper_mttc0_cause(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    mtc0_cause(other, arg1);
}

target_ulong helper_mftc0_epc(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    return other->CP0_EPC;
}

target_ulong helper_mftc0_ebase(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    return other->CP0_EBase;
}

void helper_mtc0_ebase (target_ulong arg1)
{
    /* vectored interrupts not implemented */
    env->CP0_EBase = (env->CP0_EBase & ~0x3FFFF000) | (arg1 & 0x3FFFF000);
}

void helper_mttc0_ebase(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);
    other->CP0_EBase = (other->CP0_EBase & ~0x3FFFF000) | (arg1 & 0x3FFFF000);
}

target_ulong helper_mftc0_configx(target_ulong idx)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    switch (idx) {
    case 0: return other->CP0_Config0;
    case 1: return other->CP0_Config1;
    case 2: return other->CP0_Config2;
    case 3: return other->CP0_Config3;
    /* 4 and 5 are reserved.  */
    case 6: return other->CP0_Config6;
    case 7: return other->CP0_Config7;
    default:
        break;
    }
    return 0;
}

void helper_mtc0_config0 (target_ulong arg1)
{
    env->CP0_Config0 = (env->CP0_Config0 & 0x81FFFFF8) | (arg1 & 0x00000007);
}

void helper_mtc0_config2 (target_ulong arg1)
{
    /* tertiary/secondary caches not implemented */
    env->CP0_Config2 = (env->CP0_Config2 & 0x8FFF0FFF);
}

void helper_mtc0_lladdr (target_ulong arg1)
{
    target_long mask = env->CP0_LLAddr_rw_bitmask;
    arg1 = arg1 << env->CP0_LLAddr_shift;
    env->lladdr = (env->lladdr & ~mask) | (arg1 & mask);
}

void helper_mtc0_watchlo (target_ulong arg1, uint32_t sel)
{
    /* Watch exceptions for instructions, data loads, data stores
       not implemented. */
    env->CP0_WatchLo[sel] = (arg1 & ~0x7);
}

void helper_mtc0_watchhi (target_ulong arg1, uint32_t sel)
{
    env->CP0_WatchHi[sel] = (arg1 & 0x40FF0FF8);
    env->CP0_WatchHi[sel] &= ~(env->CP0_WatchHi[sel] & arg1 & 0x7);
}

void helper_mtc0_xcontext (target_ulong arg1)
{
    target_ulong mask = (1ULL << (env->SEGBITS - 7)) - 1;
    env->CP0_XContext = (env->CP0_XContext & mask) | (arg1 & ~mask);
}

void helper_mtc0_framemask (target_ulong arg1)
{
    env->CP0_Framemask = arg1; /* XXX */
}

void helper_mtc0_debug (target_ulong arg1)
{
    env->CP0_Debug = (env->CP0_Debug & 0x8C03FC1F) | (arg1 & 0x13300120);
    if (arg1 & (1 << CP0DB_DM))
        env->hflags |= MIPS_HFLAG_DM;
    else
        env->hflags &= ~MIPS_HFLAG_DM;
}

void helper_mttc0_debug(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    uint32_t val = arg1 & ((1 << CP0DB_SSt) | (1 << CP0DB_Halt));
    CPUState *other = mips_cpu_map_tc(&other_tc);

    /* XXX: Might be wrong, check with EJTAG spec. */
    if (other_tc == other->current_tc)
        other->active_tc.CP0_Debug_tcstatus = val;
    else
        other->tcs[other_tc].CP0_Debug_tcstatus = val;
    other->CP0_Debug = (other->CP0_Debug &
                     ((1 << CP0DB_SSt) | (1 << CP0DB_Halt))) |
                     (arg1 & ~((1 << CP0DB_SSt) | (1 << CP0DB_Halt)));
}

void helper_mtc0_performance0 (target_ulong arg1)
{
    env->CP0_Performance0 = arg1 & 0x000007ff;
}

void helper_mtc0_taglo (target_ulong arg1)
{
    env->CP0_TagLo = arg1 & 0xFFFFFCF6;
}

void helper_mtc0_datalo (target_ulong arg1)
{
    env->CP0_DataLo = arg1; /* XXX */
}

void helper_mtc0_taghi (target_ulong arg1)
{
    env->CP0_TagHi = arg1; /* XXX */
}

void helper_mtc0_datahi (target_ulong arg1)
{
    env->CP0_DataHi = arg1; /* XXX */
}

/* MIPS MT functions */
target_ulong helper_mftgpr(uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.gpr[sel];
    else
        return other->tcs[other_tc].gpr[sel];
}

target_ulong helper_mftlo(uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.LO[sel];
    else
        return other->tcs[other_tc].LO[sel];
}

target_ulong helper_mfthi(uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.HI[sel];
    else
        return other->tcs[other_tc].HI[sel];
}

target_ulong helper_mftacx(uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.ACX[sel];
    else
        return other->tcs[other_tc].ACX[sel];
}

target_ulong helper_mftdsp(void)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.DSPControl;
    else
        return other->tcs[other_tc].DSPControl;
}

void helper_mttgpr(target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.gpr[sel] = arg1;
    else
        other->tcs[other_tc].gpr[sel] = arg1;
}

void helper_mttlo(target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.LO[sel] = arg1;
    else
        other->tcs[other_tc].LO[sel] = arg1;
}

void helper_mtthi(target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.HI[sel] = arg1;
    else
        other->tcs[other_tc].HI[sel] = arg1;
}

void helper_mttacx(target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.ACX[sel] = arg1;
    else
        other->tcs[other_tc].ACX[sel] = arg1;
}

void helper_mttdsp(target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUState *other = mips_cpu_map_tc(&other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.DSPControl = arg1;
    else
        other->tcs[other_tc].DSPControl = arg1;
}

/* MIPS MT functions */
target_ulong helper_dmt(void)
{
    // TODO
     return 0;
}

target_ulong helper_emt(void)
{
    // TODO
    return 0;
}

target_ulong helper_dvpe(void)
{
    CPUState *other_cpu = first_cpu;
    target_ulong prev = env->mvp->CP0_MVPControl;

    do {
        /* Turn off all VPEs except the one executing the dvpe.  */
        if (other_cpu != env) {
            other_cpu->mvp->CP0_MVPControl &= ~(1 << CP0MVPCo_EVP);
            mips_vpe_sleep(other_cpu);
        }
        other_cpu = other_cpu->next_cpu;
    } while (other_cpu);
    return prev;
}

target_ulong helper_evpe(void)
{
    CPUState *other_cpu = first_cpu;
    target_ulong prev = env->mvp->CP0_MVPControl;

    do {
        if (other_cpu != env
           /* If the VPE is WFI, dont distrub it's sleep.  */
           && !mips_vpe_is_wfi(other_cpu)) {
            /* Enable the VPE.  */
            other_cpu->mvp->CP0_MVPControl |= (1 << CP0MVPCo_EVP);
            mips_vpe_wake(other_cpu); /* And wake it up.  */
        }
        other_cpu = other_cpu->next_cpu;
    } while (other_cpu);
    return prev;
}
#endif /* !CONFIG_USER_ONLY */

void helper_fork(target_ulong arg1, target_ulong arg2)
{
    // arg1 = rt, arg2 = rs
    arg1 = 0;
    // TODO: store to TC register
}

target_ulong helper_yield(target_ulong arg)
{
    target_long arg1 = arg;

    if (arg1 < 0) {
        /* No scheduling policy implemented. */
        if (arg1 != -2) {
            if (env->CP0_VPEControl & (1 << CP0VPECo_YSI) &&
                env->active_tc.CP0_TCStatus & (1 << CP0TCSt_DT)) {
                env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
                env->CP0_VPEControl |= 4 << CP0VPECo_EXCPT;
                helper_raise_exception(EXCP_THREAD);
            }
        }
    } else if (arg1 == 0) {
        if (0 /* TODO: TC underflow */) {
            env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
            helper_raise_exception(EXCP_THREAD);
        } else {
            // TODO: Deallocate TC
        }
    } else if (arg1 > 0) {
        /* Yield qualifier inputs not implemented. */
        env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
        env->CP0_VPEControl |= 2 << CP0VPECo_EXCPT;
        helper_raise_exception(EXCP_THREAD);
    }
    return env->CP0_YQMask;
}

#ifndef CONFIG_USER_ONLY
/* TLB management */
static void cpu_mips_tlb_flush (CPUState *env, int flush_global)
{
    /* Flush qemu's TLB and discard all shadowed entries.  */
    tlb_flush (env, flush_global);
    env->tlb->tlb_in_use = env->tlb->nb_tlb;
}

static void r4k_mips_tlb_flush_extra (CPUState *env, int first)
{
    /* Discard entries from env->tlb[first] onwards.  */
    while (env->tlb->tlb_in_use > first) {
        r4k_invalidate_tlb(env, --env->tlb->tlb_in_use, 0);
    }
}

static void r4k_fill_tlb (int idx)
{
    r4k_tlb_t *tlb;

    /* XXX: detect conflicting TLBs and raise a MCHECK exception when needed */
    tlb = &env->tlb->mmu.r4k.tlb[idx];
    tlb->VPN = env->CP0_EntryHi & (TARGET_PAGE_MASK << 1);
#if defined(TARGET_MIPS64)
    tlb->VPN &= env->SEGMask;
#endif
    tlb->ASID = env->CP0_EntryHi & 0xFF;
    tlb->PageMask = env->CP0_PageMask;
    tlb->G = env->CP0_EntryLo0 & env->CP0_EntryLo1 & 1;
    tlb->V0 = (env->CP0_EntryLo0 & 2) != 0;
    tlb->D0 = (env->CP0_EntryLo0 & 4) != 0;
    tlb->C0 = (env->CP0_EntryLo0 >> 3) & 0x7;
    tlb->PFN[0] = (env->CP0_EntryLo0 >> 6) << 12;
    tlb->V1 = (env->CP0_EntryLo1 & 2) != 0;
    tlb->D1 = (env->CP0_EntryLo1 & 4) != 0;
    tlb->C1 = (env->CP0_EntryLo1 >> 3) & 0x7;
    tlb->PFN[1] = (env->CP0_EntryLo1 >> 6) << 12;
}

void r4k_helper_tlbwi (void)
{
    int idx;

    idx = (env->CP0_Index & ~0x80000000) % env->tlb->nb_tlb;

    /* Discard cached TLB entries.  We could avoid doing this if the
       tlbwi is just upgrading access permissions on the current entry;
       that might be a further win.  */
    r4k_mips_tlb_flush_extra (env, env->tlb->nb_tlb);

    r4k_invalidate_tlb(env, idx, 0);
    r4k_fill_tlb(idx);
}

void r4k_helper_tlbwr (void)
{
    int r = cpu_mips_get_random(env);

    r4k_invalidate_tlb(env, r, 1);
    r4k_fill_tlb(r);
}

void r4k_helper_tlbp (void)
{
    r4k_tlb_t *tlb;
    target_ulong mask;
    target_ulong tag;
    target_ulong VPN;
    uint8_t ASID;
    int i;

    ASID = env->CP0_EntryHi & 0xFF;
    for (i = 0; i < env->tlb->nb_tlb; i++) {
        tlb = &env->tlb->mmu.r4k.tlb[i];
        /* 1k pages are not supported. */
        mask = tlb->PageMask | ~(TARGET_PAGE_MASK << 1);
        tag = env->CP0_EntryHi & ~mask;
        VPN = tlb->VPN & ~mask;
        /* Check ASID, virtual page number & size */
        if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag) {
            /* TLB match */
            env->CP0_Index = i;
            break;
        }
    }
    if (i == env->tlb->nb_tlb) {
        /* No match.  Discard any shadow entries, if any of them match.  */
        for (i = env->tlb->nb_tlb; i < env->tlb->tlb_in_use; i++) {
            tlb = &env->tlb->mmu.r4k.tlb[i];
            /* 1k pages are not supported. */
            mask = tlb->PageMask | ~(TARGET_PAGE_MASK << 1);
            tag = env->CP0_EntryHi & ~mask;
            VPN = tlb->VPN & ~mask;
            /* Check ASID, virtual page number & size */
            if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag) {
                r4k_mips_tlb_flush_extra (env, i);
                break;
            }
        }

        env->CP0_Index |= 0x80000000;
    }
}

void r4k_helper_tlbr (void)
{
    r4k_tlb_t *tlb;
    uint8_t ASID;
    int idx;

    ASID = env->CP0_EntryHi & 0xFF;
    idx = (env->CP0_Index & ~0x80000000) % env->tlb->nb_tlb;
    tlb = &env->tlb->mmu.r4k.tlb[idx];

    /* If this will change the current ASID, flush qemu's TLB.  */
    if (ASID != tlb->ASID)
        cpu_mips_tlb_flush (env, 1);

    r4k_mips_tlb_flush_extra(env, env->tlb->nb_tlb);

    env->CP0_EntryHi = tlb->VPN | tlb->ASID;
    env->CP0_PageMask = tlb->PageMask;
    env->CP0_EntryLo0 = tlb->G | (tlb->V0 << 1) | (tlb->D0 << 2) |
                        (tlb->C0 << 3) | (tlb->PFN[0] >> 6);
    env->CP0_EntryLo1 = tlb->G | (tlb->V1 << 1) | (tlb->D1 << 2) |
                        (tlb->C1 << 3) | (tlb->PFN[1] >> 6);
}

void helper_tlbwi(void)
{
    env->tlb->helper_tlbwi();
}

void helper_tlbwr(void)
{
    env->tlb->helper_tlbwr();
}

void helper_tlbp(void)
{
    env->tlb->helper_tlbp();
}

void helper_tlbr(void)
{
    env->tlb->helper_tlbr();
}

/* Specials */
target_ulong helper_di (void)
{
    target_ulong t0 = env->CP0_Status;

    env->CP0_Status = t0 & ~(1 << CP0St_IE);
    return t0;
}

target_ulong helper_ei (void)
{
    target_ulong t0 = env->CP0_Status;

    env->CP0_Status = t0 | (1 << CP0St_IE);
    return t0;
}

static void debug_pre_eret (void)
{
    if (qemu_loglevel_mask(CPU_LOG_EXEC)) {
        qemu_log("ERET: PC " TARGET_FMT_lx " EPC " TARGET_FMT_lx,
                env->active_tc.PC, env->CP0_EPC);
        if (env->CP0_Status & (1 << CP0St_ERL))
            qemu_log(" ErrorEPC " TARGET_FMT_lx, env->CP0_ErrorEPC);
        if (env->hflags & MIPS_HFLAG_DM)
            qemu_log(" DEPC " TARGET_FMT_lx, env->CP0_DEPC);
        qemu_log("\n");
    }
}

static void debug_post_eret (void)
{
    if (qemu_loglevel_mask(CPU_LOG_EXEC)) {
        qemu_log("  =>  PC " TARGET_FMT_lx " EPC " TARGET_FMT_lx,
                env->active_tc.PC, env->CP0_EPC);
        if (env->CP0_Status & (1 << CP0St_ERL))
            qemu_log(" ErrorEPC " TARGET_FMT_lx, env->CP0_ErrorEPC);
        if (env->hflags & MIPS_HFLAG_DM)
            qemu_log(" DEPC " TARGET_FMT_lx, env->CP0_DEPC);
        switch (env->hflags & MIPS_HFLAG_KSU) {
        case MIPS_HFLAG_UM: qemu_log(", UM\n"); break;
        case MIPS_HFLAG_SM: qemu_log(", SM\n"); break;
        case MIPS_HFLAG_KM: qemu_log("\n"); break;
        default: cpu_abort(env, "Invalid MMU mode!\n"); break;
        }
    }
}

static void set_pc (target_ulong error_pc)
{
    env->active_tc.PC = error_pc & ~(target_ulong)1;
    if (error_pc & 1) {
        env->hflags |= MIPS_HFLAG_M16;
    } else {
        env->hflags &= ~(MIPS_HFLAG_M16);
    }
}

void helper_eret (void)
{
    debug_pre_eret();
    if (env->CP0_Status & (1 << CP0St_ERL)) {
        set_pc(env->CP0_ErrorEPC);
        env->CP0_Status &= ~(1 << CP0St_ERL);
    } else {
        set_pc(env->CP0_EPC);
        env->CP0_Status &= ~(1 << CP0St_EXL);
    }
    compute_hflags(env);
    debug_post_eret();
    env->lladdr = 1;
}

void helper_deret (void)
{
    debug_pre_eret();
    set_pc(env->CP0_DEPC);

    env->hflags &= MIPS_HFLAG_DM;
    compute_hflags(env);
    debug_post_eret();
    env->lladdr = 1;
}
#endif /* !CONFIG_USER_ONLY */

target_ulong helper_rdhwr_cpunum(void)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 0)))
        return env->CP0_EBase & 0x3ff;
    else
        helper_raise_exception(EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_synci_step(void)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 1)))
        return env->SYNCI_Step;
    else
        helper_raise_exception(EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_cc(void)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 2)))
        return env->CP0_Count;
    else
        helper_raise_exception(EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_ccres(void)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 3)))
        return env->CCRes;
    else
        helper_raise_exception(EXCP_RI);

    return 0;
}

void helper_pmon (int function)
{
    function /= 2;
    switch (function) {
    case 2: /* TODO: char inbyte(int waitflag); */
        if (env->active_tc.gpr[4] == 0)
            env->active_tc.gpr[2] = -1;
        /* Fall through */
    case 11: /* TODO: char inbyte (void); */
        env->active_tc.gpr[2] = -1;
        break;
    case 3:
    case 12:
        printf("%c", (char)(env->active_tc.gpr[4] & 0xFF));
        break;
    case 17:
        break;
    case 158:
        {
            unsigned char *fmt = (void *)(unsigned long)env->active_tc.gpr[4];
            printf("%s", fmt);
        }
        break;
    }
}

void helper_wait (void)
{
    env->halted = 1;
    cpu_reset_interrupt(env, CPU_INTERRUPT_WAKE);
    helper_raise_exception(EXCP_HLT);
}

#if !defined(CONFIG_USER_ONLY)

static void do_unaligned_access (target_ulong addr, int is_write, int is_user, void *retaddr);

#define MMUSUFFIX _mmu
#define ALIGNED_ONLY

#define SHIFT 0
#include "softmmu_template.h"

#define SHIFT 1
#include "softmmu_template.h"

#define SHIFT 2
#include "softmmu_template.h"

#define SHIFT 3
#include "softmmu_template.h"

static void do_unaligned_access (target_ulong addr, int is_write, int is_user, void *retaddr)
{
    env->CP0_BadVAddr = addr;
    do_restore_state (retaddr);
    helper_raise_exception ((is_write == 1) ? EXCP_AdES : EXCP_AdEL);
}

void tlb_fill(CPUState *env1, target_ulong addr, int is_write, int mmu_idx,
              void *retaddr)
{
    TranslationBlock *tb;
    CPUState *saved_env;
    unsigned long pc;
    int ret;

    saved_env = env;
    env = env1;
    ret = cpu_mips_handle_mmu_fault(env, addr, is_write, mmu_idx);
    if (ret) {
        if (retaddr) {
            /* now we have a real cpu fault */
            pc = (unsigned long)retaddr;
            tb = tb_find_pc(pc);
            if (tb) {
                /* the PC is inside the translated code. It means that we have
                   a virtual CPU fault */
                cpu_restore_state(tb, env, pc);
            }
        }
        helper_raise_exception_err(env->exception_index, env->error_code);
    }
    env = saved_env;
}

void cpu_unassigned_access(CPUState *env1, target_phys_addr_t addr,
                           int is_write, int is_exec, int unused, int size)
{
    env = env1;

    if (is_exec)
        helper_raise_exception(EXCP_IBE);
    else
        helper_raise_exception(EXCP_DBE);
}
#endif /* !CONFIG_USER_ONLY */

/* Complex FPU operations which may need stack space. */

#define FLOAT_ONE32 make_float32(0x3f8 << 20)
#define FLOAT_ONE64 make_float64(0x3ffULL << 52)
#define FLOAT_TWO32 make_float32(1 << 30)
#define FLOAT_TWO64 make_float64(1ULL << 62)

#define FLOAT_QNAN16 (int16_t)float16_default_nan /* 0x7e00 */
#define FLOAT_QNAN32 (int32_t)float32_default_nan /* 0x7fc00000 */
#define FLOAT_QNAN64 (int64_t)float64_default_nan /* 0x7ff8000000000000 */

#define FLOAT_SNAN16 (float16_default_nan ^ 0x0220) /* 0x7c20 */
#define FLOAT_SNAN32 (float32_default_nan ^ 0x00400020) /* 0x7f800020 */
#define FLOAT_SNAN64 (float64_default_nan ^ 0x0008000000000020ULL) /* 0x7ff0000000000020 */


/* convert MIPS rounding mode in FCR31 to IEEE library */
static unsigned int ieee_rm[] = {
    float_round_nearest_even,
    float_round_to_zero,
    float_round_up,
    float_round_down
};

#define RESTORE_ROUNDING_MODE \
    set_float_rounding_mode(ieee_rm[env->active_fpu.fcr31 & 3], &env->active_fpu.fp_status)

#define RESTORE_FLUSH_MODE \
    set_flush_to_zero((env->active_fpu.fcr31 & (1 << 24)) != 0, &env->active_fpu.fp_status);

target_ulong helper_cfc1 (uint32_t reg)
{
    target_ulong arg1;

    switch (reg) {
    case 0:
        arg1 = (int32_t)env->active_fpu.fcr0;
        break;
    case 25:
        arg1 = ((env->active_fpu.fcr31 >> 24) & 0xfe) | ((env->active_fpu.fcr31 >> 23) & 0x1);
        break;
    case 26:
        arg1 = env->active_fpu.fcr31 & 0x0003f07c;
        break;
    case 28:
        arg1 = (env->active_fpu.fcr31 & 0x00000f83) | ((env->active_fpu.fcr31 >> 22) & 0x4);
        break;
    case 30: /* FCSR2 mirrors CP0St_FR. Note: this implementation
              * returns CP0St_FR even when FCSR2 does not exist) */
        arg1 = env->CP0_Status & (1 << CP0St_FR);
        break;
    default:
        arg1 = (int32_t)env->active_fpu.fcr31;
        break;
    }

    return arg1;
}

void helper_ctc1 (target_ulong arg1, uint32_t reg)
{
    switch(reg) {
    case 25:
        if (arg1 & 0xffffff00)
            return;
        env->active_fpu.fcr31 = (env->active_fpu.fcr31 & 0x017fffff) | ((arg1 & 0xfe) << 24) |
                     ((arg1 & 0x1) << 23);
        break;
    case 26:
        if (arg1 & 0x007c0000)
            return;
        env->active_fpu.fcr31 = (env->active_fpu.fcr31 & 0xfffc0f83) | (arg1 & 0x0003f07c);
        break;
    case 28:
        if (arg1 & 0x007c0000)
            return;
        env->active_fpu.fcr31 = (env->active_fpu.fcr31 & 0xfefff07c) | (arg1 & 0x00000f83) |
                     ((arg1 & 0x4) << 22);
        break;
    case 30: /* FCSR2 exists only if FCR0_CR2 is set and mirrors
              * CP0St_FR */
        if ((env->active_fpu.fcr0 & (1 << FCR0_CR2))) {
          if (arg1 & (1 << CP0St_FR)) {
            env->CP0_Status |= (1 << CP0St_FR);
            env->hflags     |=  MIPS_HFLAG_F64;
          }
          else {
            env->CP0_Status &= ~(1 << CP0St_FR);
            env->hflags     &= ~MIPS_HFLAG_F64;
          }
        }

        return;
    case 31:
        if (arg1 & 0x007c0000)
            return;
        env->active_fpu.fcr31 = arg1;
        break;
    default:
        return;
    }
    /* set rounding mode */
    RESTORE_ROUNDING_MODE;
    /* set flush-to-zero mode */
    RESTORE_FLUSH_MODE;
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    if ((GET_FP_ENABLE(env->active_fpu.fcr31) | 0x20) & GET_FP_CAUSE(env->active_fpu.fcr31))
        helper_raise_exception(EXCP_FPE);
}

static inline int ieee_ex_to_mips(int xcpt)
{
    int ret = 0;
    if (xcpt) {
        if (xcpt & float_flag_invalid) {
            ret |= FP_INVALID;
        }
        if (xcpt & float_flag_overflow) {
            ret |= FP_OVERFLOW;
        }
        if (xcpt & float_flag_underflow) {
            ret |= FP_UNDERFLOW;
        }
        if (xcpt & float_flag_divbyzero) {
            ret |= FP_DIV0;
        }
        if (xcpt & float_flag_inexact) {
            ret |= FP_INEXACT;
        }
    }
    return ret;
}

static inline void update_fcr31(void)
{
    int tmp = ieee_ex_to_mips(get_float_exception_flags(&env->active_fpu.fp_status));

    SET_FP_CAUSE(env->active_fpu.fcr31, tmp);
    if (GET_FP_ENABLE(env->active_fpu.fcr31) & tmp)
        helper_raise_exception(EXCP_FPE);
    else
        UPDATE_FP_FLAGS(env->active_fpu.fcr31, tmp);
}

/* Float support.
   Single precition routines have a "s" suffix, double precision a
   "d" suffix, 32bit integer "w", 64bit integer "l", paired single "ps",
   paired single lower "pl", paired single upper "pu".  */

/* unary operations, modifying fp status  */
uint64_t helper_float_sqrt_d(uint64_t fdt0)
{
    return float64_sqrt(fdt0, &env->active_fpu.fp_status);
}

uint32_t helper_float_sqrt_s(uint32_t fst0)
{
    return float32_sqrt(fst0, &env->active_fpu.fp_status);
}

uint64_t helper_float_cvtd_s(uint32_t fst0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float32_to_float64(fst0, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint64_t helper_float_cvtd_w(uint32_t wt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = int32_to_float64(wt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint64_t helper_float_cvtd_l(uint64_t dt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = int64_to_float64(dt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint64_t helper_float_cvtl_d(uint64_t fdt0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_cvtl_s(uint32_t fst0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_cvtps_pw(uint64_t dt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = int32_to_float32(dt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = int32_to_float32(dt0 >> 32, &env->active_fpu.fp_status);
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_cvtpw_ps(uint64_t fdt0)
{
    uint32_t wt2;
    uint32_t wth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    wth2 = float32_to_int32(fdt0 >> 32, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID)) {
        wt2 = FLOAT_SNAN32;
        wth2 = FLOAT_SNAN32;
    }
    return ((uint64_t)wth2 << 32) | wt2;
}

uint32_t helper_float_cvts_d(uint64_t fdt0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float64_to_float32(fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint32_t helper_float_cvts_w(uint32_t wt0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = int32_to_float32(wt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint32_t helper_float_cvts_l(uint64_t dt0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = int64_to_float32(dt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint32_t helper_float_cvts_pl(uint32_t wt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = wt0;
    update_fcr31();
    return wt2;
}

uint32_t helper_float_cvts_pu(uint32_t wth0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = wth0;
    update_fcr31();
    return wt2;
}

uint32_t helper_float_cvtw_s(uint32_t fst0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint32_t helper_float_cvtw_d(uint64_t fdt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint64_t helper_float_roundl_d(uint64_t fdt0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_roundl_s(uint32_t fst0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint32_t helper_float_roundw_d(uint64_t fdt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint32_t helper_float_roundw_s(uint32_t fst0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint64_t helper_float_truncl_d(uint64_t fdt0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    dt2 = float64_to_int64_round_to_zero(fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_truncl_s(uint32_t fst0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    dt2 = float32_to_int64_round_to_zero(fst0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint32_t helper_float_truncw_d(uint64_t fdt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = float64_to_int32_round_to_zero(fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint32_t helper_float_truncw_s(uint32_t fst0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wt2 = float32_to_int32_round_to_zero(fst0, &env->active_fpu.fp_status);
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint64_t helper_float_ceill_d(uint64_t fdt0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_ceill_s(uint32_t fst0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint32_t helper_float_ceilw_d(uint64_t fdt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint32_t helper_float_ceilw_s(uint32_t fst0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint64_t helper_float_floorl_d(uint64_t fdt0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint64_t helper_float_floorl_s(uint32_t fst0)
{
    uint64_t dt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        dt2 = FLOAT_SNAN64;
    return dt2;
}

uint32_t helper_float_floorw_d(uint64_t fdt0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

uint32_t helper_float_floorw_s(uint32_t fst0)
{
    uint32_t wt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    RESTORE_ROUNDING_MODE;
    update_fcr31();
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & (FP_OVERFLOW | FP_INVALID))
        wt2 = FLOAT_SNAN32;
    return wt2;
}

/* unary operations, not modifying fp status  */
#define FLOAT_UNOP(name)                                       \
uint64_t helper_float_ ## name ## _d(uint64_t fdt0)                \
{                                                              \
    return float64_ ## name(fdt0);                             \
}                                                              \
uint32_t helper_float_ ## name ## _s(uint32_t fst0)                \
{                                                              \
    return float32_ ## name(fst0);                             \
}                                                              \
uint64_t helper_float_ ## name ## _ps(uint64_t fdt0)               \
{                                                              \
    uint32_t wt0;                                              \
    uint32_t wth0;                                             \
                                                               \
    wt0 = float32_ ## name(fdt0 & 0XFFFFFFFF);                 \
    wth0 = float32_ ## name(fdt0 >> 32);                       \
    return ((uint64_t)wth0 << 32) | wt0;                       \
}
FLOAT_UNOP(abs)
FLOAT_UNOP(chs)
#undef FLOAT_UNOP

/* MIPS specific unary operations */
uint64_t helper_float_recip_d(uint64_t fdt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_div(FLOAT_ONE64, fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_recip_s(uint32_t fst0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fst0, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint64_t helper_float_rsqrt_d(uint64_t fdt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_sqrt(fdt0, &env->active_fpu.fp_status);
    fdt2 = float64_div(FLOAT_ONE64, fdt2, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_rsqrt_s(uint32_t fst0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_sqrt(fst0, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fst2, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint64_t helper_float_recip1_d(uint64_t fdt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_div(FLOAT_ONE64, fdt0, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_recip1_s(uint32_t fst0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fst0, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint64_t helper_float_recip1_ps(uint64_t fdt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = float32_div(FLOAT_ONE32, fdt0 >> 32, &env->active_fpu.fp_status);
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_rsqrt1_d(uint64_t fdt0)
{
    uint64_t fdt2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_sqrt(fdt0, &env->active_fpu.fp_status);
    fdt2 = float64_div(FLOAT_ONE64, fdt2, &env->active_fpu.fp_status);
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_rsqrt1_s(uint32_t fst0)
{
    uint32_t fst2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_sqrt(fst0, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fst2, &env->active_fpu.fp_status);
    update_fcr31();
    return fst2;
}

uint64_t helper_float_rsqrt1_ps(uint64_t fdt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_sqrt(fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = float32_sqrt(fdt0 >> 32, &env->active_fpu.fp_status);
    fst2 = float32_div(FLOAT_ONE32, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_div(FLOAT_ONE32, fsth2, &env->active_fpu.fp_status);
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

#define FLOAT_OP(name, p) void helper_float_##name##_##p(void)

/* binary operations */
#define FLOAT_BINOP(name)                                          \
uint64_t helper_float_ ## name ## _d(uint64_t fdt0, uint64_t fdt1)     \
{                                                                  \
    uint64_t dt2;                                                  \
                                                                   \
    set_float_exception_flags(0, &env->active_fpu.fp_status);            \
    dt2 = float64_ ## name (fdt0, fdt1, &env->active_fpu.fp_status);     \
    update_fcr31();                                                \
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & FP_INVALID)                \
        dt2 = FLOAT_QNAN64;                                        \
    return dt2;                                                    \
}                                                                  \
                                                                   \
uint32_t helper_float_ ## name ## _s(uint32_t fst0, uint32_t fst1)     \
{                                                                  \
    uint32_t wt2;                                                  \
                                                                   \
    set_float_exception_flags(0, &env->active_fpu.fp_status);            \
    wt2 = float32_ ## name (fst0, fst1, &env->active_fpu.fp_status);     \
    update_fcr31();                                                \
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & FP_INVALID)                \
        wt2 = FLOAT_QNAN32;                                        \
    return wt2;                                                    \
}                                                                  \
                                                                   \
uint64_t helper_float_ ## name ## _ps(uint64_t fdt0, uint64_t fdt1)    \
{                                                                  \
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;                             \
    uint32_t fsth0 = fdt0 >> 32;                                   \
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;                             \
    uint32_t fsth1 = fdt1 >> 32;                                   \
    uint32_t wt2;                                                  \
    uint32_t wth2;                                                 \
                                                                   \
    set_float_exception_flags(0, &env->active_fpu.fp_status);            \
    wt2 = float32_ ## name (fst0, fst1, &env->active_fpu.fp_status);     \
    wth2 = float32_ ## name (fsth0, fsth1, &env->active_fpu.fp_status);  \
    update_fcr31();                                                \
    if (GET_FP_CAUSE(env->active_fpu.fcr31) & FP_INVALID) {              \
        wt2 = FLOAT_QNAN32;                                        \
        wth2 = FLOAT_QNAN32;                                       \
    }                                                              \
    return ((uint64_t)wth2 << 32) | wt2;                           \
}

FLOAT_BINOP(add)
FLOAT_BINOP(sub)
FLOAT_BINOP(mul)
FLOAT_BINOP(div)
#undef FLOAT_BINOP

/* ternary operations */
#define FLOAT_TERNOP(name1, name2)                                        \
uint64_t helper_float_ ## name1 ## name2 ## _d(uint64_t fdt0, uint64_t fdt1,  \
                                           uint64_t fdt2)                 \
{                                                                         \
    fdt0 = float64_ ## name1 (fdt0, fdt1, &env->active_fpu.fp_status);          \
    return float64_ ## name2 (fdt0, fdt2, &env->active_fpu.fp_status);          \
}                                                                         \
                                                                          \
uint32_t helper_float_ ## name1 ## name2 ## _s(uint32_t fst0, uint32_t fst1,  \
                                           uint32_t fst2)                 \
{                                                                         \
    fst0 = float32_ ## name1 (fst0, fst1, &env->active_fpu.fp_status);          \
    return float32_ ## name2 (fst0, fst2, &env->active_fpu.fp_status);          \
}                                                                         \
                                                                          \
uint64_t helper_float_ ## name1 ## name2 ## _ps(uint64_t fdt0, uint64_t fdt1, \
                                            uint64_t fdt2)                \
{                                                                         \
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;                                    \
    uint32_t fsth0 = fdt0 >> 32;                                          \
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;                                    \
    uint32_t fsth1 = fdt1 >> 32;                                          \
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;                                    \
    uint32_t fsth2 = fdt2 >> 32;                                          \
                                                                          \
    fst0 = float32_ ## name1 (fst0, fst1, &env->active_fpu.fp_status);          \
    fsth0 = float32_ ## name1 (fsth0, fsth1, &env->active_fpu.fp_status);       \
    fst2 = float32_ ## name2 (fst0, fst2, &env->active_fpu.fp_status);          \
    fsth2 = float32_ ## name2 (fsth0, fsth2, &env->active_fpu.fp_status);       \
    return ((uint64_t)fsth2 << 32) | fst2;                                \
}

FLOAT_TERNOP(mul, add)
FLOAT_TERNOP(mul, sub)
#undef FLOAT_TERNOP

/* negated ternary operations */
#define FLOAT_NTERNOP(name1, name2)                                       \
uint64_t helper_float_n ## name1 ## name2 ## _d(uint64_t fdt0, uint64_t fdt1, \
                                           uint64_t fdt2)                 \
{                                                                         \
    fdt0 = float64_ ## name1 (fdt0, fdt1, &env->active_fpu.fp_status);          \
    fdt2 = float64_ ## name2 (fdt0, fdt2, &env->active_fpu.fp_status);          \
    return float64_chs(fdt2);                                             \
}                                                                         \
                                                                          \
uint32_t helper_float_n ## name1 ## name2 ## _s(uint32_t fst0, uint32_t fst1, \
                                           uint32_t fst2)                 \
{                                                                         \
    fst0 = float32_ ## name1 (fst0, fst1, &env->active_fpu.fp_status);          \
    fst2 = float32_ ## name2 (fst0, fst2, &env->active_fpu.fp_status);          \
    return float32_chs(fst2);                                             \
}                                                                         \
                                                                          \
uint64_t helper_float_n ## name1 ## name2 ## _ps(uint64_t fdt0, uint64_t fdt1,\
                                           uint64_t fdt2)                 \
{                                                                         \
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;                                    \
    uint32_t fsth0 = fdt0 >> 32;                                          \
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;                                    \
    uint32_t fsth1 = fdt1 >> 32;                                          \
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;                                    \
    uint32_t fsth2 = fdt2 >> 32;                                          \
                                                                          \
    fst0 = float32_ ## name1 (fst0, fst1, &env->active_fpu.fp_status);          \
    fsth0 = float32_ ## name1 (fsth0, fsth1, &env->active_fpu.fp_status);       \
    fst2 = float32_ ## name2 (fst0, fst2, &env->active_fpu.fp_status);          \
    fsth2 = float32_ ## name2 (fsth0, fsth2, &env->active_fpu.fp_status);       \
    fst2 = float32_chs(fst2);                                             \
    fsth2 = float32_chs(fsth2);                                           \
    return ((uint64_t)fsth2 << 32) | fst2;                                \
}

FLOAT_NTERNOP(mul, add)
FLOAT_NTERNOP(mul, sub)
#undef FLOAT_NTERNOP

/* MIPS specific binary operations */
uint64_t helper_float_recip2_d(uint64_t fdt0, uint64_t fdt2)
{
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_mul(fdt0, fdt2, &env->active_fpu.fp_status);
    fdt2 = float64_chs(float64_sub(fdt2, FLOAT_ONE64, &env->active_fpu.fp_status));
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_recip2_s(uint32_t fst0, uint32_t fst2)
{
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_sub(fst2, FLOAT_ONE32, &env->active_fpu.fp_status));
    update_fcr31();
    return fst2;
}

uint64_t helper_float_recip2_ps(uint64_t fdt0, uint64_t fdt2)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;
    uint32_t fsth2 = fdt2 >> 32;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_mul(fsth0, fsth2, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_sub(fst2, FLOAT_ONE32, &env->active_fpu.fp_status));
    fsth2 = float32_chs(float32_sub(fsth2, FLOAT_ONE32, &env->active_fpu.fp_status));
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_rsqrt2_d(uint64_t fdt0, uint64_t fdt2)
{
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fdt2 = float64_mul(fdt0, fdt2, &env->active_fpu.fp_status);
    fdt2 = float64_sub(fdt2, FLOAT_ONE64, &env->active_fpu.fp_status);
    fdt2 = float64_chs(float64_div(fdt2, FLOAT_TWO64, &env->active_fpu.fp_status));
    update_fcr31();
    return fdt2;
}

uint32_t helper_float_rsqrt2_s(uint32_t fst0, uint32_t fst2)
{
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fst2 = float32_sub(fst2, FLOAT_ONE32, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_div(fst2, FLOAT_TWO32, &env->active_fpu.fp_status));
    update_fcr31();
    return fst2;
}

uint64_t helper_float_rsqrt2_ps(uint64_t fdt0, uint64_t fdt2)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;
    uint32_t fsth2 = fdt2 >> 32;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_mul(fsth0, fsth2, &env->active_fpu.fp_status);
    fst2 = float32_sub(fst2, FLOAT_ONE32, &env->active_fpu.fp_status);
    fsth2 = float32_sub(fsth2, FLOAT_ONE32, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_div(fst2, FLOAT_TWO32, &env->active_fpu.fp_status));
    fsth2 = float32_chs(float32_div(fsth2, FLOAT_TWO32, &env->active_fpu.fp_status));
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_addr_ps(uint64_t fdt0, uint64_t fdt1)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;
    uint32_t fsth1 = fdt1 >> 32;
    uint32_t fst2;
    uint32_t fsth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_add (fst0, fsth0, &env->active_fpu.fp_status);
    fsth2 = float32_add (fst1, fsth1, &env->active_fpu.fp_status);
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_mulr_ps(uint64_t fdt0, uint64_t fdt1)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;
    uint32_t fsth1 = fdt1 >> 32;
    uint32_t fst2;
    uint32_t fsth2;

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    fst2 = float32_mul (fst0, fsth0, &env->active_fpu.fp_status);
    fsth2 = float32_mul (fst1, fsth1, &env->active_fpu.fp_status);
    update_fcr31();
    return ((uint64_t)fsth2 << 32) | fst2;
}

/* compare operations */
#define FOP_COND_D(op, cond)                                   \
void helper_cmp_d_ ## op (uint64_t fdt0, uint64_t fdt1, int cc)    \
{                                                              \
    int c;                                                     \
    set_float_exception_flags(0, &env->active_fpu.fp_status);  \
    c = cond;                                                  \
    update_fcr31();                                            \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}                                                              \
void helper_cmpabs_d_ ## op (uint64_t fdt0, uint64_t fdt1, int cc) \
{                                                              \
    int c;                                                     \
    set_float_exception_flags(0, &env->active_fpu.fp_status);  \
    fdt0 = float64_abs(fdt0);                                  \
    fdt1 = float64_abs(fdt1);                                  \
    c = cond;                                                  \
    update_fcr31();                                            \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}

/* NOTE: the comma operator will make "cond" to eval to false,
 * but float64_unordered_quiet() is still called. */
FOP_COND_D(f,   (float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status), 0))
FOP_COND_D(un,  float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status))
FOP_COND_D(eq,  float64_eq_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ueq, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_eq_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(olt, float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ult, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ole, float64_le_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ule, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_le_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
/* NOTE: the comma operator will make "cond" to eval to false,
 * but float64_unordered() is still called. */
FOP_COND_D(sf,  (float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status), 0))
FOP_COND_D(ngle, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status))
FOP_COND_D(seq, float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ngl, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(lt,  float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(nge, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(le,  float64_le(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ngt, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_le(fdt0, fdt1, &env->active_fpu.fp_status))

#define FOP_COND_S(op, cond)                                   \
void helper_cmp_s_ ## op (uint32_t fst0, uint32_t fst1, int cc)    \
{                                                              \
    int c;                                                     \
    set_float_exception_flags(0, &env->active_fpu.fp_status);  \
    c = cond;                                                  \
    update_fcr31();                                            \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}                                                              \
void helper_cmpabs_s_ ## op (uint32_t fst0, uint32_t fst1, int cc) \
{                                                              \
    int c;                                                     \
    set_float_exception_flags(0, &env->active_fpu.fp_status);  \
    fst0 = float32_abs(fst0);                                  \
    fst1 = float32_abs(fst1);                                  \
    c = cond;                                                  \
    update_fcr31();                                            \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}

/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered_quiet() is still called. */
FOP_COND_S(f,   (float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status), 0))
FOP_COND_S(un,  float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status))
FOP_COND_S(eq,  float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ueq, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)  || float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(olt, float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ult, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)  || float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ole, float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ule, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)  || float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status))
/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered() is still called. */
FOP_COND_S(sf,  (float32_unordered(fst1, fst0, &env->active_fpu.fp_status), 0))
FOP_COND_S(ngle, float32_unordered(fst1, fst0, &env->active_fpu.fp_status))
FOP_COND_S(seq, float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ngl, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(lt,  float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(nge, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(le,  float32_le(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ngt, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_le(fst0, fst1, &env->active_fpu.fp_status))

#define FOP_COND_PS(op, condl, condh)                           \
void helper_cmp_ps_ ## op (uint64_t fdt0, uint64_t fdt1, int cc)    \
{                                                               \
    uint32_t fst0, fsth0, fst1, fsth1;                          \
    int ch, cl;                                                 \
    set_float_exception_flags(0, &env->active_fpu.fp_status);   \
    fst0 = fdt0 & 0XFFFFFFFF;                                   \
    fsth0 = fdt0 >> 32;                                         \
    fst1 = fdt1 & 0XFFFFFFFF;                                   \
    fsth1 = fdt1 >> 32;                                         \
    cl = condl;                                                 \
    ch = condh;                                                 \
    update_fcr31();                                             \
    if (cl)                                                     \
        SET_FP_COND(cc, env->active_fpu);                       \
    else                                                        \
        CLEAR_FP_COND(cc, env->active_fpu);                     \
    if (ch)                                                     \
        SET_FP_COND(cc + 1, env->active_fpu);                   \
    else                                                        \
        CLEAR_FP_COND(cc + 1, env->active_fpu);                 \
}                                                               \
void helper_cmpabs_ps_ ## op (uint64_t fdt0, uint64_t fdt1, int cc) \
{                                                               \
    uint32_t fst0, fsth0, fst1, fsth1;                          \
    int ch, cl;                                                 \
    fst0 = float32_abs(fdt0 & 0XFFFFFFFF);                      \
    fsth0 = float32_abs(fdt0 >> 32);                            \
    fst1 = float32_abs(fdt1 & 0XFFFFFFFF);                      \
    fsth1 = float32_abs(fdt1 >> 32);                            \
    cl = condl;                                                 \
    ch = condh;                                                 \
    update_fcr31();                                             \
    if (cl)                                                     \
        SET_FP_COND(cc, env->active_fpu);                       \
    else                                                        \
        CLEAR_FP_COND(cc, env->active_fpu);                     \
    if (ch)                                                     \
        SET_FP_COND(cc + 1, env->active_fpu);                   \
    else                                                        \
        CLEAR_FP_COND(cc + 1, env->active_fpu);                 \
}

/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered_quiet() is still called. */
FOP_COND_PS(f,   (float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status), 0),
                 (float32_unordered_quiet(fsth1, fsth0, &env->active_fpu.fp_status), 0))
FOP_COND_PS(un,  float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status),
                 float32_unordered_quiet(fsth1, fsth0, &env->active_fpu.fp_status))
FOP_COND_PS(eq,  float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_eq_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ueq, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)    || float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered_quiet(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_eq_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(olt, float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_lt_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ult, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)    || float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered_quiet(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_lt_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ole, float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_le_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ule, float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status)    || float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered_quiet(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_le_quiet(fsth0, fsth1, &env->active_fpu.fp_status))
/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered() is still called. */
FOP_COND_PS(sf,  (float32_unordered(fst1, fst0, &env->active_fpu.fp_status), 0),
                 (float32_unordered(fsth1, fsth0, &env->active_fpu.fp_status), 0))
FOP_COND_PS(ngle, float32_unordered(fst1, fst0, &env->active_fpu.fp_status),
                 float32_unordered(fsth1, fsth0, &env->active_fpu.fp_status))
FOP_COND_PS(seq, float32_eq(fst0, fst1, &env->active_fpu.fp_status),
                 float32_eq(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ngl, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)    || float32_eq(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_eq(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(lt,  float32_lt(fst0, fst1, &env->active_fpu.fp_status),
                 float32_lt(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(nge, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)    || float32_lt(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_lt(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(le,  float32_le(fst0, fst1, &env->active_fpu.fp_status),
                 float32_le(fsth0, fsth1, &env->active_fpu.fp_status))
FOP_COND_PS(ngt, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)    || float32_le(fst0, fst1, &env->active_fpu.fp_status),
                 float32_unordered(fsth1, fsth0, &env->active_fpu.fp_status)  || float32_le(fsth0, fsth1, &env->active_fpu.fp_status))



/*
 *  MSA
 */

#define DEBUG_MSACSR 1

/* Data format and vector length unpacking */
#define WRLEN(wrlen_df) (wrlen_df >> 2)
#define DF(wrlen_df) (wrlen_df & 0x03)

#define DF_BYTE   0
#define DF_HALF   1
#define DF_WORD   2
#define DF_DOUBLE 3

#define DF_FLOAT_WORD   0
#define DF_FLOAT_DOUBLE 1

static void msa_check_index(uint32_t df, uint32_t n, uint32_t wrlen) {
    switch (df) {
    case DF_BYTE: /* b */
        if (n > wrlen / 8 - 1) {
            helper_raise_exception(EXCP_RI);
        }
        break;
    case DF_HALF: /* h */
        if (n > wrlen / 16 - 1) {
            helper_raise_exception(EXCP_RI);
        }
        break;
    case DF_WORD: /* w */
        if (n > wrlen / 32 - 1) {
            helper_raise_exception(EXCP_RI);
        }
        break;
    case DF_DOUBLE: /* d */
        if (n > wrlen / 64 - 1) {
            helper_raise_exception(EXCP_RI);
        }
        break;
    default:
        /* shouldn't get here */
      assert(0);
    }
}


/* Data format min and max values */
#define DF_BITS(df) (1 << ((df) + 3))

#define DF_MAX_INT(df)  (int64_t)((1LL << (DF_BITS(df) - 1)) - 1)
#define M_MAX_INT(m)    (int64_t)((1LL << ((m)         - 1)) - 1)

#define DF_MIN_INT(df)  (int64_t)(-(1LL << (DF_BITS(df) - 1)))
#define M_MIN_INT(m)    (int64_t)(-(1LL << ((m)         - 1)))

#define DF_MAX_UINT(df) (uint64_t)(-1ULL >> (64 - DF_BITS(df)))
#define M_MAX_UINT(m)   (uint64_t)(-1ULL >> (64 - (m)))

/* Data format bit position and unsigned values */
#define BIT_POSITION(x, df) ((uint64_t)(x) % DF_BITS(df))

#define UNSIGNED(x, df) ((x) & DF_MAX_UINT(df))
#define SIGNED(x, df)                                                   \
    ((((int64_t)x) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)))

/* Element-by-element access macros */
#define DF_ELEMENTS(df, wrlen) (wrlen / DF_BITS(df))

#define  B(pwr, i) (((wr_t *)pwr)->b[i])
#define BR(pwr, i) (((wr_t *)pwr)->b[i])
#define BL(pwr, i) (((wr_t *)pwr)->b[i + wrlen/16])

#define ALL_B_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 8; i--;)

#define  H(pwr, i) (((wr_t *)pwr)->h[i])
#define HR(pwr, i) (((wr_t *)pwr)->h[i])
#define HL(pwr, i) (((wr_t *)pwr)->h[i + wrlen/32])

#define ALL_H_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 16; i--;)

#define  W(pwr, i) (((wr_t *)pwr)->w[i])
#define WR(pwr, i) (((wr_t *)pwr)->w[i])
#define WL(pwr, i) (((wr_t *)pwr)->w[i + wrlen/64])

#define ALL_W_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 32; i--;)

#define  D(pwr, i) (((wr_t *)pwr)->d[i])
#define DR(pwr, i) (((wr_t *)pwr)->d[i])
#define DL(pwr, i) (((wr_t *)pwr)->d[i + wrlen/128])

#define ALL_D_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 64; i--;)

#define Q(pwr, i) (((wr_t *)pwr)->q[i])
#define ALL_Q_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 128; i--;)

#define DONE_ALL_ELEMENTS                       \
    } while (0)

/*
 *  ADD_A, ADDV, SUBV
 */

int64_t helper_add_a_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 + abs_arg2;
}

int64_t helper_addv_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 + arg2;
}

int64_t helper_subv_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 - arg2;
}


/*
 *  ADDS_A, ADDS_S, ADDS_U, SUBS_S, SUBS_U, SUBSUU_S, SUBSUS_U
 */

int64_t helper_adds_a_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t max_int = (uint64_t)DF_MAX_INT(df);
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    if (abs_arg1 > max_int || abs_arg2 > max_int) {
        return (int64_t)max_int;
    } else {
        return (abs_arg1 < max_int - abs_arg2) ? abs_arg1 + abs_arg2 : max_int;
    }
}

int64_t helper_adds_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);

    if (arg1 < 0) {
        return (min_int - arg1 < arg2) ? arg1 + arg2 : min_int;
    } else {
        return (arg2 < max_int - arg1) ? arg1 + arg2 : max_int;
    }
}

uint64_t helper_adds_u_df(uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t max_uint = DF_MAX_UINT(df);

    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return (u_arg1 < max_uint - u_arg2) ? u_arg1 + u_arg2 : max_uint;
}

int64_t helper_subs_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);

    if (arg2 > 0) {
        return (min_int + arg2 < arg1) ? arg1 - arg2 : min_int;
    } else {
        return (arg1 < max_int + arg2) ? arg1 - arg2 : max_int;
    }
}


int64_t helper_subs_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return (u_arg1 > u_arg2) ? u_arg1 - u_arg2 : 0;
}


int64_t helper_subsuu_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);

    if (u_arg1 > u_arg2) {
        return u_arg1 - u_arg2 < (uint64_t)max_int ?
            (int64_t)(u_arg1 - u_arg2) :
            max_int;
    } else {
        return u_arg2 - u_arg1 < (uint64_t)(-min_int) ?
            (int64_t)(u_arg1 - u_arg2) :
            min_int;
    }
}

int64_t helper_subsus_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t max_uint = DF_MAX_UINT(df);

    if (arg2 >= 0) {
        uint64_t u_arg2 = (uint64_t)arg2;
        return (u_arg1 > u_arg2) ?
            (int64_t)(u_arg1 - u_arg2) :
            0;
    }
    else {
        uint64_t u_arg2 = (uint64_t)(-arg2);
        return (u_arg1 < max_uint - u_arg2) ?
            (int64_t)(u_arg1 + u_arg2) :
            (int64_t)max_uint;
    }
}



/*
 *  AND_V, ANDI_B, OR_V, ORBI_B, NOR_V, NORBI_B, XOR_V, XORI_B
 */

void helper_and_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) & D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_andi_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) & arg2;
    } DONE_ALL_ELEMENTS;
}

void helper_or_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) | D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_ori_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) | arg2;
    } DONE_ALL_ELEMENTS;
}

void helper_nor_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = ~(D(pws, i) | D(pwt, i));
    } DONE_ALL_ELEMENTS;
}

void helper_nori_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = ~(B(pws, i) | arg2);
    } DONE_ALL_ELEMENTS;
}

void helper_xor_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) ^ D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_xori_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) ^ arg2;
    } DONE_ALL_ELEMENTS;
}



/*
 *  ASUB_S, ASUB_U
 */

int64_t helper_asub_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed compare */
    return (arg1 < arg2) ?
        (uint64_t)(arg2 - arg1) : (uint64_t)(arg1 - arg2);
}

uint64_t helper_asub_u_df(uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    /* unsigned compare */
    return (u_arg1 < u_arg2) ?
        (uint64_t)(u_arg2 - u_arg1) : (uint64_t)(u_arg1 - u_arg2);
}


/*
 *  AVE_S, AVE_U
 */

int64_t helper_ave_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + (arg1 & arg2 & 1);

}

uint64_t helper_ave_u_df(uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + (u_arg1 & u_arg2 & 1);
}


/*
 *  AVER_S, AVER_U
 */

int64_t helper_aver_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + ((arg1 | arg2) & 1);

}

uint64_t helper_aver_u_df(uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + ((u_arg1 | u_arg2) & 1);
}


/*
 *  BCLR, BNEG, BSET
 */

int64_t helper_bclr_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 & (~(1LL << b_arg2)), df);
}

int64_t helper_bclri_df(int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bclr_df(arg1, arg2, df);
}

int64_t helper_bneg_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 ^ (1LL << b_arg2), df);
}

int64_t helper_bnegi_df(int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bneg_df(arg1, arg2, df);
}

int64_t helper_bset_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 | (1LL << b_arg2), df);
}

int64_t helper_bseti_df(int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bset_df(arg1, arg2, df);
}


/*
 *  BINSL, BINSR
 */

int64_t helper_binsl_df(int64_t dest,
                        int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_dest = UNSIGNED(dest, df);

    int32_t sh_d = BIT_POSITION(arg2, df) + 1;
    int32_t sh_a = DF_BITS(df) - sh_d;

    if (sh_d == DF_BITS(df)) {
        return u_arg1;
    } else {
        return UNSIGNED(UNSIGNED(u_dest << sh_d, df) >> sh_d, df) |
               UNSIGNED(UNSIGNED(u_arg1 >> sh_a, df) << sh_a, df);
    }
}

int64_t helper_binsli_df(int64_t dest,
                         int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_binsl_df(dest, arg1, arg2, df);
}

int64_t helper_binsr_df(int64_t dest,
                        int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_dest = UNSIGNED(dest, df);

    int32_t sh_d = BIT_POSITION(arg2, df) + 1;
    int32_t sh_a = DF_BITS(df) - sh_d;

    if (sh_d == DF_BITS(df)) {
        return u_arg1;
    } else {
        return UNSIGNED(UNSIGNED(u_dest >> sh_d, df) << sh_d, df) |
               UNSIGNED(UNSIGNED(u_arg1 << sh_a, df) >> sh_a, df);
    }
}

int64_t helper_binsri_df(int64_t dest,
                        int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_binsr_df(dest, arg1, arg2, df);
}


/*
 *  BMNZ
 */

#define BIT_MOVE_IF_NOT_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED(((dest & (~arg2)) | (arg1 & arg2)), df)

void helper_bmnz_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_NOT_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bmnzi_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_NOT_ZERO(B(pwd, i), B(pws, i), arg2, DF_BYTE);
    } DONE_ALL_ELEMENTS;
}


/*
 *  BMZ
 */

#define BIT_MOVE_IF_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED((dest & arg2) | (arg1 & (~arg2)), df)

void helper_bmz_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bmzi_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_ZERO(B(pwd, i), B(pws, i), arg2, DF_BYTE);
    } DONE_ALL_ELEMENTS;
}


/*
 *  BSEL
 */

#define BIT_SELECT(dest, arg1, arg2, df) \
            dest = UNSIGNED((arg1 & (~dest)) | (arg2 & dest), df)

void helper_bsel_v(void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_SELECT(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bseli_b(void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        BIT_SELECT(B(pwd, i), B(pws, i), arg2, DF_BYTE);
    } DONE_ALL_ELEMENTS;
}


/*
 *  BNZ, BZ
 */

uint32_t helper_bnz_df(void *p_arg, uint32_t df, uint32_t wrlen)
{
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_B_ELEMENTS(i, wrlen) {
            if (B(p_arg, i) == 0) {
                return 0;
            }
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_H_ELEMENTS(i, wrlen) {
            if (H(p_arg, i) == 0) {
                return 0;
            }
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_W_ELEMENTS(i, wrlen) {
            if (W(p_arg, i) == 0) {
                return 0;
            }
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_D_ELEMENTS(i, wrlen) {
            if (D(p_arg, i) == 0) {
                return 0;
            }
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    return 1;
}

uint32_t helper_bz_df(void *p_arg, uint32_t df, uint32_t wrlen)
{
    return !helper_bnz_df(p_arg, df, wrlen);
}

uint32_t helper_bnz_v(void *p_arg, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        if (D(p_arg, i) != 0) {
            return 1;
        }
    } DONE_ALL_ELEMENTS;

    return 0;
}

uint32_t helper_bz_v(void *p_arg, uint32_t wrlen)
{
    return !helper_bnz_v(p_arg, wrlen);
}


/*
 *  CEQ, CLE_S, CLE_U, CLT_S, CLT_U
 */

int64_t helper_ceq_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 == arg2 ? -1 : 0;
}

int64_t helper_cle_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 <= arg2 ? -1 : 0;
}

int64_t helper_cle_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 <= u_arg2 ? -1 : 0;
}

int64_t helper_clt_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 < arg2 ? -1 : 0;
}

int64_t helper_clt_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 < u_arg2 ? -1 : 0;
}


/*
 *  HADD_S, HADD_U, HSUB_S, HSUB_U,
 *  DOTP_S, DOTP_U, DPADD_S, DPADD_U, DPSUB_S, DPSUB_U
 */

#define SIGNED_EVEN(a, df) \
        ((((int64_t)(a)) << (64 - DF_BITS(df)/2)) >> (64 - DF_BITS(df)/2))
#define UNSIGNED_EVEN(a, df) \
        ((((uint64_t)(a)) << (64 - DF_BITS(df)/2)) >> (64 - DF_BITS(df)/2))

#define SIGNED_ODD(a, df) \
        ((((int64_t)(a)) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)/2))
#define UNSIGNED_ODD(a, df) \
        ((((uint64_t)(a)) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)/2))

#define SIGNED_EXTRACT(e, o, a, df)             \
    int64_t e = SIGNED_EVEN(a, df);             \
    int64_t o = SIGNED_ODD(a, df);

#define UNSIGNED_EXTRACT(e, o, a, df)           \
    int64_t e = UNSIGNED_EVEN(a, df);           \
    int64_t o = UNSIGNED_ODD(a, df);


int64_t helper_hadd_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return SIGNED_ODD(arg1, df) + SIGNED_EVEN(arg2, df);
}


int64_t helper_hadd_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return UNSIGNED_ODD(arg1, df) + UNSIGNED_EVEN(arg2, df);
}


int64_t helper_hsub_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return SIGNED_ODD(arg1, df) - SIGNED_EVEN(arg2, df);
}


int64_t helper_hsub_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return UNSIGNED_ODD(arg1, df) - UNSIGNED_EVEN(arg2, df);
}


int64_t helper_dotp_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

int64_t helper_dotp_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}


int64_t helper_dpadd_s_df(int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    if (df == DF_BYTE)
        helper_raise_exception(EXCP_RI);

    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}


int64_t helper_dpadd_u_df(int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    if (df == DF_BYTE)
        helper_raise_exception(EXCP_RI);

    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

int64_t helper_dpsub_s_df(int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    if (df == DF_BYTE)
        helper_raise_exception(EXCP_RI);

    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

int64_t helper_dpsub_u_df(int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    if (df == DF_BYTE)
        helper_raise_exception(EXCP_RI);

    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}



/*
 *  ILVEV, ILVOD, ILVL, ILVR, PCKEV, PCKOD, VSHF
 */

#define WRLEN(wrlen_df) (wrlen_df >> 2)
#define DF(wrlen_df) (wrlen_df & 0x03)

void helper_ilvev_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            B(pwx, 2*i)   = B(pwt, 2*i);
            B(pwx, 2*i+1) = B(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            H(pwx, 2*i)   = H(pwt, 2*i);
            H(pwx, 2*i+1) = H(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            W(pwx, 2*i)   = W(pwt, 2*i);
            W(pwx, 2*i+1) = W(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            D(pwx, 2*i)   = D(pwt, 2*i);
            D(pwx, 2*i+1) = D(pws, 2*i);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


void helper_ilvod_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            B(pwx, 2*i)   = B(pwt, 2*i+1);
            B(pwx, 2*i+1) = B(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            H(pwx, 2*i)   = H(pwt, 2*i+1);
            H(pwx, 2*i+1) = H(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            W(pwx, 2*i)   = W(pwt, 2*i+1);
            W(pwx, 2*i+1) = W(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            D(pwx, 2*i)   = D(pwt, 2*i+1);
            D(pwx, 2*i+1) = D(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


void helper_ilvl_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            B(pwx, 2*i)   = BL(pwt, i);
            B(pwx, 2*i+1) = BL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            H(pwx, 2*i)   = HL(pwt, i);
            H(pwx, 2*i+1) = HL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            W(pwx, 2*i)   = WL(pwt, i);
            W(pwx, 2*i+1) = WL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            D(pwx, 2*i)   = DL(pwt, i);
            D(pwx, 2*i+1) = DL(pws, i);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


void helper_ilvr_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            B(pwx, 2*i)   = BR(pwt, i);
            B(pwx, 2*i+1) = BR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            H(pwx, 2*i)   = HR(pwt, i);
            H(pwx, 2*i+1) = HR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            W(pwx, 2*i)   = WR(pwt, i);
            W(pwx, 2*i+1) = WR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            D(pwx, 2*i)   = DR(pwt, i);
            D(pwx, 2*i+1) = DR(pws, i);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


void helper_pckev_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            BR(pwx, i) = B(pwt, 2*i);
            BL(pwx, i) = B(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            HR(pwx, i) = H(pwt, 2*i);
            HL(pwx, i) = H(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            WR(pwx, i) = W(pwt, 2*i);
            WL(pwx, i) = W(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            DR(pwx, i) = D(pwt, 2*i);
            DL(pwx, i) = D(pws, 2*i);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


void helper_pckod_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, wrlen) {
            BR(pwx, i) = B(pwt, 2*i+1);
            BL(pwx, i) = B(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, wrlen) {
            HR(pwx, i) = H(pwt, 2*i+1);
            HL(pwx, i) = H(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, wrlen) {
            WR(pwx, i) = W(pwt, 2*i+1);
            WL(pwx, i) = W(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, wrlen) {
            DR(pwx, i) = D(pwt, 2*i+1);
            DL(pwx, i) = D(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}

void helper_vshf_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);
    uint32_t n = wrlen / DF_BITS(df);
    uint32_t k;

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_B_ELEMENTS(i, wrlen) {
            k = (B(pwd, i) & 0x3f) % (2 * n);
            B(pwx, i) =
                (B(pwd, i) & 0xc0) ? 0 : k < n ? B(pwt, k) : B(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        /* half data format */
        ALL_H_ELEMENTS(i, wrlen) {
            k = (H(pwd, i) & 0x3f) % (2 * n);
            H(pwx, i) =
                (H(pwd, i) & 0xc0) ? 0 : k < n ? H(pwt, k) : H(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        /* word data format */
        ALL_W_ELEMENTS(i, wrlen) {
            k = (W(pwd, i) & 0x3f) % (2 * n);
            W(pwx, i) =
                (W(pwd, i) & 0xc0) ? 0 : k < n ? W(pwt, k) : W(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        /* double data format */
        ALL_D_ELEMENTS(i, wrlen) {
            k = (D(pwd, i) & 0x3f) % (2 * n);
            D(pwx, i) =
                (D(pwd, i) & 0xc0) ? 0 : k < n ? D(pwt, k) : D(pws, k - n);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}


/*
 *  SHF
 */

#define SHF_POS(i, imm) ((i & 0xfc) + ((imm >> (2 * (i & 0x03))) & 0x03))

void helper_shf_b(void *pwd, void *pws, uint32_t imm, uint32_t wrlen)
{
    wr_t wx, *pwx = &wx;

    ALL_B_ELEMENTS(i, wrlen) {
        B(pwx, i) = B(pws, SHF_POS(i, imm));
    } DONE_ALL_ELEMENTS;

    helper_move_v(pwd, &wx, wrlen);
}

void helper_shf_h(void *pwd, void *pws, uint32_t imm, uint32_t wrlen)
{
    wr_t wx, *pwx = &wx;

    ALL_H_ELEMENTS(i, wrlen) {
        H(pwx, i) = H(pws, SHF_POS(i, imm));
    } DONE_ALL_ELEMENTS;

    helper_move_v(pwd, &wx, wrlen);
}

void helper_shf_w(void *pwd, void *pws, uint32_t imm, uint32_t wrlen)
{
    wr_t wx, *pwx = &wx;

    ALL_W_ELEMENTS(i, wrlen) {
        W(pwx, i) = W(pws, SHF_POS(i, imm));
    } DONE_ALL_ELEMENTS;

    helper_move_v(pwd, &wx, wrlen);
}


/*
 *  MADDV, MSUBV
 */

int64_t helper_maddv_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    return dest + arg1 * arg2;
}

int64_t helper_msubv_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    return dest - arg1 * arg2;
}


/*
 *  MAX, MIN
 */

int64_t helper_max_a_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 > abs_arg2 ? arg1 : arg2;
}


int64_t helper_max_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 > arg2 ? arg1 : arg2;
}


int64_t helper_max_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 > u_arg2 ? arg1 : arg2;
}


int64_t helper_min_a_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 < abs_arg2 ? arg1 : arg2;
}


int64_t helper_min_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 < arg2 ? arg1 : arg2;
}


int64_t helper_min_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 < u_arg2 ? arg1 : arg2;
}


/*
 *  SPLAT, and MOVE_V
 */

void helper_splat_df(void *pwd, void *pws, uint32_t rt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    uint32_t n = rt % DF_ELEMENTS(df, wrlen);

    msa_check_index(df, n, wrlen);

    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, wrlen) {
            B(pwd, i)   = B(pws, n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        ALL_H_ELEMENTS(i, wrlen) {
            H(pwd, i)   = H(pws, n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            W(pwd, i)   = W(pws, n);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            D(pwd, i)   = D(pws, n);
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}

void helper_move_v(void *pwd, void *pws, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i);
    } DONE_ALL_ELEMENTS;
}


/*
 *  LDI, FILL, INSERT, INSVE
 */
void helper_ldi_df(void *pwd, uint32_t df, uint32_t s10, uint32_t wrlen)
{
    int64_t s64 = ((int64_t)s10 << 54) >> 54;

    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, wrlen) {
            B(pwd, i)   = (int8_t)s10;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        ALL_H_ELEMENTS(i, wrlen) {
            H(pwd, i)   = (int16_t)s64;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            W(pwd, i)   = (int32_t)s64;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            D(pwd, i)   = s64;
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}

void helper_fill_df(void *pwd, uint32_t rs, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, wrlen) {
            B(pwd, i)   = (int8_t)rs;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_HALF:
        ALL_H_ELEMENTS(i, wrlen) {
            H(pwd, i)   = (int16_t)rs;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            W(pwd, i)   = (int32_t)rs;
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            D(pwd, i)   = (int64_t)rs;
        } DONE_ALL_ELEMENTS;
       break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}

void helper_insert_df(void *pwd, uint32_t rs, uint32_t n, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    msa_check_index(df, n, wrlen);

    switch (df) {
    case DF_BYTE:
        B(pwd, n)   = (int8_t)rs;
        break;

    case DF_HALF:
        H(pwd, n)   = (int16_t)rs;
        break;

    case DF_WORD:
        W(pwd, n)   = (int32_t)rs;
        break;

    case DF_DOUBLE:
        D(pwd, n)   = (int64_t)rs;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}

void helper_insve_df(void *pwd, void *pws, uint32_t n, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    msa_check_index(df, n, wrlen);

    switch (df) {
    case DF_BYTE:
        B(pwd, n)   = (int8_t)B(pws, 0);
        break;

    case DF_HALF:
        H(pwd, n)   = (int16_t)H(pws, 0);
        break;

    case DF_WORD:
        W(pwd, n)   = (int32_t)W(pws, 0);
        break;

    case DF_DOUBLE:
        D(pwd, n)   = (int64_t)D(pws, 0);
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}


/*
 *  MULV, DIV_S, DIV_U, MOD_S, MOD_U
 */

int64_t helper_mulv_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 * arg2;
}

int64_t helper_div_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
  if (arg1 == DF_MIN_INT(df) && arg2 == -1)
    return DF_MIN_INT(df);

  return arg2 ? arg1 / arg2 : 0;
}

int64_t helper_div_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg2 ? u_arg1 / u_arg2 : 0;
}

int64_t helper_mod_s_df(int64_t arg1, int64_t arg2, uint32_t df)
{
  if (arg1 == DF_MIN_INT(df) && arg2 == -1)
    return 0;

  return arg2 ? arg1 % arg2 : 0;
}

int64_t helper_mod_u_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg2 ? u_arg1 % u_arg2 : 0;
}


/*
 *  NLZC, NLOC, and PCNT
 */

int64_t helper_nlzc_df(int64_t arg, uint32_t df)
{
    /* Reference: Hacker's Delight, Section 5.3 Counting Leading 0's */

    uint64_t x, y;
    int n, c;

    x = UNSIGNED(arg, df);
    n = DF_BITS(df);
    c = DF_BITS(df) / 2;

    do {
        y = x >> c;
        if (y != 0) {
            n = n - c;
            x = y;
        }
        c = c >> 1;
    } while (c != 0);

    return n - x;
}

int64_t helper_nloc_df(int64_t arg, uint32_t df)
{
    return helper_nlzc_df(UNSIGNED((~arg), df), df);
}


int64_t helper_pcnt_df(int64_t arg, uint32_t df)
{
    /* Reference: Hacker's Delight, Section 5.1 Counting 1-Bits */

    uint64_t x;

    x = UNSIGNED(arg, df);

    x = (x & 0x5555555555555555ULL) + ((x >>  1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >>  2) & 0x3333333333333333ULL);
    x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >>  4) & 0x0F0F0F0F0F0F0F0FULL);
    x = (x & 0x00FF00FF00FF00FFULL) + ((x >>  8) & 0x00FF00FF00FF00FFULL);
    x = (x & 0x0000FFFF0000FFFFULL) + ((x >> 16) & 0x0000FFFF0000FFFFULL);
    x = (x & 0x00000000FFFFFFFFULL) + ((x >> 32));

    return x;
}


/*
 *  SAT
 */

int64_t helper_sat_u_df(int64_t arg, uint32_t m, uint32_t df)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return  u_arg < M_MAX_UINT(m+1) ? u_arg :
                                      M_MAX_UINT(m+1);
}


int64_t helper_sat_s_df(int64_t arg, uint32_t m, uint32_t df)
{
    return arg < M_MIN_INT(m+1) ? M_MIN_INT(m+1) :
                                  arg > M_MAX_INT(m+1) ? M_MAX_INT(m+1) :
                                                         arg;
}


/*
 *  SLL, SRA, SRL
 */

int64_t helper_sll_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 << b_arg2;
}


int64_t helper_slli_df(int64_t arg, uint32_t m, uint32_t df)
{
    return arg << m;
}


int64_t helper_sra_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 >> b_arg2;
}


int64_t helper_srai_df(int64_t arg, uint32_t m, uint32_t df)
{
    return arg >> m;
}


int64_t helper_srl_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return u_arg1 >> b_arg2;
}


int64_t helper_srli_df(int64_t arg, uint32_t m, uint32_t df)
{
    uint64_t u_arg = UNSIGNED(arg, df);

    return u_arg >> m;
}


/*
 *  SRAR, SRLR
 */

int64_t helper_srar_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    if (b_arg2 == 0) {
      return arg1;
    }
    else {
      int64_t r_bit = (arg1 >> (b_arg2 - 1)) & 1;
      return (arg1 >> b_arg2) + r_bit;
    }
}


int64_t helper_srari_df(int64_t arg, uint32_t m, uint32_t df)
{
    if (m == 0) {
      return arg;
    }
    else {
      int64_t r_bit = (arg >> (m - 1)) & 1;
      return (arg >> m) + r_bit;
    }
}


int64_t helper_srlr_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    if (b_arg2 == 0) {
      return u_arg1;
    }
    else {
      uint64_t r_bit = (u_arg1 >> (b_arg2 - 1)) & 1;
      return (u_arg1 >> b_arg2) + r_bit;
    }
}


int64_t helper_srlri_df(int64_t arg, uint32_t m, uint32_t df)
{
    uint64_t u_arg = UNSIGNED(arg, df);

    if (m == 0) {
      return u_arg;
    }
    else {
      uint64_t r_bit = (u_arg >> (m - 1)) & 1;
      return (u_arg >> m) + r_bit;
    }
}


/*
 *  SLD
 */

void helper_sld_df(void *pwd, void *pws, uint32_t rt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    uint32_t n = rt % DF_ELEMENTS(df, wrlen);

    uint8_t v[64];
    uint32_t i, k;

#define CONCATENATE_AND_SLIDE(s, k)             \
    do {                                        \
        for (i = 0; i < s; i++) {               \
            v[i]     = B(pws, s * k + i);       \
            v[i + s] = B(pwd, s * k + i);       \
        }                                       \
        for (i = 0; i < s; i++) {               \
            B(pwd, s * k + i) = v[i + n];       \
        }                                       \
    } while (0)

    msa_check_index(df, n, wrlen);

    switch (df) {
    case DF_BYTE:
        CONCATENATE_AND_SLIDE(wrlen/8, 0);
        break;

    case DF_HALF:
        for (k = 0; k < 2; k++) {
            CONCATENATE_AND_SLIDE(wrlen/16, k);
        }
        break;

    case DF_WORD:
        for (k = 0; k < 4; k++) {
            CONCATENATE_AND_SLIDE(wrlen/32, k);
        }
        break;

    case DF_DOUBLE:
        for (k = 0; k < 8; k++) {
            CONCATENATE_AND_SLIDE(wrlen/64, k);
        }
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }
}


/*
 *  Fixed-point operations
 */

#define GET_SIGN(s, a)                          \
    if (a < 0) {                                \
        s = -s; a = -a;                         \
    }

#define SET_SIGN(s, a)                          \
    if (s < 0) {                                \
        a = -a;                                 \
    }

int64_t helper_mul_q_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2) >> (DF_BITS(df) - 1);
}

int64_t helper_mulr_q_df(int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
}

int64_t helper_madd_q_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = (arg1 * arg2) >> (DF_BITS(df) - 1);
    q_ret = dest + q_prod;

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_maddr_q_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
    q_ret = dest + q_prod;

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_msub_q_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = (arg1 * arg2) >> (DF_BITS(df) - 1);
    q_ret = dest - q_prod;

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_msubr_q_df(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
    q_ret = dest - q_prod;

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


/* MSA helper */
#include "mips_msa_helper_dummy.h"

int64_t helper_load_wr_s64(int wreg, int df, int i)
{
    int wrlen = 128;
    msa_check_index((uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

    switch (df) {
    case DF_BYTE: /* b */
        return env->active_fpu.fpr[wreg].wr.b[i];
    case DF_HALF: /* h */
        return env->active_fpu.fpr[wreg].wr.h[i];
    case DF_WORD: /* w */
        return env->active_fpu.fpr[wreg].wr.w[i];
    case DF_DOUBLE: /* d */
        return env->active_fpu.fpr[wreg].wr.d[i];
    default:
        /* shouldn't get here */
      assert(0);
    }
}

int64_t helper_load_wr_modulo_s64(int wreg, int df, int i)
{
    int wrlen = 128;
    uint32_t n = i % DF_ELEMENTS(df, wrlen);

    return helper_load_wr_s64(wreg, df, n);
}

uint64_t helper_load_wr_i64(int wreg, int df, int i)
{
    int wrlen = 128;
    msa_check_index((uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

    switch (df) {
    case DF_BYTE: /* b */
        return (uint8_t)env->active_fpu.fpr[wreg].wr.b[i];
    case DF_HALF: /* h */
        return (uint16_t)env->active_fpu.fpr[wreg].wr.h[i];
    case DF_WORD: /* w */
        return (uint32_t)env->active_fpu.fpr[wreg].wr.w[i];
    case DF_DOUBLE: /* d */
        return (uint64_t)env->active_fpu.fpr[wreg].wr.d[i];
    default:
        /* shouldn't get here */
      assert(0);
    }
}

uint64_t helper_load_wr_modulo_i64(int wreg, int df, int i)
{
    int wrlen = 128;
    uint32_t n = i % DF_ELEMENTS(df, wrlen);

    return helper_load_wr_i64(wreg, df, n);
}

void helper_store_wr(uint64_t val, int wreg, int df, int i)
{
    int wrlen = 128;
    msa_check_index((uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

    switch (df) {
    case DF_BYTE: /* b */
        env->active_fpu.fpr[wreg].wr.b[i] = (uint8_t)val;
        break;
    case DF_HALF: /* h */
        env->active_fpu.fpr[wreg].wr.h[i] = (uint16_t)val;
        break;
    case DF_WORD: /* w */
        env->active_fpu.fpr[wreg].wr.w[i] = (uint32_t)val;
        break;
    case DF_DOUBLE: /* d */
        env->active_fpu.fpr[wreg].wr.d[i] = (uint64_t)val;
        break;
    default:
        /* shouldn't get here */
      assert(0);
    }

    return;
}

void helper_store_wr_modulo(uint64_t val, int wreg, int df, int i)
{
    int wrlen = 128;
    uint32_t n = i % DF_ELEMENTS(df, wrlen);


    helper_store_wr(val, wreg, df, n);
}





/*
 *  MSA Floating-point operations
 */

static void clear_msacsr_cause(void) {
    SET_FP_CAUSE(env->active_msa.msacsr, 0);
}


static void check_msacsr_cause(void)
{
  if ((GET_FP_CAUSE(env->active_msa.msacsr) &
       (GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED)) == 0) {
    UPDATE_FP_FLAGS(env->active_msa.msacsr,
                    GET_FP_CAUSE(env->active_msa.msacsr));

#if DEBUG_MSACSR
    printf("check_msacsr_cause: MSACSR.Cause 0x%02x, MSACSR.Flags 0x%02x\n",
           GET_FP_CAUSE(env->active_msa.msacsr),
           GET_FP_FLAGS(env->active_msa.msacsr));
#endif
  }
  else {
#if DEBUG_MSACSR
    printf("check_msacsr_cause: MSACSR.Enable 0x%02x, MSACSR.Cause 0x%02x FPE\n",
           GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED,
           GET_FP_CAUSE(env->active_msa.msacsr));
#endif
      helper_raise_exception(EXCP_MSAFPE);
  }
}


/* Flush-to-zero use cases for update_msacsr() */
#define CLEAR_FS_UNDERFLOW 1
#define CLEAR_IS_INEXACT   2

static int update_msacsr(int action)
{
    int ieee_ex;

    int c;
    int cause;
    int enable;

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);

#if DEBUG_MSACSR
    if (ieee_ex) printf("float_flag(s) 0x%x: ", ieee_ex);
    if (ieee_ex & float_flag_invalid) printf("invalid ");
    if (ieee_ex & float_flag_divbyzero) printf("divbyzero ");
    if (ieee_ex & float_flag_overflow) printf("overflow ");
    if (ieee_ex & float_flag_underflow) printf("underflow ");
    if (ieee_ex & float_flag_inexact) printf("inexact ");
    if (ieee_ex & float_flag_input_denormal) printf("input_denormal ");
    if (ieee_ex & float_flag_output_denormal) printf("output_denormal ");
    if (ieee_ex) printf("\n");
#endif

    c = ieee_ex_to_mips(ieee_ex);
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;

    /* Set Inexact (I) when flushing inputs to zero */
    if ((ieee_ex & float_flag_input_denormal) &&
        (env->active_msa.msacsr & MSACSR_FS_BIT) != 0) {
      if (action & CLEAR_IS_INEXACT) {
        c &= ~FP_INEXACT;
      }
      else {
        c |=  FP_INEXACT;
      }
    }

    /* Set Inexact (I) and Underflow (U) when flushing outputs to zero */
    if ((ieee_ex & float_flag_output_denormal) &&
        (env->active_msa.msacsr & MSACSR_FS_BIT) != 0) {
      c |= FP_INEXACT;
      if (action & CLEAR_FS_UNDERFLOW) {
        c &= ~FP_UNDERFLOW;
      }
      else {
        c |=  FP_UNDERFLOW;
      }
    }

    /* Set Inexact (I) when Overflow (O) is not enabled */
    if ((c & FP_OVERFLOW) != 0 && (enable & FP_OVERFLOW) == 0) {
      c |= FP_INEXACT;
    }

    /* Clear Exact Underflow when Underflow (U) is not enabled */
    if ((c & FP_UNDERFLOW) != 0 && (enable & FP_UNDERFLOW) == 0 &&
        (c & FP_INEXACT) == 0) {
      c = c ^ FP_UNDERFLOW;
    }

    cause = c & enable;    /* all current enabled exceptions */

    if (cause == 0) {
      /* No enabled exception, update the MSACSR Cause
         with all current exceptions */
      SET_FP_CAUSE(env->active_msa.msacsr,
                   (GET_FP_CAUSE(env->active_msa.msacsr) | c));
    }
    else {
      /* Current exceptions are enabled */
      if ((env->active_msa.msacsr & MSACSR_NX_BIT) == 0) {
        /* Exception(s) will trap, update MSACSR Cause
           with all enabled exceptions */
        SET_FP_CAUSE(env->active_msa.msacsr,
                     (GET_FP_CAUSE(env->active_msa.msacsr) | c));
      }
    }

#if DEBUG_MSACSR
    printf("update_msacsr: c 0x%02x, cause 0x%02x, MSACSR.Cause 0x%02x, MSACSR.NX %d\n",
           c, cause, GET_FP_CAUSE(env->active_msa.msacsr),
           (env->active_msa.msacsr & MSACSR_NX_BIT) != 0);
#endif

    return c;
}

#define MSA_FLOAT_UNOP0(DEST, OP, ARG, BITS)                            \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## OP(ARG,                                \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(CLEAR_FS_UNDERFLOW);                              \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
    else {                                                              \
      if (float ## BITS ## _is_any_nan(ARG)) {                          \
        DEST = 0;                                                       \
      }                                                                 \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_UNOP_XD(DEST, OP, ARG, BITS, XBITS)                   \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## OP(ARG,                                \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(CLEAR_FS_UNDERFLOW);                              \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## XBITS >> 6) << 6) | c;                     \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_UNOP(DEST, OP, ARG, BITS)                             \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## OP(ARG,                                \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(0);                                               \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_LOGB(DEST, ARG, BITS)                                 \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    set_float_rounding_mode(float_round_down,                           \
                            &env->active_msa.fp_status);                \
    DEST = float ## BITS ## _ ## log2(ARG,                              \
                                      &env->active_msa.fp_status);      \
    DEST = float ## BITS ## _ ## round_to_int(DEST,                     \
                                           &env->active_msa.fp_status); \
    set_float_rounding_mode(ieee_rm[(env->active_msa.msacsr &           \
                                     MSACSR_RM_MASK) >> MSACSR_RM_POS], \
                            &env->active_msa.fp_status);                \
                                                                        \
    set_float_exception_flags(                                          \
      get_float_exception_flags(&env->active_msa.fp_status)             \
                                    & (~float_flag_inexact),            \
      &env->active_msa.fp_status);                                      \
                                                                        \
    c = update_msacsr(0);                                               \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_BINOP(DEST, OP, ARG1, ARG2, BITS)                     \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## OP(ARG1, ARG2,                         \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(0);                                               \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_MULADD(DEST, ARG1, ARG2, ARG3, NEGATE, BITS)          \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _muladd(ARG2, ARG3, ARG1, NEGATE,           \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(0);                                               \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define NUMBER_QNAN_PAIR(ARG1, ARG2, BITS)      \
  !float ## BITS ## _is_any_nan(ARG1)           \
  && float ## BITS ## _is_quiet_nan(ARG2)


/*
 *  FADD, FSUB, FMUL, FDIV
 */

void helper_fadd_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), add, W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), add, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fsub_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), sub, W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), sub, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fmul_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), mul, W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), mul, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fdiv_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), div, W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), div, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


/*
 *  FSQRT
 */

void helper_fsqrt_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(W(pwx, i), sqrt, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), sqrt, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


/*
 *  FEXP2, FLOG2
 */

void helper_fexp2_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), scalbn, W(pws, i),
                            W(pwt, i) >  0x200 ?  0x200 :
                            W(pwt, i) < -0x200 ? -0x200 : W(pwt, i),
                            32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), scalbn, D(pws, i),
                            D(pwt, i) >  0x1000 ?  0x1000 :
                            D(pwt, i) < -0x1000 ? -0x1000 : D(pwt, i),
                            64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_flog2_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_LOGB(W(pwx, i), W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_LOGB(D(pwx, i), D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


/*
 *  FMADD, FMSUB
 */

void helper_fmadd_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_MULADD(W(pwx, i), W(pwd, i),
                           W(pws, i), W(pwt, i), 0, 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_MULADD(D(pwx, i), D(pwd, i),
                           D(pws, i), D(pwt, i), 0, 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fmsub_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_MULADD(W(pwx, i), W(pwd, i),
                           W(pws, i), W(pwt, i),
                           float_muladd_negate_product, 32);
      } DONE_ALL_ELEMENTS;
      break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_MULADD(D(pwx, i), D(pwd, i),
                           D(pws, i), D(pwt, i),
                           float_muladd_negate_product, 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}



/*
 *  FMAX, FMIN
 */


#define FMAXMIN_A(F, G, X, _S, _T, BITS)                        \
  uint## BITS ##_t S = _S, T = _T;                              \
                                                                \
  if (NUMBER_QNAN_PAIR(S, T, BITS)) {                           \
    T = S;                                                      \
  }                                                             \
  else if (NUMBER_QNAN_PAIR(T, S, BITS)) {                      \
    S = T;                                                      \
  }                                                             \
                                                                \
  uint## BITS ##_t as = float## BITS ##_abs(S);                 \
  uint## BITS ##_t at = float## BITS ##_abs(T);                 \
                                                                \
  uint## BITS ##_t xs, xt, xd;                                  \
                                                                \
  MSA_FLOAT_BINOP(xs, F,  S,  T, BITS);                         \
  MSA_FLOAT_BINOP(xt, G,  S,  T, BITS);                         \
  MSA_FLOAT_BINOP(xd, F, as, at, BITS);                         \
                                                                \
  X = (as == at || xd == float## BITS ##_abs(xs)) ? xs : xt;


void helper_fmax_a_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            FMAXMIN_A(max, min, W(pwx, i), W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
           FMAXMIN_A(max, min, D(pwx, i), D(pws, i), D(pwt, i), 64);
         } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_fmax_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_BINOP(W(pwx, i), max, W(pws, i), W(pws, i), 32);
            }
            else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_BINOP(W(pwx, i), max, W(pwt, i), W(pwt, i), 32);
            }
            else {
                MSA_FLOAT_BINOP(W(pwx, i), max, W(pws, i), W(pwt, i), 32);
            }
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_BINOP(D(pwx, i), max, D(pws, i), D(pws, i), 64);
            }
            else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_BINOP(D(pwx, i), max, D(pwt, i), D(pwt, i), 64);
            }
            else {
                MSA_FLOAT_BINOP(D(pwx, i), max, D(pws, i), D(pwt, i), 64);
            }
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_fmin_a_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            FMAXMIN_A(min, max, W(pwx, i), W(pws, i), W(pwt, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            FMAXMIN_A(min, max, D(pwx, i), D(pws, i), D(pwt, i), 64);
         } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_fmin_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_BINOP(W(pwx, i), min, W(pws, i), W(pws, i), 32);
            }
            else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_BINOP(W(pwx, i), min, W(pwt, i), W(pwt, i), 32);
            }
            else {
                MSA_FLOAT_BINOP(W(pwx, i), min, W(pws, i), W(pwt, i), 32);
            }
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_BINOP(D(pwx, i), min, D(pws, i), D(pws, i), 64);
            }
            else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_BINOP(D(pwx, i), min, D(pwt, i), D(pwt, i), 64);
            }
            else {
                MSA_FLOAT_BINOP(D(pwx, i), min, D(pws, i), D(pwt, i), 64);
            }
         } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


/*
 *  FCAF, FSAF,
 *  FCEQ, FSEQ,
 *  FCUEQ, FSUEQ,
 *  FCNE, FSNE,
 *  FCUNE, FSUNE,
 *  FCLE, FSLE,
 *  FCULE, FSULE,
 *  FCLT, FSLT,
 *  FCULT, FSULT,
 *  FCUN, FSUN,
 *  FCOR, FSOR
 */

#define MSA_FLOAT_COND(DEST, OP, ARG1, ARG2, BITS, QUIET)               \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    int64_t cond;                                                       \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    if (!QUIET) {                                                       \
      cond = float ## BITS ## _ ## OP(ARG1, ARG2,                       \
                                      &env->active_msa.fp_status);      \
    } else {                                                            \
      cond = float ## BITS ## _ ## OP ## _quiet(ARG1, ARG2,             \
                                           &env->active_msa.fp_status); \
    }                                                                   \
    DEST = cond ? M_MAX_UINT(BITS) : 0;                                 \
                                                                        \
    c = update_msacsr(CLEAR_IS_INEXACT);                                \
                                                                        \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)


#define MSA_FLOAT_AF(DEST, ARG1, ARG2, BITS, QUIET)             \
  do {                                                          \
    MSA_FLOAT_COND(DEST, eq, ARG1, ARG2, BITS, QUIET);          \
    if ((DEST & M_MAX_UINT(BITS)) == M_MAX_UINT(BITS)) {        \
      DEST = 0;                                                 \
    }                                                           \
  } while (0)

#define MSA_FLOAT_UEQ(DEST, ARG1, ARG2, BITS, QUIET)            \
  do {                                                          \
    MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, eq, ARG1, ARG2, BITS, QUIET);        \
    }                                                           \
  } while (0)

#define MSA_FLOAT_NE(DEST, ARG1, ARG2, BITS, QUIET)             \
  do {                                                          \
    MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);          \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, lt, ARG2, ARG1, BITS, QUIET);        \
    }                                                           \
  } while (0)

#define MSA_FLOAT_UNE(DEST, ARG1, ARG2, BITS, QUIET)            \
  do {                                                          \
    MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);        \
      if (DEST == 0) {                                          \
        MSA_FLOAT_COND(DEST, lt, ARG2, ARG1, BITS, QUIET);      \
      }                                                         \
    }                                                           \
  } while (0)

#define MSA_FLOAT_ULE(DEST, ARG1, ARG2, BITS, QUIET)            \
  do {                                                          \
    MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, le, ARG1, ARG2, BITS, QUIET);        \
    }                                                           \
  } while (0)

#define MSA_FLOAT_ULT(DEST, ARG1, ARG2, BITS, QUIET)            \
  do {                                                          \
    MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);        \
    }                                                           \
  } while (0)

#define MSA_FLOAT_OR(DEST, ARG1, ARG2, BITS, QUIET)             \
  do {                                                          \
    MSA_FLOAT_COND(DEST, le, ARG1, ARG2, BITS, QUIET);          \
    if (DEST == 0) {                                            \
      MSA_FLOAT_COND(DEST, le, ARG2, ARG1, BITS, QUIET);        \
    }                                                           \
  } while (0)


static void
compare_af(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_AF(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_AF(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcaf_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_af(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsaf_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_af(pwd, pws, pwt, wrlen_df, 0);
}

static void
compare_eq(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_COND(W(pwx, i), eq, W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(D(pwx, i), eq, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fceq_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_eq(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fseq_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_eq(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ueq(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UEQ(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UEQ(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcueq_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ueq(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsueq_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ueq(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ne(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_NE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_NE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcne_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ne(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsne_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ne(pwd, pws, pwt, wrlen_df, 0);
}

static void
compare_une(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcune_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_une(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsune_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_une(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_le(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(W(pwx, i), le, W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(D(pwx, i), le, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcle_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_le(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsle_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_le(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ule(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_ULE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_ULE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcule_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ule(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsule_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ule(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_lt(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(W(pwx, i), lt, W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(D(pwx, i), lt, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fclt_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_lt(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fslt_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_lt(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ult(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_ULT(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_ULT(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcult_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ult(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsult_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ult(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_un(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(W(pwx, i), unordered, W(pws, i), W(pwt, i), 32, quiet);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_COND(D(pwx, i), unordered, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_fcun_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_un(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsun_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_un(pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_or(void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_OR(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_OR(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}
void helper_fcor_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_or(pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsor_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_or(pwd, pws, pwt, wrlen_df, 0);
}


/*
 *  FCLASS
 */

#define MSA_FLOAT_CLASS_SIGNALING_NAN      0x001
#define MSA_FLOAT_CLASS_QUIET_NAN          0x002

#define MSA_FLOAT_CLASS_NEGATIVE_INFINITY  0x004
#define MSA_FLOAT_CLASS_NEGATIVE_NORMAL    0x008
#define MSA_FLOAT_CLASS_NEGATIVE_SUBNORMAL 0x010
#define MSA_FLOAT_CLASS_NEGATIVE_ZERO      0x020

#define MSA_FLOAT_CLASS_POSITIVE_INFINITY  0x040
#define MSA_FLOAT_CLASS_POSITIVE_NORMAL    0x080
#define MSA_FLOAT_CLASS_POSITIVE_SUBNORMAL 0x100
#define MSA_FLOAT_CLASS_POSITIVE_ZERO      0x200


#define MSA_FLOAT_CLASS(ARG, BITS)                              \
    do {                                                        \
        int mask;                                               \
        int snan, qnan, inf, neg, zero, dnmz;                   \
                                                                \
        snan = float ## BITS ## _is_signaling_nan(ARG);         \
        qnan = float ## BITS ## _is_quiet_nan(ARG);             \
        inf  = float ## BITS ## _is_infinity(ARG);              \
        neg  = float ## BITS ## _is_neg(ARG);                   \
        zero = float ## BITS ## _is_zero(ARG);                  \
        dnmz = float ## BITS ## _is_zero_or_denormal(ARG);      \
                                                                \
        mask = 0;                                               \
        if (snan) {                                             \
            mask |= MSA_FLOAT_CLASS_SIGNALING_NAN;}             \
        else if (qnan) {                                        \
            mask |= MSA_FLOAT_CLASS_QUIET_NAN;                  \
        } else if (neg) {                                       \
            if (inf) {                                          \
                mask |= MSA_FLOAT_CLASS_NEGATIVE_INFINITY;      \
            } else if (zero) {                                  \
                mask |= MSA_FLOAT_CLASS_NEGATIVE_ZERO;          \
            } else if (dnmz) {                                  \
                mask |= MSA_FLOAT_CLASS_NEGATIVE_SUBNORMAL;     \
            }                                                   \
            else {                                              \
                mask |= MSA_FLOAT_CLASS_NEGATIVE_NORMAL;        \
            }                                                   \
        } else {                                                \
            if (inf) {                                          \
                mask |= MSA_FLOAT_CLASS_POSITIVE_INFINITY;      \
            } else if (zero) {                                  \
                mask |= MSA_FLOAT_CLASS_POSITIVE_ZERO;          \
            } else if (dnmz) {                                  \
                mask |= MSA_FLOAT_CLASS_POSITIVE_SUBNORMAL;     \
            } else {                                            \
                mask |= MSA_FLOAT_CLASS_POSITIVE_NORMAL;        \
            }                                                   \
        }                                                       \
                                                                \
        return mask;                                            \
    } while (0)


int64_t helper_fclass_df(int64_t arg, uint32_t df)
{
    if (df == DF_WORD) {
        MSA_FLOAT_CLASS(arg, 32);
    } else {
        MSA_FLOAT_CLASS(arg, 64);
    }
}


/*
 *  FEXDO, FEXUP
 */

static float16 float16_from_float32(int32 a, flag ieee STATUS_PARAM) {
      float16 f_val;

      f_val = float32_to_float16((float32)a, ieee  STATUS_VAR);
      f_val = float16_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 15)) : f_val;
}

static float32 float32_from_float64(int64 a STATUS_PARAM) {
      float32 f_val;

      f_val = float64_to_float32((float64)a STATUS_VAR);
      f_val = float32_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 31)) : f_val;
}

static float32 float32_from_float16(int16 a, flag ieee STATUS_PARAM) {
      float32 f_val;

      f_val = float16_to_float32((float16)a, ieee STATUS_VAR);
      f_val = float32_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 31)) : f_val;
}

static float64 float64_from_float32(int32 a STATUS_PARAM) {
      float64 f_val;

      f_val = float32_to_float64((float64)a STATUS_VAR);
      f_val = float64_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1ULL << 63)) : f_val;
}

void helper_fexdo_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);
    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(HL(pwx, i), from_float32, W(pws, i), ieee, 16);
            MSA_FLOAT_BINOP(HR(pwx, i), from_float32, W(pwt, i), ieee, 16);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(WL(pwx, i), from_float64, D(pws, i), 32);
            MSA_FLOAT_UNOP(WR(pwx, i), from_float64, D(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, &wx, wrlen);
}

void helper_fexupl_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(W(pwx, i), from_float16, HL(pws, i), ieee, 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), from_float32, WL(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, &wx, wrlen);
}

void helper_fexupr_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(W(pwx, i), from_float16, HR(pws, i), ieee, 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), from_float32, WR(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, &wx, wrlen);
}


/*
 *  FFINT, FTINT, FTRUNC, FRINT
 */

#define float32_from_int32 int32_to_float32
#define float32_from_uint32 uint32_to_float32

#define float64_from_int64 int64_to_float64
#define float64_from_uint64 uint64_to_float64


void helper_ffint_s_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(W(pwx, i), from_int32, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), from_int64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_ffint_u_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(W(pwx, i), from_uint32, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), from_uint64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_ftint_s_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_int32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_int64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_ftint_u_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_uint32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_uint64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_ftrunc_s_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;
    signed char rm = env->active_msa.fp_status.float_rounding_mode;

    clear_msacsr_cause();

    env->active_msa.fp_status.float_rounding_mode = float_round_to_zero;
    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_int32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_int64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }
    env->active_msa.fp_status.float_rounding_mode = rm;

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_ftrunc_u_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;
    signed char rm = env->active_msa.fp_status.float_rounding_mode;

    clear_msacsr_cause();

    env->active_msa.fp_status.float_rounding_mode = float_round_to_zero;
    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_uint32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_uint64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }
    env->active_msa.fp_status.float_rounding_mode = rm;

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

void helper_frint_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(W(pwx, i), round_to_int, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), round_to_int, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


/*
 *  FFQ, FTQ
 */

static float32 float32_from_q16(int16 a STATUS_PARAM)
{
    float32 f_val;

    /* conversion as integer and scaling */
    f_val = int32_to_float32(a STATUS_VAR);
    f_val = float32_scalbn(f_val, -15 STATUS_VAR);

    return f_val;
}

static float64 float64_from_q32(int32 a STATUS_PARAM)
{
    float64 f_val;

    /* conversion as integer and scaling */
    f_val = int32_to_float64(a STATUS_VAR);
    f_val = float64_scalbn(f_val, -31 STATUS_VAR);

    return f_val;
}

static int16 float32_to_q16(float32 a STATUS_PARAM)
{
    int32 q_val;
    int32 q_min = 0xffff8000;
    int32 q_max = 0x00007fff;

    int ieee_ex;

    if (float32_is_any_nan(a)) {
      float_raise( float_flag_invalid STATUS_VAR);
      return 0;
    }

    /* scaling */
    a = float32_scalbn(a, 15 STATUS_VAR);

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);
    if (ieee_ex & float_flag_overflow) {
      float_raise( float_flag_inexact STATUS_VAR);
      return (int32)a < 0 ? q_min : q_max;
    }

    /* conversion to int */
    q_val = float32_to_int32(a STATUS_VAR);

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);
    if (ieee_ex & float_flag_invalid) {
      set_float_exception_flags(ieee_ex & (~float_flag_invalid),
                                &env->active_msa.fp_status);
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int32)a < 0 ? q_min : q_max;
    }

    if (q_val < q_min) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int16)q_min;
    }

    if (q_max < q_val) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int16)q_max;
    }

    return (int16)q_val;
}

static int32 float64_to_q32(float64 a STATUS_PARAM)
{
    int64 q_val;
    int64 q_min = 0xffffffff80000000LL;
    int64 q_max = 0x000000007fffffffLL;

    int ieee_ex;

    if (float64_is_any_nan(a)) {
      float_raise( float_flag_invalid STATUS_VAR);
      return 0;
    }

    /* scaling */
    a = float64_scalbn(a, 31 STATUS_VAR);

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);
    if (ieee_ex & float_flag_overflow) {
      float_raise( float_flag_inexact STATUS_VAR);
      return (int64)a < 0 ? q_min : q_max;
    }

    /* conversion to integer */
    q_val = float64_to_int64(a STATUS_VAR);

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);
    if (ieee_ex & float_flag_invalid) {
      set_float_exception_flags(ieee_ex & (~float_flag_invalid),
                                &env->active_msa.fp_status);
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int64)a < 0 ? q_min : q_max;
    }

    if (q_val < q_min) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int32)q_min;
    }

    if (q_max < q_val) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int32)q_max;
    }

    return (int32)q_val;
}

void helper_ffql_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP(W(pwx, i), from_q16, HL(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP(D(pwx, i), from_q32, WL(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}

void helper_ffqr_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(W(pwx, i), from_q16, HR(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), from_q32, WR(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(pwd, &wx, wrlen);
}

void helper_ftq_df(void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP_XD(HL(pwx, i), to_q16, W(pws, i), 32, 16);
          MSA_FLOAT_UNOP_XD(HR(pwx, i), to_q16, W(pwt, i), 32, 16);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP_XD(WL(pwx, i), to_q32, D(pws, i), 64, 32);
          MSA_FLOAT_UNOP_XD(WR(pwx, i), to_q32, D(pwt, i), 64, 32);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}

/*
 *  FRCP, FRSQRT
 */

void helper_frcp_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(W(pwx, i), div, FLOAT_ONE32, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_BINOP(D(pwx, i), div, FLOAT_ONE64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    /* Inexact flag is always set */
    set_float_exception_flags(
      get_float_exception_flags(&env->active_msa.fp_status)
                                     | ~float_flag_inexact,
      &env->active_msa.fp_status);

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}


void helper_frsqrt_df(void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause();

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
           MSA_FLOAT_UNOP(W(pwx, i), sqrt, W(pws, i), 32);
           MSA_FLOAT_BINOP(W(pwx, i), div, FLOAT_ONE32, W(pwx, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_UNOP(D(pwx, i), sqrt, D(pws, i), 64);
            MSA_FLOAT_BINOP(D(pwx, i), div, FLOAT_ONE64, D(pwx, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    /* Inexact flag is always set */
    set_float_exception_flags(
      get_float_exception_flags(&env->active_msa.fp_status)
                                     | ~float_flag_inexact,
      &env->active_msa.fp_status);

    check_msacsr_cause();
    helper_move_v(pwd, pwx, wrlen);
}



/*
 *  MSA Control Register (MSACSR) instructions: CFCMSA, CTCMSA
 */

target_ulong helper_cfcmsa(uint32_t cs)
{
    switch (cs) {
    case MSAIR_REGISTER:
        return env->active_msa.msair;

    case MSACSR_REGISTER:
#if DEBUG_MSACSR
        printf("cfcmsa 0x%08x: Cause 0x%02x, Enable 0x%02x, Flags 0x%02x\n",
               env->active_msa.msacsr & MSACSR_BITS,
               GET_FP_CAUSE(env->active_msa.msacsr & MSACSR_BITS),
               GET_FP_ENABLE(env->active_msa.msacsr & MSACSR_BITS),
               GET_FP_FLAGS(env->active_msa.msacsr & MSACSR_BITS));
#endif
        return env->active_msa.msacsr & MSACSR_BITS;

    case MSAACCESS_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msaaccess;
        else
            break;

    case MSASAVE_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msasave;
        else
            break;

    case MSAMODIFY_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msamodify;
        else
            break;

    case MSAREQUEST_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msarequest;
        else
            break;

    case MSAMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msamap;
        else
            break;

    case MSAUNMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT)
            return env->active_msa.msaunmap;
        else
            break;
    }

    // helper_raise_exception(EXCP_RI);
    return 0;
}


void helper_ctcmsa(target_ulong elm, uint32_t cd)
{
    switch (cd) {
    case MSAIR_REGISTER:
        break;

    case MSACSR_REGISTER:
        env->active_msa.msacsr = (int32_t)elm & MSACSR_BITS;

#if DEBUG_MSACSR
        printf("ctcmsa 0x%08x (0x%08x): Cause 0x%02x, Enable 0x%02x, Flags 0x%02x\n",
               env->active_msa.msacsr & MSACSR_BITS, elm,
               GET_FP_CAUSE(env->active_msa.msacsr & MSACSR_BITS),
               GET_FP_ENABLE(env->active_msa.msacsr & MSACSR_BITS),
               GET_FP_FLAGS(env->active_msa.msacsr & MSACSR_BITS));
#endif
        /* set float_status rounding mode */
        set_float_rounding_mode(
            ieee_rm[(env->active_msa.msacsr & MSACSR_RM_MASK) >> MSACSR_RM_POS],
            &env->active_msa.fp_status);

        /* set float_status flush modes */
        set_flush_to_zero(
          (env->active_msa.msacsr & MSACSR_FS_BIT) != 0 ? 1 : 0,
          &env->active_msa.fp_status);
        set_flush_inputs_to_zero(
          (env->active_msa.msacsr & MSACSR_FS_BIT) != 0 ? 1 : 0,
          &env->active_msa.fp_status);

        /* check exception */
        if ((GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED)
            & GET_FP_CAUSE(env->active_msa.msacsr)) {
            helper_raise_exception(EXCP_MSAFPE);
        }

        return;

    case MSAACCESS_REGISTER:
        break;

    case MSASAVE_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msasave = (int32_t)elm;
            return;
        }
        else
            break;

    case MSAMODIFY_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msamodify = (int32_t)elm;
            return;
        }
        else
            break;

    case MSAREQUEST_REGISTER:
        break;

    case MSAMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msamap = (int32_t)elm;

            /* TBD */

            env->active_msa.msaaccess |= 1 << (int32_t)elm;
            return;
        }
        else
            break;

    case MSAUNMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
             env->active_msa.msaunmap = (int32_t)elm;

            /* TBD */

             env->active_msa.msaaccess &= ~(1 << (int32_t)elm);
             return;
        }
        else
            break;
    }

    // helper_raise_exception(EXCP_RI);
}
