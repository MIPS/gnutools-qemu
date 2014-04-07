/*
 *  MIPS emulation helpers for qemu.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
 *  Copyright (c) 2011 Reed Kotler/MIPS Technologies - DSP ASE
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>

#include "cpu.h"

enum {
    TLBRET_GUESTEXIT = -10,
    TLBRET_DIRTY = -4,
    TLBRET_INVALID = -3,
    TLBRET_NOMATCH = -2,
    TLBRET_BADADDR = -1,
    TLBRET_MATCH = 0
};

#if !defined(CONFIG_USER_ONLY)

/* no MMU emulation */
int no_mmu_map_address (CPUState *env, target_phys_addr_t *physical, int *prot,
                        uint64_t address, int rw, int access_type)
{
    *physical = address;
    *prot = PAGE_READ | PAGE_WRITE;
    return TLBRET_MATCH;
}

/* fixed mapping MMU emulation */
int fixed_mmu_map_address (CPUState *env, target_phys_addr_t *physical, int *prot,
                           uint64_t address, int rw, int access_type)
{
    if (address <= (int32_t)0x7FFFFFFFUL) {
        if (!(env->CP0_Status & (1 << CP0St_ERL)))
            *physical = address + 0x40000000UL;
        else
            *physical = address;
    } else if (address <= (int32_t)0xBFFFFFFFUL)
        *physical = address & 0x1FFFFFFF;
    else
        *physical = address;

    *prot = PAGE_READ | PAGE_WRITE;
    return TLBRET_MATCH;
}

static inline bool isDrgValid(CPUState *env, bool instruction)
{
    return ((env->CP0_GuestCtl0 & (1 << CP0GuestCtl0_DRG) &&
             !(env->hflags & MIPS_HFLAG_GUEST) &&
             (env->hflags & MIPS_HFLAG_KSU)    &&
             ((env->CP0_GuestCtl1 >> CP0GuestCtl1_RID) & 0xff) != 0) &&
            !instruction);
}

/* MIPS32/MIPS64 R4000-style MMU emulation */
int r4k_map_address (CPUState *env, target_phys_addr_t *physical, int *prot,
                     uint64_t address, int rw, int guestCtx)
{
    uint8_t ASID;
    int i;
    int guestId;

    if (env->hflags & MIPS_HFLAG_GUEST) {
        ASID = env->Guest.CP0_EntryHi & 0xFF;
        guestId = (env->CP0_GuestCtl1 >> CP0GuestCtl1_ID) & 0xff;
    } else {
        ASID = env->CP0_EntryHi & 0xFF;
        if (isDrgValid(env, rw == 2)) {
            guestId = (env->CP0_GuestCtl1 >> CP0GuestCtl1_RID) & 0xff;
        } else {
            guestId = 0;
        }
    }
#ifdef SV_SUPPORT
    if (address & 1) {
        sv_log("tlb badaddr\n");
        return TLBRET_BADADDR;
    }
#endif
    for (i = 0; i < env->tlb->tlb_in_use; i++) {
        r4k_tlb_t *tlb = &env->tlb->mmu.r4k.tlb[i];
        /* 1k pages are not supported. */
        uint64_t mask = tlb->PageMask | ~(TARGET_PAGE_MASK << 1);
        uint64_t tag = address & ~mask;
        uint64_t VPN = tlb->VPN & ~mask;
#if defined(TARGET_MIPS64)
        tag &= env->SEGMask;
#endif
        /* Check ASID, virtual page number & size */
        if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag 
            && !tlb->hardware_invalid && tlb->GuestID == guestId 
            && tlb->isGuestCtx == guestCtx) {
            /* TLB match */
            int n = !!(address & mask & ~(mask >> 1));
            /* Check access rights */
            if (!(n ? tlb->V1 : tlb->V0)) {
#ifdef SV_SUPPORT
                sv_log("TLB[%i] INVALID address = %08x, mask = %08x, (VPN = %08x), tag = %08x\n", i,  (unsigned)address, (unsigned)mask, (unsigned)VPN, (unsigned)tag);
#endif
                return TLBRET_INVALID;
            }
            /*else if (tlb->hardware_invalid) {
                sv_log("tlb hardware_invalid\n");
                return TLBRET_INVALID;
            }*/
            if (rw == 0 || rw == 2 || (n ? tlb->D1 : tlb->D0)) {
                *physical = (tlb->PFN[n] << TARGET_PAGE_BITS) | (address & (mask >> 1));
                *prot = PAGE_READ;
                if (n ? tlb->D1 : tlb->D0)
                    *prot |= PAGE_WRITE;
                return TLBRET_MATCH;
            }
#ifdef SV_SUPPORT
            sv_log("tlb dirty %d\n", i);
#endif
            return TLBRET_DIRTY;
        }
    }
#ifdef SV_SUPPORT
    sv_log("tlb nomatch\n");
#endif
    return TLBRET_NOMATCH;
}

#ifdef SV_SUPPORT
/* MIPS32/MIPS64 R4000-style MMU emulation */
// TODO: this almost identical to r4k_map_address (only cca added for debugging)
int r4k_map_address_debug (CPUState *env, target_phys_addr_t *physical, int *prot, int *cca,
                           target_ulong address, int rw, int guestCtx)
{
    uint8_t ASID;
    int i;
    int guestId;

    if (env->hflags & MIPS_HFLAG_GUEST) {
        ASID = env->Guest.CP0_EntryHi & 0xFF;
        guestId = (env->CP0_GuestCtl1 >> CP0GuestCtl1_ID) & 0xff;
    } else {
        ASID = env->CP0_EntryHi & 0xFF;
        if (isDrgValid(env, rw == 2)) {
            guestId = (env->CP0_GuestCtl1 >> CP0GuestCtl1_RID) & 0xff;
        } else {
            guestId = 0;
        }
    }    

    for (i = 0; i < env->tlb->tlb_in_use; i++) {
        r4k_tlb_t *tlb = &env->tlb->mmu.r4k.tlb[i];
        /* 1k pages are not supported. */
        target_ulong mask = tlb->PageMask | ~(TARGET_PAGE_MASK << 1);
        target_ulong tag = address & ~mask;
        target_ulong VPN = tlb->VPN & ~mask;
#if defined(TARGET_MIPS64)
        tag &= env->SEGMask;
#endif

        /* Check ASID, virtual page number & size */
        if ((tlb->G == 1 || tlb->ASID == ASID) && VPN == tag 
            && !tlb->hardware_invalid && tlb->GuestID == guestId
            && tlb->isGuestCtx == guestCtx) {
            /* TLB match */
            int n = !!(address & mask & ~(mask >> 1));
            /* Check access rights */
            if (!(n ? tlb->V1 : tlb->V0))
                return TLBRET_INVALID;
            if (rw == 0 || (n ? tlb->D1 : tlb->D0)) {
                *physical = (tlb->PFN[n] << TARGET_PAGE_BITS) | (address & (mask >> 1));
                *prot = PAGE_READ;
                if (n ? tlb->D1 : tlb->D0)
                    *prot |= PAGE_WRITE;
                *cca = n? tlb->C1 : tlb->C0;
                return TLBRET_MATCH;
            }
            return TLBRET_DIRTY;
        }
    }
    return TLBRET_NOMATCH;
}
#endif

static int get_physical_address (CPUState *env, target_phys_addr_t *physical,
                                int *prot, target_ulong address,
                                int rw, int access_type)
{
    /* User mode can only access useg/xuseg */
    int guest_mode = !!(env->hflags & MIPS_HFLAG_GUEST);
    int user_mode = (env->hflags & MIPS_HFLAG_MODE) == MIPS_HFLAG_UM;
    int supervisor_mode = (env->hflags & MIPS_HFLAG_MODE) == MIPS_HFLAG_SM;
    int kernel_mode = !user_mode && !supervisor_mode;
    int guestCtx;
#if defined(TARGET_MIPS64)
    int UX = (env->CP0_Status & (1 << CP0St_UX)) != 0;
    int SX = (env->CP0_Status & (1 << CP0St_SX)) != 0;
    int KX = (env->CP0_Status & (1 << CP0St_KX)) != 0;
#endif
    int ret = TLBRET_MATCH;
    *physical = address;
#if 0
    qemu_log("user mode %d h %08x\n", user_mode, env->hflags);
#endif
    int GuestID = -1;
    if (guest_mode) {
        guestCtx = 1;
//        sv_log("guest mode translation\n");
        target_phys_addr_t gpa;

        if (!(env->CP0_GuestCtl0 >> CP0GuestCtl0_RAD)) {
            GuestID = (env->CP0_GuestCtl1 >> CP0GuestCtl1_ID) & 0xff;
        }
        // Lookup to Config0 CP0C0_MT
        if (address <= (int32_t)0x7FFFFFFFUL) {
            /* useg */
            if (env->Guest.CP0_Status & (1 << CP0St_ERL)) {
                gpa = address & 0xFFFFFFFF;
                *prot = PAGE_READ | PAGE_WRITE;
            } else {
                ret = env->guest_tlb->map_address(env, &gpa, prot, address, rw, guestCtx);
            }
        } else if (address < (int32_t)0xA0000000UL) {
            /* kseg0 */
            if (kernel_mode) {
                gpa = address - (int32_t)0x80000000UL;
                *prot = PAGE_READ | PAGE_WRITE;
            } else {
                ret = TLBRET_BADADDR;
            }
        } else if (address < (int32_t)0xC0000000UL) {
            /* kseg1 */
            if (kernel_mode) {
                gpa = address - (int32_t)0xA0000000UL;
                *prot = PAGE_READ | PAGE_WRITE;
            } else {
                ret = TLBRET_BADADDR;
            }
        } else if (address < (int32_t)0xE0000000UL) {
            /* sseg (kseg2) */
            if (supervisor_mode || kernel_mode) {
                ret = env->guest_tlb->map_address(env, &gpa, prot, address, rw, guestCtx);
            } else {
                ret = TLBRET_BADADDR;
            }
        } else {
            /* kseg3 */
            /* XXX: debug segment is not emulated */
            if (kernel_mode) {
                ret = env->guest_tlb->map_address(env, &gpa, prot, address, rw, guestCtx);
            } else {
                ret = TLBRET_BADADDR;
            }
        }
        if (ret != TLBRET_MATCH) {
            // guest exception
#ifdef SV_SUPPORT
            sv_log("guest exception %d\n", ret);
#endif
            return ret;
        }
        //root tlb lookup with ignoring root asid
        //guestid = n
        //root tlb
//        int GExcCode = if(ret)
//        sv_log("GVA->GPA done\n");
        guestCtx = 0;
        ret = env->tlb->map_address(env, physical, prot, gpa, rw, guestCtx);
        if (ret != TLBRET_MATCH) {
#if 0
            // root exception
            // switch to root mode?
            // clear GM bit
            env->CP0_GuestCtl0 &= ~(1 << CP0GuestCtl0_GM);
            env->CP0_GuestCtl0 |= (1 << CP0GuestCtl0_GM);
            // clear GExcCode bits
            env->CP0_GuestCtl0 &= ~(0x1F << CP0GuestCtl0_GExcCode);
            if(ret == TLBRET_NOMATCH) {
                env->CP0_GuestCtl0 |= (GVA << CP0GuestCtl0_GExcCode);
            }
            else {
                env->CP0_GuestCtl0 |= (GPA << CP0GuestCtl0_GExcCode);
            }
            env->hflags &= ~MIPS_HFLAG_GUEST;
//            env->hflags |= MIPS_HFLAG_ROOT;
#endif
#ifdef SV_SUPPORT
            sv_log("root exception (%x) %x %d\n", env->hflags & MIPS_HFLAG_GUEST, (unsigned) address, ret);
#endif
            *physical = gpa;
            ret += TLBRET_GUESTEXIT;
        }
        return ret;
    }
    else
    {
        guestCtx = 0;
    if (address <= (int32_t)0x7FFFFFFFUL) {
        /* useg */
        if (env->CP0_Status & (1 << CP0St_ERL)) {
            *physical = address & 0xFFFFFFFF;
            *prot = PAGE_READ | PAGE_WRITE;
        } else {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        }
#if defined(TARGET_MIPS64)
    } else if (address < 0x4000000000000000ULL) {
        /* xuseg */
        if (UX && address <= (0x3FFFFFFFFFFFFFFFULL & env->SEGMask)) {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        } else {
            ret = TLBRET_BADADDR;
        }
    } else if (address < 0x8000000000000000ULL) {
        /* xsseg */
        if ((supervisor_mode || kernel_mode) &&
            SX && address <= (0x7FFFFFFFFFFFFFFFULL & env->SEGMask)) {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        } else {
            ret = TLBRET_BADADDR;
        }
    } else if (address < 0xC000000000000000ULL) {
        /* xkphys */
        if (kernel_mode && KX &&
            (address & 0x07FFFFFFFFFFFFFFULL) <= env->PAMask) {
            *physical = address & env->PAMask;
            *prot = PAGE_READ | PAGE_WRITE;
        } else {
            ret = TLBRET_BADADDR;
        }
    } else if (address < 0xFFFFFFFF80000000ULL) {
        /* xkseg */
        if (kernel_mode && KX &&
            address <= (0xFFFFFFFF7FFFFFFFULL & env->SEGMask)) {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        } else {
            ret = TLBRET_BADADDR;
        }
#endif
    } else if (address < (int32_t)0xA0000000UL) {
        /* kseg0 */
        if (kernel_mode) {
            *physical = address - (int32_t)0x80000000UL;
            *prot = PAGE_READ | PAGE_WRITE;
        } else {
            ret = TLBRET_BADADDR;
        }
    } else if (address < (int32_t)0xC0000000UL) {
        /* kseg1 */
        if (kernel_mode) {
            *physical = address - (int32_t)0xA0000000UL;
            *prot = PAGE_READ | PAGE_WRITE;
        } else {
            ret = TLBRET_BADADDR;
        }
    } else if (address < (int32_t)0xE0000000UL) {
        /* sseg (kseg2) */
        if (supervisor_mode || kernel_mode) {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        } else {
            ret = TLBRET_BADADDR;
        }
    } else {
        /* kseg3 */
        /* XXX: debug segment is not emulated */
        if (kernel_mode) {
            ret = env->tlb->map_address(env, physical, prot, address, rw, guestCtx);
        } else {
            ret = TLBRET_BADADDR;
        }
    }
    }
#if 0
    qemu_log(TARGET_FMT_lx " %d %d => " TARGET_FMT_lx " %d (%d)\n",
            address, rw, access_type, *physical, *prot, ret);
#endif

    return ret;
}
#endif

static void raise_mmu_exception(CPUState *env, uint64_t address,
                                int rw, int tlb_error)
{
    int exception = 0, error_code = 0;
    bool guestTLBException;

    if (env->hflags & MIPS_HFLAG_GUEST) {
        if (tlb_error < TLBRET_GUESTEXIT) {
            // exit Guest mode and indicate Root TLB exception
            env->exitGuest = 1;
            guestTLBException = false;
            tlb_error -= TLBRET_GUESTEXIT;
        } else {
            // Guest TLB exception
            guestTLBException = true;
        }
    } else {
        // Root TLB exception (Root mode)
        guestTLBException = false;
    }

    switch (tlb_error) {
    default:
    case TLBRET_BADADDR:
        /* Reference to kernel address from user mode or supervisor mode */
        /* Reference to supervisor address from user mode */
        if (rw == 1)
            exception = EXCP_AdES;
        else
            exception = EXCP_AdEL;
        break;
    case TLBRET_NOMATCH:
        /* No TLB match for a mapped address */
        if (rw == 1)
            exception = EXCP_TLBS;
        else
            exception = EXCP_TLBL;
        error_code = 1;
        break;
    case TLBRET_INVALID:
        /* TLB match with no valid bit */
        if (rw == 1)
            exception = EXCP_TLBS;
        else
            exception = EXCP_TLBL;
        break;
    case TLBRET_DIRTY:
        /* TLB match but 'D' bit is cleared */
        exception = EXCP_LTLBL;
        break;
    }

    /* Raise exception */
    if (guestTLBException) {
        env->Guest.CP0_BadVAddr = address;
        env->Guest.CP0_Context = (env->Guest.CP0_Context & ~0x007fffff) |
            ((address >> 9) & 0x007ffff0);
    } else {
        env->CP0_BadVAddr = address;
        env->CP0_Context = (env->CP0_Context & ~0x007fffff) |
            ((address >> 9) & 0x007ffff0);
    }

#if defined(SV_SUPPORT)
    if (exception != EXCP_AdES && exception != EXCP_AdEL) {
        /* "MIPS Architecture for Programmers, Volume III: The MIPS32 and microMIPS
           Privileged Resource Architecture", Revision 5.03 Sept. 9, 2013
           - according to the manual, on Address Error Exception ContextVPN2,
           EntryHiVPN2 values are unpredictable, so in general it shouldn't
           matter whether these registers are changed or not in the simulator.
           However, in IASim EntryHiVPN2 doesn't seem to be changed,
           thus we modify qemu to work in the same way to make diff tests pass.
        */
#endif
    if (guestTLBException) {
        /* When an exception is triggered as a result of a root TLB access
         * during guest-mode execution, the handler will be executed in
         * root mode, and exception state is stored into root CP0 registers.
         * The registers affected are GuestCtl0, Root.EPC, Root.BadVAddr,
         * Root.EntryHi, Root.Cause and Root.ContextBadVPN2. */
        env->Guest.CP0_EntryHi =
            (env->Guest.CP0_EntryHi & 0x4FF) | (address & (TARGET_PAGE_MASK << 1));
    } else {
        env->CP0_EntryHi =
            (env->CP0_EntryHi & 0x4FF) | (address & (TARGET_PAGE_MASK << 1));
    }
#if defined(SV_SUPPORT)
    }
#endif
#if defined(TARGET_MIPS64)
    env->CP0_EntryHi &= env->SEGMask;
    env->CP0_XContext = (env->CP0_XContext & ((~0ULL) << (env->SEGBITS - 7))) |
                        ((address & 0xC00000000000ULL) >> (55 - env->SEGBITS)) |
                        ((address & ((1ULL << env->SEGBITS) - 1) & 0xFFFFFFFFFFFFE000ULL) >> 9);
#endif
    env->exception_index = exception;
    env->error_code = error_code;
}

#if !defined(CONFIG_USER_ONLY)
target_phys_addr_t cpu_get_phys_page_debug(CPUState *env, target_ulong addr)
{
    target_phys_addr_t phys_addr;
    int prot;

    if (get_physical_address(env, &phys_addr, &prot, addr, 0, ACCESS_INT) != 0)
        return -1;
    return phys_addr;
}
#endif

int cpu_mips_handle_mmu_fault (CPUState *env, target_ulong address, int rw,
                               int mmu_idx)
{
#if !defined(CONFIG_USER_ONLY)
    target_phys_addr_t physical;
    int prot;
    int access_type;
#endif
    int ret = 0;

#if 0
    log_cpu_state(env, 0);
#endif
    qemu_log("%s pc " TARGET_FMT_lx " ad " TARGET_FMT_lx " rw %d mmu_idx %d\n",
              __func__, env->active_tc.PC, address, rw, mmu_idx);

    /* data access */
#if !defined(CONFIG_USER_ONLY)
    /* XXX: put correct access by using cpu_restore_state()
       correctly */
    access_type = ACCESS_INT;
    ret = get_physical_address(env, &physical, &prot,
                               address, rw, access_type);
    qemu_log("%s address=" TARGET_FMT_lx " ret %d physical " TARGET_FMT_plx " prot %d\n",
              __func__, address, ret, physical, prot);
    if (ret == TLBRET_MATCH) {
        tlb_set_page(env, address & TARGET_PAGE_MASK,
                     physical & TARGET_PAGE_MASK, prot | PAGE_EXEC,
                     mmu_idx, TARGET_PAGE_SIZE);
        ret = 0;
    } else if (ret < 0)
    {
        if (env->hflags & MIPS_HFLAG_GUEST) {
            // clear GExcCode bits
            env->CP0_GuestCtl0 &= ~(0x1F << CP0GuestCtl0_GExcCode);
            if (ret < TLBRET_GUESTEXIT)
            {
                env->CP0_GuestCtl0 &= ~(1 << CP0GuestCtl0_GM);
                env->CP0_GuestCtl0 |= (1 << CP0GuestCtl0_GM);
                env->CP0_GuestCtl0 |= (GPA << CP0GuestCtl0_GExcCode);
#ifdef SV_SUPPORT
                sv_log("Guest: Root.TLB miss: updating guestctl0 = %08x\n", env->CP0_GuestCtl0);
#endif
            }
            else {
                env->CP0_GuestCtl0 |= (GVA << CP0GuestCtl0_GExcCode);
#ifdef SV_SUPPORT
                sv_log("Guest: Guest.TLB miss: updating guestctl0 = %08x\n", env->CP0_GuestCtl0);
#endif
            }
        }
#ifdef SV_SUPPORT
        else {
            // physical contains error address
            sv_log("Root: Root.TLB miss - calling raise_mmu_exception\n");
        }
#endif
        raise_mmu_exception(env, physical, rw, ret);
        ret = 1;
    }
#else
    {
        raise_mmu_exception(env, address, rw, ret);
        ret = 1;
    }
#endif

    return ret;
}

#if !defined(CONFIG_USER_ONLY)
target_phys_addr_t cpu_mips_translate_address(CPUState *env, target_ulong address, int rw)
{
    target_phys_addr_t physical;
    int prot;
    int access_type;
    int ret = 0;

    /* data access */
    access_type = ACCESS_INT;
    ret = get_physical_address(env, &physical, &prot,
                               address, rw, access_type);
    if (ret != TLBRET_MATCH) {
        raise_mmu_exception(env, address, rw, ret);
        return -1LL;
    } else {
        return physical;
    }
}
#endif

static const char * const excp_names[EXCP_LAST + 1] = {
    [EXCP_RESET] = "reset",
    [EXCP_SRESET] = "soft reset",
    [EXCP_DSS] = "debug single step",
    [EXCP_DINT] = "debug interrupt",
    [EXCP_NMI] = "non-maskable interrupt",
    [EXCP_MCHECK] = "machine check",
    [EXCP_EXT_INTERRUPT] = "interrupt",
    [EXCP_DFWATCH] = "deferred watchpoint",
    [EXCP_DIB] = "debug instruction breakpoint",
    [EXCP_IWATCH] = "instruction fetch watchpoint",
    [EXCP_AdEL] = "address error load",
    [EXCP_AdES] = "address error store",
    [EXCP_TLBF] = "TLB refill",
    [EXCP_IBE] = "instruction bus error",
    [EXCP_DBp] = "debug breakpoint",
    [EXCP_SYSCALL] = "syscall",
    [EXCP_BREAK] = "break",
    [EXCP_CpU] = "coprocessor unusable",
    [EXCP_RI] = "reserved instruction",
    [EXCP_OVERFLOW] = "arithmetic overflow",
    [EXCP_TRAP] = "trap",
    [EXCP_FPE] = "floating point",
    [EXCP_DDBS] = "debug data break store",
    [EXCP_DWATCH] = "data watchpoint",
    [EXCP_LTLBL] = "TLB modify",
    [EXCP_TLBL] = "TLB load",
    [EXCP_TLBS] = "TLB store",
    [EXCP_DBE] = "data bus error",
    [EXCP_DDBL] = "debug data break load",
    [EXCP_THREAD] = "thread",
    [EXCP_MDMX] = "MDMX",
    [EXCP_C2E] = "precise coprocessor 2",
    [EXCP_CACHE] = "cache error",
    [EXCP_MSADIS] = "MSA disabled",
    [EXCP_MSAFPE] = "MSA floating point",
};

#if !defined(CONFIG_USER_ONLY)
static target_ulong exception_resume_pc (CPUState *env)
{
    target_ulong bad_pc;
    target_ulong isa_mode;

    isa_mode = !!(env->hflags & MIPS_HFLAG_M16);
    bad_pc = env->active_tc.PC | isa_mode;
    if (env->hflags & MIPS_HFLAG_BMASK) {
        /* If the exception was raised from a delay slot, come back to
           the jump.  */
        bad_pc -= (env->hflags & MIPS_HFLAG_B16 ? 2 : 4);
    }

    return bad_pc;
}

static void set_hflags_for_handler (CPUState *env)
{
    /* Exception handlers are entered in 32-bit mode.  */
    env->hflags &= ~(MIPS_HFLAG_M16);
    /* ...except that microMIPS lets you choose.  */
    if (env->insn_flags & ASE_MICROMIPS) {
        env->hflags |= (!!(env->CP0_Config3
                           & (1 << CP0C3_ISA_ON_EXC))
                        << MIPS_HFLAG_M16_SHIFT);
    }
}
#endif

void do_interrupt (CPUState *env)
{
#if !defined(CONFIG_USER_ONLY)
    target_ulong offset;
    int cause = -1;
    const char *name;
    int * CP0_Status;
#ifdef SV_SUPPORT
#if defined(TARGET_MIPS64)
    sv_log("Info (MIPS64_EXCEPT) %s: %s - %" PRIx64,
            env->cpu_model_str,
            (env->hflags & MIPS_HFLAG_GUEST)? "Guest": "Root",
            env->active_tc.PC);
#else
    sv_log("Info (MIPS32_EXCEPT) %s: %s - %x" ,
            env->cpu_model_str,
            (env->hflags & MIPS_HFLAG_GUEST)? "Guest": "Root",
            env->active_tc.PC);
#endif
#endif

    if (env->exitGuest) {
        env->hflags &= ~MIPS_HFLAG_GUEST;
        env->exitGuest = 0;
        tlb_flush (env, 1);
        cpu_mips_silence_irq_guest(env);
    }

    //take exception as root by now
    //FIXME: VZ
    if( env->exception_index == EXCP_GUESTEXIT )
    {
        env->hflags &= ~MIPS_HFLAG_GUEST;
        tlb_flush (env, 1);
        cpu_mips_silence_irq_guest(env);
    }

    if (qemu_log_enabled() && env->exception_index != EXCP_EXT_INTERRUPT) {
        if (env->exception_index < 0 || env->exception_index > EXCP_LAST)
            name = "unknown";
        else
            name = excp_names[env->exception_index];

        qemu_log("%s enter: PC " TARGET_FMT_lx " EPC " TARGET_FMT_lx " %s exception\n",
                 __func__, env->active_tc.PC, env->CP0_EPC, name);
    }
    if (env->exception_index == EXCP_EXT_INTERRUPT &&
        (env->hflags & MIPS_HFLAG_DM))
        env->exception_index = EXCP_DINT;
    offset = 0x180;
    switch (env->exception_index) {
    case EXCP_DSS:
        env->CP0_Debug |= 1 << CP0DB_DSS;
        /* Debug single step cannot be raised inside a delay slot and
           resume will always occur on the next instruction
           (but we assume the pc has always been updated during
           code translation). */
        env->CP0_DEPC = env->active_tc.PC | !!(env->hflags & MIPS_HFLAG_M16);
        goto enter_debug_mode;
    case EXCP_DINT:
        env->CP0_Debug |= 1 << CP0DB_DINT;
        goto set_DEPC;
    case EXCP_DIB:
        env->CP0_Debug |= 1 << CP0DB_DIB;
        goto set_DEPC;
    case EXCP_DBp:
        env->CP0_Debug |= 1 << CP0DB_DBp;
        goto set_DEPC;
    case EXCP_DDBS:
        env->CP0_Debug |= 1 << CP0DB_DDBS;
        goto set_DEPC;
    case EXCP_DDBL:
        env->CP0_Debug |= 1 << CP0DB_DDBL;
    set_DEPC:
        env->CP0_DEPC = exception_resume_pc(env);
        env->hflags &= ~MIPS_HFLAG_BMASK;
 enter_debug_mode:
        env->hflags |= MIPS_HFLAG_DM | MIPS_HFLAG_64 | MIPS_HFLAG_CP0;
        env->hflags &= ~(MIPS_HFLAG_KSU);
        /* EJTAG probe trap enable is not implemented... */
        if (!(env->CP0_Status & (1 << CP0St_EXL)))
            env->CP0_Cause &= ~(1 << CP0Ca_BD);
        env->active_tc.PC = (int32_t)0xBFC00480;
        set_hflags_for_handler(env);
        break;
    case EXCP_RESET:
        cpu_reset(env);
        break;
    case EXCP_SRESET:
        env->CP0_Status |= (1 << CP0St_SR);
        memset(env->CP0_WatchLo, 0, sizeof(*env->CP0_WatchLo));
        goto set_error_EPC;
    case EXCP_NMI:
        env->CP0_Status |= (1 << CP0St_NMI);
 set_error_EPC:
        env->CP0_ErrorEPC = exception_resume_pc(env);
        env->hflags &= ~MIPS_HFLAG_BMASK;
        env->CP0_Status |= (1 << CP0St_ERL) | (1 << CP0St_BEV);
        env->hflags |= MIPS_HFLAG_64 | MIPS_HFLAG_CP0;
        env->hflags &= ~(MIPS_HFLAG_KSU);
        if (!(env->CP0_Status & (1 << CP0St_EXL)))
            env->CP0_Cause &= ~(1 << CP0Ca_BD);
        env->active_tc.PC = (int32_t)0xBFC00000;
        set_hflags_for_handler(env);
        break;
    case EXCP_EXT_INTERRUPT:
        cause = 0;
        if (env->hflags & MIPS_HFLAG_GUEST) {
            if (env->Guest.CP0_Cause & (1 << CP0Ca_IV))
            {
                offset = 0x200;

                if (env->Guest.CP0_Config3 & ((1 << CP0C3_VInt) | (1 << CP0C3_VEIC))) {
#ifdef SV_SUPPORT
                    sv_log("config3 = %x\n", env->Guest.CP0_Config3);
#endif
                    /* Vectored Interrupts.  */
                    unsigned int spacing;
                    unsigned int vector;
                    unsigned int pending = (env->Guest.CP0_Cause & CP0Ca_IP_mask) >> 8;

                    pending &= env->Guest.CP0_Status >> 8;
                    /* Compute the Vector Spacing.  */
                    spacing = (env->Guest.CP0_IntCtl >> CP0IntCtl_VS) & ((1 << 6) - 1);
                    spacing <<= 5;

                    if (env->Guest.CP0_Config3 & (1 << CP0C3_VInt)) {
                        /* For VInt mode, the MIPS computes the vector internally.  */
                        for (vector = 7; vector > 0; vector--) {
                            if (pending & (1 << vector)) {
                                /* Found it.  */
                                break;
                            }
                        }
                    } else {
                        /* For VEIC mode, the external interrupt controller feeds the
                           vector throught the CP0Cause IP lines.  */
                        vector = pending;
                    }
                    offset = 0x200 + vector * spacing;
                }
            }
        }
        else {
            if (env->CP0_Cause & (1 << CP0Ca_IV))
            {
                offset = 0x200;

                if (env->CP0_Config3 & ((1 << CP0C3_VInt) | (1 << CP0C3_VEIC))) {
#ifdef SV_SUPPORT
                    sv_log("config3 = %x\n", env->CP0_Config3);
#endif
                    /* Vectored Interrupts.  */
                    unsigned int spacing;
                    unsigned int vector;
                    unsigned int pending = (env->CP0_Cause & CP0Ca_IP_mask) >> 8;

                    pending &= env->CP0_Status >> 8;
                    /* Compute the Vector Spacing.  */
                    spacing = (env->CP0_IntCtl >> CP0IntCtl_VS) & ((1 << 6) - 1);
                    spacing <<= 5;

                    if (env->CP0_Config3 & (1 << CP0C3_VInt)) {
                        /* For VInt mode, the MIPS computes the vector internally.  */
                        for (vector = 7; vector > 0; vector--) {
                            if (pending & (1 << vector)) {
                                /* Found it.  */
                                break;
                            }
                        }
                    } else {
                        /* For VEIC mode, the external interrupt controller feeds the
                           vector throught the CP0Cause IP lines.  */
                        vector = pending;
                    }
                    offset = 0x200 + vector * spacing;
                }
            }
        }
        goto set_EPC;
    case EXCP_LTLBL:
        cause = 1;
        goto set_EPC;
    case EXCP_TLBL:
        cause = 2;
        if (env->hflags & MIPS_HFLAG_GUEST) {
            CP0_Status = &env->Guest.CP0_Status;
        } else {
            CP0_Status = &env->CP0_Status;
        }
        if (env->error_code == 1 && !(*CP0_Status & (1 << CP0St_EXL))) {
#if defined(TARGET_MIPS64)
            // TODO: VZ
            int R = env->CP0_BadVAddr >> 62;
            int UX = (*CP0_Status & (1 << CP0St_UX)) != 0;
            int SX = (*CP0_Status & (1 << CP0St_SX)) != 0;
            int KX = (*CP0_Status & (1 << CP0St_KX)) != 0;

            if (((R == 0 && UX) || (R == 1 && SX) || (R == 3 && KX)) &&
                (!(env->insn_flags & (INSN_LOONGSON2E | INSN_LOONGSON2F))))
                offset = 0x080;
            else
#endif
                offset = 0x000;
        }
        goto set_EPC;
    case EXCP_TLBS:
        cause = 3;
        if (env->hflags & MIPS_HFLAG_GUEST) {
            CP0_Status = &env->Guest.CP0_Status;
        } else {
            CP0_Status = &env->CP0_Status;
        }
        if (env->error_code == 1 && !(*CP0_Status & (1 << CP0St_EXL))) {
#if defined(TARGET_MIPS64)
            // TODO: VZ
            int R = env->CP0_BadVAddr >> 62;
            int UX = (*CP0_Status & (1 << CP0St_UX)) != 0;
            int SX = (*CP0_Status & (1 << CP0St_SX)) != 0;
            int KX = (*CP0_Status & (1 << CP0St_KX)) != 0;

            if (((R == 0 && UX) || (R == 1 && SX) || (R == 3 && KX)) &&
                (!(env->insn_flags & (INSN_LOONGSON2E | INSN_LOONGSON2F))))
                offset = 0x080;
            else
#endif
                offset = 0x000;
        }
        goto set_EPC;
    case EXCP_AdEL:
        cause = 4;
        goto set_EPC;
    case EXCP_AdES:
        cause = 5;
        goto set_EPC;
    case EXCP_IBE:
        cause = 6;
        goto set_EPC;
    case EXCP_DBE:
        cause = 7;
        goto set_EPC;
    case EXCP_SYSCALL:
        cause = 8;
        goto set_EPC;
    case EXCP_BREAK:
        cause = 9;
        goto set_EPC;
    case EXCP_RI:
        cause = 10;
        goto set_EPC;
    case EXCP_CpU:
        cause = 11;
        // VZ-ASE Onion model
        if (env->error_code >= 0x10) {
            // Cp is enabled in Guest mode but disabled in Root mode
            // Take it as root exception
            env->hflags &= ~MIPS_HFLAG_GUEST;
            env->error_code &= ~0x10;
            tlb_flush (env, 1);
            cpu_mips_silence_irq_guest(env);
        }
        if (env->hflags & MIPS_HFLAG_GUEST) {
            env->Guest.CP0_Cause = (env->Guest.CP0_Cause & ~(0x3 << CP0Ca_CE)) |
                             (env->error_code << CP0Ca_CE);
        }
        else {
            env->CP0_Cause = (env->CP0_Cause & ~(0x3 << CP0Ca_CE)) |
                             (env->error_code << CP0Ca_CE);
        }
        goto set_EPC;
    case EXCP_OVERFLOW:
        cause = 12;
        goto set_EPC;
    case EXCP_TRAP:
        cause = 13;
        goto set_EPC;
    case EXCP_MSAFPE:
        cause = 14;
        goto set_EPC;
    case EXCP_FPE:
        cause = 15;
        goto set_EPC;
    case EXCP_C2E:
        cause = 18;
        goto set_EPC;
    case EXCP_MSADIS:
        cause = 21;
        // VZ-ASE Onion model
        if (env->error_code >= 0x10) {
            // MSA is enabled in Guest mode but disabled in Root mode
            // Take it as root exception
            env->hflags &= ~MIPS_HFLAG_GUEST;
            tlb_flush (env, 1);
            cpu_mips_silence_irq_guest(env);
        }
        goto set_EPC;
    case EXCP_MDMX:
        cause = 22;
        goto set_EPC;
    case EXCP_DWATCH:
        cause = 23;
        /* XXX: TODO: manage defered watch exceptions */
        goto set_EPC;
    case EXCP_MCHECK:
        cause = 24;
        goto set_EPC;
    case EXCP_THREAD:
        cause = 25;
        goto set_EPC;
    case EXCP_DSPDIS:
        // VZ-ASE Onion model
        if (env->error_code >= 0x10) {
            // DSP is enabled in Guest mode but disabled in Root mode
            // Take it as root exception
            env->hflags &= ~MIPS_HFLAG_GUEST;
            tlb_flush (env, 1);
            cpu_mips_silence_irq_guest(env);
        }
        cause = 26;
        goto set_EPC;
    case EXCP_CACHE:
        cause = 30;
        if (env->CP0_Status & (1 << CP0St_BEV)) {
            offset = 0x100;
        } else {
            offset = 0x20000100;
        }
 set_EPC:
        if (env->hflags & MIPS_HFLAG_GUEST) {
            int32_t old = env->Guest.CP0_Status;
            if (!(env->Guest.CP0_Status & (1 << CP0St_EXL))) {
                env->Guest.CP0_EPC = exception_resume_pc(env);
                if (env->hflags & MIPS_HFLAG_BMASK) {
                    env->Guest.CP0_Cause |= (1 << CP0Ca_BD);
                } else {
                    env->Guest.CP0_Cause &= ~(1 << CP0Ca_BD);
                }
                env->Guest.CP0_Status |= (1 << CP0St_EXL);
                env->hflags |= MIPS_HFLAG_64 | MIPS_HFLAG_CP0;
                env->hflags &= ~(MIPS_HFLAG_KSU);
            }
            env->hflags &= ~MIPS_HFLAG_BMASK;
            if (env->Guest.CP0_Status & (1 << CP0St_BEV)) {
                env->active_tc.PC = (int32_t)0xBFC00200;
            } else {
                env->active_tc.PC = (int32_t)(env->Guest.CP0_EBase & ~0x3ff);
            }
            env->active_tc.PC += offset;
            set_hflags_for_handler(env);
            env->Guest.CP0_Cause = (env->Guest.CP0_Cause & ~(0x1f << CP0Ca_EC)) | (cause << CP0Ca_EC);
#ifdef SV_SUPPORT
            sv_log(": exception #%d at offset 0x%x\n", cause, (unsigned) offset);
#endif

            /* When GuestCtl0ExtFCD=1,
             * then no Guest Hardware Field Change exception is triggered.*/
            if ( !(env->CP0_GuestCtl0Ext & (1 << CP0GuestCtl0Ext_FCD)) )
            {
#define CHK_CHANGES(POS, MASK) (((old >> (POS)) & (MASK)) != ((env->Guest.CP0_Status >> (POS)) & (MASK)))
                /* . Guest Status bits: EXL when GuestCtl0MC=1
                 * . Guest Status bits: TS (set)
                 */
                if ( (CHK_CHANGES(CP0St_EXL, 1) && (env->CP0_GuestCtl0 & (1 << CP0GuestCtl0_MC))) ||
                        (CHK_CHANGES(CP0St_TS, 1) && (env->Guest.CP0_Status & (1 << CP0St_TS))) ) {
#ifdef SV_SUPPORT
                    sv_log("GHFC root mode exception\n");
#endif
                    env->exception_index = EXCP_GUESTEXIT;
                    env->error_code = GHFC;
                    cpu_loop_exit(env);
                    return;
                }
            }
        }
        else {
            if (!(env->CP0_Status & (1 << CP0St_EXL))) {
                env->CP0_EPC = exception_resume_pc(env);
                if (env->hflags & MIPS_HFLAG_BMASK) {
                    env->CP0_Cause |= (1 << CP0Ca_BD);
                } else {
                    env->CP0_Cause &= ~(1 << CP0Ca_BD);
                }
                env->CP0_Status |= (1 << CP0St_EXL);
                env->hflags |= MIPS_HFLAG_64 | MIPS_HFLAG_CP0;
                env->hflags &= ~(MIPS_HFLAG_KSU);
            }
            env->hflags &= ~MIPS_HFLAG_BMASK;
            if (env->CP0_Status & (1 << CP0St_BEV)) {
                env->active_tc.PC = (int32_t)0xBFC00200;
            } else {
                env->active_tc.PC = (int32_t)(env->CP0_EBase & ~0x3ff);
            }
            env->active_tc.PC += offset;
            set_hflags_for_handler(env);
            env->CP0_Cause = (env->CP0_Cause & ~(0x1f << CP0Ca_EC)) | (cause << CP0Ca_EC);
        }
        break;
    case EXCP_GUESTEXIT:
        cause = 27;
        env->CP0_GuestCtl0 &= ~(0x1F << CP0GuestCtl0_GExcCode);
        env->CP0_GuestCtl0 |= (env->error_code << CP0GuestCtl0_GExcCode);
        goto set_EPC;
        break;
    default:
        qemu_log("Invalid MIPS exception %d. Exiting\n", env->exception_index);
        printf("Invalid MIPS exception %d. Exiting\n", env->exception_index);
        exit(1);
    }
    if (qemu_log_enabled() && env->exception_index != EXCP_EXT_INTERRUPT) {
        qemu_log("%s: PC " TARGET_FMT_lx " EPC " TARGET_FMT_lx " cause %d\n"
                "    S %08x C %08x A %"PRIx64" D " TARGET_FMT_lx "\n",
                __func__, env->active_tc.PC, env->CP0_EPC, cause,
                env->CP0_Status, env->CP0_Cause, env->CP0_BadVAddr,
                env->CP0_DEPC);
    }
#ifdef SV_SUPPORT
    sv_log(": exception #%d at offset 0x%x\n", cause, (unsigned) offset);
    if (sv_enabled()) {
        trace_cpu_state(env, 0);
    }
#endif
#endif
    env->exception_index = EXCP_NONE;
}

#if !defined(CONFIG_USER_ONLY)
void r4k_invalidate_tlb (CPUState *env, int idx, int use_extra)
{
    r4k_tlb_t *tlb;
    target_ulong addr;
    target_ulong end;
    uint8_t ASID = env->CP0_EntryHi & 0xFF;
    target_ulong mask;

    tlb = &env->tlb->mmu.r4k.tlb[idx];
    /* The qemu TLB is flushed when the ASID changes, so no need to
       flush these entries again.  */
    if (tlb->G == 0 && tlb->ASID != ASID) {
        return;
    }

    if ((env->CP0_Config3 & (1 << CP0C3_VZ)) && !tlb->isGuestCtx) {
        /* Flush qemu tlb when overwriting r4k root tlb entry.
           TODO: this should be safe but probably not optimal.
        */
        tlb_flush(env, 1);
        return;
    }

    if (use_extra && env->tlb->tlb_in_use < MIPS_TLB_MAX) {
        /* For tlbwr, we can shadow the discarded entry into
           a new (fake) TLB entry, as long as the guest can not
           tell that it's there.  */
        env->tlb->mmu.r4k.tlb[env->tlb->tlb_in_use] = *tlb;
        env->tlb->tlb_in_use++;
        return;
    }

    /* 1k pages are not supported. */
    mask = tlb->PageMask | ~(TARGET_PAGE_MASK << 1);
    if (tlb->V0) {
        addr = tlb->VPN & ~mask;
#if defined(TARGET_MIPS64)
        if (addr >= (0xFFFFFFFF80000000ULL & env->SEGMask)) {
            addr |= 0x3FFFFF0000000000ULL;
        }
#endif
        end = addr | (mask >> 1);
        while (addr < end) {
            tlb_flush_page (env, addr);
            addr += TARGET_PAGE_SIZE;
        }
    }
    if (tlb->V1) {
        addr = (tlb->VPN & ~mask) | ((mask >> 1) + 1);
#if defined(TARGET_MIPS64)
        if (addr >= (0xFFFFFFFF80000000ULL & env->SEGMask)) {
            addr |= 0x3FFFFF0000000000ULL;
        }
#endif
        end = addr | mask;
        while (addr - 1 < end) {
            tlb_flush_page (env, addr);
            addr += TARGET_PAGE_SIZE;
        }
    }
}
#endif
