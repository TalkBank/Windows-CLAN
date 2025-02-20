/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#ifndef UNX
	#include "ced.h"
#else
	#include "c_curses.h"
#endif

#if !defined(UNX)
#define _main fixbullets_main
#define call fixbullets_call
#define getflag fixbullets_getflag
#define init fixbullets_init
#define usage fixbullets_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#if defined(UNX)
#define SPLIST struct speakers
#endif

SPLIST {
	char *sp;
	long endTime;
	SPLIST *nextsp;
} ;

#define CHATTIERS struct ChatTiers
CHATTIERS {
	long ln;
    char *sp;
    AttTYPE *attSp;
    char *line;
    AttTYPE *attLine;
    CHATTIERS *nextTier;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char GExt[];

static long offset;
static char isGemFile;
static char isMergeBullets;
static char isMergeFiles;
static long last_End = 0L;
static long tier_SNDBeg = 0L;
static long tier_SNDEnd = 0L;
static long tier_MOVBeg = 0L;
static long tier_MOVEnd = 0L;
static FNType cur_SNDFname[FILENAME_MAX];
static long cur_SNDBeg = 0L;
static long cur_SNDEnd = 0L;
static FNType cur_MOVFname[FILENAME_MAX];
static long cur_MOVBeg = 0L;
static long cur_MOVEnd = 0L;
static AttTYPE attLine[UTTLINELEN+1];
static AttTYPE atts[UTTLINELEN+1];
static CHATTIERS *tiersRoot;
static SPLIST *headsp;

static SPLIST *fixBull_clean_speaker(SPLIST *p) {
	SPLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		if (t->sp)
			free(t->sp);
		free(t);
	}
	return(NULL);
}

void init(char f) {
	if (f) {
		offset = 0L;
		tiersRoot = NULL;
		headsp = NULL;
		isGemFile = FALSE;
		isMergeBullets = FALSE;
		isMergeFiles = FALSE;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		last_End = 0L;
		tier_SNDBeg = 0L;
		tier_SNDEnd = 0L;
		tier_MOVBeg = 0L;
		tier_MOVEnd = 0L;
		cur_SNDFname[0] = EOS;
		cur_SNDBeg = 0L;
		cur_SNDEnd = 0L;
		cur_MOVFname[0] = EOS;
		cur_MOVBeg = 0L;
		cur_MOVEnd = 0L;
		headsp = fixBull_clean_speaker(headsp);
	}
}

void usage() {
	printf("Fixes the consistency of bullet times, reformats old style bullets,\n");
	printf("and inserts an @Media header.\n");
	printf("Usage: fixbullets [b o %s] filename(s)\n", mainflgs());
	puts("+b : merge multiple bullets per line into one bullets per tier");
	puts("+g : zero out first bullet and offset the rest in a file GEM file");
	puts("+m : merge all files into one file with bullets progressively offset");
	puts("+oN: time offset value N (+o800 means add 800)");
	puts("-oN: time offset value N (-o800 means subtract 800)");
	mainusage(TRUE);
}

static void pr_end(void) {
	if (isMergeFiles)
		fprintf(fpout, "@End\n");
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = FIXBULLETS;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,pr_end);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'b':
			isMergeBullets = TRUE;
			no_arg_option(f);
			break;
		case 'g':
			isGemFile = TRUE;
			strcpy(GExt, ".slice");
			no_arg_option(f);
			break;
		case 'm':
			isMergeFiles = TRUE;
			combinput = TRUE;
			no_arg_option(f);
			break;
		case 'n':
			break;
		case 'o':
			if (*(f-2) == '+')
				offset = atol(getfarg(f,f1,i));
			else
				offset = 0 - atol(getfarg(f,f1,i));
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static CHATTIERS *freeTiers(CHATTIERS *p) {
	CHATTIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->sp != NULL)
			free(t->sp);
		if (t->attSp != NULL)
			free(t->attSp);
		if (t->line != NULL)
			free(t->line);
		if (t->attLine != NULL)
			free(t->attLine);
		free(t);
	}
	return(NULL);
}

static long nextTierBegin(CHATTIERS *tier, long SNDEnd) {
	long i, tBeg, tEnd;

	for (tier=tier->nextTier; tier != NULL; tier=tier->nextTier) {
		if (tier->sp[0] == '*') {
			for (i=0L; tier->line[i]; i++) {
				if (tier->line[i] == HIDEN_C) {
					if (isdigit(tier->line[i+1])) {
						if (getMediaTagInfo(tier->line+i, &tBeg, &tEnd)) {
							return(tBeg);
						}
					} else	
						return(SNDEnd+500);
				}
			}
		}
	}
	return(SNDEnd+500);
}

static char fixBull_setLastTime(char *s, long t) {
	SPLIST *tp;

	if (*s == '%' || *s == '@')
		return(FALSE);
	for (tp=headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			tp->endTime = t;
			return(TRUE);
		}
	}
	return(FALSE);
}

static long fixBull_getLatTime(char *s) {
	SPLIST *tp;

	if (*s == '%' || *s == '@')
		return(-2L);
	for (tp=headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			return(tp->endTime);
		}
	}
	return(-1L);
}

static void fixBull_addsp(char *s) {
	SPLIST *tp;

	if (*s != '*')
		return;
	if (headsp == NULL) {
		headsp = NEW(SPLIST);
		tp = headsp;
	} else {
		for (tp=headsp; 1; tp=tp->nextsp) {
			if (!strcmp(tp->sp,s)) {
				return;
			}
			if (tp->nextsp == NULL) {
				tp->nextsp = NEW(SPLIST);
				tp = tp->nextsp;
				break;
			}
		}
	}
	if (tp == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		tiersRoot = freeTiers(tiersRoot);
		headsp = fixBull_clean_speaker(headsp);
		cutt_exit(0);
	}
	tp->nextsp = NULL;
	tp->sp = NULL;
	if ((tp->sp=(char *)malloc(strlen(s)+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		tiersRoot = freeTiers(tiersRoot);
		headsp = fixBull_clean_speaker(headsp);
		cutt_exit(0);
	}
	strcpy(tp->sp, s);
	tp->endTime = 0L;
}

static char getRawMediaTagInfo(char *line, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;
	char s[256+2];
	
	if (*line == HIDEN_C)
		line++;
	
	while (*line && (isSpace(*line) || *line == '_'))
		line++;
	if (*line == EOS)
		return(FALSE);
	
	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	beg = atol(s);
	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	end = atol(s);
	*Beg = beg;
	*End = end;
	return(TRUE);
}

static void offsetBullets(char *word, AttTYPE *atts, char *isFirstBulletInFile) {
	long i, len, lenNew;
	AttTYPE att;

	if (word[0] == HIDEN_C && isdigit(word[1])) {
		if (getRawMediaTagInfo(word, &cur_SNDBeg, &cur_SNDEnd)) {
			if (*isFirstBulletInFile == TRUE) {
				offset = 0 - cur_SNDBeg;
				*isFirstBulletInFile = FALSE;
			}
			len = strlen(word);
			cur_SNDBeg = cur_SNDBeg + offset;
			if (cur_SNDBeg < 0L)
				cur_SNDBeg = 0L;
			cur_SNDEnd = cur_SNDEnd + offset;
			if (cur_SNDEnd < 0L)
				cur_SNDEnd = 0L;
			last_End = cur_SNDEnd;
			sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
			lenNew = strlen(word);
			if (len < lenNew) {
				att = atts[len-1];
				for (i=len; i < lenNew; i++)
					atts[i] = att;
			}
		}
	}
}

static void checkBulletsConsist(CHATTIERS *tier, char *word, AttTYPE *atts, long ln) {
	long i, len, lenNew;
	long tDiff;
	FNType lastFname[FILENAME_MAX], *s;
	long lastBegTime;
	long lastEndTime;
	AttTYPE att;

	if (word[0] == HIDEN_C && isdigit(word[1])) {
		lastBegTime = cur_SNDBeg;
		lastEndTime = cur_SNDEnd;
		if (getMediaTagInfo(word, &cur_SNDBeg, &cur_SNDEnd)) {
			if (offset != 0) {
				len = strlen(word);
				cur_SNDBeg = cur_SNDBeg + offset;
				if (cur_SNDBeg < 0L)
					cur_SNDBeg = 0L;
				cur_SNDEnd = cur_SNDEnd + offset;
				if (cur_SNDEnd < 0L)
					cur_SNDEnd = 0L;
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
				lenNew = strlen(word);
				if (len < lenNew) {
					att = atts[len-1];
					for (i=len; i < lenNew; i++)
						atts[i] = att;
				}
			} else if (lastEndTime != 0L) {
				if (cur_SNDBeg < lastBegTime && tier->sp[0] == '*') {
					len = strlen(word);
					cur_SNDBeg = lastBegTime + 1;
					if (cur_SNDEnd < cur_SNDBeg)
						cur_SNDEnd = cur_SNDBeg + 1;
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				} else if (cur_SNDBeg >= cur_SNDEnd-1 && lastEndTime < cur_SNDBeg && tier->sp[0] == '*') {
					len = strlen(word);
					cur_SNDBeg = lastEndTime;
					if (cur_SNDEnd < cur_SNDBeg)
						cur_SNDEnd = cur_SNDBeg + 1;
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				} else if (cur_SNDBeg >= cur_SNDEnd-1 && tier->sp[0] == '*') {
					cur_SNDEnd = nextTierBegin(tier, cur_SNDEnd);
					if (cur_SNDBeg < cur_SNDEnd) {
						len = strlen(word);
						sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
						lenNew = strlen(word);
						if (len < lenNew) {
							att = atts[len-1];
							for (i=len; i < lenNew; i++)
								atts[i] = att;
						}
					} else {
						cur_SNDEnd += 500;
						len = strlen(word);
						sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
						lenNew = strlen(word);
						if (len < lenNew) {
							att = atts[len-1];
							for (i=len; i < lenNew; i++)
								atts[i] = att;
						}
					}
				}
				tDiff = fixBull_getLatTime(tier->sp) - cur_SNDBeg;
				if (tDiff > 500L && tDiff <= 1000L) {
					cur_SNDBeg = fixBull_getLatTime(tier->sp);
					if (cur_SNDBeg < cur_SNDEnd) {
						len = strlen(word);
						sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
						lenNew = strlen(word);
						if (len < lenNew) {
							att = atts[len-1];
							for (i=len; i < lenNew; i++)
								atts[i] = att;
						}
					}
				}
			}
			fixBull_setLastTime(tier->sp, cur_SNDEnd);
			if (isMergeBullets) {
				if (tier_SNDBeg == -1)
					tier_SNDBeg = cur_SNDBeg;
				tier_SNDEnd = cur_SNDEnd;
			}
		}
	}

	lastBegTime = cur_SNDBeg;
	lastEndTime = cur_SNDEnd;
	strcpy(lastFname, cur_SNDFname);
	if (getOLDMediaTagInfo(word, SOUNDTIER, cur_SNDFname, &cur_SNDBeg, &cur_SNDEnd)) {
		for (i=0; cur_SNDFname[i] != EOS; i++) {
			if (isSpace(cur_SNDFname[i])) {
				cur_SNDFname[i] = '-';
			}
		}
		s = strrchr(cur_SNDFname, '.');
		if (s != NULL) {
			len = strlen(s);
			if (len < 6) {
				*s = EOS;
			}
		}
		if (offset != 0) {
			len = strlen(word);
			cur_SNDBeg = cur_SNDBeg + offset;
			if (cur_SNDBeg < 0L)
				cur_SNDBeg = 0L;
			cur_SNDEnd = cur_SNDEnd + offset;
			if (cur_SNDEnd < 0L)
				cur_SNDEnd = 0L;
			sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
			lenNew = strlen(word);
			if (len < lenNew) {
				att = atts[len-1];
				for (i=len; i < lenNew; i++)
					atts[i] = att;
			}
		} else if (uS.mStricmp(lastFname, cur_SNDFname) == 0 && lastEndTime != 0L) {
			if (cur_SNDBeg < lastBegTime && tier->sp[0] == '*') {
				len = strlen(word);
				cur_SNDBeg = lastBegTime + 1;
				if (cur_SNDEnd < cur_SNDBeg)
					cur_SNDEnd = cur_SNDBeg + 1;
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
				lenNew = strlen(word);
				if (len < lenNew) {
					att = atts[len-1];
					for (i=len; i < lenNew; i++)
						atts[i] = att;
				}
			} else if (cur_SNDBeg >= cur_SNDEnd-1 && lastEndTime < cur_SNDBeg && tier->sp[0] == '*') {
				len = strlen(word);
				cur_SNDBeg = lastEndTime;
				if (cur_SNDEnd < cur_SNDBeg)
					cur_SNDEnd = cur_SNDBeg + 1;
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
				lenNew = strlen(word);
				if (len < lenNew) {
					att = atts[len-1];
					for (i=len; i < lenNew; i++)
						atts[i] = att;
				}
			} else if (cur_SNDBeg >= cur_SNDEnd-1 && tier->sp[0] == '*') {
				cur_SNDEnd = nextTierBegin(tier, cur_SNDEnd);
				if (cur_SNDBeg < cur_SNDEnd) {
					len = strlen(word);
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				} else {
					cur_SNDEnd += 500;
					len = strlen(word);
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				}
			} else
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);
		} else
			sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_SNDBeg, cur_SNDEnd, HIDEN_C);

		if (lastFname[0] != EOS && uS.mStricmp(lastFname, cur_SNDFname) != 0) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "ONLY one media file can be used in one transcript data file.\n");
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		fixBull_setLastTime(tier->sp, cur_SNDEnd);
		if (isMergeBullets) {
			if (tier_SNDBeg == -1)
				tier_SNDBeg = cur_SNDBeg;
			tier_SNDEnd = cur_SNDEnd;
		}
	} else if (word[0] == HIDEN_C && uS.mStrnicmp(word+1, SOUNDTIER, strlen(SOUNDTIER)) == 0) {
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
		fprintf(stderr, "Possibly corrupt bullet.\n");
		if (lastFname[0] != EOS && uS.mStricmp(lastFname, cur_MOVFname) != 0) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "ONLY one media file can be used in one transcript data file.\n");
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
	}

	lastBegTime = cur_MOVBeg;
	lastEndTime = cur_MOVEnd;
	strcpy(lastFname, cur_MOVFname);
	if (getOLDMediaTagInfo(word, REMOVEMOVIETAG, cur_MOVFname, &cur_MOVBeg, &cur_MOVEnd)) {
		for (i=0; cur_MOVFname[i] != EOS; i++) {
			if (isSpace(cur_MOVFname[i])) {
				cur_MOVFname[i] = '-';
			}
		}
		s = strrchr(cur_MOVFname, '.');
		if (s != NULL) {
			len = strlen(s);
			if (len < 6) {
				*s = EOS;
			}
		}
		if (offset != 0) {
			len = strlen(word);
			cur_MOVBeg = cur_MOVBeg + offset;
			if (cur_MOVBeg < 0L)
				cur_MOVBeg = 0L;
			cur_MOVEnd = cur_MOVEnd + offset;
			if (cur_MOVEnd < 0L)
				cur_MOVEnd = 0L;
			sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
			lenNew = strlen(word);
			if (len < lenNew) {
				att = atts[len-1];
				for (i=len; i < lenNew; i++)
					atts[i] = att;
			}
		} else if (uS.mStricmp(lastFname, cur_MOVFname) == 0 && lastEndTime != 0L) {
			if (cur_MOVBeg < lastBegTime && tier->sp[0] == '*') {
				len = strlen(word);
				cur_MOVBeg = lastBegTime + 1;
				if (cur_MOVEnd < cur_MOVBeg)
					cur_MOVEnd = cur_MOVBeg + 1;
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
				lenNew = strlen(word);
				if (len < lenNew) {
					att = atts[len-1];
					for (i=len; i < lenNew; i++)
						atts[i] = att;
				}
			} else if (cur_MOVBeg >= cur_MOVEnd-1 && lastEndTime < cur_MOVBeg && tier->sp[0] == '*') {
				len = strlen(word);
				cur_MOVBeg = lastEndTime;
				if (cur_MOVEnd < cur_MOVBeg)
					cur_MOVEnd = cur_MOVBeg + 1;
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
				lenNew = strlen(word);
				if (len < lenNew) {
					att = atts[len-1];
					for (i=len; i < lenNew; i++)
						atts[i] = att;
				}
			} else if (cur_MOVBeg >= cur_MOVEnd-1 && tier->sp[0] == '*') {
				cur_MOVEnd = nextTierBegin(tier, cur_MOVEnd);
				if (cur_MOVBeg < cur_MOVEnd) {
					len = strlen(word);
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				} else {
					cur_MOVEnd += 500;
					len = strlen(word);
					sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
					lenNew = strlen(word);
					if (len < lenNew) {
						att = atts[len-1];
						for (i=len; i < lenNew; i++)
							atts[i] = att;
					}
				}
			} else
				sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
		} else
			sprintf(word, "%c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
		fixBull_setLastTime(tier->sp, cur_MOVEnd);
		if (lastFname[0] != EOS && uS.mStricmp(lastFname, cur_MOVFname) != 0) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "ONLY one media file can be used in one transcript data file.\n");
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		if (isMergeBullets) {
			if (tier_MOVBeg == -1)
				tier_MOVBeg = cur_MOVBeg;
			tier_MOVEnd = cur_MOVEnd;
		}
	} else if (word[0] == HIDEN_C && uS.mStrnicmp(word+1, REMOVEMOVIETAG, strlen(REMOVEMOVIETAG)) == 0) {
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
		fprintf(stderr, "Possibly corrupt bullet.\n");
		if (lastFname[0] != EOS && uS.mStricmp(lastFname, cur_MOVFname) != 0) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "ONLY one media file can be used in one transcript data file.\n");
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
	}
}

static char foundMediaName(char *line, char *tMediaFName) {
	char buf[SPEAKERLEN];
	int i;

	line++;
	for (i=0; *line && *line != ':' && i < 256; line++)
		buf[i++] = *line;
	if (*line == EOS) {
		return(FALSE);
	}
	buf[i++] = *line;
	buf[i] = EOS;
	line++;
	if (!uS.partcmp(buf, SOUNDTIER, FALSE, FALSE) && !uS.partcmp(buf, REMOVEMOVIETAG, FALSE, FALSE)) {
		return(FALSE);
	}

	while (*line && (isSpace(*line) || *line == '_'))
		line++;
	if (*line == EOS) {
		return(FALSE);
	}
	if (*line != '"') {
		return(FALSE);
	}
	line++;
	if (*line == EOS) {
		return(FALSE);
	}

	for (i=0; *line && *line != '"' && i < 256; line++)
		tMediaFName[i++] = *line;
	tMediaFName[i] = EOS;

	if (uS.partcmp(buf, SOUNDTIER, FALSE, FALSE)) {
		strcat(tMediaFName, ", audio");
	}
	if (uS.partcmp(buf, REMOVEMOVIETAG, FALSE, FALSE)) {
		strcat(tMediaFName, ", video");
	}

	return(TRUE);
}

#if defined(_COCOA_APP)
//#include <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
//#include <AVFoundation/AVFoundation.h>
#import <AVFoundation/AVAsset.h>
//#include <CoreMedia/CoreMedia.h>
#endif

void call() {
	char isIDsfound, isBulletsFound, isCheckMedia, isFirstBulletInFile, *s;
	FNType tMediaFName[FILENAME_MAX];
	long i, ni, j;
	CHATTIERS *tier = NULL;

	isIDsfound = 0;
	isFirstBulletInFile = FALSE;
	if (isMergeFiles)
		isCheckMedia = FALSE;
	else {
		if (isGemFile)
			isFirstBulletInFile = TRUE;
		isCheckMedia = TRUE;
	}
	isBulletsFound = FALSE;
	tMediaFName[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (isCheckMedia) {
			if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
				if (isGemFile) {
					strcpy(tMediaFName, oldfname);
					s = strrchr(tMediaFName, '.');
					if (s != NULL) {
						*s = EOS;
						s = strrchr(tMediaFName, '.');
						if (s != NULL) {
							if (uS.mStricmp(s, ".gem") == 0)
								*s = EOS;
						}
					}
					strcat(tMediaFName, ".slice");
					strcpy(templineC, utterance->line);
					for (i=0; templineC[i] != EOS && templineC[i] != ',' && !isSpace(templineC[i]); i++) ;
					strcpy(utterance->line, tMediaFName);
					strcat(utterance->line, templineC+i);
				}
				tMediaFName[0] = EOS;
				isCheckMedia = FALSE;
			}
			if (uS.partcmp(utterance->speaker, "@ID", FALSE, FALSE))
				isIDsfound = 1;
			for (i=0L; utterance->line[i]; i++) {
				if (utterance->line[i] == HIDEN_C) {
					if (foundMediaName(utterance->line+i, tMediaFName)) {
						isBulletsFound = TRUE;
						break;
					}
				}
			}
		}
		if (isMergeFiles) {
			if (uS.partcmp(utterance->speaker, "@End", FALSE, FALSE)) 
				continue;
			if (offset != 0) {
				if (uS.isUTF8(utterance->speaker) || uS.isInvisibleHeader(utterance->speaker))
					continue;
				if (uS.partcmp(utterance->speaker, "@Begin", FALSE, FALSE) ||
					uS.partcmp(utterance->speaker, "@Languages", FALSE, FALSE) ||
					uS.partcmp(utterance->speaker, "@Participants", FALSE, FALSE) ||
					uS.partcmp(utterance->speaker, "@ID", FALSE, FALSE) ||
					uS.partcmp(utterance->speaker, "@Media", FALSE, FALSE)
					) 
					continue;
			}
		}
		if (tiersRoot == NULL) {
			tier = NEW(CHATTIERS);
			tiersRoot = tier;
		} else if (tier != NULL) {
			tier->nextTier = NEW(CHATTIERS);
			tier = tier->nextTier;
		}
		if (tier == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		tier->nextTier = NULL;
		tier->sp = NULL;
		tier->attSp = NULL;
		tier->line = NULL;
		tier->attLine = NULL;
		tier->ln = lineno;
		i = strlen(utterance->speaker);
		if ((tier->sp=(char *)malloc(i+1)) == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		if ((tier->attSp=(AttTYPE *)malloc((i+1)*sizeof(AttTYPE))) == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		i = strlen(utterance->line);
		if ((tier->line=(char *)malloc(i+1)) == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		if ((tier->attLine=(AttTYPE *)malloc((i+1)*sizeof(AttTYPE))) == NULL) {
			fputs("ERROR: Out of memory.\n",stderr);
			tiersRoot = freeTiers(tiersRoot);
			headsp = fixBull_clean_speaker(headsp);
			cutt_exit(0);
		}
		att_cp(0L, tier->sp, utterance->speaker, tier->attSp, utterance->attSp);
		att_cp(0L, tier->line, utterance->line, tier->attLine, utterance->attLine);
		uS.remblanks(tier->sp);
		fixBull_addsp(tier->sp);
	}
	if (isBulletsFound && tMediaFName[0] == EOS) {
		fprintf(stderr, "Can't find any legal bullets with media file names.\n");
		tiersRoot = freeTiers(tiersRoot);
		headsp = fixBull_clean_speaker(headsp);
		cutt_exit(0);
	}
	for (tier=tiersRoot; tier != NULL; tier=tier->nextTier) {
		if (tMediaFName[0] != EOS && tier->sp[0] == '*') {
			fprintf(fpout, "%s\t%s\n", MEDIAHEADER, tMediaFName);
			tMediaFName[0] = EOS;
		}
		if (tMediaFName[0] != EOS && isIDsfound) {
			if (isIDsfound == 2 && !uS.partcmp(tier->sp, "@ID", FALSE, FALSE)) {
				fprintf(fpout, "%s\t%s\n", MEDIAHEADER, tMediaFName);
				tMediaFName[0] = EOS;
				isIDsfound = 0;
			} else if (uS.partcmp(tier->sp, "@ID", FALSE, FALSE))
				isIDsfound = 2;
		}
		tier_SNDBeg = -1;
		tier_MOVBeg = -1;
		ni = 0L;
		for (i=0L; tier->line[i]; i++) {
			if (tier->line[i] == HIDEN_C && (tier->line[i+1] == '%' || isdigit(tier->line[i+1]))) {
				j = 0L;
				do {
					spareTier2[j] = tier->line[i];
					atts[j] = tier->attLine[i];
					j++;
					i++;
				} while (tier->line[i] != HIDEN_C && tier->line[i] != EOS) ;
				spareTier2[j] = tier->line[i];
				atts[j] = tier->attLine[i];
				spareTier2[j+1] = EOS;
				if (tier->line[i] == HIDEN_C) {
					if (isMergeFiles || isGemFile)
						offsetBullets(spareTier2, atts, &isFirstBulletInFile);
					else
						checkBulletsConsist(tier, spareTier2, atts, tier->ln);
					if (!isMergeBullets) {
						att_cp(ni, spareTier1, spareTier2, attLine, atts);
						ni = strlen(spareTier1);
					}
				} else {
					att_cp(ni, spareTier1, spareTier2, attLine, atts);
					ni = strlen(spareTier1);
				}
				if (tier->line[i] == EOS)
					break;
			} else if (tier->line[i]==(char)0xe2 && tier->line[i+1]==(char)0x80 && tier->line[i+2]==(char)0xa2) {
				spareTier1[ni] = HIDEN_C;
				attLine[ni] = tier->attLine[i];
				ni++;
				i += 2;
			} else {
				spareTier1[ni] = tier->line[i];
				attLine[ni] = tier->attLine[i];
				ni++;
			}
		}

		if (isMergeBullets) {
			spareTier1[ni] = EOS;
			uS.remblanks(spareTier1);
			ni = strlen(spareTier1);
			if (tier_SNDBeg != -1) {
				sprintf(spareTier2, " %c%ld_%ld%c", HIDEN_C, tier_SNDBeg, tier_SNDEnd, HIDEN_C);
				att_cp(ni, spareTier1, spareTier2, attLine, NULL);
				ni = strlen(spareTier1);
			}
			if (tier_MOVBeg != -1) {
				sprintf(spareTier2, " %c%ld_%ld%c", HIDEN_C, cur_MOVBeg, cur_MOVEnd, HIDEN_C);
				att_cp(ni, spareTier1, spareTier2, attLine, NULL);
				ni = strlen(spareTier1);
			}
		}

		spareTier1[ni] = EOS;
		printout(tier->sp, spareTier1, tier->attSp, attLine, FALSE);
		if (tMediaFName[0] != EOS && !isIDsfound && uS.partcmp(tier->sp, "@Participants", FALSE, FALSE)) {
			fprintf(fpout, "%s\t%s\n", MEDIAHEADER, tMediaFName);
			tMediaFName[0] = EOS;
		}
	}
	tiersRoot = freeTiers(tiersRoot);
	if (isMergeFiles) {
#if defined(_COCOA_APP)
		char *e;

		if (oldfname[0] == '/')
			strcpy(tMediaFName, oldfname);
		else {
			strcpy(tMediaFName, wd_dir);
			addFilename2Path(tMediaFName, oldfname);
		}
		e = strrchr(tMediaFName, '.');
		if (e != NULL) {
			*e = EOS;
			strcat(tMediaFName, ".mp4");
			if (access(tMediaFName, 0)) {
				*e = EOS;
				strcat(tMediaFName, ".wav");
			}
		}

		if (access(tMediaFName, 0)) {
			printf("\n%c%cCAN'T OPEN MEDIA FILE: ", ATTMARKER, error_start);
			printf("%s", tMediaFName);
			printf("%c%c\n", ATTMARKER, error_end);
			printf("%c%cTHE BULLET TIME WILL NOT BE ACCURATE.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
		} else if (uS.mStricmp(e, ".wav") == 0) {
			printf("\n%c%cFOUND AUDIO, not video, MEDIA FILE: ", ATTMARKER, error_start);
			printf("%s", tMediaFName);
			printf("%c%c\n\n", ATTMARKER, error_end);
		}
		
		NSURL *audioURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:tMediaFName]];
		AVAsset *audioAsset = [AVAsset assetWithURL:audioURL];
		
		NSURL *videoURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:tMediaFName]];
		AVAsset *videoAsset = [AVAsset assetWithURL:videoURL];
		
		CMTime duration;
		if (CMTimeGetSeconds(audioAsset.duration) < CMTimeGetSeconds(videoAsset.duration)) {
			duration = audioAsset.duration;
		} else {
			duration = videoAsset.duration;
		}
		Float64 durationTime = CMTimeGetSeconds(duration);
		if (durationTime > 0.0)
			offset += (long)(durationTime * 1000.0000);
		else
			offset = last_End;
#else
		offset = last_End;
#endif
	}
}
