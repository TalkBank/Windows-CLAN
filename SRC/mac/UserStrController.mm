
#import "DocumentWinController.h"
#import "UserStrController.h"

@implementation UserStrController

static UserStrController *UserStrWindow = nil;
static unCH tempUserString[UTTLINELEN+2];

- (id)init {
	UserStrWindow = nil;
	return [super initWithWindowNibName:@"UserStr"];
}

+(void)userStrDialog:(DocumentWindowController *)docWinController label:(NSString *)info str:(unCH *)strPtr max:(NSUInteger)max;
{
	NSLog(@"UserStrController: userStrDialog\n");
	if (UserStrWindow == nil) {
		UserStrWindow = [[UserStrController alloc] initWithWindowNibName:@"UserStr"];
		UserStrWindow->docWinController = docWinController;
		UserStrWindow->info = info;
		UserStrWindow->strPtr = strPtr;
		UserStrWindow->max = max;
//		[IdsWindow showWindow:nil];
		[[docWinController window] beginSheet:[UserStrWindow window] completionHandler:nil];
		[NSApp runModalForWindow:[UserStrWindow window]];
	}
}

- (void)windowDidLoad {

	NSWindow *window = [self window];

//	[window setIdentifier:@"Ids"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];

	[UserStrWindow->labelField setStringValue:info];
	[strField setStringValue:[NSString stringWithCharacters:strPtr length:strlen(strPtr)]];
}

- (IBAction)strFieldChanged:(id)sender {
#pragma unused (sender)
	NSLog(@"UserStrController: strFieldChanged\n");
//	if ([@"" isEqual:[sender stringValue]])
//		return;
	[self doneButtonClicked:self];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
#pragma unused (notification)
//	unichar st[BUFSIZ];
//	NSUInteger len;
//	NSString *comStr;
	NSLog(@"UserStrController: controlTextDidChange\n");

	if ( [ notification object ] == strField ) {
//		comStr = [strField stringValue];
//		len = [comStr length];
//		if (len < BUFSIZ) {
//			[comStr getCharacters:st range:NSMakeRange(0, len)];
//			st[len] = EOS;
//		}
	}
}

- (IBAction)cancelButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSLog(@"UserStrController: cancelButtonClicked\n");

	strPtr[0] = EOS;
	[[self window] close];
}

- (IBAction)doneButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSUInteger len;
	NSString *comStr;

	NSLog(@"UserStrController: doneButtonClicked\n");
	comStr = [strField stringValue];
	len = [comStr length];
	if (len < max) {
		[comStr getCharacters:tempUserString range:NSMakeRange(0, len)];
		tempUserString[len] = EOS;
		strPtr[0] = EOS;
		strcat(strPtr, tempUserString);
	}
	[[self window] close];
}

/* Reopen the line panel when the app's persistent state is restored.

+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}
*/

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"UserStrController: windowDidResize\n");
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"UserStrController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

//	[[NSApplication sharedApplication] stopModal];
	[[docWinController window] endSheet:[self window]];
	[NSApp stopModal];
	[UserStrWindow release];
	UserStrWindow = nil;
}

@end

@implementation UserStrController(Delegation)


@end
