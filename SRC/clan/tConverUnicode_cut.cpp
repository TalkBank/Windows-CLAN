/**********************************************************************
 "Copyright 1990-2016 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

// get infor from slave tab saparated Excell file and add it to master file

#define CHAT_MODE 0
#include "ced.h"
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

static char ftime;

void usage() {
	printf("Usage: temp [sF %s] filename(s)\n",mainflgs());
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		ftime = TRUE;
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".cut";
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

static void extractUCs(char *line, unsigned long *uc0, unsigned long *uc1) {
	char *end;

	if (*line == '"')
		line++;
	for (end=line; *end != ',' && *end != '\t' && *end != '\n' && *end != EOS; end++) ;
	sscanf(line, "%lx", uc0);
	if (*end == ',') {
		end++;
		sscanf(end, "%lx", uc1);
	}
}

void call(void) {
	int  i, j, o, ln;
	char st[256];
	unsigned long uc0;
	unsigned long uc1;
	extern short mScript;

	fprintf(fpout, "lXs Special Text file saves all fonts LxS\n");
	fprintf(fpout, "@Font:	Arial:16:7\n");
	fprintf(fpout, "@UTF8\n");
	ln = 0;
	while (fgets_cr(templineC, BUFSIZ, fpin) != NULL) {
		ln++;
		if (uS.partwcmp(templineC, "UniNum	")) {
			for (i=0; templineC[i] != EOS; i++) {
				templine[o++] = (unCH)templineC[i];
			}
			templine[o] = EOS;
			UnicodeToUTF8((unCH *)templine, o, (unsigned char *)templineC, NULL, UTTLINELEN);
			for (i=0; templineC[i] != EOS; i++) {
				putc(templineC[i], fpout);
			}
			continue;
		}
		if (uS.isUTF8(templineC)) {
			fprintf(stderr, "*** ERROR: File \"%s\" must not be in a Unicode-UTF8 format.\n", oldfname);
			cutt_exit(0);
		}
		if (templineC[0] != EOS && templineC[0] != '\n' && uS.mStrnicmp(templineC, "DEL", 3)) {
			for (i=0; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) {
				if (!isdigit(templineC[i]) && templineC[i] != 'A' && templineC[i] != 'B' && templineC[i] != 'C' &&
					templineC[i] != 'D' && templineC[i] != 'E' && templineC[i] != 'F' && templineC[i] != '"' && templineC[i] != ',') {
					fputs("Only numbers or letters A,B,C,D,E,F and ',' and '\"' are allowed in first column\n", stderr);
					fprintf(stderr,"line #%d:\t\"%s\"", ln, templineC);
					cutt_exit(0);
				}
			}
		}
		uc1 = 0xFFFF;
		if (!uS.mStrnicmp(templineC, "DEL", 3))
			uc0 = 0xFFFF;
		else {
			extractUCs(templineC, &uc0, &uc1);
		}
		o = 0;
		for (i=0; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) {
			templine[o++] = (unCH)templineC[i];
		}
		if (templineC[i] != EOS)
			templine[o++] = (unCH)templineC[i];
		i++;
		if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS) {
			if (templineC[i] == 'o')
				templine[o++] = 'o';
			if (uc0 != 0xFFFF)
				templine[o++] = (unCH)uc0;
			i++;
			if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS) {
				if (uc1 != 0xFFFF)
					templine[o++] = (unCH)uc1;
				i++;
			}
//			for (; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) {
//				templine[o++] = (unCH)uc1;
//			}
		}
		if (templineC[i] != EOS)
			templine[o++] = (unCH)templineC[i];
		if (templineC[i] != '\n') {
			for (i++; templineC[i] != EOS; i++) {
				if (templineC[i] == '\n' || templineC[i] == '\t' || templineC[i] == '\r' || templineC[i] == ' ') {
					templine[o++] = (unCH)templineC[i];
				} else if ((unsigned char)templineC[i] > (unsigned char)0x20 && (unsigned char)templineC[i] < (unsigned char)0x7F) {
					templine[o++] = (unCH)templineC[i];
				} else {
					sprintf(st, "0x%x", (unsigned char)templineC[i]);
					for (j=0; st[j] != EOS; j++) {
						templine[o++] = st[j];
					}
/*
					if (st[0] =='0' && st[1] == 'x')
						sscanf(st+2, "%lx", &uc0);
					else
						sscanf(st, "%lx", &uc0);
					if (uc0 >= 0x7F && uc0 <= 0xA0) {
						res = ANSIToUnicode((unsigned char *)templineC+i, 1, wbuf, NULL, 4, mScript);
						if (!res) {
							templine[o++] = ' ';
							templine[o++] = '(';
							templine[o++] = ' ';
							templine[o++] = wbuf[0];
							if (wbuf[0] != uc0)
								templine[o-1] = uc0;
							templine[o++] = ' ';
							templine[o++] = ')';
						}
					}
*/
				}
			}
		}
		templine[o] = EOS;
		UnicodeToUTF8((unCH *)templine, o, (unsigned char *)templineC, NULL, UTTLINELEN);
		for (i=0; templineC[i] != EOS; i++) {
			putc(templineC[i], fpout);
		}
	}
}
