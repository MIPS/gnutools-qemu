/*
 *  Common CPU TLB handling
 *
 *  Copyright (c) 2003 Fabrice Bellard
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

#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"
#include "exec/cpu_ldst.h"
#include "exec/cputlb.h"
#include "exec/memory-internal.h"
#include "exec/ram_addr.h"
#include "exec/exec-all.h"
#include "tcg/tcg.h"
#include "qemu/error-report.h"
#include "exec/log.h"
#include "exec/helper-proto.h"

/* DEBUG defines, enable DEBUG_TLB_LOG to log to the CPU_LOG_MMU target */
/* #define DEBUG_TLB */
/* #define DEBUG_TLB_LOG */

#ifdef DEBUG_TLB
# define DEBUG_TLB_GATE 1
# ifdef DEBUG_TLB_LOG
#  define DEBUG_TLB_LOG_GATE 1
# else
#  define DEBUG_TLB_LOG_GATE 0
# endif
#else
# define DEBUG_TLB_GATE 0
# define DEBUG_TLB_LOG_GATE 0
#endif

#define tlb_debug(fmt, ...) do { \
    if (DEBUG_TLB_LOG_GATE) { \
        qemu_log_mask(CPU_LOG_MMU, "%s: " fmt, __func__, \
                      ## __VA_ARGS__); \
    } else if (DEBUG_TLB_GATE) { \
        fprintf(stderr, "%s: " fmt, __func__, ## __VA_ARGS__); \
    } \
} while (0)

#define assert_cpu_is_self(this_cpu) do {                         \
        if (DEBUG_TLB_GATE) {                                     \
            g_assert(!cpu->created || qemu_cpu_is_self(cpu));     \
        }                                                         \
    } while (0)

/* We need a solution for stuffing 64 bit pointers in 32 bit ones if
 * we care about this combination */
QEMU_BUILD_BUG_ON(sizeof(target_ulong) > sizeof(void *));
QEMU_BUILD_BUG_ON(NB_MMU_MODES > 16);

#define ALL_MMUIDX_BITS ((1 << NB_MMU_MODES) - 1)

/* statistics */
int tlb_flush_count;

static void tlb_flush_nocheck(CPUState *cpu, int flush_global)
{
    CPUArchState *env = cpu->env_ptr;

    assert_cpu_is_self(cpu);
    tlb_debug("(%d)\n", flush_global);

    memset(env->tlb_table, -1, sizeof(env->tlb_table));
    memset(env->tlb_v_table, -1, sizeof(env->tlb_v_table));
    memset(cpu->tb_jmp_cache, 0, sizeof(cpu->tb_jmp_cache));

    env->vtlb_index = 0;
    env->tlb_flush_addr = -1;
    env->tlb_flush_mask = 0;
    tlb_flush_count++;

    atomic_mb_set(&cpu->pending_tlb_flush, 0);
}

static void tlb_flush_global_async_work(CPUState *cpu, void *opaque)
{
    tlb_flush_nocheck(cpu, GPOINTER_TO_INT(opaque));
}

/* NOTE:
 * If flush_global is true (the usual case), flush all tlb entries.
 * If flush_global is false, flush (at least) all tlb entries not
 * marked global.
 *
 * Since QEMU doesn't currently implement a global/not-global flag
 * for tlb entries, at the moment tlb_flush() will also flush all
 * tlb entries in the flush_global == false case. This is OK because
 * CPU architectures generally permit an implementation to drop
 * entries from the TLB at any time, so flushing more entries than
 * required is only an efficiency issue, not a correctness issue.
 */
void tlb_flush(CPUState *cpu, int flush_global)
{
    if (cpu->created && !qemu_cpu_is_self(cpu)) {
        if (atomic_mb_read(&cpu->pending_tlb_flush) != ALL_MMUIDX_BITS) {
            atomic_mb_set(&cpu->pending_tlb_flush, ALL_MMUIDX_BITS);
            async_run_on_cpu(cpu, tlb_flush_global_async_work,
                             GINT_TO_POINTER(flush_global));
        }
    } else {
        tlb_flush_nocheck(cpu, flush_global);
    }
}

static void tlb_flush_by_mmuidx_async_work(CPUState *cpu, void *mmu_bitmask)
{
    CPUArchState *env = cpu->env_ptr;
    unsigned long mmu_idx_bitmask = GPOINTER_TO_UINT(mmu_bitmask);
    int mmu_idx;

    assert_cpu_is_self(cpu);

    tlb_debug("start\n");

    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {

        if (test_bit(mmu_idx, &mmu_idx_bitmask)) {
            tlb_debug("%d\n", mmu_idx);

            memset(env->tlb_table[mmu_idx], -1, sizeof(env->tlb_table[0]));
            memset(env->tlb_v_table[mmu_idx], -1, sizeof(env->tlb_v_table[0]));
        }
    }

    memset(cpu->tb_jmp_cache, 0, sizeof(cpu->tb_jmp_cache));
}

/* Helper function to slurp va_args list into a bitmap
 */
static inline unsigned long make_mmu_index_bitmap(va_list args)
{
    unsigned long bitmap = 0;
    int mmu_index = va_arg(args, int);

    /* An empty va_list would be a bad call */
    g_assert(mmu_index > 0);

    do {
        set_bit(mmu_index, &bitmap);
        mmu_index = va_arg(args, int);
    } while (mmu_index >= 0);

    return bitmap;
}

void tlb_flush_by_mmuidx(CPUState *cpu, ...)
{
    va_list argp;
    unsigned long mmu_idx_bitmap;

    va_start(argp, cpu);
    mmu_idx_bitmap = make_mmu_index_bitmap(argp);
    va_end(argp);

    if (!qemu_cpu_is_self(cpu)) {
        uint16_t pending_flushes =
            mmu_idx_bitmap & ~atomic_mb_read(&cpu->pending_tlb_flush);
        if (pending_flushes) {
            atomic_or(&cpu->pending_tlb_flush, pending_flushes);
            async_run_on_cpu(cpu, tlb_flush_by_mmuidx_async_work,
                             GUINT_TO_POINTER(pending_flushes));
        }
    } else {
        tlb_flush_by_mmuidx_async_work(cpu, GUINT_TO_POINTER(mmu_idx_bitmap));
    }
}

static inline void tlb_flush_entry(CPUTLBEntry *tlb_entry, target_ulong addr)
{
    if (addr == (tlb_entry->addr_read &
                 (TARGET_PAGE_MASK | TLB_INVALID_MASK)) ||
        addr == (tlb_entry->addr_write &
                 (TARGET_PAGE_MASK | TLB_INVALID_MASK)) ||
        addr == (tlb_entry->addr_code &
                 (TARGET_PAGE_MASK | TLB_INVALID_MASK))) {
        memset(tlb_entry, -1, sizeof(*tlb_entry));
    }
}

void tlb_flush_page(CPUState *cpu, target_ulong addr)
{
    CPUArchState *env = cpu->env_ptr;
    int i;
    int mmu_idx;

    assert_cpu_is_self(cpu);
    tlb_debug("page :" TARGET_FMT_lx "\n", addr);

    /* Check if we need to flush due to large pages.  */
    if ((addr & env->tlb_flush_mask) == env->tlb_flush_addr) {
        tlb_debug("forcing full flush ("
                  TARGET_FMT_lx "/" TARGET_FMT_lx ")\n",
                  env->tlb_flush_addr, env->tlb_flush_mask);

        tlb_flush(cpu, 1);
        return;
    }

    addr &= TARGET_PAGE_MASK;
    i = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        tlb_flush_entry(&env->tlb_table[mmu_idx][i], addr);
    }

    /* check whether there are entries that need to be flushed in the vtlb */
    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        int k;
        for (k = 0; k < CPU_VTLB_SIZE; k++) {
            tlb_flush_entry(&env->tlb_v_table[mmu_idx][k], addr);
        }
    }

    tb_flush_jmp_cache(cpu, addr);
}

/* As we are going to hijack the bottom bits of the page address for a
 * mmuidx bit mask we need to fail to build if we can't do that
 */
QEMU_BUILD_BUG_ON(NB_MMU_MODES > TARGET_PAGE_BITS);

static void tlb_flush_page_by_mmuidx_async_work(CPUState *cpu, void *data)
{
    CPUArchState *env = cpu->env_ptr;
    uintptr_t page_and_mmuidx = GPOINTER_TO_UINT(data);
    target_ulong addr = page_and_mmuidx & TARGET_PAGE_MASK;
    int page = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
    int mmu_idx;
    int i;

    assert_cpu_is_self(cpu);

    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        if (test_bit(mmu_idx, &page_and_mmuidx)) {
            tlb_flush_entry(&env->tlb_table[mmu_idx][page], addr);

            /* check whether there are vltb entries that need to be flushed */
            for (i = 0; i < CPU_VTLB_SIZE; i++) {
                tlb_flush_entry(&env->tlb_v_table[mmu_idx][i], addr);
            }
        }
    }

    tb_flush_jmp_cache(cpu, addr);
}

void tlb_flush_page_by_mmuidx(CPUState *cpu, target_ulong addr, ...)
{
    CPUArchState *env = cpu->env_ptr;
    unsigned long mmu_idx_bitmap;
    va_list argp;

    va_start(argp, addr);
    mmu_idx_bitmap = make_mmu_index_bitmap(argp);
    va_end(argp);

    tlb_debug("addr "TARGET_FMT_lx"\n", addr);

    /* Check if we need to flush due to large pages.  */
    if ((addr & env->tlb_flush_mask) == env->tlb_flush_addr) {
        tlb_debug("forced full flush ("
                  TARGET_FMT_lx "/" TARGET_FMT_lx ")\n",
                  env->tlb_flush_addr, env->tlb_flush_mask);


        if (!qemu_cpu_is_self(cpu)) {
            uint16_t pending_flushes =
                mmu_idx_bitmap & ~atomic_mb_read(&cpu->pending_tlb_flush);
            if (pending_flushes) {
                atomic_or(&cpu->pending_tlb_flush, pending_flushes);
                async_run_on_cpu(cpu, tlb_flush_by_mmuidx_async_work,
                                 GUINT_TO_POINTER(pending_flushes));
            }
        } else {
            tlb_flush_by_mmuidx_async_work(cpu,
                                           GUINT_TO_POINTER(mmu_idx_bitmap));
        }
        return;
    }

    /* This should already be page aligned */
    addr &= TARGET_PAGE_MASK;
    addr |= mmu_idx_bitmap;

    if (!qemu_cpu_is_self(cpu)) {
        async_run_on_cpu(cpu, tlb_flush_page_by_mmuidx_async_work,
                         GUINT_TO_POINTER(addr));
    } else {
        tlb_flush_page_by_mmuidx_async_work(cpu, GUINT_TO_POINTER(addr));
    }
}

static void tlb_flush_page_async_work(CPUState *cpu, void *opaque)
{
    tlb_flush_page(cpu, GPOINTER_TO_UINT(opaque));
}

void tlb_flush_page_all(target_ulong addr)
{
    CPUState *cpu;

    CPU_FOREACH(cpu) {
        async_run_on_cpu(cpu, tlb_flush_page_async_work,
                         GUINT_TO_POINTER(addr));
    }
}

/* update the TLBs so that writes to code in the virtual page 'addr'
   can be detected */
void tlb_protect_code(ram_addr_t ram_addr)
{
    cpu_physical_memory_test_and_clear_dirty(ram_addr, TARGET_PAGE_SIZE,
                                             DIRTY_MEMORY_CODE);
}

/* update the TLB so that writes in physical page 'phys_addr' are no longer
   tested for self modifying code */
void tlb_unprotect_code(ram_addr_t ram_addr)
{
    cpu_physical_memory_set_dirty_flag(ram_addr, DIRTY_MEMORY_CODE);
}


/*
 * Dirty write flag handling
 *
 * When the TCG code writes to a location it looks up the address in
 * the TLB and uses that data to compute the final address. If any of
 * the lower bits of the address are set then the slow path is forced.
 * There are a number of reasons to do this but for normal RAM the
 * most usual is detecting writes to code regions which may invalidate
 * generated code.
 *
 * Because we want other vCPUs to respond to changes straight away we
 * update the te->addr_write field atomically. If the TLB entry has
 * been changed by the vCPU in the mean time we skip the update.
 */

static void tlb_reset_dirty_range(CPUTLBEntry *tlb_entry, uintptr_t start,
                           uintptr_t length)
{
    /* paired with atomic_mb_set in tlb_set_page_with_attrs */
    uintptr_t orig_addr = atomic_mb_read(&tlb_entry->addr_write);
    uintptr_t addr = orig_addr;

    if ((addr & (TLB_INVALID_MASK | TLB_MMIO | TLB_NOTDIRTY)) == 0) {
        addr &= TARGET_PAGE_MASK;
        addr += atomic_read(&tlb_entry->addend);
        if ((addr - start) < length) {
            uintptr_t notdirty_addr = orig_addr | TLB_NOTDIRTY;
            atomic_cmpxchg(&tlb_entry->addr_write, orig_addr, notdirty_addr);
        }
    }
}

/* This is a cross vCPU call (i.e. another vCPU resetting the flags of
 * the target vCPU). As such care needs to be taken that we don't
 * dangerously race with another vCPU update. The only thing actually
 * updated is the target TLB entry ->addr_write flags.
 */
void tlb_reset_dirty(CPUState *cpu, ram_addr_t start1, ram_addr_t length)
{
    CPUArchState *env;

    int mmu_idx;

    env = cpu->env_ptr;
    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        unsigned int i;

        for (i = 0; i < CPU_TLB_SIZE; i++) {
            tlb_reset_dirty_range(&env->tlb_table[mmu_idx][i],
                                  start1, length);
        }

        for (i = 0; i < CPU_VTLB_SIZE; i++) {
            tlb_reset_dirty_range(&env->tlb_v_table[mmu_idx][i],
                                  start1, length);
        }
    }
}

static inline void tlb_set_dirty1(CPUTLBEntry *tlb_entry, target_ulong vaddr)
{
    if (tlb_entry->addr_write == (vaddr | TLB_NOTDIRTY)) {
        tlb_entry->addr_write = vaddr;
    }
}

/* update the TLB corresponding to virtual page vaddr
   so that it is no longer dirty */
void tlb_set_dirty(CPUState *cpu, target_ulong vaddr)
{
    CPUArchState *env = cpu->env_ptr;
    int i;
    int mmu_idx;

    assert_cpu_is_self(cpu);

    vaddr &= TARGET_PAGE_MASK;
    i = (vaddr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        tlb_set_dirty1(&env->tlb_table[mmu_idx][i], vaddr);
    }

    for (mmu_idx = 0; mmu_idx < NB_MMU_MODES; mmu_idx++) {
        int k;
        for (k = 0; k < CPU_VTLB_SIZE; k++) {
            tlb_set_dirty1(&env->tlb_v_table[mmu_idx][k], vaddr);
        }
    }
}

/* Our TLB does not support large pages, so remember the area covered by
   large pages and trigger a full TLB flush if these are invalidated.  */
static void tlb_add_large_page(CPUArchState *env, target_ulong vaddr,
                               target_ulong size)
{
    target_ulong mask = ~(size - 1);

    if (env->tlb_flush_addr == (target_ulong)-1) {
        env->tlb_flush_addr = vaddr & mask;
        env->tlb_flush_mask = mask;
        return;
    }
    /* Extend the existing region to include the new page.
       This is a compromise between unnecessary flushes and the cost
       of maintaining a full variable size TLB.  */
    mask &= env->tlb_flush_mask;
    while (((env->tlb_flush_addr ^ vaddr) & mask) != 0) {
        mask <<= 1;
    }
    env->tlb_flush_addr &= mask;
    env->tlb_flush_mask = mask;
}

/* Add a new TLB entry. At most one entry for a given virtual address
 * is permitted. Only a single TARGET_PAGE_SIZE region is mapped, the
 * supplied size is only used by tlb_flush_page.
 *
 * Called from TCG-generated code, which is under an RCU read-side
 * critical section.
 */
void tlb_set_page_with_attrs(CPUState *cpu, target_ulong vaddr,
                             hwaddr paddr, MemTxAttrs attrs, int prot,
                             int mmu_idx, target_ulong size)
{
    CPUArchState *env = cpu->env_ptr;
    MemoryRegionSection *section;
    unsigned int index;
    target_ulong address;
    target_ulong code_address, write_address;
    uintptr_t addend;
    CPUTLBEntry *te;
    hwaddr iotlb, xlat, sz;
    unsigned vidx = env->vtlb_index++ % CPU_VTLB_SIZE;
    int asidx = cpu_asidx_from_attrs(cpu, attrs);

    assert_cpu_is_self(cpu);
    assert(size >= TARGET_PAGE_SIZE);
    if (size != TARGET_PAGE_SIZE) {
        tlb_add_large_page(env, vaddr, size);
    }

    sz = size;
    section = address_space_translate_for_iotlb(cpu, asidx, paddr, &xlat, &sz);
    assert(sz >= TARGET_PAGE_SIZE);

    tlb_debug("vaddr=" TARGET_FMT_lx " paddr=0x" TARGET_FMT_plx
              " prot=%x idx=%d\n",
              vaddr, paddr, prot, mmu_idx);

    address = vaddr;
    if (!memory_region_is_ram(section->mr) && !memory_region_is_romd(section->mr)) {
        /* IO memory case */
        address |= TLB_MMIO;
        addend = 0;
    } else {
        /* TLB_MMIO for rom/romd handled below */
        addend = (uintptr_t)memory_region_get_ram_ptr(section->mr) + xlat;
    }

    code_address = address;
    iotlb = memory_region_section_get_iotlb(cpu, section, vaddr, paddr, xlat,
                                            prot, &address);

    index = (vaddr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
    te = &env->tlb_table[mmu_idx][index];

    /* do not discard the translation in te, evict it into a victim tlb */
    env->tlb_v_table[mmu_idx][vidx] = *te;
    env->iotlb_v[mmu_idx][vidx] = env->iotlb[mmu_idx][index];

    /* refill the tlb */
    env->iotlb[mmu_idx][index].addr = iotlb - vaddr;
    env->iotlb[mmu_idx][index].attrs = attrs;
    te->addend = addend - vaddr;
    if (prot & PAGE_READ) {
        te->addr_read = address;
    } else {
        te->addr_read = -1;
    }

    if (prot & PAGE_EXEC) {
        te->addr_code = code_address;
    } else {
        te->addr_code = -1;
    }

    write_address = -1;
    if (prot & PAGE_WRITE) {
        if ((memory_region_is_ram(section->mr) && section->readonly)
            || memory_region_is_romd(section->mr)) {
            /* Write access calls the I/O callback.  */
            write_address = address | TLB_MMIO;
        } else if (memory_region_is_ram(section->mr)
                   && cpu_physical_memory_is_clean(
                        memory_region_get_ram_addr(section->mr) + xlat)) {
            write_address = address | TLB_NOTDIRTY;
        } else {
            write_address = address;
        }
    }

    /* Pairs with flag setting in tlb_reset_dirty_range */
    atomic_mb_set(&te->addr_write, write_address);
}

/* Add a new TLB entry, but without specifying the memory
 * transaction attributes to be used.
 */
void tlb_set_page(CPUState *cpu, target_ulong vaddr,
                  hwaddr paddr, int prot,
                  int mmu_idx, target_ulong size)
{
    tlb_set_page_with_attrs(cpu, vaddr, paddr, MEMTXATTRS_UNSPECIFIED,
                            prot, mmu_idx, size);
}

static void report_bad_exec(CPUState *cpu, target_ulong addr)
{
    /* Accidentally executing outside RAM or ROM is quite common for
     * several user-error situations, so report it in a way that
     * makes it clear that this isn't a QEMU bug and provide suggestions
     * about what a user could do to fix things.
     */
    error_report("Trying to execute code outside RAM or ROM at 0x"
                 TARGET_FMT_lx, addr);
    error_printf("This usually means one of the following happened:\n\n"
                 "(1) You told QEMU to execute a kernel for the wrong machine "
                 "type, and it crashed on startup (eg trying to run a "
                 "raspberry pi kernel on a versatilepb QEMU machine)\n"
                 "(2) You didn't give QEMU a kernel or BIOS filename at all, "
                 "and QEMU executed a ROM full of no-op instructions until "
                 "it fell off the end\n"
                 "(3) Your guest kernel has a bug and crashed by jumping "
                 "off into nowhere\n\n"
                 "This is almost always one of the first two, so check your "
                 "command line and that you are using the right type of kernel "
                 "for this machine.\n"
                 "If you think option (3) is likely then you can try debugging "
                 "your guest with the -d debug options; in particular "
                 "-d guest_errors will cause the log to include a dump of the "
                 "guest register state at this point.\n\n"
                 "Execution cannot continue; stopping here.\n\n");

    /* Report also to the logs, with more detail including register dump */
    qemu_log_mask(LOG_GUEST_ERROR, "qemu: fatal: Trying to execute code "
                  "outside RAM or ROM at 0x" TARGET_FMT_lx "\n", addr);
    log_cpu_state_mask(LOG_GUEST_ERROR, cpu, CPU_DUMP_FPU | CPU_DUMP_CCOP);
}

static inline ram_addr_t qemu_ram_addr_from_host_nofail(void *ptr)
{
    ram_addr_t ram_addr;

    ram_addr = qemu_ram_addr_from_host(ptr);
    if (ram_addr == RAM_ADDR_INVALID) {
        error_report("Bad ram pointer %p", ptr);
        abort();
    }
    return ram_addr;
}

/* NOTE: this function can trigger an exception */
/* NOTE2: the returned address is not exactly the physical address: it
 * is actually a ram_addr_t (in system mode; the user mode emulation
 * version of this function returns a guest virtual address).
 */
tb_page_addr_t get_page_addr_code(CPUArchState *env1, target_ulong addr)
{
    int mmu_idx, page_index, pd;
    void *p;
    MemoryRegion *mr;
    CPUState *cpu = ENV_GET_CPU(env1);
    CPUIOTLBEntry *iotlbentry;

    page_index = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
    mmu_idx = cpu_mmu_index(env1, true);
    if (unlikely(env1->tlb_table[mmu_idx][page_index].addr_code !=
                 (addr & TARGET_PAGE_MASK))) {
        cpu_ldub_code(env1, addr);
    }
    iotlbentry = &env1->iotlb[mmu_idx][page_index];
    pd = iotlbentry->addr & ~TARGET_PAGE_MASK;
    mr = iotlb_to_region(cpu, pd, iotlbentry->attrs);
    if (memory_region_is_unassigned(mr)) {
        CPUClass *cc = CPU_GET_CLASS(cpu);

        if (cc->do_unassigned_access) {
            cc->do_unassigned_access(cpu, addr, false, true, 0, 4);
        } else {
            report_bad_exec(cpu, addr);
            exit(1);
        }
    }
    p = (void *)((uintptr_t)addr + env1->tlb_table[mmu_idx][page_index].addend);
    return qemu_ram_addr_from_host_nofail(p);
}

/* Return true if ADDR is present in the victim tlb, and has been copied
   back to the main tlb.  */
static bool victim_tlb_hit(CPUArchState *env, size_t mmu_idx, size_t index,
                           size_t elt_ofs, target_ulong page)
{
    size_t vidx;
    for (vidx = 0; vidx < CPU_VTLB_SIZE; ++vidx) {
        CPUTLBEntry *vtlb = &env->tlb_v_table[mmu_idx][vidx];
        target_ulong cmp = *(target_ulong *)((uintptr_t)vtlb + elt_ofs);

        if (cmp == page) {
            /* Found entry in victim tlb, swap tlb and iotlb.  */
            CPUTLBEntry tmptlb, *tlb = &env->tlb_table[mmu_idx][index];
            CPUIOTLBEntry tmpio, *io = &env->iotlb[mmu_idx][index];
            CPUIOTLBEntry *vio = &env->iotlb_v[mmu_idx][vidx];

            tmptlb = *tlb; *tlb = *vtlb; *vtlb = tmptlb;
            tmpio = *io; *io = *vio; *vio = tmpio;
            return true;
        }
    }
    return false;
}

/* Macro to call the above, with local variables from the use context.  */
#define VICTIM_TLB_HIT(TY, ADDR) \
  victim_tlb_hit(env, mmu_idx, index, offsetof(CPUTLBEntry, TY), \
                 (ADDR) & TARGET_PAGE_MASK)

#define MMUSUFFIX _mmu

#define SHIFT 0
#include "softmmu_template.h"

#define SHIFT 1
#include "softmmu_template.h"

#define SHIFT 2
#include "softmmu_template.h"

#define SHIFT 3
#include "softmmu_template.h"

#ifdef CONFIG_ATOMIC128
#define SHIFT 4
#include "softmmu_template.h"
#endif

#undef MMUSUFFIX

#define MMUSUFFIX _cmmu
#undef GETPC_ADJ
#define GETPC_ADJ 0
#undef GETRA
#define GETRA() ((uintptr_t)0)
#define SOFTMMU_CODE_ACCESS

#define SHIFT 0
#include "softmmu_template.h"

#define SHIFT 1
#include "softmmu_template.h"

#define SHIFT 2
#include "softmmu_template.h"

#define SHIFT 3
#include "softmmu_template.h"
