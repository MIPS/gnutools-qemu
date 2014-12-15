/*
 * mips-avp.h
 */

#ifndef MIPS_AVP_H_
#define MIPS_AVP_H_

#ifdef MIPSSIM_COMPAT
#define SVLOG_KSU ((env->hflags & MIPS_HFLAG_KSU) ?               \
                   (((env->hflags & MIPS_HFLAG_KSU) == MIPS_HFLAG_SM) ? "Supv" : "User") : "Kern")
#define SVLOG_ERL ((env->CP0_Status >> CP0St_ERL) & 1)
#define SVLOG_CPU (*(char **)env->cpu_model)
#define SVLOG_ISROOT ("Root")
#ifdef CONFIG_USER_ONLY
#define SVLOG_START_LINE() do {                                         \
        sv_log("%s : ", SVLOG_CPU);                                     \
    } while(0)
#else
#define SVLOG_START_LINE() do {                                         \
        sv_log("%s : %s(%s%d) - ", SVLOG_CPU, SVLOG_ISROOT, SVLOG_KSU, SVLOG_ERL); \
            } while(0)
#endif

void sv_log_init(const char * filename);
#endif

#endif /* MIPS_AVP_H_ */
