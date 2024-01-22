/**********************************************************************
	"Copyright 1990-2023 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 3
#include "cu.h"
#ifdef _WIN32
#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define CODE_I   1
#define CODE_DP  2
#define CODE_PW  3
#define CODE_WW  4
#define CODE_M   5
#define CODE_P   6
#define CODE_R   7
#define CODE_END 8

/*
[^ I3] -> &-um &-um &-um or &-um [x 3]
[^ i]  -> &-um
[^ dp] word -> ≠word
[^ pw3] word -> ↫w-w-w↫want

[^ ww2] want -> want [/] want [/] want
  [^ pw3][^ ww2] want -> ↫w-w-w↫want [/] want [/] want
[^ m3] cookies -> cookies [x 4]
[^ p3] <this part> [/?] this part -> <this part> [x 3]
	                                  <this part> [/] <this part> [/] this part
    [^ p2] <and then> [/?] [^ p1] <and then he> [/?] and then he falls into the ocean .
    <and then> [/] and then <and then he> [/] and then he falls into the ocean .
*/

extern struct tier *defheadtier;
extern char OverWriteFile;

static char isFTime;

static int isCode(int i, char *line, int *codeNum, int *val) {
	*codeNum = 0;
	for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == 'I' || line[i] == 'i') {
		if (line[i+1] == ']') {
			*val = 1;
			*codeNum = CODE_I;
		} else if (isdigit(line[i+1])) {
			*val = atoi(line+i+1);
			*codeNum = CODE_I;
		}
	} else if ((line[i] == 'd'   || line[i] == 'D')   &&
			   (line[i+1] == 'p' || line[i+1] == 'P') &&
			   line[i+2] == ']') {
		*val = 0;
		*codeNum = CODE_DP;
	} else if ((line[i] == 'p'   || line[i] == 'P') &&
			   (line[i+1] == 'w' || line[i+1] == 'W')) {
		if (isdigit(line[i+2])) {
			*val = atoi(line+i+2);
			*codeNum = CODE_PW;
		}
	} else if ((line[i] == 'w'   || line[i] == 'W') &&
			   (line[i+1] == 'w' || line[i+1] == 'W')) {
		if (isdigit(line[i+2])) {
			*val = atoi(line+i+2);
			*codeNum = CODE_WW;
		}
	} else if (line[i] == 'm'   || line[i] == 'M') {
		if (isdigit(line[i+1])) {
			*val = atoi(line+i+1);
			*codeNum = CODE_M;
		}
	} else if (line[i] == 'p'   || line[i] == 'P') {
		if (isdigit(line[i+1])) {
			*val = atoi(line+i+1);
			*codeNum = CODE_P;
		}
	} else if (line[i] == 'r'   || line[i] == 'R') {
		*val = 0;
		*codeNum = CODE_R;
	}
	if (*codeNum != 0) {
		while (line[i] != ']' && line[i] != EOS)
			i++;
		if (line[i] == EOS) {
			fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "ERROR: Found '[' without closing ']'\n");
			fprintf(stderr, "%s\n", line);
			cutt_exit(0);
		}
		i++;
		while (isSpace(line[i]) || line[i] == '\n')
			i++;
	}
	return(i);
}

static int findFirstNonCode(int i, char *line, char isSkipOverAB) {
	do {
		if (line[i] == ']')
			i++;
		for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
		if (line[i] == '[' && line[i+1] != '/') {
			for (; line[i] != ']' && line[i] != EOS; i++) ;
			if (line[i] == EOS) {
				fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "ERROR: Found '[' without closing ']'\n");
				fprintf(stderr, "%s\n", line);
				cutt_exit(0);
			}
		}
	} while (line[i] == ']') ;
	if (isSkipOverAB)
		for (; isSpace(line[i]) || line[i] == '\n' || line[i] == '<'; i++) ;
	else
		for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	return(i);
}

static int findEndOfItem(int i, char *line, char *itemCopy) {
	int j;

	for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	j = 0;

repeatFindEnd:
	if (line[i] == '[') {
		for (; line[i] != ']' && line[i] != EOS; i++) {
			if (itemCopy != NULL)
				itemCopy[j++] = line[i];
		}
		if (line[i] == EOS) {
			fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "ERROR: Found '[' without closing ']'\n");
			fprintf(stderr, "%s\n", line);
			cutt_exit(0);
		}
		if (itemCopy != NULL)
			itemCopy[j++] = line[i];
		if (line[i] != EOS)
			i++;
	} else if (line[i] == '<') {
		for (; line[i] != '>' && line[i] != EOS; i++) {
			if (itemCopy != NULL)
				itemCopy[j++] = line[i];
		}
		if (line[i] == EOS) {
			fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "ERROR: Found '<' without closing '>'\n");
			fprintf(stderr, "%s\n", line);
			cutt_exit(0);
		}
		if (itemCopy != NULL)
			itemCopy[j++] = line[i];
		if (line[i] != EOS)
			i++;
	} else {
		for (; line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '[' && line[i] != '<' && line[i] != '>' && line[i] != EOS; i++) {
			if (itemCopy != NULL)
				itemCopy[j++] = line[i];
		}
		if (line[i] != '>' && line[i] != EOS) {
			itemCopy[j++] = line[i];
			i++;
			if (line[i] == '[' && line[i+1] == ':' && line[i+2] == ' ') {
				goto repeatFindEnd;
			} else
				j--;
		}
	}
	if (itemCopy != NULL)
		itemCopy[j] = EOS;
	return(i);
}

static int insertString(int i, const char *str, char isAddSpace) {
	int e, se, addSpaceAfter, addSpaceBefore;

	for (e=i; isSpace(utterance->line[e]) || utterance->line[e] == '\n'; e++) ;
	if (utterance->line[e] == '[' && utterance->line[e+1] == '/' && utterance->line[e+2] == '?' && utterance->line[e+3] == ']') {
		e += 4;
	}
	se = strlen(str) - 1;
	if (i < e)
		att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
	if (!isSpace(str[se]) && utterance->line[i] != '>' && isAddSpace)
		addSpaceAfter = 1;
	else
		addSpaceAfter = 0;
	addSpaceBefore = 0;
	if (i > 0) {
		if (!isSpace(utterance->line[i-1]) && !isSpace(str[0]) && utterance->line[i-1] != '&' && utterance->line[i-1] != '<'  &&
			utterance->line[i-1] != '-'  && utterance->line[i-1] != '+'  && utterance->line[i-1] != ')')
			addSpaceBefore = 1;
	}
	att_shiftright(utterance->line+i, utterance->attLine+i, strlen(str)+addSpaceAfter+addSpaceBefore);
	if (addSpaceBefore != 0) {
		utterance->line[i] = ' ';
		utterance->attLine[i] = 0;
		i++;
	}
	for (e=0; str[e] != EOS; e++) {
		utterance->line[i] = str[e];
		utterance->attLine[i] = 0;
		i++;
	}
	if (addSpaceAfter != 0) {
		utterance->line[i] = ' ';
		utterance->attLine[i] = 0;
		i++;
	}
	return(i);
}

static void removeABFromEnd(char *st) {
	int i, ab;

	ab = 0;
	i = strlen(st) - 1;
	while (i >= 0) {
		if (st[i] == ']' && ab <= 0)
			break;
		if (st[i] == '<') {
			if (ab == 0)
				break;
			ab--;
			strcpy(st+i, st+i+1);
		} else if (st[i] == '>') {
			ab++;
			strcpy(st+i, st+i+1);
		} else
			i--;
	}
}

static char nextInRowCODE_R(int i) {
	int e;

	do {
		for (; isSpace(utterance->line[i]) || utterance->line[i] == '\n' || utterance->line[i] == '<' || utterance->line[i] == '>'; i++) ;
		if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
			e = i + 3;
			for (; isSpace(utterance->line[e]); i++) ;
			if (utterance->line[e] == 'r' || utterance->line[e] == 'R') {
				while (utterance->line[e] != ']' && utterance->line[e] != EOS)
					e++;
				if (utterance->line[e] == EOS)
					return(FALSE);
				for (e++; isSpace(utterance->line[e]) || utterance->line[e] == '\n'; e++) ;
				att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
				return(TRUE);
			} else {
				while (utterance->line[i] != ']' && utterance->line[i] != EOS)
					i++;
				if (utterance->line[i] == ']')
					i++;
			}
		} else
			break;
	} while (utterance->line[i] != EOS) ;
	return(FALSE);
}

void call() {
	int   i, e, codeNum, val;
	char c[2], isFound, isSlashQFound;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {

// [^ I#] [^ i]
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
					e = isCode(i+3, utterance->line, &codeNum, &val);
					if (codeNum == CODE_I) {
						att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
						i = findFirstNonCode(i, utterance->line, FALSE);
						templineC3[0] = EOS;
						if (val == 0)
							strcat(templineC3, "&-um ");
						else {
							for (; val > 0; val--)
								strcat(templineC3, "&-um ");
						}
						uS.remblanks(templineC3);
						i = insertString(i, templineC3, TRUE);
					} else
						i = e;
				} else
					i++;
			}

// [/?] [^ r] -> [//]
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[' && utterance->line[i+1] == '/' && utterance->line[i+2] == '?' && utterance->line[i+3] == ']') {
					e = findEndOfItem(i, utterance->line, NULL);
					if (nextInRowCODE_R(e)) {
						att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
						i = insertString(i, "[//]", TRUE);
					} else
						i = e;
				} else
					i++;
			}

// [^ r] along
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
					e = isCode(i+3, utterance->line, &codeNum, &val);
					if (codeNum == CODE_R) {
						att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
						i = insertString(i, "[//]", TRUE);
					} else
						i = e;
				} else
					i++;
			}

// [^ p#]
			do {
				isFound = FALSE;
				i = 0;
				while (utterance->line[i] != EOS) {
					if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
						e = isCode(i+3, utterance->line, &codeNum, &val);
						if (codeNum == CODE_P) {
							att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
							i = findFirstNonCode(i, utterance->line, FALSE);
							i = findEndOfItem(i, utterance->line, templineC2);
							templineC3[0] = EOS;
							for (; val > 2; val--) {
								strcat(templineC3, " [/] ");
								strcat(templineC3, templineC2);
							}
							strcat(templineC3, " [/]");
							i = insertString(i, templineC3, TRUE);
							isFound = TRUE;
						} else
							i = e;
					} else
						i++;
				}
			} while (isFound) ;

// [^ m#]
			do {
				isFound = FALSE;
				i = 0;
				while (utterance->line[i] != EOS) {
					if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
						e = isCode(i+3, utterance->line, &codeNum, &val);
						if (codeNum == CODE_M) {
							att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
							i = findFirstNonCode(i, utterance->line, FALSE);
							i = findEndOfItem(i, utterance->line, templineC2);
							isSlashQFound = FALSE;
							if (val == 1) {
								for (e=i; isSpace(utterance->line[e]) || utterance->line[e] == '\n'; e++) ;
								if (utterance->line[e] == '[' && utterance->line[e+1] == '/' && utterance->line[e+2] == '?' && utterance->line[e+3] == ']')
									isSlashQFound = TRUE;
							} else
								isSlashQFound = TRUE;
							if (!isSlashQFound) {
								strcpy(templineC3, " [/] ");
								strcat(templineC3, templineC2);
							} else {
								templineC3[0] = EOS;
								for (; val > 2; val--) {
									strcat(templineC3, " [/] ");
									strcat(templineC3, templineC2);
								}
								strcat(templineC3, " [/]");
							}
							i = insertString(i, templineC3, TRUE);
							isFound = TRUE;
						} else
							i = e;
					} else
						i++;
				}
			} while (isFound) ;

// [^ ww#]
			do {
				isFound = FALSE;
				i = 0;
				while (utterance->line[i] != EOS) {
					if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
						e = isCode(i+3, utterance->line, &codeNum, &val);
						if (codeNum == CODE_WW) {
							att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
							i = findFirstNonCode(i, utterance->line, FALSE);
							i = findEndOfItem(i, utterance->line, templineC2);
							templineC3[0] = EOS;
							for (; val > 0; val--) {
								strcat(templineC3, " [/] ");
								strcat(templineC3, templineC2);
							}
							removeABFromEnd(templineC3);
							i = insertString(i, templineC3, TRUE);
							isFound = TRUE;
						} else
							i = e;
					} else
						i++;
				}
			} while (isFound) ;

// [^ pw#]
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
					e = isCode(i+3, utterance->line, &codeNum, &val);
					if (codeNum == CODE_PW) {
						att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
						i = findFirstNonCode(i, utterance->line, TRUE);
						if (val > 0) {
							if (utterance->line[i] == '&') {
								i++;
								if (utterance->line[i] == '+' || utterance->line[i] == '-')
									i++;
							}
							if (utterance->line[i] == '(') {
								for (; utterance->line[i] != ')' && utterance->line[i] != EOS; i++) ;
								if (utterance->line[i] == EOS) {
									fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
									fprintf(stderr, "ERROR: Found '[' without closing ']'\n");
									fprintf(stderr, "%s\n", utterance->line);
									cutt_exit(0);
								}
								i++;
							}
							c[0] = utterance->line[i];
							c[1] = EOS;
							strcpy(templineC3, "↫");
							for (; val > 0; val--) {
								strcat(templineC3, c);
								strcat(templineC3, "-");
							}
							e = strlen(templineC3) - 1;
							templineC3[e] = EOS;
							strcat(templineC3, "↫");
							i = insertString(i, templineC3, FALSE);
						}
					} else
						i = e;
				} else
					i++;
			}

// [^ dp]
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[' && utterance->line[i+1] == '^' && utterance->line[i+2] == ' ') {
					e = isCode(i+3, utterance->line, &codeNum, &val);
					if (codeNum == CODE_DP) {
						att_cp(0, utterance->line+i, utterance->line+e, utterance->attLine+i, utterance->attLine+e);
						i = findFirstNonCode(i, utterance->line, TRUE);
						if (utterance->line[i] == '&') {
							i++;
							if (utterance->line[i] == '+' || utterance->line[i] == '-')
								i++;
						}
						i = insertString(i, "≠", FALSE);
					} else
						i = e;
				} else
					i++;
			}

			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,TRUE);
		} else
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}

void init(char f) {
	if (f) {
		isFTime = TRUE;
		onlydata = 1;
		stout = FALSE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".cha";
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
//		root_list = NULL;
	} else {
		if (isFTime) {
			isFTime = FALSE;
		}
	}
}

void usage() {
	printf("Convert IISrP fluency codes to CHAT fluency codes\n");
	printf("Usage: temp filename(s)\n");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
//	root_list = freeList(root_list);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

/*
#define MY_LIST struct my_lst
struct my_lst {
	char *word;
	struct my_lst *next_word;
} ;

static MY_LIST *root_list;

static MY_LIST *freeList(MY_LIST *p) {
	MY_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_word;
		if (t->word != NULL)
			free(t->word);
		free(t);
	}
	return(NULL);
}

static void IISRP_exit(int i) {
	root_list = freeList(root_list);
	cutt_exit(i);
}

static MY_LIST *addToCompList(MY_LIST *root, char *word) {
	MY_LIST *p;

	if (root == NULL) {
		if ((p=NEW(MY_LIST)) == NULL) {
			fprintf(stderr, "\nOut of memory.\n\n");
			IISRP_exit(0);
		}
		root = p;
	} else {
		for (p=root; p->next_word != NULL; p=p->next_word) ;
		p->next_word = NEW(MY_LIST);
		p = p->next_word;
	}
	if (p == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		IISRP_exit(0);
	}
	p->next_word = NULL;
	p->word = (char *)malloc(strlen(word)+1);
	if (p->word == NULL) {
		fprintf(stderr, "\nOut of memory.\n\n");
		IISRP_exit(0);
	}
	strcpy(p->word, word);
	return(root);
}

static char isWordComp(MY_LIST *p, char *word) {
	while (p != NULL) {
		if (uS.mStricmp(p->word, word) == 0)
			return(TRUE);
		p = p->next_word;
	}
	return(FALSE);
}
*/
