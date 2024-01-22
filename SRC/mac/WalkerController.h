

#import <Cocoa/Cocoa.h>

struct PBCs {
	char enable;
	long LoopCnt,
		 speed,
		 backspace,
		 step_length,
		 pause_len;
} ;

@interface WalkerController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate> {
@public
	IBOutlet NSTextField *WalkLengthField;
	IBOutlet NSTextField *LoopNumberField;
	IBOutlet NSTextField *BackspaceField;
	IBOutlet NSTextField *WalkPauseLenField;
	IBOutlet NSTextField *PlaybackSpeedField;
}

- (IBAction)WalkLengthFieldChanged:(id)sender;
- (IBAction)LoopNumberFieldChanged:(id)sender;
- (IBAction)BackspaceFieldChanged:(id)sender;
- (IBAction)WalkPauseLenFieldChanged:(id)sender;
- (IBAction)PlaybackSpeedFieldChanged:(id)sender;

- (IBAction)createWalkerControllerWindow:(id)sender;

@end

extern WalkerController *WalkerControllerWindow;

