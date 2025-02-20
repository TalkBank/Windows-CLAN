/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"

#if !defined(UNX)
#define _main convort_main
#define call convort_call
#define getflag convort_getflag
#define init convort_init
#define usage convort_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define DICNAME "ort.cut"
#define PERIOD 50

extern struct tier *defheadtier;
extern char OverWriteFile;

static FNType dicname[256];
static char convort_FirstTime, isdup;

static struct words {
	char isFinalPos;
	char *word;
	char *toword;
	struct words *next;
} *head;

static void convort_cleanup(void) {
	struct words *t;

	while (head != NULL) {
		t = head;
		head = head->next;
		free(t->word);
		free(t->toword);
		free(t);
	}
}

static void convort_overflow() {
	fprintf(stderr,"convort: no more memory available.\n");
	convort_cleanup();
	cutt_exit(0);
}

static void convort_makenewsym(char *b, char *e, char isFinalPos) {
	struct words *nextone;

	if (head == NULL) {
		head = NEW(struct words);
		nextone = head;
	} else {
		nextone = head;
		while (nextone->next != NULL) nextone = nextone->next;
		nextone->next = NEW(struct words);
		nextone = nextone->next;
	}
	if (nextone == NULL)
		convort_overflow();
	nextone->next = NULL;
	nextone->isFinalPos = isFinalPos;
	nextone->toword = NULL;
	nextone->word = (char *)malloc(strlen(b)+1);
	if (nextone->word == NULL)
		convort_overflow();
	strcpy(nextone->word,b);
	nextone->toword = (char *)malloc(strlen(e)+1);
	if (nextone->toword == NULL)
		convort_overflow();
	strcpy(nextone->toword, e);
	if (isFinalPos)
		fprintf(stderr, "From \"%s\" to \"%s\" final word position\n", nextone->word, nextone->toword);
	else
		fprintf(stderr, "From \"%s\" to \"%s\"\n", nextone->word, nextone->toword);
}

static void convort_readdict(void) {
	FILE *fdic;
	char line[1024], *b, *e, isFinalPos;
	FNType mFileName[FNSize];

	if (dicname[0] == EOS) {
		uS.str2FNType(dicname, 0L, DICNAME);
	}
	if ((fdic=OpenGenLib(dicname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", dicname, mFileName);
		cutt_exit(0);
	}
	while (fgets_cr(line, 1024, fdic)) {
		if (uS.isUTF8(line) || uS.isInvisibleHeader(line))
			continue;
		if (line[0] == ';' && line[1] == ' ')
			continue;
		if (line[0] == '#' && line[1] == ' ')
			continue;
		if (strncmp(line, "*FINAL_WORD_POS*", 16) == 0) {
			isFinalPos = TRUE;
			strcpy(line, line+16);
		} else
			isFinalPos = FALSE;
		uS.remFrontAndBackBlanks(line);
		if (line[0] == EOS)
			continue;
		b = line;
		for (e=line; *e != ' ' && *e != '\t' && *e != '\n' && *e != EOS; e++) ;
		if (*e != EOS) {
			*e = EOS;
			for (e++; *e == ' ' || *e == '\t' || *e == '\n'; e++) ;
		}
		convort_makenewsym(b, e, isFinalPos);
	}
	fclose(fdic);
	fprintf(stderr,"\n");
}

void init(char f) {
	if (f) {
		stout = FALSE;
		*dicname = EOS;
		isdup = FALSE;
		convort_FirstTime = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		head = NULL;
		onlydata = 1;
	} else if (convort_FirstTime) {
		if (isdup == FALSE)
			convort_readdict();
		convort_FirstTime = FALSE;
	}
}

void usage() {
	puts("CONVORT changes one orthography to other one.");
	printf("Usage: convort [cF %s] filename(s)\n", mainflgs());
	printf("+cF: dictionary file (Default %s) \n", DICNAME);
	printf("+d : only duplicate speaker tier on %%ort tier\n");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(FALSE);
	puts("Dictionary file format: \"from string\" \"to string\"");
	puts("\nExample:");
	puts("\tconvort +cFileName.cut *.cha");
	cutt_exit(0);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CONVORT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	convort_cleanup();
}

static int isWordEng(char *line, int pos) {
	if (pos == 0 || (pos > 1 && isSpace(line[pos-1]))) {
		for (; !isSpace(line[pos]) && line[pos] != '\n' && line[pos] != EOS; pos++) {
			if (line[pos] == '@' && line[pos+1] == 's') {
				for (pos++; !isSpace(line[pos]) && line[pos] != '\n' && line[pos] != EOS; pos++) ;
				return(pos);
			}
		}
	}
	return(0);
}

static int change(char *line, AttTYPE *att, int pos, int wlen, int tlen, char *to_word) {
	register int i;

	if (tlen > wlen) {
		int num;
		num = tlen - wlen;
		for (i=strlen(line); i >= pos; i--) {
			line[i+num] = line[i];
			att[i+num] = att[i];
		}
	} else if (tlen < wlen) {
		if (*to_word == EOS)
			att_cp(0, line+pos+tlen, line+pos+wlen, att+pos+tlen, att+pos+wlen);
		else
			att_cp(0, line+pos+tlen-1, line+pos+wlen-1, att+pos+tlen-1, att+pos+wlen-1);
	}

	for (; *to_word != EOS; pos++) {
		line[pos] = *to_word++;
		if (pos > 0)
			att[pos] = att[pos-1];
	}
	return(pos-1);
}	

static char isRightPos(char *line, int wlen, struct words *nextone) {
	if (nextone->isFinalPos == TRUE) {
		for (; *line != EOS && !isSpace(*line) && *line != '@' && wlen > 0; wlen--)
			line++;
		if (wlen > 0)
			return(FALSE);
		else if (*line == EOS || isSpace(*line) || *line == '@')
			return(TRUE);
		return(FALSE);
	}
	return(TRUE);
}

static long FindAndChange(char *line, AttTYPE *att, long isFound) {
	register int pos;
	struct words *nextone;
	int wlen, tlen;
	char isOneMatch;

	pos = 0;
	nextone = head;
	while (line[pos] != EOS) {
		if (line[pos] == (char)0xE2 && line[pos+1] == (char)0x80) {
			pos += 3;
		} else if (line[pos] == '#' || line[pos] == '_') {
			pos++;
		} else if (line[pos] == '@') {
			for (pos++; !isSpace(line[pos]) && line[pos] != '\n' && line[pos] != EOS; pos++) ;
		} else if (line[pos] == '&' && (line[pos+1] == '=' || line[pos+1] == '~')) {
			for (pos++; !isSpace(line[pos]) && line[pos] != '\n' && line[pos] != EOS; pos++) ;
		} else if (line[pos] == '[' && (line[pos+1] == '^' || line[pos+1] == '%' || line[pos+1] == '*')) {
			for (pos++; line[pos] != ']' && line[pos] != EOS; pos++) ;
		} else if ((tlen=isWordEng(line, pos)) != 0) {
			pos = tlen;
		} else {
			isOneMatch = FALSE;
			nextone = head;
			while (nextone != NULL) {
				wlen = strlen(nextone->word);
				if (strncmp(line+pos, nextone->word, wlen) == 0 && isRightPos(line+pos, wlen, nextone)) {
					tlen = strlen(nextone->toword);
					if (tlen == 0) {
						strcpy(line+pos, line+pos+wlen);
						pos--;
					} else if (wlen == 1 && tlen == 1) {
						line[pos] = nextone->toword[0];
					} else {
						pos = change(line, att, pos, wlen, tlen, nextone->toword);
					}
					isFound++;
					isOneMatch = TRUE;
					break;
				} else
					nextone = nextone->next;
			}
			if (isOneMatch == FALSE) {
				if (line[pos] == '.' || line[pos] == ',' || line[pos] == '?' || line[pos] == '!' ||
					line[pos] == '(' || line[pos] == ')' || line[pos] == '[' || line[pos] == ']' ||
					line[pos] == '<' || line[pos] == '>' ||
					line[pos] == '~' || line[pos] == '+' || line[pos] == '%' || line[pos] == '"' ||
					line[pos] == ':' || line[pos] == '^' || line[pos] == '/' || line[pos] == '0' ||
					isSpace(line[pos])) {
				} else {
					fprintf(stderr,"\rFailed to match letter starting at \"%s\"\n", line+pos);
				}
			}
			pos++;
		}
	}
	return(isFound);
}

void call() {
	long tlineno = 0L;
	long isFound;
	int i, j;

	isFound = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
		for (i=0; isSpace(utterance->line[i]); i++) ;
		if (utterance->speaker[0] == '*') {
			uS.remblanks(utterance->line);
			att_cp(0, spareTier1, utterance->line+i, tempAtt, utterance->attLine+i);
			if (!nomap) {
				for (j=i; utterance->line[j]; j++) {
					if (MBF) {
						if (my_CharacterByteType(utterance->line, (short)j, &dFnt) == 0)
							utterance->line[j] = (char)tolower((unsigned char)utterance->line[j]);
					} else {
						utterance->line[j] = (char)tolower((unsigned char)utterance->line[j]);
					}
				}
			}
			if (isdup == FALSE)
				isFound = FindAndChange(utterance->line+i, utterance->attLine+i, isFound);
			else
				isFound++;
			printout(utterance->speaker,utterance->line+i,utterance->attSp,utterance->attLine+i, 0);
			printout("%ort:", spareTier1, NULL, tempAtt, 0);
		} else
			printout(utterance->speaker,utterance->line+i,utterance->attSp,utterance->attLine+i, 0);
	}
	fprintf(stderr,"\n");
#ifndef UNX
	if (isFound == 0L && fpout != stdout && !WD_Not_Eq_OD) {
		fprintf(stderr,"**- NO changes made in this file\n");
		if (!replaceFile) {
			fclose(fpout);
			fpout = NULL;
			if (unlink(newfname))
				fprintf(stderr, "Can't delete output file \"%s\".", newfname);
		}
	} else
#endif
	if (isFound > 0L)
		fprintf(stderr,"**+ %ld changes made in this file\n", isFound);
	else
		fprintf(stderr,"**- NO changes made in this file\n");
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				if (*f)
					uS.str2FNType(dicname, 0L, getfarg(f,f1,i));
				break;
		case 'd':
				isdup = TRUE;
				nomap = TRUE;
				no_arg_option(f);
				break;
#ifdef UNX
		case 'L':
			int len;
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
