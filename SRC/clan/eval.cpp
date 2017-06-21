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

#define CHAT_MODE 1
#include "cu.h"
#include <math.h>
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main eval_main
#define call eval_call
#define getflag eval_getflag
#define init eval_init
#define usage eval_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "eval_db.txt"

extern char OverWriteFile;
extern char outputOnlyData;
extern char isRecursive;
extern struct tier *defheadtier;
extern struct IDtype *IDField;

#define SDRESSIZE 256

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct eval_tnode {
	char *word;
	struct eval_tnode *left;
	struct eval_tnode *right;
};

struct speaker {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;
	float tm, tms;
	float tUtt;
	float total;	// total number of words
	float diff;		// number of diff words
	float clause;
	float words, morf, mUtt, xxxut; //, morfsqr;
	float exla, intr, peri, ques, tbrk;
	float werr, uerr;
	float noun, verb, prep, adv, conj, pron, det, thrS, thrnS, past, pastp, pl, presp;
	float retr, repet;
	float morTotal;	// total number of words on %mor tier
	struct eval_tnode *words_root;
	struct speaker *next_sp;
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

struct database {
	float num;
	float tm_sqr, tm, mn_tm, mx_tm;
	float tUtt_sqr, tUtt, mn_tUtt, mx_tUtt;
	float total_sqr, total, mn_total, mx_total;	/* total number of words */
	float diff_sqr, diff, mn_diff, mx_diff;	/* number of diff words */
	float TTR_sqr, TTR, mn_TTR, mx_TTR;
	float CUR_sqr, CUR, mn_CUR, mx_CUR;
	float mUtt_sqr, mUtt, mn_mUtt, mx_mUtt;
	float words_sqr, words, mn_words, mx_words;
	float morf_sqr, morf, mn_morf, mx_morf;
	float xxxut_sqr, xxxut, mn_xxxut, mx_xxxut;
	float exla_sqr, exla, mn_exla, mx_exla;
	float intr_sqr, intr, mn_intr, mx_intr;
	float peri_sqr, peri, mn_peri, mx_peri;
	float ques_sqr, ques, mn_ques, mx_ques;
	float tbrk_sqr, tbrk, mn_tbrk, mx_tbrk;
	float werr_sqr, werr, mn_werr, mx_werr;
	float uerr_sqr, uerr, mn_uerr, mx_uerr;
	float noun_sqr, noun, mn_noun, mx_noun;
	float verb_sqr, verb, mn_verb, mx_verb;
	float prep_sqr, prep, mn_prep, mx_prep;
	float adv_sqr, adv, mn_adv, mx_adv;
	float conj_sqr, conj, mn_conj, mx_conj;
	float pron_sqr, pron, mn_pron, mx_pron;
	float retr_sqr, retr, mn_retr, mx_retr;
	float repet_sqr, repet, mn_repet, mx_repet;
	float det_sqr, det, mn_det, mx_det;
	float thrS_sqr, thrS, mn_thrS, mx_thrS;
	float thrnS_sqr, thrnS, mn_thrnS, mx_thrnS;
	float past_sqr, past, mn_past, mx_past;
	float pastp_sqr, pastp, mn_pastp, mx_pastp;
	float presp_sqr, presp, mn_presp, mx_presp;
	float pl_sqr, pl, mn_pl, mx_pl;
	float morTotal_sqr, morTotal, mn_morTotal, mx_morTotal;	/* total number of words on %mor tier */
	struct speaker *db_sp;
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
static int  eval_SpecWords, DBGemNum;
static char StarDotCHA[256];
static char *DBGems[NUMGEMITEMS];
static char isSpeakerNameGiven, isXXXFound, ftime, isDebug, isCreateDB, isRawVal;
static char eval_BBS[5], eval_CBS[5], eval_group, eval_n_option, isNOptionSet, onlyApplyToDB, isGOptionSet, GemMode;
static float tmDur;
static FILE	*dbfpout;
static struct DBKeys *DBKeyRoot;
static struct speaker *sp_head;
static struct database *db;

void usage() {
	printf("Usage: eval [bS c dS g gS n s %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(UNX) || defined(_MAC_CODE)
	puts("+cN: create database file (N == 1: eval +c1 +t\"*par\")");
	puts("+d : debug.");
#endif
	puts("+dS: specify database keyword(s) S. Choices are:");
	puts("    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control,");
	puts("    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),");
	puts("    Nonfluent = (Global, Broca, Transmotor),");
	puts("    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)");
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("-g : look for gems in Database only");
	puts("+gS: select gems which are labeled by label S");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+o4: output raw values instead of percentage values");
	puts("+s : counts utterances and other words found along with \"xxx\"");
	mainusage(FALSE);
	puts("\nExample:");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70");
	puts("       eval adler15a.cha +t*par +gCinderella +u +d\"control|60-70\"");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       eval adler15a.cha +t*par +gCinderella +u +d\"control|60-70;6\"");
	puts("   Search database for \"Cinderella\" gems of \"Nonfluent\" subjects of any age");
	puts("       eval adler15a.cha +t*par +gCinderella +u +dNonfluent");
	puts("   Just compute results for \"Cinderella\" gems of adler15a.cha. Do not compare to database");
	puts("       eval adler15a.cha +t*par +gCinderella +u");
#ifdef UNX
	puts("\n   Create database");
	puts("       eval -c '+t*PAR' '*.cha'");
#endif
	cutt_exit(0);
}

static void init_db(struct database *p) {
	p->num = 0.0;
	p->tm_sqr = (float)0.0; p->tm = (float)0.0; p->mn_tm = (float)0.0; p->mx_tm = (float)0.0;
	p->tUtt_sqr = (float)0.0; p->tUtt = (float)0.0; p->mn_tUtt = (float)0.0; p->mx_tUtt = (float)0.0;
	p->total_sqr = (float)0.0; p->total = (float)0.0; p->mn_total = (float)0.0; p->mx_total = (float)0.0;
	p->diff_sqr = (float)0.0; p->diff = (float)0.0; p->mn_diff = (float)0.0; p->mx_diff = (float)0.0;
	p->TTR_sqr = (float)0.0; p->TTR = (float)0.0; p->mn_TTR = (float)0.0; p->mx_TTR = (float)0.0;
	p->CUR_sqr = (float)0.0; p->CUR = (float)0.0; p->mn_CUR = (float)0.0; p->mx_CUR = (float)0.0;
	p->mUtt_sqr = (float)0.0; p->mUtt = (float)0.0; p->mn_mUtt = (float)0.0; p->mx_mUtt = (float)0.0;
	p->words_sqr = (float)0.0; p->words = (float)0.0; p->mn_words = (float)0.0; p->mx_words = (float)0.0;
	p->morf_sqr = (float)0.0; p->morf = (float)0.0; p->mn_morf = (float)0.0; p->mx_morf = (float)0.0;
	p->xxxut_sqr = (float)0.0; p->xxxut = (float)0.0; p->mn_xxxut = (float)0.0; p->mx_xxxut = (float)0.0;
	p->exla_sqr = (float)0.0; p->exla = (float)0.0; p->mn_exla = (float)0.0; p->mx_exla = (float)0.0;
	p->intr_sqr = (float)0.0; p->intr = (float)0.0; p->mn_intr = (float)0.0; p->mx_intr = (float)0.0;
	p->peri_sqr = (float)0.0; p->peri = (float)0.0; p->mn_peri = (float)0.0; p->mx_peri = (float)0.0;
	p->ques_sqr = (float)0.0; p->ques = (float)0.0; p->mn_ques = (float)0.0; p->mx_ques = (float)0.0;
	p->tbrk_sqr = (float)0.0; p->tbrk = (float)0.0; p->mn_tbrk = (float)0.0; p->mx_tbrk = (float)0.0;
	p->werr_sqr = (float)0.0; p->werr = (float)0.0; p->mn_werr = (float)0.0; p->mx_werr = (float)0.0;
	p->uerr_sqr = (float)0.0; p->uerr = (float)0.0; p->mn_uerr = (float)0.0; p->mx_uerr = (float)0.0;
	p->noun_sqr = (float)0.0; p->noun = (float)0.0; p->mn_noun = (float)0.0; p->mx_noun = (float)0.0;
	p->verb_sqr = (float)0.0; p->verb = (float)0.0; p->mn_verb = (float)0.0; p->mx_verb = (float)0.0;
	p->prep_sqr = (float)0.0; p->prep = (float)0.0; p->mn_prep = (float)0.0; p->mx_prep = (float)0.0;
	p->adv_sqr = (float)0.0; p->adv = (float)0.0; p->mn_adv = (float)0.0; p->mx_adv = (float)0.0;
	p->conj_sqr = (float)0.0; p->conj = (float)0.0; p->mn_conj = (float)0.0; p->mx_conj = (float)0.0;
	p->pron_sqr = (float)0.0; p->pron = (float)0.0; p->mn_pron = (float)0.0; p->mx_pron = (float)0.0;
	p->retr_sqr = (float)0.0; p->retr = (float)0.0; p->mn_retr = (float)0.0; p->mx_retr = (float)0.0;
	p->repet_sqr = (float)0.0; p->repet = (float)0.0; p->mn_repet = (float)0.0; p->mx_repet = (float)0.0;
	p->det_sqr = (float)0.0; p->det = (float)0.0; p->mn_det = (float)0.0; p->mx_det = (float)0.0;
	p->thrS_sqr = (float)0.0; p->thrS = (float)0.0; p->mn_thrS = (float)0.0; p->mx_thrS = (float)0.0;
	p->thrnS_sqr = (float)0.0; p->thrnS = (float)0.0; p->mn_thrnS = (float)0.0; p->mx_thrnS = (float)0.0;
	p->past_sqr = (float)0.0; p->past = (float)0.0; p->mn_past = (float)0.0; p->mx_past = (float)0.0;
	p->pastp_sqr = (float)0.0; p->pastp = (float)0.0; p->mn_pastp = (float)0.0; p->mx_pastp = (float)0.0;
	p->presp_sqr = (float)0.0; p->presp = (float)0.0; p->mn_presp = (float)0.0; p->mx_presp = (float)0.0;
	p->pl_sqr = (float)0.0; p->pl = (float)0.0; p->mn_pl = (float)0.0; p->mx_pl = (float)0.0;
	p->morTotal_sqr = 0.0; p->morTotal = 0.0; p->mn_morTotal = 0.0; p->mx_morTotal = 0.0;
	p->db_sp = NULL;
}

static void eval_freetree(struct eval_tnode *p) {
	if (p != NULL) {
		eval_freetree(p->left);
		eval_freetree(p->right);
		if (p->word != NULL)
			free(p->word);
		free(p);
	}
}

static struct speaker *freespeakers(struct speaker *p) {
	struct speaker *ts;

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
			eval_freetree(ts->words_root);
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

static void eval_overflow(struct database *db, char IsOutOfMem) {
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
		eval_overflow(NULL, FALSE);
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

static char *eval_strsave(const char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		eval_overflow(NULL, FALSE);
	return(p);
}

static struct DBKeys *initDBKey(const char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		eval_overflow(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = eval_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = eval_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = eval_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = eval_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = eval_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = eval_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = eval_strsave("Wernicke");
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
		DBKey->key1 = eval_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = eval_strsave("Wernicke");
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
					DBKey->key1 = eval_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = eval_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = eval_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = eval_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = eval_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					eval_overflow(NULL, FALSE);
				}
				DBKey->age = eval_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control, \n");
				fprintf(stderr, "    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),\n");
				fprintf(stderr, "    Nonfluent = (Global, Broca, Transmotor),\n");
				fprintf(stderr, "    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)\n");
				eval_overflow(NULL, FALSE);
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

static struct speaker *eval_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	struct speaker *ts, *tsp;

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
			eval_overflow(db, TRUE);
		}
	}
	if ((ts=NEW(struct speaker)) == NULL)
		eval_overflow(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		eval_overflow(db, TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		eval_overflow(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL || isCreateDB)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			eval_overflow(db, TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;
	ts->tm		= (float)0.0;
	ts->tms		= (float)-1.0;
	ts->tUtt	= (float)0.0;
	ts->total   = (float)0.0;
	ts->diff    = (float)0.0;
	ts->clause	= (float)0.0;
	ts->words   = (float)0.0;
	ts->morf	= (float)0.0;
	ts->mUtt	= (float)0.0;
	ts->xxxut   = (float)0.0;
//	ts->morfsqr = (float)0.0;
	ts->exla    = (float)0.0;
	ts->intr    = (float)0.0;
	ts->peri    = (float)0.0;
	ts->ques    = (float)0.0;
	ts->tbrk    = (float)0.0;
	ts->werr    = (float)0.0;
	ts->uerr    = (float)0.0;
	ts->noun    = (float)0.0;
	ts->verb    = (float)0.0;
	ts->prep    = (float)0.0;
	ts->adv     = (float)0.0;
	ts->conj    = (float)0.0;
	ts->pron    = (float)0.0;
	ts->retr    = (float)0.0;
	ts->repet   = (float)0.0;
	ts->det     = (float)0.0;
	ts->thrS    = (float)0.0;
	ts->thrnS   = (float)0.0;
	ts->past    = (float)0.0;
	ts->pastp   = (float)0.0;
	ts->pl      = (float)0.0;
	ts->presp   = (float)0.0;
	ts->morTotal   = 0.0;
	ts->words_root = NULL;
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

static void filterAndOuputID(char *IDs) {
	char *s, *sb;
	
	for (s=IDs; isSpace(*s); s++) ;
	sb = s;
	if ((s=strchr(sb, '|')) != NULL) {
		strcpy(sb, s);
		sb++;
		if ((s=strchr(sb, '|')) != NULL) {
			strcpy(sb, s);
			sb++;
			if ((s=strchr(sb, '|')) != NULL) {
				strcpy(sb, s);
				sb++;
				if ((s=strchr(sb, '|')) != NULL) {
					sb = s + 1;
					if ((s=strchr(sb, '|')) != NULL) {
						sb = s + 1;
						if ((s=strchr(sb, '|')) != NULL) {
							*s = EOS;
							if (!isRightKeyword(sb)) {
								*s = '|';
								strcpy(sb, s);
								sb++;
							} else {
								*s = '|';
								sb = s + 1;
							}
							if ((s=strchr(sb, '|')) != NULL) {
								*s = EOS;
								if (!isRightKeyword(sb)) {
									*s = '|';
									strcpy(sb, s);
									sb++;
								} else {
									*s = '|';
									sb = s + 1;
								}
								if ((s=strchr(sb, '|')) != NULL) {
									strcpy(sb, s);
									sb++;
									if ((s=strchr(sb, '|')) != NULL) {
										*s = EOS;
										if (!isRightKeyword(sb)) {
											*s = '|';
											strcpy(sb, s);
											sb++;
										} else {
											*s = '|';
											sb = s + 1;
										}
										if ((s=strchr(sb, '|')) != NULL) {
											*s = EOS;
											if (!isRightKeyword(sb)) {
												*s = '|';
												strcpy(sb, s);
												sb++;
											} else {
												*s = '|';
												sb = s + 1;
											}
										} else if (!isRightKeyword(sb)) {
											*sb = EOS;
										}
									} else if (!isRightKeyword(sb)) {
										*sb = EOS;
									}
								} else
									*sb = EOS;
							} else if (!isRightKeyword(sb)) {
								*sb = EOS;
							}
						} else if (!isRightKeyword(sb)) {
							*sb = EOS;
						}
					}
				}
			} else
				*sb = EOS;
		} else
			*sb = EOS;
	} else
		*sb = EOS;
	fprintf(dbfpout, "+%s\n", IDs);
/*
	dbPos = ftell(dbfpout);
	fprintf(dbfpout, "                    \n", utterance->speaker, utterance->line);
*/
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

static struct eval_tnode *eval_talloc(char *word, struct database *db) {
	struct eval_tnode *p;

	if ((p=NEW(struct eval_tnode)) == NULL)
		eval_overflow(db, TRUE);
	p->left = p->right = NULL;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		eval_overflow(db, TRUE);
	}
	return(p);
}

static struct eval_tnode *eval_tree(struct eval_tnode *p, char *w, struct speaker *ts, struct database *db) {
	int cond;
	struct eval_tnode *t = p;

	if (p == NULL) {
		ts->diff++;
		p = eval_talloc(w, db);
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0) {
		p->left = eval_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0) {
			p->left = eval_tree(p->left, w, ts, db);
		} else {
			p->right = eval_tree(p->right, w, ts, db); /* if cond > 0 */
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

static char eval_mlu_excludeUtter(char *line, int pos, char *isWordsFound, char *isXXXItem) { // xxx, yyy, www
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
		if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
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
			} else if (strcmp(templineC2, "unk|yyy") == 0 ||	strcmp(templineC2, "unk|www") == 0)
				return(TRUE);
		}
	}
	return(FALSE);	
}

static void removeExtraSpaceFromTier(char *st) {
	int i;
	
	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				strcpy(st+i, st+i+1);
		} else
			i++;
	}
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

static int getStartCodeScopePos(char *ch, int e) {
	int b, sqb, agb;

	for (; e >= 0 && (isSpace(ch[e]) || ch[e] == '\n'); e--) ;
	if (e < 0)
		return(-1);
	sqb = 0;
	for (b=e; b >= 0; b--) {
		if (ch[b] == '[')
			sqb--;
		else if (ch[b] == ']')
			sqb++;
		else if (sqb == 0 && (ch[b] == '>' || !uS.isskip(ch,b,&dFnt,MBF)))
			break;
	}
	if (b < 0)
		return(-1);
	if (ch[b] == '>') {
		agb = 0;
		sqb = 0;
		for (; b >= 0; b--) {
			if (ch[b] == '[')
				sqb--;
			else if (ch[b] == ']')
				sqb++;
			else if (sqb == 0) {
				if (ch[b] == '<') {
					agb--;
					if (agb <= 0)
						break;
				} else if (ch[b] == '>')
					agb++;
			}
		}
		if (b < 0)
			return(-1);
	} else {
		for (; b >= 0; b--) {
			if (uS.isskip(ch,b,&dFnt,MBF) || ch[b] == '<' || ch[b] == '>' || ch[b] == '[' || ch[b] == ']') {
				break;
			}
		}
		b++;
	}
	return(b);
}

static void eval_process_tier(struct speaker *ts, struct database *db, char *rightIDFound, char isOutputGem) {
	int i, j, num;
	char word[1024], tword[1024];
	char tmp, isWordsFound, sq, aq, isSkip;
	char isPSDFound, curPSDFound, isXXXItem, isAuxFound, isAmbigFound;
	long stime, etime;
	float words, morf, mUtt;//, morfsqr;
//	float tLocalmorf, localmorf = (float)0.0;
	struct IDparts IDTier;
	MORFEATS word_feats, *feat;

	if (utterance->speaker[0] == '*') {
		if (isCreateDB) {
			aq = FALSE;
			sq = FALSE;
			for (i=0; utterance->line[i] != EOS; i++) {
				if (uttline[i] == '\n' || uttline[i] == '\t')
					uttline[i] = ' ';
				if (utterance->line[i] == HIDEN_C) {
					if (aq)
						uttline[i] = utterance->line[i];
					aq = !aq;
				}
				if (utterance->line[i] == '[') {
					if (utterance->line[i+1] == '+' || utterance->line[i+1] == '*') {
						sq = TRUE;
					} else if (utterance->line[i+1] == '/') {
						sq = TRUE;
						j = getStartCodeScopePos(utterance->line, i-1);
						if (j > -1) {
							for (; j < i; j++) {
								if (!isSpace(uttline[j]) && uttline[j] != '<' && uttline[j] != '>')
									break;
							}
							if (j == i && isSpace(uttline[i-2])) {
								uttline[i-2] = '&';
							}
						}
					}
				}
				if (sq || aq)
					uttline[i] = utterance->line[i];
				if (utterance->line[i] == ']')
					sq = FALSE;
			}
			uS.remFrontAndBackBlanks(uttline);
			removeExtraSpaceFromTier(uttline);
			fprintf(dbfpout, "*%s\n", uttline);
			return;
		}
		strcpy(spareTier1, utterance->line);
		if (tmDur >= 0.0) {
			ts->tm = ts->tm + tmDur;
			tmDur = -1.0;
		}
//		tLocalmorf = localmorf;
		for (i=0; utterance->line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
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
				ts->words_root = eval_tree(ts->words_root, word, ts, db);
			}
		}
		while ((i=getword(utterance->speaker, utterance->line, word, NULL, i))) {
			if (!strcmp(word, "[//]"))
				ts->retr++;
			else if (!strcmp(word, "[/]"))
				ts->repet++;
		}
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (isCreateDB) {
			for (i=0; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '\n' || utterance->line[i] == '\t')
					utterance->line[i] = ' ';
				if (uS.mStrnicmp(utterance->line+i-3, "unk|xxx", 7) == 0 ||
					uS.mStrnicmp(utterance->line+i, "|www", 4) == 0 ||
					uS.mStrnicmp(utterance->line+i-3, "unk|yyy", 7) == 0)
					;
				else if (utterance->line[i] == 'n' && utterance->line[i+1] == ':') {
					for (j=i+2; isalnum(utterance->line[j]); j++) ;
					if (i+2 < j && utterance->line[j] == '|')
						strcpy(utterance->line+i+1, utterance->line+j);
				} else if (!strncmp(utterance->line+i, "cop|", 4)) {
					strcpy(utterance->line+i+1, utterance->line+i+5);
				} else if ((!strncmp(utterance->line+i, "adv", 3) || !strncmp(utterance->line+i, "det", 3)) &&
							utterance->line[i+3] == ':') {
					for (j=i+4; isalnum(utterance->line[j]); j++) ;
					if (i+4 < j && utterance->line[j] == '|')
						strcpy(utterance->line+i+3, utterance->line+j);
				} else if ((!strncmp(utterance->line+i, "prep", 4) || !strncmp(utterance->line+i, "conj", 4)) &&
						   utterance->line[i+4] == ':') {
					for (j=i+5; isalnum(utterance->line[j]); j++) ;
					if (i+5 < j && utterance->line[j] == '|')
						strcpy(utterance->line+i+4, utterance->line+j);
				} else if (utterance->line[i] == '|') {
					for (j=i+1; utterance->line[j] != '-' && utterance->line[j] != '&' &&
						 utterance->line[j] != '+' && utterance->line[j] != '~' &&
						 utterance->line[j] != '^' && utterance->line[j] != '$' &&
						 utterance->line[j] != '#' && utterance->line[j] != '|' &&
						 !uS.isskip(utterance->line,j,&dFnt,MBF); j++) ;
					if (i+1 < j && utterance->line[j] != '|') {
						if (utterance->line[j] == '+') {
							if (i+2 < j)
								strcpy(utterance->line+i+2, utterance->line+j);
						} else
							strcpy(utterance->line+i+1, utterance->line+j);
					}
				}
			}
			uS.remFrontAndBackBlanks(utterance->line);
			removeExtraSpaceFromTier(utterance->line);
			fprintf(dbfpout, "%%%s\n", utterance->line);
			return;
		}
		if (ts == NULL)
			return;
		filterwords("%", uttline, excludePlusAndUttDels);
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (strchr(templineC2, '|') != NULL) {
				ts->morTotal++;
				for (j=0; templineC2[j] != EOS; j++) {
					if (templineC2[j] == '~')
						ts->morTotal++;
				}
			}
		}
		ts->isMORFound = TRUE;
		words		= ts->words;
		morf		= ts->morf;
		mUtt		= ts->mUtt;
//		morfsqr		= ts->morfsqr;
//		tLocalmorf	= localmorf;
		isPSDFound	= ts->isPSDFound;
		curPSDFound = FALSE;
		isXXXItem = FALSE;
		isWordsFound = FALSE;
		sq = FALSE;
		aq = FALSE;
		isSkip = FALSE;
		for (i=0; spareTier1[i] != EOS; i++) {
			if (eval_mlu_excludeUtter(spareTier1, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
				isSkip = TRUE;
				break;
			}
		}
		i = 0;
		do {
			if (eval_mlu_excludeUtter(uttline, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
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
				tmp = TRUE;
				words = words + 1;
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
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
				curPSDFound = TRUE;
			if (isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
//					if (localmorf != (float)0.0) {
//						morfsqr = morfsqr + localmorf * localmorf;
//						localmorf = (float)0.0;
//					}
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
//			if (localmorf != (float)0.0) {
//				morfsqr = morfsqr + localmorf * localmorf;
//				localmorf = (float)0.0;
//			}
			ts->words	   = words;
			ts->morf	   = morf;
			ts->mUtt	   = mUtt;
//			ts->morfsqr    = morfsqr;
			ts->isPSDFound = curPSDFound;
		}
		if (!isSkip || isXXXItem) {
			for (i=0; spareTier1[i] != EOS; i++) {
				if (spareTier1[i] == HIDEN_C && isdigit(spareTier1[i+1])) {
					if (getMediaTagInfo(spareTier1+i, &stime, &etime)) {
						if (ts->tms < 0.0)
							ts->tms = (float)(stime / 1000);
						ts->tm = (float)(etime / 1000);
						ts->tm = ts->tm - ts->tms;
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
			isXXXItem = FALSE;
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
/*
	clause count
	if (cop* || v || (part && (-PASTP || -PRESP || &PASTP || &PRESP))) {
		verb++
		if (part && isAuxFound)
			clause++
		else if (v || cop*)
			clause++
	} else if (part && isAuxFound)
		clause++ 
*/
			i = 0;
			while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
				word_feats.type = NULL;
				word_feats.typeID = 'R';
				if (ParseWordIntoFeatures(templineC2, &word_feats) == FALSE)
					eval_overflow(db, TRUE);
				for (feat=&word_feats; feat != NULL; feat=feat->comp) {
					if (feat->type != NULL && feat->typeID == '+')
						;
					else if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
						ts->noun++;
					} else if (isAllv(feat) || isnEqual("cop", feat->pos, 3)) {
						ts->verb++;
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
					} else if (isEqual("prep", feat->pos) || isnEqual("prep:", feat->pos, 5)) {
						ts->prep++;
					} else if (isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) {
						ts->adv++;
					} else if (isEqual("conj", feat->pos) || isnEqual("conj:", feat->pos, 5)) {
						ts->conj++;
					} else if (isEqual("pro", feat->pos) || isnEqual("pro:", feat->pos, 4)) {
						ts->pron++;
					} else if (isEqual("det", feat->pos) || isnEqual("det:", feat->pos, 4)) {
						ts->det++;
					}
					if (isSuffix("3s",feat) || isFusion("3s",feat)) {
						ts->thrS++;
					}
					if (isSuffix("13s",feat) || isFusion("13s",feat)) {
						ts->thrnS++;
					}
					if (isSuffix("past",feat) || isFusion("past",feat)) {
						ts->past++;
					}
					if (isSuffix("pastp",feat) || isFusion("pastp",feat)) {
						ts->pastp++;
					}
					if (isSuffix("pl",feat) || isFusion("pl",feat)) {
						ts->pl++;
					}
					if (isSuffix("presp",feat) || isFusion("presp",feat)) {
						ts->presp++;
					}
				}
				freeUpFeats(&word_feats);
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
					eval_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
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
			removeExtraSpaceFromTier(utterance->line);
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

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i') 
		return((eval_group == FALSE && found) || (eval_SpecWords == found));
	else 
		return((eval_group == TRUE && eval_SpecWords > found) || (found == 0));
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  found, t;
	char isfirst, isOutputGem, rightIDFound;
	char word[BUFSIZ];
	float tt;
	FILE *tfpin;
	struct speaker *ts;
	struct IDparts IDTier;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	isfirst = TRUE;
	if (isNOptionSet == FALSE) {
		strcpy(eval_BBS, "@*&#");
		strcpy(eval_CBS, "@*&#");
	}
	if (eval_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
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
					if (eval_SpecWords)
						fprintf(stderr,"\nERROR: No specified gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					eval_overflow(db, FALSE);
				}
				db->tm_sqr = db->tm_sqr + (ts->tm * ts->tm);
				db->tm = db->tm + ts->tm;
				if (db->mn_tm == (float)0.0 || db->mn_tm > ts->tm)
					db->mn_tm = ts->tm;
				if (db->mx_tm < ts->tm)
					db->mx_tm = ts->tm;
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
				db->morTotal_sqr = db->morTotal_sqr + (ts->morTotal * ts->morTotal);
				db->morTotal = db->morTotal + ts->morTotal;
				if (db->mn_morTotal == (float)0.0 || db->mn_morTotal > ts->morTotal)
					db->mn_morTotal = ts->morTotal;
				if (db->mx_morTotal < ts->morTotal)
					db->mx_morTotal = ts->morTotal;
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
					if (ts->morTotal == 0.0)
						ts->pl = 0.000;
					else
						ts->pl = (ts->pl/ts->morTotal)*100.0000;
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
						ts->thrS = 0.000;
					else
						ts->thrS = (ts->thrS/ts->morTotal)*100.0000;
				}
				db->thrS_sqr = db->thrS_sqr + (ts->thrS * ts->thrS);
				db->thrS = db->thrS + ts->thrS;
				if (db->mn_thrS == (float)0.0 || db->mn_thrS > ts->thrS)
					db->mn_thrS = ts->thrS;
				if (db->mx_thrS < ts->thrS)
					db->mx_thrS = ts->thrS;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->thrnS = 0.000;
					else
						ts->thrnS = (ts->thrnS/ts->morTotal)*100.0000;
				}
				db->thrnS_sqr = db->thrnS_sqr + (ts->thrnS * ts->thrnS);
				db->thrnS = db->thrnS + ts->thrnS;
				if (db->mn_thrnS == (float)0.0 || db->mn_thrnS > ts->thrnS)
					db->mn_thrnS = ts->thrnS;
				if (db->mx_thrnS < ts->thrnS)
					db->mx_thrnS = ts->thrnS;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->past = 0.000;
					else
						ts->past = (ts->past/ts->morTotal)*100.0000;
				}
				db->past_sqr = db->past_sqr + (ts->past * ts->past);
				db->past = db->past + ts->past;
				if (db->mn_past == (float)0.0 || db->mn_past > ts->past)
					db->mn_past = ts->past;
				if (db->mx_past < ts->past)
					db->mx_past = ts->past;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->pastp = 0.000;
					else
						ts->pastp = (ts->pastp/ts->morTotal)*100.0000;
				}
				db->pastp_sqr = db->pastp_sqr + (ts->pastp * ts->pastp);
				db->pastp = db->pastp + ts->pastp;
				if (db->mn_pastp == (float)0.0 || db->mn_pastp > ts->pastp)
					db->mn_pastp = ts->pastp;
				if (db->mx_pastp < ts->pastp)
					db->mx_pastp = ts->pastp;
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						ts->presp = 0.000;
					else
						ts->presp = (ts->presp/ts->morTotal)*100.0000;
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
				db->repet_sqr = db->repet_sqr + (ts->repet * ts->repet);
				db->repet = db->repet + ts->repet;
				if (db->mn_repet == (float)0.0 || db->mn_repet > ts->repet)
					db->mn_repet = ts->repet;
				if (db->mx_repet < ts->repet)
					db->mx_repet = ts->repet;
				db->db_sp = freespeakers(db->db_sp);
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(eval_BBS, "@*&#");
				strcpy(eval_CBS, "@*&#");
			}
			if (eval_SpecWords) {
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
				strcpy(utterance->speaker, "%mor:");
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
			if (eval_SpecWords && !strcmp(eval_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					eval_n_option = FALSE;
					strcpy(eval_BBS, "@BG:");
					strcpy(eval_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					eval_n_option = TRUE;
					strcpy(eval_BBS, "@G:");
					strcpy(eval_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,eval_BBS,FALSE,FALSE)) {
				if (eval_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,eval_CBS,FALSE,FALSE)) {
				if (eval_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (eval_SpecWords)
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
					ts = eval_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
					if (ts != NULL && isfirst) {
						isfirst = FALSE;
						db->num = db->num + 1.0;
						t = db->num;
						if (t == 1 || t % 15 == 0)
							fprintf(stderr,"\r%.0f ", db->num);
					}
				}
				eval_process_tier(ts, db, NULL, isOutputGem);
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
		eval_overflow(db, FALSE);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (eval_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		eval_overflow(db, FALSE);
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

static void eval_pr_result(void) {
	int    i, SDn;
	char   st[1024], *sFNane;
	float  num;
	struct speaker *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;

	if (isCreateDB)
		return;
/*
	if (!stout) {
		char  fontName[256];
#ifdef _WIN32 
		fprintf(fpout, "%s	Win95:CAfont:-15:0\n", FONTHEADER);
#else
		fprintf(fpout, "%s	CAfont:13:7\n", FONTHEADER);
#endif
	} else {
		strcpy(fontName, "CAfont:13:7\n");
		cutt_SetNewFont(fontName, EOS);
	}
*/
	if (sp_head == NULL) {
		if (eval_SpecWords && !onlyApplyToDB) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
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
//	fprintf(fpout,"\tSpeaker ID\tDuration\t# Utterances\t%% Intelligible Utts\t# Utts in MLU\tMLU Words\tMLU Morphemes");
	fprintf(fpout,"\tSpeaker ID\tDuration\tTotal Utts\tMLU Utts\tMLU Words\tMLU Morphemes");
	fprintf(fpout,"\ttypes\ttokens\tTTR\tClause/Utt");
//	fprintf(fpout,"\tExclamations\tInterruptions\tStatements\tQuestions");
//	fprintf(fpout,"\t+.");
	if (!isRawVal) {
		fprintf(fpout,"\t%% Word Errors\tUtt Errors");
	} else {
		fprintf(fpout,"\tWord Errors\tUtt Errors");
	}
	fprintf(fpout,"\tmor Words");
	if (!isRawVal) {
		fprintf(fpout,"\t%% Nouns\t%% Plurals");
		fprintf(fpout,"\t%% Verbs\t%% 3S\t%% 1S/3S\t%% PAST\t%% PASTP\t%% PRESP");
		fprintf(fpout,"\t%% prep|\t%% adv|\t%% conj|\t%% det|\t%% pro|");
	} else {
		fprintf(fpout,"\tNouns\tPlurals");
		fprintf(fpout,"\tVerbs\t3S\t1S/3S\tPAST\tPASTP\tPRESP");
		fprintf(fpout,"\tprep|\tadv|\tconj|\tdet|\tpro|");
	}
	fprintf(fpout,"\tretracing[//]\trepetition[/]\n");
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (eval_SpecWords) {
				fprintf(stderr,"\nERROR: No specified gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			} else
				fprintf(stderr,"\nERROR: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
		sFNane = strrchr(ts->fname, PATHDELIMCHR);
		if (sFNane != NULL)
			sFNane = sFNane + 1;
		else
			sFNane = ts->fname;
		fprintf(fpout,"%s",sFNane);
		if (ts->ID) {
			fprintf(fpout,"\t%s",ts->ID);
		} else
			fprintf(fpout,"\t%s",ts->sp);
		if (!ts->isMORFound) {
			fprintf(stderr,"\nWARNING: No %%mor: tier found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, sFNane);
			fprintf(fpout,"\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n");
			continue;
		}
		Secs2Str(ts->tm, st);
		fprintf(fpout,"\t%s", st);
		if (isXXXFound)
			ts->tUtt = ts->mUtt;
		else
			ts->tUtt = ts->xxxut + ts->mUtt;
		fprintf(fpout, "\t%.0f",ts->tUtt);
/*
		if (ts->tUtt == 0.0)
			fprintf(fpout, "\tN/A");
		else {
			ts->xxxut = (100 - (ts->xxxut * 100) / ts->tUtt);
			fprintf(fpout, "\t%.0f",ts->xxxut);
		}
*/
		fprintf(fpout, "\t%.0f",ts->mUtt);
		if (ts->mUtt == 0.0)
			fprintf(fpout, "\tN/A\tN/A");
		else
			fprintf(fpout, "\t%.3f\t%.3f",ts->words/ts->mUtt,ts->morf/ts->mUtt);
		fprintf(fpout, "\t%.0f\t%.0f",ts->diff,ts->total);
		if (ts->total == 0.0)
			fprintf(fpout,"\tN/A");
		else
			fprintf(fpout,"\t%.3f",ts->diff/ts->total);
		if (ts->tUtt == 0.0)
			fprintf(fpout,"\tN/A");
		else
			fprintf(fpout,"\t%.3f",ts->clause/ts->tUtt);
//		fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f",ts->exla,ts->intr,ts->peri,ts->ques);
//		fprintf(fpout, "\t%.0f",ts->tbrk);
		if (!isRawVal) {
			if (ts->total == 0.0)
				ts->werr = 0.000;
			else
				ts->werr = (ts->werr/ts->total)*100.0000;
			fprintf(fpout, "\t%.3f", ts->werr);
		} else
			fprintf(fpout, "\t%.0f",ts->werr);
		fprintf(fpout, "\t%.0f",ts->uerr);
		fprintf(fpout, "\t%.0f",ts->morTotal);
		if (!isRawVal) {
			if (ts->morTotal == 0.0) {
				ts->noun = 0.000;
				ts->pl = 0.000;
				ts->verb = 0.000;
				ts->thrS = 0.000;
				ts->thrnS = 0.000;
				ts->past = 0.000;
				ts->pastp = 0.000;
				ts->presp = 0.000;
				ts->prep = 0.000;
				ts->adv = 0.000;
				ts->conj = 0.000;
				ts->det = 0.000;
				ts->pron = 0.000;
			} else {
				ts->noun = (ts->noun/ts->morTotal)*100.0000;
				ts->pl = (ts->pl/ts->morTotal)*100.0000;
				ts->verb = (ts->verb/ts->morTotal)*100.0000;
				ts->thrS = (ts->thrS/ts->morTotal)*100.0000;
				ts->thrnS = (ts->thrnS/ts->morTotal)*100.0000;
				ts->past = (ts->past/ts->morTotal)*100.0000;
				ts->pastp = (ts->pastp/ts->morTotal)*100.0000;
				ts->presp = (ts->presp/ts->morTotal)*100.0000;
				ts->prep = (ts->prep/ts->morTotal)*100.0000;
				ts->adv = (ts->adv/ts->morTotal)*100.0000;
				ts->conj = (ts->conj/ts->morTotal)*100.0000;
				ts->det = (ts->det/ts->morTotal)*100.0000;
				ts->pron = (ts->pron/ts->morTotal)*100.0000;
			}
			fprintf(fpout, "\t%.3f", ts->noun);
			fprintf(fpout, "\t%.3f", ts->pl);
			fprintf(fpout, "\t%.3f", ts->verb);
			fprintf(fpout, "\t%.3f", ts->thrS);
			fprintf(fpout, "\t%.3f", ts->thrnS);
			fprintf(fpout, "\t%.3f", ts->past);
			fprintf(fpout, "\t%.3f", ts->pastp);
			fprintf(fpout, "\t%.3f", ts->presp);
			fprintf(fpout, "\t%.3f", ts->prep);
			fprintf(fpout, "\t%.3f", ts->adv);
			fprintf(fpout, "\t%.3f", ts->conj);
			fprintf(fpout, "\t%.3f", ts->det);
			fprintf(fpout, "\t%.3f", ts->pron);
		} else {
			fprintf(fpout, "\t%.0f", ts->noun);
			fprintf(fpout, "\t%.0f", ts->pl);
			fprintf(fpout, "\t%.0f", ts->verb);
			fprintf(fpout, "\t%.0f", ts->thrS);
			fprintf(fpout, "\t%.0f", ts->thrnS);
			fprintf(fpout, "\t%.0f", ts->past);
			fprintf(fpout, "\t%.0f", ts->pastp);
			fprintf(fpout, "\t%.0f", ts->presp);
			fprintf(fpout, "\t%.0f", ts->prep);
			fprintf(fpout, "\t%.0f", ts->adv);
			fprintf(fpout, "\t%.0f", ts->conj);
			fprintf(fpout, "\t%.0f", ts->det);
			fprintf(fpout, "\t%.0f", ts->pron);
		}
		fprintf(fpout, "\t%.0f\t%.0f",ts->retr,ts->repet);
		fprintf(fpout, "\n");
		if (DBKeyRoot != NULL) {
			num = db->num;
			fprintf(fpout,"+/-SD\t");
			SDn = 0; // this number should be less than SDRESSIZE = 256
			compute_SD(&SD[SDn++], ts->tm,  NULL, db->tm_sqr, db->tm, num);
			compute_SD(&SD[SDn++], ts->tUtt,  NULL, db->tUtt_sqr, db->tUtt, num);
//			compute_SD(&SD[SDn++], ts->xxxut,  NULL, db->xxxut_sqr, db->xxxut, num);
			compute_SD(&SD[SDn++], ts->mUtt,  NULL, db->mUtt_sqr, db->mUtt, num);
			compute_SD(&SD[SDn++], ts->words,  &ts->mUtt, db->words_sqr, db->words, num);
			compute_SD(&SD[SDn++], ts->morf,  &ts->mUtt, db->morf_sqr, db->morf, num);
			compute_SD(&SD[SDn++], ts->diff,  NULL, db->diff_sqr, db->diff, num);
			compute_SD(&SD[SDn++], ts->total,  NULL, db->total_sqr, db->total, num);
			compute_SD(&SD[SDn++], ts->diff,  &ts->total, db->TTR_sqr, db->TTR, num);
			compute_SD(&SD[SDn++], ts->clause,  &ts->tUtt, db->CUR_sqr, db->CUR, num);
//			compute_SD(&SD[SDn++], ts->exla,  NULL, db->exla_sqr, db->exla, num);
//			compute_SD(&SD[SDn++], ts->intr,  NULL, db->intr_sqr, db->intr, num);
//			compute_SD(&SD[SDn++], ts->peri,  NULL, db->peri_sqr, db->peri, num);
//			compute_SD(&SD[SDn++], ts->ques,  NULL, db->ques_sqr, db->ques, num);
//			compute_SD(&SD[SDn++], ts->tbrk,  NULL, db->tbrk_sqr, db->tbrk, num);
			compute_SD(&SD[SDn++], ts->werr,  NULL, db->werr_sqr, db->werr, num);
			compute_SD(&SD[SDn++], ts->uerr,  NULL, db->uerr_sqr, db->uerr, num);
			compute_SD(&SD[SDn++], ts->morTotal,  NULL, db->morTotal_sqr, db->morTotal, num);
			compute_SD(&SD[SDn++], ts->noun,  NULL, db->noun_sqr, db->noun, num);
			compute_SD(&SD[SDn++], ts->pl,  NULL, db->pl_sqr, db->pl, num);
			compute_SD(&SD[SDn++], ts->verb,  NULL, db->verb_sqr, db->verb, num);
			compute_SD(&SD[SDn++], ts->thrS,  NULL, db->thrS_sqr, db->thrS, num);
			compute_SD(&SD[SDn++], ts->thrnS,  NULL, db->thrnS_sqr, db->thrnS, num);
			compute_SD(&SD[SDn++], ts->past,  NULL, db->past_sqr, db->past, num);
			compute_SD(&SD[SDn++], ts->pastp,  NULL, db->pastp_sqr, db->pastp, num);
			compute_SD(&SD[SDn++], ts->presp,  NULL, db->presp_sqr, db->presp, num);
			compute_SD(&SD[SDn++], ts->prep,  NULL, db->prep_sqr, db->prep, num);
			compute_SD(&SD[SDn++], ts->adv,  NULL, db->adv_sqr, db->adv, num);
			compute_SD(&SD[SDn++], ts->conj,  NULL, db->conj_sqr, db->conj, num);
			compute_SD(&SD[SDn++], ts->det,  NULL, db->det_sqr, db->det, num);
			compute_SD(&SD[SDn++], ts->pron,  NULL, db->pron_sqr, db->pron, num);
			compute_SD(&SD[SDn++], ts->retr,  NULL, db->retr_sqr, db->retr, num);
			compute_SD(&SD[SDn++], ts->repet,  NULL, db->repet_sqr, db->repet, num);
			fprintf(fpout, "\n");
			fprintf(fpout,"\t");
			for (i=0; i < SDn; i++)
				fprintf(fpout, "\t%c%c",((SD[i].stars >= 1) ? '*' : ' '),((SD[i].stars >= 2) ? '*' : ' '));
			fprintf(fpout, "\n");
		}
	}
	if (DBKeyRoot != NULL) {
		num = db->num;
		fprintf(fpout,"Mean Database\t");
		Secs2Str(db->tm/num, st);
		fprintf(fpout,"\t%s", st);
		fprintf(fpout, "\t%.3f",db->tUtt/num);
//		fprintf(fpout, "\t%.3f",db->xxxut/num);
		fprintf(fpout, "\t%.3f",db->mUtt/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->words/num,db->morf/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->diff/num,db->total/num);
		fprintf(fpout, "\t%.3f",db->TTR/num);
		fprintf(fpout, "\t%.3f",db->CUR/num);
//		fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f",db->exla/num,db->intr/num,db->peri/num,db->ques/num);
//		fprintf(fpout, "\t%.3f",db->tbrk/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->werr/num,db->uerr/num);
		fprintf(fpout, "\t%.3f",db->morTotal/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->noun/num,db->pl/num);
		fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
				db->verb/num,db->thrS/num,db->thrnS/num,db->past/num,db->pastp/num,db->presp/num);
		fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",db->prep/num,db->adv/num,db->conj/num,db->det/num,db->pron/num);
		fprintf(fpout, "\t%.3f\t%.3f",db->retr/num,db->repet/num);
		fprintf(fpout, "\n");

		fprintf(fpout,"Min Database\t");
		Secs2Str(db->mn_tm, st);
		fprintf(fpout,"\t%s", st);
		fprintf(fpout, "\t%.0f",db->mn_tUtt);
//		fprintf(fpout, "\t%.0f",db->mn_xxxut);
		fprintf(fpout, "\t%.0f",db->mn_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mn_words,db->mn_morf);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_diff,db->mn_total);
		fprintf(fpout, "\t%.3f",db->mn_TTR);
		fprintf(fpout, "\t%.3f",db->mn_CUR);
//		fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f",db->mn_exla,db->mn_intr,db->mn_peri,db->mn_ques);
//		fprintf(fpout, "\t%.0f",db->mn_tbrk);
		if (!isRawVal)
			fprintf(fpout, "\t%.3f\t%.0f",db->mn_werr,db->mn_uerr);
		else
			fprintf(fpout, "\t%.0f\t%.0f",db->mn_werr,db->mn_uerr);
		fprintf(fpout, "\t%.0f", db->mn_morTotal);
		if (!isRawVal) {
			fprintf(fpout, "\t%.3f\t%.3f",db->mn_noun,db->mn_pl);
			fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
					db->mn_verb,db->mn_thrS,db->mn_thrnS,db->mn_past,db->mn_pastp,db->mn_presp);
			fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",db->mn_prep,db->mn_adv,db->mn_conj,db->mn_det,db->mn_pron);
		} else {
			fprintf(fpout, "\t%.0f\t%.0f",db->mn_noun,db->mn_pl);
			fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f",
					db->mn_verb,db->mn_thrS,db->mn_thrnS,db->mn_past,db->mn_pastp,db->mn_presp);
			fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f",db->mn_prep,db->mn_adv,db->mn_conj,db->mn_det,db->mn_pron);
		}
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_retr,db->mn_repet);
		fprintf(fpout, "\n");

		fprintf(fpout,"Max Database\t");
		Secs2Str(db->mx_tm, st);
		fprintf(fpout,"\t%s", st);
		fprintf(fpout, "\t%.0f",db->mx_tUtt);
//		fprintf(fpout, "\t%.0f",db->mx_xxxut);
		fprintf(fpout, "\t%.0f",db->mx_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mx_words,db->mx_morf);
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_diff,db->mx_total);
		fprintf(fpout, "\t%.3f",db->mx_TTR);
		fprintf(fpout, "\t%.3f",db->mx_CUR);
//		fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f",db->mx_exla,db->mx_intr,db->mx_peri,db->mx_ques);
//		fprintf(fpout, "\t%.0f",db->mx_tbrk);
		if (!isRawVal)
			fprintf(fpout, "\t%.3f\t%.0f",db->mx_werr,db->mx_uerr);
		else
			fprintf(fpout, "\t%.0f\t%.0f",db->mx_werr,db->mx_uerr);
		fprintf(fpout, "\t%.0f",db->mx_morTotal);
		if (!isRawVal) {
			fprintf(fpout, "\t%.3f\t%.3f",db->mx_noun,db->mx_pl);
			fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
					db->mx_verb,db->mx_thrS,db->mx_thrnS,db->mx_past,db->mx_pastp,db->mx_presp);
			fprintf(fpout, "\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",db->mx_prep,db->mx_adv,db->mx_conj,db->mx_det,db->mx_pron);
		} else {
			fprintf(fpout, "\t%.0f\t%.0f",db->mx_noun,db->mx_pl);
			fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f",
					db->mx_verb,db->mx_thrS,db->mx_thrnS,db->mx_past,db->mx_pastp,db->mx_presp);
			fprintf(fpout, "\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f",db->mx_prep,db->mx_adv,db->mx_conj,db->mx_det,db->mx_pron);
		}
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_retr,db->mx_repet);
		fprintf(fpout, "\n");

		fprintf(fpout,"SD Database\t");
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
			fprintf(fpout, "Database gems:");
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

static char isEXCPostCode(char *s) {
	for (; *s != EOS; s++) {
		if (!uS.mStrnicmp(s, "[+ exc]", 7))
			return(TRUE);
	}
	return(FALSE);
}

void call() {
	int  found;
	long tlineno = 0;
	char lRightspeaker;
	char isOutputGem;
	char word[BUFSIZ];
	struct speaker *ts;

	if (isCreateDB == 2) {
		found = strlen("/TalkBank/data-orig/AphasiaBank/English/");
		if (uS.mStrnicmp(oldfname+found, "Aphasia", 7) && uS.mStrnicmp(oldfname+found, "Control", 7))
			return;
	} else
		fprintf(stderr,"From file <%s>\n",oldfname);
	tmDur = -1.0;
	ts = NULL;
	if (isGOptionSet == FALSE)
		onlyApplyToDB = !isFoundGems();
	if (!onlyApplyToDB && isNOptionSet == FALSE) {
		strcpy(eval_BBS, "@*&#");
		strcpy(eval_CBS, "@*&#");
	}
	if (isCreateDB) {
		if (dbfpout == NULL) {
			strcpy(FileName1, "/TalkBank/data");
			addFilename2Path(FileName1, DATABASE_FILE_NAME);
			dbfpout = fopen(FileName1, "wb");
			if (dbfpout == NULL) {
				if (isCreateDB == 2) {
					fprintf(stderr,"Can't create file \"%s\", perhaps path doesn't exist or it is opened by another application\n",FileName1);
					eval_overflow(NULL, FALSE);
				} else {
					strcpy(FileName1, wd_dir);
					addFilename2Path(FileName1, DATABASE_FILE_NAME);
					dbfpout = fopen(FileName1, "wb");
					if (dbfpout == NULL) {
						fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
						eval_overflow(NULL, FALSE);
					}
				}
			}
		}
		isOutputGem = TRUE;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (utterance->speaker[0] == '*')
				break;
			if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (strncmp(utterance->line, "THIS FILE HAS NOT YET ", 22) == 0) {
					if (isCreateDB != 2)
						fprintf(stderr,"    File <%s> is ignored\n",oldfname);
					return;
				}
			}
		}
		rewind(fpin);
		if (isDebug)
			fprintf(fpout, "From file <%s>\n", oldfname);
	} else if (eval_SpecWords && !onlyApplyToDB) {
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
#if !defined(CLAN_SRV)
			if (isCreateDB != 2)
				fprintf(stderr,"\r%ld ",lineno);
#endif
		}
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
				if (isEXCPostCode(utterance->line))
					lRightspeaker = FALSE;
				else 
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!onlyApplyToDB && eval_SpecWords && !strcmp(eval_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				eval_n_option = FALSE;
				strcpy(eval_BBS, "@BG:");
				strcpy(eval_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				eval_n_option = TRUE;
				strcpy(eval_BBS, "@G:");
				strcpy(eval_CBS, "@*&#");
			}
		}
		if (isCreateDB) {
			if (utterance->speaker[0] == '@') {
				uS.remFrontAndBackBlanks(utterance->line);
				removeExtraSpaceFromTier(utterance->line);
			}
			if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE))
				fprintf(dbfpout, "G%s\n", utterance->line);
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE))
				fprintf(dbfpout, "B%s\n", utterance->line);
			if (uS.partcmp(utterance->speaker,"@EG:",FALSE,FALSE))
				fprintf(dbfpout, "E%s\n", utterance->line);
			if (uS.partcmp(utterance->speaker,"@End",FALSE,FALSE)) {
				fprintf(dbfpout, "-\n");
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
		} else if (!onlyApplyToDB && uS.partcmp(utterance->speaker,eval_BBS,FALSE,FALSE)) {
			if (eval_n_option) {
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
		} else if (found > 0 && uS.partcmp(utterance->speaker,eval_CBS,FALSE,FALSE)) {
			if (eval_n_option) {
			} else {
				if (isRightText(word)) {
					found--;
					if (found == 0) {
						if (eval_SpecWords)
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
				ts = eval_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
			}
			eval_process_tier(ts, NULL, NULL, isOutputGem);
		}
	}
#if !defined(CLAN_SRV)
	if (isCreateDB != 2)
		fprintf(stderr, "\r	  \r");
#endif
	if (!combinput)
		eval_pr_result();
}

void init(char first) {
	int  i;
	char *f;

	if (first) {
		isRawVal = FALSE;
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
//		addword('\0','\0',"+<\">");
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
		isDebug = FALSE;
		isXXXFound = FALSE;
		GemMode = '\0';
		eval_SpecWords = 0;
		eval_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		eval_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(eval_BBS, "@*&#");
		strcpy(eval_CBS, "@*&#");
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
				eval_overflow(NULL, FALSE);
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
				fprintf(stderr,"Please specify speaker's tier code with Option button in Commands window.\n");
				fprintf(stderr,"Or with \"+t\" option on command line.\n");
#else
				fprintf(stderr,"Please specify speaker's tier code with \"+t\" option on command line.\n");
#endif
				eval_overflow(NULL, FALSE);
			}
			maketierchoice("*:", '+', '\001');
			if (isCreateDB) {
			} else if (DBKeyRoot != NULL) {
				db = NEW(struct database);
				if (db == NULL)
					eval_overflow(db, TRUE);
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
		}
	}
	if (!combinput || first) {
		sp_head = NULL;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i;
	FNType twd_dir[FNSize];

	targv = argv;
	targc = argc;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = EVAL;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	isCreateDB = 0;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'c') {
				if (argv[i][2] == '1')
					isCreateDB = 2;
				else
					isCreateDB = 1;
			}
		}
	}
	if (isCreateDB == 2) {
		strcpy(StarDotCHA, "*.cha");
		if (argv[3] == NULL) {
			argc++;
			argv[3] = StarDotCHA;
		}
		strcpy(twd_dir, wd_dir);
		strcpy(wd_dir, "/TalkBank/data-orig/AphasiaBank/English");
	}
	bmain(argc,argv,eval_pr_result);
	if (isCreateDB == 2)
		strcpy(wd_dir, twd_dir);
	db = freedb(db);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	if (dbfpout != NULL) {
		if (isCreateDB != 2)
			fprintf(stderr,"Output file <%s>\n",FileName1);
		fclose(dbfpout);
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
		case 'c':
			if (*f == '1')
				isCreateDB = 2;
			else
				isCreateDB = 1;
			break;
		case 'd':
			if (*f == EOS) {
				isDebug = TRUE;
			} else {
				addDBKeys(f);
			}
			break;
		case 'g':
			if (*f == EOS) {
				if (*(f-2) == '+')
					eval_group = TRUE;
				else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				eval_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
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
				eval_n_option = TRUE;
				strcpy(eval_BBS, "@G:");
				strcpy(eval_CBS, "@*&#");
			} else {
				strcpy(eval_BBS, "@BG:");
				strcpy(eval_CBS, "@EG:");
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
				eval_overflow(NULL, FALSE);
			}
			break;
		case 't':
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
