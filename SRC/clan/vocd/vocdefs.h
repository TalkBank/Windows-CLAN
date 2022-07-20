/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 * vocdefs.h                                       * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/

#ifndef __VOCD_DEFS__
#define __VOCD_DEFS__

extern "C"
{

#define MAX_LINE     2000       /* maximum allowed chars in a line */
#define COLON        0x01       /* used for replacement '[: text]' */
#define REMOVE_INC   0x01       /* remove contents of parentheses  */
#define MAX_INP_FILES 512       /* max no of input                 */
#define MAX_FILES      10       /* max no of include/exclude files */
#define CMD_LINE_MAX  1024       /* max no of chars in the expanded
                                   command line - for summarising
								   the command line in the output  */
#define MAX_CMDNAME   100       /* max length of command name      */ 
#define MAX_EXT        10       /* max len of o/p file extension   */
#define MAX_POSTCODES  10       /* maximum number of postcodes
                                   allowed in the command line     */
#define RANDOM_TTR     0x02     /* use by interactive VOCD         */
#define NOREPLACEMENT  0x04     /* sampling without replacement    */
#define POOLED_FNAME   "pooled" /* name of outfile for +u option   */
//#define MAX_LEN_CODE 1024
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	#define fgets fgets_cr
#endif

#define Line myLine
struct LineS {
   long			 lineno;
   int           no_words;      /* number of words in the line     */
   char        **words;         /* the list of words, in sequence  */
   struct LineS  *nextline;      /* pointer to the next line        */
};
typedef struct LineS Line;
/*
struct tierS {
   char          code[MAX_LEN_CODE];	// null terminated
   struct tierS * next;          		// next tier
};
typedef struct tierS Tier;
*/
struct  Speaker_type {
	char *fname;
	char *IDs;
	char *code;  /* null terminated speaker code     */
	char isUsed;
	Line *firstline;           /* first line                       */
	Line *lastline;            /* last line in the list            */
	double denom_d_optimum;
	double d_optimum;
};
typedef struct Speaker_type VOCDSP;

/* list of speakers */
struct Speakers_s {
   char				 isTemp;
   VOCDSP			 *speaker; /* a speaker    */
   struct Speakers_s *next;    /* next speaker */
};
typedef struct Speakers_s VOCDSPs;

/* Structures for collecting main and dependent tier information
struct Code {
   char code[MAX_LEN_CODE]; // to holds the tier code
   struct Code *next;
};
typedef struct Code Code;
*/
/* Not all of the functionality implied by the above is included */
 
/* D_OPTIMUM data structures/definitions  */
#define MAX_TRIALS      3       /* number of trials to compute
                                   D_optimum                       */
typedef struct NT_Values {
   int     N;     /* number of tokens */
   int     S;     /* number of segments*/
   double  T;     /* average ttr value */
   double SD;     /* std dev. for ttr  */
   double  D;     /* value of D calculated from equation */
} NT_Values;

typedef struct D_Values {
   NT_Values * nt_tuples;   /* set of nt values    */
   int         no_tuples;   /* number of nt values */
   double      D;           /* average value of D  */
   double      STD;         /* standard dev.       */
} D_Values;

/* Struct to hold Dmin and Minimum least square values for calc. d */
struct DMIN_Values {
   double dmin;          /* minimum estimated D value  */
   double mls;           /* minimum least square value */
};
typedef struct DMIN_Values DMIN_Values;

	extern char VOCD_TYPE_D;
	extern char isDoVocdAnalises;

	extern void init_vocd(char first);
	extern char vocd_analize(char isOutput);

}

#endif // __VOCD_DEFS__
