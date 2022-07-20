/**********************************************************************
 "Copyright 1990-2016 Brian MacWhinney. Use is subject to Gnu Public License
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
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

void usage() {
	printf("Usage: temp [%s] filename(s)\n",mainflgs());
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

static void parseLine(int ln) {
	int  len;
	char *begS, *endS, *sp, *line;
	long  b, e;
	float beg, end;

	begS = templineC1;
	if (!isdigit(begS[0])) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected beginning time number, but found: %s.\n", begS);
		cutt_exit(0);
	}
	for (endS=begS; isdigit(*endS) || *endS == '.'; endS++) ;
	if (*endS == EOS) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected ending time, speaker name and speaker tier.\n");
		cutt_exit(0);
	}
	*endS = EOS;
	for (endS++; isSpace(*endS); endS++) ;
	if (*endS == EOS) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected ending time, speaker name and speaker tier.\n");
		cutt_exit(0);
	}
	if (!isdigit(endS[0])) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected ending time number, but found: %s.\n", endS);
		cutt_exit(0);
	}
	for (sp=endS; isdigit(*sp) || *sp == '.'; sp++) ;
	if (*sp == EOS) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected speaker name and speaker tier.\n");
		cutt_exit(0);
	}
	*sp = EOS;
	for (sp++; isSpace(*sp); sp++) ;
	if (*sp == EOS) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected speaker name and speaker tier.\n");
		cutt_exit(0);
	}
	for (line=sp; *line != EOS && !isSpace(*line); line++) ;
	if (*line == EOS) {
		fprintf(stderr,"*** File \"%s\": line %d.\n", oldfname, ln);
		fprintf(stderr, "Expected speaker tier.\n");
		cutt_exit(0);
	}
	*line = EOS;
	for (line++; isSpace(*line); line++) ;
	beg = atof(begS);
	beg = beg * 1000.000;
	b = (long)beg;
	end = atof(endS);
	end = end * 1000.000;
	e = (long)end;
	if (sp[0] != '*') {
		strcpy(spareTier1, "*");
		strcat(spareTier1, sp);
	} else
		strcpy(spareTier1, sp);
	uS.uppercasestr(spareTier1, &dFnt, MBF);
	strcpy(spareTier2, line);
	len = strlen(spareTier2);
	sprintf(spareTier2+len, " %c%ld_%ld%c", HIDEN_C, b, e, HIDEN_C);
	printout(spareTier1,spareTier2,NULL,NULL,TRUE);
}

void call(void) {
	int  ln;
	char *s, lang[BUFSIZ], part[BUFSIZ];

	ln = 0;
	fprintf(fpout, "@UTF8\n");
	fprintf(fpout, "@Begin\n");
	if (oldfname[0] == 'e')
		strcpy(lang, "eng");
	else if (oldfname[0] == 's')
		strcpy(lang, "spa");
	else if (oldfname[0] == 'j')
		strcpy(lang, "jpn");
	else if (oldfname[0] == 'a')
		strcpy(lang, "ara");
	else if (oldfname[0] == 'g')
		strcpy(lang, "deu");
	else if (oldfname[0] == 'm')
		strcpy(lang, "zho");
	else
		strcpy(lang, "missing");
	printout("@Languages:",lang,NULL,NULL,FALSE);
	strcpy(part, "A Subject, B Subject");
	printout("@Participants:",part,NULL,NULL,FALSE);
	sprintf(spareTier1, "%s|CallHome|A|||||Subject|||", lang);
	printout("@ID:",spareTier1,NULL,NULL,FALSE);
	sprintf(spareTier1, "%s|CallHome|B|||||Subject|||", lang);
	printout("@ID:",spareTier1,NULL,NULL,FALSE);
	strcpy(spareTier2, oldfname);
	s = strchr(spareTier2, '.');
	if (s != NULL)
		*s = EOS;
	sprintf(spareTier1, "%s, audio", spareTier2);
	printout("@Media:",spareTier1,NULL,NULL,FALSE);
	templineC1[0] = EOS;
	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC,FONTHEADER,FALSE,FALSE))
			continue;
		ln++;
		uS.remblanks(templineC);
		if (*templineC == '#') {
		} else if (*templineC != EOS) {
			if (isdigit(*templineC)) {
				if (templineC1[0] != EOS)
					parseLine(ln);
				strcpy(templineC1, templineC);
			} else {
				strcat(templineC1, " ");
				strcat(templineC1, templineC);
			}
		}
	}
	if (templineC1[0] != EOS)
		parseLine(ln);
	fprintf(fpout, "@End\n");
}
