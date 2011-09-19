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
from dspasem import *
from dspasei import *

micro_mips_instructions = set([])


for i in loose_instructions:
    micro_mips_instructions.add(i)

#print "starting"

def add_one_unpack (how, instr):
    c_unpack[instr] = how

def gen_unpack(how, instructions):
    for instr in instructions:
        if (contains_alternate(instr)):
            add_one_unpack(how, alternate_1(instr))
            add_one_unpack(how, alternate_2(instr))
        else:
            add_one_unpack(how, instr)

gen_unpack(rt_rs_rd, rt_rs_rd_instructions )
gen_unpack(rt_rs, rt_rs_instructions)
gen_unpack(rt_rs_map_rd_rt,  rt_rs_map_rd_rt_instructions)
gen_unpack(rt_rs_map_rd_rs,  rt_rs_map_rd_rs_instructions)
gen_unpack(rd_immediate,  rd_immediate_instructions)
gen_unpack(rt_immediate13,  rt_immediate13_instructions)
gen_unpack(index_base_rd,  index_base_rd_instructions)
gen_unpack(rt_rs_sa11, rt_rs_sa11_instructions )
gen_unpack(rt_rs_sa11_map_rd_rt_sa, rt_rs_sa11_map_rd_rt_sa_instructions )
gen_unpack(rt_rs_sa12_map_rd_rt_sa, rt_rs_sa12_map_rd_rt_sa_instructions )
gen_unpack(rt_rs_sa13_map_rd_rt_sa, rt_rs_sa13_map_rd_rt_sa_instructions )
gen_unpack(rs_ac, rs_ac_instructions )
gen_unpack(rt_mask, rt_mask_instructions )
gen_unpack(rt_rs_ac, rt_rs_ac_instructions )
gen_unpack(rt_rs_bp, rt_rs_bp_instructions )
gen_unpack(rt_shift_ac, rt_shift_ac_instructions )
gen_unpack(shift2_ac, shift2_ac_instructions )
gen_unpack(rt_size_ac, rt_size_ac_instructions )

# print c_unpack

def fill_n_by_8(n, data):
    col = [];
    for i in range(0, 8):
        col.append([])
    row_num = 0;
    for row in data:
        col_num = 0;
        row_num_in_data = row[0]
        for x in range(row_num, row_num_in_data ):
            for y in range (0, 7):
                 col[y].append('');
        for e in row[1]:
            col[col_num].append(e);
            if (e != ''):
                if (contains_alternate(e)):
                    micro_mips_instructions.add(alternate_1(e))
                    micro_mips_instructions.add(alternate_2(e))
                else:
                    micro_mips_instructions.add(e)
            col_num = col_num + 1
        row_num = row_num_in_data + 1;
    return col




def create_opcodes(f, col):
    col_num = 0
    for c in col:
        print hex(col_num * 8 + 5)
        row_num=0
        for e in c:
            if (e != ''):
                if (contains_alternate(e)):
                    print e + " contains alternate"
                    print alternate_1(e)
                    print alternate_2(e)
                opc_f.write("   " + make_legal_c(e) + ' = ' + hex(row_num) + ',\n')
            row_num = row_num + 1
        col_num = col_num + 1
"""
create the opcodes for pool32
"""
opc_f = open('mips_dsp_pool32a_opcodes.h', 'w');
col = fill_n_by_8(16, pool32a_5)

create_opcodes(opc_f, col)

opc_f.close();

def empty_column(c):
     print("test column")
     for e in c:
         print '"' + e + '"'
         if (e != ''):
             print "false"
             return False
     print "true"
     return True

"""
create case statement for pool32
"""
opc_case_f = open('mips_dsp_pool32a_case.h', 'w');
col_num = 0
for c in col:
    if (empty_column(c) == False):
        opc_case_f.write( "        case " + hex(col_num * 8 + 5) + ":\n");
        opc_case_f.write( "            minor = (ctx->opcode >> 6) & 0xf;\n");
        opc_case_f.write( "            switch (minor) {\n");
        row_num=0
        for e in c:
            if (e != ''):
                sig = "(env, ctx)"
                opc_case_f.write( "            case " + make_legal_c(e) + ':\n');
                if (contains_alternate(e)):
                    opc_case_f.write( '                if ((ctx->opcode >> ' + str(distinguisher_bit(e)) + ') & 1) {\n');
                    opc_case_f.write( '                    gen_m'+ make_legal_c(alternate_2(e)) + sig + ';\n');
                    opc_case_f.write( '                } else {\n');
                    opc_case_f.write( '                    gen_m'+ make_legal_c(alternate_1(e)) + sig + ';\n');
                    opc_case_f.write( '                }\n');

                else:
                    #opc_case_f.write( '               mips32_op =' + 'OPC_'+ make_legal_c(e) + ';');
                    opc_case_f.write('                gen_m' + make_legal_c(e) + sig +  ";\n")
                opc_case_f.write( "                break;\n");
            row_num = row_num + 1
        opc_case_f.write(  "            }\n");
        opc_case_f.write(  "            break;\n");
        col_num = col_num + 1
opc_case_f.close()

def calc_depth(c, n):
   for e in c:
      if e in doubles:
        return n/2
   return n

"""
This is a little irregular. TBD clean up later.
"""
def one_pool32axf(col, num, depth):
    opc_case_f = open('mips_dsp_pool32axf_'+str(num)+'_case.h', 'w');
    col_num = 0
    for c in col:
        if (empty_column(c) == False):
            opc_case_f.write( "    case " + hex(col_num * 8 + num) + ":\n");
            # opc_case_f.write( "        minor = (ctx->opcode >> 6) & 0xf;\n");
            opc_case_f.write( "        switch (minor & " +  hex(calc_depth(c, depth)-1) + ") {\n");
            row_num=0
            for e in c:
                if (e != ''):
                    sig = "(env, ctx)"
                    opc_case_f.write( "        case " + make_legal_c(e) + ':\n');
                    if (contains_alternate(e)):
                        opc_case_f.write( '            if ((ctx->opcode >> ' + str(distinguisher_bit(e)) + ') & 1) {\n');
                        opc_case_f.write( '                gen_m'+ make_legal_c(alternate_2(e)) + sig + ';\n');
                        opc_case_f.write( '            } else {\n');
                        opc_case_f.write( '                gen_m'+ make_legal_c(alternate_1(e)) + sig + ';\n');
                        opc_case_f.write( '            }\n');

                    else:
                        #opc_case_f.write( '           mips32_op =' + 'OPC_'+ make_legal_c(e) + ';');
                        opc_case_f.write('            gen_m' + make_legal_c(e) + sig +  ";\n")
                    opc_case_f.write( "            break;\n");
                row_num = row_num + 1
            opc_case_f.write('        default:\n');
            opc_case_f.write('            goto pool32axf_invalid;\n');
            #opc_case_f.write('}\n');

            opc_case_f.write("        }\n");
            opc_case_f.write("        break;\n")
        col_num = col_num + 1
    opc_case_f.close()

col_32axf_1 = fill_n_by_8(4, pool32Axf_1)
col_32axf_2 = fill_n_by_8(4, pool32Axf_2)
col_32axf_4 = fill_n_by_8(16, pool32Axf_4)
col_32axf_7 = fill_n_by_8(4, pool32Axf_7)

one_pool32axf(col_32axf_1, 1, 4)
one_pool32axf(col_32axf_2, 2, 4)
one_pool32axf(col_32axf_4, 4, 16)
one_pool32axf(col_32axf_7, 7, 1)

#print "col_32axf"
#print col_32axf_1
#print col_32axf_2
#print col_32axf_4
#print col_32axf_7

opc_f = open('mips_dsp_pool32axf_opcodes.h', 'w');
create_opcodes(opc_f, col_32axf_1)
create_opcodes(opc_f, col_32axf_2)
create_opcodes(opc_f, col_32axf_4)
create_opcodes(opc_f, col_32axf_7)

# print "instructions rev \n" , instructions_rev , "\n done"

"""
create a subroutine for each micro mips instruction
"""
num_opcodes = 0
gen_f = open('micro_mips_gen.h', 'w');
default_signature = '(CPUState *env, DisasContext *ctx)'
for instr in sorted(micro_mips_instructions):
    #print instr
    c_name = 'm' + make_legal_c(instr);
    gen_f.write('static inline void gen_' + c_name + default_signature +
               '\n{\n')
    x = find_unpack(instr)
    if (x != ""):
        gen_f.write(x);
        if instr in instructions:
            gen_f.write(instructions[instr])
            num_opcodes = num_opcodes + 1;
    gen_f.write('}\n\n');


