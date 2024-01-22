#ifndef _C_CED_
#define _C_CED_

#include "common.h"

#if defined(_MAC_CODE)
	#include <Movies.h>

	#define	DEFAULT_FONT	defFontName()
	#define	DEFAULT_ID		defFontID()
	#define	DEFAULT_SIZE	defFontSize()
	#define	DEFAULT_FACE	0
	#define FLINESPACE		1

	#define CONT_CHAR		'\310'
	#define SOFT_CR_CHAR	'\246'
	#define BULLET_CHAR		'\245'

	#define SetFontName(x, y) x = y;
	#define isFontEqual(x, y) (x == y)

	extern FNType CLAN_Programs_str[];
	extern FNType WEB_Dirs_str[];
	extern FNType Special_Characters_str[];
	extern FNType Commands_str[];
	extern FNType Commands_Shortcuts_str[];
	extern FNType CLAN_Output_str[];
	extern FNType Walker_Controller_str[];
	extern FNType Movie_Help_str[];
	extern FNType Movie_sound_str[];

#elif defined(_WIN32)
//  #include <afxwin.h>
	#include "stdafx.h"
	#include "Clan2.h"

	typedef long			Size;
//	typedef void			*WindowPtr;
//	typedef CRect			Rect;
//	typedef BOOL			Boolean;

	#define	DEFAULT_FONT	defFontName()
	#define	DEFAULT_SIZE	defFontSize()
	#define	DEFAULT_ID		0
	#define FLINESPACE		0

	#define CONT_CHAR		'\\'
	#define SOFT_CR_CHAR	'\266'                    //'\241'
	#define BULLET_CHAR		(win->Font.CharSet == 128 ? '\337' : '\225')
	#define SetFontName(x, y) strcpy(x, y);
	#define isFontEqual(x, y) (strcmp(x, y) == 0)
#else
	#define FLINESPACE		1
#endif /* _WIN32 */

extern "C"
{

#include "c_curses.h"

#define CHECK_IN_TIERS struct check_in_tiers
#define TCODES struct templates
#define TPLATES struct templates
#define ERRLIST struct errlst
#define BE_TIERS struct templates
#define SPLIST struct speakers

#ifndef EOS			/* End Of String marker			 */
	#define EOS 0 // '\0'
#endif
#define	I_VScroll_bar 	11
#define	I_HScroll_bar 	12

#define SNDBUFSIZE 10240 /* 8192 */

#define SCREENWIDTH 78L
#define UNDOLISTNUM 70 /* this number should NEVER be less then 6 */

#define isShowLineNums (isChatLineNums && uS.FNTypecmp(global_df->fileName, "CLAN Output", 0L) && (global_df->ChatMode || LineNumberingType == 0))

#define SYSTEM_7		0x0700
#define	left_key		((theWorld.isOldMac) ? 0x46 : 0x7B)
#define	right_key		((theWorld.isOldMac) ? 0x42 : 0x7C)
#define	up_key			((theWorld.isOldMac) ? 0x4D : 0x7E)
#define	down_key		((theWorld.isOldMac) ? 0x48 : 0x7D)
#define	RETURN_CHAR		'\r'
#define	ENTER_CHAR		3
#define ESC_CHAR		0x1B
#define	DELETE_CHAR		0x7F
#define BACKSPACE_CHAR	0x08

#define PRAAT			'\001'
#define PITCHWORKS		'\002'

#define EVERY_LINE		'\001'
#define EVERY_TIER		'\002'

#define LASTKEY '\0'
#define MOVEKEY '\001'
#define INSTKEY '\002'
#define DELTKEY '\003'
#define REPLKEY '\004'
#define INSTRPT '\005'
#define MOVERPT '\006'
#define DELTRPT '\007'

#define HUGE_CHAR			0x7F
#define HUGE_UNSIGNED_CHAR	0xFF
#define HUGE_INTEGER		0x7FFF
#define HUGE_UNSIGNED		0xFFFF
#define HUGE_LONG			0x7FFFFFFFL
#define HUGE_UNSIGNED_LONG	0xFFFFFFFFL
#define HUGE_NEG_INTEGER	((short)0xFFFF)

#define PCCrType			1
#define MacCrType			2

#define	OLD_CED_PREF_FILE	"CED.Prefs"
#define	OLD_CLAN_PREF_FILE	"CLAN.Prefs"
#define	CED_PREF_FILE		"CED.UPrefs"
#define	CLAN_PREF_FILE		"CLAN.UPrefs"

#define CONSOLE_ID 129
#define CODES_FILE "codes.cut"
#define KEYS_BIND_FILE "cedkeys.cut"
#define NEWFILENAME "newfile.cha"
#define DASHES  "" /* add - infront like in BEEP */
#define BEEP    "---- BEEP! ---"
#define ESC_KEY "" /* add - infront like in BEEP */
#define CTRL_X  "" /* add - infront like in BEEP */

#define TRUE_VIS_ID(n)		(n = (char)(n | (char)0x4))
#define CMP_VIS_ID(n)		(n & (char)0x4)
#define FALSE_VIS_ID(n)		(n = (char)(n & (char)0xfb))
//#define TRUE_CHECK_ID1(n)	(n = (char)(n | (char)0x1))
//#define CMP_CHECK_ID1(n)	(n & (char)0x1)
//#define FALSE_CHECK_ID1(n)	(n = (char)(n & (char)0xfe))
//#define TRUE_CHECK_ID2(n)	(n = (char)(n | (char)0x2))
//#define CMP_CHECK_ID2(n)	(n & (char)0x2)
//#define FALSE_CHECK_ID2(n)	(n = (char)(n & (char)0xfd))

#define boxWidth(box) ((box).right - (box).left)
#define boxHeight(box) ((box).bottom - (box).top)
#if (TARGET_API_MAC_CARBON == 1)

	enum {
		kMaxUnicodeInputStringLength = 1024
	};

	typedef struct {
		EventRecord inEvent;
		int numDel;
		unsigned long len;
		unCH unicodeInputString[kMaxUnicodeInputStringLength+10];
	} UnicodeInput;

	extern UnicodeInput UniInputBuf;

	extern short windWidth(WindowPtr wind);
	extern short windHeight(WindowPtr wind);
	extern char myNavGetFile(const char *prompt, short numTypes, OSType typeList[], NavObjectFilterProcPtr FilterProc, FNType *file_path);
	extern char myNavPutFile(FNType *fname, const char *prompt, OSType typeList, Boolean *isReplace);
	extern pascal void myNavEventProc(NavEventCallbackMessage callBackSelector,
							NavCBRecPtr callBackParms, NavCallBackUserData callBackUD);
	#define GetCWindowPort GetWindowPort
	#define GetCDialogPort GetDialogPort
#else // (TARGET_API_MAC_CARBON == 1)
	#define windWidth(wind) boxWidth(((GrafPtr)(wind))->portRect)
	#define windHeight(wind) boxHeight(((GrafPtr)(wind))->portRect)

	#define GetControlHilite(ctrl) ((**ctrl).contrlHilite)
	#define GetWindowFromPort(window) ((GrafPtr)window)
	#define GetWindowKind(window) (((WindowPeek)window)->windowKind)
	#define GetCWindowPort(window) ((CGrafPtr)window)
	#undef GetWindowPort
	#define GetWindowPort(window) ((GrafPtr)window)
	#define GetCDialogPort(dialog) ((CGrafPtr)dialog)
	#undef GetDialogPort
	#define GetDialogPort(dialog) ((GrafPtr)dialog)
#endif // (TARGET_API_MAC_CARBON == 1)
	extern char *defFontName(void);
	extern short defFontID(void);
  #if defined(_MAC_CODE)
	extern short defFontSize(void);
  #else
	extern long  defFontSize(void);
  #endif

#define MID_WIN_SIZE 1
#define COM_WIN_SIZE (global_df->total_num_rows - global_df->CodeWinStart)

enum {
	appleID = 1,
	File_Menu_ID,
	Edit_Menu_ID,
	Font_Menu_ID,
	Size_Menu_ID,
	Tier_Menu_ID,
	Mode_Menu_ID,
	Windows_Menu_ID,
	MOR_Grammar_ID,
//	KIDEVAL_DB_ID
} ;

enum { /* MOR_Graamar */
	English_Item = 1,
	Cantonese_Item,
	Chinese_Item,
	Danish_Item,
	Dutch_Item,
	French_Item,
	German_Item,
	Hebrew_Item,
	Italian_Item,
	Japanese_Item,
	Spanish_Item
};

enum { /* MOR_Graamar, KIDEVAL_DB menu */
	English_fp_Item = 1,
	Chinese_md_Item
};


enum { /* file menu */
	New_Item = 1,
	Open_Item,
	MOR_Grammar_item,
//	KIDEVAL_DB_item,
	Select_Media,
	Close_Item,
	Blank1_Item,
	Save_item,
	Save_As_item,
	Save_Clip_As_item,
	Blank2_Item,
	Print_Setup_Item,
	Print_Item,
	Print_Select_Item,
	Blank3_Item,
	Quit_Item
};

 /* size/style menu */
enum {
	Font_Size_End = 10,
	Font_Blank_1,
	Font_Smaller_Size,
	Font_Larger_Size,
	Font_Tab_Size,
	Font_Plain,
	Font_Underline,
	Font_Italic,
	Font_Bold,
	Font_Color_Keywords,
	Font_Color_Red // 2019-04-17 ESC-C - color Red
// 18-07-2008	Font_Color_Words
};

 /* font menu */
#define Font_Auto_Script	1
#define Font_Fav_Start		2
#define Font_Fav_End		5
#define Font_Blank_3		6
#define Font_Name_Start		7

enum { /* edit menu */
	undo_id = 1,
	redo_id,
	line1_id,
	cut_id,
	copy_id,
	paste_id,
	selectall_id,
	line2_id,
	find_id,
	enterselct_id,
	findsame_id,
	replace_id,
	goto_id,
	line3_id,
	toupper_id,
	tolower_id,
	line4_id,
	clanoptions_id,
	setdefsndana_id,
	setF5option_id,
	setlinenumsize_id,
//	setURLaddress_id,
	setThumbnails_id,
//	setStreamingSpeed_id,
	resetAllOptions_id,
	LASTEDIT_ID
} ;

enum {
	HideCtrl = 1,
	ShowCtrl,
	HiliteCtrl
} ;

struct DefWin {
	long height, width, top, left;
} ;

struct MPSR1010 {
    short top, left, height, width;
} ;

#define TIERS struct tier_s
struct tier_s {			/* specifies which tier should be	 */
    char include;		/* included or exclude from the analyses */
    unCH *tcode;		/* the code descriptor			 */
    TIERS *nexttier;	/* points to the next tier		 */
} ;

#define LINE struct line_s
struct line_s {
	unCH c;
	AttTYPE att;
    LINE *next_char, *prev_char;
} ;

#define ROWS struct text_s
struct text_s {
	unCH *line;
	AttTYPE *att;
	char flag;
    FONTINFO Font;
    ROWS *next_row, *prev_row;
} ;

#define CODES struct codes_s
struct codes_s {
    unCH *code;
    CODES *subcodes;
    CODES *NextCode;
} ;

#define UNDO struct undo_s
struct undo_s {
    CODES *FirstCode;
    CODES *CurCodeArr;
    CODES **StartCodeArr;
    long  top_win_pos;
    int   CursorCodeArr;
    int   EditorMode;
    long  LeftCol;
    long  col_win;
    long  col_chr;
    long  row_win;
    long  col_win2;
    long  col_chr2;
    long  row_win2;
    char  ShowParags;
    char  key;
    unCH  *str;
    long  lineno;
    UNDO  *NextUndo;
    UNDO  *PrevUndo;
} ;

#define pctInfo struct PICTINFO
struct PICTINFO {
	FNType pictFName[FNSize];
	char pictChanged;
} ;

#define txtInfo struct TEXTINFO
struct TEXTINFO {
	FNType textFName[FNSize];
	char textChanged;
} ;

#define movInfo struct MOVIEINFO
struct MOVIEINFO {
	unCH MovieFile[FILENAME_MAX];
	FNType rMovieFile[FNSize];
	long  MBeg, MEnd;
	movInfo *nMovie;
} ;

enum {
	isAudio = 1,
	isVideo,
	isPict,
	isText,
	isAllType,
	isPictText
} ;

#define sndInfo struct SOUNDINFO
struct SOUNDINFO {
	int   SSC_TOP;
	int   SEC_TOP;
	int   SSC_MID;
	int   SEC_MID;
	int   SSC_BOT;
	int   SEC_BOT;
	int   SNDWccol;
	int   SNDWHalfRow;
	int   SNDWprint_row1;
	int   SNDWprint_row2;
	long  BegF, EndF;
	long  dBegF, dEndF;
	long  contPlayBeg, contPlayEnd;
	long  SoundFileSize;
	long  WBegF, WEndF, WBegFM, WEndFM;
	long  AIFFOffset;
	long  SNDformat;
	short SNDsample, SNDchan;
	double SNDrate;
	unCH  SoundFile[FILENAME_MAX];
	FNType rSoundFile[FNSize];
	char  SWaveBuf[SNDBUFSIZE];
	char  SSource;
	char  dtype;
	char  IsSoundOn;
	const char *errMess;
	char  SDrawCur;
	char  isNaturalPause;
	char  DDsound;
	char  isMP3;
	FILE *SoundFPtr;
#ifdef _WIN32
	unsigned long SFileType;
#endif
#ifdef _MAC_CODE
	OSType SFileType;
	struct S_MP3 {
		Media  theSoundMedia;
		Handle hSys7SoundData;
		double ts;
	} mp3;
#endif
} ;

#define WindowInfo struct WindowsInfo
WindowInfo {
	long y; // top
	long x; // left
	long height;
	long width;
	long topLChr;
	long skipTopLChr;
	long pos1Chr;
	long skipPos1Chr;
	long pos2Chr;
	long skipPos2Chr;
} ;


#define webMFile struct WEBMEDIAFILES
struct WEBMEDIAFILES {
	FNType *name;
	webMFile *nextFile;
} ;

typedef struct {
	FNType fileName[FNSize];
	ROWS *head_text, *tail_text, *top_win, *row_txt, *cur_line, *curRowLimit;
	LINE *head_row, *tail_row, *col_txt;
	UNDO *UndoList;
	WINDOW *w1, *w2, *wm, *SoundWin;
	WINDOW *RootWindow, *RdW;
	unCH mediaFileName[FILENAME_MAX];
	long  row_win, row_win2, col_win, col_win2, col_chr, col_chr2, head_row_len;
	long  fake_row_win, fake_row_win2, fake_col_win, fake_col_win2;
	long  LeftCol;
	long  cursorCol;
	long  lineno;
	long  window_rows_offset;
	long  numberOfRows;
	long  RowLimit;
	long  Ltik;
	long  KBSize, KBIndex;
	unCH  *KillBuff;
	char  checkMessCnt;
	char  isUTF;
	char  isOutputScrolledOff;
	char  isErrorFindPict;
	char  dontAskDontTell;
	char  NoCodes;
	char  WinChange;
	char  DataChanged;
	char  redisplay;
	char  TSelectFlag;
	char  fake_TSelectFlag;
	char  isExtend;
	char  isSpelling;
	char  ChatMode;
	char  AutoWrap;
	char  ShowParags;
	char  ScrollBar;
	char  VScrollBar;
	char  HScrollBar;
	char  DrawCur;
	char  SSelectFlag;
	char  crType;
	char  err_message[ERRMESSAGELEN];
	int   EditorMode;
	int   UndoCounter;
	int   FreqCount;
	int   TextWinSize;
	int   EdWinSize;
	int   CodeWinStart;
	int   num_of_cols;
	int   total_num_rows;
	int   LastCommand;
	int   CursorCodeArr;
	int   *TopP1;
	int   *BotP1;
	int   *TopP2;
	int   *BotP2;
	int   mainVScale;
	int   CurSpLen;
	long  SLtik;
	unCH  OldCode[SPEAKERLEN];
	unCH  *SpeakerNames[10];
	unCH  VideosNameExts[10][128];
	int   VideosNameIndex;
	FNType  *cod_fname;
	char  rightspeaker;
	char  tcs;	/* 1- if all non-specified speaker tier should be selected*/
	char  tch;	/* 1- if all non-specified header tier should be selected */
	char  tct;	/* 1- if all non-specified code tier should be selected   */
	char  SpecialTextFile;
	char  isTempFile;
	char  CodeLegalPos;
	char  AtEndFound;
	char  AtBeginFound;
	char  AtIDFound;
	char  LeaveHighliteOn;
	char  EnteredCode;
	char  CodingScheme;
	char  PriorityCodes;
	char  MakeBackupFile;
	int   FreqCountLimit;
	AttTYPE  gAtt;
	AttTYPE  attOutputToScreen;
	char  isRedrawTextWindow;
	Size  scale_row;
	short winID;
	short platform;
	short MinFontSize;
	char *PIDs;
	WindowInfo winInfo;
	webMFile *webMFiles;
	pctInfo PcTr;
	txtInfo TxTr;
	sndInfo SnTr;
	movInfo MvTr;
	movInfo *cpMvTr;
	COLORWORDLIST *RootColorWord;
	COLORTEXTLIST *RootColorText;
	TIERS *headtier;
	ERRLIST *headerr;
	SPLIST *headsp;
	BE_TIERS *head_bg;
	TCODES *headcodes;
	CHECK_IN_TIERS *headt, *maint, *codet;
	CODES *RootCodes, *CurCode;
	CODES **StartCodeArr, **EndCodeArr, **RootCodesArr;
	CODES *FirstCodeOfPrevLevel, *CurFirstCodeOfPrevLevel;

#ifdef _WIN32
#endif
#ifdef _MAC_CODE
	WindowPtr wind;
	ControlRef VScrollHnd;
	ControlRef HScrollHnd;
#endif
} myFInfo;

#ifdef _MAC_CODE
#define NUMMOVIEUNDO 7
typedef struct {
	char  type;
	long  val;
	short selBeg,
	selEnd;
	short which;
} MovieUndo;

typedef struct {
	unCH			fName[FILENAME_MAX];
	char			isPlaying;
	char			toOrgWind;
	Rect 			orgMvBounds;
	Rect 			MvBounds;
	Rect			Beg;
	Rect			End;
	Rect			zoomRatio;
	Rect			begBox;
	Rect			endBox;
	Rect			posBox;
	Rect			rptTime;
	Rect			Save;
	Rect			Repeat;
	Rect			help;
	Movie			MvMovie;
	TimeValue		MDur;
	TimeValue		MBeg;
	TimeValue		MEnd;
	MovieUndo		undo[NUMMOVIEUNDO];
	short			undoIndex;
	myFInfo			*df;
	WindowPtr		win;
	MovieController	MvController;
} MovieInfo;

#define MovieTNInfo struct MMoviePics
struct MMoviePics {
	unCH		*orgFName;
	FNType		*fName;
	unCH		text[50];
	long		textLen;
	TimeValue	MBeg;
	TimeValue	MEnd;
	unsigned int row;
	double		aspectRetio;
	PicHandle	pict;
	Rect		dstRect;
	FONTINFO	Font;
	MovieTNInfo *nextPict;
} ;

#define MovieTXTInfo struct TMoviePics
struct TMoviePics {
	unCH		*text;
	AttTYPE		*atts;
	char		isShowPict;
	char		isNewLine;
	TimeValue	MBeg;
	TimeValue	MEnd;
	unsigned int row;
	double		aspectRetio;
	PicHandle	pict;
	Rect		dstRect;
	FONTINFO	Font;
	MovieTXTInfo *nextPict;
} ;

typedef struct {
	myFInfo		*FileInfo;
	short		id;
	unsigned int Offset;
	FNType		wname[FNSize];
	UInt32		fileDate;
	ControlRef VScrollHnd;
	ControlRef HScrollHnd;
#if (TARGET_API_MAC_CARBON == 1)
	TSMDocumentID idocID;
	EventHandlerUPP textHandler;
	EventHandlerRef mouseEventHandler, textEventHandler;
#endif
	short		(*EventProc)(WindowPtr, EventRecord *);
	void		(*UpdateProc)(WindowPtr);
	void		(*CloseProc)(WindowPtr);
} WindowProcRec;

struct windrec {
	WindowPtr		wind;
	WindowProcRec	*windRec;
	struct windrec	*nextWind;
} ;
typedef struct windrec MACWINDOWS;

struct MPSR1020 {
	char font[256];
} ;

struct SelectedFilesList {
	FNType *fname;
	struct SelectedFilesList *nextFile;
} ;

#endif // _MAC_CODE

#ifdef _MAC_CODE
	typedef struct {
		long	saveA4;
		GrafPtr	port;
	} PrepareStruct;
	struct MacWorld {
		Boolean isOldMac;
		Boolean isOldVersion;
		Boolean hasColorQD;
	} ;

	extern char isAjustCursor;
	extern struct MacWorld theWorld;
	extern FNType defaultPath[];
	extern FNType webDownLoadDir[];
	extern Point SFGwhere;
	extern EventRecord *gCurrentEvent;
	extern CFByteOrder byteOrder;
	extern MovieInfo *theMovie;
	extern MovieTNInfo *moviePics;
	extern struct SelectedFilesList *SelectedFiles;

	extern int  OpenWindow(short,FNType *,long,char,char,short (*EP)(WindowPtr,EventRecord *),void (*UP)(WindowPtr),void (*CL)(WindowPtr));
	extern int  TotalNumOfRows(WindowProcRec *windProc);
	extern int  NumOfRowsInAWindow(WindowPtr wp, int *char_height, int offset);
	extern char isInsideTotalScreen(Rect *r);
	extern char isInsideScreen(short top, short left, Rect *res);
	extern char GetFontOfTEXT(char *buf, FNType *fname);
	extern char DownloadURL(char *url,unsigned long maxSize,char *memBuf,int memBufLen,char *fname,char isRunIt,char isRawUrl,const char *mess);
	extern void MacintoshInit(void);
	extern void Do_A_ScrollBar(short code,ControlRef theControl,Point myPt); 
	extern void EnableDItem(DialogPtr dial, short which, short enable);
	extern void UpdateMainText(WindowPtr wind);
	extern void RestoreWindA4(PrepareStruct *rec);
	extern void PrepareWindA4(WindowPtr wind, PrepareStruct *rec);
	extern void CenterWindow(WindowPtr GetSelection, short forLR, short forTB);
	extern void UpdateWindowProc(WindowProcRec	*windProc);
	extern void MoveRegionUpAndAddLastTier(WINDOW *w);
	extern void MoveRegionDownAndAddFirstTier(WINDOW *w);
	extern void changeCurrentWindow(WindowPtr fromWindow, WindowPtr toWindow, char isSelectWin);
	extern void initFoldersList(void);
	extern void initURLDownload(char isAll);
	extern void OpenFoldersWindow(int which);
	extern void changeCommandFolder(int which, FNType *text);
	extern void addToFolders(int which, FNType *dir);
	extern void mac_call(void);
	extern void SelectCursorPosition(EventRecord *event, int extend);
	extern void setCursorPos(WindowPtr wind);
	extern void ajustCursorPos(FNType *name);
	extern void SetCurrentEventRecord(EventRecord *theEvent);
	extern void ControlCTRL(WindowPtr win, SInt32 id, short toDo, ControlPartCode val);
	extern void HandleVScrollBar(short code, ControlRef theControl,Point myPt);
	extern void HandleHScrollBar(short code, ControlRef theControl, Point myPt);
	extern void HandleWebVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleProgsVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleFoldersVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleCharsVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleHlpVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleRecallVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleThumbVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void HandleTextVScrollBar(WindowPtr win, short code, ControlRef theControl, Point myPt);
	extern void GetWindTopLeft(WindowPtr in_wind, short *left, short *top);
	extern void initFavFont(void);
	extern void addToFavFont(char *fontName, char isSavePrefs);
	extern void write_FavFonts_2014(FILE *fp);
	extern void delay_mach(Size num);
	extern void mCloseWindow(WindowPtr wind);
	extern short DoEdit(short theItem, WindowProcRec *windProc);
	extern short MainTextEvent(WindowPtr wind, EventRecord *event);
	extern short DoCommand(long key, WindowProcRec *windProc);
	extern short RealMainEvent(int *key);
	extern myFInfo *WindowFromGlobal_df(myFInfo *df);
	extern struct SelectedFilesList *freeSelectedFiles(struct SelectedFilesList *p);
	extern WindowPtr getNibWindow(CFStringRef kNibName);
	extern WindowPtr isWinExists(short id, FNType *name, char checkForDups);
	extern WindowPtr FindAWindowProc(WindowProcRec *windProc);
	extern WindowPtr FindAWindowNamed(FNType *name);
	extern WindowPtr FindAWindowID(short id);
	extern WindowProcRec *WindowProcs(WindowPtr wind);
	extern EventRecord *GetCurrentEventRecord(void);
	extern ControlRef GetWindowItemAsControl(WindowPtr myDlg, SInt32 id);
	extern short GetFontHeight(FONTINFO *fontInfo, NewFontInfo *finfo, WindowPtr wind);
	extern myFInfo *InitFileInfo(FNType *fname, long lLen, WindowPtr wind, short id, char *isUTF8Header);
#endif // _MAC_CODE

#if defined(_WIN32)
	typedef struct {
		UInt8	hours;
		UInt8	minutes;
		UInt8	seconds;
		UInt8	frames;
	} DVTimeCode;
	
	extern UINT win95_call(UINT nChar, UINT nRepCnt, UINT nFlags, UINT whichInput);
	extern void HandleVScrollBar(UINT nSBCode, UINT nPos, CWnd* win);
	extern void HandleHScrollBar(UINT nSBCode, UINT nPos, CWnd* win);
	extern void StartSelectCursorPosition(CPoint point, int extend);
	extern void EndSelectCursorPosition(CPoint point, int extend);
	extern void delay_mach(DWORD num);
	extern void AdjustName(unCH *toSt, unCH *fromSt, int WinLim);
	extern int  MidSelectCursorPosition(CPoint point, int extend);
	extern int  DisplayFileText(char *justFname);
	extern short GetFontHeight(FONTINFO *fontInfo, NewFontInfo *finfo);
	extern myFInfo *InitFileInfo(FNType *fname, long lLen, short id, char *isUTF8Header);
#endif /* else _WIN32 */


extern char defUniFontName[];
extern long defUniFontSize;
extern long stickyFontSize;
extern short ArialUnicodeFOND;
extern short SecondDefUniFOND;


extern int  FreqCountLimit;
extern int  isPlayS;
extern int	F_key;
extern int  InKey;
extern int  LineNumberDigitSize;
extern int  streamSpeedNumber;
extern unsigned int globalWhichInput;

extern short SearchFFlag;
extern short TabSize;
extern short NonModal;
extern double ThumbnailsHight;

extern char last_cr_char;
extern char LineNumberingType;
extern char isUnixCRs;
extern char isUseSPCKeyShortcuts;
extern char isUTFData;
extern char C_MBF;
extern char DisTier[];
extern char *ConstString[];
extern char *MEMPROT;
extern char DefChatMode;
extern char DefCodingScheme;
extern char DefPriorityCodes;
extern char AutoScriptSelect;
extern char PlayingSound;
extern char PlayingContSound;
extern char PlayingContMovie;
extern char MakeBackupFile;
extern unCH NextTierName[80];
extern char StartInEditorMode;
extern char ShowPercentOfFile;
extern char doMixedSTWave;
extern char mem_error;
extern char isChatLineNums;
extern char sendMessageTargetApp;
extern char F5Option;
extern char DefAutoWrap;
extern char doReWrap;
extern char rawTextInput;
extern char TranslateInAllCases;
extern char URL_Address[];
extern unCH sp[], ced_line[];
extern char spC[], ced_lineC[];
extern char tempW[];
extern FNType CodesFName[];
extern FNType KeysFName[];
extern FNType CODEFNAME[];
extern FNType STATEFNAME[];
extern myFInfo *global_df;
extern myFInfo *lastGlobal_df;

extern int  ced_getc(void);
extern int  proccessKeys(unsigned int c);
extern int  FindTextCodeLine(unCH *code, unCH *CodeMatch);
extern int  NumOfRowsOfDefaultWindow(void);
extern int  getNumberOffset(void);
extern int  WindowPageWidth(void);
extern int  WindowPageLength(int *size);
extern int  NewLine(int i);
extern int  new_getstr(unCH *st, int len, int (* fn)(WINDOW *w,unCH *st,unCH c));
extern int  BeginningOfLine(int i);
extern int  MoveDown(int i);
extern int  FindRightCode(int disp);
extern int  SaveToFile(char *fn);
extern int  GetCurHidenCode(char move, unCH *CodeMatch);
extern int  SoundMode(int i);
extern int  PlayMedia(int i);
extern int  PlayStepForward(int i);
extern int  PlayStepBackward(int i);
extern int  ShowPicture(int c);
extern int  ShowTextFile(int c);
extern int  SoundToTextSync(int i);
extern int  MovieToTextSync(int i);
extern int  MovieThumbNails(int i);
extern int  PlayContMedia(int i);
extern int  PlayContSkipMedia(int i);
extern int  QuickTrinscribeMedia(int i);
extern int  FindNextMediaTier(int i);
extern int  FindPrevMediaTier(int i);
extern int  MoveMediaEndRight(int i);
extern int  MoveMediaEndLeft(int i);
extern int  MoveMediaBegRight(int i);
extern int  MoveMediaBegLeft(int i);
extern int  DescribeKey(int i);
extern int  CursorsCommand(int i);
extern int  Apropos(int d);
extern int  AproposFile(int d);
extern int  PercentOfFile(int d);
extern int  WriteState(int d);
extern int  ChatModeSet(int d);
extern int  ExecCommand(int d);
extern int  RefreshScreen(int i);
extern int  AbortCommand(int i);
extern int  DefConstString(int i);
extern int  InsertConstString(int i);
extern int  EnterAscii(int i);
extern int  SelectTiers(int i);
extern int  SoundWinSync(int i);
extern int  Check(int d);
extern int  DeleteNextChar(int i);
extern int  DeletePrevChar(int i);
extern int  VisitFile(int i);
extern int  GetNewCodes(int i);
extern int  MoveCodeCursorUp(int i);
extern int  MoveCodeCursorDown(int i);
extern int  MoveCodeCursorLeft(int i);
extern int  MoveCodeCursorRight(int i);
extern int  MoveUp(int i);
extern int  MoveLineUp(int i);
extern int  MoveLineDown(int i);
extern int  MoveRight(int i);
extern int  MoveLeft(int i);
extern int  BeginningOfWindow(int i);
extern int  EndOfLine(int i);
extern int  EndOfWindow(int i);
extern int  MorDisambiguate(int i);
extern int  ExitCED(int i);
extern int  EditMode(int i);
extern int  Space(int i);
extern int  KillLine(int i);
extern int  YankKilled(int i);
extern int  DeleteNextWord(int i);
extern int  DeletePrevWord(int i);
extern int  GetFakeCursorCode(int i);
extern int  SetNextTierName(int d);
extern int  EndCurTierGotoNext(int i);
extern int  GoToNextMainSpeaker(int i);
extern int  NextPage(int i);
extern int  PrevPage(int i);
extern int  MoveRightWord(int i);
extern int  MoveLeftWord(int i);
extern int  SaveCurrentFile(int i);
extern int  BeginningOfFile(int i);
extern int  EndOfFile(int i);
extern int  GotoLine(int i);
extern int  SelfInsert(int i);
extern int  Undo(int i);
extern int  Redo(int i);
extern int  HandleESC(int i);
extern int  HandleCTRLX(int i);
extern int  ToTopLevel(int i);
extern int  IllegalFunction(int i);
extern int  GetCursorCode(int i);
extern int  ShowParagraphMarks(int d);
extern int  DoConstantString(void);
extern int  SelectAll(int i);
extern int  FindSame(int i);
extern int  DisplayText(FNType *fname);
extern int  NONEXISTANT(int i);
extern int  RefreshScreen(int i);
extern int  init_codes(const FNType *fname, const FNType *cname, char *fontName);
extern int  ContPlayPause(void);
extern int  clGetVersion(int d);
extern int  clWriteFile(int i);
extern int  DisplayCommands(WINDOW *w, char *st, char ch);
extern long DoLineNumber(long lineno);
extern long InsertStringWithReplace(char *buf, char *st, long pos);
extern long DoGetFreqCount(long num);
extern long Con_PrevPage(long col);
extern long ComColChr(unCH *line, long col_w);
extern long ComColWin(char isIgnoreBullets, unCH *line, long col_c);
extern long DealWithAtts(unCH *line, long i, AttTYPE att, AttTYPE oldAtt, char isJustStats);
extern long AlignMultibyteMediaStream(long num, char dir);
extern long UnicodeToUTF8(const unCH *UniStr, unsigned long actualStringLength, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen);
extern long UnicodeToANSI(unCH *UniStr, unsigned long actualStringLength, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script);
extern long UTF8ToUnicode(unsigned char *UTF8str, unsigned long actualUT8Len, unCH *UniStr, unsigned long *actualUnicodeLength, unsigned long MaxLen);
extern long UTF8ToANSI(unsigned char *UTF8str, unsigned long actualUT8Len, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script);
extern long ANSIToUTF8(unsigned char *ANSIstr, unsigned long actualANSILen, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen, short script);
extern long ANSIToUnicode(unsigned char *ANSIstr, unsigned long actualANSILen, unCH *UniStr, unsigned long *actualStringLength, unsigned long MaxLen, short script);
extern long roundUp(double num);
extern short DoFile(short theItem);
extern short DoMORGrammar(short theItem);
extern short DoKidevalDB(short theItem);
extern short IsGSet(short which);
extern char SetNewFont(char *st, char ec, NewFontInfo *finfo);
extern char isCHATFile(FNType *fname);
extern char isCEXFile(FNType *fname);
extern char SetChatModeOfDatafileExt(FNType *fname, char isJustCheck);
extern char issoundwin(void);
extern char isKeyEqualCommand(int key, int commNum);
extern char QuitDialog(FNType *st);
extern char UpdateCLANDialog(FNType *st);
extern char UpdateCodesCursorPos(int c_row, int c_col);
extern char AdjustSound(int row, int col, int ext, Size right_lim);
extern char AllocConstString(char *st, int sp);
extern char AllocSpeakerNames(unCH *st, int sp);
extern char GetSpeakerNames(unCH *st, int num, unsigned long size);
extern char ShowGRA();
extern char FindFileLine(char isTest, char *text);
extern char OpenProgressDialog(const char *str);
extern char UpdateProgressDialog(short percentFilled);
extern char DoURLFileName(char *tName);
extern char isCallFixLine(long beg_c, long col_c, char buildLine);
extern char *fgets_ced(char *beg,int size,FILE *fp,unsigned long *cnt);
extern char DeleteChank(int d);
extern char FoundEndHTier(ROWS *tr, char isMove);
extern char AtTopEnd(ROWS *p, ROWS *head, char all);
extern char AtBotEnd(ROWS *p, ROWS *tail, char all);
extern char CheckLeftCol(long i);
extern char init_windows(int com, char refresh, char isJustTextWin);
extern char GetPrefSize(short id, FNType *name, long *height, long *width, long *top, long *left);
extern char AddLineToRow(ROWS *, unCH tierType, void *data);
extern char Re_WrapLines(char (*finLine)(ROWS *, unCH tierType, void *),long,char,void *);
extern char doCopy(void);
extern char getFileDate(FNType *fn, UInt32 *date);
extern void setFileDate(FNType *fn, UInt32 date);
extern void SetPBCglobal_df(char isReset, long cur_pos);
extern void Quit_CED(int num);
extern void ShowAllTiers(int i);
extern void matchCursorColToGivenCol(long col);
extern void FindAnyBulletsOnLine(void);
extern void Con_NextPage(void);
extern void StyleItems(char which, char color);
extern void SetTextWinMenus(char isSampleAtt);
extern void setTabSize(void);
extern void RefreshAllTextWindows(char isInitWindow);
extern void resetOptions(void);
extern void myget(void);
extern void doCut(void);
extern void doPaste(void);
extern void GetCurCode(void);
extern void draw_mid_wm(void);
extern void PositionCursor(void);
extern void SetTextKeyScript(char isForce);
extern void SetDefKeyScript(void);
extern void ChangeCurLine(void);
extern void ChangeCurLineAlways(short which);
extern void SaveCKPFile(char isask);
extern void SetCurrentFontParams(short font, long size);
extern void ActivateFrontWindow(short active);
extern void SetScrollControl(void);
extern void PosAndDispl(void);
extern void ResetUndos(void);
extern void SaveUndoState(char isDummy);
extern void RemoveLastUndo(void);
extern void AddConstString(int sp);
extern void AddSpeakerNames(int sp);
extern void AddCodeTier(unCH *code, char real_code);
extern void InitFileDialog(void);
extern void InitSelectedTiers(void);
extern void InitEvalOptions(void);
extern void InitKidevalOptions(void);
extern void InitSelectedSearch(void);
extern void get_selected_file(int fnum, FNType *s, int max);
extern void CpCur_lineAttToHead_rowAtt(ROWS *cl);
extern void CpCur_lineToHead_row(ROWS *cl);
extern void PutCursorInWindow(WINDOW *w);
extern void UpdateWindowNamed(FNType *name);
extern void CloseAllWindows(void);
extern void InitMyWindows(void);
extern void set_folders(int which, FNType *text);
extern void write_folders_2011_2013(FILE *fp);
extern void InstallAEProcs(void);
extern void RemoveAEProcs(void);
extern void FreeFileInfo(myFInfo *FileInfo);
extern void ChangeWindowsMenuItem(FNType *temp, int add);
extern void ProgExit(const char *err);
extern void FindMidWindow(void);
extern void DisplayTextWindow(ROWS *row_line, char refresh);
extern void FixLine(char UpdateUndo);
extern void AddString(unCH *s, long len, char UpdateUndo);
extern void AddText(unCH *s, unCH c, short DisplayAll, long len);
extern void init_text(char *isUTF8Header, short id);
extern void FreeUpText(myFInfo *FInfo);
extern void mem_err(char bck, myFInfo *DF);
extern void DrawMouseCursor(char which);
extern void RefreshOtherCursesWindows(char all);
extern char RSoundPlay(int c, char isPreserveTimes);
extern void EnterSelection(void);
extern void freeUndo(UNDO *p);
extern void DoString(unCH *code);
extern void ExecSoundCom(int c);
extern void ced_getflag(char *f);
extern void DisplayRow(char isShift);
extern void MoveToSubcode(void);
extern void MoveToSpeaker(long num, char refresh);
extern void MoveToLine(long num, char refresh);
extern void PrepWindow(short id);
extern void FindLastPage(int wsize);
extern void FinishMainLoop(void);
extern void DoDefContStrings(void);
extern void DoCEDOptions(void);
extern void DoSndAnaOptions(void);
extern void DoSndF5Options(void);
extern void DoLineNumSize(void);
extern void DoURLAddress(void);
extern void DoThumbnails(void);
extern void SpeedDialog(void);
extern void DoColorKeywords(void);
extern void SetGOption(short which, int val);
extern void SndSyncOption(void);
extern void DoFont(short theItem);
extern void DoSize_Style(short theItem);
extern void DoDefFont(short theItem);
extern void DoDefSize_Style(short theItem);
extern void ControlAllControls(int on, short id);
extern void ced_check_init(char first);
extern void main_check_init(void);
extern void init_keys(FNType *fname, FNType *cname);
extern void DisplayCursor(int wsize);
extern void DisplayEndF(char all);
extern void MapArrToList(CODES *CurCode);
extern void SetUpParticipants(void);
extern void SaveCKPFile(char isask);
extern void AboutCLAN(char isNear);
extern void InitRedirects(void);
extern void PutSoundStats(Size ws);
extern void OpenCommandsWindow(char reOpen);
extern void PlaybackControlWindow(void);
extern void PrintSoundWin(int col, int cm, Size cur);
extern void ChangeSpeakerMenuItem(void);
extern void CloseProgressDialog(void);
extern void getTextAndSendToSoundAnalyzer(void);
extern void checkForTiersToHide(myFInfo *df, char error);
extern void FreeCodesMem(void);
extern void SortCommands(int *sl, const char *pat, char ch);
extern void AddKeysToCommand(unsigned int cm, char *st);
extern void SetUpVideos(void);
extern void getVideosExt(int key);
extern ROWS *ToNextRow(ROWS *p, char all);
extern ROWS *ToPrevRow(ROWS *p, char all);
extern CODES **DisplayCodes(int wsize);
extern unCH Char2SpChar(int key, char num);
extern FONTINFO *GetTextFont(myFInfo *df);
}

#endif /* _C_CED_ */
