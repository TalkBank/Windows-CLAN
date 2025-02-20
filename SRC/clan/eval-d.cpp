/**********************************************************************
 "Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

/*
 isCreateDB

 @ID   = +
 # gem results and unique words
 @G    = G
 @BG   = B
 @EG   = E
 @Time Duration = T
 @END  = -

 prTSResults
 retrieveTS
 evald_initTSVars
 wID
   wnum;
   wused;
 db_sp
 sp_head
 evald_FindSpeaker
 evald_FREQ_tree

 clause;
 tUtt;
 CUR_sqr, CUR, mn_CUR, mx_CUR;

 nounsNV, verbsNV, openClass, closedClass;
 NoV_sqr, NoV, mn_NoV, mx_NoV;
 OPoCl_sqr, OPoCl, mn_OPoCl, mx_OPoCl;
*/


/* debugging
#define DEBUGEVALRESULTS

 Commands for specific two gems:
 eval eval-adler01a.cha +t*PAR: +d"Broca" +gSpeech +gWindow +u > n-gems.txt
 freq ACWT01a.gem.cha -s"[+ exc]" +t*par
 mlu ACWT01a.gem.cha -s"[+ exc]" +t*par
 mlu ACWT01a.gem.cha -s"[+ exc]" +t*par -b
 freq ACWT01a.gem.cha -s"[+ exc]" +t%mor -t*
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|n,|n:*
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|v,|cop,|mod,|part
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|aux
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|prep
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|adj
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|adv
 freq ACWT01a.gem.cha -s"[+ exc]" +sm|conj
 freq ACWT01a.gem.cha -s"[+ exc]" +sm-PL +sm&PL
 freq ACWT01a.gem.cha -s"[+ exc]" +s"[//]"
 freq ACWT01a.gem.cha -s"[+ exc]" +s"[/]"

 Commands for specific all gems:
 eval eval-adler01a.cha +t*PAR: +d"Broca" +u > n-all.txt
 freq ACWT01a.cha -s"[+ exc]" +t*par
 mlu ACWT01a.cha -s"[+ exc]" +t*par
 mlu ACWT01a.cha -s"[+ exc]" +t*par -b
 freq ACWT01a.cha -s"[+ exc]" +t%mor -t*
 freq ACWT01a.cha -s"[+ exc]" +sm|n,|n:*
 freq ACWT01a.cha -s"[+ exc]" +sm|v,|cop,|mod,|part
 freq ACWT01a.cha -s"[+ exc]" +sm|aux
 freq ACWT01a.cha -s"[+ exc]" +sm|prep
 freq ACWT01a.cha -s"[+ exc]" +sm|adj
 freq ACWT01a.cha -s"[+ exc]" +sm|adv
 freq ACWT01a.cha -s"[+ exc]" +sm|conj
 freq ACWT01a.cha -s"[+ exc]" +sm-PL +sm&PL
 freq ACWT01a.cha -s"[+ exc]" +s"[//]"
 freq ACWT01a.cha -s"[+ exc]" +s"[/]"

 prDebug
*/
/*
clauses/Total Utts
	"v" or "cop*" or "part" after "aux"
word_error
	[*] or [* ...]
utt_error
	[+ *], [+ jar], [+ es], [+ cir], [+ per] or [+ gram]
Nouns
	"n" or "n:*"
Plurals
	"-PL" or "&PL"
Verbs
	"v", "part" and "-PASTP" or "-PRESP" or "&PASTP" or "&PRESP", "cop*", "mod*"
*/

#define CHAT_MODE 1
#include "cu.h"
#include <math.h>
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main evald_main
#define call evald_call
#define getflag evald_getflag
#define init evald_init
#define usage evald_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define EVAL_DB_VERSION 5

#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "_eval-d_db.cut"
#define DATABSEFILESLIST "0eval-d_Database_IDs.cex"

extern char R8;
extern char OverWriteFile;
extern char outputOnlyData;
extern char isRecursive;
extern char isLanguageExplicit;
extern char isPostcliticUse;	/* if '~' found on %mor tier break it into two words */
extern char isPrecliticUse;		/* if '$' found on %mor tier break it into two words */
extern struct tier *defheadtier;
extern struct IDtype *IDField;

enum {
	UNK = 0,
	ENG,
} ;

#define SDRESSIZE 256

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct evald_words {
	char *word;
	int  wnum;
	char wused;
	struct evald_words *left;
	struct evald_words *right;
};

struct evald_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;
	int  wID;
	float tm;
	float tUtt;
	float spWrdCnt; // counts all words on speaker tier
	float frTokens;	// total number of words
	float frTypes;	// number of diff words
	float CUR;
	float mluWords, morf, mluUtt;
	float werr, uerr;
	float morTotal;	// total number of words on %mor tier
	float density;	// CPIDR
	float noun, pl, verb, aux, mod, prep, adj, adv, conj, pron, det, thrS, thrnS, past, pastp, presp;
	float retr, repeat;
	float nounsNV, verbsNV, openClass, closedClass;
	struct evald_words *words_root;
	struct evald_speakers *next_sp;
} ;

struct database {
	float num;
	float tm_sqr, tm, mn_tm, mx_tm;
	float tUtt_sqr, tUtt, mn_tUtt, mx_tUtt;
	float frTokens_sqr, frTokens, mn_frTokens, mx_frTokens;	/* total number of words */
	float frTypes_sqr, frTypes, mn_frTypes, mx_frTypes;	/* number of diff words */
	float TTR_sqr, TTR, mn_TTR, mx_TTR;
	float WOD_sqr, WOD, mn_WOD, mx_WOD;
	float CUR_sqr, CUR, mn_CUR, mx_CUR;
	float NoV_sqr, NoV, mn_NoV, mx_NoV;
	float OPoCl_sqr, OPoCl, mn_OPoCl, mx_OPoCl;
	float mluUtt_sqr, mluUtt, mn_mluUtt, mx_mluUtt;
	float mluWords_sqr, mluWords, mn_mluWords, mx_mluWords;
	float morf_sqr, morf, mn_morf, mx_morf;
	float werr_sqr, werr, mn_werr, mx_werr;
	float uerr_sqr, uerr, mn_uerr, mx_uerr;
	float morTotal_sqr, morTotal, mn_morTotal, mx_morTotal;	/* total number of words on %mor tier */
	float density_sqr, density, mn_density, mx_density;
	float noun_sqr, noun, mn_noun, mx_noun;
	float verb_sqr, verb, mn_verb, mx_verb;
	float aux_sqr, aux, mn_aux, mx_aux;
	float mod_sqr, mod, mn_mod, mx_mod;
	float prep_sqr, prep, mn_prep, mx_prep;
	float adj_sqr, adj, mn_adj, mx_adj;
	float adv_sqr, adv, mn_adv, mx_adv;
	float conj_sqr, conj, mn_conj, mx_conj;
	float pron_sqr, pron, mn_pron, mx_pron;
	float retr_sqr, retr, mn_retr, mx_retr;
	float repeat_sqr, repeat, mn_repeat, mx_repeat;
	float det_sqr, det, mn_det, mx_det;
	float thrS_sqr, thrS, mn_thrS, mx_thrS;
	float thrnS_sqr, thrnS, mn_thrnS, mx_thrnS;
	float past_sqr, past, mn_past, mx_past;
	float pastp_sqr, pastp, mn_pastp, mx_pastp;
	float presp_sqr, presp, mn_presp, mx_presp;
	float pl_sqr, pl, mn_pl, mx_pl;
	float openClass_sqr, openClass, mn_openClass, mx_openClass;	/* total number of words */
	float closedClass_sqr, closedClass, mn_closedClass, mx_closedClass;	/* total number of words */
	struct evald_speakers *db_sp;
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
#if defined(_MAC_CODE) || defined(_WIN32)
static char **targv;
#endif
#ifdef UNX
static char *targv[MAX_ARGS];
#endif
static int  evald_SpecWords, DBGemNum, wdOffset;
static char *DBGems[NUMGEMITEMS];
static const char *lang_prefix;
static char isSpeakerNameGiven, isExcludeWords, ftime, isPWordsList, isDBFilesList, isCreateDB, isRawVal, specialOptionUsed;
static char evald_BBS[5], evald_CBS[5], evald_group, evald_n_option, isNOptionSet, onlyApplyToDB, isGOptionSet, GemMode, langType;
static float tmDur;
static FILE	*dbfpout;
static FILE *DBFilesListFP;
static struct DBKeys *DBKeyRoot;
static struct evald_speakers *sp_head;
static struct database *db;

void usage() {
	printf("Usage: eval-d [bS dS eN g gS lF n %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(_CLAN_DEBUG) || defined(UNX)
	puts("+c : create EVAL-D database file (eval-d -c +t*PAR)");
	puts("	First set working directory to: ~dementia-data");
#endif // _CLAN_DEBUG
	puts("+dS: specify database keyword(s) S. Choices are:");
	puts("    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control,");
	puts("    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),");
	puts("    Nonfluent = (Global, Broca, Transmotor),");
	puts("    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)");
	puts("+e1: create list of database files used for comparisons");
	puts("+e2: create proposition word list for each CHAT file");
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("-g : look for gems in Database only");
	puts("+gS: select gems which are labeled by label S");
	puts("+lF: specify language database file name F");
	puts("     choices: eng, fra");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+o4: output raw values instead of percentage values");
	mainusage(FALSE);
	puts("Example:"); // 2022-08-19
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70");
	puts("       eval-d 05.cha +t*par +gCinderella +u +d\"control|60-70\"");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       eval-d adler15a.cha +t*par +gCinderella +u +d\"control|60-70;6\"");
	puts("   Search database for \"Cinderella\" gems of \"Nonfluent\" subjects of any age");
	puts("       eval-d adler15a.cha +t*par +gCinderella +u +dNonfluent");
	puts("   Just compute results for \"Cinderella\" gems of adler15a.cha. Do not compare to database");
	puts("       eval-d adler15a.cha +t*par +gCinderella +u");
	cutt_exit(0);
}

static void init_db(struct database *p) {
	p->num = 0.0;
	p->tm_sqr = (float)0.0; p->tm = (float)0.0; p->mn_tm = (float)0.0; p->mx_tm = (float)0.0;
	p->tUtt_sqr = (float)0.0; p->tUtt = (float)0.0; p->mn_tUtt = (float)0.0; p->mx_tUtt = (float)0.0;
	p->frTokens_sqr = (float)0.0; p->frTokens = (float)0.0; p->mn_frTokens = (float)0.0; p->mx_frTokens = (float)0.0;
	p->frTypes_sqr = (float)0.0; p->frTypes = (float)0.0; p->mn_frTypes = (float)0.0; p->mx_frTypes = (float)0.0;
	p->TTR_sqr = (float)0.0; p->TTR = (float)0.0; p->mn_TTR = (float)0.0; p->mx_TTR = (float)0.0;
	p->WOD_sqr = (float)0.0; p->WOD = (float)0.0; p->mn_WOD = (float)0.0; p->mx_WOD = (float)0.0;
	p->CUR_sqr = (float)0.0; p->CUR = (float)0.0; p->mn_CUR = (float)0.0; p->mx_CUR = (float)0.0;
	p->NoV_sqr = (float)0.0; p->NoV = (float)0.0; p->mn_NoV = (float)0.0; p->mx_NoV = (float)0.0;
	p->OPoCl_sqr = (float)0.0; p->OPoCl = (float)0.0; p->mn_OPoCl = (float)0.0; p->mx_OPoCl = (float)0.0;
	p->mluUtt_sqr = (float)0.0; p->mluUtt = (float)0.0; p->mn_mluUtt = (float)0.0; p->mx_mluUtt = (float)0.0;
	p->mluWords_sqr = (float)0.0; p->mluWords = (float)0.0; p->mn_mluWords = (float)0.0; p->mx_mluWords = (float)0.0;
	p->morf_sqr = (float)0.0; p->morf = (float)0.0; p->mn_morf = (float)0.0; p->mx_morf = (float)0.0;
	p->werr_sqr = (float)0.0; p->werr = (float)0.0; p->mn_werr = (float)0.0; p->mx_werr = (float)0.0;
	p->uerr_sqr = (float)0.0; p->uerr = (float)0.0; p->mn_uerr = (float)0.0; p->mx_uerr = (float)0.0;
	p->morTotal_sqr = (float)0.0; p->morTotal = (float)0.0; p->mn_morTotal = (float)0.0; p->mx_morTotal = (float)0.0;
	p->density_sqr  = (float)0.0; p->density  = (float)0.0; p->mn_density  = (float)0.0; p->mx_density  = (float)0.0;
	p->noun_sqr = (float)0.0; p->noun = (float)0.0; p->mn_noun = (float)0.0; p->mx_noun = (float)0.0;
	p->verb_sqr = (float)0.0; p->verb = (float)0.0; p->mn_verb = (float)0.0; p->mx_verb = (float)0.0;
	p->aux_sqr = (float)0.0; p->aux = (float)0.0; p->mn_aux = (float)0.0; p->mx_aux = (float)0.0;
	p->mod_sqr = (float)0.0; p->mod = (float)0.0; p->mn_mod = (float)0.0; p->mx_mod = (float)0.0;
	p->prep_sqr = (float)0.0; p->prep = (float)0.0; p->mn_prep = (float)0.0; p->mx_prep = (float)0.0;
	p->adj_sqr = (float)0.0; p->adj = (float)0.0; p->mn_adj = (float)0.0; p->mx_adj = (float)0.0;
	p->adv_sqr = (float)0.0; p->adv = (float)0.0; p->mn_adv = (float)0.0; p->mx_adv = (float)0.0;
	p->conj_sqr = (float)0.0; p->conj = (float)0.0; p->mn_conj = (float)0.0; p->mx_conj = (float)0.0;
	p->pron_sqr = (float)0.0; p->pron = (float)0.0; p->mn_pron = (float)0.0; p->mx_pron = (float)0.0;
	p->retr_sqr = (float)0.0; p->retr = (float)0.0; p->mn_retr = (float)0.0; p->mx_retr = (float)0.0;
	p->repeat_sqr = (float)0.0; p->repeat = (float)0.0; p->mn_repeat = (float)0.0; p->mx_repeat = (float)0.0;
	p->det_sqr = (float)0.0; p->det = (float)0.0; p->mn_det = (float)0.0; p->mx_det = (float)0.0;
	p->thrS_sqr = (float)0.0; p->thrS = (float)0.0; p->mn_thrS = (float)0.0; p->mx_thrS = (float)0.0;
	p->thrnS_sqr = (float)0.0; p->thrnS = (float)0.0; p->mn_thrnS = (float)0.0; p->mx_thrnS = (float)0.0;
	p->past_sqr = (float)0.0; p->past = (float)0.0; p->mn_past = (float)0.0; p->mx_past = (float)0.0;
	p->pastp_sqr = (float)0.0; p->pastp = (float)0.0; p->mn_pastp = (float)0.0; p->mx_pastp = (float)0.0;
	p->presp_sqr = (float)0.0; p->presp = (float)0.0; p->mn_presp = (float)0.0; p->mx_presp = (float)0.0;
	p->pl_sqr = (float)0.0; p->pl = (float)0.0; p->mn_pl = (float)0.0; p->mx_pl = (float)0.0;
	p->openClass_sqr = (float)0.0; p->openClass = (float)0.0; p->mn_openClass = (float)0.0; p->mx_openClass = (float)0.0;
	p->closedClass_sqr = (float)0.0; p->closedClass = (float)0.0; p->mn_closedClass = (float)0.0; p->mx_closedClass = (float)0.0;
	p->db_sp = NULL;
}

static struct evald_words *evald_freetree(struct evald_words *p) {
	if (p != NULL) {
		evald_freetree(p->left);
		evald_freetree(p->right);
		if (p->word != NULL)
			free(p->word);
		free(p);
	}
	return(NULL);
}

static struct evald_speakers *freespeakers(struct evald_speakers *p) {
	struct evald_speakers *ts;

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
			evald_freetree(ts->words_root);
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

static void evald_error(struct database *db, char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	db = freedb(db);
	cutt_exit(0);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		evald_error(NULL, FALSE);
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

static char *evald_strsave(const char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		evald_error(NULL, FALSE);
	return(p);
}

static struct DBKeys *initDBKey(const char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		evald_error(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = evald_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = evald_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = evald_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = evald_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = evald_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = evald_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = evald_strsave("Wernicke");
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
		DBKey->key1 = evald_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = evald_strsave("Wernicke");
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
	if (uS.mStricmp(b,"Control") == 0 || uS.mStricmp(b,"MCI") == 0 || uS.mStricmp(b,"Memory") == 0 ||
		uS.mStricmp(b,"PossibleAD") == 0 || uS.mStricmp(b,"ProbableAD") == 0 || uS.mStricmp(b,"Vascular") == 0) {
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
					DBKey->key1 = evald_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = evald_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = evald_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = evald_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = evald_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					evald_error(NULL, FALSE);
				}
				DBKey->age = evald_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Control, MCI, Memory, PossibleAD, ProbableAD, Vascular\n");
				evald_error(NULL, FALSE);
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

static void evald_resettree(struct evald_words *p) {
	if (p != NULL) {
		evald_resettree(p->left);
		p->wused = FALSE;
		evald_resettree(p->right);
	}
}

static void evald_initTSVars(struct evald_speakers *ts, char isAll) {
	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;
	ts->tm		= (float)0.0;
	ts->tUtt	= (float)0.0;
	ts->spWrdCnt= (float)0.0;
	ts->frTokens= (float)0.0;
	ts->frTypes = (float)0.0;
	ts->CUR		= (float)0.0;
	ts->nounsNV	= (float)0.0;
	ts->verbsNV	= (float)0.0;
	ts->openClass	= (float)0.0;
	ts->closedClass = (float)0.0;
	ts->mluWords= (float)0.0;
	ts->morf	= (float)0.0;
	ts->mluUtt	= (float)0.0;
	ts->werr    = (float)0.0;
	ts->uerr    = (float)0.0;
	ts->morTotal= (float)0.0;
	ts->density = (float)0.0;
	ts->noun    = (float)0.0;
	ts->verb    = (float)0.0;
	ts->aux     = (float)0.0;
	ts->mod     = (float)0.0;
	ts->prep    = (float)0.0;
	ts->adj     = (float)0.0;
	ts->adv     = (float)0.0;
	ts->conj    = (float)0.0;
	ts->pron    = (float)0.0;
	ts->retr    = (float)0.0;
	ts->repeat  = (float)0.0;
	ts->det     = (float)0.0;
	ts->thrS    = (float)0.0;
	ts->thrnS   = (float)0.0;
	ts->past    = (float)0.0;
	ts->pastp   = (float)0.0;
	ts->pl      = (float)0.0;
	ts->presp   = (float)0.0;
	if (isAll) {
		ts->wID = 0;
		ts->words_root = NULL;
	} else {
		evald_resettree(ts->words_root);
	}
}

static struct evald_speakers *evald_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	struct evald_speakers *ts, *tsp;

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
			fprintf(stderr, "\nERROR: More than 1 speaker selected in a file: %s\n", fname);
			evald_error(db, FALSE);
		}
	}
	if ((ts=NEW(struct evald_speakers)) == NULL)
		evald_error(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		evald_error(db, TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		evald_error(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL || isCreateDB)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			evald_error(db, TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	evald_initTSVars(ts, TRUE);
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
	return(ts);
}

static void filterAndOuputID(char *IDs) {
	char *s, *sb;
	
	for (s=IDs; isSpace(*s); s++) ;
	sb = s;
	if ((s=strchr(sb, '|')) != NULL) {
	} else
		*sb = EOS;
	fprintf(dbfpout, "=%s\n", oldfname+wdOffset);
	fprintf(dbfpout, "+%s\n", IDs);
/*
	dbPos = ftell(dbfpout);
	fprintf(dbfpout, "                    \n", utterance->speaker, utterance->line);
*/
}
/*
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
*/
static char evald_isUttDel(char *line, int pos) {
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

static struct evald_words *evald_talloc(char *word, struct database *db, int wnum) {
	struct evald_words *p;

	if ((p=NEW(struct evald_words)) == NULL)
		evald_error(db, TRUE);
	p->left = p->right = NULL;
	p->wnum = wnum;
	p->wused = TRUE;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		evald_error(db, TRUE);
	}
	return(p);
}

static struct evald_words *evald_FREQ_tree(struct evald_words *p, char *w, struct evald_speakers *ts, struct database *db) {
	int cond;
	struct evald_words *t = p;

	if (p == NULL) {
		ts->wID++;
		ts->frTypes++;
		p = evald_talloc(w, db, ts->wID);
	} else if ((cond=strcmp(w,p->word)) == 0) {
		p->wused = TRUE;
	} else if (cond < 0) {
		p->left = evald_FREQ_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
			p->wused = TRUE;
		} else if (cond < 0) {
			p->left = evald_FREQ_tree(p->left, w, ts, db);
		} else {
			p->right = evald_FREQ_tree(p->right, w, ts, db); /* if cond > 0 */
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

static char isN(MORFEATS *feat) {
	if (isEqual("n", feat->pos) || isEqual("n:prop", feat->pos) || isEqual("n:pt", feat->pos))
		return(TRUE);
	return(FALSE);
}

static char isPRO(MORFEATS *feat) {
	if (isEqual("pro:per", feat->pos) || isEqual("pro:indef", feat->pos) || isEqual("pro:refl", feat->pos) || isEqual("pro:obj", feat->pos))
		return(TRUE);
	return(FALSE);
}

static char isDET_ART(MORFEATS *feat) {
	if (isEqual("that", feat->stem) || isEqual("those", feat->stem) || isEqual("this", feat->stem) || isEqual("these", feat->stem) ||
		isEqual("a", feat->stem) || isEqual("an", feat->stem) || isEqual("the", feat->stem))
		return(TRUE);
	return(FALSE);
}

static char isMOD(MORFEATS *feat) {
	if (isEqual("adj", feat->pos) || isEqual("det:poss", feat->pos) || isEqual("qn", feat->pos) ||
		isEqual("det:num", feat->pos) || isEqual("det:dem", feat->pos) || isEqual("det:art", feat->pos))
		return(TRUE);
	return(FALSE);
}

static char isOTH(MORFEATS *feat, char resOth) {
	if (isN(feat))
		return(3);
	if (isDET_ART(feat)) {
		if (resOth == 1)
			return(2);
		else
			return(0);
	}
	if (isMOD(feat))
		return(2);
	return(0);
}

static float roundFloat(double num) {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}

static void evald_process_tier(struct evald_speakers *ts, struct database *db, char *rightIDFound, char isOutputGem, FILE *PWordsListFP) {
	int i, j, num, oPos;
	char word[1024], tword[1024];
	char tchr, isWordsFound, sq, aq, isSkip, isFiller;
	char isPSDFound, curPSDFound, isAuxFound;
	long stime, etime;
	double tNum;
	float mluWords, morf, mluUtt, morphCnt;
	struct IDparts IDTier;
	MORFEATS word_feats, *clitic, *feat;

	if (utterance->speaker[0] == '*') {
		strcpy(spareTier1, utterance->line);
		strcpy(spareTier2, uttline);
		if (tmDur >= 0.0) {
			ts->tm = ts->tm + tmDur;
			tmDur = -1.0;
		}
		for (i=0; utterance->line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->mluUtt > 0.0)
					ts->mluUtt--;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				ts->isPSDFound = FALSE;
			}
		}
		i = 0;
		while ((i=getword(utterance->speaker, utterance->line, word, NULL, i))) {
			if (!strcmp(word, "[//]"))
				ts->retr++;
			else if (!strcmp(word, "[/]"))
				ts->repeat++;
		}
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (ts == NULL)
			return;
		filterwords("%", uttline, excludePlusAndUttDels);
		if (isExcludeWords)
			filterwords("%", uttline, exclude);
		strcpy(spareTier3, uttline);
		for (i=0; spareTier3[i] != EOS; i++) {
			if (spareTier3[i] == '$' && !isPrecliticUse)
				spareTier3[i] = ' ';
			if (spareTier3[i] == '~' && !isPostcliticUse)
				spareTier3[i] = ' ';
		}
		i = 0;
		while ((i=getword(utterance->speaker, spareTier3, templineC2, NULL, i))) {
			if (isWordFromMORTier(templineC2)) {
				if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
					evald_error(db, TRUE);
				for (clitic=&word_feats; clitic != NULL; clitic=clitic->clitc) {
					if (clitic->compd != NULL) {
						word[0] = EOS;
						for (feat=clitic; feat != NULL; feat=feat->compd) {
							if (feat->stem != NULL && feat->stem[0] != EOS) {
								strcat(word, feat->stem);
							}
						}
						if (word[0] != EOS && exclude(word) && isRightWord(word)) {
							uS.remblanks(word);
							ts->frTokens++;
							ts->words_root = evald_FREQ_tree(ts->words_root, word, ts, db);
						}
					} else {
						if (isPrecliticUse && clitic->typeID == '$') {
						} else if (isPostcliticUse && clitic->typeID == '~') {
						} else if (clitic->stem != NULL && clitic->stem[0] != EOS && exclude(clitic->stem) && isRightWord(clitic->stem)) {
							uS.remblanks(clitic->stem);
							ts->frTokens++;
							ts->words_root = evald_FREQ_tree(ts->words_root, clitic->stem, ts, db);
						}
					}
				}
				freeUpFeats(&word_feats);
			}
		}
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (strchr(templineC2, '|') != NULL) {
				ts->morTotal++;
				for (j=0; templineC2[j] != EOS; j++) {
					if (templineC2[j] == '$' && !isPrecliticUse)
						ts->morTotal++;
					if (templineC2[j] == '~' && !isPostcliticUse)
						ts->morTotal++;
				}
			}
		}
		isSkip = FALSE;
		if (isPostCodeOnUtt(spareTier1, "[+ mlue]")) {
			isSkip = TRUE;
		} else {
			i = 0;
			while ((i=getword(utterance->speaker, spareTier2, word, NULL, i))) {
				if (strcmp(word, "xxx") == 0 || strcmp(word, "yyy") == 0 || strcmp(word, "www") == 0) {
					isSkip = TRUE;
					break;
				}
			}
		}
		ts->isMORFound = TRUE;
		mluWords	= ts->mluWords;
		morf		= ts->morf;
		mluUtt		= ts->mluUtt;
		isPSDFound	= ts->isPSDFound;
		curPSDFound = FALSE;
		isWordsFound = FALSE;
		sq = FALSE;
		aq = FALSE;
		i = 0;
		do {
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
				isWordsFound = TRUE;
				mluWords = mluWords + 1;
				oPos = i;
				while (uttline[i]) {
					if (uS.isskip(uttline,i,&dFnt,MBF)) {
						if (uS.IsUtteranceDel(utterance->line, i)) {
							if (!uS.atUFound(utterance->line, i, &dFnt, MBF))
								break;
						} else
							break;
					}
					i++;
				}
				tchr = uttline[i];
				uttline[i] = EOS;
				morphCnt = countMorphs(uttline, oPos); // uS.ismorfchar
				morf = morf + morphCnt;
				uttline[i] = tchr;
			}
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && isPSDFound) {
				if (mluUtt > 0.0)
					mluUtt--;
				else if (mluUtt == 0.0)
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (mluUtt > 0.0)
					mluUtt--;
				else if (mluUtt == 0.0)
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
				curPSDFound = TRUE;
			if (evald_isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
					mluUtt = mluUtt + (float)1.0;
					ts->tUtt = ts->tUtt + (float)1.0;
					isWordsFound = FALSE;
				}
			}
			if (uttline[i])
				i++;
			else
				break;
		} while (uttline[i]) ;
		if (!isSkip) {
			ts->mluWords   = mluWords;
			ts->morf	   = morf;
			ts->mluUtt	   = mluUtt;
			ts->isPSDFound = curPSDFound;
		}
		for (i=0; spareTier1[i] != EOS; i++) {
			if (spareTier1[i] == HIDEN_C && isdigit(spareTier1[i+1])) {
				if (getMediaTagInfo(spareTier1+i, &stime, &etime)) {
					tNum = etime;
					tNum = tNum - stime;
					tNum = tNum / 1000.0000;
					ts->tm = ts->tm + roundFloat(tNum);
				}
			}
		}
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
			if (uS.isSqCodes(word, tword, &dFnt, FALSE)) {
				if (!strcmp(tword, "[+ *]")   || !strcmp(tword, "[+ jar]") || !strcmp(tword, "[+ es]") ||
					!strcmp(tword, "[+ cir]") || !strcmp(tword, "[+ per]") || !strcmp(tword, "[+ gram]")) {
					ts->uerr++;
				}
			} else if (word[0] != '&' && word[0] != '0' && word[0] != '-' && word[0] != '#' && word[0] != ',' &&
						word[0] != ';' && !uS.IsCharUtteranceDel(word, 0)) {
				ts->spWrdCnt++;
			}
		}
		isAuxFound = FALSE;
/*
	clause count
	if (v || cop*)
		ts->clause++;
	else if (isAuxFound && part)
		ts->clause++;
*/

		char isCopAux, isIf, isEach, isHow, isAnd, isOr, isFound[1024], resOth;
		short wi, isGoing, isGoing_to, isDetNum, adj[9], min, isLike, isKind,
			minPos, isBoth, isEither, isNeither, isNeg, isFor, isTo, isCop_pro, isCop_oth;
		wi = 0;
		resOth   = 0;
		isCopAux = FALSE;
		isIf     = FALSE;
		isEach   = FALSE;
		isHow    = FALSE;
		isAnd    = FALSE;
		isOr     = FALSE;
		isGoing  = -1;
		isGoing_to= -1;
		isDetNum = -1;
		for (j=0; j < 9; j++)
			adj[j] = -1;
		isBoth   = -1;
		isEither = -1;
		isNeither= -1;
		isNeg    = -1;
		isFor    = -1;
		isTo     = -1;
		isCop_pro= -1;
		isCop_oth= -1;
		isLike   = -1;
		isKind   = -1;
		isFiller = TRUE;
		strcpy(spareTier3, uttline);
		for (i=0; spareTier3[i] != EOS; i++) {
			if (spareTier3[i] == '$' || spareTier3[i] == '~')
				spareTier3[i] = ' ';
		}
		i = 0;
		while ((i=getword(utterance->speaker, spareTier3, templineC2, NULL, i))) {
			if (!isWordFromMORTier(templineC2))
				continue;
			isFound[wi] = FALSE;
			strcpy(tword, templineC2);
			if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
				evald_error(db, TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
				if (isEqual("adj", feat->pos) || isEqual("adj:pred", feat->pos) || isEqual("n:gerund", feat->pos) ||
					isEqual("adv", feat->pos) || isEqual("adv:int", feat->pos) ||
					isEqual("adv:tem", feat->pos) || isEqual("adv:int", feat->pos) ||
					isEqual("v", feat->pos) || isEqual("part", feat->pos) || /* isEqual("cop", feat->pos) || */
					isEqual("prep", feat->pos) || /*isEqual("det:num", feat->pos) || */isEqual("neg", feat->pos) ||
					isEqual("pro:int", feat->pos) || isEqual("det:poss", feat->pos) || isEqual("pro:poss", feat->pos) ||
					isEqual("conj", feat->pos) || isEqual("coord", feat->pos) || isEqualIxes("POSS", feat->suffix, NUM_SUFF) ||
					isEqual("qn", feat->pos) || isEqual("pro:rel", feat->pos))
					isFound[wi] = TRUE;
				else if (isEqual("det:dem", feat->pos)) {
					if (isEqual("this", feat->stem) || isEqual("that", feat->stem) ||
						isEqual("those", feat->stem) || isEqual("these", feat->stem))
						isFound[wi] = TRUE;
				}
				if (!isEqual("and", feat->stem) && !isEqual("or", feat->stem) && !isEqual("but", feat->stem) &&
					!isEqual("if", feat->stem) && !isEqual("that", feat->stem) && !isEqual("just", feat->stem) &&
					!isEqual("you", feat->stem) && !isEqual("know", feat->stem))
					isFiller = FALSE;

				if (isGoing_to > 0) {
					if (isEqual("v", feat->pos) || isEqual("cop", feat->pos)) {
						if (isGoing >= 0)
							isFound[isGoing] = FALSE;
						isFound[isGoing_to] = FALSE;
					}
				}
				if (isGoing >= 0) {
					if (isGoing_to == -1 && isEqual("to", feat->stem))
						isGoing_to = wi;
					else {
						isGoing = -1;
						isGoing_to = -1;
					}
				}
				if (isEqual("part", feat->pos) && isEqual("go", feat->stem) && isEqualIxes("PRESP", feat->suffix, NUM_SUFF))
					isGoing = wi;
				if (isCop_pro >= 0) {
					if (isPRO(feat)) {
						isFound[isCop_pro] = TRUE;
						isCop_pro= -1;
						isCop_oth= -1;
					} else
						isCop_pro= -1;
				}
				if (isCop_oth >= 0) {
					resOth = isOTH(feat, resOth);
					if (resOth == 3) {
						isFound[isCop_oth] = TRUE;
						isCop_pro= -1;
						isCop_oth= -1;
						resOth = 0;
					} else if (resOth == 0) {
						resOth = 0;
						isCop_oth= -1;
					}
				}
				if (isEqual("cop", feat->pos)) {
					isCop_pro = wi;
					isCop_oth = wi;
					resOth = 1;
				}
				if (isKind >= 0 && isEqual("prep", feat->pos) && isEqual("of", feat->stem))
					isFound[isKind] = FALSE;
				if (isEqual("adj", feat->pos) && isEqual("kind", feat->stem)) {
					isKind = wi;
				} else
					isKind = -1;
				if (isLike >= 0 && (isEqual("v", feat->pos) || isEqual("co", feat->pos) || isEqual("cop", feat->pos) || isEqual("aux", feat->pos)))
					isFound[isLike] = FALSE;
				if (isEqual("like", feat->stem)) {
					if (wi == 0) {
						isFound[wi] = FALSE;
						isLike = -1;
					} else
						isLike = wi;
				} else
					isLike = -1;
				if (isCopAux && isEqual("like", feat->stem)) {
					isFound[wi] = FALSE;
					isCopAux = FALSE;
				} else if (isEqual("cop", feat->pos) || isEqual("aux", feat->pos))
					isCopAux = TRUE;
				else
					isCopAux = FALSE;
				if (isEqual("adj", feat->pos)) {
					min = wi;
					minPos = -1;
					for (j=0; j < 9; j++) {
						if (adj[j] >= 0 && adj[j] < min) {
							minPos = j;
							min = adj[j];
						}
					}
					if (minPos > -1) {
						isFound[min] = FALSE;
						adj[minPos] = -1;
					}
				}
				for (j=0; j < 9; j++) {
					if (adj[j] >= 0 && (wi - adj[j]) > 2)
						adj[j] = -1;
				}
				if (isEqual("look", feat->stem))
					adj[0] = wi;
				else if (isEqual("seem", feat->stem))
					adj[1] = wi;
				else if (isEqual("appear", feat->stem))
					adj[2] = wi;
				else if (isEqual("feel", feat->stem))
					adj[3] = wi;
				else if (isEqual("smell", feat->stem))
					adj[4] = wi;
				else if (isEqual("taste", feat->stem))
					adj[5] = wi;
				else if (isEqual("make", feat->stem))
					adj[6] = wi;
				else if (isEqual("turn", feat->stem))
					adj[7] = wi;
				else if (isEqual("paint", feat->stem))
					adj[8] = wi;
				if (isIf && isEqual("then", feat->stem)) {
					isFound[wi] = FALSE;
					isIf = FALSE;
				}
				if (isEqual("if", feat->stem))
					isIf = TRUE;
				if (isDetNum >= 0) {
					if (isEqual("n", feat->pos)) {
						isFound[isDetNum] = TRUE;
						isDetNum = -1;
					} else if (wi - isDetNum > 5)
						isDetNum = -1;
				}
				if (isEqual("det:num", feat->pos))
					isDetNum = wi;
				if (isEach && isEqual("other", feat->stem)) {
					isFound[wi] = FALSE;
				}
				if (isEqual("each", feat->stem))
					isEach = TRUE;
				else
					isEach = FALSE;
				if (isHow && (isEqual("come", feat->stem) || isEqual("many", feat->stem))) {
					isFound[wi] = FALSE;
				}
				if (isEqual("how", feat->stem))
					isHow = TRUE;
				else
					isHow = FALSE;
				if (isAnd && isEqual("then", feat->stem)) {
					isFound[wi] = FALSE;
				}
				if (isEqual("and", feat->stem))
					isAnd = TRUE;
				else
					isAnd = FALSE;
				if (isOr && isEqual("else", feat->stem)) {
					isFound[wi] = FALSE;
				}
				if (isEqual("or", feat->stem))
					isOr = TRUE;
				else
					isOr = FALSE;
				if (isBoth >= 0) {
					if (isEqual("and", feat->stem)) {
						isFound[isBoth] = FALSE;
						isBoth = -1;
					} else if (wi - isBoth > 2)
						isBoth = -1;
				}
				if (isEqual("qn", feat->pos) && isEqual("both", feat->stem))
					isBoth = wi;
				if (isEither >= 0) {
					if (isEqual("or", feat->stem)) {
						isFound[isEither] = FALSE;
						isEither = -1;
					} else if (wi - isEither > 2)
						isEither = -1;
				}
				if (isEqual("qn", feat->pos) && isEqual("either", feat->stem))
					isEither = wi;
				if (isNeither >= 0) {
					if (isEqual("nor", feat->stem)) {
						isFound[isNeither] = FALSE;
						isNeither = -1;
					} else if (wi - isNeither > 2)
						isNeither = -1;
				}
				if (isEqual("qn", feat->pos) && isEqual("neither", feat->stem))
					isNeither = wi;
				if (isNeg >= 0) {
					if (isEqual("unless", feat->stem) ||
						(isEqualIxes("un", feat->prefix, NUM_PREF) && isEqual("adj", feat->pos) && isEqual("less", feat->stem))) {
						isFound[wi] = FALSE;
						isNeg = -1;
					} else if (wi - isNeg > 2)
						isNeg = -1;
				}
				if (isEqual("neg", feat->pos))
					isNeg = wi;
				if (isTo >= 0) {
					if (isEqual("v", feat->pos) || isEqual("cop", feat->pos)) {
						if (isFor >= 0)
							isFound[isFor] = FALSE;
						isFound[isTo] = FALSE;
					}
					isFor = -1;
					isTo  = -1;
				}
				if (isFor >= 0) {
					if (isEqual("to", feat->stem)) {
						isTo = wi;
					} else if (isTo == -1 && wi - isFor > 2) {
						isFor = -1;
						isTo  = -1;
					}
				}
				if (isEqual("for", feat->stem))
					isFor = wi;

				// counts nouns/verbes BEG
				if (isEqual("noun", feat->pos) || isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
					ts->nounsNV++;
				} else if (isAllv(feat) || isnEqual("cop", feat->pos, 3)) {
					ts->verbsNV++;
				}
				// counts nouns/verbes END
				// counts open/closed BEG
				if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isEqual("v", feat->pos) || isnEqual("v:", feat->pos, 2) ||
					isEqual("noun", feat->pos) || isEqual("verb", feat->pos) ||
					isnEqual("part", feat->pos, 3) || isnEqual("cop", feat->pos, 3) || isEqual("adj", feat->pos) ||
					isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) {
					ts->openClass++;
				} else if (isEqual("co", feat->pos) == FALSE && isEqual("on", feat->pos) == FALSE) {
					ts->closedClass++;
				}
				// counts open/closed END
				if (isEqual("noun", feat->pos) || isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
					ts->noun++;
				} else if (isAllv(feat) || isnEqual("cop", feat->pos, 3)) {
					ts->verb++;
					if (isEqual("part", feat->pos)) {
						if (isAuxFound)
							ts->CUR++;
					} else if (isEqual("verb", feat->pos) || isEqual("v", feat->pos) || isnEqual("cop", feat->pos, 3))
						ts->CUR++;
				} else if (isEqual("aux", feat->pos)) {
					ts->aux++;
					isAuxFound = TRUE;
				} else if (isnEqual("aux:", feat->pos, 4)) {
					ts->aux++;
				} else if (isEqual("mod", feat->pos)) {
					ts->mod++;
				} else if (isEqual("part", feat->pos)) {
					if (isAuxFound)
						ts->CUR++;
				} else if (isEqual("adp", feat->pos) || isEqual("prep", feat->pos) || isnEqual("prep:", feat->pos, 5)) {
					ts->prep++;
				} else if (isEqual("adj", feat->pos) || isnEqual("adj:", feat->pos, 4)) {
					ts->adj++;
				} else if (isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) {
					ts->adv++;
				} else if (isEqual("cconj", feat->pos) || isEqual("conj", feat->pos) || isnEqual("conj:", feat->pos, 5)) {
					ts->conj++;
				} else if (isEqual("pron", feat->pos) || isEqual("pro", feat->pos) || isnEqual("pro:", feat->pos, 4)) {
					ts->pron++;
				} else if (isEqual("det:art", feat->pos) || isEqual("det:dem", feat->pos) || isEqual("det:int", feat->pos)) {
					ts->det++;
				} else if (isEqual("det", feat->pos) && 
						   (isEqualIxes("art", feat->suffix, NUM_SUFF) || isEqualIxes("dem", feat->suffix, NUM_SUFF) ||
							isEqualIxes("int", feat->suffix, NUM_SUFF))) {
					ts->det++;
				}
				if (isEqualIxes("3s", feat->suffix, NUM_SUFF) || isEqualIxes("3s", feat->fusion, NUM_FUSI)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:03s*") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:vun*"))
						j = 0;
					else
						ts->thrS++;
				}
				if (isEqualIxes("13s", feat->suffix, NUM_SUFF) || isEqualIxes("13s", feat->fusion, NUM_FUSI)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:03s*") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:vun*"))
						j = 0;
					else
						ts->thrnS++;
				}
				if (!isEqual("part", feat->pos) &&
					(isEqualIxes("past", feat->suffix, NUM_SUFF) || isEqualIxes("past", feat->fusion, NUM_FUSI))) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed"))
						j = 0;
					else
						ts->past++;
				}
				if (isEqualIxes("pastp", feat->suffix, NUM_SUFF) || isEqualIxes("pastp", feat->fusion, NUM_FUSI) ||
					(isEqual("part", feat->pos) && isEqualIxes("past", feat->suffix, NUM_SUFF))) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:en") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=en"))
						j = 0;
					else
						ts->pastp++;
				}
				if (isEqualIxes("presp", feat->suffix, NUM_SUFF) || isEqualIxes("presp", feat->fusion, NUM_FUSI) ||
					(isEqual("part", feat->pos) && isEqualIxes("pres", feat->suffix, NUM_SUFF))) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing"))
						j = 0;
					else
						ts->presp++;
				}
				if (isEqualIxes("Plur", feat->suffix, NUM_SUFF) ||
					isEqualIxes("pl", feat->suffix, NUM_SUFF) || isEqualIxes("pl", feat->fusion, NUM_FUSI)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:s*") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0s*")     ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=s"))
						j = 0;
					else
						ts->pl++;
				}
			}
			freeUpFeats(&word_feats);
			wi++;
		}
		if (isFiller) {
			for (i=0; i < wi; i++) {
				isFound[i] = FALSE;
			}
		}
		if (wi > 0) {
			if (strncmp(tword, "prep|to", 7) == 0) {
				isFound[wi-1] = FALSE;
			}
			for (i=0; i < wi; i++) {
				if (isFound[i] == TRUE)
					ts->density++;
			}
			if (isPWordsList && PWordsListFP != NULL) {
				char *done, *s;

				i = 0;
				wi = 0;
				while ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
					if (strchr(templineC2, '|') == NULL)
						continue;
					s = templineC2;
					do {
						if (isFound[wi] == TRUE)
							fprintf(PWordsListFP, "P");
						else
							fprintf(PWordsListFP, " ");
						wi++;
						done = strchr(s, '$');
						if (done != NULL)
							s = done + 1;
						else {
							done = strchr(s, '~');
							if (done != NULL)
								s = done + 1;
						}
					} while (done != NULL) ;
					if (isFound[wi] == TRUE)
						fprintf(PWordsListFP, "\t%s\n", templineC2);
					else
						fprintf(PWordsListFP, "\t%s\n", templineC2);
				}
				i = 0;
				while ((i=getword(utterance->speaker, utterance->line, templineC2, NULL, i))) {
					for (wi=0; templineC2[wi] != EOS; wi++) {
						if (evald_isUttDel(templineC2, wi)) {
							fprintf(PWordsListFP, "\t%s\n", templineC2);
							break;
						}
					}
					if (templineC2[wi] != EOS)
						break;
				}
				fprintf(PWordsListFP, "\n");
			}
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
		if (db == NULL) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				if (isCreateDB) {
					uS.remblanks(utterance->line);
					filterAndOuputID(utterance->line);
				} else {
					uS.remblanks(utterance->line);
					evald_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
				}
			}
		} else if (rightIDFound != NULL) {
			uS.remblanks(utterance->line);
			breakIDsIntoFields(&IDTier, utterance->line);
			if (isKeyMatch(&IDTier))  {
				*rightIDFound = TRUE;
				maketierchoice(IDTier.code, '+', '\002');
			}
		}
	} else if (isOutputGem && uS.partcmp(utterance->speaker,"@Time Duration:",FALSE,FALSE)) {
		if (isCreateDB) {
			for (i=0; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '\n' || utterance->line[i] == '\t')
					utterance->line[i] = ' ';
			}
			uS.remFrontAndBackBlanks(utterance->line);
			removeExtraSpace(utterance->line);
			fprintf(dbfpout, "T%s\n", utterance->line);
			return;
		}
		uS.remFrontAndBackBlanks(utterance->line);
		if (ts == NULL) {
			if (tmDur == -1.0)
				tmDur = 0.0;
			tmDur = tmDur + getTimeDuration(utterance->line);
		} else
			ts->tm = ts->tm + getTimeDuration(utterance->line);
	}
}

static void evald_outputtree(struct evald_words *p) {
	if (p != NULL) {
		evald_outputtree(p->left);
		if (p->wused)
			fprintf(dbfpout, "%d ", p->wnum);
//			fprintf(dbfpout, "%s ", p->word);
		evald_outputtree(p->right);
	}
}

static char *extractFloatFromLine(char *line, float *val, struct database *db) {
	float num;

	if (*line == EOS) {
		fprintf(stderr, "\nERROR: Database is corrupt\n");
		evald_error(db, FALSE);
	}
	num = atof(line);
	*val = *val + num;
	while (isdigit(*line) || *line == '.')
		line++;
	while (isSpace(*line))
		line++;
	return(line);
}

static void retrieveTS(struct evald_speakers *ts, char *line, struct database *db) {
	char  *e;

	while (isSpace(*line))
		line++;

	line = extractFloatFromLine(line, &ts->tm, db);
	line = extractFloatFromLine(line, &ts->spWrdCnt, db);
	line = extractFloatFromLine(line, &ts->frTokens, db);
	line = extractFloatFromLine(line, &ts->CUR, db);
	line = extractFloatFromLine(line, &ts->nounsNV, db);
	line = extractFloatFromLine(line, &ts->verbsNV, db);
	line = extractFloatFromLine(line, &ts->mluWords, db);
	line = extractFloatFromLine(line, &ts->morf, db);
	line = extractFloatFromLine(line, &ts->tUtt, db);
	line = extractFloatFromLine(line, &ts->mluUtt, db);
	line = extractFloatFromLine(line, &ts->werr, db);
	line = extractFloatFromLine(line, &ts->uerr, db);
	line = extractFloatFromLine(line, &ts->morTotal, db);
	line = extractFloatFromLine(line, &ts->density, db);
	line = extractFloatFromLine(line, &ts->noun, db);
	line = extractFloatFromLine(line, &ts->verb, db);
	line = extractFloatFromLine(line, &ts->aux, db);
	line = extractFloatFromLine(line, &ts->mod, db);
	line = extractFloatFromLine(line, &ts->prep, db);
	line = extractFloatFromLine(line, &ts->adj, db);
	line = extractFloatFromLine(line, &ts->adv, db);
	line = extractFloatFromLine(line, &ts->conj, db);
	line = extractFloatFromLine(line, &ts->pron, db);
	line = extractFloatFromLine(line, &ts->det, db);
	line = extractFloatFromLine(line, &ts->thrS, db);
	line = extractFloatFromLine(line, &ts->thrnS, db);
	line = extractFloatFromLine(line, &ts->past, db);
	line = extractFloatFromLine(line, &ts->pastp, db);
	line = extractFloatFromLine(line, &ts->pl, db);
	line = extractFloatFromLine(line, &ts->presp, db);
	line = extractFloatFromLine(line, &ts->openClass, db);
	line = extractFloatFromLine(line, &ts->closedClass, db);
	line = extractFloatFromLine(line, &ts->retr, db);
	line = extractFloatFromLine(line, &ts->repeat, db);

	while (*line != EOS) {
		e = strchr(line, ' ');
		if (e != NULL)
			*e = EOS;
		ts->words_root = evald_FREQ_tree(ts->words_root, line, ts, db);
		if (e == NULL)
			break;
		line = e + 1;
		while (isSpace(*line))
			line++;
	}
}

#ifdef DEBUGEVALRESULTS
static void prDebug(struct evald_speakers *ts) {
	fprintf(stdout, "tm=%.0f\n", ts->tm);
	fprintf(stdout, "spWrdCnt=%.0f\n", ts->spWrdCnt);
	fprintf(stdout, "frTokens=%.0f\n", ts->frTokens);
	fprintf(stdout, "frTypes=%.0f\n", ts->frTypes);
	fprintf(stdout, "CUR=%.0f\n", ts->CUR);
	fprintf(stdout, "nounsNV=%.0f\n", ts->nounsNV);
	fprintf(stdout, "verbsNV=%.0f\n", ts->verbsNV);
	fprintf(stdout, "mluWords=%.0f\n", ts->mluWords);
	fprintf(stdout, "morf=%.0f\n", ts->morf);
	fprintf(stdout, "tUtt=%.0f\n", ts->tUtt);
	fprintf(stdout, "mluUtt=%.0f\n", ts->mluUtt);
	fprintf(stdout, "werr=%.0f\n", ts->werr);
	fprintf(stdout, "uerr=%.0f\n", ts->uerr);
	fprintf(stdout, "morTotal=%.0f\n", ts->morTotal);
	fprintf(stdout, "density=%.0f\n", ts->density);
	fprintf(stdout, "noun=%.0f\n", ts->noun);
	fprintf(stdout, "verb=%.0f\n", ts->verb);
	fprintf(stdout, "aux=%.0f\n", ts->aux);
	fprintf(stdout, "mod=%.0f\n", ts->mod);
	fprintf(stdout, "prep=%.0f\n", ts->prep);
	fprintf(stdout, "adj=%.0f\n", ts->adj);
	fprintf(stdout, "adv=%.0f\n", ts->adv);
	fprintf(stdout, "conj=%.0f\n", ts->conj);
	fprintf(stdout, "pron=%.0f\n", ts->pron);
	fprintf(stdout, "det=%.0f\n", ts->det);
	fprintf(stdout, "thrS=%.0f\n", ts->thrS);
	fprintf(stdout, "thrnS=%.0f\n", ts->thrnS);
	fprintf(stdout, "past=%.0f\n", ts->past);
	fprintf(stdout, "pastp=%.0f\n", ts->pastp);
	fprintf(stdout, "pl=%.0f\n", ts->pl);
	fprintf(stdout, "presp=%.0f\n", ts->presp);
	fprintf(stdout, "openClass=%.0f\n", ts->openClass);
	fprintf(stdout, "closedClass=%.0f\n", ts->closedClass);
	fprintf(stdout, "retr=%.0f\n", ts->retr);
	fprintf(stdout, "repeat=%.0f\n", ts->repeat);
}
#endif

static void prTSResults(struct evald_speakers *ts) {
	const char *format;

	format = "%.0f ";
//	format = "%f ";
	fprintf(dbfpout, format, ts->tm);
	fprintf(dbfpout, format, ts->spWrdCnt);
	fprintf(dbfpout, format, ts->frTokens);
	fprintf(dbfpout, format, ts->CUR);
	fprintf(dbfpout, format, ts->nounsNV);
	fprintf(dbfpout, format, ts->verbsNV);
	fprintf(dbfpout, format, ts->mluWords);
	fprintf(dbfpout, format, ts->morf);
	fprintf(dbfpout, format, ts->tUtt);
	fprintf(dbfpout, format, ts->mluUtt);
	fprintf(dbfpout, format, ts->werr);
	fprintf(dbfpout, format, ts->uerr);
	fprintf(dbfpout, format, ts->morTotal);
	fprintf(dbfpout, format, ts->density);
	fprintf(dbfpout, format, ts->noun);
	fprintf(dbfpout, format, ts->verb);
	fprintf(dbfpout, format, ts->aux);
	fprintf(dbfpout, format, ts->mod);
	fprintf(dbfpout, format, ts->prep);
	fprintf(dbfpout, format, ts->adj);
	fprintf(dbfpout, format, ts->adv);
	fprintf(dbfpout, format, ts->conj);
	fprintf(dbfpout, format, ts->pron);
	fprintf(dbfpout, format, ts->det);
	fprintf(dbfpout, format, ts->thrS);
	fprintf(dbfpout, format, ts->thrnS);
	fprintf(dbfpout, format, ts->past);
	fprintf(dbfpout, format, ts->pastp);
	fprintf(dbfpout, format, ts->pl);
	fprintf(dbfpout, format, ts->presp);
	fprintf(dbfpout, format, ts->openClass);
	fprintf(dbfpout, format, ts->closedClass);
	fprintf(dbfpout, format, ts->retr);
	fprintf(dbfpout, format, ts->repeat);
	evald_outputtree(ts->words_root);
	putc('\n', dbfpout);
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
		return((evald_group == FALSE && found) || (evald_SpecWords == found));
	else 
		return((evald_group == TRUE && evald_SpecWords > found) || (found == 0));
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  found, t;
	char isfirst, isOutputGem, rightIDFound, isDataFound;
	char word[BUFSIZ+1], DBFname[BUFSIZ+1], IDText[BUFSIZ+1];
	float tt;
	FILE *tfpin;
	struct evald_speakers *ts;
	struct IDparts IDTier;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	isfirst = TRUE;
	if (isNOptionSet == FALSE) {
		strcpy(evald_BBS, "@*&#");
		strcpy(evald_CBS, "@*&#");
	}
	if (evald_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	if (!fgets_cr(templineC, UTTLINELEN, fpin))
		return;
	if (templineC[0] != 'V' || !isdigit(templineC[1])) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		evald_error(db, FALSE);
	}
	t = atoi(templineC+1);
	if (t != EVAL_DB_VERSION) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		evald_error(db, FALSE);
	}
	isDataFound = FALSE;
	rightIDFound = FALSE;
	found = 0;
	IDText[0] = EOS;
	word[0] = EOS;
	DBFname[0] = EOS;
	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (templineC[0] == '=') {
			uS.remblanks(templineC+1);
			if (templineC[1] == '/')
				strncpy(DBFname, templineC+24, BUFSIZ);
			else
				strncpy(DBFname, templineC+1, BUFSIZ);
			DBFname[BUFSIZ] = EOS;
		} else if (templineC[0] == '+') {
			uS.remblanks(templineC+1);
			strncpy(IDText, templineC+1, BUFSIZ);
			IDText[BUFSIZ] = EOS;
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
			if (isDataFound && DBFilesListFP != NULL) {
				fprintf(DBFilesListFP, "-----\t%s\n", DBFname);
				fprintf(DBFilesListFP, "@ID:\t%s\n", IDText);
			}
			isDataFound = FALSE;
			if (db->db_sp != NULL) {
				*isDbSpFound = TRUE;
				ts = db->db_sp;
				if (!ts->isSpeakerFound) {
					if (evald_SpecWords)
						fprintf(stderr,"\nERROR: No specified gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					evald_error(db, FALSE);
				}
				db->tm_sqr = db->tm_sqr + (ts->tm * ts->tm);
				db->tm = db->tm + ts->tm;
				if (db->mn_tm == (float)0.0 || db->mn_tm > ts->tm)
					db->mn_tm = ts->tm;
				if (db->mx_tm < ts->tm)
					db->mx_tm = ts->tm;
#ifdef DEBUGEVALRESULTS
				fprintf(stdout, "%s\n", IDText);
				prDebug(ts);
#endif
				db->tUtt_sqr = db->tUtt_sqr + (ts->tUtt * ts->tUtt);
				db->tUtt = db->tUtt + ts->tUtt;
				if (db->mn_tUtt == (float)0.0 || db->mn_tUtt > ts->tUtt)
					db->mn_tUtt = ts->tUtt;
				if (db->mx_tUtt < ts->tUtt)
					db->mx_tUtt = ts->tUtt;
				db->frTokens_sqr = db->frTokens_sqr + (ts->frTokens * ts->frTokens);
				db->frTokens = db->frTokens + ts->frTokens;
				if (db->mn_frTokens == (float)0.0 || db->mn_frTokens > ts->frTokens)
					db->mn_frTokens = ts->frTokens;
				if (db->mx_frTokens < ts->frTokens)
					db->mx_frTokens = ts->frTokens;
				db->frTypes_sqr = db->frTypes_sqr + (ts->frTypes * ts->frTypes);
				db->frTypes = db->frTypes + ts->frTypes;
				if (db->mn_frTypes == (float)0.0 || db->mn_frTypes > ts->frTypes)
					db->mn_frTypes = ts->frTypes;
				if (db->mx_frTypes < ts->frTypes)
					db->mx_frTypes = ts->frTypes;
				if (ts->frTokens == 0.0)
					tt = 0.0;
				else
					tt = ts->frTypes / ts->frTokens;
				db->TTR_sqr = db->TTR_sqr + (tt * tt);
				db->TTR = db->TTR + tt;
				if (db->mn_TTR == (float)0.0 || db->mn_TTR > tt)
					db->mn_TTR = tt;
				if (db->mx_TTR < tt)
					db->mx_TTR = tt;
				if (ts->tm == 0.0)
					tt = 0.0;
				else {
					tt = ts->frTokens / (ts->tm / 60.0000);
				}
				db->WOD_sqr = db->WOD_sqr + (tt * tt);
				db->WOD = db->WOD + tt;
				if (db->mn_WOD == (float)0.0 || db->mn_WOD > tt)
					db->mn_WOD = tt;
				if (db->mx_WOD < tt)
					db->mx_WOD = tt;
				if (ts->tUtt == 0.0)
					tt = 0.0;
				else
					tt = ts->CUR / ts->tUtt;
				db->CUR_sqr = db->CUR_sqr + (tt * tt);
				db->CUR = db->CUR + tt;
				if (db->mn_CUR == (float)0.0 || db->mn_CUR > tt)
					db->mn_CUR = tt;
				if (db->mx_CUR < tt)
					db->mx_CUR = tt;

				if (ts->verbsNV == 0.0)
					tt = 0.0;
				else
					tt = ts->nounsNV / ts->verbsNV;
				db->NoV_sqr = db->NoV_sqr + (tt * tt);
				db->NoV = db->NoV + tt;
				if (db->mn_NoV == (float)0.0 || db->mn_NoV > tt)
					db->mn_NoV = tt;
				if (db->mx_NoV < tt)
					db->mx_NoV = tt;

				db->openClass_sqr = db->openClass_sqr + (ts->openClass * ts->openClass);
				db->openClass = db->openClass + ts->openClass;
				if (db->mn_openClass == (float)0.0 || db->mn_openClass > ts->openClass)
					db->mn_openClass = ts->openClass;
				if (db->mx_openClass < ts->openClass)
					db->mx_openClass = ts->openClass;

				db->closedClass_sqr = db->closedClass_sqr + (ts->closedClass * ts->closedClass);
				db->closedClass = db->closedClass + ts->closedClass;
				if (db->mn_closedClass == (float)0.0 || db->mn_closedClass > ts->closedClass)
					db->mn_closedClass = ts->closedClass;
				if (db->mx_closedClass < ts->closedClass)
					db->mx_closedClass = ts->closedClass;

				if (ts->closedClass == 0.0)
					tt = 0.0;
				else
					tt = ts->openClass / ts->closedClass;
				db->OPoCl_sqr = db->OPoCl_sqr + (tt * tt);
				db->OPoCl = db->OPoCl + tt;
				if (db->mn_OPoCl == (float)0.0 || db->mn_OPoCl > tt)
					db->mn_OPoCl = tt;
				if (db->mx_OPoCl < tt)
					db->mx_OPoCl = tt;

				db->mluUtt_sqr = db->mluUtt_sqr + (ts->mluUtt * ts->mluUtt);
				db->mluUtt = db->mluUtt + ts->mluUtt;
				if (db->mn_mluUtt == (float)0.0 || db->mn_mluUtt > ts->mluUtt)
					db->mn_mluUtt = ts->mluUtt;
				if (db->mx_mluUtt < ts->mluUtt)
					db->mx_mluUtt = ts->mluUtt;
				if (ts->mluUtt == 0.0)
					ts->mluWords  = 0.0;
				else
					ts->mluWords = ts->mluWords / ts->mluUtt;
				db->mluWords_sqr = db->mluWords_sqr + (ts->mluWords * ts->mluWords);
				db->mluWords = db->mluWords + ts->mluWords;
				if (db->mn_mluWords == (float)0.0 || db->mn_mluWords > ts->mluWords)
					db->mn_mluWords = ts->mluWords;
				if (db->mx_mluWords < ts->mluWords)
					db->mx_mluWords = ts->mluWords;
				if (ts->mluUtt == 0.0)
					ts->morf  = 0.0;
				else
					ts->morf = ts->morf / ts->mluUtt;
				db->morf_sqr = db->morf_sqr + (ts->morf * ts->morf);
				db->morf = db->morf + ts->morf;
				if (db->mn_morf == (float)0.0 || db->mn_morf > ts->morf)
					db->mn_morf = ts->morf;
				if (db->mx_morf < ts->morf)
					db->mx_morf = ts->morf;
				if (!isRawVal) {
					if (ts->spWrdCnt == 0.0)
						ts->werr = 0.0;
					else
						ts->werr = (ts->werr / ts->spWrdCnt) * 100.0000;
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
				db->morTotal_sqr = db->morTotal_sqr + (ts->morTotal * ts->morTotal);
				db->morTotal = db->morTotal + ts->morTotal;
				if (db->mn_morTotal == (float)0.0 || db->mn_morTotal > ts->morTotal)
					db->mn_morTotal = ts->morTotal;
				if (db->mx_morTotal < ts->morTotal)
					db->mx_morTotal = ts->morTotal;
				if (ts->morTotal == 0.0)
					ts->density = 0.000;
				else
					ts->density = ts->density / ts->morTotal;
				db->density_sqr = db->density_sqr + (ts->density * ts->density);
				db->density = db->density + ts->density;
				if (db->mn_density == (float)0.0 || db->mn_density > ts->density)
					db->mn_density = ts->density;
				if (db->mx_density < ts->density)
					db->mx_density = ts->density;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->noun = 0.000;
					else
						ts->noun = (ts->noun/ts->morTotal)*100.0000;
				}
				db->noun_sqr = db->noun_sqr + (ts->noun * ts->noun);
				db->noun = db->noun + ts->noun;
				if (db->mn_noun == (float)0.0 || db->mn_noun > ts->noun)
					db->mn_noun = ts->noun;
				if (db->mx_noun < ts->noun)
					db->mx_noun = ts->noun;
				if (!isRawVal) {
					if (ts->noun == 0.0)
						ts->pl = 0.000;
					else
						ts->pl = (ts->pl/ts->noun)*100.0000;
				}
				db->pl_sqr = db->pl_sqr + (ts->pl * ts->pl);
				db->pl = db->pl + ts->pl;
				if (db->mn_pl == (float)0.0 || db->mn_pl > ts->pl)
					db->mn_pl = ts->pl;
				if (db->mx_pl < ts->pl)
					db->mx_pl = ts->pl;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->verb = 0.000;
					else
						ts->verb = (ts->verb/ts->morTotal)*100.0000;
				}
				db->verb_sqr = db->verb_sqr + (ts->verb * ts->verb);
				db->verb = db->verb + ts->verb;
				if (db->mn_verb == (float)0.0 || db->mn_verb > ts->verb)
					db->mn_verb = ts->verb;
				if (db->mx_verb < ts->verb)
					db->mx_verb = ts->verb;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->aux = 0.000;
					else
						ts->aux = (ts->aux/ts->morTotal)*100.0000;
				}
				db->aux_sqr = db->aux_sqr + (ts->aux * ts->aux);
				db->aux = db->aux + ts->aux;
				if (db->mn_aux == (float)0.0 || db->mn_aux > ts->aux)
					db->mn_aux = ts->aux;
				if (db->mx_aux < ts->aux)
					db->mx_aux = ts->aux;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->mod = 0.000;
					else
						ts->mod = (ts->mod/ts->morTotal)*100.0000;
				}
				db->mod_sqr = db->mod_sqr + (ts->mod * ts->mod);
				db->mod = db->mod + ts->mod;
				if (db->mn_mod == (float)0.0 || db->mn_mod > ts->mod)
					db->mn_mod = ts->mod;
				if (db->mx_mod < ts->mod)
					db->mx_mod = ts->mod;
				if (!isRawVal) {
					if (ts->verb == 0.0)
						ts->thrS = 0.000;
					else
						ts->thrS = (ts->thrS/ts->verb)*100.0000;
				}
				db->thrS_sqr = db->thrS_sqr + (ts->thrS * ts->thrS);
				db->thrS = db->thrS + ts->thrS;
				if (db->mn_thrS == (float)0.0 || db->mn_thrS > ts->thrS)
					db->mn_thrS = ts->thrS;
				if (db->mx_thrS < ts->thrS)
					db->mx_thrS = ts->thrS;
				if (!isRawVal) {
					if (ts->verb == 0.0)
						ts->thrnS = 0.000;
					else
						ts->thrnS = (ts->thrnS/ts->verb)*100.0000;
				}
				db->thrnS_sqr = db->thrnS_sqr + (ts->thrnS * ts->thrnS);
				db->thrnS = db->thrnS + ts->thrnS;
				if (db->mn_thrnS == (float)0.0 || db->mn_thrnS > ts->thrnS)
					db->mn_thrnS = ts->thrnS;
				if (db->mx_thrnS < ts->thrnS)
					db->mx_thrnS = ts->thrnS;
				if (!isRawVal) {
					if (ts->verb == 0.0)
						ts->past = 0.000;
					else
						ts->past = (ts->past/ts->verb)*100.0000;
				}
				db->past_sqr = db->past_sqr + (ts->past * ts->past);
				db->past = db->past + ts->past;
				if (db->mn_past == (float)0.0 || db->mn_past > ts->past)
					db->mn_past = ts->past;
				if (db->mx_past < ts->past)
					db->mx_past = ts->past;
				if (!isRawVal) {
					if (ts->verb == 0.0)
						ts->pastp = 0.000;
					else
						ts->pastp = (ts->pastp/ts->verb)*100.0000;
				}
				db->pastp_sqr = db->pastp_sqr + (ts->pastp * ts->pastp);
				db->pastp = db->pastp + ts->pastp;
				if (db->mn_pastp == (float)0.0 || db->mn_pastp > ts->pastp)
					db->mn_pastp = ts->pastp;
				if (db->mx_pastp < ts->pastp)
					db->mx_pastp = ts->pastp;
				if (!isRawVal) {
					if (ts->verb == 0.0)
						ts->presp = 0.000;
					else
						ts->presp = (ts->presp/ts->verb)*100.0000;
				}
				db->presp_sqr = db->presp_sqr + (ts->presp * ts->presp);
				db->presp = db->presp + ts->presp;
				if (db->mn_presp == (float)0.0 || db->mn_presp > ts->presp)
					db->mn_presp = ts->presp;
				if (db->mx_presp < ts->presp)
					db->mx_presp = ts->presp;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->prep = 0.000;
					else
						ts->prep = (ts->prep/ts->morTotal)*100.0000;
				}
				db->prep_sqr = db->prep_sqr + (ts->prep * ts->prep);
				db->prep = db->prep + ts->prep;
				if (db->mn_prep == (float)0.0 || db->mn_prep > ts->prep)
					db->mn_prep = ts->prep;
				if (db->mx_prep < ts->prep)
					db->mx_prep = ts->prep;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->adj = 0.000;
					else
						ts->adj = (ts->adj/ts->morTotal)*100.0000;
				}
				db->adj_sqr = db->adj_sqr + (ts->adj * ts->adj);
				db->adj = db->adj + ts->adj;
				if (db->mn_adj == (float)0.0 || db->mn_adj > ts->adj)
					db->mn_adj = ts->adj;
				if (db->mx_adj < ts->adj)
					db->mx_adj = ts->adj;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->adv = 0.000;
					else
						ts->adv = (ts->adv/ts->morTotal)*100.0000;
				}
				db->adv_sqr = db->adv_sqr + (ts->adv * ts->adv);
				db->adv = db->adv + ts->adv;
				if (db->mn_adv == (float)0.0 || db->mn_adv > ts->adv)
					db->mn_adv = ts->adv;
				if (db->mx_adv < ts->adv)
					db->mx_adv = ts->adv;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->conj = 0.000;
					else
						ts->conj = (ts->conj/ts->morTotal)*100.0000;
				}
				db->conj_sqr = db->conj_sqr + (ts->conj * ts->conj);
				db->conj = db->conj + ts->conj;
				if (db->mn_conj == (float)0.0 || db->mn_conj > ts->conj)
					db->mn_conj = ts->conj;
				if (db->mx_conj < ts->conj)
					db->mx_conj = ts->conj;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->det = 0.000;
					else
						ts->det = (ts->det/ts->morTotal)*100.0000;
				}
				db->det_sqr = db->det_sqr + (ts->det * ts->det);
				db->det = db->det + ts->det;
				if (db->mn_det == (float)0.0 || db->mn_det > ts->det)
					db->mn_det = ts->det;
				if (db->mx_det < ts->det)
					db->mx_det = ts->det;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->pron = 0.000;
					else
						ts->pron = (ts->pron/ts->morTotal)*100.0000;
				}
				db->pron_sqr = db->pron_sqr + (ts->pron * ts->pron);
				db->pron = db->pron + ts->pron;
				if (db->mn_pron == (float)0.0 || db->mn_pron > ts->pron)
					db->mn_pron = ts->pron;
				if (db->mx_pron < ts->pron)
					db->mx_pron = ts->pron;
				db->retr_sqr = db->retr_sqr + (ts->retr * ts->retr);
				db->retr = db->retr + ts->retr;
				if (db->mn_retr == (float)0.0 || db->mn_retr > ts->retr)
					db->mn_retr = ts->retr;
				if (db->mx_retr < ts->retr)
					db->mx_retr = ts->retr;
				db->repeat_sqr = db->repeat_sqr + (ts->repeat * ts->repeat);
				db->repeat = db->repeat + ts->repeat;
				if (db->mn_repeat == (float)0.0 || db->mn_repeat > ts->repeat)
					db->mn_repeat = ts->repeat;
				if (db->mx_repeat < ts->repeat)
					db->mx_repeat = ts->repeat;

				db->db_sp = freespeakers(db->db_sp);
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(evald_BBS, "@*&#");
				strcpy(evald_CBS, "@*&#");
			}
			if (evald_SpecWords) {
				isOutputGem = FALSE;
			} else {
				isOutputGem = TRUE;
			}
			rightIDFound = FALSE;
			found = 0;
			IDText[0] = EOS;
			word[0] = EOS;
			DBFname[0] = EOS;
		} else {
			if (templineC[0] == 'G') {
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
			} else if (isdigit(templineC[0])) {
				strcpy(utterance->speaker, "*PAR:");
				strcpy(utterance->line, templineC);
			}
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			if (evald_SpecWords && !strcmp(evald_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					evald_n_option = FALSE;
					strcpy(evald_BBS, "@BG:");
					strcpy(evald_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					evald_n_option = TRUE;
					strcpy(evald_BBS, "@G:");
					strcpy(evald_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,evald_BBS,FALSE,FALSE)) {
				if (evald_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,evald_CBS,FALSE,FALSE)) {
				if (evald_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (evald_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			} else if (isOutputGem) {
				if (utterance->speaker[0] == '*') {
					uS.remFrontAndBackBlanks(utterance->line);
					isDataFound = TRUE;
					strcpy(templineC, utterance->speaker);
					ts = evald_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
					if (ts != NULL && isfirst) {
						isfirst = FALSE;
						db->num = db->num + 1.0;
						t = db->num;
#if defined(_WIN32) && defined(_DEBUG)
						if (t == 1 || t % 15 == 0)
							fprintf(stderr,"\r%.0f ", db->num);
#endif
					}
					if (ts != NULL)
						retrieveTS(ts, utterance->line, db);
				} else
					evald_process_tier(ts, db, NULL, isOutputGem, NULL);
			}
		}
	}
	fpin = tfpin;
}

static void ParseDatabase(struct database *db) {
	char isDbSpFound = FALSE;
	FILE *fp;

	tmDur = -1.0;
// remove beg
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, lang_prefix);
	strcat(FileName1, DATABASE_FILE_NAME);
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, "eval");
		addFilename2Path(FileName1, lang_prefix);
		strcat(FileName1, DATABASE_FILE_NAME);
		fp = fopen(FileName1, "r");
	}
	if (fp == NULL) {
		fprintf(stderr, "Can't find database in folder \"%s\"\n", FileName1);
		fprintf(stderr, "If you do not use database, then do not include +d option in command line\n");
		evald_error(db, FALSE);
	} else {
		fprintf(stderr,"Database File used: %s\n", FileName1);
		OpenDBFile(fp, db, &isDbSpFound);
		fprintf(stderr,"\r%.0f  \n", db->num);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (evald_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		evald_error(db, FALSE);
	}	
	SetNewVol(wd_dir);
}

static void compute_SD(struct SDs *SD, float score, float *div, float sqr_mean, float mean, float num, char *st) {
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
		excelStrCell(fpout, "NA");
		SD->stars = 0;
	} else {
		if (div != NULL && *div != 0.0)
			score = score / (*div);
		mean = mean / num; 
		cSD = (score - mean) / SD->dbSD;
		if (st != NULL) {
			Secs2Str(cSD, st, TRUE);
			excelStrCell(fpout, st);
		} else
			excelNumCell(fpout, "%.3f", cSD);
		if (cSD <= -2.0000 || cSD >= 2.0000)
			SD->stars = 2;
		else if (cSD <= -1.0000 || cSD >= 1.0000)
			SD->stars = 1;
		else
			SD->stars = 0;
	}
}

static void evald_pr_result(void) {
	int    i, SDn = 0;
	char   st[1024], *sFName;
	float  num, tt;
	struct evald_speakers *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;

	if (isCreateDB) {
		sp_head = freespeakers(sp_head);
		return;
	}
	if (sp_head == NULL) {
		if (evald_SpecWords && !onlyApplyToDB) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n\n");
	}
	excelHeader(fpout, newfname, 75);
	excelRow(fpout, ExcelRowStart);
	if (DBKeyRoot != NULL) {
		for (i=0; i < SDRESSIZE; i++) {
			SD[i].isSDComputed = FALSE;
		}
		excelStrCell(fpout, "File_DB");
	} else
		excelStrCell(fpout, "File");
	excelCommasStrCell(fpout, "Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	excelCommasStrCell(fpout, "Duration_(sec),Total_Utts,MLU_Utts,MLU_Words,MLU_Morphemes");
	excelCommasStrCell(fpout, "FREQ_types,FREQ_tokens,FREQ_TTR,Words_Min,Verbs_Utt");
	if (!isRawVal) {
		excelCommasStrCell(fpout, "%_Word_Errors,Utt_Errors");
	} else {
		excelCommasStrCell(fpout, "Word_Errors,Utt_Errors");
	}
	if (langType == ENG)
		excelStrCell(fpout, "density");
	if (!isRawVal) {
		excelCommasStrCell(fpout, "%_Nouns,%_Plurals");
		excelCommasStrCell(fpout, "%_Verbs,%_Aux");
		if (langType == ENG)
			excelStrCell(fpout, "%_Mod");
		excelCommasStrCell(fpout, "%_3S,%_13S,%_PAST,%_PASTP,%_PRESP");
		excelCommasStrCell(fpout, "%_prep,%_adj,%_adv,%_conj,%_det,%_pro");
	} else {
		excelCommasStrCell(fpout, "Nouns,Plurals");
		excelCommasStrCell(fpout, "Verbs,Aux");
		if (langType == ENG)
			excelStrCell(fpout, "Mod");
		excelCommasStrCell(fpout, "3S,13S,PAST,PASTP,PRESP");
		excelCommasStrCell(fpout, "prep,adj,adv,conj,det,pro");
	}
	excelCommasStrCell(fpout, "noun_verb,open_closed");
	excelCommasStrCell(fpout, "#open-class,#closed-class");
	excelCommasStrCell(fpout, "retracing,repetition");
	excelRow(fpout, ExcelRowEnd);
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (evald_SpecWords) {
				fprintf(stderr,"\nWARNING: No specified gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			} else
				fprintf(stderr,"\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
// ".xls"
		sFName = strrchr(ts->fname, PATHDELIMCHR);
		if (sFName != NULL)
			sFName = sFName + 1;
		else
			sFName = ts->fname;
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, sFName);
		if (ts->ID) {
			excelOutputID(fpout, ts->ID);
		} else {
			excelCommasStrCell(fpout, ".,.");
			excelStrCell(fpout, ts->sp);
			excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
		}
		if (!ts->isMORFound) {
			fprintf(stderr,"\nWARNING: No %%mor: tier found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, sFName);
			if (langType == ENG)
				excelCommasStrCell(fpout, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
			else
				excelCommasStrCell(fpout, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
			excelRow(fpout, ExcelRowEnd);
			continue;
		}
//		Secs2Str(ts->tm, st, FALSE);
		RoundSecsStr(ts->tm, st);
		excelStrCell(fpout, st);
		excelNumCell(fpout, "%.0f",ts->tUtt);
		excelNumCell(fpout, "%.0f",ts->mluUtt);
		if (ts->mluUtt == 0.0)
			excelCommasStrCell(fpout, "NA,NA");
		else {
			excelNumCell(fpout, "%.3f", ts->mluWords/ts->mluUtt);
			excelNumCell(fpout, "%.3f", ts->morf/ts->mluUtt);
		}
		excelNumCell(fpout, "%.0f", ts->frTypes);
		excelNumCell(fpout, "%.0f", ts->frTokens);
		if (ts->frTokens == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->frTypes/ts->frTokens);
		if (ts->tm == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->frTokens / (ts->tm / 60.0000));
		if (ts->tUtt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->CUR/ts->tUtt);
		if (!isRawVal) {
			if (ts->spWrdCnt == 0.0)
				ts->werr = 0.000;
			else
				ts->werr = (ts->werr/ts->spWrdCnt)*100.0000;
			excelNumCell(fpout, "%.3f", ts->werr);
		} else
			excelNumCell(fpout, "%.0f",ts->werr);
		excelNumCell(fpout, "%.0f", ts->uerr);
//		excelNumCell(fpout, "%.0f" ,ts->morTotal); // comment to remove X column
		if (langType == ENG) {
			if (ts->morTotal == 0.0) {
				ts->density = 0.000;
				excelStrCell(fpout, "NA");
			} else {
				ts->density = ts->density / ts->morTotal;
				excelNumCell(fpout, "%.3f", ts->density);
			}
		}
		if (!isRawVal) {
			if (ts->noun == 0.0)
				ts->pl = 0.000;
			else
				ts->pl = (ts->pl/ts->noun)*100.0000;
			if (ts->verb == 0.0) {
				ts->thrS = 0.000;
				ts->thrnS = 0.000;
				ts->past = 0.000;
				ts->pastp = 0.000;
				ts->presp = 0.000;
			} else {
				ts->thrS = (ts->thrS/ts->verb)*100.0000;
				ts->thrnS = (ts->thrnS/ts->verb)*100.0000;
				ts->past = (ts->past/ts->verb)*100.0000;
				ts->pastp = (ts->pastp/ts->verb)*100.0000;
				ts->presp = (ts->presp/ts->verb)*100.0000;
			}
			if (ts->morTotal == 0.0) {
				ts->noun = 0.000;
				ts->verb = 0.000;
				ts->aux = 0.000;
				ts->mod = 0.000;
				ts->prep = 0.000;
				ts->adj = 0.000;
				ts->adv = 0.000;
				ts->conj = 0.000;
				ts->det = 0.000;
				ts->pron = 0.000;
			} else {
				ts->noun = (ts->noun/ts->morTotal)*100.0000;
				ts->verb = (ts->verb/ts->morTotal)*100.0000;
				ts->aux = (ts->aux/ts->morTotal)*100.0000;
				ts->mod = (ts->mod/ts->morTotal)*100.0000;
				ts->prep = (ts->prep/ts->morTotal)*100.0000;
				ts->adj = (ts->adj/ts->morTotal)*100.0000;
				ts->adv = (ts->adv/ts->morTotal)*100.0000;
				ts->conj = (ts->conj/ts->morTotal)*100.0000;
				ts->det = (ts->det/ts->morTotal)*100.0000;
				ts->pron = (ts->pron/ts->morTotal)*100.0000;
			}
			excelNumCell(fpout, "%.3f", ts->noun);
			excelNumCell(fpout, "%.3f", ts->pl);
			excelNumCell(fpout, "%.3f", ts->verb);
			excelNumCell(fpout, "%.3f", ts->aux);
			if (langType == ENG)
				excelNumCell(fpout, "%.3f", ts->mod);
			excelNumCell(fpout, "%.3f", ts->thrS);
			excelNumCell(fpout, "%.3f", ts->thrnS);
			excelNumCell(fpout, "%.3f", ts->past);
			excelNumCell(fpout, "%.3f", ts->pastp);
			excelNumCell(fpout, "%.3f", ts->presp);
			excelNumCell(fpout, "%.3f", ts->prep);
			excelNumCell(fpout, "%.3f", ts->adj);
			excelNumCell(fpout, "%.3f", ts->adv);
			excelNumCell(fpout, "%.3f", ts->conj);
			excelNumCell(fpout, "%.3f", ts->det);
			excelNumCell(fpout, "%.3f", ts->pron);
		} else {
			excelNumCell(fpout, "%.0f", ts->noun);
			excelNumCell(fpout, "%.0f", ts->pl);
			excelNumCell(fpout, "%.0f", ts->verb);
			excelNumCell(fpout, "%.0f", ts->aux);
			if (langType == ENG)
				excelNumCell(fpout, "%.0f", ts->mod);
			excelNumCell(fpout, "%.0f", ts->thrS);
			excelNumCell(fpout, "%.0f", ts->thrnS);
			excelNumCell(fpout, "%.0f", ts->past);
			excelNumCell(fpout, "%.0f", ts->pastp);
			excelNumCell(fpout, "%.0f", ts->presp);
			excelNumCell(fpout, "%.0f", ts->prep);
			excelNumCell(fpout, "%.0f", ts->adj);
			excelNumCell(fpout, "%.0f", ts->adv);
			excelNumCell(fpout, "%.0f", ts->conj);
			excelNumCell(fpout, "%.0f", ts->det);
			excelNumCell(fpout, "%.0f", ts->pron);
		}
		if (ts->verbsNV == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->nounsNV/ts->verbsNV);
		if (ts->closedClass == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->openClass/ts->closedClass);
		excelNumCell(fpout, "%.0f", ts->openClass);
		excelNumCell(fpout, "%.0f", ts->closedClass);
		excelNumCell(fpout, "%.0f",ts->retr);
		excelNumCell(fpout, "%.0f",ts->repeat);
		excelRow(fpout, ExcelRowEnd);
		if (DBKeyRoot != NULL) {
			num = db->num;
			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout,"+/-SD, , , , , , , , , , , ");

			SDn = 0; // this number should be less than SDRESSIZE = 256
			compute_SD(&SD[SDn++], ts->tm,  NULL, db->tm_sqr, db->tm, num, st);
			compute_SD(&SD[SDn++], ts->tUtt,  NULL, db->tUtt_sqr, db->tUtt, num, NULL);
			compute_SD(&SD[SDn++], ts->mluUtt,  NULL, db->mluUtt_sqr, db->mluUtt, num, NULL);
			compute_SD(&SD[SDn++], ts->mluWords,  &ts->mluUtt, db->mluWords_sqr, db->mluWords, num, NULL);
			compute_SD(&SD[SDn++], ts->morf,  &ts->mluUtt, db->morf_sqr, db->morf, num, NULL);
			compute_SD(&SD[SDn++], ts->frTypes,  NULL, db->frTypes_sqr, db->frTypes, num, NULL);
			compute_SD(&SD[SDn++], ts->frTokens,  NULL, db->frTokens_sqr, db->frTokens, num, NULL);
			compute_SD(&SD[SDn++], ts->frTypes,  &ts->frTokens, db->TTR_sqr, db->TTR, num, NULL);
			tt = ts->tm / 60.0000;
			compute_SD(&SD[SDn++], ts->frTokens,  &tt, db->WOD_sqr, db->WOD, num, NULL);
			compute_SD(&SD[SDn++], ts->CUR,  &ts->tUtt, db->CUR_sqr, db->CUR, num, NULL);
			compute_SD(&SD[SDn++], ts->werr,  NULL, db->werr_sqr, db->werr, num, NULL);
			compute_SD(&SD[SDn++], ts->uerr,  NULL, db->uerr_sqr, db->uerr, num, NULL);
//			compute_SD(&SD[SDn++], ts->morTotal,  NULL, db->morTotal_sqr, db->morTotal, num, NULL); // comment to remove X column
			compute_SD(&SD[SDn++], ts->density,   NULL, db->density_sqr,  db->density,  num, NULL);
			compute_SD(&SD[SDn++], ts->noun,  NULL, db->noun_sqr, db->noun, num, NULL);
			compute_SD(&SD[SDn++], ts->pl,  NULL, db->pl_sqr, db->pl, num, NULL);
			compute_SD(&SD[SDn++], ts->verb,  NULL, db->verb_sqr, db->verb, num, NULL);
			compute_SD(&SD[SDn++], ts->aux,  NULL, db->aux_sqr, db->aux, num, NULL);
			compute_SD(&SD[SDn++], ts->mod,  NULL, db->mod_sqr, db->mod, num, NULL);
			compute_SD(&SD[SDn++], ts->thrS,  NULL, db->thrS_sqr, db->thrS, num, NULL);
			compute_SD(&SD[SDn++], ts->thrnS,  NULL, db->thrnS_sqr, db->thrnS, num, NULL);
			compute_SD(&SD[SDn++], ts->past,  NULL, db->past_sqr, db->past, num, NULL);
			compute_SD(&SD[SDn++], ts->pastp,  NULL, db->pastp_sqr, db->pastp, num, NULL);
			compute_SD(&SD[SDn++], ts->presp,  NULL, db->presp_sqr, db->presp, num, NULL);
			compute_SD(&SD[SDn++], ts->prep,  NULL, db->prep_sqr, db->prep, num, NULL);
			compute_SD(&SD[SDn++], ts->adj,  NULL, db->adj_sqr, db->adj, num, NULL);
			compute_SD(&SD[SDn++], ts->adv,  NULL, db->adv_sqr, db->adv, num, NULL);
			compute_SD(&SD[SDn++], ts->conj,  NULL, db->conj_sqr, db->conj, num, NULL);
			compute_SD(&SD[SDn++], ts->det,  NULL, db->det_sqr, db->det, num, NULL);
			compute_SD(&SD[SDn++], ts->pron,  NULL, db->pron_sqr, db->pron, num, NULL);
			compute_SD(&SD[SDn++], ts->nounsNV,  &ts->verbsNV, db->NoV_sqr, db->NoV, num, NULL);
			compute_SD(&SD[SDn++], ts->openClass,  &ts->closedClass, db->OPoCl_sqr, db->OPoCl, num, NULL);
			compute_SD(&SD[SDn++], ts->openClass,  NULL, db->openClass_sqr, db->openClass, num, NULL);
			compute_SD(&SD[SDn++], ts->closedClass,  NULL, db->closedClass_sqr, db->closedClass, num, NULL);
			compute_SD(&SD[SDn++], ts->retr,  NULL, db->retr_sqr, db->retr, num, NULL);
			compute_SD(&SD[SDn++], ts->repeat,  NULL, db->repeat_sqr, db->repeat, num, NULL);
			excelRow(fpout, ExcelRowEnd);

			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout, " , , , , , , , , , , , ");
			for (i=0; i < SDn; i++) {
				if (SD[i].stars >= 2)
					excelStrCell(fpout, "**");
				else if (SD[i].stars >= 1)
					excelStrCell(fpout, "*");
				else
					excelStrCell(fpout, " ");
			}
			excelRow(fpout, ExcelRowEnd);
		}
	}
	if (DBKeyRoot != NULL) {
		num = db->num;
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"Mean Database, , , , , , , , , , , ");
		Secs2Str(db->tm/num, st, FALSE);
		excelStrCell(fpout, st);
		excelNumCell(fpout, "%.3f", db->tUtt/num);
		excelNumCell(fpout, "%.3f", db->mluUtt/num);
		excelNumCell(fpout, "%.3f", db->mluWords/num);
		excelNumCell(fpout, "%.3f", db->morf/num);
		excelNumCell(fpout, "%.3f", db->frTypes/num);
		excelNumCell(fpout, "%.3f", db->frTokens/num);
		excelNumCell(fpout, "%.3f", db->TTR/num);
		excelNumCell(fpout, "%.3f", db->WOD/num);
		excelNumCell(fpout, "%.3f", db->CUR/num);
		excelNumCell(fpout, "%.3f", db->werr/num);
		excelNumCell(fpout, "%.3f", db->uerr/num);
//		excelNumCell(fpout, "%.3f", db->morTotal/num); // comment to remove X column
		excelNumCell(fpout, "%.3f", db->density/num);
		excelNumCell(fpout, "%.3f", db->noun/num);
		excelNumCell(fpout, "%.3f", db->pl/num);
		excelNumCell(fpout, "%.3f", db->verb/num);
		excelNumCell(fpout, "%.3f", db->aux/num);
		excelNumCell(fpout, "%.3f", db->mod/num);
		excelNumCell(fpout, "%.3f", db->thrS/num);
		excelNumCell(fpout, "%.3f", db->thrnS/num);
		excelNumCell(fpout, "%.3f", db->past/num);
		excelNumCell(fpout, "%.3f", db->pastp/num);
		excelNumCell(fpout, "%.3f", db->presp/num);
		excelNumCell(fpout, "%.3f", db->prep/num);
		excelNumCell(fpout, "%.3f", db->adj/num);
		excelNumCell(fpout, "%.3f", db->adv/num);
		excelNumCell(fpout, "%.3f", db->conj/num);
		excelNumCell(fpout, "%.3f", db->det/num);
		excelNumCell(fpout, "%.3f", db->pron/num);
		excelNumCell(fpout, "%.3f", db->NoV/num);
		excelNumCell(fpout, "%.3f", db->OPoCl/num);
		excelNumCell(fpout, "%.3f", db->openClass/num);
		excelNumCell(fpout, "%.3f", db->closedClass/num);
		excelNumCell(fpout, "%.3f", db->retr/num );
		excelNumCell(fpout, "%.3f", db->repeat/num);
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout, "Min Database, , , , , , , , , , , ");
		Secs2Str(db->mn_tm, st, FALSE);
		excelStrCell(fpout, st);
		excelNumCell(fpout, "%.0f", db->mn_tUtt);
		excelNumCell(fpout, "%.0f", db->mn_mluUtt);
		excelNumCell(fpout, "%.3f", db->mn_mluWords);
		excelNumCell(fpout, "%.3f", db->mn_morf);
		excelNumCell(fpout, "%.0f", db->mn_frTypes);
		excelNumCell(fpout, "%.0f", db->mn_frTokens);
		excelNumCell(fpout, "%.3f", db->mn_TTR);
		excelNumCell(fpout, "%.3f", db->mn_WOD);
		excelNumCell(fpout, "%.3f", db->mn_CUR);
		if (!isRawVal) {
			excelNumCell(fpout, "%.3f", db->mn_werr);
		} else {
			excelNumCell(fpout, "%.0f", db->mn_werr);
		}
		excelNumCell(fpout, "%.0f", db->mn_uerr);
//		excelNumCell(fpout, "%.0f", db->mn_morTotal); // comment to remove X column
		excelNumCell(fpout, "%.3f", db->mn_density);
		if (!isRawVal) {
			excelNumCell(fpout, "%.3f", db->mn_noun);
			excelNumCell(fpout, "%.3f", db->mn_pl);
			excelNumCell(fpout, "%.3f", db->mn_verb);
			excelNumCell(fpout, "%.3f", db->mn_aux);
			excelNumCell(fpout, "%.3f", db->mn_mod);
			excelNumCell(fpout, "%.3f", db->mn_thrS);
			excelNumCell(fpout, "%.3f", db->mn_thrnS);
			excelNumCell(fpout, "%.3f", db->mn_past);
			excelNumCell(fpout, "%.3f", db->mn_pastp);
			excelNumCell(fpout, "%.3f", db->mn_presp);
			excelNumCell(fpout, "%.3f", db->mn_prep);
			excelNumCell(fpout, "%.3f", db->mn_adj);
			excelNumCell(fpout, "%.3f", db->mn_adv);
			excelNumCell(fpout, "%.3f", db->mn_conj);
			excelNumCell(fpout, "%.3f", db->mn_det);
			excelNumCell(fpout, "%.3f", db->mn_pron);
		} else {
			excelNumCell(fpout, "%.0f", db->mn_noun);
			excelNumCell(fpout, "%.0f", db->mn_pl);
			excelNumCell(fpout, "%.0f", db->mn_verb);
			excelNumCell(fpout, "%.0f", db->mn_aux);
			excelNumCell(fpout, "%.0f", db->mn_mod);
			excelNumCell(fpout, "%.0f", db->mn_thrS);
			excelNumCell(fpout, "%.0f", db->mn_thrnS);
			excelNumCell(fpout, "%.0f", db->mn_past);
			excelNumCell(fpout, "%.0f", db->mn_pastp);
			excelNumCell(fpout, "%.0f", db->mn_presp);
			excelNumCell(fpout, "%.0f", db->mn_prep);
			excelNumCell(fpout, "%.0f", db->mn_adj);
			excelNumCell(fpout, "%.0f", db->mn_adv);
			excelNumCell(fpout, "%.0f", db->mn_conj);
			excelNumCell(fpout, "%.0f", db->mn_det);
			excelNumCell(fpout, "%.0f", db->mn_pron);
		}
		excelNumCell(fpout, "%.3f", db->mn_NoV);
		excelNumCell(fpout, "%.3f", db->mn_OPoCl);
		excelNumCell(fpout, "%.0f", db->mn_openClass);
		excelNumCell(fpout, "%.0f", db->mn_closedClass);
		excelNumCell(fpout, "%.0f", db->mn_retr);
		excelNumCell(fpout, "%.0f", db->mn_repeat);
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout, "Max Database, , , , , , , , , , , ");
		Secs2Str(db->mx_tm, st, FALSE);
		excelStrCell(fpout, st);
		excelNumCell(fpout, "%.0f", db->mx_tUtt);
		excelNumCell(fpout, "%.0f", db->mx_mluUtt);
		excelNumCell(fpout, "%.3f", db->mx_mluWords);
		excelNumCell(fpout, "%.3f", db->mx_morf);
		excelNumCell(fpout, "%.0f", db->mx_frTypes);
		excelNumCell(fpout, "%.0f", db->mx_frTokens);
		excelNumCell(fpout, "%.3f", db->mx_TTR);
		excelNumCell(fpout, "%.3f", db->mx_WOD);
		excelNumCell(fpout, "%.3f", db->mx_CUR);
		if (!isRawVal) {
			excelNumCell(fpout, "%.3f", db->mx_werr);
		} else {
			excelNumCell(fpout, "%.0f", db->mx_werr);
		}
		excelNumCell(fpout, "%.0f", db->mx_uerr);
//		excelNumCell(fpout, "%.0f", db->mx_morTotal); // comment to remove X column
		excelNumCell(fpout, "%.3f", db->mx_density);
		if (!isRawVal) {
			excelNumCell(fpout, "%.3f", db->mx_noun);
			excelNumCell(fpout, "%.3f", db->mx_pl);
			excelNumCell(fpout, "%.3f", db->mx_verb);
			excelNumCell(fpout, "%.3f", db->mx_aux);
			excelNumCell(fpout, "%.3f", db->mx_mod);
			excelNumCell(fpout, "%.3f", db->mx_thrS);
			excelNumCell(fpout, "%.3f", db->mx_thrnS);
			excelNumCell(fpout, "%.3f", db->mx_past);
			excelNumCell(fpout, "%.3f", db->mx_pastp);
			excelNumCell(fpout, "%.3f", db->mx_presp);
			excelNumCell(fpout, "%.3f", db->mx_prep);
			excelNumCell(fpout, "%.3f", db->mx_adj);
			excelNumCell(fpout, "%.3f", db->mx_adv);
			excelNumCell(fpout, "%.3f", db->mx_conj);
			excelNumCell(fpout, "%.3f", db->mx_det);
			excelNumCell(fpout, "%.3f", db->mx_pron);
		} else {
			excelNumCell(fpout, "%.0f", db->mx_noun);
			excelNumCell(fpout, "%.0f", db->mx_pl);
			excelNumCell(fpout, "%.0f", db->mx_verb);
			excelNumCell(fpout, "%.0f", db->mx_aux);
			excelNumCell(fpout, "%.0f", db->mx_mod);
			excelNumCell(fpout, "%.0f", db->mx_thrS);
			excelNumCell(fpout, "%.0f", db->mx_thrnS);
			excelNumCell(fpout, "%.0f", db->mx_past);
			excelNumCell(fpout, "%.0f", db->mx_pastp);
			excelNumCell(fpout, "%.0f", db->mx_presp);
			excelNumCell(fpout, "%.0f", db->mx_prep);
			excelNumCell(fpout, "%.0f", db->mx_adj);
			excelNumCell(fpout, "%.0f", db->mx_adv);
			excelNumCell(fpout, "%.0f", db->mx_conj);
			excelNumCell(fpout, "%.0f", db->mx_det);
			excelNumCell(fpout, "%.0f", db->mx_pron);
		}
		excelNumCell(fpout, "%.3f", db->mx_NoV);
		excelNumCell(fpout, "%.3f", db->mx_OPoCl);
		excelNumCell(fpout, "%.0f", db->mx_openClass);
		excelNumCell(fpout, "%.0f", db->mx_closedClass);
		excelNumCell(fpout, "%.0f", db->mx_retr);
		excelNumCell(fpout, "%.0f", db->mx_repeat);
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"SD Database, , , , , , , , , , , ");
		if (SD[0].isSDComputed == 1) {
			Secs2Str(SD[0].dbSD, st, TRUE);
			excelStrCell(fpout, st);
//			fprintf(fpout, "%.3f", SD[0].dbSD);
		} else {
			excelStrCell(fpout, "NA");
		}
		for (i=1; i < SDn; i++) {
			if (SD[i].isSDComputed == 1) {
				excelNumCell(fpout, "%.3f", SD[i].dbSD);
			} else {
				excelStrCell(fpout, "NA");
			}
		}
		excelRow(fpout, ExcelRowEnd);
	}
	excelRow(fpout, ExcelRowEmpty);
	if (sp_head != NULL) {
		if (DBKeyRoot != NULL) {
			excelRow(fpout, ExcelRowEmpty);
			excelRowOneStrCell(fpout, ExcelBlkCell, "+/- SD  * = 1 SD, ** = 2 SD");
		}
		if (DBGemNum) {
			strcpy(templineC4, "Database Gems:");
			for (i=0; i < DBGemNum; i++) {
				strcat(templineC4, DBGems[i]);
				strcat(templineC4, ", ");
			}
			uS.remblanks(templineC4);
			i = strlen(templineC4) - 1;
			if (templineC4[i] == ',')
				templineC4[i] = EOS;
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
		}
		if (DBKeyRoot != NULL) {
			strcpy(templineC4, "Database keywords:");
			i = strlen(templineC4);
			for (DBKey=DBKeyRoot; DBKey != NULL; DBKey=DBKey->next) {
				if (DBKey->key1 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->key1);
				}
				if (DBKey->key2 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->key2);
				}
				if (DBKey->key3 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->key3);
				}
				if (DBKey->key4 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->key4);
				}
				if (DBKey->age != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->age);
				}
				if (DBKey->sex != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, ",");
					strcat(templineC4, DBKey->sex);
				}
				strcat(templineC4, ", ");
			}
			uS.remblanks(templineC4);
			i = strlen(templineC4) - 1;
			if (templineC4[i] == ',')
				templineC4[i] = EOS;
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
			strcpy(templineC4, "#_files_in_database:");
			i = strlen(templineC4);
			sprintf(templineC4+i, "%.0f", db->num);
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
			if (specialOptionUsed) {
				excelRowOneStrCell(fpout, ExcelRedCell, "CAUTION:  Analyses that use the +b, +g, +n, +r or +s options should not be directly compared to the database because those options cannot be selected for the database in EVAL-D");
			}
		}
//		printArg(targv, targc, fpout, FALSE, "");
	}
	excelFooter(fpout);
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

void call() {
	int  i, found;
	long tlineno = 0;
	char lRightspeaker;
	char isOutputGem;
	char isPRCreateDBRes;
	char isPWordsListFound;
	char word[BUFSIZ], *s;
	struct evald_speakers *ts;
	FILE *PWordsListFP;
	FNType debugfile[FNSize];

	if (isCreateDB) {
		fprintf(stdout, "%s\n", oldfname+wdOffset);
//		return;
	} else {
		fprintf(stderr,"From file <%s>\n",oldfname);
	}
	PWordsListFP = NULL;
	isPWordsListFound = FALSE;
#if !defined(CLAN_SRV)
	if (isPWordsList) {
		parsfname(oldfname, debugfile, ".word_list.cex");
		PWordsListFP = fopen(debugfile, "w");
		if (PWordsListFP == NULL) {
			fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugfile);
		}
  #ifdef _MAC_CODE
		else
			settyp(debugfile, 'TEXT', the_file_creator.out, FALSE);
  #endif
		if (PWordsListFP != NULL) {
  #ifdef _MAC_CODE
			fprintf(PWordsListFP, "%s	CAfont:13:7\n", FONTHEADER);
  #endif
  #ifdef _WIN32
			fprintf(PWordsListFP, "%s	Win95:CAfont:-15:0\n", FONTHEADER);
  #endif
			fprintf(PWordsListFP,"From file <%s>\n",oldfname);
		}
	}
#endif // !defined(CLAN_SRV)
	tmDur = -1.0;
	ts = NULL;
	if (isGOptionSet == FALSE)
		onlyApplyToDB = !isFoundGems();
	if (!onlyApplyToDB && isNOptionSet == FALSE) {
		strcpy(evald_BBS, "@*&#");
		strcpy(evald_CBS, "@*&#");
	}
	if (isCreateDB) {
		if (dbfpout == NULL) {
			strcpy(FileName1, wd_dir);
			i = strlen(FileName1) - 1;
			if (FileName1[i] == PATHDELIMCHR)
				FileName1[i] = EOS;
			s = strrchr(FileName1, PATHDELIMCHR);
			if (s != NULL) {
				*s = EOS;
			}
			addFilename2Path(FileName1, lang_prefix);
			strcat(FileName1, DATABASE_FILE_NAME);
			dbfpout = fopen(FileName1, "wb");
			if (dbfpout == NULL) {
				fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
				if (PWordsListFP != NULL)
					fclose(PWordsListFP);
				evald_error(NULL, FALSE);
			} else
				fprintf(dbfpout,"V%d\n", EVAL_DB_VERSION);
		}
		isOutputGem = TRUE;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (utterance->speaker[0] == '*')
				break;
			if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (strncmp(utterance->line, "EVAL-D DATABASE EXCLUDE", 21) == 0) {
					fprintf(stderr,"    EXCLUDED FILE: %s\n",oldfname+wdOffset);
					return;
				}
			}
		}
		rewind(fpin);
		if (isPWordsList)
			fprintf(fpout, "From file <%s>\n", oldfname);
	} else if (evald_SpecWords && !onlyApplyToDB) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	langType = ENG;
	isPRCreateDBRes = FALSE;
	spareTier1[0] = EOS;
	lRightspeaker = FALSE;
	found = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
#if !defined(CLAN_SRV)
//			fprintf(stderr,"\r%ld ",lineno);
#endif
		}
		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE)) {
				uS.remFrontAndBackBlanks(utterance->line);
				if (strchr(utterance->line, ',') != NULL || strchr(utterance->line, ' ') != NULL) {
					langType = UNK;
				} else {
					for (i=0; isSpace(utterance->line[i]); i++) ;
					if (uS.mStricmp(utterance->line+i, "eng") == 0) {
						langType = ENG;
					} else {
						langType = UNK;
					}
				}
			}
		}
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
				if (isPostCodeOnUtt(utterance->line, "[+ exc]"))
					lRightspeaker = FALSE;
				else 
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!onlyApplyToDB && evald_SpecWords && !strcmp(evald_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				evald_n_option = FALSE;
				strcpy(evald_BBS, "@BG:");
				strcpy(evald_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				evald_n_option = TRUE;
				strcpy(evald_BBS, "@G:");
				strcpy(evald_CBS, "@*&#");
			}
		}
		if (isCreateDB) {
			if (utterance->speaker[0] == '@') {
				uS.remFrontAndBackBlanks(utterance->line);
				removeExtraSpace(utterance->line);
			}
			if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				if (ts != NULL) {
					if (isPRCreateDBRes) {
						prTSResults(ts);
						isPRCreateDBRes = FALSE;
					}
//					if (ts->words_root != NULL)
//						ts->words_root = evald_freetree(ts->words_root);
					evald_initTSVars(ts, FALSE);
				}
				fprintf(dbfpout, "G%s\n", utterance->line);
			} else if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				fprintf(dbfpout, "B%s\n", utterance->line);
			} else if (uS.partcmp(utterance->speaker,"@EG:",FALSE,FALSE)) {
				if (ts != NULL) {
					if (isPRCreateDBRes) {
						prTSResults(ts);
						isPRCreateDBRes = FALSE;
					}
//					if (ts->words_root != NULL)
//						ts->words_root = evald_freetree(ts->words_root);
					evald_initTSVars(ts, FALSE);
				}
				fprintf(dbfpout, "E%s\n", utterance->line);
			} else if (uS.partcmp(utterance->speaker,"@End",FALSE,FALSE)) {
				if (ts != NULL) {
					if (isPRCreateDBRes) {
						prTSResults(ts);
						isPRCreateDBRes = FALSE;
					}
					if (ts->words_root != NULL)
						ts->words_root = evald_freetree(ts->words_root);
				}
				fprintf(dbfpout, "-\n");
			}
/*
			dbPosC = ftell(dbfpout);
			if (fseek(dbfpout, dbPos, SEEK_SET) != 0) {
				fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
				cp2utf_exit(0);
			}
			fprintf(dbfpout, "                    \n", utterance->speaker, utterance->line);
			if (fseek(dbfpout, dbPosC, SEEK_SET) != 0) {
				fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
				cp2utf_exit(0);
			}
*/
		}
		if (!onlyApplyToDB && uS.partcmp(utterance->speaker,evald_BBS,FALSE,FALSE)) {
			if (evald_n_option) {
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
		} else if (found > 0 && uS.partcmp(utterance->speaker,evald_CBS,FALSE,FALSE)) {
			if (evald_n_option) {
			} else {
				if (isRightText(word)) {
					found--;
					if (found == 0) {
						if (evald_SpecWords)
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
				isPRCreateDBRes = TRUE;
				strcpy(templineC, utterance->speaker);
				ts = evald_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
				isPWordsListFound = TRUE;
			}
			evald_process_tier(ts, NULL, NULL, isOutputGem, PWordsListFP);
		}
	}
	if (PWordsListFP != NULL) {
		fclose(PWordsListFP);
		if (!isPWordsListFound)
			unlink(debugfile);
	}
#if !defined(CLAN_SRV)
//	fprintf(stderr, "\r	  \r");
#endif
	if (!combinput)
		evald_pr_result();
}

void init(char first) {
	int  i;
	char *f;
	FNType debugfile[FNSize];

	if (first) {
//		lang_prefix = NULL;
		lang_prefix = "eng";
		isRawVal = FALSE;
		specialOptionUsed = FALSE;
		ftime = TRUE;
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
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
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
//		addword('\0','\0',"+*|xxx");
//		addword('\0','\0',"+*|yyy");
//		addword('\0','\0',"+*|www");
		mor_initwords();
		dbfpout = NULL;
		isPWordsList = FALSE;
		isDBFilesList = FALSE;
		DBFilesListFP = NULL;
		isExcludeWords = FALSE;
		GemMode = '\0';
		evald_SpecWords = 0;
		evald_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		evald_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(evald_BBS, "@*&#");
		strcpy(evald_CBS, "@*&#");
		isSpeakerNameGiven = FALSE;
		sp_head = NULL;
		db = NULL;
		DBKeyRoot = NULL;
		DBGemNum = 0;
		if (isCreateDB) {
			combinput = TRUE;
			stout = TRUE;
			isRecursive = TRUE;
		}
	} else {
		if (ftime) {
			ftime = FALSE;
			if (lang_prefix == NULL) {
				fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"eval +leng\" or \"eval +leng.cut\".\n");
				cutt_exit(0);
			}
			if (isExcludeWords && DBKeyRoot != NULL) {
				fprintf(stderr, "ERROR:  Analyses that use the +s option cannot be compared to the database, \n");
				fprintf(stderr, "        because +s option cannot be applied to the database in EVAL-D\n");
				cutt_exit(0);
			}
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
				evald_error(NULL, FALSE);
			}
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
				fprintf(stderr,"Please specify at least one speaker tier code with Option button in Commands window.\n");
				fprintf(stderr,"Or with \"+t\" option on command line.\n");
#else
				fprintf(stderr,"Please specify at least one speaker tier code with \"+t\" option on command line.\n");
#endif
				evald_error(NULL, FALSE);
			}
			maketierchoice("*:", '+', '\001');
			if (isDBFilesList && DBKeyRoot == NULL) {
				fprintf(stderr,"+e1 option can only be used with +d option.\n");
				cutt_exit(0);
			}
			if (isCreateDB) {
			} else if (DBKeyRoot != NULL) {
				if (isDBFilesList && DBFilesListFP == NULL) {
					strcpy(debugfile, DATABSEFILESLIST);
					DBFilesListFP = fopen(debugfile, "w");
					if (DBFilesListFP == NULL) {
						fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugfile);
					}
#ifdef _MAC_CODE
					else
						settyp(debugfile, 'TEXT', the_file_creator.out, FALSE);
#endif
				}
				db = NEW(struct database);
				if (db == NULL)
					evald_error(db, TRUE);
				init_db(db);
				ParseDatabase(db);
				CleanUpTempIDSpeakers();
			}
			for (i=1; i < targc; i++) {
				if (*targv[i] == '+' || *targv[i] == '-') {
					if (targv[i][1] == 't') {
						maingetflag(targv[i], NULL, &i);
					}
				}
			}
			R8 = TRUE;
		}
	}
	if (!combinput || first) {
		sp_head = NULL;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i, fileArgcStart, fileArgcEnd, langArgc;
	char filesPath[3][128], langPref[32];
	char isFileGiven, isLangGiven;

	db = NULL;
	targc = argc;
#if defined(_MAC_CODE) || defined(_WIN32)
	targv = argv;
#endif
#ifdef UNX
	for (i=0; i < argc; i++) {
		targv[i] = argv[i];
	}
	argv = targv;
#endif
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = EVALD;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	DBFilesListFP = NULL;
	isCreateDB = 0;
	isFileGiven = FALSE;
	isLangGiven = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'c') {
				isCreateDB = 1;
			} else if (argv[i][1] == 'l') {
				isLangGiven = TRUE;
			}
		} else
			isFileGiven = TRUE;
	}
	fileArgcStart = 0;
	fileArgcEnd = 0;
	langArgc = 0;
	wdOffset = strlen(wd_dir);
	if (wd_dir[wdOffset-1] != PATHDELIMCHR)
		wdOffset++;
	if (isCreateDB) {
		if (!isLangGiven) {
			langArgc = argc;
			strcpy(langPref, "+leng");
			argv[langArgc] = langPref;
			argc++;
		}
		if (!isFileGiven) {
			fileArgcStart = argc;
			strcpy(filesPath[0], wd_dir);
			addFilename2Path(filesPath[0], "English/Protocol/Delaware-protect/Delaware/*.cha");
			argv[argc] = filesPath[0];
			argc++;
			strcpy(filesPath[1], wd_dir);
			addFilename2Path(filesPath[1], "English/Pitt/Control/cookie/*.cha");
			argv[argc] = filesPath[1];
			argc++;
			strcpy(filesPath[2], wd_dir);
			addFilename2Path(filesPath[2], "English/Pitt/Dementia/cookie/*.cha");
			argv[argc] = filesPath[2];
			argc++;
			fileArgcEnd = argc;
		}
	}
	bmain(argc,argv,evald_pr_result);

	if (DBFilesListFP != NULL)
		fclose(DBFilesListFP);
	db = freedb(db);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	if (dbfpout != NULL) {
		fprintf(stderr,"Output file <%s>\n",FileName1);
		fclose(dbfpout);
	}
	if (isCreateDB) {
		if (langArgc != 0) {
			argv[langArgc] = NULL;
		}
		for (i=fileArgcStart; i < fileArgcEnd; i++) {
			argv[i] = NULL;
			argc--;
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'b':
			specialOptionUsed = TRUE;
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
		case 'c':
			isCreateDB = 1;
			no_arg_option(f);
			break;
		case 'd':
			if (*f == EOS) {
				fprintf(stderr,"Missing argument for option: %s\n", f-2);
				cutt_exit(0);
			} else {
				addDBKeys(f);
			}
			break;
		case 'e':
			if (*f == '1')
				isDBFilesList = TRUE;
			else if (*f == '2')
				isPWordsList = TRUE;
			else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'g':
			if (*f == EOS) {
				if (*(f-2) == '+') {
					evald_group = TRUE;
					specialOptionUsed = TRUE;
				} else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				evald_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'l':
			lang_prefix = f;
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
				evald_n_option = TRUE;
				strcpy(evald_BBS, "@G:");
				strcpy(evald_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(evald_BBS, "@BG:");
				strcpy(evald_CBS, "@EG:");
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
		case 'r':
			if (*f != 'e')
				specialOptionUsed = TRUE;
			maingetflag(f-2,f1,i);
			break;
		case 's':
			specialOptionUsed = TRUE;
			if (*f == '[') {
				if (*(f+1) == '-') {
					if (*(f-2) == '+')
						isLanguageExplicit = 2;
					maingetflag(f-2,f1,i);
				} else {
					fprintf(stderr, "Please specify only pre-codes \"[- ...]\" with +/-s option.\n");
					fprintf(stderr, "Utterances with \"[+ exc]\" post-code are exclude automatically.\n");
					cutt_exit(0);
				}
			} else if (*(f-2) == '-') {
				if (*f == 'm' && isMorSearchOption(f, *f, *(f-2))) {
					isExcludeWords = TRUE;
					maingetflag(f-2,f1,i);
				} else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					evald_error(NULL, FALSE);
				}
			}
			break;
		case 't':
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
