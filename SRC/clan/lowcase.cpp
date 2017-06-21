/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 4

#include "cu.h"
#ifndef UNX
	#include "ced.h"
#else
	#include "c_curses.h"
#endif

#if !defined(UNX)
#define _main lowcase_main
#define call lowcase_call
#define getflag lowcase_getflag
#define init lowcase_init
#define usage lowcase_usage
#endif


#define IS_WIN_MODE FALSE
#include "mul.h" 

#define DICNAME "caps.cut"
#define PERIOD 50

static char lowcase_ftime, isFWord, isMakeBold;
static FNType lc_dicname[FNSize];
static char isChangeToUpper;

static struct words {
	char *word;
	struct words *next;
} *lowcase_head;


extern struct tier *defheadtier;
extern char OverWriteFile;

void usage() {
	puts("LOWCASE converts all words to lower case except those specified as pronouns.");
	printf("Usage: lowcase [b c d iF %s] filename(s)\n", mainflgs());
	puts("+b : mark upper case letters as bold, then convert them to lower case");
	puts("+c : convert the first word on tier only to lower case");
	fprintf(stdout, "+d : do NOT change words from \"%s\" file, lower case the rest\n", DICNAME);
	fprintf(stdout, "+d1: capitalize words from \"%s\" file, do NOT change the rest\n", DICNAME);
	fprintf(stdout, "+iF: file F with capitalize words (default: %s)\n", DICNAME);
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(TRUE);
}

static struct words *lowcase_makenewsym(char *word) {
	struct words *nextone;

	if (lowcase_head == NULL) {
		lowcase_head = NEW(struct words);
		nextone = lowcase_head;
	} else {
		nextone = lowcase_head;
		while (nextone->next != NULL) nextone = nextone->next;
		nextone->next = NEW(struct words);
		nextone = nextone->next;
	}
	nextone->word = (char *)malloc(strlen(word)+1);
	strcpy(nextone->word, word);
	nextone->next = NULL;
	return(nextone);
}

static void lowcase_readdict() {
	FILE *fdic;
	char chrs;
	int index = 0;
	struct words *nextone;
	FNType mFileName[FNSize];

	if (*lc_dicname == EOS) {
		fprintf(stderr,"Warning: No dep-file was specified.\n");
		cutt_exit(0);
		fdic = NULL;
	} else if ((fdic=OpenGenLib(lc_dicname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "Warning: Can't open either one of the dep-files:\n\t\"%s\", \"%s\"\n", lc_dicname, mFileName);
	}
	if (fdic != NULL) {
		for (chrs=getc_cr(fdic,NULL);!feof(fdic) && (chrs==' ' || chrs=='\n');chrs=getc_cr(fdic,NULL)) ;
		while (!feof(fdic)) {
			if (chrs == ' ' || chrs == '\t' || chrs == '\n') {
				templineC4[index] = EOS;
				if (templineC4[0] != EOS) {
					uS.uppercasestr(templineC4, &dFnt, C_MBF);
					nextone = lowcase_makenewsym(templineC4);
					templineC4[0] = EOS;
					index = 0;
				}
			} else
				templineC4[index++] = chrs;
			chrs = getc_cr(fdic, NULL);
		}
		fclose(fdic);
	}
}

void init(char f) {
	if (f) {
		lowcase_ftime = TRUE;
		isChangeToUpper = 10;
		isFWord = FALSE;
		isMakeBold = FALSE;
		stout = FALSE;
		lowcase_head = NULL;
		uS.str2FNType(lc_dicname, 0L, DICNAME);
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
/*
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
*/
	} else if (lowcase_ftime) {
		lowcase_readdict();
		lowcase_ftime = FALSE;
	}
}

static void free_head(struct words *lowcase_head) {
	struct words *t;
	
	while (lowcase_head != NULL) {
		t = lowcase_head;
		lowcase_head = lowcase_head->next;
		free(t->word);
		free(t);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = LOWCASE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	free_head(lowcase_head);
}

static int gotmatch(char *word) {
	struct words *nextone;

	strcpy(templineC4,word);
	uS.uppercasestr(templineC4, &dFnt, MBF);
	nextone = lowcase_head;
	while (nextone != NULL) {
		if (strcmp(templineC4,nextone->word) == 0) return(TRUE);
		nextone = nextone->next;
	}
	return(FALSE);
}

static int findNextWord(char *line, long *pos, int i) {
	register int  temp;
	register char sq;
	register char t;

getnewword:
	if (chatmode && *utterance->speaker == '%') {
		if (line[i] == EOS)
			return(0);
//6-6-03		if (i > 0) i--;
		while ((t=line[i]) != EOS && uS.isskip(line,i,&dFnt,MBF) && !uS.isRightChar(line,i,'[',&dFnt,MBF)) {
			i++;
			if (t == '<') {
				temp = i;
				for (i++; line[i] != '>' && line[i]; i++) {
					if (isdigit(line[i])) ;
					else if (line[i]== ' ' || line[i]== '\t' || line[i]== '\n') ;
					else if ((i-1 == temp+1 || !isalpha(line[i-1])) &&
							 line[i] == '-' && !isalpha(line[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(line[i-1])) &&
							 (line[i] == 'u' || line[i] == 'U') &&
							 !isalpha(line[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(line[i-1])) &&
							 (line[i] == 'w' || line[i] == 'W') &&
							 !isalpha(line[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(line[i-1])) &&
							 (line[i] == 's' || line[i] == 'S') &&
							 !isalpha(line[i+1])) ;
					else
						break;
				}
				if (line[i] == '>')
					i++;
				else
					i = temp;
			}
		}
	} else {
		while ((t=line[i]) != EOS && uS.isskip(line,i,&dFnt,MBF) && !uS.isRightChar(line,i,'[',&dFnt,MBF))
			 i++;
	}
	if (t == EOS)
		return(0);
	if (uS.isRightChar(line, i, '[',&dFnt,MBF)) {
		if (uS.isSqBracketItem(line, i+1, &dFnt, MBF))
			sq = TRUE;
		else
			sq = FALSE;
	} else
		sq = FALSE;
	*pos = i;
getword_rep:

//	if (dFnt.isUTF) {
		while ((temp=uS.HandleCAChars(line+i, NULL)) != 0) {
			i += temp;
			*pos = i;
		}
		if (uS.isRightChar(line,i,'[',&dFnt,MBF)) {
			for (; line[i] != EOS && !uS.isRightChar(line,i,']',&dFnt,MBF); i++) ;
			if (uS.isRightChar(line,i,']',&dFnt,MBF))
				goto getnewword;
		}
		if (uS.isskip(line,i,&dFnt,MBF))
			goto getnewword;
		if (line[i] == EOS)
			return(0);
//	}
	
	if ((t == '+' || t == '-') && !sq) {
		while (line[++i] != EOS && (!uS.isskip(line,i,&dFnt,MBF) ||
				uS.isRightChar(line,i,'/',&dFnt,MBF) || uS.isRightChar(line,i,'<',&dFnt,MBF) ||
				uS.isRightChar(line,i,'.',&dFnt,MBF) || uS.isRightChar(line,i,'!',&dFnt,MBF) ||
				uS.isRightChar(line,i,'?',&dFnt,MBF) || uS.isRightChar(line,i,',',&dFnt,MBF))) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
				return(i+1);
			}
		}
	} else if (uS.atUFound(line, i, &dFnt, MBF)) {
		while (line[++i] != EOS && (!uS.isskip(line,i,&dFnt,MBF) ||
				uS.isRightChar(line,i,'.',&dFnt,MBF) || uS.isRightChar(line,i,'!',&dFnt,MBF) ||
				uS.isRightChar(line,i,'?',&dFnt,MBF) || uS.isRightChar(line,i,',',&dFnt,MBF))) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
				return(i+1);
			}
		}
	} else {
		while (line[++i] != EOS && (!uS.isskip(line,i,&dFnt,MBF) || sq)) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
				return(i+1);
			}
		}
	}
	if (uS.isRightChar(line, i, '[', &dFnt, MBF)) {
		if (!uS.isSqBracketItem(line, i+1, &dFnt, MBF))
			goto getword_rep;
	}
//	if (line[i] != EOS && line[i] != '\n')
//		i++;
	return(i);
}

static int isNextCharUpper(char *line, NewFontInfo *finfo) {
	while (*line) {
		if (finfo->isUTF) {
			if (UTF8_IS_SINGLE((unsigned char)*line)) {
				if (*line == ':' || *line == '/' || *line == '(' || *line == ')' || uS.HandleCAChars(line, NULL))
					;
				else
					return(isupper((unsigned char)*line));
			} else
				return(FALSE);
		} else {
			if (*line == ':' || *line == '/' || *line == '(' || *line == ')')
				;
			else
				return(isupper((unsigned char)*line));
		}
		line++;
	}
	return(FALSE);
}

static long makeBold(char *line, AttTYPE *atts, NewFontInfo *finfo) {
	int  isUpperC;
	long isFound;
	char isHiden, isSq;

	isSq = FALSE;
	isHiden = FALSE;
	isUpperC = FALSE;
	isFound = 0L;
	while (*line) {
		if (*line == '[')
			isSq = TRUE;
		else if (*line == ']')
			isSq = FALSE;
		if (*line == HIDEN_C)
			isHiden = !isHiden;

		if (isSq || isHiden)
			isUpperC = FALSE;
		else if (finfo->isUTF) {
			if (UTF8_IS_SINGLE((unsigned char)*line)) {
				if (*line == ':' || *line == '/' || *line == '(' || *line == ')' || uS.HandleCAChars(line, NULL))
					;
				else
					isUpperC = (isupper((unsigned char)*line) && (isUpperC || isNextCharUpper(line+1, finfo)));
			} else
				isUpperC = FALSE;
		} else {
			if (*line == ':' || *line == '/' || *line == '(' || *line == ')')
				;
			else
				isUpperC = (isupper((unsigned char)*line) && (isUpperC || isNextCharUpper(line+1, finfo)));
		}

		if (isUpperC) {
			if (isupper((unsigned char)*line))
				*line = (char)tolower((unsigned char)*line);
			*atts = set_bold_to_1(*atts);
			isFound++;
		}
		line++;
		atts++;
	}
	return(isFound);
}

void call() {
	char t;
	char RightSpeaker = FALSE;
	long  i, pos;
	long isFound;
	long tlineno = 0L;


	isFound = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (!stout) {
			tlineno = tlineno + 1L;
			if (tlineno % PERIOD == 0)
				fprintf(stderr,"\r%ld ",tlineno);
		}

		if (!checktier(utterance->speaker) || 
							(!RightSpeaker && *utterance->speaker == '%') ||
							(*utterance->speaker == '*' && nomain)) {
			if (*utterance->speaker == '*') {
				if (nomain)
					RightSpeaker = TRUE;
				else
					RightSpeaker = FALSE;
			}
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			if (*utterance->speaker == '*') {
				RightSpeaker = TRUE;
				if (isMakeBold)
					isFound += makeBold(utterance->line, utterance->attLine, &dFnt);
			}
			if (isMakeBold) {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				continue;
			}

			strcpy(templineC,utterance->speaker);
			uS.uppercasestr(templineC, &dFnt, MBF);
			if (uS.partcmp(templineC,"%TRN:",FALSE,FALSE) || uS.partcmp(templineC,"%MOR:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%SPA:",FALSE,FALSE) || uS.partcmp(templineC,"%SYN:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%SIT:",FALSE,FALSE) || uS.partcmp(templineC,"%PHO:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%MOD:",FALSE,FALSE) || uS.partcmp(templineC,"%ORT:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%LAN:",FALSE,FALSE) || uS.partcmp(templineC,"%COH:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%DEF:",FALSE,FALSE) || uS.partcmp(templineC,"%COD:",FALSE,FALSE) ||
				utterance->speaker[0] == '@') {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				continue;
			}
			if (uS.partcmp(templineC,"%PHO:",FALSE,FALSE) ||
				uS.partcmp(templineC,"%MOD:",FALSE,FALSE))
				punctuation = ",.;?!";
			i = 0L;
			while ((i=findNextWord(utterance->line, &pos, i))) {
/*
				if (utterance->line[i] == EOS)
					j = i;
				else
					j = i - 1;
*/
				t = utterance->line[i];
				utterance->line[i] = EOS;
				if (gotmatch(utterance->line+pos)) {
					if (isChangeToUpper && islower((unsigned char)utterance->line[pos])) {
						isFound++;
						utterance->line[pos] = (char)toupper((unsigned char)utterance->line[pos]);
					}
				} else if (isalpha(utterance->line[pos]) || utterance->line[pos] == '(') {
					if ((utterance->line[pos] == 'I' || utterance->line[pos] == 'i') &&
						!isalpha(utterance->line[pos+1]) && utterance->line[pos+1] != ':' && 
						utterance->line[pos+1] != '(' && utterance->line[pos+1] != ')') {
						if (islower((unsigned char)utterance->line[pos])) {
							if (utterance->line[pos+1] == '@' && (char)tolower((unsigned char)utterance->line[pos+2]) == 'l')
								;
							else if (utterance->line[pos+1] == '(' && isalpha(utterance->line[pos+2]))
								;
							else {
								isFound++;
								utterance->line[pos] = (char)toupper((unsigned char)utterance->line[pos]);
							}
						}
					} else if (isChangeToUpper != 1) {
						if (uS.lowercasestr(utterance->line+pos, &dFnt, MBF))
							isFound++;
					}
				}
				utterance->line[i] = t;
				if (isFWord && isalpha(utterance->line[pos]))
					break;
			}
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
	if (!stout)
		fprintf(stderr,"\n");
#ifndef UNX
	if (isFound == 0L && fpout != stdout && !stout && !WD_Not_Eq_OD) {
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

/*
	char utsym, word[WORDLEN], done, isThisFirstWord, isSq, *line;
	int  index, pos;

	line = utterance->line;
	if (!chatmode) {
		fprintf(stderr,"This program does NOT work in this mode.\n");
		cutt_exit(0);
	}
	lowcase_count = 0;
	inlinenum = 0;
	while ((lowcase_chrs=getnchr(fpin)) == ' ' || lowcase_chrs == '\t') ;
	while (!feof(fpin)) {
		utsym = 0;
		index = 0;
		while (lowcase_chrs != ':' && lowcase_chrs != '\n' && !feof(fpin)) {
			if (index < WORDLEN) speaker[index++] = lowcase_chrs;
			else {
				fprintf(stderr,"Error: line %d is longer than %d\n",inlinenum,WORDLEN);
				cutt_exit(1);
			}
			lowcase_chrs = getnchr(fpin);
		}
		if (lowcase_chrs == '\n' || feof(fpin)) {
			if (index < WORDLEN)
				speaker[index] = EOS;
			else {
				fprintf(stderr,"Error: line %d is longer than %d\n",inlinenum,WORDLEN);
				cutt_exit(1);
			}
			fprintf(fpout,"%s\n",speaker);
			if (!feof(fpin))
				lowcase_chrs = getnchr(fpin);
		} else { // if (lowcase_chrs == '\n' || feof(fpin)) 
			if (index < WORDLEN)
				speaker[index++] = lowcase_chrs;
			else {
				fprintf(stderr,"Error: line %d is longer than %d\n",inlinenum,WORDLEN);
				cutt_exit(1);
			}
			if (index < WORDLEN)
				speaker[index] = EOS;
			else {
				fprintf(stderr,"Error: line %d is longer than %d\n",inlinenum,WORDLEN);
				cutt_exit(1);
			}
			utsym = checktier(speaker);
			lowcase_chrs = getnchr(fpin);
			if (!utsym)
				fprintf(fpout,"%s",speaker);
			while (lowcase_chrs == ' ' || lowcase_chrs == '\t') {
				if (!utsym)
					putc(lowcase_chrs,fpout);
				lowcase_chrs = getnchr(fpin);
			}
			done = FALSE;
			index = 0;
			line[0] = EOS;
			pos = 0;
			*utterance->speaker = *speaker;
			isSq = FALSE;
			isThisFirstWord = TRUE;
			while (!done) {
				word[index++] = lowcase_chrs;
				if (word[index-1] == '[') {
					isSq = TRUE;
					lowcase_chrs = getnchr(fpin);
				} else if (word[index-1] == ']') {
					isSq = FALSE;
					lowcase_chrs = getnchr(fpin);
				} else if (!uS.isskip(word, index-1, &dFnt, MBF) || isSq) {
					lowcase_chrs = getnchr(fpin);
				} else {
					word[index-1] = EOS;
					index = 0;
					if (*word != EOS) {
						if (utsym) {
							if (gotmatch(word)) {
								if (*word >= 'a' && *word <= 'z') 
										*word -= ('a' - 'A');
							} else {
								if (*word=='I' &&
									(word[1]==EOS || word[1]=='\'')) ;
								else
								if (*word=='i' && 
									(word[1]==EOS || word[1]=='\''))
									*word -= ('a' - 'A');
								else if ((!isFWord || isThisFirstWord) && *word != '[')
									uS.lowercasestr(word, &dFnt, MBF);
//26-9-02						if (*word >= 'A' && *word <= 'Z') 
//									*word += ('a' - 'A');
							}
							line[pos] = EOS;
							strcat(line,word);
							pos += strlen(word);
							isThisFirstWord = FALSE;
						} else
							fprintf(fpout,"%s",word);
						word[0] = EOS;
					}
					if (lowcase_chrs == '\n') {
						if (!utsym) putc(lowcase_chrs,fpout);
						else line[pos++] = ' ';
						lowcase_chrs=getnchr(fpin);
						if (lowcase_chrs != ' ' && lowcase_chrs != '\t') done = TRUE;
						while (lowcase_chrs == ' ' || lowcase_chrs == '\t') {
							if (!utsym) putc(lowcase_chrs,fpout);
							lowcase_chrs=getnchr(fpin);
						}
					} else { // if (lowcase_chrs == '\n')  
						if (utsym) line[pos++] = lowcase_chrs;
						else putc(lowcase_chrs,fpout);
						lowcase_chrs = getnchr(fpin);
					}
				} // else to  if (!uS.isskip(lowcase_chrs))
				if (feof(fpin))
					done = TRUE;
			} // while (!done)
			if (utsym) {
				line[pos] = EOS;
				lowcase_remblanks(line);
				printout(speaker,line,NULL,NULL,TRUE);
			}
		} // else to if (lowcase_chrs == '\n' || feof(fpin))
	} // while (!feof(fpin))
	if (!stout) printf("\n");
*/
}


void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'b':
				isMakeBold = TRUE;
				no_arg_option(f);
				break;

		case 'c':
				isFWord = TRUE;
				no_arg_option(f);
				break;

		case 'd':
				isChangeToUpper = (char)atoi(getfarg(f,f1,i));
				if (isChangeToUpper > 1) {
					fputs("The N for +d option must be between 0 - 1\n",stderr);
					cutt_exit(0);
				}
				break;

		case 'i':
				uS.str2FNType(lc_dicname, 0L, getfarg(f,f1,i));
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
