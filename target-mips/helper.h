/*
 *  MIPS emulation helpers for qemu.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
 *  Copyright (c) 2011 Reed Kotler/MIPS Technologies
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "def-helper.h"

#if defined(MIPS_AVP) && !defined(CONFIG_USER_ONLY)
DEF_HELPER_0(avp_ok, void)
DEF_HELPER_0(avp_fail, void)
#endif

DEF_HELPER_2(raise_exception_err, void, i32, int)
DEF_HELPER_1(raise_exception, void, i32)

#ifdef TARGET_MIPS64
DEF_HELPER_3(ldl, tl, tl, tl, int)
DEF_HELPER_3(ldr, tl, tl, tl, int)
DEF_HELPER_3(sdl, void, tl, tl, int)
DEF_HELPER_3(sdr, void, tl, tl, int)
#endif
DEF_HELPER_3(lwl, tl, tl, tl, int)
DEF_HELPER_3(lwr, tl, tl, tl, int)
DEF_HELPER_3(swl, void, tl, tl, int)
DEF_HELPER_3(swr, void, tl, tl, int)

#ifndef CONFIG_USER_ONLY
DEF_HELPER_2(ll, tl, tl, int)
DEF_HELPER_3(sc, tl, tl, tl, int)
#ifdef TARGET_MIPS64
DEF_HELPER_2(lld, tl, tl, int)
DEF_HELPER_3(scd, tl, tl, tl, int)
#endif
#endif

DEF_HELPER_FLAGS_1(clo, TCG_CALL_CONST | TCG_CALL_PURE, tl, tl)
DEF_HELPER_FLAGS_1(clz, TCG_CALL_CONST | TCG_CALL_PURE, tl, tl)
#ifdef TARGET_MIPS64
DEF_HELPER_FLAGS_1(dclo, TCG_CALL_CONST | TCG_CALL_PURE, tl, tl)
DEF_HELPER_FLAGS_1(dclz, TCG_CALL_CONST | TCG_CALL_PURE, tl, tl)
DEF_HELPER_2(dmult, void, tl, tl)
DEF_HELPER_2(dmultu, void, tl, tl)
#endif

DEF_HELPER_2(muls, tl, tl, tl)
DEF_HELPER_2(mulsu, tl, tl, tl)
DEF_HELPER_2(macc, tl, tl, tl)
DEF_HELPER_2(maccu, tl, tl, tl)
DEF_HELPER_2(msac, tl, tl, tl)
DEF_HELPER_2(msacu, tl, tl, tl)
DEF_HELPER_2(mulhi, tl, tl, tl)
DEF_HELPER_2(mulhiu, tl, tl, tl)
DEF_HELPER_2(mulshi, tl, tl, tl)
DEF_HELPER_2(mulshiu, tl, tl, tl)
DEF_HELPER_2(macchi, tl, tl, tl)
DEF_HELPER_2(macchiu, tl, tl, tl)
DEF_HELPER_2(msachi, tl, tl, tl)
DEF_HELPER_2(msachiu, tl, tl, tl)

DEF_HELPER_1(absq_s_qb, tl, tl)
DEF_HELPER_1(absq_s_ph, tl, tl)
DEF_HELPER_1(absq_s_w, tl, tl)
DEF_HELPER_2(addq_ph, tl, tl, tl)
DEF_HELPER_2(addq_s_ph, tl, tl, tl)
DEF_HELPER_2(addq_s_w, tl, tl, tl)
DEF_HELPER_2(addqh_ph, tl, tl, tl)
DEF_HELPER_2(addqh_r_ph, tl, tl, tl)
DEF_HELPER_2(addqh_w, tl, tl, tl)
DEF_HELPER_2(addqh_r_w, tl, tl, tl)
DEF_HELPER_2(addsc, tl, tl, tl)
DEF_HELPER_2(addu_ph, tl, tl, tl)
DEF_HELPER_2(addu_s_ph, tl, tl, tl)
DEF_HELPER_2(addu_qb, tl, tl, tl)
DEF_HELPER_2(addu_s_qb, tl, tl, tl)
DEF_HELPER_2(addwc, tl, tl, tl)
DEF_HELPER_2(adduh_qb, tl, tl, tl)
DEF_HELPER_2(adduh_r_qb, tl, tl, tl)
DEF_HELPER_3(append, tl, tl, tl, i32)
DEF_HELPER_3(balign, tl, tl, tl, i32)
DEF_HELPER_1(bitrev, tl, tl)
DEF_HELPER_0(posge32, tl)
DEF_HELPER_2(cmp_eq_ph, void, tl, tl)
DEF_HELPER_2(cmp_lt_ph, void, tl, tl)
DEF_HELPER_2(cmp_le_ph, void, tl, tl)
DEF_HELPER_2(cmpgu_eq_qb, tl, tl, tl)
DEF_HELPER_2(cmpgu_le_qb, tl, tl, tl)
DEF_HELPER_2(cmpgu_lt_qb, tl, tl, tl)
DEF_HELPER_2(cmpgdu_eq_qb, tl, tl, tl)
DEF_HELPER_2(cmpgdu_lt_qb, tl, tl, tl)
DEF_HELPER_2(cmpgdu_le_qb, tl, tl, tl)
DEF_HELPER_2(cmpu_eq_qb, void, tl, tl)
DEF_HELPER_2(cmpu_lt_qb, void, tl, tl)
DEF_HELPER_2(cmpu_le_qb, void, tl, tl)
DEF_HELPER_3(dpa_w_ph, i64, i64, tl, tl)
DEF_HELPER_3(dps_w_ph, i64, i64, tl, tl)
DEF_HELPER_4(dpaq_s_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpsq_s_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpaq_sa_l_w, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpsq_sa_l_w, i64, i32, i64, tl, tl)
DEF_HELPER_3(dpau_h_qbl, i64, i64, tl, tl)
DEF_HELPER_3(dpau_h_qbr, i64, i64, tl, tl)
DEF_HELPER_3(dpsu_h_qbl, i64, i64, tl, tl)
DEF_HELPER_3(dpsu_h_qbr, i64, i64, tl, tl)
DEF_HELPER_3(dpax_w_ph, i64, i64, tl, tl)
DEF_HELPER_3(dpsx_w_ph, i64, i64, tl, tl)
DEF_HELPER_4(dpaqx_s_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpaqx_sa_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpsqx_sa_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_4(dpsqx_s_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_2(extp, tl, i64, i32)
DEF_HELPER_2(extpdp, tl, i64, i32)
DEF_HELPER_2(extpv, tl, i64, tl)
DEF_HELPER_2(extpdpv, tl, i64, tl)
DEF_HELPER_2(extr_s_h, tl, i64, i32)
DEF_HELPER_2(extrv_s_h, tl, i64, tl)
DEF_HELPER_2(extr_w, tl, i64, i32)
DEF_HELPER_2(extr_r_w, tl, i64, i32)
DEF_HELPER_2(extr_rs_w, tl, i64, i32)
DEF_HELPER_2(extrv_w, tl, i64, tl)
DEF_HELPER_2(extrv_r_w, tl, i64, tl)
DEF_HELPER_2(extrv_rs_w, tl, i64, tl)
DEF_HELPER_2(insv, tl, tl, tl)
DEF_HELPER_2(mul_ph, tl, tl, tl)
DEF_HELPER_2(mul_s_ph, tl, tl, tl)
DEF_HELPER_2(mulq_s_ph, tl, tl, tl)
DEF_HELPER_4(maq_s_w_phl, i64, i32, i64, tl, tl)
DEF_HELPER_4(maq_sa_w_phl, i64, i32, i64, tl, tl)
DEF_HELPER_4(maq_s_w_phr, i64, i32, i64, tl, tl)
DEF_HELPER_4(maq_sa_w_phr, i64, i32, i64, tl, tl)
DEF_HELPER_2(modsub, tl, tl, tl)
DEF_HELPER_0(incPosBy32, void)
DEF_HELPER_2(muleq_s_w_phl, tl, tl, tl)
DEF_HELPER_2(muleq_s_w_phr, tl, tl, tl)
DEF_HELPER_2(muleu_s_ph_qbl, tl, tl, tl)
DEF_HELPER_2(muleu_s_ph_qbr, tl, tl, tl)
DEF_HELPER_2(mulq_rs_ph, tl, tl, tl)
DEF_HELPER_2(mulq_rs_w, tl, tl, tl)
DEF_HELPER_2(mulq_s_w, tl, tl, tl)
DEF_HELPER_3(mulsa_w_ph, i64, i64, tl, tl)
DEF_HELPER_4(mulsaq_s_w_ph, i64, i32, i64, tl, tl)
DEF_HELPER_2(packrl_ph, tl, tl, tl)
DEF_HELPER_2(pick_qb, tl, tl, tl)
DEF_HELPER_2(pick_ph, tl, tl, tl)
DEF_HELPER_1(preceq_w_phl, tl, tl)
DEF_HELPER_1(preceq_w_phr, tl, tl)
DEF_HELPER_1(precequ_ph_qbl, tl, tl)
DEF_HELPER_1(precequ_ph_qbla, tl, tl)
DEF_HELPER_1(precequ_ph_qbr, tl, tl)
DEF_HELPER_1(precequ_ph_qbra, tl, tl)
DEF_HELPER_1(preceu_ph_qbl, tl, tl)
DEF_HELPER_1(preceu_ph_qbla, tl, tl)
DEF_HELPER_1(preceu_ph_qbr, tl, tl)
DEF_HELPER_1(preceu_ph_qbra, tl, tl)
DEF_HELPER_2(precr_qb_ph, tl, tl, tl)
DEF_HELPER_3(precr_sra_ph_w, tl, tl, tl, i32)
DEF_HELPER_3(precr_sra_r_ph_w, tl, tl, tl, i32)
DEF_HELPER_2(precrq_qb_ph, tl, tl, tl)
DEF_HELPER_2(precrq_ph_w, tl, tl, tl)
DEF_HELPER_2(precrq_rs_ph_w, tl, tl, tl)
DEF_HELPER_2(precrqu_s_qb_ph, tl, tl, tl)
DEF_HELPER_3(prepend, tl, tl, tl, i32)
DEF_HELPER_1(raddu_w_qb, tl, tl)
DEF_HELPER_1(repl_ph, tl, i32)
DEF_HELPER_1(repl_qb, tl, i32)
DEF_HELPER_1(replv_ph, tl, tl)
DEF_HELPER_1(replv_qb, tl, tl)
DEF_HELPER_2(shilo, i64, i64, i32)
DEF_HELPER_2(shilov, i64, i64, tl)
DEF_HELPER_2(shll_ph, tl, tl, i32)
DEF_HELPER_2(shll_s_ph, tl, tl, i32)
DEF_HELPER_2(shll_qb, tl, tl, i32)
DEF_HELPER_2(shll_s_w, tl, tl, i32)
DEF_HELPER_2(shllv_ph, tl, tl, tl)
DEF_HELPER_2(shllv_s_ph, tl, tl, tl)
DEF_HELPER_2(shllv_qb, tl, tl, tl)
DEF_HELPER_2(shllv_s_w, tl, tl, tl)
DEF_HELPER_2(shra_ph, tl, tl, i32)
DEF_HELPER_2(shra_r_ph, tl, tl, i32)
DEF_HELPER_2(shra_qb, tl, tl, i32)
DEF_HELPER_2(shra_r_qb, tl, tl, i32)
DEF_HELPER_2(shra_r_w, tl, tl, i32)
DEF_HELPER_2(shrav_ph, tl, tl, tl)
DEF_HELPER_2(shrav_r_ph, tl, tl, tl)
DEF_HELPER_2(shrav_qb, tl, tl, tl)
DEF_HELPER_2(shrav_r_qb, tl, tl, tl)
DEF_HELPER_2(shrav_r_w, tl, tl, tl)
DEF_HELPER_2(shrl_ph, tl, tl, i32)
DEF_HELPER_2(shrl_qb, tl, tl, i32)
DEF_HELPER_2(shrlv_ph, tl, tl, tl)
DEF_HELPER_2(shrlv_qb, tl, tl, tl)

DEF_HELPER_2(subqh_ph, tl, tl, tl)
DEF_HELPER_2(subqh_r_ph, tl, tl, tl)
DEF_HELPER_2(subqh_r_w, tl, tl, tl)
DEF_HELPER_2(subqh_w, tl, tl, tl)
DEF_HELPER_2(subq_ph, tl, tl, tl)
DEF_HELPER_2(subq_s_ph, tl, tl, tl)
DEF_HELPER_2(subq_s_w, tl, tl, tl)
DEF_HELPER_2(subuh_r_qb, tl, tl, tl)
DEF_HELPER_2(subu_ph, tl, tl, tl)
DEF_HELPER_2(subu_qb, tl, tl, tl)
DEF_HELPER_2(subu_s_ph, tl, tl, tl)
DEF_HELPER_2(subu_s_qb, tl, tl, tl)
DEF_HELPER_2(subuh_qb, tl, tl, tl)

/****************************************************
 * DSP ASE R3 */

DEF_HELPER_2(add_qb, tl, tl, tl)
DEF_HELPER_2(add_s_qb, tl, tl, tl)

DEF_HELPER_2(asub_qb, tl, tl, tl)
DEF_HELPER_2(asubu_qb, tl, tl, tl)

DEF_HELPER_3(dpa_h_qb, i64, i64, tl, tl)
DEF_HELPER_3(dpau_h_qb, i64, i64, tl, tl)
DEF_HELPER_3(dps_h_qb, i64, i64, tl, tl)
DEF_HELPER_3(dpsu_h_qb, i64, i64, tl, tl)

DEF_HELPER_2(ilvev_qb, tl, tl, tl)
DEF_HELPER_2(ilvod_qb, tl, tl, tl)

DEF_HELPER_2(ilvl_qb, tl, tl, tl)
DEF_HELPER_2(ilvr_qb, tl, tl, tl)

DEF_HELPER_2(mul_qb, tl, tl, tl)
DEF_HELPER_2(mul_s_qb, tl, tl, tl)

DEF_HELPER_2(sub_qb, tl, tl, tl)
DEF_HELPER_2(sub_s_qb, tl, tl, tl)
DEF_HELPER_2(subus_s_qb, tl, tl, tl)

/* DSP ASE R3
 ***************************************************/

#ifndef CONFIG_USER_ONLY
/* CP0 helpers */
DEF_HELPER_0(mfc0_mvpcontrol, tl)
DEF_HELPER_0(mfc0_mvpconf0, tl)
DEF_HELPER_0(mfc0_mvpconf1, tl)
DEF_HELPER_0(mftc0_vpecontrol, tl)
DEF_HELPER_0(mftc0_vpeconf0, tl)
DEF_HELPER_0(mfc0_random, tl)
DEF_HELPER_0(mfc0_tcstatus, tl)
DEF_HELPER_0(mftc0_tcstatus, tl)
DEF_HELPER_0(mfc0_tcbind, tl)
DEF_HELPER_0(mftc0_tcbind, tl)
DEF_HELPER_0(mfc0_tcrestart, tl)
DEF_HELPER_0(mftc0_tcrestart, tl)
DEF_HELPER_0(mfc0_tchalt, tl)
DEF_HELPER_0(mftc0_tchalt, tl)
DEF_HELPER_0(mfc0_tccontext, tl)
DEF_HELPER_0(mftc0_tccontext, tl)
DEF_HELPER_0(mfc0_tcschedule, tl)
DEF_HELPER_0(mftc0_tcschedule, tl)
DEF_HELPER_0(mfc0_tcschefback, tl)
DEF_HELPER_0(mftc0_tcschefback, tl)
DEF_HELPER_0(mfc0_count, tl)
DEF_HELPER_0(mftc0_entryhi, tl)
DEF_HELPER_0(mftc0_status, tl)
DEF_HELPER_0(mftc0_cause, tl)
DEF_HELPER_0(mftc0_epc, tl)
DEF_HELPER_0(mftc0_ebase, tl)
DEF_HELPER_1(mftc0_configx, tl, tl)
DEF_HELPER_0(mfc0_lladdr, tl)
DEF_HELPER_1(mfc0_watchlo, tl, i32)
DEF_HELPER_1(mfc0_watchhi, tl, i32)
DEF_HELPER_0(mfc0_debug, tl)
DEF_HELPER_0(mftc0_debug, tl)
#ifdef TARGET_MIPS64
DEF_HELPER_0(dmfc0_tcrestart, tl)
DEF_HELPER_0(dmfc0_tchalt, tl)
DEF_HELPER_0(dmfc0_tccontext, tl)
DEF_HELPER_0(dmfc0_tcschedule, tl)
DEF_HELPER_0(dmfc0_tcschefback, tl)
DEF_HELPER_0(dmfc0_lladdr, tl)
DEF_HELPER_1(dmfc0_watchlo, tl, i32)
#endif /* TARGET_MIPS64 */

DEF_HELPER_1(mtc0_index, void, tl)
DEF_HELPER_1(mtc0_mvpcontrol, void, tl)
DEF_HELPER_1(mtc0_vpecontrol, void, tl)
DEF_HELPER_1(mttc0_vpecontrol, void, tl)
DEF_HELPER_1(mtc0_vpeconf0, void, tl)
DEF_HELPER_1(mttc0_vpeconf0, void, tl)
DEF_HELPER_1(mtc0_vpeconf1, void, tl)
DEF_HELPER_1(mtc0_yqmask, void, tl)
DEF_HELPER_1(mtc0_vpeopt, void, tl)
DEF_HELPER_1(mtc0_entrylo0, void, tl)
DEF_HELPER_1(mtc0_tcstatus, void, tl)
DEF_HELPER_1(mttc0_tcstatus, void, tl)
DEF_HELPER_1(mtc0_tcbind, void, tl)
DEF_HELPER_1(mttc0_tcbind, void, tl)
DEF_HELPER_1(mtc0_tcrestart, void, tl)
DEF_HELPER_1(mttc0_tcrestart, void, tl)
DEF_HELPER_1(mtc0_tchalt, void, tl)
DEF_HELPER_1(mttc0_tchalt, void, tl)
DEF_HELPER_1(mtc0_tccontext, void, tl)
DEF_HELPER_1(mttc0_tccontext, void, tl)
DEF_HELPER_1(mtc0_tcschedule, void, tl)
DEF_HELPER_1(mttc0_tcschedule, void, tl)
DEF_HELPER_1(mtc0_tcschefback, void, tl)
DEF_HELPER_1(mttc0_tcschefback, void, tl)
DEF_HELPER_1(mtc0_entrylo1, void, tl)
DEF_HELPER_1(mtc0_context, void, tl)
DEF_HELPER_1(mtc0_pagemask, void, tl)
DEF_HELPER_1(mtc0_pagegrain, void, tl)
DEF_HELPER_1(mtc0_wired, void, tl)
DEF_HELPER_1(mtc0_srsconf0, void, tl)
DEF_HELPER_1(mtc0_srsconf1, void, tl)
DEF_HELPER_1(mtc0_srsconf2, void, tl)
DEF_HELPER_1(mtc0_srsconf3, void, tl)
DEF_HELPER_1(mtc0_srsconf4, void, tl)
DEF_HELPER_1(mtc0_hwrena, void, tl)
DEF_HELPER_1(mtc0_count, void, tl)
DEF_HELPER_1(mtc0_entryhi, void, tl)
DEF_HELPER_1(mttc0_entryhi, void, tl)
DEF_HELPER_1(mtc0_compare, void, tl)
DEF_HELPER_1(mtc0_status, void, tl)
DEF_HELPER_1(mttc0_status, void, tl)
DEF_HELPER_1(mtc0_intctl, void, tl)
DEF_HELPER_1(mtc0_srsctl, void, tl)
DEF_HELPER_1(mtc0_cause, void, tl)
DEF_HELPER_1(mttc0_cause, void, tl)
DEF_HELPER_1(mtc0_ebase, void, tl)
DEF_HELPER_1(mttc0_ebase, void, tl)
DEF_HELPER_1(mtc0_config0, void, tl)
DEF_HELPER_1(mtc0_config2, void, tl)
DEF_HELPER_1(mtc0_lladdr, void, tl)
DEF_HELPER_2(mtc0_watchlo, void, tl, i32)
DEF_HELPER_2(mtc0_watchhi, void, tl, i32)
DEF_HELPER_1(mtc0_xcontext, void, tl)
DEF_HELPER_1(mtc0_framemask, void, tl)
DEF_HELPER_1(mtc0_debug, void, tl)
DEF_HELPER_1(mttc0_debug, void, tl)
DEF_HELPER_1(mtc0_performance0, void, tl)
DEF_HELPER_1(mtc0_taglo, void, tl)
DEF_HELPER_1(mtc0_datalo, void, tl)
DEF_HELPER_1(mtc0_taghi, void, tl)
DEF_HELPER_1(mtc0_datahi, void, tl)

/* MIPS MT functions */
DEF_HELPER_1(mftgpr, tl, i32);
DEF_HELPER_1(mftlo, tl, i32)
DEF_HELPER_1(mfthi, tl, i32)
DEF_HELPER_1(mftacx, tl, i32)
DEF_HELPER_0(mftdsp, tl)
DEF_HELPER_2(mttgpr, void, tl, i32)
DEF_HELPER_2(mttlo, void, tl, i32)
DEF_HELPER_2(mtthi, void, tl, i32)
DEF_HELPER_2(mttacx, void, tl, i32)
DEF_HELPER_1(mttdsp, void, tl)
DEF_HELPER_0(dmt, tl)
DEF_HELPER_0(emt, tl)
DEF_HELPER_0(dvpe, tl)
DEF_HELPER_0(evpe, tl)
#endif /* !CONFIG_USER_ONLY */

/* microMIPS functions */
DEF_HELPER_3(lwm, void, tl, tl, i32);
DEF_HELPER_3(swm, void, tl, tl, i32);
#ifdef TARGET_MIPS64
DEF_HELPER_3(ldm, void, tl, tl, i32);
DEF_HELPER_3(sdm, void, tl, tl, i32);
#endif

DEF_HELPER_2(fork, void, tl, tl)
DEF_HELPER_1(yield, tl, tl)

/* CP1 functions */
DEF_HELPER_1(cfc1, tl, i32)
DEF_HELPER_2(ctc1, void, tl, i32)

DEF_HELPER_1(float_cvtd_s, i64, i32)
DEF_HELPER_1(float_cvtd_w, i64, i32)
DEF_HELPER_1(float_cvtd_l, i64, i64)
DEF_HELPER_1(float_cvtl_d, i64, i64)
DEF_HELPER_1(float_cvtl_s, i64, i32)
DEF_HELPER_1(float_cvtps_pw, i64, i64)
DEF_HELPER_1(float_cvtpw_ps, i64, i64)
DEF_HELPER_1(float_cvts_d, i32, i64)
DEF_HELPER_1(float_cvts_w, i32, i32)
DEF_HELPER_1(float_cvts_l, i32, i64)
DEF_HELPER_1(float_cvts_pl, i32, i32)
DEF_HELPER_1(float_cvts_pu, i32, i32)
DEF_HELPER_1(float_cvtw_s, i32, i32)
DEF_HELPER_1(float_cvtw_d, i32, i64)

DEF_HELPER_2(float_addr_ps, i64, i64, i64)
DEF_HELPER_2(float_mulr_ps, i64, i64, i64)

#define FOP_PROTO(op)                       \
DEF_HELPER_1(float_ ## op ## l_s, i64, i32) \
DEF_HELPER_1(float_ ## op ## l_d, i64, i64) \
DEF_HELPER_1(float_ ## op ## w_s, i32, i32) \
DEF_HELPER_1(float_ ## op ## w_d, i32, i64)
FOP_PROTO(round)
FOP_PROTO(trunc)
FOP_PROTO(ceil)
FOP_PROTO(floor)
#undef FOP_PROTO

#define FOP_PROTO(op)                       \
DEF_HELPER_1(float_ ## op ## _s, i32, i32)  \
DEF_HELPER_1(float_ ## op ## _d, i64, i64)
FOP_PROTO(sqrt)
FOP_PROTO(rsqrt)
FOP_PROTO(recip)
#undef FOP_PROTO

#define FOP_PROTO(op)                       \
DEF_HELPER_1(float_ ## op ## _s, i32, i32)  \
DEF_HELPER_1(float_ ## op ## _d, i64, i64)  \
DEF_HELPER_1(float_ ## op ## _ps, i64, i64)
FOP_PROTO(abs)
FOP_PROTO(chs)
FOP_PROTO(recip1)
FOP_PROTO(rsqrt1)
#undef FOP_PROTO

#define FOP_PROTO(op)                             \
DEF_HELPER_2(float_ ## op ## _s, i32, i32, i32)   \
DEF_HELPER_2(float_ ## op ## _d, i64, i64, i64)   \
DEF_HELPER_2(float_ ## op ## _ps, i64, i64, i64)
FOP_PROTO(add)
FOP_PROTO(sub)
FOP_PROTO(mul)
FOP_PROTO(div)
FOP_PROTO(recip2)
FOP_PROTO(rsqrt2)
#undef FOP_PROTO

#define FOP_PROTO(op)                                 \
DEF_HELPER_3(float_ ## op ## _s, i32, i32, i32, i32)  \
DEF_HELPER_3(float_ ## op ## _d, i64, i64, i64, i64)  \
DEF_HELPER_3(float_ ## op ## _ps, i64, i64, i64, i64)
FOP_PROTO(muladd)
FOP_PROTO(mulsub)
FOP_PROTO(nmuladd)
FOP_PROTO(nmulsub)
#undef FOP_PROTO

#define FOP_PROTO(op)                               \
DEF_HELPER_3(cmp_d_ ## op, void, i64, i64, int)     \
DEF_HELPER_3(cmpabs_d_ ## op, void, i64, i64, int)  \
DEF_HELPER_3(cmp_s_ ## op, void, i32, i32, int)     \
DEF_HELPER_3(cmpabs_s_ ## op, void, i32, i32, int)  \
DEF_HELPER_3(cmp_ps_ ## op, void, i64, i64, int)    \
DEF_HELPER_3(cmpabs_ps_ ## op, void, i64, i64, int)
FOP_PROTO(f)
FOP_PROTO(un)
FOP_PROTO(eq)
FOP_PROTO(ueq)
FOP_PROTO(olt)
FOP_PROTO(ult)
FOP_PROTO(ole)
FOP_PROTO(ule)
FOP_PROTO(sf)
FOP_PROTO(ngle)
FOP_PROTO(seq)
FOP_PROTO(ngl)
FOP_PROTO(lt)
FOP_PROTO(nge)
FOP_PROTO(le)
FOP_PROTO(ngt)
#undef FOP_PROTO

/* Special functions */
#ifndef CONFIG_USER_ONLY
DEF_HELPER_0(tlbwi, void)
DEF_HELPER_0(tlbwr, void)
DEF_HELPER_0(tlbp, void)
DEF_HELPER_0(tlbr, void)
DEF_HELPER_0(di, tl)
DEF_HELPER_0(ei, tl)
DEF_HELPER_0(eret, void)
DEF_HELPER_0(deret, void)
#endif /* !CONFIG_USER_ONLY */
DEF_HELPER_0(rdhwr_cpunum, tl)
DEF_HELPER_0(rdhwr_synci_step, tl)
DEF_HELPER_0(rdhwr_cc, tl)
DEF_HELPER_0(rdhwr_ccres, tl)
DEF_HELPER_1(pmon, void, int)
DEF_HELPER_0(wait, void)

/* MSA */
DEF_HELPER_3(load_wr_s64, s64, int, int, int)
DEF_HELPER_3(load_wr_modulo_s64, s64, int, int, int)

DEF_HELPER_3(load_wr_i64, i64, int, int, int)
DEF_HELPER_3(load_wr_modulo_i64, i64, int, int, int)

DEF_HELPER_4(store_wr, void, i64, int, int, int)
DEF_HELPER_4(store_wr_modulo, void, i64, int, int, int)

#include "mips_msa_helper_decl.h"

#include "def-helper.h"
