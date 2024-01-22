
#import <Cocoa/Cocoa.h>

@interface SelectF5Controller : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate>
{
	IBOutlet NSButton *bulletOnEveryLine;
	IBOutlet NSButton *bulletOnEveryTier;

	IBOutlet NSButton *OKbutton;
	IBOutlet NSButton *CancelButton;

	NSWindow *textWin;
}

- (IBAction)BulletOnEveryLineClicked:(id)sender;
- (IBAction)BulletOnEveryTierClicked:(id)sender;

- (IBAction)OKClicked:(id)sender;
- (IBAction)CancelClicked:(id)sender;

+ (void)SelectF5Dialog:(NSWindow *)textWin;

@end
