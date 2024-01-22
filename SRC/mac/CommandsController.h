
#import <Cocoa/Cocoa.h>
#import "ListBoxController.h"
#import "FileInController.h"

@interface CommandsController : NSWindowController <NSLayoutManagerDelegate, NSTextFieldDelegate, NSWindowDelegate> {
	IBOutlet NSButton *workingButton;
	IBOutlet NSButton *outputButton;
	IBOutlet NSButton *morLibButton;

	IBOutlet NSPopUpButton *progsPopUp;
	IBOutlet NSButton *runButton;

	IBOutlet NSButton *recallButton;
	IBOutlet NSButton *versionString;
	
	BOOL isSettingSize;
@public
	IBOutlet NSButton *workingString;
	IBOutlet NSButton *outputString;
	IBOutlet NSButton *morlibString;

	IBOutlet NSButton *fileInButton;

	IBOutlet NSTextField *commandString;
	ListBoxController *wdFolderBoxWindow;
	ListBoxController *mlFolderBoxWindow;
	ListBoxController *recallStBoxWindow;

	NSFont *commandsFont;
}

+ (void)createShowCommandsWindow;

- (IBAction)createShowCommandsWindow:(id)sender;

- (IBAction)commandWdStClicked:(id)sender;
- (IBAction)commandOdStClicked:(id)sender;
- (IBAction)commandMLStClicked:(id)sender;

- (IBAction)commandWorkingClicked:(id)sender;
- (IBAction)commandOutputClicked:(id)sender;
- (IBAction)commandMorLibClicked:(id)sender;

- (IBAction)commandProgClicked:(id)sender;
- (IBAction)commandTextChanged:(id)sender;	// to receive notification from contentTextView
- (IBAction)commandRunClicked:(id)sender;
- (IBAction)commandRecallClicked:(id)sender;
- (IBAction)commandVerStClicked:(id)sender;

- (IBAction)commandFileInClicked:(id)sender;

- (void)createPopupProgMenu;
- (int)poup_insertSorted:(const char **)temp atLoc:(int)k name:(const char *)newName;

- (void)updateCommandsFont;

 @end

extern void SetClanWinIcons(void);
extern CommandsController *commandsWindow;


