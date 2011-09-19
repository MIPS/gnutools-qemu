        case OPC_ABSQ_S_PH_major:
            gen_ABSQ_S_PH_major(env, ctx);
            break;
        case OPC_ADDU_QB_major:
            gen_ADDU_QB_major(env, ctx);
            break;
        case OPC_APPEND_major:
            gen_APPEND_major(env, ctx);
            break;
        case OPC_CMPU_EQ_QB_major:
            gen_CMPU_EQ_QB_major(env, ctx);
            break;
        case OPC_DPAQ_W_PH_major:
            gen_DPAQ_W_PH_major(env, ctx);
            break;
        case OPC_EXTR_W_major:
            gen_EXTR_W_major(env, ctx);
            break;
        case OPC_INSV_major:
            gen_INSV_major(env, ctx);
            break;
        case OPC_LX_major:
            gen_LX_major(env, ctx);
            break;
        case OPC_SHLL_QB_major:
            gen_SHLL_QB_major(env, ctx);
            break;
