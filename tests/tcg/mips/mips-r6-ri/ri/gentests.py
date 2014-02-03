#!/usr/bin/python
import sys

IN_FILE = "startup_empty.s"
OUT_FILE = "startup.s"

#------------------------------------------------------------------------------
instr_dict = {}

def get_mult(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011000"
instr_dict["mult"] = get_mult

def get_multu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011001"
instr_dict["multu"] = get_multu

def get_div(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011010"
instr_dict["div"] = get_div

def get_divu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011011"
instr_dict["divu"] = get_divu

def get_dmult(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011100"
instr_dict["dmult"] = get_dmult

def get_dmultu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011101"
instr_dict["dmultu"] = get_dmultu

def get_ddiv(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011110"
instr_dict["ddiv"] = get_ddiv

def get_ddivu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+"00000"+"00000"+"011111"
instr_dict["ddivu"] = get_ddivu

def get_cop1ps(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"10110"+r5+r16
instr_dict["cop1ps"] = get_cop1ps

def get_bc1any2(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"10001"+r5+r16
instr_dict["bc1any2"] = get_bc1any2

def get_bc1any4(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"10010"+r5+r16
instr_dict["bc1any4"] = get_bc1any4

def get_ps(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010011"+r5+r5+r16
instr_dict["ps"] = get_ps

def get_movf(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+fcc+"00"+rd+"00000"+"000001"
instr_dict["movf"] = get_movf

def get_movt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+fcc+"01"+rd+"00000"+"000001"
instr_dict["movt"] = get_movt

def get_movffmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+fmt5+fcc+"00"+fs+fd+"010001"
instr_dict["movffmt"] = get_movffmt

def get_movtfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+fmt5+fcc+"01"+fs+fd+"010001"
instr_dict["movtfmt"] = get_movtfmt

def get_movzfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+fmt5+rt+fs+fd+"010010"
instr_dict["movzfmt"] = get_movzfmt

def get_movnfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+fmt5+rt+fs+fd+"010011"
instr_dict["movnfmt"] = get_movnfmt

def get_movz(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+rd+"00000"+"001010"
instr_dict["movz"] = get_movz

def get_movn(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+rt+rd+"00000"+"001011"
instr_dict["movn"] = get_movn

def get_beql(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010100"+rs+rt+r16
instr_dict["beql"] = get_beql

def get_bnel(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010101"+rs+rt+r16
instr_dict["bnel"] = get_bnel

def get_blezl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010110"+rs+"00000"+r16
instr_dict["blezl"] = get_blezl

def get_bgtzl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010111"+rs+"00000"+r16
instr_dict["bgtzl"] = get_bgtzl

def get_bltzl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"00010"+r16
instr_dict["bltzl"] = get_bltzl

def get_bgezl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"00011"+r16
instr_dict["bgezl"] = get_bgezl

def get_bltzall(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"10010"+r16
instr_dict["bltzall"] = get_bltzall

def get_bgezall(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"10011"+r16
instr_dict["bgezall"] = get_bgezall

def get_mfhi(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+"00000"+"00000"+rd+"00000"+"010000"
instr_dict["mfhi"] = get_mfhi

def get_mthi(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+"00000"+"00000"+"00000"+"010001"
instr_dict["mthi"] = get_mthi

def get_mflo(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+"00000"+"00000"+rd+"00000"+"010010"
instr_dict["mflo"] = get_mflo

def get_mtlo(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+"00000"+"00000"+"00000"+"010011"
instr_dict["mtlo"] = get_mtlo

def get_lwl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b100010"+r5+rt+r16
instr_dict["lwl"] = get_lwl

def get_lwr(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b100110"+r5+rt+r16
instr_dict["lwr"] = get_lwr

def get_ldl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011010"+r5+rt+r16
instr_dict["ldl"] = get_ldl

def get_ldr(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011011"+r5+rt+r16
instr_dict["ldr"] = get_ldr

def get_swl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b101010"+r5+rt+r16
instr_dict["swl"] = get_swl

def get_swr(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b101110"+r5+rt+r16
instr_dict["swr"] = get_swr

def get_sdl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b101100"+r5+rt+r16
instr_dict["sdl"] = get_sdl

def get_sdr(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b101101"+r5+rt+r16
instr_dict["sdr"] = get_sdr

def get_lwle(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011111"+r5+rt+r5+r4+"0011001"
instr_dict["lwle"] = get_lwle

def get_lwre(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011111"+r5+rt+r5+r4+"0011010"
instr_dict["lwre"] = get_lwre

def get_swle(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011111"+r5+rt+r5+r4+"0100001"
instr_dict["swle"] = get_swle

def get_swre(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011111"+r5+rt+r5+r4+"0100010"
instr_dict["swre"] = get_swre

def get_jr(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000000"+rs+"0000000000"+"0000"+"001000"
instr_dict["jr"] = get_jr

def get_ll(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b110000"+r5+rt+r16
instr_dict["ll"] = get_ll

def get_sc(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b111000"+r5+rt+r16
instr_dict["sc"] = get_sc

def get_lld(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b110100"+r5+rt+r16
instr_dict["lld"] = get_lld

def get_scd(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b111100"+r5+rt+r16
instr_dict["scd"] = get_scd

def get_pref(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b110011"+r5+rt+r16
instr_dict["pref"] = get_pref

def get_cache(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b101111"+r5+rt+r16
instr_dict["cache"] = get_cache

def get_tgei(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01000"+r16
instr_dict["tgei"] = get_tgei

def get_tgeiu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01001"+r16
instr_dict["tgeiu"] = get_tgeiu

def get_tlti(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01010"+r16
instr_dict["tlti"] = get_tlti

def get_tltiu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01011"+r16
instr_dict["tltiu"] = get_tltiu

def get_teqi(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01100"+r16
instr_dict["teqi"] = get_teqi

def get_tnei(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b000001"+rs+"01110"+r16
instr_dict["tnei"] = get_tnei

def get_clz(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+"00000"+"100000"
instr_dict["clz"] = get_clz

def get_clo(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+"00000"+"100001"
instr_dict["clo"] = get_clo

def get_dclz(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+"00000"+"100100"
instr_dict["dclz"] = get_dclz

def get_dclo(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+"00000"+"100101"
instr_dict["dclo"] = get_dclo

def get_sdbbp(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+r5+"111111"
instr_dict["sdbbp"] = get_sdbbp

def get_madd(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+"00000"+"00000"+"000000"
instr_dict["madd"] = get_madd

def get_maddu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+"00000"+"00000"+"000001"
instr_dict["maddu"] = get_maddu

def get_mul(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+rd+"00000"+"000010"
instr_dict["mul"] = get_mul

def get_msub(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+"00000"+"00000"+"000100"
instr_dict["msub"] = get_msub

def get_msubu(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b011100"+rs+rt+"00000"+"00000"+"000101"
instr_dict["msubu"] = get_msubu

def get_undef(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b111011"+rs+"1110"+r5+r5+r4+fcc
instr_dict["undef"] = get_undef

def get_maddfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010011"+rt+rd+fs+fd+"100"+"000"
instr_dict["maddfmt"] = get_maddfmt

def get_msubfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010011"+rt+rd+fs+fd+"101"+"000"
instr_dict["msubfmt"] = get_msubfmt

def get_nmaddfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010011"+rt+rd+fs+fd+"110"+"000"
instr_dict["msubfmt"] = get_msubfmt

def get_nmsubfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010011"+rt+rd+fs+fd+"111"+"000"
instr_dict["nmsubfmt"] = get_nmsubfmt

def get_cvtpss(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"10000"+"000000000000000"+"100110"
instr_dict["cvtpss"] = get_cvtpss

def get_cvtpspw(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"10100"+"000000000000000"+"100110"
instr_dict["cvtpspw"] = get_cvtpspw

def get_ccondfmt(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+fmt5+fs+fs+fcc+"0011"+r4
instr_dict["ccondfmt"] = get_ccondfmt

def get_bc1fl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"01000"+fcc+"10"+r16
instr_dict["bc1fl"] = get_bc1fl

def get_bc1tl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"01000"+fcc+"11"+r16
instr_dict["bc1tl"] = get_bc1tl

def get_bc2fl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010010"+"01000"+fcc+"10"+r16
instr_dict["bc2fl"] = get_bc2fl

def get_bc2tl(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010010"+"01000"+fcc+"11"+r16
instr_dict["bc2tl"] = get_bc2tl

def get_bc1f(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"01000"+fcc+"00"+r16
instr_dict["bc1f"] = get_bc1f

def get_bc1t(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010001"+"01000"+fcc+"01"+r16
instr_dict["bc1t"] = get_bc1t

def get_bc2f(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010010"+"01000"+fcc+"00"+r16
instr_dict["bc2f"] = get_bc2f

def get_bc2t(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    return "0b010010"+"01000"+fcc+"01"+r16
instr_dict["bc2t"] = get_bc2t

def get_bltzal(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    if rs == "00000":
        rs = "00001" # rs == 0 is NAL
    return "0b000001"+rs+"10000"+r16
instr_dict["bltzal"] = get_bltzal

def get_bgezal(rs, rt, rd, fs, fd, fmt, fmt5, fcc, r5, r16, r4):
    if rs == "00000":
        rs = "00001" # rs == 0 is NAL
    return "0b000001"+rs+"10001"+r16
instr_dict["bbgezal"] = get_bgezal


instr_dicts_keys = sorted(instr_dict.keys())
#------------------------------------------------------------------------------

def gen_asm(fout, instr, name):
    lstart = "        "
    lend = "\n"
    fout.write(lstart + "addiu $t8, $t8, 1" + lend)
    fout.write(lstart + ".word " + instr + " # " + name + lend)
    fout.write(lstart + "ehb" + lend)
    fout.write(lstart + "bne     $s0, $t8, test_failed" + lend)
    fout.write(lstart + "nop" + lend)
    return 

def gen_ssnop_asm(fout):
    # Check that SSNOP doesn't cause exception
    lstart = "        "
    lend = "\n"
    fout.write(lstart + ".word 0b00000000000000000000000001000000 # ssnop" + lend)
    fout.write(lstart + "ehb" + lend)
    fout.write(lstart + "bne     $s0, $t8, test_failed" + lend)
    fout.write(lstart + "nop" + lend)
    return

def gen_tests(fout):
    # Test 1: ssnop deprecated in R6. Check that no exception is raised
    gen_ssnop_asm(fout)

    # Test 2: all bits set to 1
    rs = "00000"
    rt = "00000"
    rd = "00000"
    fs = "00000"
    fd = "00000"
    r5 = "00000"
    r4 = "0000"
    fcc = "000"
    fmt = "000"
    fmt5 = "00000"
    r16 = "0000000000000000"
    for key in instr_dicts_keys:
        gen_asm(fout, instr_dict[key](rs,rt,rd,fs,fd,fmt,fmt5,fcc,r5,r16, r4), key)

    # Test 3: random bits set
    rs = "00100"
    rt = "01010"
    rd = "00010"
    fs = "01000"
    fd = "00100"
    r5 = "00100"
    r4 = "0100"
    fcc = "001"
    fmt = "010"
    fmt5 = "01000"
    r16 = "0000100100010000"
    for key in instr_dicts_keys:
        gen_asm(fout, instr_dict[key](rs,rt,rd,fs,fd,fmt,fmt5,fcc,r5,r16, r4), key)

    # Test 4: random bits set
    rs = "11101"
    rt = "01010"
    rd = "10110"
    fs = "01110"
    fd = "11110"
    r5 = "01101"
    r4 = "0110"
    fcc = "101"
    fmt = "110"
    fmt5 = "01010"
    r16 = "1011100101110101"
    for key in instr_dicts_keys:
        gen_asm(fout, instr_dict[key](rs,rt,rd,fs,fd,fmt,fmt5,fcc,r5,r16, r4), key)

    # Test 5: all bits set to 1
    rs = "11111"
    rt = "11111"
    rd = "11111"
    fs = "11111"
    fd = "11111"
    r5 = "11111"
    r4 = "1111"
    fcc = "111"
    fmt = "111"
    fmt5 = "11111"
    r16 = "1111111111111111"
    for key in instr_dicts_keys:
        gen_asm(fout, instr_dict[key](rs,rt,rd,fs,fd,fmt,fmt5,fcc,r5,r16, r4), key)

def main():
    with open(IN_FILE, "r") as fin, open(OUT_FILE, "w") as fout:
        for line in fin:
            fout.write(line)
            if line.startswith("test:"):
                gen_tests(fout)

if __name__ == '__main__':
    main()
    sys.exit(0)
