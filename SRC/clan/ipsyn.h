/**********************************************************************
	"Copyright 1990-2013 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef IPSYNDEF
#define IPSYNDEF

extern "C"
{

#define UTT_WORDS struct utt_words_list

#define PATS_LIST struct pats_list
PATS_LIST {
	char whatType;
	char isClitic;
	char *pat_str;
	MORWDLST *pat_feat;
	PATS_LIST *next_pat;
} ;

#define PAT_ELEMS struct pat_elements
PAT_ELEMS {
	char neg;
	char wild;
	char isRepeat;
	char isCheckedForDups;
	char isAllNeg;
	UTT_WORDS *matchedWord;
	PATS_LIST *pat_elem;
	PAT_ELEMS *parenElem, *refParenElem;
	PAT_ELEMS *andElem, *refAndElem;
	PAT_ELEMS *orElem;
} ;

#define RULES_COND struct rules_condition
RULES_COND {
	int point;
	int diffCnt;
	IEWORDS *deps;
	PAT_ELEMS *include;
	PAT_ELEMS *exclude;
	RULES_COND *next_cond;
} ;

#define IFLST struct if_element
IFLST {
	char *name;
	int score;
	IFLST *next_if;
} ;

#define ADDLST struct add_element
ADDLST {
	int score;
	long ln;
	IFLST *ifs;
	ADDLST *next_add;
} ;

#define RULES_LIST struct rules_list
RULES_LIST {
	char isTrueAnd;
	char *name;
	ADDLST  *adds;
	RULES_COND *cond;
	RULES_LIST *next_rule;
} ;
	
UTT_WORDS {
	int  num;
	char isLastStop;
	char isWordMatched;
	char isClitic;
	char *surf_word;
	char *oMor_word;
	char *mor_word;
	MORFEATS *word_feats;
	UTT_WORDS *next_word;
} ;

#define UTTS_LIST struct utts_list
UTTS_LIST {
	char *spkLine;
	char *morLine;
	long ln;
	UTT_WORDS *words;
	UTTS_LIST *next_utt;
} ;

#define RES_RULES struct res_rules
RES_RULES {
	char pointsFound;
	RULES_LIST *rule;
	long ln1;
	int cntStem1;
	UTT_WORDS *point1;
	long ln2;
	int cntStem2;
	UTT_WORDS *point2;
	RES_RULES *next_result;
} ;

#define SPEAKERS_LIST struct speakers
SPEAKERS_LIST {
	int  fileID;
	char isSpeakerFound;
	char *ID;
	char *speaker;
	int  uttnum;
	UTTS_LIST *utts;
	RES_RULES *resRules;
	SPEAKERS_LIST *next_sp;
} ;

	
extern char init_ipsyn(char first);
extern char isIPRepeatUtt(SPEAKERS_LIST *ts, char *line);
}

#endif /* IPSYNDEF */
