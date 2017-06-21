/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#ifdef _WIN32 
#include "ced.h"
#endif
#include "cu.h"
#include "cutt-xml.h"
#ifdef _WIN32
	#include <TextUtils.h>
#endif

#if !defined(UNX)
#define _main anvil2chat_main
#define call anvil2chat_call
#define getflag anvil2chat_getflag
#define init anvil2chat_init
#define usage anvil2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char cutt_isMultiFound;

#define TAGSLEN  128

struct AnvilAtribsRec {
	char tag[TAGSLEN];
	char chatName[TAGSLEN];
	char depOn[TAGSLEN];
	struct AnvilAtribsRec *nextTag;
} ;
typedef struct AnvilAtribsRec ATTRIBS;

#define A2C_LINESEG struct A2C_LineBulletSegment
struct A2C_LineBulletSegment {
	long  anvilID;
	long  chatRefID;
	char *text;
	long beg, end;
	A2C_LINESEG *nextSeg;
} ;

#define A2C_CHATTIERS struct ChatTiers
struct ChatTiers {
	char isWrap;
	char isOverlapOrig;
	long beg, end;
	unsigned short wordCnt;
	char *sp;
	char *refSp;
	A2C_LINESEG *lineSeg;
	A2C_LINESEG *lastLineSeg;
	A2C_CHATTIERS *depTiers;
	A2C_CHATTIERS *nextTier;
} ;

static char mediaFName[FILENAME_MAX+2];
static char lang[TAGSLEN+1];
static char isMultiBullets;
static char lEncodeSet;
static unsigned short lEncode;
static A2C_CHATTIERS *RootTiers;
static Element *Anvil_stack[30];
static ATTRIBS *attsRoot;
#ifdef _MAC_CODE
static TextEncodingVariant lVariant;
#endif

void usage() {
	printf("convert Anvil XML files to CHAT files\n");
	printf("Usage: anvil2chat [b dF lS oS %s] filename(s)\n",mainflgs());
	puts("-b:  Specify that only one bullet per line (default multiple bullets per line).");
    puts("+dF: specify attribs/tags dependencies file F.");
    puts("+lS: specify language used by speakers.");
	puts("+oS: Specify code page. Please type \"+o?\" for full listing of codes");
	puts("     utf8  - Unicode UTF-8");
	puts("     macl  - Mac Latin (German, Spanish ...)");
	puts("     pcl   - PC  Latin (German, Spanish ...)");
	mainusage(FALSE);
	puts("\nExample: anvil2chat -danvil_attribs.cut -omacl filename");
	puts("\tanvil2chat -danvil_attribs.cut -opcl +ldk filename");
	cutt_exit(0);
}

static ATTRIBS *freeAttribs(ATTRIBS *p) {
	ATTRIBS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTag;
		free(t);
	}
	return(NULL);
}

static void freeLineSeg(A2C_LINESEG *p) {
	A2C_LINESEG *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextSeg;
		if (t->text != NULL)
			free(t->text);
		free(t);
	}
}

static A2C_CHATTIERS *freeTiers(A2C_CHATTIERS *p) {
	A2C_CHATTIERS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->depTiers != NULL)
			freeTiers(t->depTiers);
		if (t->sp != NULL)
			free(t->sp);
		if (t->refSp != NULL)
			free(t->refSp);
		if (t->lineSeg != NULL)
			freeLineSeg(t->lineSeg);
		free(t);
	}
	return(NULL);
}

static void freeAnvil2ChatMem(void) {
	attsRoot = freeAttribs(attsRoot);
	RootTiers = freeTiers(RootTiers);
}

void init(char s) {
	if (s) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		stout = FALSE;
		onlydata = 1;
		lEncodeSet = 0;
		lEncode = 0;
		strcpy(lang, "UNK");
		attsRoot = NULL;
		RootTiers = NULL;
		CurrentElem = NULL;
		isMultiBullets = TRUE;
#ifdef _MAC_CODE
		lVariant = kTextEncodingDefaultVariant;
#endif
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
}

static ATTRIBS *add_each_anvilTag(ATTRIBS *root, const char *fname, long ln, const char *tag, const char *chatName, const char *depOn) {
	long len;
	ATTRIBS *p, *tp;

	if (tag[0] == EOS)
		return(root);

	if (root == NULL) {
		if ((p=NEW(ATTRIBS)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->nextTag != NULL && uS.mStricmp(p->tag, tag); p=p->nextTag) ;
		if (uS.mStricmp(p->tag, tag) == 0)
			return(root);
		
		if ((p->nextTag=NEW(ATTRIBS)) == NULL)
			out_of_mem();
		p = p->nextTag;
	}

	p->nextTag = NULL;
	if ((strlen(tag) >= TAGSLEN) || (strlen(depOn) >= TAGSLEN) || (strlen(chatName) >= TAGSLEN)) {
		freeAnvil2ChatMem();
		fprintf(stderr,"*** File \"%s\"", fname);
		if (ln != 0)
			fprintf(stderr,": line %ld.\n", ln);
		fprintf(stderr, "Tag(s) too long. Longer than %d characters\n", TAGSLEN);
		cutt_exit(0);
	}
	strcpy(p->tag, tag);
	strcpy(p->chatName, chatName);
	if (p->chatName[0] == '@' || p->chatName[0] == '*' || p->chatName[0] == '%') {
		len = strlen(p->chatName) - 1;
		if (p->chatName[len] != ':')
			strcat(p->chatName, ":");
	}
	if (depOn[0] != EOS && chatName[0] != '*') {
		for (tp=root; tp != NULL; tp=tp->nextTag) {
			if (uS.mStricmp(tp->tag, depOn) == 0 || uS.partcmp(tp->chatName, depOn, FALSE, FALSE))
				break;
		}
		if (tp == NULL) {
			freeAnvil2ChatMem();
			fprintf(stderr,"*** File \"%s\"", fname);
			if (ln != 0)
				fprintf(stderr,": line %ld.\n", ln);
			fprintf(stderr, " ** Can't match anvil tag \"%s\" to any speaker declaration in attributes file.\n", depOn);
			cutt_exit(0);
		}
		strcpy(p->depOn, tp->chatName);
	} else
		strcpy(p->depOn, depOn);

	return(root);
}

static void rd_AnvilAtts_f(const char *fname, char isFatalError) {
	int  cnt;
	const char *tag, *depOn, *chatName;
	char isQF;
	long i, j, ln;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*fname == EOS) {
		fprintf(stderr,	"No dep. tags file specified.\n");
		cutt_exit(0);
	}
	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "\n    Warning: Not using any attribs file: \"%s\", \"%s\"\n", fname, mFileName);
		if (isFatalError)
			cutt_exit(0);
		else {
//			fprintf(stderr, "    Please specify attribs file with +d option.\n\n");
			return;
		}
	}
	ln = 0L;
	while (fgets_cr(templineC, 255, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		ln++;
		if (templineC[0] == ';')
			continue;
		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;
		i = 0;
		cnt = 0;
		tag = "";
		chatName = "";
		depOn = "";
		while (1) {
			isQF = 0;
			for (; isSpace(templineC[i]); i++) ;
			if (templineC[i] == '"' || templineC[i] == '\'') {
				isQF = templineC[i];
				i++;
			}
			for (j=i; (!isSpace(templineC[j]) || isQF) && templineC[j] != EOS; j++) {
				if (templineC[j] == isQF) {
					isQF = 0;
					strcpy(templineC+j, templineC+j+1);
					j--;
				}
			}
			if (cnt == 0)
				tag = templineC+i;
			else if (cnt == 1)
				chatName = templineC+i;
			else if (cnt == 2)
				depOn = templineC+i;
			if (templineC[j] == EOS)
				break;
			templineC[j] = EOS;
			i = j + 1;
			cnt++;
		}
		if (tag[0] == EOS || chatName[0] == EOS) {
			freeAnvil2ChatMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
			if (tag[0] == EOS)
				fprintf(stderr, "Missing Anvil tag\n");
			else if (chatName[0] == EOS)
				fprintf(stderr, "Missing Chat tier name for \"%s\"\n", tag);
			cutt_exit(0);
		}
		if (depOn[0] == EOS && chatName[0] == '%') {
			freeAnvil2ChatMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
			fprintf(stderr, "Missing \"Associated with tag\" for \"%s\"\n", tag);
			cutt_exit(0);
		}
		attsRoot = add_each_anvilTag(attsRoot, fname, ln, tag, chatName, depOn);
	}
	fclose(fp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = ANVIL2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	attsRoot = freeAttribs(attsRoot);
}

static void cleanup_lang(char *lang) {
	while (*lang) {
		if (isSpace(*lang) || *lang < (unCH)' ')
			strcpy(lang, lang+1);
		else
			lang++;
	}
	if (*(lang-1) == ',')
		strcpy(lang-1, lang);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'b':
			isMultiBullets = FALSE;
			no_arg_option(f);
			break;
		case 'd':
			if (*f) {
				rd_AnvilAtts_f(getfarg(f,f1,i), TRUE);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'l':
			if (*f) {
				strncpy(lang, getfarg(f,f1,i), TAGSLEN);
				lang[TAGSLEN] = EOS;
				cleanup_lang(lang);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'o':
			lEncodeSet = UTF8;
			
#ifdef _MAC_CODE
			if (!uS.mStricmp(f, "utf8"))
				lEncode = 0xFFFF;
			else if (!uS.mStricmp(f, "macl"))
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
				if (*f != '?')
					fprintf(stderr,"Unrecognized font option \"%s\". Please use:\n", f);
				puts("     utf8  - Unicode UTF-8");
				displayOoption();
				cutt_exit(0);
			}
#endif
#ifdef _WIN32 
			if (!uS.mStricmp(f, "utf8"))
				lEncode = 0xFFFF;
			else if (!uS.mStricmp(f, "pcl"))
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
				if (*f != '?')
					fprintf(stderr,"Unrecognized font option \"%s\". Please use:\n", f);
				puts("     utf8  - Unicode UTF-8");
				displayOoption();
				cutt_exit(0);
			}
#endif
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void Anvil_addLineSeg(A2C_CHATTIERS *nt, long beg, long end, const char *line, long anvilID, char isAddBullet) {
	long len;
	A2C_LINESEG *seg;
	
	if (nt->lineSeg == NULL) {
		if ((nt->lineSeg=NEW(A2C_LINESEG)) == NULL)
			out_of_mem();
		seg = nt->lineSeg;
		nt->lastLineSeg = seg;
	} else {
//		for (seg=nt->lineSeg; seg->nextSeg != NULL; seg=seg->nextSeg) ;
		seg = nt->lastLineSeg;
		if ((seg->nextSeg=NEW(A2C_LINESEG)) == NULL)
			out_of_mem();
		seg = seg->nextSeg;
		nt->lastLineSeg = seg;
	}
	seg->anvilID = anvilID;
	seg->chatRefID = 0L;
	seg->nextSeg = NULL;
	seg->beg = beg;
	seg->end = end;
	
	if (isAddBullet && (beg != 0L || end != 0L)) {
		len = strlen(line);
		if (isSpace(line[len-1]))
			sprintf(templineC3, "%c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
		else
			sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
	} else
		templineC3[0] = EOS;
	
	seg->text = (char *)malloc(strlen(line)+strlen(templineC3)+1);
	if (seg->text == NULL)
		out_of_mem();
	strcpy(seg->text, line);
	if (templineC3[0] != EOS)
		strcat(seg->text, templineC3);
}

static void Anvil_fillNT(A2C_CHATTIERS *nt, long beg, long end, const char *sp, const char *line, char *refSp, long anvilID, char isAddBullet) {
	nt->depTiers = NULL;
	nt->isWrap = TRUE;
	nt->sp = (char *)malloc(strlen(sp)+1);
	if (nt->sp == NULL) out_of_mem();
	strcpy(nt->sp, sp);
	nt->refSp = (char *)malloc(strlen(refSp)+1);
	if (nt->refSp == NULL) out_of_mem();
	strcpy(nt->refSp, refSp);
	nt->lineSeg = NULL;
	Anvil_addLineSeg(nt, beg, end, line, anvilID, isAddBullet);
	nt->beg = beg;
	nt->end = end;
}

static A2C_CHATTIERS *Anvil_addDepTiers(A2C_CHATTIERS *depTiers, long beg, long end, char *sp, char *line, char *refSp, long anvilID, char isAddBullet) {
	A2C_CHATTIERS *nt, *tnt;
	
	if (depTiers == NULL) {
		if ((depTiers=NEW(A2C_CHATTIERS)) == NULL)
			out_of_mem();
		nt = depTiers;
		nt->nextTier = NULL;
	} else {
		tnt= depTiers;
		nt = depTiers;
		while (nt != NULL) {
			if (!strcmp(nt->sp, sp)) {
				if (end > nt->end)
					nt->end = end;
				if (beg < nt->beg)
					nt->beg = beg;
				Anvil_addLineSeg(nt, beg, end, line, anvilID, isAddBullet);
				return(depTiers);
			}
			if (beg < nt->beg)
				break;
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			tnt->nextTier = NEW(A2C_CHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
		} else if (nt == depTiers) {
			depTiers = NEW(A2C_CHATTIERS);
			if (depTiers == NULL)
				out_of_mem();
			depTiers->nextTier = nt;
			nt = depTiers;
		} else {
			nt = NEW(A2C_CHATTIERS);
			if (nt == NULL)
				out_of_mem();
			nt->nextTier = tnt->nextTier;
			tnt->nextTier = nt;
		}
	}
	
	Anvil_fillNT(nt, beg, end, sp, line, refSp, anvilID, isAddBullet);
	return(depTiers);
}

static A2C_CHATTIERS *Anvil_insertNT(A2C_CHATTIERS *nt, A2C_CHATTIERS *tnt) {
	if (nt == RootTiers) {
		RootTiers = NEW(A2C_CHATTIERS);
		if (RootTiers == NULL)
			out_of_mem();
		RootTiers->nextTier = nt;
		nt = RootTiers;
	} else {
		nt = NEW(A2C_CHATTIERS);
		if (nt == NULL)
			out_of_mem();
		nt->nextTier = tnt->nextTier;
		tnt->nextTier = nt;
	}
	return(nt);
}

static char Anvil_isUttDel(A2C_LINESEG *seg) {
	char bullet;
	int i;

	for (; seg != NULL; seg=seg->nextSeg) {
		i = 0;
		if (seg->text[i] == HIDEN_C)
			bullet = TRUE;
		else
			bullet = FALSE;
		for (; seg->text[i] != EOS; i++) {
			if ((uS.IsUtteranceDel(seg->text, i) || uS.IsCAUtteranceDel(seg->text, i)) && !bullet)
				return(TRUE);
			if (seg->text[i] == HIDEN_C)
				bullet = !bullet;
		}
	}
	return(FALSE);
}

static char Anvil_isPostCodes(char *line) {
	char bullet;
	int  sq;
	long i;

	sq = 0;
	bullet = FALSE;
	i = strlen(line) - 1;
	for (; i >= 0L && (isSpace(line[i]) || isdigit(line[i]) || line[i] == '#' || line[i] == '_' || bullet || sq); i--) {
		if (line[i] == HIDEN_C)
			bullet = !bullet;
		else if (line[i] == ']')
			sq++;
		else if (line[i] == '[') {
			sq--;
			if (line[i+1] != '+')
				return(FALSE);
		}
	}
	if (i < 0L)
		return(TRUE);
	else
		return(FALSE);
}

static char isFoundBegOverlap(char *line) {
	for (; *line != EOS; line++) {
		if (UTF8_IS_LEAD((unsigned char)*line) && *line == (char)0xe2 && *(line+1) == (char)0x8c) {
			if (*(line+2) == (char)0x88) // raised [
				return(TRUE);
			if (*(line+2) == (char)0x8a) // lowered [
				break;
		}
	}
	
	return(FALSE);
}

static char Anvil_isLineJustPause(char *line) {
	int i;

	for (i=0; isSpace(line[i]); i++) ;
	if (uS.isRightChar(line,i,'(',&dFnt,MBF) && uS.isPause(line,i,NULL,&i)) {
		if (uS.isRightChar(line,i,')',&dFnt,MBF)) {
			for (i++; isSpace(line[i]); i++) ;
			if (line[i] == EOS || line[i] == '\n')
				return(TRUE);
		}
	}
	return(FALSE);
}

static void Anvil_addToTiers(long beg, long end, char *sp, char *refSp, char *line, long anvilID, char isAddBullet) {
	char isPostCodeFound, isJustPause;
	A2C_CHATTIERS *nt, *tnt;

	if (RootTiers == NULL) {
		if ((RootTiers=NEW(A2C_CHATTIERS)) == NULL)
			out_of_mem();
		nt = RootTiers;
		nt->nextTier = NULL;
	} else {
		isJustPause = Anvil_isLineJustPause(line);
		isPostCodeFound = Anvil_isPostCodes(line);
		tnt= RootTiers;
		nt = RootTiers;
		while (nt != NULL) {
			if (sp[0] == '%') {
				if (beg >= nt->beg && beg < nt->end && uS.partcmp(refSp, nt->sp, FALSE, TRUE)) {
					nt->depTiers = Anvil_addDepTiers(nt->depTiers, beg, end, sp, line, refSp, anvilID, isAddBullet);
					return;
				} else if (end-beg < 3 && beg >= nt->beg && beg <= nt->end && uS.partcmp(refSp, nt->sp, FALSE, TRUE)) {
					nt->depTiers = Anvil_addDepTiers(nt->depTiers, beg, end, sp, line, refSp, anvilID, isAddBullet);
					return;
				} else if (beg < nt->beg) {
					if (end > nt->beg)
						end = nt->beg;
					nt = Anvil_insertNT(nt, tnt);
					Anvil_fillNT(nt, beg, end, refSp, "0.", refSp, -1L, isAddBullet);
					nt->depTiers = Anvil_addDepTiers(nt->depTiers, beg, end, sp, line, refSp, anvilID, isAddBullet);
					return;
				}
			} else if (sp[0] == '*') {
				if (beg>=nt->end && beg<=nt->end+50 && !strcmp(sp,nt->sp) && (!Anvil_isUttDel(nt->lineSeg) || isJustPause)) {
					nt->end = end;
					Anvil_addLineSeg(nt, beg, end, line, anvilID, isAddBullet);
					return;
				} else if (beg == nt->end && !strcmp(sp, nt->sp) && isPostCodeFound) {
					nt->end = end;
					Anvil_addLineSeg(nt, beg, end, line, anvilID, isAddBullet);
					return;
				} else if (beg == nt->beg && isFoundBegOverlap(line)) {
					break;
				} else if (beg < nt->beg)
					break;
			}
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			tnt->nextTier = NEW(A2C_CHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
			if (sp[0] == '%') {
				Anvil_fillNT(nt, beg, end, refSp, "0.", refSp, -1L, isAddBullet);
				nt->depTiers = Anvil_addDepTiers(nt->depTiers, beg, end, sp, line, refSp, anvilID, isAddBullet);
				return;
			}
		} else 
			nt = Anvil_insertNT(nt, tnt);
	}
	
	Anvil_fillNT(nt, beg, end, sp, line, refSp, anvilID, isAddBullet);
}

static char Anvil_isOverlapFound(A2C_LINESEG *seg, char ovChar) {
	long i;
	
	for (; seg != NULL; seg=seg->nextSeg) {
		for (i=0L; seg->text[i] != EOS; i++) {
			if (seg->text[i] == (char)0xe2 && seg->text[i+1] == (char)0x8c && seg->text[i+2] == (char)ovChar)
				return(TRUE);
		}
	}
	return(FALSE);
}


static void Anvil_finalTimeSort(void) {
	char isWrongOverlap;
	A2C_CHATTIERS *nt, *prev_nt, *prev_prev_nt;

	nt = RootTiers;
	prev_nt = nt;
	prev_prev_nt = nt;
	while (nt != NULL) {
		if (prev_nt != nt) {
			if (nt->beg == prev_nt->beg && nt->end < prev_nt->end) {
				prev_nt->nextTier = nt->nextTier;
				nt->nextTier = prev_nt;
				if (prev_prev_nt == RootTiers) {
					RootTiers = nt;
					prev_prev_nt = RootTiers;
				} else
					prev_prev_nt->nextTier = nt;
				prev_nt = prev_prev_nt;
				nt = prev_nt->nextTier;
			} else if (nt->beg == prev_nt->beg && nt->end == prev_nt->end) {
				isWrongOverlap = 0;
				if (Anvil_isOverlapFound(nt->lineSeg, (char)0x88) && !Anvil_isOverlapFound(nt->lineSeg, (char)0x8a))
					isWrongOverlap++;
				if (isWrongOverlap == 1) {
					if (Anvil_isOverlapFound(prev_nt->lineSeg, (char)0x8a) && !Anvil_isOverlapFound(prev_nt->lineSeg, (char)0x88))
						isWrongOverlap++;
				}
				if (isWrongOverlap == 2) {
					prev_nt->nextTier = nt->nextTier;
					nt->nextTier = prev_nt;
					if (prev_prev_nt == RootTiers) {
						RootTiers = nt;
						prev_prev_nt = RootTiers;
					} else
						prev_prev_nt->nextTier = nt;
					prev_nt = prev_prev_nt;
					nt = prev_nt->nextTier;
					if (nt == NULL)
						break;
				}
			}
		}
		prev_prev_nt = prev_nt;
		prev_nt = nt;
		nt = nt->nextTier;
	}
}

static char addRefCodeToTier(A2C_CHATTIERS *nt, long *refID, char *sp, char *refSp, long index) {
	A2C_LINESEG *seg;
	char bullet;
	long i, j;
	
	while (nt != NULL) {
		if (refSp == NULL || uS.partcmp(nt->refSp, refSp, FALSE, FALSE)) {
			if (uS.partcmp(nt->sp, sp, FALSE, FALSE)) {
				for (seg=nt->lineSeg; seg != NULL; seg=seg->nextSeg) {
					if (seg->anvilID == index) {
						bullet = FALSE;
						for (i=strlen(seg->text)-1; i >= 0L; i--) {
							if (seg->text[i] == HIDEN_C)
								bullet = !bullet;
							else if (!isSpace(seg->text[i]) && seg->text[i] != '\n' && !bullet)
								break;
						}
						i++;
						if (seg->chatRefID == 0L) {
							(*refID)++;
							sprintf(templineC1, "[r %ld]", *refID);
							seg->chatRefID = *refID;
							strcpy(templineC2, seg->text);
							uS.shiftright(templineC2+i, strlen(templineC1)+1);
							templineC2[i++] = ' ';
							for (j=0; templineC1[j] != EOS; j++)
								templineC2[i++] = templineC1[j];
							if (strlen(templineC2) > strlen(seg->text)) {
								free(seg->text);
								seg->text = (char *)malloc(strlen(templineC2)+1);
								if (seg->text == NULL)
									out_of_mem();
							}
							strcpy(seg->text, templineC2);
						}
						sprintf(templineC1, "r-%ld", seg->chatRefID);
						return(0);
					}
				}
			}
			if (nt->depTiers != NULL) {
				if (addRefCodeToTier(nt->depTiers, refID, sp, refSp, index) == 0)
					return(0);
			}
		}
		nt = nt->nextTier;
	}
	return(1);
}

static long addReferences(A2C_CHATTIERS *nt, long refID) {
	long i, index_val;
	long st, tst, code;
	char *refSp, t;
	A2C_LINESEG *seg;

	while (nt != NULL) {
		for (seg=nt->lineSeg; seg != NULL; seg=seg->nextSeg) {
			for (st=0L; seg->text[st] != EOS; st++) {
				if (seg->text[st] == '\001' || seg->text[st] == '\002') {
					templineC3[0] = EOS;
					index_val = -1L;
					tst = st;
					if (seg->text[tst] == '\001') {
						tst++;
						code = tst;
						for (; seg->text[tst] != '\001' && seg->text[tst] != EOS && seg->text[tst] != '\002'; tst++) ;
						if (seg->text[tst] == '\002')
							continue;
						else if (seg->text[tst] == EOS)
							break;
						t = seg->text[tst];
						seg->text[tst] = EOS;
						strcpy(templineC3, seg->text+code);
						seg->text[tst] = t;
					} else {
						tst++;
						code = tst;
						for (; seg->text[tst] != '\002' && seg->text[tst] != EOS && seg->text[tst] != '\001'; tst++) ;
						if (seg->text[tst] == '\001')
							continue;
						else if (seg->text[tst] == EOS)
							break;
						t = seg->text[tst];
						seg->text[tst] = EOS;
						index_val = atol(seg->text+code);
						seg->text[tst] = t;
					}
					if (seg->text[tst] != EOS)
						tst++;
					if (seg->text[tst] == '\001') {
						tst++;
						code = tst;
						for (; seg->text[tst] != '\001' && seg->text[tst] != EOS && seg->text[tst] != '\002'; tst++) ;
						if (seg->text[tst] == '\002')
							continue;
						else if (seg->text[tst] == EOS)
							break;
						t = seg->text[tst];
						seg->text[tst] = EOS;
						strcpy(templineC3, seg->text+code);
						seg->text[tst] = t;
					} else {
						tst++;
						code = tst;
						for (; seg->text[tst] != '\002' && seg->text[tst] != EOS && seg->text[tst] != '\001'; tst++) ;
						if (seg->text[tst] == '\001')
							continue;
						else if (seg->text[tst] == EOS)
							break;
						t = seg->text[tst];
						seg->text[tst] = EOS;
						index_val = atol(seg->text+code);
						seg->text[tst] = t;
					}
					if (seg->text[tst] != EOS)
						tst++;
					if (templineC3[0] != EOS && index_val >= 0L) {
						strcpy(seg->text+st, seg->text+tst);
						if ((refSp=strchr(templineC3, '@')) != NULL) {
							*refSp = EOS;
							refSp++;
						}
						sprintf(templineC1, "no_match_%ld", refID);
						addRefCodeToTier(RootTiers, &refID, templineC3, refSp, index_val);
						if (refSp != NULL) {
							refSp--;
							*refSp = '@';
						}
						strcpy(templineC2, seg->text);
						uS.shiftright(templineC2+st, strlen(templineC1));
						for (i=0; templineC1[i] != EOS; i++)
							templineC2[st+i] = templineC1[i];
						if (strlen(templineC2) > strlen(seg->text)) {
							free(seg->text);
							seg->text = (char *)malloc(strlen(templineC2)+1);
							if (seg->text == NULL)
								out_of_mem();
						}
						strcpy(seg->text, templineC2);
					}
				}
			}
		}
		if (nt->depTiers != NULL)
			refID = addReferences(nt->depTiers, refID);
		nt = nt->nextTier;
	}
	return(refID);
}

static void Anvil_createLineFromSegs(char *line, A2C_CHATTIERS *nt, char whichLevel) {
//	char s[512];
	long i, lastEnd;
	A2C_LINESEG *seg;

	if (isMultiBullets || nt->sp[0] == '%' || nt->sp[0] == '@') {
		nt->isWrap = TRUE;
	} else {
		nt->isWrap = FALSE;
	}
	seg = nt->lineSeg;
	if (seg != NULL)
		lastEnd = seg->end;
	else
		lastEnd = 0L;
	line[0] = EOS;
	for (; seg != NULL; seg=seg->nextSeg) {

//sprintf(s, "<%ld> ", seg->anvilID);
//strcat(line, s);

		if (whichLevel == 0) {
			if (lastEnd <= seg->beg-50)
				strcat(line, "# ");
			strcat(line, seg->text);
		} else
			strcat(line, seg->text);
		
		if (whichLevel != 0)
			strcat(line, " ");
		else if (isMultiBullets)
			strcat(line, " ");
		else
			strcat(line, "\n\t");
		
		lastEnd = seg->end;
		if (strlen(line) > UTTLINELEN-50) {
			fprintf(stderr, "Out of memory!!! Tier is too long.\n");
			fprintf(stderr, "Line:\n");
			for (i=0L; line[i] != EOS; i++) {
				putc(line[i], stderr);
			}
			putc('\n', stderr);
			RootTiers = freeTiers(RootTiers);
			cutt_exit(0);
		}
	}
	i = ::strlen(line) - 1;
	while (i >= 0 && (isSpace(line[i]) || line[i] == '\n' || line[i] == ',')) i--;
	line[i+1] = EOS;
}

static void Anvil_printOutTiers(A2C_CHATTIERS *nt, char whichLevel) {
	while (nt != NULL) {
		Anvil_createLineFromSegs(templineC3, nt, whichLevel);
		printout(nt->sp, templineC3, NULL, NULL, nt->isWrap);
		if (nt->depTiers != NULL)
			Anvil_printOutTiers(nt->depTiers, 1);
		nt = nt->nextTier;
	}
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
		fprintf(stderr, "Fatal error1: Unable to create a converter.\n");
		fprintf(stderr, "%s\n", src);
		freeXML_Elements();
		freeAnvil2ChatMem();
		cutt_exit(0);
	}
	
	len = strlen(src);
	if ((err=TECConvertText(ec, (ConstTextPtr)src, len, &ail, (TextPtr)line, UTTLINELEN, &aol)) != noErr) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
		fprintf(stderr, "%s\n",src);
		freeXML_Elements();
		freeAnvil2ChatMem();
		cutt_exit(0);
	}
	err = TECDisposeConverter(ec);
	if (ail < len) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname);
		fprintf(stderr, "Fatal error3: Converted only %ld out of %ld chars:\n", ail, len);
		fprintf(stderr, "%s\n", src);
		freeXML_Elements();
		freeAnvil2ChatMem();
		cutt_exit(0);
	}
	line[aol] = EOS;
}
#endif

static void Anvil_makeText(char *line) {
	char *e;
	unsigned int c;

#if defined(_MAC_CODE) || defined(_WIN32)
	if (lEncode != 0xFFFF) {
		AsciiToUnicodeToUTF8(line, templineC1);
		strcpy(line, templineC1);
	}
#endif
	while (*line != EOS) {
		if (!strncmp(line, "&amp;", 5)) {
			*line = '&';
			line++;
			strcpy(line, line+4);
		} else if (!strncmp(line, "&quot;", 6)) {
			*line = '"';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&apos;", 6)) {
			*line = '\'';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&lt;", 4)) {
			*line = '<';
			line++;
			strcpy(line, line+3);
		} else if (!strncmp(line, "&gt;", 4)) {
			*line = '>';
			line++;
			strcpy(line, line+3);
		} else if (*line == '{' && *(line+1) == '0' && *(line+2) == 'x') {
			for (e=line; *e != EOS && *e != '}'; e++) ;
			if (*e == '}') {
				sscanf(line+1, "%x", &c);
				*line = (char)c;
				line++;
				strcpy(line, e+1);
			} else
				line++;
		} else
			line++;
	}
}

/* Anvil-XML Begin **************************************************************** */
static void convertAnvilTag2Chat(char *chatName, char *tag, char *refSp, const char *type) {
	char *s, errFound;
	ATTRIBS *p;

	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (uS.mStricmp(p->tag, tag) == 0)
			break;
	}
	if (p == NULL) {
		errFound = FALSE;
		s = strrchr(tag, '.');
		if (tag[0] == '@') {
			strcpy(chatName, tag);
			if (refSp != NULL)
				strcpy(refSp, tag);
		} else if (uS.mStricmp(tag, "CHATtoken") == 0) {
			strcpy(chatName, "");
			if (refSp != NULL)
				strcpy(refSp, "");
		} else if (s == NULL)
			errFound = TRUE;
		else {
			*s = EOS;
			if (tag[0] == '*' && *(s+1) == '%') {
				strcpy(chatName, s+1);
				if (refSp != NULL)
					strcpy(refSp, tag);
			} else if (tag[0] == '*' && uS.mStricmp(s+1, "words") == 0) {
				strcpy(chatName, tag);
				if (refSp != NULL)
					strcpy(refSp, tag);
			} else
				errFound = TRUE;
			*s = '.';
		}
		if (errFound) {
			if (attsRoot != NULL) {
				fprintf(stderr, "\r ** Can't match anvil tag \"%s\" to CHAT %s declaration in attributes file.\n", tag, type);
				if (refSp != NULL) {
					freeXML_Elements();
					freeAnvil2ChatMem();
					cutt_exit(0);
				} else {
					chatName[0] = '$';
					strncpy(chatName+1, tag, SPEAKERLEN-3);
					chatName[SPEAKERLEN-3] = EOS;
					strcat(chatName, "_");
					attsRoot = add_each_anvilTag(attsRoot, oldfname, 0, tag, chatName, "");
				}
			} else {
				fprintf(stderr, "\r ** Can't match anvil tag \"%s\" to CHAT %s declaration in attributes file.\n", tag, type);
				fprintf(stderr, "Please create and specify attribs file with +d option.\n\n");
				freeXML_Elements();
				freeAnvil2ChatMem();
				cutt_exit(0);
			}
		}
	} else {
		strcpy(chatName, p->chatName);
		if (refSp != NULL) {
			if (p->depOn[0] == '*')
				strcpy(refSp, p->depOn);
			else
				strcpy(refSp, p->chatName);
		}
	}
}

static void remSpaces(char *s) {
/*
	while (*s != EOS) {
		if (*s == '\t' || *s == ' ')
			strcpy(s, s+1);
		else
			s++;
	}
*/
}

static void sortAnvilSpAndDepTiers(void) {
	int  lStackIndex;
	Attributes *att;
	Element *nt, *tnt, *firstTier;

	if (CurrentElem == NULL)
		return;
	lStackIndex = -1;
	nt = CurrentElem;
	tnt = nt;
	do {
		if (!strcmp(nt->name, "annotation") && nt->next == NULL && lStackIndex == -1) {
			nt = nt->data;
			lStackIndex++;
			if (nt == NULL)
				return;
		} else if (!strcmp(nt->name, "body") && lStackIndex == 0) {
			lStackIndex++;
			firstTier = nt;
			nt = nt->data;
			if (nt == NULL)
				return;
		} else if (!strcmp(nt->name, "track") && lStackIndex == 1) {
			for (att=nt->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "name")) {
					remSpaces(att->value);
					convertAnvilTag2Chat(templineC1, att->value, NULL, "Dep. tier");
					if (templineC1[0] == '*' && firstTier->data != nt) {
						tnt->next = nt->next;
						nt->next = firstTier->data;
						firstTier->data = nt;
						nt = tnt;
					}
					break;
				}
			}
		}
		tnt = nt;
		nt = nt->next;
	} while (nt != NULL);
}

static char getAnvilAttribute(Element *cElem, UTTER *utt) {
	char isValueFound, *s;
	char refSp[SPEAKERLEN];
	long len, len2;
	Element *data;
	Attributes *att;

	len = UTTLINELEN-3;
	utt->line[0] = EOS;
	do {
		if (cElem == NULL)
			return(FALSE);
		if (!strcmp(cElem->name, "attribute")) {
			isValueFound = TRUE;
			if (utt->speaker[0] != '*') {
				for (att=cElem->atts; att != NULL; att=att->next) {
					if (!strcmp(att->name, "name")) {
						remSpaces(att->value);
						convertAnvilTag2Chat(templineC1, att->value, NULL, "code");
						strncat(utt->line, templineC1, len);
						utt->line[UTTLINELEN-1] = EOS;
						len = UTTLINELEN - strlen(utt->line);
						isValueFound = FALSE;
					}
				}
			}
			if (cElem->cData != NULL) {
				len = UTTLINELEN - strlen(utt->line) - 1;
				strncat(utt->line, cElem->cData, len);
				strcat(utt->line, " ");
				utt->line[UTTLINELEN-1] = EOS;
				isValueFound = TRUE;
			} else if (cElem->data != NULL) {
				for (data=cElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line) - 1;
						strncat(utt->line, data->cData, len);
						strcat(utt->line, " ");
						utt->line[UTTLINELEN-1] = EOS;
						isValueFound = TRUE;
					} else if (!strcmp(data->name, "value-link")) {
						for (att=data->atts; att != NULL; att=att->next) {
							if (!strcmp(att->name, "ref-track")) {
								convertAnvilTag2Chat(templineC1, att->value, refSp, "code");
								if (templineC1[0] == '%') {
									strcat(templineC1, "@");
									strcat(templineC1, refSp);
								}
								len2 = strlen(utt->line);
								if (len2 > 0 && utt->line[len2-1] == ' ')
									utt->line[len2-1] = '_';
								utt->line[len2++] = '\001';
								utt->line[len2] = EOS;
								strncat(utt->line, templineC1, len-1);
								utt->line[UTTLINELEN-1] = EOS;
								len2 = strlen(utt->line);
								utt->line[len2++] = '\001';
								utt->line[len2] = EOS;
								len = UTTLINELEN - strlen(utt->line);
							} else if (!strcmp(att->name, "ref-index")) {
								len2 = strlen(utt->line);
								if (len2 > 0 && utt->line[len2-1] == ' ')
									utt->line[len2-1] = '_';
								utt->line[len2++] = '\002';
								utt->line[len2] = EOS;
								strncat(utt->line, att->value, len-1);
								utt->line[UTTLINELEN-1] = EOS;
								len2 = strlen(utt->line);
								utt->line[len2++] = '\002';
								utt->line[len2] = EOS;
								len = UTTLINELEN - strlen(utt->line);
							}
						}
						strcat(utt->line, " ");
						isValueFound = TRUE;
					}
				}
			}
			if (!isValueFound)
				strcat(utt->line, " ");
		} else if (!strcmp(cElem->name, "comment")) {
			if (cElem->cData != NULL) {
				len = UTTLINELEN - strlen(utt->line) - 6;
				strcat(utt->line, "$COM_");
				for (s=cElem->cData; *s != EOS; s++) {
					if (isSpace(*s))
						*s = '_';
				}
				strncat(utt->line, cElem->cData, len);
				utt->line[UTTLINELEN-1] = EOS;
			} else if (cElem->data != NULL) {
				for (data=cElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line) - 6;
						strcat(utt->line, "$COM_");
						for (s=data->cData; *s != EOS; s++) {
							if (isSpace(*s) || *s == '\n')
								*s = '_';
						}
						strncat(utt->line, data->cData, len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
		}
		cElem = cElem->next;
	} while (1);
	return(TRUE);
}

static char getNextAnvilTier(UTTER *utt, long *beg, long *end, char *refSp, long *anvilID) {
	double t;
	Attributes *att;

	if (CurrentElem == NULL)
		return(FALSE);

	do {
		if (CurrentElem != NULL && !strcmp(CurrentElem->name, "annotation") && CurrentElem->next == NULL)
			CurrentElem = CurrentElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Anvil_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}

		if (!strcmp(CurrentElem->name, "body") && CurrentElem->data != NULL && stackIndex == -1) {
			if (stackIndex < 29) {
				stackIndex++;
				Anvil_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freeAnvil2ChatMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "track") && CurrentElem->data != NULL && stackIndex == 0) {
			if (stackIndex < 29) {
				utt->speaker[0] = EOS;
				for (att=CurrentElem->atts; att != NULL; att=att->next) {
					if (!strcmp(att->name, "name")) {
						remSpaces(att->value);
						convertAnvilTag2Chat(utt->speaker, att->value, refSp, "Dep. tier");
						break;
					}
				}
				stackIndex++;
				Anvil_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freeAnvil2ChatMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "el") && stackIndex == 1) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "index")) {
					*anvilID = atol(att->value);
				} else if (!strcmp(att->name, "start")) {
					t = atof(att->value);
					t = t * 1000.0000;
					*beg = (long)t;
				} else if (!strcmp(att->name, "end")) {
					t = atof(att->value);
					t = t * 1000.0000;
					*end = (long)t;
				}
			}
			getAnvilAttribute(CurrentElem->data, utt);
			return(TRUE);
		}
	} while (1);
	return(FALSE);
}

static char getNextAnvilHeader(UTTER *utt, char *mediaFName) {
	long len;
	Element *data;
	Attributes *att;

	if (CurrentElem == NULL)
		return(FALSE);
	
	if (!strcmp(CurrentElem->name, "?xml") && stackIndex == -1) {
		for (att=CurrentElem->atts; att != NULL; att=att->next) {
			if (!strcmp(att->name, "encoding")) {
				strcpy(utt->speaker, CurrentElem->name);
				strcpy(utt->line, att->value);
				if (utt->line[0] == '"')
					strcpy(utt->line, utt->line+1);
				len = strlen(utt->line) - 1;
				if (utt->line[len] == '?')
					len--;
				if (utt->line[len] == '"')
					len--;
				utt->line[len+1] = EOS;
				CurrentElem = CurrentElem->next;
				return(TRUE);
			}
		}
	}

	do {
		if (CurrentElem != NULL && !strcmp(CurrentElem->name, "annotation") && CurrentElem->next == NULL)
			CurrentElem = CurrentElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Anvil_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}

		if (!strcmp(CurrentElem->name, "head") && CurrentElem->data != NULL && stackIndex == -1) {
			if (stackIndex < 29) {
				stackIndex++;
				Anvil_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freeAnvil2ChatMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "body") && stackIndex == -1) {
			break;
		}
		if (!strcmp(CurrentElem->name, "video") && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "src")) {
					strncpy(mediaFName, att->value, FILENAME_MAX);
					mediaFName[FILENAME_MAX] = EOS;
				}
			}
		}
		if (!strcmp(CurrentElem->name, "specification") && stackIndex == 0) {
			utt->speaker[0] = EOS;
			utt->line[0] = EOS;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "src")) {
					strcpy(utt->speaker, "@Comment:");
					strcpy(utt->line, "$SPECS_");
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, att->value, len);
					utt->line[UTTLINELEN-1] = EOS;
					return(TRUE);
				}
			}
		}
		if (!strcmp(CurrentElem->name, "info") && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "key")) {
					convertAnvilTag2Chat(utt->speaker, att->value, NULL, "Header tier");
					break;
				}
			}
			len = UTTLINELEN-3;
			utt->line[0] = EOS;
			if (CurrentElem->cData != NULL) {
				strncat(utt->line, CurrentElem->cData, len);
				utt->line[UTTLINELEN-1] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data=CurrentElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, data->cData, len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
			return(TRUE);
		}
	} while (1);
	return(FALSE);
}
/* Anvil-XML End ****************************************************************** */

static void printHeaders(char isBeginPrinted, char isLanguageFound, char isParticipFound, char isIDFound) {
	int i;
	char sp[SPEAKERLEN];
	ATTRIBS *p;

	if (!isBeginPrinted) {
		fprintf(fpout, "%s\n", UTF8HEADER);
		fprintf(fpout, "@Begin\n");
	}
	if (!isLanguageFound)
		fprintf(fpout, "@Languages:\t%s\n", lang);
	templineC1[0] = EOS;
	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (p->chatName[0] == '*') {
			strcat(templineC1, ", ");
			strcpy(sp, p->chatName+1);
			for (i=strlen(sp)-1; i >= 0 && (isSpace(sp[i]) || sp[i] == ':'); i--) ;
			sp[++i] = EOS;
			strcat(templineC1, sp);
			strcat(templineC1, " ");
			strcat(templineC1, p->depOn);
		}
	}
	for (i=0; templineC1[i] == ',' || isSpace(templineC1[i]); i++) ;
	if (!isParticipFound)
		printout("@Participants:", templineC1+i, NULL, NULL, TRUE);
	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (p->chatName[0] == '*') {
			strcpy(sp, p->chatName+1);
			for (i=strlen(sp)-1; i >= 0 && (isSpace(sp[i]) || sp[i] == ':'); i--) ;
			sp[++i] = EOS;
			if (!isIDFound)
				fprintf(fpout, "@ID:\t%s|change_me_later|%s|||||%s|||\n", lang, sp, p->depOn);
		}
	}
}

void call() {		/* this function is self-explanatory */
	const char *mediaType;
	char refSp[SPEAKERLEN], *p, isFirstHeaderFound;
	char isBeginPrinted, isLanguageFound, isParticipFound, isIDFound, isOptionFound;
	unsigned short tlEncode;
	long len;
	long beg, end;
	long ln = 0L, tln = 0L;
	long anvilID;
	A2C_CHATTIERS *lastSpTier;

	if (attsRoot == NULL) {
		rd_AnvilAtts_f("attribs.cut", FALSE);
	}

	tlEncode = lEncode;
	mediaFName[0] = EOS;
	BuildXMLTree(fpin);

	isBeginPrinted = FALSE;
	isLanguageFound = FALSE;
	isParticipFound = FALSE;
	isIDFound = FALSE;
	isOptionFound = FALSE;
	isFirstHeaderFound = FALSE;
	while (getNextAnvilHeader(utterance, mediaFName)) {
		uS.remblanks(utterance->speaker);
		if (!strcmp(utterance->speaker, "?xml")) {
			uS.remFrontAndBackBlanks(utterance->line);
			if (!uS.mStricmp(utterance->line, "utf-8"))
				lEncode = 0xFFFF;
#if defined(_MAC_CODE)
			else if (!uS.mStricmp(utterance->line, "ISO-8859-1"))
				lEncode = kTextEncodingWindowsLatin1;
#endif
#if defined(_WIN32)
			else if (!uS.mStricmp(utterance->line, "ISO-8859-1"))
				lEncode = 1252;
#endif
			continue;
		}
		len = strlen(utterance->speaker) - 1;
		if (utterance->line[0] != EOS) {
			if (utterance->speaker[len] != ':')
				strcat(utterance->speaker, ":");
		} else {
			if (utterance->speaker[len] == ':')
				utterance->speaker[len] = EOS;
		}
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE))
			continue;
		else if (uS.partcmp(utterance->speaker, "@Languages:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			isLanguageFound = TRUE;
			continue;
		} else if (uS.partcmp(utterance->speaker, "@Participants:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			isParticipFound = TRUE;
			continue;
		} else if (uS.partcmp(utterance->speaker, "@ID:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			isIDFound = TRUE;
			continue;
		} else if (uS.partcmp(utterance->speaker, "@Options:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			uS.remblanks(utterance->line);
			if (isMultiBullets) {
				for (beg=0; utterance->line[beg] != EOS; beg++) {
					if (!strncmp(utterance->line+beg, "multi", 5))
						break;
				}
				if (utterance->line[beg] == EOS)
					strcat(utterance->line, ", multi");
			}
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			isOptionFound = TRUE;
			continue;
		} else if (!isFirstHeaderFound && uS.partcmp(utterance->speaker, FONTHEADER, FALSE, FALSE)) {
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			continue;
		} else {
			if (!isFirstHeaderFound)
				printHeaders(isBeginPrinted, isLanguageFound, isParticipFound, isIDFound);
			Anvil_makeText(utterance->line);
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
		}
		isFirstHeaderFound = TRUE;
	}

	if (lEncodeSet == 0 && lEncode == 0) {
		fprintf(stderr, "Please specify text encoding with +o option.\n");
		freeAnvil2ChatMem();
		cutt_exit(0);
	}

	if (!isFirstHeaderFound)
		printHeaders(isBeginPrinted, isLanguageFound, isParticipFound, isIDFound);
	p = strrchr(mediaFName, '/');
	if (p != NULL)
		strcpy(mediaFName, p+1);
	p = strrchr(mediaFName, '.');
	if (p != NULL) {
		if (uS.mStricmp(p, ".wav") == 0 || uS.mStricmp(p, ".aif") == 0 || uS.mStricmp(p, ".aiff") == 0)
			mediaType = "audio";
		else
			mediaType = "video";
		*p = EOS;
	} else
		mediaType = "video";
	fprintf(fpout, "%s\t%s, %s\n", MEDIAHEADER, mediaFName, mediaType);
	if (isMultiBullets && !isOptionFound)
		fprintf(fpout, "@Options:\tmulti\n");

	ResetXMLTree();
	sortAnvilSpAndDepTiers();

	lastSpTier = RootTiers;
	refSp[0] = EOS;
	while (getNextAnvilTier(utterance, &beg, &end, refSp, &anvilID)) {
		if (ln > tln) {
			tln = ln + 200;
			fprintf(stderr,"\r%ld ",ln);
		}
		ln++;
		uS.remblanks(refSp);
		len = strlen(refSp);
		if (refSp[len-1] != ':')
			strcat(refSp, ":");
		uS.remblanks(utterance->speaker);
		len = strlen(utterance->speaker);
		if (utterance->speaker[len-1] != ':')
			strcat(utterance->speaker, ":");
		Anvil_makeText(utterance->line);
		Anvil_addToTiers(beg, end, utterance->speaker, refSp, utterance->line, anvilID, TRUE);
	}
	fprintf(stderr, "\r	  \r");
	Anvil_finalTimeSort();
	addReferences(RootTiers, 0L);
	cutt_isMultiFound = isMultiBullets;
	Anvil_printOutTiers(RootTiers, 0);
	fprintf(fpout, "@End\n");
	freeXML_Elements();
	RootTiers = freeTiers(RootTiers);
	lEncode = tlEncode;
}
