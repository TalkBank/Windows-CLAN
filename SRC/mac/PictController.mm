//
//  PictController.m

#import "DocumentWinController.h"
#import "PictController.h"

extern struct DefWin PictWinSize;

PictController *thePict = nil;

@implementation PictController


- (id)init
{
	if ((self = [super init])) {
		// initialization
	}
	thePict = nil;
	return self;
}

static BOOL findFullPictName(FNType *pictFName, FNType *curDocPath) {
	int pathLen, nameLen;
	FNType *rFileName;
	BOOL isFirstTimeAround;
	

	isFirstTimeAround = true;
	rFileName = curDocPath;
	pathLen = strlen(rFileName);
tryAgain:
	addFilename2Path(rFileName, pictFName);
	nameLen = strlen(rFileName);
	strcat(rFileName, ".jpg");
	if (access(rFileName, 0)) {
		rFileName[nameLen] = EOS;
		uS.str2FNType(rFileName, strlen(rFileName), ".jpeg");
		if (access(rFileName, 0)) {
			rFileName[nameLen] = EOS;
			uS.str2FNType(rFileName, strlen(rFileName), ".gif");
			if (access(rFileName, 0)) {
				if (isFirstTimeAround) {
					isFirstTimeAround = false;
					rFileName[pathLen] = EOS;
					strcat(rFileName, "media/");
					goto tryAgain;
				}
				rFileName[nameLen] = EOS;
				return(false);
			}
		}
	}
	return(true);
}

- (CGFloat)winTitlebarHeight {
	NSWindow *win;
	NSRect wFrame, cFrame;
	CGFloat TitleBarHeight;
	
	win = [self window];
	wFrame = [win frame];
	cFrame = [win contentRectForFrameRect:wFrame];
	TitleBarHeight = wFrame.size.height - cFrame.size.height;
	return TitleBarHeight;
}
+(void)openPict:(FNType *)pictFName curDocPath:(FNType *)curDocPath docWin:(NSWindow *)docWindow
{
	char err_mess[ERRMESSAGELEN];
	FNType *s;
	CGFloat oldHeight;
	NSRect wFrame;
	NSString *nPictFile;
	NSImage *theImage;
	NSImageRep *rep;
	NSSize imageSize;

	if (pictFName != nil) {
		s = strrchr(pictFName, '.');
		if (s != NULL)
			*s = EOS;
		strcpy(err_mess, "Can't locate picture filename:\n");
		strcat(err_mess, pictFName);
		strcat(err_mess, "\n or:\n");
		if (!findFullPictName(pictFName, curDocPath)) {
			strcat(err_mess, pictFName);
			do_warning_sheet(err_mess, docWindow);
			return;
		}
	}
	if (thePict == nil) {
		thePict = [[PictController alloc] initWithWindowNibName:@"PictController"];
		strcpy(thePict->mPictFName, curDocPath);
		[thePict showWindow:nil];
	} else {
		strcpy(thePict->mPictFName, curDocPath);
		nPictFile = [NSString stringWithUTF8String:thePict->mPictFName];
		theImage = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:nPictFile]];
		rep = [[theImage representations] objectAtIndex:0];
		imageSize = NSMakeSize(rep.pixelsWide, rep.pixelsHigh);
		wFrame = thePict.window.frame;
		oldHeight = wFrame.size.height;
		wFrame.size.height = imageSize.height + [thePict winTitlebarHeight];
		wFrame.size.width = imageSize.width;
		if (oldHeight < wFrame.size.height) {
			wFrame.origin.y = wFrame.origin.y - (wFrame.size.height - oldHeight);
			if (wFrame.origin.y < 0.0)
				wFrame.origin.y = 0.0;
		} else {
			wFrame.origin.y = wFrame.origin.y + (oldHeight - wFrame.size.height);
		}
		[thePict->pictView setImageScaling:NSImageScaleProportionallyUpOrDown];
		[thePict->pictView setImage:theImage];
		[theImage release];
		[thePict->picture addSubview:thePict->pictView];
		[[thePict window] setTitleWithRepresentedFilename:nPictFile];
		[[thePict window] setFrame:wFrame display:false];
		[thePict showWindow:nil];
	}
}

- (void)windowDidLoad {
	CGFloat oldHeight;
	NSRect wFrame;
	NSWindow *window = [self window];
	NSString *nPictFile;
	NSImage *theImage;
	NSImageRep *rep;
	NSSize imageSize;

	isSettingSize = NO;
	NSLog(@"PictController: windowDidLoad\n");
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
	nPictFile = [NSString stringWithUTF8String:mPictFName];
	theImage = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:nPictFile]];
	rep = [[theImage representations] objectAtIndex:0];
	imageSize = NSMakeSize(rep.pixelsWide, rep.pixelsHigh);
	wFrame = self.window.frame;
	oldHeight = wFrame.size.height;
	wFrame.size.height = imageSize.height + [thePict winTitlebarHeight];
	wFrame.size.width = imageSize.width;
	if (oldHeight < wFrame.size.height) {
		wFrame.origin.y = wFrame.origin.y - (wFrame.size.height - oldHeight);
		if (wFrame.origin.y < 0.0)
			wFrame.origin.y = 0.0;
	} else {
		wFrame.origin.y = wFrame.origin.y + (oldHeight - wFrame.size.height);
	}
	[pictView setImageScaling:NSImageScaleProportionallyUpOrDown];
	[pictView setImage:theImage];
	[theImage release];
	[picture addSubview:pictView];
	[window setTitleWithRepresentedFilename:nPictFile];
/*
	if (PictWinSize.height != 0 && PictWinSize.width != 0) {
		wFrame.origin.y = PictWinSize.top;
		wFrame.origin.x = PictWinSize.left;
		wFrame.size.height = PictWinSize.height;
		wFrame.size.width = PictWinSize.width;
	}
*/
	[window setFrame:wFrame display:false];
}

- (void)dealloc {
	[super dealloc];
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	
	NSLog(@"PictController: windowWillClose\n");
}

@end

@implementation PictController(Delegation)

- (void)windowWillResize:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)
	NSLog(@"PictController: windowWillResize\n");
}

- (void)windowDidResize:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)
	NSString *nPictFile;
	NSImage *theImage;
	NSLog(@"PictController: windowDidResize\n");
	
	if (!isSettingSize) {   // There is potential for recursion, but typically this is prevented in NSWindow which doesn't call this method if the frame doesn't change. However, just in case...
		isSettingSize = YES;
		nPictFile = [NSString stringWithUTF8String:mPictFName];
		theImage = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:nPictFile]];
		[[NSColor whiteColor] set];
		[pictView setImageScaling:NSImageScaleProportionallyUpOrDown];
		[pictView setImage:theImage];
		[theImage release];
		[picture addSubview:pictView];
/*
		NSRect wFrame = self.window.frame;
		if (PictWinSize.top != wFrame.origin.y || PictWinSize.left != wFrame.origin.x ||
			PictWinSize.height != wFrame.size.height || PictWinSize.width != wFrame.size.width) {
			PictWinSize.top = wFrame.origin.y;
			PictWinSize.left = wFrame.origin.x;
			PictWinSize.height = wFrame.size.height;
			PictWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
*/
		isSettingSize = NO;
	}
}

- (void)windowDidMove:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)
	
	NSLog(@"AVController: windowDidMove\n");
/*
	NSRect wFrame = self.window.frame;
	if (PictWinSize.top != wFrame.origin.y || PictWinSize.left != wFrame.origin.x ||
		PictWinSize.height != wFrame.size.height || PictWinSize.width != wFrame.size.width) {
		PictWinSize.top = wFrame.origin.y;
		PictWinSize.left = wFrame.origin.x;
		PictWinSize.height = wFrame.size.height;
		PictWinSize.width = wFrame.size.width;
		WriteCedPreference();
	}
*/
}

@end
