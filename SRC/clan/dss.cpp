/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#if !defined(UNX)
#include "ced.h"
#endif
#include "cu.h"
#include "dss.h"

#if !defined(UNX)
#define _main dss_main
#define call dss_call
#define getflag dss_getflag
#define init dss_init
#define usage dss_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#ifndef _MAX_FNAME
	#define _MAX_FNAME	256
#endif
#ifndef _MAX_PATH
	#define _MAX_PATH	2048
#endif

#define POINTS_NAME_TAG "#Points:"
#define POINTS_FORM "%-3s|"
#define POINTS_S 40
#define POINTNAME_LEN 20
#define RULE  struct rule_s
#define RULES struct rulte_s
#define DSS_MACH struct dss_bmachine
#define DSS_NMN  struct dss_nextmachnode

RULE {
	DSS_MACH *focus, *lcont, *rcont;
	char *action, *points;
	RULE *next_rlist;
} ;

RULES {
	char *name;
	RULE *rule_list;
	RULES *next_rule;
} ;

DSS_NMN {
	char neg;
	char isAliasAddress;
	DSS_MACH *tmac;
	DSS_NMN *nextmn;
} ;

DSS_MACH {
	char *pat;
	char neg;
	char wild;
	char skip;
	char *fmatch;
	int  lmatch;
	DSS_NMN *nextmac;
} ;

extern char OverWriteFile;
extern char outputOnlyData;
extern struct tier *defheadtier;
extern struct tier *headtier;

static char *INITSTACK;
static char *dss_mat, *MatCop;
static char *dss_expression;
static char DSSAutoMode;
static char dss_mferr;
static char dss_ftime;
static char PointsNames[POINTS_N][POINTNAME_LEN+2];
static char debug_level;
static char isDSSpostcode, isDSSEpostcode;
static char isFirstCall;
static int  IsOutputSpreadsheet, spNum;
static int  TotalsAcrossPoints[POINTS_N], TotalsTotal;
static int  PointsCnt;
static int  dss_firstmatch;
static float GGrandTotal;
static float GTotalNum;
static FNType debugDssName[FNSize];
static FILE *tfp;
static RULES *stack;
static RULES *root_rules;
static DSSSP dss_sp_head;

int  DSS_UTTLIM;
char dss_lang;
char isDssFileSpecified;
const char *rulesfile;
FILE *debug_dss_fp;

#ifndef KIDEVAL_LIB
void usage() {
	printf("Usage: dss [a cN dS lS %s] filename(s)\n",mainflgs());
	printf("+aN: debug dss rules level N (1-3).\n");
	printf("+cN: analyse N complete unique utterances. (default: 50 utterances)\n");
	printf("+d : outputs in SPREADSHEET format\n");
	printf("+d1: outputs in SPREADSHEET format one TOTAL line per file\n");
	printf("+dF: specify dss rules file name. (no default)\n");
//	printf("+i : interactively generates dss table\n");
	printf("+lS: specify data language (eng - English, jpn - Japanese)\n");
#ifdef UNX
	printf("+LF: specify full path F of the lib folder\n");
#endif
	mainusage(TRUE);
}
#endif // KIDEVAL_LIB

static void freeUtts(DSSSP *p) {
	int k;

	for (k=0; k < p->uttnum; k++) {
		if (p->utts[k] != NULL) {
			free(p->utts[k]);
			p->utts[k] = NULL;
		}
	}
	p->uttnum = 0;
	free(p->utts);
	p->utts = NULL;
}

void dss_freeSpeakers(void) {
	DSSSP *p, *t;

	dss_sp_head.sp[0] = EOS;
	if (dss_sp_head.ID)
		free(dss_sp_head.ID);
	freeUtts(&dss_sp_head);
	p = dss_sp_head.next_sp;
	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->ID)
			free(t->ID);
		freeUtts(t);
		free(t);
	}
	dss_sp_head.next_sp = NULL;
}

void freeLastDSSUtt(DSSSP *p) {
	if (p->uttnum > 0) {
		p->uttnum--;
		if (p->utts[p->uttnum] != NULL) {
			free(p->utts[p->uttnum]);
			p->utts[p->uttnum] = NULL;
		}
	}
}

static void dss_freeall(DSS_MACH *rootmac) {
	DSS_NMN *t, *ts;

	if (rootmac == NULL) return;
	ts = rootmac->nextmac;
	while (ts != NULL) {
		if (!ts->isAliasAddress)
			dss_freeall(ts->tmac);
		t = ts;
		ts = ts->nextmn;
		free(t);
	}
	free(rootmac);
}

static void freerule(RULE *p) {
	RULE *t;
	
	while (p) {
		t = p;
		p = p->next_rlist;
		dss_freeall(t->focus);
		dss_freeall(t->lcont);
		dss_freeall(t->rcont);
		free(t->points);
		free(t->action);
		free(t);
	}
}

static void freerules() {
	RULES *t;
	
	while (root_rules) {
		t = root_rules;
		root_rules = root_rules->next_rule;
		freerule(t->rule_list);
		free(t->name);
		free(t);
	}
	root_rules = NULL;
}

static void freestack() { /* DO NOT FREE UP ANYMORE OF THE STACK ELEMENTS */
	RULES *t;
	
	while (stack) {
		t = stack;
		stack = stack->next_rule;
		free(t);
	}
	stack = NULL;
}

void dss_cleanSearchMem(void) {
	free(dss_mat);
	dss_mat = NULL;
	free(MatCop);
	MatCop = NULL;
	free(dss_expression);
	dss_expression = NULL;
	freestack();
	freerules();
}

static DSS_NMN *dss_mknextmac() {
	DSS_NMN *t;

	t = NEW(DSS_NMN);
	if (t == NULL)
		out_of_mem();
	t->neg = 0;
	t->isAliasAddress = FALSE;
	t->tmac = NULL;
	t->nextmn = NULL;
	return(t);
}

static DSS_MACH *dss_mkmac() {
	DSS_MACH *t;
	t = NEW(DSS_MACH);
	if (t == NULL)
		out_of_mem();
	t->pat = NULL;
	t->neg = 0;
	t->wild = 0;
	t->skip = 0;
	t->fmatch = 0;
	t->lmatch = 0;
	t->nextmac = dss_mknextmac();
	return(t);
}

static void dss_display(const char *sp, char *inst, FILE  *fp) {
	register int i, k, j = 0;
	char arr[256], f = 0, ft = 1;

	if (*sp)
		fputs(sp,fp);
	for (i=0; inst[i]; i++) {
		if (inst[i] != '\n') {
			if (dss_mat[i]) {
				arr[j] = (char)((int)dss_mat[i] + (int)'0');
				dss_mat[i] = 0;
				f = 1;
			} else 
			if (inst[i] == '\t') arr[j] = '\t';
			else arr[j] = ' ';
			if (j < 255) j++;
			putc(inst[i],fp);
		} else {
			putc(inst[i],fp);
			if (dss_mat[i]) {
				arr[j] = (char)((int)dss_mat[i] + (int)'0');
				dss_mat[i] = 0;
				f = 1;
				if (j < 255) j++;
			}
			if (f) {
				if (ft) {
					for (k=0; sp[k]; k++) {
						if (sp[k] == '\t')
							putc('\t',fp);
						else
							putc(' ',fp);
					}
				}
				if (j < 255) {
					for (k=0; k < j; k++) { putc(arr[k],fp); arr[k] = ' '; }
					putc('\n',fp);
				}
			}
			ft = 0;
			f = 0;
			j = 0;
		}
	}
}

static void dss_err(const char *s, char *st) {
	int temp;

	temp = (int )(st-dss_expression);
	dss_mat[temp] = 1;
	dss_display("",dss_expression,stderr);
	for (temp=0; s[temp]; temp++)
		putc(s[temp], stderr);
	fprintf(stderr,"\n");
	dss_mferr = 1;
}

static int dss_storepat(DSS_MACH *tt, char *ln, int pos) {
	register int s1, s2;
	char t;

	s1 = pos;
	do {
		for (;  !uS.isRightChar(ln, pos, '(', &dFnt, C_MBF) && !uS.isRightChar(ln, pos, '!', &dFnt, C_MBF) && 
						!uS.isRightChar(ln, pos, '+', &dFnt, C_MBF) && !uS.isRightChar(ln, pos, ')', &dFnt, C_MBF) &&
						!uS.isRightChar(ln, pos, '^', &dFnt, C_MBF) && !uS.isRightChar(ln, pos, '\n', &dFnt, C_MBF); pos++) ;
		if (pos-1 < s1 || !uS.isRightChar(ln, pos-1, '\\', &dFnt, C_MBF)) break;
		else if (ln[pos] == '\n' || ln[pos] == EOS) {
			if (uS.isRightChar(ln, pos-1, '\\', &dFnt, C_MBF)) dss_err("Unexpected ending.",ln+pos);
			break;
		} else pos++;
	} while (1) ;
	if (uS.isRightChar(ln,pos,'!',&dFnt,C_MBF) || uS.isRightChar(ln,pos,'(',&dFnt,C_MBF))
		dss_err("Illegal element found.",ln+pos);
	s2 = pos;
	for (; s1 != s2 && uS.isskip(ln,s1,&dFnt,C_MBF) && !uS.isRightChar(ln,s1,'[',&dFnt,C_MBF) && !uS.isRightChar(ln,s1,']',&dFnt,C_MBF); s1++) ;
	if (s1 != s2) {
		for (s2--; s2 >= s1 && uS.isskip(ln,s2,&dFnt,C_MBF) && 
			!uS.isRightChar(ln, s2, '[', &dFnt, C_MBF) && !uS.isRightChar(ln, 2, ']', &dFnt, C_MBF); s2--) ;
		s2++;
	}
	if (uS.isRightChar(ln,s2-1,'*',&dFnt,C_MBF) && (!uS.isRightChar(ln,s2-2,'\\',&dFnt,C_MBF) || uS.isRightChar(ln,s2-3,'\\',&dFnt,C_MBF))) {
		s2--;
		for (; s2 >= s1 && uS.isRightChar(ln, s2, '*', &dFnt, C_MBF); s2--) ;
		if (uS.isRightChar(ln,s2,'\\',&dFnt,C_MBF) && (!uS.isRightChar(ln,s2-1,'\\',&dFnt,C_MBF) || uS.isRightChar(ln,s2-2,'\\',&dFnt,C_MBF)))
			s2 += 2;
		else
			s2++;
		if (s1 == s2) {
			s2 = s1;
			tt->wild = 1;
		} else s2++;
		t = ln[s2];
		ln[s2] = EOS;
		tt->pat = (char *)malloc(strlen(ln+s1)+1);
		strcpy(tt->pat,ln+s1);
		ln[s2] = t;
	} else {
		if (s1 == s2) {
			if (ln[s2]=='\n' && ln[s2+1]==EOS) dss_err("Unexpected ending.",ln+s1);
			else dss_err("empty item is not allowed.",ln+s1);
		}
		t = ln[s2];
		ln[s2] = EOS;
		tt->pat = (char *)malloc((size_t) (strlen(ln+s1)+1));
		strcpy(tt->pat,ln+s1);
		ln[s2] = t;
	}
	for (; s1 < s2; s1++) {
		if (uS.isRightChar(ln, s1, '[', &dFnt, C_MBF) || uS.isRightChar(ln, s1, ']', &dFnt, C_MBF)) ;
		else if (uS.isskip(ln,s1,&dFnt,C_MBF)) dss_err("Illegal character in a word.",ln+s1);
	}
	return(pos);
}

static void promote_wild(DSS_MACH *tm, char wild) {
	DSS_NMN *ts;

	ts = tm->nextmac;
	while (1) {
		if (ts->tmac != NULL) {
			if (ts->tmac->pat == NULL && tm->wild) ts->tmac->wild = 1;
			promote_wild(ts->tmac,tm->wild);
		}

		ts = ts->nextmn;
		if (ts == NULL) break;
	}
}

static void dss_debug(DSS_MACH *tm, char wild) {
	DSS_NMN *ts;

	if (tm->nextmac->tmac != NULL) putc('(',debug_dss_fp);
	if (tm->pat != NULL) {
		if (tm->neg) putc('!',debug_dss_fp);
		if (*tm->pat == EOS) fprintf(debug_dss_fp,"{*}");
		else fprintf(debug_dss_fp,"%s",tm->pat);
	}
	ts = tm->nextmac;
	while (1) {
		if (ts->tmac != NULL && tm->pat != NULL) putc('^',debug_dss_fp);
		if (ts->tmac != NULL) {
			if (ts->tmac->pat == NULL && tm->wild) ts->tmac->wild = 1;
			dss_debug(ts->tmac,tm->wild);
		}
		ts = ts->nextmn;
		if (ts == NULL) break;
		if (ts->neg) putc('&',debug_dss_fp);
		else putc('+',debug_dss_fp);
	}
	if (tm->nextmac->tmac != NULL) putc(')',debug_dss_fp);
}

static char dss_closeall(DSS_MACH *tm, DSS_MACH *tt, char isFirstAddress) {
	DSS_NMN *ts;

	if (tm == NULL || tm == tt)
		return(isFirstAddress);

	ts = tm->nextmac;
	if (ts->tmac == NULL) {
		ts->tmac = tt;
		ts->isAliasAddress = !isFirstAddress;
		isFirstAddress = FALSE;
	} else {
		while (ts != NULL) {
			isFirstAddress = dss_closeall(ts->tmac,tt,isFirstAddress);
			ts = ts->nextmn;
		}
	}
	return(isFirstAddress);
}

static int dss_makemach(char *exp, int pos, DSS_NMN *mac, char mneg) {
	DSS_NMN *lmac;
	DSS_MACH *tt, *lastt;
	char lneg = 0;

	if (uS.isRightChar(exp, pos, '^', &dFnt, C_MBF) || uS.isRightChar(exp, pos, '+', &dFnt, C_MBF) || uS.isRightChar(exp, pos, ')', &dFnt, C_MBF)) 
		dss_err("Illegal element found.",exp+pos);
	mac->tmac = dss_mkmac();
	lmac = mac;
	tt = mac->tmac;
	lastt = tt;
	if (mneg) {
		tt->neg = 1;
		lmac->neg = 1;
	}
	while (exp[pos] != EOS && !uS.isRightChar(exp, pos, ')', &dFnt, C_MBF)) {
		if (uS.isRightChar(exp, pos, '(', &dFnt, C_MBF)) {
			pos++;
			pos = dss_makemach(exp,pos,tt->nextmac,(char)(lneg || tt->neg));
			if (!uS.isRightChar(exp, pos, ')', &dFnt, C_MBF)) dss_err("No matching ')' found.",exp+pos);
			pos++;
			if (!uS.isRightChar(exp, pos, '+', &dFnt, C_MBF) && !uS.isRightChar(exp, pos, '^', &dFnt, C_MBF) && 
				!uS.isRightChar(exp, pos, ')', &dFnt, C_MBF) && !uS.isRightChar(exp, pos, '\n', &dFnt, C_MBF))
				dss_err("Illegal element found.",exp+pos);
			tt->neg = 0;
			lneg = 0;
		} else if (uS.isRightChar(exp, pos, '!', &dFnt, C_MBF)) { /* not */
			pos++;
			if (uS.isRightChar(exp, pos, '+', &dFnt, C_MBF) || uS.isRightChar(exp, pos, '^', &dFnt, C_MBF) || 
				uS.isRightChar(exp, pos, ')', &dFnt, C_MBF) || uS.isRightChar(exp, pos, '!', &dFnt, C_MBF)) 
				dss_err("Illegal element found.",exp+pos);
			else if (!uS.isRightChar(exp, pos, '(', &dFnt, C_MBF)) {
				pos = dss_storepat(tt,exp,pos);
				tt->neg = 1;
			} else if (!mneg) lneg = 1;
		} else if (uS.isRightChar(exp, pos, '+', &dFnt, C_MBF)) { /* or */
			lmac->nextmn = dss_mknextmac();
			lmac = lmac->nextmn;
			lmac->tmac = dss_mkmac();
			tt = lmac->tmac;
			lastt = tt;
			if (mneg || lneg) {
				lmac->neg = 1;
				tt->neg = 1;
			}
			lneg = 0;
			pos++;
			if (uS.isRightChar(exp, pos, '+', &dFnt, C_MBF) || uS.isRightChar(exp, pos, '^', &dFnt, C_MBF) || uS.isRightChar(exp, pos, ')', &dFnt, C_MBF)) 
				dss_err("Illegal element found.",exp+pos);
			else if (!uS.isRightChar(exp, pos, '(', &dFnt, C_MBF) && !uS.isRightChar(exp, pos, '!', &dFnt, C_MBF))
				pos = dss_storepat(tt,exp,pos);
		} else if (uS.isRightChar(exp, pos, '^', &dFnt, C_MBF)) { /* and */
			tt = dss_mkmac();
			dss_closeall(lastt,tt,TRUE);
			lastt = tt;
			if (lneg) tt->neg = 1;
			lneg = 0;
			pos++;
			if (uS.isRightChar(exp, pos, '+', &dFnt, C_MBF) || uS.isRightChar(exp, pos, '^', &dFnt, C_MBF) || uS.isRightChar(exp, pos, ')', &dFnt, C_MBF)) 
				dss_err("Illegal element found.",exp+pos);
			else if (!uS.isRightChar(exp, pos, '(', &dFnt, C_MBF) && !uS.isRightChar(exp, pos, '!', &dFnt, C_MBF))
				pos = dss_storepat(tt,exp,pos);
		} else if (uS.isRightChar(exp, pos, '\n', &dFnt, C_MBF))
			pos++;
		else
			pos = dss_storepat(tt,exp,pos);
	}
	return(pos);
}

static char *dss_setmat(char *txt, int *last, char pat) {
	int temp;

	temp = (int )(txt-uttline);
	temp += dss_firstmatch;
	if (pat != '\0') dss_mat[temp] = (char)1;
	txt += *last;
	*last = temp;
	return(txt);
}	

static int dss_subindex(char *s, char *pat, int i, char end) {
	register int j, k, n, m;
	
	dss_firstmatch = i;
	for (j = i, k = 0; pat[k]; j++, k++) {
		if (s[j] == '\001') { k--; continue; }
		if (pat[k] == '*') {
			while (pat[++k] == '*') ;
			if (pat[k] == '\\') k++;
		forward:
			while (s[j] != end && s[j] != pat[k]) j++;
			if (dss_firstmatch == -1) dss_firstmatch = j;
			if (s[j] == end) { if (!pat[k]) return(j); else return(-1); }
			for (m=j+1, n=k+1; s[m] != end && pat[n]; m++, n++) {
				if (pat[n]=='*' || pat[n]=='_' || pat[n]=='\\') break;
				else if (s[m] == '\001') n--;
				else if (s[m] != pat[n]) {
					j++;
					goto forward;
				}
			}
			if (!pat[n] && s[m] != end) {
				j++;
				goto forward;
			}
		} else {
			if (dss_firstmatch == -1) dss_firstmatch = j;
			if (pat[k] == '\\') { if (s[j] != pat[++k]) break; }
			else if (pat[k] == '_') continue; /* any character */
		}
		if (s[j] != pat[k]) break;
	}
	if (pat[k] == EOS) {
		if (s[j] == end) return(j);
		else return(-1);
	} else return(-1);
}

/* return index of pat in s, -1 if no match */
static int dss_CMatch(char *s, char *pat, char wild) {
	register int i;
	register int j;
	register int e;
	char t;
	
	if (*pat == EOS) {
		dss_firstmatch = 0;
		return(0);
	}
	if (wild) {
		i = 0;
		while (s[i]) {
			while (s[i] == ' ') i++;
			e = i;
			do {
				for (i=e; s[e] != ' ' && s[e] != '^'; e++) ;
				if (s[e] == '^') { s[e] = ' '; t = '^'; } else t = ' ';
				/* printf("s+i=%s;\n", s+i); */
				if ((j=dss_subindex(s, pat, i, ' ')) != -1) {
					s[e] = t;
					if (t != ' ') for (; s[j] != ' '; j++) ;
					while(s[j] == ' ') j++;
					return(j);
				} else s[e] = t;
				if (t == ' ') break; else e++;
			} while (1) ;
			while (s[i] != ' ' && s[i] != 0) i++;
			while (s[i] == ' ') i++;
		}
	} else {
		e = i = 0;
		do {
			for (i=e; s[e] != ' ' && s[e] != '^'; e++) ;
			if (s[e] == '^') { s[e] = ' '; t = '^'; } else t = ' ';
			/* printf("s+i=%s;\n", s+i); */
			j = dss_subindex(s, pat, i, ' ');
			s[e] = t;
			if (j != -1) {
				if (t != ' ') for (; s[j] != ' '; j++) ;
				while (s[j] == ' ') j++;
				return(j);
			}
			if (t == ' ') break; else e++;
		} while (1) ;
	}
	return(-1);
}

static char *dss_match(char *txt, DSS_MACH *tm, char wild, char *neg) {
	DSS_NMN *ts;
	int last = -1;
	char *fst, *tmp;
	char neg_or;

	if (debug_level > 2)
		fprintf(debug_dss_fp, "2; pat=%s;tm->wild=%d; wild=%d;tm->neg=%d;neg=%s;txt=%s\n", tm->pat, tm->wild, wild, tm->neg, neg, txt);

	if (tm->skip && txt < tm->fmatch && !tm->neg) return(NULL);
	if (tm->pat != NULL) {
		if (tm->fmatch >= txt) {
			dss_firstmatch = 0;
			txt = tm->fmatch;
			last = tm->lmatch;
		} else {
			if ((last=dss_CMatch(txt,tm->pat,wild)) != -1) {
				tm->fmatch = txt + dss_firstmatch;
				tm->lmatch = last - dss_firstmatch;
			}
		}
		if (last == -1) {
			if (wild && tm->pat != NULL) tm->skip = 1;
			if (!tm->neg) return(NULL);
			else neg = NULL;
		} else {
			if (*tm->pat!= 0 && neg && txt+dss_firstmatch>= neg) return(NULL);
			if (tm->neg) {
				tm->wild = 1;
				neg = txt + last;
			} else {
				if (*tm->pat != EOS) neg = NULL;
				txt = dss_setmat(txt,&last,*tm->pat);
			}
		}
	}
	ts = tm->nextmac;
	if (ts->tmac == NULL) {
		if (*tm->pat == EOS) {
			txt = txt + strlen(txt);
			return(txt);
		}
	}
	fst = txt;
	neg_or = (char)ts->neg;
	while (1) {

		if (debug_level > 2)
			fprintf(debug_dss_fp, "3; pat=%s;tm->wild=%d; wild=%d;tm->neg=%d;neg=%s;txt=%s\n", tm->pat, tm->wild, wild, tm->neg, neg, txt);

		if (ts->tmac == NULL) {
			if (neg) return(NULL); 
			else if (!tm->neg) return(txt);
			else {
				txt = txt + strlen(txt);
				return(txt);
			}
		}
		if (ts->tmac->pat == NULL && tm->wild) ts->tmac->wild = 1;
		do {
			if (debug_level > 2)
				fprintf(debug_dss_fp, "4; pat=%s;tm->wild=%d; wild=%d;tm->neg=%d;neg=%s;l=%d; txt=%s\n", tm->pat, tm->wild, wild, tm->neg, neg, last, txt);

			while (*txt == ' ') txt++;
			if ((tmp=dss_match(txt,ts->tmac,tm->wild,neg)) != NULL) {
				if (!neg_or) return(tmp);
				goto cont;
			} else if (tm->neg && !ts->tmac->wild) {
				txt = neg;
				while (*txt != ' ' && *txt != 0) txt++;
				while (*txt == ' ') txt++;
				if (*txt == EOS) return(NULL);
				if ((tmp=dss_match(txt,ts->tmac,tm->wild,NULL)) != NULL) {
					if (!neg_or) return(tmp);
					goto cont;
				}
			}
			if (tm->pat == NULL || !wild) {
				if (neg_or) return(NULL); else break;
			}
			if (last > -1) dss_mat[last] = 0;
			if (*tm->pat == EOS || last == -1 || tm->neg) {
				while (*txt != ' ' && *txt != 0) txt++;
				while (*txt == ' ') txt++;
				if (*txt == EOS) return(NULL);
			}

			if (debug_level > 2)
				fprintf(debug_dss_fp, "5; pat=%s;tm->wild=%d; wild=%d;tm->neg=%d;neg=%s;l=%d; txt=%s\n", tm->pat, tm->wild, wild, tm->neg, neg, last, txt);

			if ((last=dss_CMatch(txt,tm->pat,wild)) != -1) {
				tm->fmatch = txt + dss_firstmatch;
				tm->lmatch = last - dss_firstmatch;
				if ((neg && txt+dss_firstmatch >= neg) || tm->neg) {
					if (neg_or) return(NULL); else break;
				} else txt = dss_setmat(txt,&last,*tm->pat);
			} else if (neg_or) return(NULL); else break;

			if (debug_level > 2)
				fprintf(debug_dss_fp, "6; pat=%s;tm->wild=%d; wild=%d;tm->neg=%d;neg=%s;l=%d; txt=%s\n", tm->pat, tm->wild, wild, tm->neg, neg, last, txt);

		} while (1) ;
cont:
		ts->tmac->skip = 1;
		txt = fst;
		ts = ts->nextmn;
		if (ts == NULL) break;
	}
	if (neg_or) return(tmp);
	if (last > -1) dss_mat[last] = 0;
	return(NULL);
}

static void dss_clearmac(DSS_MACH *rootmac) {
	DSS_NMN *ts;

	if (rootmac == NULL) return;
	ts = rootmac->nextmac;
	while (ts != NULL) {
		dss_clearmac(ts->tmac);
		ts = ts->nextmn;
	}
	rootmac->skip = 0;
	rootmac->fmatch = 0;
	rootmac->lmatch = 0;
}

static int LastMatch(DSS_MACH *tm, char *txt) {
	if (tm == NULL) {
		return(TRUE);
	}
	for (; txt != uttline && *txt != ' '; txt--) ;
	for (; txt != uttline && *txt == ' '; txt--) ;
	if (tm->pat != NULL) {
		if (!strcmp(tm->pat, ":::") && txt == uttline) {
			return(TRUE);
		}
	}
	if (txt == uttline)
		return(FALSE);
	for (; txt != uttline && *txt != ' '; txt--) ;
	if (txt != uttline && *txt == ' ') txt++;
	return(dss_match(txt,tm,0,NULL) != NULL);
}

static void DebugLastMatch(DSS_MACH *tm, char *txt) {
	if (tm == NULL) {
		fprintf(debug_dss_fp, "*** last=NULL\n");
		return;
	}
	for (; txt != uttline && *txt != ' '; txt--) ;
	for (; txt != uttline && *txt == ' '; txt--) ;
	if (tm->pat != NULL) {
		if (!strcmp(tm->pat, ":::") && txt == uttline) {
			fprintf(debug_dss_fp, "*** last=:::\n");
			return;
		}
	}
	if (txt == uttline)
		return;
	for (; txt != uttline && *txt != ' '; txt--) ;
	if (txt != uttline && *txt == ' ') txt++;
	fprintf(debug_dss_fp, "*** last=");
	dss_debug(tm, (char)0); putc('\n',debug_dss_fp);
	fprintf(debug_dss_fp, "txt=%s\n", txt);
}

static int NextMatch(DSS_MACH *tm, char *txt) {
	if (tm == NULL) {
		return(TRUE);
	}
	if (*txt == EOS)
		return(FALSE);
	return(dss_match(txt,tm,0,NULL) != NULL);
}

static void DebugNextMatch(DSS_MACH *tm, char *txt) {
	if (tm == NULL) {
		fprintf(debug_dss_fp, "*** next=NULL\n");
		return;
	}
	if (*txt == EOS)
		return;
	fprintf(debug_dss_fp, "*** next=");
	dss_debug(tm, (char)0); putc('\n',debug_dss_fp);
	fprintf(debug_dss_fp, "txt=%s", txt);
}

static RULES *FindRule(char *s) {
	RULES *tr;
	
	for (tr=root_rules; tr != NULL; tr=tr->next_rule) {
		if (strcmp(tr->name, s) == 0) return(tr);
	}
	fprintf(stderr,"RULENAME %s is not found.\n", s);
	return(NULL);
}

static char TakeAction(char *st, char isDebug) {
	register int  s;
	register int  l;
	register char t;
	RULES *tr, *ts;
	
	l = 0;
	do {
		for (s=l; st[s] == ' '; s++) ;
		for (l=s; st[l] != ' ' && st[l]; l++) ;
		t = st[l];
		st[l] = EOS;
		if ((tr=FindRule(st+s+1)) == NULL)
			dss_mferr = 1;
		else if (st[s] == '+') {
			if (stack == NULL) {
				if ((stack=NEW(RULES)) == NULL) { dss_mferr = 1; goto cont; }
				ts = stack;
			} else {
				for (ts=stack; ts->next_rule != NULL; ts=ts->next_rule) {
					if (!strcmp(ts->name, st+s+1)) goto cont;
				}
				if ((ts->next_rule=NEW(RULES)) == NULL) {
					dss_mferr = 1; goto cont;
				}
				ts = ts->next_rule;
			}
			
			ts->name = tr->name;
			ts->rule_list = tr->rule_list;
			ts->next_rule = NULL;
			if (debug_level > 0 && isDebug) {
				fprintf(debug_dss_fp, "***** Added rule to stack: %s\n", st+s+1);
			}
		} else {
			for (ts=stack; ts != NULL; ts=ts->next_rule) {
				if (!strcmp(ts->name, st+s+1)) {
					if (ts == stack) stack = ts->next_rule;
					else tr->next_rule = ts->next_rule;
					free(ts);
					if (debug_level > 0 && isDebug) {
						fprintf(debug_dss_fp, "***** Removed rule from stack: %s\n", st+s+1);
					}
					goto cont;
				}
				tr = ts;
			}
		}
	cont:
		st[l] = t;
		if (dss_mferr) {
			dss_cleanSearchMem();
			if (tfp) {
				fclose(tfp);
				tfp = NULL;
			}
			if (debug_dss_fp != stdout)
				fclose(debug_dss_fp);
			return(FALSE);
		}
	} while (t) ;
	return(TRUE);
}

static char dss_findmatch(char *txt) {
	RULE  *tr;
	RULES *ts;
	char *ltxt;
	register int i;

	templineC4[0] = EOS;
	templineC3[0] = EOS;
	while (*txt == ' ')
		txt++;
	while (*txt != EOS) {
		for (ts=stack; ts != NULL; ts=ts->next_rule) {

			if (debug_level > 0)
				fprintf(debug_dss_fp, "\nRULENAME=%s;\n", ts->name);

			for (tr=ts->rule_list; tr != NULL; tr=tr->next_rlist) {

				if (debug_level > 0) {
					fprintf(debug_dss_fp, "**** focus=");
					dss_debug(tr->focus, (char)0); putc('\n',debug_dss_fp);
					if (debug_level > 1 || strcmp(templineC4, txt)) {
						fprintf(debug_dss_fp, "tier=%s;\n", txt);
						strcpy(templineC4, txt);
					}
				}

				for (i=0; i < BUFSIZ; i++)
					dss_mat[i] = 0;
				ltxt = txt;

				if ((txt=dss_match(txt,tr->focus,0,NULL)) != NULL) {
					for (i=0; i < BUFSIZ; i++) MatCop[i] = dss_mat[i];
					dss_clearmac(tr->focus);
					if (debug_level > 1) {
						fprintf(debug_dss_fp, "focus match SUCCEEDED\n");
					}
					if (LastMatch(tr->lcont, ltxt)) {
						dss_clearmac(tr->lcont);
						if (debug_level > 1) {
							DebugLastMatch(tr->lcont, ltxt);
							fprintf(debug_dss_fp, "last match SUCCEEDED\n");
						}
						if (NextMatch(tr->rcont, txt)) {
							dss_clearmac(tr->rcont);
							if (debug_level > 1) {
								DebugNextMatch(tr->rcont, txt);
								fprintf(debug_dss_fp, "next match SUCCEEDED\n");
							}

							if (tr->points != NULL) {
								if (*tr->points != EOS) {
									strcat(templineC3,tr->points);
									strcat(templineC3, "; ");
								}
							} else {
								strcat(templineC3, "<points NULL>");
								strcat(templineC3, "; ");
							}

							if (debug_level > 0) {
								putc('\n',debug_dss_fp);
								fprintf(debug_dss_fp,"f= "); dss_debug(tr->focus,(char)0); putc('\n',debug_dss_fp);
								if (tr->rcont) { 
									fprintf(debug_dss_fp,"r= "); dss_debug(tr->rcont,(char)0); putc('\n',debug_dss_fp); }
								if (tr->lcont) {
									fprintf(debug_dss_fp,"l= "); dss_debug(tr->lcont,(char)0); putc('\n',debug_dss_fp); }
								if (tr->points != NULL) {
									if (*tr->points != EOS) {
										fprintf(debug_dss_fp,"Current Points=%s\n", tr->points);
									}
								} else {
									fprintf(debug_dss_fp,"Current Points=%s\n", "<points NULL>");
								}
								if (templineC3[0] != EOS) {
									fprintf(debug_dss_fp,"Total Points=%s\n", templineC3);
								}
								fprintf(debug_dss_fp,"txt=%s\n", ltxt);
								for (i=0; i < BUFSIZ; i++)
									dss_mat[i] = MatCop[i];
								dss_display("",uttline, debug_dss_fp);
								fputs("--- MATCHED ---\n", debug_dss_fp);
							}

							if (tr->action != NULL) {
								if (!TakeAction(tr->action, TRUE))
									return(FALSE);
							}
							if (dss_lang == 'e') {
								txt = ltxt;
								goto en_cont;
							} else if (dss_lang == 'j') {
								goto jp_cont;
							}
						} else {
							if (debug_level > 1) {
								DebugNextMatch(tr->rcont, txt);
								fprintf(debug_dss_fp, "next match FAILED\n");
							}
							txt = ltxt;
							dss_clearmac(tr->rcont);
						}
					} else {
						if (debug_level > 1) {
							DebugLastMatch(tr->lcont, ltxt);
							fprintf(debug_dss_fp, "last match FAILED\n");
						}
						txt = ltxt;
						dss_clearmac(tr->lcont);
					}
				} else {
					if (debug_level > 1)
						fprintf(debug_dss_fp, "focus match FAILED\n");
					txt = ltxt;
					dss_clearmac(tr->focus);
				}
			}
en_cont: ;
		}
		if (ts == NULL) {
			while (*txt != ' ')
				txt++;
		}
jp_cont: ;
		while (*txt == ' ' || *txt == '\n')
			txt++;
	}
	uS.uppercasestr(templineC3, &dFnt, MBF);
	return(TRUE);
}

static void dss_mycpy(char *exp, char *pat) {
	while (*pat != '\n' && *pat != EOS) {
		if (*pat == ',') pat++;
		else *exp++ = *pat++;
	}
	*exp = EOS;
}

static void rule_err(FILE *fp, const char *s, const char *st) {
	fprintf(stderr,s, st);
	fclose(fp);
	dss_mferr = 1;
}

static int prep_exps(int s, int l) {
	dss_expression[l] = ':';
	for (s=l+1; dss_expression[s] == ' ' || dss_expression[s] == '\t'; s++) ;
	l = s + strlen(dss_expression+s)-1;
	for (; dss_expression[l] == '\n' || dss_expression[l] == ' '; l--) ;
	dss_expression[++l] = '\n';
	dss_expression[++l] = EOS;
	return(s);
}

static RULE *mk_patt(int contr, int s, int l, RULE  *lr, RULES *tr, FILE  *fp) {
	DSS_MACH *int_pat;
	RULE *tlr;

	s = prep_exps(s, l);
	dss_mferr = 0;
	int_pat = dss_mkmac();
	strcpy(dss_expression, dss_expression+s);
	if (!strcmp(dss_expression, ":::\n") && contr == 0) {
		dss_expression[3] = EOS;
		int_pat->pat = (char *)malloc(strlen(dss_expression)+1);
		strcpy(int_pat->pat,dss_expression);
	} else {
		dss_makemach(dss_expression,0,int_pat->nextmac,0);
		if (dss_mferr) {
			free(int_pat);
			fclose(fp);
			return(NULL);
		}
		promote_wild(int_pat, (char)0);
/*
dss_debug(int_pat, (char)0); putc('\n',debug_dss_fp);
*/
	}
	if (contr == 1) {
		if ((lr=NEW(RULE)) == NULL) {
			rule_err(fp,"ERROR: OUT OF MEMORY.\n","");
			return(NULL);
		}
		lr->focus = int_pat;
		lr->lcont = NULL;
		lr->rcont = NULL;
		lr->points = NULL;
		lr->action = NULL;
		lr->next_rlist = NULL;
		if (root_rules->rule_list == NULL) root_rules->rule_list = lr;
		else {
			tlr = root_rules->rule_list;
			for (; tlr->next_rlist != NULL; tlr=tlr->next_rlist) ;
			tlr->next_rlist = lr;
		}
	} else if (contr == 0) lr->lcont = int_pat;
	else lr->rcont = int_pat;
	return(lr);
}

static char parsePointsNames(char *line) {
	int i, j;

	PointsCnt = 0;
	for (i=8; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == '|')
		i++;
	j = 0;
	for (; line[i] != EOS; i++) {
		if (line[i] == '|') {
			for (; j < 3; j++)
				PointsNames[PointsCnt][j] = ' ';
			PointsNames[PointsCnt][j] = EOS;
			PointsCnt++;
			if (PointsCnt >= POINTS_N) {
				fprintf(stderr,"Too many points in dssrules\n");
				dss_mferr = 1;
				return(FALSE);
			}
			j = 0;
		} else if (j < POINTNAME_LEN)
			PointsNames[PointsCnt][j++] = line[i];
	}
	return(TRUE);
}

static void MakeRules() {
	register int s;
	register int l;
	char  isFoundPointsNames;
	RULE  *lr = NULL;
	RULES *tr;
	FILE  *fp;
	FNType mFileName[FNSize];

	strcpy(mFileName, lib_dir);
#if !defined(CLAN_SRV)
	addFilename2Path(mFileName, "dss");
#endif
	addFilename2Path(mFileName, rulesfile);
	if ((fp=fopen(mFileName,"r")) == NULL) {
		if (!isDssFileSpecified) {
			fprintf(stderr, "Can't open dss rules file: \"%s\"\n", mFileName);
			fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
			dss_mferr = 1;
			return;
		} else {
			strcpy(templineC, wd_dir);
			addFilename2Path(templineC, rulesfile);
			if ((fp=fopen(templineC,"r")) == NULL) {
				fprintf(stderr, "Can't open either one of the dss rules files:\n\t\"%s\"\n\t\"%s\"\n", templineC, mFileName);
				fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
				dss_mferr = 1;
				return;
			} else {
				strcpy(mFileName, templineC);
			}
		}
	}
#if !defined(CLAN_SRV)
	fprintf(stderr, "    Using dss rules file: %s\n", mFileName);
#endif
	isFoundPointsNames = FALSE;
	tr = NULL;
	while (fgets_cr(dss_expression, BUFSIZ, fp)) {
		if (uS.isUTF8(dss_expression) || uS.isInvisibleHeader(dss_expression))
			continue;
		if (uS.mStrnicmp(dss_expression, POINTS_NAME_TAG, strlen(POINTS_NAME_TAG)) == 0) {
			isFoundPointsNames = parsePointsNames(dss_expression);
			if (!isFoundPointsNames)
				return;
			continue;
		}
		for (s=0; dss_expression[s] != '#' && dss_expression[s]; s++) ;
		if (dss_expression[s] == '#')
			dss_expression[s] = EOS;
		for (s=0; dss_expression[s]== ' ' || dss_expression[s]== '\t'; s++) ;
		for (l=s; dss_expression[l] != ':' && dss_expression[l]; l++)
			;
    	if (uS.isUTF8(dss_expression))
    		continue;
		if (dss_expression[l] == EOS || dss_expression[l] == '\n')
			continue;
		if (dss_expression[l] != ':') {
			rule_err(fp,"ERROR: missing ':' character: %s\n",dss_expression);
			return;
		}
		dss_expression[l] = EOS;
		if (!strcmp(dss_expression+s, "RULENAME")) {
			if ((tr=NEW(RULES)) == NULL) {
				rule_err(fp,"ERROR: OUT OF MEMORY.\n","");
				return;
			}
			for (s=l+1; dss_expression[s] == ' ' || 
						dss_expression[s] == '\t'; s++) ;
			for (l=s; isalnum((unsigned char)dss_expression[l]) || 
						dss_expression[l] == '-'; l++) ;
			dss_expression[l] = EOS;
			tr->name = (char *)malloc(strlen(dss_expression+s)+1);
			if (tr->name == NULL) {
				free(tr);
				rule_err(fp,"ERROR: OUT OF MEMORY.\n","");
				return;
			}
			strcpy(tr->name, dss_expression+s);
			tr->rule_list = NULL;
			tr->next_rule = root_rules;
			root_rules = tr;
		} else if (!strcmp(dss_expression+s, "STARTRULES")) {
			s = prep_exps(s, l);
			INITSTACK = (char *)malloc(strlen(dss_expression+s)+1);
			if (INITSTACK == NULL) {
				rule_err(fp,"ERROR: OUT OF MEMORY.\n",""); return;
			}
			dss_mycpy(INITSTACK, dss_expression+s);
		} else if (!strcmp(dss_expression+s, "FOCUS")) {
			if (root_rules == NULL) {
				rule_err(fp,"ERROR: FOCUS found before first RULENAME.\n","");
				return;
			}
			lr = mk_patt(1, s, l, lr, tr, fp);
			if (dss_mferr) return;
		} else {
			if (lr == NULL) {
				rule_err(fp,"ERROR: POINTS found before first FOCUS.\n","");
				return;
			}
			if (!strcmp(dss_expression+s, "LCONTEXT")) {
				lr = mk_patt(0, s, l, lr, tr, fp);
				if (dss_mferr) return;
			} else if (!strcmp(dss_expression+s, "RCONTEXT")) {
				lr = mk_patt(2, s, l, lr, tr, fp);
				if (dss_mferr) return;
			} else if (!strcmp(dss_expression+s, "POINTS")) {
				s = prep_exps(s, l);
				lr->points = (char *)malloc(strlen(dss_expression+s)+1);
				if (lr->points == NULL) {
					rule_err(fp,"ERROR: OUT OF MEMORY.\n",""); return;
				}
				dss_mycpy(lr->points, dss_expression+s);
			} else if (!strcmp(dss_expression+s, "ACTION")) {
				s = prep_exps(s, l);
				lr->action = (char *)malloc(strlen(dss_expression+s)+1);
				if (lr->action == NULL) {
					rule_err(fp,"ERROR: OUT OF MEMORY.\n",""); return;
				}
				dss_mycpy(lr->action, dss_expression+s);
			} else {
				rule_err(fp,"Illegal keyword on line: %s.\n",dss_expression);
				return;
			}
		}
	}
	if (!isFoundPointsNames) {
		strcpy(PointsNames[0], "IP ");
		strcpy(PointsNames[1], "PP ");
		strcpy(PointsNames[2], "MV ");
		strcpy(PointsNames[3], "SV ");
		strcpy(PointsNames[4], "NG ");
		strcpy(PointsNames[5], "CNJ");
		strcpy(PointsNames[6], "IR ");
		strcpy(PointsNames[7], "WHQ");
		PointsCnt = 8;
	}
	fclose(fp);
}

char init_dss(char first) {
	int i;

	if (first) {
		dss_mferr = 0;
		GTotalNum = (float)0.0;
		GGrandTotal = (float)0.0;
		rulesfile = NULL;
		IsOutputSpreadsheet = 0;
		spNum = 0;
		isFirstCall = TRUE;
		isDSSpostcode = TRUE;
		isDSSEpostcode = TRUE;
		DSSAutoMode = TRUE;
		isDssFileSpecified = FALSE;
		root_rules = NULL;
		dss_lang = '\0';
		debug_level = 0;
		PointsCnt = 0;
		DSS_UTTLIM = 50;
		tfp = NULL;
		stack = NULL;
		INITSTACK = NULL;
		dss_sp_head.sp[0] = EOS;
		dss_mat = (char *)malloc((size_t) (BUFSIZ + 1));
		if (dss_mat == NULL) {
			return(FALSE);
		}
		for (i=0; i < BUFSIZ; i++)
			dss_mat[i] = 0;
		MatCop = (char *)malloc((size_t) (BUFSIZ + 1));
		if (MatCop == NULL) {
			free(dss_mat);
			dss_mat = NULL;
			return(FALSE);
		}
		for (i=0; i < BUFSIZ; i++) MatCop[i] = 0;
		dss_expression = (char *)malloc((size_t) (BUFSIZ + 1));
		if (dss_expression == NULL) {
			free(dss_mat);
			dss_mat = NULL;
			free(MatCop);
			MatCop = NULL;
			return(FALSE);
		}
	} else {
		if (dss_lang == '\0' || rulesfile == NULL) {
			if (CLAN_PROG_NUM == DSS) {
				fprintf(stderr, "Please specify data language with \"+l\" option.\n");
				fprintf(stderr, "  eng - English, jpn - Japanese\n");
			} else {
				fprintf(stderr, "Please specify data language inside \"language script file\".\n");
				fprintf(stderr, "  eng - English, jpn - Japanese, 0 - do not compute DSS\n");
			}
			return(FALSE);
		}
		if (strcmp(rulesfile, DSSNOCOMPUTE) != 0)
			MakeRules();
		if (dss_mferr) {
			dss_cleanSearchMem();
			return(FALSE);
		}
	}
	return(TRUE);
}

#ifndef KIDEVAL_LIB
void init(char first) {
	register int i;
	NewFontInfo finfo;
	struct tier *nt;

	if (first) {
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		stout = FALSE;
		FilterTier = 1;
		IsOutputSpreadsheet = 0;
		spNum = 0;
		isFirstCall = TRUE;
		dss_ftime = TRUE;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		addword('\0','\0',"+<#>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		maininitwords();
		mor_initwords();
		if (!init_dss(first)) {
			out_of_mem();
		}
		strcpy(fDepTierName, "%mor:");
		DSSAutoMode = TRUE;
	} else if (dss_ftime) {
		dss_ftime = FALSE;
		for (nt=headtier; nt != NULL; nt=nt->nexttier) {
			if (nt->tcode[0] == '*')
				break;
		}
		if (nt == NULL) {
			fprintf(stderr, "Please specify at least one speaker tier name with \"+t\" option.\n");
			cutt_exit(0);
		}
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == '!' ||
				GlobalPunctuation[i] == '?' ||
				GlobalPunctuation[i] == '.') 
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else i++;
		}
		if (!init_dss(first))
			cutt_exit(0);
		maketierchoice(fDepTierName,'+',FALSE);
		if (!DSSAutoMode) {
			finfo.fontName[0] = EOS;
			SetDefaultCAFinfo(&finfo);
			selectChoosenFont(&finfo, TRUE, TRUE);
		}
		if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
			outputOnlyData = TRUE;
			combinput = TRUE;
			OverWriteFile = TRUE;
			AddCEXExtension = ".xls";
		}
	}
}
#endif // KIDEVAL_LIB

static void dss_pr_result(void) {
	if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
		excelFooter(fpout);
	} else if (GTotalNum && !DSSAutoMode) {
		GTotalNum = GGrandTotal / GTotalNum;
		fprintf(fpout, "\nOverall developmental sentence score: %5.2f\n",GTotalNum);
	}
}

#ifndef KIDEVAL_LIB
CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = DSS;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	debug_dss_fp = stdout;
	bmain(argc,argv,dss_pr_result);
	if (debug_dss_fp != stdout)
		fclose(debug_dss_fp);
	if (combinput) {
		if (tfp) {
			fclose(tfp);
			tfp = NULL;
		}
		if (IsOutputSpreadsheet != 1 && IsOutputSpreadsheet != 2)
			dss_freeSpeakers();
	}
	dss_cleanSearchMem();
}
#endif // KIDEVAL_LIB

static int FinishPointsFixing(char *s, char *buf, int  i, char let) {
	register int j;
	register int k;

	while (isalpha(buf[i]))
		i++;
	if (isdigit(buf[i]) && (buf[i+1] == '+' || buf[i+1] == '-')) {
		k = (int)buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = (char)k;
	}
	if ((buf[i]== '+' || buf[i]== '-') && buf[i+1] != '>' && buf[i+1] != ',') {
		i++;
		if (buf[i] == '0')
			buf[i] = '-';
		if (buf[i-1] == '+') {
			k = strlen(s);
			if (k > 0) s[k++] = ' ';
			s[k++] = buf[i];
			s[k] = EOS;
			if (buf[i] != '-') {
				k = strlen(templineC3);
				templineC3[k++] = let;
				templineC3[k++] = buf[i];
				templineC3[k++] = ';';
				templineC3[k++] = ' ';
				templineC3[k] = EOS;
			}
		} else if (buf[i-1] == '-') {
			for (j=0; s[j] != EOS; j++) {
				if (s[j] == buf[i]) {
					strcpy(s+j, s+j+1);
					if (s[j] == ' ')
						strcpy(s+j, s+j+1);
					if (buf[i] == '-') break;
					for (j=0; templineC3[j] != EOS; ) {
						if (templineC3[j] == let && templineC3[j+1] == buf[i]) {
							k = j + 2;
							while (!isalpha(templineC3[k]) && templineC3[k]!= EOS)
								k++;
							strcpy(templineC3+j, templineC3+k);
						} else j++;
					}
					break;
				}
			}
		}
		while (isdigit(buf[i]))
			i++;
	} else if ((isdigit(buf[i]) || buf[i] == '-') && (buf[i+1] == '>' || buf[i+1] == ',')) {
		if (!isdigit(buf[i+2]) && buf[i+2] != '-')
			return(i+2);
		if (buf[i]   == '0')
			buf[i]   = '-';
		if (buf[i+2] == '0')
			buf[i+2] = '-';
		for (j=0; s[j] != EOS; j++) {
			if (s[j] == buf[i]) {
				s[j] = buf[i+2];
				if (buf[i] == '-') break;
				for (j=0; templineC3[j] != EOS; ) {
					if (templineC3[j] == let && templineC3[j+1] == buf[i]) {
						if (buf[i+2] == '-') {
							k = j + 2;
							while (!isalpha(templineC3[k]) && templineC3[k]!= EOS)
								k++;
							strcpy(templineC3+j, templineC3+k);
						} else {
							templineC3[j+1] = buf[i+2];
							j += 2;
						}
					} else j++;
				}
				break;
			}
		}
		while (isdigit(buf[i])) i++;
	} else if (isdigit(buf[i])) {
		if (buf[i] == '0')	
			return(i+1);
		for (j=0; s[j] != EOS; j++) {
			if (s[j] == buf[i]) {
				s[j] = '-';
				for (j=0; templineC3[j] != EOS; ) {
					if (templineC3[j] == let && templineC3[j+1] == buf[i]) {
						k = j + 2;
						while (!isalpha(templineC3[k]) && templineC3[k]!=EOS) k++;
						strcpy(templineC3+j, templineC3+k);
					} else j++;
				}
				break;
			}
		}
		while (isdigit(buf[i])) i++;
	} else {
		if (*s != EOS)
			strcat(s, " -");
		else
			strcat(s, "-");
	}
	return(i);
}

static void PrintRow(char points[POINTS_N][POINTS_S], char *line, float total, char ans, FILE *fp) {
	register int i;
	register int col;
	register int rpos;
	register int pointsNameLen;
	char ftime = TRUE, isBulletFound;
	int done = FALSE;
	int l_index = 0, index[POINTS_N];

//fprintf(fpout, "%s\n", uttline); // 2021-09-26
	while (!done) {
		if (l_index && line[l_index]) {
			col = 4;
			fputs("    ", fp);
		} else
			col = 0;
		rpos = l_index;
		isBulletFound = FALSE;
		for (; col < 39 && line[rpos]; rpos++) {
			if (line[rpos] == HIDEN_C) {
				isBulletFound = !isBulletFound;
			}
			if (!isBulletFound && (UTF8_IS_SINGLE((unsigned char)line[rpos]) || UTF8_IS_LEAD((unsigned char)line[rpos])))
				col++;
		}
		if (line[rpos]) {
			for (; isalnum((unsigned char)line[rpos]) && rpos >= l_index; rpos--)
				col--;
			if (rpos < l_index) {
				rpos += 39;
				col = 39;
			} else if (col < 39) {
				rpos++;
				col++;
			}
		}
		for (; l_index < rpos && line[l_index] != EOS; l_index++)
			putc(line[l_index], fp);
		for (; col < 39; col++)
			putc(' ', fp);
		putc('|', fp);

		done = TRUE;
		for (i=0; i < PointsCnt; i++) {
			pointsNameLen = strlen(PointsNames[i]);
			if (ftime)
				index[i] = 0;
			col = 0;
			while (points[i][index[i]] == ' ')
				index[i]++;
			if (points[i][index[i]]) {
				for (; points[i][index[i]] && col < pointsNameLen; col++, index[i]++) {
					if (points[i][index[i]] == '0')
						putc('-', fp);
					else
						putc(points[i][index[i]], fp);
				}
				if (done)
					done = (points[i][index[i]] == EOS);
			}
			for (; col < pointsNameLen; col++)
				putc(' ', fp);
			putc('|', fp);
		}
		if (total > -1.0) {
			if (ftime) {
				fprintf(fp, "%1d|", ((ans == 'p' || ans == 'P') ? 1 : 0));
				i = (int)total;
				fprintf(fp, "%3d|", i);
				TotalsTotal = TotalsTotal + i;
			} else
				fprintf(fp, " |   |");
		}
		putc('\n', fp);
		ftime = FALSE;
		done = ((done) && (line[l_index] == EOS));
	}
}

static char HandleNoAnswer(char points[POINTS_N][POINTS_S], char *line) {
	register int i;
	char *s;
	char buf[BUFSIZ];

	s = NULL;
	do {
		fprintf(stderr, "\r           Sentence                    |");
		for (i=0; i < PointsCnt; i++)
			fprintf(stderr, "%s|", PointsNames[i]);
		fprintf(stderr, "\n");
		PrintRow(points, line, -1.0, '\0', stderr);
		fputs("\nTo edit type: category +/- point value (e.g. ip+4 or vm-2, no spaces)\n",stderr);
		fputs("          or: p = sentence pt/continue; n = no sentence pt/continue;\n", stderr);
		fputs("          or: q = Quit\n", stderr);
		fprintf(stderr, "=> ");
#ifndef UNX
		StdInErrMessage = "Please finish \"dss\" answer first.";
		StdDoneMessage  = "Done with \"dss\" answer mode.";
#endif
		for (i=0; (templineC4[i]=(char)getc(stdin)) != '\n' && i < BUFSIZ-2; i++) ;
		templineC4[i++] = ' ';
		templineC4[i] = EOS;

#ifdef _WIN32 
		if (isKillProgram == 2) {
			if (debug_dss_fp != stdout)
				fclose(debug_dss_fp);
			debug_dss_fp = stdout;
			cutt_exit(0);
		}
#endif
		strcpy(buf, templineC4);
		for (i=0; buf[i] == ' ' || buf[i] == '\t'; i++) ;
		if ((buf[i]=='p' || buf[i]=='P') && buf[i+1]==' ' && buf[i+2]==EOS) {
			return('p');
		} else if ((buf[i]=='n' || buf[i]=='N') && buf[i+1]==' ' && buf[i+2]==EOS) {
			return('n');
		} else if ((buf[i]=='q' || buf[i]=='Q') && buf[i+1]==' ' && buf[i+2]==EOS) {
#ifdef UNX
			exit(1);
#else
			isKillProgram = 1;
#endif
			return('q');
		}
		if (dss_lang == 'e') {
			for (i=0; buf[i] != EOS; i++) {
				if (uS.mStrnicmp(buf+i, "ip", 2) == 0) {
					s = points[0]; i = FinishPointsFixing(s,buf,i,0+'A');
				} else if (uS.mStrnicmp(buf+i, "pp", 2) == 0) {
					s = points[1]; i = FinishPointsFixing(s,buf,i,1+'A');
				} else if (uS.mStrnicmp(buf+i, "mv", 2) == 0) {
					s = points[2]; i = FinishPointsFixing(s,buf,i,2+'A');
				} else if (uS.mStrnicmp(buf+i, "sv", 2) == 0) {
					s = points[3]; i = FinishPointsFixing(s,buf,i,3+'A');
				} else if (uS.mStrnicmp(buf+i, "ng", 2) == 0) {
					s = points[4]; i = FinishPointsFixing(s,buf,i,4+'A');
				} else if (uS.mStrnicmp(buf+i, "cnj", 3) == 0) {
					s = points[5]; i = FinishPointsFixing(s,buf,i,5+'A');
				} else if (uS.mStrnicmp(buf+i, "ir", 2) == 0) {
					s = points[6]; i = FinishPointsFixing(s,buf,i,6+'A');
				} else if (uS.mStrnicmp(buf+i, "whq", 3) == 0) {
					s = points[7]; i = FinishPointsFixing(s,buf,i,7+'A');
				} else {
					fputs("Legal codes are: ip, pp, mv, sv, ng, cnj, ir, whq\n\n",stderr);
					break;
				}
			}
		} else if (dss_lang == 'j') {
			for (i=0; buf[i] != EOS; i++) {
				if (uS.mStrnicmp(buf+i, "vm", 2) == 0) {
					s = points[0]; i = FinishPointsFixing(s,buf,i,0+'A');
				} else if (uS.mStrnicmp(buf+i, "vl", 2) == 0) {
					s = points[1]; i = FinishPointsFixing(s,buf,i,1+'A');
				} else if (uS.mStrnicmp(buf+i, "adj", 3) == 0) {
					s = points[2]; i = FinishPointsFixing(s,buf,i,2+'A');
				} else if (uS.mStrnicmp(buf+i, "cop", 3) == 0) {
					s = points[3]; i = FinishPointsFixing(s,buf,i,3+'A');
				} else if (uS.mStrnicmp(buf+i, "cnj", 3) == 0) {
					s = points[4]; i = FinishPointsFixing(s,buf,i,4+'A');
				} else if (uS.mStrnicmp(buf+i, "np", 2) == 0) {
					s = points[5]; i = FinishPointsFixing(s,buf,i,5+'A');
				} else if (uS.mStrnicmp(buf+i, "cas", 3) == 0) {
					s = points[6]; i = FinishPointsFixing(s,buf,i,6+'A');
				} else if (uS.mStrnicmp(buf+i, "adv", 3) == 0) {
					s = points[7]; i = FinishPointsFixing(s,buf,i,7+'A');
				} else if (uS.mStrnicmp(buf+i, "smo", 3) == 0) {
					s = points[8]; i = FinishPointsFixing(s,buf,i,8+'A');
				} else if (uS.mStrnicmp(buf+i, "fin", 3) == 0) {
					s = points[9]; i = FinishPointsFixing(s,buf,i,9+'A');
				} else {
					fputs("Legal codes are: vm, vl, adj, cop, cnj, np, casfq, adv, smod, finp\n\n",stderr);
					break;
				}
			}
		}
	} while (1) ;
}

float PrintOutDSSTable(char *line, int spNum, int spRowNum, char isOutput) {
	register int i;
	register int index;
	float total, num, totalPerCell;
	char *s, ans;
	char points[POINTS_N][POINTS_S];
	char sFName[FILENAME_MAX];

	for (i=0; i < PointsCnt; i++)
		points[i][0] = EOS;
	for (s=templineC3; *s; ) {
		if (isalpha(*s)) {
			i = (int)(*s - 'A');
			if (i >= PointsCnt) {
				if (isdigit(*(s+1))) {
					fprintf(stderr, "DSS: ERROR: Illegal code letter: '%c'.\n", *s);
					if (debug_dss_fp != stdout)
							fclose(debug_dss_fp);
					dss_freeSpeakers();
					dss_cleanSearchMem();
					cutt_exit(0);
				} else {
					s++;
					continue;
				}
			}
			index = strlen(points[i]);
			if (index >= POINTS_S-1) {
				fprintf(stderr, "DSS: ERROR: Too many codes.\n");
				if (debug_dss_fp != stdout)
					fclose(debug_dss_fp);
				dss_freeSpeakers();
				dss_cleanSearchMem();
				cutt_exit(0);
			}
			if (index > 0)
				points[i][index++] = ' ';
			for (s++; isdigit(*s); s++)
				points[i][index++] = *s;
			points[i][index] = EOS;
		} else
			s++;
	}

	total = 0.0;
	if (DSSAutoMode) {
		if (dss_lang == 'e') {
			ans = 'p';
			for (i=0; line[i] != EOS; i++) {
				if (line[i] == '0' && (i == 0 || uS.isskip(line,i-1,&dFnt,C_MBF))) {
					ans = 'n';
					break;
				} else if (line[i] == '[' && line[i+1] == '*' && line[i+2] == ' ' &&  (line[i+3] == 'p' || line[i+3] == 'P')) {
				} else if (line[i] == '[' && line[i+1] == '*' && (line[i+2] == ']' ||  line[i+2] == ' ')) {
					ans = 'n';
					break;
				}
			}
		} else if (dss_lang == 'j') {
			ans = 'n';
		} else
			ans = 'n';
	} else {
		ans = HandleNoAnswer(points, line);
	}
	if (ans == 'p' || ans == 'P')
		total++;

	if (IsOutputSpreadsheet == 1) {
		excelRow(fpout, ExcelRowStart);
		s = strrchr(oldfname, PATHDELIMCHR);
		if (s == NULL)
			s = oldfname;
		else
			s++;
		strcpy(sFName, s);
		s = strrchr(sFName, '.');
		if (s != NULL)
			*s = EOS;
		excelStrCell(fpout, sFName);
		totalPerCell = (float)spNum;
		excelNumCell(fpout, "%.0f", totalPerCell);
		totalPerCell = (float)spRowNum;
		excelNumCell(fpout, "%.0f", totalPerCell);
	}
	for (i=0; i < PointsCnt; i++) {
		totalPerCell = 0.0;
		for (s=points[i]; *s; s++) {
			if (isdigit(*s)) {
				num = atoi(s);
				total += (float)num;
				totalPerCell += (float)num;
				TotalsAcrossPoints[i] = TotalsAcrossPoints[i] + num;
			}
		}
		if (isOutput && IsOutputSpreadsheet == 1) {
			excelNumCell(fpout, "%.0f", totalPerCell);
		}
	}
	if (isOutput) {
		if (IsOutputSpreadsheet == 1) {
			totalPerCell = ((ans == 'p' || ans == 'P') ? 1.0000 : 0.0000);
			excelNumCell(fpout, "%.0f", totalPerCell);
			totalPerCell = (float)total;
			excelNumCell(fpout, "%.0f", totalPerCell);
			excelRow(fpout, ExcelRowEnd);
		} else if (IsOutputSpreadsheet == 2) {
		} else {
			PrintRow(points, line, total, ans, fpout);
		}
	}
	return(total);
}

int PassedDSSMorTests(char *osp, char *mor, char *dss) {
	int  i, j, pos;
	char isSubtest, *dssS, *dssV, triples[5];

/*
	printf("osp=%s", osp);
	printf("mor=%s\n", mor);
	printf("dss=%s\n", dss);
    if passes subject test lxs
*/

	if (isDSSpostcode) {
		if (isPostCodeOnUtt(osp, "[+ dss]")) {
			if (debug_level > 0) {
				fprintf(debug_dss_fp, "[+ dss] postcode found, test SUCCEEDED\n");
			}
			return(TRUE);
		}
	}
	if (isDSSEpostcode) {
		if (isPostCodeOnUtt(osp, "[+ dsse]")) {
			if (debug_level > 0) {
				fprintf(debug_dss_fp, "[+ dsse] postcode found, test FAILED\n");
			}
			return(FALSE);
		}
	}
	if (dss_lang == 'e') {
		j = 0;
		for (pos=0; osp[pos]; pos++) {
			i = 0;
			if (pos == 0 || uS.isskip(osp,pos-1,&dFnt,MBF)) {
				for (j=pos; osp[j] == 'x' || osp[j] == 'X' ||
					 osp[j] == 'y' || osp[j] == 'Y' ||
					 osp[j] == 'w' || osp[j] == 'W' || 
					 osp[j] == '(' || osp[j] == ')'; j++) {
					if (osp[j] != '(' && osp[j] != ')')
						triples[i++] = osp[j];
				}
			}
			if (i == 3) {
				if (isSpace(osp[j]) || osp[j] == '@')
					triples[i] = EOS;
				else
					triples[0] = EOS;
			} else
				triples[i] = EOS;
			uS.lowercasestr(triples, &dFnt, FALSE);
			if (strcmp(triples, "xxx") == 0) {
				if (debug_level > 0) {
					fprintf(debug_dss_fp, "xxx found, test FAILED\n");
				}
				return(FALSE);
			}
			if (strcmp(triples, "yyy") == 0) {
				if (debug_level > 0) {
					fprintf(debug_dss_fp, "yyy found, test FAILED\n");
				}
				return(FALSE);
			}
			if (strcmp(triples, "www") == 0) {
				if (debug_level > 0) {
					fprintf(debug_dss_fp, "www found, test FAILED\n");
				}
				return(FALSE);
			}
		}
		for (; *mor != '!' && *mor != '?' && *mor != '.' && *mor != EOS; mor++) ;
		if (*mor == '!' || *mor == '?') {
			if (debug_level > 0) {
				fprintf(debug_dss_fp, "'%c' found, ", *mor);
			}
			for (dssV=dss; *dssV != EOS; dssV++) {
				if (*dssV == 'C' || *dssV == 'D') {
					if (debug_level > 0) {
						fprintf(debug_dss_fp, "'%c' found, test SUCCEEDED\n", *dssV);
					}
					return(TRUE);
				}
			}
			if (debug_level > 0) {
				fprintf(debug_dss_fp, "no following 'C' or 'D' found, test FAILED\n");
			}
		} else if (*mor == '.') {
			if (debug_level > 0) {
				fprintf(debug_dss_fp, "'.' found, ");
			}
			isSubtest = FALSE;
			for (dssS=dss; *dssS != EOS; dssS++) {
				if (isSubtest) {
					if (debug_level > 0) {
						fprintf(debug_dss_fp, "\n");
					}
				}
				if (*dssS == 'A' || *dssS == 'B' || *dssS == 'Z') {
					isSubtest = TRUE;
					if (debug_level > 0) {
						fprintf(debug_dss_fp, "'%c' found, ", *dssS);
					}
					for (dssV=dss; *dssV != EOS; dssV++) {
						if (*dssV == 'C' || *dssV == 'D') {
							if (debug_level > 0) {
								fprintf(debug_dss_fp, "'%c' found, test SUCCEEDED\n", *dssV);
							}
							return(TRUE);
						}
					}
				}
			}
			if (debug_level > 0) {
				if (!isSubtest)
					fprintf(debug_dss_fp, "'A', 'B' or 'Z' not found, ");
				else
					fprintf(debug_dss_fp, "no following 'C' or 'D' found, ");
				fprintf(debug_dss_fp, "test FAILED\n");
			}
		}
	} else if (dss_lang == 'j') {
		return(TRUE);
	}
	return(FALSE);
}

static void PrintTotalRow(FILE *fp) {
	register int i;
	register int col;
	register int pointsNameLen;

	fprintf(fp, "TOTAL");
	for (col=5; col < 39; col++)
		putc(' ', fp);
	putc('|', fp);
	for (i=0; i < PointsCnt; i++) {
		pointsNameLen = strlen(PointsNames[i]);
		fprintf(fp, "%3d", TotalsAcrossPoints[i]);
		for (col=3; col < pointsNameLen; col++)
			putc(' ', fp);
		putc('|', fp);
	}
	fprintf(fp, " |%3d|", TotalsTotal);
	putc('\n', fp);
}

static void PrintTotalSpRow(FILE *fp, float TotalUtts, DSSSP *tsp) {
	int  i;
	char *s, sFName[FILENAME_MAX];
	float tf;
	
	excelRow(fpout, ExcelRowStart);
	s = strrchr(oldfname, PATHDELIMCHR);
	if (s == NULL)
		s = oldfname;
	else
		s++;
	strcpy(sFName, s);
	excelStrCell(fpout, sFName);
	if (tsp->ID) {
		excelOutputID(fpout, tsp->ID);
	} else {
		excelCommasStrCell(fpout, ".,.");
		excelStrCell(fpout, tsp->sp);
		excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.,.,.,.");
	}
	excelNumCell(fpout, "%.0f", TotalUtts);
	excelNumCell(fpout, "%.2f", tsp->TotalNum);
	for (i=0; i < PointsCnt; i++) {
		tf = (float)TotalsAcrossPoints[i];
		excelNumCell(fpout, "%.0f", tf);
	}
//	excelNumCell(fpout, "%.0f", 0.0);
	excelNumCell(fpout, "%.0f", tsp->GrandTotal);
	excelRow(fpout, ExcelRowEnd);
}

char make_DSS_tier(char *mor_tier) {
	register int k;
	
	for (k=0; mor_tier[k]; k++) {
		if (mor_tier[k] == '\n' || mor_tier[k] == '\t')
			mor_tier[k] = ' ';
	}
	for (k--; k >= 0 && mor_tier[k] == ' '; k--) ;
	k++;
	mor_tier[k] = EOS;
	if (mor_tier[k-1] != ' ') {
		mor_tier[k++] = ' ';
		mor_tier[k] = EOS;
	}
	freestack();
	if (!TakeAction(INITSTACK, FALSE))
		return(FALSE);
	if (!dss_findmatch(mor_tier))
		return(FALSE);
	return(TRUE);
}

char isDSSRepeatUtt(DSSSP *tsp, char *line) {
	int  k;
	char isEmpty;

	isEmpty = TRUE;
	for (k=0; line[k] != EOS; k++) {
		if (!isSpace(line[k]) && line[k] != '\n' && line[k] != '.' && line[k] != '!' && line[k] != '?')
			isEmpty = FALSE;
	}
	if (isEmpty)
		return(TRUE);
	for (k=0; k < tsp->uttnum; k++) {
		if (strcmp(tsp->utts[k], line) == 0)
			break;
	}
	if (k == tsp->uttnum)
		return(FALSE);
	return(TRUE);
}

char addToDSSUtts(DSSSP *tsp, char *line) {
	int num;

	num = tsp->uttnum;
	tsp->utts[num] = (char *)malloc(strlen(line)+1);
	if (tsp->utts[num] == NULL) {
		return(FALSE);
	}
	strcpy(tsp->utts[num], line);
	tsp->uttnum++;
	return(TRUE);
}

char isUttsLimit(DSSSP *tsp) {
	if (tsp->uttnum >= DSS_UTTLIM)
		return(TRUE);
	return(FALSE);
}

DSSSP *dss_FindSpeaker(char *sp, char *ID) {
	int i;
	DSSSP *tsp;

	uS.remblanks(sp);
	if (dss_sp_head.sp[0] == EOS) {
		tsp = &dss_sp_head;
	} else {
		for (tsp=&dss_sp_head; 1; tsp=tsp->next_sp) {
			if (uS.partcmp(sp, tsp->sp, FALSE, FALSE)) {
				return(tsp);
			}
			if (tsp->next_sp == NULL)
				break;
		}
		tsp->next_sp = NEW(DSSSP);
		tsp = tsp->next_sp;
	}
	if (tsp == NULL)
		out_of_mem();
	strncpy(tsp->sp, sp, DSSSPLEN);
	tsp->sp[DSSSPLEN] = EOS;
	if (ID == NULL)
		tsp->ID = NULL;
	else {
		if ((tsp->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			return(NULL);
		strcpy(tsp->ID, ID);
	}
	tsp->utts = (char **)malloc((DSS_UTTLIM+1)*sizeof(char *));
	if (tsp->utts == NULL)
		out_of_mem();
	for (i=0; i < DSS_UTTLIM; i++) {
		tsp->utts[i] = NULL;
	}
	for (i=0; i < PointsCnt; i++) {
		tsp->TotalsPoints[i] = 0;
	}
	tsp->uttnum = 0;
	tsp->TotalNum = 0.0;
	tsp->GrandTotal = 0.0;
	tsp->next_sp = NULL;
	return(tsp);
}

#ifndef KIDEVAL_LIB
void call() {
	unsigned int ln = 0;
	int  k, spRowNum;
	char isCRFound;
	char DSSFound = FALSE;
	char lRightspeaker;
	char delay = '\0';
	char tspr[SPEAKERLEN];
	float TotalUtts;
	DSSSP *tsp, *lastTsp;
	FNType tfname[FNSize];
	FILE *tf;

	if (debug_level > 0 && debug_dss_fp == stdout) {
		if (!isRefEQZero(wd_dir)) {		/* we have a lib */
			strcpy(debugDssName, wd_dir);
		} else
			*debugDssName = EOS;
		addFilename2Path(debugDssName, "0-DSS-debug.cdc");
		if ((debug_dss_fp=fopen(debugDssName,"w")) == NULL) {
			fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugDssName);
			debug_dss_fp = stdout;
		} else {
#ifdef _MAC_CODE
			settyp(debugDssName, 'TEXT', the_file_creator.out, FALSE);
#endif
			printf("*** File \"%s\"\n", debugDssName);
			fprintf(debug_dss_fp,"%s\n", UTF8HEADER);
		}
	}
	for (k=0; k < PointsCnt; k++)
		TotalsAcrossPoints[k] = 0;
	TotalsTotal = 0;

	if (INITSTACK == NULL) {
		fprintf(stderr,"STARTRULES is missing!\n");
		dss_freeSpeakers();
		dss_cleanSearchMem();
		if (debug_dss_fp != stdout)
			fclose(debug_dss_fp);
		cutt_exit(0);
	}
	if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
		spNum++;
		spRowNum = 0;
		tfp = NULL;
		if (isFirstCall) {
			isFirstCall = FALSE;
			if (IsOutputSpreadsheet == 1) {
				excelHeader(fpout, newfname, 60);
				excelRow(fpout, ExcelRowStart);
				excelStrCell(fpout,"File");
				excelCommasStrCell(fpout,"ID,Sentence");
				for (k=0; k < PointsCnt; k++)
					excelStrCell(fpout,PointsNames[k]);
				excelCommasStrCell(fpout,"S,Total");
				excelRow(fpout, ExcelRowEnd);
			} else if (IsOutputSpreadsheet == 2) {
				excelHeader(fpout, newfname, 75);
				excelRow(fpout, ExcelRowStart);
				excelStrCell(fpout,"File");
				excelCommasStrCell(fpout, "Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
				excelCommasStrCell(fpout, "DSS_Utts,DSS");
				for (k=0; k < PointsCnt; k++)
					excelStrCell(fpout,PointsNames[k]);
//				excelCommasStrCell(fpout,"S");
				excelCommasStrCell(fpout,"Total");
				excelRow(fpout, ExcelRowEnd);
			}
		}
	} else {
		parsfname(oldfname, tfname, ".dss");
		if ((tfp=openwfile(oldfname,tfname,tfp)) == NULL) {
			fprintf(stderr,"WARNING: Can't create dss file %s.\n", tfname);
		}
		for (tsp=&dss_sp_head; tsp->next_sp != NULL; tsp=tsp->next_sp) {
			tsp->TotalNum = 0.0;
			tsp->GrandTotal = 0.0;
		}
		if (dss_lang != 'j')
			fprintf(fpout, "%s	CAfont:13:7\n", FONTHEADER);
		fprintf(fpout, "\nDevelopmental Sentence Analysis\n\n");
		fprintf(fpout, "           Sentence                    |");
		for (k=0; k < PointsCnt; k++)
			fprintf(fpout, "%s|", PointsNames[k]);
		fprintf(fpout, "S|TOT|\n");
	}
	tsp = NULL;
	lastTsp = NULL;
	lRightspeaker = FALSE;
	spareTier1[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		ln++;
#if !defined(CLAN_SRV)
		if (ln % 25 == 0) {
			if (!stout)
				fprintf(stderr,"\r%u ",ln);
		}
#endif
		if (debug_level > 0 && !uS.isUTF8(utterance->speaker) && *utterance->line != EOS) {
			fprintf(debug_dss_fp, "%s%s", utterance->speaker, uttline);
			if (uttline[strlen(uttline)-1] != '\n')
				putc('\n', debug_dss_fp);
		}
		if (*utterance->speaker == '@') {
		} else if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*') {
				lRightspeaker = FALSE;
				cleanUttline(uttline);
				strcpy(spareTier1, uttline);
			}
			continue;
		} else {
			if (*utterance->speaker == '*') {
				lRightspeaker = TRUE;
				tsp = NULL;
			}
			if (!lRightspeaker)
				continue;
		}
		if (*utterance->speaker == '@') {
			if (tfp) {
				fputs(utterance->speaker,tfp);
				isCRFound = FALSE;
				for (k=0; utterance->line[k]; k++) {
					if (utterance->line[k] == '\n')
						isCRFound = TRUE;
					putc(utterance->line[k],tfp);
				}
				if (!isCRFound)
					putc('\n',tfp);
			}
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC2, TRUE)) {
					uS.remblanks(utterance->line);
					if (dss_FindSpeaker(templineC2, utterance->line) == NULL)
						out_of_mem();
				}
			} else if (uS.partcmp(utterance->speaker,"@DSS",FALSE,FALSE)) {
				for (k=0; utterance->line[k] == ' ' || 
						  utterance->line[k] == '\t'||
						  utterance->line[k] == '\n'; k++) ;
				if (utterance->line[k] == EOS) {
					if (delay == '\0')
						delay = '\002';
				} else
					delay = '\001';
			}
		} else if (*utterance->speaker == '*') {
			if (delay == '\0')
				delay = '\001';
			if (delay != '\001') {
				continue;
			}
			if (lastTsp != NULL) {
				freeLastDSSUtt(lastTsp);
				lastTsp = NULL;
			}
			strcpy(templineC2, utterance->speaker);
			tsp = dss_FindSpeaker(templineC2, NULL);
			if (tsp == NULL)
				out_of_mem();
			if (isUttsLimit(tsp)) {
				DSSFound = TRUE;
				break;
			}
			cleanUttline(uttline);
			if (strcmp(uttline, spareTier1) == 0) {
				spareTier1[0] = EOS;
				lRightspeaker = FALSE;
			} else {
/* 2013-06-10
				strcpy(templineC2, utterance->speaker);
				tsp = dss_FindSpeaker(templineC2, NULL);
				if (tsp == NULL)
					out_of_mem();
*/
				if (!isDSSRepeatUtt(tsp, uttline)) {
					if (!addToDSSUtts(tsp, uttline)) {
						if (tfp) {
							fclose(tfp);
							tfp = NULL;
						}
						out_of_mem();
					}
					strcpy(tspr, utterance->speaker);
					strcpy(spareTier1, uttline);
					strcpy(spareTier2, utterance->line);
					lastTsp = tsp;
				} else {
					spareTier1[0] = EOS;
					lRightspeaker = FALSE;
				}
			}
		} else if (tsp != NULL && fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
/* 2020-08-09
			for (k=0; uttline[k] != EOS; k++) {
				if (uttline[k] == '~' || uttline[k] == '$')
					uttline[k] = ' ';
			}
*/
			DSSFound = TRUE;
			if (!make_DSS_tier(uttline)) {
				dss_freeSpeakers();
				dss_cleanSearchMem();
				cutt_exit(0);
			}
			lastTsp = NULL;
			if (PassedDSSMorTests(spareTier2,uttline,templineC3)) {
				tsp->TotalNum = tsp->TotalNum + 1;
				for (k=0; spareTier2[k] != EOS; k++) {
					if (spareTier2[k]== '\n' || spareTier2[k]== '\t')
						spareTier2[k] = ' ';
				}
				for (k--; spareTier2[k] == ' '; k--) ;
				spareTier2[++k] = EOS;
				strcpy(templineC4, spareTier2);
				changeBullet(templineC4, NULL);
				spRowNum++;
				tsp->GrandTotal = tsp->GrandTotal + PrintOutDSSTable(templineC4, spNum, spRowNum, TRUE);
				if (IsOutputSpreadsheet == 2) {
					int i;
					for (i=0; i < PointsCnt; i++) {
						tsp->TotalsPoints[i] = TotalsAcrossPoints[i];
					}
				}
				if (tfp) {
					tf = fpout;
					fpout = tfp;
					printout(tspr,spareTier2,NULL,NULL,TRUE);
					printout(utterance->speaker,uttline,NULL,NULL,TRUE);
					if (*templineC3)
						printout("%xdss:",templineC3,NULL,NULL,TRUE);
					fpout = tf;
				}
			} else {
				freeLastDSSUtt(tsp);
			}
		}
	}
	if (!stout)
		fprintf(stderr,"\n");
	if (IsOutputSpreadsheet != 1 && IsOutputSpreadsheet != 2)
		PrintTotalRow(fpout);
	for (tsp=&dss_sp_head; tsp != NULL; tsp=tsp->next_sp) {
		TotalUtts = tsp->TotalNum;
		if (tsp->TotalNum) {
			if (tsp->TotalNum < DSS_UTTLIM) {
				if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
					if (DSS_UTTLIM == 50)
						fprintf(stderr, "WARNING: DSS requires 50 complete sentences and this transcript only has %.0f.\n", tsp->TotalNum);
					else
						fprintf(stderr, "WARNING: You asked for %d complete sentences and this transcript only has %.0f.\n", DSS_UTTLIM, tsp->TotalNum);
				} else {
					if (DSS_UTTLIM == 50)
						fprintf(fpout, "WARNING: DSS requires 50 complete sentences and this transcript only has %.0f.\n", tsp->TotalNum);
					else
						fprintf(fpout, "WARNING: You asked for %d complete sentences and this transcript only has %.0f.\n", DSS_UTTLIM, tsp->TotalNum);
				}
			}
			GTotalNum += tsp->TotalNum;
			GGrandTotal += tsp->GrandTotal;
			tsp->TotalNum = tsp->GrandTotal / tsp->TotalNum;
			if (IsOutputSpreadsheet == 2)
				PrintTotalSpRow(fpout, TotalUtts, tsp);
			else if (IsOutputSpreadsheet != 1)
				fprintf(fpout, "\nDevelopmental sentence score: %5.2f\n",tsp->TotalNum);
		} else {
			if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
				if (DSS_UTTLIM == 50)
					fprintf(stderr, "WARNING: DSS requires 50 complete sentences and this transcript only has %.0f.\n", tsp->TotalNum);
				else
					fprintf(stderr, "WARNING: You asked for %d complete sentences and this transcript only has %.0f.\n", DSS_UTTLIM, tsp->TotalNum);
			} else {
				fprintf(fpout, "\nDevelopmental sentence score: NA\n");
				if (DSS_UTTLIM == 50)
					fprintf(fpout, "WARNING: DSS requires 50 complete sentences and this transcript only has %.0f.\n", tsp->TotalNum);
				else
					fprintf(fpout, "WARNING: You asked for %d complete sentences and this transcript only has %.0f.\n", DSS_UTTLIM, tsp->TotalNum);
			}
		}
	}
	if (!combinput) {
		if (tfp) {
			fclose(tfp);
			tfp = NULL;
		}
		dss_freeSpeakers();
	} else 	if (IsOutputSpreadsheet == 1 || IsOutputSpreadsheet == 2) {
		dss_freeSpeakers();
	}
	if (!DSSFound) {
		fprintf(stderr, "\nTIER \"%%mor:\" HASN'T BEEN FOUND IN FILE \"%s\"!\n", oldfname);
	}
}

void getflag(char *f, char *f1, int *i) {
	char *s;

	f++;
	switch(*f++) {
		case 'a':
			if (!*f) {
				fprintf(stderr,"Number 1, 2, 3 expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			debug_level = atoi(f);
			break;
		case 'c':
			if ((DSS_UTTLIM=atoi(f)) <= 0)
				DSS_UTTLIM = 50;
			break;
		case 'd': 
			if (*f == EOS) {
				IsOutputSpreadsheet = 1;
			} else if (*f == '1') {
				IsOutputSpreadsheet = 2;
			} else {
				rulesfile = getfarg(f,f1,i);
				isDssFileSpecified = TRUE;
			}
			break;
		case 'e':
			DSSAutoMode = TRUE;
			no_arg_option(f);
			break;
		case 'i':
			DSSAutoMode = FALSE;
			no_arg_option(f);
			break;
		case 'l':
			if (!*f) {
				fprintf(stderr,"Letter expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			if (*f == 'e' || *f == 'E') {
				dss_lang = 'e';
				if (rulesfile == NULL)
					rulesfile = DSSRULES;
			} else if (*f == 'j' || *f == 'J') {
				dss_lang = 'j';
				if (rulesfile == NULL)
					rulesfile = DSSJPRULES;
			} else {
				fprintf(stderr, "This language is not supported: %s.\n", f);
				fprintf(stderr, "Choose: eng - English, jpn - Japanese\n");
				cutt_exit(0);
			}
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
			if (*f == '%') {
				strcpy(fDepTierName, f);
				break;
			} else if (*f == '@') {
				break;
			}
		case 's':
			s = f;
			if (*s == '+' || *s == '~')
				s++;
			if (*s == '[' || *s == '<') {
				s++;
				if (*s == '+') {
					for (s++; isSpace(*s); s++) ;
					if ((*s     == 'd' || *s     == 'D') && 
						(*(s+1) == 's' || *(s+1) == 'S') && 
						(*(s+2) == 's' || *(s+2) == 'S') && 
						(*(s+3) == ']' || *(s+3) == '>' )) {
						isDSSpostcode = FALSE;
						break;
					}
					if ((*s     == 'd' || *s     == 'D') && 
						(*(s+1) == 's' || *(s+1) == 'S') && 
						(*(s+2) == 's' || *(s+2) == 'S') && 
						(*(s+3) == 'e' || *(s+3) == 'E') && 
						(*(s+4) == ']' || *(s+4) == '>' )) {
						isDSSEpostcode = FALSE;
						break;
					}
				}
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
#endif // KIDEVAL_LIB
