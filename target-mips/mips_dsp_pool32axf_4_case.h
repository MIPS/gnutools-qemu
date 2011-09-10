    case 0x4:
        switch (minor & 0xf) {
        case ABSQ_S_QB:
            gen_mABSQ_S_QB(env, ctx);
            break;
        case ABSQ_S_PH:
            gen_mABSQ_S_PH(env, ctx);
            break;
        case ABSQ_S_W:
            gen_mABSQ_S_W(env, ctx);
            break;
        case BITREV:
            gen_mBITREV(env, ctx);
            break;
        case INSV:
            gen_mINSV(env, ctx);
            break;
        case PRECEQ_W_PHL:
            gen_mPRECEQ_W_PHL(env, ctx);
            break;
        case PRECEQ_W_PHR:
            gen_mPRECEQ_W_PHR(env, ctx);
            break;
        case PRECEQU_PH_QBL:
            gen_mPRECEQU_PH_QBL(env, ctx);
            break;
        case PRECEQU_PH_QBR:
            gen_mPRECEQU_PH_QBR(env, ctx);
            break;
        case PRECEU_PH_QBL:
            gen_mPRECEU_PH_QBL(env, ctx);
            break;
        case PRECEU_PH_QBR:
            gen_mPRECEU_PH_QBR(env, ctx);
            break;
        case RADDU_W_QB:
            gen_mRADDU_W_QB(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0xc:
        switch (minor & 0xf) {
        case REPLV_PH:
            gen_mREPLV_PH(env, ctx);
            break;
        case REPLV_QB:
            gen_mREPLV_QB(env, ctx);
            break;
        case PRECEQU_PH_QBLA:
            gen_mPRECEQU_PH_QBLA(env, ctx);
            break;
        case PRECEQU_PH_QBRA:
            gen_mPRECEQU_PH_QBRA(env, ctx);
            break;
        case PRECEU_PH_QBLA:
            gen_mPRECEU_PH_QBLA(env, ctx);
            break;
        case PRECEU_PH_QBRA:
            gen_mPRECEU_PH_QBRA(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
