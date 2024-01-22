//
//  PictController.h


#import <Cocoa/Cocoa.h>

@interface PictController: NSWindowController <NSLayoutManagerDelegate> {
	FNType mPictFName[FNSize+2];
	IBOutlet NSView *picture;
	IBOutlet NSImageView *pictView;	// the image to display
	BOOL isSettingSize;
}

+(void)openPict:(FNType *)pictFName curDocPath:(FNType *)curDocPath docWin:(NSWindow *)docWindow;

@end

extern PictController *thePict;
