/*
 * MIPS gdb server stub
 *
 * Copyright (c) 2003-2005 Fabrice Bellard
 * Copyright (c) 2013 SUSE LINUX Products GmbH
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
#include "config.h"
#include "qemu-common.h"
#include "exec/gdbstub.h"

int mips_dsp_get_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{

}

int mips_msa_set_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
  return 4;
}
int mips_msa_get_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
  return gdb_get_reg32(mem_buf, (int32_t)0);
}

#if TARGET_MIPS64
#define MIPS_ALWAYS_FP64 1
#else
#define MIPS_ALWAYS_FP64 0
#endif

int mips_fpu_set_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
    if (n == 32) {
        target_ulong tmp = ldl_p(mem_buf);
        env->active_fpu.fcr31 = tmp & 0xFF83FFFF;
        /* set rounding mode */
        restore_rounding_mode(env);
        /* set flush-to-zero mode */
        restore_flush_mode(env);
        return 4;
    } else if (n == 33) {
        /* FIR is read-only */
        return 4;
    }
    if (env->CP0_Config3 & (1 << CP0C3_MSAP)) {
        env->active_fpu.fpr[n].wr.d[0] = ldq_p(mem_buf);
        env->active_fpu.fpr[n].wr.d[1] = ldq_p(mem_buf + 8);
        return 16;
    } else if (MIPS_ALWAYS_FP64 || (env->active_fpu.fcr0 & (1 << FCR0_F64))) {
        env->active_fpu.fpr[n].d = ldq_p(mem_buf);
        return 8;
    } else {
        env->active_fpu.fpr[n].w[FP_ENDIAN_IDX] = ldl_p(mem_buf);
        return 4;
    }
}

int mips_fpu_get_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
    if (n == 32) {
        return gdb_get_reg32(mem_buf, (int32_t)env->active_fpu.fcr31);
    } else if (n == 33) {
        return gdb_get_reg32(mem_buf, (int32_t)env->active_fpu.fcr0);
    }
 
    if (env->CP0_Config3 & (1 << CP0C3_MSAP)) {
        return gdb_get_reg64(mem_buf, env->active_fpu.fpr[n].wr.d[0]) +
               gdb_get_reg64(mem_buf + 8, env->active_fpu.fpr[n].wr.d[1]);
    } else if (MIPS_ALWAYS_FP64 || (env->active_fpu.fcr0 & (1 << FCR0_F64))) {
        return gdb_get_reg64(mem_buf, env->active_fpu.fpr[n].d);
    } else {
        return gdb_get_reg32(mem_buf,
                             env->active_fpu.fpr[n].w[FP_ENDIAN_IDX]);
    }
}

int mips_cp0_get_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
    switch (n) {
    case 0:
        return gdb_get_reg32(mem_buf, (int32_t)env->CP0_Status);
    case 1:
        return gdb_get_regl(mem_buf, env->CP0_BadVAddr);
    case 2:
#ifndef CONFIG_USER_ONLY
        return gdb_get_reg32(mem_buf, (int32_t)env->CP0_Cause);
#else
	{
            int32_t cause = (int32_t)env->CP0_Cause;
            if (env->hflags & MIPS_HFLAG_BMASK)
                cause |= 0x80000000;
            return gdb_get_reg32(mem_buf, cause);
        }
#endif
    case 3:
        return gdb_get_reg32(mem_buf, (int32_t)env->CP0_Config5);
    }
}
int mips_cp0_set_reg(CPUMIPSState *env, uint8_t *mem_buf, int n)
{
    switch (n) {
    case 0:
#ifndef CONFIG_USER_ONLY
        cpu_mips_store_status(env, ldl_p(mem_buf));
#endif
	return 4;
    case 1:
        env->CP0_BadVAddr = ldtul_p(mem_buf);
	return sizeof(target_ulong);
    case 2:
#ifndef CONFIG_USER_ONLY
        cpu_mips_store_cause(env, ldl_p(mem_buf));
#endif
        return 4;
    case 3:
	env->CP0_Config5 &= ~env->CP0_Config5_rw_bitmask;
	env->CP0_Config5 |= (ldl_p(mem_buf) & env->CP0_Config5_rw_bitmask);
	compute_hflags(env);
        return 4; /*config5*/
    }
    return 0;
}

int mips_cpu_gdb_read_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;

    if (n < 32) {
        return gdb_get_regl(mem_buf, env->active_tc.gpr[n]);
    }
    switch (n) {
    case 32:
        return gdb_get_regl(mem_buf, env->active_tc.LO[0]);
    case 33:
        return gdb_get_regl(mem_buf, env->active_tc.HI[0]);
    case 34:
#ifndef CONFIG_USER_ONLY
        return gdb_get_regl(mem_buf, env->active_tc.PC |
                                     !!(env->hflags & MIPS_HFLAG_M16));
#else
	return gdb_get_regl(mem_buf, exception_resume_pc(env));
#endif
    default:
        return 0;
    }

    return 0;
}

int mips_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;
    target_ulong tmp;

    tmp = ldtul_p(mem_buf);

    if (n < 32) {
        env->active_tc.gpr[n] = tmp;
        return sizeof(target_ulong);
    }
    switch (n) {
    case 32:
        env->active_tc.LO[0] = tmp;
        break;
    case 33:
        env->active_tc.HI[0] = tmp;
        break;
    case 34:
        env->active_tc.PC = tmp & ~(target_ulong)1;
        if (tmp & 1) {
            env->hflags |= MIPS_HFLAG_M16;
        } else {
            env->hflags &= ~(MIPS_HFLAG_M16);
        }
	env->hflags &= ~(MIPS_HFLAG_BMASK);
        break;
    default:
        return 0;
    }

    return sizeof(target_ulong);
}
