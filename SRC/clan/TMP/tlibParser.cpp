#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

#if defined(_MAC_CODE) && defined(PLUGINPROJ)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

extern struct tier *defheadtier;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
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

void usage() {
	printf("Usage: LibParse [%s] filename(s)\n",mainflgs());
	mainusage();
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

static FILE *openFileAndWrite(FILE *fp, char *fname, char *line) {
	if (fp == NULL) {
		fp = fopen(fname, "w");
		if (fp == NULL) {
			fprintf(stderr, "Can't open file %s\n", fname);
			cutt_exit(0);
		}
#ifdef _MAC_CODE
		settyp(fname, 'TEXT', the_file_creator.out);
#endif
	}
	fprintf(fp, "%s\n", uttline);
	return(fp);
}

void call() {
	char crIn = FALSE;
	int c;

	c = getc(fpin);
	while (!feof(fpin)) {
		if (isalnum(c)) {
			putc(c, fpout);
			crIn = FALSE;
		} else if (!crIn) {
			putc('\n', fpout);
			crIn = TRUE;
		}
		c = getc(fpin);
	}
}
