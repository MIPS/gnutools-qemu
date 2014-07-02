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
static void gen_msa(CPUMIPSState *env, DisasContext *ctx)
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

            if ($name eq 'CTCMSA') {
                print OPCASE <<"HERECODE";
        case OPC_$name:
            gen_$namelc(env, ctx);
            /* Stop translation as we may have switched the execution mode */
            ctx->bstate = BS_STOP;
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

    if (check_msa_access(env, ctx, -1, -1, -1))
        generate_exception(ctx, EXCP_RI);

} /* end of gen_msa() */
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

        my $is_branch = $code_name =~ /BZ/ ||  $code_name =~ /BNZ/;

        if ($is_branch) {
            print OPCODES "    OPC_$code_name$spacestr= $match | OPC_CP1,\n";
        }
        else {
            print OPCODES "    OPC_$code_name$spacestr= $match | OPC_MSA,\n";
        }
    }

    print OPCODES "};\n";

    close OPCODES;

    return;
}


sub get_func_body {
    my ($inst) = @_;

    my $name = $inst->{name};

    my $opcode = $inst->{opcode};

    my $codename = lc(get_code_name($name));

    my $declare_str;

    my $func_body;

    if ($name eq 'BNZ.df') {
        $func_body .= << "C_END";
static void determ_zero_element(TCGv tresult, uint8_t df, uint8_t wt)
{
    /* Note this function only works with MSA_WRLEN = 128 */
    uint64_t eval_zero_or_big;
    uint64_t eval_big;
    switch(df) {
    case 0: /*DF_BYTE*/
        eval_zero_or_big = 0x0101010101010101ULL;
        eval_big = 0x8080808080808080ULL;
        break;
    case 1: /*DF_HALF*/
        eval_zero_or_big = 0x0001000100010001ULL;
        eval_big = 0x8000800080008000ULL;
        break;
    case 2: /*DF_WORD*/
        eval_zero_or_big = 0x0000000100000001ULL;
        eval_big = 0x8000000080000000ULL;
        break;
    case 3: /*DF_DOUBLE*/
        eval_zero_or_big = 0x0000000000000001ULL;
        eval_big = 0x8000000000000000ULL;
        break;
    }
    TCGv_i64 t0 = tcg_temp_local_new_i64();
    TCGv_i64 t1 = tcg_temp_local_new_i64();
    tcg_gen_subi_i64(t0, msa_wr_d[wt<<1], eval_zero_or_big);
    tcg_gen_andc_i64(t0, t0, msa_wr_d[wt<<1]);
    tcg_gen_andi_i64(t0, t0, eval_big);
    tcg_gen_subi_i64(t1, msa_wr_d[(wt<<1)+1], eval_zero_or_big);
    tcg_gen_andc_i64(t1, t1, msa_wr_d[(wt<<1)+1]);
    tcg_gen_andi_i64(t1, t1, eval_big);
    tcg_gen_or_i64(t0, t0, t1);
    /* if all bits is zero then all element is not zero */
    /* if some bit is non-zero then some element is zero */
    tcg_gen_setcondi_i64(TCG_COND_NE, t0, t0, 0);
    tcg_gen_trunc_i64_tl(tresult, t0);
    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);
}

C_END
        }

    foreach my $field (@{$opcode}) {
        my ($max,$min,$fieldname) = @{$field};

        my $shift = $min;
        my $mask = sprintf("0x%x", (1 << (1 + $max-$min)) - 1 );

        my $is_signed = $fieldname =~ /s5|s10|s16/;

        if ($is_signed) {
            $declare_str .= <<C_END;
    int64_t $fieldname = (ctx->opcode >> $shift) & $mask /* $fieldname \[$max:$min\] */;
C_END

            if ($fieldname eq 's5') {
                $declare_str .= <<C_END;
    $fieldname = ($fieldname << 59) >> 59; /* sign extend s5 to 64 bits*/
C_END
            }
            if ($fieldname eq 's10') {
                $declare_str .= <<C_END;
    $fieldname = ($fieldname << 54) >> 54; /* sign extend s10 to 64 bits*/
C_END
            }
            if ($fieldname eq 's16') {
                $declare_str .= <<C_END;
    $fieldname = ($fieldname << 48) >> 48; /* sign extend s16 to 64 bits*/
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
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    int df_bits = 8 * (1 << df);
    if ( n >= MSA_WRLEN / df_bits ) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

C_END

        my $is_copy = ($codename =~ /copy_[su]_df/);
        my $is_insert = ($codename =~ /insert_df/);

        if ( $is_copy || $is_insert ) {
            $declare_str .= <<C_END;
#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
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
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

C_END
        }
    }

    my $func_type = get_func_type($inst);
    my $helper_name = lc(get_helper_name($name));

    $func_body .= <<"C_END";
static void gen_$codename(CPUMIPSState *env, DisasContext *ctx)
{
C_END
#    /* func_type = $func_type */

    my $def;

    my $is_ld_v  = $codename =~ /ld_df/ && !($codename =~ /sld_df/);
    my $is_st_v  = $codename =~ /st_df/;

    if ($is_ld_v) {
        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trs = tcg_const_i32(rs);
    TCGv_i64 ts10 = tcg_const_i64(s10);

    gen_helper_msa_ld_df(cpu_env, tdf, twd, trs, ts10);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i64(ts10);

C_END
        $def = 'DEF_HELPER_5(msa_ld_df, void, env, i32, i32, i32, s64)';
    } elsif ($is_st_v) {
        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wd, -1, -1)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trs = tcg_const_i32(rs);
    TCGv_i64 ts10 = tcg_const_i64(s10);

    gen_helper_msa_st_df(cpu_env, tdf, twd, trs, ts10);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i64(ts10);

C_END
        $def = 'DEF_HELPER_5(msa_st_df, void, env, i32, i32, i32, s64)';
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
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

C_END
        }

        $func_body .= <<"C_END";
    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

C_END

       $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
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
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

C_END
        }

        $func_body .= <<"C_END";
    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif (   $func_type eq 'df_u5_ws_wd'
           || $func_type eq 'df_s5_ws_wd' ) {

        my ($imm) = $func_type =~ /([su]5)/xms or die "bad match";

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');
        my $immtype = get_arg_type($inst,$imm);

        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 t$imm = tcg_const_i64($imm);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws, t$imm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(t$imm);

C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, $immtype)";
    }
    elsif ($func_type eq 'df_wt_ws_wd_p') {

        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";

    }
    elsif ($func_type eq 'df_ws_wd_p') {

        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";

    }
    elsif ($func_type eq 'wt_ws_wd') {

        $func_body .= <<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_$helper_name(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";

    }
    elsif ($func_type eq 'i8_ws_wd') {
        my $imm = 'i8';

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 t$imm = tcg_const_i32($imm); /* FIXME */

    gen_helper_msa_$helper_name(cpu_env, twd, tws, t$imm);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(t$imm);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_i8_ws_wd') {
        my $imm = 'i8';
        my $is_shf = ($codename =~ /shf_df/);

        $func_body .=<<"C_END";
$declare_str
C_END

        if ( $is_shf ) {
          $func_body .= <<"C_END";

    /* check df: double format not allowed */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }
C_END
        }

        $func_body .= <<"C_END";

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 t$imm = tcg_const_i32($imm); /* FIXME */

    gen_helper_msa_$helper_name(cpu_env, twd, tws, t$imm, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(t$imm);

C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd') {

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'dfm_ws_wd_wd') {

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

C_END

        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_ws_wd') {

        my $stype = get_arg_type($inst,'ws');
        my $dtype = get_arg_type($inst,'wd');

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

	gen_helper_msa_$helper_name(cpu_env, tdf, twd, tws);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_s10_wd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_i32 twd = tcg_const_i32(wd);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, ts10);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i32(twd);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_s16_wt_branch') {

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    determ_zero_element(bcond, df, wt);
    /* if zero all element is non-zero */
    /* if one, some element is zero */
C_END

        if ($name eq "BNZ.df") {
            $func_body .=<< "C_END";
    tcg_gen_setcondi_tl(TCG_COND_EQ, bcond, bcond, 0);
C_END

        }
        $func_body .=<< "C_END";

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

C_END

        $def = "";
    }
    elsif ($func_type eq 's16_wt_branch') {

        $func_body .=<<"C_END";
$declare_str

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i64 t0 = tcg_temp_local_new_i64();
    tcg_gen_or_i64(t0, msa_wr_d[wt<<1], msa_wr_d[(wt<<1)+1]);
C_END
        if ($name eq 'BNZ.V') {
            $func_body .=<<"C_END";
    tcg_gen_setcondi_i64(TCG_COND_NE, t0, t0, 0);
C_END

        }
        elsif ($name eq 'BZ.V') {
            $func_body .=<<"C_END";
    tcg_gen_setcondi_i64(TCG_COND_EQ, t0, t0, 0);
C_END

        }

        $func_body .=<<"C_END";
    tcg_gen_trunc_i64_tl(bcond, t0);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free_i64(t0);
C_END

        $def= "";
    }
    elsif ($func_type eq 'dfn_ws_wd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv tn  = tcg_const_tl(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, twd, tws, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(tdf);

C_END

      $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, tl, i32)";
    }
    elsif ($func_type eq 'dfn_rs_wd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trs = tcg_const_i32(rs);
    TCGv_i32 tn  = tcg_const_i32(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, twd, trs, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(tdf);

C_END

      $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_rt_ws_wd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trt = tcg_const_i32(rt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, twd, tws, trt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(trt);
    tcg_temp_free_i32(tdf);

C_END

      $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'ws_wd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

    gen_helper_msa_$helper_name(cpu_env, twd, tws);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

C_END
      $def = "DEF_HELPER_3(msa_$helper_name, void, env, i32, i32)";
    }
    elsif ($func_type eq 'dfn_ws_rd') {

        my $stype = get_arg_type($inst,'ws');

        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, ws, ws, -1)) return;

    TCGv_i32 trd = tcg_const_i32(rd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_msa_$helper_name(cpu_env, tdf, trd, tws, tn);

    tcg_temp_free_i32(trd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
C_END
        $def = "DEF_HELPER_5(msa_$helper_name, void, env, i32, i32, i32, i32)";
    }
    elsif ($func_type eq 'df_rs_wd') {
        $func_body .=<<"C_END";
$declare_str
#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }
#endif

    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trs = tcg_const_i32(rs);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_$helper_name(cpu_env, tdf, twd, trs);

	tcg_temp_free_i32(twd);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i32(tdf);

C_END

        $def = "DEF_HELPER_4(msa_$helper_name, void, env, i32, i32, i32)";


    }
    elsif ($func_type eq 'rs_cd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, -1, -1, -1)) return;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcd = tcg_const_i32(cd);

    gen_load_gpr(telm, rs);
    gen_helper_$helper_name(cpu_env, telm, tcd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcd);

C_END

        $def = "DEF_HELPER_3($helper_name, void, env, tl, i32)";
    }
    elsif ($func_type eq 'cs_rd') {
        $func_body .=<<"C_END";
$declare_str
    if (!check_msa_access(env, ctx, -1, -1, -1)) return;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcs = tcg_const_i32(cs);

    gen_helper_$helper_name(telm, cpu_env, tcs);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcs);

C_END

    $def = "DEF_HELPER_2($helper_name, tl, env, i32)";
    }

    else {

        $func_body .= <<C_END;
    /* TODO $name ($func_type) */
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
        'LD.df' => 'mi10_rs_wd',
        'ST.df' => 'mi10_rs_wd',
        'BINSL.df' => 'df_wt_ws_wd_wd',
        'BINSR.df' => 'df_wt_ws_wd_wd',
        'BINSLI.df' => 'dfm_ws_wd_wd',
        'BINSRI.df' => 'dfm_ws_wd_wd',
        'DPADD_S.df' => 'df_wt_ws_wd_wd',
        'DPADD_U.df' => 'df_wt_ws_wd_wd',
        'DPSUB_S.df' => 'df_wt_ws_wd_wd',
        'DPSUB_U.df' => 'df_wt_ws_wd_wd',
        'BNZ.df' => 'df_s16_wt_branch',
        'BZ.df' => 'df_s16_wt_branch',
        'BNZ.V' => 's16_wt_branch',
        'BZ.V' => 's16_wt_branch',
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
            'match_mm' => '0x5800001a',
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
            'match_mm' => '0x58000012',
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
            'match_mm' => '0x5880001a',
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
            'match_mm' => '0x58800012',
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
            'match_mm' => '0x5900001a',
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
            'match_mm' => '0x59000012',
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
            'match_mm' => '0x5980001a',
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
            'match_mm' => '0x59800012',
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
            'match_mm' => '0x5a00001a',
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
            'match_mm' => '0x5a000012',
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
            'match_mm' => '0x5a80001a',
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
            'match_mm' => '0x5a800012',
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
            'match_mm' => '0x5b00001a',
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
            'match_mm' => '0x5b000012',
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
            'match_mm' => '0x5b80001a',
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
            'match_mm' => '0x5b800012',
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
            'match_mm' => '0x5800002a',
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
            'match_mm' => '0x58000029',
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
            'match_mm' => '0x5880002a',
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
            'match_mm' => '0x58800029',
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
            'match_mm' => '0x5900002a',
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
            'match_mm' => '0x59000029',
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
            'match_mm' => '0x5980002a',
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
            'match_mm' => '0x59800029',
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
            'match_mm' => '0x5a00002a',
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
            'match_mm' => '0x5a000029',
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
            'match_mm' => '0x5a80002a',
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
            'match_mm' => '0x5a800029',
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
            'match_mm' => '0x5b00002a',
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
            'match_mm' => '0x5b80002a',
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
            'match_mm' => '0x5800003a',
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
            'match_mm' => '0x58000039',
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
            'match_mm' => '0x5900003a',
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
            'match_mm' => '0x59000039',
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
            'match_mm' => '0x5980003a',
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
            'match_mm' => '0x59800039',
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
            'match_mm' => '0x5a00003a',
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
            'match_mm' => '0x5a000039',
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
            'match_mm' => '0x5a80003a',
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
            'match_mm' => '0x5a800039',
            'mask_mm' => '0xff80003f',
            'mask' => '0xff80003f'
          },
          {
            'opcode' => [
                          [
                            '25',
                            '16',
                            's10'
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
                          ],
                          [
                            '1',
                            '0',
                            'df'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '25',
                               '16',
                               's10'
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
                             ],
                             [
                               '5',
                               '4',
                               'df'
                             ]
                           ],
            'match' => '0x78000020',
            'name' => 'LD.df',
            'match_mm' => '0x58000007',
            'mask_mm' => '0xfc00000f',
            'mask' => '0xfc00003c'
          },
          {
            'opcode' => [
                          [
                            '25',
                            '16',
                            's10'
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
                          ],
                          [
                            '1',
                            '0',
                            'df'
                          ]
                        ],
            'opcode_mm' => [
                             [
                               '25',
                               '16',
                               's10'
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
                             ],
                             [
                               '5',
                               '4',
                               'df'
                             ]
                           ],
            'match' => '0x78000024',
            'name' => 'ST.df',
            'match_mm' => '0x5800000f',
            'mask_mm' => '0xfc00000f',
            'mask' => '0xfc00003c'
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
            'match_mm' => '0x58000022',
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
            'match_mm' => '0x58800022',
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
            'match_mm' => '0x58000003',
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
            'match_mm' => '0x58800003',
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
            'match_mm' => '0x59000003',
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
            'match_mm' => '0x59800003',
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
            'match_mm' => '0x5a000003',
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
            'match_mm' => '0x5a800003',
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
            'match_mm' => '0x5b000003',
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
            'match_mm' => '0x5b800003',
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
            'match_mm' => '0x58000013',
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
            'match_mm' => '0x58800013',
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
            'match_mm' => '0x59000013',
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
            'match_mm' => '0x59800013',
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
            'match_mm' => '0x5a000013',
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
            'match_mm' => '0x5a800013',
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
            'match_mm' => '0x58000023',
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
            'match_mm' => '0x58800023',
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
            'match_mm' => '0x59000023',
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
            'match_mm' => '0x5a000023',
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
            'match_mm' => '0x5a800023',
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
            'match_mm' => '0x5b000023',
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
            'match_mm' => '0x5b800023',
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
            'match_mm' => '0x58000033',
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
            'match_mm' => '0x58800033',
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
            'match_mm' => '0x59000033',
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
            'match_mm' => '0x59800033',
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
            'match_mm' => '0x5a000033',
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
            'match_mm' => '0x5a800033',
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
            'match_mm' => '0x5800000b',
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
            'match_mm' => '0x58000016',
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
            'match_mm' => '0x5880000b',
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
            'match_mm' => '0x58400016',
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
            'match_mm' => '0x5900000b',
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
            'match_mm' => '0x5980000b',
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
            'match_mm' => '0x5a00000b',
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
            'match_mm' => '0x5a80000b',
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
            'match_mm' => '0x5b00000b',
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
            'match_mm' => '0x5b80000b',
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
            'match_mm' => '0x5800001b',
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
            'match_mm' => '0x5880001b',
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
            'match_mm' => '0x59000022',
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
            'match_mm' => '0x5900001b',
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
            'match_mm' => '0x59800022',
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
            'match_mm' => '0x5a00001b',
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
            'match_mm' => '0x5a80001b',
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
            'match_mm' => '0x5b00001b',
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
            'match_mm' => '0x5b80001b',
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
            'match_mm' => '0x5800002e',
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
            'match_mm' => '0x58000001',
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
            'match_mm' => '0x5820002e',
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
            'match_mm' => '0x59000001',
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
            'match_mm' => '0x5840002e',
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
            'match_mm' => '0x5a000001',
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
            'match_mm' => '0x5860002e',
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
            'match_mm' => '0x5b000001',
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
            'match_mm' => '0x5880002e',
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
            'match_mm' => '0x58000011',
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
            'match_mm' => '0x58a0002e',
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
            'match_mm' => '0x59000011',
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
            'match_mm' => '0x58c0002e',
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
            'match_mm' => '0x5a000011',
            'mask_mm' => '0xff00003f',
            'mask' => '0xff00003f'
          },
          {
            'opcode' => [
                          [
                            '25',
                            '24',
                            'df'
                          ],
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
                               '25',
                               '24',
                               'df'
                             ],
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
            'name' => 'SHF.df',
            'match_mm' => '0x58000021',
            'mask_mm' => '0xfc00003f',
            'mask' => '0xfc00003f'
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
                            '0',
                            's16'
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
                               '0',
                               's16'
                             ]
                           ],
            'match' => '0x45e00000',
            'name' => 'BNZ.V',
            'match_mm' => '0x81e00000',
            'mask_mm' => '0xffe00000',
            'mask' => '0xffe00000'
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
                            '0',
                            's16'
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
                               '0',
                               's16'
                             ]
                           ],
            'match' => '0x45600000',
            'name' => 'BZ.V',
            'match_mm' => '0x81600000',
            'mask_mm' => '0xffe00000',
            'mask' => '0xffe00000'
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
            'match_mm' => '0x5b00002e',
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
            'match_mm' => '0x5b04002e',
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
            'match_mm' => '0x5b08002e',
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
            'match_mm' => '0x5b0c002e',
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
            'match_mm' => '0x58800016',
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
            'match_mm' => '0x58c00016',
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
            'match_mm' => '0x59000016',
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
            'match_mm' => '0x59400016',
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
                            '0',
                            's16'
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
                               '0',
                               's16'
                             ]
                           ],
            'match' => '0x47800000',
            'name' => 'BNZ.df',
            'match_mm' => '0x83800000',
            'mask_mm' => '0xff800000',
            'mask' => '0xff800000'
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
                            '0',
                            's16'
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
                               '0',
                               's16'
                             ]
                           ],
            'match' => '0x47000000',
            'name' => 'BZ.df',
            'match_mm' => '0x83000000',
            'mask_mm' => '0xff800000',
            'mask' => '0xff800000'
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
            'match' => '0x7b000007',
            'name' => 'LDI.df',
            'match_mm' => '0x5b000039',
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
            'match_mm' => '0x58000026',
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
            'match_mm' => '0x58400026',
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
            'match_mm' => '0x58800026',
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
            'match_mm' => '0x58c00026',
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
            'match_mm' => '0x59000026',
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
            'match_mm' => '0x59400026',
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
            'match_mm' => '0x59800026',
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
            'match_mm' => '0x59c00026',
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
            'match_mm' => '0x5a000026',
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
            'match_mm' => '0x5a400026',
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
            'match_mm' => '0x5a800026',
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
            'match_mm' => '0x5ac00026',
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
            'match_mm' => '0x5b000026',
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
            'match_mm' => '0x5b400026',
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
            'match_mm' => '0x5b800026',
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
            'match_mm' => '0x5bc00026',
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
            'match_mm' => '0x58000036',
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
            'match_mm' => '0x58400036',
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
            'match_mm' => '0x58800036',
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
            'match_mm' => '0x58c00036',
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
            'match_mm' => '0x59000036',
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
            'match_mm' => '0x59400036',
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
            'match_mm' => '0x59c00036',
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
            'match_mm' => '0x5a000036',
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
            'match_mm' => '0x5a800036',
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
            'match_mm' => '0x5b000036',
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
            'match_mm' => '0x5b400036',
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
            'match_mm' => '0x5b800036',
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
            'match_mm' => '0x5bc00036',
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
            'match_mm' => '0x5840000e',
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
            'match_mm' => '0x5880000e',
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
            'match_mm' => '0x58c0000e',
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
            'match_mm' => '0x5900000e',
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
            'match_mm' => '0x5940000e',
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
            'match_mm' => '0x5980000e',
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
            'match_mm' => '0x5a40000e',
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
            'match_mm' => '0x5a80000e',
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
            'match_mm' => '0x5ac0000e',
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
            'match_mm' => '0x5b00000e',
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
            'match_mm' => '0x5b40000e',
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
            'match_mm' => '0x5b80000e',
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
            'match_mm' => '0x5b20002e',
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
            'match_mm' => '0x5b22002e',
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
            'match_mm' => '0x5b24002e',
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
            'match_mm' => '0x5b26002e',
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
            'match_mm' => '0x5b28002e',
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
            'match_mm' => '0x5b2a002e',
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
            'match_mm' => '0x5b2c002e',
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
            'match_mm' => '0x5b2e002e',
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
            'match_mm' => '0x5b30002e',
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
            'match_mm' => '0x5b32002e',
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
            'match_mm' => '0x5b34002e',
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
            'match_mm' => '0x5b36002e',
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
            'match_mm' => '0x5b38002e',
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
            'match_mm' => '0x5b3a002e',
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
            'match_mm' => '0x5b3c002e',
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
            'match_mm' => '0x5b3e002e',
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
            'match_mm' => '0x583e0016',
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
            'match_mm' => '0x587e0016',
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
            'match_mm' => '0x58be0016',
            'mask_mm' => '0xffff003f',
            'mask' => '0xffff003f'
          },
        ];
}
