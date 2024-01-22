//
//  NSWindowController+CLANOptions.m
//  Clan
//

#import "ced.h"
#import "SelectF5Controller.h"
#import "CommandsController.h"

@implementation SelectF5Controller

static SelectF5Controller *SelectF5Window = nil;

- (id)init {
	SelectF5Window = nil;
	return [super initWithWindowNibName:@"SelectF5option"];
}

+ (void)SelectF5Dialog:(NSWindow *)callerTextWin;
{
//	NSRect wFrame;

	NSLog(@"SelectF5Controller: SelectF5Dialog\n");
	if (SelectF5Window == nil) {
		SelectF5Window = [[SelectF5Controller alloc] initWithWindowNibName:@"SelectF5option"];

		SelectF5Window->textWin = callerTextWin;
//		[SelectF5Window showWindow:nil];
		[SelectF5Window->textWin beginSheet:[SelectF5Window window] completionHandler:nil];

//		[NSApplication
//		[[NSApplication sharedApplication] runModalForWindow:[SelectF5Window window]];
//		[NSApp runModalForWindow:[SelectF5Window window]];
	}
}

- (void)windowDidLoad {
	NSWindow *window = [self window];

	[window setIdentifier:@"Kideval"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
/*
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{//TODO
		[[NSApplication sharedApplication] runModalForWindow:self.window];
	});
*/
//	NSRunLoop *theRL = [NSRunLoop currentRunLoop];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];

	if (F5Option == EVERY_LINE) {
		bulletOnEveryLine.state = NSControlStateValueOn;
		bulletOnEveryTier.state = NSControlStateValueOff;
	} else {
		bulletOnEveryLine.state = NSControlStateValueOff;
		bulletOnEveryTier.state = NSControlStateValueOn;
	}
}

- (IBAction)BulletOnEveryLineClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"SelectF5Controller: BulletOnEveryLineClicked\n");

	bulletOnEveryLine.state = NSControlStateValueOn;
	bulletOnEveryTier.state = NSControlStateValueOff;
}

- (IBAction)BulletOnEveryTierClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"SelectF5Controller: BulletOnEveryTierClicked\n");

	bulletOnEveryLine.state = NSControlStateValueOff;
	bulletOnEveryTier.state = NSControlStateValueOn;
}

- (IBAction)OKClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"SelectF5Controller: SelectF5OKClicked\n");

	if (bulletOnEveryLine.state == NSControlStateValueOn)
		F5Option = EVERY_LINE;
	else if (bulletOnEveryTier.state == NSControlStateValueOn)
		F5Option = EVERY_TIER;
	WriteCedPreference();
//	[NSApp stopModalWithCode:NSModalResponseOK];
	[[self window] close];
}

- (IBAction)CancelClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"SelectF5Controller: SelectF5CancelClicked\n");

//	[NSApp stopModalWithCode:NSModalResponseCancel];
	[[self window] close];
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"SelectF5Controller: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

//	[[NSApplication sharedApplication] stopModal];
	[SelectF5Window->textWin endSheet:[self window]];

	[SelectF5Window release];
	SelectF5Window = nil;
}

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"SelectF5Controller: windowDidResize\n");
}

// awakeFromNib is called when this object is done being unpacked from the nib file;
// at this point, we can do any needed initialization before turning app control over to the user
- (void)awakeFromNib
{
	int i;

	i = 12;
	// We don't actually need to do anything here, so it's empty
}


@end
