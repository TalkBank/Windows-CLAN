/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"
/* // NO QT
#ifdef _WIN32
	#include <TextUtils.h>
#endif
*/

#if !defined(UNX)
#define _main tang2chat_main
#define call tang2chat_call
#define getflag tang2chat_getflag
#define init tang2chat_init
#define usage tang2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;

void usage() {
	puts("TANG2CHAT converts plain text file into CHAT file");
	printf("Usage: tang2chat [%s] filename(s)\n",mainflgs());
	mainusage(TRUE);
}

void init(char s) {
	if (s) {
		stout = FALSE;
		onlydata = 3;
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TANG2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
#if !defined(_CLAN_DEBUG)
	fprintf(stderr, "This command is not fully implemented yet.\n\n");
	return;
#endif
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++)
	{
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
/*
static void lowercaseTier(char *line) {
	if (isupper((unsigned char)*line) && (*line != 'I' || isalnum(*(line+1))))
		*line = (char)tolower((unsigned char)*line);
	for (; *line != EOS; line++) {
		if (UTF8_IS_LEAD((unsigned char)*line) && *line == (char)0xE2) {
			if (*(line+1) == (char)0x80 && *(line+2) == (char)0x9C) {
				if (isupper((unsigned char)*(line+3)))
					*(line+3) = (char)tolower((unsigned char)*(line+3));
			}
		}
	}
}

 static void addUttDel(char *line) {
	int i;

	i = strlen(line) - 1;
	for (; i >= 0; i--) {
		if (line[i] == '.' || line[i] == '!' || line[i] == '?')
			break;
	}
	if (i < 0)
		strcat(line, " .");
}
*/
static void printTier(char *line) {
	int i, j;

	if (line[0] != EOS) {
		if (utterance->speaker[0] == 'S' || utterance->speaker[0] == 's') {
			strcpy(utterance->speaker, "*PAR:");
		} else if (utterance->speaker[0] != '*') {
			strcpy(utterance->speaker, "*INV:");
		}
		j = 0;
		i = 0;
		while (line[i] != EOS) {
			if (line[i] == '{') {
				utterance->line[j++] = '&';
				utterance->line[j++] = '=';
				for (i++; line[i] != '}' && line[i] != EOS; i++)
					utterance->line[j++] = line[i];
				if (line[i] == '}')
					i++;
			} else if (line[i] == '#') {
				utterance->line[j++] = '(';
				utterance->line[j++] = '.';
				utterance->line[j++] = ')';
				i++;
			} else if (line[i] == '=') {
				utterance->line[j++] = ' ';
				utterance->line[j++] = '[';
				utterance->line[j++] = '/';
				utterance->line[j++] = ']';
				i++;
			} else if ((line[i] == '(' && line[i+1] == '(') || (line[i] == ')' && line[i+1] == ')')) {
				i += 2;
			} else
				utterance->line[j++] = line[i++];
		}
		utterance->line[j] = EOS;
		
		printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
	}
}

void call() {		/* this function is self-explanatory */
	char gem[BUFSIZ], isEOS;
	long pos, oPos, ti;
	
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	fprintf(fpout, "@Participants:\tPAR Participant, INV Investigator\n");
	fprintf(fpout, "@ID:	eng|tang|PAR|||||Participant|||\n");
	fprintf(fpout, "@ID:	eng|text|INV|||||Investigator|||\n");
	gem[0] = EOS;
	utterance->speaker[0] = EOS;
	oPos = 0L;
	uttline[0] = EOS;
	spareTier1[0] = EOS;
	while (fgets_cr(spareTier1, UTTLINELEN, fpin) != NULL) {
		while (uS.isInvisibleHeader(spareTier1)) {
			if (fgets_cr(spareTier1, UTTLINELEN, fpin) == NULL) {
				spareTier1[0] = EOS;
				break;
			}
		}
		for (pos=0L; spareTier1[pos] != EOS; pos++) {
			if (spareTier1[pos] == '\t' || spareTier1[pos] == '\n')
				spareTier1[pos] = ' ';
		}

		pos = 0L;
		for (; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
		for (ti=pos; !isSpace(spareTier1[ti]) && spareTier1[ti] != EOS; ti++) ;
		if (spareTier1[ti] != EOS) {
			spareTier1[ti] = EOS;
			for (ti++; isSpace(spareTier1[ti]) || spareTier1[ti] == '\n'; pos++) ;
			isEOS = TRUE;
		} else
			isEOS = FALSE;
		if (uS.mStricmp(gem, spareTier1) != 0) {
			strcpy(gem, spareTier1+pos);
			printout("@G:", gem, NULL, NULL, TRUE);
		}
		if (isEOS) {
			for (pos=ti; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
			for (ti=pos; !isSpace(spareTier1[ti]) && spareTier1[ti] != EOS; ti++) ;
			if (spareTier1[ti] != EOS) {
				spareTier1[ti] = EOS;
				for (ti++; isSpace(spareTier1[ti]) || spareTier1[ti] == '\n'; pos++) ;
				isEOS = TRUE;
			} else
				isEOS = FALSE;
			strcpy(utterance->speaker, spareTier1+pos);
			if (isEOS) { //  beg time 
				for (pos=ti; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
				for (ti=pos; !isSpace(spareTier1[ti]) && spareTier1[ti] != EOS; ti++) ;
				if (spareTier1[ti] != EOS) { // end time
					for (pos=ti+1; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
					for (ti=pos; !isSpace(spareTier1[ti]) && spareTier1[ti] != EOS; ti++) ;
					if (spareTier1[ti] != EOS) { // speaker utterance
						for (pos=ti+1; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
						while (spareTier1[pos] != EOS) {
							uttline[oPos] = spareTier1[pos++];
							if (uttline[oPos] == '.' || uttline[oPos] == '!' || uttline[oPos] == '?') {
								oPos++;
								uttline[oPos] = EOS;
								if (uttline[0]) {
									uS.remblanks(uttline);
//									lowercaseTier(uttline);
									printTier(uttline);
								}
								oPos = 0L;
								uttline[0] = EOS;
								for (; isSpace(spareTier1[pos]) || spareTier1[pos] == '\n'; pos++) ;
							} else
								oPos++;
						}
						if (utterance->speaker[0] != 'S' && utterance->speaker[0] != 's') {
//							oPos++;
							uttline[oPos] = EOS;
							if (uttline[0]) {
								uS.remblanks(uttline);
//								lowercaseTier(uttline);
								printTier(uttline);
							}
							oPos = 0L;
							uttline[0] = EOS;
						}
					}
				}
			}
		}
	}
	
	if (*uttline != EOS) {
		uttline[pos] = EOS;
		for (pos=0; uttline[pos] == '\n'; pos++) ;
		for (; uttline[pos] == ' ' || uttline[pos] == '\n'; pos++) ;
		if (uttline[pos] != EOS) {
			uS.remblanks(uttline+pos);
//			lowercaseTier(uttline+pos);
//			addUttDel(uttline+pos);
			printTier(uttline+pos);
		}
	}
	fprintf(fpout, "@End\n");
}
