static void gen_sll_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_sll_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_slli_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_slli_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_sra_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_sra_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_srai_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_srai_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_srl_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_srl_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_srli_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_srli_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_bclr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_bclr_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_bclri_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_bclri_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_bset_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_bset_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_bseti_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_bseti_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_bneg_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_bneg_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_bnegi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_bnegi_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_binsl_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_binsl_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_binsli_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_binsli_df(td, cpu_env, td, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_binsr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_binsr_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_binsri_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_binsri_df(td, cpu_env, td, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_addv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_addv_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_addvi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_addv_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_subv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_subv_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_subvi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_subv_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_max_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_max_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_maxi_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_max_s_df(td, cpu_env, ts, ts5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_max_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_max_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_maxi_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_max_u_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_min_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_min_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_mini_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_min_s_df(td, cpu_env, ts, ts5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_min_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_min_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_mini_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_min_u_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_max_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_max_a_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_min_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_min_a_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_ceq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_ceq_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_ceqi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_ceq_df(td, cpu_env, ts, ts5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_clt_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_clt_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_clti_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_clt_s_df(td, cpu_env, ts, ts5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_clt_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_clt_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_clti_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_clt_u_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_cle_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_cle_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_clei_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_cle_s_df(td, cpu_env, ts, ts5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_cle_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_cle_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_clei_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_cle_u_df(td, cpu_env, ts, tu5, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_ld_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = mi10_rs_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;

    int64_t s10 = (ctx->opcode >> 16) & 0x3ff /* s10 [25:16] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;
    uint8_t df = (ctx->opcode >> 0) & 0x3 /* df [1:0] */;

    int16_t offset = s10 << df;

    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    /* 
     *  set element granularity to 8 bits (df = 0) because the load
     *  is implemented using tcg_gen_qemu_ld8u()
     */
    df = 0;
    int df_bits = 8 * (1 << df);

    int i;
    TCGv td = tcg_temp_new();
    TCGv taddr = tcg_temp_new();
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        gen_base_offset_addr(ctx, taddr, rs, offset + (i << df));
        tcg_gen_qemu_ld8u(td, taddr, ctx->mem_idx);
        gen_helper_store_wr_elem_target(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(twd);
    tcg_temp_free(td);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_st_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = mi10_rs_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;

    int64_t s10 = (ctx->opcode >> 16) & 0x3ff /* s10 [25:16] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;
    uint8_t df = (ctx->opcode >> 0) & 0x3 /* df [1:0] */;

    int16_t offset = s10 << df;

    if (!check_msa_access(env, ctx, wd, -1, -1)) return;

    /* 
     *  set element granularity to 8 bits (df = 0) because the store
     *  is implemented using tcg_gen_qemu_st8()
     */
    df = 0;
    int df_bits = 8 * (1 << df);

    int i;
    TCGv td = tcg_temp_new();
    TCGv taddr = tcg_temp_new();
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);

    for (i = 0; i < wrlen / df_bits; i++) {
        TCGv_i32 ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_target_i64(td, cpu_env, twd, tdf, ti);
        gen_base_offset_addr(ctx, taddr, rs, offset + (i << df));
        tcg_gen_qemu_st8(td, taddr, ctx->mem_idx);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(td);
    tcg_temp_free_i32(twd);
    tcg_temp_free(taddr);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_sat_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_sat_s_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_sat_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_sat_u_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_add_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_add_a_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_adds_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_adds_a_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_adds_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_adds_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_adds_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_i64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_i64(tt, cpu_env, twt, tdf, ti);
        gen_helper_adds_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_ave_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_ave_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_ave_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_i64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_i64(tt, cpu_env, twt, tdf, ti);
        gen_helper_ave_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_aver_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_aver_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_aver_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_i64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_i64(tt, cpu_env, twt, tdf, ti);
        gen_helper_aver_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_subs_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_subs_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_subs_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_subs_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_subsus_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_subsus_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_subsuu_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_subsuu_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_asub_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_asub_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_asub_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_i64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_i64(tt, cpu_env, twt, tdf, ti);
        gen_helper_asub_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_mulv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_mulv_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_maddv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_maddv_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_msubv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_msubv_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_div_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_div_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_div_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_div_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_mod_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_mod_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_mod_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_mod_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dotp_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_dotp_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dotp_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_dotp_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dpadd_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_dpadd_s_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dpadd_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_dpadd_u_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dpsub_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_dpsub_s_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_dpsub_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_dpsub_u_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_sld_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_rt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv trt = tcg_temp_new();
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trt, rt);
    gen_helper_sld_df(cpu_env, tpwd, tpws, trt, twrlen_df);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free(trt);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_sldi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv tn  = tcg_const_tl(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_sld_df(cpu_env, tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_splat_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_rt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv trt = tcg_temp_new();
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trt, rt);
    gen_helper_splat_df(cpu_env, tpwd, tpws, trt, twrlen_df);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free(trt);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_splati_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv tn  = tcg_const_tl(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_splat_df(cpu_env, tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_pckev_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_pckev_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_pckod_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_pckod_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ilvl_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvl_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ilvr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvr_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ilvev_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvev_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ilvod_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvod_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_vshf_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_vshf_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_srar_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_srar_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_srari_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_srari_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_srlr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_srlr_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_srlri_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_srlri_df(td, cpu_env, ts, tm, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tm);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);

    update_msa_modify(env, ctx, wd);
}

static void gen_hadd_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_hadd_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_hadd_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_hadd_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_hsub_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_hsub_s_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_hsub_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_hsub_u_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_and_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_and_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_andi_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_andi_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_or_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_or_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_ori_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_ori_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_nor_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_nor_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_nori_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_nori_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_xor_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_xor_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_xori_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_xori_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_bmnz_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bmnz_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_bmnzi_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bmnzi_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_bmz_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bmz_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_bmzi_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bmzi_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_bsel_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bsel_v(cpu_env, tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_bseli_b(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bseli_b(cpu_env, tpwd, tpws, ti8, twrlen);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_shf_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_i8_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_shf_df(cpu_env, tpwd, tpws, ti8, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);
    tcg_temp_free_i32(ti8);

    update_msa_modify(env, ctx, wd);
}

static void gen_bnz_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = s16_wt_branch */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i32 ts16 = tcg_const_i32(s16);
    TCGv_ptr tpwt  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();
    gen_helper_bnz_v(tbcond, cpu_env, tpwt, twrlen);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(ts16);
    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_i32(twrlen);
}

static void gen_bz_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = s16_wt_branch */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i32 ts16 = tcg_const_i32(s16);
    TCGv_ptr tpwt  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();
    gen_helper_bz_v(tbcond, cpu_env, tpwt, twrlen);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(ts16);
    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_i32(twrlen);
}

static void gen_fill_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_rs_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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

    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trs, rs);
    gen_helper_fill_df(cpu_env, tpwd, trs, twrlen_df);

    tcg_temp_free(trs);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_pcnt_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_pcnt_df(td, cpu_env, ts, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(td);

    update_msa_modify(env, ctx, wd);
}

static void gen_nloc_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_nloc_df(td, cpu_env, ts, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(td);

    update_msa_modify(env, ctx, wd);
}

static void gen_nlzc_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_nlzc_df(td, cpu_env, ts, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(td);

    update_msa_modify(env, ctx, wd);
}

static void gen_copy_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_rd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
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

    TCGv telm = tcg_temp_new();
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_load_wr_elem_target_s64(telm, cpu_env, tws, tdf, tn);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
}

static void gen_copy_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_rd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
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

    TCGv telm = tcg_temp_new();
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tn = tcg_const_i32(n);

    gen_helper_load_wr_elem_target_i64(telm, cpu_env, tws, tdf, tn);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tn);
}

static void gen_insert_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_rs_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
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

    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_load_gpr(trs, rs);
    gen_helper_insert_df(cpu_env, tpwd, trs, tn, twrlen_df);

    tcg_temp_free(trs);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_insve_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    if ( n >= wrlen / df_bits ) {
        if (check_msa_access(env, ctx, -1, -1, -1))
            generate_exception(ctx, EXCP_RI);
        return;
    }

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv tn  = tcg_const_tl(n);

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_insve_df(cpu_env, tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free(tn);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_bnz_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s16_wt_branch */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts16 = tcg_const_i32(s16);
    TCGv_ptr tpwt  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();

    gen_helper_bnz_df(tbcond, cpu_env, tpwt, tdf, twrlen);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts16);
    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_i32(twrlen);
}

static void gen_bz_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s16_wt_branch */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    int64_t s16 = (ctx->opcode >> 0) & 0xffff /* s16 [15:0] */;
    s16 = (s16 << 48) >> 48; /* sign extend s16 to 64 bits*/

    if (!check_msa_access(env, ctx, wt, -1, -1)) return;
    gen_check_delay_fbn_slot(ctx);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts16 = tcg_const_i32(s16);
    TCGv_ptr tpwt  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    TCGv tbcond = tcg_temp_new();

    gen_helper_bz_df(tbcond, cpu_env, tpwt, tdf, twrlen);

    int64_t offset = s16 << 2;
    ctx->btarget = ctx->pc + offset + 4; /* insn_bytes hardcoded 4 */

    tcg_gen_setcondi_tl(TCG_COND_NE, bcond, tbcond, 0);
    ctx->hflags |= MIPS_HFLAG_BC;

    tcg_temp_free(tbcond);
    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts16);
    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_i32(twrlen);
}

static void gen_ldi_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_s10_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    int64_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    s10 = (s10 << 54) >> 54; /* sign extend s10 to 64 bits*/
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wd, wd, wd)) return;

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_ldi_df(cpu_env, tpwd, tdf, ts10, twrlen);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcaf_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcaf_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcun_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcun_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fceq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fceq_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcueq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcueq_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fclt_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fclt_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcult_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcult_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcle_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcle_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcule_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcule_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsaf_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsaf_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsun_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsun_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fseq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fseq_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsueq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsueq_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fslt_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fslt_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsult_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsult_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsle_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsle_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsule_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsule_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fadd_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fadd_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsub_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsub_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmul_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmul_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fdiv_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fdiv_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmadd_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmadd_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmsub_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmsub_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fexp2_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexp2_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fexdo_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexdo_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ftq_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftq_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmin_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmin_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmin_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmin_a_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmax_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmax_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fmax_a_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmax_a_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcor_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcor_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcune_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcune_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fcne_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fcne_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_mul_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_mul_q_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_madd_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_madd_q_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_msub_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_msub_q_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsor_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsor_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsune_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsune_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsne_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, wt, ws, wd)) return;

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsne_df(cpu_env, tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_ptr(tpwt);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_mulr_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_mulr_q_df(td, cpu_env, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_maddr_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_maddr_q_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_msubr_q_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i64 tt = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen / df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_load_wr_elem_s64(tt, cpu_env, twt, tdf, ti);
        gen_helper_load_wr_elem_s64(td, cpu_env, twd, tdf, ti);
        gen_helper_msubr_q_df(td, cpu_env, td, ts, tt, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i64(td);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(tt);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twt);
    tcg_temp_free_i32(tdf);

    update_msa_modify(env, ctx, wd);
}

static void gen_fclass_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

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
        gen_helper_load_wr_elem_s64(ts, cpu_env, tws, tdf, ti);
        gen_helper_fclass_df(td, cpu_env, ts, tdf);
        gen_helper_store_wr_elem(cpu_env, td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free_i64(ts);
    tcg_temp_free_i64(td);

    update_msa_modify(env, ctx, wd);
}

static void gen_ftrunc_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftrunc_s_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ftrunc_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftrunc_u_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fsqrt_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsqrt_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_frsqrt_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_frsqrt_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_frcp_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_frcp_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_frint_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_frint_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_flog2_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_flog2_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fexupl_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexupl_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_fexupr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexupr_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ffql_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ffql_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ffqr_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ffqr_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ftint_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftint_s_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ftint_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftint_u_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ffint_s_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ffint_s_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ffint_u_df(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;


    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ffint_u_df(cpu_env, tpwd, tpws, twrlen_df);

    tcg_temp_free_ptr(tpws);
    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_i32(twrlen_df);

    update_msa_modify(env, ctx, wd);
}

static void gen_ctcmsa(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = rs_cd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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

static void gen_cfcmsa(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = cs_rd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


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

static void gen_move_v(CPUMIPSState *env, DisasContext *ctx) {
    /* func_type = ws_wd */

    /* Implementation fixed to 128-bit vector registers */
    int __attribute__((unused)) wrlen = 128;


    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    if (!check_msa_access(env, ctx, ws, ws, wd)) return;

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_fpu.fpr[wd]));

    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_move_v(cpu_env, tpwd, tpws, twrlen);

    tcg_temp_free_ptr(tpwd);
    tcg_temp_free_ptr(tpws);
    tcg_temp_free_i32(twrlen);

    update_msa_modify(env, ctx, wd);
}

