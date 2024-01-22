/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// chars format in unicode.cut: <actual ASCII char>, ^C<actual ASCII char>, 0x18, ^C0x18

#include "ced.h"
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main cptoutf_main
#define call cptoutf_call
#define getflag cptoutf_getflag
#define init cptoutf_init
#define usage cptoutf_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 4

#define AtU(s,i) (s[i] == '@' && (s[i+1] == 'u' || s[i+1] == 'U'))

#define UMAC 1
#define UPC  2

extern struct tier *defheadtier;
extern struct tier *headtier;
extern char OverWriteFile;

typedef struct WCHARMASK {
	char c1, c2;
} WCHARMASK;

#define ERR_CHARS struct err_chars
struct err_chars {
	char *err;
	ERR_CHARS *next_char;
} *root_err_chars;

static int convCol;
static char isOnlyTier;
static char isBOM;
static char isRemoveBullets;
static char isMakeTXT;
static char fTime;
static char lEncodeSet;
static char isUnibet;
static char isCUsed;
static char isLIPP;
static char isUTFFound;
static short lEncode;
static unsigned long isErrFound;
static unsigned short Convs[256][2];
#ifdef _MAC_CODE
static TextEncodingVariant lVariant;
#endif

#define dicname "unicode.cut"


static ERR_CHARS *cp2utf_free_errors(ERR_CHARS *p, char isDisplay) {
	ERR_CHARS *t;

	if (p != NULL && isDisplay) {
		fprintf(stderr, "\n\n********** ERROR ***********\n");
	}
	while (p != NULL) {
		t = p;
		p = p->next_char;
		if (isDisplay) {
			fprintf(stderr, "    Character not found: %s\n", t->err);
		}
		free(t->err);
		free(t);
	}
	return(NULL);
}

static void cp2utf_exit(int i) {
	root_err_chars = cp2utf_free_errors(root_err_chars, FALSE);
	cutt_exit(i);
}

static void add_err(char *err) {
	ERR_CHARS *t;

	if (root_err_chars == NULL) {
		if ((root_err_chars=NEW(ERR_CHARS)) == NULL) out_of_mem();
		t = root_err_chars;
	} else {
		for (t=root_err_chars; 1; t=t->next_char) {
			if (!strcmp(err, t->err))
				return;
			if (t->next_char == NULL)
				break;
		}
		if ((t->next_char=NEW(ERR_CHARS)) == NULL)
			out_of_mem();
		t = t->next_char;
	}
	if ((t->err=(char *)malloc(strlen(err)+1)) == NULL)
		out_of_mem();
	strcpy(t->err, err);
	t->next_char = NULL;
}

static void extractUCs(char *line, unsigned long *uc0, unsigned long *uc1) {
	char *end;

	if (*line == '"')
		line++;
	for (end=line; *end != ',' && *end != '\t' && *end != '\n' && *end != EOS; end++) ;
	sscanf(line, "%lx", uc0);
	if (*end == ',') {
		end++;
		sscanf(end, "%lx", uc1);
	}
}


static void readConvFile(void) {
	FILE *fdic;
	int i, j;
	unsigned char x, col[10];
	unsigned long uc0;
	unsigned long uc1;
	unsigned long hex;
	FNType mFileName[FNSize];

	for (uc0=0; uc0 < 33; uc0++) {
		Convs[uc0][0] = 0;
		Convs[uc0][1] = 0;
	}
	for (uc0=33; uc0 < 256; uc0++) {
		Convs[uc0][0] = 0x0394;
		Convs[uc0][1] = 0;
	}

	if ((fdic=OpenGenLib(dicname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", dicname, mFileName);
		return;
	}
	fprintf(stderr, "    Using Unicode table file: %s.\n", mFileName);
	while (fgets_cr(templineC, UTTLINELEN, fdic) != NULL) {
		if (uS.isUTF8(templineC) || templineC[0] == '#' || uS.isInvisibleHeader(templineC) ||
			strncmp(templineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0 || uS.partwcmp(templineC, "UniNum	"))
			continue;
		if (templineC[0] == (char)0xFF || templineC[0] == (char)0xFE) {
			fprintf(stderr, "*** ERROR: File \"%s\" must not be in a Unicode-UTF16 format.\n", dicname);
			cp2utf_exit(0);
		}
		if (uS.isUTF8(templineC)) {
			fprintf(stderr, "*** ERROR: File \"%s\" must not be in a Unicode-UTF8 format.\n", dicname);
			cp2utf_exit(0);
		}
		uc0 = 0L;
		uc1 = 0L;
		x = 0;
		col[convCol] = 0;

		if (templineC[0] != EOS && templineC[0] != '\n' && uS.mStrnicmp(templineC, "DEL", 3)) {
			for (i=0; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) {
				if (!isdigit(templineC[i]) && templineC[i] != 'A' && templineC[i] != 'B' && templineC[i] != 'C' &&
					  templineC[i] != 'D' && templineC[i] != 'E' && templineC[i] != 'F' && templineC[i] != '"' && templineC[i] != ',') {
					fputs("Only numbers or letters A,B,C,D,E,F and ',' and '\"' are allowed in first column\n", stderr);
					fprintf(stderr,"line:\t\"%s\"", templineC);
					fclose(fdic);
					cp2utf_exit(0);
				}
			}
		}
		if (!uS.mStrnicmp(templineC, "DEL", 3))
			uc0 = 0xFFFF;
		else {
			extractUCs(templineC, &uc0, &uc1);
		}
		for (i=0; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i]; i++) ;
		if (templineC[i] != '\t')
			continue;
		i++;
		if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
			x = templineC[i++];
		if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
			x = templineC[i++];

		for (j=0; j < 10; j++) {
			for (; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i]; i++) ;
			if (templineC[i] != '\t' && templineC[i] != '\n')
				break;
			if (templineC[i] == '\n')
				break;
			i++;
			if (templineC[i] == '^') {
				if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
					col[j] = templineC[i++];
				if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
					col[j] = templineC[i++];

				if (templineC[i] == '0' && templineC[i+1] == 'x') {
					i += 2;
					sscanf(templineC+i, "%lx", &hex);
					col[j] = (unsigned char)hex;
				} else
					if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
						col[j] = templineC[i++];
			} else if (templineC[i] == '0' && templineC[i+1] == 'x') {
				i += 2;
				sscanf(templineC+i, "%lx", &hex);
				col[j] = (unsigned char)hex;
			} else {
				if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
					col[j] = templineC[i++];
				if (templineC[i] == '0' && templineC[i+1] == 'x') {
					i += 2;
					sscanf(templineC+i, "%lx", &hex);
					col[j] = (unsigned char)hex;
				} else if (templineC[i] != '\t' && templineC[i] != '\n' && templineC[i])
					col[j] = templineC[i++];
			}
		}

		if (col[convCol] < 33) {
			if (Convs[col[convCol]][0] != 0 && Convs[col[convCol]][0] != (unsigned short)uc0 && Convs[col[convCol]][1] == 0 && uc1 == 0L) {
				fprintf(stderr,"Re-mapping of char:\"%c\"(0x%x) from: Unicode (0x%x) to: (0x%x)\n", col[convCol], col[convCol], Convs[col[convCol]][0], (short)uc0);
			} else
			if (Convs[col[convCol]][0] != 0 && Convs[col[convCol]][0] != (unsigned short)uc0 && Convs[col[convCol]][1] != 0 && Convs[col[convCol]][1] != (unsigned short)uc1) {
				fprintf(stderr,"Re-mapping of char:\"%c\"(0x%x) from: Unicode (0x%x)(0x%x) to: (0x%x)(0x%x)\n",
						col[convCol], col[convCol], Convs[col[convCol]][0], Convs[col[convCol]][1], (short)uc0, (short)uc1);
			}
		} else {
			if (Convs[col[convCol]][0] != 0x0394 && Convs[col[convCol]][0] != (unsigned short)uc0 && Convs[col[convCol]][1] == 0x0394 && uc1 == 0L) {
				fprintf(stderr,"Re-mapping of char:\"%c\"(0x%x) from: Unicode (0x%x) to: (0x%x)\n", col[convCol], col[convCol], Convs[col[convCol]][0], (short)uc0);
			} else
			if (Convs[col[convCol]][0] != 0x0394 && Convs[col[convCol]][0] != (unsigned short)uc0 && Convs[col[convCol]][1] != 0x0394 && Convs[col[convCol]][1] != (unsigned short)uc1) {
				fprintf(stderr,"Re-mapping of char:\"%c\"(0x%x) from: Unicode (0x%x)(0x%x) to: (0x%x)(0x%x)\n", 
						col[convCol], col[convCol], Convs[col[convCol]][0], Convs[col[convCol]][1], (short)uc0, (short)uc1);
			}
		}
		if (col[convCol] > 0) {
			Convs[col[convCol]][0] = (unsigned short)uc0;
			Convs[col[convCol]][1] = (unsigned short)uc1;
		}
	}
	fclose(fdic);
}

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		isBOM = FALSE;
		isOnlyTier = FALSE;
		isRemoveBullets = FALSE;
		isMakeTXT = FALSE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		fTime = TRUE;
		isUnibet = FALSE;
		isCUsed = FALSE;
		isLIPP = FALSE;
		isUTFFound = FALSE;
		lEncodeSet = 0;
#ifdef _MAC_CODE
		lVariant = kTextEncodingDefaultVariant;
#endif
		convCol = 1;
	} else if (fTime) {
		struct tier *p;
		char tcs = FALSE, tch = FALSE, tct = FALSE;

		readConvFile();
		if (chatmode != 0) {
			for (p=headtier; p != NULL; p=p->nexttier) {
				if (p->tcode[0] == '@')
					tch = TRUE;
				else if (p->tcode[0] == '*')
					tcs = TRUE;
				else if (p->tcode[0] == '%')
					tct = TRUE;
			}
			if (!tct) {
				if (!tcs && !tch)
					maketierchoice("%pho:", '+', FALSE);
				else
					maketierchoice("%:", '-', FALSE);
			}
			if (!tcs)
				maketierchoice("*", '-', FALSE);
			if (!tch)
				maketierchoice("@", '-', FALSE);
			for (p=headtier; p != NULL; p=p->nexttier)
				p->used = TRUE;
		}
		if (isRemoveBullets && !isMakeTXT) {
			fputs("The +d1 option can only be used with +d2 option\n", stderr);
			cutt_exit(0);
		}
		if (isMakeTXT)
			AddCEXExtension = ".txt";
		fTime = FALSE;
	}
}

void usage() {
	printf("Usage: cp2utf [cN d oS %s] filename(s)\n",mainflgs());
//	puts("+b : add BOM symbol to the output files");
	puts("+cN: specify column number (3-7) (default: 4, IPATimes)");
	printf("+d : convert ONLY tiers specified with +t option. It overrides %s Header exception\n", UTF8HEADER);
	puts("+d1: remove bullets from data file");
	puts("+d2: convert CHAT file to .txt file to help applications recognize UTF-8 encoding");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+oS: specify code page. Please type \"+o?\" for full listing of codes");
	puts("     utf16 - Unicode UTF-16 data file");
	puts("     macl  - Mac Latin (German, Spanish ...)");
	puts("     pcl   - PC  Latin (German, Spanish ...)");
#ifdef _MAC_CODE
/*
	puts("+vS: Specify code page variant)");
	puts("     macjp1- Mac Japanese Standard Variant");
	puts("     macjp2- Mac Japanese Standard No Verticals Variant");
	puts("     macjp3- Mac Japanese Basic Variant");
	puts("     macjp4- Mac Japanese PostScript Screen Variant");
	puts("     macjp5- Mac Japanese PostScript Print Variant");
	puts("     macjp6- Mac Japanese Vertical At KuPlusTen Variant");
*/
#endif
	mainusage(FALSE);
	puts("\nExample:");
	puts("\tunibet_word@u: cp2utf +t@u *.cha");
	puts("\tIPATimes: cp2utf -c4 +t%pho: *.cha");
	puts("\tSAMPA:  cp2utf -c5 +t%pho: *.cha");
	puts("\tLIPP:   cp2utf -c6 *.lip\n");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	char tisUTFData;

	isUTFFound = FALSE;
	root_err_chars = NULL;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CP2UTF;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	tisUTFData = isUTFData;
	isUTFData = FALSE;
	bmain(argc,argv,NULL);
	isUTFData = tisUTFData;
	root_err_chars = cp2utf_free_errors(root_err_chars, TRUE);
	if (isUTFFound) {
#ifdef UNX
		printf("\nFOUND DATA FILES WITH %s HEADER. THOSE FILES WERE NOT CONVERTED.\n", UTF8HEADER);
#else
		printf("\n%c%cFOUND DATA FILES WITH %s HEADER. THOSE FILES WERE NOT CONVERTED.%c%c\n", ATTMARKER, error_start, UTF8HEADER, ATTMARKER, error_end);
#endif
		if (convCol == 1 || convCol == 2) {
#ifdef UNX
			printf("IF YOU WANT TO OVERRIDE THE %s HEADER EXCEPTION, THEN ADD \"+d\" OPTION.\n", UTF8HEADER);
#else
			printf("%c%cIF YOU WANT TO OVERRIDE THE %s HEADER EXCEPTION, THEN ADD \"+d\" OPTION.%c%c\n", ATTMARKER, error_start, UTF8HEADER, ATTMARKER, error_end);
#endif
		}
	}
}
		
void getflag(char *f, char *f1, int *i) {
	char res;

	f++;
	switch(*f++) {
/*
		case 'b':
			isBOM = TRUE;
			no_arg_option(f);
			break;
*/
		case 'c':
			convCol = atoi(getfarg(f,f1,i)) - 3;
			if (convCol < 0 || convCol > 4) {
				fprintf(stderr, "Column number must be between 3 and 7\n");
				cp2utf_exit(0);
			}
			if (convCol == 3)
				isLIPP = TRUE;
			isCUsed = TRUE;
			break;
		case 'd':
			res = (char)atoi(getfarg(f,f1,i));
			if (res > 2) {
				fputs("The N for +d must be between 0 - 2\n", stderr);
				cutt_exit(0);
			}
			if (res == 0)
				isOnlyTier = TRUE;
			else if (res == 1)
				isRemoveBullets = TRUE;
			else if (res == 2)
				isMakeTXT = TRUE;
			break;
#ifdef UNX
		case 'L':
			int len;
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 't':
			if (!uS.mStricmp(getfarg(f,f1,i), "@U"))
				isUnibet = TRUE;
			else
				maingetflag(f-2,f1,i);
			break;
		case 'o':
			lEncodeSet = UTF8;
#ifdef _MAC_CODE
			if (!uS.mStricmp(f, "utf16"))
				lEncodeSet = UTF16;
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
				puts("     utf16 - Unicode UTF-16 data file\n");
				displayOoption();
				cutt_exit(0);
			}
#endif
#ifdef _WIN32 
			if (!uS.mStricmp(f, "utf16"))
				lEncodeSet = UTF16;
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
				puts("     utf16 - Unicode UTF-16 data file\n");
				displayOoption();
				cutt_exit(0);
			}
#endif
			break;
		case 'v':
#ifdef _MAC_CODE
			if (!uS.mStricmp(f, "macjp1"))
				lVariant = kMacJapaneseStandardVariant;
			else if (!uS.mStricmp(f, "macjp2"))
				lVariant = kMacJapaneseStdNoVerticalsVariant;
			else if (!uS.mStricmp(f, "macjp3"))
				lVariant = kMacJapaneseBasicVariant;
			else if (!uS.mStricmp(f, "macjp4"))
				lVariant = kMacJapanesePostScriptScrnVariant;
			else if (!uS.mStricmp(f, "macjp5"))
				lVariant = kMacJapanesePostScriptPrintVariant;
			else if (!uS.mStricmp(f, "macjp6"))
				lVariant = kMacJapaneseVertAtKuPlusTenVariant;
			else {
				sprintf(templineC3, "Unrecognized font variant \"%s\".", f);
				do_warning(templineC3, 0);
			}
#else
			sprintf(templineC3, "Unrecognized font variant \"%s\".", f);
			do_warning(templineC3, 0);
#endif
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static long isCanConvert(unsigned char uChr, short isCA, long lout) {
	long aol;

	aol = 0L;
	if (isCA == MACcain) {
		if (uChr == 0xA4) { // up-arrow
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x86;
			templineC[lout+2] = 0x91;
			aol = 3L;
		} else if (uChr == 0xC3) {// down-arrow
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x86;
			templineC[lout+2] = 0x93;
			aol = 3L;
		} else if (uChr == 0xBC) {// super-0
			templineC[lout] = 0xc2;
			templineC[lout+1] = 0xb0;
			aol = 2L;
		} else if (uChr == 0xD2) {// // lowered [
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x8a;
			aol = 3L;
		} else if (uChr == 0xD4) {// // lowered ]
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x8b;
			aol = 3L;
		} else if (uChr == 0xD3) {// raised [
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x88;
			aol = 3L;
		} else if (uChr == 0xD5) {// raised ]
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x89;
			aol = 3L;
		}
	} else {
		if (uChr == 0xA7) { // up-arrow
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x86;
			templineC[lout+2] = 0x91;
			aol = 3L;
		} else if (uChr == 0xAF) {// down-arrow
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x86;
			templineC[lout+2] = 0x93;
			aol = 3L;
		} else if (uChr == 0xBA) {// super-0
			templineC[lout] = 0xc2;
			templineC[lout+1] = 0xb0;
			aol = 2L;
		} else if (uChr == 0xA5) {// raised [
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x8a;
			aol = 3L;
		} else if (uChr == 0xA4) {// raised ]
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x8b;
			aol = 3L;
		} else if (uChr == 0xAB) {// lowered [
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x88;
			aol = 3L;
		} else if (uChr == 0xAA) {// lowered ]
			templineC[lout] = 0xe2;
			templineC[lout+1] = 0x8c;
			templineC[lout+2] = 0x89;
			aol = 3L;
		}
	}
	return(aol);
}


/* convert to UNICODE begin */
#ifdef _WIN32 
#include <mbstring.h>

/*
  DOSLatinUS       = code page 437
  DOSGreek         = code page 737 (formerly code page 437G)
  DOSBalticRim     = code page 775
  DOSLatin1        = code page 850, "Multilingual"
  DOSGreek1        = code page 851
  DOSLatin2        = code page 852, Slavic
  DOSCyrillic      = code page 855, IBM Cyrillic
  DOSTurkish       = code page 857, IBM Turkish
  DOSPortuguese    = code page 860
  DOSIcelandic     = code page 861
  DOSHebrew        = code page 862
  DOSCanadianFrench = code page 863
  DOSArabic        = code page 864
  DOSNordic        = code page 865
  DOSRussian       = code page 866
  DOSGreek2        = code page 869, IBM Modern Greek
  DOSThai          = code page 874, also for Windows
  DOSJapanese      = code page 932, also for Windows; Shift-JIS with additions
  DOSChineseSimplif = code page 936, also for Windows; was EUC-CN, now GBK (EUC-CN extended)
  DOSKorean        = code page 949, also for Windows; Unified Hangul Code (EUC-KR extended)
  DOSChineseTrad   = code page 950, also for Windows; Big-5
  WindowsLatin1    = code page 1252
  WindowsANSI      = code page 1252 (alternate name)
  WindowsLatin2    = code page 1250, Central Europe
  WindowsCyrillic  = code page 1251, Slavic Cyrillic
  WindowsGreek     = code page 1253
  WindowsLatin5    = code page 1254, Turkish
  WindowsHebrew    = code page 1255
  WindowsArabic    = code page 1256
  WindowsBalticRim = code page 1257
  WindowsVietnamese = code page 1258
  WindowsKoreanJohab = code page 1361, for Windows NT
*/
/*
static void WideToUTF8(LPWSTR line, long wchars) {
	long uchars=WideCharToMultiByte(CP_UTF8,0,line,wchars,NULL,0,NULL,NULL);
	LPSTR lpuText = (LPSTR)malloc(uchars+1);
	WideCharToMultiByte(CP_UTF8,0,line,wchars,lpuText,uchars,NULL,NULL);
	lpuText[uchars] = '\0';
	fputs(lpuText, fpout);

	free(lpuText);
}
*/
static void AsciiToUnicodeToUTF8(char *lpszText) {
	long UTF8Len;
	long total = strlen(lpszText);
	long wchars=MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText,total,NULL,0);
//	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, (wchars+1)*2);
//	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);
//	MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText,total,lpwText,wchars);
	MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText,total,templineW,wchars);

//	UnicodeToUTF8(lpwText, wchars, (unsigned char *)templineC4, (unsigned long *)&UTF8Len, UTTLINELEN);
	UnicodeToUTF8(templineW, wchars, (unsigned char *)templineC4, (unsigned long *)&UTF8Len, UTTLINELEN);
	if (UTF8Len == 0 && wchars > 0) {
		putc('\n', stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
		return;
	}
	fputs(templineC4, fpout);


//	LocalUnlock(hwText);
//	HLOCAL LocalFree(hwText);
	
}

static void IPAToUTF8(unsigned char *line) {
	long i, j, wchars;
	WCHARMASK *wc;

	wchars = 0L;
	for (i=0L; line[i] != EOS; i++) {
		if (line[i] < 256 && wchars < UTTLINELEN-3) {
			if (Convs[line[i]][0] == 0xFFFF)
				;
			else if (Convs[line[i]][0] != 0) {
				wc = (struct WCHARMASK *)&Convs[line[i]][0];
				templineC[wchars++] = wc->c1;
				templineC[wchars++] = wc->c2;
				if (Convs[line[i]][1] != 0) {
					wc = (struct WCHARMASK *)&Convs[line[i]][1];
					templineC[wchars++] = wc->c1;
					templineC[wchars++] = wc->c2;
				}
				if (Convs[line[i]][0] == 0x0394) {
					sprintf(templineC1, "(0x%x)", line[i]);
					add_err(templineC1);
					for (j=0L; templineC1[j] != EOS; j++) {
						templineC[wchars++] = templineC1[j];
						templineC[wchars++] = 0;
					}
				}
			} else {
				templineC[wchars++] = line[i];
				templineC[wchars++] = 0;
			}
		}
	}
	wchars /= 2;
	UnicodeToUTF8((LPWSTR)templineC, wchars, (unsigned char *)templineC4, (unsigned long *)&j, UTTLINELEN);
	if (j == 0 && wchars > 0) {
		putc('\n', stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, line);
		return;
	}
	fputs(templineC4, fpout);
}

static void CAToUTF8(unsigned char *lpszText, AttTYPE *att, short isCA) {
	long i, len, lout, wchars, aol;
//	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, 5);
//	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);

	len = strlen((char *)lpszText);
	lout = 0L;
	for (i=0L; i < len; i++) {
		aol = isCanConvert(lpszText[i], isCA, lout);

		if (aol == 0) {
			wchars = MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,NULL,0);
//			MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,lpwText,5);
			MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,templineW,5);

//			aol = WideCharToMultiByte(CP_UTF8,0,lpwText,wchars,NULL,0,NULL,NULL);
			aol = WideCharToMultiByte(CP_UTF8,0,templineW,wchars,NULL,0,NULL,NULL);
//			WideCharToMultiByte(CP_UTF8,0,lpwText,wchars,templineC+lout,UTTLINELEN-lout,NULL,NULL);
			WideCharToMultiByte(CP_UTF8,0,templineW,wchars,templineC+lout,UTTLINELEN-lout,NULL,NULL);
		}

		for (; aol > 0; aol--)
			tempAtt[lout++] = att[i];

	}
//	LocalUnlock(hwText);
//	HLOCAL LocalFree(hwText);

	templineC[lout] = EOS;
	printout(NULL, templineC, NULL, tempAtt, FALSE);

}

static void UnibetToUTF8(unsigned char *lpszText, AttTYPE *att) {
	long i, len, lout, wchars, aol;
//	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, 5);
//	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);
	WCHARMASK *wc;

	len = strlen((char *)lpszText);
	lout = 0L;
	for (i=0L; i < len;) {
		if (uS.atUFound((char *)lpszText,i,&dFnt,MBF) && !AtU(lpszText, i)) {
			while (1) {
				wchars = 0L;
				if (Convs[lpszText[i]][0] == 0xFFFF)
					;
				else if (Convs[lpszText[i]][0] != 0) {
					wc = (struct WCHARMASK *)&Convs[lpszText[i]][0];
					templineC3[wchars++] = wc->c1;
					templineC3[wchars++] = wc->c2;
					if (Convs[lpszText[i]][1] != 0) {
						wc = (struct WCHARMASK *)&Convs[lpszText[i]][1];
						templineC3[wchars++] = wc->c1;
						templineC3[wchars++] = wc->c2;
					}
					if (Convs[lpszText[i]][0] == 0x0394) {
						sprintf(templineC1, "(0x%x)", lpszText[i]);
						add_err(templineC1);
						for (aol=0L; templineC1[aol] != EOS; aol++) {
							templineC3[wchars++] = 0;
							templineC3[wchars++] = templineC1[aol];
						}
					}
				} else {
					sprintf(templineC1, "(0x%x)", lpszText[i]);
					for (aol=0L; templineC1[aol] != EOS; aol++) {
						templineC3[wchars++] = 0;
						templineC3[wchars++] = templineC1[aol];
					}
				}

				wchars /= 2;
				UnicodeToUTF8((LPWSTR)templineC3, wchars, (unsigned char *)templineC + lout, (unsigned long *)&aol, UTTLINELEN - lout);
				if (aol == 0) {
					putc('\n', stderr);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				}

				for (; aol > 0; aol--)
					tempAtt[lout++] = att[i];
				i++;
				if (AtU(lpszText, i))
					break;
			}
		} else if (dFnt.isUTF) {
			templineC[lout] = lpszText[i];
			tempAtt[lout] = att[i];
			lout++;
			i++;
		} else {
			wchars = MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,NULL,0);
//			MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,lpwText,5);
			MultiByteToWideChar(dFnt.orgEncod,0,(const char*)lpszText+i,1,templineW,5);

//			aol = WideCharToMultiByte(CP_UTF8,0,lpwText,wchars,NULL,0,NULL,NULL);
			aol = WideCharToMultiByte(CP_UTF8,0,templineW,wchars,NULL,0,NULL,NULL);
//			WideCharToMultiByte(CP_UTF8,0,lpwText,wchars,templineC+lout,UTTLINELEN-lout,NULL,NULL);
			WideCharToMultiByte(CP_UTF8,0,templineW,wchars,templineC+lout,UTTLINELEN-lout,NULL,NULL);

			for (; aol > 0; aol--)
				tempAtt[lout++] = att[i];
			i++;
		}
	}
//	LocalUnlock(hwText);
//	HLOCAL LocalFree(hwText);

	templineC[lout] = EOS;
	printout(NULL, templineC, NULL, tempAtt, FALSE);

}
#endif /* _WIN32 */

/*
enum {
  kTextEncodingMacRoman         = 0L,
  kTextEncodingMacJapanese      = 1,
  kTextEncodingMacChineseTrad   = 2,
  kTextEncodingMacKorean        = 3,
  kTextEncodingMacArabic        = 4,
  kTextEncodingMacHebrew        = 5,
  kTextEncodingMacGreek         = 6,
  kTextEncodingMacCyrillic      = 7,
  kTextEncodingMacDevanagari    = 9,
  kTextEncodingMacGurmukhi      = 10,
  kTextEncodingMacGujarati      = 11,
  kTextEncodingMacOriya         = 12,
  kTextEncodingMacBengali       = 13,
  kTextEncodingMacTamil         = 14,
  kTextEncodingMacTelugu        = 15,
  kTextEncodingMacKannada       = 16,
  kTextEncodingMacMalayalam     = 17,
  kTextEncodingMacSinhalese     = 18,
  kTextEncodingMacBurmese       = 19,
  kTextEncodingMacKhmer         = 20,
  kTextEncodingMacThai          = 21,
  kTextEncodingMacLaotian       = 22,
  kTextEncodingMacGeorgian      = 23,
  kTextEncodingMacArmenian      = 24,
  kTextEncodingMacChineseSimp   = 25,
  kTextEncodingMacTibetan       = 26,
  kTextEncodingMacMongolian     = 27,
  kTextEncodingMacEthiopic      = 28,
  kTextEncodingMacCentralEurRoman = 29,
  kTextEncodingMacVietnamese    = 30,
  kTextEncodingMacExtArabic     = 31,   // The following use script code 0, smRoman
  kTextEncodingMacSymbol        = 33,
  kTextEncodingMacDingbats      = 34,
  kTextEncodingMacTurkish       = 35,
  kTextEncodingMacCroatian      = 36,
  kTextEncodingMacIcelandic     = 37,
  kTextEncodingMacRomanian      = 38,
  kTextEncodingMacCeltic        = 39,
  kTextEncodingMacGaelic        = 40,
  kTextEncodingMacKeyboardGlyphs = 41
};

// The following are older names for backward compatibility
enum {
  kTextEncodingMacTradChinese   = kTextEncodingMacChineseTrad,
  kTextEncodingMacRSymbol       = 8,
  kTextEncodingMacSimpChinese   = kTextEncodingMacChineseSimp,
  kTextEncodingMacGeez          = kTextEncodingMacEthiopic,
  kTextEncodingMacEastEurRoman  = kTextEncodingMacCentralEurRoman,
  kTextEncodingMacUninterp      = 32
};

enum {
  kTextEncodingMacUnicode       = 0x7E  // Meta-value, Unicode as a Mac encoding
};

// Variant Mac OS encodings that use script codes other than 0
enum {
                                        // The following use script code 4, smArabic
  kTextEncodingMacFarsi         = 0x8C, // Like MacArabic but uses Farsi digits
                                        // The following use script code 7, smCyrillic
  kTextEncodingMacUkrainian     = 0x98, // Meta-value in TEC 1.5 & later; maps to kTextEncodingMacCyrillic variant    
                                        // The following use script code 28, smEthiopic
  kTextEncodingMacInuit         = 0xEC, // The following use script code 32, smUnimplemented
  kTextEncodingMacVT100         = 0xFC  // VT100/102 font from Comm Toolbox: Latin-1 repertoire + box drawing etc
};

// Special Mac OS encodings
enum {
  kTextEncodingMacHFS           = 0xFF  // Meta-value, should never appear in a table.
};

// Unicode & ISO UCS encodings begin at 0x100
enum {
  kTextEncodingUnicodeDefault   = 0x0100, // Meta-value, should never appear in a table.
  kTextEncodingUnicodeV1_1      = 0x0101,
  kTextEncodingISO10646_1993    = 0x0101, // Code points identical to Unicode 1.1
  kTextEncodingUnicodeV2_0      = 0x0103, // New location for Korean Hangul
  kTextEncodingUnicodeV2_1      = 0x0103, // We treat both Unicode 2.0 and Unicode 2.1 as 2.1
  kTextEncodingUnicodeV3_0      = 0x0104,
  kTextEncodingUnicodeV3_1      = 0x0105, // Adds characters requiring surrogate pairs in UTF-16
  kTextEncodingUnicodeV3_2      = 0x0106
};

// ISO 8-bit and 7-bit encodings begin at 0x200
enum {
  kTextEncodingISOLatin1        = 0x0201, // ISO 8859-1
  kTextEncodingISOLatin2        = 0x0202, // ISO 8859-2
  kTextEncodingISOLatin3        = 0x0203, // ISO 8859-3
  kTextEncodingISOLatin4        = 0x0204, // ISO 8859-4
  kTextEncodingISOLatinCyrillic = 0x0205, // ISO 8859-5
  kTextEncodingISOLatinArabic   = 0x0206, // ISO 8859-6, = ASMO 708, =DOS CP 708
  kTextEncodingISOLatinGreek    = 0x0207, // ISO 8859-7
  kTextEncodingISOLatinHebrew   = 0x0208, // ISO 8859-8
  kTextEncodingISOLatin5        = 0x0209, // ISO 8859-9
  kTextEncodingISOLatin6        = 0x020A, // ISO 8859-10                           
  kTextEncodingISOLatin7        = 0x020D, // ISO 8859-13, Baltic Rim                   
  kTextEncodingISOLatin8        = 0x020E, // ISO 8859-14, Celtic                    
  kTextEncodingISOLatin9        = 0x020F // ISO 8859-15, 8859-1 changed for EURO & CP1252 letters  
};

// MS-DOS & Windows encodings begin at 0x400
enum {
  kTextEncodingDOSLatinUS       = 0x0400, // code page 437
  kTextEncodingDOSGreek         = 0x0405, // code page 737 (formerly code page 437G)
  kTextEncodingDOSBalticRim     = 0x0406, // code page 775
  kTextEncodingDOSLatin1        = 0x0410, // code page 850, "Multilingual"
  kTextEncodingDOSGreek1        = 0x0411, // code page 851
  kTextEncodingDOSLatin2        = 0x0412, // code page 852, Slavic
  kTextEncodingDOSCyrillic      = 0x0413, // code page 855, IBM Cyrillic
  kTextEncodingDOSTurkish       = 0x0414, // code page 857, IBM Turkish
  kTextEncodingDOSPortuguese    = 0x0415, // code page 860
  kTextEncodingDOSIcelandic     = 0x0416, // code page 861
  kTextEncodingDOSHebrew        = 0x0417, // code page 862
  kTextEncodingDOSCanadianFrench = 0x0418, // code page 863
  kTextEncodingDOSArabic        = 0x0419, // code page 864
  kTextEncodingDOSNordic        = 0x041A, // code page 865
  kTextEncodingDOSRussian       = 0x041B, // code page 866
  kTextEncodingDOSGreek2        = 0x041C, // code page 869, IBM Modern Greek
  kTextEncodingDOSThai          = 0x041D, // code page 874, also for Windows
  kTextEncodingDOSJapanese      = 0x0420, // code page 932, also for Windows; Shift-JIS with additions
  kTextEncodingDOSChineseSimplif = 0x0421, // code page 936, also for Windows; was EUC-CN, now GBK (EUC-CN extended)
  kTextEncodingDOSKorean        = 0x0422, // code page 949, also for Windows; Unified Hangul Code (EUC-KR extended)
  kTextEncodingDOSChineseTrad   = 0x0423, // code page 950, also for Windows; Big-5
  kTextEncodingWindowsLatin1    = 0x0500, // code page 1252
  kTextEncodingWindowsANSI      = 0x0500, // code page 1252 (alternate name)
  kTextEncodingWindowsLatin2    = 0x0501, // code page 1250, Central Europe
  kTextEncodingWindowsCyrillic  = 0x0502, // code page 1251, Slavic Cyrillic
  kTextEncodingWindowsGreek     = 0x0503, // code page 1253
  kTextEncodingWindowsLatin5    = 0x0504, // code page 1254, Turkish
  kTextEncodingWindowsHebrew    = 0x0505, // code page 1255
  kTextEncodingWindowsArabic    = 0x0506, // code page 1256
  kTextEncodingWindowsBalticRim = 0x0507, // code page 1257
  kTextEncodingWindowsVietnamese = 0x0508, // code page 1258
  kTextEncodingWindowsKoreanJohab = 0x0510 // code page 1361, for Windows NT
};

// Various national standards begin at 0x600
enum {
  kTextEncodingUS_ASCII         = 0x0600,
  kTextEncodingJIS_X0201_76     = 0x0620, // JIS Roman and 1-byte katakana (halfwidth)
  kTextEncodingJIS_X0208_83     = 0x0621,
  kTextEncodingJIS_X0208_90     = 0x0622,
  kTextEncodingJIS_X0212_90     = 0x0623,
  kTextEncodingJIS_C6226_78     = 0x0624,
  kTextEncodingShiftJIS_X0213_00 = 0x0628, // Shift-JIS format encoding of JIS X0213 planes 1 and 2
  kTextEncodingGB_2312_80       = 0x0630,
  kTextEncodingGBK_95           = 0x0631, // annex to GB 13000-93; for Windows 95; EUC-CN extended
  kTextEncodingGB_18030_2000    = 0x0632,
  kTextEncodingKSC_5601_87      = 0x0640, // same as KSC 5601-92 without Johab annex
  kTextEncodingKSC_5601_92_Johab = 0x0641, // KSC 5601-92 Johab annex
  kTextEncodingCNS_11643_92_P1  = 0x0651, // CNS 11643-1992 plane 1
  kTextEncodingCNS_11643_92_P2  = 0x0652, // CNS 11643-1992 plane 2
  kTextEncodingCNS_11643_92_P3  = 0x0653 // CNS 11643-1992 plane 3 (was plane 14 in 1986 version)
};

// ISO 2022 collections begin at 0x800
enum {
  kTextEncodingISO_2022_JP      = 0x0820, // RFC 1468
  kTextEncodingISO_2022_JP_2    = 0x0821, // RFC 1554
  kTextEncodingISO_2022_JP_1    = 0x0822, // RFC 2237
  kTextEncodingISO_2022_JP_3    = 0x0823, // JIS X0213
  kTextEncodingISO_2022_CN      = 0x0830,
  kTextEncodingISO_2022_CN_EXT  = 0x0831,
  kTextEncodingISO_2022_KR      = 0x0840
};

// EUC collections begin at 0x900
enum {
  kTextEncodingEUC_JP           = 0x0920, // ISO 646, 1-byte katakana, JIS 208, JIS 212
  kTextEncodingEUC_CN           = 0x0930, // ISO 646, GB 2312-80
  kTextEncodingEUC_TW           = 0x0931, // ISO 646, CNS 11643-1992 Planes 1-16
  kTextEncodingEUC_KR           = 0x0940 // ISO 646, KS C 5601-1987
};

// Misc standards begin at 0xA00
enum {
  kTextEncodingShiftJIS         = 0x0A01, // plain Shift-JIS
  kTextEncodingKOI8_R           = 0x0A02, // Russian internet standard
  kTextEncodingBig5             = 0x0A03, // Big-5 (has variants)
  kTextEncodingMacRomanLatin1   = 0x0A04, // Mac OS Roman permuted to align with ISO Latin-1
  kTextEncodingHZ_GB_2312       = 0x0A05, // HZ (RFC 1842, for Chinese mail & news)
  kTextEncodingBig5_HKSCS_1999  = 0x0A06 // Big-5 with Hong Kong special char set supplement
};
*/

#ifdef _MAC_CODE
/* 7-4-03
static void WideToUTF8(char *line, long wchars) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	unsigned long ail, aol;
	long i;

	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, kTextEncodingUnicodeDefault, utf8Encoding)) != noErr) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error4: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, line);
		return;
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)line, wchars, &ail, (TextPtr)templineC4, UTTLINELEN, &aol)) != noErr) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error5: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, line);
		return;
	}
	err = TECDisposeConverter(ec);
	if (ail < wchars) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error6: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, line);
		return;
	}
	templineC4[aol] = EOS;
	for (i=0L; templineC4[i]; i++) {
		if (templineC4[i] == '\n') {
			putc('\n', fpout);
		} else
			putc(templineC4[i], fpout);
	}
}
*/
static void AsciiToUnicodeToUTF8(char *lpszText) {
	OSStatus err;
	long len;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	TextEncoding MacRomanEncoding;
	unsigned long ail, aol;

	MacRomanEncoding = CreateTextEncoding( (long)dFnt.orgEncod, lVariant, kTextEncodingDefaultFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error1: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
		return;
	}

repeat:
	len = strlen(lpszText);
	if ((err=TECConvertText(ec, (ConstTextPtr)lpszText, len, &ail, (TextPtr)templineC, UTTLINELEN, &aol)) != noErr) {
		if (err == kTextMalformedInputErr && ail < len) {
			isErrFound++;
			strncpy(templineC1, lpszText, ail);
			sprintf(templineC1+ail, "(0x%x)", (unsigned char)lpszText[ail]);
			strcat(templineC1, lpszText+ail+1);
			strcpy(lpszText, templineC1);
			goto repeat;
		} else {
			putc('\n', fpout);
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
			fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
			return;
		}
	}
	err = TECDisposeConverter(ec);
	if (ail < len) {
		putc('\n', fpout);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error3: Converted only %ld out of %ld chars:\n", ail, len);
		fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
		return;
	}
	templineC[aol] = EOS;
	for (len=0L; templineC[len]; len++) {
		if (templineC[len] == '\n') {
			putc('\n', fpout);
		} else
			putc(templineC[len], fpout);
	}
//	fputs(templineC, fpout);
}

static void IPAToUTF8(unsigned char *line) {
	long i, j, wchars;
	WCHARMASK *wc;

	wchars = 0L;
	for (i=0L; line[i] != EOS; i++) {
		if (wchars < UTTLINELEN-3) {
			if (Convs[line[i]][0] == 0xFFFF)
				;
			else if (Convs[line[i]][0] != 0) {
				wc = (struct WCHARMASK *)&Convs[line[i]][0];
				templineC[wchars++] = wc->c1;
				templineC[wchars++] = wc->c2;
				if (Convs[line[i]][1] != 0) {
					wc = (struct WCHARMASK *)&Convs[line[i]][1];
					templineC[wchars++] = wc->c1;
					templineC[wchars++] = wc->c2;
				}
				if (Convs[line[i]][0] == 0x0394) {
					sprintf(templineC1, "(0x%x)", line[i]);
					add_err(templineC1);
					for (j=0L; templineC1[j] != EOS; j++) {
						if (byteOrder == CFByteOrderLittleEndian) {
							templineC[wchars++] = templineC1[j];
							templineC[wchars++] = 0;
						} else {
							templineC[wchars++] = 0;
							templineC[wchars++] = templineC1[j];
						}
					}
				}
			} else {
				if (byteOrder == CFByteOrderLittleEndian) {
					templineC[wchars++] = line[i];
					templineC[wchars++] = 0;
				} else {
					templineC[wchars++] = 0;
					templineC[wchars++] = line[i];
				}
			}
		}
	}
	UnicodeToUTF8((unCH *)templineC, wchars/2, (unsigned char *)templineC4, (unsigned long *)&j, UTTLINELEN);
	if (j == 0 && wchars > 0) {
		putc('\n', stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, line);
		return;
	}
	for (i=0L; templineC4[i]; i++) {
		if (templineC4[i] == '\n') {
			putc('\n', fpout);
		} else
			putc(templineC4[i], fpout);
	}
}

static void CAToUTF8(unsigned char *lpszText, AttTYPE *att, short isCA) {
	OSStatus err;
	long i, len, lout;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	TextEncoding MacRomanEncoding;
	unsigned long ail, aol;

	MacRomanEncoding = CreateTextEncoding( (long)dFnt.orgEncod, lVariant, kTextEncodingDefaultFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Fatal error1: Unable to convert the following line:\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
		return;
	}

	len = strlen((char *)lpszText);
	lout = 0L;
	for (i=0L; i < len; i++) {
		aol = isCanConvert(lpszText[i], isCA, lout);
		if (aol == 0L) {
			if ((err=TECConvertText(ec, (ConstTextPtr)lpszText+i, 1, &ail, (TextPtr)templineC+lout, UTTLINELEN-lout, &aol)) != noErr) {
				if (err == kTextMalformedInputErr && ail < 1) {
					putc('\n', fpout);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				} else {
					putc('\n', fpout);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error2.5: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				}
			}
			if (ail < 1) {
				putc('\n', fpout);
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Fatal error3: Converted only %ld out of %ld chars:\n", i, len);
				fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
				return;
			}
		}
		for (; aol > 0; aol--)
			tempAtt[lout++] = att[i];
	}
	err = TECDisposeConverter(ec);
	templineC[lout] = EOS;
	printout(NULL, templineC, NULL, tempAtt, FALSE);
}

static void UnibetToUTF8(unsigned char *lpszText, AttTYPE *att) {
	OSStatus err;
	long i, len, lout, wchars;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	TextEncoding MacRomanEncoding;
	WCHARMASK *wc;
	unsigned long ail, aol;

	MacRomanEncoding = CreateTextEncoding( (long)dFnt.orgEncod, lVariant, kTextEncodingDefaultFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		fprintf(stderr, "Fatal error1: Unable to create proper converter\n");
		return;
	}

	len = strlen((char *)lpszText);
	lout = 0L;
	for (i=0L; i < len;) {
		if (uS.atUFound((char *)lpszText,i,&dFnt,MBF) && !AtU(lpszText, i)) {
			while (1) {
				wchars = 0L;
				if (Convs[lpszText[i]][0] == 0xFFFF)
					;
				else if (Convs[lpszText[i]][0] != 0) {
					wc = (struct WCHARMASK *)&Convs[lpszText[i]][0];
					templineC3[wchars++] = wc->c1;
					templineC3[wchars++] = wc->c2;
					if (Convs[lpszText[i]][1] != 0) {
						wc = (struct WCHARMASK *)&Convs[lpszText[i]][1];
						templineC3[wchars++] = wc->c1;
						templineC3[wchars++] = wc->c2;
					}
					if (Convs[lpszText[i]][0] == 0x0394) {
						sprintf(templineC1, "(0x%x)", lpszText[i]);
						add_err(templineC1);
						for (aol=0L; templineC1[aol] != EOS; aol++) {
							if (byteOrder == CFByteOrderLittleEndian) {
								templineC3[wchars++] = templineC1[aol];
								templineC3[wchars++] = 0;
							} else {
								templineC3[wchars++] = 0;
								templineC3[wchars++] = templineC1[aol];
							}
						}
					}
				} else {
					sprintf(templineC1, "(0x%x)", lpszText[i]);
					for (aol=0L; templineC1[aol] != EOS; aol++) {
						if (byteOrder == CFByteOrderLittleEndian) {
							templineC3[wchars++] = templineC1[aol];
							templineC3[wchars++] = 0;
						} else {
							templineC3[wchars++] = 0;
							templineC3[wchars++] = templineC1[aol];
						}
					}
				}

				UnicodeToUTF8((unCH *)templineC3, wchars/2, (unsigned char *)templineC+lout, &aol, UTTLINELEN-lout);
				if (aol == 0) {
					putc('\n', stderr);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				}

				for (; aol > 0; aol--)
					tempAtt[lout++] = att[i];
				i++;
				if (AtU(lpszText, i))
					break;
			}
		} else if (dFnt.isUTF) {
			templineC[lout] = lpszText[i];
			tempAtt[lout] = att[i];
			lout++;
			i++;
		} else {
			if ((err=TECConvertText(ec, (ConstTextPtr)lpszText+i, 1, &ail, (TextPtr)templineC+lout, UTTLINELEN-lout, &aol)) != noErr) {
				if (err == kTextMalformedInputErr && ail < 1) {
					putc('\n', fpout);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error2: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				} else {
					putc('\n', fpout);
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Fatal error2.5: Unable to convert the following line:\n");
					fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
					return;
				}
			}
			if (ail < 1) {
				putc('\n', fpout);
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Fatal error3: Converted only %ld out of %ld chars:\n", i, len);
				fprintf(stderr, "%s%s\n", utterance->speaker, lpszText);
				return;
			}
			for (; aol > 0; aol--)
				tempAtt[lout++] = att[i];
			i++;
		}
	}
	err = TECDisposeConverter(ec);
	templineC[lout] = EOS;
	printout(NULL, templineC, NULL, tempAtt, FALSE);
}
#endif
/* convert to UNICODE begin */

/* {===================== LIPP Document Reading Begin ========================} */
#define BlankCode			0
#define MainBufferSize		5000
#define DataLineTypeSize	2400
#define LIPP30				"LIPP30"
#define LIPP14				"LIPP14"
#define LIPP10				"LIPP10"

/*
struct DataType {
	char  Ch[13];     //{   13 Bytes }
	short TapeCount;  //{    2 Bytes }
	short Start;      //{    2 Bytes }
	short Duration;   //{    2 Bytes }
	char Comment[4];  //{    4 Bytes }
} ; 23 bytes long
*/

typedef struct DataLineType {
	unsigned char Line[80][23];//{ 23X80=1840 Bytes}  Line:Array[1..80] of DataType;
	char LineComment[100];     //{        100 Bytes}
	char FileName[220];        //{        220 Bytes - Added for Win}
	char Speaker[60];          //{        60 Bytes}
	char TimerStart[20];       //{        20 Bytes}
	char TimerEnd[20];         //{        20 Bytes}
	char Dummy1[20];           //{        20 Bytes}
	char Dummy2[20];           //{        20 Bytes}
	char Dummy3[20];           //{        20 Bytes}
	char Dummy4[20];           //{        20 Bytes}
	char Dummy5[20];           //{        20 Bytes}
	char Dummy6[20];           //{        20 Bytes}
	char Dummy7[20];           //{        20 Bytes}
} DataLineType;


static long addToLIPPUnicodeOutput(unCH *line, long pos, unsigned char c, unCH *s) {
	long i;

	if (s == NULL) {
		line[pos++] = c;
	} else {
		for (i=0; s[i] != EOS; i++) {
			line[pos++] = s[i];
		}
	}
	return(pos);
}

static unCH *ST_Lipp(int Value, unCH *tw) {
	long wchars;

	wchars = 0L;
	if (Value > 0 && Value < 256) {
		if (Convs[Value][0] == 0xFFFF)
			;
		else if (Convs[Value][0] != 0) {
			tw[wchars++] = Convs[Value][0];
			if (Convs[Value][1] != 0) {
				tw[wchars++] = Convs[Value][1];
			}
			if (Convs[Value][0] == 0x0394) {
				uS.sprintf(tw, cl_T("(0x%x)"), Value);
				wchars = strlen(tw);
				sprintf(templineC1, "(0x%x)", Value);
				add_err(templineC1);
			}
		} else {
			uS.sprintf(tw, cl_T("(0x%x)"), Value);
			wchars = strlen(tw);
			sprintf(templineC1, "(0x%x)", Value);
			add_err(templineC1);
		}
	}
	tw[wchars] = EOS;
	return(tw);
}

static void ReadLIPPDocument(void) {
    DataLineType D;
    unsigned char *T; //       :Array[1..DataLineTypeSize] of Byte Absolute D;  { 40 Times 19 bytes / Seg }
    unsigned char Buffer[DataLineTypeSize*2];
    unsigned char MainBuffer[MainBufferSize];
    short MainBufferPointer;
    short ReadSize = MainBufferSize;
	unCH tw[128];
    long Size;
    short i, j, k, Pos;
    char TempID[7];
	long wchars;

	T = (unsigned char *)&D;
	if (fseek(fpin, 0L, SEEK_END) != 0) {
		fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
		cp2utf_exit(0);
	}
	Size = ftell(fpin);
	if (fseek(fpin, 0L, SEEK_SET) != 0) {
		fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
		cp2utf_exit(0);
	}
	if (fread(TempID, 1, 6, fpin) != 6) {
		fprintf(stderr, "Error reading data file(1): %s\n", oldfname);
		return;
	}
	TempID[6] = EOS;
	Size = Size - 6;

	if (strcmp(TempID, LIPP10) == 0) {
	} else if (strcmp(TempID, LIPP14) == 0) {
		Size = Size - 200;
		if (fread(templineC, 1, 200, fpin) != 200) {
			fprintf(stderr, "Error reading data file(2): %s\n", oldfname);
			return;
		}
	} else if (strcmp(TempID, LIPP30) == 0) {
		Size = Size - 2430; // 2400, 2430, 2435
		if (fread(templineC, 1, 2430, fpin) != 2430) {
			fprintf(stderr, "Error reading data file(2.5): %s\n", oldfname);
			return;
		}
	} else {
		fprintf(stderr, "Error reading data file(3): %s\n", oldfname);
		fprintf(stderr, "LIPP format \"%s\" is not supported\n", TempID);
		return;
	}
	wchars = 0;
	MainBufferPointer = 0;
	while (!(Size == 0 && MainBufferPointer == 0)) {
/*
		i = 0;
		Buffer[i] = 0;
		if (MainBufferPointer == 0 && Size > 0) {
			if (Size < MainBufferSize)
				ReadSize = Size;
			else
				ReadSize = MainBufferSize;
			if (fread(MainBuffer, 1, ReadSize, fpin) != ReadSize) {
				fprintf(stderr, "Error reading data file(5): %s\n", oldfname);
				return;
			}
			Size = Size - ReadSize;
			MainBufferPointer = ReadSize;
		}
		if (MainBufferPointer > 0) {
			Buffer[i] = MainBuffer[ReadSize-MainBufferPointer];
			MainBufferPointer = MainBufferPointer - 1;
		}
*/
		i = 0;
		do {
			Buffer[i] = 0;
			if (MainBufferPointer == 0 && Size > 0) {
				if (Size < MainBufferSize)
					ReadSize = Size;
				else
					ReadSize = MainBufferSize;
				if (fread(MainBuffer, 1, ReadSize, fpin) != ReadSize) {
					fprintf(stderr, "Error reading data file(5): %s\n", oldfname);
					return;
				}
				Size = Size - ReadSize;
				MainBufferPointer = ReadSize;
			}
			if (MainBufferPointer > 0) {
				Buffer[i] = MainBuffer[ReadSize-MainBufferPointer];
				MainBufferPointer = MainBufferPointer - 1;
			}
			if (i > 0) {
				if ((Buffer[i-1] == BlankCode && Buffer[i] == BlankCode) || (Size == 0 && MainBufferPointer == 0))
					break;
			}
			i++;
		} while (1) ;

		for (j=0; j < 40; j++) {
			for (k=0; k < 23; k++)
				D.Line[j][k] = 0;
		}
		for (k=0; k < 80; k++)
			D.LineComment[k] = '\0';
		Pos = 0;
		j = 0;
		while (j < i-1) {
			if (Buffer[j] == BlankCode) {
				Pos = Pos + Buffer[j+1];
				j = j+1;
			} else {
				if (Pos < 0 || Pos >= 2400) {
					fprintf(stderr, "Error reading data file(6)Pos=%d: %s\n", Pos, oldfname);
					return;
				}
				T[Pos] = Buffer[j];
				Pos = Pos + 1;
			}
			j = j + 1;
		}
		for (j=0; j < 40; j++) {
			if (D.Line[j][0] > 0)
				wchars = addToLIPPUnicodeOutput(templineW, wchars, D.Line[j][0], NULL);
			else
				wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
		}
		templineW[wchars] = EOS;
		UnicodeToUTF8(templineW, wchars, (unsigned char *)templineC, NULL, UTTLINELEN);

		for (j=0; isSpace(templineC[j]); j++) ;
		if (isalpha(templineC[j]) && isdigit(templineC[j+1]))
			j++;
		if (j > 0)
			strcpy(templineC, templineC+j);

		for (i=0; templineC[i] != EOS; i++) {
			putc(templineC[i], fpout);
		}
		putc('\n', fpout);
		wchars = 0;

//		wchars = addToLIPPUnicodeOutput(templineW, wchars, '#', NULL);
		for (j=0; j < 40; j++) {
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][2], tw));
			if (D.Line[j][2] == 0 && D.Line[j][1] != 0)
				wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][8], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][9], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][10], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][11], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][12], tw));
			if (D.Line[j][2] != 0)
				wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
		}
//		wchars = addToLIPPUnicodeOutput(templineW, wchars, '#', NULL);
//		wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
		templineW[wchars] = EOS;
		UnicodeToUTF8(templineW, wchars, (unsigned char *)templineC, NULL, UTTLINELEN);
		for (i=0; templineC[i] != EOS; i++) {
			putc(templineC[i], fpout);
		}
		putc('\n', fpout);
		wchars = 0;

//		wchars = addToLIPPUnicodeOutput(templineW, wchars, '#', NULL);
		for (j=0; j < 40; j++) {
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][1], tw));
			if (D.Line[j][1] == 0 && D.Line[j][2] != 0)
				wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][3], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][4], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][5], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][6], tw));
			wchars = addToLIPPUnicodeOutput(templineW, wchars, 0, ST_Lipp(D.Line[j][7], tw));
			if (D.Line[j][1] != 0)
				wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
		}
//		wchars = addToLIPPUnicodeOutput(templineW, wchars, '#', NULL);
//		wchars = addToLIPPUnicodeOutput(templineW, wchars, ' ', NULL);
		templineW[wchars] = EOS;
		UnicodeToUTF8(templineW, wchars, (unsigned char *)templineC, NULL, UTTLINELEN);
		for (i=0; templineC[i] != EOS; i++) {
			putc(templineC[i], fpout);
		}
		putc('\n', fpout);
		wchars = 0;
	}
	templineW[wchars] = EOS;
	UnicodeToUTF8(templineW, wchars, (unsigned char *)templineC, NULL, UTTLINELEN);
	for (i=0; templineC[i] != EOS; i++) {
		if (templineC[i] == '\n') {
			putc('\n', fpout);
		} else
			putc(templineC[i], fpout);
	}
}
/* {===================== LIPP Document Reading End ========================} */

static char isCAFile(FNType *fname) {
	register int j;
	unCH ext[3];
	
	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);
	
	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'C' && ext[1] == 'A' && ext[2] == EOS)
			return('\001');
	}
	return(FALSE);
}

void call() {
	char  tStout, isCRFound;
	long  i, j;
	short isIPA, isCA;
	FNType *s;

	if (chatmode != 0) {
		s = strchr(oldfname, '.');
		if (s == NULL) {
			chatmode = 0;
		} else if (SetChatModeOfDatafileExt(oldfname, TRUE) && uS.mStricmp(s, ".cut") && uS.mStricmp(s, ".cdc")) {
			chatmode = 4;
		} else {
			chatmode = 0;
		}
	}
	isErrFound = 0L;
	if (isBOM) {
		fputc(0xef, fpout);
		fputc(0xbb, fpout);
		fputc(0xbf, fpout);
	}
//	fprintf(fpout, "%c%c%c", 0xef, 0xbb, 0xbf); // BOM UTF8
	if (!isMakeTXT)
		fprintf(fpout, "%s\n", UTF8HEADER);
	if (isCAFile(oldfname)) {
#ifdef _WIN32 
		fprintf(fpout, "%s	Win95:CAfont:-15:0", FONTHEADER);
#else
		fprintf(fpout, "%s	CAfont:13:7", FONTHEADER);
#endif
		putc('\n', fpout);
	}
	if (!feof(fpin)) {
		int  c;

		c = getc(fpin);
		if (c == (int)0xFF || c == (int)0xFE) {
			if (c == (int)0xFF)
				lEncode = UPC;
			else
				lEncode = UMAC;
			lEncodeSet = UTF16;
		}
#if defined(UNX)
		if (!stin)
#endif
			rewind(fpin);
	}

	if (isLIPP && lEncodeSet != UTF16) {
		fclose(fpin);
		if ((fpin=fopen(oldfname, "rb")) == NULL) {
			fprintf(stderr,"Can't open file %s.\n",oldfname);
			return;
		}
		ReadLIPPDocument(); //  LIPPoUTF8((unsigned char *)templineC4);
		return;
	}
	if (isMakeTXT) {
		if (!isBOM) {
			fputc(0xef, fpout);
			fputc(0xbb, fpout);
			fputc(0xbf, fpout); // BOM UTF8
		}
		while (fgets_cr(templineC, UTTLINELEN, fpin)) {
			if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
				continue;
			if (templineC[0] == (char)0xef && templineC[0] == (char)0xbb && templineC[0] == (char)0xbf)
				strcpy(templineC, templineC+3);
			if (isRemoveBullets) {
				for (i=0L; templineC[i] != EOS; i++) {
					if (templineC[i] == HIDEN_C) {
						for (j=i+1; templineC[j] != HIDEN_C && templineC[j] != EOS; j++) ;
						if (templineC[j] != EOS)
							strcpy(templineC+i, templineC+j+1);
						i = j - 1;
					}
				}
			}
			fprintf(fpout, "%s", templineC);
		}		
		return;
	}
	if (lEncodeSet == UTF16) {
		char isNLFound = FALSE;

		fclose(fpin);
		if ((fpin=fopen(oldfname, "rb")) == NULL) {
			fprintf(stderr,"Can't open file %s.\n",oldfname);
			return;
		}
		getc(fpin);
		if (!feof(fpin))
			getc(fpin);
		i = 0L;

#ifdef _MAC_CODE
		if (byteOrder == CFByteOrderLittleEndian) {
			if (lEncode == UPC)
				lEncode = UMAC;
			else 
				lEncode = UPC;
		}
#endif

		while (!feof(fpin)) {
#ifdef _WIN32 
			if (lEncode == UPC) {
				templineC[i] = getc(fpin);
				if (feof(fpin))
					break;
				templineC[i+1] = getc(fpin);
			} else {
				templineC[i+1] = getc(fpin);
				if (feof(fpin))
					break;
				templineC[i] = getc(fpin);
			}
#else
			if (lEncode == UPC) {
				templineC[i+1] = getc(fpin);
				if (feof(fpin))
					break;
				templineC[i] = getc(fpin);
			} else {
				templineC[i] = getc(fpin);
				if (feof(fpin))
					break;
				templineC[i+1] = getc(fpin);
			}
#endif
			if ((templineC[i] == '\r' && templineC[i+1] == EOS) ||
				(templineC[i] == '\n' && templineC[i+1] == EOS) ||
				(templineC[i] == EOS  && templineC[i+1] == '\r') ||
				(templineC[i] == EOS  && templineC[i+1] == '\n')) {
				if (isNLFound) {
					isNLFound = FALSE;
					i -= 2;
				} else
					isNLFound = TRUE;
			} else
				isNLFound = FALSE;
			i += 2;
			if (i > UTTLINELEN-10) {
				templineC[i] = EOS;
				templineC[i+1] = EOS;
				UnicodeToUTF8((unCH *)templineC, i/2, (unsigned char *)templineC4, NULL, UTTLINELEN);
				fputs(templineC4, fpout);
				i = 0;
			}
		}
		if (i > 0) {
			templineC[i] = EOS;
			templineC[i+1] = EOS;
			UnicodeToUTF8((unCH *)templineC, i/2, (unsigned char *)templineC4, NULL, UTTLINELEN);
			fputs(templineC4, fpout);
		}
		return;
	}		
	dFnt.isUTF = 0;
	isIPA = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	tStout = stout;
	stout = TRUE;
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (strchr(utterance->line, '\n') == NULL)
				strcat(utterance->line, "\n");
		}
		stout = tStout;
		if (dFnt.isUTF && !isUnibet && !isOnlyTier) {
			int c;

//			fprintf(stderr, "\n** FILE \"%s\" IS ALREADY IN UTF8 FORMAT!!\n\n", oldfname);
			putc(currentchar, fpout);
			while ((c=getc(fpin)) != EOF) {
				putc(c, fpout);
			}
			isUTFFound = TRUE;
			return;
		}
		if (lEncodeSet)
			dFnt.orgEncod = lEncode;
		if (dFnt.orgFType == MACipa || dFnt.orgFType == WIN95ipa)
			isIPA = 1;
		else
			isIPA = 0;
		if (dFnt.orgFType == MACcain || dFnt.orgFType == WIN95cain)
			isCA = dFnt.orgFType;
		else
			isCA = 0;
		if (chatmode == 0) {
			if (uS.partcmp(utterance->line, FONTHEADER, FALSE, FALSE))
				continue;
			else if (dFnt.isUTF && uS.isUTF8(utterance->line))
				continue;
		} else {
			if (uS.partcmp(utterance->speaker, FONTHEADER, FALSE, FALSE))
				continue;
			else if (dFnt.isUTF && uS.isUTF8(utterance->speaker))
				continue;
		}
		for (i=0L; utterance->line[i]; i++) {
			if (uS.partwcmp(utterance->line+i, FONTMARKER)) {
				for (j=i; utterance->line[j] && !uS.isRightChar(utterance->line, j, ']', &dFnt, MBF); j++) ;
				if (utterance->line[j]) {
					j++;
					while (isSpace(utterance->line[j]))
						j++;
				}
				if (i < j)
					att_cp(0, utterance->line+i, utterance->line+j, utterance->attLine+i, utterance->attLine+j);
			}
		}
		if (dFnt.isUTF) {
			if (isCA && utterance->speaker[0] == '*')
				i = 1L;
			else
				i = 0L;
			for (; utterance->speaker[i]; i++)
				putc(utterance->speaker[i], fpout);
		} else {
			if (isCA && utterance->speaker[0] == '*')
				AsciiToUnicodeToUTF8(utterance->speaker+1);
			else
				AsciiToUnicodeToUTF8(utterance->speaker);
		}
		if (isUnibet) {
			UnibetToUTF8((unsigned char *)utterance->line, utterance->attLine);
		} else if (isCA) {
			CAToUTF8((unsigned char *)utterance->line, utterance->attLine, isCA);
		} else if ((isCUsed && chatmode == 0) || isIPA || (isCUsed && chatmode && checktier(utterance->speaker) && (!nomain || utterance->speaker[0] != '*'))) {
			IPAToUTF8((unsigned char *)utterance->line);
		} else if (isOnlyTier) {
			isCRFound = FALSE;
			for (i=0L; utterance->line[i] != EOS; i++) {
				putc(utterance->line[i], fpout);
				if (utterance->line[i] == '\n')
					isCRFound = TRUE;
				else
					isCRFound = FALSE;
			}
			if (!isCRFound)
				putc('\n', fpout);
		} else {
			AsciiToUnicodeToUTF8(utterance->line);
		}
		tStout = stout;
		stout = TRUE;
	}
	stout = tStout;
	if (isErrFound > 0L)
		fprintf(stderr, "*** %ld bad character(s).\n*** Please search for \"(0x\" in this file.\n", isErrFound);
}
