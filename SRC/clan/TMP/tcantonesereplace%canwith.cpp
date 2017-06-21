#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 3

extern struct tier *defheadtier;

void init(char f) {
	if (f) {
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage()			/* print proper usage and exit */
{
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}

/* temp_parsfname(old,fname,fext) take the file name in "old" removes, if it 
   exists, an extention from the end of it. Then it removes the full path, if 
   it exists. Then it copies the remainder to the "fname" and concatanate the 
   extention "ext" to the end of string in "fname".
*/
static void temp_parsfname(char *old, char *fname, char *fext) {
    register int i;
    register int ti;

    i = strlen(old) - 1;
    while (i >= 0 && isSpace(old[i]))
    	i--;
    old[i+1] = '\0';
    i = strlen(old);
    while (i > 0 && old[i] != '.' && old[i] != PATHDELIMCHR)
    	i--;
    ti = 0;

    if (old[i] == PATHDELIMCHR)
    	i = strlen(old);
    else if (old[i] == '.') {
		if (old[i-1] == PATHDELIMCHR)
			i = strlen(old);
    } else {
        ti = 0;
        if (old[i] != '.')
        	i = strlen(old);
    }

    while (ti < i)
    	*fname++ = old[ti++];
    strcpy(fname,fext);
}

void call() {
	long macLineno, tagLineno;
	char tagCurrentchar, macCurrentchar;
	char tagCurrentATT, macCurrentATT;
	char DoneTag = FALSE, DoneMac = FALSE;
	char new_name[FILENAME_MAX];
	FILE *fptag, *tfpin;

	temp_parsfname(oldfname,new_name,".tag");
	if ((fptag=fopen(new_name, "r")) == NULL) {
		fprintf(stderr,"Can't open file %s.\n", new_name);
		cutt_exit(0);
	}
	macLineno = tagLineno = lineno;

    tfpin = fpin;
    lineno = macLineno;
    currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);
    do {
    	DoneMac = !getwholeutter();
    	if (!DoneMac && !partcmp(utterance->speaker,"%can:",FALSE))
    		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber,FALSE);
    	else
    		break;
    } while (!DoneMac);
    macCurrentchar = currentchar;
    macCurrentATT = currentatt;
    macLineno = lineno;

    fpin = fptag;
    lineno = tagLineno;
    currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);
    do {
	    DoneTag = !getwholeutter();
    	if (!DoneTag && partcmp(utterance->speaker,"*",FALSE)) {
    		printout("%can:",utterance->line,NULL,utterance->attLine,utterance->tierNumber,TRUE);
    		break;
    	}
    } while (!DoneTag);
    tagCurrentchar = currentchar;
    tagCurrentATT = currentatt;
    tagLineno = lineno;

	while (!DoneTag || !DoneMac) {
		if (!DoneMac) {
		    fpin = tfpin;
			lineno = macLineno;
			currentatt = macCurrentATT;
		    currentchar = macCurrentchar;
		    do {
		    	DoneMac = !getwholeutter();
		    	if (!DoneMac && !partcmp(utterance->speaker,"%can:",FALSE))
		    		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber,FALSE);
		    	else
		    		break;
		    } while (!DoneMac);
		    macCurrentchar = currentchar;
		    macCurrentchar = currentchar;
		    macLineno = lineno;
		}

		if (!DoneTag) {
		    fpin = fptag;
			lineno = tagLineno;
		    currentchar = tagCurrentchar;
		    currentatt = tagCurrentATT;
		    do {
			    DoneTag = !getwholeutter();
		    	if (!DoneTag && partcmp(utterance->speaker,"*",FALSE)) {
		    		printout("%can:",utterance->line,NULL,utterance->attLine,utterance->tierNumber,TRUE);
		    		break;
		    	}
		    } while (!DoneTag);
		    tagCurrentchar = currentchar;
		    tagCurrentATT = currentatt;
			tagLineno = lineno;
		}
	}
	fclose(fptag);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
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
