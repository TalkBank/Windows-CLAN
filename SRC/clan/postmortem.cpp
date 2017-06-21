/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"
#ifndef UNX
	#include "ced.h"
#else
	#define RGBColor int
	#include "c_curses.h"
#endif

#if !defined(UNX)
#define _main postmortem_main
#define call postmortem_call
#define getflag postmortem_getflag
#define init postmortem_init
#define usage postmortem_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define RULESFNAME "postmortem.cut"
#define PERIOD 50

extern struct tier *defheadtier;
extern char OverWriteFile;

struct PATS {
    char *pat;
	char isNeg;
	int  si, li;
    struct PATS *nextpat;
} ;

static struct rules {
	struct PATS *from;
	struct PATS *fromI;
	char        isMatch;
	struct PATS *to;
	struct rules *next;
} *head;

static char AutoMode, isTierSpecified;
static int postmortem_FirstTime;
static FNType rulesFName[FNSize];
static AttTYPE orgAtt[UTTLINELEN+2];

static void postmortem_overflow() {
	fprintf(stderr,"postmortem: no more memory available.\n");
	cutt_exit(1);
}

static struct PATS *freePATS(struct PATS *ptr) {
	struct PATS *t;

	while (ptr != NULL) {
		t = ptr;
		ptr = ptr->nextpat;
		if (t->pat)
			free(t->pat);
		free(t);
	}
	return(ptr);
}

static void postmortem_cleanup() {
	struct rules *t;

	while (head != NULL) {
		t = head;
		head = head->next;
		t->from = freePATS(t->from);
		t->to = freePATS(t->to);
		free(t);
	}
}

static struct PATS *makePats(char *st, char isChangeCaps) {
	char *isAddStar, isNeg;
	long i;
	struct PATS *root, *p;

	i = 0L;
	root = NULL;
	while (1) {
		if (isSpace(*st) || *st == EOS) {
			templineC1[i] = EOS;
			if (isChangeCaps)
				uS.lowercasestr(templineC1, &dFnt, MBF);
			if (i > 0) {
				isNeg = FALSE;
				if (root == NULL) {
					if ((p=NEW(struct PATS)) == NULL)
						postmortem_overflow();
					root = p;
				} else {
					if ((p->nextpat=NEW(struct PATS)) == NULL)
						postmortem_overflow();
					p = p->nextpat;
				}
				if (templineC1[0] == '!') {
					isNeg = TRUE;
					strcpy(templineC1, templineC1+1);
				}
				i = strlen(templineC1);
				if (!strcmp(templineC1, "$b") || !strcmp(templineC1, "$e") || templineC1[i-1] == '*')
					isAddStar = templineC1;
				else
					isAddStar = strchr(templineC1, '|');
				if (isAddStar == NULL)
					i = 3L;
				else
					i = 1L;
				p->pat = (char *)malloc(strlen(templineC1)+i);
				if (p->pat == NULL) 
					postmortem_overflow();
				strcpy(p->pat, templineC1);
				if (isAddStar == NULL)
					strcat(p->pat, "|*");
				p->isNeg = isNeg;
				p->nextpat = NULL;
			}
			i = 0L;
		} else
			templineC1[i++] = *st;
		if (*st == EOS)
			break;
		st++;
	}
	return(root);
}

static void makenewsym(char *rule) {
	long secondPart;
	struct rules *t;

	if (head == NULL) {
		head = NEW(struct rules);
		t = head;
	} else {
		t = head;
		while (t->next != NULL) t = t->next;
		t->next = NEW(struct rules);
		t = t->next;
	}
	if (t == NULL)
		postmortem_overflow();
	t->from = NULL;
	t->fromI = NULL;
	t->isMatch = FALSE;
	t->to   = NULL;
	t->next = NULL;

	for (secondPart=0L; rule[secondPart] != EOS; secondPart++) {
		if (rule[secondPart] == '=' && rule[secondPart+1] == '>') {
			break;
		}
	}
	if (rule[secondPart] == EOS) {
		postmortem_cleanup();
		fprintf(stderr,"**** Error: missing 'to' part in rule:\n\t%s\n", rule);
		cutt_exit(0);
	}
	rule[secondPart] = EOS;
	t->from = makePats(rule, TRUE);
	t->fromI = t->from;
	t->isMatch = FALSE;
	t->to   = makePats(rule+secondPart+2, FALSE);
}

static void readdict(void) {
	long i;
	FILE *fdic;
	FNType mFileName[FNSize];

	if (rulesFName[0] == EOS) {
		uS.str2FNType(rulesFName, 0L, RULESFNAME);
//		if (access(rulesFName,0) != 0)
//			return;
	}
	if ((fdic=OpenGenLib(rulesFName,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", rulesFName, mFileName);
		cutt_exit(0);
	}
	while (fgets_cr(templineC, UTTLINELEN, fdic)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		for (i=0L; isSpace(templineC[i]); i++) ;
		if (i > 0L)
			strcpy(templineC, templineC+i);
		uS.remblanks(templineC);
		if (templineC[0] == '%' || templineC[0] == '#' || templineC[0] == EOS)
			continue;
		makenewsym(templineC);
	}
	fclose(fdic);
}

void init(char f) {
	if (f) {
		stout = FALSE;
#if !defined(UNX)
		AutoMode = 0;
#else
		AutoMode = 1;
#endif
		isTierSpecified = FALSE;
		*rulesFName = EOS;
		postmortem_FirstTime = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL) 
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		head = NULL;
		onlydata = 1;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		maininitwords();
		mor_initwords();
	} else if (postmortem_FirstTime) {
		if (!isTierSpecified) {
			fprintf(stderr,"Please use +t option to specify either %%mor or %%trn tier.\n");
			cutt_exit(0);
		}
		readdict();
		nomain = TRUE;
		postmortem_FirstTime = FALSE;
	}
}

void usage() {
#if !defined(UNX)
	printf("Usage: postmortem [aN cF %s] filename(s)\n", mainflgs());
	printf("+a : create files with ambiguous results (Default: disambiguate during program run)\n");
	printf("+a1: create files without ambiguous results, just new string replacements\n");
#else
	printf("Usage: postmortem [cF %s] filename(s)\n", mainflgs());
#endif
	printf("+cF: dictionary file. (Default %s)\n", RULESFNAME);
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(FALSE);
	puts("Dictionary file format: \"det adj v  => det n v\"");
	cutt_exit(0);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = POSTMORTEM;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	postmortem_cleanup();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			if (*f == EOS)
				AutoMode = 1;
			else
				AutoMode = 2;
			break;
		case 'c':
			if (*f)
				uS.str2FNType(rulesFName, 0L, getfarg(f,f1,i));
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
		case 't':
			if (*(f-2) == '+' && *f == '%') {
				isTierSpecified = TRUE;
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int skipDelims(int i) {
	register int  temp;

	if (chatmode && *utterance->speaker == '%') {
		if (uttline[i] == EOS)
			return(i);
		if (i > 0)
			i--;
		while (uttline[i] != EOS && uS.isskip(uttline,i,&dFnt,MBF) && !uS.isRightChar(uttline,i,'[',&dFnt,MBF)) {
			i++;
			if (uttline[i-1] == '<') {
				temp = i;
				for (i++; uttline[i] != '>' && uttline[i]; i++) {
					if (isdigit(uttline[i])) ;
					else if (uttline[i]== ' ' || uttline[i]== '\t' || uttline[i]== '\n') ;
					else if ((i-1 == temp+1 || !isalpha(uttline[i-1])) &&
							 uttline[i] == '-' && !isalpha(uttline[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(uttline[i-1])) &&
							 (uttline[i] == 'u' || uttline[i] == 'U') &&
							 !isalpha(uttline[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(uttline[i-1])) &&
							 (uttline[i] == 'w' || uttline[i] == 'W') &&
							 !isalpha(uttline[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(uttline[i-1])) &&
							 (uttline[i] == 's' || uttline[i] == 'S') &&
							 !isalpha(uttline[i+1])) ;
					else
						break;
				}
				if (uttline[i] == '>')
					i++;
				else
					i = temp;
			}
		}
	} else {
		while (uttline[i] != EOS && uS.isskip(uttline,i,&dFnt,MBF) && !uS.isRightChar(uttline,i,'[',&dFnt,MBF))
			 i++;
	}
	return(i);
}

static void make_new_to_str(char *old_pat, char *old_s, int si, int li, char *new_pat, char *new_s) {
    register int j, k;
    register int n, m;
    int t, end;

	*new_s = EOS;
    if (old_s[si] == EOS)
    	return;
    for (j=si, k=0; old_pat[k]; j++, k++) {
		if (old_pat[k] == '*') {	  /* wildcard */
			k++; t = j;
f1:
			while (j < li && (islower((unsigned char)old_s[j]) ? (char)toupper((unsigned char)old_s[j]) : old_s[j]) != 
						(islower((unsigned char)old_pat[k]) ? (char)toupper((unsigned char)old_pat[k]) : old_pat[k]))
				j++;
			end = j;
			if (j < li) {
	    		for (m=j+1, n=k+1; m < li && old_pat[n]; m++, n++) {
					if (old_pat[n] == '*')
						break;
					else if ((islower((unsigned char)old_s[m]) ? (char)toupper((unsigned char)old_s[m]) : old_s[m]) != 
					 (islower((unsigned char)old_pat[n]) ? (char)toupper((unsigned char)old_pat[n]) : old_pat[n])) {
		    			j++;
						goto f1;
					}
				}
				if (m < li && !old_pat[n]) {
					j++;
					goto f1;
				}
			}
			while (*new_pat != '*' && *new_pat != EOS)
				*new_s++ = *new_pat++;
			if (*new_pat != EOS) {
				new_pat++;
				while (t < end) {
					*new_s++ = old_s[t++];
				}
			}
			if (j == li || old_pat[k] == EOS)
				break;
		}
	}
	while (*new_pat != EOS)
		*new_s++ = *new_pat++;
	*new_s = EOS;
}

static char my_equal_items(char orgC, char lineC) {
	if (isSpace(orgC) || orgC == '\n' || isSpace(lineC) || lineC == '\n')
		return(FALSE);
	if (orgC == lineC)
		return(TRUE);
	return(FALSE);
}

static char my_equal_space(char orgC, char lineC) {
	if ((isSpace(orgC) || orgC == '\n') && (isSpace(lineC) || lineC == '\n'))
		return(TRUE);
	return(FALSE);
}

static void createQueryUlternatives(char *org, char *line, char *qt) {
	int lineI, tLineI, orgI, tOrgI, q;

	lineI = 0;
	orgI = 0;
	strcpy(qt, "Do replace? ");
	q = strlen(qt);
	while (1) {
		while (isSpace(line[lineI]) || line[lineI] == '\n')
			lineI++;
		while (isSpace(org[orgI]) || org[orgI] == '\n')
			orgI++;
		if (line[lineI] == EOS && org[orgI] == EOS)
			break;
		else if (line[lineI] != EOS && org[orgI] == EOS) {
			postmortem_cleanup();
			fprintf(stderr,"**** Error: extra items on postmortem tier found:\n");
			fprintf(stderr,"org: %s\n", org);
			fprintf(stderr,"new: %s\n", line);
			cutt_exit(0);
		} else if (line[lineI] == EOS && org[orgI] != EOS) {
			postmortem_cleanup();
			fprintf(stderr,"**** Error: extra items on original tier found:\n");
			fprintf(stderr,"org: %s\n", org);
			fprintf(stderr,"new: %s\n", line);
			cutt_exit(0);
		}
		tLineI = lineI;
		tOrgI = orgI;
		while (my_equal_items(org[orgI], line[lineI])) {
			lineI++; orgI++;
		}
		if (!my_equal_space(org[orgI], line[lineI])) {
			orgI = tOrgI;
#if !defined(UNX)
			uS.shiftright(org+orgI, 2);
			org[orgI++] = ATTMARKER;
			org[orgI++] = error_start;
#endif
			qt[q++] = '"';
			while (!isSpace(org[orgI]) && org[orgI] != '\n' && org[orgI] != EOS) {
				qt[q++] = org[orgI];
				orgI++;
			}
#if !defined(UNX)
			uS.shiftright(org+orgI, 2);
			org[orgI++] = ATTMARKER;
			org[orgI++] = error_end;
#endif
			strcpy(qt+q, "\" with \"");
			q = strlen(qt);
			lineI = tLineI;
			while (!isSpace(line[lineI]) && line[lineI] != '\n' && line[lineI] != EOS) {
				qt[q++] = line[lineI];
				lineI++;
			}
			qt[q++] = '"';
		}
	}
	qt[q] = EOS;
}

static int replaceItems(struct rules *t, char *line, AttTYPE *atts, long *isFound) {
	int i, j, k, lI, next;
	char frSPCFound, toSPCFound;
	struct PATS *to, *fr;

	i = 0;
	j = 0;
	lI = 0;
	to = t->to;
	for (fr=t->from; fr != NULL; fr=fr->nextpat) {
		for (i=lI; i < fr->si; i++) {
			templineC1[j] = line[i];
			tempAtt[j] = atts[i];
			j++;
		}
		frSPCFound = !strcmp(fr->pat, "$b") || !strcmp(fr->pat, "$e");
		toSPCFound = FALSE;
		if (to != NULL) {
			toSPCFound = !strcmp(to->pat, "$b") || !strcmp(to->pat, "$e");
			if ((frSPCFound && !toSPCFound) || (!frSPCFound && toSPCFound)) {
				postmortem_cleanup();
				fprintf(stderr,"**** Error: miss-matched symbols \"%s\" and \"%s\".\n", fr->pat, to->pat);
				cutt_exit(0);
			}
			if (!frSPCFound && !toSPCFound) {
				make_new_to_str(fr->pat, line, fr->si, fr->li, to->pat, templineC1+j);
				k = strlen(templineC1);
				for (; j < k; j++)
					tempAtt[j] = 0;
			}
			to = to->nextpat;
		} else if ((frSPCFound && !toSPCFound) || (!frSPCFound && toSPCFound)) {
			postmortem_cleanup();
			fprintf(stderr,"**** Error: unmatched symbol \"%s\".\n", fr->pat);
			cutt_exit(0);
		}

		lI = fr->li;
	}
	next = j;
	for (i=lI; line[i] != EOS; i++) {
		templineC1[j] = line[i];
		tempAtt[j] = atts[i];
		j++;
	}
	templineC1[j] = EOS;

#if !defined(UNX)
	if (AutoMode == 0) {
		strcpy(templineC, line);
		createQueryUlternatives(templineC, templineC1, templineC2);
		fprintf(stderr,"----------------------------------------------\n");
		fprintf(stderr, "%s%s", spareTier1, spareTier2);
		fprintf(stderr, "%s%s", utterance->speaker, templineC);
		i = QueryDialog(templineC2, 140);
		if (i == 1) {
			att_cp(0, line, templineC1, atts, tempAtt);
			*isFound = *isFound + 1;
		} else if (i == 0)
#ifdef UNX
			exit(1);
#else
			isKillProgram = 1;
#endif
	} else {
#endif
		*isFound = *isFound + 1;
		att_cp(0, line, templineC1, atts, tempAtt);
#if !defined(UNX)
	}
#endif
	return(next);
}

static long MatchedAndReplaced(void) {
	char isBeg, isInterrupted;
	char w[512];
	int  i, sI, lI;
	long isFound;
	struct rules *t;

	isFound = 0L;
	for (t=head; t != NULL; t=t->next) {
		if (t->fromI == NULL)
			continue;
		isInterrupted = FALSE;
		t->fromI = t->from;
		t->isMatch = FALSE;
		isBeg = TRUE;
		i = 0;
		while (uttline[i] != EOS) {
			for (; isSpace(uttline[i]); i++) ;
			sI = i;
			i = getword(utterance->speaker, uttline, w, NULL, i);
			if (uS.isskip(uttline,i-1,&dFnt,MBF) && !uS.isRightChar(uttline, i-1, ']', &dFnt, MBF))
				lI = i - 1;
			else
				lI = i;
			if (i == 0)
				break;
			if (!strncmp(utterance->line+i, "+...", 4) ||
				  !strncmp(utterance->line+i, "+/.", 3)  || !strncmp(utterance->line+i, "+//.", 4) ||
				  !strncmp(utterance->line+i, "+/?", 3)  || !strncmp(utterance->line+i, "+//?", 4))
				isInterrupted = TRUE;
			i = skipDelims(i);
			if (!strcmp(t->fromI->pat, "$b")) {
				if ((isBeg && !t->fromI->isNeg) || (!isBeg && t->fromI->isNeg)) {
					if (t->from == t->fromI)
						t->isMatch = TRUE;
					t->fromI->si = sI;
					t->fromI->li = lI;
					t->fromI = t->fromI->nextpat;
				} else
					t->fromI = t->from;
			}
			if (t->fromI != NULL) {
				if (uS.patmat(w, t->fromI->pat) && !t->fromI->isNeg) {
					if (t->from == t->fromI)
						t->isMatch = TRUE;
					t->fromI->si = sI;
					t->fromI->li = lI;
					t->fromI = t->fromI->nextpat;
				} else
					t->fromI = t->from;
				if (t->fromI != NULL) {
					if (!strcmp(t->fromI->pat, "$e") && !t->fromI->isNeg && !isBeg && uttline[i] == EOS && !isInterrupted) {
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextpat;
						if (t->from == t->fromI)
							t->isMatch = TRUE;
					}
				}
			}
			if (t->fromI == NULL && t->isMatch) {
				i = replaceItems(t, utterance->line, utterance->attLine, &isFound);
				if (isKillProgram)
					return(0);
				t->fromI = t->from;
				t->isMatch = FALSE;
				if (uttline != utterance->line) {
					strcpy(uttline,utterance->line);
					filterData(utterance->speaker,uttline);
				}				
//				break;
			}
			isBeg = FALSE;
		}
	}

	return(isFound);
}

static void addFullUlternatives(char *org, AttTYPE *orgAtts, char *line, AttTYPE *atts) {
	int i, ti, j, tj, k;

	i = 0;
	j = 0;
	k = 0;
	while (1) {
		while (isSpace(line[i]) || line[i] == '\n') {
			templineC1[k] = line[i];
			tempAtt[k] = atts[i];
			i++;
			k++;
		}
		while (isSpace(org[j]) || org[j] == '\n')
			j++;
		if (line[i] == EOS && org[j] == EOS)
			break;
		else if (line[i] != EOS && org[j] == EOS) {
			postmortem_cleanup();
			fprintf(stderr,"**** Error: extra items on postmortem tier found:\n");
			fprintf(stderr,"org: %s\n", org);
			fprintf(stderr,"new: %s\n", line);
			cutt_exit(0);
		} else if (line[i] == EOS && org[j] != EOS) {
			postmortem_cleanup();
			fprintf(stderr,"**** Error: extra items on original tier found:\n");
			fprintf(stderr,"org: %s\n", org);
			fprintf(stderr,"new: %s\n", line);
			cutt_exit(0);
		}
		ti = i;
		tj = j;
		while (my_equal_items(org[j], line[i])) {
			i++; j++;
		}
		if (my_equal_space(org[j], line[i])) {
			while (ti < i) {
				templineC1[k] = line[ti];
				tempAtt[k] = atts[ti];
				ti++;
				k++;
			}
		} else {
			j = tj;
			while (!isSpace(org[j]) && org[j] != '\n' && org[j] != EOS) {
				templineC1[k] = org[j];
				tempAtt[k] = orgAtts[j];
				k++;
				j++;
			}
			templineC1[k] = '^';
			tempAtt[k] = 0;
			k++;
			i = ti;
			while (!isSpace(line[i]) && line[i] != '\n' && line[i] != EOS) {
				templineC1[k] = line[i];
				tempAtt[k] = atts[i];
				i++;
				k++;
			}
		}
	}
	templineC1[k] = EOS;
	att_cp(0, line, templineC1, atts, tempAtt);
}

void call() {
	char isChanged;
	long isFound, res;

	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	isFound = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		isChanged = FALSE;
		if (checktier(utterance->speaker)) {
			if (utterance->speaker[0] == '*') {
				strcpy(spareTier1, utterance->speaker);
				strcpy(spareTier2, utterance->line);
			}
			if (utterance->speaker[0] == '%') {
				if (AutoMode == 1)
					att_cp(0, templineC, utterance->line, orgAtt, utterance->attLine);
				if ((res=MatchedAndReplaced()) != 0L) {
					isChanged = TRUE;
					isFound += res;
					if (AutoMode == 1)
						addFullUlternatives(templineC, orgAtt, utterance->line, utterance->attLine);
				}
				if (isKillProgram)
					return;
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,isChanged);
	}
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
}
