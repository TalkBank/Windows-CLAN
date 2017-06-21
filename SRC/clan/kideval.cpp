/**********************************************************************
 "Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

/*
 isCreateDB
 @ID   = +
 *PAR  = *
 %mor  = %
 @G    = G
 @BG   = B
 @EG   = E
 @Time Duration = T
 @END  = -
*/

// #define JUST_ONE_SPEAKER

#define CHAT_MODE 1
#include "cu.h"
#include "dss.h"
#include "vocd/vocdefs.h"
#include "vocd/speaker.h"
#if !defined(_MAC_CODE)
    #include <sys/types.h>
#endif
#include <time.h>
#include <math.h>
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main kideval_main
#define call kideval_call
#define getflag kideval_getflag
#define init kideval_init
#define usage kideval_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "kideval_db.txt"

extern char morTierName[];
extern char OverWriteFile;
extern char outputOnlyData;
extern char isRecursive;
extern struct tier *defheadtier;
extern struct IDtype *IDField;

#define SDRESSIZE 256
#define NUMCOMPS 128

enum {
	BASE_COM = 0,
	DSS_COM,
	VOCD_COM,
	LAST_COMMAND
} ;

struct label_cols {
	char *label;
	char isDisplay;
	int  const_label_num;
	int  num;
	struct label_cols *next_label;
};

#define COLS struct all_cols
COLS {
	MORWDLST *pats;
	struct label_cols *labelP;
	char isClitic;
	COLS *next_col;
};

struct kideval_words {
	char *word;
	struct kideval_words *left;
	struct kideval_words *right;
};

struct speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;
	float tUtt;
	float total;	// total number of words
	float diff;		// number of diff words
	float clause;
	float words, morf, mUtt, xxxut;
	float words100, morf100, mUtt100;
	float tdWords, tdUtts, tdur;
	float exla, intr, peri, ques, tbrk;
	float werr, uerr;
	float retr, repet;
	float dssTotal;
	float dssGTotal;
	float morTotal;	// total number of words on %mor tier
	DSSSP *dss_sp;
	VOCDSP *vocd_sp;
	double vocdOptimum;
	struct kideval_words *words_root;
	unsigned int *mor_count;
	struct speakers *next_sp;
} ;

struct IDparts {
	char *lang;	// @Languages:, @Language of #:
	char *corp;	// such as: MacWhinney, Bates, Sachs, etc...
	char *code;	// @Participants: *CHI
	char *ages;	// @Age of #:
	char *sex;	// @Sex of #:
	char *group;// @Group of #:
	char *SES;	// @Ses of #:
	char *role;	// @Participants: Target_Child
	char *edu;	// @Education of #:
	char *UNQ;	// file name or other unique ID
} ;

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct database {
	float num;
	float tUtt_sqr, tUtt, mn_tUtt, mx_tUtt;
	float total_sqr, total, mn_total, mx_total;	/* total number of words */
	float diff_sqr, diff, mn_diff, mx_diff;	/* number of diff words */
	float TTR_sqr, TTR, mn_TTR, mx_TTR;
	float CUR_sqr, CUR, mn_CUR, mx_CUR;
	float mUtt_sqr, mUtt, mn_mUtt, mx_mUtt;
	float words_sqr, words, mn_words, mx_words;
	float morf_sqr, morf, mn_morf, mx_morf;
	float mUtt100_sqr, mUtt100, mn_mUtt100, mx_mUtt100;
	float words100_sqr, words100, mn_words100, mx_words100;
	float morf100_sqr, morf100, mn_morf100, mx_morf100;
	float xxxut_sqr, xxxut, mn_xxxut, mx_xxxut;
	float tdWords_sqr, tdWords, mn_tdWords, mx_tdWords;
	float tdUtts_sqr, tdUtts, mn_tdUtts, mx_tdUtts;
	float tdur_sqr, tdur, mn_tdur, mx_tdur;
	float tdWT_sqr, tdWT, mn_tdWT, mx_tdWT;
	float tdUT_sqr, tdUT, mn_tdUT, mx_tdUT;
	float exla_sqr, exla, mn_exla, mx_exla;
	float intr_sqr, intr, mn_intr, mx_intr;
	float peri_sqr, peri, mn_peri, mx_peri;
	float ques_sqr, ques, mn_ques, mx_ques;
	float tbrk_sqr, tbrk, mn_tbrk, mx_tbrk;
	float werr_sqr, werr, mn_werr, mx_werr;
	float uerr_sqr, uerr, mn_uerr, mx_uerr;
	float retr_sqr, retr, mn_retr, mx_retr;
	float repet_sqr, repet, mn_repet, mx_repet;
	float dssTotal_sqr, dssTotal, mn_dssTotal, mx_dssTotal;
	float dssGTotal_sqr, dssGTotal, mn_dssGTotal, mx_dssGTotal;
	float vocdOptimum_sqr, vocdOptimum, mn_vocdOptimum, mx_vocdOptimum;
	float morTotal_sqr, morTotal, mn_morTotal, mx_morTotal;	/* total number of words on %mor tier */
	struct speakers *db_sp;
} ;

struct DBKeys {
	char *key1;
	char *key2;
	char *key3;
	char *key4;
	char *age;
	int  agef;
	int  aget;
	char *sex;
	struct DBKeys *next;
};

static int  targc;
static char **targv;
static int  kideval_SpecWords, DBGemNum;
static int  colLabelsNum;
static char *DBGems[NUMGEMITEMS], *script_file;
static char dssRulesFName[256];
static char isSpeakerNameGiven, isXXXFound, ftime, isDebug, isRawVal;
static char kideval_BBS[5], kideval_CBS[5], kideval_group, kideval_n_option, isNOptionSet, onlyApplyToDB, isGOptionSet, GemMode;
static struct DBKeys *DBKeyRoot;
static struct speakers *sp_head;
static struct database *db;
static COLS *colsRoot;
static struct label_cols *labelsRoot;

void usage() {
	printf("Usage: kideval [bS dS g gS lF n oN s %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(UNX) || defined(_MAC_CODE)
	puts("+d : debug.");
#endif
#ifdef _CLAN_DEBUG
	puts("+dS: specify database keyword(s) S. Choices are:");
	puts("    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control,");
	puts("    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),");
	puts("    Nonfluent = (Global, Broca, Transmotor),");
	puts("    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)");
#endif // _CLAN_DEBUG
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("-g : look for Gems in Database only");
	puts("+gS: select Gems which are labeled by label S");
	puts("+lF: specify language script file name F");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+o4: output raw values instead of percentage values");
	puts("+s : counts utterances and other words found along with \"xxx\"");
	mainusage(FALSE);
	puts("\nExamples:");
#ifdef _CLAN_DEBUG
	puts("   Search database for \"Breakfast\" Gems of \"control\" subjects between ages of 60 and 70");
	puts("       kideval *.cha +t*chi +gBreakfast +d\"control|60-70\"");
	puts("   Search database for \"Breakfast\" Gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       kideval *.cha +t*chi +gBreakfast +d\"control|60-70;6\"");
	puts("   Search database for \"Breakfast\" Gems of \"Nonfluent\" subjects of any age");
	puts("       kideval *.cha +t*chi +gBreakfast +dNonfluent");
	puts("   Just compute results for \"Breakfast\" Gems of adler15a.cha. Do not compare to database");
	puts("       kideval *.cha +t*chi +gBreakfast");
#else // _CLAN_DEBUG
	puts("   Compute results for \"Breakfast\" Gems of speaker *CHI");
	puts("       kideval +t*chi +gBreakfast +leng *.cha");
	puts("   Compute results only for speaker *CHI");
	puts("       kideval +t*chi +leng *.cha");
#endif // else _CLAN_DEBUG
	cutt_exit(0);
}

static void init_db(struct database *p) {
	p->num = 0.0;
	p->tUtt_sqr = 0.0; p->tUtt = 0.0; p->mn_tUtt = 0.0; p->mx_tUtt = 0.0;
	p->total_sqr = 0.0; p->total = 0.0; p->mn_total = 0.0; p->mx_total = 0.0;
	p->diff_sqr = 0.0; p->diff = 0.0; p->mn_diff = 0.0; p->mx_diff = 0.0;
	p->TTR_sqr = 0.0; p->TTR = 0.0; p->mn_TTR = 0.0; p->mx_TTR = 0.0;
	p->CUR_sqr = 0.0; p->CUR = 0.0; p->mn_CUR = 0.0; p->mx_CUR = 0.0;
	p->mUtt_sqr = 0.0; p->mUtt = 0.0; p->mn_mUtt = 0.0; p->mx_mUtt = 0.0;
	p->words_sqr = 0.0; p->words = 0.0; p->mn_words = 0.0; p->mx_words = 0.0;
	p->morf_sqr = 0.0; p->morf = 0.0; p->mn_morf = 0.0; p->mx_morf = 0.0;
	p->mUtt100_sqr = 0.0; p->mUtt100 = 0.0; p->mn_mUtt100 = 0.0; p->mx_mUtt100 = 0.0;
	p->words100_sqr = 0.0; p->words100 = 0.0; p->mn_words100 = 0.0; p->mx_words100 = 0.0;
	p->morf100_sqr = 0.0; p->morf100 = 0.0; p->mn_morf100 = 0.0; p->mx_morf100 = 0.0;
	p->xxxut_sqr = 0.0; p->xxxut = 0.0; p->mn_xxxut = 0.0; p->mx_xxxut = 0.0;
	p->tdWords_sqr = 0.0; p->tdWords = 0.0; p->mn_tdWords = 0.0; p->mx_tdWords = 0.0;
	p->tdUtts_sqr = 0.0; p->tdur = 0.0; p->mn_tdUtts = 0.0; p->mx_tdur = 0.0;
	p->tdur_sqr = 0.0; p->tdUtts = 0.0; p->mn_tdur = 0.0; p->mx_tdUtts = 0.0;
	p->tdWT_sqr = 0.0; p->tdWT = 0.0; p->mn_tdWT = 0.0; p->mx_tdWT = 0.0;
	p->tdUT_sqr = 0.0; p->tdUT = 0.0; p->mn_tdUT = 0.0; p->mx_tdUT = 0.0;
	p->exla_sqr = 0.0; p->exla = 0.0; p->mn_exla = 0.0; p->mx_exla = 0.0;
	p->intr_sqr = 0.0; p->intr = 0.0; p->mn_intr = 0.0; p->mx_intr = 0.0;
	p->peri_sqr = 0.0; p->peri = 0.0; p->mn_peri = 0.0; p->mx_peri = 0.0;
	p->ques_sqr = 0.0; p->ques = 0.0; p->mn_ques = 0.0; p->mx_ques = 0.0;
	p->tbrk_sqr = 0.0; p->tbrk = 0.0; p->mn_tbrk = 0.0; p->mx_tbrk = 0.0;
	p->werr_sqr = 0.0; p->werr = 0.0; p->mn_werr = 0.0; p->mx_werr = 0.0;
	p->uerr_sqr = 0.0; p->uerr = 0.0; p->mn_uerr = 0.0; p->mx_uerr = 0.0;
	p->retr_sqr = 0.0; p->retr = 0.0; p->mn_retr = 0.0; p->mx_retr = 0.0;
	p->repet_sqr = 0.0; p->repet = 0.0; p->mn_repet = 0.0; p->mx_repet = 0.0;
	p->dssGTotal_sqr = 0.0; p->dssGTotal = 0.0; p->mn_dssGTotal = 0.0; p->mx_dssGTotal = 0.0;
	p->dssTotal_sqr = 0.0; p->dssTotal = 0.0; p->mn_dssTotal = 0.0; p->mx_dssTotal = 0.0;
	p->vocdOptimum_sqr = 0.0; p->vocdOptimum = 0.0; p->mn_vocdOptimum = 0.0; p->mx_vocdOptimum = 0.0;
	p->morTotal_sqr = 0.0; p->morTotal = 0.0; p->mn_morTotal = 0.0; p->mx_morTotal = 0.0;
	p->db_sp = NULL;
}

static struct label_cols *freeLabels(struct label_cols *p) {
	struct label_cols *t;

	while (p != NULL) {
		t = p;
		p = p->next_label;
		if (t->label)
			free(t->label);
		free(t);
	}
	return(NULL);
}

static COLS *freeColsP(COLS *p) {
	COLS *t;

	while (p != NULL) {
		t = p;
		p = p->next_col;
		t->pats = freeMorWords(t->pats);
		free(t);
	}
	return(NULL);
}

static void freeFreqTree(struct kideval_words *p) {
	if (p != NULL) {
		freeFreqTree(p->left);
		freeFreqTree(p->right);
		if (p->word != NULL)
			free(p->word);
		free(p);
	}
}

static struct speakers *freespeakers(struct speakers *p) {
	struct speakers *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
		if (ts->words_root != NULL)
			freeFreqTree(ts->words_root);
		if (ts->mor_count != NULL)
			free(ts->mor_count);
		free(ts);
	}
	return(NULL);
}

static struct database *freedb(struct database *db) {
	if (db != NULL) {
		db->db_sp = freespeakers(db->db_sp);
		free(db);
	}
	return(NULL);
}

static struct DBKeys *freeDBKeys(struct DBKeys *p) {
	struct DBKeys *t;

	while (p) {
		t = p;
		p = p->next;
		if (t->key1 != NULL)
			free(t->key1);
		if (t->key2 != NULL)
			free(t->key2);
		if (t->key3 != NULL)
			free(t->key3);
		if (t->key4 != NULL)
			free(t->key4);
		if (t->age != NULL)
			free(t->age);
		if (t->sex != NULL)
			free(t->sex);
		free(t);
	}
	return(NULL);
}

static void kideval_error(struct database *db, char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	labelsRoot = freeLabels(labelsRoot);
	colsRoot = freeColsP(colsRoot);
	db = freedb(db);
	dss_freeSpeakers();
	dss_cleanSearchMem();
	speakers = speaker_free_up_speakers(speakers, TRUE);
	if (VOCD_TYPE_D)
		denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
	cutt_exit(0);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		kideval_error(NULL, FALSE);
	}
	DBGems[DBGemNum] = gem;
	DBGemNum++;
}

static int excludeGemKeywords(char *word) {
	int i;

	if (word[0] == '+' || strcmp(word, "!") == 0 || strcmp(word, "?") == 0 || strcmp(word, ".") == 0)
		return(FALSE);
	for (i=0; i < DBGemNum; i++) {
		if (uS.mStricmp(DBGems[i], word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static char isAge(char *b, int *agef, int *aget) {
	int  ageNum;
	char *s;

	for (s=b; *s != EOS; s++) {
		if (!isdigit(*s) && *s != ';' && *s != '.' && *s != '~' && *s != '-' && *s != ' ')
			return(FALSE);
	}
	*agef = 0;
	*aget = 0;
	for (s=b; isSpace(*s) || *s == '~'; s++) ;
	if (*s == EOS)
		return(TRUE);
	ageNum = atoi(s) * 12;
	for (; *s != ';' && *s != '-' && *s != EOS; s++) ;
	if (*s == ';') {
		s++;
		if (isdigit(*s))
			ageNum = ageNum + atoi(s);
		for (; *s != '-' && *s != EOS; s++) ;
	}
	*agef = ageNum;
	if (*s == EOS)
		return(TRUE);
	for (s++; isSpace(*s) || *s == '~'; s++) ;
	ageNum = atoi(s) * 12;
	for (; *s != ';' && *s != '-' && *s != EOS; s++) ;
	if (*s == ';') {
		s++;
		if (isdigit(*s))
			ageNum = ageNum + atoi(s);
		else
			ageNum = ageNum + 11;
	} else
		ageNum = ageNum + 11;
	*aget = ageNum;
	return(TRUE);
}

#ifdef _CLAN_DEBUG
static char *kideval_strsave(const char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		kideval_error(NULL, TRUE);
	return(p);
}

static struct DBKeys *initDBKey(const char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		kideval_error(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = kideval_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = kideval_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = kideval_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = kideval_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = kideval_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = kideval_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = kideval_strsave("Wernicke");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("TransSensory",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Conduction",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Anomic",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("NotAphasicByWAB",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"Nonfluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = kideval_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = kideval_strsave("Wernicke");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("TransSensory",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Conduction",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Anomic",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Global",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("NotAphasicByWAB",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} 
}

static char isRightKeyword(char *b) {
	if (uS.mStricmp(b,"Anomic") == 0 || uS.mStricmp(b,"Global") == 0 || uS.mStricmp(b,"Broca") == 0 ||
		uS.mStricmp(b,"Wernicke") == 0 || uS.mStricmp(b,"TransSensory") == 0 || uS.mStricmp(b,"TransMotor") == 0 ||
		uS.mStricmp(b,"Conduction") == 0 || uS.mStricmp(b,"Fluent") == 0 || uS.mStricmp(b,"Nonfluent") == 0 ||
		uS.mStricmp(b,"AllAphasia") == 0 ||uS.mStricmp(b,"NotAphasicByWAB") == 0 ||uS.mStricmp(b,"control") == 0) {
		return(TRUE);
	}
	return(FALSE);
}

static void addDBKeys(char *key) {
	char *b, *e, isFound;
	struct DBKeys *DBKey, *cDBKey;

	DBKey = initDBKey(NULL, NULL, NULL, NULL, NULL, 0, 0, NULL);
	isFound = FALSE;
	strcpy(templineC4, key);
	b = templineC4;
	do {
		e = strchr(b, '|');
		if (e != NULL) {
			*e = EOS;
			e++;
		}
		uS.remFrontAndBackBlanks(b);
		if (*b != EOS) {
			if (isRightKeyword(b)) {
				if (DBKey->key1 == NULL)
					DBKey->key1 = kideval_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = kideval_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = kideval_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = kideval_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = kideval_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					kideval_error(NULL, FALSE);
				}
				DBKey->age = kideval_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control, \n");
				fprintf(stderr, "    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),\n");
				fprintf(stderr, "    Nonfluent = (Global, Broca, Transmotor),\n");
				fprintf(stderr, "    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)\n");
				kideval_error(NULL, FALSE);
			}
			isFound = TRUE;
		}
		b = e;
	} while (b != NULL) ;
	if (isFound) {
		if (DBKeyRoot == NULL) {
			DBKeyRoot = DBKey;
		} else {
			for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
			cDBKey->next = DBKey;
		}
		multiplyDBKeys(DBKey);
	}
}
#endif // _CLAN_DEBUG

static int isEqualKey(char *DBk, char *IDk) {
	if (DBk == NULL || IDk == NULL) {
		return(1);
	}
	return(uS.mStricmp(DBk, IDk));
}

static char isKeyMatch(struct IDparts *IDt) {
	int  cAgef, cAget, numSpec, numMatched;
	struct DBKeys *DBKey;

	for (DBKey=DBKeyRoot; DBKey != NULL; DBKey=DBKey->next) {
		numSpec = 0;
		numMatched = 0;
		if (DBKey->key1 != NULL) {
			numSpec++;
			if (isEqualKey(DBKey->key1, IDt->lang) == 0 || isEqualKey(DBKey->key1, IDt->corp) == 0  ||
				isEqualKey(DBKey->key1, IDt->code) == 0 || isEqualKey(DBKey->key1, IDt->group) == 0 ||
				isEqualKey(DBKey->key1, IDt->SES) == 0  || isEqualKey(DBKey->key1, IDt->role) == 0  ||
				isEqualKey(DBKey->key1, IDt->edu) == 0  || isEqualKey(DBKey->key1, IDt->UNQ) == 0)
				numMatched++;
		}
		if (DBKey->key2 != NULL) {
			numSpec++;
			if (isEqualKey(DBKey->key2, IDt->lang) == 0 || isEqualKey(DBKey->key2, IDt->corp) == 0  ||
				isEqualKey(DBKey->key2, IDt->code) == 0 || isEqualKey(DBKey->key2, IDt->group) == 0 ||
				isEqualKey(DBKey->key2, IDt->SES) == 0  || isEqualKey(DBKey->key2, IDt->role) == 0  ||
				isEqualKey(DBKey->key2, IDt->edu) == 0  || isEqualKey(DBKey->key2, IDt->UNQ) == 0)
				numMatched++;
		}
		if (DBKey->key3 != NULL) {
			numSpec++;
			if (isEqualKey(DBKey->key3, IDt->lang) == 0 || isEqualKey(DBKey->key3, IDt->corp) == 0  ||
				isEqualKey(DBKey->key3, IDt->code) == 0 || isEqualKey(DBKey->key3, IDt->group) == 0 ||
				isEqualKey(DBKey->key3, IDt->SES) == 0  || isEqualKey(DBKey->key3, IDt->role) == 0  ||
				isEqualKey(DBKey->key3, IDt->edu) == 0  || isEqualKey(DBKey->key3, IDt->UNQ) == 0)
				numMatched++;
		}
		if (DBKey->key4 != NULL) {
			numSpec++;
			if (isEqualKey(DBKey->key4, IDt->lang) == 0 || isEqualKey(DBKey->key4, IDt->corp) == 0  ||
				isEqualKey(DBKey->key4, IDt->code) == 0 || isEqualKey(DBKey->key4, IDt->group) == 0 ||
				isEqualKey(DBKey->key4, IDt->SES) == 0  || isEqualKey(DBKey->key4, IDt->role) == 0  ||
				isEqualKey(DBKey->key4, IDt->edu) == 0  || isEqualKey(DBKey->key4, IDt->UNQ) == 0)
				numMatched++;
		}
		if (DBKey->age != NULL) {
			numSpec++;
			isAge(IDt->ages, &cAgef, &cAget);
			if (cAget != 0) {
				if ((DBKey->agef <= cAgef && cAgef <= DBKey->aget) ||
					(DBKey->agef <= cAget && cAget <= DBKey->aget))
					numMatched++;
			} else {
				if (DBKey->agef <= cAgef && cAgef <= DBKey->aget)
					numMatched++;
			}
		}
		if (DBKey->sex != NULL) {
			numSpec++;
			if (isEqualKey(DBKey->sex, IDt->sex) == 0)
				numMatched++;
		}
		if (numSpec == numMatched)
			return(TRUE);
	}
	return(FALSE);
}

static struct speakers *kideval_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	int i;
	struct speakers *ts, *tsp;

	if (db == NULL)
		tsp = sp_head;
	else
		tsp = db->db_sp;
	uS.remblanks(sp);
	for (ts=tsp; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
#ifdef JUST_ONE_SPEAKER
			fprintf(stderr, "\nERROR: More than 1 speaker selected in a file: %s\n", fname);
			kideval_error(db, FALSE);
#endif // JUST_ONE_SPEAKER
		}
	}
	if ((ts=NEW(struct speakers)) == NULL)
		kideval_error(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		free(ts);
		kideval_error(db, TRUE);
	}	
	if (db == NULL) {
		if (sp_head == NULL) {
			sp_head = ts;
		} else {
			for (tsp=sp_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
			tsp->next_sp = ts;
		}
		ts->next_sp = NULL;
	} else {
		ts->next_sp = db->db_sp;
		db->db_sp = ts;
	}
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		kideval_error(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			kideval_error(db, TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;
	ts->tUtt	= 0.0;
	ts->total   = 0.0;
	ts->diff    = 0.0;
	ts->clause	= 0.0;
	ts->words   = 0.0;
	ts->morf	= 0.0;
	ts->mUtt	= 0.0;
	ts->words100= 0.0;
	ts->morf100	= 0.0;
	ts->mUtt100	= 0.0;
	ts->xxxut   = 0.0;
	ts->tdWords	= 0.0;
	ts->tdUtts	= 0.0;
	ts->tdur	= 0.0;
	ts->exla    = 0.0;
	ts->intr    = 0.0;
	ts->peri    = 0.0;
	ts->ques    = 0.0;
	ts->tbrk    = 0.0;
	ts->werr    = 0.0;
	ts->uerr    = 0.0;
	ts->retr    = 0.0;
	ts->repet   = 0.0;
	ts->dss_sp  = NULL;
	ts->dssTotal= 0.0;
	ts->dssGTotal= 0.0;
	ts->vocd_sp  = NULL;
	ts->vocdOptimum = 0.0;
	ts->words_root = NULL;
	ts->morTotal   = 0.0;
	if (colLabelsNum > 0) {
		ts->mor_count = (unsigned int *)malloc(sizeof(unsigned int) * colLabelsNum);
		if (ts->mor_count == NULL) {
			kideval_error(db, TRUE);
		}
		for (i=0; i < colLabelsNum; i++)
			ts->mor_count[i] = 0;
	} else
		ts->mor_count = NULL;
	return(ts);
}

static void breakIDsIntoFields(struct IDparts *idTier, char *IDs) {
	char *s;

	for (s=IDs; isSpace(*s); s++) ;
	idTier->lang = s;
	idTier->corp = NULL;
	idTier->code = NULL;
	idTier->ages = NULL;
	idTier->sex = NULL;
	idTier->group = NULL;
	idTier->SES = NULL;
	idTier->role = NULL;
	idTier->edu = NULL;
	idTier->UNQ = NULL;
	if ((s=strchr(idTier->lang, '|')) != NULL) {
		*s++ = EOS;
		idTier->corp = s;
		if ((s=strchr(idTier->corp, '|')) != NULL) {
			*s++ = EOS;
			idTier->code = s;
			if ((s=strchr(idTier->code, '|')) != NULL) {
				*s++ = EOS;
				idTier->ages = s;
				if ((s=strchr(idTier->ages, '|')) != NULL) {
					*s++ = EOS;
					idTier->sex = s;
					if ((s=strchr(idTier->sex, '|')) != NULL) {
						*s++ = EOS;
						idTier->group = s;
						if ((s=strchr(idTier->group, '|')) != NULL) {
							*s++ = EOS;
							idTier->SES = s;
							if ((s=strchr(idTier->SES, '|')) != NULL) {
								*s++ = EOS;
								idTier->role = s;
								if ((s=strchr(idTier->role, '|')) != NULL) {
									*s++ = EOS;
									idTier->edu = s;
									if ((s=strchr(idTier->edu, '|')) != NULL) {
										*s++ = EOS;
										idTier->UNQ = s;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

static void dealWithDiscontinuousWord(char *word, int i) {
	if (word[strlen(word)-1] == '-') {
		while ((i=getword(utterance->speaker, uttline, templineC4, NULL, i))) {
			if (templineC4[0] == '-' && !uS.isToneUnitMarker(templineC4)) {
				strcat(word, templineC4+1);
				break;
			}
		}
	}
}

static char isUttDel(char *line, int pos) {
	if (line[pos] == '?' && line[pos+1] == '|')
		;
	else if (uS.IsUtteranceDel(line, pos)) {
		if (!uS.atUFound(line, pos, &dFnt, MBF))
			return(TRUE);
	}
	return(FALSE);
}

static char isRightWord(char *word) {
	if (uS.mStricmp(word, "xxx") == 0       || uS.mStricmp(word, "yyy") == 0       || uS.mStrnicmp(word, "www", 3) == 0 ||
		uS.mStrnicmp(word, "xxx@s", 5) == 0 || uS.mStrnicmp(word, "yyy@s", 5) == 0 || uS.mStrnicmp(word, "www@s", 5) == 0 ||
		word[0] == '+' || word[0] == '-' || word[0] == '!' || word[0] == '?' || word[0] == '.')
		return(FALSE);
	return(TRUE);
}

static struct kideval_words *kideval_talloc(char *word, struct database *db) {
	struct kideval_words *p;

	if ((p=NEW(struct kideval_words)) == NULL)
		kideval_error(db, TRUE);
	p->left = p->right = NULL;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		kideval_error(db, TRUE);
	}
	return(p);
}

static struct kideval_words *kideval_tree(struct kideval_words *p, char *w, struct speakers *ts, struct database *db) {
	int cond;
	struct kideval_words *t = p;

	if (p == NULL) {
		ts->diff++;
		p = kideval_talloc(w, db);
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0) {
		p->left = kideval_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0) {
			p->left = kideval_tree(p->left, w, ts, db);
		} else {
			p->right = kideval_tree(p->right, w, ts, db); /* if cond > 0 */
		}
		return(t);
	}
	return(p);
}

static int excludePlusAndUttDels(char *word) {
	if (word[0] == '+' || strcmp(word, "!") == 0 || strcmp(word, "?") == 0 || strcmp(word, ".") == 0)
		return(FALSE);
	return(TRUE);
}

static char kideval_mlu_excludeUtter(char *line, int pos, char *isWordsFound, char *isXXXItem) { // xxx, yyy, www
	int  i, j;
	char isSkipFirst = FALSE;

	if (MBF) {
		if (my_CharacterByteType(line, (short)pos, &dFnt) != 0 ||
			my_CharacterByteType(line, (short)pos+1, &dFnt) != 0 ||
			my_CharacterByteType(line, (short)pos+2, &dFnt) != 0)
			isSkipFirst = TRUE;
	}
	if (!isSkipFirst) {
		i = 0;
		if (pos == 0 || uS.isskip(line,pos-1,&dFnt,MBF)) {
			for (j=pos; line[j] == 'x' || line[j] == 'X' ||
				   line[j] == 'y' || line[j] == 'Y' ||
				   line[j] == 'w' || line[j] == 'W' || 
				   line[j] == '(' || line[j] == ')'; j++) {
				if (line[j] != '(' && line[j] != ')')
					templineC2[i++] = line[j];
			}
		}
		if (i == 3) {
			if (isSpace(line[j]) || line[j] == '@')
				templineC2[i] = EOS;
			else
				templineC2[0] = EOS;
		} else
			templineC2[i] = EOS;
		uS.lowercasestr(templineC2, &dFnt, FALSE);
		if (isXXXFound && strcmp(templineC2, "xxx") == 0) {
			if (isWordsFound == NULL && (CntWUT == 2 || CntWUT == 3)) {
			} else {
				line[pos] = ' ';
				line[pos+1] = ' ';
				line[pos+2] = ' ';
			}
			if (isXXXItem != NULL)
				*isXXXItem = TRUE;
			if (isWordsFound != NULL)
				*isWordsFound = TRUE;
		} else if (strcmp(templineC2, "xxx") == 0) {
			if (isXXXItem != NULL)
				*isXXXItem = TRUE;
//			if (uS.isskip(line,pos+3,&dFnt,MBF) || line[pos+3] == EOS)
				return(TRUE);
		} else if (strcmp(templineC2, "yyy") == 0 ||
				   strcmp(templineC2, "www") == 0) {
//			if (uS.isskip(line,pos+3,&dFnt,MBF) || line[pos+3] == EOS)
				return(TRUE);
		}
	}
	if (chatmode) {
		if (uS.partcmp(utterance->speaker,morTierName,FALSE,FALSE)) {
			if (MBF) {
				if (my_CharacterByteType(line, (short)pos, &dFnt)   != 0 ||
					my_CharacterByteType(line, (short)pos+1, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+2, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+3, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+4, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+5, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+6, &dFnt) != 0)
					return(FALSE);
			}
			if (!uS.isskip(line,pos+7,&dFnt,MBF) && line[pos+7] != EOS)
				return(FALSE);
			strncpy(templineC2, line+pos, 7);
			templineC2[7] = EOS;
			uS.lowercasestr(templineC2, &dFnt, FALSE);
			if (isXXXFound && strcmp(templineC2, "unk|xxx") == 0) {
				if (isWordsFound == NULL && (CntWUT == 2 || CntWUT == 3)) {
				} else {
					line[pos] = ' ';
					line[pos+1] = ' ';
					line[pos+2] = ' ';
					line[pos+3] = ' ';
					line[pos+4] = ' ';
					line[pos+5] = ' ';
					line[pos+6] = ' ';
				}
				if (isXXXItem != NULL)
					*isXXXItem = TRUE;
				if (isWordsFound != NULL)
					*isWordsFound = TRUE;
			} else if (strcmp(templineC2, "unk|xxx") == 0) {
				if (isXXXItem != NULL)
					*isXXXItem = TRUE;
				return(TRUE);
			} else if (strcmp(templineC2, "unk|yyy") == 0 ||
					   strcmp(templineC2, "unk|www") == 0)
				return(TRUE);
		}
	}
	return(FALSE);	
}

static int countWord(char *line, int pos) {
	int  i, sqb, agb, num;
	char spf;

	for (i=pos-1; i >= 0 && (isSpace(line[i]) || line[i] == '\n'); i--) ;
	if (i < 0)
		return(0);
	sqb = 0;
	for (; i >= 0; i--) {
		if (line[i] == '[')
			sqb--;
		else if (line[i] == ']')
			sqb++;
		else if (sqb == 0 && (line[i] == '>' || !uS.isskip(line,i,&dFnt,MBF)))
			break;
	}
	if (i < 0)
		return(0);
	if (line[i] == '>') {
		num = 0;
		agb = 0;
		sqb = 0;
		spf = TRUE;
		for (; i >= 0; i--) {
			if (line[i] == '[') {
				sqb--;
				spf = TRUE;
			} else if (line[i] == ']') {
				sqb++;
				spf = TRUE;
			} else if (sqb == 0) {
				if (line[i] == '<') {
					agb--;
					spf = TRUE;
					if (agb <= 0)
						break;
				} else if (line[i] == '>') {
					agb++;
					spf = TRUE;
				} else if (!uS.isskip(line,i,&dFnt,MBF)) {
					if (spf)
						num++;
					spf = FALSE;
				} else
					spf = TRUE;
			}
		}
		if (i < 0)
			return(0);
		else
			return(num);
	} else {
		return(1);
	}
}

static void kideval_process_tier(struct speakers *ts, struct database *db, char *rightIDFound) {
	int i, j, comps, num;
	char word[1024], tword[1024], *w[NUMCOMPS], isMatchFound;
	char tmp, isWordsFound, isWordsFound100, sq, aq, isSkip;
	char isPSDFound, curPSDFound, isXXXItem, isAuxFound, isAmbigFound;
	long stime, etime, duration;
	float words, morf, mUtt, words100, morf100, mUtt100;
	struct IDparts IDTier;
	COLS *col;
	MORFEATS word_feats, *feat;

	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return;
		strcpy(spareTier1, utterance->line);
		for (i=0; spareTier1[i] != EOS; i++) {
			if (spareTier1[i] == HIDEN_C && isdigit(spareTier1[i+1])) {
				if (getMediaTagInfo(spareTier1+i, &stime, &etime)) {
					duration = (etime - stime)  / 1000;
					ts->tdur = ts->tdur + (float)duration;
				}
			}
		}
		ts->tdUtts = ts->tdUtts + 1;
		R6_override = TRUE;
		filterData("*", spareTier1);
		R6_override = FALSE;
		i = 0;
		while ((i=getword("*", spareTier1, word, NULL, i))) {
			if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
				continue;
			if (exclude(word) && isRightWord(word)) {
				ts->tdWords = ts->tdWords + 1;
			}
		}			
		strcpy(spareTier1, utterance->line);
		for (i=0; utterance->line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->mUtt100 > 0.0)
					ts->mUtt100--;
				if (ts->mUtt > 0.0)
					ts->mUtt--;
				ts->isPSDFound = FALSE;
			}
		}
		i = 0;
		while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
			if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
				continue;
			dealWithDiscontinuousWord(word, i);
			if (exclude(word) && isRightWord(word)) {
				uS.remblanks(word);
				ts->total++;
				ts->words_root = kideval_tree(ts->words_root, word, ts, db);
			}
		}
		while ((i=getword(utterance->speaker, utterance->line, word, NULL, i))) {
			if (!strcmp(word, "[//]"))
				ts->retr++;
			else if (!strcmp(word, "[/]"))
				ts->repet++;
		}
	} else if (uS.partcmp(utterance->speaker,morTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return;
		filterwords("%", uttline, excludePlusAndUttDels);
		i = 0;
		while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
			uS.remblanks(word);
			if (strchr(word, '|') != NULL) {
				ts->morTotal++;
				strcpy(tword, word);
				for (j=0; tword[j] != EOS; j++) {
					if (tword[j] == '~')
						ts->morTotal++;
				}
				comps = 0;
				w[comps++] = tword;
				for (j=0; tword[j] != EOS; j++) {
					if (tword[j] == '~' || tword[j] == '$') {
						tword[j] = EOS;
						j++;
						if (comps >= NUMCOMPS) {
							fprintf(stderr, "    Intenal Error: # of pre/post clitics is > %d in word: \"%s\"\n", NUMCOMPS-1, word);
							sp_head = freespeakers(sp_head);
							DBKeyRoot = freeDBKeys(DBKeyRoot);
							labelsRoot = freeLabels(labelsRoot);
							colsRoot = freeColsP(colsRoot);
							db = freedb(db);
							dss_freeSpeakers();
							dss_cleanSearchMem();
							speakers = speaker_free_up_speakers(speakers, TRUE);
							if (VOCD_TYPE_D)
								denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
							cutt_exit(0);
						}
						w[comps++] = tword+j;
					}
				}
				for (j=0; j < comps; j++) {
					for (col=colsRoot; col != NULL; col=col->next_col) {
						if (col->labelP == NULL || col->labelP->const_label_num > 0) {
							;
						} else if (col->isClitic) {
							isMatchFound = isMorPatMatchedWord(col->pats, word);
							if (isMatchFound == 2)
								kideval_error(db, TRUE);
							if (j == 0 && isMatchFound) {
								if (col->labelP != NULL)
									ts->mor_count[col->labelP->num] = ts->mor_count[col->labelP->num] + 1;
							}
						} else {
							isMatchFound = isMorPatMatchedWord(col->pats, w[j]);
							if (isMatchFound == 2)
								kideval_error(db, TRUE);
							if (isMatchFound) {
								if (col->labelP != NULL)
									ts->mor_count[col->labelP->num] = ts->mor_count[col->labelP->num] + 1;
							}
						}
					}
				}
			}
		}
		ts->isMORFound = TRUE;
		words100	= ts->words100;
		morf100		= ts->morf100;
		mUtt100		= ts->mUtt100;
		words		= ts->words;
		morf		= ts->morf;
		mUtt		= ts->mUtt;
		isPSDFound	= ts->isPSDFound;
		curPSDFound = FALSE;
		isXXXItem = FALSE;
		isWordsFound = FALSE;
		isWordsFound100 = FALSE;
		sq = FALSE;
		aq = FALSE;
		isSkip = FALSE;
		for (i=0; spareTier1[i] != EOS; i++) {
			if (kideval_mlu_excludeUtter(spareTier1, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
				isSkip = TRUE;
				break;
			}
		}
		i = 0;
		do {
			if (kideval_mlu_excludeUtter(uttline, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
				isSkip = TRUE;
				break;
			}
			if (uS.isRightChar(uttline, i, '[', &dFnt, MBF)) {
				sq = TRUE;
			} else if (uS.isRightChar(uttline, i, ']', &dFnt, MBF))
				sq = FALSE;
			else {
				if (uS.isRightChar(uttline, i, '<', &dFnt, MBF)) {
					int tl;
					for (tl=i+1; !uS.isRightChar(uttline, tl, '>', &dFnt, MBF) && uttline[tl]; tl++) {
						if (isdigit(uttline[tl])) ;
						else if (uttline[tl]== ' ' || uttline[tl]== '\t' || uttline[tl]== '\n') ;
						else if ((tl-1 == i+1 || !isalpha(uttline[tl-1])) && 
								 uS.isRightChar(uttline, tl, '-', &dFnt, MBF) && !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == i+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'u', &dFnt, MBF) || uS.isRightChar(uttline,tl,'U', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == i+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'w', &dFnt, MBF) || uS.isRightChar(uttline,tl,'W', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == i+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'s', &dFnt, MBF) || uS.isRightChar(uttline,tl,'S', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else
							break;
					}
					if (uS.isRightChar(uttline, tl, '>', &dFnt, MBF)) aq = TRUE;
				} else
					if (uS.isRightChar(uttline, i, '>', &dFnt, MBF)) aq = FALSE;
			}
			if (!uS.isskip(uttline,i,&dFnt,MBF) && !sq && !aq) {
				isAmbigFound = FALSE;
				isWordsFound = TRUE;
				isWordsFound100 = TRUE;
				tmp = TRUE;
				words = words + 1;
				words100 = words100 + 1;
				while (uttline[i]) {
					if (uttline[i] == '^')
						isAmbigFound = TRUE;
					if (uS.isskip(uttline,i,&dFnt,MBF)) {
						if (uS.IsUtteranceDel(utterance->line, i)) {
							if (!uS.atUFound(utterance->line, i, &dFnt, MBF))
								break;
						} else
							break;
					}
					if (!uS.ismorfchar(uttline, i, &dFnt, rootmorf, MBF) && !isAmbigFound) {
						if (tmp) {
							if (uttline[i] != EOS) {
								if (i >= 2 && uttline[i-1] == '+' && uttline[i-2] == '|')
									;
								else {
									morf = morf + 1;
									morf100 = morf100 + 1;
//									localmorf = localmorf + 1;
								}
							}
							tmp = FALSE;
						}
					} else
						tmp = TRUE;
					i++;
				}
			}
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && isPSDFound) {
				if (mUtt100 > 0.0)
					mUtt100--;
				else if (mUtt100 == 0.0) 
					isWordsFound100 = FALSE;
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (mUtt100 > 0.0)
					mUtt100--;
				else if (mUtt100 == 0.0) 
					isWordsFound100 = FALSE;
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))
				curPSDFound = TRUE;
			if (isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound100) {
					mUtt100 = mUtt100 + (float)1.0;
					isWordsFound100 = FALSE;
				}
				if (isWordsFound) {
					mUtt = mUtt + (float)1.0;
					isWordsFound = FALSE;
				}
			}
			if (uttline[i])
				i++;
			else
				break;
		} while (uttline[i]) ;
		if (!isSkip) {
			if (mUtt >= 25 && mUtt100 <= 100) {
				ts->words100	= words100;
				ts->morf100		= morf100;
				ts->mUtt100		= mUtt100;
			}
			ts->words	   = words;
			ts->morf	   = morf;
			ts->mUtt	   = mUtt;
			ts->isPSDFound = curPSDFound;
		}
		if (!isSkip || isXXXItem) {
			isXXXItem = FALSE;
			for (i=0; spareTier1[i] != EOS; i++) {
				if (spareTier1[i] == '[' && spareTier1[i+1] == '*' && (spareTier1[i+2] == ']' || spareTier1[i+2] == ' ')) {
					num = countWord(spareTier1, i);
					if (num > 0)
						ts->werr += num;
					else
						ts->werr++;
				}
			}
			i = 0;
			while ((i=getword("*", spareTier1, word, NULL, i))) {
				if (!strcmp(word, "xxx") || !strncmp(word, "xxx@s", 5)) {
					if (!isXXXItem)
						ts->xxxut++;
					isXXXItem = TRUE;
				}
				if (uS.isSqCodes(word, tword, &dFnt, FALSE)) {
					if (!strcmp(tword, "[+ *]")   || !strcmp(tword, "[+ jar]") || !strcmp(tword, "[+ es]") ||
						!strcmp(tword, "[+ cir]") || !strcmp(tword, "[+ per]") || !strcmp(tword, "[+ gram]")) {
						ts->uerr++;
					}
				} else if (!strcmp(word, "!")) {
					ts->exla++;
				} else if (!strcmp(word, "+/.") || !strcmp(word, "+//.") || !strcmp(word, "+/?") ||
						   !strcmp(word, "+//?") || !strcmp(word, "+...") || !strcmp(word, "+..?")) {
					ts->intr++;
				} else if (!strcmp(word, ".") || !strcmp(word, "+\"/.") || !strcmp(word, "+\".")) {
					ts->peri++;
				} else if (!strcmp(word, "?") || !strcmp(word, "+!?")) {
					ts->ques++;
				} else if (!strcmp(word, "+.")) {
					ts->tbrk++;
				}
			}
			isAuxFound = FALSE;
			i = 0;
			while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
				word_feats.type = NULL;
				word_feats.typeID = 'R';
				if (ParseWordIntoFeatures(templineC2, &word_feats) == FALSE)
					kideval_error(db, TRUE);
				for (feat=&word_feats; feat != NULL; feat=feat->comp) {
					if (feat->type != NULL && feat->typeID == '+')
						;
					else if (isAllv(feat) || isnEqual("cop", feat->pos, 3)) {
						if (isEqual("part", feat->pos)) {
							if (isAuxFound)
								ts->clause++;
						} else if (isEqual("v", feat->pos) || isnEqual("cop", feat->pos, 3))
							ts->clause++;
					} else if (isEqual("aux", feat->pos)) {
						isAuxFound = TRUE;
					} else if (isEqual("part", feat->pos)) {
						if (isAuxFound)
							ts->clause++;
					}
				}
				freeUpFeats(&word_feats);
			}
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
		if (db == NULL) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				uS.remblanks(utterance->line);
				kideval_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
			}
		} else if (rightIDFound != NULL) {
			uS.remblanks(utterance->line);
			breakIDsIntoFields(&IDTier, utterance->line);
			if (isKeyMatch(&IDTier))  {
				*rightIDFound = TRUE;
				maketierchoice(IDTier.code, '+', '\002');
			}
		}
	}
}

static char kideval_process_dss_tier(struct speakers *ts, struct database *db, char lRightspeaker) {
	int k;
	DSSSP *dss_sp;

	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return(lRightspeaker);
		dss_sp = dss_FindSpeaker(ts->sp, NULL);
		if (dss_sp == NULL)
			kideval_error(NULL, TRUE);
		if (isUttsLimit(dss_sp)) {
			spareTier1[0] = EOS;
			return(FALSE);
		}
		cleanUttline(uttline);
		if (strcmp(uttline, spareTier1) == 0) {
			spareTier1[0] = EOS;
			return(FALSE);
		}
		if (!isDSSRepeatUtt(dss_sp, uttline)) {
			if (!addToDSSUtts(dss_sp, uttline)) {
				kideval_error(db, TRUE);
			}
			strcpy(spareTier1, uttline);
			strcpy(spareTier2, utterance->line);
		} else {
			spareTier1[0] = EOS;
			lRightspeaker = FALSE;
		}
	} else if (uS.partcmp(utterance->speaker,morTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return(lRightspeaker);
		dss_sp = dss_FindSpeaker(ts->sp, NULL);
		if (dss_sp == NULL)
			kideval_error(NULL, TRUE);
		ts->dss_sp = dss_sp;
		for (k=0; uttline[k] != EOS; k++) {
			if (uttline[k] == '~')
				uttline[k] = ' ';
		}
		if (!make_DSS_tier(uttline))
			kideval_error(db, FALSE);
		if(PassedDSSMorTests(spareTier2,uttline,templineC3)) {
			dss_sp->TotalNum = dss_sp->TotalNum + 1;
			for (k=0; spareTier2[k] != EOS; k++) {
				if (spareTier2[k]== '\n' || spareTier2[k]== '\t')
					spareTier2[k] = ' ';
			}
			for (k--; spareTier2[k] == ' '; k--) ;
			spareTier2[++k] = EOS;
			dss_sp->GrandTotal = dss_sp->GrandTotal + PrintOutDSSTable(spareTier2, FALSE);
		} else {
			dss_sp = dss_FindSpeaker(ts->sp, NULL);
			if (dss_sp == NULL)
				kideval_error(NULL, TRUE);
			freeLastDSSUtt(dss_sp);
		}
		spareTier1[0] = EOS;
	}
	return(lRightspeaker);
}

static void kideval_process_vocd_tier(struct speakers *ts, struct database *db) {
	int i;
	VOCDSP *vocd_sp;
	
	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return;
		isDoVocdAnalises = TRUE;
		if ((vocd_sp=speaker_add_speaker(&speakers, ts->sp, NULL, TRUE, TRUE)) == NULL) {
			fprintf(stderr,"cannot add speaker to list\n");
			kideval_error(db, FALSE);
		}
		ts->vocd_sp = vocd_sp;
		if (!nomain && vocd_sp != NULL) {
			uS.remblanks(uttline);
			for (i=0; uttline[i] != EOS; i++) {
				if (uS.isskip(uttline,i,&dFnt,MBF))
					uttline[i] = ' ';
			}
			if (!speaker_add_line(vocd_sp, uttline, lineno)) {
				fprintf(stderr,"cannot add line to current speaker\n");
				kideval_error(db, FALSE);
			} 
		}				
	}
/*
	else if (nomain && uS.partcmp(utterance->speaker,morTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return;
		isDoVocdAnalises = TRUE;
		if ((vocd_sp=speaker_add_speaker(&speakers, ts->sp, NULL, TRUE, TRUE)) == NULL) {
			fprintf(stderr,"cannot add speaker to list\n");
			kideval_error(db, FALSE);
		}
		ts->vocd_sp = vocd_sp;
		if (vocd_sp != NULL) {
			uS.remblanks(uttline);
			for (i=0; uttline[i] != EOS; i++) {
				if (uS.isskip(uttline,i,&dFnt,MBF))
					uttline[i] = ' ';
			}
			if (!speaker_add_line(vocd_sp, uttline, lineno)) {
				fprintf(stderr,"cannot add line to current speaker\n");
				kideval_error(db, FALSE);
			}
		}
	}
*/
}

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i') 
		return((kideval_group == FALSE && found) || (kideval_SpecWords == found));
	else 
		return((kideval_group == TRUE && kideval_SpecWords > found) || (found == 0));
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  found, t;
	char isfirst, isOutputGem, rightIDFound;
	char word[BUFSIZ];
	float tt;
	FILE *tfpin;
	struct speakers *ts;
	struct IDparts IDTier;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	isfirst = TRUE;
	if (isNOptionSet == FALSE) {
		strcpy(kideval_BBS, "@*&#");
		strcpy(kideval_CBS, "@*&#");
	}
	if (kideval_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	spareTier1[0] = EOS;
	rightIDFound = FALSE;
	found = 0;
	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (templineC[0] == '+') {
			uS.remblanks(templineC+1);
			breakIDsIntoFields(&IDTier, templineC+1);
			if (!isKeyMatch(&IDTier))  {
				while (fgets_cr(templineC, UTTLINELEN, fpin)) {
					if (templineC[0] == '-') {
						break;
					}
				}
				rightIDFound = FALSE;
			} else
				rightIDFound = TRUE;
		} else if (templineC[0] == '-') {
			if (db->db_sp != NULL) {
				*isDbSpFound = TRUE;
				ts = db->db_sp;
				if (!ts->isSpeakerFound) {
					if (kideval_SpecWords)
						fprintf(stderr,"\nERROR: No specified Gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					kideval_error(db, FALSE);
				}
				if (isXXXFound)
					ts->tUtt = ts->mUtt;
				else
					ts->tUtt = ts->xxxut + ts->mUtt;
				db->tUtt_sqr = db->tUtt_sqr + (ts->tUtt * ts->tUtt);
				db->tUtt = db->tUtt + ts->tUtt;
				if (db->mn_tUtt == (float)0.0 || db->mn_tUtt > ts->tUtt)
					db->mn_tUtt = ts->tUtt;
				if (db->mx_tUtt < ts->tUtt)
					db->mx_tUtt = ts->tUtt;
				db->total_sqr = db->total_sqr + (ts->total * ts->total);
				db->total = db->total + ts->total;
				if (db->mn_total == (float)0.0 || db->mn_total > ts->total)
					db->mn_total = ts->total;
				if (db->mx_total < ts->total)
					db->mx_total = ts->total;
				db->diff_sqr = db->diff_sqr + (ts->diff * ts->diff);
				db->diff = db->diff + ts->diff;
				if (db->mn_diff == (float)0.0 || db->mn_diff > ts->diff)
					db->mn_diff = ts->diff;
				if (db->mx_diff < ts->diff)
					db->mx_diff = ts->diff;
				if (ts->total == 0.0)
					tt = 0.0;
				else
					tt = ts->diff / ts->total;
				db->TTR_sqr = db->TTR_sqr + (tt * tt);
				db->TTR = db->TTR + tt;
				if (db->mn_TTR == (float)0.0 || db->mn_TTR > tt)
					db->mn_TTR = tt;
				if (db->mx_TTR < tt)
					db->mx_TTR = tt;
				if (ts->tUtt == 0.0)
					tt = 0.0;
				else
					tt = ts->clause / ts->tUtt;
				db->CUR_sqr = db->CUR_sqr + (tt * tt);
				db->CUR = db->CUR + tt;
				if (db->mn_CUR == (float)0.0 || db->mn_CUR > tt)
					db->mn_CUR = tt;
				if (db->mx_CUR < tt)
					db->mx_CUR = tt;
				db->mUtt_sqr = db->mUtt_sqr + (ts->mUtt * ts->mUtt);
				db->mUtt = db->mUtt + ts->mUtt;
				if (db->mn_mUtt == (float)0.0 || db->mn_mUtt > ts->mUtt)
					db->mn_mUtt = ts->mUtt;
				if (db->mx_mUtt < ts->mUtt)
					db->mx_mUtt = ts->mUtt;
				ts->words = ts->words / ts->mUtt;
				db->words_sqr = db->words_sqr + (ts->words * ts->words);
				db->words = db->words + ts->words;
				if (db->mn_words == (float)0.0 || db->mn_words > ts->words)
					db->mn_words = ts->words;
				if (db->mx_words < ts->words)
					db->mx_words = ts->words;
				ts->morf = ts->morf / ts->mUtt;
				db->morf_sqr = db->morf_sqr + (ts->morf * ts->morf);
				db->morf = db->morf + ts->morf;
				if (db->mn_morf == (float)0.0 || db->mn_morf > ts->morf)
					db->mn_morf = ts->morf;
				if (db->mx_morf < ts->morf)
					db->mx_morf = ts->morf;
				db->mUtt100_sqr = db->mUtt100_sqr + (ts->mUtt100 * ts->mUtt100);
				db->mUtt100 = db->mUtt100 + ts->mUtt100;
				if (db->mn_mUtt100 == (float)0.0 || db->mn_mUtt100 > ts->mUtt100)
					db->mn_mUtt100 = ts->mUtt100;
				if (db->mx_mUtt100 < ts->mUtt100)
					db->mx_mUtt100 = ts->mUtt100;
				ts->words100 = ts->words100 / ts->mUtt100;
				db->words100_sqr = db->words100_sqr + (ts->words100 * ts->words100);
				db->words100 = db->words100 + ts->words100;
				if (db->mn_words100 == (float)0.0 || db->mn_words100 > ts->words100)
					db->mn_words100 = ts->words100;
				if (db->mx_words100 < ts->words100)
					db->mx_words100 = ts->words100;
				ts->morf100 = ts->morf100 / ts->mUtt100;
				db->morf100_sqr = db->morf100_sqr + (ts->morf100 * ts->morf100);
				db->morf100 = db->morf100 + ts->morf100;
				if (db->mn_morf100 == (float)0.0 || db->mn_morf100 > ts->morf100)
					db->mn_morf100 = ts->morf100;
				if (db->mx_morf100 < ts->morf100)
					db->mx_morf100 = ts->morf100;
				if (ts->tUtt == 0.0)
					ts->xxxut = 0.0;
				else
					ts->xxxut = (100 - (ts->xxxut * 100) / ts->tUtt);
				db->xxxut_sqr = db->xxxut_sqr + (ts->xxxut * ts->xxxut);
				db->xxxut = db->xxxut + ts->xxxut;
				if (db->mn_xxxut == (float)0.0 || db->mn_xxxut > ts->xxxut)
					db->mn_xxxut = ts->xxxut;
				if (db->mx_xxxut < ts->xxxut)
					db->mx_xxxut = ts->xxxut;
				db->tdWords_sqr = db->tdWords_sqr + (ts->tdWords * ts->tdWords);
				db->tdWords = db->tdWords + ts->tdWords;
				if (db->mn_tdWords == (float)0.0 || db->mn_tdWords > ts->tdWords)
					db->mn_tdWords = ts->tdWords;
				if (db->mx_tdWords < ts->tdWords)
					db->mx_tdWords = ts->tdWords;
				db->tdUtts_sqr = db->tdUtts_sqr + (ts->tdUtts * ts->tdUtts);
				db->tdUtts = db->tdUtts + ts->tdUtts;
				if (db->mn_tdUtts == (float)0.0 || db->mn_tdUtts > ts->tdUtts)
					db->mn_tdUtts = ts->tdUtts;
				if (db->mx_tdUtts < ts->tdUtts)
					db->mx_tdUtts = ts->tdUtts;
				db->tdur_sqr = db->tdur_sqr + (ts->tdur * ts->tdur);
				db->tdur = db->tdur + ts->tdur;
				if (db->mn_tdur == (float)0.0 || db->mn_tdur > ts->tdur)
					db->mn_tdur = ts->tdur;
				if (db->mx_tdur < ts->tdur)
					db->mx_tdur = ts->tdur;
				if (ts->total == 0.0)
					tt = 0.0;
				else
					tt = ts->tdWords / ts->tdur;
				db->tdWT_sqr = db->tdWT_sqr + (tt * tt);
				db->tdWT = db->tdWT + tt;
				if (db->mn_tdWT == (float)0.0 || db->mn_tdWT > tt)
					db->mn_tdWT = tt;
				if (db->mx_tdWT < tt)
					db->mx_tdWT = tt;
				if (ts->total == 0.0)
					tt = 0.0;
				else
					tt = ts->tdUtts / ts->tdur;
				db->tdUT_sqr = db->tdUT_sqr + (tt * tt);
				db->tdUT = db->tdUT + tt;
				if (db->mn_tdUT == (float)0.0 || db->mn_tdUT > tt)
					db->mn_tdUT = tt;
				if (db->mx_tdUT < tt)
					db->mx_tdUT = tt;
				db->exla_sqr = db->exla_sqr + (ts->exla * ts->exla);
				db->exla = db->exla + ts->exla;
				if (db->mn_exla == (float)0.0 || db->mn_exla > ts->exla)
					db->mn_exla = ts->exla;
				if (db->mx_exla < ts->exla)
					db->mx_exla = ts->exla;
				db->intr_sqr = db->intr_sqr + (ts->intr * ts->intr);
				db->intr = db->intr + ts->intr;
				if (db->mn_intr == (float)0.0 || db->mn_intr > ts->intr)
					db->mn_intr = ts->intr;
				if (db->mx_intr < ts->intr)
					db->mx_intr = ts->intr;
				db->peri_sqr = db->peri_sqr + (ts->peri * ts->peri);
				db->peri = db->peri + ts->peri;
				if (db->mn_peri == (float)0.0 || db->mn_peri > ts->peri)
					db->mn_peri = ts->peri;
				if (db->mx_peri < ts->peri)
					db->mx_peri = ts->peri;
				db->ques_sqr = db->ques_sqr + (ts->ques * ts->ques);
				db->ques = db->ques + ts->ques;
				if (db->mn_ques == (float)0.0 || db->mn_ques > ts->ques)
					db->mn_ques = ts->ques;
				if (db->mx_ques < ts->ques)
					db->mx_ques = ts->ques;
				db->tbrk_sqr = db->tbrk_sqr + (ts->tbrk * ts->tbrk);
				db->tbrk = db->tbrk + ts->tbrk;
				if (db->mn_tbrk == (float)0.0 || db->mn_tbrk > ts->tbrk)
					db->mn_tbrk = ts->tbrk;
				if (db->mx_tbrk < ts->tbrk)
					db->mx_tbrk = ts->tbrk;
				if (!isRawVal) {
					if (ts->total == 0.0)
						ts->werr = 0.0;
					else
						ts->werr = (ts->werr / ts->total) * 100.0000;
				}
				db->werr_sqr = db->werr_sqr + (ts->werr * ts->werr);
				db->werr = db->werr + ts->werr;
				if (db->mn_werr == (float)0.0 || db->mn_werr > ts->werr)
					db->mn_werr = ts->werr;
				if (db->mx_werr < ts->werr)
					db->mx_werr = ts->werr;
				db->uerr_sqr = db->uerr_sqr + (ts->uerr * ts->uerr);
				db->uerr = db->uerr + ts->uerr;
				if (db->mn_uerr == (float)0.0 || db->mn_uerr > ts->uerr)
					db->mn_uerr = ts->uerr;
				if (db->mx_uerr < ts->uerr)
					db->mx_uerr = ts->uerr;
				db->retr_sqr = db->retr_sqr + (ts->retr * ts->retr);
				db->retr = db->retr + ts->retr;
				if (db->mn_retr == (float)0.0 || db->mn_retr > ts->retr)
					db->mn_retr = ts->retr;
				if (db->mx_retr < ts->retr)
					db->mx_retr = ts->retr;
				db->repet_sqr = db->repet_sqr + (ts->repet * ts->repet);
				db->repet = db->repet + ts->repet;
				if (db->mn_repet == (float)0.0 || db->mn_repet > ts->repet)
					db->mn_repet = ts->repet;
				if (db->mx_repet < ts->repet)
					db->mx_repet = ts->repet;
				db->db_sp = freespeakers(db->db_sp);
				db->dssTotal_sqr = db->dssTotal_sqr + (ts->dssTotal * ts->dssTotal);
				db->dssTotal = db->dssTotal + ts->dssTotal;
				if (db->mn_dssTotal == (float)0.0 || db->mn_dssTotal > ts->dssTotal)
					db->mn_dssTotal = ts->dssTotal;
				if (db->mx_dssTotal < ts->dssTotal)
					db->mx_dssTotal = ts->dssTotal;
				ts->dssGTotal = ts->dssGTotal / ts->dssTotal;
				db->dssGTotal_sqr = db->dssGTotal_sqr + (ts->dssGTotal * ts->dssGTotal);
				db->dssGTotal = db->dssGTotal + ts->dssGTotal;
				if (db->mn_dssGTotal == (float)0.0 || db->mn_dssGTotal > ts->dssGTotal)
					db->mn_dssGTotal = ts->dssGTotal;
				if (db->mx_dssGTotal < ts->dssGTotal)
					db->mx_dssGTotal = ts->dssGTotal;
				db->vocdOptimum_sqr = db->vocdOptimum_sqr + (ts->vocdOptimum * ts->vocdOptimum);
				db->vocdOptimum = db->vocdOptimum + ts->vocdOptimum;
				if (db->mn_vocdOptimum == (float)0.0 || db->mn_vocdOptimum > ts->vocdOptimum)
					db->mn_vocdOptimum = ts->vocdOptimum;
				if (db->mx_vocdOptimum < ts->vocdOptimum)
					db->mx_vocdOptimum = ts->vocdOptimum;
				db->morTotal_sqr = db->morTotal_sqr + (ts->morTotal * ts->morTotal);
				db->morTotal = db->morTotal + ts->morTotal;
				if (db->mn_morTotal == (float)0.0 || db->mn_morTotal > ts->morTotal)
					db->mn_morTotal = ts->morTotal;
				if (db->mx_morTotal < ts->morTotal)
					db->mx_morTotal = ts->morTotal;
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(kideval_BBS, "@*&#");
				strcpy(kideval_CBS, "@*&#");
			}
			if (kideval_SpecWords) {
				isOutputGem = FALSE;
			} else {
				isOutputGem = TRUE;
			}
			rightIDFound = FALSE;
			found = 0;
		} else {
			if (templineC[0] == '*') {
				strcpy(utterance->speaker, "*PAR:");
				strcpy(utterance->line, templineC+1);
			} else if (templineC[0] == '%') {
				strcpy(utterance->speaker, morTierName);
				strcpy(utterance->line, templineC+1);
			} else if (templineC[0] == 'G') {
				strcpy(utterance->speaker, "@G:");
				strcpy(utterance->line, templineC+1);
			} else if (templineC[0] == 'B') {
				strcpy(utterance->speaker, "@BG:");
				strcpy(utterance->line, templineC+1);
			} else if (templineC[0] == 'E') {
				strcpy(utterance->speaker, "@EG:");
				strcpy(utterance->line, templineC+1);
			} else if (templineC[0] == 'T') {
				strcpy(utterance->speaker, "@Time Duration:");
				strcpy(utterance->line, templineC+1);
			}
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			filterData(utterance->speaker,uttline);
			if (kideval_SpecWords && !strcmp(kideval_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					kideval_n_option = FALSE;
					strcpy(kideval_BBS, "@BG:");
					strcpy(kideval_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					kideval_n_option = TRUE;
					strcpy(kideval_BBS, "@G:");
					strcpy(kideval_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,kideval_BBS,FALSE,FALSE)) {
				if (kideval_n_option) {
					if (isRightText(word)) {
						isOutputGem = TRUE;
					} else
						isOutputGem = FALSE;
				} else {
					if (isRightText(word)) {
						found++;
						if (found == 1 || GemMode != '\0') {
							isOutputGem = TRUE;
						}
					}
				}
			} else if (found > 0 && uS.partcmp(utterance->speaker,kideval_CBS,FALSE,FALSE)) {
				if (kideval_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (kideval_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			} else if (isOutputGem) {
				if (utterance->speaker[0] == '*') {
					strcpy(templineC, utterance->speaker);
					ts = kideval_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
					if (ts != NULL && isfirst) {
						isfirst = FALSE;
						db->num = db->num + 1.0;
						t = db->num;
						if (t == 1 || t % 15 == 0)
							fprintf(stderr,"\r%.0f ", db->num);
					}
				}
				kideval_process_tier(ts, db, NULL);
			}
		}
	}
	fpin = tfpin;
}

static void ParseDatabase(struct database *db) {
	char   isDbSpFound = FALSE;
	FILE *fp;

	if ((fp=OpenGenLib(DATABASE_FILE_NAME,"r",TRUE,TRUE,FileName1)) != NULL) {
		fprintf(stderr,"Database File: %s\n", FileName1);
		OpenDBFile(fp, db, &isDbSpFound);
		fprintf(stderr,"\r%.0f  \n", db->num);
	} else {
		strcpy(FileName2, wd_dir);
		addFilename2Path(FileName2, DATABASE_FILE_NAME);
		fprintf(stderr, "Can't find database in folder \"%s\" or \"%s\"\n", FileName1, FileName2);
		fprintf(stderr, "If you do not use database, then do not include +d option in command line\n");
		kideval_error(db, FALSE);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (kideval_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified Gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		kideval_error(db, FALSE);
	}	
	SetNewVol(wd_dir);
}

static void compute_SD(struct SDs *SD, float score, float *div, float sqr_mean, float mean, float num) {
	float cSD;

	if (!SD->isSDComputed) {
		if (num < 2.0) {
			SD->dbSD = 0.0;
			SD->isSDComputed = 2;
		} else {
			SD->dbSD = (sqr_mean - ((mean * mean) / num)) / (num - 1);
			if (SD->dbSD < 0.0)
				SD->dbSD = -SD->dbSD;
			SD->dbSD = sqrt(SD->dbSD);
			SD->isSDComputed = 1;
		}
	}
	if (SD->dbSD == 0.0 || (div != NULL && *div == 0.0)) {
		fprintf(fpout, "\tN/A");
		SD->stars = 0;
	} else {
		if (div != NULL && *div != 0.0)
			score = score / (*div);
		mean = mean / num; 
		cSD = (score - mean) / SD->dbSD;
		fprintf(fpout, "\t%.3f", cSD);
		if (abs(cSD) >= 2.0)
			SD->stars = 2;
		else if (abs(cSD) >= 1.0)
			SD->stars = 1;
		else
			SD->stars = 0;
	}
}

static void printConstLabel(int const_label_num) {
	if (const_label_num == 1)
		fprintf(fpout,"\tTotal Utts");
	else if (const_label_num == 2)
		fprintf(fpout,"\tMLU Utts");
	else if (const_label_num == 3)
		fprintf(fpout,"\tMLU Words");
	else if (const_label_num == 4)
		fprintf(fpout,"\tMLU Morphemes");
	else if (const_label_num == 5)
		fprintf(fpout,"\tMLU100 Utts");
	else if (const_label_num == 6)
		fprintf(fpout,"\tMLU100 Words");
	else if (const_label_num == 7)
		fprintf(fpout,"\tMLU100 Morphemes");
	else if (const_label_num == 8)
		fprintf(fpout,"\ttypes");
	else if (const_label_num == 9)
		fprintf(fpout,"\ttokens");
	else if (const_label_num == 10)
		fprintf(fpout,"\tTTR");
	else if (const_label_num == 11)
		fprintf(fpout,"\tClause/Utt");
	else if (const_label_num == 12)
		fprintf(fpout,"\tDur Words");
	else if (const_label_num == 13)
		fprintf(fpout,"\tDur Utts");
	else if (const_label_num == 14)
		fprintf(fpout,"\tDur Time (secs)");
	else if (const_label_num == 15)
		fprintf(fpout,"\tWords/Time");
	else if (const_label_num == 16)
		fprintf(fpout,"\tUtts/Time");
	else if (const_label_num == 17) {
		if (isRawVal) {
			fprintf(fpout,"\tWord Errors");
		} else {
			fprintf(fpout,"\t%% Word Errors");
		}
	} else if (const_label_num == 18) {
		fprintf(fpout,"\tUtt Errors");
	} else if (const_label_num == 19) {
		fprintf(fpout,"\tretracing[//]");
	} else if (const_label_num == 20) {
		fprintf(fpout,"\trepetition[/]");
	} else if (const_label_num == 21) {
		fprintf(fpout, "\tDSS Utts");
	} else if (const_label_num == 22) {
		fprintf(fpout, "\tDSS");
	} else if (const_label_num == 23) {
		fprintf(fpout, "\tVOCD D_optimum_average");
	} else if (const_label_num == 24) {
		fprintf(fpout, "\tmor Words");
	}
}

static void kideval_pr_result(void) {
	int    i, cnt, SDn;
	char   *sFName;
	float  num;
	struct speakers *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;
	struct label_cols *col;

	if (sp_head == NULL) {
		if (kideval_SpecWords && !onlyApplyToDB) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified Gems found for this speaker\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n\n");
	}
	if (DBKeyRoot != NULL) {
		for (i=0; i < SDRESSIZE; i++) {
			SD[i].isSDComputed = FALSE;
		}
		fprintf(fpout,"File/DB");
	} else
		fprintf(fpout,"File");
	fprintf(fpout,"\tLanguage\tCorpus\tCode\tAge\tSex\tGroup\tSES\tRole\tEducation\tCustom_field");
	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (col->isDisplay) {
			if (col->label != NULL)
				fprintf(fpout,"\t%s", col->label);
			else
				printConstLabel(col->const_label_num);
		}
	}
	fprintf(fpout, "\n");
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (kideval_SpecWords) {
				fprintf(stderr,"\nERROR: No specified Gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			} else
				fprintf(stderr,"\nERROR: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
		sFName = strrchr(ts->fname, PATHDELIMCHR);
		if (sFName != NULL)
			sFName = sFName + 1;
		else
			sFName = ts->fname;
		fprintf(fpout,"%s",sFName);
		if (ts->ID) {
			cnt = 0;
			for (i=0; ts->ID[i] != EOS; i++) {
				if (ts->ID[i] == '|') {
					cnt++;
					if (cnt < 10)
						ts->ID[i] = '\t';
					else
						ts->ID[i] = EOS;
				}
			}
			fprintf(fpout,"\t%s", ts->ID);
		} else
			fprintf(fpout,"\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.", ts->sp);
		if (!ts->isMORFound) {
			fprintf(stderr,"\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
			fprintf(stderr,"WARNING: Speaker \"%s\" has no \"%s\" tiers.\n\n", ts->sp, morTierName);
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				if (col->const_label_num >= 1) {
					fprintf(fpout,"\t0");
				} else {
					if (col->isDisplay)
						fprintf(fpout,"\t0");
				}
			}
			fprintf(fpout, "\n");
			continue;
		}
		if (isXXXFound)
			ts->tUtt = ts->mUtt;
		else
			ts->tUtt = ts->xxxut + ts->mUtt;
		for (col=labelsRoot; col != NULL; col=col->next_label) {
			if (col->const_label_num == 1) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->tUtt);
			} else if (col->const_label_num == 2) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->mUtt);
			} else if (col->const_label_num == 3) {
				if (col->isDisplay) {
					if (ts->mUtt == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->words/ts->mUtt);
				}
			} else if (col->const_label_num == 4) {
				if (col->isDisplay) {
					if (ts->mUtt == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->morf/ts->mUtt);
				}
			} else if (col->const_label_num == 5) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->mUtt100);
			} else if (col->const_label_num == 6) {
				if (col->isDisplay) {
					if (ts->mUtt100 == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->words100/ts->mUtt100);
				}
			} else if (col->const_label_num == 7) {
				if (col->isDisplay) {
					if (ts->mUtt100 == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->morf100/ts->mUtt100);
				}
			} else if (col->const_label_num == 8) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->diff);
			} else if (col->const_label_num == 9) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->total);
			} else if (col->const_label_num == 10) {
				if (col->isDisplay) {
					if (ts->total == 0.0)
						fprintf(fpout,"\tN/A");
					else
						fprintf(fpout,"\t%.3f",ts->diff/ts->total);
				}
			} else if (col->const_label_num == 11) {
				if (col->isDisplay) {
					if (ts->tUtt == 0.0)
						fprintf(fpout,"\tN/A");
					else
						fprintf(fpout,"\t%.3f",ts->clause/ts->tUtt);
				}
			} else if (col->const_label_num == 12) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->tdWords);
			} else if (col->const_label_num == 13) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->tdUtts);
			} else if (col->const_label_num == 14) {
				if (col->isDisplay)
					fprintf(fpout,"\t%.0f", ts->tdur);
			} else if (col->const_label_num == 15) {
				if (col->isDisplay) {
					if (ts->tdur == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->tdWords/ts->tdur);
				}
			} else if (col->const_label_num == 16) {
				if (col->isDisplay) {
					if (ts->tdur == 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.3f",ts->tdUtts/ts->tdur);
				}
			} else if (col->const_label_num == 17) {
				if (col->isDisplay) {
					if (!isRawVal) {
						if (ts->total == 0.0)
							ts->werr = 0.000;
						else
							ts->werr = (ts->werr / ts->total) * 100.0000;
						fprintf(fpout, "\t%.3f", ts->werr);
					} else
						fprintf(fpout, "\t%.0f",ts->werr);
				}
			} else if (col->const_label_num == 18) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->uerr);
			} else if (col->const_label_num == 19) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->retr);
			} else if (col->const_label_num == 20) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->repet);
			} else if (col->const_label_num == 21) {
				if (col->isDisplay) {
					fprintf(fpout, "\t%.0f", ts->dssTotal);
				}
			} else if (col->const_label_num == 22) {
				if (col->isDisplay) {
					num = ts->dssGTotal;
					if (ts->dssTotal <= 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.2f", num / ts->dssTotal);
				}
			} else if (col->const_label_num == 23) {
				if (col->isDisplay) {
					if (ts->vocdOptimum <= 0.0)
						fprintf(fpout, "\tN/A");
					else
						fprintf(fpout, "\t%.2f", ts->vocdOptimum);
				}
			} else if (col->const_label_num == 24) {
				if (col->isDisplay)
					fprintf(fpout, "\t%.0f",ts->morTotal);
			} else if (ts->mor_count == NULL) {
				fprintf(fpout, "\t.");
			} else {
				if (col->isDisplay) {
					if (!isRawVal) {
						num = (float)ts->mor_count[col->num];
						if (ts->morTotal == 0.0)
							fprintf(fpout, "\t%.3f", 0.000);
						else
							fprintf(fpout, "\t%.3f", (num/ts->morTotal)*100.0000);
					} else
						fprintf(fpout,"\t%u", ts->mor_count[col->num]);
				}
			}
		}
		fprintf(fpout, "\n");
		if (DBKeyRoot != NULL) {
			num = db->num;
			fprintf(fpout,"+/-SD\t.\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
			SDn = 0; // this number should be less than SDRESSIZE = 256
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				if (col->const_label_num == 1) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tUtt,  NULL, db->tUtt_sqr, db->tUtt, num);
				} else if (col->const_label_num == 2) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->mUtt,  NULL, db->mUtt_sqr, db->mUtt, num);
				} else if (col->const_label_num == 3) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->words,  &ts->mUtt, db->words_sqr, db->words, num);
				} else if (col->const_label_num == 4) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->morf,  &ts->mUtt, db->morf_sqr, db->morf, num);
				} else if (col->const_label_num == 5) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->mUtt100,  NULL, db->mUtt100_sqr, db->mUtt100, num);
				} else if (col->const_label_num == 6) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->words100,  &ts->mUtt100, db->words100_sqr, db->words100, num);
				} else if (col->const_label_num == 7) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->morf100,  &ts->mUtt100, db->morf100_sqr, db->morf100, num);
				} else if (col->const_label_num == 8) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->diff,  NULL, db->diff_sqr, db->diff, num);
				} else if (col->const_label_num == 9) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->total,  NULL, db->total_sqr, db->total, num);
				} else if (col->const_label_num == 10) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->diff,  &ts->total, db->TTR_sqr, db->TTR, num);
				} else if (col->const_label_num == 11) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->clause,  &ts->tUtt, db->CUR_sqr, db->CUR, num);
				} else if (col->const_label_num == 12) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tdWords,  NULL, db->tdWords_sqr, db->tdWords, num);
				} else if (col->const_label_num == 13) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tdUtts,  NULL, db->tdUtts_sqr, db->tdUtts, num);
				} else if (col->const_label_num == 14) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tdur,  NULL, db->tdur_sqr, db->tdur, num);
				} else if (col->const_label_num == 15) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tdWords,  &ts->tdur, db->tdWT_sqr, db->tdWT, num);
				} else if (col->const_label_num == 16) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->tdUtts,  &ts->tdur, db->tdUT_sqr, db->tdUT, num);
				} else if (col->const_label_num == 17) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->werr,  NULL, db->werr_sqr, db->werr, num);
				} else if (col->const_label_num == 18) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->uerr,  NULL, db->uerr_sqr, db->uerr, num);
				} else if (col->const_label_num == 19) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->retr,  NULL, db->retr_sqr, db->retr, num);
				} else if (col->const_label_num == 20) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->repet,  NULL, db->repet_sqr, db->repet, num);
				} else if (col->const_label_num == 21) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->dssTotal,  NULL, db->dssTotal_sqr, db->dssTotal, num);
				} else if (col->const_label_num == 22) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->dssGTotal,  &ts->dssTotal, db->dssGTotal_sqr, db->dssGTotal, num);
				} else if (col->const_label_num == 23) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->vocdOptimum,  NULL, db->vocdOptimum_sqr, db->vocdOptimum, num);
				} else if (col->const_label_num == 24) {
					if (col->isDisplay)
						compute_SD(&SD[SDn++], ts->morTotal,  NULL, db->morTotal_sqr, db->morTotal, num);
				}
			}
			fprintf(fpout, "\n");
			fprintf(fpout,"\t");
			for (i=0; i < SDn; i++)
				fprintf(fpout, "\t%c%c",((SD[i].stars >= 1) ? '*' : ' '),((SD[i].stars >= 2) ? '*' : ' '));
			fprintf(fpout, "\n");
		}
	}
	if (DBKeyRoot != NULL) {
		num = db->num;
		fprintf(fpout,"Mean Database\t.\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
		fprintf(fpout, "\t%.3f",db->tUtt/num);
		fprintf(fpout, "\t%.3f",db->mUtt/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->words/num,db->morf/num);
		fprintf(fpout, "\t%.3f",db->mUtt100/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->words100/num,db->morf100/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->diff/num,db->total/num);
		fprintf(fpout, "\t%.3f",db->TTR/num);
		fprintf(fpout, "\t%.3f",db->CUR/num);
		fprintf(fpout, "\t%.3f\t%.3f\t%.0f\t%.3f\t%.3f",db->tdWords/num,db->tdUtts/num,db->tdur/num,db->tdWT/num,db->tdUT/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->werr/num,db->uerr/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->retr/num,db->repet/num);
		fprintf(fpout, "\t%.3f",db->dssGTotal/num);
		fprintf(fpout, "\t%.3f",db->morTotal/num);
		fprintf(fpout, "\n");

		fprintf(fpout,"Min Database\t.\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
		fprintf(fpout, "\t%.0f",db->mn_tUtt);
		fprintf(fpout, "\t%.0f",db->mn_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mn_words,db->mn_morf);
		fprintf(fpout, "\t%.0f",db->mn_mUtt100);
		fprintf(fpout, "\t%.3f\t%.3f",db->mn_words100,db->mn_morf100);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_diff,db->mn_total);
		fprintf(fpout, "\t%.3f",db->mn_TTR);
		fprintf(fpout, "\t%.3f",db->mn_CUR);
		fprintf(fpout,"\t%.3f\t%.3f\t%.0f\t%.3f\t%.3f",db->mn_tdWords,db->mn_tdUtts,db->mn_tdur,db->mn_tdWT,db->mn_tdUT);
		if (!isRawVal)
			fprintf(fpout, "\t%.3f\t%.0f",db->mn_werr,db->mn_uerr);
		else
			fprintf(fpout, "\t%.0f\t%.0f",db->mn_werr,db->mn_uerr);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_retr,db->mn_repet);
		fprintf(fpout, "\t%.3f",db->mn_dssGTotal);
		fprintf(fpout, "\t%.0f",db->mn_morTotal);
		fprintf(fpout, "\n");

		fprintf(fpout,"Max Database\t.\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
		fprintf(fpout, "\t%.0f",db->mx_tUtt);
		fprintf(fpout, "\t%.0f",db->mx_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mx_words,db->mx_morf);
		fprintf(fpout, "\t%.0f",db->mx_mUtt100);
		fprintf(fpout, "\t%.3f\t%.3f",db->mx_words100,db->mx_morf100);
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_diff,db->mx_total);
		fprintf(fpout, "\t%.3f",db->mx_TTR);
		fprintf(fpout, "\t%.3f",db->mx_CUR);
		fprintf(fpout,"\t%.3f\t%.3f\t%.0f\t%.3f\t%.3f",db->mx_tdWords,db->mx_tdUtts,db->mx_tdur,db->mx_tdWT,db->mx_tdUT);
		if (!isRawVal)
			fprintf(fpout, "\t%.3f\t%.0f",db->mx_werr,db->mx_uerr);
		else
			fprintf(fpout, "\t%.0f\t%.0f",db->mx_werr,db->mx_uerr);
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_retr,db->mx_repet);
		fprintf(fpout, "\t%.3f",db->mx_dssGTotal);
		fprintf(fpout, "\t%.0f",db->mx_morTotal);
		fprintf(fpout, "\n");

		fprintf(fpout,"SD Database\t.\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
		for (i=0; i < SDn; i++) {
			if (SD[i].isSDComputed == 1) {
				fprintf(fpout, "\t%.3f", SD[i].dbSD);
			} else {
				fprintf(fpout, "\tN/A");
			}
		}
		fprintf(fpout, "\n");
	}
	fprintf(fpout, "\n");
	if (sp_head != NULL) {
		if (DBKeyRoot != NULL)
			fprintf(fpout, "+/- SD  * = 1 SD, ** = 2 SD\n");
		if (DBGemNum) {
			fprintf(fpout, "Database Gems:");
			for (i=0; i < DBGemNum; i++) {
				if (i == 0)
					fprintf(fpout, " %s", DBGems[i]);
				else
					fprintf(fpout, ", %s", DBGems[i]);
			}
			fprintf(fpout, "\n");
		}
		if (DBKeyRoot != NULL) {
			fprintf(fpout, "Database keywords:");
			for (DBKey=DBKeyRoot; DBKey != NULL; DBKey=DBKey->next) {
				templineC4[0] = EOS;
				if (DBKey->key1 != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key1);
				}
				if (DBKey->key2 != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key2);
				}
				if (DBKey->key3 != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key3);
				}
				if (DBKey->key4 != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key4);
				}
				if (DBKey->age != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->age);
				}
				if (DBKey->sex != NULL) {
					if (templineC4[0] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->sex);
				}
				if (DBKey == DBKeyRoot)
					fprintf(fpout, " %s", templineC4);
				else
					fprintf(fpout, ", %s", templineC4);
			}
			fprintf(fpout, "\n");
			fprintf(fpout, "# files in database: %.0f\n",db->num);
			fprintf(fpout, "\n");
		}
//		printArg(targv, targc, fpout, FALSE, "");
	}
	sp_head = freespeakers(sp_head);
}

static char isFoundGems(void) {
	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (uS.partcmp(templineC,"@BG:",FALSE,FALSE) ||
			uS.partcmp(templineC,"@G:",FALSE,FALSE)) {
			rewind(fpin);
			return(TRUE);
		}
	}
	rewind(fpin);
	return(FALSE);
}
static void init_words(int command) {
	if (command == BASE_COM) {
		addword('\0','\0',"+</>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
//		addword('\0','\0',"+xxx");
//		addword('\0','\0',"+yyy");
//		addword('\0','\0',"+www");
//		addword('\0','\0',"+xxx@s*");
//		addword('\0','\0',"+yyy@s*");
//		addword('\0','\0',"+www@s*");
//		addword('\0','\0',"+unk|xxx");
//		addword('\0','\0',"+unk|yyy");
//		addword('\0','\0',"+*|www");
		mor_initwords();
	} else if (command == DSS_COM) {
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
	} else if (command == VOCD_COM) {
		addword('\0','\0',"+xx");
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yy");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+*|xx");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+*|yy");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
	}
}

static void init_s_option(int command) {
	int  i;

	clean_s_option();
	init_words(command);
	for (i=1; i < targc; i++) {
		if (*targv[i] == '+' || *targv[i] == '-') {
			if (targv[i][1] == 's') {
				if (i+1 < targc) {
					getflag(targv[i],targv[i+1],&i);
				} else {
					getflag(targv[i],NULL,&i);
				}
			}
		}
	}	
}

void call() {
	int  found, command;
	long tlineno = 0;
	char lRightspeaker;
	char isOutputGem;
	char word[BUFSIZ];
	struct speakers *ts;

	fprintf(stderr,"From file <%s>\n",oldfname);
	ts = NULL;
	if (isGOptionSet == FALSE)
		onlyApplyToDB = !isFoundGems();
	clean_speakers(speakers);
	for (command=BASE_COM; command < LAST_COMMAND; command++) {
		if (command != BASE_COM) {
			rewind(fpin);
			init_s_option(command);
		}
		if (!onlyApplyToDB && isNOptionSet == FALSE) {
			strcpy(kideval_BBS, "@*&#");
			strcpy(kideval_CBS, "@*&#");
		}
		if (kideval_SpecWords && !onlyApplyToDB) {
			isOutputGem = FALSE;
		} else {
			isOutputGem = TRUE;
		}
		spareTier1[0] = EOS;
		lRightspeaker = FALSE;
		found = 0;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (lineno > tlineno) {
				tlineno = lineno + 200;
			}
			if (!checktier(utterance->speaker)) {
				if (*utterance->speaker == '*') {
					lRightspeaker = FALSE;
					if (command == DSS_COM) {
						cleanUttline(uttline);
						strcpy(spareTier1, uttline);
					}
				}
				continue;
			} else {
				if (*utterance->speaker == '*') {
					lRightspeaker = TRUE;
				}
				if (!lRightspeaker && *utterance->speaker != '@')
					continue;
			}
			if (!onlyApplyToDB && kideval_SpecWords && !strcmp(kideval_BBS, "@*&#")) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					kideval_n_option = FALSE;
					strcpy(kideval_BBS, "@BG:");
					strcpy(kideval_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					kideval_n_option = TRUE;
					strcpy(kideval_BBS, "@G:");
					strcpy(kideval_CBS, "@*&#");
				}
			}
			if (!onlyApplyToDB && uS.partcmp(utterance->speaker,kideval_BBS,FALSE,FALSE)) {
				if (kideval_n_option) {
					if (isRightText(word)) {
						isOutputGem = TRUE;
					} else
						isOutputGem = FALSE;
				} else {
					if (isRightText(word)) {
						found++;
						if (found == 1 || GemMode != '\0') {
							isOutputGem = TRUE;
						}
					}
				}
			} else if (found > 0 && uS.partcmp(utterance->speaker,kideval_CBS,FALSE,FALSE)) {
				if (kideval_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (kideval_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			}
			if (*utterance->speaker == '@' || isOutputGem) {
				if (utterance->speaker[0] == '*') {
					strcpy(templineC, utterance->speaker);
					ts = kideval_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
				}
				if (command == BASE_COM) {
					kideval_process_tier(ts, NULL, NULL);
				} else if (command == DSS_COM) {
					if (dss_lang != '0') {
						lRightspeaker = kideval_process_dss_tier(ts, NULL, lRightspeaker);
					}
				} else if (command == VOCD_COM) {
					kideval_process_vocd_tier(ts, NULL);
				}
			}
		}
	}
	if (!vocd_analize(FALSE)) {
		kideval_error(NULL, FALSE);
	}
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, oldfname) == 0) {
			if (ts->dss_sp != NULL) {
				ts->dssTotal = ts->dss_sp->TotalNum;
				ts->dssGTotal = ts->dss_sp->GrandTotal;
				ts->dss_sp = NULL;
			} else {
				ts->dssTotal = 0.0;
				ts->dssGTotal = 0.0;
			}
			if (ts->vocd_sp != NULL) {
				ts->vocdOptimum = ts->vocd_sp->d_optimum;
				ts->vocd_sp = NULL;
			} else
				ts->vocdOptimum = 0.0;
		}
	}
	if (!combinput)
		kideval_pr_result();
	dss_freeSpeakers();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	time_t t;

	targv = argv;
	targc = argc;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = KIDEVAL;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
#ifdef UNX
	int i, j;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'L') {
				strcpy(lib_dir, argv[i]+2);
				j = strlen(lib_dir);
				if (j > 0 && lib_dir[j-1] != '/')
					strcat(lib_dir, "/");
			}
		}
	}
#endif
	debug_dss_fp = stdout;
#ifdef _CLAN_DEBUG
	t = 1283562804L; // RAND
#else
	t = time(NULL);
#endif
	srand(t);    // seed the random number generator
	VOCD_TYPE_D = FALSE;
	bmain(argc,argv,kideval_pr_result);
	db = freedb(db);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	dss_cleanSearchMem();
	labelsRoot = freeLabels(labelsRoot);
	colsRoot = freeColsP(colsRoot);
}

static struct label_cols *addNewLabel(int cln, char *label, char isDisplay) {
	int  cnt;
	char isSameLabel;
	struct label_cols *p;

	isSameLabel = FALSE;
	cnt = 0;
	if (labelsRoot == NULL) {
		labelsRoot = NEW(struct label_cols);
		p = labelsRoot;
	} else {
		for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
			cnt++;
		}
		if (p->const_label_num > 0 && cln == 0 && p->label == NULL) {
			isSameLabel = TRUE;
		} else if (p->const_label_num == 0 && cln > 0 && p->label != NULL) {
			isSameLabel = TRUE;
			p->const_label_num = cln;
		} else {
			p->next_label = NEW(struct label_cols);
			p = p->next_label;
			cnt++;
		}
	}
	if (p == NULL) {
		kideval_error(NULL, TRUE);
	}
	if (!isSameLabel) {
		p->next_label = NULL;
		p->isDisplay = isDisplay;
		p->num = cnt;
		p->const_label_num = cln;
		colLabelsNum++;
	}
	if (label == NULL)
		p->label = NULL;
	else {
		templineC2[0] = EOS;
		if (p->const_label_num == 17 || p->const_label_num == 0) {
			if (!isRawVal) {
				strcpy(templineC2, "% ");
				strcat(templineC2, label);
			} else
				strcpy(templineC2, label);
		} else
			strcpy(templineC2, label);
		p->label = (char *)malloc(strlen(templineC2)+1);
		if (p->label == NULL) {
			kideval_error(NULL, TRUE);
		}
		strcpy(p->label, templineC2);
	}
	return(p);
}

static COLS *add2col(COLS *root, char isFirstElem, char *pat, FNType *mFileName, int ln, char isDisplay) {
	int  i;
	char ch, res;
	COLS *p;

	if (isFirstElem || root == NULL) {
		if (root == NULL) {
			root = NEW(COLS);
			p = root;
		} else {
			for (p=root; p->next_col != NULL; p=p->next_col) ;
			p->next_col = NEW(COLS);
			p = p->next_col;
		}
		if (p == NULL) {
			kideval_error(NULL, TRUE);
		}
		p->next_col = NULL;
		p->pats = NULL;
		p->labelP = NULL;
		p->isClitic = FALSE;
	} else
		for (p=root; p->next_col != NULL; p=p->next_col) ;
	if (isdigit(pat[0])) {
		i = atoi(pat);
		if (i < 1 || i > 24) {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
			fprintf(stderr, "    ERROR: illegal built-in column number %d. Please choose between 1 and 24\n", i);
			sp_head = freespeakers(sp_head);
			DBKeyRoot = freeDBKeys(DBKeyRoot);
			labelsRoot = freeLabels(labelsRoot);
			colsRoot = freeColsP(colsRoot);
			db = freedb(db);
			dss_freeSpeakers();
			dss_cleanSearchMem();
			speakers = speaker_free_up_speakers(speakers, TRUE);
			if (VOCD_TYPE_D)
				denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
			cutt_exit(0);
		}
		p->labelP = addNewLabel(i, NULL, isDisplay);
	} else if (pat[0] == '"' || pat[0] == '\'') {
		for (i=0; pat[i] != EOS && (pat[i] == '"' || pat[i] == '\'' || isSpace(pat[i])); i++) ;
		if (pat[i] != EOS) {
			if (p->labelP != NULL && p->labelP->label != NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
				fprintf(stderr, "    ERROR: only one label is allow for each line: \"%s\"\n", pat);
				fprintf(stderr, "    Perhaps this label name is used on another line\n");
				sp_head = freespeakers(sp_head);
				DBKeyRoot = freeDBKeys(DBKeyRoot);
				labelsRoot = freeLabels(labelsRoot);
				colsRoot = freeColsP(colsRoot);
				db = freedb(db);
				dss_freeSpeakers();
				dss_cleanSearchMem();
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
			p->labelP = addNewLabel(0, pat+i, isDisplay);
		}
	} else {
		i = 0;
		if (pat[i] == '-') {
			i++;
			ch = 'e';
		} else if (pat[i] == '+') {
			i++;
			ch = 'i';
		} else {
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
			fprintf(stderr, "    ERROR: Illegal character \"%c\" found on line \"%s\"\n", pat[i], pat);
			fprintf(stderr, "    Expected characters are: + - \" '\n\n");
			sp_head = freespeakers(sp_head);
			DBKeyRoot = freeDBKeys(DBKeyRoot);
			labelsRoot = freeLabels(labelsRoot);
			colsRoot = freeColsP(colsRoot);
			db = freedb(db);
			dss_freeSpeakers();
			dss_cleanSearchMem();
			speakers = speaker_free_up_speakers(speakers, TRUE);
			if (VOCD_TYPE_D)
				denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
			cutt_exit(0);
		}
		if (pat[i] == 's' || pat[i] == 'S')
			i++;
		if (pat[i] == '@')
			i++;
		if (strchr(pat+i, '~') != NULL || strchr(pat+i, '$') != NULL)
			p->isClitic = TRUE;
		p->pats = makeMorWordList(p->pats, &res, pat+i, ch);
		if (p->pats == NULL) {
			if (res == TRUE)
				kideval_error(NULL, TRUE);
			else if (res == FALSE) {
				sp_head = freespeakers(sp_head);
				DBKeyRoot = freeDBKeys(DBKeyRoot);
				labelsRoot = freeLabels(labelsRoot);
				colsRoot = freeColsP(colsRoot);
				db = freedb(db);
				dss_freeSpeakers();
				dss_cleanSearchMem();
				speakers = speaker_free_up_speakers(speakers, TRUE);
				if (VOCD_TYPE_D)
					denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
				cutt_exit(0);
			}
		}
	}
	return(root);
}

static void read_script(char *lang) {
	int  ln;
	char *b, *e, t, isQT, isFirstElem, isDisplay;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*lang == EOS) {
		fprintf(stderr,	"No language specified with +l option.\n");
		fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
		fprintf(stderr,"For example, \"kideval +leng\" or \"kideval +leng.cut\".\n");
		cutt_exit(0);
	}
	strcpy(mFileName, lib_dir);
	addFilename2Path(mFileName, "kideval");
	addFilename2Path(mFileName, lang);
	if ((b=strchr(lang, '.')) != NULL) {
		if (uS.mStricmp(b, ".cut") != 0)
			strcat(mFileName, ".cut");
	} else
		strcat(mFileName, ".cut");
	if ((fp=fopen(mFileName,"r")) == NULL) {
		if (b != NULL) {
			if (uS.mStricmp(b, ".cut") == 0) {
				strcpy(templineC, wd_dir);
				addFilename2Path(templineC, lang);
				if ((fp=fopen(templineC,"r")) != NULL) {
					strcpy(mFileName, templineC);
				}
			}
		}
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate language file: \"%s\".\n", mFileName);
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
		cutt_exit(0);
	}
	fprintf(stderr,"    Using language file: %s\n", mFileName);
	ln = 0;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		ln++;
		if (templineC[0] == ';' || templineC[0] == '%')
			continue;
		if (uS.mStrnicmp(templineC, "dss ", 4) == 0) {
			b = templineC + 4;
			e = b;
			while (*b != EOS) {
				if (*e == '"' || *e == '\'') {
					if (isQT == *e)
						isQT = '\0';
					else if (isQT == '\0')
						isQT = *e;
				}
				if (((isSpace(*e) || *e == '"' || *e == '\'') && isQT == '\0') || *e == EOS) {
					t = *e;
					*e = EOS;
					strcpy(templineC1, b);
					uS.remblanks(templineC1);
					if ((templineC1[0] == '+' || templineC1[0] == '-') && (templineC1[1] == 'l' || templineC1[1] == 'L')) {
						if (templineC1[2] == 'e' || templineC1[2] == 'E') {
							dss_lang = 'e';
							if (rulesfile == NULL)
								rulesfile = DSSRULES;
						} else if (templineC1[2] == 'j' || templineC1[2] == 'J') {
							dss_lang = 'j';
							if (rulesfile == NULL)
								rulesfile = DSSJPRULES;
						} else if (templineC1[2] == '0' || templineC1[2] == 'O') {
							dss_lang = '0';
							if (rulesfile == NULL)
								rulesfile = DSSNOCOMPUTE;
						} else {
							fprintf(stderr,"*** File \"%s\": line: \"%d\"\n", mFileName, ln);
							fprintf(stderr,"    Illegal option argument found: %s\n", templineC1);
							fprintf(stderr,"    Please specify either \"+le\" for English or \"+lj\" for Japanese.\n");
							cutt_exit(0);
						}
					} else if ((templineC1[0] == '+' || templineC1[0] == '-') && (templineC1[1] == 'd' || templineC1[1] == 'D')) {
						strncpy(dssRulesFName, templineC1+2, 255);
						dssRulesFName[255] = EOS;
						rulesfile = dssRulesFName;
					} else {
						fprintf(stderr,"*** File \"%s\": line: \"%d\"\n", mFileName, ln);
						fprintf(stderr,"    Unsupported option specified: %s\n", templineC1);
						cutt_exit(0);
					}
					*e = t;
					if (*e != EOS)
						e++;
					b = e;
				} else
					e++;
			}
			continue;
		}
		uS.remFrontAndBackBlanks(templineC);
		if (templineC[0] == EOS)
			continue;
		if (templineC[0] == '#') {
			for (b=templineC; *b == '#' || isSpace(*b); b++) ;
			if (b != templineC)
				strcpy(templineC, b);
			isDisplay = FALSE;
		} else
			isDisplay = TRUE;
		isFirstElem = TRUE;
		isQT = '\0';
		b = templineC;
		e = b;
		while (*b != EOS) {
			if (*e == '"' || *e == '\'') {
				if (isQT == *e)
					isQT = '\0';
				else if (isQT == '\0')
					isQT = *e;
			}
			if (((isSpace(*e) || *e == '"' || *e == '\'') && isQT == '\0') || *e == EOS) {
				t = *e;
				*e = EOS;
				strcpy(templineC1, b);
				uS.remblanks(templineC1);
				if (templineC1[0] != EOS) {
					colsRoot = add2col(colsRoot, isFirstElem, templineC1, mFileName, ln, isDisplay);
					isFirstElem = FALSE;
				}
				*e = t;
				if (*e != EOS)
					e++;
				b = e;
			} else
				e++;
		}
	}
	fclose(fp);
	if (colsRoot == NULL) {
		fprintf(stderr,"Can't find any usable declarations in language script file \"%s\".\n", mFileName);
		cutt_exit(0);
	}
/*
struct label_cols *p;
for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
	fprintf(stdout, "%s: %d\n", p->label, p->num);
}
*/
}

void init(char first) {
	int  i;

	if (first) {
		ftime = TRUE;
		isRawVal = FALSE;
		script_file = NULL;
		stout = FALSE;
		outputOnlyData = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".xls";
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		init_words(BASE_COM);
		isDebug = FALSE;
		isXXXFound = FALSE;
		GemMode = '\0';
		kideval_SpecWords = 0;
		kideval_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		kideval_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(morTierName, "%mor:");
		strcpy(kideval_BBS, "@*&#");
		strcpy(kideval_CBS, "@*&#");
		isSpeakerNameGiven = FALSE;
		sp_head = NULL;
		colLabelsNum = 0;
		colsRoot = NULL;
		labelsRoot = NULL;
		db = NULL;
		DBKeyRoot = NULL;
		DBGemNum = 0;
		combinput = TRUE;
		if (!init_dss(first))
			kideval_error(NULL, TRUE);
		init_vocd(first);
	} else {
		if (ftime) {
			ftime = FALSE;
			if (script_file == NULL) {
				fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"kideval +leng\" or \"kideval +leng.cut\".\n");
				cutt_exit(0);
			}
			read_script(script_file);
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					  GlobalPunctuation[i] == '?' ||
					  GlobalPunctuation[i] == '.') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
			if (!chatmode) {
				fputs("+/-t option is not allowed with TEXT format\n", stderr);
				kideval_error(NULL, FALSE);
			}
#ifdef JUST_ONE_SPEAKER
			for (i=1; i < targc; i++) {
				if (*targv[i] == '+' || *targv[i] == '-') {
					if (targv[i][1] == 't') {
						f = targv[i]+2;
						if (*f == '@') {
							if (uS.mStrnicmp(f+1, "ID=", 3) == 0 && *(f+4) != EOS) {
								isSpeakerNameGiven = TRUE;
							}
						} else if (*f == '#') {
							isSpeakerNameGiven = TRUE;
						} else if (*f != '%' && *f != '@') {
							isSpeakerNameGiven = TRUE;
						}
					}
				}
			}	
			if (!isSpeakerNameGiven) {
#ifndef UNX
				fprintf(stderr,"Please specify speaker's tier code with Option button in Commands window.\n");
				fprintf(stderr,"Or with \"+t\" option on command line.\n");
#else
				fprintf(stderr,"Please specify speaker's tier code with \"+t\" option on command line.\n");
#endif
				kideval_error(NULL, FALSE);
			}
			maketierchoice("*:", '+', '\001');
#endif // JUST_ONE_SPEAKER
			if (DBKeyRoot != NULL) {
				db = NEW(struct database);
				if (db == NULL)
					kideval_error(db, TRUE);
				init_db(db);
				ParseDatabase(db);
				CleanUpTempIDSpeakers();
			}
			for (i=1; i < targc; i++) {
				if (*targv[i] == '+' || *targv[i] == '-') {
					if (targv[i][1] == 't') {
						if (targv[i][2] == '%')
							strcpy(morTierName, targv[i]+2);
						else
							maingetflag(targv[i], NULL, &i);
					}
				}
			}	
			if (!init_dss(first))
				kideval_error(NULL, FALSE);
		} else
			init_s_option(BASE_COM);
		init_vocd(first);
	}
	if (!combinput || first) {
		sp_head = NULL;
	}
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'b':
			morf = getfarg(f,f1,i);
			if (*(f-2) == '-') {
				if (*morf != EOS) {
					for (j=0; rootmorf[j] != EOS; ) {
						if (uS.isCharInMorf(rootmorf[j],morf)) 
							strcpy(rootmorf+j,rootmorf+j+1);
						else
							j++;
					}
				} else
					rootmorf[0] = EOS;
			} else {
				if (*morf != EOS) {
					t = rootmorf;
					rootmorf = (char *)malloc(strlen(t)+strlen(morf)+1);
					if (rootmorf == NULL) {
						fprintf(stderr,"No more space left in core.\n");
						cutt_exit(1);
					}
					strcpy(rootmorf,t);
					strcat(rootmorf,morf);
					free(t);
				}
			}
			break;
		case 'd':
			if (*f == EOS) {
				isDebug = TRUE;
			} else {
#ifdef _CLAN_DEBUG
				addDBKeys(f);
#else // _CLAN_DEBUG
				no_arg_option(f);
#endif // else _CLAN_DEBUG
			}
			break;
		case 'g':
			if (*f == EOS) {
				if (*(f-2) == '+')
					kideval_group = TRUE;
				else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				kideval_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'l':
			script_file = f;
			break;
#ifdef UNX
		case 'L':
			strcpy(lib_dir, f);
			j = strlen(lib_dir);
			if (j > 0 && lib_dir[j-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 'n':
			if (*(f-2) == '+') {
				kideval_n_option = TRUE;
				strcpy(kideval_BBS, "@G:");
				strcpy(kideval_CBS, "@*&#");
			} else {
				strcpy(kideval_BBS, "@BG:");
				strcpy(kideval_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
			break;
		case 'o':
			if (*f == '4') {
				isRawVal = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 's':
			if (*(f-2) == '+')
				isXXXFound = TRUE;
			else {
				fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
				fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
				kideval_error(NULL, FALSE);
			}
			break;
		case 't':
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
