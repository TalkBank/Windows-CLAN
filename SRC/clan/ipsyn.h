/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef IPSYNDEF
#define IPSYNDEF

extern "C"
{

#define UTT_WORDS struct utt_words_list
#define IPSYNNOCOMPUTE "Do_NoT_CoMpUtE"

#define PATS_LIST struct pats_list
PATS_LIST {
	char whatType;
	char isPClitic;
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
	unsigned long diffCnt;
	char isDiffSpec;
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
	char *info;
	ADDLST  *adds;
	RULES_COND *cond;
	RULES_LIST *next_rule;
} ;
	
UTT_WORDS {
	int  num;
	char isLastStop;
	char isWordMatched;
	char isWClitic;
	char isFullWord;
	char *surf_word;
	char *oMor_word;
	char *fMor_word;
	char *mor_word;
	char *gra_word;
	MORFEATS *word_feats;
	UTT_WORDS *next_word;
} ;

#define IPSYN_UTT struct ipsyn_utts
IPSYN_UTT {
	char *spkLine;
	char *morLine;
	char *graLine;
	long ln;
	UTT_WORDS *words;
	IPSYN_UTT *next_utt;
} ;

#define RES_RULES struct res_rules
RES_RULES {
	char pointToSet;
	char pointsEarned;
	RULES_LIST *rule;
	long ln1;
	unsigned long cntStem1;
	UTT_WORDS *point1;
	long ln2;
	unsigned long cntStem2;
	UTT_WORDS *point2;
	RES_RULES *next_result;
} ;

#define IPSYN_SP struct speakers_ipsyn
IPSYN_SP {
	int  fileID;
	char *fname;
	char isSpeakerFound;
	char *ID;
	char *speaker;
	int  UttNum;
	float  TotalScore;
	IPSYN_UTT *utts;
	IPSYN_UTT *cUtt;
	RES_RULES *resRules;
	IPSYN_SP *next_sp;
} ;

extern int  IPS_UTTLIM;
extern char ipsyn_lang[];

extern char compute_ipsyn(char isOutput);
extern char init_ipsyn(char first, int limit);
extern char isIPRepeatUtt(IPSYN_SP *ts, char *line);
extern char isExcludeIpsynTier(char *sp, char *line, char *tline);
extern char forceInclude(char *line);
extern char add2Utts(IPSYN_SP *sp, char *spkLine);
extern void ipsyn_freeSpeakers(void);
extern void ipsyn_cleanSearchMem(void);
extern void removeLastUtt(IPSYN_SP *ts) ;
extern IPSYN_SP *ipsyn_FindSpeaker(char *sp, char *ID, char isSpeakerFound);

}

#endif /* IPSYNDEF */
