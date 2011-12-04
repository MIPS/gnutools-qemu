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

from dspaseutil import *
from dspase import *
from dspasei import *
import dspase

# major_opcodes = [['1', '2', 'LX'], ['1', '4', 'INSV']]
import re


def to_int_from_octal_pair(n):
   return int(n[0])*8 + int(n[1])

def opcode_enum(name):
    return "OPC_" + name

major_opc_gen_f = open('mips_major_dsp_special3_opcodes_gen.h', 'w');
opc_gen_f = open('mips_dsp_special3_opcodes_gen.h', 'w');
num_opcodes=0;

"""
 first do the major opcode groups. with dsp, each major group has
  a set of instructions distinguished by a secondary opcode. these major
  opcodes are 'special3' opcodes to qemu. in the hardware manual they occupy
  two octal digits, or 6 bits and are specified as pair of octal digits in the
  hardware manual. we enter them as in the manual and them make a hex
  code from those.
"""

major_opc_f = open('mips_major_dsp_special3_opcodes.h', 'w');

for key in sorted(special3_opcodes.keys()):
    major_special3_opcodes[key] = to_int_from_octal_pair (major_special3_opcodes[key]);
    key_c_name = opcode_enum(make_legal_c(key)) + '_major';
    major_opc_f.write('   ' + key_c_name + ' = ' + hex(major_special3_opcodes[key]) + ' | OPC_SPECIAL3,\n')

major_opc_f.close();

"""
For each special3 opcode, we create a function and in there process the
instructions associated with it. In Qemu this means we need to create the
cases and for that and then call the function.
"""
major_opc_case_f = open('mips_major_dsp_special3_opcodes_case.h', 'w');
for key in sorted(special3_opcodes.keys()):
    if (key != 'ADDUH.QB'):
        key_c_name = make_legal_c(key) + '_major';
        major_opc_case_f.write('        case ' + opcode_enum(key_c_name) + ':\n');
        major_opc_case_f.write('            gen_' + key_c_name + '(env, ctx);\n')
        major_opc_case_f.write('            break;\n');

major_opc_case_f.close()

"""
create the enumeration literals for each of the actual opcodes of the dsp ase.
"""
opc_f = open('mips_dsp_special3_opcodes.h', 'w')
for key in sorted(special3_opcodes.keys()):
    opc_f.write('/* special3 opcodes for ' + key + '*/\n')
    opc_f.write('enum {')
    first = True
    for key2 in sorted(special3_opcodes[key].keys()):
        if first:
            first = False
        else:
            opc_f.write(',')
        opc_f.write('\n');
        key2_c_name = make_legal_c(key2);
        special3_opcodes[key][key2] = to_int_from_octal_pair (special3_opcodes[key][key2]);
        opc_f.write('    ' + opcode_enum(key2_c_name) + ' = ' );
        opc_f.write(hex((special3_opcodes[key][key2] << 6) | major_special3_opcodes[key]));

        opc_f.write(' | OPC_SPECIAL3');
    opc_f.write('\n};\n\n')

"""
create the functions for each major opcode which call all it's secondary
opcodes.

"""

for key in sorted(special3_opcodes.keys()):
    key_c_name = make_legal_c(key) + '_major';
    default_signature = '(CPUState *env, DisasContext *ctx)'
    major_opc_gen_f.write('static void gen_' + key_c_name + default_signature+ '\n{\n')
    lower_mask = "0xfc0007ff"; # "0b111111" + 15*'0' + '11111' + '111111';
    major_opc_gen_f.write('    int opc = ctx->opcode & ' + lower_mask + ';\n')
    major_opc_gen_f.write('    switch (opc) {\n');
    for key2 in sorted(special3_opcodes[key].keys()):
        key2_c_name = make_legal_c(key2);
        major_opc_gen_f.write('    case ' + opcode_enum(key2_c_name) + ':\n');
        major_opc_gen_f.write('        gen_' + key2_c_name + '(env, ctx);\n')
        major_opc_gen_f.write('        break;\n');
    major_opc_gen_f.write('    }\n');
    major_opc_gen_f.write('}\n\n');

"""
create all the functions for each opcode.
"""

for key in sorted(special3_opcodes.keys()):
    key_c_name = make_legal_c(key) + '_major';
    default_signature = '(CPUState *env, DisasContext *ctx)'
    for key2 in sorted(special3_opcodes[key].keys()):
        key2_c_name = make_legal_c(key2);
        opc_gen_f.write('static void gen_' + key2_c_name + default_signature +
               '\n{\n')
        opc_gen_f.write(find_unpack(key2));
        if key2 in instructions:
            opc_gen_f.write(instructions[key2])
        num_opcodes = num_opcodes + 1;
        opc_gen_f.write('}\n\n');

#print "num opcodes = ", num_opcodes




