/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 3

#include "cu.h"

#if !defined(UNX)
#define _main segment_main
#define call segment_call
#define getflag segment_getflag
#define init segment_init
#define usage segment_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

struct rtiers {
	char *sp;	/* code descriptor field of the turn	 */
	char *line;		/* text field of the turn		 */
	char *lineOrg;	/* text field of the turn		 */
	char matched;
	long lineno;
	struct rtiers *nexttier;	/* pointer to the next utterance, if	 */
} ;				/* there is only 1 utterance, i.e. no	 */

extern struct tier *defheadtier;
extern char OverWriteFile;

static char isFTime, isGuesstimateBullet;

static char isUtteranceDelim(char *word) {
	int i;
	char isDelimFound;
	
	isDelimFound = FALSE;
	for (i=0; uS.IsCharUtteranceDel(word, i); i++) {
		if (uS.IsUtteranceDel(word, i))
			isDelimFound = TRUE;
	}
	if (isDelimFound)
		return(TRUE);
	else
		return(FALSE);
}

static char isAtEnd(char *main, int m) {
	
	for (; main[m] != EOS; m++) {
		if (!isSpace(main[m]) && main[m] != '\n')
			return(FALSE);
	}
	return(TRUE);
}

static int getNextWord(char *line, int i, char *word) {
	int j;

	for (; isSpace(line[i]); i++) ;
	if (line[i] == EOS) {
		word[0] = EOS;
		return(0);
	}
	j = 0;
	for (; !isSpace(line[i]) && line[i] != EOS; i++) {
		word[j++] = line[i];
	}
	word[j] = EOS;
	return(i);
}

static int findWorEnd(char *main, char *wor) {
	int m, w, ow;
	char mw[BUFSIZ], ww[BUFSIZ], isGetNextWor;

	isGetNextWor = TRUE;
	m = 0;
	w = 0;
	while (1) {
		m = getNextWord(main, m, mw);
		if (m == 0) {
			break;
		}
		if (isGetNextWor == TRUE) {
			for (; uS.IsCharUtteranceDel(wor, w); w++) ;
			ow = w;
			w = getNextWord(wor, w, ww);
			for (; isSpace(wor[w]); w++) ;
			if (wor[w] == HIDEN_C) {
				for (w++; wor[w] != HIDEN_C && wor[w] != EOS; w++) ;
				if (wor[w] == HIDEN_C)
					w++;
			}
		}
		if (w == 0) {
		}
		if (uS.mStricmp(mw, ww) == 0) {
			isGetNextWor = TRUE;
		} else if (isUtteranceDelim(mw) == TRUE && isUtteranceDelim(ww) == FALSE) {
			w = ow;
		} else if (isUtteranceDelim(ww) == TRUE) {
			isGetNextWor = TRUE;
		} else
			isGetNextWor = FALSE;
	}
	return(w);
}

/*
static char isSegPostCode(char *line, int *i) {
	int t = *i;
	
	if (line[t] != ']')
		return(FALSE);
	for (t--; line[t] != '[' && t >= 0; t--) ;
	if (t < 0)
		return(FALSE);
	if (line[t+1] == '+' && line[t+2] == ' ') {
		*i = t - 1;
		return(TRUE);
	}
	return(FALSE);
}

static void addDelim(char *line) {
	register int i;
	char sb = FALSE, hid = FALSE;
	
	i = strlen(line) - 1;
	while (i >= 0) {
		if (!isSpace(line[i]) && line[i] != '\n') {
			if (i > 1 && line[i] == '&' && line[i-1] == '&' && line[i-2] == '&')
				i -= 2;
			else if (line[i] == HIDEN_C)
				hid = !hid;
			else if (line[i] == ']')
				sb = TRUE;
			else if (line[i] == '[')
				sb = FALSE;
			else if (!sb && !hid) {
				if (!uS.IsUtteranceDel(line, i)) {
					if (line[i] == '-' && line[i+1] == ' ')
						line[i+1] = '.';
					else {
						i = strlen(line) - 1;
						do {
							for (; i >= 0 && (isSpace(line[i]) || line[i] == '\n'); i--) ;
							if ( i < 0)
								break;
							if (i > 2 && line[i] == '&' && line[i-1] == '&' && line[i-2] == '&')
								i -= 3;
							else if (line[i] == HIDEN_C) {
								for (i--; line[i] != HIDEN_C && i >= 0; i--) ;
								if (i >= 0)
									i--;
							} else if (isSegPostCode(line, &i))
								;
							else
								break;
						} while (i >= 0) ;
						i++;
						att_shiftright(line+i, utterance->attLine+i, 2);
						line[i++] = ' ';
						line[i++] = '.';
					}
				}
				break;
			}
		}
		i--;
	}
}
*/

static void getDelim(char *line, char *delim) {
	int i, j, sq;
	char isDelimFound;

	delim[0] = EOS;
	sq = 0;
	i = 0;
	while (line[i] != EOS) {
		if (line[i] == '[')
			sq++;
		else if (line[i] == ']') {
			sq--;
			if (sq < 0)
				sq = 0;
		}
		if (uS.IsCharUtteranceDel(line, i) && sq == 0) {
			isDelimFound = FALSE;
			j = 0;
			delim[j++] = ' ';
			while (uS.IsCharUtteranceDel(line, i)) {
				if (uS.IsUtteranceDel(line, i))
					isDelimFound = TRUE;
				delim[j] = line[i];
				j++;
				i++;
			}
			if (isDelimFound == TRUE)
				delim[j] = EOS;
			else
				delim[0] = EOS;
		} else
			i++;
	}
}

static void removeDelims(char *line) {
	int i, j, sq;
	char isDelimFound;

	i = 0;
	sq = 0;
	while (line[i] != EOS) {
		if (line[i] == '[')
			sq++;
		else if (line[i] == ']') {
			sq--;
			if (sq < 0)
				sq = 0;
		}
		if (uS.IsCharUtteranceDel(line, i) && sq == 0) {
			j = i;
			isDelimFound = FALSE;
			while (uS.IsCharUtteranceDel(line, i)) {
				if (uS.IsUtteranceDel(line, i))
					isDelimFound = TRUE;
				i++;
			}
			if (isDelimFound == TRUE) {
				for (; j < i; j++)
					line[j] = ' ';
			}
		} else
			i++;
	}
}

static void replaceDelim(char *wor, char *delim) {
	int i, j, end;
	char isDelimFound = FALSE;
	
	i = strlen(wor) - 1;
	while (i > 0) {
		if (uS.IsCharUtteranceDel(wor, i)) {
			isDelimFound = FALSE;
			end = i + 1;
			while (uS.IsCharUtteranceDel(wor, i)) {
				if (uS.IsUtteranceDel(wor, i))
					isDelimFound = TRUE;
				i--;
			}
			if (isDelimFound == TRUE) {
				i++;
				strcpy(wor+i, wor+end);
				if (delim[0] != EOS) {
					uS.shiftright(wor+i, strlen(delim));
					for (j=0; delim[j] != EOS; j++)
						wor[i++] = delim[j];
				}
				break;
			}
		} else
			i--;
	}
	if (isDelimFound == FALSE && delim[0] != EOS) {
		strcat(wor, delim);
	}
}

static int handleBullets(char *line, char isRemove) {
	int i, cnt;

	cnt = 0;
	i = 0;
	while (line[i] != EOS) {
		if (line[i] == HIDEN_C) {
			cnt++;
			if (isRemove)
				line[i] = ' ';
			for (i++; line[i] != HIDEN_C && line[i] != EOS; i++) {
				if (isRemove)
					line[i] = ' ';
			}
			if (line[i] == HIDEN_C) {
				if (isRemove)
					line[i] = ' ';
				i++;
			}
		} else
			i++;
	}
	return(cnt);
}

static void getWorBegEnd(char *line, long *beg, long *end) {
	int i;
	char isBegFound;

	isBegFound = FALSE;
	i = 0;
	while (line[i] != EOS) {
		if (line[i] == HIDEN_C) {
			if (isBegFound == FALSE) {
				*beg = atol(line+i+1);
				isBegFound = TRUE;
			}
			for (i++; line[i] != HIDEN_C && line[i] != EOS; i++) {
				if (line[i] == '_')
					*end = atol(line+i+1);
			}
			if (line[i] == HIDEN_C)
				i++;
		} else
			i++;
	}
}

static void printTiers(char *sp, char *main, char *wor, char hasChanged, char isWorFound) {
//	int bcnt;
	char delim[128];
	long beg, end;

	delim[0] = EOS;
	if (main != NULL) {
		if (main[0] != EOS) {
			strcpy(templineC, main);
			uS.remFrontAndBackBlanks(templineC);
			getDelim(templineC, delim);
//			bcnt = handleBullets(templineC, FALSE);
			if (hasChanged) {
				if (isWorFound == FALSE) {
					beg = 0L;
					end = 0L;
					getWorBegEnd(templineC, &beg, &end);
					if (beg == 0L && end == 0L) {
						getWorBegEnd(spareTier3, &beg, &end);
						if (isGuesstimateBullet == TRUE) {
						}
					}
					handleBullets(templineC, TRUE);
					sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
					uS.remblanks(templineC);
					strcat(templineC, templineC3);
				} else {
					if (wor != NULL) {
						handleBullets(templineC, TRUE);
					}
//					if (delim[0] == EOS)
//						addDelim(templineC);
					if (wor != NULL) {
						getWorBegEnd(wor, &beg, &end);
						sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
						uS.remblanks(templineC);
						strcat(templineC, templineC3);
					}
				}
			}
			removeExtraSpace(templineC);
			printout(sp, templineC, NULL, NULL, FALSE);
			main[0] = EOS;
		}
	}
	if (wor != NULL) {
		if (wor[0] != EOS) {
			strcpy(templineC, wor);
			if (hasChanged) {
				removeDelims(templineC);
			}
			uS.remFrontAndBackBlanks(templineC);
//			if (delim[0] == EOS)
//				strcat(delim, " .");
			replaceDelim(templineC, delim);
			removeExtraSpace(templineC);
			printout("%wor:", templineC, NULL, NULL, FALSE);
			wor[0] = EOS;
		}
	}
}

static void segmentMainWorTiers(char *sp, char *main, char *wor, char *isJoinNext, char isWorFound) {
	int m, w;
	char cm, cw, hasChanged;

//if (lineno == 169)
//m = 0;
	hasChanged = *isJoinNext;
	*isJoinNext = FALSE;
	m = 0;
	while (main[m] != EOS) {
		if (main[m] == '&' && main[m+1] == '&' && main[m+2] == '&') {
			hasChanged = TRUE;
			strcpy(main+m, main+m+3);
			if (isAtEnd(main, m) == TRUE) {
				*isJoinNext = TRUE;
				return;
			}
			cm = main[m];
			main[m] = EOS;
			w = findWorEnd(main, wor);
			cw = wor[w];
			wor[w] = EOS;
			printTiers(sp, main, wor, hasChanged, isWorFound);
			main[m] = cm;
			strcpy(main, main+m);
			m = 0;
			wor[w] = cw;
			strcpy(wor, wor+w);
			if (main[m] == EOS)
				break;
		} else
			m++;
	}
	if (main[0] != EOS) {
		printTiers(sp, main, wor, hasChanged, isWorFound);
	}
}

static void worHasSymbol(char *main, char *wor) {
	int w;

	for (w=0; wor[w] != EOS; w++) {
		if (wor[w] == '&' && wor[w+1] == '&' && wor[w+2] == '&') {
			strcat(main, " &&&");
			strcpy(wor+w, wor+w+3);
			return;
		}
	}
}

void call() {
	int i;
	char isJoinNext;
	char sp[SPEAKERLEN];
	char isWorFound;

	sp[0] = EOS;
	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	isJoinNext = FALSE;
	isWorFound = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		for (i=0; utterance->line[i] != EOS; i++) {
			if (utterance->line[i] == '\t' || utterance->line[i] == '\n')
				utterance->line[i] = ' ';
		}
		if (utterance->speaker[0] == '*') {
			if (isWorFound == FALSE && spareTier1[0] != EOS) {
				strcpy(spareTier3, spareTier1);
				segmentMainWorTiers(sp, spareTier1, spareTier2, &isJoinNext, isWorFound);
				if (isJoinNext == FALSE)
					spareTier1[0] = EOS;
			}
			if (isJoinNext == FALSE) {
				if (spareTier1[0] != EOS) {
					printTiers(sp, spareTier1, NULL, isJoinNext, isWorFound);
					spareTier1[0] = EOS;
				}
			} else if (!uS.partcmp(utterance->speaker, sp, FALSE, FALSE)) {
				strcat(spareTier1, " &&&");
				printTiers(sp, spareTier1, spareTier2, isJoinNext, isWorFound);
				isJoinNext = FALSE;
			}
			if (isJoinNext == FALSE) {
				strcpy(sp, utterance->speaker);
				uS.remblanks(sp);
			}
			strcat(spareTier1, utterance->line);
			isWorFound = FALSE;
		} else if (uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
			isWorFound = TRUE;
			strcat(spareTier2, utterance->line);
			worHasSymbol(spareTier1, spareTier2);
			segmentMainWorTiers(sp, spareTier1, spareTier2, &isJoinNext, isWorFound);
			if (isJoinNext == FALSE)
				spareTier1[0] = EOS;
		} else {
			isWorFound = TRUE;
			if (spareTier1[0] != EOS) {
				segmentMainWorTiers(sp, spareTier1, spareTier2, &isJoinNext, isWorFound);
				if (isJoinNext == FALSE)
					spareTier1[0] = EOS;
			}
			uS.remblanks(utterance->speaker);
			uS.remblanks(utterance->line);
			printout(utterance->speaker, utterance->line, NULL, NULL, TRUE);
		}
	}
}

void init(char f) {
	if (f) {
		isFTime = TRUE;
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		isGuesstimateBullet = FALSE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (isFTime) {
			onlydata = 1;
			isFTime = FALSE;
		}
	}
}

void usage() {
	printf("segment tiers at &&& symbol\n");
	printf("Usage: segment [%s] filename(s)\n",mainflgs());
	puts("+a : if no %wor tiers, then guesstimate bullet time (default: keep original bullets)");
	mainusage(FALSE);
	puts("When you put break up symbol in the original file, ");
	puts("    you have to add appropriate delimiter before the breakup symbol.");
	puts("When you put join symbol in the original file, ");
	puts("    you have to remove the delimiter before the join symbol.");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SEGMENT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			isGuesstimateBullet = TRUE;
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
