/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2017 Imagination Technologies
 *
 */

#ifndef MIPS_GICWDTIMER_H
#define MIPS_GICWDTIMER_H

typedef struct MIPSGICWDTimerVPState MIPSGICWDTimerVPState;
typedef struct MIPSGICWDTimerState MIPSGICWDTimerState;

typedef void MIPSGICWDTimerCB(void *opaque, uint32_t vp_index);

struct MIPSGICWDTimerVPState {
    QEMUTimer *qtimer;
    uint32_t vp_index;
    uint32_t count;
    uint32_t startcount;
    uint32_t config;
    uint64_t expirytime;
    MIPSGICWDTimerState *gicwdtimer;
};

struct MIPSGICWDTimerState {
    void *opaque;
    int32_t num_vps;
    MIPSGICWDTimerVPState *vpwdtimers;
    MIPSGICWDTimerCB *cb;
};

uint32_t mips_gicwdtimer_get_freq(MIPSGICWDTimerState *gic);
uint32_t mips_gicwdtimer_get_vp_startcount(MIPSGICWDTimerState *gictimer,
                                           uint32_t vp_index);
void mips_gicwdtimer_store_vp_startcount(MIPSGICWDTimerState *gic,
                                         uint32_t vp_index, uint32_t startcount);

uint32_t mips_gicwdtimer_get_vp_count(MIPSGICWDTimerState *gicwdtimer,
                                      uint32_t vp_index);

uint32_t mips_gicwdtimer_get_config(MIPSGICWDTimerState *gic, uint32_t vp_index);
void mips_gicwdtimer_store_config(MIPSGICWDTimerState *gic,
                                  uint32_t vp_index, uint32_t data);

MIPSGICWDTimerState *mips_gicwdtimer_init(void *opaque, uint32_t nvps,
                                          MIPSGICWDTimerCB *cb);

#endif /* MIPS_GICWDTIMER_H */
