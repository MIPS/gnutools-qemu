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

from dspaseutil import *
from dspase import instructions_rev

instructions = {}

def use_unary_helper_rd_rt(name):
    return """
    gen_unary_rd_rt_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rd, rt);

    MIPS_DEBUG("%s %s, %s", \"""" + name.lower() + """\", regnames[rd], regnames[rt]);
"""

def use_binary_helper_rs_rt(name):
    return """
    gen_binary_rs_rt_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rs, rt);

    MIPS_DEBUG("%s %s, %s", \"""" + name.lower() + """\", regnames[rs], regnames[rt]);
"""

def use_binary_helper_rs_rd(name):
    return """
    gen_binary_rs_rd_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rs, rd);

    MIPS_DEBUG("%s %s, %s", \"""" + name.lower() + """\", regnames[rd], regnames[rs]);
"""

def use_binary_helper_immed_rd(name):
    return """
    gen_binary_immed_rd_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, immediate,
        rd);

    MIPS_DEBUG("%s %s, 0x%x", \"""" + name.lower() + """\", regnames[rd], immediate);
"""

def use_binary_helper_rs_rt_ret(name):
    return """
    gen_binary_rs_rt_ret_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rs, rt);

    MIPS_DEBUG("%s %s, %s", \"""" + name.lower() + """\", regnames[rt], regnames[rs]);
"""

def use_binary_helper_rs_rd_rt(name):
    return """
    gen_binary_rs_rd_rt_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", \"""" + name.lower() + """\", regnames[rd], regnames[rs],
        regnames[rt]);
"""

def use_ternary_helper_rs_rt_sa(name):
    return """
    gen_ternary_rs_rt_sa_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """,
        rs, rt, sa);

    MIPS_DEBUG("%s %s, %s, %d", \"""" + name.lower() + """\", regnames[rt],
        regnames[rs], sa);
"""

def use_binary_helper_rt_rd_sa(name):
    return """
    gen_binary_rt_rd_sa_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", \"""" + name.lower() + """\", regnames[rd], regnames[rt], sa);
"""

def use_ternary_helper_rs_rt_bp(name):
    return """
    gen_ternary_rs_rt_bp_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rs, rt, bp);

    MIPS_DEBUG("%s %s, %s, %d", \"""" + name.lower() + """\", regnames[rt], regnames[rs], bp);
"""

def use_ternary_helper_rs_rt_ac(name):
    return """
    gen_ternary_rs_rt_ac_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", \"""" + name.lower() + """\", ac, regnames[rs], regnames[rt]);
"""

def use_binary_helper_rs_ac(name):
    return """
    gen_binary_rs_ac_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rs, ac);

    MIPS_DEBUG("%s %d, %s", \"""" + name.lower() + """\", ac, regnames[rs]);
"""

def use_ternary_helper_rs_rt_ac2(name):
    return """
    gen_ternary_rs_rt_ac2_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", \"""" + name.lower() + """\", ac, regnames[rs],
        regnames[rt]);
"""

def add_unary_from_helper(name):
    instructions[name] =  use_unary_helper_rd_rt(name)

def add_binary_from_helper(name):
    print "add binary " + name
    instructions[name] =  use_binary_helper_rs_rd_rt(name)
    print instructions[name]

def add_binary_cc_from_helper(name):
    print "add binary " + name
    instructions[name] =  use_binary_helper_rs_rt(name)
    print instructions[name]

unary_from_helper_list = [
    'ABSQ_S.QB', 'ABSQ_S.PH', 'ABSQ_S.W', 'BITREV', 'PRECEQ.W.PHR', 'PRECEQ.W.PHL',
    'PRECEQU.PH.QBL', 'PRECEQU.PH.QBLA', 'PRECEQU.PH.QBR', 'PRECEQU.PH.QBRA',
    'PRECEU.PH.QBL', 'PRECEU.PH.QBLA', 'PRECEU.PH.QBR', 'PRECEU.PH.QBRA'
]

def process_unary_from_helper_list() :
    for name in unary_from_helper_list:
        add_unary_from_helper(name)

process_unary_from_helper_list()

binary_from_helper_list = [
    'ADDQH.PH', 'ADDQH_R.PH', 'ADDQH_R.W', 'ADDQH.W', 'ADDQ.PH', 'ADDQ_S.PH',
    'ADDQ_S.W', 'ADDSC', 'ADDUH.QB', 'ADDUH_R.QB', 'ADDU.PH', 'ADDU.QB',
    'ADDU_S.PH', 'ADDU_S.QB', 'ADDWC', 'CMPGDU.EQ.QB',
    'CMPGDU.LE.QB', 'CMPGDU.LT.QB', 'CMPGU.EQ.QB', 'CMPGU.LE.QB',
    'CMPGU.LT.QB', 'MODSUB', 'MULEQ_S.W.PHL', 'MULEQ_S.W.PHR',
    'MULEU_S.PH.QBL', 'MULEU_S.PH.QBR', 'MUL.PH', 'MULQ_RS.PH', 'MULQ_RS.W',
    'MULQ_S.PH', 'MULQ_S.W', 'MUL_S.PH', 'PACKRL.PH', 'PICK.PH', 'PICK.QB',
    'PRECR.QB.PH',
    'PRECRQU_S.QB.PH', 'PRECRQ.PH.W', 'PRECRQ.QB.PH', 'PRECRQ_RS.PH.W', 'SUBQH.PH',
    'SUBQH_R.PH', 'SUBQH_R.W', 'SUBQH.W', 'SUBQ.PH', 'SUBQ_S.PH', 'SUBQ_S.W',
    'SUBUH.QB', 'SUBUH_R.QB', 'SUBU.PH', 'SUBU.QB', 'SUBU_S.PH', 'SUBU_S.QB',
    'SHLLV.PH', 'SHLLV_S.PH', 'SHLLV.QB', 'SHLLV_S.W', 'SHRAV.PH', 'SHRAV_R.PH',
    'SHRAV.QB', 'SHRAV_R.QB', 'SHRAV_R.W', 'SHRLV.PH', 'SHRLV.QB'
]

def process_binary_from_helper_list() :
    for name in binary_from_helper_list:
        add_binary_from_helper(name)

process_binary_from_helper_list()

binary_cc_from_helper_list = [
    'CMP.EQ.PH', 'CMP.LE.PH', 'CMP.LT.PH',
    'CMPU.EQ.QB', 'CMPU.LE.QB', 'CMPU.LT.QB',
]

def process_binary_cc_from_helper_list() :
    for name in binary_cc_from_helper_list:
        add_binary_cc_from_helper(name)

process_binary_cc_from_helper_list()


def use_extp_helper(name, field):
    return """
    gen_extp_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rt, ac, """ + field + """);

    MIPS_DEBUG("%s %s, %d, %d", \"""" + name.lower() + """\", regnames[rt], ac, """ + field + """);
"""

def use_extpv_helper(name):
    return """
    gen_extpv_wrapper(env, ctx, """ + instructions_rev[name] + """, gen_helper_""" + make_legal_c(name).lower() + """, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", \"""" + name.lower() + """\", regnames[rt], ac, regnames[rs]);
"""

def use_binary_helper_lx(name, action):
    func = ''

    if (action == "bu"):
        func = 'tcg_gen_qemu_ld8u'
    elif (action == "hs"):
        func = 'tcg_gen_qemu_ld16s'
    elif (action =="ws"):
        func = 'tcg_gen_qemu_ld32s'

    return """
    gen_binary_lx_wrapper(env, ctx, """ + instructions_rev[name] + ', ' + func + """, rd, index, base,
        ctx->mem_idx);

    MIPS_DEBUG("%s %s, %s(%s)", \"""" + name.lower() + """\", regnames[rd], regnames[index],
        regnames[base]);
"""

instructions['APPEND'] = use_ternary_helper_rs_rt_sa('APPEND')

instructions['BALIGN'] = use_ternary_helper_rs_rt_bp('BALIGN')


instructions['DPA.W.PH'] = use_ternary_helper_rs_rt_ac('DPA.W.PH')


# seems like this instruction does not actually exist.
#
#instructions['DPAQ.W.PH'] = """
#
#"""

instructions['DPAQ_S.W.PH'] = use_ternary_helper_rs_rt_ac2('DPAQ_S.W.PH')


#"""
instructions['DPAQX_S.W.PH'] = use_ternary_helper_rs_rt_ac2('DPAQX_S.W.PH')

instructions['DPAQX_SA.W.PH'] = use_ternary_helper_rs_rt_ac2('DPAQX_SA.W.PH')

instructions['DPAQ_SA.L.W'] = use_ternary_helper_rs_rt_ac2('DPAQ_SA.L.W');

instructions['DPAU.H.QBL'] = use_ternary_helper_rs_rt_ac('DPAU.H.QBL')

instructions['DPAU.H.QBR'] = use_ternary_helper_rs_rt_ac('DPAU.H.QBR')


instructions['DPAX.W.PH'] = use_ternary_helper_rs_rt_ac('DPAX.W.PH')

instructions['DPS.W.PH'] = use_ternary_helper_rs_rt_ac('DPS.W.PH')

instructions['DPSQX_S.W.PH'] = use_ternary_helper_rs_rt_ac2('DPSQX_S.W.PH')

# use_ternary_helper_rs_rt_ac('DPSQX_S.W.PH')

instructions['DPSQX_SA.W.PH'] =  use_ternary_helper_rs_rt_ac2('DPSQX_SA.W.PH')


instructions['DPSQ_S.W.PH'] = use_ternary_helper_rs_rt_ac2('DPSQ_S.W.PH')

instructions['DPSQ_SA.L.W'] = use_ternary_helper_rs_rt_ac2('DPSQ_SA.L.W');

instructions['DPSU.H.QBL'] = use_ternary_helper_rs_rt_ac('DPSU.H.QBL')


instructions['DPSU.H.QBR'] = use_ternary_helper_rs_rt_ac('DPSU.H.QBR')

instructions['DPSX.W.PH'] = use_ternary_helper_rs_rt_ac('DPSX.W.PH')

instructions['EXTP'] = use_extp_helper('EXTP', 'size')

instructions['EXTPDP'] = use_extp_helper('EXTPDP', 'size')

instructions['EXTPDPV'] = use_extpv_helper('EXTPDPV')

instructions['EXTPV'] = use_extpv_helper('EXTPV')

instructions['EXTR.W'] = use_extp_helper('EXTR.W', 'shift')

instructions['EXTR_R.W'] = use_extp_helper('EXTR_R.W', 'shift')

instructions['EXTR_RS.W'] = use_extp_helper('EXTR_RS.W', 'shift')


instructions['EXTRV.W'] = use_extpv_helper('EXTRV.W')

instructions['EXTRV_R.W'] = use_extpv_helper('EXTRV_R.W')

instructions['EXTRV_RS.W'] = use_extpv_helper('EXTRV_RS.W')

instructions['EXTRV_S.H'] = use_extpv_helper('EXTRV_S.H')

instructions['EXTR_S.H'] = use_extp_helper('EXTR_S.H', 'shift')

instructions['INSV'] = use_binary_helper_rs_rt_ret('INSV')

instructions['LBUX'] = use_binary_helper_lx('LBUX', "bu")

instructions['LHX'] = use_binary_helper_lx('LHX', "hs")

instructions['LWX'] = use_binary_helper_lx('LWX', "ws")


instructions['MAQ_S.W.PHL'] = use_ternary_helper_rs_rt_ac2('MAQ_S.W.PHL')


instructions['MAQ_S.W.PHR'] = use_ternary_helper_rs_rt_ac2('MAQ_S.W.PHR')

instructions['MAQ_SA.W.PHL'] = use_ternary_helper_rs_rt_ac2('MAQ_SA.W.PHL')

instructions['MAQ_SA.W.PHR'] = use_ternary_helper_rs_rt_ac2('MAQ_SA.W.PHR')


instructions['MTHLIP'] = r"""
    gen_mthlip_wrapper(env, ctx, """ + instructions_rev['MTHLIP'] + """, rs, ac);

    MIPS_DEBUG("%s %s, %d", "mthlip", regnames[rs], ac);
"""

# instructions['MUL.PH'] = use_binary_helper("MUL.PH")
instructions['MULSA.W.PH'] = use_ternary_helper_rs_rt_ac('MULSA.W.PH')


instructions['MULSAQ_S.W.PH'] = use_ternary_helper_rs_rt_ac2 ('MULSAQ_S.W.PH')

instructions['PRECR_SRA_R.PH.W'] = use_ternary_helper_rs_rt_sa('PRECR_SRA_R.PH.W')

instructions['PRECR_SRA.PH.W'] = use_ternary_helper_rs_rt_sa('PRECR_SRA.PH.W')

instructions['PREPEND'] =  use_ternary_helper_rs_rt_sa('PREPEND')


instructions['RADDU.W.QB'] = use_binary_helper_rs_rd('RADDU.W.QB')


instructions['RDDSP'] = """
    gen_rdwr_dspctrl(env, ctx, rd, mask, 0);
"""

instructions['REPL.PH'] = use_binary_helper_immed_rd('REPL.PH')


instructions['REPL.QB'] = use_binary_helper_immed_rd('REPL.QB')


instructions['REPLV.PH'] = use_unary_helper_rd_rt('REPLV.PH')


instructions['REPLV.QB'] = use_unary_helper_rd_rt('REPLV.QB')

instructions['SHILO'] = """
    gen_shilo_wrapper(env, ctx, """ + instructions_rev['SHILO'] + """, gen_helper_shilo, ac, shift);

    MIPS_DEBUG("%s %d, %d", "shilo", ac, shift);
"""

instructions['SHILOV'] = use_binary_helper_rs_ac('SHILOV')

instructions['SHLL.PH'] = use_binary_helper_rt_rd_sa('SHLL.PH')

instructions['SHLL_S.PH'] = use_binary_helper_rt_rd_sa('SHLL_S.PH')

instructions['SHLL.QB'] = use_binary_helper_rt_rd_sa('SHLL.QB')

instructions['SHLL_S.W'] =  use_binary_helper_rt_rd_sa('SHLL_S.W')

instructions['SHRA.PH'] = use_binary_helper_rt_rd_sa('SHRA.PH')

instructions['SHRA_R.PH'] = use_binary_helper_rt_rd_sa('SHRA_R.PH')

instructions['SHRA_R.W'] = use_binary_helper_rt_rd_sa('SHRA_R.W')

instructions['SHRA_R.QB'] = use_binary_helper_rt_rd_sa('SHRA_R.QB')

instructions['SHRA.QB'] = use_binary_helper_rt_rd_sa('SHRA.QB')

instructions['SHRL.PH'] = use_binary_helper_rt_rd_sa('SHRL.PH')

instructions['SHRL.QB'] = use_binary_helper_rt_rd_sa('SHRL.QB')

instructions['WRDSP'] = """
    gen_rdwr_dspctrl(env, ctx, rs, mask, 1);
"""

