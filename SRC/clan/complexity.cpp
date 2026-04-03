/**********************************************************************
	"Copyright 1990-2026 Brian MacWhinney. Use is subject to Gnu Public License
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
#define _main complexity_main
#define call complexity_call
#define getflag complexity_getflag
#define init complexity_init
#define usage complexity_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CSP struct complexity_tnode
CSP {
	char isFound;
	char *ID;
	char *fname;
	char *sp;
	int  uID;
	float CSUBJ;

	float CCOMP;
	float XCOMP;
	float ACL;
	float ADVCL;
	float APPOS;
	float EXPL;
	
	float COMP;
	float CPRED;
	float CPOBJ;
	float COBJ;
	float CJCT;
	float XJCT;
	float NJCT;
	float CMOD;
	float XMOD;
	float tokens;
	float total_tokens;
	CSP *next_sp;
} ;

extern char OverWriteFile;
extern char outputOnlyData;

static int fileID;
static char complexity_ftime;
static CSP *root_sp;

static CSP *freeCSp(CSP *p) {
	CSP *t;
	
	if (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->ID != NULL)
			free(t->ID);
		if (t->fname != NULL)
			free(t->fname);
		if (t->sp != NULL)
			free(t->sp);
		free(t);
	}
	return(NULL);
}

static void complexity_error(const char *mess) {
	root_sp = freeCSp(root_sp);
	fprintf(stderr, "%s\n", mess);
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		fileID = 0;
		root_sp = NULL;
		complexity_ftime = TRUE;
		stout = FALSE;
		outputOnlyData = TRUE;
		OverWriteFile = TRUE;
		AddCEXExtension = ".xls";
		combinput = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
	} else {
		if (complexity_ftime) {
			complexity_ftime = FALSE;
			maketierchoice("@ID:",'+','\001');
			maketierchoice("%gra:",'+','\001');
		}
	}
}

void usage() {
	printf("Compute complexity ratio from %gra tier\n");
	printf("Usage: codes [%s] filename(s)\n", mainflgs());
	mainusage(FALSE);
	cutt_exit(0);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 's':
			if (*f == '[' && *(f+1) == '-') {
				maingetflag(f-2,f1,i);
			} else if ((*f == '[' && *(f+1) == '+') || ((*f == '+' || *f == '~') && *(f+1) == '[' && *(f+2) == '+')) {
				maingetflag(f-2,f1,i);
			} else {
				fprintf(stderr, "Please specify only postcodes, \"[+ ...]\", or precodes \"[- ...]\" with +/-s option.\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static CSP *findSpeaker(char *sp, char *ID) {
	char *s;
	CSP *ts;

	uS.remblanks(sp);
	if (root_sp == NULL) {
		if ((ts=NEW(CSP)) == NULL) {
			complexity_error("Error: out of memory");
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
		if ((ts->next_sp=NEW(CSP)) == NULL) {
			complexity_error("Error: out of memory");
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->isFound = FALSE;
	ts->ID = NULL;
	ts->fname = NULL;
	ts->sp = NULL;
	ts->uID = fileID;
	ts->CSUBJ = 0.0;

	ts->CCOMP = 0.0;
	ts->XCOMP = 0.0;
	ts->ACL = 0.0;
	ts->ADVCL = 0.0;
	ts->APPOS = 0.0;
	ts->EXPL = 0.0;

	ts->COMP = 0.0;
	ts->CPRED = 0.0;
	ts->CPOBJ = 0.0;
	ts->COBJ = 0.0;
	ts->CJCT = 0.0;
	ts->XJCT = 0.0;
	ts->NJCT = 0.0;
	ts->CMOD = 0.0;
	ts->XMOD = 0.0;

	if (ID == NULL)
		ts->ID = NULL;
	else {
		ts->ID = (char *)malloc(strlen(ID)+1);
		if (ts->ID == NULL) {
			complexity_error("Error: out of memory");
		}
		strcpy(ts->ID, ID);
	}
	ts->sp = (char *)malloc(strlen(sp)+1);
	if (ts->sp == NULL) {
		complexity_error("Error: out of memory");
	}
	strcpy(ts->sp, sp);
// ".xls"
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	if ((ts->fname=(char *)malloc(strlen(s)+1)) == NULL) {
		complexity_error("Error: out of memory");
	}	
	strcpy(ts->fname, s);
	s = strrchr(ts->fname, '.');
	if (s != NULL)
		*s = EOS;
	return(ts);
}

static void contCodes(CSP *p) {
	int  i;
	char codeItem[BUFSIZ], *code;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, codeItem, NULL, i))) {
		uS.remblanks(codeItem);
		code = strrchr(codeItem, '|');
		if (code == NULL)
			return;
		code++;
		if (uS.mStricmp(code, "PUNCT") != 0)
			p->total_tokens++;
		if (uS.mStricmp(code, "CSUBJ") == 0 || uS.mStrnicmp(code, "CSUBJ-", 6) == 0 || uS.mStrnicmp(code, "CSUBJ:", 6) == 0) {
			p->CSUBJ++;
			p->tokens++;
		} else if (uS.mStricmp(code, "CCOMP") == 0 || uS.mStrnicmp(code, "CCOMP-", 6) == 0 || uS.mStrnicmp(code, "CCOMP:", 6) == 0) {
			p->CCOMP++;
			p->tokens++;
		} else if (uS.mStricmp(code, "XCOMP") == 0 || uS.mStrnicmp(code, "XCOMP-", 6) == 0 || uS.mStrnicmp(code, "XCOMP:", 6) == 0) {
			p->XCOMP++;
			p->tokens++;
		} else if (uS.mStricmp(code, "ACL") == 0 || uS.mStrnicmp(code, "ACL-", 4) == 0 || uS.mStrnicmp(code, "ACL:", 4) == 0) {
			p->ACL++;
			p->tokens++;
		} else if (uS.mStricmp(code, "ADVCL") == 0 || uS.mStrnicmp(code, "ADVCL-", 6) == 0 || uS.mStrnicmp(code, "ADVCL:", 6) == 0) {
			p->ADVCL++;
			p->tokens++;
		} else if (uS.mStricmp(code, "APPOS") == 0 || uS.mStrnicmp(code, "APPOS-", 6) == 0 || uS.mStrnicmp(code, "APPOS:", 6) == 0) {
			p->APPOS++;
			p->tokens++;
		} else if (uS.mStricmp(code, "EXPL") == 0 || uS.mStrnicmp(code, "EXPL-", 5) == 0 || uS.mStrnicmp(code, "EXPL:", 5) == 0) {
			p->EXPL++;
			p->tokens++;
		} else if (uS.mStricmp(code, "COMP") == 0 || uS.mStrnicmp(code, "COMP-", 5) == 0 || uS.mStrnicmp(code, "COMP:", 5) == 0) {
			p->COMP++;
			p->tokens++;
		} else if (uS.mStricmp(code, "CPRED") == 0 || uS.mStrnicmp(code, "CPRED-", 6) == 0 || uS.mStrnicmp(code, "CPRED:", 6) == 0) {
			p->CPRED++;
			p->tokens++;
		} else if (uS.mStricmp(code, "CPOBJ") == 0 || uS.mStrnicmp(code, "CPOBJ-", 6) == 0 || uS.mStrnicmp(code, "CPOBJ:", 6) == 0) {
			p->CPOBJ++;
			p->tokens++;
		} else if (uS.mStricmp(code, "COBJ") == 0 || uS.mStrnicmp(code, "COBJ-", 5) == 0 || uS.mStrnicmp(code, "COBJ:", 5) == 0) {
			p->COBJ++;
			p->tokens++;
		} else if (uS.mStricmp(code, "CJCT") == 0 || uS.mStrnicmp(code, "CJCT-", 5) == 0 || uS.mStrnicmp(code, "CJCT:", 5) == 0) {
			p->CJCT++;
			p->tokens++;
		} else if (uS.mStricmp(code, "XJCT") == 0 || uS.mStrnicmp(code, "XJCT-", 5) == 0 || uS.mStrnicmp(code, "XJCT:", 5) == 0) {
			p->XJCT++;
			p->tokens++;
		} else if (uS.mStricmp(code, "NJCT") == 0 || uS.mStrnicmp(code, "NJCT-", 5) == 0 || uS.mStrnicmp(code, "NJCT:", 5) == 0) {
			p->NJCT++;
			p->tokens++;
		} else if (uS.mStricmp(code, "CMOD") == 0 || uS.mStrnicmp(code, "CMOD-", 5) == 0 || uS.mStrnicmp(code, "CMOD:", 5) == 0) {
			p->CMOD++;
			p->tokens++;
		} else if (uS.mStricmp(code, "XMOD") == 0 || uS.mStrnicmp(code, "XMOD-", 5) == 0 || uS.mStrnicmp(code, "XMOD:", 5) == 0) {
			p->XMOD++;
			p->tokens++;
		}
	}
}

static void LSp_pr_result(void) {
	CSP *ts;

	excelHeader(fpout, newfname, 95);
	excelRow(fpout, ExcelRowStart);
	excelCommasStrCell(fpout, "File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	excelCommasStrCell(fpout, "CSUBJ");
	if (MorCodes == UD) {
		excelCommasStrCell(fpout, "CCOMP,XCOMP,ACL,ADVCL,APPOS,EXPL");
	} else {
		excelCommasStrCell(fpout, "COMP,CPRED,CPOBJ,COBJ,CJCT,XJCT,NJCT,CMOD,XMOD");
	}
	excelCommasStrCell(fpout, "Tokens,TotalTokens,Ratio");
	excelRow(fpout, ExcelRowEnd);
	for (ts=root_sp; ts != NULL; ts=ts->next_sp) {
		if (ts->isFound == TRUE) {
			excelRow(fpout, ExcelRowStart);
			excelStrCell(fpout, ts->fname);
			if (ts->ID != NULL) {
				excelOutputID(fpout, ts->ID);
			} else {
				excelCommasStrCell(fpout, ".,.");
				excelStrCell(fpout, ts->sp);
				excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
			}
			excelNumCell(fpout, "%.0f", ts->CSUBJ);
			if (MorCodes == UD) {
				excelNumCell(fpout, "%.0f", ts->CCOMP);
				excelNumCell(fpout, "%.0f", ts->XCOMP);
				excelNumCell(fpout, "%.0f", ts->ACL);
				excelNumCell(fpout, "%.0f", ts->ADVCL);
				excelNumCell(fpout, "%.0f", ts->APPOS);
				excelNumCell(fpout, "%.0f", ts->EXPL);
			} else {
				excelNumCell(fpout, "%.0f", ts->COMP);
				excelNumCell(fpout, "%.0f", ts->CPRED);
				excelNumCell(fpout, "%.0f", ts->CPOBJ);
				excelNumCell(fpout, "%.0f", ts->COBJ);
				excelNumCell(fpout, "%.0f", ts->CJCT);
				excelNumCell(fpout, "%.0f", ts->XJCT);
				excelNumCell(fpout, "%.0f", ts->NJCT);
				excelNumCell(fpout, "%.0f", ts->CMOD);
				excelNumCell(fpout, "%.0f", ts->XMOD);
			}
			excelNumCell(fpout, "%.0f", ts->tokens);
			excelNumCell(fpout, "%.0f", ts->total_tokens);
			excelNumCell(fpout, "%f", ts->tokens/ts->total_tokens);
			excelRow(fpout, ExcelRowEnd);
		}
	}
	excelFooter(fpout);
}

void call() {
	CSP *ts = NULL;

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
		} else if (uS.partcmp(utterance->speaker, "%gra:", FALSE, FALSE) && ts != NULL) {
			ts->isFound = TRUE;
			contCodes(ts);
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = COMPLEXITY;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,LSp_pr_result);
	root_sp = freeCSp(root_sp);
}
