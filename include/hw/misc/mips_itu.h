/*
 * Inter-Thread Communication Unit emulation.
 *
 * Copyright (c) 2016 Imagination Technologies
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

#ifndef MIPS_ITU_H
#define MIPS_ITU_H

#define TYPE_MIPS_ITU "mips-itu"
#define MIPS_ITU(obj) OBJECT_CHECK(MIPSITUState, (obj), TYPE_MIPS_ITU)

#define ITC_ADDRESSMAP_NUM 2

typedef struct MIPSITUState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/

    int32_t num_fifo;
    int32_t num_semaphores;

    /* ITC Storage */
    MemoryRegion storage_io;

    /* ITC Configuration Tags */
    uint64_t ITCAddressMap[ITC_ADDRESSMAP_NUM];
    MemoryRegion tag_io;
} MIPSITUState;

/* Get ITC Configuration Tag memory region. */
MemoryRegion *mips_itu_get_tag_region(MIPSITUState *itu);

#endif /* MIPS_ITU_H */
