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

#if ($func_name =~ /maddv/) {
#    print "huang $func_name\n";
#}
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
static void gen_msa(CPUState *env, DisasContext *ctx)
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

            print OPCASE <<"HERECODE";
        case OPC_$name:
            gen_$namelc(env, ctx);
            return;
HERECODE
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
    elsif ($func_type eq 'df_rs_wd') {
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

        my $is_signed = $fieldname eq 's5';

        if ($is_signed) {
            $declare_str .= <<C_END;
            int64_t $fieldname = (ctx->opcode >> $shift) & $mask /* $fieldname \[$max:$min\] */;
            $fieldname = ($fieldname << 59) >> 59; /* sign extend s5 to 64 bits*/
C_END
        }
        else {
            $declare_str .= <<C_END;
    uint8_t $fieldname = (ctx->opcode >> $shift) & $mask /* $fieldname \[$max:$min\] */;
C_END
        }

        if ($fieldname eq 'df') {
            # adjust df value for Fixed-Point and Floating-Point instructions
            my $is_floating = $name =~ /\AF/;
            my $is_fixed = $name =~ /_Q/;
            if ($is_floating) {
                $declare_str .= <<C_END;

    /* adjust df value for floating-point instruction */
    df = df + 2;

    /* FIXME propagate MSAIR T flag -- should happen only when MSA is
             instantiated / first used */
    env->active_msa.fp_status.float_detect_tininess = 
        (env->active_msa.msair & MSAIR_TINY_BEFORE) ? 
        float_tininess_before_rounding : float_tininess_after_rounding;

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

    uint32_t df, n;

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
    } else {                            /* should not get here */
        assert(0);
    }

C_END
        }
        elsif ($fieldname eq 'dfm') {
            $declare_str .= <<C_END;

    uint32_t df, m;

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
    } else {                            /* should not get here */
        assert(0);
    }

C_END
        }
    }

    my $func_type = get_func_type($inst);
    my $helper_name = lc(get_helper_name($name));

    my $func_body = <<"C_END";
static void gen_$codename(CPUState *env, DisasContext *ctx) {
    /* func_type = $func_type */
C_END

    my $def;

    if ( $func_type eq 'df_wt_ws_wd' ) {

        my $stype = get_arg_type($inst,'ws');
        my $ttype = get_arg_type($inst,'wt');
        my $dtype = get_arg_type($inst,'wd');

        $func_body .= <<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_$stype(ts, tws, tdf, ti);
        gen_helper_load_wr_$ttype(tt, twt, tdf, ti);
        gen_helper_$helper_name(td, ts, tt, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(td);
    tcg_temp_free(ts);
    tcg_temp_free(tt);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(twt);
    tcg_temp_free(tdf);
C_END

       $def = "DEF_HELPER_3($helper_name, $dtype, $stype, $ttype, i32)";
    }
    elsif ( $func_type eq 'df_wt_ws_wd_wd' ) {
        # same as df_wt_ws_wd but wd used as input too
        $func_body .= <<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, tt, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(td);
    tcg_temp_free(ts);
    tcg_temp_free(tt);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(twt);
    tcg_temp_free(tdf);
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
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 t$imm = tcg_const_i64($imm);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_$stype(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, t$imm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(t$imm);
    tcg_temp_free(td);
    tcg_temp_free(ts);
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
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 t$imm = tcg_const_i64($imm);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_$stype(ts, tws, tdf, ti);
        gen_helper_load_wr_$dtype(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, t$imm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(t$imm);
    tcg_temp_free(td);
    tcg_temp_free(ts);
C_END

        $def = "DEF_HELPER_4($helper_name, $dtype, $dtype, $stype, $immtype, i32)";
    }
    elsif ($func_type eq 'df_wt_ws_wd_p') {

        $func_body .= <<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_$helper_name(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, ptr, i32)";

    }
    elsif ($func_type eq 'wt_ws_wd') {

        $func_body .= <<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_$helper_name(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, ptr, i32)";

    }
    elsif ($func_type eq 'i8_ws_wd') {
        my $imm = 'i8';

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 t$imm = tcg_const_i32($imm); // FIXME

    gen_helper_$helper_name(tpwd, tpws, t$imm, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(t$imm);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd') {

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
C_END

        $def = "DEF_HELPER_3($helper_name, s64, s64, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd_wd') {

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_$helper_name(td, td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
C_END

        $def = "DEF_HELPER_4($helper_name, s64, s64, s64, i32, i32)";
    }
    elsif ($func_type eq 'df_ws_wd') {

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_$stype(ts, tws, tdf, ti);
        gen_helper_$helper_name(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
C_END

        $def = "DEF_HELPER_2($helper_name, $dtype, $stype, i32)";
    }
    elsif ($func_type eq 'df_s10_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);
    
    gen_helper_$helper_name(tpwd, tdf, ts10, twrlen);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_s10_wd_branch') {

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_$helper_name(tdf, tpwd, tdf, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_3($helper_name, i32, ptr, i32, i32)";
    }
    elsif ($func_type eq 's10_wd_branch') {

        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_i32 tdf  = tcg_const_i32(0); /* where is df? */
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_$helper_name(tdf, tpwd, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_2($helper_name, i32, ptr, i32)";
    }
    elsif ($func_type eq 's10_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);
    
    gen_helper_$helper_name(tpwd, ts10, twrlen);

    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_3($helper_name, void, i64, i32, i32)";
    }
    elsif ($func_type eq 'dfn_ws_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);
 
    gen_helper_$helper_name(tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);
C_END

      $def = "DEF_HELPER_4($helper_name, void, ptr, ptr, i32, i32)";
    }
    elsif ($func_type eq 'ws_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);
 
    gen_helper_$helper_name(tpwd, tpws, twrlen);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(twrlen);
C_END
      $def = "DEF_HELPER_3($helper_name, void, ptr, ptr, i32)";
    }
    elsif ($func_type eq 's5_rs_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv_i32 ts5 = tcg_const_i32(s5);
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    TCGv trs = tcg_temp_new();
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, ts5, trs, twrlen);

    tcg_temp_free_i32(ts5);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twrlen);
C_END

        $def = "DEF_HELPER_4($helper_name, void, ptr, i32, i32, i32)";
    }
    elsif ($func_type eq 'rt_rs_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    gen_helper_$helper_name(tpwd, trt, trs, twrlen);

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

C_END

       $def = "DEF_HELPER_4($helper_name, void, ptr, tl, tl, i32)";
    }
    elsif ($func_type eq 'dfn_ws_rd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv telm = tcg_temp_new();
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_load_wr_s64(telm, tws, tdf, tn);
    // TODO sign<->unsign
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);

C_END
        $def = ''; # no helper required
    }
    elsif ($func_type eq 'df_rs_wd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
    TCGv telm = tcg_temp_new();
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tdf = tcg_const_i32(df);
    int i;
    int wrlen = (env->active_msa.msair & MSAIR_WIDTH_256) ? 256 : 128;
    int df_bits = 8 * (1 << df);
    TCGv_i32 ti;

    gen_load_gpr(telm, rs);
    // TODO sign<->unsign
    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_store_wr(telm, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(telm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tdf);
C_END
        $def = ''; # no heper required
    }
    elsif ($func_type eq 'rs_cd') {
        $func_body .=<<"C_END";
    check_msa(env, ctx);
$declare_str
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
    check_msa(env, ctx);
$declare_str
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
        'BINSL.df' => 'df_wt_ws_wd_wd',
        'BINSR.df' => 'df_wt_ws_wd_wd',
        'BINSLI.df' => 'dfm_ws_wd_wd',
        'BINSRI.df' => 'dfm_ws_wd_wd',
        'DPADD_S.df' => 'df_wt_ws_wd_wd',
        'DPADD_U.df' => 'df_wt_ws_wd_wd',
        'DPSUB_S.df' => 'df_wt_ws_wd_wd',
        'DPSUB_U.df' => 'df_wt_ws_wd_wd',
        'DPADDI_S.df' => 'df_u5_ws_wd_wd',
        'DPADDI_U.df' => 'df_u5_ws_wd_wd',
        'DPSUBI_S.df' => 'df_u5_ws_wd_wd',
        'DPSUBI_U.df' => 'df_u5_ws_wd_wd',
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

        'FMADD.df' => 'df_wt_ws_wd_p',
        'FMSUB.df' => 'df_wt_ws_wd_p',
        'FEXUP.df' => 'df_wt_ws_wd_p',        
        'FEXDO.df' => 'df_wt_ws_wd_p',        
        'FFQ.df'   => 'df_wt_ws_wd_p',        
        'FTQ.df'   => 'df_wt_ws_wd_p',        

        'MADDV.df' => 'df_wt_ws_wd_wd',
        'MSUBV.df' => 'df_wt_ws_wd_wd',
        'MADDVI.df' => 'df_u5_ws_wd_wd',
        'MSUBVI.df' => 'df_u5_ws_wd_wd',
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

        'AVEI_U.df/wd',
        'AVEI_U.df/ws',
        'AVEI_U.df/u5',

        'ASUB_U.df/wd',
        'ASUB_U.df/ws',
        'ASUB_U.df/wt',

        'ASUBI_U.df/wd',
        'ASUBI_U.df/ws',
        'ASUBI_U.df/u5',
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
        'ASUBI_S_df' => 'ASUB_S_df',
        'ASUBI_U_df' => 'ASUB_U_df',
        'SUBSI_S_df' => 'SUBS_S_df',
        'SUBSI_U_df' => 'SUBS_U_df',
        'SUBSSI_U_df' => 'SUBSS_U_df',
        'MAXI_A_df' => 'MAX_A_df',
        'MAXI_S_df' => 'MAX_S_df',
        'MAXI_U_df' => 'MAX_U_df',
        'MINI_A_df' => 'MIN_A_df',
        'MINI_S_df' => 'MIN_S_df',
        'MINI_U_df' => 'MIN_U_df',
        'AVEI_S_df' => 'AVE_S_df',
        'AVEI_U_df' => 'AVE_U_df',
        'MULVI_df' => 'MULV_df',
        'MADDVI_df' => 'MADDV_df',
        'MSUBVI_df' => 'MSUBV_df',
# skip these ones since they use dfm for immediate - decide whether to change
#        'SHLI_df' => 'SHL_df',
#        'SRAI_df' => 'SRA_df',
#        'SRLI_df' => 'SRL_df',
#        'BCLRI_df' => 'BCLR_df',
#        'BSETI_df' => 'BSET_df',
        'CEQI_df' => 'CEQ_df',
        'CLTI_S_df' => 'CLT_S_df',
        'CLEI_S_df' => 'CLE_S_df',
        'CLTI_U_df' => 'CLT_U_df',
        'CLEI_U_df' => 'CLE_U_df',
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
            'match' => '0x78000004',
            'name' => 'ADDVI.df',
            'match_mm' => '0xc8000007',
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
            'name' => 'ADD_A.df',
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
            'match' => '0x78800004',
            'name' => 'ADDI_A.df',
            'match_mm' => '0xc8800007',
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
            'name' => 'ADDS_A.df',
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
            'match' => '0x79000004',
            'name' => 'ADDSI_A.df',
            'match_mm' => '0xc9000007',
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
            'name' => 'ADDS_S.df',
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
            'match' => '0x79800004',
            'name' => 'ADDSI_S.df',
            'match_mm' => '0xc9800007',
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
            'name' => 'ADDS_U.df',
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
            'match' => '0x7a000004',
            'name' => 'ADDSI_U.df',
            'match_mm' => '0xca000007',
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
            'name' => 'SUBV.df',
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
            'match' => '0x7a800004',
            'name' => 'SUBVI.df',
            'match_mm' => '0xca800007',
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
            'name' => 'ASUB_S.df',
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
            'match' => '0x7b000004',
            'name' => 'ASUBI_S.df',
            'match_mm' => '0xcb000007',
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
            'name' => 'ASUB_U.df',
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
            'match' => '0x7b800004',
            'name' => 'ASUBI_U.df',
            'match_mm' => '0xcb800007',
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
            'name' => 'SUBS_S.df',
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
            'match' => '0x78000005',
            'name' => 'SUBSI_S.df',
            'match_mm' => '0xc8000009',
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
            'match' => '0x7880000f',
            'name' => 'SUBS_U.df',
            'match_mm' => '0xc8800019',
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
            'match' => '0x78800005',
            'name' => 'SUBSI_U.df',
            'match_mm' => '0xc8800009',
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
            'name' => 'SUBSS_U.df',
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
            'match' => '0x79000005',
            'name' => 'SUBSSI_U.df',
            'match_mm' => '0xc9000009',
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
            'name' => 'MAX_A.df',
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
            'match' => '0x79800005',
            'name' => 'MAXI_A.df',
            'match_mm' => '0xc9800009',
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
            'name' => 'MAX_S.df',
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
            'match' => '0x7a000005',
            'name' => 'MAXI_S.df',
            'match_mm' => '0xca000009',
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
            'name' => 'MAX_U.df',
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
            'match' => '0x7a800005',
            'name' => 'MAXI_U.df',
            'match_mm' => '0xca800009',
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
            'match' => '0x7b00000f',
            'name' => 'MIN_A.df',
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
            'match' => '0x7b000005',
            'name' => 'MINI_A.df',
            'match_mm' => '0xcb000009',
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
            'match' => '0x7b80000f',
            'name' => 'MIN_S.df',
            'match_mm' => '0xcb800019',
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
            'match' => '0x7b800005',
            'name' => 'MINI_S.df',
            'match_mm' => '0xcb800009',
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
            'name' => 'MIN_U.df',
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
            'name' => 'MINI_U.df',
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
            'match' => '0x78800010',
            'name' => 'AVE_S.df',
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
            'match' => '0x78800006',
            'name' => 'AVEI_S.df',
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
            'match' => '0x79000010',
            'name' => 'AVE_U.df',
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
            'match' => '0x79000006',
            'name' => 'AVEI_U.df',
            'match_mm' => '0xc900000a',
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
            'match' => '0x7800000b',
            'name' => 'SAT_S.df',
            'match_mm' => '0xc8000012',
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
            'match' => '0x7880000b',
            'name' => 'SAT_U.df',
            'match_mm' => '0xc8800012',
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
            'name' => 'MULV.df',
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
            'match' => '0x79800006',
            'name' => 'MULVI.df',
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
            'match' => '0x7a000010',
            'name' => 'MADDV.df',
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
            'match' => '0x7a000006',
            'name' => 'MADDVI.df',
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
            'match' => '0x7a800010',
            'name' => 'MSUBV.df',
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
            'name' => 'MSUBVI.df',
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
            'match' => '0x7b000010',
            'name' => 'DOTP_S.df',
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
            'match' => '0x7b000006',
            'name' => 'DOTPI_S.df',
            'match_mm' => '0xcb00000a',
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
            'name' => 'DOTP_U.df',
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
            'match' => '0x7b800006',
            'name' => 'DOTPI_U.df',
            'match_mm' => '0xcb80000a',
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
            'name' => 'DPADD_S.df',
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
            'match' => '0x78000007',
            'name' => 'DPADDI_S.df',
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
            'match' => '0x78800011',
            'name' => 'DPADD_U.df',
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
            'match' => '0x78800007',
            'name' => 'DPADDI_U.df',
            'match_mm' => '0xc880000b',
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
            'name' => 'DPSUB_S.df',
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
            'match' => '0x79000007',
            'name' => 'DPSUBI_S.df',
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
            'match' => '0x79800011',
            'name' => 'DPSUB_U.df',
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
            'name' => 'DPSUBI_U.df',
            'match_mm' => '0xc980000b',
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
            'match' => '0x7a000018',
            'name' => 'MUL_Q.df',
            'match_mm' => '0xca000029',
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
            'match' => '0x7a400018',
            'name' => 'MULR_Q.df',
            'match_mm' => '0xca400029',
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
            'match' => '0x7a800018',
            'name' => 'MADD_Q.df',
            'match_mm' => '0xca800029',
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
            'match' => '0x7ac00018',
            'name' => 'MADDR_Q.df',
            'match_mm' => '0xcac00029',
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
            'match' => '0x7b000018',
            'name' => 'MSUB_Q.df',
            'match_mm' => '0xcb000029',
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
            'match' => '0x7b400018',
            'name' => 'MSUBR_Q.df',
            'match_mm' => '0xcb400029',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
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
            'match' => '0x7800001b',
            'name' => 'AND.V',
            'match_mm' => '0xc800002e',
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
            'match' => '0x7820001b',
            'name' => 'OR.V',
            'match_mm' => '0xc820002e',
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
            'match' => '0x7840001b',
            'name' => 'NOR.V',
            'match_mm' => '0xc840002e',
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
            'match' => '0x7860001b',
            'name' => 'XOR.V',
            'match_mm' => '0xc860002e',
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
            'name' => 'SHL.df',
            'match_mm' => '0xca00001b',
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
            'match' => '0x7900000b',
            'name' => 'SHLI.df',
            'match_mm' => '0xc9000012',
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
            'name' => 'SRA.df',
            'match_mm' => '0xca80001b',
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
            'match' => '0x7980000b',
            'name' => 'SRAI.df',
            'match_mm' => '0xc9800012',
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
            'match' => '0x7b000011',
            'name' => 'SRL.df',
            'match_mm' => '0xcb00001b',
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
            'match' => '0x7a00000b',
            'name' => 'SRLI.df',
            'match_mm' => '0xca000012',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
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
            'match' => '0x7b04001b',
            'name' => 'PCNT.df',
            'match_mm' => '0xcb04002e',
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
            'match' => '0x7b08001b',
            'name' => 'NLOC.df',
            'match_mm' => '0xcb08002e',
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
            'match' => '0x7b0c001b',
            'name' => 'NLZC.df',
            'match_mm' => '0xcb0c002e',
            'mask_mm' => '0xfffc003f',
            'mask' => '0xfffc003f'
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
            'match' => '0x7b800011',
            'name' => 'BINSL.df',
            'match_mm' => '0xcb80001b',
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
            'match' => '0x7a80000b',
            'name' => 'BINSLI.df',
            'match_mm' => '0xca800012',
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
            'name' => 'BINSR.df',
            'match_mm' => '0xc800001e',
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
            'match' => '0x7b00000b',
            'name' => 'BINSRI.df',
            'match_mm' => '0xcb000012',
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
            'name' => 'BCLR.df',
            'match_mm' => '0xc880001e',
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
            'match' => '0x7b80000b',
            'name' => 'BCLRI.df',
            'match_mm' => '0xcb800012',
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
            'name' => 'BSET.df',
            'match_mm' => '0xc900001e',
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
            'match' => '0x7800000c',
            'name' => 'BSETI.df',
            'match_mm' => '0xc8000013',
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
            'match' => '0x7880001b',
            'name' => 'BMNZ.V',
            'match_mm' => '0xc880002e',
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
            'match' => '0x78a0001b',
            'name' => 'BMZ.V',
            'match_mm' => '0xc8a0002e',
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
            'match' => '0x78c0001b',
            'name' => 'BSEL.V',
            'match_mm' => '0xc8c0002e',
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
            'match' => '0x79800012',
            'name' => 'CEQ.df',
            'match_mm' => '0xc980001e',
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
            'name' => 'CEQI.df',
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
            'match' => '0x7a000012',
            'name' => 'CLT_S.df',
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
            'match' => '0x7a800007',
            'name' => 'CLTI_S.df',
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
            'name' => 'CLE_S.df',
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
            'match' => '0x7b000007',
            'name' => 'CLEI_S.df',
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
            'name' => 'CLT_U.df',
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
            'match' => '0x7b800007',
            'name' => 'CLTI_U.df',
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
            'name' => 'CLE_U.df',
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
            'match' => '0x78000008',
            'name' => 'CLEI_U.df',
            'match_mm' => '0xc800000e',
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
            'match' => '0x7800000d',
            'name' => 'BNZ.df',
            'match_mm' => '0xc8000016',
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
            'match' => '0x7880000d',
            'name' => 'BZ.df',
            'match_mm' => '0xc8800016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
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
            'match' => '0x78e0001b',
            'name' => 'BNZ.V',
            'match_mm' => '0xc8e0002e',
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
            'match' => '0x7900001b',
            'name' => 'BZ.V',
            'match_mm' => '0xc900002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
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
            'match' => '0x78000018',
            'name' => 'SLD.df',
            'match_mm' => '0xc8000029',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
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
            'match' => '0x7b000001',
            'name' => 'SHF.B',
            'match_mm' => '0xcb000002',
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
            'name' => 'SHF.H',
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
            'name' => 'SHF.W',
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
            'name' => 'SHF.D',
            'match_mm' => '0xca000003',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
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
            'name' => 'VSHF.df',
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
            'name' => 'PCKEV.df',
            'match_mm' => '0xc880001f',
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
            'match' => '0x7920001b',
            'name' => 'PCKEV.Q',
            'match_mm' => '0xc920002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
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
            'name' => 'PCKOD.df',
            'match_mm' => '0xc900001f',
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
            'match' => '0x7940001b',
            'name' => 'PCKOD.Q',
            'match_mm' => '0xc940002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
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
            'name' => 'ILVL.df',
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
            'name' => 'ILVR.df',
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
            'name' => 'ILVEV.df',
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
            'match' => '0x7b000013',
            'name' => 'ILVOD.df',
            'match_mm' => '0xcb00001f',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
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
            'match' => '0x787f0018',
            'name' => 'MOVE.V',
            'match_mm' => '0xc87f0029',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
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
            'match' => '0x78400018',
            'name' => 'MOVE.df',
            'match_mm' => '0xc8400029',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
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
            'match' => '0x78bf0018',
            'name' => 'CTCMSA',
            'match_mm' => '0xc8bf0029',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
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
            'match' => '0x78800018',
            'name' => 'MVTG_S.df',
            'match_mm' => '0xc8800029',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
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
            'match' => '0x78ff0018',
            'name' => 'CFCMSA',
            'match_mm' => '0xc8ff0029',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
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
            'match' => '0x7b00001b',
            'name' => 'MVFG.df',
            'match_mm' => '0xcb00002e',
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
            'match' => '0x78c00018',
            'name' => 'MVTG_U.df',
            'match_mm' => '0xc8c00029',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          },
          {
            'opcode' => [
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
            'match' => '0x7960001b',
            'name' => 'LD.V',
            'match_mm' => '0xc960002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
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
            'match' => '0x7900000d',
            'name' => 'LDI.df',
            'match_mm' => '0xc9000016',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
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
            'match' => '0x7980001b',
            'name' => 'LDX.V',
            'match_mm' => '0xc980002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
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
            'match' => '0x79a0001b',
            'name' => 'ST.V',
            'match_mm' => '0xc9a0002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
          },
          {
            'opcode' => [
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
            'match' => '0x79c0001b',
            'name' => 'STX.V',
            'match_mm' => '0xc9c0002e',
            'mask_mm' => '0xffe0003f',
            'mask' => '0xffe0003f'
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
            'match' => '0x7b800018',
            'name' => 'FADD.df',
            'match_mm' => '0xcb800029',
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
            'match' => '0x7bc00018',
            'name' => 'FSUB.df',
            'match_mm' => '0xcbc00029',
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
            'match' => '0x78000019',
            'name' => 'FMUL.df',
            'match_mm' => '0xc800002a',
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
            'match' => '0x78400019',
            'name' => 'FDIV.df',
            'match_mm' => '0xc840002a',
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
            'match' => '0x78800019',
            'name' => 'FREM.df',
            'match_mm' => '0xc880002a',
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
            'match' => '0x7b20001b',
            'name' => 'FSQRT.df',
            'match_mm' => '0xcb20002e',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
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
            'match' => '0x78c00019',
            'name' => 'FMADD.df',
            'match_mm' => '0xc8c0002a',
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
            'match' => '0x79000019',
            'name' => 'FMSUB.df',
            'match_mm' => '0xc900002a',
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
            'match' => '0x7b22001b',
            'name' => 'FRINT.df',
            'match_mm' => '0xcb22002e',
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
            'match' => '0x7b24001b',
            'name' => 'FLOG2.df',
            'match_mm' => '0xcb24002e',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
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
            'match' => '0x79400019',
            'name' => 'FEXP2.df',
            'match_mm' => '0xc940002a',
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
            'match' => '0x79800019',
            'name' => 'FMAX.df',
            'match_mm' => '0xc980002a',
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
            'match' => '0x79c00019',
            'name' => 'FMAX_A.df',
            'match_mm' => '0xc9c0002a',
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
            'match' => '0x7a000019',
            'name' => 'FMIN.df',
            'match_mm' => '0xca00002a',
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
            'match' => '0x7a400019',
            'name' => 'FMIN_A.df',
            'match_mm' => '0xca40002a',
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
            'match' => '0x7a800019',
            'name' => 'FCEQ.df',
            'match_mm' => '0xca80002a',
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
            'match' => '0x7ac00019',
            'name' => 'FCLT.df',
            'match_mm' => '0xcac0002a',
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
            'match' => '0x7b000019',
            'name' => 'FCLE.df',
            'match_mm' => '0xcb00002a',
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
            'match' => '0x7b400019',
            'name' => 'FCUN.df',
            'match_mm' => '0xcb40002a',
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
            'match' => '0x7b800019',
            'name' => 'FCEQU.df',
            'match_mm' => '0xcb80002a',
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
            'match' => '0x7bc00019',
            'name' => 'FCLTU.df',
            'match_mm' => '0xcbc0002a',
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
            'match' => '0x7800001a',
            'name' => 'FCLEU.df',
            'match_mm' => '0xc800002b',
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
            'match' => '0x7b26001b',
            'name' => 'FCLASS.df',
            'match_mm' => '0xcb26002e',
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
            'match' => '0x7b28001b',
            'name' => 'FTINT_S.df',
            'match_mm' => '0xcb28002e',
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
            'match' => '0x7b2a001b',
            'name' => 'FFINT_S.df',
            'match_mm' => '0xcb2a002e',
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
            'match' => '0x7b2c001b',
            'name' => 'FTINT_U.df',
            'match_mm' => '0xcb2c002e',
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
            'match' => '0x7b2e001b',
            'name' => 'FFINT_U.df',
            'match_mm' => '0xcb2e002e',
            'mask_mm' => '0xfffe003f',
            'mask' => '0xfffe003f'
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
            'name' => 'FTQ.df',
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
            'name' => 'FFQ.df',
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
            'name' => 'FEXDO.df',
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
            'name' => 'FEXUP.df',
            'match_mm' => '0xc900002b',
            'mask_mm' => '0xffc0003f',
            'mask' => '0xffc0003f'
          }

         ];

}
