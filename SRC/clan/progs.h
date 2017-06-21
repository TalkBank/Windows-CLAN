/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* constants for all the programs */
#ifndef PROGDEF
#define PROGDEF

enum {
	CHAINS,
	CHIP,
	COMBO,
	COOCCUR,
	DIST,
	DSS,
	EVAL,
	FREQ,
	FREQMERGE,
	FREQPOS,
	GEM,
	GEMFREQ,
	GEMLIST,
	IPSYN,
	KEYMAP,
	KIDEVAL,
	KWAL,
	MAXWD,
	MLT,
	MLU,
	MODREP,
	MORTABLE,
	PHONFREQ,
	RELY,
	SCRIPT_P,
	TIMEDUR,
	VOCD,
	WDLEN,

	MEGRASP,
	MOR_P,
	POST,
	POSTLIST,
	POSTMODRULES,
	POSTMORTEM,
	POSTTRAIN,

	CHSTRING,
	ANVIL2CHAT,
	CHAT2ANVIL,
	CHAT2CA,
	CHAT2ELAN,
	CHAT2PRAAT,
	CHAT2XMAR,
	CHECK,
	COMBTIER,
	COMPOUND,
	CP2UTF,
	DATES,
	DATACLEANUP,
	DELIM,
	ELAN2CHAT,
	FIXBULLETS,
	FIXIT,
	FIXLANG,
	FIXMP3,
	FLO,
	IMDI_P,
	INDENT,
	INSERT,
	JOINITEMS,
	LAB2CHAT,
	LIPP2CHAT,
	LONGTIER,
	LOWCASE,
	MAKEMOD,
	OLAC_P,
	ORT,
	PRAAT2CHAT,
	QUOTES,
	REPEAT,
	RETRACE,
	RTFIN,
	SALTIN,
	SILENCE_P,
	SPREADSH,
	SUBTITLES,
	SYNCODING,
	TEXTIN,
	TIERORDER,
	TRNFIX,
	UNIQ,

	TEMP01,
	TEMP02,
	TEMP03,
	TEMP04,
	TEMP05,
	TEMP06,
	TEMP07,
	TEMP08,
	TEMP09,
	TEMP10,

	TEMP,
	DOS2UNIX,
	FIXCA,
	FIXOVLPSYM,
	GPS,
	LINES_P,
	PP,
	LAST_CLAN_PROG
};

#define D_OPTION 1L
#define F_OPTION  (1L << 1)
#define K_OPTION  (1L << 2)
#define O_OPTION  (1L << 3)
#define P_OPTION  (1L << 4)
#define R_OPTION  (1L << 5)
#define SP_OPTION (1L << 6)
#define SM_OPTION (1L << 7)
#define T_OPTION  (1L << 8)
#define UP_OPTION (1L << 9)
#define UM_OPTION (1L << 10)
#define W_OPTION  (1L << 11)
#define Y_OPTION  (1L << 12)
#define Z_OPTION  (1L << 13)
#define FR_OPTION  (1L << 14)
#define RE_OPTION (1L << 15)
#define L_OPTION (1L << 16)

extern int CLAN_PROG_NUM;

#endif /* #ifndef PROGDEF */
