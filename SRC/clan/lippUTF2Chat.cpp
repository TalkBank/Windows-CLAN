/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main lipp2chat_main
#define call lipp2chat_call
#define getflag lipp2chat_getflag
#define init lipp2chat_init
#define usage lipp2chat_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

static char lang[64];

extern char OverWriteFile;

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		stout = FALSE;
		onlydata = 1;
		*lang = EOS;
	}
}

void usage() {
	printf("Usage: lipp2chat [lS %s] filename(s)\n", mainflgs());
    puts("+lS: Specify language S");
	mainusage(TRUE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = LIPP2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

static void checkQT(char *line) {
	int i, j;
	char isSecondCheck;

	isSecondCheck = FALSE;
	uS.remblanks(line);
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '"') {
			if (isSecondCheck) {
				strcpy(line+i, line+i+1);
				i--;
			} else {
				for (; i >= 0 && (line[i] == '"' || isSpace(line[i])); i--) ;
				i++;
				for (j=i; line[j] == '"' || isSpace(line[j]); j++) ;
				if (j > i)
					strcpy(line+i, line+j);
				uS.shiftright(line+i, 4);
				line[i++] = ' ';
				line[i++] = '[';
				line[i++] = '%';
				line[i++] = ' ';
				strcat(line, "]");
				isSecondCheck = TRUE;
			}
		}
	}
}

static int checkDelims(char *line) {
	int  i;
	char UTDfound;

	UTDfound = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (uS.IsUtteranceDel(line, i)) {
			UTDfound = TRUE;
		}
	}
	if (!UTDfound)
		strcat(line, ".");
	return(TRUE);
}

void call() {
	int i, j;

	if (*lang == EOS) {
	    fprintf(stderr,"Please specify language for @Languages: header with \"+l\" option.\n");
	    cutt_exit(0);
	}

	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	%s\n", lang);
	fprintf(fpout, "@Participants:	CHI Target_Child\n");
	fprintf(fpout, "@ID:	%s|lipp|CHI|||||Target_Child|||\n", lang);
	i = -1;
	utterance->line[0]    = EOS;
	utterance->tuttline[0] = EOS;
	spareTier1[0] = EOS;
	while (fgets_cr(templineC4, UTTLINELEN, fpin)) {
		lineno++;

		uS.remblanks(templineC4);
		for (j=0; isSpace(templineC4[j]); j++) ;
		if (isalpha(templineC[0]) && isdigit(templineC[1]))
			j++;
		for (; isdigit(templineC4[j]); j++) ;
		if (templineC4[j] == '#' && templineC4[j+1] == '"')
			j++;
		else if (templineC4[j] == ' ' && templineC4[j+1] != '"' && isalnum(templineC4[j+1]))
			templineC4[j] = '"';
		if (templineC4[j] < ' ' && templineC4[j+1] == '"')
			j++;
		for (; isSpace(templineC4[j]); j++) ;
		if (j > 0)
			strcpy(templineC4, templineC4+j);

		if (uS.isUTF8(templineC4) || uS.isInvisibleHeader(templineC4))
			continue;
		if (templineC4[0] == '\n' || templineC4[0] == EOS)
			continue;
		if (templineC4[0] == '#' && templineC4[1] == '#')
			continue;
		if (templineC4[0] != '#' && templineC4[0] != '"') {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno-1);
			fprintf(stderr,"Corrupted or missing either # # or \" \" lines.\n");
			cutt_exit(0);
		}
		if (templineC4[0] == '"') {
			if (i == 0 || utterance->line[0] != EOS) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno-1);
				fprintf(stderr,"Missing or corrupted either # # or \" \" lines above; i=%d.\n", i);
				cutt_exit(0);
			}
			i = 0;
		} else {
			i++;
			if (i > 2) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno-1);
				fprintf(stderr,"Too many # # lines; i=%d.\n", i);
				fprintf(stderr,"  Possibly \" \" line is missing first \" character or it is not in the first column.\n");
				cutt_exit(0);
			}
		}
		if (i == 0) {
			strcpy(utterance->line, templineC4+1);
			j = strlen(utterance->line) - 1;
			while (j >= 0 && (isSpace(utterance->line[j]) || utterance->line[j] == '\n' || utterance->line[j] == '"'))
				j--;
			utterance->line[j+1] = EOS;

			for (j=0; isSpace(utterance->line[j]); j++) ;
			if (j > 0)
				strcpy(utterance->line, utterance->line+j);
			checkQT(utterance->line);
			if (utterance->line[0] == EOS)
				strcpy(utterance->line, "xxx .");
			else {
				checkDelims(utterance->line);
				removeExtraSpace(utterance->line);
			}
		} else if (i == 1) {
			strcpy(spareTier1, templineC4+1);

			j = strlen(spareTier1) - 1;
			while (j >= 0 && (isSpace(spareTier1[j]) || spareTier1[j] == '\n' || spareTier1[j] == '#'))
				j--;
			spareTier1[j+1] = EOS;

			for (j=0; isSpace(spareTier1[j]); j++) ;
			if (j > 0)
				strcpy(spareTier1, spareTier1+j);
		} else if (i == 2) {
			strcpy(utterance->tuttline, templineC4+1);
				
			j = strlen(utterance->tuttline) - 1;
			while (j >= 0 && (isSpace(utterance->tuttline[j]) || utterance->tuttline[j] == '\n' || utterance->tuttline[j] == '#'))
				j--;
			utterance->tuttline[j+1] = EOS;

			for (j=0; isSpace(utterance->tuttline[j]); j++) ;
			if (j > 0)
				strcpy(utterance->tuttline, utterance->tuttline+j);

			if (utterance->line[0] != EOS)
				printout("*CHI:", utterance->line, NULL, NULL, TRUE);
			if (utterance->tuttline[0] != EOS)
				printout("%pho:", utterance->tuttline, NULL, NULL, TRUE);
			if (utterance->tuttline[0] != EOS)
				printout("%mod:", spareTier1, NULL, NULL, TRUE);
			utterance->line[0]    = EOS;
			utterance->tuttline[0] = EOS;
			spareTier1[0] = EOS;
		}
	}
	if (utterance->line[0] != EOS)
		printout("*CHI:", utterance->line, NULL, NULL, TRUE);
	if (utterance->tuttline[0] != EOS)
		printout("%pho:", utterance->tuttline, NULL, NULL, TRUE);
	fprintf(fpout, "@End\n");
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'f':
				break;
		case 'l':
		    if (!*f) {
		        fprintf(stderr,"Please specify language after +l option.\n");
				cutt_exit(0);
		    }
		    strncpy(lang, f, 64-2);
		    lang[64-2] = EOS;
		    break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
