/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef C_CLAN
#define C_CLAN

#define ALIAS_FILE_D	"0aliases.cut"
#define ALIAS_FILE_U	"aliases.cut"
#define	MAX_ARGS		500L
#define EXPANSION_SIZE	5000L /* This number must be larger than 1024 */

#define isOrgUnicodeFont(x) (x == WINArialUC || x == MACArialUC)

extern "C"
{

#ifdef _COCOA_APP
enum {
	up_arrow=1,
	down_arrow
} ;
#endif
extern FNType	wd_dir[],   // working directory
				od_dir[];   // output directory

extern short lastCommand;

#ifdef _MAC_CODE

extern void DoCharStdInput(unCH *st);
extern void DoStdInput(char *st);
#ifdef _COCOA_APP
extern void RecallCommand(short type);
#else
extern void RecallCommand(WindowPtr win, short type);
#endif

#endif // _MAC_CODE

#ifndef _COCOA_APP
extern char *fbuffer;
//extern char *expandedArgv;
#endif
extern FNType *StdInWindow;
extern const char *StdInErrMessage, *StdDoneMessage;

extern int  isKillProgram;
extern int  get_clan_prog_num(char *s, char isLoad);
extern int  rename_each_file(FNType *s, FNType *t, char isChangeCase);
extern int  GetNumberOfCommands(void);
extern int  getCommandNumber(char *cmd);
#ifdef _COCOA_APP
extern int  command_length(void);
extern int  getRecall_curCommand(void);
#endif

extern char init_clan(void);
extern char *NextArg(char *s);
extern char MakeArgs(char *com);
extern char selectChoosenFont(NewFontInfo *finfo, char isForce, char isKeepSize);
#ifdef _COCOA_APP
extern char getAliasProgName(char *inBuf, char *progName, int size);
extern char SetDefaultUnicodeFinfo(NewFontInfo *finfo);
extern char *getCommandAtIndex(long clickedRow);
extern char *getFolderAtIndex(long clickedRow, int which);
#endif

extern void cutt_exit(int i);
extern void att_cp(long pos, char *desSt, const char *srcSt, AttTYPE *desAtt, AttTYPE *srcAtt);
extern void att_shiftright(char *srcSt, AttTYPE *srcAtt, long num);
extern void execute(char *inputBuf, char tDataChanged);
extern void func_init(void);
extern void init_commands(void);
#ifndef _COCOA_APP
extern void free_commands(void);
#endif
extern void set_commands(char *text);
extern void set_lastCommand(short num);
extern void write_commands1977(FILE *fp);
extern void OpenHelpWindow(const FNType *fname);
extern void OpenWebWindow(void);
extern void OpenCharsWindow(void);
extern void show_info(char isJustComm);
extern void InitOptions(void);
extern void SysEventCheck(long);
extern void SetDefaultThaiFinfo(NewFontInfo *finfo);
extern void SetDefaultCAFinfo(NewFontInfo *finfo);
extern void re_readAliases(void);
extern void readAliases(char isInit);
extern void free_aliases(void);
#ifdef _COCOA_APP
extern void delFromRecall(int index);
extern void delFromFolderList(int index, int which);
extern void AddToClan_commands(char *st);
extern void AddComStringToComWin(char *com);
extern void SetWdLibFolder(char *str, int which);

extern NSDictionary *get_next_obj_command(NSUInteger *i);
extern NSDictionary *get_next_obj_folder(NSUInteger *i, int which);
#else
extern char getAliasProgName(char *inBuf, char *progName, int size);
extern char SetDefaultUnicodeFinfo(NewFontInfo *finfo);
#endif
}

#endif // C_CLAN
