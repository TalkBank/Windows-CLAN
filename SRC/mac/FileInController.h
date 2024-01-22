

#import <Cocoa/Cocoa.h>

@interface FileInController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate> {
	IBOutlet NSPopUpButton *foldersPopUp;
	IBOutlet NSButton *showOnlyChaCexButton;
	IBOutlet NSButton *filePathString;
	IBOutlet NSButton *openButtonString;

	IBOutlet NSArrayController *FilesDiskArray;
	IBOutlet NSTableView *FilesDiskView;

	IBOutlet NSArrayController *FilesUserArray;
	IBOutlet NSTableView *FilesUserView;

@public
	BOOL isCallKideval;
	BOOL isCallEval;
}

- (IBAction)foldersButtonClicked:(id)sender;
- (IBAction)removeButtonClicked:(id)sender;
- (IBAction)clearButtonClicked:(id)sender;
- (IBAction)doneButtonClicked:(id)sender;
- (IBAction)showOnlyChaCexClicked:(id)sender;
- (IBAction)desktopButtonClicked:(id)sender;
- (IBAction)downloadsButtonClicked:(id)sender;
- (IBAction)homeButtonClicked:(id)sender;
- (IBAction)wdDirButtonClicked:(id)sender;
- (IBAction)openButtonClicked:(id)sender;
- (IBAction)addAllButtonClicked:(id)sender;
- (IBAction)diskTableClicked:(id)sender;
- (IBAction)userTableClicked:(id)sender;

+ (void)fileInDialog;



@end

