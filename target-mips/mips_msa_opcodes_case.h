static void gen_msa(CPUState *env, DisasContext *ctx)
{
    uint32_t opcode = ctx->opcode;

    /* !!! It must do longest prefix match here! */

    switch (opcode & 0xffff003f) {
        case OPC_MOVE_V:
            gen_move_v(env, ctx);
            return;
        case OPC_CTCMSA:
            gen_ctcmsa(env, ctx);
            return;
        case OPC_CFCMSA:
            gen_cfcmsa(env, ctx);
            return;
    }

    switch (opcode & 0xfffe003f) {
        case OPC_FSQRT_df:
            gen_fsqrt_df(env, ctx);
            return;
        case OPC_FRINT_df:
            gen_frint_df(env, ctx);
            return;
        case OPC_FLOG2_df:
            gen_flog2_df(env, ctx);
            return;
        case OPC_FCLASS_df:
            gen_fclass_df(env, ctx);
            return;
        case OPC_FTINT_S_df:
            gen_ftint_s_df(env, ctx);
            return;
        case OPC_FFINT_S_df:
            gen_ffint_s_df(env, ctx);
            return;
        case OPC_FTINT_U_df:
            gen_ftint_u_df(env, ctx);
            return;
        case OPC_FFINT_U_df:
            gen_ffint_u_df(env, ctx);
            return;
    }

    switch (opcode & 0xfffc003f) {
        case OPC_PCNT_df:
            gen_pcnt_df(env, ctx);
            return;
        case OPC_NLOC_df:
            gen_nloc_df(env, ctx);
            return;
        case OPC_NLZC_df:
            gen_nlzc_df(env, ctx);
            return;
        case OPC_MVFG_df:
            gen_mvfg_df(env, ctx);
            return;
    }

    switch (opcode & 0xffe0003f) {
        case OPC_AND_V:
            gen_and_v(env, ctx);
            return;
        case OPC_OR_V:
            gen_or_v(env, ctx);
            return;
        case OPC_NOR_V:
            gen_nor_v(env, ctx);
            return;
        case OPC_XOR_V:
            gen_xor_v(env, ctx);
            return;
        case OPC_BMNZ_V:
            gen_bmnz_v(env, ctx);
            return;
        case OPC_BMZ_V:
            gen_bmz_v(env, ctx);
            return;
        case OPC_BSEL_V:
            gen_bsel_v(env, ctx);
            return;
        case OPC_BNZ_V:
            gen_bnz_v(env, ctx);
            return;
        case OPC_BZ_V:
            gen_bz_v(env, ctx);
            return;
        case OPC_PCKEV_Q:
            gen_pckev_q(env, ctx);
            return;
        case OPC_PCKOD_Q:
            gen_pckod_q(env, ctx);
            return;
        case OPC_LD_V:
            gen_ld_v(env, ctx);
            return;
        case OPC_LDX_V:
            gen_ldx_v(env, ctx);
            return;
        case OPC_ST_V:
            gen_st_v(env, ctx);
            return;
        case OPC_STX_V:
            gen_stx_v(env, ctx);
            return;
    }

    switch (opcode & 0xffc0003f) {
        case OPC_MUL_Q_df:
            gen_mul_q_df(env, ctx);
            return;
        case OPC_MULR_Q_df:
            gen_mulr_q_df(env, ctx);
            return;
        case OPC_MADD_Q_df:
            gen_madd_q_df(env, ctx);
            return;
        case OPC_MADDR_Q_df:
            gen_maddr_q_df(env, ctx);
            return;
        case OPC_MSUB_Q_df:
            gen_msub_q_df(env, ctx);
            return;
        case OPC_MSUBR_Q_df:
            gen_msubr_q_df(env, ctx);
            return;
        case OPC_SLD_df:
            gen_sld_df(env, ctx);
            return;
        case OPC_MOVE_df:
            gen_move_df(env, ctx);
            return;
        case OPC_MVTG_S_df:
            gen_mvtg_s_df(env, ctx);
            return;
        case OPC_MVTG_U_df:
            gen_mvtg_u_df(env, ctx);
            return;
        case OPC_FADD_df:
            gen_fadd_df(env, ctx);
            return;
        case OPC_FSUB_df:
            gen_fsub_df(env, ctx);
            return;
        case OPC_FMUL_df:
            gen_fmul_df(env, ctx);
            return;
        case OPC_FDIV_df:
            gen_fdiv_df(env, ctx);
            return;
        case OPC_FREM_df:
            gen_frem_df(env, ctx);
            return;
        case OPC_FMADD_df:
            gen_fmadd_df(env, ctx);
            return;
        case OPC_FMSUB_df:
            gen_fmsub_df(env, ctx);
            return;
        case OPC_FEXP2_df:
            gen_fexp2_df(env, ctx);
            return;
        case OPC_FMAX_df:
            gen_fmax_df(env, ctx);
            return;
        case OPC_FMAX_A_df:
            gen_fmax_a_df(env, ctx);
            return;
        case OPC_FMIN_df:
            gen_fmin_df(env, ctx);
            return;
        case OPC_FMIN_A_df:
            gen_fmin_a_df(env, ctx);
            return;
        case OPC_FCEQ_df:
            gen_fceq_df(env, ctx);
            return;
        case OPC_FCLT_df:
            gen_fclt_df(env, ctx);
            return;
        case OPC_FCLE_df:
            gen_fcle_df(env, ctx);
            return;
        case OPC_FCUN_df:
            gen_fcun_df(env, ctx);
            return;
        case OPC_FCEQU_df:
            gen_fcequ_df(env, ctx);
            return;
        case OPC_FCLTU_df:
            gen_fcltu_df(env, ctx);
            return;
        case OPC_FCLEU_df:
            gen_fcleu_df(env, ctx);
            return;
        case OPC_FTQ_df:
            gen_ftq_df(env, ctx);
            return;
        case OPC_FFQ_df:
            gen_ffq_df(env, ctx);
            return;
        case OPC_FEXDO_df:
            gen_fexdo_df(env, ctx);
            return;
        case OPC_FEXUP_df:
            gen_fexup_df(env, ctx);
            return;
    }

    switch (opcode & 0xff80003f) {
        case OPC_ADDV_df:
            gen_addv_df(env, ctx);
            return;
        case OPC_ADDVI_df:
            gen_addvi_df(env, ctx);
            return;
        case OPC_ADD_A_df:
            gen_add_a_df(env, ctx);
            return;
        case OPC_ADDI_A_df:
            gen_addi_a_df(env, ctx);
            return;
        case OPC_ADDS_A_df:
            gen_adds_a_df(env, ctx);
            return;
        case OPC_ADDSI_A_df:
            gen_addsi_a_df(env, ctx);
            return;
        case OPC_ADDS_S_df:
            gen_adds_s_df(env, ctx);
            return;
        case OPC_ADDSI_S_df:
            gen_addsi_s_df(env, ctx);
            return;
        case OPC_ADDS_U_df:
            gen_adds_u_df(env, ctx);
            return;
        case OPC_ADDSI_U_df:
            gen_addsi_u_df(env, ctx);
            return;
        case OPC_SUBV_df:
            gen_subv_df(env, ctx);
            return;
        case OPC_SUBVI_df:
            gen_subvi_df(env, ctx);
            return;
        case OPC_ASUB_S_df:
            gen_asub_s_df(env, ctx);
            return;
        case OPC_ASUBI_S_df:
            gen_asubi_s_df(env, ctx);
            return;
        case OPC_ASUB_U_df:
            gen_asub_u_df(env, ctx);
            return;
        case OPC_ASUBI_U_df:
            gen_asubi_u_df(env, ctx);
            return;
        case OPC_SUBS_S_df:
            gen_subs_s_df(env, ctx);
            return;
        case OPC_SUBSI_S_df:
            gen_subsi_s_df(env, ctx);
            return;
        case OPC_SUBS_U_df:
            gen_subs_u_df(env, ctx);
            return;
        case OPC_SUBSI_U_df:
            gen_subsi_u_df(env, ctx);
            return;
        case OPC_SUBSS_U_df:
            gen_subss_u_df(env, ctx);
            return;
        case OPC_SUBSSI_U_df:
            gen_subssi_u_df(env, ctx);
            return;
        case OPC_MAX_A_df:
            gen_max_a_df(env, ctx);
            return;
        case OPC_MAXI_A_df:
            gen_maxi_a_df(env, ctx);
            return;
        case OPC_MAX_S_df:
            gen_max_s_df(env, ctx);
            return;
        case OPC_MAXI_S_df:
            gen_maxi_s_df(env, ctx);
            return;
        case OPC_MAX_U_df:
            gen_max_u_df(env, ctx);
            return;
        case OPC_MAXI_U_df:
            gen_maxi_u_df(env, ctx);
            return;
        case OPC_MIN_A_df:
            gen_min_a_df(env, ctx);
            return;
        case OPC_MINI_A_df:
            gen_mini_a_df(env, ctx);
            return;
        case OPC_MIN_S_df:
            gen_min_s_df(env, ctx);
            return;
        case OPC_MINI_S_df:
            gen_mini_s_df(env, ctx);
            return;
        case OPC_MIN_U_df:
            gen_min_u_df(env, ctx);
            return;
        case OPC_MINI_U_df:
            gen_mini_u_df(env, ctx);
            return;
        case OPC_AVE_S_df:
            gen_ave_s_df(env, ctx);
            return;
        case OPC_AVEI_S_df:
            gen_avei_s_df(env, ctx);
            return;
        case OPC_AVE_U_df:
            gen_ave_u_df(env, ctx);
            return;
        case OPC_AVEI_U_df:
            gen_avei_u_df(env, ctx);
            return;
        case OPC_SAT_S_df:
            gen_sat_s_df(env, ctx);
            return;
        case OPC_SAT_U_df:
            gen_sat_u_df(env, ctx);
            return;
        case OPC_MULV_df:
            gen_mulv_df(env, ctx);
            return;
        case OPC_MULVI_df:
            gen_mulvi_df(env, ctx);
            return;
        case OPC_MADDV_df:
            gen_maddv_df(env, ctx);
            return;
        case OPC_MADDVI_df:
            gen_maddvi_df(env, ctx);
            return;
        case OPC_MSUBV_df:
            gen_msubv_df(env, ctx);
            return;
        case OPC_MSUBVI_df:
            gen_msubvi_df(env, ctx);
            return;
        case OPC_DOTP_S_df:
            gen_dotp_s_df(env, ctx);
            return;
        case OPC_DOTPI_S_df:
            gen_dotpi_s_df(env, ctx);
            return;
        case OPC_DOTP_U_df:
            gen_dotp_u_df(env, ctx);
            return;
        case OPC_DOTPI_U_df:
            gen_dotpi_u_df(env, ctx);
            return;
        case OPC_DPADD_S_df:
            gen_dpadd_s_df(env, ctx);
            return;
        case OPC_DPADDI_S_df:
            gen_dpaddi_s_df(env, ctx);
            return;
        case OPC_DPADD_U_df:
            gen_dpadd_u_df(env, ctx);
            return;
        case OPC_DPADDI_U_df:
            gen_dpaddi_u_df(env, ctx);
            return;
        case OPC_DPSUB_S_df:
            gen_dpsub_s_df(env, ctx);
            return;
        case OPC_DPSUBI_S_df:
            gen_dpsubi_s_df(env, ctx);
            return;
        case OPC_DPSUB_U_df:
            gen_dpsub_u_df(env, ctx);
            return;
        case OPC_DPSUBI_U_df:
            gen_dpsubi_u_df(env, ctx);
            return;
        case OPC_SHL_df:
            gen_shl_df(env, ctx);
            return;
        case OPC_SHLI_df:
            gen_shli_df(env, ctx);
            return;
        case OPC_SRA_df:
            gen_sra_df(env, ctx);
            return;
        case OPC_SRAI_df:
            gen_srai_df(env, ctx);
            return;
        case OPC_SRL_df:
            gen_srl_df(env, ctx);
            return;
        case OPC_SRLI_df:
            gen_srli_df(env, ctx);
            return;
        case OPC_BINSL_df:
            gen_binsl_df(env, ctx);
            return;
        case OPC_BINSLI_df:
            gen_binsli_df(env, ctx);
            return;
        case OPC_BINSR_df:
            gen_binsr_df(env, ctx);
            return;
        case OPC_BINSRI_df:
            gen_binsri_df(env, ctx);
            return;
        case OPC_BCLR_df:
            gen_bclr_df(env, ctx);
            return;
        case OPC_BCLRI_df:
            gen_bclri_df(env, ctx);
            return;
        case OPC_BSET_df:
            gen_bset_df(env, ctx);
            return;
        case OPC_BSETI_df:
            gen_bseti_df(env, ctx);
            return;
        case OPC_CEQ_df:
            gen_ceq_df(env, ctx);
            return;
        case OPC_CEQI_df:
            gen_ceqi_df(env, ctx);
            return;
        case OPC_CLT_S_df:
            gen_clt_s_df(env, ctx);
            return;
        case OPC_CLTI_S_df:
            gen_clti_s_df(env, ctx);
            return;
        case OPC_CLE_S_df:
            gen_cle_s_df(env, ctx);
            return;
        case OPC_CLEI_S_df:
            gen_clei_s_df(env, ctx);
            return;
        case OPC_CLT_U_df:
            gen_clt_u_df(env, ctx);
            return;
        case OPC_CLTI_U_df:
            gen_clti_u_df(env, ctx);
            return;
        case OPC_CLE_U_df:
            gen_cle_u_df(env, ctx);
            return;
        case OPC_CLEI_U_df:
            gen_clei_u_df(env, ctx);
            return;
        case OPC_BNZ_df:
            gen_bnz_df(env, ctx);
            return;
        case OPC_BZ_df:
            gen_bz_df(env, ctx);
            return;
        case OPC_VSHF_df:
            gen_vshf_df(env, ctx);
            return;
        case OPC_PCKEV_df:
            gen_pckev_df(env, ctx);
            return;
        case OPC_PCKOD_df:
            gen_pckod_df(env, ctx);
            return;
        case OPC_ILVL_df:
            gen_ilvl_df(env, ctx);
            return;
        case OPC_ILVR_df:
            gen_ilvr_df(env, ctx);
            return;
        case OPC_ILVEV_df:
            gen_ilvev_df(env, ctx);
            return;
        case OPC_ILVOD_df:
            gen_ilvod_df(env, ctx);
            return;
        case OPC_LDI_df:
            gen_ldi_df(env, ctx);
            return;
    }

    switch (opcode & 0xff00003f) {
        case OPC_ANDI_B:
            gen_andi_b(env, ctx);
            return;
        case OPC_ORI_B:
            gen_ori_b(env, ctx);
            return;
        case OPC_NORI_B:
            gen_nori_b(env, ctx);
            return;
        case OPC_XORI_B:
            gen_xori_b(env, ctx);
            return;
        case OPC_BMNZI_B:
            gen_bmnzi_b(env, ctx);
            return;
        case OPC_BMZI_B:
            gen_bmzi_b(env, ctx);
            return;
        case OPC_BSELI_B:
            gen_bseli_b(env, ctx);
            return;
        case OPC_SHF_B:
            gen_shf_b(env, ctx);
            return;
        case OPC_SHF_H:
            gen_shf_h(env, ctx);
            return;
        case OPC_SHF_W:
            gen_shf_w(env, ctx);
            return;
        case OPC_SHF_D:
            gen_shf_d(env, ctx);
            return;
    }

    MIPS_INVAL("MSA instruction");
    generate_exception(ctx, EXCP_RI);
} // end of gen_msa()
