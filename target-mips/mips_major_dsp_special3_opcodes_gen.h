static void gen_ABSQ_S_PH_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_ABSQ_S_PH:
        gen_ABSQ_S_PH(env, ctx);
        break;
    case OPC_ABSQ_S_QB:
        gen_ABSQ_S_QB(env, ctx);
        break;
    case OPC_ABSQ_S_W:
        gen_ABSQ_S_W(env, ctx);
        break;
    case OPC_BITREV:
        gen_BITREV(env, ctx);
        break;
    case OPC_PRECEQ_W_PHL:
        gen_PRECEQ_W_PHL(env, ctx);
        break;
    case OPC_PRECEQ_W_PHR:
        gen_PRECEQ_W_PHR(env, ctx);
        break;
    case OPC_PRECEQU_PH_QBL:
        gen_PRECEQU_PH_QBL(env, ctx);
        break;
    case OPC_PRECEQU_PH_QBLA:
        gen_PRECEQU_PH_QBLA(env, ctx);
        break;
    case OPC_PRECEQU_PH_QBR:
        gen_PRECEQU_PH_QBR(env, ctx);
        break;
    case OPC_PRECEQU_PH_QBRA:
        gen_PRECEQU_PH_QBRA(env, ctx);
        break;
    case OPC_PRECEU_PH_QBL:
        gen_PRECEU_PH_QBL(env, ctx);
        break;
    case OPC_PRECEU_PH_QBLA:
        gen_PRECEU_PH_QBLA(env, ctx);
        break;
    case OPC_PRECEU_PH_QBR:
        gen_PRECEU_PH_QBR(env, ctx);
        break;
    case OPC_PRECEU_PH_QBRA:
        gen_PRECEU_PH_QBRA(env, ctx);
        break;
    case OPC_REPL_PH:
        gen_REPL_PH(env, ctx);
        break;
    case OPC_REPL_QB:
        gen_REPL_QB(env, ctx);
        break;
    case OPC_REPLV_PH:
        gen_REPLV_PH(env, ctx);
        break;
    case OPC_REPLV_QB:
        gen_REPLV_QB(env, ctx);
        break;
    }
}

static void gen_ADDU_QB_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_ADDQ_PH:
        gen_ADDQ_PH(env, ctx);
        break;
    case OPC_ADDQ_S_PH:
        gen_ADDQ_S_PH(env, ctx);
        break;
    case OPC_ADDQ_S_W:
        gen_ADDQ_S_W(env, ctx);
        break;
    case OPC_ADDSC:
        gen_ADDSC(env, ctx);
        break;
    case OPC_ADDU_PH:
        gen_ADDU_PH(env, ctx);
        break;
    case OPC_ADDU_QB:
        gen_ADDU_QB(env, ctx);
        break;
    case OPC_ADDU_S_PH:
        gen_ADDU_S_PH(env, ctx);
        break;
    case OPC_ADDU_S_QB:
        gen_ADDU_S_QB(env, ctx);
        break;
    case OPC_ADDWC:
        gen_ADDWC(env, ctx);
        break;
    case OPC_MODSUB:
        gen_MODSUB(env, ctx);
        break;
    case OPC_MULEQ_S_W_PHL:
        gen_MULEQ_S_W_PHL(env, ctx);
        break;
    case OPC_MULEQ_S_W_PHR:
        gen_MULEQ_S_W_PHR(env, ctx);
        break;
    case OPC_MULEU_S_PH_QBL:
        gen_MULEU_S_PH_QBL(env, ctx);
        break;
    case OPC_MULEU_S_PH_QBR:
        gen_MULEU_S_PH_QBR(env, ctx);
        break;
    case OPC_MULQ_RS_PH:
        gen_MULQ_RS_PH(env, ctx);
        break;
    case OPC_MULQ_S_PH:
        gen_MULQ_S_PH(env, ctx);
        break;
    case OPC_RADDU_W_QB:
        gen_RADDU_W_QB(env, ctx);
        break;
    case OPC_SUBQ_PH:
        gen_SUBQ_PH(env, ctx);
        break;
    case OPC_SUBQ_S_PH:
        gen_SUBQ_S_PH(env, ctx);
        break;
    case OPC_SUBQ_S_W:
        gen_SUBQ_S_W(env, ctx);
        break;
    case OPC_SUBU_PH:
        gen_SUBU_PH(env, ctx);
        break;
    case OPC_SUBU_QB:
        gen_SUBU_QB(env, ctx);
        break;
    case OPC_SUBU_S_PH:
        gen_SUBU_S_PH(env, ctx);
        break;
    case OPC_SUBU_S_QB:
        gen_SUBU_S_QB(env, ctx);
        break;
    }
}

static void gen_ADDUH_QB_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_ADDQH_PH:
        gen_ADDQH_PH(env, ctx);
        break;
    case OPC_ADDQH_W:
        gen_ADDQH_W(env, ctx);
        break;
    case OPC_ADDQH_R_PH:
        gen_ADDQH_R_PH(env, ctx);
        break;
    case OPC_ADDQH_R_W:
        gen_ADDQH_R_W(env, ctx);
        break;
    case OPC_ADDUH_QB:
        gen_ADDUH_QB(env, ctx);
        break;
    case OPC_ADDUH_R_QB:
        gen_ADDUH_R_QB(env, ctx);
        break;
    case OPC_MUL_PH:
        gen_MUL_PH(env, ctx);
        break;
    case OPC_MULQ_RS_W:
        gen_MULQ_RS_W(env, ctx);
        break;
    case OPC_MULQ_S_W:
        gen_MULQ_S_W(env, ctx);
        break;
    case OPC_MUL_S_PH:
        gen_MUL_S_PH(env, ctx);
        break;
    case OPC_SUBQH_PH:
        gen_SUBQH_PH(env, ctx);
        break;
    case OPC_SUBQH_W:
        gen_SUBQH_W(env, ctx);
        break;
    case OPC_SUBQH_R_PH:
        gen_SUBQH_R_PH(env, ctx);
        break;
    case OPC_SUBQH_R_W:
        gen_SUBQH_R_W(env, ctx);
        break;
    case OPC_SUBUH_QB:
        gen_SUBUH_QB(env, ctx);
        break;
    case OPC_SUBUH_R_QB:
        gen_SUBUH_R_QB(env, ctx);
        break;
    }
}

static void gen_APPEND_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_APPEND:
        gen_APPEND(env, ctx);
        break;
    case OPC_BALIGN:
        gen_BALIGN(env, ctx);
        break;
    case OPC_PREPEND:
        gen_PREPEND(env, ctx);
        break;
    }
}

static void gen_CMPU_EQ_QB_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_CMP_EQ_PH:
        gen_CMP_EQ_PH(env, ctx);
        break;
    case OPC_CMP_LE_PH:
        gen_CMP_LE_PH(env, ctx);
        break;
    case OPC_CMP_LT_PH:
        gen_CMP_LT_PH(env, ctx);
        break;
    case OPC_CMPGDU_EQ_QB:
        gen_CMPGDU_EQ_QB(env, ctx);
        break;
    case OPC_CMPGDU_LE_QB:
        gen_CMPGDU_LE_QB(env, ctx);
        break;
    case OPC_CMPGDU_LT_QB:
        gen_CMPGDU_LT_QB(env, ctx);
        break;
    case OPC_CMPGU_EQ_QB:
        gen_CMPGU_EQ_QB(env, ctx);
        break;
    case OPC_CMPGU_LE_QB:
        gen_CMPGU_LE_QB(env, ctx);
        break;
    case OPC_CMPGU_LT_QB:
        gen_CMPGU_LT_QB(env, ctx);
        break;
    case OPC_CMPU_EQ_QB:
        gen_CMPU_EQ_QB(env, ctx);
        break;
    case OPC_CMPU_LE_QB:
        gen_CMPU_LE_QB(env, ctx);
        break;
    case OPC_CMPU_LT_QB:
        gen_CMPU_LT_QB(env, ctx);
        break;
    case OPC_PACKRL_PH:
        gen_PACKRL_PH(env, ctx);
        break;
    case OPC_PICK_PH:
        gen_PICK_PH(env, ctx);
        break;
    case OPC_PICK_QB:
        gen_PICK_QB(env, ctx);
        break;
    case OPC_PRECR_QB_PH:
        gen_PRECR_QB_PH(env, ctx);
        break;
    case OPC_PRECRQ_PH_W:
        gen_PRECRQ_PH_W(env, ctx);
        break;
    case OPC_PRECRQ_QB_PH:
        gen_PRECRQ_QB_PH(env, ctx);
        break;
    case OPC_PRECRQU_S_QB_PH:
        gen_PRECRQU_S_QB_PH(env, ctx);
        break;
    case OPC_PRECRQ_RS_PH_W:
        gen_PRECRQ_RS_PH_W(env, ctx);
        break;
    case OPC_PRECR_SRA_PH_W:
        gen_PRECR_SRA_PH_W(env, ctx);
        break;
    case OPC_PRECR_SRA_R_PH_W:
        gen_PRECR_SRA_R_PH_W(env, ctx);
        break;
    }
}

static void gen_DPAQ_W_PH_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_DPA_W_PH:
        gen_DPA_W_PH(env, ctx);
        break;
    case OPC_DPAQX_S_W_PH:
        gen_DPAQX_S_W_PH(env, ctx);
        break;
    case OPC_DPAQX_SA_W_PH:
        gen_DPAQX_SA_W_PH(env, ctx);
        break;
    case OPC_DPAQ_S_W_PH:
        gen_DPAQ_S_W_PH(env, ctx);
        break;
    case OPC_DPAQ_SA_L_W:
        gen_DPAQ_SA_L_W(env, ctx);
        break;
    case OPC_DPAU_H_QBL:
        gen_DPAU_H_QBL(env, ctx);
        break;
    case OPC_DPAU_H_QBR:
        gen_DPAU_H_QBR(env, ctx);
        break;
    case OPC_DPAX_W_PH:
        gen_DPAX_W_PH(env, ctx);
        break;
    case OPC_DPS_W_PH:
        gen_DPS_W_PH(env, ctx);
        break;
    case OPC_DPSQX_S_W_PH:
        gen_DPSQX_S_W_PH(env, ctx);
        break;
    case OPC_DPSQX_SA_W_PH:
        gen_DPSQX_SA_W_PH(env, ctx);
        break;
    case OPC_DPSQ_S_W_PH:
        gen_DPSQ_S_W_PH(env, ctx);
        break;
    case OPC_DPSQ_SA_L_W:
        gen_DPSQ_SA_L_W(env, ctx);
        break;
    case OPC_DPSU_H_QBL:
        gen_DPSU_H_QBL(env, ctx);
        break;
    case OPC_DPSU_H_QBR:
        gen_DPSU_H_QBR(env, ctx);
        break;
    case OPC_DPSX_W_PH:
        gen_DPSX_W_PH(env, ctx);
        break;
    case OPC_MAQ_S_W_PHL:
        gen_MAQ_S_W_PHL(env, ctx);
        break;
    case OPC_MAQ_S_W_PHR:
        gen_MAQ_S_W_PHR(env, ctx);
        break;
    case OPC_MAQ_SA_W_PHL:
        gen_MAQ_SA_W_PHL(env, ctx);
        break;
    case OPC_MAQ_SA_W_PHR:
        gen_MAQ_SA_W_PHR(env, ctx);
        break;
    case OPC_MULSA_W_PH:
        gen_MULSA_W_PH(env, ctx);
        break;
    case OPC_MULSAQ_S_W_PH:
        gen_MULSAQ_S_W_PH(env, ctx);
        break;
    }
}

static void gen_EXTR_W_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_EXTP:
        gen_EXTP(env, ctx);
        break;
    case OPC_EXTPDP:
        gen_EXTPDP(env, ctx);
        break;
    case OPC_EXTPDPV:
        gen_EXTPDPV(env, ctx);
        break;
    case OPC_EXTPV:
        gen_EXTPV(env, ctx);
        break;
    case OPC_EXTR_W:
        gen_EXTR_W(env, ctx);
        break;
    case OPC_EXTRV_W:
        gen_EXTRV_W(env, ctx);
        break;
    case OPC_EXTRV_R_W:
        gen_EXTRV_R_W(env, ctx);
        break;
    case OPC_EXTRV_RS_W:
        gen_EXTRV_RS_W(env, ctx);
        break;
    case OPC_EXTRV_S_H:
        gen_EXTRV_S_H(env, ctx);
        break;
    case OPC_EXTR_R_W:
        gen_EXTR_R_W(env, ctx);
        break;
    case OPC_EXTR_RS_W:
        gen_EXTR_RS_W(env, ctx);
        break;
    case OPC_EXTR_S_H:
        gen_EXTR_S_H(env, ctx);
        break;
    case OPC_MTHLIP:
        gen_MTHLIP(env, ctx);
        break;
    case OPC_RDDSP:
        gen_RDDSP(env, ctx);
        break;
    case OPC_SHILO:
        gen_SHILO(env, ctx);
        break;
    case OPC_SHILOV:
        gen_SHILOV(env, ctx);
        break;
    case OPC_WRDSP:
        gen_WRDSP(env, ctx);
        break;
    }
}

static void gen_INSV_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_INSV:
        gen_INSV(env, ctx);
        break;
    }
}

static void gen_LX_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_LBUX:
        gen_LBUX(env, ctx);
        break;
    case OPC_LHX:
        gen_LHX(env, ctx);
        break;
    case OPC_LWX:
        gen_LWX(env, ctx);
        break;
    }
}

static void gen_SHLL_QB_major(CPUState *env, DisasContext *ctx)
{
    int opc = ctx->opcode & 0xfc0007ff;
    switch (opc) {
    case OPC_SHLL_PH:
        gen_SHLL_PH(env, ctx);
        break;
    case OPC_SHLL_QB:
        gen_SHLL_QB(env, ctx);
        break;
    case OPC_SHLLV_PH:
        gen_SHLLV_PH(env, ctx);
        break;
    case OPC_SHLLV_QB:
        gen_SHLLV_QB(env, ctx);
        break;
    case OPC_SHLLV_S_PH:
        gen_SHLLV_S_PH(env, ctx);
        break;
    case OPC_SHLLV_S_W:
        gen_SHLLV_S_W(env, ctx);
        break;
    case OPC_SHLL_S_PH:
        gen_SHLL_S_PH(env, ctx);
        break;
    case OPC_SHLL_S_W:
        gen_SHLL_S_W(env, ctx);
        break;
    case OPC_SHRA_PH:
        gen_SHRA_PH(env, ctx);
        break;
    case OPC_SHRA_QB:
        gen_SHRA_QB(env, ctx);
        break;
    case OPC_SHRAV_PH:
        gen_SHRAV_PH(env, ctx);
        break;
    case OPC_SHRAV_QB:
        gen_SHRAV_QB(env, ctx);
        break;
    case OPC_SHRAV_R_PH:
        gen_SHRAV_R_PH(env, ctx);
        break;
    case OPC_SHRAV_R_QB:
        gen_SHRAV_R_QB(env, ctx);
        break;
    case OPC_SHRAV_R_W:
        gen_SHRAV_R_W(env, ctx);
        break;
    case OPC_SHRA_R_PH:
        gen_SHRA_R_PH(env, ctx);
        break;
    case OPC_SHRA_R_QB:
        gen_SHRA_R_QB(env, ctx);
        break;
    case OPC_SHRA_R_W:
        gen_SHRA_R_W(env, ctx);
        break;
    case OPC_SHRL_PH:
        gen_SHRL_PH(env, ctx);
        break;
    case OPC_SHRL_QB:
        gen_SHRL_QB(env, ctx);
        break;
    case OPC_SHRLV_PH:
        gen_SHRLV_PH(env, ctx);
        break;
    case OPC_SHRLV_QB:
        gen_SHRLV_QB(env, ctx);
        break;
    }
}

