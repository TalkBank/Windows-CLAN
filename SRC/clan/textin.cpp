/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"
#ifdef _WIN32
	#include <TextUtils.h>
#endif

#if !defined(UNX)
#define _main textin_main
#define call textin_call
#define getflag textin_getflag
#define init textin_init
#define usage textin_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;

void usage() {
	puts("TEXTIN converts plain text file into CHAT file");
	printf("Usage: textin [%s] filename(s)\n",mainflgs());
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
	CLAN_PROG_NUM = TEXTIN;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	maingetflag(f,f1,i);
}

void call() {		/* this function is self-explanatory */
	register long pos;
	register int cr;
	register int i;
	char bl, qf;

	if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
		return;
	if (uS.partcmp(utterance->line,FONTHEADER,FALSE,FALSE)) {
		cutt_SetNewFont(utterance->line, EOS);
		fputs(utterance->line, fpout);
		if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
			return;
	}
	pos = 0L;
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	fprintf(fpout, "@Participants:\tTXT Text\n");
	fprintf(fpout, "@Options:\theritage\n");
	fprintf(fpout, "@ID:	eng|text|TXT|||||Text|||\n");
	i = 0;
	cr = 0;
	bl = TRUE;
	qf = FALSE;
	*uttline = EOS;
	do {
		if (utterance->line[pos] == '\t')
			utterance->line[pos] = ' ';
		if (utterance->line[pos] == '\n') {
			if (bl) {
				i = cr;
				fprintf(fpout, "@Blank\n");
			}
			bl = TRUE;
			cr = i;
		} else if (utterance->line[pos] != ' ')
			bl = FALSE;

		if (utterance->line[pos] == '"') {
			if (!qf) {
				uttline[i++] = (char)0xe2;
				uttline[i++] = (char)0x80;
				uttline[i++] = (char)0x9c;
				qf = TRUE;
			} else {
				uttline[i++] = (char)0xe2;
				uttline[i++] = (char)0x80;
				uttline[i++] = (char)0x9d;
				qf = FALSE;
			}
		} else
			uttline[i++] = utterance->line[pos];
		if (uS.partwcmp(uttline, UTF8HEADER) && i >= 5) {
			i -= 5;
			pos++;
		} else if (uS.partwcmp(utterance->line+pos, FONTMARKER)) {
			cutt_SetNewFont(utterance->line,']');
			uttline[i-1] = EOS;
			for (i=0; uttline[i] == '\n'; i++) ;
			if (uttline[i] == ' ')
				fprintf(fpout, "@Indent\n");
			for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
			if (uttline[i])
				printout("*TXT:", uttline+i, NULL, NULL, TRUE);
			*uttline = EOS;
			qf = FALSE;
			i = 0;
			uttline[i++] = utterance->line[pos];
			pos++;
		} else if (utterance->line[pos] == '.' || utterance->line[pos] == '!' || utterance->line[pos] == '?' || 
			(i > UTTLINELEN-80 && (utterance->line[pos] == ' ' || utterance->line[pos] == '\t'))) {
			uttline[i] = EOS;
			for (i=0; uttline[i] == '\n'; i++) ;
			if (uttline[i] == ' ')
				fprintf(fpout, "@Indent\n");
			for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
			if (uttline[i])
				printout("*TXT:", uttline+i, NULL, NULL, TRUE);
			*uttline = EOS;
			qf = FALSE;
			i = 0;
			pos++;
			while (utterance->line[pos] == ' ' || utterance->line[pos] == '\t')
				pos++;
		} else
			pos++;
		if (utterance->line[pos] == EOS) {
			if (feof(fpin))
				break;
			if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
				break;
			pos = 0L;
		}
	} while (TRUE) ;
	if (*uttline != EOS) {
		uttline[i] = EOS;
		for (i=0; uttline[i] == '\n'; i++) ;
		if (uttline[i] == ' ')
			fprintf(fpout, "@Indent\n");
		for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
		if (uttline[i])
			printout("*TXT:", uttline+i, NULL, NULL, TRUE);
	}
	fprintf(fpout, "@End\n");
}
