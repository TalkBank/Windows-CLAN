/**********************************************************************
	"Copyright 1990-2016 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 1
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

extern struct tier *defheadtier;
extern char OverWriteFile;

#define COMPS_LIST struct com_words_lst
struct com_words_lst {
	char *word;
	struct com_words_lst *next_word;
} ;

static char isFirstFile;
static COMPS_LIST *root_compounds;

static COMPS_LIST *freeCompList(COMPS_LIST *p) {
	COMPS_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_word;
		if (t->word != NULL)
			free(t->word);
		free(t);
	}
	return(NULL);
}

static void temp_exit(int i) {
	root_compounds = freeCompList(root_compounds);
	cutt_exit(i);
}

static COMPS_LIST *addToCompList(COMPS_LIST *root, char *word) {
	COMPS_LIST *p;

	if (root == NULL) {
		if ((p=NEW(COMPS_LIST)) == NULL) {
			fprintf(stderr, "\nOut of memory.\n\n");
			temp_exit(0);
		}
		root = p;
	} else {
		for (p=root; p->next_word != NULL; p=p->next_word) ;
		p->next_word = NEW(COMPS_LIST);
		p = p->next_word;
	}
	if (p == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		temp_exit(0);
	}
	p->next_word = NULL;
	p->word = (char *)malloc(strlen(word)+1);
	if (p->word == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		temp_exit(0);
	}
	strcpy(p->word, word);
	return(root);
}

void init(char f) {
	if (f) {
		isFirstFile = TRUE;
		onlydata = 1;
		stout = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		root_compounds = NULL;
	} else {
		if (!isFirstFile) {
			chatmode = 0;
			stout = FALSE;
		}
	}
}

void usage() {
	printf("Reads compound words CHAT file and compares it to lex.cut files\n");
	printf("Usage: temp chat_file.cha filename(s).cut\n");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	root_compounds = freeCompList(root_compounds);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char isWordComp(COMPS_LIST *p, char *word) {
	while (p != NULL) {
		if (uS.mStricmp(p->word, word) == 0)
			return(TRUE);
		p = p->next_word;
	}
	return(FALSE);
}

void call() {
	char *c, isRemoveWord;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (isFirstFile) {
			if (utterance->speaker[0] == '*') {
				c = strchr(uttline, ' ');
				if (c == NULL) {
					c = strchr(uttline, '\t');
				}
				if (c != NULL) {
					*c = EOS;
					strcpy(spareTier1, uttline);
				} else
					strcpy(spareTier1, uttline);
				uS.remFrontAndBackBlanks(spareTier1);
			} else if (utterance->speaker[0] == '%') {
				if (strchr(uttline, '+') != NULL) {
					root_compounds = addToCompList(root_compounds, spareTier1);
				}
			}
		} else {
			isRemoveWord = FALSE;
			strcpy(spareTier1, utterance->line);
			c = strchr(spareTier1, ' ');
			if (c == NULL) {
				c = strchr(spareTier1, '\t');
			}
			if (c != NULL) {
				*c = EOS;
				isRemoveWord = isWordComp(root_compounds, spareTier1);
			}
			if (!isRemoveWord)
				fprintf(fpout, "%s", utterance->line);
		}
	}
	isFirstFile = FALSE;

}
