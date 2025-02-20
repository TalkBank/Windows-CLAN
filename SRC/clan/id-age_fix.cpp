/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

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

extern struct tier *headtier;
extern char OverWriteFile;

void init(char f) {

    if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
    }
}

void usage()			/* print proper usage and exit */
{
	puts("TEMP copy @ID header from orig file.");
    printf("Usage: temp [%s] filename(s)\n",mainflgs());
    mainusage(TRUE);
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

static char readIDFromOrig(FNType* oldfname) {
	int  len;
	char isIDsFound;
	FNType origFname[FNSize];
	FILE *fp;

	strcpy(origFname, wd_dir);
	len = strlen(origFname);
	if (origFname[len-1] == '/')
		origFname[len-1] = EOS;
	strcat(origFname, "-orig");
	strcat(origFname, oldfname+len);
	
	if ((fp=fopen(origFname,"r")) == NULL) {
		fprintf(stderr, "***ERROR: Can't open orig file:\n\t%s\n", origFname);
		return(FALSE);
	}
	fprintf(stderr, "Reading orig file: %s\n", origFname);
	isIDsFound = FALSE;
	while (fgets_cr(spareTier1, UTTLINELEN, fp)) {
		if (uS.partcmp(spareTier1,"@ID:",FALSE,FALSE)) {
			fprintf(fpout, "%s", spareTier1);
			isIDsFound = TRUE;
		} else if (isIDsFound == TRUE)
			break;
	}
	fclose(fp);
	if (isIDsFound == FALSE) {
		fprintf(stderr, "***ERROR: Can't find any IDs in orig file\n");
		return(FALSE);
	} else
		return(TRUE);
}

void call() {   
	char isReadOrigIDs, isWroteNewIDs;

	isWroteNewIDs = TRUE;
	isReadOrigIDs = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (isReadOrigIDs == FALSE) {
				isReadOrigIDs = TRUE;
				if (readIDFromOrig(oldfname) == FALSE)
					isWroteNewIDs = FALSE;
			}
			if (isWroteNewIDs == FALSE)
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
}
