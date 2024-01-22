

#import <Cocoa/Cocoa.h>

enum {
	wdFolders=1,
	mlFolders,
	recallStr
} ;

@interface ListBoxController : NSWindowController <NSTableViewDelegate> {
	IBOutlet NSButton *deleteButton;
	IBOutlet NSButton *selectButton;
@public
	IBOutlet NSArrayController *myTableArray;
	IBOutlet NSTableView *myTableView;
	short whichBox;
}

- (IBAction)deleteButtonClicked:(id)sender;
- (IBAction)selectButtonClicked:(id)sender;



@end

