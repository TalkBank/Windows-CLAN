#import "DocumentWinController.h"
#import "WalkerController.h"
#import "cu.h"

extern char fgets_cr_lc;
extern char ced_version[];
extern struct PBCs PBC;

char isSavePrefs = TRUE;

unCH NextTierName[512];
FNType home_dir[FNSize];
FNType prefsDir[FNSize];
BOOL DefClan;
BOOL DefWindowDims;
BOOL isCursorPosRestore;
char isUTFData;
char isChatLineNums;
char isUpdateCLAN;
char F5Option;

DocumentWindowController *gLastTextWinController;

NSFont *defUniFont;
NSFont *defCAFont;

struct DefWin defWinSize;// 2020-05-15
struct DefWin ClanOutSize;// 2020-05-14
struct DefWin ClanWinSize;// 2020-01-30
struct DefWin MovieWinSize;
struct DefWin PictWinSize; // 2021-05-01
struct DefWin SpCharsWinSize;// 2020-05-14

creator_type the_file_creator;
CFByteOrder byteOrder;

#define myIsAllDigit(c) ((c >= '0' && c <= '9') || c == '-')

static void SetOption(char *text) {
	int  i, id;
	int height, width, top;

	id = atoi(text);
	while (isdigit(*text)) text++;
	while (*text == ' ' || *text == '\t' || *text == '=') text++;

	if (id == 500) {
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						MovieWinSize.left = atoi(text);
						MovieWinSize.height = height;
						MovieWinSize.width = width;
						MovieWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 501) { // 2020-01-30
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						ClanWinSize.left = atoi(text);
						ClanWinSize.height = height;
						ClanWinSize.width = width;
						ClanWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 504) {// 2021-05-01
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						PictWinSize.left = atoi(text);
//						PictWinSize.height = height;
//						PictWinSize.width = width;
						PictWinSize.top = top;
					}
				}
			}
		}
	} else if (id == CLANWIN) {// 2020-05-14
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						ClanOutSize.left = atoi(text);
						ClanOutSize.height = height;
						ClanOutSize.width = width;
						ClanOutSize.top = top;
					}
				}
			}
		}
	} else if (id == SpChrWIN) {// 2020-05-14
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						SpCharsWinSize.left = atoi(text);
						SpCharsWinSize.height = height;
						SpCharsWinSize.width = width;
						SpCharsWinSize.top = top;
					}
				}
			}
		}
	} else if (id == DOCWIN) {// 2020-05-15
		if (*text != EOS) {
			height = atoi(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				width = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
					top = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						defWinSize.left = atoi(text);
						defWinSize.height = height;
						defWinSize.width = width;
						defWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 1969) {
		if (*text != EOS) {
			PBC.LoopCnt = atoi(text);
			if (PBC.LoopCnt < 1)
				PBC.LoopCnt = 1;
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS) {
				PBC.backspace = atol(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				for (; *text && !myIsAllDigit(*text); text++) ;
				if (*text != EOS) {
// 2019-01-29		PBC.speed = atoi(text);
					for (; *text && myIsAllDigit(*text); text++) ;
					for (; *text && !myIsAllDigit(*text); text++) ;
					if (*text != EOS) {
						PBC.step_length = atol(text);
						if (PBC.step_length < 0L)
							PBC.step_length = -PBC.step_length;
						for (; *text && myIsAllDigit(*text); text++) ;
						for (; *text && !myIsAllDigit(*text); text++) ;
						if (*text != EOS) {
//							PBC.enable = atoi(text);
							for (; *text && myIsAllDigit(*text); text++) ;
							for (; *text && !myIsAllDigit(*text); text++) ;
							if (*text != EOS) {
								PBC.pause_len = atol(text);
							}
						}
					}
				}
			}
		}
	} else if (id == 1970) {
/*
 		if (*text != EOS) {
			if (!access(text, 0)) {
				strcpy(lib_dir, text);
				if (isRefEQZero(mor_lib_dir))
					strcpy(mor_lib_dir, lib_dir);
			}
		}
*/
	} else if (id == 1972) {
		for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
		text[top] = EOS;
		strcpy(DisTier, text);
		if (*DisTier == EOS)
			strcpy(DisTier, "%MOR:");
		else
			uS.uppercasestr(DisTier, &dFnt, C_MBF);
	} else if (id == 1974) {
		if (*text != EOS) {
			if (!access(text, 0))
				strcpy(wd_dir, text);
		}
	} else if (id == 1975) {
		if (*text != EOS) {
			if (!access(text, 0))
				strcpy(od_dir, text);
		}
	} else if (id == 19760) {
#ifdef _MAC_CODE
		if (*text != EOS) {
			stickyFontSize = atol(text);
		}
#endif // _MAC_CODE
	} else if (id == 1977) {
		if (*text != EOS) {
			for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
			text[top] = EOS;
			set_commands(text);
		}
	} else if (id == 1978) {
		if (*text != EOS) {
			DefClan = (char)atoi(text);
		}
	} else if (id == 1979) {
		if (*text != EOS) {
			DefWindowDims = (char)atoi(text);
		}
	} else if (id == 1980) {
		if (*text != EOS) {
			set_lastCommand((short)atoi(text));
		}
	} else if (id == 1989) {
		if (*text != EOS) {
			isCursorPosRestore = (char)atoi(text);
		}
	} else if (id == 1994) {
		if (*text != EOS) {
			if (!access(text, 0))
				strcpy(mor_lib_dir, text);
		}
	} else if (id == 1995) {
		if (*text != EOS) {
			doMixedSTWave = (char)atoi(text);
		}
	} else if (id == 1998) {
		if (*text != EOS)
			isChatLineNums = atoi(text);
	} else if (id == 2006) {
		if (*text != EOS)
			F5Option = atoi(text);
	} else if (id == 2011) {
		if (*text != EOS) {
			for (i=0; text[i] != '\n' && text[i] != EOS; i++) ;
			text[i] = EOS;
			set_folders(1, text);
		}
	} else if (id == 2012) {
		if (*text != EOS) {
			for (i=0; text[i] != '\n' && text[i] != EOS; i++) ;
			text[i] = EOS;
			set_folders(2, text);
		}
	}
}

static void ReadPreference(const char *fname, FNType *path) {
	int   len;
	FILE  *fp;

	len = strlen(path);
	addFilename2Path(path, fname);
	if ((fp=fopen(path,"r")) == NULL) {
		path[len] = 0;
		return;
	}
	path[len] = 0;

	fgets_cr_lc = '\0';
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remFrontAndBackBlanks(templineC);
		if (isdigit(*templineC)) {
			SetOption(templineC);
		}
	}
	fclose(fp);
}

static void WriteCedPrefs(FILE *fp) {
#ifdef _MAC_CODE
/* 2015 lxs
	Str255 pFontName;
	char FontName[256];
*/
#endif // _MAC_CODE

	fprintf(fp, "[ced]\n");

	fprintf(fp, "%d=%d %d %d %d\n", 500,
			MovieWinSize.height,MovieWinSize.width,MovieWinSize.top,MovieWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 501,
			ClanWinSize.height, ClanWinSize.width, ClanWinSize.top, ClanWinSize.left); // 2020-01-30
	fprintf(fp, "%d=%d %d %d %d\n", 504,
			PictWinSize.height, PictWinSize.width, PictWinSize.top, PictWinSize.left);// 2021-05-01
	fprintf(fp, "%d=%d %d %d %d\n", CLANWIN,
			ClanOutSize.height, ClanOutSize.width, ClanOutSize.top, ClanOutSize.left); // 2020-05-14
	fprintf(fp, "%d=%d %d %d %d\n", SpChrWIN,
			SpCharsWinSize.height, SpCharsWinSize.width, SpCharsWinSize.top, SpCharsWinSize.left); // 2020-05-14
	fprintf(fp, "%d=%d %d %d %d\n", DOCWIN,
			defWinSize.height, defWinSize.width, defWinSize.top, defWinSize.left); // 2020-05-15
	fprintf(fp, "%d=%d %ld %d %ld %d %ld\n", 1969, PBC.LoopCnt, PBC.backspace, PBC.speed, PBC.step_length, (int)PBC.enable, PBC.pause_len);
	fprintf(fp, "%d=%s\n", 1970, lib_dir);
	fprintf(fp, "%d=%s\n", 1972, DisTier);
	fprintf(fp, "%d=%s\n", 1974, wd_dir);
	fprintf(fp, "%d=%s\n", 1975, od_dir);
	fprintf(fp, "%d=%ld\n", 19760, stickyFontSize);
	fprintf(fp, "%d=%d\n", 1978, DefClan);
	fprintf(fp, "%d=%d\n", 1979, DefWindowDims);
	fprintf(fp, "%d=%d\n", 1989, isCursorPosRestore);
	fprintf(fp, "%d=%s\n", 1994, mor_lib_dir);
	fprintf(fp, "%d=%d\n", 1995, doMixedSTWave);
	fprintf(fp, "%d=%d\n", 1998, isChatLineNums);
	fprintf(fp, "%d=%d\n", 2006, F5Option);
}

void WriteCedPreference(void) {
	int	 t;
	FILE *fp;

	t = strlen(prefsDir);
	if (t == 0)
		return;
	addFilename2Path(prefsDir, CED_PREF_FILE);
	if ((fp=fopen(prefsDir,"w")) == NULL) {
		sprintf(templineC4, "Can't write file: %s", prefsDir);
		prefsDir[t] = EOS;
		do_warning(templineC4, 0);
		return;
	}
#ifdef _MAC_CODE
	settyp(prefsDir, 'TEXT', the_file_creator.out, FALSE);
#endif
	prefsDir[t] = EOS;
	if (isSavePrefs)
		WriteCedPrefs(fp);
	fclose(fp);
}

void WriteClanPreference(void) {
	int	 t;
	char tIsUnixCRs;
	FILE *fp;

	t = strlen(prefsDir);
	if (t == 0)
		return;
	addFilename2Path(prefsDir, CLAN_PREF_FILE);
	if ((fp=fopen(prefsDir,"w")) == NULL) {
		sprintf(templineC4, "Can't write file: %s", prefsDir);
		prefsDir[t] = EOS;
		do_warning(templineC4, 0);
		return;
	}
#ifdef _MAC_CODE
	settyp(prefsDir, 'TEXT', the_file_creator.out, FALSE);
#endif
	prefsDir[t] = EOS;

	if (isSavePrefs) {
		tIsUnixCRs = isUnixCRs;
		isUnixCRs = FALSE;
 		write_commands1977(fp);
 		write_folders_2011_2013(fp);
		fprintf(fp, "%d=%d\n", 1980, lastCommand);
		isUnixCRs = tIsUnixCRs;
	}
	fclose(fp);
}


void LocalInit(void) {
	int i;
	FNType mFileName[FNSize+2];
	FSRef  tRef;
	extern int  CLANVersion;
	extern char VERSION[];
	extern long MAXOUTCOL;

//2015 lxs 	FSSpec fss;

	the_file_creator.out = 'MCed';

/*
	SetDefaultUnicodeFinfo(&dFnt);
	dFnt.fontTable = NULL;
	dFnt.isUTF = 1;
*/
	strcpy(dFnt.fontName, DEFAULT_FONT);
	dFnt.fontId = DEFAULT_ID;
	dFnt.fontSize = DEFAULT_SIZE;
	dFnt.fontType = getFontType(DEFAULT_FONT, FALSE);
	dFnt.orgFType = dFnt.fontType;
	dFnt.CharSet = my_FontToScript(dFnt.fontId, dFnt.CharSet);
	dFnt.Encod = dFnt.CharSet;
	dFnt.orgEncod = dFnt.Encod;
	dFnt.isUTF = 0;

	stickyFontSize = 15;

	initFoldersList();
/* 2015 lxs
	OSStatus err = HGetVol(fss.name, &fss.vRefNum, &fss.parID);
	fss.name[0] = 0;
	err = FSpMakeFSRef(&fss, &tRef);
	err = my_FSRefMakePath(&tRef, mFileName, FNSize);
*/
//	getcwd(mFileName, FNSize);
	NSBundle *main = [NSBundle mainBundle];
	strncpy(mFileName, [[main bundlePath] UTF8String], FNSize);
	mFileName[FNSize] = EOS;
	i = strlen(mFileName);
	if (mFileName[i-1] != PATHDELIMCHR)
		uS.str2FNType(mFileName, i, PATHDELIMSTR);
	strcpy(home_dir, mFileName);
	for (i=strlen(home_dir)-1; i >= 0; i--) {
		if (uS.FNTypeicmp(home_dir+i, ".app", 4) == 0) {
			for (; i >= 0 && home_dir[i] != PATHDELIMCHR; i--) ;
			break;
		}
	}
	if (i >= 0 && home_dir[i] == '/') {
		i++;
		home_dir[i] = EOS;
		strcpy(wd_dir, home_dir);
		addFilename2Path(wd_dir, "work/");
		strcpy(lib_dir, home_dir);
		addFilename2Path(lib_dir, "lib/");
		if (access(lib_dir, 0)) {
			strcpy(lib_dir, "/Users/WORK/ClanW/(LIB)/");
		}
	} else {
		strcpy(wd_dir, mFileName);
		strcpy(lib_dir, mFileName);
	}
	strcpy(mor_lib_dir, "MOR lib is not set!");
	strcpy(od_dir, wd_dir);

	gLastTextWinController = nil;

	doMixedSTWave = FALSE;
	isUTFData = TRUE;
	byteOrder = CFByteOrderGetCurrent();
	isUnixCRs = FALSE;
	DefClan = TRUE;
	DefWindowDims = FALSE;
	isCursorPosRestore = TRUE;
	isChatLineNums = FALSE;// 2020-09-18
	F5Option = EVERY_TIER;

	NextTierName[0] = EOS;
	strcpy(DisTier, "%MOR:");

	MAXOUTCOL = 76L;

	defWinSize.top = defWinSize.width = defWinSize.height = defWinSize.left = 0;// 2020-05-15
	ClanOutSize.top = ClanOutSize.width = ClanOutSize.height = ClanOutSize.left = 0;// 2020-05-14
	ClanWinSize.top = ClanWinSize.left = ClanWinSize.width = ClanWinSize.height = 0;// 2020-01-30
	MovieWinSize.top = MovieWinSize.width = MovieWinSize.height = MovieWinSize.left = 0;// 2020-03-06
	PictWinSize.top = PictWinSize.width = PictWinSize.height = PictWinSize.left = 0;// 2021-05-01
	SpCharsWinSize.top = SpCharsWinSize.width = SpCharsWinSize.height = SpCharsWinSize.left = 0;// 2020-05-14

	PBC.enable = 0;
	PBC.LoopCnt = 1;
	PBC.backspace = 500L;
	PBC.speed = 100;
	PBC.step_length = 4000L;
	PBC.pause_len = 0L;

	InitFileDialog();
	InitKidevalOptions();
	InitEvalOptions();
	main_check_init();
	init_commands();

	isSavePrefs = TRUE;
	ced_lineC[0] = EOS; ced_lineC[1] = EOS;

	/* taking care of preferences in preferences file */
	if (FSFindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, prefsDir, FNSize);
		addFilename2Path(prefsDir, "CLAN/");
		if (access(prefsDir, 0)) {
			if (my_mkdir(prefsDir, S_IRWXU|S_IRGRP|S_IROTH))
				strcpy(prefsDir, home_dir);
		}
	} else
		strcpy(prefsDir, home_dir);

//2015 lxs 	strcpy(webDownLoadDir, prefsDir);

	ReadPreference(CED_PREF_FILE, prefsDir);
	ReadPreference(CLAN_PREF_FILE, prefsDir);

	CGFloat fontSize = (CGFloat)stickyFontSize;

	defUniFont = [NSFont fontWithName:@"Arial Unicode MS" size:fontSize]; // 15.0];
	if (defUniFont == nil) {
		//		do_warning("Can't locate \"Arial Unicode MS\" font. Using default font instead.", 0);
		defUniFont = [NSFont userFontOfSize:fontSize]; // 15.0];
	}

	defCAFont = [NSFont fontWithName:@"CAfont" size:fontSize]; // 15.0];
	if (defCAFont == nil) {
		do_warning("Can't locate \"CAfont\".\nPlease consider installing CAfont from\n\"https://dali.talkbank.org/clan/CAfont.otf\"\nUsing default font instead.", 0);
		defCAFont = [NSFont userFontOfSize:fontSize]; // 15.0];
	}

	/* taking care of preferences in resourse fork */
	strcpy(FileName1, lib_dir);

	i = 0;
	//	ced_version[i++] = 'U';
	ced_version[i++] = VERSION[2];
	ced_version[i++] = VERSION[3];
	if (!strncmp(VERSION+5, "Jan", 3))
		strcpy(ced_version+i, "Jan");
	else if (!strncmp(VERSION+5, "Feb", 3))
		strcpy(ced_version+i, "Feb");
	else if (!strncmp(VERSION+5, "Mar", 3))
		strcpy(ced_version+i, "Mar");
	else if (!strncmp(VERSION+5, "Apr", 3))
		strcpy(ced_version+i, "Apr");
	else if (!strncmp(VERSION+5, "May", 3))
		strcpy(ced_version+i, "May");
	else if (!strncmp(VERSION+5, "Jun", 3))
		strcpy(ced_version+i, "Jun");
	else if (!strncmp(VERSION+5, "Jul", 3))
		strcpy(ced_version+i, "Jul");
	else if (!strncmp(VERSION+5, "Aug", 3))
		strcpy(ced_version+i, "Aug");
	else if (!strncmp(VERSION+5, "Sep", 3))
		strcpy(ced_version+i, "Sep");
	else if (!strncmp(VERSION+5, "Oct", 3))
		strcpy(ced_version+i, "Oct");
	else if (!strncmp(VERSION+5, "Nov", 3))
		strcpy(ced_version+i, "Nov");
	else if (!strncmp(VERSION+5, "Dec", 3))
		strcpy(ced_version+i, "Dec");
	else
		strcpy(ced_version+i, "--");
/*
	if (!strncmp(VERSION+5, "Jan", 3))
		strcpy(ced_version+i, "01");
	else if (!strncmp(VERSION+5, "Feb", 3))
		strcpy(ced_version+i, "02");
	else if (!strncmp(VERSION+5, "Mar", 3))
		strcpy(ced_version+i, "03");
	else if (!strncmp(VERSION+5, "Apr", 3))
		strcpy(ced_version+i, "04");
	else if (!strncmp(VERSION+5, "May", 3))
		strcpy(ced_version+i, "05");
	else if (!strncmp(VERSION+5, "Jun", 3))
		strcpy(ced_version+i, "06");
	else if (!strncmp(VERSION+5, "Jul", 3))
		strcpy(ced_version+i, "07");
	else if (!strncmp(VERSION+5, "Aug", 3))
		strcpy(ced_version+i, "08");
	else if (!strncmp(VERSION+5, "Sep", 3))
		strcpy(ced_version+i, "09");
	else if (!strncmp(VERSION+5, "Oct", 3))
		strcpy(ced_version+i, "10");
	else if (!strncmp(VERSION+5, "Nov", 3))
		strcpy(ced_version+i, "11");
	else if (!strncmp(VERSION+5, "Dec", 3))
		strcpy(ced_version+i, "12");
	else
 		strcpy(ced_version+i, "--");
*/
	i = strlen(ced_version);
	ced_version[i++] = VERSION[11];
	ced_version[i++] = VERSION[12];

}

