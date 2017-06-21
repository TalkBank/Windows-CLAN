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
	printf("Usage: CD Const [%s] filename(s)\n",mainflgs());
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

void call() {
	char oname[512], oartist[512], oalbum[512];
	char name[512], artist[512], album[512];
	int tn, ttn, ltn, lttn;
	int i, j;

	ltn = lttn = 0;
	oname[0] = EOS;
	oartist[0] = EOS;
	oalbum[0] = EOS;
	while (!feof(fpin)) {
		if (fgets_cr(templineC, UTTLINELEN-1, fpin) == NULL)
			break;
		i = 0;
		for (j=0; templineC[j] != '\t' && templineC[j] != '\n'; j++)
			name[i++] = templineC[j];
		name[i] = EOS;

		i = 0;
		for (j++; templineC[j] != '\t' && templineC[j] != '\n'; j++)
			artist[i++] = templineC[j];
		artist[i] = EOS;

		i = 0;
		for (j++; templineC[j] != '\t' && templineC[j] != '\n'; j++)
			album[i++] = templineC[j];
		album[i] = EOS;
		
		j++;
		tn = atoi(templineC+j);
		for (; templineC[j] != '\t' && templineC[j] != '\n'; j++) ;
		
		j++;
		ttn = atoi(templineC+j);

		if (!_stricmp(oartist, artist) && strcmp(oartist, artist)) {
			fprintf(fpout, "**%d\t%d\t%s\t%s\n", i, lttn, oartist, oalbum);
		} else if (!_stricmp(oalbum, album) && strcmp(oalbum, album)) {
			fprintf(fpout, "**%d\t%d\t%s\t%s\n", i, lttn, oartist, oalbum);
		}

		if ((_stricmp(oartist, artist) || _stricmp(oalbum, album)) && ltn != lttn) {
			for (i=ltn+1; i <= lttn; i++)
				fprintf(fpout, "%d\t%d\t%s\t%s\n", i, lttn, oartist, oalbum);
		}
		if (ttn != 0) {
			if (!_stricmp(oartist, artist) && !_stricmp(oalbum, album) && tn != ltn+1) {
				for (ltn++; ltn < tn; ltn++)
					fprintf(fpout, "%d\t%d\t%s\t%s\n", ltn, ttn, artist, album);
			} else if ((_stricmp(oartist, artist) || _stricmp(oalbum, album)) && tn > 1) {
				for (ltn=1; ltn < tn; ltn++)
					fprintf(fpout, "%d\t%d\t%s\t%s\n", ltn, ttn, artist, album);
			}
		}
		ltn  = tn;
		lttn = ttn;
		strcpy(oname, name);
		strcpy(oartist, artist);
		strcpy(oalbum, album);
	}
}
