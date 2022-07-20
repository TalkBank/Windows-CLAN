/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* multiple version mul.c */

#if !defined(UNX)
#ifndef _COCOA_APP
#include "ced.h"
#endif
#endif
#include "cu.h"
#include <setjmp.h>

static jmp_buf my_jmp_buf;
char skip_prog = 0;

int isKillProgram;

/* constants for all the commands */

const char *clan_name[LAST_CLAN_PROG];

/* set up an array of pointers to functions */

void (*clan_main[LAST_CLAN_PROG]) (int argc, char *argv[]);
void (*clan_usage[LAST_CLAN_PROG]) (void);
void (*clan_getflag[LAST_CLAN_PROG]) (char *f,char *f1, int *i);
void (*clan_init[LAST_CLAN_PROG]) (char c);
void (*clan_call[LAST_CLAN_PROG]) (void);


/* ************************************************************* */
/*    This is sample dummy version of the function definitions 

void dummy_main(int argc, char *argv[]);
void dummy_usage(void);
void dummy_getflag(char *f,char *f1, int *i);
void dummy_init(char c);
void dummy_call(void);

*/ 
/* ************************************************************* */
void anvil2chat_main(int argc, char *argv[]);
void anvil2chat_usage(void);
void anvil2chat_getflag(char *f,char *f1, int *i);
void anvil2chat_init(char c);
void anvil2chat_call(void);

void chains_main(int argc, char *argv[]);
void chains_usage(void);
void chains_getflag(char *f,char *f1, int *i);
void chains_init(char c);
void chains_call(void);

void chat2anvil_main(int argc, char *argv[]);
void chat2anvil_usage(void);
void chat2anvil_getflag(char *f,char *f1, int *i);
void chat2anvil_init(char c);
void chat2anvil_call(void);

void Chat2Conll_main(int argc, char *argv[]);
void Chat2Conll_usage(void);
void Chat2Conll_getflag(char *f,char *f1, int *i);
void Chat2Conll_init(char c);
void Chat2Conll_call(void);

void chat2elan_main(int argc, char *argv[]);
void chat2elan_usage(void);
void chat2elan_getflag(char *f,char *f1, int *i);
void chat2elan_init(char c);
void chat2elan_call(void);

void chat2praat_main(int argc, char *argv[]);
void chat2praat_usage(void);
void chat2praat_getflag(char *f,char *f1, int *i);
void chat2praat_init(char c);
void chat2praat_call(void);

void chat2srt_main(int argc, char *argv[]);
void chat2srt_usage(void);
void chat2srt_getflag(char *f,char *f1, int *i);
void chat2srt_init(char c);
void chat2srt_call(void);

void chat2xmar_main(int argc, char *argv[]);
void chat2xmar_usage(void);
void chat2xmar_getflag(char *f,char *f1, int *i);
void chat2xmar_init(char c);
void chat2xmar_call(void);

void check_main(int argc, char *argv[]);
void check_usage(void);
void check_getflag(char *f,char *f1, int *i);
void check_init(char c);
void check_call(void);

void chip_main(int argc, char *argv[]);
void chip_usage(void);
void chip_getflag(char *f,char *f1, int *i);
void chip_init(char c);
void chip_call(void);

void chstring_main(int argc, char *argv[]);
void chstring_usage(void);
void chstring_getflag(char *f,char *f1, int *i);
void chstring_init(char c);
void chstring_call(void);
			
void c_nnla_main(int argc, char *argv[]);
void c_nnla_usage(void);
void c_nnla_getflag(char *f,char *f1, int *i);
void c_nnla_init(char c);
void c_nnla_call(void);

void codes_main(int argc, char *argv[]);
void codes_usage(void);
void codes_getflag(char *f,char *f1, int *i);
void codes_init(char c);
void codes_call(void);

void Conll2Chat_main(int argc, char *argv[]);
void Conll2Chat_usage(void);
void Conll2Chat_getflag(char *f,char *f1, int *i);
void Conll2Chat_init(char c);
void Conll2Chat_call(void);

void combo_main(int argc, char *argv[]);
void combo_usage(void);
void combo_getflag(char *f,char *f1, int *i);
void combo_init(char c);
void combo_call(void);

void combtier_main(int argc, char *argv[]);
void combtier_usage(void);
void combtier_getflag(char *f,char *f1, int *i);
void combtier_init(char c);
void combtier_call(void);

void cooccur_main(int argc, char *argv[]);
void cooccur_usage(void);
void cooccur_getflag(char *f,char *f1, int *i);
void cooccur_init(char c);
void cooccur_call(void);

void corelex_main(int argc, char *argv[]);
void corelex_usage(void);
void corelex_getflag(char *f,char *f1, int *i);
void corelex_init(char c);
void corelex_call(void);

void cptoutf_main(int argc, char *argv[]);
void cptoutf_usage(void);
void cptoutf_getflag(char *f,char *f1, int *i);
void cptoutf_init(char c);
void cptoutf_call(void);

void c_qpa_main(int argc, char *argv[]);
void c_qpa_usage(void);
void c_qpa_getflag(char *f,char *f1, int *i);
void c_qpa_init(char c);
void c_qpa_call(void);

void dates_main(int argc, char *argv[]);
void dates_usage(void);
void dates_getflag(char *f,char *f1, int *i);
void dates_init(char c);
void dates_call(void);

void dc_main(int argc, char *argv[]);
void dc_usage(void);
void dc_getflag(char *f,char *f1, int *i);
void dc_init(char c);
void dc_call(void);

void delim_main(int argc, char *argv[]);
void delim_usage(void);
void delim_getflag(char *f,char *f1, int *i);
void delim_init(char c);
void delim_call(void);

void dist_main(int argc, char *argv[]);
void dist_usage(void);
void dist_getflag(char *f,char *f1, int *i);
void dist_init(char c);
void dist_call(void);

void dss_main(int argc, char *argv[]);
void dss_usage(void);
void dss_getflag(char *f,char *f1, int *i);
void dss_init(char c);
void dss_call(void);

void elan2chat_main(int argc, char *argv[]);
void elan2chat_usage(void);
void elan2chat_getflag(char *f,char *f1, int *i);
void elan2chat_init(char c);
void elan2chat_call(void);

void eval_main(int argc, char *argv[]);
void eval_usage(void);
void eval_getflag(char *f,char *f1, int *i);
void eval_init(char c);
void eval_call(void);

void fixbullets_main(int argc, char *argv[]);
void fixbullets_usage(void);
void fixbullets_getflag(char *f,char *f1, int *i);
void fixbullets_init(char c);
void fixbullets_call(void);

void fixit_main(int argc, char *argv[]);
void fixit_usage(void);
void fixit_getflag(char *f,char *f1, int *i);
void fixit_init(char c);
void fixit_call(void);

#ifndef _COCOA_APP
void fixMP3s_main(int argc, char *argv[]);
void fixMP3s_usage(void);
void fixMP3s_getflag(char *f,char *f1, int *i);
void fixMP3s_init(char c);
void fixMP3s_call(void);
#endif

void flo_main(int argc, char *argv[]);
void flo_usage(void);
void flo_getflag(char *f,char *f1, int *i);
void flo_init(char c);
void flo_call(void);

void flucalc_main(int argc, char *argv[]);
void flucalc_usage(void);
void flucalc_getflag(char *f,char *f1, int *i);
void flucalc_init(char c);
void flucalc_call(void);

void freq_main(int argc, char *argv[]);
void freq_usage(void);
void freq_getflag(char *f,char *f1, int *i);
void freq_init(char c);
void freq_call(void);

void freqpos_main(int argc, char *argv[]);
void freqpos_usage(void);
void freqpos_getflag(char *f,char *f1, int *i);
void freqpos_init(char c);
void freqpos_call(void);

void gem_main(int argc, char *argv[]);
void gem_usage(void);
void gem_getflag(char *f,char *f1, int *i);
void gem_init(char c);
void gem_call(void);

void gemfreq_main(int argc, char *argv[]);
void gemfreq_usage(void);
void gemfreq_getflag(char *f,char *f1, int *i);
void gemfreq_init(char c);
void gemfreq_call(void);

void gemlist_main(int argc, char *argv[]);
void gemlist_usage(void);
void gemlist_getflag(char *f,char *f1, int *i);
void gemlist_init(char c);
void gemlist_call(void);

void cmdi_main(int argc, char *argv[]);
void cmdi_usage(void);
void cmdi_getflag(char *f,char *f1, int *i);
void cmdi_init(char c);
void cmdi_call(void);

void indentoverlap_main(int argc, char *argv[]);
void indentoverlap_usage(void);
void indentoverlap_getflag(char *f,char *f1, int *i);
void indentoverlap_init(char c);
void indentoverlap_call(void);

void ipsyn_main(int argc, char *argv[]);
void ipsyn_usage(void);
void ipsyn_getflag(char *f,char *f1, int *i);
void ipsyn_init(char c);
void ipsyn_call(void);

void keymap_main(int argc, char *argv[]);
void keymap_usage(void);
void keymap_getflag(char *f,char *f1, int *i);
void keymap_init(char c);
void keymap_call(void);

void kideval_main(int argc, char *argv[]);
void kideval_usage(void);
void kideval_getflag(char *f,char *f1, int *i);
void kideval_init(char c);
void kideval_call(void);

void kwal_main(int argc, char *argv[]);
void kwal_usage(void);
void kwal_getflag(char *f,char *f1, int *i);
void kwal_init(char c);
void kwal_call(void);

void lab2chat_main(int argc, char *argv[]);
void lab2chat_usage(void);
void lab2chat_getflag(char *f,char *f1, int *i);
void lab2chat_init(char c);
void lab2chat_call(void);

void lena2chat_main(int argc, char *argv[]);
void lena2chat_usage(void);
void lena2chat_getflag(char *f,char *f1, int *i);
void lena2chat_init(char c);
void lena2chat_call(void);

void lipp2chat_main(int argc, char *argv[]);
void lipp2chat_usage(void);
void lipp2chat_getflag(char *f,char *f1, int *i);
void lipp2chat_init(char c);
void lipp2chat_call(void);

void longtier_main(int argc, char *argv[]);
void longtier_usage(void);
void longtier_getflag(char *f,char *f1, int *i);
void longtier_init(char c);
void longtier_call(void);

void lowcase_main(int argc, char *argv[]);
void lowcase_usage(void);
void lowcase_getflag(char *f,char *f1, int *i);
void lowcase_init(char c);
void lowcase_call(void);

void makemod_main(int argc, char *argv[]);
void makemod_usage(void);
void makemod_getflag(char *f,char *f1, int *i);
void makemod_init(char c);
void makemod_call(void);

void maxwd_main(int argc, char *argv[]);
void maxwd_usage(void);
void maxwd_getflag(char *f,char *f1, int *i);
void maxwd_init(char c);
void maxwd_call(void);

void medialine_main(int argc, char *argv[]);
void medialine_usage(void);
void medialine_getflag(char *f,char *f1, int *i);
void medialine_init(char c);
void medialine_call(void);

void megrasp_main(int argc, char *argv[]);
void megrasp_usage(void);
void megrasp_getflag(char *f,char *f1, int *i);
void megrasp_init(char c);
void megrasp_call(void);

void mlt_main(int argc, char *argv[]);
void mlt_usage(void);
void mlt_getflag(char *f,char *f1, int *i);
void mlt_init(char c);
void mlt_call(void);

void mlu_main(int argc, char *argv[]);
void mlu_usage(void);
void mlu_getflag(char *f,char *f1, int *i);
void mlu_init(char c);
void mlu_call(void);

void modrep_main(int argc, char *argv[]);
void modrep_usage(void);
void modrep_getflag(char *f,char *f1, int *i);
void modrep_init(char c);
void modrep_call(void);

void mor_main(int argc, char *argv[]);
void mor_usage(void);
void mor_getflag(char *f,char *f1, int *i);
void mor_init(char c);
void mor_call(void);

void mortable_main(int argc, char *argv[]);
void mortable_usage(void);
void mortable_getflag(char *f,char *f1, int *i);
void mortable_init(char c);
void mortable_call(void);

void olac_main(int argc, char *argv[]);
void olac_usage(void);
void olac_getflag(char *f,char *f1, int *i);
void olac_init(char c);
void olac_call(void);

void ort_main(int argc, char *argv[]);
void ort_usage(void);
void ort_getflag(char *f,char *f1, int *i);
void ort_init(char c);
void ort_call(void);

void phonfreq_main(int argc, char *argv[]);
void phonfreq_usage(void);
void phonfreq_getflag(char *f,char *f1, int *i);
void phonfreq_init(char c);
void phonfreq_call(void);

void postmortem_main(int argc, char *argv[]);
void postmortem_usage(void);
void postmortem_getflag(char *f,char *f1, int *i);
void postmortem_init(char c);
void postmortem_call(void);

void play2chat_main(int argc, char *argv[]);
void play2chat_usage(void);
void play2chat_getflag(char *f,char *f1, int *i);
void play2chat_init(char c);
void play2chat_call(void);

void praat2chat_main(int argc, char *argv[]);
void praat2chat_usage(void);
void praat2chat_getflag(char *f,char *f1, int *i);
void praat2chat_init(char c);
void praat2chat_call(void);

void quotes_main(int argc, char *argv[]);
void quotes_usage(void);
void quotes_getflag(char *f,char *f1, int *i);
void quotes_init(char c);
void quotes_call(void);

void rely_main(int argc, char *argv[]);
void rely_usage(void);
void rely_getflag(char *f,char *f1, int *i);
void rely_init(char c);
void rely_call(void);

void repeat_main(int argc, char *argv[]);
void repeat_usage(void);
void repeat_getflag(char *f,char *f1, int *i);
void repeat_init(char c);
void repeat_call(void);

void retrace_main(int argc, char *argv[]);
void retrace_usage(void);
void retrace_getflag(char *f,char *f1, int *i);
void retrace_init(char c);
void retrace_call(void);

void rtf2chat_main(int argc, char *argv[]);
void rtf2chat_usage(void);
void rtf2chat_getflag(char *f,char *f1, int *i);
void rtf2chat_init(char c);
void rtf2chat_call(void);

void salt2chat_main(int argc, char *argv[]);
void salt2chat_usage(void);
void salt2chat_getflag(char *f,char *f1, int *i);
void salt2chat_init(char c);
void salt2chat_call(void);

#ifndef _COCOA_APP
void silence_main(int argc, char *argv[]);
void silence_usage(void);
void silence_getflag(char *f,char *f1, int *i);
void silence_init(char c);
void silence_call(void);
#endif

void script_main(int argc, char *argv[]);
void script_usage(void);
void script_getflag(char *f,char *f1, int *i);
void script_init(char c);
void script_call(void);

void srt2chat_main(int argc, char *argv[]);
void srt2chat_usage(void);
void srt2chat_getflag(char *f,char *f1, int *i);
void srt2chat_init(char c);
void srt2chat_call(void);

void sugar_main(int argc, char *argv[]);
void sugar_usage(void);
void sugar_getflag(char *f,char *f1, int *i);
void sugar_init(char c);
void sugar_call(void);

void text2chat_main(int argc, char *argv[]);
void text2chat_usage(void);
void text2chat_getflag(char *f,char *f1, int *i);
void text2chat_init(char c);
void text2chat_call(void);

void time_main(int argc, char *argv[]);
void time_usage(void);
void time_getflag(char *f,char *f1, int *i);
void time_init(char c);
void time_call(void);

void tierorder_main(int argc, char *argv[]);
void tierorder_usage(void);
void tierorder_getflag(char *f,char *f1, int *i);
void tierorder_init(char c);
void tierorder_call(void);

void toCA_main(int argc, char *argv[]);
void toCA_usage(void);
void toCA_getflag(char *f,char *f1, int *i);
void toCA_init(char c);
void toCA_call(void);

void trnfix_main(int argc, char *argv[]);
void trnfix_usage(void);
void trnfix_getflag(char *f,char *f1, int *i);
void trnfix_init(char c);
void trnfix_call(void);

void uniq_main(int argc, char *argv[]);
void uniq_usage(void);
void uniq_getflag(char *f,char *f1, int *i);
void uniq_init(char c);
void uniq_call(void);

void usedlex_main(int argc, char *argv[]);
void usedlex_usage(void);
void usedlex_getflag(char *f,char *f1, int *i);
void usedlex_init(char c);
void usedlex_call(void);

void vocd_main(int argc, char *argv[]);
void vocd_usage(void);
void vocd_getflag(char *f,char *f1, int *i);
void vocd_init(char c);
void vocd_call(void);

void wdlen_main(int argc, char *argv[]);
void wdlen_usage(void);
void wdlen_getflag(char *f,char *f1, int *i);
void wdlen_init(char c);
void wdlen_call(void);

void compound_main(int argc, char *argv[]);
void compound_usage(void);
void compound_getflag(char *f,char *f1, int *i);
void compound_init(char c);
void compound_call(void);

void postprog_main(int argc, char *argv[]);
void postprog_usage(void);
void postprog_getflag(char *f,char *f1, int *i);
void postprog_init(char c);
void postprog_call(void);

void postlist_main(int argc, char *argv[]);
void postlist_usage(void);
void postlist_getflag(char *f,char *f1, int *i);
void postlist_init(char c);
void postlist_call(void);

void postmodrules_main(int argc, char *argv[]);
void postmodrules_usage(void);
void postmodrules_getflag(char *f,char *f1, int *i);
void postmodrules_init(char c);
void postmodrules_call(void);

void posttrain_main(int argc, char *argv[]);
void posttrain_usage(void);
void posttrain_getflag(char *f,char *f1, int *i);
void posttrain_init(char c);
void posttrain_call(void);


// all TEMP??

void temp_main(int argc, char *argv[]);
void temp_usage(void);
void temp_getflag(char *f,char *f1, int *i);
void temp_init(char c);
void temp_call(void);

void d2u_main(int argc, char *argv[]);
void d2u_usage(void);
void d2u_getflag(char *f,char *f1, int *i);
void d2u_init(char c);
void d2u_call(void);

void lines_main(int argc, char *argv[]);
void lines_usage(void);
void lines_getflag(char *f,char *f1, int *i);
void lines_init(char c);
void lines_call(void);

void pp_main(int argc, char *argv[]);
void pp_usage(void);
void pp_getflag(char *f,char *f1, int *i);
void pp_init(char c);
void pp_call(void);

void gps_main(int argc, char *argv[]);
void gps_usage(void);
void gps_getflag(char *f,char *f1, int *i);
void gps_init(char c);
void gps_call(void);

// all TEMP??

/*
void _main(int argc, char *argv[]);
void _usage(void);
void _getflag(char *f,char *f1, int *i);
void _init(char c);
void _call(void);
*/

static int already_inited = 0;

void func_init(void) {
	int i;

	if (already_inited)
		return;	

	already_inited = 1;
	for(i=0; i < LAST_CLAN_PROG; i++) {
		clan_main[i]		= NULL;
		clan_usage[i] 		= NULL;
		clan_getflag[i]		= NULL;
		clan_init[i]		= NULL;
		clan_call[i]		= NULL;
		clan_name[i]		= "";
	}

/* ************************************************************* */
/*    This is sample dummy version of the function definitions 

	clan_main[DUMMY]		= dummy_main;
	clan_usage[DUMMY] 		= dummy_usage;
	clan_getflag[DUMMY]		= dummy_getflag;
	clan_init[DUMMY]		= dummy_init;
	clan_call[DUMMY]		= dummy_call;

*/ 
/* ************************************************************* */

	clan_main[CHAINS]		= chains_main;
	clan_usage[CHAINS] 		= chains_usage;
	clan_getflag[CHAINS]	= chains_getflag;
	clan_init[CHAINS]		= chains_init;
	clan_call[CHAINS]		= chains_call;

	clan_main[CHIP]			= chip_main;
	clan_usage[CHIP] 		= chip_usage;
	clan_getflag[CHIP]		= chip_getflag;
	clan_init[CHIP]			= chip_init;
	clan_call[CHIP]			= chip_call;

	clan_main[C_NNLA]		= c_nnla_main;
	clan_usage[C_NNLA]		= c_nnla_usage;
	clan_getflag[C_NNLA]	= c_nnla_getflag;
	clan_init[C_NNLA]		= c_nnla_init;
	clan_call[C_NNLA]		= c_nnla_call;

	clan_main[CODES_P]		= codes_main;
	clan_usage[CODES_P]		= codes_usage;
	clan_getflag[CODES_P]	= codes_getflag;
	clan_init[CODES_P]		= codes_init;
	clan_call[CODES_P]		= codes_call;

	clan_main[COMBO]		= combo_main;
	clan_usage[COMBO] 		= combo_usage;
	clan_getflag[COMBO]		= combo_getflag;
	clan_init[COMBO]		= combo_init;
	clan_call[COMBO]		= combo_call;

	clan_main[COOCCUR]		= cooccur_main;
	clan_usage[COOCCUR] 	= cooccur_usage;
	clan_getflag[COOCCUR]	= cooccur_getflag;
	clan_init[COOCCUR]		= cooccur_init;
	clan_call[COOCCUR]		= cooccur_call;

	clan_main[CORELEX]		= corelex_main;
	clan_usage[CORELEX]		= corelex_usage;
	clan_getflag[CORELEX]	= corelex_getflag;
	clan_init[CORELEX]		= corelex_init;
	clan_call[CORELEX]		= corelex_call;

	clan_main[DIST]			= dist_main;
	clan_usage[DIST] 		= dist_usage;
	clan_getflag[DIST]		= dist_getflag;
	clan_init[DIST]			= dist_init;
	clan_call[DIST]			= dist_call;

	clan_main[DSS]			= dss_main;
	clan_usage[DSS] 		= dss_usage;
	clan_getflag[DSS]		= dss_getflag;
	clan_init[DSS]			= dss_init;
	clan_call[DSS]			= dss_call;

	clan_main[EVAL]			= eval_main;
	clan_usage[EVAL]		= eval_usage;
	clan_getflag[EVAL]		= eval_getflag;
	clan_init[EVAL]			= eval_init;
	clan_call[EVAL]			= eval_call;

	clan_main[FLUCALC]		= flucalc_main;
	clan_usage[FLUCALC] 	= flucalc_usage;
	clan_getflag[FLUCALC]	= flucalc_getflag;
	clan_init[FLUCALC]		= flucalc_init;
	clan_call[FLUCALC]		= flucalc_call;

	clan_main[FREQ]			= freq_main;
	clan_usage[FREQ] 		= freq_usage;
	clan_getflag[FREQ]		= freq_getflag;
	clan_init[FREQ]			= freq_init;
	clan_call[FREQ]			= freq_call;

	clan_main[FREQPOS]		= freqpos_main;
	clan_usage[FREQPOS] 	= freqpos_usage;
	clan_getflag[FREQPOS]	= freqpos_getflag;
	clan_init[FREQPOS]		= freqpos_init;
	clan_call[FREQPOS]		= freqpos_call;

	clan_main[GEM]			= gem_main;
	clan_usage[GEM] 		= gem_usage;
	clan_getflag[GEM]		= gem_getflag;
	clan_init[GEM]			= gem_init;
	clan_call[GEM]			= gem_call;

	clan_main[GEMFREQ]		= gemfreq_main;
	clan_usage[GEMFREQ] 	= gemfreq_usage;
	clan_getflag[GEMFREQ]	= gemfreq_getflag;
	clan_init[GEMFREQ]		= gemfreq_init;
	clan_call[GEMFREQ]		= gemfreq_call;

	clan_main[GEMLIST]		= gemlist_main;
	clan_usage[GEMLIST] 	= gemlist_usage;
	clan_getflag[GEMLIST]	= gemlist_getflag;
	clan_init[GEMLIST]		= gemlist_init;
	clan_call[GEMLIST]		= gemlist_call;

	clan_main[IPSYN]		= ipsyn_main;
	clan_usage[IPSYN]		= ipsyn_usage;
	clan_getflag[IPSYN]		= ipsyn_getflag;
	clan_init[IPSYN]		= ipsyn_init;
	clan_call[IPSYN]		= ipsyn_call;

	clan_main[KEYMAP]		= keymap_main;
	clan_usage[KEYMAP] 		= keymap_usage;
	clan_getflag[KEYMAP]	= keymap_getflag;
	clan_init[KEYMAP]		= keymap_init;
	clan_call[KEYMAP]		= keymap_call;

	clan_main[KIDEVAL]		= kideval_main;
	clan_usage[KIDEVAL]		= kideval_usage;
	clan_getflag[KIDEVAL]	= kideval_getflag;
	clan_init[KIDEVAL]		= kideval_init;
	clan_call[KIDEVAL]		= kideval_call;

	clan_main[KWAL]			= kwal_main;
	clan_usage[KWAL] 		= kwal_usage;
	clan_getflag[KWAL]		= kwal_getflag;
	clan_init[KWAL]			= kwal_init;
	clan_call[KWAL]			= kwal_call;

	clan_main[MAXWD]		= maxwd_main;
	clan_usage[MAXWD] 		= maxwd_usage;
	clan_getflag[MAXWD]		= maxwd_getflag;
	clan_init[MAXWD]		= maxwd_init;
	clan_call[MAXWD]		= maxwd_call;

	clan_main[MEGRASP]		= megrasp_main;
	clan_usage[MEGRASP]		= megrasp_usage;
	clan_getflag[MEGRASP]	= megrasp_getflag;
	clan_init[MEGRASP]		= megrasp_init;
	clan_call[MEGRASP]		= megrasp_call;

	clan_main[MLT]			= mlt_main;
	clan_usage[MLT] 		= mlt_usage;
	clan_getflag[MLT]		= mlt_getflag;
	clan_init[MLT]			= mlt_init;
	clan_call[MLT]			= mlt_call;

	clan_main[MLU]			= mlu_main;
	clan_usage[MLU] 		= mlu_usage;
	clan_getflag[MLU]		= mlu_getflag;
	clan_init[MLU]			= mlu_init;
	clan_call[MLU]			= mlu_call;

	clan_main[MODREP]		= modrep_main;
	clan_usage[MODREP] 		= modrep_usage;
	clan_getflag[MODREP]	= modrep_getflag;
	clan_init[MODREP]		= modrep_init;
	clan_call[MODREP]		= modrep_call;

	clan_main[MOR_P]		= mor_main;
	clan_usage[MOR_P] 		= mor_usage;
	clan_getflag[MOR_P]		= mor_getflag;
	clan_init[MOR_P]		= mor_init;
	clan_call[MOR_P]		= mor_call;

	clan_main[MORTABLE]		= mortable_main;
	clan_usage[MORTABLE]	= mortable_usage;
	clan_getflag[MORTABLE]	= mortable_getflag;
	clan_init[MORTABLE]		= mortable_init;
	clan_call[MORTABLE]		= mortable_call;

	clan_main[PHONFREQ]		= phonfreq_main;
	clan_usage[PHONFREQ] 	= phonfreq_usage;
	clan_getflag[PHONFREQ]	= phonfreq_getflag;
	clan_init[PHONFREQ]		= phonfreq_init;
	clan_call[PHONFREQ]		= phonfreq_call;

	clan_main[POST]			= postprog_main;
	clan_usage[POST] 		= postprog_usage;
	clan_getflag[POST]		= postprog_getflag;
	clan_init[POST]			= postprog_init;
	clan_call[POST]			= postprog_call;

	clan_main[POSTLIST]		= postlist_main;
	clan_usage[POSTLIST] 	= postlist_usage;
	clan_getflag[POSTLIST]	= postlist_getflag;
	clan_init[POSTLIST]		= postlist_init;
	clan_call[POSTLIST]		= postlist_call;

	clan_main[POSTMODRULES]	= postmodrules_main;
	clan_usage[POSTMODRULES]= postmodrules_usage;
	clan_getflag[POSTMODRULES]= postmodrules_getflag;
	clan_init[POSTMODRULES]	= postmodrules_init;
	clan_call[POSTMODRULES]	= postmodrules_call;

	clan_main[POSTMORTEM]	= postmortem_main;
	clan_usage[POSTMORTEM]	= postmortem_usage;
	clan_getflag[POSTMORTEM]= postmortem_getflag;
	clan_init[POSTMORTEM]	= postmortem_init;
	clan_call[POSTMORTEM]	= postmortem_call;

	clan_main[POSTTRAIN]	= posttrain_main;
	clan_usage[POSTTRAIN] 	= posttrain_usage;
	clan_getflag[POSTTRAIN]	= posttrain_getflag;
	clan_init[POSTTRAIN]	= posttrain_init;
	clan_call[POSTTRAIN]	= posttrain_call;

	clan_main[C_QPA]		= c_qpa_main;
	clan_usage[C_QPA]		= c_qpa_usage;
	clan_getflag[C_QPA]		= c_qpa_getflag;
	clan_init[C_QPA]		= c_qpa_init;
	clan_call[C_QPA]		= c_qpa_call;

	clan_main[RELY]			= rely_main;
	clan_usage[RELY] 		= rely_usage;
	clan_getflag[RELY]		= rely_getflag;
	clan_init[RELY]			= rely_init;
	clan_call[RELY]			= rely_call;

	clan_main[SCRIPT_P]		= script_main;
	clan_usage[SCRIPT_P] 	= script_usage;
	clan_getflag[SCRIPT_P]	= script_getflag;
	clan_init[SCRIPT_P]		= script_init;
	clan_call[SCRIPT_P]		= script_call;

	clan_main[SUGAR]		= sugar_main;
	clan_usage[SUGAR]		= sugar_usage;
	clan_getflag[SUGAR]		= sugar_getflag;
	clan_init[SUGAR]		= sugar_init;
	clan_call[SUGAR]		= sugar_call;

	clan_main[TIMEDUR]		= time_main;
	clan_usage[TIMEDUR] 	= time_usage;
	clan_getflag[TIMEDUR]	= time_getflag;
	clan_init[TIMEDUR]		= time_init;
	clan_call[TIMEDUR]		= time_call;

	clan_main[VOCD]			= vocd_main;
	clan_usage[VOCD] 		= vocd_usage;
	clan_getflag[VOCD]		= vocd_getflag;
	clan_init[VOCD]			= vocd_init;
	clan_call[VOCD]			= vocd_call;

	clan_main[WDLEN]		= wdlen_main;
	clan_usage[WDLEN] 		= wdlen_usage;
	clan_getflag[WDLEN]		= wdlen_getflag;
	clan_init[WDLEN]		= wdlen_init;
	clan_call[WDLEN]		= wdlen_call;


	clan_main[CHSTRING]		= chstring_main;
	clan_usage[CHSTRING] 	= chstring_usage;
	clan_getflag[CHSTRING]	= chstring_getflag;
	clan_init[CHSTRING]		= chstring_init;
	clan_call[CHSTRING]		= chstring_call;

	clan_main[ANVIL2CHAT]	= anvil2chat_main;
	clan_usage[ANVIL2CHAT]	= anvil2chat_usage;
	clan_getflag[ANVIL2CHAT]= anvil2chat_getflag;
	clan_init[ANVIL2CHAT]	= anvil2chat_init;
	clan_call[ANVIL2CHAT]	= anvil2chat_call;

	clan_main[CHAT2ANVIL]	= chat2anvil_main;
	clan_usage[CHAT2ANVIL]	= chat2anvil_usage;
	clan_getflag[CHAT2ANVIL]= chat2anvil_getflag;
	clan_init[CHAT2ANVIL]	= chat2anvil_init;
	clan_call[CHAT2ANVIL]	= chat2anvil_call;

	clan_main[CHAT2CA]		= toCA_main;
	clan_usage[CHAT2CA]		= toCA_usage;
	clan_getflag[CHAT2CA]	= toCA_getflag;
	clan_init[CHAT2CA]		= toCA_init;
	clan_call[CHAT2CA]		= toCA_call;

	clan_main[CHAT2CONLL]	= Chat2Conll_main;
	clan_usage[CHAT2CONLL]	= Chat2Conll_usage;
	clan_getflag[CHAT2CONLL]= Chat2Conll_getflag;
	clan_init[CHAT2CONLL]	= Chat2Conll_init;
	clan_call[CHAT2CONLL]	= Chat2Conll_call;

	clan_main[CHAT2ELAN]	= chat2elan_main;
	clan_usage[CHAT2ELAN]	= chat2elan_usage;
	clan_getflag[CHAT2ELAN]	= chat2elan_getflag;
	clan_init[CHAT2ELAN]	= chat2elan_init;
	clan_call[CHAT2ELAN]	= chat2elan_call;

	clan_main[CHAT2PRAAT]	= chat2praat_main;
	clan_usage[CHAT2PRAAT]	= chat2praat_usage;
	clan_getflag[CHAT2PRAAT]= chat2praat_getflag;
	clan_init[CHAT2PRAAT]	= chat2praat_init;
	clan_call[CHAT2PRAAT]	= chat2praat_call;

	clan_main[CHAT2SRT]		= chat2srt_main;
	clan_usage[CHAT2SRT]	= chat2srt_usage;
	clan_getflag[CHAT2SRT]	= chat2srt_getflag;
	clan_init[CHAT2SRT]		= chat2srt_init;
	clan_call[CHAT2SRT]		= chat2srt_call;

	clan_main[CHAT2XMAR]	= chat2xmar_main;
	clan_usage[CHAT2XMAR]	= chat2xmar_usage;
	clan_getflag[CHAT2XMAR]	= chat2xmar_getflag;
	clan_init[CHAT2XMAR]	= chat2xmar_init;
	clan_call[CHAT2XMAR]	= chat2xmar_call;

	clan_main[CHECK_P]		= check_main;
	clan_usage[CHECK_P] 	= check_usage;
	clan_getflag[CHECK_P]	= check_getflag;
	clan_init[CHECK_P]		= check_init;
	clan_call[CHECK_P]		= check_call;

	clan_main[CONLL2CHAT]	= Conll2Chat_main;
	clan_usage[CONLL2CHAT]	= Conll2Chat_usage;
	clan_getflag[CONLL2CHAT]= Conll2Chat_getflag;
	clan_init[CONLL2CHAT]	= Conll2Chat_init;
	clan_call[CONLL2CHAT]	= Conll2Chat_call;

	clan_main[COMBTIER]		= combtier_main;
	clan_usage[COMBTIER]	= combtier_usage;
	clan_getflag[COMBTIER]	= combtier_getflag;
	clan_init[COMBTIER]		= combtier_init;
	clan_call[COMBTIER]		= combtier_call;

	clan_main[COMPOUND]		= compound_main;
	clan_usage[COMPOUND] 	= compound_usage;
	clan_getflag[COMPOUND]	= compound_getflag;
	clan_init[COMPOUND]		= compound_init;
	clan_call[COMPOUND]		= compound_call;

	clan_main[CP2UTF]		= cptoutf_main;
	clan_usage[CP2UTF] 		= cptoutf_usage;
	clan_getflag[CP2UTF]	= cptoutf_getflag;
	clan_init[CP2UTF]		= cptoutf_init;
	clan_call[CP2UTF]		= cptoutf_call;

	clan_main[DATES]		= dates_main;
	clan_usage[DATES] 		= dates_usage;
	clan_getflag[DATES]		= dates_getflag;
	clan_init[DATES]		= dates_init;
	clan_call[DATES]		= dates_call;

	clan_main[DATACLEANUP]	= dc_main;
	clan_usage[DATACLEANUP]	= dc_usage;
	clan_getflag[DATACLEANUP]= dc_getflag;
	clan_init[DATACLEANUP]	= dc_init;
	clan_call[DATACLEANUP]	= dc_call;

	clan_main[DELIM]		= delim_main;
	clan_usage[DELIM]		= delim_usage;
	clan_getflag[DELIM]		= delim_getflag;
	clan_init[DELIM]		= delim_init;
	clan_call[DELIM]		= delim_call;

	clan_main[ELAN2CHAT]	= elan2chat_main;
	clan_usage[ELAN2CHAT]	= elan2chat_usage;
	clan_getflag[ELAN2CHAT]	= elan2chat_getflag;
	clan_init[ELAN2CHAT]	= elan2chat_init;
	clan_call[ELAN2CHAT]	= elan2chat_call;

	clan_main[FIXBULLETS]	= fixbullets_main;
	clan_usage[FIXBULLETS]	= fixbullets_usage;
	clan_getflag[FIXBULLETS]= fixbullets_getflag;
	clan_init[FIXBULLETS]	= fixbullets_init;
	clan_call[FIXBULLETS]	= fixbullets_call;

	clan_main[FIXIT]		= fixit_main;
	clan_usage[FIXIT] 		= fixit_usage;
	clan_getflag[FIXIT]		= fixit_getflag;
	clan_init[FIXIT]		= fixit_init;
	clan_call[FIXIT]		= fixit_call;

#ifndef _COCOA_APP
	clan_main[FIXMP3]		= fixMP3s_main;
	clan_usage[FIXMP3]		= fixMP3s_usage;
	clan_getflag[FIXMP3]	= fixMP3s_getflag;
	clan_init[FIXMP3]		= fixMP3s_init;
	clan_call[FIXMP3]		= fixMP3s_call;
#endif

	clan_main[FLO]			= flo_main;
	clan_usage[FLO] 		= flo_usage;
	clan_getflag[FLO]		= flo_getflag;
	clan_init[FLO]			= flo_init;
	clan_call[FLO]			= flo_call;

	clan_main[CMDI_P]		= cmdi_main;
	clan_usage[CMDI_P]		= cmdi_usage;
	clan_getflag[CMDI_P]	= cmdi_getflag;
	clan_init[CMDI_P]		= cmdi_init;
	clan_call[CMDI_P]		= cmdi_call;

	clan_main[INDENT]		= indentoverlap_main;
	clan_usage[INDENT]		= indentoverlap_usage;
	clan_getflag[INDENT]	= indentoverlap_getflag;
	clan_init[INDENT]		= indentoverlap_init;
	clan_call[INDENT]		= indentoverlap_call;

	clan_main[LAB2CHAT]		= lab2chat_main;
	clan_usage[LAB2CHAT]	= lab2chat_usage;
	clan_getflag[LAB2CHAT]	= lab2chat_getflag;
	clan_init[LAB2CHAT]		= lab2chat_init;
	clan_call[LAB2CHAT]		= lab2chat_call;

	clan_main[LENA2CHAT]	= lena2chat_main;
	clan_usage[LENA2CHAT]	= lena2chat_usage;
	clan_getflag[LENA2CHAT]	= lena2chat_getflag;
	clan_init[LENA2CHAT]	= lena2chat_init;
	clan_call[LENA2CHAT]	= lena2chat_call;

	clan_main[LIPP2CHAT]	= lipp2chat_main;
	clan_usage[LIPP2CHAT]	= lipp2chat_usage;
	clan_getflag[LIPP2CHAT]	= lipp2chat_getflag;
	clan_init[LIPP2CHAT]	= lipp2chat_init;
	clan_call[LIPP2CHAT]	= lipp2chat_call;

	clan_main[LONGTIER]		= longtier_main;
	clan_usage[LONGTIER] 	= longtier_usage;
	clan_getflag[LONGTIER]	= longtier_getflag;
	clan_init[LONGTIER]		= longtier_init;
	clan_call[LONGTIER]		= longtier_call;

	clan_main[LOWCASE]		= lowcase_main;
	clan_usage[LOWCASE] 	= lowcase_usage;
	clan_getflag[LOWCASE]	= lowcase_getflag;
	clan_init[LOWCASE]		= lowcase_init;
	clan_call[LOWCASE]		= lowcase_call;

	clan_main[MAKEMOD]		= makemod_main;
	clan_usage[MAKEMOD] 	= makemod_usage;
	clan_getflag[MAKEMOD]	= makemod_getflag;
	clan_init[MAKEMOD]		= makemod_init;
	clan_call[MAKEMOD]		= makemod_call;

	clan_main[MEDIALINE]	= medialine_main;
	clan_usage[MEDIALINE]	= medialine_usage;
	clan_getflag[MEDIALINE]= medialine_getflag;
	clan_init[MEDIALINE]	= medialine_init;
	clan_call[MEDIALINE]	= medialine_call;

	clan_main[OLAC_P]		= olac_main;
	clan_usage[OLAC_P]		= olac_usage;
	clan_getflag[OLAC_P]	= olac_getflag;
	clan_init[OLAC_P]		= olac_init;
	clan_call[OLAC_P]		= olac_call;

	clan_main[ORT]			= ort_main;
	clan_usage[ORT] 		= ort_usage;
	clan_getflag[ORT]		= ort_getflag;
	clan_init[ORT]			= ort_init;
	clan_call[ORT]			= ort_call;

	clan_main[PLAY2CHAT]	= play2chat_main;
	clan_usage[PLAY2CHAT]	= play2chat_usage;
	clan_getflag[PLAY2CHAT]	= play2chat_getflag;
	clan_init[PLAY2CHAT]	= play2chat_init;
	clan_call[PLAY2CHAT]	= play2chat_call;

	clan_main[PRAAT2CHAT]	= praat2chat_main;
	clan_usage[PRAAT2CHAT]	= praat2chat_usage;
	clan_getflag[PRAAT2CHAT]= praat2chat_getflag;
	clan_init[PRAAT2CHAT]	= praat2chat_init;
	clan_call[PRAAT2CHAT]	= praat2chat_call;

	clan_main[QUOTES]		= quotes_main;
	clan_usage[QUOTES]		= quotes_usage;
	clan_getflag[QUOTES]	= quotes_getflag;
	clan_init[QUOTES]		= quotes_init;
	clan_call[QUOTES]		= quotes_call;

	clan_main[REPEAT]		= repeat_main;
	clan_usage[REPEAT] 		= repeat_usage;
	clan_getflag[REPEAT]	= repeat_getflag;
	clan_init[REPEAT]		= repeat_init;
	clan_call[REPEAT]		= repeat_call;

	clan_main[RETRACE]		= retrace_main;
	clan_usage[RETRACE] 	= retrace_usage;
	clan_getflag[RETRACE]	= retrace_getflag;
	clan_init[RETRACE]		= retrace_init;
	clan_call[RETRACE]		= retrace_call;

	clan_main[RTF2CHAT]		= rtf2chat_main;
	clan_usage[RTF2CHAT] 	= rtf2chat_usage;
	clan_getflag[RTF2CHAT]	= rtf2chat_getflag;
	clan_init[RTF2CHAT]		= rtf2chat_init;
	clan_call[RTF2CHAT]		= rtf2chat_call;

	clan_main[SALT2CHAT]	= salt2chat_main;
	clan_usage[SALT2CHAT] 	= salt2chat_usage;
	clan_getflag[SALT2CHAT]	= salt2chat_getflag;
	clan_init[SALT2CHAT]	= salt2chat_init;
	clan_call[SALT2CHAT]	= salt2chat_call;

#ifndef _COCOA_APP
	clan_main[SILENCE_P]	= silence_main;
	clan_usage[SILENCE_P]	= silence_usage;
	clan_getflag[SILENCE_P]	= silence_getflag;
	clan_init[SILENCE_P]	= silence_init;
	clan_call[SILENCE_P]	= silence_call;
#endif

	clan_main[SRT2CHAT]		= srt2chat_main;
	clan_usage[SRT2CHAT]	= srt2chat_usage;
	clan_getflag[SRT2CHAT]	= srt2chat_getflag;
	clan_init[SRT2CHAT]		= srt2chat_init;
	clan_call[SRT2CHAT]		= srt2chat_call;

	clan_main[TEXT2CHAT]	= text2chat_main;
	clan_usage[TEXT2CHAT] 	= text2chat_usage;
	clan_getflag[TEXT2CHAT]	= text2chat_getflag;
	clan_init[TEXT2CHAT]	= text2chat_init;
	clan_call[TEXT2CHAT]	= text2chat_call;

	clan_main[TIERORDER]	= tierorder_main;
	clan_usage[TIERORDER] 	= tierorder_usage;
	clan_getflag[TIERORDER]	= tierorder_getflag;
	clan_init[TIERORDER]	= tierorder_init;
	clan_call[TIERORDER]	= tierorder_call;

	clan_main[TRNFIX]		= trnfix_main;
	clan_usage[TRNFIX]		= trnfix_usage;
	clan_getflag[TRNFIX]	= trnfix_getflag;
	clan_init[TRNFIX]		= trnfix_init;
	clan_call[TRNFIX]		= trnfix_call;

	clan_main[UNIQ]			= uniq_main;
	clan_usage[UNIQ]		= uniq_usage;
	clan_getflag[UNIQ]		= uniq_getflag;
	clan_init[UNIQ]			= uniq_init;
	clan_call[UNIQ]			= uniq_call;

	clan_main[USEDLEX]		= usedlex_main;
	clan_usage[USEDLEX]		= usedlex_usage;
	clan_getflag[USEDLEX]	= usedlex_getflag;
	clan_init[USEDLEX]		= usedlex_init;
	clan_call[USEDLEX]		= usedlex_call;

/*
	clan_main[TEMP01]		= _main;
	clan_usage[TEMP01]		= _usage;
	clan_getflag[TEMP01]	= _getflag;
	clan_init[TEMP01]		= _init;
	clan_call[TEMP01]		= _call;

	clan_main[TEMP02]		= _main;
	clan_usage[TEMP02]		= _usage;
	clan_getflag[TEMP02]	= _getflag;
	clan_init[TEMP02]		= _init;
	clan_call[TEMP02]		= _call;

	clan_main[TEMP03]		= _main;
	clan_usage[TEMP03]		= _usage;
	clan_getflag[TEMP03]	= _getflag;
	clan_init[TEMP03]		= _init;
	clan_call[TEMP03]		= _call;

	clan_main[TEMP04]		= _main;
	clan_usage[TEMP04]		= _usage;
	clan_getflag[TEMP04]	= _getflag;
	clan_init[TEMP04]		= _init;
	clan_call[TEMP04]		= _call;

	clan_main[TEMP05]		= _main;
	clan_usage[TEMP05]		= _usage;
	clan_getflag[TEMP05]	= _getflag;
	clan_init[TEMP05]		= _init;
	clan_call[TEMP05]		= _call;

	clan_main[TEMP06]		= _main;
	clan_usage[TEMP06]		= _usage;
	clan_getflag[TEMP06]	= _getflag;
	clan_init[TEMP06]		= _init;
	clan_call[TEMP06]		= _call;

	clan_main[TEMP07]		= _main;
	clan_usage[TEMP07]		= _usage;
	clan_getflag[TEMP07]	= _getflag;
	clan_init[TEMP07]		= _init;
	clan_call[TEMP07]		= _call;

	clan_main[TEMP08]		= _main;
	clan_usage[TEMP08]		= _usage;
	clan_getflag[TEMP08]	= _getflag;
	clan_init[TEMP08]		= _init;
	clan_call[TEMP08]		= _call;

	clan_main[TEMP09]		= _main;
	clan_usage[TEMP09]		= _usage;
	clan_getflag[TEMP09]	= _getflag;
	clan_init[TEMP09]		= _init;
	clan_call[TEMP09]		= _call;

	clan_main[TEMP10]		= _main;
	clan_usage[TEMP10]		= _usage;
	clan_getflag[TEMP10]	= _getflag;
	clan_init[TEMP10]		= _init;
	clan_call[TEMP10]		= _call;

*/
	clan_main[TEMP]			= temp_main;
	clan_usage[TEMP]		= temp_usage;
	clan_getflag[TEMP]		= temp_getflag;
	clan_init[TEMP]			= temp_init;
	clan_call[TEMP]			= temp_call;

	clan_main[DOS2UNIX]		= d2u_main;
	clan_usage[DOS2UNIX] 	= d2u_usage;
	clan_getflag[DOS2UNIX]	= d2u_getflag;
	clan_init[DOS2UNIX]		= d2u_init;
	clan_call[DOS2UNIX]		= d2u_call;

	clan_main[GPS]			= gps_main;
	clan_usage[GPS]			= gps_usage;
	clan_getflag[GPS]		= gps_getflag;
	clan_init[GPS]			= gps_init;
	clan_call[GPS]			= gps_call;

	clan_main[LINES_P]		= lines_main;
	clan_usage[LINES_P]		= lines_usage;
	clan_getflag[LINES_P]	= lines_getflag;
	clan_init[LINES_P]		= lines_init;
	clan_call[LINES_P]		= lines_call;

	clan_main[PP]			= pp_main;
	clan_usage[PP]			= pp_usage;
	clan_getflag[PP]		= pp_getflag;
	clan_init[PP]			= pp_init;
	clan_call[PP]			= pp_call;


	clan_name[CHAINS]		= "chains";
	clan_name[CHIP]			= "chip";
	clan_name[C_NNLA]		= "c-nnla";
	clan_name[C_QPA]		= "c-qpa";
	clan_name[CODES_P]		= "codes";
	clan_name[COMBO]		= "combo";
	clan_name[COMPOUND]		= "compound";
	clan_name[COOCCUR]		= "cooccur";
	clan_name[CORELEX]		= "corelex";
	clan_name[DATES]		= "dates";
	clan_name[DIST]			= "dist";
	clan_name[DSS]			= "dss";
	clan_name[EVAL]			= "eval";
	clan_name[FLUCALC]		= "flucalc";
	clan_name[FREQ]			= "freq";
	clan_name[FREQPOS]		= "freqpos";
	clan_name[GEM]			= "gem";
	clan_name[GEMFREQ]		= "gemfreq";
	clan_name[GEMLIST]		= "gemlist";
	clan_name[IPSYN]		= "ipsyn";
	clan_name[KEYMAP]		= "keymap";
	clan_name[KIDEVAL]		= "kideval";
	clan_name[KWAL]			= "kwal";
	clan_name[MAXWD]		= "maxwd";
	clan_name[MLU]			= "mlu";
	clan_name[MLT]			= "mlt";
	clan_name[MODREP]		= "modrep";
	clan_name[MORTABLE]		= "mortable";
	clan_name[PHONFREQ]		= "phonfreq";
	clan_name[RELY]			= "rely";
	clan_name[SCRIPT_P]		= "script";
	clan_name[SUGAR]		= "sugar";
	clan_name[TIMEDUR]		= "timedur";
	clan_name[VOCD]			= "vocd";
	clan_name[WDLEN]		= "wdlen";


	clan_name[MEGRASP]		= "megrasp";
	clan_name[MOR_P]		= "mor";
	clan_name[POST]			= "post";
	clan_name[POSTLIST]		= "postlist";
	clan_name[POSTMODRULES]	= "postmodrules";
	clan_name[POSTMORTEM]	= "postmortem";
	clan_name[POSTTRAIN]	= "posttrain";


	clan_name[CHSTRING]		= "chstring";
	clan_name[ANVIL2CHAT]	= "anvil2chat";
	clan_name[CHAT2ANVIL]	= "chat2anvil";
	clan_name[CHAT2CA]		= "chat2ca";
	clan_name[CHAT2CONLL]	= "chat2conll";
	clan_name[CHAT2ELAN]	= "chat2elan";
	clan_name[CHAT2PRAAT]	= "chat2praat";
	clan_name[CHAT2SRT]		= "chat2srt";
	clan_name[CHAT2XMAR]	= "chat2xmar";
	clan_name[CHECK_P]		= "check";
	clan_name[CONLL2CHAT]	= "conll2chat";
	clan_name[COMBTIER]		= "combtier";
	clan_name[CP2UTF]		= "cp2utf";
	clan_name[DATACLEANUP]	= "dataclean";
	clan_name[DELIM]		= "delim";
	clan_name[ELAN2CHAT]	= "elan2chat";
	clan_name[FIXBULLETS]	= "fixbullets";
	clan_name[FIXIT]		= "fixit";
#ifndef _COCOA_APP
	clan_name[FIXMP3]		= "fixmp3s";
#endif
	clan_name[FLO]			= "flo";
	clan_name[CMDI_P]		= "cmdi";
	clan_name[INDENT]		= "indent";
	clan_name[LAB2CHAT]		= "lab2chat";
	clan_name[LENA2CHAT]	= "lena2chat";
	clan_name[LIPP2CHAT]	= "lipp2chat";
	clan_name[LONGTIER]		= "longtier";
	clan_name[LOWCASE]		= "lowcase";
	clan_name[MAKEMOD]		= "makemod";
	clan_name[MEDIALINE]	= "medialine";
	clan_name[OLAC_P]		= "olac";
	clan_name[ORT]			= "ort";
	clan_name[PLAY2CHAT]	= "play2chat";
	clan_name[PRAAT2CHAT]	= "praat2chat";
	clan_name[QUOTES]		= "quotes";
	clan_name[REPEAT]		= "repeat";
	clan_name[RETRACE]		= "retrace";
	clan_name[RTF2CHAT]		= "rtf2chat";
	clan_name[SALT2CHAT]	= "salt2chat";
#ifndef _COCOA_APP
	clan_name[SILENCE_P]	= "silence";
#endif
	clan_name[SRT2CHAT]		= "srt2chat";
	clan_name[TEXT2CHAT]	= "text2chat";
	clan_name[TIERORDER]	= "tierorder";
	clan_name[TRNFIX]		= "trnfix";
	clan_name[UNIQ]			= "uniq";
	clan_name[USEDLEX]		= "usedlex";


	clan_name[TEMPLATE]		= "";
	clan_name[TEMP]			= "temp";


	clan_name[DOS2UNIX]		= "dos2unix";
	clan_name[GPS]			= "gps";
	clan_name[LINES_P]		= "lines";
	clan_name[PP]			= "pp";
}

int get_clan_prog_num(char *s, char isLoad) {
	int i;
	
	func_init();

	strcpy(templineC,s);
	uS.lowercasestr(templineC, &dFnt, FALSE);

	for (i=0;i < LAST_CLAN_PROG;i++) {
		if (strcmp(templineC, clan_name[i]) == 0) {
			if (clan_main[i])
				return(i);
			else
				return(-1);
		}
	}
	return(-1);
}	

extern void main_clan(int argc, char *argv[]);
void main_clan(int argc, char *argv[]) {
	int n;
	int jmp_return;
#ifdef _MAC_CODE
#ifndef _COCOA_APP
	GrafPtr savePort;
#endif
#endif
	func_init();

	if (argc && *argv[0]) {
		n = get_clan_prog_num(argv[0], TRUE);
		if (n >= 0) {
		 	CLAN_PROG_NUM = n;
#ifdef _MAC_CODE
#ifndef _COCOA_APP
			DrawMouseCursor(2);
			GetPort(&savePort);
#endif
#endif
//			expandedArgv = NULL;
			skip_prog = -1;

			jmp_return = setjmp(my_jmp_buf);
			if (jmp_return != 0) {
			} else if (skip_prog <= 0) {
				isKillProgram = 0;
				(*clan_main[CLAN_PROG_NUM])(argc, argv);
			}
			my_flush_chr();
			if (isKillProgram && isKillProgram != 2) {
				isKillProgram = 0;			
				fprintf(stderr, "\n\nProgram \"%s\" aborted by the user.\n", argv[0]);
				isKillProgram = 1;			
			}
/*
 			if (expandedArgv != NULL) {
				free(expandedArgv);
				expandedArgv = NULL;
			}
*/
#ifdef _MAC_CODE
#ifndef _COCOA_APP
			SetPort(savePort);
#endif
#endif
			skip_prog = 0;
#ifndef _COCOA_APP
			freeXML_Elements();
#endif
		} else {
		 	fprintf(stderr,"Unknown command: %s\n",argv[0]);
			show_info(1);
		}
	}
}

#if !defined(UNX)
void myjmp(int jval) {
	if (jval > 0) {
#ifndef _COCOA_APP
		Quit_CED(jval);
#endif
	} else if (jval < 0) {
		isKillProgram = 2;
	} else {
		skip_prog = 1;
		longjmp(my_jmp_buf, jval);
	}
}

static void pretty_list(const char *s_list[], int num, int maxcols) {
	register int i, k, ncols;
	int longest_name = 0;
	BOOL isCRAdded;

	for (i=0; i < num; i++) {
		if ((k=strlen(s_list[i])) > longest_name)
			longest_name = k;
	}

	ncols = maxcols / (longest_name + 3);
	if (!ncols)
		ncols = 1;

	isCRAdded = false;
	k = 0;
	for (i=0; i < num; i++) {
		if (!(++k % ncols)) {
			isCRAdded = true;
			fprintf(stdout,"%-*s\n",longest_name,s_list[i]);
		} else {
			isCRAdded = false;
			fprintf(stdout,"%-*s  ",longest_name,s_list[i]);
		}
	}
	if (isCRAdded == false)
		fprintf(stdout,"\n");
}

static int insertSorted(const char *temp[], int k, const char *newName) {
	int i, j;

	for (i=0; i < k; i++) {
		if (strcmp(temp[i], newName) == 0)
			return(k);
		else if (strcmp(temp[i], newName) > 0)
			break;
	}
	for (j=k; j > i; j--)
		temp[j] = temp[j-1];
	temp[i] = newName;
	return(k+1);		
}

static char isDisplayInInfo(int i, char isAll) {
	if (isAll == 2)
		return(TRUE);
	if (i == CMDI_P || i == OLAC_P || i == UNIQ ||
		i == COMPOUND  || i == DATACLEANUP || i == FIXMP3 || i == ORT || i == PHONFREQ || i == USEDLEX) {
		return(FALSE);
	}
	return(TRUE);
}

void ListAvailable(char isAll) {
	int i, k;
	const char *temp[512];

	func_init();

	fprintf(stdout,"\nYou can run any of the following CLAN analysis commands:\n");
	for (i=k=0; i < C_NNLA; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
		   k = insertSorted(temp, k, clan_name[i]);
	}
	pretty_list(temp,k,80);

	fprintf(stdout,"\nAny of these commands for computing profiles:\n");
	for (i=C_NNLA, k=0; i < MEGRASP; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
			k = insertSorted(temp, k, clan_name[i]);
	}
	pretty_list(temp,k,80);

	fprintf(stdout,"\nAny of these commands for morphosyntactic coding:\n");
	for (i=MEGRASP, k=0; i < ANVIL2CHAT; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
			k = insertSorted(temp, k, clan_name[i]);
	}
	pretty_list(temp,k,80);

	fprintf(stdout,"\nAny of these format conversion commands:\n");
	for (i=ANVIL2CHAT, k=0; i < CHSTRING; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
			k = insertSorted(temp, k, clan_name[i]);
	}
	k = insertSorted(temp, k, "chat2text");
	pretty_list(temp,k,80);

	fprintf(stdout,"\nAny of these reformatting commands:\n");
	for (i=CHSTRING, k=0; i < COMBTIER; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
			k = insertSorted(temp, k, clan_name[i]);
	}
	k = insertSorted(temp, k, "trim");
	pretty_list(temp,k,80);

	fprintf(stdout,"\nOr any of these format repair commands\n");
	for (i=COMBTIER, k=0; i < TEMP; i++) {
		if (clan_main[i] && clan_name[i][0] != EOS && isDisplayInInfo(i, isAll))
			k = insertSorted(temp, k, clan_name[i]);
	}
	pretty_list(temp,k,80);

	if (isAll == 2) {
		fprintf(stdout,"\nOr any of the following extra utility commands:\n");
		for (i=TEMP, k=0; i < LAST_CLAN_PROG; i++) {
			if (clan_main[i] && clan_name[i][0] != EOS)
			   k = insertSorted(temp, k, clan_name[i]);
		}
		pretty_list(temp,k,80);
	}
}
#endif
