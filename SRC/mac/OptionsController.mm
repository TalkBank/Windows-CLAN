//
//  NSWindowController+CLANOptions.m
//  Clan
//

#import "OptionsController.h"
#import "DocumentWinController.h"
#import "Document.h"
#import "Controller.h"
#import "ced.h"
#import "c_clan.h"

extern char isSavePrefs;
extern FNType prefsDir[];

@implementation OptionsPanelController

extern BOOL DefClan;
extern BOOL DefWindowDims;
extern BOOL isCursorPosRestore;

- (id)init {
	return [super initWithWindowNibName:@"CLANOptions"];
}

- (void)windowDidLoad {
	extern char ced_version[];

	NSWindow *window = [self window];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...

	[disambiguateField setStringValue:[NSString stringWithUTF8String:DisTier]];

	if (DefClan)
		OpenCommandsButton.state = NSControlStateValueOn;
	else
		OpenCommandsButton.state = NSControlStateValueOff;

	if (DefWindowDims)
		NewfilePosSizeButton.state = NSControlStateValueOn;
	else
		NewfilePosSizeButton.state = NSControlStateValueOff;

	if (isCursorPosRestore)
		RestoreCursorButton.state = NSControlStateValueOn;
	else
		RestoreCursorButton.state = NSControlStateValueOff;
	
	if (doMixedSTWave)
		doMixedSTWaveButton.state = NSControlStateValueOn;
	else
		doMixedSTWaveButton.state = NSControlStateValueOff;

}

/*
	NSInteger i;

	if ([(NSButton*)sender state] == NSControlStateValueOn) {
		i = 1;
	} else {
		i = 0;
	}

	if ([OpenCommandsButton state] == NSControlStateValueOn) {
		i = 1;
	} else if ([OpenCommandsButton state] == NSControlStateValueOff) {
		i = 0;
	} else {
		i = 12;
	}
//	[OpenCommandsButton performClick:self];
*/

- (IBAction)optionsNewfilePosSizeClicked:(NSButton *)sender
{
#pragma unused (sender)
	if ([NewfilePosSizeButton state] == NSControlStateValueOn) {// 2019-11-07
		DefWindowDims = TRUE;
	} else if ([NewfilePosSizeButton state] == NSControlStateValueOff) {
		DefWindowDims = FALSE;
	} else {
	}
	WriteCedPreference();
}

- (IBAction)optionsOpenCommandsClicked:(NSButton *)sender
{
#pragma unused (sender)
	if ([OpenCommandsButton state] == NSControlStateValueOn) {// 2019-11-07
		DefClan = TRUE;
	} else if ([OpenCommandsButton state] == NSControlStateValueOff) {
		DefClan = FALSE;
	} else {
	}
	WriteCedPreference();
}

- (IBAction)optionsRestoreCursorClicked:(NSButton *)sender
{
#pragma unused (sender)
	if ([RestoreCursorButton state] == NSControlStateValueOn) {// 2019-11-07
		isCursorPosRestore = TRUE;
	} else if ([RestoreCursorButton state] == NSControlStateValueOff) {
		isCursorPosRestore = FALSE;
	} else {
	}
	WriteCedPreference();
}

- (IBAction)optionsdoMixedSTWaveClicked:(NSButton *)sender // 2021-05-15 beg
{
#pragma unused (sender)
	Document *cDoc;
	DocumentWindowController *winCtrl;
	NSUInteger i, docsCnt, winCtrlCount;
	NSArray *documents;

	winCtrl = nil;
	if ([doMixedSTWaveButton state] == NSControlStateValueOn) {
		if (doMixedSTWave != TRUE) {
			documents = [[NSDocumentController sharedDocumentController] documents];
			docsCnt = [documents count];
			for (i=0; i < docsCnt; i++) {
				cDoc = [documents objectAtIndex:i];
				if (cDoc != NULL && [cDoc get_wID] == DOCWIN) {
					winCtrlCount = [[cDoc windowControllers] count];
					for (i=0; i < winCtrlCount; i++) {             //firstObject
						winCtrl = [[cDoc windowControllers] objectAtIndex:i];
					}
				}
			}
		}
		if (winCtrl != nil) {
			if (winCtrl->SnTr.IsSoundOn == false)
				winCtrl = nil;
		}
		if (winCtrl != nil) {
			[winCtrl SonicMode:nil];
		}
		doMixedSTWave = TRUE;
		if (winCtrl != nil) {
			[winCtrl SonicMode:nil];
		}
	} else if ([doMixedSTWaveButton state] == NSControlStateValueOff) {
		if (doMixedSTWave != FALSE) {
			documents = [[NSDocumentController sharedDocumentController] documents];
			docsCnt = [documents count];
			for (i=0; i < docsCnt; i++) {
				cDoc = [documents objectAtIndex:i];
				if (cDoc != NULL && [cDoc get_wID] == DOCWIN) {
					winCtrlCount = [[cDoc windowControllers] count];
					for (i=0; i < winCtrlCount; i++) {             //firstObject
						winCtrl = [[cDoc windowControllers] objectAtIndex:i];
					}
				}
			}
		}
		if (winCtrl != nil) {
			if (winCtrl->SnTr.IsSoundOn == false)
				winCtrl = nil;
		}
		if (winCtrl != nil) {
			[winCtrl SonicMode:nil];
		}
		doMixedSTWave = FALSE;
		if (winCtrl != nil) {
			[winCtrl SonicMode:nil];
		}
	} else {
	}
	WriteCedPreference();
} // 2021-05-15 end

- (IBAction)disambiguateFieldChanged:(id)sender {
//	if ([@"" isEqual:[sender stringValue]])
//		return;
	[self optionsCloseClicked:self];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	BOOL isChange;
	NSUInteger len;
	NSString *comStr;
	unichar st[BUFSIZ];

	if ( [ notification object ] == disambiguateField ) {
		comStr = [disambiguateField stringValue];
		len = [comStr length];
		if (len < BUFSIZ) {
			[comStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			isChange = false;
			if (len >= 48) {
				st[48] = EOS;
				[disambiguateField setStringValue:[NSString stringWithCharacters:st length:strlen(st)]];
				NSBeep();
			} else {
				if (st[0] != '%') {
					DisTier[0] = '%';
					u_strcpy(DisTier+1, st, 50);
				} else
					u_strcpy(DisTier, st, 50);
				len = strlen(DisTier);
				if (len > 0) {
					if (DisTier[len-1] != ':')
						strcat(DisTier, ":");
				}
				uS.uppercasestr(DisTier, &dFnt, C_MBF);
				WriteCedPreference();
			}
		}
	}
}
- (IBAction)optionsCloseClicked:(NSButton *)sender
{
#pragma unused (sender)

	[self close];
//	[self release];
}

// awakeFromNib is called when this object is done being unpacked from the nib file;
// at this point, we can do any needed initialization before turning app control over to the user
- (void)awakeFromNib
{
	int i;

	i = 12;
	// We don't actually need to do anything here, so it's empty
}

- (IBAction)resetOptions:(NSButton *)sender { // 2021-08-20 beg
#pragma unused (sender)

	int  ans, t;
	char message[128]; 
	FNType new_name[FNSize];
	
	t = strlen(prefsDir);
	if (t == 0)
		return;
	strcpy(message, "Are you sure you want to reset all options? in: ");
	strcat(message, prefsDir);
	ans = QueryDialog(message, 147);
	if (ans > 0) {
		isSavePrefs = FALSE;
		addFilename2Path(prefsDir, CED_PREF_FILE);
		strcpy(new_name, prefsDir);
		strcat(new_name, ".bck");
		unlink(new_name);
		rename(prefsDir, new_name);
		prefsDir[t] = EOS;
		WriteCedPreference();
		addFilename2Path(prefsDir, CLAN_PREF_FILE);
		strcpy(new_name, prefsDir);
		strcat(new_name, ".bck");
		unlink(new_name);
		rename(prefsDir, new_name);
		prefsDir[t] = EOS;
		WriteClanPreference();
		do_warning("Please quit CLAN completely when ready and re-start CLAN again.", -1);
	}
	prefsDir[t] = EOS;
}// 2021-08-20 end


@end
