/**********************************************************************
	"Copyright 1990-2012 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

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

#define WLEN 256

extern char OverWriteFile;
extern char *AddCEXExtension;

struct list {
	char word[WLEN];
	struct list *next;
};

static char isFirst;
static struct list *fc_root;

void usage() {
	printf("Usage: temp <Master filename> <fl filename>\n");
	mainusage(TRUE);
}

void init(char s) {
	if (s) {
		isFirst = TRUE;
		fc_root = NULL;
		stout = TRUE;
		onlydata = 3;
		OverWriteFile = TRUE;
	}
	if (isFirst) {
		stout = TRUE;
	} else {
		stout = FALSE;
	}
}

static void free_fc_root(struct list *p) {
	struct list *t;
	
	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	free_fc_root(fc_root);
	fc_root = NULL;
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f,f1,i);
			break;
	}
}

static void add2list(char *w) {
	struct list *p;

	if (fc_root == NULL) {
		p = NEW(struct list);
		if (p == NULL)
			out_of_mem();
		fc_root = p;
	} else {
		for (p=fc_root; 1; p=p->next) {
			if (strcmp(p->word,w) == 0)
				return;
			if (p->next == NULL) {
				p->next = NEW(struct list);
				if (p->next == NULL)
					out_of_mem();
				p = p->next;
				break;
			}
		}
	}
	strcpy(p->word,w);
	p->next = NULL;
}

static char foundInList(char *w) {
	struct list *p;

	for (p=fc_root; p != NULL; p=p->next) {
		if (strcmp(p->word,w) == 0)
			return(TRUE);
	}
	return(FALSE);
}

void call() {		/* this function is self-explanatory */
	char word[WLEN];
	char *e, t;
	FILE *fp = NULL;

	word[0] = EOS;
	while (fgets_cr(uttline, UTTLINELEN, fpin) != NULL) {
		if (uS.partwcmp(uttline, UTF8HEADER) || uttline[0] == '#' || uttline[0] == '%') {
			continue;
		}
		for (e=uttline; !isSpace(*e) && *e != '\n' && *e != EOS; e++) ;
		t = *e;
		*e = EOS;
		strcpy(word, uttline);
		*e = t;
		if (isFirst) {
			if (word[0] != EOS)
				add2list(word);
		} else {
			if (foundInList(word)) {
				if (fp == NULL) {
					AddCEXExtension = ".removed.cex";
					parsfname(oldfname, FileName1, "");
					if ((fp=openwfile(oldfname, FileName1, NULL)) == NULL) {
						fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
						fp = stderr;
					}
				}
				if (fp != NULL) {
					fprintf(fp, "%s", uttline);
				}
			} else {
				fprintf(fpout, "%s", uttline);
			}
		}
	}
	if (!isFirst) {
		free_fc_root(fc_root);
		fc_root = NULL;
	}
	if (isFirst)
		isFirst = FALSE;
	else {
		if (fp != NULL) {
			if (fp != stderr)
				fclose(fp);
			fprintf(stderr, "Some words were removed. Please look for file ending with \".removed.cex\"\n");
		} else
			fprintf(stderr, "\n\t No words were removed\n\n");		
		isFirst = TRUE;
	}
}
