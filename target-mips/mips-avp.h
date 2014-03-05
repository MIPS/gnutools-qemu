/*
 * mips-avp.h
 */

#ifndef MIPS_AVP_H_
#define MIPS_AVP_H_

#ifdef MIPSSIM_COMPAT

// sysemu.h
extern char *cpu_model_name;
extern char *cpu_config_name;

#endif

#ifdef SV_SUPPORT
void sv_log_init(const char * filename);
#endif

#endif /* MIPS_AVP_H_ */
