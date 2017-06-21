/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
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
extern char morTierName[];

static int  fileID;
static int  IPS_UTTLIM;
static char ipsyn_ftime, isAndSymFound, isVerb;
static char isIPpostcode, isIPEpostcode;
static SPEAKERS_LIST *root_sp;
static RULES_LIST *root_rules;
static PATS_LIST *root_pats;
static VARS_LIST  *root_vars;
static TEMPPAT tempPat;
static FILE *tfp;

#ifndef KIDEVAL_LIB
void usage() {
	printf("Creates coding tier %%syn: with Syntactic codings derived from existing data\n");
	printf("Usage: syncoding [cN d lF sS %s] filename(s)\n", mainflgs());
	printf("+cN: analyse N complete unique utterances. (default: 100 utterances)\n");
	puts("+d : show file and line number where points are found.");
	puts("+lF: specify ipsyn rules file name F");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+sS: specify [+ ip] or [+ ipe] to ignore those postcodes.");
	puts("-sS: specify [+ ip] or [+ ipe] to ignore those postcodes");
	mainusage(TRUE);
}
#endif // KIDEVAL_LIB

static void freeUpAllFeats(MORFEATS *p) {
	MORFEATS *t;

	while (p != NULL) {
		t = p;
		p = p->comp;
		if (t->free_prefix)
			free(t->prefix);
		if (t->free_suffix0)
			free(t->suffix0);
		if (t->free_suffix1)
			free(t->suffix1);
		if (t->free_suffix2)
			free(t->suffix2);
		if (t->free_fusion0)
			free(t->fusion0);
		if (t->free_fusion1)
			free(t->fusion1);
		if (t->free_fusion2)
			free(t->fusion2);
		if (t->free_trans)
			free(t->trans);
		if (t->free_repls)
			free(t->repls);
		if (t->free_error0)
			free(t->error0);
		if (t->free_error1)
			free(t->error1);
		if (t->free_error2)
			free(t->error2);
		free(t);
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
			if (t->mor_word != NULL)
				free(t->mor_word);
			freeUpAllFeats(t->word_feats);
		}
		free(t);
	}
}

static void freeUtts(UTTS_LIST *p) {
	UTTS_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_utt;
		if (t->spkLine != NULL)
			free(t->spkLine);
		if (t->morLine != NULL)
			free(t->morLine);
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

static SPEAKERS_LIST *freeSpeakers(SPEAKERS_LIST *p) {
	SPEAKERS_LIST *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_sp;
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

static SPEAKERS_LIST *freeSpkLines(SPEAKERS_LIST *p) {
	SPEAKERS_LIST *t;
	UTTS_LIST *utt;

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

static PATS_LIST *freePats(PATS_LIST *p) {
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

static void freePats(PAT_ELEMS *p, char isRemoveOR) {
	if (p == NULL)
		return;
	if (p->parenElem != NULL)
		freePats(p->parenElem, isRemoveOR);
	if (p->andElem != NULL)
		freePats(p->andElem, isRemoveOR);
	if (isRemoveOR && p->orElem != NULL)
		freePats(p->orElem, isRemoveOR);
	free(p);
}

static void freeRulesConds(RULES_COND *p) {
	RULES_COND *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_cond;
		freeIEWORDS(t->deps);
		freePats(t->exclude, TRUE);
		freePats(t->include, TRUE);
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

static RULES_LIST *freeRules(RULES_LIST *p) {
	RULES_LIST *t;

	while (p != NULL) {
		t = p;
		p = p->next_rule;
		if (t->name != NULL)
			free(t->name);
		freeAdds(t->adds);
		freeRulesConds(t->cond);
		free(t);
	}
	return(NULL);
}

static void ipsyn_overflow(char isOutOfMem) {
	if (isOutOfMem)
		fprintf(stderr,"Error: out of memory\n");
	root_sp = freeSpeakers(root_sp);
	root_rules = freeRules(root_rules);
	root_pats = freePats(root_pats);
	root_vars = freeVars(root_vars);
	tempPat.duplicatesList = freeIEWORDS(tempPat.duplicatesList);
	freePats(tempPat.origmac, TRUE);
	freePats(tempPat.flatmac, TRUE);
	if (tfp != NULL) {
		fclose(tfp);
		tfp = NULL;
	}
	cutt_exit(0);
}

char init_ipsyn(char first) {
	if (first) {
		fileID = 0;
		IPS_UTTLIM = 100;
		isVerb = FALSE;
		ipsyn_ftime = TRUE;
		isAndSymFound = FALSE;
		isIPpostcode = TRUE;
		isIPEpostcode = TRUE;
		root_rules = NULL;
		root_pats = NULL;
		root_vars = NULL;
		root_sp = NULL;
		tempPat.duplicatesList = NULL;
		tempPat.origmac = NULL;
		tempPat.flatmac = NULL;
		tfp = NULL;
		strcpy(morTierName, "%mor:");
	} else {
		if (root_rules == NULL) {
			if (CLAN_PROG_NUM == IPSYN) {
				fprintf(stderr,"Please specify ipsyn rules file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"ipsyn +leng\" or \"ipsyn +leng.cut\".\n");
			} else {
				fprintf(stderr, "Please specify IPSYN's rules file name inside \"language script file\".\n");
			}
			return(FALSE);
		}
	}
	return(TRUE);
}

#ifndef KIDEVAL_LIB
void init(char first) {
	int i;

	if (first) {
		stout = FALSE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
//		onlydata = 1;
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
		init_ipsyn(first);
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
			if (!init_ipsyn(first))
				cutt_exit(0);
			maketierchoice(morTierName,'+',FALSE);
		}
	}
}
#endif // KIDEVAL_LIB

static UTT_WORDS *addUtt_words(UTT_WORDS *words, char *mor_word, char *surf_word, char isClitic, char isLastStop) {
	int num, len;
	UTT_WORDS *utt_word;

	num = 1;
	if (words == NULL) {
		utt_word = NEW(UTT_WORDS);
		if (utt_word == NULL)
			ipsyn_overflow(TRUE);
		words = utt_word;
	} else {
		for (utt_word=words; utt_word->next_word != NULL; utt_word=utt_word->next_word) 
			num++;
		if ((utt_word->next_word=NEW(UTT_WORDS)) == NULL)
			ipsyn_overflow(TRUE);
		num++;
		utt_word = utt_word->next_word;
	}
	utt_word->next_word = NULL;
	utt_word->isWordMatched = FALSE;
	utt_word->isClitic = isClitic;
	utt_word->surf_word = NULL;
	utt_word->oMor_word = NULL;
	utt_word->mor_word = NULL;
	utt_word->word_feats = NULL;
	utt_word->num = num;
	utt_word->isLastStop = isLastStop;
	if (isLastStop == FALSE) {
		if ((utt_word->surf_word=(char *)malloc(strlen(surf_word)+1)) == NULL)
			ipsyn_overflow(TRUE);
		strcpy(utt_word->surf_word, surf_word);
		len = strlen(mor_word) + 1;
		if ((utt_word->oMor_word=(char *)malloc(len)) == NULL)
			ipsyn_overflow(TRUE);
		strcpy(utt_word->oMor_word, mor_word);
		if ((utt_word->mor_word=(char *)malloc(len)) == NULL)
			ipsyn_overflow(TRUE);
		strcpy(utt_word->mor_word, mor_word);
		if (strchr(mor_word, '|') != NULL) {
			if ((utt_word->word_feats=NEW(MORFEATS)) == NULL)
				ipsyn_overflow(TRUE);
			utt_word->word_feats->type = NULL;
			utt_word->word_feats->typeID = 'R';
			if (ParseWordIntoFeatures(utt_word->mor_word, utt_word->word_feats) == FALSE)
				ipsyn_overflow(TRUE);
		}
	}
	return(words);
}

static void createWordFeats(UTTS_LIST *utt) {
	int  i;
	char *st, *cmor_word, mor_word[BUFSIZ], surf_word[BUFSIZ], isClitic;

	for (; utt != NULL; utt=utt->next_utt) {
		if (utt->morLine != NULL) {
			i = 0;
			while ((i=getNextDepTierPair(utt->morLine, mor_word, surf_word, NULL, i)) != 0) {
				uS.remblanks(mor_word);
				uS.remblanks(surf_word);
				if (mor_word[0] != '[' && mor_word[0] != EOS && surf_word[0] != EOS) {
					cmor_word = mor_word;
					isClitic = FALSE;
					do {
						st = strchr(cmor_word, '~');
						if (st != NULL)
							*st = EOS;
						if (cmor_word != mor_word) {
							isClitic = TRUE;
							strcpy(surf_word, "");
						}
						utt->words = addUtt_words(utt->words, cmor_word, surf_word, isClitic, FALSE);
						if (st != NULL)
							cmor_word = st + 1;
					} while (st != NULL) ;
				}
			}
			utt->words = addUtt_words(utt->words, NULL, NULL, isClitic, TRUE); // adding DUMMY element
		}
	}
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

static char isPatMatch(PATS_LIST *pat_elem, UTT_WORDS *utt_word) {
	if (pat_elem->whatType == SPK_TIER) {
		if (uS.patmat(utt_word->surf_word, pat_elem->pat_str))
			return(TRUE);
	} else if (pat_elem->whatType == MOR_TIER) {
		if (pat_elem->isClitic == TRUE && utt_word->isClitic == FALSE)
			return(FALSE);
		else if (matchToMorFeatures(pat_elem->pat_feat, utt_word->word_feats, TRUE, FALSE))
			return(TRUE);
	} else if (pat_elem->whatType == GRA_TIER) {
	}
	return(FALSE);
}

static UTT_WORDS *CMatch(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, TEMPMATCH *tempMatch) {
	char isRepeat;
	PATS_LIST *pat_elem;

	isRepeat = FALSE;
	pat_elem = tm->pat_elem;
	if (tm->isRepeat) {
		if (pat_elem != findNextPat_elem(NULL, tm))
			isRepeat = TRUE;
	}
	if (strcmp(pat_elem->pat_str, "_") == 0) {
		tempMatch->firstUtt_word = utt_word;
		return(utt_word->next_word);
	} else {
		if (pat_elem->whatType != MOR_TIER) {
			while (utt_word != NULL && utt_word->isLastStop == FALSE && utt_word->isClitic == TRUE)
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
					while (utt_word != NULL && utt_word->isLastStop == FALSE && utt_word->isClitic == TRUE)
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

static UTT_WORDS *match(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, char *isStillMatched, char isTrueAnd, TEMPMATCH *tempMatch) {
	char isNegFound;
	UTT_WORDS *last, *tmp, *negSf, *negSft, *negSl, *tUtt_word;

	if (utt_word->isLastStop == TRUE)
		return(NULL);
	isNegFound = FALSE;
	negSf = NULL;
	negSl = NULL;
	if (tm->pat_elem != NULL) {
		if (utt_word->isWordMatched)
			return(NULL);
		else if (tm->pat_elem->pat_str[0] == EOS) {
		} else if ((last=CMatch(utt_word,tm,wild,tempMatch)) != NULL) {
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
		if ((tmp=match(utt_word,tm->parenElem,wild,isStillMatched,isTrueAnd,tempMatch)) != NULL) {
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
				utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,tempMatch);
				if (wild && utt_word == NULL && tUtt_word->num <= negSf->num) {
					utt_word = tUtt_word;
					tmp = utt_word;
					while (utt_word != NULL && utt_word->num <= negSf->num) {
						utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,tempMatch);
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
				utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,tempMatch);
				if (tm->neg && isNegFound && utt_word != NULL)
					*isStillMatched = FALSE;
			}
			negSl = NULL;
		} else {
			utt_word = match(utt_word,tm->andElem,tm->wild,isStillMatched,isTrueAnd,tempMatch);
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

static UTT_WORDS *negMatch(UTT_WORDS *utt_word, PAT_ELEMS *tm, char wild, char isTrueAnd, TEMPMATCH *tempMatch) {
	UTT_WORDS *last, *tmp;

	if (utt_word->isLastStop == TRUE)
		return(NULL);
	if (tm->pat_elem != NULL) {
		if (utt_word->isWordMatched)
			return(NULL);
		else if (tm->pat_elem->pat_str[0] == EOS) {
		} else if ((last=CMatch(utt_word,tm,wild,tempMatch)) != NULL) {
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
		if ((tmp=negMatch(utt_word,tm->parenElem,wild,isTrueAnd,tempMatch)) != NULL) {
			utt_word = tmp;
		} else {
			return(NULL);
		}
	}
	if (tm->andElem != NULL) {
		utt_word = negMatch(utt_word,tm->andElem,tm->wild,isTrueAnd,tempMatch);
	}
	return(utt_word);
}

static UTT_WORDS *copyUtt_word(UTT_WORDS *root, UTT_WORDS *utt_word) {
	UTT_WORDS *t;

	if (root == NULL) {
		t = NEW(UTT_WORDS);
		if (t == NULL)
			ipsyn_overflow(TRUE);
		root = t;
	} else {
		for (t=root; t->next_word != NULL; t=t->next_word) ;
		if ((t->next_word=NEW(UTT_WORDS)) == NULL)
			ipsyn_overflow(TRUE);
		t = t->next_word;
	}
	t->num = utt_word->num;
	t->isLastStop = utt_word->isLastStop;
	t->isWordMatched = utt_word->isWordMatched;
	t->isClitic = utt_word->isClitic;
	t->surf_word = utt_word->surf_word;
	t->oMor_word = utt_word->oMor_word;
	t->mor_word = utt_word->mor_word;
	t->word_feats = utt_word->word_feats;
	t->next_word = NULL;
	return(root);
}

static RES_RULES *addResRuleToSp(SPEAKERS_LIST *sp, RULES_LIST *rule, UTT_WORDS *utt_word, int point) {
	RES_RULES *t;

	if (sp->resRules == NULL) {
		t = NEW(RES_RULES);
		if (t == NULL)
			ipsyn_overflow(TRUE);
		sp->resRules = t;
	} else {
		for (t=sp->resRules; 1; t=t->next_result) {
			if (t->rule == rule) {
				if (point == 1 && t->pointsFound == 0) {
					t->cntStem1++;
					t->point1 = copyUtt_word(t->point1, utt_word);
				} else if (point == 2 && t->pointsFound == 1) {
					t->cntStem2++;
					t->point2 = copyUtt_word(t->point2, utt_word);
				}
				return(t);
			}
			if (t->next_result == NULL)
				break;
		}
		if ((t->next_result=NEW(RES_RULES)) == NULL)
			ipsyn_overflow(TRUE);
		t = t->next_result;
	}
	t->next_result = NULL;
	t->rule = rule;
	t->pointsFound = 0;
	t->cntStem1 = 1;
	t->point1 = NULL;
	t->cntStem2 = 0;
	t->point2 = NULL;
	t->point1 = copyUtt_word(t->point1, utt_word);
	return(t);
}

static void getStem(UTT_WORDS *utt_word, char *str) {
	MORFEATS *word_feat;

	str[0] = EOS;
	if (utt_word->word_feats == NULL)
		strcat(str, utt_word->oMor_word);
	else {
		for (word_feat=utt_word->word_feats; word_feat != NULL; word_feat=word_feat->comp) {
			if (word_feat->typeID == '+')
				strcat(str, "+");
			if (word_feat->stem != NULL)
				strcat(str, word_feat->stem);
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

static RES_RULES *setmat(RES_RULES *res, PAT_ELEMS *cur, SPEAKERS_LIST *sp, RULES_LIST *rule, int point) {
	if (cur->parenElem != NULL) {
		if (!cur->neg)
			res = setmat(res, cur->parenElem, sp, rule, point);
	} else if (cur->pat_elem != NULL) {
		if (cur->pat_elem->pat_str[0] != EOS && !cur->neg && cur->matchedWord != NULL) {
			res = addResRuleToSp(sp, rule, cur->matchedWord, point);
		}
	}
	if (cur->andElem != NULL) {
		res = setmat(res, cur->andElem, sp, rule, point);
	}
	return(res);
}

static void addDummyToSpResRules(RES_RULES *resRules, int point, long ln) {
	if (point == 1) {
		resRules->ln1 = ln;
		resRules->point1 = addUtt_words(resRules->point1, NULL, NULL, FALSE, TRUE); // adding DUMMY element
	} else if (point == 2) {
		resRules->ln2 = ln;
		resRules->point2 = addUtt_words(resRules->point2, NULL, NULL, FALSE, TRUE); // adding DUMMY element
	}
}

static void resetSpResRule(RES_RULES *resRules, int point) {
	if (point == 1 && resRules->point1 == NULL) {
		resRules->pointsFound = 0;
	} else if (point == 2 && resRules->point2 == NULL) {
		resRules->pointsFound = 1;
	}
}

static void setSpResRule(RES_RULES *resRules, int point) {
	if (point == 1 && resRules->point1 != NULL) {
		resRules->pointsFound = 1;
	} else if (point == 2 && resRules->point2 != NULL) {
		resRules->pointsFound = 2;
	}
}

static void resetUtt_wordMatched(UTTS_LIST *utt) {
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

static char EarnedPoint(RES_RULES *spResRule, RULES_COND *rule_cond) {
	int cnt, cntMatched;
	char word1[BUFSIZ], word2[BUFSIZ];
	UTT_WORDS *point1, *point2;

	if (spResRule->cntStem1 > spResRule->cntStem2)
		cnt = spResRule->cntStem2;
	else
		cnt = spResRule->cntStem1;
	cntMatched = 0;
	for (point1=spResRule->point1; point1 != NULL && point1->isLastStop == FALSE; point1=point1->next_word) {
		getStem(point1, word1);
		for (point2=spResRule->point2; point2 != NULL && point2->isLastStop == FALSE; point2=point2->next_word) {
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

static char isExcludeResult(RULES_LIST *rule, RULES_COND *rule_cond, RES_RULES *spResRule, int point) {
	char wild, isStillMatched;
	PAT_ELEMS *flatp;
	UTT_WORDS *utt_word, *tUtt_word;
	TEMPMATCH tempMatch;

	if (exclude == NULL)
		return(FALSE);
	if (rule->isTrueAnd)
		wild = 1;
	else
		wild = 0;
	for (flatp=rule_cond->exclude; flatp != NULL; flatp=flatp->orElem) {
		if (point == 1)
			utt_word = spResRule->point1;
		else if (point == 2)
			utt_word = spResRule->point2;
		else
			return(FALSE);
		while (utt_word != NULL && utt_word->isLastStop == FALSE) {
			resetElems(flatp);
			tUtt_word = utt_word;
			isStillMatched = TRUE;
			if (flatp->isAllNeg) {
				utt_word = negMatch(utt_word,flatp,wild,rule->isTrueAnd,&tempMatch);
				if (utt_word != NULL) {
					break;
				} else {
					utt_word = tUtt_word;
					utt_word = utt_word->next_word;
				}
			} else {
				utt_word = match(utt_word,flatp,wild,&isStillMatched,rule->isTrueAnd,&tempMatch);
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

static int getAdditionalScore(RULES_LIST *rule, RES_RULES *resRules) {
	int cnt, cntMatched;
	IFLST *ifs;
	ADDLST *adds;
	RULES_LIST *tRule;
	RES_RULES *resRule;

	for (adds=rule->adds; adds != NULL; adds=adds->next_add) {
		cnt = 0;
		cntMatched = 0;
		for (ifs=adds->ifs; ifs != NULL; ifs=ifs->next_if) {
			cnt++;
			for (tRule=root_rules; tRule != NULL; tRule=tRule->next_rule) {
				if (uS.mStricmp(tRule->name, ifs->name) == 0)
					break;
			}
			if (tRule == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, adds->ln);
				fprintf(stderr, "Rule \"%s\" was not declared before referenced in ADD element of rule \"%s\"\n", ifs->name, rule->name);
				ipsyn_overflow(FALSE);
			}
			for (resRule=resRules; resRule != NULL; resRule=resRule->next_result) {
				if (resRule->rule != NULL) {
					if (uS.mStricmp(ifs->name,resRule->rule->name) == 0 && resRule->pointsFound == ifs->score) {
						cntMatched++;
						break;
					}
				}
			}
		}
		if (cnt == cntMatched) {
			return(adds->score);
		}
	}
	return(0);
}

static void cleanSpID(SPEAKERS_LIST *sp) {
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
static void createIPSynCoding(void) {
	int  point, N, V, Q, S, addScore;
	char wild, isStillMatched, isMatchFound, *sFName;
	RULES_COND *rule_cond;
	RULES_LIST *rule;
	UTTS_LIST *utt;
	SPEAKERS_LIST *sp;
	PAT_ELEMS *flatp;
	RES_RULES *spResRule;
	UTT_WORDS *utt_word, *tUtt_word, *orgUtt_word;
	TEMPMATCH tempMatch;

	for (sp=root_sp; sp != NULL; sp=sp->next_sp) {
		if (sp->isSpeakerFound == FALSE)
			continue;
		if (sp->utts == NULL) {
			fprintf(stderr,"\nWARNING: No %%mor: tier found for speaker: %s\n\n", sp->speaker);
			continue;
		}
		createWordFeats(sp->utts);
		for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
			resetUtt_wordMatched(sp->utts);
			for (point=1; point < 3; point++) {
				isMatchFound = FALSE;
				for (rule_cond=rule->cond; rule_cond != NULL; rule_cond=rule_cond->next_cond) {
					if (rule_cond->include != NULL && FoundRule(sp->resRules,rule_cond->deps) &&
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
										utt_word = negMatch(utt_word,flatp,wild,rule->isTrueAnd,&tempMatch);
										if (utt_word != NULL) {
											break;
										} else {
											utt_word = tUtt_word;
											utt_word = utt_word->next_word;
										}
									} else {
										utt_word = match(utt_word,flatp,wild,&isStillMatched,rule->isTrueAnd,&tempMatch);
										if (utt_word != NULL && isStillMatched) {
											spResRule = NULL;
											spResRule = setmat(spResRule, flatp, sp, rule, point);
											if (spResRule != NULL) {
												addDummyToSpResRules(spResRule, point, utt->ln);
												if (!isExcludeResult(rule, rule_cond, spResRule, point)) {
													if (point == 2) {
														if (rule_cond->diffCnt == -1) {
															isMatchFound = TRUE;
															setUtt_wordMatched(flatp);
															setSpResRule(spResRule, point);
														} else if (EarnedPoint(spResRule, rule_cond)) {
															isMatchFound = TRUE;
															setUtt_wordMatched(flatp);
															setSpResRule(spResRule, point);
															break;
														} else {
															freeUtt_words(spResRule->point2, FALSE);
															spResRule->point2 = NULL;
															resetSpResRule(spResRule, point);
														}
													} else {
														isMatchFound = TRUE;
														setUtt_wordMatched(flatp);
														setSpResRule(spResRule, point);
														break;
													}
												} else {
													if (point == 1) {
														freeUtt_words(spResRule->point1, FALSE);
														spResRule->point1 = NULL;
													} else if (point == 2) {
														freeUtt_words(spResRule->point2, FALSE);
														spResRule->point2 = NULL;
													}
													resetSpResRule(spResRule, point);
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
			if (0)
				cleanSpID(sp);
			fprintf(fpout,"\t%s\n", sp->ID);
		} else {
//			fprintf(fpout,"\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.\n", sp->speaker);
			fputc('\n', fpout);
		}
		fputc('\n', fpout);
		N = 0;
		V = 0;
		Q = 0;
		S = 0;
		for (rule=root_rules; rule != NULL; rule=rule->next_rule) {
			for (spResRule=sp->resRules; spResRule != NULL; spResRule=spResRule->next_result) {
				if (spResRule->rule == rule && (spResRule->point1 != NULL || spResRule->point2 != NULL)) {
					fprintf(fpout, "Rule: %s\n", rule->name);
					if (spResRule->point1 != NULL) {
						if (isVerb && !combinput)
							fprintf(fpout,"        File \"%s\": line %d.\n", oldfname, spResRule->ln1);
						fprintf(fpout, "    Point1:");
						for (utt_word=spResRule->point1; utt_word != NULL && utt_word->isLastStop == FALSE; utt_word=utt_word->next_word) {
							if (utt_word->isClitic)
								fprintf(fpout, " ~%s", utt_word->oMor_word);
							else
								fprintf(fpout, " %s", utt_word->oMor_word);
						}
						fprintf(fpout, "\n");
					}
					if (spResRule->point2 != NULL) {
						if (isVerb && !combinput)
							fprintf(fpout,"        File \"%s\": line %d.\n", oldfname, spResRule->ln2);
						fprintf(fpout, "    Point2:");
						for (utt_word=spResRule->point2; utt_word != NULL && utt_word->isLastStop == FALSE; utt_word=utt_word->next_word) {
							if (utt_word->isClitic)
								fprintf(fpout, " ~%s", utt_word->oMor_word);
							else
								fprintf(fpout, " %s", utt_word->oMor_word);
						}
						fprintf(fpout, "\n");
					}
					if (rule->adds == NULL)
						addScore = 0;
					else
						addScore = getAdditionalScore(rule, sp->resRules);
					addScore = addScore + spResRule->pointsFound;
					fprintf(fpout, "  Score: %d\n\n", addScore);
					if (rule->name[0] == 'N' || rule->name[0] == 'n')
						N += addScore;
					else if (rule->name[0] == 'V' || rule->name[0] == 'v')
						V += addScore;
					else if (rule->name[0] == 'Q' || rule->name[0] == 'q')
						Q += addScore;
					else if (rule->name[0] == 'S' || rule->name[0] == 's')
						S += addScore;
					break;
				}
			}
			if (spResRule == NULL) {
				fprintf(fpout, "Rule: %s\n", rule->name);
				if (rule->adds == NULL)
					addScore = 0;
				else
					addScore = getAdditionalScore(rule, sp->resRules);
				fprintf(fpout, "  Score: %d\n\n", addScore);
				if (addScore > 0) {
					if (rule->name[0] == 'N' || rule->name[0] == 'n')
						N += addScore;
					else if (rule->name[0] == 'V' || rule->name[0] == 'v')
						V += addScore;
					else if (rule->name[0] == 'Q' || rule->name[0] == 'q')
						Q += addScore;
					else if (rule->name[0] == 'S' || rule->name[0] == 's')
						S += addScore;
				}
			}
		}
		fprintf(fpout, "N = %d\n", N);
		fprintf(fpout, "V = %d\n", V);
		fprintf(fpout, "Q = %d\n", Q);
		fprintf(fpout, "S = %d\n", S);
		fprintf(fpout, "\nTotal = %d\n", N+V+Q+S);
		freeResRules(sp->resRules);
		sp->resRules = NULL;
	}
}

static SPEAKERS_LIST *ipsyn_FindSpeaker(char *sp, char *ID, char isSpeakerFound) {
	SPEAKERS_LIST *ts;
	
	uS.remblanks(sp);
	if (root_sp == NULL) {
		if ((ts=NEW(SPEAKERS_LIST)) == NULL)
			ipsyn_overflow(TRUE);
		root_sp = ts;
	} else {
		for (ts=root_sp; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->speaker, sp, FALSE, FALSE) && (ts->fileID == fileID || combinput)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
			if (ts->next_sp == NULL)
				break;
		}
		if ((ts->next_sp=NEW(SPEAKERS_LIST)) == NULL) {
			ipsyn_overflow(TRUE);
		}
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	ts->isSpeakerFound = isSpeakerFound;
	ts->speaker = NULL;
	ts->utts = NULL;
	ts->uttnum = 0;
	ts->resRules = NULL;
	ts->fileID = fileID;
	ts->speaker = (char *)malloc(strlen(sp)+1);
	if (ts->speaker == NULL)
		ipsyn_overflow(TRUE);
	strcpy(ts->speaker, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			ipsyn_overflow(TRUE);
		strcpy(ts->ID, ID);
	}
	return(ts);
}

char isIPRepeatUtt(SPEAKERS_LIST *ts, char *line) {
	int  k;
	char isEmpty;
	UTTS_LIST *utt;
	
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

static char forceInclude(char *line) {
	char *s, *sp;
	
	if (isIPpostcode) {
		for (sp=line; *sp; sp++) {
			if (*sp == '[' && *(sp+1) == '+') {
				for (s=sp+2; isSpace(*s); s++) ;
				if ((*s     == 'i' || *s     == 'I') && 
					(*(s+1) == 'p' || *(s+1) == 'P') && 
					*(s+2) == ']') {
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

static char isExcludeIpsynTier(char *line, char *tline) {
	char *s, *sp, isUttDelFound;

	if (isIPEpostcode) {
		for (sp=line; *sp; sp++) {
			if (*sp == '[' && *(sp+1) == '+') {
				for (s=sp+2; isSpace(*s); s++) ;
				if ((*s     == 'i' || *s     == 'I') && 
					(*(s+1) == 'p' || *(s+1) == 'P') && 
					(*(s+2) == 'e' || *(s+2) == 'E') && 
					*(s+3)  == ']') {
					return(TRUE);
				}
			}
		}
	}
	for (sp=line; *sp; sp++) {
		if (*sp == 'x' || *sp == 'X') {
			if ((*(sp+1) == 'x' || *(sp+1) == 'X') && 
				(*(sp+2) == 'x' || *(sp+2) == 'X' || !isalpha(*(sp+2)))) {
				return(TRUE);
			}
		} else if (*sp == 'y' || *sp == 'Y') {
			if ((*(sp+1) == 'y' || *(sp+1) == 'Y') && 
				(*(sp+2) == 'y' || *(sp+2) == 'Y' || !isalpha(*(sp+2)))) {
				return(TRUE);
			}
		} else if (*sp == 'w' || *sp == 'W') {
			if ((*(sp+1) == 'w' || *(sp+1) == 'W') && 
				(*(sp+2) == 'w' || *(sp+2) == 'W' || !isalpha(*(sp+2)))) {
				return(TRUE);
			}
		}
	}
	isUttDelFound = FALSE;
	for (sp=tline; *sp; sp++) {
		if (*sp == '.' || *sp == '!' || *sp == '?')
			isUttDelFound = TRUE;
	}
	if (isUttDelFound == FALSE)
		return(TRUE);
	return(FALSE);
}

static UTTS_LIST *add2Utts(SPEAKERS_LIST *sp, char *spkLine) {
	UTTS_LIST *utt;

	if (sp->utts == NULL) {
		if ((utt=NEW(UTTS_LIST)) == NULL) {
			ipsyn_overflow(TRUE);
		}
		sp->utts = utt;
	} else {
		for (utt=sp->utts; utt->next_utt != NULL; utt=utt->next_utt) ;
		if ((utt->next_utt=NEW(UTTS_LIST)) == NULL) {
			ipsyn_overflow(TRUE);
		}
		utt = utt->next_utt;
	}
	utt->next_utt = NULL;
	utt->spkLine = NULL;
	utt->morLine = NULL;
	utt->words = NULL;
	utt->ln = lineno;
	utt->spkLine = (char *)malloc(strlen(spkLine)+1);
	if (utt->spkLine == NULL) {
		ipsyn_overflow(TRUE);
	}
	strcpy(utt->spkLine, spkLine);
	return(utt);
}

static void ips_notEnough(void) {
	SPEAKERS_LIST *tsp;

	for (tsp=root_sp; tsp != NULL; tsp=tsp->next_sp) {
		if (tsp->uttnum < IPS_UTTLIM) {
			fprintf(stderr, "\n\nCAN'T FIND NECESSARY SAMPLE SIZE OF %d OF COMPLETE UTTERANCES.\n", IPS_UTTLIM);
			fprintf(stderr, "%s FOUND ONLY %d UTTERANCES\n", tsp->speaker, tsp->uttnum);
			fprintf(stderr, "IF YOU WANT TO USE SMALLER SAMPLE SIZE, THEN USE \"+c\" OPTION\n\n");
			if (fpout != stdout) {
				fprintf(fpout, "\n\nCAN'T FIND NECESSARY SAMPLE SIZE OF %d OF COMPLETE UTTERANCES.\n", IPS_UTTLIM);
				fprintf(fpout, "%s FOUND ONLY %d UTTERANCES\n", tsp->speaker, tsp->uttnum);
				fprintf(fpout, "IF YOU WANT TO USE SMALLER SAMPLE SIZE, THEN USE \"+c\" OPTION\n\n");
			}
			tsp->isSpeakerFound = FALSE;
		}
	}
}

#ifndef KIDEVAL_LIB
void call() {
	char lRightspeaker, isFC;
	FNType tfname[FNSize];
	FILE *tf;
	UTTS_LIST *utt;
	SPEAKERS_LIST *ts;

	parsfname(oldfname, tfname, ".ipcore");
	if ((tfp=openwfile(oldfname,tfname,tfp)) == NULL) {
		fprintf(stderr,"WARNING: Can't open dss file %s.\n", tfname);
	}
	fileID++;
	ts = NULL;
	utt = NULL;
	lRightspeaker = FALSE;
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
				utt = NULL;
			}
			if (!lRightspeaker)
				continue;
		}
		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC1, TRUE)) {
					uS.remblanks(utterance->line);
					ipsyn_FindSpeaker(templineC1, utterance->line, FALSE);
				}
			}
		} else if (utterance->speaker[0] == '*') {
			ts = ipsyn_FindSpeaker(utterance->speaker, NULL, TRUE);
			if (ts->uttnum >= IPS_UTTLIM) {
				ts = NULL;
				utt = NULL;
				lRightspeaker = FALSE;
			} else {
				cleanUttline(uttline);
				isFC = forceInclude(utterance->line);
				if (!isFC && (strcmp(uttline, spareTier1) == 0 || isExcludeIpsynTier(utterance->line, uttline))) {
					spareTier1[0] = EOS;
					ts = NULL;
					lRightspeaker = FALSE;
				} else {
					if (!isIPRepeatUtt(ts, uttline) || isFC) {
						ts->uttnum++;
						utt = add2Utts(ts, uttline);
						strcpy(spareTier1, uttline);
						if (tfp != NULL) {
							tf = fpout;
							fpout = tfp;
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
							fpout = tf;
						}
					} else {
						spareTier1[0] = EOS;
						ts = NULL;
						lRightspeaker = FALSE;
					}
				}
			}
		} else if (ts != NULL && uS.partcmp(utterance->speaker, morTierName, FALSE, FALSE)) {
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			createMorUttline(utterance->line, org_spTier, utterance->tuttline, TRUE);
			strcpy(utterance->tuttline, utterance->line);
			if (strchr(uttline, dMarkChr) == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", oldfname, lineno);
				fprintf(stderr,"%%mor: tier does not have one-to-one correspondence to it's speaker tier.\n");
				fprintf(stderr,"Speaker tier: %s\n%%mor tier: %s\n", org_spTier, utterance->line);
				ipsyn_overflow(FALSE);
			}
			if (utt == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", oldfname, lineno);
				fprintf(stderr,"INTERNAL ERROR utt == null");
				ipsyn_overflow(FALSE);
			}
			utt->morLine = (char *)malloc(strlen(utterance->tuttline)+1);
			if (utt->morLine == NULL) {
				ipsyn_overflow(TRUE);
			}
			strcpy(utt->morLine, utterance->tuttline);
		}
	}
	if (!combinput) {
		if (tfp != NULL) {
			fclose(tfp);
			tfp = NULL;
		}
		ips_notEnough();
		freeSpkLines(root_sp);
		createIPSynCoding();
		root_sp = freeSpeakers(root_sp);
	}
}
#endif // KIDEVAL_LIB

static void ips_pr_result(void) {
	ips_notEnough();
	freeSpkLines(root_sp);
	createIPSynCoding();
	root_sp = freeSpeakers(root_sp);
}

#ifndef KIDEVAL_LIB
CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = IPSYN;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	root_rules = NULL;
	root_pats = NULL;
	root_vars = NULL;
	tempPat.duplicatesList = NULL;
	tempPat.origmac = NULL;
	tempPat.flatmac = NULL;
	bmain(argc,argv,ips_pr_result);
	if (combinput) {
		if (tfp != NULL) {
			fclose(tfp);
			tfp = NULL;
		}
	}
	root_rules = freeRules(root_rules);
	root_pats = freePats(root_pats);
	root_vars = freeVars(root_vars);
	tempPat.duplicatesList = freeIEWORDS(tempPat.duplicatesList);
	freePats(tempPat.origmac, TRUE);
	freePats(tempPat.flatmac, TRUE);
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

static char isMorPat(char *f) {
	if (*f == '+' || *f == '~')
		f++;
	if (*f == '#') // prefix
		return(TRUE);
	else if (*f == '~') // postclitic
		return(TRUE);
	else if (*f == '$') // preclitic
		return(TRUE);
	else if (*f == '|') // part of speech
		return(TRUE);
	else if (*f == 'r') {// stem
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	} else if (*f == ';') // stem
		return(TRUE);
	else if (*f == '-') // suffix
		return(TRUE);
	else if (*f == '&') // fusion
		return(TRUE);
	else if (*f == '=') // trans
		return(TRUE);
	else if (*f == '@') // repls
		return(TRUE);
	else if (*f == '*') // errors
		return(TRUE);
	else if (*f == 'o') {// other (default *)
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	}
	return(FALSE);
}

static PATS_LIST *addToPatsList(int ln, char *pat_str) {
	char res, isClitic;
	PATS_LIST *pat_elem;

	if (pat_str[0] == '~') {
		isClitic = TRUE;
		strcpy(pat_str, pat_str+1);
	} else
		isClitic = FALSE;
	if (root_pats == NULL) {
		if ((pat_elem=NEW(PATS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(TRUE);
		}
		root_pats = pat_elem;
	} else {
		for (pat_elem=root_pats; 1; pat_elem=pat_elem->next_pat) {
			if (uS.mStricmp(pat_elem->pat_str, pat_str) == 0 && pat_elem->isClitic == isClitic)
				return(pat_elem);
			if (pat_elem->next_pat == NULL)
				break;
		}
		if ((pat_elem->next_pat=NEW(PATS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(TRUE);
		}
		pat_elem = pat_elem->next_pat;
	}
	pat_elem->next_pat = NULL;
	pat_elem->pat_feat = NULL;
	pat_elem->pat_str = NULL;
	if ((pat_elem->pat_str=(char *)malloc(strlen(pat_str)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		ipsyn_overflow(TRUE);
	}
	strcpy(pat_elem->pat_str, pat_str);
	if (isMorPat(pat_str))
		pat_elem->whatType = MOR_TIER;
	else
		pat_elem->whatType = SPK_TIER;
	pat_elem->isClitic = isClitic;
	if (pat_str[0] == EOS) {
		pat_elem->pat_feat = NULL;
	} else if (strcmp(pat_str, "_") == 0) {
		pat_elem->pat_feat = NULL;
	} else if (pat_elem->whatType == MOR_TIER) {
		pat_elem->pat_feat = makeMorWordList(pat_elem->pat_feat, &res, pat_str, 'i');
		if (pat_elem->pat_feat == NULL) {
			fprintf(stderr,"*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(res);
		}				
	} else
		pat_elem->pat_feat = NULL;
	return(pat_elem);
}

static PAT_ELEMS *mkPatElem(char isTrueAnd) {
	PAT_ELEMS *t;

	if ((t=NEW(PAT_ELEMS)) == NULL)
		ipsyn_overflow(TRUE);
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

static void expandVarInLine(int ln, char *isRepeat, char *from, char *to) {
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
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Undecleared variable: %s\n", name);
				ipsyn_overflow(FALSE);
			}
			from[i] = t;
			if ((initial > 0 || t != EOS) && *isRepeat) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Illegal use of repeating variable \"%s\" along with other strings in:\n", name);
				fprintf(stderr, "    %s\n", from);
				ipsyn_overflow(FALSE);
			}
			if ((initial > 0 || t != EOS) && templineC4[0] == '(') {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Illegal use of variable \"%s\" containing operator(s) '+' or '^'\n", name);
				fprintf(stderr, "    along with other strings in: %s\n", from);
				ipsyn_overflow(FALSE);
			}
			strcpy(to+j, templineC4);
			j = strlen(to);
		} else
			to[j++] = from[i++];
	}
	to[j] = EOS;
}

static long makePatElems(int ln, char *rname, const char *part, char *exp, long pos, PAT_ELEMS *elem, char isTrueAnd);

static long storepats(int ln, char *rname, const char *part, PAT_ELEMS *elem, char *exp, long pos, char isTrueAnd) {
	long p1, p2;
	char t, isRepeat, *buf;

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
	if (strchr(templineC2, '$') == NULL) 
		elem->pat_elem = addToPatsList(ln, templineC2);
	else {
		isRepeat = 0;
		do {
			expandVarInLine(ln, &isRepeat, templineC2, templineC3);
			if (templineC3[0] == '(') {
				if (isRepeat) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Repeating variable is not allowed to have operator(s):\n");
					fprintf(stderr, "    %s += %s\n", templineC2, templineC3);
					ipsyn_overflow(FALSE);
				} else {
					buf = (char *)malloc(strlen(templineC3)+1);
					if (buf == NULL) {
						ipsyn_overflow(TRUE);
					}
					strcpy(buf, templineC3);
					elem->pat_elem = NULL;
					p1 = 1;
					elem->parenElem = mkPatElem(isTrueAnd);
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
		elem->pat_elem = addToPatsList(ln, templineC2);
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

	if ((t=NEW(IEWORDS)) == NULL)
		ipsyn_overflow(TRUE);
	t->word = (char *)malloc(strlen(str)+1);
	if (t->word == NULL) {
		free(t);
		ipsyn_overflow(TRUE);
	}
	strcpy(t->word, str);
	t->nextword = lst;
	lst = t;
	return(lst);
}

static void removeDuplicateFlatMacs(char isEcho) {
	PAT_ELEMS *flatp, *lflatp;

	lflatp = tempPat.flatmac;
	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		if (!flatp->isCheckedForDups) {
			flatp->isCheckedForDups = TRUE;
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			if (!isInLst(tempPat.duplicatesList, templineC1)) {
				tempPat.duplicatesList = InsertFlatp(tempPat.duplicatesList, templineC1);
				if (isEcho)
					fprintf(stderr, "%s\n", templineC1);
			} else {
				lflatp->orElem = flatp->orElem;
				freePats(flatp, FALSE);
				flatp = lflatp;
			}
		} else if (isEcho) {
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			fprintf(stderr, "%s\n", templineC1);
		}
		lflatp = flatp;
	}
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

static void addParenToLastFlatMacs(PAT_ELEMS *cur, char isTrueAnd) {
	PAT_ELEMS *elem, *flatp;

	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->parenElem == NULL && elem->pat_elem == NULL) {
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->matchedWord = cur->matchedWord;
			elem->refParenElem = cur->parenElem;
			elem->parenElem = mkPatElem(isTrueAnd);
		}
	}
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

static void addAndToFlatMacs(PAT_ELEMS *cur, char isTrueAnd) {
	PAT_ELEMS *elem, *flatp;

	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->andElem == NULL) {
			elem->refAndElem = cur->andElem;
			elem->andElem = mkPatElem(isTrueAnd);
		}
	}
}

static void copyMac(PAT_ELEMS *elem, PAT_ELEMS *lastElem, PAT_ELEMS *flatp, char isTrueAnd) {
	if (flatp != NULL && lastElem != flatp) {
		elem->wild = flatp->wild;
		elem->neg = flatp->neg;
		elem->matchedWord = flatp->matchedWord;
		if (flatp->parenElem != NULL) {
			elem->refParenElem = flatp->refParenElem;
			elem->parenElem = mkPatElem(isTrueAnd);
			copyMac(elem->parenElem, lastElem, flatp->parenElem, isTrueAnd);
		} else if (flatp->pat_elem != NULL) {
			elem->pat_elem = flatp->pat_elem;
		}
		if (flatp->andElem != NULL) {
			elem->refAndElem = flatp->refAndElem;
			elem->andElem = mkPatElem(isTrueAnd);
			copyMac(elem->andElem, lastElem, flatp->andElem, isTrueAnd);
		}
	}
}

static void duplicateFlatMacs(char isTrueAnd) {
	PAT_ELEMS *elem, *elemRoot, *flatp, *lastElem;

	removeDuplicateFlatMacs(FALSE);
	elemRoot = NULL;
	for (flatp=tempPat.flatmac; flatp != NULL; flatp=flatp->orElem) {
		lastElem = getLastElem(flatp);
		if (lastElem != NULL) {
			if (elemRoot == NULL) {
				elemRoot = mkPatElem(isTrueAnd);
				elem = elemRoot;
			} else {
				for (elem=elemRoot; elem->orElem != NULL; elem=elem->orElem) ;
				elem->orElem = mkPatElem(isTrueAnd);
				elem = elem->orElem;
			}
			copyMac(elem, lastElem, flatp, isTrueAnd);
		}
	}
	if (tempPat.flatmac == NULL) {
		tempPat.flatmac = elemRoot;
	} else {
		for (flatp=tempPat.flatmac; flatp->orElem != NULL; flatp=flatp->orElem) ;
		flatp->orElem = elemRoot;
	}
}

static void addToStack(PAT_ELEMS *elem) {
	if (tempPat.stackAndsI >= 200) {
		fprintf(stderr, "ipsyn: stack index exceeded 200 \"^\" elements\n");
		ipsyn_overflow(FALSE);
	}
	tempPat.stackAnds[tempPat.stackAndsI] = elem;
	tempPat.stackAndsI = tempPat.stackAndsI + 1;
}

static void flatten_expr(PAT_ELEMS *cur, char isTrueAnd) {
	if (cur->parenElem != NULL) {
		addParenToLastFlatMacs(cur, isTrueAnd);
		addToStack(cur->parenElem);
		flatten_expr(cur->parenElem, isTrueAnd);
		tempPat.stackAndsI = tempPat.stackAndsI - 1;
	} else if (cur->pat_elem != NULL) {
		addPatToLastFlatMacs(cur);
	}
	if (cur->andElem != NULL) {
		addAndToFlatMacs(cur, isTrueAnd);
		addToStack(cur->andElem);
		flatten_expr(cur->andElem, isTrueAnd);
		tempPat.stackAndsI = tempPat.stackAndsI - 1;
	}
	if (cur->orElem != NULL) {
		duplicateFlatMacs(isTrueAnd);
		flatten_expr(cur->orElem, isTrueAnd);
	}
}

static void remSpaces(char *s) {
	while (*s != EOS) {
		if (*s == '\t' || *s == ' ' || *s == '\n')
			strcpy(s, s+1);
		else
			s++;
	}
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

static IFLST *addIfs(int ln, char *line, int b, char *rname) {
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
			ipsyn_overflow(FALSE);
		}
		nameb = b;
		for (; isalpha(line[b]); b++) ;
		for (; isdigit(line[b]); b++) ;
		if (line[b] != '=') {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected \"=\" character in ADD element of RULENAME: %s\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			ipsyn_overflow(FALSE);
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
			ipsyn_overflow(FALSE);
		}
		score = atoi(line+b);
		for (; isdigit(line[b]); b++) ;
		if (line[b] != ',' && line[b] != EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Expected \",\" character or end of line in ADD element of RULENAME: %s\n", rname);
			fprintf(stderr, "ADD: ");
			ipsyn_err(line, b, TRUE);
			ipsyn_overflow(FALSE);
		}
		if (root == NULL) {
			ifs = NEW(IFLST);
			if (ifs == NULL)
				ipsyn_overflow(TRUE);
			root = ifs;
		} else {
			for (ifs=root; ifs->next_if != NULL; ifs=ifs->next_if) ;
			if ((ifs->next_if=NEW(IFLST)) == NULL)
				ipsyn_overflow(TRUE);
			ifs = ifs->next_if;
		}
		ifs->next_if = NULL;
		ifs->score = score;
		if ((ifs->name=(char *)malloc(strlen(name)+1)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(TRUE);
		}
		strcpy(ifs->name, name);
		if (line[b] == EOS)
			break;
		else
			b++;
	}
	return(root);
}

static ADDLST *parseADDField(ADDLST *root, int ln, char *line, char *rname) {
	int score, b;
	ADDLST *adds;

	b = 0;
	if (!isdigit(line[b])) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    Expected score number in ADD element of RULENAME: %s\n", rname);
		fprintf(stderr, "ADD: ");
		ipsyn_err(line, b, TRUE);
		ipsyn_overflow(FALSE);
	}
	score = atoi(line+b);
	for (; isdigit(line[b]); b++) ;
	if ((line[b] != 'I' && line[b] != 'i') || (line[b+1] != 'F' && line[b+1] != 'f')) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    Expected \"IF\" keyword in ADD element of RULENAME: %s\n", rname);
		fprintf(stderr, "ADD: ");
		ipsyn_err(line, b, TRUE);
		ipsyn_overflow(FALSE);
	}
	b = b + 2;
	if (root == NULL) {
		adds = NEW(ADDLST);
		if (adds == NULL)
			ipsyn_overflow(TRUE);
		root = adds;
	} else {
		for (adds=root; adds->next_add != NULL; adds=adds->next_add) ;
		if ((adds->next_add=NEW(ADDLST)) == NULL)
			ipsyn_overflow(TRUE);
		adds = adds->next_add;
	}
	adds->next_add = NULL;
	adds->score = score;
	adds->ln = ln;
	adds->ifs = NULL;
	adds->ifs = addIfs(ln, line, b, rname);
	return(root);
}

static void addVar(int ln, char isRepeat, char *name, char *line) {
	int i;
	VARS_LIST *var;

	if (root_vars == NULL) {
		if ((var=NEW(VARS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(TRUE);
		}
		root_vars = var;
	} else {
		for (var=root_vars; 1; var=var->next_var) {
			if (uS.mStricmp(var->name, name) == 0) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Variable by name \"%s\" has already been declared\n", name);
				ipsyn_overflow(FALSE);
			}
			if (var->next_var == NULL)
				break;
		}
		if ((var->next_var=NEW(VARS_LIST)) == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			ipsyn_overflow(TRUE);
		}
		var = var->next_var;
	}
	var->next_var = NULL;
	var->isRepeat = isRepeat;
	var->name = NULL;
	var->line = NULL;
	if ((var->name=(char *)malloc(strlen(name)+1)) == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		ipsyn_overflow(TRUE);
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
		ipsyn_overflow(TRUE);
	}
	strcpy(var->line, templineC2);
}

static RULES_LIST *createNewRule(int ln, char *name) {
	RULES_LIST *rule;

	if (root_rules == NULL) {
		if ((rule=NEW(RULES_LIST)) == NULL)
			ipsyn_overflow(TRUE);
		root_rules = rule;
	} else {
		for (rule=root_rules; 1; rule=rule->next_rule) {
			if (uS.mStricmp(rule->name, name) == 0) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr,"RULENAME \"%s\" has already been declared\n", name);
				ipsyn_overflow(FALSE);
			}
			if (rule->next_rule == NULL)
				break;
		}
		if ((rule->next_rule=NEW(RULES_LIST)) == NULL)
			ipsyn_overflow(TRUE);
		rule = rule->next_rule;
	}
	rule->next_rule = NULL;
	rule->isTrueAnd = isAndSymFound;
	rule->adds = NULL;
	rule->cond = NULL;
	if ((rule->name=(char *)malloc(strlen(name)+1)) == NULL)
		ipsyn_overflow(TRUE);
	strcpy(rule->name, name);
	return(rule);
}

static RULES_COND *createNewCond(RULES_LIST *rule) {
	RULES_COND *cond;
	
	if (rule->cond == NULL) {
		if ((cond=NEW(RULES_COND)) == NULL)
			ipsyn_overflow(TRUE);
		rule->cond = cond;
	} else {
		for (cond=rule->cond; cond->next_cond != NULL; cond=cond->next_cond) ;
		if ((cond->next_cond=NEW(RULES_COND)) == NULL)
			ipsyn_overflow(TRUE);
		cond = cond->next_cond;
	}
	cond->next_cond = NULL;
	cond->point = 0;
	cond->diffCnt = -1;
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

static void addPatElems(int ln, const char *op, RULES_LIST *rule, RULES_COND *cond, char *line) {	
	char isNeg;

	if (op[0] == 'i') {
		if (cond->include != NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Only one INCLUDE element allowed in \"if\" part of RULENAME \"%s\".\n", rule->name);
			ipsyn_overflow(FALSE);
		}
	} else if (op[0] == 'e') {
		if (cond->exclude != NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
			fprintf(stderr, "    Only one EXCLUDE element allowed in \"if\" part of RULENAME \"%s\".\n", rule->name);
			ipsyn_overflow(FALSE);
		}
	}
	tempPat.origmac = mkPatElem(rule->isTrueAnd);
	if (makePatElems(ln,rule->name,op,line,0L,tempPat.origmac,rule->isTrueAnd) == -1L) {
		ipsyn_overflow(FALSE);
	}
	tempPat.duplicatesList = NULL;
	tempPat.stackAndsI = 0;
	tempPat.flatmac = mkPatElem(rule->isTrueAnd);
	flatten_expr(tempPat.origmac, rule->isTrueAnd);
	removeDuplicateFlatMacs(FALSE);
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
		ipsyn_overflow(FALSE);
	}	
	tempPat.flatmac = NULL;
	freePats(tempPat.origmac, TRUE);
	tempPat.origmac = NULL;
}

static RULES_COND *isRuleDeclErr(int ln, char *name, char *line, RULES_LIST *rule, RULES_COND *cond) {
	if (rule == NULL) {
		fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
		fprintf(stderr, "    No rule name declared up to this line: %s %s\n", name, line);
		ipsyn_overflow(FALSE);
	}
	if (cond == NULL)
		cond = createNewCond(rule);
	return(cond);
}

static void parseRules(int ln, TEMP_RULE *tr) {
	char *name, *line, isRepeat, isVarFound;

	if (templineC1[0] == '^')
		isAndSymFound = TRUE;
	else if (templineC1[0] == '>')
		isAndSymFound = FALSE;
	else {
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
				ipsyn_overflow(FALSE);
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
		remSpaces(line);
		if (isVarFound) {
			addVar(ln, isRepeat, name, line);
		} else {
			if (uS.mStricmp(name, "RULENAME:") == 0) {
				tr->rule = createNewRule(ln, line);
				tr->cond = NULL;
			} else if (uS.mStricmp(name, "ADD:") == 0) {
				if (tr->rule == NULL) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    No rule name declared up to this line: %s %s\n", name, line);
					ipsyn_overflow(FALSE);
				}
				tr->rule->adds = parseADDField(tr->rule->adds, ln, line, tr->rule->name);
			} else if (uS.mStricmp(name, "if") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, NULL);
			} else if (uS.mStricmp(name, "INCLUDE:") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond);
				addPatElems(ln, "include", tr->rule, tr->cond, line);
			} else if (uS.mStricmp(name, "EXCLUDE:") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond);
				addPatElems(ln, "exclude", tr->rule, tr->cond, line);
			} else if (uS.mStricmp(name, "ALSO:") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond);
				validateDepRules(ln, line, tr->rule->name, "ALSO");
				tr->cond->deps = addDeps(tr->cond->deps, line);
			} else if (uS.mStricmp(name, "POINT:") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond);
				if (tr->cond->point != 0) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Only one POINT element allowed in \"if\" part of RULENAME \"%s\".\n", tr->rule->name);
					ipsyn_overflow(FALSE);
				}
				if (line[0] == '1' && line[1] == EOS)
					tr->cond->point = 1;
				else if (line[0] == '2' && line[1] == EOS)
					tr->cond->point = 2;
				else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    POINT: field can only have \"1\" or \"2\", but not %s %s\n", name, line);
					ipsyn_overflow(FALSE);
				}
			} else if (uS.mStricmp(name, "DIFFERENT_STEMS:") == 0) {
				tr->cond = isRuleDeclErr(ln, name, line, tr->rule, tr->cond);
				if (tr->cond->diffCnt != -1) {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    Only one DIFFERENT_STEMS element allowed in \"if\" part of RULENAME \"%s\".\n", tr->rule->name);
					ipsyn_overflow(FALSE);
				}
				if (line[0] == '1' && line[1] == EOS)
					tr->cond->diffCnt = 1;
				else if (line[0] == '>' && isdigit(line[1]))
					tr->cond->diffCnt = atoi(line+1);
				else {
					fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
					fprintf(stderr, "    DIFFERENT_STEMS: illegal text in field: %s %s\n", name, line);
					ipsyn_overflow(FALSE);
				}
			} else {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", FileName1, ln);
				fprintf(stderr, "    Unknown element \"%s\" found\n", name);
				ipsyn_overflow(FALSE);
			}
		}
	}
}

static void read_ipsyn(char *lang) {
	int  i, ln, cln, len;
	char *c;
	FILE *fp;
	TEMP_RULE tRule;

	if (*lang == EOS) {
		fprintf(stderr,	"No ipsyn rules specified with +l option.\n");
		fprintf(stderr,"Please specify ipsyn rules file name with \"+l\" option.\n");
		fprintf(stderr,"For example, \"ipsyn +leng\" or \"ipsyn +leng.cut\".\n");
		cutt_exit(0);
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
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
		cutt_exit(0);
	}
	fprintf(stderr,"    Using script file: %s\n", FileName1);
	tRule.rule = NULL;
	tRule.cond = NULL;
	ln = 0;
	cln = ln;
	isAndSymFound = FALSE;
	templineC1[0] = EOS;
	len = 0;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE) ||
			uS.partcmp(templineC, CKEYWORDHEADER, FALSE, FALSE) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		ln++;		
		if (templineC[0] == ';' || templineC[0] == '%' || templineC[0] == '#')
			continue;
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
			if (templineC1[0] != EOS)
				parseRules(cln, &tRule);
			cln = ln;
			strcpy(templineC1, templineC);
			len = strlen(templineC1) - 1;
		}
	}
	if (templineC1[0] != EOS)
		parseRules(cln, &tRule);
	fclose(fp);
	if (root_rules == NULL) {
		fprintf(stderr,"Can't find any usable declarations in ipsyn rules file \"%s\".\n", FileName1);
		cutt_exit(0);
	}
	root_vars = freeVars(root_vars);
}
// END reading rules

#ifndef KIDEVAL_LIB
void getflag(char *f, char *f1, int *i) {
	char *s;

	f++;
	switch(*f++) {
		case 'c': 
			if ((IPS_UTTLIM=atoi(f)) <= 0)
				IPS_UTTLIM = 100;
			break;
		case 'd':
			no_arg_option(f);
			isVerb = TRUE;
			break;
		case 'l':
			read_ipsyn(f);
			break;
#ifdef UNX
		case 'L':
			strcpy(lib_dir, f);
			j = strlen(lib_dir);
			if (j > 0 && lib_dir[j-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 't':
			if (*f == '%')
				strcpy(morTierName, getfarg(f,f1,i));
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
/*
						if (*(f-2) == '+')
							isIPpostcode = TRUE;
						 else
*/
							isIPpostcode = FALSE;
					}
					if ((*s     == 'i' || *s     == 'I') && 
						(*(s+1) == 'p' || *(s+1) == 'P') && 
						(*(s+2) == 'e' || *(s+2) == 'E') && 
						(*(s+3) == ']' || *(s+3) == '>' )) {
//						if (*(f-2) == '+')
							isIPEpostcode = FALSE;
/*
						 else
							isIPEpostcode= TRUE;
*/
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
