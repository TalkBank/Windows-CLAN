#define CHAT_MODE 0

#include "cu.h"
#ifdef _WIN32
	#include "TextUtils.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#define CHAT_MODE 0
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;

static char ca[UTTLINELEN];
static char cv[UTTLINELEN];
static char ma[UTTLINELEN];
static char mv[UTTLINELEN];

void usage() {
	puts("tMcCune converts McCune text file into CHAT file");
	printf("Usage: temp [%s] filename(s)\n",mainflgs());
	mainusage();
}

void init(char s) {
	if (s) {
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 3;
	}
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

static char isAllNumbers(char *line) {
	long i;
	char isNumFound, isSMColFound, isOtherFound;
	isNumFound = false;
	isSMColFound = false;
	isOtherFound = false;
	for (i=0L; line[i] != EOS; i++) {
		if (line[i] == '\n' || line[i] == '\r' || isSpace(line[i]) || line[i] == EOS)
			;
		else if (isdigit(line[i]))
			isNumFound = true;
		else if (line[i] == ';')
			isSMColFound = true;
		else
			isOtherFound = true;
	}
	if (isNumFound && isSMColFound && !isOtherFound)
		return(TRUE);
	else
		return(FALSE);
}

static char mccune_isEmptyLine(char *line) {
	long i;
	
	for (i=0L; line[i] != EOS; i++) {
		if (line[i] != '\n' && line[i] != '\r' && !isSpace(line[i]) && line[i] != EOS)
			return(FALSE);
	}
	return(TRUE);
}

static void pr_idfld(void) {
	if (*oldfname == EOS)
		fprintf(stderr,"*** File \"%s\": ", "Stdin");
	else
		fprintf(stderr,"*** File \"%s\": ", oldfname);
	fprintf(stderr,"line %ld. ", lineno/2);
	putc('\n',stderr);
}

static char getca(char *ct, char *line) {
	long i;
	
	ct[0] = EOS;
	for (i=0L; line[i] != EOS && line[i] != ';'; i++)
		ct[i] = line[i];
	if (line[i] == EOS) {
		pr_idfld();
		fprintf(stderr, "ERROR: 1\n");
		return(FALSE);
	}
	ct[i] = EOS;
	uS.remblanks(ct);
	for (i=0L; isSpace(ct[i]); i++) ;
	if (i > 0L)
		strcpy(ct, ct+i);
	return(TRUE);
}

static char getcv(char *ct, char *line) {
	long i, j;

	ct[0] = EOS;
	for (i=0L; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 2\n");
		return(FALSE);
	}
	for (i++, j=0L; line[i] != EOS && line[i] != ';'; i++, j++)
		ct[j] = line[i];
	ct[j] = EOS;
	uS.remblanks(ct);
	for (i=0L; isSpace(ct[i]); i++) ;
	if (i > 0L)
		strcpy(ct, ct+i);
	return(TRUE);
}

static char getma(char *ct, char *line) {
	long i, j;

	ct[0] = EOS;
	for (i=0L; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 4\n");
		return(FALSE);
	}
	for (i++; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 5\n");
		return(FALSE);
	}
	for (i++, j=0L; line[i] != EOS && line[i] != ';'; i++, j++)
		ct[j] = line[i];
	ct[j] = EOS;
	uS.remblanks(ct);
	for (i=0L; isSpace(ct[i]); i++) ;
	if (i > 0L)
		strcpy(ct, ct+i);
	return(TRUE);
}

static char getmv(char *ct, char *line) {
	long i, j;

	ct[0] = EOS;
	for (i=0L; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 7\n");
		return(FALSE);
	}
	for (i++; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 8\n");
		return(FALSE);
	}
	for (i++; line[i] != EOS && line[i] != ';'; i++) ;
	if (line[i] == EOS) {
//		pr_idfld();
//		fprintf(stderr, "ERROR: 9\n");
		return(FALSE);
	}
	for (i++, j=0L; line[i] != EOS && line[i] != ';'; i++, j++)
		ct[j] = line[i];
	ct[j] = EOS;
	uS.remblanks(ct);
	for (i=0L; isSpace(ct[i]); i++) ;
	if (i > 0L)
		strcpy(ct, ct+i);
	return(TRUE);
}

static char processLine(char *line) {
	if (mccune_isEmptyLine(line))
		;
	else if (isAllNumbers(line))
		printout("@New Episode", NULL, NULL, NULL, TRUE);
	else {
		uS.lowercasestr(line, dFnt.Encod, MBF);
		if (!getca(ca, line))
			;
//			return(FALSE);
		if (!getcv(cv, line))
			;
//			return(FALSE);
		if (!getma(ma, line))
			;
//			return(FALSE);
		if (!getmv(mv, line))
			;
//			return(FALSE);
		if (*cv != EOS) {
			if (*ma != EOS || *mv != EOS) {
				strcpy(templineC, "<");
				strcat(templineC, cv);
				strcat(templineC, "> [>]");
				printout("*CHI:", templineC, NULL, NULL, TRUE);
			} else {
				printout("*CHI:", cv, NULL, NULL, TRUE);
			}
			if (*ca != EOS) {
				printout("%act:", ca, NULL, NULL, TRUE);
			}
		} else if (*ca != EOS) {
			if (*ma != EOS || *mv != EOS) {
				strcpy(templineC, "<0.> [>]");
				printout("*CHI:", templineC, NULL, NULL, TRUE);
			} else {
				strcpy(templineC, "0.");
				printout("*CHI:", templineC, NULL, NULL, TRUE);
			}
			printout("%act:", ca, NULL, NULL, TRUE);
		}

		if (*mv != EOS) {
			if (*ca != EOS || *cv != EOS) {
				strcpy(templineC, "<");
				strcat(templineC, mv);
				strcat(templineC, "> [<]");
				printout("*MOT:", templineC, NULL, NULL, TRUE);
			} else {
				printout("*MOT:", mv, NULL, NULL, TRUE);
			}
			if (*ma != EOS) {
				printout("%act:", ma, NULL, NULL, TRUE);
			}
		} else if (*ma != EOS) {
			if (*ca != EOS || *cv != EOS) {
				strcpy(templineC, "<0.> [<]");
				printout("*MOT:", templineC, NULL, NULL, TRUE);
			} else {
				strcpy(templineC, "0.");
				printout("*MOT:", templineC, NULL, NULL, TRUE);
			}
			printout("%act:", ma, NULL, NULL, TRUE);
		}
	}
	return(TRUE);
}

void call() {
	lineno = 1L;
	fprintf(fpout, "@Font:	Monaco:9:0\n");
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Participants:\tCHI Child, MOT Mother\n");
	*uttline = EOS;
	if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
		return;
	lineno++;
	fprintf(fpout, "@Header:\t%s", utterance->line);
	if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
		return;
	lineno++;
	do {
		if (!processLine(utterance->line))
			break;
		fgets_cr(utterance->line, UTTLINELEN, fpin);
		lineno++;
	} while (!feof(fpin)) ;
	fprintf(fpout, "@End\n");
}
