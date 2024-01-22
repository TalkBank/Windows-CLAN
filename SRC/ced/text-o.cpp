#include "ced.h"
#include "cu.h"
#include "c_clan.h"
#include "my_ctype.h"
#include "MMedia.h"

typedef union WCHARUMASK {
	short num;
	unsigned char c[2];
} WCHARUMASK;

#if defined(_MAC_CODE) || defined(_WIN32)
void RecompEdWinSize(void);
#endif

static void AddCharAfter(unCH c);
static void AddRowAfter(unCH *buf, char RED, FONTINFO *fontInfo, NewFontInfo *finfo);
static char ContNextRow(void);

extern void SaveVarUndoState(UNDO *p);
extern void ResetVarStatus(UNDO *p);
extern "C"
{
	extern void init_punct(char which);
}

myFInfo *global_df;

char  rawTextInput;
char  isDontAskDontTell = FALSE;
char  doReWrap = FALSE;
char  mem_error = FALSE;
char  DefAutoWrap = FALSE;
unCH  sp[SPEAKERLEN];
char  spC[SPEAKERLEN];
unCH  ced_line[UTTLINELEN+1]; // found uttlinelen
char  ced_lineC[UTTLINELEN+1];
char  tempW[TEMPWLEN];
char  DOSdataErr[256];
short DOSdata;
WCHARUMASK wMask;

#define W_UTTLINELEN UTTLINELEN+UTTLINELEN+4
static char in_buf[UTTLINELEN];
static unCH wIn_buf[W_UTTLINELEN];


void mem_err(char bck, myFInfo *DF) {
	char dt;

	if (DF == NULL) dt = FALSE;
	else dt = DF->DataChanged;
	mem_error = TRUE;
	if (dt && bck) SaveCKPFile(FALSE);
#if !defined(_MAC_CODE) && !defined(_WIN32)
	clear();
	refresh();
	endwin();
	if (dt) fprintf(stderr,"Your file %s has been checkpointed.\n", DataFile.dfname);
	MemErr;
#else /* !defined(_MAC_CODE) && !defined(_WIN32) */
#ifdef _MAC_CODE
	if (MEMPROT) free(MEMPROT);
#endif
	if (dt && bck) ProgExit("Out of memory!\rPlease increase memory size.\r\rYour data has been saved in a file with '.CKP' extention.");
	else ProgExit("Out of memory!\rPlease increase memory size.");
#endif /* else !defined(_MAC_CODE) && !defined(_WIN32) */
}

static char SetTextAttW(unCH *st, AttTYPE *att) {
	char c;
	char found = FALSE;

	c = (char)st[1];
	if (c == underline_start) {
		*att = set_underline_to_1(*att);
		found = TRUE;
	} else if (c == underline_end) {
		*att = set_underline_to_0(*att);
		found = TRUE;
	} else if (c == italic_start) {
		*att = set_italic_to_1(*att);
		found = TRUE;
	} else if (c == italic_end) {
		*att = set_italic_to_0(*att);
		found = TRUE;
	} else if (c == bold_start) {
		*att = set_bold_to_1(*att);
		found = TRUE;
	} else if (c == bold_end) {
		*att = set_bold_to_0(*att);
		found = TRUE;
	} else if (c == error_start) {
		*att = set_error_to_1(*att);
		found = TRUE;
	} else if (c == error_end) {
		*att = set_error_to_0(*att);
		found = TRUE;
	} else if (c == blue_start) {
		*att = set_color_num(blue_color, *att);
		found = TRUE;
	} else if (c == red_start) {
		*att = set_color_num(red_color, *att);
		found = TRUE;
	} else if (c == green_start) {
		*att = set_color_num(green_color, *att);
		found = TRUE;
	} else if (c == magenta_start) {
		*att = set_color_num(magenta_color, *att);
		found = TRUE;
	} else if (c == color_end) {
		*att = zero_color_num(*att);
		found = TRUE;
	}

	if (found) {
		strcpy(st, st+2);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

static long RebuildStrAtts(char *line, long i, AttTYPE att, AttTYPE oldAtt, char isJustStats) {
	if (att != oldAtt) {
		if (is_underline(att) != is_underline(oldAtt)) {
			if (!isJustStats) {
				if (is_underline(att))
					uS.sprintf(line+i, "%c%c", ATTMARKER, underline_start);
				else
					uS.sprintf(line+i, "%c%c", ATTMARKER, underline_end);
			}
			i += 2;
		}
		if (is_italic(att) != is_italic(oldAtt)) {
			if (!isJustStats) {
				if (is_italic(att))
					uS.sprintf(line+i, "%c%c", ATTMARKER, italic_start);
				else
					uS.sprintf(line+i, "%c%c", ATTMARKER, italic_end);
			}
			i += 2;
		}
		if (is_bold(att) != is_bold(oldAtt)) {
			if (!isJustStats) {
				if (is_bold(att))
					uS.sprintf(line+i, "%c%c", ATTMARKER, bold_start);
				else
					uS.sprintf(line+i, "%c%c", ATTMARKER, bold_end);
			}
			i += 2;
		}
		if (is_error(att) != is_error(oldAtt)) {
			if (!isJustStats) {
				if (is_error(att))
					uS.sprintf(line+i, "%c%c", ATTMARKER, error_start);
				else
					uS.sprintf(line+i, "%c%c", ATTMARKER, error_end);
			}
			i += 2;
		}
		if (is_word_color(att) != is_word_color(oldAtt)) {
			if (!isJustStats) {
				char color;

				color = get_color_num(att);
				if (color) {
					if (color == blue_color)
						uS.sprintf(line+i, "%c%c", ATTMARKER, blue_start);
					else if (color == red_color)
						uS.sprintf(line+i, "%c%c", ATTMARKER, red_start);
					else if (color == green_color)
						uS.sprintf(line+i, "%c%c", ATTMARKER, green_start);
					else // if (color == magenta_color)
						uS.sprintf(line+i, "%c%c", ATTMARKER, magenta_start);
				} else
					uS.sprintf(line+i, "%c%c", ATTMARKER, color_end);
			}
			i += 2;
		}
	}
	return(i);
}

#if defined(_WIN32)
static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *font,NEWTEXTMETRICEX *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == (int)font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}

static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *font,NEWTEXTMETRIC FAR *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == (int)font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}
#endif /* _WIN32 */

char SetNewFont(char *st, char ec, NewFontInfo *finfo) {
	const char *e;
	int  i, orgPlatform;

	if (rawTextInput)  {
		return(FALSE);
	}

	for (i=0; st[i] && st[i] != ':' && st[i] != ec; i++) ;
	if (st[i] != ':')
		return(FALSE);

	orgPlatform = global_df->platform;
	if ((e=GetDatasFont(st+i+1, ec, &dFnt, global_df->isUTF, finfo)) == NULL)
		return(FALSE);
	global_df->platform = finfo->platform;

#ifdef _MAC_CODE
	if (orgPlatform == DOSDATA && global_df->platform != DOSDATA) {
		return(FALSE);
	}
	if (finfo->platform != MACDATA) {
		if (finfo->fontType != NOCHANGE) {
			if (finfo->platform == DOSDATA)
				DOSdata = 1;
			else if (finfo->platform == WIN95DATA)
				DOSdata = 3;
		}

		if (!FindTTable(finfo, MACDATA)) {
/* 2007-08-16
			if (finfo->platform == WIN95DATA)
				sprintf(DOSdataErr, "Window's font \"%s\" Script: %d is not supported on this computer.",finfo->fontName, finfo->CharSet);
			else
				sprintf(DOSdataErr, "Font \"%s\" is not supported on this computer.",finfo->fontName);
			DOSdata = 6;
*/
			return(FALSE);
		}
	}
	if (strcmp(finfo->fontName, "LxS SpEcIaL FoNt")) {
		if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
/* 2007-08-28
			sprintf(DOSdataErr, "Font \"%s\" is not supported on this computer.",finfo->fontName);
			DOSdata = 6;
*/
			return(FALSE);
		}
		finfo->CharSet = my_FontToScript(finfo->fontId, finfo->CharSet);
	} else {
		strcpy(finfo->fontName, defUniFontName);
		if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
			sprintf(DOSdataErr, "Font \"%s\" is not supported on this computer.",finfo->fontName);
			DOSdata = 6;
			return(FALSE);
		}
	}
	if (DOSdata == -1) 
		DOSdata = 0;
	if (finfo->fontSize < 9)
		finfo->fontSize = 10;
	finfo->charHeight = GetFontHeight(NULL, finfo, global_df->wind);
	if (global_df->MinFontSize > finfo->charHeight)
		global_df->MinFontSize = finfo->charHeight;
	finfo->Encod = finfo->CharSet;
	strcpy(st, e);
	return(TRUE);
#elif defined(_WIN32) // _MAC_CODE
	LOGFONT t_lfDefFont;
	unCH fontNameU[256];

	if (orgPlatform == DOSDATA && global_df->platform != DOSDATA) {
		return(FALSE);
	}
	if (finfo->platform != WIN95DATA) {
		if (finfo->fontType != NOCHANGE) {
			if (finfo->platform == DOSDATA)
				DOSdata = 1;
			else if (finfo->platform == MACDATA)
				DOSdata = 2;
		}

		if (!FindTTable(finfo, WIN95DATA)) {
/* 2007-08-28
			if (finfo->platform == MACDATA)
				sprintf(DOSdataErr, "Macintosh's font \"%s\" is not supported on this computer.",finfo->fontName);
			else
				sprintf(DOSdataErr, "Font \"%s\" is not supported on this computer.",finfo->fontName);
			DOSdata = 6;
*/
			return(FALSE);
		}
	}
	SetLogfont(&t_lfDefFont, NULL, finfo); 
	if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&t_lfDefFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)finfo,0) != 0) {
		u_strcpy(fontNameU, finfo->fontName, 256);
		if (EnumFontFamilies(GlobalDC->GetSafeHdc(),fontNameU,(FONTENUMPROC)EnumFontFamProc,(LPARAM)finfo) != 0) {
/* 2007-08-28
 			sprintf(DOSdataErr, "Font \"%s\" is not supported on this computer.",finfo->fontName);
			DOSdata = 6;
*/
			return(FALSE);
		}
	}
	if (DOSdata == -1) 
		DOSdata = 0;
	finfo->charHeight = GetFontHeight(NULL, finfo);
	if (global_df->MinFontSize > finfo->charHeight)
		global_df->MinFontSize = finfo->charHeight;
	strcpy(st, e);
	return(TRUE);
#else /* _MAC_CODE && _WIN32 */
	DOSdata = 0;
	return(FALSE);
#endif /* _WIN32 */
}

static char *getPIDs(char *line) {
	int  i;
	char *PIDs;

	uS.remFrontAndBackBlanks(line);
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '\n' || line[i] == '\t')
			line[i] = ' ';
	}
	removeExtraSpace(line);
	i = strlen(PIDHEADER);
	for (; isSpace(line[i]); i++) ;
	if (line[i] == EOS)
		return(NULL);
	PIDs = (char *)malloc(strlen(line+i)+1);
	if (PIDs != NULL)
		strcpy(PIDs, line+i);
	return(PIDs);
}

#ifdef _MAC_CODE
char GetFontOfTEXT(char *buf, FNType *fname) {
	char res;
    Handle InfoHand;
	short ResRefNum;
	int   oldResRefNum;
	struct MPSR1020 *fontInfo;
	FSRef  ref;
	
	res = FALSE;
	InfoHand = NULL;
	oldResRefNum = CurResFile();
	my_FSPathMakeRef(fname, &ref); 
	if ((ResRefNum=FSOpenResFile(&ref, fsRdPerm)) != -1) {
		UseResFile(ResRefNum);
		if ((InfoHand=GetResource('MPSR',1020))) {
			HLock(InfoHand);
			fontInfo = (struct MPSR1020 *)*InfoHand;
			strcpy(buf, fontInfo->font);
			res = TRUE;
			HUnlock(InfoHand);
		}
		CloseResFile(ResRefNum);
	}
	UseResFile(oldResRefNum);
	return(res);
}
#endif // _MAC_CODE

static char isXMLFile(char *in_buf, NewFontInfo *finfo) {
	int i;

	if (uS.partcmp(in_buf, "<?xml ", FALSE, FALSE)) {
		for (i=6; in_buf[i] != '>' && in_buf[i] != EOS; i++) ;
		if (uS.partcmp(in_buf+i-1, "?>", FALSE, FALSE)) {
			for (i=6; in_buf[i] != EOS; i++) {
				if (uS.partcmp(in_buf+i, "encoding=", FALSE, FALSE)) {
					i += 9;
					if (in_buf[i] == '"')
						i++;
					if (uS.partcmp(in_buf+i, "ISO-8859-1", FALSE, FALSE)) {
#if defined(_MAC_CODE)
						finfo->Encod = kTextEncodingWindowsLatin1;
#elif defined(_WIN32)
						finfo->Encod = 1252;
#endif
						strcpy(in_buf+i, in_buf+i+5);
						in_buf[i++] = 'U';
						in_buf[i++] = 'T';
						in_buf[i++] = 'F';
						in_buf[i++] = '-';
						in_buf[i++] = '8';
					} else if (!uS.mStrnicmp(in_buf+i, "utf-8", 5)) {
						global_df->isUTF = 1;
					}
					break;
				}
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

static char isXLSFile(FNType *fname) {
	register int j;
	unCH ext[4];

	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);
	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'X' && ext[1] == 'L' && ext[2] == 'S')
			return(TRUE);
		else
			return(FALSE);
	} else
		return(FALSE);
}

static char isCAFontHeader(char *in_buf) {
	int i;

	for (i=0; in_buf[i] != EOS; i++) {
		if (!uS.mStrnicmp(in_buf+i, "CAfont", 6))
			return(TRUE);
	}
	return(FALSE);
}

static char ReadInText(char *isUTF8Header, short id) {
	register char tDataChanged = 0;
	register long len;
	register unsigned long RL1 = 0L;
	register unsigned long RL2 = 0L;
	register unsigned long RT = 0L;
	register unsigned long cnt;
	long i, err = 0L;
	NewFontInfo ufinfo;
	AttTYPE att;
	char AttFound, FirstLine;
	char fontHeaderFound, isClanOutFile;
	NewFontInfo finfo;
	NewFontInfo defFInfo;
	FILE *fp;
#if defined(_WIN32)
	float tf;
	extern float scalingSize;
#endif

	if (isDontAskDontTell)
		global_df->dontAskDontTell = TRUE;
	isDontAskDontTell = FALSE;
	global_df->platform = NOCHANGE;
	finfo.isUTF = 0;

	SetDefaultUnicodeFinfo(&ufinfo);
	if (global_df->MinFontSize > ufinfo.charHeight)
		global_df->MinFontSize = ufinfo.charHeight;
	ufinfo.fontTable = NULL;
	strcpy(finfo.fontName, DEFAULT_FONT);
	finfo.fontSize = DEFAULT_SIZE;
	finfo.fontId = DEFAULT_ID;
#ifdef _MAC_CODE
	finfo.orgFType = getFontType(finfo.fontName, FALSE);
	finfo.CharSet = my_FontToScript(DEFAULT_ID, 0);
	finfo.Encod = finfo.orgEncod = finfo.CharSet;
#elif defined(_WIN32)
	finfo.orgFType = getFontType(finfo.fontName, TRUE);
	finfo.CharSet = NSCRIPT;
	finfo.Encod = finfo.orgEncod = 1252;
#endif
	finfo.fontTable = NULL;
#if defined(_WIN32)
	finfo.charHeight = GetFontHeight(NULL, &finfo);
#endif
#if defined(_MAC_CODE)
	finfo.charHeight = GetFontHeight(NULL, &finfo, global_df->wind);
#endif
	copyNewFontInfo(&defFInfo, &finfo);
	if (uS.FNTypecmp(global_df->fileName, NEWFILENAME, 0L) == 0)
		;
	else if (id != 1964 && *global_df->fileName != EOS && (fp=fopen(global_df->fileName,"rb")) != NULL) {
		tDataChanged = isCHATFile(global_df->fileName);
		if ((tDataChanged == '\002') || (isCEXFile(global_df->fileName) == '\001') || isXLSFile(global_df->fileName))
			isClanOutFile = '\001';
		else if (tDataChanged == '\003' || tDataChanged == '\005')
			isClanOutFile = '\002';
		else
/*
#ifdef _MAC_CODE
			isClanOutFile = 0;
#endif
#ifdef _WIN32
*/
		if (tDataChanged == '\001')
			isClanOutFile = '\001';
		else
			isClanOutFile = '\002';
/*
#endif
*/
		fontHeaderFound = 0;
		last_cr_char = 0;
		tDataChanged = 0;
#ifdef _MAC_CODE
		DrawMouseCursor(2);
		if (DefChatMode == 2 && !global_df->ChatMode) {
			if (GetFontOfTEXT(in_buf, global_df->fileName)) {
				if (SetNewFont(in_buf, EOS, &defFInfo)) {
					copyNewFontInfo(&finfo, &defFInfo);
					if (!isOrgUnicodeFont(finfo.orgFType))
						fontHeaderFound = 1;
					else
						fontHeaderFound = -1;
				}
			}
		}
#endif
		if (rawTextInput) {
			if (DOSdata == -1) 
				DOSdata = 0;
		}
		FirstLine = TRUE;
		last_cr_char = 0;

		wMask.c[0] = getc(fp);
		if (wMask.c[0] == (int)0xFF || wMask.c[0] == (int)0xFE) {
			char lEncode;
			short isNLFound;

			isNLFound = 0;
			*isUTF8Header = TRUE;
			if (wMask.c[0] == (int)0xFF)
				lEncode = 1;
			else
				lEncode = 0;
			wMask.c[0] = getc(fp);
			len = 0L;
#ifdef _MAC_CODE
			if (byteOrder == CFByteOrderLittleEndian) {
				if (lEncode == 1)
					lEncode = 0;
				else 
					lEncode = 1;
			}
#endif
			while (!feof(fp)) {
#ifdef _WIN32 
				if (lEncode == 1) {
					wMask.c[0] = getc(fp);
					if (feof(fp))
						break;
					wMask.c[1] = getc(fp);
				} else {
					wMask.c[1] = getc(fp);
					if (feof(fp))
						break;
					wMask.c[0] = getc(fp);
				}
#else
				if (lEncode == 1) {
					wMask.c[1] = getc(fp);
					if (feof(fp))
						break;
					wMask.c[0] = getc(fp);
				} else {
					wMask.c[0] = getc(fp);
					if (feof(fp))
						break;
					wMask.c[1] = getc(fp);
				}
#endif
				if (wMask.num == (short)'\n' || wMask.num == (short)'\r') {
					wIn_buf[len] = EOS;
					if (isNLFound && isNLFound != wMask.num)
						isNLFound = 0;
					else {
						AddRowAfter(wIn_buf, FALSE, NULL, &ufinfo);
						isNLFound = wMask.num;
					}
					len = 0;
				} else {
					wIn_buf[len++] = wMask.num;
					isNLFound = 0;
				}

			}
			wIn_buf[len] = EOS;
			if (len > 0)
				AddRowAfter(wIn_buf, FALSE, NULL, &ufinfo);
		} else {
			rewind(fp);
			if (isXLSFile(global_df->fileName)) {
				SetDefaultUnicodeFinfo(&finfo);
				if (global_df->MinFontSize > finfo.charHeight)
					global_df->MinFontSize = finfo.charHeight;
				fontHeaderFound = 1;
				copyNewFontInfo(&defFInfo, &finfo);
				global_df->isUTF = 1;
				finfo.fontTable = NULL;
			} else if (*isUTF8Header == TRUE) {
				SetDefaultUnicodeFinfo(&finfo);
				if (global_df->MinFontSize > finfo.charHeight)
					global_df->MinFontSize = finfo.charHeight;
				copyNewFontInfo(&defFInfo, &finfo);
				global_df->isUTF = 1;
				finfo.fontTable = NULL;
				DOSdata = 1962;
			}
			while (fgets_ced(in_buf,UTTLINELEN, fp, &cnt)) {
				if (FirstLine) {
					if (in_buf[0] == (char)0xef && in_buf[1] == (char)0xbb && in_buf[2] == (char)0xbf) {
						strcpy(in_buf, in_buf+3);
						if (global_df->isUTF == 0 && fontHeaderFound <= 0) {
							char res;
							long  lFontSize;
							int   lCharSet;
							short lCharHeight;

							lFontSize   = finfo.fontSize;
							lCharSet    = finfo.CharSet;
							lCharHeight = finfo.charHeight;
							res = SetDefaultUnicodeFinfo(&finfo);
							if (fontHeaderFound == -1 && res) {
								finfo.fontSize   = lFontSize;
								finfo.CharSet    = lCharSet;
								finfo.charHeight = lCharHeight;
							}
							if (global_df->MinFontSize > finfo.charHeight)
								global_df->MinFontSize = finfo.charHeight;
							if (DOSdata != 6)
								DOSdata = 0;
							copyNewFontInfo(&defFInfo, &finfo);
						}
						global_df->isUTF = 1;
						finfo.fontTable = NULL;
						*isUTF8Header = TRUE;
					} else if (isXMLFile(in_buf, &finfo)) {
						if (fontHeaderFound <= 0) {
							char res;
							long  lFontSize;
							int   lCharSet;
							short lCharHeight;

							lFontSize   = finfo.fontSize;
							lCharSet    = finfo.CharSet;
							lCharHeight = finfo.charHeight;
							res = SetDefaultUnicodeFinfo(&finfo);
							if (fontHeaderFound == -1 && res) {
								finfo.fontSize   = lFontSize;
								finfo.CharSet    = lCharSet;
								finfo.charHeight = lCharHeight;
							}
							if (global_df->MinFontSize > finfo.charHeight)
								global_df->MinFontSize = finfo.charHeight;
							if (DOSdata != 6)
								DOSdata = 0;
							copyNewFontInfo(&defFInfo, &finfo);
						}
						finfo.fontTable = NULL;
						*isUTF8Header = TRUE;
						isClanOutFile = 0;
						global_df->dontAskDontTell = TRUE;
					}
				}
				if (rawTextInput) {
					if (DOSdata == 5)
#if defined(_MAC_CODE)
						global_df->crType = PCCrType;
#endif
#if defined(_WIN32)
						global_df->crType = MacCrType;
#endif
				}

				if (strncmp(in_buf, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) == 0) {
					global_df->SpecialTextFile = TRUE;
					continue;
				}
				att = 0;
				AttFound = FALSE;
				for (len=0L; in_buf[len]; len++) {
					while (in_buf[len] == ATTMARKER) {
						if (SetTextAtt(in_buf+len, '\0', &att))
							AttFound = TRUE;
						else
							break;
					}
					/* DO NOT PUT ELSE AFTER THIS IF */

					if (in_buf[len] == EOS) {
						break;
					} else if (in_buf[len] == NL_C) {
						strcpy(in_buf+len, in_buf+len+1);
						len--;
					} else if (in_buf[len] == '\n') {
						in_buf[len] = NL_C;
					} else if (uS.partwcmp(in_buf+len, FONTMARKER)) {
						if (!SetNewFont(in_buf+len,']',&finfo)) {
							copyNewFontInfo(&finfo, &defFInfo);
						} else {
							if (global_df->isUTF)
								finfo.fontTable = NULL;
							fontHeaderFound = 1;
							copyNewFontInfo(&defFInfo, &finfo);
						}
					}
					tempAtt[len] = att;
				}
				if (global_df->dontAskDontTell) {
				} else if (uS.partcmp(in_buf, PIDHEADER, FALSE, FALSE)) {
					if (!rawTextInput) {
						if (global_df->PIDs != NULL)
							free(global_df->PIDs);
						global_df->PIDs = getPIDs(in_buf);
						continue;
					}
				} else if (uS.partcmp(in_buf, WINDOWSINFO, FALSE, FALSE)) {
					if (!rawTextInput) {
						// 2020-02-01
						continue;
					}
				} else if (uS.partcmp(in_buf, CKEYWORDHEADER, FALSE, FALSE)) {
					if (!rawTextInput) {
						FreeColorText(global_df->RootColorText);
						global_df->RootColorText = NULL;
						global_df->RootColorText = createColorTextKeywordsList(global_df->RootColorText, in_buf);
						continue;
					}
/*
extern char *TempFileStringTag;
				} else if (TempFileStringTag != NULL && uS.partcmp(in_buf, TempFileStringTag, FALSE, FALSE)) {
					global_df->isTempFile = 2;
*/
				} else if (uS.partcmp(in_buf, "@Languages:", FALSE, TRUE) || uS.partcmp(in_buf, "@Options:", FALSE, TRUE)) {
					char isCAFound = FALSE, isIPAFound = FALSE, isThaiFound = FALSE;

					createColorWordsList(in_buf);
					for (i=0; in_buf[i] != ':' && in_buf[i] != EOS; i++) ;
					if (in_buf[i] == ':')
						i++;
					for (; isSpace(in_buf[i]); i++) ;
					for (; in_buf[i] != EOS; i++) {
						if (in_buf[i] == 'C' && in_buf[i+1] == 'A') {
							if (in_buf[i+2] != '-')
								isCAFound = TRUE;
						} else if (in_buf[i] == 'I' && in_buf[i+1] == 'P' && in_buf[i+2] == 'A')
							isIPAFound = TRUE;
						else if (in_buf[i]== 't' && in_buf[i+1]== 'h' && 
								 (isSpace(in_buf[i+2]) || in_buf[i+2]== ',' || in_buf[i+2]== EOS || in_buf[i+2]== NL_C))
							isThaiFound = TRUE;
					}
					if (isThaiFound || isCAFound) {
						ROWS *tr = global_df->row_txt;

						if (isThaiFound)
							SetDefaultThaiFinfo(&finfo);
						else {
							finfo.fontName[0] = EOS;
							SetDefaultCAFinfo(&finfo);
						}
						if (global_df->MinFontSize > finfo.charHeight)
							global_df->MinFontSize = finfo.charHeight;
						if (DOSdata != 6)
							DOSdata = 0;
						global_df->isUTF = 1;
						*isUTF8Header = TRUE;
						fontHeaderFound = 1;
						for (tr=global_df->head_text->next_row; tr != global_df->tail_text; tr=tr->next_row) {
#if defined(_WIN32)
							SetFontName(tr->Font.FName, finfo.fontName);
#else
							SetFontName(tr->Font.FName, finfo.fontId);
#endif /* else _WIN32 */
							tr->Font.FSize = finfo.fontSize;
							tr->Font.CharSet = finfo.CharSet;
							tr->Font.FHeight = finfo.charHeight;
						}
					}
					if (!rawTextInput && isClanOutFile == '\001') {
						if (global_df->isUTF) {
							if (isCAFound) {
								if (strcmp(finfo.fontName, "CAfont") && strcmp(finfo.fontName, "Ascender Uni Duo")) {
									strcpy(DOSdataErr, "Please select \"CAfont\" or \"Ascender Uni Duo\" font for CA file as per \"@Options:\" tier.");
									DOSdata = 6;
								}
							} else if (isIPAFound) {
								if (strcmp(finfo.fontName, "Charis SIL")) {
									strcpy(DOSdataErr, "Please select \"Charis SIL\" font for IPA file as per \"@Options:\" tier.");
									DOSdata = 6;
								}
							}
						} else {
							if (isCAFound) {
								if (strcmp(ufinfo.fontName, "CAfont") && strcmp(ufinfo.fontName, "Ascender Uni Duo")) {
									strcpy(DOSdataErr, "Please select \"CAfont\" or \"Ascender Uni Duo\" font for CA file as per \"@Options:\" tier.");
									DOSdata = 6;
								}
							} else if (isIPAFound) {
								if (strcmp(ufinfo.fontName, "Charis SIL")) {
									strcpy(DOSdataErr, "Please select \"Charis SIL\" font for IPA file as per \"@Options:\" tier.");
									DOSdata = 6;
								}
							}
						}
						if (isCAFound && isIPAFound) {
							strcpy(DOSdataErr, "Illegal use of both CA and IPA on \"Options:\" tier.");
							DOSdata = 6;
						}
					}
				} else if (uS.isUTF8(in_buf)) {
					if (global_df->isUTF == 0 || isUTFData) {
						if (fontHeaderFound <= 0) {
							char res;
							long  lFontSize;
							int   lCharSet;
							short lCharHeight;

							lFontSize   = finfo.fontSize;
							lCharSet    = finfo.CharSet;
							lCharHeight = finfo.charHeight;
							res = SetDefaultUnicodeFinfo(&finfo);
							if (fontHeaderFound == -1 && res) {
								finfo.fontSize   = lFontSize;
								finfo.CharSet    = lCharSet;
								finfo.charHeight = lCharHeight;
							}
							if (global_df->MinFontSize > finfo.charHeight)
								global_df->MinFontSize = finfo.charHeight;
							if (DOSdata != 6)
								DOSdata = 0;
							copyNewFontInfo(&defFInfo, &finfo);
						}
						global_df->isUTF = 1;
						finfo.fontTable = NULL;
						*isUTF8Header = TRUE;
					}
					continue;
				} else if (uS.partcmp(in_buf, FONTHEADER, FALSE, FALSE)) {
					if (global_df->SpecialTextFile == TRUE || isCAFontHeader(in_buf)) {
						if (!SetNewFont(in_buf, EOS, &finfo)) {
							if (isOrgUnicodeFont(finfo.orgFType))
								DOSdata = 0;
							copyNewFontInfo(&finfo, &defFInfo);
						} else {
							RT++;
							RL1++;
							if (!isOrgUnicodeFont(finfo.orgFType))
								fontHeaderFound = 1;
							else
								fontHeaderFound = -1;
							if (global_df->isUTF)
								finfo.fontTable = NULL;
							copyNewFontInfo(&defFInfo, &finfo);
							if (uS.mStricmp(finfo.fontName, "CAfont") == 0) {
								global_df->isUTF = 1;
#if defined(_WIN32)
								if (scalingSize != 1) {
									tf = finfo.fontSize;
									tf = tf * scalingSize;
									finfo.fontSize = (long)tf;
									finfo.charHeight = GetFontHeight(NULL, &finfo);
								}
#endif
								finfo.fontTable = NULL;
								*isUTF8Header = TRUE;
							}
						}
					} else {
						if (uS.mStricmp(finfo.fontName, "CAfont") == 0) {
							global_df->isUTF = 1;
#if defined(_WIN32)
							if (scalingSize != 1) {
								tf = finfo.fontSize;
								tf = tf * scalingSize;
								finfo.fontSize = (long)tf;
								finfo.charHeight = GetFontHeight(NULL, &finfo);
							}
#endif
							finfo.fontTable = NULL;
							*isUTF8Header = TRUE;
						}
					}
					continue;
				} else if (uS.partcmp(in_buf, MEDIAHEADER, FALSE, FALSE)) {
					u_strcpy(global_df->mediaFileName, in_buf+strlen(MEDIAHEADER), FILENAME_MAX-1);
					global_df->mediaFileName[FILENAME_MAX-1] = EOS;
					CleanMediaName(global_df->mediaFileName);
				}

				if (FirstLine && isUTFData && global_df->isUTF == 0 && fontHeaderFound <= 0 && isClanOutFile) {
					SetDefaultUnicodeFinfo(&finfo);
					if (global_df->MinFontSize > finfo.charHeight)
						global_df->MinFontSize = finfo.charHeight;
					DOSdata = 0;
					copyNewFontInfo(&defFInfo, &finfo);
					finfo.fontTable = NULL;
					global_df->isUTF = 1;
					*isUTF8Header = TRUE;
				}
				FirstLine = FALSE;

				if (finfo.fontTable != NULL && !global_df->isUTF) {
					long j;
					for (i=0L, j=0L; in_buf[i]; i++) {
						len = (*finfo.fontTable)(in_buf, &i, templineC2+j, NULL, NULL);
/* 28-03-03
						long k;
						if (len > 1) {
							for (k=0; k < len; k++)
								ced_line[j+k] = tempAtt[i];
						}
*/
						j += len;
					}
					templineC2[j] = EOS;
					strcpy(in_buf, templineC2);
					if (AttFound)
						len = j;
				} else {
					if (AttFound)
						len = strlen(in_buf);
				}

				cnt = strlen(in_buf);
				if (global_df->isUTF) {
					if (isClanOutFile != '\002') {
						if (AttFound) {
							AttTYPE oldAtt = 0;
							long j;

							i = 0L;
							j = 0L;
							while (in_buf[j] != EOS) {
								i = RebuildStrAtts(templineC, i, tempAtt[j], oldAtt, FALSE);
								oldAtt = tempAtt[j];
								templineC[i++] = in_buf[j];
								j++;
							}
							i = RebuildStrAtts(templineC, i, 0, oldAtt, FALSE);
							templineC[i] = EOS;
							strcpy(in_buf, templineC);
							cnt = strlen(in_buf);
						}
					}

					i = UTF8ToUnicode((unsigned char *)in_buf, cnt, wIn_buf, &cnt, W_UTTLINELEN);
					if (i)
						err = i;

					if (isClanOutFile != '\002') {
						if (AttFound) {
							att = 0;
							for (len=0L; wIn_buf[len]; len++) {
								if (wIn_buf[len] == 0xa0) {
									wIn_buf[len] = ' ';
									tDataChanged = 1;
								} else if (wIn_buf[len] == 0x202f) {
									wIn_buf[len] = ' ';
								} else if (wIn_buf[len] == 0x2022) {
									wIn_buf[len] = HIDEN_C;
									tDataChanged = 2;
								} else {
									while (wIn_buf[len] == ATTMARKER) {
										if (!SetTextAttW(wIn_buf+len, &att))
											break;
									}
								}
								tempAtt[len] = att;
							}
						} else {
							for (len=0L; wIn_buf[len]; len++) {
								if (wIn_buf[len] == 0xa0) {
									wIn_buf[len] = ' ';
									tDataChanged = 1;
								} else if (wIn_buf[len] == 0x202f) {
									wIn_buf[len] = ' ';
								} else if (wIn_buf[len] == 0x2022) {
									wIn_buf[len] = HIDEN_C;
									tDataChanged = 2;
								}
							}
						}
					}
					AddRowAfter(wIn_buf, FALSE, NULL, &finfo);
				} else {
					i = ANSIToUnicode((unsigned char *)in_buf, cnt, wIn_buf, NULL, W_UTTLINELEN, finfo.Encod);
					if (i)
						err = i;
					AddRowAfter(wIn_buf, FALSE, NULL, &ufinfo);
				}

				if (AttFound) {
					if ((global_df->row_txt->att=(AttTYPE *)malloc((len+1)*sizeof(AttTYPE)))==NULL)
						mem_err(TRUE, global_df);
					for (i=0; i < len; i++)
						global_df->row_txt->att[i] = tempAtt[i];
				}
				RT++;
				if (isSpeaker(in_buf[0])) RL1++;
				else if (in_buf[0]== '\t' || (in_buf[0]== ' ' && in_buf[1]== ' ')) RL2++;
			}
		}
#if defined(_MAC_CODE) || defined(_WIN32)
		if (global_df->numberOfRows > 1)
			global_df->numberOfRows--;
#endif
#ifdef _MAC_CODE
		DrawMouseCursor(0);
#endif

		fclose(fp);
		last_cr_char = 0;
	}
	if (global_df->head_text->next_row == global_df->tail_text) {
		if (isUTFData) {
			SetDefaultUnicodeFinfo(&finfo);
			if (global_df->MinFontSize > finfo.charHeight)
				global_df->MinFontSize = finfo.charHeight;
			global_df->isUTF = 1;
		}
		AddRowAfter(cl_T(""), FALSE, NULL, &finfo);
		global_df->EdWinSize = 1;
		DOSdata = 0;
		*isUTF8Header = TRUE;
	} else {
		if (DefChatMode < 2) {
			if ((RL1+RL2)*100/RT >= 90 - (100/RT)) {
				if (RL2 == 0L) {
					global_df->ChatMode = TRUE;
					if (global_df->ShowParags)
						global_df->ShowParags = FALSE;
				} else if (RL1*100/RL2 >= 5) {
					global_df->ChatMode = TRUE;
					if (global_df->ShowParags)
						global_df->ShowParags = FALSE;
				}
			}
		}
	}
	if (rawTextInput)
		DOSdata = 0;
	if (DOSdata == 5 && global_df->isUTF)
		DOSdata = 0;
	global_df->isUTF = 1;
	if (err) {
		DOSdata = 200;
#ifdef _MAC_CODE
		sprintf(DOSdataErr, "Error reading data file, data may be corrupt. (%ld)", err);
#else // _MAC_CODE
		LPVOID lpMsgBuf;
		FormatMessage( 
					  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL,
					  err,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					  (LPTSTR) &lpMsgBuf,
					  0,
					  NULL 
					 );
		u_strcpy(templineC, (LPCTSTR)lpMsgBuf, UTTLINELEN);
		LocalFree( lpMsgBuf );

		sprintf(DOSdataErr, "Error reading data file, data may be corrupt.\r\n%s (%u)", templineC, err);
#endif // else _MAC_CODE
	} else if (tDataChanged) {
		DOSdata = 200;
		sprintf(DOSdataErr, "Data file \"%s\" has been changed. Please save it.", global_df->fileName);
		if (tDataChanged == 2)
			strcat(DOSdataErr, " Bullet symbols have been changed. You can also run \"fixbullets\" command to change multiple CHAT files.");
		tDataChanged = TRUE;
	}
	return(tDataChanged);
}

void getVideosExt(int key) {
	int i;
	char err_mess[256];

	if (key == '0')
		i = 9;
	else
		i = key - '1';
	if (i >= 0 && i < 10) {
		if (global_df->VideosNameExts[i][0] != EOS) {
			global_df->VideosNameIndex = i;
		} else {
			sprintf(err_mess, "Can't find %dth media file.", i+1);
			do_warning(err_mess, 0);
		}
	}
}

void SetUpVideos(void) {
	register int i;
	unCH *s, *e, t;
	ROWS *tt;

	for (i=0; i < 10; i++)
		global_df->VideosNameExts[i][0] = EOS;
	global_df->VideosNameIndex = -1;
	ChangeCurLineAlways(0);
	tt = global_df->head_text->next_row;
	while (tt != global_df->tail_text) {
		if (tt->line[0] == '*')
			break;
		for (i=0; tt->line[i] && tt->line[i] != ':' && i < SPEAKERLEN-1; i++) {
			sp[i] = tt->line[i];
		}
		sp[i] = EOS;
		if (strcmp(sp, "@Videos") == 0) {
			strcpy(templine, tt->line);
			tt = tt->next_row;
			while (tt != global_df->tail_text) {
				if (isSpeaker(*tt->line))
					break;
				strcat(templine, tt->line);
				tt = tt->next_row;
			}
			for (s=templine; *s && *s != ':'; s++) ;
			for (; *s == ':' || isSpace(*s) || *s == ',' || *s == NL_C || *s == '\n'; s++) ;
			while (*s) {
				for (; isSpace(*s) || *s == ',' || *s == NL_C || *s == '\n'; s++) ;
				for (e=s; *e != EOS && !isSpace(*e) && *e != ',' && *e != NL_C && *e != '\n'; e++) ;
				t = *e;
				*e = EOS;
				if (*s != EOS) {
					for (i=0; i < 10; i++) {
						if (global_df->VideosNameExts[i][0] == EOS) {
							strncpy(global_df->VideosNameExts[i], s, 127);
							global_df->VideosNameExts[i][127] = EOS;
							break;
						}
					}
				}
				*e = t;
				if (*e == EOS)
					break;
				else
					s = e + 1;
			}
			if (global_df->VideosNameExts[i][0] != EOS)
				global_df->VideosNameIndex = 0;
			break;
		}
		tt = tt->next_row;
	}
}

void init_text(char *isUTF8Header, short id) {
	register char tDataChanged = FALSE;

	global_df->LeftCol = 0;
	global_df->redisplay = 1;
	global_df->UndoList = NULL;
	if ((global_df->UndoList=NEW(UNDO)) == NULL)
		mem_err(FALSE, global_df);
	global_df->UndoList->str = NULL;
	global_df->UndoList->NextUndo = NULL;
	global_df->UndoList->PrevUndo = NULL;
	global_df->UndoCounter = 0;
	global_df->numberOfRows = 0L;
	global_df->ShowParags = FALSE;
	global_df->head_text = NEW(ROWS);
	if (global_df->head_text == NULL) mem_err(FALSE, global_df);
	global_df->tail_text = NEW(ROWS);
	if (global_df->tail_text == NULL) mem_err(FALSE, global_df);
	global_df->head_text->line = NULL;
	global_df->head_text->att = NULL;

	global_df->window_rows_offset = 0L;
	global_df->head_text->Font.FSize = DEFAULT_SIZE;
#ifdef _MAC_CODE
	global_df->head_text->Font.FName = DEFAULT_ID;
	global_df->head_text->Font.CharSet = my_FontToScript(global_df->head_text->Font.FName, global_df->head_text->Font.CharSet);
	global_df->head_text->Font.FHeight = GetFontHeight(&global_df->head_text->Font,NULL,global_df->wind);
#else
	strcpy(global_df->head_text->Font.FName, DEFAULT_FONT);
	global_df->head_text->Font.CharSet = DEFAULT_CHARSET;
	global_df->head_text->Font.FHeight = GetFontHeight(&global_df->head_text->Font, NULL);
#endif
	global_df->head_text->next_row = global_df->tail_text;
	global_df->head_text->prev_row = NULL;
	global_df->tail_text->line = NULL;
	global_df->tail_text->att = NULL;
	global_df->tail_text->Font.FSize = global_df->head_text->Font.FSize;
	global_df->tail_text->Font.CharSet = global_df->head_text->Font.CharSet;
	SetFontName(global_df->tail_text->Font.FName, global_df->head_text->Font.FName);
	global_df->tail_text->Font.FHeight = global_df->head_text->Font.FHeight;
	global_df->tail_text->next_row = NULL;
	global_df->tail_text->prev_row = global_df->head_text;

	global_df->head_row = NEW(LINE);
	if (global_df->head_row == NULL) mem_err(FALSE, global_df);
	global_df->tail_row = NEW(LINE);
	if (global_df->tail_row == NULL) mem_err(FALSE, global_df);
	global_df->head_row_len = 0L;
	global_df->head_row->c = 0;
	global_df->head_row->att = 0;
	global_df->head_row->next_char = global_df->tail_row;
	global_df->head_row->prev_char = NULL;
	global_df->tail_row->c = 0;
	global_df->tail_row->att = 0;
	global_df->tail_row->next_char = NULL;
	global_df->tail_row->prev_char = global_df->head_row;

	global_df->row_txt = global_df->head_text;
	global_df->col_txt = global_df->head_row;
	global_df->top_win = global_df->row_txt;
	tDataChanged = ReadInText(isUTF8Header, id);
	global_df->row_win = 0L;
	global_df->col_win = 0L;
	global_df->col_chr = 0L;
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;

	global_df->fake_row_win = 0L;
	global_df->fake_col_win = 0L;
	global_df->fake_row_win2 = 0L;
	global_df->fake_col_win2 = -2L;

	global_df->row_txt = global_df->head_text->next_row;
	global_df->cur_line = global_df->row_txt;
	global_df->top_win = global_df->row_txt;
	global_df->lineno = 1L;
	CpCur_lineToHead_row(global_df->cur_line);
	global_df->col_txt = global_df->head_row->next_char;
	global_df->DataChanged = tDataChanged;

	global_df->head_text->flag = 0;
	FALSE_VIS_ID(global_df->head_text->flag);
	global_df->tail_text->flag = 0;
	FALSE_VIS_ID(global_df->tail_text->flag);


	Re_WrapLines(AddLineToRow, 0L, TRUE, NULL);

#ifdef _MAC_CODE
	SetCurrentFontParams(global_df->row_txt->Font.FName, global_df->row_txt->Font.FSize);
	SetTextWinMenus(TRUE);
#endif // _MAC_CODE

	if (DOSdata == 1962)
		global_df->DataChanged = TRUE;
	if (global_df->ChatMode) {
		SetUpParticipants();
		SetUpVideos();
		strcpy(global_df->err_message, DASHES);
#ifdef _MAC_CODE
		ChangeSpeakerMenuItem();
#endif // _MAC_CODE
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	else if (DOSdata > 0 && DOSdata < 6) 
		global_df->DataChanged = TRUE;
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
}

void FreeUpText(myFInfo *f) {
	ROWS *tt;
	LINE *tl;

	while (f->head_text != f->tail_text) {
		tt = f->head_text;
		f->head_text = f->head_text->next_row;
		if (tt->line) free(tt->line); 
		if (tt->att) free(tt->att); 
		free(tt);
	}
	if (f->head_text->line) free(f->head_text->line);
	if (f->head_text->att) free(f->head_text->att);
	free(f->head_text);

	while (f->head_row != f->tail_row) {
		tl = f->head_row;
		f->head_row = f->head_row->next_char;
		free(tl);
	}
	free(f->head_row);
	freeUndo(global_df->UndoList);
}

char CheckLeftCol(long i) {
	if (global_df->row_win2 || global_df->col_win2 != -2)
		return(FALSE);
	else if (i >= (long)global_df->num_of_cols + global_df->LeftCol) {
		global_df->LeftCol = i - ((long)global_df->num_of_cols / 2L) + 1L;
		if (global_df->LeftCol < 0)
			global_df->LeftCol = 0;
		return(TRUE);
	} else if (i < global_df->LeftCol) {
		if (i < (long)global_df->num_of_cols)
			global_df->LeftCol = 0;
		else {
			global_df->LeftCol = i - ((long)global_df->num_of_cols / 2L) + 1L;
			if (global_df->LeftCol < 0)
				global_df->LeftCol = 0;
		}
		return(TRUE);
	}
	return(FALSE);
}

void FindMidWindow(void) {
	register int num;
#if defined(_MAC_CODE) || defined(_WIN32)
	register int tTextWinSize = global_df->TextWinSize;
	ROWS *line = global_df->top_win;
#endif

	if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) {
		global_df->row_win = 0L;
		global_df->top_win = global_df->row_txt;
#if defined(_MAC_CODE) || defined(_WIN32)
		if (line == global_df->head_text) line = ToNextRow(line,FALSE);
		global_df->EdWinSize = 1;
		do {
			tTextWinSize -= line->Font.FHeight;
			if (tTextWinSize < 0 || global_df->EdWinSize > global_df->w1->num_rows)
				break;
			global_df->EdWinSize++;
			line = ToNextRow(line,FALSE);
		} while (line != global_df->tail_text) ;
		global_df->EdWinSize--;
		num = global_df->EdWinSize;
		if (tTextWinSize > 0) {
			num = num + tTextWinSize / global_df->top_win->Font.FHeight;
		}
		num = num / 2;
#else
		num = (global_df->EdWinSize / 2);
#endif
		while (!AtTopEnd(global_df->top_win,global_df->head_text,FALSE) && num > 0) {
			num--;
			global_df->row_win++;
			global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
		}
	}
	CheckLeftCol(global_df->col_win);
}

void CpCur_lineAttToHead_rowAtt(ROWS *cl) {
	AttTYPE *s;
	LINE *tl;

	if (cl->att != NULL) {
		global_df->head_row->att = 1;
		s = cl->att;
		tl = global_df->head_row->next_char;
		while (tl != global_df->tail_row) {
			tl->att = *s;
			s++;
			tl = tl->next_char;
		}
		if (global_df->DataChanged == FALSE)
			global_df->DataChanged = TRUE;
	}
}

void CpCur_lineToHead_row(ROWS *cl) {
	unCH *s;
	AttTYPE *s1;

	if (cl->line == NULL)
		return;
	if (cl->att == NULL) {
		for (s=cl->line; *s; s++) {
			AddCharAfter(*s);
			global_df->col_txt->att = 0;
		}
	} else {
		global_df->head_row->att = 1;
		for (s=cl->line,s1=cl->att; *s; s++, s1++) {
			AddCharAfter(*s);
			global_df->col_txt->att = *s1;
		}
	}
}

static void CpHead_rowToCur_line(void) {
	unCH *s;
	AttTYPE *s1;
	LINE *tl, *fl;

	if (global_df->head_row_len >= (int)strlen(global_df->cur_line->line)) {
		free(global_df->cur_line->line);
		if (global_df->cur_line->att)
			free(global_df->cur_line->att);

		global_df->cur_line->att = NULL;
		if (global_df->head_row->att) {
			global_df->cur_line->att = (AttTYPE *)malloc((global_df->head_row_len+1)*sizeof(AttTYPE));
		}
		global_df->cur_line->line = (unCH *)malloc((global_df->head_row_len+1) * sizeof(unCH));
		if (global_df->cur_line->line == NULL) {
//			global_df->cur_line->line = "";
			mem_err(TRUE, global_df);
		}
	}
	s = global_df->cur_line->line;
	s1 = global_df->cur_line->att;
	tl = global_df->head_row->next_char;
	while (tl != global_df->tail_row) {
		*s++ = tl->c;
		if (s1 != NULL)
			*s1++ = tl->att;
		fl = tl;
		tl = tl->next_char;
		free(fl);
	}
	*s = EOS;
}

void ChangeCurLineAlways(short which) {
	char tDataChanged;
	long j;

	tDataChanged = global_df->DataChanged;
	if (which == 1 || which == 0) {
		CpHead_rowToCur_line();
		global_df->head_row->next_char = global_df->tail_row;
		global_df->tail_row->prev_char = global_df->head_row;
		global_df->head_row_len = 0L;
		global_df->col_txt  = global_df->head_row;
		global_df->cur_line = global_df->row_txt;
	}
	if (which == 2 || which == 0) {
		CpCur_lineToHead_row(global_df->cur_line);
		for (global_df->col_txt=global_df->head_row->next_char, j=0L; global_df->col_txt != global_df->tail_row && j < global_df->col_chr; j++)
			global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_chr = j;
	}
	if (!tDataChanged) 
		global_df->DataChanged = '\0';
}

void ChangeCurLine(void) {
	if (global_df->row_txt != global_df->cur_line) {
		long j;

		CpHead_rowToCur_line();
		global_df->head_row->next_char = global_df->tail_row;
		global_df->tail_row->prev_char = global_df->head_row;
		global_df->head_row_len = 0L;
		global_df->col_txt = global_df->head_row;
		global_df->cur_line = global_df->row_txt;
		CpCur_lineToHead_row(global_df->cur_line);
		for (global_df->col_txt=global_df->head_row->next_char, j=0L; global_df->col_txt != global_df->tail_row && j < global_df->col_chr; j++)
			global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_chr = j;
	} else
	if (global_df->DataChanged == FALSE)
		global_df->DataChanged = TRUE;
}

static void AddCharAfter(unCH c) {
	LINE *NewChar;

	if (global_df->DataChanged == FALSE)
		global_df->DataChanged = TRUE;
	NewChar = NEW(LINE);
	if (NewChar == NULL) mem_err(TRUE, global_df);
	NewChar->next_char = global_df->col_txt->next_char;
	NewChar->prev_char = global_df->col_txt;
	global_df->col_txt->next_char->prev_char = NewChar;
	global_df->col_txt->next_char = NewChar;
	global_df->head_row_len = global_df->head_row_len + 1;
	NewChar->c = c;
	if (NewChar->prev_char != global_df->head_row)
		NewChar->att = global_df->gAtt;
	else
		NewChar->att = 0;
	global_df->col_txt = NewChar;
}

static void AddRowAfter(unCH *buf, char RED, FONTINFO *fontInfo, NewFontInfo *finfo) {
	ROWS *NewRow;

	if (global_df->RowLimit && global_df->numberOfRows >= global_df->RowLimit &&
		global_df->curRowLimit == global_df->head_text->next_row) {
		global_df->RowLimit++;
	}
	if (global_df->RowLimit && global_df->numberOfRows >= global_df->RowLimit) {
		if (global_df->top_win == global_df->head_text->next_row)
			global_df->top_win = global_df->head_text->next_row->next_row;
		if (global_df->row_txt == global_df->head_text->next_row)
			global_df->row_txt = global_df->head_text->next_row->next_row;
		if (global_df->cur_line == global_df->head_text->next_row)
			ChangeCurLineAlways(0);
		NewRow = global_df->head_text->next_row;
		NewRow->next_row->prev_row = global_df->head_text;
		global_df->head_text->next_row = NewRow->next_row;
		if (NewRow->line != NULL)
			free(NewRow->line);
		if ((NewRow->line=(unCH *)malloc((strlen(buf)+1)*sizeof(unCH)))==NULL)
			mem_err(TRUE, global_df);
		if (NewRow->att != NULL)
			free(NewRow->att);
		global_df->numberOfRows--;
		global_df->lineno--;
		if (global_df->curRowLimit == NewRow)
			global_df->isOutputScrolledOff = TRUE;
	} else {
		NewRow = NEW(ROWS);
		if (NewRow == NULL) 
			mem_err(TRUE, global_df);
		if ((NewRow->line=(unCH *)malloc((strlen(buf)+1)*sizeof(unCH)))==NULL)
			mem_err(TRUE, global_df);
	}
	NewRow->next_row = global_df->row_txt->next_row;
	NewRow->prev_row = global_df->row_txt;
	global_df->row_txt->next_row->prev_row = NewRow;
	global_df->row_txt->next_row = NewRow;

	NewRow->flag = 0;
//	TRUE_CHECK_ID1(NewRow->flag);
//	TRUE_CHECK_ID2(NewRow->flag);
	TRUE_VIS_ID(NewRow->flag);
	
	if (fontInfo != NULL) {
		SetFontName(NewRow->Font.FName, fontInfo->FName);
		NewRow->Font.FSize = fontInfo->FSize;
		NewRow->Font.CharSet = fontInfo->CharSet;
		NewRow->Font.FHeight = fontInfo->FHeight;
	} else if (finfo != NULL) {
#if defined(_WIN32)
		SetFontName(NewRow->Font.FName, finfo->fontName);
#else
		SetFontName(NewRow->Font.FName, finfo->fontId);
#endif /* else _WIN32 */
		NewRow->Font.FSize = finfo->fontSize;
		NewRow->Font.CharSet = finfo->CharSet;
		NewRow->Font.FHeight = finfo->charHeight;
	}
	strcpy(NewRow->line, buf);
	NewRow->att = NULL;

	global_df->gAtt = 0;
	global_df->row_txt = NewRow;
	global_df->lineno++;
	global_df->numberOfRows++;
#if defined(_MAC_CODE) || defined(_WIN32)
	if (RED)
		RecompEdWinSize();
#endif
}

void AddString(unCH *s, long len, char UpdateUndo) {
	unCH c = *s;
	AttTYPE att = 0;

	while (*s == ATTMARKER && len > 1) {
		SetTextAtt(NULL, *(s+1), &att);
		s += 2;
		len -= 2;
		global_df->head_row->att = 1;
	}
	if (*s == SNL_C) {
		if (*(s+1) == NL_C && len > 1) {
			s++;
			len--;
			while (*s == ATTMARKER && len > 1) {
				SetTextAtt(NULL, *(s+1), &att);
				s += 2;
				len -= 2;
				global_df->head_row->att = 1;
			}
		} else {
			global_df->col_txt = global_df->col_txt->next_char;
			global_df->redisplay = 0;
			NewLine(-1);
			global_df->redisplay = 1;
			global_df->col_txt = global_df->col_txt->prev_char;
		}
	}
	if (*s == NL_C/* && !global_df->ShowParags*/) {
		if (*(s+1) == SNL_C && len > 1) {
			s++;
			len--;
			while (*s == ATTMARKER && len > 1) {
				SetTextAtt(NULL, *(s+1), &att);
				s += 2;
				len -= 2;
				global_df->head_row->att = 1;
			}
		}
		AddCharAfter(NL_C);
		global_df->col_txt->att = att;
		global_df->col_chr++;
		global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		global_df->redisplay = 0;
		NewLine(-1);
		global_df->redisplay = 1;
		global_df->col_txt = global_df->col_txt->prev_char;
	} else if (*s != SNL_C && *s != '\0') {
		AddCharAfter(*s);
		global_df->col_txt->att = att;
		global_df->col_chr++;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	}
	s++;
	len--;
	while (*s == ATTMARKER && len > 1) {
		SetTextAtt(NULL, *(s+1), &att);
		s += 2;
		len -= 2;
		global_df->head_row->att = 1;
	}
	if (isSpeaker(c) && UpdateUndo && global_df->ChatMode) {
		long new_col;
		LINE *tl=global_df->col_txt->prev_char;
		if (tl != global_df->head_row) {
			while (isSpace(tl->c) && tl != global_df->head_row) tl=tl->prev_char;
			if (tl == global_df->head_row) {
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = DELTRPT;
				}
				new_col = global_df->col_chr;
				for (tl=global_df->head_row->next_char; tl != global_df->col_txt; tl=tl->next_char)
					global_df->col_chr--;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				if (UpdateUndo) {
					new_col -= global_df->col_chr;
					if ((global_df->UndoList->str=(unCH *)malloc((new_col+1)*sizeof(unCH)))== NULL) 
						mem_err(TRUE, global_df);
					new_col = 0L;
				}
				while (global_df->head_row->next_char != global_df->col_txt) {
					tl = global_df->head_row->next_char;
					global_df->head_row->next_char = tl->next_char;
					global_df->head_row_len--;
					if (UpdateUndo)
						global_df->UndoList->str[new_col++] = tl->c;
					free(tl);
				}
				if (UpdateUndo)
					global_df->UndoList->str[new_col] = EOS;

				if (UpdateUndo) {
					global_df->col_chr--;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
					global_df->col_chr++;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
				global_df->col_txt->prev_char = global_df->head_row;
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = INSTRPT;
				}
			}
		}
	}

	for (; len > 0L; s++, len--) {
		while (*s == ATTMARKER && len > 1) {
			SetTextAtt(NULL, *(s+1), &att);
			s += 2;
			len -= 2;
			global_df->head_row->att = 1;
		}
		if (*s == SNL_C) {
			if (*(s+1) == NL_C && len > 1) {
				s++;
				len--;
				while (*s == ATTMARKER && len > 1) {
					SetTextAtt(NULL, *(s+1), &att);
					s += 2;
					len -= 2;
					global_df->head_row->att = 1;
				}
			} else {
				global_df->col_txt = global_df->col_txt->next_char;
				global_df->redisplay = 0;
				NewLine(-1);
				global_df->redisplay = 1;
				global_df->col_txt = global_df->col_txt->prev_char;
				if (len == 1) break;
			}
		}
		if (*s == NL_C/* && !global_df->ShowParags*/) {
			if (*(s+1) == SNL_C && len > 1) {
				s++;
				len--;
				while (*s == ATTMARKER && len > 1) {
					SetTextAtt(NULL, *(s+1), &att);
					s += 2;
					len -= 2;
					global_df->head_row->att = 1;
				}
			}
			AddCharAfter(NL_C);
			global_df->col_txt->att = att;
			global_df->col_chr++;
			global_df->col_txt = global_df->col_txt->next_char;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			if (isCallFixLine(0L, global_df->head_row_len, TRUE) && UpdateUndo) {
				FixLine(UpdateUndo);
				if (global_df->col_txt->c == NL_C || global_df->col_txt->c == SNL_C) {
					global_df->col_chr++;
					global_df->col_txt = global_df->col_txt->next_char;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}					
				SaveUndoState(FALSE);
				global_df->UndoList->key = INSTRPT;
			}
			global_df->redisplay = 0;
			NewLine(-1);
			global_df->redisplay = 1;
			global_df->col_txt = global_df->col_txt->prev_char;
		} else if (*s != SNL_C && *s != '\0') {
			AddCharAfter(*s);
			global_df->col_txt->att = att;
			global_df->col_chr++;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
	}
}

void AddText(unCH *s, unCH c, short DisplayAll, long len) {
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return;
	}

	if (global_df->UndoList->NextUndo) {
		if (global_df->UndoList->key != INSTRPT)
			global_df->UndoList->key = INSTKEY;
	}
	ChangeCurLine();

	if (global_df->col_txt == global_df->tail_row && global_df->col_txt->prev_char->c == NL_C) {
		global_df->UndoList->key = MOVEKEY;
		global_df->col_chr--;
		global_df->col_txt = global_df->col_txt->prev_char;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		SaveUndoState(FALSE);
		global_df->UndoList->key = INSTRPT;
	}
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	global_df->col_txt = global_df->col_txt->prev_char;
	if (s != NULL) {
		AddString(s, len, (char)(global_df->UndoList->NextUndo != NULL));
	} else {
		if (c != EOS) {
			global_df->col_chr++;
			AddCharAfter(c);
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
		if (isSpeaker(c) && global_df->ChatMode) {
			long new_col;
			LINE *tl=global_df->col_txt->prev_char;
			if (tl != global_df->head_row) {
				while (isSpace(tl->c) && tl != global_df->head_row) tl=tl->prev_char;
				if (tl == global_df->head_row) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = DELTRPT;
					new_col = global_df->col_chr;
					for (tl=global_df->head_row->next_char; tl != global_df->col_txt; tl=tl->next_char)
						global_df->col_chr--;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					new_col -= global_df->col_chr;
					if ((global_df->UndoList->str=(unCH *)malloc((new_col+1)*sizeof(unCH)))== NULL) 
						mem_err(TRUE, global_df);
					new_col = 0L;
					while (global_df->head_row->next_char != global_df->col_txt) {
						tl = global_df->head_row->next_char;
						global_df->head_row->next_char = tl->next_char;
						global_df->head_row_len--;
						global_df->UndoList->str[new_col++] = tl->c;
						free(tl);
					}
					global_df->UndoList->str[new_col] = EOS;
					global_df->col_chr--;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
					global_df->col_chr++;
					global_df->col_txt->prev_char = global_df->head_row;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
			}
		}
	}

	global_df->col_txt = global_df->col_txt->next_char;
	if (DisplayAll == -2)
		DisplayTextWindow(global_df->cur_line, 1);
	else if (DisplayAll == -1) ;
	else if (global_df->ChatMode || global_df->AutoWrap) {
		if (isCallFixLine(0L, global_df->head_row_len, TRUE)) {
			FixLine(TRUE);
			DisplayTextWindow(NULL, 1);
		} else if (DisplayAll == 1) DisplayTextWindow(NULL, 1);
		else DisplayTextWindow(global_df->cur_line, 1);
	} else {
		if (DisplayAll == 1) DisplayTextWindow(NULL, 1);
		else DisplayTextWindow(global_df->cur_line, 1);
	}
}

void AddCodeTier(unCH *code, char real_code) {
	int  BlankLine;

	global_df->FreqCount++;
	if (real_code == '\002') {
		if (global_df->row_txt == global_df->cur_line) {
			LINE *tl = global_df->head_row->next_char;
			while (tl != global_df->tail_row && isSpace(tl->c))
				tl = tl->next_char;
			BlankLine = (tl == global_df->tail_row);
		} else {
			for (BlankLine=0; isSpace(global_df->row_txt->line[BlankLine]); BlankLine++) ;
			BlankLine = (global_df->row_txt->line[BlankLine] == EOS);
		}
		if (!BlankLine) {
			EndOfLine(-1);
			SaveUndoState(FALSE);
			AddRowAfter(cl_T(""), TRUE, &global_df->row_txt->Font, NULL);
			global_df->row_win++;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			FindMidWindow();
		}
//		if (*code != '%' && *code != '@')
//			AddLineNumber(-1);
		SaveUndoState(FALSE);
		AddText(code, EOS, !BlankLine, (long)strlen(code));
	} else if (global_df->CurCode == global_df->RootCodes && global_df->ChatMode && (global_df->cod_fname != NULL || !real_code)) {
		global_df->redisplay = 0;
		while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
			if (AtBotEnd(global_df->row_txt, global_df->cur_line, FALSE)) {
				if (!isSpace(global_df->head_row->next_char->c) || 
							 global_df->head_row->next_char == global_df->tail_row)
					break;
			} else {
				if (!isSpace(*global_df->row_txt->next_row->line))
					break;
			}
			MoveDown(-1);
		}
		if (global_df->row_txt == global_df->cur_line) {
			LINE *tl = global_df->head_row->next_char;
			while (tl != global_df->tail_row && isSpace(tl->c))
				tl = tl->next_char;
			BlankLine = (tl == global_df->tail_row);
		} else {
			for (BlankLine=0; isSpace(global_df->row_txt->line[BlankLine]); BlankLine++) ;
			BlankLine = (global_df->row_txt->line[BlankLine] == EOS);
		}
		global_df->redisplay = 1;
		FindMidWindow();
		if (!BlankLine) {
			EndOfLine(-1);
			SaveUndoState(FALSE);
			AddRowAfter(cl_T(""), TRUE, &global_df->row_txt->Font, NULL);
			global_df->row_win++;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			FindMidWindow();
		}
		AddText(NULL, code[0], 0, 1L);
		if (code[1])
			AddText(code+1, EOS, 1, (long)strlen(code+1));
	} else
		AddText(code, EOS, 0, (long)strlen(code));
	if (real_code == TRUE)
		MoveToSubcode();
}

static void collectLine(unCH *dest, long beg_c, long col_c, char isIgnoreBullets) {
	long col, len;
	long colWin, newcol;
	LINE *ColTxt;
	colWin = 0L;
	ColTxt = global_df->head_row->next_char;
	for (col=0L; ColTxt != global_df->tail_row && col < beg_c; ColTxt=ColTxt->next_char, col++) {
		if (ColTxt->c == '\t') {
			newcol = (((colWin / TabSize) + 1) * TabSize);
			for (; colWin < newcol; colWin++) ;
		} else if (ColTxt->c == HIDEN_C && (global_df->ShowParags != '\002' || isIgnoreBullets)) {
			col++;
			ColTxt = ColTxt->next_char;
			if (ColTxt == global_df->tail_row || col >= beg_c)
				break;
			for (; ColTxt != global_df->tail_row && col < beg_c && ColTxt->c != HIDEN_C; ColTxt=ColTxt->next_char, col++) ;
			if (ColTxt->c == HIDEN_C && col < beg_c) {
				colWin++;
			}
		} else if (ColTxt->c == NL_C && global_df->ShowParags != '\001') ;
		else {
			colWin++;
		}
	}
	for (len=0L; ColTxt != global_df->tail_row && col < col_c && len < UTTLINELEN-1; 
						ColTxt=ColTxt->next_char, col++) {
		if (ColTxt->c == '\t') {
			newcol = (((colWin / TabSize) + 1) * TabSize);
			for (; colWin < newcol; colWin++) 
				dest[len++] = ' ';
		} else if (ColTxt->c == HIDEN_C && (global_df->ShowParags != '\002' || isIgnoreBullets)) {
			col++;
			ColTxt = ColTxt->next_char;
			if (ColTxt == global_df->tail_row || col >= col_c)
				break;
			for (; ColTxt != global_df->tail_row && col < col_c && ColTxt->c != HIDEN_C; ColTxt=ColTxt->next_char, col++) ;
			if (ColTxt->c == HIDEN_C && col < col_c) {
				dest[len++] = ColTxt->c;
				colWin++;
			}
		} else if (ColTxt->c == NL_C && global_df->ShowParags != '\001') ;
		else {
			colWin++;
			dest[len++] = ColTxt->c;
		}
	}
	dest[len] = EOS;
}

char isCallFixLine(long beg_c, long col_c, char buildLine) {
	int  width;
	long len;

	if (!doReWrap || !global_df->ChatMode) {
		if (beg_c == 0L)
			return((char)(ComColWin(TRUE, NULL, col_c) >= SCREENWIDTH));
		else {
			return((char)((ComColWin(TRUE, NULL, col_c) - ComColWin(TRUE, NULL, beg_c)) >= SCREENWIDTH));
		}
	} else {
#ifdef _MAC_CODE
		GrafPtr savePort;
		Rect box;

		GetPort(&savePort);
	 	SetPortWindowPort(global_df->wind);
		TextFont(global_df->row_txt->Font.FName);
		TextSize(global_df->row_txt->Font.FSize);
		GetWindowPortBounds(global_df->wind, &box);
		width = box.right-box.left-LEFTMARGIN-global_df->w1->textOffset-SCROLL_BAR_SIZE;
		if (buildLine)
			collectLine(templine4, beg_c, col_c, TRUE);
		len = ComColWin(FALSE, templine4, strlen(templine4));
		if (TextWidthInPix(templine4,0,len,&global_df->row_txt->Font,0) >= width) {
			SetPort(savePort);
			return(TRUE);
		}
		SetPort(savePort);
#elif defined(_WIN32)
		RECT theRect;
		CSize tw;
		CWnd *gWin;
		LOGFONT lfFont;
		CFont l_font;

		SetLogfont(&lfFont, &global_df->row_txt->Font, NULL);
		l_font.CreateFontIndirect(&lfFont);
		CFont* pOldFont = GlobalDC->SelectObject(&l_font);
		gWin = GlobalDC->GetWindow();
		if (!gWin)
			return(TRUE);
		gWin->GetClientRect(&theRect);		
		width = theRect.right - theRect.left - LEFTMARGIN - global_df->w1->textOffset - SCROLL_BAR_SIZE;
		if (buildLine)
			collectLine(templine4, beg_c, col_c, TRUE);
		len = strlen(templine4);
		tw = GlobalDC->GetTextExtent(templine4, len);
		len = tw.cx;
		GlobalDC->SelectObject(pOldFont);
		l_font.DeleteObject();
		if (len >= width) {
			return(TRUE);
		}
#endif
	}
	return(FALSE);
}

long ComColWin(char isIgnoreBullets, unCH *line, long col_c) {
	long col, ColWin;

	ColWin = 0L;
	if (line == NULL) {
		LINE *ColTxt;

		for (ColTxt=global_df->head_row->next_char, col=0L;
				ColTxt != global_df->tail_row && col < col_c; ColTxt=ColTxt->next_char, col++) {
			if (ColTxt->c == '\t') {
				ColWin = (((ColWin / TabSize) + 1) * TabSize);
			} else if (ColTxt->c == HIDEN_C &&  (global_df->ShowParags != '\002' || isIgnoreBullets)) {
				col++;
				ColTxt = ColTxt->next_char;
				if (ColTxt == global_df->tail_row || col >= col_c)
					break;
				for (; ColTxt != global_df->tail_row && col < col_c && ColTxt->c != HIDEN_C; ColTxt=ColTxt->next_char, col++) ;
				if (ColTxt->c == HIDEN_C && col < col_c)
					ColWin++;
			} else if (ColTxt->c == NL_C && global_df->ShowParags != '\001') ;
			else {
				ColWin++;
			}
		}
	} else {
		for (col=0L; line[col] != EOS && col < col_c; col++) {
			if (line[col] == '\t') {
				ColWin = (((ColWin / TabSize) + 1) * TabSize);
			} else if (line[col] == HIDEN_C &&  (global_df->ShowParags != '\002' || isIgnoreBullets)) {
				col++;
				if (line[col] == EOS || col >= col_c)
					break;
				for (; line[col] != EOS && col < col_c && line[col] != HIDEN_C; col++) ;
				if (line[col] == HIDEN_C && col < col_c)
					ColWin++;
			} else if (line[col] == NL_C && global_df->ShowParags != '\001') ;
			else {
				ColWin++;
			}
		}
	}
	return(ColWin);
}

long ComColChr(unCH *line, long col_w) {
	long col, ColChr;

	ColChr = 0L;
	if (line == NULL) {
		LINE *ColTxt;

		for (ColTxt=global_df->head_row->next_char, col=0L;
				ColTxt != global_df->tail_row && col <= col_w; ColTxt=ColTxt->next_char, ColChr++) {
			if (ColTxt->c == '\t') {
				col = (((col / TabSize) + 1) * TabSize) - 1;
			} else if (ColTxt->c == HIDEN_C && global_df->ShowParags != '\002') {
				col++;
				ColTxt = ColTxt->next_char;
				if (ColTxt == global_df->tail_row || col > col_w)
					break;
				for (; ColTxt != global_df->tail_row && col <= col_w && ColTxt->c != HIDEN_C;  ColTxt=ColTxt->next_char, ColChr++) ;
			} else {
				col++;
			}
		}
		if (ColTxt != global_df->tail_row || col > col_w) {
			if (ColChr > 0L)
				ColChr--;
		}
	} else {
		for (col=0L; line[ColChr] != EOS && col <= col_w; ColChr++) {
			if (line[ColChr] == '\t') {
				col = (((col / TabSize) + 1) * TabSize) - 1;
			} else if (line[ColChr] == HIDEN_C && global_df->ShowParags != '\002') {
				col++;
				ColChr++;
				if (line[ColChr] == EOS || col >= col_w)
					break;
				for (; line[ColChr] != EOS && col <= col_w && line[ColChr] != HIDEN_C; ColChr++) ;
			} else {
				col++;
			}
		}
		if (line[ColChr] != EOS || col > col_w) {
			if (ColChr > 0L)
				ColChr--;
		}
	}
	return(ColChr);
}

static void FixClanOutputLine(unCH *s) {
	char res;
	long i, win, offset;

	if (!doReWrap || !global_df->ChatMode) {
		win = ComColWin(FALSE, NULL, global_df->col_chr);
		if (win == 0L && (!strncmp(s, "*** File \"", 10) || !strncmp(s, "    File \"", 10)))
			return;
	} else {
		collectLine(templine4, 0L, global_df->col_chr, FALSE);
		win = strlen(templine4);
	}
	offset = 0L;
	for (i=0; s[i]; i++) {
		if (s[i] == '\n') {
			win = 0L;
			offset = i + 1;		
		} else {
			if (!doReWrap || !global_df->ChatMode) {
				res = (char)(win+ComColWin(FALSE, s+offset, i-offset) >= SCREENWIDTH);
			} else {
				strncpy(templine4+win, s+offset, i-offset);
				templine4[win+i-offset] = EOS;
				res = isCallFixLine(0L, 0L, FALSE);
			}
			if (isSpace(s[i]) && res) {
				s[i] = '\n';
				offset = i + 1;
				win = 0L;
			}
		}
	}
}

static void OutputNewLine(void) {
	long num = 0L;
	LINE *tl, *ttail_row;

	ChangeCurLine();
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	for (tl=global_df->tail_row; tl != global_df->col_txt; tl=tl->prev_char)
		num++;
	ttail_row = global_df->tail_row;
	global_df->tail_row = tl;
	global_df->head_row_len -= num;
	CpHead_rowToCur_line();
	global_df->head_row_len = num;
	global_df->head_row->next_char = tl;
	tl->prev_char = global_df->head_row;
	global_df->tail_row = ttail_row;
	AddRowAfter(cl_T(""), TRUE, NULL, &dFnt);
	global_df->cur_line = global_df->row_txt;
	for (global_df->col_chr=0,tl=global_df->head_row->next_char; tl!= global_df->col_txt; tl=tl->next_char)
		global_df->col_chr++;
	global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	global_df->row_win++;
	if (global_df->row_win >= (long)global_df->EdWinSize ||
		global_df->row_win >= (long)global_df->w1->num_rows) {
		global_df->row_win--;
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
	}
	return;
}

void OutputToScreen(unCH *s) {
	unCH *t;
	long len;
	short nlFound;
	myFInfo *saveGlobal_df;
#ifdef _MAC_CODE
	WindowProcRec	*rec;	
	extern WindowProcRec *FindAProcRecID(short id);
#endif

	saveGlobal_df = global_df;
#ifdef _MAC_CODE
	if (global_df == NULL || global_df->winID != 1964) {
		if ((rec=FindAProcRecID(1964)) != NULL) {
			global_df = rec->FileInfo;
		}
	}
#endif
	if (global_df) {
//		CheckForInterupt();
		SysEventCheck(150L);
		if (isKillProgram)
			global_df->attOutputToScreen = 0;
		if (*s == EOS) {
			global_df = saveGlobal_df;
			return;
		}

		ChangeCurLine();
		if (global_df->AutoWrap) {
			FixClanOutputLine(s);
		}
		for (t=s, len=0; *t; t++, len++) {
			if (*t == '\n')
				*t = NL_C;
		}
		if (global_df->col_txt == global_df->tail_row && global_df->col_txt->prev_char->c == NL_C) {
			global_df->col_chr--;
			global_df->col_txt = global_df->col_txt->prev_char;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
#if defined(_WIN32)
		SetFontName(global_df->row_txt->Font.FName, dFnt.fontName);
#else
		SetFontName(global_df->row_txt->Font.FName, dFnt.fontId);
#endif /* else _WIN32 */
		global_df->row_txt->Font.FSize = dFnt.fontSize;
		global_df->row_txt->Font.CharSet = dFnt.CharSet;
		global_df->row_txt->Font.FHeight = dFnt.charHeight;
		if (dFnt.isUTF)
			global_df->isUTF = dFnt.isUTF;
//		TRUE_CHECK_ID1(global_df->row_txt->flag);
//		TRUE_CHECK_ID2(global_df->row_txt->flag);
		global_df->col_txt = global_df->col_txt->prev_char;
		nlFound = FALSE;
		if (*s == NL_C/* && !global_df->ShowParags*/) {
			global_df->attOutputToScreen = 0;
			AddCharAfter(NL_C);
			global_df->col_txt->att = global_df->attOutputToScreen;
			global_df->col_chr++;
			global_df->col_txt = global_df->col_txt->next_char;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			OutputNewLine();
			nlFound = TRUE;
			global_df->col_txt = global_df->col_txt->prev_char;
		} else if (*s == '\r') {
			LINE *tl=global_df->head_row->next_char;

			global_df->attOutputToScreen = 0;
			if (tl != global_df->tail_row) {
				while (global_df->head_row->next_char != global_df->tail_row) {
					tl = global_df->head_row->next_char;
					global_df->head_row->next_char = tl->next_char;
					free(tl);
				}
				global_df->tail_row->prev_char = global_df->head_row;
				global_df->head_row_len = 0L;
				global_df->col_txt  = global_df->head_row;
				global_df->col_chr = 0;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			}
		} else {
			while (*s == ATTMARKER) {
				if (SetTextAtt(NULL, *(s+1), &global_df->attOutputToScreen)) {
					s += 2;
					len -= 2L;
				} else
					break;
			}
			if (len > 0L) {
				if (*s == 0x202f)
					*s = ' ';
				AddCharAfter(*s);
				if (global_df->attOutputToScreen != 0)
					global_df->head_row->att = 1;
				global_df->col_txt->att = global_df->attOutputToScreen;
				global_df->col_chr++;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			}
		}
		s++;
		len--;

		for (; len > 0L; s++, len--) {
			if (*s == NL_C/* && !global_df->ShowParags*/) {
				global_df->attOutputToScreen = 0;
				AddCharAfter(NL_C);
				global_df->col_txt->att = global_df->attOutputToScreen;
				global_df->col_chr++;
				global_df->col_txt = global_df->col_txt->next_char;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				OutputNewLine();
				nlFound = TRUE;
				global_df->col_txt = global_df->col_txt->prev_char;
			} else if (*s == '\r') {
				LINE *tl=global_df->head_row->next_char;

				global_df->attOutputToScreen = 0;
				if (tl != global_df->tail_row) {
					while (global_df->head_row->next_char != global_df->tail_row) {
						tl = global_df->head_row->next_char;
						global_df->head_row->next_char = tl->next_char;
						free(tl);
					}
					global_df->tail_row->prev_char = global_df->head_row;
					global_df->head_row_len = 0L;
					global_df->col_txt  = global_df->head_row;
					global_df->col_chr = 0;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
				nlFound = TRUE;
			} else if (*s == ATTMARKER) {
				while (*s == ATTMARKER) {
					if (SetTextAtt(NULL, *(s+1), &global_df->attOutputToScreen)) {
						s += 2;
						len -= 2L;
					} else {
						s++;
						len--;
						break;
					}
				}
				s--;
				len++;
			} else {
				if (*s == 0x202f)
					*s = ' ';
				AddCharAfter(*s);
				if (global_df->attOutputToScreen != 0)
					global_df->head_row->att = 1;
				global_df->col_txt->att = global_df->attOutputToScreen;
				global_df->col_chr++;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			}
		}
		global_df->col_txt = global_df->col_txt->next_char;
		if (nlFound)
			DisplayTextWindow(NULL, 2);
		else
			DisplayTextWindow(global_df->cur_line, 2);
	}
	global_df = saveGlobal_df;
}

void DisplayRow(char isShift) {
	FindMidWindow();
	if (isShift) {
		if (global_df->col_win >= (long)global_df->num_of_cols + global_df->LeftCol) {
			global_df->LeftCol = global_df->col_win - ((long)global_df->num_of_cols / 2L) + 1L;
			if (global_df->LeftCol < 0)
				global_df->LeftCol = 0;
		} else if (global_df->col_win < global_df->LeftCol) {
			if (global_df->col_win < (long)global_df->num_of_cols)
				global_df->LeftCol = 0;
			else {
				global_df->LeftCol = global_df->col_win - ((long)global_df->num_of_cols / 2L) + 1L;
				if (global_df->LeftCol < 0)
					global_df->LeftCol = 0;
			}
		}
	}
	DisplayTextWindow(NULL, 1);
	GetCurCode();
	FindRightCode(1);
	strcpy(global_df->err_message, DASHES);
}

int NewLine(int i) {
	long num = 0L;
	LINE *tl, *ttail_row;

	if (i > -1) {
		if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
		if (!DeleteChank(1)) return(21);
		SaveUndoState(FALSE);
		if (global_df->UndoList->NextUndo)
			global_df->UndoList->key = INSTKEY;
	}
	ChangeCurLine();
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	if (!global_df->ChatMode && i > -1) {
		AddText(NULL, (unCH)NL_C, -1, 1L);
		ChangeCurLine();
	}
	for (tl=global_df->tail_row; tl != global_df->col_txt; tl=tl->prev_char) num++;
	ttail_row = global_df->tail_row;
	global_df->tail_row = tl;
	global_df->head_row_len -= num;
	CpHead_rowToCur_line();
	global_df->head_row_len = num;
	global_df->head_row->next_char = tl;
	tl->prev_char = global_df->head_row;
	global_df->tail_row = ttail_row;
	AddRowAfter(cl_T(""), TRUE, &global_df->row_txt->Font, NULL);
	global_df->cur_line = global_df->row_txt;
	if (i != -1 && global_df->ChatMode) {
		if (global_df->head_row->next_char == global_df->tail_row || 
			!isSpeaker(global_df->head_row->next_char->c)) {
			global_df->col_txt = global_df->head_row;
			AddCharAfter('\t');
			global_df->col_txt = tl;
		}
	}
	for (global_df->col_chr=0,tl=global_df->head_row->next_char; tl!= global_df->col_txt; tl=tl->next_char)
		global_df->col_chr++;
	global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	if (++global_df->row_win >= (long)global_df->EdWinSize) {
		global_df->row_win--;
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
	}
	if (global_df->redisplay) DisplayTextWindow(NULL, 1);
	return(21);
}

char AtTopEnd(ROWS *p, ROWS *head, char all) {
	if (!global_df->ChatMode || all) {
		if (p->prev_row == head) return(TRUE);
		else return(FALSE);
	} else {
		while (1) {
			if (p->prev_row == head) return(TRUE);
			p = p->prev_row;
			if (CMP_VIS_ID(p->flag)) return(FALSE);
		}
	}
}

ROWS *ToPrevRow(ROWS *p, char all) {
	if (!global_df->ChatMode) {
		return(p->prev_row);
	} else if (all) {
		ROWS *t;
		p = p->prev_row;
		for (t=p; t != global_df->head_text; t=t->prev_row) {
			TRUE_VIS_ID(t->flag);
			if (t == global_df->cur_line) {
				LINE *tl= global_df->head_row->next_char;
				if (tl != global_df->tail_row && isSpeaker(tl->c))
					break;
			} else {
				if (isSpeaker(*t->line))
					break;
			}
		}
		return(p);
	} else {
		while (p != global_df->head_text) {
			p = p->prev_row;
			if (CMP_VIS_ID(p->flag)) return(p);
		}
		return(p);
	}
}

char AtBotEnd(ROWS *p, ROWS *tail, char all) {
	if (!global_df->ChatMode || all) {
		if (p->next_row == tail) return(TRUE);
		else return(FALSE);
	} else {
		while (1) {
			if (p->next_row == tail) return(TRUE);
			p = p->next_row;
			if (CMP_VIS_ID(p->flag)) return(FALSE);
		}
	} 
}

ROWS *ToNextRow(ROWS *p, char all) {
	if (!global_df->ChatMode) {
		return(p->next_row);
	} else if (all) {
		ROWS *t;
		p = p->next_row;
		TRUE_VIS_ID(p->flag);
		for (t=p->next_row; t != global_df->tail_text; t=t->next_row) {
			if (t == global_df->cur_line) {
				LINE *tl= global_df->head_row->next_char;
				if (tl != global_df->tail_row && isSpeaker(tl->c)) 
					break;
			} else {
				if (isSpeaker(*t->line))
					break;
			}
			TRUE_VIS_ID(t->flag);
		}
		return(p);
	} else {
		while (p != global_df->tail_text) {
			p = p->next_row;
			if (CMP_VIS_ID(p->flag)) return(p);
		}
		return(p);
	}
}

// re-wrap routines begin
static char isAtt(AttTYPE *att, long pos) {
	long i;
	
	for (i=0; i < pos; i++) {
		if (att[i] != 0)
			return(TRUE);
	}
	return(FALSE);
}

char AddLineToRow(ROWS *rt, unCH tierType, void *data) {
	long i, pos;

	if (rt->line[0] != EOS) {
		pos = strlen(rt->line);
		AddRowAfter(rt->line, FALSE, &rt->Font, NULL);
		if (rt->att != NULL && isAtt(rt->att, pos)) {
			if ((global_df->row_txt->att=(AttTYPE *)malloc((pos+1)*sizeof(AttTYPE)))==NULL)
				mem_err(TRUE, global_df);
			for (i=0; i < pos; i++)
				global_df->row_txt->att[i] = rt->att[i];
		}
	}
	return(TRUE);
}

static char isLineFull(long max, long pos, ROWS *rt) {
	int  width;
	long t, len;
	long colWin, newcol;
#ifdef _MAC_CODE
	GrafPtr savePort;
	Rect box;

	if (max == 0) {
		GetPort(&savePort);
	 	SetPortWindowPort(global_df->wind);
		TextFont(rt->Font.FName);
		TextSize(rt->Font.FSize);
		GetWindowPortBounds(global_df->wind, &box);
		width = box.right-box.left-LEFTMARGIN-global_df->w1->textOffset-SCROLL_BAR_SIZE;
		colWin = 0L;
		for (t=0L, len=0L; t < pos && len < UTTLINELEN-1; t++) {
			if (rt->line[t] == '\t') {
				newcol = (((colWin / TabSize) + 1) * TabSize);
				for (; colWin < newcol; colWin++) 
					templine2[len++] = ' ';
			} else {
				colWin++;
				templine2[len++] = rt->line[t];
			}
		}
		if (TextWidthInPix(templine2,0,len,&rt->Font,0) >= width) {
			SetPort(savePort);
			return(TRUE);
		}
		SetPort(savePort);
	}
#elif defined(_WIN32)
	RECT theRect;
	CSize tw;
	CWnd *gWin;
	LOGFONT lfFont;
	CFont l_font;

	if (max == 0) {
		SetLogfont(&lfFont, &rt->Font, NULL);
		l_font.CreateFontIndirect(&lfFont);
		CFont* pOldFont = GlobalDC->SelectObject(&l_font);
		gWin = GlobalDC->GetWindow();
		if (!gWin)
			return(TRUE);
		gWin->GetClientRect(&theRect);		
		width = theRect.right - theRect.left - LEFTMARGIN -
							global_df->w1->textOffset - SCROLL_BAR_SIZE;
		colWin = 0L;
		for (t=0L, len=0L; t < pos && len < UTTLINELEN-1; t++) {
			if (rt->line[t] == '\t') {
				newcol = (((colWin / TabSize) + 1) * TabSize);
				for (; colWin < newcol; colWin++) 
					templine2[len++] = ' ';
			} else {
				colWin++;
				templine2[len++] = rt->line[t];
			}
		}
		tw = GlobalDC->GetTextExtent(templine2, len);
		len = tw.cx;
		GlobalDC->SelectObject(pOldFont);
		l_font.DeleteObject();
		if (len >= width) {
			return(TRUE);
		}
	}
#endif
	else {
		colWin = 0L;
		for (t=0L; t < pos; t++) {
			if (rt->line[t] == '\t') {
				newcol = (((colWin / TabSize) + 1) * 8 /*TabSize*/);
				for (; colWin < newcol; colWin++) ;
			} else {
				colWin++;
			}
			if (colWin >= max)
				return(TRUE);
		}
	}	return(FALSE);
}

static void uAtt_cp(long pos, unCH *desSt, unCH *srcSt, AttTYPE *desAtt, AttTYPE *srcAtt) {
	long i;

	for (i=0; srcSt[i]; i++, pos++) {
		desSt[pos] = srcSt[i];
		desAtt[pos] = srcAtt[i];
	}
	desSt[pos] = EOS;
}

static char isLineEmpty(unCH *line) {
	long i;
	
	for (i=0; isBreakInLine(line[i]); i++) ;
	if (line[i] == EOS)
		return(TRUE);
	else
		return(FALSE);
}

char Re_WrapLines(char (*finLine)(ROWS *rt, unCH tierType, void *), long max, char isChange, void *data) {
	register long pos;
	register long lPos;
	register long oPos;
	char fTime, sq, hc;
	unCH tierType;
	char tDoReWrap;
	ROWS *rt, *tr, *end, newRow;
	LINE *tl, *t;

	tDoReWrap = doReWrap;
	if (!global_df->ChatMode)
		doReWrap = FALSE;

	if (!doReWrap && isChange) {
		doReWrap = tDoReWrap;
		return(TRUE);
	}

	rt = global_df->head_text->next_row;
	if (isChange) {
		ResetUndos();
		global_df->tail_text->prev_row->next_row = NULL;
		
		global_df->head_text->next_row = global_df->tail_text;
		global_df->head_text->prev_row = NULL;
		global_df->tail_text->next_row = NULL;
		global_df->tail_text->prev_row = global_df->head_text;
		global_df->row_txt = global_df->head_text;
		global_df->col_txt = global_df->head_row;
		global_df->top_win = global_df->row_txt;
	
		t = global_df->head_row->next_char;
		while (t != global_df->tail_row) {
			tl = t;
			t = t->next_char;
			free(tl);
		}
		global_df->head_row_len = 0L;
		global_df->head_row->c = 0;
		global_df->head_row->att = 0;
		global_df->head_row->next_char = global_df->tail_row;
		global_df->head_row->prev_char = NULL;
		global_df->tail_row->c = 0;
		global_df->tail_row->att = 0;
		global_df->tail_row->next_char = NULL;
		global_df->tail_row->prev_char = global_df->head_row;
		end = NULL;
	} else {
		end = global_df->tail_text;
	}

	ced_line[0] = EOS;
	templine4[0] = 0;
	newRow.line = ced_line;
	newRow.att  = tempAtt;
	copyFontInfo(&newRow.Font, &rt->Font, FALSE);
	pos = 0L;
	lPos = pos;
	tierType = 0;
	while (rt != end) {
		if (global_df->ChatMode) {
			if (rt->line[0] == '*' || rt->line[0] == '%' || rt->line[0] == '@')
				tierType = rt->line[0];
		}
		if (!doReWrap) {
			if (!(*finLine)(rt, tierType, data)) {
				doReWrap = tDoReWrap;
				return(FALSE);
			}
		} else {
			oPos = 0L;
			fTime = TRUE;
			if (isSpace(rt->line[oPos])) {
				fTime = FALSE;
				while (isSpace(rt->line[oPos]))
					oPos++;
				oPos--;
				if (pos > 0L && ced_line[pos-1] == ':')
					rt->line[oPos] = '\t';
				else
					rt->line[oPos] = ' ';
			}
			sq = FALSE;
			hc = FALSE;
			do {
				if (global_df->ChatMode) {
					if (!isSpeaker(*ced_line)) {
						if (*ced_line == ' ')
							*ced_line = '\t';
					}
					if (isLineFull(max, pos, &newRow)) {
						ced_line[pos] = EOS;
						ced_line[lPos] = EOS;
						if (!(*finLine)(&newRow, tierType, data)) {
							doReWrap = tDoReWrap;
							return(FALSE);
						}
						if (lPos == pos)
							ced_line[0] = EOS;
						else {
							if (lPos > 0L)
								uAtt_cp(0,ced_line+1,ced_line+lPos+1,tempAtt+1,tempAtt+lPos+1);
							ced_line[0] = '\t';
							tempAtt[0] = 0;
							if (isLineEmpty(ced_line))
								ced_line[0] = EOS;
						}
						pos = strlen(ced_line);
					}
					if (fTime && isSpeaker(*rt->line)) {
						ced_line[pos] = EOS;
						if (!(*finLine)(&newRow, tierType, data)) {
							doReWrap = tDoReWrap;
							return(FALSE);
						}
						pos = 0L;
						fTime = FALSE;
					}
				}
				lPos = pos;
				while (isBreakInLine(rt->line[oPos])) {
					if (rt->line[oPos] != NL_C && rt->line[oPos] != SNL_C) {
						ced_line[pos] = rt->line[oPos];
						if (rt->att == NULL)
							tempAtt[pos] = 0;
						else
							tempAtt[pos] = rt->att[oPos];
						pos++;
					}
					oPos++;
					copyFontInfo(&newRow.Font, &rt->Font, FALSE);
				}
				while ((!isBreakInLine(rt->line[oPos]) || hc || sq) && rt->line[oPos] != EOS) {
					if (rt->line[oPos] == '[')
						sq = TRUE;
					else if (rt->line[oPos] == ']')
						sq = FALSE;
					if (rt->line[oPos] == HIDEN_C)
						hc = !hc;
					ced_line[pos] = rt->line[oPos];
					if (rt->att == NULL)
						tempAtt[pos] = 0;
					else
						tempAtt[pos] = rt->att[oPos];
					pos++;
					oPos++;
				}
			} while (rt->line[oPos] != EOS) ;
		}
		tr = rt;
		rt = rt->next_row;
		if (isChange) {
			if (tr->line) free(tr->line); 
			if (tr->att) free(tr->att); 
			free(tr);
		}
	}
	if (doReWrap) {
		if (global_df->ChatMode) {
			if (isLineFull(max, pos, &newRow)) {
				ced_line[pos] = EOS;
				ced_line[lPos] = EOS;
				if (!(*finLine)(&newRow, tierType, data)) {
					doReWrap = tDoReWrap;
					return(FALSE);
				}
				if (lPos == pos)
					ced_line[0] = EOS;
				else {
					if (lPos > 0L)
						uAtt_cp(0,ced_line+1,ced_line+lPos+1,tempAtt+1,tempAtt+lPos+1);
					ced_line[0] = '\t';
					tempAtt[0] = 0;
					if (isLineEmpty(ced_line))
						ced_line[0] = EOS;
				}
				pos = strlen(ced_line);
			}
			ced_line[pos] = EOS;
			if (!(*finLine)(&newRow, tierType, data)) {
				doReWrap = tDoReWrap;
				return(FALSE);
			}
		}
	}
	if (isChange) {
		if (global_df->head_text->next_row == global_df->tail_text)
			AddRowAfter(cl_T(""), FALSE, &newRow.Font, NULL);
		global_df->row_win = 0L;
		global_df->col_win = 0L;
		global_df->col_chr = 0L;
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
		global_df->row_txt = global_df->head_text->next_row;
		global_df->cur_line = global_df->row_txt;
		global_df->top_win = global_df->row_txt;
		global_df->lineno = 1L;
		CpCur_lineToHead_row(global_df->cur_line);
		global_df->col_txt = global_df->head_row->next_char;
		global_df->DataChanged = FALSE;
		
		global_df->head_text->flag = 0;
		FALSE_VIS_ID(global_df->head_text->flag);
		global_df->tail_text->flag = 0;
		FALSE_VIS_ID(global_df->tail_text->flag);
	}
	doReWrap = tDoReWrap;
	return(TRUE);
}
// re-wrap routines end

static LINE *FL_Head_rowToCur_line(LINE *temp_col_txt, LINE *old_col_txt, char full_exchange) {
	unCH *s;
	AttTYPE *s1;
	LINE *tl, *fl;

	if (global_df->head_row_len >= (int)strlen(global_df->cur_line->line)) {
		free(global_df->cur_line->line);
		if (global_df->cur_line->att)
			free(global_df->cur_line->att);

		global_df->cur_line->att = NULL;
		if (global_df->head_row->att)
			global_df->cur_line->att = (AttTYPE *)malloc((global_df->head_row_len+1)*sizeof(AttTYPE));
		global_df->cur_line->line = (unCH *)malloc((global_df->head_row_len+1)*sizeof(unCH));
		if (global_df->cur_line->line == NULL) {
//			global_df->cur_line->line = "";
			mem_err(TRUE, global_df);
		}
	}
	s = global_df->cur_line->line;
	s1 = global_df->cur_line->att;
	tl = global_df->head_row->next_char;
	while (tl != global_df->tail_row) {
		*s++ = tl->c;
		if (s1 != NULL)
			*s1++ = tl->att;
		fl = tl;
		tl = tl->next_char;
		if (full_exchange) {
			if (old_col_txt == fl)
				temp_col_txt = fl;
			else
				free(fl);
		}
	}
	*s = EOS;
	return(temp_col_txt);
}

static LINE *FixLine_ChangeCurLineAlways(LINE *temp_col_txt, LINE *old_col_txt) {
	long j;
	char full_exchange;

	j = (long)global_df->DataChanged;
	if (global_df->cur_line != global_df->row_txt)
		full_exchange = TRUE;
	else
		full_exchange = FALSE;
	temp_col_txt = FL_Head_rowToCur_line(temp_col_txt, old_col_txt, full_exchange);

	if (temp_col_txt == global_df->tail_row) {
		LINE *NewChar;

		NewChar = NEW(LINE);
		if (NewChar == NULL) mem_err(TRUE, global_df);
		NewChar->next_char = global_df->tail_row->next_char;
		NewChar->prev_char = global_df->tail_row->prev_char;
		NewChar->prev_char->next_char = NewChar;
		if (global_df->col_txt == global_df->tail_row)
			global_df->col_txt = NewChar;
		global_df->tail_row = NewChar;
	}
	if (full_exchange) {
		global_df->head_row->next_char = global_df->tail_row;
		global_df->tail_row->prev_char = global_df->head_row;
		global_df->head_row_len = 0L;
		global_df->col_txt  = global_df->head_row;
		global_df->cur_line = global_df->row_txt;
		CpCur_lineToHead_row(global_df->cur_line);
	}
	if (!j) 
		global_df->DataChanged = '\0';
	for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0L; j--)
		global_df->col_txt = global_df->col_txt->next_char;
	return(temp_col_txt);
}

static char ContNextRow(void) {
	ROWS *tl = global_df->row_txt->next_row;

	if (global_df->ChatMode) {
		if (tl == global_df->tail_text || isSpeaker(*tl->line))
			return(FALSE);
	} else {
		if (tl == global_df->tail_text || global_df->tail_row->prev_char->c == NL_C) return(FALSE);
	}
	return(TRUE);
}

static int isSQ(LINE *col_txt) {
	int sq = 0;

	for (; col_txt != global_df->head_row; col_txt = col_txt->prev_char) {
		if (col_txt->c == '[')
			sq++;
		else if (col_txt->c == ']' && sq > 0)
			sq--;
	}
	return(sq);
}

static int isHidenC(LINE *col_txt) {
	int HC = 0;

	for (; col_txt != global_df->head_row; col_txt = col_txt->prev_char) {
		if (col_txt->c == HIDEN_C)
			HC++;
		else if (col_txt->c == HIDEN_C && HC > 0)
			HC--;
	}
	return(HC);
}

void FixLine(char UpdateUndo) {
	long curs_fnd = -1L;
	long num;
	long last_col, last_colc;
	unCH *t;
	AttTYPE *tAtt;
	int sq, HC;
	char spf;
	UNDO cursor_state;
	LINE *old_col_txt = global_df->col_txt, *temp_col_txt = NULL, *LC, *ttail_row;
   
	if (!global_df->ChatMode && !global_df->AutoWrap) return;

	if (UpdateUndo) {
		SaveUndoState(FALSE);
		global_df->UndoList->key = MOVERPT;
	}
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	ChangeCurLine();
	do {
		spf = FALSE;
		BeginningOfLine(-1);
		last_col  = global_df->col_win;
		last_colc = global_df->col_chr;
		LC = global_df->col_txt;
		sq = isSQ(global_df->col_txt);
		HC = isHidenC(global_df->col_txt);
		while (global_df->col_txt != global_df->tail_row) {
			if (global_df->col_txt->c == NL_C)
				break;
			if (isSpace(global_df->col_txt->c) && sq == 0 && HC == 0) {
				while (isSpace(global_df->col_txt->c) && global_df->col_txt != global_df->tail_row) {
					if (global_df->col_txt == old_col_txt && !isCallFixLine(0L, global_df->col_chr, TRUE)) {
						curs_fnd = -2L;
						SaveVarUndoState(&cursor_state);
					}
					global_df->col_chr++;
					global_df->col_txt = global_df->col_txt->next_char;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
				if (isCallFixLine(0L, global_df->col_chr, TRUE))
					break;
				last_col  = global_df->col_win;
				last_colc = global_df->col_chr;
				LC = global_df->col_txt;
				spf = TRUE;
			} else {
				if (global_df->col_txt->c == '[')
					sq++;
				else if (global_df->col_txt->c == ']' && sq > 0)
					sq--;
				if (global_df->col_txt->c == HIDEN_C)
					HC++;
				else if (global_df->col_txt->c == HIDEN_C && HC > 0)
					HC--;
				if (global_df->col_txt == old_col_txt && !isCallFixLine(0L, global_df->col_chr, TRUE)) {
					curs_fnd = -2L;
					SaveVarUndoState(&cursor_state);
				}
				global_df->col_chr++;
				global_df->col_txt = global_df->col_txt->next_char;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			}
		}
		if (global_df->col_txt == old_col_txt && !isCallFixLine(0L, global_df->col_chr, TRUE)) {
			curs_fnd = -2L;
			SaveVarUndoState(&cursor_state);
		}
		if (isCallFixLine(0L, global_df->col_chr, TRUE) && spf) {
			if (global_df->ChatMode) {
				if (!doReWrap || !global_df->ChatMode) {
					if (global_df->col_win-last_col+TabSize >= SCREENWIDTH)
						break;
				} else {
					collectLine(templine4, last_colc, global_df->col_chr, FALSE);
					strcat(templine4, "        ");
					if (isCallFixLine(0L, 0L, FALSE))
						break;
				}
			} else if (global_df->AutoWrap) {
				if (!doReWrap || !global_df->ChatMode) {
					if (global_df->col_win-last_col >= SCREENWIDTH)
						break;
				} else {
					collectLine(templine4, last_colc, global_df->col_chr, FALSE);
					if (isCallFixLine(0L, 0L, FALSE))
						break;
				}
			}
			global_df->col_win = last_col;
			global_df->col_chr = last_colc;
			global_df->col_txt = LC;
			num = ComColWin(FALSE, NULL, global_df->head_row_len);
			if (!isCallFixLine(global_df->col_chr, global_df->head_row_len, TRUE) && ContNextRow()) {
				for (num=0, LC=global_df->col_txt; LC != global_df->tail_row; LC=LC->next_char, num++) ;
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = DELTRPT;
					if ((global_df->UndoList->str=(unCH *)malloc((num+1L)*sizeof(unCH))) == NULL)
						mem_err(TRUE, global_df);
					t = global_df->UndoList->str;
				} else {
					if ((t=(unCH *)malloc((num+1L)*sizeof(unCH))) == NULL) 
						mem_err(TRUE, global_df);
				}
				if (global_df->head_row->att) {
					if ((tAtt=(AttTYPE *)malloc((num+1L)*sizeof(AttTYPE))) == NULL)
						mem_err(TRUE, global_df);
				} else
					tAtt = NULL;
				last_col = 0;
				while (global_df->col_txt != global_df->tail_row) {
					if (global_df->col_txt->c == NL_C)
						t[last_col] = ' ';
					else
						t[last_col] = global_df->col_txt->c;
					if (tAtt != NULL)
						tAtt[last_col] = global_df->col_txt->att;
					LC = global_df->col_txt->prev_char;
					LC->next_char = global_df->col_txt->next_char;
					global_df->col_txt->next_char->prev_char = LC;
					if (global_df->col_txt == old_col_txt) {
						temp_col_txt = global_df->col_txt;
						curs_fnd = last_col;
					} else
						free(global_df->col_txt);
					global_df->col_txt = LC->next_char;
					last_col++;
				}
				if (global_df->col_txt == old_col_txt) {
					temp_col_txt = global_df->col_txt;
					curs_fnd = last_col;
				}
				global_df->head_row_len = global_df->head_row_len - last_col;
				t[last_col] = EOS;
		
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
				}
				global_df->redisplay = 0;
				MoveDown(-1);
				global_df->redisplay = 1;
				BeginningOfLine(-1);
				if (tAtt != NULL)
					global_df->head_row->att = 1;
				if (global_df->ChatMode) {
					for(num=0L; isSpace(global_df->row_txt->line[global_df->col_chr]); global_df->col_chr++, num++) ;
					global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				}
				temp_col_txt = FixLine_ChangeCurLineAlways(temp_col_txt, old_col_txt);
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = INSTRPT;
				}
				global_df->col_txt = global_df->col_txt->prev_char;
				if (global_df->ChatMode) {
					if (global_df->col_win < TabSize) {
						global_df->col_chr++;
						AddCharAfter('\t');
						global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					}
				}
				for (last_col=0; t[last_col]; last_col++) {
					if (curs_fnd == last_col && curs_fnd >= 0L) {
						global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
						curs_fnd = -2L;
						SaveVarUndoState(&cursor_state);
					}
					global_df->col_chr++;
					AddCharAfter(t[last_col]);
					if (tAtt != NULL)
						global_df->col_txt->att = tAtt[last_col];
				}
				if (tAtt != NULL)
					free(tAtt);
				if (!UpdateUndo && t != NULL)
					free(t);
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				if (curs_fnd == last_col && curs_fnd >= 0L) {
					curs_fnd = -2L;				
					SaveVarUndoState(&cursor_state);
				}
				global_df->col_txt = global_df->col_txt->next_char;
			} else {
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = INSTRPT;
				}
				temp_col_txt = FixLine_ChangeCurLineAlways(temp_col_txt, old_col_txt);
				for (num=0L, LC=global_df->tail_row; LC != global_df->col_txt; LC=LC->prev_char) num++;
				ttail_row = global_df->tail_row;
				global_df->tail_row = LC;
				global_df->head_row_len -= num;
				temp_col_txt = FL_Head_rowToCur_line(temp_col_txt, old_col_txt, TRUE);
				global_df->head_row_len = num;
				global_df->head_row->next_char = LC;
				LC->prev_char = global_df->head_row;
				global_df->tail_row = ttail_row;
				AddRowAfter(cl_T(""), TRUE, &global_df->row_txt->Font, NULL);
				global_df->cur_line = global_df->row_txt;
				if (++global_df->row_win >= (long)global_df->EdWinSize) {
					global_df->row_win--;
					global_df->top_win = ToNextRow(global_df->top_win, FALSE);
				}
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
				if (global_df->ChatMode) {
					if (global_df->head_row->next_char == global_df->tail_row || !isSpeaker(global_df->head_row->next_char->c)) {
						global_df->col_txt = global_df->head_row;	/* to pad 8 spaces begin */
						AddCharAfter('\t');
						global_df->col_txt = LC;					/* to pad 8 spaces end */
						global_df->col_chr = 1;
						global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					}
				}
				if (UpdateUndo) {
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
				}
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
				for (LC=global_df->head_row->next_char; LC != global_df->tail_row; LC=LC->next_char) {
					if (LC == old_col_txt) {
						global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
						curs_fnd = -2L;
						SaveVarUndoState(&cursor_state);
						break;
					}
					global_df->col_chr++;
				}
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			}
//			TRUE_CHECK_ID1(global_df->row_txt->flag);
//			TRUE_CHECK_ID2(global_df->row_txt->flag);
			temp_col_txt = FixLine_ChangeCurLineAlways(temp_col_txt, old_col_txt);
			if (UpdateUndo) {
				SaveUndoState(FALSE);
				global_df->UndoList->key = MOVERPT;
			}
		} else
			break;
	} while (1) ;
	if (curs_fnd != -1L)
		ResetVarStatus(&cursor_state);
	if (temp_col_txt != NULL)
		free(temp_col_txt);
}

static char DisplayOneRow(int  row, ROWS *win) {
	register long lineChr;
	register long colChr;
	register long colWin;
	long newcol;
	char NeedClearing;
	NewFontInfo finfo;
	LINE *ch;

	NeedClearing = TRUE;
	finfo.Encod = my_FontToScript(win->Font.FName, win->Font.CharSet);
	finfo.isUTF = global_df->isUTF;
	copyFontInfo(global_df->w1->RowFInfo[row], &win->Font, FALSE);
	wmove(global_df->w1,row,0);
	colChr = 0L;
	colWin = 0L;
	if (win == global_df->cur_line) {
		for (ch=global_df->head_row->next_char; ch!=global_df->tail_row; ch=ch->next_char) {
			if (colChr >= global_df->w1->num_cols) {
				if (row >= global_df->EdWinSize-1)
					NeedClearing = FALSE;
				break;
			} else if (ch->c == NL_C && global_df->ShowParags != '\001') {
				continue;
/*12-5-02
			} else if (colWin == global_df->LeftCol+global_df->num_of_cols-1 &&
		 				ch->next_char != global_df->tail_row) {
				waddch(global_df->w1,(char)CONT_CHAR, NULL, 0L, ch->att);
				colChr++;
				colWin++;
*/
			} else if (colWin >= global_df->LeftCol) {
				if (ch->c == '\t') {
					newcol = (((colWin / TabSize) + 1) * TabSize);
					for (; colWin < newcol && colChr < global_df->w1->num_cols; colWin++, colChr++)
						waddch(global_df->w1,' ', NULL, 0L, ch->att);
				} else {
					colWin++;
					colChr++;
					if (ch->c == NL_C)
						waddch(global_df->w1,SOFT_CR_CHAR, NULL, 0L, ch->att);
					else if (ch->c == HIDEN_C) {
						if (global_df->isUTF) {
							waddch(global_df->w1,(unCH)0x2022, NULL, 0L, ch->att);
						} else
							waddch(global_df->w1,BULLET_CHAR, NULL, 0L, ch->att);
						if (global_df->ShowParags != '\002') {
							for (ch=ch->next_char; ch->c != HIDEN_C && ch!=global_df->tail_row; ch=ch->next_char) ;
							if (ch == global_df->tail_row)
								ch = ch->prev_char;
						}
					} else
						waddch(global_df->w1,ch->c, NULL, 0L, ch->att);
				}
			} else if (ch->c == '\t') {
				newcol = (((colWin / TabSize) + 1) * TabSize);
				if (colWin < global_df->LeftCol && newcol > global_df->LeftCol) {
					colWin = global_df->LeftCol;
					for (; colWin < newcol && colChr < global_df->w1->num_cols; colWin++, colChr++) 
						waddch(global_df->w1,' ', NULL, 0L, ch->att);
				} else
					colWin = newcol;
			} else if (ch->c == HIDEN_C) {
				colWin++;
				if (global_df->ShowParags != '\002') {
					for (ch=ch->next_char; ch->c != HIDEN_C && ch!=global_df->tail_row; ch=ch->next_char) ;
					if (ch == global_df->tail_row)
						ch = ch->prev_char;
				}
			} else {
				colWin++;
			}
		}
	} else {
		lineChr = 0L;
		while (win->line[lineChr]) {
			if (colChr >= global_df->w1->num_cols) {
				if (row >= global_df->EdWinSize-1)
					NeedClearing = FALSE;
				break;
			} else if (win->line[lineChr] == NL_C && global_df->ShowParags != '\001') {
				lineChr++;
				continue;
/*12-5-02
			} else if (colWin == global_df->LeftCol+global_df->num_of_cols-1 &&
				win->line[lineChr+1] != EOS) {
				waddch(global_df->w1,(char)CONT_CHAR, win->att, lineChr, 0);
				colChr++;
				colWin++;
*/
			} else if (colWin >= global_df->LeftCol) {
				if (win->line[lineChr] == '\t') {
					newcol = (((colWin / TabSize) + 1) * TabSize);
					for (; colWin < newcol && colChr < global_df->w1->num_cols; colWin++, colChr++) 
						waddch(global_df->w1,' ', win->att, lineChr, 0);
				} else {
					colWin++;
					colChr++;
					if (win->line[lineChr] == NL_C)
						waddch(global_df->w1,SOFT_CR_CHAR, win->att, lineChr, 0);
					else if (win->line[lineChr] == HIDEN_C) {
						if (global_df->isUTF) {
							waddch(global_df->w1,(unCH)0x2022, win->att, lineChr, 0);
						} else
							waddch(global_df->w1,BULLET_CHAR, win->att, lineChr, 0);
						if (global_df->ShowParags != '\002') {
							for (lineChr++; win->line[lineChr] != HIDEN_C && win->line[lineChr]; lineChr++) ;
							if (win->line[lineChr] == EOS)
								lineChr--;
						}
					} else
						waddch(global_df->w1,win->line[lineChr], win->att, lineChr, 0);
				}
			} else if (win->line[lineChr] == '\t') {
				newcol = (((colWin / TabSize) + 1) * TabSize);
				if (colWin < global_df->LeftCol && newcol > global_df->LeftCol) {
					colWin = global_df->LeftCol;
					for (; colWin < newcol && colChr < global_df->w1->num_cols; colWin++, colChr++) 
						waddch(global_df->w1,' ', win->att, lineChr, 0);
				} else
					colWin = newcol;
			} else if (win->line[lineChr] == HIDEN_C) {
				colWin++;
				if (global_df->ShowParags != '\002') {
					for (lineChr++; win->line[lineChr] != HIDEN_C && win->line[lineChr]; lineChr++) ;
					if (win->line[lineChr] == EOS)
						lineChr--;
				}
			} else {
				colWin++;
			}
			lineChr++;
		}
	}
	if (NeedClearing)
		wclrtoeol(global_df->w1, false);
	return(NeedClearing);
}

void DisplayTextWindow(ROWS *row_line, char refresh) {
	int row;
	int tTextWinSize = global_df->TextWinSize;
	unsigned long lineno = 0L;
	ROWS *win;
	char NeedClearing;

	if (!global_df->isRedrawTextWindow)
		return;
	if (global_df->w1 == NULL)
		return;
	global_df->w1->isUTF = global_df->isUTF;
/*	if (!global_df->ChatMode) */
	if (row_line != NULL && refresh == 1 && CheckLeftCol(global_df->col_win))
		row_line = NULL;

	if (global_df->RootColorText != NULL)
		row_line = NULL;

	if (row_line == NULL) {
		NeedClearing = TRUE;
		if (isShowLineNums) {
			ChangeCurLineAlways(0);
			lineno = 1L;
			for (win=global_df->head_text->next_row;
						win != global_df->tail_text && win != global_df->top_win;
										win=win->next_row) {
				if (LineNumberingType == 0 || isMainSpeaker(win->line[0]))
					lineno++;
			}
		}
		win = global_df->top_win;
		global_df->EdWinSize = 1;
		for (row=0; row < global_df->EdWinSize && win!=global_df->tail_text; win=win->next_row) {
			if (CMP_VIS_ID(win->flag) || !global_df->ChatMode) {
				tTextWinSize -= win->Font.FHeight;
				if (tTextWinSize < 0 || row >= global_df->w1->num_rows)
					break;
				global_df->EdWinSize++;
				NeedClearing = DisplayOneRow(row, win);
				if (isShowLineNums) {
					if (LineNumberingType == 0 || isMainSpeaker(win->line[0]))
						wsetlineno(global_df->w1, row, lineno);
					else
						wsetlineno(global_df->w1, row, 0L);
				}
				row++;
			}
			if (isShowLineNums) {
				if (LineNumberingType == 0 || isMainSpeaker(win->line[0]))
					lineno++;
			}
		}
		global_df->EdWinSize--;
		if (NeedClearing)
			wclrtobot(global_df->w1);
	} else {
		if (global_df->w1->cur_row == -1) 
			PutCursorInWindow(global_df->w1);
		NeedClearing = DisplayOneRow(global_df->row_win, row_line);
	}
	if (refresh > 0) {
//		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
	}
}

void RecompEdWinSize(void) {
	register int tTextWinSize = global_df->TextWinSize;
	ROWS *line = global_df->top_win;

	if (global_df->w1 == NULL)
		return;
	if (line == global_df->head_text)
		line = ToNextRow(line,FALSE);
	global_df->EdWinSize = 1;
	do {
		tTextWinSize -= line->Font.FHeight;
		if (tTextWinSize < 0 || global_df->EdWinSize > global_df->w1->num_rows)
			break;
		global_df->EdWinSize++;
		line = ToNextRow(line,FALSE);
	} while (line != global_df->tail_text) ;
	global_df->EdWinSize--;
}

/* tier names location begin */
static void DeleteCodeIfEmpty() {
	char found = FALSE;
	int i = 0, cnt = 0;
	LINE *l = global_df->head_row->next_char;

	if (global_df->row_txt == global_df->cur_line) {
		if (l->c == '%') {
			cnt++;
			for (; l != global_df->tail_row && l->c != ':'; l=l->next_char) cnt++;
			if (l != global_df->tail_row) {
				for (l=l->next_char; l != global_df->tail_row; l=l->next_char) {
					cnt++;
					if (!isSpace(l->c)) {
						found = TRUE;
						break;
					}
				}
			} else cnt--;
		} else found = TRUE;
	} else {
		if (global_df->row_txt->line[i] == '%') {
			cnt++;
			for (; global_df->row_txt->line[i] && global_df->row_txt->line[i] != ':'; i++) cnt++;
			if (global_df->row_txt->line[i]) {
				for (i++; global_df->row_txt->line[i]; i++) {
					cnt++;
					if (!isSpace(global_df->row_txt->line[i])) {
						found = TRUE;
						break;
					}
				}
			} else cnt--;
		} else found = TRUE;
	}
	if (!found) {
		global_df->redisplay = 0;
		EndOfLine(-1);
		if (global_df->cur_line->next_row != global_df->tail_text) {
			sp[cnt] = SNL_C;
			sp[cnt+1] = EOS;

		} else
			sp[cnt] = EOS;
		for (; cnt > 0; cnt--) {
			sp[cnt-1] = global_df->col_txt->prev_char->c;
			DeletePrevChar(-1);
		}
		global_df->redisplay = 1;
		DeleteNextChar(-1);
		global_df->UndoList->key = DELTKEY;
		if ((global_df->UndoList->str = (unCH *)malloc((strlen(sp)+1)*sizeof(unCH))) == NULL)
			mem_err(TRUE, global_df);
		strcpy(global_df->UndoList->str, sp);
		SaveUndoState(FALSE);
	}
}

int EndCurTierGotoNext(int i) {
	LINE *tc;
	unCH *l, *p;
	char found = FALSE;

	if (global_df->EditorMode && i != -1) {
		RemoveLastUndo();
		strcpy(global_df->err_message, "-This command works in coder mode only.");
		return(8);
	}
	if (global_df->CurCode != global_df->RootCodes) {
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2;
		global_df->col_chr2 = -2L;
		ToTopLevel(-1);
		DeleteCodeIfEmpty();
		global_df->EnteredCode = '\0';
	}
	if (global_df->EnteredCode != '\001') {
		global_df->redisplay = 0;
		while (1) {
			if (global_df->row_txt == global_df->cur_line) {
				if (global_df->head_row->next_char->c == '*') { 
					if (NextTierName[0] == EOS) {
						found = TRUE;
						break;
					}
					tc = global_df->head_row->next_char->next_char; 
					p = NextTierName+1;
					for (; tc != global_df->tail_row && *p!= EOS; tc=tc->next_char, p++) {
						if (tc->c >= 'a' && tc->c <= 'z') {
							if (to_unCH_upper(tc->c) != *p)
								break;
						} else if (tc->c != *p) break;
					}
					if (*p == ':') {
						if (tc == global_df->tail_row) {
							found = TRUE;
							break;
						}
					} else if (*p == EOS) {
						found = TRUE;
						break;
					}
				}
			} else {
				if (*global_df->row_txt->line == '*') { 
					if (NextTierName[0] == EOS) {
						found = TRUE;
						break;
					}
					l = global_df->row_txt->line+1;
					p = NextTierName+1;
					for (; *p != EOS; l++, p++) {
						if (*l >= 'a' && *l <= 'z') {
							if (to_unCH_upper(*l) != *p)
								break;
						} else if (*l != *p)
							break;
					}
					if (*p == ':') {
						if (*l == EOS) {
							found = TRUE;
							break;
						}
					} else if (*p == EOS) {
						found = TRUE;
						break;
					}
				}
			}
			if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
				break;
			MoveDown(i);
		}
		global_df->redisplay = 1;
		DisplayTextWindow(NULL, 1);
	} else {
		found = TRUE;
		global_df->EnteredCode = '\0';
	}
	if (found)
		GetCursorCode(i);
	return(8);
}

int GoToNextMainSpeaker(int d) {
	RemoveLastUndo();
	if (!global_df->ChatMode) {
		strcpy(global_df->err_message, "+Not available in TEXT mode.");
		return(37);
	}
	global_df->redisplay = 0;
	while (1) {
		MoveDown(1);
		EndOfLine(-1);
		if (global_df->row_txt == global_df->cur_line) {
			if (isMainSpeaker(global_df->head_row->next_char->c)) { 
				break;
			}
		} else {
			if (isMainSpeaker(*global_df->row_txt->line)) { 
				break;
			}
		}
		if (FoundEndHTier(global_df->row_txt, TRUE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
			EndOfLine(-1);
			SaveUndoState(FALSE);
			AddCodeTier(cl_T("*"), FALSE);
			strcpy(global_df->err_message, "-Press a number between 0 and 9");
			break;
		}
	}
	global_df->redisplay = 1;
	DisplayTextWindow(NULL, 1);
	return(37);
}

char FoundEndHTier(ROWS *tr, char isMove) {
	if (tr == global_df->cur_line) {
		LINE *tl= global_df->head_row->next_char;

		if (tl->c == '@' && tl != global_df->tail_row) {
			tl = tl->next_char;
			if (tl->c == 'E' && tl != global_df->tail_row) {
				tl = tl->next_char;
				if (tl->c == 'n' && tl != global_df->tail_row) {
					tl = tl->next_char;
					if (tl->c == 'd' && tl != global_df->tail_row) {
						tl = tl->next_char;
						if (!is_unCH_alnum(tl->c) || tl == global_df->tail_row) {
							if (isMove && tr == global_df->row_txt)
								MoveUp(-1);
							return(TRUE);
						}
					}
				}
			}
		}
	} else {
		unCH *s = tr->line;

		if (*s == '@' && *(s+1) == 'E' && *(s+2) == 'n' && *(s+3) == 'd' && !is_unCH_alnum(*(s+4))) { 
			if (isMove && tr == global_df->row_txt)
				MoveUp(-1);
			return(TRUE);
		}
	}
	return(FALSE);
}

void GetCurCode(void) {
	register int i;
	ROWS *tr = global_df->row_txt;

	*sp = EOS;
	if (global_df->row_txt == global_df->tail_text)
		return;
	while (1) {
		if (tr == global_df->cur_line) {
			LINE *tl=global_df->head_row->next_char;

			if (tl != global_df->tail_row && isSpeaker(tl->c)) {
				for (i=0; tl!= global_df->tail_row && tl->c!= ':'; tl=tl->next_char)
					sp[i++] = tl->c;
				if (tl != global_df->tail_row)
					sp[i++] = ':';
				sp[i] = EOS;
				break;
			}
		} else {
			if (isSpeaker(tr->line[0])) {
				for (i=0; tr->line[i] != ':' && tr->line[i]; i++)
					sp[i] = tr->line[i];
				sp[i] = tr->line[i];
				i++;
				sp[i] = EOS;
				break;
			}
		}
		tr = tr->prev_row;
		if (tr == global_df->head_text)
			return;
	}
}

#define FoundTextCode1 												\
	global_df->row_txt = tr;										\
	global_df->lineno -= (long)offset;								\
	global_df->col_win = 0L;										\
	global_df->col_chr = 0L;										\
	global_df->row_win -= (long)offset;								\
	if (global_df->row_win < 0L) {									\
		global_df->row_win = 0L;									\
		while (!AtTopEnd(tr, global_df->head_text, FALSE)) {		\
			if (tr == global_df->cur_line) {						\
				if (global_df->head_row->next_char->c == '*')		\
					break;											\
			} else if (*tr->line == '*')							\
				break;												\
			tr = ToPrevRow(tr, FALSE);								\
			global_df->row_win++;									\
		}															\
		global_df->top_win = tr;									\
		DisplayTextWindow(NULL, 1);									\
	}

#define FoundTextCode2 												\
	global_df->row_txt = tr;										\
	global_df->lineno += (long)offset;								\
	global_df->col_win = 0L;										\
	global_df->col_chr = 0L;										\
	global_df->row_win += (long)offset;								\
	if (global_df->row_win >= (long)global_df->EdWinSize) {			\
		global_df->row_win = 0L;									\
		while (!AtTopEnd(tr, global_df->head_text, FALSE)) {		\
			if (tr == global_df->cur_line) {						\
				if (global_df->head_row->next_char->c == '*')   	\
					break;											\
			} else if (*tr->line == '*')							\
				break;												\
			tr = ToPrevRow(tr, FALSE);								\
			global_df->row_win++;									\
		}															\
		global_df->top_win = tr;									\
		DisplayTextWindow(NULL, 1);									\
	}

static char isHidenCodeFound(unCH c) {
	if (c == '%' || is_unCH_digit(c))
		return(TRUE);
	else
		return(FALSE);
}

int GetCurHidenCode(char move, unCH *CodeMatch) {
	register int offset;
	int d;
	long i = 0L, c, ti = 0L, index;
	char hc_found, isCodFound;
	ROWS *tr;
	LINE *tl, *ttl;

	*sp = EOS;
	offset = 0;
	tr = global_df->row_txt;
	if (global_df->row_txt == global_df->cur_line) {
		tl = global_df->col_txt;
		if (tl->prev_char->c == HIDEN_C && 
			isHidenCodeFound(tl->c) &&
			tl->prev_char != global_df->head_row) {
			ttl = tl->prev_char;
		} else
		if (tl->c == HIDEN_C && 
			isHidenCodeFound(tl->next_char->c) &&
			tl->next_char != global_df->tail_row) {
			ttl = tl;
			tl = tl->next_char;
		} else
		if (tl->c == HIDEN_C && 
			(is_unCH_digit(tl->prev_char->c) || tl->prev_char->c == '-' || tl->prev_char->c == '"') &&
			tl->prev_char != global_df->head_row) {
			for (tl=tl->prev_char; tl->c != HIDEN_C && tl != global_df->head_row->next_char; tl=tl->prev_char) ;
			if (tl->c == HIDEN_C && isHidenCodeFound(tl->next_char->c)) {
				ttl = tl;
				tl = tl->next_char;
			} else
				return(FALSE);
		} else
		if (tl->prev_char->c == HIDEN_C && 
			(is_unCH_digit(tl->prev_char->prev_char->c) || tl->prev_char->prev_char->c == '-' || 
															tl->prev_char->prev_char->c == '"') &&
			tl->prev_char != global_df->head_row && tl->prev_char->prev_char != global_df->head_row) {
			for (tl=tl->prev_char->prev_char; tl->c != HIDEN_C && tl != global_df->head_row->next_char; tl=tl->prev_char) ;
			if (tl->c == HIDEN_C && isHidenCodeFound(tl->next_char->c)) {
				ttl = tl;
				tl = tl->next_char;
			} else
				return(FALSE);
		} else {
			isCodFound = 0;
			hc_found = FALSE;
			c = 0;
			ttl = global_df->head_row->next_char;
			for (tl=global_df->head_row->next_char; tl != global_df->tail_row; tl=tl->next_char) {
				if (hc_found && CodeMatch != NULL) {
					if (isCodFound == 0 && is_unCH_digit(tl->c)) {
						isCodFound = 2;
						strcpy(sp, SOUNDTIER);
						if (uS.partwcmp(sp, CodeMatch)) {
							tl = ttl->next_char;
							break;
						}
						c = 0;
					} else if (isCodFound == 0 || isCodFound == 1) {
						isCodFound = 1;
						if (c >= SPEAKERLEN)
							isCodFound = 2;
						else
							sp[c++] = tl->c;
						if (tl->c == ':') {
							sp[c] = EOS;
							if (uS.partwcmp(sp, CodeMatch)) {
								tl = ttl->next_char;
								break;
							}
							if (!strcmp(CodeMatch, SOUNDTIER) && uS.partwcmp(sp, REMOVEMOVIETAG)) {
								tl = ttl->next_char;
								break;
							}
							c = 0;
						}
					}
				}
				if (tl->c == HIDEN_C) {
					hc_found = !hc_found;
					isCodFound = 0;
					ttl = tl;
				}
				if (tl == global_df->col_txt && hc_found) {
					tl = ttl->next_char;
					break;
				}
			}
			if (tl == global_df->tail_row)
				return(FALSE);
		}

		isCodFound = 0;
		if (tl->prev_char != global_df->head_row && tl->prev_char->c == HIDEN_C && is_unCH_digit(tl->c)) {
			strcpy(sp, SOUNDTIER);
			isCodFound = 1;
		} else {
			for (c=0; tl->c != ':' && tl != global_df->tail_row; c++, tl=tl->next_char) {
				if (c >= SPEAKERLEN)
					break;
				sp[c] = tl->c;
			}
			if (tl->c == ':') {
				sp[c++] = ':';
				sp[c] = EOS;
				isCodFound = 1;
			}
		}
		if (isCodFound == 1) {
			if (move) {
				FoundTextCode1;
				global_df->col_txt = ttl;
				global_df->col_chr = 0L;
				for (tl=global_df->head_row->next_char; tl != global_df->col_txt; global_df->col_chr++,tl=tl->next_char) ;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				d = (int)global_df->DataChanged;
				ChangeCurLine();
				if (!d) 
					global_df->DataChanged = '\0';
			}
			return(TRUE);
		} else {
			c = 0;
			sp[c] = EOS;
		}
	} else {
		if (global_df->row_txt->line[global_df->col_chr-1] == HIDEN_C && 
			isHidenCodeFound(global_df->row_txt->line[global_df->col_chr])) {
			ti = global_df->col_chr - 1;
			i = global_df->col_chr;
		} else
		if (global_df->row_txt->line[global_df->col_chr]   == HIDEN_C && 
			isHidenCodeFound(global_df->row_txt->line[global_df->col_chr+1])) {
			ti = global_df->col_chr;
			i = global_df->col_chr + 1;
		} else
		if (global_df->row_txt->line[global_df->col_chr] == HIDEN_C && 
					(is_unCH_digit(global_df->row_txt->line[global_df->col_chr-1]) ||
					 global_df->row_txt->line[global_df->col_chr-1] == '-'   || 
					 global_df->row_txt->line[global_df->col_chr-1] == '"')) {
			for (i=global_df->col_chr-1; global_df->row_txt->line[i] != HIDEN_C && i > 0; i--) ;
			if (global_df->row_txt->line[i] == HIDEN_C && isHidenCodeFound(global_df->row_txt->line[i+1])) {
				ti = i;
				i = i + 1;
			} else
				return(FALSE);
		} else
		if (global_df->row_txt->line[global_df->col_chr-1] == HIDEN_C && 
					(is_unCH_digit(global_df->row_txt->line[global_df->col_chr-2]) ||
					 global_df->row_txt->line[global_df->col_chr-2] == '-'   ||
					 global_df->row_txt->line[global_df->col_chr-2] == '"')) {
			for (i=global_df->col_chr-2; global_df->row_txt->line[i] != HIDEN_C && i > 0; i--) ;
			if (global_df->row_txt->line[i] == HIDEN_C && isHidenCodeFound(global_df->row_txt->line[i+1])) {
				ti = i;
				i = i + 1;
			} else
				return(FALSE);
		} else {
			isCodFound = 0;
			hc_found = FALSE;
			c = 0;
			for (index=0; global_df->row_txt->line[index]; index++) {
				if (hc_found && CodeMatch != NULL) {
					if (isCodFound == 0 && is_unCH_digit(global_df->row_txt->line[index])) {
						isCodFound = 2;
						strcpy(sp, SOUNDTIER);
						if (uS.partwcmp(sp, CodeMatch)) {
							i = ti + 1;
							break;
						}
						c = 0;
					} else if (isCodFound == 0 || isCodFound == 1) {
						isCodFound = 1;
						if (c >= SPEAKERLEN)
							isCodFound = 2;
						else
							sp[c++] = global_df->row_txt->line[index];
						if (global_df->row_txt->line[index] == ':') {
							sp[c] = EOS;
							if (uS.partwcmp(sp, CodeMatch)) {
								i = ti + 1;
								break;
							}
							if (!strcmp(CodeMatch, SOUNDTIER) && uS.partwcmp(sp, REMOVEMOVIETAG)) {
								i = ti + 1;
								break;
							}
							c = 0;
						}
					}
				}
				if (global_df->row_txt->line[index] == HIDEN_C) {
					hc_found = !hc_found;
					isCodFound = 0;
					ti = index;
				}
				if (index == global_df->col_chr && hc_found) {
					i = ti + 1;
					break;
				}
			}
			if (global_df->row_txt->line[index] == EOS)
				return(FALSE);
		}

		isCodFound = 0;
		if (i > 0 && global_df->row_txt->line[i-1] == HIDEN_C && is_unCH_digit(global_df->row_txt->line[i])) {
			strcpy(sp, SOUNDTIER);
			isCodFound = 1;
		} else {
			for (c=0; global_df->row_txt->line[i] != ':' && global_df->row_txt->line[i]; c++, i++) {
				if (c >= SPEAKERLEN)
					break;
				sp[c] = global_df->row_txt->line[i];
			}
			if (global_df->row_txt->line[i] == ':') {
				sp[c++] = ':';
				sp[c] = EOS;
				isCodFound = 1;
			}
		}
		if (isCodFound == 1) {
			if (move) {
				FoundTextCode2;
				global_df->col_chr = ti;
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				d = (int)global_df->DataChanged;
				ChangeCurLine();
				if (!d) 
					global_df->DataChanged = '\0';
			}
			return(TRUE);
		} else {
			c = 0;
			sp[c] = EOS;
		}
	}
	return(FALSE);
}

void FindAnyBulletsOnLine(void) {
	register int offset;
	long index, ti;
	char hc_found;
	ROWS *tr;
	LINE *tl, *ttl;

	offset = 0;
	tr = global_df->row_txt;
	if (global_df->row_txt == global_df->cur_line) {
		hc_found = FALSE;
		ttl = NULL;
		tl = global_df->col_txt;
		if (tl->prev_char != global_df->head_row && tl->prev_char->c == HIDEN_C) {
			tl = tl->prev_char;
			tl = tl->prev_char;
			if (tl != global_df->head_row && iswdigit(tl->c)) {
				for (; tl != global_df->head_row; tl=tl->prev_char) {
					if (tl->c == HIDEN_C)
						break;
				}
				if (tl == global_df->head_row)
					tl = global_df->col_txt;
			} else
				tl = global_df->col_txt;
		}
		for (; tl != global_df->tail_row; tl=tl->next_char) {
			if (tl->c == HIDEN_C) {
				hc_found = !hc_found;
				if (hc_found && ttl != NULL)
					return;
				ttl = tl;
				if (!hc_found && ttl != NULL)
					break;
			}
		}
		if (ttl != NULL) {
			FoundTextCode1;
			global_df->col_txt = ttl;
			global_df->col_chr = 0L;
			for (tl=global_df->head_row->next_char; tl != global_df->col_txt; global_df->col_chr++,tl=tl->next_char) ;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			ChangeCurLine();
			SaveUndoState(FALSE);
		}
	} else {
		hc_found = FALSE;
		ti = -1;
		index = global_df->col_chr;
		if (index > 0 && global_df->row_txt->line[index-1] == HIDEN_C) {
			index -= 2;
			if (index >= 0 && iswdigit(global_df->row_txt->line[index])) {
				for (; index >= 0; index--) {
					if (global_df->row_txt->line[index] == HIDEN_C)
						break;
				}
				if (index < 0)
					index = global_df->col_chr;
			} else
				index = global_df->col_chr;
		}
		for (; global_df->row_txt->line[index]; index++) {
			if (global_df->row_txt->line[index] == HIDEN_C) {
				hc_found = !hc_found;
				if (hc_found && ti != -1)
					return;
				ti = index;
				if (!hc_found && ti != -1)
					break;
			}
		}
		if (ti != -1) {
			FoundTextCode2;
			global_df->col_chr = ti;
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			ChangeCurLine();
			SaveUndoState(FALSE);
		}
	}
}

int FindTextCodeLine(unCH *code, unCH *CodeMatch) {
	register int j;
	register int c;
	register int offset;
	ROWS *tr;
	LINE *tl, *ttl;

	if (GetCurHidenCode(TRUE, CodeMatch)) {
		if (uS.partwcmp(sp, code))
			return(TRUE);
		if (!strcmp(code, SOUNDTIER) && uS.partwcmp(sp, REMOVEMOVIETAG))
			return(TRUE);
	}
	*sp = EOS;
	offset = 0;
	tr = global_df->row_txt;
	while (1) { /* search backward of current line */
		if (tr == global_df->cur_line) {
			tl = global_df->head_row->next_char;
			if (tl->c == '%') {
				for (c=0, ttl=tl; ttl->c == code[c] && ttl != global_df->tail_row;
										c++, ttl = ttl->next_char) ;
				if (code[c] == EOS) {
					global_df->col_txt = global_df->head_row->next_char;
					FoundTextCode1;
					strcpy(sp, code);
					return(TRUE);
				}
			} else if (isMainSpeaker(tl->c))
				break;
		} else {
			if (*tr->line == '%') {
				for (c=0, j=0; tr->line[j] == code[c] && tr->line[j]; c++, j++) ;
				if (code[c] == EOS) {
					FoundTextCode1;
					strcpy(sp, code);
					return(TRUE);
				}
			} else if (isMainSpeaker(*tr->line))
				break;
		}
		offset++;
		tr = ToPrevRow(tr, FALSE);
		if (tr == global_df->head_text)
			return(FALSE);
	}
	offset = 0;
	tr = global_df->row_txt;
	if (tr == global_df->cur_line) {
		LINE *tl=global_df->head_row->next_char;
		if (isMainSpeaker(tl->c)) {
			tr = ToNextRow(tr, FALSE);
			offset = 1;
		}
	} else if (isMainSpeaker(*tr->line)) {
		tr = ToNextRow(tr, FALSE);
		offset = 1;
	}
	if (tr == global_df->tail_text)
		return(FALSE);
	while (1) { /* search forward of current line */
		if (tr == global_df->cur_line) {
			tl = global_df->head_row->next_char;
			if (tl->c == '%') {
				for (c=0, ttl=tl; ttl->c == code[c] && ttl != global_df->tail_row;
										c++, ttl = ttl->next_char) ;
				if (code[c] == EOS) {
					global_df->col_txt = global_df->head_row->next_char;
					FoundTextCode2;
					strcpy(sp, code);
					return(TRUE);
				}
			} else if (isMainSpeaker(tl->c))
				break;
		} else {
			if (*tr->line == '%') {
				for (c=0, j=0; tr->line[j] == code[c] && tr->line[j]; c++, j++) ;
				if (code[c] == EOS) {
					FoundTextCode2;
					strcpy(sp, code);
					return(TRUE);
				}
			} else if (isMainSpeaker(*tr->line))
				break;
		}
		offset++;
		tr = ToNextRow(tr, FALSE);
		if (tr == global_df->tail_text)
			return(FALSE);
	}
	return(FALSE);
}
/* tier names location end */

static char changeStyleOfCurrentLine(char which, char color, char setAtt, ROWS *st, long sc, long ec) {
	char isOKtoChange, isDoAll, sb, isBullet;
	long cnt, t;

	cnt = 0;
	// (which == 99) - 2019-04-17 ESC-C - color Red
	isDoAll = (which == 0) || (which == 99) || (!global_df->ChatMode);
	sb = FALSE;
	isBullet = FALSE;
	if (!isDoAll) {
		isOKtoChange = FALSE;
		if (isSpeaker(st->line[cnt])) {
			for (; st->line[cnt] != ':' && st->line[cnt] != EOS; cnt++) ;
			if (st->line[cnt] == ':')
				cnt++;
		}
		do {
			if (st->line[cnt] == '[')
				sb = TRUE;
			else if (st->line[cnt] == ']')
				sb = FALSE;
			else if (st->line[cnt] == HIDEN_C)
				isBullet = !isBullet;
			if (isSpace(st->line[cnt])) {
				for (; cnt < ec && isSpace(st->line[cnt]); cnt++) ;
			} else if (st->line[cnt] == '[' && st->line[cnt+1] == '-') {
				for (; cnt < ec && st->line[cnt] != ']' && st->line[cnt] != EOS; cnt++) ;
				if (st->line[cnt] == ']')
					cnt++;
			} else if (st->line[cnt] == '+') {
				for (; cnt < ec && !isSpace(st->line[cnt]) && st->line[cnt] != '[' && st->line[cnt] != EOS; cnt++) ;
			} else
				isOKtoChange = TRUE;
		} while (!isOKtoChange && st->line[cnt] != EOS && cnt < ec) ;
		if (st->line[cnt] == EOS)
			return(FALSE);
	} else
		isOKtoChange = TRUE;
	if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
		return(FALSE);
	for (; cnt < sc; cnt++) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		else if (st->line[cnt] == HIDEN_C)
			isBullet = !isBullet;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
			return(FALSE);
	}
	if (which == 4 && isOKtoChange && !sb) {
		for (t=cnt-1; t >= 0 && !uS.isskip(st->line,t,&dFnt,TRUE) && st->line[t] != ']'; t--) {
			st->att[t] = set_color_num(color, st->att[t]);
		}
	}
	for (; cnt < ec; cnt++) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		else if (st->line[cnt] == HIDEN_C)
			isBullet = !isBullet;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll) {
			ec = cnt;
			for (cnt--; cnt > 0; cnt--) {
				if (st->line[cnt] == '+' || (uS.isskip(st->line,cnt,&dFnt,TRUE) && !uS.IsUtteranceDel(st->line, cnt)) || is_unCH_alnum(st->line[cnt]))
					break;
			}
			if (st->line[cnt] == '+') {
				t = cnt;
				for (; cnt < ec; cnt++) {
					st->att[cnt] = set_underline_to_0(st->att[cnt]);
					st->att[cnt] = set_italic_to_0(st->att[cnt]);
					st->att[cnt] = set_bold_to_0(st->att[cnt]);
				}
				ec = t;
			}
			break;
		}

		if (!isOKtoChange) ;
		else if (which == 0) {
			st->att[cnt] = set_underline_to_0(st->att[cnt]);
			st->att[cnt] = set_italic_to_0(st->att[cnt]);
			st->att[cnt] = set_bold_to_0(st->att[cnt]);
#ifdef _CLAN_DEBUG // 2019-04-17 ESC-C - color Red
			st->att[cnt] = set_error_to_0(st->att[cnt]);
#endif
		} else if ((!isBullet && !sb) || !global_df->ChatMode) {
			if (which == 1) {
				if (setAtt)
					st->att[cnt] = set_underline_to_1(st->att[cnt]);
				else
					st->att[cnt] = set_underline_to_0(st->att[cnt]);
			} else if (which == 2) {
				if (setAtt)
					st->att[cnt] = set_italic_to_1(st->att[cnt]);
				else
					st->att[cnt] = set_italic_to_0(st->att[cnt]);
			} else if (which == 3) {
				if (setAtt)
					st->att[cnt] = set_bold_to_1(st->att[cnt]);
				else
					st->att[cnt] = set_bold_to_0(st->att[cnt]);
			} else if (which == 4) {
				if (!uS.isskip(st->line,cnt,&dFnt,TRUE) && !sb)
					st->att[cnt] = set_color_num(color, st->att[cnt]);
			} else if (which == 99) {  // 2019-04-17 ESC-C - color Red
				if (setAtt)
					st->att[cnt] = set_error_to_1(st->att[cnt]);
				else
					st->att[cnt] = set_error_to_0(st->att[cnt]);
			}
		}
	}
	if (isOKtoChange) {
		if (which == 4 && !uS.isskip(st->line,ec-1,&dFnt,TRUE) && !sb) {
			for (cnt=ec; st->line[cnt] && !uS.isskip(st->line,cnt,&dFnt,TRUE) && st->line[cnt] != '['; cnt++) {
				st->att[cnt] = set_color_num(color, st->att[cnt]);
			}
		} else
		for (cnt=ec-1; cnt > sc && isSpace(st->line[cnt]); cnt--) {
			if (which == 0) {
				st->att[cnt] = set_underline_to_0(st->att[cnt]);
				st->att[cnt] = set_italic_to_0(st->att[cnt]);
				st->att[cnt] = set_bold_to_0(st->att[cnt]);
			} else if (which == 1) {
				st->att[cnt] = set_underline_to_0(st->att[cnt]);
			} else if (which == 2) {
				st->att[cnt] = set_italic_to_0(st->att[cnt]);
			} else if (which == 3) {
				st->att[cnt] = set_bold_to_0(st->att[cnt]);
			} else if (which == 4) {
				st->att[cnt] = zero_color_num(st->att[cnt]);
			} else if (which == 99) { // 2019-04-17 ESC-C - color Red
				st->att[cnt] = set_error_to_0(st->att[cnt]);
			}
		}
	}
	return(TRUE);
}

static char changeStyleOfGlobalLine(char which, char color, char setAtt, ROWS *st, long sc) {
	char isOKtoChange, isDoAll, sb;
	long cnt, ec;

	ec = sc;
	cnt = 0;
	isDoAll = (which == 0) || (!global_df->ChatMode);
	sb = FALSE;
	if (!isDoAll) {
		isOKtoChange = FALSE;
		if (isSpeaker(st->line[cnt])) {
			for (; st->line[cnt] != ':' && st->line[cnt] != EOS; cnt++) ;
			if (st->line[cnt] == ':')
				cnt++;
		}
		do {
			if (st->line[cnt] == '[')
				sb = TRUE;
			else if (st->line[cnt] == ']')
				sb = FALSE;
			if (isSpace(st->line[cnt])) {
				for (; cnt < ec && isSpace(st->line[cnt]); cnt++) ;
			} else if (st->line[cnt] == '[' && st->line[cnt+1] == '-') {
				for (; cnt < ec && st->line[cnt] != ']' && st->line[cnt] != EOS; cnt++) ;
				if (st->line[cnt] == ']')
					cnt++;
			} else if (st->line[cnt] == '+') {
				for (; cnt < ec && !isSpace(st->line[cnt]) && st->line[cnt] != '[' && st->line[cnt] != EOS; cnt++) ;
			} else
				isOKtoChange = TRUE;
		} while (!isOKtoChange && st->line[cnt] != EOS && cnt < ec) ;
		if (st->line[cnt] == EOS)
			return(FALSE);
	} else
		isOKtoChange = TRUE;
	if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
		return(FALSE);
	for (; cnt < sc; cnt++) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
			return(FALSE);
	}
	if (which == 4 && isOKtoChange && !sb) {
		global_df->gAtt = set_color_num(color, global_df->gAtt);
	}
	if (cnt <= ec) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll) {
			global_df->gAtt = set_underline_to_0(global_df->gAtt);
			global_df->gAtt = set_italic_to_0(global_df->gAtt);
			global_df->gAtt = set_bold_to_0(global_df->gAtt);
			return(TRUE);
		}

		if (!isOKtoChange) ;
		else if (which == 0) {
			global_df->gAtt = set_underline_to_0(global_df->gAtt);
			global_df->gAtt = set_italic_to_0(global_df->gAtt);
			global_df->gAtt = set_bold_to_0(global_df->gAtt);
		} else if (which == 1) {
			if (setAtt) {
				global_df->gAtt = set_underline_to_1(global_df->gAtt);
				global_df->head_row->att = 1;
			} else
				global_df->gAtt = set_underline_to_0(global_df->gAtt);
		} else if (which == 2) {
			if (setAtt) {
				global_df->gAtt = set_italic_to_1(global_df->gAtt);
				global_df->head_row->att = 1;
			} else
				global_df->gAtt = set_italic_to_0(global_df->gAtt);
		} else if (which == 3) {
			if (setAtt) {
				global_df->gAtt = set_bold_to_1(global_df->gAtt);
				global_df->head_row->att = 1;
			} else
				global_df->gAtt = set_bold_to_0(global_df->gAtt);
		} else if (which == 4) {
			if (!uS.isskip(st->line,cnt,&dFnt,TRUE) && !sb)
				global_df->head_row->att = 1;
			global_df->gAtt = set_color_num(color, global_df->gAtt);
		}
	}
	return(TRUE);
}

void StyleItems(char which, char color) {
    char setAtt = FALSE, isRightTier;
	long cnt;
	long sc = 0L, ec = 0L;
	ROWS *st, *et = global_df->row_txt, *tt;

	init_punct(0);
	ChangeCurLineAlways(0);
	if (global_df->ScrollBar && (global_df->row_win2 || global_df->col_win2 != -2L)) {
		if (global_df->row_win2 == 0) {
			for (tt=global_df->row_txt; !isSpeaker(tt->line[0]) && !AtTopEnd(tt,global_df->head_text,FALSE); tt=ToPrevRow(tt, FALSE)) ;
			if (which == 99) // 2019-04-17 ESC-C - color Red
				isRightTier = TRUE;
			else if (tt->line[0] == '*' || !global_df->ChatMode)
				isRightTier = TRUE;
			else
				return;
			if (global_df->row_txt->att == NULL) {
				global_df->row_txt->att = (AttTYPE *)malloc((strlen(global_df->row_txt->line)+1)*sizeof(AttTYPE));
				if (global_df->row_txt->att == NULL)
					mem_err(TRUE, global_df);
				for (cnt=0; global_df->row_txt->line[cnt]; cnt++)
					global_df->row_txt->att[cnt] = 0;
			}
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			if (which == 0) {
				setAtt = FALSE;
			} else if (which == 1) {
				if (is_underline(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 2) {
				if (is_italic(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 3) {
				if (is_bold(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 4) {
				setAtt = TRUE;
			} else if (which == 99) { // 2019-04-17 ESC-C - color Red
				if (is_error(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			}
			if (changeStyleOfCurrentLine(which, color, setAtt, global_df->row_txt, sc, ec))
				CpCur_lineAttToHead_rowAtt(global_df->cur_line);
		} else {
			if (global_df->row_win2 < 0) {
				et = global_df->row_txt;
				for (cnt=global_df->row_win2, st=global_df->row_txt; cnt && !AtTopEnd(st,global_df->head_text,FALSE); 
								cnt++, st=ToPrevRow(st, FALSE)) ;
				sc = global_df->col_chr2; ec = global_df->col_chr;
			} else if (global_df->row_win2 > 0L) {
				st = global_df->row_txt;
				for (cnt=global_df->row_win2, et=global_df->row_txt; cnt && !AtBotEnd(et,global_df->tail_text,FALSE);
								cnt--, et=ToNextRow(et, FALSE)) ;
				sc = global_df->col_chr; ec = global_df->col_chr2;
			} else
				st = global_df->row_txt;

			for (tt=st; !isSpeaker(tt->line[0]) && !AtTopEnd(tt,global_df->head_text,FALSE); tt=ToPrevRow(tt, FALSE)) ;
			if (which == 99) // 2019-04-17 ESC-C - color Red
				isRightTier = TRUE;
			else if (tt->line[0] == '*' || !global_df->ChatMode)
				isRightTier = TRUE;
			else
				isRightTier = FALSE;

			if (isRightTier) {
				if (st->att == NULL) {
					st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
					if (st->att == NULL)
						mem_err(TRUE, global_df);
					for (cnt=0; st->line[cnt]; cnt++)
						st->att[cnt] = 0;
				}
				if (which == 0) {
					setAtt = FALSE;
				} else if (which == 1) {
					if (is_underline(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 2) {
					if (is_italic(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 3) {
					if (is_bold(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 4) {
					setAtt = TRUE;
				} else if (which == 99) { // 2019-04-17 ESC-C - color Red
					if (is_error(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				}
				changeStyleOfCurrentLine(which, color, setAtt, st, sc, strlen(st->line));
			} else {
				if (which == 0)
					setAtt = FALSE;
				else
					setAtt = TRUE;
			}

			if (!AtBotEnd(st,et,FALSE)) {
				do {
					st = ToNextRow(st, FALSE);
					if (which == 99) // 2019-04-17 ESC-C - color Red
						isRightTier = TRUE;
					else if (st->line[0] == '*' || !global_df->ChatMode)
						isRightTier = TRUE;
					else if (st->line[0] == '@' || st->line[0] == '%')
						isRightTier = FALSE;
					if (isRightTier) {
						if (st->att == NULL) {
							st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
							if (st->att == NULL)
								mem_err(TRUE, global_df);
							for (cnt=0; st->line[cnt]; cnt++)
								st->att[cnt] = 0;
						}
						changeStyleOfCurrentLine(which, color, setAtt, st, 0, strlen(st->line));
					}
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			st = ToNextRow(st, FALSE);
			if (which == 99) // 2019-04-17 ESC-C - color Red
				isRightTier = TRUE;
			else if (st->line[0] == '*' || !global_df->ChatMode)
				isRightTier = TRUE;
			else if (st->line[0] == '@' || st->line[0] == '%')
				isRightTier = FALSE;
			if (isRightTier) {
				if (st->att == NULL) {
					st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
					if (st->att == NULL)
						mem_err(TRUE, global_df);
					for (cnt=0; st->line[cnt]; cnt++)
						st->att[cnt] = 0;
				}
				changeStyleOfCurrentLine(which, color, setAtt, st, 0, ec);
			}
			CpCur_lineAttToHead_rowAtt(global_df->cur_line);
		}
		DisplayTextWindow(NULL, 1);
		PosAndDispl();
#ifdef _MAC_CODE
		SetTextWinMenus(TRUE);
#else
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		} else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
			else
				global_df->gAtt = 0;
		} else
			global_df->gAtt = 0;
#endif
	} else {
		sc = global_df->col_chr;
		st = global_df->row_txt;
		if (which == 0) {
			setAtt = FALSE;
		} else if (which == 1) {
			if (is_underline(global_df->gAtt))
				setAtt = FALSE;
			else
				setAtt = TRUE;
		} else if (which == 2) {
			if (is_italic(global_df->gAtt))
				setAtt = FALSE;
			else
				setAtt = TRUE;
		} else if (which == 3) {
			if (is_bold(global_df->gAtt))
				setAtt = FALSE;
			else
				setAtt = TRUE;
		} else if (which == 4) {
			setAtt = TRUE;
		} else if (which == 99) { // 2019-04-17 ESC-C - color Red
			if (is_error(global_df->gAtt))
				setAtt = FALSE;
			else
				setAtt = TRUE;
		}
		changeStyleOfGlobalLine(which, color, setAtt, st, sc);
#ifdef _MAC_CODE
		SetTextWinMenus(FALSE);
#endif
	}
}

static void remAllBlanks(unCH *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == '_'; i++) ;
	if (i > 0)
		strcpy(st, st+i);
	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '_' || st[i] == NL_C || st[i] == SNL_C)) i--;
	st[i+1] = EOS;
}

static char *getTierText(ROWS *tr, int max) {
	int  i;
	int  j;
	char hf, spf;

	while (!isSpeaker(tr->line[0]) && !AtTopEnd(tr, global_df->head_text, FALSE))
		tr = ToPrevRow(tr, FALSE);
	for (i=0; tr->line[i] != ':' && tr->line[i] != EOS; i++) ;
	if (tr->line[i] == ':')
		for (i++; isSpace(tr->line[i]); i++) ;
	hf = FALSE;
	spf = FALSE;
	for (j=0; j < max-2 && tr->line[i] != EOS; i++) {
		if (tr->line[i] == HIDEN_C)
			hf = !hf;
		else if (!hf) {
			if (isSpace(tr->line[i]) || tr->line[i] == '<' || tr->line[i] == '>' || tr->line[i] == NL_C) {
				if (!spf)
					templine2[j++] = '_';
				spf = TRUE;
			} else {
				templine2[j++] = tr->line[i];
				spf = FALSE;
			}
		}
	}
	templine2[j] = EOS;
	remAllBlanks(templine2);
	UnicodeToUTF8(templine2, j, (unsigned char *)templineC2, NULL, UTTLINELEN);
	return(templineC2);
}

void getTextAndSendToSoundAnalyzer(void) {
	FNType fname[FNSize];
	const char *err;
	char reportError;
	long beg, end;
	long cnt;
	double dBeg, dEnd;
	extern const char *sendMessage(const char *mess);
	extern const char *sendpraat (void *display, const char *programName, long timeOut, const char *text);

	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	cnt = global_df->row_win2;
	if (cnt < 0L) {
		beg = 0 - cnt;
		while (cnt < 0L) {
			if (AtTopEnd(global_df->row_txt,global_df->head_text,FALSE))
				return;
			cnt++;
			MoveUp(0);
		}
		cnt = beg;
	}
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	global_df->isExtend = 0;
	if (cnt < 0L) {
		cnt++;
		MoveUp(0);
		reportError = FALSE;
	} else if (cnt > 0L) {
		cnt--;
//2013-04-10		MoveDown(0);
		reportError = FALSE;
	} else
		reportError = TRUE;

repeat_this:
	ChangeCurLine();
	if (FindSndInfoAndCopyIt(fname, &beg, &end)) {
		dBeg = beg;
		dEnd = end;
		if (sendMessageTargetApp == PRAAT) {
			dBeg /= 1000;
			dEnd /= 1000;
			sprintf(templineC, "Open long sound file... %s\n", fname);
//	.mov		sprintf(templineC, "Read from file... %s\n", fname);
			sprintf(templineC+strlen(templineC), "Rename... temp\n");
			sprintf(templineC+strlen(templineC), "Extract part... %lf %lf yes\n", dBeg, dEnd);
//	.mov		sprintf(templineC+strlen(templineC), "Extract part... %lf %lf Rectangular 1.0 yes\n", dBeg, dEnd);
			sprintf(templineC+strlen(templineC), "Rename... %s\n", getTierText(global_df->row_txt, 40));
			sprintf(templineC+strlen(templineC), "select LongSound temp\n");
			sprintf(templineC+strlen(templineC), "Remove\n");
			err = sendpraat((void *)NULL, "Praat", 0L, templineC);
		} else {
			err = "This function is not implemented yet. If you get this message please report problem with PitchWorks application to macw@cmu.edu.";
/*
#ifdef _MAC_CODE
			creator_type t;
			EventRecord theEvent;
			extern char LaunchAppFromSignature(OSType sig, char isTest);

			if (!LaunchAppFromSignature('PHW0', FALSE))
				err = "Can't fine PitchWorks application";
			else {
				EventAvail-commented(everyEvent, &theEvent);
				err = NULL;
			}
			t.out = global_df->SnTr.SFileType;
			sprintf(templineC, "OPEN_FILE\377%s\377%c%c%c%c\r", fname, t.in[0], t.in[1], t.in[2], t.in[3]);
			sprintf(templineC+strlen(templineC), "PLAY_SELECTION\377%ld\377%ld\r", beg, end);
			sprintf(templineC+strlen(templineC), "COMMANDS_EXECUTE");
#elif defined(_WIN32) // _MAC_CODE
			err = "This function is not working on PC yet";
			sprintf(templineC, "OPEN_FILE\r%s\r%ld", fname, global_df->SnTr.SFileType);
#endif
			if (err == NULL) {
				if (*global_df->SnTr.SoundFile != EOS) {
					if (global_df->SnTr.isMP3 == TRUE) {
						global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE // NO QT lxs-mp3
						if (global_df->SnTr.mp3.hSys7SoundData)
							DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
						global_df->SnTr.mp3.theSoundMedia = NULL;
						global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
					} else {
						fclose(global_df->SnTr.SoundFPtr);
					}
					global_df->SnTr.SoundFPtr = 0;
					global_df->SnTr.SoundFile[0] = EOS;
				}
				err = sendMessage(templineC);
			}
*/
		}

		if (err != NULL) {
			if (*err == '-')
				strcpy(global_df->err_message, err);
			else
				do_warning(err, 0);
			return;
		}
	} else if (reportError)
		do_warning("Media marker not found at cursor position. Please make sure to play bullet first.", 0);
	if (cnt != 0L) {
		if (cnt < 0L) {
			if (AtTopEnd(global_df->row_txt,global_df->head_text,FALSE))
				return;
			cnt++;
			MoveUp(0);
		} else if (cnt > 0L) {
			if (AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE))
				return;
			cnt--;
			MoveDown(0);
		}
		goto repeat_this;
	}
}
