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
#include "hw/sysbus.h"
#include "sysemu/sysemu.h"
#include "hw/mips/mips_gcr.h"
#include "hw/mips/mips_gic.h"

/* #define DEBUG */

#ifdef DEBUG
#define DPRINTF(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define DPRINTF(fmt, ...)
#endif

/* Read GCR registers */
static uint64_t gcr_read(void *opaque, hwaddr addr, unsigned size)
{
    MIPSGCRState *gcr = (MIPSGCRState *) opaque;

    DPRINTF("Info read %d bytes at GCR offset 0x%" PRIx64 " (GCR) -> ",
            size, addr);

    switch (addr) {
    /* Global Control Block Register */
    case GCR_CONFIG_OFS:
        /* Set PCORES to 0 */
        DPRINTF("0x%016x\n", 0);
        return 0;
    case GCR_BASE_OFS:
        DPRINTF("GCMP_BASE_ADDR: %016llx\n", GCR_BASE_ADDR);
        return GCR_BASE_ADDR;
    case GCR_REV_OFS:
        DPRINTF("0x%016x\n", gcr->gcr_rev);
        return gcr->gcr_rev;
    case GCR_GIC_BASE_OFS:
        DPRINTF("0x" TARGET_FMT_lx "\n", GIC_BASE_ADDR);
        return GIC_BASE_ADDR;
    case GCR_GIC_STATUS_OFS:
        DPRINTF("0x%016x\n", GCR_GIC_STATUS_GICEX_MSK);
        return GCR_GIC_STATUS_GICEX_MSK;
    case GCR_CPC_STATUS_OFS:
        DPRINTF("0x%016x\n", 0);
        return 0;
    case GCR_L2_CONFIG_OFS:
        /* L2 BYPASS */
        DPRINTF("0x%016x\n", GCR_L2_CONFIG_BYPASS_MSK);
        return GCR_L2_CONFIG_BYPASS_MSK;

        /* Core-Local and Core-Other Control Blocks */
    case MIPS_CLCB_OFS + GCR_CL_CONFIG_OFS:
    case MIPS_COCB_OFS + GCR_CL_CONFIG_OFS:
        /* Set PVP to # cores - 1 */
        DPRINTF("0x%016x\n", smp_cpus - 1);
        return smp_cpus - 1;
    case MIPS_CLCB_OFS + GCR_CL_OTHER_OFS:
        DPRINTF("0x%016x\n", 0);
        return 0;

    default:
        DPRINTF("Warning *** unimplemented GCR read at offset 0x%" PRIx64 "\n",
                addr);
        return 0;
    }
    return 0ULL;
}

/* Write GCR registers */
static void gcr_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    /*
     * MIPSGCRState *gcr = (MIPSGCRState *) opaque;
     * */

    switch (addr) {
    case GCR_GIC_BASE_OFS:
        DPRINTF("Info write %d bytes at GCR offset %" PRIx64 " <- 0x%016lx\n",
                size, addr, data);
        break;
    default:
        DPRINTF("Warning *** unimplemented GCR write at offset 0x%" PRIx64 "\n",
                addr);
        break;
    }
}

static const MemoryRegionOps gcr_ops = {
    .read = gcr_read,
    .write = gcr_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {
        .max_access_size = 8,
    },
};

static void mips_gcr_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    MIPSGCRState *s = MIPS_GCR(obj);

    memory_region_init_io(&s->gcr_mem, OBJECT(s), &gcr_ops, s,
                          "mips-gcr", GCR_ADDRSPACE_SZ);
    sysbus_init_mmio(sbd, &s->gcr_mem);
}

static Property mips_gcr_properties[] = {
    DEFINE_PROP_INT32("num-cpu", MIPSGCRState, num_cpu, 1),
    DEFINE_PROP_INT32("gcr-rev", MIPSGCRState, gcr_rev, 0x800),
    DEFINE_PROP_END_OF_LIST(),
};

static void mips_gcr_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->props = mips_gcr_properties;
}

static const TypeInfo mips_gcr_info = {
    .name          = TYPE_MIPS_GCR,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(MIPSGCRState),
    .instance_init = mips_gcr_init,
    .class_init    = mips_gcr_class_init,
};

static void mips_gcr_register_types(void)
{
    type_register_static(&mips_gcr_info);
}

type_init(mips_gcr_register_types)
