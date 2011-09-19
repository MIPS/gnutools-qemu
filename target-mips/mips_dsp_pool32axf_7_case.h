    case 0x7:
        switch (minor & 0x0) {
        case SHRAl__R_r_QB:
            if ((ctx->opcode >> 12) & 1) {
                gen_mSHRA_R_QB(env, ctx);
            } else {
                gen_mSHRA_QB(env, ctx);
            }
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0xf:
        switch (minor & 0x0) {
        case SHRL_PH:
            gen_mSHRL_PH(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
    case 0x17:
        switch (minor & 0x0) {
        case REPL_QB:
            gen_mREPL_QB(env, ctx);
            break;
        default:
            goto pool32axf_invalid;
        }
        break;
