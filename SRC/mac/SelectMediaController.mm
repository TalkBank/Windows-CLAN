
#import "DocumentWinController.h"
#import "SelectMediaController.h"

extern NSFont *defUniFont;

struct f_list {
	FNType *fname;
	struct f_list *nextFile;
} ;

static FNType curPath[FNSize]; // working directory
static struct f_list *DiskList = NULL;

static struct f_list *CleanupFileList(struct f_list *fl) {
	struct f_list *t;

	while (fl != NULL) {
		t = fl;
		fl = fl->nextFile;
		free(t->fname);
		free(t);
	}
	return(NULL);
}

static BOOL get_a_file(int fnum, FNType *s, struct f_list *fl, int max) {
	struct f_list *t;

	*s = EOS;
	if (fnum < 1 || fl == NULL)
		return(NO);

	for (t=fl; t != NULL; fnum--, t=t->nextFile) {
		if (fnum == 1) {
			strncpy(s, t->fname, max-1);
			s[max-1] = EOS;
			break;
		}
	}
	if (t == NULL)
		return(NO);
	return(YES);
}

static struct f_list *add_a_file(FNType *s, struct f_list *fl, int *fn) {
	FNType *tFname;
	struct f_list *tw, *t, *tt;

	tw = NEW(struct f_list);
	if (tw == NULL)
		return(fl);
	tFname = (FNType *)malloc((strlen(s)+1)*sizeof(FNType));
	if (tFname == NULL) {
		free(tw);
		return(fl);
	}
	if (fl == NULL) {
		fl = tw;
		tw->nextFile = NULL;
	} else if (uS.mStricmp(fl->fname, s) > 0) {
		tw->nextFile = fl;
		fl = tw;
	} else {
		t = fl;
		tt = fl->nextFile;
		while (tt != NULL) {
			if (uS.mStricmp(tt->fname, s) > 0)
				break;
			t = tt;
			tt = tt->nextFile;
		}
		if (tt == NULL) {
			t->nextFile = tw;
			tw->nextFile = NULL;
		} else {
			tw->nextFile = tt;
			t->nextFile = tw;
		}
	}
	tw->fname = tFname;
	strcpy(tw->fname, s);
	if (fn != NULL)
		(*fn)++;
	return(fl);
}

@implementation SelectMediaController

static SelectMediaController *SelectMediaWindow = nil;

- (id)init {
	curPath[0] = EOS;
	DiskList = CleanupFileList(DiskList);
	return [super initWithWindowNibName:@"SelectMedia"];
}

- (void)fillDiskArray {
	BOOL isRightFile;
	FNType mFileName[FNSize];
	NSUInteger i, len, cnt;
	CGSize textSize;
	NSDictionary *addedObject, *attributes;
	NSFileManager *fileManager;
	NSString *str, *cPath, *ext;
	NSArray *folderList;
	NSDictionary<NSFileAttributeKey, id> *itemAtt;

	attributes = @{NSFontAttributeName:defUniFont};
	fileManager = [NSFileManager defaultManager];
	for (cnt=0; cnt < 2; cnt++) {
		cPath = [NSString stringWithUTF8String:curPath];
		folderList = [fileManager contentsOfDirectoryAtPath:cPath error:NULL];
		len = 0;
		if (folderList != nil) {
			for (i=0; i < [folderList count]; i++) {
				str = (NSString *)[folderList objectAtIndex:i];
				if ([str characterAtIndex:0] != '.') {
					itemAtt = [fileManager attributesOfItemAtPath:[cPath stringByAppendingPathComponent:str] error:nil];
					if ([[itemAtt fileType] isEqualToString:NSFileTypeDirectory]) {
					} else {
						ext = [[str pathExtension] lowercaseString];
						if (ext == nil)
							isRightFile = NO;
						else if ([ext isEqualToString:@"mov"] ||
								 [ext isEqualToString:@"mp4"] ||
								 [ext isEqualToString:@"mp3"] ||
								 [ext isEqualToString:@"wav"] ||
								 [ext isEqualToString:@"wave"]||
								 [ext isEqualToString:@"aif"] ||
								 [ext isEqualToString:@"m4v"] ||
								 [ext isEqualToString:@"avi"] ||
								 [ext isEqualToString:@"wmv"] ||
								 [ext isEqualToString:@"mpg"] ||
								 [ext isEqualToString:@"mpeg"]||
								 [ext isEqualToString:@"aif"] ||
								 [ext isEqualToString:@"aiff"])
							isRightFile = YES;
						else
							isRightFile = NO;
						if (isRightFile == YES) {
							strcpy(mediaFName, [str UTF8String]);
							DiskList = add_a_file(mediaFName, DiskList, NULL);
						}
					}
				}
			}
		} else
			break;
		addFilename2Path(curPath, "media");
	}
	for (i=1; get_a_file(i, mediaFName, DiskList, FNSize) == YES; i++) {
		extractFileName(mFileName, mediaFName);
		str = [NSString stringWithUTF8String:mFileName];
		addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
		textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
		if (len < textSize.width)
			len = textSize.width;
		[FilesDiskArray addObject:addedObject];
	}
	if (len < 215)
		len = 200;
	if (len > 0 && len < 5000)
		FilesDiskView.tableColumns[0].width = len;
	len = [[FilesDiskArray arrangedObjects] count];
	if (len > 0)
		[FilesDiskArray setSelectionIndex:0];
}

+ (void)SelectMediaDialog:(DocumentWindowController *)docWinController str:(FNType *)mediaFName;
{
	Document *cDoc;

	NSLog(@"SelectMediaController: SelectMediaDialog\n");

	if (SelectMediaWindow == nil) {
		cDoc = [docWinController document];
		if (cDoc->filePath[0] == EOS) {
			mediaFName[0] = EOS;
			do_warning_sheet("Please save your data file on hard drive first", [docWinController window]);
			return;
		}
		extractPath(curPath, cDoc->filePath);

		SelectMediaWindow = [[SelectMediaController alloc] initWithWindowNibName:@"SelectMedia"];
		SelectMediaWindow->mediaFName = mediaFName;
		SelectMediaWindow->docWinController = docWinController;
//		[SelectMediaWindow showWindow:nil];
		[[docWinController window] beginSheet:[SelectMediaWindow window] completionHandler:nil];
		[NSApp runModalForWindow:[SelectMediaWindow window]];
	}
}

- (void)windowDidLoad {
	NSWindow *window = [self window];

	[window setIdentifier:@"SelectMedia"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
/*
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{//TODO
		[[NSApplication sharedApplication] runModalForWindow:self.window];
	});
*/
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];

	FilesDiskView.doubleAction = @selector(doubleClickDisk:);
//	FilesDiskView.delegate = self;

	mediaFName[0] = EOS;
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

- (IBAction)cancelButtonClicked:(id)sender;
{
#pragma unused (sender)

	NSLog(@"SelectMediaController: cancelButtonClicked\n");
	mediaFName[0] = EOS;
	[[self window] close];
}

- (IBAction)okButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSInteger clickedRow;
	FNType mFileName[FNSize];

	NSLog(@"SelectMediaController: okButtonClicked\n");

	mediaFName[0] = EOS;
	clickedRow = [FilesDiskArray selectionIndex];
	if (clickedRow < 0 || clickedRow >= [FilesDiskView numberOfRows])
		[[self window] close];
	if (get_a_file(clickedRow+1, mFileName, DiskList, FNSize) == YES) {
		strcpy(mediaFName, mFileName);
	} else
		mediaFName[0] = EOS;
	[[self window] close];
}

- (IBAction)diskTableClicked:(id)sender
{
#pragma unused (sender)
	NSLog(@"SelectMediaController: diskTableClicked\n");
}

-(void)doubleClickDisk:(id)sender {
#pragma unused (sender)
	NSInteger clickedRow;
	FNType mFileName[FNSize];

	NSLog(@"SelectMediaController: doubleClickDisk\n");

	clickedRow = [FilesDiskView clickedRow];
	if (clickedRow < 0 || clickedRow >= [FilesDiskView numberOfRows])
		return;
	if (get_a_file(clickedRow+1, mFileName, DiskList, FNSize) == YES) {
		strcpy(mediaFName, mFileName);
		[[self window] close];
	}
}

/* Reopen the line panel when the app's persistent state is restored.

+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}
*/

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"SelectMediaController: windowDidResize\n");
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"SelectMediaController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

//	[[NSApplication sharedApplication] stopModal];
	[[docWinController window] endSheet:[self window]];
	[NSApp stopModal];

//	NSUInteger selectionIndex = [FilesUserArray selectionIndex];

	DiskList = CleanupFileList(DiskList);
	[SelectMediaWindow release];
	SelectMediaWindow = nil;

}

@end

@implementation SelectMediaController(Delegation)

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
#pragma unused (notification)
}

@end
