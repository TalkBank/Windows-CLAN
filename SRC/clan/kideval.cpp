/**********************************************************************
 "Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
*/

//  kideval -c1 '+t*CHI'
// /Users/WORK/clanW/src/unix/bin/kideval -c1 '+t*CHI' -L/Users/WORK/clanW/src/unix/bin/kideval_lib
// /Users/WORK/clanW/src/unix/bin/kideval '+t*CHI' -re -leng -L/Users/WORK/clanW/src/unix/bin/kideval_lib *.cha

/*
 isCreateDB

 @ID   = +
 # gem results and unique words
 @G    = G
 @BG   = B
 @EG   = E
 @END  = -

 kideval_initTSVars
 prTSResults - write DB
 retrieveTS  - read DB
 wID
	wnum;
	wused;
 addFilePath

 KIDEVAL_DB_ID
 result = DoKidevalDB(theItem);
 KidevalDBMenu
 KIDEVAL_DB_item
*/

//#define DEBUGEVALRESULTS

#define KIDEVAL_DB_VERSION 7

#define DATABASE_FILE_NAME "_db.cut"
#define DATABSEFILESLIST "0kideval_Database_IDs.cex"
#define DATABASEFILESDEBUG "0kideval_DB_results.txt"


#define CHAT_MODE 1
#include "cu.h"
#include "c_curses.h"
#include "dss.h"
#include "ipsyn.h"
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

extern char OverWriteFile;
extern char outputOnlyData;
extern char GExt[];
extern char isRecursive;
extern char isLanguageExplicit;
extern char PostCodeMode;
extern struct tier *defheadtier;
extern struct tier *headtier;
extern struct IDtype *SPRole;
extern FNType prefsDir[];

#define TYPESLEN 32
#define SDRESSIZE 256
#define NUMCOMPS 128

enum {
	BASE_COM = 0,
	DSS_COM,
	VOCD_COM,
	IPSYN_COM,
	LAST_COMMAND
} ;

struct label_cols {
	char *label;
	int  num;
	struct label_cols *next_label;
};

#define PATS struct pats_pair
PATS {
	MORWDLST *pats1;
	MORWDLST *pats2;
	PATS *next_pats;
};

#define COLS struct all_cols
COLS {
	PATS *pats_elem;
	struct label_cols *labelP;
	char isClitic;
	COLS *next_col;
};

struct kideval_words {
	char *word;
	int  wnum;
	char wused;
	struct kideval_words *left;
	struct kideval_words *right;
};

struct kideval_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;
	int  wID;
	char *Design, *Activity, *Group;
	float tUtt;
	float frTokens;	// total number of words
	float frTypes;	// number of diff words
	float NDW, NDWTotal;	// number of diff words
	float CUR;
	float mWords, morf, mUtt;
	float mWords50, morf50, mUtt50;
	float tdWords, tdUtts, tdur;
	float werr, uerr;
	float retr, repet;
	DSSSP *dss_sp;
	float dssUttCnt;
	float dssGTotal;
	VOCDSP *vocd_sp;
	float vocdOptimum;
	IPSYN_SP *ipsyn_sp;
	float ipsynUttCnt;
	float ipsynScore;
	float morTotal;	// total number of words on %mor tier
	struct kideval_words *words_root;
	struct kideval_words *NDW_root;
	float *mor_count;
	struct kideval_speakers *next_sp;
} ;

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct database {
	float tUtt_sqr, tUtt, mn_tUtt, mx_tUtt, tUtt_num;
	float frTokens_sqr, frTokens, mn_frTokens, mx_frTokens, frTokens_num;	/* total number of words */
	float frTypes_sqr, frTypes, mn_frTypes, mx_frTypes, frTypes_num;	/* number of diff words */
	float NDW_sqr, NDW, mn_NDW, mx_NDW, NDW_num;	/* number of diff words */
	float TTR_sqr, TTR, mn_TTR, mx_TTR, TTR_num;
	float CUR_sqr, CUR, mn_CUR, mx_CUR, CUR_num;
	float mUtt_sqr, mUtt, mn_mUtt, mx_mUtt, mUtt_num;
	float mWords_sqr, mWords, mn_mWords, mx_mWords, mWords_num;
	float morf_sqr, morf, mn_morf, mx_morf, morf_num;
	float mUtt50_sqr, mUtt50, mn_mUtt50, mx_mUtt50, mUtt50_num;
	float mWords50_sqr, mWords50, mn_mWords50, mx_mWords50, mWords50_num;
	float morf50_sqr, morf50, mn_morf50, mx_morf50, morf50_num;
	float tdWords_sqr, tdWords, mn_tdWords, mx_tdWords, tdWords_num;
	float tdUtts_sqr, tdUtts, mn_tdUtts, mx_tdUtts, tdUtts_num;
	float tdur_sqr, tdur, mn_tdur, mx_tdur, tdur_num;
	float tdWT_sqr, tdWT, mn_tdWT, mx_tdWT, tdWT_num;
	float tdUT_sqr, tdUT, mn_tdUT, mx_tdUT, tdUT_num;
	float werr_sqr, werr, mn_werr, mx_werr, werr_num;
	float uerr_sqr, uerr, mn_uerr, mx_uerr, uerr_num;
	float retr_sqr, retr, mn_retr, mx_retr, retr_num;
	float repet_sqr, repet, mn_repet, mx_repet, repet_num;
	float dssUttCnt_sqr, dssUttCnt, mn_dssUttCnt, mx_dssUttCnt, dssUttCnt_num;
	float dssGTotal_sqr, dssGTotal, mn_dssGTotal, mx_dssGTotal, dssGTotal_num;
	float vocdOptimum_sqr, vocdOptimum, mn_vocdOptimum, mx_vocdOptimum, vocdOptimum_num;
	float ipsynUttCnt_sqr, ipsynUttCnt, mn_ipsynUttCnt, mx_ipsynUttCnt, ipsynUttCnt_num;
	float ipsynScore_sqr, ipsynScore, mn_ipsynScore, mx_ipsynScore, ipsynScore_num;
	float morTotal_sqr, morTotal, mn_morTotal, mx_morTotal, morTotal_num;	/* total number of words on %mor tier */
	float *morItems_sqr, *morItems, morItems_num;
	struct kideval_speakers *db_sp;
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
static int  MinUttLimit, DssIpsynUttLimit, wdOffset;
static int  colLabelsNum;
static int  gMisalignmentError;
static float DB_UTT_NUM_LIMIT;
static long agesFound[13], dummyFiles;
static char *ke_script_file;
static char DB_version[65], DB_type[128], DB_MIN_UTTS[16+1];
static char ftime, isDBFilesList, isCreateDB, isRawVal, isLinkAge;
static char ke_Design[TYPESLEN+1], ke_Activity[TYPESLEN+1], ke_Group[TYPESLEN+1];
static FILE	*dbfpout;
static FILE *DBFilesListFP;
static struct DBKeys *DBKeyRoot;
static struct kideval_speakers *sp_head;
static struct database *gdb;
static COLS *colsRoot;
static struct label_cols *labelsRoot;
#ifdef DEBUGEVALRESULTS
FILE *dbResults;
#endif

void usage() {
	printf("Usage: kideval [aN bS dS eN g gS lF n oN s %s] filename(s)\n", mainflgs());
	puts("+aN: limit analysis to files containing more than N utterances (default: no minimal limit)");
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(_CLAN_DEBUG)
	puts("    to create database file set \"working\" to: ~/SERVERS/data");
	puts("+c : kideval +leng +c_toyplay");
	puts("+c : kideval +leng +c_narrative");
	puts("+c : kideval +lengu +c_toyplay");
	puts("+c : kideval +lengu +c_narrative");
	puts("+c : kideval +lfra +c_toyplay");
	puts("+c : kideval +lfra +c25_narrative");
	puts("+c : kideval +lnld +c_toyplay");
	puts("+c : kideval +lspa +c_toyplay");
	puts("+c : kideval +lspa +c25_narrative");
	puts("+c : kideval +lzho +c_toyplay");
	puts("+c : kideval +lzho +c25_narrative");
	puts("+c : kideval +ljpn +c_toyplay");
#endif // _CLAN_DEBUG
	puts("+dtd~S: specify database keyword(s) S. For example:");
	puts("    +dtoyplay~\"2;7-3;\" for children of age 31 to 36 months old.");
	puts("    +dnarrative~\"2;-2;6|female\" for females children of age 24 to 30 months old.");
	puts("+dmd~S: specify database keyword(s) S. For example:");
	puts("    +dmd~\"2;7-3;\" for children of age 31 to 36 months old.");
	puts("    +dmd~\"2;-2;6|female\" for females children of age 24 to 30 months old.");
	puts("+e1: create list of database files used for comparisons");
	puts("+lF: specify language script file name F (default: eng)");
	puts("     choices: eng, engu, fra, jpn, nld, spa, yue, zho");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+o4: output percentage values instead of raw values");
	puts("+qN: change the minimum limit of utterance for DSS and IPSYN (default: DSS=50, IPSYN=50)");
	puts("+s : counts utterances and other words found along with \"xxx\"");
	mainusage(FALSE);
#ifdef UNX
	printf("PLEASE SEE SLP MANUAL FOR RECOMMENDED INTERPRETATION.\n\n");
#else
	printf("%c%cPLEASE SEE SLP MANUAL FOR RECOMMENDED INTERPRETATION.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	puts("IF NO \"+t*...\" option specified, then speaker defaults to \"Target_Child\"");
	puts("IF NO \"+l*...\" option specified, then language defaults to \"eng\"");
	puts("Examples:");
	puts("   Search database for female children between ages of 24 and 30 months");
	puts("       kideval -leng +dtoyplay~\"2;-2;6|female\" *.cha");
	puts("       kideval -lzho +dmd~\"2;-2;6|female\" *.cha");
	puts("   Search database for children between ages of 31 and 36 months");
	puts("       kideval -ljpn +dtd~\"2;7-3;\" *.cha ");
	puts("       kideval -lzho +dmd~\"2;7-3;\" *.cha ");
	puts("   Compute results only for English speaker *CHI");
	puts("       kideval *.cha");
	puts("   Compute results only for Spanish speaker *PAR");
	puts("       kideval +t*par +lspa *.cha");
	cutt_exit(0);
}

static void init_db(struct database *p) {
	int i;

	p->tUtt_sqr = 0.0; p->tUtt = 0.0; p->mn_tUtt = 0.0; p->mx_tUtt = 0.0; p->tUtt_num = 0.0;
	p->frTokens_sqr = 0.0; p->frTokens = 0.0; p->mn_frTokens = 0.0; p->mx_frTokens = 0.0; p->frTokens_num = 0.0;
	p->frTypes_sqr = 0.0; p->frTypes = 0.0; p->mn_frTypes = 0.0; p->mx_frTypes = 0.0; p->frTypes_num = 0.0;
	p->NDW_sqr = 0.0; p->NDW = 0.0; p->mn_NDW = 0.0; p->mx_NDW = 0.0; p->NDW_num = 0.0;
	p->TTR_sqr = 0.0; p->TTR = 0.0; p->mn_TTR = 0.0; p->mx_TTR = 0.0; p->TTR_num = 0.0;
	p->CUR_sqr = 0.0; p->CUR = 0.0; p->mn_CUR = 0.0; p->mx_CUR = 0.0; p->CUR_num = 0.0;
	p->mUtt_sqr = 0.0; p->mUtt = 0.0; p->mn_mUtt = 0.0; p->mx_mUtt = 0.0; p->mUtt_num = 0.0;
	p->mWords_sqr = 0.0; p->mWords = 0.0; p->mn_mWords = 0.0; p->mx_mWords = 0.0; p->mWords_num = 0.0;
	p->morf_sqr = 0.0; p->morf = 0.0; p->mn_morf = 0.0; p->mx_morf = 0.0; p->morf_num = 0.0;
	p->mUtt50_sqr = 0.0; p->mUtt50 = 0.0; p->mn_mUtt50 = 0.0; p->mx_mUtt50 = 0.0; p->mUtt50_num = 0.0;
	p->mWords50_sqr = 0.0; p->mWords50 = 0.0; p->mn_mWords50 = 0.0; p->mx_mWords50 = 0.0;  p->mWords50_num = 0.0;
	p->morf50_sqr = 0.0; p->morf50 = 0.0; p->mn_morf50 = 0.0; p->mx_morf50 = 0.0; p->morf50_num = 0.0;
	p->tdWords_sqr = 0.0; p->tdWords = 0.0; p->mn_tdWords = 0.0; p->mx_tdWords = 0.0; p->tdWords_num = 0.0;
	p->tdUtts_sqr = 0.0; p->tdUtts = 0.0; p->mn_tdUtts = 0.0; p->mx_tdUtts = 0.0; p->tdUtts_num = 0.0;
	p->tdur_sqr = 0.0; p->tdur = 0.0; p->mn_tdur = 0.0; p->mx_tdur = 0.0; p->tdur_num = 0.0;
	p->tdWT_sqr = 0.0; p->tdWT = 0.0; p->mn_tdWT = 0.0; p->mx_tdWT = 0.0; p->tdWT_num = 0.0;
	p->tdUT_sqr = 0.0; p->tdUT = 0.0; p->mn_tdUT = 0.0; p->mx_tdUT = 0.0; p->tdUT_num = 0.0;
	p->werr_sqr = 0.0; p->werr = 0.0; p->mn_werr = 0.0; p->mx_werr = 0.0; p->werr_num = 0.0;
	p->uerr_sqr = 0.0; p->uerr = 0.0; p->mn_uerr = 0.0; p->mx_uerr = 0.0; p->uerr_num = 0.0;
	p->retr_sqr = 0.0; p->retr = 0.0; p->mn_retr = 0.0; p->mx_retr = 0.0; p->retr_num = 0.0;
	p->repet_sqr = 0.0; p->repet = 0.0; p->mn_repet = 0.0; p->mx_repet = 0.0; p->repet_num = 0.0;
	p->dssGTotal_sqr = 0.0; p->dssGTotal = 0.0; p->mn_dssGTotal = 0.0; p->mx_dssGTotal = 0.0; p->dssGTotal_num = 0.0;
	p->dssUttCnt_sqr = 0.0; p->dssUttCnt = 0.0; p->mn_dssUttCnt = 0.0; p->mx_dssUttCnt = 0.0; p->dssUttCnt_num = 0.0;
	p->vocdOptimum_sqr = 0.0; p->vocdOptimum = 0.0; p->mn_vocdOptimum = 0.0; p->mx_vocdOptimum = 0.0; p->vocdOptimum_num = 0.0;
	p->ipsynUttCnt_sqr = 0.0; p->ipsynUttCnt = 0.0; p->mn_ipsynUttCnt = 0.0; p->mx_ipsynUttCnt = 0.0; p->ipsynUttCnt_num = 0.0;
	p->ipsynScore_sqr = 0.0; p->ipsynScore = 0.0; p->mn_ipsynScore = 0.0; p->mx_ipsynScore = 0.0; p->ipsynScore_num = 0.0;
	p->morTotal_sqr = 0.0; p->morTotal = 0.0; p->mn_morTotal = 0.0; p->mx_morTotal = 0.0; p->morTotal_num = 0.0;

	for (i=0; i < colLabelsNum; i++) {
		p->morItems_sqr[i] = 0.0; p->morItems[i] = 0.0;
	}
	gdb->morItems_num = 0.0;

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

static PATS *freePatsElems(PATS *p) {
	PATS *t;

	while (p != NULL) {
		t = p;
		p = p->next_pats;
		t->pats1 = freeMorWords(t->pats1);
		if (t->pats2 != NULL)
			t->pats2 = freeMorWords(t->pats2);
		free(t);
	}
	return(NULL);
}

static COLS *freeColsP(COLS *p) {
	COLS *t;

	while (p != NULL) {
		t = p;
		p = p->next_col;
		t->pats_elem = freePatsElems(t->pats_elem);
		free(t);
	}
	return(NULL);
}

static struct kideval_words *kideval_freetree(struct kideval_words *p) {
	if (p != NULL) {
		kideval_freetree(p->left);
		kideval_freetree(p->right);
		if (p->word != NULL)
			free(p->word);
		free(p);
	}
	return(NULL);
}

static struct kideval_speakers *freespeakers(struct kideval_speakers *p) {
	struct kideval_speakers *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
		if (ts->Design != NULL)
			free(ts->Design);
		if (ts->Activity != NULL)
			free(ts->Activity);
		if (ts->Group != NULL)
			free(ts->Group);
		if (ts->words_root != NULL)
			kideval_freetree(ts->words_root);
		if (ts->NDW_root != NULL)
			kideval_freetree(ts->NDW_root);
		if (ts->mor_count != NULL)
			free(ts->mor_count);
		free(ts);
	}
	return(NULL);
}

static struct database *freedb(struct database *db) {
	if (db != NULL) {
		db->db_sp = freespeakers(db->db_sp);
		if (db->morItems_sqr != NULL)
			free(db->morItems_sqr);
		if (db->morItems != NULL)
			free(db->morItems);
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

static void kideval_error(struct database *tdb, char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	labelsRoot = freeLabels(labelsRoot);
	colsRoot = freeColsP(colsRoot);
	if (tdb != NULL)
		tdb = freedb(tdb);
	else
		gdb = freedb(gdb);
	dss_freeSpeakers();
	dss_cleanSearchMem();
	ipsyn_freeSpeakers();
	ipsyn_cleanSearchMem();
	speakers = speaker_free_up_speakers(speakers, TRUE);
	if (VOCD_TYPE_D)
		denom_speakers = speaker_free_up_speakers(denom_speakers, TRUE);
	cutt_exit(0);
}

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

static void addDBKeys(const char *key) {
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
			if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = kideval_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					kideval_error(NULL, FALSE);
				}
				DBKey->age = kideval_strsave(b);
			} else {
				if (DBKey->key1 == NULL)
					DBKey->key1 = kideval_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = kideval_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = kideval_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = kideval_strsave(b);
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
	}
}

static int isEqualKey(char *DBk, char *IDk) {
	if (DBk == NULL || IDk == NULL) {
		return(1);
	}
	return(uS.mStricmp(IDk, DBk));
}

static void changeAgeField(char *fr, char *to) {
	int i, j, k, cnt, years, months, days;
	char age[BUFSIZ], *s;

	j = 0;
	cnt = 0;
	for (i=0; cnt < 3 && fr[i] != EOS; i++) {
		to[j++] = fr[i];
		if (fr[i] == '|')
			cnt++;
	}
	if (cnt == 3 && fr[i] != EOS) {
		k = 0;
		for (; fr[i] != '|' && fr[i] != EOS; i++) {
			age[k++] = fr[i];
		}
		age[k] = EOS;
		s = strchr(age, ';');
		if (s != NULL) {
			years = atoi(age);
			strcpy(age, s+1);
		} else
			years = 0;
		s = strchr(age, '.');
		if (s != NULL) {
			months = atoi(age);
			strcpy(age, s+1);
			days = atoi(age);
		} else {
			months = 0;
			days = 0;
		}
		cnt = years * 12;
		cnt = cnt + months;
		if (days >= 15)
			cnt++;
		sprintf(to+j, "%d", cnt);
		k = j;
		j = j + strlen(to+k);
	}
	for (; fr[i] != EOS; i++)
		to[j++] = fr[i];
	to[j] = EOS;
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

static void kideval_resettree(struct kideval_words *p) {
	if (p != NULL) {
		kideval_resettree(p->left);
		p->wused = FALSE;
		kideval_resettree(p->right);
	}
}

static void kideval_initTSVars(struct kideval_speakers *ts, char isAll) {
	int i;

	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;
	ts->tUtt	= 0.0;
	ts->frTokens= 0.0;
	ts->frTypes = 0.0;
	ts->NDW		= 0.0;
	ts->NDWTotal= 0.0;
	ts->vocd_sp  = NULL;
	ts->vocdOptimum = 0.0;
	ts->CUR		= 0.0;
	ts->mWords  = 0.0;
	ts->morf	= 0.0;
	ts->mUtt	= 0.0;
	ts->mWords50= 0.0;
	ts->morf50	= 0.0;
	ts->mUtt50	= 0.0;
	ts->tdWords	= 0.0;
	ts->tdUtts	= 0.0;
	ts->tdur	= 0.0;
	ts->werr    = 0.0;
	ts->uerr    = 0.0;
	ts->retr    = 0.0;
	ts->repet   = 0.0;
	ts->dss_sp  = NULL;
	ts->dssUttCnt= 0.0;
	ts->dssGTotal= 0.0;
	ts->ipsyn_sp = NULL;
	ts->ipsynUttCnt = 0.0;
	ts->ipsynScore = 0.0;
	ts->morTotal   = 0.0;
	if (colLabelsNum > 0) {
		ts->mor_count = (float *)malloc(sizeof(float) * colLabelsNum);
		if (ts->mor_count == NULL) {
			kideval_error(gdb, TRUE);
		}
		for (i=0; i < colLabelsNum; i++)
			ts->mor_count[i] = 0.0;
	} else
		ts->mor_count = NULL;
	if (isAll) {
		ts->wID = 0;
		ts->words_root = NULL;
		ts->NDW_root   = NULL;
	} else {
		kideval_resettree(ts->words_root);
		kideval_resettree(ts->NDW_root);
	}
}

static struct kideval_speakers *kideval_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	struct kideval_speakers *ts, *tsp;

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
		}
	}
	if ((ts=NEW(struct kideval_speakers)) == NULL)
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
	if (ID == NULL || isCreateDB)
		ts->ID = NULL;
	else {
		changeAgeField(ID, templineC4);
		if ((ts->ID=(char *)malloc(strlen(templineC4)+1)) == NULL)
			kideval_error(db, TRUE);
		strcpy(ts->ID, templineC4);
	}
	ts->Design = NULL;
	ts->Activity = NULL;
	ts->Group = NULL;
	ts->isSpeakerFound = isSpeakerFound;
	kideval_initTSVars(ts, TRUE);
	return(ts);
}

static void filterAndOuputID(char *line, char *IDText) {
	char *s, *sb;

	for (s=line; isSpace(*s); s++) ;
	sb = s;
	if ((s=strchr(sb, '|')) != NULL) {
	} else
		*sb = EOS;
	strcpy(IDText, line);
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
static char kideval_isUttDel(char *line, int pos) {
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

static struct kideval_words *kideval_talloc(char *word, struct database *db, int wnum) {
	struct kideval_words *p;

	if ((p=NEW(struct kideval_words)) == NULL)
		kideval_error(db, TRUE);
	p->left = p->right = NULL;
	p->wnum = wnum;
	p->wused = TRUE;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		kideval_error(db, TRUE);
	}
	return(p);
}

static struct kideval_words *kideval_FREQ_tree(struct kideval_words *p, char *w, struct kideval_speakers *ts, struct database *db) {
	int cond;
	struct kideval_words *t = p;

	if (p == NULL) {
		ts->wID++;
		ts->frTypes++;
		p = kideval_talloc(w, db, ts->wID);
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0) {
		p->left = kideval_FREQ_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0) {
			p->left = kideval_FREQ_tree(p->left, w, ts, db);
		} else {
			p->right = kideval_FREQ_tree(p->right, w, ts, db); /* if cond > 0 */
		}
		return(t);
	}
	return(p);
}

static struct kideval_words *kideval_NDW_tree(struct kideval_words *p, char *w, struct kideval_speakers *ts, struct database *db) {
	int cond;
	struct kideval_words *t = p;

	if (p == NULL) {
		ts->NDW++;
		p = kideval_talloc(w, db, ts->wID);
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0) {
		p->left = kideval_NDW_tree(p->left, w, ts, db);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0) {
			p->left = kideval_NDW_tree(p->left, w, ts, db);
		} else {
			p->right = kideval_NDW_tree(p->right, w, ts, db); /* if cond > 0 */
		}
		return(t);
	}
	return(p);
}

static char isNextWord(int i, char *line,  MORWDLST *pats) {
	char morWord[1024];

	if ((i=getword(utterance->speaker, line, morWord, NULL, i)) == 0)
		return(FALSE);
	if (isWordFromMORTier(morWord)) {
		return(isMorPatMatchedWord(pats, morWord));
	}
	return(FALSE);
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

static float roundFloat(double num) {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}

static void getTypes(char *line) {
	int i, j;

	for (i=0; line[i] != EOS && isSpace(line[i]); i++) ;
	if (line[i] != EOS) {
		j = 0;
		for (; j < TYPESLEN && line[i] != EOS && line[i] != ',' && !isSpace(line[i]); i++) {
			ke_Design[j++] = line[i];
		}
		ke_Design[j] = EOS;
		for (; line[i] != EOS && (line[i] == ',' || isSpace(line[i])); i++) ;
		if (line[i] != EOS) {
			j = 0;
			for (; j < TYPESLEN && line[i] != EOS && line[i] != ',' && !isSpace(line[i]); i++) {
				ke_Activity[j++] = line[i];
			}
			ke_Activity[j] = EOS;
			for (; line[i] != EOS && (line[i] == ',' || isSpace(line[i])); i++) ;
			if (line[i] != EOS) {
				j = 0;
				for (; j < TYPESLEN && line[i] != EOS && line[i] != ',' && !isSpace(line[i]); i++) {
					ke_Group[j++] = line[i];
				}
				ke_Group[j] = EOS;
				for (; line[i] != EOS && line[i] != ',' && !isSpace(line[i]); i++) ;
			}
		}
	}
	uS.remblanks(ke_Design);
	uS.remblanks(ke_Activity);
	uS.remblanks(ke_Group);
}

static void kideval_process_tier(struct kideval_speakers *ts, struct database *db) {
	int i, j, comps, num, oPos;
	char word[BUFSIZ+1], tword[1024], *w[NUMCOMPS], isMatchFound, isMatch2Found;
	char tchr, patsType, isWordsFound, isWordsFound50, sq, aq, isSkip;
	char isPSDFound, curPSDFound, isAuxFound;
	long stime, etime;
	double tNum;
	float mWords, morf, mUtt, mWords50, morf50, mUtt50, morphCnt;
	COLS *col;
	PATS *pats;
	MORFEATS word_feats, *clitic, *feat;

	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return;

		num = isPostCodeFound(utterance->speaker, utterance->line);
		if (PostCodeMode == 'i' && num == 1)
			return;
		if (PostCodeMode == 'e' && num == 5)
			return;
		if (ts->Design == NULL) {
			if ((ts->Design=(char *)malloc(strlen(ke_Design)+1)) != NULL)
				strcpy(ts->Design, ke_Design);
		}
		if (ts->Activity == NULL) {
			if ((ts->Activity=(char *)malloc(strlen(ke_Activity)+1)) != NULL)
				strcpy(ts->Activity, ke_Activity);
		}
		if (ts->Group == NULL) {
			if ((ts->Group=(char *)malloc(strlen(ke_Group)+1)) != NULL)
				strcpy(ts->Group, ke_Group);
		}
		strcpy(spareTier1, utterance->line);
		for (i=0; spareTier1[i] != EOS; i++) {
			if (spareTier1[i] == HIDEN_C && isdigit(spareTier1[i+1])) {
				if (getMediaTagInfo(spareTier1+i, &stime, &etime)) {
					tNum = etime;
					tNum = tNum - stime;
					tNum = tNum / 1000.0000;
//					ts->tdur = ts->tdur + roundFloat(tNum);
					ts->tdur = ts->tdur + tNum;
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
		strcpy(spareTier3, uttline);
		for (i=0; utterance->line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->mUtt50 > 0.0)
					ts->mUtt50--;
				if (ts->mUtt > 0.0)
					ts->mUtt--;
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
				ts->repet++;
		}
	} else if (fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return;
		filterwords("%", uttline, excludePlusAndUttDels);
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			if (isWordFromMORTier(templineC2)) {
				if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
					kideval_error(db, TRUE);
				for (clitic=&word_feats; clitic != NULL; clitic=clitic->clitc) {
					if (clitic->compd != NULL) {
						word[0] = EOS;
						for (feat=clitic; feat != NULL; feat=feat->compd) {
							if (feat->stem != NULL && feat->stem[0] != EOS) {
								strcat(word, feat->stem);
							}
						}
						if (word[0] != EOS) {
							if (ts->NDWTotal < 100) {
								ts->NDWTotal++;
								ts->NDW_root = kideval_NDW_tree(ts->NDW_root, word, ts, db);
							}
							if (exclude(word) && isRightWord(word)) {
								uS.remblanks(word);
								ts->frTokens++;
								ts->words_root = kideval_FREQ_tree(ts->words_root, word, ts, db);
							}
						}
					} else {
						if (clitic->stem != NULL && clitic->stem[0] != EOS) {
							if (ts->NDWTotal < 100) {
								ts->NDWTotal++;
								ts->NDW_root = kideval_NDW_tree(ts->NDW_root, clitic->stem, ts, db);
							}
							if (exclude(clitic->stem) && isRightWord(clitic->stem)) {
								uS.remblanks(clitic->stem);
								ts->frTokens++;
								ts->words_root = kideval_FREQ_tree(ts->words_root, clitic->stem, ts, db);
							}
						}
					}
				}
				freeUpFeats(&word_feats);
			}
		}
		i = 0;
		while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
			uS.remblanks(word);
			if (isWordFromMORTier(word)) {
				ts->morTotal++;
				strcpy(tword, word);
				for (j=0; tword[j] != EOS; j++) {
					if (tword[j] == '~' || tword[j] == '$')
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
//2021-03-24 if (strcmp(col->labelP->label, "irr-3S") == 0)
//num = 0;
						if (col->isClitic) {
							for (pats=col->pats_elem; pats != NULL; pats = pats->next_pats) {
								if (pats->pats1 != NULL) {
									patsType = pats->pats1->type;
									if (patsType == 'e')
										pats->pats1->type = 'i';
								}
								isMatchFound = isMorPatMatchedWord(pats->pats1, word);
								if (isMatchFound == 2)
									kideval_error(db, TRUE);
								if (pats->pats1 != NULL && patsType == 'e') {
									pats->pats1->type = patsType;
									if (isMatchFound)
										break;
								}
								if (pats->pats2 == NULL) {
									isMatch2Found = 1;
								} else {
									isMatch2Found = isNextWord(i, uttline, pats->pats2);
									if (isMatch2Found == 2)
										kideval_error(db, TRUE);
								}
								if (j == 0 && isMatchFound && isMatch2Found) {
									if (col->labelP != NULL)
										ts->mor_count[col->labelP->num] = ts->mor_count[col->labelP->num] + 1.0;
									break;
								}
							}
						} else {
							for (pats=col->pats_elem; pats != NULL; pats = pats->next_pats) {
								if (pats->pats1 != NULL) {
									patsType = pats->pats1->type;
									if (patsType == 'e')
										pats->pats1->type = 'i';
								}
								isMatchFound = isMorPatMatchedWord(pats->pats1, w[j]);
								if (isMatchFound == 2)
									kideval_error(db, TRUE);
								if (pats->pats1 != NULL && patsType == 'e') {
									pats->pats1->type = patsType;
									if (isMatchFound)
										break;
								}
								if (pats->pats2 == NULL) {
									isMatch2Found = 1;
								} else if (isMatchFound) {
									isMatch2Found = isNextWord(i, uttline, pats->pats2);
									if (isMatch2Found == 2)
										kideval_error(db, TRUE);
// 2024-03-25 if (isMatchFound && isMatch2Found)
//num = 0;
								}
								if (isMatchFound && isMatch2Found) {
									if (col->labelP != NULL)
										ts->mor_count[col->labelP->num] = ts->mor_count[col->labelP->num] + 1.0;
									break;
								}
							}
						}
					}
				}
			}
		}
		isSkip = FALSE;
		if (isPostCodeOnUtt(spareTier1, "[+ mlue]")) {
			isSkip = TRUE;
		} else {
			i = 0;
			while ((i=getword(utterance->speaker, spareTier3, word, NULL, i))) {
				if (strcmp(word, "xxx") == 0 || strcmp(word, "yyy") == 0 || strcmp(word, "www") == 0) {
					isSkip = TRUE;
					break;
				}
			}
		}
		ts->isMORFound = TRUE;
		mWords50	= ts->mWords50;
		morf50		= ts->morf50;
		mUtt50		= ts->mUtt50;
		mWords		= ts->mWords;
		morf		= ts->morf;
		mUtt		= ts->mUtt;
		isPSDFound	= ts->isPSDFound;
		curPSDFound = FALSE;
		isWordsFound = FALSE;
		isWordsFound50 = FALSE;
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
				isWordsFound50 = TRUE;
				mWords = mWords + 1;
				mWords50 = mWords50 + 1;
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
				morf50 = morf50 + morphCnt;
// fprintf(stderr, "morphCnt=%.0f, new=%.0f: word=%s\n", morphCnt, morf, uttline+oPos);
//				localmorf = localmorf + morphCnt;
				uttline[i] = tchr;
			}
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && isPSDFound) {
				if (mUtt50 > 0.0)
					mUtt50--;
				else if (mUtt50 == 0.0) 
					isWordsFound50 = FALSE;
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (mUtt50 > 0.0)
					mUtt50--;
				else if (mUtt50 == 0.0) 
					isWordsFound50 = FALSE;
				if (mUtt > 0.0)
					mUtt--;
				else if (mUtt == 0.0) 
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))
				curPSDFound = TRUE;
			if (kideval_isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound50) {
					mUtt50 = mUtt50 + (float)1.0;
					isWordsFound50 = FALSE;
				}
				if (isWordsFound) {
					mUtt = mUtt + (float)1.0;
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
			if (mUtt50 <= 50) {
				ts->mWords50	= mWords50;
				ts->morf50		= morf50;
				ts->mUtt50		= mUtt50;
			}
			ts->mWords	   = mWords;
			ts->morf	   = morf;
			ts->mUtt	   = mUtt;
			ts->isPSDFound = curPSDFound;
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
			}
		}
		isAuxFound = FALSE;
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			if (isWordFromMORTier(templineC2)) {
				if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
					kideval_error(db, TRUE);
				for (clitic=&word_feats; clitic != NULL; clitic=clitic->clitc) {
					for (feat=clitic; feat != NULL; feat=feat->compd) {
						if (feat->type != NULL && feat->typeID == '+')
							;
						else if (isAllv(feat) || isnEqual("cop", feat->pos, 3) || isnEqual("mod", feat->pos, 3)) {
							if (isEqual("part", feat->pos)) {
								if (isAuxFound)
									ts->CUR++;
							} else if (isEqual("verb", feat->pos) || isEqual("v", feat->pos) || isnEqual("cop", feat->pos, 3))
								ts->CUR++;
						} else if (isEqual("aux", feat->pos)) {
							isAuxFound = TRUE;
						} else if (isEqual("part", feat->pos)) {
							if (isAuxFound)
								ts->CUR++;
						}
					}
				}
				freeUpFeats(&word_feats);
			}
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE) && isCreateDB == 0) {
		if (db == NULL) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				uS.remblanks(utterance->line);
				kideval_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
			}
		}
	} else if (uS.partcmp(utterance->speaker,"@Types:",FALSE,FALSE) && isCreateDB == 0) {
		getTypes(utterance->line);
	}
}

static char kideval_process_dss_tier(struct kideval_speakers *ts, struct database *db, char lRightspeaker, DSSSP **lastdsp) {
	int k;
	DSSSP *dss_sp;

	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return(lRightspeaker);

		if (*lastdsp != NULL) {
			freeLastDSSUtt(*lastdsp);
			*lastdsp = NULL;
		}
		dss_sp = dss_FindSpeaker(ts->sp, NULL);
		if (dss_sp == NULL)
			kideval_error(db, TRUE);
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
			*lastdsp = dss_sp;
		} else {
			spareTier1[0] = EOS;
			lRightspeaker = FALSE;
		}
	} else if (fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return(lRightspeaker);
		dss_sp = dss_FindSpeaker(ts->sp, NULL);
		if (dss_sp == NULL)
			kideval_error(db, TRUE);
		ts->dss_sp = dss_sp;
		if (!make_DSS_tier(uttline))
			kideval_error(db, FALSE);
		*lastdsp = NULL;
		if (PassedDSSMorTests(spareTier2,uttline,templineC3)) {
			dss_sp->TotalNum = dss_sp->TotalNum + 1;
			for (k=0; spareTier2[k] != EOS; k++) {
				if (spareTier2[k]== '\n' || spareTier2[k]== '\t')
					spareTier2[k] = ' ';
			}
			for (k--; spareTier2[k] == ' '; k--) ;
			spareTier2[++k] = EOS;
			dss_sp->GrandTotal = dss_sp->GrandTotal + PrintOutDSSTable(spareTier2, 0, 0, FALSE); // 2021-09-26 TRUE
		} else {
			dss_sp = dss_FindSpeaker(ts->sp, NULL);
			if (dss_sp == NULL)
				kideval_error(db, TRUE);
			freeLastDSSUtt(dss_sp);
		}
		spareTier1[0] = EOS;
	}
	return(lRightspeaker);
}

static void kideval_process_vocd_tier(struct kideval_speakers *ts, struct database *db) {
	int  i;
	char word[BUFSIZ+1];
	MORFEATS word_feats, *clitic, *feat;
	VOCDSP *vocd_sp;

	if (utterance->speaker[0] == '*') {
/*
		if (ts == NULL)
			return;
		isDoVocdAnalises = TRUE;
		if ((vocd_sp=speaker_add_speaker(&speakers, ts->sp, NULL, NULL, TRUE, TRUE)) == NULL) {
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
*/
	} else if (fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return;
		isDoVocdAnalises = TRUE;
		if ((vocd_sp=speaker_add_speaker(&speakers, ts->sp, NULL, NULL, TRUE, TRUE)) == NULL) {
			fprintf(stderr,"cannot add speaker to list\n");
			kideval_error(db, FALSE);
		}
		ts->vocd_sp = vocd_sp;
		if (vocd_sp != NULL) {
			uS.remblanks(uttline);
			filterwords("%", uttline, excludePlusAndUttDels);
			templineC1[0] = EOS;
			i = 0;
			while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
				if (isWordFromMORTier(templineC2)) {
					if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
						kideval_error(db, TRUE);
					for (clitic=&word_feats; clitic != NULL; clitic=clitic->clitc) {
						if (clitic->compd != NULL) {
							word[0] = EOS;
							for (feat=clitic; feat != NULL; feat=feat->compd) {
								if (feat->stem != NULL && feat->stem[0] != EOS) {
									if (word[0] != EOS)
										strcat(word, "+");
									strcat(word, feat->stem);
								}
							}
							if (word[0] != EOS) {
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									strcat(templineC1, word);
									strcat(templineC1, " ");
								}
							}
						} else {
							if (clitic->stem != NULL && clitic->stem[0] != EOS) {
								if (exclude(clitic->stem) && isRightWord(clitic->stem)) {
									uS.remblanks(clitic->stem);
									strcat(templineC1, clitic->stem);
									strcat(templineC1, " ");
								}
							}
						}
					}
					freeUpFeats(&word_feats);
				}
			}
			strcpy(uttline, templineC1);
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
}

static char kideval_process_ipsyn_tier(struct kideval_speakers *ts, struct database *db, char lRightspeaker, int *misalignmentError, IPSYN_SP **lastipsp) {
	char isFC;
	IPSYN_SP *ipsyn_sp;

	if (utterance->speaker[0] == '*') {
		if (ts == NULL)
			return(lRightspeaker);
		if (*lastipsp != NULL) {
			removeLastUtt(*lastipsp);
			*lastipsp = NULL;
		}
		ipsyn_sp = ipsyn_FindSpeaker(ts->sp, NULL, TRUE);
		if (ipsyn_sp == NULL)
			kideval_error(db, FALSE);
		if (ipsyn_sp->UttNum >= IPS_UTTLIM) {
			lRightspeaker = FALSE;
		} else {
			cleanUttline(uttline);
			isFC = forceInclude(utterance->line);
			if (!isFC && (strcmp(uttline, spareTier1) == 0 ||
						  isExcludeIpsynTier(utterance->speaker, utterance->line, uttline))) {
				spareTier1[0] = EOS;
				lRightspeaker = FALSE;
			} else {
				if (!isIPRepeatUtt(ipsyn_sp, uttline) || isFC) {
					ipsyn_sp->UttNum++;
					add2Utts(ipsyn_sp, uttline);
					strcpy(spareTier1, uttline);
					*lastipsp = ipsyn_sp;
				} else {
					spareTier1[0] = EOS;
					lRightspeaker = FALSE;
				}
			}
		}
	} else if (fDepTierName[0] != EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
		if (ts == NULL)
			return(lRightspeaker);
		*lastipsp = NULL;
		ipsyn_sp = ipsyn_FindSpeaker(ts->sp, NULL, TRUE);
		if (ipsyn_sp == NULL)
			kideval_error(db, FALSE);
		ts->ipsyn_sp = ipsyn_sp;
		strcpy(utterance->tuttline,utterance->line);
		createMorUttline(utterance->line, org_spTier, "%mor:", utterance->tuttline, TRUE, TRUE);
		strcpy(utterance->tuttline, utterance->line);
		if (strchr(uttline, dMarkChr) == NULL) {
			if (isCreateDB) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"WARNING: %s tier does not have one-to-one correspondence to its speaker tier.\n", fDepTierName);
				fprintf(stderr,"%s%s%%mor tier: %s\n", org_spName, org_spTier, utterance->line);
				kideval_error(db, FALSE);
			}
			*misalignmentError = *misalignmentError + 1;
			return(lRightspeaker);
		}
		if (ipsyn_sp->cUtt == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"INTERNAL ERROR utt == null");
			kideval_error(db, FALSE);
		}
		ipsyn_sp->cUtt->morLine = (char *)malloc(strlen(utterance->tuttline)+1);
		if (ipsyn_sp->cUtt->morLine == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			kideval_error(db, FALSE);
		}
		strcpy(ipsyn_sp->cUtt->morLine, utterance->tuttline);
		spareTier1[0] = EOS;
	} else if (fDepTierName[0] == '%' && fDepTierName[1] == 'u' && fDepTierName[2] == 'm') {
		if (uS.partcmp(utterance->speaker,"%ugra:",FALSE,FALSE)) {
			if (ts == NULL)
				return(lRightspeaker);
			ipsyn_sp = ipsyn_FindSpeaker(ts->sp, NULL, TRUE);
			if (ipsyn_sp == NULL)
				kideval_error(db, FALSE);
			ts->ipsyn_sp = ipsyn_sp;
			if (ipsyn_sp->cUtt == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"INTERNAL ERROR utt == null");
				kideval_error(db, FALSE);
			}
			ipsyn_sp->cUtt->graLine = (char *)malloc(strlen(utterance->tuttline)+1);
			if (ipsyn_sp->cUtt->graLine == NULL) {
				fprintf(stderr,"Error: out of memory\n");
				kideval_error(db, FALSE);
			}
			strcpy(ipsyn_sp->cUtt->graLine, utterance->tuttline);
		}
	} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
		if (ts == NULL)
			return(lRightspeaker);
		ipsyn_sp = ipsyn_FindSpeaker(ts->sp, NULL, TRUE);
		if (ipsyn_sp == NULL)
			kideval_error(db, FALSE);
		ts->ipsyn_sp = ipsyn_sp;
		if (ipsyn_sp->cUtt == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"INTERNAL ERROR utt == null");
			kideval_error(db, FALSE);
		}
		ipsyn_sp->cUtt->graLine = (char *)malloc(strlen(utterance->tuttline)+1);
		if (ipsyn_sp->cUtt->graLine == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			kideval_error(db, FALSE);
		}
		strcpy(ipsyn_sp->cUtt->graLine, utterance->tuttline);
	}
	return(lRightspeaker);
}

static char *extractFloatFromLine(char *line, float *val, struct database *db) {
	float num;

	if (*line == EOS) {
		fprintf(stderr, "\nERROR: Database is corrupt\n");
		kideval_error(db, FALSE);
	}
	num = atof(line);
	*val = *val + num;
	while (isdigit(*line) || *line == '.')
		line++;
	while (isSpace(*line))
		line++;
	return(line);
}

static void retrieveTS(struct kideval_speakers *ts, char *line, struct database *db) {
	float tf;
	struct label_cols *col;

	while (isSpace(*line))
		line++;

	line = extractFloatFromLine(line, &ts->mWords, db);
	line = extractFloatFromLine(line, &ts->morf, db);
	line = extractFloatFromLine(line, &ts->tUtt, db);
	line = extractFloatFromLine(line, &ts->mUtt, db);

	line = extractFloatFromLine(line, &ts->mWords50, db);
	line = extractFloatFromLine(line, &ts->morf50, db);
	line = extractFloatFromLine(line, &ts->mUtt50, db);

	line = extractFloatFromLine(line, &ts->frTokens, db);
	line = extractFloatFromLine(line, &ts->frTypes, db);

	line = extractFloatFromLine(line, &ts->NDW, db);
	line = extractFloatFromLine(line, &ts->NDWTotal, db);

	line = extractFloatFromLine(line, &ts->vocdOptimum, db);
	line = extractFloatFromLine(line, &ts->CUR, db);


	line = extractFloatFromLine(line, &ts->tdWords, db);
	line = extractFloatFromLine(line, &ts->tdUtts, db);
	line = extractFloatFromLine(line, &ts->tdur, db);

	line = extractFloatFromLine(line, &ts->werr, db);
	line = extractFloatFromLine(line, &ts->uerr, db);

	line = extractFloatFromLine(line, &ts->retr, db);
	line = extractFloatFromLine(line, &ts->repet, db);

	line = extractFloatFromLine(line, &ts->dssUttCnt, db);
	line = extractFloatFromLine(line, &ts->dssGTotal, db);

	line = extractFloatFromLine(line, &ts->ipsynUttCnt, db);
	line = extractFloatFromLine(line, &ts->ipsynScore, db);

	line = extractFloatFromLine(line, &ts->morTotal, db);

	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (ts->mor_count == NULL) {
			line = extractFloatFromLine(line, &tf, db);
		} else {
			tf = ts->mor_count[col->num];
			line = extractFloatFromLine(line, &tf, db);
			ts->mor_count[col->num] = tf;
		}
	}
}

#ifdef DEBUGEVALRESULTS
static void prDebug(struct kideval_speakers *ts, char *IDText, char *fileName) {
	struct label_cols *col;

	if (ts == NULL)
		return;
	if (dbResults == NULL) {
		dbResults = fopen(DATABASEFILESDEBUG, "wb");
		if (dbResults == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps path doesn't exist or it is opened by another application\n", "DB_results.txt");
			kideval_error(NULL, FALSE);
		}
	}
	fprintf(dbResults, "*** File=%s\n", fileName);
	fprintf(dbResults, "@Types=%s,%s,%s\n", ts->Design, ts->Activity, ts->Group);
	fprintf(dbResults, "@ID=%s\n", IDText);

	fprintf(dbResults, "mWords=%.0f\n", ts->mWords);
	fprintf(dbResults, "morf=%.0f\n", ts->morf);
	fprintf(dbResults, "tUtt=%.0f\n", ts->tUtt);
	fprintf(dbResults, "mUtt=%.0f\n", ts->mUtt);

	fprintf(dbResults, "mWords50=%.0f\n", ts->mWords50);
	fprintf(dbResults, "morf50=%.0f\n", ts->morf50);
	fprintf(dbResults, "mUtt50=%.0f\n", ts->mUtt50);

	fprintf(dbResults, "frTokens=%.0f\n", ts->frTokens);
	fprintf(dbResults, "frTypes=%.0f\n", ts->frTypes);

	fprintf(dbResults, "NDW=%.0f\n", ts->NDW);
	fprintf(dbResults, "NDW=%.0f\n", ts->NDWTotal);

	fprintf(dbResults, "vocdOptimum=%f\n", ts->vocdOptimum);
	fprintf(dbResults, "CUR=%.0f\n", ts->CUR);

	fprintf(dbResults, "tdWords=%.0f\n", ts->tdWords);
	fprintf(dbResults, "tdUtts=%.0f\n", ts->tdUtts);
	fprintf(dbResults, "tdur=%.0f\n", ts->tdur);

	fprintf(dbResults, "werr=%.0f\n", ts->werr);
	fprintf(dbResults, "uerr=%.0f\n", ts->uerr);

	fprintf(dbResults, "retr=%.0f\n", ts->retr);
	fprintf(dbResults, "repet=%.0f\n", ts->repet);

	fprintf(dbResults, "dssUttCnt=%.0f\n", ts->dssUttCnt);
	fprintf(dbResults, "dssGTotal=%.0f\n", ts->dssGTotal);

	fprintf(dbResults, "ipsynUttCnt=%.0f\n", ts->ipsynUttCnt);
	fprintf(dbResults, "ipsynScore=%.0f\n", ts->ipsynScore);

	fprintf(dbResults, "morTotal=%.0f\n", ts->morTotal);

	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (ts->mor_count == NULL) {
			fprintf(dbResults, "mor_count[%d]=NULL\n", col->num);
		} else {
			fprintf(dbResults, "mor_count[%d]=%.0f\n", col->num, ts->mor_count[col->num]);
		}
	}
}
#endif

static void prTSResults(struct kideval_speakers *ts, char *IDText) {
	const char *format, *fn;
	struct label_cols *col;

	format = "%.0f ";
//	format = "%f ";

	fn = strchr(oldfname+wdOffset, PATHDELIMCHR);
	if (fn != NULL)
		fn++;
	else
		fn = oldfname+wdOffset;
	fprintf(dbfpout, "=%s\n", fn);
	fprintf(dbfpout, "+%s\n", IDText);
	fprintf(dbfpout, format, ts->mWords);
	fprintf(dbfpout, format, ts->morf);
	fprintf(dbfpout, format, ts->tUtt);
	fprintf(dbfpout, format, ts->mUtt);

	fprintf(dbfpout, format, ts->mWords50);
	fprintf(dbfpout, format, ts->morf50);
	fprintf(dbfpout, format, ts->mUtt50);

	fprintf(dbfpout, format, ts->frTokens);
	fprintf(dbfpout, format, ts->frTypes);

	fprintf(dbfpout, format, ts->NDW);
	fprintf(dbfpout, format, ts->NDWTotal);

	fprintf(dbfpout, "%f ", ts->vocdOptimum);
	fprintf(dbfpout, format, ts->CUR);

	fprintf(dbfpout, format, ts->tdWords);
	fprintf(dbfpout, format, ts->tdUtts);
	fprintf(dbfpout, format, ts->tdur);

	fprintf(dbfpout, format, ts->werr);
	fprintf(dbfpout, format, ts->uerr);

	fprintf(dbfpout, format, ts->retr);
	fprintf(dbfpout, format, ts->repet);

	fprintf(dbfpout, format, ts->dssUttCnt);
	fprintf(dbfpout, format, ts->dssGTotal);

	fprintf(dbfpout, format, ts->ipsynUttCnt);
	fprintf(dbfpout, format, ts->ipsynScore);

	fprintf(dbfpout, format, ts->morTotal);

	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (ts->mor_count == NULL) {
			fprintf(dbfpout, format, 0.0);
		} else {
			fprintf(dbfpout, format, ts->mor_count[col->num]);
		}
	}
	putc('\n', dbfpout);
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  t, i;
	char word[BUFSIZ+1], DBFname[BUFSIZ+1], IDText[BUFSIZ+1];
	float tt;
	FILE *tfpin;
	struct kideval_speakers *ts;
	struct IDparts IDTier;
	struct label_cols *col;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	if (!fgets_cr(templineC, UTTLINELEN, fpin))
		return;
	if (uS.isUTF8(templineC)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			return;
	}
	if (uS.isInvisibleHeader(templineC)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			return;
		if (uS.isInvisibleHeader(templineC)) {
			if (!fgets_cr(templineC, UTTLINELEN, fpin))
				return;
			if (uS.isInvisibleHeader(templineC)) {
				if (!fgets_cr(templineC, UTTLINELEN, fpin))
					return;
			}
		}
	}
	if (templineC[0] != 'V' || !isdigit(templineC[1])) {
		fprintf(stderr, "\nERROR: Selected database is corrupted.\n");
		kideval_error(db, FALSE);
	}
	t = atoi(templineC+1);
	if (t != KIDEVAL_DB_VERSION) {
		fprintf(stderr, "\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr, "Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		kideval_error(db, FALSE);
	}
	for (t=1; isdigit(templineC[t]); t++) ;
	for (; isSpace(templineC[t]); t++) ;
	for (i=0; isdigit(templineC[t]) && i < 15; t++) {
		DB_MIN_UTTS[i++] = templineC[t];
	}
	DB_MIN_UTTS[i] = EOS;
	uS.remblanks(DB_MIN_UTTS);
	for (; isdigit(templineC[t]) || templineC[t] == '.'; t++) ;
	for (; isSpace(templineC[t]); t++) ;
	strncpy(DB_version, templineC+t, 64);
	DB_version[64] = EOS;
	uS.remblanks(DB_version);
	spareTier1[0] = EOS;
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
			}
		} else if (templineC[0] == '-') {
			ts = db->db_sp;
			if (ts != NULL) {
				if (!ts->isSpeakerFound) {
					fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					kideval_error(db, FALSE);
				}
				*isDbSpFound = TRUE;
#ifdef DEBUGEVALRESULTS
				prDebug(ts, IDText, DBFname);
#endif
				if (ts->tUtt >= DB_UTT_NUM_LIMIT) {
					if (DBFilesListFP != NULL) {
						fprintf(DBFilesListFP, "-----\t%s\n", DBFname);
						fprintf(DBFilesListFP, "@ID:\t%s\n", IDText);
					}

					db->tUtt_num++;
					db->tUtt_sqr = db->tUtt_sqr + (ts->tUtt * ts->tUtt);
					db->tUtt = db->tUtt + ts->tUtt;
					if (db->mn_tUtt == (float)0.0 || db->mn_tUtt > ts->tUtt)
						db->mn_tUtt = ts->tUtt;
					if (db->mx_tUtt < ts->tUtt)
						db->mx_tUtt = ts->tUtt;
					db->frTokens_num++;
					db->frTokens_sqr = db->frTokens_sqr + (ts->frTokens * ts->frTokens);
					db->frTokens = db->frTokens + ts->frTokens;
					if (db->mn_frTokens == (float)0.0 || db->mn_frTokens > ts->frTokens)
						db->mn_frTokens = ts->frTokens;
					if (db->mx_frTokens < ts->frTokens)
						db->mx_frTokens = ts->frTokens;

					db->frTypes_num++;
					db->frTypes_sqr = db->frTypes_sqr + (ts->frTypes * ts->frTypes);
					db->frTypes = db->frTypes + ts->frTypes;
					if (db->mn_frTypes == (float)0.0 || db->mn_frTypes > ts->frTypes)
						db->mn_frTypes = ts->frTypes;
					if (db->mx_frTypes < ts->frTypes)
						db->mx_frTypes = ts->frTypes;
					if (ts->NDWTotal >= 100.0) {
						db->NDW_num++;
						db->NDW_sqr = db->NDW_sqr + (ts->NDW * ts->NDW);
						db->NDW = db->NDW + ts->NDW;
						if (db->mn_NDW == (float)0.0 || db->mn_NDW > ts->NDW)
							db->mn_NDW = ts->NDW;
						if (db->mx_NDW < ts->NDW)
							db->mx_NDW = ts->NDW;
					}
					if (ts->frTokens == 0.0)
						tt = 0.0;
					else
						tt = ts->frTypes / ts->frTokens;
					db->TTR_num++;
					db->TTR_sqr = db->TTR_sqr + (tt * tt);
					db->TTR = db->TTR + tt;
					if (db->mn_TTR == (float)0.0 || db->mn_TTR > tt)
						db->mn_TTR = tt;
					if (db->mx_TTR < tt)
						db->mx_TTR = tt;
					if (ts->tUtt == 0.0)
						tt = 0.0;
					else
						tt = ts->CUR / ts->tUtt;
					db->CUR_num++;
					db->CUR_sqr = db->CUR_sqr + (tt * tt);
					db->CUR = db->CUR + tt;
					if (db->mn_CUR == (float)0.0 || db->mn_CUR > tt)
						db->mn_CUR = tt;
					if (db->mx_CUR < tt)
						db->mx_CUR = tt;
					if (ts->mUtt > 0) {
						db->mUtt_num++;
						db->mUtt_sqr = db->mUtt_sqr + (ts->mUtt * ts->mUtt);
						db->mUtt = db->mUtt + ts->mUtt;
						if (db->mn_mUtt == (float)0.0 || db->mn_mUtt > ts->mUtt)
							db->mn_mUtt = ts->mUtt;
						if (db->mx_mUtt < ts->mUtt)
							db->mx_mUtt = ts->mUtt;

						ts->mWords = ts->mWords / ts->mUtt;
						db->mWords_num++;
						db->mWords_sqr = db->mWords_sqr + (ts->mWords * ts->mWords);
						db->mWords = db->mWords + ts->mWords;
						if (db->mn_mWords == (float)0.0 || db->mn_mWords > ts->mWords)
							db->mn_mWords = ts->mWords;
						if (db->mx_mWords < ts->mWords)
							db->mx_mWords = ts->mWords;

						ts->morf = ts->morf / ts->mUtt;
						db->morf_num++;
						db->morf_sqr = db->morf_sqr + (ts->morf * ts->morf);
						db->morf = db->morf + ts->morf;
						if (db->mn_morf == (float)0.0 || db->mn_morf > ts->morf)
							db->mn_morf = ts->morf;
						if (db->mx_morf < ts->morf)
							db->mx_morf = ts->morf;
					}
					if (ts->mUtt50 >= 50.0) {
						db->mUtt50_num++;
						db->mUtt50_sqr = db->mUtt50_sqr + (ts->mUtt50 * ts->mUtt50);
						db->mUtt50 = db->mUtt50 + ts->mUtt50;
						if (db->mn_mUtt50 == (float)0.0 || db->mn_mUtt50 > ts->mUtt50)
							db->mn_mUtt50 = ts->mUtt50;
						if (db->mx_mUtt50 < ts->mUtt50)
							db->mx_mUtt50 = ts->mUtt50;

						ts->mWords50 = ts->mWords50 / ts->mUtt50;
						db->mWords50_num++;
						db->mWords50_sqr = db->mWords50_sqr + (ts->mWords50 * ts->mWords50);
						db->mWords50 = db->mWords50 + ts->mWords50;
						if (db->mn_mWords50 == (float)0.0 || db->mn_mWords50 > ts->mWords50)
							db->mn_mWords50 = ts->mWords50;
						if (db->mx_mWords50 < ts->mWords50)
							db->mx_mWords50 = ts->mWords50;

						ts->morf50 = ts->morf50 / ts->mUtt50;
						db->morf50_num++;
						db->morf50_sqr = db->morf50_sqr + (ts->morf50 * ts->morf50);
						db->morf50 = db->morf50 + ts->morf50;
						if (db->mn_morf50 == (float)0.0 || db->mn_morf50 > ts->morf50)
							db->mn_morf50 = ts->morf50;
						if (db->mx_morf50 < ts->morf50)
							db->mx_morf50 = ts->morf50;
					}
					db->tdWords_num++;
					db->tdWords_sqr = db->tdWords_sqr + (ts->tdWords * ts->tdWords);
					db->tdWords = db->tdWords + ts->tdWords;
					if (db->mn_tdWords == (float)0.0 || db->mn_tdWords > ts->tdWords)
						db->mn_tdWords = ts->tdWords;
					if (db->mx_tdWords < ts->tdWords)
						db->mx_tdWords = ts->tdWords;

					db->tdUtts_num++;
					db->tdUtts_sqr = db->tdUtts_sqr + (ts->tdUtts * ts->tdUtts);
					db->tdUtts = db->tdUtts + ts->tdUtts;
					if (db->mn_tdUtts == (float)0.0 || db->mn_tdUtts > ts->tdUtts)
						db->mn_tdUtts = ts->tdUtts;
					if (db->mx_tdUtts < ts->tdUtts)
						db->mx_tdUtts = ts->tdUtts;

					db->tdur_num++;
					db->tdur_sqr = db->tdur_sqr + (ts->tdur * ts->tdur);
					db->tdur = db->tdur + ts->tdur;
					if (db->mn_tdur == (float)0.0 || db->mn_tdur > ts->tdur)
						db->mn_tdur = ts->tdur;
					if (db->mx_tdur < ts->tdur)
						db->mx_tdur = ts->tdur;

					if (ts->tdur == 0.0)
						tt = 0.0;
					else
						tt = ts->tdWords / ts->tdur;
					db->tdWT_num++;
					db->tdWT_sqr = db->tdWT_sqr + (tt * tt);
					db->tdWT = db->tdWT + tt;
					if (db->mn_tdWT == (float)0.0 || db->mn_tdWT > tt)
						db->mn_tdWT = tt;
					if (db->mx_tdWT < tt)
						db->mx_tdWT = tt;
					if (ts->tdur == 0.0)
						tt = 0.0;
					else
						tt = ts->tdUtts / ts->tdur;
					db->tdUT_num++;
					db->tdUT_sqr = db->tdUT_sqr + (tt * tt);
					db->tdUT = db->tdUT + tt;
					if (db->mn_tdUT == (float)0.0 || db->mn_tdUT > tt)
						db->mn_tdUT = tt;
					if (db->mx_tdUT < tt)
						db->mx_tdUT = tt;

					if (!isRawVal) {
						if (ts->frTokens == 0.0)
							ts->werr = 0.0;
						else
							ts->werr = (ts->werr / ts->frTokens) * 100.0000;
					}
					db->werr_num++;
					db->werr_sqr = db->werr_sqr + (ts->werr * ts->werr);
					db->werr = db->werr + ts->werr;
					if (db->mn_werr == (float)0.0 || db->mn_werr > ts->werr)
						db->mn_werr = ts->werr;
					if (db->mx_werr < ts->werr)
						db->mx_werr = ts->werr;
					db->uerr_num++;
					db->uerr_sqr = db->uerr_sqr + (ts->uerr * ts->uerr);
					db->uerr = db->uerr + ts->uerr;
					if (db->mn_uerr == (float)0.0 || db->mn_uerr > ts->uerr)
						db->mn_uerr = ts->uerr;
					if (db->mx_uerr < ts->uerr)
						db->mx_uerr = ts->uerr;
					db->retr_num++;
					db->retr_sqr = db->retr_sqr + (ts->retr * ts->retr);
					db->retr = db->retr + ts->retr;
					if (db->mn_retr == (float)0.0 || db->mn_retr > ts->retr)
						db->mn_retr = ts->retr;
					if (db->mx_retr < ts->retr)
						db->mx_retr = ts->retr;
					db->repet_num++;
					db->repet_sqr = db->repet_sqr + (ts->repet * ts->repet);
					db->repet = db->repet + ts->repet;
					if (db->mn_repet == (float)0.0 || db->mn_repet > ts->repet)
						db->mn_repet = ts->repet;
					if (db->mx_repet < ts->repet)
						db->mx_repet = ts->repet;
					if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0 && ts->dssUttCnt >= 50.0) {
						db->dssUttCnt_num++;
						db->dssUttCnt_sqr = db->dssUttCnt_sqr + (ts->dssUttCnt * ts->dssUttCnt);
						db->dssUttCnt = db->dssUttCnt + ts->dssUttCnt;
						if (db->mn_dssUttCnt == (float)0.0 || db->mn_dssUttCnt > ts->dssUttCnt)
							db->mn_dssUttCnt = ts->dssUttCnt;
						if (db->mx_dssUttCnt < ts->dssUttCnt)
							db->mx_dssUttCnt = ts->dssUttCnt;
						ts->dssGTotal = ts->dssGTotal / ts->dssUttCnt;
						db->dssGTotal_num++;
						db->dssGTotal_sqr = db->dssGTotal_sqr + (ts->dssGTotal * ts->dssGTotal);
						db->dssGTotal = db->dssGTotal + ts->dssGTotal;
						if (db->mn_dssGTotal == (float)0.0 || db->mn_dssGTotal > ts->dssGTotal)
							db->mn_dssGTotal = ts->dssGTotal;
						if (db->mx_dssGTotal < ts->dssGTotal)
							db->mx_dssGTotal = ts->dssGTotal;
					}
					if (ts->vocdOptimum > 0.0) {
						db->vocdOptimum_num++;
						db->vocdOptimum_sqr = db->vocdOptimum_sqr + (ts->vocdOptimum * ts->vocdOptimum);
						db->vocdOptimum = db->vocdOptimum + ts->vocdOptimum;
						if (db->mn_vocdOptimum == (float)0.0 || db->mn_vocdOptimum > ts->vocdOptimum)
							db->mn_vocdOptimum = ts->vocdOptimum;
						if (db->mx_vocdOptimum < ts->vocdOptimum)
							db->mx_vocdOptimum = ts->vocdOptimum;
					}
					if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0 && ts->ipsynUttCnt >= (float)IPS_UTTLIM) {
						db->ipsynUttCnt_num++;
						db->ipsynUttCnt_sqr = db->ipsynUttCnt_sqr + (ts->ipsynUttCnt * ts->ipsynUttCnt);
						db->ipsynUttCnt = db->ipsynUttCnt + ts->ipsynUttCnt;
						if (db->mn_ipsynUttCnt == (float)0.0 || db->mn_ipsynUttCnt > ts->ipsynUttCnt)
							db->mn_ipsynUttCnt = ts->ipsynUttCnt;
						if (db->mx_ipsynUttCnt < ts->ipsynUttCnt)
							db->mx_ipsynUttCnt = ts->ipsynUttCnt;
						db->ipsynScore_num++;
						db->ipsynScore_sqr = db->ipsynScore_sqr + (ts->ipsynScore * ts->ipsynScore);
						db->ipsynScore = db->ipsynScore + ts->ipsynScore;
						if (db->mn_ipsynScore == (float)0.0 || db->mn_ipsynScore > ts->ipsynScore)
							db->mn_ipsynScore = ts->ipsynScore;
						if (db->mx_ipsynScore < ts->ipsynScore)
							db->mx_ipsynScore = ts->ipsynScore;
					}
					db->morTotal_num++;
					db->morTotal_sqr = db->morTotal_sqr + (ts->morTotal * ts->morTotal);
					db->morTotal = db->morTotal + ts->morTotal;
					if (db->mn_morTotal == (float)0.0 || db->mn_morTotal > ts->morTotal)
						db->mn_morTotal = ts->morTotal;
					if (db->mx_morTotal < ts->morTotal)
						db->mx_morTotal = ts->morTotal;
					
					db->morItems_num++;
					for (col=labelsRoot; col != NULL; col=col->next_label) {
						tt = ts->mor_count[col->num];
						db->morItems_sqr[col->num] = db->morItems_sqr[col->num] + (tt * tt);
						db->morItems[col->num] = db->morItems[col->num] + tt;
					}
				}

				db->db_sp = freespeakers(db->db_sp);

			}
			ts = NULL;
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
			} else if (isdigit(templineC[0])) {
				strcpy(utterance->speaker, "*CHI:");
				strcpy(utterance->line, templineC);
			}
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			if (utterance->speaker[0] == '*') {
				uS.remFrontAndBackBlanks(utterance->line);
				strcpy(templineC, utterance->speaker);
				ts = kideval_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
				if (ts != NULL)
					retrieveTS(ts, utterance->line, db);
			} else
				kideval_process_tier(ts, db);
		}
	}
	fpin = tfpin;
}

static void ParseDatabase(struct database *db, char *lang) {
	char   isDbSpFound = FALSE;
	FILE *fp;

	// remove beg
/*
#ifdef UNX
	strcpy(FileName1, lib_dir);
#else
	strcpy(FileName1, prefsDir);
#endif
	addFilename2Path(FileName1, lang);
	strcat(FileName1, DB_type);
	strcat(FileName1, DATABASE_FILE_NAME);
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
*/
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, "kideval");
		addFilename2Path(FileName1, lang);
		strcat(FileName1, DB_type);
		strcat(FileName1, DATABASE_FILE_NAME);
		fp = fopen(FileName1, "r");
//	}
	if (fp == NULL) {
		fprintf(stderr, "\nCan't find database in libary \"%s\"\n", FileName1);
#ifdef UNX
		fprintf(stderr,"Please use -L option to specify libary path.\n");
#else
		fprintf(stderr,"Please make sure that the \"lib\" in the Commands window is set to the \"clan/lib\" directory.\n");
#endif
//		fprintf(stderr, "\t or in preferences folder \n");
//		fprintf(stderr, "Please download database with \"File->Get KIDEVAL Database\" menu\n");
		fprintf(stderr, "If you do not use database, then do not include +d option in command line\n");
		kideval_error(db, FALSE);
	} else {
		fprintf(stderr, "    Database File used: %s\n", FileName1);
		OpenDBFile(fp, db, &isDbSpFound);
//		fprintf(stderr, "    %.0f      \n", db->tUtt_num);
	}
	if (!isDbSpFound || db->tUtt_num == 0.0) {
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
	if (SD->dbSD == 0.0 || (div != NULL && *div <= 0.0)) {
		excelStrCell(fpout, "NA");
		SD->stars = 0;
	} else {
		if (div != NULL && *div != 0.0)
			score = score / (*div);
		mean = mean / num; 
		cSD = (score - mean) / SD->dbSD;
		excelNumCell(fpout, "%.3f", cSD);
		if (cSD <= -2.0000 || cSD >= 2.0000)
			SD->stars = 2;
		else if (cSD <= -1.0000 || cSD >= 1.0000)
			SD->stars = 1;
		else
			SD->stars = 0;
	}
}

static void set_SD2NA(struct SDs *SD, float sqr_mean, float mean, float num) {
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
	excelStrCell(fpout, "NA");
	SD->stars = 0;
}

static char *getFileName(char *oFname) {
	int  i, cnt;

	cnt = 0;
	i=strlen(oFname) - 1;
	while (i >= 0) {
		if (oFname[i] == PATHDELIMCHR) {
			cnt++;
			if (cnt == 2)
				break;
		}
		i--;
	}
	i++;
	return(oFname+i);
}

static void kideval_pr_result(void) {
	int    i, SDn = 0;
	char   *sFName;
	float  tNum, nonZeroMorTotal;
	struct kideval_speakers *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;
	struct label_cols *col;

	if (isCreateDB) {
		sp_head = freespeakers(sp_head);
		return;
	}
	if (sp_head == NULL) {
		fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
		if (isLinkAge)
			fprintf(stderr,"OR No input file had speaker in the right age range\n");
		fprintf(stderr,"\n");
	}
	excelHeader(fpout, newfname, 95);
	excelRow(fpout, ExcelRowStart);
	if (DBKeyRoot != NULL) {
		for (i=0; i < SDRESSIZE; i++) {
			SD[i].isSDComputed = FALSE;
		}
		excelStrCell(fpout,"File_DB");
	} else
		excelStrCell(fpout,"File");
	excelCommasStrCell(fpout, "Language,Corpus,Code,Age(Month),Sex,Group,Race,SES,Role,Education,Custom_field");
	excelCommasStrCell(fpout, "Design,Activity,Group");
	excelCommasStrCell(fpout, "Total_Utts");
	excelCommasStrCell(fpout, "MLU_Utts,MLU_Words,MLU_Morphemes");
	excelCommasStrCell(fpout, "MLU50_Utts,MLU50_Words,MLU50_Morphemes");
	excelCommasStrCell(fpout, "FREQ_types,FREQ_tokens");
// 2024-04-11	excelCommasStrCell(fpout, "FREQ_types,FREQ_tokens,FREQ_TTR");
	excelCommasStrCell(fpout, "NDW_100");
	excelCommasStrCell(fpout, "VOCD_D_optimum_average");
	excelCommasStrCell(fpout, "Verbs_Utt");
	if (DBKeyRoot == NULL) { // exclude TD columns
		excelCommasStrCell(fpout, "TD_Words,TD_Utts,TD_Time_(secs),TD_Words_Time,TD_Utts_Time");
	}
	if (isRawVal) {
		excelCommasStrCell(fpout, "Word_Errors");
	} else {
		excelCommasStrCell(fpout, "%_Word_Errors");
	}
	excelCommasStrCell(fpout, "Utt_Errors");
	excelCommasStrCell(fpout, "retracing,repetition");
	if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
		excelCommasStrCell(fpout, "DSS_Utts,DSS");
	}
	if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
		excelCommasStrCell(fpout, "IPSyn_Utts,IPSyn_Total");
	}
	excelCommasStrCell(fpout, "mor_Words");
	for (col=labelsRoot; col != NULL; col=col->next_label) {
		if (col->label != NULL)
			excelStrCell(fpout, col->label);
	}
	excelCommasStrCell(fpout, "Total_non_zero_mors");
	excelRow(fpout, ExcelRowEnd);
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			fprintf(stderr,"\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
		if (ts->tUtt < MinUttLimit)
			continue;
// ".xls"
		excelRow(fpout, ExcelRowStart);
		sFName = getFileName(ts->fname);
		excelStrCell(fpout, sFName);
		if (ts->ID) {
			excelOutputID(fpout, ts->ID);
		} else {
			excelCommasStrCell(fpout, ".,.");
			excelStrCell(fpout, ts->sp);
			excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
		}
		if (ts->Design == NULL)
			excelStrCell(fpout, "");
		else
			excelStrCell(fpout, ts->Design);
		if (ts->Activity == NULL)
			excelStrCell(fpout, "");
		else
			excelStrCell(fpout, ts->Activity);
		if (ts->Group == NULL)
			excelStrCell(fpout, "");
		else
			excelStrCell(fpout, ts->Group);
		if (!ts->isMORFound) {
			fprintf(stderr,"\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
			fprintf(stderr,"WARNING: Speaker \"%s\" has no \"%s\" tiers.\n\n", ts->sp, fDepTierName);
			excelStrCell(fpout, "0");
			excelCommasStrCell(fpout, "0,0,0,0,0,0");
			excelCommasStrCell(fpout, "0,0,0");
			excelStrCell(fpout, "0");
			excelStrCell(fpout, "0");
			excelStrCell(fpout, "0");
			if (DBKeyRoot == NULL) // exclude TD columns
				excelCommasStrCell(fpout, "0,0,0,0,0");
			excelStrCell(fpout, "0");
			excelStrCell(fpout, "0");
			excelCommasStrCell(fpout, "0,0");
			if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
				excelCommasStrCell(fpout, "0,0");
			}
			if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
				excelCommasStrCell(fpout, "0,0");
			}
			excelStrCell(fpout, "0");
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				excelStrCell(fpout, "0");
			}
			excelStrCell(fpout, "0");
			excelRow(fpout, ExcelRowEnd);
			excelRow(fpout, ExcelRowEmpty);
			continue;
		}
		excelNumCell(fpout, "%.0f", ts->tUtt);
		excelNumCell(fpout, "%.0f", ts->mUtt);
		if (ts->mUtt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->mWords/ts->mUtt);
		if (ts->mUtt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->morf/ts->mUtt);
		excelNumCell(fpout, "%.0f", ts->mUtt50);
		if (ts->mUtt50 < 50.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->mWords50/ts->mUtt50);
		if (ts->mUtt50 < 50.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->morf50/ts->mUtt50);
		excelNumCell(fpout, "%.0f", ts->frTypes);
		excelNumCell(fpout, "%.0f", ts->frTokens);
/* 2024-04-11
		if (ts->frTokens == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->frTypes/ts->frTokens);
*/
		if (ts->NDWTotal < 100)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.0f", ts->NDW);
		if (ts->vocdOptimum <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.2f", ts->vocdOptimum);
		if (ts->tUtt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->CUR/ts->tUtt);
		if (DBKeyRoot == NULL) { // exclude TD columns
			excelNumCell(fpout, "%.0f", ts->tdWords);
			excelNumCell(fpout, "%.0f", ts->tdUtts);
//			excelNumCell(fpout, "%.0f", ts->tdur);
			excelNumCell(fpout, "%.0f", roundFloat(ts->tdur));
			if (ts->tdur == 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f", ts->tdWords/ts->tdur);
			if (ts->tdur == 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f", ts->tdUtts/ts->tdur);
		}
		if (!isRawVal) {
			if (ts->frTokens == 0.0)
				ts->werr = 0.000;
			else
				ts->werr = (ts->werr / ts->frTokens) * 100.0000;
			excelNumCell(fpout, "%.3f", ts->werr);
		} else
			excelNumCell(fpout, "%.0f", ts->werr);
		excelNumCell(fpout, "%.0f", ts->uerr);
		excelNumCell(fpout, "%.0f", ts->retr);
		excelNumCell(fpout, "%.0f", ts->repet);
		if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
			excelNumCell(fpout, "%.0f", ts->dssUttCnt);
			tNum = ts->dssGTotal;
/* 2019-04-24 original
			if (DssIpsynUttLimit > 0 && ts->dssUttCnt <= DssIpsynUttLimit)
				excelNumCell(fpout, "%.2f", tNum / ts->dssUttCnt);
			else if (ts->dssUttCnt < 50.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.2f", tNum / ts->dssUttCnt);
*/
/* changed on 2019-04-24
			if (DssIpsynUttLimit > 0) {
				if (ts->dssUttCnt < DssIpsynUttLimit)
					excelStrCell(fpout, "NA");
				else
					excelNumCell(fpout, "%.2f", tNum / ts->dssUttCnt);
			} else {
*/
				if (ts->dssUttCnt < (float)DSS_UTTLIM)
					excelStrCell(fpout, "NA");
				else
					excelNumCell(fpout, "%.2f", tNum / ts->dssUttCnt);
//			}
		}
		if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
			excelNumCell(fpout, "%.0f", ts->ipsynUttCnt);
/* 2019-04-24 original

			if (DssIpsynUttLimit > 0 && ts->dssUttCnt <= DssIpsynUttLimit)
				excelNumCell(fpout, "%.2f", ts->ipsynScore);
			else if (ts->ipsynUttCnt < (float)IPS_UTTLIM)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.2f", ts->ipsynScore);
*/
/* changed on 2019-04-24
			if (DssIpsynUttLimit > 0) {
				if (ts->ipsynUttCnt < DssIpsynUttLimit)
					excelStrCell(fpout, "NA");
				else
					excelNumCell(fpout, "%.2f", ts->ipsynScore);
			} else {
*/
				if (ts->ipsynUttCnt < (float)IPS_UTTLIM)
					excelStrCell(fpout, "NA");
				else
					excelNumCell(fpout, "%.2f", ts->ipsynScore);
//			}
		}
		excelNumCell(fpout, "%.0f", ts->morTotal);
		nonZeroMorTotal = 0.0;
		for (col=labelsRoot; col != NULL; col=col->next_label) {
			if (ts->mor_count == NULL) {
				excelStrCell(fpout, ".");
			} else {
				tNum = ts->mor_count[col->num];
				if (!isRawVal) {
					if (ts->morTotal == 0.0)
						excelStrCell(fpout, "0.000");
					else {
						excelNumCell(fpout, "%.3f", (tNum/ts->morTotal)*100.0000);
						if (tNum > 0)
							nonZeroMorTotal++;
					}
				} else {
					excelNumCell(fpout, "%.0f", tNum);
					if (tNum > 0)
						nonZeroMorTotal++;
				}
			}
		}

		excelNumCell(fpout, "%.0f", nonZeroMorTotal);

		excelRow(fpout, ExcelRowEnd);
		if (DBKeyRoot != NULL) {
			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout,"+/-SD,.,.,.,.,.,.,.,.,.,.,.,.,.,.");

			SDn = 0; // this number should be less than SDRESSIZE = 256
			excelStrCell(fpout, ""); SDn++;
//			compute_SD(&SD[SDn++], ts->tUtt,  NULL, gdb->tUtt_sqr, gdb->tUtt, gdb->tUtt_num);
			compute_SD(&SD[SDn++], ts->mUtt,  NULL, gdb->mUtt_sqr, gdb->mUtt, gdb->mUtt_num);

			compute_SD(&SD[SDn++], ts->mWords,  &ts->mUtt, gdb->mWords_sqr, gdb->mWords, gdb->mWords_num);
			compute_SD(&SD[SDn++], ts->morf,  &ts->mUtt, gdb->morf_sqr, gdb->morf, gdb->morf_num);

// for mUtt50 always say NA
			excelStrCell(fpout, "NA");
			SD[SDn++].stars = 0;
//			compute_SD(&SD[SDn++], ts->mUtt50,  NULL, gdb->mUtt50_sqr, gdb->mUtt50, gdb->mUtt50_num);
			if (ts->mUtt50 < 50.0) {
				set_SD2NA(&SD[SDn++], gdb->mWords50_sqr, gdb->mWords50, gdb->mWords50_num);
				set_SD2NA(&SD[SDn++], gdb->morf50_sqr, gdb->morf50, gdb->morf50_num);
			} else {
				compute_SD(&SD[SDn++], ts->mWords50,  &ts->mUtt50, gdb->mWords50_sqr, gdb->mWords50, gdb->mWords50_num);
				compute_SD(&SD[SDn++], ts->morf50,  &ts->mUtt50, gdb->morf50_sqr, gdb->morf50, gdb->morf50_num);
			}
			compute_SD(&SD[SDn++], ts->frTypes,  NULL, gdb->frTypes_sqr, gdb->frTypes, gdb->frTypes_num);
			compute_SD(&SD[SDn++], ts->frTokens,  NULL, gdb->frTokens_sqr, gdb->frTokens, gdb->frTokens_num);
// 2024-04-11			compute_SD(&SD[SDn++], ts->frTypes,  &ts->frTokens, gdb->TTR_sqr, gdb->TTR, gdb->TTR_num);
			if (ts->NDWTotal < 100)
				set_SD2NA(&SD[SDn++], gdb->NDW_sqr, gdb->NDW, gdb->NDW_num);
			else
				compute_SD(&SD[SDn++], ts->NDW,  NULL, gdb->NDW_sqr, gdb->NDW, gdb->NDW_num);

			if (ts->vocdOptimum <= 0.0)
				set_SD2NA(&SD[SDn++], gdb->vocdOptimum_sqr, gdb->vocdOptimum, gdb->vocdOptimum_num);
			else
				compute_SD(&SD[SDn++], ts->vocdOptimum,  NULL, gdb->vocdOptimum_sqr, gdb->vocdOptimum, gdb->vocdOptimum_num);
			compute_SD(&SD[SDn++], ts->CUR,  &ts->tUtt, gdb->CUR_sqr, gdb->CUR, gdb->CUR_num);
/* exclude TD columns
			compute_SD(&SD[SDn++], ts->tdWords,  NULL, gdb->tdWords_sqr, gdb->tdWords, gdb->tdWords_num);
			compute_SD(&SD[SDn++], ts->tdUtts,  NULL, gdb->tdUtts_sqr, gdb->tdUtts, gdb->tdUtts_num);
			compute_SD(&SD[SDn++], ts->tdur,  NULL, gdb->tdur_sqr, gdb->tdur, gdb->tdur_num);
			compute_SD(&SD[SDn++], ts->tdWords,  &ts->tdur, gdb->tdWT_sqr, gdb->tdWT, gdb->tdWT_num);
			compute_SD(&SD[SDn++], ts->tdUtts,  &ts->tdur, gdb->tdUT_sqr, gdb->tdUT, gdb->tdUT_num);
*/
			compute_SD(&SD[SDn++], ts->werr,  NULL, gdb->werr_sqr, gdb->werr, gdb->werr_num);
			compute_SD(&SD[SDn++], ts->uerr,  NULL, gdb->uerr_sqr, gdb->uerr, gdb->uerr_num);

			compute_SD(&SD[SDn++], ts->retr,  NULL, gdb->retr_sqr, gdb->retr, gdb->retr_num);
			compute_SD(&SD[SDn++], ts->repet,  NULL, gdb->repet_sqr, gdb->repet, gdb->repet_num);

			if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
// for DSS Utts always say NA
				excelStrCell(fpout, "NA");
				SD[SDn++].stars = 0;
//				compute_SD(&SD[SDn++], ts->dssUttCnt,  NULL, gdb->dssUttCnt_sqr, gdb->dssUttCnt, gdb->dssUttCnt_num);
				if (ts->dssUttCnt < (float)DSS_UTTLIM)
					set_SD2NA(&SD[SDn++], gdb->dssGTotal_sqr, gdb->dssGTotal, gdb->dssGTotal_num);
				else
					compute_SD(&SD[SDn++], ts->dssGTotal,  &ts->dssUttCnt, gdb->dssGTotal_sqr, gdb->dssGTotal, gdb->dssGTotal_num);
			}

			if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
// for IPSyn Utts always say NA
				excelStrCell(fpout, "NA");
				SD[SDn++].stars = 0;
//				compute_SD(&SD[SDn++], ts->ipsynUttCnt,  NULL, gdb->ipsynUttCnt_sqr, gdb->ipsynUttCnt, gdb->ipsynUttCnt_num);
				if (ts->ipsynUttCnt < (float)IPS_UTTLIM)
					set_SD2NA(&SD[SDn++], gdb->ipsynScore_sqr, gdb->ipsynScore, gdb->ipsynScore_num);
				else
					compute_SD(&SD[SDn++], ts->ipsynScore,  NULL, gdb->ipsynScore_sqr, gdb->ipsynScore, gdb->ipsynScore_num);
			}
			compute_SD(&SD[SDn++], ts->morTotal,  NULL, gdb->morTotal_sqr, gdb->morTotal, gdb->morTotal_num);

			// exclude %mor items columns
			if (uS.mStricmp(ke_script_file,"eng") != 0 /* engu && uS.mStricmp(ke_script_file,"engu") != 0*/) {
				for (col=labelsRoot; col != NULL; col=col->next_label) {
					if (ts->mor_count != NULL) {
						compute_SD(&SD[SDn++], ts->mor_count[col->num],  NULL, gdb->morItems_sqr[col->num], gdb->morItems[col->num], gdb->morItems_num);
					}
				}
			}
			excelRow(fpout, ExcelRowEnd);

			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout, " , , , , , , , , , , , , , , ");
			for (i=0; i < SDn; i++) {
				if (i == 0) {
					excelStrCell(fpout, "");
				} else {
					if (SD[i].stars >= 2)
						excelStrCell(fpout, "**");
					else if (SD[i].stars >= 1)
						excelStrCell(fpout, "*");
					else
						excelStrCell(fpout, " ");
				}
			}
			excelRow(fpout, ExcelRowEnd);
		}
	}
	if (DBKeyRoot != NULL) {
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"Mean Database,.,.,.,.,.,.,.,.,.,.,.,.,.,.");
		excelStrCell(fpout, "");
//		if (gdb->tUtt_num <= 0.0)
//			excelStrCell(fpout, "NA");
//		else
//			excelNumCell(fpout, "%.3f",gdb->tUtt/gdb->tUtt_num);
		if (gdb->mUtt_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->mUtt/gdb->mUtt_num);
		if (gdb->mWords_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->mWords/gdb->mWords_num);
		if (gdb->morf_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->morf/gdb->morf_num);

		if (gdb->mUtt50_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->mUtt50/gdb->mUtt50_num);
		if (gdb->mWords50_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->mWords50/gdb->mWords50_num);
		if (gdb->morf50_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->morf50/gdb->morf50_num);

		if (gdb->frTypes_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->frTypes/gdb->frTypes_num);
		if (gdb->frTokens_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->frTokens/gdb->frTokens_num);
/* 2024-04-11
		if (gdb->TTR_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->TTR/gdb->TTR_num);
*/
		if (gdb->NDW_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->NDW/gdb->NDW_num);

		if (gdb->vocdOptimum_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->vocdOptimum/gdb->vocdOptimum_num);
		if (gdb->CUR_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->CUR/gdb->CUR_num);
/* exclude TD columns
		if (gdb->tdWords_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->tdWords/gdb->tdWords_num);
		if (gdb->tdUtts_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->tdUtts/gdb->tdUtts_num);
		if (gdb->tdur_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.0f",gdb->tdur/gdb->tdur_num);
		if (gdb->tdWT_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->tdWT/gdb->tdWT_num);
		if (gdb->tdUT_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->tdUT/gdb->tdUT_num);
*/
		if (gdb->werr_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->werr/gdb->werr_num);
		if (gdb->uerr_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->uerr/gdb->uerr_num);

		if (gdb->retr_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->retr/gdb->retr_num);
		if (gdb->repet_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->repet/gdb->repet_num);

		if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
			if (gdb->dssUttCnt_num <= 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f",gdb->dssUttCnt/gdb->dssUttCnt_num);
			if (gdb->dssGTotal_num <= 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f",gdb->dssGTotal/gdb->dssGTotal_num);
		}

		if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
			if (gdb->ipsynUttCnt_num <= 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f",gdb->ipsynUttCnt/gdb->ipsynUttCnt_num);
			if (gdb->ipsynScore_num <= 0.0)
				excelStrCell(fpout, "NA");
			else
				excelNumCell(fpout, "%.3f",gdb->ipsynScore/gdb->ipsynScore_num);
		}

		if (gdb->morTotal_num <= 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f",gdb->morTotal/gdb->morTotal_num);

		// exclude %mor items columns
		if (uS.mStricmp(ke_script_file, "eng") != 0) {
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				if (gdb->morItems_num <= 0.0)
					excelStrCell(fpout, "NA");
				else
					excelNumCell(fpout, "%.3f",gdb->morItems[col->num]/gdb->morItems_num);
			}
		}
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"SD Database,.,.,.,.,.,.,.,.,.,.,.,.,.,.");
		for (i=0; i < SDn; i++) {
			if (i == 0) {
				excelStrCell(fpout, "");
			} else {
				if (SD[i].isSDComputed == 1) {
					excelNumCell(fpout, "%.3f", SD[i].dbSD);
				} else {
					excelStrCell(fpout, "NA");
				}
			}
		}
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"Number files,.,.,.,.,.,.,.,.,.,.,.,.,.,.");
		excelStrCell(fpout, "");
//		excelNumCell(fpout, "%.0f",gdb->tUtt_num);
		excelNumCell(fpout, "%.0f",gdb->mUtt_num);

		excelNumCell(fpout, "%.0f",gdb->mWords_num);
		excelNumCell(fpout, "%.0f",gdb->morf_num);

		excelNumCell(fpout, "%.0f",gdb->mUtt50_num);
		excelNumCell(fpout, "%.0f",gdb->mWords50_num);
		excelNumCell(fpout, "%.0f",gdb->morf50_num);

		excelNumCell(fpout, "%.0f",gdb->frTypes_num);
		excelNumCell(fpout, "%.0f",gdb->frTokens_num);
// 2024-04-11		excelNumCell(fpout, "%.0f",gdb->TTR_num);
		excelNumCell(fpout, "%.0f",gdb->NDW_num);

		excelNumCell(fpout, "%.0f",gdb->vocdOptimum_num);
		excelNumCell(fpout, "%.0f",gdb->CUR_num);
/* exclude TD columns
		excelNumCell(fpout, "%.0f",gdb->tdWords_num);
		excelNumCell(fpout, "%.0f",gdb->tdUtts_num);
		excelNumCell(fpout, "%.0f",gdb->tdur_num);
		excelNumCell(fpout, "%.0f",gdb->tdWT_num);
		excelNumCell(fpout, "%.0f",gdb->tdUT_num);
*/
		excelNumCell(fpout, "%.0f",gdb->werr_num);
		excelNumCell(fpout, "%.0f",gdb->uerr_num);

		excelNumCell(fpout, "%.0f",gdb->retr_num);
		excelNumCell(fpout, "%.0f",gdb->repet_num);

		if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
			excelNumCell(fpout, "%.0f",gdb->dssUttCnt_num);
			excelNumCell(fpout, "%.0f",gdb->dssGTotal_num);
		}
		if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
			excelNumCell(fpout, "%.0f",gdb->ipsynUttCnt_num);
			excelNumCell(fpout, "%.0f",gdb->ipsynScore_num);
		}
		excelNumCell(fpout, "%.0f",gdb->morTotal_num);

		// exclude %mor items columns
		if (uS.mStricmp(ke_script_file, "eng") != 0) {
			for (col=labelsRoot; col != NULL; col=col->next_label) {
				if (gdb->morItems != NULL) {
					excelNumCell(fpout, "%.0f",gdb->morItems_num);
				}
			}
		}
		excelRow(fpout, ExcelRowEnd);
	}
	excelRow(fpout, ExcelRowEmpty);
	if (sp_head != NULL) {
		excelRowOneStrCell(fpout, ExcelRedCell, "IF YOU DID NOT MARK ERRORS, DSS TOTAL WILL BE INFLATED.");
		excelRowOneStrCell(fpout, ExcelRedCell, "Please be sure to mark utterances with errors with [*] to ensure accurate DSS computation.");
		excelRow(fpout, ExcelRowEmpty);
		if (DBKeyRoot != NULL) {
			excelRowOneStrCell(fpout, ExcelRedCell, "PLEASE SEE SLP MANUAL FOR RECOMMENDED INTERPRETATION.");
			excelRow(fpout, ExcelRowEmpty);
			excelRowOneStrCell(fpout, ExcelBlkCell, "+/- SD  * = 1 SD, ** = 2 SD");
		}
		if (DBKeyRoot != NULL) {
			strcpy(templineC4, "Database keywords: ");
			i = strlen(templineC4);
			for (DBKey=DBKeyRoot; DBKey != NULL; DBKey=DBKey->next) {
				if (DBKey->key1 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key1);
				}
				if (DBKey->key2 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key2);
				}
				if (DBKey->key3 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key3);
				}
				if (DBKey->key4 != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->key4);
				}
				if (DBKey->age != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->age);
				}
				if (DBKey->sex != NULL) {
					if (templineC4[i] != EOS)
						strcat(templineC4, "|");
					strcat(templineC4, DBKey->sex);
				}
				strcat(templineC4, ", ");
			}
			uS.remblanks(templineC4);
			i = strlen(templineC4) - 1;
			if (templineC4[i] == ',')
				templineC4[i] = EOS;
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);

			strcpy(templineC4, "Database type: ");
			strcat(templineC4, ke_script_file);
			strcat(templineC4, " ");
			if (DB_type[0] == '_')
				strcat(templineC4, DB_type+1);
			else
				strcat(templineC4, DB_type);
			strcat(templineC4, " TD");
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);

			strcpy(templineC4, "Database date: ");
			strcat(templineC4, DB_version);
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
			
			strcpy(templineC4, "Database minimum utterances: ");
			strcat(templineC4, DB_MIN_UTTS);
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
		}
//		printArg(targv, targc, fpout, FALSE, "");
	}
	excelFooter(fpout);
	sp_head = freespeakers(sp_head);
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
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
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
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
	} else if (command == IPSYN_COM) {
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

static char isExcludeUtt(char *sp, char *line, char *word) {
	int i;

	i = 0;
	while ((i=getword(sp, line, word, NULL, i))) {
		if (word[0] == '[' && word[1] == '+') {
			removeExtraSpace(word);
			if (uS.mStricmp(word, "[+ exc]") == 0)
				return(TRUE);
		} else if (!uS.mStricmp(word, "www"))
			return(TRUE);
	}
	return(FALSE);
}

static void computeDSS_VOCD_IPSYN(int misalignmentError) {
	struct kideval_speakers *ts;

	gMisalignmentError += misalignmentError;
	if (!vocd_analize(FALSE)) {
		kideval_error(NULL, FALSE);
	}
	if (misalignmentError == 0) {
		if (!compute_ipsyn(FALSE)) {
			kideval_error(NULL, FALSE);
		}
	} else {
		if (mor_link.error_found) {
			mor_link.error_found = FALSE;
		}
	}
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, oldfname) == 0) {
			if (ts->dss_sp != NULL) {
				ts->dssUttCnt = ts->dss_sp->TotalNum;
				ts->dssGTotal = ts->dss_sp->GrandTotal;
				ts->dss_sp = NULL;
			} else {
				ts->dssUttCnt = 0.0;
				ts->dssGTotal = 0.0;
			}
			if (ts->vocd_sp != NULL) {
				ts->vocdOptimum = ts->vocd_sp->d_optimum;
				ts->vocd_sp = NULL;
			} else
				ts->vocdOptimum = 0.0;
			if (misalignmentError == 0 && ts->ipsyn_sp != NULL) {
				ts->ipsynUttCnt = (float)ts->ipsyn_sp->UttNum;
				ts->ipsynScore = ts->ipsyn_sp->TotalScore;
				ts->ipsyn_sp = NULL;
			} else {
				ts->ipsynUttCnt = 0.0;
				ts->ipsynScore = 0.0;
			}
		}
	}
	dss_freeSpeakers();
	ipsyn_freeSpeakers();
}

static int getAgeIndex(char *s) {
	int agef, aget, m_AgeAuto;
	char *age;

	m_AgeAuto = 0;
	s = strchr(s, '|');
	if (s != NULL) {
		s++;
		s = strchr(s, '|');
		if (s != NULL) {
			s++;
			s = strchr(s, '|');
			if (s != NULL) {
				age = s + 1;
				s = strchr(age, '|');
				if (s != NULL) {
					*s = EOS;
					uS.remblanks(age);
					if (isAge(age, &agef, &aget)) {
						m_AgeAuto = agef;
					}
					*s = '|';
				}
			}
		}
	}
	if (m_AgeAuto >= 18 && m_AgeAuto <= 23) {
		aget = 0; // 1;6-1;11
	} else if (m_AgeAuto >= 24 && m_AgeAuto <= 29) {
		aget = 1; // 2;-2;5
	} else if (m_AgeAuto >= 30 && m_AgeAuto <= 35) {
		aget = 2; // 2;6-2;11
	} else if (m_AgeAuto >= 36 && m_AgeAuto <= 41) {
		aget = 3; // 3;-3;5
	} else if (m_AgeAuto >= 42 && m_AgeAuto <= 47) {
		aget = 4; // 3;6-3;11
	} else if (m_AgeAuto >= 48 && m_AgeAuto <= 53) {
		aget = 5; // 4;-4;5
	} else if (m_AgeAuto >= 54 && m_AgeAuto <= 59) {
		aget = 6; // 4;6-4;11
	} else if (m_AgeAuto >= 60 && m_AgeAuto <= 65) {
		aget = 7; // 5;-5;5"
	} else if (m_AgeAuto >= 66 && m_AgeAuto <= 72) {
		aget = 8; // 5;6-6;
	} else if (m_AgeAuto == 0) {
		aget = 9; // age not found
	} else {
		aget = 10; //age out of range
	}
	return(aget);
}

void call() {
	int  i, command, misalignmentError = 0;
	char isPRCreateDBRes, isSpeakerFound, isDummy;
	char lRightspeaker, rightIDFound;
	char isGRA, isMOR;
	char word[BUFSIZ+1], IDText[BUFSIZ+1];
	time_t timer;
	struct tm *TmS;
	struct kideval_speakers *ts;
	struct IDparts IDTier;
	DSSSP *lastdsp;
	IPSYN_SP *lastipsp;
	extern int getc_cr_lc;
	extern char fgets_cr_lc, Tspchanged, contSpeaker[];

	isSpeakerFound = FALSE;
	isDummy = FALSE;
	IDText[0] = EOS;
	isMOR = FALSE;
	ts = NULL;
	clean_speakers(speakers, TRUE);
	ke_Design[0] = EOS;
	ke_Activity[0] = EOS;
	ke_Group[0] = EOS;
	if (isCreateDB) {
		if (dbfpout == NULL) {
			strcpy(FileName1, wd_dir);
			i = strlen(FileName1) - 1;
			if (FileName1[i] == PATHDELIMCHR)
				FileName1[i] = EOS;
/* 2023-03-08
			s = strrchr(FileName1, PATHDELIMCHR);
			if (s != NULL) {
				*s = EOS;
			}
*/
			addFilename2Path(FileName1, ke_script_file);
			strcat(FileName1, DB_type);
			strcat(FileName1, DATABASE_FILE_NAME);
			dbfpout = fopen(FileName1, "wb");
			if (dbfpout == NULL) {
				fprintf(stderr,"Can't create file \"%s\", perhaps path doesn't exist or it is opened by another application\n",FileName1);
				kideval_error(NULL, FALSE);
			}
			if (dbfpout != NULL) {
				time(&timer);
				TmS = localtime(&timer);
				fprintf(dbfpout,"V%d %.0f %d-%s%d-%s%d, %s%d:%s%d\n", KIDEVAL_DB_VERSION, DB_UTT_NUM_LIMIT, TmS->tm_year+1900,
						((TmS->tm_mon+1) < 10 ? "0":""), TmS->tm_mon+1, ((TmS->tm_mday) < 10 ? "0":""), TmS->tm_mday,
						((TmS->tm_hour) < 10 ? "0":""), TmS->tm_hour, ((TmS->tm_min) < 10 ? "0":""), TmS->tm_min);
			}
		}
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (utterance->speaker[0] == '*')
				break;
			if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (strncmp(utterance->line, "KIDEVAL DATABASE EXCLUDE", 21) == 0) {
					fprintf(stderr,"    EXCLUDED FILE: %s\n",oldfname+wdOffset);
					return;
				}
				for (i=0; utterance->line[i] != EOS; i++) {
					if (uS.mStrnicmp(utterance->line+i, "dummy file", 10) == 0) {
						isDummy = TRUE;
						break;
					}
				}
			} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					filterAndOuputID(utterance->line, IDText);
				}
			} else if (uS.partcmp(utterance->speaker,"@Types:",FALSE,FALSE)) {
				getTypes(utterance->line);
			}
		}
		if (uS.mStricmp(ke_Activity, "pictures") == 0 && uS.mStricmp(DB_type+1, "narrative") == 0 && uS.mStricmp(ke_Group, "TD") == 0) {
		} else if (uS.mStricmp(ke_Activity, DB_type+1) != 0 || uS.mStricmp(ke_Group, "TD") != 0) {
//			fprintf(stderr, "--- Wrong Type: %s, %s, %s\n	*** File \"%s\"\n", ke_Design, ke_Activity, ke_Group, oldfname+wdOffset);
			return;
		}
		if (isDummy == TRUE) {
			dummyFiles++;
			return;
		}
		if (IDText[0] == EOS) {
			agesFound[12]++;
			fprintf(stderr, "--- No CHI @ID:\n	*** File \"%s\"\n", oldfname+wdOffset);
			return;
		}
		i = getAgeIndex(IDText);
		 if (i == 9) {
			agesFound[9]++;
			fprintf(stderr, "--- Missing age in ID=%s in file:\n	*** File \"%s\"\n", IDText, oldfname+wdOffset);
			return;
		} else if (i == 10) {
			agesFound[10]++;
//			fprintf(stderr, "--- Wrong age in ID=%s in file:\n	*** File \"%s\"\n", IDText, oldfname+wdOffset);
			return;
		}

		rewind(fpin);
		fprintf(stdout, "~%s\n", oldfname+wdOffset);
//		return;
	} else {
		fprintf(stderr,"From file <%s>\n",oldfname);
		if (isLinkAge) {
			rightIDFound = FALSE;
			currentatt = 0;
			currentchar = (char)getc_cr(fpin, &currentatt);
			while (getwholeutter()) {
				if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
					breakIDsIntoFields(&IDTier, utterance->line);
					spareTier1[0] = EOS;
					if (IDTier.code[0] != '*')
						strcat(spareTier1, "*");
					strcat(spareTier1, IDTier.code);
					if (checktier(spareTier1)) {
						if (isKeyMatch(&IDTier)) {
							rightIDFound = TRUE;
							break;
						}
					}
				}
				if (utterance->speaker[0] == '*')
					break;
			}
			if (!rightIDFound) {
				fprintf(stderr,"\n*** File \"%s\"\n", oldfname);
				fprintf(stderr,"\tdoes not have a speaker that matches all chosen criteria.\n");
				cutt_exit(0);
//				return;
			}
			rewind(fpin);
		}
	}
	for (command=BASE_COM; command < LAST_COMMAND; command++) {
		if (command != BASE_COM) {
			rewind(fpin);
			init_s_option(command);
			getc_cr_lc = '\0';
			fgets_cr_lc = '\0';
			contSpeaker[0] = EOS;
			Tspchanged = FALSE;
			if (lineno > -1L) {
				tlineno = 1L;
				lineno = deflineno;
			}
		}

		lastdsp = NULL;
		lastipsp = NULL;
		isPRCreateDBRes = FALSE;
		spareTier1[0] = EOS;
		lRightspeaker = FALSE;
		misalignmentError = 0;
		isGRA = TRUE;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
/*
			if (lineno > tlineno) {
				tlineno = lineno + 200;
			}
*/
			if (!checktier(utterance->speaker)) {
				if (*utterance->speaker == '*') {
					lRightspeaker = FALSE;
					if (command == DSS_COM || command == IPSYN_COM) {
						cleanUttline(uttline);
						strcpy(spareTier1, uttline);
					}
				}
				continue;
			} else {
				if (*utterance->speaker == '*') {
					if (isExcludeUtt(utterance->speaker, utterance->line, word))
						lRightspeaker = FALSE;
					else
						lRightspeaker = TRUE;
				}
				if (!lRightspeaker && *utterance->speaker != '@')
					continue;
			}
			if (isCreateDB && command == LAST_COMMAND-1) {
				if (utterance->speaker[0] == '@') {
					uS.remFrontAndBackBlanks(utterance->line);
					removeExtraSpace(utterance->line);
				}
				if (uS.partcmp(utterance->speaker,"@End",FALSE,FALSE)) {
					computeDSS_VOCD_IPSYN(misalignmentError);
					if (isPRCreateDBRes) {
						if (ts->tUtt >= DB_UTT_NUM_LIMIT) {
							prTSResults(ts, IDText);
							fprintf(dbfpout, "-\n");
							i = getAgeIndex(IDText);
							if (i >= 0 && i < 9)
								agesFound[i]++;
							if (!isMOR)
								fprintf(stderr, "--- Missing %%MOR tier(s)\n	*** File \"%s\"\n", oldfname+wdOffset);
						} else {
//							fprintf(stderr, "--- <%.0f:\n	*** File \"%s\"\n", DB_UTT_NUM_LIMIT, oldfname+wdOffset);
							agesFound[11]++;
						}
						ke_Design[0] = EOS;
						ke_Activity[0] = EOS;
						ke_Group[0] = EOS;

						isPRCreateDBRes = FALSE;
						misalignmentError = 0;
					}
					if (ts != NULL) {
						if (ts->words_root != NULL)
							ts->words_root = kideval_freetree(ts->words_root);
						if (ts->NDW_root != NULL)
							ts->NDW_root = kideval_freetree(ts->NDW_root);
					}
				}
			}
			if (utterance->speaker[0] == '*') {
				isPRCreateDBRes = TRUE;
				isSpeakerFound = TRUE;
				strcpy(templineC, utterance->speaker);
				ts = kideval_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
			}
			if (fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE))
				isMOR = TRUE;
			if (command == BASE_COM) {
				kideval_process_tier(ts, NULL);
			} else if (command == DSS_COM) {
				if (strcmp(dss_script_file, DSSNOCOMPUTE) != 0) {
					lRightspeaker = kideval_process_dss_tier(ts, NULL, lRightspeaker, &lastdsp);
				}
			} else if (command == VOCD_COM) {
				kideval_process_vocd_tier(ts, NULL);
			} else if (command == IPSYN_COM) {
				if (strcmp(ipsyn_lang, IPSYNNOCOMPUTE) != 0) {
					if (utterance->speaker[0] == '*') {
						if (!isGRA) {
							fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
							fprintf(stderr,"--- Speaker tier does not have corresponding %%GRA tier.\n");
							break;
						}
					} else if (fDepTierName[0] != EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,FALSE)) {
						isGRA = FALSE;
					} else if (fDepTierName[0] == '%' && fDepTierName[1] == 'u' && fDepTierName[2] == 'm') {
						if (uS.partcmp(utterance->speaker,"%ugra:",FALSE,FALSE))
							isGRA = TRUE;
					} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
						isGRA = TRUE;
					}
					lRightspeaker = kideval_process_ipsyn_tier(ts, NULL, lRightspeaker, &misalignmentError, &lastipsp);
				}
			}
		}
	}
	if (isCreateDB == 0) {
		computeDSS_VOCD_IPSYN(misalignmentError);
	} else if (isSpeakerFound == FALSE && isDummy == FALSE) {
		fprintf(stderr, "--- No speakers found:\n	*** File \"%s\"\n", oldfname+wdOffset);
	}
}

static struct label_cols *kideval_addNewLabel(char *label) {
	int  cnt;
	struct label_cols *p;

	cnt = 0;
	if (labelsRoot == NULL) {
		labelsRoot = NEW(struct label_cols);
		p = labelsRoot;
	} else {
		for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
			cnt++;
		}
		p->next_label = NEW(struct label_cols);
		p = p->next_label;
		cnt++;
	}
	if (p == NULL) {
		kideval_error(NULL, TRUE);
	}
	p->next_label = NULL;
	p->num = cnt;
	colLabelsNum++;
	if (label == NULL)
		p->label = NULL;
	else {
		templineC2[0] = EOS;
		if (!isRawVal) {
			strcpy(templineC2, "%_");
			strcat(templineC2, label);
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

static PATS *makePatsElems(COLS *p, char ch) {
	PATS *pats;

	if (p->pats_elem == NULL) {
		p->pats_elem = NEW(PATS);
		pats = p->pats_elem;
		pats->next_pats = NULL;
	} else {
		if (ch == 'e') {
			pats = NEW(PATS);
			if (pats == NULL)
				kideval_error(NULL, TRUE);
			pats->next_pats = p->pats_elem;
			p->pats_elem = pats;
		} else {
			for (pats=p->pats_elem; pats->next_pats != NULL; pats=pats->next_pats) ;
			pats->next_pats = NEW(PATS);
			pats = pats->next_pats;
			if (pats == NULL)
				kideval_error(NULL, TRUE);
			pats->next_pats = NULL;
		}
	}
	pats->pats1 = NULL;
	pats->pats2 = NULL;
	return(pats);
}

static COLS *kideval_add2col(COLS *root, char isFirstElem, char *pat, const FNType *mFileName, int ln) {
	int  i;
	char ch, res, *pat2;
	COLS *p;
	PATS *pats_elem;

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
		p->pats_elem = NULL;
		p->labelP = NULL;
		p->isClitic = FALSE;
	} else
		for (p=root; p->next_col != NULL; p=p->next_col) ;
	if (pat[0] == '"' || pat[0] == '\'') {
		for (i=0; pat[i] != EOS && (pat[i] == '"' || pat[i] == '\'' || isSpace(pat[i])); i++) ;
		if (pat[i] != EOS) {
			if (p->labelP != NULL && p->labelP->label != NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
				fprintf(stderr, "    ERROR: only one label is allow for each line: \"%s\"\n", pat);
				fprintf(stderr, "    Perhaps this label name is used on another line\n");
				kideval_error(gdb, FALSE);
			}
			p->labelP = kideval_addNewLabel(pat+i);
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
			ch = 0;
			fprintf(stderr,"\n*** File \"%s\": line %d.\n", mFileName, ln);
			fprintf(stderr, "    ERROR: Illegal character \"%c\" found on line \"%s\"\n", pat[i], pat);
			fprintf(stderr, "    Expected characters are: + - \" '\n\n");
			kideval_error(gdb, FALSE);
		}
		if (pat[i] == 's' || pat[i] == 'S')
			i++;
		if (pat[i] == '@')
			i++;
		if (strchr(pat+i, '~') != NULL || strchr(pat+i, '$') != NULL)
			p->isClitic = TRUE;
		pat2 = strchr(pat+i, '^');
		if (pat2 != NULL)
			*pat2 = EOS;
		pats_elem = makePatsElems(p, ch);
		pats_elem->pats1 = makeMorSeachList(pats_elem->pats1, &res, pat+i, ch);
		if (pat2 != NULL)
			pats_elem->pats2 = makeMorSeachList(pats_elem->pats2, &res, pat2+1, ch);
		if (pat2 != NULL) {
			*pat2 = '^';
		}
		if (pats_elem->pats1 == NULL || (pat2 != NULL && pats_elem->pats2 == NULL)) {
			if (res == TRUE)
				kideval_error(gdb, TRUE);
			else if (res == FALSE) {
				kideval_error(gdb, FALSE);
			}
		}
	}
	return(root);
}

static void processScriptLine(const FNType *mFileName, int  ln, const char *line) {
	char *b, *e, t, isQT, isFirstElem;

	if (line != NULL) // DO NOT REMOVE THIS LINE
		strcpy(templineC, line);
	if (uS.mStrnicmp(templineC, "dss ", 4) == 0) {
		b = templineC + 4;
		e = b;
		isQT = '\0';
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
					if (strlen(templineC1+2) > 250) {
						fprintf(stderr, "This language is too long (250): %d.\n", strlen(templineC1+2));
						fprintf(stderr, "Choose: eng - English, bss - English, jpn - Japanese\n");
						cutt_exit(0);
					} else
						strcpy(dss_script_file, templineC1+2);
					uS.lowercasestr(dss_script_file, &dFnt, C_MBF);
					if (*dss_script_file == 'b' || *dss_script_file == 'e' || *dss_script_file == 'j' ||
						*dss_script_file == '0') {
					} else {
						fprintf(stderr,"*** File \"%s\": line: \"%d\"\n", mFileName, ln);
						fprintf(stderr,"    Illegal option argument found: %s\n", templineC1);
						fprintf(stderr,"    Please specify either \"+le\" for English or \"+lj\" for Japanese.\n");
						cutt_exit(0);
					}
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
	} else if (uS.mStrnicmp(templineC, "ipsyn ", 6) == 0) {
		b = templineC + 6;
		e = b;
		isQT = '\0';
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
					if (templineC1[2] == '0') {
						strcpy(ipsyn_lang, IPSYNNOCOMPUTE);
					} else {
						strncpy(ipsyn_lang, templineC1+2, 256);
						ipsyn_lang[256] = EOS;
					}
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
	} else {
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
					colsRoot = kideval_add2col(colsRoot, isFirstElem, templineC1, mFileName, ln);
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
}

// fllowed by word rule: +|sconj^|verb,-Inf,-S

static void processEng(void) {
	processScriptLine("Eng", 0, "dss +leng");
	processScriptLine("Eng", 0, "ipsyn +leng");
	processScriptLine("Eng", 0, "+-PRESP		\"*-PRESP\"");
	processScriptLine("Eng", 0, "+;in		\"in\"");
	processScriptLine("Eng", 0, "+;on		\"on\"");
	processScriptLine("Eng", 0, "+-PL		\"*-PL\"");
	processScriptLine("Eng", 0, "+&PAST		\"irr-PAST\"");
	processScriptLine("Eng", 0, "+-POSS		\"*-POSS\"");
	processScriptLine("Eng", 0, "+|cop -|*,~|cop	\"u-cop\"");
	processScriptLine("Eng", 0, "+|det:art		\"det:art\"");
	processScriptLine("Eng", 0, "+-PAST		\"*-PAST\"");
	processScriptLine("Eng", 0, "+-3S		\"*-3S\"");
	processScriptLine("Eng", 0, "+&3S -|cop,;be,&3S -|aux,;be,&3S	\"irr-3S\"");
	processScriptLine("Eng", 0, "+|aux -|*,~|aux	\"u-aux\"");
	processScriptLine("Eng", 0, "+|*,~|cop	\"c-cop\"");
	processScriptLine("Eng", 0, "+|*,~|aux	\"c-aux\"");
}

static void processEngU(void) {
	processScriptLine("EngU", 0, "dss +lengu");
//	processScriptLine("EngU", 0, "ipsyn +leng");
	processScriptLine("EngU", 0, "ipsyn +l0");
	processScriptLine("EngU", 0, "+-Part,-Pres +-Ger	\"*-PRESP\"");
	processScriptLine("EngU", 0, "+;in		\"in\"");
	processScriptLine("EngU", 0, "+;on		\"on\"");
	processScriptLine("EngU", 0, "+-Plur		\"*-PL\"");
	processScriptLine("EngU", 0, "+|*,~|part,;s		\"*-POSS\"");
	processScriptLine("EngU", 0, "+|aux,;be -|*,~|aux,;be	\"u-cop\"");
	processScriptLine("EngU", 0, "+|det,-Art		\"det:art\"");
	processScriptLine("EngU", 0, "+-Past		\"*-PAST\"");
	processScriptLine("EngU", 0, "+-S3 -;be -;do -|pron -|aux --Past	\"*-S3\"");
	processScriptLine("EngU", 0, "+-S3,;be +-S3,;do -|pron -|aux	\"irr-S3\"");
	processScriptLine("EngU", 0, "+|aux -|*,~|aux -|aux,;be	\"u-aux\"");
	processScriptLine("EngU", 0, "+|*,~|aux,;be	\"c-cop\"");
	processScriptLine("EngU", 0, "+|*,~|aux -|*,~|aux,;be	\"c-aux\"");
}

static void processFra(void) {
	processScriptLine("Fra", 0, "dss +l0");
	processScriptLine("Fra", 0, "ipsyn +l0");
	processScriptLine("Fra", 0, "+-Part,-Pres	\"-Part,-Pres\"");
	processScriptLine("Fra", 0, "+;en	\"en\"");
	processScriptLine("Fra", 0, "+;à	\"à\"");
	processScriptLine("Fra", 0, "+|noun,-Plur	\"noun|-Plur\"");
	processScriptLine("Fra", 0, "+|pron,-Rel	\"pron|-Rel\"");
	processScriptLine("Fra", 0, "+;de	\"de\"");
	processScriptLine("Fra", 0, "+|verb,;être	\"verb,;être\"");
	processScriptLine("Fra", 0, "+-Det,-Art	\"-Det,-Art\"");
	processScriptLine("Fra", 0, "+|verb,-Past	\"verb|-Past\"");
	processScriptLine("Fra", 0, "+|verb,-3S	\"verb|-3S\"");
	processScriptLine("Fra", 0, "+|aux,-3S	\"aux|-3S\"");
	processScriptLine("Fra", 0, "+|aux,;être	\"aux,;être\"");
	processScriptLine("Fra", 0, "+|verb,-Sub	\"verb|-Sub\"");
	processScriptLine("Fra", 0, "+|pron,;y	\"pron,;y\"");

/* 2023-01-03
	processScriptLine("Fra", 0, "+-PP +&PP	\"PP\"");
	processScriptLine("Fra", 0, "+-PRES +&PRES	\"PRES\"");
	processScriptLine("Fra", 0, "+-IMP +&IMP	\"IMP\"");
	processScriptLine("Fra", 0, "+-IMPF +&IMPF	\"IMPF\"");
	processScriptLine("Fra", 0, "+-FUT +&FUT	\"FUT\"");
	processScriptLine("Fra", 0, "+-COND +&COND	\"COND\"");
	processScriptLine("Fra", 0, "+-SUB +&SUB	\"SUB\"");
	processScriptLine("Fra", 0, "+-INF	\"INF\"");
	processScriptLine("Fra", 0, "+-PPRF	\"PPRF\"");
	processScriptLine("Fra", 0, "+&1s	\"irr-1s\"");
	processScriptLine("Fra", 0, "+&2s	\"irr-2s\"");
	processScriptLine("Fra", 0, "+&3s	\"irr-3s\"");
	processScriptLine("Fra", 0, "+&13s	\"irr-13s\"");
	processScriptLine("Fra", 0, "+&12s	\"irr-12s\"");
	processScriptLine("Fra", 0, "+&m	\"irr-m\"");
	processScriptLine("Fra", 0, "+&f	\"irr-f\"");
	processScriptLine("Fra", 0, "+&pl +-PL	\"PL\"");
	processScriptLine("Fra", 0, "+&sg +-SG	\"SG\"");
	processScriptLine("Fra", 0, "+&f,&pl	\"irr-f_irr-pl\"");
	processScriptLine("Fra", 0, "+&m,&pl	\"irr-m_irr-pl\"");
	processScriptLine("Fra", 0, "+&f,&sg	\"irr-f_irr-sg\"");
	processScriptLine("Fra", 0, "+&m,&sg	\"irr-m_irr-sg\"");
	processScriptLine("Fra", 0, "+|pro	\"pro\"");
	processScriptLine("Fra", 0, "+|adv	\"adv\"");
	processScriptLine("Fra", 0, "+|adv:int	\"adv:int\"");
	processScriptLine("Fra", 0, "+|adv:place	\"adv:place\"");
	processScriptLine("Fra", 0, "+|co	\"co\"");
	processScriptLine("Fra", 0, "+|adj	\"adj\"");
	processScriptLine("Fra", 0, "+|n	\"n\"");
	processScriptLine("Fra", 0, "+|n:prop	\"n:prop\"");
	processScriptLine("Fra", 0, "+|num	\"num\"");
	processScriptLine("Fra", 0, "+|v	\"v\"");
	processScriptLine("Fra", 0, "+|v:mdllex	\"v:mdllex\"");
	processScriptLine("Fra", 0, "+|det:art	\"det:art\"");
	processScriptLine("Fra", 0, "+|det:dem	\"det:dem\"");
	processScriptLine("Fra", 0, "+|det:int	\"det:int\"");
	processScriptLine("Fra", 0, "+|det:gen	\"det:gen\"");
	processScriptLine("Fra", 0, "+|det:poss	\"det:poss\"");
	processScriptLine("Fra", 0, "+|prep	\"prep\"");
	processScriptLine("Fra", 0, "+|pro:ind	\"pro:ind\"");
	processScriptLine("Fra", 0, "+|pro:obj	\"pro:obj\"");
	processScriptLine("Fra", 0, "+|pro:refl	\"pro:refl\"");
	processScriptLine("Fra", 0, "+|pro:rel	\"pro:rel\"");
	processScriptLine("Fra", 0, "+|pro:sub	\"pro:sub\"");
	processScriptLine("Fra", 0, "+|pro:dem	\"pro:dem\"");
	processScriptLine("Fra", 0, "+|pro:int	\"pro:int\"");
	processScriptLine("Fra", 0, "+|pro:y	\"pro:y\"");
	processScriptLine("Fra", 0, "+|v:aux	\"v:aux\"");
	processScriptLine("Fra", 0, "+|v:mdl	\"v:mdl\"");
	processScriptLine("Fra", 0, "+|v:exist	\"v:exist\"");
	processScriptLine("Fra", 0, "+|v:poss	\"v:poss\"");
	processScriptLine("Fra", 0, "+|conj	\"conj\"");
	processScriptLine("Fra", 0, "+|adv:neg	\"adv:neg\"");
*/
}

static void processNld(void) {
	processScriptLine("Nld", 0, "dss +l0");
	processScriptLine("Nld", 0, "ipsyn +l0");
	processScriptLine("Nld", 0, "+-Part,-Pres	\"-Part,-Pres\"");
	processScriptLine("Nld", 0, "+-Part,-Past	\"-Part,&Past\"");
	processScriptLine("Nld", 0, "+-Def	\"-Def\"");
	processScriptLine("Nld", 0, "+|noun,-Plur	\"noun|-Plur\"");
	processScriptLine("Nld", 0, "+|pron,-Rel	\"pron|-Rel\"");
	processScriptLine("Nld", 0, "+|pron,-Gen	\"pron|-Gen\"");
	processScriptLine("Nld", 0, "+|pron,-Dat	\"pron|-Dat\"");
	processScriptLine("Nld", 0, "+|pron,-Int	\"pron|-Int\"");
	processScriptLine("Nld", 0, "+|aux,-Past	\"aux|-Past\"");
	processScriptLine("Nld", 0, "+|aux,-Part	\"aux|-Part\"");
	processScriptLine("Nld", 0, "+|verb,-Inf	\"verb|-Inf\"");
	processScriptLine("Nld", 0, "+|verb,-Past	\"verb|-Past\"");
	processScriptLine("Nld", 0, "+-Cmp	\"-Cmp\"");
	processScriptLine("Nld", 0, "+-Sup	\"-Sup\"");
}
static void processSpa(void) {
	processScriptLine("Spa", 0, "dss +l0");
	processScriptLine("Spa", 0, "ipsyn +l0");
	processScriptLine("Spa", 0, "+-Part,-Pres	\"-Part,-Pres\"");
	processScriptLine("Spa", 0, "+;en	\"en\"");
	processScriptLine("Spa", 0, "+;a	\"a\"");
	processScriptLine("Spa", 0, "+|noun,-Plur	\"noun|-Plur\"");
	processScriptLine("Spa", 0, "+|pron,-Rel	\"pron|-Rel\"");
	processScriptLine("Spa", 0, "+;de	\"de\"");
	processScriptLine("Spa", 0, "+|pron,-IntRel	\"pron|-IntRel\"");
	processScriptLine("Spa", 0, "+-Det,-Art	\"-Det,-Art\"");
	processScriptLine("Spa", 0, "+|verb,-Past	\"verb|-Past\"");
	processScriptLine("Spa", 0, "+|verb,-3S	\"verb|-3S\"");
	processScriptLine("Spa", 0, "+|aux,-3S	\"aux|-3S\"");
	processScriptLine("Spa", 0, "+|adj,-Sup	\"adj|-Sup\"");
	processScriptLine("Spa", 0, "+|verb,-Fut	\"verb|-Fut\"");
	processScriptLine("Spa", 0, "+|verb,-Ger	\"verb|-Ger\"");
/* 2023-01-03
	processScriptLine("Spa", 0, "+-1S	\"*-1S\"");
	processScriptLine("Spa", 0, "+-2S	\"*-2S\"");
	processScriptLine("Spa", 0, "+-3S	\"*-3S\"");
	processScriptLine("Spa", 0, "+-1P	\"*-1P\"");
	processScriptLine("Spa", 0, "+-2P	\"*-2P\"");
	processScriptLine("Spa", 0, "+-3P	\"*-3P\"");
	processScriptLine("Spa", 0, "+-INF	\"*-INF\"");
	processScriptLine("Spa", 0, "+&IMP	\"irr-IMP\"");
	processScriptLine("Spa", 0, "+-PROG	\"*-PROG\"");
	processScriptLine("Spa", 0, "+&PRES	\"irr-PRES\"");
	processScriptLine("Spa", 0, "+&PRET	\"irr-PRET\"");
	processScriptLine("Spa", 0, "+&FUT	\"irr-FUT\"");
	processScriptLine("Spa", 0, "+&COND	\"irr-COND\"");
	processScriptLine("Spa", 0, "+&SUB,&PRES	\"irr-SUB_irr-PRES\"");
	processScriptLine("Spa", 0, "+&SUB,&PAS	\"irr-SUB_irr-PAS\"");
	processScriptLine("Spa", 0, "+|pro:obj	\"|pro:obj\"");
	processScriptLine("Spa", 0, "+|pro:refl	\"|pro:refl\"");
	processScriptLine("Spa", 0, "+|pro:clit	\"|pro:clit\"");
*/
}

static void processZho(void) {
	processScriptLine("Zho", 0, "dss +l0");
	processScriptLine("Zho", 0, "ipsyn +l0");
	processScriptLine("Zho", 0, "+;过	\"过\"");
	processScriptLine("Zho", 0, "+;了	\"了\"");
	processScriptLine("Zho", 0, "+;着	\"着\"");
	processScriptLine("Zho", 0, "+|part,;的	\"part,;的\"");
	processScriptLine("Zho", 0, "+|part,;得	\"part,;得\"");
	processScriptLine("Zho", 0, "+|adv,;在	\"adv,;在\"");
	processScriptLine("Zho", 0, "+|adp,;在	\"adp,;在\"");
	processScriptLine("Zho", 0, "+|verb,;给	\"verb,;给\"");
	processScriptLine("Zho", 0, "+|num,;多	\"num,;多\"");
	processScriptLine("Zho", 0, "+|adj,;多	\"adj,;多\"");
	processScriptLine("Zho", 0, "+|pron,;啥	\"pron,;啥\"");
	processScriptLine("Zho", 0, "+|pron,;谁	\"pron,;谁\"");
	processScriptLine("Zho", 0, "+|noun,;后	\"noun,;后\"");
	processScriptLine("Zho", 0, "+|noun,;里	\"noun,;里\"");
/* 2024-01-11
	processScriptLine("Zho", 0, "+|n	\"n\"");
	processScriptLine("Zho", 0, "+|v,|v:*	\"v\"");
	processScriptLine("Zho", 0, "+|adv	\"adv\"");
	processScriptLine("Zho", 0, "+|cl	\"cl\"");
	processScriptLine("Zho", 0, "+|conj	\"conj\"");
	processScriptLine("Zho", 0, "+|prep	\"prep\"");
*/
}

static void processJpn(void) {
	processScriptLine("Jpn", 0, "dss +ljpn");
	processScriptLine("Jpn", 0, "ipsyn +l0");
	processScriptLine("Jpn", 0, "+|noun				\"n\"");
	processScriptLine("Jpn", 0, "+|verb				\"v\"");
	processScriptLine("Jpn", 0, "+|adj				\"adj\"");
	processScriptLine("Jpn", 0, "+|adv				\"adv\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;有る,-Inf,-S +|sconj,;て^|verb,;来る,-Inf,-S +|sconj,;て^|verb,;行く,-Inf,-S +|sconj,;て^|verb,;上げる,-Inf,-S +|sconj,;て^|verb,;置く,-Inf,-S +|sconj,;て^|verb,;見る,-Inf,-S +|sconj,;て^|verb,;貰う,-Inf,-S +|sconj,;て^|aux,;下さる,-Inf,-S	\"v:sub\"");
	processScriptLine("Jpn", 0, "+|aux,;だ,-Inf,-S		\"v:cop\"");
	processScriptLine("Jpn", 0, "+|sconj,;の			\"p-snrの\"");
	processScriptLine("Jpn", 0, "+|sconj,;から +|sconj,;ので +|sconj,;と +|sconj,;のに +|sconj,;まで +|sconj,;より	\"p-conj\"");
	processScriptLine("Jpn", 0, "+|cconj,;だ^|sconj,;から +|cconj,;だ^|sconj,;けど +|cconj,;で +|cconj,;もし +|cconj,;そして +|cconj,;じゃ +|cconj,;でも	\"conj\"");
	processScriptLine("Jpn", 0, "+|adp,;が +|adp,;を		\"p-case\"");
	processScriptLine("Jpn", 0, "+|adp,;に +|adp,;で +|adp,;から +|adp,;まで +|adp,;へ +|adp,;と +|adp,;より	\"p-post\"");
	processScriptLine("Jpn", 0, "+|adp,;は			\"p-top\"");
	processScriptLine("Jpn", 0, "+|adp,;って +|adp,;と		\"p-quot\"");
	processScriptLine("Jpn", 0, "+|adp,;の			\"p-attrの\"");
	processScriptLine("Jpn", 0, "+|adp,;しか +|adp,;だけ +|adp,;ばかり +|adp,;ぐらい +|adp,;ほど +|adp,;こそ +|adp,;も +|adp,;とか +|adp,;とか +|adp,;ずつ		\"p-foc\"");
	processScriptLine("Jpn", 0, "+|part,;よ +|part,;ね +|part,;ぞ +|part,;か +|part,;な +|part,;の +|part,;っけ +|part,;さあ +|part,;ぜ	\"p-final\"");
	processScriptLine("Jpn", 0, "+|adp,;は			\"1_p-topは\"");
	processScriptLine("Jpn", 0, "+|adp,;の			\"1歳後半_p-attrの\"");
	processScriptLine("Jpn", 0, "+|aux,;た			\"1歳後半_V-PASTた\"");
	processScriptLine("Jpn", 0, "+|sconj,;て			\"1歳後半_V-IMP:te\""); //	% not separated from connective"
	processScriptLine("Jpn", 0, "+|part,;が			\"2歳前半_p-caseが\"");
	processScriptLine("Jpn", 0, "+|adp,;で			\"2歳前半_p-postで\"");
	processScriptLine("Jpn", 0, "+|adp,;に			\"2歳前半_p-postに\"");
	processScriptLine("Jpn", 0, "+|adp,;と			\"2歳前半_p-postと\"");
	processScriptLine("Jpn", 0, "+|adp,;も			\"2歳前半_p-focも\"");
	processScriptLine("Jpn", 0, "+|adp,;って			\"2歳前半_p-quotって\"");
	processScriptLine("Jpn", 0, "+|aux,;ちゃう,-Inf,-S		\"2歳前半_COMPLちゃう\"");
	processScriptLine("Jpn", 0, "+|aux,;てる			\"2歳前半_ASPてる\"");
	processScriptLine("Jpn", 0, "+|aux,;ない,-Inf,-Neg,-S		\"2歳前半_NEGない\"");
	processScriptLine("Jpn", 0, "+|aux,;られる,-Inf,-S		\"2歳後半_POTれる\""); // %? Potential not separated from passive
	processScriptLine("Jpn", 0, "+|aux,;たい,-Inf,-S		\"2歳後半_DESIDたい\"");
	processScriptLine("Jpn", 0, "+|aux,;ます +|aux,;です		\"2歳後半_POLます\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;来る,-Inf,-S	\"2歳後半_v:subてくる\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;行く,-Inf,-S	\"2歳後半_v:subていく\"");
	processScriptLine("Jpn", 0, "+|adp,;を			\"3歳前半_p-caseを\"");
	processScriptLine("Jpn", 0, "+|adp,;から			\"3歳前半_p-postから\"");
	processScriptLine("Jpn", 0, "+|adp,;まで			\"3歳前半_p-postまで\"");
	processScriptLine("Jpn", 0, "+|adp,;だけ +|part,;だけ		\"3歳前半_p-focだけ\"");
	processScriptLine("Jpn", 0, "+|adp,;と			\"3歳前半_p-quotと\""); //  % not separated from to \"with\" and
	processScriptLine("Jpn", 0, "+|aux,;様,-Inf,-S		\"3歳前半_V-HORTよう\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;上げる,-Inf,-S	\"3歳後半_v:subてあげる\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;有る,-Inf,-S	\"3歳後半_v:subてある\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;置く,-Inf,-S	\"3歳後半_v:subておく\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;見る,-Inf,-S	\"3歳後半_v:subてみる\"");
	processScriptLine("Jpn", 0, "+|sconj,;ば			\"4歳後半_V-CONDば\"");
	processScriptLine("Jpn", 0, "+|aux,;為さる,-Inf,-S		\"4歳後半_V-IMPなさい\"");
	processScriptLine("Jpn", 0, "+|aux,;させる,-Inf,-S		\"4歳後半_CAUSさせる\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|aux,;下さる,-Inf,-S	\"4歳後半_v:subてください\"");
	processScriptLine("Jpn", 0, "+|sconj,;て^|verb,;貰う,-Inf,-S	\"4歳後半_v:subてもらう\"");
}

static void read_script(char *lang) {
	int  ln;
	char *b;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*lang == EOS) {
		fprintf(stderr,	"No language specified with +l option.\n");
		fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
		fprintf(stderr,"For example, \"kideval +leng\" or \"kideval +leng.cut\".\n");
		cutt_exit(0);
	}
	if (uS.mStricmp(lang, "eng") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "eng");
		processEng();
		return;
	} else if (uS.mStricmp(lang, "engu") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "engu");
		processEngU();
		return;
	} else if (uS.mStricmp(lang, "fra") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "fra");
		processFra();
		return;
	} else if (uS.mStricmp(lang, "nld") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "nld");
		processNld();
		return;
	} else if (uS.mStricmp(lang, "spa") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "spa");
		processSpa();
		return;
	} else if (uS.mStricmp(lang, "zho") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "zho");
		processZho();
		return;
	} else if (uS.mStricmp(lang, "jpn") == 0) {
		fprintf(stderr,"    Using default, build into KIDEVAL source code, language file: %s\n", "jpn");
		processJpn();
		return;
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
			if (uS.mStricmp(b, ".cut") == 0 || uS.mStricmp(b, ".txt") == 0) {
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
#ifdef _MAC_CODE
		fprintf(stderr, "\n");
#else
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
#endif
		cutt_exit(0);
	}
	fprintf(stderr,"    Using language file: %s\n", mFileName);
	ln = 0;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		ln++;
		if (templineC[0] == '%' || templineC[0] == '#')
			continue;
		if ((b=strchr(templineC, '%')) != NULL) {
			if (b != templineC && isSpace(*(b-1))) {
				*b = EOS;
			}
		}
		uS.remFrontAndBackBlanks(templineC);
		if (templineC[0] != EOS)
			processScriptLine(mFileName, ln, NULL);
	}
	fclose(fp);
/*
	if (colsRoot == NULL) {
		fprintf(stderr,"Can't find any usable declarations in language script file \"%s\".\n", mFileName);
		cutt_exit(0);
	}
*/
/*
struct label_cols *p;
for (p=labelsRoot; p->next_label != NULL; p=p->next_label) {
	fprintf(stdout, "%s: %d\n", p->label, p->num);
}
*/
}

void init(char first) {
	int  i;
	struct tier *nt;
	FNType debugfile[FNSize];

	if (first) {
		ftime = TRUE;
		isLinkAge = FALSE;
		DB_version[0] = EOS;
		isRawVal = TRUE;
		ke_script_file = NULL;
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
		dbfpout = NULL;
		isDBFilesList = FALSE;
		DBFilesListFP = NULL;
		strcpy(fDepTierName, "%mor:");
		sp_head = NULL;
		colLabelsNum = 0;
		colsRoot = NULL;
		labelsRoot = NULL;
		gdb = NULL;
		DBKeyRoot = NULL;
		if (isCreateDB) {
			stout = TRUE;
			isRecursive = TRUE;
		}
		MinUttLimit = 0;
		DssIpsynUttLimit = 0;
		combinput = TRUE;
		gMisalignmentError = 0;
		if (!init_dss(first))
			kideval_error(NULL, TRUE);
		init_vocd(first);
		if (!init_ipsyn(first, 50))
			kideval_error(NULL, FALSE);
	} else {
		if (ftime) {
			ftime = FALSE;
			if (ke_script_file == NULL) {
				fprintf(stderr,"\nPlease specify language script file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"kideval +leng\" or \"kideval +leng.cut\".\n");
				cutt_exit(0);
			}
			if (DBKeyRoot != NULL && strcmp(ke_script_file, "eng") && strcmp(ke_script_file, "engu") &&
				strcmp(ke_script_file, "fra") && strcmp(ke_script_file, "nld") &&
				strcmp(ke_script_file, "spa") && strcmp(ke_script_file, "zho") &&
				strcmp(ke_script_file, "jpn")) {
				fprintf(stderr,"\nThe database can only be used with \"eng\", \"engu\", \"fra\", \"nld\", \"spa\", \"zho\" or \"jpn\" languages.\n");
				cutt_exit(0);
			}
			read_script(ke_script_file);
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
			if (DssIpsynUttLimit > 0 && DBKeyRoot != NULL) {
				fprintf(stderr,"\n+qN option can not be used with +d option, database.\n");
				cutt_exit(0);
			}
			if (isDBFilesList && DBKeyRoot == NULL) {
				fprintf(stderr,"\n+e1 option can only be used with +d option.\n");
				cutt_exit(0);
			}
			if (isCreateDB) {
				if (strcmp(ke_script_file, "eng") && strcmp(ke_script_file, "engu") &&
					strcmp(ke_script_file, "fra") && strcmp(ke_script_file, "nld") && 
					strcmp(ke_script_file, "spa") && strcmp(ke_script_file, "zho") &&
					strcmp(ke_script_file, "jpn")) {
					fprintf(stderr,"\nThe database can only be used with \"eng\", \"engu\", \"fra\", \"nld\", \"spa\", \"zho\" or \"jpn\" languages.\n");
					cutt_exit(0);
				}
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
				gdb = NEW(struct database);
				if (gdb == NULL)
					kideval_error(gdb, TRUE);
				gdb->morItems_sqr = (float *)malloc(sizeof(float) * colLabelsNum);
				if (gdb->morItems_sqr == NULL) {
					kideval_error(gdb, TRUE);
				}
				gdb->morItems = (float *)malloc(sizeof(float) * colLabelsNum);
				if (gdb->morItems == NULL) {
					kideval_error(gdb, TRUE);
				}
				init_db(gdb);
				ParseDatabase(gdb, ke_script_file);
				CleanUpTempIDSpeakers();
				isRawVal = TRUE;
			}
			for (i=1; i < targc; i++) {
				if (*targv[i] == '+' || *targv[i] == '-') {
					if (targv[i][1] == 't') {
						if (targv[i][2] == '%')
							strcpy(fDepTierName, targv[i]+2);
						else
							maingetflag(targv[i], NULL, &i);
					}
				}
			}	
			for (nt=headtier; nt != NULL; nt=nt->nexttier) {
				if (nt->tcode[0] == '*')
					break;
			}
			if (nt == NULL && SPRole == NULL) {
				fprintf(stderr, "\nPlease specify at least one speaker tier code with \"+t\" option.\n");
				cutt_exit(0);
			}
			if (!init_dss(first))
				kideval_error(NULL, FALSE);
			if (!init_ipsyn(first, 50))
				kideval_error(NULL, FALSE);
// 2019-04-24
			if (DssIpsynUttLimit > 0) {
				DSS_UTTLIM = DssIpsynUttLimit;
				IPS_UTTLIM = DssIpsynUttLimit;
			}
// 2019-04-24
		} else
			init_s_option(BASE_COM);
		init_vocd(first);
	}
}

static int addFilePath(const char *filePath, int argc, char *argv[]) {
	strcpy(templineC, wd_dir);
	addFilename2Path(templineC, filePath);
	argv[argc] = (char *)malloc(strlen(templineC)+1);
	if (argv[argc] == NULL) {
		kideval_error(NULL, TRUE);
	}
	strcpy(argv[argc], templineC);
	argc++;
	return(argc);
}

static int fillUpEngTdFilePaths(int argc, char *argv[]) {
	argc = addFilePath("childes-data/Eng-NA/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/English-ECSC/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/English-MiamiBiling/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/English-MiamiMono/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/English-Slobin/*.cha", argc, argv);
	argc = addFilePath("childes-data/Clinical-Eng/*.cha", argc, argv);
	return(argc);
}

static int fillUpFraTdFilePaths(int argc, char *argv[]) {
	argc = addFilePath("childes-data/French/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/French-Duguine/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/French-Lyon/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/French-MTLN/*.cha", argc, argv);
	return(argc);
}

static int fillUpNldTdFilePaths(int argc, char *argv[]) {
	argc = addFilePath("childes-data/DutchAfrikaans/Asymmetries/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/DeHouwer/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/DeHouwerBornstein-protect/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/Gillis/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/Groningen/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/Schaerlaekens/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/SchlichtingVanKampen/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/VanKampen/*.cha", argc, argv);
	argc = addFilePath("childes-data/DutchAfrikaans/Wijnen/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Dutch-AarssenBos/*.cha", argc, argv);
	return(argc);
}

static int fillUpSpaTdFilePaths(int argc, char *argv[]) {
	argc = addFilePath("childes-data/Spanish/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Spanish-Aguilar/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Spanish-MiamiBiling/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Spanish-Ornat/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Spanish-Sebastian/*.cha", argc, argv);
	return(argc);
}

static int fillUpZhoMdFilePaths(int argc, char *argv[]) {
	argc = addFilePath("childes-data/Chinese/Mandarin/AcadLang/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Chinese/Mandarin/Beijing/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Chang1/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Chang2/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/ChangPlay/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/ChangPN/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Erbaugh/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/LiZhou/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/NSCtoys/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/TCCM/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Tong/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Zhou1/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Zhou2/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/Zhou3/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/ZhouAssessment/*.cha", argc, argv);
	argc = addFilePath("childes-data/Chinese/Mandarin/ZhouNarratives/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Chinese-Chang1/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Chinese-Chang2/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Chinese-Guo/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Chinese-Tardif/*.cha", argc, argv);
	argc = addFilePath("childes-data/Frogs/Chinese-Zhou/*.cha", argc, argv);
	return(argc);
}

static int fillUpJpnTdFilePaths(int argc, char *argv[]) {
//	argc = addFilePath("childes-data/Japanese/*.cha", argc, argv);

	argc = addFilePath("childes-data/Japanese/Hamasaki/*.cha", argc, argv);
	argc = addFilePath("childes-data/Japanese/Miyata/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Ogawa/*.cha", argc, argv);
	argc = addFilePath("childes-data/Japanese/Okayama/*.cha", argc, argv);
	argc = addFilePath("childes-data/Japanese/Yokoyama/*.cha", argc, argv);

//	argc = addFilePath("childes-data/Japanese/Miyata/Aki/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Miyata/Ryo/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Miyata/Tai/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Noji/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Ogawa/*.cha", argc, argv);
//	argc = addFilePath("childes-data/Japanese/Yokoyama/*.cha", argc, argv);

	return(argc);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
#ifdef UNX
	int len;
#endif
	int i, j, fileArgcStart, fileArgcEnd, langArgc, spArgc;
	time_t t;
	char *cOption, *lOption, langPref[32], spPref[128];
	char isFileGiven, isLangGiven, isSpGiven;

#if defined(_MAC_CODE) || defined(_WIN32)
	targv = argv;
#endif
#ifdef UNX
	for (i=0; i < argc; i++) {
		targv[i] = argv[i];
	}
	argv = targv;
#endif
	for (i=0; i < 13; i++) {
		agesFound[i] = 0L;
	}
	dummyFiles = 0L;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = KIDEVAL;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
#ifdef DEBUGEVALRESULTS
	dbResults = NULL;
#endif
	cOption = NULL;
	lOption = NULL;
	DBKeyRoot = NULL;
	DBFilesListFP = NULL;
	isCreateDB = 0;
	isFileGiven = FALSE;
	isLangGiven = FALSE;
	isSpGiven = FALSE;
	DB_UTT_NUM_LIMIT = 50.0;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+' || *argv[i] == '-') {
			if (argv[i][1] == 'c') {
				isCreateDB = 1;
				j = 2;
				if (isdigit(argv[i][j])) {
					DB_UTT_NUM_LIMIT = (float)atoi(argv[i]+j);
					for (; isdigit(argv[i][j]) || argv[i][j] == '.'; j++) ;
				}
				cOption = argv[i]+j;
			} else if (argv[i][1] == 't') {
				if (argv[i][2] == '*')
					isSpGiven = TRUE;
				else if (argv[i][2] == '@' && argv[i][3] == 'I' && argv[i][4] == 'D' && argv[i][5] == '=')
					isSpGiven = TRUE;
				else if (argv[i][2] == '#')
					isSpGiven = TRUE;
			} else if (argv[i][1] == 'l') {
				isLangGiven = TRUE;
				lOption = argv[i]+2;
#ifdef UNX
			} else if (argv[i][1] == 'L') {
				strcpy(lib_dir, argv[i]+2);
				len = strlen(lib_dir);
				if (len > 0 && lib_dir[len-1] != '/')
					strcat(lib_dir, "/");
#endif
			}
		} else
			isFileGiven = TRUE;
	}
	fileArgcStart = 0;
	fileArgcEnd = 0;
	spArgc = 0;
	langArgc = 0;
//	CRDBcnt = 0;
	DB_type[0] = EOS;
	wdOffset = strlen(wd_dir);
	if (wd_dir[wdOffset-1] != PATHDELIMCHR)
		wdOffset++;
	if (argc > 1) {
		if (!isSpGiven) {
			spArgc = argc;
			strcpy(spPref, "+t*CHI:");
			strcpy(spPref, "+t#Target_Child"); // 2019-12-18
			argv[spArgc] = spPref;
			argc++;
		}
		if (!isLangGiven) {
			langArgc = argc;
			strcpy(langPref, "+leng");
			argv[langArgc] = langPref;
			argc++;
		}
		if (isCreateDB) {
			if (!isLangGiven) {
				fprintf(stderr, "\nSpecify language with \"-l\" option.\n");
				cutt_exit(0);
			} else {
				if (cOption == NULL) {
					fprintf(stderr, "\nSpecify database type with \"+c\" option.\n");
					cutt_exit(0);
				} else if (strcmp(cOption, "_toyplay") == 0 || strcmp(cOption, "_narrative") == 0 ||
						   strcmp(cOption, "_md") == 0 || strcmp(cOption, "_td") == 0) {
					strcpy(DB_type, cOption);
				} else {
					fprintf(stderr, "\nUnknown database type specified with \"-c%s\" option.\n", cOption);
					cutt_exit(0);
				}
			}
			if (!isFileGiven) {
				fileArgcStart = argc;
				if (strcmp(lOption, "eng") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpEngTdFilePaths(argc, argv);
				else if (strcmp(lOption, "eng") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpEngTdFilePaths(argc, argv);
				else if (strcmp(lOption, "engu") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpEngTdFilePaths(argc, argv);
				else if (strcmp(lOption, "engu") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpEngTdFilePaths(argc, argv);
				else if (strcmp(lOption, "fra") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpFraTdFilePaths(argc, argv);
				else if (strcmp(lOption, "fra") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpFraTdFilePaths(argc, argv);
				else if (strcmp(lOption, "nld") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpNldTdFilePaths(argc, argv);
				else if (strcmp(lOption, "nld") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpNldTdFilePaths(argc, argv);
				else if (strcmp(lOption, "spa") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpSpaTdFilePaths(argc, argv);
				else if (strcmp(lOption, "spa") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpSpaTdFilePaths(argc, argv);
				else if (strcmp(lOption, "zho") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpZhoMdFilePaths(argc, argv);
				else if (strcmp(lOption, "zho") == 0 && strcmp(DB_type, "_narrative") == 0)
					argc = fillUpZhoMdFilePaths(argc, argv);
				else if (strcmp(lOption, "jpn") == 0 && strcmp(DB_type, "_toyplay") == 0)
					argc = fillUpJpnTdFilePaths(argc, argv);
				else {
					fprintf(stderr, "\nUnknown database type specified with \"-l%s\" and \"-c%s\" and options.\n",
							lOption, cOption);
					cutt_exit(0);
				}
				fileArgcEnd = argc;
			}
		}
	}
	targc = argc;
	debug_dss_fp = stdout;
#ifdef _CLAN_DEBUG
	t = 1283562804L; // RAND
#else
	t = time(NULL);
#endif
	srand(t);    // seed the random number generator
	VOCD_TYPE_D = FALSE;
	bmain(argc,argv,kideval_pr_result);

	printf("\n");
#ifdef UNX
	printf("IF YOU DID NOT MARK ERRORS, DSS TOTAL WILL BE INFLATED.\n");
	printf("Please be sure to mark utterances with errors with [*] to ensure accurate DSS computation.\n\n");
#else
	printf("%c%cIF YOU DID NOT MARK ERRORS, DSS TOTAL WILL BE INFLATED.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cPlease be sure to mark utterances with errors with [*] to ensure accurate DSS computation.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	if (DBKeyRoot != NULL) {
#ifdef UNX
		printf("PLEASE SEE SLP MANUAL FOR RECOMMENDED INTERPRETATION.\n");
#else
		printf("%c%cPLEASE SEE SLP MANUAL FOR RECOMMENDED INTERPRETATION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	}
	if (DBFilesListFP != NULL)
		fclose(DBFilesListFP);
	gdb = freedb(gdb);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	dss_cleanSearchMem();
	ipsyn_cleanSearchMem();
	labelsRoot = freeLabels(labelsRoot);
	colsRoot = freeColsP(colsRoot);
	if (gMisalignmentError != 0) {
		fprintf(stderr,"\nWARNING: FOUND %d %%mor: TIERS THAT DO NOT LINK IN SIZE TO THEIR SPEAKER TIERS.\n", gMisalignmentError);
		fprintf(stderr,"THIS WILL EFFECT THE RESULT OF IPSyn ANALYSES.\n");
	}
	if (isCreateDB) {
		fprintf(stderr,"\nDONE CREATING DATABASE.\n");
	}
	if (dbfpout != NULL) {
//		fprintf(stderr,"Output file <%s>\n",FileName1);
		fclose(dbfpout);
	}
	if (spArgc != 0) {
		argv[spArgc] = NULL;
		argc--;
	}
	if (langArgc != 0) {
		argv[langArgc] = NULL;
		argc--;
	}
	if (isCreateDB) {
		for (i=fileArgcStart; i < fileArgcEnd; i++) {
			if (argv[i] != NULL) {
				free(argv[i]);
				argv[i] = NULL;
			}
			argc--;
		}
		fprintf(stderr, "files dummy @Comments:   %ld\n", dummyFiles);
		fprintf(stderr, "\n%s files:\n", DB_type);
		fprintf(stderr, "files for ages 1;6-1;11: %ld\n", agesFound[0]);
		fprintf(stderr, "files for ages 2;-2;5:    %ld\n", agesFound[1]);
		fprintf(stderr, "files for ages 2;6-2;11: %ld\n", agesFound[2]);
		fprintf(stderr, "files for ages 3;-3;5:    %ld\n", agesFound[3]);
		fprintf(stderr, "files for ages 3;6-3;11: %ld\n", agesFound[4]);
		fprintf(stderr, "files for ages 4;-4;5:    %ld\n", agesFound[5]);
		fprintf(stderr, "files for ages 4;6-4;11: %ld\n", agesFound[6]);
		fprintf(stderr, "files for ages 5;-5;5:     %ld\n", agesFound[7]);
		fprintf(stderr, "files for ages 5;6-6:      %ld\n", agesFound[8]);
		fprintf(stderr, "files right age but <%.0f Utts: %ld\n", DB_UTT_NUM_LIMIT, agesFound[11]);
		fprintf(stderr, "files missing age:        %ld\n", agesFound[9]);
		fprintf(stderr, "files out of range age:   %ld\n", agesFound[10]);
		fprintf(stderr, "files NO CHI @ID:        %ld\n", agesFound[12]);
	}
#ifdef DEBUGEVALRESULTS
	if (dbResults != NULL)
		fclose(dbResults);
#endif
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'a':
			MinUttLimit = atoi(f);
			break;
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
			break;
		case 'd':
			if (*f == EOS) {
				fprintf(stderr,"Missing argument for option: %s\n", f-2);
				cutt_exit(0);
			} else {
				t = strchr(f, '~');
				if (t != NULL) {
					*t = EOS;
					strcpy(DB_type, "_");
					strcat(DB_type, f);
					*t = '~';
					f = t + 1;
					if (*f == EOS) {
						fprintf(stderr,"Missing argument for option: %s\n", f-2);
						cutt_exit(0);
					}
				} else {
					fprintf(stderr,"Missing database type argument for option: %s\n", f-2);
					cutt_exit(0);
				}
				addDBKeys(f);
			}
			break;
		case 'e':
			if (*f == '1')
				isDBFilesList = TRUE;
			else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'l':
			ke_script_file = f;
			break;
		case 'L':
			if (strcmp(f-1, "LinkAge") == 0) {
				isLinkAge = TRUE;
			}
#ifdef UNX
			else {
				strcpy(lib_dir, f);
				j = strlen(lib_dir);
				if (j > 0 && lib_dir[j-1] != '/')
					strcat(lib_dir, "/");
			}
#endif
			break;
		case 'o':
			if (*f == '4') {
				isRawVal = FALSE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'q':
			DssIpsynUttLimit = atoi(f);
			break;
		case 's':
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
			} else {
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
