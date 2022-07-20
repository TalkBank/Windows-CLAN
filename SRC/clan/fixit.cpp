/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"

#if !defined(UNX)
#define _main fixit_main
#define call fixit_call
#define getflag fixit_getflag
#define init fixit_init
#define usage fixit_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define PERIOD 50
#define HEADT struct codeT
#define TIERS struct tierT
#define CODES struct codeT

struct codeT {
	char *type;
	char *code;
	CODES *NextCode;
} ;

struct tierT {
	char *tier;
	int NumWords;
	CODES *codes;
	TIERS *NextTier;
} ;

static char OldSpeaker[SPEAKERLEN];
static char *temp_uttline;
static char check_mode;
static int  fixit_interturn;
static TIERS *RootTier;
static HEADT *HeadTier;

extern struct tier *defheadtier;
extern char OverWriteFile;


void usage() {
   puts("FIXIT converts tiers with multiple utterances to single.");
   printf("Usage: fixit [c %s] filename(s)\n",mainflgs());
   puts("+c : find all the errors possible in one pass.");
   mainusage(TRUE);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		fixit_interturn = 0;
		RootTier = NULL;
		*OldSpeaker = EOS;
		FilterTier = 0;
		OverWriteFile = TRUE;
		check_mode = FALSE;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		temp_uttline = (char *)malloc(UTTLINELEN+1);
		onlydata = 3;
	}
	HeadTier = NULL;
	*OldSpeaker = EOS;
}

static void fixit_remblanks(char *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (st[i] == ' ' || st[i] == '\t' || st[i] == '\n' ||
					  st[i] == ',' || st[i] == ';')) i--;
	st[i+1] = EOS;
}

static void fixit_overflow() {
	fprintf(stderr,"Fixit: Out of memory.\n");
	if (!stout) {
		fclose(fpout);
		if (!check_mode) unlink(newfname);
	}
	cutt_exit(1);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = FIXIT;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	free(temp_uttline);
}

static void free_tiers() {
	TIERS *t;
	CODES *code, *c;

	while (RootTier != NULL) {
		t = RootTier;
		RootTier = RootTier->NextTier;
		c = t->codes;
		code = c;
		while (code != NULL) {
			code = code->NextCode;
			free(c->type);
			free(c->code);
			free(c);
			c = code;
		}
		free(t->tier);
		free(t);
	}
	RootTier = NULL;
}

static char fixit_err(const char *s, const char *s1) {
	HEADT *TH;

	if (!check_mode) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		if (s1 != NULL)
			fprintf(stderr,"%s in:\n    %s\n",s1,s);
		else
			fprintf(stderr,"Illegal location representation: %s\n",s);
		free(temp_uttline);
		free_tiers();
		while (HeadTier != NULL) {
			TH = HeadTier;
			HeadTier = HeadTier->NextCode;
			free(TH->type);
			free(TH->code);
			free(TH);
		}
		unlink(newfname);
		cutt_exit(0);
	} else {
		fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		if (s1 != NULL)
			fprintf(fpout,"%s in:\n    %s\n",s1,s);
		else
			fprintf(fpout,"Illegal location representation: %s\n",s);
	}
	return(0);
}

static CODES *MkCodes(CODES *c, const char *scope, char *text) {
	CODES *new_code;
	char *s;

	for (new_code=c; new_code != NULL; new_code=new_code->NextCode)
		if (uS.partcmp(utterance->speaker,new_code->type,FALSE,FALSE)) break;
	if (new_code == NULL) {
		if ((new_code=NEW(CODES)) == NULL) fixit_overflow();
		new_code->NextCode = c;
		new_code->type = (char *)malloc(strlen(utterance->speaker)+1);
		if (new_code->type == NULL) fixit_overflow();
		strcpy(new_code->type,utterance->speaker);
		fixit_remblanks(text);
		new_code->code = (char *)malloc(strlen(scope)+strlen(text)+1);
		if (new_code->code == NULL) fixit_overflow();
		strcpy(new_code->code,scope);
		strcat(new_code->code,text);
		return(new_code);
	} else {
		s = new_code->code;
		new_code->code = (char *)malloc(strlen(s)+strlen(scope)+strlen(text)+2);
		if (new_code->code == NULL) fixit_overflow();
		strcpy(new_code->code,s);
		strcat(new_code->code," ");
		strcat(new_code->code,scope);
		strcat(new_code->code,text);
		free(s);
		return(c);
	}
}

static void SetScope(char *scope, int NumWords, 
						   int u1, int u2, int w1, int w2, int s1, int s2) {
	if (s1 > 0) sprintf(scope,"<%dw%ds", w1, s1);
	else sprintf(scope,"<%dw", w1);
	if (w1+w2 > NumWords) w2 = NumWords - w1;
	else if (u2 > 0 && w2 <= 0) w2 = NumWords - w1;
	if (w2 > 0) sprintf(scope+strlen(scope),"-%dw", w1+w2);
	else if (s2 > 0) sprintf(scope+strlen(scope),"-%dw%ds", w1+w2, s1+s2);
	strcat(scope,"> ");
}

static char AddCodes(int b1,int u1,int u2,int w1,int RealW1,int w2,int s1,int s2,
				char *s, char *text) {
	TIERS *t;
	char scope[50];

	if (b1 == -3) {
		RootTier->codes = MkCodes(RootTier->codes,"<bef> ",text);
		return(1);
	} else if (b1 == -1) {
		for (t=RootTier; t->NextTier != NULL; t=t->NextTier) ;
		t->codes = MkCodes(t->codes,"<aft> ",text);
		return(1);
	} else  if (b1 == -2) {
		for (t=RootTier; t != NULL; t=t->NextTier)
			t->codes = MkCodes(t->codes,"<dur> ",text);
		return(1);
	}
	for (t=RootTier; t != NULL && u1 > 1; t=t->NextTier) {
		u1--;
		if (t->NextTier == NULL && u1 == 1 && RealW1 == 0) {
			t->codes = MkCodes(t->codes,"<aft> ",text);
			return(1);
		}
	}
	if (t == NULL)
		return(fixit_err(s,"Scope refers to non-existent utterance"));
	while (1) {
		if (w1 - t->NumWords > 0) {
			w1 -= t->NumWords;
			if (t->NextTier == NULL && u1 == 1 && w1 == 1) {
				t->codes = MkCodes(t->codes,"<aft> ",text);
				return(1);
			}
			t = t->NextTier;
			if (t == NULL)
				return(fixit_err(s,"Scope refers to non-existent word"));
		} else break;
	}
	SetScope(scope,t->NumWords,u1,u2,w1,w2,s1,s2);
	t->codes = MkCodes(t->codes,scope,text);
	w2 = w2 - (t->NumWords - w1 + 1);
	if (u2 > 0 || w2 >= 0) {
		w1 = 1;
		while (u2 > 0 || w2 >= 0) {
			t = t->NextTier;
			if (t == NULL)
				return(fixit_err(s,"Scope refers to non-existent token"));
			SetScope(scope,t->NumWords,u1,u2,w1,w2,s1,s2);
			t->codes = MkCodes(t->codes,scope,text);
			w2 = w2 - (t->NumWords - w1 + 1);
			if (u2 > 0) u2--;
		}
	}
	return(1);
}

static char fixit_setdot(char *s, int *b1, int *b2, char frstnum, char numf) {
	if (*b1 < 0)
		return(fixit_err(s,"Dot '.' is not allowed with either <bef>, <dur> or <aft>."));
	if (frstnum) {
		if (*b1) return(fixit_err(s,"Only one dot '.' allowed"));
		if (!numf) *b1 = 1; else *b1 = 2;
	} else {

		if (*b2) return(fixit_err(s,"Only one dot '.' allowed"));
		if (!numf) *b2 = 1; else *b2 = 2;
	}
	return(1);
}

static char fixit_comppos(char *s, int u1, int w1, int s1, int b1, 
					  int u2, int w2, int s2, int b2, char *text) {
	int RealW1 = w1;

	if (b1 == 2) {
		if (s1) s1++;
		else if (w1) w1++;
		else if (u1) u1++;
	}
	if (b2 == 1) {
		if (s2) s2--;
		else if (w2) w2--;
		else if (u2) u2--;
	}
	if (u1 == 0) u1 = 1;
	if (w1 == 0) w1 = 1;
	if (u2 != 0) u2 -= u1;
	if (u2 < 0) return(fixit_err(s,"Second utterance must be located after the first one"));
	if (u2 == 0) {
		if (w2 != 0) w2 -= w1;
		if (w2 < 0) return(fixit_err(s,"Second word must be located after the first one"));
	}
	if (s1 != 0 && (w2 != 0 || u2 != 0))
		return(fixit_err(s,"The second segment must be in the same word"));
	if (s1 != 0 && s2 != 0) {
		s2 -= s1;
		if (s2<0) return(fixit_err(s,"Second segment must be located after the first one"));
	}
/*
fixit_remblanks(s);
printf("*** s=%s; ",s);
printf("b=%d,%d+%du,%d+%dw,%d+%ds;\n",b1,u1,u2,w1,w2,s1,s2);
*/
	return(AddCodes(b1,u1,u2,w1,RealW1,w2,s1,s2,s,text));
}
 
static char *fixit_getnum(char *st, int *u, int *w, int *s, int prev) {
	char *f, *l, t, *dot, sf = 0, uf = 0, wf = 0;
	int lev = 0;
 
	f = st;
	l = f;
	dot = NULL;
	while (*st != '>' && *st != ',' && *st != '-') {
		if (isdigit(*st)) {
			if (dot) return(NULL);
			lev++;
			f = st;
			for (; isdigit(*st); st++) ;
			l = st;
		} else {
			if (*st == '.')  dot = st;
			else
			if (*st == 'u') {
				if (dot) return(NULL);
				if (uf) return(NULL);
				t = *l;
				*l = EOS;
				*u = atoi(f);
				*l = t;
				f = l;
				uf = 1;
			} else
			if (*st == 'w') {
				if (dot) return(NULL);
				if (wf) return(NULL);
				t = *l;
				*l = EOS;
				*w = atoi(f);
				*l = t;
				f = l;
				wf = 1;
			} else 
			if (*st == 's') {
				if (dot) return(NULL);
				if (sf) return(NULL);
				t = *l;
				*l = EOS;
				*s = atoi(f);
				*l = t;
				f = l;
				sf = 1;
			}
			st++;
		}
	}
	if (lev < 1 || (lev > 2 && !sf) || lev > 3) return(NULL);
	if (f != l) {
		t = *l;
		*l = EOS;
		if (wf || prev) *s = atoi(f);
		else *w = atoi(f);
		*l = t;
	}
	if (dot) return(dot);
	else return(st);
}
 
static char ParsScope(char *st, char *text) {
	char cf = 0, numf = 0, frstnum = 1;
	char *s;
	int u1, w1, s1, b1, u2, w2, s2, b2;
 
	u1 = 0; w1 = 0; s1 = 0; b1 = 0; u2 = 0; w2 = 0; s2 = 0; b2 = 0;
	if (*st == '<') {
		s = st;
		st++;
		while (*st != '>') {
			if (isdigit(*st)) {
				if (cf) return(fixit_err(s,"Illegal characters found before numbers"));
				numf = 1;
				if (fixit_interturn) {
					fixit_interturn = atoi(st);
					for (; isdigit(*st); st++) ;
					for (; *st == ' ' || *st == '\t'; st++) ;
					if (*st != '>') return(fixit_err(s,"Illegal scoping"));
				} else {
					if (frstnum) {
						if ((st=fixit_getnum(st,&u1,&w1,&s1,0))==NULL) return(fixit_err(s,NULL));
					} else {
						if ((st=fixit_getnum(st,&u2,&w2,&s2,s1))==NULL) return(fixit_err(s,NULL));
					}
				}
			} else {
				if (*st == '.') {
					if (!fixit_setdot(s,&b1,&b2,frstnum,numf)) return(0);
				} else if (*st == '-') {
					if (!numf)
						return(fixit_err(s,"No position representation found"));
					numf = 0;
					if (frstnum)
						frstnum = 0;
					else
						return(fixit_err(s,"Too many '-' symbols"));
				} else if (*st == ',') {
					if (!numf)
						return(fixit_err(s,"No position representation found"));
					if (!fixit_comppos(s,u1,w1,s1,b1,u2,w2,s2,b2,text))
						return(0);
					numf = 0; frstnum = 1;
					u1= 0; w1= 0; s1= 0; b1= 0; u2= 0; w2= 0; s2= 0; b2= 0;
				} else if (*st == 'a' && *(st+1) == 'f' && *(st+2) == 't') {
					if (numf || !frstnum) 
						return(fixit_err(s,"<aft> can't be after any number"));
					numf = 1;
					if (b1 > 0) return(fixit_err(s,"Dot '.' is not allowed with <aft>."));
					b1 = -1;
					st += 2;

				} else if (*st == 'b' && *(st+1) == 'e' && *(st+2) == 'f') {
					if (numf || !frstnum) 
						return(fixit_err(s,"<bef> can't be after any number"));
					numf = 1;
					if (b1 > 0) return(fixit_err(s,"Dot '.' is not allowed with <bef>."));
					b1 = -3;
					st += 2;
				} else if (*st == 'd' && *(st+1) == 'u' && *(st+2) == 'r') {
					if (numf || !frstnum) 
						return(fixit_err(s,"<dur> can't be after any number"));
					numf = 1;
					if (b1 > 0) return(fixit_err(s,"Dot '.' is not allowed with <dur>."));
					b1 = -2;
					st += 2;
				} else if (*st == '$' && *(st+1) == '/' && *(st+2) == '=') {
					return(fixit_err(s,"<$/=n> is not implemented"));
//					fixit_interturn = 1;
//					st += 2;
				} else if (*st != ' ' && *st != '\t' && *st != '\n') cf = 1;
				st++;
			}
		}
		if (!numf) return(fixit_err(s,"No position representation found"));
	} else s = st;
	return(fixit_comppos(s,u1,w1,s1,b1,u2,w2,s2,b2,text));
}

static void fixit_pr_result() {
	HEADT *TH;
	TIERS *t;
	CODES *c;

	if (RootTier == NULL) {
		if (*OldSpeaker != EOS && !check_mode)
			fprintf(fpout,"%s\n", OldSpeaker);
	} else {
			if (!check_mode) {
			for (t=RootTier; t != NULL; t=t->NextTier) {
				printout(OldSpeaker, t->tier, NULL, NULL, TRUE);
				for (c=t->codes; c != NULL; c=c->NextCode)
					printout(c->type, c->code, NULL, NULL, TRUE);
			}
		}
		free_tiers();
	}
	while (HeadTier != NULL) {
		if (!check_mode) {
			printout(HeadTier->type, HeadTier->code, NULL, NULL, TRUE);
		}
		TH = HeadTier;
		HeadTier = HeadTier->NextCode;
		free(TH->type);
		free(TH->code);
		free(TH);
	}
	HeadTier = NULL;
}

static char Duplicate_Scoping() {
	char *s, *s1, *ts, sq, paran, LastAB;
	int ab = 0, ab1, ab2;

	sq = FALSE;
	paran = FALSE;
	s = uttline;
	ts = temp_uttline;
	while (1) {
		if (*s == '[') sq = TRUE;
		else if (*s == ']') sq = FALSE;
		else if (*s == '(') paran = TRUE;
		else if (*s == ')') paran = FALSE;
		else if (*s == '<') {
			if (s != uttline && *(s-1) == '+')
				;
			else if (!sq)
				ab++;
		} else if (*s == '>') {
			if (!sq)
				ab--;
		}
		if (uS.IsUtteranceDel(s, 0) && !sq && !paran) {
			if (ab > 0) {
				while (uS.IsUtteranceDel(s, 0)) *ts++ = *s++;
				while (*s == ' ' || *s == '\n' || *s == '\t' || 
					   *s == '[' || *s == ']' || sq ||
					   *s == '(' || *s == ')' || paran) {
					*ts++ = *s;
					if (*s == '[') sq = TRUE;
					else if (*s == ']') sq = FALSE;
					else if (*s == '(') paran = TRUE;
					else if (*s == ')') paran = FALSE;
					s++;
				}
				s1 = s;
				ab1 = ab;
				ab2 = 0;
				sq = FALSE;
				paran = FALSE;
				LastAB = TRUE;
				while (ab1 > 0) {
					if (*s1 == '[') sq = TRUE;
					else if (*s1 == ']') sq = FALSE;
					else if (*s1 == '(') paran = TRUE;
					else if (*s1 == ')') paran = FALSE;
					else if (*s1 == '<') {
						if (s1 != uttline && *(s1-1) == '+')
							;
						else if (!sq && !paran)
							ab2++;
					} else if (*s1 == '>') {
						if (!sq && !paran) {
							if (ab2 == 0) {
								ab1--;
								for (ts--; *ts == ' ' || *ts == '\t'; ts--) ;
								ts++;
								if (LastAB) {
									ab--;
									while (*s1 && *s1 != '[') *ts++ = *s1++;
								} else {
									*ts++ = '>';
									*ts++ = ' ';
									while (*s1 && *s1 != '[') s1++;
								}
								if (*s1 == '[') {
									while (*s1 && *s1 != ']') *ts++ = *s1++;
									if (!*s1) return(fixit_err(uttline,"Missing ']'"));
									else {
										*ts++ = ']';
										if (LastAB) s = s1 + 1;
									}
								} else 
								   return(fixit_err(uttline,"Missing code '[]' after '>'"));
							} else ab2--;
						}
					} else if (*s1 != ' ' && *s1 != '\t' && *s1 != '\n') 
						LastAB = FALSE;
					if (!*s1) {
						if (ab1 > 0) return(fixit_err(uttline,"Missing '>' character"));
						break;
					}
					s1++;
				}
				*ts++ = ' ';
				while (*s == ' ' || *s == '\t' || *s == '\n') s++;
				for (ab1=ab; ab1 > 0; ab1--)
					*ts++ = '<';
			} else {
				if (!*s) {
					if (ab > 0) return(fixit_err(uttline,"Missing '>' character"));
					break;
				}
				*ts++ = *s++;
			}
		} else {
			if (!*s) {
				if (ab > 0) return(fixit_err(uttline,"Missing '>' character"));
				break;
			}
			*ts++ = *s++;
		}
	}
	*ts = EOS;
	return(1);
}

static void fixit_mktier(char *st, int nw) {
	TIERS *t;

	if ((t=NEW(TIERS)) == NULL) fixit_overflow();
	t->NextTier = RootTier;
	RootTier = t;
	while (*st == ' ' || *st == '\t' || *st == '\n') st++;
	fixit_remblanks(st);
	t->codes = NULL;
	t->NumWords = nw;
	t->tier = (char *)malloc(strlen(st)+1);
	if (t->tier == NULL) fixit_overflow();
	strcpy(t->tier, st);
}

static char BreakTier() {
	char t, sq = FALSE, hb = FALSE,paran = FALSE, ft = TRUE;
	int beg, end, tbeg, NumTiers = 0;
	int NumWords = 0;

	end = strlen(temp_uttline) - 1;
	beg = end;
	while (1) {
		if (temp_uttline[beg] == ']') {
			if (temp_uttline[beg-1] != '.') sq = TRUE;
		} else if (temp_uttline[beg] == '[') sq = FALSE;
		else if (temp_uttline[beg] == HIDEN_C) hb = !hb;
		else if (temp_uttline[beg] == ')') paran = TRUE;
		else if (temp_uttline[beg] == '(') paran = FALSE;

		if (!uS.isskip(temp_uttline, beg, &dFnt, MBF) && !sq && !hb) {
			if (ft) { 
				NumWords++;
				ft = FALSE;
			}
		} else
			ft = TRUE;

		if ((uS.IsUtteranceDel(temp_uttline, beg) && !sq && !paran && !hb) || beg == 0) {
			for (tbeg=beg; uS.IsUtteranceDel(temp_uttline, tbeg) && tbeg > 0; tbeg--) ;
			if (temp_uttline[tbeg] != '-' || beg == 0) {
				if (beg != 0) {
					tbeg++;
					if (temp_uttline[tbeg] == HIDEN_C)
						 hb = !hb;
					while (uS.isskip(temp_uttline, tbeg, &dFnt, MBF) || sq || hb) {
						if (temp_uttline[tbeg] == '<' && !sq)
							break;
						if (temp_uttline[tbeg] == '[') {
							if (temp_uttline[tbeg+1] == '#')
								break;
							else
								sq = TRUE;
						} else if (temp_uttline[tbeg] == ']')
							sq = FALSE;
						tbeg++;
						if (temp_uttline[tbeg] == HIDEN_C) {
							 hb = !hb;
							 tbeg++;
						}
					}
				}
				t = temp_uttline[end];
				temp_uttline[end] = EOS;
				if (tbeg < end) {
/*
printf("tier=%s;\n", temp_uttline+tbeg);
*/
					fixit_mktier(temp_uttline+tbeg, NumWords);
					NumTiers++;
				}
				if (beg == 0) break;
				temp_uttline[end] = t;
				end = tbeg;
				NumWords = 0;
			} else if (tbeg > 0) beg = tbeg;
		} 
		beg--;
	}
	return(NumTiers > 1);
}

static char *isscope(char *st) {
	char t, *f, of = TRUE, nf = FALSE;

	f = st + 1;
	for (st++; *st != '>' && *st != EOS; st++) {
		if (isdigit(*st)) nf = TRUE;
		else if (*st != 'u' && *st != 'w' && *st != 's' && *st != ' ' && 
				 *st != '-' && *st != ',' && *st != '.') of = FALSE;
	}

	if (*st == EOS)
		return(NULL);

	if (!nf) {
		t = *st;
		*st = EOS;
		if (strcmp(f,"bef") && strcmp(f,"dur") && strcmp(f,"aft")) {
			*st = t;
			return(NULL);
		}
		*st = t;
	} else if (!of) return(NULL);
	return(st);
}

static char FoundScoping(char *st) {
	char t, sb, *TextEnd, *text, ScopeFound = FALSE;

	sb = FALSE;
	while (*st) {
		if (*st == '[') {
			sb = TRUE;
			st++;
		} else if (*st == ']') {
			sb = FALSE;
			st++;
		} else if (!sb && *st == '<') {
			if ((text=isscope(st)) == NULL) {
				st++;
				continue;
			}
			TextEnd = text;
			text++;
			do {
				for (TextEnd++; *TextEnd && *TextEnd != '<'; TextEnd++) ;
				if (*TextEnd == '<') {
					if (isscope(TextEnd) != NULL) break;
				} else break;
			} while (1) ;
			t = *TextEnd;
			*TextEnd = EOS;
			if (!ParsScope(st,text)) return(-1);
			*TextEnd = t;
			st = TextEnd;
			ScopeFound = TRUE;
		} else
			st++;
	}
	return(ScopeFound);
}

static char ParsErr() {
	TIERS *t;
	int cnt;
	char *tier, *beg, tc;

	beg = uttline;
	for (t=RootTier; t != NULL; t=t->NextTier) {
		cnt = 0;
		for (tier=t->tier; *tier; tier++) {
			if (*tier == '[' && *(tier+1) == '*' && *(tier+2) == ']') {
				cnt++;
				tier += 2;
			}
		}
		if (cnt > 0) {
			if (*beg == EOS)
				return(fixit_err(t->tier,"More data on main than on %err tier."));
			for (tier=beg+1; *tier; tier++) {
				if (*tier == '|' || *tier == ';') {
					if (--cnt == 0) break;
				}
			}
			if (*tier == EOS) cnt--;
			if (cnt > 0) return(fixit_err(t->tier,"More data on main than on %err tier."));
			tc = *tier;
			*tier = EOS;
			while (*beg== ' ' || *beg== '\t' || *beg=='|' || *beg==';') beg++;
			t->codes = MkCodes(t->codes,"",beg);
			*tier = tc;
			beg = tier;
		}
	}
	return(1);
}

static char ParsMor() {
	TIERS *t;
	int tindex;
	char *tier, *t2, *beg, tc, ft = TRUE, sq = FALSE;

	beg = uttline;
	t2 = beg;
	for (t=RootTier; t != NULL; t=t->NextTier) {
		for (tier=t->tier, tindex=0; tier[tindex]; tindex++) {
			if (tier[tindex] == '[') sq = TRUE;
			else if (tier[tindex] == ']') sq = FALSE;
			if (!uS.isskip(tier, tindex, &dFnt, MBF) && !sq) {
				if (ft) {
					ft = FALSE;
					while (*t2 == ' ' || *t2 == '\t' || *t2 == '\n') t2++;
					if (*t2 == EOS)
						return(fixit_err(t->tier,"More data on main than on %mor tier."));
					for (; *t2; t2++) {
						if (*t2 == ' ' || *t2 == '\t' || 
							*t2 == '\n' || *t2 == EOS) break;
					}
				}
			} else ft = TRUE;
		}
		if (t2 != beg) {
			tc = *t2;
			*t2 = EOS;
			while (*beg == ' ' || *beg == '\t' || *beg == '\n') beg++;
			t->codes = MkCodes(t->codes,"",beg);
			*t2 = tc;
			beg = t2;
		}
	}
	return(1);
}

static char ParsEng(char *beg, const char *err_mess) {
	TIERS *t;
	char *t2, tc, sq;

	t2 = beg;
	for (t=RootTier; t != NULL; t=t->NextTier) {
		sq = FALSE;
		while (*beg) {
			if (*t2 == '[') sq = TRUE;
			else if (*t2 == ']') sq = FALSE;
			else if ((uS.IsUtteranceDel(t2, 0) && !sq) || *t2 == EOS) {
				while (uS.IsUtteranceDel(t2, 0)) t2++;
				tc = *t2;
				*t2 = EOS;
				while (*beg == ' ' || *beg == '\t' || *beg == '\n') beg++;
				if (beg != t2) t->codes = MkCodes(t->codes,"",beg);
				*t2 = tc;
				beg = t2;
				break;
			}
			t2++;
		}
	}
	while (*beg == ' ' || *beg == '\t' || *beg == '\n') beg++;
	if (*beg != EOS) {
		return(fixit_err(uttline, err_mess));
	}
	return(1);
}

static void AddToHead() {
	HEADT *TH;

	if (HeadTier == NULL) {
		if ((HeadTier=NEW(HEADT)) == NULL) fixit_overflow();
		TH = HeadTier;
	} else {
		for (TH=HeadTier; TH->NextCode != NULL; TH=TH->NextCode) ;
		TH->NextCode = NEW(HEADT);
		TH = TH->NextCode;
		if (TH == NULL)
			fixit_overflow();
	}
	TH->type = (char *)malloc(strlen(utterance->speaker)+1);
	if (TH->type == NULL)
		fixit_overflow();
	strcpy(TH->type,utterance->speaker);
	TH->code = (char *)malloc(strlen(utterance->line)+1);
	if (TH->code == NULL)
		fixit_overflow();
	strcpy(TH->code,utterance->line);
	TH->NextCode = NULL;
}

void call() {
	char res, lTempline[SPEAKERLEN], PassThrough = 0;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
/* 2018-03-28
		if (lineno % 200 == 0)
			fprintf(stderr,"\r%ld ", lineno);
*/
		if (*utterance->speaker == '@') {
			AddToHead();
		} else if (*utterance->speaker == '*') {
			if (*OldSpeaker != EOS || HeadTier != NULL)
				fixit_pr_result();
			strcpy(OldSpeaker, utterance->speaker);
			if (!Duplicate_Scoping())
				continue;
			PassThrough = BreakTier();
		} else {
			fixit_remblanks(utterance->speaker);
			res = FoundScoping(utterance->line);
			if (res == -1) continue;
			if (!res) {
				strcpy(lTempline,utterance->speaker);
				uS.uppercasestr(lTempline, &dFnt, MBF);
				if (uS.partcmp(lTempline, "%ERR",FALSE,FALSE) && PassThrough) {
					if (!ParsErr()) continue;
				} else if (uS.partcmp(lTempline, "%MOR",FALSE,FALSE) && PassThrough) {
					if (!ParsMor()) continue; 
		/* ParsEng(uttline, "More data on %mor than on main tier."); */
				} else if (uS.partcmp(lTempline, "%ENG",FALSE,FALSE) && PassThrough) {
					if (!ParsEng(uttline, "More data on %eng than on main tier."))
						continue;
				} else if (RootTier == NULL) {
					fixit_err(utterance->line,"Code tier is found before any main tier is");
				} else {
					RootTier->codes = MkCodes(RootTier->codes,"",uttline);
				}
			}
		}
	}
// 2018-03-28	fprintf(stderr,"\n");
	if (*OldSpeaker != EOS || HeadTier != NULL)
		fixit_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
			check_mode = TRUE;
			no_arg_option(f);
			break;

	default:
		maingetflag(f-2,f1,i);
			break;
	}
}
