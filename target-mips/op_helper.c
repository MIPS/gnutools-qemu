/*
 *  MIPS emulation helpers for qemu.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
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
#include "qemu/host-utils.h"

#include "helper.h"

#if !defined(CONFIG_USER_ONLY)
#include "exec/softmmu_exec.h"
#endif /* !defined(CONFIG_USER_ONLY) */

#ifndef CONFIG_USER_ONLY
static inline void cpu_mips_tlb_flush (CPUMIPSState *env, int flush_global);

#ifdef MIPSSIM_COMPAT
#include "sysemu/sysemu.h"
#include "mips-avp.h"

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

int cpu_mips_cacheability(CPUMIPSState *env, target_ulong vaddr, int rw)
{
    // this function doesn't care of kernel/super/user mode as it is a debug only function.
    // FIXME: MIPS64
    hwaddr physical;
    int prot;
    int cca = 2;// uncached by default

    if (vaddr <= (int32_t)0x7FFFFFFFUL) {
        /* useg */
        if ((env->CP0_Status) & (1 << CP0St_ERL)) {
            cca = 2;
        }
        //tlb based
        if (env->tlb->map_address == &r4k_map_address) {
            r4k_map_address_debug(env, &physical, &prot, &cca, vaddr, rw, 0);
        }
        //fixed mapping
        else if (env->tlb->map_address == &fixed_mmu_map_address) {
            //From KU field of Config Register
            cca = (env->CP0_Config0 >> CP0C0_KU) & 0x7;
        }
    }
    else if (vaddr < (int32_t)0xA0000000UL) {
        /* kseg0 */
        //From K0 field of Config Register
        cca = (env->CP0_Config0) & 0x7;
    }
    else if (vaddr < (int32_t)0xC0000000UL) {
        /* kseg1 */
        cca = 2; // Uncached
    }
    else if (vaddr < (int32_t)0xE0000000UL) {
        /* sseg (kseg2) */
        //tlb based
        if (env->tlb->map_address == &r4k_map_address) {
            r4k_map_address_debug(env, &physical, &prot, &cca, vaddr, rw, 0);
        }
        //fixed mapping
        else if (env->tlb->map_address == &fixed_mmu_map_address) {
            //From K23 field of Config Register
            cca = (env->CP0_Config0 >> CP0C0_K23) & 0x7;
        }
    }
    else {
        /* kseg3 */
        //tlb based
        if (env->tlb->map_address == &r4k_map_address) {
            r4k_map_address_debug(env, &physical, &prot, &cca, vaddr, rw, 0);
        }
        //fixed mapping
        else if (env->tlb->map_address == &fixed_mmu_map_address) {
            //From K23 field of Config Register
            cca = (env->CP0_Config0 >> CP0C0_K23) & 0x7;
        }
    }
    return cca;
}
#endif
#endif

#ifdef MIPSSIM_COMPAT
void helper_trace_mem_access(CPUMIPSState *env,
                                           target_ulong val,
                                           target_ulong addr,
                                           uint32_t rw_size)
{
#ifndef CONFIG_USER_ONLY
    sv_log(" : Memory %s ["TARGET_FMT_lx" "TARGET_FMT_lx" %u] = ",
            (rw_size >> 16)? "Write":"Read",
            addr,
            (target_long) cpu_mips_translate_address(env, addr, rw_size >> 16),
            cpu_mips_cacheability(env, addr, rw_size >> 16)
            );
#else
    sv_log(" : Memory %s ["TARGET_FMT_lx"] = ",
            (rw_size >> 16)? "Write":"Read",
            addr);
#endif

    switch(rw_size & 0xffff)
    {
    case 1:
        sv_log("%02x\n", (uint8_t) val);
        break;
    case 2:
        sv_log("%04x\n", (uint16_t) val);
        break;
    case 4:
        sv_log("%08x\n", (uint32_t) val);
        break;
    case 8:
        sv_log("%016lx\n", (uint64_t) val);
        break;
    default:
        sv_log("\n");
        break;
    }
}
void helper_trace_reg_access(CPUMIPSState *env, target_ulong val)
{
    sv_log("reg = "TARGET_FMT_lx"\n", val);
}
#endif

/*****************************************************************************/
/* Exceptions processing helpers */

static inline void QEMU_NORETURN do_raise_exception_err(CPUMIPSState *env,
                                                        uint32_t exception,
                                                        int error_code,
                                                        uintptr_t pc)
{
    CPUState *cs = CPU(mips_env_get_cpu(env));

    if (exception < EXCP_SC) {
        qemu_log("%s: %d %d\n", __func__, exception, error_code);
    }
    cs->exception_index = exception;
    env->error_code = error_code;

    if (pc) {
        /* now we have a real cpu fault */
        cpu_restore_state(cs, pc);
    }

    cpu_loop_exit(cs);
}

static inline void QEMU_NORETURN do_raise_exception(CPUMIPSState *env,
                                                    uint32_t exception,
                                                    uintptr_t pc)
{
    do_raise_exception_err(env, exception, 0, pc);
}

void helper_raise_exception_err(CPUMIPSState *env, uint32_t exception,
                                int error_code)
{
    do_raise_exception_err(env, exception, error_code, 0);
}

void helper_raise_exception(CPUMIPSState *env, uint32_t exception)
{
    do_raise_exception(env, exception, 0);
}

#if defined(CONFIG_USER_ONLY)
#define HELPER_LD(name, insn, type)                                     \
static inline type do_##name(CPUMIPSState *env, target_ulong addr,      \
                             int mem_idx)                               \
{                                                                       \
    return (type) insn##_raw(addr);                                     \
}
#else
#define HELPER_LD(name, insn, type)                                     \
static inline type do_##name(CPUMIPSState *env, target_ulong addr,      \
                             int mem_idx)                               \
{                                                                       \
    switch (mem_idx)                                                    \
    {                                                                   \
    case 0: return (type) cpu_##insn##_kernel(env, addr); break;        \
    case 1: return (type) cpu_##insn##_super(env, addr); break;         \
    default:                                                            \
    case 2: return (type) cpu_##insn##_user(env, addr); break;          \
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
static inline void do_##name(CPUMIPSState *env, target_ulong addr,      \
                             type val, int mem_idx)                     \
{                                                                       \
    insn##_raw(addr, val);                                              \
}
#else
#define HELPER_ST(name, insn, type)                                     \
static inline void do_##name(CPUMIPSState *env, target_ulong addr,      \
                             type val, int mem_idx)                     \
{                                                                       \
    switch (mem_idx)                                                    \
    {                                                                   \
    case 0: cpu_##insn##_kernel(env, addr, val); break;                 \
    case 1: cpu_##insn##_super(env, addr, val); break;                  \
    default:                                                            \
    case 2: cpu_##insn##_user(env, addr, val); break;                   \
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
static inline uint64_t get_HILO(CPUMIPSState *env)
{
    return ((uint64_t)(env->active_tc.HI[0]) << 32) | (uint32_t)env->active_tc.LO[0];
}

static inline target_ulong set_HIT0_LO(CPUMIPSState *env, uint64_t HILO)
{
    target_ulong tmp;
    env->active_tc.LO[0] = (int32_t)(HILO & 0xFFFFFFFF);
    tmp = env->active_tc.HI[0] = (int32_t)(HILO >> 32);
    return tmp;
}

static inline target_ulong set_HI_LOT0(CPUMIPSState *env, uint64_t HILO)
{
    target_ulong tmp = env->active_tc.LO[0] = (int32_t)(HILO & 0xFFFFFFFF);
    env->active_tc.HI[0] = (int32_t)(HILO >> 32);
    return tmp;
}

/* Multiplication variants of the vr54xx. */
target_ulong helper_muls(CPUMIPSState *env, target_ulong arg1,
                         target_ulong arg2)
{
    return set_HI_LOT0(env, 0 - ((int64_t)(int32_t)arg1 *
                                 (int64_t)(int32_t)arg2));
}

target_ulong helper_mulsu(CPUMIPSState *env, target_ulong arg1,
                          target_ulong arg2)
{
    return set_HI_LOT0(env, 0 - (uint64_t)(uint32_t)arg1 *
                       (uint64_t)(uint32_t)arg2);
}

target_ulong helper_macc(CPUMIPSState *env, target_ulong arg1,
                         target_ulong arg2)
{
    return set_HI_LOT0(env, (int64_t)get_HILO(env) + (int64_t)(int32_t)arg1 *
                       (int64_t)(int32_t)arg2);
}

target_ulong helper_macchi(CPUMIPSState *env, target_ulong arg1,
                           target_ulong arg2)
{
    return set_HIT0_LO(env, (int64_t)get_HILO(env) + (int64_t)(int32_t)arg1 *
                       (int64_t)(int32_t)arg2);
}

target_ulong helper_maccu(CPUMIPSState *env, target_ulong arg1,
                          target_ulong arg2)
{
    return set_HI_LOT0(env, (uint64_t)get_HILO(env) +
                       (uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2);
}

target_ulong helper_macchiu(CPUMIPSState *env, target_ulong arg1,
                            target_ulong arg2)
{
    return set_HIT0_LO(env, (uint64_t)get_HILO(env) +
                       (uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2);
}

target_ulong helper_msac(CPUMIPSState *env, target_ulong arg1,
                         target_ulong arg2)
{
    return set_HI_LOT0(env, (int64_t)get_HILO(env) - (int64_t)(int32_t)arg1 *
                       (int64_t)(int32_t)arg2);
}

target_ulong helper_msachi(CPUMIPSState *env, target_ulong arg1,
                           target_ulong arg2)
{
    return set_HIT0_LO(env, (int64_t)get_HILO(env) - (int64_t)(int32_t)arg1 *
                       (int64_t)(int32_t)arg2);
}

target_ulong helper_msacu(CPUMIPSState *env, target_ulong arg1,
                          target_ulong arg2)
{
    return set_HI_LOT0(env, (uint64_t)get_HILO(env) -
                       (uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2);
}

target_ulong helper_msachiu(CPUMIPSState *env, target_ulong arg1,
                            target_ulong arg2)
{
    return set_HIT0_LO(env, (uint64_t)get_HILO(env) -
                       (uint64_t)(uint32_t)arg1 * (uint64_t)(uint32_t)arg2);
}

target_ulong helper_mulhi(CPUMIPSState *env, target_ulong arg1,
                          target_ulong arg2)
{
    return set_HIT0_LO(env, (int64_t)(int32_t)arg1 * (int64_t)(int32_t)arg2);
}

target_ulong helper_mulhiu(CPUMIPSState *env, target_ulong arg1,
                           target_ulong arg2)
{
    return set_HIT0_LO(env, (uint64_t)(uint32_t)arg1 *
                       (uint64_t)(uint32_t)arg2);
}

target_ulong helper_mulshi(CPUMIPSState *env, target_ulong arg1,
                           target_ulong arg2)
{
    return set_HIT0_LO(env, 0 - (int64_t)(int32_t)arg1 *
                       (int64_t)(int32_t)arg2);
}

target_ulong helper_mulshiu(CPUMIPSState *env, target_ulong arg1,
                            target_ulong arg2)
{
    return set_HIT0_LO(env, 0 - (uint64_t)(uint32_t)arg1 *
                       (uint64_t)(uint32_t)arg2);
}

target_ulong helper_bitswap(target_ulong rt)
{
    target_ulong v = rt;
#ifdef TARGET_MIPS64
    v = ((v >> 1) & 0x5555555555555555) | ((v & 0x5555555555555555) << 1);
    v = ((v >> 2) & 0x3333333333333333) | ((v & 0x3333333333333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F0F0F0F0F) | ((v & 0x0F0F0F0F0F0F0F0F) << 4);
    return v;
#else
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    return v;
#endif
}

#ifndef CONFIG_USER_ONLY

static inline hwaddr do_translate_address(CPUMIPSState *env,
                                                      target_ulong address,
                                                      int rw)
{
    hwaddr lladdr;

    lladdr = cpu_mips_translate_address(env, address, rw);

    if (lladdr == -1LL) {
        cpu_loop_exit(CPU(mips_env_get_cpu(env)));
    } else {
        return lladdr;
    }
}

#define HELPER_LD_ATOMIC(name, insn)                                          \
target_ulong helper_##name(CPUMIPSState *env, target_ulong arg, int mem_idx)  \
{                                                                             \
    env->lladdr = do_translate_address(env, arg, 0);                          \
    env->llval = do_##insn(env, arg, mem_idx);                                \
    return env->llval;                                                        \
}
HELPER_LD_ATOMIC(ll, lw)
#ifdef TARGET_MIPS64
HELPER_LD_ATOMIC(lld, ld)
#endif
#undef HELPER_LD_ATOMIC

#define HELPER_ST_ATOMIC(name, ld_insn, st_insn, almask)                      \
target_ulong helper_##name(CPUMIPSState *env, target_ulong arg1,              \
                           target_ulong arg2, int mem_idx)                    \
{                                                                             \
    target_long tmp;                                                          \
                                                                              \
    if (arg2 & almask) {                                                      \
        env->CP0_BadVAddr = arg2;                                             \
        helper_raise_exception(env, EXCP_AdES);                               \
    }                                                                         \
    if (do_translate_address(env, arg2, 1) == env->lladdr) {                  \
        tmp = do_##ld_insn(env, arg2, mem_idx);                               \
        if (tmp == env->llval) {                                              \
            do_##st_insn(env, arg2, arg1, mem_idx);                           \
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

void helper_swl(CPUMIPSState *env, target_ulong arg1, target_ulong arg2,
                int mem_idx)
{
    do_sb(env, arg2, (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK(arg2) <= 2)
        do_sb(env, GET_OFFSET(arg2, 1), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK(arg2) <= 1)
        do_sb(env, GET_OFFSET(arg2, 2), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK(arg2) == 0)
        do_sb(env, GET_OFFSET(arg2, 3), (uint8_t)arg1, mem_idx);
}

void helper_swr(CPUMIPSState *env, target_ulong arg1, target_ulong arg2,
                int mem_idx)
{
    do_sb(env, arg2, (uint8_t)arg1, mem_idx);

    if (GET_LMASK(arg2) >= 1)
        do_sb(env, GET_OFFSET(arg2, -1), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK(arg2) >= 2)
        do_sb(env, GET_OFFSET(arg2, -2), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK(arg2) == 3)
        do_sb(env, GET_OFFSET(arg2, -3), (uint8_t)(arg1 >> 24), mem_idx);
}

#if defined(TARGET_MIPS64)
/* "half" load and stores.  We must do the memory access inline,
   or fault handling won't work.  */

#ifdef TARGET_WORDS_BIGENDIAN
#define GET_LMASK64(v) ((v) & 7)
#else
#define GET_LMASK64(v) (((v) & 7) ^ 7)
#endif

void helper_sdl(CPUMIPSState *env, target_ulong arg1, target_ulong arg2,
                int mem_idx)
{
    do_sb(env, arg2, (uint8_t)(arg1 >> 56), mem_idx);

    if (GET_LMASK64(arg2) <= 6)
        do_sb(env, GET_OFFSET(arg2, 1), (uint8_t)(arg1 >> 48), mem_idx);

    if (GET_LMASK64(arg2) <= 5)
        do_sb(env, GET_OFFSET(arg2, 2), (uint8_t)(arg1 >> 40), mem_idx);

    if (GET_LMASK64(arg2) <= 4)
        do_sb(env, GET_OFFSET(arg2, 3), (uint8_t)(arg1 >> 32), mem_idx);

    if (GET_LMASK64(arg2) <= 3)
        do_sb(env, GET_OFFSET(arg2, 4), (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK64(arg2) <= 2)
        do_sb(env, GET_OFFSET(arg2, 5), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK64(arg2) <= 1)
        do_sb(env, GET_OFFSET(arg2, 6), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK64(arg2) <= 0)
        do_sb(env, GET_OFFSET(arg2, 7), (uint8_t)arg1, mem_idx);
}

void helper_sdr(CPUMIPSState *env, target_ulong arg1, target_ulong arg2,
                int mem_idx)
{
    do_sb(env, arg2, (uint8_t)arg1, mem_idx);

    if (GET_LMASK64(arg2) >= 1)
        do_sb(env, GET_OFFSET(arg2, -1), (uint8_t)(arg1 >> 8), mem_idx);

    if (GET_LMASK64(arg2) >= 2)
        do_sb(env, GET_OFFSET(arg2, -2), (uint8_t)(arg1 >> 16), mem_idx);

    if (GET_LMASK64(arg2) >= 3)
        do_sb(env, GET_OFFSET(arg2, -3), (uint8_t)(arg1 >> 24), mem_idx);

    if (GET_LMASK64(arg2) >= 4)
        do_sb(env, GET_OFFSET(arg2, -4), (uint8_t)(arg1 >> 32), mem_idx);

    if (GET_LMASK64(arg2) >= 5)
        do_sb(env, GET_OFFSET(arg2, -5), (uint8_t)(arg1 >> 40), mem_idx);

    if (GET_LMASK64(arg2) >= 6)
        do_sb(env, GET_OFFSET(arg2, -6), (uint8_t)(arg1 >> 48), mem_idx);

    if (GET_LMASK64(arg2) == 7)
        do_sb(env, GET_OFFSET(arg2, -7), (uint8_t)(arg1 >> 56), mem_idx);
}
#endif /* TARGET_MIPS64 */

static const int multiple_regs[] = { 16, 17, 18, 19, 20, 21, 22, 23, 30 };

void helper_lwm(CPUMIPSState *env, target_ulong addr, target_ulong reglist,
                uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            env->active_tc.gpr[multiple_regs[i]] =
                (target_long)do_lw(env, addr, mem_idx);
            addr += 4;
        }
    }

    if (do_r31) {
        env->active_tc.gpr[31] = (target_long)do_lw(env, addr, mem_idx);
    }
}

void helper_swm(CPUMIPSState *env, target_ulong addr, target_ulong reglist,
                uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            do_sw(env, addr, env->active_tc.gpr[multiple_regs[i]], mem_idx);
            addr += 4;
        }
    }

    if (do_r31) {
        do_sw(env, addr, env->active_tc.gpr[31], mem_idx);
    }
}

#if defined(TARGET_MIPS64)
void helper_ldm(CPUMIPSState *env, target_ulong addr, target_ulong reglist,
                uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            env->active_tc.gpr[multiple_regs[i]] = do_ld(env, addr, mem_idx);
            addr += 8;
        }
    }

    if (do_r31) {
        env->active_tc.gpr[31] = do_ld(env, addr, mem_idx);
    }
}

void helper_sdm(CPUMIPSState *env, target_ulong addr, target_ulong reglist,
                uint32_t mem_idx)
{
    target_ulong base_reglist = reglist & 0xf;
    target_ulong do_r31 = reglist & 0x10;

    if (base_reglist > 0 && base_reglist <= ARRAY_SIZE (multiple_regs)) {
        target_ulong i;

        for (i = 0; i < base_reglist; i++) {
            do_sd(env, addr, env->active_tc.gpr[multiple_regs[i]], mem_idx);
            addr += 8;
        }
    }

    if (do_r31) {
        do_sd(env, addr, env->active_tc.gpr[31], mem_idx);
    }
}
#endif

#ifndef CONFIG_USER_ONLY
/* SMP helpers.  */
static bool mips_vpe_is_wfi(MIPSCPU *c)
{
    CPUState *cpu = CPU(c);
    CPUMIPSState *env = &c->env;

    /* If the VPE is halted but otherwise active, it means it's waiting for
       an interrupt.  */
    return cpu->halted && mips_vpe_active(env);
}

static inline void mips_vpe_wake(MIPSCPU *c)
{
    /* Dont set ->halted = 0 directly, let it be done via cpu_has_work
       because there might be other conditions that state that c should
       be sleeping.  */
    cpu_interrupt(CPU(c), CPU_INTERRUPT_WAKE);
}

static inline void mips_vpe_sleep(MIPSCPU *cpu)
{
    CPUState *cs = CPU(cpu);

    /* The VPE was shut off, really go to bed.
       Reset any old _WAKE requests.  */
    cs->halted = 1;
    cpu_reset_interrupt(cs, CPU_INTERRUPT_WAKE);
}

static inline void mips_tc_wake(MIPSCPU *cpu, int tc)
{
    CPUMIPSState *c = &cpu->env;

    /* FIXME: TC reschedule.  */
    if (mips_vpe_active(c) && !mips_vpe_is_wfi(cpu)) {
        mips_vpe_wake(cpu);
    }
}

static inline void mips_tc_sleep(MIPSCPU *cpu, int tc)
{
    CPUMIPSState *c = &cpu->env;

    /* FIXME: TC reschedule.  */
    if (!mips_vpe_active(c)) {
        mips_vpe_sleep(cpu);
    }
}

/**
 * mips_cpu_map_tc:
 * @env: CPU from which mapping is performed.
 * @tc: Should point to an int with the value of the global TC index.
 *
 * This function will transform @tc into a local index within the
 * returned #CPUMIPSState.
 */
/* FIXME: This code assumes that all VPEs have the same number of TCs,
          which depends on runtime setup. Can probably be fixed by
          walking the list of CPUMIPSStates.  */
static CPUMIPSState *mips_cpu_map_tc(CPUMIPSState *env, int *tc)
{
    MIPSCPU *cpu;
    CPUState *cs;
    CPUState *other_cs;
    int vpe_idx;
    int tc_idx = *tc;

    if (!(env->CP0_VPEConf0 & (1 << CP0VPEC0_MVP))) {
        /* Not allowed to address other CPUs.  */
        *tc = env->current_tc;
        return env;
    }

    cs = CPU(mips_env_get_cpu(env));
    vpe_idx = tc_idx / cs->nr_threads;
    *tc = tc_idx % cs->nr_threads;
    other_cs = qemu_get_cpu(vpe_idx);
    if (other_cs == NULL) {
        return env;
    }
    cpu = MIPS_CPU(other_cs);
    return &cpu->env;
}

/* The per VPE CP0_Status register shares some fields with the per TC
   CP0_TCStatus registers. These fields are wired to the same registers,
   so changes to either of them should be reflected on both registers.

   Also, EntryHi shares the bottom 8 bit ASID with TCStauts.

   These helper call synchronizes the regs for a given cpu.  */

/* Called for updates to CP0_Status.  */
static void sync_c0_status(CPUMIPSState *env, CPUMIPSState *cpu, int tc)
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
static void sync_c0_tcstatus(CPUMIPSState *cpu, int tc,
                             target_ulong v)
{
    uint32_t status;
    uint32_t tcu, tmx, tasid, tksu;
    uint32_t mask = ((1U << CP0St_CU3)
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
static void sync_c0_entryhi(CPUMIPSState *cpu, int tc)
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
target_ulong helper_mfc0_mvpcontrol(CPUMIPSState *env)
{
    return env->mvp->CP0_MVPControl;
}

target_ulong helper_mfc0_mvpconf0(CPUMIPSState *env)
{
    return env->mvp->CP0_MVPConf0;
}

target_ulong helper_mfc0_mvpconf1(CPUMIPSState *env)
{
    return env->mvp->CP0_MVPConf1;
}

target_ulong helper_mfc0_random(CPUMIPSState *env)
{
    return (int32_t)cpu_mips_get_random(env);
}

target_ulong helper_mfc0_tcstatus(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCStatus;
}

target_ulong helper_mftc0_tcstatus(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCStatus;
    else
        return other->tcs[other_tc].CP0_TCStatus;
}

target_ulong helper_mfc0_tcbind(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCBind;
}

target_ulong helper_mftc0_tcbind(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCBind;
    else
        return other->tcs[other_tc].CP0_TCBind;
}

target_ulong helper_mfc0_tcrestart(CPUMIPSState *env)
{
    return env->active_tc.PC;
}

target_ulong helper_mftc0_tcrestart(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.PC;
    else
        return other->tcs[other_tc].PC;
}

target_ulong helper_mfc0_tchalt(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCHalt;
}

target_ulong helper_mftc0_tchalt(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCHalt;
    else
        return other->tcs[other_tc].CP0_TCHalt;
}

target_ulong helper_mfc0_tccontext(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCContext;
}

target_ulong helper_mftc0_tccontext(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCContext;
    else
        return other->tcs[other_tc].CP0_TCContext;
}

target_ulong helper_mfc0_tcschedule(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCSchedule;
}

target_ulong helper_mftc0_tcschedule(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCSchedule;
    else
        return other->tcs[other_tc].CP0_TCSchedule;
}

target_ulong helper_mfc0_tcschefback(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCScheFBack;
}

target_ulong helper_mftc0_tcschefback(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.CP0_TCScheFBack;
    else
        return other->tcs[other_tc].CP0_TCScheFBack;
}

target_ulong helper_mfc0_count(CPUMIPSState *env)
{
    return (int32_t)cpu_mips_get_count(env);
}

target_ulong helper_mftc0_entryhi(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    return other->CP0_EntryHi;
}

target_ulong helper_mftc0_cause(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    int32_t tccause;
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc) {
        tccause = other->CP0_Cause;
    } else {
        tccause = other->CP0_Cause;
    }

    return tccause;
}

target_ulong helper_mftc0_status(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    return other->CP0_Status;
}

target_ulong helper_mfc0_lladdr(CPUMIPSState *env)
{
    return (int32_t)(env->lladdr >> env->CP0_LLAddr_shift);
}

target_ulong helper_mfc0_maar (CPUMIPSState *env)
{
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return 0;
    }
    if (env->CP0_MAARI < MIPS_MAAR_MAX) {
        return (int32_t) env->CP0_MAAR[env->CP0_MAARI];
    }
    return 0;
}

target_ulong helper_mfc0_watchlo(CPUMIPSState *env, uint32_t sel)
{
    return (int32_t)env->CP0_WatchLo[sel];
}

target_ulong helper_mfc0_watchhi(CPUMIPSState *env, uint32_t sel)
{
    return env->CP0_WatchHi[sel];
}

target_ulong helper_mfc0_debug(CPUMIPSState *env)
{
    target_ulong t0 = env->CP0_Debug;
    if (env->hflags & MIPS_HFLAG_DM)
        t0 |= 1 << CP0DB_DM;

    return t0;
}

target_ulong helper_mftc0_debug(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    int32_t tcstatus;
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        tcstatus = other->active_tc.CP0_Debug_tcstatus;
    else
        tcstatus = other->tcs[other_tc].CP0_Debug_tcstatus;

    /* XXX: Might be wrong, check with EJTAG spec. */
    return (other->CP0_Debug & ~((1 << CP0DB_SSt) | (1 << CP0DB_Halt))) |
            (tcstatus & ((1 << CP0DB_SSt) | (1 << CP0DB_Halt)));
}

#if defined(TARGET_MIPS64)
target_ulong helper_dmfc0_tcrestart(CPUMIPSState *env)
{
    return env->active_tc.PC;
}

target_ulong helper_dmfc0_tchalt(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCHalt;
}

target_ulong helper_dmfc0_tccontext(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCContext;
}

target_ulong helper_dmfc0_tcschedule(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCSchedule;
}

target_ulong helper_dmfc0_tcschefback(CPUMIPSState *env)
{
    return env->active_tc.CP0_TCScheFBack;
}

target_ulong helper_dmfc0_lladdr(CPUMIPSState *env)
{
    return env->lladdr >> env->CP0_LLAddr_shift;
}

target_ulong helper_dmfc0_maar (CPUMIPSState *env)
{
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return 0;
    }
    if (env->CP0_MAARI < MIPS_MAAR_MAX) {
        return env->CP0_MAAR[env->CP0_MAARI];
    }
    return 0;
}

target_ulong helper_dmfc0_watchlo(CPUMIPSState *env, uint32_t sel)
{
    return env->CP0_WatchLo[sel];
}
#endif /* TARGET_MIPS64 */

void helper_mtc0_index(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t index_p = env->CP0_Index & 0x80000000;
    uint32_t tlb_index = arg1 & 0x7fffffff;
    if (tlb_index < env->tlb->nb_tlb) {
        if (env->insn_flags & ISA_MIPS32R6) {
            // In R6 architecture CP0_Index.P field can be set to 1
            index_p |= arg1 & 0x80000000;
        }
        env->CP0_Index = index_p | tlb_index;
    }
}

void helper_mtc0_mvpcontrol(CPUMIPSState *env, target_ulong arg1)
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

void helper_mtc0_vpecontrol(CPUMIPSState *env, target_ulong arg1)
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

void helper_mttc0_vpecontrol(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);
    uint32_t mask;
    uint32_t newval;

    mask = (1 << CP0VPECo_YSI) | (1 << CP0VPECo_GSI) |
           (1 << CP0VPECo_TE) | (0xff << CP0VPECo_TargTC);
    newval = (other->CP0_VPEControl & ~mask) | (arg1 & mask);

    /* TODO: Enable/disable TCs.  */

    other->CP0_VPEControl = newval;
}

target_ulong helper_mftc0_vpecontrol(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);
    /* FIXME: Mask away return zero on read bits.  */
    return other->CP0_VPEControl;
}

target_ulong helper_mftc0_vpeconf0(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    return other->CP0_VPEConf0;
}

void helper_mtc0_vpeconf0(CPUMIPSState *env, target_ulong arg1)
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

void helper_mttc0_vpeconf0(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);
    uint32_t mask = 0;
    uint32_t newval;

    mask |= (1 << CP0VPEC0_MVP) | (1 << CP0VPEC0_VPA);
    newval = (other->CP0_VPEConf0 & ~mask) | (arg1 & mask);

    /* TODO: TC exclusive handling due to ERL/EXL.  */
    other->CP0_VPEConf0 = newval;
}

void helper_mtc0_vpeconf1(CPUMIPSState *env, target_ulong arg1)
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

void helper_mtc0_yqmask(CPUMIPSState *env, target_ulong arg1)
{
    /* Yield qualifier inputs not implemented. */
    env->CP0_YQMask = 0x00000000;
}

void helper_mtc0_vpeopt(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_VPEOpt = arg1 & 0x0000ffff;
}

static void mtc0_entrylo_common(CPUMIPSState *env, int isR6, uint64_t * CP0_EntryLo, target_ulong arg1,
                                target_ulong rixi, uint32_t write_rixi_lshift) {
    uint32_t pabits = (env->PABITS > 36) ? 36 : env->PABITS;
    uint32_t mask;
    target_ulong newval;

    mask = (1 << (30 - (36 - pabits))) - 1;
    newval = (arg1 & mask) | (rixi << write_rixi_lshift);

    if (isR6) {
        if (((arg1 >> CP0EnLo_C) & 0x6) != 0x2) {
            // Leave old C field value if new value not allowed
            newval = (newval & ~0x00000038) | (*CP0_EntryLo & 0x00000038);
        }
    }
    *CP0_EntryLo = newval;
}

void helper_mtc0_entrylo0(CPUMIPSState *env, target_ulong arg1)
{
    /* Large physaddr (PABITS) not implemented on MIPS64 */
    /* 1k pages not implemented */
    target_ulong rxie = arg1 & (env->CP0_PageGrain & (3 << CP0PG_XIE));
    mtc0_entrylo_common(env, env->insn_flags & ISA_MIPS32R6,
                        &env->CP0_EntryLo0, arg1, rxie, CP0EnLo_RI - 31);
}

#if defined(TARGET_MIPS64)
void helper_dmtc0_entrylo0(CPUMIPSState *env, uint64_t arg1)
{
    /* Large physaddr (PABITS) not implemented */
    /* 1k pages not implemented */
    uint64_t rxie = arg1 & (((uint64_t)env->CP0_PageGrain & (3 << CP0PG_XIE)) << 32);
    mtc0_entrylo_common(env, env->insn_flags & ISA_MIPS32R6,
                        &env->CP0_EntryLo0, arg1, rxie, 0);
}
#endif

#ifndef TARGET_MIPS64
static inline void xpa_mthc0(CPUMIPSState *env, uint64_t * reg, target_ulong val)
{
    unsigned int xpabits = (env->PABITS > 36) ? (env->PABITS - 36) : 0;

    if (env->CP0_PageGrain & (1 << CP0PG_ELPA)) {
        val &= (1 << xpabits) - 1;
        *reg = ((uint64_t)val << 32) | (*reg & 0x00000000ffffffffULL);
    }
}

static inline target_ulong xpa_mfhc0(CPUMIPSState *env, const uint64_t * reg)
{
    if (env->CP0_PageGrain & (1 << CP0PG_ELPA)) {
        return *reg >> 32;
    } else {
        return 0;
    }    
}

void helper_mthc0_entrylo0 (CPUMIPSState *env, target_ulong arg1)
{
    xpa_mthc0(env, &env->CP0_EntryLo0, arg1);
}

target_ulong helper_mfhc0_entrylo0 (CPUMIPSState *env)
{
    return xpa_mfhc0(env, &env->CP0_EntryLo0);
}

void helper_mthc0_entrylo1 (CPUMIPSState *env, target_ulong arg1)
{
    xpa_mthc0(env, &env->CP0_EntryLo1, arg1);
}

target_ulong helper_mfhc0_entrylo1 (CPUMIPSState *env)
{
    return xpa_mfhc0(env, &env->CP0_EntryLo1);
}

void helper_mthc0_taglo(CPUMIPSState *env, target_ulong arg1)
{
    xpa_mthc0(env, &env->CP0_TagLo, arg1);
}

target_ulong helper_mfhc0_taglo(CPUMIPSState *env)
{
    return xpa_mfhc0(env, &env->CP0_TagLo);
}

void helper_mthc0_lladdr (CPUMIPSState *env, target_ulong arg1)
{
    xpa_mthc0(env, &env->lladdr, arg1);
}

void helper_mthc0_maar (CPUMIPSState *env, target_ulong arg1)
{
    uint64_t high_mask = 0x007FFFFFULL;
    uint64_t low_mask =  0xFFFFF003ULL;
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return;
    }
    if (env->CP0_MAARI < MIPS_MAAR_MAX) {
        env->CP0_MAAR[env->CP0_MAARI] = ((arg1 & high_mask) << 32) |
                (env->CP0_MAAR[env->CP0_MAARI] & low_mask);
    }
}

target_ulong helper_mfhc0_lladdr (CPUMIPSState *env)
{
    return xpa_mfhc0(env, &env->lladdr);
}

target_ulong helper_mfhc0_maar (CPUMIPSState *env)
{
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return 0;
    }
    if (env->CP0_MAARI < MIPS_MAAR_MAX) {
        return env->CP0_MAAR[env->CP0_MAARI] >> 32;
    }
    return 0;
}
#endif

void helper_mtc0_tcstatus(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t mask = env->CP0_TCStatus_rw_bitmask;
    uint32_t newval;

    newval = (env->active_tc.CP0_TCStatus & ~mask) | (arg1 & mask);

    env->active_tc.CP0_TCStatus = newval;
    sync_c0_tcstatus(env, env->current_tc, newval);
}

void helper_mttc0_tcstatus(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCStatus = arg1;
    else
        other->tcs[other_tc].CP0_TCStatus = arg1;
    sync_c0_tcstatus(other, other_tc, arg1);
}

void helper_mtc0_tcbind(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t mask = (1 << CP0TCBd_TBE);
    uint32_t newval;

    if (env->mvp->CP0_MVPControl & (1 << CP0MVPCo_VPC))
        mask |= (1 << CP0TCBd_CurVPE);
    newval = (env->active_tc.CP0_TCBind & ~mask) | (arg1 & mask);
    env->active_tc.CP0_TCBind = newval;
}

void helper_mttc0_tcbind(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    uint32_t mask = (1 << CP0TCBd_TBE);
    uint32_t newval;
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

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

void helper_mtc0_tcrestart(CPUMIPSState *env, target_ulong arg1)
{
    env->active_tc.PC = arg1;
    env->active_tc.CP0_TCStatus &= ~(1 << CP0TCSt_TDS);
    env->lladdr = 0ULL;
    /* MIPS16 not implemented. */
}

void helper_mttc0_tcrestart(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

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

void helper_mtc0_tchalt(CPUMIPSState *env, target_ulong arg1)
{
    MIPSCPU *cpu = mips_env_get_cpu(env);

    env->active_tc.CP0_TCHalt = arg1 & 0x1;

    // TODO: Halt TC / Restart (if allocated+active) TC.
    if (env->active_tc.CP0_TCHalt & 1) {
        mips_tc_sleep(cpu, env->current_tc);
    } else {
        mips_tc_wake(cpu, env->current_tc);
    }
}

void helper_mttc0_tchalt(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);
    MIPSCPU *other_cpu = mips_env_get_cpu(other);

    // TODO: Halt TC / Restart (if allocated+active) TC.

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCHalt = arg1;
    else
        other->tcs[other_tc].CP0_TCHalt = arg1;

    if (arg1 & 1) {
        mips_tc_sleep(other_cpu, other_tc);
    } else {
        mips_tc_wake(other_cpu, other_tc);
    }
}

void helper_mtc0_tccontext(CPUMIPSState *env, target_ulong arg1)
{
    env->active_tc.CP0_TCContext = arg1;
}

void helper_mttc0_tccontext(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCContext = arg1;
    else
        other->tcs[other_tc].CP0_TCContext = arg1;
}

void helper_mtc0_tcschedule(CPUMIPSState *env, target_ulong arg1)
{
    env->active_tc.CP0_TCSchedule = arg1;
}

void helper_mttc0_tcschedule(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCSchedule = arg1;
    else
        other->tcs[other_tc].CP0_TCSchedule = arg1;
}

void helper_mtc0_tcschefback(CPUMIPSState *env, target_ulong arg1)
{
    env->active_tc.CP0_TCScheFBack = arg1;
}

void helper_mttc0_tcschefback(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.CP0_TCScheFBack = arg1;
    else
        other->tcs[other_tc].CP0_TCScheFBack = arg1;
}

void helper_mtc0_entrylo1(CPUMIPSState *env, target_ulong arg1)
{
    /* Large physaddr (PABITS) not implemented on MIPS64 */
    /* 1k pages not implemented */
    target_ulong rxie = arg1 & (env->CP0_PageGrain & (3 << CP0PG_XIE));
    mtc0_entrylo_common(env, env->insn_flags & ISA_MIPS32R6,
                        &env->CP0_EntryLo1, arg1, rxie, CP0EnLo_RI - 31);
}

#if defined(TARGET_MIPS64)
void helper_dmtc0_entrylo1(CPUMIPSState *env, uint64_t arg1)
{
    /* Large physaddr (PABITS) not implemented */
    /* 1k pages not implemented */
    uint64_t rxie = arg1 & (((uint64_t)env->CP0_PageGrain & (3 << CP0PG_XIE)) << 32);
    mtc0_entrylo_common(env, env->insn_flags & ISA_MIPS32R6,
                        &env->CP0_EntryLo1, arg1, rxie, 0);
}
#endif

void helper_mtc0_context(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Context = (env->CP0_Context & 0x007FFFFF) | (arg1 & ~0x007FFFFF);
}

void helper_mtc0_pagemask(CPUMIPSState *env, target_ulong arg1)
{
    uint64_t mask = arg1 >> (TARGET_PAGE_BITS + 1);
    /* Write new value only if valid */
    if ((arg1 == (target_ulong)-1) ||
        (mask == 0x0000 || mask == 0x0003 || mask == 0x000F ||
         mask == 0x003F || mask == 0x00FF || mask == 0x03FF ||
         mask == 0x0FFF || mask == 0x3FFF || mask == 0xFFFF)) {
        env->CP0_PageMask = arg1 & (0x1FFFFFFF & (TARGET_PAGE_MASK << 1));
    }
}

void helper_mtc0_pagegrain(CPUMIPSState *env, target_ulong arg1)
{
    /* SmartMIPS not implemented */
    /* Large physaddr (PABITS) not implemented on MIPS64 */
    /* 1k pages not implemented */
    env->CP0_PageGrain = (arg1 & env->CP0_PageGrain_rw_bitmask) |
                         (env->CP0_PageGrain & ~env->CP0_PageGrain_rw_bitmask);
}

void helper_mtc0_wired(CPUMIPSState *env, target_ulong arg1)
{
    if (arg1 < env->tlb->nb_tlb) {
        env->CP0_Wired = arg1;
    }
}

void helper_mtc0_srsconf0(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_SRSConf0 |= arg1 & env->CP0_SRSConf0_rw_bitmask;
}

void helper_mtc0_srsconf1(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_SRSConf1 |= arg1 & env->CP0_SRSConf1_rw_bitmask;
}

void helper_mtc0_srsconf2(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_SRSConf2 |= arg1 & env->CP0_SRSConf2_rw_bitmask;
}

void helper_mtc0_srsconf3(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_SRSConf3 |= arg1 & env->CP0_SRSConf3_rw_bitmask;
}

void helper_mtc0_srsconf4(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_SRSConf4 |= arg1 & env->CP0_SRSConf4_rw_bitmask;
}

void helper_mtc0_hwrena(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t mask = 0x0000000F;

    if (env->CP0_Config3 & (1 << CP0C3_ULRI)) {
        mask |= 0x20000000;
    }

    env->CP0_HWREna = arg1 & mask;
}

void helper_mtc0_count(CPUMIPSState *env, target_ulong arg1)
{
    cpu_mips_store_count(env, arg1);
}

void helper_mtc0_entryhi(CPUMIPSState *env, target_ulong arg1)
{
    target_ulong old, val, mask;
    mask = ((TARGET_PAGE_MASK << 1) | 0xFF);
    if (env->insn_flags & INSN_TLBINV) {
        mask |= 1 << CP0EnHi_EHINV;
    }

    /* 1k pages not implemented */
    val = arg1 & ((TARGET_PAGE_MASK << 1) | 0xFF);
    if (((env->CP0_Config4 >> CP0C4_IE) & 0x3) >= 2) {
        val |= arg1 & (1 << CP0EntryHiEHINV);
    }
#if defined(TARGET_MIPS64)
    if ((arg1 >> 62) == 0x2) {
        mask &= ~(0x3ull << 62); // reserved value
    }
    mask &= env->SEGMask;
#endif
    old = env->CP0_EntryHi;
    val = (arg1 & mask) | (old & ~mask);
    env->CP0_EntryHi = val;
    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        sync_c0_entryhi(env, env->current_tc);
    }
    /* If the ASID changes, flush qemu's TLB.  */
    if ((old & 0xFF) != (val & 0xFF))
        cpu_mips_tlb_flush(env, 1);
}

void helper_mttc0_entryhi(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    other->CP0_EntryHi = arg1;
    sync_c0_entryhi(other, other_tc);
}

void helper_mtc0_compare(CPUMIPSState *env, target_ulong arg1)
{
    cpu_mips_store_compare(env, arg1);
}

void helper_mtc0_status(CPUMIPSState *env, target_ulong arg1)
{
    MIPSCPU *cpu = mips_env_get_cpu(env);
    uint32_t val, old;
    uint32_t mask = env->CP0_Status_rw_bitmask;
    if (((env->CP0_Status >> CP0St_KSU) & 0x3) == 3) {
        // leave the field unmodified on illegal value write
        mask &= ~(3 << CP0St_KSU);
    }

    // CP0St_SR and CP0St_NMI: ignore a write of 1
    mask &= ~(0x00180000 & arg1);
    val = arg1 & mask;
    old = env->CP0_Status;
    env->CP0_Status = (env->CP0_Status & ~mask) | val;
    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        sync_c0_status(env, env, env->current_tc);
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
        default:
            cpu_abort(CPU(cpu), "Invalid MMU mode!\n");
            break;
        }
    }
}

void helper_mttc0_status(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    other->CP0_Status = arg1 & ~0xf1000018;
    sync_c0_status(env, other, other_tc);
}

void helper_mtc0_intctl(CPUMIPSState *env, target_ulong arg1)
{
    int vs = arg1 & 0x000003e0;
    if (vs & (vs - 1)) {
        vs = env->CP0_IntCtl & 0x000003e0;
    }
    /* vectored interrupts not implemented, no performance counters. */
    env->CP0_IntCtl = (env->CP0_IntCtl & ~0x000003e0) | vs;
}

void helper_mtc0_srsctl(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t srs_hss = (env->CP0_SRSCtl >> CP0SRSCtl_HSS) & 0xf;
    uint32_t arg_ess = (arg1 >> CP0SRSCtl_ESS) & 0xf;
    uint32_t arg_pss = (arg1 >> CP0SRSCtl_PSS) & 0xf;
    uint32_t mask = 0;

    if (arg_ess <= srs_hss) {
        mask |= 0xf << CP0SRSCtl_ESS;
    }

    if (arg_pss <= srs_hss) {
        mask |= 0xf << CP0SRSCtl_PSS;
    }

    env->CP0_SRSCtl = (env->CP0_SRSCtl & ~mask) | (arg1 & mask);
}

static void mtc0_cause(CPUMIPSState *cpu, target_ulong arg1)
{
    uint32_t mask = 0x00C00300;
    uint32_t old = cpu->CP0_Cause;
    int i;

    if (cpu->insn_flags & ISA_MIPS32R2) {
        mask |= 1 << CP0Ca_DC;
    }

    mask &= ~((1 << CP0Ca_WP) & arg1); // CP0Ca_WP: ignore a write of 1
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

void helper_mtc0_cause(CPUMIPSState *env, target_ulong arg1)
{
    mtc0_cause(env, arg1);
}

void helper_mttc0_cause(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    mtc0_cause(other, arg1);
}

target_ulong helper_mftc0_epc(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    return other->CP0_EPC;
}

target_ulong helper_mftc0_ebase(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    return other->CP0_EBase;
}

void helper_mtc0_ebase(CPUMIPSState *env, target_ulong arg1)
{
    /* vectored interrupts not implemented */
    env->CP0_EBase = (env->CP0_EBase & ~0x3FFFF000) | (arg1 & 0x3FFFF000);
}

void helper_mttc0_ebase(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);
    other->CP0_EBase = (other->CP0_EBase & ~0x3FFFF000) | (arg1 & 0x3FFFF000);
}

target_ulong helper_mftc0_configx(CPUMIPSState *env, target_ulong idx)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

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

void helper_mtc0_config0(CPUMIPSState *env, target_ulong arg1)
{
    uint32_t mask = 0;
    uint32_t is_preR6 = !(env->insn_flags & ISA_MIPS32R6);
    if (is_preR6 || (arg1 & 0x6) == 0x2) { // Allowed CCA: 2 or 3
        mask |= 0x00000007; // K0
    }
    if (((env->CP0_Config0 >> CP0C0_MT) & 0x7) == 3) {
        // Fixed Mapping MMU: K32 and KU fields available
        if (is_preR6 || (((arg1 >> CP0C0_K23) & 0x6) == 0x2)) {
            mask |= (0x7 << CP0C0_K23);
        }
        if (is_preR6 || ((arg1 >> CP0C0_KU) & 0x6) == 0x2) {
            mask |= (0x7 << CP0C0_KU);
        }
    }
    env->CP0_Config0 = (env->CP0_Config0 & ~mask) | (arg1 & mask);
}

void helper_mtc0_config2(CPUMIPSState *env, target_ulong arg1)
{
    /* tertiary/secondary caches not implemented */
    env->CP0_Config2 = (env->CP0_Config2 & 0x8FFF0FFF);
}

void helper_mtc0_config4(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Config4 = (env->CP0_Config4 & (~env->CP0_Config4_rw_bitmask)) |
                       (arg1 & env->CP0_Config4_rw_bitmask);
}

void helper_mtc0_config5(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Config5 = (env->CP0_Config5 & (~env->CP0_Config5_rw_bitmask)) |
                       (arg1 & env->CP0_Config5_rw_bitmask);
}

void helper_mtc0_lladdr(CPUMIPSState *env, target_ulong arg1)
{
    target_long mask = env->CP0_LLAddr_rw_bitmask;
    arg1 = arg1 << env->CP0_LLAddr_shift;
    env->lladdr = (env->lladdr & ~mask) | (arg1 & mask);
}

void helper_mtc0_maar (CPUMIPSState *env, target_ulong arg1)
{
    uint64_t mask = 0x7FFFFFFFFFF003ULL;
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return;
    }
    if (env->CP0_MAARI < MIPS_MAAR_MAX) {
        env->CP0_MAAR[env->CP0_MAARI]= arg1 & mask;
    }
}

void helper_mtc0_maari (CPUMIPSState *env, target_ulong arg1)
{
    int index = arg1 & 0x3f;
    if (!(env->CP0_Config5 & (1 << CP0C5_MRP))) {
        return;
    }
    if (index == 0x3f) {
        // Software may write all ones to INDEX to determine the
        // maximum value supported.
        env->CP0_MAARI = MIPS_MAAR_MAX - 1;
    }
    else if (index < MIPS_MAAR_MAX) {
        env->CP0_MAARI = arg1;
    }
    // Other than the all ones, if the
    // value written is not supported, then INDEX is unchanged
    // from its previous value.
}

void helper_mtc0_watchlo(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    /* Watch exceptions for instructions, data loads, data stores
       not implemented. */
    env->CP0_WatchLo[sel] = (arg1 & ~0x7);
}

void helper_mtc0_watchhi(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    target_ulong mask = 0x40FF0FF8;
    env->CP0_WatchHi[sel] = (arg1 & mask) | (env->CP0_WatchHi[sel] & ~mask);
    env->CP0_WatchHi[sel] &= ~(env->CP0_WatchHi[sel] & arg1 & 0x7);
}

void helper_mtc0_xcontext(CPUMIPSState *env, target_ulong arg1)
{
    target_ulong mask = (1ULL << (env->SEGBITS - 7)) - 1;
    env->CP0_XContext = (env->CP0_XContext & mask) | (arg1 & ~mask);
}

void helper_mtc0_framemask(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Framemask = arg1; /* XXX */
}

void helper_mtc0_debug(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Debug = (env->CP0_Debug & 0x8C03FC1F) | (arg1 & 0x13300100);
}

void helper_mttc0_debug(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    uint32_t val = arg1 & ((1 << CP0DB_SSt) | (1 << CP0DB_Halt));
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    /* XXX: Might be wrong, check with EJTAG spec. */
    if (other_tc == other->current_tc)
        other->active_tc.CP0_Debug_tcstatus = val;
    else
        other->tcs[other_tc].CP0_Debug_tcstatus = val;
    other->CP0_Debug = (other->CP0_Debug &
                     ((1 << CP0DB_SSt) | (1 << CP0DB_Halt))) |
                     (arg1 & ~((1 << CP0DB_SSt) | (1 << CP0DB_Halt)));
}

void helper_mtc0_performance0(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_Performance0 = arg1 & 0x000007ff;
}

void helper_mtc0_taglo(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_TagLo = arg1 & 0xFFFFFCF6;
}

void helper_mtc0_datalo(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_DataLo = arg1; /* XXX */
}

void helper_mtc0_taghi(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_TagHi = arg1; /* XXX */
}

void helper_mtc0_datahi(CPUMIPSState *env, target_ulong arg1)
{
    env->CP0_DataHi = arg1; /* XXX */
}

void helper_mtc0_kscratch (CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    if ((1 << sel) & (0xff & (env->CP0_Config4 >> CP0C4_KScrExist))) {
        env->CP0_KScratch[sel-2] = arg1;
    }
}

target_ulong helper_mfc0_kscratch (CPUMIPSState *env, uint32_t sel)
{
    if ((1 << sel) & (0xff & (env->CP0_Config4 >> CP0C4_KScrExist))) {
        return env->CP0_KScratch[sel-2];
    } else {
        return 0;
    }
}

/* MIPS MT functions */
target_ulong helper_mftgpr(CPUMIPSState *env, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.gpr[sel];
    else
        return other->tcs[other_tc].gpr[sel];
}

target_ulong helper_mftlo(CPUMIPSState *env, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.LO[sel];
    else
        return other->tcs[other_tc].LO[sel];
}

target_ulong helper_mfthi(CPUMIPSState *env, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.HI[sel];
    else
        return other->tcs[other_tc].HI[sel];
}

target_ulong helper_mftacx(CPUMIPSState *env, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.ACX[sel];
    else
        return other->tcs[other_tc].ACX[sel];
}

target_ulong helper_mftdsp(CPUMIPSState *env)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        return other->active_tc.DSPControl;
    else
        return other->tcs[other_tc].DSPControl;
}

void helper_mttgpr(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.gpr[sel] = arg1;
    else
        other->tcs[other_tc].gpr[sel] = arg1;
}

void helper_mttlo(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.LO[sel] = arg1;
    else
        other->tcs[other_tc].LO[sel] = arg1;
}

void helper_mtthi(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.HI[sel] = arg1;
    else
        other->tcs[other_tc].HI[sel] = arg1;
}

void helper_mttacx(CPUMIPSState *env, target_ulong arg1, uint32_t sel)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

    if (other_tc == other->current_tc)
        other->active_tc.ACX[sel] = arg1;
    else
        other->tcs[other_tc].ACX[sel] = arg1;
}

void helper_mttdsp(CPUMIPSState *env, target_ulong arg1)
{
    int other_tc = env->CP0_VPEControl & (0xff << CP0VPECo_TargTC);
    CPUMIPSState *other = mips_cpu_map_tc(env, &other_tc);

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

target_ulong helper_dvpe(CPUMIPSState *env)
{
    CPUState *other_cs = first_cpu;
    target_ulong prev = env->mvp->CP0_MVPControl;

    CPU_FOREACH(other_cs) {
        MIPSCPU *other_cpu = MIPS_CPU(other_cs);
        /* Turn off all VPEs except the one executing the dvpe.  */
        if (&other_cpu->env != env) {
            other_cpu->env.mvp->CP0_MVPControl &= ~(1 << CP0MVPCo_EVP);
            mips_vpe_sleep(other_cpu);
        }
    }
    return prev;
}

target_ulong helper_evpe(CPUMIPSState *env)
{
    CPUState *other_cs = first_cpu;
    target_ulong prev = env->mvp->CP0_MVPControl;

    CPU_FOREACH(other_cs) {
        MIPSCPU *other_cpu = MIPS_CPU(other_cs);

        if (&other_cpu->env != env
            /* If the VPE is WFI, don't disturb its sleep.  */
            && !mips_vpe_is_wfi(other_cpu)) {
            /* Enable the VPE.  */
            other_cpu->env.mvp->CP0_MVPControl |= (1 << CP0MVPCo_EVP);
            mips_vpe_wake(other_cpu); /* And wake it up.  */
        }
    }
    return prev;
}
#endif /* !CONFIG_USER_ONLY */

void helper_fork(target_ulong arg1, target_ulong arg2)
{
    // arg1 = rt, arg2 = rs
    // TODO: store to TC register
}

target_ulong helper_yield(CPUMIPSState *env, target_ulong arg)
{
    target_long arg1 = arg;

    if (arg1 < 0) {
        /* No scheduling policy implemented. */
        if (arg1 != -2) {
            if (env->CP0_VPEControl & (1 << CP0VPECo_YSI) &&
                env->active_tc.CP0_TCStatus & (1 << CP0TCSt_DT)) {
                env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
                env->CP0_VPEControl |= 4 << CP0VPECo_EXCPT;
                helper_raise_exception(env, EXCP_THREAD);
            }
        }
    } else if (arg1 == 0) {
        if (0 /* TODO: TC underflow */) {
            env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
            helper_raise_exception(env, EXCP_THREAD);
        } else {
            // TODO: Deallocate TC
        }
    } else if (arg1 > 0) {
        /* Yield qualifier inputs not implemented. */
        env->CP0_VPEControl &= ~(0x7 << CP0VPECo_EXCPT);
        env->CP0_VPEControl |= 2 << CP0VPECo_EXCPT;
        helper_raise_exception(env, EXCP_THREAD);
    }
    return env->CP0_YQMask;
}

#ifndef CONFIG_USER_ONLY
/* TLB management */
static void cpu_mips_tlb_flush (CPUMIPSState *env, int flush_global)
{
    MIPSCPU *cpu = mips_env_get_cpu(env);

    /* Flush qemu's TLB and discard all shadowed entries.  */
    tlb_flush(CPU(cpu), flush_global);
    env->tlb->tlb_in_use = env->tlb->nb_tlb;
}

static void r4k_mips_tlb_flush_extra (CPUMIPSState *env, int first)
{
    /* Discard entries from env->tlb[first] onwards.  */
    while (env->tlb->tlb_in_use > first) {
        r4k_invalidate_tlb(env, --env->tlb->tlb_in_use, 0);
    }
}

static void r4k_fill_tlb(CPUMIPSState *env, int idx)
{
    r4k_tlb_t *tlb;
    uint64_t mask = (uint32_t)env->CP0_PageMask >> (TARGET_PAGE_BITS + 1);
    // if mask is invalid then set all bits to 1
    if (mask & (mask + 1)) {
        mask = -1;
    }

    /* XXX: detect conflicting TLBs and raise a MCHECK exception when needed */
    tlb = &env->tlb->mmu.r4k.tlb[idx];
    if (env->CP0_EntryHi & (1 << CP0EnHi_EHINV)) {
        tlb->EHINV = 1;
        return;
    }
    tlb->EHINV = 0;
    tlb->VPN = env->CP0_EntryHi & (~mask << (TARGET_PAGE_BITS + 1));
#if defined(TARGET_MIPS64)
    tlb->VPN &= env->SEGMask;
#endif
    tlb->ASID = env->CP0_EntryHi & 0xFF;
    tlb->PageMask = env->CP0_PageMask;
    tlb->G = env->CP0_EntryLo0 & env->CP0_EntryLo1 & 1;
    tlb->V0 = (env->CP0_EntryLo0 & 2) != 0;
    tlb->D0 = (env->CP0_EntryLo0 & 4) != 0;
    tlb->C0 = (env->CP0_EntryLo0 >> 3) & 0x7;
    tlb->XI0 = (env->CP0_EntryLo0 >> CP0EnLo_XI) & 1;
    tlb->RI0 = (env->CP0_EntryLo0 >> CP0EnLo_RI) & 1;
#if defined(TARGET_MIPS64)
    tlb->PFN[0] = ((env->CP0_EntryLo0 >> 6) & ~mask);
#else
    tlb->PFN[0] = ((env->CP0_EntryLo0 & 0x3fffffff) >> 6 | /* PFN */
                   (env->CP0_EntryLo0 >> 32) << 24) & ~mask; /* PFNX */
#endif
    tlb->V1 = (env->CP0_EntryLo1 & 2) != 0;
    tlb->D1 = (env->CP0_EntryLo1 & 4) != 0;
    tlb->C1 = (env->CP0_EntryLo1 >> 3) & 0x7;
    tlb->XI1 = (env->CP0_EntryLo1 >> CP0EnLo_XI) & 1;
    tlb->RI1 = (env->CP0_EntryLo1 >> CP0EnLo_RI) & 1;
#if defined(TARGET_MIPS64)
    tlb->PFN[1] = ((env->CP0_EntryLo1 >> 6) & ~mask);
#else
    tlb->PFN[1] = ((env->CP0_EntryLo1 & 0x3fffffff) >> 6 | /* PFN */
                   (env->CP0_EntryLo1 >> 32) << 24) & ~mask; /* PFNX */
#endif

#ifdef MIPSSIM_COMPAT
    sv_log("FILL TLB index %d, ", idx);
    sv_log("VPN 0x" TARGET_FMT_lx ", ", tlb->VPN);
    sv_log("PFN0 0x%016" PRIx64 " ", tlb->PFN[0] << 12);
    sv_log("PFN1 0x%016" PRIx64 " ", tlb->PFN[1] << 12);
    sv_log("mask 0x%08x ", tlb->PageMask);
    sv_log("G %x ", tlb->G);
    sv_log("V0 %x ", tlb->V0);
    sv_log("V1 %x ", tlb->V1);
    sv_log("D0 %x ", tlb->D0);
    sv_log("D1 %x ", tlb->D1);
    sv_log("ASID %08x\n", tlb->ASID);

    sv_log(" : Write TLB Entry[%d] = ", idx);
    sv_log("%08x ", env->CP0_PageMask);
    sv_log("0x" TARGET_FMT_lx " ", env->CP0_EntryHi);
    sv_log("%016" PRIx64 " ", (uint64_t)(env->CP0_EntryLo1 & ~1ULL) | tlb->G);
    sv_log("%016" PRIx64 "\n", (uint64_t)(env->CP0_EntryLo0 & ~1ULL) | tlb->G);
#endif
}

void r4k_helper_tlbinv(CPUMIPSState *env)
{
    int idx;
    r4k_tlb_t *tlb;
    uint8_t ASID;
    ASID = env->CP0_EntryHi & 0xFF;

    for (idx = 0; idx < env->tlb->nb_tlb; idx++) {
        tlb = &env->tlb->mmu.r4k.tlb[idx];
        if (!tlb->G && (tlb->ASID == ASID) && !tlb->EHINV) {
            env->tlb->mmu.r4k.tlb[idx].EHINV = 1;
        }
    }
    cpu_mips_tlb_flush(env, 1);
}

void r4k_helper_tlbinvf(CPUMIPSState *env)
{
    int idx;

    for (idx = 0; idx < env->tlb->nb_tlb; idx++) {
        env->tlb->mmu.r4k.tlb[idx].EHINV = 1;
    }
    cpu_mips_tlb_flush(env, 1);
}

void r4k_helper_tlbwi(CPUMIPSState *env)
{
    r4k_tlb_t *tlb;
    int idx;
    target_ulong VPN;
    uint8_t ASID;
    bool G, V0, D0, V1, D1;

#ifdef MIPSSIM_COMPAT
#if defined(TARGET_MIPS64)
    sv_log("Info (MIPS64_TLB) TLBWI ");
#else
    sv_log("Info (MIPS32_TLB) TLBWI ");
#endif
#endif

    idx = (env->CP0_Index & ~0x80000000) % env->tlb->nb_tlb;
    tlb = &env->tlb->mmu.r4k.tlb[idx];
    VPN = env->CP0_EntryHi & (TARGET_PAGE_MASK << 1);
#if defined(TARGET_MIPS64)
    VPN &= env->SEGMask;
#endif
    ASID = env->CP0_EntryHi & 0xff;
    G = env->CP0_EntryLo0 & env->CP0_EntryLo1 & 1;
    V0 = (env->CP0_EntryLo0 & 2) != 0;
    D0 = (env->CP0_EntryLo0 & 4) != 0;
    V1 = (env->CP0_EntryLo1 & 2) != 0;
    D1 = (env->CP0_EntryLo1 & 4) != 0;

    /* Discard cached TLB entries, unless tlbwi is just upgrading access
       permissions on the current entry. */
    if (tlb->VPN != VPN || tlb->ASID != ASID || tlb->G != G ||
        (tlb->V0 && !V0) || (tlb->D0 && !D0) ||
        (tlb->V1 && !V1) || (tlb->D1 && !D1)) {
        r4k_mips_tlb_flush_extra(env, env->tlb->nb_tlb);
    }

    r4k_invalidate_tlb(env, idx, 0);
    r4k_fill_tlb(env, idx);
}

void r4k_helper_tlbwr(CPUMIPSState *env)
{
#ifdef MIPSSIM_COMPAT
#if defined(TARGET_MIPS64)
    sv_log("Info (MIPS64_TLB) TLBWR ");
#else
    sv_log("Info (MIPS32_TLB) TLBWR ");
#endif
#endif
    int idx = (env->CP0_Index & ~0x80000000) % env->tlb->nb_tlb;
    int r = cpu_mips_get_random(env);
    if ((r == idx) && !(env->CP0_Index & 0x80000000)) {
        // Index.P=0 causes TLBWR to use different TLB entry than Index
        r = (r + 1) % env->tlb->nb_tlb;
    }

    r4k_invalidate_tlb(env, r, 1);
    r4k_fill_tlb(env, r);
}

void r4k_helper_tlbp(CPUMIPSState *env)
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
#if defined(TARGET_MIPS64)
        tag &= env->SEGMask;
#endif
        /* Check ASID, virtual page number & size */
        if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag && !tlb->EHINV) {
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
#if defined(TARGET_MIPS64)
            tag &= env->SEGMask;
#endif
            /* Check ASID, virtual page number & size */
            if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag) {
                r4k_mips_tlb_flush_extra (env, i);
                break;
            }
        }

        env->CP0_Index |= 0x80000000;
    }
#ifdef MIPSSIM_COMPAT
#if defined(TARGET_MIPS64)
    sv_log("Info (MIPS64_TLB) TLBP ");
#else
    sv_log("Info (MIPS32_TLB) TLBP ");
#endif
    sv_log("VPN 0x" TARGET_FMT_lx" ", tag);
    sv_log("P %d ", (env->CP0_Index & 0x80000000) >> 31);
    sv_log("Index %d\n", env->CP0_Index & 0x7FFFFFFF);
#endif
}

void r4k_helper_tlbr(CPUMIPSState *env)
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

    if (tlb->EHINV) {
        env->CP0_EntryHi = 1 << CP0EnHi_EHINV;
        env->CP0_PageMask = 0;
        env->CP0_EntryLo0 = 0;
        env->CP0_EntryLo1 = 0;
    } else {
        env->CP0_EntryHi = tlb->VPN | tlb->ASID;
        env->CP0_PageMask = tlb->PageMask;
#if defined(TARGET_MIPS64)
        env->CP0_EntryLo0 = tlb->G | (tlb->V0 << 1) | (tlb->D0 << 2) |
                        ((target_ulong)tlb->RI1 << CP0EnLo_RI) | 
                        ((target_ulong)tlb->XI1 << CP0EnLo_XI) |
                        (tlb->C0 << 3) | (tlb->PFN[0] << 6);
        env->CP0_EntryLo1 = tlb->G | (tlb->V1 << 1) | (tlb->D1 << 2) |
                        ((target_ulong)tlb->RI1 << CP0EnLo_RI) | 
                        ((target_ulong)tlb->XI1 << CP0EnLo_XI) |
                        (tlb->C1 << 3) | (tlb->PFN[1] << 6);
#else
        env->CP0_EntryLo0 = tlb->G | (tlb->V0 << 1) | (tlb->D0 << 2) |
                        ((target_ulong)tlb->RI1 << CP0EnLo_RI) |
                        ((target_ulong)tlb->XI1 << CP0EnLo_XI) |
                        (tlb->C0 << 3) | 
                        ((tlb->PFN[0] & ((1 << 24) - 1)) << 6) | /* PFN */
                        ((tlb->PFN[0] >> 24) << 32); /* PFNX */
        env->CP0_EntryLo1 = tlb->G | (tlb->V1 << 1) | (tlb->D1 << 2) |
                        ((target_ulong)tlb->RI1 << CP0EnLo_RI) |
                        ((target_ulong)tlb->XI1 << CP0EnLo_XI) |
                        (tlb->C1 << 3) | 
                        ((tlb->PFN[1] & ((1 << 24) - 1)) << 6) | /* PFN */
                        ((tlb->PFN[1] >> 24) << 32); /* PFNX */
#endif
    }
#ifdef MIPSSIM_COMPAT
#if defined(TARGET_MIPS64)
    sv_log("Info (MIPS64_TLB) : TLBR ");
#else
    sv_log("Info (MIPS32_TLB) : TLBR ");
#endif
    sv_log("VPN 0x" TARGET_FMT_lx, tlb->VPN >> 11);
    sv_log(" G %x ", tlb->G);
    sv_log("V0 %x ", tlb->V0);
    sv_log("V1 %x ", tlb->V1);
    sv_log("D0 %x ", tlb->D0);
    sv_log("D1 %x ", tlb->D1);
    sv_log("ASID tlb=0x%08x ", tlb->ASID);
    sv_log("EnHi=0x" TARGET_FMT_lx "\n", env->CP0_EntryHi & 0xff);
#endif
}

void helper_tlbwi(CPUMIPSState *env)
{
    env->tlb->helper_tlbwi(env);
}

void helper_tlbwr(CPUMIPSState *env)
{
    env->tlb->helper_tlbwr(env);
}

void helper_tlbp(CPUMIPSState *env)
{
    env->tlb->helper_tlbp(env);
}

void helper_tlbr(CPUMIPSState *env)
{
    env->tlb->helper_tlbr(env);
}

void helper_tlbinv(CPUMIPSState *env)
{
    env->tlb->helper_tlbinv(env);
}

void helper_tlbinvf(CPUMIPSState *env)
{
    env->tlb->helper_tlbinvf(env);
}

/* Specials */
target_ulong helper_di(CPUMIPSState *env)
{
    target_ulong t0 = env->CP0_Status;

    env->CP0_Status = t0 & ~(1 << CP0St_IE);
    return t0;
}

target_ulong helper_ei(CPUMIPSState *env)
{
    target_ulong t0 = env->CP0_Status;

    env->CP0_Status = t0 | (1 << CP0St_IE);
    return t0;
}

static void debug_pre_eret(CPUMIPSState *env)
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

static void debug_post_eret(CPUMIPSState *env)
{
    MIPSCPU *cpu = mips_env_get_cpu(env);

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
        default:
            cpu_abort(CPU(cpu), "Invalid MMU mode!\n");
            break;
        }
    }
}

static void set_pc(CPUMIPSState *env, target_ulong error_pc)
{
    if (env->insn_flags & (ASE_MIPS16 | ASE_MICROMIPS)) {
        env->active_tc.PC = error_pc & ~(target_ulong)1;
        if (error_pc & 1) {
            env->hflags |= MIPS_HFLAG_M16;
        } else {
            env->hflags &= ~(MIPS_HFLAG_M16);
        }
    } else {
        env->active_tc.PC = error_pc;
    }
}

void helper_eret(CPUMIPSState *env)
{
    debug_pre_eret(env);
    if (env->CP0_Status & (1 << CP0St_ERL)) {
        set_pc(env, env->CP0_ErrorEPC);
        env->CP0_Status &= ~(1 << CP0St_ERL);
    } else {
        set_pc(env, env->CP0_EPC);
        env->CP0_Status &= ~(1 << CP0St_EXL);
    }
    compute_hflags(env);
    debug_post_eret(env);
    env->lladdr = 1;
}

void helper_deret(CPUMIPSState *env)
{
    debug_pre_eret(env);
    set_pc(env, env->CP0_DEPC);

    env->hflags &= ~MIPS_HFLAG_DM;
    compute_hflags(env);
    debug_post_eret(env);
    env->lladdr = 1;
}
#endif /* !CONFIG_USER_ONLY */

target_ulong helper_rdhwr_cpunum(CPUMIPSState *env)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 0)))
        return env->CP0_EBase & 0x3ff;
    else
        helper_raise_exception(env, EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_synci_step(CPUMIPSState *env)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 1)))
        return env->SYNCI_Step;
    else
        helper_raise_exception(env, EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_cc(CPUMIPSState *env)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 2)))
        return env->CP0_Count;
    else
        helper_raise_exception(env, EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_ccres(CPUMIPSState *env)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 3)))
        return env->CCRes;
    else
        helper_raise_exception(env, EXCP_RI);

    return 0;
}

target_ulong helper_rdhwr_ulr(CPUMIPSState *env)
{
    if ((env->hflags & MIPS_HFLAG_CP0) ||
        (env->CP0_HWREna & (1 << 29))) {
        return env->CP0_UserLocal;
    } else {
        helper_raise_exception(env, EXCP_RI);
    }

    return 0;
}

void helper_pmon(CPUMIPSState *env, int function)
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
            unsigned char *fmt = (void *)(uintptr_t)env->active_tc.gpr[4];
            printf("%s", fmt);
        }
        break;
    }
}

void helper_wait(CPUMIPSState *env)
{
    CPUState *cs = CPU(mips_env_get_cpu(env));

    cs->halted = 1;
    cpu_reset_interrupt(cs, CPU_INTERRUPT_WAKE);
    helper_raise_exception(env, EXCP_HLT);
}

#if !defined(CONFIG_USER_ONLY)

static void QEMU_NORETURN do_unaligned_access(CPUMIPSState *env,
                                              target_ulong addr, int is_write,
                                              int is_user, uintptr_t retaddr);

#define MMUSUFFIX _mmu
#define ALIGNED_ONLY

#define SHIFT 0
#include "exec/softmmu_template.h"

#define SHIFT 1
#include "exec/softmmu_template.h"

#define SHIFT 2
#include "exec/softmmu_template.h"

#define SHIFT 3
#include "exec/softmmu_template.h"

static void do_unaligned_access(CPUMIPSState *env, target_ulong addr,
                                int is_write, int is_user, uintptr_t retaddr)
{
    env->CP0_BadVAddr = addr;
    do_raise_exception(env, (is_write == 1) ? EXCP_AdES : EXCP_AdEL, retaddr);
}

void tlb_fill(CPUState *cs, target_ulong addr, int is_write, int mmu_idx,
              uintptr_t retaddr)
{
    int ret;

    ret = mips_cpu_handle_mmu_fault(cs, addr, is_write, mmu_idx);
    if (ret) {
        MIPSCPU *cpu = MIPS_CPU(cs);
        CPUMIPSState *env = &cpu->env;

        do_raise_exception_err(env, cs->exception_index,
                               env->error_code, retaddr);
    }
}

void mips_cpu_unassigned_access(CPUState *cs, hwaddr addr,
                                bool is_write, bool is_exec, int unused,
                                unsigned size)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;

    if (is_exec) {
        helper_raise_exception(env, EXCP_IBE);
    } else {
        helper_raise_exception(env, EXCP_DBE);
    }
}
#endif /* !CONFIG_USER_ONLY */

/* Complex FPU operations which may need stack space. */

#define FLOAT_ONE32 make_float32(0x3f8 << 20)
#define FLOAT_ONE64 make_float64(0x3ffULL << 52)
#define FLOAT_TWO32 make_float32(1 << 30)
#define FLOAT_TWO64 make_float64(1ULL << 62)
#define FP_TO_INT32_OVERFLOW 0x7fffffff
#define FP_TO_INT64_OVERFLOW 0x7fffffffffffffffULL

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

static inline void restore_rounding_mode(CPUMIPSState *env)
{
    set_float_rounding_mode(ieee_rm[env->active_fpu.fcr31 & 3],
                            &env->active_fpu.fp_status);
}

static inline void restore_flush_mode(CPUMIPSState *env)
{
    set_flush_to_zero((env->active_fpu.fcr31 & (1 << 24)) != 0,
                      &env->active_fpu.fp_status);

    set_flush_inputs_to_zero((env->active_fpu.fcr31 & (1 << 24)) != 0,
                             &env->active_fpu.fp_status);

    set_float_detect_tininess((env->active_fpu.fcr31 & (1 << 24)) != 0,
                              &env->active_fpu.fp_status);
}

target_ulong helper_cfc1(CPUMIPSState *env, uint32_t reg)
{
    target_ulong arg1 = 0;

    switch (reg) {
    case 0:
        arg1 = (int32_t)env->active_fpu.fcr0;
        break;
    case 1:
        /* UFR Support - Read Status FR */
        if (env->active_fpu.fcr0 & (1 << FCR0_UFRP)) {
            if (env->CP0_Config5 & (1 << CP0C5_UFR)) {
                arg1 = (int32_t)
                       ((env->CP0_Status & (1  << CP0St_FR)) >> CP0St_FR);
            } else {
                helper_raise_exception(env, EXCP_RI);
            }
        }
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
    default:
        arg1 = (int32_t)env->active_fpu.fcr31;
        break;
    }

    return arg1;
}

void helper_ctc1(CPUMIPSState *env, target_ulong arg1, uint32_t fs, uint32_t rt)
{
    switch (fs) {
    case 1:
        /* UFR Alias - Reset Status FR */
        if (!((env->active_fpu.fcr0 & (1 << FCR0_UFRP)) && (rt == 0))) {
            return;
        }
        if (env->CP0_Config5 & (1 << CP0C5_UFR)) {
            env->CP0_Status &= ~(1 << CP0St_FR);
            compute_hflags(env);
        } else {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case 4:
        /* UNFR Alias - Set Status FR */
        if (!((env->active_fpu.fcr0 & (1 << FCR0_UFRP)) && (rt == 0))) {
            return;
        }
        if (env->CP0_Config5 & (1 << CP0C5_UFR)) {
            env->CP0_Status |= (1 << CP0St_FR);
            compute_hflags(env);
        } else {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case 25:
        if ((env->insn_flags & ISA_MIPS32R6) || (arg1 & 0xffffff00)) {
            return;
        }
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
    case 31:
        {
            uint32_t ronly_mask = 0x007c0000;
            if (env->insn_flags & ISA_MIPS32R6) {
                ronly_mask |= 0xfe800000;
            }
            env->active_fpu.fcr31 = (arg1 & ~ronly_mask) | 
                     (env->active_fpu.fcr31 & ronly_mask);
        }
        break;
    default:
        return;
    }
    /* set rounding mode */
    restore_rounding_mode(env);
    /* set flush-to-zero mode */
    restore_flush_mode(env);
    set_float_exception_flags(0, &env->active_fpu.fp_status);
    if ((GET_FP_ENABLE(env->active_fpu.fcr31) | 0x20) & GET_FP_CAUSE(env->active_fpu.fcr31))
        do_raise_exception(env, EXCP_FPE, GETPC());
}

static inline int ieee_ex_to_mips(CPUMIPSState *env, int xcpt)
{
    int ret = 0;
    int flushToZero = env->active_fpu.fcr31 & (1 << 24);
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

        if (flushToZero) {
            // Alternate Flush to Zero Underflow Handling
            if (xcpt & float_flag_output_denormal) {
               /* Flushing of tiny non-zero results causes Inexact and Underflow
                  Exceptions to be signaled. */
                ret |= FP_INEXACT;
                ret |= FP_UNDERFLOW;
            } else if (xcpt & float_flag_input_denormal) {
                /* Flushing of subnormal input operands in all instructions
                   except comparisons causes Inexact Exception to be signaled. */
                ret |= FP_INEXACT;
            }
        } else {
            // Underflow handling
            if (xcpt & float_flag_output_denormal &&
                (GET_FP_ENABLE(env->active_fpu.fcr31) & FP_UNDERFLOW)) {
                /* When an underflow trap is enabled (through the FCSR Enable
                   field bit), underflow is signaled when tininess is
                   detected regardless of loss of accuracy */
                ret |= FP_UNDERFLOW;
            }
        }
    }
    return ret;
}

static inline void update_fcr31(CPUMIPSState *env, uintptr_t pc)
{
    int tmp = ieee_ex_to_mips(env, get_float_exception_flags(&env->active_fpu.fp_status));

    SET_FP_CAUSE(env->active_fpu.fcr31, tmp);

    if (tmp) {
        set_float_exception_flags(0, &env->active_fpu.fp_status);

        if (GET_FP_ENABLE(env->active_fpu.fcr31) & tmp) {
            do_raise_exception(env, EXCP_FPE, pc);
        } else {
            UPDATE_FP_FLAGS(env->active_fpu.fcr31, tmp);
        }
    }
}

#define HANDLE_INVALID_FLOAT_INT_CONVERSION(fwidth, iwidth)             \
static void                                                             \
handle_invalid_f ## fwidth ## _i ## iwidth ## _conversion(CPUMIPSState *env, \
                                                          uint ## fwidth ## _t fst0,  \
                                                          uint ## iwidth ## _t * ret) \
{                                                                       \
    if (env->active_fpu.fcr0 & (1 << FCR0_Has2008)) {                   \
        if (float ## fwidth ## _is_quiet_nan(fst0) ||                   \
            float ## fwidth ## _is_signaling_nan(fst0)) {               \
            *ret = 0;                                                   \
        } else if (float ## fwidth ## _is_infinity(fst0) &&             \
                   float ## fwidth ## _is_neg(fst0)) {                  \
            *ret = 1ULL << (iwidth - 1);                                \
        }                                                               \
    } else {                                                            \
        *ret = FP_TO_INT ## iwidth ## _OVERFLOW;                        \
    }                                                                   \
}

HANDLE_INVALID_FLOAT_INT_CONVERSION(32,32)
HANDLE_INVALID_FLOAT_INT_CONVERSION(64,32)
HANDLE_INVALID_FLOAT_INT_CONVERSION(32,64)
HANDLE_INVALID_FLOAT_INT_CONVERSION(64,64)

/* Float support.
   Single precition routines have a "s" suffix, double precision a
   "d" suffix, 32bit integer "w", 64bit integer "l", paired single "ps",
   paired single lower "pl", paired single upper "pu".  */

/* unary operations, modifying fp status  */
uint64_t helper_float_sqrt_d(CPUMIPSState *env, uint64_t fdt0)
{
    fdt0 = float64_sqrt(fdt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt0;
}

uint32_t helper_float_sqrt_s(CPUMIPSState *env, uint32_t fst0)
{
    fst0 = float32_sqrt(fst0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst0;
}

uint64_t helper_float_cvtd_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t fdt2;

    fdt2 = float32_to_float64(fst0, &env->active_fpu.fp_status);
    if (env->active_fpu.fcr0 & (1 << FCR0_Has2008)) {
        if (get_float_exception_flags(&env->active_fpu.fp_status)
            & float_flag_invalid) {
            if (float32_is_quiet_nan(fst0) || float32_is_signaling_nan(fst0)) {
                // Preserve sign and return always QNaN
                fdt2 |= 1ULL << 51;
                fdt2 |= ((uint64_t)fst0 >> 31) << 63;
            }
        }
    }
    update_fcr31(env, GETPC());
    return fdt2;
}

uint64_t helper_float_cvtd_w(CPUMIPSState *env, uint32_t wt0)
{
    uint64_t fdt2;

    fdt2 = int32_to_float64(wt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint64_t helper_float_cvtd_l(CPUMIPSState *env, uint64_t dt0)
{
    uint64_t fdt2;

    fdt2 = int64_to_float64(dt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint64_t helper_float_cvtl_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t dt2;

    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i64_conversion(env, fdt0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_cvtl_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t dt2;

    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i64_conversion(env, fst0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_cvtps_pw(CPUMIPSState *env, uint64_t dt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    fst2 = int32_to_float32(dt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = int32_to_float32(dt0 >> 32, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_cvtpw_ps(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;
    uint32_t wth2;
    int excp, excph;

    wt2 = float32_to_int32(fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    excp = get_float_exception_flags(&env->active_fpu.fp_status);
    if (excp & (float_flag_overflow | float_flag_invalid)) {
        wt2 = FP_TO_INT32_OVERFLOW;
    }

    set_float_exception_flags(0, &env->active_fpu.fp_status);
    wth2 = float32_to_int32(fdt0 >> 32, &env->active_fpu.fp_status);
    excph = get_float_exception_flags(&env->active_fpu.fp_status);
    if (excph & (float_flag_overflow | float_flag_invalid)) {
        wth2 = FP_TO_INT32_OVERFLOW;
    }

    set_float_exception_flags(excp | excph, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());

    return ((uint64_t)wth2 << 32) | wt2;
}

uint32_t helper_float_cvts_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t fst2;

    fst2 = float64_to_float32(fdt0, &env->active_fpu.fp_status);
    if (env->active_fpu.fcr0 & (1 << FCR0_Has2008)) {
        if (get_float_exception_flags(&env->active_fpu.fp_status)
            & float_flag_invalid) {
            if (float64_is_quiet_nan(fdt0) || float64_is_signaling_nan(fdt0)) {
                // Preserve sign and return always QNaN
                fst2 |= 1 << 22;
                fst2 |= (fdt0 >> 63) << 31;
            }
        }
    }
    update_fcr31(env, GETPC());
    return fst2;
}

uint32_t helper_float_cvts_w(CPUMIPSState *env, uint32_t wt0)
{
    uint32_t fst2;

    fst2 = int32_to_float32(wt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint32_t helper_float_cvts_l(CPUMIPSState *env, uint64_t dt0)
{
    uint32_t fst2;

    fst2 = int64_to_float32(dt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint32_t helper_float_cvts_pl(CPUMIPSState *env, uint32_t wt0)
{
    uint32_t wt2;

    wt2 = wt0;
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_cvts_pu(CPUMIPSState *env, uint32_t wth0)
{
    uint32_t wt2;

    wt2 = wth0;
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_cvtw_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t wt2;

    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i32_conversion(env, fst0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_cvtw_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;

    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i32_conversion(env, fdt0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint64_t helper_float_roundl_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i64_conversion(env, fdt0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_roundl_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i64_conversion(env, fst0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint32_t helper_float_roundw_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i32_conversion(env, fdt0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_roundw_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_nearest_even, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i32_conversion(env, fst0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint64_t helper_float_truncl_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t dt2;

    dt2 = float64_to_int64_round_to_zero(fdt0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i64_conversion(env, fdt0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_truncl_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t dt2;

    dt2 = float32_to_int64_round_to_zero(fst0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i64_conversion(env, fst0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint32_t helper_float_truncw_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;

    wt2 = float64_to_int32_round_to_zero(fdt0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i32_conversion(env, fdt0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_truncw_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t wt2;

    wt2 = float32_to_int32_round_to_zero(fst0, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i32_conversion(env, fst0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint64_t helper_float_ceill_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i64_conversion(env, fdt0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_ceill_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i64_conversion(env, fst0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint32_t helper_float_ceilw_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i32_conversion(env, fdt0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_ceilw_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i32_conversion(env, fst0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint64_t helper_float_floorl_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dt2 = float64_to_int64(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i64_conversion(env, fdt0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint64_t helper_float_floorl_s(CPUMIPSState *env, uint32_t fst0)
{
    uint64_t dt2;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dt2 = float32_to_int64(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i64_conversion(env, fst0, &dt2);
    }
    update_fcr31(env, GETPC());
    return dt2;
}

uint32_t helper_float_floorw_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    wt2 = float64_to_int32(fdt0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f64_i32_conversion(env, fdt0, &wt2);
    }
    update_fcr31(env, GETPC());
    return wt2;
}

uint32_t helper_float_floorw_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t wt2;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    wt2 = float32_to_int32(fst0, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        handle_invalid_f32_i32_conversion(env, fst0, &wt2);
    }
    update_fcr31(env, GETPC());
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

uint32_t helper_float_maddf_s(CPUMIPSState *env, uint32_t fs, uint32_t ft, uint32_t fd)
{
    uint32_t fdret;

    fdret = float32_muladd(fs, ft, fd, 0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint64_t helper_float_maddf_d(CPUMIPSState *env, uint64_t fs, uint64_t ft, uint64_t fd)
{
    uint64_t fdret;

    fdret = float64_muladd(fs, ft, fd, 0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint32_t helper_float_msubf_s(CPUMIPSState *env, uint32_t fs, uint32_t ft, uint32_t fd)
{
    uint32_t fdret;

    fdret = float32_muladd(fs, ft, fd, float_muladd_negate_product, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint64_t helper_float_msubf_d(CPUMIPSState *env, uint64_t fs, uint64_t ft, uint64_t fd)
{
    uint64_t fdret;

    fdret = float64_muladd(fs, ft, fd, float_muladd_negate_product, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint32_t helper_float_max_s(CPUMIPSState *env, uint32_t fs, uint32_t ft)
{
    uint32_t fdret;

    fdret = float32_maxnum(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint32_t helper_float_maxa_s(CPUMIPSState *env, uint32_t fs, uint32_t ft)
{
    uint32_t fdret;
    
    fdret = float32_maxnummag(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    
    return fdret;
}

uint64_t helper_float_max_d(CPUMIPSState *env, uint64_t fs, uint64_t ft)
{
    uint64_t fdret;

    fdret = float64_maxnum(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint64_t helper_float_maxa_d(CPUMIPSState *env, uint64_t fs, uint64_t ft)
{
    uint64_t fdret;

    fdret = float64_maxnummag(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint32_t helper_float_min_s(CPUMIPSState *env, uint32_t fs, uint32_t ft)
{
    uint32_t fdret;

    fdret = float32_minnum(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint32_t helper_float_mina_s(CPUMIPSState *env, uint32_t fs, uint32_t ft)
{
    uint32_t fdret;

    fdret = float32_minnummag(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());

    return fdret;
}

uint64_t helper_float_min_d(CPUMIPSState *env, uint64_t fs, uint64_t ft)
{
    uint64_t fdret;

    fdret = float64_minnum(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

uint64_t helper_float_mina_d(CPUMIPSState *env, uint64_t fs, uint64_t ft)
{
    uint64_t fdret;

    fdret = float64_minnummag(fs, ft, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdret;
}

#define FLOAT_CLASS_SIGNALING_NAN      0x001
#define FLOAT_CLASS_QUIET_NAN          0x002

#define FLOAT_CLASS_NEGATIVE_INFINITY  0x004
#define FLOAT_CLASS_NEGATIVE_NORMAL    0x008
#define FLOAT_CLASS_NEGATIVE_SUBNORMAL 0x010
#define FLOAT_CLASS_NEGATIVE_ZERO      0x020

#define FLOAT_CLASS_POSITIVE_INFINITY  0x040
#define FLOAT_CLASS_POSITIVE_NORMAL    0x080
#define FLOAT_CLASS_POSITIVE_SUBNORMAL 0x100
#define FLOAT_CLASS_POSITIVE_ZERO      0x200

#define FLOAT_CLASS(ARG, BITS)                              \
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
            mask |= FLOAT_CLASS_SIGNALING_NAN;}             \
        else if (qnan) {                                        \
            mask |= FLOAT_CLASS_QUIET_NAN;                  \
        } else if (neg) {                                       \
            if (inf) {                                          \
                mask |= FLOAT_CLASS_NEGATIVE_INFINITY;      \
            } else if (zero) {                                  \
                mask |= FLOAT_CLASS_NEGATIVE_ZERO;          \
            } else if (dnmz) {                                  \
                mask |= FLOAT_CLASS_NEGATIVE_SUBNORMAL;     \
            }                                                   \
            else {                                              \
                mask |= FLOAT_CLASS_NEGATIVE_NORMAL;        \
            }                                                   \
        } else {                                                \
            if (inf) {                                          \
                mask |= FLOAT_CLASS_POSITIVE_INFINITY;      \
            } else if (zero) {                                  \
                mask |= FLOAT_CLASS_POSITIVE_ZERO;          \
            } else if (dnmz) {                                  \
                mask |= FLOAT_CLASS_POSITIVE_SUBNORMAL;     \
            } else {                                            \
                mask |= FLOAT_CLASS_POSITIVE_NORMAL;        \
            }                                                   \
        }                                                       \
                                                                \
        return mask;                                            \
    } while (0)

uint32_t helper_float_class_s(uint32_t arg)
{
    FLOAT_CLASS(arg, 32);
}

uint64_t helper_float_class_d(uint64_t arg)
{
    FLOAT_CLASS(arg, 64);
}

uint32_t helper_float_rint_s(CPUMIPSState *env, uint32_t fs)
{
    uint32_t fd;

    fd = float32_round_to_int(fs, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fd;
}

uint64_t helper_float_rint_d(CPUMIPSState *env, uint64_t fs)
{
    uint64_t fd; 

    fd = float64_round_to_int(fs, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fd;
}


/* MIPS specific unary operations */
uint64_t helper_float_recip_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t fdt2;

    fdt2 = float64_div(float64_one, fdt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_recip_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t fst2;

    fst2 = float32_div(float32_one, fst0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_rsqrt_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t fdt2;

    fdt2 = float64_sqrt(fdt0, &env->active_fpu.fp_status);
    fdt2 = float64_div(float64_one, fdt2, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_rsqrt_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t fst2;

    fst2 = float32_sqrt(fst0, &env->active_fpu.fp_status);
    fst2 = float32_div(float32_one, fst2, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_recip1_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t fdt2;

    fdt2 = float64_div(float64_one, fdt0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_recip1_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t fst2;

    fst2 = float32_div(float32_one, fst0, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_recip1_ps(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    fst2 = float32_div(float32_one, fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = float32_div(float32_one, fdt0 >> 32, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_rsqrt1_d(CPUMIPSState *env, uint64_t fdt0)
{
    uint64_t fdt2;

    fdt2 = float64_sqrt(fdt0, &env->active_fpu.fp_status);
    fdt2 = float64_div(float64_one, fdt2, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_rsqrt1_s(CPUMIPSState *env, uint32_t fst0)
{
    uint32_t fst2;

    fst2 = float32_sqrt(fst0, &env->active_fpu.fp_status);
    fst2 = float32_div(float32_one, fst2, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_rsqrt1_ps(CPUMIPSState *env, uint64_t fdt0)
{
    uint32_t fst2;
    uint32_t fsth2;

    fst2 = float32_sqrt(fdt0 & 0XFFFFFFFF, &env->active_fpu.fp_status);
    fsth2 = float32_sqrt(fdt0 >> 32, &env->active_fpu.fp_status);
    fst2 = float32_div(float32_one, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_div(float32_one, fsth2, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

#define FLOAT_OP(name, p) void helper_float_##name##_##p(CPUMIPSState *env)

/* binary operations */
#define FLOAT_BINOP(name)                                          \
uint64_t helper_float_ ## name ## _d(CPUMIPSState *env,            \
                                     uint64_t fdt0, uint64_t fdt1) \
{                                                                  \
    uint64_t dt2;                                                  \
                                                                   \
    dt2 = float64_ ## name (fdt0, fdt1, &env->active_fpu.fp_status);     \
    update_fcr31(env, GETPC());                                    \
    return dt2;                                                    \
}                                                                  \
                                                                   \
uint32_t helper_float_ ## name ## _s(CPUMIPSState *env,            \
                                     uint32_t fst0, uint32_t fst1) \
{                                                                  \
    uint32_t wt2;                                                  \
                                                                   \
    wt2 = float32_ ## name (fst0, fst1, &env->active_fpu.fp_status);     \
    update_fcr31(env, GETPC());                                    \
    return wt2;                                                    \
}                                                                  \
                                                                   \
uint64_t helper_float_ ## name ## _ps(CPUMIPSState *env,           \
                                      uint64_t fdt0,               \
                                      uint64_t fdt1)               \
{                                                                  \
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;                             \
    uint32_t fsth0 = fdt0 >> 32;                                   \
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;                             \
    uint32_t fsth1 = fdt1 >> 32;                                   \
    uint32_t wt2;                                                  \
    uint32_t wth2;                                                 \
                                                                   \
    wt2 = float32_ ## name (fst0, fst1, &env->active_fpu.fp_status);     \
    wth2 = float32_ ## name (fsth0, fsth1, &env->active_fpu.fp_status);  \
    update_fcr31(env, GETPC());                                    \
    return ((uint64_t)wth2 << 32) | wt2;                           \
}

FLOAT_BINOP(add)
FLOAT_BINOP(sub)
FLOAT_BINOP(mul)
FLOAT_BINOP(div)
#undef FLOAT_BINOP

#define UNFUSED_FMA(prefix, a, b, c, flags)                          \
{                                                                    \
    a = prefix##_mul(a, b, &env->active_fpu.fp_status);              \
    if ((flags) & float_muladd_negate_c) {                           \
        a = prefix##_sub(a, c, &env->active_fpu.fp_status);          \
    } else {                                                         \
        a = prefix##_add(a, c, &env->active_fpu.fp_status);          \
    }                                                                \
    if ((flags) & float_muladd_negate_result) {                      \
        a = prefix##_chs(a);                                         \
    }                                                                \
}

/* FMA based operations */
#define FLOAT_FMA(name, type)                                        \
uint64_t helper_float_ ## name ## _d(CPUMIPSState *env,              \
                                     uint64_t fdt0, uint64_t fdt1,   \
                                     uint64_t fdt2)                  \
{                                                                    \
    UNFUSED_FMA(float64, fdt0, fdt1, fdt2, type);                    \
    update_fcr31(env, GETPC());                                      \
    return fdt0;                                                     \
}                                                                    \
                                                                     \
uint32_t helper_float_ ## name ## _s(CPUMIPSState *env,              \
                                     uint32_t fst0, uint32_t fst1,   \
                                     uint32_t fst2)                  \
{                                                                    \
    UNFUSED_FMA(float32, fst0, fst1, fst2, type);                    \
    update_fcr31(env, GETPC());                                      \
    return fst0;                                                     \
}                                                                    \
                                                                     \
uint64_t helper_float_ ## name ## _ps(CPUMIPSState *env,             \
                                      uint64_t fdt0, uint64_t fdt1,  \
                                      uint64_t fdt2)                 \
{                                                                    \
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;                               \
    uint32_t fsth0 = fdt0 >> 32;                                     \
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;                               \
    uint32_t fsth1 = fdt1 >> 32;                                     \
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;                               \
    uint32_t fsth2 = fdt2 >> 32;                                     \
                                                                     \
    UNFUSED_FMA(float32, fst0, fst1, fst2, type);                    \
    UNFUSED_FMA(float32, fsth0, fsth1, fsth2, type);                 \
    update_fcr31(env, GETPC());                                      \
    return ((uint64_t)fsth0 << 32) | fst0;                           \
}
FLOAT_FMA(madd, 0)
FLOAT_FMA(msub, float_muladd_negate_c)
FLOAT_FMA(nmadd, float_muladd_negate_result)
FLOAT_FMA(nmsub, float_muladd_negate_result | float_muladd_negate_c)
#undef FLOAT_FMA

/* MIPS specific binary operations */
uint64_t helper_float_recip2_d(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt2)
{
    fdt2 = float64_mul(fdt0, fdt2, &env->active_fpu.fp_status);
    fdt2 = float64_chs(float64_sub(fdt2, float64_one, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_recip2_s(CPUMIPSState *env, uint32_t fst0, uint32_t fst2)
{
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_sub(fst2, float32_one, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_recip2_ps(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt2)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;
    uint32_t fsth2 = fdt2 >> 32;

    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_mul(fsth0, fsth2, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_sub(fst2, float32_one, &env->active_fpu.fp_status));
    fsth2 = float32_chs(float32_sub(fsth2, float32_one, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_rsqrt2_d(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt2)
{
    fdt2 = float64_mul(fdt0, fdt2, &env->active_fpu.fp_status);
    fdt2 = float64_sub(fdt2, float64_one, &env->active_fpu.fp_status);
    fdt2 = float64_chs(float64_div(fdt2, FLOAT_TWO64, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return fdt2;
}

uint32_t helper_float_rsqrt2_s(CPUMIPSState *env, uint32_t fst0, uint32_t fst2)
{
    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fst2 = float32_sub(fst2, float32_one, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_div(fst2, FLOAT_TWO32, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return fst2;
}

uint64_t helper_float_rsqrt2_ps(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt2)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst2 = fdt2 & 0XFFFFFFFF;
    uint32_t fsth2 = fdt2 >> 32;

    fst2 = float32_mul(fst0, fst2, &env->active_fpu.fp_status);
    fsth2 = float32_mul(fsth0, fsth2, &env->active_fpu.fp_status);
    fst2 = float32_sub(fst2, float32_one, &env->active_fpu.fp_status);
    fsth2 = float32_sub(fsth2, float32_one, &env->active_fpu.fp_status);
    fst2 = float32_chs(float32_div(fst2, FLOAT_TWO32, &env->active_fpu.fp_status));
    fsth2 = float32_chs(float32_div(fsth2, FLOAT_TWO32, &env->active_fpu.fp_status));
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_addr_ps(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt1)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;
    uint32_t fsth1 = fdt1 >> 32;
    uint32_t fst2;
    uint32_t fsth2;

    fst2 = float32_add (fst0, fsth0, &env->active_fpu.fp_status);
    fsth2 = float32_add (fst1, fsth1, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

uint64_t helper_float_mulr_ps(CPUMIPSState *env, uint64_t fdt0, uint64_t fdt1)
{
    uint32_t fst0 = fdt0 & 0XFFFFFFFF;
    uint32_t fsth0 = fdt0 >> 32;
    uint32_t fst1 = fdt1 & 0XFFFFFFFF;
    uint32_t fsth1 = fdt1 >> 32;
    uint32_t fst2;
    uint32_t fsth2;

    fst2 = float32_mul (fst0, fsth0, &env->active_fpu.fp_status);
    fsth2 = float32_mul (fst1, fsth1, &env->active_fpu.fp_status);
    update_fcr31(env, GETPC());
    return ((uint64_t)fsth2 << 32) | fst2;
}

/* compare operations */
#define FOP_COND_D(op, cond)                                   \
void helper_cmp_d_ ## op(CPUMIPSState *env, uint64_t fdt0,     \
                         uint64_t fdt1, int cc)                \
{                                                              \
    int c;                                                     \
    c = cond;                                                  \
    update_fcr31(env, GETPC());                                \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}                                                              \
void helper_cmpabs_d_ ## op(CPUMIPSState *env, uint64_t fdt0,  \
                            uint64_t fdt1, int cc)             \
{                                                              \
    int c;                                                     \
    fdt0 = float64_abs(fdt0);                                  \
    fdt1 = float64_abs(fdt1);                                  \
    c = cond;                                                  \
    update_fcr31(env, GETPC());                                \
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
FOP_COND_D(ngle,float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status))
FOP_COND_D(seq, float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ngl, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(lt,  float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(nge, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(le,  float64_le(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_COND_D(ngt, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status)  || float64_le(fdt0, fdt1, &env->active_fpu.fp_status))

#define FOP_COND_S(op, cond)                                   \
void helper_cmp_s_ ## op(CPUMIPSState *env, uint32_t fst0,     \
                         uint32_t fst1, int cc)                \
{                                                              \
    int c;                                                     \
    c = cond;                                                  \
    update_fcr31(env, GETPC());                                \
    if (c)                                                     \
        SET_FP_COND(cc, env->active_fpu);                      \
    else                                                       \
        CLEAR_FP_COND(cc, env->active_fpu);                    \
}                                                              \
void helper_cmpabs_s_ ## op(CPUMIPSState *env, uint32_t fst0,  \
                            uint32_t fst1, int cc)             \
{                                                              \
    int c;                                                     \
    fst0 = float32_abs(fst0);                                  \
    fst1 = float32_abs(fst1);                                  \
    c = cond;                                                  \
    update_fcr31(env, GETPC());                                \
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
FOP_COND_S(ngle,float32_unordered(fst1, fst0, &env->active_fpu.fp_status))
FOP_COND_S(seq, float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ngl, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(lt,  float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(nge, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(le,  float32_le(fst0, fst1, &env->active_fpu.fp_status))
FOP_COND_S(ngt, float32_unordered(fst1, fst0, &env->active_fpu.fp_status)  || float32_le(fst0, fst1, &env->active_fpu.fp_status))

#define FOP_COND_PS(op, condl, condh)                           \
void helper_cmp_ps_ ## op(CPUMIPSState *env, uint64_t fdt0,     \
                          uint64_t fdt1, int cc)                \
{                                                               \
    uint32_t fst0, fsth0, fst1, fsth1;                          \
    int ch, cl;                                                 \
    fst0 = fdt0 & 0XFFFFFFFF;                                   \
    fsth0 = fdt0 >> 32;                                         \
    fst1 = fdt1 & 0XFFFFFFFF;                                   \
    fsth1 = fdt1 >> 32;                                         \
    cl = condl;                                                 \
    ch = condh;                                                 \
    update_fcr31(env, GETPC());                                 \
    if (cl)                                                     \
        SET_FP_COND(cc, env->active_fpu);                       \
    else                                                        \
        CLEAR_FP_COND(cc, env->active_fpu);                     \
    if (ch)                                                     \
        SET_FP_COND(cc + 1, env->active_fpu);                   \
    else                                                        \
        CLEAR_FP_COND(cc + 1, env->active_fpu);                 \
}                                                               \
void helper_cmpabs_ps_ ## op(CPUMIPSState *env, uint64_t fdt0,  \
                             uint64_t fdt1, int cc)             \
{                                                               \
    uint32_t fst0, fsth0, fst1, fsth1;                          \
    int ch, cl;                                                 \
    fst0 = float32_abs(fdt0 & 0XFFFFFFFF);                      \
    fsth0 = float32_abs(fdt0 >> 32);                            \
    fst1 = float32_abs(fdt1 & 0XFFFFFFFF);                      \
    fsth1 = float32_abs(fdt1 >> 32);                            \
    cl = condl;                                                 \
    ch = condh;                                                 \
    update_fcr31(env, GETPC());                                 \
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
FOP_COND_PS(ngle,float32_unordered(fst1, fst0, &env->active_fpu.fp_status),
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

/* R6 compare operations */
#define FOP_CONDN_D(op, cond)                                                 \
uint64_t helper_r6_cmp_d_ ## op(CPUMIPSState *env, uint64_t fdt0,             \
                         uint64_t fdt1)                                       \
{                                                                             \
    uint64_t c;                                                               \
    uint32_t ignore_flush = ~(float_flag_input_denormal |                     \
                              float_flag_output_denormal);                    \
    c = cond;                                                                 \
    ignore_flush &= get_float_exception_flags(&env->active_fpu.fp_status);    \
    set_float_exception_flags(ignore_flush, &env->active_fpu.fp_status);      \
    update_fcr31(env, GETPC());                                               \
    if (c) {                                                                  \
        return -1;                                                            \
    }                                                                         \
    else {                                                                    \
        return 0;                                                             \
    }                                                                         \
}

/* NOTE: the comma operator will make "cond" to eval to false,
 * but float64_unordered_quiet() is still called. */
FOP_CONDN_D(af,  (float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status), 0))
FOP_CONDN_D(un,  float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status))
FOP_CONDN_D(eq,  float64_eq_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(ueq, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_eq_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(lt,  float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(ult, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(le,  float64_le_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(ule, float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_le_quiet(fdt0, fdt1, &env->active_fpu.fp_status))
/* NOTE: the comma operator will make "cond" to eval to false,
 * but float64_unordered() is still called. */
FOP_CONDN_D(saf,  (float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status), 0))
FOP_CONDN_D(sun,  float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status))
FOP_CONDN_D(seq,  float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(sueq, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status) || float64_eq(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(slt,  float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(sult, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(sle,  float64_le(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(sule, float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status) || float64_le(fdt0, fdt1, &env->active_fpu.fp_status))
FOP_CONDN_D(or,   (float64_le_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_le_quiet(fdt0, fdt1, &env->active_fpu.fp_status)))
FOP_CONDN_D(une,  (float64_unordered_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status)))
FOP_CONDN_D(ne,   (float64_lt_quiet(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt_quiet(fdt0, fdt1, &env->active_fpu.fp_status)))
FOP_CONDN_D(sor,  (float64_le(fdt1, fdt0, &env->active_fpu.fp_status) || float64_le(fdt0, fdt1, &env->active_fpu.fp_status)))
FOP_CONDN_D(sune, (float64_unordered(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt(fdt0, fdt1, &env->active_fpu.fp_status)))
FOP_CONDN_D(sne,  (float64_lt(fdt1, fdt0, &env->active_fpu.fp_status) || float64_lt(fdt0, fdt1, &env->active_fpu.fp_status)))

#define FOP_CONDN_S(op, cond)                                                 \
uint32_t helper_r6_cmp_s_ ## op(CPUMIPSState *env, uint32_t fst0,             \
                         uint32_t fst1)                                       \
{                                                                             \
    uint64_t c;                                                               \
    uint32_t ignore_flush = ~(float_flag_input_denormal |                     \
                              float_flag_output_denormal);                    \
    c = cond;                                                                 \
    ignore_flush &= get_float_exception_flags(&env->active_fpu.fp_status);    \
    set_float_exception_flags(ignore_flush, &env->active_fpu.fp_status);      \
    update_fcr31(env, GETPC());                                               \
    if (c) {                                                                  \
        return -1;                                                            \
    }                                                                         \
    else {                                                                    \
        return 0;                                                             \
    }                                                                         \
}

/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered_quiet() is still called. */
FOP_CONDN_S(af,   (float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status), 0))
FOP_CONDN_S(un,   float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status))
FOP_CONDN_S(eq,   float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(ueq,  float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_eq_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(lt,   float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(ult,  float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(le,   float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(ule,  float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status))
/* NOTE: the comma operator will make "cond" to eval to false,
 * but float32_unordered() is still called. */
FOP_CONDN_S(saf,  (float32_unordered(fst1, fst0, &env->active_fpu.fp_status), 0))
FOP_CONDN_S(sun,  float32_unordered(fst1, fst0, &env->active_fpu.fp_status))
FOP_CONDN_S(seq,  float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(sueq, float32_unordered(fst1, fst0, &env->active_fpu.fp_status) || float32_eq(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(slt,  float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(sult, float32_unordered(fst1, fst0, &env->active_fpu.fp_status) || float32_lt(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(sle,  float32_le(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(sule, float32_unordered(fst1, fst0, &env->active_fpu.fp_status) || float32_le(fst0, fst1, &env->active_fpu.fp_status))
FOP_CONDN_S(or,   (float32_le_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_le_quiet(fst0, fst1, &env->active_fpu.fp_status)))
FOP_CONDN_S(une,  (float32_unordered_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_lt_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status)))
FOP_CONDN_S(ne,   (float32_lt_quiet(fst1, fst0, &env->active_fpu.fp_status) || float32_lt_quiet(fst0, fst1, &env->active_fpu.fp_status)))
FOP_CONDN_S(sor,  (float32_le(fst1, fst0, &env->active_fpu.fp_status) || float32_le(fst0, fst1, &env->active_fpu.fp_status)))
FOP_CONDN_S(sune, (float32_unordered(fst1, fst0, &env->active_fpu.fp_status) || float32_lt(fst1, fst0, &env->active_fpu.fp_status) || float32_lt(fst0, fst1, &env->active_fpu.fp_status)))
FOP_CONDN_S(sne,  (float32_lt(fst1, fst0, &env->active_fpu.fp_status) || float32_lt(fst0, fst1, &env->active_fpu.fp_status)))

/*
 *  MSA
 */

#define DEBUG_MSACSR 0

/* Data format and vector length unpacking */
#define WRLEN(wrlen_df) (wrlen_df >> 2)
#define DF(wrlen_df) (wrlen_df & 0x03)

#define DF_BYTE   0
#define DF_HALF   1
#define DF_WORD   2
#define DF_DOUBLE 3

#define DF_FLOAT_WORD   0
#define DF_FLOAT_DOUBLE 1

static void msa_check_index(CPUMIPSState *env,
        uint32_t df, uint32_t n, uint32_t wrlen) {
    switch (df) {
    case DF_BYTE: /* b */
        if (n > wrlen / 8 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_HALF: /* h */
        if (n > wrlen / 16 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_WORD: /* w */
        if (n > wrlen / 32 - 1) {
            helper_raise_exception(env, EXCP_RI);
        }
        break;
    case DF_DOUBLE: /* d */
        if (n > wrlen / 64 - 1) {
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

int64_t helper_add_a_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 + abs_arg2;
}

int64_t helper_addv_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 + arg2;
}

int64_t helper_subv_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 - arg2;
}


/*
 *  ADDS_A, ADDS_S, ADDS_U, SUBS_S, SUBS_U, SUBSUU_S, SUBSUS_U
 */

int64_t helper_adds_a_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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

int64_t helper_adds_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);

    if (arg1 < 0) {
        return (min_int - arg1 < arg2) ? arg1 + arg2 : min_int;
    } else {
        return (arg2 < max_int - arg1) ? arg1 + arg2 : max_int;
    }
}

uint64_t helper_adds_u_df(CPUMIPSState *env, uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t max_uint = DF_MAX_UINT(df);

    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return (u_arg1 < max_uint - u_arg2) ? u_arg1 + u_arg2 : max_uint;
}

int64_t helper_subs_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t max_int = DF_MAX_INT(df);
    int64_t min_int = DF_MIN_INT(df);

    if (arg2 > 0) {
        return (min_int + arg2 < arg1) ? arg1 - arg2 : min_int;
    } else {
        return (arg1 < max_int + arg2) ? arg1 - arg2 : max_int;
    }
}


int64_t helper_subs_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return (u_arg1 > u_arg2) ? u_arg1 - u_arg2 : 0;
}


int64_t helper_subsuu_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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

int64_t helper_subsus_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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

void helper_and_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) & D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_andi_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) & arg2;
    } DONE_ALL_ELEMENTS;
}

void helper_or_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) | D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_ori_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) | arg2;
    } DONE_ALL_ELEMENTS;
}

void helper_nor_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = ~(D(pws, i) | D(pwt, i));
    } DONE_ALL_ELEMENTS;
}

void helper_nori_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = ~(B(pws, i) | arg2);
    } DONE_ALL_ELEMENTS;
}

void helper_xor_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i) ^ D(pwt, i);
    } DONE_ALL_ELEMENTS;
}

void helper_xori_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        B(pwd, i) = B(pws, i) ^ arg2;
    } DONE_ALL_ELEMENTS;
}



/*
 *  ASUB_S, ASUB_U
 */

int64_t helper_asub_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed compare */
    return (arg1 < arg2) ?
        (uint64_t)(arg2 - arg1) : (uint64_t)(arg1 - arg2);
}

uint64_t helper_asub_u_df(CPUMIPSState *env, uint64_t arg1, uint64_t arg2, uint32_t df)
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

int64_t helper_ave_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + (arg1 & arg2 & 1);

}

uint64_t helper_ave_u_df(CPUMIPSState *env, uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + (u_arg1 & u_arg2 & 1);
}


/*
 *  AVER_S, AVER_U
 */

int64_t helper_aver_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    /* signed shift */
    return (arg1 >> 1) + (arg2 >> 1) + ((arg1 | arg2) & 1);

}

uint64_t helper_aver_u_df(CPUMIPSState *env, uint64_t arg1, uint64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    /* unsigned shift */
    return (u_arg1 >> 1) + (u_arg2 >> 1) + ((u_arg1 | u_arg2) & 1);
}


/*
 *  BCLR, BNEG, BSET
 */

int64_t helper_bclr_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 & (~(1LL << b_arg2)), df);
}

int64_t helper_bclri_df(CPUMIPSState *env, int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bclr_df(env, arg1, arg2, df);
}

int64_t helper_bneg_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 ^ (1LL << b_arg2), df);
}

int64_t helper_bnegi_df(CPUMIPSState *env, int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bneg_df(env, arg1, arg2, df);
}

int64_t helper_bset_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return UNSIGNED(arg1 | (1LL << b_arg2), df);
}

int64_t helper_bseti_df(CPUMIPSState *env, int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_bset_df(env, arg1, arg2, df);
}


/*
 *  BINSL, BINSR
 */

int64_t helper_binsl_df(CPUMIPSState *env, int64_t dest,
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

int64_t helper_binsli_df(CPUMIPSState *env, int64_t dest,
                         int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_binsl_df(env, dest, arg1, arg2, df);
}

int64_t helper_binsr_df(CPUMIPSState *env, int64_t dest,
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

int64_t helper_binsri_df(CPUMIPSState *env, int64_t dest,
                        int64_t arg1, uint32_t arg2, uint32_t df)
{
    return helper_binsr_df(env, dest, arg1, arg2, df);
}


/*
 *  BMNZ
 */

#define BIT_MOVE_IF_NOT_ZERO(dest, arg1, arg2, df) \
            dest = UNSIGNED(((dest & (~arg2)) | (arg1 & arg2)), df)

void helper_bmnz_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_NOT_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bmnzi_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
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

void helper_bmz_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_MOVE_IF_ZERO(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bmzi_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
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

void helper_bsel_v(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        BIT_SELECT(D(pwd, i), D(pws, i), D(pwt, i), DF_DOUBLE);
    } DONE_ALL_ELEMENTS;
}

void helper_bseli_b(CPUMIPSState *env, void *pwd, void *pws, uint32_t arg2, uint32_t wrlen)
{
    ALL_B_ELEMENTS(i, wrlen) {
        BIT_SELECT(B(pwd, i), B(pws, i), arg2, DF_BYTE);
    } DONE_ALL_ELEMENTS;
}


/*
 *  BNZ, BZ
 */

target_ulong helper_bnz_df(CPUMIPSState *env, void *p_arg, uint32_t df, uint32_t wrlen)
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

target_ulong helper_bz_df(CPUMIPSState *env, void *p_arg, uint32_t df, uint32_t wrlen)
{
    return !helper_bnz_df(env, p_arg, df, wrlen);
}

target_ulong helper_bnz_v(CPUMIPSState *env, void *p_arg, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        if (D(p_arg, i) != 0) {
            return 1;
        }
    } DONE_ALL_ELEMENTS;

    return 0;
}

target_ulong helper_bz_v(CPUMIPSState *env, void *p_arg, uint32_t wrlen)
{
    return !helper_bnz_v(env, p_arg, wrlen);
}


/*
 *  CEQ, CLE_S, CLE_U, CLT_S, CLT_U
 */

int64_t helper_ceq_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 == arg2 ? -1 : 0;
}

int64_t helper_cle_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 <= arg2 ? -1 : 0;
}

int64_t helper_cle_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 <= u_arg2 ? -1 : 0;
}

int64_t helper_clt_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 < arg2 ? -1 : 0;
}

int64_t helper_clt_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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


int64_t helper_hadd_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return SIGNED_ODD(arg1, df) + SIGNED_EVEN(arg2, df);
}


int64_t helper_hadd_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return UNSIGNED_ODD(arg1, df) + UNSIGNED_EVEN(arg2, df);
}


int64_t helper_hsub_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return SIGNED_ODD(arg1, df) - SIGNED_EVEN(arg2, df);
}


int64_t helper_hsub_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return UNSIGNED_ODD(arg1, df) - UNSIGNED_EVEN(arg2, df);
}


int64_t helper_dotp_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

int64_t helper_dotp_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}


int64_t helper_dpadd_s_df(CPUMIPSState *env, int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}


int64_t helper_dpadd_u_df(CPUMIPSState *env, int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest + (even_arg1 * even_arg2) + (odd_arg1 * odd_arg2);
}

int64_t helper_dpsub_s_df(CPUMIPSState *env, int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    SIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    SIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}

int64_t helper_dpsub_u_df(CPUMIPSState *env, int64_t dest,
                          int64_t arg1, int64_t arg2, uint32_t df)
{
    UNSIGNED_EXTRACT(even_arg1, odd_arg1, arg1, df);
    UNSIGNED_EXTRACT(even_arg2, odd_arg2, arg2, df);

    return dest - ((even_arg1 * even_arg2) + (odd_arg1 * odd_arg2));
}



/*
 *  ILVEV, ILVOD, ILVL, ILVR, PCKEV, PCKOD, VSHF
 */

#define WRLEN(wrlen_df) (wrlen_df >> 2)
#define DF(wrlen_df) (wrlen_df & 0x03)

void helper_ilvev_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


void helper_ilvod_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


void helper_ilvl_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


void helper_ilvr_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


void helper_pckev_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


void helper_pckod_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}

void helper_vshf_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}


/*
 *  SHF
 */

#define SHF_POS(i, imm) ((i & 0xfc) + ((imm >> (2 * (i & 0x03))) & 0x03))

void helper_shf_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t imm, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    switch (df) {
    case DF_BYTE:
      ALL_B_ELEMENTS(i, wrlen) {
        B(pwx, i) = B(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;

    case DF_HALF:
      ALL_H_ELEMENTS(i, wrlen) {
        H(pwx, i) = H(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;

    case DF_WORD:
      ALL_W_ELEMENTS(i, wrlen) {
        W(pwx, i) = W(pws, SHF_POS(i, imm));
      } DONE_ALL_ELEMENTS;
      break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    helper_move_v(env, pwd, &wx, wrlen);
}


/*
 *  MADDV, MSUBV
 */

int64_t helper_maddv_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    return dest + arg1 * arg2;
}

int64_t helper_msubv_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    return dest - arg1 * arg2;
}


/*
 *  MAX, MIN
 */

int64_t helper_max_a_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 > abs_arg2 ? arg1 : arg2;
}


int64_t helper_max_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 > arg2 ? arg1 : arg2;
}


int64_t helper_max_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 > u_arg2 ? arg1 : arg2;
}


int64_t helper_min_a_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t abs_arg1 = arg1 >= 0 ? arg1 : -arg1;
    uint64_t abs_arg2 = arg2 >= 0 ? arg2 : -arg2;

    return abs_arg1 < abs_arg2 ? arg1 : arg2;
}


int64_t helper_min_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 < arg2 ? arg1 : arg2;
}


int64_t helper_min_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg1 < u_arg2 ? arg1 : arg2;
}


/*
 *  SPLAT, and MOVE_V
 */

void helper_splat_df(CPUMIPSState *env, void *pwd, void *pws, target_ulong rt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    uint32_t n = rt % DF_ELEMENTS(df, wrlen);

    msa_check_index(env, df, n, wrlen);

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

void helper_move_v(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen)
{
    ALL_D_ELEMENTS(i, wrlen) {
        D(pwd, i) = D(pws, i);
    } DONE_ALL_ELEMENTS;
}


/*
 *  LDI, FILL, INSERT, INSVE
 */
void helper_ldi_df(CPUMIPSState *env, void *pwd, uint32_t df, uint32_t s10, uint32_t wrlen)
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

void helper_fill_df(CPUMIPSState *env, void *pwd, target_ulong rs, uint32_t wrlen_df)
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

void helper_insert_df(CPUMIPSState *env, void *pwd, target_ulong rs, uint32_t n, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    msa_check_index(env, df, n, wrlen);

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

void helper_insve_df(CPUMIPSState *env, void *pwd, void *pws, target_ulong n, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    msa_check_index(env, df, n, wrlen);

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

int64_t helper_mulv_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    return arg1 * arg2;
}

int64_t helper_div_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
  if (arg1 == DF_MIN_INT(df) && arg2 == -1)
    return DF_MIN_INT(df);

  return arg2 ? arg1 / arg2 : 0;
}

int64_t helper_div_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg2 ? u_arg1 / u_arg2 : 0;
}

int64_t helper_mod_s_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
  if (arg1 == DF_MIN_INT(df) && arg2 == -1)
    return 0;

  return arg2 ? arg1 % arg2 : 0;
}

int64_t helper_mod_u_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    uint64_t u_arg2 = UNSIGNED(arg2, df);

    return u_arg2 ? u_arg1 % u_arg2 : 0;
}


/*
 *  NLZC, NLOC, and PCNT
 */

int64_t helper_nlzc_df(CPUMIPSState *env, int64_t arg, uint32_t df)
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

int64_t helper_nloc_df(CPUMIPSState *env, int64_t arg, uint32_t df)
{
    return helper_nlzc_df(env, UNSIGNED((~arg), df), df);
}


int64_t helper_pcnt_df(CPUMIPSState *env, int64_t arg, uint32_t df)
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

int64_t helper_sat_u_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    uint64_t u_arg = UNSIGNED(arg, df);
    return  u_arg < M_MAX_UINT(m+1) ? u_arg :
                                      M_MAX_UINT(m+1);
}


int64_t helper_sat_s_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    return arg < M_MIN_INT(m+1) ? M_MIN_INT(m+1) :
                                  arg > M_MAX_INT(m+1) ? M_MAX_INT(m+1) :
                                                         arg;
}


/*
 *  SLL, SRA, SRL
 */

int64_t helper_sll_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 << b_arg2;
}


int64_t helper_slli_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    return arg << m;
}


int64_t helper_sra_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int32_t b_arg2 = BIT_POSITION(arg2, df);
    return arg1 >> b_arg2;
}


int64_t helper_srai_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    return arg >> m;
}


int64_t helper_srl_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    uint64_t u_arg1 = UNSIGNED(arg1, df);
    int32_t b_arg2 = BIT_POSITION(arg2, df);

    return u_arg1 >> b_arg2;
}


int64_t helper_srli_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    uint64_t u_arg = UNSIGNED(arg, df);

    return u_arg >> m;
}


/*
 *  SRAR, SRLR
 */

int64_t helper_srar_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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


int64_t helper_srari_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
{
    if (m == 0) {
      return arg;
    }
    else {
      int64_t r_bit = (arg >> (m - 1)) & 1;
      return (arg >> m) + r_bit;
    }
}


int64_t helper_srlr_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
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


int64_t helper_srlri_df(CPUMIPSState *env, int64_t arg, uint32_t m, uint32_t df)
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

void helper_sld_df(CPUMIPSState *env, void *pwd, void *pws, target_ulong rt, uint32_t wrlen_df)
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

    msa_check_index(env, df, n, wrlen);

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

int64_t helper_mul_q_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2) >> (DF_BITS(df) - 1);
}

int64_t helper_mulr_q_df(CPUMIPSState *env, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_min  = DF_MIN_INT(df);
    int64_t q_max  = DF_MAX_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    if (arg1 == q_min && arg2 == q_min) {
        return q_max;
    }

    return (arg1 * arg2 + r_bit) >> (DF_BITS(df) - 1);
}

int64_t helper_madd_q_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_maddr_q_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) + q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_msub_q_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


int64_t helper_msubr_q_df(CPUMIPSState *env, int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    int64_t q_prod, q_ret;

    int64_t q_max  = DF_MAX_INT(df);
    int64_t q_min  = DF_MIN_INT(df);
    int64_t r_bit  = 1 << (DF_BITS(df) - 2);

    q_prod = arg1 * arg2;
    q_ret = ((dest << (DF_BITS(df) - 1)) - q_prod + r_bit) >> (DF_BITS(df) - 1);

    return (q_ret < q_min) ? q_min : (q_max < q_ret) ? q_max : q_ret;
}


/* MSA helper */
#include "mips_msa_helper_dummy.h"

int64_t helper_load_wr_elem_s64(CPUMIPSState *env, int32_t wreg, int32_t df, int32_t i)
{
    int wrlen = 128;

    i %= DF_ELEMENTS(df, wrlen);
    msa_check_index(env, (uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

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

target_ulong helper_load_wr_elem_target_s64(CPUMIPSState *env, int32_t wreg, int32_t df, int32_t i)
{
  return (target_ulong)helper_load_wr_elem_s64(env, wreg, df, i);
}



uint64_t helper_load_wr_elem_i64(CPUMIPSState *env, int32_t wreg, int32_t df, int32_t i)
{
    int wrlen = 128;

    i %= DF_ELEMENTS(df, wrlen);
    msa_check_index(env, (uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

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

target_ulong helper_load_wr_elem_target_i64(CPUMIPSState *env, int32_t wreg, int32_t df, int32_t i)
{
  return (target_ulong)helper_load_wr_elem_i64(env, wreg, df, i);
}


void helper_store_wr_elem(CPUMIPSState *env, uint64_t val, int32_t wreg, int32_t df, int32_t i)
{
    int wrlen = 128;

    i %= DF_ELEMENTS(df, wrlen);
    msa_check_index(env, (uint32_t)df, (uint32_t)i, (uint32_t)wrlen);

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

void helper_store_wr_elem_target(CPUMIPSState *env, target_ulong val, int32_t wreg, int32_t df, int32_t i)
{
  return helper_store_wr_elem(env, (uint64_t)val, wreg, df, i);
}





/*
 *  MSA Floating-point operations
 */

static void clear_msacsr_cause(CPUMIPSState *env) {
    SET_FP_CAUSE(env->active_msa.msacsr, 0);
}


static void check_msacsr_cause(CPUMIPSState *env)
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
      helper_raise_exception(env, EXCP_MSAFPE);
  }
}


/* Flush-to-zero use cases for update_msacsr() */
#define CLEAR_FS_UNDERFLOW 1
#define CLEAR_IS_INEXACT   2
#define RECIPROCAL_INEXACT 4

static int update_msacsr(CPUMIPSState *env, int action, int denormal)
{
    int ieee_ex;

    int c;
    int cause;
    int enable;

    ieee_ex = get_float_exception_flags(&env->active_msa.fp_status);

    /* QEMU softfloat does not signal all underflow cases */
    if (denormal) {
#if DEBUG_MSACSR
      puts("FORCING UNDERFLOW");
#endif
      ieee_ex |= float_flag_underflow;
    }

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

    c = ieee_ex_to_mips(env, ieee_ex);
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

#define float16_is_zero(ARG) 0
#define float16_is_zero_or_denormal(ARG) 0

#define IS_DENORMAL(ARG, BITS)                                   \
  (!float ## BITS ## _is_zero(ARG)                               \
   && float ## BITS ## _is_zero_or_denormal(ARG))                \


#define MSA_FLOAT_UNOP0(DEST, OP, ARG, BITS)                            \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## OP(ARG,                                \
                                    &env->active_msa.fp_status);        \
    c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
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
    c = update_msacsr(env, CLEAR_FS_UNDERFLOW, 0);                      \
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
    c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
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
    c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
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
    c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
    enable = GET_FP_ENABLE(env->active_msa.msacsr) | FP_UNIMPLEMENTED;  \
    cause = c & enable;                                                 \
                                                                        \
    if (cause) {                                                        \
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_MAXOP(DEST, OP, ARG1, ARG2, BITS)                     \
  do {                                                                  \
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
      DEST = ((FLOAT_SNAN ## BITS >> 6) << 6) | c;                      \
    }                                                                   \
  } while (0)

#define MSA_FLOAT_RECIPROCAL(DEST, ARG, BITS)                           \
  do {                                                                  \
    int c;                                                              \
    int cause;                                                          \
    int enable;                                                         \
                                                                        \
    set_float_exception_flags(0, &env->active_msa.fp_status);           \
    DEST = float ## BITS ## _ ## div(FLOAT_ONE ## BITS, ARG,            \
                                     &env->active_msa.fp_status);       \
    c = update_msacsr(env, float ## BITS ## _is_infinity(ARG) ||        \
                      float ## BITS ## _is_quiet_nan(DEST)?             \
                      0 : RECIPROCAL_INEXACT,                           \
                      IS_DENORMAL(DEST, BITS));                         \
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
    c = update_msacsr(env, 0, IS_DENORMAL(DEST, BITS));                 \
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

void helper_fadd_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fsub_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fmul_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fdiv_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


/*
 *  FSQRT
 */

void helper_fsqrt_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


/*
 *  FEXP2, FLOG2
 */

void helper_fexp2_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_flog2_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


/*
 *  FMADD, FMSUB
 */

void helper_fmadd_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fmsub_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
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
  MSA_FLOAT_MAXOP(xs, F,  S,  T, BITS);                         \
  MSA_FLOAT_MAXOP(xt, G,  S,  T, BITS);                         \
  MSA_FLOAT_MAXOP(xd, F, as, at, BITS);                         \
                                                                \
  X = (as == at || xd == float## BITS ##_abs(xs)) ? xs : xt;


void helper_fmax_a_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_fmax_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pws, i), W(pws, i), 32);
            }
            else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pwt, i), W(pwt, i), 32);
            }
            else {
                MSA_FLOAT_MAXOP(W(pwx, i), max, W(pws, i), W(pwt, i), 32);
            }
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pws, i), D(pws, i), 64);
            }
            else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pwt, i), D(pwt, i), 64);
            }
            else {
                MSA_FLOAT_MAXOP(D(pwx, i), max, D(pws, i), D(pwt, i), 64);
            }
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_fmin_a_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_fmin_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(W(pws, i), W(pwt, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pws, i), W(pws, i), 32);
            }
            else if (NUMBER_QNAN_PAIR(W(pwt, i), W(pws, i), 32)) {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pwt, i), W(pwt, i), 32);
            }
            else {
                MSA_FLOAT_MAXOP(W(pwx, i), min, W(pws, i), W(pwt, i), 32);
            }
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            if (NUMBER_QNAN_PAIR(D(pws, i), D(pwt, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pws, i), D(pws, i), 64);
            }
            else if (NUMBER_QNAN_PAIR(D(pwt, i), D(pws, i), 64)) {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pwt, i), D(pwt, i), 64);
            }
            else {
                MSA_FLOAT_MAXOP(D(pwx, i), min, D(pws, i), D(pwt, i), 64);
            }
         } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
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
    c = update_msacsr(env, CLEAR_IS_INEXACT, 0);                        \
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
compare_af(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcaf_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_af(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsaf_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_af(env, pwd, pws, pwt, wrlen_df, 0);
}

static void
compare_eq(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fceq_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_eq(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fseq_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_eq(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ueq(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcueq_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ueq(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsueq_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ueq(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ne(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcne_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ne(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsne_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ne(env, pwd, pws, pwt, wrlen_df, 0);
}

static void
compare_une(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcune_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_une(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsune_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_une(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_le(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcle_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_le(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsle_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_le(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ule(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcule_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ule(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsule_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ule(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_lt(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fclt_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_lt(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fslt_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_lt(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_ult(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcult_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ult(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsult_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_ult(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_un(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_fcun_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_un(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsun_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_un(env, pwd, pws, pwt, wrlen_df, 0);
}


static void
compare_or(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df, int quiet) {
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}
void helper_fcor_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_or(env, pwd, pws, pwt, wrlen_df, 1);
}

void helper_fsor_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
  compare_or(env, pwd, pws, pwt, wrlen_df, 0);
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


int64_t helper_fclass_df(CPUMIPSState *env, int64_t arg, uint32_t df)
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

static float32 float32_from_float16(int16_t a, flag ieee STATUS_PARAM) {
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

void helper_fexdo_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, &wx, wrlen);
}

void helper_fexupl_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, &wx, wrlen);
}

void helper_fexupr_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, &wx, wrlen);
}


/*
 *  FFINT, FTINT, FTRUNC, FRINT
 */

#define float32_from_int32 int32_to_float32
#define float32_from_uint32 uint32_to_float32

#define float64_from_int64 int64_to_float64
#define float64_from_uint64 uint64_to_float64


void helper_ffint_s_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_ffint_u_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_ftint_s_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_ftint_u_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_ftrunc_s_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_int32_round_to_zero, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_int64_round_to_zero, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_ftrunc_u_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(W(pwx, i), to_uint32_round_to_zero, W(pws, i), 32);
        } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_UNOP0(D(pwx, i), to_uint64_round_to_zero, D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

void helper_frint_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


/*
 *  FFQ, FTQ
 */

static float32 float32_from_q16(int16_t a STATUS_PARAM)
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

static int16_t float32_to_q16(float32 a STATUS_PARAM)
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

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
                              STATUS_VAR);

    if (ieee_ex & float_flag_overflow) {
      float_raise( float_flag_inexact STATUS_VAR);
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
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int32)a < 0 ? q_min : q_max;
    }

    if (q_val < q_min) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int16_t)q_min;
    }

    if (q_max < q_val) {
      float_raise( float_flag_overflow | float_flag_inexact STATUS_VAR);
      return (int16_t)q_max;
    }

    return (int16_t)q_val;
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

    ieee_ex = get_float_exception_flags(status);
    set_float_exception_flags(ieee_ex & (~float_flag_underflow)
                              STATUS_VAR);

    if (ieee_ex & float_flag_overflow) {
      float_raise( float_flag_inexact STATUS_VAR);
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

void helper_ffql_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}

void helper_ffqr_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
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

    helper_move_v(env, pwd, &wx, wrlen);
}

void helper_ftq_df(CPUMIPSState *env, void *pwd, void *pws, void *pwt, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

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

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}

/*
 *  FRCP, FRSQRT
 */

void helper_frcp_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
            MSA_FLOAT_RECIPROCAL(W(pwx, i), W(pws, i), 32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
            MSA_FLOAT_RECIPROCAL(D(pwx, i), D(pws, i), 64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}


void helper_frsqrt_df(CPUMIPSState *env, void *pwd, void *pws, uint32_t wrlen_df)
{
    uint32_t df = DF(wrlen_df);
    uint32_t wrlen = WRLEN(wrlen_df);

    wr_t wx, *pwx = &wx;

    clear_msacsr_cause(env);

    switch (df) {
    case DF_WORD:
        ALL_W_ELEMENTS(i, wrlen) {
          MSA_FLOAT_RECIPROCAL(W(pwx, i),
                               float32_sqrt(W(pws, i),
                                            &env->active_msa.fp_status),
                               32);
         } DONE_ALL_ELEMENTS;
        break;

    case DF_DOUBLE:
        ALL_D_ELEMENTS(i, wrlen) {
          MSA_FLOAT_RECIPROCAL(D(pwx, i),
                               float64_sqrt(D(pws, i),
                                            &env->active_msa.fp_status),
                               64);
        } DONE_ALL_ELEMENTS;
        break;

    default:
        /* shouldn't get here */
      assert(0);
    }

    check_msacsr_cause(env);
    helper_move_v(env, pwd, pwx, wrlen);
}



/*
 *  MSA Control Register (MSACSR) instructions: CFCMSA, CTCMSA
 */

target_ulong helper_cfcmsa(CPUMIPSState *env, uint32_t cs)
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


void helper_ctcmsa(CPUMIPSState *env, target_ulong elm, uint32_t cd)
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
            helper_raise_exception(env, EXCP_MSAFPE);
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


/*
 *  MIPS R6 DLSA and LSA
 */

#define LSA(rs, rt, u2) ((rs << (u2 + 1)) + rt)

target_ulong helper_dlsa(CPUMIPSState *env, target_ulong rt, target_ulong rs, uint32_t u2)
{
  return LSA(rs, rt, u2);
}


target_ulong helper_lsa(CPUMIPSState *env, target_ulong rt, target_ulong rs, uint32_t u2)
{
  return (uint32_t)LSA(rs, rt, u2);
}
