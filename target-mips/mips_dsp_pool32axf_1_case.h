    case 0x9:
        switch (minor & 0x3) {
        case MTHLIP:
            gen_mMTHLIP(env, ctx);
            break;
        case SHILOV:
            gen_mSHILOV(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x21:
        switch (minor & 0x1) {
        case SHLL_QB:
            gen_mSHLL_QB(env, ctx);
            break;
        case SHRL_QB:
            gen_mSHRL_QB(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x29:
        switch (minor & 0x3) {
        case MAQ_S_W_PHR:
            gen_mMAQ_S_W_PHR(env, ctx);
            break;
        case MAQ_S_W_PHL:
            gen_mMAQ_S_W_PHL(env, ctx);
            break;
        case MAQ_SA_W_PHR:
            gen_mMAQ_SA_W_PHR(env, ctx);
            break;
        case MAQ_SA_W_PHL:
            gen_mMAQ_SA_W_PHL(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x39:
        switch (minor & 0x3) {
        case EXTR_W:
            gen_mEXTR_W(env, ctx);
            break;
        case EXTR_R_W:
            gen_mEXTR_R_W(env, ctx);
            break;
        case EXTR_RS_W:
            gen_mEXTR_RS_W(env, ctx);
            break;
        case EXTR_S_H:
            gen_mEXTR_S_H(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
