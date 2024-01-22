/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#ifdef UNX
	#define RGBColor int
#endif

#if !defined(UNX)
#define _main wdlen_main
#define call wdlen_call
#define getflag wdlen_getflag
#define init wdlen_init
#define usage wdlen_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char R8;
extern char OverWriteFile;
extern char Parans;

#define numberOfElements 9
#define ARLEN 80

#define WDLNGS 0
#define MLNGS 1
#define UTLNG 2
#define MORPH 3
#define TUTTS 4
#define TWRDS 5
#define CUTTS 6
#define SLNGS 7
#define SYLLB 8

#define BOTHTIERS 1
#define SPONLYTRS 2
#define DPONLYTRS 3

#define WDLENSP struct wdlen_cnt
struct wdlen_cnt {
	char sp[SPEAKERLEN];
	int  stats[numberOfElements][ARLEN];
	int  syllablecnt;
	int  morphcnt;
	int  wordcnt;
	WDLENSP *next_sp;
} ;

#define W_UTT struct indent_utterance
W_UTT {
    char    *sp;
    char    *line;
	char    *mline;
	int		ClauseCnt;
    W_UTT *nextUtt;
} ;

#define WDLEN_CLAUSES struct clause
WDLEN_CLAUSES {
	char *st;
	int  len;
	WDLEN_CLAUSES *next_clause;
} ;

static char wdlen_isXXXFound = FALSE;
static char wdlen_isYYYFound = FALSE;
static char whichTier, wdlen_ftime, wdlen_ftime2;
static WDLENSP *wdlen_head;
static WDLEN_CLAUSES *wdlen_clause;

void usage() {
	puts("WDLEN gives length histograms for characters, morphemes, words, utterances and turns.");
#ifdef UNX
	printf("WDLEN REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cWDLEN REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	printf("Usage: wdlen [a bS cS %s] filename(s)\n",mainflgs());
	puts("+a : compute both words and morphemes from dependent tier only");
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
	puts("+cS: look for clause marker S or markers listed in file @S");
	mainusage(FALSE);
#ifdef UNX
	printf("WDLEN REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cWDLEN REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

void init(char f) {
	int i;

	if (f) {
		Parans = 3;
		if (chatmode)
			whichTier = BOTHTIERS;
		else
			whichTier = SPONLYTRS;
		wdlen_head = NULL;
		wdlen_clause = NULL;
		wdlen_isXXXFound = FALSE;
		wdlen_isYYYFound = FALSE;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+$*");
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		mor_initwords();
		wdlen_ftime = TRUE;
		wdlen_ftime2 = TRUE;
	} else {
		if (wdlen_ftime) {
			wdlen_ftime = FALSE;
			if (chatmode && whichTier != SPONLYTRS) {
				maketierchoice("%mor",'+',FALSE);
				if (isMORSearch())
					linkMain2Mor = TRUE;
			}
			if (R8)
				linkMain2Mor = FALSE;
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
		}
		if (onlydata == 1) {
			if (!f_override)
				stout = FALSE;
			AddCEXExtension = ".xls";
			OverWriteFile = TRUE;
		}
	}
}

static WDLENSP *wdlen_free_root(WDLENSP *p) {
	WDLENSP *t;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		free(t);
	}
	return(NULL);
}


static WDLEN_CLAUSES *wdlen_free_clause(WDLEN_CLAUSES *p) {
	WDLEN_CLAUSES *t;

	while (p != NULL) {
		t = p;
		p = p->next_clause;
		if (t->st != NULL)
			free(t->st);
		free(t);
	}
	return(NULL);
}

static void wdlen_AddClause(char *s) {
	WDLEN_CLAUSES *tc;

	if ((tc=NEW(WDLEN_CLAUSES)) == NULL)
		out_of_mem();
	if ((tc->st=(char *)malloc(strlen(s)+1)) == NULL)
		out_of_mem();
	strcpy(tc->st, s);
	tc->len = strlen(tc->st);
	tc->next_clause = wdlen_clause;
	wdlen_clause = tc;
}

static void wdlen_AddClauseFromFile(FNType *fname) {
	FILE *fp;
	char wd[256];
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the delemeter files:\n\t\"%s\", \"%s\"\n",fname,mFileName);
		cutt_exit(0);
	}
	while (fgets_cr(wd, 255, fp)) {
		if (uS.isUTF8(wd) || uS.isInvisibleHeader(wd))
			continue;
		wdlen_AddClause(wd);
	}
	fclose(fp);
}


void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'a':
			no_arg_option(f);
			whichTier = DPONLYTRS;
			break;
		case 'b':
			morf = getfarg(f,f1,i);
			if (*(f-2) == '-') {
				if (*morf != EOS) {
					for (j=0; rootmorf[j] != EOS; ) {
						if (uS.isCharInMorf(rootmorf[j],morf)) 
							strcpy(rootmorf+j,rootmorf+j+1);
						else
							j++;
					}
				} else
					rootmorf[0] = EOS;
			} else {
				if (*morf != EOS) {
					t = rootmorf;
					rootmorf = (char *)malloc(strlen(t)+strlen(morf)+1);
					if (rootmorf == NULL) {
						fprintf(stderr,"No more space left in core.\n");
						cutt_exit(0);
					}
					strcpy(rootmorf,t);
					strcat(rootmorf,morf);
					free(t);
				}
			}
			break;
		case 'c':
			if (!*f) {
				fprintf(stderr,"Specify clause delemeters after +c option.\n");
				cutt_exit(0);
			}
			if (*f == '@') {
				f++;
				wdlen_AddClauseFromFile(f);
			} else {
				if (*f == '\\')
					f++;
				wdlen_AddClause(f);
			}
			break;
		case 't':
			if (chatmode) {
				if (*(f-2) == '+' && *f == '%') {
					wdlen_ftime2 = FALSE;
					linkMain2Mor = FALSE;
				} else if (*(f-2) == '-' && *f == '%') {
					if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:")) {
						wdlen_ftime2 = FALSE;
						whichTier = SPONLYTRS;
						linkMain2Mor = FALSE;
					}
				} else if (*(f-2) == '-' && !strcmp(f, "*")) {
					whichTier = DPONLYTRS;
				}
				maingetflag(f-2,f1,i);
			} else {
				fputs("+/-t option is not allowed with TEXT format\n",stderr);
				cutt_exit(0);
			}
			break;
		case 's':
			t = getfarg(f,f1,i);
			j = strlen(t) - 3;
			if (uS.mStricmp(t+j, "xxx") == 0) {
				if (*(f-2) == '+')
					wdlen_isXXXFound = TRUE;
				else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					cutt_exit(0);
				}
				break;
			} else if (uS.mStricmp(t+j, "yyy") == 0) {
				if (*(f-2) == '+')
					wdlen_isYYYFound = TRUE;
				else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					cutt_exit(0);
				}
				break;
			} else if (uS.mStricmp(t+j, "yyy") == 0 || uS.mStricmp(t+j, "www") == 0) {
				fprintf(stderr,"%s \"%s\" is not allowed.\n", ((*(f-2) == '+') ? "Including" : "Excluding"), f);
				cutt_exit(0);
				break;
			} else if (wdlen_ftime2)
				linkMain2Mor = TRUE;
		case 'd':
			if (*f == EOS) {
				combinput = TRUE;
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int computeLimit(int num) {
	int i, lim;
	WDLENSP *ts;

	lim = 1;
	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		for (i=ARLEN-1; i > 1; i--) {
			if (ts->stats[num][i] != 0)
				break;
		}
		if (i > lim)
			lim = i;
	}
	lim++;
	return(lim);
}

static void wdlen_pr_result(void) {
	int i, lim;
	char  fontName[256];
	double sum, num;
	WDLENSP *ts;

	if (onlydata == 1)
		fprintf(fpout,"Number\tof\twords\tof\teach\tlength\tin\tcharacters\n");
	else {
		if (!stout) {
#ifdef _WIN32 
			fprintf(fpout, "%s	Win95:CAfont:-15:0", FONTHEADER);
#else
			fprintf(fpout, "%s	CAfont:13:7", FONTHEADER);
#endif
		} else {
			strcpy(fontName, "CAfont:13:7\n");
			cutt_SetNewFont(fontName, EOS);
		}
		fprintf(fpout,"\nNumber of words of each length in characters\n");
	}
	lim = computeLimit(WDLNGS);
	if (onlydata == 1)
		fprintf(fpout,"lengths\t");
	else
		fprintf(fpout,"lengths ");
	for (i=1; i < lim; i++) {
		if (onlydata == 1)
			fprintf(fpout,"%d\t", i);
		else
			fprintf(fpout," %4d", i);
	}
	if (onlydata == 1) {
		if (lim == ARLEN)
			fprintf(fpout,"+ Mean\n");
		else 
			fprintf(fpout,"Mean\n");
	} else {
		if (lim == ARLEN)
			fprintf(fpout,"+  Mean\n");
		else 
			fprintf(fpout,"   Mean\n");
	}
	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		sum = 0.0;
		num = 0.0;		
		if (onlydata == 1)
			fprintf(fpout,"%s\t", ts->sp);
		else {
			strncpy(templineC, ts->sp, 8);
			templineC[8] = EOS;
			fprintf(fpout,"%-8s", templineC);
		}
		for (i=1; i < lim; i++) {
			sum += ((double)i * (double)ts->stats[WDLNGS][i]);
			num += (double)ts->stats[WDLNGS][i];
			if (onlydata == 1)
				fprintf(fpout,"%d\t", ts->stats[WDLNGS][i]);
			else
				fprintf(fpout," %4d", ts->stats[WDLNGS][i]);
		}
		if (onlydata == 1) {
			if (num > 0.0)
				fprintf(fpout, "%f\n", sum/num);
			else
				fprintf(fpout, "%f\n", 0.0);
		} else {
			if (num > 0.0)
				fprintf(fpout, " %6.3f\n", sum/num);
			else
				fprintf(fpout, " %6.3f\n", 0.0);
		}
	}
	fprintf(fpout,"-------\n");
	if (onlydata == 1)
		fprintf(fpout,"\nNumber\tof\tutterances\tof\teach\tof\tthese\tlengths\tin\twords\n");
	else
		fprintf(fpout,"\nNumber of utterances of each of these lengths in words\n");
	lim = computeLimit(UTLNG);
	if (onlydata == 1)
		fprintf(fpout,"lengths\t");
	else
		fprintf(fpout,"lengths ");
	for (i=1; i < lim; i++) {
		if (onlydata == 1)
			fprintf(fpout,"%d\t", i);
		else
			fprintf(fpout," %4d", i);
	}
	if (onlydata == 1) {
		if (lim == ARLEN)
			fprintf(fpout,"+ Mean\n");
		else 
			fprintf(fpout,"Mean\n");
	} else {
		if (lim == ARLEN)
			fprintf(fpout,"+  Mean\n");
		else 
			fprintf(fpout,"   Mean\n");
	}
	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		sum = 0.0;
		num = 0.0;		
		if (onlydata == 1)
			fprintf(fpout,"%s\t", ts->sp);
		else {
			strncpy(templineC, ts->sp, 8);
			templineC[8] = EOS;
			fprintf(fpout,"%-8s", templineC);
		}
		for (i=1; i < lim; i++) {
			sum += ((double)i * (double)ts->stats[UTLNG][i]);
			num += (double)ts->stats[UTLNG][i];
			if (onlydata == 1)
				fprintf(fpout,"%d\t", ts->stats[UTLNG][i]);
			else
				fprintf(fpout," %4d", ts->stats[UTLNG][i]);
		}
		if (onlydata == 1) {
			if (num > 0.0)
				fprintf(fpout, "%f\n", sum/num);
			else
				fprintf(fpout, "%f\n", 0.0);
		} else {
			if (num > 0.0)
				fprintf(fpout, " %6.3f\n", sum/num);
			else
				fprintf(fpout, " %6.3f\n", 0.0);
		}
	}
	if (wdlen_clause != NULL) {
		fprintf(fpout,"-------\n");
		if (onlydata == 1)
			fprintf(fpout,"\nNumber\tof\tutterances\tof\teach\tof\tthese\tlengths\tin\tclauses\n");
		else
			fprintf(fpout,"\nNumber of utterances of each of these lengths in clauses\n");
		lim = computeLimit(CUTTS);
		if (onlydata == 1)
			fprintf(fpout,"lengths\t");
		else
			fprintf(fpout,"lengths ");
		for (i=0; i < lim; i++) {
			if (onlydata == 1)
				fprintf(fpout,"%d\t", i);
			else
				fprintf(fpout," %4d", i);
		}
		if (onlydata == 1) {
			if (lim == ARLEN)
				fprintf(fpout,"+ Mean\n");
			else
				fprintf(fpout,"Mean\n");
		} else {
			if (lim == ARLEN)
				fprintf(fpout,"+  Mean\n");
			else
				fprintf(fpout,"   Mean\n");
		}
		for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
			sum = 0.0;
			num = 0.0;
			if (onlydata == 1)
				fprintf(fpout,"%s\t", ts->sp);
			else {
				strncpy(templineC, ts->sp, 8);
				templineC[8] = EOS;
				fprintf(fpout,"%-8s", templineC);
			}
			for (i=0; i < lim; i++) {
				sum += ((double)i * (double)ts->stats[CUTTS][i]);
				num += (double)ts->stats[CUTTS][i];
				if (onlydata == 1)
					fprintf(fpout,"%d\t", ts->stats[CUTTS][i]);
				else
					fprintf(fpout," %4d", ts->stats[CUTTS][i]);
			}
			if (onlydata == 1) {
				if (num > 0.0)
					fprintf(fpout, "%f\n", sum/num);
				else
					fprintf(fpout, "%f\n", 0.0);
			} else {
				if (num > 0.0)
					fprintf(fpout, " %6.3f\n", sum/num);
				else
					fprintf(fpout, " %6.3f\n", 0.0);
			}
		}
	}
	if (chatmode) {
		fprintf(fpout,"-------\n");
		if (onlydata == 1)
			fprintf(fpout,"\nNumber\tof\tsingle\tturns\tof\teach\tof\tthese\tlengths\tin\tutterances\n");
		else
			fprintf(fpout,"\nNumber of single turns of each of these lengths in utterances\n");
		lim = computeLimit(TUTTS);
		if (onlydata == 1)
			fprintf(fpout,"lengths\t");
		else
			fprintf(fpout,"lengths ");
		for (i=1; i < lim; i++) {
			if (onlydata == 1)
				fprintf(fpout,"%d\t", i);
			else
				fprintf(fpout," %4d", i);
		}
		if (onlydata == 1) {
			if (lim == ARLEN)
				fprintf(fpout,"+ Mean\n");
			else 
				fprintf(fpout,"Mean\n");
		} else {
			if (lim == ARLEN)
				fprintf(fpout,"+  Mean\n");
			else 
				fprintf(fpout,"   Mean\n");
		}
		for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
			sum = 0.0;
			num = 0.0;		
			if (onlydata == 1)
				fprintf(fpout,"%s\t", ts->sp);
			else {
				strncpy(templineC, ts->sp, 8);
				templineC[8] = EOS;
				fprintf(fpout,"%-8s", templineC);
			}
			for (i=1; i < lim; i++) {
				sum += ((double)i * (double)ts->stats[TUTTS][i]);
				num += (double)ts->stats[TUTTS][i];
				if (onlydata == 1)
					fprintf(fpout,"%d\t", ts->stats[TUTTS][i]);
				else
					fprintf(fpout," %4d", ts->stats[TUTTS][i]);
			}
			if (onlydata == 1) {
				if (num > 0.0)
					fprintf(fpout, "%f\n", sum/num);
				else
					fprintf(fpout, "%f\n", 0.0);
			} else {
				if (num > 0.0)
					fprintf(fpout, " %6.3f\n", sum/num);
				else
					fprintf(fpout, " %6.3f\n", 0.0);
			}
		}
		fprintf(fpout,"-------\n");
		if (onlydata == 1)
			fprintf(fpout,"\nNumber\tof\tsingle\tturns\tof\teach\tof\tthese\tlengths\tin\twords\n");
		else
			fprintf(fpout,"\nNumber of single turns of each of these lengths in words\n");
		lim = computeLimit(TWRDS);
		if (onlydata == 1)
			fprintf(fpout,"lengths\t");
		else
			fprintf(fpout,"lengths ");
		for (i=1; i < lim; i++) {
			if (onlydata == 1)
				fprintf(fpout,"%d\t", i);
			else
				fprintf(fpout," %4d", i);
		}
		if (onlydata == 1) {
			if (lim == ARLEN)
				fprintf(fpout,"+ Mean\n");
			else 
				fprintf(fpout,"Mean\n");
		} else {
			if (lim == ARLEN)
				fprintf(fpout,"+  Mean\n");
			else 
				fprintf(fpout,"   Mean\n");
		}
		for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
			sum = 0.0;
			num = 0.0;		
			if (onlydata == 1)
				fprintf(fpout,"%s\t", ts->sp);
			else {
				strncpy(templineC, ts->sp, 8);
				templineC[8] = EOS;
				fprintf(fpout,"%-8s", templineC);
			}
			for (i=1; i < lim; i++) {
				sum += ((double)i * (double)ts->stats[TWRDS][i]);
				num += (double)ts->stats[TWRDS][i];
				if (onlydata == 1)
					fprintf(fpout,"%d\t", ts->stats[TWRDS][i]);
				else
					fprintf(fpout," %4d", ts->stats[TWRDS][i]);
			}
			if (onlydata == 1) {
				if (num > 0.0)
					fprintf(fpout, "%f\n", sum/num);
				else
					fprintf(fpout, "%f\n", 0.0);
			} else {
				if (num > 0.0)
					fprintf(fpout, " %6.3f\n", sum/num);
				else
					fprintf(fpout, " %6.3f\n", 0.0);
			}
		}
	}
	fprintf(fpout,"-------\n");
	if (onlydata == 1)
		fprintf(fpout,"\nNumber\tof\twords\tof\teach\tof\tthese\tmorpheme\tlengths\n");
	else
		fprintf(fpout,"\nNumber of words of each of these morpheme lengths\n");
	lim = computeLimit(MLNGS);
	if (onlydata == 1)
		fprintf(fpout,"lengths\t");
	else
		fprintf(fpout,"lengths ");
	for (i=1; i < lim; i++) {
		if (onlydata == 1)
			fprintf(fpout,"%d\t", i);
		else
			fprintf(fpout," %4d", i);
	}
	if (onlydata == 1) {
		if (lim == ARLEN)
			fprintf(fpout,"+ Mean\n");
		else 
			fprintf(fpout,"Mean\n");
	} else {
		if (lim == ARLEN)
			fprintf(fpout,"+  Mean\n");
		else 
			fprintf(fpout,"   Mean\n");
	}
	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		sum = 0.0;
		num = 0.0;		
		if (onlydata == 1)
			fprintf(fpout,"%s\t", ts->sp);
		else {
			strncpy(templineC, ts->sp, 8);
			templineC[8] = EOS;
			fprintf(fpout,"%-8s", templineC);
		}
		for (i=1; i < lim; i++) {
			sum += ((double)i * (double)ts->stats[MLNGS][i]);
			num += (double)ts->stats[MLNGS][i];
			if (onlydata == 1)
				fprintf(fpout,"%d\t", ts->stats[MLNGS][i]);
			else
				fprintf(fpout," %4d", ts->stats[MLNGS][i]);
		}
		if (onlydata == 1) {
			if (num > 0.0)
				fprintf(fpout, "%f\n", sum/num);
			else
				fprintf(fpout, "%f\n", 0.0);
		} else {
			if (num > 0.0)
				fprintf(fpout, " %6.3f\n", sum/num);
			else
				fprintf(fpout, " %6.3f\n", 0.0);
		}
	}
	fprintf(fpout,"-------\n");
	if (onlydata == 1)
		fprintf(fpout,"\nNumber\tof\tutterances\tof\teach\tof\tthese\tlengths\tin\tmorphemes\n");
	else
		fprintf(fpout,"\nNumber of utterances of each of these lengths in morphemes\n");
	lim = computeLimit(MORPH);
	if (onlydata == 1)
		fprintf(fpout,"lengths\t");
	else
		fprintf(fpout,"lengths ");
	for (i=1; i < lim; i++) {
		if (onlydata == 1)
			fprintf(fpout,"%d\t", i);
		else
			fprintf(fpout," %4d", i);
	}
	if (onlydata == 1) {
		if (lim == ARLEN)
			fprintf(fpout,"+ Mean\n");
		else 
			fprintf(fpout,"Mean\n");
	} else {
		if (lim == ARLEN)
			fprintf(fpout,"+  Mean\n");
		else 
			fprintf(fpout,"   Mean\n");
	}
	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		sum = 0.0;
		num = 0.0;		
		if (onlydata == 1)
			fprintf(fpout,"%s\t", ts->sp);
		else {
			strncpy(templineC, ts->sp, 8);
			templineC[8] = EOS;
			fprintf(fpout,"%-8s", templineC);
		}
		for (i=1; i < lim; i++) {
			sum += ((double)i * (double)ts->stats[MORPH][i]);
			num += (double)ts->stats[MORPH][i];
			if (onlydata == 1)
				fprintf(fpout,"%d\t", ts->stats[MORPH][i]);
			else
				fprintf(fpout," %4d", ts->stats[MORPH][i]);
		}
		if (onlydata == 1) {
			if (num > 0.0)
				fprintf(fpout, "%f\n", sum/num);
			else
				fprintf(fpout, "%f\n", 0.0);
		} else {
			if (num > 0.0)
				fprintf(fpout, " %6.3f\n", sum/num);
			else
				fprintf(fpout, " %6.3f\n", 0.0);
		}
	}
	wdlen_head = wdlen_free_root(wdlen_head);
}

static WDLENSP *wdlen_FindSpeaker(const char *sp) {
	int i, j;
	WDLENSP *ts;

	for (ts=wdlen_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->sp, sp) == 0)
			return(ts);
	}
	if ((ts=NEW(WDLENSP)) == NULL)
		out_of_mem();
	strcpy(ts->sp, sp);
	ts->wordcnt = 0;
	ts->morphcnt = 0;
	ts->syllablecnt = 0;
	for (j=0; j < numberOfElements; j++) {
		for (i = 0; i < ARLEN; i++)
			ts->stats[j][i] = 0;
	}
	ts->next_sp = wdlen_head;
	wdlen_head = ts;
	return(ts);
}

static void words_count(char *line, WDLENSP *ts, int *turn_uttCnt, int *turn_wordCnt) {
	register int i;
	register int c;
	register int wlen;
	register char isSameDelim;

	c = 0;
	while (line[c] != EOS) {
		if (line[c] == '+' || uS.IsUtteranceDel(line, c)) {
			isSameDelim = FALSE;
			for (; line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF); c++) {
				if (!isSameDelim && uS.IsUtteranceDel(line, c)) {
					isSameDelim = TRUE;
					*turn_uttCnt = *turn_uttCnt + 1;
					*turn_wordCnt = *turn_wordCnt + ts->wordcnt;
					if (ts->wordcnt < ARLEN)
						ts->stats[UTLNG][ts->wordcnt]++;
					else
						ts->stats[UTLNG][ARLEN-1]++;
					ts->wordcnt = 0;
				}
			}
		} else {
			i = 0;
			wlen = 0;
			while (line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF)) {
				if (!uS.isRightChar(line, c, '+', &dFnt, MBF) && !uS.isRightChar(line, c, '-', &dFnt, MBF) && 
					!uS.isRightChar(line, c, '#', &dFnt, MBF) && !uS.isRightChar(line, c, '`', &dFnt, MBF) && 
					!uS.isRightChar(line, c, '~', &dFnt, MBF) && !uS.isRightChar(line, c, '$', &dFnt, MBF) &&
					!uS.isRightChar(line, c, '\'', &dFnt, MBF)) {
					if (uS.isRightChar(line, c, '@', &dFnt, MBF)) {
						while (line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF))
							templineC3[i++] = line[c++];
						break;
					} else if (uS.isRightChar(line, c, '^', &dFnt, MBF)) {
						templineC3[i++] = line[c++];
						if (line[c] == EOS)
							break;
					} else if (UTF8_IS_SINGLE((unsigned char)line[c]) || UTF8_IS_LEAD((unsigned char)line[c]))
						wlen++;
				}
				templineC3[i++] = line[c++];
			}
			templineC3[i] = EOS;
			if (i > 0 && exclude(templineC3)) {
				ts->wordcnt++;
				if (wlen < ARLEN)
					ts->stats[WDLNGS][wlen]++;
				else
					ts->stats[WDLNGS][ARLEN-1]++;
			}
		}
		for (; line[c] != EOS && uS.isskip(line,c,&dFnt,MBF); c++) ;
	}
}

static void morphs_count(char *line, WDLENSP *ts) {
	register int i;
	register int c;
	register int mlen;
	char isSameDelim, isAmbigFound;

	c = 0;
	while (line[c] != EOS) {
		if (line[c] == '+' || uS.IsUtteranceDel(line, c)) {
			isSameDelim = FALSE;
			for (; line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF); c++) {
				if (!isSameDelim && uS.IsUtteranceDel(line, c)) {
					isSameDelim = TRUE;
					if (ts->morphcnt < ARLEN)
						ts->stats[MORPH][ts->morphcnt]++;
					else
						ts->stats[MORPH][ARLEN-1]++;
					ts->morphcnt = 0;
				}
			}
		} else {
			isAmbigFound = FALSE;
			i = 0;
			mlen = 0;
			while (line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF)) {
				if (line[c] == '^')
					isAmbigFound = TRUE;
				if (uS.ismorfchar(line, c, &dFnt, rootmorf, MBF) && !isAmbigFound) {
					ts->morphcnt++;
					mlen++;
				}
				templineC3[i++] = line[c++];
			}
			templineC3[i] = EOS;
			if (i > 0 && exclude(templineC3)) {
				mlen++;
				ts->morphcnt++;
				if (mlen < ARLEN)
					ts->stats[MLNGS][mlen]++;
				else
					ts->stats[MLNGS][ARLEN-1]++;
			}
		}
		for (; line[c] != EOS && uS.isskip(line,c,&dFnt,MBF); c++) ;
	}
}

static WDLENSP *countTiers(WDLENSP *ts, W_UTT *cUT, char *oldSp, int *turn_uttCnt, int *turn_wordCnt) {
	if (!chatmode)
		ts = wdlen_FindSpeaker("GENERIC");
	else {
		if (uS.mStricmp(oldSp, cUT->sp)) {
			strcpy(oldSp, cUT->sp);
			if (ts != NULL) {
				if (*turn_uttCnt < ARLEN)
					ts->stats[TUTTS][(*turn_uttCnt)]++;
				else
					ts->stats[TUTTS][ARLEN-1]++;
				if (*turn_wordCnt < ARLEN)
					ts->stats[TWRDS][(*turn_wordCnt)]++;
				else
					ts->stats[TWRDS][ARLEN-1]++;
			}
			*turn_uttCnt = 0;
			*turn_wordCnt = 0;
		}
		ts = wdlen_FindSpeaker(cUT->sp);
	}
	if (whichTier == DPONLYTRS) {
		words_count(cUT->mline, ts, turn_uttCnt, turn_wordCnt);
	} else {
		words_count(cUT->line, ts, turn_uttCnt, turn_wordCnt);
	}
	ts->wordcnt = 0;
	ts->morphcnt = 0;
	if (whichTier == SPONLYTRS) {
		morphs_count(cUT->line, ts);
	} else {
		morphs_count(cUT->mline, ts);
	}
	*turn_wordCnt = *turn_wordCnt + ts->wordcnt;
	if (ts->wordcnt < ARLEN)
		ts->stats[UTLNG][ts->wordcnt]++;
	else
		ts->stats[UTLNG][ARLEN-1]++;
	if (ts->morphcnt < ARLEN)
		ts->stats[MORPH][ts->morphcnt]++;
	else
		ts->stats[MORPH][ARLEN-1]++;
	if (wdlen_clause != NULL) {
		if (cUT->ClauseCnt < ARLEN)
			ts->stats[CUTTS][cUT->ClauseCnt]++;
		else
			ts->stats[CUTTS][ARLEN-1]++;
	}
	return(ts);
}

static W_UTT *freeUttsCluster(W_UTT *p) {
	W_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->nextUtt;
    	if (t->sp)
    		free(t->sp);
    	if (t->line)
    		free(t->line);
    	if (t->mline)
    		free(t->mline);
		free(t);
	}
	return(NULL);
}

static void removeUttDels(char *line) {
	int i, j;

	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '+' && (i == 0 || uS.isskip(line,i-1,&dFnt,MBF))) {
			for (j=i; !uS.isskip(line,j,&dFnt,MBF) && !uS.IsUtteranceDel(line, j) && line[j] != EOS; j++) ;
			if (uS.IsUtteranceDel(line, j) && !uS.atUFound(line, j, &dFnt, MBF)) {
				for (j++; uS.IsUtteranceDel(line, j) && line[j] != EOS; j++) ;
				for (; i < j; i++)
					line[i] = ' ';
				i--;
			}
		} else if (uS.IsUtteranceDel(line, i)) {
			if (!uS.atUFound(line, i, &dFnt, MBF)) {
				for (j=i+1; uS.IsUtteranceDel(line, j) && line[j] != EOS; j++) ;
				for (; i < j; i++)
					line[i] = ' ';
				i--;
			}
		}
	}
	uS.remFrontAndBackBlanks(line);
}

static W_UTT *addUttCluster(W_UTT *root, char isAddTier, char *sp, char *line, char *mline, int ClauseCnt) {
	char *cLine, *cMLine;
	W_UTT *p, *lastSp;

	if (root == NULL) {
		if ((root=NEW(W_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			root = freeUttsCluster(root);
			wdlen_clause = wdlen_free_clause(wdlen_clause);
			cutt_exit(0);
		}
		p = root;
	} else {
		if (isAddTier) {
			lastSp = NULL;
			for (p=root; 1; p=p->nextUtt) {
				if (uS.mStricmp(p->sp, sp) == 0)
					lastSp = p;
				if (p->nextUtt == NULL)
					break;
			}
			if (lastSp != NULL) {
				p = lastSp;
				cLine = p->line;
				cMLine = p->mline;
				removeUttDels(cLine);
				removeUttDels(cMLine);
				if ((p->line=(char *)malloc(strlen(cLine)+strlen(line)+1)) == NULL) {
					fprintf(stderr,"ERROR: no more memory available.\n");
					root = freeUttsCluster(root);
					wdlen_clause = wdlen_free_clause(wdlen_clause);
					cutt_exit(0);
				}
				strcpy(p->line, cLine);
				strcat(p->line, " ");
				strcat(p->line, line);
				if ((p->mline=(char *)malloc(strlen(cMLine)+strlen(mline)+1)) == NULL) {
					fprintf(stderr,"ERROR: no more memory available.\n");
					root = freeUttsCluster(root);
					wdlen_clause = wdlen_free_clause(wdlen_clause);
					cutt_exit(0);
				}
				strcpy(p->mline, cMLine);
				strcat(p->mline, " ");
				strcat(p->mline, mline);
				p->ClauseCnt = p->ClauseCnt + ClauseCnt;
				free(cLine);
				free(cMLine);
				return(root);
			}
		} else {
			for (p=root; p->nextUtt != NULL; p=p->nextUtt) ;
		}
		if ((p->nextUtt=NEW(W_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			root = freeUttsCluster(root);
			wdlen_clause = wdlen_free_clause(wdlen_clause);
			cutt_exit(0);
		}
		p = p->nextUtt;
	}
    p->sp      = NULL;
    p->line    = NULL;
	p->mline   = NULL;
	p->ClauseCnt = ClauseCnt;
    p->nextUtt = NULL;
	if ((p->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		root = freeUttsCluster(root);
		wdlen_clause = wdlen_free_clause(wdlen_clause);
		cutt_exit(0);
	}
	strcpy(p->sp, sp);
	if ((p->line=(char *)malloc(strlen(line)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		root = freeUttsCluster(root);
		wdlen_clause = wdlen_free_clause(wdlen_clause);
		cutt_exit(0);
	}
	strcpy(p->line, line);
	if ((p->mline=(char *)malloc(strlen(mline)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		root = freeUttsCluster(root);
		wdlen_clause = wdlen_free_clause(wdlen_clause);
		cutt_exit(0);
	}
	strcpy(p->mline, mline);
	return(root);
}


static char isTierContinuation(char *line, char isPSDFound) {
	int i;

	if (isPSDFound == 2)
		return(TRUE);
	if (isPSDFound == 1) {
		for (i=0; line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(line,i-1,&dFnt,MBF)) && line[i] == '+' && uS.isRightChar(line,i+1,',',&dFnt, MBF)) {
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

static char isNextTierContinuation(char *line) {
	int i;

	for (i=0; line[i] != EOS; i++) {
		if (isTierContSymbol(line, i, TRUE))
			return(2);
		if (isTierContSymbol(line, i, FALSE))
			return(1);
	}
	return(FALSE);
}

static int ClauseCount(char *line) {
	int i, ClauseCnt;
	WDLEN_CLAUSES *p;

	ClauseCnt = 0;
	for (i=0; line[i] != EOS; i++) {
		for (p=wdlen_clause; p != NULL; p=p->next_clause) {
			if (strncmp(line+i, p->st, p->len) == 0)
				ClauseCnt++;
		}
	}
	return(ClauseCnt);
}

void call()	{		/* tabulate array of word lengths */
	int turn_uttCnt, turn_wordCnt, ClauseCnt;
	char isAddTier, isPSDFound;
	char *spkUtt, *morUtt, oldSp[SPEAKERLEN];
	WDLENSP *ts;
	W_UTT *rootUtt, *cUT;

	rootUtt = NULL;
	spkUtt = spareTier1;
	morUtt = spareTier2;
	spkUtt[0] = EOS;
	morUtt[0] = EOS;
	ClauseCnt = 0;
	oldSp[0] = EOS;
	isAddTier = FALSE;
	isPSDFound = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '%' && chatmode) {
			strcpy(morUtt, uttline);
			if (linkMain2Mor && uS.partcmp(utterance->speaker,"%mor",FALSE,FALSE)) {
				strcpy(spkUtt, uttline);
				removeDepTierItems(spkUtt);
				uS.remFrontAndBackBlanks(spkUtt);
				removeMainTierWords(morUtt);
			}
			uS.remFrontAndBackBlanks(morUtt);
		} else {
			if (spkUtt[0] != EOS)
				rootUtt = addUttCluster(rootUtt, isAddTier, oldSp, spkUtt, morUtt, ClauseCnt);
			spkUtt[0] = EOS;
			morUtt[0] = EOS;
			if (utterance->speaker[0] == '*' || !chatmode) {
				if (wdlen_clause != NULL)
					ClauseCnt = ClauseCount(utterance->line);
				isAddTier = isTierContinuation(utterance->line, isPSDFound);
				isPSDFound = isNextTierContinuation(utterance->line);
				strcpy(oldSp, utterance->speaker);
				uS.remblanks(oldSp);
				uS.uppercasestr(oldSp, &dFnt, MBF);
				strcpy(spkUtt, uttline);
				uS.remFrontAndBackBlanks(spkUtt);
			}
		}
	}
	if (spkUtt[0] != EOS)
		rootUtt = addUttCluster(rootUtt, isAddTier, oldSp, spkUtt, morUtt, ClauseCnt);
	oldSp[0] = EOS;
	ts = NULL;
	turn_uttCnt = 0;
	turn_wordCnt = 0;
	for (cUT=rootUtt; cUT != NULL; cUT=cUT->nextUtt) {
		ts = countTiers(ts, cUT, oldSp, &turn_uttCnt, &turn_wordCnt);
	}
	rootUtt = freeUttsCluster(rootUtt);
	if (chatmode && ts != NULL) {
		if (turn_uttCnt < ARLEN)
			ts->stats[TUTTS][turn_uttCnt]++;
		else
			ts->stats[TUTTS][ARLEN-1]++;
		if (turn_wordCnt < ARLEN)
			ts->stats[TWRDS][turn_wordCnt]++;
		else
			ts->stats[TWRDS][ARLEN-1]++;
	}
	if (!combinput)
		wdlen_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = WDLEN;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,wdlen_pr_result);
	wdlen_clause = wdlen_free_clause(wdlen_clause);
	wdlen_head = wdlen_free_root(wdlen_head);
}
