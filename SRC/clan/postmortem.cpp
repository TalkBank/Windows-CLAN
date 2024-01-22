/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*************************************
# Instructions:
# '~'	in the rule means clitic in the utterance is not treated as space, otherwise it is
# !	used before element refers to the whole element
# ,!	used withing element after "," refers only to that choice
# $-	means copy whatever search pattern matched in full and unchanged to the output
# (...)	means match choices within zero or any number of times; the "to" part must be ()
# [...]	means match choices within zero or only one time; the "to" part must be []
# pro:*|*,n|*,!pro:dem|*	means match "pro:*|" or "n|" once, but fail at "pro:dem|";
# 				the "to" part must be $-
# !part|*P*P	means match only if part|*P*P not found; the "to" part must be !-
# \	means the rule continues on the next line
# RULE starts with ^ means printout utterances that this rule matches
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
extern char isRecursive;

struct PAT {
	char *elem;
	struct PAT *nextElem;
} ;

struct PATS {
	char *pat;
	char *to;
	char isMultRep;
	int  si, li;
	struct PAT *negs;
	struct PATS *nextPat;
} ;

struct rules {
	struct PATS *from;
	struct PATS *fromI;
	struct PATS *fromNotNeg;
	char		isDebugRule;
	char        isMatch;
	char        isClitic;
	struct rules *nextRule;
} ;

static char AutoMode, isTierSpecified, isLookInWD;
static int postmortem_FirstTime;
static char postmortem_CliticPunctuation[50];
static struct rules *head;
static FNType rulesFName[FNSize];
static AttTYPE orgAtt[UTTLINELEN+2];

static void freePAT(struct PAT *p) {
	struct PAT *t;

	while (p != NULL) {
		t = p;
		p = p->nextElem;
		if (t->elem)
			free(t->elem);
		free(t);
	}
}

static struct PATS *freePATS(struct PATS *p) {
	struct PATS *t;

	while (p != NULL) {
		t = p;
		p = p->nextPat;
		if (t->pat)
			free(t->pat);
		if (t->to)
			free(t->to);
		if (t->negs)
			freePAT(t->negs);
		free(t);
	}
	return(p);
}

static void postmortem_cleanup() {
	struct rules *t;

	while (head != NULL) {
		t = head;
		head = head->nextRule;
		t->from = freePATS(t->from);
		free(t);
	}
}

static void postmortem_overflow() {
	fprintf(stderr,"postmortem: no more memory available.\n");
	postmortem_cleanup();
	cutt_exit(0);
}

static struct PAT *addNegs(char *elem) {
	struct PAT *root, *p;
	char *s, *e;

	root = NULL;
	s = elem;
	do {
		e = strchr(s, ',');
		if (e != NULL)
			*e = EOS;
		if (s[0] != EOS) {
			if (root == NULL) {
				root = NEW(struct PAT);
				p = root;
			} else {
				for (p=root; p->nextElem != NULL; p=p->nextElem) ;
				p->nextElem = NEW(struct PAT);
				p = p->nextElem;
			}
			if (p == NULL)
				postmortem_overflow();
			p->nextElem = NULL;
			if (*s == '!')
				s++;
			p->elem = (char *)malloc(strlen(s)+1);
			if (p->elem == NULL)
				postmortem_overflow();
			strcpy(p->elem, s);
		}
		if (e != NULL) {
			*e = ',';
			s = e + 1;
		}
	} while (e != NULL) ;
	return(root);
}

static struct PATS *makeFromPats(char *fname, long ln, char *st) {
	char isNeg, isMultRep;
	int  lastc;
	long i;
	struct PATS *root, *p = NULL;

	i = 0L;
	root = NULL;
	while (1) {
		if (isSpace(*st) || *st == EOS) {
			templineC1[i] = EOS;
			if (i > 0) {
				if (root == NULL) {
					root = NEW(struct PATS);
					p = root;
				} else if (p != NULL) {
					p->nextPat = NEW(struct PATS);
					p = p->nextPat;
				}
				if (p == NULL)
					postmortem_overflow();
				p->pat = NULL;
				p->isMultRep = 0;
				p->to = NULL;
				p->negs = NULL;
				p->nextPat = NULL;
				if (templineC1[0] != EOS) {
					isNeg = FALSE;
					isMultRep = 0;
					if (templineC1[0] == '!' && templineC1[1] != EOS) {
						isNeg = TRUE;
						strcpy(templineC1, templineC1+1);
					}
					lastc = strlen(templineC1) - 1;
					if ((templineC1[0] == '(' && templineC1[lastc] == ')') ||
						(templineC1[0] == '[' && templineC1[lastc] == ']')) {
						if (templineC1[0] == '[')
							isMultRep = 1;
						else
							isMultRep = 2;
						templineC1[lastc] = EOS;
						strcpy(templineC1, templineC1+1);
					}
					if (isNeg) {
						if (isMultRep != 0) {
							fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
							fprintf(stderr,"Error: illegal use of (%s) or [%s] with negative \"!\" marker: %s\n", templineC1, templineC1, templineC1);
							postmortem_cleanup();
							cutt_exit(0);
						}
						p->negs = addNegs(templineC1);
					} else {
						if (p->pat != NULL) {
							fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
							fprintf(stderr,"Error: \"from\" pat already exists \"%s\"\n", p->pat);
							postmortem_cleanup();
							cutt_exit(0);
						}
						p->isMultRep = isMultRep;
						p->pat = (char *)malloc(strlen(templineC1)+1);
						if (p->pat == NULL)
							postmortem_overflow();
						strcpy(p->pat, templineC1);
					}
				}
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

static char isPatChoice(char *pat) {
	char *s, *e;

	s = pat;
	do {
		e = strchr(s, ',');
		if (e != NULL) {
			s = e + 1;
			if (*s != '!')
				return(TRUE);
		}
	} while (e != NULL) ;
	return(FALSE);
}

static void addTos(char *fname, long ln, struct PATS *from, char *st) {
	char isSkip;
	int  lastc;
	long i;

	i = 0L;
	while (1) {
		if (isSpace(*st) || *st == EOS) {
			templineC1[i] = EOS;
			if (i > 0) {
				isSkip = FALSE;
				if (templineC1[0] == '!' && templineC1[1] != EOS) {
					isSkip = TRUE;
				} else {
					lastc = strlen(templineC1) - 1;
					if ((templineC1[0] == '(' && templineC1[lastc] == ')') ||
						(templineC1[0] == '[' && templineC1[lastc] == ']')){
						isSkip = TRUE;
					}
				}
				if (!isSkip) {
					if (from == NULL) {
						fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
						fprintf(stderr,"Error: Unknown error in the rules file\n");
						postmortem_cleanup();
						cutt_exit(0);
					}
					while (from->isMultRep || from->pat == NULL) {
						from = from->nextPat;
						if (from == NULL)
							break;
					}
					if (from == NULL) {
						fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
						fprintf(stderr,"Error: Unknown error in the rules file\n");
						postmortem_cleanup();
						cutt_exit(0);
					}
					if (strchr(templineC1, ',') != NULL) {
						fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
						fprintf(stderr,"Error: illegal use of choices \",\" in \"to\" part: %s\n", templineC1);
						postmortem_cleanup();
						cutt_exit(0);
					}
					from->to = (char *)malloc(strlen(templineC1)+1);
					if (from->to == NULL)
						postmortem_overflow();
					strcpy(from->to, templineC1);
					if (isPatChoice(from->pat) && strcmp(from->to, "$-") != 0) {
						fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
#ifndef UNX
						fprintf(stderr,"Error: illegal use of \"from\" %c%c%s%c%c choices with literal \"to\" %c%c%s%c%c, (use $-)\n",
								ATTMARKER, error_start, from->pat, ATTMARKER, error_end, ATTMARKER, error_start, from->to, ATTMARKER, error_end);
#else
						fprintf(stderr,"Error: illegal use of \"from\" \"%s\" choices with non-literal \"to\" \"%s\", (use $-)\n",
								from->pat, from->to);
#endif
						postmortem_cleanup();
						cutt_exit(0);
					}
					from = from->nextPat;
				}
			}
			i = 0L;
		} else
			templineC1[i++] = *st;
		if (*st == EOS)
			break;
		st++;
	}
}

static void makenewsym(char *fname, long ln, char *rule) {
	short offset;
	long secondPart;
	struct rules *t;

	if (head == NULL) {
		head = NEW(struct rules);
		t = head;
	} else {
		t = head;
		while (t->nextRule != NULL) t = t->nextRule;
		t->nextRule = NEW(struct rules);
		t = t->nextRule;
	}
	if (t == NULL)
		postmortem_overflow();
	t->from = NULL;
	t->fromI = NULL;
	t->fromNotNeg = NULL;
	if (rule[0] == '^') {
		t->isDebugRule = TRUE;
		rule++;
	} else
		t->isDebugRule = FALSE;
	t->isMatch = FALSE;
	t->isClitic = FALSE;
	t->nextRule = NULL;

	offset = 0;
	for (secondPart=0L; rule[secondPart] != EOS; secondPart++) {
		if (rule[secondPart] == '=' && rule[secondPart+1] == '>') {
			offset = 2;
			break;
		} else if (rule[secondPart] == '=' && rule[secondPart+1] == '=' && rule[secondPart+2] == '>') {
			offset = 3;
			break;
		}
	}
	if (rule[secondPart] == EOS) {
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr,"Error: missing 'to' part in rule:\n\t%s\n", rule);
		postmortem_cleanup();
		cutt_exit(0);
	}
	rule[secondPart] = EOS;
	if (strchr(rule, '~') != NULL)
		t->isClitic = TRUE;
	if (rule[0] == '~' && rule[1] == ' ')
		t->from = makeFromPats(fname, ln, rule+2);
	else
		t->from = makeFromPats(fname, ln, rule);
	t->fromI = t->from;
	for (t->fromNotNeg=t->from; t->fromNotNeg != NULL; t->fromNotNeg=t->fromNotNeg->nextPat) {
		if (t->fromNotNeg->negs == NULL)
			break;
	}
	if (t->fromNotNeg == NULL) {
		t->fromNotNeg = t->from;
	}
	t->isMatch = FALSE;
	addTos(fname, ln, t->from, rule+secondPart+offset);
}

static void readdict(void) {
	int i, ln;
	FILE *fdic;
	FNType mFileName[FNSize];

	if (rulesFName[0] == EOS) {
		uS.str2FNType(rulesFName, 0L, RULESFNAME);
//		if (access(rulesFName,0) != 0)
//			return;
	}
// 2019-04-04 looking in current working
	if (isLookInWD) {
		strcpy(mFileName, wd_dir);
		addFilename2Path(mFileName, rulesFName);
		fdic = fopen(mFileName, "r");
	} else
		fdic = NULL;

	if (fdic == NULL) {
		if ((fdic=OpenMorLib(rulesFName,"r",TRUE,TRUE,mFileName)) == NULL) {
			fputs("Can't open either one of the changes files:\n",stderr);
			fprintf(stderr,"\t\"%s\", \"%s\"\n", rulesFName, mFileName);
			cutt_exit(0);
		}
	}
	fprintf(stderr,"    Using rules file: %s\n", mFileName);
	ln = 0;
	templineC[0] = EOS;
	while (fgets_cr(spareTier3, UTTLINELEN, fdic)) {
		if (uS.isUTF8(spareTier3) || uS.isInvisibleHeader(spareTier3))
			continue;
		ln++;
		for (i=0; isSpace(spareTier3[i]); i++) ;
		if (i > 0)
			strcpy(spareTier3, spareTier3+i);
		uS.remblanks(spareTier3);
		if (spareTier3[0] == EOS) {
			if (templineC[0] != EOS) {
				uS.remblanks(templineC);
				if (templineC[0] != '%' && templineC[0] != '#')
					makenewsym(mFileName, ln, templineC);
				templineC[0] = EOS;
			}
			continue;
		}
		i = strlen(spareTier3) - 1;
		if (spareTier3[i] == '\\') {
			spareTier3[i] = EOS;
			uS.remblanks(spareTier3);
			strcat(templineC, spareTier3);
			strcat(templineC, " ");
		} else {
			strcat(templineC, spareTier3);
			uS.remblanks(templineC);
			if (templineC[0] != '%' && templineC[0] != '#')
				makenewsym(mFileName, ln, templineC);
			templineC[0] = EOS;
		}
	}
	fclose(fdic);
}

void init(char f) {
	long i;

	if (f) {
		stout = FALSE;
		AutoMode = 2;
		isLookInWD = FALSE;
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
//		maininitwords();
//		mor_initwords();
		addword('\0','\0',"+bq|*"); //2019-04-29
		addword('\0','\0',"+eq|*"); //2019-04-29
		addword('\0','\0',"+[- *]");
	} else if (postmortem_FirstTime) {
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == '!' ||
				GlobalPunctuation[i] == '?' ||
				GlobalPunctuation[i] == '.')
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else
				i++;
		}
		if (!isTierSpecified)
			maketierchoice("%mor:",'+',FALSE);
		readdict();
		nomain = TRUE;
		postmortem_FirstTime = FALSE;
		strcpy(postmortem_CliticPunctuation, GlobalPunctuation);
		strcat(postmortem_CliticPunctuation, "~");
	}
}

void usage() {
#if !defined(UNX)
	printf("Usage: postmortem [aN cF %s] filename(s)\n", mainflgs());
	printf("+a : create files with ambiguous results (Default: create files without ambiguous results)\n");
	printf("+a1: disambiguate interactively by hand during program run\n");
	printf("+a2: create output with all changes clearly marked in red color\n");
#else
	printf("Usage: postmortem [cF %s] filename(s)\n", mainflgs());
	printf("+a : create files with ambiguous results (Default: create files without ambiguous results)\n");
	printf("+a2: create output with all changes clearly marked\n");
#endif
	printf("+cF: dictionary file. (Default %s)\n", RULESFNAME);
#ifdef UNX
	puts("+LF: specify full path F of the mor lib folder");
#endif
	puts("+p1: look for \"postmortem.cut\" in working directory first");
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
	replaceFile = TRUE;
	bmain(argc,argv,NULL);
	postmortem_cleanup();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			if (*f == EOS)
				AutoMode = 1;
			else if (*f == '1')
				AutoMode = 0;
			else if (*f == '2')
				AutoMode = 3;
			break;
		case 'c':
			if (*f == EOS)
				uS.str2FNType(rulesFName, 0L, RULESFNAME);
			else
				uS.str2FNType(rulesFName, 0L, getfarg(f,f1,i));
			break;
		case 'd':
			if (*f != '1') {
				maingetflag(f-2,f1,i);
			}
			break;
#ifdef UNX
		case 'L':
			int len;
			strcpy(mor_lib_dir, f);
			len = strlen(mor_lib_dir);
			if (len > 0 && mor_lib_dir[len-1] != '/')
				strcat(mor_lib_dir, "/");
			break;
#endif
		case 'p':
			if (*f == '1') {
				isLookInWD = TRUE;
			}
			break;
		case 's':
			if (*f == '[' && *(f+1) == '-') {
				maingetflag(f-2,f1,i);
/* enabled 2017-12-04, disabled 2018-04-04
			} else if (*f == '[' && *(f+1) == '+') {
				maingetflag(f-2,f1,i);
*/
			} else {
				fprintf(stderr, "Please specify only language codes, \"[- ...]\", with +/-s option.\n");
				cutt_exit(0);
			}
			break;
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

static void make_new_to_str(char *old_patF, char *old_s, int si, int li, char *new_pat, char *new_s) {
    register int j, k;
    register int n, m;
    int  t, end;
	char *comma, *old_pat;

	*new_s = EOS;
	if (old_s[si] == EOS)
		return;
	if (strcmp(new_pat, "$-") == 0) {
		for (j=si; j < li; j++) {
			*new_s++ = old_s[j];
		}
		*new_s = EOS;
		return;
	}

	strcpy(templineC2, old_patF);
	old_pat = templineC2;
	comma = strchr(old_pat, ',');
	if (comma != NULL)
		*comma = EOS;
    for (j=si, k=0; old_pat[k]; j++, k++) {
		if (old_pat[k] == '*') {	  /* wildcard */
			k++; t = j;
f1:
			while (j < li && old_s[j] != old_pat[k])
				j++;
			end = j;
			if (j < li) {
	    		for (m=j+1, n=k+1; m < li && old_pat[n]; m++, n++) {
					if (old_pat[n] == '*')
						break;
					else if (old_s[m] != old_pat[n]) {
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
	int lineI, tLineI, orgI, tOrgI;

	lineI = 0;
	orgI = 0;
	strcpy(qt, "Replace ORIG tier with NEW tier? ");
//	q = strlen(qt);
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
			lineI = tLineI;
#if !defined(UNX)
			uS.shiftright(org+orgI, 2);
			org[orgI++] = ATTMARKER;
			org[orgI++] = error_start;
			uS.shiftright(line+lineI, 2);
			line[lineI++] = ATTMARKER;
			line[lineI++] = error_start;
#endif
//			qt[q++] = '"';
			while (!isSpace(org[orgI]) && org[orgI] != '\n' && org[orgI] != EOS) {
//				qt[q++] = org[orgI];
				orgI++;
			}
//			strcpy(qt+q, "\" with \"");
//			q = strlen(qt);
			while (!isSpace(line[lineI]) && line[lineI] != '\n' && line[lineI] != EOS) {
//				qt[q++] = line[lineI];
				lineI++;
			}
//			qt[q++] = '"';
#if !defined(UNX)
			uS.shiftright(org+orgI, 2);
			org[orgI++] = ATTMARKER;
			org[orgI++] = error_end;
			uS.shiftright(line+lineI, 2);
			line[lineI++] = ATTMARKER;
			line[lineI++] = error_end;
#endif
		}
	}
//	qt[q] = EOS;
}

static int replaceItems(struct rules *t, char *line, int i, long *isFound) {
	int j, lI, next;
	char frSPCFound, toSPCFound;
	struct PATS *fr;

	i = 0;
	j = 0;
	lI = 0;
	for (fr=t->from; fr != NULL; fr=fr->nextPat) {
		if (fr->isMultRep != 0 || fr->pat == NULL) {
		} else {
			for (i=lI; i < fr->si; i++) {
				templineC1[j] = line[i];
				j++;
			}
			frSPCFound = !strcmp(fr->pat, "$b") || !strcmp(fr->pat, "$e");
			toSPCFound = FALSE;
			if (fr->to == NULL) {
				// error
			} else {
				toSPCFound = !strcmp(fr->to, "$b") || !strcmp(fr->to, "$e");
				if ((frSPCFound && !toSPCFound) || (!frSPCFound && toSPCFound)) {
					postmortem_cleanup();
					fprintf(stderr,"**** Error: miss-matched symbols \"%s\" and \"%s\".\n", fr->pat, fr->to);
					cutt_exit(0);
				}
				if (!frSPCFound && !toSPCFound) {
					make_new_to_str(fr->pat, line, fr->si, fr->li, fr->to, templineC1+j);
					j = strlen(templineC1);
				}
			}
			lI = fr->li;
		}
	}
	next = j;
	for (i=lI; line[i] != EOS; i++) {
		templineC1[j] = line[i];
		j++;
	}
	templineC1[j] = EOS;

#if !defined(UNX)
	if (AutoMode == 0 || AutoMode == 3) {
		strcpy(templineC, line);
		if (AutoMode == 0) {
			strcpy(spareTier3, templineC1);
			createQueryUlternatives(templineC, spareTier3, templineC2);
		} else {
			createQueryUlternatives(templineC, templineC1, templineC2);
			strcpy(spareTier3, templineC1);
		}
		uS.remblanks(templineC);
		uS.remblanks(spareTier3);
		fprintf(stderr,"----------------------------------------------\n");
		fprintf(stderr, "%s%s", spareTier1, spareTier2);
		fprintf(stderr, "ORIG: %s%s\n", utterance->speaker, templineC);
		fprintf(stderr, "NEW : %s%s\n", utterance->speaker, spareTier3);
		if (AutoMode == 0)
			i = QueryDialog(templineC2, 140);
		else if (AutoMode == 3)
			i = 1;
		if (i == 1) {
			strcpy(line, templineC1);
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
		strcpy(line, templineC1);
#if !defined(UNX)
	}
#endif
	return(next);
}

static char isEndOfTier(char *line, int i) {
	if (line[i] == EOS || uS.IsUtteranceDel(line, i))
		return(TRUE);
	else if (line[i] == '+') {
		for (i++; !uS.isskip(line,i,&dFnt,MBF) && line[i] != EOS; i++) {
			if (uS.IsUtteranceDel(line, i))
				return(TRUE);
		}
	}
	return(FALSE);
}

static char multi_patmat(char *w, char *pat) {
	char *s, *e, isFound;

	strcpy(templineC2, pat);
	isFound = FALSE;
	s = templineC2;
	do {
		e = strchr(s, ',');
		if (e != NULL)
			*e = EOS;
		if (s[0] != EOS) {
			if (*s == '!') {
				if (uS.patmat(w, s+1)) {
					isFound = FALSE;
					if (e != NULL)
						*e = ',';
					break;
				}
			} else {
				if (uS.patmat(w, s))
					isFound = TRUE;
			}
		}
		if (e != NULL) {
			*e = ',';
			s = e + 1;
		}
	} while (e != NULL) ;
	return(isFound);
}

static long MatchedAndReplaced(void) {
	char isBeg, isInterrupted;
	char w[BUFSIZ+1];
	int  i, wi, cI, sI, lI;
	long isFound;
	struct rules *t;
	struct PAT *neg;

	isFound = 0L;
	for (t=head; t != NULL; t=t->nextRule) {
		if (t->fromI == NULL)
			continue;
		isInterrupted = FALSE;
		t->fromI = t->from;
		t->isMatch = FALSE;
		isBeg = TRUE;
		i = 0;
		if (t->isClitic)
			punctuation = GlobalPunctuation;
		else
			punctuation = postmortem_CliticPunctuation;
		while (uttline[i] != EOS) {
			for (; isSpace(uttline[i]) || uttline[i] == '\n' || (uttline[i] == '~' && !t->isClitic); i++) ;
			cI = i;
			i = getword(utterance->speaker, uttline, w, NULL, i);
			for (wi=0; w[wi] != '#' && w[wi] != EOS; wi++) ;
			if (w[wi] == '#')
				wi++;
			else
				wi = 0;
			sI = cI + wi;
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
			for (neg=t->fromI->negs; neg != NULL; neg=neg->nextElem) {
				if (!strcmp(neg->elem, "$b")) {
					if (isBeg) {
						t->fromI = t->from;
						break;
					}
				} else if (uS.patmat(w+wi, neg->elem)) {
					t->fromI = t->from;
					break;
				}
			}
			if (neg == NULL) {
repeatWord:
				if (t->fromI->pat != NULL && !strcmp(t->fromI->pat, "$b")) {
					if (isBeg) {
						if (t->fromI == t->fromNotNeg)
							t->isMatch = TRUE;
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextPat;
					} else
						t->fromI = t->from;
				}
				if (t->fromI != NULL) {
					if (t->fromI->pat != NULL && multi_patmat(w+wi, t->fromI->pat)) {
						if (t->fromI->isMultRep == 0) {
							if (t->fromI == t->fromNotNeg)
								t->isMatch = TRUE;
							t->fromI->si = sI;
							t->fromI->li = lI;
							t->fromI = t->fromI->nextPat;
						} else if (t->fromI->isMultRep == 1) {
							t->fromI = t->fromI->nextPat;
						}
					} else if (t->fromI->isMultRep != 0 && t->fromI->nextPat != NULL) {
						t->fromI = t->fromI->nextPat;
						if (t->fromI->isMultRep != 0) {
							goto repeatWord;
						} else if (t->fromI->pat != NULL && multi_patmat(w+wi, t->fromI->pat)) {
							if (t->fromI == t->fromNotNeg)
								t->isMatch = TRUE;
							t->fromI->si = sI;
							t->fromI->li = lI;
							t->fromI = t->fromI->nextPat;
						} else if (t->fromI->negs != NULL && t->fromI->pat == NULL) {
							for (neg=t->fromI->negs; neg != NULL; neg=neg->nextElem) {
								if (!strcmp(neg->elem, "$e")) {
									if (!isBeg && isEndOfTier(uttline, i) && !isInterrupted)
										break;
								} else if (uS.patmat(w+wi, neg->elem)) {
									break;
								}
							}
							if (neg == NULL) {
//								i = cI;
								t->fromI->li = lI;
								t->fromI = t->fromI->nextPat;
							} else {
								if (t->fromI != NULL && t->fromI != t->from)
									i = t->from->li;
								t->fromI = t->from;
								t->isMatch = FALSE;
							}
						} else {
							if (t->fromI != NULL && t->fromI != t->from)
								i = t->from->li;
							t->fromI = t->from;
							t->isMatch = FALSE;
						}
					} else if (t->fromI->negs != NULL && t->fromI->pat == NULL) {
						for (neg=t->fromI->negs; neg != NULL; neg=neg->nextElem) {
							if (!strcmp(neg->elem, "$e")) {
								if (!isBeg && isEndOfTier(uttline, i) && !isInterrupted)
									break;
							} else if (uS.patmat(w+wi, neg->elem)) {
								break;
							}
						}
						if (neg == NULL) {
//							i = cI;
							t->fromI->li = lI;
							t->fromI = t->fromI->nextPat;
						} else {
							if (t->fromI != NULL && t->fromI != t->from)
								i = t->from->li;
							t->fromI = t->from;
							t->isMatch = FALSE;
						}
					} else {
						if (t->fromI != NULL && t->fromI != t->from)
							i = t->from->li;
						t->fromI = t->from;
						t->isMatch = FALSE;
					}
					if (t->fromI != NULL) {
						for (neg=t->fromI->negs; neg != NULL; neg=neg->nextElem) {
							if (!strcmp(neg->elem, "$e") && (isBeg || isEndOfTier(uttline, i) || isInterrupted)) {
								break;
							}
						}
						if (neg == NULL && t->fromI->pat != NULL) {
							if (!strcmp(t->fromI->pat, "$e") && !isBeg && isEndOfTier(uttline, i) && !isInterrupted) {
								t->fromI->si = sI;
								t->fromI->li = lI;
								t->fromI = t->fromI->nextPat;
								if (t->fromI == t->fromNotNeg)
									t->isMatch = TRUE;
							}
						}
					}
				}
			}
			if (t->fromI == NULL && t->isMatch) {
				if (t->isDebugRule) {
					fprintf(stderr, "%s%s", utterance->speaker, utterance->line);
				}
				i = replaceItems(t, utterance->line, i, &isFound);
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

static void addFullAlternatives(char *org, AttTYPE *orgAtts, char *line, AttTYPE *atts) {
	int i, ti, j, tj, k;

	i = 0;
	j = 0;
	k = 0;
	while (1) {
		while (isSpace(line[i]) || line[i] == '\n') {
			templineC1[k] = line[i];
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
				ti++;
				k++;
			}
		} else {
			j = tj;
			while (!isSpace(org[j]) && org[j] != '\n' && org[j] != EOS) {
				templineC1[k] = org[j];
				k++;
				j++;
			}
			templineC1[k] = '^';
			k++;
			i = ti;
			while (!isSpace(line[i]) && line[i] != '\n' && line[i] != EOS) {
				templineC1[k] = line[i];
				i++;
				k++;
			}
		}
	}
	templineC1[k] = EOS;
	strcpy(line, templineC1);
}

void call() {
	char isPostCodeExclude;
	int  postRes;
	long isFound, res;

	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	isFound = 0L;
	isPostCodeExclude = TRUE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (checktier(utterance->speaker)) {
			if (utterance->speaker[0] == '*') {
				strcpy(spareTier1, utterance->speaker);
				strcpy(spareTier2, utterance->line);
				postRes = isPostCodeFound(utterance->speaker, utterance->line);
				isPostCodeExclude = (postRes == 5 || postRes == 1);
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			} else if (utterance->speaker[0] == '%' && !isPostCodeExclude) {
				if (AutoMode == 1)
					att_cp(0, templineC, utterance->line, orgAtt, utterance->attLine);
				if ((res=MatchedAndReplaced()) != 0L) {
					isFound += res;
					if (AutoMode == 1)
						addFullAlternatives(templineC, orgAtt, utterance->line, utterance->attLine);
					printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
				} else
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			} else
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		if (isKillProgram)
			return;
	}
#ifndef UNX
	if (isFound == 0L && fpout != stdout && !stout && !WD_Not_Eq_OD) {
// lxs 2019-03-15		if (!isRecursive)
			fprintf(stderr,"**- NO changes made in this file\n");
		if (!replaceFile) {
			fclose(fpout);
			fpout = NULL;
//			if (unlink(newfname))
//				fprintf(stderr, "Can't delete output file \"%s\".", newfname);
		}
	} else
#endif
	if (isFound > 0L) {
// lxs 2019-03-15		if (!isRecursive)
			fprintf(stderr,"**+ %ld changes made in this file\n", isFound);
	} else // lxs 2019-03-15 if (!isRecursive)
		fprintf(stderr,"**- NO changes made in this file\n");
}
