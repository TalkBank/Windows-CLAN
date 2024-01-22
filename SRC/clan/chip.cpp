/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

/* CHIP: a computer program for the automatic coding and analysis		*/
/* of parent-child conversational interaction.  The program process		*/
/* CHAT files adding interactional information as described in the		*/
/* manual entry.														*/

/* Designed and implemented by Jeff Sokolov (1988) with help from		*/
/* Bruce Adams (basic data structures for utterances and words) and		*/
/* Leonid Spektor (many utility routines and much general advice).		*/

/* Last updated for the PC and morphological analyses by Jeff Sokolov.  */
/* Prototyped to pass a gcc compiler (04/04/90) Darius Clynes.			*/
/* Previous version whipped into shape by Jeff and Leonid (11/12/90).	*/
/* Present version modified to utilize utterances rather than responses */
/* to be more consistent with the child language literature.  Also		*/
/* changed default uttwindow to be six also to be consistent with the	*/
/* literature (5/91).                                                   */

/* INCLUDE THE FOLLOWING FILES                                          */
#include "cu.h"		/* All the normal includes are in here				*/
#include <math.h>	/* Floating point stuff (DC)						*/
#include "c_curses.h"
#include "chip.h"

#if !defined(UNX)
#define _main chip_main
#define call chip_call
#define getflag chip_getflag
#define init chip_init
#define usage chip_usage
#endif

#include "mul.h"
#define root chip_root
#define IS_WIN_MODE FALSE

/* GLOBAL DEFINITIONS */
#define MAX_WORDS_PER_CLASS 100
#define null_string(s) (*(s) == EOS)	/* Is this string empty?		*/
#define MAXUTTWINDOW 7

int chip_done;

/* DEFAULT VALUES FOR PROGRAM OPTIONS */
int DOING_ADU = TRUE;		/* Doing adult responses			*/
int DOING_CHI = TRUE;		/* Doing child responses			*/
int DOING_SLF = TRUE;		/* Doing self-repetitions			*/
int DOING_OTH = FALSE;		/* Doing responses for unknown speakers		*/
int UTTWINDOW;			/* Default for the utterance window		*/
double MININDEX = 0.0;		/* Minimum repetition index			*/
int DOING_CLASS = FALSE;	/* TRUE if wordclass include file specified */
int DOING_SUBST = FALSE;	/* TRUE if substitutions are to be coded	*/
char isMorTier;
static char ftime;
static char *codes = NULL;		/* Array for storing interactional codes  */
static char *repstring = NULL;	/* Array for storing interactional codes  */
static char *dist = NULL;		/* Array for storing distance codes  */
static real_utterance *utters[MAXUTTWINDOW];

/* GLOBAL INTERACTIONAL INFORMATION */
int TOTAL_UTTER_NUM = 0;		/* Total number of utterances		*/

struct inter_data
  adu_dat = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
			 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0},
  chi_dat = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
			 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0},
  asr_dat = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
			 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0},
  csr_dat = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
			 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0};


/*  *************** chip prototypes **************** */
void usage(void)	/* Print proper usage and exit.  (Called by CLAN.)  */
{
#ifdef UNX
	printf("CHIP NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN CHIP ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cCHIP NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN CHIP ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	printf("CHIP codes conversational interaction between adults and children.\n");
	printf("Usage: chip [cS bS g hF nC xN qN %s] filename(s)\n", mainflgs());
	printf("+bS: Specify that speaker ID S is an adult.\n");
	printf("+cS: Specify that speaker ID S is a child.\n");
	printf("+g : Enable the substitution option.\n");
//	puts(  "+hF: file F has words to be included");
	puts(  "-hF: file F has words to be excluded");
	printf("-nC: Do not code C: b (adu), c (chi), or s (asr & csr) responses.\n");
	printf("+qN: Set the utterance window to N utterances before the response (2-%d).\n", MAXUTTWINDOW);
	printf("+xN: Set minimum repetition index for coding.\n");
	mainusage(FALSE);
#ifdef UNX
	printf("CHIP NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN CHIP ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cCHIP NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN CHIP ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

static void clean_all(void) {
	int i;	/* Index to current utterance in UTTWINDOW		*/

	for (i=0; i < MAXUTTWINDOW; i++) {
		if (utters[i] != NULL)
			free_utter(utters[i]);
		utters[i] = NULL;
	}
	if (dist != NULL)
		free(dist);
	if (codes != NULL)
		free(codes);
	if (repstring != NULL)
		free(repstring);	
	dist = NULL;
	codes = NULL;
	repstring = NULL;
}

void chip_exit(const char *err, char isQuit) {
	clean_all();
	if (isQuit) {
		if (err != NULL)
			fprintf(stderr,"Memory allocation error: %s\n",err);
		cutt_exit(0);
	}
}

void init(char first) {
	int i;
	extern int number_of_children;
	extern int number_of_adults;

	if (first) {
		addword('\0','\0',"+qqq");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+xx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+yy");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		// defined in "mmaininit" and "globinit" - nomap = TRUE;
		maininitwords();
		mor_initwords();
		ftime = TRUE;
		isMorTier = TRUE;
		UTTWINDOW = 6; /* the response plus 5 previous utterances */
		DOING_ADU = TRUE;		/* Doing adult responses					*/
		DOING_CHI = TRUE;		/* Doing child responses					*/
		DOING_SLF = TRUE;		/* Doing self-repetitions					*/
		DOING_OTH = FALSE;		/* Doing unknown speakers					*/
		MININDEX = 0.0;			/* Minimum repetition index					*/
		DOING_CLASS = FALSE;	/* TRUE if wordclass include file specified */
		DOING_SUBST = FALSE;	/* TRUE if substitutions are to be coded	*/
		number_of_children = 0;
		number_of_adults = 0;
		for (i=0; i < MAXUTTWINDOW; i++)
			utters[i] = NULL;
		codes = NULL;		/* Array for storing interactional codes  */
		repstring = NULL;	/* Array for storing interactional codes  */
		dist = NULL;		/* Array for storing distance codes  */
	} else {
		if (ftime) {
			ftime = FALSE;
//			OverWriteFile = FALSE;
			if (onlydata == 3) {
				stout = FALSE;
				AddCEXExtension = ".xls";
			}
			if (isMorTier) {
				maketierchoice("%mor",'+',FALSE);
			} else {
				strcat(GlobalPunctuation, ":");
				if (DOING_SUBST && !isMorTier) {
					fprintf(stderr,"The substitution option (+g) can't be used on main speaker tiers.\n");
					chip_exit(NULL, TRUE);
				}
			}
		}
	}

	if (first || !combinput) {

		TOTAL_UTTER_NUM = 0;	/* Total number of utterances				*/

		adu_dat.rep_index = chi_dat.rep_index =
			asr_dat.rep_index = csr_dat.rep_index = 0.0;

		adu_dat.dist = chi_dat.dist =
			asr_dat.dist = csr_dat.dist = 0;

		adu_dat.utters = chi_dat.utters =
			asr_dat.utters = csr_dat.utters = 0.0;

		adu_dat.responses = chi_dat.responses =
			asr_dat.responses = csr_dat.responses = 0.0;

		adu_dat.clresponses = chi_dat.clresponses =
			asr_dat.clresponses = csr_dat.clresponses = 0.0;

		adu_dat.no_overlap = chi_dat.no_overlap =
			asr_dat.no_overlap = csr_dat.no_overlap = 0.0;

		adu_dat.add_ops = chi_dat.add_ops =
			asr_dat.add_ops = csr_dat.add_ops = 0.0;

		adu_dat.del_ops = chi_dat.del_ops =
			asr_dat.del_ops = csr_dat.del_ops = 0.0;

		adu_dat.exa_ops = chi_dat.exa_ops =
			asr_dat.exa_ops = csr_dat.exa_ops = 0.0;

		adu_dat.sub_ops = chi_dat.sub_ops =
			asr_dat.sub_ops = csr_dat.sub_ops = 0.0;

		adu_dat.add_words = chi_dat.add_words =
			asr_dat.add_words = csr_dat.add_words = 0.0;

		adu_dat.del_words = chi_dat.del_words =
			asr_dat.del_words = csr_dat.del_words = 0.0;

		adu_dat.exa_words = chi_dat.exa_words =
			asr_dat.exa_words = csr_dat.exa_words = 0.0;

		adu_dat.sub_words = chi_dat.sub_words =
			asr_dat.sub_words = csr_dat.sub_words = 0.0;

		adu_dat.fro_words = chi_dat.fro_words =
			asr_dat.fro_words = csr_dat.fro_words = 0.0;

		adu_dat.madd = chi_dat.madd =
			asr_dat.madd = csr_dat.madd = 0.0;

		adu_dat.mdel = chi_dat.mdel =
			asr_dat.mdel = csr_dat.mdel = 0.0;

		adu_dat.mexa = chi_dat.mexa =
			asr_dat.mexa = csr_dat.mexa = 0.0;

		adu_dat.msub = chi_dat.msub =
			asr_dat.msub = csr_dat.msub = 0.0;

		adu_dat.exact_match = chi_dat.exact_match =
			asr_dat.exact_match = csr_dat.exact_match = 0.0;

		adu_dat.expanded_match = chi_dat.expanded_match =
			asr_dat.expanded_match = csr_dat.expanded_match = 0.0;

		adu_dat.reduced_match = chi_dat.reduced_match =
			asr_dat.reduced_match = csr_dat.reduced_match = 0.0;

		adu_dat.subst_match = chi_dat.subst_match =
			asr_dat.subst_match = csr_dat.subst_match = 0.0;

/*
if (!first) {
	printf("CLASS_COUNT = %d\n", CLASS_COUNT);
	for (i = 0; i < CLASS_COUNT; i++)
		printf("%s\n", WORD_CLASS[i]);
}
*/

	}
}

static void update_special(
	enum classification_enum int_type,
	char codes[],			/* The codes this routine returns		*/
	double add_ops,	  		/* # of add operations in response		*/
	double del_ops,			/* # of del operations in source		*/
	double exa_ops,			/* # of exa operations for both			*/
	double cladd_ops,  		/* # of add operations in response		*/
	double cldel_ops,		/* # of del operations in source		*/
	double clexa_ops,		/* # of exa operations for both	   		*/
	double clsub_ops,		/* # of sub operations for class		*/
	double madd,			/* # of morphemes added					*/
	double mdel,			/* # of morphemes deleted				*/
	double mexa,			/* # of morphemes repeated				*/
	double msub) {			/* # of morphemes substituted			*/

	switch (int_type) {
		case adult:
			if ((add_ops + del_ops + clsub_ops + madd + mdel + msub) == 0) {
				if (((!DOING_CLASS) && (exa_ops > 0)) || ((DOING_CLASS) && (clexa_ops > 0))) {
					strcat(codes, " $EXACT");
					++adu_dat.exact_match;
				}
			} else if (((del_ops + clsub_ops + mdel + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (add_ops > 0)) || ((DOING_CLASS) && (cladd_ops > 0))) {
					strcat(codes, " $EXPAN");
					++adu_dat.expanded_match;
				}
			} else if (((add_ops + clsub_ops + madd + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (del_ops > 0)) || ((DOING_CLASS) && (cldel_ops > 0))) {
					strcat(codes, " $REDUC");
					++adu_dat.reduced_match;
				}
			} else if (((add_ops + del_ops + madd + mdel) == 0) && (exa_ops > 0) && (DOING_CLASS) && (clsub_ops > 0)) {
					strcat(codes, " $SUBST");
					++adu_dat.subst_match;
   			}
			break;

		case child:
			if ((add_ops + del_ops + clsub_ops + madd + mdel + msub) == 0) {
				if (((!DOING_CLASS) && (exa_ops > 0)) || ((DOING_CLASS) && (clexa_ops > 0))) {
					strcat(codes, " $EXACT");
					++chi_dat.exact_match;
				}
			} else if (((del_ops + clsub_ops + mdel + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (add_ops > 0)) || ((DOING_CLASS) && (cladd_ops > 0))) {
					strcat(codes, " $EXPAN");
					++chi_dat.expanded_match;
				}
			} else if (((add_ops + clsub_ops + madd + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (del_ops > 0)) || ((DOING_CLASS) && (cldel_ops > 0))) {
					strcat(codes, " $REDUC");
					++chi_dat.reduced_match;
				}
			} else if (((add_ops + del_ops + madd + mdel) == 0) && (exa_ops > 0) && (DOING_CLASS) && (clsub_ops > 0)) {
				strcat(codes, " $SUBST");
				++chi_dat.subst_match;
   			}
			break;


		case aduslf:
			if ((add_ops + del_ops + clsub_ops + madd + mdel + msub) == 0) {
				if (((!DOING_CLASS) && (exa_ops > 0)) || ((DOING_CLASS) && (clexa_ops > 0))) {
					strcat(codes, " $EXACT");
					++asr_dat.exact_match;
				}
			} else if (((del_ops + clsub_ops + mdel + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (add_ops > 0)) || ((DOING_CLASS) && (cladd_ops > 0))) {
					strcat(codes, " $EXPAN");
					++asr_dat.expanded_match;
				}
			} else if (((add_ops + clsub_ops + madd + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (del_ops > 0)) || ((DOING_CLASS) && (cldel_ops > 0))) {
					strcat(codes, " $REDUC");
					++asr_dat.reduced_match;
				}
			} else if (((add_ops + del_ops + madd + mdel) == 0) && (exa_ops > 0) && (DOING_CLASS) && (clsub_ops > 0)) {
				strcat(codes, " $SUBST");
				++asr_dat.subst_match;
   			}
			break;

   		case chislf:
			if ((add_ops + del_ops + clsub_ops + madd + mdel + msub) == 0) {
				if (((!DOING_CLASS) && (exa_ops > 0)) || ((DOING_CLASS) && (clexa_ops > 0))) {
					strcat(codes, " $EXACT");
					++csr_dat.exact_match;
				}
			} else if (((del_ops + clsub_ops + mdel + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (add_ops > 0)) || ((DOING_CLASS) && (cladd_ops > 0))) {
					strcat(codes, " $EXPAN");
					++csr_dat.expanded_match;
				}
			} else if (((add_ops + clsub_ops + madd + msub) == 0) && (exa_ops > 0)) {
				if (((!DOING_CLASS) && (del_ops > 0)) || ((DOING_CLASS) && (cldel_ops > 0))) {
					strcat(codes, " $REDUC");
					++csr_dat.reduced_match;
				}
			} else if (((add_ops + del_ops + madd + mdel) == 0) && (exa_ops > 0) && (DOING_CLASS) && (clsub_ops > 0)) {
					strcat(codes, " $SUBST");
					++csr_dat.subst_match;
   			}
			break;
		default:
			break;
	}
}



static void update_data(
	enum classification_enum int_type,
	double add_ops,	  		/* # of add operations in response		*/
	double del_ops,			/* # of del operations in source		*/
	double exa_ops,			/* # of exa operations for both			*/
	double sub_ops,			/* # of sub operations for both			*/
	double add_words,		/* # of add words in response			*/
	double del_words,		/* # of del words in source				*/
	double exa_words,		/* # of exa words for both				*/
	double sub_words,		/* # of sub words for both				*/
	int Fro_Word) {			/* T if fronted word from word-class	*/

	switch (int_type) {

		case adult:
			adu_dat.add_ops = adu_dat.add_ops + add_ops;
			adu_dat.del_ops = adu_dat.del_ops + del_ops;
			adu_dat.exa_ops = adu_dat.exa_ops + exa_ops;
			adu_dat.sub_ops = adu_dat.sub_ops + sub_ops;
			adu_dat.add_words = adu_dat.add_words + add_words;
			adu_dat.del_words = adu_dat.del_words + del_words;
			adu_dat.exa_words = adu_dat.exa_words + exa_words;
			adu_dat.sub_words = adu_dat.sub_words + sub_words;
			if (Fro_Word) ++adu_dat.fro_words;
			break;

		case child:
			chi_dat.add_ops = chi_dat.add_ops + add_ops;
			chi_dat.del_ops = chi_dat.del_ops + del_ops;
			chi_dat.exa_ops = chi_dat.exa_ops + exa_ops;
			chi_dat.sub_ops = chi_dat.sub_ops + sub_ops;
			chi_dat.add_words = chi_dat.add_words + add_words;
			chi_dat.del_words = chi_dat.del_words + del_words;
			chi_dat.exa_words = chi_dat.exa_words + exa_words;
			chi_dat.sub_words = chi_dat.sub_words + sub_words;
			if (Fro_Word) ++chi_dat.fro_words;
			break;

		case aduslf:
			asr_dat.add_ops = asr_dat.add_ops + add_ops;
			asr_dat.del_ops = asr_dat.del_ops + del_ops;
			asr_dat.exa_ops = asr_dat.exa_ops + exa_ops;
			asr_dat.sub_ops = asr_dat.sub_ops + sub_ops;
			asr_dat.add_words = asr_dat.add_words + add_words;
			asr_dat.del_words = asr_dat.del_words + del_words;
			asr_dat.exa_words = asr_dat.exa_words + exa_words;
			asr_dat.sub_words = asr_dat.sub_words + sub_words;
			if (Fro_Word) ++asr_dat.fro_words;
			break;

		case chislf:
			csr_dat.add_ops = csr_dat.add_ops + add_ops;
			csr_dat.del_ops = csr_dat.del_ops + del_ops;
			csr_dat.exa_ops = csr_dat.exa_ops + exa_ops;
			csr_dat.sub_ops = csr_dat.sub_ops + sub_ops;
			csr_dat.add_words = csr_dat.add_words + add_words;
			csr_dat.del_words = csr_dat.del_words + del_words;
			csr_dat.exa_words = csr_dat.exa_words + exa_words;
			csr_dat.sub_words = csr_dat.sub_words + sub_words;
			if (Fro_Word) ++csr_dat.fro_words;
			break;
		default:
			break;
	}
}


static int process_codes(
	real_utterance *source,/* The source utterance data			*/
	int s_length,			/* Number of words in source			*/
	real_utterance *response,/* The response utterance data		*/
	int r_length,			/* Number of words in source			*/
	char codes[],			/* The codes this routine returns		*/
	enum classification_enum int_type,
	double madd,			/* # of morphemes added					*/
	double mdel,			/* # of morphemes deleted				*/
	double mexa,			/* # of morphemes repeated				*/
	double msub) {			/* # of morphemes substituted			*/

	real_word *s_word;			/* The current word from the source		*/
	real_word *r_word;			/* The current word from the source		*/
	enum match_enum s_match;/* Codes for word matching and marking  */
	enum match_enum r_match;/* Codes for word matching and marking  */
	int i, j;				/* Integers controlling the main loop   */
	int ops_match = FALSE;
	int class_word = FALSE;
	int Fro_Word = FALSE;		/* T if item from class is fronted	*/
	double add_ops = 0.0;		/* # of add operations in response	*/
	double del_ops = 0.0;		/* # of del operations in source	*/
	double exa_ops = 0.0;		/* # of exa operations for both		*/
	double add_words = 0.0;		/* # of add words in response		*/
	double del_words = 0.0;		/* # of del words in source			*/
	double exa_words = 0.0;		/* # of exa words for both			*/
	double cladd_ops = 0.0;		/* # of add operations in response	*/
	double cldel_ops = 0.0;		/* # of del operations in source	*/
	double clexa_ops = 0.0;		/* # of exa operations for both		*/
	double clsub_ops = 0.0;		/* # of sub operations for both		*/
	double cladd_words = 0.0;	/* # of add words in response		*/
	double cldel_words = 0.0;	/* # of del words in source			*/
	double clexa_words = 0.0;	/* # of exa words for both			*/
	double clsub_words = 0.0;	/* # of sub words for both			*/

	i = 0;
	while (i < r_length) {
		r_word = utter_word(response, i);
		r_match = word_code(r_word);
		if (r_match == match_exact) {
			if (DOING_CLASS) {
				++clexa_ops; ++clexa_words;
				ops_match = TRUE;
				if (i == 0) {
					strcat(codes, " $FRO");
					Fro_Word = TRUE;
				}
			} else
				ops_match = FALSE;
			++exa_ops; ++exa_words;
			strcat(codes, " $EXA:");
			strcat(codes, word_stem(r_word));
			insert_code(r_word, match_unmarked);
			i++;
			j = i;
			while ((r_match=word_code(utter_word(response, j))) == match_exact) {
				strcat(codes, "-");
				r_word = utter_word(response, j);
				++exa_words;
				if (DOING_CLASS) {
					++clexa_words;
					if (ops_match == FALSE) {
						++clexa_ops;
						ops_match = TRUE;
					}
  				}
				strcat(codes, word_stem(r_word));
				insert_code(r_word, match_unmarked);
				j++;
				r_match = word_code(utter_word(response, j));
			}
		}
		i++;
	}


	i = 0;
	while (i < r_length) {
		r_word = utter_word(response, i);
		r_match = word_code(r_word);
		if (r_match == match_added) {
			if (DOING_CLASS) {
				++cladd_ops; ++cladd_words;
				ops_match = TRUE;
				if (i == 0) {
					strcat(codes, " $FRO");
					Fro_Word = TRUE;
				}
		   } else ops_match = FALSE;
		   ++add_ops; ++add_words;
			strcat(codes, " $ADD:");
			strcat(codes, word_text(r_word));
			insert_code(r_word, match_unmarked);
			i++;
			j = i;
			while ((r_match=word_code(utter_word(response, j))) == match_added) {
				strcat(codes, "-");
				r_word = utter_word(response, j);
				++add_words;
				if (DOING_CLASS) {
					++cladd_words;
					if (ops_match == FALSE) {
						++cladd_ops;
						ops_match = TRUE;
					}
				}
				strcat(codes, word_text(r_word));
				insert_code(r_word, match_unmarked);
				j++;
			}
		}
		i++;
	}


	if (DOING_SUBST) {
		i = 0;
		while (i < r_length) {
			r_word = utter_word(response, i);
			r_match = word_code(r_word);
			if (r_match == match_subst) {
				if (i == 0) {
					strcat(codes, " $FRO");
					Fro_Word = TRUE;
				}
				++clsub_ops;
				++clsub_words;
				strcat(codes, " $SUB:");
				strcat(codes, word_text(r_word));
				insert_code(r_word, match_unmarked);
				i++;
				j = i;
				while ((r_match=word_code(utter_word(response, j))) == match_subst) {
					strcat(codes, "-");
					r_word = utter_word(response, j);
					++clsub_words;
					strcat(codes, word_text(r_word));
					insert_code(r_word, match_unmarked);
					j++;
				}
			}
			i++;
		}
	}


	i = 0;
	while (i < s_length) {
		s_word = utter_word(source, i);
		s_match = word_code(s_word);
		if (s_match == match_deleted) {
			if (DOING_CLASS) {
				++cldel_ops; ++cldel_words;
				ops_match = TRUE;
			} else ops_match = FALSE;
			++del_ops; ++del_words;
			strcat(codes, " $DEL:");
			strcat(codes, word_text(s_word));
			insert_code(s_word, match_unmarked);
			i++;
			j = i;
			while ((s_match=word_code(utter_word(source, j))) == match_deleted) {
				strcat(codes, "-");
				s_word = utter_word(source, j);
				++del_words;
				if (DOING_CLASS) {
					++cldel_words;
					if (ops_match == FALSE) {
						++cldel_ops;
						ops_match = TRUE;
					}
				}
				strcat(codes, word_text(s_word));
				insert_code(s_word, match_unmarked);
				j++;
			}
		}
		i++;
	}

   /* Basic data about operations and words */
	if (!DOING_CLASS) {
		update_data(int_type, add_ops, del_ops, exa_ops, 0,add_words, del_words, exa_words, 0, Fro_Word);
	} else if (DOING_CLASS) {
		update_data(int_type, cladd_ops, cldel_ops,clexa_ops, clsub_ops, cladd_words,cldel_words, clexa_words, clsub_words, Fro_Word);
	}

   /* Data about special responses */
   if (!DOING_CLASS) {
	   if ((add_ops > 0) || (del_ops > 0) || (exa_ops > 0)) {
		   update_special(int_type, codes, add_ops, del_ops, exa_ops,cladd_ops, cldel_ops, clexa_ops, clsub_ops,madd, mdel, mexa, msub);
	   }
   } else if (DOING_CLASS) {
	   if ((cladd_ops > 0) || (cldel_ops > 0) || (clexa_ops > 0) || (clsub_ops > 0)) {
		   update_special(int_type, codes, add_ops, del_ops, exa_ops, cladd_ops, cldel_ops, clexa_ops, clsub_ops, madd, mdel, mexa, msub);
		   class_word = TRUE;
		   /* Test of class overlap counter */
		   /* if (cldel_ops > 0) class_word = TRUE;*/
	   }
   }
   return class_word;
}


static void update_responses(
	enum classification_enum int_type,
	double rep_index,
	int class_word) {

	switch (int_type) {
		case adult:
			if (DOING_ADU && (rep_index >= 0.0)) { /* response is not empty */
				if (DOING_CLASS && class_word && (rep_index > MININDEX))
					++adu_dat.clresponses;
				if (rep_index > MININDEX) {
					++adu_dat.responses;
					adu_dat.rep_index = adu_dat.rep_index + rep_index;
				} else ++adu_dat.no_overlap;
			}
			break;
		case child:
			if (DOING_CHI && (rep_index >= 0.0)) { /* response is not empty */
				if (DOING_CLASS && class_word && (rep_index > MININDEX))
					++chi_dat.clresponses;
				if (rep_index > MININDEX) {
					++chi_dat.responses;
					chi_dat.rep_index = chi_dat.rep_index + rep_index;
				} else ++chi_dat.no_overlap;
			}
			break;
		case aduslf:
			if (DOING_SLF && (rep_index >= 0.0)) { /* response is not empty */
				if (DOING_CLASS && class_word && (rep_index > MININDEX))
					++asr_dat.clresponses;
				if (rep_index > MININDEX) {
					++asr_dat.responses;
					asr_dat.rep_index = asr_dat.rep_index + rep_index;
				} else ++asr_dat.no_overlap;
			}
			break;
		case chislf:
			if (DOING_SLF && (rep_index >= 0.0)) { /* response is not empty */
				if (DOING_CLASS && class_word && (rep_index > MININDEX))
					++csr_dat.clresponses;
				if (rep_index > MININDEX) {
					++csr_dat.responses;
					csr_dat.rep_index = csr_dat.rep_index + rep_index;
				} else ++csr_dat.no_overlap;
			}
			break;
		default:
			break;
	}
}

static int morph_code(
	char mcodes[],	/* The morphological code string	*/
	char rmorph[],	/* The morphology on the response word	*/
	char smorph[]) {	/* The morphology on the source word	*/

	int level = 0;

	if (rmorph && smorph) {
		if (strcmp(rmorph, smorph) == 0) {
			level = 1;				/* Morph has been repeated		*/
			strcat(mcodes, " $MEXA:");
			strcat(mcodes, rmorph);
		} else {
			level = 4;		/* Morph has been substituted		*/
			strcat(mcodes, " $MSUB:");
			strcat(mcodes, rmorph);
		}
	} else if (rmorph && !smorph) {
		level = 2;				/* Morph has been added				*/
		strcat(mcodes, " $MADD:");
		strcat(mcodes, rmorph);
	} else if (!rmorph && smorph) {
		level = 3;				/* Morph has been deleted		*/
		strcat(mcodes, " $MDEL:");
		strcat(mcodes, smorph);
	} else level = 0;				/* There is no morphology		*/

	return level;
}


static void update_morph(
	enum classification_enum int_type,
	double madd,		/* # of morphemes added			*/
	double mdel,		/* # of morphemes deleted		*/
	double mexa,		/* # of morphemes repeated		*/
	double msub) {		/* # of morphemes substituted	*/

	switch(int_type) {
		case adult:
			adu_dat.madd = adu_dat.madd + madd;
			adu_dat.mdel = adu_dat.mdel + mdel;
			adu_dat.mexa = adu_dat.mexa + mexa;
			adu_dat.msub = adu_dat.msub + msub;
			break;
		case child:
			chi_dat.madd = chi_dat.madd + madd;
			chi_dat.mdel = chi_dat.mdel + mdel;
			chi_dat.mexa = chi_dat.mexa + mexa;
			chi_dat.msub = chi_dat.msub + msub;
			break;
		case aduslf:
			asr_dat.madd = asr_dat.madd + madd;
			asr_dat.mdel = asr_dat.mdel + mdel;
			asr_dat.mexa = asr_dat.mexa + mexa;
			asr_dat.msub = asr_dat.msub + msub;
			break;
		case chislf:
			csr_dat.madd = csr_dat.madd + madd;
			csr_dat.mdel = csr_dat.mdel + mdel;
			csr_dat.mexa = csr_dat.mexa + mexa;
			csr_dat.msub = csr_dat.msub + msub;
			break;
		default:
			break;
	}
}

static double single_interaction(
	real_utterance *source,/* The source utterance data			*/
	real_utterance *response,/* The response utterance data		*/
	char codes[],			/* The codes this routine returns		*/
	enum classification_enum int_type) {

	int s_length;			/* Number of words in source			*/
	int r_length;			/* Number of words in response			*/
	real_word *s_word;		/* The current word from the source		*/
	real_word *r_word;		/* The current word from the response   */
	double overlap;			/* Signals overlap between utterances   */
	double rep_index;		/* Source-response repetition index	 	*/
	int i, j;				/* Integers controlling the main loop   */
	int DO_MORPH = FALSE;	/* T is we should update morph vars		*/
	int class_word = FALSE;	/* T if word from WORD-CLASS found		*/
	double madd = 0.0;		/* # of morphemes added					*/
	double mdel = 0.0;		/* # of morphemes deleted				*/
	double mexa = 0.0;		/* # of morphemes repeated				*/
	double msub = 0.0;		/* # of morphemes substituted			*/



	make_null(codes); make_null(spareTier1);
	overlap = 0;
	s_length = utter_words(source);
	r_length = utter_words(response);

	i = 0;
	while (i < r_length) {
		r_word = utter_word(response, i);
		i++;
		j = 0;
		while (j < s_length) {
			s_word = utter_word(source, j);
			j++;
			if (word_match(r_word, s_word, TRUE)) {
				overlap++;
				insert_code(s_word, match_exact);
				insert_code(r_word, match_exact);
				if (!DOING_CLASS || (DOING_CLASS && word_pos_match(r_word, s_word, FALSE)))
					DO_MORPH = TRUE;
				switch(morph_code(spareTier1, word_prefix(r_word), word_prefix(s_word))) {
					case 0: break;		/* No morphology */
					case 1: if (DO_MORPH) ++mexa; break;
					case 2: if (DO_MORPH) ++madd; break;
					case 3: if (DO_MORPH) ++mdel; break;
					case 4: if (DO_MORPH) ++msub; break;
					default:
						fprintf(stderr, "Unknown level for morphology.\n");
						chip_exit(NULL, TRUE);
						break;
				}
				switch(morph_code(spareTier1, word_suffix(r_word), word_suffix(s_word))) {
					case 0: break;		/* No morphology */
					case 1: if (DO_MORPH) ++mexa; break;
					case 2: if (DO_MORPH) ++madd; break;
					case 3: if (DO_MORPH) ++mdel; break;
					case 4: if (DO_MORPH) ++msub; break;
					default:
						fprintf(stderr, "Unknown level for morphology.\n");
						chip_exit(NULL, TRUE);
						break;
				}
				break;
			}
		}
	}

	i = 0;
	while (i < r_length) {
		r_word = utter_word(response, i);
		i++;
		j = 0;
		while (j < s_length) {
			s_word = utter_word(source, j);
			j++;
			if (word_match(r_word, s_word, FALSE)) {
				break;
			} else if (DOING_SUBST && word_pos_match(r_word, s_word, TRUE)) {
				insert_code(s_word, match_subst);
				insert_code(r_word, match_subst);
			}
		}
	}
	
	i = 0;
	while (i < r_length) {
		r_word = utter_word(response, i);
		if (word_code(r_word) == match_unmarked) {
			insert_code(r_word, match_added);
		}
		i++;
	}

	j = 0;
	while ((j < s_length) && (r_length > 0)) {
		s_word = utter_word(source, j);
		if (word_code(s_word) == match_unmarked) {
			insert_code(s_word, match_deleted);
		}
		j++;
	}


	/* Do not process or output codes if there is not enough overlap between utterances */

   if (r_length == 0) {
		rep_index = -1.0;
	} else rep_index = overlap / r_length;

	if (r_length == 0) {
		strcat(codes, "\0");
   } else if (overlap == 0) {
		strcat(codes, " $NO_REP");
	} else if (rep_index < MININDEX) {
		strcat(codes, " $LO_REP");
	} else {
		class_word =
		 process_codes(source, s_length, response, r_length, codes, int_type, madd, mdel, mexa, msub);
		 strcat(codes, spareTier1);
	}

	update_responses(int_type, rep_index, class_word);
	update_morph(int_type, madd, mdel, mexa, msub);

	/* Make sure the source and response words are unmarked	*/
	for (i = 0; i < r_length; i++) {
		insert_code(utter_word(response, i), match_unmarked);
	}
	for (j = 0; j < s_length; j++) {
		insert_code(utter_word(source, j), match_unmarked);
	}

	return rep_index;
}

static void print_data()
{
	if (TOTAL_UTTER_NUM > 0) {
		fprintf(fpout, "===========================================================\n");
		if (!combinput)
			fprintf(fpout, "File: %s\n\n", oldfname);
		if (onlydata != 3) {
			if (stout) {
				char  fontName[256];
				strcpy(fontName, "CAfont:13:7\n");
				cutt_SetNewFont(fontName, EOS);
			} else
				fprintf(fpout,"%s	CAfont:13:7\n", FONTHEADER);
		}
		/* Output TOTAL UTTERANCES */
		fprintf(fpout, "Total ");
		output_adult_codes(fpout);
		fprintf(fpout, " scored utterances: %.0f\n", adu_dat.utters);
		fprintf(fpout, "Total ");
		output_child_codes(fpout);
		fprintf(fpout, " scored utterances: %.0f\n", chi_dat.utters);
		fprintf(fpout, "\n");

		fprintf(fpout, "Measure  \tADU\tCHI\tASR\tCSR\n");
		fprintf(fpout, "-----------------------------------------------------------\n");

/*	fprintf(fpout, "%s\tUtterances for all speakers: %d\n", oldfname, TOTAL_UTTER_NUM);*/

		/* Output TOTAL RESPONSES */
		fprintf(fpout, "Responses\t");
		if (DOING_ADU) {
		   fprintf(fpout, "%.0f\t", (adu_dat.responses + adu_dat.no_overlap));
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
		   fprintf(fpout, "%.0f\t", (chi_dat.responses + chi_dat.no_overlap));
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
		   fprintf(fpout, "%.0f\t", (asr_dat.responses + asr_dat.no_overlap));
		   fprintf(fpout, "%.0f", (csr_dat.responses + csr_dat.no_overlap));
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* Output OVERLAP */
		fprintf(fpout, "Overlap  \t");
		if (DOING_ADU) {
			fprintf(fpout, "%.0f\t", adu_dat.responses);
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			fprintf(fpout, "%.0f\t", chi_dat.responses);
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			fprintf(fpout, "%.0f\t", asr_dat.responses);
			fprintf(fpout, "%.0f", csr_dat.responses);
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* Output CLASS_RESP */
		if (DOING_CLASS) {
			fprintf(fpout, "Class_Overlap\t");
			if (DOING_ADU) {
				fprintf(fpout, "%.0f\t", adu_dat.clresponses);
			} else fprintf(fpout, "adu\t");
			if (DOING_CHI) {
				fprintf(fpout, "%.0f\t", chi_dat.clresponses);
			} else fprintf(fpout, "chi\t");
			if (DOING_SLF) {
				fprintf(fpout, "%.0f\t", asr_dat.clresponses);
				fprintf(fpout, "%.0f", csr_dat.clresponses);
			} else {
				fprintf(fpout, "asr\t");
				fprintf(fpout, "csr");
			}
			fprintf(fpout, "\n");
		}

		/* Output NO_OVERLAP */
		fprintf(fpout, "No_Overlap\t");
		if (DOING_ADU) {
			fprintf(fpout, "%.0f\t", adu_dat.no_overlap);
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			fprintf(fpout, "%.0f\t", chi_dat.no_overlap);
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			fprintf(fpout, "%.0f\t", asr_dat.no_overlap);
			fprintf(fpout, "%.0f", csr_dat.no_overlap);
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* Output %_OVERLAP */
		fprintf(fpout, "%%_Overlap\t");
		if (DOING_ADU) {
			if (adu_dat.utters > 0) {
				if (DOING_CLASS) {
					fprintf(fpout, "%2.3f\t", (adu_dat.clresponses / adu_dat.utters));
				} else fprintf(fpout, "%2.3f\t", (adu_dat.responses / adu_dat.utters));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			if (chi_dat.utters > 0) {
				if (DOING_CLASS) {
					fprintf(fpout, "%2.3f\t", (chi_dat.clresponses / chi_dat.utters));
				} else fprintf(fpout, "%2.3f\t", (chi_dat.responses / chi_dat.utters));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (adu_dat.utters > 0) {
				if (DOING_CLASS) {
					fprintf(fpout, "%2.3f\t",
						(asr_dat.clresponses / adu_dat.utters));
				} else fprintf(fpout, "%2.3f\t", (asr_dat.responses / adu_dat.utters));
			} else fprintf(fpout, "0.00\t");
			if (chi_dat.utters > 0) {
				if (DOING_CLASS) {
					fprintf(fpout, "%2.3f", (csr_dat.clresponses / chi_dat.utters));
				} else fprintf(fpout, "%2.3f", (csr_dat.responses / chi_dat.utters));
			} else fprintf(fpout, "0.00");

		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* Output AVERAGE DISTANCE BETWEEN TURNS */
		fprintf(fpout, "Avg_Dist\t");
		if (DOING_ADU) {
			if (adu_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (adu_dat.dist / adu_dat.responses));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			if (chi_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (chi_dat.dist / chi_dat.responses));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (asr_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (asr_dat.dist / asr_dat.responses));
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.responses > 0) {
				fprintf(fpout, "%3.2f", (csr_dat.dist / csr_dat.responses));
			} else fprintf(fpout, "0.00");
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* Output REPETITION INDICES */
		fprintf(fpout, "Rep_Index\t");
		if (DOING_ADU) {
			if (adu_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (adu_dat.rep_index / adu_dat.responses));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			if (chi_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (chi_dat.rep_index / chi_dat.responses));
			} else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (asr_dat.responses > 0) {
				fprintf(fpout, "%3.2f\t", (asr_dat.rep_index / asr_dat.responses));
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.responses > 0) {
				fprintf(fpout, "%3.2f", (csr_dat.rep_index / csr_dat.responses));
			} else fprintf(fpout, "0.00");
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");

		/* information concerning operations */
		pr_ops();

		/* information concerning words */
		pr_words();

		/* information concerning morphemes */
		pr_morph();

		/* information concerning avg words per operation */
		pr_avg();

		/* information concerning fronting */
		if (DOING_CLASS) pr_front();

		/* information concerning special respones */
		pr_special();

	}
}
static void interact_file()
{
	int previous;	/* Index to previous matching class in UTTWINDOW	*/
	int current;	/* Index to current utterance in UTTWINDOW		*/
	int  LastUtter;
	int same_spkr;	/* Data_type to indicate the current speaker		*/
	int child_spkr;
	int adult_spkr;
	double rep_index = 0.0;	/* rep_index for  each utterance pair	  */
	
	codes = (char *)malloc((size_t) (((2*UTTLINELEN) + 1) * sizeof(char)) );
	if(!codes) chip_exit("interact_file: codes", TRUE);
	repstring = (char *)malloc((size_t) ((UTTLINELEN + 1) * sizeof(char)) );
	if(!repstring) chip_exit("interact_file: repstring", TRUE);
	dist = (char *) malloc((size_t) ((UTTLINELEN + 1) * sizeof(char)) );
	if(!dist) chip_exit("interact_file: dist", TRUE);
	LastUtter = 1;
	current = 0;
	utters[current] = new_utter();
	if (utters[current] == NULL) {
		perror("Couldn't allocate a new utterance.");
		chip_exit(NULL, TRUE);
	}
	while (parse_utter(utters[current])) {
		if (utter_words(utters[current]) > 0) {
			++TOTAL_UTTER_NUM;
			if (utter_class(utters[current]) == adult) {
				++adu_dat.utters;
				/*printf("Found an adult utterance\n");*/
			} else if (utter_class(utters[current]) == child) {
				++chi_dat.utters;
				/*printf("Found a child utterance\n");*/
			}
			if (DOING_SLF) {
				previous = 1; same_spkr = FALSE;
				while ((!same_spkr) && (previous < LastUtter)) {
					if (same_speaker(utter_speaker(utters[current]), utter_speaker(utters[previous])) && utter_words(utters[previous]) > 0) {
						same_spkr = TRUE;
					} else previous++;
				}
				if (same_spkr) {
					switch (utter_class(utters[current])) {
						case adult:
							rep_index = single_interaction(utters[previous], utters[current], codes, aduslf);
							if (rep_index > MININDEX)
								asr_dat.dist = asr_dat.dist + previous;
							if (onlydata != 2 && onlydata != 3 && rep_index >= 0.0) {
								if (rep_index > MININDEX) {
									strcat(codes, " $DIST = ");
									sprintf(dist, "%d", previous);
									strcat(codes, dist);
								}
								strcat(codes, " $REP = ");
								sprintf(repstring, "%1.2f", rep_index);
								strcat(codes, repstring);
								printout("%asr:", codes+1, NULL, NULL, TRUE);
							}
							break;
							
						case child:
							rep_index = single_interaction(utters[previous], utters[current], codes, chislf);
							if (rep_index > MININDEX)
								csr_dat.dist = csr_dat.dist + previous;
							if (onlydata != 2 && onlydata != 3 && rep_index >= 0.0) {
								if (rep_index > MININDEX) {
									strcat(codes, " $DIST = ");
									sprintf(dist, "%d", previous);
									strcat(codes, dist);
								}
								strcat(codes, " $REP = ");
								sprintf(repstring, "%1.2f", rep_index);
								strcat(codes, repstring);
								printout("%csr:", codes+1, NULL, NULL, TRUE);
							}
							break;
						default:
							break;
					}
				}
			}
			switch (utter_class(utters[current])) {
				case adult:		/* adult interaction output in %adu tier */
					if (DOING_ADU) {
 						previous = 1; child_spkr = FALSE;
						while ((!child_spkr) && (previous < LastUtter)) {
							if ((utter_class(utters[previous]) == child) && (utter_words(utters[previous]) > 0)) {
								child_spkr = TRUE;
							} else previous++;
						}
						if (child_spkr) {
							rep_index = single_interaction(utters[previous], utters[current], codes, adult);
							if (rep_index > MININDEX)
								adu_dat.dist = adu_dat.dist+previous;
							if (onlydata != 2 && onlydata != 3 && rep_index >= 0.0) {
								if (rep_index > MININDEX) {
									strcat(codes, " $DIST = ");
									sprintf(dist, "%d", previous);
									strcat(codes, dist);
								}
								strcat(codes, " $REP = ");
								sprintf(repstring, "%1.2f", rep_index);
								strcat(codes, repstring);
								printout("%adu:", codes+1, NULL, NULL, TRUE);
							}
						}
					}
					break;
				case child:		/* child interaction output in %chi tier */
					if (DOING_CHI) {
						previous = 1; adult_spkr = FALSE;
						while ((!adult_spkr) && (previous < LastUtter)) {
							if ((utter_class(utters[previous]) == adult) && (utter_words(utters[previous]) > 0)) {
								adult_spkr = TRUE;
							} else previous++;
						}
						if (adult_spkr) {
							rep_index = single_interaction(utters[previous], utters[current], codes, child);
							if (rep_index > MININDEX)
								chi_dat.dist = chi_dat.dist+previous;
							if (onlydata != 2 && onlydata != 3 && rep_index >= 0.0) {
								if (rep_index > MININDEX) {
									strcat(codes, " $DIST = ");
									sprintf(dist, "%d", previous);
									strcat(codes, dist);
								}
								strcat(codes, " $REP = ");
								sprintf(repstring, "%1.2f", rep_index);
								strcat(codes, repstring);
								printout("%chi:", codes+1, NULL, NULL, TRUE);
							}
						}
					}
					break;
					
				default:		/* others OTHer. */
					break;
			}
		}
		if (LastUtter == UTTWINDOW) {
			free_utter(utters[LastUtter-1]);
			utters[LastUtter-1] = NULL;
		} else 
			LastUtter++;
		for (current = LastUtter-1; current > 0; current--)
			utters[current] = utters[current-1];
		current = 0;
		utters[current] = new_utter();
		if (utters[current] == NULL) {
			perror("Couldn't allocate a new utterance");
			chip_exit(NULL, TRUE);
		}
	}
	clean_all();
}


void call() 		/* Handle an input file (Called by CLAN) */
{
	chip_done = 1;
	*utterance->speaker = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);	/* Prime the single character input buffer */
	interact_file();
	if (!combinput && onlydata != 1)
		print_data();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = CHIP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 3;
	UttlineEqUtterance = FALSE;	   /* Copy of input data is saved		*/
	bmain(argc, argv, print_data); /* Call to the CLAN main routine */
}


void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'n':
			switch (*f) {
				case 's':
				case 'S': DOING_SLF = FALSE; break;
				case 'b':
				case 'B': DOING_ADU = FALSE; break;
				case 'c':
				case 'C': DOING_CHI = FALSE; break;
				default:
					fprintf(stderr, "ERROR: The only allowed letters are: b, c, or s");
					chip_exit(NULL, TRUE);
			}
			break;
		case 'h':
			if (*(f-2) == '+') {
				fprintf(stderr,"Invalid option: %s\n", f-2);
				chip_exit(NULL, TRUE);
			} else
				rdexclf('h','e',getfarg(f,f1,i));
			break;
		case 'x':		/* Set minimum repetition index						*/
			MININDEX = atof(f);
			break;
			
		case 'g':		/* Doing substitutions								*/
			DOING_SUBST = TRUE;
			DOING_CLASS = TRUE;
			no_arg_option(f);
			break;
			
		case 'c':		/* Name a speaker as a child.						*/
			if (*f == '*')
				f++;
			uS.uppercasestr(f, &dFnt, C_MBF);
			set_child_speaker(f);
			break;
			
		case 'b':		/* Name a speaker as an adult.						*/
			if (*f == '*')
				f++;
			uS.uppercasestr(f, &dFnt, C_MBF);
			set_adult_speaker(f);
			break;
			
		case 'q':		/* Set utterance window */
			UTTWINDOW = atoi(f);
			if (UTTWINDOW < 2 || UTTWINDOW > MAXUTTWINDOW) {
				fprintf(stderr,"Range of utterance window is 2-%d.\n", MAXUTTWINDOW);
				chip_exit(NULL, TRUE);
			}
			break;
		case 't':
			if (*(f-2) == '-' && (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:"))) {
				isMorTier = FALSE;
			} else
				maingetflag(f-2,f1,i);
			break;			
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
