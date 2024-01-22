/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * main.c                                                  *
 *        - command line parsing                           *
 *        - filtering                                      *
 *        - computation of D_optimum                       *
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * VOCD Version 1.1, February 2000                         *
 * VOCD Version 1.2, June 2000                             *
 * VOCD Version 1.3, April 2004                            *
 *                                                         * 
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/

#define CHAT_MODE 1

#include "cu.h"
#include <math.h>
#include "c_curses.h"
#if !defined(_MAC_CODE)
#include <sys/types.h>
#endif
#include <time.h>
#include "args.h"
#include "vocdefs.h"
#include "speaker.h"
#include "tokens.h"
#include "filter.h"
#include "dcompute.h"

#if !defined(UNX)
#define _main vocd_main
#define call vocd_call
#define getflag vocd_getflag
#define init vocd_init
#define usage vocd_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;
extern struct IDtype *IDField;
extern struct IDtype *SPRole;

#define NUMSOPTIONS 500

static int D_from    = 35;
static int D_to      = 50;
static int D_inc     =  1;
static int D_samples =100;
static int CAPITALISATION;
static int SPLIT_HALF_ODD;
static int SPLIT_HALF_EVEN;
static int SEQUENTIAL_SAMPLING;	// flag for sequential sampling
static int tkns_in_seq;
static char **token_seq;
static char vocd_isCombineSpeakers;
static char vocd_FTime;
static char vocd_isPrintIDHeader;
static char vocd_isDefaultMOR;
static char isSpeakerSpecified;
static char numer_directives[CMD_LINE_MAX];
static char *numer_s_options[NUMSOPTIONS];
static char denom_directives[CMD_LINE_MAX];
static char *denom_s_options[NUMSOPTIONS];
static long d_flags;

char isDoVocdAnalises, vocd_show_warnings;
char VOCD_TYPE_D;		// flag for type-type-d ratio
VOCDSPs *speakers;
VOCDSPs *denom_speakers;

#ifndef KIDEVAL_LIB
void usage() {
    puts("VOCD computes the lexical diversity index");
#ifdef UNX
	printf("VOCD NOW WORKS ON \"%%mor:\" TIER AND PERFORMS LEMMA-BASED ANALYSIS BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO OVERRIDE DEFAULT LEMMA-BASED ANALYSIS ON \"%%mor:\" TIER PLEASE USE \"+o\" OPTION.\n");
#else
	printf("%c%cVOCD NOW WORKS ON \"%%mor:\" TIER AND PERFORMS LEMMA-BASED ANALYSIS BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO OVERRIDE LEMMA-BASED ANALYSIS ON \"%%mor:\" TIER PLEASE USE \"+o\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
    printf("Usage : vocd [b0 b1 bCN c dN gCS oN %s] filename(s)\n",mainflgs());
	puts("+b0 : D_optimum - use split half; even.");
	puts("+b1 : D_optimum - use split half; odd.");
	puts("+bsN: D_optimum - size N of starting sample (default  35)");
	puts("+blN: D_optimum - size N of largest sample  (default  50)");
	puts("+biN: D_optimum - size N of increments      (default   1)");
	puts("+bnN: D_optimum - the N number of samples   (default 100)");
	puts("+br:  D_optimum - use random sampling with replacement (default: no replacement)");
	puts("+be:  D_optimum - use sequential sampling");
	puts("+c :  find capitalized words only");
	puts("+d :  outputs utterances and type-token information");
	puts("+d1:  outputs only VOCD results summary on one line");
	puts("+d2:  outputs only type/token information");
	puts("+d3:  outputs only type/token information in SPREADSHEET format");
	puts("+gnS: compute \"limiting type-type ratio\" S=NUMERATOR +s directives");
	puts("-gnS: compute \"limiting type-type ratio\" S=NUMERATOR -s directives");
	puts("+gdS: compute \"limiting type-type ratio\" S=DENOMINATOR +s directives");
	puts("-gdS: compute \"limiting type-type ratio\" S=DENOMINATOR -s directives");
	puts("+o:  override default lemma-based analysis");
	puts("+o3: combine selected speakers from each file into one results list");
	mainusage(FALSE);
	puts("Examples:");
	puts("lemma-based analysis is performed by default");
	puts("limiting relative diversity (LRD), compare verb and noun stems diversity");
	puts("    vocd +t*CHI +gn\"m|v.;*,o%%\" +gd\"m|n,;*,o%%\" filename.cha");
	puts("    vocd +t*CHI +gn\"m|v.;*,o%%\" +gd\"m|n,;*,o%%\" +r4 filename.cha");
#ifdef UNX
	printf("VOCD NOW WORKS ON \"%%mor:\" TIER AND PERFORMS LEMMA-BASED ANALYSIS BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO OVERRIDE DEFAULT LEMMA-BASED ANALYSIS ON \"%%mor:\" TIER PLEASE USE \"+o\" OPTION.\n");
#else
	printf("%c%cVOCD NOW WORKS ON \"%%mor:\" TIER AND PERFORMS LEMMA-BASED ANALYSIS BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO OVERRIDE LEMMA-BASED ANALYSIS ON \"%%mor:\" TIER PLEASE USE \"+o\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}
#endif // KIDEVAL_LIB

static void default_excludes(void) {
	addword('\0','\0',"+xx");
	addword('\0','\0',"+xxx");
	addword('\0','\0',"+yy");
	addword('\0','\0',"+yyy");
	addword('\0','\0',"+www");
	addword('\0','\0',"+*|xx");
	addword('\0','\0',"+unk|xxx");
	addword('\0','\0',"+*|yy");
	addword('\0','\0',"+unk|yyy");
	addword('\0','\0',"+*|www");
	addword('\0','\0',"+.");
	addword('\0','\0',"+?");
	addword('\0','\0',"+!");
	maininitwords();
	mor_initwords();
}

void init_vocd(char first) {
	int i;

	if (first) {
		vocd_show_warnings = FALSE;
		isDoVocdAnalises = FALSE;
		vocd_isCombineSpeakers = FALSE;
		speakers = NULL;
		denom_speakers = NULL;
		tkns_in_seq = 0;
		token_seq = NULL;
		D_from    = 35;
		D_to      = 50;
		D_inc     =  1;
		D_samples =100;
		d_flags = 0 | NOREPLACEMENT;
		CAPITALISATION = 0;
		SPLIT_HALF_ODD = 0;
		SPLIT_HALF_EVEN = 0;
		SEQUENTIAL_SAMPLING = 0;
		numer_directives[0] = EOS;
		denom_directives[0] = EOS;
		for (i=0; i < NUMSOPTIONS; i++) {
			numer_s_options[i] = NULL;
			denom_s_options[i] = NULL;
		}
		vocd_isDefaultMOR = FALSE;
	} else {
		if (token_seq)
			free (token_seq);
		tkns_in_seq = 0;
		token_seq = NULL;
	}
}

#ifndef KIDEVAL_LIB
void init(char first) {
	char ts[256];

	if (first) {
		vocd_FTime = TRUE;
		vocd_isPrintIDHeader = FALSE;
		LocalTierSelect = TRUE;
		if (!VOCD_TYPE_D) {
			default_excludes();
		}
		// defined in "mmaininit" and "globinit" - nomap = TRUE;
		isSpeakerSpecified = FALSE;
		onlydata = 0;
		init_vocd(first);
		vocd_isDefaultMOR = TRUE;
		vocd_show_warnings = TRUE;
	} else {
		if (vocd_FTime) {
			vocd_FTime = FALSE;
			if (IDField == NULL && SPRole == NULL && !isSpeakerSpecified && vocd_isCombineSpeakers) {
				isSpeakerSpecified = TRUE;
				strcpy(ts, "*");
				if (speaker_add_speaker(&speakers, ts, NULL, NULL, FALSE, TRUE) == NULL) { // add default speaker
					fprintf(stderr,"cannot add speaker to list\n");
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				if (VOCD_TYPE_D) {
					strcpy(ts, "*");
					if (speaker_add_speaker(&denom_speakers, ts, NULL, NULL, FALSE, TRUE) == NULL) { // add default speaker
						fprintf(stderr,"cannot add speaker to list\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					}
				}
			}
			if (onlydata == 4) {
				stout = FALSE;
				combinput = TRUE;
				AddCEXExtension = ".xls";
				vocd_isPrintIDHeader = TRUE;
			}
			if (vocd_isDefaultMOR) {
				strcpy(ts, "m;*,o%");
				ts[0] = '\002';
				addword('s','i',ts);
			}
			if ((isMORSearch() || isMultiMorSearch) && chatmode) {
				nomain = TRUE;
				maketierchoice("%mor",'+',FALSE);
			}
		}
		if (!combinput || onlydata == 4) {
			init_vocd(first);
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	int  j;

	f++;
	switch(*f++) {
		case 'b':
			if (*f == '0') {
				SPLIT_HALF_EVEN = 1;
				SPLIT_HALF_ODD  = 0;
				if (*(f+1) != EOS) {
					fprintf(stderr,"Option \"%s\" does not allow anything after +b0.\n", f-2);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
			} else if (*f == '1') {
				SPLIT_HALF_EVEN = 0;
				SPLIT_HALF_ODD  = 1;
				if (*(f+1) != EOS) {
					fprintf(stderr,"Option \"%s\" does not allow anything after +b1.\n", f-2);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
			} else if (*f == 's') {
				D_from = atoi(f+1);
			} else if (*f == 'l') {
				D_to = atoi(f+1);
			} else if (*f == 'i') {
				D_inc = atoi(f+1);
			} else if (*f == 'n') {
				D_samples = atoi(f+1);
			} else if (*f == 'r') {
				d_flags &= ~NOREPLACEMENT;
				if (*(f+1) != EOS) {
					fprintf(stderr,"Option \"%s\" does not allow anything after +br.\n", f-2);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
			} else if (*f == 'e') {
				SEQUENTIAL_SAMPLING = 1;
				if (*(f+1) != EOS) {
					fprintf(stderr,"Option \"%s\" does not allow anything after +be.\n", f-2);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
			} else {
				fprintf(stderr,"The only +b's allowed are 0, 1, s, l, i, n, r, e.\n");
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
			break;
		case 'c':
			CAPITALISATION = 1;
			nomap = TRUE;
			no_arg_option(f);
			vocd_isDefaultMOR = FALSE;
			break;
		case 'd':
			onlydata = atoi(getfarg(f,f1,i))+1;
			if (onlydata > OnlydataLimit) {
				fprintf(stderr, "The only +d levels allowed are 0-%d.\n", OnlydataLimit-1);
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
			if (onlydata == 4)
				OverWriteFile = TRUE;
			break;
		case 'g':
			vocd_isDefaultMOR = FALSE;
			if (*f == 'n') {
				for (j=0; j < NUMSOPTIONS && numer_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				numer_s_options[j] = f - 2;
				if (strlen(numer_directives) + strlen(f-1) < CMD_LINE_MAX-2) {
					if (numer_directives[0] != EOS)
						strcat(numer_directives, " ");
					if (*(f-2) == '+')
						strcat(numer_directives, "+s");
					else
						strcat(numer_directives, "-s");
					strcat(numer_directives, f+1);
				}
			} else if (*f == 'd') {
				for (j=0; j < NUMSOPTIONS && denom_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				denom_s_options[j] = f - 2;
				if (strlen(denom_directives) + strlen(f-1) < CMD_LINE_MAX-2) {
					if (denom_directives[0] != EOS)
						strcat(denom_directives, " ");
					if (*(f-2) == '+')
						strcat(denom_directives, "+s");
					else
						strcat(denom_directives, "-s");
					strcat(denom_directives, f+1);
				}
			} else {
				fprintf(stderr,"The only +/-g's allowed are n or d.\n");
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
			break;
		case 'o':
			if (*f == EOS) {
				vocd_isDefaultMOR = FALSE;
			} else if (*f == '3') {
				vocd_isCombineSpeakers = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
			break;
		case 'r':
			if (*f == '6' && VOCD_TYPE_D) {
				for (j=0; j < NUMSOPTIONS && numer_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				numer_s_options[j] = f - 2;
				for (j=0; j < NUMSOPTIONS && denom_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				denom_s_options[j] = f - 2;
			} else
				maingetflag(f-2,f1,i);
			break;
		case 's':
			if (*f != '[')
				vocd_isDefaultMOR = FALSE;
			if (VOCD_TYPE_D) {
				for (j=0; j < NUMSOPTIONS && numer_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				numer_s_options[j] = f - 2;
				for (j=0; j < NUMSOPTIONS && denom_s_options[j] != NULL; j++) ;
				if (j == NUMSOPTIONS) {
					fprintf(stderr,"Exceeded number of allowed options; more than %d.\n", NUMSOPTIONS);
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				denom_s_options[j] = f - 2;
			} else
				maingetflag(f-2,f1,i);
			break;
		case 't':
			if (*f == '%') {
				vocd_isDefaultMOR = FALSE;
			}
			maingetflag(f-2,f1,i);
			break;
/*
		case 't':
			if (*(f-2) == '+' && *f != '%' && *f != '@' && *f != '#') {
				char sp[SPEAKERLEN];
				if (vocd_isCombineSpeakers) {
					if (*f == '*')
						strcpy(sp, f);
					else {
						strcpy(sp, "*");
						strcat(sp, f);
					}
					if (speaker_add_speaker(&speakers, sp, NULL, FALSE, TRUE) == NULL) { // add default speaker
						fprintf(stderr,"cannot add speaker to list\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						if (VOCD_TYPE_D)
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					}
					if (VOCD_TYPE_D) {
						if (speaker_add_speaker(&denom_speakers, sp, NULL, FALSE, TRUE) == NULL) { // add default speaker
							fprintf(stderr,"cannot add speaker to list\n");
							speakers = speaker_free_up_speakers(speakers, TRUE);
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
							cutt_exit(0);
						}
					}
					isSpeakerSpecified = TRUE;
				}
			}
*/
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
#endif // KIDEVAL_LIB
/*
 * int generate_d_optimum  (VOCDSPs *speakers, FNType *input_fname, double *d_optimum);
 *
 * The main function for generating the D_optimum values
 */
static int generate_d_optimum (VOCDSP *speaker, FNType *input_fname, double *d_optimum, char isOutput) {
//	VOCDSPs *Spk_pt=NULL;
	int i, t;
	double ttr;
	int types, tokens;
	int utterance_count;
	FILE * ofp;

	if (isOutput)
		ofp = fpout;
	else
		ofp = NULL;
	// Output from the program
	// Count the number of utterances; if 0, quit. - no point in proceeding.
//	Spk_pt = speakers;
	utterance_count = 0;
//	while (Spk_pt != NULL) {
		utterance_count += speaker_linecount(speaker);
//		utterance_count += speaker_linecount(Spk_pt->speaker);
//		Spk_pt = Spk_pt->next;
//	}
	if (utterance_count==0) {
		if (vocd_show_warnings) {
			fprintf(stderr,"\n*** File \"%s\": Speaker: \"%s\"\n", input_fname, speaker->code);
			fprintf(stderr,"vocd: WARNING: Utterances count for this speaker = 0\n\n");
		}
		if (ofp != NULL && onlydata == 4) {
			if (!SEQUENTIAL_SAMPLING || d_flags & RANDOM_TTR)
				t = MAX_TRIALS;
			else
				t = 1;
			if (speaker->fname != NULL)
				outputStringForExcel(ofp, speaker->fname, 1);
			else {
				char *f;
// ".xls"
				strcpy(FileName2, oldfname);
				f = strrchr(FileName2, PATHDELIMCHR);
				if (f != NULL) {
					f++;
					strcpy(FileName2, f);
				}
				f = strchr(FileName2, '.');
				if (f != NULL)
					*f = EOS;
				if (!combinput || onlydata == 4)
					fprintf(ofp,"%s,", FileName2);
				else
					fprintf(ofp,"%s,", POOLED_FNAME);
			}
			if (speaker->IDs != NULL)
				outputIDForExcel(ofp, speaker->IDs, 1);
			else {
				fprintf(ofp,".,.,%s,.,.,.,.,.,.,.,.,", speaker->code);
			}
			fprintf(ofp,".,.,.,");
			for (i=0;i<t;i++) {
				fprintf(ofp,".,");
			}
			fprintf(ofp,".\n");
		}
//2013-02-08		if (ofp == NULL || onlydata == 4)
			return 1;
//		else
//			return -1;
	}
	// print the speaker utterances
	if (ofp != NULL) {
		if (onlydata == 0 || onlydata == 1) {
//			for (Spk_pt=speakers; Spk_pt != NULL; Spk_pt=Spk_pt->next) {
				speaker_fprint_speaker(ofp, speaker);
//				speaker_fprint_speaker(ofp, Spk_pt->speaker);
//			}
		}
	}
	if (onlydata == 1 || onlydata == 3) {
		// Compute TTR on the full sequence
//		Spk_pt = speakers;
		ttr = speaker_ttr(speaker, &types, &tokens);
//		ttr = speaker_ttr(Spk_pt->speaker, &types, &tokens);
		if (ofp != NULL) {
			fprintf(ofp,"\n");
			fprintf(ofp,"VOCD RESULTS SUMMARY\n");
			fprintf(ofp,"====================\n");
			fprintf(ofp,"Types, Tokens, TTR:  <%i,%i,%f>\n", types, tokens, ttr);
			fprintf(ofp,"\n");
		}
	}
	if (onlydata == 0 || onlydata == 2 || onlydata == 4) {
		// Copy everything into a sequence
		tkns_in_seq = speaker_create_sequence(speaker, &token_seq);
//		tkns_in_seq = speaker_create_sequence(speakers, &token_seq);
		if (SPLIT_HALF_ODD || SPLIT_HALF_EVEN) {  // take even or odd words
			int n;
			int no_split_tokens;
			
			if (SPLIT_HALF_EVEN)
				n = 0;
			else if (SPLIT_HALF_ODD)
				n = 1;
			else
				n = 0;
			
			no_split_tokens = tkns_in_seq/2 + (tkns_in_seq%2)*n;
			
			for (i=0;i<no_split_tokens;i++) {
				token_seq[i] = token_seq[i*2+1-n];
			}
			tkns_in_seq = no_split_tokens;
		}
		// compute the D_optimum values
		if (SEQUENTIAL_SAMPLING) 
			d_compute(speaker, token_seq,tkns_in_seq,D_from,D_to,D_inc,D_samples,d_flags,ofp,input_fname,d_optimum); 
		else
			d_compute(speaker, token_seq,tkns_in_seq,D_from,D_to,D_inc,D_samples,d_flags|RANDOM_TTR,ofp,input_fname,d_optimum); 
	}  // end if (!TTR) ...
	return 1;
}
/*
 * void filter_rest (VOCDSP *speaker);
 * All of the filtering is directed from this function.
 */
static void filter_rest(VOCDSP *speaker) {
//	VOCDSPs *Spk_pt=NULL;
	Line  *lpt;
	NODE  *exclude;
/*
	NODE  *overlap;
	char  **words;
	int   i;
	overlap = tokens_create_node(2);
	tokens_insert(overlap, "[<]");
//	Spk_pt = speakers;
//	while (Spk_pt!=NULL) {
		lpt = speaker->firstline;
//		lpt = Spk_pt->speaker->firstline;
		while (lpt!=NULL) {
			words = lpt->words;
			for (i=0;i<lpt->no_words; i++) 
				if (tokens_member(words[i], overlap)) {
					filter_overlapping ( lpt, 0 );
					break;
				}
			lpt = lpt->nextline;
		}
//		Spk_pt = Spk_pt->next;
//	}
*/
	// Filter empty strings; long winded, but consistent and should clean up all empty strings from the list of utterances
	exclude = tokens_create_node(NO_SUBNODES);
	tokens_insert(exclude, "");
//	Spk_pt = speakers;
//	while (Spk_pt!=NULL) {
		lpt = speaker->firstline;
//		lpt = Spk_pt->speaker->firstline;
		while (lpt!=NULL) {
			filter_exclude(lpt,exclude);
			lpt = lpt->nextline;
		}
//		Spk_pt = Spk_pt->next;
//	}
	tokens_free_tree(exclude);
	// Select capitalisation words only
	if (CAPITALISATION)
		filter_capitalisation(speaker);
}

char vocd_analize(char isOutput) {
	VOCDSPs *Spk_pt, *D_Spk_pt;

	if (!isDoVocdAnalises)
		return(TRUE);

	isDoVocdAnalises = FALSE;
	if (VOCD_TYPE_D) {
		Spk_pt = speakers;
		D_Spk_pt = denom_speakers;
		while (Spk_pt!=NULL && D_Spk_pt != NULL) {
			if (Spk_pt->speaker->isUsed) {
				filter_rest(Spk_pt->speaker);
				Spk_pt->speaker->d_optimum=0.0;
				if (!combinput || onlydata == 4 || !isOutput)
					strcpy(FileName1, oldfname);
				else
					strcpy(FileName1, POOLED_FNAME);
				if (isOutput && onlydata != 4 && speakers->next != NULL)
					fprintf(fpout,"****** Speaker: %s\n", Spk_pt->speaker->code);
				if (generate_d_optimum(Spk_pt->speaker,FileName1,&Spk_pt->speaker->d_optimum,isOutput) == -1) {
					return(FALSE);
				}
				filter_rest(D_Spk_pt->speaker);
				Spk_pt->speaker->denom_d_optimum = 0.0;
				if (!combinput || onlydata == 4 || !isOutput)
					strcpy(FileName1, oldfname);
				else
					strcpy(FileName1, POOLED_FNAME);
				if (generate_d_optimum(D_Spk_pt->speaker,FileName1,&Spk_pt->speaker->denom_d_optimum,isOutput) == -1) {
					return(FALSE);
				}
				if (isOutput) {
					fprintf(fpout,"\n");
					fprintf(fpout,"VOCD: LIMITING-TYPE-TYPE-RATIO RESULTS SUMMARY\n");
					fprintf(fpout,"==============================================\n");
					fprintf(fpout,"          Numerator:  %s\n", numer_directives);
					fprintf(fpout,"        Denominator:  %s\n", denom_directives);
					fprintf(fpout,"    Numerator D_optimum:  %.2f\n", Spk_pt->speaker->d_optimum);
					fprintf(fpout,"  Denominator D_optimum:  %.2f\n", Spk_pt->speaker->denom_d_optimum);
					fprintf(fpout,"  SQRT(Num.)/SQRT(Den.):  %.2f\n", sqrt(Spk_pt->speaker->d_optimum)/sqrt(Spk_pt->speaker->denom_d_optimum));
					fprintf(fpout,"\n");
				}
			}
			Spk_pt = Spk_pt->next;
			D_Spk_pt = D_Spk_pt->next;
		}
	} else {
		Spk_pt = speakers;
		while (Spk_pt!=NULL) {
			if (Spk_pt->speaker->isUsed) {
				filter_rest(Spk_pt->speaker);
				Spk_pt->speaker->d_optimum=0.0;
				if (!combinput || onlydata == 4 || !isOutput)
					strcpy(FileName1, oldfname);
				else
					strcpy(FileName1, POOLED_FNAME);
				if (isOutput && onlydata != 4 && speakers->next != NULL)
					fprintf(fpout,"****** Speaker: %s\n", Spk_pt->speaker->code);
				if (generate_d_optimum(Spk_pt->speaker, FileName1, &Spk_pt->speaker->d_optimum, isOutput) == -1) {
					return(FALSE);
				}
			}
			Spk_pt = Spk_pt->next;
		}
	}
	speakers = speaker_free_up_speakers(speakers, FALSE);
	if (VOCD_TYPE_D)
		denom_speakers = speaker_free_up_speakers(denom_speakers, FALSE);
	return(TRUE);
}

static void vocd_pr_result(void) {
	if (!vocd_analize(TRUE)) {
		speakers = speaker_free_up_speakers(speakers, TRUE);
		if (VOCD_TYPE_D)
			denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
		cutt_exit(0);
	}
}

static void vocd_createIDInfo(char *s, char *fname, char *IDs) {
	int  i, cnt;
	char isBlank, *f;

	cnt = 0;
	isBlank = TRUE;
	strcpy(FileName1, oldfname);
	f = strrchr(FileName1, PATHDELIMCHR);
	if (f != NULL) {
		f++;
		strcpy(FileName1, f);
	}
	f = strchr(FileName1, '.');
	if (f != NULL)
		*f = EOS;
	strcpy(fname, FileName1);
	i = 0;
	for (; *s != EOS; s++) {
		IDs[i++] = *s;
	}
	IDs[i] = EOS;
}

void clean_speakers(VOCDSPs *Spk_pt, char isSubstitute) {
	if (Spk_pt == NULL && isSubstitute)
		Spk_pt = speakers;
	while (Spk_pt!=NULL) {
		Line *lpt, *tmp_line;
		lpt = Spk_pt->speaker->firstline;
		Spk_pt->speaker->firstline = NULL;
		Spk_pt->speaker->lastline = NULL;
		while (lpt!=NULL) {
			tmp_line = lpt;
			lpt = lpt->nextline;
			speaker_free_line(tmp_line);
		}
		Spk_pt = Spk_pt->next;
	}
}

#ifndef KIDEVAL_LIB
void call() {
	int i, t;
	char ts[256];
	VOCDSP  *current_speaker = NULL;
	long tlineno = 0;

	if (vocd_isPrintIDHeader) {
		vocd_isPrintIDHeader = FALSE;
		fprintf(fpout, "%c%c%c", 0xef, 0xbb, 0xbf); // BOM UTF8
		fprintf(fpout,"File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field,");
		fprintf(fpout,"Types,Tokens,TTR,");
		if (!SEQUENTIAL_SAMPLING || d_flags & RANDOM_TTR)
			t = MAX_TRIALS;
		else
			t = 1;
		for (i=0;i<t;i++) {
			fprintf(fpout,"D_optimum_values %d,", i+1);
		}
		fprintf(fpout,"D_optimum_average\n");
	}
	if (!combinput || onlydata == 4) {
		clean_speakers(speakers, FALSE);
	}
	if (VOCD_TYPE_D) {
		clean_s_option();
		default_excludes();
		for (i=0; i < NUMSOPTIONS && numer_s_options[i] != NULL; i++) {
			ts[0] = EOS;
			if (numer_s_options[i][1] == 'g') {
				spareTier1[0] = numer_s_options[i][0];
				strcpy(spareTier1+1, "s");
				strcat(spareTier1+1, numer_s_options[i]+3);
				maingetflag(spareTier1,ts,&t);
			} else
				maingetflag(numer_s_options[i],ts,&t);
		}
		if (isMORSearch() && chatmode)
			maketierchoice("%mor",'+',FALSE);
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
#if !defined(CLAN_SRV)
			fprintf(stderr,"\r%ld ",lineno);
#endif
		}
		if (onlydata == 4) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					vocd_createIDInfo(utterance->line, templineC1, templineC2);
					if (speaker_add_speaker(&speakers, templineC, templineC1, templineC2, !isSpeakerSpecified, FALSE) == NULL) {
						fprintf(stderr,"cannot add speaker to list\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						if (VOCD_TYPE_D)
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					}
				}
				continue;
			}
		}
		if (*utterance->speaker == '*') {
			current_speaker = NULL;
			if (checktier(utterance->speaker)) {
				isDoVocdAnalises = TRUE;
				if ((current_speaker=speaker_add_speaker(&speakers, utterance->speaker, NULL, NULL, !isSpeakerSpecified, TRUE)) == NULL) {
					fprintf(stderr,"cannot add speaker to list\n");
					speakers = speaker_free_up_speakers(speakers, TRUE);
					if (VOCD_TYPE_D)
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
					cutt_exit(0);
				}
				if (!nomain && current_speaker != NULL) {
					uS.remblanks(uttline);
					for (i=0; uttline[i] != EOS; i++) {
						if (uS.isskip(uttline,i,&dFnt,MBF))
							uttline[i] = ' ';
					}
					if (!speaker_add_line(current_speaker, uttline, lineno)) {
						fprintf(stderr,"cannot add line to current speaker\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						if (VOCD_TYPE_D)
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					} 
				}				
			}
		} else if (*utterance->speaker == '%') {
			if (checktier(utterance->speaker)) {
				isDoVocdAnalises = TRUE;
				if (current_speaker != NULL) {
					uS.remblanks(uttline);
					for (i=0; uttline[i] != EOS; i++) {
						if (uS.isskip(uttline,i,&dFnt,MBF))
							uttline[i] = ' ';
					}
					if (!speaker_add_line(current_speaker, uttline, lineno)) {
						fprintf(stderr,"cannot add line to current speaker\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						if (VOCD_TYPE_D)
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					}
				}
			}
		}
	}
#if !defined(CLAN_SRV)
	fprintf(stderr, "\r	  \r");
#endif
	if (VOCD_TYPE_D) {
		rewind(fpin);
		clean_s_option();
		default_excludes();
		for (i=0; i < NUMSOPTIONS && denom_s_options[i] != NULL; i++) {
			ts[0] = EOS;
			if (denom_s_options[i][1] == 'g') {
				spareTier1[0] = denom_s_options[i][0];
				strcpy(spareTier1+1, "s");
				strcat(spareTier1+1, denom_s_options[i]+3);
				maingetflag(spareTier1,ts,&t);
			} else
				maingetflag(denom_s_options[i],ts,&t);
			if (isMORSearch() && chatmode)
				maketierchoice("%mor",'+',FALSE);
		}
		if (!combinput || onlydata == 4) {
			clean_speakers(denom_speakers, FALSE);
		}
		tlineno = 0;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (lineno > tlineno) {
				tlineno = lineno + 200;
#if !defined(CLAN_SRV)
				fprintf(stderr,"\r%ld ",lineno);
#endif
			}
			if (*utterance->speaker == '*') {
				current_speaker = NULL;
				if (checktier(utterance->speaker)) {
					if ((current_speaker=speaker_add_speaker(&denom_speakers, utterance->speaker, NULL, NULL, !isSpeakerSpecified, TRUE)) == NULL) {
						fprintf(stderr,"cannot add speaker to list\n");
						speakers = speaker_free_up_speakers(speakers, TRUE);
						denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
						cutt_exit(0);
					}
					if (!nomain && current_speaker != NULL) {
						uS.remblanks(uttline);
						for (i=0; uttline[i] != EOS; i++) {
							if (uS.isskip(uttline,i,&dFnt,MBF))
								uttline[i] = ' ';
						}
						if (!speaker_add_line(current_speaker, uttline, lineno)) {
							fprintf(stderr,"cannot add line to current speaker\n");
							speakers = speaker_free_up_speakers(speakers, TRUE);
							denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
							cutt_exit(0);
						} 
					}				
				}
			} else if (*utterance->speaker == '%') {
				if (checktier(utterance->speaker)) {
					if (current_speaker != NULL) {
						uS.remblanks(uttline);
						for (i=0; uttline[i] != EOS; i++) {
							if (uS.isskip(uttline,i,&dFnt,MBF))
								uttline[i] = ' ';
						}
						if (!speaker_add_line(current_speaker, uttline, lineno)) {
							fprintf(stderr,"cannot add line\n");
							break;
						}
					}
				}
			}
		}
		fprintf(stderr,"\n");
	}
	if (!combinput || onlydata == 4) {
		if (!vocd_analize(TRUE)) {
			speakers = speaker_free_up_speakers(speakers, TRUE);
			if (VOCD_TYPE_D)
				denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
			cutt_exit(0);
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[])
{
	int i;
	time_t t;

#ifdef _CLAN_DEBUG
	t = 1283562804L; // RAND
#else
	t = time(NULL);
#endif
	srand(t);    // seed the random number generator
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = VOCD;
	chatmode = CHAT_MODE;
	OnlydataLimit = 4;
	UttlineEqUtterance = FALSE;
	VOCD_TYPE_D = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+' || *argv[i] == '-') {
			if (argv[i][1] == 'g') {
				VOCD_TYPE_D = TRUE;
				break;
			}
		}
	}	
	bmain(argc,argv,vocd_pr_result);
}
#endif // KIDEVAL_LIB
