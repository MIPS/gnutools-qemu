static void gen_msa(CPUState *env, DisasContext *ctx, int *is_branch)
{
    uint32_t opcode = ctx->opcode;

    /* !!! It must do longest prefix match here! */

    switch (opcode & 0xffff003f) {
        case OPC_CTCMSA:
            gen_ctcmsa(env, ctx);
            return;
        case OPC_CFCMSA:
            gen_cfcmsa(env, ctx);
            return;
        case OPC_MOVE_V:
            gen_move_v(env, ctx);
            return;
    }

    switch (opcode & 0xfffe003f) {
        case OPC_FCLASS_df:
            gen_fclass_df(env, ctx);
            return;
        case OPC_FSQRT_df:
            gen_fsqrt_df(env, ctx);
            return;
        case OPC_FRSQRT_df:
            gen_frsqrt_df(env, ctx);
            return;
        case OPC_FRCP_df:
            gen_frcp_df(env, ctx);
            return;
        case OPC_FRINT_df:
            gen_frint_df(env, ctx);
            return;
        case OPC_FLOG2_df:
            gen_flog2_df(env, ctx);
            return;
        case OPC_FEXUPL_df:
            gen_fexupl_df(env, ctx);
            return;
        case OPC_FEXUPR_df:
            gen_fexupr_df(env, ctx);
            return;
        case OPC_FFQL_df:
            gen_ffql_df(env, ctx);
            return;
        case OPC_FFQR_df:
            gen_ffqr_df(env, ctx);
            return;
        case OPC_FTINT_S_df:
            gen_ftint_s_df(env, ctx);
            return;
        case OPC_FTINT_U_df:
            gen_ftint_u_df(env, ctx);
            return;
        case OPC_FFINT_S_df:
            gen_ffint_s_df(env, ctx);
            return;
        case OPC_FFINT_U_df:
            gen_ffint_u_df(env, ctx);
            return;
        case OPC_FTRUNC_S_df:
            gen_ftrunc_s_df(env, ctx);
            return;
        case OPC_FTRUNC_U_df:
            gen_ftrunc_u_df(env, ctx);
            return;
    }

    switch (opcode & 0xfffc003f) {
        case OPC_FILL_df:
            gen_fill_df(env, ctx);
            return;
        case OPC_PCNT_df:
            gen_pcnt_df(env, ctx);
            return;
        case OPC_NLOC_df:
            gen_nloc_df(env, ctx);
            return;
        case OPC_NLZC_df:
            gen_nlzc_df(env, ctx);
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
            *is_branch = 1;
            return;
        case OPC_BZ_V:
            gen_bz_v(env, ctx);
            *is_branch = 1;
            return;
    }

    switch (opcode & 0xffc0003f) {
        case OPC_SLDI_df:
            gen_sldi_df(env, ctx);
            return;
        case OPC_SPLATI_df:
            gen_splati_df(env, ctx);
            return;
        case OPC_COPY_S_df:
            gen_copy_s_df(env, ctx);
            return;
        case OPC_COPY_U_df:
            gen_copy_u_df(env, ctx);
            return;
        case OPC_INSERT_df:
            gen_insert_df(env, ctx);
            return;
        case OPC_INSVE_df:
            gen_insve_df(env, ctx);
            return;
        case OPC_FCAF_df:
            gen_fcaf_df(env, ctx);
            return;
        case OPC_FCUN_df:
            gen_fcun_df(env, ctx);
            return;
        case OPC_FCEQ_df:
            gen_fceq_df(env, ctx);
            return;
        case OPC_FCUEQ_df:
            gen_fcueq_df(env, ctx);
            return;
        case OPC_FCLT_df:
            gen_fclt_df(env, ctx);
            return;
        case OPC_FCULT_df:
            gen_fcult_df(env, ctx);
            return;
        case OPC_FCLE_df:
            gen_fcle_df(env, ctx);
            return;
        case OPC_FCULE_df:
            gen_fcule_df(env, ctx);
            return;
        case OPC_FSAF_df:
            gen_fsaf_df(env, ctx);
            return;
        case OPC_FSUN_df:
            gen_fsun_df(env, ctx);
            return;
        case OPC_FSEQ_df:
            gen_fseq_df(env, ctx);
            return;
        case OPC_FSUEQ_df:
            gen_fsueq_df(env, ctx);
            return;
        case OPC_FSLT_df:
            gen_fslt_df(env, ctx);
            return;
        case OPC_FSULT_df:
            gen_fsult_df(env, ctx);
            return;
        case OPC_FSLE_df:
            gen_fsle_df(env, ctx);
            return;
        case OPC_FSULE_df:
            gen_fsule_df(env, ctx);
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
        case OPC_FMADD_df:
            gen_fmadd_df(env, ctx);
            return;
        case OPC_FMSUB_df:
            gen_fmsub_df(env, ctx);
            return;
        case OPC_FEXP2_df:
            gen_fexp2_df(env, ctx);
            return;
        case OPC_FEXDO_df:
            gen_fexdo_df(env, ctx);
            return;
        case OPC_FTQ_df:
            gen_ftq_df(env, ctx);
            return;
        case OPC_FMIN_df:
            gen_fmin_df(env, ctx);
            return;
        case OPC_FMIN_A_df:
            gen_fmin_a_df(env, ctx);
            return;
        case OPC_FMAX_df:
            gen_fmax_df(env, ctx);
            return;
        case OPC_FMAX_A_df:
            gen_fmax_a_df(env, ctx);
            return;
        case OPC_FCOR_df:
            gen_fcor_df(env, ctx);
            return;
        case OPC_FCUNE_df:
            gen_fcune_df(env, ctx);
            return;
        case OPC_FCNE_df:
            gen_fcne_df(env, ctx);
            return;
        case OPC_MUL_Q_df:
            gen_mul_q_df(env, ctx);
            return;
        case OPC_MADD_Q_df:
            gen_madd_q_df(env, ctx);
            return;
        case OPC_MSUB_Q_df:
            gen_msub_q_df(env, ctx);
            return;
        case OPC_FSOR_df:
            gen_fsor_df(env, ctx);
            return;
        case OPC_FSUNE_df:
            gen_fsune_df(env, ctx);
            return;
        case OPC_FSNE_df:
            gen_fsne_df(env, ctx);
            return;
        case OPC_MULR_Q_df:
            gen_mulr_q_df(env, ctx);
            return;
        case OPC_MADDR_Q_df:
            gen_maddr_q_df(env, ctx);
            return;
        case OPC_MSUBR_Q_df:
            gen_msubr_q_df(env, ctx);
            return;
    }

    switch (opcode & 0xff80003f) {
        case OPC_SLL_df:
            gen_sll_df(env, ctx);
            return;
        case OPC_SLLI_df:
            gen_slli_df(env, ctx);
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
        case OPC_BNEG_df:
            gen_bneg_df(env, ctx);
            return;
        case OPC_BNEGI_df:
            gen_bnegi_df(env, ctx);
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
        case OPC_ADDV_df:
            gen_addv_df(env, ctx);
            return;
        case OPC_ADDVI_df:
            gen_addvi_df(env, ctx);
            return;
        case OPC_SUBV_df:
            gen_subv_df(env, ctx);
            return;
        case OPC_SUBVI_df:
            gen_subvi_df(env, ctx);
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
        case OPC_MAX_A_df:
            gen_max_a_df(env, ctx);
            return;
        case OPC_MIN_A_df:
            gen_min_a_df(env, ctx);
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
        case OPC_CLT_U_df:
            gen_clt_u_df(env, ctx);
            return;
        case OPC_CLTI_U_df:
            gen_clti_u_df(env, ctx);
            return;
        case OPC_CLE_S_df:
            gen_cle_s_df(env, ctx);
            return;
        case OPC_CLEI_S_df:
            gen_clei_s_df(env, ctx);
            return;
        case OPC_CLE_U_df:
            gen_cle_u_df(env, ctx);
            return;
        case OPC_CLEI_U_df:
            gen_clei_u_df(env, ctx);
            return;
        case OPC_LD_df:
            gen_ld_df(env, ctx);
            return;
        case OPC_ST_df:
            gen_st_df(env, ctx);
            return;
        case OPC_LDX_df:
            gen_ldx_df(env, ctx);
            return;
        case OPC_STX_df:
            gen_stx_df(env, ctx);
            return;
        case OPC_SAT_S_df:
            gen_sat_s_df(env, ctx);
            return;
        case OPC_SAT_U_df:
            gen_sat_u_df(env, ctx);
            return;
        case OPC_ADD_A_df:
            gen_add_a_df(env, ctx);
            return;
        case OPC_ADDS_A_df:
            gen_adds_a_df(env, ctx);
            return;
        case OPC_ADDS_S_df:
            gen_adds_s_df(env, ctx);
            return;
        case OPC_ADDS_U_df:
            gen_adds_u_df(env, ctx);
            return;
        case OPC_AVE_S_df:
            gen_ave_s_df(env, ctx);
            return;
        case OPC_AVE_U_df:
            gen_ave_u_df(env, ctx);
            return;
        case OPC_AVER_S_df:
            gen_aver_s_df(env, ctx);
            return;
        case OPC_AVER_U_df:
            gen_aver_u_df(env, ctx);
            return;
        case OPC_SUBS_S_df:
            gen_subs_s_df(env, ctx);
            return;
        case OPC_SUBS_U_df:
            gen_subs_u_df(env, ctx);
            return;
        case OPC_SUBSUS_U_df:
            gen_subsus_u_df(env, ctx);
            return;
        case OPC_SUBSUU_S_df:
            gen_subsuu_s_df(env, ctx);
            return;
        case OPC_ASUB_S_df:
            gen_asub_s_df(env, ctx);
            return;
        case OPC_ASUB_U_df:
            gen_asub_u_df(env, ctx);
            return;
        case OPC_MULV_df:
            gen_mulv_df(env, ctx);
            return;
        case OPC_MADDV_df:
            gen_maddv_df(env, ctx);
            return;
        case OPC_MSUBV_df:
            gen_msubv_df(env, ctx);
            return;
        case OPC_DIV_S_df:
            gen_div_s_df(env, ctx);
            return;
        case OPC_DIV_U_df:
            gen_div_u_df(env, ctx);
            return;
        case OPC_MOD_S_df:
            gen_mod_s_df(env, ctx);
            return;
        case OPC_MOD_U_df:
            gen_mod_u_df(env, ctx);
            return;
        case OPC_DOTP_S_df:
            gen_dotp_s_df(env, ctx);
            return;
        case OPC_DOTP_U_df:
            gen_dotp_u_df(env, ctx);
            return;
        case OPC_DPADD_S_df:
            gen_dpadd_s_df(env, ctx);
            return;
        case OPC_DPADD_U_df:
            gen_dpadd_u_df(env, ctx);
            return;
        case OPC_DPSUB_S_df:
            gen_dpsub_s_df(env, ctx);
            return;
        case OPC_DPSUB_U_df:
            gen_dpsub_u_df(env, ctx);
            return;
        case OPC_SLD_df:
            gen_sld_df(env, ctx);
            return;
        case OPC_SPLAT_df:
            gen_splat_df(env, ctx);
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
        case OPC_VSHF_df:
            gen_vshf_df(env, ctx);
            return;
        case OPC_SRAR_df:
            gen_srar_df(env, ctx);
            return;
        case OPC_SRARI_df:
            gen_srari_df(env, ctx);
            return;
        case OPC_SRLR_df:
            gen_srlr_df(env, ctx);
            return;
        case OPC_SRLRI_df:
            gen_srlri_df(env, ctx);
            return;
        case OPC_HADD_S_df:
            gen_hadd_s_df(env, ctx);
            return;
        case OPC_HADD_U_df:
            gen_hadd_u_df(env, ctx);
            return;
        case OPC_HSUB_S_df:
            gen_hsub_s_df(env, ctx);
            return;
        case OPC_HSUB_U_df:
            gen_hsub_u_df(env, ctx);
            return;
        case OPC_BNZ_df:
            gen_bnz_df(env, ctx);
            *is_branch = 1;
            return;
        case OPC_BZ_df:
            gen_bz_df(env, ctx);
            *is_branch = 1;
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
    }

    MIPS_INVAL("MSA instruction");
    generate_exception(ctx, EXCP_RI);
} // end of gen_msa()
