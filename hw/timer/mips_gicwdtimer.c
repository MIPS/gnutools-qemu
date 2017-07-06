/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2017 Imagination Technologies
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/hw.h"
#include "hw/sysbus.h"
#include "qemu/timer.h"
#include "sysemu/sysemu.h"
#include "hw/timer/mips_gicwdtimer.h"

#define GIC_WDT_CONFIG_MASK     (0x1F)
#define GIC_WDT_CONFIG_RESET    (1 << 7)
#define GIC_WDT_CONFIG_WDINTR   (1 << 6)
#define  GIC_WDT_CONFIG_ONESHOT 0
#define  GIC_WDT_CONFIG_SECOND_COUNTDOWN 1
#define  GIC_WDT_CONFIG_PIT     2
#define GIC_WDT_CONFIG_START    (1 << 0)

#define TIMER_PERIOD 10 /* 10 ns period for 100 Mhz frequency */

uint32_t mips_gicwdtimer_get_freq(MIPSGICWDTimerState *gic)
{
    return NANOSECONDS_PER_SECOND / TIMER_PERIOD;
}

static void gic_vpwdtimer_start(MIPSGICWDTimerVPState *vpwdtimer)
{
    uint64_t now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) / TIMER_PERIOD;
    uint64_t next = now + vpwdtimer->startcount;

    vpwdtimer->expirytime = next;
    timer_mod(vpwdtimer->qtimer, next * TIMER_PERIOD);
}

static void gic_vpwdtimer_cb(void *opaque)
{
    MIPSGICWDTimerVPState *vpwdtimer = opaque;

    if (!(vpwdtimer->config & 1)) {
        /* Timer stopped */
        return;
    }

    switch((vpwdtimer->config >> 1) & 7) {
        case GIC_WDT_CONFIG_SECOND_COUNTDOWN:
            if (vpwdtimer->config & GIC_WDT_CONFIG_WDINTR) {
                /* Previous interrupt went unacknowledged - reset! */
                vpwdtimer->config |= GIC_WDT_CONFIG_RESET;
                qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
            }
            /* Fall through */
        case GIC_WDT_CONFIG_PIT:
            gic_vpwdtimer_start(vpwdtimer);
            break;
        default:
            break;
    }

    /* Flag interrupt occured */
    vpwdtimer->config |= GIC_WDT_CONFIG_WDINTR;

    vpwdtimer->gicwdtimer->cb(vpwdtimer->gicwdtimer->opaque, vpwdtimer->vp_index);
}

uint32_t mips_gicwdtimer_get_vp_count(MIPSGICWDTimerState *gicwdtimer,
                                      uint32_t vp_index)
{
    MIPSGICWDTimerVPState *vpwdtimer = &gicwdtimer->vpwdtimers[vp_index];
    uint64_t now;

    if (!timer_pending(vpwdtimer->qtimer))
        return 0;

    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    return vpwdtimer->expirytime - (now / TIMER_PERIOD);
}

void mips_gicwdtimer_store_vp_startcount(MIPSGICWDTimerState *gicwdtimer,
                                         uint32_t vp_index, uint32_t startcount)
{
    MIPSGICWDTimerVPState *vpwdtimer = &gicwdtimer->vpwdtimers[vp_index];
    vpwdtimer->startcount = startcount;
}

void mips_gicwdtimer_store_config(MIPSGICWDTimerState *gicwdtimer,
                                  uint32_t vp_index, uint32_t data)
{
    MIPSGICWDTimerVPState *vpwdtimer = &gicwdtimer->vpwdtimers[vp_index];

    if ((vpwdtimer->config ^ data) & GIC_WDT_CONFIG_START) {
        if (data & GIC_WDT_CONFIG_START)
            gic_vpwdtimer_start(vpwdtimer);
        else
            timer_del(vpwdtimer->qtimer);
    }
    data = data & GIC_WDT_CONFIG_MASK;
    vpwdtimer->config = data & (~GIC_WDT_CONFIG_WDINTR & ~GIC_WDT_CONFIG_RESET);
}

uint32_t mips_gicwdtimer_get_config(MIPSGICWDTimerState *gic, uint32_t vp_index)
{
    MIPSGICWDTimerVPState *vpwdtimer = &gic->vpwdtimers[vp_index];

    return vpwdtimer->config;
}

MIPSGICWDTimerState *mips_gicwdtimer_init(void *opaque, uint32_t nvps,
                                          MIPSGICWDTimerCB *cb)
{
    int i;

    MIPSGICWDTimerState *gicwdtimer = g_new(MIPSGICWDTimerState, 1);
    gicwdtimer->vpwdtimers = g_new(MIPSGICWDTimerVPState, nvps);
    gicwdtimer->num_vps = nvps;
    gicwdtimer->opaque = opaque;
    gicwdtimer->cb = cb;
    for (i = 0; i < nvps; i++) {
        gicwdtimer->vpwdtimers[i].gicwdtimer = gicwdtimer;
        gicwdtimer->vpwdtimers[i].vp_index = i;
        gicwdtimer->vpwdtimers[i].qtimer = timer_new_ns(QEMU_CLOCK_VIRTUAL,
                                            &gic_vpwdtimer_cb,
                                            &gicwdtimer->vpwdtimers[i]);
    }
    return gicwdtimer;
}
