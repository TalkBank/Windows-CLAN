

#import <Cocoa/Cocoa.h>

@interface UserStrController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate> {
	IBOutlet NSTextField *labelField;
	IBOutlet NSTextField *strField;
	unCH *strPtr;
	NSUInteger max;
	NSString *info;
	DocumentWindowController *docWinController;
}

- (IBAction)strFieldChanged:(id)sender;
- (IBAction)cancelButtonClicked:(id)sender;
- (IBAction)doneButtonClicked:(id)sender;

+(void)userStrDialog:(DocumentWindowController *)docWinController label:(NSString *)info str:(unCH *)strPtr max:(NSUInteger)max;

@end

