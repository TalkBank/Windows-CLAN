/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef _MLLIB
#define _MLLIB

#if defined(UNX)
#define mlu_excludeUtter mlx_excludeUtter
#define mlt_excludeUtter mlx_excludeUtter
#endif

extern struct IDtype *IDField;
extern struct IDtype *SPRole;

extern void SetSPRoleIDs(char *line);
extern void SetSPRoleParticipants(char *line);
extern void SetIDTier(char *line);


extern "C"
{
extern char ml_isSkip;
extern char ml_isXXXFound;
extern char ml_isYYYFound;
extern char ml_isWWWFound;
extern char ml_isPlusSUsed;
extern char isMLUEpostcode;
extern char ml_spchanged;
extern char ml_WdMode;
extern IEWORDS *ml_WdHead;


extern void ml_mkwdlist(char opt, char ch, FNType *fname);
extern void ml_addwd(char opt, char ch, char *wd);
extern void ml_AddClause(char opt, char *s);
extern void ml_AddClauseFromFile(char opt, FNType *fname);
extern void ml_exit(int i);
//extern void ml_checktier(char *s);
extern char mlu_excludeUtter(char *line, int pos, char *isWordsFound);
extern char mlt_excludeUtter(char *line, int pos, char *isWordsFound);
extern int  ml_getwholeutter(void);
}

#endif // _MLLIB
