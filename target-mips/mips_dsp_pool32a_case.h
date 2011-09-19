        case 0x5:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case CMP_EQ_PH:
                gen_mCMP_EQ_PH(env, ctx);
                break;
            case CMP_LT_PH:
                gen_mCMP_LT_PH(env, ctx);
                break;
            case CMP_LE_PH:
                gen_mCMP_LE_PH(env, ctx);
                break;
            case CMPGU_EQ_QB:
                gen_mCMPGU_EQ_QB(env, ctx);
                break;
            case CMPGU_LT_QB:
                gen_mCMPGU_LT_QB(env, ctx);
                break;
            case CMPGU_LE_QB:
                gen_mCMPGU_LE_QB(env, ctx);
                break;
            case CMPGDU_EQ_QB:
                gen_mCMPGDU_EQ_QB(env, ctx);
                break;
            case CMPGDU_LT_QB:
                gen_mCMPGDU_LT_QB(env, ctx);
                break;
            case CMPGDU_LE_QB:
                gen_mCMPGDU_LE_QB(env, ctx);
                break;
            case CMPU_EQ_QB:
                gen_mCMPU_EQ_QB(env, ctx);
                break;
            case CMPU_LT_QB:
                gen_mCMPU_LT_QB(env, ctx);
                break;
            case CMPU_LE_QB:
                gen_mCMPU_LE_QB(env, ctx);
                break;
            case ADDQ_S_W:
                gen_mADDQ_S_W(env, ctx);
                break;
            case SUBQ_S_W:
                gen_mSUBQ_S_W(env, ctx);
                break;
            case ADDSC:
                gen_mADDSC(env, ctx);
                break;
            case ADDWC:
                gen_mADDWC(env, ctx);
                break;
            }
            break;
        case 0xd:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case ADDQl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDQ_S_PH(env, ctx);
                } else {
                    gen_mADDQ_PH(env, ctx);
                }
                break;
            case ADDQHl__R_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDQH_R_PH(env, ctx);
                } else {
                    gen_mADDQH_PH(env, ctx);
                }
                break;
            case ADDQHl__R_r_W:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDQH_R_W(env, ctx);
                } else {
                    gen_mADDQH_W(env, ctx);
                }
                break;
            case ADDUl__S_r_QB:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDU_S_QB(env, ctx);
                } else {
                    gen_mADDU_QB(env, ctx);
                }
                break;
            case ADDUl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDU_S_PH(env, ctx);
                } else {
                    gen_mADDU_PH(env, ctx);
                }
                break;
            case ADDUHl__R_r_QB:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mADDUH_R_QB(env, ctx);
                } else {
                    gen_mADDUH_QB(env, ctx);
                }
                break;
            case SHRAVl__R_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSHRAV_R_PH(env, ctx);
                } else {
                    gen_mSHRAV_PH(env, ctx);
                }
                break;
            case SHRAVl__R_r_QB:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSHRAV_R_QB(env, ctx);
                } else {
                    gen_mSHRAV_QB(env, ctx);
                }
                break;
            case SUBQl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBQ_S_PH(env, ctx);
                } else {
                    gen_mSUBQ_PH(env, ctx);
                }
                break;
            case SUBQHl__R_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBQH_R_PH(env, ctx);
                } else {
                    gen_mSUBQH_PH(env, ctx);
                }
                break;
            case SUBQHl__R_r_W:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBQH_R_W(env, ctx);
                } else {
                    gen_mSUBQH_W(env, ctx);
                }
                break;
            case SUBUl__S_r_QB:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBU_S_QB(env, ctx);
                } else {
                    gen_mSUBU_QB(env, ctx);
                }
                break;
            case SUBUl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBU_S_PH(env, ctx);
                } else {
                    gen_mSUBU_PH(env, ctx);
                }
                break;
            case SUBUHl__R_r_QB:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSUBUH_R_QB(env, ctx);
                } else {
                    gen_mSUBUH_QB(env, ctx);
                }
                break;
            case SHLLVl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSHLLV_S_PH(env, ctx);
                } else {
                    gen_mSHLLV_PH(env, ctx);
                }
                break;
            case PRECR_SRAl__R_r_PH_W:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mPRECR_SRA_R_PH_W(env, ctx);
                } else {
                    gen_mPRECR_SRA_PH_W(env, ctx);
                }
                break;
            }
            break;
        case 0x15:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case MULEU_S_PH_QBL:
                gen_mMULEU_S_PH_QBL(env, ctx);
                break;
            case MULEU_S_PH_QBR:
                gen_mMULEU_S_PH_QBR(env, ctx);
                break;
            case MULQ_RS_PH:
                gen_mMULQ_RS_PH(env, ctx);
                break;
            case MULQ_S_PH:
                gen_mMULQ_S_PH(env, ctx);
                break;
            case MULQ_RS_W:
                gen_mMULQ_RS_W(env, ctx);
                break;
            case MULQ_S_W:
                gen_mMULQ_S_W(env, ctx);
                break;
            case APPEND:
                gen_mAPPEND(env, ctx);
                break;
            case PREPEND:
                gen_mPREPEND(env, ctx);
                break;
            case MODSUB:
                gen_mMODSUB(env, ctx);
                break;
            case SHRAV_R_W:
                gen_mSHRAV_R_W(env, ctx);
                break;
            case SHRLV_PH:
                gen_mSHRLV_PH(env, ctx);
                break;
            case SHRLV_QB:
                gen_mSHRLV_QB(env, ctx);
                break;
            case SHLLV_QB:
                gen_mSHLLV_QB(env, ctx);
                break;
            case SHLLV_S_W:
                gen_mSHLLV_S_W(env, ctx);
                break;
            }
            break;
        case 0x1d:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case SHILO:
                gen_mSHILO(env, ctx);
                break;
            }
            break;
        case 0x25:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case MULEQ_S_W_PHL:
                gen_mMULEQ_S_W_PHL(env, ctx);
                break;
            case MULEQ_S_W_PHR:
                gen_mMULEQ_S_W_PHR(env, ctx);
                break;
            case LHX:
                gen_mLHX(env, ctx);
                break;
            case LWX:
                gen_mLWX(env, ctx);
                break;
            case LBUX:
                gen_mLBUX(env, ctx);
                break;
            }
            break;
        case 0x2d:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case MULl__S_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mMUL_S_PH(env, ctx);
                } else {
                    gen_mMUL_PH(env, ctx);
                }
                break;
            case PRECR_QB_PH:
                gen_mPRECR_QB_PH(env, ctx);
                break;
            case PRECRQ_QB_PH:
                gen_mPRECRQ_QB_PH(env, ctx);
                break;
            case PRECRQ_PH_W:
                gen_mPRECRQ_PH_W(env, ctx);
                break;
            case PRECRQ_RS_PH_W:
                gen_mPRECRQ_RS_PH_W(env, ctx);
                break;
            case PRECRQU_S_QB_PH:
                gen_mPRECRQU_S_QB_PH(env, ctx);
                break;
            case PACKRL_PH:
                gen_mPACKRL_PH(env, ctx);
                break;
            case PICK_QB:
                gen_mPICK_QB(env, ctx);
                break;
            case PICK_PH:
                gen_mPICK_PH(env, ctx);
                break;
            }
            break;
        case 0x35:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case SHRA_R_W:
                gen_mSHRA_R_W(env, ctx);
                break;
            case SHRAl__R_r_PH:
                if ((ctx->opcode >> 10) & 1) {
                    gen_mSHRA_R_PH(env, ctx);
                } else {
                    gen_mSHRA_PH(env, ctx);
                }
                break;
            case SHLLl__S_r_PH:
                if ((ctx->opcode >> 11) & 1) {
                    gen_mSHLL_S_PH(env, ctx);
                } else {
                    gen_mSHLL_PH(env, ctx);
                }
                break;
            case SHLL_S_W:
                gen_mSHLL_S_W(env, ctx);
                break;
            }
            break;
        case 0x3d:
            minor = (ctx->opcode >> 6) & 0xf;
            switch (minor) {
            case REPL_PH:
                gen_mREPL_PH(env, ctx);
                break;
            }
            break;
