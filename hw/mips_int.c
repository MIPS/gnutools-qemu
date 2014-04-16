/*
 * QEMU MIPS interrupt support
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "hw.h"
#include "mips_cpudevs.h"
#include "cpu.h"

static void cpu_mips_irq_request(void *opaque, int irq, int level)
{
    CPUState *env = (CPUState *)opaque;

    if (irq < 0 || irq > 7)
        return;

    if (level) {
        env->CP0_Cause |= 1 << (irq + CP0Ca_IP);
    } else {
        env->CP0_Cause &= ~(1 << (irq + CP0Ca_IP));
    }

    if (env->CP0_Cause & CP0Ca_IP_mask) {
        cpu_interrupt(env, CPU_INTERRUPT_HARD);
    } else {
        cpu_reset_interrupt(env, CPU_INTERRUPT_HARD);
    }
}

static void cpu_mips_guest_irq_request(void *opaque, int irq, int level)
{
    CPUState *env = (CPUState *)opaque;
    if (irq < 0 || irq > 7)
         return;

    if (level){
        env->Guest.CP0_Cause |= 1 << (irq + CP0Ca_IP);
    }
    else {
        env->Guest.CP0_Cause &= ~(1 << (irq + CP0Ca_IP));
    }

    if (env->hflags & MIPS_HFLAG_GUEST) {
        // only initiate when running in guest mode
        if (env->Guest.CP0_Cause & CP0Ca_IP_mask) {
            cpu_interrupt(env, CPU_INTERRUPT_HARD);
        } else {
            cpu_reset_interrupt(env, CPU_INTERRUPT_HARD);
        }
    }
}

void cpu_mips_check_irq_guest(CPUState *env)
{
    if (env->Guest.CP0_Cause & CP0Ca_IP_mask) {
        cpu_interrupt(env, CPU_INTERRUPT_HARD);
    }
}

void cpu_mips_silence_irq_guest(CPUState *env)
{
    // There is no pending root interrupt in guest mode.
    // As the root context interrupt system is always active,
    // even during guest mode execution.
    cpu_reset_interrupt(env, CPU_INTERRUPT_HARD);
}

void cpu_mips_irq_init_cpu(CPUState *env)
{
    qemu_irq *qi;
    int i;

    qi = qemu_allocate_irqs(cpu_mips_irq_request, env, 8);
    for (i = 0; i < 8; i++) {
        env->irq[i] = qi[i];
    }
    if (env->CP0_Config3 & (1 << CP0C3_VZ)) {
        qi = qemu_allocate_irqs(cpu_mips_guest_irq_request, env, 8);
        for (i = 0; i < 8; i++) {
            env->guest_irq[i] = qi[i];
        }
    }
}

void cpu_mips_soft_irq(CPUState *env, int irq, int level)
{
    if (irq < 0 || irq > 2) {
        return;
    }

    qemu_set_irq(env->irq[irq], level);
}

void cpu_mips_soft_irq_guest(CPUState *env, int irq, int level)
{
    if (irq < 0 || irq > 2) {
        return;
    }

    qemu_set_irq(env->guest_irq[irq], level);
}
