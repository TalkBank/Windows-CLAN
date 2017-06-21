/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

#define SPKRTIER 1
#define DEPTTIER 2
#define TAGSLEN  128
#define TBTSNUM 10
#define TBTSLEN 32

struct TagsRec {
	char isUsed;
	char whichTier;
	char tag[TAGSLEN];
	char depOn[TAGSLEN];
	char chatName[TAGSLEN];
	struct TagsRec *nextTag;
} ;
typedef struct TagsRec TAGS;

struct linesRec {
	char whichTier;
	char depOn[TAGSLEN];
	char chatName[TAGSLEN];
	char *line;
	long bt, et;
	struct linesRec *nextLine;
} ;
typedef struct linesRec LINESLIST;

static TAGS *tiersRoot, *excludedTags;
static LINESLIST *sLinesRoot, *dLinesRoot;
static FNType actualFName[FNSize];
static long ln;
static double timeOffset;
static char *MFN, TBTS[TBTSNUM][TBTSLEN+1];
static int  TBTScnt;
static char lEncodeSet;
static short lEncode;
#ifdef _MAC_CODE
static TextEncodingVariant lVariant;
#endif

extern struct tier *defheadtier;
extern char AddCEXExtension;
extern char OverWriteFile;
extern long option_flags[];
extern char GExt[];
extern char isRecursive;

void init(char f) {
	int i;
	extern char GExt[];

	if (f) {
		timeOffset = 0.0;
		excludedTags = NULL;
		tiersRoot = NULL;
		sLinesRoot = NULL;
		dLinesRoot = NULL;
		lEncodeSet = 0;
		lEncode = 0;
#ifdef _MAC_CODE
		lVariant = kTextEncodingDefaultVariant;
#endif
		MFN = NULL;
		OverWriteFile = TRUE;
		AddCEXExtension = FALSE;
		for (i=0; i < TBTSNUM; i++)
			TBTS[i][0] = EOS;
		TBTScnt = 0;
		stin = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (lEncodeSet == 0) {
			fprintf(stderr, "Please specify text encoding with +o option.\n");
			cutt_exit(0);
		}
		strcpy(GExt, ".cha");
		option_flags[CLAN_PROG_NUM] = 0L;
		if (tiersRoot == NULL) {
			fprintf(stderr,	"Please specify tiers translation table with +d option.\n");
			cutt_exit(0);
		}
	}
	stout = TRUE;
}

void usage() {
	printf("Converts WaveSurfer text files to CHAT text files\n");
	printf("Usage: temp [+dF +fN +mF +tN] filename(s)\n");
    puts("+dF: specify tags dependencies file F.");
    puts("+mF: specify movie file name F (default: input file name).");
    puts("+tN: specify movie segment start time offset relative to actual movie start.");
	puts("+oS: Specify code page");
	puts("     macl  - Mac Latin (German, Spanish ...)");
	puts("     macce - Mac Central Europe");
	puts("     pcl   - PC  Latin (German, Spanish ...)");
	puts("     pcce  - PC  Central Europe\n");
	
	puts("     macar - Mac Arabic");
	puts("     pcar  - PC  Arabic\n");
	
	puts("     maccs - Mac Chinese Simplified");
	puts("     macct - Mac Chinese Traditional");
	puts("     pccs  - PC  Chinese Simplified");
	puts("     pcct  - PC  Chinese Traditional\n");
	
	puts("     maccr - Mac Croatian");
	
	puts("     maccy - Mac Cyrillic");
	puts("     pccy  - PC  Cyrillic");
	
	puts("     machb - Mac Hebrew");
	puts("     pchb  - PC  Hebrew\n");
	
	puts("     macjp - Mac Japanese");
	puts("     pcjp  - PC  Japanese (Shift-JIS)\n");
	
	puts("     krn   - Mac and Windows Korean\n");
	
	puts("     macth - Mac Thai");
	puts("     pcth  - PC  Thai\n");
	
	puts("     pcturk- PC  Turkish\n");
	
	puts("     macvt - Mac Vietnamese");
	puts("     pcvt  - PC  Vietnamese\n");

	puts("Example: temp +dattribs.cut -opcl *");
	puts("         temp +dattribs.cut -omacl litman");
	puts("         temp +dattribs.cut -opcl BW5032");
	cutt_exit(0);
}

static TAGS *freeTiers(TAGS *p) {
	TAGS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTag;
		free(t);
	}
	return(NULL);
}

static LINESLIST *freeLines(LINESLIST *p) {
	LINESLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextLine;
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static void freeMem(void) {
	sLinesRoot = freeLines(sLinesRoot);
	dLinesRoot = freeLines(dLinesRoot);
	tiersRoot = freeTiers(tiersRoot);
	excludedTags = freeTiers(excludedTags);
}

static TAGS *add_each_tag(TAGS *root, char *fname, char whichTier, char *tag, char *depOn, char *chatName) {
	TAGS *p;

	if (tag == NULL)
		return(root);

	if (tag[0] == EOS)
		return(root);

	if (root == NULL) {
		if ((p=NEW(TAGS)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->nextTag != NULL && uS.mStricmp(p->tag, tag); p=p->nextTag) ;
		if (uS.mStricmp(p->tag, tag) == 0)
			return(root);

		if ((p->nextTag=NEW(TAGS)) == NULL)
			out_of_mem();
		p = p->nextTag;
	}

	p->nextTag = NULL;
	if ((strlen(tag) >= TAGSLEN) || (strlen(depOn) >= TAGSLEN) || (strlen(chatName) >= TAGSLEN)) {
		freeMem();
		fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr, "Tag(s) too long. Longer than %d characters\n", TAGSLEN);
		cutt_exit(0);
	}
	p->whichTier = whichTier;
	strcpy(p->tag, tag);
	strcpy(p->depOn, depOn);
	if (chatName[0] != '*' && chatName[0] != '%' && chatName[0] != '@') {
		strcpy(p->chatName, "%");
		strcat(p->chatName, chatName);
	} else
		strcpy(p->chatName, chatName);
	strcat(p->chatName, ":");
	return(root);
}

static void rd_dep_f(char *fname) {
	FILE *fp;
	char *tag, *depOn, *chatName;
	long i, j;
	FNType mFileName[FNSize];

	if (*fname == EOS) {
		fprintf(stderr,	"No dep. tags file specified.\n");
		cutt_exit(0);
	}

	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of dep. tags files:\n\t\"%s\", \"%s%s\"\n", fname, templineC, fname);
		cutt_exit(0);
	}
	ln = 0L;
	while (fgets_cr(templineC, 255, fp)) {
		ln++;
		if (uS.isUTF8(templineC))
			continue;
		if (templineC[0] == ';')
			continue;

		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;

		if (uS.partwcmp(templineC, "TBTS") || uS.partwcmp(templineC, "tbts")) {
			if (TBTScnt < TBTSNUM) {
				for (i=4; isSpace(templineC[i]); i++) ;
				strncpy(TBTS[TBTScnt], templineC+i, TBTSLEN);
				TBTS[TBTScnt][TBTSLEN] = EOS;
				TBTScnt++;
			}
		} else if (templineC[0] == '*') {
			for (i=0; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing speaker name\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			tag = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			if (templineC[i] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			depOn = templineC+i;
			tiersRoot = add_each_tag(tiersRoot, fname, SPKRTIER, depOn, depOn, tag);
		} else if (templineC[0] == '-') {
			for (i=0; isSpace(templineC[i]) || templineC[i] == '-'; i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			tag = templineC+i;
			excludedTags = add_each_tag(excludedTags, fname, DEPTTIER, tag, tag, tag);
		} else {
			for (i=0; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			tag = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			depOn = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			if (templineC[i] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			for (j=i; !isSpace(templineC[j]) && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			chatName = templineC+i;
			tiersRoot = add_each_tag(tiersRoot, fname, DEPTTIER, tag, depOn, chatName);
		}
	}
	fclose(fp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;

	bmain(argc,argv,NULL);

	sLinesRoot = freeLines(sLinesRoot);
	dLinesRoot = freeLines(dLinesRoot);
	tiersRoot = freeTiers(tiersRoot);
	excludedTags = freeTiers(excludedTags);
}

static double calculateOffset(char *s) {
	double time;

	time = (double)(atof(s) * (double)1000.0);
	return(time);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'd':
			if (*f) {
				rd_dep_f(getfarg(f,f1,i));
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'm':
			MFN = f;
			break;
		case 't':
			if (*f) {
				strcpy(templineC1, getfarg(f,f1,i));
				timeOffset = calculateOffset(templineC1);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'o':
			lEncodeSet = UTF8;
			
#ifdef _MAC_CODE
			if (!uS.mStricmp(f, "macl"))
				lEncode = kTextEncodingMacRoman;
			else if (!uS.mStricmp(f, "macce"))
				lEncode = kTextEncodingMacCentralEurRoman;
			else if (!uS.mStricmp(f, "pcl"))
				lEncode = kTextEncodingWindowsLatin1;
			else if (!uS.mStricmp(f, "pcce"))
				lEncode = kTextEncodingWindowsLatin2;
			
			else if (!uS.mStricmp(f, "macar"))
				lEncode = kTextEncodingMacArabic;
			else if (!uS.mStricmp(f, "pcar"))
				lEncode = kTextEncodingDOSArabic;
			
			else if (!uS.mStricmp(f, "maccs"))
				lEncode = kTextEncodingMacChineseSimp;
			else if (!uS.mStricmp(f, "macct"))
				lEncode = kTextEncodingMacChineseTrad;
			else if (!uS.mStricmp(f, "pccs"))
				lEncode = kTextEncodingDOSChineseSimplif;
			else if (!uS.mStricmp(f, "pcct"))
				lEncode = kTextEncodingDOSChineseTrad;
			
			else if (!uS.mStricmp(f, "maccr"))
				lEncode = kTextEncodingMacCroatian;
			
			else if (!uS.mStricmp(f, "maccy"))
				lEncode = kTextEncodingMacCyrillic;
			else if (!uS.mStricmp(f, "pccy"))
				lEncode = kTextEncodingWindowsCyrillic;
			
			else if (!uS.mStricmp(f, "machb"))
				lEncode = kTextEncodingMacHebrew;
			else if (!uS.mStricmp(f, "pchb"))
				lEncode = kTextEncodingDOSHebrew;
			
			else if (!uS.mStricmp(f, "macjp"))
				lEncode = kTextEncodingMacJapanese;
			else if (!uS.mStricmp(f, "pcjp"))
				lEncode = kTextEncodingDOSJapanese;
			else if (!uS.mStricmp(f, "macj1"))
				lEncode = kTextEncodingJIS_X0201_76;
			else if (!uS.mStricmp(f, "macj2"))
				lEncode = kTextEncodingJIS_X0208_83;
			else if (!uS.mStricmp(f, "macj3"))
				lEncode = kTextEncodingJIS_X0208_90;
			else if (!uS.mStricmp(f, "macj4"))
				lEncode = kTextEncodingJIS_X0212_90;
			else if (!uS.mStricmp(f, "macj5"))
				lEncode = kTextEncodingJIS_C6226_78;
			else if (!uS.mStricmp(f, "macj6"))
				lEncode = kTextEncodingShiftJIS_X0213_00;
			else if (!uS.mStricmp(f, "macj7"))
				lEncode = kTextEncodingISO_2022_JP;
			else if (!uS.mStricmp(f, "macj8"))
				lEncode = kTextEncodingISO_2022_JP_1;
			else if (!uS.mStricmp(f, "macj9"))
				lEncode = kTextEncodingISO_2022_JP_2;
			else if (!uS.mStricmp(f, "macj10"))
				lEncode = kTextEncodingISO_2022_JP_3;
			else if (!uS.mStricmp(f, "macj11"))
				lEncode = kTextEncodingEUC_JP;
			else if (!uS.mStricmp(f, "macj12"))
				lEncode = kTextEncodingShiftJIS;
			
			else if (!uS.mStricmp(f, "krn"))
				lEncode = kTextEncodingMacKorean;
			else if (!uS.mStricmp(f, "pckr"))
				lEncode = kTextEncodingDOSKorean;
			else if (!uS.mStricmp(f, "pckrj"))
				lEncode = kTextEncodingWindowsKoreanJohab;
			
			else if (!uS.mStricmp(f, "macth"))
				lEncode = kTextEncodingMacThai;
			else if (!uS.mStricmp(f, "pcth"))
				lEncode = kTextEncodingDOSThai;
			
			else if (!uS.mStricmp(f, "pcturk"))
				lEncode = kTextEncodingWindowsLatin5; // kTextEncodingDOSTurkish
			
			else if (!uS.mStricmp(f, "macvt"))
				lEncode = kTextEncodingMacVietnamese;
			else if (!uS.mStricmp(f, "pcvt"))
				lEncode = kTextEncodingWindowsVietnamese;
			
			
			else {
				if (*f == EOS)
					fprintf(stderr,"Unrecognized font option \"%s\"", f);
				else
					fprintf(stderr,"Unrecognized font option \"%s\". Please talk to Leonid.", f);
				cutt_exit(0);
			}
#endif
#ifdef _WIN32 
			if (!uS.mStricmp(f, "pcl"))
				lEncode = 1252;
			else if (!uS.mStricmp(f, "pcce"))
				lEncode = 1250;
			
			else if (!uS.mStricmp(f, "pcar"))
				lEncode = 1256;
			
			else if (!uS.mStricmp(f, "pccs"))
				lEncode = 936;
			else if (!uS.mStricmp(f, "pcct"))
				lEncode = 950;
			
			else if (!uS.mStricmp(f, "pccy"))
				lEncode = 1251;
			
			else if (!uS.mStricmp(f, "pchb"))
				lEncode = 1255;
			
			else if (!uS.mStricmp(f, "pcjp"))
				lEncode = 932;
			
			else if (!uS.mStricmp(f, "krn"))
				lEncode = 949;
			else if (!uS.mStricmp(f, "pckr"))
				lEncode = 949;
			else if (!uS.mStricmp(f, "pckrj"))
				lEncode = 1361;
			
			else if (!uS.mStricmp(f, "pcth"))
				lEncode = 874;
			
			else if (!uS.mStricmp(f, "pcturk"))
				lEncode = 1254; // 857
			
			else if (!uS.mStricmp(f, "pcvt"))
				lEncode = 1258;
			
			else {
				if (*f == EOS)
					fprintf(stderr,"Unrecognized font option \"%s\"", f);
				else
					fprintf(stderr,"Unrecognized font option \"%s\". Please talk to Leonid.", f);
				cutt_exit(0);
			}
#endif
			break;
		case 'r':
			if (*f == 'e')
				isRecursive = TRUE;
			break;
		case 's':
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

static char whatKindTag(char *tag, char *depOn, char *chatName) {
	TAGS *p;

	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (tag == NULL) {
			if (p->whichTier == SPKRTIER)
				return(SPKRTIER);
		} else if (uS.mStricmp(p->tag, tag) == 0) {
			if (chatName != NULL)
				strcpy(chatName, p->chatName);
			if (depOn != NULL)
				strcpy(depOn, p->depOn);
			return(p->whichTier);
		}
	}
	return(0);
}

static LINESLIST *insertEmptySpeaker(LINESLIST *root, LINESLIST **ont, LINESLIST *tnt, char *depOn, long bt, long et) {
	char chatName[TAGSLEN];
	LINESLIST *nt = *ont;

	if (nt == NULL) {
		tnt->nextLine = NEW(LINESLIST);
		if (tnt->nextLine == NULL)
			out_of_mem();
		nt = tnt->nextLine;
		nt->nextLine = NULL;
	} else if (nt == root) {
		root = NEW(LINESLIST);
		if (root == NULL)
			out_of_mem();
		root->nextLine = nt;
		nt = root;
	} else {
		nt = NEW(LINESLIST);
		if (nt == NULL)
			out_of_mem();
		nt->nextLine = tnt->nextLine;
		tnt->nextLine = nt;
	}

	whatKindTag(depOn, NULL, chatName);
	sprintf(templineC2, " %c%ld_%ld%c\n\t", HIDEN_C, bt, et, HIDEN_C);

	nt->whichTier = SPKRTIER;
	strcpy(nt->depOn, depOn);
	strcpy(nt->chatName, chatName);
	nt->line = (char *)malloc(strlen("0.")+strlen(templineC2)+1);
	if (nt->line == NULL) {
		freeMem();
		out_of_mem();
	}
	strcpy(nt->line, "0.");
	strcat(nt->line, templineC2);

	nt->bt = bt;
	nt->et = et;

	*ont = nt;
	return(root);
}

static LINESLIST *add_each_line(LINESLIST *root, char isMerge, char whichTier, char *depOn, char *chatName, char *line, long bt, long et, FILE *tFp) {
	long len;
	LINESLIST *nt, *tnt;

	if (root == NULL) {
		if ((root=NEW(LINESLIST)) == NULL)
			out_of_mem();
		nt = root;
		nt->nextLine = NULL;
	} else {
		tnt= root;
		nt = root;
		if (whichTier == SPKRTIER) {
			while (nt != NULL) {
				if (nt->whichTier == SPKRTIER && bt < nt->bt)
					break;
				tnt = nt;
				nt = nt->nextLine;
			}
		} else if (whichTier == DEPTTIER) {
			if (!isMerge) {
				while (nt != NULL) {
					if (bt < nt->bt)
						break;
					tnt = nt;
					nt = nt->nextLine;
				}
			} else {
				while (nt != NULL) {
					if (nt->whichTier == SPKRTIER && !uS.mStricmp(nt->depOn,depOn) && bt >= nt->bt && bt < nt->et) {
						tnt = nt;
						nt = nt->nextLine;
						break;
					} else if (nt->whichTier == SPKRTIER && bt < nt->bt) {
						root = insertEmptySpeaker(root, &nt, tnt, depOn, bt, et);
						tnt = nt;
						nt = nt->nextLine;
						break;
					}
					tnt = nt;
					nt = nt->nextLine;
				}
			}
		} else {
			return(root);
		}

		if (nt == NULL) {
			tnt->nextLine = NEW(LINESLIST);
			if (tnt->nextLine == NULL)
				out_of_mem();
			nt = tnt->nextLine;
			nt->nextLine = NULL;
		} else if (nt == root) {
			root = NEW(LINESLIST);
			if (root == NULL)
				out_of_mem();
			root->nextLine = nt;
			nt = root;
		} else {
			nt = NEW(LINESLIST);
			if (nt == NULL)
				out_of_mem();
			nt->nextLine = tnt->nextLine;
			tnt->nextLine = nt;
		}
	}
	nt->line = NULL;
	if (strlen(chatName) >= TAGSLEN || strlen(depOn) >= TAGSLEN) {
		freeMem();
		fprintf(stderr,"*** File \"%s%s\": line %ld.\n", DirPathName, actualFName, ln);
		fprintf(stderr, "Tag is too long > %d\n", TAGSLEN);
		if (tFp != NULL) {
			fclose(fpin);
			fpin = tFp;
		}
		cutt_exit(0);
	}
	if (whichTier == SPKRTIER) {
		for (len=strlen(line); len >= 0; len--) {
			if (uS.IsUtteranceDel(line, len))
				break;
		}
		if (len < 0)
			sprintf(templineC2, " +. %c%ld_%ld%c\n\t", HIDEN_C, bt, et, HIDEN_C);
		else
			sprintf(templineC2, " %c%ld_%ld%c\n\t", HIDEN_C, bt, et, HIDEN_C);
	} else
		templineC2[0] = EOS;

	nt->whichTier = whichTier;
	strcpy(nt->depOn, depOn);
	strcpy(nt->chatName, chatName);
	if (!isMerge) {
		nt->line = (char *)malloc(strlen(line)+strlen(templineC2)+1);
		if (nt->line == NULL) {
			freeMem();
			if (tFp != NULL) {
				fclose(fpin);
				fpin = tFp;
			}
			out_of_mem();
		}
		strcpy(nt->line, line);
		strcat(nt->line, templineC2);
	} else
		nt->line = line;
	nt->bt = bt;
	nt->et = et;
	return(root);
}

static void addDepsToSpeaker(void) {
	LINESLIST *t;

	while (dLinesRoot != NULL) {
		t = dLinesRoot;
		sLinesRoot = add_each_line(sLinesRoot, TRUE, t->whichTier, t->depOn, t->chatName, t->line, t->bt, t->et, NULL);	
		dLinesRoot = dLinesRoot->nextLine;
		free(t);
	}
}

static char excludeTag(char *tag) {
	TAGS *t;

	for (t=excludedTags; t != NULL; t=t->nextTag) {
		if (!uS.mStricmp(t->tag, tag))
			return(TRUE);
	}
	return(FALSE);
}

static void outputTiers(LINESLIST *p) {
	for (; p != NULL; p=p->nextLine) {
		uS.remblanks(p->line);
		if (p->line[0] != EOS)
			printout(p->chatName, p->line, NULL, NULL, TRUE);
	}
}

static void output_Participants_ID(void) {
	int  len, i;
	TAGS *p;

	templineC[0] = EOS;
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == SPKRTIER && p->isUsed) {
			len = strlen(templineC);
			if (len > 0) {
				strcat(templineC, ", ");
				len = strlen(templineC);
			}
			strcpy(templineC2, p->chatName+1);
			for (i=strlen(templineC2)-1; i >= 0 && (isSpace(templineC2[i]) || templineC2[i] == ':'); i--) ;
			templineC2[++i] = EOS;
			sprintf(templineC+len, "%s Child", templineC2);
		}
	}
	if (templineC[0] != EOS)
		printout("@Participants:", templineC, NULL, NULL, TRUE);
	
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == SPKRTIER && p->isUsed) {
			strcpy(templineC2, p->chatName+1);
			for (i=strlen(templineC2)-1; i >= 0 && (isSpace(templineC2[i]) || templineC2[i] == ':'); i--) ;
			templineC2[++i] = EOS;
			fprintf(fpout, "@ID:	en|sample|%s|||||Child||\n", templineC2);
		}
	}
}


static FILE *openFile(char *tag) {
	int index;
	FNType *ext, *dot, fname[FILENAME_MAX], tFName[FILENAME_MAX];
	FILE *fp;

	index = 1;
	while (index=Get_File(fname, index)) {
		strncpy(tFName, fname, FILENAME_MAX-1);
		tFName[FILENAME_MAX-1] = EOS;
		ext = strchr(fname, '.');
		if (ext == NULL)
			continue;
		*ext = EOS;
		ext++;
		dot = strchr(ext, '.');
		if (dot != NULL)
			*dot = EOS;
		if (uS.fIpatmat(fname, FileName1) && uS.mStricmp(ext, tag) == 0) {
			fp = fopen(tFName, "r");
			if (FileName2[0] == EOS)
				strcpy(FileName2, fname);
			strcpy(actualFName, tFName);
			return(fp);
		}
	}
	strcpy(actualFName, FileName1);
	return(NULL);
}

#ifdef _WIN32 
#include <mbstring.h>
static void AsciiToUnicodeToUTF8(char *src, char *line) {
	long UTF8Len;
	long total = strlen(src);
	long wchars=MultiByteToWideChar(lEncode,0,(const char*)src,total,NULL,0);
	
	MultiByteToWideChar(lEncode,0,(const char*)src,total,templineW,wchars);
	UnicodeToUTF8(templineW, wchars, (unsigned char *)line, (unsigned long *)&UTF8Len, UTTLINELEN);
	if (UTF8Len == 0 && wchars > 0) {
		putc('\n', stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error: Unable to convert the following line:\n");
		fprintf(stderr, "%s\n", src);
	}
}
#endif

#ifdef _MAC_CODE
static void AsciiToUnicodeToUTF8(char *src, char *line) {
	OSStatus err;
	long len;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	TextEncoding MacRomanEncoding;
	unsigned long ail, aol;
	
	MacRomanEncoding = CreateTextEncoding( (long)lEncode, lVariant, kTextEncodingDefaultFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error1: Unable to craete a converter.\n");
		fprintf(stderr, "%s\n", src);
		cutt_exit(0);
	}
	
	len = strlen(src);
	if ((err=TECConvertText(ec, (ConstTextPtr)src, len, &ail, (TextPtr)line, UTTLINELEN, &aol)) != noErr) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
		fprintf(stderr, "%s\n",src);
		cutt_exit(0);
	}
	err = TECDisposeConverter(ec);
	if (ail < len) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error3: Converted only %ld out of %ld chars:\n", ail, len);
		fprintf(stderr, "%s\n", src);
		cutt_exit(0);
	}
	line[aol] = EOS;
}
#endif

static void cleanupLine(char *st) {
	int b, e;

	b = 0;
	if (st[b] == '"') {
		for (e=strlen(st)-1; isSpace(st[e]) && e > b; e--) ;
		if (st[e] == '"') {
			for (b++; isSpace(st[b]); b++) ;
			strcpy(st, st+b);
			for (e=strlen(st)-1; (isSpace(st[e]) || st[e] == '"') && e >= 0; e--) ;
			st[e+1] = EOS;
		}
	}
}

static char processFiles(TAGS *p) {
	int  i;
	char *bS, *eS, *line, areTiersFound;
	long bt, et;
	FILE *tFp;

	tFp = fpin;
	fpin = openFile(p->tag);
	if (fpin == NULL) {
		fpin = tFp;
		if (!isRecursive)
			fprintf(stderr, "Can't open file \"%s%s.%s\"\n", DirPathName, actualFName, p->tag);
		return(FALSE);
	}
	if (fpout == stdout && FileName2[0] != EOS) {
		parsfname(FileName2,newfname,GExt);
		fpout = fopen(newfname,"w");
		if (fpout == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n", newfname);
			freeMem();
			fclose(fpin);
			fpin = tFp;
			fpout = stdout;
			cutt_exit(0);
		}
#ifdef _MAC_CODE
		settyp(newfname, 'TEXT', the_file_creator.out, FALSE);
#endif
	}

	areTiersFound = FALSE;
	ln = 0L;
    while (fgets_cr(templineC, UTTLINELEN, fpin)) {
    	ln++;
		if (uS.isUTF8(templineC))
			continue;
		uS.remblanks(templineC);
		if (uS.partwcmp(templineC, "#")) {
			areTiersFound = TRUE;
			break;
		}
	}

	if (!areTiersFound) {
		fclose(fpin);
		fpin = tFp;
		return(TRUE);
	}

	et = 0;
    while (fgets_cr(templineC, UTTLINELEN, fpin)) {
    	ln++;
		if (uS.isUTF8(templineC))
			continue;
		uS.remblanks(templineC);
		
		bt = et;
		for (bS=templineC; isSpace(*bS); bS++) ;
		if (*bS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s%s\": line %ld.\n", DirPathName, actualFName, ln);
			fprintf(stderr, "File is corrupted\n");
			fclose(fpin);
			fpin = tFp;
			cutt_exit(0);
		}
		for (eS=bS; !isSpace(*eS) && *eS != EOS; eS++) ;
		if (*eS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s%s\": line %ld.\n", DirPathName, actualFName, ln);
			fprintf(stderr, "File is corrupted\n");
			fclose(fpin);
			fpin = tFp;
			cutt_exit(0);
		}
		et = (long)(calculateOffset(bS) - timeOffset);

		for (; isSpace(*eS); eS++) ;
		for (; !isSpace(*eS) && *eS != EOS; eS++) ;
		for (; isSpace(*eS); eS++) ;
		
		if (*eS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s%s\": line %ld.\n", DirPathName, actualFName, ln);
			fprintf(stderr, "File is corrupted\n");
			fclose(fpin);
			fpin = tFp;
			cutt_exit(0);
		}

		line = eS;
		for (i=0; i < TBTScnt; i++) {
			if (TBTS[i][0] != EOS && uS.patmat(line, TBTS[i]))
				break;
		}
		if (i >= TBTScnt) {
			cleanupLine(line);
			AsciiToUnicodeToUTF8(line, templineC1);
			strcpy(line, templineC1);
			if (p->whichTier == SPKRTIER) {
				p->isUsed = TRUE;
				sLinesRoot = add_each_line(sLinesRoot, FALSE, p->whichTier, p->depOn, p->chatName, line, bt, et, tFp);
			} else if (p->whichTier == DEPTTIER) {
				p->isUsed = TRUE;
				dLinesRoot = add_each_line(dLinesRoot, FALSE, p->whichTier, p->depOn, p->chatName, line, bt, et, tFp);
			}
		}
    }
	fclose(fpin);
	fpin = tFp;
	return(TRUE);
}

void call() {
	char isSpeakerFound;
	char *s;
	TAGS *p;

	if (tiersRoot == NULL) {
		rd_dep_f("attribs.cut");
		if (tiersRoot == NULL) {
			fprintf(stderr,"Please specify attribs.cut file with +d option.\n");
			cutt_exit(0);
		}
	}
	if (whatKindTag(NULL, NULL, NULL) != SPKRTIER) {
	    fprintf(stderr,"Please specify the name of main speaker(s) in attribs.cut file.\n");
	    cutt_exit(0);
	}

	strcpy(DirPathName, oldfname);
	s = strrchr(DirPathName, PATHDELIMCHR);
	if (s != NULL) {
		strcpy(FileName1, s+1);
		s[1] = EOS;
	} else {
		DirPathName[0] = EOS;
		strcpy(FileName1, oldfname);
	}
	s = strchr(FileName1, '.');
	if (s != NULL)
		*s = EOS;
	strcpy(actualFName, FileName1);

	for (p=tiersRoot; p != NULL; p=p->nextTag)
		p->isUsed = FALSE;

	if (DirPathName[0] != EOS)
		SetNewVol(DirPathName);
	FileName2[0] = EOS;
	isSpeakerFound = FALSE;
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == SPKRTIER) {
			isSpeakerFound = TRUE;
			if (!processFiles(p) && !isRecursive)
				goto fin;
		}
	}
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == DEPTTIER) {
			if (!processFiles(p) && !isRecursive)
				goto fin;
		}
	}
    if (!isSpeakerFound) {
		fprintf(stderr, "Can't find speaker(s) specified in attribs.txt file anywhere in this file: %s%s\n", DirPathName, FileName1);
		freeMem();
		cutt_exit(0);
    }

	if (FileName2[0] != EOS) {
		fprintf(fpout, "@UTF8\n");
		fprintf(fpout, "@Begin\n");
		fprintf(fpout, "@Languages:	en\n");
		output_Participants_ID();
		if (MFN == NULL || *MFN == EOS) {
			fprintf(fpout, "%s\t%s\n", MEDIAHEADER, FileName2);
		} else
			fprintf(fpout, "%s\t%s\n", MEDIAHEADER, MFN);
		addDepsToSpeaker();
		outputTiers(sLinesRoot);
		fprintf(fpout, "@End\n");
		fprintf(stderr,"Done with file <%s>\n", newfname);
	}
fin:
    sLinesRoot = freeLines(sLinesRoot);
    dLinesRoot = freeLines(dLinesRoot);
	if (fpout != stdout && fpout != NULL && fpout != stderr)
		fclose(fpout);
	fpout = stdout;
	if (DirPathName[0] != EOS) {
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
		else
			SetNewVol(wd_dir);
	}
}
