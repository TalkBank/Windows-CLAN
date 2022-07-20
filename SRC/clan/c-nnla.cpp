/**********************************************************************
 "Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
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
 db_sp
 sp_head
 c_nnla_FindSpeaker

 NoV_sqr, NoV, mn_NoV, mx_NoV;
 OPoCl_sqr, OPoCl, mn_OPoCl, mx_OPoCl;
*/


/* debugging
#define DEBUGC_NNLARESULTS
 prDebug
*/

#define CHAT_MODE 1
#include "cu.h"
#include <math.h>
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main c_nnla_main
#define call c_nnla_call
#define getflag c_nnla_getflag
#define init c_nnla_init
#define usage c_nnla_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define C_NNLA_DB_VERSION 1

#define aecsMax 20
#define NUMGEMITEMS 10
#define DATABASE_FILE_NAME "_c-nnla_db.cut"
#define DATABSEFILESLIST "0c-nnla_Database_IDs.cex"

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

struct Vs {
	char isV;
	char isPart;
	char *word;
	struct Vs *nextV;
} ;

struct SDs {
	char isSDComputed;
	float dbSD;
	char stars;
} ;

struct c_nnla_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;

	float tm;
	float tUtt;
	float totalWords;	// total number of words
	float mluWords;
	float mluUtt;
	float openClass;
	float openClassP;
	float closedClass;
	float closedClassP;
	float noun;
	float nounP;
	float verb;
	float verbP;
	float adj;
	float adv;
	float det;
	float pro;
	float aux;
	float conj;
	float comp;
	float mod;
	float prep;
	float neg;
	float inf;
	float qn;
	float whwd;
//	float part;
	float dCP;
	float dSP;
	float poss;
	float dPL;
	float aPL;
	float d3S;
	float dPAST;
	float aPAST;
	float dPASTP;
	float aPASTP;
	float dPRESP;
	float griv;  // error free regular inflected verbs - RIV_sqr, RIV (1)
	float ariv;  // all regular inflected verbs - RIV_sqr, RIV
	float giiv;  // error free irregular inflected verbs - IIV_sqr, IIV (2)
	float aiiv;  // all irregular inflected verbs - IIV_sqr, IIV
	float aspt;  // all SPEAKER TIERs except when xxx or &= is by itself - aspt
	float sntp;  // sentences produced - sntp, aspt, SNTP_sqr, SNTP (3)
	float scss;  // sentences with correct syntax, semantics - scss, sntp, SCSS_sqr, SCSS (4)
	float sfsyn; // sentences with flawed syntax - sfsyn, sntp, SFSYN_sqr, SFSYN (5)
	float sfsem; // sentences with flawed semantics - sfsem, sntp, SFSEM_sqr, SFSEM (6)
	float scrc;  // sentence complexity ratio - complexs - SCRCS_sqr, SCRCS (7)
	float scrs;  // sentence complexity ratio - simple - SCRCS_sqr, SCRCS (7)
	int   aecsTable[aecsMax]; // # embedded clauses/sentence - aecs - AECS_sqr, AECS (8)
	float aecs;  // # embedded clauses/sentence - aecs - AECS_sqr, AECS (8)
	float espt;  // # all embedded clauses sentences - espt - AECS_sqr, AECS (8)
	float allv;  // all verbs for 1-place verbs, 2-place verbs and 3-place verbs over all verbs - allv
	float pv1;   // 1-place verbs/all_verbs - pv1, allv, PV1_sqr, PV1
	float pv2;   // 2-place verbs/all_verbs - pv2, allv, PV2_sqr, PV2
	float pv3;   // 3-place verbs/all_verbs - pv3, allv, PV3_sqr, PV3
	float pvg1;  // 1-place verbs with correct argument structure
	float pvg2;  // 2-place verbs with correct argument structure
	float pvg3;  // 3-place verbs with correct argument structure
	struct c_nnla_speakers *next_sp;
} ;

/*
	c_nnla_speakers	database

	initTSVars	init_db

	retrieveTS		prDebug		prTSResults

	c_nnla_process_tier

 	OpenDBFile

	c_nnla_pr_result
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
	float OPoCl_sqr, OPoCl;
	float noun_sqr, noun;
	float nounP_sqr, nounP;
	float verb_sqr, verb;
	float verbP_sqr, verbP;
	float NoV_sqr, NoV;
	float adj_sqr, adj;
	float adv_sqr, adv;
	float det_sqr, det;
	float pro_sqr, pro;
	float aux_sqr, aux;
	float conj_sqr, conj;
	float comp_sqr, comp;
	float mod_sqr, mod;
	float prep_sqr, prep;
	float neg_sqr, neg;
	float inf_sqr, inf;
	float qn_sqr, qn;
	float whwd_sqr, whwd;
//	float part_sqr, part;
	float dCP_sqr, dCP;
	float dSP_sqr, dSP;
	float poss_sqr, poss;
	float dPL_sqr, dPL;
	float aPL_sqr, aPL;
	float d3S_sqr, d3S;
	float dPAST_sqr, dPAST;
	float aPAST_sqr, aPAST;
	float dPASTP_sqr, dPASTP;
	float aPASTP_sqr, aPASTP;
	float dPRESP_sqr, dPRESP;
	float RIV_sqr, RIV;     // regular inflected verbs - griv, ariv
	float IIV_sqr, IIV;     // irregular inflected verbs - giiv, aiiv
	float SNTP_sqr, SNTP;   // sentences produced - sntp, aspt
	float SCSS_sqr, SCSS;   // sentences with correct syntax, semantics - scss, sntp
	float SFSYN_sqr, SFSYN; // entences with flawed syntax - sfsyn, sntp
	float SFSEM_sqr, SFSEM; // sentences with flawed semantics - sfsem, sntp
	float SCRCS_sqr, SCRCS; // irregular inflected verbs - scrc, scrs
	float AECS_sqr, AECS;   // # embedded clauses/sentence - aecs, espt
	float PV1_sqr, PV1;     // 1-place verbs/all_verbs - pv1, allv
	float PV2_sqr, PV2;     // 2-place verbs/all_verbs - pv2, allv
	float PV3_sqr, PV3;     // 3-place verbs/all_verbs - pv3, allv
	float pvg1_sqr, pvg1;   // 1-place verbs with correct argument structure
	float pvg2_sqr, pvg2;   // 1-place verbs with correct argument structure
	float pvg3_sqr, pvg3;   // 1-place verbs with correct argument structure
	struct c_nnla_speakers *db_sp;
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
static int  c_nnla_SpecWords, DBGemNum, wdOffset;
static char *DBGems[NUMGEMITEMS];
static const char *lang_prefix;
static char isSpeakerNameGiven, isExcludeWords, ftime, isPWordsList, isDBFilesList, isCreateDB, specialOptionUsed, isDebugMode;
static char c_nnla_BBS[5], c_nnla_CBS[5], c_nnla_group, c_nnla_n_option, isNOptionSet, onlyApplyToDB, isGOptionSet, GemMode;
static float tmDur;
static FILE	*dbfpout;
static FILE *DBFilesListFP;
static struct DBKeys *DBKeyRoot;
static struct c_nnla_speakers *sp_head;
static struct database *db;
static struct Vs *v1, *v2op, *v2ob, *v3op, *v3ob;


static struct c_nnla_speakers *freespeakers(struct c_nnla_speakers *p) {
	struct c_nnla_speakers *ts;

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

static struct Vs *freeVs(struct Vs *p) {
	struct Vs *t;

	while (p) {
		t = p;
		p = p->nextV;
		if (t->word != NULL)
			free(t->word);
		free(t);
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

static void c_nnla_error(struct database *db, char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	v1 = freeVs(v1);
	v2op = freeVs(v2op);
	v2ob = freeVs(v2ob);
	v3op = freeVs(v3op);
	v3ob = freeVs(v3ob);
	sp_head = freespeakers(sp_head);
	DBKeyRoot = freeDBKeys(DBKeyRoot);
	db = freedb(db);
	cutt_exit(0);
}

static char *extractFloatFromLine(char *line, float *val, struct database *db) {
	float num;

	if (*line == EOS) {
		fprintf(stderr, "\nERROR: Database is corrupt\n");
		c_nnla_error(db, FALSE);
	}
	num = atof(line);
	*val = *val + num;
	while (isdigit(*line) || *line == '.')
		line++;
	while (isSpace(*line))
		line++;
	return(line);
}

static void initTSVars(struct c_nnla_speakers *ts, char isAll) {
	int i;

	ts->isMORFound	= FALSE;
	ts->isPSDFound	= FALSE;
	ts->tm			= (float)0.0;
	ts->tUtt		= (float)0.0;
	ts->totalWords	= (float)0.0;
	ts->mluWords	= (float)0.0;
	ts->mluUtt		= (float)0.0;
	ts->openClass	= (float)0.0;
	ts->openClassP	= (float)0.0;
	ts->closedClass = (float)0.0;
	ts->closedClassP= (float)0.0;
	ts->noun		= (float)0.0;
	ts->nounP		= (float)0.0;
	ts->verb		= (float)0.0;
	ts->verbP		= (float)0.0;
	ts->adj			= (float)0.0;
	ts->adv			= (float)0.0;
	ts->det			= (float)0.0;
	ts->pro			= (float)0.0;
	ts->aux			= (float)0.0;
	ts->conj		= (float)0.0;
	ts->comp		= (float)0.0;
	ts->mod			= (float)0.0;
	ts->prep		= (float)0.0;
	ts->neg			= (float)0.0;
	ts->inf			= (float)0.0;
	ts->qn			= (float)0.0;
	ts->whwd		= (float)0.0;
//	ts->part		= (float)0.0;
	ts->dCP			= (float)0.0;
	ts->dSP			= (float)0.0;
	ts->poss		= (float)0.0;
	ts->dPL			= (float)0.0;
	ts->aPL			= (float)0.0;
	ts->d3S			= (float)0.0;
	ts->dPAST		= (float)0.0;
	ts->aPAST		= (float)0.0;
	ts->dPASTP		= (float)0.0;
	ts->aPASTP		= (float)0.0;
	ts->dPRESP		= (float)0.0;
	ts->griv		= (float)0.0;
	ts->ariv		= (float)0.0;
	ts->giiv		= (float)0.0;
	ts->aiiv		= (float)0.0;
	ts->aspt		= (float)0.0;
	ts->sntp		= (float)0.0;
	ts->scss		= (float)0.0;
	ts->sfsyn		= (float)0.0;
	ts->sfsem		= (float)0.0;
	ts->scrc		= (float)0.0;
	ts->scrs		= (float)0.0;
	for (i=0; i < aecsMax; i++)
		ts->aecsTable[i] = 0;
	ts->aecs		= (float)0.0;
	ts->espt		= (float)0.0;
	ts->allv		= (float)0.0;
	ts->pv1			= (float)0.0;
	ts->pv2			= (float)0.0;
	ts->pv3			= (float)0.0;
	ts->pvg1		= (float)0.0;
	ts->pvg2		= (float)0.0;
	ts->pvg3		= (float)0.0;
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
	p->OPoCl_sqr = (float)0.0; p->OPoCl = (float)0.0;
	p->noun_sqr = (float)0.0; p->noun = (float)0.0;
	p->nounP_sqr = (float)0.0; p->nounP = (float)0.0;
	p->verb_sqr = (float)0.0; p->verb = (float)0.0;
	p->verbP_sqr = (float)0.0; p->verbP = (float)0.0;
	p->NoV_sqr = (float)0.0; p->NoV = (float)0.0;
	p->adj_sqr = (float)0.0; p->adj = (float)0.0;
	p->adv_sqr = (float)0.0; p->adv = (float)0.0;
	p->det_sqr = (float)0.0; p->det = (float)0.0;
	p->pro_sqr = (float)0.0; p->pro = (float)0.0;
	p->aux_sqr = (float)0.0; p->aux = (float)0.0;
	p->conj_sqr = (float)0.0; p->conj = (float)0.0;
	p->comp_sqr = (float)0.0; p->comp = (float)0.0;
	p->mod_sqr = (float)0.0; p->mod = (float)0.0;
	p->prep_sqr = (float)0.0; p->prep = (float)0.0;
	p->neg_sqr = (float)0.0; p->neg = (float)0.0;
	p->inf_sqr = (float)0.0; p->inf = (float)0.0;
	p->qn_sqr = (float)0.0; p->qn = (float)0.0;
	p->whwd_sqr = (float)0.0; p->whwd = (float)0.0;
//	p->part_sqr = (float)0.0; p->part = (float)0.0;
	p->dCP_sqr = (float)0.0; p->dCP = (float)0.0;
	p->dSP_sqr = (float)0.0; p->dSP = (float)0.0;
	p->poss_sqr = (float)0.0; p->poss = (float)0.0;
	p->dPL_sqr = (float)0.0; p->dPL = (float)0.0;
	p->aPL_sqr = (float)0.0; p->aPL = (float)0.0;
	p->d3S_sqr = (float)0.0; p->d3S = (float)0.0;
	p->dPAST_sqr = (float)0.0; p->dPAST = (float)0.0;
	p->aPAST_sqr = (float)0.0; p->aPAST = (float)0.0;
	p->dPASTP_sqr = (float)0.0; p->dPASTP = (float)0.0;
	p->aPASTP_sqr = (float)0.0; p->aPASTP = (float)0.0;
	p->dPRESP_sqr = (float)0.0; p->dPRESP = (float)0.0;
	p->RIV_sqr = (float)0.0; p->RIV = (float)0.0; // griv, ariv
	p->IIV_sqr = (float)0.0; p->IIV = (float)0.0; // giiv, aiiv
	p->SNTP_sqr = (float)0.0; p->SNTP = (float)0.0; // sntp, aspt
	p->SCSS_sqr = (float)0.0; p->SCSS = (float)0.0; // scss, sntp
	p->SFSYN_sqr = (float)0.0; p->SFSYN = (float)0.0; // sfsyn, sntp
	p->SFSEM_sqr = (float)0.0; p->SFSEM = (float)0.0; // sfsem, sntp
	p->SCRCS_sqr = (float)0.0; p->SCRCS = (float)0.0; // scrc, scrs
	p->AECS_sqr = (float)0.0; p->AECS = (float)0.0; // aecs, espt
	p->PV1_sqr = (float)0.0; p->PV1 = (float)0.0; // pv1, allv
	p->PV2_sqr = (float)0.0; p->PV2 = (float)0.0; // pv2, allv
	p->PV3_sqr = (float)0.0; p->PV3 = (float)0.0; // pv3, allv
	p->pvg1_sqr = (float)0.0; p->pvg1 = (float)0.0;
	p->pvg2_sqr = (float)0.0; p->pvg2 = (float)0.0;
	p->pvg3_sqr = (float)0.0; p->pvg3 = (float)0.0;
	p->db_sp = NULL;
}

static void retrieveTS(struct c_nnla_speakers *ts, char *line, struct database *db) {
	while (isSpace(*line))
		line++;

	line = extractFloatFromLine(line, &ts->tm, db);
	line = extractFloatFromLine(line, &ts->tUtt, db);
	line = extractFloatFromLine(line, &ts->totalWords, db);
	line = extractFloatFromLine(line, &ts->mluWords, db);
	line = extractFloatFromLine(line, &ts->mluUtt, db);
	line = extractFloatFromLine(line, &ts->openClass, db);
	line = extractFloatFromLine(line, &ts->closedClass, db);
	line = extractFloatFromLine(line, &ts->noun, db);
	line = extractFloatFromLine(line, &ts->verb, db);
	line = extractFloatFromLine(line, &ts->adj, db);
	line = extractFloatFromLine(line, &ts->adv, db);
	line = extractFloatFromLine(line, &ts->det, db);
	line = extractFloatFromLine(line, &ts->pro, db);
	line = extractFloatFromLine(line, &ts->aux, db);
	line = extractFloatFromLine(line, &ts->conj, db);
	line = extractFloatFromLine(line, &ts->comp, db);
	line = extractFloatFromLine(line, &ts->mod, db);
	line = extractFloatFromLine(line, &ts->prep, db);
	line = extractFloatFromLine(line, &ts->neg, db);
	line = extractFloatFromLine(line, &ts->inf, db);
	line = extractFloatFromLine(line, &ts->qn, db);
	line = extractFloatFromLine(line, &ts->whwd, db);
//	line = extractFloatFromLine(line, &ts->part, db);
	line = extractFloatFromLine(line, &ts->dCP, db);
	line = extractFloatFromLine(line, &ts->dSP, db);
	line = extractFloatFromLine(line, &ts->poss, db);
	line = extractFloatFromLine(line, &ts->dPL, db);
	line = extractFloatFromLine(line, &ts->aPL, db);
	line = extractFloatFromLine(line, &ts->d3S, db);
	line = extractFloatFromLine(line, &ts->dPAST, db);
	line = extractFloatFromLine(line, &ts->aPAST, db);
	line = extractFloatFromLine(line, &ts->dPASTP, db);
	line = extractFloatFromLine(line, &ts->aPASTP, db);
	line = extractFloatFromLine(line, &ts->dPRESP, db);
	line = extractFloatFromLine(line, &ts->griv, db);
	line = extractFloatFromLine(line, &ts->ariv, db);
	line = extractFloatFromLine(line, &ts->giiv, db);
	line = extractFloatFromLine(line, &ts->aiiv, db);
	line = extractFloatFromLine(line, &ts->aspt, db); // all sentences
	line = extractFloatFromLine(line, &ts->sntp, db);
	line = extractFloatFromLine(line, &ts->scss, db);
	line = extractFloatFromLine(line, &ts->sfsyn, db);
	line = extractFloatFromLine(line, &ts->sfsem, db);
	line = extractFloatFromLine(line, &ts->scrc, db);
	line = extractFloatFromLine(line, &ts->scrs, db);
	line = extractFloatFromLine(line, &ts->aecs, db);
	line = extractFloatFromLine(line, &ts->espt, db);
	line = extractFloatFromLine(line, &ts->allv, db); // all verbs
	line = extractFloatFromLine(line, &ts->pv1, db);
	line = extractFloatFromLine(line, &ts->pv2, db);
	line = extractFloatFromLine(line, &ts->pv3, db);
	line = extractFloatFromLine(line, &ts->pvg1, db);
	line = extractFloatFromLine(line, &ts->pvg2, db);
	line = extractFloatFromLine(line, &ts->pvg3, db);
}

#ifdef DEBUGC_NNLARESULTS
static void prDebug(struct c_nnla_speakers *ts) {
	fprintf(stdout, "tm=%.0f\n", ts->tm);
	fprintf(stdout, "tUtt=%.0f\n", ts->tUtt);
	fprintf(stdout, "totalWords=%.0f\n", ts->totalWords);
	fprintf(stdout, "mluWords=%.0f\n", ts->mluWords);
	fprintf(stdout, "mluUtt=%.0f\n", ts->mluUtt);
	fprintf(stdout, "openClass=%.0f\n", ts->openClass);
	fprintf(stdout, "closedClass=%.0f\n", ts->closedClass);
	fprintf(stdout, "noun=%.0f\n", ts->noun);
	fprintf(stdout, "verb=%.0f\n", ts->verb);
	fprintf(stdout, "adj=%.0f\n", ts->adj);
	fprintf(stdout, "adv=%.0f\n", ts->adv);
	fprintf(stdout, "det=%.0f\n", ts->det);
	fprintf(stdout, "pro=%.0f\n", ts->pro);
	fprintf(stdout, "aux=%.0f\n", ts->aux);
	fprintf(stdout, "conj=%.0f\n", ts->conj);
	fprintf(stdout, "comp=%.0f\n", ts->comp);
	fprintf(stdout, "mod=%.0f\n", ts->mod);
	fprintf(stdout, "prep=%.0f\n", ts->prep);
	fprintf(stdout, "neg=%.0f\n", ts->neg);
	fprintf(stdout, "inf=%.0f\n", ts->inf);
	fprintf(stdout, "qn=%.0f\n", ts->qn);
	fprintf(stdout, "whwd=%.0f\n", ts->whwd);
//	fprintf(stdout, "part=%.0f\n", ts->part);
	fprintf(stdout, "dCP=%.0f\n", ts->dCP);
	fprintf(stdout, "dSP=%.0f\n", ts->dSP);
	fprintf(stdout, "~poss|s=%.0f\n", ts->poss);
	fprintf(stdout, "-PL=%.0f\n", ts->dPL);
	fprintf(stdout, "&PL=%.0f\n", ts->aPL);
	fprintf(stdout, "-3S=%.0f\n", ts->d3S);
	fprintf(stdout, "-PAST=%.0f\n", ts->dPAST);
	fprintf(stdout, "&PAST=%.0f\n", ts->aPAST);
	fprintf(stdout, "-PASTP=%.0f\n", ts->dPASTP);
	fprintf(stdout, "&PASTP=%.0f\n", ts->aPASTP);
	fprintf(stdout, "-PRESP=%.0f\n", ts->dPRESP);
	fprintf(stdout, "griv=%.0f\n", ts->griv);
	fprintf(stdout, "ariv=%.0f\n", ts->ariv);
	fprintf(stdout, "giiv=%.0f\n", ts->giiv);
	fprintf(stdout, "aiiv=%.0f\n", ts->aiiv);
	fprintf(stdout, "aspt=%.0f\n", ts->aspt); // all sentences
	fprintf(stdout, "sntp=%.0f\n", ts->sntp);
	fprintf(stdout, "scss=%.0f\n", ts->scss);
	fprintf(stdout, "sfsyn=%.0f\n", ts->sfsyn);
	fprintf(stdout, "sfsem=%.0f\n", ts->sfsem);
	fprintf(stdout, "scrc=%.0f\n", ts->scrc);
	fprintf(stdout, "scrs=%.0f\n", ts->scrs);
	fprintf(stdout, "aecs=%.0f\n", ts->aecs);
	fprintf(stdout, "espt=%.0f\n", ts->espt);
	fprintf(stdout, "allv=%.0f\n", ts->allv); // all verbs
	fprintf(stdout, "pv1=%.0f\n", ts->pv1);
	fprintf(stdout, "pv2=%.0f\n", ts->pv2);
	fprintf(stdout, "pv3=%.0f\n", ts->pv3);
	fprintf(stdout, "pvg1=%.0f\n", ts->pvg1);
	fprintf(stdout, "pvg2=%.0f\n", ts->pvg2);
	fprintf(stdout, "pvg3=%.0f\n", ts->pvg3);
}
#endif

static void prTSResults(struct c_nnla_speakers *ts) {
	int i;
	const char *format;

	format = "%.0f ";
	//	format = "%f ";
	fprintf(dbfpout, format, ts->tm);
	fprintf(dbfpout, format, ts->tUtt);
	fprintf(dbfpout, format, ts->totalWords);
	fprintf(dbfpout, format, ts->mluWords);
	fprintf(dbfpout, format, ts->mluUtt);
	fprintf(dbfpout, format, ts->openClass);
	fprintf(dbfpout, format, ts->closedClass);
	fprintf(dbfpout, format, ts->noun);
	fprintf(dbfpout, format, ts->verb);
	fprintf(dbfpout, format, ts->adj);
	fprintf(dbfpout, format, ts->adv);
	fprintf(dbfpout, format, ts->det);
	fprintf(dbfpout, format, ts->pro);
	fprintf(dbfpout, format, ts->aux);
	fprintf(dbfpout, format, ts->conj);
	fprintf(dbfpout, format, ts->conj);
	fprintf(dbfpout, format, ts->comp);
	fprintf(dbfpout, format, ts->prep);
	fprintf(dbfpout, format, ts->neg);
	fprintf(dbfpout, format, ts->inf);
	fprintf(dbfpout, format, ts->qn);
	fprintf(dbfpout, format, ts->whwd);
//	fprintf(dbfpout, format, ts->part);
	fprintf(dbfpout, format, ts->dCP);
	fprintf(dbfpout, format, ts->dSP);
	fprintf(dbfpout, format, ts->poss);
	fprintf(dbfpout, format, ts->dPL);
	fprintf(dbfpout, format, ts->aPL);
	fprintf(dbfpout, format, ts->d3S);
	fprintf(dbfpout, format, ts->dPAST);
	fprintf(dbfpout, format, ts->aPAST);
	fprintf(dbfpout, format, ts->dPASTP);
	fprintf(dbfpout, format, ts->aPASTP);
	fprintf(dbfpout, format, ts->dPRESP);
	fprintf(dbfpout, format, ts->griv);
	fprintf(dbfpout, format, ts->ariv);
	fprintf(dbfpout, format, ts->giiv);
	fprintf(dbfpout, format, ts->aiiv);
	fprintf(dbfpout, format, ts->aspt); // all sentences
	fprintf(dbfpout, format, ts->sntp);
	fprintf(dbfpout, format, ts->scss);
	fprintf(dbfpout, format, ts->sfsyn);
	fprintf(dbfpout, format, ts->sfsem);
	fprintf(dbfpout, format, ts->scrc);
	fprintf(dbfpout, format, ts->scrs);
	ts->aecs = 0.0;
	ts->espt = 0.0;
	for (i=1; i < aecsMax; i++) {
		ts->aecs = ts->aecs + (float)(i * ts->aecsTable[i]);
		ts->espt = ts->espt + (float)ts->aecsTable[i];
	}
	fprintf(dbfpout, format, ts->aecs);
	fprintf(dbfpout, format, ts->espt);
	fprintf(dbfpout, format, ts->allv); // all verbs
	fprintf(dbfpout, format, ts->pv1);
	fprintf(dbfpout, format, ts->pv2);
	fprintf(dbfpout, format, ts->pv3);
	fprintf(dbfpout, format, ts->pvg1);
	fprintf(dbfpout, format, ts->pvg2);
	fprintf(dbfpout, format, ts->pvg3);
	putc('\n', dbfpout);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		c_nnla_error(NULL, FALSE);
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

static char *c_nnla_strsave(const char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		c_nnla_error(NULL, FALSE);
	return(p);
}

static struct DBKeys *initDBKey(const char *k1, char *k2, char *k3, char *k4, char *a, int af, int at, char *s) {
	struct DBKeys *p;

	p = NEW(struct DBKeys);
	if (p == NULL)
		c_nnla_error(NULL, TRUE);
	if (k1 == NULL)
		p->key1 = NULL;
	else
		p->key1 = c_nnla_strsave(k1);
	if (k2 == NULL)
		p->key2 = NULL;
	else
		p->key2 = c_nnla_strsave(k2);
	if (k3 == NULL)
		p->key3 = NULL;
	else
		p->key3 = c_nnla_strsave(k3);
	if (k4 == NULL)
		p->key4 = NULL;
	else
		p->key4 = c_nnla_strsave(k4);
	if (a == NULL)
		p->age = NULL;
	else
		p->age = c_nnla_strsave(a);
	p->agef = af;
	p->aget = at;
	if (s == NULL)
		p->sex = NULL;
	else
		p->sex = c_nnla_strsave(s);
	p->next = NULL;
	return(p);
}

static void multiplyDBKeys(struct DBKeys *DBKey) {
	struct DBKeys *p, *cDBKey;

	if (DBKey->key1 == NULL)
		return;
	if (uS.mStricmp(DBKey->key1,"Fluent") == 0) {
		free(DBKey->key1);
		DBKey->key1 = c_nnla_strsave("Wernicke");
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
		DBKey->key1 = c_nnla_strsave("Global");
		for (cDBKey=DBKeyRoot; cDBKey->next != NULL; cDBKey=cDBKey->next) ;
		p = initDBKey("Broca",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
		p = initDBKey("Transmotor",DBKey->key2,DBKey->key3,DBKey->key4,DBKey->age,DBKey->agef,DBKey->aget,DBKey->sex);
		cDBKey->next = p;
		cDBKey = cDBKey->next;
	} else if (uS.mStricmp(DBKey->key1,"AllAphasia") == 0) {
		free(DBKey->key1);
		DBKey->key1 = c_nnla_strsave("Wernicke");
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
					DBKey->key1 = c_nnla_strsave(b);
				else if (DBKey->key2 == NULL)
					DBKey->key2 = c_nnla_strsave(b);
				else if (DBKey->key3 == NULL)
					DBKey->key3 = c_nnla_strsave(b);
				else if (DBKey->key4 == NULL)
					DBKey->key4 = c_nnla_strsave(b);
			} else if (uS.mStricmp(b,"male")==0 || uS.mStricmp(b,"female")==0) {
				DBKey->sex = c_nnla_strsave(b);
			} else if (isAge(b, &DBKey->agef, &DBKey->aget)) {
				if (DBKey->aget == 0) {
					fprintf(stderr, "\nERROR: Please specify the age range instead of just \"%s\"\n", b);
					fprintf(stderr, "For example: \"60-80\" means people between 60 and 80 years old.\n");
					c_nnla_error(NULL, FALSE);
				}
				DBKey->age = c_nnla_strsave(b);
			} else {
				fprintf(stderr, "\nUnrecognized keyword specified with +d option: \"%s\"\n", b);
				fprintf(stderr, "Choices are:\n");
				fprintf(stderr, "    Anomic, Global, Broca, Wernicke, TransSensory, TransMotor, Conduction, NotAphasicByWAB, control, \n");
				fprintf(stderr, "    Fluent = (Wernicke, TransSensory, Conduction, Anomic, NotAphasicByWAB),\n");
				fprintf(stderr, "    Nonfluent = (Global, Broca, Transmotor),\n");
				fprintf(stderr, "    AllAphasia = (Wernicke, TransSensory, Conduction, Anomic, Global, Broca, Transmotor, NotAphasicByWAB)\n");
				c_nnla_error(NULL, FALSE);
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

static struct c_nnla_speakers *c_nnla_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound, struct database *db) {
	struct c_nnla_speakers *ts, *tsp;

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
			c_nnla_error(db, FALSE);
		}
	}
	if ((ts=NEW(struct c_nnla_speakers)) == NULL)
		c_nnla_error(db, TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		c_nnla_error(db, TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		c_nnla_error(db, TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL || isCreateDB)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			c_nnla_error(db, TRUE);
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

static void isCountUtt(char *line, char *morline, struct c_nnla_speakers *ts, int *tsntp) {
	int i;
	char isCountSntp, isCountScss, isCountSfsyn, isCountSfsem, *POS;

	isCountSntp =  FALSE;
	isCountScss =  FALSE;
	isCountSfsyn = 0;
	isCountSfsem = 0;
	i = 0;
	while ((i=getword("%mor:", morline, spareTier2, NULL, i))) {
		POS = strchr(spareTier2, '#');
		if (POS != NULL)
			POS++;
		else
			POS = spareTier2;
		if (uS.mStrnicmp(POS, "v|", 2) == 0   || uS.mStrnicmp(POS, "cop|", 4) == 0 ||
			uS.mStrnicmp(POS, "mod|", 4) == 0 || uS.mStrnicmp(POS, "part|", 5) == 0) {
			isCountSntp = TRUE;
			isCountScss = TRUE;
			isCountSfsyn = 1;
			isCountSfsem = 1;
		}
	}
	i = 0;
	while ((i=getword("*:", line, spareTier2, NULL, i))) {
		if (spareTier2[0] == '[' && spareTier2[1] == '+') {
			removeExtraSpace(spareTier2);
			if (uS.mStricmp(spareTier2, "[+ gram]") == 0) {
				isCountScss = FALSE;
				if (isCountSfsyn == 1)
					isCountSfsyn = 2;
			} else if (uS.mStricmp(spareTier2, "[+ sem]") == 0) {
				isCountScss = FALSE;
				if (isCountSfsem == 1)
					isCountSfsem = 2;
			}
		} else if (uS.mStricmp(spareTier2, "+...") == 0 || uS.mStricmp(spareTier2, "+/.") == 0 ||
				   uS.mStricmp(spareTier2, "+//.") == 0 || uS.mStricmp(spareTier2, "+/?") == 0 ||
				   uS.mStricmp(spareTier2, "+//?") == 0) {
//			isCountSntp = FALSE;

		}
	}
	if (isCountSntp) {
		ts->sntp++;
		*tsntp = *tsntp + 1;
	}
	if (isCountScss)
		ts->scss++;
	if (isCountSfsyn == 2)
		ts->sfsyn++;
	if (isCountSfsem == 2)
		ts->sfsem++;
}

static char c_nnla_isUttDel(char *line, int pos) {
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

static char checkV(struct Vs *v, MORFEATS *feat) {
	while (v) {
		if (v->isV) {
			if (isEqual("v", feat->pos) && isEqual(v->word, feat->stem))
				return(TRUE);
		}
		if (v->isPart) {
			if (isEqual("part", feat->pos) && isEqual(v->word, feat->stem))
				return(TRUE);
		}
		v = v->nextV;
	}
	return(FALSE);
}
/*
static char nextItem(char *line, int i, const char *st1, const char *st2) {
 	int  i;
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
static char isFoundAnyWhereOnGRA(char *line, const char *tag) {
	int  i;
	char morWord[1024], graWord[1024], *vb;

	i = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			vb = strrchr(graWord, '|');
			if (vb != NULL) {
				vb++;
				if (uS.mStricmp(vb, tag) == 0) {
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

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

static char isCurLinkedtoTag(int *num, char *line, const char *tag) {
	int  i, cnt;
	char morWord[1024], graWord[1024], *vb, *pos;

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
					}
				}
				break;
			}
		}
	}
	return(FALSE);
}

static char isLinkedtoTag(char *graTarget, char *line, const char *tag1, const char *tag2, const char *tag3) {
	int num;
	char *vb;

	vb = strchr(graTarget, '|');
	if (vb != NULL) {
		vb++;
		num = atoi(vb);
	} else
		return(FALSE);
	if (isCurLinkedtoTag(&num, line, tag1)) {
		if (tag2 == NULL)
			return(TRUE);
		else {
			if (isCurLinkedtoTag(&num, line, tag2)) {
				if (tag3 == NULL)
					return(TRUE);
				else {
					if (isCurLinkedtoTag(&num, line, tag3)) {
						return(TRUE);
					}
				}
			}
		}
	}
	return(FALSE);
}


static void ifLinkedtoTagThenBlankIt(char *graTarget, char *line, const char *tag) {
	int  i, wi, cnt, num;
	char morWord[1024], graWord[1024], *vb;

	vb = strchr(graTarget, '|');
	if (vb != NULL) {
		vb++;
		num = atoi(vb);
	} else
		return;

	i = 0;
	cnt = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, &wi, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			cnt++;
			if (cnt == num) {
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (uS.mStricmp(vb, tag) == 0) {
						for (; uttline[wi] != sMarkChr && uttline[wi] != EOS; wi++)
							uttline[wi] = ' ';
					}
				}
				break;
			}
		}
	}
}

static void ifFollowedByTOandVERBThenBlankIt(char *line, int i) {
	int  wi;
	char morWord[1024], graWord[1024];

	if ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			if (strcmp(morWord, "inf|to") == 0) {
				if ((i=getNextDepTierPair(uttline, graWord, morWord, &wi, i)) != 0) {
					if (graWord[0] != EOS && morWord[0] != EOS) {
						if (strncmp(morWord, "cop|", 4) == 0 || strncmp(morWord, "v|", 2) == 0) {
							for (; uttline[wi] != sMarkChr && uttline[wi] != EOS; wi++)
								uttline[wi] = ' ';
						}
					}
				}
			}
		}
	}
}

static char isNumWords(char *line, int num) {
	int  i, cnt;
	char morWord[1024], graWord[1024];

	i = 0;
	cnt = 0;
	while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
		if (graWord[0] != EOS && morWord[0] != EOS) {
			cnt++;
		}
	}
	if (num == cnt - 1)
		return(TRUE);
	return(FALSE);
}

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

static void c_nnla_process_tier(struct c_nnla_speakers *ts, struct database *db, char *rightIDFound, char isOutputGem, FILE *PWordsListFP) {
	int i, j, wi;
	int tscrc, tscrs, taecs, tsntp, tgriv, tgiiv, tvall, tvone, tvtwo, tvthree;
	char word[1024], graWord[1024];
	char tmp, isWordsFound, sq, aq, isSkip, *vb;
	char isPSDFound, curPSDFound, isAmbigFound;
	char isPvg1Counted, isPvg2Counted, isPvg3Counted;
	long stime, etime;
	double tNum;
	float mluWords, mluUtt;
	struct IDparts IDTier;
	MORFEATS word_feats, *feat;

	if (utterance->speaker[0] == '*') {
		org_depTier[0] = EOS;
		strcpy(spareTier1, utterance->line);
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
		if (ts == NULL)
			return;
		i = 0;
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (templineC2[0] != '0' && templineC2[0] != '&' && templineC2[0] != '[' && templineC2[0] != '-' &&
				templineC2[0] != '+' && templineC2[0] != '!' && templineC2[0] != '?' && templineC2[0] != '.' &&
				templineC2[0] != HIDEN_C && templineC2[0] != '#') {
				ts->aspt++;
				break;
			}
		}
		if (i == 0)
			j = 0;
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (org_depTier[0] == EOS) {
			strcpy(org_depTier, utterance->line);
		}
		if (ts == NULL)
			return;

		tsntp = 0;
		tgriv = 0;
		tgiiv = 0;
		isCountUtt(spareTier1, uttline, ts, &tsntp); // lxs point 3., 4., 5., 6.
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
				if ((i=getword(utterance->speaker, uttline, templineC2, &j, i))) {
					if (uS.mStrnicmp(templineC2, "adv:tem|then", 12) == 0) {
						for (; uttline[j] != EOS && j < i; j++)
							uttline[j] = ' ';
					}
				}
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
		while ((i=getword(utterance->speaker, uttline, templineC2, NULL, i))) {
			uS.remblanks(templineC2);
			if (strchr(templineC2, '|') != NULL) {
				ts->totalWords++;
				for (j=0; templineC2[j] != EOS; j++) {
					if (templineC2[j] == '$' && !isPrecliticUse)
						ts->totalWords++;
					if (templineC2[j] == '~' && !isPostcliticUse)
						ts->totalWords++;
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
		mluWords	= ts->mluWords;
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
				isAmbigFound = FALSE;
				isWordsFound = TRUE;
				tmp = TRUE;
				mluWords = mluWords + 1;
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
			if (c_nnla_isUttDel(utterance->line, i)) {
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
		strcpy(spareTier3, uttline);
/*
		for (i=0; spareTier3[i] != EOS; i++) {
			if (spareTier3[i] == '$' || spareTier3[i] == '~')
				spareTier3[i] = ' ';
		}
*/
		tvall = 0;
		tvone = 0;
		tvtwo = 0;
		tvthree=0;
		i = 0;
		while ((i=getword(utterance->speaker, spareTier3, templineC2, NULL, i))) {
			if (!isWordFromMORTier(templineC2))
				continue;
			if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
				c_nnla_error(db, TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
				// counts open/closed BEG
				if (isEqual("adv:int", feat->pos) || isEqual("comp", feat->pos)) {
					ts->closedClass++;
				} else if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) || isnEqual("cop", feat->pos, 3) ||
						   isEqual("adj", feat->pos) || isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) {
					ts->openClass++;
				} else if (isEqual("co", feat->pos) == FALSE && isEqual("on", feat->pos) == FALSE) {
					ts->closedClass++;
				}
				// counts pen/closed END
				if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2)) {
					ts->noun++;
				} else if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) || isEqual("part", feat->pos)) {
					ts->verb++;
					ts->allv++;
					tvall++;
				} else if (isEqual("adj", feat->pos) || isnEqual("adj:", feat->pos, 4)) {
					ts->adj++;
				} else if (isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) {
					if (isEqual("adv:int", feat->pos) && isnEqual("wh", feat->stem, 2)) {
						j = 0;
					} else
						ts->adv++;
				} else if (isEqual("det:art", feat->pos) || isEqual("det:dem", feat->pos)) {
					ts->det++;
				} else if (isEqual("pro", feat->pos) || isnEqual("pro:", feat->pos, 4)) {
					if (isEqual("pro:rel", feat->pos) && isnEqual("wh", feat->stem, 2)) {
						j = 0;
					} else if (isEqual("pro:int", feat->pos) && isnEqual("wh", feat->stem, 2)) {
						j = 0;
					} else
						ts->pro++;
				} else if (isEqual("det:poss", feat->pos)) {
					ts->pro++;
				} else if (isEqual("aux", feat->pos)) {
					ts->aux++;
				} else if (isEqual("conj", feat->pos) || isnEqual("conj:", feat->pos, 5)) {
					if (isEqual("conj", feat->pos) && isnEqual("wh", feat->stem, 2)) {
						j = 0;
					} else
						ts->conj++;
				} else if (isEqual("coord", feat->pos)) {
					ts->conj++;
				} else if (isEqual("comp", feat->pos)) {
					ts->comp++;
				} else if (isEqual("mod", feat->pos) || isEqual("mod:aux", feat->pos)) {
					ts->mod++;
				} else if (isEqual("prep", feat->pos) || isnEqual("prep:", feat->pos, 5)) {
					ts->prep++;
				} else if (isEqual("neg", feat->pos)) {
					ts->neg++;
				} else if (isEqual("co", feat->pos) && isEqual("no", feat->stem)) {
					ts->neg++;
				} else if (isEqual("inf", feat->pos)) {
					ts->inf++;
				} else if (isEqual("qn", feat->pos) || isEqual("det:num", feat->pos) ||
						   isEqual("post", feat->pos)) {
					ts->qn++;
				}

				if (isEqual("det:int", feat->pos)) {
					ts->whwd++;
				} else if (isnEqual("wh", feat->stem, 2)) {
					if (isEqual("adv:int", feat->pos) || isEqual("conj", feat->pos) ||
						isEqual("pro:rel", feat->pos) || isEqual("pro:int", feat->pos)) {
						ts->whwd++;
					}
				}

				if (isEqualIxes("CP", feat->suffix, NUM_SUFF)) {
					ts->dCP++;
				} else if (isEqualIxes("SP", feat->suffix, NUM_SUFF)) {
					ts->dSP++;
				} else if (isEqualIxes("PL", feat->suffix, NUM_SUFF)) {
					if (isEqual("n", feat->pos) && (isIxesMatchPat(feat->error, NUM_ERRS, "m:0s*") ||
													isIxesMatchPat(feat->error, NUM_ERRS, "m:=s")))
						j = 0;
					else
						ts->dPL++;
				} else if (isEqualIxes("PL", feat->fusion, NUM_FUSI)) {
					if (isEqual("n", feat->pos) && (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:s*") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=s")))
						j = 0;
					else
						ts->aPL++;
				} else if (isEqualIxes("3S", feat->suffix, NUM_SUFF)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:03s*"))
						j = 0;
					else
						ts->d3S++;
				} else if (isEqualIxes("PAST", feat->suffix, NUM_SUFF)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed"))
						j = 0;
					else
						ts->dPAST++;
				} else if (isEqualIxes("PAST", feat->fusion, NUM_FUSI)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:ed"))
						j = 0;
					else
						ts->aPAST++;
				} else if (isEqualIxes("PASTP", feat->suffix, NUM_SUFF)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:=en"))
						j = 0;
					else
						ts->dPASTP++;
				} else if (isEqualIxes("PASTP", feat->fusion, NUM_FUSI)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=en"))
						j = 0;
					else
						ts->aPASTP++;
				} else if (isEqualIxes("PRESP", feat->suffix, NUM_SUFF)) {
					if (isEqual("n:gerund", feat->pos))
						j = 0;
					else if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing"))
						j = 0;
					else
						ts->dPRESP++;
				} else if (isEqualIxes("POSS", feat->suffix, NUM_SUFF)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0's") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0s'"))
						j = 0;
					else
						ts->poss++;
				}

				// regular inflected verbs - griv, ariv
				if ((isEqual("v", feat->pos) && isEqualIxes("PAST", feat->suffix, NUM_SUFF))     ||
					(isEqual("v", feat->pos) && isEqualIxes("3S", feat->suffix, NUM_SUFF))       ||
					(isEqual("cop", feat->pos) && isEqualIxes("PAST", feat->suffix, NUM_SUFF))   ||
					(isEqual("cop", feat->pos) && isEqualIxes("3S", feat->suffix, NUM_SUFF))     ||
					(isEqual("part", feat->pos) && isEqualIxes("PASTP", feat->suffix, NUM_SUFF)) ||
					(isEqual("part", feat->pos) && isEqualIxes("PRESP", feat->suffix, NUM_SUFF)) ||
					(isEqual("aux", feat->pos) && isEqualIxes("PRESP", feat->suffix, NUM_SUFF))  ||
					(isEqual("mod", feat->pos) && isEqual("do", feat->stem) && isEqualIxes("3S", feat->fusion, NUM_SUFF))
					) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:03s")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+ing")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+ed")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+en")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++ing") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++3s")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++en")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:03s:a") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s:a"))
						j = 0;
					else {
						ts->griv++;
						tgriv++;
					}
					ts->ariv++;
				} else if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) || isEqual("part", feat->pos)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:0ing")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:03s")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:0ed")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:+ing")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:+ed")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:+en")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++ing") ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++3s")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++en")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:03s:a") ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:+3s:a")) {
						ts->ariv++;
					}
				}

				// irregular inflected verbs - giiv, aiiv
				if ((isEqual("cop", feat->pos) && isEqualIxes("3S", feat->fusion, NUM_FUSI)) ||
					(isEqual("aux", feat->pos) && isEqualIxes("3S", feat->fusion, NUM_FUSI)) ||
					(isEqual("aux", feat->pos) && isEqual("be", feat->stem) && isEqualIxes("PRES", feat->fusion, NUM_SUFF))
					) {
				} else if ( (isEqual("v", feat->pos) && isEqualIxes("PAST", feat->fusion, NUM_FUSI)) ||
							(isEqual("v", feat->pos) && isEqualIxes("3S", feat->fusion, NUM_FUSI))   ||
							(isEqual("part", feat->pos) && isEqualIxes("PASTP", feat->fusion, NUM_FUSI)) ||
							(isEqual("cop", feat->pos) && isIxesMatchPat(feat->fusion, NUM_FUSI, "_*"))  ||
							(isEqual("aux", feat->pos) && isIxesMatchPat(feat->fusion, NUM_FUSI, "_*"))  ||
						    (isEqual("mod", feat->pos) && isEqual("do", feat->stem) && isEqualIxes("PAST", feat->fusion, NUM_SUFF))
						   ) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en") ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:irr:ed")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:irr:en")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:ed")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:en")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed")     ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:=en")     ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed:i")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:++en:i")  ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:vsg:a")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:vun:a")   ||
						isIxesMatchPat(feat->error, NUM_ERRS, "m:m"))
						j = 0;
					else {
						ts->giiv++;
						tgiiv++;
					}
					ts->aiiv++;
				} else if (isEqual("v", feat->pos) || isEqual("cop", feat->pos) || isEqual("part", feat->pos)) {
					if (isIxesMatchPat(feat->error, NUM_ERRS, "m:base:ed") ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:base:en") ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:irr:ed")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:irr:en")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:ed")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:sub:en")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:=ed")     ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:=en")     ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++ed:i")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:++en:i")  ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:vsg:a")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:vun:a")   ||
						   isIxesMatchPat(feat->error, NUM_ERRS, "m:m")) {
						ts->aiiv++;
					}
				}


				if (checkV(v1, feat)) {
					ts->pv1++;
					tvone++;
//					ts->allv++;
				}
				if (checkV(v2op, feat) || checkV(v2ob, feat)) {
					ts->pv2++;
					tvtwo++;
//					ts->allv++;
				}
				if (checkV(v3op, feat) || checkV(v3ob, feat)) {
					ts->pv3++;
					tvthree++;
//					ts->allv++;
				}

			}
			freeUpFeats(&word_feats);
		}
		if (isDebugMode == TRUE || isDebugMode == 'P' || isDebugMode == 'p') {
			fprintf(stderr, "sentences produced=%d:\n", tsntp);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier3);
			putc('\n',stderr);

		}
		if (isDebugMode == TRUE || isDebugMode == 'R' || isDebugMode == 'r') {
			fprintf(stderr, "correct regular verb inflection=%d:\n", tgriv);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier3);
			putc('\n',stderr);
		}
		if (isDebugMode == TRUE || isDebugMode == 'I' || isDebugMode == 'i') {
			fprintf(stderr, "correct irregular verb inflection=%d:\n", tgiiv);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier3);
			putc('\n',stderr);
		}
		if (isDebugMode == TRUE || isDebugMode == '1') {
			fprintf(stderr, "1-place verbs=%d; 2-place verbs=%d; 3-place verbs=%d; all verbs=%d:\n", tvone, tvtwo, tvthree, tvall);
		}
	} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
		if (ts == NULL)
			return;
		if (org_depTier[0] == EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"ERROR: %%mor tier is not found before %%gra tier.\n");
			c_nnla_error(db, FALSE);
		}
		for (j=0; org_depTier[j] != EOS; j++) {
			if (org_depTier[j] == '~' || org_depTier[j] == '$')
				org_depTier[j] = ' ';
		}
		isWordsFound = FALSE;
		i = 0;
		while ((i=getword("%mor:", org_depTier, templineC2, NULL, i))) {
			if (uS.mStrnicmp(templineC2, "v|", 2) == 0  || uS.mStrnicmp(templineC2, "cop|", 4) == 0 ||
				uS.mStrnicmp(templineC2, "mod|", 4) == 0 || uS.mStrnicmp(templineC2, "part|", 5) == 0) {
				isWordsFound = TRUE;
				break;
			}
		}
		createMorUttline(utterance->line,org_depTier,"%gra",utterance->tuttline,FALSE,TRUE);
		strcpy(utterance->tuttline, utterance->line);
		filterMorTier(uttline, utterance->line, 2, TRUE);

		taecs = 0;
		tscrc = 0;
		tscrs = 0;
		tvone = 0;
		tvtwo = 0;
		tvthree=0;
		if (isWordsFound) {
			i = 0;
			word[0] = EOS;
			while ((i=getNextDepTierPair(uttline, graWord, templineC2, NULL, i)) != 0) {
				if (graWord[0] != EOS && templineC2[0] != EOS) {
					isPvg1Counted = FALSE;
					isPvg2Counted = FALSE;
					isPvg3Counted = FALSE;
					if (!isWordFromMORTier(templineC2))
						continue;
					if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
						c_nnla_error(db, TRUE);
					for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
						if (checkV(v1, feat)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg1Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ")) {
										ts->pvg1++;
										tvone++;
										isPvg1Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg1Counted == FALSE) {
									if (isEqual("v", feat->pos) && isNumWords(uttline, 1)) {
										ts->pvg1++;
										tvone++;
										isPvg1Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COMP") == 0 && isPvg1Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
										ts->pvg1++;
										tvone++;
										isPvg1Counted = TRUE;
									}
								}
							}
						}
						if (checkV(v2op, feat)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg2Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ")) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg2Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL) &&
										isFoundAnyWhereOnGRA(uttline, "SUBJ")) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg2Counted == FALSE) {
									if (isEqual("v", feat->pos) && isNumWords(uttline, 1)) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg2Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL) &&
										isEqual("v", feat->pos) && isFirstGRA(uttline, "ROOT")) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COMP") == 0 && isPvg2Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg2Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "COMP", NULL)) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg2Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										isFoundAnyWhereOnGRA(uttline, "AUX") &&
										isFoundAnyWhereOnMOR(uttline, "aux|be*") &&
										(isFoundAnyWhereOnMOR(uttline, "*-PASTP*") ||
										 isFoundAnyWhereOnMOR(uttline, "*&PASTP*"))) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
							}
						}
						if (checkV(v2ob, feat)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg2Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										(isFoundAnyWhereOnGRA(uttline, "OBJ") ||
										 isFoundAnyWhereOnGRA(uttline, "POBJ"))) {
											ts->pvg2++;
											tvtwo++;
											isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg2Counted == FALSE) {
									if (isEqual("v", feat->pos) && isNumWords(uttline, 1)) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg2Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL) &&
										isEqual("v", feat->pos) && isFirstGRA(uttline, "ROOT")) {
										ts->pvg2++;
										tvtwo++;
										isPvg2Counted = TRUE;
									}
								}
							}
						}
						if (isEqual("v", feat->pos) && (isEqual("leave", feat->stem) ||
														isEqual("dress", feat->stem))) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ")) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
							}
						}
						if (checkV(v3op, feat)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										(isFoundAnyWhereOnGRA(uttline, "OBJ") ||
										 isFoundAnyWhereOnGRA(uttline, "POBJ"))) {
											ts->pvg3++;
											tvthree++;
											isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isEqual("v", feat->pos) && isNumWords(uttline, 1)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg3Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT,v|", NULL) &&
										isFirstGRA(uttline, "ROOT") && isEqual("v", feat->pos)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg3Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT", NULL) &&
										isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										isEqual("v", feat->pos)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										isFoundAnyWhereOnGRA(uttline, "AUX") &&
										isFoundAnyWhereOnMOR(uttline, "aux|be*") &&
										(isFoundAnyWhereOnMOR(uttline, "*-PASTP*") ||
										 isFoundAnyWhereOnMOR(uttline, "*&PASTP*"))) {
											ts->pvg3++;
											tvthree++;
											isPvg3Counted = TRUE;
										}
								}
								if (uS.mStricmp(vb, "COMP") == 0 && isPvg3Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "ROOT", NULL, NULL)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg3Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "COMP", NULL)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
							}
						}
						if (checkV(v3ob, feat)) {
							vb = strrchr(graWord, '|');
							if (vb != NULL) {
								vb++;
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isFoundAnyWhereOnGRA(uttline, "SUBJ") &&
										(isFoundAnyWhereOnGRA(uttline, "OBJ") ||
										 isFoundAnyWhereOnGRA(uttline, "POBJ"))) {
											ts->pvg3++;
											tvthree++;
											isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "ROOT") == 0 && isPvg3Counted == FALSE) {
									if (isEqual("v", feat->pos) && isNumWords(uttline, 1)) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}
								if (uS.mStricmp(vb, "COORD") == 0 && isPvg3Counted == FALSE) {
									if (isLinkedtoTag(graWord, uttline, "CONJ", "ROOT,v|", NULL) &&
										isFirstGRA(uttline, "ROOT")) {
										ts->pvg3++;
										tvthree++;
										isPvg3Counted = TRUE;
									}
								}

							}
						}
					}
					freeUpFeats(&word_feats);
					vb = strrchr(graWord, '|');
					if (vb != NULL) {
						vb++;
						strcpy(word, vb);
					}
				}
			}
			i = 0;
			while ((i=getNextDepTierPair(uttline, graWord, word, &wi, i)) != 0) {
				if (graWord[0] != EOS && word[0] != EOS && isWordFromMORTier(word)) {
					uS.remblanks(graWord);
					if (strchr(graWord, '|') != NULL) {
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							if (uS.mStricmp(vb, "LINK") == 0 &&
								(strcmp(word, "coord|and") == 0 || strcmp(word, "coord|or") == 0 || strcmp(word, "conj|but") == 0)) {
								ifLinkedtoTagThenBlankIt(graWord, uttline, "CJCT");
							}
							if (strncmp(word, "part|go-", 8) == 0) {
								ifFollowedByTOandVERBThenBlankIt(uttline, i);
							}
						}
					}
				}
			}
			isWordsFound = FALSE;
			i = 0;
			while ((i=getNextDepTierPair(uttline, graWord, word, NULL, i)) != 0) {
				if (graWord[0] != EOS && word[0] != EOS && isWordFromMORTier(word)) {
					uS.remblanks(graWord);
					if (strchr(graWord, '|') != NULL) {
						vb = strrchr(graWord, '|');
						if (vb != NULL) {
							vb++;
							if (uS.mStricmp(vb, "CSUBJ") == 0 || uS.mStricmp(vb, "COMP") == 0  || uS.mStricmp(vb, "CPRED") == 0 ||
									   uS.mStricmp(vb, "CPOBJ") == 0 || uS.mStricmp(vb, "COBJ") == 0  || uS.mStricmp(vb, "XJCT") == 0  ||
									   uS.mStricmp(vb, "CJCT") == 0  || uS.mStricmp(vb, "CMOD") == 0  || uS.mStricmp(vb, "XMOD") == 0) {
								isWordsFound = TRUE;
								taecs++;
							}
						}
					}
				}
			}
			if (taecs >= 20) {
				fprintf(stderr, "\nERROR: exceeded the memory limit for embedded clauses of %d\n", aecsMax);
				c_nnla_error(db, FALSE);
			} else {
				ts->aecsTable[taecs] = ts->aecsTable[taecs] + 1;
				if (taecs > 0)
					ts->espt++;
			}
			if (isWordsFound) {
				tscrc++;
				ts->scrc++;
			} else {
				tscrs++;
				ts->scrs++;
			}
		}
		if (isDebugMode == TRUE || isDebugMode == '1') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "1-place w-correct arg=%d, 2-place w-correct arg=%d, 3-place w-correct arg=%d:\n", tvone, tvtwo, tvthree);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}
		if (isDebugMode == TRUE || isDebugMode == 'S' || isDebugMode == 's') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "Sentence complexity - complex=%d; simple=%d:\n", tscrc, tscrs);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}
		if (isDebugMode == TRUE || isDebugMode == 'E' || isDebugMode == 'e') {
			strcpy(spareTier2, uttline);
			combineMainDepWords(spareTier2, TRUE);
			fprintf(stderr, "# embedded clauses=%d:\n", taecs);
			fprintf(stderr, "%s", org_spName);
			printoutline(stderr, spareTier1);
			printoutline(stderr, spareTier2);
			putc('\n',stderr);
		}
	} else if (uS.partcmp(utterance->speaker,"%slc:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"%xslc:",FALSE,FALSE)) {
		if (isDebugMode != FALSE) {
			fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
		if (db == NULL) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				if (isCreateDB) {
					uS.remblanks(utterance->line);
					filterAndOuputID(utterance->line);
				} else {
					uS.remblanks(utterance->line);
					c_nnla_FindSpeaker(oldfname, templineC, utterance->line, FALSE, db);
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
		return((c_nnla_group == FALSE && found) || (c_nnla_SpecWords == found));
	else 
		return((c_nnla_group == TRUE && c_nnla_SpecWords > found) || (found == 0));
}

static void OpenDBFile(FILE *fp, struct database *db, char *isDbSpFound) {
	int  found, t;
	char isfirst, isOutputGem, rightIDFound, isDataFound;
	char word[BUFSIZ+1], DBFname[BUFSIZ+1], IDText[BUFSIZ+1];
	float tt;
	FILE *tfpin;
	struct c_nnla_speakers *ts;
	struct IDparts IDTier;

	tfpin = fpin;
	fpin = fp;
	ts = NULL;
	isfirst = TRUE;
	if (isNOptionSet == FALSE) {
		strcpy(c_nnla_BBS, "@*&#");
		strcpy(c_nnla_CBS, "@*&#");
	}
	if (c_nnla_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	if (!fgets_cr(templineC, UTTLINELEN, fpin))
		return;
	if (templineC[0] != 'V' || !isdigit(templineC[1])) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		c_nnla_error(db, FALSE);
	}
	t = atoi(templineC+1);
	if (t != C_NNLA_DB_VERSION) {
		fprintf(stderr,"\nERROR: Selected database is incompatible with this version of CLAN.\n");
		fprintf(stderr,"Please download a new CLAN from talkbank.org/clan server and re-install it.\n\n");
		c_nnla_error(db, FALSE);
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
					if (c_nnla_SpecWords)
						fprintf(stderr,"\nERROR: No specified gems found in database \"%s\"\n\n", FileName1);
					else
						fprintf(stderr,"\nERROR: No speaker matching +d option found in database \"%s\"\n\n", FileName1);
					c_nnla_error(db, FALSE);
				}
#ifdef DEBUGC_NNLARESULTS
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

				if (ts->mluUtt == 0.0)
					ts->mluWords  = 0.0;
				else
					ts->mluWords = ts->mluWords / ts->mluUtt;
				db->mluWords_sqr = db->mluWords_sqr + (ts->mluWords * ts->mluWords);
				db->mluWords = db->mluWords + ts->mluWords;

				db->mluUtt_sqr = db->mluUtt_sqr + (ts->mluUtt * ts->mluUtt);
				db->mluUtt = db->mluUtt + ts->mluUtt;

				db->openClass_sqr = db->openClass_sqr + (ts->openClass * ts->openClass);
				db->openClass = db->openClass + ts->openClass;

				if (ts->totalWords == 0.0)
					ts->openClassP = 0.0000;
				else
					ts->openClassP = (ts->openClass/ts->totalWords)*100.0000;
				db->openClassP_sqr = db->openClassP_sqr + (ts->openClassP * ts->openClassP);
				db->openClassP = db->openClassP + ts->openClassP;

				db->closedClass_sqr = db->closedClass_sqr + (ts->closedClass * ts->closedClass);
				db->closedClass = db->closedClass + ts->closedClass;

				if (ts->totalWords == 0.0)
					ts->closedClassP = 0.0000;
				else
					ts->closedClassP = (ts->closedClass/ts->totalWords)*100.0000;
				db->closedClassP_sqr = db->closedClassP_sqr + (ts->closedClassP * ts->closedClassP);
				db->closedClassP = db->closedClassP + ts->closedClassP;

				if (ts->closedClass == 0.0)
					tt = 0.0000;
				else
					tt = ts->openClass / ts->closedClass;
				db->OPoCl_sqr = db->OPoCl_sqr + (tt * tt);
				db->OPoCl = db->OPoCl + tt;

				db->noun_sqr = db->noun_sqr + (ts->noun * ts->noun);
				db->noun = db->noun + ts->noun;

				if (ts->totalWords == 0.0)
					ts->nounP = 0.0000;
				else
					ts->nounP = (ts->noun/ts->totalWords)*100.0000;
				db->nounP_sqr = db->nounP_sqr + (ts->nounP * ts->nounP);
				db->nounP = db->nounP + ts->nounP;

				db->verb_sqr = db->verb_sqr + (ts->verb * ts->verb);
				db->verb = db->verb + ts->verb;

				if (ts->totalWords == 0.0)
					ts->verbP = 0.0000;
				else
					ts->verbP = (ts->verb/ts->totalWords)*100.0000;
				db->verbP_sqr = db->verbP_sqr + (ts->verbP * ts->verbP);
				db->verbP = db->verbP + ts->verbP;

				if (ts->verb == 0.0)
					tt = 0.0000;
				else
					tt = ts->noun / ts->verb;
				db->NoV_sqr = db->NoV_sqr + (tt * tt);
				db->NoV = db->NoV + tt;

				db->adj_sqr = db->adj_sqr + (ts->adj * ts->adj);
				db->adj = db->adj + ts->adj;

				db->adv_sqr = db->adv_sqr + (ts->adv * ts->adv);
				db->adv = db->adv + ts->adv;

				db->det_sqr = db->det_sqr + (ts->det * ts->det);
				db->det = db->det + ts->det;

				db->pro_sqr = db->pro_sqr + (ts->pro * ts->pro);
				db->pro = db->pro + ts->pro;

				db->aux_sqr = db->aux_sqr + (ts->aux * ts->aux);
				db->aux = db->aux + ts->aux;

				db->conj_sqr = db->conj_sqr + (ts->conj * ts->conj);
				db->conj = db->conj + ts->conj;

				db->comp_sqr = db->comp_sqr + (ts->comp * ts->comp);
				db->comp = db->comp + ts->comp;

				db->mod_sqr = db->mod_sqr + (ts->mod * ts->mod);
				db->mod = db->mod + ts->mod;

				db->prep_sqr = db->prep_sqr + (ts->prep * ts->prep);
				db->prep = db->prep + ts->prep;

				db->neg_sqr = db->neg_sqr + (ts->neg * ts->neg);
				db->neg = db->neg + ts->neg;

				db->inf_sqr = db->inf_sqr + (ts->inf * ts->inf);
				db->inf = db->inf + ts->inf;

				db->qn_sqr = db->qn_sqr + (ts->qn * ts->qn);
				db->qn = db->qn + ts->qn;

				db->whwd_sqr = db->whwd_sqr + (ts->whwd * ts->whwd);
				db->whwd = db->whwd + ts->whwd;
/*
 				db->part_sqr = db->part_sqr + (ts->part * ts->part);
				db->part = db->part + ts->part;
*/
				db->dCP_sqr = db->dCP_sqr + (ts->dCP * ts->dCP);
				db->dCP = db->dCP + ts->dCP;

				db->dSP_sqr = db->dSP_sqr + (ts->dSP * ts->dSP);
				db->dSP = db->dSP + ts->dSP;

				db->poss_sqr = db->poss_sqr + (ts->poss * ts->poss);
				db->poss = db->poss + ts->poss;

				db->dPL_sqr = db->dPL_sqr + (ts->dPL * ts->dPL);
				db->dPL = db->dPL + ts->dPL;

				db->aPL_sqr = db->aPL_sqr + (ts->aPL * ts->aPL);
				db->aPL = db->aPL + ts->aPL;

				db->d3S_sqr = db->d3S_sqr + (ts->d3S * ts->d3S);
				db->d3S = db->d3S + ts->d3S;

				db->dPAST_sqr = db->dPAST_sqr + (ts->dPAST * ts->dPAST);
				db->dPAST = db->dPAST + ts->dPAST;

				db->aPAST_sqr = db->aPAST_sqr + (ts->aPAST * ts->aPAST);
				db->aPAST = db->aPAST + ts->aPAST;

				db->dPASTP_sqr = db->dPASTP_sqr + (ts->dPASTP * ts->dPASTP);
				db->dPASTP = db->dPASTP + ts->dPASTP;

				db->aPASTP_sqr = db->aPASTP_sqr + (ts->aPASTP * ts->aPASTP);
				db->aPASTP = db->aPASTP + ts->aPASTP;

				db->dPRESP_sqr = db->dPRESP_sqr + (ts->dPRESP * ts->dPRESP);
				db->dPRESP = db->dPRESP + ts->dPRESP;

				if (ts->ariv == 0.0)
					tt = 0.0000;
				else
					tt = (ts->griv / ts->ariv)*100.0000;
				db->RIV_sqr = db->RIV_sqr + (tt * tt);
				db->RIV = db->RIV + tt;

				if (ts->aiiv == 0.0)
					tt = 0.0000;
				else
					tt = (ts->giiv / ts->aiiv)*100.0000;
				db->IIV_sqr = db->IIV_sqr + (tt * tt);
				db->IIV = db->IIV + tt;

				if (ts->aspt == 0.0)
					tt = 0.0000;
				else
					tt = (ts->sntp / ts->aspt)*100.0000;
				db->SNTP_sqr = db->SNTP_sqr + (tt * tt);
				db->SNTP = db->SNTP + tt;

				if (ts->sntp == 0.0)
					tt = 0.0000;
				else
					tt = (ts->scss / ts->sntp)*100.0000;
				db->SCSS_sqr = db->SCSS_sqr + (tt * tt);
				db->SCSS = db->SCSS + tt;

				if (ts->sntp == 0.0)
					tt = 0.0000;
				else
					tt = (ts->sfsyn / ts->sntp)*100.0000;
				db->SFSYN_sqr = db->SFSYN_sqr + (tt * tt);
				db->SFSYN = db->SFSYN + tt;

				if (ts->sntp == 0.0)
					tt = 0.0000;
				else
					tt = (ts->sfsem / ts->sntp)*100.0000;
				db->SFSEM_sqr = db->SFSEM_sqr + (tt * tt);
				db->SFSEM = db->SFSEM + tt;

				if (ts->scrs == 0.0)
					tt = 0.0000;
				else
					tt = ts->scrc / ts->scrs;
				db->SCRCS_sqr = db->SCRCS_sqr + (tt * tt);
				db->SCRCS = db->SCRCS + tt;

				if (ts->espt == 0.0)
					tt = 0.0000;
				else
					tt = ts->aecs / ts->espt;
				db->AECS_sqr = db->AECS_sqr + (tt * tt);
				db->AECS = db->AECS + tt;

				if (ts->allv == 0.0)
					tt = 0.0000;
				else
					tt = ts->pv1 / ts->allv;
				db->PV1_sqr = db->PV1_sqr + (tt * tt);
				db->PV1 = db->PV1 + tt;

				if (ts->allv == 0.0)
					tt = 0.0000;
				else
					tt = ts->pv2 / ts->allv;
				db->PV2_sqr = db->PV2_sqr + (tt * tt);
				db->PV2 = db->PV2 + tt;

				if (ts->allv == 0.0)
					tt = 0.0000;
				else
					tt = ts->pv3 / ts->allv;
				db->PV3_sqr = db->PV3_sqr + (tt * tt);
				db->PV3 = db->PV3 + tt;


				if (ts->pv1 == 0.0)
					tt = 0.0000;
				else
					tt = ts->pvg1 / ts->pv1;
				db->pvg1_sqr = db->pvg1_sqr + (tt * tt);
				db->pvg1 = db->pvg1 + tt;

				if (ts->pv2 == 0.0)
					tt = 0.0000;
				else
					tt = ts->pvg2 / ts->pv2;
				db->pvg2_sqr = db->pvg2_sqr + (tt * tt);
				db->pvg2 = db->pvg2 + tt;

				if (ts->pv3 == 0.0)
					tt = 0.0000;
				else
					tt = ts->pvg3 / ts->pv3;
				db->pvg3_sqr = db->pvg3_sqr + (tt * tt);
				db->pvg3 = db->pvg3 + tt;

				db->db_sp = freespeakers(db->db_sp);
			}
			ts = NULL;
			isfirst = TRUE;
			if (isNOptionSet == FALSE) {
				strcpy(c_nnla_BBS, "@*&#");
				strcpy(c_nnla_CBS, "@*&#");
			}
			if (c_nnla_SpecWords) {
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
			if (c_nnla_SpecWords && !strcmp(c_nnla_BBS, "@*&#") && rightIDFound) {
				if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
					c_nnla_n_option = FALSE;
					strcpy(c_nnla_BBS, "@BG:");
					strcpy(c_nnla_CBS, "@EG:");
				} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
					c_nnla_n_option = TRUE;
					strcpy(c_nnla_BBS, "@G:");
					strcpy(c_nnla_CBS, "@*&#");
				}
			}
			if (uS.partcmp(utterance->speaker,c_nnla_BBS,FALSE,FALSE)) {
				if (c_nnla_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,c_nnla_CBS,FALSE,FALSE)) {
				if (c_nnla_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (c_nnla_SpecWords)
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
					ts = c_nnla_FindSpeaker(FileName1, templineC, NULL, TRUE, db);
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
					c_nnla_process_tier(ts, db, NULL, isOutputGem, NULL);
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
		addFilename2Path(FileName1, "c-nnla");
		addFilename2Path(FileName1, lang_prefix);
		strcat(FileName1, DATABASE_FILE_NAME);
		fp = fopen(FileName1, "r");
	}
	if (fp == NULL) {
		fprintf(stderr, "Can't find database in folder \"%s\"\n", FileName1);
		fprintf(stderr, "If you do not use database, then do not include +d option in command line\n");
		c_nnla_error(db, FALSE);
	} else {
		fprintf(stderr,"Database File used: %s\n", FileName1);
		OpenDBFile(fp, db, &isDbSpFound);
		fprintf(stderr,"\r%.0f  \n", db->num);
	}
	if (!isDbSpFound || db->num == 0.0) {
		if (c_nnla_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n");
			fprintf(stderr,"OR No specified gems found for selected speakers in the database\n\n");
		} else
			fprintf(stderr,"\nERROR: No speaker matching +d option found in the database\n\n");
		c_nnla_error(db, FALSE);
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

static void c_nnla_pr_result(void) {
	int    i, SDn = 0;
	char   st[1024], *sFName;
	float  num, tt;
	struct c_nnla_speakers *ts;
	struct SDs SD[SDRESSIZE];
	struct DBKeys *DBKey;

	if (isCreateDB) {
		sp_head = freespeakers(sp_head);
		return;
	}
	if (sp_head == NULL) {
		if (c_nnla_SpecWords && !onlyApplyToDB) {
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
	excelStrCell(fpout, "Duration (sec)");
	excelStrCell(fpout, "Words/Min");
	excelStrCell(fpout, "Total Utts");
	excelStrCell(fpout, "Total Words");
	excelStrCell(fpout, "MLU Words");
	excelStrCell(fpout, "open-class");
	excelStrCell(fpout, "% open-class/all words");
	excelStrCell(fpout, "closed-class");
	excelStrCell(fpout, "% closed-class/all words");
	excelStrCell(fpout, "open/closed");
	excelStrCell(fpout, "Nouns"); //   +|n,|n:*
	excelStrCell(fpout, "% Nouns/all words");
	excelStrCell(fpout, "Verbs"); //   +|v,|cop
	excelStrCell(fpout, "% Verbs/all words");
	excelStrCell(fpout, "noun/verb");
	excelStrCell(fpout, "adj|"); //  +|adj,|adj:*
	excelStrCell(fpout, "adv|"); //  +|adv,|adv:*  -|adv:int,;wh*
	excelStrCell(fpout, "det|"); //  +|det:art,|det:dem
	excelStrCell(fpout, "pro|"); //  +|pro,|pro:*  -|pro:rel,;wh* -|pro:int,;wh*
	excelStrCell(fpout, "aux|"); //  +|aux
	excelStrCell(fpout, "conj|");//  +|conj,|conj:*,|coord  -|conj,;wh*
	excelStrCell(fpout, "complementizers");//  +|comp
	excelStrCell(fpout, "modals");// +|mod,|mod:aux
	excelStrCell(fpout, "prep|"); //  +|prep,|prep:*
	excelStrCell(fpout, "negation markers"); // +|neg
	excelStrCell(fpout, "infinitival markers"); // +|inf
	excelStrCell(fpout, "quantifiers"); //|qn
	excelStrCell(fpout, "wh-words"); // +|adv:int,;wh* +|pro:rel,;wh* +|pro:int,;wh* +|conj,;wh* +|det:int
//	excelStrCell(fpout, "particles"); // do not do
	excelStrCell(fpout, "comparative suffixes"); // -CP
	excelStrCell(fpout, "superlative suffixes"); // -SP
	excelStrCell(fpout, "possessive markers"); // ~poss|s
	excelStrCell(fpout, "regular plural markers"); // -PL
	excelStrCell(fpout, "irregular plural forms"); // &PL
	excelStrCell(fpout, "3rd person present tense markers"); // -3S d3S
	excelStrCell(fpout, "regular past tense markers"); // -PAST dPAST
	excelStrCell(fpout, "irregular past tense markers"); // &PAST aPAST
	excelStrCell(fpout, "regular perfect aspect markers"); // -PASTP dPASTP
	excelStrCell(fpout, "irregular perfect participles"); // &PASTP aPASTP
	excelStrCell(fpout, "progressive aspect markers"); // -PRESP dPRESP
	excelStrCell(fpout, "% correct regular verb inflection"); // point 1. griv, ariv, RIV_sqr, RIV
	excelStrCell(fpout, "% correct irregular verb inflection"); // point 2. giiv, aiiv, IIV_sqr, IIV
	excelStrCell(fpout, "% sentences produced"); // point 3. sntp, aspt, SNTP_sqr, SNTP
	excelStrCell(fpout, "% sentences with correct syntax, semantics*"); // point 4. scss, sntp, SCSS_sqr, SCSS
	excelStrCell(fpout, "% sentences with flawed syntax"); // point 5. sfsyn, sntp, SFSYN_sqr, SFSYN
	excelStrCell(fpout, "% sentences with flawed semantics*"); // point 6. sfsem, sntp, SFSEM_sqr, SFSEM
	excelStrCell(fpout, "sentence complexity ratio"); // point 7. scrc, scrs, SCRCS_sqr, SCRCS
	excelStrCell(fpout, "# embedded clauses/sentence"); // point 8. aecs, espt, AECS_sqr, AECS
/* 2019-11-20
	excelStrCell(fpout, "1-place verbs/all verbs"); // 1-place verbs/all_verbs - pv1, allv, PV1_sqr, PV1
	excelStrCell(fpout, "2-place verbs/all verbs"); // 2-place verbs/all_verbs - pv2, allv, PV2_sqr, PV2
	excelStrCell(fpout, "3-place verbs/all verbs"); // 3-place verbs/all_verbs - pv3, allv, PV3_sqr, PV3
	excelStrCell(fpout, "1-place verbs with correct argument structure"); // 1-place verbs with correct argument structure
	excelStrCell(fpout, "2-place verbs with correct argument structure");
	excelStrCell(fpout, "3-place verbs with correct argument structure");
*/
	excelRow(fpout, ExcelRowEnd);

	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (c_nnla_SpecWords) {
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
		if (ts->tm == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->totalWords / (ts->tm / 60.0000)); // Words/Min
		excelNumCell(fpout, "%.0f", ts->tUtt); // Total Utts
		excelNumCell(fpout, "%.0f", ts->totalWords); //Total Words
		if (ts->mluUtt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->mluWords/ts->mluUtt); // MLU Words

		excelNumCell(fpout, "%.0f", ts->openClass); // open-class
		if (ts->totalWords == 0.0) {
			ts->openClassP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->openClassP = (ts->openClass/ts->totalWords)*100.0000;
			excelNumCell(fpout, "%.3f", ts->openClassP); // % open-class/all words
		}

		excelNumCell(fpout, "%.0f", ts->closedClass); // closed-class
		if (ts->totalWords == 0.0) {
			ts->closedClassP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->closedClassP = (ts->closedClass/ts->totalWords)*100.0000;
			excelNumCell(fpout, "%.3f", ts->closedClassP); // % closed-class/all words
		}

		if (ts->closedClass == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->openClass/ts->closedClass); // open/closed

		excelNumCell(fpout, "%.0f", ts->noun);
		if (ts->totalWords == 0.0) {
			ts->nounP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->nounP = (ts->noun/ts->totalWords)*100.0000;
			excelNumCell(fpout, "%.3f", ts->nounP);
		}

		excelNumCell(fpout, "%.0f", ts->verb);
		if (ts->totalWords == 0.0) {
			ts->verbP = 0.0000;
			excelStrCell(fpout, "NA");
		} else {
			ts->verbP = (ts->verb/ts->totalWords)*100.0000;
			excelNumCell(fpout, "%.3f", ts->verbP);
		}

		if (ts->verb == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->noun/ts->verb);

		excelNumCell(fpout, "%.0f", ts->adj);
		excelNumCell(fpout, "%.0f", ts->adv);
		excelNumCell(fpout, "%.0f", ts->det);
		excelNumCell(fpout, "%.0f", ts->pro);
		excelNumCell(fpout, "%.0f", ts->aux);
		excelNumCell(fpout, "%.0f", ts->conj);
		excelNumCell(fpout, "%.0f", ts->comp);
		excelNumCell(fpout, "%.0f", ts->mod);
		excelNumCell(fpout, "%.0f", ts->prep);
		excelNumCell(fpout, "%.0f", ts->neg);
		excelNumCell(fpout, "%.0f", ts->inf);
		excelNumCell(fpout, "%.0f", ts->qn);
		excelNumCell(fpout, "%.0f", ts->whwd);
//		excelStrCell(fpout, "do_not_do"); // ts->part
		excelNumCell(fpout, "%.0f", ts->dCP);
		excelNumCell(fpout, "%.0f", ts->dSP);
		excelNumCell(fpout, "%.0f", ts->poss);
		excelNumCell(fpout, "%.0f", ts->dPL);
		excelNumCell(fpout, "%.0f", ts->aPL);
		excelNumCell(fpout, "%.0f", ts->d3S);
		excelNumCell(fpout, "%.0f", ts->dPAST);
		excelNumCell(fpout, "%.0f", ts->aPAST);
		excelNumCell(fpout, "%.0f", ts->dPASTP);
		excelNumCell(fpout, "%.0f", ts->aPASTP);
		excelNumCell(fpout, "%.0f", ts->dPRESP);

		if (ts->ariv == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->griv / ts->ariv)*100.0000);

		if (ts->giiv == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->giiv / ts->aiiv)*100.0000);

		if (ts->aspt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->sntp / ts->aspt)*100.0000);

		if (ts->sntp == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->scss / ts->sntp)*100.0000);

		if (ts->sntp == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->sfsyn / ts->sntp)*100.0000);

		if (ts->sntp == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->sfsem / ts->sntp)*100.0000);

		if (ts->scrs == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->scrc / ts->scrs);

		ts->aecs = 0.0;
		ts->espt = 0.0;
		ts->espt = ts->espt + (float)ts->aecsTable[0];
		if (isDebugMode == TRUE || isDebugMode == 'E' || isDebugMode == 'e') {
			fprintf(stderr, "\n# embedded clauses=%d; #sentences=%d:\n", 0, ts->aecsTable[0]);
		}
		for (i=1; i < aecsMax; i++) {
			ts->aecs = ts->aecs + (float)(i * ts->aecsTable[i]);
			ts->espt = ts->espt + (float)ts->aecsTable[i];
			if (isDebugMode == TRUE || isDebugMode == 'E' || isDebugMode == 'e') {
				if (ts->aecsTable[i] > 0)
					fprintf(stderr, "# embedded clauses=%d; #sentences=%d:\n", i, ts->aecsTable[i]);
			}
		}
		if (ts->espt == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", ts->aecs / ts->espt);

/* 2019-11-20
		if (ts->allv == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pv1 / ts->allv)*100.0000);

		if (ts->allv == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pv2 / ts->allv)*100.0000);

		if (ts->allv == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pv3 / ts->allv)*100.0000);

		if (ts->pv1 == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pvg1 / ts->pv1)*100.0000);

		if (ts->pv2 == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pvg2 / ts->pv2)*100.0000);

		if (ts->pv3 == 0.0)
			excelStrCell(fpout, "NA");
		else
			excelNumCell(fpout, "%.3f", (ts->pvg3 / ts->pv3)*100.0000);
*/
		excelRow(fpout, ExcelRowEnd);

		if (DBKeyRoot != NULL) {
			num = db->num;
			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout,"+/-SD, , , , , , , , , , ");

			SDn = 0; // this number should be less than SDRESSIZE = 256
			compute_SD(&SD[SDn++], ts->tm,  NULL, db->tm_sqr, db->tm, num, st);
			tt = ts->tm / 60.0000;
			compute_SD(&SD[SDn++], ts->totalWords,  &tt, db->WOD_sqr, db->WOD, num, NULL);
			compute_SD(&SD[SDn++], ts->tUtt,  NULL, db->tUtt_sqr, db->tUtt, num, NULL);
			compute_SD(&SD[SDn++], ts->totalWords,  NULL, db->totalWords_sqr, db->totalWords, num, NULL);
			compute_SD(&SD[SDn++], ts->mluWords,  &ts->mluUtt, db->mluWords_sqr, db->mluWords, num, NULL);
			compute_SD(&SD[SDn++], ts->openClass,  NULL, db->openClass_sqr, db->openClass, num, NULL);
			compute_SD(&SD[SDn++], ts->openClassP,  NULL, db->openClassP_sqr, db->openClassP, num, NULL);
			compute_SD(&SD[SDn++], ts->closedClass,  NULL, db->closedClass_sqr, db->closedClass, num, NULL);
			compute_SD(&SD[SDn++], ts->closedClassP,  NULL, db->closedClassP_sqr, db->closedClassP, num, NULL);
			compute_SD(&SD[SDn++], ts->openClass,  &ts->closedClass, db->OPoCl_sqr, db->OPoCl, num, NULL);
			compute_SD(&SD[SDn++], ts->noun,  NULL, db->noun_sqr, db->noun, num, NULL);
			compute_SD(&SD[SDn++], ts->nounP,  NULL, db->nounP_sqr, db->nounP, num, NULL);
			compute_SD(&SD[SDn++], ts->verb,  NULL, db->verb_sqr, db->verb, num, NULL);
			compute_SD(&SD[SDn++], ts->verbP,  NULL, db->verbP_sqr, db->verbP, num, NULL);
			compute_SD(&SD[SDn++], ts->noun,  &ts->verb, db->NoV_sqr, db->NoV, num, NULL);
			compute_SD(&SD[SDn++], ts->adj,  NULL, db->adj_sqr, db->adj, num, NULL);
			compute_SD(&SD[SDn++], ts->adv,  NULL, db->adv_sqr, db->adv, num, NULL);
			compute_SD(&SD[SDn++], ts->det,  NULL, db->det_sqr, db->det, num, NULL);
			compute_SD(&SD[SDn++], ts->pro,  NULL, db->pro_sqr, db->pro, num, NULL);
			compute_SD(&SD[SDn++], ts->aux,  NULL, db->aux_sqr, db->aux, num, NULL);
			compute_SD(&SD[SDn++], ts->conj,  NULL, db->conj_sqr, db->conj, num, NULL);
			compute_SD(&SD[SDn++], ts->comp,  NULL, db->comp_sqr, db->comp, num, NULL);
			compute_SD(&SD[SDn++], ts->mod,  NULL, db->mod_sqr, db->mod, num, NULL);
			compute_SD(&SD[SDn++], ts->prep,  NULL, db->prep_sqr, db->prep, num, NULL);
			compute_SD(&SD[SDn++], ts->neg,  NULL, db->neg_sqr, db->neg, num, NULL);
			compute_SD(&SD[SDn++], ts->inf,  NULL, db->inf_sqr, db->inf, num, NULL);
			compute_SD(&SD[SDn++], ts->qn,  NULL, db->qn_sqr, db->qn, num, NULL);
			compute_SD(&SD[SDn++], ts->whwd,  NULL, db->whwd_sqr, db->whwd, num, NULL);
//			compute_SD(&SD[SDn++], ts->part,  NULL, db->part_sqr, db->part, num, NULL);
			compute_SD(&SD[SDn++], ts->dCP,  NULL, db->dCP_sqr, db->dCP, num, NULL);
			compute_SD(&SD[SDn++], ts->dSP,  NULL, db->dSP_sqr, db->dSP, num, NULL);
			compute_SD(&SD[SDn++], ts->poss,  NULL, db->dSP_sqr, db->dSP, num, NULL);
			compute_SD(&SD[SDn++], ts->dPL,  NULL, db->dPL_sqr, db->dPL, num, NULL);
			compute_SD(&SD[SDn++], ts->aPL,  NULL, db->aPL_sqr, db->aPL, num, NULL);
			compute_SD(&SD[SDn++], ts->d3S,  NULL, db->d3S_sqr, db->d3S, num, NULL);
			compute_SD(&SD[SDn++], ts->dPAST,  NULL, db->dPAST_sqr, db->dPAST, num, NULL);
			compute_SD(&SD[SDn++], ts->aPAST,  NULL, db->aPAST_sqr, db->aPAST, num, NULL);
			compute_SD(&SD[SDn++], ts->dPASTP,  NULL, db->dPASTP_sqr, db->dPASTP, num, NULL);
			compute_SD(&SD[SDn++], ts->aPASTP,  NULL, db->aPASTP_sqr, db->aPASTP, num, NULL);
			compute_SD(&SD[SDn++], ts->dPRESP,  NULL, db->dPRESP_sqr, db->dPRESP, num, NULL);
			tt = ts->ariv / 100.0000;
			compute_SD(&SD[SDn++], ts->griv,  &tt, db->RIV_sqr, db->RIV, num, NULL);
			tt = ts->aiiv / 100.0000;
			compute_SD(&SD[SDn++], ts->giiv,  &tt, db->IIV_sqr, db->IIV, num, NULL);
			tt = ts->aspt / 100.0000;
			compute_SD(&SD[SDn++], ts->sntp,  &tt, db->SNTP_sqr, db->SNTP, num, NULL);
			tt = ts->sntp / 100.0000;
			compute_SD(&SD[SDn++], ts->scss,  &tt, db->SCSS_sqr, db->SCSS, num, NULL);
			compute_SD(&SD[SDn++], ts->sfsyn, &tt, db->SFSYN_sqr, db->SFSYN, num, NULL);
			compute_SD(&SD[SDn++], ts->sfsem, &tt, db->SFSEM_sqr, db->SFSEM, num, NULL);
			compute_SD(&SD[SDn++], ts->scrc, &ts->scrs, db->SCRCS_sqr, db->SCRCS, num, NULL);
			compute_SD(&SD[SDn++], ts->aecs, &ts->espt, db->AECS_sqr, db->AECS, num, NULL);
			tt = ts->allv / 100.0000;
			compute_SD(&SD[SDn++], ts->pv1, &tt, db->PV1_sqr, db->PV1, num, NULL);
			compute_SD(&SD[SDn++], ts->pv2, &tt, db->PV2_sqr, db->PV2, num, NULL);
			compute_SD(&SD[SDn++], ts->pv3, &tt, db->PV3_sqr, db->PV3, num, NULL);
			tt = ts->pv1 / 100.0000;
			compute_SD(&SD[SDn++], ts->pvg1,  &tt, db->pvg1_sqr, db->pvg1, num, NULL);
			tt = ts->pv2 / 100.0000;
			compute_SD(&SD[SDn++], ts->pvg2,  &tt, db->pvg2_sqr, db->pvg2, num, NULL);
			tt = ts->pv3 / 100.0000;
			compute_SD(&SD[SDn++], ts->pvg3,  &tt, db->pvg3_sqr, db->pvg3, num, NULL);
			excelRow(fpout, ExcelRowEnd);

			excelRow(fpout, ExcelRowStart);
			excelCommasStrCell(fpout, " , , , , , , , , , , ");
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
		excelCommasStrCell(fpout,"Mean Database, , , , , , , , , , ");
		Secs2Str(db->tm/num, st, FALSE);
		excelStrCell(fpout, st);
		excelNumCell(fpout, "%.3f", db->WOD/num);
		excelNumCell(fpout, "%.3f", db->tUtt/num);
		excelNumCell(fpout, "%.3f", db->totalWords/num);
		excelNumCell(fpout, "%.3f", db->mluWords/num);
		excelNumCell(fpout, "%.3f", db->openClass/num);
		excelNumCell(fpout, "%.3f", db->openClassP/num);
		excelNumCell(fpout, "%.3f", db->closedClass/num);
		excelNumCell(fpout, "%.3f", db->closedClassP/num);
		excelNumCell(fpout, "%.3f", db->OPoCl/num);
		excelNumCell(fpout, "%.3f", db->noun/num);
		excelNumCell(fpout, "%.3f", db->nounP/num);
		excelNumCell(fpout, "%.3f", db->verb/num);
		excelNumCell(fpout, "%.3f", db->verbP/num);
		excelNumCell(fpout, "%.3f", db->NoV/num);
		excelNumCell(fpout, "%.3f", db->adj/num);
		excelNumCell(fpout, "%.3f", db->adv/num);
		excelNumCell(fpout, "%.3f", db->det/num);
		excelNumCell(fpout, "%.3f", db->pro/num);
		excelNumCell(fpout, "%.3f", db->aux/num);
		excelNumCell(fpout, "%.3f", db->conj/num);
		excelNumCell(fpout, "%.3f", db->comp/num);
		excelNumCell(fpout, "%.3f", db->mod/num);
		excelNumCell(fpout, "%.3f", db->prep/num);
		excelNumCell(fpout, "%.3f", db->neg/num);
		excelNumCell(fpout, "%.3f", db->inf/num);
		excelNumCell(fpout, "%.3f", db->qn/num);
		excelNumCell(fpout, "%.3f", db->whwd/num);
//		excelNumCell(fpout, "%.3f", db->part/num);
		excelNumCell(fpout, "%.3f", db->dCP/num);
		excelNumCell(fpout, "%.3f", db->dSP/num);
		excelNumCell(fpout, "%.3f", db->poss/num);
		excelNumCell(fpout, "%.3f", db->dPL/num);
		excelNumCell(fpout, "%.3f", db->aPL/num);
		excelNumCell(fpout, "%.3f", db->d3S/num);
		excelNumCell(fpout, "%.3f", db->dPAST/num);
		excelNumCell(fpout, "%.3f", db->aPAST/num);
		excelNumCell(fpout, "%.3f", db->dPASTP/num);
		excelNumCell(fpout, "%.3f", db->aPASTP/num);
		excelNumCell(fpout, "%.3f", db->dPRESP/num);
		excelNumCell(fpout, "%.3f", db->RIV/num);   // griv, ariv
		excelNumCell(fpout, "%.3f", db->IIV/num);   // giiv, aiiv
		excelNumCell(fpout, "%.3f", db->SNTP/num);  // sntp, aspt
		excelNumCell(fpout, "%.3f", db->SCSS/num);  // scss, sntp
		excelNumCell(fpout, "%.3f", db->SFSYN/num); // sfsyn, sntp
		excelNumCell(fpout, "%.3f", db->SFSEM/num); // sfsem, sntp
		excelNumCell(fpout, "%.3f", db->SCRCS/num); // scrc, scrs
		excelNumCell(fpout, "%.3f", db->AECS/num);  // aecs, espt
		excelNumCell(fpout, "%.3f", db->PV1/num);   // pv1, allv
		excelNumCell(fpout, "%.3f", db->PV2/num);   // pv2, allv
		excelNumCell(fpout, "%.3f", db->PV3/num);   // pv2, allv
		excelNumCell(fpout, "%.3f", db->pvg1/num);
		excelNumCell(fpout, "%.3f", db->pvg2/num);
		excelNumCell(fpout, "%.3f", db->pvg3/num);
		excelRow(fpout, ExcelRowEnd);

		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"SD Database, , , , , , , , , , ");
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
				excelRowOneStrCell(fpout, ExcelRedCell, "CAUTION:  Analyses that use the +b, +g, +n, +r or +s options should not be directly compared to the database because those options cannot be selected for the database in C-NNLA");
			}
		}
//		printArg(targv, targc, fpout, FALSE, "");
	}
	excelFooter(fpout);
	sp_head = freespeakers(sp_head);
}

void usage() {
	printf("Usage: c-nnla [bS dS eN g gS lF n %s] filename(s)\n",mainflgs());
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
#if defined(_CLAN_DEBUG) || defined(UNX)
	puts("+c : create C-NNLA database file (c-nnla -c +t*PAR)");
	puts("	First set working directory to: ~aphasia-data");
#endif // _CLAN_DEBUG
	puts("+d1: debug mode for all");
	puts("+d1R:correct irregular verb inflection(1)");
	puts("+d1I:correct irregular verb inflection(2)");
	puts("+d1P:ssentences produced(3)");
	puts("+d1S:sentence complexity ratio(7)");
	puts("+d1E:# embedded clauses(8)");
	puts("+d11:1-3 place verbs(8.5-11)");
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
	puts("       c-nnla adler15a.cha +t*par +gCinderella +u +d\"control|60-70\"");
	puts("   Search database for \"Cinderella\" gems of \"control\" subjects between ages of 60 and 70 and 6 month");
	puts("       c-nnla adler15a.cha +t*par +gCinderella +u +d\"control|60-70;6\"");
	puts("   Search database for \"Cinderella\" gems of \"Nonfluent\" subjects of any age");
	puts("       c-nnla adler15a.cha +t*par +gCinderella +u +dNonfluent");
	puts("   Just compute results for \"Cinderella\" gems of adler15a.cha. Do not compare to database");
	puts("       c-nnla adler15a.cha +t*par +gCinderella +u");
	cutt_exit(0);
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
	struct c_nnla_speakers *ts;
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
		strcpy(c_nnla_BBS, "@*&#");
		strcpy(c_nnla_CBS, "@*&#");
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
				c_nnla_error(NULL, FALSE);
			} else
				fprintf(dbfpout, "V%d\n", C_NNLA_DB_VERSION);
		}
		isOutputGem = TRUE;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (utterance->speaker[0] == '*')
				break;
			if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (strncmp(utterance->line, "EVAL DATABASE EXCLUDE", 21) == 0) {
					fprintf(stderr,"    EXCLUDED FILE: %s\n",oldfname+wdOffset);
					return;
				}
			}
		}
		rewind(fpin);
		if (isPWordsList)
			fprintf(fpout, "From file <%s>\n", oldfname);
	} else if (c_nnla_SpecWords && !onlyApplyToDB) {
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
				if (isPostCodeOnUtt(utterance->line, "[+ exc]"))
					lRightspeaker = FALSE;
				else
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (!onlyApplyToDB && c_nnla_SpecWords && !strcmp(c_nnla_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				c_nnla_n_option = FALSE;
				strcpy(c_nnla_BBS, "@BG:");
				strcpy(c_nnla_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				c_nnla_n_option = TRUE;
				strcpy(c_nnla_BBS, "@G:");
				strcpy(c_nnla_CBS, "@*&#");
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
//						ts->words_root = c_nnla_freetree(ts->words_root);
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
//						ts->words_root = c_nnla_freetree(ts->words_root);
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
		if (!onlyApplyToDB && uS.partcmp(utterance->speaker,c_nnla_BBS,FALSE,FALSE)) {
			if (c_nnla_n_option) {
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
		} else if (found > 0 && uS.partcmp(utterance->speaker,c_nnla_CBS,FALSE,FALSE)) {
			if (c_nnla_n_option) {
			} else {
				if (isRightText(word)) {
					found--;
					if (found == 0) {
						if (c_nnla_SpecWords)
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
				ts = c_nnla_FindSpeaker(oldfname, templineC, NULL, TRUE, NULL);
				isPWordsListFound = TRUE;
			}
			c_nnla_process_tier(ts, NULL, NULL, isOutputGem, PWordsListFP);
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
		c_nnla_pr_result();
}

static struct Vs *parseV(FILE *fp, FNType *mFileName, struct Vs *root) {
	int  i, ln;
	char isV;
	char isPart;
	struct Vs *p;

	ln = 0;
	while (fgets_cr(templineC2, UTTLINELEN, fp)) {
		ln++;
		uS.remblanks(templineC2);
		if (templineC2[0] == '%' || templineC2[0] == '#' || templineC2[0] == EOS)
			continue;
		if (uS.isUTF8(templineC2) || uS.isInvisibleHeader(templineC2) ||
			strncmp(templineC2, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0)
			continue;
		isV = FALSE;
		isPart = FALSE;
		for (i=0; templineC2[i] != EOS && templineC2[i] != ';'; i++) {
			if (templineC2[i] == '|' && templineC2[i+1] == 'v' && templineC2[i+2] == ',')
				isV = TRUE;
			else if (templineC2[i] == '|' && templineC2[i+1] == 'p' && templineC2[i+2] == 'a' &&
				templineC2[i+3] == 'r' && templineC2[i+4] == 't')
				isPart = TRUE;
		}
		if (templineC2[i] == ';') {
			if (root == NULL) {
				root = NEW(struct Vs);
				p = root;
			} else {
				for (p=root; p->nextV != NULL; p=p->nextV) ;
				p->nextV = NEW(struct Vs);
				p = p->nextV;
			}
			if (p == NULL) {
				c_nnla_error(NULL, TRUE);
			}
			p->nextV = NULL;
			p->word = NULL;
			p->isV = isV;
			p->isPart = isPart;
			p->word = c_nnla_strsave(templineC2+i+1);
		} else if (templineC2[i] == EOS) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, ln);
			fprintf(stderr, "Can't find word stem ';' on line %d\n", ln);
			fprintf(stderr, "%s\n", templineC2);
			c_nnla_error(NULL, FALSE);
		}
	}
	fclose(fp);
	return(root);
}

static struct Vs *readVFile(const char *fname, char isCheckWDdir, struct Vs *v) {
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
		addFilename2Path(mFileName, "c-nnla");
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName,"r");
	}
	if (fp == NULL) {
		fprintf(stderr, "\nERROR: Can't locate verbs file: \"%s\".\n", mFileName);
		fprintf(stderr, "Check to see if \"lib\" directory in Commands window is set correctly.\n\n");
		c_nnla_error(NULL, FALSE);
	} else {
		v = parseV(fp, mFileName, v);
	}
	fprintf(stderr,"    Using verbs file: %s\n", mFileName);
	return(v);
}

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
		c_nnla_SpecWords = 0;
		c_nnla_group = FALSE;
		onlyApplyToDB = FALSE;
		isGOptionSet = FALSE;
		c_nnla_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(c_nnla_BBS, "@*&#");
		strcpy(c_nnla_CBS, "@*&#");
		isSpeakerNameGiven = FALSE;
		v1 = NULL;
		v2op = NULL;
		v2ob = NULL;
		v3op = NULL;
		v3ob = NULL;
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
				fprintf(stderr,"For example, \"c-nnla +leng\" or \"c-nnla +leng.cut\".\n");
				cutt_exit(0);
			}
			if (isExcludeWords && DBKeyRoot != NULL) {
				fprintf(stderr, "ERROR:  Analyses that use the +s option cannot be compared to the database, \n");
				fprintf(stderr, "        because +s option cannot be applied to the database in C-NNLA\n");
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
				c_nnla_error(NULL, FALSE);
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
				c_nnla_error(NULL, FALSE);
			}
			v1 = readVFile("v1.cut",   TRUE, v1);
			v2op = readVFile("v2op.cut", TRUE, v2op);
			v2ob = readVFile("v2ob.cut", TRUE, v2ob);
			v3op = readVFile("v3op.cut", TRUE, v3op);
			v3ob = readVFile("v3ob.cut", TRUE, v3ob);
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
					c_nnla_error(db, TRUE);
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
	char filesPath[2][128], langPref[32];
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
	CLAN_PROG_NUM = C_NNLA;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	DBFilesListFP = NULL;
	isCreateDB = 0;
	isDebugMode = FALSE;
	isFileGiven = FALSE;
	isLangGiven = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'c') {
				isCreateDB = 1;
			} else if (argv[i][1] == 'l') {
				isLangGiven = TRUE;
			} else if (argv[i][1] == 'd') {
				if (argv[i][2] == '1') {
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
	}
	bmain(argc,argv,c_nnla_pr_result);

	v1 = freeVs(v1);
	v2op = freeVs(v2op);
	v2ob = freeVs(v2ob);
	v3op = freeVs(v3op);
	v3ob = freeVs(v3ob);
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
					c_nnla_group = TRUE;
					specialOptionUsed = TRUE;
				} else {
					isGOptionSet = TRUE;
					onlyApplyToDB = TRUE;
				}
			} else {
				GemMode = 'i';
				c_nnla_SpecWords++;
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
				c_nnla_n_option = TRUE;
				strcpy(c_nnla_BBS, "@G:");
				strcpy(c_nnla_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(c_nnla_BBS, "@BG:");
				strcpy(c_nnla_CBS, "@EG:");
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
					c_nnla_error(NULL, FALSE);
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
