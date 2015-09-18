/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012  MIPS Technologies, Inc.  All rights reserved.
 * Authors: Sanjay Lal <sanjayl@kymasys.com>
*/

#include "hw/hw.h"
#include "qemu/bitmap.h"
#include "exec/memory.h"
#include "sysemu/sysemu.h"

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
#define NUMVPES     4

/* XXXKYMA: Spoof a bit of the GCR as well, just enough to get Linux to detect it */
typedef struct gic_t
{
    CPUMIPSState *env[NR_CPUS];
    MemoryRegion gcr_mem, gic_mem;

    qemu_irq *irqs;

    /* GCR Registers */
    uint32_t gcr_gic_base_reg;

    /* Shared Section Registers */
    uint32_t gic_gl_config;
    uint32_t gic_gl_intr_pol_reg[8];
    uint32_t gic_gl_intr_trigtype_reg[8];
    uint32_t gic_gl_intr_pending_reg[8];
    uint32_t gic_gl_intr_mask_reg[8];

    uint32_t gic_gl_map_pin[256];

    /* Sparse array, need a better way */
    uint32_t gic_gl_map_vpe[0x7fa];

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
} gic_t;


// prototypes
uint32_t gic_get_count(gic_t *gic);
static uint32_t gic_timer_update(gic_t *gic);
static void gic_set_irq(void *opaque, int n_IRQ, int level);

static uint64_t
gic_read(void *opaque, hwaddr addr, unsigned size)
{
    int reg;
    gic_t *gic = (gic_t *) opaque;
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
        ret = gic_get_count(gic);
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

    /* Other cases */
    if (addr >= GIC_SH_INTR_MAP_TO_PIN_BASE_OFS
        && addr <= GIC_SH_MAP_TO_PIN(255)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) / 4;
        ret = gic->gic_gl_map_pin[reg];
        DPRINTF("(GIC) -> 0x%016x\n", ret);
        return ret;
    }

    if (addr >= GIC_SH_INTR_MAP_TO_VPE_BASE_OFS
        && addr <= GIC_SH_MAP_TO_VPE_REG_OFF(255, 63)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_VPE_BASE_OFS) / 4;
        ret = gic->gic_gl_map_vpe[reg];
        DPRINTF("(GIC) -> 0x%016x\n", ret);
        return ret;
    }

    if (addr >= GIC_VPELOCAL_BASE_ADDR && addr < GIC_VPEOTHER_BASE_ADDR) {
        switch (addr - GIC_VPELOCAL_BASE_ADDR){
        case GIC_VPE_CTL_OFS:
            DPRINTF("(GIC_VPE_CTL) -> 0x%016x\n", gic->gic_vpe_ctl[0]);
            return gic->gic_vpe_ctl[0];
        case GIC_VPE_PEND_OFS:
//            printf("!");
//            fflush(0);
            gic_get_count(gic);
            DPRINTF("(GIC_VPE_PEND) -> 0x%016x\n", gic->gic_vpe_pend[0]);
            return gic->gic_vpe_pend[0];
        case GIC_VPE_MASK_OFS:
            DPRINTF("(GIC_VPE_MASK) -> 0x%016x\n", gic->gic_vpe_mask[0]);
            return gic->gic_vpe_mask[0];
        case GIC_VPE_OTHER_ADDR_OFS:
            DPRINTF("(GIC_VPE_OTHER_ADDR) -> 0x%016x\n", gic->gic_vpe_other_addr[0]);
            return gic->gic_vpe_other_addr[0];
        case GIC_VPE_COMPARE_LO_OFS:
            DPRINTF("(GIC_VPE_COMPARELO) -> 0x%016x\n", gic->gic_vpe_comparelo[0]);
            return gic->gic_vpe_comparelo[0];
        case GIC_VPE_COMPARE_HI_OFS:
            DPRINTF("(GIC_VPE_COMPAREhi) -> 0x%016x\n", gic->gic_vpe_comparehi[0]);
            return gic->gic_vpe_comparehi[0];
        default:
            SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** read %d bytes at GIC offset 0x%04x \n", size, addr);
            break;
        }

    }

    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
    }

//    DPRINTF("%s: unimplemented register @ %#" PRIx64 "\n", __func__, addr);
    return 0ULL;
}

static void
gic_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    int reg, intr;
    gic_t *gic = (gic_t *) opaque;

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
                timer_mod(gic->timer, muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL), get_ticks_per_sec(), TIMER_FREQ));
            }
            if ((pre & 0x10000000) && !(gic->gic_gl_config & 0x10000000)) {
                DPRINTF("Info (MIPS64_CMP) GIC_SH_CONFIG.COUNTSTOP modified STARTING\n");
                timer_mod(gic->timer, muldiv64(1, get_ticks_per_sec(), TIMER_FREQ));
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
        switch(addr - 0x8000) {
        case GIC_VPE_CTL_OFS:
            gic->gic_vpe_ctl[0] &= ~1;
            gic->gic_vpe_ctl[0] |= data & 1;
//            DPRINTF("QEMU: GIC Local interrupt control reg WRITE %x\n", data);
            break;
        case GIC_VPE_OTHER_ADDR_OFS:
            gic->gic_vpe_other_addr[0] = data;
//            DPRINTF("QEMU: GIC other addressing reg WRITE %x\n", data);
            if (data != 0) {
//                DPRINTF("QEMU: GIC Other addressing OOPS\n");
            }
            break;
        case GIC_VPE_OTHER_ADDR_OFS + 4:
        // do nothing
            break;
        case GIC_VPE_SMASK_OFS:
            DPRINTF("QEMU Local SMASK org_mask %x, ", gic->gic_vpe_mask[0]);
            gic->gic_vpe_mask[0] |= (data & 0x3f);
            DPRINTF("data %x, mask %x\n", data, gic->gic_vpe_mask[0]);
            break;
        case GIC_VPE_COMPARE_LO_OFS:
            gic->gic_vpe_comparelo[0] = (uint32_t) data;
            if (!(gic->gic_gl_config & 0x10000000)) {
DPRINTF("GIC Compare modified (GIC_VPE_Compare=0x%x GIC_Counter=0x%x) - schedule CMP timer interrupt after 0x%x\n",
        gic->gic_vpe_comparelo[0], gic->gic_sh_counterlo, gic_timer_update(gic));
//                gic_timer_update(gic);
            }
            gic->gic_vpe_pend[0] &= ~(1 << 1);
            if (gic->gic_vpe_compare_map[0] & 0x80000000) {
                printf("~%d", gic->gic_vpe_compare_map[0] & 0x3F);
                fflush(0);
//                gic_set_irq(gic, gic->gic_vpe_compare_map[0] & 0x3F, 0);
                qemu_set_irq(gic->env[0]->irq[(gic->gic_vpe_compare_map[0] & 0x3F)+2], 0);
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

    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
        switch(addr - 0xc000) {
        case GIC_VPE_RMASK_OFS:
            DPRINTF("QEMU Other RMASK org_mask %x, ", gic->gic_vpe_mask[0]);
            gic->gic_vpe_mask[0] &= ~(data & 0x3f) & 0x3f;
            DPRINTF("data %x, mask %x\n", data, gic->gic_vpe_mask[0]);
            break;
        case GIC_VPE_SMASK_OFS:
            DPRINTF("QEMU Other SMASK org_mask %x, ", gic->gic_vpe_mask[0]);
            gic->gic_vpe_mask[0] |= (data & 0x3f);
            DPRINTF("data %x, mask %x\n", data, gic->gic_vpe_mask[0]);
            break;
        case GIC_VPE_WD_MAP_OFS:
            gic->gic_vpe_wd_map[0] = data & 0xE000003F;
            break;
        case GIC_VPE_COMPARE_MAP_OFS:
            gic->gic_vpe_compare_map[0] = data & 0xE000003F;
            DPRINTF("QEMU: GIC COMPARE MAP %x %x\n", data, gic->gic_vpe_compare_map[0]);
            break;
        case GIC_VPE_TIMER_MAP_OFS:
            gic->gic_vpe_timer_map[0] = data & 0xE000003F;
            DPRINTF("QEMU: GIC Timer MAP %x %x\n", data, gic->gic_vpe_timer_map[0]);
            break;
        case GIC_VPE_COMPARE_LO_OFS:
            gic->gic_vpe_comparelo[0] = (uint32_t) data;
            if (!(gic->gic_gl_config & 0x10000000)) {
DPRINTF("GIC Compare modified (GIC_VPE_Compare=0x%x GIC_Counter=0x%x) - schedule CMP timer interrupt after 0x%x\n",
        gic->gic_vpe_comparelo[0], gic->gic_sh_counterlo, gic_timer_update(gic));
//                gic_timer_update(gic);
            }
            gic->gic_vpe_pend[0] &= ~(1 << 1);
            if (gic->gic_vpe_compare_map[0] & 0x80000000) {
                printf("~%d", gic->gic_vpe_compare_map[0] & 0x3F);
                                fflush(0);
//                gic_set_irq(gic, gic->gic_vpe_compare_map[0] & 0x3F, 0);
                qemu_set_irq(gic->env[0]->irq[(gic->gic_vpe_compare_map[0] & 0x3F)+2], 0);
            }
            break;
        case GIC_VPE_COMPARE_HI_OFS:
            break;
        default:
//            DPRINTF("QEMU: not supported GIC Other WRITE request addr: %#" PRIx64 ", %lx\n", addr, data);
            SPRINTF("Warning (MIPS64_CMP) I6400_CPU0 *** write %d bytes at GIC offset 0x%04x 0x%08lx\n", size, addr, data);
            break;
        }

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
        DPRINTF("0x%016lx\n", gic->gcr_gic_base_reg);
        return gic->gcr_gic_base_reg;
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
        gic->gcr_gic_base_reg = data;
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

    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    wait = gic->gic_vpe_comparelo[0] - gic->gic_sh_counterlo -
        (uint32_t)muldiv64(now, TIMER_FREQ, get_ticks_per_sec());
    next = now + muldiv64(wait, get_ticks_per_sec(), TIMER_FREQ);
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
static void gic_timer_expire(gic_t *gic)
{
    gic_timer_update(gic);
//    if (env->insn_flags & ISA_MIPS32R2) {
//        env->CP0_Cause |= 1 << CP0Ca_TI;
//    }
//    qemu_irq_raise(env->irq[(env->CP0_IntCtl >> CP0IntCtl_IPTI) & 0x7]);
    if (!gic->gic_vpe_comparelo[0] && !gic->gic_vpe_comparehi[0]) {
//        DPRINTF("ignored--- gic timer expire\n");
        return;
    }
    gic->gic_vpe_pend[0] |= (1 << 1);
    DPRINTF("gic_timer_expire pend0 %x", gic->gic_vpe_pend[0]);

    print_gic_status(gic);

    if (gic->gic_vpe_pend[0] & (gic->gic_vpe_mask[0] & (1 << 1))) {
        if (gic->gic_vpe_compare_map[0] & 0x80000000) {
//            DPRINTF("QEMU GIC TIMER set IRQ%d\n", gic->gic_vpe_compare_map[0]);
            printf("~%d", gic->gic_vpe_compare_map[0] & 0x3F);
//            printf("T%d", (gic->env[0]->CP0_IntCtl >> CP0IntCtl_IPTI) & 0x7);
            fflush(0);
//            gic_set_irq(gic, gic->gic_vpe_compare_map[0] & 0x3F, 1);
            qemu_set_irq(gic->env[0]->irq[(gic->gic_vpe_compare_map[0] & 0x3F)+2], 1);
        }
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
#if 0 || defined(MIPSSIM_COMPAT)
    qemu_log("%s\n", __func__);
#endif

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

void gic_sh_timer_init (gic_t *gic)
{
    gic->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, &gic_timer_cb, gic);
    if (!(gic->gic_gl_config & 0x10000000)) {
        timer_mod(gic->timer, muldiv64(1, get_ticks_per_sec(), TIMER_FREQ));
    }
}

static void gic_reset_register(gic_t *gic)
{
//    gic->gic_vpe_ctl[0] = (1 << 0x3) | (1 << 0x1);
    gic->gic_gl_config = 0x180f0000;
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
    uint32_t x;

    if (ncpus > NUMVPES) {
        fprintf(stderr, "Unable to initialize GIC - ncpus %d > NUMVPES!", ncpus);
        return NULL;
    }

    gic = (gic_t *) g_malloc0(sizeof(gic_t));

    gic_reset_register(gic);

    /* Register the CPU env for all cpus with the GIC */
    for (x = 0; x < ncpus; x++) {
        if (cs != NULL) {
            gic->env[x] = cs->env_ptr;
            cs = CPU_NEXT(cs);
        } else {
            fprintf(stderr, "Unable to initialize GIC - CPUMIPSState for CPU #%d not valid!", x);
            return NULL;
        }
    }

    /* Register GCR & GIC regions */
    memory_region_init_io(&gic->gcr_mem, NULL, &gcr_ops, gic, "GCR",
                          GCMP_ADDRSPACE_SZ);
    memory_region_init_io(&gic->gic_mem, NULL, &gic_ops, gic, "GIC",
                          GIC_ADDRSPACE_SZ);

    memory_region_add_subregion(address_space, GCMP_BASE_ADDR, &gic->gcr_mem);
    memory_region_add_subregion(address_space, GIC_BASE_ADDR, &gic->gic_mem);

    qemu_register_reset(gic_reset, gic);

    gic->irqs = qemu_allocate_irqs(gic_set_irq, gic, GIC_NUM_INTRS);
    gic_sh_timer_init(gic);

    printf("GIC INIT : Cause %x\n", gic->env[0]->CP0_Cause);
    return (gic->irqs);
}
