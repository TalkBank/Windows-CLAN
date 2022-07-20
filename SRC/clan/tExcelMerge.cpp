/**********************************************************************
 "Copyright 1990-2015 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

// get infor from slave tab saparated Excell file and add it to master file

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

#define TROWS struct TextRows
TROWS {
    char *code;
	char *sText;
    char *oText;
	TROWS *next_row;
} ;

static char *slave, ftime;
static TROWS *rootRows;

void usage() {
	printf("Usage: temp [sF %s] filename(s)\n",mainflgs());
	puts("+sF: specify slave filename F");
	mainusage(FALSE);
	puts("\nExample:\ntemp -sslave.txt master.txt");
	cutt_exit(0);
}

static TROWS *freeTempRows(TROWS *p) {
	TROWS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_row;
		if (t->code != NULL)
			free(t->code);
		if (t->sText != NULL)
			free(t->sText);
		if (t->oText != NULL)
			free(t->oText);
		free(t);
	}
	return(NULL);
}

static void remSpaces(char *s) {
	while (*s != EOS) {
		if (!isalnum(*s))
			strcpy(s, s+1);
		else
			s++;
	}
}

static void cleanText(char *orgS) {
	char *s;
	int cnt;

	cnt = 0;
	for (s=orgS; *s != EOS; s++) {
		if (*s == '"')
			cnt++;
	}
	if (cnt % 2) {
		s = orgS;
		while (*s != EOS) {
			if (*s == '"')
				strcpy(s, s+1);
			else
				s++;
		}
	}
}


static TROWS *addToRows(TROWS *root, char *code, char *text) {
	TROWS *t;
	
	if (root == NULL) {
		t = NEW(TROWS);
		if (t == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			root = freeTempRows(root);
			cutt_exit(0);
		}
		root = t;
	} else {
		for (t=root; t->next_row != NULL; t=t->next_row) ;
		if ((t->next_row=NEW(TROWS)) == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			root = freeTempRows(root);
			cutt_exit(0);
		}
		t = t->next_row;
	}
	t->next_row = NULL;
	t->code = NULL;
	t->sText = NULL;
	t->oText = NULL;
	if ((t->code=(char *)malloc(strlen(code)+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTempRows(root);
		cutt_exit(0);
	}
	strcpy(t->code, code);
	if ((t->oText=(char *)malloc(strlen(text)+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTempRows(root);
		cutt_exit(0);
	}
	strcpy(t->oText, text);
	remSpaces(text);
	if ((t->sText=(char *)malloc(strlen(text)+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTempRows(root);
		cutt_exit(0);
	}
	strcpy(t->sText, text);
	return(root);
}

static void readSlaveFile(void) {
	int  ln;
	char *code, *text;
	FILE *fp;

	fp = fopen(slave, "r");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file \"%s\".\n", slave);
		cutt_exit(0);
	}
	rootRows = NULL;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC,FONTHEADER,FALSE,FALSE))
			continue;
		ln++;
		uS.remFrontAndBackBlanks(templineC);
		if (*templineC != EOS) {
			code = templineC;
			for (text=templineC; *text != EOS && *text != '\t'; text++) ;
			if (text == EOS) {
				fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
				fprintf(stderr, "Please specify slave filename with \"+sF\" option.\n");
				cutt_exit(0);
			}
			*text = EOS;
			for (text++; isSpace(*text); text++) ;
			cleanText(text);
			rootRows = addToRows(rootRows, code, text);
		}
	}
	if (rootRows == NULL) {
		fprintf(stderr, "Can't find any rows in slave filename %s.\n", slave);
		cutt_exit(0);
	}
}

void init(char f) {
	if (f) {
		ftime = TRUE;
		slave = NULL;
		rootRows = NULL;
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".xls";
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (ftime) {
			ftime = FALSE;
			if (slave == NULL) {
				fprintf(stderr, "Please specify slave filename with \"+sF\" option.\n");
				cutt_exit(0);
			}
			readSlaveFile();
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	rootRows = NULL;
	bmain(argc,argv,NULL);
	rootRows = freeTempRows(rootRows);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 's':
			slave = f;
    		break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char isSubSet(char *big, char *small, int len) {
	while (*big != EOS) {
		if (uS.mStrnicmp(big, small, len) == 0)
			return(TRUE);
		big++;
	}
	return(FALSE);
}

static char findCode(char *code, char *res) {
	TROWS *t;

	for (t=rootRows; t != NULL; t=t->next_row) {
		if (uS.mStricmp(t->code, code) == 0) {
			if (t->sText != NULL) {
				if (isSubSet(t->sText, res, 20)) {
					strcpy(res, t->oText);
					free(t->oText);
					t->oText = NULL;
					free(t->sText);
					t->sText = NULL;
					return(TRUE);
				} else if (isSubSet(res, t->sText, 20)) {
					strcpy(res, t->oText);
					free(t->oText);
					t->oText = NULL;
					free(t->sText);
					t->sText = NULL;
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

void call(void) {
	int  ln;
	char *res, *code, *text;

	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC,FONTHEADER,FALSE,FALSE))
			continue;
		ln++;
		uS.remblanks(templineC);
		if (*templineC != EOS) {
			res = templineC;
			for (text=templineC; *text != EOS && *text != '\t'; text++) ;
			if (*text == EOS) {
				fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
				fprintf(stderr, "Can't find sentence and link columns.\n");
				cutt_exit(0);
			}
			*text = EOS;
			for (text++; *text == ' '; text++) ;
			for (code=text; *code != EOS && *code != '\t'; code++) ;
			if (*code == EOS) {
				fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
				fprintf(stderr, "Can't find link column.\n");
				cutt_exit(0);
			}
			*code = EOS;
			for (code++; *code == ' '; code++) ;
			cleanText(text);
			strcpy(templineC1, text);
			remSpaces(templineC1);
			if (findCode(code, templineC1)) {
				fprintf(fpout, "%s\t%s\t%s\n", templineC1, text, code);
			} else {
				fprintf(fpout, "%s\t%s\t%s\n", res, text, code);
			}
		}
	}
}
