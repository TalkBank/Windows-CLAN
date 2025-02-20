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

 initTSVars
 sp_head
 sugar_FindSpeaker

*/

#define CHAT_MODE 1
#include "cu.h"
#include <math.h>
#if defined(_MAC_CODE) || defined(UNX)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#if !defined(UNX)
#define _main sugar_main
#define call sugar_call
#define getflag sugar_getflag
#define init sugar_init
#define usage sugar_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

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

struct sugar_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.
	char *fname;
	char *sp;
	char *ID;

	float tUtt;
	float totalWords; // TNW
	float mluMorfS;   // MLU
	float mluUtt;
	float vUtt;
	float vWords;     //WPS
	float clauses;    //CPS
	struct sugar_speakers *next_sp;
} ;

static int  targc;
#if defined(_MAC_CODE) || defined(_WIN32)
static char **targv;
#endif
#ifdef UNX
static char *targv[MAX_ARGS];
#endif
static float MinUttLimit;
static char  isSpeakerNameGiven, isExcludeWords, ftime, specialOptionUsed, isDebug;
static struct sugar_speakers *sp_head;


static struct sugar_speakers *freespeakers(struct sugar_speakers *p) {
	struct sugar_speakers *ts;

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

static void sugar_error(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	cutt_exit(0);
}

static void initTSVars(struct sugar_speakers *ts, char isAll) {
	ts->isMORFound	= FALSE;
	ts->isPSDFound	= FALSE;
	ts->tUtt		= (float)0.0;
	ts->totalWords	= (float)0.0;
	ts->mluMorfS	= (float)0.0;
	ts->mluUtt		= (float)0.0;
	ts->vWords      = (float)0.0;
	ts->vUtt		= (float)0.0;
	ts->clauses	    = (float)0.0;
}

static struct sugar_speakers *sugar_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound) {
	struct sugar_speakers *ts, *tsp;

	tsp = sp_head;
	uS.remblanks(sp);
	for (ts=tsp; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
			fprintf(stderr, "\nERROR: More than 1 speaker selected in a file: %s\n", fname);
			sugar_error(FALSE);
		}
	}
	if ((ts=NEW(struct sugar_speakers)) == NULL)
		sugar_error(TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		sugar_error(TRUE);
	}	
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		sugar_error(TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			sugar_error(TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	initTSVars(ts, TRUE);
	if (sp_head == NULL) {
		sp_head = ts;
	} else {
		for (tsp=sp_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
		tsp->next_sp = ts;
	}
	ts->next_sp = NULL;
	return(ts);
}

static char sugar_isUttDel(char *line, int pos) {
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

static void sugar_process_tier(struct sugar_speakers *ts, char *isVerbFound) {
	int i, j, wi, oPos;
	char morWord[1024], graWord[1024], word[BUFSIZ+1];
	char tchr, isWordsFound, sq, aq, *vb, isSkip;
	char curPSDFound;
	float morphCnt;
	MORFEATS word_feats, *feat;

	if (utterance->speaker[0] == '*') {
		org_depTier[0] = EOS;
		strcpy(spareTier1, utterance->line);
		strcpy(spareTier3, uttline);
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (org_depTier[0] == EOS) {
			strcpy(org_depTier, utterance->line);
		}
		if (ts == NULL)
			return;

		filterwords("%", uttline, excludePlusAndUttDels);
		if (isExcludeWords)
			filterwords("%", uttline, exclude);

		*isVerbFound = FALSE;
		i = 0;
		while ((i=getword(utterance->speaker, uttline, morWord, NULL, i))) {
			if (!isWordFromMORTier(morWord))
				continue;
			if (ParseWordMorElems(morWord, &word_feats) == FALSE)
				sugar_error(TRUE);
			for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
				if (isEqual("v", feat->pos)   || isEqual("cop", feat->pos) || isEqual("aux", feat->pos) ||
					isEqual("mod", feat->pos) || isEqual("part", feat->pos)) {
					*isVerbFound = TRUE;
				} else if (isEqual("mod:aux", feat->pos)) {
					*isVerbFound = TRUE;
					ts->mluMorfS = ts->mluMorfS + (float)1.0;
				}
			}
			freeUpFeats(&word_feats);
		}
		if (*isVerbFound == FALSE) {
			j = 0;
			if (isDebug) {
				fprintf(stderr, "%s%s", org_spName, org_spTier);
				fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
			}
		}

		for (i=0; spareTier1[i] != EOS; i++) {
			if ((i == 0 || uS.isskip(spareTier1,i-1,&dFnt,MBF)) && spareTier1[i] == '+' &&
				uS.isRightChar(spareTier1,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->mluUtt > 0.0)
					ts->mluUtt--;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				if (ts->vUtt > 0.0)
					ts->vUtt--;
				ts->isPSDFound = FALSE;
			}
		}

		i = 0;
		while ((i=getword(utterance->speaker, uttline, morWord, NULL, i))) {
			uS.remblanks(morWord);
			if (strchr(morWord, '|') != NULL) {
				if (*isVerbFound == TRUE)
					ts->vWords++;
				ts->totalWords++;
/* 2019-08-12
				for (j=0; word[j] != EOS; j++) {
					if (word[j] == '$' && !isPrecliticUse) {
						if (*isVerbFound == TRUE)
							ts->vWords++;
						ts->totalWords++;
					}
					if (word[j] == '~' && !isPostcliticUse) {
						if (*isVerbFound == TRUE)
							ts->vWords++;
						ts->totalWords++;
					}
				}
*/
			}
		}
		isSkip = FALSE;
		i = 0;
		while ((i=getword(utterance->speaker, spareTier3, word, NULL, i))) {
			if (strcmp(word, "xxx") == 0 || strcmp(word, "yyy") == 0 || strcmp(word, "www") == 0) {
				isSkip = TRUE;
				break;
			}
		}
		ts->isMORFound = TRUE;
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
				if (!isSkip)
					ts->mluMorfS = ts->mluMorfS + morphCnt;
				uttline[i] = tchr;
			}
			if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' && 
				uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
				if (ts->mluUtt > 0.0)
					ts->mluUtt--;
				else if (ts->mluUtt == 0.0)
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				if (ts->vUtt > 0.0)
					ts->vUtt--;
				ts->isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, i, TRUE)) {
				if (ts->mluUtt > 0.0)
					ts->mluUtt--;
				else if (ts->mluUtt == 0.0)
					isWordsFound = FALSE;
				if (ts->tUtt > 0.0)
					ts->tUtt--;
				if (ts->vUtt > 0.0)
					ts->vUtt--;
			}
			if (isTierContSymbol(utterance->line, i, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
				curPSDFound = TRUE;
			if (sugar_isUttDel(utterance->line, i)) {
				if (uS.isRightChar(utterance->line, i, '[', &dFnt, MBF)) {
					for (; utterance->line[i] && !uS.isRightChar(utterance->line, i, ']', &dFnt, MBF); i++) ;
				}
				if (uS.isRightChar(utterance->line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
					if (!isSkip)
						ts->mluUtt = ts->mluUtt + (float)1.0;
					isWordsFound = FALSE;
				} else
					j = 0;
				ts->tUtt = ts->tUtt + (float)1.0;
				if (*isVerbFound == TRUE)
					ts->vUtt = ts->vUtt + (float)1.0;
			}
			if (uttline[i])
				i++;
			else
				break;
		} while (uttline[i]) ;
		ts->isPSDFound = curPSDFound;
	} else if (uS.partcmp(utterance->speaker,"%gra:",FALSE,FALSE)) {
		if (ts == NULL)
			return;
		if (org_depTier[0] == EOS) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"ERROR: %%mor tier is not found before %%gra tier.\n");
			sugar_error(FALSE);
		}
		if (*isVerbFound == TRUE)
			ts->clauses = ts->clauses + (float)1.0;
		for (j=0; org_depTier[j] != EOS; j++) {
			if (org_depTier[j] == '~' || org_depTier[j] == '$')
				org_depTier[j] = ' ';
		}
		createMorUttline(utterance->line,org_depTier,"%gra",utterance->tuttline,FALSE,TRUE);
		strcpy(utterance->tuttline, utterance->line);
		filterMorTier(uttline, utterance->line, 2, TRUE);

		i = 0;
		while ((i=getNextDepTierPair(uttline, graWord, morWord, NULL, i)) != 0) {
			if (graWord[0] != EOS && morWord[0] != EOS) {
				if (!isWordFromMORTier(morWord))
					continue;
				vb = strrchr(graWord, '|');
				if (vb != NULL) {
					vb++;
					if (uS.mStricmp(vb, "CSUBJ") == 0 || uS.mStricmp(vb, "CPRED") == 0 ||
						uS.mStricmp(vb, "CPOBJ") == 0 || uS.mStricmp(vb, "COBJ") == 0 || uS.mStricmp(vb, "CJCT") == 0 ||
						uS.mStricmp(vb, "XJCT") == 0 || uS.mStricmp(vb, "CMOD") == 0 || uS.mStricmp(vb, "XMOD") == 0) {
						if (*isVerbFound == TRUE)
							ts->clauses++;
					} else if (uS.mStricmp(vb, "COMP") == 0) {
						wi = atoi(graWord);
						if (isNumItemPointedToBy(uttline, wi, "INF") == FALSE) {
							if (*isVerbFound == TRUE)
								ts->clauses++;
						} else {
							j++;
						}
					} else if (uS.mStricmp(vb, "NJCT") == 0) {
						if (*isVerbFound == TRUE) {
							if (strncmp(morWord, "prep|", 5) != 0)
								ts->clauses++;
							else
								j = 0;
						}
					}
				}
			}
		}
	} else if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
		if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
			uS.remblanks(utterance->line);
			sugar_FindSpeaker(oldfname, templineC, utterance->line, FALSE);
		}
	}
}

static void sugar_pr_result(void) {
	char   *sFName;
	struct sugar_speakers *ts;
	if (sp_head == NULL) {
		fprintf(stderr,"\nERROR: No speaker matching +t option found\n\n");
	}
	excelHeader(fpout, newfname, 75);
	excelHeightRowStart(fpout, 75);
//	excelRow(fpout, ExcelRowStart);
	excelStrCell(fpout, "File");
	excelCommasStrCell(fpout, "Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field");
	excelStrCell(fpout, "MLU-S");
	excelStrCell(fpout, "TNW");
	excelStrCell(fpout, "WPS");
	excelStrCell(fpout, "CPS");
	excelRow(fpout, ExcelRowEnd);

	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			fprintf(stderr,"\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
		if (ts->tUtt < MinUttLimit) {
			fprintf(stderr, "\n\nCAN'T FIND MINIMUM OF %0.f UTTERANCES IN FILE: %s\n", MinUttLimit, ts->fname);
			fprintf(stderr, "FOUND ONLY %0.f COMPLETE UTTERANCES FOR SPEAKER: %s\n", ts->tUtt, ts->sp);
			fprintf(stderr, "IF YOU WANT TO USE SMALLER SAMPLE SIZE, THEN USE \"+c\" OPTION\n\n");
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
			excelCommasStrCell(fpout, "0,0,0,0");
			excelRow(fpout, ExcelRowEnd);
			continue;
		}
		if (ts->mluUtt == 0.0)
			excelStrCell(fpout, "NA"); // MLU
		else
			excelNumCell(fpout, "%.3f", ts->mluMorfS/ts->mluUtt);

		excelNumCell(fpout, "%.0f", ts->totalWords); //TNW

		if (ts->vUtt == 0.0)
			excelStrCell(fpout, "NA"); // WPS
		else
			excelNumCell(fpout, "%.3f", ts->vWords/ts->vUtt);

		if (ts->vUtt == 0.0)
			excelStrCell(fpout, "NA"); // CPS
		else
			excelNumCell(fpout, "%.3f", ts->clauses/ts->vUtt);

		excelRow(fpout, ExcelRowEnd);
//		printArg(targv, targc, fpout, FALSE, "");
	}
	excelFooter(fpout);
	sp_head = freespeakers(sp_head);
}

void usage() {
	printf("Usage: SUGAR [bS %s] filename(s)\n",mainflgs());
	puts("+aN: set minimal utterances number limit to N utterances (default: 50 minimal limit)");
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
	puts("-d : show utterances that do not meet at least one verb requirement");
	mainusage(FALSE);
	cutt_exit(0);
}

void call() {
	long tlineno = 0;
	char lRightspeaker, isVerbFound;
	struct sugar_speakers *ts;

	fprintf(stderr,"From file <%s>\n",oldfname);
	ts = NULL;
	spareTier1[0] = EOS;
	lRightspeaker = FALSE;
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
			if (*utterance->speaker == '*') {
				lRightspeaker = FALSE;
				isVerbFound = FALSE;
			}
			continue;
		} else {
			if (*utterance->speaker == '*') {
				if (isPostCodeOnUtt(utterance->line, "[+ exc]"))
					lRightspeaker = FALSE;
				else 
					lRightspeaker = TRUE;
				isVerbFound = FALSE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (utterance->speaker[0] == '*') {
			strcpy(templineC, utterance->speaker);
			ts = sugar_FindSpeaker(oldfname, templineC, NULL, TRUE);
			if (ts->tUtt == MinUttLimit)
				break;
		}
		if (*utterance->speaker == '@' || lRightspeaker) {
			sugar_process_tier(ts, &isVerbFound);
		}
	}
#if !defined(CLAN_SRV)
//	fprintf(stderr, "\r	  \r");
#endif
	if (!combinput)
		sugar_pr_result();
}

void init(char first) {
	int  i;
	char *f;

	if (first) {
		isDebug = FALSE;
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
		isSpeakerNameGiven = FALSE;
		sp_head = NULL;
		combinput = TRUE;
		MinUttLimit = 50.0;
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
				sugar_error(FALSE);
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
				sugar_error(FALSE);
			}
			maketierchoice("*:", '+', '\001');
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
	int i;

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
	CLAN_PROG_NUM = SUGAR;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
		}
	}
	bmain(argc,argv,sugar_pr_result);
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'a':
			if (isdigit(*f)) {
				j = atoi(f);
				MinUttLimit = (float)j;
			}
			break;
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
		case 'd':
			no_arg_option(f);
			isDebug = TRUE;
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
					sugar_error(FALSE);
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
