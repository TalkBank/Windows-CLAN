/**********************************************************************
 "Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */


#define CHAT_MODE 3
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

#define PERIOD 50

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char AddCEXExtension;
extern char isRecursive;

static char *headersFolder, *bodyFolder;

void usage() {
	printf("Usage: temp [b h re] filename(s)\n");
	puts("+bS: specify the folder where body files are located.");
	puts("+hS: specify the folder where header files are located.");
	puts("+re: run program recursively on all sub-directories.");
	puts("Example: set working folder to header files, then type\n\ttemp +b../Thomas-orig *.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		headersFolder = NULL;
		bodyFolder = NULL;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (headersFolder == NULL && bodyFolder == NULL) {
			fprintf(stderr, "Please use either +b or +h option to specify body or header files folder.\n");
			cutt_exit(0);
		}
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
		case 'b':
			if (headersFolder != NULL) {
				fprintf(stderr, "Please specify only one header/body option.\n");
				cutt_exit(0);
			}
			bodyFolder = getfarg(f,f1,i);
			break;
		case 'h':
			if (bodyFolder != NULL) {
				fprintf(stderr, "Please specify only one header/body option.\n");
				cutt_exit(0);
			}
			headersFolder = getfarg(f,f1,i);
			break;
		case 'r':
			if (*f == 'e') {
				f++;
				no_arg_option(f);
				isRecursive = TRUE;
				break;
			}
		case '1':
			replaceFile = TRUE;
			no_arg_option(f);
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

void call() {
	char *s, isSpeakerFound;
	FILE *fp, *tfp;
	long tlineno = 0L;
	FNType tFileName[FNSize];

	strcpy(tFileName, wd_dir);
	if (bodyFolder != NULL)
		addFilename2Path(tFileName, bodyFolder);
	else
		addFilename2Path(tFileName, headersFolder);
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	addFilename2Path(tFileName, s);
	fp = fopen(tFileName, "r");
	if (fp == NULL) {
		fprintf(stderr,"Can't open file to read: %s\n", tFileName);
		cutt_exit(0);
	}
	if (bodyFolder == NULL) {
		tfp = fpin;
		fpin = fp;
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
		if (utterance->speaker[0] == '*')
			break;
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
	if (bodyFolder == NULL) {
		fpin = tfp;
	} else {
		tfp = fpin;
		fpin = fp;
	}
	isSpeakerFound = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
		if (utterance->speaker[0] == '*')
			isSpeakerFound = TRUE;
		if (isSpeakerFound)
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
	if (bodyFolder != NULL) {
		fpin = tfp;
	}
	fclose(fp);
	fprintf(stderr,"\n");
}
