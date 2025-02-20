/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* constants for all the programs */
#ifndef PROGDEF
#define PROGDEF

enum {
	CHAINS,
	CHECK_P,
	CHIP,
	CHIPUTIL,
	CODES_P,
	COMBO,
	COOCCUR,
	DIST,
	FREQ,
	FREQPOS,
	GEM,
	GEMFREQ,
	GEMLIST,
	KEYMAP,
	KWAL,
	MAXWD,
	MLT,
	MLU,
	MLUMOR,
	MODREP,
	RELY,
	SCRIPT_P,
	TIMEDUR,
	VOCD,
	WDLEN,

	C_NNLA,
	C_QPA,
	CORELEX,
	DSS,
	EVAL,
	EVALD,
	FLUCALC,
	FLUPOS,
	IPSYN,
	KIDEVAL,
	MORTABLE,
	SUGAR,

	MEGRASP,
	MOR_P,
	POST,
	POSTLIST,
	POSTMODRULES,
	POSTMORTEM,
	POSTTRAIN,
	TRNFIX,

	ANVIL2CHAT,
	CHAT2ANVIL,
	CHAT2CA,
	CHAT2ELAN,
	CHAT2PRAAT,
	CHAT2SRT,
	CHAT2XMAR,
	ELAN2CHAT,
	LAB2CHAT,
	LENA2CHAT,
	LIPP2CHAT,
	PLAY2CHAT,
	PRAAT2CHAT,
	RTF2CHAT,
	SALT2CHAT,
	SRT2CHAT,
	TANG2CHAT,
	TEXT2CHAT,

	CHSTRING,
	COMPOUND,
	CONVORT,
	DATES,
	DATACLEANUP,
	FIXMP3,
	FLO,
	INDENT,
	LINES_P,
	LONGTIER,
	MAKEMOD,
	MEDIALINE,
	MERGE,
	NUM2WORD,
	ORT,
	PHONFREQ,
	PID_P,
	REPEAT,
	RETRACE,
	ROLES,
	SEGMENT,
	TIERORDER,
	UNIQ,
	USEDLEX,
	VALIDATEMFA,

	COMBTIER,
	CP2UTF,
	DELIM,
	FIXBULLETS,
	FIXIT,
	LOWCASE,
	QUOTES,
	SILENCE_P,
#ifdef _MAC_CODE
	SLICE_P,
#endif

	TEMPLATE,
	TEMP,

	DOS2UNIX,
	GPS,
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
#define J_OPTION (1L << 17)

extern int CLAN_PROG_NUM;

#endif /* #ifndef PROGDEF */
