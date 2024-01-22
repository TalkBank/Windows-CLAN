

#import <Cocoa/Cocoa.h>

@interface SelectMediaController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate> {
	IBOutlet NSArrayController *FilesDiskArray;
	IBOutlet NSTableView *FilesDiskView;
	FNType *mediaFName;
	DocumentWindowController *docWinController;
}


- (IBAction)cancelButtonClicked:(id)sender;
- (IBAction)okButtonClicked:(id)sender;
- (IBAction)diskTableClicked:(id)sender;

+ (void)SelectMediaDialog:(DocumentWindowController *)docWinController str:(FNType *)strPtr;

@end

