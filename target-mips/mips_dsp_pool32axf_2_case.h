    case 0x2:
        switch (minor & 0x3) {
        case DPA_W_PH:
            gen_mDPA_W_PH(env, ctx);
            break;
        case DPAX_W_PH:
            gen_mDPAX_W_PH(env, ctx);
            break;
        case DPAU_H_QBL:
            gen_mDPAU_H_QBL(env, ctx);
            break;
        case DPAU_H_QBR:
            gen_mDPAU_H_QBR(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0xa:
        switch (minor & 0x3) {
        case DPAQ_S_W_PH:
            gen_mDPAQ_S_W_PH(env, ctx);
            break;
        case DPAQ_SA_L_W:
            gen_mDPAQ_SA_L_W(env, ctx);
            break;
        case DPAQX_S_W_PH:
            gen_mDPAQX_S_W_PH(env, ctx);
            break;
        case DPAQX_SA_W_PH:
            gen_mDPAQX_SA_W_PH(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x12:
        switch (minor & 0x3) {
        case DPS_W_PH:
            gen_mDPS_W_PH(env, ctx);
            break;
        case DPSX_W_PH:
            gen_mDPSX_W_PH(env, ctx);
            break;
        case DPSU_H_QBL:
            gen_mDPSU_H_QBL(env, ctx);
            break;
        case DPSU_H_QBR:
            gen_mDPSU_H_QBR(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x1a:
        switch (minor & 0x3) {
        case DPSQ_S_W_PH:
            gen_mDPSQ_S_W_PH(env, ctx);
            break;
        case DPSQ_SA_L_W:
            gen_mDPSQ_SA_L_W(env, ctx);
            break;
        case DPSQX_S_W_PH:
            gen_mDPSQX_S_W_PH(env, ctx);
            break;
        case DPSQX_SA_W_PH:
            gen_mDPSQX_SA_W_PH(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x22:
        switch (minor & 0x3) {
        case BALIGN:
            gen_mBALIGN(env, ctx);
            break;
        case EXTPV:
            gen_mEXTPV(env, ctx);
            break;
        case EXTPDPV:
            gen_mEXTPDPV(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x3a:
        switch (minor & 0x3) {
        case EXTRV_W:
            gen_mEXTRV_W(env, ctx);
            break;
        case EXTRV_R_W:
            gen_mEXTRV_R_W(env, ctx);
            break;
        case EXTRV_RS_W:
            gen_mEXTRV_RS_W(env, ctx);
            break;
        case EXTRV_S_H:
            gen_mEXTRV_S_H(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
