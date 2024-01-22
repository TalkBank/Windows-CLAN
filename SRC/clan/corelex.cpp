/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
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
#define _main corelex_main
#define call corelex_call
#define getflag corelex_getflag
#define init corelex_init
#define usage corelex_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMCOMPS 128

#define ALTLABELS struct altLabel
struct altLabel {
	char *label;
	ALTLABELS *next_alt; 
};

#define LABELS struct labels_list
LABELS {
	ALTLABELS *labelP;
	int num;
	char isStem;
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
	char isSpFound;
	int  uID;
	unsigned int firstBulletTime;
	unsigned int *count;
	long begTotaTime;
	long endTotaTime;
	long totaTime;
	LSP *next_sp;
} ;

extern char OverWriteFile;
extern char outputOnlyData;
extern struct tier *defheadtier;
extern struct IDtype *IDField;

static int fileID;
static char *corelex_script_file;
static char corelex_ftime, corelex_isCombineSpeakers, corelex_n_option, isNOptionSet, corelex_isWor;
static char corelex_BBS[5], corelex_CBS[5];
static int  colLabelsNum;
static LABELS *labelsRoot;
static LSP *root_sp;

static void corelex_freeAltLabels(ALTLABELS *p) {
	ALTLABELS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_alt;
		if (t->label)
			free(t->label);
		free(t);
	}
}

static LABELS *corelex_freeLabels(LABELS *p) {
	LABELS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_label;
		corelex_freeAltLabels(t->labelP);
		free(t);
	}
	return(NULL);
}

static LSP *corelex_freeLSp(LSP *p) {
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

static void corelex_error(const char *mess) {
	labelsRoot = corelex_freeLabels(labelsRoot);
	root_sp = corelex_freeLSp(root_sp);
	fprintf(stderr, "%s\n", mess);
	cutt_exit(0);
}

static ALTLABELS *addAltLabels(char *line, char *isAllStems) {
	char *b, *e, t;
	ALTLABELS *root, *p;
	
	root = NULL;
	b = line;
	e = line;
	while (*b != EOS) {
		if (isSpace(*e) || *e == ',' || *e == ';' || *e == EOS) {
			t = *e;
			*e = EOS;
			if (root == NULL) {
				root = NEW(ALTLABELS);
				p = root;
			} else {
				for (p=root; p->next_alt != NULL; p=p->next_alt) ;
				p->next_alt = NEW(ALTLABELS);
				p = p->next_alt;
			}
			if (p == NULL) {
				corelex_error("Error: out of memory");
			}			
			p->next_alt = NULL;
			p->label = (char *)malloc(strlen(b)+1);
			if (p->label == NULL) {
				corelex_error("Error: out of memory");
			}
			strcpy(p->label, b);
			if (*b == '-') {
				*isAllStems = FALSE;
			} else if (*isAllStems == FALSE) {
				corelex_error("Illegal mix of suffix and stem labels in the same column");
			}
			*e = t;
			b = e;
			if (*b != EOS) {
				for (b++; isSpace(*b) || *b == ',' || *b == ';'; b++) ;
				e = b;
			}
		} else
			e++;
	}
	return(root);
}

static LABELS *corelex_addNewLabel(LABELS *root, char *label) {
	int cnt;
	char isAllStems;
	LABELS *p;

	cnt = 0;
	if (root == NULL) {
		root = NEW(LABELS);
		p = root;
	} else {
		for (p=root; p->next_label != NULL; p=p->next_label) {
			cnt++;
		}
		p->next_label = NEW(LABELS);
		p = p->next_label;
		cnt++;
	}
	if (p == NULL) {
		corelex_error("Error: out of memory");
	}
	p->next_label = NULL;
	p->num = cnt;
	isAllStems = TRUE;
	p->labelP = addAltLabels(label, &isAllStems);
	p->isStem = isAllStems;
	colLabelsNum++;
	return(root);
}

static void corelex_read_script(char *script) {
//	int  ln;
	char *b;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*script == EOS) {
		fprintf(stderr,	"No words group specified with +l option.\n");
		fprintf(stderr,"Please specify words group file name with \"+l\" option.\n");
		fprintf(stderr,"For example, \"corelex +lCinderella\" or \"corelex +lCinderella.cut\".\n");
		cutt_exit(0);
	}
	strcpy(mFileName, lib_dir);
	addFilename2Path(mFileName, "corelex");
	addFilename2Path(mFileName, script);
	if ((b=strchr(script, '.')) != NULL) {
		if (uS.mStricmp(b, ".cut") != 0)
			strcat(mFileName, ".cut");
	} else
		strcat(mFileName, ".cut");
	if ((fp=fopen(mFileName,"r")) == NULL) {
		strcpy(templineC, wd_dir);
		addFilename2Path(templineC, script);
		if (b == NULL)
			strcat(templineC, ".cut");
		if ((fp=fopen(templineC,"r")) != NULL) {
			strcpy(mFileName, templineC);
		}
	}
	if (b != NULL) {
		if (uS.mStricmp(b, ".cut") == 0)
			*b = EOS;
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate words group file: \"%s\".\n", mFileName);
#ifdef _MAC_CODE
		fprintf(stderr, "\n");
#else
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
#endif
		cutt_exit(0);
	}
	fprintf(stderr,"    Using words group file: %s\n", mFileName);
	b = strrchr(script, '-');
	if (b != NULL)
		*b = EOS;
	else if (corelex_isWor == TRUE) {
		fprintf(stderr,"\nThe \"+w\" option requires the '-' character in filename specified with +l option.\n");
		cutt_exit(0);
	}
//	ln = 0;
	while (fgets_cr(templineC, 255, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
//		ln++;
		if (templineC[0] == '%' || templineC[0] == '#')
			continue;
		uS.remFrontAndBackBlanks(templineC);
		if (templineC[0] == EOS)
			continue;
		uS.remFrontAndBackBlanks(templineC);
		if (templineC[0] != EOS) {
			labelsRoot = corelex_addNewLabel(labelsRoot, templineC);
		}
	}
	fclose(fp);
	if (labelsRoot == NULL) {
		fprintf(stderr,"Can't find any usable declarations in words group file \"%s\".\n", mFileName);
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
		corelex_script_file = NULL;
		fileID = 0;
		colLabelsNum = 0;
		labelsRoot = NULL;
		root_sp = NULL;
		corelex_ftime = TRUE;
		corelex_isCombineSpeakers = FALSE;
		stout = FALSE;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		outputOnlyData = TRUE;
		AddCEXExtension = ".xls";
		combinput = TRUE;
		corelex_n_option = FALSE;
		isNOptionSet = FALSE;
		corelex_isWor = FALSE;
		strcpy(corelex_BBS, "@*&#");
		strcpy(corelex_CBS, "@*&#");
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (corelex_ftime) {
			corelex_ftime = FALSE;
			if (corelex_script_file == NULL) {
				fprintf(stderr,"Please specify words group file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"corelex +lCinderella\" or \"corelex +lCinderella.cut\".\n");
				cutt_exit(0);
			}
			corelex_read_script(corelex_script_file);
		}
	}
}

void usage() {
	printf("Creates table of frequency count of parts of speech and bound morphemes\n");
	printf("Usage: corelex [lF oN %s] filename(s)\n", mainflgs());
	puts("+lF: specify words group name F");
	puts("     if name has a dash (-), then it will be abbreviated at (-) to get the GEM name");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+o3: combine selected speakers from each file into one results list for that file");
	puts("+w : find first time values for words group from file specified with +l option");
	mainusage(FALSE);
	puts("Example:");
	puts("   To use words group from GEM \"Cinderella\"");
	puts("       corelex +lCinderella ...");
	puts("       corelex +lCinderella.cut ...");
	puts("   To find first time values for words group from GEM \"Cookie\"");
	puts("       corelex +w +lcookie-short ...");
	puts("       corelex +w +lcookie-short.cut ...");
	cutt_exit(0);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'l':
			corelex_script_file = f;
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
		case 'n':
			if (*(f-2) == '+') {
				corelex_n_option = TRUE;
				strcpy(corelex_BBS, "@G:");
				strcpy(corelex_CBS, "@*&#");
			} else {
				strcpy(corelex_BBS, "@BG:");
				strcpy(corelex_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
			break;
		case 'o':
			if (*f == '3') {
				corelex_isCombineSpeakers = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'w':
			corelex_isWor = TRUE;
			no_arg_option(f);
			break;
		case 't':
			if (*(f-2) == '+' && *f == '*' && *(f+1) == EOS) {
				corelex_isCombineSpeakers = TRUE;
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void fillInLabels(ALTLABELS *p, char *lables) {
	lables[0] = EOS;
	while (p != NULL) {
		if (lables[0] != EOS)
			strcat(lables, ",");
		strcat(lables, p->label);
		p = p->next_alt;
	}
}

static void print_labels(LABELS *col) {
	if (corelex_isWor == TRUE)
		excelStrCell(fpout, "Duration Time");
	for (; col != NULL; col=col->next_label) {
		if (col->labelP != NULL) {
			fillInLabels(col->labelP, templineC1);
			excelStrCell(fpout, templineC1);
		} else
			excelStrCell(fpout, ".");
	}
	if (corelex_isWor == FALSE)
		excelStrCell(fpout, "Types");
}

static void corelex_pr_result(void) {
	float fcount, types;
	LSP *p;
	LABELS  *col;

	p = root_sp;
	excelHeader(fpout, newfname, 95);
	excelRow(fpout, ExcelRowStart);
	excelCommasStrCell(fpout, "File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	if (p != NULL) {
		print_labels(labelsRoot);
	}
	excelRow(fpout, ExcelRowEnd);
	while (p != NULL) {
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, p->fname);
		if (p->ID != NULL) {
			excelOutputID(fpout, p->ID);
		} else {
			excelCommasStrCell(fpout, ".,.");
			excelStrCell(fpout, p->sp);
			excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
		}
		types = 0.0;
		if (p->isSpFound) {
			if (corelex_isWor == TRUE) {
				fcount = (float)p->totaTime; // pure speech time
//				excelNumCell(fpout, "%.0f", fcount);
				fcount = (float)p->endTotaTime - p->begTotaTime; // total time include pause between tiers
				excelNumCell(fpout, "%.0f", fcount);
			}
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				fcount = (float)p->count[col->num];
				if (corelex_isWor == TRUE) {
					if (p->count[col->num] == 0)
						excelStrCell(fpout, "NA");
					else if (p->count[col->num] == 1)
						excelNumCell(fpout, "%.0f", 0.0);
					else
						excelNumCell(fpout, "%.0f", fcount);
				} else
					excelNumCell(fpout, "%.0f", fcount);
				if (fcount > 0.0)
					types++;
			}
		} else {
			if (corelex_isWor == TRUE)
				excelStrCell(fpout, ".");
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				excelStrCell(fpout, ".");
			}
		}
		if (corelex_isWor == FALSE)
			excelNumCell(fpout, "%.0f", types);
		excelRow(fpout, ExcelRowEnd);
		p = p->next_sp;
	}
	excelFooter(fpout);
}

static LSP *findSpeaker(char *sp, char *ID, char isSpFound) {
	int  i;
	char *s;
	LSP *ts;

	uS.remblanks(sp);
	if (root_sp == NULL) {
		if ((ts=NEW(LSP)) == NULL) {
			corelex_error("Error: out of memory");
		}
		root_sp = ts;
	} else {
		for (ts=root_sp; ts->next_sp != NULL; ts=ts->next_sp) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && ts->uID == fileID) {
				ts->isSpFound = isSpFound;
				return(ts);
			}
		}
		if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && ts->uID == fileID) {
			ts->isSpFound = isSpFound;
			return(ts);
		}
		if ((ts->next_sp=NEW(LSP)) == NULL) {
			corelex_error("Error: out of memory");
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->ID = NULL;
	ts->fname = NULL;
	ts->sp = NULL;
	ts->isSpFound = isSpFound;
	ts->firstBulletTime = 0;
	ts->count = (unsigned int *)malloc(sizeof(unsigned int) * colLabelsNum);
	if (ts->count == NULL) {
		corelex_error("Error: out of memory");
	}
	for (i=0; i < colLabelsNum; i++)
		ts->count[i] = 0;
	ts->totaTime = 0L;
	ts->begTotaTime = 0L;
	ts->endTotaTime = 0L;
	ts->uID = fileID;
	if (ID == NULL)
		ts->ID = NULL;
	else {
		ts->ID = (char *)malloc(strlen(ID)+1);
		if (ts->ID == NULL) {
			corelex_error("Error: out of memory");
		}
		strcpy(ts->ID, ID);
	}
	ts->sp = (char *)malloc(strlen(sp)+1);
	if (ts->sp == NULL) {
		corelex_error("Error: out of memory");
	}
	strcpy(ts->sp, sp);
// ".xls"
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	if ((ts->fname=(char *)malloc(strlen(s)+1)) == NULL) {
		corelex_error("Error: out of memory");
	}	
	strcpy(ts->fname, s);
	s = strrchr(ts->fname, '.');
	if (s != NULL)
		*s = EOS;
	return(ts);
}

#define isStringEmpty(x) (x == NULL || *x == EOS)

static void addIxesMatchPat(IXXS *ixes, int max, char *stem, const char *s) {
	int i;
	
	for (i=0; i < max; i++) {
		if (!isStringEmpty(ixes[i].ix_s)) {
			if (s != NULL)
				strcat(stem, s);
			strcat(stem, ixes[i].ix_s);
		}
	}
}

static BOOL isMatch(char *stem, ALTLABELS *p) {
	while (p != NULL) {
		if (uS.mStricmp(stem, p->label) == 0)
			return(true);
		p = p->next_alt;
	}
	return(false);
}

static char isSuffixSearchAndMatch(MORFEATS *word_feat, ALTLABELS *p) {
	while (p != NULL) {
		if (isEqualIxes(p->label+1, word_feat->suffix, NUM_SUFF)) {
			return(TRUE);
		}
		p = p->next_alt;
	}
	return(FALSE);
}

static void countCols(LSP *p) {
	int  i, j, comps;
	char stem[BUFSIZ], word[BUFSIZ], *w[NUMCOMPS], isMatchFound;
	LABELS *col;
	MORFEATS *word_clitc, *word_feat, word_feats;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
		uS.remblanks(word);
		if (strchr(word, '|') != NULL) {
			comps = 0;
			w[comps++] = word;
			for (j=0; word[j] != EOS; j++) {
				if (word[j] == '~' || word[j] == '$') {
					word[j] = EOS;
					j++;
					if (comps >= NUMCOMPS) {
						fprintf(stderr, "    Intenal Error: # of pre/post clitics is > %d in word: \"%s\"\n", NUMCOMPS-1, word);
						labelsRoot = corelex_freeLabels(labelsRoot);
						root_sp = corelex_freeLSp(root_sp);
						cutt_exit(0);
					}
					w[comps++] = word+j;
				}
			}
			for (j=0; j < comps; j++) {
				for (col=labelsRoot; col != NULL; col=col->next_label) {
					if (col->isStem == TRUE) {
						strcpy(templineC1, w[j]);
						if (ParseWordMorElems(templineC1, &word_feats) == FALSE)
							corelex_error("Error: out of memory");
						stem[0] = EOS;
						for (word_clitc=&word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
							for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
								if (word_feat->stem != NULL) {
									addIxesMatchPat(word_feat->prefix, NUM_PREF, stem, NULL);
									strcat(stem, word_feat->stem);
								}
							}
						}
						freeUpFeats(&word_feats);
						if (isMatch(stem, col->labelP)) {
							p->count[col->num] = p->count[col->num] + 1;
							break;
						}
					}
				}
				
				isMatchFound = FALSE;
				for (col=labelsRoot; col != NULL; col=col->next_label) {
					if (col->isStem == FALSE) {
						strcpy(templineC1, w[j]);
						if (ParseWordMorElems(templineC1, &word_feats) == FALSE)
							corelex_error("Error: out of memory");
						for (word_clitc=&word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
							for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
								if (isSuffixSearchAndMatch(word_feat, col->labelP)) {
									p->count[col->num] = p->count[col->num] + 1;
									isMatchFound = TRUE;
								}
							}
						}
						freeUpFeats(&word_feats);
						if (isMatchFound == TRUE)
							break;
					}
				}
			}
		}
	}
}

static long getTierTime(char *line) {
	int  i;
	long beg, end, totalTime = 0;

	for (i=0; line[i] != HIDEN_C && line[i] != EOS; i++) ;
	if (line[i] == HIDEN_C) {
		if (getMediaTagInfo(line+i, &beg, &end)) {
			totalTime = end - beg;
		}
	}
	return(totalTime);
}

static unsigned int bulletTime(char *line, int i, char *word) {
	unsigned int begTime;
	long beg, end;

	begTime = 0;
	for (; line[i] != HIDEN_C && line[i] != EOS; i++) ;
//	for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == HIDEN_C) {
		if (getMediaTagInfo(line+i, &beg, &end)) {
			if (beg == 0L)
				begTime = 1;
			else
				begTime = beg;
		}
	} else {
#ifdef UNX
		printf("\nTarget word \"%s\" is missing bullet(s) on %%wor tier in this file.\n\n", word);
#else
		printf("\n%c%cTarget word \"%s\" is missing bullet(s) on %%wor tier in this file.%c%c\n\n", ATTMARKER, error_start, word, ATTMARKER, error_end);
#endif
	}
	return(begTime);
}

static unsigned int findFirstBulletTime(char *line) {
	int i;
	unsigned int begTime;
	long beg, end;

	begTime = 0;
	for (i=0; line[i] != HIDEN_C && line[i] != EOS; i++) ;
	if (line[i] == HIDEN_C) {
		if (getMediaTagInfo(line+i, &beg, &end)) {
			begTime = beg;
		}
	}
	return(begTime);
}

static long findLastBulletTime(char *line) {
	int i;
	long endTime;
	long beg, end;

	endTime = 0;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == HIDEN_C) {
			if (getMediaTagInfo(line+i, &beg, &end)) {
				endTime = end;
			}
		}
	}
	return(endTime);
}

static void countColTimes(LSP *p) {
	int  i;
	char word[BUFSIZ];
	LABELS *col;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
		uS.remblanks(word);
		for (col=labelsRoot; col != NULL; col=col->next_label) {
			if (isMatch(word, col->labelP)) {
				if (p->count[col->num] == 0) {
					p->count[col->num] = bulletTime(utterance->line, i, word);
					if (p->count[col->num] == 0 && p->firstBulletTime > 0)
						p->count[col->num] = 0;
					else
						p->count[col->num] = p->count[col->num] - p->firstBulletTime;
				}
				break;
			}
		}
	}
}

static int isRightText(char *gem_word) {
	int i = 0;
	char word[BUFSIZ];
	
	while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
		if (uS.mStricmp(gem_word, word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

void call() {
	LSP *ts = NULL;
	char spname[SPEAKERLEN];
	char lRightspeaker, isOutputGem, isAnyGemFound, isWorFound, isLastBulletFound;
	long currentBegTime, tLong;

	if (isNOptionSet == FALSE) {
		strcpy(corelex_BBS, "@*&#");
		strcpy(corelex_CBS, "@*&#");
	}
	isLastBulletFound = FALSE;
	isAnyGemFound = FALSE;
	isOutputGem = FALSE; // TRUE;
	isWorFound = FALSE;
	fileID++;
	spname[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
//				if (isPostCodeOnUtt(utterance->line, "[+ exc]"))
//					lRightspeaker = FALSE;
//				else
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!strcmp(corelex_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				corelex_n_option = FALSE;
				strcpy(corelex_BBS, "@BG:");
				strcpy(corelex_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				corelex_n_option = TRUE;
				strcpy(corelex_BBS, "@G:");
				strcpy(corelex_CBS, "@*&#");
			}
		}
		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC1, TRUE)) {
					uS.remblanks(utterance->line);
					findSpeaker(templineC1, utterance->line, FALSE);
				}
			}
			if (uS.partcmp(utterance->speaker,corelex_BBS,FALSE,FALSE)) {
				if (corelex_n_option) {
					if (isRightText(corelex_script_file)) {
						isOutputGem = TRUE;
					} else
						isOutputGem = FALSE;
				} else {
					if (isRightText(corelex_script_file)) {
						isOutputGem = TRUE;
					}
				}
			} else if (uS.partcmp(utterance->speaker,corelex_CBS,FALSE,FALSE)) {
				if (corelex_n_option) {
				} else {
					if (isRightText(corelex_script_file)) {
						isOutputGem = FALSE;
					}
				}
			}
		} else if (isOutputGem) {
			isAnyGemFound = TRUE;
			if (utterance->speaker[0] == '*') {
				if (corelex_isCombineSpeakers) {
					strcpy(spname, "*COMBINED*");
				} else {
					strcpy(spname, utterance->speaker);
				}
				ts = findSpeaker(spname, NULL, TRUE);
				if (corelex_isWor == TRUE) {
					ts->totaTime = ts->totaTime + getTierTime(utterance->line);
				}
				isLastBulletFound = FALSE;
			} else if (corelex_isWor==FALSE && uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE) && ts!=NULL) {
				countCols(ts);
			} else if (uS.partcmp(utterance->speaker, "%wor:", FALSE, FALSE) && ts != NULL) {
				isWorFound = TRUE;
				currentBegTime = findFirstBulletTime(utterance->line);
				if (ts->begTotaTime == 0L)
					ts->begTotaTime = currentBegTime;
				if (ts->firstBulletTime == 0)
					ts->firstBulletTime = currentBegTime;
//				else
//					ts->firstBulletTime = ts->firstBulletTime + (currentBegTime-ts->endTotaTime); // pure time & offset for pauses
				tLong = findLastBulletTime(utterance->line);
				if (tLong != 0L) {
					isLastBulletFound = TRUE;
					ts->endTotaTime = tLong;
				}
				if (corelex_isWor == TRUE)
					countColTimes(ts);
			}
		}
	}
	if (!isAnyGemFound) {
#ifdef UNX
		printf("\nNo specified GEM(s) found in this file.\n\n");
#else
		printf("\n%c%cNo specified GEM(s) found in this file.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	}
	if (corelex_isWor == TRUE) {
		if (isWorFound == FALSE) {
#ifdef UNX
			printf("\nNo %%wor tier found in this file.\n\n");
#else
			printf("\n%c%cNo %%wor tier found in this file.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		} else if (isLastBulletFound == FALSE) {
#ifdef UNX
			printf("\nNo bullets found on last tier in this file.\n\n");
#else
			printf("\n%c%cNo bullets found on last tier in this file.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		}
	}
	if (!combinput) {
		corelex_pr_result();
		root_sp = corelex_freeLSp(root_sp);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CORELEX;
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
	bmain(argc,argv,corelex_pr_result);
	labelsRoot = corelex_freeLabels(labelsRoot);
	root_sp = corelex_freeLSp(root_sp);
}
