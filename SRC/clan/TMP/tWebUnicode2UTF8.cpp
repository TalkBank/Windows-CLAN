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
extern char AddCEXExtension;
extern char isRecursive;

static unCH output[BUFSIZ];
static char line[BUFSIZ], num[BUFSIZ];

void usage() {
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call() {
	int i, j, k, l;

	line[0] = '\0';
	while (!feof(fpin)) {
		fgets_cr(line, BUFSIZ, fpin);
		j = 0;
		for (i=0; line[i] != '\0'; i++) {
			if (line[i] == '&' && line[i+1] == '#') {
				k = 0;
				for (l=i+2; isdigit(line[l]); l++) {
					num[k++] = line[l];
				}
				num[k] = '\0';
				if (line[l] == ';') {
					i = l;
					output[j++] = atoi(num);
				} else
					output[j++] = line[i];
			} else {
				output[j++] = line[i];
			}
		}
		output[j] = '\0';
		u_strcpy(line, output, BUFSIZ);
		fputs(line, fpout);
	}
}
