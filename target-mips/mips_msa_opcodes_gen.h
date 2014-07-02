static void gen_sll_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_sll_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_slli_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_slli_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_sra_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_sra_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_srai_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_srai_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_srl_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_srl_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_srli_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_srli_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_bclr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bclr_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_bclri_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_bclri_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_bset_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bset_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_bseti_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_bseti_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_bneg_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bneg_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_bnegi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_bnegi_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_binsl_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_binsl_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_binsli_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_binsli_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_binsr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_binsr_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_binsri_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_binsri_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_addv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_addv_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_addvi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_addvi_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_subv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_subv_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_subvi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_subvi_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_max_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_max_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_maxi_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
    s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);

    gen_helper_msa_maxi_s_df(cpu_env, tdf, twd, tws, ts5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);

}

static void gen_max_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_max_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_maxi_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_maxi_u_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_min_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_min_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mini_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
    s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);

    gen_helper_msa_mini_s_df(cpu_env, tdf, twd, tws, ts5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);

}

static void gen_min_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_min_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mini_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_mini_u_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_max_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_max_a_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_min_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_min_a_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ceq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_ceq_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ceqi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
    s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);

    gen_helper_msa_ceqi_df(cpu_env, tdf, twd, tws, ts5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);

}

static void gen_clt_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_clt_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_clti_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
    s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);

    gen_helper_msa_clti_s_df(cpu_env, tdf, twd, tws, ts5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);

}

static void gen_clt_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_clt_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_clti_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_clti_u_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_cle_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_cle_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_clei_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
    s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);

    gen_helper_msa_clei_s_df(cpu_env, tdf, twd, tws, ts5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);

}

static void gen_cle_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_cle_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_clei_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);

    gen_helper_msa_clei_u_df(cpu_env, tdf, twd, tws, tu5);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);

}

static void gen_ld_df(CPUMIPSState *env, DisasContext *ctx)
{
    int64_t s10 = (ctx->opcode >> 16) & 0x3ff /* s10 [25:16] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;
    uint8_t df = (ctx->opcode >> 0) & 0x3 /* df [1:0] */;

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

}

static void gen_st_df(CPUMIPSState *env, DisasContext *ctx)
{
    int64_t s10 = (ctx->opcode >> 16) & 0x3ff /* s10 [25:16] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;
    uint8_t df = (ctx->opcode >> 0) & 0x3 /* df [1:0] */;

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

}

static void gen_sat_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_sat_s_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_sat_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_sat_u_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_add_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_add_a_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_adds_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_adds_a_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_adds_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_adds_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_adds_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_adds_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ave_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_ave_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ave_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_ave_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_aver_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_aver_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_aver_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_aver_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_subs_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_subs_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_subs_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_subs_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_subsus_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_subsus_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_subsuu_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_subsuu_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_asub_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_asub_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_asub_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_asub_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mulv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_mulv_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_maddv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_maddv_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_msubv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_msubv_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_div_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_div_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_div_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_div_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mod_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_mod_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mod_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_mod_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_dotp_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dotp_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_dotp_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dotp_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_dpadd_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dpadd_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_dpadd_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dpadd_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_dpsub_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dpsub_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_dpsub_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_dpsub_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_sld_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trt = tcg_const_i32(rt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_sld_df(cpu_env, twd, tws, trt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(trt);
    tcg_temp_free_i32(tdf);

}

static void gen_sldi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv tn  = tcg_const_tl(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_sldi_df(cpu_env, twd, tws, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(tdf);

}

static void gen_splat_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trt = tcg_const_i32(rt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_splat_df(cpu_env, twd, tws, trt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(trt);
    tcg_temp_free_i32(tdf);

}

static void gen_splati_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv tn  = tcg_const_tl(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_splati_df(cpu_env, twd, tws, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(tdf);

}

static void gen_pckev_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_pckev_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_pckod_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_pckod_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ilvl_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ilvl_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ilvr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ilvr_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ilvev_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ilvev_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ilvod_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ilvod_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_vshf_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_vshf_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_srar_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_srar_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_srari_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_srari_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_srlr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_srlr_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_srlri_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;
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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);

    gen_helper_msa_srlri_df(cpu_env, tdf, twd, tws, tm);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

static void gen_hadd_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_hadd_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_hadd_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_hadd_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_hsub_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_hsub_s_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_hsub_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    /* check df: byte format not allowed */
    if (df == 0) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_hsub_u_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_and_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_and_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_andi_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_andi_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_or_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_or_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_ori_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_ori_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_nor_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_nor_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_nori_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_nori_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_xor_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_xor_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_xori_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_xori_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_bmnz_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bmnz_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_bmnzi_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_bmnzi_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_bmz_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bmz_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_bmzi_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_bmzi_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_bsel_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_bsel_v(cpu_env, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);

}

static void gen_bseli_b(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_bseli_b(cpu_env, twd, tws, ti8);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(ti8);

}

static void gen_shf_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 24) & 0x3 /* df [25:24] */;
    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;


    /* check df: double format not allowed */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 ti8 = tcg_const_i32(i8); /* FIXME */

    gen_helper_msa_shf_df(cpu_env, twd, tws, ti8, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ti8);

}

static void gen_bnz_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/


    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i64 t0 = tcg_temp_local_new_i64();
    tcg_gen_or_i64(t0, msa_wr_d[wt<<1], msa_wr_d[(wt<<1)+1]);
    tcg_gen_setcondi_i64(TCG_COND_NE, t0, t0, 0);
    tcg_gen_trunc_i64_tl(bcond, t0);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free_i64(t0);
}

static void gen_bz_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/


    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i64 t0 = tcg_temp_local_new_i64();
    tcg_gen_or_i64(t0, msa_wr_d[wt<<1], msa_wr_d[(wt<<1)+1]);
    tcg_gen_setcondi_i64(TCG_COND_EQ, t0, t0, 0);
    tcg_gen_trunc_i64_tl(bcond, t0);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free_i64(t0);
}

static void gen_fill_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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

    gen_helper_msa_fill_df(cpu_env, tdf, twd, trs);

	tcg_temp_free_i32(twd);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i32(tdf);

}

static void gen_pcnt_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

	gen_helper_msa_pcnt_df(cpu_env, tdf, twd, tws);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);

}

static void gen_nloc_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

	gen_helper_msa_nloc_df(cpu_env, tdf, twd, tws);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);

}

static void gen_nlzc_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

	gen_helper_msa_nlzc_df(cpu_env, tdf, twd, tws);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);

}

static void gen_copy_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }
#endif

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, -1)) return;

    TCGv_i32 trd = tcg_const_i32(rd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_msa_copy_s_df(cpu_env, tdf, trd, tws, tn);

    tcg_temp_free_i32(trd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
}

static void gen_copy_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }
#endif

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, -1)) return;

    TCGv_i32 trd = tcg_const_i32(rd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_msa_copy_u_df(cpu_env, tdf, trd, tws, tn);

    tcg_temp_free_i32(trd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
}

static void gen_insert_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

#if !defined(TARGET_MIPS64)
    /* Double format valid only for MIPS64 */
    if (df == 3) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }
#endif

    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 trs = tcg_const_i32(rs);
    TCGv_i32 tn  = tcg_const_i32(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_insert_df(cpu_env, twd, trs, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(trs);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(tdf);

}

static void gen_insve_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv tn  = tcg_const_tl(n);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_insve_df(cpu_env, twd, tws, tn, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(tdf);

}

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

static void gen_bnz_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    determ_zero_element(bcond, df, wt);
    /* if zero all element is non-zero */
    /* if one, some element is zero */
    tcg_gen_setcondi_tl(TCG_COND_EQ, bcond, bcond, 0);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

}

static void gen_bz_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    determ_zero_element(bcond, df, wt);
    /* if zero all element is non-zero */
    /* if one, some element is zero */

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    ctx->hflags |= MIPS_HFLAG_BC;

}

static void gen_ldi_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_i32 twd = tcg_const_i32(wd);

    gen_helper_msa_ldi_df(cpu_env, tdf, twd, ts10);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i32(twd);

}

static void gen_fcaf_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcaf_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcun_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcun_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fceq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fceq_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcueq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcueq_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fclt_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fclt_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcult_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcult_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcle_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcle_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcule_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcule_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsaf_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsaf_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsun_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsun_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fseq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fseq_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsueq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsueq_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fslt_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fslt_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsult_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsult_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsle_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsle_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsule_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsule_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fadd_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fadd_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsub_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsub_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmul_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmul_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fdiv_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fdiv_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmadd_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmadd_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmsub_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmsub_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fexp2_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fexp2_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fexdo_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fexdo_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_ftq_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ftq_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmin_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmin_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmin_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmin_a_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmax_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmax_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fmax_a_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fmax_a_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcor_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcor_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcune_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcune_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fcne_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fcne_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mul_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_mul_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_madd_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_madd_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_msub_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_msub_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_fsor_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsor_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsune_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsune_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_fsne_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsne_df(cpu_env, twd, tws, twt, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_mulr_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_mulr_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

}

static void gen_maddr_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_maddr_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_msubr_q_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twt = tcg_const_i32(wt);

    gen_helper_msa_msubr_q_df(cpu_env, tdf, twd, tws, twt);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);
}

static void gen_fclass_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

	gen_helper_msa_fclass_df(cpu_env, tdf, twd, tws);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);

}

static void gen_ftrunc_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ftrunc_s_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ftrunc_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ftrunc_u_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_fsqrt_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fsqrt_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_frsqrt_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_frsqrt_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_frcp_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_frcp_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_frint_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_frint_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_flog2_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_flog2_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_fexupl_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fexupl_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_fexupr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_fexupr_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ffql_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ffql_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ffqr_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ffqr_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ftint_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ftint_s_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ftint_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ftint_u_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ffint_s_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ffint_s_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ffint_u_df(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);

    gen_helper_msa_ffint_u_df(cpu_env, twd, tws, tdf);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);

}

static void gen_ctcmsa(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t cd = (ctx->opcode >> 6) & 0x1f /* cd [10:6] */;

    if (!check_msa_access(env, ctx, -1, -1, -1)) return;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcd = tcg_const_i32(cd);

    gen_load_gpr(telm, rs);
    gen_helper_ctcmsa(cpu_env, telm, tcd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcd);

}

static void gen_cfcmsa(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t cs = (ctx->opcode >> 11) & 0x1f /* cs [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

    if (!check_msa_access(env, ctx, -1, -1, -1)) return;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcs = tcg_const_i32(cs);

    gen_helper_cfcmsa(telm, cpu_env, tcs);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcs);

}

static void gen_move_v(CPUMIPSState *env, DisasContext *ctx)
{
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);

    gen_helper_msa_move_v(cpu_env, twd, tws);

    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);

}

