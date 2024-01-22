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
#define _main codes_main
#define call codes_call
#define getflag codes_getflag
#define init codes_init
#define usage codes_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define LABELS struct labels_list
LABELS {
	char *code;
	char *colon1;
	LABELS *next_label;
};

#define COLS struct cols_list
COLS {
	LABELS *labelP;
	char *name;
	long count;
	COLS *next_col;
};

#define LSP struct levelsp_tnode
LSP {
	char isFound;
	char *ID;
	char *fname;
	char *sp;
	int  uID;
	COLS *cols1;
	long MCNA;
	long totalMCs;
	COLS *cols2;
	LSP *next_sp;
} ;

extern char R8;
extern char OverWriteFile;
extern char outputOnlyData;
extern struct IDtype *IDField;

static int fileID;
static char codes_ftime;
static LABELS *label1Root, *label2Root;
static LSP *root_sp;

static LABELS *freeLabels(LABELS *p) {
	LABELS *t;

	while (p != NULL) {
		t = p;
		p = p->next_label;
		if (t->code)
			free(t->code);
		if (t->colon1)
			free(t->colon1);
		free(t);
	}
	return(NULL);
}

static COLS *freeColsP(COLS *p) {
	COLS *t;

	while (p != NULL) {
		t = p;
		p = p->next_col;
		if (t->name != NULL)
			free(t->name);
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
		if (t->cols1 != NULL)
			freeColsP(t->cols1);
		if (t->cols2 != NULL)
			freeColsP(t->cols2);
		free(t);
	}
	return(NULL);
}

static void codes_error(const char *mess) {
	root_sp = freeLSp(root_sp);
	label1Root = freeLabels(label1Root);
	label2Root = freeLabels(label2Root);
	fprintf(stderr, "%s\n", mess);
	cutt_exit(0);
}

static LABELS *codes_addNewLabel(const char *code, const char *colon1, char isFirst) {
	int num;
	LABELS *root, *tL, *t, *tt;

	num = atoi(colon1);
	if (isFirst)
		root = label1Root;
	else
		root = label2Root;
	tL = NEW(LABELS);
	if (tL == NULL) {
		codes_error("Error: out of memory");
	}
	if (root == NULL) {
		root = tL;
		tL->next_label = NULL;
	} else if ((num == 0 && strcmp(root->code, code) == 0 && strcmp(root->colon1, colon1) == 0) ||
			   (num > 0  && strcmp(root->code, code) == 0 && atol(root->colon1) == num)) {
		return(root);
	} else if ((num == 0 && strcmp(root->code, code) >= 0 && strcmp(root->colon1, colon1) >= 0) ||
			   (num > 0  && strcmp(root->code, code) >= 0 && atol(root->colon1) >= num)) {
		tL->next_label = root;
		root = tL;
	} else {
		t = root;
		tt = root->next_label;
		while (tt != NULL) {
			if ((num == 0 && strcmp(tt->code, code) == 0 && strcmp(tt->colon1, colon1) == 0) ||
				(num > 0  && strcmp(tt->code, code) == 0 && atol(tt->colon1) == num)) {
				return(tt);
			}
			if ((num == 0 && strcmp(tt->code, code) >= 0 && strcmp(tt->colon1, colon1) >= 0) ||
				(num > 0  && strcmp(tt->code, code) >= 0 && atol(tt->colon1) >= num))
				break;
			t = tt;
			tt = tt->next_label;
		}
		if (tt == NULL) {
			t->next_label = tL;
			tL->next_label = NULL;
		} else {
			tL->next_label = tt;
			t->next_label = tL;
		}
	}
	if ((tL->code=(char *)malloc(strlen(code)+1)) == NULL) {
		codes_error("Error: out of memory");
	}
	strcpy(tL->code, code);
	if ((tL->colon1=(char *)malloc(strlen(colon1)+1)) == NULL) {
		codes_error("Error: out of memory");
	}
	strcpy(tL->colon1, colon1);
	if (isFirst)
		label1Root = root;
	else
		label2Root = root;
	return(tL);
}

void init(char f) {
	if (f) {
		fileID = 0;
		root_sp = NULL;
		label1Root = NULL;
		label2Root = NULL;
		codes_ftime = TRUE;
		stout = FALSE;
		OverWriteFile = TRUE;
		outputOnlyData = TRUE;
		AddCEXExtension = ".xls";
		combinput = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
	} else {
		if (codes_ftime) {
			codes_ftime = FALSE;
			maketierchoice("@ID:",'+','\001');
			maketierchoice("%cod:",'+','\001');
		}
	}
}

void usage() {
	printf("Creates spreadsheet of entry and frequency count of MC codes\n");
	printf("Usage: codes [%s] filename(s)\n", mainflgs());
	mainusage(FALSE);
	cutt_exit(0);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static LSP *findSpeaker(char *sp, char *ID) {
	char *s;
	LSP *ts;

	uS.remblanks(sp);
	if (root_sp == NULL) {
		if ((ts=NEW(LSP)) == NULL) {
			codes_error("Error: out of memory");
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
			codes_error("Error: out of memory");
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->isFound = FALSE;
	ts->ID = NULL;
	ts->fname = NULL;
	ts->sp = NULL;
	ts->uID = fileID;
	ts->cols1 = NULL;
	ts->MCNA = 0L;
	ts->totalMCs = 0L;
	ts->cols2 = NULL;
	if (ID == NULL)
		ts->ID = NULL;
	else {
		ts->ID = (char *)malloc(strlen(ID)+1);
		if (ts->ID == NULL) {
			codes_error("Error: out of memory");
		}
		strcpy(ts->ID, ID);
	}
	ts->sp = (char *)malloc(strlen(sp)+1);
	if (ts->sp == NULL) {
		codes_error("Error: out of memory");
	}
	strcpy(ts->sp, sp);
// ".xls"
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	if ((ts->fname=(char *)malloc(strlen(s)+1)) == NULL) {
		codes_error("Error: out of memory");
	}	
	strcpy(ts->fname, s);
	s = strrchr(ts->fname, '.');
	if (s != NULL)
		*s = EOS;
	return(ts);
}

static char isAlreadyPresent(char *cName, char *name) {
	char *b, *e, t;

	b = cName;
	while (*b != EOS) {
		for (e=b; *e != ',' && *e != ' ' && *e != EOS; e++) ;
		if (*e != EOS) {
			t = *e;
			*e = EOS;
			if (uS.mStricmp(b, name) == 0) {
				*e = t;
				return(TRUE);
			}
			*e = t;
			for (; *e == ',' || *e == ' '; e++) ;
		} else if (uS.mStricmp(b, name) == 0) {
			return(TRUE);
		}
		b = e;
	}
	return(FALSE);
}

static COLS *code_addNewCol(COLS *root, char *name, LABELS *cLabel, char isFirst) {
	char *s;
	COLS *p;

	if (root == NULL) {
		root = NEW(COLS);
		p = root;
	} else {
		for (p=root; p->next_col != NULL; p=p->next_col) {
			if (p->labelP == cLabel) {
				p->count++;
				if (isFirst && p->name != NULL && !isAlreadyPresent(p->name, name)) {
					s = p->name;
					if ((p->name=(char *)malloc(strlen(s)+strlen(name)+1)) == NULL) {
						codes_error("Error: out of memory");
					}
					strcpy(p->name, s);
					strcat(p->name, ", ");
					strcat(p->name, name);
					free(s);
				}
				return(root);
			}
		}
		if (p->labelP == cLabel) {
			p->count++;
			if (isFirst && p->name != NULL && !isAlreadyPresent(p->name, name)) {
				s = p->name;
				if ((p->name=(char *)malloc(strlen(s)+strlen(name)+1)) == NULL) {
					codes_error("Error: out of memory");
				}
				strcpy(p->name, s);
				strcat(p->name, ", ");
				strcat(p->name, name);
				free(s);
			}
			return(root);
		}
		p->next_col = NEW(COLS);
		p = p->next_col;
	}
	if (p == NULL) {
		codes_error("Error: out of memory");
	}
	p->count = 1;
	if ((p->name=(char *)malloc(strlen(name)+1)) == NULL) {
		codes_error("Error: out of memory");
	}
	strcpy(p->name, name);
	p->labelP = cLabel;
	p->next_col = NULL;
	return(root);
}

static void addCode(LSP *p) {
	int  i;
	char code[BUFSIZ], *colon1, *colon2;
	LABELS *cLabel;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, code, NULL, i))) {
		uS.remblanks(code);
		colon1 = strchr(code, ':');
		if (colon1 != NULL) {
			*colon1 = EOS;
			colon1++;
			colon2 = strchr(colon1, ':');
			if (colon2 != NULL) {
				*colon2 = EOS;
				colon2++;
			}
		}
		if (colon2 != NULL) {
			cLabel = codes_addNewLabel(code, colon1, TRUE);
			p->cols1 = code_addNewCol(p->cols1, colon2, cLabel, TRUE);
			cLabel = codes_addNewLabel(colon2, ".", FALSE);
			p->cols2 = code_addNewCol(p->cols2, colon2, cLabel, FALSE);
			p->totalMCs++;
		} else if (uS.mStricmp(code, "$MC") == 0 && uS.mStricmp(colon1, "NA") == 0) {
			p->MCNA++;
		}
	}
}

static void print_labels(LABELS *cLabel) {
	for (; cLabel != NULL; cLabel=cLabel->next_label) {
		if (cLabel->colon1[0] == '.' && cLabel->colon1[1] == EOS) {
			strcpy(templineC, cLabel->code);
			strcat(templineC, " total");
			excelStrCell(fpout, templineC);
		} else {
			strcpy(templineC, cLabel->code);
			strcat(templineC, ":");
			strcat(templineC, cLabel->colon1);
			excelStrCell(fpout, templineC);
		}
	}
}

static void print_vals(LSP *ts) {
	LABELS *cLabel;
	COLS *col;
	float tf, Composite;

	for (cLabel=label1Root; cLabel != NULL; cLabel=cLabel->next_label) {
		for (col=ts->cols1; col != NULL; col=col->next_col) {
			if (col->labelP == cLabel) {
				excelStrCell(fpout, col->name);
				break;
			}
		}
		if (col == NULL)
			excelStrCell(fpout, ".");
	}
	tf = ts->MCNA;
	excelNumCell(fpout, "%.0f", tf);
	tf = ts->totalMCs;
	excelNumCell(fpout, "%.0f", tf);
	Composite = 0.0;
	for (cLabel=label2Root; cLabel != NULL; cLabel=cLabel->next_label) {
		for (col=ts->cols2; col != NULL; col=col->next_col) {
			if (col->labelP == cLabel) {
				tf = col->count;
				excelNumCell(fpout, "%.0f", tf);
				if (uS.mStricmp(cLabel->code, "AC") == 0)
					Composite += (tf * 3);
				else if (uS.mStricmp(cLabel->code, "AI") == 0 || uS.mStricmp(cLabel->code, "IC") == 0)
					Composite += (tf * 2);
				else if (uS.mStricmp(cLabel->code, "II") == 0)
					Composite += (tf * 1);
				break;
			}
		}
		if (col == NULL)
			excelStrCell(fpout, ".");
	}
	excelNumCell(fpout, "%.0f", Composite);
}

static void LSp_pr_result(void) {
	LSP *p;

	excelHeader(fpout, newfname, 95);
	excelRow(fpout, ExcelRowStart);
	excelCommasStrCell(fpout, "File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	print_labels(label1Root);
	excelCommasStrCell(fpout, "$MC:NA,Total # of MCs");
	print_labels(label2Root);
	excelCommasStrCell(fpout, "Composite");
	excelRow(fpout, ExcelRowEnd);
	for (p=root_sp; p != NULL; p=p->next_sp) {
		if (p->isFound == TRUE) {
			excelRow(fpout, ExcelRowStart);
			excelStrCell(fpout, p->fname);
			if (p->ID != NULL) {
				excelOutputID(fpout, p->ID);
			} else {
				excelCommasStrCell(fpout, ".,.");
				excelStrCell(fpout, p->sp);
				excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
			}
			print_vals(p);
			excelRow(fpout, ExcelRowEnd);
		}
	}
	excelFooter(fpout);
}

void call() {
	LSP *ts = NULL;

	fileID++;
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
			ts = findSpeaker(utterance->speaker, NULL);
		} else if (uS.partcmp(utterance->speaker, "%cod:", FALSE, FALSE) && ts != NULL) {
			ts->isFound = TRUE;
			addCode(ts);
		}
	}
	if (!combinput) {
		LSp_pr_result();
		fileID = 0;
		label1Root = freeLabels(label1Root);
		label2Root = freeLabels(label2Root);
		root_sp = freeLSp(root_sp);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CODES_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,LSp_pr_result);
	label1Root = freeLabels(label1Root);
	label2Root = freeLabels(label2Root);
	root_sp = freeLSp(root_sp);
}
