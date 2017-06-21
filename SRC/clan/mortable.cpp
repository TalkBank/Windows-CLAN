/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"
#include "check.h"
#ifndef UNX
#include "ced.h"
#else
#define RGBColor int
#include "c_curses.h"
#endif
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main mortable_main
#define call mortable_call
#define getflag mortable_getflag
#define init mortable_init
#define usage mortable_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMCOMPS 128

#define LABELS struct labels_list
LABELS {
	char *label;
	char isDisplay;
	int num;
	LABELS *next_label;
};
#define COLS struct cols_list
COLS {
	MORWDLST *pats;
	LABELS *labelP;
	char isClitic;
	COLS *next_col;
};
#define LSP struct levelsp_tnode
LSP {
	char *ID;
	char *fname;
	char *sp;
	int  uID;
	unsigned int totalW;
	unsigned int *count;
	LSP *next_sp;
} ;

extern char OverWriteFile;
extern char outputOnlyData;
extern struct IDtype *IDField;

static int fileID;
static char *script_file;
static char mortable_ftime, isFTimeLabel, mortable_isCombineSpeakers, isRawVal;
static int  colLabelsNum;
static COLS *colOrRoot, *colAndRoot;
static LABELS *labelsRoot;
static LSP *root_sp;

static COLS *freeColsP(COLS *p) {
	COLS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_col;
		t->pats = freeMorWords(t->pats);
		free(t);
	}
	return(NULL);
}

static LABELS *freeLabels(LABELS *p) {
	LABELS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_label;
		if (t->label)
			free(t->label);
		free(t);
	}
	return(NULL);
}

static LSP *freeLSp(LSP *p) {
	LSP *t;
	
	if (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->ID != NULL)
			free(t->ID);
		if (t->fname != NULL)
			free(t->fname);
		if (t->sp != NULL)
			free(t->sp);
		if (t->count != NULL)
			free(t->count);
		free(t);
	}
	return(NULL);
}

static void mortable_error(const char *mess) {
	colOrRoot = freeColsP(colOrRoot);
	colAndRoot = freeColsP(colAndRoot);
	labelsRoot = freeLabels(labelsRoot);
	root_sp = freeLSp(root_sp);
	fprintf(stderr, "%s\n", mess);
	cutt_exit(0);
}

static LABELS *addNewLabel(char *label, char isDisplay) {
	int cnt;
	LABELS *p;

	cnt = 0;
	if (labelsRoot == NULL) {
		labelsRoot = NEW(LABELS);
		p = labelsRoot;
	} else {
		for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
/* two lines in "eng.cut" file with the same label will be merged into one column
			if (uS.mStricmp(p->label, label) == 0) {
				return(p);
			}
*/
			cnt++;
		}
/* two lines in "eng.cut" file with the same label will be merged into one column
		if (uS.mStricmp(p->label, label) == 0) {
			return(p);
		}
*/
		p->next_label = NEW(LABELS);
		p = p->next_label;
		cnt++;
	}
	if (p == NULL) {
		mortable_error("Error: out of memory");
	}
	p->next_label = NULL;
	p->isDisplay = isDisplay;
	p->num = cnt;
	p->label = (char *)malloc(strlen(label)+1);
	if (p->label == NULL) {
		mortable_error("Error: out of memory");
	}
	strcpy(p->label, label);
	colLabelsNum++;
	return(p);
}

static COLS *add2col(COLS *root, char isFirstElem, char *pat, FNType *mFileName, int ln, char isDisplay) {
	int  i;
	char ch, res;
	COLS *p;

	if (isFirstElem || root == NULL) {
		if (root == NULL) {
			root = NEW(COLS);
			p = root;
		} else {
			for (p=root; p->next_col != NULL; p=p->next_col) ;
			p->next_col = NEW(COLS);
			p = p->next_col;
		}
		if (p == NULL) {
			mortable_error("Error: out of memory");
		}
		p->next_col = NULL;
		p->pats = NULL;
		p->labelP = NULL;
		p->isClitic = FALSE;
	} else
		for (p=root; p->next_col != NULL; p=p->next_col) ;
	if (pat[0] == '"' || pat[0] == '\'') {
		for (i=0; pat[i] != EOS && (pat[i] == '"' || pat[i] == '\'' || isSpace(pat[i])); i++) ;
		if (pat[i] != EOS) {
			if (p->labelP != NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
				fprintf(stderr, "    ERROR: only one label is allow for each line: \"%s\"\n", pat);
				fprintf(stderr, "    Perhaps this label name is used on another line\n");
				colOrRoot = freeColsP(colOrRoot);
				colAndRoot = freeColsP(colAndRoot);
				labelsRoot = freeLabels(labelsRoot);
				root_sp = freeLSp(root_sp);
				cutt_exit(0);
			}
			templineC2[0] = EOS;
			if (isRawVal) {
				if (pat[i] == '%')
					i++;
				while (isSpace(pat[i]))
					i++;
			} else {
				if (pat[i] != '%')
					strcpy(templineC2, "% ");
			}
			strcat(templineC2, pat+i);
			p->labelP = addNewLabel(templineC2, isDisplay);
		}
	} else {
		i = 0;
		if (pat[i] == '-') {
			i++;
			ch = 'e';
		} else if (pat[i] == '+') {
			i++;
			ch = 'i';
		} else {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
			fprintf(stderr, "    ERROR: Illegal character \"%c\" found on line \"%s\"\n", pat[i], pat);
			fprintf(stderr, "    Expected characters are: + - \" '\n\n");
			colOrRoot = freeColsP(colOrRoot);
			colAndRoot = freeColsP(colAndRoot);
			labelsRoot = freeLabels(labelsRoot);
			root_sp = freeLSp(root_sp);
			cutt_exit(0);
		}
		if (pat[i] == 's' || pat[i] == 'S')
			i++;
		if (pat[i] == '@')
			i++;
		if (strchr(pat+i, '~') != NULL || strchr(pat+i, '$') != NULL)
			p->isClitic = TRUE;
		p->pats = makeMorWordList(p->pats, &res, pat+i, ch);
		if (p->pats == NULL) {
			if (res == TRUE)
				mortable_error("Error: out of memory");
			else if (res == FALSE) {
				colOrRoot = freeColsP(colOrRoot);
				colAndRoot = freeColsP(colAndRoot);
				labelsRoot = freeLabels(labelsRoot);
				root_sp = freeLSp(root_sp);
				cutt_exit(0);
			}
		}
	}
	return(root);
}

static void read_script(char *lang) {
	int  ln;
	char *b, *e, t, isQT, isFirstElem, isORGoup, isDisplay;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*lang == EOS) {
		fprintf(stderr,	"No language specified with +l option.\n");
		fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
		fprintf(stderr,"For example, \"mortable +leng\" or \"mortable +leng.cut\".\n");
		cutt_exit(0);
	}
	strcpy(mFileName, lib_dir);
	addFilename2Path(mFileName, "mortable");
	addFilename2Path(mFileName, lang);
	if ((b=strchr(lang, '.')) != NULL) {
		if (uS.mStricmp(b, ".cut") != 0)
			strcat(mFileName, ".cut");
	} else
		strcat(mFileName, ".cut");
	if ((fp=fopen(mFileName,"r")) == NULL) {
		if (b != NULL) {
			if (uS.mStricmp(b, ".cut") == 0) {
				strcpy(templineC, wd_dir);
				addFilename2Path(templineC, lang);
				if ((fp=fopen(templineC,"r")) != NULL) {
					strcpy(mFileName, templineC);
				}
			}
		}
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate language file: \"%s\".\n", mFileName);
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
		cutt_exit(0);
	}
	fprintf(stderr,"    Using language file: %s\n", mFileName);
	isORGoup = FALSE;
	ln = 0;
	while (fgets_cr(templineC, 255, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		ln++;
		if (templineC[0] == ';' || templineC[0] == '%')
			continue;
		if (templineC[0] == '#') {
			for (b=templineC; *b == '#' || isSpace(*b); b++) ;
			if (b != templineC)
				strcpy(templineC, b);
			isDisplay = FALSE;
		} else
			isDisplay = TRUE;
		uS.remFrontAndBackBlanks(templineC);
		if (templineC[0] == EOS)
			continue;
		if (uS.mStrnicmp(templineC, "OR", 2) == 0) {
			isORGoup = TRUE;
			continue;
		} else if (uS.mStrnicmp(templineC, "AND", 3) == 0) {
			isORGoup = FALSE;
			continue;
		}
		isFirstElem = TRUE;
		isQT = '\0';
		b = templineC;
		e = b;
		while (*b != EOS) {
			if (*e == '"' || *e == '\'') {
				if (isQT == *e)
					isQT = '\0';
				else if (isQT == '\0')
					isQT = *e;
			}
			if (((isSpace(*e) || *e == '"' || *e == '\'') && isQT == '\0') || *e == EOS) {
				t = *e;
				*e = EOS;
				strcpy(templineC1, b);
				uS.remblanks(templineC1);
				if (templineC1[0] != EOS) {
					if (isORGoup)
						colOrRoot = add2col(colOrRoot, isFirstElem, templineC1, mFileName, ln, isDisplay);
					else
						colAndRoot = add2col(colAndRoot, isFirstElem, templineC1, mFileName, ln, isDisplay);
					isFirstElem = FALSE;
				}
				*e = t;
				if (*e != EOS)
					e++;
				b = e;
			} else
				e++;
		}
	}
	fclose(fp);
	if (colOrRoot == NULL && colAndRoot == NULL) {
		fprintf(stderr,"Can't find any usable declarations in language script file \"%s\".\n", mFileName);
		cutt_exit(0);
	}
/*
LABELS *p;
for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
	fprintf(stdout, "%s: %d\n", p->label, p->num);
}
*/
}

void init(char f) {
	if (f) {
		isRawVal = FALSE;
		script_file = NULL;
		fileID = 0;
		colLabelsNum = 0;
		colOrRoot = NULL;
		colAndRoot = NULL;
		labelsRoot = NULL;
		root_sp = NULL;
		mortable_ftime = TRUE;
		isFTimeLabel = TRUE;
		mortable_isCombineSpeakers = FALSE;
		stout = FALSE;
		OverWriteFile = TRUE;
		outputOnlyData = TRUE;
		AddCEXExtension = ".xls";
		combinput = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
//		addword('\0','\0',"+<\">");
		maininitwords();
		mor_initwords();
	} else {
		if (mortable_ftime) {
			mortable_ftime = FALSE;
			if (script_file == NULL) {
				fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"mortable +leng\" or \"mortable +leng.cut\".\n");
				cutt_exit(0);
			}
			read_script(script_file);
		}
		maketierchoice("@ID:",'+','\001');
		maketierchoice("%mor:",'+','\001');
	}
}

void usage() {
	printf("Creates table of frequency count of parts of speech and bound morphemes\n");
	printf("Usage: mortable[lF oN %s] filename(s)\n", mainflgs());
	puts("+lF: specify language script file name F");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+o3: combine selected speakers from each file into one results list for that file");
	puts("+o4: output raw values instead of percentage values");
	mainusage(FALSE);
	puts("Example:");
#ifdef _WIN32
	puts("   To use \"eng.cut\" script file located in CLAN\\lib\\mortable folder");
#else
	puts("   To use \"eng.cut\" script file located in CLAN/lib/mortable folder");
#endif
	puts("       mortable +leng ...");
	puts("       mortable +leng.cut ...");
	cutt_exit(0);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'l':
			script_file = f;
			break;
#ifdef UNX
		case 'L':
			int j;
			strcpy(lib_dir, f);
			j = strlen(lib_dir);
			if (j > 0 && lib_dir[j-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 'o':
			if (*f == '3') {
				mortable_isCombineSpeakers = TRUE;
			} else if (*f == '4') {
				isRawVal = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && *f == '*' && *(f+1) == EOS) {
				mortable_isCombineSpeakers = TRUE;
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void print_labels(LABELS *col) {
	for (; col != NULL; col=col->next_label) {
		if (col->isDisplay) {
			if (col->label != NULL)
				fprintf(fpout,"\t%s", col->label);
			else
				fprintf(fpout,"\t");
		}
	}
}

static void print_vals(unsigned int *count, float ftotalW) {
	float fcount;
	LABELS  *col;

	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (col->isDisplay) {
			if (isRawVal)
				fprintf(fpout,"\t%u", count[col->num]);
			else if (ftotalW == 0)
				fprintf(fpout,"\t0.000");
			else {
				fcount = (float)count[col->num];
				fprintf(fpout,"\t%.3f", (fcount/ftotalW)*100.0000);
			}
		}
	}
}

static void LSp_pr_result(void) {
	int i, cnt;
	LSP *p;

	p = root_sp;
	fprintf(fpout,"File\tLanguage\tCorpus\tCode\tAge\tSex\tGroup\tSES\tRole\tEducation\tCustom_field");
	fprintf(fpout,"\t# words");
	if (p != NULL && (isFTimeLabel || !combinput)) {
		isFTimeLabel = FALSE;
		print_labels(labelsRoot);
	}
	fprintf(fpout,"\n");
	while (p != NULL) {
		fprintf(fpout,"%s\t", p->fname);
		if (p->ID != NULL) {
			cnt = 0;
			for (i=0; p->ID[i] != EOS; i++) {
				if (p->ID[i] == '|') {
					cnt++;
					if (cnt < 10)
						p->ID[i] = '\t';
					else
						p->ID[i] = EOS;
				}
			}
			fprintf(fpout,"%s", p->ID);
		} else
			fprintf(fpout,".\t.\t%s\t.\t.\t.\t.\t.\t.\t.", p->sp);
		fprintf(fpout,"\t%u", p->totalW);
		print_vals(p->count, (float)p->totalW);
		fprintf(fpout,"\n");
		p = p->next_sp;
	}
}

static LSP *findSpeaker(char *sp, char *ID) {
	int  i;
	char *s;
	LSP *ts;

	uS.remblanks(sp);
	if (root_sp == NULL) {
		if ((ts=NEW(LSP)) == NULL) {
			mortable_error("Error: out of memory");
		}
		root_sp = ts;
	} else {
		for (ts=root_sp; ts->next_sp != NULL; ts=ts->next_sp) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && ts->uID == fileID) {
				return(ts);
			}
		}
		if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && ts->uID == fileID) {
			return(ts);
		}
		if ((ts->next_sp=NEW(LSP)) == NULL) {
			mortable_error("Error: out of memory");
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->ID = NULL;
	ts->fname = NULL;
	ts->sp = NULL;
	ts->totalW = 0;
	ts->count = (unsigned int *)malloc(sizeof(unsigned int) * colLabelsNum);
	if (ts->count == NULL) {
		mortable_error("Error: out of memory");
	}
	for (i=0; i < colLabelsNum; i++)
		ts->count[i] = 0;
	ts->uID = fileID;
	if (ID == NULL)
		ts->ID = NULL;
	else {
		ts->ID = (char *)malloc(strlen(ID)+1);
		if (ts->ID == NULL) {
			mortable_error("Error: out of memory");
		}
		strcpy(ts->ID, ID);
	}
	ts->sp = (char *)malloc(strlen(sp)+1);
	if (ts->sp == NULL) {
		mortable_error("Error: out of memory");
	}
	strcpy(ts->sp, sp);
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	if ((ts->fname=(char *)malloc(strlen(s)+1)) == NULL) {
		mortable_error("Error: out of memory");
	}	
	strcpy(ts->fname, s);
	s = strrchr(ts->fname, '.');
	if (s != NULL)
		*s = EOS;
	return(ts);
}

static void countCols(LSP *p) {
	int  i, j, comps;
	char wordO[BUFSIZ], wordC[BUFSIZ], *w[NUMCOMPS], isMatchFound;
	COLS *col;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, wordO, NULL, i))) {
		uS.remblanks(wordO);
		if (strchr(wordO, '|') != NULL) {
			p->totalW++;
			strcpy(wordC, wordO);
			comps = 0;
			w[comps++] = wordC;
			for (j=0; wordC[j] != EOS; j++) {
				if (wordC[j] == '~' || wordC[j] == '$') {
					wordC[j] = EOS;
					j++;
					if (comps >= NUMCOMPS) {
						fprintf(stderr, "    Intenal Error: # of pre/post clitics is > %d in word: \"%s\"\n", NUMCOMPS-1, wordO);
						colOrRoot = freeColsP(colOrRoot);
						colAndRoot = freeColsP(colAndRoot);
						labelsRoot = freeLabels(labelsRoot);
						root_sp = freeLSp(root_sp);
						cutt_exit(0);
					}
					w[comps++] = wordC+j;
				}
			}
			for (j=0; j < comps; j++) {
				for (col=colOrRoot; col != NULL; col=col->next_col) {
					if (col->isClitic) {
						isMatchFound = isMorPatMatchedWord(col->pats, wordO);
						if (isMatchFound == 2)
							mortable_error("Error: out of memory");
						if (j == 0 && isMatchFound) {
//if (col->labelP->num == 14) fprintf(stdout, "%s\n", wordO);
							if (col->labelP != NULL) {
								p->count[col->labelP->num] = p->count[col->labelP->num] + 1;
								break;
							}
						}
					} else {
						isMatchFound = isMorPatMatchedWord(col->pats, w[j]);
						if (isMatchFound == 2)
							mortable_error("Error: out of memory");
						if (isMatchFound) {
//if (col->labelP->num == 14) fprintf(stdout, "%s\n", wordO);
							if (col->labelP != NULL) {
								p->count[col->labelP->num] = p->count[col->labelP->num] + 1;
								break;
							}
						}
					}
				}
				for (col=colAndRoot; col != NULL; col=col->next_col) {
					if (col->isClitic) {
						isMatchFound = isMorPatMatchedWord(col->pats, wordO);
						if (isMatchFound == 2)
							mortable_error("Error: out of memory");
						if (j == 0 && isMatchFound) {
							if (col->labelP != NULL)
								p->count[col->labelP->num] = p->count[col->labelP->num] + 1;
						}
					} else {
						isMatchFound = isMorPatMatchedWord(col->pats, w[j]);
						if (isMatchFound == 2)
							mortable_error("Error: out of memory");
						if (isMatchFound) {
							if (col->labelP != NULL)
								p->count[col->labelP->num] = p->count[col->labelP->num] + 1;
						}
					}
				}
			}
		}
	}
}

void call() {
	LSP *ts;
	char spname[SPEAKERLEN];

	fileID++;
	spname[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (isIDSpeakerSpecified(utterance->line, templineC1, TRUE)) {
				uS.remblanks(utterance->line);
				findSpeaker(templineC1, utterance->line);
			}
			continue;
		}
		if (utterance->speaker[0] == '*') {
			if (mortable_isCombineSpeakers) {
				strcpy(spname, "*COMBINED*");
			} else {
				strcpy(spname, utterance->speaker);
			}
			ts = findSpeaker(spname, NULL);
		} else if (uS.partcmp(utterance->speaker, "%mor:", FALSE, FALSE)) {
			countCols(ts);
		}
	}
	if (!combinput) {
		LSp_pr_result();
		root_sp = freeLSp(root_sp);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MORTABLE;
	OnlydataLimit = 2;
	UttlineEqUtterance = FALSE;
#ifdef UNX
	int i, j;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'L') {
				strcpy(lib_dir, argv[i]+2);
				j = strlen(lib_dir);
				if (j > 0 && lib_dir[j-1] != '/')
					strcat(lib_dir, "/");
			}
		}
	}
#endif
	bmain(argc,argv,LSp_pr_result);
	colOrRoot = freeColsP(colOrRoot);
	colAndRoot = freeColsP(colAndRoot);
	labelsRoot = freeLabels(labelsRoot);
	root_sp = freeLSp(root_sp);
}
