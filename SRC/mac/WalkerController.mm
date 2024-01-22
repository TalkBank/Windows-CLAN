
#import "DocumentWinController.h"
#import "WalkerController.h"
#import "IntegerFormater.h"

struct PBCs PBC = {false, 0, 0, 0, 0, 0};

@implementation WalkerController

WalkerController *WalkerControllerWindow = nil;

- (id)init {
	WalkerControllerWindow = nil;
	return [super initWithWindowNibName:@"WalkerController"];
}

- (IBAction)createWalkerControllerWindow:(id)sender;
{
#pragma unused (sender)

	NSLog(@"WalkerController: createWalkerControllerWindow\n");

	if (WalkerControllerWindow == nil) {
		WalkerControllerWindow = [[WalkerController alloc] initWithWindowNibName:@"WalkerController"];
		[WalkerControllerWindow showWindow:nil];
	} else
		[WalkerControllerWindow showWindow:nil];
}

- (void)windowDidLoad {

	char numS[256];
	NSWindow *window = [self window];
	OnlyIntegerValueFormatter *formatter = [[[OnlyIntegerValueFormatter alloc] init] autorelease];

	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...

	uS.sprintf(numS, "%ld", PBC.step_length);
	[WalkLengthField setStringValue:[NSString stringWithUTF8String:numS]];
	uS.sprintf(numS, "%ld", PBC.LoopCnt);
	[LoopNumberField setStringValue:[NSString stringWithUTF8String:numS]];
	uS.sprintf(numS, "%ld", PBC.backspace);
	[BackspaceField setStringValue:[NSString stringWithUTF8String:numS]];
	uS.sprintf(numS, "%ld", PBC.pause_len);
	[WalkPauseLenField setStringValue:[NSString stringWithUTF8String:numS]];
	uS.sprintf(numS, "%ld", PBC.speed);
	[PlaybackSpeedField setStringValue:[NSString stringWithUTF8String:numS]];


	[WalkLengthField setFormatter:formatter];
	[LoopNumberField setFormatter:formatter];
	[BackspaceField setFormatter:formatter];
	[WalkPauseLenField setFormatter:formatter];
	[PlaybackSpeedField setFormatter:formatter];

	PBC.enable = true;
}

- (IBAction)WalkLengthFieldChanged:(id)sender {
	NSLog(@"WalkerController: WalkLengthFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)LoopNumberFieldChanged:(id)sender {
	NSLog(@"WalkerController: LoopNumberFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)BackspaceFieldChanged:(id)sender {
	NSLog(@"WalkerController: BackspaceFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)WalkPauseLenFieldChanged:(id)sender {
	NSLog(@"WalkerController: WalkPauseLenFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)PlaybackSpeedFieldChanged:(id)sender {
	NSLog(@"WalkerController: PlaybackSpeedFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	unichar numS[256];
	NSUInteger len;
	NSString *comStr;

	NSLog(@"WalkerController: controlTextDidChange\n");
	if ( [ notification object ] == WalkLengthField ) {
		comStr = [WalkLengthField stringValue];
		len = [comStr length];
		if (len > 0) {
			[comStr getCharacters:numS range:NSMakeRange(0, len)];
			numS[len] = EOS;
			PBC.step_length = uS.atol(numS);
		}
	} else if ( [ notification object ] == LoopNumberField ) {
		comStr = [LoopNumberField stringValue];
		len = [comStr length];
		if (len > 0) {
			[comStr getCharacters:numS range:NSMakeRange(0, len)];
			numS[len] = EOS;
			PBC.LoopCnt = uS.atol(numS);
		}
	} else if ( [ notification object ] == BackspaceField ) {
		comStr = [BackspaceField stringValue];
		len = [comStr length];
		if (len > 0) {
			[comStr getCharacters:numS range:NSMakeRange(0, len)];
			numS[len] = EOS;
			PBC.backspace = uS.atol(numS);
		}
	} else if ( [ notification object ] == WalkPauseLenField ) {
		comStr = [WalkPauseLenField stringValue];
		len = [comStr length];
		if (len > 0) {
			[comStr getCharacters:numS range:NSMakeRange(0, len)];
			numS[len] = EOS;
			PBC.pause_len = uS.atol(numS);
		}
	} else if ( [ notification object ] == PlaybackSpeedField ) {
		comStr = [PlaybackSpeedField stringValue];
		len = [comStr length];
		if (len > 0) {
			[comStr getCharacters:numS range:NSMakeRange(0, len)];
			numS[len] = EOS;
			PBC.speed = uS.atol(numS);
		}
	}
}

/* Reopen the line panel when the app's persistent state is restored.

+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}
*/
@end

@implementation WalkerController(Delegation)


- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"WalkerController: windowDidResize\n");
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	unCH numS[256];
	NSUInteger len;
	NSString *fieldStr;

	NSLog(@"WalkerController: windowWillClose\n");

	PBC.enable = false;
	
	fieldStr = [WalkLengthField stringValue];
	len = [fieldStr length];
	[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
	numS[len] = EOS;
	PBC.step_length = uS.atol(numS);

	fieldStr = [LoopNumberField stringValue];
	len = [fieldStr length];
	[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
	numS[len] = EOS;
	PBC.LoopCnt = uS.atol(numS);

	fieldStr = [BackspaceField stringValue];
	len = [fieldStr length];
	[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
	numS[len] = EOS;
	PBC.backspace = uS.atol(numS);

	fieldStr = [WalkPauseLenField stringValue];
	len = [fieldStr length];
	[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
	numS[len] = EOS;
	PBC.pause_len = uS.atol(numS);

	fieldStr = [PlaybackSpeedField stringValue];
	len = [fieldStr length];
	[fieldStr getCharacters:numS range:NSMakeRange(0, len)];
	numS[len] = EOS;
	PBC.speed = uS.atol(numS);
	
	WriteCedPreference();

	[WalkerControllerWindow release];
	WalkerControllerWindow = nil;
}


@end
