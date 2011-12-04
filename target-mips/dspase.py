r"""
 *  MIPS emulation helpers for qemu.
 *
 *
 *  Copyright (c) 2011 Reed Kotler/MIPS Technologies DSP ASE
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
 """

major_special3_opcodes = {}

major_special3_opcodes = {'LX': ['1', '2'], 'INSV': ['1', '4'],
    'ADDU.QB': ['2', '0'], 'CMPU.EQ.QB': ['2', '1'], 'ABSQ_S.PH': ['2', '2'],
    'SHLL.QB': ['2', '3'], 'ADDUH.QB': ['3', '0'],
    'DPAQ.W.PH':['6', '0'], 'APPEND': ['6', '1'],
    'EXTR.W': ['7', '0']
}


#default format for fields, in bits, is 6,5,5,5, [5,6]
# TBD
major_special3_format = {'LX': ['base', 'index', 'rd'], 'INSV': ['1', '4'],
    'ADDU.QB': ['rs', 'rt', 'rd'], 'CMPU.EQ.QB': ['rs', 'rt', 'rd'],
    'ABSQ_S.PH': [0, 'rt', 'rd'],
    'SHLL.QB': ['rs', 'rt', 'rd'], 'ADDUH.QB': ['3', '0'],
    'DPAQ.W.PH':['rs', 'rt', [3, 0], [2, 'ac']],
    'APPEND': ['6', '1'],
}

# exceptions to major_special3_format go here

EXTR_format = ['rs', 'rt', [3, 0], [2, 'ac']]
EXTR_S_format = ['shift', 'rt', [3, 0], [2, 'ac']]

major_format = {'REPL.PH': [[10, 'immediate'], 'rd'],
    'EXTR_R': 'EXTR_W_1',  'EXTRV_R': 'EXTR_W_1',
    'WRDSP': ['rs', [4, 0], [6, 'mask']]
}

major_regimm_opcodes = {'BPOSGE32': ['3', '4', '1']}

special3_opcodes = {}

special3_opcodes['ADDU.QB'] = {
    'ADDU.QB': ['0', '0', '1'], 'SUBU.QB':['0', '1', '1'],
    'ADDU_S.QB': ['0', '4', '1'], 'SUBU_S.QB': ['0', '5', '1'],
    'MULEU_S.PH.QBL': ['0', '6', '1'], 'MULEU_S.PH.QBR': ['0', '7', '1'],
    'ADDU.PH': ['1', '0', '2'], 'SUBU.PH': ['1', '1', '2'],
    'ADDQ.PH': ['1', '2', '1'], 'SUBQ.PH': ['1', '3', '1'],
    'ADDU_S.PH': ['1', '4', '2'], 'SUBU_S.PH': ['1', '5', '2'],
    'ADDQ_S.PH': ['1', '6', '1'], 'SUBQ_S.PH': ['1', '7', '1'],
    'ADDSC': ['2', '0', '1'], 'ADDWC': ['2', '1', '1'],
    'MODSUB': ['2', '2', '1'], 'RADDU.W.QB': ['2', '4', '1'],
    'ADDQ_S.W': ['2', '6', '1'], 'SUBQ_S.W': ['2', '7', '1'],
    'MULEQ_S.W.PHL': ['3', '4', '1'], 'MULEQ_S.W.PHR': ['3', '5', '1'],
    'MULQ_S.PH': ['3', '6', '2'], 'MULQ_RS.PH': ['3', '7', '1']
}

special3_opcodes['CMPU.EQ.QB'] = {
    'CMPU.EQ.QB': ['0', '0', '1'], 'CMPU.LT.QB': ['0', '1', '1'],
    'CMPU.LE.QB': ['0', '2', '1'], 'PICK.QB': ['0', '3', '1'],
    'CMPGU.EQ.QB': ['0', '4', '1'], 'CMPGU.LT.QB': ['0', '5', '1'],
    'CMPGU.LE.QB': ['0', '6', '1'], 'CMP.EQ.PH': ['1', '0', '1'],
    'CMP.LT.PH': ['1', '1', '1'], 'CMP.LE.PH': ['1', '2', '1'],
    'PICK.PH': ['1', '3', '1'], 'PRECRQ.QB.PH': ['1', '4', '1'],
    'PRECR.QB.PH': ['1', '5', '2'], 'PACKRL.PH': ['1', '6', '1'],
    'PRECRQU_S.QB.PH': ['1', '7', '1'], 'PRECRQ.PH.W':['2', '4', '1'],
    'PRECRQ_RS.PH.W': ['2', '5', '1'], 'CMPGDU.EQ.QB': ['3', '0', '2'],
    'CMPGDU.LT.QB': ['3', '1', '2'], 'CMPGDU.LE.QB': ['3', '2', '2'],
    'PRECR_SRA.PH.W': ['3', '6', '2'], 'PRECR_SRA_R.PH.W': ['3', '7', '2']
    }

special3_opcodes['ABSQ_S.PH'] = {
    'ABSQ_S.QB': ['0', '1', '2'], 'REPL.QB': ['0', '2', '1'],
    'REPLV.QB': ['0', '3', '1'], 'PRECEQU.PH.QBL': ['0', '4', '1'],
    'PRECEQU.PH.QBR': ['0', '5', '1'], 'PRECEQU.PH.QBLA': ['0', '6', '1'],
    'PRECEQU.PH.QBRA': ['0', '7', '1'], 'ABSQ_S.PH': ['1', '1', '1'],
    'REPL.PH': ['1', '2', '1'], 'REPLV.PH': ['1', '3', '1'],
    'PRECEQ.W.PHL': ['1', '4', '1'], 'PRECEQ.W.PHR': ['1', '5', '1'],
    'ABSQ_S.W': ['2', '1', '1'], 'BITREV': ['3', '3', '1'],
    'PRECEU.PH.QBL': ['3', '4', '1'], 'PRECEU.PH.QBR': ['3', '5', '1'],
    'PRECEU.PH.QBLA': ['3', '6', '1'], 'PRECEU.PH.QBRA': ['3', '7', '1']
}

special3_opcodes['SHLL.QB'] = {
    'SHLL.QB': ['0', '0', '1'], 'SHRL.QB': ['0', '1', '1'],
    'SHLLV.QB': ['0', '2', '1'], 'SHRLV.QB': ['0', '3', '1'],
    'SHRA.QB': ['0', '4', '2'], 'SHRA_R.QB': ['0', '5', '2'],
    'SHRAV.QB': ['0', '6', '2'], 'SHRAV_R.QB': ['0', '7', '2'],
    'SHLL.PH': ['1', '0', '1'], 'SHRA.PH': ['1', '1', '1'],
    'SHLLV.PH': ['1', '2', '1'], 'SHRAV.PH': ['1', '3', '1'],
    'SHLL_S.PH': ['1', '4', '1'], 'SHRA_R.PH': ['1', '5', '1'],
    'SHLLV_S.PH': ['1', '6', '1'], 'SHRAV_R.PH': ['1', '7', '1'],
    'SHLL_S.W': ['2', '4', '1'], 'SHRA_R.W': ['2', '5', '1'],
    'SHLLV_S.W': ['2', '6', '1'], 'SHRAV_R.W': ['2', '7', '1'],
    'SHRL.PH': ['3', '1', '2'], 'SHRLV.PH': ['3', '3', '2']
}

special3_opcodes['LX'] = {
    'LWX': ['0', '0', '1'], 'LHX': ['0', '4', '1'],
    'LBUX': ['0', '6', '1']
}

special3_opcodes['DPAQ.W.PH'] = {
    'DPA.W.PH': ['0', '0', '2'], 'DPS.W.PH': ['0', '1', '2'],
    'MULSA.W.PH': ['0', '2', '2'], 'DPAU.H.QBL': ['0', '3', '1'],
    'DPAQ_S.W.PH': ['0', '4', '1'], 'DPSQ_S.W.PH': ['0', '5', '1'],
    'MULSAQ_S.W.PH': ['0', '6', '1'], 'DPAU.H.QBR': ['0', '7', '1'],
    'DPAX.W.PH': ['1', '0', '2'], 'DPSX.W.PH': ['1', '1', '2'],
    'DPSU.H.QBL': ['1', '3', '1'], 'DPAQ_SA.L.W': ['1', '4', '1'],
    'DPSQ_SA.L.W': ['1', '5', '1'], 'DPSU.H.QBR': ['1', '7', '1'],
    'MAQ_SA.W.PHL': ['2', '0', '1'], 'MAQ_SA.W.PHR': ['2', '2', '1'],
    'MAQ_S.W.PHL': ['2', '4', '1'], 'MAQ_S.W.PHR': ['2', '6', '1'],
    'DPAQX_S.W.PH': ['3', '0', '2'], 'DPSQX_S.W.PH': ['3', '1', '2'],
    'DPAQX_SA.W.PH': ['3', '2', '2'], 'DPSQX_SA.W.PH': ['3', '3', '2']
}

special3_opcodes['EXTR.W'] = {
    'EXTR.W': ['0', '0', '1'], 'EXTRV.W': ['0', '1', '1'],
    'EXTP': ['0', '2', '1'], 'EXTPV': ['0', '3', '1'],
    'EXTR_R.W': ['0', '4', '1'], 'EXTRV_R.W': ['0', '5', '1'],
    'EXTR_RS.W': ['0', '6', '1'], 'EXTRV_RS.W': ['0', '7', '1'],
    'EXTPDP': ['1', '2', '1'], 'EXTPDPV': ['1', '3', '1'],
    'EXTR_S.H': ['1', '6', '1'], 'EXTRV_S.H': ['1', '7', '1'],
    'RDDSP': ['2', '2', '1'], 'WRDSP': ['2', '3', '1'],
    'SHILO': ['3', '2', '1'], 'SHILOV': ['3', '3', '1'],
    'MTHLIP': ['3', '7', '1']
}

special3_opcodes['ADDUH.QB'] = {
    'ADDUH.QB': ['0', '0', '2'], 'SUBUH.QB': ['0', '1', '2'],
    'ADDUH_R.QB': ['0', '2', '2'], 'SUBUH_R.QB': ['0', '3', '2'],
    'ADDQH.PH': ['1', '0', '2'], 'SUBQH.PH': ['1', '1', '2'],
    'ADDQH_R.PH': ['1', '2', '2'], 'SUBQH_R.PH': ['1', '3', '2'],
    'MUL.PH': ['1', '4', '2'], 'MUL_S.PH': ['1', '6', '2'],
    'ADDQH.W': ['2', '0', '2'], 'SUBQH.W': ['2', '1', '2'],
    'ADDQH_R.W': ['2', '2', '2'], 'SUBQH_R.W': ['2', '3', '2'],
    'MULQ_S.W': ['2', '6', '2'], 'MULQ_RS.W': ['2', '7', '2']
}


special3_opcodes['APPEND'] = {
    'APPEND': ['0', '0', '2'], 'PREPEND': ['0', '1', '2'],
    'BALIGN': ['2', '0', '2']
}

special3_opcodes['INSV'] = {'INSV' : ['0', '0', '1']}

# create the map from child form to parent

special3_parent = {}

#print "creating parent map"

for i in special3_opcodes.keys():
    for j in special3_opcodes[i].keys():
        special3_parent[j] = i;

instructions_rev = {}

#print "creating revision map"

for i in major_regimm_opcodes.keys():
    instructions_rev[j] = major_regimm_opcodes[i][2];

for i in special3_opcodes.keys():
    for j in special3_opcodes[i].keys():
        instructions_rev[j] = special3_opcodes[i][j][2];



# these unpack could be automatically generated

c_default_unpack = {}

c_default_unpack['rs'] = """    uint8_t rs = (ctx->opcode >> 21) & 0x1f;"""

c_default_unpack['rt'] = """    uint8_t rt = (ctx->opcode >> 16) & 0x1f;"""


c_default_unpack['rd'] = """    uint8_t rd = (ctx->opcode >> 11) & 0x1f;"""

c_default_unpack['ac'] = """    uint8_t ac = (ctx->opcode >> 11) & 0x3;"""

c_default_unpack['sa'] = """    uint8_t sa = (ctx->opcode >> 11) & 0x1f;"""

def get_default_unpack(n):
    result = ""
    for field in n:
       result += ( (c_default_unpack[field]) + '\n')
    return result

c_unpack = {}

rs_rt_rd = get_default_unpack(['rs','rt', 'rd'])
rs_rt = get_default_unpack(['rs','rt'])
rs_rd = get_default_unpack(['rs','rd'])
rt_rd = get_default_unpack(['rt', 'rd'])
rd = get_default_unpack(['rd'])
rs = get_default_unpack(['rs'])
rt = get_default_unpack(['rt'])
ac = get_default_unpack(['ac'])
sa = get_default_unpack(['sa'])

rs_rt_ac = get_default_unpack(['rs', 'rt', 'ac'])

rs_rt_sa = get_default_unpack(['rs','rt', 'sa'])

c_unpack['ADDU.QB'] = rs_rt_rd
c_unpack['CMPU.EQ.QB'] = rs_rt

for comp in ['EQ', 'LE', 'LT']:
    c_unpack['CMPGDU.' + comp + '.QB'] = rs_rt_rd
    c_unpack['CMPGU.' + comp + '.QB'] = rs_rt_rd

cmpu_eq_qb_exceptions = [
    'PACKRL.PH', 'PICK.PH', 'PICK.QB', 'PRECR.QB.PH', 'PRECRQ.PH.W',
    'PRECRQ.QB.PH', 'PRECRQ_RS.PH.W', 'PRECRQU_S.QB.PH'
    ]

for i in cmpu_eq_qb_exceptions:
    c_unpack[i] = rs_rt_rd

c_unpack['ABSQ_S.PH'] = rt_rd; # rs==0
c_unpack['REPL.PH'] = """    uint16_t immediate = (ctx->opcode >> 16) & 0x3ff;
""" + rd
c_unpack['REPL.QB'] = """    uint16_t immediate = (ctx->opcode >> 16) & 0xff;
""" + rd
c_unpack['SHLL.QB']= rs_rt_rd;
c_unpack['LX'] = """    uint8_t base = (ctx->opcode >> 21) & 0x1f;
    uint8_t index = (ctx->opcode >> 16) & 0x1f;"""
c_unpack['DPAQ.W.PH'] = rs_rt_ac
c_unpack['ADDUH.QB'] = rs_rt_rd;
c_unpack['APPEND'] = rs_rt_sa
c_unpack['BALIGN'] = rs_rt + """    uint8_t bp = (ctx->opcode >> 11) & 0x1f;
"""

c_unpack['INSV'] = rs_rt;

c_unpack['LX'] = """    uint8_t base = (ctx->opcode >> 21) & 0x1f;
    uint8_t index = (ctx->opcode >> 16) & 0x1f;
""" + rd

c_unpack['MUL.PH'] = rs_rt_rd;


def EXTR_unpack_template(name):
    return rt + ac + """    uint8_t """ + name + """ = (ctx->opcode >> 21) & 0x1f ;
"""

c_unpack['EXTP'] = EXTR_unpack_template('size');

c_unpack['EXTPDP'] = EXTR_unpack_template('size');

c_unpack['EXTPV'] = EXTR_unpack_template('rs');

c_unpack['EXTPDPV'] = EXTR_unpack_template('rs');

c_unpack['EXTR_S.H'] = EXTR_unpack_template('shift');

c_unpack['EXTR.W'] = EXTR_unpack_template('shift');

c_unpack['EXTR_R.W'] = EXTR_unpack_template('shift');

c_unpack['EXTR_RS.W'] = EXTR_unpack_template('shift');

c_unpack['EXTRV.W'] = EXTR_unpack_template('rs');

c_unpack['EXTRV_R.W'] = EXTR_unpack_template('rs');

c_unpack['EXTRV_RS.W'] = EXTR_unpack_template('rs');

c_unpack['EXTRV_S.H'] = EXTR_unpack_template('rs');

c_unpack['MTHLIP'] = rs + ac;

c_unpack['PRECR_SRA.PH.W'] = rs_rt_sa;

c_unpack['PRECR_SRA_R.PH.W'] = rs_rt_sa;

c_unpack['RADDU.W.QB'] = rs_rd

c_unpack['RDDSP'] = rd + """    uint16_t mask = (ctx->opcode >> 16) & 0x3ff;
"""

c_unpack['SHILO'] = """
    uint8_t shift = (ctx->opcode >> 20) & 0x3f;
""" + ac;

c_unpack['SHILOV'] = rs + ac

# TBD sort out number of bits for the shift field
# these variations all are basically the same
#
c_unpack['SHLL.PH'] = rt + rd + """
    uint8_t sa = (ctx->opcode >> 21) & 0x1f;
"""

c_unpack['SHLL_S.PH'] = c_unpack['SHLL.PH']

c_unpack['SHLL.QB'] = c_unpack['SHLL.PH']

c_unpack['SHLL_S.W'] = c_unpack['SHLL.PH']

c_unpack['SHLLV.PH']  = rs_rt_rd;

c_unpack['SHLLV_S.PH']  = rs_rt_rd;

c_unpack['SHLLV.QB']  = rs_rt_rd;

c_unpack['SHLLV_S.W']  = rs_rt_rd;

c_unpack['SHRA.PH'] = c_unpack['SHLL.PH']

c_unpack['SHRA_R.PH'] = c_unpack['SHLL.PH']

c_unpack['SHRA.QB'] = c_unpack['SHLL.PH']

c_unpack['SHRA_R.QB'] = c_unpack['SHLL.PH']

c_unpack['SHRA_R.W'] = c_unpack['SHLL.PH']

c_unpack['SHRAV.PH']  = rs_rt_rd;

c_unpack['SHRAV_R.PH']  = rs_rt_rd;

c_unpack['SHRAV.QB']  = rs_rt_rd;

c_unpack['SHRAV_R.QB']  = rs_rt_rd;

c_unpack['SHRAV_R.W']  = rs_rt_rd;

c_unpack['SHRL.PH'] = c_unpack['SHLL.PH']

c_unpack['SHRL.QB'] = c_unpack['SHLL.PH']

c_unpack['SHRLV.PH'] = rs_rt_rd;

c_unpack['SHRLV.QB'] = rs_rt_rd;

c_unpack['WRDSP'] = rs + """    uint16_t mask = (ctx->opcode >> 11) & 0x3ff;
"""

# first look for the most specific
# then look for super class
def find_unpack(i):
    if i in c_unpack:
        return c_unpack[i]
    elif i in special3_parent:
        p = special3_parent[i]
        if p in c_unpack:
            return c_unpack[p]
    return "/* unpack for " + i + " not found */"

from dspasei import *

