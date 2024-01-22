/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef DSSDEF
#define DSSDEF

extern "C"
{

#define DSSSPLEN 128
#define DSSRULES "eng.cut"
#define DSSBSSRULES "bss.cut"
#define DSSJPRULES "jpn.cut"
#define DSSNOCOMPUTE "DO_NOT_COMPUTE"

#define POINTS_N 20

#define DSSSP struct speakers_dss
DSSSP {
	char sp[DSSSPLEN+1];
	char *ID;
	char **utts;
	int  TotalsPoints[POINTS_N];
	int  uttnum;
	float TotalNum;
	float GrandTotal;
	DSSSP *next_sp;
} ;

extern int  DSS_UTTLIM;
extern char dss_lang;
extern char isDssFileSpecified;
extern const char *rulesfile;
extern FILE *debug_dss_fp;

extern int PassedDSSMorTests(char *osp, char *mor, char *dss);

extern float PrintOutDSSTable(char *line, int spNum, int spRowNum, char isOutput);

extern char isUttsLimit(DSSSP *tsp);
extern char isDSSRepeatUtt(DSSSP *tsp, char *line);
extern char addToDSSUtts(DSSSP *tsp, char *line);
extern char init_dss(char first);
extern char make_DSS_tier(char *mor_tier);

extern void dss_freeSpeakers(void);
extern void freeLastDSSUtt(DSSSP *tsp);
extern void dss_cleanSearchMem(void);

extern DSSSP *dss_FindSpeaker(char *sp, char *ID);

}

#endif /* DSSDEF */
