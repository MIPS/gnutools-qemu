/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012  MIPS Technologies, Inc.  All rights reserved.
 * Authors: Sanjay Lal <sanjayl@kymasys.com>
 *
 * Copyright (C) 2015 Imagination Technologies
*/

#include "hw/hw.h"
#include "qemu/bitmap.h"
#include "exec/memory.h"
#include "sysemu/sysemu.h"
#include "qom/cpu.h"
#include "exec/address-spaces.h"

#ifdef CONFIG_KVM
#include "sysemu/kvm.h"
#include "kvm_mips.h"
#endif

#include "hw/mips/mips_gic.h"
#include "hw/mips/mips_gcmpregs.h"

//#define DEBUG
//
//#ifdef DEBUG
//#define DPRINTF(fmt, ...) fprintf(stderr, "QEMU %s: " fmt, __FUNCTION__, ##__VA_ARGS__)
//#else
//#define DPRINTF(fmt, ...)
//#endif

//#define DPRINTF(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define DPRINTF(fmt, ...)
#define SPRINTF(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#if defined(MIPSSIM_COMPAT)
/* In SV - C0COUNT has to be incremented by a number of executed instructions
   as this is how IASim works.
*/
#define TIMER_FREQ get_ticks_per_sec()
#else
#define TIMER_FREQ  100 * 1000 * 1000
#endif

/* Support upto 4 VPEs */
//#define NUMVPES     4
#define NUMVPES     32

/* XXXKYMA: Spoof a bit of the GCR as well, just enough to get Linux to detect it */
typedef struct _gic_timer_t
{
    QEMUTimer *timer;
    uint32_t vp_index;
    struct gic_t *gic;
} gic_timer_t;

typedef struct gic_t
{
    CPUMIPSState *env[NUMVPES];
//    CPUMIPSState *env[NR_CPUS];
    MemoryRegion gcr_mem, gic_mem;

    qemu_irq *irqs;

    /* GCR Registers */
    target_ulong gcr_gic_base;

    /* Shared Section Registers */
    uint32_t gic_gl_config;
    uint32_t gic_gl_intr_pol_reg[8];
    uint32_t gic_gl_intr_trigtype_reg[8];
    uint32_t gic_gl_intr_pending_reg[8];
    uint32_t gic_gl_intr_mask_reg[8];

    uint32_t gic_gl_map_pin[256];

    /* Sparse array, need a better way */
    uint32_t gic_gl_map_vpe[0x7fa];
//    uint32_t gic_gl_map_vpe[512];

    /* VPE Local Section Registers */
    /* VPE Other Section Registers, aliased to local, use the other field to access the correct instance */
    uint32_t gic_vpe_ctl[NUMVPES];
    uint32_t gic_vpe_pend[NUMVPES];
    uint32_t gic_vpe_mask[NUMVPES];
    uint32_t gic_vpe_wd_map[NUMVPES];
    uint32_t gic_vpe_compare_map[NUMVPES];
    uint32_t gic_vpe_timer_map[NUMVPES];
    uint32_t gic_vpe_comparelo[NUMVPES];
    uint32_t gic_vpe_comparehi[NUMVPES];

    uint32_t gic_vpe_other_addr[NUMVPES];
    /* User Mode Visible Section Registers */

    uint32_t gic_sh_counterlo;
    QEMUTimer *timer; /* Global timer */
    uint32_t num_cpu;
    gic_timer_t *gic_timer;
} gic_t;


// prototypes
uint32_t gic_get_count(gic_t *gic);
uint32_t gic_get_sh_count(gic_t *gic);
static uint32_t gic_timer_update(gic_t *gic);
static uint32_t gic_vpe_timer_update(gic_t *gic, uint32_t vp_index);
static void gic_set_irq(void *opaque, int n_IRQ, int level);

static inline int gic_get_current_cpu(gic_t *g)
{
    if (g->num_cpu > 1) {
        return current_cpu->cpu_index;
    }
    return 0;
}

static uint64_t gic_read_vpe(gic_t *gic, uint32_t vp_index,
                                     hwaddr addr, unsigned size)
{
    switch (addr){
    case GIC_VPE_CTL_OFS:
        DPRINTF("(GIC_VPE_CTL) -> 0x%016x\n", gic->gic_vpe_ctl[vp_index]);
        return gic->gic_vpe_ctl[vp_index];
    case GIC_VPE_PEND_OFS:
//        gic_get_count(gic);
        gic_get_sh_count(gic);
        DPRINTF("(GIC_VPE_PEND) -> 0x%016x\n", gic->gic_vpe_pend[vp_index]);
        return gic->gic_vpe_pend[vp_index];
    case GIC_VPE_MASK_OFS:
        DPRINTF("(GIC_VPE_MASK) -> 0x%016x\n", gic->gic_vpe_mask[vp_index]);
        return gic->gic_vpe_mask[vp_index];
    case GIC_VPE_OTHER_ADDR_OFS:
        DPRINTF("(GIC_VPE_OTHER_ADDR) -> 0x%016x\n",
                gic->gic_vpe_other_addr[vp_index]);
        return gic->gic_vpe_other_addr[vp_index];
    case GIC_VPE_COMPARE_LO_OFS:
        DPRINTF("(GIC_VPE_COMPARELO) -> 0x%016x\n",
                gic->gic_vpe_comparelo[vp_index]);
        return gic->gic_vpe_comparelo[vp_index];
    case GIC_VPE_COMPARE_HI_OFS:
        DPRINTF("(GIC_VPE_COMPAREhi) -> 0x%016x\n",
                gic->gic_vpe_comparehi[vp_index]);
        return gic->gic_vpe_comparehi[vp_index];
    default:
        SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** read %d bytes "
                "at GIC offset LOCAL/OTHER 0x%04x\n", size, addr);
        break;
    }
    return 0;
}

static uint64_t
gic_read(void *opaque, hwaddr addr, unsigned size)
{
    int reg;
    gic_t *gic = (gic_t *) opaque;
    uint32_t vp_index = gic_get_current_cpu(gic);
    uint32_t ret = 0;

//    DPRINTF("addr: %#" PRIx64 ", size: %#x\n", addr, size);
        DPRINTF("Info (MIPS64_CMP) read %d bytes at GIC offset 0x%04x ", size, addr);

    switch (addr) {
    case GIC_SH_CONFIG_OFS:
//        DPRINTF("(GIC_SH_CONFIG) -> 0x%016x\n", 0x10040000);
//        return 0x10040000; // | ((NUMVPES - 1) & GIC_SH_CONFIG_NUMVPES_MSK);
        DPRINTF("(GIC_SH_CONFIG) -> 0x%016x\n", gic->gic_gl_config);
        return gic->gic_gl_config;
        break;
    case GIC_SH_CONFIG_OFS + 4:
        //do nothing
        return 0;
        break;
    case GIC_SH_COUNTERLO_OFS:
        //return cpu_mips_get_count(gic->env[0]);
    {
//        ret = gic_get_count(gic);
        ret = gic_get_sh_count(gic);
        DPRINTF("(GIC_SH_COUNTERLO) -> 0x%016x\n", ret);
        return ret;
    }
        break;
    case GIC_SH_COUNTERHI_OFS:
        DPRINTF("(Not supported GIC_SH_COUNTERHI) -> 0x%016x\n", 0);
        return 0;
        break;

    case GIC_SH_POL_31_0_OFS:
    case GIC_SH_POL_63_32_OFS:
    case GIC_SH_POL_95_64_OFS:
    case GIC_SH_POL_127_96_OFS:
    case GIC_SH_POL_159_128_OFS:
    case GIC_SH_POL_191_160_OFS:
    case GIC_SH_POL_223_192_OFS:
    case GIC_SH_POL_255_224_OFS:
        reg = (addr - GIC_SH_POL_31_0_OFS) / 4;
        ret = gic->gic_gl_intr_pol_reg[reg];
        DPRINTF("(GIC_SH_POL) -> 0x%016x\n", ret);
        return ret;
        break;

    case GIC_SH_TRIG_31_0_OFS:
    case GIC_SH_TRIG_63_32_OFS:
    case GIC_SH_TRIG_95_64_OFS:
    case GIC_SH_TRIG_127_96_OFS:
    case GIC_SH_TRIG_159_128_OFS:
    case GIC_SH_TRIG_191_160_OFS:
    case GIC_SH_TRIG_223_192_OFS:
    case GIC_SH_TRIG_255_224_OFS:
        reg = (addr - GIC_SH_TRIG_31_0_OFS) / 4;
        ret = gic->gic_gl_intr_trigtype_reg[reg];
        DPRINTF("(GIC_SH_TRIG) -> 0x%016x\n", ret);
        return ret;
        break;

    case GIC_SH_RMASK_31_0_OFS:
    case GIC_SH_RMASK_63_32_OFS:
    case GIC_SH_RMASK_95_64_OFS:
    case GIC_SH_RMASK_127_96_OFS:
    case GIC_SH_RMASK_159_128_OFS:
    case GIC_SH_RMASK_191_160_OFS:
    case GIC_SH_RMASK_223_192_OFS:
    case GIC_SH_RMASK_255_224_OFS:
        break;

    case GIC_SH_PEND_31_0_OFS:
    case GIC_SH_PEND_63_32_OFS:
    case GIC_SH_PEND_95_64_OFS:
    case GIC_SH_PEND_127_96_OFS:
    case GIC_SH_PEND_159_128_OFS:
    case GIC_SH_PEND_191_160_OFS:
    case GIC_SH_PEND_223_192_OFS:
    case GIC_SH_PEND_255_224_OFS:
        reg = (addr - GIC_SH_PEND_31_0_OFS) / 4;
//        DPRINTF("pending[%d]: %#" PRIx32 "\n", reg, gic->gic_gl_intr_pending_reg[reg]);
        ret = gic->gic_gl_intr_pending_reg[reg];
        DPRINTF("(GIC_SH_PEND) -> 0x%016x\n", ret);
        return ret;
        break;

    case GIC_SH_MASK_31_0_OFS:
    case GIC_SH_MASK_63_32_OFS:
    case GIC_SH_MASK_95_64_OFS:
    case GIC_SH_MASK_127_96_OFS:
    case GIC_SH_MASK_159_128_OFS:
    case GIC_SH_MASK_191_160_OFS:
    case GIC_SH_MASK_223_192_OFS:
    case GIC_SH_MASK_255_224_OFS:
        reg = (addr - GIC_SH_MASK_31_0_OFS) / 4;
        ret =  gic->gic_gl_intr_mask_reg[reg];
        DPRINTF("(GIC_SH_MASK) -> 0x%016x\n", ret);
        return ret;
        break;

    default:
        if (addr < GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) {
            SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** read %d bytes at GIC offset 0x%04x \n", size, addr);
        }
        break;
    }

    /* Global Interrupt Map SrcX to Pin register */
    if (addr >= GIC_SH_INTR_MAP_TO_PIN_BASE_OFS
        && addr <= GIC_SH_MAP_TO_PIN(255)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) / 4;
        ret = gic->gic_gl_map_pin[reg];
        DPRINTF("(GIC) -> 0x%016x\n", ret);
        return ret;
    }

    /* Global Interrupt Map SrcX to VPE register */
    if (addr >= GIC_SH_INTR_MAP_TO_VPE_BASE_OFS
        && addr <= GIC_SH_MAP_TO_VPE_REG_OFF(255, 63)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_VPE_BASE_OFS) / 4;
        ret = gic->gic_gl_map_vpe[reg];
        DPRINTF("(GIC) -> 0x%016x\n", ret);
        return ret;
    }

    /* VPE-Local Register */
    if (addr >= GIC_VPELOCAL_BASE_ADDR && addr < GIC_VPEOTHER_BASE_ADDR) {
        return gic_read_vpe(gic, vp_index, addr - GIC_VPELOCAL_BASE_ADDR, size);
    }

    /* VPE-Other Register */
    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
        uint32_t other_index = gic->gic_vpe_other_addr[vp_index];
        return gic_read_vpe(gic, other_index, addr - GIC_VPEOTHER_BASE_ADDR, size);
    }

//    DPRINTF("%s: unimplemented register @ %#" PRIx64 "\n", __func__, addr);
    return 0ULL;
}

static uint64_t gic_write_vpe(gic_t *gic, uint32_t vp_index, hwaddr addr,
                              uint64_t data, unsigned size)
{
    switch(addr) {
    case GIC_VPE_CTL_OFS:
        gic->gic_vpe_ctl[vp_index] &= ~1;
        gic->gic_vpe_ctl[vp_index] |= data & 1;
//            DPRINTF("QEMU: GIC Local interrupt control reg WRITE %x\n", data);
        break;
    case GIC_VPE_OTHER_ADDR_OFS:
        gic->gic_vpe_other_addr[vp_index] = data;
//            DPRINTF("QEMU: GIC other addressing reg WRITE %x\n", data);
        break;
    case GIC_VPE_OTHER_ADDR_OFS + 4:
        // do nothing
        break;
    case GIC_VPE_RMASK_OFS:
        DPRINTF("QEMU VPE%d RMASK org_mask %x, ", vp_index,
                gic->gic_vpe_mask[vp_index]);
        gic->gic_vpe_mask[vp_index] &= ~(data & 0x3f) & 0x3f;
        DPRINTF("data %x, mask %x\n", data, gic->gic_vpe_mask[vp_index]);
        break;
    case GIC_VPE_SMASK_OFS:
        DPRINTF("QEMU VPE%d SMASK org_mask %x, ", vp_index,
                gic->gic_vpe_mask[vp_index]);
        gic->gic_vpe_mask[vp_index] |= (data & 0x3f);
        DPRINTF("data %x, mask %x\n", data, gic->gic_vpe_mask[vp_index]);
        break;
    case GIC_VPE_WD_MAP_OFS:
        gic->gic_vpe_wd_map[vp_index] = data & 0xE000003F;
        break;
    case GIC_VPE_COMPARE_MAP_OFS:
        gic->gic_vpe_compare_map[vp_index] = data & 0xE000003F;
        DPRINTF("QEMU: GIC COMPARE MAP %x %x\n", data,
                gic->gic_vpe_compare_map[vp_index]);
        break;
    case GIC_VPE_TIMER_MAP_OFS:
        gic->gic_vpe_timer_map[vp_index] = data & 0xE000003F;
        DPRINTF("QEMU: GIC Timer MAP %x %x\n", data,
                gic->gic_vpe_timer_map[vp_index]);
        break;
    case GIC_VPE_COMPARE_LO_OFS:
        gic->gic_vpe_comparelo[vp_index] = (uint32_t) data;
        if (!(gic->gic_gl_config & 0x10000000)) {
//            uint32_t wait = gic_timer_update(gic);
            uint32_t wait = gic_vpe_timer_update(gic, vp_index);
            DPRINTF("GIC Compare modified (GIC_VPE_Compare=0x%x GIC_Counter=0x%x) - schedule CMP timer interrupt after 0x%x\n",
                    gic->gic_vpe_comparelo[vp_index], gic->gic_sh_counterlo, wait);
        }
        gic->gic_vpe_pend[vp_index] &= ~(1 << 1);
        if (gic->gic_vpe_compare_map[vp_index] & 0x80000000) {
            printf("x%d", gic->gic_vpe_compare_map[vp_index] & 0x3F);
            fflush(0);
//                gic_set_irq(gic, gic->gic_vpe_compare_map[vp_index] & 0x3F, 0);
            qemu_set_irq(gic->env[vp_index]->irq[(gic->gic_vpe_compare_map[vp_index] & 0x3F)+2], 0);
        }
        break;
    case GIC_VPE_COMPARE_HI_OFS:
        break;
    default:
//            DPRINTF("QEMU: not supported GIC Local WRITE request addr: %#" PRIx64 ", %lx\n", addr, data);
        SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** write %d bytes at GIC offset 0x%04x 0x%08lx\n", size, addr, data);
        break;
    }
}

static void
gic_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    int reg, intr;
    gic_t *gic = (gic_t *) opaque;
    uint32_t vp_index = gic_get_current_cpu(gic);

//    DPRINTF("addr: %#" PRIx64 ", data: %#" PRIx64 ", size: %#x\n", addr, data,
//            size);
        if (addr == 0x104 || addr == 0x184 || addr == 0x304 ||
                addr == 0x10c || addr == 0x18c || addr == 0x30c) {
            DPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** unimplemented GIC read at offset 0x%04x\n", addr);
        }
            DPRINTF("Info (MIPS64_CMP) write %d bytes at GIC offset 0x%04x (GIC) <- 0x%016x\n", size, addr, data);

    switch (addr) {
    case GIC_SH_CONFIG_OFS:
    {
        uint32_t pre = gic->gic_gl_config;
        gic->gic_gl_config = (gic->gic_gl_config & 0xEFFFFFFF) |
                             (data & 0x10000000);
        if (pre != gic->gic_gl_config) {
            if (!(pre & 0x10000000) && (gic->gic_gl_config & 0x10000000)) {
                DPRINTF("Info (MIPS64_CMP) GIC_SH_CONFIG.COUNTSTOP modified STOPPING\n");
                printf("Info (MIPS64_CMP) GIC_SH_CONFIG.COUNTSTOP modified STOPPING\n");
//                timer_mod(gic->timer, muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL), get_ticks_per_sec(), TIMER_FREQ));
            }
            if ((pre & 0x10000000) && !(gic->gic_gl_config & 0x10000000)) {
                DPRINTF("Info (MIPS64_CMP) GIC_SH_CONFIG.COUNTSTOP modified STARTING\n");
                printf("Info (MIPS64_CMP) GIC_SH_CONFIG.COUNTSTOP modified STARTING\n");
//                timer_mod(gic->timer, muldiv64(1, get_ticks_per_sec(), TIMER_FREQ));
            }
        }
//        gic_timer_update(gic);
        {
            int i;
            for (i = 0; i < gic->num_cpu; i++) {
                gic_vpe_timer_update(gic, i);
            }
        }
    }
        break;
    case GIC_SH_CONFIG_OFS + 4:
        //do nothing
        break;
    case GIC_SH_COUNTERLO_OFS:
    case GIC_SH_COUNTERHI_OFS:
//        DPRINTF("QEMU: not supported TIMER WRITE request %lx\n", data);
        break;

    case GIC_SH_POL_31_0_OFS:
    case GIC_SH_POL_63_32_OFS:
    case GIC_SH_POL_95_64_OFS:
    case GIC_SH_POL_127_96_OFS:
    case GIC_SH_POL_159_128_OFS:
    case GIC_SH_POL_191_160_OFS:
    case GIC_SH_POL_223_192_OFS:
    case GIC_SH_POL_255_224_OFS:
        reg = (addr - GIC_SH_POL_31_0_OFS) / 4;
        gic->gic_gl_intr_pol_reg[reg] = data;
        break;

    case GIC_SH_TRIG_31_0_OFS:
    case GIC_SH_TRIG_63_32_OFS:
    case GIC_SH_TRIG_95_64_OFS:
    case GIC_SH_TRIG_127_96_OFS:
    case GIC_SH_TRIG_159_128_OFS:
    case GIC_SH_TRIG_191_160_OFS:
    case GIC_SH_TRIG_223_192_OFS:
    case GIC_SH_TRIG_255_224_OFS:
        reg = (addr - GIC_SH_TRIG_31_0_OFS) / 4;
        gic->gic_gl_intr_trigtype_reg[reg] = data;
        break;

    case GIC_SH_RMASK_31_0_OFS:
    case GIC_SH_RMASK_63_32_OFS:
    case GIC_SH_RMASK_95_64_OFS:
    case GIC_SH_RMASK_127_96_OFS:
    case GIC_SH_RMASK_159_128_OFS:
    case GIC_SH_RMASK_191_160_OFS:
    case GIC_SH_RMASK_223_192_OFS:
    case GIC_SH_RMASK_255_224_OFS:
        reg = (addr - GIC_SH_RMASK_31_0_OFS) / 4;
        gic->gic_gl_intr_mask_reg[reg] &= ~data;
        break;

    case GIC_SH_WEDGE_OFS:
        DPRINTF("addr: %#" PRIx64 ", data: %#" PRIx64 ", size: %#x\n", addr,
               data, size);

        /* Figure out which VPE/HW Interrupt this maps to */
        intr = data & 0x7FFFFFFF;

        /* XXXSL  Mask/Enabled Checks */
        if (data & 0x80000000)
            qemu_set_irq(gic->irqs[intr], 1);
        else
            qemu_set_irq(gic->irqs[intr], 0);

        break;

    case GIC_SH_PEND_31_0_OFS:
    case GIC_SH_PEND_63_32_OFS:
    case GIC_SH_PEND_95_64_OFS:
    case GIC_SH_PEND_127_96_OFS:
    case GIC_SH_PEND_159_128_OFS:
    case GIC_SH_PEND_191_160_OFS:
    case GIC_SH_PEND_223_192_OFS:
    case GIC_SH_PEND_255_224_OFS:
        break;

    case GIC_SH_SMASK_31_0_OFS:
    case GIC_SH_SMASK_63_32_OFS:
    case GIC_SH_SMASK_95_64_OFS:
    case GIC_SH_SMASK_127_96_OFS:
    case GIC_SH_SMASK_159_128_OFS:
    case GIC_SH_SMASK_191_160_OFS:
    case GIC_SH_SMASK_223_192_OFS:
    case GIC_SH_SMASK_255_224_OFS:
        reg = (addr - GIC_SH_SMASK_31_0_OFS) / 4;
        gic->gic_gl_intr_mask_reg[reg] |= data;
        break;

    default:
        if (addr < GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) {
//            DPRINTF("QEMU: not supported GIC WRITE request addr: %#" PRIx64 ", %lx\n", addr, data);
            SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** write %d bytes at GIC offset 0x%04x 0x%08lx\n", size, addr, data);
        }
        break;
    }

    /* Other cases */
    if (addr >= GIC_SH_INTR_MAP_TO_PIN_BASE_OFS
        && addr <= GIC_SH_MAP_TO_PIN(255)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) / 4;
        gic->gic_gl_map_pin[reg] = data;
    }
    if (addr >= GIC_SH_INTR_MAP_TO_VPE_BASE_OFS
        && addr <= GIC_SH_MAP_TO_VPE_REG_OFF(255, 63)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_VPE_BASE_OFS) / 4;
        gic->gic_gl_map_vpe[reg] = data;
    }

    if (addr >= GIC_VPELOCAL_BASE_ADDR && addr < GIC_VPEOTHER_BASE_ADDR) {
        gic_write_vpe(gic, vp_index, addr - GIC_VPELOCAL_BASE_ADDR,
                      data, size);
    }

    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
        uint32_t other_index = gic->gic_vpe_other_addr[vp_index];
        gic_write_vpe(gic, other_index, addr - GIC_VPEOTHER_BASE_ADDR,
                      data, size);
    }
}

static uint64_t
gcr_read(void *opaque, hwaddr addr, unsigned size)
{
    gic_t *gic = (gic_t *) opaque;

//    DPRINTF("addr: %#" PRIx64 ", size: %#x\n", addr, size);
//    DPRINTF("GCMP_BASE_ADDR: %#" PRIx64 "\n", GCMP_BASE_ADDR);

    DPRINTF("Info (MIPS64_CMP) I6400_CPU0 read %d bytes at GCR offset 0x%04x (GCR) -> ", size, addr);

    switch (addr) {
    case GCMP_GCB_GC_OFS:
        /* Set PCORES to # cores - 1 */
        DPRINTF("0x%016lx\n", smp_cpus-1);
        return smp_cpus - 1;
        break;

    case GCMP_GCB_GCMPB_OFS:
        DPRINTF("0x%016lx\n", GCMP_BASE_ADDR);
        DPRINTF("GCMP_BASE_ADDR: %#" PRIx64 "\n", GCMP_BASE_ADDR);
        return GCMP_BASE_ADDR;
        break;
    case GCMP_GCB_GCMPREV_OFS:
//        DPRINTF("0x%016lx\n", 0x700);
//        return 0x700;
        DPRINTF("0x%016lx\n", 0x800);
        return 0x800;

    case GCMP_GCB_GICBA_OFS:
        DPRINTF("0x%016lx\n", gic->gcr_gic_base);
        return gic->gcr_gic_base;
        break;

    case GCMP_GCB_GICST_OFS:
        /* FIXME indicates a connection between GIC and CM */
        DPRINTF("0x%016lx\n", GCMP_GCB_GICST_EX_MSK);
        return GCMP_GCB_GICST_EX_MSK;

    case 0xf0: //GCR_CPC_STATUS
        DPRINTF("0x%016lx\n", 0);
        return 0;

    default:
        DPRINTF("0x%016lx\n", 0);
        DPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** unimplemented GCR read at offset 0x%04x\n", addr);
//        DPRINTF("Unsupported Reg Read @ offset %#" PRIx64 "\n", addr);
        return 0;
    }

    return 0ULL;
}

static void
gcr_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    gic_t *gic = (gic_t *) opaque;

//    DPRINTF("addr: %#" PRIx64 ", data: %#" PRIx64 ", size: %#x\n", addr, data,
//            size);

    switch (addr) {
    case GCMP_GCB_GICBA_OFS:
        DPRINTF("Info (MIPS64_CMP) I6400_CPU0 write %d bytes at GCR offset %04x (GCR) <- 0x%016lx\n", size, addr, data);
        gic->gcr_gic_base = data & ~1;
        if (data & 1) {
            memory_region_del_subregion(get_system_memory(), &gic->gic_mem);
            memory_region_add_subregion(get_system_memory(), gic->gcr_gic_base, &gic->gic_mem);
            printf ("init gic base addr %x %x\n", data, gic->gcr_gic_base);
        }
        break;

    default:
        DPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** unimplemented GCR write at offset 0x%04x\n", addr);
        break;
    }
}



static void
gic_set_irq(void *opaque, int n_IRQ, int level)
{
    int vpe = -1, pin = -1, i;
    gic_t *gic = (gic_t *) opaque;

//    printf("gic_set_irq %d\n", n_IRQ);
    pin = gic->gic_gl_map_pin[n_IRQ] & 0x7;

    for (i = 0; i < NUMVPES; i++) {
        vpe = gic-> gic_gl_map_vpe[(GIC_SH_MAP_TO_VPE_REG_OFF(n_IRQ, i) - GIC_SH_INTR_MAP_TO_VPE_BASE_OFS) / 4];
        if (vpe & GIC_SH_MAP_TO_VPE_REG_BIT(i)) {
            vpe = i;
            break;
        }
    }

    if (pin >= 0 && vpe >= 0) {
        int offset;
        DPRINTF("[%s] INTR %d maps to PIN %d on VPE %d\n", (level ? "ASSERT" : "DEASSERT"), n_IRQ, pin, vpe);
        /* Set the Global PEND register */
        offset = GIC_INTR_OFS(n_IRQ) / 4;
        if (level)
            gic->gic_gl_intr_pending_reg[offset] |= (1 << GIC_INTR_BIT(n_IRQ));
        else
            gic->gic_gl_intr_pending_reg[offset] &= ~(1 << GIC_INTR_BIT(n_IRQ));

#ifdef CONFIG_KVM
        if (kvm_enabled())  {
            kvm_mips_set_ipi_interrupt (gic->env[vpe], pin+2, level);
        }
#endif
        if (n_IRQ != 3)
            printf("--%d: GIC set VPE%d IRQ%d level %d\n", n_IRQ, vpe, pin+2, level);
        qemu_set_irq(gic->env[vpe]->irq[pin+2], level);
    }
}

static void
gic_reset(void *opaque)
{
    int i;
    gic_t *gic = (gic_t *) opaque;

    /* Rest value is map to pin */
    for (i = 0; i < 256; i++)
        gic->gic_gl_map_pin[i] = GIC_MAP_TO_PIN_MSK;

    gic->gic_sh_counterlo = 0;
}

static const MemoryRegionOps gic_ops = {
    .read = gic_read,
    .write = gic_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static const MemoryRegionOps gcr_ops = {
    .read = gcr_read,
    .write = gcr_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static uint32_t gic_timer_update(gic_t *gic)
{
    uint64_t now, next;
    uint32_t wait;
    uint32_t comparelo;
    int i;

    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    if (gic->gic_gl_config & 0x10000000) {
        wait = muldiv64(now, get_ticks_per_sec(), TIMER_FREQ);
    } else {
        comparelo = gic->gic_vpe_comparelo[0];
        for (i = 1; i < gic->num_cpu; i++) {
            if (comparelo > gic->gic_vpe_comparelo[i] &&
                gic->gic_vpe_comparelo[i] != 0) {
                comparelo = gic->gic_vpe_comparelo[i];
            }
        }

        wait = comparelo - gic->gic_sh_counterlo -
                (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec());
        next = now + muldiv64(wait, get_ticks_per_sec(), TIMER_FREQ);
    }
    timer_mod(gic->timer, next);
    return wait;
}

static void print_gic_status(gic_t *gic)
{
    int i;
    printf("__GIC__\n");
    printf("SHMASK ");
    for (i = 0; i < 8; i++) {
        printf("%08x ", gic->gic_gl_intr_mask_reg[i]);
    }
    printf("\n");

    printf("MAPVPE ");
    for (i = 0; i < 32; i++) {
        printf("%d:%x ", i / 2, gic->gic_gl_map_vpe[i]);
    }
    printf("\n");

    printf("MAPPIN ");
    for (i = 0; i < 32; i++) {
        printf("%d:%x ", i, gic->gic_gl_map_pin[i]);
    }
    printf("\n");

    printf("V0LocalMask %08x\n", gic->gic_vpe_mask[0]);
    printf("V0CompareMap %08x\n", gic->gic_vpe_compare_map[0]);
    printf("V0CTL %08x\n", gic->gic_vpe_ctl[0]);

}

static int gic_local_timer_expire(gic_t *gic, uint32_t vp_index, uint64_t now)
{
    uint64_t compare = gic->gic_vpe_comparelo[vp_index] |
            (gic->gic_vpe_comparehi[vp_index] << 32);
    if (!gic->gic_vpe_comparelo[vp_index] &&
        !gic->gic_vpe_comparehi[vp_index]) {
        return 0;
    }

    if (compare <=
            (gic->gic_sh_counterlo +
            (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec()))) {
        return 1;
    }
    return 0;
}

static void gic_timer_expire(gic_t *gic)
{
    int i;
    int num_requests = 0;
    uint32_t expired = 0;
    uint64_t now;
    gic_timer_update(gic);
    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    for (i = 0; i < gic->num_cpu; i++) {
        if (gic_local_timer_expire(gic, i, now)) {
            expired |= (1 << i);
            gic->gic_vpe_pend[i] |= (1 << 1);
            DPRINTF("gic_timer_expire pend%d %x", i, gic->gic_vpe_pend[i]);
        }
    }
    if (expired == 0) {
//        DPRINTF("ignored--- gic timer expire\n");
        return;
    }

//    print_gic_status(gic);

    for (i = 0; i < gic->num_cpu; i++) {
        if (gic->gic_vpe_pend[i] & (gic->gic_vpe_mask[i] & (1 << 1))) {
            if (gic->gic_vpe_compare_map[i] & 0x80000000) {
//              DPRINTF("QEMU GIC TIMER set IRQ%d\n", gic->gic_vpe_compare_map[i]);
                printf("~%d", gic->gic_vpe_compare_map[i] & 0x3F);
                fflush(0);
//              gic_set_irq(gic, gic->gic_vpe_compare_map[i] & 0x3F, 1);
                qemu_set_irq(gic->env[i]->irq[(gic->gic_vpe_compare_map[i] & 0x3F)+2], 1);
            }
        }
    }
}

void gic_store_count(gic_t *gic, uint32_t count)
{
    printf("---QEMU: gic_store_count()\n");
    if ((gic->gic_gl_config & 0x10000000) || !gic->timer) {
        gic->gic_sh_counterlo = count;
    } else {
        /* Store new count register */
        gic->gic_sh_counterlo =
            count - (uint32_t)muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL),
                                       TIMER_FREQ, get_ticks_per_sec());
        /* Update timer timer */
        gic_timer_update(gic);
    }
}

uint32_t gic_get_count(gic_t *gic)
{
    if (gic->gic_gl_config & (1 << 28)) {
        return gic->gic_sh_counterlo;
    } else {
        uint64_t now;

        now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        if (timer_pending(gic->timer)
            && timer_expired(gic->timer, now)) {
            /* The timer has already expired.  */
//            printf("!");
//            fflush(0);

            gic_timer_expire(gic);
        }

        return gic->gic_sh_counterlo +
            (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec());
    }
}

static void gic_timer_cb (void *opaque)
{
    gic_t *gic;

    gic = opaque;

//    printf("@");
//    fflush(0);
//    DPRINTF("---QEMU: mips_timer_cb() 1\n");
//    if (env->CP0_Cause & (1 << CP0Ca_DC))
//        return;
//    DPRINTF("---QEMU: mips_timer_cb() 2\n");
    /* ??? This callback should occur when the counter is exactly equal to
       the comparator value.  Offset the count by one to avoid immediately
       retriggering the callback before any virtual time has passed.  */
//    env->CP0_Count++;
//    cpu_mips_timer_expire(env);
//    env->CP0_Count--;
    gic->gic_sh_counterlo++;
//    DPRINTF("---QEMU: gic timer tick\n");
    gic_timer_expire(gic);
    gic->gic_sh_counterlo--;
}

void gic_global_timer_init (gic_t *gic)
{
    gic->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, &gic_timer_cb, gic);
    gic_store_count(gic, gic->gic_sh_counterlo);
//    if (!(gic->gic_gl_config & 0x10000000)) {
//        timer_mod(gic->timer, muldiv64(1, get_ticks_per_sec(), TIMER_FREQ));
//    }
//    gic_timer_update(gic);
}

// VPE Local Timer
static uint32_t gic_vpe_timer_update(gic_t *gic, uint32_t vp_index)
{
    uint64_t now, next;
    uint32_t wait;
    int i;

    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    if (gic->gic_gl_config & 0x10000000) {
        wait = muldiv64(now, get_ticks_per_sec(), TIMER_FREQ);
    } else {
        wait = gic->gic_vpe_comparelo[vp_index] - gic->gic_sh_counterlo -
                (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec());
        next = now + muldiv64(wait, get_ticks_per_sec(), TIMER_FREQ);
    }
    timer_mod(gic->gic_timer[vp_index].timer, next);
    return wait;
}

static void gic_vpe_timer_expire(gic_t *gic, uint32_t vp_index)
{
    gic_vpe_timer_update(gic, vp_index);
    gic->gic_vpe_pend[vp_index] |= (1 << 1);

    if (gic->gic_vpe_pend[vp_index] & (gic->gic_vpe_mask[vp_index] & (1 << 1))) {
        if (gic->gic_vpe_compare_map[vp_index] & 0x80000000) {
//              DPRINTF("QEMU GIC TIMER set IRQ%d\n", gic->gic_vpe_compare_map[i]);
            printf("~%d", gic->gic_vpe_compare_map[vp_index] & 0x3F);
            fflush(0);
//                qemu_set_irq(gic->env[i]->irq[(gic->gic_vpe_compare_map[i] & 0x3F)+2], 1);
            qemu_irq_raise(gic->env[vp_index]->irq[(gic->gic_vpe_compare_map[vp_index] & 0x3F)+2]);
        }
    }
}

void gic_store_sh_count(gic_t *gic, uint32_t count)
{
    printf("---QEMU: gic_store_count()\n");
    int i;
    if ((gic->gic_gl_config & 0x10000000) || !gic->timer) {
        gic->gic_sh_counterlo = count;
    } else {
        /* Store new count register */
        gic->gic_sh_counterlo =
            count - (uint32_t)muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL),
                                       TIMER_FREQ, get_ticks_per_sec());
        /* Update timer timer */
        for (i = 0; i < gic->num_cpu; i++) {
            gic_vpe_timer_update(gic, i);
        }
    }
}

uint32_t gic_get_sh_count(gic_t *gic)
{
    int i;
    if (gic->gic_gl_config & (1 << 28)) {
        return gic->gic_sh_counterlo;
    } else {
        uint64_t now;

        now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        for (i = 0; i < gic->num_cpu; i++) {
            if (timer_pending(gic->gic_timer[i].timer)
                && timer_expired(gic->gic_timer[i].timer, now)) {
                /* The timer has already expired.  */
    //            printf("!");
    //            fflush(0);

                gic_vpe_timer_expire(gic, i);
            }
        }

        return gic->gic_sh_counterlo +
            (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec());
    }
}


static void gic_vpe_timer_cb(void *opaque)
{
    gic_timer_t *gic_timer = opaque;
    gic_timer->gic->gic_sh_counterlo++;
    gic_vpe_timer_expire(gic_timer->gic, gic_timer->vp_index);
    gic_timer->gic->gic_sh_counterlo--;
}


void gic_timer_init(gic_t *gic, uint32_t ncpus)
{
    int i;
    gic->gic_timer = (void *) g_malloc0(sizeof(gic_timer_t) * ncpus);
    for (i = 0; i < ncpus; i++) {
        gic->gic_timer[i].gic = gic;
        gic->gic_timer[i].vp_index = i;
        gic->gic_timer[i].timer =
            timer_new_ns(QEMU_CLOCK_VIRTUAL, &gic_vpe_timer_cb,
                         &gic->gic_timer[i]);
    }
    gic_store_sh_count(gic, gic->gic_sh_counterlo);
}

static void gic_reset_register(gic_t *gic)
{
//    gic->gic_vpe_ctl[0] = (1 << 0x3) | (1 << 0x1);
    gic->gic_gl_config = 0x180f0000 | gic->num_cpu;
//    gic->gic_gl_config = 0x10030000;
//    gic->gic_gl_config = 0x100f0007;
//    gic->gic_vpe_mask[0] = 0x3e;
//    abort();
}

void gcr_init(uint32_t ncpus, CPUState *cs, MemoryRegion * address_space)
{

}

qemu_irq *
gic_init(uint32_t ncpus, CPUState *cs, MemoryRegion * address_space)
{
    gic_t *gic;
    uint32_t i;

    if (ncpus > NUMVPES) {
        fprintf(stderr, "Unable to initialize GIC - ncpus %d > NUMVPES!", ncpus);
        return NULL;
    }

    gic = (gic_t *) g_malloc0(sizeof(gic_t));


    gic->num_cpu = ncpus;

    gic_reset_register(gic);

    /* Register the CPU env for all cpus with the GIC */
    for (i = 0; i < ncpus; i++) {
        if (cs != NULL) {
            gic->env[i] = cs->env_ptr;
            cs = CPU_NEXT(cs);
        } else {
            fprintf(stderr, "Unable to initialize GIC - CPUMIPSState for CPU #%d not valid!", i);
            return NULL;
        }
    }

    /* Register GCR & GIC regions */
    memory_region_init_io(&gic->gcr_mem, NULL, &gcr_ops, gic, "GCR",
                          GCMP_ADDRSPACE_SZ);
    memory_region_init_io(&gic->gic_mem, NULL, &gic_ops, gic, "GIC",
                          GIC_ADDRSPACE_SZ);

    // GCMP
    /* The MIPS default location for the GCR_BASE address is 0x1FBF_8. */
    memory_region_add_subregion(address_space, GCMP_BASE_ADDR, &gic->gcr_mem);
    memory_region_add_subregion(address_space, GIC_BASE_ADDR, &gic->gic_mem);
//    memory_region_add_subregion(get_system_memory(), GIC_BASE_ADDR, &gic->gic_mem);

    qemu_register_reset(gic_reset, gic);

    gic->irqs = qemu_allocate_irqs(gic_set_irq, gic, GIC_NUM_INTRS);
//    gic_global_timer_init(gic);
    gic_timer_init(gic, ncpus);
    return (gic->irqs);
}
