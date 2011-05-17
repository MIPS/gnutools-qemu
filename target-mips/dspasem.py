r"""
 *  MIPS emulation helpers for qemu.
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

pool32a_opcodes = {}

rt_rs_instructions = [
    'CMP.LE.PH','CMP.LT.PH', 'CMP.EQ.PH', 'CMPU.EQ.QB', 'CMPU.LT.QB', 'CMPU.LE.QB',
    'INSV',
]

rt_rs_map_rd_rs_instructions = [
      'RADDU.W.QB',
]

rt_rs_map_rd_rt_instructions = [

   'ABSQ_S.QB', 'REPLV.PH',  'ABSQ_S.PH', 'REPLV.QB', 'ABSQ_S.W', 'BITREV',
     'PRECEQ.W.PHL',  'PRECEQ.W.PHR',  'PRECEQU.PH.QBL',  'PRECEQU.PH.QBLA',
   'PRECEQU.PH.QBR', 'PRECEQU.PH.QBRA',  'PRECEU.PH.QBL', 'PRECEU.PH.QBLA',
   'PRECEU.PH.QBR',  'PRECEU.PH.QBRA',

]

rt_rs_rd_instructions = [
    'ADDQ.PH', 'ADDQ_S.PH', 'MULEQ_S.W.PHL', 'MUL[_S].PH', 'MULEQ_S.W.PHR', 'PRECR.QB.PH',
    'ADDQH[_R].PH', 'ADDQH[_R].W', 'MULEU_S.PH.QBL', 'PRECRQ.QB.PH', 'CMPGU.EQ.QB', 'ADDU[_S].QB',
    'MULEU_S.PH.QBR', 'PRECRQ.PH.W', 'CMPGU.LT.QB', 'ADDU[_S].PH', 'MULQ_RS.PH', 'PRECRQ_RS.PH.W',
    'CMPGU.LE.QB', 'ADDUH[_R].QB', 'MULQ_S.PH', 'PRECRQU_S.QB.PH', 'CMPGDU.EQ.QB', 'SHRAV[_R].PH',
    'MULQ_RS.W', 'PACKRL.PH', 'CMPGDU.LT.QB', 'SHRAV[_R].QB',  'MULQ_S.W', 'PICK.QB', 'CMPGDU.LE.QB',
    'SUBQ[_S].PH', 'PICK.PH', 'SUBQH[_R].PH', 'SUBQH[_R].W',  'MODSUB', 'SUBU[_S].QB',
     'SHRAV_R.W', 'ADDQ_S.W',  'SUBU[_S].PH', 'SHRLV.PH',  'SUBQ_S.W', 'SUBUH[_R].QB', 'SHRLV.QB',
     'ADDSC', 'SHLLV[_S].PH', 'SHLLV.QB', 'ADDWC', 'SHLLV_S.W', 'SUBQH[_R].PH',
]

rd_immediate_instructions = [
   'REPL.PH',
]

rt_immediate13_instructions = [
   'REPL.QB',
]

index_base_rd_instructions = [
    'LHX', 'LWX', 'LBUX',
]

rt_rs_sa11_map_rd_rt_sa_instructions = [
    'SHLL_S.W',  'SHRA_R.W',
]

rt_rs_sa12_map_rd_rt_sa_instructions = [
    'SHLL[_S].PH', 'SHRL.PH', 'SHRA[_R].PH',
]

rt_rs_sa13_map_rd_rt_sa_instructions = [
    'SHLL.QB', 'SHRL.QB', 'SHRA[_R].QB',
]

rt_rs_sa11_instructions = [
     'APPEND', 'PRECR_SRA[_R].PH.W', 'PREPEND'
]

rs_ac_instructions = [
     'MFHI', 'MTHLIP', 'MFLO',   'SHILOV', 'MTLO',
]

rt_mask_instructions = [
     'RDDSP',  'WRDSP',
]

rt_rs_ac_instructions = [
     'MAQ_S[A].W.PHR', 'MAQ_S[A].W.PHL', 'DPAX.W.PH',  'DPAQ_S.W.PH', 'DPS.W.PH',
     'DPSQ_S.W.PH', 'EXTRV.W', 'DPAX.W.PH',  'DPAQ_SA.L.W',  'DPSX.W.PH',
     'DPSQ_SA.L.W',  'EXTRV_R.W', 'DPAU.H.QBL',  'DPAQX_S.W.PH', 'DPSU.H.QBL',
     'DPSQX_S.W.PH', 'EXTPV', 'MULSA.W.PH',  'EXTRV_RS.W', 'DPAU.H.QBR',
     'DPAQX_SA.W.PH', 'DPSU.H.QBR', 'DPSQX_SA.W.PH', 'EXTPDPV',
     'MULSAQ_S.W.PH', 'EXTRV_S.H', 'DPA.W.PH'
]

rt_rs_bp_instructions = [
      'BALIGN',
]

rt_shift_ac_instructions = [
     'EXTR.W', 'EXTR_R.W', 'EXTR_RS.W', 'EXTR_S.H'
]

shift2_ac_instructions = [
    'SHILO',
]

rt_size_ac_instructions = [
     'EXTPDP', 'EXTP'
 ]


doubles = [ 'SHLL.QB', 'SHRL.QB',
]

pool32a_5  = [

 [0x0, ['CMP.EQ.PH', 'ADDQ[_S].PH', '', 'SHILO',  'MULEQ_S.W.PHL', 'MUL[_S].PH', '', 'REPL.PH']],
 [0x1, ['CMP.LT.PH', 'ADDQH[_R].PH', '', '', 'MULEQ_S.W.PHR', 'PRECR.QB.PH', '', '']],
 [0x2, ['CMP.LE.PH', 'ADDQH[_R].W', 'MULEU_S.PH.QBL', '', '', 'PRECRQ.QB.PH', '', '']],
 [0x3, ['CMPGU.EQ.QB', 'ADDU[_S].QB', 'MULEU_S.PH.QBR', '', '', 'PRECRQ.PH.W', '', '']],
 [0x4, ['CMPGU.LT.QB', 'ADDU[_S].PH', 'MULQ_RS.PH', '', '', 'PRECRQ_RS.PH.W', '', '']],
 [0x5, ['CMPGU.LE.QB', 'ADDUH[_R].QB', 'MULQ_S.PH', '', 'LHX', 'PRECRQU_S.QB.PH', '', '']],
 [0x6, ['CMPGDU.EQ.QB', 'SHRAV[_R].PH', 'MULQ_RS.W', '', 'LWX', 'PACKRL.PH', '', '']],
 [0x7, ['CMPGDU.LT.QB', 'SHRAV[_R].QB', 'MULQ_S.W', '', '', 'PICK.QB', '', '']],
 [0x8, ['CMPGDU.LE.QB', 'SUBQ[_S].PH', 'APPEND', '', 'LBUX', 'PICK.PH', '', '']],
 [0x9, ['CMPU.EQ.QB', 'SUBQH[_R].PH', 'PREPEND', '', '', '', '', '']],
 [0xA, ['CMPU.LT.QB', 'SUBQH[_R].W', 'MODSUB', '', '', '', '', '']],
 [0xB, ['CMPU.LE.QB', 'SUBU[_S].QB', 'SHRAV_R.W', '', '', '', 'SHRA_R.W', '']],
 [0xC, ['ADDQ_S.W', 'SUBU[_S].PH', 'SHRLV.PH', '', '', '', 'SHRA[_R].PH', '']],
 [0xD, ['SUBQ_S.W', 'SUBUH[_R].QB', 'SHRLV.QB', '', '', '', '', '']],
 [0xE, ['ADDSC', 'SHLLV[_S].PH', 'SHLLV.QB', '', '', '', 'SHLL[_S].PH', '']],
 [0xF, ['ADDWC', 'PRECR_SRA[_R].PH.W', 'SHLLV_S.W', '', '', '', 'SHLL_S.W', '']]
]

# extra bits ofr shll, shrl, MAQ_S[A]W.PHR, MAQ_S[A].W.PHL
# get rid of MFHI, MFLO, MTHI, MTHLO
# get rid of whole column of rddsp.. handle extp and extdp as special case
#
pool32Axf_1  = [
[0x0, ['', 'MTHLIP', '',  '', 'SHLL.QB', 'MAQ_S.W.PHR', '', 'EXTR.W']],
[0x1, ['', 'SHILOV', '',  '', 'SHRL.QB', 'MAQ_S.W.PHL', '', 'EXTR_R.W']],
[0x2, ['', '', '',  '', '', 'MAQ_SA.W.PHR', '', 'EXTR_RS.W']],
[0x3, ['', '', '',  '', '', 'MAQ_SA.W.PHL', '', 'EXTR_S.H']],

]

# MADD, MADDU, MSUB, MSUBU, MULT, MULTU removed because was
# already handled.
##
pool32Axf_2  = [
[0x0, ['DPA.W.PH', 'DPAQ_S.W.PH', 'DPS.W.PH',  'DPSQ_S.W.PH', 'BALIGN', '', '', 'EXTRV.W']],
[0x1, ['DPAX.W.PH', 'DPAQ_SA.L.W', 'DPSX.W.PH',  'DPSQ_SA.L.W', '', '', '', 'EXTRV_R.W']],
[0x2, ['DPAU.H.QBL', 'DPAQX_S.W.PH', 'DPSU.H.QBL',  'DPSQX_S.W.PH', 'EXTPV', '', '', 'EXTRV_RS.W']],
[0x3, ['DPAU.H.QBR', 'DPAQX_SA.W.PH', 'DPSU.H.QBR',  'DPSQX_SA.W.PH', 'EXTPDPV', '', '', 'EXTRV_S.H']],

]

pool32Axf_4  = [
[0x0, ['ABSQ_S.QB', 'REPLV.PH', '',  '', '', '', '', '']],
[0x1, ['ABSQ_S.PH', 'REPLV.QB', '',  '', '', '', '', '']],
[0x2, ['ABSQ_S.W', '', '',  '', '', '', '', '']],
[0x3, ['BITREV', '', '',  '', '', '', '', '']],
[0x4, ['INSV', '', '',  '', '', '', '', '']],
[0x5, ['PRECEQ.W.PHL', '', '',  '', '', '', '', '']],
[0x6, ['PRECEQ.W.PHR', '', '',  '', '', '', '', '']],
[0x7, ['PRECEQU.PH.QBL', 'PRECEQU.PH.QBLA', '',  '', '', '', '', '']],
[0x9, ['PRECEQU.PH.QBR', 'PRECEQU.PH.QBRA', '',  '', '', '', '', '']],
[0xA, ['', '', '',  '', '', '', '', '']],
[0xB, ['PRECEU.PH.QBL', 'PRECEU.PH.QBLA', '',  '', '', '', '', '']],
[0xC, ['', '', '',  '', '', '', '', '']],
[0xD, ['PRECEU.PH.QBR', 'PRECEU.PH.QBRA', '',  '', '', '', '', '']],
[0xE, ['', '', '',  '', '', '', '', '']],
[0xF, ['RADDU.W.QB', '', '',  '', '', '', '', '']],
]

pool32Axf_7  = [
[0x0, ['SHRA[_R].QB', 'SHRL.PH', 'REPL.QB',  '', '', '', '', '']],
]

# not connected because of conflicts with other things
#
loose_instructions = ['EXTP', 'EXTPDP', 'MULSA.W.PH', 'MULSAQ_S.W.PH']
distinguisher_exceptions = { 'SHLL[_S].PH' : 11, 'SHRA[_R].QB': 12}

def distinguisher_bit(instr):
    if instr in distinguisher_exceptions:
         return distinguisher_exceptions[instr]
    else:
        return 10    # default

"""
We can in principal reuse common tables for opcode signatures but for now we don't have a common database.
"""

r"""
pool3232Axf_X  = [
0x0, '', '', '',  '', '', '', '', '',
0x1, '', '', '',  '', '', '', '', '',
0x2, '', '', '',  '', '', '', '', '',
0x3, '', '', '',  '', '', '', '', '',
0x4, '', '', '',  '', '', '', '', '',
0x5, '', '', '',  '', '', '', '', '',
0x6, '', '', '',  '', '', '', '', '',
0x7, '', '', '',  '', '', '', '', '',
0x8, '', '', '',  '', '', '', '', '',
0x9, '', '', '',  '', '', '', '', '',
0xA, '', '', '',  '', '', '', '', '',
0xB, '', '', '',  '', '', '', '', '',
0xC, '', '', '',  '', '', '', '', '',
0xD, '', '', '',  '', '', '', '', '',
0xE, '', '', '',  '', '', '', '', '',
0xF, '', '', '',  '', '', '', '', '',
]
"""

"""
TBD. Merge most of this with the non micro mips version.
Just need to know what bit various fields start on. rest is the same.

"""
c_default_unpack = {}

c_default_unpack['rs'] = """    uint8_t rs = (ctx->opcode >> 16) & 0x1f;"""
c_default_unpack['rt'] = """    uint8_t rt = (ctx->opcode >> 21) & 0x1f;"""
c_default_unpack['rd'] = """    uint8_t rd = (ctx->opcode >> 11) & 0x1f;"""
c_default_unpack['ac'] = """    uint8_t ac = (ctx->opcode >> 14) & 0x3;"""
c_default_unpack['sa11'] = """    uint8_t sa = (ctx->opcode >> 11) & 0x1f;"""
c_default_unpack['sa12'] = """    uint8_t sa = (ctx->opcode >> 12) & 0xf;"""
c_default_unpack['sa13'] = """    uint8_t sa = (ctx->opcode >> 13) & 0x7;"""

c_default_unpack['bp'] = """    uint8_t bp = (ctx->opcode >> 14) & 0x3;"""

c_default_unpack['rt_to_rd'] = """    uint8_t rd = (ctx->opcode >> 21) & 0x1f;"""
c_default_unpack['rs_to_rt'] = """    uint8_t rt = (ctx->opcode >> 16) & 0x1f;"""

c_default_unpack['immediate'] = """    uint16_t immediate = (ctx->opcode >> 16) & 0x3FF;"""
c_default_unpack['immediate13'] = """    uint16_t immediate = (ctx->opcode >> 13) & 0xFF;"""

c_default_unpack['index'] = """    uint8_t index = (ctx->opcode >> 21) & 0x1F;"""
c_default_unpack['base'] = """    uint8_t base = (ctx->opcode >> 16) & 0x1F;"""
c_default_unpack['mask'] = """    uint8_t mask = (ctx->opcode >> 14) & 0x7F;"""
c_default_unpack['shift'] = """    uint8_t shift = (ctx->opcode >> 16) & 0x1F;"""
c_default_unpack['shift2'] = """    uint8_t shift = (ctx->opcode >> 16) & 0x3F;"""
c_default_unpack['size'] = """    uint8_t size = (ctx->opcode >> 16) & 0x1F;"""

def get_default_unpack(n):
    result = ""
    for field in n:
       result += ( (c_default_unpack[field]) + '\n')
    return result

c_unpack = {}

rt_rs_rd = get_default_unpack(['rs','rt', 'rd'])
rt_rs = get_default_unpack(['rs','rt'])
rt_rd = get_default_unpack(['rt', 'rd'])
rd = get_default_unpack(['rd'])
rs = get_default_unpack(['rs'])
rt = get_default_unpack(['rt'])
ac = get_default_unpack(['ac'])
sa11 = get_default_unpack(['sa11'])
sa12 = get_default_unpack(['sa12'])
sa13 = get_default_unpack(['sa13'])


immediate = get_default_unpack(['immediate'])
immediate13 = get_default_unpack(['immediate13'])


rt_rs_map_rd_rt = get_default_unpack(['rs_to_rt','rt_to_rd'])

rt_rs_map_rd_rs = get_default_unpack(['rs','rt_to_rd'])

rt_map_rd = get_default_unpack(['rt_to_rd'])

rs_ac = get_default_unpack(['rs', 'ac'])

rt_rs_ac = get_default_unpack(['rs', 'rt', 'ac'])

rt_rs_sa11 = get_default_unpack(['rs','rt', 'sa11'])
rt_rs_sa12 = get_default_unpack(['rs','rt', 'sa12'])
rt_rs_sa13 = get_default_unpack(['rs','rt', 'sa13'])

rt_rs_bp = get_default_unpack(['rs','rt', 'bp'])

rd_immediate = rd + immediate;
rt_immediate13 = rt_map_rd + immediate13;

index_base_rd = get_default_unpack(['index', 'base']) + rd

rt_rs_sa11_map_rd_rt_sa = get_default_unpack(['rs_to_rt','rt_to_rd', 'sa11'])
rt_rs_sa12_map_rd_rt_sa = get_default_unpack(['rs_to_rt','rt_to_rd', 'sa12'])
rt_rs_sa13_map_rd_rt_sa = get_default_unpack(['rs_to_rt','rt_to_rd', 'sa13'])

rt_mask = rt + get_default_unpack(['mask'])

rt_shift_ac =  get_default_unpack(['rt', 'ac', 'shift'])

shift2_ac =  get_default_unpack(['ac', 'shift2'])

rt_size_ac =  get_default_unpack(['rt', 'ac', 'size'])

def find_unpack(i):
    if i in c_unpack:
        return c_unpack[i]
    return ""
