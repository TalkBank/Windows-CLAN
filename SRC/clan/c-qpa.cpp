/**********************************************************************
 "Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
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
 initTSVars
 wID
   wnum;
   wused;
 db_sp
 sp_head
 c_qpa_FindSpeaker
*/


/* debugging
#define DEBUGC_QPARESULTS
 prDebug
*/

#define CHAT_MODE 1
#include "cu.h"
#include <math.h>
#include "c_curses.h"
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main c_qpa_main
#define call c_qpa_call
#define getflag c_qpa_getflag
#define init c_qpa_init
#define usage c_qpa_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define C_QPA_DB_VERSION 1

#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "_c-qpa_db.cut"
#define DATABSEFILESLIST "0c-qpa_Database_IDs.cex"

extern char R8;
extern char OverWriteFile;
extern char outputOnlyData;
extern char isRecursive;
extern char isLanguageExplicit;
extern char isPostcliticUse;	/* if '~' found on %mor tier break it into two words */
extern char isPrecliticUse;		/* if '$' found on %mor tier break it into two words */
extern char org_depTier[];
extern struct tier *defheadtier;
extern struct IDtype *IDField;

#define SDRESSIZE 256

struct VIR {
	char *word;
	struct VIR *nextVIR;
} ;

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct c_qpa_words {
	char *word;
	int  wnum;
	char wused;
	struct c_qpa_words *left;
	struct c_qpa_words *right;
};

struct c_qpa_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;
	int  wID;

	float tm;
	float tUtt;   // Total Utts
	float totalWords;// (A)
	float openClass; // (B)
	float closedClass; // # Closed Class Words
	float closedClassP;// Proportion Closed Class
	float noun;    // (C)
	float NRDs;    // (D)
	float NRDwD;   // (E)
	float detI;    // DET Index
	float pron;    // (F)
	float pronP;   // Proportion Pronouns
	float verb;    // (G)
	float verbP;   // Proportion Verbs
	float inverb;  // (H)
	float inverbI; // (J)
	float infIP;   // Inflection Index
	float matVerb; // (K)
	float auxScore;// (L) AUX Score
	float auxComP; // Aux Complexity
	float mSs;     // (M)
	float nWSs;    // (N)
	float pWSsP;   // Proportion Words in Ss
	float wfSs;    // (Q)
	float pWFSsP;  // Proportion Well Formed Ss
	float SNPs;    // (R)
	float ocSNP;   // (S)
	float mSNP;    // Mean SNP Length
	float VPs;     // (T)
	float ocVP;    // (U)



	float embedd;  // (V)
	float embedP;  // Embedding index
	float numUtt;  // (W)
	float uttLenP; // Mean Utterance Length

//ANALYSIS WORKSHEET beg
	float aw_sentenceUtts; // Sentence Utterance (1)
	float aw_otherUtts; // Other Utterance (enter)
	float aw_totalWords; // #Narrative Wds
	float aw_openClass; // #Open Class Wds
	float aw_noun;    // #Nouns
	float aw_NRDs;    // #Ns Req DET (NRDs)
	float aw_NRDwD;   // #NRDs w/DETs
	float aw_pron;    // #Pronouns
	float aw_verb;    // #Verbs
	float aw_inverb;  // #Inflectable Vs (IVs)
	float aw_inverbI; // #IVs Inflected
	float aw_matVerb; // #Matrix Verbs
	float aw_auxScore;// AUX Score
	float aw_embedd; // #Embeddings
	float aw_swf;    // S Well-formed?
	float aw_PSNP;   // # Phrases SNP
	float aw_PVP;    // # Phrases VP
	float aw_ocSNP;  // # Open Class Words + Pron SNP
	float aw_ocVP;   // # Open Class Words + Pron VP
	float aw_nWSs;   // # Wds in Ss
//ANALYSIS WORKSHEET end


	struct c_qpa_speakers *next_sp;
} ;

/*
	c_qpa_speakers	database

	initTSVars	init_db

	retrieveTS		prDebug		prTSResults

	c_qpa_process_tier

 	OpenDBFile

	c_qpa_pr_result
	HEADERs
	"%.0f", ts->...
	"+/-SD"
	"Mean Database"
*/
struct database {
	float num;
	float tm_sqr, tm;
	float WOD_sqr, WOD;
	float tUtt_sqr, tUtt;
	float totalWords_sqr, totalWords;
	float mluWords_sqr, mluWords;
	float mluUtt_sqr, mluUtt;
	float openClass_sqr, openClass;
	float openClassP_sqr, openClassP;
	float closedClass_sqr, closedClass;	/* total number of words */
	float closedClassP_sqr, closedClassP;	/* total number of words */
	float noun_sqr, noun;
	float NRDs_sqr, NRDs;
	float NRDwD_sqr, NRDwD;
	float detI_sqr, detI;
	float pron_sqr, pron;
	float pronP_sqr, pronP;
	float verb_sqr, verb;
	float verbP_sqr, verbP;
	float inverb_sqr, inverb;
	float inverbI_sqr, inverbI;
	float infIP_sqr, infIP;
	float matVerb_sqr, matVerb;
	float auxScore_sqr, auxScore;
	float auxComP_sqr, auxComP;
	float mSs_sqr, mSs;
	float nWSs_sqr, nWSs;
	float pWSsP_sqr, pWSsP;
	float wfSs_sqr, wfSs;
	float pWFSsP_sqr, pWFSsP;
	float SNPs_sqr, SNPs;
	float ocSNP_sqr, ocSNP;
	float mSNP_sqr, mSNP;
	float VPs_sqr, VPs;
	float ocVP_sqr, ocVP;





	float embedd_sqr, embedd;
	float embedP_sqr, embedP;
	float numUtt_sqr, numUtt;
	float uttLenP_sqr, uttLenP;
	struct c_qpa_speakers *db_sp;
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
static int  c_qpa_SpecWords, DBGemNum, wdOffset;
static char *DBGems[NUMGEMITEMS];
static const char *lang_prefix;
static char isSpeakerNameGiven, isExcludeWords, ftime, isPWordsList, isDBFilesList,
			isCreateDB, specialOptionUsed, isCountMSs, isDebugMode;
static char c_qpa_BBS[5], c_qpa_CBS[5], c_qpa_group, c_qpa_n_option, isNOptionSet, onlyApplyToDB,
			isGOptionSet, GemMode;
static char *last_utt; //ANALYSIS WORKSHEET
static float tmDur;
static FILE	*dbfpout;
static FILE *DBFilesListFP;
static struct DBKeys *DBKeyRoot;
static struct c_qpa_speakers *sp_head;
static struct database *db;
static struct VIR *virs;

static struct c_qpa_speakers *freespeakers(struct c_qpa_speakers *p) {
	struct c_qpa_speakers *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
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

static struct VIR *freeVIR(struct VIR *p) {
	struct VIR *t;

	while (p) {
		t = p;
		p = p->nextVIR;
		if (t->word != NULL)
			free(t->word);
		free(t);
	}
	return(NULL);
}

static void c_qpa_error(struct database *db, char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	virs = freeVIR(virs);
	sp_head = freespeakers(sp_head);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	db = freedb(db);
	cutt_exit(0);
}

static char *extractFloatFromLine(char *line, float *val, struct database *db) {
	float num;

	if (*line == EOS) {
		fprintf(stderr, "\nERROR: Database is corrupt\n");
		c_qpa_error(db, FALSE);
	}
	num = atof(line);
	*val = *val + num;
	while (isdigit(*line) || *line == '.')
		line++;
	while (isSpace(*line))
		line++;
	return(line);
}

static void initTSVars(struct c_qpa_speakers *ts, char isAll) {
	ts->isMORFound	= FALSE;
	ts->isPSDFound	= FALSE;
	ts->tm			= (float)0.0;
	ts->tUtt		= (float)0.0;
	ts->totalWords	= (float)0.0;
	ts->openClass	= (float)0.0;
	ts->closedClass = (float)0.0;
	ts->closedClassP= (float)0.0;
	ts->noun		= (float)0.0;
	ts->NRDs		= (float)0.0;
	ts->NRDwD		= (float)0.0;
	ts->detI		= (float)0.0;
	ts->pron		= (float)0.0;
	ts->pronP		= (float)0.0;
	ts->verb		= (float)0.0;
	ts->verbP		= (float)0.0;
	ts->inverb		= (float)0.0;
	ts->inverbI		= (float)0.0;
	ts->infIP		= (float)0.0;
	ts->matVerb		= (float)0.0;
	ts->auxScore	= (float)0.0;
	ts->auxComP		= (float)0.0;
	ts->mSs			= (float)0.0;
	ts->nWSs		= (float)0.0;
	ts->pWSsP		= (float)0.0;
	ts->wfSs		= (float)0.0;
	ts->pWFSsP		= (float)0.0;
	ts->SNPs		= (float)0.0;
	ts->ocSNP		= (float)0.0;
	ts->mSNP		= (float)0.0;
	ts->VPs			= (float)0.0;
	ts->ocVP		= (float)0.0;




	ts->embedd		= (float)0.0;
	ts->embedP		= (float)0.0;
	ts->numUtt		= (float)0.0;
	ts->uttLenP		= (float)0.0;

//ANALYSIS WORKSHEET beg
	ts->aw_sentenceUtts= (float)0.0;
	ts->aw_otherUtts= (float)0.0;
	ts->aw_totalWords= (float)0.0;
	ts->aw_openClass= (float)0.0;
	ts->aw_noun		= (float)0.0;
	ts->aw_NRDs		= (float)0.0;
	ts->aw_NRDwD	= (float)0.0;
	ts->aw_pron		= (float)0.0;
	ts->aw_verb		= (float)0.0;
	ts->aw_inverb	= (float)0.0;
	ts->aw_inverbI	= (float)0.0;
	ts->aw_matVerb	= (float)0.0;
	ts->aw_auxScore	= (float)0.0;
	ts->aw_embedd	= (float)0.0;
	ts->aw_swf		= (float)0.0;
	ts->aw_PSNP		= (float)0.0;
	ts->aw_ocSNP	= (float)0.0;
	ts->aw_PVP		= (float)0.0;
	ts->aw_ocVP		= (float)0.0;
	ts->aw_nWSs		= (float)0.0;
//ANALYSIS WORKSHEET end

	if (isAll) {
		ts->wID = 0;
	}
}

static void init_db(struct database *p) {
	p->num = 0.0;
	p->tm_sqr = (float)0.0; p->tm = (float)0.0;
	p->WOD_sqr = (float)0.0; p->WOD = (float)0.0;
	p->tUtt_sqr = (float)0.0; p->tUtt = (float)0.0;
	p->totalWords_sqr = (float)0.0; p->totalWords = (float)0.0;
	p->mluWords_sqr = (float)0.0; p->mluWords = (float)0.0;
	p->mluUtt_sqr = (float)0.0; p->mluUtt = (float)0.0;
	p->openClass_sqr = (float)0.0; p->openClass = (float)0.0;
	p->openClassP_sqr = (float)0.0; p->openClassP = (float)0.0;
	p->closedClass_sqr = (float)0.0; p->closedClass = (float)0.0;
	p->closedClassP_sqr = (float)0.0; p->closedClassP = (float)0.0;
	p->noun_sqr = (float)0.0; p->noun = (float)0.0;
	p->NRDs_sqr = (float)0.0; p->NRDs = (float)0.0;
	p->NRDwD_sqr = (float)0.0; p->NRDwD = (float)0.0;
	p->detI_sqr = (float)0.0; p->detI = (float)0.0;
	p->pron_sqr = (float)0.0; p->pron = (float)0.0;
	p->pronP_sqr = (float)0.0; p->pronP = (float)0.0;
	p->verb_sqr = (float)0.0; p->verb = (float)0.0;
	p->verbP_sqr = (float)0.0; p->verbP = (float)0.0;
	p->inverb_sqr = (float)0.0; p->inverb = (float)0.0;
	p->inverbI_sqr = (float)0.0; p->inverbI = (float)0.0;
	p->infIP_sqr = (float)0.0; p->infIP = (float)0.0;
	p->matVerb_sqr = (float)0.0; p->matVerb = (float)0.0;
	p->auxScore_sqr = (float)0.0; p->auxScore = (float)0.0;
	p->auxComP_sqr = (float)0.0; p->auxComP = (float)0.0;
	p->mSs_sqr = (float)0.0; p->mSs = (float)0.0;
	p->nWSs_sqr = (float)0.0; p->nWSs = (float)0.0;
	p->pWSsP_sqr = (float)0.0; p->pWSsP = (float)0.0;
	p->wfSs_sqr = (float)0.0; p->wfSs = (float)0.0;
	p->pWFSsP_sqr = (float)0.0; p->pWFSsP = (float)0.0;
	p->SNPs_sqr = (float)0.0; p->SNPs = (float)0.0;
	p->ocSNP_sqr = (float)0.0; p->ocSNP = (float)0.0;
	p->mSNP_sqr = (float)0.0; p->mSNP = (float)0.0;
	p->VPs_sqr = (float)0.0; p->VPs = (float)0.0;
	p->ocVP_sqr = (float)0.0; p->ocVP = (float)0.0;




	p->embedd_sqr = (float)0.0; p->embedd = (float)0.0;
	p->embedP_sqr = (float)0.0; p->embedP = (float)0.0;
	p->numUtt_sqr = (float)0.0; p->numUtt = (float)0.0;
	p->uttLenP_sqr = (float)0.0; p->uttLenP = (float)0.0;
	p->db_sp = NULL;
}

static void retrieveTS(struct c_qpa_speakers *ts, char *line, struct database *db) {
	while (isSpace(*line))
		line++;

	line = extractFloatFromLine(line, &ts->tm, db);
	line = extractFloatFromLine(line, &ts->tUtt, db);
	line = extractFloatFromLine(line, &ts->totalWords, db);
	line = extractFloatFromLine(line, &ts->openClass, db);
	line = extractFloatFromLine(line, &ts->closedClass, db);
	line = extractFloatFromLine(line, &ts->noun, db);
	line = extractFloatFromLine(line, &ts->NRDs, db);
	line = extractFloatFromLine(line, &ts->NRDwD, db);
	line = extractFloatFromLine(line, &ts->detI, db);
	line = extractFloatFromLine(line, &ts->pron, db);
	line = extractFloatFromLine(line, &ts->pronP, db);
	line = extractFloatFromLine(line, &ts->verb, db);
	line = extractFloatFromLine(line, &ts->verbP, db);
	line = extractFloatFromLine(line, &ts->inverb, db);
	line = extractFloatFromLine(line, &ts->inverbI, db);
	line = extractFloatFromLine(line, &ts->infIP, db);
	line = extractFloatFromLine(line, &ts->matVerb, db);
	line = extractFloatFromLine(line, &ts->auxScore, db);
	line = extractFloatFromLine(line, &ts->auxComP, db);
	line = extractFloatFromLine(line, &ts->mSs, db);
	line = extractFloatFromLine(line, &ts->nWSs, db);
	line = extractFloatFromLine(line, &ts->pWSsP, db);
	line = extractFloatFromLine(line, &ts->wfSs, db);
	line = extractFloatFromLine(line, &ts->pWFSsP, db);
	line = extractFloatFromLine(line, &ts->SNPs, db);
	line = extractFloatFromLine(line, &ts->ocSNP, db);
	line = extractFloatFromLine(line, &ts->mSNP, db);
	line = extractFloatFromLine(line, &ts->VPs, db);
	line = extractFloatFromLine(line, &ts->ocVP, db);





	line = extractFloatFromLine(line, &ts->embedd, db);
	line = extractFloatFromLine(line, &ts->embedP, db);
	line = extractFloatFromLine(line, &ts->numUtt, db);
	line = extractFloatFromLine(line, &ts->uttLenP, db);
}

#ifdef DEBUGC_QPARESULTS
static void prDebug(struct c_qpa_speakers *ts) {
	fprintf(stdout, "tm=%.0f\n", ts->tm);
	fprintf(stdout, "tUtt=%.0f\n", ts->tUtt);
	fprintf(stdout, "totalWords=%.0f\n", ts->totalWords);
	fprintf(stdout, "openClass=%.0f\n", ts->openClass);
	fprintf(stdout, "closedClass=%.0f\n", ts->closedClass);
	fprintf(stdout, "noun=%.0f\n", ts->noun);
	fprintf(stdout, "NRDs=%.0f\n", ts->NRDs);
	fprintf(stdout, "NRDwD=%.0f\n", ts->NRDwD);
	fprintf(stdout, "detI=%.0f\n", ts->detI);
	fprintf(stdout, "pron=%.0f\n", ts->pron);
	fprintf(stdout, "pronP=%.0f\n", ts->pronP);
	fprintf(stdout, "verb=%.0f\n", ts->verb);
	fprintf(stdout, "verbP=%.0f\n", ts->verbP);
	fprintf(stdout, "inverb=%.0f\n", ts->inverb);
	fprintf(stdout, "inverbI=%.0f\n", ts->inverbI);
	fprintf(stdout, "infIP=%.0f\n", ts->infIP);
	fprintf(stdout, "matVerb=%.0f\n", ts->matVerb);
	fprintf(stdout, "auxScore=%.0f\n", ts->auxScore);
	fprintf(stdout, "auxComP=%.0f\n", ts->auxComP);
	fprintf(stdout, "mSs=%.0f\n", ts->mSs);
	fprintf(stdout, "nWSs=%.0f\n", ts->nWSs);
	fprintf(stdout, "pWSsP=%.0f\n", ts->pWSsP);
	fprintf(stdout, "nWSs=%.0f\n", ts->wfSs);
	fprintf(stdout, "pWFSsP=%.0f\n", ts->pWFSsP);
	fprintf(stdout, "SNPs=%.0f\n", ts->SNPs);
	fprintf(stdout, "ocSNP=%.0f\n", ts->ocSNP);
	fprintf(stdout, "mSNP=%.0f\n", ts->mSNP);
	fprintf(stdout, "VPs=%.0f\n", ts->VPs);
	fprintf(stdout, "ocVP=%.0f\n", ts->ocVP);




	fprintf(stdout, "embedd=%.0f\n", ts->embedd);
	fprintf(stdout, "embedP=%.0f\n", ts->embedP);
	fprintf(stdout, "numUtt=%.0f\n", ts->numUtt);
	fprintf(stdout, "uttLenP=%.0f\n", ts->uttLenP);
}
#endif

static void prTSResults(struct c_qpa_speakers *ts) {
	const char *format;

	format = "%.0f ";
	//	format = "%f ";
	fprintf(dbfpout, format, ts->tm);
	fprintf(dbfpout, format, ts->tUtt);
	fprintf(dbfpout, format, ts->totalWords);
	fprintf(dbfpout, format, ts->openClass);
	fprintf(dbfpout, format, ts->closedClass);
	fprintf(dbfpout, format, ts->noun);
	fprintf(dbfpout, format, ts->NRDs);
	fprintf(dbfpout, format, ts->NRDwD);
	fprintf(dbfpout, format, ts->detI);
	fprintf(dbfpout, format, ts->pron);
	fprintf(dbfpout, format, ts->pronP);
	fprintf(dbfpout, format, ts->verb);
	fprintf(dbfpout, format, ts->verbP);
	fprintf(dbfpout, format, ts->inverb);
	fprintf(dbfpout, format, ts->inverbI);
	fprintf(dbfpout, format, ts->infIP);
	fprintf(dbfpout, format, ts->matVerb);
	fprintf(dbfpout, format, ts->auxScore);
	fprintf(dbfpout, format, ts->auxComP);
	fprintf(dbfpout, format, ts->mSs);
	fprintf(dbfpout, format, ts->nWSs);
	fprintf(dbfpout, format, ts->pWSsP);
	fprintf(dbfpout, format, ts->wfSs);
	fprintf(dbfpout, format, ts->pWFSsP);
	fprintf(dbfpout, format, ts->SNPs);
	fprintf(dbfpout, format, ts->ocSNP);
	fprintf(dbfpout, format, ts->mSNP);
	fprintf(dbfpout, format, ts->VPs);
	fprintf(dbfpout, format, ts->ocVP);




	fprintf(dbfpout, format, ts->embedd);
	fprintf(dbfpout, format, ts->embedP);
	fprintf(dbfpout, format, ts->numUtt);
	fprintf(dbfpout, format, ts->uttLenP);
	putc('\n', dbfpout);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		c_qpa_error(NULL, FALSE);
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

static char *c_qpa_strsave(const char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		c_qpa_error(NULL, FALSE);
	return(p);
}

static struct DBKeys *initDBKey(const char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		c_qpa_error(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = c_qpa_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = c_qpa_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = c_qpa_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = c_qpa_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = c_qpa_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = c_qpa_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = c_qpa_strsave("Wernicke");
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
		DBKey->key1 = c_qpa_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = c_qpa_strsave("Wernicke");
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
					DBKey->key1 = c_qpa_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = c_qpa_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = c_qpa_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = c_qpa_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = c_qpa_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					c_qpa_error(NULL, FALSE);
				}
				DBKey->age = c_qpa_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control, \n");
				fprintf(stderr, "    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),\n");
				fprintf(stderr, "    Nonfluent = (Global, Broca, Transmotor),\n");
				fprintf(stderr, "    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)\n");
				c_qpa_error(NULL, FALSE);
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

static struct c_qpa_speakers *c_qpa_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	struct c_qpa_speakers *ts, *tsp;

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
			c_qpa_error(db, FALSE);
		}
	}
	if ((ts=NEW(struct c_qpa_speakers)) == NULL)
		c_qpa_error(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		c_qpa_error(db, TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		c_qpa_error(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL || isCreateDB)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			c_qpa_error(db, TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	initTSVars(ts, TRUE);
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

static void outputAnalysisSheet(FILE *AnalysisSheetFP, char *last_utt, struct c_qpa_speakers *ts) {
	int i;

	for (i=0; last_utt[i] != EOS; i++) {
		if (last_utt[i] == '\n')
			last_utt[i] = ' ';
	}
	removeExtraSpace(last_utt);
	excelRow(AnalysisSheetFP, ExcelRowStart);
	excelStrCell(AnalysisSheetFP, last_utt);  // UTTERANCES
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_sentenceUtts);// Sentence Utterance (1)
//	excelNumCell(AnalysisSheetFP, "%.0f", 1.0); // TC Utterance (1)
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_otherUtts);   // Other Utterance (enter)
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_totalWords);  // #Narrative Wds
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_openClass);   // #Open Class Wds
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_noun);   // #Nouns
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_NRDs);   // #Ns Req DET (NRDs)
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_NRDwD);  // #NRDs w/DETs
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_pron);   // #Pronouns
	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_verb);   // #Verbs
//	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_inverb); // #Inflectable Vs (IVs)
//	excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_inverbI);// #IVs Inflected
	if (ts->aw_sentenceUtts != 0.0) {
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_matVerb);// #Matrix Verbs
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_auxScore);// AUX Score
//		excelStrCell(AnalysisSheetFP, "don't care"); // AUX Token
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_embedd); // #Embeddings
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_swf);  // S Well-formed?
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_PSNP); // #Phrases SNP
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_PVP); // #Phrases VP
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_ocSNP);// # Open Class Words + Pron SNP
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_ocVP); // # Open Class Words + Pron VP
		excelNumCell(AnalysisSheetFP, "%.0f", ts->aw_nWSs); // # Wds in Ss
	} else {
		excelStrCell(AnalysisSheetFP, "N/A"); // #Matrix Verbs
		excelStrCell(AnalysisSheetFP, "N/A"); // AUX Score
//		excelStrCell(AnalysisSheetFP, "N/A"); // AUX Token
		excelStrCell(AnalysisSheetFP, "N/A"); // #Embeddings
		excelStrCell(AnalysisSheetFP, "N/A"); // S Well-formed?
		excelStrCell(AnalysisSheetFP, "N/A"); // #Phrases SNP
		excelStrCell(AnalysisSheetFP, "N/A"); // #Phrases VP
		excelStrCell(AnalysisSheetFP, "N/A"); // # Open Class Words + Pron SNP
		excelStrCell(AnalysisSheetFP, "N/A"); // # Open Class Words + Pron VP
		excelStrCell(AnalysisSheetFP, "N/A"); // # Wds in Ss
	}
//	excelStrCell(AnalysisSheetFP, "don't care"); // # Wds in TCs (auto. filled)
	excelStrCell(AnalysisSheetFP, "");        // Comments
	excelRow(AnalysisSheetFP, ExcelRowEnd);
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

static void isCountUtt(char *line, char *morline, struct c_qpa_speakers *ts) {
	int i;
	char isCountwfSs;

	isCountwfSs = TRUE;
/*
	i = 0;
	while ((i=getword("%mor:", morline, spareTier2, NULL, i))) {
		if (isWordFromMORTier(spareTier2)) {
		}
		POS = strchr(spareTier2, '#');
		if (POS != NULL)
			POS++;
		else
			POS = spareTier2;
		if (uS.mStrnicmp(POS, "pro|", 4) == 0) {
		}
	}
*/
	i = 0;
	while ((i=getword("*:", line, spareTier2, NULL, i))) {
		if (spareTier2[0] == '[' && spareTier2[1] == '+') {
			removeExtraSpace(spareTier2);
			if (uS.mStricmp(spareTier2, "[+ gram]") == 0) {
				isCountwfSs = FALSE;
			}
		} else if (uS.mStricmp(spareTier2, "+...") == 0 || uS.mStricmp(spareTier2, "+..?") == 0 ||
				   uS.mStricmp(spareTier2, "+//.") == 0 || uS.mStricmp(spareTier2, "+//?") == 0 ||
				   uS.mStricmp(spareTier2, "+/.") == 0  || uS.mStricmp(spareTier2, "+/?") == 0) {
			isCountMSs = FALSE;
		}
	}
	if (isCountwfSs) {
// 2020-02-14		ts->wfSs++;
// 2020-02-14		ts->aw_swf = 1.0;
	}
}

static char c_qpa_isUttDel(char *line, int pos) {
	if (line[pos] == '?' && line[pos+1] == '|')
		;
	else if (uS.IsUtteranceDel(line, pos)) {
		if (!uS.atUFound(line, pos, &dFnt, MBF))
			return(TRUE);
	}
	return(FALSE);
}

static int excludePlusAndUttDels(char *word) {
	if (word[0] == '+' || strcmp(word, "!") == 0 || strcmp(word, "?") == 0 || strcmp(word, ".") == 0)
		return(FALSE);
	return(TRUE);
}

static float roundFloat(double num) {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}
/*
static char checkVIR(struct VIR *v, MORFEATS *feat) {
	while (v) {
		if (isEqual(v->word, feat->stem))
			return(TRUE);
		v = v->nextVIR;
	}
	return(FALSE);
}
*/
/*
static char nextGRAItems(char *line, int i, const char *st1, const char *st2) {
	char morWord[1024], graWord[1024], *vb;

	if ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			vb = strrchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				if (uS.mStricmp(vb, st1) == 0) {
					if (st2 == NULL) {
						return(TRUE);
					} else {
						if ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
							if (graWord[0] != EOS && morWord[0] != EOS) {
								vb = strrchr(graWord, '|');
								if (vb != NULL) {
									vb++;
									if (uS.mStricmp(vb, st2) == 0) {
										return(TRUE);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return(FALSE);
}
*/
static char isFoundAnyWhereOnLine(char *line, const char *tag) {
	int  i;
	char word[1024];

	i = 0;
	while ((i=getword("*", line, word, NULL, i)) != 0) {
		if (uS.patmat(word, tag)) {
			return(TRUE);
		}
	}
	return(FALSE);
}
/*
static char isFoundAnyWhereOnMOR(char *line, const char *tag) {
	int  i;
	char morWord[1024], graWord[1024];

	i = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			if (uS.patmat(morWord, tag) == 0) {
				return(TRUE);
			}
		}
	}
	return(FALSE);
}
*/
static char isCurLinkedtoTag(int *num, char *line, const char *tag, char isStopAtNext) {
	int  i, cnt;
	char morWord[1024], graWord[1024], *vb, *pos;

repeat:
	i = 0;
	cnt = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			cnt++;
			if (cnt == *num) {
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					pos = strchr(tag, ',');
					if (pos != NULL) {
						strcpy(templineC3, tag);
						tag = templineC3;
						pos = strchr(tag, ',');
						*pos = EOS;
					}
					if (uS.mStricmp(vb, tag) == 0) {
						vb = strchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							*num = atoi(vb);
						}
						if (pos == NULL)
							return(TRUE);
						else if (uS.mStrnicmp(morWord, pos+1, strlen(pos+1)) == 0)
							return(TRUE);
					} else if (!isStopAtNext) {
						vb = strchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							*num = atoi(vb);
						}
						goto repeat;
					}
				}
				break;
			}
		}
	}
	return(FALSE);
}

static char isLinkedtoTag(char *graTarget, char *line, const char *tag1, const char *tag2, const char *tag3) {
	int  num;
	char *vb;

	vb = strchr(graTarget, '|');
	if (vb != NULL) {
		vb++;
		num = atoi(vb);
	} else
		return(FALSE);
	if (isCurLinkedtoTag(&num, line, tag1, TRUE)) {
		if (tag2 == NULL)
			return(TRUE);
		else {
			if (isCurLinkedtoTag(&num, line, tag2, TRUE)) {
				if (tag3 == NULL)
					return(TRUE);
				else {
					if (isCurLinkedtoTag(&num, line, tag3, TRUE)) {
						return(TRUE);
					}
				}
			}
		}
	}
	return(FALSE);
}

static char isCurEventuallyLinkedtoTags(int *num, char *line, const char *tag1, const char *tag2) {
	int  i, cnt, tnum;
	char morWord[1024], graWord[1024], *vb;

repeat:
	i = 0;
	cnt = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			cnt++;
			if (cnt == *num) {
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (uS.mStricmp(vb, tag1) == 0) {
						vb = strchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							tnum = atoi(vb);
							*num = atoi(vb);
							if (isCurLinkedtoTag(&tnum, line, tag2, TRUE)) {
								return(TRUE);
							}
						}
					} else {
						vb = strchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							*num = atoi(vb);
						}
						goto repeat;
					}
				}
				break;
			}
		}
	}
	return(FALSE);
}
/*
static char isRightSentenceUtt(char *line) {  // 2020-08-26
	int  i;
	char morWord[1024], graWord[1024], *vb;
	MORFEATS word_feats, *feat;

	i = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			vb = strrchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				if (uS.mStricmp(vb, "ROOT") == 0) {
					if (ParseWordMorElems(morWord, &word_feats) == FALSE)
						c_qpa_error(db, TRUE);
					for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
						if (isEqual("v", feat->pos) &&
							!isIxesMatchPat(feat->fusion, NUM_FUSI, "_*") && !isIxesMatchPat(feat->suffix, NUM_SUFF, "_*")) {
							return(TRUE);
						} else if (isEqual("cop", feat->pos) && isEqual("be", feat->stem)) {
							return(TRUE);
						}
					}
					freeUpFeats(&word_feats);
				}
			}
			break;
		}
	}
	return(FALSE);
}
*/
static char isFirstGRA(char *line, const char *tag) {
	int  i, cnt;
	char morWord[1024], graWord[1024], *vb;

	i = 0;
	cnt = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			cnt++;
			if (cnt == 1) {
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (uS.mStricmp(vb, tag) == 0) {
						return(TRUE);
					}
				}
			}
			break;
		}
	}
	return(FALSE);
}

static char isNumItemPointedToBy(char *line, int num, const char *tag) {
	int  i, cnum;
	char morWord[1024], graWord[1024], *vb;

	i = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			vb = strchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				cnum = atoi(vb);
			} else
				cnum = 0;
			vb = strrchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				if (num == cnum && uS.mStricmp(vb, tag) == 0) {
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

static char isNextPOS(int i, char *sp, char *line, const char *POS, const char *stem) {
	char morWord[1024], graWord[1024];
	MORFEATS word_feats, *feat;

	if (sp == NULL) {
		if ((i=getNextDepTierPair(line, graWord, morWord, NULL, i)) == 0)
			return(FALSE);
	} else {
		if ((i=getword(sp, line, morWord, NULL, i)) == 0)
			return(FALSE);
	}
	if (isWordFromMORTier(morWord)) {
		if (ParseWordMorElems(morWord, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
			if (isEqual(POS, feat->pos)) {
				if (stem == NULL)
					return(TRUE);
				else if (isEqual(stem, feat->stem))
					return(TRUE);
			}
		}
		freeUpFeats(&word_feats);
	}
	return(FALSE);
}

static char isFeatClitic(MORFEATS *feat, const char *POS, const char *stem) {
	for (; feat != NULL; feat=feat->clitc) {
		if (isEqual(POS, feat->pos)) {
			if (stem == NULL)
				return(TRUE);
			else if (isEqual(stem, feat->stem))
				return(TRUE);
		}
	}
	return(FALSE);
}

static char isNextGRA(int i, char *line, const char *tag) {
	char morWord[1024], graWord[1024], *vb;

	while ((i=getNextDepTierPair(line, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			vb = strrchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				if (uS.mStricmp(vb, tag) == 0) {
					return(TRUE);
				} else
					return(FALSE);
			}
		}
	}
	return(FALSE);
}

static int isNextMorAdjNP(int i, char *line) {
	int wordCnt;
	MORFEATS word_feats, *feat;

	wordCnt = 0;
	while ((i=getword(utterance->speaker, line, templineC3, NULL, i))) {
		wordCnt++;
		if (!isWordFromMORTier(templineC3))
			continue;
		if (ParseWordMorElems(templineC3, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
			if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
				freeUpFeats(&word_feats);
				return(wordCnt);
			} else if (isEqual("adj", feat->pos)) {
			} else {
				freeUpFeats(&word_feats);
				return(0);
			}
		}
		freeUpFeats(&word_feats);
	}
	return(0);
}

static int isNextMorNP(int i, char *line) {
	int wordCnt;
	MORFEATS word_feats, *feat;

	wordCnt = 0;
	while ((i=getword(utterance->speaker, line, templineC3, NULL, i))) {
		wordCnt++;
		if (!isWordFromMORTier(templineC3))
			continue;
		if (ParseWordMorElems(templineC3, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
			if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
				freeUpFeats(&word_feats);
				return(wordCnt);
			} else if (isEqual("adj", feat->pos) ||
					   isEqual("det:art", feat->pos) || isEqual("det:poss", feat->pos) ||
					   isEqual("det:num", feat->pos) || isEqual("det:dem", feat->pos) ||
					   isEqual("qn", feat->pos)) {
			} else {
				freeUpFeats(&word_feats);
				return(0);
			}
		}
		freeUpFeats(&word_feats);
	}
	return(0);
}

static char isNextMorPro(int i, char *line) {
	MORFEATS word_feats, *feat;

	while ((i=getword(utterance->speaker, line, templineC3, NULL, i))) {
		if (!isWordFromMORTier(templineC3))
			continue;
		if (ParseWordMorElems(templineC3, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		feat = &word_feats;
		if (feat != NULL) {
			if (isnEqual("pro:", feat->pos, 4)) {
				freeUpFeats(&word_feats);
				return(TRUE);
			} else {
				freeUpFeats(&word_feats);
				return(FALSE);
			}
		}
		freeUpFeats(&word_feats);
	}
	return(FALSE);
}

static char isNextMorNoun(int i, char *line) {
	MORFEATS word_feats, *feat;

	while ((i=getword(utterance->speaker, line, templineC3, NULL, i))) {
		if (!isWordFromMORTier(templineC3))
			continue;
		if (ParseWordMorElems(templineC3, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		feat = &word_feats;
		if (feat != NULL) {
			if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
				freeUpFeats(&word_feats);
				return(TRUE);
			} else {
				freeUpFeats(&word_feats);
				return(FALSE);
			}
		}
		freeUpFeats(&word_feats);
	}
	return(FALSE);
}

static char isRightPRED_ROOT(char *line) {
	int  i;
	char morWord[1024], graWord[1024], *vb;
	MORFEATS word_feats, *feat;

	i = 0;
	while ((i=getNextDepTierPair(line, graWord, morWord, NULL, i)) != 0) {
		if (!isWordFromMORTier(morWord))
			continue;
		if (graWord[0] != EOS && morWord[0] != EOS) {
			if (ParseWordMorElems(morWord, &word_feats) == FALSE)
				c_qpa_error(db, TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
// if anywhere pro:exist/SUBJ or pro:indef(all)/SUBJ -> ROOT
				if (isEqual("pro:exist", feat->pos) || isnEqual("all", feat->stem, 2)) {
					vb = strrchr(graWord, '|');
					if (vb != NULL) {
						vb++;
						if (uS.mStricmp(vb, "SUBJ") == 0) {
							if (isLinkedtoTag(graWord, line, "ROOT", NULL, NULL)) {
								return(TRUE);
							}
						}
					}
				}
			}
			freeUpFeats(&word_feats);
		}
	}
	return(FALSE);
}

static char isSNPgra(char *graWord, char *line) {
	char *vb;

	vb = strrchr(graWord, '|');
	if (vb != NULL) {
		vb++;
		if (uS.mStricmp(vb, "SUBJ") == 0) {
			if (isLinkedtoTag(graWord, line, "ROOT", NULL, NULL)) {
				return(TRUE);
			}
		} else if (uS.mStricmp(vb, "PRED") == 0) {
			if (isLinkedtoTag(graWord, line, "ROOT", NULL, NULL)) {
				if (isRightPRED_ROOT(line))
					return(TRUE);
			}
		} else if (uS.mStricmp(vb, "COORD") == 0) {
			if (isLinkedtoTag(graWord, line, "CONJ", "SUBJ", "ROOT")) {
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

static char isSNP_word_gra(char *graWord, char *line) {
	int  num;
	char *vb;

	vb = strrchr(graWord, '|');
	if (vb != NULL) {
		vb++;
		if (uS.mStricmp(vb, "PRED") == 0) {
			if (isLinkedtoTag(graWord, line, "ROOT", NULL, NULL)) {
				if (isRightPRED_ROOT(line))
					return(TRUE);
			}
		}
		num = atoi(graWord);
		if (isCurEventuallyLinkedtoTags(&num, line, "SUBJ", "ROOT")) {
			return(TRUE);
		}
	}
	return(FALSE);
}

static char isSNPOpenCLass(MORFEATS *feat, char *graWord, char *line) {
	if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) || isnEqual("cop", feat->pos, 3) ||
		isEqual("adj", feat->pos) || isEqual("pro:indef", feat->pos) || isEqual("pro:per", feat->pos) ||
		isEqual("pro:sub", feat->pos) || isEqual("pro:dem", feat->pos) || isEqual("det:num", feat->pos) ||
		isEqual("det:poss", feat->pos)) {
		if (isSNP_word_gra(graWord, line)) {
			return(TRUE);
		}
	} else if (isEqual("adv:int", feat->pos) || isEqual("comp", feat->pos)) {
	} else if ((isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) &&
			   isEqualIxes("LY", feat->suffix, NUM_SUFF)) {
		if (isSNP_word_gra(graWord, line)) {
			return(TRUE);
		}
	}
	return(FALSE);
}

static int isSNP(int i, char *line, int wi, char *graWord, char *morWord, char *isOcSNPWord, int *tOcSNP, char *isThisSNP) {
	int li;
	MORFEATS word_feats, *feat;

	*tOcSNP = 0;
	do {
		li = i;
		if (!isWordFromMORTier(morWord))
			continue;
		if (ParseWordMorElems(morWord, &word_feats) == FALSE)
			c_qpa_error(db, TRUE);
		for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
			if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
				if (isSNPOpenCLass(feat, graWord, line)) {
					*tOcSNP = *tOcSNP + 1;
					isOcSNPWord[wi] = 's';
				}
				if (isSNPgra(graWord, line)) {
					*isThisSNP = TRUE;
					isOcSNPWord[wi] = 's';
					freeUpFeats(&word_feats);
					return(i);
				} else if (!isNextMorNoun(i, line)) {
					freeUpFeats(&word_feats);
					return(i);
				}
			} else if (isEqual("pro", feat->pos) || isnEqual("pro:", feat->pos, 4)) {
				if (isSNPOpenCLass(feat, graWord, line)) {
					*tOcSNP = *tOcSNP + 1;
					isOcSNPWord[wi] = 's';
				}
				if (isEqual("pro:rel", feat->pos) || isEqual("pro:int", feat->pos) ||
					isEqual("pro:exist", feat->pos)) {
					freeUpFeats(&word_feats);
					return(i);
				}
				if (isSNPgra(graWord, line)) {
					*isThisSNP = TRUE;
					isOcSNPWord[wi] = 's';
				}
				freeUpFeats(&word_feats);
				return(i);
			} else if (isEqual("adj", feat->pos) ||
					   isEqual("det:art", feat->pos) || isEqual("det:poss", feat->pos) ||
					   isEqual("det:dem", feat->pos) || isEqual("qn", feat->pos)) {
				if (isSNPOpenCLass(feat, graWord, line)) {
					*tOcSNP = *tOcSNP + 1;
					isOcSNPWord[wi] = 's';
				}
			} else if (isEqual("det:num", feat->pos)) {
				if (!isNextMorPro(i, line) && isSNPOpenCLass(feat, graWord, line)) {
					*tOcSNP = *tOcSNP + 1;
					isOcSNPWord[wi] = 's';
				}
			} else {
				if (isSNPOpenCLass(feat, graWord, line)) {
					*tOcSNP = *tOcSNP + 1;
					isOcSNPWord[wi] = 's';
				}
				freeUpFeats(&word_feats);
				return(i);
			}
		}
		freeUpFeats(&word_feats);
	} while ((i=getNextDepTierPair(line, graWord, morWord, &wi, i)) != 0) ;
	return(li);
}

static char isOcVP(int i, char *line, char *graWord, MORFEATS *feat, int *tOcVP) {
	char isOcVPfound;
	int num;
	char *vb;

	isOcVPfound = FALSE;
	vb = strchr(graWord, '|');
	if (vb != NULL) {
		vb++;
		num = atoi(vb);
		vb = strrchr(graWord, '|');
		vb++;
	} else
		return(isOcVPfound);
	*tOcVP = 0;
	for (; feat != NULL; feat=feat->clitc) {
		if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) || isnEqual("cop", feat->pos, 3) ||
			isEqual("adj", feat->pos) || isEqual("pro:sub", feat->pos) || isEqual("pro:obj", feat->pos) ||
			isEqual("pro:indef", feat->pos) || isEqual("pro:dem", feat->pos)) {
			if (isCurLinkedtoTag(&num, line, "ROOT", FALSE) || uS.mStricmp(vb, "ROOT") == 0) {
				*tOcVP = *tOcVP + 1;
				isOcVPfound = TRUE;
			}
		} else if (isEqual("adv:int", feat->pos) || isEqual("comp", feat->pos)) {
		} else if ((isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) &&
				   isEqualIxes("LY", feat->suffix, NUM_SUFF)) {
			if (isCurLinkedtoTag(&num, line, "ROOT", FALSE)) {
				*tOcVP = *tOcVP + 1;
				isOcVPfound = TRUE;
			}
		}
	}
	return(isOcVPfound);
}

static void c_qpa_process_tier(struct c_qpa_speakers *ts, struct database *db, char *rightIDFound, char isOutputGem,
							   FILE *PWordsListFP, FILE *AnalysisSheetFP) {
	int i, j, wi, wordCnt, nextNrd, tOcSNP, tOcVP, curScore, auxBeScore;
//	char isLastINF;
	char word[1024], graWord[1024], lFeatPOS[256], lFeatSTEM[256], lGraWord[1024];
	char tmp, isWordsFound, sq, aq, isSkip, *vb, *isOcSNPWord, isAnySNPFound, isAuxBeFound, isThisSNP, isSentenceUttsCorrected;
	char isPSDFound, curPSDFound, isAmbigFound, isCountNV, isCountInflectionsAuxScore, isMSsCounted;
	char isMOD_INFfound;
	long stime, etime;
	double tNum;
	struct IDparts IDTier;
	MORFEATS word_feats, *feat;

	if (utterance->speaker[0] == '*') {
		isCountMSs = TRUE;
		org_depTier[0] = EOS;
		strcpy(spareTier1, utterance->line);
		if (tmDur >= 0.0) {
			ts->tm = ts->tm + tmDur;
			tmDur = -1.0;
		}
		for (i=0; utterance->line[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				ts->isPSDFound = FALSE;
			}
		}
		if (ts == NULL)
			return;
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (templineC2[0] == '&' && templineC2[1] == '=') {
				j = 0;
			} else if (templineC2[0] != '[' && templineC2[0] != '-' &&
				templineC2[0] != '+' && templineC2[0] != '!' && templineC2[0] != '?' && templineC2[0] != '.' &&
				templineC2[0] != HIDEN_C) {
				break;
			}
		}
		if (i == 0)
			j = 0;
		else
			ts->numUtt++;
//ANALYSIS WORKSHEET beg
		if (last_utt != NULL) {
			if (last_utt[0] != EOS) {
				outputAnalysisSheet(AnalysisSheetFP, last_utt, ts);
			}
			strcpy(last_utt, utterance->line);
		}
		ts->aw_sentenceUtts = 0.0;
		ts->aw_otherUtts = 0.0;
		ts->aw_totalWords = 0.0;
		ts->aw_openClass = 0.0;
		ts->aw_noun  = 0.0;
		ts->aw_NRDs  = 0.0;
		ts->aw_NRDwD = 0.0;
		ts->aw_pron = 0.0;
		ts->aw_verb = 0.0;
		ts->aw_inverb = 0.0;
		ts->aw_inverbI = 0.0;
		ts->aw_matVerb = 0.0;
		ts->aw_auxScore = 0.0;
		ts->aw_embedd = 0.0;



		ts->aw_swf = 0.0;
		ts->aw_PSNP = 0.0;
		ts->aw_ocSNP = 0.0;
		ts->aw_PVP = 0.0;
		ts->aw_ocVP = 0.0;
		ts->aw_nWSs = 0.0;
//ANALYSIS WORKSHEET end

	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (org_depTier[0] == EOS) {
			strcpy(org_depTier, utterance->line);
		}
		if (ts == NULL)
			return;

		isCountUtt(spareTier1, uttline, ts); // lxs point (M) (m)
		filterwords("%", uttline, excludePlusAndUttDels);
		if (isExcludeWords)
			filterwords("%", uttline, exclude);
		i = 0;
		if ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
			if (uS.mStrnicmp(templineC2, "coord|and", 9) == 0) {
				for (; uttline[j] != EOS && j < i; j++)
					uttline[j] = ' ';
				if ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
					if (uS.mStrnicmp(templineC2, "adv:tem|then", 12) == 0) {
						for (; uttline[j] != EOS && j < i; j++)
							uttline[j] = ' ';
					} else if (uS.mStrnicmp(templineC2, "co|so", 5) == 0) {
						for (; uttline[j] != EOS && j < i; j++)
							uttline[j] = ' ';
					}
				}
			} else if (uS.mStrnicmp(templineC2, "conj|but", 8) == 0) {
				for (; uttline[j] != EOS && j < i; j++)
					uttline[j] = ' ';
			} else if (uS.mStrnicmp(templineC2, "co|so", 5) == 0) {
				for (; uttline[j] != EOS && j < i; j++)
					uttline[j] = ' ';
			} else if (uS.mStrnicmp(templineC2, "adv:tem|then", 12) == 0) {
				for (; uttline[j] != EOS && j < i; j++)
					uttline[j] = ' ';
			} else if (uS.mStrnicmp(templineC2, "co|well", 7) == 0) {
				for (; uttline[j] != EOS && j < i; j++)
					uttline[j] = ' ';
			}
		}

		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
				if (uS.mStricmp(templineC2, "adv|once") == 0) {
					wi = i;
					if ((wi=getword(utterance->speaker, uttline, templineC2, NULL, wi))) {
						if (uS.mStricmp(templineC2, "prep|upon") == 0) {
							if ((wi=getword(utterance->speaker, uttline, templineC2, NULL, wi))) {
								if (uS.mStricmp(templineC2, "det:art|a") == 0) {
									if ((wi=getword(utterance->speaker, uttline, templineC2, NULL, wi))) {
										if (uS.mStricmp(templineC2, "n|time") == 0) {
											for (; uttline[j] != EOS && j < wi; j++)
												uttline[j] = ' ';
										}
									}
								}
							}
						}
					}
				} else if (uS.mStricmp(templineC2, "adv|happy&dadj-LY") == 0) {
					wi = i;
					if ((wi=getword(utterance->speaker, uttline, templineC2, NULL, wi))) {
						if (uS.mStricmp(templineC2, "adv|ever") == 0) {
							if ((wi=getword(utterance->speaker, uttline, templineC2, NULL, wi))) {
								if (uS.patmat(templineC2, "*|after")) {
									for (; uttline[j] != EOS && j < wi; j++)
										uttline[j] = ' ';
								}
							}
						}
					}
				}
		}

		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
			if (!isWordFromMORTier(templineC2))
				continue;
			if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
				c_qpa_error(db, TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
				if (isIxesMatchPat(feat->error, NUM_ERRS, "n:uk") ||
					isIxesMatchPat(feat->error, NUM_ERRS, "s:uk") ||
					isIxesMatchPat(feat->error, NUM_ERRS, "n:uk:s")) {
					for (; uttline[j] != EOS && j < i; j++)
						uttline[j] = ' ';
				}
			}
			freeUpFeats(&word_feats);
		}


		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (strchr(templineC2, '|') != NULL && templineC2[0] != '0') {
				ts->totalWords++;
				ts->aw_totalWords++;
				for (j=0; templineC2[j] != EOS; j++) {
					if (templineC2[j] == '$' && !isPrecliticUse) {
						ts->totalWords++;
						ts->aw_totalWords++;
					}
					if (templineC2[j] == '~' && !isPostcliticUse) {
						ts->totalWords++;
						ts->aw_totalWords++;
					}
				}
			}
		}
		isSkip = FALSE;
		if (isPostCodeOnUtt(spareTier1, "[+ mlue]")) {
			isSkip = TRUE;
		} else {
			i = 0;
			while ((i=getword(utterance->speaker, spareTier1, word, NULL, i))) {
				if (strcmp(word, "xxx") == 0 || strcmp(word, "yyy") == 0 || strcmp(word, "www") == 0) {
					isSkip = TRUE;
					break;
				}
			}
		}
		ts->isMORFound = TRUE;
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
				isAmbigFound = FALSE;
				isWordsFound = TRUE;
				tmp = TRUE;
//				mluWords = mluWords + 1;
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
//									morf = morf + 1;
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
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (ts->tUtt > 0.0)
					ts->tUtt--;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
				curPSDFound = TRUE;
			if (c_qpa_isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
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
		strcpy(spareTier3, uttline);
/*
		for (i=0; spareTier3[i] != EOS; i++) {
			if (spareTier3[i] == '$' || spareTier3[i] == '~')
				spareTier3[i] = ' ';
		}
*/
		isMOD_INFfound = 0;
		lFeatPOS[0] = EOS;
		i = 0;
		wordCnt = 0;
		nextNrd = wordCnt;
		while ((i=getword(utterance->speaker, spareTier3, templineC2, NULL, i))) {
			wordCnt++;
			if (!isWordFromMORTier(templineC2))
				continue;
			if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
				c_qpa_error(db, TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
				// counts open/closed BEG
				if (isEqual("adv:int", feat->pos)) {
					ts->closedClass++;
				} else if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) || isnEqual("cop", feat->pos, 3) ||
						   isEqual("adj", feat->pos)) {
					ts->openClass++;
					ts->aw_openClass++;
				} else if ((isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) &&
						   isEqualIxes("LY", feat->suffix, NUM_SUFF)) {
					ts->openClass++;
					ts->aw_openClass++;
				} else if (isEqual("co", feat->pos) == FALSE && isEqual("on", feat->pos) == FALSE) {
					ts->closedClass++;
				}
				// counts open/closed END
				if ((isEqual("mod", feat->pos) && isEqual("do", feat->stem) && isEqualIxes("PAST", feat->fusion, NUM_FUSI)) ||
					(isEqual("mod", feat->pos) && isEqual("can", feat->stem))    ||
					(isEqual("mod", feat->pos) && isEqual("should", feat->stem)) ||
					(isEqual("mod", feat->pos) && isEqual("will", feat->stem) && isEqualIxes("COND", feat->fusion, NUM_FUSI)) ||
					isEqual("mod:aux", feat->pos) || isEqual("inf", feat->pos)) {
					isMOD_INFfound = 1;
				} else if (isMOD_INFfound == 2 || isMOD_INFfound == 3) {
					if (isEqual("and", feat->stem) || isEqual("or", feat->stem)) {
						isMOD_INFfound = 1;
					} else if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) || isEqual("part", feat->pos)) {
						if (isMOD_INFfound == 2) {
							isMOD_INFfound = 1;
						} else if (isMOD_INFfound == 3) {
							isMOD_INFfound = 0;
						}
					} else {
						isMOD_INFfound = 3;
					}
				}
				if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
					ts->noun++;
					ts->aw_noun++;
				} else if (isEqual("adj", feat->pos) && isEqualIxes("POSS", feat->suffix, NUM_SUFF) &&
						   isEqualIxes("dn", feat->fusion, NUM_FUSI)) {
					ts->noun++;
					ts->aw_noun++;
					wi = isNextMorAdjNP(i, spareTier3);
					if (wordCnt > nextNrd && wi > 0) {
						nextNrd = wordCnt + wi;
						ts->NRDs++;
						ts->aw_NRDs++;
						ts->NRDwD++;
						ts->aw_NRDwD++;
					}
				} else if (isEqual("0det:art", feat->pos)) {
					wi = isNextMorNP(i, spareTier3);
					if (wordCnt > nextNrd && wi > 0) {
						nextNrd = wordCnt + wi;
						ts->NRDs++;
						ts->aw_NRDs++;
					}
				} else if (isEqual("det:num", feat->pos)) {
					wi = isNextMorAdjNP(i, spareTier3);
					if (wordCnt > nextNrd && wi > 0) {
						nextNrd = wordCnt + wi;
						ts->NRDs++;
						ts->aw_NRDs++;
						ts->NRDwD++;
						ts->aw_NRDwD++;
					}
				} else if (isEqual("det:art", feat->pos) || isEqual("det:dem", feat->pos) ||
						   isEqual("det:poss", feat->pos) || isEqual("pro:dem", feat->pos)) {
					if (isEqual("det:poss", feat->pos)) {
						ts->pron++;
						ts->aw_pron++;
					}
					wi = isNextMorNP(i, spareTier3);
					if (wordCnt > nextNrd && wi > 0) {
						nextNrd = wordCnt + wi;
						ts->NRDs++;
						ts->aw_NRDs++;
						ts->NRDwD++;
						ts->aw_NRDwD++;
					}
				} else if (isEqual("pro", feat->pos) || isnEqual("pro:", feat->pos, 4)) {
					if (isEqual("pro:rel", feat->pos) || isEqual("pro:int", feat->pos) ||
						isEqual("pro:exist", feat->pos)) {
						j = 0;
					} else {
						ts->pron++;
						ts->aw_pron++;
					}
				} else if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) || isEqual("part", feat->pos)) {
					if (isEqual("part", feat->pos) && isEqual("go", feat->stem) &&
						(isFeatClitic(feat->clitc, "inf", "to") || isNextPOS(i, utterance->speaker, spareTier3, "inf", "to"))) {
						j = 0;
					} else {
						ts->verb++;
						ts->aw_verb++;
					}
/* 2018-10-23
					if (isEqual("part", feat->pos) && isEqualIxes("PRESP", feat->suffix, NUM_SUFF)) {
						ts->inverb++;
						ts->inverbI++;
					} else
					if (!checkVIR(virs, feat)) {
*/
						if (isMOD_INFfound) {
							j = 0;
							isMOD_INFfound = 2;
						} else if (isIxesMatchPat(feat->fusion, NUM_FUSI, "_*")) {
							j = 0;
						} else if (isEqual("cop", feat->pos) && isEqual("be", feat->stem)) {
							j = 0;
						} else if (lFeatPOS[0] == EOS) {
							if (isFoundAnyWhereOnLine(spareTier1, "+\"")) {
								j = 0;
							} else {
								ts->inverb++;
								ts->aw_inverb++;
								if (isIxesMatchPat(feat->suffix, NUM_SUFF, "_*")) {
									if (isIxesMatchPat(feat->error, NUM_ERRS, "m:03s*")   ||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed")||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")    ||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en")||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing")   ||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed")    ||
										isIxesMatchPat(feat->error, NUM_ERRS, "m:=en")) {
										j = 0;
									} else {
										ts->inverbI++;
										ts->aw_inverbI++;
									}
								}
							}
						} else if (isEqual("beg", lFeatPOS)) {
							j = 0;
						} else {
							ts->inverb++;
							ts->aw_inverb++;
							if (isIxesMatchPat(feat->suffix, NUM_SUFF, "_*")) {
								if (isIxesMatchPat(feat->error, NUM_ERRS, "m:03s*")   ||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed")||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")    ||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en")||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing")   ||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed")    ||
									isIxesMatchPat(feat->error, NUM_ERRS, "m:=en")) {
									j = 0;
								} else {
									ts->inverbI++;
									ts->aw_inverbI++;
								}
							}
						}
/* 2018-10-23
					} else
						j = 0;
*/
				}
/* 2019-02-26
				if (isEqual("neg", feat->pos) && isEqual("not", feat->stem)) {
					if (lFeatPOS[0] != EOS) {
						if (isEqual("mod", lFeatPOS) && isNextPOS(i, spareTier3, "v")) {
							ts->auxScore++;
							ts->aw_auxScore++;
						} else if (isEqual("mod", lFeatPOS) && isNextPOS(i, spareTier3, "aux")) {
							ts->auxScore++;
							ts->aw_auxScore++;
						} else if (isEqual("aux", lFeatPOS) && isNextPOS(i, spareTier3, "mod")) {
							ts->auxScore++;
							ts->aw_auxScore++;
						} else if (isEqual("aux", lFeatPOS) && isNextPOS(i, spareTier3, "aux")) {
							ts->auxScore++;
							ts->aw_auxScore++;
						} else if (isEqual("aux", lFeatPOS) && isNextPOS(i, spareTier3, "part")) {
							ts->auxScore++;
							ts->aw_auxScore++;
						} else if (isEqual("cop", lFeatPOS)) {
							ts->auxScore++;
							ts->aw_auxScore++;
						}
					}
				}
*/
// lxs				countMisingAuxBe();


				// regular inflected verbs - griv, ariv
				if ((isEqual("v", feat->pos) && isEqualIxes("PAST", feat->suffix, NUM_SUFF)) ||
					(isEqual("v", feat->pos) && isEqualIxes("3S", feat->suffix, NUM_SUFF)) ||
					((isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) && isEqualIxes("PL", feat->suffix, NUM_SUFF)) ||
					((isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) && isEqualIxes("POSS", feat->suffix, NUM_SUFF)) ||
					(isEqual("cop", feat->pos) && isEqualIxes("3S", feat->suffix, NUM_SUFF)) ||
					(isEqual("cop", feat->pos) && isEqualIxes("PAST", feat->suffix, NUM_SUFF)) ||
					((isEqual("part", feat->pos) || isnEqual("part:", feat->pos, 5)) && isEqualIxes("PASTP", feat->suffix, NUM_SUFF)) ||
					((isEqual("part", feat->pos) || isnEqual("part:", feat->pos, 5)) && isEqualIxes("PRESP", feat->suffix, NUM_SUFF))) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:03s")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0s")    ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0's")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0s'")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+ing")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+ed")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+en")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+s")    ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++ing") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++3s")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++en")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++s")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++'s")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:03s:a") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s:a") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0s:a")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+s:a")) {
						j = 0;
					} else {
//						ts->griv++;
					}
//					ts->ariv++;
				}

				if (isIxesMatchPat(feat->error, NUM_ERRS, "m:+ing")  ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s")   ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:+ed")   ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:+en")   ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:+s")    ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++ing") ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++3s")  ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed")  ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++en")  ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++s")   ||
					isIxesMatchPat(feat->error, NUM_ERRS, "m:++'s")) {
//					ts->ariv++;
				}

				strcpy(lFeatPOS, feat->pos);
			}
			freeUpFeats(&word_feats);
		}
		if (isDebugMode == TRUE || isDebugMode == 'K' || isDebugMode == 'k') {
			strcpy(spareTier2, uttline);
			fprintf(stderr, "#Inflectable Vs=%0.f; #IVs Inflected=%0.f:\n", ts->aw_inverb, ts->aw_inverbI);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}
		if (isDebugMode == TRUE || isDebugMode == 'P' || isDebugMode == 'p') {
			strcpy(spareTier2, uttline);
			fprintf(stderr, "#Pronouns=%0.f; #Nouns=%0.f; #Verbs=%0.f:\n", ts->aw_pron, ts->aw_noun, ts->aw_verb);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}
	} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
		isSentenceUttsCorrected = FALSE;
		if (ts == NULL)
			return;
		if (org_depTier[0] == EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"ERROR: %%mor tier is not found before %%gra tier.\n");
			c_qpa_error(db, FALSE);
		}
		for (j=0; org_depTier[j] != EOS; j++) {
			if (org_depTier[j] == '~' || org_depTier[j] == '$')
				org_depTier[j] = ' ';
		}
		createMorUttline(utterance->line,org_depTier,"%gra",utterance->tuttline,FALSE,TRUE);
		strcpy(utterance->tuttline, utterance->line);
		filterMorTier(uttline, utterance->line, 2, TRUE);

		isCountNV = FALSE;
		if (isPostCodeOnUtt(spareTier1, "[+ 0subj]") == TRUE)
			isMSsCounted = TRUE;
		else
			isMSsCounted = FALSE;
		i = 0;
		while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
			if (graWord[0] != EOS && templineC2[0] != EOS) {
				if (!isWordFromMORTier(templineC2))
					continue;
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (uS.mStricmp(vb, "SUBJ") == 0) {
						if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
							ts->aw_sentenceUtts = 1.0;
							if (ts->aw_otherUtts > 0.0)
								ts->aw_otherUtts = 0.0;
						}
					} else if (uS.mStricmp(vb, "INCROOT") == 0) {
						if (ts->aw_sentenceUtts == 0.0)
							ts->aw_otherUtts = 1.0;
					}
					if (isCountMSs) {
						if (uS.mStricmp(vb, "SUBJ") == 0 && isMSsCounted == FALSE) {
							if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
// 2020-02-14								ts->mSs++;
								isCountNV = TRUE;
								isMSsCounted = TRUE;
							}
						}
					}
				}
			}
		}
		if (isMSsCounted == FALSE && isCountMSs) {
			if (isFirstGRA(uttline, "ROOT")) {
// 2020-02-14				ts->mSs++;
				isCountNV = TRUE;
				isMSsCounted = TRUE;
			} else {
				i = 0;
				while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
					if (graWord[0] != EOS && templineC2[0] != EOS) {
						if (!isWordFromMORTier(templineC2))
							continue;
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							if (uS.mStricmp(vb, "BEGP") == 0 && isNextGRA(i, uttline, "ROOT")) {
// 2020-02-14								ts->mSs++;
								isCountNV = TRUE;
								isMSsCounted = TRUE;
								break;
							}
						}
					}
				}
			}
		}
		if (ts->aw_sentenceUtts == 0.0 && isPostCodeOnUtt(spareTier1, "[+ 0subj]") == FALSE) {
//			if (isRightSentenceUtt(uttline)) { // 2020-08-26
			if (isFirstGRA(uttline, "ROOT")) {
				ts->aw_sentenceUtts = 1.0;
				isSentenceUttsCorrected = TRUE;
				if (ts->aw_otherUtts > 0.0)
					ts->aw_otherUtts = 0.0;
			} else {
				i = 0;
				while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
					if (graWord[0] != EOS && templineC2[0] != EOS) {
						if (!isWordFromMORTier(templineC2))
							continue;
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							if (uS.mStricmp(vb, "BEGP") == 0 && isNextGRA(i, uttline, "ROOT")) {
								ts->aw_sentenceUtts = 1.0;
								if (ts->aw_otherUtts > 0.0)
									ts->aw_otherUtts = 0.0;
								break;
							}
						}
					}
				}
			}
		}
		if (ts->aw_sentenceUtts == 1.0) {// 2020-02-14
			ts->mSs++;
			if (isPostCodeOnUtt(spareTier1, "[+ gram]") == FALSE) {
				ts->wfSs++;
				ts->aw_swf = 1.0;
			}
		}

		if (ts->aw_sentenceUtts == 0.0 && ts->aw_otherUtts == 0.0) {
			ts->aw_otherUtts = 1.0;
		}

		isOcSNPWord = templineC1;
		for (i=0; uttline[i] != EOS; i++) {
			isOcSNPWord[i] = 'w';
		}
		isAnySNPFound = FALSE; // SNPs ocSNP
		i = 0;
		while ((i=getNextDepTierPair(uttline, graWord, templineC2, &wi, i)) != 0) {
			if (graWord[0] != EOS && templineC2[0] != EOS) {
				if (!isWordFromMORTier(templineC2))
					continue;
				isThisSNP = FALSE;
				tOcSNP = 0;
				j = i;
				i = isSNP(i, uttline, wi, graWord, templineC2, isOcSNPWord, &tOcSNP, &isThisSNP);
				if (isThisSNP == TRUE) {
					isAnySNPFound = TRUE;
//					ts->SNPs++;
					ts->aw_PSNP++;
					if (tOcSNP > 0) {
//						ts->ocSNP = ts->ocSNP + tOcSNP;
						ts->aw_ocSNP = ts->aw_ocSNP + tOcSNP;
					}
				} else if (tOcSNP > 0) {
//					ts->ocSNP = ts->ocSNP + tOcSNP;
					ts->aw_ocSNP = ts->aw_ocSNP + tOcSNP;
				} else {
					for (; j < i; j++) {
						isOcSNPWord[j] = 'w';
					}
				}
			}
		}
		if (!isAnySNPFound && isPostCodeOnUtt(spareTier1, "[+ 0subj]") == FALSE) {
			if (isFirstGRA(uttline, "ROOT")) {
//				ts->SNPs++;
				ts->aw_PSNP++;
				ts->openClass++;
				ts->aw_openClass++;
//				ts->ocSNP = ts->ocSNP + tOcSNP;
				ts->aw_ocSNP = ts->aw_ocSNP + tOcSNP;
			} else {
				i = 0;
				while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
					if (graWord[0] != EOS && templineC2[0] != EOS) {
						if (!isWordFromMORTier(templineC2))
							continue;
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							if (uS.mStricmp(vb, "BEGP") == 0 && isNextGRA(i, uttline, "ROOT")) {
//								ts->SNPs++;
								ts->aw_PSNP++;
//								ts->ocSNP = ts->ocSNP + tOcSNP;
								ts->aw_ocSNP = ts->aw_ocSNP + tOcSNP;
								break;
							}
						}
					}
				}
			}
		}
		if (isDebugMode == TRUE || isDebugMode == 'B' || isDebugMode == 'b') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "Sentence utterance=%0.f:\n", ts->aw_sentenceUtts);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}

		if (isDebugMode == TRUE || isDebugMode == 'S' || isDebugMode == 's') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "SNP=%0.f; OpenClass=%.0f:\n", ts->aw_PSNP, ts->aw_ocSNP);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}

int all_OCs = 0;
		lGraWord[0] = EOS;
		i = 0; // VPs ocVP
		while ((i=getNextDepTierPair(uttline, graWord, templineC2, &wi, i)) != 0) {
			if (graWord[0] != EOS && templineC2[0] != EOS) {
				if (!isWordFromMORTier(templineC2))
					continue;
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (isDebugMode == TRUE || isDebugMode == 'T' || isDebugMode == 't') {
						strcpy(spareTier2, templineC2);
					}
					if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
						c_qpa_error(db, TRUE);
					if (isOcVP(i, uttline, graWord, &word_feats, &tOcVP)) {
						if (isOcSNPWord[wi] != 's') {
//							ts->ocVP = ts->ocVP + tOcVP;
							ts->aw_ocVP = ts->aw_ocVP + tOcVP;
							if (isDebugMode == TRUE || isDebugMode == 'T' || isDebugMode == 't') {
								fprintf(stderr, "OC=%s;\n", spareTier2);
							}
						} else {
							if (isDebugMode == TRUE || isDebugMode == 'T' || isDebugMode == 't') {
								fprintf(stderr, "SNP-OC=%s;\n", spareTier2);
							}
						}
						all_OCs++;
					}
					if (uS.mStricmp(vb, "ROOT") == 0) {
//						ts->VPs++;
						ts->aw_PVP++;
					} else if (uS.mStricmp(vb, "COORD") == 0 && uS.mStricmp(lGraWord, "CONJ") == 0) {
						if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL)) {
//							ts->VPs++;
							ts->aw_PVP++;
						}
					} else if (uS.mStricmp(vb, "ENUM") == 0) {
						if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
//							ts->VPs++;
							ts->aw_PVP++;
						}
					} else if (uS.mStricmp(vb, "LINK") == 0 && isEqual("coord", word_feats.pos)) {
						if (isLinkedtoTag(graWord, uttline, "CJCT", "ROOT", NULL)) {
//							ts->VPs++;
							ts->aw_PVP++;
						}
					}
					strcpy(lGraWord, vb);
					freeUpFeats(&word_feats);
				}
			}
		}
		if (isDebugMode == TRUE || isDebugMode == 'T' || isDebugMode == 't') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "VP=%0.f; OpenClass=%.0f; allOpenClass=%d:\n", ts->aw_PVP, ts->aw_ocVP, all_OCs);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}

		auxBeScore = 0;
		isAuxBeFound = 0;
		lFeatPOS[0] = EOS;
		lFeatSTEM[0]= EOS;
		i = 0;
		while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
			if (graWord[0] != EOS && templineC2[0] != EOS) {
				if (!isWordFromMORTier(templineC2) || templineC2[0] == '0')
					continue;
				if (uS.patmat(graWord, "*|*|BEGP") || uS.patmat(graWord, "*|*|ENDP") ||
					uS.patmat(graWord, "*|*|LP") || uS.patmat(graWord, "*|*|PUNCT")) {
					j = 0;
				} else
					ts->aw_nWSs++;
				if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
					c_qpa_error(db, TRUE);
				for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
					if (isCountNV) {
						if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) ||
							isEqual("part", feat->pos)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0) {
//									ts->matVerb++;
									ts->aw_matVerb++;
								} else if (uS.mStricmp(vb, "COORD") == 0) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL)) {
//										ts->matVerb++;
										ts->aw_matVerb++;
									}
								} else if (uS.mStricmp(vb, "ENUM") == 0) {
									if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
//										ts->matVerb++;
										ts->aw_matVerb++;
									}
								}
							}
						}
						if (isEqual("mod", feat->pos) &&
							(isEqual("could", feat->stem) || isEqual("should", feat->stem) ||
							 isEqual("would", feat->stem))) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "INCROOT") == 0) {
//									ts->matVerb++;
									ts->aw_matVerb++;
								}
							}
						}
					}
					if (isEqual("neg", feat->pos) && isEqual("not", feat->stem)) {
						if (lFeatPOS[0] != EOS) {
							if (isEqual("cop", lFeatPOS)) {
//								ts->auxScore++;
								ts->aw_auxScore++;
							} else if (isEqual("mod", lFeatPOS)) {
//								ts->auxScore++;
								ts->aw_auxScore++;
							} else if (isEqual("aux", lFeatPOS)) {
//								ts->auxScore++;
								ts->aw_auxScore++;
							}
						}
					}
					if (isEqual("coord", feat->pos) && isEqual("and", feat->stem)) {
						if (isAuxBeFound == 1)
							isAuxBeFound = 2;
					}
					if (isEqual("cop", feat->pos) || isEqual("v", feat->pos)   ||
						isEqual("aux", feat->pos) || isEqual("mod", feat->pos) ||
						isEqual("part", feat->pos)) {
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							isCountInflectionsAuxScore = FALSE;
							if (isEqual("v", feat->pos) && isEqual("inf", lFeatPOS) && isEqual("to", lFeatSTEM)) {
							} else if (isEqualIxes("3S", feat->suffix, NUM_SUFF)    ||
								isEqualIxes("13S", feat->fusion, NUM_FUSI)   || isEqualIxes("PRES", feat->fusion, NUM_FUSI) ||
								isEqualIxes("PRESP", feat->suffix, NUM_SUFF) ||
								isEqualIxes("PAST", feat->suffix, NUM_SUFF)  || isEqualIxes("PAST", feat->fusion, NUM_FUSI) ||
								isEqualIxes("PASTP", feat->suffix, NUM_SUFF) || isEqualIxes("PASTP", feat->fusion, NUM_FUSI)) {
								isCountInflectionsAuxScore = TRUE;
							} else 	if (isEqualIxes("3S", feat->fusion, NUM_FUSI)) {
								if ((isEqual("cop", feat->pos) || isEqual("aux", feat->pos)) && isEqual("be", feat->stem)) {
								} else {
									isCountInflectionsAuxScore = TRUE;
								}
							}
							if (isEqual("part", feat->pos) && isAuxBeFound == 2) {
//								ts->auxScore = ts->auxScore + auxBeScore;
								if (isEqual("part", feat->pos) && isEqual("go", feat->stem) &&
									(isFeatClitic(feat->clitc, "inf", "to") || isNextPOS(i, NULL, uttline, "inf", "to"))) {
									j = 0;
								} else {
									ts->aw_auxScore = ts->aw_auxScore + auxBeScore;
								}
							}
							curScore = 0;
							if (isCountInflectionsAuxScore) {
								if (uS.mStricmp(vb, "ROOT") == 0) {
									curScore = 2;
								} else if (uS.mStricmp(vb, "AUX") == 0) {
									if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
										curScore = 2;
									} else if (isLinkedtoTag(graWord, uttline, "COORD", NULL, NULL)) {
										curScore = 2;
									}
								} else if (uS.mStricmp(vb, "COORD") == 0) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL)) {
										curScore = 2;
									}
								}
							} else if (isEqual("v", feat->pos) && isEqual("inf", lFeatPOS) && isEqual("to", lFeatSTEM)) {
							} else if (uS.mStricmp(vb, "ROOT") == 0) {
								curScore = 1;
							} else if (uS.mStricmp(vb, "AUX") == 0) {
								if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
									curScore = 1;
								} else if (isEqual("aux", feat->pos) && isEqual("be", feat->stem)) {
									if (isLinkedtoTag(graWord, uttline, "COORD", NULL, NULL)) {
										curScore = 1;
									}
								}
							} else if (uS.mStricmp(vb, "COORD") == 0) {
								if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL)) {
									curScore = 1;
								}
							}
//							ts->auxScore = ts->auxScore + curScore;
							ts->aw_auxScore = ts->aw_auxScore + curScore;
							if (isEqual("aux", feat->pos) && isEqual("be", feat->stem)) {
								if (isAuxBeFound == 2) {
									isAuxBeFound = 1;
									auxBeScore = 0;
								} else {
									isAuxBeFound = 1;
								}
								auxBeScore = curScore;
							}
						}
					}
					strcpy(lFeatPOS, feat->pos);
					strcpy(lFeatSTEM, feat->stem);
				}
				freeUpFeats(&word_feats);
			}
		}

		if (isDebugMode == TRUE || isDebugMode == 'L' || isDebugMode == 'l') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "(K)=%0.f; (L)=%0.f:\n", ts->aw_matVerb, ts->aw_auxScore);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}

		if (isCountNV) {
//			isLastINF = FALSE;
//			ts->nWSs += ts->aw_nWSs; // 2020-05-15
			j = 0;
			i = 0;
			while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
				uS.remblanks(graWord);
				if (graWord[0] != EOS && templineC2[0] != EOS) {
					vb = strrchr(graWord, '|');
					if (vb != NULL) {
						vb++;
						if (uS.mStricmp(vb, "COMP") == 0 || uS.mStricmp(vb, "CPRED") == 0 ||
							uS.mStricmp(vb, "XJCT") == 0 || uS.mStricmp(vb, "XMOD") == 0) {
							wi = atoi(graWord);
							if (isNumItemPointedToBy(uttline, wi, "SUBJ")) {
								j++;
							}
						} else if (uS.mStricmp(vb, "CSUBJ") == 0 || uS.mStricmp(vb, "COBJ") == 0 ||
								   uS.mStricmp(vb, "CJCT") == 0  || uS.mStricmp(vb, "CMOD") == 0) {
							j++;
						}
//						if (uS.mStricmp(vb, "INF") == 0)
//							isLastINF = TRUE;
//						else
//							isLastINF = FALSE;
					}
				}
			}
//			ts->embedd = ts->embedd + j;
			ts->aw_embedd = ts->aw_embedd + j;
		}
		if (isSentenceUttsCorrected && ts->aw_ocSNP == 0.0) { //lxs PASTP
			ts->aw_ocSNP = 1.0;
		}
		if (ts->aw_sentenceUtts == 1.0) {// 2020-05-15
			ts->matVerb += ts->aw_matVerb;
			ts->auxScore += ts->aw_auxScore;
			ts->embedd += ts->aw_embedd;
			ts->SNPs += ts->aw_PSNP;
			ts->VPs += ts->aw_PVP;
			ts->ocSNP += ts->aw_ocSNP;
			ts->ocVP += ts->aw_ocVP;
			ts->nWSs += ts->aw_nWSs;
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
		if (db == NULL) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				if (isCreateDB) {
					uS.remblanks(utterance->line);
					filterAndOuputID(utterance->line);
				} else {
					uS.remblanks(utterance->line);
					c_qpa_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
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

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i') 
		return((c_qpa_group == FALSE && found) || (c_qpa_SpecWords == found));
	else 
		return((c_qpa_group == TRUE && c_qpa_SpecWords > found) || (found == 0));
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  found, t;
	char isfirst, isOutputGem, rightIDFound, isDataFound;
	char word[BUFSIZ+1], DBFname[BUFSIZ+1], IDText[BUFSIZ+1];
	float tt;
	FILE *tfpin;
	struct c_qpa_speakers *ts;
	struct IDparts IDTier;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	isfirst = TRUE;
	if (isNOptionSet == FALSE) {
		strcpy(c_qpa_BBS, "@*&#");
		strcpy(c_qpa_CBS, "@*&#");
	}
	if (c_qpa_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	if (!fgets_cr(templineC, UTTLINELEN, fpin))
		return;
	if (templineC[0] != 'V' || !isdigit(templineC[1])) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		c_qpa_error(db, FALSE);
	}
	t = atoi(templineC+1);
	if (t != C_QPA_DB_VERSION) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		c_qpa_error(db, FALSE);
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
					if (c_qpa_SpecWords)
						fprintf(stderr,"\nERROR: No specified gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					c_qpa_error(db, FALSE);
				}
#ifdef DEBUGC_QPARESULTS
				fprintf(stdout, "%s\n", IDText);
				prDebug(ts);
#endif

				db->tm_sqr = db->tm_sqr + (ts->tm * ts->tm);
				db->tm = db->tm + ts->tm;

				if (ts->tm == 0.0)
					tt = 0.0000;
				else {
					tt = ts->totalWords / (ts->tm / 60.0000);
				}
				db->WOD_sqr = db->WOD_sqr + (tt * tt);
				db->WOD = db->WOD + tt;

				db->tUtt_sqr = db->tUtt_sqr + (ts->tUtt * ts->tUtt);
				db->tUtt = db->tUtt + ts->tUtt;

				db->totalWords_sqr = db->totalWords_sqr + (ts->totalWords * ts->totalWords);
				db->totalWords = db->totalWords + ts->totalWords;

				db->openClass_sqr = db->openClass_sqr + (ts->openClass * ts->openClass);
				db->openClass = db->openClass + ts->openClass;

				db->closedClass_sqr = db->closedClass_sqr + (ts->closedClass * ts->closedClass);
				db->closedClass = db->closedClass + ts->closedClass;

				if (ts->totalWords == 0.0)
					ts->closedClassP = 0.0000;
				else
					ts->closedClassP = ts->closedClass / ts->totalWords;
				db->closedClassP_sqr = db->closedClassP_sqr + (ts->closedClassP * ts->closedClassP);
				db->closedClassP = db->closedClassP + ts->closedClassP;

				db->noun_sqr = db->noun_sqr + (ts->noun * ts->noun);
				db->noun = db->noun + ts->noun;

				db->NRDs_sqr = db->NRDs_sqr + (ts->NRDs * ts->NRDs);
				db->NRDs = db->NRDs + ts->NRDs;

				db->NRDwD_sqr = db->NRDwD_sqr + (ts->NRDwD * ts->NRDwD);
				db->NRDwD = db->NRDwD + ts->NRDwD;

				if (ts->NRDs == 0.0)
					ts->detI = 0.0000;
				else
					ts->detI = ts->NRDwD / ts->NRDs;
				db->detI_sqr = db->detI_sqr + (ts->detI * ts->detI);
				db->detI = db->detI + ts->detI;

				db->pron_sqr = db->pron_sqr + (ts->pron * ts->pron);
				db->pron = db->pron + ts->pron;

				if ((ts->noun + ts->pron) == 0.0)
					ts->pronP = 0.0000;
				else
					ts->pronP = ts->pron / (ts->noun + ts->pron);
				db->pronP_sqr = db->pronP_sqr + (ts->pronP * ts->pronP);
				db->pronP = db->pronP + ts->pronP;

				db->verb_sqr = db->verb_sqr + (ts->verb * ts->verb);
				db->verb = db->verb + ts->verb;

				if ((ts->noun + ts->verb) == 0.0)
					ts->verbP = 0.0000;
				else
					ts->verbP = ts->verb / (ts->noun + ts->verb);
				db->verbP_sqr = db->verbP_sqr + (ts->verbP * ts->verbP);
				db->verbP = db->verbP + ts->verbP;

				db->inverb_sqr = db->inverb_sqr + (ts->inverb * ts->inverb);
				db->inverb = db->inverb + ts->inverb;

				db->inverbI_sqr = db->inverbI_sqr + (ts->inverbI * ts->inverbI);
				db->inverbI = db->inverbI + ts->inverbI;

				if (ts->inverb == 0.0)
					ts->infIP = 0.0000;
				else
					ts->infIP = ts->inverbI / ts->inverb;
				db->infIP_sqr = db->infIP_sqr + (ts->infIP * ts->infIP);
				db->infIP = db->infIP + ts->infIP;

				db->matVerb_sqr = db->matVerb_sqr + (ts->matVerb * ts->matVerb);
				db->matVerb = db->matVerb + ts->matVerb;

				db->auxScore_sqr = db->auxScore_sqr + (ts->auxScore * ts->auxScore);
				db->auxScore = db->auxScore + ts->auxScore;

				if (ts->matVerb == 0.0)
					ts->auxComP = 0.0000;
				else
					ts->auxComP = (ts->auxScore / ts->matVerb) - 1;
				db->auxComP_sqr = db->auxComP_sqr + (ts->auxComP * ts->auxComP);
				db->auxComP = db->auxComP + ts->auxComP;

				db->mSs_sqr = db->mSs_sqr + (ts->mSs * ts->mSs);
				db->mSs = db->mSs + ts->mSs;

				db->nWSs_sqr = db->nWSs_sqr + (ts->nWSs * ts->nWSs);
				db->nWSs = db->nWSs + ts->nWSs;

				if (ts->totalWords == 0.0)
					ts->pWSsP = 0.0000;
				else
					ts->pWSsP = ts->nWSs / ts->totalWords;
				db->pWSsP_sqr = db->pWSsP_sqr + (ts->pWSsP * ts->pWSsP);
				db->pWSsP = db->pWSsP + ts->pWSsP;

				db->wfSs_sqr = db->wfSs_sqr + (ts->wfSs * ts->wfSs);
				db->wfSs = db->wfSs + ts->wfSs;

				if (ts->mSs == 0.0)
					ts->pWFSsP = 0.0000;
				else
					ts->pWFSsP = ts->wfSs / ts->mSs;
				db->pWFSsP_sqr = db->pWFSsP_sqr + (ts->pWFSsP * ts->pWFSsP);
				db->pWFSsP = db->pWFSsP + ts->pWFSsP;

				db->SNPs_sqr = db->SNPs_sqr + (ts->SNPs * ts->SNPs);
				db->SNPs = db->SNPs + ts->SNPs;

				db->ocSNP_sqr = db->ocSNP_sqr + (ts->ocSNP * ts->ocSNP);
				db->ocSNP = db->ocSNP + ts->ocSNP;

				if (ts->SNPs == 0.0)
					ts->mSNP = 0.0000;
				else
					ts->mSNP = ts->ocSNP / ts->SNPs;
				db->mSNP_sqr = db->mSNP_sqr + (ts->mSNP * ts->mSNP);
				db->mSNP = db->mSNP + ts->mSNP;

				db->VPs_sqr = db->VPs_sqr + (ts->VPs * ts->VPs);
				db->VPs = db->VPs + ts->VPs;

				db->ocVP_sqr = db->ocVP_sqr + (ts->ocVP * ts->ocVP);
				db->ocVP = db->ocVP + ts->ocVP;







				db->embedd_sqr = db->embedd_sqr + (ts->embedd * ts->embedd);
				db->embedd = db->embedd + ts->embedd;

				if (ts->mSs == 0.0)
					ts->embedP = 0.0000;
				else
					ts->embedP = ts->embedd / ts->mSs;
				db->embedP_sqr = db->embedP_sqr + (ts->embedP * ts->embedP);
				db->embedP = db->embedP + ts->embedP;

				db->numUtt_sqr = db->numUtt_sqr + (ts->numUtt * ts->numUtt);
				db->numUtt = db->numUtt + ts->numUtt;

				if (ts->numUtt == 0.0)
					ts->uttLenP = 0.0000;
				else
					ts->uttLenP = ts->totalWords / ts->numUtt;
				db->uttLenP_sqr = db->uttLenP_sqr + (ts->uttLenP * ts->uttLenP);
				db->uttLenP = db->uttLenP + ts->uttLenP;

				db->db_sp = freespeakers(db->db_sp);
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(c_qpa_BBS, "@*&#");
				strcpy(c_qpa_CBS, "@*&#");
			}
			if (c_qpa_SpecWords) {
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
			if (c_qpa_SpecWords && !strcmp(c_qpa_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					c_qpa_n_option = FALSE;
					strcpy(c_qpa_BBS, "@BG:");
					strcpy(c_qpa_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					c_qpa_n_option = TRUE;
					strcpy(c_qpa_BBS, "@G:");
					strcpy(c_qpa_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,c_qpa_BBS,FALSE,FALSE)) {
				if (c_qpa_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,c_qpa_CBS,FALSE,FALSE)) {
				if (c_qpa_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (c_qpa_SpecWords)
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
					ts = c_qpa_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
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
					c_qpa_process_tier(ts, db, NULL, isOutputGem, NULL, NULL);
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
		addFilename2Path(FileName1, "c-qpa");
		addFilename2Path(FileName1, lang_prefix);
		strcat(FileName1, DATABASE_FILE_NAME);
		fp = fopen(FileName1, "r");
	}
	if (fp == NULL) {
		fprintf(stderr, "Can't find database in folder \"%s\"\n", FileName1);
		fprintf(stderr, "If you do not use database, then do not include +d option in command line\n");
		c_qpa_error(db, FALSE);
	} else {
		fprintf(stderr,"Database File used: %s\n", FileName1);
		OpenDBFile(fp, db, &isDbSpFound);
		fprintf(stderr,"\r%.0f  \n", db->num);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (c_qpa_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		c_qpa_error(db, FALSE);
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

static void c_qpa_pr_result(void) {
	int    i, SDn = 0;
	char   st[1024], *sFName;
	float  num, tt;
	struct c_qpa_speakers *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;

	if (isCreateDB) {
		sp_head = freespeakers(sp_head);
		return;
	}
	if (sp_head == NULL) {
		if (c_qpa_SpecWords && !onlyApplyToDB) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n\n");
	}
	excelHeader(fpout, newfname, 75);
	excelHeightRowStart(fpout, 75);
//	excelRow(fpout, ExcelRowStart);
	if (DBKeyRoot != NULL) {
		for (i=0; i < SDRESSIZE; i++) {
			SD[i].isSDComputed = FALSE;
		}
		excelStrCell(fpout, "File/DB");
	} else
		excelStrCell(fpout, "File");
	excelCommasStrCell(fpout, "Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	excelStrCell(fpout, "Duration (sec)"); 				// tm;
	excelStrCell(fpout, "# Narrative words");			// totalWords; (A)
	excelStrCell(fpout, "# Words per Minute");
	excelStrCell(fpout, "# Open Class Words");			// openClass;  (B)
	excelStrCell(fpout, "# Closed Class Words ");		// closedClass; # Closed Class Words
	excelStrCell(fpout, "Proportion Closed Class ");	// closedClassP; Proportion Closed Class
	excelStrCell(fpout, "Nouns"); //   +|n,|n:*			// noun;   	   (C)
	excelStrCell(fpout, "# NRDs");						// NRDs;       (D)
	excelStrCell(fpout, "# NRDs w/ Determiners");		// NRDwD;      (E)
	excelStrCell(fpout, "DET Index");					// detI; DET Index
	excelStrCell(fpout, "# Pronouns");					// pron;       (F)
	excelStrCell(fpout, "Proportion Pronouns");			// pronP; Proportion Pronouns
	excelStrCell(fpout, "Verbs");						// verb;       (G)
	excelStrCell(fpout, "Proportion Verbs");			// verbP; Proportion Verbs
//	excelStrCell(fpout, "# Inflectable Verbs");			// inverb;     (H)
//	excelStrCell(fpout, "# Inflectable Verbs Inflected");//inverbI;    (J)
//	excelStrCell(fpout, "Inflection Index");			// infIP; Inflection Index
	excelStrCell(fpout, "# Matrix Verbs");				// matVerb;    (K)
	excelStrCell(fpout, "Total Aux Score");				// auxScore;   (L)
	excelStrCell(fpout, "Aux Complexity");				// auxComP; Aux Complexity
	excelStrCell(fpout, "# Ss");						// mSs;        (M)
	excelStrCell(fpout, "# Words in Ss");				// nWSs;       (N)
	excelStrCell(fpout, "Proportion Words in Ss");		// pWSsP; Proportion Words in Ss
	excelStrCell(fpout, "# Well-formed Ss");			// wfSs;       (Q)
	excelStrCell(fpout, "Proportion Well Formed Ss");	// pWFSsP; Proportion Well Formed Ss
	excelStrCell(fpout, "# SNPs");						// SNPs;       (R)
	excelStrCell(fpout, "# Words in SNPs");				// ocSNP;      (S)
	excelStrCell(fpout, "Mean SNP Length");				// mSNP; Mean SNP Length
	excelStrCell(fpout, "SNP Elaboration");				//   SNP Elaboration
	excelStrCell(fpout, "# VPs");						// VPs;        (T)
	excelStrCell(fpout, "# Words in VPs");				// ocVP;       (U)
	excelStrCell(fpout, "Mean VP Length");				//   Mean VP Length
	excelStrCell(fpout, "VP Elaboration");				//   VP Elaboration
	excelStrCell(fpout, "S Elaboration");				//   S Elaboration





	excelStrCell(fpout, "# Embeddings");				// embedd;     (V)
	excelStrCell(fpout, "Embedding index");				// embedP; Embedding index
	excelStrCell(fpout, "# Utterances");				// numUtt;     (W)
	excelStrCell(fpout, "Mean Utterance Length");		// uttLenP; Mean Utterance Length
	excelRow(fpout, ExcelRowEnd);

	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (c_qpa_SpecWords) {
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
			excelCommasStrCell(fpout, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
			excelRow(fpout, ExcelRowEnd);
			continue;
		}
//		Secs2Str(ts->tm, st, FALSE);
		RoundSecsStr(ts->tm, st);
		excelStrCell(fpout, st); // Duration (sec)
		excelNumCell(fpout, "%.0f", ts->totalWords); //Total Words, # Narrative words
		if (ts->tm == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->totalWords / (ts->tm / 60.0000)); // Words/Min

		excelNumCell(fpout, "%.0f", ts->openClass); // open-class

		excelNumCell(fpout, "%.0f", ts->closedClass); // closed-class
		if (ts->totalWords == 0.0) {
			ts->closedClassP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->closedClassP = ts->closedClass / ts->totalWords;
			excelNumCell(fpout, "%.3f", ts->closedClassP); // % closed-class/all words
		}

		excelNumCell(fpout, "%.0f", ts->noun);

		excelNumCell(fpout, "%.0f", ts->NRDs);

		excelNumCell(fpout, "%.0f", ts->NRDwD);

		if (ts->NRDs == 0.0) {
			ts->detI = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->detI = ts->NRDwD / ts->NRDs;
			excelNumCell(fpout, "%.3f", ts->detI);
		}

		excelNumCell(fpout, "%.0f", ts->pron);

		if ((ts->pron + ts->noun) == 0.0) {
			ts->pronP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->pronP = ts->pron / (ts->pron + ts->noun);
			excelNumCell(fpout, "%.3f", ts->pronP);
		}

		excelNumCell(fpout, "%.0f", ts->verb);

		if ((ts->verb + ts->noun) == 0.0) {
			ts->verbP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->verbP = ts->verb / (ts->verb + ts->noun);
			excelNumCell(fpout, "%.3f", ts->verbP);
		}
/* 2020-05-15
		excelNumCell(fpout, "%.0f", ts->inverb);

		excelNumCell(fpout, "%.0f", ts->inverbI);

		if (ts->inverb == 0.0) {
			ts->infIP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->infIP = ts->inverbI / ts->inverb;
			excelNumCell(fpout, "%.3f", ts->infIP);
		}
*/
		excelNumCell(fpout, "%.0f", ts->matVerb);

		excelNumCell(fpout, "%.0f", ts->auxScore);

		if (ts->matVerb == 0.0) {
			ts->auxComP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->auxComP = (ts->auxScore / ts->matVerb) - 1;
			excelNumCell(fpout, "%.3f", ts->auxComP);
		}

		excelNumCell(fpout, "%.0f", ts->mSs);

		excelNumCell(fpout, "%.0f", ts->nWSs);

		if (ts->totalWords == 0.0) {
			ts->pWSsP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->pWSsP = ts->nWSs / ts->totalWords;
			if (ts->pWSsP > 1.0)
				ts->pWSsP = 1.0;
			excelNumCell(fpout, "%.3f", ts->pWSsP);
		}

		excelNumCell(fpout, "%.0f", ts->wfSs);

		if (ts->mSs == 0.0) {
			ts->pWFSsP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->pWFSsP = ts->wfSs / ts->mSs;
			excelNumCell(fpout, "%.3f", ts->pWFSsP);
		}

		excelNumCell(fpout, "%.0f", ts->SNPs);

		excelNumCell(fpout, "%.0f", ts->ocSNP);

		if (ts->SNPs == 0.0) {
			ts->mSNP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->mSNP = ts->ocSNP / ts->SNPs;
			excelNumCell(fpout, "%.3f", ts->mSNP);
		}

		if (ts->SNPs == 0.0) {
			excelStrCell(fpout, "NA");
			num = 0.0;
		} else {
			num = ((ts->ocSNP / ts->SNPs) - 1);
			excelNumCell(fpout, "%.3f", num);
		}


		excelNumCell(fpout, "%.0f", ts->VPs);

		excelNumCell(fpout, "%.0f", ts->ocVP);

		if (ts->VPs == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->ocVP / ts->VPs);

		if (ts->VPs == 0.0) {
			tt = 0.0;
			excelStrCell(fpout, "NA");
		} else {
			tt = ((ts->ocVP / ts->VPs) - 1);
			excelNumCell(fpout, "%.3f", tt);
		}


		excelNumCell(fpout, "%.3f", num + tt);





		excelNumCell(fpout, "%.0f", ts->embedd);

		if (ts->mSs == 0.0) {
			ts->embedP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->embedP = ts->embedd / ts->mSs;
			excelNumCell(fpout, "%.3f", ts->embedP);
		}

		excelNumCell(fpout, "%.0f", ts->numUtt);

		if (ts->numUtt == 0.0) {
			ts->uttLenP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->uttLenP = ts->totalWords / ts->numUtt;
			excelNumCell(fpout, "%.3f", ts->uttLenP);
		}
		excelRow(fpout, ExcelRowEnd);

		if (DBKeyRoot != NULL) {
			num = db->num;
			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout,"+/-SD, , , , , , , , , , , ");

			SDn = 0; // this number should be less than SDRESSIZE = 256
			compute_SD(&SD[SDn++], ts->tm,  NULL, db->tm_sqr, db->tm, num, st);
			compute_SD(&SD[SDn++], ts->totalWords,  NULL, db->totalWords_sqr, db->totalWords, num, NULL);
			tt = ts->tm / 60.0000;
			compute_SD(&SD[SDn++], ts->totalWords,  &tt, db->WOD_sqr, db->WOD, num, NULL);
			compute_SD(&SD[SDn++], ts->openClass,  NULL, db->openClass_sqr, db->openClass, num, NULL);
			compute_SD(&SD[SDn++], ts->closedClass,  NULL, db->closedClass_sqr, db->closedClass, num, NULL);
			compute_SD(&SD[SDn++], ts->closedClassP,  NULL, db->closedClassP_sqr, db->closedClassP, num, NULL);
			compute_SD(&SD[SDn++], ts->noun,  NULL, db->noun_sqr, db->noun, num, NULL);
			compute_SD(&SD[SDn++], ts->NRDs,  NULL, db->NRDs_sqr, db->NRDs, num, NULL);
			compute_SD(&SD[SDn++], ts->NRDwD,  NULL, db->NRDwD_sqr, db->NRDwD, num, NULL);
			compute_SD(&SD[SDn++], ts->detI,  NULL, db->detI_sqr, db->detI, num, NULL);
			compute_SD(&SD[SDn++], ts->pron,  NULL, db->pron_sqr, db->pron, num, NULL);
			compute_SD(&SD[SDn++], ts->pronP,  NULL, db->pronP_sqr, db->pronP, num, NULL);
			compute_SD(&SD[SDn++], ts->verb,  NULL, db->verb_sqr, db->verb, num, NULL);
			compute_SD(&SD[SDn++], ts->verbP,  NULL, db->verbP_sqr, db->verbP, num, NULL);
/* 2020-05-15
			compute_SD(&SD[SDn++], ts->inverb,  NULL, db->inverb_sqr, db->inverb, num, NULL);
			compute_SD(&SD[SDn++], ts->inverbI,  NULL, db->inverbI_sqr, db->inverbI, num, NULL);
			compute_SD(&SD[SDn++], ts->infIP,  NULL, db->infIP_sqr, db->infIP, num, NULL);
*/
			compute_SD(&SD[SDn++], ts->matVerb,  NULL, db->matVerb_sqr, db->matVerb, num, NULL);
			compute_SD(&SD[SDn++], ts->auxScore,  NULL, db->auxScore_sqr, db->auxScore, num, NULL);
			compute_SD(&SD[SDn++], ts->auxComP,  NULL, db->auxComP_sqr, db->auxComP, num, NULL);
			compute_SD(&SD[SDn++], ts->mSs,  NULL, db->mSs_sqr, db->mSs, num, NULL);
			compute_SD(&SD[SDn++], ts->nWSs,  NULL, db->nWSs_sqr, db->nWSs, num, NULL);
			compute_SD(&SD[SDn++], ts->pWSsP,  NULL, db->pWSsP_sqr, db->pWSsP, num, NULL);
			compute_SD(&SD[SDn++], ts->wfSs,  NULL, db->wfSs_sqr, db->wfSs, num, NULL);
			compute_SD(&SD[SDn++], ts->pWFSsP,  NULL, db->pWFSsP_sqr, db->pWFSsP, num, NULL);
			compute_SD(&SD[SDn++], ts->SNPs,  NULL, db->SNPs_sqr, db->SNPs, num, NULL);
			compute_SD(&SD[SDn++], ts->ocSNP,  NULL, db->ocSNP_sqr, db->ocSNP, num, NULL);
			compute_SD(&SD[SDn++], ts->mSNP,  NULL, db->mSNP_sqr, db->mSNP, num, NULL);
			compute_SD(&SD[SDn++], ts->VPs,  NULL, db->VPs_sqr, db->VPs, num, NULL);
			compute_SD(&SD[SDn++], ts->ocVP,  NULL, db->ocVP_sqr, db->ocVP, num, NULL);




			compute_SD(&SD[SDn++], ts->embedd,  NULL, db->embedd_sqr, db->embedd, num, NULL);
			compute_SD(&SD[SDn++], ts->embedP,  NULL, db->embedP_sqr, db->embedP, num, NULL);
			compute_SD(&SD[SDn++], ts->numUtt,  NULL, db->numUtt_sqr, db->numUtt, num, NULL);
			compute_SD(&SD[SDn++], ts->uttLenP,  NULL, db->uttLenP_sqr, db->uttLenP, num, NULL);
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
		excelNumCell(fpout, "%.3f", db->totalWords/num);
		excelNumCell(fpout, "%.3f", db->WOD/num);
		excelNumCell(fpout, "%.3f", db->mluWords/num);
		excelNumCell(fpout, "%.3f", db->openClass/num);
		excelNumCell(fpout, "%.3f", db->closedClass/num);
		excelNumCell(fpout, "%.3f", db->closedClassP/num);
		excelNumCell(fpout, "%.3f", db->noun/num);
		excelNumCell(fpout, "%.3f", db->NRDs/num);
		excelNumCell(fpout, "%.3f", db->NRDwD/num);
		excelNumCell(fpout, "%.3f", db->detI/num);
		excelNumCell(fpout, "%.3f", db->pron/num);
		excelNumCell(fpout, "%.3f", db->pronP/num);
		excelNumCell(fpout, "%.3f", db->verb/num);
		excelNumCell(fpout, "%.3f", db->verbP/num);
/* 2020-05-15
		excelNumCell(fpout, "%.3f", db->inverb/num);
		excelNumCell(fpout, "%.3f", db->inverbI/num);
		excelNumCell(fpout, "%.3f", db->infIP/num);
*/
		excelNumCell(fpout, "%.3f", db->matVerb/num);
		excelNumCell(fpout, "%.3f", db->auxScore/num);
		excelNumCell(fpout, "%.3f", db->auxComP/num);
		excelNumCell(fpout, "%.3f", db->mSs/num);
		excelNumCell(fpout, "%.3f", db->nWSs/num);
		excelNumCell(fpout, "%.3f", db->pWSsP/num);
		excelNumCell(fpout, "%.3f", db->wfSs/num);
		excelNumCell(fpout, "%.3f", db->pWFSsP/num);
		excelNumCell(fpout, "%.3f", db->SNPs/num);
		excelNumCell(fpout, "%.3f", db->ocSNP/num);
		excelNumCell(fpout, "%.3f", db->mSNP/num);
		excelNumCell(fpout, "%.3f", db->VPs/num);
		excelNumCell(fpout, "%.3f", db->ocVP/num);




		excelNumCell(fpout, "%.3f", db->embedd/num);
		excelNumCell(fpout, "%.3f", db->embedP/num);
		excelNumCell(fpout, "%.3f", db->numUtt/num);
		excelNumCell(fpout, "%.3f", db->uttLenP/num);
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
			strcpy(templineC4, "# files in database: ");
			i = strlen(templineC4);
			sprintf(templineC4+i, "%.0f", db->num);
			excelRowOneStrCell(fpout, ExcelBlkCell, templineC4);
			if (specialOptionUsed) {
				excelRowOneStrCell(fpout, ExcelRedCell, "CAUTION:  Analyses that use the +b, +g, +n, +r or +s options should not be directly compared to the database because those options cannot be selected for the database in C-QPA");
			}
		}
//		printArg(targv, targc, fpout, FALSE, "");
	}
	excelFooter(fpout);
	sp_head = freespeakers(sp_head);
}

void usage() {
printf("Usage: c-qpa [bS dS eN g gS lF n %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(_CLAN_DEBUG) || defined(UNX)
	puts("+c : create C-QPA database file (c-qpa -c +t*PAR)");
	puts("	First set working directory to: ~aphasia-data");
#endif // _CLAN_DEBUG
	puts("+d0: do NOT create AS files");
	puts("+d1: debug mode for all");
	puts("+d1B:debug mode for Analysis sheet Sentence Utterance");
	puts("+d1K:debug mode for Analysis sheet # Inflectable Vs, # IVs Inflected");
	puts("+d1L:debug mode for (K) and (L)");
	puts("+d1P:debug mode for Pronouns, Nouns and Verbs");
	puts("+d1S:debug mode for Analysis sheet Phrases SNP, Open Class Words + Pron SNP");
	puts("+d1T:debug mode for Analysis sheet Phrases VP, Open Class Words + Pron VP");
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
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	mainusage(FALSE);
	puts("Example:");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70");
	puts("       c-qpa adler15a.cha +t*par +gCinderella +u +d\"control|60-70\"");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       c-qpa adler15a.cha +t*par +gCinderella +u +d\"control|60-70;6\"");
	puts("   Search database for \"Cinderella\" gems of \"Nonfluent\" subjects of any age");
	puts("       c-qpa adler15a.cha +t*par +gCinderella +u +dNonfluent");
	puts("   Just compute results for \"Cinderella\" gems of adler15a.cha. Do not compare to database");
	puts("       c-qpa adler15a.cha +t*par +gCinderella +u");
	cutt_exit(0);
}
/*
static char isSpecUttDel(char *sp, char *line, const char *uttDel) {
	int i;

	i = 0;
	while ((i=getword(sp, line, templineC2, NULL, i))) {
		if (strcmp(templineC2, uttDel) == 0)
			return(TRUE);
	}
	return(FALSE);
}
*/
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
	struct c_qpa_speakers *ts;
	FILE *PWordsListFP, *AnalysisSheetFP;
	FNType ASfile[FNSize], debugfile[FNSize];

	PWordsListFP = NULL;
	AnalysisSheetFP = NULL;
	if (isCreateDB) {
		fprintf(stdout, "%s\n", oldfname+wdOffset);
//		return;
	} else {
//ANALYSIS WORKSHEET beg
		if (last_utt != NULL) {
			last_utt[0] = EOS;
			parsfname(oldfname, ASfile, ".cqp.AS.xls");
			AnalysisSheetFP = fopen(ASfile, "w");
			if (AnalysisSheetFP == NULL) {
				fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", ASfile);
			} else {
#ifdef _MAC_CODE
				settyp(ASfile, 'TEXT', the_file_creator.out, FALSE);
#endif
				excelHeader(AnalysisSheetFP, ASfile, 75);
				excelHeightRowStart(AnalysisSheetFP, 75);
				excelStrCell(AnalysisSheetFP, "UTTERANCES");
				excelStrCell(AnalysisSheetFP, "Sentence Utterance (1)");
//				excelStrCell(AnalysisSheetFP, "TC Utterance (1)");
				excelStrCell(AnalysisSheetFP, "Other Utterance (enter)");
				excelStrCell(AnalysisSheetFP, "#Narrative Wds");
				excelStrCell(AnalysisSheetFP, "#Open Class Wds");
				excelStrCell(AnalysisSheetFP, "#Nouns");
				excelStrCell(AnalysisSheetFP, "#Ns Req DET (NRDs)");
				excelStrCell(AnalysisSheetFP, "#NRDs w/DETs");
				excelStrCell(AnalysisSheetFP, "#Pronouns");
				excelStrCell(AnalysisSheetFP, "#Verbs");
/* 2020-05-15
				excelStrCell(AnalysisSheetFP, "#Inflectable Vs (IVs)");
				excelStrCell(AnalysisSheetFP, "#IVs Inflected");
*/
				excelStrCell(AnalysisSheetFP, "#Matrix Verbs");
				excelStrCell(AnalysisSheetFP, "AUX Score");
//				excelStrCell(AnalysisSheetFP, "AUX Token");
				excelStrCell(AnalysisSheetFP, "#Embeddings");
				excelStrCell(AnalysisSheetFP, "S Well-formed?");
				excelStrCell(AnalysisSheetFP, "#Phrases SNP");
				excelStrCell(AnalysisSheetFP, "#Phrases VP");
				excelStrCell(AnalysisSheetFP, "#Open Class Words + Pron SNP");
				excelStrCell(AnalysisSheetFP, "#Open Class Words + Pron VP");
				excelStrCell(AnalysisSheetFP, "#Wds in Ss (auto. filled)");
//				excelStrCell(AnalysisSheetFP, "#Wds in TCs (auto. filled)");
				excelStrCell(AnalysisSheetFP, "Comments");
				excelRow(AnalysisSheetFP, ExcelRowEnd);
			}
		}
//ANALYSIS WORKSHEET end
		fprintf(stderr,"From file <%s>\n",oldfname);
	}
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
		strcpy(c_qpa_BBS, "@*&#");
		strcpy(c_qpa_CBS, "@*&#");
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
				if (AnalysisSheetFP != NULL) {
					excelFooter(AnalysisSheetFP);
					fclose(AnalysisSheetFP);
					fprintf(stderr, "Output file <%s>\n", ASfile);
				}
				c_qpa_error(NULL, FALSE);
			} else
				fprintf(dbfpout,"V%d\n", C_QPA_DB_VERSION);
		}
		isOutputGem = TRUE;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (utterance->speaker[0] == '*')
				break;
			if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (strncmp(utterance->line, "C-QPA DATABASE EXCLUDE", 21) == 0) {
					fprintf(stderr,"    EXCLUDED FILE: %s\n",oldfname+wdOffset);
					return;
				}
			}
		}
		rewind(fpin);
		if (isPWordsList)
			fprintf(fpout, "From file <%s>\n", oldfname);
	} else if (c_qpa_SpecWords && !onlyApplyToDB) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
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
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
				if (isPostCodeOnUtt(utterance->line, "[+ exc]")/* || isSpecUttDel(utterance->speaker, utterance->line, "+\"/.")*/)
					lRightspeaker = FALSE;
				else
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!onlyApplyToDB && c_qpa_SpecWords && !strcmp(c_qpa_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				c_qpa_n_option = FALSE;
				strcpy(c_qpa_BBS, "@BG:");
				strcpy(c_qpa_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				c_qpa_n_option = TRUE;
				strcpy(c_qpa_BBS, "@G:");
				strcpy(c_qpa_CBS, "@*&#");
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
//						ts->words_root = c_qpa_freetree(ts->words_root);
					initTSVars(ts, FALSE);
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
//						ts->words_root = c_qpa_freetree(ts->words_root);
					initTSVars(ts, FALSE);
				}
				fprintf(dbfpout, "E%s\n", utterance->line);
			} else if (uS.partcmp(utterance->speaker,"@End",FALSE,FALSE)) {
				if (ts != NULL) {
					if (isPRCreateDBRes) {
						prTSResults(ts);
						isPRCreateDBRes = FALSE;
					}
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
		if (!onlyApplyToDB && uS.partcmp(utterance->speaker,c_qpa_BBS,FALSE,FALSE)) {
			if (c_qpa_n_option) {
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
		} else if (found > 0 && uS.partcmp(utterance->speaker,c_qpa_CBS,FALSE,FALSE)) {
			if (c_qpa_n_option) {
			} else {
				if (isRightText(word)) {
					found--;
					if (found == 0) {
						if (c_qpa_SpecWords)
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
				ts = c_qpa_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
				isPWordsListFound = TRUE;
			}
			c_qpa_process_tier(ts, NULL, NULL, isOutputGem, PWordsListFP, AnalysisSheetFP);
		}
	}
//ANALYSIS WORKSHEET beg
	if (last_utt != NULL) {
		if (last_utt[0] != EOS && ts != NULL) {
			outputAnalysisSheet(AnalysisSheetFP, last_utt, ts);
			last_utt[0] = EOS;
		}
	}
	if (AnalysisSheetFP != NULL) {
		excelFooter(AnalysisSheetFP);
		fclose(AnalysisSheetFP);
		fprintf(stderr, "Output file <%s>\n", ASfile);
	}
//ANALYSIS WORKSHEET end
	if (PWordsListFP != NULL) {
		fclose(PWordsListFP);
		if (!isPWordsListFound)
			unlink(debugfile);
	}
#if !defined(CLAN_SRV)
//	fprintf(stderr, "\r	  \r");
#endif
	if (!combinput)
		c_qpa_pr_result();
}
/*
static struct VIR *parseVIR(FILE *fp, FNType *mFileName, struct VIR *root) {
	int ln;
	struct VIR *p;

	ln = 0;
	while (fgets_cr(templineC2, UTTLINELEN, fp)) {
		ln++;
		uS.remblanks(templineC2);
		if (templineC2[0] == '%' || templineC2[0] == '#' || templineC2[0] == EOS)
			continue;
		if (uS.isUTF8(templineC2) || isInvisibleHeader(templineC2) ||
			strncmp(templineC2, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		if (root == NULL) {
			root = NEW(struct VIR);
			p = root;
		} else {
			for (p=root; p->nextVIR != NULL; p=p->nextVIR) ;
			p->nextVIR = NEW(struct VIR);
			p = p->nextVIR;
		}
		if (p == NULL) {
			c_qpa_error(NULL, TRUE);
		}
		p->nextVIR = NULL;
		p->word = NULL;
		p->word = c_qpa_strsave(templineC2);
	}
	fclose(fp);
	return(root);
}

static struct VIR *readVIRFile(const char *fname, char isCheckWDdir, struct VIR *v) {
	FNType mFileName[FNSize];
	FILE *fp;

	if (isCheckWDdir) {
		strcpy(mFileName, wd_dir);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName,"r");
	} else
		fp = NULL;
	if (fp == NULL) {
		strcpy(mFileName, lib_dir);
		addFilename2Path(mFileName, "c-qpa");
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName,"r");
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate verbs file: \"%s\".\n", mFileName);
#ifdef _MAC_CODE
		fprintf(stderr, "\n");
#else
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
 #endif
		c_qpa_error(NULL, FALSE);
	} else {
		v = parseVIR(fp, mFileName, v);
	}
	fprintf(stderr,"    Using irregular verbs file: %s\n", mFileName);
	return(v);
}
*/
void init(char first) {
	int  i;
	char *f;
	FNType debugfile[FNSize];

	if (first) {
//		lang_prefix = NULL;
		lang_prefix = "eng";
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
//		addword('\0','\0',"+0*");
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
		c_qpa_SpecWords = 0;
		c_qpa_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		c_qpa_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(c_qpa_BBS, "@*&#");
		strcpy(c_qpa_CBS, "@*&#");
		isSpeakerNameGiven = FALSE;
		virs = NULL;
		sp_head = NULL;
		db = NULL;
		DBKeyRoot = NULL;
		DBGemNum = 0;
		combinput = TRUE;
		if (isCreateDB) {
			stout = TRUE;
			isRecursive = TRUE;
		}
	} else {
		if (ftime) {
			ftime = FALSE;
			if (lang_prefix == NULL) {
				fprintf(stderr,"Please specify language script file name with \"+l\" option.\n");
				fprintf(stderr,"For example, \"c-qpa +leng\" or \"c-qpa +leng.cut\".\n");
				cutt_exit(0);
			}
			if (isExcludeWords && DBKeyRoot != NULL) {
				fprintf(stderr, "ERROR:  Analyses that use the +s option cannot be compared to the database, \n");
				fprintf(stderr, "        because +s option cannot be applied to the database in C-QPA\n");
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
				c_qpa_error(NULL, FALSE);
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
				c_qpa_error(NULL, FALSE);
			}
// 2018-10-23			virs = readVIRFile("v-irr.cut", TRUE, virs);
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
					c_qpa_error(db, TRUE);
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
	isCountMSs = TRUE;
	if (!combinput || first) {
		sp_head = NULL;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i, fileArgcStart, fileArgcEnd, langArgc;
	char filesPath[2][128], langPref[32];
	char isFileGiven, isLangGiven, isASfile;

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
	CLAN_PROG_NUM = C_QPA;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	DBFilesListFP = NULL;
	isCreateDB = 0;
	isDebugMode = FALSE;
	isASfile = TRUE;
	isFileGiven = FALSE;
	isLangGiven = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'c') {
				isCreateDB = 1;
			} else if (argv[i][1] == 'l') {
				isLangGiven = TRUE;
			} else if (argv[i][1] == 'd') {
				if (argv[i][2] == '0') {
					isASfile = FALSE;
				} else if (argv[i][2] == '1') {
					isASfile = FALSE;
					if (argv[i][3] == EOS) {
						isDebugMode = TRUE;
					} else {
						isDebugMode = argv[i][3];
					}
				}
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
		last_utt = NULL;
		if (!isLangGiven) {
			langArgc = argc;
			strcpy(langPref, "+leng");
			argv[langArgc] = langPref;
			argc++;
		}
		if (!isFileGiven) {
			fileArgcStart = argc;
			strcpy(filesPath[0], wd_dir);
			addFilename2Path(filesPath[0], "English/Aphasia/*.cha");
			argv[argc] = filesPath[0];
			argc++;
			strcpy(filesPath[1], wd_dir);
			addFilename2Path(filesPath[1], "English/Control/*.cha");
			argv[argc] = filesPath[1];
			argc++;
			fileArgcEnd = argc;
		}
	} else {
		if (isASfile)
			last_utt = (char *)malloc(UTTLINELEN+2);
		else
			last_utt = NULL;
	}
	bmain(argc,argv,c_qpa_pr_result);

	virs = freeVIR(virs);
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
	if (last_utt != NULL)
		free(last_utt);
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
			} else if (*f == '0' || *f == '1') {
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
					c_qpa_group = TRUE;
					specialOptionUsed = TRUE;
				} else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				c_qpa_SpecWords++;
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
				c_qpa_n_option = TRUE;
				strcpy(c_qpa_BBS, "@G:");
				strcpy(c_qpa_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(c_qpa_BBS, "@BG:");
				strcpy(c_qpa_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
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
					c_qpa_error(NULL, FALSE);
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
