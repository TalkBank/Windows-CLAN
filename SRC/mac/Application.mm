
#import <AppKit/AppKit.h>
#import "DocumentController.h"
#import "Document.h"
#import "ced.h"
#import "CommandsController.h"
#import "AVController.h"
#import "PictController.h"
#import "WalkerController.h"

extern struct DefWin ClanWinSize; // 2020-01-30
extern WalkerController *WalkerControllerWindow;
extern struct PBCs PBC;
extern struct DefWin MovieWinSize;
extern struct DefWin PictWinSize; // 2021-05-01


@interface CLANApplication : NSApplication

//- (void)terminate:(id)sender;
//- (void)sendEvent:(NSEvent *)anEvent;

@end

@implementation CLANApplication

- (id)init
{
	int j;

    self = [super init];
    if (self) {
		j = 0;
        // Initialization code here.
    }
    
    return self;
}

- (void)terminate:(id)sender;
{
	unCH numS[256];
	NSUInteger len;
	NSString *fieldStr;
	NSRect wFrame;

	NSLog(@"CLANApplication: terminate\n");
	if (commandsWindow != nil) {
		wFrame = commandsWindow.window.frame;  // 2020-01-30
		if (ClanWinSize.top != wFrame.origin.y || ClanWinSize.left != wFrame.origin.x ||
			ClanWinSize.height != wFrame.size.height || ClanWinSize.width != wFrame.size.width) {
			ClanWinSize.top = wFrame.origin.y;
			ClanWinSize.left = wFrame.origin.x;
			ClanWinSize.height = wFrame.size.height;
			ClanWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
		[commandsWindow release];
		commandsWindow = nil;
	}
	if (thePict != nil) {
		wFrame = thePict.window.frame;
		if (PictWinSize.top != wFrame.origin.y || PictWinSize.left != wFrame.origin.x ||
			PictWinSize.height != wFrame.size.height || PictWinSize.width != wFrame.size.width) {
			PictWinSize.top = wFrame.origin.y;
			PictWinSize.left = wFrame.origin.x;
			PictWinSize.height = wFrame.size.height;
			PictWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
		[thePict close];
		[thePict release];
		thePict = nil;
	}
	if (AVMediaPlayer != nil) {
		wFrame = AVMediaPlayer.window.frame;
		if (MovieWinSize.top != wFrame.origin.y || MovieWinSize.left != wFrame.origin.x ||
			MovieWinSize.height != wFrame.size.height || MovieWinSize.width != wFrame.size.width) {
			MovieWinSize.top = wFrame.origin.y;
			MovieWinSize.left = wFrame.origin.x;
			MovieWinSize.height = wFrame.size.height;
			MovieWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
		if (AVMediaPlayer->audioTimer != nil) {
			[AVMediaPlayer->audioTimer invalidate];
			AVMediaPlayer->audioTimer = nil;
		}
		if (AVMediaPlayer->audioPlayer != nil) {
			[AVMediaPlayer->audioPlayer release];
			AVMediaPlayer->audioPlayer = nil;
		}
		if (AVMediaPlayer->timeObserverToken != nil) {
			[AVMediaPlayer->playerView.player removeTimeObserver:AVMediaPlayer->timeObserverToken];
			AVMediaPlayer->timeObserverToken = nil;
		}
		if (AVMediaPlayer->isPlayerItemObserverSet == YES) {
			AVMediaPlayer->playerItemContext = 0;
			[AVMediaPlayer->playerItem removeObserver:AVMediaPlayer forKeyPath:@"status" context:&AVMediaPlayer->playerItemContext];
			AVMediaPlayer->isPlayerItemObserverSet = NO;
		}
		[AVMediaPlayer close];
		[AVMediaPlayer release];
		AVMediaPlayer = nil;
	}
	if (WalkerControllerWindow != nil) {
		fieldStr = [WalkerControllerWindow->WalkLengthField stringValue];
		len = [fieldStr length];
		[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		PBC.step_length = uS.atol(numS);

		fieldStr = [WalkerControllerWindow->LoopNumberField stringValue];
		len = [fieldStr length];
		[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		PBC.LoopCnt = uS.atol(numS);

		fieldStr = [WalkerControllerWindow->BackspaceField stringValue];
		len = [fieldStr length];
		[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		PBC.backspace = uS.atol(numS);

		fieldStr = [WalkerControllerWindow->WalkPauseLenField stringValue];
		len = [fieldStr length];
		[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		PBC.pause_len = uS.atol(numS);

		fieldStr = [WalkerControllerWindow->PlaybackSpeedField stringValue];
		len = [fieldStr length];
		[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		PBC.speed = uS.atol(numS);

		WriteCedPreference();
	}
/*
    
    NSArray *myDocuments = [[TSDocumentController sharedDocumentController]  documents];
    id obj;
    i = 0;
    while (i < [myDocuments count]) {
        j = i;
        obj = [myDocuments objectAtIndex:j];
        i++;
        skip = [(TSDocument *)obj skipTextWindow];
        // NSLog(@"considered");
        if (skip) {
            // NSLog(@"will close");
// Yusuke Terada patch to avoid crash at close
            id pdfWindow = [(TSDocument *)obj pdfWindow];
            id pdfKitWindow = [(TSDocument *)obj pdfKitWindow];
            
            if (pdfWindow && [pdfWindow respondsToSelector:@selector(isVisible)] && [pdfWindow isVisible] && [pdfWindow respondsToSelector:@selector(performClose:)])[pdfWindow performClose:self];
            else if (pdfKitWindow && [pdfKitWindow respondsToSelector:@selector(isVisible)] && [pdfKitWindow isVisible] && [pdfKitWindow respondsToSelector:@selector(performClose:)])
                [pdfKitWindow performClose:self];
// end of patch
            // [(TSDocument *)obj close];
            // NSLog(@"called close");
        }
    }
 
 */

    [super terminate:sender];
}

- (void)sendEvent:(NSEvent *)theEvent
{ // 2019-10-15
	int j;
	NSWindow *window;

	window = [theEvent window];

	if (([theEvent type] == NSKeyDown) && ([theEvent modifierFlags] & NSCommandKeyMask)) {
		j = 0;
	}
	[super sendEvent:theEvent];
}

@end
