#include "ced.h"
#include "cu.h"
#include "search.h"

#ifdef _MAC_CODE
    #include "my_ctype.h"
	#include "mac_print.h"
#endif // _MAC_CODE
#ifdef _WIN32
	#include <TextUtils.h>
#endif /* _WIN32 */

#include "MMedia.h"

extern char isAllFile;
extern char ced_version[];
extern char fgets_cr_lc;
extern char ClanAutoWrap;
extern long ClanWinRowLim;

static int  argc;
static char **argv;
static char isSavePrefs = TRUE;

static int  ReadPreference(const char *fname, int argc, FNType *path);

char *MEMPROT = NULL;
FNType prefsDir[FNSize];
FNType home_dir[FNSize];
FNType lib_dir[FNSize];
FNType mor_lib_dir[FNSize];
char URL_Address[512];
char proxyAddPort[256];
char DefClan;
char DefWindowDims;
char isUseSPCKeyShortcuts;
char isUnixCRs;
char isUTFData;
char isChatLineNums;
char isUpdateCLAN;
char isCursorPosRestore;
char PlayingSound;
char PlayingContSound;
char PlayingContMovie;
char AutoScriptSelect;
char sendMessageTargetApp;
char *TempFileStringTag;
char F5Option;
char  rptMark;
int   rptPTime;
int   WebCLANVersion;
int   streamSpeedNumber;
time_t versionTime;
short TabSize;
short mScript;
short ArialUnicodeFOND;
short SecondDefUniFOND;
double ThumbnailsHight;
NewFontInfo dFnt, oFnt;
struct DefWin defWinSize;
struct DefWin MovieWinSize;
struct DefWin PictWinSize;
struct DefWin TextWinSize;
struct DefWin ClanOutSize;
struct DefWin ClanWinSize;
struct DefWin ThumbnailWin;
struct DefWin RecallWinSize;
struct DefWin ProgsWinSize;
struct DefWin FolderWinSize;
struct DefWin WebItemsWinSize;
struct DefWin SpCharsWinSize;
struct DefWin HlpCmdsWinSize;
struct DefWin MVHLPWinSize;
struct DefWin PBCWinSize;
struct DefWin WarningWinSize;
struct DefWin ProgresWinSize;
//launch app by path; FileSpecs spellCheck;

#ifdef _MAC_CODE
extern long   pasteIndex;
extern Handle pasteStr;

creator_type the_file_creator;
CFByteOrder byteOrder;

FNType webDownLoadDir[FNSize];
FNType globalDir[FNSize];

FNType CLAN_Programs_str[FNSize];
FNType WEB_Dirs_str[FNSize];
FNType Special_Characters_str[FNSize];
FNType Commands_str[FNSize];
FNType Commands_Shortcuts_str[FNSize];
FNType CLAN_Output_str[FNSize]; // if changing look for "CLAN Output" in project
FNType Walker_Controller_str[FNSize];
FNType Movie_Help_str[FNSize];
FNType Movie_sound_str[FNSize];
FNType Picture_Name_str[FNSize];
FNType warning_str[FNSize];
FNType progress_str[FNSize];
FNType Recall_str[FNSize];
FNType CLAN_Folder_str[FNSize];
FNType ThumbNails_str[FNSize];
#endif

void LocalInit(void) {
	int i;
	extern int  CLANVersion;
	extern char VERSION[];

#ifdef _MAC_CODE
	char   buf[BUFSIZ];
	FSRef  tRef;
	FSSpec fss;
	FNType mFileName[FNSize];
	FILE *fpin, *fpout;

	print_init();
	InitMyWindows();
    MEMPROT = (char *)malloc((size_t)30000);
	pasteStr = 0;
	pasteIndex = 0L;

	the_file_creator.out = 'MCed';

	uS.str2FNType(CLAN_Programs_str, 0L, "CLAN Programs");
	uS.str2FNType(WEB_Dirs_str, 0L, "WEB Data");
	uS.str2FNType(Special_Characters_str, 0L, "Special Characters");
	uS.str2FNType(Commands_str, 0L, "Commands");
	uS.str2FNType(Commands_Shortcuts_str, 0L, "Commands and Shortcuts");
	uS.str2FNType(CLAN_Output_str, 0L, "CLAN Output"); // if changing look for "CLAN Output" in project
	uS.str2FNType(Walker_Controller_str, 0L, "Walker Controller");
	uS.str2FNType(Movie_Help_str, 0L, "Movie Help");
	uS.str2FNType(Movie_sound_str, 0L, "Movie - Sound");
	uS.str2FNType(Picture_Name_str, 0L, "Picture");
	uS.str2FNType(warning_str, 0L, "Warning");
	uS.str2FNType(progress_str, 0L, "Progress");
	uS.str2FNType(Recall_str, 0L, "Recall");
	uS.str2FNType(CLAN_Folder_str, 0L, " Folders");
	uS.str2FNType(ThumbNails_str, 0L, "ThumbNails");

	strcpy(defUniFontName, "Arial Unicode MS"); // lxs font lxslxs
	defUniFontSize = 12L;
	if (!GetFontNumber(defUniFontName, &ArialUnicodeFOND)) {
		ArialUnicodeFOND = 0;
		strcpy(defUniFontName, "CAfont"/*UNICODEFONT*/);
		defUniFontSize = 14L;
		if (!GetFontNumber(defUniFontName, &SecondDefUniFOND))
			SecondDefUniFOND = 0;
	} else {
		strcpy(templineC, "CAfont"/*UNICODEFONT*/);
		if (!GetFontNumber(templineC, &SecondDefUniFOND))
			SecondDefUniFOND = 0;
	}

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
	UniInputBuf.len = 0L;

	OSStatus err = HGetVol(fss.name, &fss.vRefNum, &fss.parID);
	fss.name[0] = 0;
	err = FSpMakeFSRef(&fss, &tRef);
	err = my_FSRefMakePath(&tRef, globalDir, FNSize);
//	getcwd(globalDir, FNSize);
	i = strlen(globalDir);
	if (globalDir[i-1] != PATHDELIMCHR)
		uS.str2FNType(globalDir, i, PATHDELIMSTR);
	strcpy(home_dir, globalDir);
	for (i=strlen(home_dir)-1; i >= 0; i--) {
		if (uS.FNTypeicmp(home_dir+i, ".app/", 5) == 0) {
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
	} else {
		strcpy(wd_dir, globalDir);
		strcpy(lib_dir, globalDir);
	}
	strcpy(mor_lib_dir, lib_dir);
	strcpy(od_dir, wd_dir);
	strcpy(webDownLoadDir, globalDir);

	byteOrder = CFByteOrderGetCurrent();
	initFavFont();
	initFoldersList();
	initURLDownload(TRUE);
	isUnixCRs = FALSE;
#endif // _MAC_CODE

#ifdef _WIN32
	wchar_t wDirPathName[FNSize];

	MEMPROT = NULL;
	strcpy(dFnt.fontName, DEFAULT_FONT);
	dFnt.fontSize = DEFAULT_SIZE;
	dFnt.fontType = getFontType(dFnt.fontName, TRUE);
	dFnt.orgFType = dFnt.fontType;
	dFnt.CharSet = DEFAULT_CHARSET;
	dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
	dFnt.isUTF = 0;

	winEncod = 0;
	unCH cp[7];
	if (GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_IDEFAULTCODEPAGE,cp,7) != 0) {
		if (strcmp(cp, "932") == 0)
			winEncod = JSCRIPT;
		else if (strcmp(cp, "936") == 0 || strcmp(cp, "950") == 0)
			winEncod = CSCRIPT;
		else if (strcmp(cp, "949") == 0 || strcmp(cp, "1361") == 0)
			winEncod = KSCRIPT;
	}
//GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SABBREVLANGNAME,cp,7);
//GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,cp,7);
//GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_IDEFAULTOEMCODEPAGE,cp,7);
//GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_IDEFAULTEBCDICCODEPAGE,cp,7);
	isUnixCRs = FALSE;
#endif

//launch app by path; 	spellCheck.name[0] = EOS;
	isSavePrefs = TRUE;
	TempFileStringTag = NULL;
    C_MBF = FALSE;
	isPlayS = 0;
	F_key = 0;
	TabSize = 8;
	DefClan = TRUE;
	DefWindowDims = FALSE;
	isChatLineNums = FALSE;
	doReWrap = FALSE;
	isUseSPCKeyShortcuts = TRUE;
	global_df = NULL;
	StdInWindow = NULL;
	StdInErrMessage = NULL;
	StdDoneMessage = NULL;
	DefAutoWrap = FALSE;
	ClanAutoWrap = FALSE;
	isUpdateCLAN = TRUE;
	doMixedSTWave = FALSE;
	rptMark = 0;
	rptPTime = 0;
	sendMessageTargetApp = PRAAT;
	F5Option = EVERY_TIER;
	F5_Offset = 0L;
	LineNumberDigitSize = 0;
	LineNumberingType = 0;
	isCursorPosRestore = FALSE;
	rawTextInput = FALSE;
	WebCLANVersion = CLANVersion;
	streamSpeedNumber = 0;
	versionTime = 0L;
#ifdef _UNICODE
	isUTFData = TRUE;
#else
	isUTFData = FALSE;
#endif
	FreqCountLimit = 0;
	SearchFFlag = TRUE;
	NextTierName[0] = EOS;
	CodesFName[0] = EOS;
	KeysFName[0] = EOS;
	MakeBackupFile = FALSE;
	ThumbnailsHight = 50.0000;
	DefChatMode = 2;
	DefCodingScheme = '\0';
	DefPriorityCodes = '\001';
	AutoScriptSelect = TRUE;
	PlayingSound = FALSE;
	PlayingContSound = FALSE;
	PlayingContMovie = FALSE;
	StartInEditorMode = TRUE;
	ShowPercentOfFile = FALSE;
	strcpy(URL_Address, "");
	strcpy(proxyAddPort, "");
	strcpy(DisTier, "%MOR:");
	uS.str2FNType(CODEFNAME, 0L, CODES_FILE);
	uS.str2FNType(STATEFNAME, 0L, KEYS_BIND_FILE);
	defWinSize.top = defWinSize.width = defWinSize.height = defWinSize.left = 0;
	WarningWinSize.top = WarningWinSize.width = WarningWinSize.height = WarningWinSize.left = 0;
	PBCWinSize.top = PBCWinSize.width = PBCWinSize.height = PBCWinSize.left = 0;
	ProgresWinSize.top = ProgresWinSize.width = ProgresWinSize.height = ProgresWinSize.left = 0;
	RecallWinSize.top = RecallWinSize.width = RecallWinSize.height = RecallWinSize.left = 0;
	ProgsWinSize.top = ProgsWinSize.width = ProgsWinSize.height = ProgsWinSize.left = 0;
	FolderWinSize.top = FolderWinSize.width = FolderWinSize.height = FolderWinSize.left = 0;	
	WebItemsWinSize.top = WebItemsWinSize.width = WebItemsWinSize.height = WebItemsWinSize.left = 0;
	SpCharsWinSize.top = SpCharsWinSize.width = SpCharsWinSize.height = SpCharsWinSize.left = 0;
	HlpCmdsWinSize.top = HlpCmdsWinSize.width = HlpCmdsWinSize.height = HlpCmdsWinSize.left = 0;
	ClanOutSize.top = ClanOutSize.width = ClanOutSize.height = ClanOutSize.left = 0;
	ThumbnailWin.top = ThumbnailWin.width = ThumbnailWin.height = ThumbnailWin.left = 0;
	TextWinSize.top = TextWinSize.width = TextWinSize.height = TextWinSize.left = 0;
	ClanWinSize.top = ClanWinSize.width = ClanWinSize.height = ClanWinSize.left = 0;
	MovieWinSize.top = MovieWinSize.width = MovieWinSize.height = MovieWinSize.left = 0;
	PictWinSize.top = PictWinSize.width = PictWinSize.height = PictWinSize.left = 0;
	MVHLPWinSize.top = 50;
	MVHLPWinSize.left = 50;
	MVHLPWinSize.width = 339;
	MVHLPWinSize.height = 327;
	MVWinWidth = 0;
	MVWinZoom = 1;
	PBC.enable = 0;
	PBC.isPC = 0;
	PBC.walk = 0;
	PBC.LoopCnt = 1;
	PBC.backspace = 500L;
	PBC.speed = 100;
	PBC.step_length = 4000L;
	PBC.pause_len = 0L;
	PBC.cur_pos = 0L;
	init_Search();
	InitFileDialog();
	InitSelectedTiers();
	InitEvalOptions();
	InitSelectedSearch();
	main_check_init();
	init_commands();
	for (i=0; i < 10; i++)
		ConstString[i] = NULL;

	ced_lineC[0] = EOS; ced_lineC[1] = EOS;
/* taking care of preferences in preferences file */
#ifdef _MAC_CODE
	if (FSFindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, prefsDir, FNSize);
		addFilename2Path(prefsDir, "CLAN/");
		if (access(prefsDir, 0)) {
			if (my_mkdir(prefsDir, S_IRWXU|S_IRGRP|S_IROTH))
				strcpy(prefsDir, home_dir);
		}
	} else
		strcpy(prefsDir, home_dir);

	i = strlen(prefsDir);
	addFilename2Path(prefsDir, CED_PREF_FILE);
	if (access(prefsDir, 0)) {
		strncpy(mFileName, prefsDir, i);
		mFileName[i] = EOS;
		addFilename2Path(mFileName, OLD_CED_PREF_FILE);
		if (!access(mFileName, 0)) {
			fpin = fopen(mFileName, "rb");
			if (fpin != NULL) {
				fpout = fopen(prefsDir, "wb");
				if (fpout != NULL) {
					settyp(prefsDir, 'TEXT', the_file_creator.out, FALSE);
					while (!feof(fpin)) {
						fgets(buf, BUFSIZ, fpin);
						fputs(buf, fpout);
					}
					fclose(fpin);
					fclose(fpout);
				} else
					fclose(fpin);
				fpin = NULL;
			}
		}
	}
	prefsDir[i] = EOS;
	addFilename2Path(prefsDir, CLAN_PREF_FILE);
	if (access(prefsDir, 0)) {
		strncpy(mFileName, prefsDir, i);
		mFileName[i] = EOS;
		addFilename2Path(mFileName, OLD_CLAN_PREF_FILE);
		if (!access(mFileName, 0)) {
			fpin = fopen(mFileName, "rb");
			if (fpin != NULL) {
				fpout = fopen(prefsDir, "wb");
				if (fpout != NULL) {
					settyp(prefsDir, 'TEXT', the_file_creator.out, FALSE);
					while (!feof(fpin)) {
						fgets(buf, BUFSIZ, fpin);
						if (strncmp(buf, "2011", 4) && strncmp(buf, "2012", 4) && strncmp(buf, "2013", 4) && strncmp(buf, "2014", 4))
							fputs(buf, fpout);
					}
					fclose(fpin);
					fclose(fpout);
				} else
					fclose(fpin);
				fpin = NULL;
			}
		}
	}
	prefsDir[i] = EOS;
	strcpy(webDownLoadDir, prefsDir);
#endif
#ifdef _WIN32
	if (GetWindowsDirectory(wDirPathName, FNSize) == 0)
		strcpy(prefsDir, PATHDELIMSTR);
	else {
		u_strcpy(prefsDir, wDirPathName, FNSize);
		addFilename2Path(prefsDir, "Clan");
	}
	strcpy(home_dir, "C:\\talkbank\\clan");
	if (chdir(home_dir)) {
		if (getcwd(home_dir, FNSize) == NULL) {
			if (GetCurrentDirectory(FNSize, wDirPathName) == 0)
				home_dir[0] = EOS;
			else
				u_strcpy(home_dir, wDirPathName, FNSize);
		}
	}
	if (chdir(prefsDir)) {
		if (my_mkdir(prefsDir, 0)) {
			if (home_dir[0] == EOS)
				strcpy(prefsDir, PATHDELIMSTR);
			else
				strcpy(prefsDir, home_dir);
/*
			if (GetWindowsDirectory(templine, FNSize) == 0)
				strcpy(prefsDir, PATHDELIMSTR);
			else
				strcpy(prefsDir, templine);
*/
		}
	}
	if (home_dir[0] == EOS) {
		strcpy(home_dir, prefsDir);	
		strcat(home_dir, PATHDELIMSTR);	
		strcpy(lib_dir, home_dir);
	} else {
		strcat(home_dir, PATHDELIMSTR);	
		strcpy(lib_dir, home_dir);
		addFilename2Path(lib_dir, "lib\\");
	}
	strcat(prefsDir, PATHDELIMSTR);	
	strcpy(mor_lib_dir, lib_dir);
	strcpy(wd_dir, home_dir);
	addFilename2Path(wd_dir, "work\\");
	strcpy(od_dir, wd_dir);
#endif
	argc = 1;
	argc = ReadPreference(CED_PREF_FILE, argc, prefsDir);
	argc = ReadPreference(CLAN_PREF_FILE, argc, prefsDir);
/* taking care of preferences in preferences file */

	if ((argv=(char **)malloc(sizeof(char *) * argc))== NULL)
		mem_err(FALSE, global_df);
	if ((argv[0] = (char *)malloc(strlen("CLAN")+1)) == NULL)
		mem_err(FALSE, global_df);
	strcpy(argv[0], "CLAN");

/* taking care of preferences in resourse fork */
	argc = 1;
	if (ced_lineC[1] != EOS) {
		int  beg;

		beg = 1;
		do {
			if (ced_lineC[beg] == '-' || ced_lineC[beg] == '+') {
				if ((argv[argc]=(char *)malloc(strlen(ced_lineC+beg)+1)) == NULL)
					mem_err(FALSE, global_df);
				strcpy(argv[argc], ced_lineC+beg);
				argc++;
				beg += strlen(ced_lineC+beg) + 1;
				if (ced_lineC[beg] == EOS) break;
			} else if (ced_lineC[beg] == EOS && ced_lineC[beg+1] == EOS) {
				break;
			} else beg++;
		} while (1) ;
	}
/* taking care of preferences in resourse fork */
#ifdef _MAC_CODE
	SetCurrentFontParams(dFnt.fontId, dFnt.fontSize);
#endif
#ifdef _WIN32
	for (i=1; i < argc; i++) {
		if (*argv[i] == '-' || *argv[i] == '+')
			ced_getflag(argv[i]);
	}
#endif
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, STATEFNAME);
	init_keys(FileName1, STATEFNAME);

	i = 0;
#ifdef _UNICODE
//	ced_version[i++] = 'U';
#endif
	ced_version[i++] = VERSION[2];
	ced_version[i++] = VERSION[3];
	if (!strncmp(VERSION+5, "Jan", 3))
		strcpy(ced_version+i, "jan");
	else if (!strncmp(VERSION+5, "Feb", 3))
		strcpy(ced_version+i, "feb");
	else if (!strncmp(VERSION+5, "Mar", 3))
		strcpy(ced_version+i, "mar");
	else if (!strncmp(VERSION+5, "Apr", 3))
		strcpy(ced_version+i, "apr");
	else if (!strncmp(VERSION+5, "May", 3))
		strcpy(ced_version+i, "may");
	else if (!strncmp(VERSION+5, "Jun", 3))
		strcpy(ced_version+i, "jun");
	else if (!strncmp(VERSION+5, "Jul", 3))
		strcpy(ced_version+i, "jul");
	else if (!strncmp(VERSION+5, "Aug", 3))
		strcpy(ced_version+i, "aug");
	else if (!strncmp(VERSION+5, "Sep", 3))
		strcpy(ced_version+i, "sep");
	else if (!strncmp(VERSION+5, "Oct", 3))
		strcpy(ced_version+i, "oct");
	else if (!strncmp(VERSION+5, "Nov", 3))
		strcpy(ced_version+i, "nov");
	else if (!strncmp(VERSION+5, "Dec", 3))
		strcpy(ced_version+i, "dec");
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
		ced_version[i] = EOS;
*/
	i = strlen(ced_version);
	ced_version[i++] = VERSION[11];
	ced_version[i++] = VERSION[12];
}

#define myIsAllDigit(c) ((c >= '0' && c <= '9') || c == '-')

static void SetOption(char *text) {
	int  id;
#ifdef _MAC_CODE
	Str255 pFontName;
	char FontName[256];
#endif // _MAC_CODE
#ifdef _WIN32
	wchar_t wDirPathName[FNSize];
#endif // _WIN32
	int height, width, top;

	id = atoi(text);
	while (isdigit(*text)) text++;
	while (*text == ' ' || *text == '\t' || *text == '=') text++;

	if (id == 139) {
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
						WarningWinSize.left = atoi(text);
						WarningWinSize.height = height;
						WarningWinSize.width = width;
						WarningWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 145) {
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
						ProgresWinSize.left = atoi(text);
						ProgresWinSize.height = height;
						ProgresWinSize.width = width;
						ProgresWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 501) {
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
	} else if (id == 502) {
		if (*text != EOS) {
		}
	} else if (id == 503) {
		if (*text != EOS) {
		}
	} else if (id == 506) {
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
						PBCWinSize.left = atoi(text);
						PBCWinSize.height = height;
						PBCWinSize.width = width;
						PBCWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 500) {
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
						for (; *text && myIsAllDigit(*text); text++) ;
						for (; *text && !myIsAllDigit(*text); text++) ;
						if (*text != EOS) {
							MVWinZoom = atoi(text);
							if (MVWinZoom >= 3 || MVWinZoom < -1)
								MVWinZoom = 1;
							for (; *text && myIsAllDigit(*text); text++) ;
							for (; *text && !myIsAllDigit(*text); text++) ;
							if (*text != EOS) {
								MVWinWidth = atoi(text);
							}
						}
					}
				}
			}
		}
	} else if (id == 504) {
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
	} else if (id == 1963) {
		if (*text != EOS) {
			if (strncmp(text, "CURRENT_CLAN_VERSION-", 21) == 0) {
				text = text + 21;
				top = atoi(text);
				if (WebCLANVersion < top || top == 0)
					WebCLANVersion = top;
				text = strchr(text, '-');
				if (text != NULL) {
					text++;
					versionTime = atol(text);
				}
			}
		}
	} else if (id == 1964) {
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
	}  else if (id == 1965) {
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
						ThumbnailWin.left = atoi(text);
						ThumbnailWin.height = height;
						ThumbnailWin.width = width;
						ThumbnailWin.top = top;
					}
				}
			}
		}
	}  else if (id == 1966) {
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
						TextWinSize.left = atoi(text);
						TextWinSize.height = height;
						TextWinSize.width = width;
						TextWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 2000) {
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
						RecallWinSize.left = atoi(text);
						RecallWinSize.height = height;
						RecallWinSize.width = width;
						RecallWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 2001) {
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
						ProgsWinSize.left = atoi(text);
						ProgsWinSize.height = height;
						ProgsWinSize.width = width;
						ProgsWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 2002) {
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
						HlpCmdsWinSize.left = atoi(text);
						HlpCmdsWinSize.height = height;
						HlpCmdsWinSize.width = width;
						HlpCmdsWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 2007) {
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
	} else if (id == 2009) {
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
						WebItemsWinSize.left = atoi(text);
						WebItemsWinSize.height = height;
						WebItemsWinSize.width = width;
						WebItemsWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 2010) {
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
						FolderWinSize.left = atoi(text);
						FolderWinSize.height = height;
						FolderWinSize.width = width;
						FolderWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 505) {
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
						MVHLPWinSize.left = atoi(text);
						MVHLPWinSize.height = height;
						MVHLPWinSize.width = width;
						MVHLPWinSize.top = top;
					}
				}
			}
		}
	} else if (id == 1962) {
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
					PBC.speed = atoi(text);
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
		if (*text != EOS) {
#ifdef _MAC_CODE
			if (!access(text, 0)) {
				strcpy(lib_dir, text);
				if (isRefEQZero(mor_lib_dir))
					strcpy(mor_lib_dir, lib_dir);
			}
#endif // _MAC_CODE
#ifdef _WIN32
			strcpy(lib_dir, text);
			id = strlen(lib_dir) - 1;
			if (lib_dir[id] == PATHDELIMCHR)
				lib_dir[id] = EOS;
			if (my_chdir(lib_dir)) {
				strcpy(lib_dir, home_dir);
				id = strlen(lib_dir) - 1;
				if (lib_dir[id] == PATHDELIMCHR)
					lib_dir[id] = EOS;
				if (my_chdir(lib_dir)) {
					if (GetCurrentDirectory(FNSize, wDirPathName) == 0){
						if (GetWindowsDirectory(wDirPathName, FNSize) == 0)
							*lib_dir = EOS;
						else {
							u_strcpy(lib_dir, wDirPathName, FNSize);
							addFilename2Path(lib_dir, "Clan");
						}
					} else
						u_strcpy(lib_dir, wDirPathName, FNSize);
				}
			}
			strcat(lib_dir, PATHDELIMSTR);
			if (*mor_lib_dir == EOS)
				strcpy(mor_lib_dir, lib_dir);
#endif /* _WIN32 */
		}
	} else if (id == 1971) {
		if (*text != EOS) {
//			doReWrap = (char)atoi(text);
		}
	} else if (id == 1972) {
		for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
		text[top] = EOS;
		strcpy(DisTier, text);
		if (*DisTier == EOS)
			strcpy(DisTier, "%MOR:");
		else
			uS.uppercasestr(DisTier, &dFnt, C_MBF);
	} else if (id == 1973) {
// NOT USED
	} else if (id == 1974) {
		if (*text != EOS) {
#ifdef _MAC_CODE
			if (!access(text, 0))
				strcpy(wd_dir, text);
#endif // _MAC_CODE
#ifdef _WIN32
			strcpy(wd_dir, text);
			id = strlen(wd_dir) - 1;
			if (wd_dir[id] == PATHDELIMCHR && wd_dir[id-1] != ':')
				wd_dir[id] = EOS;
			if (my_chdir(wd_dir))
				strcpy(wd_dir, lib_dir);
			else if (wd_dir[id] != PATHDELIMCHR)
				strcat(wd_dir, PATHDELIMSTR);
#endif /* _WIN32 */
		}
	} else if (id == 1975) {
		if (*text != EOS) {
#ifdef _MAC_CODE
			if (!access(text, 0))
				strcpy(od_dir, text);
#endif // _MAC_CODE
#ifdef _WIN32
			strcpy(od_dir, text);
			if (WD_Not_Eq_OD) {
				id = strlen(od_dir) - 1;
				if (od_dir[id] == PATHDELIMCHR)
					od_dir[id] = EOS;
				if (my_chdir(od_dir))
					strcpy(od_dir, wd_dir);
				else {
					my_chdir(wd_dir);
					strcat(od_dir, PATHDELIMSTR);
				}
			}
#endif /* _WIN32 */
		}
	} else if (id == 1976) {
		if (*text != EOS) {
#ifdef _MAC_CODE
			if (*text == '"') {
				text++;
				for (top=0; *text != '"' && *text != EOS; text++)
					FontName[top++] = *text;
				FontName[top] = EOS;
				GetFontNumber(FontName, &dFnt.fontId);
				if (*text == '"')
					text++;
			} else {
				dFnt.fontId = atoi(text);
				for (; *text && myIsAllDigit(*text); text++) ;
				GetFontName(dFnt.fontId, pFontName);
				p2cstrcpy(FontName, pFontName);
			}
			for (; *text && !myIsAllDigit(*text) && *text != EOS; text++) ;
			dFnt.fontSize = atol(text);
/*
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS)
				dFnt.CharSet = atoi(text);
			else
				dFnt.CharSet = 1;
*/
			dFnt.Encod = my_FontToScript(dFnt.fontId, dFnt.CharSet);
			dFnt.orgEncod = dFnt.Encod;
			dFnt.CharSet = dFnt.Encod;
			dFnt.isUTF = TRUE;
			if (uS.mStricmp(FontName, "ARIAL UNICODE MS") == 0) {
				dFnt.fontType = MACArialUC;
			} else if (uS.mStricmp(FontName, "CAFONT") == 0) {
				dFnt.fontType = MacCAFont;
			} else {
				dFnt.fontType = UNICODEDATA;
			}
			dFnt.orgFType = NOCHANGE;
			C_MBF = FALSE;
			if (FontName[0] != EOS) {
				C_MBF = (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3);
				strcpy(dFnt.fontName, FontName);
			} else {
				strcpy(dFnt.fontName, DEFAULT_FONT);
				dFnt.fontId = DEFAULT_ID;
				dFnt.fontSize = DEFAULT_SIZE;
				dFnt.fontType = getFontType(DEFAULT_FONT, FALSE);
				dFnt.orgFType = dFnt.fontType;
				dFnt.Encod = my_FontToScript(dFnt.fontId, dFnt.CharSet);
				dFnt.orgEncod = dFnt.Encod;
				dFnt.CharSet = dFnt.Encod;
			}
			
			strcpy(defUniFontName, dFnt.fontName);
			defUniFontSize = dFnt.fontSize;
#endif // _MAC_CODE
#ifdef _WIN32
			C_MBF = FALSE;
			dFnt.fontSize = atol(text);
			for (; *text && *text != ' '; text++) ;
			for (; *text && *text == ' '; text++) ;
			if (myIsAllDigit(*text)) {
				dFnt.CharSet = atoi(text);
				for (; *text && *text != ' '; text++) ;
				for (; *text && *text == ' '; text++) ;
			} else
				dFnt.CharSet = DEFAULT_CHARSET;
			strcpy(dFnt.fontName, text);
			dFnt.isUTF = TRUE;
			if (uS.mStricmp(dFnt.fontName, "ARIAL UNICODE MS") == 0) {
				dFnt.fontType = WINArialUC;
			} else if (uS.mStricmp(dFnt.fontName, "CAFONT"/*UNICODEFONT*/) == 0) {
				dFnt.fontType = WINCAFont;
			} else {
				dFnt.fontType = UNICODEDATA;
			}
			uS.remFrontAndBackBlanks(dFnt.fontName);
			dFnt.orgFType = NOCHANGE;
			dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
			C_MBF = (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3);

			strcpy(defUniFontName, dFnt.fontName);
			defUniFontSize = dFnt.fontSize;

			if (!strcmp(m_lfDefFont.lfFaceName, dFnt.fontName)) {
				extern long w95_fontSize;
				m_lfDefFont.lfHeight = dFnt.fontSize;
				w95_fontSize = 0L;
			}
#endif /* _WIN32 */
		}
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
	} else if (id == 1981) {
		if (*text != EOS) {
			isUpdateCLAN = (char)atoi(text);
		}
	} else if (id == 1982) {
		if (*text != EOS) {
			ClanAutoWrap = (char)atoi(text);
		}
	} else if (id == 1983) {
		if (*text != EOS && strncmp(text, "FILE_FILTER-", 12) == 0) {
//			isAllFile = (char)atoi(text+12);
		}
	} else if (id == 1984) {
		if (*text != EOS) {
			if (strncmp(text, "SPEED-", 6) == 0) {
				text = text + 6;
				streamSpeedNumber = atoi(text);
				if (streamSpeedNumber < 0 || streamSpeedNumber > 5)
					streamSpeedNumber = 0;
			}
		}
	} else if (id == 1985) {
// NOT USED
	} else if (id == 1986) {
		if (*text != EOS) {
			ClanWinRowLim = atol(text);
			if (ClanWinRowLim > 0L && ClanWinRowLim < 10L)
				ClanWinRowLim = 10L;
		}
	} else if (id == 1987) {
		if (*text != EOS) {
			SearchWrap = ((*text == 'y') ? TRUE : FALSE);
		}
	} else if (id == 1988) {
		if (*text != EOS) {
			AutoScriptSelect = (char)atoi(text);
		}
	} else if (id == 1989) {
		if (*text != EOS) {
			isCursorPosRestore = (char)atoi(text);
		}
	} else if (id == 2003) {
		if (*text != EOS) {
//			rawTextInput = (char)atoi(text);
// NOT USED
		}
	} else if (id == 2004) {
		if (*text != EOS) {
			isUnixCRs = (char)atoi(text);
		}
	} else if (id == 2005) {
// NOT USED
	} else if (id == 2006) {
		if (*text != EOS)
			F5Option = atoi(text);
	} else if (id == 2008) {
		if (*text != EOS) {
//			isUseSPCKeyShortcuts = (char)atoi(text);
// NOT USED
		}
	} else if (id == 2011) {
		if (*text != EOS) {
			for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
			text[top] = EOS;
			set_folders(1, text);
		}
	} else if (id == 2012) {
		if (*text != EOS) {
			for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
			text[top] = EOS;
			set_folders(2, text);
		}
	} else if (id == 2013) {
		if (*text != EOS) {
			for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
			text[top] = EOS;
			set_folders(3, text);
		}
#ifdef _MAC_CODE
	} else if (id == 2014) {
		if (*text != EOS) {
			for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
			text[top] = EOS;
			addToFavFont(text, FALSE);
		}
#endif
	} else if (id == 2015) {
		if (*text != EOS) {
			ThumbnailsHight = atoi(text);
			if (ThumbnailsHight < 30.0000) {
				ThumbnailsHight = 30.0000;
			}
		}
	} else if (id == 1990) {
		if (*text != EOS) {
			if (!isdigit(*text)) {
				rptMark = *text;
				rptPTime = atoi(text+1);
			} else
				rptPTime = atoi(text);
			if (rptPTime < 0)
				rptPTime = 0;
		}
	} else if (id == 1991) {
		if (*text != EOS) {
			TabSize = atoi(text);
			if (TabSize <= 0 || TabSize > 40)
				TabSize = 8;
		}
	} else if (id == 1992) {
		if (*text != EOS) {
// NOT USED
		}
	} else if (id == 1993) {
		for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
		text[top] = EOS;
		for (top=0; isSpace(text[top]); top++) ;
		strcpy(URL_Address, text+top);
		uS.remblanks(URL_Address);
	} else if (id == 1961) {
		for (top=0; text[top] != '\n' && text[top] != EOS; top++) ;
		text[top] = EOS;
		for (top=0; isSpace(text[top]); top++) ;
		strcpy(proxyAddPort, text+top);
		uS.remblanks(proxyAddPort);
	} else if (id == 1994) {
		if (*text != EOS) {
#ifdef _MAC_CODE
			if (!access(text, 0))
				strcpy(mor_lib_dir, text);
#endif // _MAC_CODE
#ifdef _WIN32
			strcpy(mor_lib_dir, text);
			id = strlen(mor_lib_dir) - 1;
			if (mor_lib_dir[id] == PATHDELIMCHR)
				mor_lib_dir[id] = EOS;
			if (my_chdir(mor_lib_dir)) {
				strcpy(mor_lib_dir, home_dir);
				id = strlen(mor_lib_dir) - 1;
				if (mor_lib_dir[id] == PATHDELIMCHR)
					mor_lib_dir[id] = EOS;
				if (my_chdir(mor_lib_dir)) {
					if (GetCurrentDirectory(FNSize, wDirPathName) == 0){
						if (GetWindowsDirectory(wDirPathName, FNSize) == 0)
							*mor_lib_dir = EOS;
						else {
							u_strcpy(mor_lib_dir, wDirPathName, FNSize);
							addFilename2Path(mor_lib_dir, "Clan");
						}
					} else
						u_strcpy(mor_lib_dir, wDirPathName, FNSize);
				}
			}
			strcat(mor_lib_dir, PATHDELIMSTR);
#endif /* _WIN32 */
		}
	} else if (id == 1995) {
		if (*text != EOS) {
			doMixedSTWave = (char)atoi(text);
		}
	} else if (id == 1996) {
		if (*text != EOS) {
			LineNumberDigitSize = atol(text);
			for (; *text && myIsAllDigit(*text); text++) ;
			for (; *text && !myIsAllDigit(*text); text++) ;
			if (*text != EOS)
				LineNumberingType = (char)atoi(text);
		}
	} else if (id == 1997) {
		for (top=0; text[top] != ' ' && text[top] != EOS; top++) ;
		if (text[top] == ' ') {
			text[top] = EOS;
			width = atoi(text);
			top++;
			if (text[strlen(text)-1] == '\n')
				text[strlen(text)-1] = EOS;
			AllocConstString(text+top, width);
		}
	} else if (id == 1998) {
		if (*text != EOS)
			isChatLineNums = atoi(text);
	} else if (id == 1999) {
		if (*text != EOS)
			sendMessageTargetApp = atoi(text);
	}
}

static int ReadPreference(const char *fname, int argc, FNType *path) {
	int   len;
	char  text[257];
	FILE  *fp;

	len = strlen(path);
	addFilename2Path(path, fname);
	if ((fp=fopen(path,"r")) == NULL) {
		path[len] = 0;
		return(argc);
	}
	path[len] = 0;

	fgets_cr_lc = '\0';
	while (fgets_cr(text, 256, fp)) {
		if (uS.isUTF8(text) || uS.partcmp(text, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remFrontAndBackBlanks(text);
		if (*text == '[') ;
		else if (isdigit(*text)) {
			SetOption(text);
		} else {
			if (*text != EOS) {
				for (len=0; ced_lineC[len] != EOS || ced_lineC[len+1] != EOS; len++) ;
				len++;
				strcpy(ced_lineC+len, text);
				len = len + strlen(ced_lineC+len);
				ced_lineC[len+1] = EOS;
				argc++;
			}
		}
	}
	fclose(fp);
	return(argc);
}

static void WriteCedPrefs(FILE *fp) {
	int i;
#ifdef _MAC_CODE
	Str255 pFontName;
	char FontName[256];
#endif // _MAC_CODE

	fprintf(fp, "[ced]\n");

	fprintf(fp, "%d=%d %d %d %d\n", 139, WarningWinSize.height, WarningWinSize.width, WarningWinSize.top, WarningWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 145, ProgresWinSize.height, ProgresWinSize.width, ProgresWinSize.top, ProgresWinSize.left);
	fprintf(fp, "%d=%d %d %d %d %d %d\n", 500, MovieWinSize.height,MovieWinSize.width,MovieWinSize.top,MovieWinSize.left,MVWinZoom,MVWinWidth);
	fprintf(fp, "%d=%d %d %d %d\n", 504, PictWinSize.height, PictWinSize.width, PictWinSize.top, PictWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 501, ClanWinSize.height, ClanWinSize.width, ClanWinSize.top, ClanWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 506, PBCWinSize.height, PBCWinSize.width, PBCWinSize.top, PBCWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 505, MVHLPWinSize.height, MVHLPWinSize.width, MVHLPWinSize.top, MVHLPWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 1962,defWinSize.height, defWinSize.width, defWinSize.top, defWinSize.left);
	fprintf(fp, "%d=CURRENT_CLAN_VERSION-%d-%lu\n", 1963, WebCLANVersion, versionTime);
	fprintf(fp, "%d=%d %d %d %d\n", 1964,ClanOutSize.height, ClanOutSize.width, ClanOutSize.top, ClanOutSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 1965,ThumbnailWin.height, ThumbnailWin.width, ThumbnailWin.top, ThumbnailWin.left);
	fprintf(fp, "%d=%d %d %d %d\n", 1966,TextWinSize.height, TextWinSize.width, TextWinSize.top, TextWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2000,RecallWinSize.height, RecallWinSize.width, RecallWinSize.top, RecallWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2001,ProgsWinSize.height, ProgsWinSize.width, ProgsWinSize.top, ProgsWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2002,HlpCmdsWinSize.height, HlpCmdsWinSize.width, HlpCmdsWinSize.top, HlpCmdsWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2007,SpCharsWinSize.height, SpCharsWinSize.width, SpCharsWinSize.top, SpCharsWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2009,WebItemsWinSize.height, WebItemsWinSize.width, WebItemsWinSize.top, WebItemsWinSize.left);
	fprintf(fp, "%d=%d %d %d %d\n", 2010,FolderWinSize.height, FolderWinSize.width, FolderWinSize.top, FolderWinSize.left);

	fprintf(fp, "%d=%d %ld %d %ld %d %ld\n", 1969, PBC.LoopCnt, PBC.backspace, PBC.speed, PBC.step_length, (int)PBC.enable, PBC.pause_len);
	fprintf(fp, "%d=%d\n", 1971, (int)doReWrap);
#ifdef _MAC_CODE
	GetFontName(dFnt.fontId, pFontName);
	p2cstrcpy(FontName, pFontName);
	fprintf(fp, "%d=\"%s\" %ld %d\n", 1976, FontName, dFnt.fontSize, (int)dFnt.CharSet);
#endif // _MAC_CODE
#ifdef _WIN32
	fprintf(fp, "%d=%ld %d %s\n", 1976, dFnt.fontSize, (int)dFnt.CharSet, dFnt.fontName);
#endif /* _WIN32 */
	fprintf(fp, "%d=%s\n", 1970, lib_dir);
	fprintf(fp, "%d=%s\n", 1974, wd_dir);
	fprintf(fp, "%d=%s\n", 1975, od_dir);
	fprintf(fp, "%d=%s\n", 1994, mor_lib_dir);
	fprintf(fp, "%d=%s\n", 1972, DisTier);
	fprintf(fp, "%d=%d\n", 1978, DefClan);
	fprintf(fp, "%d=%d\n", 1979, DefWindowDims);
	fprintf(fp, "%d=%d\n", 1981, isUpdateCLAN);
	fprintf(fp, "%d=%d\n", 1982, ClanAutoWrap);
	fprintf(fp, "%d=FILE_FILTER-%d\n", 1983, isAllFile);
	fprintf(fp, "%d=SPEED-%d\n", 1984, streamSpeedNumber);
// NOT USED		fprintf(fp, "%d=%s\n", 1985, );
	fprintf(fp, "%d=%ld\n", 1986, ClanWinRowLim);
	fprintf(fp, "%d=%c\n", 1987, ((SearchWrap == TRUE) ? 'y' : 'n'));
	fprintf(fp, "%d=%d\n", 1988, AutoScriptSelect);
	fprintf(fp, "%d=%d\n", 1989, isCursorPosRestore);
	if (rptMark == 0)
		fprintf(fp, "%d=%d\n", 1990, rptPTime);
	else
		fprintf(fp, "%d=%c%d\n", 1990, rptMark, rptPTime);
	fprintf(fp, "%d=%d\n", 1991, TabSize);
// NOT USED 	fprintf(fp, "%d=%d\n", 1992, isUseURL);
	fprintf(fp, "%d=%s\n", 1993, URL_Address);
	fprintf(fp, "%d=%s\n", 1961, proxyAddPort);
	fprintf(fp, "%d=%d\n", 1995, doMixedSTWave);
	fprintf(fp, "%d=%ld %d\n", 1996, LineNumberDigitSize, (int)LineNumberingType);
	for (i=0; i < 10; i++) {
		if (ConstString[i] != NULL)
			fprintf(fp, "%d=%d %s\n", 1997, i, ConstString[i]);
	}
	fprintf(fp, "%d=%d\n", 1998, isChatLineNums);
	fprintf(fp, "%d=%d\n", 1999, sendMessageTargetApp);
// NOT USED 	fprintf(fp, "%d=%d\n", 2003, rawTextInput);
	fprintf(fp, "%d=%d\n", 2004, isUnixCRs);
// NOT USED 	fprintf(fp, "%d=%d\n", 2005, isUTFData);
	fprintf(fp, "%d=%d\n", 2006, F5Option);
// NOT USED 	fprintf(fp, "%d=%d\n", 2008, isUseSPCKeyShortcuts);
	fprintf(fp, "%d=%lf\n", 2015, ThumbnailsHight);

	fprintf(fp, "+b%d\n", FreqCountLimit);
	if (*CodesFName != EOS)
		fprintf(fp, "+c%s\n", CodesFName);
	if (*KeysFName != EOS)
		fprintf(fp, "+k%s\n", KeysFName);
	if (MakeBackupFile == FALSE)
		fprintf(fp, "-d\n");
	else
		fprintf(fp, "+d\n");
	if (!StartInEditorMode)
		fprintf(fp, "-e\n");
	if (ShowPercentOfFile)
		fprintf(fp, "-p\n");
	if (DefAutoWrap)
		fprintf(fp, "-a\n");
	if (DefChatMode < 2) {
		if (DefChatMode)
			fprintf(fp, "+w\n");
		else
			fprintf(fp, "-w\n");
	}
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
#ifdef _MAC_CODE
		write_FavFonts_2014(fp);
#endif
		fprintf(fp, "%d=%d\n", 1980, lastCommand);
		isUnixCRs = tIsUnixCRs;
	}
	fclose(fp);
}

void resetOptions(void) {
	int  ans, t;
	char message[128]; 
	FNType new_name[FNSize];

	t = strlen(prefsDir);
	if (t == 0)
		return;
	strcpy(message, "Are you sure you want to reset all options?");
	ans = QueryDialog(message, 147);
	if (ans > 0) {
		isSavePrefs = FALSE;
		addFilename2Path(prefsDir, CED_PREF_FILE);
		strcpy(new_name, prefsDir);
		strcat(new_name, ".bck");
		unlink(new_name);
		rename(prefsDir, new_name);
		prefsDir[t] = EOS;
		WriteCedPreference();
		addFilename2Path(prefsDir, CLAN_PREF_FILE);
		strcpy(new_name, prefsDir);
		strcat(new_name, ".bck");
		unlink(new_name);
		rename(prefsDir, new_name);
		prefsDir[t] = EOS;
		WriteClanPreference();
		do_warning("Please quit CLAN completely when ready and re-start CLAN again.", -1);
	}
	prefsDir[t] = EOS;
}

#ifdef _WIN32
extern void freeArgs(void);
void freeArgs(void) {
	int i;

	for (i=0; i < argc; i++)
		free(argv[i]);
	free(argv);
	free_commands();
}
#endif

#ifdef _MAC_CODE
#undef main

#include <wchar.h>

/*
static URLSystemEventUPP	myDownloadProcUPP;

static pascal OSStatus MyDownloadCallBackFunction(void *userContext, EventRecord *event) {
	OSStatus err;

	err = InvokeURLSystemEventUPP(userContext, event, myDownloadProcUPP);
	return(noErr);
}
*/
static void	checkWebVersion(void) {
	int			vn;
	char		*s, res;
	time_t		timer;
//	Handle		DH;
	FNType		gFile[FNSize];
	FSRef		tRef;
//	FSSpec		fss;
//	FILE		*fp;
	OSStatus	err;
//	URLOpenFlags		openFlags;
	extern Boolean isHaveURLAccess;

	time(&timer);
	if (versionTime > timer)
		return;
	if (!isUpdateCLAN)
		return;
	if (!isHaveURLAccess)
		return;
// strcpy(templineC, ctime(&timer));
	versionTime = timer + 345600L; // this number = 4 days time
// strcpy(templineC, ctime((time_t *)&versionTime));
	strcpy(templineC2, "http://childes.talkbank.org/clan/CLAN_VERSION.txt");
/*
	DH  = NewHandle(0);
	if (MemError()) {
		WriteCedPreference();
		return;
	}
	openFlags = kURLDisplayProgressFlag | kURLReplaceExistingFlag;
	err = URLSimpleDownload(templineC, NULL, DH, openFlags, NULL, 0);
	templineC[0] = EOS;
	if (err == noErr) {
		HLock(DH);
		vn = GetHandleSize(DH);
		if (vn > UTTLINELEN)
			vn = UTTLINELEN;
		strncpy(templineC, *DH, UTTLINELEN);
		templineC[vn] = EOS;
		HUnlock(DH);
	}
	DisposeHandle(DH);
*/
	DownloadURL(templineC2, 0L, templineC, UTTLINELEN, NULL, FALSE, FALSE);
	if (templineC[0] == EOS) {
		WriteCedPreference();
		return;
	}
	vn = atoi(templineC);
	if (vn == 0) {
		WriteCedPreference();
		return;
	}
	for (s=templineC; *s != EOS && *s != '\n' && *s != '\r'; s++) ;
	while (*s == '\n' || *s == '\r')
		s++;
	uS.remFrontAndBackBlanks(s);
	if (WebCLANVersion < vn) {
		res = UpdateCLANDialog(s);
		if (res == 'n') {
			WebCLANVersion = vn;
		} else if (res == 'd') {
			isUpdateCLAN = FALSE;
		} else if (res == 'y') {
			err = FSFindFolder(kOnSystemDisk,kTemporaryFolderType,kDontCreateFolder,&tRef);
			if (err != noErr) {
				err = FSFindFolder(kUserDomain,kTemporaryFolderType,kDontCreateFolder,&tRef);
				if (err != noErr) {
					strcpy(gFile, webDownLoadDir);
					addFilename2Path(gFile, "clan.dmg");
//					if ((fp=fopen(gFile, "w")) != NULL) fclose(fp);
//					my_FSPathMakeRef(gFile, &tRef);
				}
			}
			if (err == noErr) {
				my_FSRefMakePath(&tRef, gFile, FNSize);
				addFilename2Path(gFile, "clan.dmg");
//				if ((fp=fopen(gFile, "w")) != NULL) fclose(fp);
//				my_FSPathMakeRef(gFile, &tRef); 
			}
			strcpy(templineC, "http://childes.talkbank.org/clan/clan.dmg");
			DownloadURL(templineC, 3000000L, NULL, 0, gFile, TRUE, FALSE);
		}
	}
	WriteCedPreference();
}

int main() {
	int i;
	long buf[5], format[5];
	extern char	  isCloseProgressDialog;
	extern SInt16 saveCurrentKeyScript;

	isCloseProgressDialog = FALSE;
	SelectedFiles = NULL;
	MacintoshInit();
	LocalInit();
	mScript = FontScript();
	saveCurrentKeyScript = mScript;
	InstallAEProcs();

i = sizeof(wchar_t);
format[0] = '%';
format[1] = 'd';
format[2] = 0;
swprintf((wchar_t *)buf, 4, (wchar_t *)format, 1);
format[0] = '1';
if (buf[0] != format[0]) {
	do_warning("FATAL ERROR with vswprintf wchar converter!!!!", 0);
	RemoveAEProcs();
	SelectedFiles = freeSelectedFiles(SelectedFiles);
	ExitToShell();
}

/*
#if __option(ushort_wchar_t)
#error "ushort_wchar_t defined"
#else
#error "NO ushort_wchar_t defined"
#endif
*/
// -fwritable-strings
// -fwide-exec-charset=UTF-16
// -fpack-struct=n
/*
wchar_t test[256];
ANSIToUnicode((unsigned char *)"AbCdEfG", strlen("AbCdEfG"), test, NULL, 256, mScript);
int i1 = sizeof(wchar_t);
int i2 = sizeof(test[0]);
int i3 = sizeof(UniChar);
// int i4 = sizeof(_MSL_WCHAR_T_TYPE);

int len = strlen(test);

wchar_t c = to_unCH_lower(test[0]);

test[0] = '1';
test[1] = '9';
test[2] = '6';
test[3] = '2';
test[4] = 0;

int  resI = atoi(test);
long resL = atol(test);
*/


	for (i=1; i < argc; i++) {
		if (*argv[i] == '-' || *argv[i] == '+')
			ced_getflag(argv[i]);
	}
//	if (ArialUnicodeFOND == 0 && SecondDefUniFOND == 0)
//		do_warning("Can't find either \"Arial Unicode MS\" or \"TITUS Cyberbit Basic\" font. Please look on web page \"http://childes.talkbank.org/clan/\" for more information.", 0);
	checkWebVersion();
	mac_call();
	KeyScript(mScript);
	if (isMovieAvialable)
		ExitMovies();

	RemoveAEProcs();
	SelectedFiles = freeSelectedFiles(SelectedFiles);
	ExitToShell();
	return(0);
}
#endif //_MAC_CODE
