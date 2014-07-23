/*
 * MIPS SIMD Architecture Module Instruction emulation helpers for QEMU.
 *
 * Copyright (c) 2014 Imagination Technologies
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

#include "cpu.h"
#include "helper.h"

#define DF_BYTE   0
#define DF_HALF   1
#define DF_WORD   2
#define DF_DOUBLE 3

static void msa_check_index(CPUMIPSState *env,
        uint32_t df, uint32_t n) {
    switch (df) {
    case DF_BYTE: /* b */
        if (n > MSA_WRLEN / 8 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_HALF: /* h */
        if (n > MSA_WRLEN / 16 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_WORD: /* w */
        if (n > MSA_WRLEN / 32 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_DOUBLE: /* d */
        if (n > MSA_WRLEN / 64 - 1) {
            helper_raise_exception(env, EXCP_RI);
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
#define BL(pwr, i) (((wr_t *)pwr)->b[i + MSA_WRLEN/16])

#define ALL_B_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 8; i--;)

#define  H(pwr, i) (((wr_t *)pwr)->h[i])
#define HR(pwr, i) (((wr_t *)pwr)->h[i])
#define HL(pwr, i) (((wr_t *)pwr)->h[i + MSA_WRLEN/32])

#define ALL_H_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 16; i--;)

#define  W(pwr, i) (((wr_t *)pwr)->w[i])
#define WR(pwr, i) (((wr_t *)pwr)->w[i])
#define WL(pwr, i) (((wr_t *)pwr)->w[i + MSA_WRLEN/64])

#define ALL_W_ELEMENTS(i, wrlen)                \
    do {                                        \
        uint32_t i;                             \
        for (i = wrlen / 32; i--;)

#define  D(pwr, i) (((wr_t *)pwr)->d[i])
#define DR(pwr, i) (((wr_t *)pwr)->d[i])
#define DL(pwr, i) (((wr_t *)pwr)->d[i + MSA_WRLEN/128])

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

static inline void msa_move_v(void *pwd, void *pws)
{
    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        D(pwd, i) = D(pws, i);
    } DONE_ALL_ELEMENTS;
}

static inline uint64_t msa_load_wr_elem_i64(CPUMIPSState *env, int32_t wreg,
        int32_t df, int32_t i)
{
    i %= DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, (uint32_t)df, (uint32_t)i);

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

static inline int64_t msa_load_wr_elem_s64(CPUMIPSState *env, int32_t wreg,
        int32_t df, int32_t i)
{
    i %= DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, (uint32_t)df, (uint32_t)i);

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

static inline void msa_store_wr_elem(CPUMIPSState *env, uint64_t val,
        int32_t wreg, int32_t df, int32_t i)
{
    i %= DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, (uint32_t)df, (uint32_t)i);

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
}

static inline int64_t msa_add_a_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 + abs_arg2;
}

void helper_msa_add_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_add_a_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_addv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = (int64_t) ts + tt;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_addvi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = (int64_t) ts + u5;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_subv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = (int64_t) ts - tt;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_subvi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = (int64_t) ts - u5;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_adds_a_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
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

void helper_msa_adds_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_adds_a_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_adds_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);
    if (arg1 < 0) {
        return (min_int - arg1 < arg2) ? arg1 + arg2 : min_int;
    } else {
        return (arg2 < max_int - arg1) ? arg1 + arg2 : max_int;
    }
}

void helper_msa_adds_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_adds_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline uint64_t msa_adds_u_df(CPUMIPSState *env, uint32_t df,
        uint64_t arg1, uint64_t arg2)
{
    uint64_t max_uint = DF_MAX_UINT(df);
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return (u_arg1 < max_uint - u_arg2) ? u_arg1 + u_arg2 : max_uint;
}

void helper_msa_adds_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_adds_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_subs_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);
    if (arg2 > 0) {
        return (min_int + arg2 < arg1) ? arg1 - arg2 : min_int;
    } else {
        return (arg1 < max_int + arg2) ? arg1 - arg2 : max_int;
    }
}

void helper_msa_subs_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_subs_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_subs_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return (u_arg1 > u_arg2) ? u_arg1 - u_arg2 : 0;
}

void helper_msa_subs_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_subs_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_subsuu_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
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

void helper_msa_subsuu_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_subsuu_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_subsus_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t max_uint = DF_MAX_UINT(df);
    if (arg2 >= 0) {
        uint64_t u_arg2 = (uint64_t)arg2;
        return (u_arg1 > u_arg2) ?
            (int64_t)(u_arg1 - u_arg2) :
            0;
    } else {
        uint64_t u_arg2 = (uint64_t)(-arg2);
        return (u_arg1 < max_uint - u_arg2) ?
            (int64_t)(u_arg1 + u_arg2) :
            (int64_t)max_uint;
    }
}

void helper_msa_subsus_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_subsus_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_and_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        D(pwd, i) = D(pws, i) & D(pwt, i);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_andi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        B(pwd, i) = B(pws, i) & i8;
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_or_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        D(pwd, i) = D(pws, i) | D(pwt, i);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_ori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        B(pwd, i) = B(pws, i) | i8;
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_nor_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        D(pwd, i) = ~(D(pws, i) | D(pwt, i));
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_nori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        B(pwd, i) = ~(B(pws, i) | i8);
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_xor_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        D(pwd, i) = D(pws, i) ^ D(pwt, i);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_xori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        B(pwd, i) = B(pws, i) ^ i8;
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_asub_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    /* signed compare */
    return (arg1 < arg2) ?
        (uint64_t)(arg2 - arg1) : (uint64_t)(arg1 - arg2);
}

void helper_msa_asub_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_asub_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline uint64_t msa_asub_u_df(CPUMIPSState *env, uint32_t df,
        uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned compare */
    return (u_arg1 < u_arg2) ?
        (uint64_t)(u_arg2 - u_arg1) : (uint64_t)(u_arg1 - u_arg2);
}

void helper_msa_asub_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_asub_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_ave_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + (arg1 & arg2 & 1);
}

void helper_msa_ave_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_ave_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline uint64_t msa_ave_u_df(CPUMIPSState *env, uint32_t df,
        uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + (u_arg1 & u_arg2 & 1);
}

void helper_msa_ave_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_ave_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_aver_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + ((arg1 | arg2) & 1);
}

void helper_msa_aver_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_aver_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline uint64_t msa_aver_u_df(CPUMIPSState *env, uint32_t df,
        uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + ((u_arg1 | u_arg2) & 1);
}

void helper_msa_aver_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_aver_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_bclr_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 & (~(1LL << b_arg2)), df);
}

void helper_msa_bclr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_bclr_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bclri_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_bclr_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_bneg_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 ^ (1LL << b_arg2), df);
}

void helper_msa_bneg_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_bneg_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bnegi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_bneg_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_bset_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 | (1LL << b_arg2), df);
}

void helper_msa_bset_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_bset_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bseti_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_bset_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_binsl_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
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

void helper_msa_binsl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_binsl_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_binsli_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_binsl_df(env, df, td, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_binsr_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
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

void helper_msa_binsr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_binsr_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_binsri_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_binsr_df(env, df, td, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

#define BIT_MOVE_IF_NOT_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED(((dest & (~arg2)) | (arg1 & arg2)), df)

void helper_msa_bmnz_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        BIT_MOVE_IF_NOT_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bmnzi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        BIT_MOVE_IF_NOT_ZERO(B(pwd, i), B(pws, i), i8, DF_BYTE);
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

#define BIT_MOVE_IF_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED((dest & arg2) | (arg1 & (~arg2)), df)

void helper_msa_bmz_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        BIT_MOVE_IF_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bmzi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        BIT_MOVE_IF_ZERO(B(pwd, i), B(pws, i), i8, DF_BYTE);
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

#define BIT_SELECT(dest, arg1, arg2, df) \
            dest = UNSIGNED((arg1 & (~dest)) | (arg2 & dest), df)

void helper_msa_bsel_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    ALL_D_ELEMENTS(i, MSA_WRLEN) {
        BIT_SELECT(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_bseli_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t i8)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    ALL_B_ELEMENTS(i, MSA_WRLEN) {
        BIT_SELECT(B(pwd, i), B(pws, i), i8, DF_BYTE);
    } DONE_ALL_ELEMENTS;
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_ceq_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    return arg1 == arg2 ? -1 : 0;
}

void helper_msa_ceq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_ceq_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_ceqi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t i5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_ceq_df(env, df, ts, i5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_cle_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return arg1 <= arg2 ? -1 : 0;
}

void helper_msa_cle_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_cle_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_clei_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t s5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_cle_s_df(env, df, ts, s5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_cle_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 <= u_arg2 ? -1 : 0;
}

void helper_msa_cle_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_cle_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_clei_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        td = msa_cle_u_df(env, df, ts, u5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_clt_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return arg1 < arg2 ? -1 : 0;
}

void helper_msa_clt_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_clt_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_clti_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t s5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_clt_s_df(env, df, ts, s5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_clt_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 < u_arg2 ? -1 : 0;
}

void helper_msa_clt_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_clt_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_clti_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_clt_u_df(env, df, ts, u5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_copy_s_df(CPUMIPSState *env, uint32_t df, uint32_t rd,
        uint32_t ws, uint32_t n)
{
    n %= DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, (uint32_t)df, (uint32_t)n);
    switch (df) {
    case DF_BYTE: /* b */
        env->active_tc.gpr[rd] = (int8_t)env->active_fpu.fpr[ws].wr.b[n];
        break;
    case DF_HALF: /* h */
        env->active_tc.gpr[rd] = (int16_t)env->active_fpu.fpr[ws].wr.h[n];
        break;
    case DF_WORD: /* w */
        env->active_tc.gpr[rd] = (int32_t)env->active_fpu.fpr[ws].wr.w[n];
        break;
#ifdef TARGET_MIPS64
    case DF_DOUBLE: /* d */
        env->active_tc.gpr[rd] = (int64_t)env->active_fpu.fpr[ws].wr.d[n];
        break;
#endif
    default:
        /* shouldn't get here */
        assert(0);
    }
}

void helper_msa_copy_u_df(CPUMIPSState *env, uint32_t df, uint32_t rd,
        uint32_t ws, uint32_t n)
{
    n %= DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, (uint32_t)df, (uint32_t)n);
    switch (df) {
    case DF_BYTE: /* b */
        env->active_tc.gpr[rd] = (uint8_t)env->active_fpu.fpr[ws].wr.b[n];
        break;
    case DF_HALF: /* h */
        env->active_tc.gpr[rd] = (uint16_t)env->active_fpu.fpr[ws].wr.h[n];
        break;
    case DF_WORD: /* w */
        env->active_tc.gpr[rd] = (uint32_t)env->active_fpu.fpr[ws].wr.w[n];
        break;
#ifdef TARGET_MIPS64
    case DF_DOUBLE: /* d */
        env->active_tc.gpr[rd] = (uint64_t)env->active_fpu.fpr[ws].wr.d[n];
        break;
#endif
    default:
        /* shouldn't get here */
        assert(0);
    }
}


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

static inline int64_t msa_hadd_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return SIGNED_ODD(arg1, df) + SIGNED_EVEN(arg2, df);
}

void helper_msa_hadd_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_hadd_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_hadd_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return UNSIGNED_ODD(arg1, df) + UNSIGNED_EVEN(arg2, df);
}

void helper_msa_hadd_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_hadd_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_hsub_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return SIGNED_ODD(arg1, df) - SIGNED_EVEN(arg2, df);
}

void helper_msa_hsub_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_hsub_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_hsub_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return UNSIGNED_ODD(arg1, df) - UNSIGNED_EVEN(arg2, df);
}

void helper_msa_hsub_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_hsub_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dotp_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

void helper_msa_dotp_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_dotp_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dotp_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

void helper_msa_dotp_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_dotp_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dpadd_s_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

void helper_msa_dpadd_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_dpadd_s_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dpadd_u_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

void helper_msa_dpadd_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_dpadd_u_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dpsub_s_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

void helper_msa_dpsub_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_dpsub_s_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_dpsub_u_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

void helper_msa_dpsub_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_dpsub_u_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_ilvev_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            B(pwx, 2*i)   = B(pwt, 2*i);
            B(pwx, 2*i+1) = B(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            H(pwx, 2*i)   = H(pwt, 2*i);
            H(pwx, 2*i+1) = H(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            W(pwx, 2*i)   = W(pwt, 2*i);
            W(pwx, 2*i+1) = W(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            D(pwx, 2*i)   = D(pwt, 2*i);
            D(pwx, 2*i+1) = D(pws, 2*i);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_ilvev_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_ilvev_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_ilvod_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            B(pwx, 2*i)   = B(pwt, 2*i+1);
            B(pwx, 2*i+1) = B(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            H(pwx, 2*i)   = H(pwt, 2*i+1);
            H(pwx, 2*i+1) = H(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            W(pwx, 2*i)   = W(pwt, 2*i+1);
            W(pwx, 2*i+1) = W(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            D(pwx, 2*i)   = D(pwt, 2*i+1);
            D(pwx, 2*i+1) = D(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_ilvod_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_ilvod_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_ilvl_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            B(pwx, 2*i)   = BL(pwt, i);
            B(pwx, 2*i+1) = BL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            H(pwx, 2*i)   = HL(pwt, i);
            H(pwx, 2*i+1) = HL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            W(pwx, 2*i)   = WL(pwt, i);
            W(pwx, 2*i+1) = WL(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            D(pwx, 2*i)   = DL(pwt, i);
            D(pwx, 2*i+1) = DL(pws, i);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_ilvl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_ilvl_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_ilvr_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            B(pwx, 2*i)   = BR(pwt, i);
            B(pwx, 2*i+1) = BR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            H(pwx, 2*i)   = HR(pwt, i);
            H(pwx, 2*i+1) = HR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            W(pwx, 2*i)   = WR(pwt, i);
            W(pwx, 2*i+1) = WR(pws, i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            D(pwx, 2*i)   = DR(pwt, i);
            D(pwx, 2*i+1) = DR(pws, i);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_ilvr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_ilvr_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_pckev_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            BR(pwx, i) = B(pwt, 2*i);
            BL(pwx, i) = B(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            HR(pwx, i) = H(pwt, 2*i);
            HL(pwx, i) = H(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            WR(pwx, i) = W(pwt, 2*i);
            WL(pwx, i) = W(pws, 2*i);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            DR(pwx, i) = D(pwt, 2*i);
            DL(pwx, i) = D(pws, 2*i);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_pckev_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_pckev_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_pckod_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            BR(pwx, i) = B(pwt, 2*i+1);
            BL(pwx, i) = B(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            HR(pwx, i) = H(pwt, 2*i+1);
            HL(pwx, i) = H(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            WR(pwx, i) = W(pwt, 2*i+1);
            WL(pwx, i) = W(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_Q_ELEMENTS(i, MSA_WRLEN) {
            DR(pwx, i) = D(pwt, 2*i+1);
            DL(pwx, i) = D(pws, 2*i+1);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_pckod_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_pckod_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_vshf_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, void *pwt)
{
    uint32_t n = MSA_WRLEN / DF_BITS(df);
    uint32_t k;
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
        /* byte data format */
        ALL_B_ELEMENTS(i, MSA_WRLEN) {
            k = (B(pwd, i) & 0x3f) % (2 * n);
            B(pwx, i) =
                (B(pwd, i) & 0xc0) ? 0 : k < n ? B(pwt, k) : B(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        /* half data format */
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            k = (H(pwd, i) & 0x3f) % (2 * n);
            H(pwx, i) =
                (H(pwd, i) & 0xc0) ? 0 : k < n ? H(pwt, k) : H(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        /* word data format */
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            k = (W(pwd, i) & 0x3f) % (2 * n);
            W(pwx, i) =
                (W(pwd, i) & 0xc0) ? 0 : k < n ? W(pwt, k) : W(pws, k - n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        /* double data format */
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            k = (D(pwd, i) & 0x3f) % (2 * n);
            D(pwx, i) =
                (D(pwd, i) & 0xc0) ? 0 : k < n ? D(pwt, k) : D(pws, k - n);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_vshf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    msa_vshf_df(env, df, pwd, pws, pwt);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}


#define SHF_POS(i, imm) ((i & 0xfc) + ((imm >> (2 * (i & 0x03))) & 0x03))

static inline void msa_shf_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, uint32_t imm)
{
    wr_t wx, *pwx = &wx;
    switch (df) {
    case DF_BYTE:
      ALL_B_ELEMENTS(i, MSA_WRLEN) {
        B(pwx, i) = B(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;
    case DF_HALF:
      ALL_H_ELEMENTS(i, MSA_WRLEN) {
        H(pwx, i) = H(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;
    case DF_WORD:
      ALL_W_ELEMENTS(i, MSA_WRLEN) {
        W(pwx, i) = W(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    msa_move_v(pwd, pwx);
}

void helper_msa_shf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t imm)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_shf_df(env, df, pwd, pws, imm);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_maddv_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    return dest + arg1 * arg2;
}

void helper_msa_maddv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_maddv_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_msubv_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    return dest - arg1 * arg2;
}

void helper_msa_msubv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_msubv_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_max_a_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 > abs_arg2 ? arg1 : arg2;
}

void helper_msa_max_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_max_a_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_max_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return arg1 > arg2 ? arg1 : arg2;
}

void helper_msa_max_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_max_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_maxi_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t s5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_max_s_df(env, df, ts, s5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_max_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 > u_arg2 ? arg1 : arg2;
}

void helper_msa_max_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_max_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_maxi_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        td = msa_max_u_df(env, df, ts, u5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_min_a_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 < abs_arg2 ? arg1 : arg2;
}

void helper_msa_min_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_min_a_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_min_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    return arg1 < arg2 ? arg1 : arg2;
}

void helper_msa_min_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_min_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_mini_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t s5)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_min_s_df(env, df, ts, s5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_min_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 < u_arg2 ? arg1 : arg2;
}

void helper_msa_min_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_min_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_mini_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, int64_t u5)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        td = msa_min_u_df(env, df, ts, u5);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_splat_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, target_ulong rt)
{
    uint32_t n = rt % DF_ELEMENTS(df, MSA_WRLEN);
    msa_check_index(env, df, n);
    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, MSA_WRLEN) {
            B(pwd, i)   = B(pws, n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            H(pwd, i)   = H(pws, n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            W(pwd, i)   = W(pws, n);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            D(pwd, i)   = D(pws, n);
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
}

void helper_msa_splat_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t rt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_splat_df(env, df, pwd, pws, env->active_tc.gpr[rt]);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_splati_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t n)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_splat_df(env, df, pwd, pws, n);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_move_v(CPUMIPSState *env, uint32_t wd, uint32_t ws)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_move_v(pwd, pws);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_ldi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t s10)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    int64_t s64 = ((int64_t)s10 << 54) >> 54;
    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, MSA_WRLEN) {
            B(pwd, i)   = (int8_t)s10;
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            H(pwd, i)   = (int16_t)s64;
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            W(pwd, i)   = (int32_t)s64;
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            D(pwd, i)   = s64;
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_fill_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t rs)
{
    void *pwd = &(env->active_fpu.fpr[wd]);

    switch (df) {
    case DF_BYTE:
        ALL_B_ELEMENTS(i, MSA_WRLEN) {
            B(pwd, i)   = (int8_t)env->active_tc.gpr[rs];
        } DONE_ALL_ELEMENTS;
        break;
    case DF_HALF:
        ALL_H_ELEMENTS(i, MSA_WRLEN) {
            H(pwd, i)   = (int16_t)env->active_tc.gpr[rs];
        } DONE_ALL_ELEMENTS;
        break;
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            W(pwd, i)   = (int32_t)env->active_tc.gpr[rs];
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            D(pwd, i)   = (int64_t)env->active_tc.gpr[rs];
        } DONE_ALL_ELEMENTS;
       break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_insert_df(CPUMIPSState *env, uint32_t df, void *pwd,
        target_ulong rs, uint32_t n)
{
    msa_check_index(env, df, n);
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

void helper_msa_insert_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t rs, uint32_t n)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    msa_insert_df(env, df, pwd, env->active_tc.gpr[rs], n);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_insve_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, uint32_t n)
{
    msa_check_index(env, df, n);
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

void helper_msa_insve_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t n)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_insve_df(env, df, pwd, pws, n);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_mulv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = ts * tt;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_div_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    if (arg1 == DF_MIN_INT(df) && arg2 == -1) {
        return DF_MIN_INT(df);
    }
    return arg2 ? arg1 / arg2 : 0;
}

void helper_msa_div_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_div_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_div_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg2 ? u_arg1 / u_arg2 : 0;
}

void helper_msa_div_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_div_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_mod_s_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    if (arg1 == DF_MIN_INT(df) && arg2 == -1) {
        return 0;
    }
    return arg2 ? arg1 % arg2 : 0;
}

void helper_msa_mod_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_mod_s_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_mod_u_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg2 ? u_arg1 % u_arg2 : 0;
}

void helper_msa_mod_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    uint64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        tt = msa_load_wr_elem_i64(env, wt, df, i);
        td = msa_mod_u_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_nlzc_df(CPUMIPSState *env, uint32_t df, int64_t arg)
{
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

void helper_msa_nlzc_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_nlzc_df(env, df, ts);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_nloc_df(CPUMIPSState *env, uint32_t df, int64_t arg)
{
    return msa_nlzc_df(env, df, UNSIGNED((~arg), df));
}

void helper_msa_nloc_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_nloc_df(env, df, ts);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_pcnt_df(CPUMIPSState *env, uint32_t df, int64_t arg)
{
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

void helper_msa_pcnt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_pcnt_df(env, df, ts);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_sat_u_df(CPUMIPSState *env, uint32_t df, int64_t arg,
        uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return  u_arg < M_MAX_UINT(m+1) ? u_arg :
                                      M_MAX_UINT(m+1);
}

void helper_msa_sat_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_i64(env, ws, df, i);
        td = msa_sat_u_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_sat_s_df(CPUMIPSState *env, uint32_t df, int64_t arg,
        uint32_t m)
{
    return arg < M_MIN_INT(m+1) ? M_MIN_INT(m+1) :
                                  arg > M_MAX_INT(m+1) ? M_MAX_INT(m+1) :
                                                         arg;
}

void helper_msa_sat_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_sat_s_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_sll_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 << b_arg2;
}

void helper_msa_sll_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_sll_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_slli_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = ts << m;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_sra_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 >> b_arg2;
}

void helper_msa_sra_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_sra_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_srai_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = ts >> m;
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srl_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return u_arg1 >> b_arg2;
}

void helper_msa_srl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_srl_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srli_df(CPUMIPSState *env, uint32_t df, int64_t arg,
        uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return u_arg >> m;
}

void helper_msa_srli_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_srli_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srar_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    if (b_arg2 == 0) {
        return arg1;
    } else {
        int64_t r_bit = (arg1 >> (b_arg2 - 1)) & 1;
        return (arg1 >> b_arg2) + r_bit;
    }
}

void helper_msa_srar_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_srar_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srari_df(CPUMIPSState *env, uint32_t df, int64_t arg,
        uint32_t m)
{
    if (m == 0) {
        return arg;
    } else {
        int64_t r_bit = (arg >> (m - 1)) & 1;
        return (arg >> m) + r_bit;
    }
}

void helper_msa_srari_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_srari_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srlr_df(CPUMIPSState *env, uint32_t df, int64_t arg1,
        int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    if (b_arg2 == 0) {
        return u_arg1;
    } else {
        uint64_t r_bit = (u_arg1 >> (b_arg2 - 1)) & 1;
        return (u_arg1 >> b_arg2) + r_bit;
    }
}

void helper_msa_srlr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_srlr_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_srlri_df(CPUMIPSState *env, uint32_t df, int64_t arg,
        uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    if (m == 0) {
        return u_arg;
    } else {
        uint64_t r_bit = (u_arg >> (m - 1)) & 1;
        return (u_arg >> m) + r_bit;
    }
}

void helper_msa_srlri_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t m)
{
    int64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);
    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_srlri_df(env, df, ts, m);
        msa_store_wr_elem(env, td, wd, df, i);
    }
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}
static inline void msa_sld_df(CPUMIPSState *env, uint32_t df, void *pwd,
        void *pws, target_ulong rt)
{
    uint32_t n = rt % DF_ELEMENTS(df, MSA_WRLEN);
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

    msa_check_index(env, df, n);
    switch (df) {
    case DF_BYTE:
        CONCATENATE_AND_SLIDE(MSA_WRLEN/8, 0);
        break;
    case DF_HALF:
        for (k = 0; k < 2; k++) {
            CONCATENATE_AND_SLIDE(MSA_WRLEN/16, k);
        }
        break;
    case DF_WORD:
        for (k = 0; k < 4; k++) {
            CONCATENATE_AND_SLIDE(MSA_WRLEN/32, k);
        }
        break;
    case DF_DOUBLE:
        for (k = 0; k < 8; k++) {
            CONCATENATE_AND_SLIDE(MSA_WRLEN/64, k);
        }
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }
}

void helper_msa_sld_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t rt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    msa_sld_df(env, df, pwd, pws, env->active_tc.gpr[rt]);
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

void helper_msa_sldi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t n)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    msa_sld_df(env, df, pwd, pws, n);

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_mul_q_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2) >> (DF_BITS(df) - 1);
}

void helper_msa_mul_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_mul_q_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_mulr_q_df(CPUMIPSState *env, uint32_t df,
        int64_t arg1, int64_t arg2)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
}

void helper_msa_mulr_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_mulr_q_df(env, df, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_madd_q_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

void helper_msa_madd_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_madd_q_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_maddr_q_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

void helper_msa_maddr_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_maddr_q_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_msub_q_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

void helper_msa_msub_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_msub_q_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_msubr_q_df(CPUMIPSState *env, uint32_t df,
        int64_t dest, int64_t arg1, int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

void helper_msa_msubr_q_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    int64_t td, ts, tt;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        tt = msa_load_wr_elem_s64(env, wt, df, i);
        td = msa_load_wr_elem_s64(env, wd, df, i);
        td = msa_msubr_q_df(env, df, td, ts, tt);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline int64_t msa_ld_df(CPUMIPSState *env, uint32_t df_bits,
        target_ulong addr)
{
    switch (df_bits) {
    case 8:
        return  do_ld8(env, addr, env->hflags & MIPS_HFLAG_KSU);
    case 16:
        return  do_ld16(env, addr, env->hflags & MIPS_HFLAG_KSU);
    case 32:
        return (int64_t) do_ld32(env, addr, env->hflags & MIPS_HFLAG_KSU);
    case 64:
        return (int64_t) do_ld64(env, addr, env->hflags & MIPS_HFLAG_KSU);
    }
    return 0;
}

void helper_msa_ld_df(CPUMIPSState *env, uint32_t df, uint32_t wd, uint32_t rs,
        int64_t s10)
{
    int64_t td;
    int df_bits = 8 * (1 << df);
    int i;
    target_ulong addr;
    int16_t offset = s10 << df;

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        addr = env->active_tc.gpr[rs] + offset + (i << df);
        td = msa_ld_df(env, df_bits, addr);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_st_df(CPUMIPSState *env, uint32_t df_bits,
        target_ulong addr, int64_t val)
{
    switch (df_bits) {
    case 8:
        do_st8(env, addr, val, env->hflags & MIPS_HFLAG_KSU);
        break;
    case 16:
        do_st16(env, addr, val, env->hflags & MIPS_HFLAG_KSU);
        break;
    case 32:
        do_st32(env, addr, val, env->hflags & MIPS_HFLAG_KSU);
        break;
    case 64:
        do_st64(env, addr, val, env->hflags & MIPS_HFLAG_KSU);
        break;
    }
}

void helper_msa_st_df(CPUMIPSState *env, uint32_t df, uint32_t wd, uint32_t rs,
        int64_t s10)
{
    int64_t td;
    int df_bits = 8 * (1 << df);
    int i;
    target_ulong addr;
    int16_t offset = s10 << df;

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        addr = env->active_tc.gpr[rs] + offset + (i << df);
        td = msa_load_wr_elem_i64(env, wd, df, i);
        msa_st_df(env, df_bits, addr, td);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

#define FLOAT_ONE32 make_float32(0x3f8 << 20)
#define FLOAT_ONE64 make_float64(0x3ffULL << 52)

#define FLOAT_SNAN16 (float16_default_nan ^ 0x0220)
        /* 0x7c20 */
#define FLOAT_SNAN32 (float32_default_nan ^ 0x00400020)
        /* 0x7f800020 */
#define FLOAT_SNAN64 (float64_default_nan ^ 0x0008000000000020ULL)
        /* 0x7ff0000000000020 */

static inline void clear_msacsr_cause(CPUMIPSState *env)
{
    SET_FP_CAUSE(env->active_msa.msacsr, 0);
}

static inline void check_msacsr_cause(CPUMIPSState *env)
{
    if ((GET_FP_CAUSE(env->active_msa.msacsr) &
            (GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED)) == 0) {
        UPDATE_FP_FLAGS(env->active_msa.msacsr,
                GET_FP_CAUSE(env->active_msa.msacsr));
    } else {
        helper_raise_exception(env, EXCP_MSAFPE);
    }
}

/* Flush-to-zero use cases for update_msacsr() */
#define CLEAR_FS_UNDERFLOW 1
#define CLEAR_IS_INEXACT   2
#define RECIPROCAL_INEXACT 4

static inline int update_msacsr(CPUMIPSState *env, int action, int denormal)
{
    int ieee_ex;

    int c;
    int cause;
    int enable;

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);

    /* QEMU softfloat does not signal all underflow cases */
    if (denormal) {
        ieee_ex |= float_flag_underflow;
    }

    c = ieee_ex_to_mips(env, ieee_ex);
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;

    /* Set Inexact (I) when flushing inputs to zero */
    if ((ieee_ex & float_flag_input_denormal) &&
            (env->active_msa.msacsr & MSACSR_FS_BIT) != 0) {
        if (action & CLEAR_IS_INEXACT) {
            c &= ~FP_INEXACT;
        } else {
            c |=  FP_INEXACT;
        }
    }

    /* Set Inexact (I) and Underflow (U) when flushing outputs to zero */
    if ((ieee_ex & float_flag_output_denormal) &&
            (env->active_msa.msacsr & MSACSR_FS_BIT) != 0) {
        c |= FP_INEXACT;
        if (action & CLEAR_FS_UNDERFLOW) {
            c &= ~FP_UNDERFLOW;
        } else {
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
        c &= ~FP_UNDERFLOW;
    }

    /* Reciprocal operations set only Inexact when valid and not
       divide by zero */
    if ((action & RECIPROCAL_INEXACT) &&
            (c & (FP_INVALID | FP_DIV0)) == 0) {
        c = FP_INEXACT;
    }

    cause = c & enable;    /* all current enabled exceptions */

    if (cause == 0) {
        /* No enabled exception, update the MSACSR Cause
         with all current exceptions */
        SET_FP_CAUSE(env->active_msa.msacsr,
                (GET_FP_CAUSE(env->active_msa.msacsr) | c));
    } else {
        /* Current exceptions are enabled */
        if ((env->active_msa.msacsr & MSACSR_NX_BIT) == 0) {
            /* Exception(s) will trap, update MSACSR Cause
           with all enabled exceptions */
            SET_FP_CAUSE(env->active_msa.msacsr,
                    (GET_FP_CAUSE(env->active_msa.msacsr) | c));
        }
    }

    return c;
}

#define float16_is_zero(ARG) 0
#define float16_is_zero_or_denormal(ARG) 0

#define IS_DENORMAL(ARG, BITS)                      \
    (!float ## BITS ## _is_zero(ARG)                \
    && float ## BITS ## _is_zero_or_denormal(ARG))

#define MSA_FLOAT_UNOP0(DEST, OP, ARG, BITS)                                \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
        else {                                                              \
            if (float ## BITS ## _is_any_nan(ARG)) {                        \
                DEST = 0;                                                   \
            }                                                               \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_UNOP_XD(DEST, OP, ARG, BITS, XBITS)                       \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## XBITS >> 6) << 6) | c;                   \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_UNOP(DEST, OP, ARG, BITS)                                 \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_LOGB(DEST, ARG, BITS)                                     \
    do {                                                                    \
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
            get_float_exception_flags(&env->active_msa.fp_status)           \
                                                & (~float_flag_inexact),    \
            &env->active_msa.fp_status);                                    \
                                                                            \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_BINOP(DEST, OP, ARG1, ARG2, BITS)                         \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG1, ARG2,                         \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_MAXOP(DEST, OP, ARG1, ARG2, BITS)                         \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG1, ARG2,                         \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, 0);                                       \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_RECIPROCAL(DEST, ARG, BITS)                               \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## div(FLOAT_ONE ## BITS, ARG,            \
                                         &env->active_msa.fp_status);       \
        c = update_msacsr(env, float ## BITS ## _is_infinity(ARG) ||        \
                          float ## BITS ## _is_quiet_nan(DEST) ?            \
                          0 : RECIPROCAL_INEXACT,                           \
                          IS_DENORMAL(DEST, BITS));                         \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_MULADD(DEST, ARG1, ARG2, ARG3, NEGATE, BITS)              \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _muladd(ARG2, ARG3, ARG1, NEGATE,           \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define NUMBER_QNAN_PAIR(ARG1, ARG2, BITS)      \
    !float ## BITS ## _is_any_nan(ARG1)         \
    && float ## BITS ## _is_quiet_nan(ARG2)

void helper_msa_fadd_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(W(pwx, i), add, W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(D(pwx, i), add, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fsub_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(W(pwx, i), sub, W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(D(pwx, i), sub, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fmul_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(W(pwx, i), mul, W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(D(pwx, i), mul, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fdiv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(W(pwx, i), div, W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(D(pwx, i), div, D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fsqrt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), sqrt, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), sqrt, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fexp2_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_BINOP(W(pwx, i), scalbn, W(pws, i),
                            W(pwt, i) >  0x200 ?  0x200 :
                            W(pwt, i) < -0x200 ? -0x200 : W(pwt, i),
                            32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
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

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_flog2_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_LOGB(W(pwx, i), W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_LOGB(D(pwx, i), D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmadd_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_MULADD(W(pwx, i), W(pwd, i),
                           W(pws, i), W(pwt, i), 0, 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_MULADD(D(pwx, i), D(pwd, i),
                           D(pws, i), D(pwt, i), 0, 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmsub_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_MULADD(W(pwx, i), W(pwd, i),
                           W(pws, i), W(pwt, i),
                           float_muladd_negate_product, 32);
      } DONE_ALL_ELEMENTS;
      break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_MULADD(D(pwx, i), D(pwd, i),
                           D(pws, i), D(pwt, i),
                           float_muladd_negate_product, 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

#define FMAXMIN_A(F, G, X, _S, _T, BITS)                            \
{                                                                   \
    uint## BITS ##_t S = _S, T = _T;                                \
    if (NUMBER_QNAN_PAIR(S, T, BITS)) {                             \
        T = S;                                                      \
    }                                                               \
    else if (NUMBER_QNAN_PAIR(T, S, BITS)) {                        \
        S = T;                                                      \
    }                                                               \
    uint## BITS ##_t as = float## BITS ##_abs(S);                   \
    uint## BITS ##_t at = float## BITS ##_abs(T);                   \
    uint## BITS ##_t xs, xt, xd;                                    \
    MSA_FLOAT_MAXOP(xs, F,  S,  T, BITS);                           \
    MSA_FLOAT_MAXOP(xt, G,  S,  T, BITS);                           \
    MSA_FLOAT_MAXOP(xd, F, as, at, BITS);                           \
    X = (as == at || xd == float## BITS ##_abs(xs)) ? xs : xt;      \
}

void helper_msa_fmax_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            FMAXMIN_A(max, min, W(pwx, i), W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
           FMAXMIN_A(max, min, D(pwx, i), D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmax_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pws, i), W(pws, i), 32);
            } else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pwt, i), W(pwt, i), 32);
            } else {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pws, i), W(pwt, i), 32);
            }
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pws, i), D(pws, i), 64);
            } else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pwt, i), D(pwt, i), 64);
            } else {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pws, i), D(pwt, i), 64);
            }
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmin_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            FMAXMIN_A(min, max, W(pwx, i), W(pws, i), W(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            FMAXMIN_A(min, max, D(pwx, i), D(pws, i), D(pwt, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmin_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pws, i), W(pws, i), 32);
            } else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pwt, i), W(pwt, i), 32);
            } else {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pws, i), W(pwt, i), 32);
            }
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pws, i), D(pws, i), 64);
            } else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pwt, i), D(pwt, i), 64);
            } else {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pws, i), D(pwt, i), 64);
            }
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

#define MSA_FLOAT_COND(DEST, OP, ARG1, ARG2, BITS, QUIET)                   \
    do {                                                                    \
        int c;                                                              \
        int cause;                                                          \
        int enable;                                                         \
        int64_t cond;                                                       \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        if (!QUIET) {                                                       \
            cond = float ## BITS ## _ ## OP(ARG1, ARG2,                     \
                                          &env->active_msa.fp_status);      \
        } else {                                                            \
            cond = float ## BITS ## _ ## OP ## _quiet(ARG1, ARG2,           \
                                               &env->active_msa.fp_status); \
        }                                                                   \
        DEST = cond ? M_MAX_UINT(BITS) : 0;                                 \
        c = update_msacsr(env, CLEAR_IS_INEXACT, 0);                        \
        enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
        cause = c & enable;                                                 \
                                                                            \
        if (cause) {                                                        \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_AF(DEST, ARG1, ARG2, BITS, QUIET)                 \
    do {                                                            \
        MSA_FLOAT_COND(DEST, eq, ARG1, ARG2, BITS, QUIET);          \
        if ((DEST & M_MAX_UINT(BITS)) == M_MAX_UINT(BITS)) {        \
            DEST = 0;                                               \
        }                                                           \
    } while (0)

#define MSA_FLOAT_UEQ(DEST, ARG1, ARG2, BITS, QUIET)                \
    do {                                                            \
        MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, eq, ARG1, ARG2, BITS, QUIET);      \
        }                                                           \
    } while (0)

#define MSA_FLOAT_NE(DEST, ARG1, ARG2, BITS, QUIET)                 \
    do {                                                            \
        MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);          \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, lt, ARG2, ARG1, BITS, QUIET);      \
        }                                                           \
    } while (0)

#define MSA_FLOAT_UNE(DEST, ARG1, ARG2, BITS, QUIET)                \
    do {                                                            \
        MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);      \
            if (DEST == 0) {                                        \
                MSA_FLOAT_COND(DEST, lt, ARG2, ARG1, BITS, QUIET);  \
            }                                                       \
        }                                                           \
    } while (0)

#define MSA_FLOAT_ULE(DEST, ARG1, ARG2, BITS, QUIET)                \
    do {                                                            \
        MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, le, ARG1, ARG2, BITS, QUIET);      \
        }                                                           \
    } while (0)

#define MSA_FLOAT_ULT(DEST, ARG1, ARG2, BITS, QUIET)                \
    do {                                                            \
        MSA_FLOAT_COND(DEST, unordered, ARG1, ARG2, BITS, QUIET);   \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, lt, ARG1, ARG2, BITS, QUIET);      \
        }                                                           \
    } while (0)

#define MSA_FLOAT_OR(DEST, ARG1, ARG2, BITS, QUIET)                 \
    do {                                                            \
        MSA_FLOAT_COND(DEST, le, ARG1, ARG2, BITS, QUIET);          \
        if (DEST == 0) {                                            \
            MSA_FLOAT_COND(DEST, le, ARG2, ARG1, BITS, QUIET);      \
        }                                                           \
    } while (0)

static inline void compare_af(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_AF(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_AF(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcaf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_af(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsaf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_af(env, pwd, pws, pwt, df, 0);
}

static inline void compare_eq(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_COND(W(pwx, i), eq, W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(D(pwx, i), eq, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fceq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_eq(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fseq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_eq(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ueq(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UEQ(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UEQ(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcueq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ueq(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsueq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ueq(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ne(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_NE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_NE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcne_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ne(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsne_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ne(env, pwd, pws, pwt, df, 0);
}

static inline void compare_une(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcune_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_une(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsune_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_une(env, pwd, pws, pwt, df, 0);
}

static inline void compare_le(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(W(pwx, i), le, W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(D(pwx, i), le, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcle_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_le(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsle_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_le(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ule(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_ULE(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_ULE(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcule_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ule(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsule_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ule(env, pwd, pws, pwt, df, 0);
}

static inline void compare_lt(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(W(pwx, i), lt, W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(D(pwx, i), lt, D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fclt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_lt(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fslt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_lt(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ult(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_ULT(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_ULT(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcult_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ult(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsult_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_ult(env, pwd, pws, pwt, df, 0);
}

static inline void compare_un(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(W(pwx, i), unordered, W(pws, i), W(pwt, i), 32,
                    quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_COND(D(pwx, i), unordered, D(pws, i), D(pwt, i), 64,
                    quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcun_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_un(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsun_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_un(env, pwd, pws, pwt, df, 0);
}

static inline void compare_or(CPUMIPSState *env, void *pwd, void *pws,
        void *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_OR(W(pwx, i), W(pws, i), W(pwt, i), 32, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_OR(D(pwx, i), D(pws, i), D(pwt, i), 64, quiet);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcor_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_or(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsor_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);
    compare_or(env, pwd, pws, pwt, df, 0);
}

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
            mask |= MSA_FLOAT_CLASS_SIGNALING_NAN;              \
        }                                                       \
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
        return mask;                                            \
    } while (0)

static inline int64_t msa_fclass_df(CPUMIPSState *env, uint32_t df, int64_t arg)
{
    if (df == DF_WORD) {
        MSA_FLOAT_CLASS(arg, 32);
    } else {
        MSA_FLOAT_CLASS(arg, 64);
    }
}

void helper_msa_fclass_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    uint64_t td, ts;
    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < MSA_WRLEN / df_bits; i++) {
        ts = msa_load_wr_elem_s64(env, ws, df, i);
        td = msa_fclass_df(env, df, ts);
        msa_store_wr_elem(env, td, wd, df, i);
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline float16 float16_from_float32(int32 a, flag ieee STATUS_PARAM)
{
      float16 f_val;

      f_val = float32_to_float16((float32)a, ieee  STATUS_VAR);
      f_val = float16_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 15)) : f_val;
}

static inline float32 float32_from_float64(int64 a STATUS_PARAM)
{
      float32 f_val;

      f_val = float64_to_float32((float64)a STATUS_VAR);
      f_val = float32_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 31)) : f_val;
}

static inline float32 float32_from_float16(int16_t a, flag ieee STATUS_PARAM)
{
      float32 f_val;

      f_val = float16_to_float32((float16)a, ieee STATUS_VAR);
      f_val = float32_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1 << 31)) : f_val;
}

static inline float64 float64_from_float32(int32 a STATUS_PARAM)
{
      float64 f_val;

      f_val = float32_to_float64((float64)a STATUS_VAR);
      f_val = float64_maybe_silence_nan(f_val);

      return a < 0 ? (f_val | (1ULL << 63)) : f_val;
}

void helper_msa_fexdo_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(HL(pwx, i), from_float32, W(pws, i), ieee, 16);
            MSA_FLOAT_BINOP(HR(pwx, i), from_float32, W(pwt, i), ieee, 16);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(WL(pwx, i), from_float64, D(pws, i), 32);
            MSA_FLOAT_UNOP(WR(pwx, i), from_float64, D(pwt, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fexupl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(W(pwx, i), from_float16, HL(pws, i), ieee, 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_float32, WL(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fexupr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(W(pwx, i), from_float16, HR(pws, i), ieee, 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_float32, WR(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

#define float32_from_int32 int32_to_float32
#define float32_from_uint32 uint32_to_float32

#define float64_from_int64 int64_to_float64
#define float64_from_uint64 uint64_to_float64

void helper_msa_ffint_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), from_int32, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_int64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ffint_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), from_uint32, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_uint64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftint_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_int32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_int64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftint_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_uint32, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_uint64, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftrunc_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_int32_round_to_zero, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_int64_round_to_zero, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftrunc_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_uint32_round_to_zero, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_uint64_round_to_zero, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_frint_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), round_to_int, W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), round_to_int, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

static inline float32 float32_from_q16(int16_t a STATUS_PARAM)
{
    float32 f_val;

    /* conversion as integer and scaling */
    f_val = int32_to_float32(a STATUS_VAR);
    f_val = float32_scalbn(f_val, -15 STATUS_VAR);

    return f_val;
}

static inline float64 float64_from_q32(int32 a STATUS_PARAM)
{
    float64 f_val;

    /* conversion as integer and scaling */
    f_val = int32_to_float64(a STATUS_VAR);
    f_val = float64_scalbn(f_val, -31 STATUS_VAR);

    return f_val;
}

static inline int16_t float32_to_q16(float32 a STATUS_PARAM)
{
    int32 q_val;
    int32 q_min = 0xffff8000;
    int32 q_max = 0x00007fff;

    int ieee_ex;

    if (float32_is_any_nan(a)) {
        float_raise(float_flag_invalid STATUS_VAR);
        return 0;
    }

    /* scaling */
    a = float32_scalbn(a, 15 STATUS_VAR);

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
                              STATUS_VAR);

    if (ieee_ex & float_flag_overflow) {
        float_raise(float_flag_inexact STATUS_VAR);
        return (int32)a < 0 ? q_min : q_max;
    }

    /* conversion to int */
    q_val = float32_to_int32(a STATUS_VAR);

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
                              STATUS_VAR);

    if (ieee_ex & float_flag_invalid) {
        set_float_exception_flags(ieee_ex & (~float_flag_invalid)
                                STATUS_VAR);
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int32)a < 0 ? q_min : q_max;
    }

    if (q_val < q_min) {
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int16_t)q_min;
    }

    if (q_max < q_val) {
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int16_t)q_max;
    }

    return (int16_t)q_val;
}

static inline int32 float64_to_q32(float64 a STATUS_PARAM)
{
    int64 q_val;
    int64 q_min = 0xffffffff80000000LL;
    int64 q_max = 0x000000007fffffffLL;

    int ieee_ex;

    if (float64_is_any_nan(a)) {
        float_raise(float_flag_invalid STATUS_VAR);
        return 0;
    }

    /* scaling */
    a = float64_scalbn(a, 31 STATUS_VAR);

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
            STATUS_VAR);

    if (ieee_ex & float_flag_overflow) {
        float_raise(float_flag_inexact STATUS_VAR);
        return (int64)a < 0 ? q_min : q_max;
    }

    /* conversion to integer */
    q_val = float64_to_int64(a STATUS_VAR);

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
            STATUS_VAR);

    if (ieee_ex & float_flag_invalid) {
        set_float_exception_flags(ieee_ex & (~float_flag_invalid)
                STATUS_VAR);
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int64)a < 0 ? q_min : q_max;
    }

    if (q_val < q_min) {
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int32)q_min;
    }

    if (q_max < q_val) {
        float_raise(float_flag_overflow | float_flag_inexact STATUS_VAR);
        return (int32)q_max;
    }

    return (int32)q_val;
}

void helper_msa_ffql_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), from_q16, HL(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_q32, WL(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    msa_move_v(pwd, pwx);
}

void helper_msa_ffqr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(W(pwx, i), from_q16, HR(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP(D(pwx, i), from_q32, WR(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    msa_move_v(pwd, pwx);
}

void helper_msa_ftq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);
    void *pwt = &(env->active_fpu.fpr[wt]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP_XD(HL(pwx, i), to_q16, W(pws, i), 32, 16);
            MSA_FLOAT_UNOP_XD(HR(pwx, i), to_q16, W(pwt, i), 32, 16);
        } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_UNOP_XD(WL(pwx, i), to_q32, D(pws, i), 64, 32);
            MSA_FLOAT_UNOP_XD(WR(pwx, i), to_q32, D(pwt, i), 64, 32);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}
void helper_msa_frcp_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_RECIPROCAL(W(pwx, i), W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_RECIPROCAL(D(pwx, i), D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_frsqrt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    void *pwd = &(env->active_fpu.fpr[wd]);
    void *pws = &(env->active_fpu.fpr[ws]);

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_RECIPROCAL(W(pwx, i), float32_sqrt(W(pws, i),
                    &env->active_msa.fp_status), 32);
         } DONE_ALL_ELEMENTS;
        break;
    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, MSA_WRLEN) {
            MSA_FLOAT_RECIPROCAL(D(pwx, i), float64_sqrt(D(pws, i),
                    &env->active_msa.fp_status), 64);
        } DONE_ALL_ELEMENTS;
        break;
    default:
        /* shouldn't get here */
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}


target_ulong helper_msa_cfcmsa(CPUMIPSState *env, uint32_t cs)
{
    switch (cs) {
    case MSAIR_REGISTER:
        return env->active_msa.msair;
    case MSACSR_REGISTER:
        return env->active_msa.msacsr & MSACSR_BITS;
    }

    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        switch (cs) {
        case MSAACCESS_REGISTER:
            return env->active_msa.msaaccess;
        case MSASAVE_REGISTER:
            return env->active_msa.msasave;
        case MSAMODIFY_REGISTER:
            return env->active_msa.msamodify;
        case MSAREQUEST_REGISTER:
            return env->active_msa.msarequest;
        case MSAMAP_REGISTER:
            return env->active_msa.msamap;
        case MSAUNMAP_REGISTER:
            return env->active_msa.msaunmap;
        }
    }
    return 0;
}

void helper_msa_ctcmsa(CPUMIPSState *env, target_ulong elm, uint32_t cd)
{
    switch (cd) {
    case MSAIR_REGISTER:
        break;
    case MSACSR_REGISTER:
        env->active_msa.msacsr = (int32_t)elm & MSACSR_BITS;
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
            helper_raise_exception(env, EXCP_MSAFPE);
        }
        break;
    case MSAACCESS_REGISTER:
        break;
    case MSASAVE_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msasave = (int32_t)elm;
        }
        break;
    case MSAMODIFY_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msamodify = (int32_t)elm;
        }
        break;
    case MSAREQUEST_REGISTER:
        break;
    case MSAMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msamap = (int32_t)elm;
            env->active_msa.msaaccess |= 1 << (int32_t)elm;
            return;
        }
        break;
    case MSAUNMAP_REGISTER:
        if (env->active_msa.msair & MSAIR_WRP_BIT) {
            env->active_msa.msaunmap = (int32_t)elm;
            env->active_msa.msaaccess &= ~(1 << (int32_t)elm);
        }
        break;
    }
}
