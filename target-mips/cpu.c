/*
 * QEMU MIPS CPU
 *
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>
 */

#include "cpu.h"
#include "kvm_mips.h"
#include "qemu-common.h"
#include "sysemu/kvm.h"

#ifdef MIPSSIM_COMPAT
FILE *svtracefile;
#endif

static void mips_cpu_set_pc(CPUState *cs, vaddr value)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;

    env->active_tc.PC = value & ~(target_ulong)1;
    if (value & 1) {
        env->hflags |= MIPS_HFLAG_M16;
    } else {
        env->hflags &= ~(MIPS_HFLAG_M16);
    }
}

static void mips_cpu_synchronize_from_tb(CPUState *cs, TranslationBlock *tb)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;

    env->active_tc.PC = tb->pc;
    env->hflags &= ~MIPS_HFLAG_BMASK;
    env->hflags |= tb->flags & MIPS_HFLAG_BMASK;
}

static bool mips_cpu_has_work(CPUState *cs)
{
    MIPSCPU *cpu = MIPS_CPU(cs);
    CPUMIPSState *env = &cpu->env;
    bool has_work = false;

    /* It is implementation dependent if non-enabled interrupts
       wake-up the CPU, however most of the implementations only
       check for interrupts that can be taken. */
    if ((cs->interrupt_request & CPU_INTERRUPT_HARD) &&
        cpu_mips_hw_interrupts_pending(env)) {
        has_work = true;
    }

    /* MIPS-MT has the ability to halt the CPU.  */
    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        /* The QEMU model will issue an _WAKE request whenever the CPUs
           should be woken up.  */
        if (cs->interrupt_request & CPU_INTERRUPT_WAKE) {
            has_work = true;
        }

        if (!mips_vpe_active(env)) {
            has_work = false;
        }
    }
    if (env->CP0_Config5 & (1 << CP0C5_VP)) {
        if (cs->interrupt_request & CPU_INTERRUPT_WAKE) {
            has_work = true;
        }
        if (!mips_vp_active(env)) {
            has_work = false;
        }
    }
    return has_work;
}

/* CPUClass::reset() */
static void mips_cpu_reset(CPUState *s)
{
    MIPSCPU *cpu = MIPS_CPU(s);
    MIPSCPUClass *mcc = MIPS_CPU_GET_CLASS(cpu);
    CPUMIPSState *env = &cpu->env;

    mcc->parent_reset(s);

    memset(env, 0, offsetof(CPUMIPSState, mvp));
    tlb_flush(s, 1);

    cpu_state_reset(env);

#ifndef CONFIG_USER_ONLY
    if (kvm_enabled()) {
        kvm_mips_reset_vcpu(cpu);
    }
#endif
}

static void mips_cpu_realizefn(DeviceState *dev, Error **errp)
{
    CPUState *cs = CPU(dev);
    MIPSCPU *cpu = MIPS_CPU(cs);
    MIPSCPUClass *mcc = MIPS_CPU_GET_CLASS(dev);
    CPUMIPSState *env = &cpu->env;

    cpu_reset(cs);
      fprintf(stderr, "real\n");
    if (env->CP0_Config3 & (1 << CP0C3_MSAP)) {
      fprintf(stderr, "msa\n");
        gdb_register_coprocessor(cs, mips_msa_get_reg, mips_msa_set_reg,
                                 2, "mips-msa.xml", 0);
        gdb_register_coprocessor(cs, mips_fpu_get_reg, mips_fpu_set_reg,
                                 34, "mips-fpu128.xml", 0);
    }
#if !TARGET_MIPS64
    else {
      fprintf(stderr, "fp64\n");
        gdb_register_coprocessor(cs, mips_fpu_get_reg, mips_fpu_set_reg,
                                 34, "mips-fpu64.xml", 0);
    }
#else
    else if (env->active_fpu.fcr0 & (1 << FCR0_F64)) {
        gdb_register_coprocessor(cs, mips_fpu_get_reg, mips_fpu_set_reg,
                                 34, "mips-fpu64.xml", 0);
    } else {
        gdb_register_coprocessor(cs, mips_fpu_get_reg, mips_fpu_set_reg,
                                 34, "mips-fpu.xml", 0);
    }
#endif
    gdb_register_coprocessor(cs, mips_cp0_get_reg, mips_cp0_set_reg,
                             4, "mips-cp0.xml");
//    if (env->CP0_Config3 & (1 << CP0C3_DSPP)) {
//        gdb_register_coprocessor(cs, mips_dsp_get_reg, mips_dsp_set_reg,
//                                 7, "mips-dsp.xml");
//    }

    qemu_init_vcpu(cs);

    mcc->parent_realize(dev, errp);
}

static void mips_cpu_initfn(Object *obj)
{
    CPUState *cs = CPU(obj);
    MIPSCPU *cpu = MIPS_CPU(obj);
    CPUMIPSState *env = &cpu->env;

    cs->env_ptr = env;
    cpu_exec_init(env);

    if (tcg_enabled()) {
        mips_tcg_init();
    }
}

static void mips_cpu_class_init(ObjectClass *c, void *data)
{
    MIPSCPUClass *mcc = MIPS_CPU_CLASS(c);
    CPUClass *cc = CPU_CLASS(c);
    DeviceClass *dc = DEVICE_CLASS(c);

    mcc->parent_realize = dc->realize;
    dc->realize = mips_cpu_realizefn;

    mcc->parent_reset = cc->reset;
    cc->reset = mips_cpu_reset;

    cc->has_work = mips_cpu_has_work;
    cc->do_interrupt = mips_cpu_do_interrupt;
    cc->cpu_exec_interrupt = mips_cpu_exec_interrupt;
    cc->dump_state = mips_cpu_dump_state;
    cc->set_pc = mips_cpu_set_pc;
    cc->synchronize_from_tb = mips_cpu_synchronize_from_tb;
    cc->gdb_read_register = mips_cpu_gdb_read_register;
    cc->gdb_write_register = mips_cpu_gdb_write_register;
#ifdef CONFIG_USER_ONLY
    cc->handle_mmu_fault = mips_cpu_handle_mmu_fault;
#else
    cc->do_unassigned_access = mips_cpu_unassigned_access;
    cc->do_unaligned_access = mips_cpu_do_unaligned_access;
    cc->get_phys_page_debug = mips_cpu_get_phys_page_debug;
#endif

    cc->gdb_num_core_regs = 35;
#if TARGET_MIPS64
    cc->gdb_core_xml_file = "mips64-cpu.xml";
#else
    cc->gdb_core_xml_file = "mips-cpu.xml";
#endif

    cc->gdb_stop_before_watchpoint = true;
}

static const TypeInfo mips_cpu_type_info = {
    .name = TYPE_MIPS_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(MIPSCPU),
    .instance_init = mips_cpu_initfn,
    .abstract = false,
    .class_size = sizeof(MIPSCPUClass),
    .class_init = mips_cpu_class_init,
};

static void mips_cpu_register_types(void)
{
    type_register_static(&mips_cpu_type_info);
}

type_init(mips_cpu_register_types)
