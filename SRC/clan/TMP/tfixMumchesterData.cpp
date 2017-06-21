/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


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
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

static char isFirstFOpen;
static char orgPath[1024], oCurc;
static char oLine[UTTLINELEN+1];
static long oLineno;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

static void mkOrgPath(char *curPath, char *orgPath) {
	int i, t;

	strcpy(orgPath, wd_st_full);
	i = strlen(orgPath) - 1;
	if (i <= 0) {
		strcat(orgPath, "-old");
		return;
	}
	if (orgPath[i] == PATHDELIMCHR)
		i--;
	for (t=i; t >= 0 && orgPath[t] != PATHDELIMCHR && orgPath[t] != '-'; t--) ;
	if (orgPath[t] == '-')
		i = t;
	else
		i++;
	strcpy(orgPath+i, "-old:");
}

void init(char f) {
	if (f) {
		isFirstFOpen = TRUE;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		mkOrgPath(wd_st_full, orgPath);
	}
}

void usage() {
	printf("changes word-0ed [*] to word [* word-0ed]\n");
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

static char getNewOrgLine(FILE *oFp) {
	int i;
	char isCRFound;

	oLine[0] = EOS;
	if (feof(oFp))
		return(FALSE);
	if (oCurc != '*') {
		do {
			if (oCurc == '\n') {
				oLineno++;
				isCRFound = TRUE;
			} else
				isCRFound = FALSE;
			oCurc = (char)getc_cr(oFp, NULL);
			if (oCurc == '*' && isCRFound)
				break;
			if (feof(oFp))
				return(FALSE);
		} while (1) ;
		if (oCurc == '\n') {
			oLineno++;
			isCRFound = TRUE;
		} else
			isCRFound = FALSE;
	}
	if (oCurc == '*') {
		i = 0;
		do {
			if (oCurc == '\n') {
				oLineno++;
				isCRFound = TRUE;
			} else
				isCRFound = FALSE;
			oLine[i++] = oCurc;
			oCurc = (char)getc_cr(oFp, NULL);
			if (isSpeaker(oCurc, FALSE) && isCRFound)
				break;
			if (feof(oFp))
				break;
		} while (1) ;
		oLine[i] = EOS;
	}
	return(TRUE);
}

static char matched(long *olIndex) {
	long i, j, oi, oj;

	oi = *olIndex;
	for (i=strlen(utterance->line)-1; i > 0; i--) {
		if (utterance->line[i] == '[' && utterance->line[i+1] == '*' && utterance->line[i+2] == ']') {
			for (j=i; j > 0 && uS.isskip(utterance->line,j,&dFnt,MBF); j--) ;
			oj = oi - 1;
			if (oj >= 0 && j >= 0 && oLine[oj] == utterance->line[j]) {
				for (; oj >= 0 && j >= 0; oj--, j--) {
					if (uS.isskip(utterance->line,j,&dFnt,MBF) && uS.isskip(oLine,oj,&dFnt,MBF)) {
						for (j=0L, oj=oi+1; oLine[oj] != EOS && !uS.isskip(oLine,oj,&dFnt,MBF); oj++, j++) {
							templineC[j] = oLine[oj];
						}
						templineC[j] = EOS;
						i = i + 2;
						att_shiftright(utterance->line+i, utterance->attLine+i, j+1);
						utterance->line[i] = ' ';
						utterance->attLine[i] = 0;
						for (j=0L, i++; templineC[j] != EOS; j++, i++) {
							utterance->line[i] = templineC[j];
							utterance->attLine[i] = 0;
						}
						*olIndex = oi;
						return(TRUE);
					}
					if (uS.isskip(utterance->line,j,&dFnt,MBF) || uS.isskip(oLine,oj,&dFnt,MBF)) {
						i = j + 1;
						break;
					}
					if (oLine[oj] != utterance->line[j]) {
						i = j + 1;
						break;
					}
				}
				if ((oj < 0 && uS.isskip(utterance->line,j,&dFnt,MBF)) || (j < 0 && uS.isskip(oLine,oj,&dFnt,MBF)) || (oj < 0 && j < 0)) {
					for (j=0L, oj=oi+1; oLine[oj] != EOS && !uS.isskip(oLine,oj,&dFnt,MBF); oj++, j++) {
						templineC[j] = oLine[oj];
					}
					templineC[j] = EOS;
					i = i + 2;
					att_shiftright(utterance->line+i, utterance->attLine+i, j+1);
					utterance->line[i] = ' ';
					utterance->attLine[i] = 0;
					for (j=0L, i++; templineC[j] != EOS; j++, i++) {
						utterance->line[i] = templineC[j];
						utterance->attLine[i] = 0;
					}
					*olIndex = oi;
					return(TRUE);
				}
			}
		}
	}
	*olIndex = oi;
	return(FALSE);
}

static char findErr(long *olIndex) {
	char sq;
	long oi;

	sq = FALSE;
	for (oi=*olIndex-1; oi > 0; oi--) {
		if (uS.isRightChar(oLine,oi,']',&dFnt,MBF)) sq = TRUE;
		else if (uS.isRightChar(oLine,oi,'[',&dFnt,MBF)) sq = FALSE;
		if (!sq && oLine[oi] == '-' && oLine[oi+1] == '0') {
			*olIndex = oi;
			return(TRUE);
		}
	}
	*olIndex = oi;
	return(FALSE);
}

void call() {
	long olIndex;
	char pathIndex, isFoundErr;
	FILE *oFp;

	pathIndex = strlen(orgPath);
	strcat(orgPath, oldfname);
	oFp = fopen(orgPath, "r");
	if (oFp == NULL) {
		if (isFirstFOpen) {
			char *s;

			s = strrchr(orgPath, PATHDELIMCHR);
			if (s == NULL)
				fprintf(stderr, "Can't open original/old file: %s\n", orgPath);
			else {
				*s = EOS;
				s = strrchr(orgPath, PATHDELIMCHR);
				if (s == NULL)
					s = orgPath;
				else
					s++;
				fprintf(stderr, "\n**** Can't open original file: %s\n", oldfname);
				fprintf(stderr, "**** Make sure to call original directory: %s\n", s);
			}
		} else
			fprintf(stderr, "\n**** Can't open original file: %s\n\n", orgPath);
		cutt_exit(0);
	}
	
	isFirstFOpen = FALSE;
	oLineno = 0L;
	oCurc = 0;
	do {
		if (!getNewOrgLine(oFp))
			break;
		olIndex = strlen(oLine);
		isFoundErr = findErr(&olIndex);
	} while (!isFoundErr) ;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			while (isFoundErr) {
				if (matched(&olIndex))
					isFoundErr = findErr(&olIndex);
				else
					break;
			}
			while (!isFoundErr) {
				if (!getNewOrgLine(oFp))
					break;
				olIndex = strlen(oLine);
				isFoundErr = findErr(&olIndex);
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
	while (!isFoundErr) {
		if (!getNewOrgLine(oFp))
			break;
		olIndex = strlen(oLine);
		isFoundErr = findErr(&olIndex);
	}
	if (oFp != NULL)
		fclose(oFp);
	if (isFoundErr) {
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", orgPath, oLineno-1);
		fprintf(stderr, "Can't find match for line:\n%s\n", oLine);
		oLineno = oLineno - 25;
		if (oLineno < 0L)
			oLineno = 1;
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, oLineno);
		cutt_exit(0);
	}

	orgPath[pathIndex] = EOS;
}
