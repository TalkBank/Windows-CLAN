
#include "ced.h"
#include "c_clan.h"
#import "ListBoxController.h"

extern NSFont *defUniFont;

@implementation ListBoxController

- (id)init {
	return [super initWithWindowNibName:@"ListBox"];
}

- (void)windowDidLoad {
	NSUInteger i, num, len;
	CGSize textSize;
	NSDictionary *addedObject;
	NSWindow *window = [self window];

	[window setIdentifier:@"ListBox"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	myTableView.doubleAction = @selector(doubleClick:);
	//	myTableView.delegate = self;

	NSDictionary *attributes = @{NSFontAttributeName:defUniFont};
	len = 0;
	if (self->whichBox == wdFolders) {
		for (i=0; true; i++) {
			addedObject = get_next_obj_folder(&i, wdFolders);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			myTableView.tableColumns[0].width = len;
		num = [[myTableArray arrangedObjects] count];
		if (0 < num)
			[myTableArray setSelectionIndex:0];
	} else if (self->whichBox == mlFolders) {
		for (i=0; true; i++) {
			addedObject = get_next_obj_folder(&i, mlFolders);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			myTableView.tableColumns[0].width = len;
		num = [[myTableArray arrangedObjects] count];
		if (0 < num)
			[myTableArray setSelectionIndex:0];
	} else if (self->whichBox == recallStr) {
		for (i=0; true; i++) {
			addedObject = get_next_obj_command(&i);
			if (addedObject != nil) {
				textSize = [[addedObject valueForKey:@"lines"] sizeWithAttributes:attributes];
				if (len < textSize.width)
					len = textSize.width;
				[myTableArray addObject:addedObject];
			} else
				break;
		}
		if (len > 0 && len < 5000)
			 myTableView.tableColumns[0].width = len;
		num = [[myTableArray arrangedObjects] count];
		i = getRecall_curCommand();
		if (i < num)
			[myTableArray setSelectionIndex:i];
	}
}

- (IBAction)deleteButtonClicked:(id)sender;
{
	NSUInteger selectionIndex;

	NSLog(@"ListBoxController: deleteButtonClicked\n");
	selectionIndex = [myTableArray selectionIndex];
	[myTableArray removeObjectAtArrangedObjectIndex:selectionIndex];
	if (self->whichBox == wdFolders) {
		delFromFolderList(selectionIndex, wdFolders);
	} else if (self->whichBox == mlFolders) {
		delFromFolderList(selectionIndex, mlFolders);
	} else if (self->whichBox == recallStr) {
		delFromRecall(selectionIndex);
	}
}

- (IBAction)selectButtonClicked:(id)sender;
{
	NSUInteger selectionIndex;

	NSLog(@"ListBoxController: selectButton\n");
	selectionIndex = [myTableArray selectionIndex];
	if (self->whichBox == wdFolders) {
		SetWdLibFolder(getFolderAtIndex(selectionIndex, wdFolders), wdFolders);
	} else if (self->whichBox == mlFolders) {
		SetWdLibFolder(getFolderAtIndex(selectionIndex, mlFolders), mlFolders);
	} else if (self->whichBox == recallStr) {
		AddComStringToComWin(getCommandAtIndex(selectionIndex));
	}
	[[self window] close];
}

-(void)doubleClick:(id)sender {
#pragma unused (sender)
	NSInteger clickedRow;
	NSLog(@"ListBoxController: doubleClick\n");

//	NSUInteger selectionIndex = [myTableArray selectionIndex];
//	[myTableArray setSelectionIndex:1];

	clickedRow = [myTableView clickedRow];
//	NSInteger clickedColumn = [myTableView clickedColumn];
	if (self->whichBox == wdFolders) {
		SetWdLibFolder(getFolderAtIndex(clickedRow, wdFolders), wdFolders);
	} else if (self->whichBox == mlFolders) {
		SetWdLibFolder(getFolderAtIndex(clickedRow, mlFolders), mlFolders);
	} else if (self->whichBox == recallStr) {
		AddComStringToComWin(getCommandAtIndex(clickedRow));
	}
	[[self window] close];
}
/*
	[myTableArray addObjects:[NSArray arrayWithObjects:
							  [NSDictionary dictionaryWithObjectsAndKeys:
							   @"One", @"lines", nil],
							  [NSDictionary dictionaryWithObjectsAndKeys:
							   @"Two", @"lines", nil],
							  [NSDictionary dictionaryWithObjectsAndKeys:
							   @"Three", @"lines", nil],
							  nil]];
*/

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"ListBoxController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];

}

/* Reopen the line panel when the app's persistent state is restored.
 */
+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}

@end

@implementation ListBoxController(Delegation)

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
#pragma unused (notification)
	NSUInteger row;
	NSTableView *cTableView;

	NSLog(@"ListBoxController: tableViewSelectionDidChange\n");
	cTableView = [notification object];
	row = [myTableArray selectionIndex];
//	clickedRow = [myTableView clickedRow];
//	clickedColumn = [myTableView clickedColumn];


}

@end
