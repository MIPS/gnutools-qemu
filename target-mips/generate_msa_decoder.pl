#! /usr/local/bin/perl5.8.8 -w
# -*- mmm-classes: here-doc -*-

# <COPYRIGHT CLASS="1A" YEAR="2011">
# Unpublished work (c) MIPS Technologies, Inc.  All rights reserved.
# Unpublished rights reserved under the copyright laws of the U.S.A. and other
# countries.
#
# PROPRIETARY / SECRET CONFIDENTIAL INFORMATION OF MIPS TECHNOLOGIES, INC.
# FOR INTERNAL USE ONLY.
#
# Under no circumstances (contract or otherwise) may this information be
# disclosed to, or copied, modified or used by anyone other than employees or
# contractors of MIPS Technologies having a need to know.
# </COPYRIGHT>

use strict;
use warnings;

use English qw( -no_match_vars );

#$SIG{__DIE__} = sub { use Carp; confess @_ };
#$SIG{INT} = sub { use Carp; confess @_ };

my $inst_ref = read_instructions();

my %insts_for_mask;

foreach my $inst (@{$inst_ref}) {
    my $mask = $inst->{mask};

    push @{$insts_for_mask{$mask}}, $inst;
}

write_opcode_file("mips_msa_opcodes.h");

write_opcodes_case("mips_msa_opcodes_case.h");

write_opcodes_gen("mips_msa_opcodes_gen.h");

write_helper_decl("mips_msa_helper_decl.h");

write_helper_dummy("mips_msa_helper_dummy.h");


my %helper_def_of;
my %helper_type_of;
sub def_helper {
    my ($func_name,$decl,$func_type) = @_;

    die "ERROR: no helper def for $func_name, $func_type" if !defined $decl;

    my $existing = $helper_def_of{$func_name};
    die "ERROR: inconsistent helper declarations for $func_name ($existing, $decl,$func_type)" if $existing && $existing ne $decl;

    $helper_def_of{$func_name} = $decl;
    $helper_type_of{$func_name} = $func_type;

    return;
}

# create mips_msa_helper_decl.h
sub write_helper_decl {
    my ($filename) = @_;

    open(HELPERDECL, '>', $filename) or die "Can't open file $filename: $!\n";

    foreach my $func_name (sort keys %helper_def_of) {
        my $helper_decl = $helper_def_of{$func_name};

        print HELPERDECL "$helper_decl\n" if $helper_decl;
    }

    close HELPERDECL;
}

# create mips_msa_helper_dummy.h
sub write_helper_dummy {
    my ($filename) = @_;

    open(HELPERDUMMY, '>', $filename) or die "Can't open file $filename: $!\n";

    my @helper_lines = `grep helper op_helper.c`;

  FUNCLOOP:
    foreach my $func_name (sort keys %helper_def_of) {

        next FUNCLOOP if grep { /\shelper_$func_name\s*\(/xms } @helper_lines;

        my $helper_dummy = get_helper_dummy($func_name);

        print HELPERDUMMY "$helper_dummy\n";
    }

    close HELPERDUMMY;
}

# create mips_msa_opcodes_gen.h
sub write_opcodes_gen {
    my ($filename) = @_;

    open(OPCODEGEN, '>', $filename) or die "Can't open file $filename: $!\n";

    foreach my $inst (@{$inst_ref}) {
        my $func_body = get_func_body($inst);

        print OPCODEGEN $func_body;
    }

    close OPCODEGEN;
}

# create mips_msa_opcodes_case.h
sub write_opcodes_case {
    my ($filename) = @_;

    open(OPCASE, '>', $filename) or die "Can't open file $filename: $!\n";

    print OPCASE <<'HERECODE';
static void gen_msa(CPUState *env, DisasContext *ctx, int *is_branch)
{
    uint32_t opcode = ctx->opcode;

    /* !!! It must do longest prefix match here! */

HERECODE

    # reverse is a MUST for doing longest prefix match
    foreach my $mask (reverse sort keys %insts_for_mask) {
        my @insts = @{$insts_for_mask{$mask}};

        print OPCASE "    switch (opcode & $mask) {\n";

        foreach my $inst (@insts) {
            my $name = get_code_name($inst->{name});
            my $namelc = lc($name);
            my $is_branch = $namelc =~ /bnz_/ || $namelc =~ /bz_/;

            if ($is_branch) {
            print OPCASE <<"HERECODE";
        case OPC_$name:
            gen_$namelc(env, ctx);
            *is_branch = 1;
            return;
HERECODE
            }
            else {
            print OPCASE <<"HERECODE";
        case OPC_$name:
            gen_$namelc(env, ctx);
            return;
HERECODE
            }
        }

    print OPCASE <<'HERECODE';
    }

HERECODE

    }

    print OPCASE <<'HERECODE';
    MIPS_INVAL("MSA instruction");
    generate_exception(ctx, EXCP_RI);
} // end of gen_msa()
HERECODE

    close OPCASE;
}

# mips_msa_opcodes.h
sub write_opcode_file {
    my ($filename) = @_;

    open(OPCODES, '>', $filename) or die "Can't open file $filename: $!\n";

    print OPCODES <<'HERECODE';
/* MSA Opcodes */
enum {
HERECODE

    foreach my $inst (@{$inst_ref}) {

        my $name = $inst->{name};
        my $match = $inst->{match};

        my $code_name = get_code_name($name);

        my $spacestr = " " x (12 - length($code_name));
        print OPCODES "    OPC_$code_name$spacestr= $match | OPC_MSA,\n";
    }


    print OPCODES "};\n";

    close OPCODES;

    return;
}

sub get_helper_dummy {
    my ($func_name) = @_;

    my $helper_name = "helper_$func_name";
    my $func_type = $helper_type_of{$func_name};

    my $helper_body = "/* FIXME Unknown $func_type ($func_name) */";

    my $is_ld_v  = $func_name =~ /ld_df/ && !($func_name =~ /sld_df/);
    my $is_ldx_v = $func_name =~ /ldx_df/;
    my $is_st_v  = $func_name =~ /st_df/;
    my $is_stx_v = $func_name =~ /stx_df/;

    my $is_copy = $func_name =~ /copy/;

    if ($is_ld_v || $is_ldx_v || $is_st_v || $is_stx_v || $is_copy) {
        return "/* $func_name doesn't require helper function. */";
    }

    if ( $func_type eq 'df_wt_ws_wd' ) {
        $helper_body = <<"C_END";
int64_t $helper_name(int64_t arg1, int64_t arg2, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ( $func_type eq 'df_wt_ws_wd_wd' ) {
        $helper_body = <<"C_END";
int64_t $helper_name(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ($func_type eq 'df_u5_ws_wd' ) {
    $helper_body = <<"C_END";
int64_t $helper_name(int64_t arg1, int64_t arg2, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END

    }
    elsif ($func_type eq 'df_u5_ws_wd_wd' ) {
    $helper_body = <<"C_END";
int64_t $helper_name(int64_t dest, int64_t arg1, int64_t arg2, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END

    }
    elsif ($func_type eq 'df_s5_ws_wd' ) {
           $helper_body = <<"C_END";
int64_t $helper_name(int64_t arg1, int64_t arg2, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ($func_type eq 'wt_ws_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, void * pwt, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'df_wt_ws_wd_p') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, void * pwt, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'df_ws_wd_p') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'i8_ws_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, uint32_t imm, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'dfm_ws_wd') {
        $helper_body = <<"C_END";
int64_t $helper_name(int64_t s, uint32_t m, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ($func_type eq 'dfm_ws_wd_wd') {
        $helper_body = <<"C_END";
int64_t $helper_name(int64_t s, int64t s, uint32_t m, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ($func_type eq 'df_ws_wd') {
        $helper_body = <<"C_END";
int64_t $helper_name(int64_t arg, uint32_t df)
{
    printf("%s()\\n", __func__);
    return 0; // FIXME
}

C_END
    }
    elsif ($func_type eq 'df_s10_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t df, uint32_t s10, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'df_s10_wd_branch') {
        $helper_body = <<"C_END";
uint32_t $helper_name(void * pwd, uint32_t df, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
    return 0;
}

C_END
    }
    elsif ($func_type eq 's10_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t s10, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 's10_wd_branch') {
        $helper_body = <<"C_END";
uint32_t $helper_name(void * pwd, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
    return 0;
}

C_END
    }
    elsif ($func_type eq 'dfn_ws_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, uint32_t n, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'dfn_rs_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t rs, uint32_t n, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'df_rt_ws_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, uint32_t rt, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'df_rt_rs_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t rs, uint32_t rt, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'ws_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, void * pws, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 's5_rs_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t s5, uint32_t rs, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'rt_rs_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, target_ulong rt, target_ulong rs, uint32_t wrlen)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'dfn_ws_rd') {
    }
    elsif ($func_type eq 'df_rt_ws_rd') {
    }
    elsif ($func_type eq 'df_rs_wd') {
        $helper_body = <<"C_END";
void $helper_name(void * pwd, uint32_t rs, uint32_t wrlen_df)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'rs_cd') {
        $helper_body = <<"C_END";
void $helper_name(target_ulong elm, uint32_t cd)
{
    printf("%s()\\n", __func__);
}

C_END
    }
    elsif ($func_type eq 'cs_rd') {
        $helper_body = <<"C_END";
target_ulong $helper_name(uint32_t cs)
{
    printf("%s()\\n", __func__);
    return 0;
}

C_END
    }
    else {
        die "ERROR: don't know what to do for helper declaration '$func_type' ($func_name)";
    }

    return $helper_body;
}

sub get_func_body {
    my ($inst) = @_;

    my $name = $inst->{name};

    my $opcode = $inst->{opcode};

    my $codename = lc(get_code_name($name));

    my $declare_str;

    foreach my $field (@{$opcode}) {
        my ($max,$min,$fieldname) = @{$field};

        my $shift = $min;
        my $mask = sprintf("0x%x", (1 << (1 + $max-$min)) - 1 );

        my $is_signed = $fieldname =~ /s5|s10/;

        if ($is_signed) {
            $declare_str .= <<C_END;
    int64_t $fieldname = (ctx->opcode >> $shift) & $mask /* $fieldname \[$max:$min\] */;
C_END

            if ($fieldname eq 's5') {
                $declare_str .= <<C_END;
    $fieldname = ($fieldname << 59) >> 59; /* sign extend s5 to 64 bits*/
C_END
            }
        }
        else {
            $declare_str .= <<C_END;
    uint8_t $fieldname = (ctx->opcode >> $shift) & $mask /* $fieldname \[$max:$min\] */;
C_END
        }

        if ($fieldname eq 'df') {
            # adjust df value for Fixed-Point and Floating-Point instructions
            my $is_floating = ($name =~ /\AF/) && !($name =~ /\AFILL/);
            my $is_fixed = $name =~ /_Q/;
            if ($is_floating) {
                $declare_str .= <<C_END;


    /* adjust df value for floating-point instruction */
    df = df + 2;
C_END
            } elsif ($is_fixed) {
                $declare_str .= <<C_END;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
C_END
            }
        }

        if ($fieldname eq 'dfn') {
            $declare_str .= <<C_END;

    uint32_t df = 0, n = 0;  /* may be used uninitialized */

    if ((dfn & 0x20) == 0x00) {         /* byte data format */
        n = dfn & 0x1f;
        df = 0;
    } else if ((dfn & 0x30) == 0x20) {  /* half data format */
        n = dfn & 0x0f;
        df = 1;
    } else if ((dfn & 0x38) == 0x30) {  /* word data format */
        n = dfn & 0x07;
        df = 2;
    } else if ((dfn & 0x3c) == 0x38) {  /* double data format */
        n = dfn & 0x3;
        df = 3;
    } else {
        generate_exception(ctx, EXCP_RI);
    }

    int df_bits = 8 * (1 << df);
    if ( n >= wrlen / df_bits ) {
        generate_exception(ctx, EXCP_RI);
    }

C_END

        my $is_copy = ($codename =~ /copy_[su]_df/);
        my $is_insert = ($codename =~ /insert_df/);

        if ( $is_copy || $is_insert ) {
            $declare_str .= <<C_END;
#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        generate_exception(ctx, EXCP_RI);
    }
#endif

C_END
        }

        }
        elsif ($fieldname eq 'dfm') {
            $declare_str .= <<C_END;

    uint32_t df = 0, m = 0;  /* may be used uninitialized */

    if ((dfm & 0x40) == 0x00) {         /* double data format */
        m = dfm & 0x3f;
        df = 3;
    } else if ((dfm & 0x60) == 0x40) {  /* word data format */
        m = dfm & 0x1f;
        df = 2;
    } else if ((dfm & 0x70) == 0x60) {  /* half data format */
        m = dfm & 0x0f;
        df = 1;
    } else if ((dfm & 0x78) == 0x70) {  /* byte data format */
        m = dfm & 0x7;
        df = 0;
    } else {
        generate_exception(ctx, EXCP_RI);
    }

C_END
        }
    }

    my $func_type = get_func_type($inst);
    my $helper_name = lc(get_helper_name($name));

    my $func_body = <<"C_END";
static void gen_$codename(CPUState *env, DisasContext *ctx) {
    /* func_type = $func_type */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;

C_END

    my $def;

    my $is_ld_v  = $codename =~ /ld_df/ && !($codename =~ /sld_df/);
    my $is_ldx_v = $codename =~ /ldx_df/;
    my $is_st_v  = $codename =~ /st_df/;
    my $is_stx_v = $codename =~ /stx_df/;

    if ($is_ld_v) {
        $func_body .= <<"C_END";
$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 twd = tcg_const_i32(wd);

    // set element granularity to 32 bits, in line with tcg_gen_qemu_ld32s()
    if (df != 2) df = 2; /* FIXME: use df not 'w' format */
    int df_bits = 8 * (1 << df);
    int16_t offset = s5 * (wrlen/8);

    int i;
    TCGv td = tcg_temp_new();
    TCGv taddr = tcg_temp_new();
    TCGv_i32 tdf = tcg_const_i32(df);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        gen_base_offset_addr(ctx, taddr, rs, offset + i*df_bits/8);
        tcg_gen_qemu_ld32s(td, taddr, ctx->mem_idx);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(twd);
    tcg_temp_free(td);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
C_END
        $def = ''; # no helper required
    } elsif ($is_ldx_v) {
        $func_body .= <<"C_END";
$declare_str
    check_msa_access(env, ctx, wd, -1, -1);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_i32 twd = tcg_const_i32(wd);

    // set element granularity to 32 bits, in line with tcg_gen_qemu_ld32s()
    if (df != 2) df = 2; /* FIXME: use df not 'w' format */
    int df_bits = 8 * (1 << df);

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    TCGv taddr = tcg_temp_new();
    gen_op_addr_add(ctx, taddr, trs, trt);

    int i;
    TCGv td = tcg_temp_new();
    TCGv telemoff  = tcg_temp_new(); /* element offset */
    TCGv telemaddr = tcg_temp_new(); /* element addr */
    TCGv_i32 tdf = tcg_const_i32(df);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        tcg_gen_movi_tl(telemoff, i*df_bits/8);
        gen_op_addr_add(ctx, telemaddr, taddr, telemoff);
        tcg_gen_qemu_ld32s(td, telemaddr, ctx->mem_idx);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twd);
    tcg_temp_free(td);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);
    tcg_temp_free(telemoff);
    tcg_temp_free(telemaddr);

    update_msa_modify(env, ctx, wd);
C_END
        $def = ''; # no helper required
    } elsif ($is_st_v) {
        $func_body .= <<"C_END";
$declare_str
    check_msa_access(env, ctx, wd, -1, -1);

    TCGv_i32 twd = tcg_const_i32(wd);

    // set element granularity to 32 bits, in line with tcg_gen_qemu_st32()
    if (df != 2) df = 2; /* FIXME: use df not 'w' format */
    int df_bits = 8 * (1 << df);
    int16_t offset = s5 * (wrlen/8);

    int i;
    TCGv td = tcg_temp_new();
    TCGv taddr = tcg_temp_new();
    TCGv_i32 tdf = tcg_const_i32(df);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_i64(td, twd, tdf, ti);
        gen_base_offset_addr(ctx, taddr, rs, offset + i*df_bits/8);
        tcg_gen_qemu_st32(td, taddr, ctx->mem_idx);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(twd);
    tcg_temp_free(td);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
C_END
        $def = ''; # no helper required
    } elsif ($is_stx_v) {
        $func_body .= <<"C_END";
$declare_str
    check_msa_access(env, ctx, wd, -1, -1);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_i32 twd = tcg_const_i32(wd);

    // set element granularity to 32 bits, in line with tcg_gen_qemu_ld32s()
    if (df != 2) df = 2; /* FIXME: use df not 'w' format  */
    int df_bits = 8 * (1 << df);

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    TCGv taddr = tcg_temp_new();
    gen_op_addr_add(ctx, taddr, trs, trt);

    int i;
    TCGv td = tcg_temp_new();
    TCGv telemoff  = tcg_temp_new(); /* element offset */
    TCGv telemaddr = tcg_temp_new(); /* element addr */
    TCGv_i32 tdf = tcg_const_i32(df);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_i64(td, twd, tdf, ti);
        tcg_gen_movi_tl(telemoff, i*df_bits/8);
        gen_op_addr_add(ctx, telemaddr, taddr, telemoff);
        tcg_gen_qemu_st32(td, telemaddr, ctx->mem_idx);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twd);
    tcg_temp_free(td);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);
    tcg_temp_free(telemoff);
    tcg_temp_free(telemaddr);

    update_msa_modify(env, ctx, wd);
C_END
        $def = ''; # no helper required
    } elsif ( $func_type eq 'df_wt_ws_wd' ) {

        my $stype = get_arg_type($inst,'ws');
        my $ttype = get_arg_type($inst,'wt');
        my $dtype = get_arg_type($inst,'wd');

        my $is_dotp = ($codename =~ /dotp_[su]_df/);
        my $is_hadd = ($codename =~ /hadd_[su]_df/);
        my $is_hsub = ($codename =~ /hsub_[su]_df/);

          $func_body .= <<"C_END";

$declare_str
C_END

        if ( $is_dotp || $is_hadd || $is_hsub ) {
          $func_body .= <<"C_END";
    /* check df: byte format not allowed */
    if (df == 0) {
        generate_exception(ctx, EXCP_RI);
    }

C_END
        }

        $func_body .= <<"C_END";
    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_$stype(ts, tws, tdf, ti);
        gen_helper_load_wr_modulo_$ttype(tt, twt, tdf, ti);
        gen_helper_$helper_name(td, ts, tt, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(td);
    tcg_temp_free(ts);
    tcg_temp_free(tt);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(twt);
    tcg_temp_free(tdf);

    update_msa_modify(env, ctx, wd);
C_END

       $def = "DEF_HELPER_3($helper_name, $dtype, $stype, $ttype, i32)";
    }
    elsif ( $func_type eq 'df_wt_ws_wd_wd' ) {

        my $is_dpadd = ($codename =~ /dpadd_[su]_df/);
        my $is_dpsub = ($codename =~ /dpsub_[su]_df/);

        # same as df_wt_ws_wd but wd used as input too
        $func_body .= <<"C_END";

$declare_str
C_END

        if ( $is_dpadd || $is_dpsub ) {
          $func_body .= <<"C_END";
    /* check df: byte format not allowed */
    if (df == 0) {
        generate_exception(ctx, EXCP_RI);
    }

C_END
        }

        $func_body .= <<"C_END";
    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_modulo_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_modulo_s64(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, tt, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(td);
    tcg_temp_free(ts);
    tcg_temp_free(tt);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(twt);
    tcg_temp_free(tdf);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, s64, s64, s64, s64, i32)";
    }
    elsif (   $func_type eq 'df_u5_ws_wd'
           || $func_type eq 'df_s5_ws_wd' ) {

        my ($imm) = $func_type =~ /([su]5)/xms or die "bad match";

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');
        my $immtype = get_arg_type($inst,$imm);

        $func_body .= <<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 t$imm = tcg_const_i64($imm);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_$stype(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, t$imm, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(t$imm);
    tcg_temp_free(td);
    tcg_temp_free(ts);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, $dtype, $stype, $immtype, i32)";
    }
    elsif (   $func_type eq 'df_u5_ws_wd_wd'
           || $func_type eq 'df_s5_ws_wd_wd' ) {

        my ($imm) = $func_type =~ /([su]5)/xms or die "bad match";

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');
        my $immtype = get_arg_type($inst,$imm);

        $func_body .= <<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 t$imm = tcg_const_i64($imm);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_$stype(ts, tws, tdf, ti);
        gen_helper_load_wr_modulo_$dtype(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, t$imm, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(t$imm);
    tcg_temp_free(td);
    tcg_temp_free(ts);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, $dtype, $dtype, $stype, $immtype, i32)";
    }
    elsif ($func_type eq 'df_wt_ws_wd_p') {

        $func_body .= <<"C_END";

$declare_str
    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_$helper_name(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, ptr, i32)";

    }
    elsif ($func_type eq 'df_ws_wd_p') {

        $func_body .= <<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_$helper_name(tpwd, tpws, twrlen_df);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, void, ptr, ptr, i32)";

    }
    elsif ($func_type eq 'wt_ws_wd') {

        $func_body .= <<"C_END";

$declare_str
    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_$helper_name(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, ptr, i32)";

    }
    elsif ($func_type eq 'i8_ws_wd') {
        my $imm = 'i8';

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 t$imm = tcg_const_i32($imm); // FIXME

    gen_helper_$helper_name(tpwd, tpws, t$imm, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(t$imm);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd') {

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_s64(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, tm, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, s64, s64, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd_wd') {

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_modulo_s64(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, tm, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, s64, s64, s64, i32, i32)";
    }
    elsif ($func_type eq 'df_ws_wd') {

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_modulo_$stype(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, tdf);
        gen_helper_store_wr_modulo(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_2($helper_name, $dtype, $stype, i32)";
    }
    elsif ($func_type eq 'df_s10_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_$helper_name(tpwd, tdf, ts10, twrlen);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_s10_wd_branch') {

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, -1);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();

    gen_helper_$helper_name(tbcond, tpwd, tdf, twrlen);

    int64_t offset = (s10 << 54) >> 52;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */
    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, i32, ptr, i32, i32)";
    }
    elsif ($func_type eq 's10_wd_branch') {

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, -1);

    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  =
tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();
    gen_helper_$helper_name(tbcond, tpwd, twrlen);

    int64_t offset = (s10 << 54) >> 52;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */
    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_2($helper_name, i32, ptr, i32)";
    }
    elsif ($func_type eq 's10_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_$helper_name(tpwd, ts10, twrlen);

    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, void, i64, i32, i32)";
    }
    elsif ($func_type eq 'dfn_ws_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_$helper_name(tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

      $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, i32, i32)";
    }
    elsif ($func_type eq 'dfn_rs_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, trs, tn, twrlen_df);

    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

      $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_rt_rs_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, trs, trt, twrlen_df);

    tcg_temp_free(trs);
    tcg_temp_free(trt);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

      $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_rt_ws_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv trt = tcg_temp_new();
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trt, rt);
    gen_helper_$helper_name(tpwd, tpws, trt, twrlen_df);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free(trt);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

      $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, i32, i32)";
    }
    elsif ($func_type eq 'ws_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_$helper_name(tpwd, tpws, twrlen);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END
      $def = "DEF_HELPER_3($helper_name, void, ptr, ptr, i32)";
    }
    elsif ($func_type eq 's5_rs_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts5 = tcg_const_i32(s5);
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));
    TCGv trs = tcg_temp_new();

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, ts5, trs, twrlen);

    tcg_temp_free_i32(ts5);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'rt_rs_wd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, trt, trs, twrlen);

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
C_END

       $def = "DEF_HELPER_4($helper_name, void, ptr, tl, tl, i32)";
    }
    elsif ($func_type eq 'dfn_ws_rd') {

        my $stype = get_arg_type($inst,'ws');

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, -1);

    TCGv telm = tcg_temp_new();
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_load_wr_modulo_$stype(telm, tws, tdf, tn);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
C_END
        $def = ''; # no helper required
    }
    elsif ($func_type eq 'df_rt_ws_rd') {

        my $stype = get_arg_type($inst,'ws');

        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, ws, ws, -1);

    TCGv telm = tcg_temp_new();
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv trt = tcg_temp_new();

    gen_load_gpr(trt, rt);
    gen_helper_load_wr_modulo_$stype(telm, tws, tdf, trt);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free(trt);

C_END
        $def = ''; # no helper required
    }
    elsif ($func_type eq 'df_rs_wd') {
        $func_body .=<<"C_END";

$declare_str
#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        generate_exception(ctx, EXCP_RI);
    }
#endif

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, trs, twrlen_df);

    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
C_END

        $def = "DEF_HELPER_3($helper_name, void, ptr, i32, i32)";


    }
    elsif ($func_type eq 'rs_cd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, -1, -1, -1);

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcd = tcg_const_i32(cd);

    gen_load_gpr(telm, rs);
    gen_helper_$helper_name(telm, tcd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcd);

C_END

        $def = "DEF_HELPER_2($helper_name, void, tl, i32)";
    }
    elsif ($func_type eq 'cs_rd') {
        $func_body .=<<"C_END";

$declare_str
    check_msa_access(env, ctx, -1, -1, -1);

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcs = tcg_const_i32(cs);

    gen_helper_$helper_name(telm, tcs);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcs);

C_END

    $def = "DEF_HELPER_1($helper_name, tl, i32)";
    }

    else {

        $func_body .= <<C_END;
    // TODO $name ($func_type)
    assert(0);
C_END
#        die "ERROR: don't know what to do for func_type '$func_type' ($name)";

    }

    $func_body .= <<C_END;
}

C_END

    def_helper($helper_name,$def,$func_type);

    return $func_body;
};

my %func_type_of;
sub get_func_type {
    my $inst = shift;

    my $opcode = $inst->{opcode};
    my $name = $inst->{name};

    my @fields;
    foreach my $field (@{$opcode}) {
        my ($max,$min,$fieldname) = @{$field};
        push @fields, $fieldname;
    }

    my $default_func_type = join("_",@fields);

    %func_type_of = (
        'LD.df' => 's5_rs_wd',
        'ST.df' => 's5_rs_wd',
        'BINSL.df' => 'df_wt_ws_wd_wd',
        'BINSR.df' => 'df_wt_ws_wd_wd',
        'BINSLI.df' => 'dfm_ws_wd_wd',
        'BINSRI.df' => 'dfm_ws_wd_wd',
        'DPADD_S.df' => 'df_wt_ws_wd_wd',
        'DPADD_U.df' => 'df_wt_ws_wd_wd',
        'DPSUB_S.df' => 'df_wt_ws_wd_wd',
        'DPSUB_U.df' => 'df_wt_ws_wd_wd',
        'BNZ.df' => 'df_s10_wd_branch',
        'BZ.df' => 'df_s10_wd_branch',
        'BNZ.V' => 's10_wd_branch',
        'BZ.V' => 's10_wd_branch',
        'ILVEV.df' => 'df_wt_ws_wd_p',
        'ILVOD.df' => 'df_wt_ws_wd_p',
        'ILVL.df' => 'df_wt_ws_wd_p',
        'ILVR.df' => 'df_wt_ws_wd_p',
        'PCKEV.df' => 'df_wt_ws_wd_p',
        'PCKOD.df' => 'df_wt_ws_wd_p',

        'FADD.df' => 'df_wt_ws_wd_p',
        'FSUB.df' => 'df_wt_ws_wd_p',
        'FMUL.df' => 'df_wt_ws_wd_p',
        'FDIV.df' => 'df_wt_ws_wd_p',
        'FEXP2.df' => 'df_wt_ws_wd_p',

        'FMAX.df' => 'df_wt_ws_wd_p',
        'FMAX_A.df' => 'df_wt_ws_wd_p',
        'FMIN.df' => 'df_wt_ws_wd_p',
        'FMIN_A.df' => 'df_wt_ws_wd_p',

        'FCAF.df' => 'df_wt_ws_wd_p',
        'FCOR.df' => 'df_wt_ws_wd_p',
        'FCUEQ.df' => 'df_wt_ws_wd_p',
        'FCULE.df' => 'df_wt_ws_wd_p',
        'FCULT.df' => 'df_wt_ws_wd_p',
        'FCUNE.df' => 'df_wt_ws_wd_p',

        'FCEQ.df' => 'df_wt_ws_wd_p',
        'FCEQU.df' => 'df_wt_ws_wd_p',
        'FCLEU.df' => 'df_wt_ws_wd_p',
        'FCLTU.df' => 'df_wt_ws_wd_p',


        'FCUN.df' => 'df_wt_ws_wd_p',
        'FCEQ.df' => 'df_wt_ws_wd_p',
        'FCNE.df' => 'df_wt_ws_wd_p',
        'FCLT.df' => 'df_wt_ws_wd_p',
        'FCLE.df' => 'df_wt_ws_wd_p',

        'FSAF.df' => 'df_wt_ws_wd_p',
        'FSOR.df' => 'df_wt_ws_wd_p',
        'FSUEQ.df' => 'df_wt_ws_wd_p',
        'FSULE.df' => 'df_wt_ws_wd_p',
        'FSULT.df' => 'df_wt_ws_wd_p',
        'FSUN.df' => 'df_wt_ws_wd_p',
        'FSUNE.df' => 'df_wt_ws_wd_p',

        'FSEQ.df' => 'df_wt_ws_wd_p',
        'FSNE.df' => 'df_wt_ws_wd_p',
        'FSLT.df' => 'df_wt_ws_wd_p',
        'FSLE.df' => 'df_wt_ws_wd_p',

        'FSQRT.df' => 'df_ws_wd_p',
        'FLOG2.df' => 'df_ws_wd_p',

        'FFINT_S.df' => 'df_ws_wd_p',
        'FFINT_U.df' => 'df_ws_wd_p',
        'FTINT_S.df' => 'df_ws_wd_p',
        'FTINT_U.df' => 'df_ws_wd_p',
        'FRINT.df' => 'df_ws_wd_p',

        'FTRUNC_S.df' => 'df_ws_wd_p',
        'FTRUNC_U.df' => 'df_ws_wd_p',

        'FRCP.df' => 'df_ws_wd_p',
        'FRSQRT.df' => 'df_ws_wd_p',


        'FMADD.df' => 'df_wt_ws_wd_p',
        'FMSUB.df' => 'df_wt_ws_wd_p',


        'FEXUPL.df' => 'df_ws_wd_p',
        'FEXUPR.df' => 'df_ws_wd_p',

        'FEXDO.df' => 'df_wt_ws_wd_p',

        'FFQL.df'   => 'df_ws_wd_p',
        'FFQR.df'   => 'df_ws_wd_p',

        'FTQ.df'   => 'df_wt_ws_wd_p',

        'MADDV.df' => 'df_wt_ws_wd_wd',
        'MSUBV.df' => 'df_wt_ws_wd_wd',
        'MADD_Q.df' => 'df_wt_ws_wd_wd',
        'MSUB_Q.df' => 'df_wt_ws_wd_wd',
        'MADDR_Q.df' => 'df_wt_ws_wd_wd',
        'MSUBR_Q.df' => 'df_wt_ws_wd_wd',
        'VSHF.df' => 'df_wt_ws_wd_p',
    ) if !%func_type_of;

    my $func_type = $func_type_of{$name} || $default_func_type;

    return $func_type;
}


my %is_unsigned;
sub get_arg_type {
    my ($inst,$arg) = @_;

    my $name = $inst->{name};

    %is_unsigned = map {$_ => 1} (
        'ADDS_U.df/wd',
        'ADDS_U.df/ws',
        'ADDS_U.df/wt',

        'ADDSI_U.df/wd',
        'ADDSI_U.df/ws',
        'ADDSI_U.df/u5',

        'AVE_U.df/wd',
        'AVE_U.df/ws',
        'AVE_U.df/wt',

        'AVER_U.df/wd',
        'AVER_U.df/ws',
        'AVER_U.df/wt',

        'ASUB_U.df/wd',
        'ASUB_U.df/ws',
        'ASUB_U.df/wt',

        'COPY_U.df/ws',
    ) if !%is_unsigned;

    my $arg_type_of = $is_unsigned{"$name/$arg"} ? 'i64' : 's64';

    return $arg_type_of;
}

my %helpername_of;
sub get_helper_name {
    my $inst = shift;

    # can be used to allow instructions to share a helper
    %helpername_of = (
        'ADDVI_df' => 'ADDV_df',
        'ADDI_A_df' => 'ADD_A_df',
        'ADDSI_A_df' => 'ADDS_A_df',
        'ADDSI_S_df' => 'ADDS_S_df',
        'ADDSI_U_df' => 'ADDS_U_df',
        'SUBVI_df' => 'SUBV_df',
        'MAXI_S_df' => 'MAX_S_df',
        'MAXI_U_df' => 'MAX_U_df',
        'MINI_A_df' => 'MIN_A_df',
        'MINI_S_df' => 'MIN_S_df',
        'MINI_U_df' => 'MIN_U_df',
        'CEQI_df' => 'CEQ_df',
        'CLTI_S_df' => 'CLT_S_df',
        'CLEI_S_df' => 'CLE_S_df',
        'CLTI_U_df' => 'CLT_U_df',
        'CLEI_U_df' => 'CLE_U_df',
        'SLDI_df' => 'SLD_df',
        'SPLATI_df' => 'SPLAT_df',
    ) if !%helpername_of;

    my $codename = get_code_name($inst);
    my $helper_name = $helpername_of{$codename} || $codename;

    return $helper_name;
}

sub get_code_name {
    my ($name) = @_;

    $name =~ s/[.]/_/;

    return $name;
}

# read list of instructions as Data::Dumper eval from stdin
sub read_instructions {
#    my $input = join('',<>);
#    my $VAR1;
#    my $inst_ref = eval($input);
#    die "ERROR: $EVAL_ERROR" if $EVAL_ERROR;#

    return [
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800000d',
            'name' => 'SLL.df',
            'match_mm' => '0xc8000016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000009',
            'name' => 'SLLI.df',
            'match_mm' => '0xc800000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880000d',
            'name' => 'SRA.df',
            'match_mm' => '0xc8800016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800009',
            'name' => 'SRAI.df',
            'match_mm' => '0xc880000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900000d',
            'name' => 'SRL.df',
            'match_mm' => '0xc9000016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000009',
            'name' => 'SRLI.df',
            'match_mm' => '0xc900000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980000d',
            'name' => 'BCLR.df',
            'match_mm' => '0xc9800016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800009',
            'name' => 'BCLRI.df',
            'match_mm' => '0xc980000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a00000d',
            'name' => 'BSET.df',
            'match_mm' => '0xca000016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000009',
            'name' => 'BSETI.df',
            'match_mm' => '0xca00000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80000d',
            'name' => 'BNEG.df',
            'match_mm' => '0xca800016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800009',
            'name' => 'BNEGI.df',
            'match_mm' => '0xca80000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00000d',
            'name' => 'BINSL.df',
            'match_mm' => '0xcb000016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000009',
            'name' => 'BINSLI.df',
            'match_mm' => '0xcb00000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80000d',
            'name' => 'BINSR.df',
            'match_mm' => '0xcb800016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800009',
            'name' => 'BINSRI.df',
            'match_mm' => '0xcb80000f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800000e',
            'name' => 'ADDV.df',
            'match_mm' => '0xc8000017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000006',
            'name' => 'ADDVI.df',
            'match_mm' => '0xc800000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880000e',
            'name' => 'SUBV.df',
            'match_mm' => '0xc8800017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800006',
            'name' => 'SUBVI.df',
            'match_mm' => '0xc880000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900000e',
            'name' => 'MAX_S.df',
            'match_mm' => '0xc9000017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000006',
            'name' => 'MAXI_S.df',
            'match_mm' => '0xc900000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980000e',
            'name' => 'MAX_U.df',
            'match_mm' => '0xc9800017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800006',
            'name' => 'MAXI_U.df',
            'match_mm' => '0xc980000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a00000e',
            'name' => 'MIN_S.df',
            'match_mm' => '0xca000017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000006',
            'name' => 'MINI_S.df',
            'match_mm' => '0xca00000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80000e',
            'name' => 'MIN_U.df',
            'match_mm' => '0xca800017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800006',
            'name' => 'MINI_U.df',
            'match_mm' => '0xca80000a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00000e',
            'name' => 'MAX_A.df',
            'match_mm' => '0xcb000017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80000e',
            'name' => 'MIN_A.df',
            'match_mm' => '0xcb800017',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800000f',
            'name' => 'CEQ.df',
            'match_mm' => '0xc8000019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000007',
            'name' => 'CEQI.df',
            'match_mm' => '0xc800000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900000f',
            'name' => 'CLT_S.df',
            'match_mm' => '0xc9000019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000007',
            'name' => 'CLTI_S.df',
            'match_mm' => '0xc900000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980000f',
            'name' => 'CLT_U.df',
            'match_mm' => '0xc9800019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800007',
            'name' => 'CLTI_U.df',
            'match_mm' => '0xc980000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a00000f',
            'name' => 'CLE_S.df',
            'match_mm' => '0xca000019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000007',
            'name' => 'CLEI_S.df',
            'match_mm' => '0xca00000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80000f',
            'name' => 'CLE_U.df',
            'match_mm' => '0xca800019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'u5'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'u5'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800007',
            'name' => 'CLEI_U.df',
            'match_mm' => '0xca80000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000007',
            'name' => 'LD.df',
            'match_mm' => '0xcb00000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            's5'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               's5'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800007',
            'name' => 'ST.df',
            'match_mm' => '0xcb80000b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'rt'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'rt'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00000f',
            'name' => 'LDX.df',
            'match_mm' => '0xcb000019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'rt'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'rt'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80000f',
            'name' => 'STX.df',
            'match_mm' => '0xcb800019',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800000a',
            'name' => 'SAT_S.df',
            'match_mm' => '0xc8000011',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880000a',
            'name' => 'SAT_U.df',
            'match_mm' => '0xc8800011',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000010',
            'name' => 'ADD_A.df',
            'match_mm' => '0xc800001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800010',
            'name' => 'ADDS_A.df',
            'match_mm' => '0xc880001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000010',
            'name' => 'ADDS_S.df',
            'match_mm' => '0xc900001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800010',
            'name' => 'ADDS_U.df',
            'match_mm' => '0xc980001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000010',
            'name' => 'AVE_S.df',
            'match_mm' => '0xca00001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800010',
            'name' => 'AVE_U.df',
            'match_mm' => '0xca80001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000010',
            'name' => 'AVER_S.df',
            'match_mm' => '0xcb00001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800010',
            'name' => 'AVER_U.df',
            'match_mm' => '0xcb80001a',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000011',
            'name' => 'SUBS_S.df',
            'match_mm' => '0xc800001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800011',
            'name' => 'SUBS_U.df',
            'match_mm' => '0xc880001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000011',
            'name' => 'SUBSUS_U.df',
            'match_mm' => '0xc900001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800011',
            'name' => 'SUBSUU_S.df',
            'match_mm' => '0xc980001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000011',
            'name' => 'ASUB_S.df',
            'match_mm' => '0xca00001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800011',
            'name' => 'ASUB_U.df',
            'match_mm' => '0xca80001b',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000012',
            'name' => 'MULV.df',
            'match_mm' => '0xc800001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800012',
            'name' => 'MADDV.df',
            'match_mm' => '0xc880001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000012',
            'name' => 'MSUBV.df',
            'match_mm' => '0xc900001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000012',
            'name' => 'DIV_S.df',
            'match_mm' => '0xca00001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800012',
            'name' => 'DIV_U.df',
            'match_mm' => '0xca80001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000012',
            'name' => 'MOD_S.df',
            'match_mm' => '0xcb00001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800012',
            'name' => 'MOD_U.df',
            'match_mm' => '0xcb80001e',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000013',
            'name' => 'DOTP_S.df',
            'match_mm' => '0xc800001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800013',
            'name' => 'DOTP_U.df',
            'match_mm' => '0xc880001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000013',
            'name' => 'DPADD_S.df',
            'match_mm' => '0xc900001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800013',
            'name' => 'DPADD_U.df',
            'match_mm' => '0xc980001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000013',
            'name' => 'DPSUB_S.df',
            'match_mm' => '0xca00001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800013',
            'name' => 'DPSUB_U.df',
            'match_mm' => '0xca80001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'rt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'rt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000014',
            'name' => 'SLD.df',
            'match_mm' => '0xc8000021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000019',
            'name' => 'SLDI.df',
            'match_mm' => '0xc800002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'rt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'rt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800014',
            'name' => 'SPLAT.df',
            'match_mm' => '0xc8800021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78400019',
            'name' => 'SPLATI.df',
            'match_mm' => '0xc840002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000014',
            'name' => 'PCKEV.df',
            'match_mm' => '0xc9000021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79800014',
            'name' => 'PCKOD.df',
            'match_mm' => '0xc9800021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000014',
            'name' => 'ILVL.df',
            'match_mm' => '0xca000021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800014',
            'name' => 'ILVR.df',
            'match_mm' => '0xca800021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000014',
            'name' => 'ILVEV.df',
            'match_mm' => '0xcb000021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800014',
            'name' => 'ILVOD.df',
            'match_mm' => '0xcb800021',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000015',
            'name' => 'VSHF.df',
            'match_mm' => '0xc8000023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78800015',
            'name' => 'SRAR.df',
            'match_mm' => '0xc8800023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900000a',
            'name' => 'SRARI.df',
            'match_mm' => '0xc9000011',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000015',
            'name' => 'SRLR.df',
            'match_mm' => '0xc9000023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '16',
                            'dfm'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '16',
                               'dfm'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980000a',
            'name' => 'SRLRI.df',
            'match_mm' => '0xc9800011',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000015',
            'name' => 'HADD_S.df',
            'match_mm' => '0xca000023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a800015',
            'name' => 'HADD_U.df',
            'match_mm' => '0xca800023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000015',
            'name' => 'HSUB_S.df',
            'match_mm' => '0xcb000023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b800015',
            'name' => 'HSUB_U.df',
            'match_mm' => '0xcb800023',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800001e',
            'name' => 'AND.V',
            'match_mm' => '0xc8000032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000000',
            'name' => 'ANDI.B',
            'match_mm' => '0xc8000001',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7820001e',
            'name' => 'OR.V',
            'match_mm' => '0xc8200032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000000',
            'name' => 'ORI.B',
            'match_mm' => '0xc9000001',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7840001e',
            'name' => 'NOR.V',
            'match_mm' => '0xc8400032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000000',
            'name' => 'NORI.B',
            'match_mm' => '0xca000001',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7860001e',
            'name' => 'XOR.V',
            'match_mm' => '0xc8600032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b000000',
            'name' => 'XORI.B',
            'match_mm' => '0xcb000001',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880001e',
            'name' => 'BMNZ.V',
            'match_mm' => '0xc8800032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000001',
            'name' => 'BMNZI.B',
            'match_mm' => '0xc8000002',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78a0001e',
            'name' => 'BMZ.V',
            'match_mm' => '0xc8a00032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000001',
            'name' => 'BMZI.B',
            'match_mm' => '0xc9000002',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78c0001e',
            'name' => 'BSEL.V',
            'match_mm' => '0xc8c00032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000001',
            'name' => 'BSELI.B',
            'match_mm' => '0xca000002',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78000002',
            'name' => 'SHF.B',
            'match_mm' => '0xc8000003',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000002',
            'name' => 'SHF.H',
            'match_mm' => '0xc9000003',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '23',
                            '16',
                            'i8'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '23',
                               '16',
                               'i8'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a000002',
            'name' => 'SHF.W',
            'match_mm' => '0xca000003',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '11',
                            's10'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '11',
                               's10'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900001e',
            'name' => 'BNZ.V',
            'match_mm' => '0xc9000032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '20',
                            '11',
                            's10'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '20',
                               '11',
                               's10'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7920001e',
            'name' => 'BZ.V',
            'match_mm' => '0xc9200032',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
                          [
                            '17',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '17',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00001e',
            'name' => 'FILL.df',
            'match_mm' => '0xcb000032',
            'mask_mm' => '0xfffc003f',
            'mask' => '0xfffc003f'
          },
          {
            'opcode' => [
                          [
                            '17',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '17',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b04001e',
            'name' => 'PCNT.df',
            'match_mm' => '0xcb040032',
            'mask_mm' => '0xfffc003f',
            'mask' => '0xfffc003f'
          },
          {
            'opcode' => [
                          [
                            '17',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '17',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b08001e',
            'name' => 'NLOC.df',
            'match_mm' => '0xcb080032',
            'mask_mm' => '0xfffc003f',
            'mask' => '0xfffc003f'
          },
          {
            'opcode' => [
                          [
                            '17',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '17',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b0c001e',
            'name' => 'NLZC.df',
            'match_mm' => '0xcb0c0032',
            'mask_mm' => '0xfffc003f',
            'mask' => '0xfffc003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'rd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'rd'
                             ]
                           ],
            'match' => '0x78800019',
            'name' => 'COPY_S.df',
            'match_mm' => '0xc880002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'rd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'rd'
                             ]
                           ],
            'match' => '0x78c00019',
            'name' => 'COPY_U.df',
            'match_mm' => '0xc8c0002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79000019',
            'name' => 'INSERT.df',
            'match_mm' => '0xc900002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '16',
                            'dfn'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '16',
                               'dfn'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79400019',
            'name' => 'INSVE.df',
            'match_mm' => '0xc940002a',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '11',
                            's10'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '11',
                               's10'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800000c',
            'name' => 'BNZ.df',
            'match_mm' => '0xc8000013',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '11',
                            's10'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '11',
                               's10'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880000c',
            'name' => 'BZ.df',
            'match_mm' => '0xc8800013',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '22',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '11',
                            's10'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '22',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '11',
                               's10'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900000c',
            'name' => 'LDI.df',
            'match_mm' => '0xc9000013',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800001a',
            'name' => 'FCAF.df',
            'match_mm' => '0xc800002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7840001a',
            'name' => 'FCUN.df',
            'match_mm' => '0xc840002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880001a',
            'name' => 'FCEQ.df',
            'match_mm' => '0xc880002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78c0001a',
            'name' => 'FCUEQ.df',
            'match_mm' => '0xc8c0002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900001a',
            'name' => 'FCLT.df',
            'match_mm' => '0xc900002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7940001a',
            'name' => 'FCULT.df',
            'match_mm' => '0xc940002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980001a',
            'name' => 'FCLE.df',
            'match_mm' => '0xc980002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79c0001a',
            'name' => 'FCULE.df',
            'match_mm' => '0xc9c0002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a00001a',
            'name' => 'FSAF.df',
            'match_mm' => '0xca00002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a40001a',
            'name' => 'FSUN.df',
            'match_mm' => '0xca40002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80001a',
            'name' => 'FSEQ.df',
            'match_mm' => '0xca80002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7ac0001a',
            'name' => 'FSUEQ.df',
            'match_mm' => '0xcac0002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00001a',
            'name' => 'FSLT.df',
            'match_mm' => '0xcb00002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b40001a',
            'name' => 'FSULT.df',
            'match_mm' => '0xcb40002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80001a',
            'name' => 'FSLE.df',
            'match_mm' => '0xcb80002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7bc0001a',
            'name' => 'FSULE.df',
            'match_mm' => '0xcbc0002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7800001b',
            'name' => 'FADD.df',
            'match_mm' => '0xc800002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7840001b',
            'name' => 'FSUB.df',
            'match_mm' => '0xc840002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880001b',
            'name' => 'FMUL.df',
            'match_mm' => '0xc880002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78c0001b',
            'name' => 'FDIV.df',
            'match_mm' => '0xc8c0002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900001b',
            'name' => 'FMADD.df',
            'match_mm' => '0xc900002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7940001b',
            'name' => 'FMSUB.df',
            'match_mm' => '0xc940002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x79c0001b',
            'name' => 'FEXP2.df',
            'match_mm' => '0xc9c0002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a00001b',
            'name' => 'FEXDO.df',
            'match_mm' => '0xca00002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80001b',
            'name' => 'FTQ.df',
            'match_mm' => '0xca80002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00001b',
            'name' => 'FMIN.df',
            'match_mm' => '0xcb00002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b40001b',
            'name' => 'FMIN_A.df',
            'match_mm' => '0xcb40002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80001b',
            'name' => 'FMAX.df',
            'match_mm' => '0xcb80002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7bc0001b',
            'name' => 'FMAX_A.df',
            'match_mm' => '0xcbc0002e',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7840001c',
            'name' => 'FCOR.df',
            'match_mm' => '0xc840002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7880001c',
            'name' => 'FCUNE.df',
            'match_mm' => '0xc880002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78c0001c',
            'name' => 'FCNE.df',
            'match_mm' => '0xc8c0002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7900001c',
            'name' => 'MUL_Q.df',
            'match_mm' => '0xc900002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7940001c',
            'name' => 'MADD_Q.df',
            'match_mm' => '0xc940002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7980001c',
            'name' => 'MSUB_Q.df',
            'match_mm' => '0xc980002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a40001c',
            'name' => 'FSOR.df',
            'match_mm' => '0xca40002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7a80001c',
            'name' => 'FSUNE.df',
            'match_mm' => '0xca80002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7ac0001c',
            'name' => 'FSNE.df',
            'match_mm' => '0xcac0002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b00001c',
            'name' => 'MULR_Q.df',
            'match_mm' => '0xcb00002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b40001c',
            'name' => 'MADDR_Q.df',
            'match_mm' => '0xcb40002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '21',
                            '21',
                            'df'
                          ],
                          [
                            '20',
                            '16',
                            'wt'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '21',
                               '21',
                               'df'
                             ],
                             [
                               '20',
                               '16',
                               'wt'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b80001c',
            'name' => 'MSUBR_Q.df',
            'match_mm' => '0xcb80002f',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b20001e',
            'name' => 'FCLASS.df',
            'match_mm' => '0xcb200032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b26001e',
            'name' => 'FSQRT.df',
            'match_mm' => '0xcb260032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b28001e',
            'name' => 'FRSQRT.df',
            'match_mm' => '0xcb280032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b2a001e',
            'name' => 'FRCP.df',
            'match_mm' => '0xcb2a0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b2c001e',
            'name' => 'FRINT.df',
            'match_mm' => '0xcb2c0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b2e001e',
            'name' => 'FLOG2.df',
            'match_mm' => '0xcb2e0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b30001e',
            'name' => 'FEXUPL.df',
            'match_mm' => '0xcb300032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b32001e',
            'name' => 'FEXUPR.df',
            'match_mm' => '0xcb320032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b34001e',
            'name' => 'FFQL.df',
            'match_mm' => '0xcb340032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b36001e',
            'name' => 'FFQR.df',
            'match_mm' => '0xcb360032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b38001e',
            'name' => 'FTINT_S.df',
            'match_mm' => '0xcb380032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b3a001e',
            'name' => 'FTINT_U.df',
            'match_mm' => '0xcb3a0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b3c001e',
            'name' => 'FFINT_S.df',
            'match_mm' => '0xcb3c0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b3e001e',
            'name' => 'FFINT_U.df',
            'match_mm' => '0xcb3e0032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
          },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b22001e',
            'name' => 'FTRUNC_S.df',
            'match_mm' => '0xcb220032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
           },
          {
            'opcode' => [
                          [
                            '16',
                            '16',
                            'df'
                          ],
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '16',
                               '16',
                               'df'
                             ],
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x7b24001e',
            'name' => 'FTRUNC_U.df',
            'match_mm' => '0xcb240032',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
           },
          {
            'opcode' => [
                          [
                            '15',
                            '11',
                            'rs'
                          ],
                          [
                            '10',
                            '6',
                            'cd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '15',
                               '11',
                               'rs'
                             ],
                             [
                               '10',
                               '6',
                               'cd'
                             ]
                           ],
            'match' => '0x783e0019',
            'name' => 'CTCMSA',
            'match_mm' => '0xc83e002a',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
          },
          {
            'opcode' => [
                          [
                            '15',
                            '11',
                            'cs'
                          ],
                          [
                            '10',
                            '6',
                            'rd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '15',
                               '11',
                               'cs'
                             ],
                             [
                               '10',
                               '6',
                               'rd'
                             ]
                           ],
            'match' => '0x787e0019',
            'name' => 'CFCMSA',
            'match_mm' => '0xc87e002a',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
          },
          {
            'opcode' => [
                          [
                            '15',
                            '11',
                            'ws'
                          ],
                          [
                            '10',
                            '6',
                            'wd'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '15',
                               '11',
                               'ws'
                             ],
                             [
                               '10',
                               '6',
                               'wd'
                             ]
                           ],
            'match' => '0x78be0019',
            'name' => 'MOVE.V',
            'match_mm' => '0xc8be002a',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
          }
        ];
}
