/**********************************************************************
	"Copyright 1990-2023 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 3
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

static char isFTime;

void call() {
	int i;
	BOOL sq;

	sq = false;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "%wor:", FALSE, FALSE)) {
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[')
					sq = true;
				else if (utterance->line[i] == ']')
					sq = false;
				if (sq == false && utterance->line[i] == '&' &&
					(i == 0 || isSpace(utterance->line[i-1]) || utterance->line[i-1] == '\n' || utterance->line[i-1] == '<')) {
					i++;
					if (!isSpace(utterance->line[i]) && utterance->line[i] != '\n' && utterance->line[i] != '~' &&
						utterance->line[i] != '-' && utterance->line[i] != '+' && utterance->line[i] != '*' &&
						utterance->line[i] != '=' && utterance->line[i] != '{' && utterance->line[i] != '}') {
						att_shiftright(utterance->line+i, utterance->attLine+i, 1);
						utterance->line[i] = '~';
						utterance->attLine[i] = utterance->attLine[i+1];
					}
				}
				i++;
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine, FALSE);
	}
}

void init(char f) {
	if (f) {
		isFTime = TRUE;
		onlydata = 1;
		stout = FALSE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".cha";
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
//		root_list = NULL;
	} else {
		if (isFTime) {
			isFTime = FALSE;
		}
	}
}

void usage() {
	printf("ampersand convert &[a-z] to &~\n");
	printf("Usage: temp filename(s)\n");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
//	root_list = freeList(root_list);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

/*
#define MY_LIST struct my_lst
struct my_lst {
	char *word;
	struct my_lst *next_word;
} ;

static MY_LIST *root_list;

static MY_LIST *freeList(MY_LIST *p) {
	MY_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_word;
		if (t->word != NULL)
			free(t->word);
		free(t);
	}
	return(NULL);
}

static void IISRP_exit(int i) {
	root_list = freeList(root_list);
	cutt_exit(i);
}

static MY_LIST *addToCompList(MY_LIST *root, char *word) {
	MY_LIST *p;

	if (root == NULL) {
		if ((p=NEW(MY_LIST)) == NULL) {
			fprintf(stderr, "\nOut of memory.\n\n");
			IISRP_exit(0);
		}
		root = p;
	} else {
		for (p=root; p->next_word != NULL; p=p->next_word) ;
		p->next_word = NEW(MY_LIST);
		p = p->next_word;
	}
	if (p == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		IISRP_exit(0);
	}
	p->next_word = NULL;
	p->word = (char *)malloc(strlen(word)+1);
	if (p->word == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		IISRP_exit(0);
	}
	strcpy(p->word, word);
	return(root);
}

static char isWordComp(MY_LIST *p, char *word) {
	while (p != NULL) {
		if (uS.mStricmp(p->word, word) == 0)
			return(TRUE);
		p = p->next_word;
	}
	return(FALSE);
}
*/
