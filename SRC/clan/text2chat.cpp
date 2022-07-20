/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
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
#define _main text2chat_main
#define call text2chat_call
#define getflag text2chat_getflag
#define init text2chat_init
#define usage text2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;

static char isIndBlankHeaders;
static char isLineUtt;
static char isLowerCase;
static char isHeritage;
static char isPausesINV;

void usage() {
	puts("TEXT2CHAT converts plain text file into CHAT file");
	printf("Usage: text2chat [cN %s] filename(s)\n",mainflgs());
	puts("+c0: insert @Blank and @Indent headers when appropriate");
	puts("+c1: each line is an utterance regardless of presence of utterance delimiter");
	puts("+c2: convert first capitalized word of utterance and quotation to lower case");
	puts("+c3: do not insert \"@Options: heritage\" header");
	puts("+c4: convert lines [...] to *INV: ...");
	mainusage(TRUE);
}

void init(char s) {
	if (s) {
		isIndBlankHeaders = FALSE;
		isLineUtt = FALSE;
		isLowerCase = FALSE;
		isHeritage = TRUE;
		isPausesINV = FALSE;
		stout = FALSE;
		onlydata = 3;
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEXT2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++)
	{
		case 'c':
			if (*f == '0') {
				isIndBlankHeaders = TRUE;
			} else if (*f == '1') {
				isLineUtt = TRUE;
			} else if (*f == '2') {
				isLowerCase = TRUE;
			} else if (*f == '3') {
				isHeritage = FALSE;
			} else if (*f == '4') {
				isPausesINV = TRUE;
			} else {
				fprintf(stderr, "Please choose one of for +c option: '0', '1', '2' or '3'\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

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

static void addTab(char *line) {
	for (; *line != EOS; line++) {
		if (*line == ':') {
			*(line+1) = '\t';
			break;
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

static void printTier(char *line) {
	int len;
	
	len = strlen(line);
	if (isPausesINV && len > 0) {
		len--;
		if (line[0] == '[' && line[len] == ']') {
			line[len] = EOS;
			printout("*INV:", line+1, NULL, NULL, TRUE);
		} else
			printout("*TXT:", line, NULL, NULL, TRUE);
	} else {
		printout("*TXT:", line, NULL, NULL, TRUE);
	}
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
#ifdef _COCOA_APP
	while (uS.isInvisibleHeader(utterance->line)) {
		if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
			return;		
	}
#endif
	pos = 0L;
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	if (isPausesINV)
		fprintf(fpout, "@Participants:\tTXT Text, INV Investigator\n");
	else
		fprintf(fpout, "@Participants:\tTXT Text\n");
	if (isHeritage)
		fprintf(fpout, "@Options:\theritage\n");
	if (isPausesINV) {
		fprintf(fpout, "@ID:	eng|text|TXT|||||Text|||\n");
		fprintf(fpout, "@ID:	eng|text|INV|||||Investigator|||\n");
	} else
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
				if (isIndBlankHeaders)
					fprintf(fpout, "@Blank\n");
			}
			bl = TRUE;
			cr = i;
		} else if (utterance->line[pos] != ' ')
			bl = FALSE;

		if (utterance->line[pos] == '(' && utterance->line[pos+1] == '.' && utterance->line[pos+2] == ')') {
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
		}
		if (utterance->line[pos] == '(' && utterance->line[pos+1] == '.' && utterance->line[pos+2] == '.' &&
			utterance->line[pos+3] == ')') {
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
		}
		if (utterance->line[pos] == '(' && utterance->line[pos+1] == '.' && utterance->line[pos+2] == '.' &&
			utterance->line[pos+3] == '.' && utterance->line[pos+4] == ')') {
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
			uttline[i++] = utterance->line[pos++];
		}
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
			if (uttline[i] == ' ' && isIndBlankHeaders)
				fprintf(fpout, "@Indent\n");
			for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
			if (uttline[i]) {
				uS.remblanks(uttline+i);
				if (isLowerCase)
					lowercaseTier(uttline+i);
				if (uttline[i] == '@') {
					addTab(uttline+i);
					fprintf(fpout, "%s\n", uttline+i);
				} else {
//2019-11-07					if (!isHeritage)
						addUttDel(uttline+i);
					printTier(uttline+i);
				}
			}
			*uttline = EOS;
			qf = FALSE;
			i = 0;
			uttline[i++] = utterance->line[pos];
			pos++;
		} else if (utterance->line[pos] == '\n' && isLineUtt) {
			uttline[i] = EOS;
			for (i=0; uttline[i] == '\n'; i++) ;
			if (uttline[i] == ' ' && isIndBlankHeaders)
				fprintf(fpout, "@Indent\n");
			for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
			if (uttline[i]) {
				uS.remblanks(uttline+i);
				if (isLowerCase)
					lowercaseTier(uttline+i);
				if (uttline[i] == '@') {
					addTab(uttline+i);
					fprintf(fpout, "%s\n", uttline+i);
				} else {
//2019-11-07					if (!isHeritage)
						addUttDel(uttline+i);
					printTier(uttline+i);
				}
			}
			*uttline = EOS;
			qf = FALSE;
			i = 0;
			pos++;
			while (utterance->line[pos] == ' ' || utterance->line[pos] == '\t')
				pos++;
		} else if (!isLineUtt && (utterance->line[pos] == '.' || utterance->line[pos] == '!' ||
								  utterance->line[pos] == '?')) {
		   uttline[i] = EOS;
		   for (i=0; uttline[i] == '\n'; i++) ;
		   if (uttline[i] == ' ' && isIndBlankHeaders)
			   fprintf(fpout, "@Indent\n");
		   for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
		   if (uttline[i]) {
			   uS.remblanks(uttline+i);
			   if (isLowerCase)
				   lowercaseTier(uttline+i);
			   printTier(uttline+i);
		   }
		   *uttline = EOS;
		   qf = FALSE;
		   i = 0;
		   pos++;
		   while (utterance->line[pos] == ' ' || utterance->line[pos] == '\t')
			   pos++;
	   } else if (i > UTTLINELEN-80 && (utterance->line[pos] == ' ' || utterance->line[pos] == '\t')) {
		  uttline[i] = EOS;
		  for (i=0; uttline[i] == '\n'; i++) ;
		  if (uttline[i] == ' ' && isIndBlankHeaders)
			  fprintf(fpout, "@Indent\n");
		  for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
		  if (uttline[i]) {
			  uS.remblanks(uttline+i);
			  if (isLowerCase)
				  lowercaseTier(uttline+i);
			  printTier(uttline+i);
		  }
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
#ifdef _COCOA_APP
			while (uS.isInvisibleHeader(utterance->line)) {
				if (fgets_cr(utterance->line, UTTLINELEN, fpin) == NULL)
					return;		
			}			
#endif
			pos = 0L;
		}
	} while (TRUE) ;
	if (*uttline != EOS) {
		uttline[i] = EOS;
		for (i=0; uttline[i] == '\n'; i++) ;
		if (uttline[i] == ' ' && isIndBlankHeaders)
			fprintf(fpout, "@Indent\n");
		for (; uttline[i] == ' ' || uttline[i] == '\n'; i++) ;
		if (uttline[i]) {
			uS.remblanks(uttline+i);
			if (isLowerCase)
				lowercaseTier(uttline+i);
			if (uttline[i] == '@' && isLineUtt) {
				uS.remblanks(uttline+i);
				addTab(uttline+i);
				fprintf(fpout, "%s\n", uttline+i);
			} else {
//2019-11-07				if (!isHeritage)
					addUttDel(uttline+i);
				printTier(uttline+i);
			}
		}
	}
	fprintf(fpout, "@End\n");
}
