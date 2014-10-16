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

#ifdef DEBUG
#define DPRINTF(fmt, ...) fprintf(stderr, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define DPRINTF(fmt, ...)
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
    uint32_t gic_gl_intr_pol_reg[8];
    uint32_t gic_gl_intr_trigtype_reg[8];
    uint32_t gic_gl_intr_pending_reg[8];
    uint32_t gic_gl_intr_mask_reg[8];

    uint32_t gic_gl_map_pin[256];

    /* Sparse array, need a better way */
    uint32_t gic_gl_map_vpe[0x7fa];

    /* VPE Local Section Registers */
    /* VPE Other Section Registers, aliased to local, use the other field to access the correct instance */
    uint32_t gic_local_vpe_regs[NUMVPES][0x1000];

    /* User Mode Visible Section Registers */
} gic_t;


static uint64_t
gic_read(void *opaque, hwaddr addr, unsigned size)
{
    int reg;
    gic_t *gic = (gic_t *) opaque;

    DPRINTF("addr: %#" PRIx64 ", size: %#x\n", addr, size);

    switch (addr) {
    case GIC_SH_CONFIG_OFS:
        return 0x8040000 | ((NUMVPES - 1) & GIC_SH_CONFIG_NUMVPES_MSK);
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
        return gic->gic_gl_intr_pol_reg[reg];
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
        return gic->gic_gl_intr_trigtype_reg[reg];
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
        DPRINTF("pending[%d]: %#" PRIx32 "\n", reg, gic->gic_gl_intr_pending_reg[reg]);
        return gic->gic_gl_intr_pending_reg[reg];
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
        return gic->gic_gl_intr_mask_reg[reg];
        break;

    default:
        break;
    }

    /* Other cases */
    if (addr >= GIC_SH_INTR_MAP_TO_PIN_BASE_OFS
        && addr <= GIC_SH_MAP_TO_PIN(255)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_PIN_BASE_OFS) / 4;
        return gic->gic_gl_map_pin[reg];
    }

    if (addr >= GIC_SH_INTR_MAP_TO_VPE_BASE_OFS
        && addr <= GIC_SH_MAP_TO_VPE_REG_OFF(255, 63)) {
        reg = (addr - GIC_SH_INTR_MAP_TO_VPE_BASE_OFS) / 4;
        return gic->gic_gl_map_vpe[reg];
    }

    if (addr >= GIC_VPELOCAL_BASE_ADDR && addr < GIC_VPEOTHER_BASE_ADDR) {
    }

    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
    }

    DPRINTF("%s: unimplemented register @ %#" PRIx64 "\n", __func__, addr);
    return 0ULL;
}

static void
gic_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    int reg, intr;
    gic_t *gic = (gic_t *) opaque;

    DPRINTF("addr: %#" PRIx64 ", data: %#" PRIx64 ", size: %#x\n", addr, data,
            size);

    switch (addr) {
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
    }

    if (addr >= GIC_VPEOTHER_BASE_ADDR && addr < GIC_USERMODE_BASE_ADDR) {
    }
}

static uint64_t
gcr_read(void *opaque, hwaddr addr, unsigned size)
{
    gic_t *gic = (gic_t *) opaque;

    DPRINTF("addr: %#" PRIx64 ", size: %#x\n", addr, size);

    switch (addr) {
    case GCMP_GCB_GC_OFS:
        /* Set PCORES to # cores - 1 */
        return smp_cpus - 1;
        break;

    case GCMP_GCB_GCMPB_OFS:
        return GCMP_BASE_ADDR;
        break;

    case GCMP_GCB_GICBA_OFS:
        return gic->gcr_gic_base_reg;
        break;

    default:
        DPRINTF("Unsupported Reg Read @ offset %#" PRIx64 "\n", addr);
        return 0;
    }

    return 0ULL;
}

static void
gcr_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    gic_t *gic = (gic_t *) opaque;

    DPRINTF("addr: %#" PRIx64 ", data: %#" PRIx64 ", size: %#x\n", addr, data,
            size);

    switch (addr) {
    case GCMP_GCB_GICBA_OFS:
        gic->gcr_gic_base_reg = data;
        break;

    default:
        break;
    }
}



static void
gic_set_irq(void *opaque, int n_IRQ, int level)
{
    int vpe = -1, pin = -1, i;
    gic_t *gic = (gic_t *) opaque;

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

    return (gic->irqs);
}
