/**********************************************************************
	"Copyright 1990-2016 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 0
#include "cu.h"
#ifdef _WIN32
#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern char fgets_cr_lc;
extern struct tier *defheadtier;
extern char OverWriteFile;

#define TR_CH_NUM 10

#define TR_TABLE struct translation_table
struct translation_table {
	unsigned char ch;
	int cnt;
	unCH chrs[TR_CH_NUM];
	translation_table *next_char;
} ;

#define MISSING_CHARS struct missing_chars
struct missing_chars {
	char err[32];
	MISSING_CHARS *next_char;
} ;

static TR_TABLE *root_table;
static MISSING_CHARS *root_mchars;


static TR_TABLE *freeTrTable(TR_TABLE *p) {
	TR_TABLE *t;

	while (p != NULL) {
		t = p;
		p = p->next_char;
		free(t);
	}
	return(NULL);
}

static MISSING_CHARS *free_missingChrs(MISSING_CHARS *p, char isDisplay) {
	MISSING_CHARS *t;

	if (p != NULL && isDisplay) {
		fprintf(stderr, "\n\n********** ERROR ***********\n");
	}
	while (p != NULL) {
		t = p;
		p = p->next_char;
		if (isDisplay) {
			fprintf(stderr, "    Character not found: %s\n", t->err);
		}
		free(t);
	}
	return(NULL);
}

static void temp_exit(int i) {
	root_table = freeTrTable(root_table);
	root_mchars = free_missingChrs(root_mchars, FALSE);
	cutt_exit(i);
}

static void add_missing_chr(char *err) {
	MISSING_CHARS *t;

	if (root_mchars == NULL) {
		if ((root_mchars=NEW(MISSING_CHARS)) == NULL) {
			fprintf(stderr, "\nOut of memory.\n\n");
			temp_exit(0);
		}
		t = root_mchars;
	} else {
		for (t=root_mchars; 1; t=t->next_char) {
			if (!strcmp(err, t->err))
				return;
			if (t->next_char == NULL)
				break;
		}
		if ((t->next_char=NEW(MISSING_CHARS)) == NULL) {
			fprintf(stderr, "\nOut of memory.\n\n");
			temp_exit(0);
		}
		t = t->next_char;
	}
	strncpy(t->err, err, 31);
	t->err[31] = EOS;
	t->next_char = NULL;
}

static TR_TABLE *createTableRow(TR_TABLE *root, unsigned char ch, unCH *chrs, int cnt) {
	int i;
	TR_TABLE *p;

	if (root == NULL) {
		if ((p=NEW(TR_TABLE)) == NULL)
			temp_exit(0);
		root = p;
	} else {
		for (p=root; p->next_char != NULL; p=p->next_char) ;
		p->next_char = NEW(TR_TABLE);
		p = p->next_char;
	}
	if (p == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		temp_exit(0);
	}
	p->next_char = NULL;
	p->ch = ch;
	for (i=0; i < cnt; i++)
		p->chrs[i] = chrs[i];
	p->cnt = cnt;
	return(root);
}

static int extractUCs(char *line, int b, unsigned long *uc) {
	int e;

	for (e=b; line[e] != ' ' && line[e] != '\t' && line[e] != '\n' && line[e] != EOS; e++) ;
	if (line[b] == '0' && line[b+1] == 'x')
		b += 2;
	sscanf(line+b, "%lx", uc);
	return(e);
}

static void readConvFile(char *fname) {
	int  i, cnt;
	long ln;
	unsigned char ch;
	unCH chrs[TR_CH_NUM];
	FILE *fdic;
	unsigned long hex;
	FNType mFileName[FNSize];

	if ((fdic=OpenGenLib(fname,"r",TRUE,TRUE, mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", fname, mFileName);
		return;
	}
	fprintf(stderr, "    Using conversion table file: %s.\n", mFileName);
	ln = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fdic) != NULL) {
		uS.remFrontAndBackBlanks(templineC);
		ln++;
		if (uS.partwcmp(templineC, "key") || uS.partwcmp(templineC, "//")) {
			continue;
		}
		i=0;
		if (uS.isUTF8(templineC+i)) {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", mFileName, ln);
			fputs("Translation file must not be in UTF8 format\n",stderr);
			temp_exit(0);
		}
		if (templineC[i] == EOS)
			continue;
		if (!isSpace(templineC[i+1]) || (templineC[i] == '0' && templineC[i+1] == 'x')) {
			i = extractUCs(templineC, i, &hex);
			ch = (unsigned char)hex;
		} else {
			ch = (unsigned char)templineC[i];
			for (; templineC[i] != ' ' && templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) ;
		}
		for (; templineC[i] == ' ' || templineC[i] == '\t'; i++) ;
		if (templineC[i] == '\n' || templineC[i] == EOS) {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", mFileName, ln);
			fputs("Unexpected ending\n",stderr);
			temp_exit(0);
		}
		cnt = 0;
		while (1) {
			i = extractUCs(templineC, i, &hex);
			chrs[cnt++] = (unCH)hex;
			for (; templineC[i] == ' '; i++) ;
			if (templineC[i] == '\t' || templineC[i] == EOS || templineC[i] == '\n')
				break;
		}
		root_table = createTableRow(root_table, ch, chrs, cnt);
	}
	fclose(fdic);
}

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		root_table = NULL;
		root_mchars = NULL;
	} else {
		if (root_table == NULL) {
			fprintf(stderr, "\nCan't find translation table.\n");
			fprintf(stderr, "Please use +tF option to specify translation file's name.\n");
			temp_exit(0);
		}
	}
}

void usage() {
	printf("Usage: temp [tF] filename(s)\n");
	puts("+tF: specify translation's file name F.)");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	root_table = NULL;
	root_mchars = NULL;
	bmain(argc,argv,NULL);
	root_table = freeTrTable(root_table);
	root_mchars = free_missingChrs(root_mchars, TRUE);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 't':
			readConvFile(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void convert_PEP(unsigned char *from, unCH *to) {
	int i, j, k;
	TR_TABLE *p;

	j = 0;
	for (i=0L; from[i] != EOS; i++) {
		for (p=root_table; p != NULL; p=p->next_char) {
			if (p->ch == (unsigned char)from[i]) {
				for (k=0; k < p->cnt; k++) {
					to[j++] = p->chrs[k];
				}
				break;
			}
		}
		if (p == NULL) {
			to[j++] = (unCH)from[i];
			if (from[i] == ' ' || from[i] == '\t' || from[i] == '\n') {
			} else {
				sprintf(templineC1, "%c (0x%x)", from[i], from[i]);
				add_missing_chr(templineC1);
			}
		}
	}
	to[j] = EOS;
}

void call() {
	int  i, cnt;
	long ln;

	fprintf(fpout, "%s\t%s\n", "@Font:", "Charis SIL:14:0");
	fprintf(fpout, "%s\n", UTF8HEADER);
	fgets_cr_lc = '\0';
	ln = 0L;
	cnt = 0;
	while (fgets_cr(utterance->line, UTTLINELEN, fpin)) {
		ln++;
		uS.remFrontAndBackBlanks(utterance->line);
		if (utterance->line[0] == EOS || utterance->line[0] == 0x0C) {
			break;
		} else if (uS.partwcmp(utterance->line, "@Comment:")) {
			fprintf(fpout, "%s\n", utterance->line);
		} else {
			cnt++;
			for(i=0; utterance->line[i] != '\t' && utterance->line[i] != EOS; i++) ;
			if (utterance->line[i] == '\t') {
				for(; utterance->line[i] == '\t'; i++) ;
				strcpy(utterance->line, utterance->line+i);
			}
			i = 0;
			if (cnt == 1) {
				printout("*PAR:",utterance->line,NULL,NULL,TRUE);
			} else if (cnt == 2) {
				convert_PEP((unsigned char *)utterance->line, templineW);
				UnicodeToUTF8(templineW, strlen(templineW), (unsigned char *)templineC, NULL, UTTLINELEN);
				printout("%dp1:",templineC,NULL,NULL,TRUE);
			} else if (cnt == 3) {
				convert_PEP((unsigned char *)utterance->line, templineW);
				UnicodeToUTF8(templineW, strlen(templineW), (unsigned char *)templineC, NULL, UTTLINELEN);
				printout("%dp2:",templineC,NULL,NULL,TRUE);
			}
		}
		if (cnt == 3)
			cnt = 0;

	}
}
