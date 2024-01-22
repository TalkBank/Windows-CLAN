//  AppDelegate.mm
#import "DocumentWinController.h"
#import "CommandsController.h"
#import "Controller.h"

extern DocumentWindowController *getDocWinController(NSWindow *win);
extern AVController *AVMediaPlayer;
extern BOOL DefClan;

@interface AppDelegate : NSObject <NSApplicationDelegate> {
}
@end

@interface AppDelegate ()
@end

@implementation AppDelegate

- (void)changeFont:(id)sender
{
	NSFont *font;
	NSWindow *win = [[NSApplication sharedApplication] mainWindow];
	Document *cDoc;
	NSString *familyName;
	CGFloat oldFontHeight, newFontHeight, pointSize;

	NSLog(@"changeFont\n");

	win = [[NSApplication sharedApplication] mainWindow];
//	win = [[NSApplication sharedApplication] keyWindow];
//	[[[NSApplication sharedApplication] mainWindow] contentView];

	DocumentWindowController *winCtrl = getDocWinController(win);
	font = nil;
	cDoc = nil;
	if (winCtrl != nil) {
 		cDoc = [winCtrl document];
		font = [cDoc docFont];
	} else if (commandsWindow != nil) {
		font = commandsWindow->commandsFont;
	}


	if (font != nil) {
		font = [sender convertFont:font];
		pointSize = [font pointSize];
		stickyFontSize = (long)pointSize;
		NSLog(@"%@", font);
		if (winCtrl != nil && cDoc != nil) {
			oldFontHeight = [cDoc fontHeight];
			newFontHeight = [[winCtrl myLayoutManager] defaultLineHeightForFont:font];
			if (oldFontHeight != newFontHeight) {
				[cDoc setDocFont:font];
				[cDoc setFontHeight:newFontHeight];
				[winCtrl->lowerView setLowerFont:font];
				[winCtrl->lowerView setLowerFontHeight:newFontHeight];
				[winCtrl setupWindowForDocument];
			}
		}
		if (commandsWindow != nil) {
			familyName = [font familyName];
			if (pointSize > 24.0)
				pointSize = 24.0;
			commandsWindow->commandsFont = [NSFont fontWithName:familyName size:pointSize];
			[commandsWindow updateCommandsFont];
		}
		WriteCedPreference();
	}
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename {
#pragma unused (sender, filename)
	NSLog(@"application:openFile\n");
	return(NO);
}

- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls {
#pragma unused (application, urls)
	NSLog(@"application:openURLs\n");
}

- (void)applicationWillTerminate:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"applicationWillTerminate\n");
	// Insert code here to tear down your application
}


// above DOESN'T work, below DOES work


- (BOOL)applicationOpenUntitledFile:(NSApplication *)sender {
#pragma unused (sender)
	NSLog(@"applicationOpenUntitledFile\n");
	return(NO); // YES - mean Done Here, NO - mean do it in the system
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
#pragma unused (sender)
	NSLog(@"applicationShouldOpenUntitledFile\n");
	if (DefClan) {
		[CommandsController createShowCommandsWindow];
		return(NO);  // YES - means open on start-up, NO - means do not open
	} else
		return(YES); // YES - means open on start-up, NO - means do not open
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"applicationDidFinishLaunching\n");

	NSFontManager *fontManager = [NSFontManager sharedFontManager];
	[fontManager setDelegate:self];
	[fontManager setTarget:self];

	// Insert code here to initialize your application
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
#pragma unused (sender)
	short err = 0;
	extern AEEventHandlerUPP hOpenFilePos;

	NSLog(@"applicationShouldTerminate\n");
	err = AERemoveEventHandler(758934755, 0, hOpenFilePos, false);
	return(NSTerminateNow); // NSTerminateNow - YES, NSTerminateCancel - NO, NSTerminateLater
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"applicationDidBecomeActive\n");
}

- (void)applicationDidResignActive:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"applicationDidResignActive\n");
	if (AVMediaPlayer != nil) {
		if (AVMediaPlayer->playMode != stopped)
			[AVMediaPlayer StopAV];
	}
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
#pragma unused (sender)
	NSLog(@"applicationShouldTerminateAfterLastWindowClosed\n");
	return(NO); // YES - means quit app, NO - means do not quit
}

@end
