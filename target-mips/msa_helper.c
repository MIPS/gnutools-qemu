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
#define DF_QUAD   4

/* Data format min and max values */
#define DF_BITS(df) (1 << ((df) + 3))

#define DF_MAX_INT(df)  (int64_t)((1LL << (DF_BITS(df) - 1)) - 1)
#define M_MAX_INT(m)    (int64_t)((1LL << ((m)         - 1)) - 1)

#define DF_MIN_INT(df)  (int64_t)(-(1LL << (DF_BITS(df) - 1)))
#define M_MIN_INT(m)    (int64_t)(-(1LL << ((m)         - 1)))

#define DF_MAX_UINT(df) (uint64_t)(-1ULL >> (64 - DF_BITS(df)))
#define M_MAX_UINT(m)   (uint64_t)(-1ULL >> (64 - (m)))

#define UNSIGNED(x, df) ((x) & DF_MAX_UINT(df))
#define SIGNED(x, df)                                                   \
    ((((int64_t)x) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)))

/* Element-by-element access macros */
#define DF_ELEMENTS(df) (MSA_WRLEN / DF_BITS(df))

static inline void update_msamodify(CPUMIPSState *env, uint32_t wd) {
    if (env->active_msa.msair & MSAIR_WRP_BIT) {
        env->active_msa.msamodify |= (1 << wd);
    }
}

static inline void msa_move_v(wr_t *pwd, wr_t *pws)
{
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        pwd->d[i] = pws->d[i];
    }
}

void helper_msa_and_v(CPUMIPSState *env, uint32_t wd, uint32_t ws,
        uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        pwd->d[i] = pws->d[i] & pwt->d[i];
    }
    update_msamodify(env, wd);
}

void helper_msa_andi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        pwd->b[i] = pws->b[i] & i8;
    }
    update_msamodify(env, wd);
}

void helper_msa_or_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        pwd->d[i] = pws->d[i] | pwt->d[i];
    }
    update_msamodify(env, wd);
}

void helper_msa_ori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        pwd->b[i] = pws->b[i] | i8;
    }
    update_msamodify(env, wd);
}

void helper_msa_nor_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        pwd->d[i] = ~(pws->d[i] | pwt->d[i]);
    }
    update_msamodify(env, wd);
}

void helper_msa_nori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        pwd->b[i] = ~(pws->b[i] | i8);
    }
    update_msamodify(env, wd);
}

void helper_msa_xor_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        pwd->d[i] = pws->d[i] ^ pwt->d[i];
    }
    update_msamodify(env, wd);
}

void helper_msa_xori_b(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        pwd->b[i] = pws->b[i] ^ i8;
    }
    update_msamodify(env, wd);
}

#define BIT_MOVE_IF_NOT_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED(((dest & (~arg2)) | (arg1 & arg2)), df)

void helper_msa_bmnz_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        BIT_MOVE_IF_NOT_ZERO(pwd->d[i], pws->d[i], pwt->d[i], DF_DOUBLE);
    }
    update_msamodify(env, wd);
}

void helper_msa_bmnzi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
                        uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        BIT_MOVE_IF_NOT_ZERO(pwd->b[i], pws->b[i], i8, DF_BYTE);
    }
    update_msamodify(env, wd);
}

#define BIT_MOVE_IF_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED((dest & arg2) | (arg1 & (~arg2)), df)

void helper_msa_bmz_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        BIT_MOVE_IF_ZERO(pwd->d[i], pws->d[i], pwt->d[i], DF_DOUBLE);
    }
    update_msamodify(env, wd);
}

void helper_msa_bmzi_b(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        BIT_MOVE_IF_ZERO(pwd->b[i], pws->b[i], i8, DF_BYTE);
    }
    update_msamodify(env, wd);
}

#define BIT_SELECT(dest, arg1, arg2, df) \
            dest = UNSIGNED((arg1 & (~dest)) | (arg2 & dest), df)

void helper_msa_bsel_v(CPUMIPSState *env, uint32_t wd, uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
        BIT_SELECT(pwd->d[i], pws->d[i], pwt->d[i], DF_DOUBLE);
    }
    update_msamodify(env, wd);
}

void helper_msa_bseli_b(CPUMIPSState *env, uint32_t wd, uint32_t ws,
                        uint32_t i8)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
        BIT_SELECT(pwd->b[i], pws->b[i], i8, DF_BYTE);
    }
    update_msamodify(env, wd);
}

void helper_msa_copy_s_df(CPUMIPSState *env, uint32_t df, uint32_t rd,
                          uint32_t ws, uint32_t n)
{
    n %= DF_ELEMENTS(df);

    switch (df) {
    case DF_BYTE:
        env->active_tc.gpr[rd] = (int8_t)env->active_fpu.fpr[ws].wr.b[n];
        break;
    case DF_HALF:
        env->active_tc.gpr[rd] = (int16_t)env->active_fpu.fpr[ws].wr.h[n];
        break;
    case DF_WORD:
        env->active_tc.gpr[rd] = (int32_t)env->active_fpu.fpr[ws].wr.w[n];
        break;
#ifdef TARGET_MIPS64
    case DF_DOUBLE:
        env->active_tc.gpr[rd] = (int64_t)env->active_fpu.fpr[ws].wr.d[n];
        break;
#endif
    default:
        assert(0);
    }
}

void helper_msa_copy_u_df(CPUMIPSState *env, uint32_t df, uint32_t rd,
                          uint32_t ws, uint32_t n)
{
    n %= DF_ELEMENTS(df);

    switch (df) {
    case DF_BYTE:
        env->active_tc.gpr[rd] = (uint8_t)env->active_fpu.fpr[ws].wr.b[n];
        break;
    case DF_HALF:
        env->active_tc.gpr[rd] = (uint16_t)env->active_fpu.fpr[ws].wr.h[n];
        break;
    case DF_WORD:
        env->active_tc.gpr[rd] = (uint32_t)env->active_fpu.fpr[ws].wr.w[n];
        break;
#ifdef TARGET_MIPS64
    case DF_DOUBLE:
        env->active_tc.gpr[rd] = (uint64_t)env->active_fpu.fpr[ws].wr.d[n];
        break;
#endif
    default:
        assert(0);
    }
}

void helper_msa_ilvev_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwx->b[2*i]   = pwt->b[2*i];
            pwx->b[2*i+1] = pws->b[2*i];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwx->h[2*i]   = pwt->h[2*i];
            pwx->h[2*i+1] = pws->h[2*i];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwx->w[2*i]   = pwt->w[2*i];
            pwx->w[2*i+1] = pws->w[2*i];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            pwx->d[2*i]   = pwt->d[2*i];
            pwx->d[2*i+1] = pws->d[2*i];
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

void helper_msa_ilvod_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwx->b[2*i]   = pwt->b[2*i+1];
            pwx->b[2*i+1] = pws->b[2*i+1];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwx->h[2*i]   = pwt->h[2*i+1];
            pwx->h[2*i+1] = pws->h[2*i+1];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwx->w[2*i]   = pwt->w[2*i+1];
            pwx->w[2*i+1] = pws->w[2*i+1];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            pwx->d[2*i]   = pwt->d[2*i+1];
            pwx->d[2*i+1] = pws->d[2*i+1];
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

#define BR(pwr, i) (pwr->b[i])
#define BL(pwr, i) (pwr->b[i + DF_ELEMENTS(DF_BYTE)/2])

#define HR(pwr, i) (pwr->h[i])
#define HL(pwr, i) (pwr->h[i + DF_ELEMENTS(DF_HALF)/2])

#define WR(pwr, i) (pwr->w[i])
#define WL(pwr, i) (pwr->w[i + DF_ELEMENTS(DF_WORD)/2])

#define DR(pwr, i) (pwr->d[i])
#define DL(pwr, i) (pwr->d[i + DF_ELEMENTS(DF_DOUBLE)/2])

void helper_msa_ilvl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwx->b[2*i]   = BL(pwt, i);
            pwx->b[2*i+1] = BL(pws, i);
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwx->h[2*i]   = HL(pwt, i);
            pwx->h[2*i+1] = HL(pws, i);
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwx->w[2*i]   = WL(pwt, i);
            pwx->w[2*i+1] = WL(pws, i);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            pwx->d[2*i]   = DL(pwt, i);
            pwx->d[2*i+1] = DL(pws, i);
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

void helper_msa_ilvr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwx->b[2*i]   = BR(pwt, i);
            pwx->b[2*i+1] = BR(pws, i);
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwx->h[2*i]   = HR(pwt, i);
            pwx->h[2*i+1] = HR(pws, i);
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwx->w[2*i]   = WR(pwt, i);
            pwx->w[2*i+1] = WR(pws, i);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            pwx->d[2*i]   = DR(pwt, i);
            pwx->d[2*i+1] = DR(pws, i);
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

void helper_msa_pckev_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            BR(pwx, i) = pwt->b[2*i];
            BL(pwx, i) = pws->b[2*i];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            HR(pwx, i) = pwt->h[2*i];
            HL(pwx, i) = pws->h[2*i];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            WR(pwx, i) = pwt->w[2*i];
            WL(pwx, i) = pws->w[2*i];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            DR(pwx, i) = pwt->d[2*i];
            DL(pwx, i) = pws->d[2*i];
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

void helper_msa_pckod_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            BR(pwx, i) = pwt->b[2*i+1];
            BL(pwx, i) = pws->b[2*i+1];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            HR(pwx, i) = pwt->h[2*i+1];
            HL(pwx, i) = pws->h[2*i+1];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            WR(pwx, i) = pwt->w[2*i+1];
            WL(pwx, i) = pws->w[2*i+1];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_QUAD); i++) {
            DR(pwx, i) = pwt->d[2*i+1];
            DL(pwx, i) = pws->d[2*i+1];
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

void helper_msa_vshf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t n = DF_ELEMENTS(df);
    uint32_t k;
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            k = (pwd->b[i] & 0x3f) % (2 * n);
            pwx->b[i] =
                (pwd->b[i] & 0xc0) ? 0 : k < n ? pwt->b[k] : pws->b[k - n];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            k = (pwd->h[i] & 0x3f) % (2 * n);
            pwx->h[i] =
                (pwd->h[i] & 0xc0) ? 0 : k < n ? pwt->h[k] : pws->h[k - n];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            k = (pwd->w[i] & 0x3f) % (2 * n);
            pwx->w[i] =
                (pwd->w[i] & 0xc0) ? 0 : k < n ? pwt->w[k] : pws->w[k - n];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            k = (pwd->d[i] & 0x3f) % (2 * n);
            pwx->d[i] =
                (pwd->d[i] & 0xc0) ? 0 : k < n ? pwt->d[k] : pws->d[k - n];
        }
       break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

#define SHF_POS(i, imm) ((i & 0xfc) + ((imm >> (2 * (i & 0x03))) & 0x03))

void helper_msa_shf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                       uint32_t ws, uint32_t imm)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t wx, *pwx = &wx;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            pwx->b[i] = pws->b[SHF_POS(i, imm)];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwx->h[i] = pws->h[SHF_POS(i, imm)];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwx->w[i] = pws->w[SHF_POS(i, imm)];
        }
        break;
    default:
        assert(0);
    }
    msa_move_v(pwd, pwx);
    update_msamodify(env, wd);
}

static inline void msa_splat_df(CPUMIPSState *env, uint32_t df, wr_t *pwd,
                                wr_t *pws, target_ulong rt)
{
    uint32_t n = rt % DF_ELEMENTS(df);
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            pwd->b[i] = pws->b[n];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwd->h[i] = pws->h[n];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwd->w[i] = pws->w[n];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwd->d[i] = pws->d[n];
        }
       break;
    default:
        assert(0);
    }
}

void helper_msa_splat_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t rt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    msa_splat_df(env, df, pwd, pws, env->active_tc.gpr[rt]);
    update_msamodify(env, wd);
}

void helper_msa_splati_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                          uint32_t ws, uint32_t n)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    msa_splat_df(env, df, pwd, pws, n);
    update_msamodify(env, wd);
}

void helper_msa_move_v(CPUMIPSState *env, uint32_t wd, uint32_t ws)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    msa_move_v(pwd, pws);
    update_msamodify(env, wd);
}

void helper_msa_ldi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                       uint32_t s10)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    int64_t s64 = ((int64_t)s10 << 54) >> 54;
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            pwd->b[i] = (int8_t)s10;
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwd->h[i] = (int16_t)s64;
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwd->w[i] = (int32_t)s64;
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwd->d[i] = s64;
        }
       break;
    default:
        assert(0);
    }
    update_msamodify(env, wd);
}

void helper_msa_fill_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t rs)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    uint32_t i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            pwd->b[i] = (int8_t)env->active_tc.gpr[rs];
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwd->h[i] = (int16_t)env->active_tc.gpr[rs];
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwd->w[i] = (int32_t)env->active_tc.gpr[rs];
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwd->d[i] = (int64_t)env->active_tc.gpr[rs];
        }
       break;
    default:
        assert(0);
    }
    update_msamodify(env, wd);
}

void helper_msa_insert_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                          uint32_t rs_num, uint32_t n)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    target_ulong rs = env->active_tc.gpr[rs_num];

    switch (df) {
    case DF_BYTE:
        pwd->b[n] = (int8_t)rs;
        break;
    case DF_HALF:
        pwd->h[n] = (int16_t)rs;
        break;
    case DF_WORD:
        pwd->w[n] = (int32_t)rs;
        break;
    case DF_DOUBLE:
        pwd->d[n] = (int64_t)rs;
        break;
    default:
        assert(0);
    }
    update_msamodify(env, wd);
}

void helper_msa_insve_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t n)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    switch (df) {
    case DF_BYTE:
        pwd->b[n] = (int8_t)pws->b[0];
        break;
    case DF_HALF:
        pwd->h[n] = (int16_t)pws->h[0];
        break;
    case DF_WORD:
        pwd->w[n] = (int32_t)pws->w[0];
        break;
    case DF_DOUBLE:
        pwd->d[n] = (int64_t)pws->d[0];
        break;
    default:
        assert(0);
    }
    update_msamodify(env, wd);
}

#define CONCATENATE_AND_SLIDE(s, k)             \
    do {                                        \
        for (i = 0; i < s; i++) {               \
            v[i]     = pws->b[s * k + i];       \
            v[i + s] = pwd->b[s * k + i];       \
        }                                       \
        for (i = 0; i < s; i++) {               \
            pwd->b[s * k + i] = v[i + n];       \
        }                                       \
    } while (0)

static inline void msa_sld_df(CPUMIPSState *env, uint32_t df, wr_t *pwd,
                              wr_t *pws, target_ulong rt)
{
    uint32_t n = rt % DF_ELEMENTS(df);
    uint8_t v[64];
    uint32_t i, k;

    switch (df) {
    case DF_BYTE:
        CONCATENATE_AND_SLIDE(DF_ELEMENTS(DF_BYTE), 0);
        break;
    case DF_HALF:
        for (k = 0; k < 2; k++) {
            CONCATENATE_AND_SLIDE(DF_ELEMENTS(DF_HALF), k);
        }
        break;
    case DF_WORD:
        for (k = 0; k < 4; k++) {
            CONCATENATE_AND_SLIDE(DF_ELEMENTS(DF_WORD), k);
        }
        break;
    case DF_DOUBLE:
        for (k = 0; k < 8; k++) {
            CONCATENATE_AND_SLIDE(DF_ELEMENTS(DF_DOUBLE), k);
        }
        break;
    default:
        assert(0);
    }
}

void helper_msa_sld_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                       uint32_t ws, uint32_t rt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    msa_sld_df(env, df, pwd, pws, env->active_tc.gpr[rt]);
    update_msamodify(env, wd);
}

void helper_msa_sldi_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t n)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);

    msa_sld_df(env, df, pwd, pws, n);
    update_msamodify(env, wd);
}

void helper_msa_ld_df(CPUMIPSState *env, uint32_t df, uint32_t wd, uint32_t rs,
                      int64_t s10)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    target_ulong addr = env->active_tc.gpr[rs] + (s10 << df);
    int i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            pwd->b[i] = do_ld8(env, addr + (i << DF_BYTE),
                               env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            pwd->h[i] = do_ld16(env, addr + (i << DF_HALF),
                                env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            pwd->w[i] = do_ld32(env, addr + (i << DF_WORD),
                                env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            pwd->d[i] = do_ld64(env, addr + (i << DF_DOUBLE),
                                env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    }
    update_msamodify(env, wd);
}

void helper_msa_st_df(CPUMIPSState *env, uint32_t df, uint32_t wd, uint32_t rs,
                      int64_t s10)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    target_ulong addr = env->active_tc.gpr[rs] + (s10 << df);
    int i;

    switch (df) {
    case DF_BYTE:
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {
            do_st8(env, addr + (i << DF_BYTE), pwd->b[i],
                   env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_HALF:
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {
            do_st16(env, addr + (i << DF_HALF), pwd->h[i],
                    env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            do_st32(env, addr + (i << DF_WORD), pwd->w[i],
                    env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            do_st64(env, addr + (i << DF_DOUBLE), pwd->d[i],
                    env->hflags & MIPS_HFLAG_KSU);
        }
        break;
    }
    update_msamodify(env, wd);
}

static inline int64_t msa_add_a_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 + abs_arg2;
}

static inline int64_t msa_addv_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 + arg2;
}

static inline int64_t msa_subv_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 - arg2;
}

static inline int64_t msa_adds_a_df(uint32_t df, int64_t arg1, int64_t arg2)
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

static inline int64_t msa_adds_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);
    if (arg1 < 0) {
        return (min_int - arg1 < arg2) ? arg1 + arg2 : min_int;
    } else {
        return (arg2 < max_int - arg1) ? arg1 + arg2 : max_int;
    }
}

static inline uint64_t msa_adds_u_df(uint32_t df, uint64_t arg1, uint64_t arg2)
{
    uint64_t max_uint = DF_MAX_UINT(df);
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return (u_arg1 < max_uint - u_arg2) ? u_arg1 + u_arg2 : max_uint;
}

static inline int64_t msa_subs_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);
    if (arg2 > 0) {
        return (min_int + arg2 < arg1) ? arg1 - arg2 : min_int;
    } else {
        return (arg1 < max_int + arg2) ? arg1 - arg2 : max_int;
    }
}

static inline int64_t msa_subs_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return (u_arg1 > u_arg2) ? u_arg1 - u_arg2 : 0;
}

static inline int64_t msa_subsuu_s_df(uint32_t df, int64_t arg1, int64_t arg2)
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

static inline int64_t msa_subsus_u_df(uint32_t df, int64_t arg1, int64_t arg2)
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

static inline int64_t msa_asub_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    /* signed compare */
    return (arg1 < arg2) ?
        (uint64_t)(arg2 - arg1) : (uint64_t)(arg1 - arg2);
}

static inline uint64_t msa_asub_u_df(uint32_t df, uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned compare */
    return (u_arg1 < u_arg2) ?
        (uint64_t)(u_arg2 - u_arg1) : (uint64_t)(u_arg1 - u_arg2);
}

static inline int64_t msa_ave_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + (arg1 & arg2 & 1);
}

static inline uint64_t msa_ave_u_df(uint32_t df, uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + (u_arg1 & u_arg2 & 1);
}

static inline int64_t msa_aver_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + ((arg1 | arg2) & 1);
}

static inline uint64_t msa_aver_u_df(uint32_t df, uint64_t arg1, uint64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + ((u_arg1 | u_arg2) & 1);
}

/* Data format bit position and unsigned values */
#define BIT_POSITION(x, df) ((uint64_t)(x) % DF_BITS(df))

static inline int64_t msa_bclr_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 & (~(1LL << b_arg2)), df);
}

static inline int64_t msa_bneg_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 ^ (1LL << b_arg2), df);
}

static inline int64_t msa_bset_df(uint32_t df, int64_t arg1,
        int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return UNSIGNED(arg1 | (1LL << b_arg2), df);
}

static inline int64_t msa_binsl_df(uint32_t df, int64_t dest, int64_t arg1,
                                   int64_t arg2)
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

static inline int64_t msa_binsr_df(uint32_t df, int64_t dest, int64_t arg1,
                                   int64_t arg2)
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

static inline int64_t msa_ceq_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 == arg2 ? -1 : 0;
}

static inline int64_t msa_cle_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 <= arg2 ? -1 : 0;
}

static inline int64_t msa_cle_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 <= u_arg2 ? -1 : 0;
}

static inline int64_t msa_clt_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 < arg2 ? -1 : 0;
}

static inline int64_t msa_clt_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 < u_arg2 ? -1 : 0;
}

#define SIGNED_EVEN(a, df) \
        ((((int64_t)(a)) << (64 - DF_BITS(df)/2)) >> (64 - DF_BITS(df)/2))

#define UNSIGNED_EVEN(a, df) \
        ((((uint64_t)(a)) << (64 - DF_BITS(df)/2)) >> (64 - DF_BITS(df)/2))

#define SIGNED_ODD(a, df) \
        ((((int64_t)(a)) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)/2))

#define UNSIGNED_ODD(a, df) \
        ((((uint64_t)(a)) << (64 - DF_BITS(df))) >> (64 - DF_BITS(df)/2))

static inline int64_t msa_hadd_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return SIGNED_ODD(arg1, df) + SIGNED_EVEN(arg2, df);
}

static inline int64_t msa_hadd_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return UNSIGNED_ODD(arg1, df) + UNSIGNED_EVEN(arg2, df);
}

static inline int64_t msa_hsub_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return SIGNED_ODD(arg1, df) - SIGNED_EVEN(arg2, df);
}

static inline int64_t msa_hsub_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return UNSIGNED_ODD(arg1, df) - UNSIGNED_EVEN(arg2, df);
}

#define SIGNED_EXTRACT(e, o, a, df)             \
    int64_t e = SIGNED_EVEN(a, df);             \
    int64_t o = SIGNED_ODD(a, df);

#define UNSIGNED_EXTRACT(e, o, a, df)           \
    int64_t e = UNSIGNED_EVEN(a, df);           \
    int64_t o = UNSIGNED_ODD(a, df);

static inline int64_t msa_dotp_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

static inline int64_t msa_dotp_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

static inline int64_t msa_dpadd_s_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

static inline int64_t msa_dpadd_u_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

static inline int64_t msa_dpsub_s_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

static inline int64_t msa_dpsub_u_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);
    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

static inline int64_t msa_maddv_df(uint32_t df, int64_t dest, int64_t arg1,
                                   int64_t arg2)
{
    return dest + arg1 * arg2;
}

static inline int64_t msa_msubv_df(uint32_t df, int64_t dest, int64_t arg1,
                                   int64_t arg2)
{
    return dest - arg1 * arg2;
}

static inline int64_t msa_max_a_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 > abs_arg2 ? arg1 : arg2;
}

static inline int64_t msa_max_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 > arg2 ? arg1 : arg2;
}

static inline int64_t msa_max_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 > u_arg2 ? arg1 : arg2;
}

static inline int64_t msa_min_a_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;
    return abs_arg1 < abs_arg2 ? arg1 : arg2;
}

static inline int64_t msa_min_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 < arg2 ? arg1 : arg2;
}

static inline int64_t msa_min_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg1 < u_arg2 ? arg1 : arg2;
}

static inline int64_t msa_mulv_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    return arg1 * arg2;
}

static inline int64_t msa_div_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    if (arg1 == DF_MIN_INT(df) && arg2 == -1) {
        return DF_MIN_INT(df);
    }
    return arg2 ? arg1 / arg2 : 0;
}

static inline int64_t msa_div_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg2 ? u_arg1 / u_arg2 : 0;
}

static inline int64_t msa_mod_s_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    if (arg1 == DF_MIN_INT(df) && arg2 == -1) {
        return 0;
    }
    return arg2 ? arg1 % arg2 : 0;
}

static inline int64_t msa_mod_u_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);
    return u_arg2 ? u_arg1 % u_arg2 : 0;
}

static inline int64_t msa_nlzc_df(uint32_t df, int64_t arg)
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

static inline int64_t msa_nloc_df(uint32_t df, int64_t arg)
{
    return msa_nlzc_df(df, UNSIGNED((~arg), df));
}

static inline int64_t msa_pcnt_df(uint32_t df, int64_t arg)
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

static inline int64_t msa_sat_u_df(uint32_t df, int64_t arg, uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return  u_arg < M_MAX_UINT(m+1) ? u_arg :
                                      M_MAX_UINT(m+1);
}

static inline int64_t msa_sat_s_df(uint32_t df, int64_t arg, uint32_t m)
{
    return arg < M_MIN_INT(m+1) ? M_MIN_INT(m+1) :
                                  arg > M_MAX_INT(m+1) ? M_MAX_INT(m+1) :
                                                         arg;
}

static inline int64_t msa_sll_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 << b_arg2;
}

static inline int64_t msa_slli_df(uint32_t df, int64_t arg1, uint32_t m)
{
    return arg1 << m;
}

static inline int64_t msa_sra_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 >> b_arg2;
}

static inline int64_t msa_srl_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return u_arg1 >> b_arg2;
}

static inline int64_t msa_srli_df(uint32_t df, int64_t arg, uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return u_arg >> m;
}

static inline int64_t msa_srar_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    if (b_arg2 == 0) {
        return arg1;
    } else {
        int64_t r_bit = (arg1 >> (b_arg2 - 1)) & 1;
        return (arg1 >> b_arg2) + r_bit;
    }
}

static inline int64_t msa_srari_df(uint32_t df, int64_t arg, uint32_t m)
{
    if (m == 0) {
        return arg;
    } else {
        int64_t r_bit = (arg >> (m - 1)) & 1;
        return (arg >> m) + r_bit;
    }
}

static inline int64_t msa_srlr_df(uint32_t df, int64_t arg1, int64_t arg2)
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

static inline int64_t msa_srlri_df(uint32_t df, int64_t arg, uint32_t m)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    if (m == 0) {
        return u_arg;
    } else {
        uint64_t r_bit = (u_arg >> (m - 1)) & 1;
        return (u_arg >> m) + r_bit;
    }
}

static inline int64_t msa_mul_q_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int64_t q_min = DF_MIN_INT(df);
    int64_t q_max = DF_MAX_INT(df);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2) >> (DF_BITS(df) - 1);
}

static inline int64_t msa_mulr_q_df(uint32_t df, int64_t arg1, int64_t arg2)
{
    int64_t q_min = DF_MIN_INT(df);
    int64_t q_max = DF_MAX_INT(df);
    int64_t r_bit = 1 << (DF_BITS(df) - 2);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
}

static inline int64_t msa_madd_q_df(uint32_t df, int64_t dest, int64_t arg1,
                                    int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max = DF_MAX_INT(df);
    int64_t q_min = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

static inline int64_t msa_maddr_q_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max = DF_MAX_INT(df);
    int64_t q_min = DF_MIN_INT(df);
    int64_t r_bit = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

static inline int64_t msa_msub_q_df(uint32_t df, int64_t dest, int64_t arg1,
                                    int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max = DF_MAX_INT(df);
    int64_t q_min = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}

static inline int64_t msa_msubr_q_df(uint32_t df, int64_t dest, int64_t arg1,
                                     int64_t arg2)
{
    int64_t q_prod, q_ret;

    int64_t q_max = DF_MAX_INT(df);
    int64_t q_min = DF_MIN_INT(df);
    int64_t r_bit = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
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

static inline int64_t msa_fclass_df(uint32_t df, int64_t arg)
{
    if (df == DF_WORD) {
        MSA_FLOAT_CLASS(arg, 32);
    } else {
        MSA_FLOAT_CLASS(arg, 64);
    }
}

#define MSA_UNOP_DF(func) \
void helper_msa_ ## func ## _df(CPUMIPSState *env, uint32_t df,         \
                              uint32_t wd, uint32_t ws)                 \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pws->b[i]);             \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pws->h[i]);             \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pws->w[i]);             \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pws->d[i]);             \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_UNOP_DF(nlzc)
MSA_UNOP_DF(nloc)
MSA_UNOP_DF(pcnt)
MSA_UNOP_DF(fclass)
#undef MSA_UNOP_DF

#define MSA_BINOP_DF(func) \
void helper_msa_ ## func ## _df(CPUMIPSState *env, uint32_t df,         \
                                uint32_t wd, uint32_t ws, uint32_t wt)  \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pws->b[i], pwt->b[i]);  \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pws->h[i], pwt->h[i]);  \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pws->w[i], pwt->w[i]);  \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pws->d[i], pwt->d[i]);  \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_BINOP_DF(add_a)
MSA_BINOP_DF(addv)
MSA_BINOP_DF(subv)
MSA_BINOP_DF(adds_a)
MSA_BINOP_DF(adds_s)
MSA_BINOP_DF(adds_u)
MSA_BINOP_DF(subs_s)
MSA_BINOP_DF(subs_u)
MSA_BINOP_DF(subsuu_s)
MSA_BINOP_DF(subsus_u)
MSA_BINOP_DF(asub_s)
MSA_BINOP_DF(asub_u)
MSA_BINOP_DF(ave_s)
MSA_BINOP_DF(ave_u)
MSA_BINOP_DF(aver_s)
MSA_BINOP_DF(aver_u)
MSA_BINOP_DF(bclr)
MSA_BINOP_DF(bneg)
MSA_BINOP_DF(bset)
MSA_BINOP_DF(ceq)
MSA_BINOP_DF(cle_s)
MSA_BINOP_DF(cle_u)
MSA_BINOP_DF(clt_s)
MSA_BINOP_DF(clt_u)
MSA_BINOP_DF(hadd_s)
MSA_BINOP_DF(hadd_u)
MSA_BINOP_DF(hsub_s)
MSA_BINOP_DF(hsub_u)
MSA_BINOP_DF(dotp_s)
MSA_BINOP_DF(dotp_u)
MSA_BINOP_DF(max_a)
MSA_BINOP_DF(max_s)
MSA_BINOP_DF(max_u)
MSA_BINOP_DF(min_a)
MSA_BINOP_DF(min_s)
MSA_BINOP_DF(min_u)
MSA_BINOP_DF(mulv)
MSA_BINOP_DF(div_s)
MSA_BINOP_DF(div_u)
MSA_BINOP_DF(mod_s)
MSA_BINOP_DF(mod_u)
MSA_BINOP_DF(sll)
MSA_BINOP_DF(sra)
MSA_BINOP_DF(srl)
MSA_BINOP_DF(srar)
MSA_BINOP_DF(srlr)
MSA_BINOP_DF(mul_q)
MSA_BINOP_DF(mulr_q)
#undef MSA_BINOP_DF


#define MSA_BINOP_IMM_DF(helper, func)                                  \
void helper_msa_ ## helper ## _df(CPUMIPSState *env, uint32_t df, uint32_t wd, \
                       uint32_t ws, int64_t u5)                         \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pws->b[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pws->h[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pws->w[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pws->d[i], u5);         \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_BINOP_IMM_DF(addvi, addv)
MSA_BINOP_IMM_DF(subvi, subv)
MSA_BINOP_IMM_DF(ceqi, ceq)
MSA_BINOP_IMM_DF(clei_s, cle_s)
MSA_BINOP_IMM_DF(clei_u, cle_u)
MSA_BINOP_IMM_DF(clti_s, clt_s)
MSA_BINOP_IMM_DF(clti_u, clt_u)
MSA_BINOP_IMM_DF(maxi_s, max_s)
MSA_BINOP_IMM_DF(maxi_u, max_u)
MSA_BINOP_IMM_DF(mini_s, min_s)
MSA_BINOP_IMM_DF(mini_u, min_u)
#undef MSA_BINOP_IMM_DF

#define MSA_BINOP_IMMU_DF(helper, func)                                  \
void helper_msa_ ## helper ## _df(CPUMIPSState *env, uint32_t df, uint32_t wd, \
                       uint32_t ws, uint32_t u5)                        \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pws->b[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pws->h[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pws->w[i], u5);         \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pws->d[i], u5);         \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_BINOP_IMMU_DF(bclri, bclr)
MSA_BINOP_IMMU_DF(bnegi, bneg)
MSA_BINOP_IMMU_DF(bseti, bset)
MSA_BINOP_IMMU_DF(sat_u, sat_u)
MSA_BINOP_IMMU_DF(sat_s, sat_s)
MSA_BINOP_IMMU_DF(slli, slli)
MSA_BINOP_IMMU_DF(srai, sra)
MSA_BINOP_IMMU_DF(srli, srli)
MSA_BINOP_IMMU_DF(srari, srari)
MSA_BINOP_IMMU_DF(srlri, srlri)
#undef MSA_BINOP_IMMU_DF

#define MSA_TEROP_DF(func) \
void helper_msa_ ## func ## _df(CPUMIPSState *env, uint32_t df, uint32_t wd,   \
                          uint32_t ws, uint32_t wt)                     \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pwd->b[i], pws->b[i],   \
                                            pwt->b[i]);                 \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pwd->h[i], pws->h[i],   \
                                            pwt->h[i]);                 \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pwd->w[i], pws->w[i],   \
                                            pwt->w[i]);                 \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pwd->d[i], pws->d[i],   \
                                            pwt->d[i]);                 \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_TEROP_DF(binsl)
MSA_TEROP_DF(binsr)
MSA_TEROP_DF(dpadd_s)
MSA_TEROP_DF(dpadd_u)
MSA_TEROP_DF(dpsub_s)
MSA_TEROP_DF(dpsub_u)
MSA_TEROP_DF(maddv)
MSA_TEROP_DF(msubv)
MSA_TEROP_DF(madd_q)
MSA_TEROP_DF(maddr_q)
MSA_TEROP_DF(msub_q)
MSA_TEROP_DF(msubr_q)
#undef MSA_TEROP_DF

#define MSA_TEROP_IMMU_DF(helper, func)                                  \
void helper_msa_ ## helper ## _df(CPUMIPSState *env, uint32_t df,       \
                                  uint32_t wd, uint32_t ws, uint32_t u5) \
{                                                                       \
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);                          \
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);                          \
    uint32_t i;                                                         \
                                                                        \
    switch (df) {                                                       \
    case DF_BYTE:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_BYTE); i++) {                    \
            pwd->b[i] = msa_ ## func ## _df(df, pwd->b[i], pws->b[i],   \
                                            u5);                        \
        }                                                               \
        break;                                                          \
    case DF_HALF:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_HALF); i++) {                    \
            pwd->h[i] = msa_ ## func ## _df(df, pwd->h[i], pws->h[i],   \
                                            u5);                        \
        }                                                               \
        break;                                                          \
    case DF_WORD:                                                       \
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {                    \
            pwd->w[i] = msa_ ## func ## _df(df, pwd->w[i], pws->w[i],   \
                                            u5);                        \
        }                                                               \
        break;                                                          \
    case DF_DOUBLE:                                                     \
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {                  \
            pwd->d[i] = msa_ ## func ## _df(df, pwd->d[i], pws->d[i],   \
                                            u5);                        \
        }                                                               \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
    }                                                                   \
    update_msamodify(env, wd);                                          \
}

MSA_TEROP_IMMU_DF(binsli, binsl)
MSA_TEROP_IMMU_DF(binsri, binsr)
#undef MSA_TEROP_IMMU_DF

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

static inline int get_enabled_exceptions(const CPUMIPSState *env, int c) {
    int enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;
    return c & enable;
}

#define float16_is_zero(ARG) 0
#define float16_is_zero_or_denormal(ARG) 0

#define IS_DENORMAL(ARG, BITS)                      \
    (!float ## BITS ## _is_zero(ARG)                \
    && float ## BITS ## _is_zero_or_denormal(ARG))

#define MSA_FLOAT_UNOP0(DEST, OP, ARG, BITS)                                \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        } else if (float ## BITS ## _is_any_nan(ARG)) {                     \
            DEST = 0;                                                       \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_UNOP_XD(DEST, OP, ARG, BITS, XBITS)                       \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## XBITS >> 6) << 6) | c;                   \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_UNOP(DEST, OP, ARG, BITS)                                 \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG, &env->active_msa.fp_status);   \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_LOGB(DEST, ARG, BITS)                                     \
    do {                                                                    \
        int c;                                                              \
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
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_BINOP(DEST, OP, ARG1, ARG2, BITS)                         \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG1, ARG2,                         \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_MAXOP(DEST, OP, ARG1, ARG2, BITS)                         \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## OP(ARG1, ARG2,                         \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, 0);                                       \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_RECIPROCAL(DEST, ARG, BITS)                               \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _ ## div(FLOAT_ONE ## BITS, ARG,            \
                                         &env->active_msa.fp_status);       \
        c = update_msacsr(env, float ## BITS ## _is_infinity(ARG) ||        \
                          float ## BITS ## _is_quiet_nan(DEST) ?            \
                          0 : RECIPROCAL_INEXACT,                           \
                          IS_DENORMAL(DEST, BITS));                         \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
            DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                    \
        }                                                                   \
    } while (0)

#define MSA_FLOAT_MULADD(DEST, ARG1, ARG2, ARG3, NEGATE, BITS)              \
    do {                                                                    \
        int c;                                                              \
                                                                            \
        set_float_exception_flags(0, &env->active_msa.fp_status);           \
        DEST = float ## BITS ## _muladd(ARG2, ARG3, ARG1, NEGATE,           \
                                        &env->active_msa.fp_status);        \
        c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
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
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_BINOP(pwx->w[i], add, pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_BINOP(pwx->d[i], add, pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fsub_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_BINOP(pwx->w[i], sub, pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_BINOP(pwx->d[i], sub, pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fmul_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_BINOP(pwx->w[i], mul, pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_BINOP(pwx->d[i], mul, pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fdiv_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_BINOP(pwx->w[i], div, pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_BINOP(pwx->d[i], div, pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fsqrt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], sqrt, pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], sqrt, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fexp2_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_BINOP(pwx->w[i], scalbn, pws->w[i],
                            pwt->w[i] >  0x200 ?  0x200 :
                            pwt->w[i] < -0x200 ? -0x200 : pwt->w[i],
                            32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_BINOP(pwx->d[i], scalbn, pws->d[i],
                            pwt->d[i] >  0x1000 ?  0x1000 :
                            pwt->d[i] < -0x1000 ? -0x1000 : pwt->d[i],
                            64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_flog2_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_LOGB(pwx->w[i], pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_LOGB(pwx->d[i], pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmadd_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_MULADD(pwx->w[i], pwd->w[i],
                           pws->w[i], pwt->w[i], 0, 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_MULADD(pwx->d[i], pwd->d[i],
                           pws->d[i], pwt->d[i], 0, 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmsub_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_MULADD(pwx->w[i], pwd->w[i],
                           pws->w[i], pwt->w[i],
                           float_muladd_negate_product, 32);
      }
      break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_MULADD(pwx->d[i], pwd->d[i],
                           pws->d[i], pwt->d[i],
                           float_muladd_negate_product, 64);
        }
        break;
    default:
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
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            FMAXMIN_A(max, min, pwx->w[i], pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
           FMAXMIN_A(max, min, pwx->d[i], pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmax_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            if (NUMBER_QNAN_PAIR(pws->w[i], pwt->w[i], 32)) {
                MSA_FLOAT_MAXOP(pwx->w[i], max, pws->w[i], pws->w[i], 32);
            } else if (NUMBER_QNAN_PAIR(pwt->w[i], pws->w[i], 32)) {
                MSA_FLOAT_MAXOP(pwx->w[i], max, pwt->w[i], pwt->w[i], 32);
            } else {
                MSA_FLOAT_MAXOP(pwx->w[i], max, pws->w[i], pwt->w[i], 32);
            }
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            if (NUMBER_QNAN_PAIR(pws->d[i], pwt->d[i], 64)) {
                MSA_FLOAT_MAXOP(pwx->d[i], max, pws->d[i], pws->d[i], 64);
            } else if (NUMBER_QNAN_PAIR(pwt->d[i], pws->d[i], 64)) {
                MSA_FLOAT_MAXOP(pwx->d[i], max, pwt->d[i], pwt->d[i], 64);
            } else {
                MSA_FLOAT_MAXOP(pwx->d[i], max, pws->d[i], pwt->d[i], 64);
            }
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmin_a_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            FMAXMIN_A(min, max, pwx->w[i], pws->w[i], pwt->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            FMAXMIN_A(min, max, pwx->d[i], pws->d[i], pwt->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fmin_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
        uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            if (NUMBER_QNAN_PAIR(pws->w[i], pwt->w[i], 32)) {
                MSA_FLOAT_MAXOP(pwx->w[i], min, pws->w[i], pws->w[i], 32);
            } else if (NUMBER_QNAN_PAIR(pwt->w[i], pws->w[i], 32)) {
                MSA_FLOAT_MAXOP(pwx->w[i], min, pwt->w[i], pwt->w[i], 32);
            } else {
                MSA_FLOAT_MAXOP(pwx->w[i], min, pws->w[i], pwt->w[i], 32);
            }
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            if (NUMBER_QNAN_PAIR(pws->d[i], pwt->d[i], 64)) {
                MSA_FLOAT_MAXOP(pwx->d[i], min, pws->d[i], pws->d[i], 64);
            } else if (NUMBER_QNAN_PAIR(pwt->d[i], pws->d[i], 64)) {
                MSA_FLOAT_MAXOP(pwx->d[i], min, pwt->d[i], pwt->d[i], 64);
            } else {
                MSA_FLOAT_MAXOP(pwx->d[i], min, pws->d[i], pwt->d[i], 64);
            }
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

#define MSA_FLOAT_COND(DEST, OP, ARG1, ARG2, BITS, QUIET)                   \
    do {                                                                    \
        int c;                                                              \
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
                                                                            \
        if (get_enabled_exceptions(env, c)) {                               \
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

static inline void compare_af(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_AF(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_AF(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcaf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_af(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsaf_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_af(env, pwd, pws, pwt, df, 0);
}

static inline void compare_eq(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_COND(pwx->w[i], eq, pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_COND(pwx->d[i], eq, pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fceq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_eq(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fseq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_eq(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ueq(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                               wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UEQ(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UEQ(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcueq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ueq(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsueq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ueq(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ne(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet) {
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_NE(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_NE(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcne_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ne(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsne_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ne(env, pwd, pws, pwt, df, 0);
}

static inline void compare_une(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                               wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UNE(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UNE(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcune_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_une(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsune_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_une(env, pwd, pws, pwt, df, 0);
}

static inline void compare_le(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_COND(pwx->w[i], le, pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_COND(pwx->d[i], le, pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcle_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_le(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsle_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_le(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ule(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                               wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_ULE(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_ULE(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcule_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ule(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsule_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ule(env, pwd, pws, pwt, df, 0);
}

static inline void compare_lt(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_COND(pwx->w[i], lt, pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_COND(pwx->d[i], lt, pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fclt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_lt(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fslt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_lt(env, pwd, pws, pwt, df, 0);
}

static inline void compare_ult(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                               wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_ULT(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_ULT(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcult_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ult(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsult_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_ult(env, pwd, pws, pwt, df, 0);
}

static inline void compare_un(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_COND(pwx->w[i], unordered, pws->w[i], pwt->w[i], 32,
                    quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_COND(pwx->d[i], unordered, pws->d[i], pwt->d[i], 64,
                    quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcun_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_un(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsun_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_un(env, pwd, pws, pwt, df, 0);
}

static inline void compare_or(CPUMIPSState *env, wr_t *pwd, wr_t *pws,
                              wr_t *pwt, uint32_t df, int quiet)
{
    wr_t wx, *pwx = &wx;
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_OR(pwx->w[i], pws->w[i], pwt->w[i], 32, quiet);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_OR(pwx->d[i], pws->d[i], pwt->d[i], 64, quiet);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_fcor_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_or(env, pwd, pws, pwt, df, 1);
}

void helper_msa_fsor_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws, uint32_t wt)
{
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    compare_or(env, pwd, pws, pwt, df, 0);
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
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(HL(pwx, i), from_float32, pws->w[i], ieee, 16);
            MSA_FLOAT_BINOP(HR(pwx, i), from_float32, pwt->w[i], ieee, 16);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(WL(pwx, i), from_float64, pws->d[i], 32);
            MSA_FLOAT_UNOP(WR(pwx, i), from_float64, pwt->d[i], 32);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fexupl_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                          uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(pwx->w[i], from_float16, HL(pws, i), ieee, 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_float32, WL(pws, i), 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);
    msa_move_v(pwd, pwx);
}

void helper_msa_fexupr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                          uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            /* Half precision floats come in two formats: standard
               IEEE and "ARM" format.  The latter gains extra exponent
               range by omitting the NaN/Inf encodings.  */
            flag ieee = 1;

            MSA_FLOAT_BINOP(pwx->w[i], from_float16, HR(pws, i), ieee, 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_float32, WR(pws, i), 64);
        }
        break;
    default:
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
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], from_int32, pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_int64, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ffint_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                           uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], from_uint32, pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_uint64, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftint_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                           uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UNOP0(pwx->w[i], to_int32, pws->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UNOP0(pwx->d[i], to_int64, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftint_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                           uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UNOP0(pwx->w[i], to_uint32, pws->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UNOP0(pwx->d[i], to_uint64, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftrunc_s_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                            uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UNOP0(pwx->w[i], to_int32_round_to_zero, pws->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UNOP0(pwx->d[i], to_int64_round_to_zero, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_ftrunc_u_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                            uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
          MSA_FLOAT_UNOP0(pwx->w[i], to_uint32_round_to_zero, pws->w[i], 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
          MSA_FLOAT_UNOP0(pwx->d[i], to_uint64_round_to_zero, pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_frint_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                         uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], round_to_int, pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], round_to_int, pws->d[i], 64);
        }
        break;
    default:
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
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], from_q16, HL(pws, i), 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_q32, WL(pws, i), 64);
        }
        break;
    default:
        assert(0);
    }

    msa_move_v(pwd, pwx);
}

void helper_msa_ffqr_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP(pwx->w[i], from_q16, HR(pws, i), 32);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP(pwx->d[i], from_q32, WR(pws, i), 64);
        }
        break;
    default:
        assert(0);
    }

    msa_move_v(pwd, pwx);
}

void helper_msa_ftq_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                       uint32_t ws, uint32_t wt)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    wr_t *pwt = &(env->active_fpu.fpr[wt].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_UNOP_XD(HL(pwx, i), to_q16, pws->w[i], 32, 16);
            MSA_FLOAT_UNOP_XD(HR(pwx, i), to_q16, pwt->w[i], 32, 16);
        }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_UNOP_XD(WL(pwx, i), to_q32, pws->d[i], 64, 32);
            MSA_FLOAT_UNOP_XD(WR(pwx, i), to_q32, pwt->d[i], 64, 32);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}
void helper_msa_frcp_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                        uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_RECIPROCAL(pwx->w[i], pws->w[i], 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_RECIPROCAL(pwx->d[i], pws->d[i], 64);
        }
        break;
    default:
        assert(0);
    }

    check_msacsr_cause(env);

    msa_move_v(pwd, pwx);
}

void helper_msa_frsqrt_df(CPUMIPSState *env, uint32_t df, uint32_t wd,
                          uint32_t ws)
{
    wr_t wx, *pwx = &wx;
    wr_t *pwd = &(env->active_fpu.fpr[wd].wr);
    wr_t *pws = &(env->active_fpu.fpr[ws].wr);
    uint32_t i;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        for (i = 0; i < DF_ELEMENTS(DF_WORD); i++) {
            MSA_FLOAT_RECIPROCAL(pwx->w[i], float32_sqrt(pws->w[i],
                    &env->active_msa.fp_status), 32);
         }
        break;
    case DF_DOUBLE:
        for (i = 0; i < DF_ELEMENTS(DF_DOUBLE); i++) {
            MSA_FLOAT_RECIPROCAL(pwx->d[i], float64_sqrt(pws->d[i],
                    &env->active_msa.fp_status), 64);
        }
        break;
    default:
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

