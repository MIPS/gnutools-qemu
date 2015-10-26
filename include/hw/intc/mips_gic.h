/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000, 07 MIPS Technologies, Inc.
 * Copyright (C) 2015 Imagination Technologies
 *
 */
#ifndef _MIPS_GIC_H
#define _MIPS_GIC_H

/*
 * GIC Specific definitions
 */

/* The MIPS default location */
#define GIC_BASE_ADDR           0x1bdc0000ULL
#define GIC_ADDRSPACE_SZ        (128 * 1024)

/* GIC Address Space Offsets */
#define GIC_SHARED_BASE_ADDR    0x0000
#define GIC_VPLOCAL_BASE_ADDR  0x8000
#define GIC_VPOTHER_BASE_ADDR  0xC000
#define GIC_USERMODE_BASE_ADDR  0x10000

/* Constants */
#define GIC_POL_POS     1
#define GIC_POL_NEG     0
#define GIC_TRIG_EDGE   1
#define GIC_TRIG_LEVEL  0

#define MSK(n)              ((1 << (n)) - 1)

/* GIC Address Space */
#define SHARED_SECTION_OFS          0x0000
#define SHARED_SECTION_SIZE         0x8000
#define VP_LOCAL_SECTION_OFS       0x8000
#define VP_LOCAL_SECTION_SIZE      0x4000
#define VP_OTHER_SECTION_OFS       0xc000
#define VP_OTHER_SECTION_SIZE      0x4000
#define USM_VISIBLE_SECTION_OFS     0x10000
#define USM_VISIBLE_SECTION_SIZE    0x10000

/* Register Map for Shared Section */

#define GIC_SH_CONFIG_OFS           0x0000

/* Shared Global Counter */
#define GIC_SH_COUNTERLO_OFS        0x0010
#define GIC_SH_COUNTERHI_OFS        0x0014
#define GIC_SH_REVISIONID_OFS       0x0020

/* Interrupt Polarity */
#define GIC_SH_POL_31_0_OFS         0x0100
#define GIC_SH_POL_63_32_OFS        0x0104
#define GIC_SH_POL_95_64_OFS        0x0108
#define GIC_SH_POL_127_96_OFS       0x010c
#define GIC_SH_POL_159_128_OFS      0x0110
#define GIC_SH_POL_191_160_OFS      0x0114
#define GIC_SH_POL_223_192_OFS      0x0118
#define GIC_SH_POL_255_224_OFS      0x011c

/* Edge/Level Triggering */
#define GIC_SH_TRIG_31_0_OFS        0x0180
#define GIC_SH_TRIG_63_32_OFS       0x0184
#define GIC_SH_TRIG_95_64_OFS       0x0188
#define GIC_SH_TRIG_127_96_OFS      0x018c
#define GIC_SH_TRIG_159_128_OFS     0x0190
#define GIC_SH_TRIG_191_160_OFS     0x0194
#define GIC_SH_TRIG_223_192_OFS     0x0198
#define GIC_SH_TRIG_255_224_OFS     0x019c

/* Dual Edge Triggering */
#define GIC_SH_DUAL_31_0_OFS        0x0200
#define GIC_SH_DUAL_63_32_OFS       0x0204
#define GIC_SH_DUAL_95_64_OFS       0x0208
#define GIC_SH_DUAL_127_96_OFS      0x020c
#define GIC_SH_DUAL_159_128_OFS     0x0210
#define GIC_SH_DUAL_191_160_OFS     0x0214
#define GIC_SH_DUAL_223_192_OFS     0x0218
#define GIC_SH_DUAL_255_224_OFS     0x021c

/* Set/Clear corresponding bit in Edge Detect Register */
#define GIC_SH_WEDGE_OFS            0x0280

/* Reset Mask - Disables Interrupt */
#define GIC_SH_RMASK_31_0_OFS       0x0300
#define GIC_SH_RMASK_63_32_OFS      0x0304
#define GIC_SH_RMASK_95_64_OFS      0x0308
#define GIC_SH_RMASK_127_96_OFS     0x030c
#define GIC_SH_RMASK_159_128_OFS    0x0310
#define GIC_SH_RMASK_191_160_OFS    0x0314
#define GIC_SH_RMASK_223_192_OFS    0x0318
#define GIC_SH_RMASK_255_224_OFS    0x031c

/* Set Mask (WO) - Enables Interrupt */
#define GIC_SH_SMASK_31_0_OFS       0x0380
#define GIC_SH_SMASK_63_32_OFS      0x0384
#define GIC_SH_SMASK_95_64_OFS      0x0388
#define GIC_SH_SMASK_127_96_OFS     0x038c
#define GIC_SH_SMASK_159_128_OFS    0x0390
#define GIC_SH_SMASK_191_160_OFS    0x0394
#define GIC_SH_SMASK_223_192_OFS    0x0398
#define GIC_SH_SMASK_255_224_OFS    0x039c

/* Global Interrupt Mask Register (RO) - Bit Set == Interrupt enabled */
#define GIC_SH_MASK_31_0_OFS        0x0400
#define GIC_SH_MASK_63_32_OFS       0x0404
#define GIC_SH_MASK_95_64_OFS       0x0408
#define GIC_SH_MASK_127_96_OFS      0x040c
#define GIC_SH_MASK_159_128_OFS     0x0410
#define GIC_SH_MASK_191_160_OFS     0x0414
#define GIC_SH_MASK_223_192_OFS     0x0418
#define GIC_SH_MASK_255_224_OFS     0x041c

/* Pending Global Interrupts (RO) */
#define GIC_SH_PEND_31_0_OFS        0x0480
#define GIC_SH_PEND_63_32_OFS       0x0484
#define GIC_SH_PEND_95_64_OFS       0x0488
#define GIC_SH_PEND_127_96_OFS      0x048c
#define GIC_SH_PEND_159_128_OFS     0x0490
#define GIC_SH_PEND_191_160_OFS     0x0494
#define GIC_SH_PEND_223_192_OFS     0x0498
#define GIC_SH_PEND_255_224_OFS     0x049c

#define GIC_SH_MAP0_PIN_OFS         0x0500
#define GIC_SH_MAP255_PIN_OFS       0x08fc

#define GIC_SH_MAP0_VP31_0_OFS     0x2000
#define GIC_SH_MAP255_VP63_32_OFS  0x3fe4

/* Register Map for Local Section */
#define GIC_VP_CTL_OFS                 0x0000
#define GIC_VP_PEND_OFS                0x0004
#define GIC_VP_MASK_OFS                0x0008
#define GIC_VP_RMASK_OFS               0x000c
#define GIC_VP_SMASK_OFS               0x0010
#define GIC_VP_WD_MAP_OFS              0x0040
#define GIC_VP_COMPARE_MAP_OFS         0x0044
#define GIC_VP_TIMER_MAP_OFS           0x0048
#define GIC_VP_PERFCTR_MAP_OFS         0x0050
#define GIC_VP_SWINT0_MAP_OFS          0x0054
#define GIC_VP_SWINT1_MAP_OFS          0x0058
#define GIC_VP_OTHER_ADDR_OFS          0x0080
#define GIC_VP_IDENT_OFS               0x0088
#define GIC_VP_WD_CONFIG0_OFS          0x0090
#define GIC_VP_WD_COUNT0_OFS           0x0094
#define GIC_VP_WD_INITIAL0_OFS         0x0098
#define GIC_VP_COMPARE_LO_OFS          0x00a0
#define GIC_VP_COMPARE_HI_OFS          0x00a4
#define GIC_VL_BRK_GROUP                0x3080

/* User-Mode Visible Section Register */
/* Read-only alias for GIC Shared CounterLo */
#define GIC_USER_MODE_COUNTERLO         0x0000
/* Read-only alias for GIC Shared CounterHi */
#define GIC_USER_MODE_COUNTERHI         0x0004

/* Masks */
#define GIC_SH_CONFIG_COUNTSTOP_SHF     28
#define GIC_SH_CONFIG_COUNTSTOP_MSK     (MSK(1) << GIC_SH_CONFIG_COUNTSTOP_SHF)

#define GIC_SH_CONFIG_COUNTBITS_SHF     24
#define GIC_SH_CONFIG_COUNTBITS_MSK     (MSK(4) << GIC_SH_CONFIG_COUNTBITS_SHF)

#define GIC_SH_CONFIG_NUMINTRS_SHF      16
#define GIC_SH_CONFIG_NUMINTRS_MSK      (MSK(8) << GIC_SH_CONFIG_NUMINTRS_SHF)

#define GIC_SH_CONFIG_NUMVPS_SHF       0
#define GIC_SH_CONFIG_NUMVPS_MSK       (MSK(8) << GIC_SH_CONFIG_NUMVPS_SHF)

#define GIC_SH_WEDGE_RW_SHF             31
#define GIC_SH_WEDGE_RW_MSK             (MSK(1) << GIC_SH_WEDGE_RW_SHF)

#define GIC_MAP_TO_PIN_SHF              31
#define GIC_MAP_TO_PIN_MSK              (MSK(1) << GIC_MAP_TO_PIN_SHF)
#define GIC_MAP_TO_NMI_SHF              30
#define GIC_MAP_TO_NMI_MSK              (MSK(1) << GIC_MAP_TO_NMI_SHF)
#define GIC_MAP_TO_YQ_SHF               29
#define GIC_MAP_TO_YQ_MSK               (MSK(1) << GIC_MAP_TO_YQ_SHF)
#define GIC_MAP_SHF                     0
#define GIC_MAP_MSK                     (MSK(6) << GIC_MAP_SHF)
#define GIC_MAP_TO_PIN_REG_MSK          \
    (GIC_MAP_TO_PIN_MSK | GIC_MAP_TO_NMI_MSK | GIC_MAP_TO_YQ_MSK | GIC_MAP_MSK)

/* GIC_VP_CTL Masks */
#define GIC_VP_CTL_PERFCNT_RTBL_SHF    2
#define GIC_VP_CTL_PERFCNT_RTBL_MSK    (MSK(1) << GIC_VP_CTL_PERFCNT_RTBL_SHF)
#define GIC_VP_CTL_TIMER_RTBL_SHF      1
#define GIC_VP_CTL_TIMER_RTBL_MSK      (MSK(1) << GIC_VP_CTL_TIMER_RTBL_SHF)
#define GIC_VP_CTL_EIC_MODE_SHF        0
#define GIC_VP_CTL_EIC_MODE_MSK        (MSK(1) << GIC_VP_CTL_EIC_MODE_SHF)

/* GIC_VP_MASK Masks */
#define GIC_VP_MASK_WD_SHF         0
#define GIC_VP_MASK_WD_MSK         (MSK(1) << GIC_VP_MASK_WD_SHF)
#define GIC_VP_MASK_CMP_SHF        1
#define GIC_VP_MASK_CMP_MSK        (MSK(1) << GIC_VP_MASK_CMP_SHF)
#define GIC_VP_MASK_TIMER_SHF      2
#define GIC_VP_MASK_TIMER_MSK      (MSK(1) << GIC_VP_MASK_TIMER_SHF)
#define GIC_VP_MASK_PERFCNT_SHF    3
#define GIC_VP_MASK_PERFCNT_MSK    (MSK(1) << GIC_VP_MASK_PERFCNT_SHF)
#define GIC_VP_MASK_SWINT0_SHF     4
#define GIC_VP_MASK_SWINT0_MSK     (MSK(1) << GIC_VP_MASK_SWINT0_SHF)
#define GIC_VP_MASK_SWINT1_SHF     5
#define GIC_VP_MASK_SWINT1_MSK     (MSK(1) << GIC_VP_MASK_SWINT1_SHF)
#define GIC_VP_MASK_FDC_SHF        6
#define GIC_VP_MASK_FDC_MSK        (MSK(1) << GIC_VP_MASK_FDC_SHF)
#define GIC_VP_SET_RESET_MSK       (MSK(7) << GIC_VP_MASK_WD_SHF)

#define GIC_CPU_PIN_OFFSET          2

#define TYPE_MIPS_GIC "mips-gic"
#define MIPS_GIC(obj) OBJECT_CHECK(MIPSGICState, (obj), TYPE_MIPS_GIC)

/* Support up to 32 VPs and 256 IRQs */
#define GIC_MAX_VPS             32
#define GIC_MAX_INTRS           256

typedef struct MIPSGICState MIPSGICState;
typedef struct MIPSGICTimerState MIPSGICTimerState;

struct MIPSGICTimerState {
    QEMUTimer *qtimer;
    uint32_t vp_index;
    MIPSGICState *gic;
};

typedef struct MIPSGICIRQState {
    bool enabled;
    bool pending;
    bool polarity;
    bool trigger_type;
    bool dual_edge;
    uint32_t map_pin;
    int32_t map_vp;
    qemu_irq irq;
} MIPSGICIRQState;

typedef struct MIPSGICVPState {
    uint32_t ctl;
    uint32_t pend;
    uint32_t mask;
    uint32_t wd_map;
    uint32_t compare_map;
    uint32_t timer_map;
    uint32_t comparelo;
    uint32_t comparehi;
    uint32_t other_addr;

    CPUMIPSState *env;
    MIPSGICTimerState *gic_timer;
} MIPSGICVPState;

struct MIPSGICState {
    SysBusDevice parent_obj;

    MemoryRegion gic_mem;

    /* Shared Section Registers */
    uint32_t sh_config;
    uint32_t sh_counterlo;
    MIPSGICIRQState *irq_state;

    /* VP Local/Other Section Registers */
    MIPSGICVPState *vps;

    int32_t num_vps;
    int32_t num_irq;
};

#endif /* _MIPS_GIC_H */
