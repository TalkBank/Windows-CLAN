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
extern char OverWriteFile;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		OverWriteFile = TRUE;
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
	printf("Usage: FILEEXCHANGE [%s] filename(s)\n",mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

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
}
