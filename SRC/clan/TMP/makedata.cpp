#define CHAT_MODE 4
#include "cu.h"
#ifdef UNX
#include "fontconvert.h"
#include "c_curses.h"
#else
#include "ced.h"
#endif

#if defined(_WIN32)
#include <direct.h>
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main makedata_main
#define call makedata_call
#define getflag makedata_getflag
#define init makedata_init
#define usage makedata_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define LUTTLINELEN  UTTLINELEN

extern char AcceptAllFiles;
extern char isRecursive;
extern char OutputDrive;
extern struct tier *defheadtier;

char  MkUnix, MkMac, MkDos, MkW95, insertFontHeader;
char  *uline = NULL, *wline = NULL, *mline = NULL, *dline = NULL;
char  *attwline = NULL, *attmline = NULL;
char  FontName[BUFSIZ];
char  makedata_start_path[2048];
char  *makedata_arg;
char  RealOverWrite;
char  localAcceptAllFiles;
char  *md_defFontName;
char  *defFontPref;
int   defCharsSet;
long  md_defFontSize;
long  ASCIIAdjustment;
short defPlatform, defFontType;
FILE *ufp, *wfp, *dfp, *mfp;


void Translate(void);
void ASCIIToDos850(char *st);
void ASCIIToDos860(char *st);
void ASCIIToMonaco(char *st);
FILE *MAKEDATA_OpenFiles(char *fn, char *ext);


void init(char f) {
	if (f) {
		if (uline == NULL) {
			if ((uline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				cutt_exit(0);
			}
		}
		if (wline == NULL) {
			if ((wline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				free(uline);
				uline = NULL;
				cutt_exit(0);
			}
		}
		if (attwline == NULL) {
			if ((attwline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				free(uline);
				free(wline);
				uline = NULL;
				wline = NULL;
				cutt_exit(0);
			}
		}
		if (mline == NULL) {
			if ((mline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				free(wline);
				free(attwline);
				free(uline);
				uline = NULL;
				wline = NULL;
				attwline = NULL;
				cutt_exit(0);
			}
		}
		if (attmline == NULL) {
			if ((attmline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				free(wline);
				free(attwline);
				free(mline);
				free(uline);
				uline = NULL;
				wline = NULL;
				attwline = NULL;
				mline = NULL;
				cutt_exit(0);
			}
		}
		if (dline == NULL) {
			if ((dline=(char *)malloc(LUTTLINELEN)) == NULL) {
				fputs("ERROR: Out of memory.\n",stderr);
				free(wline);
				free(attwline);
				free(mline);
				free(attmline);
				free(uline);
				uline = NULL;
				wline = NULL;
				attwline = NULL;
				mline = NULL;
				attmline = NULL;
				cutt_exit(0);
			}
		}
		FilterTier = 0;
		LocalTierSelect = TRUE;
		isRecursive = TRUE;
		if (localAcceptAllFiles)
			AcceptAllFiles = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		insertFontHeader = FALSE;
		combinput = TRUE;
		RealOverWrite = FALSE;
		makedata_arg = "*.CHA";
		md_defFontName = "";
		defFontPref = "";
		md_defFontSize = 0;
		defCharsSet = 0;
		defPlatform = NOCHANGE;
		defFontType = 0;
		MkUnix  = FALSE;
		MkMac   = FALSE;
		MkDos   = FALSE;
		MkW95   = FALSE;
	} else {
		if (!MkUnix && !MkMac && !MkDos && !MkW95) {
			fputs("Please specify the destination platform.\n",stderr);
			cutt_exit(0);
		}
		if (isRecursive) {
			strcpy(templineC, oldfname);
			uS.uppercasestr(templineC, &dFnt, C_MBF);
			if (uS.fpatmat(templineC, makedata_arg) || uS.fpatmat(templineC, "*.MOR") || uS.fpatmat(templineC, "*.CA"))
				chatmode = CHAT_MODE;
			else if (uS.fpatmat(templineC, "*.CEX") && uS.SetChatModeOfDatafileExt(templineC, TRUE))
				chatmode = CHAT_MODE;
			else
				chatmode = 0;
		}
	}
}

void usage() {
	puts("Set working directory to the folder of files in need of conversion.");
	puts("Example: makedata +m");
	puts("MAKEDATA Convert data of one system to data of other systems");
	printf("Usage: makedata [b d h m oC u w %s] filename(s)\n",mainflgs());
	puts("+b : Override font in original CHAT files with font specified by +o option.");
	puts("+d : Create data for DOS only.");
	puts("+h : Insert font header into Text, i.e. non-Chat, file");
	puts("+m : Create data for MAC only.");
	puts("+u : Create data for UNIX only.");
	puts("+w : Create data for Win95/98/2000 only.");
//12-4-99
//	puts("+cS: Specifies the extension of CHAT files (default: *.cha).");
	puts("+oC: Define font if original data file is missing \"@Font\" header.");
	puts("    d- DOS (850), p- DOS (860),");
	puts("    m- MAC (Monaco), c- MAC (Courier), o- MAC (Osaka),");
	puts("    w- Win95/98/2000 (Courier).");
	mainusage();
}

static void fatalError(char isError) {
	if (uline != NULL) free(uline);
	if (wline != NULL) free(wline);
	if (attwline != NULL) free(attwline);
	if (mline != NULL) free(mline);
	if (attmline != NULL) free(attmline);
	if (dline != NULL) free(dline);
	uline = NULL;
	wline = NULL;
	attwline = NULL;
	mline = NULL;
	attmline = NULL;
	dline = NULL;
	if (isError) {
		if (dfp) fclose(dfp);
		if (ufp) fclose(ufp);
		if (wfp) fclose(wfp);
		if (mfp) fclose(mfp);
		cutt_exit(0);
	}
}

FILE *MAKEDATA_OpenFiles(char *fn, char *ext) {
	FILE *fp;

	if (!isRecursive)
		parsfname(oldfname, fn, ext);

	if ((fp=fopen(fn,"wb")) == NULL) {
		fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",fn);
	} else { 
#if defined(_MAC_CODE)
		if (!isRecursive)
			settyp(fn, 'TEXT', the_file_creator.out, FALSE);
#endif /* defined(_MAC_CODE) */
	}
	return(fp);
}

static void makeAllDir(char *path, int len) {
#if defined(_MAC_CODE) || defined(_WIN32)
	char *new_part;

	new_part = path;
	while (new_part = strchr(new_part,PATHDELIMCHR)) {
		*new_part = EOS;
		if (strlen(path) > len) {
#if defined(_MAC_CODE)
			mkdir(path, 0);
#else
			mkdir(path);
#endif
		}
		*new_part = PATHDELIMCHR;
		new_part++;
	}
#endif /* defined(_MAC_CODE) */
}

void call(void) {
	int c, len;
	char tfname[1024];
	char isUTF8SymFound;
#ifndef UNX
	char tIsUnixCRs = isUnixCRs;
#endif

#if defined(_MAC_CODE)
	creator_type makedata_file_type, makedata_file_creator;
#endif /* defined(_MAC_CODE) */
	if (chatmode) {
		if (!chattest(oldfname, TRUE, &isUTF8SymFound)) {
			chatmode = 0;
			fprintf(fpout, "  *");
		}
		rewind(fpin);
		if (isUTF8SymFound) {
			getc(fpin); getc(fpin); getc(fpin);
		}
	}
	ufp = wfp = mfp = dfp = NULL;
	if (isRecursive) {
#if defined(_MAC_CODE)
		gettyp(oldfname, &wd_ref, &makedata_file_type.out, &makedata_file_creator.out);
		strcpy(templineC, oldfname);
		uS.uppercasestr(templineC, &dFnt, C_MBF);
		if (uS.SetChatModeOfDatafileExt(templineC, TRUE) || uS.fpatmat(templineC, "*.CUT") || uS.fpatmat(templineC, "*.CDC")) {
			makedata_file_type.out = 'TEXT';
		}
		if (makedata_file_type.out != 'TEXT') {
			fclose(fpin);
			if ((fpin=fopen(oldfname, "rb")) == NULL) {
				fprintf(stderr,"Can't open file %s.\n",oldfname);
				return;
			}
		}
#endif /* defined(_MAC_CODE) */
		strcpy(tfname, oldfname);
		len = strlen(makedata_start_path) - 1;
		uS.shiftright(tfname+len, 4);
	}
	if (MkUnix) {
		if (isRecursive) {
			tfname[len] = '.';
			tfname[len+1] = 'U';
			tfname[len+2] = 'N';
			tfname[len+3] = 'X';
			makeAllDir(tfname, len);
			unlink(tfname);
		}
		if ((ufp=MAKEDATA_OpenFiles(tfname, ".unx")) == NULL)
			return;
#if defined(_MAC_CODE)
		if (isRecursive)
			settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
#endif /* defined(_MAC_CODE) */
	}
	if (MkW95) {
		if (isRecursive) {
			tfname[len] = '.';
			tfname[len+1] = 'W';
			tfname[len+2] = 'I';
			tfname[len+3] = 'N';
			makeAllDir(tfname, len);
			unlink(tfname);
		}
		if ((wfp=MAKEDATA_OpenFiles(tfname, ".win")) == NULL) {
			if (ufp) fclose(ufp);
			return;
		}
#if defined(_MAC_CODE)
		if (isRecursive) {
			strcpy(templineC, oldfname);
			uS.uppercasestr(templineC, &dFnt, C_MBF);
			if (!uS.SetChatModeOfDatafileExt(templineC, TRUE) && !uS.fpatmat(templineC, "*.CUT") && 
										 					 !uS.fpatmat(templineC, "*.CDC"))
				settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
			else if (makedata_file_type.out == 'TEXT')
				settyp(tfname, 'TEXT', PROGCREATOR, FALSE);
			else
				settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
		}
#endif /* defined(_MAC_CODE) */
	}
	if (MkMac) {
		if (isRecursive) {
			tfname[len] = '.';
			tfname[len+1] = 'M';
			tfname[len+2] = 'A';
			tfname[len+3] = 'C';
			makeAllDir(tfname, len);
			unlink(tfname);
		}
		if ((mfp=MAKEDATA_OpenFiles(tfname, ".mac")) == NULL) {
			if (ufp) fclose(ufp);
			if (wfp) fclose(wfp);
			return;
		}
#if defined(_MAC_CODE)
		if (isRecursive) {
			strcpy(templineC, oldfname);
			uS.uppercasestr(templineC, &dFnt, C_MBF);
			if (!uS.SetChatModeOfDatafileExt(templineC, TRUE) && !uS.fpatmat(templineC, "*.CUT") && 
										 					 !uS.fpatmat(templineC, "*.CDC"))
				settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
			else if (makedata_file_type.out == 'TEXT')
				settyp(tfname, 'TEXT', PROGCREATOR, FALSE);
			else
				settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
		}
#endif /* defined(_MAC_CODE) */
	}
	if (MkDos) {
		if (isRecursive) {
			tfname[len] = '.';
			tfname[len+1] = 'W';
			tfname[len+2] = 'I';
			tfname[len+3] = 'N';
			makeAllDir(tfname, len);
			unlink(tfname);
		}
		if ((dfp=MAKEDATA_OpenFiles(tfname, ".dos")) == NULL) {
			if (ufp) fclose(ufp);
			if (wfp) fclose(wfp);
			if (mfp) fclose(mfp);
			return;
		}
#if defined(_MAC_CODE)
		if (isRecursive)
			settyp(tfname, makedata_file_type.out, makedata_file_creator.out, FALSE);
#endif /* defined(_MAC_CODE) */
	}

#ifndef UNX
	isUnixCRs = FALSE;
#endif
	if (isRecursive) {
		strcpy(templineC, oldfname);
		uS.uppercasestr(templineC, &dFnt, C_MBF);
#if defined(_MAC_CODE)
		if (makedata_file_type.out != 'TEXT')
#else
		if (!uS.SetChatModeOfDatafileExt(templineC, TRUE) && !uS.fpatmat(templineC, "*.CUT") && 
									 					 !uS.fpatmat(templineC, "*.CDC"))
#endif
										 {
			while ((c=getc(fpin)) != EOF) {
				if (MkUnix)
					putc(c, ufp);
				if (MkW95)
					putc(c, wfp);
				if (MkMac)
					putc(c, mfp);
				if (MkDos)
					putc(c, dfp);
			}
			fprintf(fpout, "  Binary Transfer ");
			if (MkUnix) fprintf(fpout, "to UNIX; ");
			if (MkW95)  fprintf(fpout, "to Win95/98/2000; ");
			if (MkMac)  fprintf(fpout, "to MAC; ");
			if (MkDos)  fprintf(fpout, "to DOS; ");
			fprintf(fpout, "\n");
		} else {
			Translate();
		}
	} else
		Translate();
#ifndef UNX
	isUnixCRs = tIsUnixCRs;
#endif
	if (MkUnix) fclose(ufp);
	if (MkW95) fclose(wfp);
	if (MkMac) fclose(mfp);
	if (MkDos) fclose(dfp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  i;
	char foundFile = FALSE;

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MAKEDATA;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	localAcceptAllFiles = TRUE;
	if (argc < 2 && !stin) {
		usage();
	} else {
		for (i=1; i < argc; i++) {
			if (*argv[i] == '+'  || *argv[i] == '-') {
				if (argv[i][1] == 'r' && argv[i][2] == 'e') {
					localAcceptAllFiles = TRUE;
				}
			} else
				foundFile = TRUE;
		}
		if (!foundFile && localAcceptAllFiles && argc < MAX_ARGS)
			argv[argc++] = "*";

#if defined(_MAC_CODE)
		createPathStr(makedata_start_path, &wd_ref, nil);
		for (i=0; makedata_start_path[i] != PATHDELIMCHR; i++) ;
		if (makedata_start_path[i] != PATHDELIMCHR || makedata_start_path[i+1] == EOS) {
			fprintf(stderr,"    Can't create directory at top level.");
			return;
		}
#elif defined(_WIN32)
		strcpy(makedata_start_path, wd_st_full);
#elif defined(UNX)
		getcwd(makedata_start_path, 2048);
#endif /* defined(_MAC_CODE) */
		bmain(argc,argv,NULL);
		fatalError(FALSE);
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'b':
			no_arg_option(f);
			RealOverWrite = TRUE;
			break;
/* 12-4-99
		case 'c':
			makedata_arg = f;
			uS.uppercasestr(makedata_arg, &dFnt, C_MBF);
			break;
*/
		case 'h':
			no_arg_option(f);
			insertFontHeader = TRUE;
			break;
		case 'o':
			if (*f == 'd' || *f == 'D') {
				md_defFontName = "RealDosCodePage";
				defFontPref = "";
				md_defFontSize = 850;
				defCharsSet = 1;
				defPlatform = DOSDATA;
				defFontType = DOS850;
			} else if (*f == 'p' || *f == 'P') {
				md_defFontName = "RealDosCodePage";
				defFontPref = "";
				md_defFontSize = 860;
				defCharsSet = 1;
				defPlatform = DOSDATA;
				defFontType = DOS860;
			} else if (*f == 'c' || *f == 'C') {
				md_defFontName = "Courier";
				defFontPref = "";
				md_defFontSize = 10;
				defCharsSet = 0;
				defPlatform = MACDATA;
				defFontType = COURIER;
			} else if (*f == 'm' || *f == 'M') {
				md_defFontName = "Monaco";
				defFontPref = "";
				md_defFontSize = 9;
				defCharsSet = 0;
				defPlatform = MACDATA;
				defFontType = MONACO;
			} else if (*f == 'o' || *f == 'O') {
				md_defFontName = "Osaka";
				defFontPref = "";
				md_defFontSize = 9;
				defCharsSet = 1;
				defPlatform = MACDATA;
				defFontType = MACjpn;
			} else if (*f == 'w' || *f == 'W') {
				md_defFontName = "Courier";
				defFontPref = "Win95:";
				md_defFontSize = -13;
				defCharsSet = 0;
				defPlatform = WIN95DATA;
				defFontType = WIN95cour;
			} else {
				fprintf(stderr,
"Choose either letter 'd', 'p', 'c', 'm' or 'w' as an argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'u':
			no_arg_option(f);
			MkUnix  = TRUE;
			MkMac   = FALSE;
			MkDos   = FALSE;
			MkW95   = FALSE;
			break;
		case 'm':
			no_arg_option(f);
			MkMac   = TRUE;
			MkUnix  = FALSE;
			MkDos   = FALSE;
			MkW95   = FALSE;
			break;
		case 'd':
			no_arg_option(f);
			MkDos   = TRUE;
			MkUnix  = FALSE;
			MkMac   = FALSE;
			MkW95   = FALSE;
			break;
		case 'w':
			no_arg_option(f);
			MkW95   = TRUE;
			MkUnix  = FALSE;
			MkMac   = FALSE;
			MkDos   = FALSE;
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

/**************************************************************/
/*	 printout in chat format									  */
static void makedata_ActualPrint(char *st, char *att, char *oldAtt, char justFirstAtt, char needCR, FILE *fp, char dlm, char dlm2) {
	char lastCR = FALSE;

	while (*st) {
		if (att != NULL)
			printAtts(*att, *oldAtt, fp);
		else
			printAtts(0, *oldAtt, fp);
		if (*st == '\n') {
			lastCR = TRUE;
			putc(dlm,fp);
			if (dlm2 != '\0')
				putc(dlm2,fp);
		} else {
			lastCR = FALSE;
			putc(*st,fp);
		}
		st++;
		if (att != NULL) {
			*oldAtt = *att;
			if (!justFirstAtt)
				att++;
		} else
			*oldAtt = 0;
	}
	if (!lastCR && needCR) {
		printAtts(0, *oldAtt, fp);
		putc(dlm,fp);
		if (dlm2 != '\0')
			putc(dlm2,fp);
	}
}

/* printout(sp,line) displays tier properly indented according to the CHAT
   format. "sp" points to the code identification part, "line" points to
   the text line.
*/
static void makedata_printout(FILE *fp, char *sp, char *line, char *attSp, char *attLine, char dlm, char dlm2, char force) {
	register char oldAtt;

	if (!chatmode && !force) {
		sp = NULL;
		attSp = NULL;
	}
	oldAtt = 0;
	if (isCainFile) {
		if (sp != NULL && *sp == '*') {
			strcpy(sp, sp+1);
			if (attSp != NULL)
				strcpy(attSp, attSp+1);
		}
	}
	if (sp != NULL && *sp != EOS) {
		makedata_ActualPrint(sp, attSp, &oldAtt, FALSE, FALSE, fp, dlm, dlm2);
		if (!isSpace(sp[strlen(sp)-1]) && line != NULL && *line != EOS && *line != '\n')
			putc('\t',fp);
	}
	if (line != NULL)
		makedata_ActualPrint(line, attLine, &oldAtt, FALSE, (chatmode || force), fp, dlm, dlm2);
}

static char FindNewFont(char *st, char ec, char *att, NewFontInfo *orgFInfo) {
	register int i;
	char *e;
	NewFontInfo tfinfo;
	
	if (ec != EOS) {
		for (i=0; st[i] && st[i] != ':' && st[i] != ec; i++) ;
		if (st[i] != ':')
			return(FALSE);
		i++;
	} else {
		if ((!chatmode || isCainFile) && strncmp(st, FONTHEADER, 6) == 0) {
			for (i=6; isSpace(st[i]); i++) ;
		} else
			i = 0;
	}
	if ((e=GetDatasFont(st+i, ec, &dFnt, dFnt.isUTF, &tfinfo)) == NULL)
		return(FALSE);

	if (att == NULL)
		strcpy(st, e);
	else
		att_cp(0, st, e, att, att+(e-st));
	if (!RealOverWrite) {
		strcpy(orgFInfo->fontName, tfinfo.fontName);
		orgFInfo->fontPref = tfinfo.fontPref;
		orgFInfo->fontSize = tfinfo.fontSize;
		orgFInfo->CharSet  = tfinfo.CharSet;
		orgFInfo->platform = tfinfo.platform;
		orgFInfo->fontType = tfinfo.fontType;
		orgFInfo->orgFType = tfinfo.orgFType;
		orgFInfo->fontId   = tfinfo.fontId;
		orgFInfo->isUTF    = tfinfo.isUTF;
		orgFInfo->orgEncod = tfinfo.orgEncod;
		orgFInfo->fontTable = tfinfo.fontTable;
	}
	return(TRUE);
}

static void FindTransTable(NewFontInfo *thisFInfo, NewFontInfo *orgFInfo, short toPlatform) {
	copyNewFontInfo(thisFInfo, orgFInfo);
	FindTTable(thisFInfo, toPlatform);
}

static char isCainFont(char *buf) {
	register int i;

	if (isCainFile && strncmp(buf, "@Font:", 6) == 0) {
		for (i=6; isSpace(buf[i]); i++) ;
		if (strncmp(buf+i, "Win95:CA", 8) == 0) {
			return(TRUE);
		} else if (strncmp(buf+i, "CA", 2) == 0) {
			return(TRUE);
		}
	}
	return(FALSE);
}

void Translate(void) {
	char ft;
	long i, j, uch, wch, mch, dch;
	long uindx, windx, mindx, dindx;
	NewFontInfo ufinfo, dfinfo, mfinfo, wfinfo, orgFInfo;

	ft = TRUE;
	*utterance->speaker = EOS;
	if (!isRecursive)
		fprintf(stderr, "\tTranslating file ..."); fflush(stderr);

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	ASCIIAdjustment = 0L;
	uch = wch = mch = dch = 0L;
	strcpy(orgFInfo.fontName, md_defFontName);
	orgFInfo.fontPref = defFontPref;
	orgFInfo.fontSize = md_defFontSize;
	orgFInfo.orgFType = NOCHANGE;
	orgFInfo.CharSet = defCharsSet;
	orgFInfo.platform = defPlatform;
	orgFInfo.fontType = defFontType;
	orgFInfo.fontTable = NULL;
	ufinfo.fontTable = NULL;
	wfinfo.fontTable = NULL;
	mfinfo.fontTable = NULL;
	dfinfo.fontTable = NULL;
	while (getwholeutter()) {
		if ((chatmode && uS.partcmp(utterance->speaker, FONTHEADER, FALSE, FALSE)) ||
			(!chatmode && uS.partcmp(utterance->line, FONTHEADER, FALSE, FALSE))   || isCainFont(utterance->line)) {
			if (!dFnt.isUTF && FindNewFont(utterance->line, EOS, NULL, &orgFInfo)) {
				if (MkUnix) {
					FindTransTable(&ufinfo, &orgFInfo, UNIXDATA);
					if (!strcmp(ufinfo.fontPref, "Win95:") && ufinfo.CharSet >= 2)
						sprintf(uline, "%s%s:%ld:%d", ufinfo.fontPref, ufinfo.fontName, ufinfo.fontSize,(int)ufinfo.CharSet);
					else
						sprintf(uline, "%s%s:%ld", ufinfo.fontPref, ufinfo.fontName, ufinfo.fontSize);
					makedata_printout(ufp, FONTHEADER, uline, NULL, NULL,'\n', '\0', TRUE);
				}
				if (MkW95) {
					FindTransTable(&wfinfo, &orgFInfo, WIN95DATA);
					sprintf(wline, "%s%s:%ld:%d", wfinfo.fontPref, wfinfo.fontName, wfinfo.fontSize, (int)wfinfo.CharSet);
					makedata_printout(wfp, FONTHEADER, wline, NULL, NULL, '\r', '\n', TRUE);
				}
				if (MkMac) {
					FindTransTable(&mfinfo, &orgFInfo, MACDATA);
					sprintf(mline, "%s%s:%ld:%d", mfinfo.fontPref, mfinfo.fontName, mfinfo.fontSize, (int)mfinfo.CharSet);
					makedata_printout(mfp, FONTHEADER, mline, NULL, NULL, '\r', '\0', TRUE);
				}
				if (MkDos) {
					FindTransTable(&dfinfo, &orgFInfo, DOSDATA);
					if (!strcmp(dfinfo.fontPref, "Win95:") && dfinfo.CharSet >= 2)
						sprintf(dline, "%s%s:%ld:%d", dfinfo.fontPref, dfinfo.fontName, dfinfo.fontSize,(int)dfinfo.CharSet);
					else
						sprintf(dline, "%s%s:%ld", dfinfo.fontPref, dfinfo.fontName, dfinfo.fontSize);
					makedata_printout(dfp, FONTHEADER, dline, NULL, NULL, '\r', '\n', TRUE);
				}
				ft = FALSE;
				continue;
			}
		}
		
		if (ft) {
			if (!dFnt.isUTF && defPlatform == NOCHANGE) {
				fprintf(stderr,"File \"%s\" is missing \"@Font\" header.\n", oldfname);
				fprintf(stderr,"Please specify font for non CHAT files with +o option.\n");
				fprintf(stderr,"If it has \"@Font\" header then it is not recognized as CHAT file.\n");
				fprintf(stderr,"Please change it's extension to \".cha\".\n");
				fatalError(TRUE);
			}
			ft = FALSE;
			if (!dFnt.isUTF) {
				if (MkUnix) {
					FindTransTable(&ufinfo, &orgFInfo, UNIXDATA);
					if (chatmode || insertFontHeader) {
						if (!strcmp(ufinfo.fontPref, "Win95:") && ufinfo.CharSet >= 2)
							sprintf(uline, "%s%s:%ld:%d", ufinfo.fontPref, ufinfo.fontName, ufinfo.fontSize,(int)ufinfo.CharSet);
						else
							sprintf(uline, "%s%s:%ld", ufinfo.fontPref, ufinfo.fontName, ufinfo.fontSize);
						makedata_printout(ufp, FONTHEADER, uline, NULL, NULL, '\n', '\0', TRUE);
					}
				}
				if (MkW95) {
					FindTransTable(&wfinfo, &orgFInfo, WIN95DATA);
					if (chatmode || insertFontHeader) {
						sprintf(wline, "%s%s:%ld:%d", wfinfo.fontPref, wfinfo.fontName, wfinfo.fontSize, (int)wfinfo.CharSet);
						makedata_printout(wfp, FONTHEADER, wline, NULL, NULL, '\r', '\n', TRUE);
					}
				}
				if (MkMac) {
					FindTransTable(&mfinfo, &orgFInfo, MACDATA);
					if (chatmode || insertFontHeader) {
						sprintf(mline, "%s%s:%ld:%d", mfinfo.fontPref, mfinfo.fontName, mfinfo.fontSize,(int)mfinfo.CharSet);
						makedata_printout(mfp, FONTHEADER, mline, NULL, NULL, '\r', '\0', TRUE);
					}
				}
				if (MkDos) {
					FindTransTable(&dfinfo, &orgFInfo, DOSDATA);
					if (chatmode || insertFontHeader) {
						if (!strcmp(dfinfo.fontPref, "Win95:") && dfinfo.CharSet >= 2)
							sprintf(dline, "%s%s:%ld:%d", dfinfo.fontPref, dfinfo.fontName, dfinfo.fontSize,(int)dfinfo.CharSet);
						else
							sprintf(dline, "%s%s:%ld", dfinfo.fontPref, dfinfo.fontName, dfinfo.fontSize);
						makedata_printout(dfp, FONTHEADER, dline, NULL, NULL, '\r', '\n', TRUE);
					}
				}
			}
		}

		*uline = EOS; *mline = EOS; *dline = EOS;
		uindx = 0L; windx = 0L; mindx = 0L; dindx = 0L;
		for (i=0; utterance->line[i]; i++) {
			if (!dFnt.isUTF && chatmode && uS.partwcmp(utterance->line+i, FONTMARKER)) {
				if (FindNewFont(utterance->line+i, ']', utterance->attLine+i, &orgFInfo)) {
					if (MkUnix) {
						FindTransTable(&ufinfo, &orgFInfo, UNIXDATA);
						sprintf(uline+uindx, "%s%s%s:%ld] ", FONTMARKER, ufinfo.fontPref, ufinfo.fontName, ufinfo.fontSize);
						uindx += strlen(uline+uindx);
					}
					if (MkW95) {
						FindTransTable(&wfinfo, &orgFInfo, WIN95DATA);
						sprintf(wline+windx, "%s%s%s:%ld:%d] ", FONTMARKER, wfinfo.fontPref, wfinfo.fontName, wfinfo.fontSize, (int)wfinfo.CharSet);
						j = windx + strlen(wline+windx);
						for (; windx < j; windx++)
							attwline[windx] = utterance->attLine[i];
					}
					if (MkMac) {
						FindTransTable(&mfinfo, &orgFInfo, MACDATA);
						sprintf(mline+mindx, "%s%s%s:%ld:%d] ", FONTMARKER, mfinfo.fontPref, mfinfo.fontName, mfinfo.fontSize, (int)mfinfo.CharSet);
						j = mindx + strlen(mline+mindx);
						for (; mindx < j; mindx++)
							attmline[mindx] = utterance->attLine[i];
					}
					if (MkDos) {
						FindTransTable(&dfinfo, &orgFInfo, DOSDATA);
						sprintf(dline+dindx, "%s%s%s:%ld] ", FONTMARKER, dfinfo.fontPref, dfinfo.fontName, dfinfo.fontSize);
						dindx += strlen(dline+dindx);
					}
					if (utterance->line[i] == EOS)
						break;
				}
			}
/*
			if (utterance->line[i] == '^') {
				if ((orgFInfo.platform == UNIXDATA || orgFInfo.platform == DOSDATA ||
					 orgFInfo.platform == DOS850   || orgFInfo.platform == DOS860) &&
					 !uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
					if (orgFInfo.fontType == DOS850) ASCIIToDos850(utterance->line+i);
					else if (orgFInfo.fontType == DOS860) ASCIIToDos860(utterance->line+i);
					else if (orgFInfo.fontType == MONACO) ASCIIToMonaco(utterance->line+i);
					else if (orgFInfo.fontType == COURIER) ASCIIToMonaco(utterance->line+i);
				}
			}
*/
			if (MkUnix) {
				if (ufinfo.fontTable != NULL)
					uindx += (*ufinfo.fontTable)(utterance->line, &i, uline+uindx, &uch, NULL);
				else
					uline[uindx++] = utterance->line[i];
				if (uindx > LUTTLINELEN-100) {
				    fprintf(stderr, "ERROR: Utterance longer than %ld.\n", LUTTLINELEN-100);
				    fprintf(stderr, "In file %s; on line %ld.\n", oldfname, lineno);
				    fatalError(TRUE);
				}
			}
			if (MkW95) {
				j = windx;
				if (wfinfo.fontTable != NULL)
					windx += (*wfinfo.fontTable)(utterance->line, &i, wline+windx, &wch, NULL);
				else
					wline[windx++] = utterance->line[i];
				for (; j < windx; j++)
					attwline[j] = utterance->attLine[i];
				if (windx > LUTTLINELEN-100) {
				    fprintf(stderr, "ERROR: Utterance longer than %ld.\n", LUTTLINELEN-100);
				    fprintf(stderr, "In file %s; on line %ld.\n", oldfname, lineno);
				    fatalError(TRUE);
				}
			}
			if (MkMac) {
				j = mindx;
				if (mfinfo.fontTable != NULL)
					mindx += (*mfinfo.fontTable)(utterance->line, &i, mline+mindx, &mch, NULL);
				else
					mline[mindx++] = utterance->line[i];
				for (; j < mindx; j++)
					attmline[j] = utterance->attLine[i];
				if (mindx > LUTTLINELEN-100) {
				    fprintf(stderr, "ERROR: Utterance longer than %ld.\n", LUTTLINELEN-100);
				    fprintf(stderr, "In file %s; on line %ld.\n", oldfname, lineno);
				    fatalError(TRUE);
				}
			}
			if (MkDos) {
				if (dfinfo.fontTable != NULL)
					dindx += (*dfinfo.fontTable)(utterance->line, &i, dline+dindx, &dch, NULL);
				else
					dline[dindx++] = utterance->line[i];
				if (dindx > LUTTLINELEN-100) {
				    fprintf(stderr, "ERROR: Utterance longer than %ld.\n", LUTTLINELEN-100);
				    fprintf(stderr, "In file %s; on line %ld.\n", oldfname, lineno);
				    fatalError(TRUE);
				}
			}
		}

		if (MkUnix) {
			uline[uindx] = EOS;
			makedata_printout(ufp, utterance->speaker, uline, utterance->attSp, NULL, '\n', '\0', FALSE);
		}
		if (MkW95) {
			wline[windx] = EOS;
			makedata_printout(wfp, utterance->speaker, wline, utterance->attSp, attwline, '\r', '\n', FALSE);
		}
		if (MkMac) {
			mline[mindx] = EOS;
			makedata_printout(mfp, utterance->speaker, mline, utterance->attSp, attmline, '\r', '\0', FALSE);
		}
		if (MkDos) {
			dline[dindx] = EOS;
			makedata_printout(dfp, utterance->speaker, dline, utterance->attSp, NULL, '\r', '\n', FALSE);
		}
	}
	if (!isRecursive) {
		fprintf(stderr, " Done!\n");
		fprintf(fpout, "	Changes from ASCII to extended: %ld\n", ASCIIAdjustment);
		if (MkUnix) fprintf(fpout, "	Changes to UNIX: %ld\n", uch);
		if (MkW95)  fprintf(fpout, "	Changes to Win95/98/2000:  %ld\n", wch);
		if (MkMac)  fprintf(fpout, "	Changes to MAC:  %ld\n", mch);
		if (MkDos)  fprintf(fpout, "	Changes to DOS:  %ld\n", dch);
	} else {
		if (!chatmode)
			fprintf(fpout, "  Text ");
		else if (!isCainFile)
			fprintf(fpout, "  CHAT ");
		else
			fprintf(fpout, "  CA   ");
		if (MkUnix) fprintf(fpout, "to UNIX: %ld; ", uch);
		if (MkW95)  fprintf(fpout, "to Win95/98/2000: %ld; ", wch);
		if (MkMac)  fprintf(fpout, "to MAC: %ld; ", mch);
		if (MkDos)  fprintf(fpout, "to DOS: %ld; ", dch);
		fprintf(fpout, "\n");
	}
}

#define ISASCII(a, b) (st[1] == (a) && st[2] == (b))
#define ASCIITOCHAR(c) { strcpy(st, st+2); *st = (c); ASCIIAdjustment++; }

void ASCIIToDos850(char *st){
	if (ISASCII('A', '"')) ASCIITOCHAR('\216')
	else if (ISASCII('A', '0')) ASCIITOCHAR('\217')
	else if (ISASCII('C', 'c')) ASCIITOCHAR('\200')
	else if (ISASCII('C', ',')) ASCIITOCHAR('\200')
	else if (ISASCII('E', '\'')) ASCIITOCHAR('\220')
	else if (ISASCII('N', '~')) ASCIITOCHAR('\245')
	else if (ISASCII('O', '"')) ASCIITOCHAR('\231')
	else if (ISASCII('U', '"')) ASCIITOCHAR('\232')
	else if (ISASCII('a', '\'')) ASCIITOCHAR('\240')
	else if (ISASCII('a', '`')) ASCIITOCHAR('\205')
	else if (ISASCII('a', '^')) ASCIITOCHAR('\203')
	else if (ISASCII('a', '"')) ASCIITOCHAR('\204')
	else if (ISASCII('a', '~')) ASCIITOCHAR('\306')
	else if (ISASCII('a', '0')) ASCIITOCHAR('\206')
	else if (ISASCII('c', 'c')) ASCIITOCHAR('\207')
	else if (ISASCII('c', ',')) ASCIITOCHAR('\207')
	else if (ISASCII('e', '\'')) ASCIITOCHAR('\202')
	else if (ISASCII('e', '`')) ASCIITOCHAR('\212')
	else if (ISASCII('e', '^')) ASCIITOCHAR('\210')
	else if (ISASCII('e', '"')) ASCIITOCHAR('\211')
	else if (ISASCII('i', '\'')) ASCIITOCHAR('\241')
	else if (ISASCII('i', '`')) ASCIITOCHAR('\215')
	else if (ISASCII('i', '^')) ASCIITOCHAR('\214')
	else if (ISASCII('i', '"')) ASCIITOCHAR('\213')
	else if (ISASCII('n', '~')) ASCIITOCHAR('\244')
	else if (ISASCII('o', '\'')) ASCIITOCHAR('\242')
	else if (ISASCII('o', '`')) ASCIITOCHAR('\225')
	else if (ISASCII('o', '^')) ASCIITOCHAR('\223')
	else if (ISASCII('o', '"')) ASCIITOCHAR('\224')
	else if (ISASCII('o', '~')) ASCIITOCHAR('\344')
	else if (ISASCII('u', '\'')) ASCIITOCHAR('\243')
	else if (ISASCII('u', '`')) ASCIITOCHAR('\227')
	else if (ISASCII('u', '^')) ASCIITOCHAR('\226')
	else if (ISASCII('u', '"')) ASCIITOCHAR('\201')
	else if (ISASCII('s', 's')) ASCIITOCHAR('\341')
	else if (ISASCII('A', 'E')) ASCIITOCHAR('\222')
	else if (ISASCII('O', '/')) ASCIITOCHAR('\235')
	else if (ISASCII('a', 'e')) ASCIITOCHAR('\221')
	else if (ISASCII('o', '/')) ASCIITOCHAR('\233')
	else if (ISASCII('A', '`')) ASCIITOCHAR('\267')
	else if (ISASCII('A', '~')) ASCIITOCHAR('\307')
	else if (ISASCII('O', '~')) ASCIITOCHAR('\345')
	else if (ISASCII('y', '"')) ASCIITOCHAR('\230')

	else if (ISASCII('A', '\'')) ASCIITOCHAR('\265')
	else if (ISASCII('A', '^')) ASCIITOCHAR('\266')
	else if (ISASCII('d', '\\')) ASCIITOCHAR('\320')
	else if (ISASCII('D', '\\')) ASCIITOCHAR('\321')
	else if (ISASCII('E', '^')) ASCIITOCHAR('\322')
	else if (ISASCII('E', '"')) ASCIITOCHAR('\323')
	else if (ISASCII('E', '`')) ASCIITOCHAR('\324')
	else if (ISASCII('I', '^')) ASCIITOCHAR('\327')
	else if (ISASCII('I', '"')) ASCIITOCHAR('\330')
	else if (ISASCII('I', '`')) ASCIITOCHAR('\336')
	else if (ISASCII('O', '\'')) ASCIITOCHAR('\340')
	else if (ISASCII('O', '^')) ASCIITOCHAR('\342')
	else if (ISASCII('O', '`')) ASCIITOCHAR('\343')
	else if (ISASCII('P', 'D')) ASCIITOCHAR('\347')
	else if (ISASCII('p', 'd')) ASCIITOCHAR('\350')
	else if (ISASCII('U', '\'')) ASCIITOCHAR('\351')
	else if (ISASCII('U', '^')) ASCIITOCHAR('\352')
	else if (ISASCII('U', '`')) ASCIITOCHAR('\353')
	else if (ISASCII('y', '\'')) ASCIITOCHAR('\354')
	else if (ISASCII('Y', '\'')) ASCIITOCHAR('\355')
}

void ASCIIToDos860(char *st){
	if (ISASCII('a', '\'')) ASCIITOCHAR('\240')
	else if (ISASCII('a', '`')) ASCIITOCHAR('\205')
	else if (ISASCII('a', '^')) ASCIITOCHAR('\203')
	else if (ISASCII('a', '~')) ASCIITOCHAR('\204')
	else if (ISASCII('c', 'c')) ASCIITOCHAR('\207')
	else if (ISASCII('c', ',')) ASCIITOCHAR('\207')
	else if (ISASCII('e', '\'')) ASCIITOCHAR('\202')
	else if (ISASCII('e', '`')) ASCIITOCHAR('\212')
	else if (ISASCII('e', '^')) ASCIITOCHAR('\210')
	else if (ISASCII('i', '\'')) ASCIITOCHAR('\241')
	else if (ISASCII('i', '`')) ASCIITOCHAR('\215')
	else if (ISASCII('n', '~')) ASCIITOCHAR('\244')
	else if (ISASCII('o', '\'')) ASCIITOCHAR('\242')
	else if (ISASCII('o', '`')) ASCIITOCHAR('\225')
	else if (ISASCII('o', '^')) ASCIITOCHAR('\223')
	else if (ISASCII('o', '~')) ASCIITOCHAR('\224')
	else if (ISASCII('s', 's')) ASCIITOCHAR('\341')
	else if (ISASCII('u', '\'')) ASCIITOCHAR('\243')
	else if (ISASCII('u', '`')) ASCIITOCHAR('\227')
	else if (ISASCII('u', '"')) ASCIITOCHAR('\201')
	else if (ISASCII('A', '\'')) ASCIITOCHAR('\206')
	else if (ISASCII('A', '^')) ASCIITOCHAR('\217')
	else if (ISASCII('A', '`')) ASCIITOCHAR('\221')
	else if (ISASCII('A', '~')) ASCIITOCHAR('\216')
	else if (ISASCII('C', 'c')) ASCIITOCHAR('\200')
	else if (ISASCII('C', ',')) ASCIITOCHAR('\200')
	else if (ISASCII('E', '^')) ASCIITOCHAR('\211')
	else if (ISASCII('E', '`')) ASCIITOCHAR('\222')
	else if (ISASCII('E', '\'')) ASCIITOCHAR('\220')
	else if (ISASCII('I', '\'')) ASCIITOCHAR('\213')
	else if (ISASCII('I', '`')) ASCIITOCHAR('\230')
	else if (ISASCII('N', '~')) ASCIITOCHAR('\245')
	else if (ISASCII('O', '\'')) ASCIITOCHAR('\237')
	else if (ISASCII('O', '^')) ASCIITOCHAR('\214')
	else if (ISASCII('O', '`')) ASCIITOCHAR('\251')
	else if (ISASCII('O', '~')) ASCIITOCHAR('\231')
	else if (ISASCII('U', '\'')) ASCIITOCHAR('\226')
	else if (ISASCII('U', '`')) ASCIITOCHAR('\235')
	else if (ISASCII('U', '"')) ASCIITOCHAR('\232')
}

void ASCIIToMonaco(char *st){
	if (ISASCII('A', '"')) ASCIITOCHAR('\200')
	else if (ISASCII('A', '0')) ASCIITOCHAR('\201')
	else if (ISASCII('C', 'c')) ASCIITOCHAR('\202')
	else if (ISASCII('C', ',')) ASCIITOCHAR('\202')
	else if (ISASCII('E', '\'')) ASCIITOCHAR('\203')
	else if (ISASCII('N', '~')) ASCIITOCHAR('\204')
	else if (ISASCII('O', '"')) ASCIITOCHAR('\205')
	else if (ISASCII('U', '"')) ASCIITOCHAR('\206')
	else if (ISASCII('a', '\'')) ASCIITOCHAR('\207')
	else if (ISASCII('a', '`')) ASCIITOCHAR('\210')
	else if (ISASCII('a', '^')) ASCIITOCHAR('\211')
	else if (ISASCII('a', '"')) ASCIITOCHAR('\212')
	else if (ISASCII('a', '~')) ASCIITOCHAR('\213')
	else if (ISASCII('a', '0')) ASCIITOCHAR('\214')
	else if (ISASCII('c', 'c')) ASCIITOCHAR('\215')
	else if (ISASCII('c', ',')) ASCIITOCHAR('\215')
	else if (ISASCII('e', '\'')) ASCIITOCHAR('\216')
	else if (ISASCII('e', '`')) ASCIITOCHAR('\217')
	else if (ISASCII('e', '^')) ASCIITOCHAR('\220')
	else if (ISASCII('e', '"')) ASCIITOCHAR('\221')
	else if (ISASCII('i', '\'')) ASCIITOCHAR('\222')
	else if (ISASCII('i', '`')) ASCIITOCHAR('\223')
	else if (ISASCII('i', '^')) ASCIITOCHAR('\224')
	else if (ISASCII('i', '"')) ASCIITOCHAR('\225')
	else if (ISASCII('n', '~')) ASCIITOCHAR('\226')
	else if (ISASCII('o', '\'')) ASCIITOCHAR('\227')
	else if (ISASCII('o', '`')) ASCIITOCHAR('\230')
	else if (ISASCII('o', '^')) ASCIITOCHAR('\231')
	else if (ISASCII('o', '"')) ASCIITOCHAR('\232')
	else if (ISASCII('o', '~')) ASCIITOCHAR('\233')
	else if (ISASCII('u', '\'')) ASCIITOCHAR('\234')
	else if (ISASCII('u', '`')) ASCIITOCHAR('\235')
	else if (ISASCII('u', '^')) ASCIITOCHAR('\236')
	else if (ISASCII('u', '"')) ASCIITOCHAR('\237')
	else if (ISASCII('s', 's')) ASCIITOCHAR('\247')
	else if (ISASCII('A', 'E')) ASCIITOCHAR('\256')
	else if (ISASCII('O', '/')) ASCIITOCHAR('\257')
	else if (ISASCII('a', 'e')) ASCIITOCHAR('\276')
	else if (ISASCII('o', '/')) ASCIITOCHAR('\277')
	else if (ISASCII('A', '`')) ASCIITOCHAR('\313')
	else if (ISASCII('A', '~')) ASCIITOCHAR('\314')
	else if (ISASCII('O', '~')) ASCIITOCHAR('\315')
	else if (ISASCII('O', 'E')) ASCIITOCHAR('\316')
	else if (ISASCII('o', 'e')) ASCIITOCHAR('\317')
	else if (ISASCII('y', '"')) ASCIITOCHAR('\330')
}
