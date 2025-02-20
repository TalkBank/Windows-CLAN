/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main toCA_main
#define call toCA_call
#define getflag toCA_getflag
#define init toCA_init
#define usage toCA_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

#define MEM_SIZE 40000L

static char *mem;

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		AddCEXExtension = ".ca";
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage() {
	printf("Convert CHAT notation to CA specific notation\n");
	printf("Usage: chat2ca [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHAT2CA;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	mem = (char *)malloc(MEM_SIZE+5);
	if (mem == NULL)
		fprintf(stderr, "\tERROR: Out of memory\n");
	else {
		bmain(argc,argv,NULL);
		if (mem != NULL)
			free(mem);
	}
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void replaceDelimWithEqual(long b, long *e) {
	char isHiden;
	long c;

	if (mem[b] != '*')
		return;

	for (c=b; c < *e; c++) {
		if (mem[c] == '\n' && (mem[c+1] == '%' || mem[c+1] == '@'))
			break;
	}
	isHiden = FALSE;
	for (c--; c >= b; c--) {
		if (mem[c] == HIDEN_C)
			isHiden = !isHiden;
		else if (isHiden)
			;
		else if (!isSpace(mem[c]) && mem[c] != '\n')
			break;
	}
	c++;
	uS.shiftright(mem+c, 2);
	mem[c++] = ' ';
	mem[c] = '=';
	*e += 2;
}

static long chat2ca_FindScope(long b, char *line, long i) {
	if (!uS.isRightChar(line, i, '[', &dFnt, MBF) && i >= b) {
		for (; !uS.isRightChar(line, i, '[', &dFnt, MBF) && i >= b; i--) ;
	}
	if (i < b) {
		fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
		fprintf(stderr,"In tier on line: %ld.\n", lineno);
		fprintf(stderr,"text=%s\n",line);
		return(-2L);
	} else
		i--;
	while (!uS.isRightChar(line, i, '>', &dFnt, MBF) && uS.isskip(line,i,&dFnt,MBF) && i >= b) {
		if (uS.isRightChar(line, i, ']', &dFnt, MBF) || uS.isRightChar(line, i, '<', &dFnt, MBF))
			break;
		else
			i--;
	}
	if (uS.isRightChar(line, i, '>', &dFnt, MBF)) {
		while (!uS.isRightChar(line, i, '<', &dFnt, MBF) && i >= b) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF))
				i = chat2ca_FindScope(b, line,i);
			else
				i--;
		}
		if (i < b) {
			fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
			fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
			fprintf(stderr,"text=%s\n",line);
			return(-2L);
		} else
			i--;
	} else if (i < b) ;
	else if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
		i = chat2ca_FindScope(b, line,i);
	} else {
		while (!uS.isskip(line,i,&dFnt,MBF) && i >= b) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF))
				i = chat2ca_FindScope(b, line,i);
			else
				i--;
		}
	}
	return(i);
}

static long analyze(long *b, long e) {
	char isSPT, sq, isSLANT_UP, isSLANT_DOWN, isSTAR, isAddParans;
	int  matchType;
	long c, t, tt;

	if (mem[*b] != '*')
		return(e);

/* 2006-05-26
	for (c=*b; c < e; c++) {
		if (mem[c] == '.') {
			if (mem[c-2] == '+'	&& mem[c-1] == '+') { // rempove "++."
				c -= 2;
				strcpy(mem+c, mem+c+3);
				e -= 3;
			} else 
			if (mem[c+1] != '.' && mem[c-1] != '.' && mem[c-1] != '/' && mem[c-1] != '=') {
				if (isSpace(mem[c-1])) {
					c--;
					strcpy(mem+c, mem+c+2);
					e -= 2;
				} else {
					strcpy(mem+c, mem+c+1);
					e -= 1;
				}
			} else {
				for (; c < e && mem[c] == '.'; c++) ;
			}
		}
	}
*/

	sq = FALSE;
	isSPT = TRUE;
	isSTAR = FALSE;
	isSLANT_UP = FALSE;
	isAddParans = FALSE;
	isSLANT_DOWN = FALSE;
	for (c=*b; c < e; ) {
		if (mem[c] == '\n' && (mem[c+1] == '%' || mem[c+1] == '@')) {
			if (isAddParans) {
				t = c;
				do {
					for (; t >= *b && (mem[t] == '\n' || isSpace(mem[t])); t--) ;
					if (mem[t] == ')' && (isdigit(mem[t-1]) || mem[t-1] == '.') && t >= *b) {
						for (t--; t >= *b && (mem[t] == '.' || mem[t] == '(' || isdigit(mem[t])); t--) ;
					} else
						break;
				} while (1) ;
				t++;
				uS.shiftright(mem+t, 2);
				mem[t++] = ')';
				mem[t] = ')';
				e += 2;
				c += 2;
			}
			isAddParans = FALSE;
			isSPT = FALSE;
			sq = FALSE;
			c++;
			if (mem[c] == '%') {
				for (t=c; t < e && mem[t] != ':'; t++) ;
				if (mem[t] == ':') {
					for (t++; t < e && isSpace(mem[t]); t++) ;
					strcpy(mem+c, mem+t);
					e -= (t - c);
					isAddParans = TRUE;
					uS.shiftright(mem+c, 3);
					mem[c++] = '\t';
					mem[c++] = '(';
					mem[c++] = '(';
					e += 3;
				}
			}
				
		}
		if (isSPT) {
			if (mem[c] == '[') {
				sq = TRUE;
				if (mem[c+1] == '/' && mem[c+2] == ']') {
					strcpy(mem+c, mem+c+1);
					mem[c++] = '-';
					mem[c] = ' ';
					e -= 1;
					sq = FALSE;
				} else if (mem[c+1] == '/' && mem[c+2] == '/' && mem[c+3] == ']') {
					strcpy(mem+c, mem+c+2);
					mem[c++] = '-';
					mem[c] = ' ';
					e -= 2;
					sq = FALSE;
				} else if (mem[c+1] == '?' && mem[c+2] == ']') {
					t = chat2ca_FindScope(*b, mem, c);
					if (t != -2L) {
						strcpy(mem+c, mem+c+3);
						e -= 3;
						for (tt=c-1; tt >= *b && isSpace(mem[tt]); tt--) ;
						t++;
						if (mem[tt] == '>' && tt < c) {
							strcpy(mem+tt, mem+c);
							e -= (c - tt);
							c = tt;
							for (; t < c && isSpace(mem[t]); t++) ;
							if (mem[t] == '<')
								mem[t] = '(';
							else {
								uS.shiftright(mem+t, 1);
								mem[t] = '(';
								e += 1;
							}
						} else {
							uS.shiftright(mem+t, 1);
							mem[t] = '(';
							e += 1;
						}
						uS.shiftright(mem+c, 1);
						mem[c] = ')';
						e += 1;

						sq = FALSE;
					}
				} else if (mem[c+1] == '%' && mem[c+2] == ' ') {
					strcpy(mem+c, mem+c+1);
					e -= 1;
					mem[c++] = '(';
					mem[c++] = '(';
					for (; c < e && mem[c] != ']'; c++) ;
					if (c < e) {
						uS.shiftright(mem+c, 1);
						mem[c++] = ')';
						mem[c] = ')';
						e += 1;
					} else {
						uS.shiftright(mem+c, 2);
						mem[c++] = ')';
						mem[c] = ')';
						e += 2;
					}
					sq = FALSE;
				}
			} else if (mem[c] == ']')
				sq = FALSE;
			else if (sq)
				;
			else if (mem[c] == 'x' && mem[c+1] == 'x' && mem[c+2] == 'x' && uS.isskip(mem,c+3,&dFnt,MBF)) {
				mem[c++] = '(';
				mem[c++] = ' ';
				mem[c] = ')';
			} else if (mem[c] == 'w' && mem[c+1] == 'w' && mem[c+2] == 'w' && uS.isskip(mem,c+3,&dFnt,MBF)) {
				mem[c++] = '(';
				mem[c++] = ' ';
				mem[c] = ')';
			} else if (mem[c] == '&' && mem[c+1] == '=' && !uS.isskip(mem,c+2,&dFnt,MBF)) {
				mem[c++] = '(';
				mem[c++] = '(';
				for (t=c; t < e && !uS.isskip(mem,t,&dFnt,MBF); t++) {
					if (uS.HandleCAChars(mem+t, &matchType) != 0) {
						c--;
						break;
					}
				}
				uS.shiftright(mem+t, 2);
				mem[t++] = ')';
				mem[t] = ')';
				e += 2;
			} else if (mem[c] == '#') {
				for (t=c+1; t < e && (isdigit(mem[t]) || mem[t] == '_'); t++) {
					if (mem[t] == '_')
						mem[t] = '.';
				}
				if (t == c+1) {
					uS.shiftright(mem+c, 2);
					mem[c++] = '(';
					mem[c++] = '.';
					mem[c] = ')';
					e += 2;
				} else {
					uS.shiftright(mem+t, 1);
					mem[c] = '(';
					mem[t] = ')';
					c = t;
					e += 1;
				}
			} else if (mem[c] == '+') {
				if (mem[c+1] == '.' && mem[c+2] == '.' && mem[c+3] == '.') {
					strcpy(mem+c, mem+c+3);
					mem[c] = '-';
					e -= 3;
				} else if (mem[c+1] == '/' && mem[c+2] == '/' && mem[c+3] == '.') {
					strcpy(mem+c, mem+c+3);
					mem[c] = '-';
					e -= 3;
				} else if (mem[c+1] == '/' && mem[c+2] == '.') {
					strcpy(mem+c, mem+c+2);
					mem[c] = '-';
					e -= 2;
				} else if (mem[c+1] == '+' && mem[c+2] == '.') {
					strcpy(mem+c, mem+c+3);
					e -= 3;
				} else if (mem[c+1] == '.' && mem[c+2] != '.') {
					strcpy(mem+c, mem+c+2);
					e -= 3;
				} else if (mem[c+1] == '^') {
					replaceDelimWithEqual(0, b);
					c += 2; // includes side effect from "replaceDelimWithEqual"
					mem[c] = '=';
					strcpy(mem+c+1, mem+c+2);
					e += 1; // includes side effect from "replaceDelimWithEqual"
				}
			} else if ((t=(long)uS.HandleCAChars(mem+c, &matchType)) != 0) {
				t--;
				if (matchType == CA_APPLY_FASTER) {
					if (t > 0L) {
						strcpy(mem+c, mem+c+t);
						e -= t;
					}
					if (!isSLANT_UP)
						mem[c] = '>';
					else
						mem[c] = '<';
					isSLANT_UP = !isSLANT_UP;
				} else if (matchType == CA_APPLY_SLOWER) {
					if (t > 0L) {
						strcpy(mem+c, mem+c+t);
						e -= t;
					}
					if (!isSLANT_DOWN)
						mem[c] = '<';
					else
						mem[c] = '>';
					isSLANT_DOWN = !isSLANT_DOWN;
				} else if (matchType == CA_APPLY_INHALATION) {
					if (t > 0L) {
						strcpy(mem+c, mem+c+t);
						e -= t;
					}
					mem[c] = '.';
				} else if (matchType == CA_APPLY_LATCHING) {
					if (t > 0L) {
						strcpy(mem+c, mem+c+t);
						e -= t;
					}
					mem[c] = '=';
				} else if (matchType == CA_APPLY_CREAKY) {
					if (t > 0L) {
						strcpy(mem+c, mem+c+t);
						e -= t;
					}
					mem[c] = '*';
/* 2006-05-26 upside down question mark
				} else if (matchType == CA_APPLY_RISING) {
					for (tt=c-1; tt >= *b && (isSpace(mem[tt]) || mem[tt] == '\n'); tt--) ;
					if (mem[tt] == '?') {
						strcpy(mem+tt, mem+tt+1);
						e -= 1;
						c += t;
					} else {
						c = c + t + 1;
						for (tt=c; tt < e && (isSpace(mem[tt]) || mem[tt] == '\n'); tt++) ;
						if (mem[tt] == '?') {
							strcpy(mem+tt, mem+tt+1);
							e -= 1;
						}
					}
*/
				} else
					c += t;
			}
		} else {
			if (mem[c] == '#') {
				for (t=c+1; t < e && (isdigit(mem[t]) || mem[t] == '_'); t++) {
					if (mem[t] == '_')
						mem[t] = '.';
				}
				if (t == c+1) {
					uS.shiftright(mem+c, 2);
					mem[c++] = '(';
					mem[c++] = '.';
					mem[c] = ')';
					e += 2;
				} else {
					uS.shiftright(mem+t, 1);
					mem[c] = '(';
					mem[t] = ')';
					c = t;
					e += 1;
				}
			}
		}
		c++;
	}

	if (isAddParans) {
		t = c;
		do {
			for (; t >= *b && (mem[t] == '\n' || mem[t] == EOS || isSpace(mem[t])); t--) ;
			if (mem[t] == ')' && (isdigit(mem[t-1]) || mem[t-1] == '.') && t >= *b) {
				for (t--; t >= *b && (mem[t] == '.' || mem[t] == '(' || isdigit(mem[t])); t--) ;
			} else
				break;
		} while (1) ;
		t++;
		uS.shiftright(mem+t, 2);
		mem[t++] = ')';
		mem[t] = ')';
		e += 2;
	}

	return(e);
}

static void writeOut(long e) {
	long c;
	char t;

	t = mem[e];
	mem[e] = EOS;
	for (c=0L; mem[c] != EOS; c++) {
		if (mem[c-1] == '\n' && mem[c] == '*')
			strcpy(mem+c, mem+c+1);
	}
	if (mem[0] == '*')
		fputs(mem+1, fpout);
	else
		fputs(mem, fpout);
	mem[e] = t;
}

void call() {
	char isCR;
	long mi, i;
	long secSPCluster;

	isCR = FALSE;
	mi = 0L;
	secSPCluster = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fpin) != NULL) {
		for (i=0L; templineC[i] != EOS; i++) {
			if (templineC[i] == '*' && isCR) {
				mem[mi] = EOS;
				if (secSPCluster == 0L)
					mi = analyze(&secSPCluster, mi);
				else {
					mi = analyze(&secSPCluster, mi);
					writeOut(secSPCluster);
					strcpy(mem, mem+secSPCluster);
					mi = strlen(mem);
				}
				secSPCluster = mi;
			}
			if (mi >= MEM_SIZE) {
				fprintf(stderr, "ERROR: Out of INTERNAL PROGRAM memory.\n");
				free(mem);
				mem = NULL;
				cutt_exit(0);
			}
			mem[mi++] = templineC[i];
			if (templineC[i] == '\n')
				isCR = TRUE;
			else
				isCR = FALSE;
		}
	}
	mem[mi] = EOS;
	if (secSPCluster != 0)
		mi = analyze(&secSPCluster, mi);
	writeOut(mi);
}
