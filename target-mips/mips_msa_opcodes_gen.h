static void gen_addv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_addv_df(td, ts, tt, tdf);
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
}

static void gen_addvi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_addv_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_add_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_add_a_df(td, ts, tt, tdf);
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
}

static void gen_addi_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_add_a_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_adds_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_adds_a_df(td, ts, tt, tdf);
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
}

static void gen_addsi_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_adds_a_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_adds_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_adds_s_df(td, ts, tt, tdf);
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
}

static void gen_addsi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_adds_s_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_adds_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_load_wr_i64(tt, twt, tdf, ti);
        gen_helper_adds_u_df(td, ts, tt, tdf);
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
}

static void gen_addsi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_adds_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_subv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_subv_df(td, ts, tt, tdf);
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
}

static void gen_subvi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_subv_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_asub_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_asub_s_df(td, ts, tt, tdf);
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
}

static void gen_asubi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_asub_s_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_asub_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_load_wr_i64(tt, twt, tdf, ti);
        gen_helper_asub_u_df(td, ts, tt, tdf);
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
}

static void gen_asubi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_asub_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_subs_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_subs_s_df(td, ts, tt, tdf);
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
}

static void gen_subsi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_subs_s_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_subs_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_subs_u_df(td, ts, tt, tdf);
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
}

static void gen_subsi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_subs_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_subss_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_subss_u_df(td, ts, tt, tdf);
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
}

static void gen_subssi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_subss_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_max_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_max_a_df(td, ts, tt, tdf);
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
}

static void gen_maxi_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_max_a_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_max_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_max_s_df(td, ts, tt, tdf);
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
}

static void gen_maxi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_max_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_max_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_max_u_df(td, ts, tt, tdf);
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
}

static void gen_maxi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_max_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_min_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_min_a_df(td, ts, tt, tdf);
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
}

static void gen_mini_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_min_a_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_min_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_min_s_df(td, ts, tt, tdf);
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
}

static void gen_mini_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_min_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_min_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_min_u_df(td, ts, tt, tdf);
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
}

static void gen_mini_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_min_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_ave_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_ave_s_df(td, ts, tt, tdf);
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
}

static void gen_avei_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ave_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_ave_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_load_wr_i64(tt, twt, tdf, ti);
        gen_helper_ave_u_df(td, ts, tt, tdf);
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
}

static void gen_avei_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_i64(ts, tws, tdf, ti);
        gen_helper_ave_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_sat_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_sat_s_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_sat_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_sat_u_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_mulv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_mulv_df(td, ts, tt, tdf);
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
}

static void gen_mulvi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_mulv_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_maddv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_maddv_df(td, td, ts, tt, tdf);
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
}

static void gen_maddvi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_maddv_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_msubv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_msubv_df(td, td, ts, tt, tdf);
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
}

static void gen_msubvi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_msubv_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dotp_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_dotp_s_df(td, ts, tt, tdf);
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
}

static void gen_dotpi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_dotpi_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dotp_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_dotp_u_df(td, ts, tt, tdf);
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
}

static void gen_dotpi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_dotpi_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dpadd_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpadd_s_df(td, td, ts, tt, tdf);
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
}

static void gen_dpaddi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpaddi_s_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dpadd_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpadd_u_df(td, td, ts, tt, tdf);
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
}

static void gen_dpaddi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpaddi_u_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dpsub_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpsub_s_df(td, td, ts, tt, tdf);
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
}

static void gen_dpsubi_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpsubi_s_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_dpsub_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpsub_u_df(td, td, ts, tt, tdf);
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
}

static void gen_dpsubi_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_dpsubi_u_df(td, td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_mul_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_mul_q_df(td, ts, tt, tdf);
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
}

static void gen_mulr_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_mulr_q_df(td, ts, tt, tdf);
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
}

static void gen_madd_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_madd_q_df(td, td, ts, tt, tdf);
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
}

static void gen_maddr_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_maddr_q_df(td, td, ts, tt, tdf);
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
}

static void gen_msub_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_msub_q_df(td, td, ts, tt, tdf);
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
}

static void gen_msubr_q_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    /* adjust df value for fixed-point instruction */
    df = df + 1;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_msubr_q_df(td, td, ts, tt, tdf);
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
}

static void gen_and_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_and_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_andi_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_andi_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_or_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_or_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_ori_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_ori_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_nor_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_nor_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_nori_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_nori_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_xor_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_xor_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_xori_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_xori_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_shl_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_shl_df(td, ts, tt, tdf);
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
}

static void gen_shli_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_shli_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_sra_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_sra_df(td, ts, tt, tdf);
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
}

static void gen_srai_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_srai_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_srl_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_srl_df(td, ts, tt, tdf);
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
}

static void gen_srli_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_srli_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_pcnt_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_pcnt_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_nloc_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_nloc_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_nlzc_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_nlzc_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_binsl_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_binsl_df(td, td, ts, tt, tdf);
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
}

static void gen_binsli_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_binsli_df(td, td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_binsr_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_binsr_df(td, td, ts, tt, tdf);
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
}

static void gen_binsri_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(td, twd, tdf, ti);
        gen_helper_binsri_df(td, td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_bclr_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_bclr_df(td, ts, tt, tdf);
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
}

static void gen_bclri_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_bclri_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_bset_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_bset_df(td, ts, tt, tdf);
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
}

static void gen_bseti_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfm_ws_wd */

    uint8_t dfm = (ctx->opcode >> 16) & 0x7f /* dfm [22:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tm  = tcg_const_i32(m);
    TCGv_i64 twd = tcg_const_i64(wd);
    TCGv_i64 tws = tcg_const_i64(ws);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_bseti_df(td, ts, tm, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(tm);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_bmnz_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bmnz_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bmnzi_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bmnzi_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_bmz_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bmz_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bmzi_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bmzi_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_bsel_v(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_bsel_v(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bseli_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_bseli_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_ceq_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_ceq_df(td, ts, tt, tdf);
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
}

static void gen_ceqi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ceq_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_clt_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_clt_s_df(td, ts, tt, tdf);
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
}

static void gen_clti_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_clt_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_cle_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_cle_s_df(td, ts, tt, tdf);
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
}

static void gen_clei_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 ts5 = tcg_const_i64(s5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_cle_s_df(td, ts, ts5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(ts5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_clt_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_clt_u_df(td, ts, tt, tdf);
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
}

static void gen_clti_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_clt_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_cle_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_cle_u_df(td, ts, tt, tdf);
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
}

static void gen_clei_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_u5_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t u5 = (ctx->opcode >> 16) & 0x1f /* u5 [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i64 tu5 = tcg_const_i64(u5);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_cle_u_df(td, ts, tu5, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free(tdf);
    tcg_temp_free(twd);
    tcg_temp_free(tws);
    tcg_temp_free_i64(tu5);
    tcg_temp_free(td);
    tcg_temp_free(ts);
}

static void gen_bnz_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s10_wd_branch */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_bnz_df(tdf, tpwd, tdf, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bz_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s10_wd_branch */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_bz_df(tdf, tpwd, tdf, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bnz_v(CPUState *env, DisasContext *ctx) {
    /* func_type = s10_wd_branch */

    uint8_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_i32 tdf  = tcg_const_i32(0); /* where is df? */
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_bnz_v(tdf, tpwd, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_bz_v(CPUState *env, DisasContext *ctx) {
    /* func_type = s10_wd_branch */

    uint8_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_i32 tdf  = tcg_const_i32(0); /* where is df? */
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    assert(0); /* fix me, need to return branch/true/false, and branch */

    gen_helper_bz_v(tdf, tpwd, twrlen);

    /* reuse tdf as branch condition? */

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_sld_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_wd */

    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_sld_df(tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_shf_b(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_shf_b(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_shf_h(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_shf_h(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_shf_w(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_shf_w(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_shf_d(CPUState *env, DisasContext *ctx) {
    /* func_type = i8_ws_wd */

    uint8_t i8 = (ctx->opcode >> 16) & 0xff /* i8 [23:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME
    TCGv_i32 ti8 = tcg_const_i32(i8); // FIXME

    gen_helper_shf_d(tpwd, tpws, ti8, twrlen);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
    tcg_temp_free_i32(ti8);
}

static void gen_vshf_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_vshf_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_pckev_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_pckev_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_pckev_q(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_pckev_q(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_pckod_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_pckod_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_pckod_q(CPUState *env, DisasContext *ctx) {
    /* func_type = wt_ws_wd */

    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_pckod_q(tpwd, tpws, tpwt, twrlen);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_ilvl_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvl_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_ilvr_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvr_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_ilvev_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvev_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_ilvod_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ilvod_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_move_v(CPUState *env, DisasContext *ctx) {
    /* func_type = ws_wd */

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_move_v(tpwd, tpws, twrlen);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(twrlen);
}

static void gen_move_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_wd */

    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    TCGv_i32 tn  = tcg_const_i32(n);

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_move_df(tpwd, tpws, tn, twrlen_df);

    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i32(tn);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_ctcmsa(CPUState *env, DisasContext *ctx) {
    /* func_type = rs_cd */

    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t cd = (ctx->opcode >> 6) & 0x1f /* cd [10:6] */;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcd = tcg_const_i32(cd);

    gen_load_gpr(telm, rs);
    gen_helper_ctcmsa(telm, tcd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcd);

}

static void gen_mvtg_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_rd */

    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

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

}

static void gen_cfcmsa(CPUState *env, DisasContext *ctx) {
    /* func_type = cs_rd */

    uint8_t cs = (ctx->opcode >> 11) & 0x1f /* cs [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

    TCGv telm = tcg_temp_new();
    TCGv_i32 tcs = tcg_const_i32(cs);

    gen_helper_cfcmsa(telm, tcs);
    gen_store_gpr(telm, rd);

    tcg_temp_free(telm);
    tcg_temp_free_i32(tcs);

}

static void gen_mvfg_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_rs_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x3 /* df [17:16] */;
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv telm = tcg_temp_new();
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i32 tdf = tcg_const_i32(df);
    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
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
}

static void gen_mvtg_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = dfn_ws_rd */

    uint8_t dfn = (ctx->opcode >> 16) & 0x3f /* dfn [21:16] */;

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

    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t rd = (ctx->opcode >> 6) & 0x1f /* rd [10:6] */;

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

}

static void gen_ld_v(CPUState *env, DisasContext *ctx) {
    /* func_type = s5_rs_wd */

            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts5 = tcg_const_i32(s5);
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    TCGv trs = tcg_temp_new();
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trs, rs);
    gen_helper_ld_v(tpwd, ts5, trs, twrlen);

    tcg_temp_free_i32(ts5);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twrlen);
}

static void gen_ldi_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_s10_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x3 /* df [22:21] */;
    uint8_t s10 = (ctx->opcode >> 11) & 0x3ff /* s10 [20:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 tdf  = tcg_const_i32(df);
    TCGv_i32 ts10 = tcg_const_i32(s10);
    TCGv_ptr tpwd  = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));

    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen);

    gen_helper_ldi_df(tpwd, tdf, ts10, twrlen);

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(ts10);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);
}

static void gen_ldx_v(CPUState *env, DisasContext *ctx) {
    /* func_type = rt_rs_wd */

    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    gen_helper_ldx_v(tpwd, trt, trs, twrlen);

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

}

static void gen_st_v(CPUState *env, DisasContext *ctx) {
    /* func_type = s5_rs_wd */

            int64_t s5 = (ctx->opcode >> 16) & 0x1f /* s5 [20:16] */;
            s5 = (s5 << 59) >> 59; /* sign extend s5 to 64 bits*/
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv_i32 ts5 = tcg_const_i32(s5);
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    TCGv trs = tcg_temp_new();
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trs, rs);
    gen_helper_st_v(tpwd, ts5, trs, twrlen);

    tcg_temp_free_i32(ts5);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free(trs);
    tcg_temp_free_i32(twrlen);
}

static void gen_stx_v(CPUState *env, DisasContext *ctx) {
    /* func_type = rt_rs_wd */

    uint8_t rt = (ctx->opcode >> 16) & 0x1f /* rt [20:16] */;
    uint8_t rs = (ctx->opcode >> 11) & 0x1f /* rs [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wd, wd, wd);

    TCGv trt = tcg_temp_new();
    TCGv trs = tcg_temp_new();
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen = tcg_const_i32(wrlen); // FIXME

    gen_load_gpr(trt, rt);
    gen_load_gpr(trs, rs);
    gen_helper_stx_v(tpwd, trt, trs, twrlen);

    tcg_temp_free(trt);
    tcg_temp_free(trs);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen);

}

static void gen_fadd_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fadd_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fsub_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsub_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fmul_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmul_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fdiv_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fdiv_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_frem_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_frem_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fsqrt_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fsqrt_df(tpwd, tpws, twrlen_df);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fmadd_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmadd_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fmsub_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fmsub_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_frint_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_frint_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_flog2_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd_p */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_flog2_df(tpwd, tpws, twrlen_df);

    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fexp2_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexp2_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fmax_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fmax_df(td, ts, tt, tdf);
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
}

static void gen_fmax_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fmax_a_df(td, ts, tt, tdf);
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
}

static void gen_fmin_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fmin_df(td, ts, tt, tdf);
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
}

static void gen_fmin_a_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fmin_a_df(td, ts, tt, tdf);
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
}

static void gen_fceq_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fceq_df(td, ts, tt, tdf);
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
}

static void gen_fclt_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fclt_df(td, ts, tt, tdf);
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
}

static void gen_fcle_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fcle_df(td, ts, tt, tdf);
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
}

static void gen_fcun_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fcun_df(td, ts, tt, tdf);
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
}

static void gen_fcequ_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fcequ_df(td, ts, tt, tdf);
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
}

static void gen_fcltu_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fcltu_df(td, ts, tt, tdf);
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
}

static void gen_fcleu_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

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
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_load_wr_s64(tt, twt, tdf, ti);
        gen_helper_fcleu_df(td, ts, tt, tdf);
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
}

static void gen_fclass_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_fclass_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_ftint_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ftint_s_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_ffint_s_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ffint_s_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_ftint_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ftint_u_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_ffint_u_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_ws_wd */

    uint8_t df = (ctx->opcode >> 16) & 0x1 /* df [16:16] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, ws, ws, wd);

    TCGv_i32 tdf = tcg_const_i32(df);
    TCGv_i32 tws = tcg_const_i32(ws);
    TCGv_i32 twd = tcg_const_i32(wd);
    TCGv_i64 td = tcg_temp_new_i64();
    TCGv_i64 ts = tcg_temp_new_i64();
    TCGv_i32 ti;

    int i;
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    int df_bits = 8 * (1 << df);

    for (i = 0; i < wrlen/df_bits; i++) {
        ti = tcg_const_i32(i);
        gen_helper_load_wr_s64(ts, tws, tdf, ti);
        gen_helper_ffint_u_df(td, ts, tdf);
        gen_helper_store_wr(td, twd, tdf, ti);
        tcg_temp_free_i32(ti);
    }

    tcg_temp_free_i32(tdf);
    tcg_temp_free_i32(tws);
    tcg_temp_free_i32(twd);
    tcg_temp_free(ts);
    tcg_temp_free(td);
}

static void gen_ftq_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ftq_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_ffq_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_ffq_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fexdo_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexdo_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

static void gen_fexup_df(CPUState *env, DisasContext *ctx) {
    /* func_type = df_wt_ws_wd_p */

    uint8_t df = (ctx->opcode >> 21) & 0x1 /* df [21:21] */;
    check_msa_fp(env, ctx);

    /* adjust df value for floating-point instruction */
    df = df + 2;
    uint8_t wt = (ctx->opcode >> 16) & 0x1f /* wt [20:16] */;
    uint8_t ws = (ctx->opcode >> 11) & 0x1f /* ws [15:11] */;
    uint8_t wd = (ctx->opcode >> 6) & 0x1f /* wd [10:6] */;

    check_msa_access(env, ctx, wt, ws, wd);

    TCGv_ptr tpwt = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wt]));
    TCGv_ptr tpws = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[ws]));
    TCGv_ptr tpwd = tcg_const_ptr((tcg_target_long)&(env->active_msa.wr[wd]));
    int wrlen = (env->active_msa.msair & MSAIR_W_BIT) ? 256 : 128;
    TCGv_i32 twrlen_df = tcg_const_i32((wrlen << 2) | df);

    gen_helper_fexup_df(tpwd, tpws, tpwt, twrlen_df);

    tcg_temp_free_i64(tpwt);
    tcg_temp_free_i64(tpws);
    tcg_temp_free_i64(tpwd);
    tcg_temp_free_i32(twrlen_df);
}

