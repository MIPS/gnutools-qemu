/*
 * QEMU/mipssim emulation
 *
 * Emulates a very simple machine model similar to the one used by the
 * proprietary MIPS emulator.
 * 
 * Copyright (c) 2007 Thiemo Seufer
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
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/hw.h"
#include "hw/mips/mips.h"
#include "hw/mips/cpudevs.h"
#include "hw/char/serial.h"
#include "hw/isa/isa.h"
#include "net/net.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/mips/bios.h"
#include "hw/loader.h"
#include "elf.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "qemu/error-report.h"
#include "sysemu/qtest.h"

static struct _loaderparams {
    int ram_size;
    const char *kernel_filename;
    const char *kernel_cmdline;
    const char *initrd_filename;
} loaderparams;

typedef struct ResetData {
    MIPSCPU *cpu;
    uint64_t vector;
} ResetData;

#ifdef MIPSSIM_COMPAT
/* For AVPS:
 *  Use -kernel to load first ELF file (as usual)
 *  Use -initrd to load second MIPS 'hex' file (normally a binary image)
 *  Leave the starting point as the reset vector
 */
static long load_mips_hex(const char *filename)
{
#ifdef TARGET_WORDS_BIGENDIAN
    const int big_endian = 1;
#else
    const int big_endian = 0;
#endif

#if defined(WORDS_BIGENDIAN)
    const int host_big_endian = 1;
#else
    const int host_big_endian = 0;
#endif

    const char *hex_intro = "# Endian";
    const size_t hex_intro_len = strlen(hex_intro);

    FILE *fd = NULL;
    char *line = NULL;
    size_t line_length = 0;
    long size = -1;
    uint32_t addr, data;

    line = malloc(LINE_MAX);

    if (line == NULL) {
        fprintf(stderr, "qemu: %s line buffer allocation failed\n", __func__);
        goto done;
    }

    /* open the file */
    fd = fopen(filename, "r");
    if (!fd) {
        fprintf(stderr, "qemu: could not open mips hex file '%s'\n", filename);
        goto done;
    }

    line_length = fread(line, sizeof(char), hex_intro_len, fd);

    if (line_length == -1) {
        goto done;
    }

    line[hex_intro_len] = 0;

    if (strstr(line, "# Endian") != line) {
        goto done;
    }

    fseek(fd, 0, SEEK_SET);

    size = 0;

    do {
        /* read a line */
        ssize_t num_read = getline(&line, &line_length, fd);

        if (num_read == -1) {
            if (feof(fd)) {
                break;
            }

            fprintf(stderr, "qemu: %s getline failed for '%s'\n", __func__,
                filename);

            size = -1;
            goto done;
        }

        /* ignore comments (lines starting with '#') */
        if (line[0] == '#') {
            continue;
        }

        /* extract two hex values from the line */
        if (sscanf(line, "%" SCNx32 " %" SCNx32, &addr, &data) != 2) {
            fprintf(stderr, "qemu: %s sscanf failed reading '%s' line '%s'\n",
                __func__, filename, line);
            size = -1;
            goto done;
        }

        if (host_big_endian ? !big_endian : big_endian) {
            bswap32s(&data);
        }

        /* put the data into the physical memory */
        cpu_physical_memory_write_rom(&address_space_memory, addr << 2, (uint8_t *)&data, 4);

        size += 4;
    } while (!feof(fd));

done:
    if (fd) {
        fclose(fd);
    }

    if (line) {
        free(line);
    }

    return size;
}
#endif

static int64_t load_kernel(void)
{
    int64_t entry, kernel_high;
    long kernel_size;
    long initrd_size;
    ram_addr_t initrd_offset;
    int big_endian;

#ifdef TARGET_WORDS_BIGENDIAN
    big_endian = 1;
#else
    big_endian = 0;
#endif

    kernel_size = load_elf(loaderparams.kernel_filename, cpu_mips_kseg0_to_phys,
                           NULL, (uint64_t *)&entry, NULL,
                           (uint64_t *)&kernel_high, big_endian,
                           EM_MIPS, 1, 0);
    if (kernel_size >= 0) {
        if ((entry & ~0x7fffffffULL) == 0x80000000)
            entry = (int32_t)entry;
    } else {
        fprintf(stderr, "qemu: could not load kernel '%s'\n",
                loaderparams.kernel_filename);
        exit(1);
    }

    /* load initrd */
    initrd_size = 0;
    initrd_offset = 0;
    if (loaderparams.initrd_filename) {
#ifdef MIPSSIM_COMPAT
        initrd_size = load_mips_hex(loaderparams.initrd_filename);
        (void)initrd_offset;
#else
        initrd_size = get_image_size (loaderparams.initrd_filename);
        if (initrd_size > 0) {
            initrd_offset = (kernel_high + ~INITRD_PAGE_MASK) & INITRD_PAGE_MASK;
            if (initrd_offset + initrd_size > loaderparams.ram_size) {
                fprintf(stderr,
                        "qemu: memory too small for initial ram disk '%s'\n",
                        loaderparams.initrd_filename);
                exit(1);
            }
            initrd_size = load_image_targphys(loaderparams.initrd_filename,
                initrd_offset, loaderparams.ram_size - initrd_offset);
        }
#endif
        if (initrd_size == (target_ulong) -1) {
            fprintf(stderr, "qemu: could not load initial ram disk '%s'\n",
                    loaderparams.initrd_filename);
            exit(1);
        }
    }
    return entry;
}

static void main_cpu_reset(void *opaque)
{
    ResetData *s = (ResetData *)opaque;
    CPUMIPSState *env = &s->cpu->env;

    cpu_reset(CPU(s->cpu));
    env->active_tc.PC = s->vector & ~(target_ulong)1;
    if (s->vector & 1) {
        env->hflags |= MIPS_HFLAG_M16;
    }
}

#ifndef MIPSSIM_COMPAT
static void mipsnet_init(int base, qemu_irq irq, NICInfo *nd)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, "mipsnet");
    qdev_set_nic_properties(dev, nd);
    qdev_init_nofail(dev);

    s = SYS_BUS_DEVICE(dev);
    sysbus_connect_irq(s, 0, irq);
    memory_region_add_subregion(get_system_io(),
                                base,
                                sysbus_mmio_get_region(s, 0));
}
#endif

static void
mips_mipssim_init(MachineState *machine)
{
    ram_addr_t ram_size = machine->ram_size;
    const char *cpu_model = machine->cpu_model;
    const char *kernel_filename = machine->kernel_filename;
    const char *kernel_cmdline = machine->kernel_cmdline;
    const char *initrd_filename = machine->initrd_filename;
    char *filename;
    MemoryRegion *address_space_mem = get_system_memory();
#ifndef MIPSSIM_COMPAT
    MemoryRegion *isa = g_new(MemoryRegion, 1);
#endif
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    MemoryRegion *bios = g_new(MemoryRegion, 1);
    MIPSCPU *cpu;
    CPUMIPSState *env;
    ResetData *reset_info;
    int bios_size;
#ifdef MIPSSIM_COMPAT
    uint64_t entry_bios = 0;
#ifdef TARGET_WORDS_BIGENDIAN
    const int big_endian = 1;
#else
    const int big_endian = 0;
#endif
#endif

    /* Init CPUs. */
    if (cpu_model == NULL) {
#ifdef TARGET_MIPS64
        cpu_model = "5Kf";
#else
        cpu_model = "24Kf";
#endif
    }
    cpu = cpu_mips_init(cpu_model);
    if (cpu == NULL) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    env = &cpu->env;

    reset_info = g_malloc0(sizeof(ResetData));
    reset_info->cpu = cpu;
    reset_info->vector = env->active_tc.PC;
    qemu_register_reset(main_cpu_reset, reset_info);

    /* Allocate RAM. */
    memory_region_allocate_system_memory(ram, NULL, "mips_mipssim.ram",
                                         ram_size);
    memory_region_init_ram(bios, NULL, "mips_mipssim.bios", BIOS_SIZE,
                           &error_fatal);
    vmstate_register_ram_global(bios);
    memory_region_set_readonly(bios, true);

    memory_region_add_subregion(address_space_mem, 0, ram);

    /* Map the BIOS / boot exception handler. */
    memory_region_add_subregion(address_space_mem, 0x1fc00000LL, bios);
    /* Load a BIOS / boot exception handler image. */
    if (bios_name == NULL)
        bios_name = BIOS_FILENAME;
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, bios_name);
#if defined(MIPSSIM_COMPAT)
    /* Use -bios to load test.hex for SV against IASim */
    bios_size = load_elf(filename, cpu_mips_kseg0_to_phys, NULL,
                         &entry_bios, NULL, NULL, big_endian, EM_MIPS, 1, 0);

    if (bios_size < 0 && !kernel_filename) {
#else
    if (filename) {
        bios_size = load_image_targphys(filename, 0x1fc00000LL, BIOS_SIZE);
        g_free(filename);
    } else {
        bios_size = -1;
    }
    if ((bios_size < 0 || bios_size > BIOS_SIZE) &&
        !kernel_filename && !qtest_enabled()) {
#endif
        /* Bail out if we have neither a kernel image nor boot vector code. */
        error_report("Could not load MIPS bios '%s', and no "
                     "-kernel argument was specified", bios_name);
        exit(1);
    } else {
        /* We have a boot vector start address. */
        env->active_tc.PC = (target_long)(int32_t)0xbfc00000;
    }

    if (kernel_filename) {
        loaderparams.ram_size = ram_size;
        loaderparams.kernel_filename = kernel_filename;
        loaderparams.kernel_cmdline = kernel_cmdline;
        loaderparams.initrd_filename = initrd_filename;
        reset_info->vector = load_kernel();
#if defined(MIPSSIM_COMPAT)
        if (bios_size > 0) {
            reset_info->vector = entry_bios;
        }
#endif
    }

    /* Init CPU internal devices. */
    cpu_mips_irq_init_cpu(cpu);
    cpu_mips_clock_init(cpu);

#ifndef MIPSSIM_COMPAT
    /* Register 64 KB of ISA IO space at 0x1fd00000. */
    memory_region_init_alias(isa, NULL, "isa_mmio",
                             get_system_io(), 0, 0x00010000);
    memory_region_add_subregion(get_system_memory(), 0x1fd00000, isa);

    /* A single 16450 sits at offset 0x3f8. It is attached to
       MIPS CPU INT2, which is interrupt 4. */
    if (serial_hds[0])
        serial_init(0x3f8, env->irq[4], 115200, serial_hds[0],
                    get_system_io());

    if (nd_table[0].used)
        /* MIPSnet uses the MIPS CPU INT0, which is interrupt 2. */
        mipsnet_init(0x4200, env->irq[2], &nd_table[0]);
#else
    if (serial_hds[0]) {
        /* MIPSsim has a single 16450 at 0x1fff_f000, aka 0xbfff_f000 */
        hwaddr serial_addr = 0x20000000;
        serial_addr -= ((hwaddr)1 << TARGET_PAGE_BITS);

        serial_mm_init(address_space_mem, serial_addr, 3, env->irq[4],
            115200, serial_hds[0], big_endian);
    }
#endif
}

static void mips_mipssim_machine_init(MachineClass *mc)
{
    mc->desc = "MIPS MIPSsim platform";
    mc->init = mips_mipssim_init;
}

DEFINE_MACHINE("mipssim", mips_mipssim_machine_init)
