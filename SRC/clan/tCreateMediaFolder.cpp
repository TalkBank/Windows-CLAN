/**********************************************************************
 "Copyright 1990-2015 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

// get infor from slave tab saparated Excell file and add it to master file

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#define mkdir my_mkdir
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#if defined(UNX) || defined(_MAC_CODE)
#define MODE S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#endif
#ifdef _WIN32
#define MODE 0
#endif

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char isRecursive;

void usage() {
	printf("Usage: temp [%s] *.mov\n", mainflgs());
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		isRecursive = TRUE;
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
	} else {
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

static void remPathsChars(char *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == PATHDELIMCHR)) i--;
	if (st[i+1] != EOS)
		st[i+1] = EOS;
}

void call(void) {
	char *s, *b, *e, t;
	FILE *fp;

	extractPath(templineC, oldfname);
	extractFileName(FileName1, oldfname);
	strcpy(DirPathName, wd_dir);
	remPathsChars(DirPathName);
	s = strrchr(DirPathName, PATHDELIMCHR);
	if (s != NULL)
		*s = EOS;
	addFilename2Path(DirPathName, "media");
	mkdir(DirPathName, MODE);
	strcpy(templineC, templineC+strlen(wd_dir));
	if (templineC[0] == PATHDELIMCHR)
		strcpy(templineC, templineC+1);
	remPathsChars(templineC);
	b = templineC;
	while (*b != EOS) {
		for (e=b; *e != EOS && *e != PATHDELIMCHR; e++) ;
		t = *e;
		*e = EOS;
		if (uS.mStricmp(b, "media") == 0) {
			break;
		}
		addFilename2Path(DirPathName, b);
		mkdir(DirPathName, MODE);
		if (t == PATHDELIMCHR) {
			*e = t;
			b = e + 1;
		} else
			break;
	}
	addFilename2Path(DirPathName, FileName1);
	fp = fopen(DirPathName, "w");
	if (fp != NULL)
		fclose(fp);
}
