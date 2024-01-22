//
//  NSWindowController+CLANOptions.m
//  Clan
//

#import "ced.h"
#import "cu.h"
#import "EvalController.h"
#import "CommandsController.h"

#define SP_CODE_FIRST	  1
#define SP_CODE_LAST	  8

#define CODESTRLEN  6

extern int  F_numfiles;
extern int  cl_argc;
extern char *cl_argv[];
extern char wd_dir[];

static char AnomicTp, GlobalTp, BrocaTp, WernickeTp, TranssenTp, TransmotTp,
	ConductionTp, ControlTp, NotAphByWab, MaleOnly, FemaleOnly, SpeechGm,
	StrokeGm, WindowGm, Impevent, Umbrella, Cat, Flood, Cinderella, Sandwich;
static char AgeRange[256];
static char spCodeEv[SP_CODE_LAST][CODESTRLEN + 1];
static BOOL spSetEv[SP_CODE_LAST];

void InitEvalOptions(void) {
	AnomicTp = 0;
	GlobalTp = 0;
	BrocaTp = 0;
	WernickeTp = 0;
	TranssenTp = 0;
	TransmotTp = 0;
	ConductionTp = 0;
	ControlTp = 0;
	NotAphByWab = 0;
	MaleOnly = 0;
	FemaleOnly = 0;
	SpeechGm = 0;
	StrokeGm = 0;
	WindowGm = 0;
	Impevent = 0;
	Umbrella = 0;
	Cat = 0;
	Flood = 0;
	Cinderella = 0;
	Sandwich = 0;
	AgeRange[0] = EOS;
}

static void addToSpArr(char *code) {
	int i;

	for (i=SP_CODE_FIRST-1; i < SP_CODE_LAST; i++) {
		if (spCodeEv[i][0] == EOS)
			break;
		if (uS.mStricmp(spCodeEv[i], code) == 0) {
			return;
		}
	}
	if (i < SP_CODE_LAST) {
		strncpy(spCodeEv[i], code, CODESTRLEN);
		spCodeEv[i][CODESTRLEN] = EOS;
	}
}

static char getSpNamesFromFile(int *cnt, char *fname, char isFoundFile) {
	char *s, *code;
	FILE *fp;

	fp = fopen(fname, "r");
	if (fp == NULL)
		return(isFoundFile);
//	*cnt = *cnt + 1;
	while (fgets_cr(templineC3, UTTLINELEN, fp)) {
		if (templineC3[0] == '*') {
			if (isFoundFile)
				break;
			s = strchr(templineC3, ':');
			if (s != NULL) {
				*s = EOS;
				addToSpArr(templineC3);
				isFoundFile = TRUE;
			}
		} else if (strncmp(templineC3, "@ID:", 4) == 0) {
			s = templineC3 + 4;
			while (isSpace(*s))
				s++;
			code = s;
			s = strchr(templineC3, '|');
			if (s != NULL) {
				s++;
				code = strchr(s, '|');
				if (code != NULL) {
					*code = '*';
					s = strchr(code, '|');
					if (s != NULL) {
						isFoundFile = TRUE;
						*s = EOS;
						uS.remblanks(code);
						addToSpArr(code);
					}
				}
			}
		}
	}
	fclose(fp);
	return(isFoundFile);
}

static char makeSpeakersList(void) {
	int		i, j, len, cnt;
	char	isFoundFile;
	FILE	*fp;

	SetNewVol(wd_dir);
	isFoundFile = FALSE;
	cnt = 0;
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles; j++) {
					get_selected_file(j, FileName1, FNSize);
					isFoundFile = getSpNamesFromFile(&cnt, FileName1, isFoundFile);
				}
			} else if (cl_argv[i][0] == '@' && cl_argv[i][1] == ':') {
				uS.str2FNType(FileName1, 0L, cl_argv[i]+2);
				fp = fopen(FileName1, "r");
				if (fp == NULL) {
					return(isFoundFile);
				}
				while (fgets_cr(FileName1, FNSize, fp)) {
					uS.remFrontAndBackBlanks(FileName1);
					isFoundFile = getSpNamesFromFile(&cnt, FileName1, isFoundFile);
				}
				fclose(fp);
			} else if (strchr(cl_argv[i], '*') == NULL) {
				isFoundFile = getSpNamesFromFile(&cnt, cl_argv[i], isFoundFile);
			} else {
				strcpy(DirPathName, wd_dir);
				len = strlen(DirPathName);
				j = 1;
				while ((j=Get_File(FileName1, j)) != 0) {
					if (uS.fIpatmat(FileName1, cl_argv[i])) {
						addFilename2Path(DirPathName, FileName1);
						isFoundFile = getSpNamesFromFile(&cnt, DirPathName, isFoundFile);
						DirPathName[len] = EOS;
					}
				}
			}
		}
	}
	return(isFoundFile);
}

@implementation EvalController

static EvalController *EvalWindow = nil;

- (id)init {
	EvalWindow = nil;
	return [super initWithWindowNibName:@"Kideval"];
}

+ (const char *)EvalDialog;
{
	int	i;
//	NSRect wFrame;

	NSLog(@"EvalController: EvalDialog\n");

	if (F_numfiles <= 0) {
		return("ERROR EVAL. Please specify input data files first");
	}
	cl_argc = 1;
	strcpy(templineC2, " @");
	if (!MakeArgs(templineC2))
		return("Internal Error!");
	if (EvalWindow == nil) {
		for (i=SP_CODE_FIRST-1; i < SP_CODE_LAST; i++) {
			spCodeEv[i][0] = EOS;
			spSetEv[i] = FALSE;
		}
		if (makeSpeakersList() == FALSE) {
			return("Can't open any of specified files. Please check for specified files in working directory");
		}
		EvalWindow = [[EvalController alloc] initWithWindowNibName:@"Eval"];

//		[EvalWindow showWindow:nil];
		[[commandsWindow window] beginSheet:[EvalWindow window] completionHandler:nil];
	}
	return(NULL);
}

- (void)setButton:(NSButton *)tButton index:(int)i {
	if (spCodeEv[i][0] != EOS) {
		tButton.title = [NSString stringWithUTF8String:spCodeEv[i]];
		if (spSetEv[i])
			tButton.state =   NSControlStateValueOn;
		else
			tButton.state =   NSControlStateValueOff;
		[tButton setHidden:NO];
	} else {
		tButton.state =   NSControlStateValueOff;
		[tButton setHidden:YES];
	}
}

- (void)windowDidLoad {
	int	i;
	NSWindow *window = [self window];

	[window setIdentifier:@"Eval"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];


	for (i=SP_CODE_FIRST-1; i < SP_CODE_LAST; i++) {
		if (uS.mStricmp(spCodeEv[i], "*PAR") == 0) {
			spSetEv[i] = TRUE;
		} else {
			spSetEv[i] = FALSE;
		}
	}
	[self setButton:sp_oneCH   index:0];
	[self setButton:sp_twoCH   index:1];
	[self setButton:sp_threeCH index:2];
	[self setButton:sp_fourCH  index:3];
	[self setButton:sp_fiveCH  index:4];
	[self setButton:sp_sixCH   index:5];
	[self setButton:sp_sevenCH index:6];
	[self setButton:sp_eightCH index:7];

	[AgeRangeField setStringValue:[NSString stringWithUTF8String:AgeRange]];

	AnomicCH.state =    NSControlStateValueOff;
	GlobalCH.state =    NSControlStateValueOff;
	ControlCH.state =   NSControlStateValueOff;
	BrocaCH.state =     NSControlStateValueOff;
	WernickeCH.state =  NSControlStateValueOff;
	ConductionCH.state =   NSControlStateValueOff;
	TransSensoryCH.state =  NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state =  NSControlStateValueOff;
	if (AnomicTp != 0)
		AnomicCH.state =  NSControlStateValueOn;
	if (GlobalTp != 0)
		GlobalCH.state =   NSControlStateValueOn;
	if (BrocaTp != 0)
		BrocaCH.state =  NSControlStateValueOn;
	if (WernickeTp != 0)
		WernickeCH.state = NSControlStateValueOn;
	if (TranssenTp != 0)
		TransSensoryCH.state =  NSControlStateValueOn;
	if (TransmotTp != 0)
		TransMotorCH.state =   NSControlStateValueOn;
	if (ConductionTp != 0)
		ConductionCH.state =   NSControlStateValueOn;
	if (ControlTp != 0)
		ControlCH.state =   NSControlStateValueOn;
	if (NotAphByWab != 0)
		NotAphasicByWABCH.state =  NSControlStateValueOn;

	maleCH.state = NSControlStateValueOff;
	femaleCH.state = NSControlStateValueOff;
	if (MaleOnly)
		maleCH.state = NSControlStateValueOn;
	else if (FemaleOnly)
		femaleCH.state = NSControlStateValueOn;

	SpeechCH.state =   NSControlStateValueOff;
	CatCH.state =    NSControlStateValueOff;
	FloodCH.state =   NSControlStateValueOff;
	CinderellaCH.state =  NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state =   NSControlStateValueOff;
	Important_EventCH.state =  NSControlStateValueOff;
	StrokeCH.state =   NSControlStateValueOff;
	WindowCH.state =  NSControlStateValueOff;
	if (SpeechGm != 0)
		SpeechCH.state =   NSControlStateValueOn;
	if (StrokeGm != 0)
		StrokeCH.state =   NSControlStateValueOn;
	if (WindowGm != 0)
		WindowCH.state =  NSControlStateValueOn;
	if (Impevent != 0)
		Important_EventCH.state =  NSControlStateValueOn;
	if (Umbrella != 0)
		UmbrellaCH.state = NSControlStateValueOn;
	if (Cat != 0)
		CatCH.state =    NSControlStateValueOn;
	if (Flood != 0)
		FloodCH.state =   NSControlStateValueOn;
	if (Cinderella != 0)
		CinderellaCH.state =  NSControlStateValueOn;
	if (Sandwich != 0)
		SandwichCH.state =   NSControlStateValueOn;
}


- (IBAction)sp_oneClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOn;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[0] = TRUE;
}

- (IBAction)sp_twoClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOn;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[1] = TRUE;
}

- (IBAction)sp_threeClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOn;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[2] = TRUE;
}

- (IBAction)sp_fourClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOn;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[3] = TRUE;
}

- (IBAction)sp_fiveClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOn;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[4] = TRUE;
}

- (IBAction)sp_sixClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOn;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[5] = TRUE;
}

- (IBAction)sp_sevenClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOn;
	sp_eightCH.state = NSControlStateValueOff;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[6] = TRUE;
}

- (IBAction)sp_eightClicked:(NSButton *)sender
{
	int i;
#pragma unused (sender)

	sp_oneCH.state =   NSControlStateValueOff;
	sp_twoCH.state =   NSControlStateValueOff;
	sp_threeCH.state = NSControlStateValueOff;
	sp_fourCH.state =  NSControlStateValueOff;
	sp_fiveCH.state =  NSControlStateValueOff;
	sp_sixCH.state =   NSControlStateValueOff;
	sp_sevenCH.state = NSControlStateValueOff;
	sp_eightCH.state = NSControlStateValueOn;
	for (i=SP_CODE_FIRST; i <= SP_CODE_LAST; i++) {
		spSetEv[i-1] = FALSE;
	}
	spSetEv[7] = TRUE;
}

- (IBAction)DeselectDatabaseClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)AnomicClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOn;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)GlobalClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOn;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)ControlClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOn;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)BrocaClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOn;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)WernickeClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOn;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)ConductionClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOn;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)TransSensoryClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOn;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)TransMotorClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOn;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)NotAphasicByWABClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOn;
}


- (IBAction)FluentClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOn;
	GlobalCH.state = NSControlStateValueOff;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOff;
	WernickeCH.state = NSControlStateValueOn;
	ConductionCH.state = NSControlStateValueOn;
	TransSensoryCH.state = NSControlStateValueOn;
	TransMotorCH.state = NSControlStateValueOff;
	NotAphasicByWABCH.state = NSControlStateValueOn;
}

- (IBAction)NonFluentClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOff;
	GlobalCH.state = NSControlStateValueOn;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOn;
	WernickeCH.state = NSControlStateValueOff;
	ConductionCH.state = NSControlStateValueOff;
	TransSensoryCH.state = NSControlStateValueOff;
	TransMotorCH.state = NSControlStateValueOn;
	NotAphasicByWABCH.state = NSControlStateValueOff;
}

- (IBAction)AllAphasiaClicked:(NSButton *)sender
{
#pragma unused (sender)
	AnomicCH.state = NSControlStateValueOn;
	GlobalCH.state = NSControlStateValueOn;
	ControlCH.state = NSControlStateValueOff;
	BrocaCH.state = NSControlStateValueOn;
	WernickeCH.state = NSControlStateValueOn;
	ConductionCH.state = NSControlStateValueOn;
	TransSensoryCH.state = NSControlStateValueOn;
	TransMotorCH.state = NSControlStateValueOn;
	NotAphasicByWABCH.state = NSControlStateValueOn;
}


- (IBAction)maleClicked:(NSButton *)sender
{
#pragma unused (sender)

	maleCH.state = NSControlStateValueOn;
	femaleCH.state = NSControlStateValueOff;
}

- (IBAction)femaleClicked:(NSButton *)sender
{
#pragma unused (sender)

	maleCH.state = NSControlStateValueOff;
	femaleCH.state = NSControlStateValueOn;
}


- (IBAction)DeselectAllGemsClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)SelectAllGemsClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOn;
	CatCH.state = NSControlStateValueOn;
	FloodCH.state = NSControlStateValueOn;
	CinderellaCH.state = NSControlStateValueOn;
	UmbrellaCH.state = NSControlStateValueOn;
	SandwichCH.state = NSControlStateValueOn;
	Important_EventCH.state = NSControlStateValueOn;
	StrokeCH.state = NSControlStateValueOn;
	WindowCH.state = NSControlStateValueOn;
}


- (IBAction)SpeechClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOn;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)CatClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOn;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)FloodClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOn;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)CinderellaClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOn;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)UmbrellaClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOn;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)SandwichClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOn;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)Important_EventClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOn;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)StrokeClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOn;
	WindowCH.state = NSControlStateValueOff;
}

- (IBAction)WindowClicked:(NSButton *)sender
{
#pragma unused (sender)
	SpeechCH.state = NSControlStateValueOff;
	CatCH.state = NSControlStateValueOff;
	FloodCH.state = NSControlStateValueOff;
	CinderellaCH.state = NSControlStateValueOff;
	UmbrellaCH.state = NSControlStateValueOff;
	SandwichCH.state = NSControlStateValueOff;
	Important_EventCH.state = NSControlStateValueOff;
	StrokeCH.state = NSControlStateValueOff;
	WindowCH.state = NSControlStateValueOn;
}


//	WriteCedPreference();

- (IBAction)OKClicked:(NSButton *)sender
{
	int			i;
	char		isGender;
	NSUInteger len;
	NSString *cStr;

#pragma unused (sender)
	NSLog(@"EvalController: OKClicked\n");

	templineC3[0] = EOS;
	if (commandsWindow != nil) {
		AnomicTp = (AnomicCH.state == NSControlStateValueOn);
		GlobalTp = (GlobalCH.state == NSControlStateValueOn);
		BrocaTp = (BrocaCH.state == NSControlStateValueOn);
		WernickeTp = (WernickeCH.state == NSControlStateValueOn);
		TranssenTp = (TransSensoryCH.state == NSControlStateValueOn);
		TransmotTp = (TransMotorCH.state == NSControlStateValueOn);
		ConductionTp = (ConductionCH.state == NSControlStateValueOn);
		ControlTp = (ControlCH.state == NSControlStateValueOn);
		NotAphByWab = (NotAphasicByWABCH.state == NSControlStateValueOn);
		MaleOnly = (maleCH.state == NSControlStateValueOn);
		FemaleOnly = (femaleCH.state == NSControlStateValueOn);
		SpeechGm = (SpeechCH.state == NSControlStateValueOn);
		StrokeGm = (StrokeCH.state == NSControlStateValueOn);
		WindowGm = (WindowCH.state == NSControlStateValueOn);
		Impevent = (Important_EventCH.state == NSControlStateValueOn);
		Umbrella = (UmbrellaCH.state == NSControlStateValueOn);
		Cat = (CatCH.state == NSControlStateValueOn);
		Flood = (FloodCH.state == NSControlStateValueOn);
		Cinderella = (CinderellaCH.state == NSControlStateValueOn);
		Sandwich = (SandwichCH.state == NSControlStateValueOn);

		cStr = [AgeRangeField stringValue];
		len = [cStr length];
		if (len > 256)
			len = 255;
		[cStr getCharacters:templineW range:NSMakeRange(0, len)];
		templineW[len] = EOS;
		if (templineW[0] != EOS)
			u_strcpy(AgeRange, templineW, 256);
		else
			AgeRange[0] = EOS;

		if (SpeechGm && StrokeGm && WindowGm && Impevent && Umbrella && Cat && Flood && Cinderella && Sandwich) {
			SpeechGm = StrokeGm = WindowGm = Impevent = Umbrella = Cat = Flood = Cinderella = Sandwich = 0;
		}

		for (i=SP_CODE_FIRST-1; i < SP_CODE_LAST; i++) {
			if (spSetEv[i] == TRUE && spCodeEv[i][0] != EOS) {
				strcat(templineC3, " +t");
				strcat(templineC3, spCodeEv[i]);
				strcat(templineC3, ":");
			}
		}

		if (MaleOnly)
			isGender = 1;
		else if (FemaleOnly)
			isGender = 2;
		else
			isGender = 0;
		if (AnomicTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Anomic");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (GlobalTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Global");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (BrocaTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Broca");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (WernickeTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Wernicke");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (TranssenTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "TransSensory");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (TransmotTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "TransMotor");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (ConductionTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Conduction");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (ControlTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "control");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (NotAphByWab) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "NotAphasicByWAB");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (SpeechGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Speech");
			strcat(templineC3, "\"");
		}
		if (StrokeGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Stroke");
			strcat(templineC3, "\"");
		}
		if (WindowGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Window");
			strcat(templineC3, "\"");
		}
		if (Impevent) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Important_Event");
			strcat(templineC3, "\"");
		}
		if (Umbrella) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Umbrella");
			strcat(templineC3, "\"");
		}
		if (Cat) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cat");
			strcat(templineC3, "\"");
		}
		if (Flood) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Flood");
			strcat(templineC3, "\"");
		}
		if (Cinderella) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cinderella");
			strcat(templineC3, "\"");
		}
		if (Sandwich) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Sandwich");
			strcat(templineC3, "\"");
		}
		strcat(templineC3, " +u");

//		cStr = [commandsWindow->commandString stringValue];
		cStr = [NSString stringWithUTF8String:"eval @"];
		cStr = [cStr stringByAppendingString:[NSString stringWithUTF8String:templineC3]];
		[commandsWindow->commandString setStringValue:cStr];
	}
	[[self window] close];
}

- (IBAction)CancelClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"EvalController: CancelClicked\n");

	[[self window] close];
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"EvalController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

	[[commandsWindow window] endSheet:[self window]];

	[EvalWindow release];
	EvalWindow = nil;
}

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"EvalController: windowDidResize\n");
}

// awakeFromNib is called when this object is done being unpacked from the nib file;
// at this point, we can do any needed initialization before turning app control over to the user
- (void)awakeFromNib
{
	int i;

	i = 12;
	// We don't actually need to do anything here, so it's empty
}


@end
