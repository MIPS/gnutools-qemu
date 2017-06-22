/*
 * MIPS SEAD-3 development board emulation.
 *
 * Copyright (c) 2017 Imagination Technologies
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
#include "qemu-common.h"

#include "exec/address-spaces.h"
#include "hw/block/flash.h"
#include "hw/boards.h"
#include "hw/char/serial.h"
#include "hw/devices.h"
#include "hw/hw.h"
#include "hw/loader.h"
#include "hw/loader-fit.h"
#include "hw/mips/cps.h"
#include "hw/mips/cpudevs.h"
#include "hw/usb/hcd-ehci.h"
#include "net/net.h"
#include "qapi/error.h"
#include "qemu/cutils.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "chardev/char.h"
#include "sysemu/device_tree.h"
#include "sysemu/sysemu.h"
#include "sysemu/qtest.h"

#include <libfdt.h>

#define TYPE_MIPS_SEAD3 "mips-sead-3"
#define SEAD3(obj) OBJECT_CHECK(SEAD3State, (obj), TYPE_MIPS_SEAD3)

typedef struct {
    SysBusDevice parent_obj;

    MachineState *mach;
    MIPSCPSState *cps;
    SerialState *uart[2];

    CharBackend lcd_display;
    char lcd_content[16 * 2];
    bool lcd_inited;
    uint8_t lcd_ctl;
    uint8_t lcd_data;
    unsigned int lcd_pos;

    hwaddr reset_pc;
    target_ulong argc;
    hwaddr argv;

    uint32_t spd_cfg;
    uint32_t spd_addr;
    uint8_t spd[128];
#define SEAD3_SPD_DATA_ERR      BIT(9)
} SEAD3State;

#define SEAD3_CFG_USB           BIT(4)
#define SEAD3_CFG_DDR2          BIT(3)

enum sead3_spd_reg {
    SPD_CFG     = 0x00,
    SPD_ADDR    = 0x08,
    SPD_DATA    = 0x10,
};

static uint64_t sead3_spd_read(void *opaque, hwaddr addr, unsigned size)
{
    SEAD3State *s = SEAD3(opaque);

    switch (addr) {
    case SPD_CFG:
        return s->spd_cfg;

    case SPD_ADDR:
        return s->spd_addr;

    case SPD_DATA:
        if (s->spd_addr > sizeof(s->spd))
            return SEAD3_SPD_DATA_ERR;

        return s->spd[s->spd_addr];

    default:
        return 0;
    }
}

static void sead3_spd_write(void *opaque, hwaddr addr,
                            uint64_t val, unsigned size)
{
    SEAD3State *s = SEAD3(opaque);

    switch (addr) {
    case SPD_CFG:
        s->spd_cfg = val;
        break;

    case SPD_ADDR:
        s->spd_addr = val;
        break;
    }
}

static const MemoryRegionOps sead3_spd_ops = {
    .read = sead3_spd_read,
    .write = sead3_spd_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static uint64_t sead3_zero_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0;
}

static void sead3_nop_write(void *opaque, hwaddr addr,
                            uint64_t val, unsigned size)
{
}

static const MemoryRegionOps sead3_sram_cfg_ops = {
    .read = sead3_zero_read,
    .write = sead3_nop_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

enum sead3_fpga_reg {
    SEAD3_PI_SWRESET    = 0x050,
#define SEAD3_PI_SWRESET_MAGIC  0x4d
    LCD_CTL             = 0x400,
#define LCD_CTL_CHAR    (0xf << 0)
#define LCD_CTL_LINE    (1 << 6)
#define LCD_CTL_WRITE   (1 << 7)
    LCD_DATA            = 0x408,
    CPLD_CTL            = 0x410,
    CPLD_DATA           = 0x418,
};

static void sead3_lcd_update(SEAD3State *s)
{
    qemu_chr_fe_printf(&s->lcd_display, "\e[H%-16.16s\n\r%-16.16s",
                       s->lcd_content, &s->lcd_content[16]);
}

static void sead3_lcd_event(void *opaque, int event)
{
    SEAD3State *s = opaque;

    if (event == CHR_EVENT_OPENED && !s->lcd_inited) {
        memset(s->lcd_content, ' ', sizeof(s->lcd_content));
        sead3_lcd_update(s);
        s->lcd_inited = true;
    }
}

static uint64_t sead3_fpga_read(void *opaque, hwaddr addr, unsigned size)
{
    SEAD3State *s = opaque;
    uint32_t val;

    switch (addr) {
    case LCD_CTL:
        return s->lcd_ctl;

    case LCD_DATA:
        return s->lcd_data;

    case CPLD_CTL:
        val = ((qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) / 10) & 0x1) << 1;
        return val;

    default:
        return 0x0;
    }
}

static void sead3_fpga_write(void *opaque, hwaddr addr,
                             uint64_t val, unsigned size)
{
    SEAD3State *s = opaque;

    switch (addr) {
    case SEAD3_PI_SWRESET:
        if (val == SEAD3_PI_SWRESET_MAGIC)
            qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
        break;

    case LCD_CTL:
        s->lcd_ctl = val;

        if (val & LCD_CTL_WRITE) {
            s->lcd_pos = val & LCD_CTL_CHAR;
            if (val & LCD_CTL_LINE)
                s->lcd_pos += 16;
        } else {
            if (val & 0x1)
                memset(s->lcd_content, ' ', sizeof(s->lcd_content));

            if (val & 0x2)
                s->lcd_pos = 0;
        }

        s->lcd_data = s->lcd_content[s->lcd_pos];
        break;

    case LCD_DATA:
        s->lcd_data = val;
        s->lcd_content[s->lcd_pos++] = val;
        s->lcd_pos %= sizeof(s->lcd_content);
        sead3_lcd_update(s);
        break;
    }
}

static const MemoryRegionOps sead3_fpga_ops = {
    .read = sead3_fpga_read,
    .write = sead3_fpga_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const TypeInfo sead3_device = {
    .name          = TYPE_MIPS_SEAD3,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SEAD3State),
};

static void sead3_register_types(void)
{
    type_register_static(&sead3_device);
}
type_init(sead3_register_types)

static const void *sead3_kernel_filter(void *opaque, const void *kernel,
                                       hwaddr *load_addr, hwaddr *entry_addr)
{
    SEAD3State *s = SEAD3(opaque);

    s->reset_pc = *entry_addr;
    s->argv = *load_addr;

    return kernel;
}

static const struct fit_loader_match sead3_matches[] = {
    { "mti,sead-3" },
    { NULL },
};

static const struct fit_loader sead3_fit_loader = {
    .matches = sead3_matches,
    .addr_to_phys = cpu_mips_kseg0_to_phys,
    .kernel_filter = sead3_kernel_filter,
};

static void sead3_gen_spd(SEAD3State *s)
{
    uint8_t *spd = s->spd;
    uint8_t nbanks, nrow_bits, ncol_bits;
    size_t sz;

    sz = s->mach->ram_size / 8;

    nbanks = 8;
    sz /= nbanks;

    sz = 31 - clz32(sz);
    nrow_bits = MIN(11, (sz / 2) + 1);
    ncol_bits = sz - nrow_bits;

    spd[0] = sizeof(s->spd);
    spd[1] = 31 - clz32(sizeof(s->spd));
    spd[2] = 0x08;
    spd[3] = nrow_bits;
    spd[4] = ncol_bits;
    spd[6] = 0x40;
    spd[17] = nbanks;
}

static void sead3_add_cmdline(SEAD3State *s)
{
    char *buf, *arg, *ch;
    size_t sz;
    int argc;

    /* Only proceed if we have somewhere to place arguments */
    if (!s->argv)
        return;

    /*
     * Conservatively calculate the number of arguments. We have the kernel
     * filename, plus one more than the number of spaces in kernel_cmdline.
     */
    argc = 2;
    for (ch = s->mach->kernel_cmdline; *ch; ch++)
        if (*ch == ' ')
            argc++;

    /*
     * Calculate the size of the buffer we require to hold arguments, starting
     * with the array of 32 bit pointers to each argument.
     */
    sz = argc * sizeof(uint32_t);

    /* Plus the size of the kernel filename (argv[0]) */
    sz += strlen(s->mach->kernel_filename) + 1; /* space */

    /* Plus the size of the other provided arguments */
    sz += strlen(s->mach->kernel_cmdline) + 1; /* \0 */

    /* Calculate the base address for the arguments */
    s->argv = QEMU_ALIGN_DOWN(s->argv - sz, 4);

    /*
     * Allocate a buffer, leaving the first 4*argc bytes alone for now &
     * filling the rest with space-separated arguments.
     */
    buf = g_malloc(sz);
    arg = &buf[argc * 4];
    strcpy(arg, s->mach->kernel_filename);
    strcat(arg, " ");
    strcat(arg, s->mach->kernel_cmdline);

    /*
     * Now we walk through the string portion of the buffer, and each time we
     * find the end of an argument (ie. a space or the terminating NULL
     * character) we fill out an entry in the array that occupies the first
     * 4*argc bytes of the buffer. Spaces are replaced with NULL characters
     * such that each argument is a separate NULL-terminated string.
     */
    arg = ch = &buf[argc * 4];
    argc = 0;

    while (1) {
        /* Skip to the end of the argument */
        while (*ch && *ch != ' ')
            ch++;

        /* Store a 32b pointer to the argument in the array */
        stl_p(&buf[argc * 4], s->argv + arg - buf);
        argc++;

        /* If we hit the buffers original NULL-terminator then we're done */
        if (!*ch)
            break;

        /* Replace the space with a NULL character, and start a new argument */
        *ch++ = 0;
        arg = ch;
    }

    /* Set argc for sead3_reset() */
    s->argc = argc;

    /* Place the argument data in memory */
    rom_add_blob_fixed("args", buf, sz, cpu_mips_kseg0_to_phys(NULL, s->argv));
}

static void sead3_reset(void *opaque)
{
    SEAD3State *s = opaque;
    CPUMIPSState *env;
    CPUState *cpu;

    cpu = qemu_get_cpu(0);
    cpu_reset(cpu);

    env = &MIPS_CPU(cpu)->env;
    env->active_tc.PC = s->reset_pc;
    env->active_tc.gpr[4] = s->argc;
    env->active_tc.gpr[5] = s->argv;
    env->active_tc.gpr[6] = 0;
    env->active_tc.gpr[7] = 0;
}

static void sead3_mach_init(MachineState *machine)
{
    MemoryRegion *mem, *sys_mem = get_system_memory();
    const char *cpu_model;
    int fw_size, fit_err;
    DeviceState *dev;
    DriveInfo *dinfo;
    bool big_endian;
    void *board_id;
    SEAD3State *s;
    MIPSCPU *cpu;
    Chardev *chr;

    if (machine->ram_size > (384 * M_BYTE)) {
        error_report("Maximum memory size is 384MB");
        exit(1);
    }

    cpu_model = machine->cpu_model ?: "M14Kc";

#ifdef TARGET_WORDS_BIGENDIAN
    big_endian = true;
#else
    big_endian = false;
#endif

    dev = qdev_create(NULL, TYPE_MIPS_SEAD3);
    qdev_init_nofail(dev);

    s = SEAD3(dev);
    s->mach = machine;
    s->reset_pc = 0xbfc00000;
    s->argv = 0;

    cpu = cpu_mips_init(cpu_model);
    if (cpu == NULL) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    cpu_mips_irq_init_cpu(cpu);
    cpu_mips_clock_init(cpu);

    /* Register DDR */
    mem = g_new(MemoryRegion, 1);
    memory_region_allocate_system_memory(mem, NULL, "sead-3.ddr",
                                         machine->ram_size);
    memory_region_add_subregion(sys_mem, 0x00000000, mem);

    /* Register DDR Serial Presence Detect (SPD) interface */
    mem =  g_new(MemoryRegion, 1);
    memory_region_init_io(mem, NULL, &sead3_spd_ops, s, "sead-3.spd", 32);
    memory_region_add_subregion(sys_mem, 0x1b000040, mem);

    /* Register SEAD-3 configuration register */
    mem =  g_new(MemoryRegion, 1);
    memory_region_init_rom(mem, NULL, "sead-3.cfg", 4, &error_abort);
    stl_p(memory_region_get_ram_ptr(mem), SEAD3_CFG_DDR2 | SEAD3_CFG_USB);
    memory_region_add_subregion(sys_mem, 0x1b100110, mem);

    /* Register EHCI USB controller */
    sysbus_create_simple(TYPE_SEAD3_EHCI, 0x1b200000, cpu->env.irq[2]);

    /* Register system flash (typically used for a root filesystem) */
    dinfo = drive_get_by_index(IF_PFLASH, 0);
    pflash_cfi01_register(0x1c000000, NULL, "sead3.system-flash", 32 * M_BYTE,
                          dinfo ? blk_by_legacy_dinfo(dinfo) : NULL,
                          32 * M_BYTE / 128, 128, 4,
                          0x0000, 0x0000, 0x0000, 0x0000,
                          big_endian);

    /* Register SRAM configuration registers */
    mem =  g_new(MemoryRegion, 1);
    memory_region_init_io(mem, NULL, &sead3_sram_cfg_ops, s, "sead-3.sram-cfg", 4 * M_BYTE);
    memory_region_add_subregion(sys_mem, 0x1e000000, mem);

    /* Create LCD device, controlled via FPGA register region */
    chr = qemu_chr_new("lcd", "vc:320x240");
    qemu_chr_fe_init(&s->lcd_display, chr, NULL);
    qemu_chr_fe_set_handlers(&s->lcd_display, NULL, NULL,
                             sead3_lcd_event, s, NULL, true);

    /* Register FPGA register region */
    mem =  g_new(MemoryRegion, 1);
    memory_region_init_io(mem, NULL, &sead3_fpga_ops, s, "sead-3.fpga-regs", 0xa00);
    memory_region_add_subregion(sys_mem, 0x1f000000, mem);

    /* Register boot flash */
    dinfo = drive_get_by_index(IF_PFLASH, 1);
    pflash_cfi01_register(0x1fa00000, NULL, "sead3.boot-flash", 6 * M_BYTE,
                          dinfo ? blk_by_legacy_dinfo(dinfo) : NULL,
                          65536, (6 * M_BYTE) / 65536, 4,
                          0x0000, 0x0000, 0x0000, 0x0000,
                          big_endian);

    /* Register uart1 */
    serial_hds[1] = serial_hds[1] ?: qemu_chr_new("serial1", "null");
    s->uart[1] = serial_mm_init(sys_mem, 0x1f000800, 2, cpu->env.irq[4],
                                10000000, serial_hds[1], DEVICE_NATIVE_ENDIAN);

    /* Register uart0 */
    serial_hds[0] = serial_hds[0] ?: qemu_chr_new("serial0", "null");
    s->uart[0] = serial_mm_init(sys_mem, 0x1f000900, 2, cpu->env.irq[4],
                                10000000, serial_hds[0], DEVICE_NATIVE_ENDIAN);

    /* Register ethernet controller */
    lan9118_init(&nd_table[0], 0x1f010000, cpu->env.irq[6]);

    if (machine->firmware) {
        fw_size = load_image_targphys(machine->firmware,
                                      0x1fc00000, 4 * M_BYTE);
        if (fw_size == -1) {
            error_printf("unable to load firmware image '%s'\n",
                          machine->firmware);
            exit(1);
        }

        board_id = rom_ptr(0x1fc00010);
    } else if (machine->kernel_filename) {
        fit_err = load_fit(&sead3_fit_loader, machine->kernel_filename, s);
        if (fit_err) {
            error_printf("unable to load FIT image\n");
            exit(1);
        }

        board_id = g_malloc(4);
        rom_add_blob_fixed("sead3.board-id", board_id, 4, 0x1fc00010);
    } else if (!qtest_enabled()) {
        error_printf("Please provide either a -kernel or -bios argument\n");
        exit(1);
    }

    /*
     * Write the board ID into the ROM blob. This isn't perfectly emulated
     * since on a real system software would be unable to overwrite the board
     * ID, however QEMU refuses to execute from the flash if we overlay an I/O
     * region & in practice for real software this is close enough.
     */
    stl_p(board_id, 0x40);

    sead3_gen_spd(s);
    sead3_add_cmdline(s);

    qemu_register_reset(sead3_reset, s);
}

static void sead3_mach_class_init(MachineClass *mc)
{
    mc->desc = "MIPS SEAD-3";
    mc->init = sead3_mach_init;
    mc->block_default_type = IF_PFLASH;
    mc->default_ram_size = 384 * M_BYTE;
    mc->max_cpus = 1;
}

DEFINE_MACHINE("sead-3", sead3_mach_class_init)
