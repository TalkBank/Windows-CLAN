/**********************************************************************
 "Copyright 1990-2012 Brian MacWhinney. Use is subject to Gnu Public License
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
#define _main measures_main
#define call measures_call
#define getflag measures_getflag
#define init measures_init
#define usage measures_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "eval_db.txt"

extern char OverWriteFile;
extern char outputOnlyData;
extern char *AddCEXExtension;
extern char GlobalPunctuation[];
extern char isRecursive;
extern struct tier *defheadtier;
extern struct IDtype *IDField;

#define SDRESSIZE 256

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct measures_tnode {
	char *word;
	struct measures_tnode *left;
	struct measures_tnode *right;
};

struct speaker {
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
	struct measures_tnode *words_root;
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
static int  measures_SpecWords, DBGemNum;
static char *DBGems[NUMGEMITEMS];
static char isSpeakerNameGiven, isXXXFound, ftime, isDebug;
static char measures_BBS[5], measures_CBS[5], measures_group, measures_n_option, isNOptionSet, onlyApplyToDB, isGOptionSet, GemMode;
static struct DBKeys *DBKeyRoot;
static struct speaker *sp_head;
static struct database *db;

void usage() {
	printf("Usage: measures [bS dS g gS n s %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(UNX) || defined(_MAC_CODE)
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
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+s : counts utterances and other words found along with \"xxx\"");
	mainusage(FALSE);
	puts("\nExample:");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70");
	puts("       measures adler15a.cha +t*par +gCinderella +u +d\"control|60-70\"");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       measures adler15a.cha +t*par +gCinderella +u +d\"control|60-70;6\"");
	puts("   Search database for \"Cinderella\" gems of \"Nonfluent\" subjects of any age");
	puts("       measures adler15a.cha +t*par +gCinderella +u +dNonfluent");
	puts("   Just compute results for \"Cinderella\" gems of adler15a.cha. Do not compare to database");
	puts("       measures adler15a.cha +t*par +gCinderella +u");
	cutt_exit(0);
}

static void init_db(struct database *p) {
	p->num = 0.0;
	p->tUtt_sqr = (float)0.0; p->tUtt = (float)0.0; p->mn_tUtt = (float)0.0; p->mx_tUtt = (float)0.0;
	p->total_sqr = (float)0.0; p->total = (float)0.0; p->mn_total = (float)0.0; p->mx_total = (float)0.0;
	p->diff_sqr = (float)0.0; p->diff = (float)0.0; p->mn_diff = (float)0.0; p->mx_diff = (float)0.0;
	p->TTR_sqr = (float)0.0; p->TTR = (float)0.0; p->mn_TTR = (float)0.0; p->mx_TTR = (float)0.0;
	p->CUR_sqr = (float)0.0; p->CUR = (float)0.0; p->mn_CUR = (float)0.0; p->mx_CUR = (float)0.0;
	p->mUtt_sqr = (float)0.0; p->mUtt = (float)0.0; p->mn_mUtt = (float)0.0; p->mx_mUtt = (float)0.0;
	p->words_sqr = (float)0.0; p->words = (float)0.0; p->mn_words = (float)0.0; p->mx_words = (float)0.0;
	p->morf_sqr = (float)0.0; p->morf = (float)0.0; p->mn_morf = (float)0.0; p->mx_morf = (float)0.0;
	p->mUtt100_sqr = (float)0.0; p->mUtt100 = (float)0.0; p->mn_mUtt100 = (float)0.0; p->mx_mUtt100 = (float)0.0;
	p->words100_sqr = (float)0.0; p->words100 = (float)0.0; p->mn_words100 = (float)0.0; p->mx_words100 = (float)0.0;
	p->morf100_sqr = (float)0.0; p->morf100 = (float)0.0; p->mn_morf100 = (float)0.0; p->mx_morf100 = (float)0.0;
	p->xxxut_sqr = (float)0.0; p->xxxut = (float)0.0; p->mn_xxxut = (float)0.0; p->mx_xxxut = (float)0.0;
	p->tdWords_sqr = (float)0.0; p->tdWords = (float)0.0; p->mn_tdWords = (float)0.0; p->mx_tdWords = (float)0.0;
	p->tdUtts_sqr = (float)0.0; p->tdur = (float)0.0; p->mn_tdUtts = (float)0.0; p->mx_tdur = (float)0.0;
	p->tdur_sqr = (float)0.0; p->tdUtts = (float)0.0; p->mn_tdur = (float)0.0; p->mx_tdUtts = (float)0.0;
	p->tdWT_sqr = (float)0.0; p->tdWT = (float)0.0; p->mn_tdWT = (float)0.0; p->mx_tdWT = (float)0.0;
	p->tdUT_sqr = (float)0.0; p->tdUT = (float)0.0; p->mn_tdUT = (float)0.0; p->mx_tdUT = (float)0.0;
	p->exla_sqr = (float)0.0; p->exla = (float)0.0; p->mn_exla = (float)0.0; p->mx_exla = (float)0.0;
	p->intr_sqr = (float)0.0; p->intr = (float)0.0; p->mn_intr = (float)0.0; p->mx_intr = (float)0.0;
	p->peri_sqr = (float)0.0; p->peri = (float)0.0; p->mn_peri = (float)0.0; p->mx_peri = (float)0.0;
	p->ques_sqr = (float)0.0; p->ques = (float)0.0; p->mn_ques = (float)0.0; p->mx_ques = (float)0.0;
	p->tbrk_sqr = (float)0.0; p->tbrk = (float)0.0; p->mn_tbrk = (float)0.0; p->mx_tbrk = (float)0.0;
	p->werr_sqr = (float)0.0; p->werr = (float)0.0; p->mn_werr = (float)0.0; p->mx_werr = (float)0.0;
	p->uerr_sqr = (float)0.0; p->uerr = (float)0.0; p->mn_uerr = (float)0.0; p->mx_uerr = (float)0.0;
	p->retr_sqr = (float)0.0; p->retr = (float)0.0; p->mn_retr = (float)0.0; p->mx_retr = (float)0.0;
	p->repet_sqr = (float)0.0; p->repet = (float)0.0; p->mn_repet = (float)0.0; p->mx_repet = (float)0.0;
	p->db_sp = NULL;
}

static void measures_freetree(struct measures_tnode *p) {
	if (p != NULL) {
		measures_freetree(p->left);
		measures_freetree(p->right);
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
			measures_freetree(ts->words_root);
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

static void measures_overflow(struct database *db, char IsOutOfMem) {
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
		measures_overflow(NULL, FALSE);
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

static char *measures_strsave(char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		measures_overflow(NULL, FALSE);
	return(p);
}

static struct DBKeys *initDBKey(char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		measures_overflow(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = measures_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = measures_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = measures_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = measures_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = measures_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = measures_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = measures_strsave("Wernicke");
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
		DBKey->key1 = measures_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = measures_strsave("Wernicke");
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
					DBKey->key1 = measures_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = measures_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = measures_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = measures_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = measures_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					measures_overflow(NULL, FALSE);
				}
				DBKey->age = measures_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control, \n");
				fprintf(stderr, "    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),\n");
				fprintf(stderr, "    Nonfluent = (Global, Broca, Transmotor),\n");
				fprintf(stderr, "    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)\n");
				measures_overflow(NULL, FALSE);
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

static struct speaker *measures_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
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
			measures_overflow(db, TRUE);
		}
	}
	if ((ts=NEW(struct speaker)) == NULL)
		measures_overflow(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		measures_overflow(db, TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		measures_overflow(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			measures_overflow(db, TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;
	ts->tUtt	= (float)0.0;
	ts->total   = (float)0.0;
	ts->diff    = (float)0.0;
	ts->clause	= (float)0.0;
	ts->words   = (float)0.0;
	ts->morf	= (float)0.0;
	ts->mUtt	= (float)0.0;
	ts->words100= (float)0.0;
	ts->morf100	= (float)0.0;
	ts->mUtt100	= (float)0.0;
	ts->xxxut   = (float)0.0;
	ts->tdWords	= (float)0.0;
	ts->tdUtts	= (float)0.0;
	ts->tdur	= (float)0.0;
	ts->exla    = (float)0.0;
	ts->intr    = (float)0.0;
	ts->peri    = (float)0.0;
	ts->ques    = (float)0.0;
	ts->tbrk    = (float)0.0;
	ts->werr    = (float)0.0;
	ts->uerr    = (float)0.0;
	ts->retr    = (float)0.0;
	ts->repet   = (float)0.0;
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

static struct measures_tnode *measures_talloc(char *word, struct database *db) {
	struct measures_tnode *p;

	if ((p=NEW(struct measures_tnode)) == NULL)
		measures_overflow(db, TRUE);
	p->left = p->right = NULL;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		measures_overflow(db, TRUE);
	}
	return(p);
}

static struct measures_tnode *measures_tree(struct measures_tnode *p, char *w, struct speaker *ts, struct database *db) {
	int cond;
	struct measures_tnode *t = p;

	if (p == NULL) {
		ts->diff++;
		p = measures_talloc(w, db);
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0) {
		p->left = measures_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0) {
			p->left = measures_tree(p->left, w, ts, db);
		} else {
			p->right = measures_tree(p->right, w, ts, db); /* if cond > 0 */
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

static char measures_mlu_excludeUtter(char *line, int pos, char *isWordsFound, char *isXXXItem) { // xxx, yyy, www
	char isSkipFirst = FALSE;

	if (MBF) {
		if (my_CharacterByteType(line, (short)pos, &dFnt) != 0 ||
			  my_CharacterByteType(line, (short)pos+1, &dFnt) != 0 ||
			  my_CharacterByteType(line, (short)pos+2, &dFnt) != 0)
			isSkipFirst = TRUE;
	}
	if (!isSkipFirst) {
		strncpy(templineC2, line+pos, 3);
		templineC2[3] = EOS;
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
			} else if (strcmp(templineC2, "unk|yyy") == 0 ||
					   strcmp(templineC2, "unk|www") == 0)
				return(TRUE);
		}
	}
	return(FALSE);	
}

static char isEqual(char *st, char *pat) {
	if (pat == NULL)
		return(FALSE);
	if (!uS.mStricmp(st, pat))
		return(TRUE);
	else
		return(FALSE);
}

static char isnEqual(char *st, char *pat, int len) {
	if (pat == NULL)
		return(FALSE);
	if (!uS.mStrnicmp(st, pat, len))
		return(TRUE);
	else
		return(FALSE);
}

static char isSuffix(char *st, MORFEATS *feat) {
	if (feat->suffix0 != NULL) {
		if (!uS.mStricmp(st, feat->suffix0))
			return(TRUE);
	}
	if (feat->suffix1 != NULL) {
		if (!uS.mStricmp(st, feat->suffix1))
			return(TRUE);
	}
	if (feat->suffix2 != NULL) {
		if (!uS.mStricmp(st, feat->suffix2))
			return(TRUE);
	}
	return(FALSE);
}

static char isFusion(char *st, MORFEATS *feat) {
	if (feat->fusion0 != NULL) {
		if (!uS.mStricmp(st, feat->fusion0))
			return(TRUE);
	}
	if (feat->fusion1 != NULL) {
		if (!uS.mStricmp(st, feat->fusion1))
			return(TRUE);
	}
	if (feat->fusion2 != NULL) {
		if (!uS.mStricmp(st, feat->fusion2))
			return(TRUE);
	}
	return(FALSE);
}

static char isAllv(MORFEATS *feat) {
	if (isEqual("v", feat->pos) || 
		(isEqual("part",feat->pos) && (isSuffix("PERF",feat) || isSuffix("PROG",feat) || isFusion("PERF",feat) || isFusion("PROG",feat))))
		return(TRUE);
	return(FALSE);
}

static void measures_process_tier(struct speaker *ts, struct database *db, char *rightIDFound) {
	int i;
	char word[1024], tword[1024];
	char tmp, isWordsFound, isWordsFound100, sq, aq, isSkip;
	char isPSDFound, curPSDFound, isXXXItem, isAuxFound, isAmbigFound;
	long stime, etime, duration;
	float words, morf, mUtt, words100, morf100, mUtt100;
	struct IDparts IDTier;
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
				ts->words_root = measures_tree(ts->words_root, word, ts, db);
			}
		}
		while ((i=getword(utterance->speaker, utterance->line, word, NULL, i))) {
			if (!strcmp(word, "[//]"))
				ts->retr++;
			else if (!strcmp(word, "[/]"))
				ts->repet++;
		}
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (ts == NULL)
			return;
		filterwords("%", uttline, excludePlusAndUttDels);
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
			if (measures_mlu_excludeUtter(spareTier1, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
				isSkip = TRUE;
				break;
			}
		}
		i = 0;
		do {
			if (measures_mlu_excludeUtter(uttline, i, &isWordsFound, &isXXXItem)) { // xxx, yyy, www
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
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,'.',&dFnt, MBF)) {
				if (uS.isskip(utterance->line,i+2,&dFnt,MBF) && !uS.IsUtteranceDel(utterance->line,i+2)) {
					if (mUtt100 > 0.0)
						mUtt100--;
					else if (mUtt100 == 0.0) 
						isWordsFound100 = FALSE;
					if (mUtt > 0.0)
						mUtt--;
					else if (mUtt == 0.0) 
						isWordsFound = FALSE;
				}
			}
			if (utterance->line[i] == '+') {
				if (utterance->line[i+1] == '/') {
					if (utterance->line[i+2] == '.' && uS.isskip(utterance->line,i+3,&dFnt,MBF))
						curPSDFound = TRUE;
					else if (utterance->line[i+2] == '/' && utterance->line[i+3] == '.' && uS.isskip(utterance->line,i+4,&dFnt,MBF))
						curPSDFound = TRUE;
					else if (utterance->line[i+2] == '?' && uS.isskip(utterance->line,i+3,&dFnt,MBF))
						curPSDFound = TRUE;
					else if (utterance->line[i+2] == '/' && utterance->line[i+3] == '?' && uS.isskip(utterance->line,i+4,&dFnt,MBF))
						curPSDFound = TRUE;
				} else if (utterance->line[i+1] == '.') {
					if (utterance->line[i+2] == '.' && utterance->line[i+3] == '.' && uS.isskip(utterance->line,i+4,&dFnt,MBF))
						curPSDFound = TRUE;
					else if (utterance->line[i+2] == '.' && utterance->line[i+3] == '?' && uS.isskip(utterance->line,i+4,&dFnt,MBF))
						curPSDFound = TRUE;
				}
			}
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
			i = 0;
			while ((i=getword("*", spareTier1, word, NULL, i))) {
				if (!strcmp(word, "xxx") || !strncmp(word, "xxx@s", 5)) {
					if (!isXXXItem)
						ts->xxxut++;
					isXXXItem = TRUE;
				}
				if (uS.isSqCodes(word, tword, &dFnt, FALSE)) {
					if (tword[0] == '[' && tword[1]== '*' && (tword[2] == ']' || tword[2] == ' ')) {
						ts->werr++;
					}
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
				if (ParseWordIntoFeatures(templineC2, &word_feats) == FALSE)
					measures_overflow(db, TRUE);
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
				measures_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
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

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i') 
		return((measures_group == FALSE && found) || (measures_SpecWords == found));
	else 
		return((measures_group == TRUE && measures_SpecWords > found) || (found == 0));
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
		strcpy(measures_BBS, "@*&#");
		strcpy(measures_CBS, "@*&#");
	}
	if (measures_SpecWords) {
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
					if (measures_SpecWords)
						fprintf(stderr,"\nERROR: No specified gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					measures_overflow(db, FALSE);
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
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(measures_BBS, "@*&#");
				strcpy(measures_CBS, "@*&#");
			}
			if (measures_SpecWords) {
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
			if (measures_SpecWords && !strcmp(measures_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					measures_n_option = FALSE;
					strcpy(measures_BBS, "@BG:");
					strcpy(measures_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					measures_n_option = TRUE;
					strcpy(measures_BBS, "@G:");
					strcpy(measures_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,measures_BBS,FALSE,FALSE)) {
				if (measures_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,measures_CBS,FALSE,FALSE)) {
				if (measures_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (measures_SpecWords)
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
					ts = measures_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
					if (ts != NULL && isfirst) {
						isfirst = FALSE;
						db->num = db->num + 1.0;
						t = db->num;
						if (t == 1 || t % 15 == 0)
							fprintf(stderr,"\r%.0f ", db->num);
					}
				}
				measures_process_tier(ts, db, NULL);
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
		measures_overflow(db, FALSE);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (measures_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		measures_overflow(db, FALSE);
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

static void measures_pr_result(void) {
	int    i, cnt, SDn;
	char   *sFNane;
	float  num;
	struct speaker *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;

	if (sp_head == NULL) {
		if (measures_SpecWords && !onlyApplyToDB) {
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
	fprintf(fpout,"\tLanguage\tCorpus\tCode\tAge\tSex\tGroup\tSES\tRole\tEducation\tCustom_field");
	fprintf(fpout,"\tTotal Utts");
	fprintf(fpout,"\tMLU Utts\tMLU Words\tMLU Morphemes");
	fprintf(fpout,"\tMLU100 Utts\tMLU100 Words\tMLU100 Morphemes");
	fprintf(fpout,"\ttypes\ttokens\tTTR\tClause/Utt");
	fprintf(fpout,"\tDur Words\tDur Utts\tDur Time (secs)\tWords/Time\tUtts/Time");
	fprintf(fpout,"\tWord Errors\tUtt Errors");
	fprintf(fpout,"\tretracing[//]\trepetition[/]");
	fprintf(fpout, "\n");
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (measures_SpecWords) {
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
			fprintf(stderr,"\nERROR: No %%mor: tier found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, sFNane);
			fprintf(fpout,"\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n");
			continue;
		}
		if (isXXXFound)
			ts->tUtt = ts->mUtt;
		else
			ts->tUtt = ts->xxxut + ts->mUtt;
		fprintf(fpout, "\t%.0f",ts->tUtt);
		fprintf(fpout, "\t%.0f",ts->mUtt);
		if (ts->mUtt == 0.0)
			fprintf(fpout, "\tN/A\tN/A");
		else
			fprintf(fpout, "\t%.3f\t%.3f",ts->words/ts->mUtt,ts->morf/ts->mUtt);
		fprintf(fpout, "\t%.0f",ts->mUtt100);
		if (ts->mUtt100 == 0.0)
			fprintf(fpout, "\tN/A\tN/A");
		else
			fprintf(fpout, "\t%.3f\t%.3f",ts->words100/ts->mUtt100,ts->morf100/ts->mUtt100);
		fprintf(fpout, "\t%.0f\t%.0f",ts->diff,ts->total);
		if (ts->total == 0.0)
			fprintf(fpout,"\tN/A");
		else
			fprintf(fpout,"\t%.3f",ts->diff/ts->total);
		if (ts->tUtt == 0.0)
			fprintf(fpout,"\tN/A");
		else
			fprintf(fpout,"\t%.3f",ts->clause/ts->tUtt);
		fprintf(fpout, "\t%.0f\t%.0f",ts->tdWords,ts->tdUtts);
		fprintf(fpout,"\t%.0f", ts->tdur);
		if (ts->tdur == 0.0)
			fprintf(fpout, "\tN/A\tN/A");
		else
			fprintf(fpout, "\t%.3f\t%.3f",ts->tdWords/ts->tdur,ts->tdUtts/ts->tdur);
		fprintf(fpout, "\t%.0f\t%.0f",ts->werr,ts->uerr);
		fprintf(fpout, "\t%.0f\t%.0f",ts->retr,ts->repet);
		fprintf(fpout, "\n");
		if (DBKeyRoot != NULL) {
			num = db->num;
			fprintf(fpout,"+/-SD\t");
			SDn = 0; // this number should be less than SDRESSIZE = 256
			compute_SD(&SD[SDn++], ts->tUtt,  NULL, db->tUtt_sqr, db->tUtt, num);
			compute_SD(&SD[SDn++], ts->mUtt,  NULL, db->mUtt_sqr, db->mUtt, num);
			compute_SD(&SD[SDn++], ts->words,  &ts->mUtt, db->words_sqr, db->words, num);
			compute_SD(&SD[SDn++], ts->morf,  &ts->mUtt, db->morf_sqr, db->morf, num);
			compute_SD(&SD[SDn++], ts->mUtt100,  NULL, db->mUtt100_sqr, db->mUtt100, num);
			compute_SD(&SD[SDn++], ts->words100,  &ts->mUtt100, db->words100_sqr, db->words100, num);
			compute_SD(&SD[SDn++], ts->morf100,  &ts->mUtt100, db->morf100_sqr, db->morf100, num);
			compute_SD(&SD[SDn++], ts->diff,  NULL, db->diff_sqr, db->diff, num);
			compute_SD(&SD[SDn++], ts->total,  NULL, db->total_sqr, db->total, num);
			compute_SD(&SD[SDn++], ts->diff,  &ts->total, db->TTR_sqr, db->TTR, num);
			compute_SD(&SD[SDn++], ts->clause,  &ts->tUtt, db->CUR_sqr, db->CUR, num);
			compute_SD(&SD[SDn++], ts->tdWords,  NULL, db->tdWords_sqr, db->tdWords, num);
			compute_SD(&SD[SDn++], ts->tdUtts,  NULL, db->tdUtts_sqr, db->tdUtts, num);
			compute_SD(&SD[SDn++], ts->tdur,  NULL, db->tdur_sqr, db->tdur, num);
			compute_SD(&SD[SDn++], ts->tdWords,  &ts->tdur, db->tdWT_sqr, db->tdWT, num);
			compute_SD(&SD[SDn++], ts->tdUtts,  &ts->tdur, db->tdUT_sqr, db->tdUT, num);
			compute_SD(&SD[SDn++], ts->werr,  NULL, db->werr_sqr, db->werr, num);
			compute_SD(&SD[SDn++], ts->uerr,  NULL, db->uerr_sqr, db->uerr, num);
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
		fprintf(fpout, "\n");

		fprintf(fpout,"Min Database\t");
		fprintf(fpout, "\t%.0f",db->mn_tUtt);
		fprintf(fpout, "\t%.0f",db->mn_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mn_words,db->mn_morf);
		fprintf(fpout, "\t%.0f",db->mn_mUtt100);
		fprintf(fpout, "\t%.3f\t%.3f",db->mn_words100,db->mn_morf100);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_diff,db->mn_total);
		fprintf(fpout, "\t%.3f",db->mn_TTR);
		fprintf(fpout, "\t%.3f",db->mn_CUR);
		fprintf(fpout,"\t%.3f\t%.3f\t%.0f\t%.3f\t%.3f",db->mn_tdWords,db->mn_tdUtts,db->mn_tdur,db->mn_tdWT,db->mn_tdUT);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_werr,db->mn_uerr);
		fprintf(fpout, "\t%.0f\t%.0f",db->mn_retr,db->mn_repet);
		fprintf(fpout, "\n");

		fprintf(fpout,"Max Database\t");
		fprintf(fpout, "\t%.0f",db->mx_tUtt);
		fprintf(fpout, "\t%.0f",db->mx_mUtt);
		fprintf(fpout, "\t%.3f\t%.3f",db->mx_words,db->mx_morf);
		fprintf(fpout, "\t%.0f",db->mx_mUtt100);
		fprintf(fpout, "\t%.3f\t%.3f",db->mx_words100,db->mx_morf100);
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_diff,db->mx_total);
		fprintf(fpout, "\t%.3f",db->mx_TTR);
		fprintf(fpout, "\t%.3f",db->mx_CUR);
		fprintf(fpout,"\t%.3f\t%.3f\t%.0f\t%.3f\t%.3f",db->mx_tdWords,db->mx_tdUtts,db->mx_tdur,db->mx_tdWT,db->mx_tdUT);
		fprintf(fpout, "\t%.0f\t%.0f",db->mx_werr,db->mx_uerr);
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

void call() {
	int  found;
	long tlineno = 0;
	char lRightspeaker;
	char isOutputGem;
	char word[BUFSIZ];
	struct speaker *ts;

	fprintf(stderr,"From file <%s>\n",oldfname);
	ts = NULL;
	if (isGOptionSet == FALSE)
		onlyApplyToDB = !isFoundGems();
	if (!onlyApplyToDB && isNOptionSet == FALSE) {
		strcpy(measures_BBS, "@*&#");
		strcpy(measures_CBS, "@*&#");
	}
	if (measures_SpecWords && !onlyApplyToDB) {
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
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*')
				lRightspeaker = TRUE;
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!onlyApplyToDB && measures_SpecWords && !strcmp(measures_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				measures_n_option = FALSE;
				strcpy(measures_BBS, "@BG:");
				strcpy(measures_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				measures_n_option = TRUE;
				strcpy(measures_BBS, "@G:");
				strcpy(measures_CBS, "@*&#");
			}
		}
		if (!onlyApplyToDB && uS.partcmp(utterance->speaker,measures_BBS,FALSE,FALSE)) {
			if (measures_n_option) {
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
		} else if (found > 0 && uS.partcmp(utterance->speaker,measures_CBS,FALSE,FALSE)) {
			if (measures_n_option) {
			} else {
				if (isRightText(word)) {
					found--;
					if (found == 0) {
						if (measures_SpecWords)
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
				ts = measures_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
			}
			measures_process_tier(ts, NULL, NULL);
		}
	}
	if (!combinput)
		measures_pr_result();
}

void init(char first) {
	int  i;
	char *f;

	if (first) {
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
//		addword('\0','\0',"+unk|xxx");
//		addword('\0','\0',"+unk|yyy");
//		addword('\0','\0',"+*|www");
		mor_initwords();
		isDebug = FALSE;
		isXXXFound = FALSE;
		GemMode = '\0';
		measures_SpecWords = 0;
		measures_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		measures_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(measures_BBS, "@*&#");
		strcpy(measures_CBS, "@*&#");
		isSpeakerNameGiven = FALSE;
		sp_head = NULL;
		db = NULL;
		DBKeyRoot = NULL;
		DBGemNum = 0;
		combinput = TRUE;
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
				measures_overflow(NULL, FALSE);
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
				measures_overflow(NULL, FALSE);
			}
			maketierchoice("*:", '+', '\001');
			if (DBKeyRoot != NULL) {
				db = NEW(struct database);
				if (db == NULL)
					measures_overflow(db, TRUE);
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
	targv = argv;
	targc = argc;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MEASURES;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,measures_pr_result);
	db = freedb(db);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
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
				addDBKeys(f);
			}
			break;
		case 'g':
			if (*f == EOS) {
				if (*(f-2) == '+')
					measures_group = TRUE;
				else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				measures_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'n':
			if (*(f-2) == '+') {
				measures_n_option = TRUE;
				strcpy(measures_BBS, "@G:");
				strcpy(measures_CBS, "@*&#");
			} else {
				strcpy(measures_BBS, "@BG:");
				strcpy(measures_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
			break;
		case 's':
			if (*(f-2) == '+')
				isXXXFound = TRUE;
			else {
				fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
				fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
				measures_overflow(NULL, FALSE);
			}
			break;
		case 't':
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
