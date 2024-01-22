#import "CommandsController.h"
#import "Document.h"
#import "AboutClanController.h"
#import "Controller.h"
#import "ced.h"
#import "cu.h"
#import "c_clan.h"

extern char ced_version[];
extern long option_flags[];
extern struct DefWin ClanWinSize; // 2020-01-30
extern NSFont *defUniFont;

static int curProgNum;
CommandsController *commandsWindow = nil;
FNType mDirPathName[FNSize];

//controlTextDid
//textDidChange
// project: texshopsource351

// NSTextField [workinString setStringValue:[NSString stringWithUTF8String:wd_dir]];
// NSButton [workingString setTitle:[NSString stringWithUTF8String:wd_dir]];

//	strcpy(wd_dir, [[workinString stringValue] UTF8String]);
//	[ [workinString stringValue] getCharacters:bufU range:aRange];
//	bufU[aRange.length] = EOS;

int command_length(void) {
	int  len;
	NSString *comStr;

	if (commandsWindow != nil) {
		comStr = [commandsWindow->commandString stringValue];
		len = [comStr length];
	} else
		len = 0;
	return(len);
}

void AddComStringToComWin(char *com) {
	NSInteger strLen;
	NSRange aRange;
	NSText* fieldEditor;

	if (commandsWindow != nil && com != nil && com[0] != EOS) {
		[commandsWindow->commandString setStringValue:[NSString stringWithUTF8String:com]];
		strLen = [[commandsWindow->commandString stringValue] length];
		aRange.location = strLen;
		aRange.length = 0;
		fieldEditor = [commandsWindow->commandString currentEditor];
		[fieldEditor setSelectedRange:aRange];
	}
}

void SetWdLibFolder(char *str, int which) {
	if (str == nil || commandsWindow == nil)
		return;
	if (which == wdFolders) {
		strcpy(mDirPathName, wd_dir);
		strcpy(wd_dir, str);
		if (pathcmp(od_dir, mDirPathName) == 0)
			strcpy(od_dir, wd_dir);
		[commandsWindow->workingString setTitle:[NSString stringWithUTF8String:wd_dir]];
		if (!WD_Not_Eq_OD)
			[commandsWindow->outputString setTitle:[NSString stringWithUTF8String:""]];
		else
			[commandsWindow->outputString setTitle:[NSString stringWithUTF8String:od_dir]];
		WriteCedPreference();
		addToFolders(which, wd_dir);
	} else if (which == mlFolders) {
		strcpy(mor_lib_dir, str);
		[commandsWindow->morlibString setTitle:[NSString stringWithUTF8String:mor_lib_dir]];
		WriteCedPreference();
		addToFolders(2, mor_lib_dir);
	}
}

static void HideClanWinIcons(void) {
	[commandsWindow->fileInButton setHidden:YES];
}

void SetClanWinIcons(void) {
	int ProgNum;
	char *com, *s, progName[512+1];
	NSString *comStr;

	if (commandsWindow == nil)
		return;
	if (commandsWindow->commandString == nil) {
		HideClanWinIcons();
		return;
	}
	comStr = [commandsWindow->commandString stringValue];
	if (comStr == nil) {
		HideClanWinIcons();
		return;
	}
	if ([comStr length] == 0) {
		HideClanWinIcons();
		return;
	}
	strcpy(spareTier3, [comStr UTF8String]);
	com = spareTier3;
	s = strchr(com, ' ');
	if (s != NULL)
		*s = EOS;
	if (getAliasProgName(com, progName, 512)) {
		if (s != NULL)
			*s = ' ';
		s = NULL;
		com = progName;
	}
	if ((ProgNum=get_clan_prog_num(com, FALSE)) < 0) {
		if (strcmp(com, "bat") == 0 || strcmp(com, "batch") == 0 || strcmp(com, "dir") == 0 || strcmp(com, "list") == 0) {
			[commandsWindow->fileInButton setHidden:NO];
			[commandsWindow->fileInButton setTitle:[NSString stringWithUTF8String:"File In"]];
		} else
			HideClanWinIcons();
	} else {
		curProgNum = ProgNum;
		[commandsWindow->fileInButton setHidden:NO];
		if (curProgNum == EVAL || /*curProgNum == C_NNLA || */curProgNum == KIDEVAL)
			[commandsWindow->fileInButton setTitle:[NSString stringWithUTF8String:"Option"]];
		else
			[commandsWindow->fileInButton setTitle:[NSString stringWithUTF8String:"File In"]];
		if (option_flags[ProgNum] & T_OPTION && /*curProgNum != C_NNLA && */curProgNum != EVAL && curProgNum != KIDEVAL) {
//			ControlCTRL(win, I_Tiers, ShowCtrl, 0);
		} else {
//			ControlCTRL(win, I_Tiers, HideCtrl, 0);
		}
		if (option_flags[ProgNum] & SP_OPTION || option_flags[ProgNum] & SM_OPTION) {
//			ControlCTRL(win, I_Search, ShowCtrl, 0);
		} else {
//			ControlCTRL(win, I_Search, HideCtrl, 0);
		}
#ifdef COMMANDS_TEST
//		ControlCTRL(win, I_OutputFile, ShowCtrl, 0);
#else
//		ControlCTRL(win, I_OutputFile, HideCtrl, 0);
#endif // ! COMMANDS_TEST
	}
}

@implementation CommandsController

/*
+ (void)showWindow {
	commandsWindow = [[CommandsController alloc] initWithWindowNibName:@"Commands"];
//	[commandsWindow showWindow:nil];
//	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
//	[super showWindow:self];
}
*/
- (id)init {
	commandsWindow = nil;
	commandsFont = nil;
	if ((self = [super init])) {
		// initialization
	}
	return self;
//	return [super initWithWindowNibName:@"Commands"];
}

+ (void)createShowCommandsWindow
{
	extern char isMORXiMode;

	if (commandsWindow == nil) {
		commandsWindow = [[CommandsController alloc] initWithWindowNibName:@"Commands"];
		[commandsWindow showWindow:nil];
		isMORXiMode = FALSE;
	} else
		[commandsWindow showWindow:nil];
}

- (IBAction)createShowCommandsWindow:(id)sender
{
#pragma unused (sender)
	extern char isMORXiMode;

	if (commandsWindow == nil) {
		commandsWindow = [[CommandsController alloc] initWithWindowNibName:@"Commands"];
		[commandsWindow showWindow:nil];
		isMORXiMode = FALSE;
	} else
		[commandsWindow showWindow:nil];
}

// 2020-06-25 begin
- (int)poup_insertSorted:(const char **)temp atLoc:(int)k name:(const char *)newName {
	int i, j;

	for (i=0; i < k; i++) {
		if (strcmp(temp[i], newName) == 0)
			return(k);
		else if (strcmp(temp[i], newName) > 0)
			break;
	}
	for (j=k; j > i; j--)
		temp[j] = temp[j-1];
	temp[i] = newName;
	return(k+1);
}

- (void)createPopupProgMenu {
	NSInteger	pi, cnt;
	const char *temp[512];
	ALIASES_LIST *al;
	NSString *str;
	extern ALIASES_LIST *aliases;

/*
	if ((pi=[progsPopUp numberOfItems]) > 0) {
		[progsPopUp removeAllItems];
	}
	[progsPopUp insertItemWithTitle:@"Progs" atIndex:0];

	if ((pi=[progsPopUp numberOfItems]) > 0) {
		for (pi--; pi > 0; pi--)
			[progsPopUp removeItemAtIndex:(NSInteger)pi];
	}
*/
	cnt = 0;
	for (al=aliases; al != NULL; al=al->next_alias) {
		if (al->isPullDownC == 1) {
			cnt = [self poup_insertSorted:temp atLoc:cnt name:al->alias];
		}
	}
	for (pi=0; pi < MEGRASP; pi++) {
		if (clan_name[pi][0] != EOS)
			cnt = [self poup_insertSorted:temp atLoc:cnt name:clan_name[pi]];
	}
	for (pi=0; pi < cnt; pi++) {
		str = [NSString stringWithUTF8String:temp[pi]];
		[progsPopUp insertItemWithTitle:str atIndex:pi+1]; // 2020-02-20
	}
}
// 2020-06-25 end
/*
-(void)commandsChangeFont:(id)sender
{
	NSFont *oldFont, *newFont;

	oldFont = defUniFont; //commandString.font;
//	newFont = [[NSFontManager sharedFontManager] selectedFont];
	NSFontManager *fontManager = [NSFontManager sharedFontManager];
	newFont = [fontManager convertFont:oldFont];
	newFont = [sender selectedFont];

	NSFontPanel *fp = [NSFontPanel sharedFontPanel];
	newFont = [fp panelConvertFont:fontManager.selectedFont];
	fp = [fontManager fontPanel:NO];
	newFont = [fp panelConvertFont:fontManager.selectedFont];
//	NSDictionary *dict = fp->_selection;

	NSString *selectedFontName = [[sender selectedFont] displayName];
	//	NSFont *oldFont = [self font];
	//	NSFont *newFont = [sender convertFont:oldFont];
	//	[self setFont:newFont];
}
*/

- (void)updateCommandsFont {
	workingString.font = commandsFont;
	outputString.font = commandsFont;
	morlibString.font = commandsFont;
	versionString.font = commandsFont;
	commandString.font = commandsFont;

	NSFontManager *fontManager = [NSFontManager sharedFontManager];
	[fontManager setSelectedFont:commandsFont isMultiple:NO];
}

- (void)windowDidLoad {
	NSRect wFrame;
	CGFloat pointSize;
	NSString *familyName;
	NSFontManager *fontManager = [NSFontManager sharedFontManager];

	NSWindow *window = [self window];
//	[window setIdentifier:@"Command"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
	isSettingSize = NO;
	readAliases(1);
	func_init();
	InitOptions();

	commandsFont = defUniFont;
//	commandsFont = [workingString font];

	[self createPopupProgMenu]; // 2020-06-25
	[workingString setTitle:[NSString stringWithUTF8String:wd_dir]];
	if (!WD_Not_Eq_OD)
		[outputString setTitle:[NSString stringWithUTF8String:""]];
	else
		[outputString setTitle:[NSString stringWithUTF8String:od_dir]];
	[morlibString setTitle:[NSString stringWithUTF8String:mor_lib_dir]];
	[versionString setTitle:[NSString stringWithUTF8String:ced_version]];

	wdFolderBoxWindow = nil;
	mlFolderBoxWindow = nil;
	recallStBoxWindow = nil;
	curProgNum = 0;
	SetClanWinIcons();
	if (ClanWinSize.height != 0 && ClanWinSize.width != 0) { // 2020-01-30
		wFrame.origin.y = ClanWinSize.top;
		wFrame.origin.x = ClanWinSize.left;
		wFrame.size.height = ClanWinSize.height;
		wFrame.size.width = ClanWinSize.width;
		[window setFrame:wFrame display:false];
	}
	commandString.delegate = self;

	familyName = [commandsFont familyName];
	pointSize = (CGFloat)stickyFontSize;
	if (pointSize > 24.0)
		pointSize = 24.0;
	commandsFont = [NSFont fontWithName:familyName size:pointSize];
	[self updateCommandsFont];
	[fontManager setSelectedFont:commandsFont isMultiple:NO];

//	[commandString allowsEditingTextAttributes:YES];

//	NSText *fieldEditor = [commandString currentEditor];
//	[fieldEditor setUsesFontPanel:YES];
//	[fieldEditor setDelegate:self];

//	[fontManager setTarget:self];
//	[fontManager setAction:@selector(commandsChangeFont:)];
//	[fontManager orderFrontFontPanel:self];

//	[fontManager setDelegate:self];
//	NSFontPanel *fp = [NSFontPanel sharedFontPanel];
//	[fp makeKeyAndOrderFront:self];
//	[[self window] makeFirstResponder:self];

}

/* Reopen the line panel when the app's persistent state is restored.
 */
+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
//	completionHandler([[(Controller *)[NSApp delegate] commandsController] window], NULL);
}

- (IBAction)commandWdStClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSUInteger i, num, len;
	CGSize textSize;
	NSDictionary *attributes;
	NSDictionary *addedObject;

	if (wdFolderBoxWindow == nil) {
		wdFolderBoxWindow = [[ListBoxController alloc] initWithWindowNibName:@"ListBox"];
		wdFolderBoxWindow->whichBox = wdFolders;
		[wdFolderBoxWindow showWindow:nil];
	} else {
		attributes = @{NSFontAttributeName:defUniFont};
		len = 0;
		num = [[wdFolderBoxWindow->myTableArray arrangedObjects] count];
//		[wdFolderBoxWindow->myTableArray removeObjectAtArrangedObjectIndex:2];
		NSIndexSet *range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
		[wdFolderBoxWindow->myTableArray removeObjectsAtArrangedObjectIndexes:range];
		for (i=0; true; i++) {
			addedObject = get_next_obj_folder(&i, wdFolders);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[wdFolderBoxWindow->myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			wdFolderBoxWindow->myTableView.tableColumns[0].width = len;
		num = [[wdFolderBoxWindow->myTableArray arrangedObjects] count];
		if (0 < num)
			[wdFolderBoxWindow->myTableArray setSelectionIndex:0];
		[wdFolderBoxWindow showWindow:nil];
	}
}

- (IBAction)commandOdStClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"CommandsController: commandOdStClicked\n");
}

- (IBAction)commandMLStClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSUInteger i, num, len;
	CGSize textSize;
	NSDictionary *attributes;
	NSDictionary *addedObject;

	if (mlFolderBoxWindow == nil) {
		mlFolderBoxWindow = [[ListBoxController alloc] initWithWindowNibName:@"ListBox"];
		mlFolderBoxWindow->whichBox = mlFolders;
		[mlFolderBoxWindow showWindow:nil];
	} else {
		attributes = @{NSFontAttributeName:defUniFont};
		len = 0;
		num = [[mlFolderBoxWindow->myTableArray arrangedObjects] count];
//		[mlFolderBoxWindow->myTableArray removeObjectAtArrangedObjectIndex:2];
		NSIndexSet *range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
		[mlFolderBoxWindow->myTableArray removeObjectsAtArrangedObjectIndexes:range];
		for (i=0; true; i++) {
			addedObject = get_next_obj_folder(&i, mlFolders);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[mlFolderBoxWindow->myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			mlFolderBoxWindow->myTableView.tableColumns[0].width = len;
		num = [[mlFolderBoxWindow->myTableArray arrangedObjects] count];
		if (0 < num)
			[mlFolderBoxWindow->myTableArray setSelectionIndex:0];
		[mlFolderBoxWindow showWindow:nil];
	}
}

- (IBAction)commandRecallClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSUInteger i, num, len;
	CGSize textSize;
	NSDictionary *attributes;
	NSDictionary *addedObject;

	if (recallStBoxWindow == nil) {
		recallStBoxWindow = [[ListBoxController alloc] initWithWindowNibName:@"ListBox"];
		recallStBoxWindow->whichBox = recallStr;
		[recallStBoxWindow showWindow:nil];
	} else {
		attributes = @{NSFontAttributeName:defUniFont};
		len = 0;
		num = [[recallStBoxWindow->myTableArray arrangedObjects] count];
//		[recallStBoxWindow->myTableArray removeObjectAtArrangedObjectIndex:2];
		NSIndexSet *range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
		[recallStBoxWindow->myTableArray removeObjectsAtArrangedObjectIndexes:range];
		for (i=0; true; i++) {
			addedObject = get_next_obj_command(&i);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[recallStBoxWindow->myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			recallStBoxWindow->myTableView.tableColumns[0].width = len;
		num = [[recallStBoxWindow->myTableArray arrangedObjects] count];
		i = getRecall_curCommand();
		if (i < num)
			[recallStBoxWindow->myTableArray setSelectionIndex:i];
		[recallStBoxWindow showWindow:nil];
	}
}

- (IBAction)commandWorkingClicked:(NSButton *)sender
{
#pragma unused (sender)

	strcpy(mDirPathName, wd_dir);
	LocateDir("Please locate working directory",wd_dir,true);
	if (pathcmp(od_dir, mDirPathName) == 0)
		strcpy(od_dir, wd_dir);
	[workingString setTitle:[NSString stringWithUTF8String:wd_dir]];
	if (!WD_Not_Eq_OD)
		[outputString setTitle:[NSString stringWithUTF8String:""]];
	else
		[outputString setTitle:[NSString stringWithUTF8String:od_dir]];
	WriteCedPreference();
	addToFolders(1, wd_dir);
}

- (IBAction)commandOutputClicked:(NSButton *)sender
{
#pragma unused (sender)

	LocateDir("Please locate output directory",od_dir,false);
	if (!WD_Not_Eq_OD)
		[outputString setTitle:[NSString stringWithUTF8String:""]];
	else
		[outputString setTitle:[NSString stringWithUTF8String:od_dir]];
	WriteCedPreference();
}

- (IBAction)commandMorLibClicked:(NSButton *)sender
{
#pragma unused (sender)

	char *s;
	int  len;

	strcpy(mDirPathName, mor_lib_dir);
	len = strlen(mDirPathName) - 1;
	if (len >= 0 && mDirPathName[len] == PATHDELIMCHR)
		mDirPathName[len] = EOS;
	s = strrchr(mDirPathName, PATHDELIMCHR);
	if (s != NULL)
		*s = EOS;
	if (LocateDir("Please locate mor library directory",mDirPathName,false))
		strcpy(mor_lib_dir, mDirPathName);
	[morlibString setTitle:[NSString stringWithUTF8String:mor_lib_dir]];
	WriteCedPreference();
	addToFolders(2, mor_lib_dir);
}

- (IBAction)commandFileInClicked:(NSButton *)sender // 2019-12-04
{
#pragma unused (sender)
	[FileInController fileInDialog];
}

/*
 NSString *userinput = @"Here is a string";
 NSString *newstring = @"";
 newstring = [userinput substringWithRange:NSMakeRange(2, 4)];
 NSLog(newstring);

 Result is "re i" in the console log.
*/
- (IBAction)commandProgClicked:(NSPopUpButton *)sender
{
	NSInteger strLen;
	NSString *menuStr;
	NSRange aRange;
	NSText* fieldEditor;

	strLen = [sender indexOfSelectedItem] + 1;
	if (strLen > 1) {
		menuStr = [sender titleOfSelectedItem];
		menuStr = [menuStr stringByAppendingString:@" "];
//		commandString.stringValue = menuStr;
		[commandString setStringValue:menuStr];

		strLen = [[commandString stringValue] length];
		aRange.location = strLen;
		aRange.length = 0;
		fieldEditor = [commandString currentEditor];
		[fieldEditor setSelectedRange:aRange];
		SetClanWinIcons();
	}
	[progsPopUp selectItemAtIndex:0]; // 2020-02-20 dinamically update Commands->progs menu

}

// 2020-06-11 beg
- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
	NSLog(@"Selector method is (%@)", NSStringFromSelector( commandSelector ) );
	if (commandSelector == @selector(insertNewline:)) {
		return NO;
	} else if (commandSelector == @selector(moveUp:)) { // 2020-06-12
		RecallCommand(up_arrow);
		SetClanWinIcons();
		return YES;
	} else if (commandSelector == @selector(moveDown:)) {// 2020-06-12
		RecallCommand(down_arrow);
		SetClanWinIcons();
		return YES;
	} else if (commandSelector == @selector(cancelOperation:)) {
		return NO;
	} else if (commandSelector == @selector(cancel:)) {// 2020-06-12
		[commandString setStringValue:@""];
		SetClanWinIcons();
		return YES;
	} else
		return NO;
/*
cancelOperation:
capitalizeWord:
centerSelectionInVisibleArea:
changeCaseOfLetter:
complete:
deleteBackward:
deleteBackwardByDecomposingPreviousCharacter:
deleteForward:
deleteToBeginningOfLine:
deleteToBeginningOfParagraph:
deleteToEndOfLine:
deleteToEndOfParagraph:
deleteToMark:
deleteWordBackward:
deleteWordForward:
doCommandBySelector:
indent:
insertBacktab:
insertContainerBreak:
insertDoubleQuoteIgnoringSubstitution:
insertLineBreak:
insertNewline:
insertNewlineIgnoringFieldEditor:
insertParagraphSeparator:
insertSingleQuoteIgnoringSubstitution:
insertTab:
insertTabIgnoringFieldEditor:
insertText:
lowercaseWord:
makeBaseWritingDirectionLeftToRight:
makeBaseWritingDirectionNatural:
makeBaseWritingDirectionRightToLeft:
makeTextWritingDirectionLeftToRight:
makeTextWritingDirectionNatural:
makeTextWritingDirectionRightToLeft:
moveBackward:
moveBackwardAndModifySelection:
moveDown:
moveDownAndModifySelection:
moveForward:
moveForwardAndModifySelection:
moveLeft:
moveLeftAndModifySelection:
moveParagraphBackwardAndModifySelection:
moveParagraphForwardAndModifySelection:
moveRight:
moveRightAndModifySelection:
moveToBeginningOfDocument:
moveToBeginningOfDocumentAndModifySelection:
moveToBeginningOfLine:
moveToBeginningOfLineAndModifySelection:
moveToBeginningOfParagraph:
moveToBeginningOfParagraphAndModifySelection:
moveToEndOfDocument:
moveToEndOfDocumentAndModifySelection:
moveToEndOfLine:
moveToEndOfLineAndModifySelection:
moveToEndOfParagraph:
moveToEndOfParagraphAndModifySelection:
moveToLeftEndOfLine:
moveToLeftEndOfLineAndModifySelection:
moveToRightEndOfLine:
moveToRightEndOfLineAndModifySelection:
moveUp:
moveUpAndModifySelection:
moveWordBackward:
moveWordBackwardAndModifySelection:
moveWordForward:
moveWordForwardAndModifySelection:
moveWordLeft:
moveWordLeftAndModifySelection:
moveWordRight:
moveWordRightAndModifySelection:
pageDown:
pageDownAndModifySelection:
pageUp:
pageUpAndModifySelection:
quickLookPreviewItems:
scrollLineDown:
scrollLineUp:
scrollPageDown:
scrollPageUp:
scrollToBeginningOfDocument:
scrollToEndOfDocument:
selectAll:
selectLine:
selectParagraph:
selectToMark:
selectWord:
setMark:
swapWithMark:
transpose:
transposeWords:
uppercaseWord:
yank:
*/
}
// 2020-06-11 end

// ANY text change

- (void)controlTextDidChange:(NSNotification *)notification
{
	if ( [ notification object ] != commandString ) {
		return;
	}
	SetClanWinIcons();

//	strcpy(spareTier3, [comStr UTF8String]);
/*
	int i;
	unichar *bufU;
	NSRange aRange;

	i = [oldString length];
	bufU = (unichar *)malloc((i*sizeof(unichar))+1);
	if (bufU == NULL)
		return;
	aRange.location = 0;
	aRange.length = i;
	[oldString getCharacters:bufU range:aRange];
	bufU[i] = 0;
*/

/*
	NSText *fieldEditor = [[notification userInfo] objectForKey: @"NSFieldEditor"];
	if (!fieldEditor)
		return;
	NSString *oldString = [fieldEditor string];
	NSRange selectedRange = [fieldEditor selectedRange];
	NSString *newString;

	if (g_shouldFilter == kMacJapaneseFilterMode) {
		newString = filterBackslashToYen(oldString);
		[fieldEditor setString: newString];
		[fieldEditor setSelectedRange: selectedRange];
	} else if (g_shouldFilter == kOtherJapaneseFilterMode) {
		newString = filterYenToBackslash(oldString);
		[fieldEditor setString: newString];
		[fieldEditor setSelectedRange: selectedRange];
	}
*/
}

/* If the user enters a line specification and hits return, we want to order the panel out if successful.  Hence this extra action method.
 */
- (IBAction)commandTextChanged:(id)sender {
#pragma unused (sender)
	int i;

	i = 12;
}

- (IBAction)commandRunClicked:(NSButton *)sender
{
#pragma unused (sender)
/*
	NSString *comStr;
	NSInteger strLen;
	NSInteger i, comLen;
	NSRange aRange;

	i = sizeof(wchar_t);
	i = sizeof(unichar);
	comStr = [commandString stringValue];
	comLen = [comStr length];
	aRange.location = 0;
	aRange.length = comLen;
	dispatch_async(dispatch_get_global_queue(0, 0), ^{
		unichar *bufU;

		bufU = (unichar *)malloc((comLen*sizeof(unichar))+1);
		if (bufU == NULL)
			return;
		[comStr getCharacters:bufU range:aRange];
		bufU[comLen] = 0;
		RunCommand(bufU);
		free(bufU);
	});
	[commandString setStringValue:@""];
	strLen = [[commandString stringValue] length];
	aRange.location = strLen;
	aRange.length = 0;
	[[commandString currentEditor] setSelectedRange:aRange];
*/
	NSString *comStr;
	NSInteger strLen;
	NSRange aRange;

	NSInteger i;
	i = sizeof(wchar_t);
	i = sizeof(unichar);

	comStr = [commandString stringValue];

//	dispatch_async(dispatch_get_main_queue(), ^(void){

//  DISPATCH_QUEUE_PRIORITY_HIGH
//	DISPATCH_QUEUE_PRIORITY_DEFAULT
//	DISPATCH_QUEUE_PRIORITY_LOW
//	DISPATCH_QUEUE_PRIORITY_BACKGROUND: Int32
//	dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(void){
		RunCommand(comStr);
//		dispatch_async(dispatch_get_main_queue(), ^(void){
			//Run UI Updates
//		});
//	});


//[[commandString currentEditor] replaceCharactersInRange:aRange withString:@""];
//[commandString setStringValue:[NSString stringWithFormat: @"%@ %@", [commandString stringValue], [sender title]]];
//commandString.stringValue = [commandString.stringValue stringByAppendingString:sender.title];
//commandString.stringValue = [NSString stringWithFormat:@"%@ %@", commandString.stringValue, sender.title];
	[commandString setStringValue:@""];
	strLen = [[commandString stringValue] length];
	aRange.location = strLen;
	aRange.length = 0;
	[[commandString currentEditor] setSelectedRange:aRange];

}

- (IBAction)commandVerStClicked:(NSButton *)sender
{
#pragma unused (sender)
	AboutClanController *aboutClanWindow = [[AboutClanController alloc] initWithWindowNibName:@"AboutClan"];
	[aboutClanWindow showWindow:nil];
}


// awakeFromNib is called when this object is done being unpacked from the nib file;
// at this point, we can do any needed initialization before turning app control over to the user
- (void)awakeFromNib
{
	NSLog(@"CommandsController: awakeFromNib\n");
    // We don't actually need to do anything here, so it's empty
}

// Handling the Help menu:
// -----------------------
// It used to be that we had a routine here that opened our ReadMe.html help file using Help Viewer via
// NSWorkspace's -openFile:withApplication:, but now we do things the "modern" way.  This means that we
// let Cocoa handle things automatically for us.  The only thing we need is a folder containing our help
// files that sits in the Resources folder inside the SimpleCocoaApp bundle, a new meta tag in our help title
// page (the "AppleTitle" tag), and a few Info.plist keys (see the "Expert" view of Targets -> Application Settings).
// The two Info.plist keys needed are CFBundleHelpBookFolder and CFBundleHelpBookName.  Once that is done,
// Help Viewer will automatically open the Help page when the Help menu item is selected, etc.  No code is needed!


- (void)dealloc {
	[super dealloc]; // NSWindowController deallocates all the nib objects
}
@end

@implementation CommandsController(Delegation)

- (void)windowDidBecomeKey:(NSNotification *)notification {
#pragma unused (notification)
	NSFontManager *fontManager = [NSFontManager sharedFontManager];
	if (commandsFont == nil)
		commandsFont = [workingString font];
	if (commandsFont != nil)
		[fontManager setSelectedFont:commandsFont isMultiple:NO];
}

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	if (!isSettingSize) {   // There is potential for recursion, but typically this is prevented in NSWindow which doesn't call this method if the frame doesn't change. However, just in case...
		isSettingSize = YES;
		NSRect wFrame;
		NSRect workingStringFrame;
		NSRect workingButtonFrame;
		NSRect outputStringFrame;
		NSRect outputButtonFrame;
		NSRect morlibStringFrame;
		NSRect morLibButtonFrame;
		NSRect progsPopUpFrame;
		NSRect commandStringFrame;
		NSRect runButtonFrame;
		NSRect recallButtonFrame;
		NSRect versionStringFrame;

		wFrame = self.window.frame;

		workingStringFrame = workingString.frame;
		workingButtonFrame = workingButton.frame;
		outputStringFrame  = outputString.frame;
		outputButtonFrame  = outputButton.frame;
		morlibStringFrame  = morlibString.frame;
		morLibButtonFrame  = morLibButton.frame;
		progsPopUpFrame    = progsPopUp.frame;
		commandStringFrame = commandString.frame;
		runButtonFrame     = runButton.frame;
		recallButtonFrame  = recallButton.frame;
		versionStringFrame = versionString.frame;
		
		recallButtonFrame.origin.y = -2.5;
		versionStringFrame.origin.y = 1.0;
		runButtonFrame.origin.y = -2.5;
		runButtonFrame.origin.x = wFrame.size.width - runButtonFrame.size.width - 18;

		commandStringFrame.origin.y = recallButtonFrame.size.height;
		commandStringFrame.size.width = wFrame.size.width - 7;
		commandStringFrame.size.height = wFrame.size.height -
										workingButtonFrame.size.height -
										outputButtonFrame.size.height -
										morLibButtonFrame.size.height -
										(progsPopUpFrame.size.height * 2) -
										recallButtonFrame.size.height - 10;
		if (commandStringFrame.size.height < 0)
			commandStringFrame.size.height = 0;

		workingStringFrame.size.width = wFrame.size.width - workingButtonFrame.size.width;
		if (workingStringFrame.size.width < 0)
			workingStringFrame.size.width = 0;
		outputStringFrame.size.width  = wFrame.size.width - outputButtonFrame.size.width;
		if (outputStringFrame.size.width < 0)
			outputStringFrame.size.width = 0;
		morlibStringFrame.size.width  = wFrame.size.width - morLibButtonFrame.size.width;
		if (morlibStringFrame.size.width < 0)
			morlibStringFrame.size.width = 0;

		workingString.frame = workingStringFrame;
		outputString.frame  = outputStringFrame;
		morlibString.frame  = morlibStringFrame;
		commandString.frame = commandStringFrame;
		runButton.frame     = runButtonFrame;
		recallButton.frame  = recallButtonFrame;
		versionString.frame = versionStringFrame;
/*
		if (ClanWinSize.top != wFrame.origin.y || ClanWinSize.left != wFrame.origin.x ||
			ClanWinSize.height != wFrame.size.height || ClanWinSize.width != wFrame.size.width) {
			ClanWinSize.top = wFrame.origin.y;
			ClanWinSize.left = wFrame.origin.x;
			ClanWinSize.height = wFrame.size.height;
			ClanWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
*/
		isSettingSize = NO;
	}
}

- (void)windowDidMove:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"CommandsController: windowDidMove\n");
/*
	NSRect wFrame = self.window.frame;
	if (ClanWinSize.top != wFrame.origin.y || ClanWinSize.left != wFrame.origin.x ||
		ClanWinSize.height != wFrame.size.height || ClanWinSize.width != wFrame.size.width) {
		ClanWinSize.top = wFrame.origin.y;
		ClanWinSize.left = wFrame.origin.x;
		ClanWinSize.height = wFrame.size.height;
		ClanWinSize.width = wFrame.size.width;
		WriteCedPreference();
	}
*/
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"CommandsController: windowWillClose\n");
}

@end

