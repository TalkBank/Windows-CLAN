
#import "AboutClanController.h"
#import "Controller.h"

@implementation AboutClanController

- (id)init {
    return [super initWithWindowNibName:@"AboutClan"];
}

- (void)windowDidLoad {
	extern char VERSION[];

	NSWindow *window = [self window];
	[window setIdentifier:@"AboutClan"];
    [window setRestorationClass:[self class]];
    [super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...

	[versionField setStringValue:[NSString stringWithUTF8String:VERSION]];
	[infoField setStringValue:[NSString stringWithUTF8String:"For help on CLAN please download the manual from http://childes.talkbank.org. Send specific questions to macw@cmu.edu. c 1990-2021"]];
}

/* Reopen the line panel when the app's persistent state is restored. 
*/
+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
//    completionHandler([[(Controller *)[NSApp delegate] aboutController] window], NULL);
}

@end
