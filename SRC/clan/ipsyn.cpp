/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*

 RULENAME: N8
 if
 INCLUDE:	$MOD ^ $N ^ $V
 DIFFERENT_STEMS:		>2

 DIFFERENT_STEMS >2 means that at least 2 have to be different.

 point1-"one sheep is", point2-"other sheep is" fails for point2
 point1-"one sheep is", point2-"other hen is" succeeds for point2

*/
#define CHAT_MODE 1
#include "cu.h"
#include "check.h"
#ifndef UNX
#include "ced.h"
#else
#define RGBColor int
#include "c_curses.h"
#endif
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "ipsyn.h"

#if !defined(UNX)
#define _main ipsyn_main
#define call ipsyn_call
#define getflag ipsyn_getflag
#define init ipsyn_init
#define usage ipsyn_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define TYPESLEN 32

#define BEGSYM '\002'
#define ENDSYM '\003'
enum {
	MOR_TIER = 1,
	SPK_TIER,
	GRA_TIER
} ;

#define VARS_LIST struct vars_list
VARS_LIST {
	char *name;
	char *line;
	char isRepeat;
	VARS_LIST *next_var;
} ;

#define RULESUMS_LIST struct rsums_list
RULESUMS_LIST {
	char *name;
	char isSum;
	int  scoreSum;
	IEWORDS *pats;
	RULESUMS_LIST *next_rsum;
} ;

#define TEMPPAT struct TempPatElem
TEMPPAT {
	int	stackAndsI;
	PAT_ELEMS *origmac;
	PAT_ELEMS *flatmac;
	PAT_ELEMS *stackAnds[200];
	IEWORDS *duplicatesList;
} ;

#define TEMPMATCH struct TempMatchVars
TEMPMATCH {
	UTT_WORDS *firstUtt_word;
	UTT_WORDS *lastUtt_word;
} ;

#define TEMP_RULE struct temp_rule_elem
TEMP_RULE {
    RULES_LIST *rule;
	RULES_COND *cond;
} ;

extern char OverWriteFile;
extern char GExt[];

#define RULESSUMSMAX 50
#define RULESSUMSNAMEMAX 20

static int  fileID;
static char ipsyn_ftime, isAndSymFound, is100Limit;
static char isIPpostcode, isIPEpostcode;
static char Design[TYPESLEN+1], Activity[TYPESLEN+1], Group[TYPESLEN+1];
static IPSYN_SP *root_sp;
static RULES_LIST *root_rules;
static PATS_LIST *root_pats;
static VARS_LIST  *root_vars;
static RULESUMS_LIST  *root_rsum;
static TEMPPAT tempPat;
static UTTER *sTier;
static FILE *tfp;

int  IPS_UTTLIM;
char ipsyn_lang[256+2];

#ifndef KIDEVAL_LIB
void usage() {
	printf("Creates coding tier %%syn: with Syntactic codings derived from existing data\n");
	printf("Usage: ipsyn [cN d lF sS %s] filename(s)\n", mainflgs());
	printf("+cN: analyse N complete unique utterances. (default: %d utterances)\n", IPS_UTTLIM);
	puts("+d : do not show file and line number where points are found.");
	puts("+d1: outputs in SPREADSHEET format");
	puts("+lF: specify ipsyn rules file name F");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+o : use the original rule set with 100 utterances.");
	puts("-sS: specify [+ ip] or [+ ipe] to ignore those postcodes.");
	mainusage(TRUE);
}
#endif // KIDEVAL_LIB

static void freeUpAllFeats(MORFEATS *c) {
	MORFEATS *t, *p;

	while (c != NULL) {
		p = c;
		c = c->clitc;
		while (p != NULL) {
			t = p;
			p = p->compd;
			freeUpIxes(t->prefix, NUM_PREF);
			freeUpIxes(t->suffix, NUM_SUFF);
			freeUpIxes(t->fusion, NUM_FUSI);
			if (t->free_trans)
				free(t->trans);
			if (t->free_repls)
				free(t->repls);
			freeUpIxes(t->error, NUM_ERRS);
			free(t);
		}
	}
}

static void freeUtt_words(UTT_WORDS *p, char isAllFree) {
	UTT_WORDS *t;

	while (p != NULL) {
		t = p;
		p = p->next_word;
		if (isAllFree) {
			if (t->surf_word != NULL)
				free(t->surf_word);
			if (t->oMor_word != NULL)
				free(t->oMor_word);
			if (t->fMor_word != NULL)
				free(t->fMor_word);
			if (t->mor_word != NULL)
				free(t->mor_word);
			if (t->gra_word != NULL)
				free(t->gra_word);
			freeUpAllFeats(t->word_feats);
		}
		free(t);
	}
}

static void freeUtts(IPSYN_UTT *p) {
	IPSYN_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->next_utt;
		if (t->spkLine != NULL)
			free(t->spkLine);
		if (t->morLine != NULL)
			free(t->morLine);
		if (t->graLine != NULL)
			free(t->graLine);
		freeUtt_words(t->words, TRUE);
		free(t);
	}
}

static void freeResRules(RES_RULES *p) {
	RES_RULES *t;

	while (p != NULL) {
		t = p;
		p = p->next_result;
		freeUtt_words(t->point1, FALSE);
		freeUtt_words(t->point2, FALSE);
		free(t);
	}
}

static IPSYN_SP *freeIpsynSpeakers(IPSYN_SP *p) {
	IPSYN_SP *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->fname != NULL)
			free(t->fname);
		if (t->speaker != NULL)
			free(t->speaker);
		if (t->ID != NULL)
			free(t->ID);
		freeUtts(t->utts);
		freeResRules(t->resRules);
		free(t);
	}
	return(NULL);
}

static IPSYN_SP *freeSpkLines(IPSYN_SP *p) {
	IPSYN_SP *t;
	IPSYN_UTT *utt;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		for (utt=t->utts; utt != NULL; utt=utt->next_utt) {
			if (utt->spkLine != NULL) {
				free(utt->spkLine);
				utt->spkLine = NULL;
			}
		}
	}
	return(NULL);
}

static PATS_LIST *freeIpsynPats(PATS_LIST *p) {
	PATS_LIST *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_pat;
		if (t->pat_str != NULL)
			free(t->pat_str);
		if (t->pat_feat != NULL)
			t->pat_feat = freeMorWords(t->pat_feat);
		free(t);
	}
	return(NULL);
}

static VARS_LIST *freeVars(VARS_LIST *p) {
	VARS_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_var;
		if (t->name != NULL)
			free(t->name);
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static RULESUMS_LIST *freeRSums(RULESUMS_LIST *p) {
	RULESUMS_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_rsum;
		if (t->name != NULL)
			free(t->name);
		t->pats = freeIEWORDS(t->pats);
		free(t);
	}
	return(NULL);
}

static void freeIpsynElemPats(PAT_ELEMS *p, char isRemoveOR) {
	if (p == NULL)
		return;
	if (p->parenElem != NULL)
		freeIpsynElemPats(p->parenElem, isRemoveOR);
	if (p->andElem != NULL)
		freeIpsynElemPats(p->andElem, isRemoveOR);
	if (isRemoveOR && p->orElem != NULL)
		freeIpsynElemPats(p->orElem, isRemoveOR);
	free(p);
}

static void freeRulesConds(RULES_COND *p) {
	RULES_COND *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_cond;
		freeIEWORDS(t->deps);
		freeIpsynElemPats(t->exclude, TRUE);
		freeIpsynElemPats(t->include, TRUE);
		free(t);
	}
}

static IFLST *freeIfs(IFLST *p) {
	IFLST *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_if;
		if (t->name != NULL)
			free(t->name);
		free(t);
	}
	return(NULL);
}

static ADDLST *freeAdds(ADDLST *p) {
	ADDLST *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_add;
		freeIfs(t->ifs);
		free(t);
	}
	return(NULL);
}

static RULES_LIST *freeIpsynRules(RULES_LIST *p) {
	RULES_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_rule;
		if (t->name != NULL)
			free(t->name);
		if (t->info != NULL)
			free(t->info);
		freeAdds(t->adds);
		freeRulesConds(t->cond);
		free(t);
	}
	return(NULL);
}

void ipsyn_cleanSearchMem(void) {
	root_rules = freeIpsynRules(root_rules);
	root_pats = freeIpsynPats(root_pats);
	root_vars = freeVars(root_vars);
	root_rsum = freeRSums(root_rsum);
	tempPat.duplicatesList = freeIEWORDS(tempPat.duplicatesList);
	freeIpsynElemPats(tempPat.origmac, TRUE);
	freeIpsynElemPats(tempPat.flatmac, TRUE);
	tempPat.origmac = NULL;
	tempPat.flatmac = NULL;
}

static void ipsyn_overflow(void) {
	root_sp = freeIpsynSpeakers(root_sp);
	ipsyn_cleanSearchMem();
	if (tfp != NULL) {
		fclose(tfp);
		tfp = NULL;
	}
}

void removeLastUtt(IPSYN_SP *ts) {
	IPSYN_UTT *t, *ot;

	if (ts->UttNum > 0)
		ts->UttNum--;
	if (ts->utts != NULL) {
		t = ts->utts;
		if (t->next_utt == NULL) {
			if (t->spkLine != NULL)
				free(t->spkLine);
			if (t->morLine != NULL)
				free(t->morLine);
			if (t->graLine != NULL)
				free(t->graLine);
			freeUtt_words(t->words, TRUE);
			free(t);
			ts->utts = NULL;
			ts->cUtt = NULL;
		} else {
			for (ot=t; ot->next_utt != NULL; ot=ot->next_utt) ;
			for (; t->next_utt != ot; t=t->next_utt) ;
			t->next_utt = ot->next_utt;
			if (ot->spkLine != NULL)
				free(ot->spkLine);
			if (ot->morLine != NULL)
				free(ot->morLine);
			if (ot->graLine != NULL)
				free(ot->graLine);
			freeUtt_words(ot->words, TRUE);
			free(ot);
			ts->cUtt = t;
		}
	}
}

static UTT_WORDS *addUtt_words(UTT_WORDS *words, const char *fmw, const char *mw, const char *gw, const char *sw, char isClitic, char isLastStop) {
	int i, num, len, len2;
	UTT_WORDS *utt_word;

	num = 1;
	if (words == NULL) {
		utt_word = NEW(UTT_WORDS);
		if (utt_word == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		words = utt_word;
	} else {
		for (utt_word=words; utt_word->next_word != NULL; utt_word=utt_word->next_word) 
			num++;
		if ((utt_word->next_word=NEW(UTT_WORDS)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		num++;
		utt_word = utt_word->next_word;
	}
	utt_word->next_word = NULL;
	utt_word->isWordMatched = FALSE;
	utt_word->isWClitic = isClitic;
	utt_word->isFullWord = FALSE;
	utt_word->surf_word = NULL;
	utt_word->oMor_word = NULL;
	utt_word->fMor_word = NULL;
	utt_word->mor_word = NULL;
	utt_word->gra_word = NULL;
	utt_word->word_feats = NULL;
	utt_word->num = num;
	utt_word->isLastStop = isLastStop;
	if (sw != NULL) {
		if ((utt_word->surf_word=(char *)malloc(strlen(sw)+1)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(utt_word->surf_word, sw);
	}
	if (isLastStop == FALSE) {
		len = strlen(mw) + 1;
		if ((utt_word->oMor_word=(char *)malloc(len)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(utt_word->oMor_word, mw);
		len2 = strlen(fmw) + 1;
		if ((utt_word->fMor_word=(char *)malloc(len2)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(utt_word->fMor_word, fmw);
		for (i=0; fmw[i] != EOS && fmw[i] != '~' && fmw[i] != '$'; i++) ;
		if (fmw[i] != EOS) {
			if ((utt_word->mor_word=(char *)malloc(len2)) == NULL) {
				fprintf(stderr,"Error: out of memory\n");
				return(NULL);
			}
			strcpy(utt_word->mor_word, fmw);
		} else {
			if ((utt_word->mor_word=(char *)malloc(len)) == NULL) {
				fprintf(stderr,"Error: out of memory\n");
				return(NULL);
			}
			strcpy(utt_word->mor_word, mw);
		}
		if ((utt_word->gra_word=(char *)malloc(strlen(gw)+1)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(utt_word->gra_word, gw);
		for (i=0; mw[i] != EOS && mw[i] != '|'; i++) ;
		if (mw[i] == '|') {
			if ((utt_word->word_feats=NEW(MORFEATS)) == NULL) {
				fprintf(stderr,"Error: out of memory\n");
				return(NULL);
			}
			if (ParseWordMorElems(utt_word->mor_word, utt_word->word_feats) == FALSE) {
				fprintf(stderr,"Error: out of memory\n");
				return(NULL);
			}
		}
	}
	return(words);
}

static char createWordFeats(IPSYN_UTT *utt) {
	int  i, j;
	char isClitic, isGRADone;
	const char *sw, *fm;
	char *cb, *ce, b, e;
	char *gra_st, fmor_word[BUFSIZ], mor_word[BUFSIZ], gra_word[BUFSIZ], surf_word[BUFSIZ];
	UTT_WORDS *p;

	for (; utt != NULL; utt=utt->next_utt) {
		if (utt->morLine != NULL) {
			i = 0;
			j = 0;
			isGRADone = FALSE;
			while ((i=getNextDepTierPair(utt->morLine, mor_word, surf_word, NULL, i)) != 0) {
				uS.remblanks(mor_word);
				uS.remblanks(surf_word);
				if (mor_word[0] != '[' && mor_word[0] != EOS && surf_word[0] != EOS) {
					strcpy(fmor_word, mor_word);
					cb = strchr(mor_word, '^');
					if (cb != NULL)
						*cb = EOS;
					isClitic = '\0';
					cb = mor_word;
					b = '\0';
					while (*cb != EOS) {
						for (ce=cb; *ce != EOS && *ce != '$' && *ce != '~' && *ce != '*' && *ce != '@'; ce++) ;
						e = *ce;
						*ce = EOS;
						if (b == '~')
							isClitic = '~';
						else if (e == '$')
							isClitic = '$';
						else
							isClitic = '\0';
						if (isClitic == '\0') {
							sw = surf_word;
							fm = fmor_word;
						} else {
							sw = "";
							fm = "";
						}
						if (utt->graLine == NULL)
							gra_word[0] = EOS;
						else if (!isGRADone) {
							j = getword("%gra", utt->graLine, gra_word, NULL, j);
							if (j == 0)
								isGRADone = TRUE;
						}
						if (isGRADone && utt->graLine != NULL) {
							fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, utt->ln);
							fprintf(stderr,"** %%gra tier does not have one-to-one correspondence to %%mor tier.\n");
							removeMainTierWords(utt->morLine);
							removeAllSpaces(utt->morLine);
							fprintf(stderr,"%%mor: %s\n", utt->morLine);
							fprintf(stderr,"%%gra: %s\n", utt->graLine);
							return(FALSE);
						}
						gra_st = strrchr(gra_word, '|');
						if (gra_st != NULL)
							strcpy(gra_word, gra_st+1);
						p = addUtt_words(utt->words, fm, cb, gra_word, sw, isClitic, FALSE);
						if (p == NULL) {
							fprintf(stderr,"Error: out of memory\n");
							return(FALSE);
						}
						utt->words = p;
						*ce = e;
						if (e == '$' || e == '~') {
							b = e;
							cb = ce + 1;
						} else
							break;
					}
				}
			}
			if (!isGRADone && j > 0 && utt->graLine != NULL) {
				j = getword("%gra", utt->graLine, gra_word, NULL, j);
				if (j > 0) {
					fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, utt->ln);
					fprintf(stderr,"** %%gra tier does not have one-to-one correspondence to %%mor tier.\n");
					removeMainTierWords(utt->morLine);
					removeAllSpaces(utt->morLine);
					fprintf(stderr,"%%mor: %s\n", utt->morLine);
					fprintf(stderr,"%%gra: %s\n", utt->graLine);
					return(FALSE);
				}
			}
			p = addUtt_words(utt->words, NULL, NULL, NULL, NULL, '\0', TRUE); // adding DUMMY element
			if (p == NULL) {
				fprintf(stderr,"Error: out of memory\n");
				return(FALSE);
			}
			utt->words = p;
		}
	}
	return(TRUE);
}

/*
 char *type;		char typeID;
 char *prefix;	//	char mPrefix;  #
 char *pos;		//	char mPos;     |
 char *stem;	//	char mStem;    r
 char *suffix0;	//	char mSuffix0; -
 char *suffix1;	//	char mSuffix1; -
 char *suffix2;	//	char mSuffix2; -
 char *fusion0;	//	char mFusion0; &
 char *fusion1;	//	char mFusion1; &
 char *fusion2;	//	char mFusion2; &
 char *trans;   //	char mTrans;   =
 char *repls;   //	char mRepls;   @
 char *error;   //	char mError;   *
 */

static UTT_WORDS *findPrevOf(UTT_WORDS *utt_word, UTT_WORDS *firstUtt_word) {
	if (utt_word == firstUtt_word)
		return(NULL);
	while (utt_word != NULL && utt_word->next_word != firstUtt_word)
		utt_word = utt_word->next_word;
	return(utt_word);
}

static UTT_WORDS *findDummy(UTT_WORDS *utt_word) {
	while (utt_word->isLastStop == FALSE)
		utt_word = utt_word->next_word;
	return(utt_word);
}

static PATS_LIST *findNextPat_elem(PATS_LIST *pat_elem, PAT_ELEMS *tm) {
	if (pat_elem == NULL && tm->parenElem != NULL)
		pat_elem = findNextPat_elem(pat_elem, tm->parenElem);
	else if (pat_elem == NULL && tm->pat_elem != NULL) {
		if (tm->pat_elem->pat_str[0] != EOS && !tm->neg)
			pat_elem = tm->pat_elem;
	}
	if (pat_elem == NULL && tm->andElem != NULL)
		pat_elem = findNextPat_elem(pat_elem, tm->andElem);
	return(pat_elem);
}

static char isSkipClitic(char *s) {
	int cnt;

	if (strchr(s, '~') == NULL && strchr(s, '$') == NULL)
		return(TRUE);
	cnt = 0;
	while (*s != EOS) {
		if (*s == '|')
			cnt++;
		s++;
	}
	if (cnt >= 2)
		return(FALSE);
	else
		return(TRUE);
}

static char isPatMatch(PATS_LIST *pat_elem, UTT_WORDS *utt_word) {
	if (pat_elem->whatType == SPK_TIER) {
		if (uS.patmat(utt_word->surf_word, pat_elem->pat_str))
			return(TRUE);
	} else if (pat_elem->whatType == MOR_TIER) {
		if (pat_elem->isPClitic != '\0' && utt_word->isWClitic == '\0')
			return(FALSE);
		else {
			if (matchToMorElems(pat_elem->pat_feat, utt_word->word_feats, TRUE, isSkipClitic(pat_elem->pat_str), FALSE))
				return(TRUE);
		}
	} else if (pat_elem->whatType == GRA_TIER) {
		if (uS.patmat(utt_word->gra_word, pat_elem->pat_str))
			return(TRUE);
	}
	return(FALSE);
}

static UTT_WORDS *CMatch(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, UTT_WORDS *bWord, TEMPMATCH *tempMatch) {
	char isRepeat;
	PATS_LIST *pat_elem;

	isRepeat = FALSE;
	pat_elem = tm->pat_elem;
	if (tm->isRepeat) {
		if (pat_elem != findNextPat_elem(NULL, tm))
			isRepeat = TRUE;
	}
	if (pat_elem->pat_str[0] == BEGSYM && pat_elem->pat_str[1] == EOS) {
		if (utt_word == bWord)
			return(utt_word);
		else
			return(NULL);
	} else if (pat_elem->pat_str[0] == ENDSYM && pat_elem->pat_str[1] == EOS) {
		if (!utt_word->isLastStop && isUttDel(utt_word->fMor_word))
			return(utt_word->next_word);
		else
			return(NULL);
	} else if (strcmp(pat_elem->pat_str, "_") == 0) {
		tempMatch->firstUtt_word = utt_word;
		return(utt_word->next_word);
	} else {
		if (pat_elem->whatType != MOR_TIER) {
			while (utt_word != NULL && utt_word->isLastStop == FALSE && utt_word->isWClitic != '\0')
				utt_word = utt_word->next_word;
			if (utt_word == NULL || utt_word->isLastStop == TRUE)
				return(NULL);
		}
		if (wild == 1) {
			while (utt_word != NULL) {
				if (isPatMatch(pat_elem, utt_word)) {
					tempMatch->firstUtt_word = utt_word;
					utt_word = utt_word->next_word;
					while (isRepeat) {
						if (utt_word == NULL || utt_word->isLastStop == TRUE)
							break;
						if (!isPatMatch(pat_elem, utt_word))
							break;
						utt_word = utt_word->next_word;
					}
					return(utt_word);
				}
				utt_word = utt_word->next_word;
				if (pat_elem->whatType != MOR_TIER) {
					while (utt_word != NULL && utt_word->isLastStop == FALSE && utt_word->isWClitic != '\0')
						utt_word = utt_word->next_word;
				}
				if (utt_word == NULL || utt_word->isLastStop == TRUE)
					return(NULL);
			}
		} else {
			if (isPatMatch(pat_elem, utt_word)) {
				tempMatch->firstUtt_word = utt_word;
				utt_word = utt_word->next_word;
				while (isRepeat) {
					if (utt_word == NULL || utt_word->isLastStop == TRUE)
						break;
					if (!isPatMatch(pat_elem, utt_word))
						break;
					utt_word = utt_word->next_word;
				}
				return(utt_word);
			}
		}
	}
	return(NULL);
}

static UTT_WORDS *match(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, char *isStillMatched, char isTrueAnd, UTT_WORDS *bWord, TEMPMATCH *tempMatch, char isFromExclude) {
	char isNegFound;
	UTT_WORDS *last, *tmp, *negSf, *negSft, *negSl, *tUtt_word;

	if (utt_word->isLastStop == TRUE) {
		if (isFromExclude && tm != NULL && utt_word->surf_word != NULL) {
			if (isUttDel(utt_word->surf_word) && tm->pat_elem != NULL) {
				if (tm->pat_elem->pat_str[0] == ENDSYM && tm->pat_elem->pat_str[1] == EOS &&
					tm->parenElem == NULL && tm->refParenElem == NULL && tm->andElem == NULL &&
					tm->refAndElem == NULL && tm->orElem == NULL) {
					tm->matchedWord = tempMatch->firstUtt_word;
					return(utt_word);
				}
			}
		}
		return(NULL);
	}
	isNegFound = FALSE;
	negSf = NULL;
	negSl = NULL;
	negSft = NULL;
	if (tm->pat_elem != NULL) {
		if (utt_word->isWordMatched)
			return(NULL);
		else if (tm->pat_elem->pat_str[0] == EOS) {
		} else if ((last=CMatch(utt_word,tm,wild,bWord,tempMatch)) != NULL) {
			if (tm->neg) {
				if (isTrueAnd)
					return(NULL);
				negSf = findPrevOf(utt_word, tempMatch->firstUtt_word);
				if (negSf != NULL) {
					negSft = negSf->next_word;
					negSf->next_word = findDummy(utt_word);
				}
				negSl = last;
				isNegFound = TRUE;
			} else {
				tm->matchedWord = tempMatch->firstUtt_word;
				tmp = last;
				if (isTrueAnd) {
					if (tempMatch->lastUtt_word->num < tmp->num)
						tempMatch->lastUtt_word = tmp;
				} else
					utt_word = tmp;
			}
		} else {
			if (!tm->neg)
				return(NULL);
		}
	} else if (tm->parenElem != NULL) {
		if ((tmp=match(utt_word,tm->parenElem,wild,isStillMatched,isTrueAnd,bWord,tempMatch,isFromExclude)) != NULL) {
			utt_word = tmp;
			if (tm->neg) {
				if (isTrueAnd)
					return(NULL);
				negSf = findPrevOf(utt_word, tempMatch->firstUtt_word);
				if (negSf != NULL) {
					negSft = negSf->next_word;
					negSf->next_word = findDummy(utt_word);
				}
				negSl = utt_word;
				isNegFound = TRUE;
			}
		} else {
			if (!tm->neg)
				return(NULL);
		}
	}
	if (tm->andElem != NULL) {
		if (negSf != NULL || negSl != NULL) {
			if (negSf != NULL) {
				tUtt_word = utt_word;
				utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,bWord,tempMatch,isFromExclude);
				if (wild && utt_word == NULL && tUtt_word->num <= negSf->num) {
					utt_word = tUtt_word;
					tmp = utt_word;
					while (utt_word != NULL && utt_word->num <= negSf->num) {
						utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,bWord,tempMatch,isFromExclude);
						if (*isStillMatched == FALSE)
							break;
						if (utt_word != NULL) {
							break;
						} else {
							tmp = tmp->next_word;
							if (tmp == NULL || tmp->num > negSf->num)
								break;
							utt_word = tmp;
						}
					}
				}
				negSf->next_word = negSft;
				negSf = NULL;
			}
			if (utt_word == NULL && *isStillMatched && negSl != NULL) {
				utt_word = negSl;
				utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,bWord,tempMatch,isFromExclude);
				if (tm->neg && isNegFound && utt_word != NULL)
					*isStillMatched = FALSE;
			}
			negSl = NULL;
		} else {
			utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,bWord,tempMatch,isFromExclude);
			if (tm->neg && isNegFound && utt_word != NULL)
				*isStillMatched = FALSE;
		}
	} else {
		if (negSf != NULL) {
			negSf->next_word = negSft;
			negSf = NULL;
		}
		if (negSl != NULL) {
			negSl = NULL;
		}
		if (isNegFound) {
			*isStillMatched = FALSE;
			return(NULL);
		}
	}
	return(utt_word);
}

static UTT_WORDS *negMatch(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, char isTrueAnd, UTT_WORDS *bWord, TEMPMATCH *tempMatch) {
	UTT_WORDS *last, *tmp;

	if (utt_word->isLastStop == TRUE)
		return(NULL);
	if (tm->pat_elem != NULL) {
		if (utt_word->isWordMatched)
			return(NULL);
		else if (tm->pat_elem->pat_str[0] == EOS) {
		} else if ((last=CMatch(utt_word,tm,wild,bWord,tempMatch)) != NULL) {
			tm->matchedWord = tempMatch->firstUtt_word;
			tmp = last;
			if (isTrueAnd) {
				if (tempMatch->lastUtt_word->num < tmp->num)
					tempMatch->lastUtt_word = tmp;
			} else
				utt_word = tmp;
		} else {
			return(NULL);
		}
	} else if (tm->parenElem != NULL) {
		if ((tmp=negMatch(utt_word,tm->parenElem,wild,isTrueAnd,bWord,tempMatch)) != NULL) {
			utt_word = tmp;
		} else {
			return(NULL);
		}
	}
	if (tm->andElem != NULL) {
		utt_word = negMatch(utt_word,tm->andElem,tm->wild,isTrueAnd,bWord,tempMatch);
	}
	return(utt_word);
}

static UTT_WORDS *copyUtt_word(UTT_WORDS *root, UTT_WORDS *utt_word, char isFullWord) {
	UTT_WORDS *t;

	if (root == NULL) {
		t = NEW(UTT_WORDS);
		if (t == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		root = t;
	} else {
		for (t=root; t->next_word != NULL; t=t->next_word) ;
		if ((t->next_word=NEW(UTT_WORDS)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		t = t->next_word;
	}
	t->num = utt_word->num;
	t->isLastStop = utt_word->isLastStop;
	t->isWordMatched = utt_word->isWordMatched;
	t->isWClitic = utt_word->isWClitic;
	t->isFullWord = isFullWord;
	t->surf_word = utt_word->surf_word;
	t->oMor_word = utt_word->oMor_word;
	t->fMor_word = utt_word->fMor_word;
	t->mor_word = utt_word->mor_word;
	t->gra_word = utt_word->gra_word;
	t->word_feats = utt_word->word_feats;
	t->next_word = NULL;
	return(root);
}

static RES_RULES *addResRuleToSp(IPSYN_SP *sp, RULES_LIST *rule, UTT_WORDS *utt_word, int point, char isFullWord) {
	RES_RULES *t;
	UTT_WORDS *p;

	if (sp->resRules == NULL) {
		t = NEW(RES_RULES);
		if (t == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		sp->resRules = t;
	} else {
		for (t=sp->resRules; 1; t=t->next_result) {
			if (t->rule == rule) {
				if (point == 1 && t->pointToSet == 1) {
					t->cntStem1++;
					p = copyUtt_word(t->point1, utt_word, isFullWord);
					if (p == NULL) {
						return(NULL);
					}
					t->point1 = p;
				} else if (point == 2 && t->pointToSet == 2) {
					t->cntStem2++;
					p = copyUtt_word(t->point2, utt_word, isFullWord);
					if (p == NULL) {
						return(NULL);
					}
					t->point2 = p;
				}
				return(t);
			}
			if (t->next_result == NULL)
				break;
		}
		if ((t->next_result=NEW(RES_RULES)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		t = t->next_result;
	}
	t->next_result = NULL;
	t->rule = rule;
	t->pointToSet = 0;
	t->pointsEarned = 0;
	t->cntStem1 = 0L;
	t->point1 = NULL;
	t->cntStem2 = 0L;
	t->point2 = NULL;
	if (point == 1) {
		p = copyUtt_word(t->point1, utt_word, isFullWord);
		if (p == NULL) {
			return(NULL);
		}
		t->pointToSet = 1;
		t->cntStem1 = 1L;
		t->point1 = p;
	}
/*
	else if (point == 2) {
		p = copyUtt_word(t->point1, utt_word, isFullWord);
			if (p == NULL) {
			return(NULL);
		}
		t->pointToSet = 2;
		t->cntStem2 = 1L;
		t->point2 = p;
	}
*/
	return(t);
}

static void getStem(UTT_WORDS *utt_word, char *stem) {
	MORFEATS *word_clitc, *word_feat;

	stem[0] = EOS;
	if (utt_word->word_feats == NULL)
		strcat(stem, utt_word->oMor_word);
	else {
		for (word_clitc=utt_word->word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID == '$') {
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+')
						strcat(stem, "+");
					if (word_feat->stem != NULL)
						strcat(stem, word_feat->stem);
				}
				strcat(stem, "$");
			}
		}
		for (word_clitc=utt_word->word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID != '$'){
				if (word_clitc->typeID == '~')
					strcat(stem, "~");
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+')
						strcat(stem, "+");
					if (word_feat->stem != NULL)
						strcat(stem, word_feat->stem);
				}
			}
		}
	}
}

static void resetElems(PAT_ELEMS *cur) {
	cur->matchedWord = NULL;
	if (cur->parenElem != NULL) {
		resetElems(cur->parenElem);
	}
	if (cur->andElem != NULL) {
		resetElems(cur->andElem);
	}
}

static RES_RULES *setmat(RES_RULES *res, PAT_ELEMS *cur, IPSYN_SP *sp, RULES_LIST *rule, int point, char *isErr) {
	char isDisplayFullWord;
	RES_RULES *p;

	if (cur->parenElem != NULL) {
		if (!cur->neg) {
			res = setmat(res, cur->parenElem, sp, rule, point, isErr);
		}
	} else if (cur->pat_elem != NULL) {
		if ((cur->pat_elem->pat_str[0] == BEGSYM || cur->pat_elem->pat_str[0] == ENDSYM) && cur->pat_elem->pat_str[1] == EOS) {
		} else if (cur->pat_elem->pat_str[0] != EOS && !cur->neg && cur->matchedWord != NULL) {
			if (cur->pat_elem->whatType == SPK_TIER) {
				isDisplayFullWord = TRUE;
			} else if (cur->pat_elem->whatType == MOR_TIER) {
				if (isSkipClitic(cur->pat_elem->pat_str))
					isDisplayFullWord = FALSE;
				else
					isDisplayFullWord = TRUE;
			} else
				isDisplayFullWord = FALSE;
			p = addResRuleToSp(sp, rule, cur->matchedWord, point, isDisplayFullWord);
			if (p == NULL) {
				*isErr = TRUE;
				return(NULL);
			}
			res = p;
		}
	}
	if (cur->andElem != NULL) {
		res = setmat(res, cur->andElem, sp, rule, point, isErr);
	}
	return(res);
}

static char addDummyToSpResRules(RES_RULES *resRules, int point, long ln, char *surf_word) {
	UTT_WORDS *p;

	if (surf_word != NULL && isUttDel(surf_word) == FALSE)
		surf_word = NULL;
	if (point == 1) {
		resRules->ln1 = ln;
		p = addUtt_words(resRules->point1, NULL, NULL, NULL, surf_word, '\0', TRUE); // adding DUMMY element
		if (p == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		resRules->point1 = p;
	} else if (point == 2) {
		resRules->ln2 = ln;
		p = addUtt_words(resRules->point2, NULL, NULL, NULL, surf_word, '\0', TRUE); // adding DUMMY element
		if (p == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		resRules->point2 = p;
	}
	return(TRUE);
}

static void resetSpResRule(RES_RULES *resRule, int point) {
	if (point == 1 && resRule->point1 == NULL) {
		resRule->pointToSet = 1;
	} else if (point == 2 && resRule->point2 == NULL) {
		resRule->pointToSet = 2;
	}
}

static void setSpResRule(RES_RULES *resRule, int point) {
	if (point == 1 && resRule->point1 != NULL) {
		resRule->pointToSet = 2;
	} else if (point == 2 && resRule->point2 != NULL) {
		resRule->pointToSet = 3;
	}
}

static void resetUtt_wordMatched(IPSYN_UTT *utt) {
	UTT_WORDS *utt_word;

	for (; utt != NULL; utt=utt->next_utt) {
		for (utt_word=utt->words; utt_word != NULL && utt_word->isLastStop == FALSE; utt_word=utt_word->next_word)
			utt_word->isWordMatched = FALSE;
	}
}

static void setUtt_wordMatched(PAT_ELEMS *cur) {
	if (cur->parenElem != NULL) {
		if (!cur->neg)
			setUtt_wordMatched(cur->parenElem);
	} else if (cur->pat_elem != NULL) {
		if (cur->pat_elem->pat_str[0] != EOS && !cur->neg && cur->matchedWord != NULL) {
			cur->matchedWord->isWordMatched = TRUE;
		}
	}
	if (cur->andElem != NULL) {
		setUtt_wordMatched(cur->andElem);
	}
}

static char EarnedPoint(RES_RULES *resRule, RULES_COND *rule_cond) {
	int  j;
	unsigned long cnt, cntMatched;
	char word1[BUFSIZ], word2[BUFSIZ];
	UTT_WORDS *point1, *point2;

	if (resRule->point1 == NULL || resRule->point2 == NULL)
		return(TRUE);
	if (rule_cond->isDiffSpec == 1) {
		cnt = rule_cond->diffCnt;
		if (cnt == 0L)
			return(TRUE);
		j = 0;
		point1 = resRule->point1;
		point2 = resRule->point2;
		while (point1 != NULL && point1->isLastStop == FALSE && point2 != NULL && point2->isLastStop == FALSE) {
			if (j >= 32)
				break;
			getStem(point1, word1);
			getStem(point2, word2);
			if ((cnt & 1L) &&  uS.mStricmp(word1, word2) == 0)
				return(FALSE);
			cnt = cnt >> 1;
			j++;
			point1 = point1->next_word;
			point2 = point2->next_word;
		}
		return(TRUE);
	} else if (rule_cond->isDiffSpec == 2) {
		if (resRule->cntStem1 > resRule->cntStem2)
			cnt = resRule->cntStem2;
		else
			cnt = resRule->cntStem1;
		cntMatched = 0L;
		for (point1=resRule->point1; point1 != NULL && point1->isLastStop == FALSE; point1=point1->next_word) {
			getStem(point1, word1);
			for (point2=resRule->point2; point2 != NULL && point2->isLastStop == FALSE; point2=point2->next_word) {
				getStem(point2, word2);
				if (uS.mStricmp(word1, word2) == 0)
					cntMatched++;
			}
		}
		if (cntMatched <= cnt) {
			if (cnt - cntMatched >= rule_cond->diffCnt)
				return(TRUE);
		} else {
			if (cntMatched - cnt >= rule_cond->diffCnt)
				return(TRUE);
		}
		return(FALSE);
	}
	return(TRUE);
}
/*
static char FoundRule(RES_RULES *resRules, IEWORDS *deps) {
	int  b, e, t, cnt, cntMatched;
	char *str;
	IEWORDS *dep;
	RES_RULES *resRule;

	if (deps == NULL)
		return(TRUE);
	else if (resRules == NULL)
		return(FALSE);
	else {
		for (dep=deps; dep != NULL; dep=dep->nextword) {
			str = dep->word;
			b = 0;
			cnt = 0;
			cntMatched = 0;
			while (str[b] != EOS) {
				for (e=b; str[e] != EOS && str[e] != ','; e++) ;
				if (str[e] == ',' || str[e] == EOS) {
					t = str[e];
					str[e] = EOS;
					if (str[b] != EOS) {
						cnt++;
						for (resRule=resRules; resRule != NULL; resRule=resRule->next_result) {
							if (resRule->rule != NULL) {
								if (uS.mStricmp(str+b,resRule->rule->name) == 0 && (resRule->point1 != NULL || resRule->point2 != NULL))
									cntMatched++;
							}
						}
					}
					if (t == EOS)
						b = e;
					else {
						str[e] = t;
						b = e + 1;
					}
				} else
					b++;
			}
			if (cnt == cntMatched)
				return(TRUE);
		}
	}
	return(FALSE);
}
*/
static char isExcludeResult(RULES_LIST *rule, RULES_COND *rule_cond, RES_RULES *resRule, int point) {
	char wild, isStillMatched;
	PAT_ELEMS *flatp;
	UTT_WORDS *utt_word, *tUtt_word, *bWord;
	TEMPMATCH tempMatch;

	if (rule_cond->exclude == NULL)
		return(FALSE);
	if (rule->isTrueAnd)
		wild = 1;
	else
		wild = 0;
	for (flatp=rule_cond->exclude; flatp != NULL; flatp=flatp->orElem) {
		if (point == 1) {
			utt_word = resRule->point1;
			if (resRule->point1->num == 1)
				bWord = utt_word;
			else
				bWord = NULL;
		} else if (point == 2) {
			utt_word = resRule->point2;
			if (resRule->point2->num == 1)
				bWord = utt_word;
			else
				bWord = NULL;
		} else {
			return(FALSE);
		}
		while (utt_word != NULL && utt_word->isLastStop == FALSE) {
			resetElems(flatp);
			tUtt_word = utt_word;
			isStillMatched = TRUE;
			if (flatp->isAllNeg) {
				utt_word = negMatch(utt_word,flatp,wild,rule->isTrueAnd,bWord,&tempMatch);
				if (utt_word != NULL) {
					break;
				} else {
					utt_word = tUtt_word;
					utt_word = utt_word->next_word;
				}
			} else {
				utt_word = match(utt_word,flatp,wild,&isStillMatched,rule->isTrueAnd,bWord,&tempMatch,TRUE);
				if (utt_word != NULL && isStillMatched) {
					return(TRUE);
				} else {
					if (rule->isTrueAnd)
						break;
					if (utt_word == NULL) {
						utt_word = tUtt_word;
						utt_word = utt_word->next_word;
					}
				}
			}
		}
	}
	return(FALSE);
}

static char isWrongPoint(RES_RULES *resRule, int point) {
	if (point == 1 && resRule->pointToSet == 1) {
		return(FALSE);
	} else if (point == 2 && resRule->pointToSet == 2) {
		return(FALSE);
	}
	return(TRUE);
}

static int getAdditionalScore(RULES_LIST *rule, RES_RULES *resRules, char *isErr, char *fromNames) {
	int total;
	IFLST *ifs;
	ADDLST *adds;
	RULES_LIST *tRule;
	RES_RULES *resRule;

	total = 0;
	fromNames[0] = EOS;
	for (adds=rule->adds; adds != NULL; adds=adds->next_add) {
		for (ifs=adds->ifs; ifs != NULL; ifs=ifs->next_if) {
			for (tRule=root_rules; tRule != NULL; tRule=tRule->next_rule) {
				if (uS.mStricmp(tRule->name, ifs->name) == 0)
					break;
			}
			if (tRule == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", FileName1, adds->ln);
				fprintf(stderr, "Rule \"%s\" was not declared before referenced in ADD element of rule \"%s\"\n", ifs->name, rule->name);
				*isErr = TRUE;
				return(0);
			}
			for (resRule=resRules; resRule != NULL; resRule=resRule->next_result) {
				if (resRule->rule != NULL) {
					if (uS.mStricmp(ifs->name,resRule->rule->name) == 0 && resRule->pointsEarned == ifs->score) {
						if (fromNames[0] != EOS)
							strcat(fromNames, " ");
						strcat(fromNames, resRule->rule->name);
						total += adds->score;
						break;
					}
				}
			}
		}
	}
	if (fromNames[0] == EOS)
		strcpy(fromNames, "unknown");
	return(total);
}
/*
static void cleanSpID(IPSYN_SP *sp) {
	int i, cnt;
	cnt = 0;

	for (i=0; sp->ID[i] != EOS; i++) {
		if (sp->ID[i] == '|') {
			cnt++;
			if (cnt < 10)
				sp->ID[i] = '\t';
			else
				sp->ID[i] = EOS;
		}
	}
}
*/

static char createIPSynCoding(char isOutput) {
	int  point, addScore, totalSum;
	char wild, isStillMatched, isMatchFound, *sFName, isErr, isOnlyOneWord;
	RULES_COND *rule_cond;
	RULES_LIST *rule;
	IPSYN_UTT *utt;
	IPSYN_SP *sp;
	PAT_ELEMS *flatp;
	RES_RULES *resRule;
	UTT_WORDS *utt_word, *tUtt_word, *orgUtt_word;
	TEMPMATCH tempMatch;
	RULESUMS_LIST *rsum;
	IEWORDS *pat;

	if (onlydata == 2) {
		isOutput = FALSE;
		excelHeader(fpout, newfname, 95);
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, "File");
		excelCommasStrCell(fpout, "Language,Corpus,Code,Age(Month),Sex,Group,Race,SES,Role,Education,Custom_field");
		excelCommasStrCell(fpout, "Design,Activity,Group");
		for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
			excelStrCell(fpout, rsum->name);
		}
		excelStrCell(fpout, "Total");
		excelRow(fpout, ExcelRowEnd);
	}
	for (sp=root_sp; sp != NULL; sp=sp->next_sp) {
		if (sp->isSpeakerFound == FALSE)
			continue;
		if (sp->utts == NULL) {
			if (isOutput || onlydata == 2)
				fprintf(stderr,"\nWARNING: No %%mor: tier found for speaker: %s\n\n", sp->speaker);
			if (onlydata == 2) {
				sFName = strrchr(sp->fname, PATHDELIMCHR);
				if (sFName != NULL)
					sFName = sFName + 1;
				else
					sFName = sp->fname;
				excelRow(fpout, ExcelRowStart);
				excelStrCell(fpout, sFName);
				if (sp->ID) {
					excelOutputID(fpout, sp->ID);
				} else {
					excelCommasStrCell(fpout, ".,.");
					excelStrCell(fpout, sp->speaker);
					excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.,.,.,.");
				}
				excelStrCell(fpout, Design);
				excelStrCell(fpout, Activity);
				excelStrCell(fpout, Group);
				for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
					excelStrCell(fpout, "NA");
				}
				excelRow(fpout, ExcelRowEnd);
			}
			continue;
		}
		if (!createWordFeats(sp->utts))
			return(FALSE);
		for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
			resetUtt_wordMatched(sp->utts);
//if (!strcmp(rule->name, "S8"))
//int bbbb = 0;
			for (point=1; point < 3; point++) {
				isMatchFound = FALSE;
				for (rule_cond=rule->cond; rule_cond != NULL; rule_cond=rule_cond->next_cond) {
					if (rule_cond->include != NULL && /* FoundRule(sp->resRules,rule_cond->deps) && */
						(rule_cond->point==0 || rule_cond->point==point)) {
						for (utt=sp->utts; utt != NULL; utt=utt->next_utt) {
							if (rule->isTrueAnd)
								wild = 1;
							else
								wild = 0;
							orgUtt_word = utt->words;
							utt_word = utt->words;
							for (flatp=rule_cond->include; flatp != NULL; flatp=flatp->orElem) {
								while (utt_word != NULL && utt_word->isLastStop == FALSE) {
									resetElems(flatp);
									tUtt_word = utt_word;
									tempMatch.firstUtt_word = NULL;
									tempMatch.lastUtt_word = utt_word;
									isStillMatched = TRUE;
									if (flatp->isAllNeg) {
										utt_word = negMatch(utt_word,flatp,wild,rule->isTrueAnd,orgUtt_word,&tempMatch);
										if (utt_word != NULL) {
											break;
										} else {
											utt_word = tUtt_word;
											utt_word = utt_word->next_word;
										}
									} else {
										utt_word = match(utt_word,flatp,wild,&isStillMatched,rule->isTrueAnd,orgUtt_word,&tempMatch,FALSE);
										if (utt_word != NULL && isStillMatched) {
											isErr = FALSE;
											resRule = setmat(NULL, flatp, sp, rule, point, &isErr);
											if (isErr) {
												return(FALSE);
											}
											if (resRule != NULL) {
												if (!addDummyToSpResRules(resRule, point, utt->ln, utt_word->surf_word))
													return(FALSE);
												if (isWrongPoint(resRule, point)) {
													if (point == 1) {
														freeUtt_words(resRule->point1, FALSE);
														resRule->point1 = NULL;
													} else if (point == 2) {
														freeUtt_words(resRule->point2, FALSE);
														resRule->point2 = NULL;
													}
												} else if (!isExcludeResult(rule, rule_cond, resRule, point)) {
													if (point == 2) {
														if (rule_cond->isDiffSpec == 0) {
															isMatchFound = TRUE;
															setUtt_wordMatched(flatp);
															setSpResRule(resRule, point);
															break;
														} else if (EarnedPoint(resRule, rule_cond)) {
															isMatchFound = TRUE;
															setUtt_wordMatched(flatp);
															setSpResRule(resRule, point);
															break;
														} else {
															freeUtt_words(resRule->point2, FALSE);
															resRule->point2 = NULL;
															resetSpResRule(resRule, point);
														}
													} else {
														isMatchFound = TRUE;
														setUtt_wordMatched(flatp);
														setSpResRule(resRule, point);
														break;
													}
												} else {
													if (point == 1) {
														freeUtt_words(resRule->point1, FALSE);
														resRule->point1 = NULL;
													} else if (point == 2) {
														freeUtt_words(resRule->point2, FALSE);
														resRule->point2 = NULL;
													}
													resetSpResRule(resRule, point);
												}
											}
											if (rule->isTrueAnd)
												utt_word = tempMatch.lastUtt_word;
											else if (utt_word == tUtt_word) {
												if (utt_word->isLastStop == FALSE)
													utt_word = utt_word->next_word;
											}
										} else {
											if (rule->isTrueAnd)
												break;
											if (isMatchFound)
												break;
											if (utt_word == NULL) {
												utt_word = tUtt_word;
												utt_word = utt_word->next_word;
											}
										}
									}
								}
								if (isMatchFound)
									break;
								utt_word = orgUtt_word;
							}
							if (isMatchFound)
								break;
						}
					}
					if (isMatchFound)
						break;
				}
			}
		}
		if (isOutput) {
			if (sp != root_sp)
				fprintf(fpout, "\n");
			fprintf(fpout, "\n*** Speaker: %s\n", sp->speaker);
			if (!combinput) {
				sFName = strrchr(oldfname, PATHDELIMCHR);
				if (sFName != NULL)
					sFName = sFName + 1;
				else
					sFName = oldfname;
				fprintf(fpout,"%s", sFName);
			}
			if (sp->ID) {
//				if (0)
//					cleanSpID(sp);
				fprintf(fpout,"\t%s\n", sp->ID);
			} else {
//				fprintf(fpout,"\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.\n", sp->speaker);
				fputc('\n', fpout);
			}
			fputc('\n', fpout);
		}
		for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
			for (resRule=sp->resRules; resRule != NULL; resRule=resRule->next_result) {
				if (resRule->rule == rule && (resRule->point1 != NULL || resRule->point2 != NULL)) {
					if (resRule->point1 != NULL) {
						resRule->pointsEarned++;
					}
					if (resRule->point2 != NULL) {
						resRule->pointsEarned++;
					}
					break;
				}
			}
		}
		totalSum = 0;
		for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
			rsum->scoreSum = 0;
		}
		for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
			for (resRule=sp->resRules; resRule != NULL; resRule=resRule->next_result) {
				if (resRule->rule == rule && (resRule->point1 != NULL || resRule->point2 != NULL)) {
					if (isOutput) {
						if (rule->info != NULL)
							fprintf(fpout, "Rule: %s - %s\n", rule->name, rule->info);
						else
							fprintf(fpout, "Rule: %s\n", rule->name);
						if (resRule->point1 != NULL) {
							isOnlyOneWord = FALSE;
							utt_word = resRule->point1;
							if (utt_word->isLastStop || utt_word->next_word == NULL || utt_word->next_word->isLastStop)
								isOnlyOneWord = TRUE;
							if (onlydata == 0 && !combinput)
								fprintf(fpout,"        File \"%s\": line %ld.\n", oldfname, resRule->ln1);
							fprintf(fpout, "    Point1:");
							for (utt_word=resRule->point1; utt_word != NULL && utt_word->isLastStop == FALSE; utt_word=utt_word->next_word) {
								if (utt_word->isFullWord)
									fprintf(fpout, " %s", utt_word->fMor_word);
								else if (utt_word->isWClitic != '\0')
									fprintf(fpout, " %c%s", utt_word->isWClitic, utt_word->oMor_word);
								else if (isOnlyOneWord && strchr(utt_word->fMor_word, '~') != NULL)
									fprintf(fpout, " %s", utt_word->fMor_word);
								else
									fprintf(fpout, " %s", utt_word->oMor_word);
							}
							fprintf(fpout, "\n");
						}
						if (resRule->point2 != NULL) {
							isOnlyOneWord = FALSE;
							utt_word = resRule->point2;
							if (utt_word->isLastStop || utt_word->next_word == NULL || utt_word->next_word->isLastStop)
								isOnlyOneWord = TRUE;
							if (onlydata == 0 && !combinput)
								fprintf(fpout,"        File \"%s\": line %ld.\n", oldfname, resRule->ln2);
							fprintf(fpout, "    Point2:");
							for (utt_word=resRule->point2; utt_word != NULL && utt_word->isLastStop == FALSE; utt_word=utt_word->next_word) {
								if (utt_word->isFullWord)
									fprintf(fpout, " %s", utt_word->fMor_word);
								else if (utt_word->isWClitic != '\0')
									fprintf(fpout, " %c%s", utt_word->isWClitic, utt_word->oMor_word);
								else if (isOnlyOneWord && strchr(utt_word->fMor_word, '~') != NULL)
									fprintf(fpout, " %s", utt_word->fMor_word);
								else
									fprintf(fpout, " %s", utt_word->oMor_word);
							}
							fprintf(fpout, "\n");
						}
					}
					if (rule->adds == NULL)
						addScore = 0;
					else {
						isErr = FALSE;
						addScore = getAdditionalScore(rule, sp->resRules, &isErr, spareTier3);
						if (isErr) {
							return(FALSE);
						}
					}
					if (isOutput && addScore > 0) {
						fprintf(fpout, "    ADD: score: %d from %s\n", addScore, spareTier3);
					}
					addScore = addScore + resRule->pointsEarned;

					if (addScore > 2)
						addScore = 2;

					if (isOutput)
						fprintf(fpout, "  Score: %d\n\n", addScore);

					totalSum += addScore;
					if (isOutput || onlydata == 2) {
						for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
							for (pat=rsum->pats; pat != NULL; pat=pat->nextword) {
								if (uS.patmat(rule->name, pat->word)) {
									rsum->scoreSum += addScore;
								}
							}
							if (pat != NULL)
								break;
						}
					}
					break;
				}
			}
			if (resRule == NULL) {
				if (isOutput) {
					if (rule->info != NULL)
						fprintf(fpout, "Rule: %s - %s\n", rule->name, rule->info);
					else
						fprintf(fpout, "Rule: %s\n", rule->name);
				}
				if (rule->adds == NULL)
					addScore = 0;
				else {
					isErr = FALSE;
					addScore = getAdditionalScore(rule, sp->resRules, &isErr, spareTier3);
					if (isErr) {
						return(FALSE);
					}
				}

				if (addScore > 2)
					addScore = 2;

				if (isOutput) {
					if (addScore > 0) {
						fprintf(fpout, "    ADD: score: %d from %s\n", addScore, spareTier3);
					}
					fprintf(fpout, "  Score: %d\n\n", addScore);
				}
				if (addScore > 0) {
					totalSum += addScore;
					if (isOutput || onlydata == 2) {
						for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
							for (pat=rsum->pats; pat != NULL; pat=pat->nextword) {
								if (uS.patmat(rule->name, pat->word)) {
									rsum->scoreSum += addScore;
								}
							}
							if (pat != NULL)
								break;
						}
					}
				}
			}
		}
		if (onlydata == 2) {
			sFName = strrchr(sp->fname, PATHDELIMCHR);
			if (sFName != NULL)
				sFName = sFName + 1;
			else
				sFName = sp->fname;
			excelRow(fpout, ExcelRowStart);
			excelStrCell(fpout, sFName);
			if (sp->ID) {
				excelOutputID(fpout, sp->ID);
			} else {
				excelCommasStrCell(fpout, ".,.");
				excelStrCell(fpout, sp->speaker);
				excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.,.,.,.");
			}
			excelStrCell(fpout, Design);
			excelStrCell(fpout, Activity);
			excelStrCell(fpout, Group);
			for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
				excelNumCell(fpout, "%.0f", rsum->scoreSum);
			}
			excelNumCell(fpout, "%.0f", totalSum);
			excelRow(fpout, ExcelRowEnd);
		}
		if (isOutput) {
			for (rsum=root_rsum; rsum != NULL; rsum=rsum->next_rsum) {
				fprintf(fpout, "%s = %d\n", rsum->name, rsum->scoreSum);
			}
		}
		if (isOutput) {
			fprintf(fpout, "\nTotal = %d\n", totalSum);
		}
		sp->TotalScore = (float) totalSum;
		freeResRules(sp->resRules);
		sp->resRules = NULL;
	}
	if (onlydata == 2) {
		excelFooter(fpout);
	}
	return(TRUE);
}

IPSYN_SP *ipsyn_FindSpeaker(char *sp, char *ID, char isSpeakerFound) {
	char isCombSpeakers;
	IPSYN_SP *ts;
	
	uS.remblanks(sp);
	if (onlydata == 2) {
		isCombSpeakers = FALSE;
	} else if (combinput) {
		isCombSpeakers = TRUE;
	} else {
		isCombSpeakers = FALSE;
	}
	if (root_sp == NULL) {
		if ((ts=NEW(IPSYN_SP)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		root_sp = ts;
	} else {
		for (ts=root_sp; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->speaker, sp, FALSE, FALSE) && (ts->fileID == fileID || isCombSpeakers)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
			if (ts->next_sp == NULL)
				break;
		}
		if ((ts->next_sp=NEW(IPSYN_SP)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->isSpeakerFound = isSpeakerFound;
	ts->fname = NULL;
	ts->speaker = NULL;
	ts->utts = NULL;
	ts->cUtt = NULL;
	ts->UttNum = 0;
	ts->TotalScore = 0.0;
	ts->resRules = NULL;
	ts->fileID = fileID;
	ts->ID = NULL;
	ts->speaker = (char *)malloc(strlen(sp)+1);
	if (ts->speaker == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		return(NULL);
	}
	strcpy(ts->speaker, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(ts->ID, ID);
	}
	if (onlydata == 2) {
		if ((ts->fname=(char *)malloc(strlen(oldfname)+1)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		strcpy(ts->fname, oldfname);
	}
	return(ts);
}

char isIPRepeatUtt(IPSYN_SP *ts, char *line) {
	int  k;
	char isEmpty;
	IPSYN_UTT *utt;
	
	isEmpty = TRUE;
	for (k=0; line[k] != EOS; k++) {
		if (!isSpace(line[k]) && line[k] != '\n' && line[k] != '.' && line[k] != '!' && line[k] != '?')
			isEmpty = FALSE;
	}
	if (isEmpty)
		return(TRUE);
	for (utt=ts->utts; utt != NULL; utt=utt->next_utt) {
		if (strcmp(utt->spkLine, line) == 0)
			return(TRUE);
	}
	return(FALSE);
}

char forceInclude(char *line) {
	if (isIPpostcode) {
		if (isPostCodeOnUtt(line, "[+ ip]")) {
			return(TRUE);
		}
	}
	return(FALSE);
}

char isExcludeIpsynTier(char *sp, char *line, char *tline) {
	int i;
	char *c, isUttDelFound, isXXXFound;

	if (isIPEpostcode) {
		if (isPostCodeOnUtt(line, "[+ ipe]")) {
			return(TRUE);
		}
	}

	i = 0;
	isXXXFound = FALSE;
	while ((i=getword(sp, line, templineC4, NULL, i))) {
		if (uS.mStricmp(templineC4, "xxx") == 0 || uS.mStricmp(templineC4, "yyy") == 0 || uS.mStricmp(templineC4, "www") == 0)
			isXXXFound = TRUE;
		else if (templineC4[0] != '<' && templineC4[0] != '>' && templineC4[0] != '[' && templineC4[0] != ']' &&
				 templineC4[0] != '#' && templineC4[0] != '&' && templineC4[0] != '+' &&
				 templineC4[0] != '.' && templineC4[0] != '!' && templineC4[0] != '?' &&
				 templineC4[0] != HIDEN_C) {
			isXXXFound = FALSE;
			break;
		}
	}
	if (isXXXFound)
		return(TRUE);
/* 2020-06-08
	for (c=line; *utterance->speaker, ; c++) {
		if (*c == 'x' || *c == 'X') {
			if ((*(c+1) == 'x' || *(c+1) == 'X') &&
				(*(c+2) == 'x' || *(c+2) == 'X' || !isalpha(*(c+2)))) {
				return(TRUE);
			}
		} else if (*c == 'y' || *c == 'Y') {
			if ((*(c+1) == 'y' || *(c+1) == 'Y') &&
				(*(c+2) == 'y' || *(c+2) == 'Y' || !isalpha(*(c+2)))) {
				return(TRUE);
			}
		} else if (*c == 'w' || *c == 'W') {
			if ((*(c+1) == 'w' || *(c+1) == 'W') &&
				(*(c+2) == 'w' || *(c+2) == 'W' || !isalpha(*(c+2)))) {
				return(TRUE);
			}
		}
	}
*/
	isUttDelFound = FALSE;
	for (c=tline; *c; c++) {
		if (*c == '.' || *c == '!' || *c == '?')
			isUttDelFound = TRUE;
	}
	if (isUttDelFound == FALSE)
		return(TRUE);
	return(FALSE);
}

char add2Utts(IPSYN_SP *sp, char *spkLine) {
	IPSYN_UTT *utt;

	if (sp->utts == NULL) {
		if ((utt=NEW(IPSYN_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		sp->utts = utt;
	} else {
		for (utt=sp->utts; utt->next_utt != NULL; utt=utt->next_utt) ;
		if ((utt->next_utt=NEW(IPSYN_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		utt = utt->next_utt;
	}
	utt->next_utt = NULL;
	utt->spkLine = NULL;
	utt->morLine = NULL;
	utt->graLine = NULL;
	utt->words = NULL;
	utt->ln = lineno+tlineno;
	utt->spkLine = (char *)malloc(strlen(spkLine)+1);
	if (utt->spkLine == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		return(FALSE);
	}
	strcpy(utt->spkLine, spkLine);
	sp->cUtt = utt;
	return(TRUE);
}

static void ips_notEnough(void) {
	IPSYN_SP *tsp;

	for (tsp=root_sp; tsp != NULL; tsp=tsp->next_sp) {
		if (IPS_UTTLIM == 0) {
			fprintf(stderr, "DID NOT VERIFY SAMPLE SIZE OF COMPLETE UTTERANCES.\n");
			fprintf(stderr, "%s FOUND %d COMPLETE UTTERANCES\n", tsp->speaker, tsp->UttNum);
			return;
		} else if (tsp->UttNum < IPS_UTTLIM) {
			fprintf(stderr, "\n\nCAN'T FIND NECESSARY SAMPLE SIZE OF %d OF COMPLETE UTTERANCES.\n", IPS_UTTLIM);
			fprintf(stderr, "%s FOUND ONLY %d COMPLETE UTTERANCES\n", tsp->speaker, tsp->UttNum);
			fprintf(stderr, "IF YOU WANT TO USE SMALLER SAMPLE SIZE, THEN USE \"+c\" OPTION\n\n");
			if (fpout != stdout && onlydata != 2) {
				fprintf(fpout, "\n\nCAN'T FIND NECESSARY SAMPLE SIZE OF %d OF COMPLETE UTTERANCES.\n", IPS_UTTLIM);
				fprintf(fpout, "%s FOUND ONLY %d COMPLETE UTTERANCES\n", tsp->speaker, tsp->UttNum);
				fprintf(fpout, "IF YOU WANT TO USE SMALLER SAMPLE SIZE, THEN USE \"+c\" OPTION\n\n");
			}
			tsp->isSpeakerFound = FALSE;
		}
	}
}

char compute_ipsyn(char isOutput) {
	freeSpkLines(root_sp);
	if (!createIPSynCoding(isOutput)) {
		return(FALSE);
	}
	if (isOutput)
		root_sp = freeIpsynSpeakers(root_sp);
	return(TRUE);
}

void ipsyn_freeSpeakers(void) {
	root_sp = freeIpsynSpeakers(root_sp);
}

#ifndef KIDEVAL_LIB
static void ips_pr_result(void) {
	ips_notEnough();
	if (!compute_ipsyn(TRUE)) {
		ipsyn_overflow();
		cutt_exit(0);
	}
}

void call() {
	int  i, j;
	char lRightspeaker, isFC, isGRA;
	FNType tfname[FNSize];
	FILE *tf;
	IPSYN_SP *ts, *lastTs;

	if (onlydata != 2) {
		if (is100Limit)
			parsfname(oldfname, tfname, ".ipcore-100");
		else
			parsfname(oldfname, tfname, ".ipcore");
		if ((tfp=openwfile(oldfname,tfname,tfp)) == NULL) {
			fprintf(stderr,"WARNING: Can't open ipsyn file %s.\n", tfname);
		}
	}
	fileID++;
	ts = NULL;
	lastTs = NULL;
	lRightspeaker = FALSE;
	isGRA = TRUE;
	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
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
				ts = NULL;
			}
			if (!lRightspeaker)
				continue;
		}
		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC1, TRUE)) {
					uS.remblanks(utterance->line);
					if (ipsyn_FindSpeaker(templineC1, utterance->line, FALSE) == NULL) {
						ipsyn_overflow();
						cutt_exit(0);
					}
				}
			} else if (uS.partcmp(utterance->speaker,"@Types:",FALSE,FALSE)) {
				for (i=0; utterance->line[i] != EOS && isSpace(utterance->line[i]); i++) ;
				if (utterance->line[i] != EOS) {
					j = 0;
					for (; j < TYPESLEN && utterance->line[i] != EOS && utterance->line[i] != ',' && !isSpace(utterance->line[i]); i++) {
						Design[j++] = utterance->line[i];
					}
					Design[j] = EOS;
					for (; utterance->line[i] != EOS && (utterance->line[i] == ',' || isSpace(utterance->line[i])); i++) ;
					if (utterance->line[i] != EOS) {
						j = 0;
						for (; j < TYPESLEN && utterance->line[i] != EOS && utterance->line[i] != ',' && !isSpace(utterance->line[i]); i++) {
							Activity[j++] = utterance->line[i];
						}
						Activity[j] = EOS;
						for (; utterance->line[i] != EOS && (utterance->line[i] == ',' || isSpace(utterance->line[i])); i++) ;
						if (utterance->line[i] != EOS) {
							j = 0;
							for (; j < TYPESLEN && utterance->line[i] != EOS && utterance->line[i] != ',' && !isSpace(utterance->line[i]); i++) {
								Group[j++] = utterance->line[i];
							}
							Group[j] = EOS;
							for (; utterance->line[i] != EOS && utterance->line[i] != ',' && !isSpace(utterance->line[i]); i++) ;
						}
					}
				}
			}
		} else if (utterance->speaker[0] == '*') {
			if (lastTs != NULL) {
				removeLastUtt(lastTs);
				lastTs = NULL;
			} else if (!isGRA) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
				fprintf(stderr,"Speaker tier does not have corresponding %%GRA tier.\n");
				ipsyn_overflow();
				cutt_exit(0);
			}
			ts = ipsyn_FindSpeaker(utterance->speaker, NULL, TRUE);
			if (ts == NULL) {
				ipsyn_overflow();
				cutt_exit(0);
			}
			if (IPS_UTTLIM > 0 && ts->UttNum >= IPS_UTTLIM) {
				ts = NULL;
				lRightspeaker = FALSE;
			} else {
				cleanUttline(uttline);
				isFC = forceInclude(utterance->line);
				if (!isFC && (strcmp(uttline, spareTier1) == 0 ||
							  isExcludeIpsynTier(utterance->speaker, utterance->line, uttline))) {
					spareTier1[0] = EOS;
					ts = NULL;
					lRightspeaker = FALSE;
				} else if (!isIPRepeatUtt(ts, uttline) || isFC) {
					ts->UttNum++;
					if (!add2Utts(ts, uttline)) {
						ipsyn_overflow();
						cutt_exit(0);
					}
					lastTs = ts;
					strcpy(spareTier1, uttline);
					att_cp(0,sTier->speaker,utterance->speaker,sTier->attSp,utterance->attSp);
					att_cp(0,sTier->line,utterance->line,sTier->attLine,utterance->attLine);
					if (tfp != NULL) {
						tf = fpout;
						fpout = tfp;
						fpout = tf;
					}
				} else {
					spareTier1[0] = EOS;
					ts = NULL;
					lRightspeaker = FALSE;
				}
			}
		} else if (ts != NULL) {
			if (fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
				lastTs = NULL;
				if (tfp != NULL) {
					tf = fpout;
					fpout = tfp;
					printout(sTier->speaker,sTier->line,sTier->attSp,sTier->attLine,FALSE);
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					fpout = tf;
				}
				strcpy(utterance->tuttline,utterance->line);
				createMorUttline(utterance->line, org_spTier, "%mor:", utterance->tuttline, TRUE, TRUE);
				strcpy(utterance->tuttline, utterance->line);
				if (strchr(uttline, dMarkChr) == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
					fprintf(stderr,"%s tier does not have one-to-one correspondence to its speaker tier.\n", utterance->speaker);
					fprintf(stderr,"Speaker and dependent tiers:\n");
					fprintf(stderr,"%s%s", org_spName, org_spTier);
					fprintf(stderr,"%s%s\n", utterance->speaker, utterance->line);
					ipsyn_overflow();
					cutt_exit(0);
				}
				if (ts->cUtt == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
					fprintf(stderr,"INTERNAL ERROR utt == null");
					ipsyn_overflow();
					cutt_exit(0);
				}
				ts->cUtt->morLine = (char *)malloc(strlen(utterance->tuttline)+1);
				if (ts->cUtt->morLine == NULL) {
					fprintf(stderr,"Error: out of memory\n");
					ipsyn_overflow();
					cutt_exit(0);
				}
				strcpy(ts->cUtt->morLine, utterance->tuttline);
				isGRA = FALSE;
			} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
				if (tfp != NULL) {
					tf = fpout;
					fpout = tfp;
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					fpout = tf;
				}
				if (ts->cUtt == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
					fprintf(stderr,"INTERNAL ERROR utt == null");
					ipsyn_overflow();
					cutt_exit(0);
				}
				ts->cUtt->graLine = (char *)malloc(strlen(utterance->tuttline)+1);
				if (ts->cUtt->graLine == NULL) {
					fprintf(stderr,"Error: out of memory\n");
					ipsyn_overflow();
					cutt_exit(0);
				}
				strcpy(ts->cUtt->graLine, utterance->tuttline);
				isGRA = TRUE;
			}
		}
	}
	if (!combinput) {
		if (tfp != NULL) {
			fclose(tfp);
			tfp = NULL;
		}
		ips_pr_result();
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  i;

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = IPSYN;
	OnlydataLimit = 2;
	UttlineEqUtterance = FALSE;
	root_rules = NULL;
	root_pats = NULL;
	root_vars = NULL;
	root_rsum = NULL;
	tempPat.duplicatesList = NULL;
	tempPat.origmac = NULL;
	tempPat.flatmac = NULL;
	sTier = NEW(UTTER);
	if (sTier == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		cutt_exit(0);
	}
	is100Limit = FALSE;
	IPS_UTTLIM = 50; // 2022-01-27 100// 2019-08-26 50;
	for (i=1; i < argc; i++) {
		if ((*argv[i] == '+' || *argv[i] == '-') && argv[i][1] == 'o') {
			IPS_UTTLIM = 100;
			is100Limit = TRUE;
			break;
		}
	}
	sTier->speaker[0] = EOS;
	sTier->line[0] = EOS;
	sTier->tuttline[0] = EOS;
	bmain(argc,argv,ips_pr_result);
	if (sTier != NULL) {
		free(sTier);
	}
	if (combinput) {
		if (tfp != NULL) {
			fclose(tfp);
			tfp = NULL;
		}
	}
	ipsyn_cleanSearchMem();
}
#endif // KIDEVAL_LIB

// BEGIN reading rules
static void ipsyn_err(char *exp, int pos, char isforce) {
	int  i;
	
	if (exp[pos] == EOS && !isforce)
		return;
	for (i=0; exp[i]; i++) {
		if (i == pos) {
#ifndef UNX
			fprintf(stderr, "%c%c", ATTMARKER, error_start);
#endif
			fprintf(stderr, "(1)");
#ifndef UNX
			fprintf(stderr, "%c%c", ATTMARKER, error_end);
#endif
		}
		putc(exp[i],stderr);		
	}
	if (isforce) {
		if (i == pos) {
#ifndef UNX
			fprintf(stderr, "%c%c", ATTMARKER, error_start);
#endif
			fprintf(stderr, "(1)");
#ifndef UNX
			fprintf(stderr, "%c%c", ATTMARKER, error_end);
#endif
		}
	}
}

static PATS_LIST *addToPatsList(int ln, char *pat_str) {
	char res, isClitic, *gra_st;
	PATS_LIST *pat_elem;

	if (pat_str[0] == '~' || pat_str[0] == '$') {
		isClitic = pat_str[0];
		strcpy(pat_str, pat_str+1);
	} else
		isClitic = '\0';
	if (root_pats == NULL) {
		if ((pat_elem=NEW(PATS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		root_pats = pat_elem;
	} else {
		for (pat_elem=root_pats; 1; pat_elem=pat_elem->next_pat) {
			if (uS.mStricmp(pat_elem->pat_str, pat_str) == 0 && pat_elem->isPClitic == isClitic)
				return(pat_elem);
			if (pat_elem->next_pat == NULL)
				break;
		}
		if ((pat_elem->next_pat=NEW(PATS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		pat_elem = pat_elem->next_pat;
	}
	pat_elem->next_pat = NULL;
	pat_elem->pat_feat = NULL;
	pat_elem->pat_str = NULL;
	pat_elem->pat_feat = NULL;
	if (isMorPat(pat_str))
		pat_elem->whatType = MOR_TIER;
	else if (isGraPat(pat_str)) {
		pat_elem->whatType = GRA_TIER;
		gra_st = strrchr(pat_str, '|');
		if (gra_st != NULL)
			strcpy(pat_str, gra_st+1);
	} else
		pat_elem->whatType = SPK_TIER;
	if ((pat_elem->pat_str=(char *)malloc(strlen(pat_str)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr,"Error: out of memory\n");
		return(NULL);
	}
	strcpy(pat_elem->pat_str, pat_str);
	pat_elem->isPClitic = isClitic;
	if (pat_elem->whatType == MOR_TIER) {
		if (pat_str[0] != EOS && strcmp(pat_str, "_") != 0) {
			pat_elem->pat_feat = makeMorSeachList(pat_elem->pat_feat, &res, pat_str, 'i');
			if (pat_elem->pat_feat == NULL) {
				fprintf(stderr,"*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr,"*** PLEASE READ MESSAGE ABOVE PREVIOUS LINE ***\n");
				return(NULL);
			}
		}
	}
	return(pat_elem);
}

static PAT_ELEMS *mkPatElem(char isTrueAnd) {
	PAT_ELEMS *t;

	if ((t=NEW(PAT_ELEMS)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		return(NULL);
	}
	t->pat_elem = NULL;
	t->neg = 0;
	if (isTrueAnd)
		t->wild = 1;
	else
		t->wild = 0;
	t->isRepeat = 0;
	t->isCheckedForDups = FALSE;
	t->isAllNeg = FALSE;
	t->matchedWord = NULL;
	t->parenElem = NULL;
	t->refParenElem = NULL;
	t->andElem = NULL;
	t->refAndElem = NULL;
	t->orElem = NULL;
	return(t);
}

static char getVarValue(char *name, char *to, char *isRepeat) {
	VARS_LIST *var;
	
	for (var=root_vars; var != NULL; var=var->next_var) {
		if (uS.mStricmp(var->name, name) == 0) {
			*isRepeat = var->isRepeat;
			strcpy(to, var->line);
			return(TRUE);
		}
	}
	return(FALSE);
}

static char expandVarInLine(int ln, char *isRepeat, char *from, char *to) {
	int  i, j, initial;
	char *name, t;
	
	i = 0;
	j = 0;
	while (from[i] != EOS) {
		if (from[i] == '$' && (i == 0 || !isalnum(from[i-1])) && isalnum(from[i+1])) {
			initial = i;
			i++;
			name = from + i;
			while (from[i] != EOS && !isSpace(from[i]) && from[i] != ',' && from[i] != '+' &&
				   from[i] != '^' && from[i] != '(' && from[i] != ')') {
				i++;
			}
			t = from[i];
			from[i] = EOS;
			if (!getVarValue(name, templineC4, isRepeat)) {
				if (strcmp(name, "b") == 0 || strcmp(name, "B") == 0) {
					templineC4[0] = BEGSYM;
					templineC4[1] = EOS;
				} else if (strcmp(name, "e") == 0 || strcmp(name, "E") == 0) {
					templineC4[0] = ENDSYM;
					templineC4[1] = EOS;
				} else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Undecleared variable: %s\n", name);
					return(FALSE);
				}
			}
			from[i] = t;
			if ((initial > 0 || t != EOS) && *isRepeat) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Illegal use of repeating variable \"%s\" along with other strings in:\n", name);
				fprintf(stderr, "    %s\n", from);
				return(FALSE);
			}
			if ((initial > 0 || t != EOS) && templineC4[0] == '(') {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Illegal use of variable \"%s\" containing operator(s) '+' or '^'\n", name);
				fprintf(stderr, "    along with other strings in: %s\n", from);
				return(FALSE);
			}
			strcpy(to+j, templineC4);
			j = strlen(to);
		} else
			to[j++] = from[i++];
	}
	to[j] = EOS;
	if (strcmp(from, to) == 0) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    Undecleared or invalid variable in string: %s\n", from);
		return(FALSE);
	}
	return(TRUE);
}

static long makePatElems(int ln, char *rname, const char *part, char *exp, long pos, PAT_ELEMS *elem, char isTrueAnd);

static long storepats(int ln, char *rname, const char *part, PAT_ELEMS *elem, char *exp, long pos, char isTrueAnd) {
	long p1, p2;
	char t, isRepeat, *buf;
	PATS_LIST *pl;

	p1 = pos;
	do {
		t = FALSE;
		for (; !uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF) && 
			 !uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,')',&dFnt,C_MBF) &&
			 !uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) && exp[pos] != EOS; pos++) ;
		if (pos-1 < p1 || !uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF))
			break;
		else if (exp[pos] == EOS) {
			if (uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF) && !uS.isRightChar(exp, pos-2, '\\', &dFnt, C_MBF)) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "Unexpected ending.\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			}
			break;
		} else
			pos++;
	} while (1) ;
	if (uS.isRightChar(exp,pos,'!', &dFnt, C_MBF) || uS.isRightChar(exp,pos,'(', &dFnt, C_MBF)) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
		fprintf(stderr, "Unexpected element \"!\" or '\"(\" found.\n");
		ipsyn_err(exp,pos,FALSE);
		return(-1L);
	}
	p2 = pos;
	for(; p1!=p2 && uS.isskip(exp,p1,&dFnt,C_MBF) && exp[p1] != '.' && exp[p1] != '!' && exp[p1] != '?'; p1++);
	if (p1 != p2) {
		for (p2--; p2 >= p1 && uS.isskip(exp,p2,&dFnt,C_MBF) && exp[p2] != '.' && exp[p2] != '!' && exp[p2] != '?'; p2--) ;
		p2++;
	}
	if (uS.isRightChar(exp,p2-1,'*',&dFnt,C_MBF) && (!uS.isRightChar(exp,p2-2,'\\',&dFnt,C_MBF) || uS.isRightChar(exp,p2-3,'\\',&dFnt,C_MBF))) {
		p2--;
		for (; p2 >= p1 && uS.isRightChar(exp,p2,'*',&dFnt,C_MBF); p2--) ;
		if (uS.isRightChar(exp,p2,'\\',&dFnt,C_MBF) && (!uS.isRightChar(exp,p2-1,'\\',&dFnt,C_MBF) || uS.isRightChar(exp,p2-2,'\\',&dFnt,C_MBF)))
			p2 += 2;	
		else
			p2++;
		if (p1 == p2) {
			elem->wild = 1;
		} else
			p2++;
		t = exp[p2];
		exp[p2] = EOS;
		strcpy(templineC2, exp+p1);
		exp[p2] = t;
		if (templineC2[0] == EOS && isTrueAnd) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
			fprintf(stderr, "Character '*' as an item is not allowed with TRUE AND option.\n");
			ipsyn_err(exp,p1,FALSE);
			return(-1L);
		}
	} else {
		if (p1 == p2) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
			if (exp[p2] == EOS) {
				fprintf(stderr, "Unexpected ending.\n");
			} else {
				fprintf(stderr, "Missing item or item is part of punctuation/delimiters set.\n");
			}
			ipsyn_err(exp,p1,FALSE);
			return(-1L);
		}
		t = exp[p2];
		exp[p2] = EOS;
		strcpy(templineC2, exp+p1);
		exp[p2] = t;
	}
	if (strchr(templineC2, '$') == NULL) {
		pl = addToPatsList(ln, templineC2);
		if (pl == NULL) {
			return(-1);
		}
		elem->pat_elem = pl;
	} else {
		isRepeat = 0;
		do {
			if (!expandVarInLine(ln, &isRepeat, templineC2, templineC3)) {
				return(-1);
			}
			if (templineC3[0] == '(') {
				if (isRepeat) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Repeating variable is not allowed to have operator(s):\n");
					fprintf(stderr, "    %s += %s\n", templineC2, templineC3);
					return(-1);
				} else {
					buf = (char *)malloc(strlen(templineC3)+1);
					if (buf == NULL) {
						fprintf(stderr,"Error: out of memory\n");
						return(-1);
					}
					strcpy(buf, templineC3);
					elem->pat_elem = NULL;
					p1 = 1;
					elem->parenElem = mkPatElem(isTrueAnd);
					if (elem->parenElem == NULL) {
						return(-1);
					}
					p1 = makePatElems(ln,rname,part,buf,p1,elem->parenElem,isTrueAnd);
					if (p1 == -1L)
						return(-1L);
					if (!uS.isRightChar(buf,p1,')',&dFnt,C_MBF)) {
						fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
						fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
						fprintf(stderr, "No matching ')' found.\n");
						ipsyn_err(buf,p1,FALSE);
						return(-1L);
					}
					if (buf[p1])
						p1++;
					if (!uS.isRightChar(buf,p1,'+',&dFnt,C_MBF) && !uS.isRightChar(buf,p1,'^',&dFnt,C_MBF) && 
						!uS.isRightChar(buf,p1,')',&dFnt,C_MBF) && buf[p1] != EOS) {
						fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
						fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
						fprintf(stderr, "Expected element \"+\" or \"^\" or '\")\".\n");
						ipsyn_err(buf,p1,FALSE);
						return(-1L);
					}
					free(buf);
				}
				return(pos);
			} else {
				strcpy(templineC2, templineC3);
			}
		} while (strchr(templineC2, '$') != NULL) ;
		elem->isRepeat = isRepeat;
		pl = addToPatsList(ln, templineC2);
		if (pl == NULL) {
			return(-1);
		}
		elem->pat_elem = pl;
	}
	return(pos);
}

static long makePatElems(int ln, char *rname, const char *part, char *exp, long pos, PAT_ELEMS *elem, char isTrueAnd) {
	if (uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
		fprintf(stderr, "Unexpected element \"+\" or \"^\" or '\")\" found.\n");
		ipsyn_err(exp,pos,FALSE);
		return(-1L);
	}
	while (exp[pos] != EOS && !uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
		if (uS.isRightChar(exp,pos,'(',&dFnt,C_MBF)) {
			pos++;
			elem->parenElem = mkPatElem(isTrueAnd);
			if (elem->parenElem == NULL) {
				return(-1);
			}
			pos = makePatElems(ln,rname,part,exp,pos,elem->parenElem,isTrueAnd);
			if (pos == -1L)
				return(-1L);
			if (!uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "No matching ')' found.\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			}
			if (exp[pos])
				pos++;
			if (!uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) && 
				!uS.isRightChar(exp,pos,')',&dFnt,C_MBF) && exp[pos] != EOS) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "Expected element \"+\" or \"^\" or '\")\".\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) { // not
			pos++;
			elem->neg = 1;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || 
				uS.isRightChar(exp,pos,')',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "Unexpected element \"+\" or \"^\" or '\"!\" or '\")\" found.\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF)) {
				pos = storepats(ln, rname, part, elem, exp, pos, isTrueAnd);
				if (pos == -1L)
					return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF)) { // or
			elem->orElem = mkPatElem(isTrueAnd);
			if (elem->orElem == NULL) {
				return(-1);
			}
			elem = elem->orElem;
			pos++;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "Unexpected element \"+\" or \"^\" or '\")\" found.\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				pos = storepats(ln, rname, part, elem, exp, pos, isTrueAnd);
				if (pos == -1L)
					return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'^',&dFnt,C_MBF)) { // and
			elem->andElem = mkPatElem(isTrueAnd);
			if (elem->andElem == NULL) {
				return(-1);
			}
			elem = elem->andElem;
			pos++;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "ERROR: In rule name \"%s\" %s part:\n", rname, part);
				fprintf(stderr, "Unexpected element \"+\" or \"^\" or '\")\" found.\n");
				ipsyn_err(exp,pos,FALSE);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				pos = storepats(ln, rname, part, elem, exp, pos, isTrueAnd);
				if (pos == -1L)
					return(-1L);
			}
		} else {
			pos = storepats(ln, rname, part, elem, exp, pos, isTrueAnd);
			if (pos == -1L)
				return(-1L);
		}
	}
	return(pos);
}

static void echo_expr(PAT_ELEMS *elem, char isEchoOr, char *str) {
	if (elem->parenElem != NULL) {
		if (elem->neg)
			strcat(str, "!");
		strcat(str, "(");
		echo_expr(elem->parenElem, isEchoOr, str);
		strcat(str, ")");
	} else if (elem->pat_elem != NULL) {
		if (elem->neg)
			strcat(str, "!");
		if (elem->pat_elem->pat_str[0] == EOS)
			strcat(str, "{*}");
		else if (strcmp(elem->pat_elem->pat_str, "_") == 0)
			strcat(str, "{_}");
		else
			strcat(str, elem->pat_elem->pat_str);
	}
	if (elem->andElem != NULL) {
		strcat(str, "^");
		echo_expr(elem->andElem, isEchoOr, str);
	}
	if (isEchoOr && elem->orElem != NULL) {
		strcat(str, "+");
		echo_expr(elem->orElem, isEchoOr, str);
	}
}

static char isInLst(IEWORDS *twd, char *str) {	
	for (; twd != NULL; twd = twd->nextword) {
		if (strcmp(str, twd->word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static IEWORDS *InsertFlatp(IEWORDS *lst, char *str) {
	IEWORDS *t;

	if ((t=NEW(IEWORDS)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		return(NULL);
	}
	t->word = (char *)malloc(strlen(str)+1);
	if (t->word == NULL) {
		free(t);
		fprintf(stderr,"Error: out of memory\n");
		return(NULL);
	}
	strcpy(t->word, str);
	t->nextword = lst;
	lst = t;
	return(lst);
}

static char removeDuplicateFlatMacs(char isEcho) {
	PAT_ELEMS *flatp, *lflatp;
	IEWORDS *p;

	lflatp = tempPat.flatmac;
	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		if (!flatp->isCheckedForDups) {
			flatp->isCheckedForDups = TRUE;
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			if (!isInLst(tempPat.duplicatesList, templineC1)) {
				p = InsertFlatp(tempPat.duplicatesList, templineC1);
				if (p == NULL) {
					return(FALSE);
				}
				tempPat.duplicatesList = p;
				if (isEcho)
					fprintf(stderr, "%s\n", templineC1);
			} else {
				lflatp->orElem = flatp->orElem;
				freeIpsynElemPats(flatp, FALSE);
				flatp = lflatp;
			}
		} else if (isEcho) {
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			fprintf(stderr, "%s\n", templineC1);
		}
		lflatp = flatp;
	}
	return(TRUE);
}

static PAT_ELEMS *getLastElem(PAT_ELEMS *elem) {
	int i;

	for (i=0; i < tempPat.stackAndsI; i++) {
		if (elem == NULL) {
			return(NULL);
		} else if (elem->andElem != NULL) {
			if (elem->refAndElem != tempPat.stackAnds[i])
				return(NULL);
			else
				elem = elem->andElem;
		} else if (elem->parenElem != NULL) {
			if (elem->refParenElem != tempPat.stackAnds[i])
				return(NULL);
			else
				elem = elem->parenElem;
		} else
			return(NULL);
	}
	return(elem);
}

static char addParenToLastFlatMacs(PAT_ELEMS *cur, char isTrueAnd) {
	PAT_ELEMS *elem, *flatp;

	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->parenElem == NULL && elem->pat_elem == NULL) {
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->matchedWord = cur->matchedWord;
			elem->refParenElem = cur->parenElem;
			elem->parenElem = mkPatElem(isTrueAnd);
			if (elem->parenElem == NULL) {
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

static void addPatToLastFlatMacs(PAT_ELEMS *cur) {
	PAT_ELEMS *elem, *flatp;

	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->parenElem == NULL && elem->pat_elem == NULL) {
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->matchedWord = cur->matchedWord;
			elem->pat_elem = cur->pat_elem;
		}
	}
}

static char addAndToFlatMacs(PAT_ELEMS *cur, char isTrueAnd) {
	PAT_ELEMS *elem, *flatp;

	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->andElem == NULL) {
			elem->refAndElem = cur->andElem;
			elem->andElem = mkPatElem(isTrueAnd);
			if (elem->andElem == NULL) {
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

static char copyMac(PAT_ELEMS *elem, PAT_ELEMS *lastElem, PAT_ELEMS *flatp, char isTrueAnd) {
	if (flatp != NULL && lastElem != flatp) {
		elem->wild = flatp->wild;
		elem->neg = flatp->neg;
		elem->matchedWord = flatp->matchedWord;
		if (flatp->parenElem != NULL) {
			elem->refParenElem = flatp->refParenElem;
			elem->parenElem = mkPatElem(isTrueAnd);
			if (elem->parenElem == NULL) {
				return(FALSE);
			}
			if (!copyMac(elem->parenElem, lastElem, flatp->parenElem, isTrueAnd)) {
				return(FALSE);
			}
		} else if (flatp->pat_elem != NULL) {
			elem->pat_elem = flatp->pat_elem;
		}
		if (flatp->andElem != NULL) {
			elem->refAndElem = flatp->refAndElem;
			elem->andElem = mkPatElem(isTrueAnd);
			if (elem->andElem == NULL) {
				return(FALSE);
			}
			if (!copyMac(elem->andElem, lastElem, flatp->andElem, isTrueAnd)) {
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

static char duplicateFlatMacs(char isTrueAnd, char isResetDuplicates) {
	PAT_ELEMS *elem, *elemRoot, *flatp, *lastElem;

	if (isResetDuplicates) {
		tempPat.duplicatesList = freeIEWORDS(tempPat.duplicatesList);
		for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
			flatp->isCheckedForDups = FALSE;
		}
	}
	if (!removeDuplicateFlatMacs(FALSE)) {
		return(FALSE);
	}
	elemRoot = NULL;
	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		lastElem = getLastElem(flatp);
		if (lastElem != NULL) {
			if (elemRoot == NULL) {
				elemRoot = mkPatElem(isTrueAnd);
				if (elemRoot == NULL) {
					return(FALSE);
				}
				elem = elemRoot;
			} else {
				for (elem=elemRoot; elem->orElem != NULL; elem=elem->orElem) ;
				elem->orElem = mkPatElem(isTrueAnd);
				if (elem->orElem == NULL) {
					return(FALSE);
				}
				elem = elem->orElem;
			}
			if (!copyMac(elem, lastElem, flatp, isTrueAnd)) {
				return(FALSE);
			}
		}
	}
	if (tempPat.flatmac == NULL) {
		tempPat.flatmac = elemRoot;
	} else {
		for (flatp=tempPat.flatmac; flatp->orElem != NULL; flatp=flatp->orElem) ;
		flatp->orElem = elemRoot;
	}
	return(TRUE);
}

static char addToStack(PAT_ELEMS *elem) {
	if (tempPat.stackAndsI >= 200) {
		fprintf(stderr, "ipsyn: stack index exceeded 200 \"^\" elements\n");
		return(FALSE);
	}
	tempPat.stackAnds[tempPat.stackAndsI] = elem;
	tempPat.stackAndsI = tempPat.stackAndsI + 1;
	return(TRUE);
}

static char flatten_expr(PAT_ELEMS *cur, char isTrueAnd, char isResetDuplicates) {
	if (cur->parenElem != NULL) {
		if (!addParenToLastFlatMacs(cur, isTrueAnd)) {
			return(FALSE);
		}
		if (!addToStack(cur->parenElem)) {
			return(FALSE);
		}
		if (!flatten_expr(cur->parenElem, isTrueAnd, FALSE)) {
			return(FALSE);
		}
		tempPat.stackAndsI = tempPat.stackAndsI - 1;
	} else if (cur->pat_elem != NULL) {
		addPatToLastFlatMacs(cur);
	}
	if (cur->andElem != NULL) {
		if (!addAndToFlatMacs(cur, isTrueAnd)) {
			return(FALSE);
		}
		if (!addToStack(cur->andElem)) {
			return(FALSE);
		}
		if (!flatten_expr(cur->andElem, isTrueAnd, FALSE)) {
			return(FALSE);
		}
		tempPat.stackAndsI = tempPat.stackAndsI - 1;
	}
	if (cur->orElem != NULL) {
		if (!duplicateFlatMacs(isTrueAnd, isResetDuplicates)) {
			return(FALSE);
		}
		if (!flatten_expr(cur->orElem, isTrueAnd, isResetDuplicates)) {
			return(FALSE);
		}
	}
	return(TRUE);
}

static char isNeedParans(char *line) {
	char paran;

	paran = 0;
	for (; *line != EOS; line++) {
		if (*line == '(')
			paran++;
		else if (*line == ')')
			paran--;
		else if ((*line == '+' || *line == '^') && paran == 0)
			return(TRUE);
	}
	return(FALSE);
}
/*
ALSO was implemented back-ward - we checked to see if that code has scores, instead we should give that code credit also
static void validateDepRules(int ln, char *deps, char *rname, const char *part) {
	int b, e, t;
	RULES_LIST *rule;
	
	for (b=0; isSpace(deps[b]); b++) ;
	while (deps[b] != EOS) {
		for (e=b; deps[e] != EOS && deps[e] != ','; e++) ;
		if (deps[e] == ',' || deps[e] == EOS) {
			t = deps[e];
			deps[e] = EOS;
			if (deps[b] != EOS) {
				for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
					if (uS.mStricmp(rule->name, deps+b) == 0)
						break;
				}
				if (rule == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "Rule \"%s\" was not declared before referenced in %s element of rule \"%s\"\n", deps+b, part, rname);
					ipsyn_overflow(FALSE);
				}
			}
			if (t == EOS)
				b = e;
			else {
				deps[e] = t;
				b = e + 1;
			}
		} else
			b++;
	}
}
ALSO was implemented back-ward - we checked to see if that code has scores, instead we should give that code credit also
static IEWORDS *addDeps(IEWORDS *root, char *word) {
	IEWORDS *t;
	
	if (root == NULL) {
		t = NEW(IEWORDS);
		if (t == NULL)
			ipsyn_overflow(TRUE);
		root = t;
	} else {
		for (t=root; 1; t=t->nextword) {
			if (uS.mStricmp(t->word, word) == 0)
				return(root);
			if (t->nextword == NULL)
				break;
		}
		if ((t->nextword=NEW(IEWORDS)) == NULL)
			ipsyn_overflow(TRUE);
		t = t->nextword;
	}
	t->nextword = NULL;
	if ((t->word=(char *)malloc(strlen(word)+1)) == NULL)
		ipsyn_overflow(TRUE);
	strcpy(t->word, word);
	return(root);
}
*/
static IFLST *addIfs(int ln, char *line, int b, char *rname, char *isErr) {
	int score, nameb;
	char t, name[BUFSIZ];
	IFLST *root, *ifs;

	root = NULL;
	while (1) {
		if (!isalpha(line[b])) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected rule's name the ADD element of RULENAME \"%s\" depends on.\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			*isErr = TRUE;
			return(root);
		}
		nameb = b;
		for (; isalpha(line[b]); b++) ;
		for (; isdigit(line[b]); b++) ;
		if (line[b] != '=') {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected \"=\" character in ADD element of RULENAME: %s\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			*isErr = TRUE;
			return(root);
		}
		t = line[b];
		line[b] = EOS;
		strcpy(name, line+nameb);
		line[b] = t;
		b++;
		if (!isdigit(line[b])) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected conditional score number in ADD element of RULENAME: %s\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			*isErr = TRUE;
			return(root);
		}
		score = atoi(line+b);
		for (; isdigit(line[b]); b++) ;
		if (line[b] != ',' && line[b] != EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected \",\" character or end of line in ADD element of RULENAME: %s\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			*isErr = TRUE;
			return(root);
		}
		if (root == NULL) {
			ifs = NEW(IFLST);
			if (ifs == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr,"Error: out of memory\n");
				*isErr = TRUE;
				return(root);
			}
			root = ifs;
		} else {
			for (ifs=root; ifs->next_if != NULL; ifs=ifs->next_if) ;
			if ((ifs->next_if=NEW(IFLST)) == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr,"Error: out of memory\n");
				*isErr = TRUE;
				return(root);
			}
			ifs = ifs->next_if;
		}
		ifs->next_if = NULL;
		ifs->score = score;
		if ((ifs->name=(char *)malloc(strlen(name)+1)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			*isErr = TRUE;
			return(root);
		}
		strcpy(ifs->name, name);
		if (line[b] == EOS)
			break;
		else
			b++;
	}
	return(root);
}

static ADDLST *parseADDField(ADDLST *root, int ln, char *line, char *rname, char *isAddErr) {
	int  score, b;
	char isErr;
	ADDLST *adds;

	b = 0;
	if (!isdigit(line[b])) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    Expected score number in ADD element of RULENAME: %s\n", rname);
		fprintf(stderr, "ADD: ");
		ipsyn_err(line, b, TRUE);
		*isAddErr = TRUE;
		return(root);
	}
	score = atoi(line+b);
	for (; isdigit(line[b]); b++) ;
	if ((line[b] != 'I' && line[b] != 'i') || (line[b+1] != 'F' && line[b+1] != 'f')) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    Expected \"IF\" keyword in ADD element of RULENAME: %s\n", rname);
		fprintf(stderr, "ADD: ");
		ipsyn_err(line, b, TRUE);
		*isAddErr = TRUE;
		return(root);
	}
	b = b + 2;
	if (root == NULL) {
		adds = NEW(ADDLST);
		if (adds == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			*isAddErr = TRUE;
			return(root);
		}
		root = adds;
	} else {
		for (adds=root; adds->next_add != NULL; adds=adds->next_add) ;
		if ((adds->next_add=NEW(ADDLST)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			*isAddErr = TRUE;
			return(root);
		}
		adds = adds->next_add;
	}
	adds->next_add = NULL;
	adds->score = score;
	adds->ln = ln;
	adds->ifs = NULL;
	isErr = FALSE;
	adds->ifs = addIfs(ln, line, b, rname, &isErr);
	if (isErr) {
		*isAddErr = TRUE;
		return(root);
	}
	return(root);
}

static char addVar(int ln, char isRepeat, char *name, char *line) {
	int i;
	VARS_LIST *var;

	if (root_vars == NULL) {
		if ((var=NEW(VARS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		root_vars = var;
	} else {
		for (var=root_vars; 1; var=var->next_var) {
			if (uS.mStricmp(var->name, name) == 0) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Variable by name \"%s\" has already been declared\n", name);
				return(FALSE);
			}
			if (var->next_var == NULL)
				break;
		}
		if ((var->next_var=NEW(VARS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		var = var->next_var;
	}
	var->next_var = NULL;
	var->isRepeat = isRepeat;
	var->name = NULL;
	var->line = NULL;
	if ((var->name=(char *)malloc(strlen(name)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr,"Error: out of memory\n");
		return(FALSE);
	}
	strcpy(var->name, name);
	if (strchr(line, '+') != NULL || strchr(line, '^') != NULL) {
		if (isNeedParans(line)) {
			strcpy(templineC2, "(");
			strcat(templineC2, line);
			strcat(templineC2, ")");
		} else
			strcpy(templineC2, line);
	} else {
		strcpy(templineC2, line);
		i = 0;
		while (templineC2[i] != EOS) {
			if (templineC2[i] == '(' || templineC2[i] == ')')
				strcpy(templineC2+i, templineC2+i+1);
			else
				i++;
		}
	}
	if ((var->line=(char *)malloc(strlen(templineC2)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr,"Error: out of memory\n");
		return(FALSE);
	}
	strcpy(var->line, templineC2);
	return(TRUE);
}

static char addRuleSums(int ln, char *name, char *line, char isSum) {
	int  b, e;
	char t;
	IEWORDS *pat;
	RULESUMS_LIST *p;

	if (root_rsum == NULL) {
		if ((p=NEW(RULESUMS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		root_rsum = p;
	} else {
		for (p=root_rsum; 1; p=p->next_rsum) {
			if (uS.mStricmp(p->name, name) == 0) {
				if (onlydata != 2 || p->isSum == TRUE) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Rule by name \"%s\" has already been declared\n", name);
					return(FALSE);
				} else {
					p->isSum = isSum;
					return(TRUE);
				}
			}
			if (p->next_rsum == NULL)
				break;
		}
		if ((p->next_rsum=NEW(RULESUMS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		p = p->next_rsum;
	}
	p->next_rsum = NULL;
	p->name = NULL;
	p->isSum = FALSE;
	p->pats = NULL;
	p->scoreSum = 0;
	if ((p->name=(char *)malloc(strlen(name)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr,"Error: out of memory\n");
		return(FALSE);
	}
	strcpy(p->name, name);
	for (b=0; isSpace(line[b]) || line[b] == ',' || line[b] == '+'; b++) ;
	while (line[b] != EOS) {
		for (e=b; line[e] != EOS && !isSpace(line[e]) && line[e] != ',' && line[e] != '+'; e++) ;
		t = line[e];
		line[e] = EOS;
		if ((pat=NEW(IEWORDS)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			return(FALSE);
		}
		pat->word = (char *)malloc(strlen(line+b)+1);
		if (pat->word == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr,"Error: out of memory\n");
			free(pat);
			return(FALSE);
		}
		strcpy(pat->word, line+b);
		pat->nextword = p->pats;
		p->pats = pat;
		line[e] = t;
		for (b=e; isSpace(line[b]) || line[b] == ',' || line[b] == '+'; b++) ;
		p->isSum = isSum;
	}
	return(TRUE);
}

static RULES_LIST *createNewRule(int ln, char *full_name, char *isErr) {
	int  i;
	char name[512], *info;
	RULES_LIST *rule;

	for (i=0; !isSpace(full_name[i]) && full_name[i] != '-' && full_name[i] != EOS; i++) {
		name[i] = full_name[i];
	}
	name[i] = EOS;
	for (; isSpace(full_name[i]) || full_name[i] == '-'; i++) ;
	info = full_name + i;
	if (root_rules == NULL) {
		if ((rule=NEW(RULES_LIST)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			*isErr = TRUE;
			return(rule);
		}
		root_rules = rule;
	} else {
		for (rule=root_rules; 1; rule=rule->next_rule) {
			if (uS.mStricmp(rule->name, name) == 0) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr,"RULENAME \"%s\" has already been declared\n", name);
				*isErr = TRUE;
				return(rule);
			}
			if (rule->next_rule == NULL)
				break;
		}
		if ((rule->next_rule=NEW(RULES_LIST)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			*isErr = TRUE;
			return(rule);
		}
		rule = rule->next_rule;
	}
	rule->next_rule = NULL;
	rule->isTrueAnd = isAndSymFound;
	rule->adds = NULL;
	rule->cond = NULL;
	rule->info = NULL;
	if ((rule->name=(char *)malloc(strlen(name)+1)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		*isErr = TRUE;
		return(rule);
	}
	strcpy(rule->name, name);
	if ((rule->info=(char *)malloc(strlen(info)+1)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		*isErr = TRUE;
		return(rule);
	}
	strcpy(rule->info, info);
	return(rule);
}

static RULES_COND *createNewCond(RULES_LIST *rule) {
	RULES_COND *cond;
	
	if (rule->cond == NULL) {
		if ((cond=NEW(RULES_COND)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		rule->cond = cond;
	} else {
		for (cond=rule->cond; cond->next_cond != NULL; cond=cond->next_cond) ;
		if ((cond->next_cond=NEW(RULES_COND)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			return(NULL);
		}
		cond = cond->next_cond;
	}
	cond->next_cond = NULL;
	cond->point = 0;
	cond->diffCnt = 0L;
	cond->isDiffSpec = 0;
	cond->deps = NULL;
	cond->include = NULL;
	cond->exclude = NULL;
	return(cond);
}

static void findAllNegFlats(PAT_ELEMS *cur, char *isNeg) {
	char tNeg;

	if (cur->parenElem != NULL) {
		tNeg = TRUE;
		findAllNegFlats(cur->parenElem, &tNeg);
		if (cur->neg)
			tNeg = TRUE;
		if (!tNeg)
			*isNeg = FALSE;
	} else if (cur->pat_elem != NULL) {
		if (!cur->neg && cur->pat_elem->pat_str[0] != EOS && strcmp(cur->pat_elem->pat_str, "_")) {
			*isNeg = FALSE;
		}
	}
	if (cur->andElem != NULL) {
		findAllNegFlats(cur->andElem, isNeg);
	}
	cur->isAllNeg = *isNeg;
	if (cur->orElem != NULL) {
		*isNeg = TRUE;
		findAllNegFlats(cur->orElem, isNeg);
	}	
}

static char addPatElems(int ln, const char *op, RULES_LIST *rule, RULES_COND *cond, char *line) {
	char isNeg;

	if (op[0] == 'i') {
		if (cond->include != NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Only one INCLUDE element allowed in \"if\" part of RULENAME \"%s\".\n", rule->name);
			return(FALSE);
		}
	} else if (op[0] == 'e') {
		if (cond->exclude != NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Only one EXCLUDE element allowed in \"if\" part of RULENAME \"%s\".\n", rule->name);
			return(FALSE);
		}
	}
	tempPat.origmac = mkPatElem(rule->isTrueAnd);
	if (tempPat.origmac == NULL) {
		return(FALSE);
	}
	if (makePatElems(ln,rule->name,op,line,0L,tempPat.origmac,rule->isTrueAnd) == -1L) {
		return(FALSE);
	}
	tempPat.duplicatesList = NULL;
	tempPat.stackAndsI = 0;
	tempPat.flatmac = mkPatElem(rule->isTrueAnd);
	if (tempPat.flatmac == NULL) {
		return(FALSE);
	}
	if (!flatten_expr(tempPat.origmac, rule->isTrueAnd, TRUE)) {
		return(FALSE);
	}
	if (!removeDuplicateFlatMacs(FALSE)) {
		return(FALSE);
	}
	tempPat.duplicatesList = freeIEWORDS(tempPat.duplicatesList);
	isNeg = TRUE;
	findAllNegFlats(tempPat.flatmac, &isNeg);
	if (op[0] == 'i')
		cond->include = tempPat.flatmac;
	else if (op[0] == 'e')
		cond->exclude = tempPat.flatmac;
	else {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    INTERNAL ERROR \"op\" variable can only be \"include\" or \"exclude\".\n");
		return(FALSE);
	}
	tempPat.flatmac = NULL;
	freeIpsynElemPats(tempPat.origmac, TRUE);
	tempPat.origmac = NULL;
	return(TRUE);
}

static RULES_COND *isRuleDeclErr(int ln, char *name, char *line, RULES_LIST *rule, RULES_COND *cond, char *isErr) {
	if (rule == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    No rule name declared up to this line: %s %s\n", name, line);
		*isErr = TRUE;
		return(cond);
	}
	if (cond == NULL) {
		cond = createNewCond(rule);
		if (cond == NULL) {
			*isErr = TRUE;
			return(cond);
		}
	}
	return(cond);
}

static void assignDiffElems(RULES_COND *cond, char *line) {
	int  i;
	char *b, *e;
	unsigned long t;

	b = line;
	while (*b != EOS) {
		e = strchr(b, ',');
		if (e != NULL)
			*e = EOS;
		while (isSpace(*b))
			b++;
		uS.remblanks(b);
		i = atoi(b) - 1;
		if (i >= 0 && i < 32) {
			t = 1;
			if (i > 0)
				t = t << i;
			cond->diffCnt = (cond->diffCnt | t);
		}
		if (e == NULL)
			break;
		b = e + 1;
	}
}

static char isDiffStemPosText(char *line) {
	while (isSpace(*line) || isdigit(*line) || *line == ',' || *line == '\n')
		line++;
	if (*line == EOS)
		return(TRUE);
	else
		return(FALSE);
}

static char parseRules(int ln, TEMP_RULE *tr) {
	char *name, *line, isRepeat, isVarFound, isErr;

	if (templineC1[0] == '^')
		isAndSymFound = TRUE;
	else if (templineC1[0] == '>')
		isAndSymFound = FALSE;
	else if (templineC1[0] == '+') {
		for (line=templineC1+1; isSpace(*line); line++) ;
		name = line;
		for (; !isSpace(*line) && *line != '=' && *line != EOS; line++) ;
		if (*line == EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    rules sum is missing \"=\" rules search patterns\n");
			return(FALSE);
		}
		*line = EOS;
		for (line++; isSpace(*line); line++) ;
		if (*line == '=') {
			for (line++; isSpace(*line); line++) ;
		}
		if (!addRuleSums(ln, name, line, TRUE)) {
			return(FALSE);
		}
	} else {
		isRepeat = FALSE;
		isVarFound = FALSE;
		name = templineC1;
		for (line=templineC1; !isSpace(*line) && *line != ':' && *line != EOS; line++) ;
		if (*line == ':')
			line++;
		if (*line == EOS) {
			if (uS.mStricmp(name, "if") != 0) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Illegal element \"%s\" and unexpected end of line found\n", templineC1);
				return(FALSE);
			}
		} else {
			*line = EOS;
			for (line++; isSpace(*line); line++) ;
			if (*line == '+' && *(line+1) == '=') {
				isRepeat = TRUE;
				isVarFound = TRUE;
				for (line=line+2; isSpace(*line); line++) ;
			} else if (*line == '=') {
				isVarFound = TRUE;
				for (line++; isSpace(*line); line++) ;
			}
		}
		if (isVarFound) {
			removeAllSpaces(line);
			if (!addVar(ln, isRepeat, name, line)) {
				return(FALSE);
			}
		} else {
			if (uS.mStricmp(name, "RULENAME:") != 0)
				removeAllSpaces(line);
			if (uS.mStricmp(name, "RULENAME:") == 0) {
				isErr = FALSE;
				tr->cond = NULL;
				tr->rule = createNewRule(ln, line, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (onlydata == 2) {
					if (!addRuleSums(ln, line, line, FALSE)) {
						return(FALSE);
					}
				}
			} else if (uS.mStricmp(name, "ADD:") == 0) {
				if (tr->rule == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    No rule name declared up to this line: %s %s\n", name, line);
					return(FALSE);
				}
				isErr = FALSE;
				tr->rule->adds = parseADDField(tr->rule->adds, ln, line, tr->rule->name, &isErr);
				if (isErr) {
					return(FALSE);
				}
			} else if (uS.mStricmp(name, "if") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, NULL, &isErr);
				if (isErr) {
					return(FALSE);
				}
			} else if (uS.mStricmp(name, "INCLUDE:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (!addPatElems(ln, "include", tr->rule, tr->cond, line)) {
					return(FALSE);
				}
			} else if (uS.mStricmp(name, "EXCLUDE:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (!addPatElems(ln, "exclude", tr->rule, tr->cond, line)) {
					return(FALSE);
				}
/* ALSO was implemented back-ward - we checked to see if that code has scores, instead we should give that code credit also
			} else if (uS.mStricmp(name, "ALSO:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				validateDepRules(ln, line, tr->rule->name, "ALSO");
				tr->cond->deps = addDeps(tr->cond->deps, line);
*/
			} else if (uS.mStricmp(name, "POINT:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (tr->cond->point != 0) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Only one POINT element allowed in \"if\" part of RULENAME \"%s\".\n", tr->rule->name);
					return(FALSE);
				}
				if (line[0] == '1' && line[1] == EOS)
					tr->cond->point = 1;
				else if (line[0] == '2' && line[1] == EOS)
					tr->cond->point = 2;
				else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    POINT: field can only have \"1\" or \"2\", but not %s %s\n", name, line);
					return(FALSE);
				}
			} else if (uS.mStricmp(name, "DIFFERENT_STEMS_POS:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (tr->cond->isDiffSpec != 0) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Only one DIFFERENT_STEMS or DIFFERENT_STEMS_POS element allowed in \"if\" part of RULENAME \"%s\".\n", tr->rule->name);
					return(FALSE);
				}
				tr->cond->isDiffSpec = 1;
				if (isDiffStemPosText(line)) {
					tr->cond->diffCnt = 0L;
					assignDiffElems(tr->cond, line);
					if (tr->cond->diffCnt == 0L) {
						fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
						fprintf(stderr, "    Malformed DIFFERENT_STEMS_POS element in RULENAME \"%s\".\n", tr->rule->name);
						return(FALSE);
					}
				} else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    DIFFERENT_STEMS_POS: illegal text in field: %s %s\n", name, line);
					return(FALSE);
				}
			} else if (uS.mStricmp(name, "DIFFERENT_STEMS:") == 0) {
				isErr = FALSE;
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond, &isErr);
				if (isErr) {
					return(FALSE);
				}
				if (tr->cond->isDiffSpec != 0) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Only one DIFFERENT_STEMS or DIFFERENT_STEMS_POS element allowed in \"if\" part of RULENAME \"%s\".\n", tr->rule->name);
					return(FALSE);
				}
				tr->cond->isDiffSpec = 2;
				if (line[0] == '1' && line[1] == EOS)
					tr->cond->diffCnt = 1L;
				else if (line[0] == '>' && isdigit(line[1]))
					tr->cond->diffCnt = atol(line+1);
				else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    DIFFERENT_STEMS: illegal text in field: %s %s\n", name, line);
					return(FALSE);
				}
			} else {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Unknown element \"%s\" found\n", name);
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

static char read_ipsyn(char *lang) {
	int  i, ln, cln, len;
	char *c;
	FILE *fp;
	TEMP_RULE tRule;

	if (*lang == EOS) {
		fprintf(stderr,	"No ipsyn rules specified with +l option.\n");
		fprintf(stderr,"Please specify ipsyn rules file name with \"+l\" option.\n");
		if (is100Limit)
			fprintf(stderr,"For example, \"ipsyn +leng-100\" or \"ipsyn +leng-100.cut\".\n");
		else
			fprintf(stderr,"For example, \"ipsyn +leng\" or \"ipsyn +leng.cut\".\n");
		return(FALSE);
	}
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, "ipsyn");
	addFilename2Path(FileName1, lang);
	if ((c=strchr(lang, '.')) != NULL) {
		if (uS.mStricmp(c, ".cut") != 0)
			strcat(FileName1, ".cut");
	} else
		strcat(FileName1, ".cut");
	if ((fp=fopen(FileName1,"r")) == NULL) {
		if (c != NULL) {
			if (uS.mStricmp(c, ".cut") == 0) {
				strcpy(templineC, wd_dir);
				addFilename2Path(templineC, lang);
				if ((fp=fopen(templineC,"r")) != NULL) {
					strcpy(FileName1, templineC);
				}
			}
		}
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate ipsyn rules file: \"%s\".\n", FileName1);
#ifdef _MAC_CODE
		fprintf(stderr, "\n");
#else
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
#endif
		return(FALSE);
	}
	fprintf(stderr,"    Using IPSyn rules file: %s\n", FileName1);
	tRule.rule = NULL;
	tRule.cond = NULL;
	ln = 0;
	cln = ln;
	isAndSymFound = FALSE;
	templineC1[0] = EOS;
	len = 0;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		ln++;		
		if (templineC[0] == '%')
			continue;
		c = strchr(templineC, '%');
		if (c != NULL)
			*c = EOS;
		for (i=0; isSpace(templineC[i]) || templineC[i] == '\n'; i++) ;
		if (i > 0)
			strcpy(templineC, templineC+i);
		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;
		if (templineC1[len] == '\\') {
			templineC1[len] = ' ';
			strcat(templineC1, templineC);
			len = strlen(templineC1) - 1;
		} else {
			if (templineC1[0] != EOS) {
				if (!parseRules(cln, &tRule)) {
					fclose(fp);
					root_vars = freeVars(root_vars);
					root_rsum = freeRSums(root_rsum);
					return(FALSE);
				}
			}
			cln = ln;
			strcpy(templineC1, templineC);
			len = strlen(templineC1) - 1;
		}
	}
	if (templineC1[0] != EOS) {
		if (!parseRules(cln, &tRule)) {
			fclose(fp);
			root_vars = freeVars(root_vars);
			root_rsum = freeRSums(root_rsum);
			return(FALSE);
		}
	}
	fclose(fp);
	root_vars = freeVars(root_vars);
	if (root_rules == NULL) {
		fprintf(stderr,"Can't find any usable declarations in ipsyn rules file \"%s\".\n", FileName1);
		return(FALSE);
	}
	return(TRUE);
}
// END reading rules

char init_ipsyn(char first, int limit) {
	int  i;
	char rulesPunctuation[50];
	const char *tPunctuation;

	if (first) {
		ipsyn_lang[0] = EOS;
		fileID = 0;
		if (IPS_UTTLIM != limit)
			IPS_UTTLIM = limit; // 2019-08-26 50;
		ipsyn_ftime = TRUE;
		isAndSymFound = FALSE;
		isIPpostcode = TRUE;
		isIPEpostcode = TRUE;
		root_rules = NULL;
		root_pats = NULL;
		root_vars = NULL;
		root_rsum = NULL;
		root_sp = NULL;
		tempPat.duplicatesList = NULL;
		tempPat.origmac = NULL;
		tempPat.flatmac = NULL;
		tfp = NULL;
		strcpy(fDepTierName, "%mor:");
	} else {
		if (ipsyn_lang[0] == EOS) {
			if (CLAN_PROG_NUM == IPSYN) {
				fprintf(stderr,"Please specify ipsyn rules file name with \"+l\" option.\n");
				if (is100Limit)
					fprintf(stderr,"For example, \"ipsyn +leng-100\" or \"ipsyn +leng-100.cut\".\n");
				else
					fprintf(stderr,"For example, \"ipsyn +leng\" or \"ipsyn +leng.cut\".\n");
			} else {
				fprintf(stderr, "Please specify IPSYN's rules file name inside \"language script file\".\n");
				fprintf(stderr, "  0 - do not compute IPSYN\n");
			}
			return(FALSE);
		}
		if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
			tPunctuation = punctuation;
			strcpy(rulesPunctuation, punctuation);
			for (i=0; rulesPunctuation[i] != EOS;) {
				if (rulesPunctuation[i] == ';')
					strcpy(rulesPunctuation+i,rulesPunctuation+i+1);
				else
					i++;
			}
			punctuation = rulesPunctuation;
			if (!read_ipsyn(ipsyn_lang)) {
				punctuation = tPunctuation;
				return(FALSE);
			}
			punctuation = tPunctuation;
		}
	}
	return(TRUE);
}

#ifndef KIDEVAL_LIB
void init(char first) {
	int  i;
	char *s;

	if (first) {
		stout = FALSE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
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
//		mor_initwords();
		if (!init_ipsyn(first, IPS_UTTLIM)) {
			ipsyn_overflow();
			cutt_exit(0);
		}
	} else {
		if (ipsyn_ftime) {
			ipsyn_ftime = FALSE;
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.')
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
			if (is100Limit) {
				strcat(GExt, "-100");
				s = strrchr(ipsyn_lang, PATHDELIMCHR);
				if (s == NULL)
					s = ipsyn_lang;
				else
					s++;
				if (uS.mStricmp(s, "eng") == 0)
					strcat(ipsyn_lang, "-100");
			}
			if (!init_ipsyn(first, IPS_UTTLIM)) {
				ipsyn_overflow();
				cutt_exit(0);
			}
			maketierchoice(fDepTierName,'+',FALSE);
			maketierchoice("%gra:",'+',FALSE);
			if (onlydata == 2) {
				AddCEXExtension = ".xls";
				combinput = TRUE;
			}
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	char *s;

	f++;
	switch(*f++) {
		case 'c':
/*
			if (*f == EOS) {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			} else
*/
			if ((IPS_UTTLIM=atoi(f)) <= 0) {
				fprintf(stderr, "The argument number for +c has to be greater than 0\n");
				cutt_exit(0);
			}
			break;
		case 'd':
			onlydata = (char)(atoi(getfarg(f,f1,i))+1);
			if (onlydata < 0 || onlydata > OnlydataLimit) {
				fprintf(stderr, "The only +d levels allowed are 0-%d.\n", OnlydataLimit-1);
				cutt_exit(0);
			}
			break;
		case 'l':
			strncpy(ipsyn_lang, f, 253);
			ipsyn_lang[253] = EOS;
			break;
#ifdef UNX
		case 'L':
			int  j;

			strcpy(lib_dir, f);
			j = strlen(lib_dir);
			if (j > 0 && lib_dir[j-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 'o':
			no_arg_option(f);
			break;
		case 't':
			if (*f == '%')
				strcpy(fDepTierName, getfarg(f,f1,i));
			else
				maingetflag(f-2,f1,i);
			break;
		case 's':
			s = f;
			if (*s == '+' || *s == '~')
				s++;
			if (*s == '[' || *s == '<') {
				s++;
				if (*s == '+') {
					for (s++; isSpace(*s); s++) ;
					if ((*s     == 'i' || *s     == 'I') && 
						(*(s+1) == 'p' || *(s+1) == 'P') && 
						(*(s+2) == ']' || *(s+2) == '>' )) {
						isIPpostcode = FALSE;
					}
					if ((*s     == 'i' || *s     == 'I') && 
						(*(s+1) == 'p' || *(s+1) == 'P') && 
						(*(s+2) == 'e' || *(s+2) == 'E') && 
						(*(s+3) == ']' || *(s+3) == '>' )) {
						isIPEpostcode = FALSE;
					}
				}
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
#endif // KIDEVAL_LIB
