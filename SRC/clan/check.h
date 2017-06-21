/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef CHECKDEF
#define CHECKDEF

extern "C"
{

#define CHECK_MORPHS "-#~"

#define NUMSPCHARS 55 // up-arrow

#define isPostCodeMark(x,y) ((x == '+' || x == '-') && y == ' ')

#define DEPFILE   "depfile.cut"
//#define DEPADD    "00depadd.cut" // 16-10-02
#define UTTDELSYM "[UTD]"
#define NUTTDELSYM "[-UTD]"
#define SUFFIXSYM "[AFX]" // is affix matches don't check the rest, if no affix -> do normal check
#define UTTIGNORE "[IGN]"
#define WORDCKECKSYM "[NOWORDCHECK]"
#define UPREFSSYM "[UPREFS *]"
#define DEPENDENT "@Dependent"
#define AGEOF     "@Age of *"
#define SEXOF	  "@Sex of *"
#define SESOF	  "@Ses of *"
#define BIRTHOF   "@Birth of *"
#define EDUCOF    "@Education of *"
#define GROUPOF   "@Group of *"
#define IDOF      "@ID"
#define DATEOF	  "@Date"
#define PARTICIPANTS "@Participants"

#define ALNUM 0x1
#define ALPHA 0x2
#define DIGIT 0x3
#define OR_CH 0x4
#define PARAO 0x5
#define PARAC 0x6
#define ONE_T 0x7
#define ZEROT 0x8

enum { // CA CHARS
	CA_NOT_ALL = 0,
	CA_APPLY_SHIFT_TO_HIGH_PITCH,	// up-arrow
	CA_APPLY_SHIFT_TO_LOW_PITCH,	// down-arrow
	CA_APPLY_RISETOHIGH,			// rise to high
	CA_APPLY_RISETOMID,				// rise to mid
	CA_APPLY_LEVEL,					// level
	CA_APPLY_FALLTOMID,				// fall to mid
	CA_APPLY_FALLTOLOW,				// fall to low
	CA_APPLY_UNMARKEDENDING,		// unmarked ending
	CA_APPLY_CONTINUATION,			// continuation
	CA_APPLY_INHALATION,			// inhalation - raised period
	CA_APPLY_LATCHING,				// latching
	CA_APPLY_UPTAKE,				// uptake
	CA_APPLY_OPTOPSQ,				// raised [
	CA_APPLY_OPBOTSQ,				// lowered [
	CA_APPLY_CLTOPSQ,				// raised ]
	CA_APPLY_CLBOTSQ,				// lowered ]
	CA_APPLY_FASTER,				// faster
	CA_APPLY_SLOWER,				// slower
	CA_APPLY_CREAKY,				// creaky
	CA_APPLY_UNSURE,				// unsure
	CA_APPLY_SOFTER,				// softer
	CA_APPLY_LOUDER,				// louder
	CA_APPLY_LOW_PITCH,				// low pitch - low bar
	CA_APPLY_HIGH_PITCH,			// high pitch - high bar
	CA_APPLY_SMILE_VOICE,			// smile voice - pound sterling sign
	CA_BREATHY_VOICE,				// breathy-voice -♋-
	CA_APPLY_WHISPER,				// whisper
	CA_APPLY_YAWN,					// yawn
	CA_APPLY_SINGING,				// singing
	CA_APPLY_PRECISE,				// precise
	CA_APPLY_CONSTRICTION,			// constriction
	CA_APPLY_PITCH_RESET,			// pitch reset
	CA_APPLY_LAUGHINWORD,			// laugh in a word
	NOTCA_VOCATIVE,					// Vocative or summons - ‡ - NOT CA
	NOTCA_ARABIC_DOT_DIACRITIC,		// Arabic dot diacritic - NOT CA
	NOTCA_ARABIC_RAISED,			// Arabic raised h - NOT CA
	NOTCA_STRESS,					// Stress - ̄ - NOT CA
	NOTCA_GLOTTAL_STOP,				// Glottal stop - ʔ - NOT CA
	NOTCA_HEBREW_GLOTTAL,			// Hebrew glottal - ʕ - NOT CA
	NOTCA_CARON,					// caron - ̌- NOT CA
	NOTCA_GROUP_START,				// Group start marker - NOT CA
	NOTCA_GROUP_END,				// Group end marker - NOT CA
	NOTCA_DOUBLE_COMMA,				// Tag or sentence final particle; „ - NOT CA
	NOTCA_RAISED_STROKE,			// raised stroke - NOT CA
	NOTCA_LOWERED_STROKE,			// lowered stroke - NOT CA
	NOTCA_SIGN_GROUP_START,			// sign group start marker - NOT CA
	NOTCA_SIGN_GROUP_END,			// sign group end marker - NOT CA
	NOTCA_UNDERLINE,				// underline - NOT CA
	NOTCA_MISSINGWORD,				// %pho missing word -…- NOT CA
	NOTCA_OPEN_QUOTE,				// open quote “ - NOT CA 0x201C, E2 80 9C
	NOTCA_CLOSE_QUOTE,				// close quote ” - NOT CA 0x201D, E2 80 9D
	NOTCA_OPEN_S_QUOTE,				// open small quote ‘ - NOT CA 0x2018, E2 80 98
	NOTCA_CLOSE_S_QUOTE,			// close small quote ’ - NOT CA 0x2019, E2 80 99
	NOTCA_CROSSED_EQUAL,			// crossed equal ≠ - NOT CA 0x2260, E2 89 A0
	NOTCA_LEFT_ARROW_CIRCLE			// left arrow with circle ↫ - NOT CA 0x21AB, E2 86 AB
} ;

extern const char *lHeaders[];

}

#endif /* CHECKDEF */
