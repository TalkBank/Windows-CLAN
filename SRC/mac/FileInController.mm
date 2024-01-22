
#import "ced.h"
#import "c_clan.h"
#import "FileInController.h"
#import "EvalController.h"
#import "KidevalController.h"
#import "CommandsController.h"

extern int F_numfiles;

//extern struct DefWin ClanWinSize;
extern FNType mDirPathName[];
extern NSFont *defUniFont;


struct f_list {
	FNType *fname;
	BOOL isDir;
	struct f_list *nextFile;
} ;

static FNType curPath[FNSize]; // working directory
static struct f_list *FileList = NULL, *DiskList = NULL;
static NSControlStateValue OnlyChaCexButton = NSControlStateValueOn;

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

static BOOL get_a_file(int fnum, FNType *s, BOOL *isDir, struct f_list *fl, int max) {
	struct f_list *t;

	*s = EOS;
	if (fnum < 1 || fl == NULL)
		return(NO);

	for (t=fl; t != NULL; fnum--, t=t->nextFile) {
		if (fnum == 1) {
			strncpy(s, t->fname, max-1);
			s[max-1] = EOS;
			if (isDir != nil)
				*isDir = t->isDir;
			break;
		}
	}
	if (t == NULL)
		return(NO);
	return(YES);
}

static struct f_list *add_a_file(FNType *s, BOOL isDir, struct f_list *fl, int *fn) {
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
	} else if (uS.mStricmp(fl->fname, s) > 0 && (!fl->isDir || isDir)) {
		tw->nextFile = fl;
		fl = tw;
	} else {
		t = fl;
		tt = fl->nextFile;
		if (tt != NULL) {
			if (!isDir && tt->isDir) {
				while (tt != NULL) {
					if (!tt->isDir)
						break;
					t = tt;
					tt = tt->nextFile;
				}
			}
		}
		while (tt != NULL) {
			if (uS.mStricmp(tt->fname, s) > 0 || (isDir && !tt->isDir))
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
	tw->isDir = isDir;
	if (fn != NULL)
		(*fn)++;
	return(fl);
}

static void remove_FileList_file(int fnum) {
	struct f_list *t, *tt;

	if (FileList == NULL) ;
	else if (fnum == 1) {
		t = FileList;
		FileList = FileList->nextFile;
		free(t->fname);
		free(t);
		F_numfiles--;
	} else {
		tt = FileList;
		t  = FileList->nextFile;
		for (fnum--; t != NULL; fnum--) {
			if (fnum == 1) {
				tt->nextFile = t->nextFile;
				free(t->fname);
				free(t);
				F_numfiles--;
				return;
			}
			tt = t;
			t = t->nextFile;
		}
	}
}

static int isDuplicate_FileList_File(FNType *fs) {
	struct f_list *t;

	for (t=FileList; t != NULL; t=t->nextFile) {
		if (!strcmp(t->fname, fs))
			return(TRUE);
	}
	return(FALSE);
}

void get_selected_file(int fnum, FNType *s, int max) {
	get_a_file(fnum, s, nil, FileList, max);
}

void InitFileDialog(void) {
	FileList = NULL;
	DiskList = NULL;
	F_numfiles = 0;
}

@implementation FileInController

static FileInController *FileInWindow = nil;


- (id)init {
	curPath[0] = EOS;
	DiskList = CleanupFileList(DiskList);
	FileList = CleanupFileList(FileList);
	F_numfiles = 0;
	return [super initWithWindowNibName:@"FileIn"];
}

- (void)fillDiskArray {
	BOOL isRightFile;
	BOOL isDir;
	FNType mFileName[FNSize];
	NSUInteger i, len;
	CGSize textSize;
	NSDictionary *addedObject, *attributes;
	NSFileManager *fileManager;
	NSString *str, *cPath, *ext;
	NSArray *folderList;
	NSDictionary<NSFileAttributeKey, id> *itemAtt;

	attributes = @{NSFontAttributeName:defUniFont};
	fileManager = [NSFileManager defaultManager];
	cPath = [NSString stringWithUTF8String:curPath];
	folderList = [fileManager contentsOfDirectoryAtPath:cPath error:NULL];
	len = 0;
	if (folderList != nil) {
		for (i=0; i < [folderList count]; i++) {
			str = (NSString *)[folderList objectAtIndex:i];
			if ([str characterAtIndex:0] != '.') {
				itemAtt = [fileManager attributesOfItemAtPath:[cPath stringByAppendingPathComponent:str] error:nil];
				strcpy(mDirPathName, curPath);
				if ([[itemAtt fileType] isEqualToString:NSFileTypeDirectory]) {
					addFilename2Path(mDirPathName, [str UTF8String]);
					DiskList = add_a_file(mDirPathName, YES, DiskList, NULL);
				} else {
					if ([showOnlyChaCexButton state] == NSControlStateValueOn) {
						ext = [[str pathExtension] lowercaseString];
						if (ext == nil)
							isRightFile = NO;
#ifdef _CLAN_DEBUG
						else if ([ext isEqualToString:@"cha"] || [ext isEqualToString:@"cex"] ||
								 [ext isEqualToString:@"chi"])
#else
						else if ([ext isEqualToString:@"cha"] || [ext isEqualToString:@"cex"])
#endif
							isRightFile = YES;
						else
							isRightFile = NO;
					} else
						isRightFile = YES;
					addFilename2Path(mDirPathName, [str UTF8String]);
					if (isDuplicate_FileList_File(mDirPathName))
						isRightFile = NO;
					if (isRightFile == YES)
						DiskList = add_a_file(mDirPathName, NO, DiskList, NULL);
				}
			}
		}
		for (i=1; get_a_file(i, mDirPathName, &isDir, DiskList, FNSize) == YES; i++) {
			extractFileName(mFileName, mDirPathName);
			str = [NSString stringWithUTF8String:mFileName];
			if (isDir)
				str = [str stringByAppendingString:@"/"];
			addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
			textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
			if (len < textSize.width)
				len = textSize.width;
			[FilesDiskArray addObject:addedObject];
		}
	}
	if (len < 215)
		len = 200;
	if (len > 0 && len < 5000)
		FilesDiskView.tableColumns[0].width = len;
	len = [[FilesDiskArray arrangedObjects] count];
	if (len > 0)
		[FilesDiskArray setSelectionIndex:0];
}

- (void)fillUserArray {
	FNType mFileName[FNSize];
	NSUInteger i, len;
	CGSize textSize;
	NSDictionary *addedObject, *attributes;
	NSString *str;

	len = 0;
	attributes = @{NSFontAttributeName:defUniFont};
	for (i=1; i <= F_numfiles; i++) {
		get_selected_file(i, mDirPathName, FNSize);
		extractFileName(mFileName, mDirPathName);
		str = [NSString stringWithUTF8String:mFileName];
		addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
		textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
		if (len < textSize.width)
			len = textSize.width;
		[FilesUserArray addObject:addedObject];
	}

	if (len < 200)
		len = 200;
	if (len > 0 && len < 5000)
		FilesUserView.tableColumns[0].width = len;
	len = [[FilesUserArray arrangedObjects] count];
	if (len > 0)
		[FilesUserArray setSelectionIndex:0];
}

- (void)createPopupFoldersMenu {
	NSUInteger i;
	FNType *folder, temp[FNSize];
	NSString *str;
	NSUInteger num;
	NSIndexSet *range;

	strcpy(temp, curPath);
	folder = strrchr(temp, '/');
	i = 0;
	while (folder != NULL) {
		if (*(folder+1) != EOS) {
			str = [NSString stringWithUTF8String:folder+1];
			[foldersPopUp insertItemWithTitle:str atIndex:i];
			i++;
		}
		*folder = EOS;
		folder = strrchr(temp, '/');
	}
	str = [NSString stringWithUTF8String:"/"];
	[foldersPopUp insertItemWithTitle:str atIndex:i];

	[foldersPopUp selectItemAtIndex:0];
	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

+ (void)fileInDialog
{
	NSString *comStr;

	NSLog(@"FileInController: fileInDialog\n");

	if (FileInWindow == nil) {
		FileInWindow = [[FileInController alloc] initWithWindowNibName:@"FileIn"];
		comStr = [commandsWindow->commandString stringValue];
		FileInWindow->isCallKideval = false;
		FileInWindow->isCallEval = false;
		if ([comStr length] >= 7) {
			if ([comStr compare:@"kideval" options:NSCaseInsensitiveSearch range:NSMakeRange(0,7)] == NSOrderedSame) {
				FileInWindow->isCallKideval = true;
			} else if ([comStr compare:@"eval" options:NSCaseInsensitiveSearch range:NSMakeRange(0,4)] == NSOrderedSame) {
				FileInWindow->isCallEval = true;
			}
		} else if ([comStr length] >= 4) {
			if ([comStr compare:@"eval" options:NSCaseInsensitiveSearch range:NSMakeRange(0,4)] == NSOrderedSame) {
				FileInWindow->isCallEval = true;
			}
		}

//		[FileInWindow showWindow:nil];
		[[commandsWindow window] beginSheet:[FileInWindow window] completionHandler:nil];
	}

}

- (void)windowDidLoad {
	NSWindow *window = [self window];

	[window setIdentifier:@"FileIn"];
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
	FilesUserView.doubleAction = @selector(doubleClickUser:);
//	FilesDiskView.delegate = self;
//	FilesUserArray.delegate = self;

	showOnlyChaCexButton.state = OnlyChaCexButton;
	mDirPathName[0] = EOS;
	[filePathString setTitle:[NSString stringWithUTF8String:mDirPathName]];

//	if (curPath[0] == EOS)
		strcpy(curPath, wd_dir);
	[self createPopupFoldersMenu];
	[self fillUserArray];
}

- (IBAction)foldersButtonClicked:(id)sender
{
#pragma unused (sender)
	NSInteger len;
	NSUInteger num;
	NSIndexSet *range;
	NSInteger menuPos;
	NSString *menuStr;

	NSLog(@"FileInController: foldersButtonClicked\n");
	menuPos = [sender indexOfSelectedItem];
	if (menuPos == 0)
		return;
	len = [foldersPopUp numberOfItems] - 1;
	curPath[0] = EOS;
	while (len >= menuPos) {
		menuStr = [foldersPopUp itemTitleAtIndex:len];
		num = strlen(curPath);
		if (num > 0 && curPath[num-1] != '/')
			strcat(curPath, "/");
		strcat(curPath, [menuStr UTF8String]);
		len--;
	}
	[foldersPopUp removeAllItems];
	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	[self createPopupFoldersMenu];
}

- (IBAction)removeButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	NSUInteger selectionIndex;

	NSLog(@"FileInController: removeButtonClicked\n");

	selectionIndex = [FilesUserArray selectionIndex];
	[FilesUserArray removeObjectAtArrangedObjectIndex:selectionIndex];
	remove_FileList_file(selectionIndex+1);

	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

- (IBAction)clearButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;

	NSLog(@"FileInController: clearButtonClicked\n");


	num = [[FilesUserArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesUserArray removeObjectsAtArrangedObjectIndexes:range];
	FileList = CleanupFileList(FileList);
	F_numfiles = 0;

	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

- (IBAction)doneButtonClicked:(id)sender;
{
#pragma unused (sender)

	NSLog(@"FileInController: doneButtonClicked\n");
	[[self window] close];
}

- (IBAction)showOnlyChaCexClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	NSLog(@"FileInController: showOnlyChaCexClicked\n");

	if ([showOnlyChaCexButton state] == NSControlStateValueOn) {
		OnlyChaCexButton = NSControlStateValueOn;
	} else if ([showOnlyChaCexButton state] == NSControlStateValueOff) {
		OnlyChaCexButton = NSControlStateValueOff;
	} else {
	}
	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

- (IBAction)desktopButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	FSRef  tRef;

	NSLog(@"FileInController: desktopButtonClicked\n");
	if (FSFindFolder(kOnSystemDisk,kDesktopFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, mDirPathName, FNSize);
		if (mDirPathName[0] != EOS) {
			strcpy(curPath, mDirPathName);
			num = strlen(curPath);
			if (num > 0 && curPath[num-1] == '/')
				curPath[num-1] = EOS;
			[foldersPopUp removeAllItems];
			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			[self createPopupFoldersMenu];
		}
	}
}

- (IBAction)downloadsButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	FSRef  tRef;

	NSLog(@"FileInController: downloadsButtonClicked\n");
	if (FSFindFolder(kOnSystemDisk,kDownloadsFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, mDirPathName, FNSize);
		if (mDirPathName[0] != EOS) {
			strcpy(curPath, mDirPathName);
			num = strlen(curPath);
			if (num > 0 && curPath[num-1] == '/')
				curPath[num-1] = EOS;
			[foldersPopUp removeAllItems];
			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			[self createPopupFoldersMenu];
		}
	}
}

- (IBAction)homeButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	FSRef  tRef;

	NSLog(@"FileInController: homeButtonClicked\n");
	if (FSFindFolder(kOnSystemDisk,kCurrentUserFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, mDirPathName, FNSize);
		if (mDirPathName[0] != EOS) {
			strcpy(curPath, mDirPathName);
			num = strlen(curPath);
			if (num > 0 && curPath[num-1] == '/')
				curPath[num-1] = EOS;
			[foldersPopUp removeAllItems];
			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			[self createPopupFoldersMenu];
		}
	}
}

- (IBAction)wdDirButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;

	NSLog(@"FileInController: homeButtonClicked\n");
	strcpy(curPath, wd_dir);
	num = strlen(curPath);
	if (num > 0 && curPath[num-1] == '/')
		curPath[num-1] = EOS;
	[foldersPopUp removeAllItems];
	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	[self createPopupFoldersMenu];
}

- (IBAction)openButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	NSInteger clickedRow;
	BOOL isDir;

	NSLog(@"FileInController: openButtonClicked\n");

	clickedRow = [FilesDiskArray selectionIndex];
	if (clickedRow < 0 || clickedRow >= [FilesDiskView numberOfRows])
		return;
	if (get_a_file(clickedRow+1, mDirPathName, &isDir, DiskList, FNSize) == YES) {
		if (isDir == YES) {
			strcpy(curPath, mDirPathName);
			num = strlen(curPath);
			if (num > 0 && curPath[num-1] == '/')
				curPath[num-1] = EOS;
			[foldersPopUp removeAllItems];
			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			[self createPopupFoldersMenu];
		} else if (!isDuplicate_FileList_File(mDirPathName)) {
			FileList = add_a_file(mDirPathName, NO, FileList, &F_numfiles);

			num = [[FilesUserArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesUserArray removeObjectsAtArrangedObjectIndexes:range];
			[self fillUserArray];

			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			DiskList = CleanupFileList(DiskList);
			[self fillDiskArray];
		}
	}
}

- (IBAction)addAllButtonClicked:(id)sender;
{
#pragma unused (sender)
	BOOL isDir;
	NSInteger i;
	NSUInteger num;
	NSIndexSet *range;

	NSLog(@"FileInController: addAllButtonClicked\n");

	for (i=1; get_a_file(i, mDirPathName, &isDir, DiskList, FNSize) == YES; i++) {
		if (isDir == NO) {
			if (!isDuplicate_FileList_File(mDirPathName)) {
				FileList = add_a_file(mDirPathName, NO, FileList, &F_numfiles);
			}
		}
	}
	num = [[FilesUserArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesUserArray removeObjectsAtArrangedObjectIndexes:range];
	[self fillUserArray];

	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

- (IBAction)diskTableClicked:(id)sender
{
#pragma unused (sender)
	NSInteger clickedRow;
	BOOL isDir;

	NSLog(@"FileInController: diskTableClicked\n");

	clickedRow = [FilesDiskView clickedRow];
	if (clickedRow < 0 || clickedRow >= [FilesDiskView numberOfRows])
		return;

	if (get_a_file(clickedRow+1, mDirPathName, &isDir, DiskList, FNSize) == YES) {
		if (isDir == YES) {
			[openButtonString setTitle:[NSString stringWithUTF8String:"Open"]];
		} else {
			[openButtonString setTitle:[NSString stringWithUTF8String:"Add ->"]];
		}
	}
}

- (IBAction)userTableClicked:(id)sender
{
#pragma unused (sender)
	NSInteger clickedRow;

	NSLog(@"FileInController: userTableClicked\n");

	clickedRow = [FilesUserView clickedRow];
	if (clickedRow == -1)
		return;
	get_selected_file(clickedRow+1, mDirPathName, FNSize);
	[filePathString setTitle:[NSString stringWithUTF8String:mDirPathName]];
}

-(void)doubleClickDisk:(id)sender {
#pragma unused (sender)
	NSUInteger num;
	NSIndexSet *range;
	NSInteger clickedRow;
	BOOL isDir;

	NSLog(@"FileInController: doubleClickDisk\n");

	clickedRow = [FilesDiskView clickedRow];
	if (clickedRow < 0 || clickedRow >= [FilesDiskView numberOfRows])
		return;
	if (get_a_file(clickedRow+1, mDirPathName, &isDir, DiskList, FNSize) == YES) {
		if (isDir == YES) {
			strcpy(curPath, mDirPathName);
			num = strlen(curPath);
			if (num > 0 && curPath[num-1] == '/')
				curPath[num-1] = EOS;
			[foldersPopUp removeAllItems];
			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			[self createPopupFoldersMenu];
		} else if (!isDuplicate_FileList_File(mDirPathName)) {
			FileList = add_a_file(mDirPathName, NO, FileList, &F_numfiles);

			num = [[FilesUserArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesUserArray removeObjectsAtArrangedObjectIndexes:range];
			[self fillUserArray];

			num = [[FilesDiskArray arrangedObjects] count];
			range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
			[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
			DiskList = CleanupFileList(DiskList);
			[self fillDiskArray];
		}
	}
}

-(void)doubleClickUser:(id)sender {
	NSUInteger num;
	NSIndexSet *range;
	NSInteger clickedRow;

	NSLog(@"FileInController: doubleClickDisk\n");

	clickedRow = [FilesUserView clickedRow];
	if (clickedRow < 0 || clickedRow >= [FilesUserView numberOfRows])
		return;

	[FilesUserArray removeObjectAtArrangedObjectIndex:clickedRow];
	remove_FileList_file(clickedRow+1);

	num = [[FilesDiskArray arrangedObjects] count];
	range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, num)];
	[FilesDiskArray removeObjectsAtArrangedObjectIndexes:range];
	DiskList = CleanupFileList(DiskList);
	[self fillDiskArray];
}

/* Reopen the line panel when the app's persistent state is restored.

+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}
*/

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"FileInController: windowDidResize\n");
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	int i;
	BOOL isAtAdded;
	const char *err;
	NSInteger strLen;
	NSRange aRange;
	NSText* fieldEditor;
	NSString *comStr;

	NSLog(@"FileInController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

//	[[NSApplication sharedApplication] stopModal];
	[[commandsWindow window] endSheet:[self window]];


//	NSUInteger selectionIndex = [FilesUserArray selectionIndex];
	
	DiskList = CleanupFileList(DiskList);
	if (commandsWindow != nil) {
		if (commandsWindow->commandString != nil) {
			isAtAdded = NO;
			comStr = [commandsWindow->commandString stringValue];
			if (comStr != nil) {
				strcpy(spareTier3, [comStr UTF8String]);
				for (i=0; spareTier3[i] != EOS; i++) {
					if (spareTier3[i] == '@' && i > 0) {
						if (isSpace(spareTier3[i-1]) && (isSpace(spareTier3[i+1]) || spareTier3[i+1] == EOS)) {
							isAtAdded = YES;
							break;
						}
					}
				}
			}
			if (isAtAdded == NO) {
				uS.remblanks(spareTier3);
				strcat(spareTier3, " @");
				[commandsWindow->commandString setStringValue:[NSString stringWithUTF8String:spareTier3]];
				strLen = [[commandsWindow->commandString stringValue] length];
				aRange.location = strLen;
				aRange.length = 0;
				fieldEditor = [commandsWindow->commandString currentEditor];
				[fieldEditor setSelectedRange:aRange];
				SetClanWinIcons();
			}
		}
		[FileInWindow release];
		FileInWindow = nil;
		if (isCallKideval) {
			err = [KidevalController KidevalDialog];
			if (err != NULL)
				do_warning_sheet(err, [commandsWindow window]);
		} else if (isCallEval) {
			err = [EvalController EvalDialog];
			if (err != NULL)
				do_warning_sheet(err, [commandsWindow window]);
		}
	}
}

@end

@implementation FileInController(Delegation)

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
#pragma unused (notification)
	NSUInteger row;
	NSTableView *cFileView;
	BOOL isDir;

	NSLog(@"FileInController: tableViewSelectionDidChange\n");
	cFileView = [notification object];
	if (cFileView == FilesDiskView) {
		row = [FilesDiskArray selectionIndex];
		if (get_a_file(row+1, mDirPathName, &isDir, DiskList, FNSize) == YES) {
			if (isDir == YES) {
				[openButtonString setTitle:[NSString stringWithUTF8String:"Open"]];
			} else {
				[openButtonString setTitle:[NSString stringWithUTF8String:"Add ->"]];
			}
		}
	} else {
		row = [FilesUserArray selectionIndex];
		get_selected_file(row+1, mDirPathName, FNSize);
		[filePathString setTitle:[NSString stringWithUTF8String:mDirPathName]];
	}
//	clickedRow = [FilesDiskView clickedRow];
//	clickedColumn = [FilesDiskView clickedColumn];


}

@end
