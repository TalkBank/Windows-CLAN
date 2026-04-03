/**********************************************************************
	"Copyright 1990-2026 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#ifdef UNX
	#define RGBColor int
#endif

#if !defined(UNX)
#define _main wdsize_main
#define call wdsize_call
#define getflag wdsize_getflag
#define init wdsize_init
#define usage wdsize_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char R8;
extern char OverWriteFile;
extern char Parans;
extern char isRecursive;

#define numberOfElements 1
#define ARLEN 80

#define WDLNGS 0

#define WDSIZESP struct wdsize_cnt
struct wdsize_cnt {
	char *sp;
	char *ID;
	char *fname;
	int  stats[numberOfElements][ARLEN];
	int  wordcnt;
	WDSIZESP *next_sp;
} ;

#define W_UTT struct indent_utterance
W_UTT {
    char    *sp;
    char    *line;
    W_UTT *nextUtt;
} ;

static char wdsize_isXXXFound = FALSE;
static char wdsize_isYYYFound = FALSE;
static char isSpTier, wdsize_ftime;
static char filterwdsize_cmp;			/* = Equal, < les than, > greater than */
static long filterWdLen;
static WDSIZESP *wdsize_head;

void usage() {
#ifdef UNX
	printf("WDSIZE REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cWDSIZE REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	puts("WDSIZE gives length histograms for characters in each word.");
	printf("Usage: wdsize [a bS cS %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
	puts("+wCN: include only words which are C (>, <, =) than N characters long");
	mainusage(FALSE);
#ifdef UNX
	printf("WDSIZE REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("THIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cWDSIZE REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTHIS MEANS ALSO THAT YOU SHOULD USE THE +sm FACILITY FOR SEARCH STRING SPECIFICATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		Parans = 3;
		isSpTier = FALSE;
		filterwdsize_cmp = 0;
		filterWdLen = 0L;
		wdsize_head = NULL;
		wdsize_isXXXFound = FALSE;
		wdsize_isYYYFound = FALSE;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+*|xxx");
		addword('\0','\0',"+*|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+$*");
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		mor_initwords();
		wdsize_ftime = TRUE;
	} else {
		if (wdsize_ftime) {
			wdsize_ftime = FALSE;
			if (chatmode) {
				if (onlydata == 1)
					maketierchoice("@ID:",'+',FALSE);
				if (isSpTier == FALSE) {
					maketierchoice("%mor",'+',FALSE);
				}
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

static WDSIZESP *wdsize_free_root(WDSIZESP *p) {
	WDSIZESP *ts;

	while (p != NULL) {
		ts = p;
		p = p->next_sp;
		if (ts->sp)
			free(ts->sp);
		if (ts->fname)
			free(ts->fname);
		if (ts->ID != NULL)
			free(ts->ID);
		free(ts);
	}
	return(NULL);
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
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
		case 'w':
			if (*f == '=' || *f == '<' || *f == '>') {
				filterwdsize_cmp = *f++;
				if (*f == EOS) {
					fprintf(stderr, "\nPlease specify number of characters in a word\n");
					fprintf(stderr,"\tFor example: +w%c7\n", filterwdsize_cmp);
					cutt_exit(0);
				}
				filterWdLen = atol(f);
			} else {
				fprintf(stderr, "\nPlease specify comparison type (=, <, >) and number of characters in a word\n");
				fprintf(stderr,"\tFor example: +w%c7\n", filterwdsize_cmp);
				cutt_exit(0);
			}
			break;
		case 't':
			if (chatmode) {
				if (*(f-2) == '-' && *f == '%') {
					if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:")) {
						isSpTier = TRUE;
					}
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
					wdsize_isXXXFound = TRUE;
				else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					cutt_exit(0);
				}
				break;
			} else if (uS.mStricmp(t+j, "yyy") == 0) {
				if (*(f-2) == '+')
					wdsize_isYYYFound = TRUE;
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
			}
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
	WDSIZESP *ts;

	lim = 1;
	for (ts=wdsize_head; ts != NULL; ts=ts->next_sp) {
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

static void wdsize_pr_result(void) {
	int  i, lim;
	char fontName[256];
	double sum, num;
	WDSIZESP *ts;

	if (onlydata == 1) {
		excelHeader(fpout, newfname, 105);
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout, "Number,of,words,of,each,length,in,characters");
		excelRow(fpout, ExcelRowEnd);
	} else {
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
	if (onlydata == 1) {
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, "File");
		excelCommasStrCell(fpout, "Language,Corpus,Code,Age(Month),Sex,Group,Race,SES,Role,Education,Custom_field,lengths");
	} else
		fprintf(fpout,"lengths ");
	for (i=1; i < lim; i++) {
		if (onlydata == 1)
			excelLongNumCell(fpout, "%d", i);
		else
			fprintf(fpout," %4d", i);
	}
	if (onlydata == 1) {
		if (lim == ARLEN)
			excelStrCell(fpout, "+ Mean");
		else 
			excelStrCell(fpout, "Mean");
		excelRow(fpout, ExcelRowEnd);
	} else {
		if (lim == ARLEN)
			fprintf(fpout,"+  Mean\n");
		else 
			fprintf(fpout,"   Mean\n");
	}
	for (ts=wdsize_head; ts != NULL; ts=ts->next_sp) {
		sum = 0.0;
		num = 0.0;		
		if (onlydata == 1) {
			excelRow(fpout, ExcelRowStart);
			excelStrCell(fpout, ts->fname);
			if (ts->ID) {
				excelOutputID(fpout, ts->ID);
			} else {
				excelCommasStrCell(fpout, ".,.");
				excelStrCell(fpout, ts->sp);
				excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
			}
			excelCommasStrCell(fpout, ".");
		} else {
			strncpy(templineC, ts->sp, 8);
			templineC[8] = EOS;
			fprintf(fpout,"%-8s", templineC);
		}
		for (i=1; i < lim; i++) {
			sum += ((double)i * (double)ts->stats[WDLNGS][i]);
			num += (double)ts->stats[WDLNGS][i];
			if (onlydata == 1)
				excelLongNumCell(fpout, "%d", ts->stats[WDLNGS][i]);
			else
				fprintf(fpout," %4d", ts->stats[WDLNGS][i]);
		}
		if (onlydata == 1) {
			if (num > 0.0)
				excelNumCell(fpout, "%.5f", sum/num);
			else
				excelNumCell(fpout, "%f", 0.0);
			excelRow(fpout, ExcelRowEnd);
		} else {
			if (num > 0.0)
				fprintf(fpout, " %6.4f\n", sum/num);
			else
				fprintf(fpout, " %6.4f\n", 0.0);
		}
	}
	excelFooter(fpout);
	wdsize_head = wdsize_free_root(wdsize_head);
}

static WDSIZESP *wdsize_FindSpeaker(const char *sp, char *ID) {
	int  i, j;
	char *s;
	WDSIZESP *ts;

	if (isRecursive == FALSE) {
		s = strrchr(oldfname, PATHDELIMCHR);
		if (s != NULL)
			strcpy(FileName1, s+1);
		else
			strcpy(FileName1, oldfname);
	} else
		strcpy(FileName1, oldfname);
	if (wdsize_head == NULL) {
		if ((ts=NEW(WDSIZESP)) == NULL)
			out_of_mem();
		wdsize_head = ts;
	} else {
		for (ts=wdsize_head; ts->next_sp != NULL; ts=ts->next_sp) {
			if (uS.mStricmp(ts->fname, FileName1) == 0) {
				if (uS.mStricmp(ts->sp, sp) == 0)
					return(ts);
			}
		}
		if (uS.mStricmp(ts->fname, FileName1) == 0) {
			if (uS.mStricmp(ts->sp, sp) == 0)
				return(ts);
		}
		if ((ts->next_sp=NEW(WDSIZESP)) == NULL)
			out_of_mem();
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->sp = (char *)malloc(strlen(sp)+1);
	if (ts->sp == NULL)
		out_of_mem();
	strcpy(ts->sp, sp);
	if (ID[0] == EOS)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			out_of_mem();
		strcpy(ts->ID, ID);
	}
	ts->fname = (char *)malloc(strlen(FileName1)+1);
	if (ts->fname == NULL)
		out_of_mem();
	strcpy(ts->fname, FileName1);
	ts->wordcnt = 0;
	for (j=0; j < numberOfElements; j++) {
		for (i = 0; i < ARLEN; i++)
			ts->stats[j][i] = 0;
	}
	return(ts);
}

static void words_count(char *line, WDSIZESP *ts) {
	int i;
	int c;
	int wlen;
	
	c = 0;
	while (line[c] != EOS) {
		i = 0;
		wlen = 0;
		while (line[c] != EOS && !uS.isskip(line,c,&dFnt,MBF)) {
			if (UTF8_IS_SINGLE((unsigned char)line[c]) || UTF8_IS_LEAD((unsigned char)line[c]))
				wlen++;
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
		for (; line[c] != EOS && uS.isskip(line,c,&dFnt,MBF); c++) ;
	}
}

static WDSIZESP *countTiers(WDSIZESP *ts, W_UTT *cUT, char *oldSp) {
	if (!chatmode) {
		ts = wdsize_FindSpeaker("GENERIC", spareTier2);
	} else {
		if (uS.mStricmp(oldSp, cUT->sp)) {
			strcpy(oldSp, cUT->sp);
		}
		ts = wdsize_FindSpeaker(cUT->sp, spareTier2);
	}
	words_count(cUT->line, ts);
	ts->wordcnt = 0;
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
		free(t);
	}
	return(NULL);
}
/*
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
*/
static W_UTT *addUttCluster(W_UTT *root, char *sp, char *line) {
	W_UTT *p;

	if (root == NULL) {
		if ((root=NEW(W_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			root = freeUttsCluster(root);
			cutt_exit(0);
		}
		p = root;
	} else {
		for (p=root; p->nextUtt != NULL; p=p->nextUtt) ;
		if ((p->nextUtt=NEW(W_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			root = freeUttsCluster(root);
			cutt_exit(0);
		}
		p = p->nextUtt;
	}
    p->sp      = NULL;
    p->line    = NULL;
    p->nextUtt = NULL;
	if ((p->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		root = freeUttsCluster(root);
		cutt_exit(0);
	}
	strcpy(p->sp, sp);
	if ((p->line=(char *)malloc(strlen(line)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		root = freeUttsCluster(root);
		cutt_exit(0);
	}
	strcpy(p->line, line);
	return(root);
}

void call()	{		/* tabulate array of word lengths */
	int  i;
	char word[BUFSIZ+1], comp_word[BUFSIZ+1];
	char *line, *s, oldSp[SPEAKERLEN];
	WDSIZESP *ts;
	W_UTT *rootUtt, *cUT;
	MORFEATS word_feats, *compd, *feat;

	spareTier2[0] = EOS;
	rootUtt = NULL;
	line = spareTier1;
	line[0] = EOS;
	oldSp[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				uS.remblanks(utterance->line);
				strcpy(spareTier2, utterance->line);
			}
		} else if (utterance->speaker[0] == '%' && chatmode && isSpTier == FALSE) {
			line[0] = EOS;
			i = 0;
			while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
				if (word[0] != EOS) {
					s = strchr(word, '|');
					if (s != NULL) {
						strcpy(templineC2, word);
						if (ParseWordMorElems(templineC2, &word_feats) == FALSE) {
							fprintf(stderr,"ERROR: no more memory available.\n");
							rootUtt = freeUttsCluster(rootUtt);
							cutt_exit(0);
						}
						for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
							if (line[0] != EOS)
								strcat(line, " ");
							if (feat->compd != NULL) {
								comp_word[0] = EOS;
								for (compd=feat; compd != NULL; compd=compd->compd) {
									if (compd->stem != NULL && compd->stem[0] != EOS)
										strcat(comp_word, compd->stem);
								}
								if (filterwdsize_cmp != 0) {
									if ((strlen(comp_word) == filterWdLen && filterwdsize_cmp == '=') ||
										(strlen(comp_word) < filterWdLen && filterwdsize_cmp == '<')  ||
										(strlen(comp_word) > filterWdLen && filterwdsize_cmp == '>')) {
										strcat(line, comp_word);
									}
								} else {
									strcat(line, comp_word);
								}
							} else {
								if (filterwdsize_cmp != 0) {
									if ((strlen(feat->stem) == filterWdLen && filterwdsize_cmp == '=') ||
										(strlen(feat->stem) < filterWdLen && filterwdsize_cmp == '<')  ||
										(strlen(feat->stem) > filterWdLen && filterwdsize_cmp == '>')) {
										strcat(line, feat->stem);
									}
								} else {
									strcat(line, feat->stem);
								}
							}
						}
						freeUpFeats(&word_feats);
					}
				}
			}
			uS.remFrontAndBackBlanks(line);
		} else {
			if (line[0] != EOS)
				rootUtt = addUttCluster(rootUtt, oldSp, line);
			line[0] = EOS;
			if (utterance->speaker[0] == '*'|| !chatmode) {
				strcpy(oldSp, utterance->speaker);
				if (oldSp[0] == '*')
					strcpy(oldSp, oldSp+1);
				s = strrchr(oldSp, ':');
				if (s != NULL)
					*s = EOS;
				uS.remblanks(oldSp);
				uS.uppercasestr(oldSp, &dFnt, MBF);
				if (isSpTier == TRUE) {
					strcpy(line, uttline);
					uS.remFrontAndBackBlanks(line);
				}
			}
		}
	}
	if (line[0] != EOS)
		rootUtt = addUttCluster(rootUtt, oldSp, line);
	oldSp[0] = EOS;
	ts = NULL;
	for (cUT=rootUtt; cUT != NULL; cUT=cUT->nextUtt) {
		ts = countTiers(ts, cUT, oldSp);
	}
	rootUtt = freeUttsCluster(rootUtt);
	if (!combinput)
		wdsize_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = WDSIZE;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,wdsize_pr_result);
	wdsize_head = wdsize_free_root(wdsize_head);
}
