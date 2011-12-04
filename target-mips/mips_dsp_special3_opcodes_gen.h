static void gen_ABSQ_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_absq_s_ph, rd, rt);

    MIPS_DEBUG("%s %s, %s", "absq_s.ph", regnames[rd], regnames[rt]);
}

static void gen_ABSQ_S_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 2, gen_helper_absq_s_qb, rd, rt);

    MIPS_DEBUG("%s %s, %s", "absq_s.qb", regnames[rd], regnames[rt]);
}

static void gen_ABSQ_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_absq_s_w, rd, rt);

    MIPS_DEBUG("%s %s, %s", "absq_s.w", regnames[rd], regnames[rt]);
}

static void gen_BITREV(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_bitrev, rd, rt);

    MIPS_DEBUG("%s %s, %s", "bitrev", regnames[rd], regnames[rt]);
}

static void gen_PRECEQ_W_PHL(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceq_w_phl, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceq.w.phl", regnames[rd], regnames[rt]);
}

static void gen_PRECEQ_W_PHR(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceq_w_phr, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceq.w.phr", regnames[rd], regnames[rt]);
}

static void gen_PRECEQU_PH_QBL(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_precequ_ph_qbl, rd, rt);

    MIPS_DEBUG("%s %s, %s", "precequ.ph.qbl", regnames[rd], regnames[rt]);
}

static void gen_PRECEQU_PH_QBLA(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_precequ_ph_qbla, rd, rt);

    MIPS_DEBUG("%s %s, %s", "precequ.ph.qbla", regnames[rd], regnames[rt]);
}

static void gen_PRECEQU_PH_QBR(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_precequ_ph_qbr, rd, rt);

    MIPS_DEBUG("%s %s, %s", "precequ.ph.qbr", regnames[rd], regnames[rt]);
}

static void gen_PRECEQU_PH_QBRA(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_precequ_ph_qbra, rd, rt);

    MIPS_DEBUG("%s %s, %s", "precequ.ph.qbra", regnames[rd], regnames[rt]);
}

static void gen_PRECEU_PH_QBL(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceu_ph_qbl, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceu.ph.qbl", regnames[rd], regnames[rt]);
}

static void gen_PRECEU_PH_QBLA(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceu_ph_qbla, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceu.ph.qbla", regnames[rd], regnames[rt]);
}

static void gen_PRECEU_PH_QBR(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceu_ph_qbr, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceu.ph.qbr", regnames[rd], regnames[rt]);
}

static void gen_PRECEU_PH_QBRA(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_preceu_ph_qbra, rd, rt);

    MIPS_DEBUG("%s %s, %s", "preceu.ph.qbra", regnames[rd], regnames[rt]);
}

static void gen_REPL_PH(CPUState *env, DisasContext *ctx)
{
    uint16_t immediate = (ctx->opcode >> 16) & 0x3ff;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_immed_rd_wrapper(env, ctx, 1, gen_helper_repl_ph, immediate,
        rd);

    MIPS_DEBUG("%s %s, 0x%x", "repl.ph", regnames[rd], immediate);
}

static void gen_REPL_QB(CPUState *env, DisasContext *ctx)
{
    uint16_t immediate = (ctx->opcode >> 16) & 0xff;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_immed_rd_wrapper(env, ctx, 1, gen_helper_repl_qb, immediate,
        rd);

    MIPS_DEBUG("%s %s, 0x%x", "repl.qb", regnames[rd], immediate);
}

static void gen_REPLV_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_replv_ph, rd, rt);

    MIPS_DEBUG("%s %s, %s", "replv.ph", regnames[rd], regnames[rt]);
}

static void gen_REPLV_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_unary_rd_rt_wrapper(env, ctx, 1, gen_helper_replv_qb, rd, rt);

    MIPS_DEBUG("%s %s, %s", "replv.qb", regnames[rd], regnames[rt]);
}

static void gen_ADDQ_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addq_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addq.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQ_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addq_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addq_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQ_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addq_s_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addq_s.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDSC(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addsc,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addsc", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDU_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addu_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addu.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDU_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addu_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addu.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDU_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addu_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addu_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDU_S_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addu_s_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addu_s.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDWC(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_addwc,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addwc", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MODSUB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_modsub,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "modsub", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULEQ_S_W_PHL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_muleq_s_w_phl,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "muleq_s.w.phl", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULEQ_S_W_PHR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_muleq_s_w_phr,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "muleq_s.w.phr", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULEU_S_PH_QBL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_muleu_s_ph_qbl,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "muleu_s.ph.qbl", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULEU_S_PH_QBR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_muleu_s_ph_qbr,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "muleu_s.ph.qbr", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULQ_RS_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_mulq_rs_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mulq_rs.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULQ_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_mulq_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mulq_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_RADDU_W_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_wrapper(env, ctx, 1, gen_helper_raddu_w_qb, rs, rd);

    MIPS_DEBUG("%s %s, %s", "raddu.w.qb", regnames[rd], regnames[rs]);
}

static void gen_SUBQ_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_subq_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subq.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQ_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_subq_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subq_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQ_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_subq_s_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subq_s.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBU_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subu_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subu.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBU_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_subu_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subu.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBU_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subu_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subu_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBU_S_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_subu_s_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subu_s.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQH_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addqh_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addqh.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addqh_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addqh.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQH_R_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addqh_r_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addqh_r.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDQH_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_addqh_r_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "addqh_r.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDUH_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_adduh_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "adduh.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_ADDUH_R_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_adduh_r_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "adduh_r.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MUL_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_mul_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mul.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULQ_RS_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_mulq_rs_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mulq_rs.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MULQ_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_mulq_s_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mulq_s.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_MUL_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_mul_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "mul_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQH_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subqh_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subqh.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subqh_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subqh.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQH_R_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subqh_r_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subqh_r.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBQH_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subqh_r_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subqh_r.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBUH_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subuh_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subuh.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SUBUH_R_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_subuh_r_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "subuh_r.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_APPEND(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t sa = (ctx->opcode >> 11) & 0x1f;

    gen_ternary_rs_rt_sa_wrapper(env, ctx, 2, gen_helper_append,
        rs, rt, sa);

    MIPS_DEBUG("%s %s, %s, %d", "append", regnames[rt],
        regnames[rs], sa);
}

static void gen_BALIGN(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t bp = (ctx->opcode >> 11) & 0x1f;

    gen_ternary_rs_rt_bp_wrapper(env, ctx, 2, gen_helper_balign, rs, rt, bp);

    MIPS_DEBUG("%s %s, %s, %d", "balign", regnames[rt], regnames[rs], bp);
}

static void gen_PREPEND(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t sa = (ctx->opcode >> 11) & 0x1f;

    gen_ternary_rs_rt_sa_wrapper(env, ctx, 2, gen_helper_prepend,
        rs, rt, sa);

    MIPS_DEBUG("%s %s, %s, %d", "prepend", regnames[rt],
        regnames[rs], sa);
}

static void gen_CMP_EQ_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmp_eq_ph, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmp.eq.ph", regnames[rs], regnames[rt]);
}

static void gen_CMP_LE_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmp_le_ph, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmp.le.ph", regnames[rs], regnames[rt]);
}

static void gen_CMP_LT_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmp_lt_ph, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmp.lt.ph", regnames[rs], regnames[rt]);
}

static void gen_CMPGDU_EQ_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_cmpgdu_eq_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgdu.eq.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPGDU_LE_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_cmpgdu_le_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgdu.le.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPGDU_LT_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_cmpgdu_lt_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgdu.lt.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPGU_EQ_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_cmpgu_eq_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgu.eq.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPGU_LE_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_cmpgu_le_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgu.le.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPGU_LT_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_cmpgu_lt_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "cmpgu.lt.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_CMPU_EQ_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmpu_eq_qb, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmpu.eq.qb", regnames[rs], regnames[rt]);
}

static void gen_CMPU_LE_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmpu_le_qb, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmpu.le.qb", regnames[rs], regnames[rt]);
}

static void gen_CMPU_LT_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_wrapper(env, ctx, 1, gen_helper_cmpu_lt_qb, rs, rt);

    MIPS_DEBUG("%s %s, %s", "cmpu.lt.qb", regnames[rs], regnames[rt]);
}

static void gen_PACKRL_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_packrl_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "packrl.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PICK_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_pick_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "pick.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PICK_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_pick_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "pick.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECR_QB_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_precr_qb_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "precr.qb.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECRQ_PH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_precrq_ph_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "precrq.ph.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECRQ_QB_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_precrq_qb_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "precrq.qb.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECRQU_S_QB_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_precrqu_s_qb_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "precrqu_s.qb.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECRQ_RS_PH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_precrq_rs_ph_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "precrq_rs.ph.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_PRECR_SRA_PH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t sa = (ctx->opcode >> 11) & 0x1f;

    gen_ternary_rs_rt_sa_wrapper(env, ctx, 2, gen_helper_precr_sra_ph_w,
        rs, rt, sa);

    MIPS_DEBUG("%s %s, %s, %d", "precr_sra.ph.w", regnames[rt],
        regnames[rs], sa);
}

static void gen_PRECR_SRA_R_PH_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t sa = (ctx->opcode >> 11) & 0x1f;

    gen_ternary_rs_rt_sa_wrapper(env, ctx, 2, gen_helper_precr_sra_r_ph_w,
        rs, rt, sa);

    MIPS_DEBUG("%s %s, %s, %d", "precr_sra_r.ph.w", regnames[rt],
        regnames[rs], sa);
}

static void gen_DPA_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 2, gen_helper_dpa_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpa.w.ph", ac, regnames[rs], regnames[rt]);
}

static void gen_DPAQX_S_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 2, gen_helper_dpaqx_s_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpaqx_s.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPAQX_SA_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 2, gen_helper_dpaqx_sa_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpaqx_sa.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPAQ_S_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_dpaq_s_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpaq_s.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPAQ_SA_L_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_dpaq_sa_l_w,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpaq_sa.l.w", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPAU_H_QBL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 1, gen_helper_dpau_h_qbl,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpau.h.qbl", ac, regnames[rs], regnames[rt]);
}

static void gen_DPAU_H_QBR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 1, gen_helper_dpau_h_qbr,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpau.h.qbr", ac, regnames[rs], regnames[rt]);
}

static void gen_DPAX_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 2, gen_helper_dpax_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpax.w.ph", ac, regnames[rs], regnames[rt]);
}

static void gen_DPS_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 2, gen_helper_dps_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dps.w.ph", ac, regnames[rs], regnames[rt]);
}

static void gen_DPSQX_S_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 2, gen_helper_dpsqx_s_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsqx_s.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPSQX_SA_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 2, gen_helper_dpsqx_sa_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsqx_sa.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPSQ_S_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_dpsq_s_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsq_s.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPSQ_SA_L_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_dpsq_sa_l_w,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsq_sa.l.w", ac, regnames[rs],
        regnames[rt]);
}

static void gen_DPSU_H_QBL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 1, gen_helper_dpsu_h_qbl,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsu.h.qbl", ac, regnames[rs], regnames[rt]);
}

static void gen_DPSU_H_QBR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 1, gen_helper_dpsu_h_qbr,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsu.h.qbr", ac, regnames[rs], regnames[rt]);
}

static void gen_DPSX_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 2, gen_helper_dpsx_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "dpsx.w.ph", ac, regnames[rs], regnames[rt]);
}

static void gen_MAQ_S_W_PHL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_maq_s_w_phl,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "maq_s.w.phl", ac, regnames[rs],
        regnames[rt]);
}

static void gen_MAQ_S_W_PHR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_maq_s_w_phr,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "maq_s.w.phr", ac, regnames[rs],
        regnames[rt]);
}

static void gen_MAQ_SA_W_PHL(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_maq_sa_w_phl,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "maq_sa.w.phl", ac, regnames[rs],
        regnames[rt]);
}

static void gen_MAQ_SA_W_PHR(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_maq_sa_w_phr,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "maq_sa.w.phr", ac, regnames[rs],
        regnames[rt]);
}

static void gen_MULSA_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac_wrapper(env, ctx, 2, gen_helper_mulsa_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "mulsa.w.ph", ac, regnames[rs], regnames[rt]);
}

static void gen_MULSAQ_S_W_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_ternary_rs_rt_ac2_wrapper(env, ctx, 1, gen_helper_mulsaq_s_w_ph,
        rs, rt, ac);

    MIPS_DEBUG("%s %d, %s, %s", "mulsaq_s.w.ph", ac, regnames[rs],
        regnames[rt]);
}

static void gen_EXTP(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t size = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extp, rt, ac, size);

    MIPS_DEBUG("%s %s, %d, %d", "extp", regnames[rt], ac, size);
}

static void gen_EXTPDP(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t size = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extpdp, rt, ac, size);

    MIPS_DEBUG("%s %s, %d, %d", "extpdp", regnames[rt], ac, size);
}

static void gen_EXTPDPV(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extpdpv, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extpdpv", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTPV(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extpv, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extpv", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTR_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t shift = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extr_w, rt, ac, shift);

    MIPS_DEBUG("%s %s, %d, %d", "extr.w", regnames[rt], ac, shift);
}

static void gen_EXTRV_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extrv_w, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extrv.w", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTRV_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extrv_r_w, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extrv_r.w", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTRV_RS_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extrv_rs_w, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extrv_rs.w", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTRV_S_H(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t rs = (ctx->opcode >> 21) & 0x1f ;

    gen_extpv_wrapper(env, ctx, 1, gen_helper_extrv_s_h, rt, ac, rs);

    MIPS_DEBUG("%s %s, %d, %s", "extrv_s.h", regnames[rt], ac, regnames[rs]);
}

static void gen_EXTR_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t shift = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extr_r_w, rt, ac, shift);

    MIPS_DEBUG("%s %s, %d, %d", "extr_r.w", regnames[rt], ac, shift);
}

static void gen_EXTR_RS_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t shift = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extr_rs_w, rt, ac, shift);

    MIPS_DEBUG("%s %s, %d, %d", "extr_rs.w", regnames[rt], ac, shift);
}

static void gen_EXTR_S_H(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;
    uint8_t shift = (ctx->opcode >> 21) & 0x1f ;

    gen_extp_wrapper(env, ctx, 1, gen_helper_extr_s_h, rt, ac, shift);

    MIPS_DEBUG("%s %s, %d, %d", "extr_s.h", regnames[rt], ac, shift);
}

static void gen_MTHLIP(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_mthlip_wrapper(env, ctx, 1, rs, ac);

    MIPS_DEBUG("%s %s, %d", "mthlip", regnames[rs], ac);
}

static void gen_RDDSP(CPUState *env, DisasContext *ctx)
{
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;
    uint16_t mask = (ctx->opcode >> 16) & 0x3ff;

    gen_rdwr_dspctrl(env, ctx, rd, mask, 0);
}

static void gen_SHILO(CPUState *env, DisasContext *ctx)
{

    uint8_t shift = (ctx->opcode >> 20) & 0x3f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_shilo_wrapper(env, ctx, 1, gen_helper_shilo, ac, shift);

    MIPS_DEBUG("%s %d, %d", "shilo", ac, shift);
}

static void gen_SHILOV(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t ac = (ctx->opcode >> 11) & 0x3;

    gen_binary_rs_ac_wrapper(env, ctx, 1, gen_helper_shilov, rs, ac);

    MIPS_DEBUG("%s %d, %s", "shilov", ac, regnames[rs]);
}

static void gen_WRDSP(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint16_t mask = (ctx->opcode >> 11) & 0x3ff;

    gen_rdwr_dspctrl(env, ctx, rs, mask, 1);
}

static void gen_INSV(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;

    gen_binary_rs_rt_ret_wrapper(env, ctx, 1, gen_helper_insv, rs, rt);

    MIPS_DEBUG("%s %s, %s", "insv", regnames[rt], regnames[rs]);
}

static void gen_LBUX(CPUState *env, DisasContext *ctx)
{
    uint8_t base = (ctx->opcode >> 21) & 0x1f;
    uint8_t index = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_lx_wrapper(env, ctx, 1, tcg_gen_qemu_ld8u, rd, index, base,
        ctx->mem_idx);

    MIPS_DEBUG("%s %s, %s(%s)", "lbux", regnames[rd], regnames[index],
        regnames[base]);
}

static void gen_LHX(CPUState *env, DisasContext *ctx)
{
    uint8_t base = (ctx->opcode >> 21) & 0x1f;
    uint8_t index = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_lx_wrapper(env, ctx, 1, tcg_gen_qemu_ld16s, rd, index, base,
        ctx->mem_idx);

    MIPS_DEBUG("%s %s, %s(%s)", "lhx", regnames[rd], regnames[index],
        regnames[base]);
}

static void gen_LWX(CPUState *env, DisasContext *ctx)
{
    uint8_t base = (ctx->opcode >> 21) & 0x1f;
    uint8_t index = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_lx_wrapper(env, ctx, 1, tcg_gen_qemu_ld32s, rd, index, base,
        ctx->mem_idx);

    MIPS_DEBUG("%s %s, %s(%s)", "lwx", regnames[rd], regnames[index],
        regnames[base]);
}

static void gen_SHLL_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shll_ph, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shll.ph", regnames[rd], regnames[rt], sa);
}

static void gen_SHLL_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shll_qb, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shll.qb", regnames[rd], regnames[rt], sa);
}

static void gen_SHLLV_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shllv_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shllv.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHLLV_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shllv_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shllv.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHLLV_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shllv_s_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shllv_s.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHLLV_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shllv_s_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shllv_s.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHLL_S_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shll_s_ph, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shll_s.ph", regnames[rd], regnames[rt], sa);
}

static void gen_SHLL_S_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shll_s_w, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shll_s.w", regnames[rd], regnames[rt], sa);
}

static void gen_SHRA_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shra_ph, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shra.ph", regnames[rd], regnames[rt], sa);
}

static void gen_SHRA_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 2, gen_helper_shra_qb, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shra.qb", regnames[rd], regnames[rt], sa);
}

static void gen_SHRAV_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shrav_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrav.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRAV_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_shrav_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrav.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRAV_R_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shrav_r_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrav_r.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRAV_R_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_shrav_r_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrav_r.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRAV_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shrav_r_w,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrav_r.w", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRA_R_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shra_r_ph, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shra_r.ph", regnames[rd], regnames[rt], sa);
}

static void gen_SHRA_R_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 2, gen_helper_shra_r_qb, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shra_r.qb", regnames[rd], regnames[rt], sa);
}

static void gen_SHRA_R_W(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shra_r_w, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shra_r.w", regnames[rd], regnames[rt], sa);
}

static void gen_SHRL_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 2, gen_helper_shrl_ph, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shrl.ph", regnames[rd], regnames[rt], sa);
}

static void gen_SHRL_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    uint8_t sa = (ctx->opcode >> 21) & 0x1f;

    gen_binary_rt_rd_sa_wrapper(env, ctx, 1, gen_helper_shrl_qb, rt, rd, sa);

    MIPS_DEBUG("%s %s, %s, %d", "shrl.qb", regnames[rd], regnames[rt], sa);
}

static void gen_SHRLV_PH(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 2, gen_helper_shrlv_ph,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrlv.ph", regnames[rd], regnames[rs],
        regnames[rt]);
}

static void gen_SHRLV_QB(CPUState *env, DisasContext *ctx)
{
    uint8_t rs = (ctx->opcode >> 21) & 0x1f;
    uint8_t rt = (ctx->opcode >> 16) & 0x1f;
    uint8_t rd = (ctx->opcode >> 11) & 0x1f;

    gen_binary_rs_rd_rt_wrapper(env, ctx, 1, gen_helper_shrlv_qb,
        rs, rd, rt);

    MIPS_DEBUG("%s %s, %s, %s", "shrlv.qb", regnames[rd], regnames[rs],
        regnames[rt]);
}

