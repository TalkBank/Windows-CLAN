/*
     File: Document.m
 Abstract: Document object for TextEdit. 
 
  Version: 1.9
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2013 Apple Inc. All Rights Reserved.
 
 */
/*
 Usually, you should not need to subclass NSDocumentController. Almost anything that can be done by
 subclassing can be done just as easily by the application’s delegate. However, it is possible to
 subclass NSDocumentController if you need to.

 For example, if you need to customize the Open panel, an NSDocumentController subclass is clearly needed.
 You can override the NSDocumentController method runModalOpenPanel:forTypes: to customize the panel or add
 an accessory view. The addDocument: and removeDocument: methods are provided for subclassers that want to
 know when documents are opened or closed.
*/
/*
139  = "Warning"
142  = "Input File SF"
145  = "Progress"
155  = "Warning-Quit"
500  = "Movie"
501  = "Commands"
504  = "Picture"
505  = "Movie Help"
506  = "Walker Conroller"
1962 = "text"
1964 = "clan output"
1965 = "Movie ThumbNail"
1966 = "%Txt"
2000 = "recall"
2001 = "Clan Progs"
2002 = "Help Commands"
2007 = "Special Chars"
2009 = "Web Data"
2010 = "Clan Folders"
 */


#include <Foundation/NSString.h>
#import "Document.h"
#import "DocumentController.h"
#import "DocumentWinController.h"
#import "PrintPanelAccessoryController.h"
#import "PrintingTextView.h"
#import "AppDefaultsKeys.h"
#import "AppErrors.h"
#import "AppMisc.h"
#import "c_clan.h"
#import <objc/message.h> // objc_msgSend


#define oldEditPaddingCompensation 12.0

extern char isMORXiMode;
extern DocumentWindowController *getDocWinController(NSWindow *win);
extern NSFont *defUniFont;
extern NSFont *defCAFont;

extern "C"
{
	extern void CleanUpAll(char all);
}


static char *bArgv[10];
static char fbuffer[UTTLINELEN];

char  tComWinDataChanged;

@implementation ProgressController

- (id)init {
	if ((self = [super init])) {
		// initialization
	}
	return self;
	//	return [super initWithWindowNibName:@"Commands"];
}
- (void)windowDidLoad {
	int j;

	j = 0;
}

@end


/* Menu validation: Arbitrary numbers to determine the state of the menu items whose titles change. Speeds up the validation... Not zero. */
#define TagForFirst 42
#define TagForSecond 43

static void validateToggleItem(NSMenuItem *aCell, BOOL useFirst, NSString *first, NSString *second) {
	if (useFirst) {
		if ([aCell tag] != TagForFirst) {
			[aCell setTitleWithMnemonic:first];
			[aCell setTag:TagForFirst];
		}
	} else {
		if ([aCell tag] != TagForSecond) {
			[aCell setTitleWithMnemonic:second];
			[aCell setTag:TagForSecond];
		}
	}
}

/* Truncate string to no longer than truncationLength; should be > 10
 */
NSString *truncatedString(NSString *str, NSUInteger truncationLength) {
	NSUInteger len = [str length];
	if (len < truncationLength)
		return str;
	return [[str substringToIndex:truncationLength - 10] stringByAppendingString:@"\u2026"];	// Unicode character 2026 is ellipsis
}

/* Returns the default padding on the left/right edges of text views
 */
CGFloat defaultTextPadding(void) {
	static CGFloat padding = -1;
	if (padding < 0.0) {
		NSTextContainer *container = [[NSTextContainer alloc] init];
		padding = [container lineFragmentPadding];
		[container release];
	}
	return padding;
}

DocumentWindowController *getDocWinController(NSWindow *win) {
	int i;
	Document	*cDoc;
	NSUInteger	num;
	NSArray		*documents;
	DocumentWindowController *winCtrl;

	winCtrl = nil;
	documents = [[NSDocumentController sharedDocumentController] documents];
	num = [documents count];
	for (i=0; i < num; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc windowForSheet] == win) {
			num = [[cDoc windowControllers] count];
			for (i=0; i < num; i++) {             //firstObject
				winCtrl = [[cDoc windowControllers] objectAtIndex:i];
			}
			break;
		}
	}
/*
	num = [[cDoc windowControllers] count];
	for (i=0; i < num; i++) {             //firstObject
		winCtrl = [[cDoc windowControllers] objectAtIndex:i];
	}
*/
	return winCtrl;
}

NSWindow *getWindow(unsigned short wID) { // 2020-01-16
	int			i;
	Document	*cDoc;
	NSUInteger	num;
	NSArray		*documents;
	NSWindow	*window;

	documents = [[NSDocumentController sharedDocumentController] documents];
	num = [documents count];
	for (i=0; i < num; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == wID) // 2019-09-06
			break;
	}
	if (i >= num)
		window = NULL;
	else
		window = [cDoc windowForSheet];
	return(window);
}

static NSUInteger getOutputWindowLen(void) {
	int i;
	NSUInteger num;
	NSDocumentController *controller;
	NSArray *documents;
	Document *cDoc;
	NSTextStorage *text;

	documents = [[NSDocumentController sharedDocumentController] documents];
	num = [documents count];
	for (i=0; i < num; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == CLANWIN) // 2019-09-06
			break;
	}
	if (i >= num) {
		controller = [NSDocumentController sharedDocumentController];
		cDoc = [controller openUntitledDocumentAndDisplay:NO error:nil];
		if (cDoc != NULL) {
			[cDoc set_wID:CLANWIN]; // 2019-09-06
			[cDoc makeWindowControllers];
			[cDoc showWindows];
		}
	}
	if (cDoc != NULL) {
		text = [cDoc textStorage];
//		[text beginEditing];
		num = [text length];
//		[text endEditing];
	}
	return(num);
}

static char *getBatchArgs(char *com) {
	register int i;
	register char *endCom;

	for (i=0; i < 10; i++)
		bArgv[i] = NULL;

	i = 0;
	while (*com != EOS) {
		for (; *com == ' ' || *com == '\t'; com++) ;
		if (*com == EOS)
			break;
		endCom = NextArg(com);
		if (endCom == NULL)
			return(NULL);

		if (i >= 10) {
			do_warning("out of memory; Too many arguments.", 0);
			return(NULL);
		}
		bArgv[i++] = com;
		com = endCom;
	}
	return(com);
}

static char fixArgs(char *fname, char *com) {
	register int  i;
	register int  num;
	register char qt = 0;

	for (i=0; com[i] != EOS; i++) {
		if (com[i] == '\'' /*'*/ || com[i] == '"') {
			if (qt == 0)
				qt = com[i];
			else if (qt == com[i])
				qt = 0;
		} else if (qt == 0 && com[i] == '%' && isdigit(com[i+1]) && !isdigit(com[i+2])) {
			num = atoi(com+i+1) - 1;
			if (num < 0 || num > 9 || bArgv[num] == NULL) {
				sprintf(com, "Argument %%%d was not specified with batch file \"%s\".", num+1, fname);
				do_warning(com, 0);
				return(FALSE);
			}
			strcpy(com+i, com+i+2);
			uS.shiftright(com+i,(int)strlen(bArgv[num]));
			strncpy(com+i,bArgv[num],strlen(bArgv[num]));
			i = i + strlen(bArgv[num]) - 1;
		}
	}
	return(TRUE);
}

/* 2019-10-22 old version
void OutputToScreen(unCH *st) {
	int i;
	NSUInteger num, stLen;
	NSDocumentController *controller;
	NSArray *documents;
	Document *cDoc;
	NSTextStorage *text;
	NSString *myString;
	NSString *com;

	NSUInteger len;

	controller = [NSDocumentController sharedDocumentController];
	documents = [[NSDocumentController sharedDocumentController] documents];
	num = [documents count];
	for (i=0; i < num; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == CLANWIN) // 2019-09-06
			break;
	}
	if (i >= num) {
		cDoc = [controller openUntitledDocumentAndDisplay:NO error:nil];
		if (cDoc != NULL) {
			[cDoc set_wID:CLANWIN]; // 2019-09-06
			[cDoc makeWindowControllers];
			[cDoc showWindows];
		}
	}
	if (cDoc != NULL) {
		NSRange endRange;

		text = [cDoc textStorage];
 //[text replaceCharactersInRange:NSMakeRange(0, [text length]) withAttributedString:string];
 //		selectedRange = [myText selectedRange];

		[text beginEditing];
 //		myString = [text string];

		stLen = strlen(st);
		len = [text length];
		endRange = NSMakeRange(len, 0);
		com = [NSString stringWithCharacters:st length:stLen]; // NSUnicodeStringEncoding
		myString = [[NSString alloc]initWithString:@""];
		myString = [myString stringByAppendingString:com];

		[text replaceCharactersInRange:endRange withString:myString];

 //		[myText setString: myString];
 //		[text appendString:string];

 		[text endEditing];

// 2019-09-10		endRange = NSMakeRange(len+stLen, 0);
// 2019-09-10		[text edited:NSTextStorageEditedCharacters range:endRange changeInLength:stLen];
	}
}
*/

/*
static BOOL isDigitSpaceNoCR(unCH *st) {
	for (; *st != EOS; st++) {
		if (!isdigit(*st) && !isSpace(*st))
			return(false);
	}
	return(true);
}

static BOOL isSpaceR(unCH *st) {
	for (; *st != EOS; st++) {
		if (!isSpace(*st)) {
			if (*st == '\r')
				return(true);
			else
				return(false);
		}
	}
	return(true);
}
*/

void OutputToScreen(unCH *st) {
	int i;
//	int offset;
	unichar ch, *cSt;
	Document *cDoc;
	BOOL isIncrementPos, isCRFound;
	NSUInteger docsCnt, pos, tPos, len;
	NSRange  endRange;
	NSArray *documents;
	NSTextStorage *text;
	NSString *textSt;
	NSAttributedString *myAttSt;
	NSMutableDictionary *textAttributes;

/*
	offset = 0;
	if (st[0] == '\r') {
		if (isDigitSpaceNoCR(st+1)) {
			return;
		} if (isSpaceR(st+1)) {
			for (offset=0; st[offset] == '\r' || isSpace(st[offset]); offset++) ;
		} else {
			for (offset=0; st[offset] == '\r'; offset++) ;
		}
	}
*/

	documents = [[NSDocumentController sharedDocumentController] documents];
	docsCnt = [documents count];
	for (i=0; i < docsCnt; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == CLANWIN) // 2019-09-06
			break;
	}
	if (i >= docsCnt) {
		cDoc = [[NSDocumentController sharedDocumentController] openUntitledDocumentAndDisplay:NO error:nil];
		if (cDoc != NULL) {
			[cDoc set_wID:CLANWIN]; // 2019-09-06
			[cDoc makeWindowControllers];
			[cDoc showWindows];
		}
	}
	if (cDoc == NULL)
		return;
// 2020-07-29 beg
	cSt = strrchr(st, '\r');
	if (cSt != NULL) {
		cSt++;
		isCRFound = true;
	} else {
		cSt = st;
		isCRFound = false;
	}
// 2020-07-29 end

	textSt = [NSString stringWithCharacters:cSt length:strlen(cSt)]; // NSUnicodeStringEncoding
	textAttributes = [[NSMutableDictionary alloc] initWithCapacity:2];
//	textAttributes = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
	[textAttributes setObject:[cDoc docFont] forKey:NSFontAttributeName];
	myAttSt = [[NSAttributedString alloc] initWithString:textSt attributes:textAttributes];

	text = [cDoc textStorage];
	textSt = [text string];
	pos = [text length];
	[text beginEditing];
// 2020-07-29 beg
	if (isCRFound == false) {
		[text appendAttributedString:myAttSt];
	} else {
		len = [text length];
		pos = len - 1;
		while (pos > 0) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				pos++;
				break;
			}
			pos--;
		}
		if (pos >= len) {
			[text appendAttributedString:myAttSt];
		} else {
			endRange = NSMakeRange(pos, len-pos); // (NSUInteger pos, NSUInteger len)
			[text replaceCharactersInRange:endRange withAttributedString:myAttSt];
		}
	}
// 2020-07-29 end

	len = [text length];
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		isIncrementPos = true;
		if (ch == ATTMARKER) { // 2020-07-31 beg
			ch = [textSt characterAtIndex:pos+1];
			if (ch == error_start) {
				tPos = pos + 2;
				while (tPos < len) { // get speaker code in variable "suSt"
					ch = [textSt characterAtIndex:tPos];
					if (ch == '\n')
						break;
					if (ch == ATTMARKER) {
						ch = [textSt characterAtIndex:tPos+1];
						if (ch == error_end) {
							endRange = NSMakeRange(pos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:endRange withString:@""];
							tPos -= 2;
							endRange = NSMakeRange(tPos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:endRange withString:@""];
							endRange = NSMakeRange(pos, tPos-pos);
							[text addAttribute:NSForegroundColorAttributeName value:[NSColor redColor] range:endRange];
							textSt = [text string];
							len = [text length];
							ch = [textSt characterAtIndex:pos];
							isIncrementPos = false;
							break;
						}
					}
					tPos++;
				}
			}
		} // 2020-07-31 end
		if (isIncrementPos) {
			pos++;
		}
	}

	[text endEditing];
	[myAttSt release];
	[textAttributes release];



/* 2020-07-13
	NSRange endRange;
	[text beginEditing];
	endRange = NSMakeRange(textLen, 0);
	[text replaceCharactersInRange:endRange withAttributedString:myAttSt];

//	NSRange textStRange = NSMakeRange(textLen, [textSt length]);
//	[text replaceCharactersInRange:endRange withString:textSt];
//	[text setAttributes:textAttributes range:textStRange];
	[text endEditing];
*/

// 2019-10-22
	NSLayoutManager *tLayoutMgr;
	NSTextContainer *textContainer;
	NSTextView *textView;
	NSRange theTextRange;
	NSRect layoutRect;
	NSPoint containerOrigin;
	NSPoint cursorPoint;
	NSEvent *theEvent; // 2020-07-21
	NSRunLoop *theRL; // 2020-07-21
	unsigned short keyCode;// 2020-07-21
	NSUInteger textLen;

 	textLen = [text length];
	endRange = NSMakeRange(textLen, 0);
	for (tLayoutMgr in [text layoutManagers]) {
		for (textContainer in [tLayoutMgr textContainers]) {
			textView = [textContainer textView];
			if (textView) {
				theTextRange = [[textView layoutManager] glyphRangeForCharacterRange:endRange actualCharacterRange:NULL];
				layoutRect = [[textView layoutManager] boundingRectForGlyphRange:theTextRange inTextContainer:[textView textContainer]];
				containerOrigin = [textView textContainerOrigin];
				cursorPoint.x = layoutRect.origin.x + containerOrigin.x;
				cursorPoint.y = layoutRect.origin.y + containerOrigin.y;
				theTextRange.length = 0;
				[textView setSelectedRange:theTextRange];
				[textView scrollPoint:cursorPoint];
				[textView setNeedsDisplay:YES];
				textView.needsDisplay = YES;
				[textView display];
//				break;
			}
		}
	}

// 2020-07-21 beg
	theRL = [NSRunLoop currentRunLoop];
	[theRL limitDateForMode:NSDefaultRunLoopMode];
//	NSDate *nextDate = [theRL limitDateForMode:NSDefaultRunLoopMode];
//	[theRL runMode:NSDefaultRunLoopMode beforeDate:nextDate];
//	NSWindow *win = getWindow(CLANWIN);
//	NSEvent *theEvent = [win currentEvent];
//	NSEvent *theEvent = [NSApp currentEvent];
	theEvent = [NSApp nextEventMatchingMask:NSEventMaskKeyDown untilDate:[NSDate dateWithTimeIntervalSinceNow:0.000001] inMode:NSDefaultRunLoopMode dequeue:YES];
	if ([theEvent type] == NSKeyDown) {
		keyCode = [theEvent keyCode];
		if ([theEvent modifierFlags] & NSCommandKeyMask && keyCode == 47) {
			isKillProgram = 1;
		}
	}
// 2020-07-21 end
}

void RunCommand(NSString *comStr) {
	NSInteger comLen;
	NSRange aRange;
	unichar *bufU;
	FILE	*fp;

	comLen = [comStr length];
	if (StdInErrMessage != NULL) { // 2020-08-13
		do_warning(StdInErrMessage, 0);
		return;
	}
	bufU = (unichar *)malloc((comLen*sizeof(unichar))+1);
	if (bufU == NULL)
		return;
	aRange.location = 0;
	aRange.length = comLen;
	[comStr getCharacters:bufU range:aRange];
	bufU[comLen] = 0;
	isKillProgram = 0;
	uS.remFrontAndBackBlanks(bufU);
	u_strcpy(fbuffer, bufU, UTTLINELEN);
	if (fbuffer[0] != EOS) {
		AddToClan_commands(fbuffer);
//		set_CommandsInfo(win, TRUE);
//		if (global_df) {
			global_df = NULL;
//			global_df->AutoWrap = ClanAutoWrap;
//			if (isUTFData && !dFnt.isUTF) {
//				SetDefaultUnicodeFinfo(&dFnt);
//				dFnt.fontTable = NULL;
//				dFnt.isUTF = 1;
//			}
//			dFnt.charHeight = GetFontHeight(NULL, &dFnt, win);
//			dFnt.Encod = my_FontToScript(dFnt.fontId, dFnt.CharSet);
//			dFnt.orgEncod = dFnt.Encod;
//			copyNewFontInfo(&oFnt, &dFnt);
//			EndOfFile(1);
//			SetScrollControl();
//			ResetUndos();
//			copyNewToFontInfo(&global_df->row_txt->Font, &dFnt);
//			global_df->attOutputToScreen = 0;
			if (getOutputWindowLen() == 0) {
				OutputToScreen(cl_T("> "));
				tComWinDataChanged = FALSE;
			}
//			dFnt.isUTF = isUTFData;
			if (!uS.mStrnicmp(fbuffer, "batch ", 6) || !uS.mStrnicmp(fbuffer, "bat ", 4)) {
				char *com, *eCom;
				FNType mDirPathName[FNSize];

				uS.remFrontAndBackBlanks(bufU);
				OutputToScreen(bufU);
				for (com=fbuffer; *com != EOS && *com != ' ' && *com != '\t'; com++) ;
				for (; *com == ' ' || *com == '\t'; com++) ;
				eCom = com;
				if (*com != EOS) {
					for (; *eCom != EOS && *eCom != ' ' && *eCom != '\t'; eCom++) ;
					if (*eCom != EOS) {
						*eCom = EOS;
						eCom++;
					}
				}
				strcpy(mDirPathName, wd_dir);
				addFilename2Path(mDirPathName, com);
				if ((fp=fopen(mDirPathName, "r")) == NULL) {
					strcpy(mDirPathName, lib_dir);
					addFilename2Path(mDirPathName, com);
					if ((fp=fopen(mDirPathName, "r")) == NULL) {
						sprintf(ced_lineC, "Can't find batch file \"%s\" in either working or library directories", com);
						do_warning(ced_lineC, 0);
					}
				}
				if (fp != NULL) {
					if ((eCom=getBatchArgs(eCom)) != NULL) {
						eCom++;
						while (fgets_ced(eCom, 1024, fp, NULL)) {
							uS.remFrontAndBackBlanks(eCom);
							if (uS.isUTF8(eCom) || uS.partcmp(eCom,FONTHEADER,FALSE,FALSE) || eCom[0] == '%' || eCom[0] == '#' || eCom[0] == EOS)
								continue;
							if (!fixArgs(com, eCom))
								break;
							if (*eCom == EOS ||
								((char)toupper((unsigned char)eCom[0]) == 'T' && (char)toupper((unsigned char)eCom[1]) == 'Y') ||
								((char)toupper((unsigned char)eCom[0]) == 'P' && (char)toupper((unsigned char)eCom[1]) == 'A')) ;
							else {
								OutputToScreen(cl_T("\nBATCH> "));
								execute(eCom, tComWinDataChanged);
								if (isKillProgram) {
									if (isKillProgram != 2)
										OutputToScreen(cl_T("\n    BATCH FILE ABORTED\n"));
									break;
								}
							}
						}
					}
					fclose(fp);
				}
			} else
				execute(fbuffer, tComWinDataChanged);

//			copyNewFontInfo(&dFnt, &oFnt);
			if (isKillProgram)
				isKillProgram = 0;
//			if (global_df != NULL && global_df->winID == 1964)
//				OpenCommandsWindow(TRUE);
			if (!isMORXiMode)
				OutputToScreen(cl_T("\n> "));
//			tComWinDataChanged;
//		}
	}
	free(bufU);
}

// 2020-07-31 beg
static void setCursorToSpeaker(Document *cDoc, long num) {
	unichar ch, lastCh;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger len, pos;

	cDoc->topC = -1L;
	cDoc->skipTop = -1L;
	cDoc->pos1C = 0L;
	cDoc->skipP1 = 0L;
	cDoc->pos2C = 0L;
	cDoc->skipP2 = 0L;

	text = [cDoc textStorage];
	textSt = [text string];
	len = [text length];
	lastCh = '\n';
	pos = 0;
	while (pos < len) { // get speaker code in variable "suSt"
		ch = [textSt characterAtIndex:pos];
		if (lastCh == '\n' && ch == '*') {
			num--;
		}
		if (num == 0) {
			cDoc->pos1C = pos;
			while (pos < len) {
				ch = [textSt characterAtIndex:pos];
				if (ch == '\n')
					break;
				pos++;
			}
			pos++;
			if (pos >= len)
				pos--;
			cDoc->pos2C = pos;
			break;
		}
		lastCh = ch;
		pos++;
	}
}

static void setCursorToLine(Document *cDoc, long num) {
	unichar ch;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger len, pos, begPos;

	cDoc->topC = -1L;
	cDoc->skipTop = -1L;
	cDoc->pos1C = 0L;
	cDoc->skipP1 = 0L;
	cDoc->pos2C = 0L;
	cDoc->skipP2 = 0L;

	text = [cDoc textStorage];
	textSt = [text string];
	len = [text length];
	begPos = 0;
	pos = 0;
	while (pos < len) { // get speaker code in variable "suSt"
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			num--;
		}
		if (num == 0) {
			begPos++;
			if (begPos >= len)
				begPos--;
			cDoc->pos1C = begPos;
			pos++;
			if (pos >= len)
				pos--;
			cDoc->pos2C = pos;
			break;
		}
		if (ch == '\n') {
			begPos = pos;
		}
		pos++;
	}
}

static void lExtractFileName(unCH *line, char *path, FNType *fname, char term) {
	long  i;
	FNType mFileName[FNSize];

	for (i=0L; line[i] != term && line[i] != EOS; i++) ;
	if (line[i] == term) {
		line[i] = EOS;
		if (!strcmp(line, "Stdin"))
			return;

		if (!isRefEQZero(line)) {
			u_strcpy(fname, line, FNSize);
		} else {
			u_strcpy(mFileName, line, FNSize);
			extractPath(fname, path);
			addFilename2Path(fname, mFileName);
			if (my_access(fname, 0)) {
				strcpy(fname, wd_dir);
				addFilename2Path(fname, mFileName);
			}
		}
		strcpy(line, line+i+1);
	}
}

char FindFileLine(char isTest, char *fPath, unCH *text) {
	int i;
	FNType fname[FNSize], *ext;
	char isZeroFound = FALSE, isSpeakerNumber;
	unCH *line;
	long len;
	long ln = 0L;
	Document *cDoc;
	NSString *nFName;
	NSUInteger docsCnt;
	NSArray *documents;
	BOOL tIsCursorPosRestore;
	extern BOOL isCursorPosRestore;
//	extern char isDontAskDontTell;

	if (text == NULL)
		return(FALSE);
	strncpy(ced_line, text, UTTLINELEN);
	ced_line[UTTLINELEN] = EOS;
	if (ced_line[0] == '0') {
		isZeroFound = TRUE;
		strcpy(ced_line, ced_line+1);
	}
	len = (long)strlen("@Comment:	");
	if (strncmp(ced_line, "@Comment:	", (size_t)len) == 0)
		strcpy(ced_line, ced_line+len);
	line = ced_line;
	fname[0] = EOS;
	if (strncmp(line, "From file <", (size_t)11) == 0) {
		strcpy(line, line+11);
		lExtractFileName(line, fPath, fname, '>');
	} else if (strncmp(line, "Output file <", (size_t)13) == 0) {
		strcpy(line, line+13);
		lExtractFileName(line, fPath, fname, '>');
	} else {
		len = 0;
		if (line[0] == '\t' && line[1] == '*') {
			len++;
		}
		if (*line == '*' || *line == ' ' || *line == '\t') {
			for (; line[len] == '*'; len++) ;
			for (; line[len] == ' '; len++) ;
			strcpy(line, line+len);
			len = strlen("File \"");
			if (strncmp(line, "File \"", len) == 0) {
				strcpy(line, line+len);
				lExtractFileName(line, fPath, fname, '"');
			}
			if (*fname == EOS)
				return(FALSE);

			for (len=0L; line[len] == ':' || line[len] == ' '; len++) ;
			if (len > 0L)
				strcpy(line, line+len);
			len = strlen("line ");
			if (strncmp(line, "line ", len) == 0) {
				strcpy(line, line+len);
				ln = uS.atol(line);
				isSpeakerNumber = FALSE;
			} else {
				len = strlen("speaker ");
				if (strncmp(line, "speaker ", len) == 0) {
					strcpy(line, line+len);
					ln = uS.atol(line);
					isSpeakerNumber = TRUE;
				}
			}
		} else
			return(FALSE);
	}

	if (*fname != EOS && !isTest) {
		if (my_access(fname, 0)) {
			sprintf(templineC, "Can't open file \"%s\".", fname);
			do_warning(templineC, 0);
			return(FALSE);
		}
//		isAjustCursor = FALSE;
		ext = strrchr(fname, '.');
		if (ext != NULL && !strcmp(ext, ".xls")) {
			strcpy(FileName2, "open \"");
			strcat(FileName2, fname);
			strcat(FileName2, "\"");
			if (!system(FileName2))
				return(TRUE);
		}
//		if (isZeroFound)
//			isDontAskDontTell = TRUE;

		documents = [[NSDocumentController sharedDocumentController] documents];
		docsCnt = [documents count];
		for (i=0; i < docsCnt; i++) {
			cDoc = [documents objectAtIndex:i];
			if (cDoc != NULL) {
				if ([cDoc get_wID] == DOCWIN && uS.mStricmp(cDoc->filePath, fname) == 0)
					break;
			}
		}
		if (i >= docsCnt) {
			nFName = [NSString stringWithUTF8String:fname];
			cDoc = [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[NSURL fileURLWithPath:nFName] display:NO error:nil];
			if (cDoc != NULL) {
				[cDoc makeWindowControllers];
			}
		}
		if (cDoc == NULL)
			return(FALSE);
		if (ln > 0L) {
			if (isSpeakerNumber)
				setCursorToSpeaker(cDoc, ln);
			else
				setCursorToLine(cDoc, ln);
		}
		tIsCursorPosRestore = isCursorPosRestore;
		isCursorPosRestore = FALSE;
		[cDoc showWindows];
		isCursorPosRestore = tIsCursorPosRestore;

		dispatch_async(dispatch_get_main_queue(), ^{
			NSTextStorage *text;
			NSLayoutManager *tLayoutMgr;
			NSTextContainer *textContainer;
			NSTextView *textView;
			NSRange  cursorRange;

			cursorRange = NSMakeRange(cDoc->pos1C+cDoc->skipP1, cDoc->pos2C+cDoc->skipP2-cDoc->pos1C+cDoc->skipP1);
			text = [cDoc textStorage];
			for (tLayoutMgr in [text layoutManagers]) {
				for (textContainer in [tLayoutMgr textContainers]) {
					textView = [textContainer textView];
					if (textView) {
						[textView setSelectedRange:cursorRange];
						[textView scrollRangeToVisible:cursorRange]; // 2020-05-13
						[NSApp activateIgnoringOtherApps:YES];
					}
				}
			}
		});
//		isDontAskDontTell = FALSE;
	}
	return(TRUE);
}
// 2020-07-31 end

NSUInteger getUtts(NSUInteger pos, NSUInteger len, NSString *textSt) {
	NSUInteger i;
	unichar ch;

	templine[0] = EOS;
	for (i=0; pos < len && i < UTTLINELEN; i++) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (i < UTTLINELEN)
			templine[i] = ch;
		if (ch == '\n') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
				if (ch == '*' || ch == '%' || ch == '@') {
					pos++;
					i++;
					break;
				}
			}
		}
		pos++;
	}
	templine[i] = EOS;

	for (i=0; templine[i] != EOS && templine[i] != ':' && i < SPEAKERLEN-2; i++)
		sp[i] = templine[i];
	if (templine[i] == ':') {
		sp[i] = ':';
		i++;
	}
	sp[i] = EOS;
	for (; isSpace(templine[i]); i++) ;
	strcpy(ced_line, templine+i);
	return(pos);
}



@implementation Document

+ (NSString *)readableTypeForType:(NSString *)type {
	// There is a partial order on readableTypes given by UTTypeConformsTo. We linearly extend the partial order to a total order using <.
	// Therefore we can compute the ancestor with greatest level (furthest from root) by linear search in the resulting array.
	// Why do we have to do this?  Because type might conform to multiple readable types, such as "public.rtf" and "public.text" and "public.data"
	// and we want to find the most specialized such type.
	static NSArray *topologicallySortedReadableTypes;
	static dispatch_once_t pred;
	dispatch_once(&pred, ^{
		topologicallySortedReadableTypes = [self readableTypes];
		topologicallySortedReadableTypes = [topologicallySortedReadableTypes sortedArrayUsingComparator:^NSComparisonResult(id type1, id type2) {
			if (type1 == type2)
				return NSOrderedSame;
			if (UTTypeConformsTo((CFStringRef)type1, (CFStringRef)type2))
				return NSOrderedAscending;
			if (UTTypeConformsTo((CFStringRef)type2, (CFStringRef)type1))
				return NSOrderedDescending;
			return (((NSUInteger)type1 < (NSUInteger)type2) ? NSOrderedAscending : NSOrderedDescending);
		}];
		[topologicallySortedReadableTypes retain];
	});
	for (NSString *readableType in topologicallySortedReadableTypes) {
		if (UTTypeConformsTo((CFStringRef)type, (CFStringRef)readableType)) return readableType;
	}
	return nil;
}

- (id)init {
	if ((self = [super init])) {
		[[self undoManager] disableUndoRegistration];

		rawTextInput = false;
		ShowPercentOfFile = false;
		isCAFont = false;
		wID = NEWDOCWIN;
		mediaFileName[0] = EOS;
		pidSt[0] = EOS;
		RootColorText = nil;
		top = 0L;
		left = 0L;
		height = 0L;
		width = 0L;
		topC = 0L;
		skipTop = 0L;
		pos1C = 0L;
		skipP1 = 0L;
		pos2C = 0L;
		skipP2 = 0L;
		filePath[0] = EOS;
		docFont = nil;
		textStorage = [[NSTextStorage allocWithZone:[self zone]] init];

		[self setBackgroundColor:[NSColor whiteColor]];
		[self setEncodingForSaving:NoStringEncoding];
		inDuplicate = NO;

		// Assume the default file type for now, since -initWithType:error: does not currently get called when creating documents using AppleScript. (4165700)
		[self setFileType:[[NSDocumentController sharedDocumentController] defaultType]];

		[self setPrintInfo:[self printInfo]];


		[self setDocUsesScreenFonts:NO]; // YES];

		[[self undoManager] enableUndoRegistration];
	}
	return self;
}

/* 2019-10-02 beg
- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError * _Nullable *)outError {
#pragma unused (typeName, outError)
	char buf[BUFSIZ];
	NSUInteger lenText, loc;
	NSRange  endRange;
	NSString *bulletStr, *textSt, *headerSt;
	NSTextStorage *text;
	NSData *data;
	BOOL writeSuccess;

	NSLog(@"Document: writeToURL\n");
// 2020-05-09 beg
	NSLayoutManager *tLayoutMgr;
	NSTextContainer *textContainer;
	NSTextView *textView;
	NSValue *cursorValue;
	NSRange cursorRange;

 	text = [self textStorage];
	for (tLayoutMgr in [text layoutManagers]) {
		for (textContainer in [tLayoutMgr textContainers]) {
			textView = [textContainer textView];
			if (textView) {
				cursorValue = [[textView selectedRanges] objectAtIndex:0];
				cursorRange = [cursorValue rangeValue];
				topC = -1L;
				skipTop = -1L;
				pos1C = cursorRange.location;
				skipP1 = 0L;
				pos2C = pos1C + cursorRange.length;
				skipP2 = 0L;
			}
		}
	}

// 2020-05-09 end

//1	bulletStr = [[NSString alloc]initWithString:@"\25"];
	bulletStr = [NSString stringWithFormat:@"%C", 0x0015]; // \25 - 0x0015
	text = [self textStorage];
	textSt = [text string];
	loc = 0;
	lenText = [text length];
	[text beginEditing];
	while (loc < lenText) {	// Run through the string in terms of attachment runs
		unichar ch = [textSt characterAtIndex:loc];
		if (ch == 0x2022) {
			endRange = NSMakeRange(loc, 1); // (NSUInteger loc, NSUInteger len)
			[text replaceCharactersInRange:endRange withString:bulletStr];
		}
		loc++;
	}
	[text endEditing];
	writeSuccess = YES;
//1	[bulletStr release]; // dealloc

	strcpy(buf, UTF8HEADER);
	strcat(buf, "\n");
	if (pidSt[0] != EOS) {// 2020-05-08 beg
		strcat(buf, PIDHEADER);
		strcat(buf, "\t");
		strcat(buf, pidSt);
		strcat(buf, "\n");// 2020-05-08 end
	}
	strcat(buf, WINDOWSINFO); // 2020-03-13 beg
	strcat(buf, "\t");
	sprintf(buf+strlen(buf), "%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld",
			top, left, height, width, topC, skipTop, pos1C, skipP1, pos2C, skipP2);
	strcat(buf, "\n");        // 2020-03-13 end
	headerSt = [[NSString alloc] initWithUTF8String:buf];
	textSt = [headerSt stringByAppendingString:textSt];
	data = [textSt dataUsingEncoding:NSUTF8StringEncoding];
	if (data) {
		writeSuccess = [data writeToURL:absoluteURL atomically:true];

		if ([typeName isEqualToString:(NSString *)kUTTypeCHAT]) // 2020-01-02
			writeSuccess = [super writeToURL:absoluteURL ofType:(NSString *)kUTTypeText error:outError];
		else
			writeSuccess = [super writeToURL:absoluteURL ofType:typeName error:outError];

	} else {
		writeSuccess = NO;
	}
//	[headerSt release];

//2	bulletStr = [[NSString alloc]initWithString:@"•"]; // 0x2022 - E2 80 A2
//	bulletStr = [NSString stringWithUTF8String:"\xe2\x80\xa2"]; // • - 0x2022 - E2 80 A2
	bulletStr = [NSString stringWithFormat:@"%C", 0x2022]; // • - 0x2022 - E2 80 A2
	[text beginEditing];
	loc = 0;
	while (loc < lenText) {	// Run through the string in terms of attachment runs
		unichar ch = [textSt characterAtIndex:loc];
		if (ch == HIDEN_C) {
			endRange = NSMakeRange(loc, 1); // (NSUInteger loc, NSUInteger len)
			[text replaceCharactersInRange:endRange withString:bulletStr];
		}
		loc++;
	}
	[text endEditing];

//2	[bulletStr release]; // dealloc
// 2020-05-09 beg

	text = [self textStorage];
	for (tLayoutMgr in [text layoutManagers]) {
		for (textContainer in [tLayoutMgr textContainers]) {
			textView = [textContainer textView];
			if (textView) {
				[textView setSelectedRange:cursorRange];
			}
		}
	}

// 2020-05-09 end

	return writeSuccess;
}
*/
/* 2019-10-02 end */

/*
// 2019-10-03 beg
	NSUInteger lenSt;
	NSMutableData *dataM;
	NSFileHandle *file;
	NSFileManager *fileManager;

	const char *pathSt;
	NSString *pathNSt;
	NSURL *URL = [self fileURL];
	pathSt = [URL fileSystemRepresentation];
	pathSt = [absoluteURL fileSystemRepresentation];
	pathNSt = [URL path];
	pathNSt = [absoluteURL path];
	fileManager = [NSFileManager defaultManager];
	if ([fileManager fileExistsAtPath:pathNSt] == NO) {
		[fileManager createFileAtPath:pathNSt contents:nil  attributes:nil];
	}

	file = [NSFileHandle fileHandleForWritingAtPath:pathNSt];
//	file = [NSFileHandle fileHandleForWritingToURL:absoluteURL error:outError];
	if (file != nil && *outError == nil) {
		const char *st;
		st = [textSt UTF8String];
		lenSt = strlen(st);
		dataM = [NSMutableData dataWithBytes:st length:lenSt];
		[file writeData:dataM];
		[file closeFile];
		writeSuccess = YES;
	} else
	writeSuccess = NO;
// 2019-10-03 end
 */


// 2019-09-10 2020-05-10 beg
- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError * _Nullable *)outError {
#pragma unused (typeName, outError)
	char buf[BUFSIZ];
	unichar ch;
	cCOLORTEXTLIST *tCT;
	NSUInteger len, pos, tPos;
	NSRange  cursorRange, posRange;
	NSString *textSt, *headerSt, *attStr;
	NSTextStorage *text;
	NSData *data;
	BOOL writeSuccess = NO, isIncrementPos;

	NSLog(@"Document: writeToURL\n");

	text = [self textStorage];
// 2020-05-09 beg
	NSLayoutManager *tLayoutMgr;
	NSTextContainer *textContainer;
	NSTextView *textView;
	NSValue *cursorValue;

	for (tLayoutMgr in [text layoutManagers]) {
		for (textContainer in [tLayoutMgr textContainers]) {
			textView = [textContainer textView];
			if (textView) {
				cursorValue = [[textView selectedRanges] objectAtIndex:0];
				cursorRange = [cursorValue rangeValue];
				topC = -1L;
				skipTop = -1L;
				pos1C = cursorRange.location;
				skipP1 = 0L;
				pos2C = pos1C + cursorRange.length;
				skipP2 = 0L;
			}
		}
	}
// 2020-05-09 end

	[text beginEditing]; // 2020-07-30 beg

// 2022-01-27 beg
	textSt = [text string];
	len = [text length];
	pos = 0;
	while (pos < len) { // get speaker code in variable "suSt"
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			tPos = pos;
			while (tPos > 0) {
				ch = [textSt characterAtIndex:tPos-1];
				if (ch != ' ' && ch != '\t')
					break;
				tPos--;
			}
			if (tPos < pos) {
				if (pos1C > pos)
					pos1C -= (pos - tPos);
				if (pos2C > pos)
					pos2C -= (pos - tPos);
				posRange = NSMakeRange(tPos, (pos - tPos));
				[text replaceCharactersInRange:posRange withString:@""];
				textSt = [text string];
				len = [text length];
			}
			pos = tPos + 1;
		} else
			pos++;
	}
	if (pos >= len) {
		tPos = pos;
		while (tPos > 0) {
			ch = [textSt characterAtIndex:tPos-1];
			if (ch != ' ' && ch != '\t')
				break;
			tPos--;
		}
		if (tPos < pos) {
			if (pos1C > pos)
				pos1C -= (pos - tPos);
			if (pos2C > pos)
				pos2C -= (pos - tPos);
			posRange = NSMakeRange(tPos, (pos - tPos));
			[text replaceCharactersInRange:posRange withString:@""];
			textSt = [text string];
			len = [text length];
		}
		if (tPos > len)
			tPos = len;
		if (tPos > 0) {
			ch = [textSt characterAtIndex:tPos-1];
			if (ch != '\n') {
				posRange = NSMakeRange(tPos, 0);
				[text replaceCharactersInRange:posRange withString:@"\n"];
			}
		}
	}
// 2022-01-27 end

	textSt = [text string];
	len = [text length];
	pos = 0;
	NSColor *color;
	while (pos < len) { // get speaker code in variable "suSt"
		color = [text attribute:NSForegroundColorAttributeName atIndex:pos effectiveRange:&cursorRange];
/*
		CGFloat errRed, errGreen, errBlue, errAlpha;
		CGFloat red, green, blue, alpha;
		NSColor *color;
		[[NSColor redColor] getRed:&errRed green:&errGreen blue:&errBlue alpha:&errAlpha];
		color = [text attribute:NSForegroundColorAttributeName atIndex:pos effectiveRange:&range];
		if (color != nil) {
			[color getRed:&red green:&green blue:&blue alpha:&alpha];
		}
*/
		if (color == [NSColor redColor]) {
			attStr = [NSString stringWithFormat:@"%c%c", ATTMARKER, error_start];
			posRange = NSMakeRange(pos, 0);
			[text replaceCharactersInRange:posRange withString:attStr];
			if (pos1C > pos)
				pos1C += 2;
			if (pos2C > pos)
				pos2C += 2;
			attStr = [NSString stringWithFormat:@"%c%c", ATTMARKER, error_end];
			pos = pos + cursorRange.length + 2;
			posRange = NSMakeRange(pos, 0);
			[text replaceCharactersInRange:posRange withString:attStr];
			if (pos1C > pos)
				pos1C += 2;
			if (pos2C > pos)
				pos2C += 2;
			pos += 2;
			len = [text length];
		} else
			pos++;
	}

	textSt = [text string];
	len = [text length];
	pos = 0;
	id isUnderlined;
	NSInteger val;
	while (pos < len) { // get speaker code in variable "suSt"
		isUnderlined = [text attribute:NSUnderlineStyleAttributeName atIndex:pos effectiveRange:&cursorRange];
		val = [isUnderlined integerValue];
		if (val != 0) {
			attStr = [NSString stringWithFormat:@"%c%c", ATTMARKER, underline_start];
			posRange = NSMakeRange(pos, 0);
			[text replaceCharactersInRange:posRange withString:attStr];
			if (pos1C > pos)
				pos1C += 2;
			if (pos2C > pos)
				pos2C += 2;
			attStr = [NSString stringWithFormat:@"%c%c", ATTMARKER, underline_end];
			pos = pos + cursorRange.length + 2;
			posRange = NSMakeRange(pos, 0);
			[text replaceCharactersInRange:posRange withString:attStr];
			if (pos1C > pos)
				pos1C += 2;
			if (pos2C > pos)
				pos2C += 2;
			pos += 2;
			len = [text length];
		} else
			pos++;
	}
	[text endEditing];  // 2020-07-30 beg

// 2020-05-20 beg
//	bulletStr = [[NSString alloc]initWithString:@"•"]; // 0x2022 - E2 80 A2
//	bulletStr = [NSString stringWithString:@"•"]; // 0x2022 - E2 80 A2
//	bulletStr = [NSString stringWithUTF8String:"\xe2\x80\xa2"]; // • - 0x2022 - E2 80 A2
//	bulletStr = [NSString stringWithFormat:@"%C", 0x2022]; // • - 0x2022 - E2 80 A2
	textSt = [textSt stringByReplacingOccurrencesOfString:@"•" withString:@"\x15"]; // 0x2022 - E2 80 A2

	strcpy(buf, UTF8HEADER);
	strcat(buf, "\n");
	if (pidSt[0] != EOS) {// 2020-05-08 beg
		strcat(buf, PIDHEADER);
		strcat(buf, "\t");
		strcat(buf, pidSt);
		strcat(buf, "\n");// 2020-05-08 end
	}
	if (RootColorText != nil) {
		strcat(buf, CKEYWORDHEADER);
		strcat(buf, "\t");
		for (tCT=RootColorText; tCT != NULL; tCT=tCT->nextCT) {
			u_strcpy(templineC3, tCT->keyWord, UTTLINELEN);
			strcat(buf, templineC3);
			pos = strlen(buf);
			uS.sprintf(buf+pos, " %d %d %d %d ", tCT->cWordFlag, tCT->red, tCT->green, tCT->blue);
		}
		strcat(buf, "\n");        // 2020-03-13 end
	}
	strcat(buf, WINDOWSINFO); // 2020-03-13 beg
	strcat(buf, "\t");
	sprintf(buf+strlen(buf), "%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld_%ld",
			top, left, height, width, topC, skipTop, pos1C, skipP1, pos2C, skipP2);
	strcat(buf, "\n");        // 2020-03-13 end
//	headerSt = [[NSString alloc] initWithUTF8String:buf];
//	[headerSt release]; // 2020-05-10
	headerSt = [NSString stringWithUTF8String:buf];
	textSt = [headerSt stringByAppendingString:textSt];
// 2020-05-20 end
	data = [textSt dataUsingEncoding:NSUTF8StringEncoding];
	if (data) {
		writeSuccess = [data writeToURL:absoluteURL atomically:true];
	} else {
		writeSuccess = NO;
	}

//	if (writeSuccess == YES && filePath[0] == EOS)
//		strcpy(filePath, [absoluteURL fileSystemRepresentation]);
	textSt = [text string];
	len = [text length];
	[text beginEditing];
	pos = 0;
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		isIncrementPos = true;
		if (ch == ATTMARKER) { // 2020-07-27 beg
			ch = [textSt characterAtIndex:pos+1];
			if (ch == error_start) {
				tPos = pos + 2;
				while (tPos < len) {
					ch = [textSt characterAtIndex:tPos];
					if (ch == '\n')
						break;
					if (ch == ATTMARKER) {
						ch = [textSt characterAtIndex:tPos+1];
						if (ch == error_end) {
							posRange = NSMakeRange(pos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:posRange withString:@""];
							tPos -= 2;
							posRange = NSMakeRange(tPos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:posRange withString:@""];
							if (pos1C > pos)
								pos1C -= 2;
							if (pos2C > pos)
								pos2C -= 2;
							if (pos1C > tPos)
								pos1C -= 2;
							if (pos2C > tPos)
								pos2C -= 2;
							textSt = [text string];
							len = [text length];
							ch = [textSt characterAtIndex:pos];
							isIncrementPos = false;
							break;
						}
					}
					tPos++;
				}
			}
		} // 2020-07-27 end
		if (isIncrementPos) {
			pos++;
		}
	}

	textSt = [text string];
	len = [text length];
	pos = 0;
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		isIncrementPos = true;
		if (ch == ATTMARKER) { // 2021-07-16 beg
			ch = [textSt characterAtIndex:pos+1];
			if (ch == underline_start) {
				tPos = pos + 2;
				while (tPos < len) {
					ch = [textSt characterAtIndex:tPos];
					if (ch == '\n')
						break;
					if (ch == ATTMARKER) {
						ch = [textSt characterAtIndex:tPos+1];
						if (ch == underline_end) {
							posRange = NSMakeRange(pos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:posRange withString:@""];
							tPos -= 2;
							posRange = NSMakeRange(tPos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:posRange withString:@""];
							if (pos1C > pos)
								pos1C -= 2;
							if (pos2C > pos)
								pos2C -= 2;
							if (pos1C > tPos)
								pos1C -= 2;
							if (pos2C > tPos)
								pos2C -= 2;
							textSt = [text string];
							len = [text length];
							ch = [textSt characterAtIndex:pos];
							isIncrementPos = false;
							break;
						}
					}
					tPos++;
				}
			}
		} // 2021-07-16 end
		if (isIncrementPos) {
			pos++;
		}
	}
	[text endEditing];
	return writeSuccess;
}
// 2019-09-10 2020-05-10 end

/*
void CleanMediaName(unCH *tMediaFileName) {// 2020-03-12
	char qf;
	unCH *s;
	int i;

	for (i=0; isSpace(tMediaFileName[i]); i++) ;
	if (i > 0)
		strcpy(tMediaFileName, tMediaFileName+i);
	qf = FALSE;
	for (i=0; tMediaFileName[i] != EOS; i++) {
		if (tMediaFileName[i] == '"')
			qf = !qf;
		if (tMediaFileName[i] == ',' || (!qf && isSpace(tMediaFileName[i]))) {
			tMediaFileName[i] = EOS;
			if (isPlayAudioMenuSet == 0)
				i++;
			break;
		}
	}
	for (; tMediaFileName[i] != EOS; i++) {
		if (uS.mStrnicmp(tMediaFileName+i, "audio", 5) == 0) {
			isPlayAudioFirst = TRUE;
#ifdef _MAC_CODE
			SetTextWinMenus(FALSE);
#endif
			break;
		} else if (strncmp(tMediaFileName+i, "video", 5) == 0) {
			isPlayAudioFirst = FALSE;
#ifdef _MAC_CODE
			SetTextWinMenus(FALSE);
#endif
			break;
		}
	}
	uS.remFrontAndBackBlanks(tMediaFileName);
	s = strrchr(tMediaFileName, '.');
	if (s != NULL)
		*s = EOS;
	if (global_df->VideosNameIndex >= 0 && global_df->VideosNameIndex < 10) {
		if (global_df->VideosNameExts[global_df->VideosNameIndex][0] != EOS) {
			strcat(tMediaFileName, "-");
			strcat(tMediaFileName, global_df->VideosNameExts[global_df->VideosNameIndex]);
		}
	}
}
*/
- (void)getMediaName:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2020-03-12
	NSUInteger i;
	unichar ch;
	NSString *subSt;

	mediaFileName[0] = EOS;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != ' ' && ch != '\t')
			break;
	}
	if (pos >= len)
		return;
	i = pos;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == ',' || ch == '\n')
			break;
	}
	if (pos == i)
		return;
	subSt = [textSt substringWithRange:NSMakeRange(i, pos-i)];
	if ([subSt length] < FILENAME_MAX) {
		strcpy(mediaFileName, [subSt UTF8String]);
//		CleanMediaName(mediaFileName);
	}
}

- (NSUInteger)getPIDInfo:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2020-05-08
	int i;
	char buf[BUFSIZ+1];
	unichar ch;

	pidSt[0] = EOS;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != '\t' && ch != ' ')
			break;
	}
	i = 0;
	for (; pos < len && pos < BUFSIZ; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
		buf[i++] = ch;
	}
	buf[i] = EOS;
	strncpy(pidSt, buf, 256-1);
	pidSt[256-1] = EOS;
	return(pos);
}

- (NSUInteger)getColorInfo:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2020-03-13
	int i, wordLen;
	int cWordFlag, red, green, blue;
	unCH *tKeyWord, *tfWord;
	unCH buf[BUFSIZ];
	unichar ch;
	cCOLORTEXTLIST *t;

	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != '\t' && ch != ' ')
			break;
	}
	i = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
		buf[i++] = ch;
	}
	buf[i] = EOS;
	i = 0;
	while (buf[i] != EOS) {
		for (wordLen=0; !isSpace(buf[i]) && buf[i] != EOS; i++, wordLen++)
			templine3[wordLen] = buf[i];
		templine3[wordLen] = EOS;
		for (; isSpace(buf[i]); i++) ;
		if (!isdigit(buf[i]))
			return(pos);
		cWordFlag = uS.atoi(buf+i);
		for (; isdigit(buf[i]); i++) ;
		for (; isSpace(buf[i]); i++) ;
		if (!isdigit(buf[i]))
			return(pos);
		red = uS.atoi(buf+i);
		for (; isdigit(buf[i]); i++) ;
		for (; isSpace(buf[i]); i++) ;
		if (!isdigit(buf[i]))
			return(pos);
		green = uS.atoi(buf+i);
		for (; isdigit(buf[i]); i++) ;
		for (; isSpace(buf[i]); i++) ;
		if (!isdigit(buf[i]))
			return(pos);
		blue = uS.atoi(buf+i);
		for (; isdigit(buf[i]); i++) ;
		for (; isSpace(buf[i]); i++) ;

		tKeyWord = (unCH *)malloc((strlen(templine3)+1)*sizeof(unCH));
		if (tKeyWord == NULL)
			return(pos);
		strcpy(tKeyWord, templine3);
		tfWord = (unCH *)malloc((strlen(templine3)+1)*sizeof(unCH));
		if (tfWord == NULL) {
			free(tKeyWord);
			return(pos);
		}
		strcpy(tfWord, templine3);
		if (!is_case_word(cWordFlag)) {
			uS.uppercasestr(tfWord, &dFnt, C_MBF);
		}
		if (RootColorText == NULL) {
			RootColorText = NEW(cCOLORTEXTLIST);
			if (RootColorText == NULL) {
				free(tKeyWord);
				free(tfWord);
				return(pos);
			}
			t = RootColorText;
		} else {
			t->nextCT = NEW(cCOLORTEXTLIST);
			if (t->nextCT == NULL) {
				free(tKeyWord);
				free(tfWord);
				return(pos);
			}
			t = t->nextCT;
		}
		t->nextCT = NULL;
		t->keyWord = tKeyWord;
		t->fWord = tfWord;
		t->len = strlen(tfWord);
		t->cWordFlag = cWordFlag;
		t->red = red;
		t->green = green;
		t->blue = blue;
	}
	return(pos);
}

- (NSUInteger)getWindowInfo:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2020-03-13
	int  i;
	long lTop, lLeft, lHeight, lWidth, lTopC, lSkipTop, lPos1C, lSkipP1, lPos2C, lSkipP2;
	unCH buf[BUFSIZ];
	unichar ch;

	top = 0L;
	left = 0L;
	height = 0L;
	width = 0L;
	topC = 0L;
	skipTop = 0L;
	pos1C = 0L;
	skipP1 = 0L;
	pos2C = 0L;
	skipP2 = 0L;

	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != '\t' && ch != ' ')
			break;
	}
	i = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (!isdigit(ch) && ch != '-' && ch != '_')
			break;
		buf[i++] = ch;
	}
	buf[i] = EOS;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
	}

	lTop = uS.atol(buf); // top

	for (i=0; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lLeft = uS.atol(buf+i); // left

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lHeight = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lWidth = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	if (buf[i] == '-' && buf[i+1] == '1' && buf[i+2] == '_')
		lTopC = -1;
	else
		lTopC = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	if (buf[i] == '-' && buf[i+1] == '1' && buf[i+2] == '_')
		lSkipTop = -1;
	else
		lSkipTop = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lPos1C = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lSkipP1 = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lPos2C = uS.atol(buf+i);

	for (; buf[i] != EOS && buf[i] != '_'; i++) ;
	if (buf[i] == EOS)
		return(0);
	i++;
	lSkipP2 = uS.atol(buf+i);

	if (lHeight <= 0L || lWidth <= 0L) {
		lTop = 0L;
		lLeft = 0L;
		lHeight = 0L;
		lWidth = 0L;
	}

	if (lTop < 40L)
		lTop = 40L;
	if (lLeft < 0L)
		lLeft = 0L;
/*
	if (lTopC != -1 && lSkipTop != -1) {
		lTop = 0L;
		lLeft = 0L;
		lHeight = 0L;
		lWidth = 0L;
		lTopC = 0L;
		lSkipTop = 0L;
		lPos1C = 0L;
		lSkipP1 = 0L;
		lPos2C = 0L;
		lSkipP2 = 0L;
	}
*/
	if (lTopC != -1 && lSkipTop != -1) {
		lTopC = 0L;
		lSkipTop = 0L;
		lPos1C = 0L;
		lSkipP1 = 0L;
		lPos2C = 0L;
		lSkipP2 = 0L;
	} else if (lPos1C > lPos2C || lSkipP1 != 0 || lSkipP2 != 0 || lPos1C > len || lPos2C > len) {
		lTopC = 0L;
		lSkipTop = 0L;
		lPos1C = 0L;
		lSkipP1 = 0L;
		lPos2C = 0L;
		lSkipP2 = 0L;
	}

	top = lTop;
	left = lLeft;
	height = lHeight;
	width = lWidth;
	topC = lTopC;
	skipTop = lSkipTop;
	pos1C = lPos1C;
	skipP1 = lSkipP1;
	pos2C = lPos2C;
	skipP2 = lSkipP2;
	return(pos);
}

- (BOOL)getOptionsInfo:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2020-03-13
	unichar ch;

	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != '\t' && ch != ' ')
			break;
	}
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 'C' && pos+1 < len) {
			ch = [textSt characterAtIndex:pos+1];
			if (ch == 'A') {
				if (pos+2 >= len)
					return(true);
				else {
					ch = [textSt characterAtIndex:pos+2];
					if (ch != '_') {
						return(true);
					}
				}
			}
		}
		if (ch == '\n')
			break;
	}
	return(false);
}

- (NSUInteger)getFontInfo:(NSString *)textSt AtIndex:(NSUInteger)pos MaxLen:(NSUInteger)len { // 2021-03-15
	int i;
	char buf[BUFSIZ+1];
	unichar ch;

	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch != '\t' && ch != ' ')
			break;
	}
	i = 0;
	for (; pos < len && pos < BUFSIZ; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
		buf[i++] = ch;
	}
	buf[i] = EOS;
	if (uS.mStrnicmp(buf, "CAfont:", 7) == 0)
		return(pos);
	return(0);
}

/* This method is called by the document controller. The message is passed on after information about the selected encodding
 (from our controller subclass) and preference regarding HTML and RTF formatting has been added.
  encodding is the default encodding if the document was opened without an open panel.
*/
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
	NSLog(@"Document: readFromURL\n");
	unichar ch, lastCh, suSt[256];
	NSMutableDictionary *options = [NSMutableDictionary dictionaryWithCapacity:5];
	NSDictionary *docAttrs;
	id val;
//	id paperSizeVal, viewSizeVal;
	NSUInteger suSti, len, pos, tPos, endPos;
	NSString *bulletStr, *textSt;
	NSRange  endRange;
	NSTextStorage *text = [self textStorage];
	BOOL success; //, isIncrementPos;

	[fileTypeToSet release];
	fileTypeToSet = nil;

	[[self undoManager] disableUndoRegistration];

	[options setObject:absoluteURL forKey:NSBaseURLDocumentOption];
	[options setObject:NSPlainTextDocumentType forKey:NSDocumentTypeDocumentOption]; // Force plain
	typeName = (NSString *)kUTTypeCHAT /*kUTTypeText*/;

	[[text mutableString] setString:@""];
	// Remove the layout managers while loading the text; mutableCopy retains the array so the layout managers aren't released
	NSMutableArray *layoutMgrs = [[text layoutManagers] mutableCopy];
	NSEnumerator *layoutMgrEnum = [layoutMgrs objectEnumerator];
	NSLayoutManager *tLayoutMgr = nil;
	while ((tLayoutMgr = [layoutMgrEnum nextObject]))
		[text removeLayoutManager:tLayoutMgr];

	[text beginEditing];
	success = [text readFromURL:absoluteURL options:options documentAttributes:&docAttrs error:outError];

	if (!success) {
		[text endEditing];
		layoutMgrEnum = [layoutMgrs objectEnumerator]; // rewind
		while ((tLayoutMgr = [layoutMgrEnum nextObject]))
			[text addLayoutManager:tLayoutMgr];   // Add the layout managers back
		[layoutMgrs release];
		return NO;	// return NO on error; outError has already been set
	}
	[text endEditing];


//const char *st;
//st = [suSt UTF8String];
	[self set_wID:DOCWIN]; // 2019-09-06
// 2020-03-13 beg
	[text beginEditing];
	textSt = [text string];
	len = [text length];
	pos = 0;
		
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == '\r') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
				if (ch == '\n') {
					[text replaceCharactersInRange:NSMakeRange(pos, 1) withString:@""];
					len = [text length];
				} else {
					[text replaceCharactersInRange:NSMakeRange(pos, 1) withString:@"\n"];
					len = [text length];
				}
			} else {
				[text replaceCharactersInRange:NSMakeRange(pos, 1) withString:@"\n"];
				len = [text length];
			}
		} else
			pos++;
	}
	[text endEditing];
	isCAFont = false;
	[text beginEditing];
	textSt = [text string];
	len = [text length];
	pos = 0;
	suSti = 0;
	lastCh = '\n';
	RootColorText = nil;
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == '@' && lastCh == '\n') {
			suSti = 0;
			tPos = pos;
			while (tPos < len) { // get speaker code in variable "suSt"
				ch = [textSt characterAtIndex:tPos];
				suSt[suSti++] = ch;
				tPos++;
				if (ch == ':') {
					suSt[suSti] = EOS;
					break;
				}
				if (ch == '\n') {
					suSt[suSti-1] = EOS;
					break;
				}
				if (suSti >= 256-1) {
					suSti = 0;
					suSt[0] = EOS;
					break;
				}
			}
			if (suSti > 0) {
				if (uS.mStricmp(suSt, MEDIAHEADER) == 0) {
					[self getMediaName:textSt AtIndex:pos+MEDIAHEADERLEN MaxLen:len]; // 2020-03-12
					lastCh = ch;
					pos++;
				} else if (uS.mStricmp(suSt, UTF8HEADER) == 0) {
					endRange = NSMakeRange(pos, UTF8HEADERLEN+1); // (NSUInteger pos, NSUInteger len)
					[text replaceCharactersInRange:endRange withString:@""];
					textSt = [text string];
					len = [text length];
				} else if (pos+PIDHEADERLEN <= len && uS.mStricmp(suSt, PIDHEADER) == 0) {
					endPos = [self getPIDInfo:textSt AtIndex:pos+PIDHEADERLEN MaxLen:len]; // 2020-05-08
					endRange = NSMakeRange(pos, endPos-pos); // (NSUInteger pos, NSUInteger len)
					[text replaceCharactersInRange:endRange withString:@""];
					textSt = [text string];
					len = [text length];
				} else if (pos+CKEYWORDHEADERLEN <= len && uS.mStricmp(suSt, CKEYWORDHEADER) == 0) {
					endPos = [self getColorInfo:textSt AtIndex:pos+CKEYWORDHEADERLEN MaxLen:len]; // 2020-05-08
					endRange = NSMakeRange(pos, endPos-pos); // (NSUInteger pos, NSUInteger len)
					[text replaceCharactersInRange:endRange withString:@""];
					textSt = [text string];
					len = [text length];
				} else if (pos+WINDOWSINFOLEN <= len && uS.mStricmp(suSt, WINDOWSINFO) == 0) {
					endPos = [self getWindowInfo:textSt AtIndex:pos+WINDOWSINFOLEN MaxLen:len]; // 2020-03-13
					endRange = NSMakeRange(pos, endPos-pos); // (NSUInteger pos, NSUInteger len)
					[text replaceCharactersInRange:endRange withString:@""];
					textSt = [text string];
					len = [text length];
				} else if (pos+FONTHEADERLEN <= len && uS.mStricmp(suSt, FONTHEADER) == 0) {
					endPos = [self getFontInfo:textSt AtIndex:pos+FONTHEADERLEN MaxLen:len]; // 2021-03-15
					if (endPos > 0) {
						endRange = NSMakeRange(pos, endPos-pos); // (NSUInteger pos, NSUInteger len)
						[text replaceCharactersInRange:endRange withString:@""];
						textSt = [text string];
						len = [text length];
						isCAFont = true;
					} else {
						lastCh = ch;
						pos++;
					}
				} else if (pos+9 <= len && uS.mStricmp(suSt, "@Options:") == 0) {
					isCAFont = [self getOptionsInfo:textSt AtIndex:pos+9 MaxLen:len]; // 2020-03-13
					lastCh = ch;
					pos++;
				} else {
					lastCh = ch;
					pos++;
				}
			} else {
				lastCh = ch;
				pos++;
			}
		} else {
			lastCh = ch;
			pos++;
		}
	}
	[text endEditing];
	if ([self docFont] == nil) {
		if (isCAFont)
			[self setDocFont:defCAFont];
		else
			[self setDocFont:defUniFont];
	}
	[self applyDefaultTextAttributes:NO];
// 2020-03-13 end
// 2019-09-10 beg
//	bulletStr = [[NSString alloc]initWithString:@"•"]; // 0x2022 - E2 80 A2
//	bulletStr = [NSString stringWithUTF8String:"\xe2\x80\xa2"]; // • - 0x2022 - E2 80 A2
	bulletStr = [NSString stringWithFormat:@"%C", 0x2022]; // • - 0x2022 - E2 80 A2
	textSt = [text string];
	len = [text length];
	[text beginEditing];
	pos = 0;
	while (pos < len) {	// Run through the string in terms of attachment runs
		ch = [textSt characterAtIndex:pos];
		lastCh = ch;
		if (ch == HIDEN_C) {
			endRange = NSMakeRange(pos, 1); // (NSUInteger pos, NSUInteger len)
			[text replaceCharactersInRange:endRange withString:bulletStr];
//			len = [text length]; // New length
		}
		pos++;
	}
	[text endEditing];
//	[bulletStr release]; // dealloc
// 2019-09-10 end

	strcpy(filePath, [absoluteURL fileSystemRepresentation]);
	[self setFileType:typeName];
	// If we're reverting, NSDocument will set the file type behind out backs. This enables restoring that type.
	fileTypeToSet = [typeName copy];

	endRange = NSMakeRange(0, [text length]); // (NSUInteger pos, NSUInteger len)
	layoutMgrEnum = [layoutMgrs objectEnumerator]; // rewind
	while ((tLayoutMgr = [layoutMgrEnum nextObject])) {
		[text addLayoutManager:tLayoutMgr];   // Add the layout managers back
		[tLayoutMgr invalidateDisplayForCharacterRange:endRange];
	}
	[layoutMgrs release];

	[self willChangeValueForKey:@"printInfo"];
	if ((val = [docAttrs objectForKey:NSLeftMarginDocumentAttribute]))
		[[self printInfo] setLeftMargin:[val doubleValue]];
	if ((val = [docAttrs objectForKey:NSRightMarginDocumentAttribute]))
		[[self printInfo] setRightMargin:[val doubleValue]];
	if ((val = [docAttrs objectForKey:NSBottomMarginDocumentAttribute]))
		[[self printInfo] setBottomMargin:[val doubleValue]];
	if ((val = [docAttrs objectForKey:NSTopMarginDocumentAttribute]))
		[[self printInfo] setTopMargin:[val doubleValue]];
	[self didChangeValueForKey:@"printInfo"];


	/* Pre MacOSX versions of TextEdit wrote out the view (window) size in PaperSize.
	 If we encounter a non-MacOSX RTF file, and it's written by TextEdit, use PaperSize as ViewSize */
/*
	viewSizeVal = [docAttrs objectForKey:NSViewSizeDocumentAttribute];
	paperSizeVal = [docAttrs objectForKey:NSPaperSizeDocumentAttribute];
	if (paperSizeVal && NSEqualSizes([paperSizeVal sizeValue], NSZeroSize))
		paperSizeVal = nil;	// Protect against some old documents with 0 paper size

	if (viewSizeVal) {
		[self setViewSize:[viewSizeVal sizeValue]];
		if (paperSizeVal)
			[self setPaperSize:[paperSizeVal sizeValue]];
	} else {	// No ViewSize...
		if (paperSizeVal) {	// See if PaperSize should be used as ViewSize; if so, we also have some tweaking to do on it
			val = [docAttrs objectForKey:NSCocoaVersionDocumentAttribute];
			if (val && ([val integerValue] < 100)) {	// Indicates old RTF file; value described in AppKit/NSAttributedString.h
				NSSize size = [paperSizeVal sizeValue];
				if (size.width > 0 && size.height > 0) {
					size.width = size.width - oldEditPaddingCompensation;
					[self setViewSize:size];
				}
			} else {
				[self setPaperSize:[paperSizeVal sizeValue]];
			}
		}
	}
*/
	[self setBackgroundColor:[NSColor whiteColor]];
//	[self setBackgroundColor:(val = [docAttrs objectForKey:NSBackgroundColorDocumentAttribute]) ? val : [NSColor whiteColor]];

	// Set the document properties, generically, going through key value coding
	[self setReadOnly:((val = [docAttrs objectForKey:NSReadOnlyDocumentAttribute]) && ([val integerValue] > 0))];

	[self setOriginalOrientationSections:[docAttrs objectForKey:NSTextLayoutSectionsAttribute]];

	[self setDocUsesScreenFonts:NO]; // YES];

	[[self undoManager] enableUndoRegistration];

    return YES;
}

- (NSDictionary *)defaultTextAttributes:(BOOL)forRichText {
	static NSParagraphStyle *defaultRichParaStyle = nil;
	NSLog(@"Document: defaultTextAttributes\n");
	NSMutableDictionary *textAttributes = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
	if (forRichText) {
		[textAttributes setObject:[NSFont userFontOfSize:0.0] forKey:NSFontAttributeName];
		if (defaultRichParaStyle == nil) {	// We do this once...
			NSInteger cnt;
			NSString *measurementUnits = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleMeasurementUnits"];
			CGFloat tabInterval = ([@"Centimeters" isEqual:measurementUnits]) ? (72.0 / 2.54) : (72.0 / 2.0);  // Every cm or half inch
			NSMutableParagraphStyle *paraStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
			NSTextTabType type = ((NSWritingDirectionRightToLeft == [NSParagraphStyle defaultWritingDirectionForLanguage:nil]) ? NSRightTabStopType : NSLeftTabStopType);
			[paraStyle setTabStops:[NSArray array]];	// This first clears all tab stops
			for (cnt = 0; cnt < 12; cnt++) {	// Add 12 tab stops, at desired intervals...
				NSTextTab *tabStop = [[NSTextTab alloc] initWithType:type location:tabInterval * (cnt + 1)];
				[paraStyle addTabStop:tabStop];
				[tabStop release];
			}
			defaultRichParaStyle = [paraStyle copy];
		}
		[textAttributes setObject:defaultRichParaStyle forKey:NSParagraphStyleAttributeName];
	} else {
//		ArialUnicode = [NSFont userFixedPitchFontOfSize:0.0];
		NSFont *charWidthFont = [[self docFont] screenFontWithRenderingMode:NSFontDefaultRenderingMode];
		NSInteger tabWidth = [[NSUserDefaults standardUserDefaults] integerForKey:TabWidth];
		CGFloat charWidth = [@" " sizeWithAttributes:[NSDictionary dictionaryWithObject:charWidthFont forKey:NSFontAttributeName]].width;
		if (charWidth == 0)
			charWidth = [charWidthFont maximumAdvancement].width;

		// Now use a default paragraph style, but with the tab width adjusted
		NSMutableParagraphStyle *mStyle = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
		mStyle.headIndent = 29; // 2021-09-05
		[mStyle setTabStops:[NSArray array]];
		[mStyle setDefaultTabInterval:(charWidth * tabWidth)];
		[textAttributes setObject:[[mStyle copy] autorelease] forKey:NSParagraphStyleAttributeName];

		// Also set the font
		[textAttributes setObject:[self docFont] forKey:NSFontAttributeName];
	}
	return textAttributes;
}

- (void)applyDefaultTextAttributes:(BOOL)forRichText {
	NSLog(@"Document: applyDefaultTextAttributes\n");
	NSDictionary *textAttributes = [self defaultTextAttributes:forRichText];
	NSTextStorage *text = [self textStorage];
	// We now preserve base writing direction even for plain text, using the 10.6-introduced attribute enumeration API
	[text enumerateAttribute:NSParagraphStyleAttributeName inRange:NSMakeRange(0, [text length]) options:0 usingBlock:^(id paragraphStyle, NSRange paragraphStyleRange, BOOL *stop){
		NSWritingDirection writingDirection = paragraphStyle ? [(NSParagraphStyle *)paragraphStyle baseWritingDirection] : NSWritingDirectionNatural;
		// We also preserve NSWritingDirectionAttributeName (new in 10.6)
		[text enumerateAttribute:NSWritingDirectionAttributeName inRange:paragraphStyleRange options:0 usingBlock:^(id value, NSRange attributeRange, BOOL *stop){
			[value retain];
			[text setAttributes:textAttributes range:attributeRange];
			if (value)
				[text addAttribute:NSWritingDirectionAttributeName value:value range:attributeRange];
			[value release];
		}];
		if (writingDirection != NSWritingDirectionNatural) [text setBaseWritingDirection:writingDirection range:paragraphStyleRange];
	}];
}


/* This method will return a suggested encodding for the document. We use UTF-8.
*/
- (NSStringEncoding)suggestedDocumentEncoding {
	return NSUTF8StringEncoding;
}

/* Clear the delegates of the text views and window, then release all resources and go away...
*/
- (void)dealloc {
	if (wID == CLANWIN) {
		isMORXiMode = FALSE; // 2020-08-14
		CleanUpAll(TRUE);
	}
	StdInWindow = NULL;
	StdInErrMessage = NULL;
	StdDoneMessage  = NULL;
	[textStorage release];
    [backgroundColor release];

    [fileTypeToSet release];

    [originalOrientationSections release];
    [super dealloc];
}

- (NSSize)viewSize {
	return viewSize;
}

- (void)setViewSize:(NSSize)size {
	viewSize = size;
}

- (NSFont *)docFont {
	return docFont;
}

- (void)setDocFont:(NSFont *)newDocFont {
	docFont = newDocFont;
}

- (CGFloat)fontHeight {
	return fontHeight;
}

- (void)setFontHeight:(CGFloat)newFontHeight {
	fontHeight = newFontHeight;
}

- (void)setReadOnly:(BOOL)flag {
	isReadOnly = flag;
}

- (BOOL)isReadOnly {
    return isReadOnly;
}

- (void)set_wID:(unsigned short)num { // 2019-09-06
	wID = num;
}

- (unsigned short)get_wID { // 2019-09-06
	return wID;
}

- (void)setBackgroundColor:(NSColor *)color {
	id oldCol = backgroundColor;
	backgroundColor = [color copy];
	[oldCol release];
}

- (NSColor *)backgroundColor {
    return backgroundColor;
}

- (NSTextStorage *)textStorage {
    return textStorage;
}

- (NSSize)paperSize {
    return [[self printInfo] paperSize];
}

- (void)setPaperSize:(NSSize)size {
	NSPrintInfo *oldPrintInfo = [self printInfo];
	if (!NSEqualSizes(size, [oldPrintInfo paperSize])) {
		NSPrintInfo *newPrintInfo = [oldPrintInfo copy];
		[newPrintInfo setPaperSize:size];
		[self setPrintInfo:newPrintInfo];
		[newPrintInfo release];
	}
}

/* Layout orientation sections */
- (void)setOriginalOrientationSections:(NSArray *)array {
    [originalOrientationSections release];
    originalOrientationSections = [array copy];
}

- (NSArray *)originalOrientationSections {
    return originalOrientationSections; 
}

/* Screen fonts property */
- (void)setDocUsesScreenFonts:(BOOL)aFlag {
    usesDocScreenFonts = aFlag;
}

- (BOOL)usesDocScreenFonts {
    return usesDocScreenFonts;
}

/* This is the encodding used for saving; valid only during a save operation
*/
- (NSUInteger)encodingForSaving {
    return documentEncodingForSaving;
}

- (void)setEncodingForSaving:(NSUInteger)SaveEncoding {
    documentEncodingForSaving = SaveEncoding;
}

- (void)setFileType:(NSString *)type {
    /* Due to sandboxing, we cannot (usefully) directly change the document's file URL except for changing it to nil.
	 This means that when we convert a document from rich text to plain text, our only way of updating the file URL
	 is to have NSDocument do it for us in response to a change in our file type.  However, it is not as simple as
	 setting our type to kUTTypeText, as would be accurate, because if we have a rtf document, public.rtf inherits
	 from public.text and so NSDocument wouldn't change our extension (which is correct, BTW, since it's also perfectly
	 valid to open a rtf and not interpret the rtf commands, in which case a save of a rtf document as kUTTypeText
	 should not change the extension).  Therefore we need to save using a subtype of kUTTypeText that isn't in the
	 path from kUTTypeText to kUTTypeRTF.  The obvious candidate is kUTTypePlainText.  Therefore, we need to save
	 using kUTTypePlainText when we convert a rtf to plain text and then map the file type from kTTypePlainText to
	 kUTTypeText.  The inverse of the mapping occurs here. */
    if ([type isEqualToString:(NSString *)kUTTypePlainText])
		type = (NSString *)kUTTypeCHAT /*kUTTypeText*/;
    [super setFileType:type];
}

- (void)setValue:(id)value forKey:(NSString *)key {
 	[super setValue:value forKey:key];  // In case some other KVC call is sent to Document, we treat it normally
}

- (NSPrintOperation *)printOperationWithSettings:(NSDictionary *)printSettings error:(NSError **)outError {
	NSPrintInfo *tempPrintInfo = [[[self printInfo] copy] autorelease];
	[[tempPrintInfo dictionary] addEntriesFromDictionary:printSettings];

	if ([[self windowControllers] count] == 0) [self makeWindowControllers];
	NSView *documentView = [[[self windowControllers] objectAtIndex:0] documentView];

//	id printingView;
//	printingView = [[[PrintingTextView alloc] init] autorelease];   // PrintingTextView is a simple subclass of NSTextView. Creating the view this way creates rest of the text system, which it will release when dealloc'ed (since the print panel will be releasing this, we want to hand off the responsibility of release everything)
//	NSLayoutManager *layoutManager = [[[[printingView textContainer] layoutManager] retain] autorelease];
//	NSTextStorage *unnecessaryTextStorage = [layoutManager textStorage];  // We don't want the text storage, since we will use the one we have
//	[unnecessaryTextStorage removeLayoutManager:layoutManager];
//	[unnecessaryTextStorage release];
//	[textStorage addLayoutManager:layoutManager];
//	[textStorage retain];   // Since later release of the printingView will release the textStorage as well
//	[printingView setLayoutOrientation:[[[[self windowControllers] objectAtIndex:0] firstTextView] layoutOrientation]]; // 2021-09-28

	PrintPanelAccessoryController *accessoryController = [[[PrintPanelAccessoryController alloc] init] autorelease]; // 2021-09-28
//	NSPrintOperation *op = [NSPrintOperation printOperationWithView:printingView printInfo:tempPrintInfo];
	NSPrintOperation *op = [NSPrintOperation printOperationWithView:[[[self windowControllers] objectAtIndex:0] documentView] printInfo:tempPrintInfo];
	[op setShowsPrintPanel:YES];
	[op setShowsProgressPanel:YES];
	// Since this printing view may not be embedded in a window, we have to manually set the print job title.
	[op setJobTitle:[documentView printJobTitle]];

	// If we're in wrap-to-page mode, no need to let the user tweak wrap-to-page mode printing
	[accessoryController setShowsWrappingToFit:YES]; // 2021-09-28

	NSPrintPanel *printPanel = [op printPanel];
//	[printingView setOriginalSize:[[[[self windowControllers] objectAtIndex:0] firstTextView] frame].size]; // 2019-08-27 2021-09-28
//	[printingView setPrintPanelAccessoryController:accessoryController]; // 2021-09-28
	// We allow changing print parameters if not in "Wrap to Page" mode, where the page setup settings are used
	[printPanel setOptions:[printPanel options] | NSPrintPanelShowsPaperSize | NSPrintPanelShowsOrientation];
	[printPanel addAccessoryController:accessoryController]; //2021-09-28

    return op;
}

- (NSPrintInfo *)printInfo {
	NSPrintInfo *printInfo = [super printInfo];
	if (!setUpPrintInfoDefaults) {
		setUpPrintInfoDefaults = YES;
		[printInfo setHorizontalPagination:NSFitPagination];
		[printInfo setHorizontallyCentered:NO];
		[printInfo setVerticallyCentered:NO];
		[printInfo setLeftMargin:72.0];
		[printInfo setRightMargin:72.0];
		[printInfo setTopMargin:72.0];
		[printInfo setBottomMargin:72.0];
	}
    return printInfo;
}

/* Open Special Characters document
 */
// 2019-10-19 beg
#define STSIZE 2048
#define isHex(x) (x == 'A' || x == 'B' || x == 'C' || x == 'D' || x == 'E' || x == 'F')
static void makeUnicodeString(unichar *st, const char *src) {
	long srci, sti, hexi;
	unsigned long uc;
	char hexStr[256];

	for (srci=0L; src[srci] != EOS; srci++) ;
	for (sti=0L; st[sti] != EOS; sti++) ;
	if (sti+srci >= STSIZE) {
		return;
	}
	srci = 0L;
	while (src[srci] != EOS) {
		if (src[srci] == '0' && src[srci+1] == 'x') {
			hexi = 0L;
			hexStr[hexi++] = src[srci++];
			hexStr[hexi++] = src[srci++];
			for (; isdigit(src[srci]) || isHex(src[srci]); hexi++)
				hexStr[hexi] = src[srci++];
			hexStr[hexi] = EOS;
			sscanf(hexStr, "%lx", &uc);
			st[sti++] = uc;
		} else
			st[sti++] = src[srci++];
	}
	st[sti++] = '\n';
	st[sti] = EOS;
}
// 2019-10-19 end

// 2019-10-19 beg
- (IBAction)openSpecialCharacters:(id)sender {
#pragma unused (sender)
	int i;
	unichar st[STSIZE];
	Document *cDoc;
	NSUInteger docsCnt, textLen;
	NSArray *documents;
	NSTextStorage *text;
	NSString *mySt;
	NSRange endRange, myStRange;
	NSMutableDictionary *textAttributes;

	documents = [[NSDocumentController sharedDocumentController] documents];
	docsCnt = [documents count];
	for (i=0; i < docsCnt; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == SpChrWIN) {
			[cDoc makeWindowControllers];
			[cDoc showWindows];
			return;
		}
	}
	cDoc = [[NSDocumentController sharedDocumentController] openUntitledDocumentAndDisplay:NO error:nil];
	if (cDoc != NULL) {
		cDoc->isCAFont = true;
		[cDoc setDocFont:defCAFont];
		[cDoc set_wID:SpChrWIN]; // 2019-10-22
		[cDoc makeWindowControllers];
		[cDoc showWindows];

		st[0] = EOS;
		/* 1*/	makeUnicodeString(st, "0x2191 shift to high pitch; F1 up-arrow"); // CA_BREATHY_VOICE
		/* 2*/	makeUnicodeString(st, "0x2193 shift to low pitch; F1 down-arrow");
		/* 3*/	makeUnicodeString(st, "0x21D7 rising to high; F1 1");
		/* 4*/	makeUnicodeString(st, "0x2197 rising to mid; F1 2");
		/* 5*/	makeUnicodeString(st, "0x2192 level; F1 3");
		/* 6*/	makeUnicodeString(st, "0x2198 falling to mid; F1 4");
		/* 7*/	makeUnicodeString(st, "0x21D8 falling to low; F1 5");
		/* 8*/	makeUnicodeString(st, "0x221E unmarked ending; F1 6");
		/* 9*/	makeUnicodeString(st, "0x224B 0x224Bcontinuation; F1 +");
		/*10*/	makeUnicodeString(st, "0x2219 inhalation; F1 .");
		/*11*/	makeUnicodeString(st, "0x2248 latching0x2248; F1 =");
		/*12*/	makeUnicodeString(st, "0x2261 0x2261uptake; F1 u");
		/*13*/	makeUnicodeString(st, "0x2308 top begin overlap; F1 [");
		/*14*/	makeUnicodeString(st, "0x2309 top end overlap; F1 ]");
		/*15*/	makeUnicodeString(st, "0x230A bottom begin overlap; F1 {");
		/*16*/	makeUnicodeString(st, "0x230B bottom end overlap; F1 }");
		/*17*/	makeUnicodeString(st, "0x2206 0x2206faster0x2206; F1 right-arrow");
		/*18*/	makeUnicodeString(st, "0x2207 0x2207slower0x2207; F1 left-arrow");
		/*19*/	makeUnicodeString(st, "0x204E 0x204Ecreaky0x204E; F1 *");
		/*20*/	makeUnicodeString(st, "0x2047 0x2047unsure0x2047; F1 /");
		/*21*/	makeUnicodeString(st, "0x00B0 0x00B0softer0x00B0; F1 0");
		/*22*/	makeUnicodeString(st, "0x25C9 0x25C9louder0x25C9; F1 )");
		/*23*/	makeUnicodeString(st, "0x2581 0x2581low pitch0x2581; F1 d");
		/*24*/	makeUnicodeString(st, "0x2594 0x2594high pitch0x2594; F1 h");
		/*25*/	makeUnicodeString(st, "0x263A 0x263Asmile voice0x263A; F1 l");
		/*26*/	makeUnicodeString(st, "0x264B 0x264Bbreathy voice0x264B marker; F1 b");
		/*27*/	makeUnicodeString(st, "0x222C 0x222Cwhisper0x222C; F1 w");
		/*28*/	makeUnicodeString(st, "0x03AB 0x03AByawn0x03AB; F1 y");
		/*29*/	makeUnicodeString(st, "0x222E 0x222Esinging0x222E; F1 s");
		/*30*/	makeUnicodeString(st, "0x00A7 0x00A7precise0x00A7; F1 p");
		/*31*/	makeUnicodeString(st, "0x223E constriction0x223E; F1 n");
		/*32*/	makeUnicodeString(st, "0x21BB 0x21BBpitch reset; F1 r");
		/*33*/	makeUnicodeString(st, "0x1F29 laugh in a word; F1 c");
		/*34*/	makeUnicodeString(st, "0x201E Tag or sentence final particle; F2 t");
		/*35*/	makeUnicodeString(st, "0x2021 0x2021 Vocative or summons; F2 v");
		/*36*/	makeUnicodeString(st, "0x0323 Arabic dot diacritic; F2 ,");
		/*37*/	makeUnicodeString(st, "0x02B0 Arabic raised h; F2 H");
		/*38*/	makeUnicodeString(st, "0x0304 Stress; F2 -");
		/*39*/	makeUnicodeString(st, "0x0294 Glottal stop0x0294; F2 q");
		/*40*/	makeUnicodeString(st, "0x0295 Reverse glottal0x0295; F2 Q");
		/*41*/	makeUnicodeString(st, "0x030C Caron; F2 ;");
		/*42*/	makeUnicodeString(st, "0x02C8 raised0x02C8 stroke; F2 1");
		/*43*/	makeUnicodeString(st, "0x02CC lowered0x02CC stroke; F2 2");
		/*44*/	makeUnicodeString(st, "0x02D0 length on the %pho line; F2 :");
		/*45*/	makeUnicodeString(st, "0x2039 0x2039begin phono group0x203A marker; F2 <");
		/*46*/	makeUnicodeString(st, "0x203A 0x2039end phono group0x203A marker; F2 >");
		/*47*/	makeUnicodeString(st, "0x3014 0x3014begin sign group0x3015; F2 {");
		/*48*/	makeUnicodeString(st, "0x3015 0x3014end sign group0x3015; F2 }");
		/*49*/	makeUnicodeString(st, "0x2026 %pho missing word; F2 m");
		/*50*/	makeUnicodeString(st, "0x0332 und0x0332e0x0332r0x0332line; F2 <underline>");
		/*51*/	makeUnicodeString(st, "0x201C open 0x201Cquote0x201D; F2 '");
		/*52*/	makeUnicodeString(st, "0x201D close 0x201Cquote0x201D; F2 \"");
		/*53*/	makeUnicodeString(st, "0x2260 0x2260row; F2 =");
		/*54*/	makeUnicodeString(st, "0x21AB 0x21ABr-r0x21ABrabbit; F2 /");

		mySt = [NSString stringWithCharacters:st length:strlen(st)]; // NSUnicodeStringEncoding

		textAttributes = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
		[textAttributes setObject:defCAFont forKey:NSFontAttributeName];

		text = [cDoc textStorage];
		[text beginEditing];
		textLen = [text length];
		endRange = NSMakeRange(textLen, 0);
		myStRange = NSMakeRange(textLen, [mySt length]);
		[text replaceCharactersInRange:endRange withString:mySt];
		[text setAttributes:textAttributes range:myStRange];
		[text endEditing];

		[cDoc setReadOnly:YES];
	}
}
// 2019-10-19 end

// 2020-10-13 beg
- (IBAction)openKeysList:(id)sender {
#pragma unused (sender)
	int i;
	unichar st[STSIZE];
	Document *cDoc;
	NSUInteger docsCnt, textLen;
	NSArray *documents;
	NSTextStorage *text;
	NSString *mySt;
	NSRange endRange, myStRange;
	NSMutableDictionary *textAttributes;

	documents = [[NSDocumentController sharedDocumentController] documents];
	docsCnt = [documents count];
	for (i=0; i < docsCnt; i++) {
		cDoc = [documents objectAtIndex:i];
		if (cDoc != NULL && [cDoc get_wID] == KeysLWIN) {
			[cDoc makeWindowControllers];
			[cDoc showWindows];
			return;
		}
	}
	cDoc = [[NSDocumentController sharedDocumentController] openUntitledDocumentAndDisplay:NO error:nil];
	if (cDoc != NULL) {
		[cDoc set_wID:KeysLWIN]; // 2019-10-22
		[cDoc makeWindowControllers];
		[cDoc showWindows];

		st[0] = EOS;
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "ESC_Prefix:  ESC");
		makeUnicodeString(st, "abort-command:  command-. 0x2318-.");
		makeUnicodeString(st, "editor-coding-mode-toggle:  ESC-E ESC-e");
		makeUnicodeString(st, "play-current-tier:  F4");
		makeUnicodeString(st, "play-from-now-on-walker-steps:  F6");
		makeUnicodeString(st, "play-previous-walker-step:  F7");
		makeUnicodeString(st, "repeat-current-walker-step:  F8");
		makeUnicodeString(st, "play-next-walker-step:  F9");
		makeUnicodeString(st, "play-bullets-from-now-on:  ESC-8");
		makeUnicodeString(st, "play-bullets-from-now-on-minus-silence:  ESC-9");
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "coding-finish-current-code-or-tier:  ESC-C ESC-c");
		makeUnicodeString(st, "coding-finish-current-tier-goto-next:  ^T");
		makeUnicodeString(st, "coding-insert-highlighted-code-at-cursor:  ^C");
		makeUnicodeString(st, "coding-show-subcodes-under-cursor:  ESC-S ESC-s");
		makeUnicodeString(st, "open-new-coding-file:  ESC-\" ESC-3");
		makeUnicodeString(st, "set-next-coding-tier-name:  ESC-T ESC-t");
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "disambiguate-mor-tier:     ESC-2");
		makeUnicodeString(st, "expand-bullets:            ESC-A ESC-a");
		makeUnicodeString(st, "transcribe-from-now-on:    F5");
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "cursor-beginning-of-file:  [Home]");
		makeUnicodeString(st, "cursor-end-of-file:        [End]");
		makeUnicodeString(st, "cursor-beginning-of-line:  CTRL-A CTRL-[Home]");
		makeUnicodeString(st, "cursor-end-of-line:        CTRL-E CTRL-[End]");
		makeUnicodeString(st, "cursor-left:               CTRL-B [Left Arrow]");
		makeUnicodeString(st, "cursor-right:              CTRL-F [Right Arrow]");
		makeUnicodeString(st, "cursor-next-page:          CTRL-[Down Arrow] [PgDn]");
		makeUnicodeString(st, "cursor-previous-page:      CTRL-[Up Arrow][PgUp]");
		makeUnicodeString(st, "cursor-down:               CTRL-N [Down Arrow]");
		makeUnicodeString(st, "cursor-up:                 CTRL-P [Up Arrow]");
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "delete-next-character:     CTRL-D");
		makeUnicodeString(st, "delete-previous-character: CTRL-H [Delelet]");
		makeUnicodeString(st, "    ");
		makeUnicodeString(st, "goto-line:                 command-l 0x2318-l");
//		makeUnicodeString(st, "display-percent-of-file:  ESC-P ESC-p");
//		makeUnicodeString(st, "edit_copy-selected-text:  ESC-6 ESC-F-89");
//		makeUnicodeString(st, "edit_find-text:  ESC-7 ESC-F-24");
//		makeUnicodeString(st, "edit_paste-buffer-text:");
//		makeUnicodeString(st, "enter-ascii-code:  ESC-G ESC-g");
//		makeUnicodeString(st, "find-next-tier-with-media:  F8");
//		makeUnicodeString(st, "find-previous-tier-with-media:  F6 F19");
//		makeUnicodeString(st, "find-text-associated-with-movie:");
//		makeUnicodeString(st, "find-text-associated-with-sound:");
//		makeUnicodeString(st, "macro-insert-string:  ^W");
//		makeUnicodeString(st, "macro-set-string:  ESC-N ESC-n");
//		makeUnicodeString(st, "move-code-cursor-down:  + =");
//		makeUnicodeString(st, "move-code-cursor-left:");
//		makeUnicodeString(st, "move-code-cursor-right:  ^X-^L");
//		makeUnicodeString(st, "move-code-cursor-up:  -");
//		makeUnicodeString(st, "move-media-beg-left:  CTRL-[Left Arrow]");
//		makeUnicodeString(st, "move-media-beg-right:  CTRL-[Right Arrow]");
//		makeUnicodeString(st, "move-media-end-left:  CMD-[Left Arrow]");
//		makeUnicodeString(st, "move-media-end-right:  CMD-[Right Arrow]");
//		makeUnicodeString(st, "movie-thumb-nails:  ESC-( ESC-5");
//		makeUnicodeString(st, "play-current-tier:  ESC-& ESC-1 F4");
//		makeUnicodeString(st, "play-current-walker-step:  F9");
//		makeUnicodeString(st, "play-current-walker-step:  F7");
//		makeUnicodeString(st, "play-from-now-on:  ESC-! ESC-8");
//		makeUnicodeString(st, "play-from-now-on-minus-silence:  ESC-9");
//		makeUnicodeString(st, "redisplay-screen:  ^X-^R");
//		makeUnicodeString(st, "sound-mode-transcribe:  ESC-0");
//		makeUnicodeString(st, "text-mode:  ESC-M ESC-m");

		mySt = [NSString stringWithCharacters:st length:strlen(st)]; // NSUnicodeStringEncoding

		textAttributes = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
		[textAttributes setObject:defCAFont forKey:NSFontAttributeName];

		text = [cDoc textStorage];
		[text beginEditing];
		textLen = [text length];
		endRange = NSMakeRange(textLen, 0);
		myStRange = NSMakeRange(textLen, [mySt length]);
		[text replaceCharactersInRange:endRange withString:mySt];
		[text setAttributes:textAttributes range:myStRange];
		[text endEditing];

		[cDoc setReadOnly:YES];
	}
}
// 2020-10-13 end

/* Toggles read-only state of the document
*/
- (IBAction)toggleReadOnly:(id)sender {
#pragma unused (sender)
    [[self undoManager] registerUndoWithTarget:self selector:@selector(toggleReadOnly:) object:nil];
    [[self undoManager] setActionName:[self isReadOnly] ?
        NSLocalizedString(@"Allow Editing", @"Menu item to make the current document editable (not read-only)") :
        NSLocalizedString(@"Prevent Editing", @"Menu item to make the current document read-only")];
    [self setReadOnly:![self isReadOnly]];
}

/* Menu validation
*/
- (BOOL)validateMenuItem:(NSMenuItem *)aCell {
	SEL action = [aCell action];

	if (action == @selector(toggleReadOnly:)) {
		validateToggleItem(aCell, [self isReadOnly], NSLocalizedString(@"Allow Editing", @"Menu item to make the current document editable (not read-only)"), NSLocalizedString(@"Prevent Editing", @"Menu item to make the current document read-only"));
		return YES;
	}
	return [super validateMenuItem:aCell];
}

// For scripting. We already have a -textStorage method implemented above.
- (void)setTextStorage:(id)ts {
    // Warning, undo support can eat a lot of memory if a long text is changed frequently
    NSAttributedString *textStorageCopy = [[self textStorage] copy];
    [[self undoManager] registerUndoWithTarget:self selector:@selector(setTextStorage:) object:textStorageCopy];
    [textStorageCopy release];

    // ts can actually be a string or an attributed string.
    if ([ts isKindOfClass:[NSAttributedString class]]) {
        [[self textStorage] replaceCharactersInRange:NSMakeRange(0, [[self textStorage] length]) withAttributedString:ts];
    } else {
        [[self textStorage] replaceCharactersInRange:NSMakeRange(0, [[self textStorage] length]) withString:ts];
    }
}

- (BOOL)revertToContentsOfURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)outError {
    BOOL success = [super revertToContentsOfURL:url ofType:type error:outError];
    if (success) {
        if (fileTypeToSet) {	// If we're reverting, NSDocument will set the file type behind out backs. This enables restoring that type.
            [self setFileType:fileTypeToSet];
            [fileTypeToSet release];
            fileTypeToSet = nil;
        }
        [[self windowControllers] makeObjectsPerformSelector:@selector(setupTextViewForDocument)];
    }
    return success;
}

@end

@implementation Document (TextEditNSDocumentOverrides)

+ (BOOL)autosavesInPlace {
    return NO; // 2019-08-28
}

+ (BOOL)canConcurrentlyReadDocumentsOfType:(NSString *)typeName {
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    return !([workspace type:typeName conformsToType:(NSString *)kUTTypeHTML] || [workspace type:typeName conformsToType:(NSString *)kUTTypeWebArchive]);
}

/* Indicate the types we know we can save safely asynchronously.
 */
- (BOOL)canAsynchronouslyWriteToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation {
#pragma unused (url, saveOperation)
	BOOL res;
//	res = [[self class] canConcurrentlyReadDocumentsOfType:typeName];
	res = YES;
	return res;
}

- (void)makeWindowControllers {
	NSLog(@"Document: makeWindowControllers\n");
    NSArray *myControllers = [self windowControllers];

    // If this document displaced a transient document, it will already have been assigned a window controller. If that is not the case, create one.
    if ([myControllers count] == 0) {
        [self addWindowController:[[[DocumentWindowController allocWithZone:[self zone]] init] autorelease]];
    }
}

/* One of the determinants of whether a file is locked is whether its type is one of our writable types. However, the writable types are a function of
 whether the document contains attachments. But whether we are locked cannot be a function of whether the document contains attachments, because we
 won't be asked to redetermine autosaving safety after an undo operation resulting from a cancel, so the document would continue to appear locked if
 an image were dragged in and then cancel was pressed.  Therefore, we must use an "ignoreTemporary" boolean to treat RTF as temporarily writable despite
 the attachments.  That's fine since -checkAutosavingSafetyAfterChangeAndReturnError: will perform this check again with ignoreTemporary set to NO,
 and that method will be called when the operation is done and undone, so no inconsistency results.
*/
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)saveOperation ignoreTemporaryState:(BOOL)ignoreTemporary {
    NSMutableArray *outArray = [[[[self class] writableTypes] mutableCopy] autorelease];
    if (saveOperation == NSSaveAsOperation) { // 2020-01-02
		// Documents that contain attachments can only be saved in formats that support embedded graphics.
		if (!ignoreTemporary && [textStorage containsAttachments]) { // ALWAYS NO
			[outArray setArray:[NSArray arrayWithObjects:(NSString *)kUTTypeRTFD, (NSString *)kUTTypeWebArchive, nil]];
		}
    }
    return outArray;
}

- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)saveOperation {
	return [self writableTypesForSaveOperation:saveOperation ignoreTemporaryState:NO];
}

- (NSString *)fileNameExtensionForType:(NSString *)inTypeName saveOperation:(NSSaveOperationType)inSaveOperation {
    /* We use kUTTypeText as our plain text type.  However, kUTTypeText is really a class of types and therefore contains no preferred extension.
	 Therefore we must specify a preferred extension, that of kUTTypePlainText. */
	if ([inTypeName isEqualToString:(NSString *)kUTTypeCHAT /*kUTTypeText*/])
//		return @"txt";
		return @"cha";
    return [super fileNameExtensionForType:inTypeName saveOperation:inSaveOperation];
}

static BOOL isMCEDFileType(char *fpath) {
	int len;

	len = strlen(fpath);
	if (len < 4)
		return(false);
	if (uS.mStricmp(fpath+len-4, ".cha") == 0 || uS.mStricmp(fpath+len-4, ".cut") == 0 ||
		uS.mStricmp(fpath+len-4, ".cex") == 0 || uS.mStricmp(fpath+len-4, ".cdc") == 0)
		return(true);
	return(false);
}

/* When we save, we send a notification so that views that are currently coalescing undo actions can break that. This is
 done for two reasons, one technical and the other HI oriented.

 Firstly, since the dirty state tracking is based on undo, for a coalesced set of changes that span over a save operation,
 the changes that occur between the save and the next time the undo coalescing stops will not mark the document as dirty.
 Secondly, allowing the user to undo back to the precise point of a save is good UI.

 In addition we overwrite this method as a way to tell that the document has been saved successfully. If so, we set the
 save time parameters in the document.
*/
- (void)saveToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation completionHandler:(void (^)(NSError *error))handler {
	char fpath[FNSize];

	// Note that we do the breakUndoCoalescing call even during autosave, which means the user's undo of long typing will take them back to the last spot an autosave occured. This might seem confusing, and a more elaborate solution may be possible (cause an autosave without having to breakUndoCoalescing), but since this change is coming late in Leopard, we decided to go with the lower risk fix.
    [[self windowControllers] makeObjectsPerformSelector:@selector(breakUndoCoalescing)];
	if ([absoluteURL getFileSystemRepresentation:fpath maxLength:FNSize] == NO)
		strcpy(fpath, self->filePath);
	if (uS.mStricmp(fpath, self->filePath) != 0 || isMCEDFileType(fpath) == false) {
		strcpy(self->filePath, fpath);
		[self performAsynchronousFileAccessUsingBlock:^(void (^fileAccessCompletionHandler)(void) ) {
			currentSaveOperation = saveOperation;
			[super saveToURL:absoluteURL ofType:typeName forSaveOperation:saveOperation completionHandler:^(NSError *error) {
				[self setEncodingForSaving:NoStringEncoding];   // This is set during prepareSavePanel:, but should be cleared for future save operation without save panel
				fileAccessCompletionHandler();
				handler(error);
			}];
		}];
	} else {
		[self performSynchronousFileAccessUsingBlock:^(void) {
			currentSaveOperation = saveOperation;
			[super saveToURL:absoluteURL ofType:typeName forSaveOperation:saveOperation completionHandler:^(NSError *error) {
				[self setEncodingForSaving:NoStringEncoding];   // This is set during prepareSavePanel:, but should be cleared for future save operation without save panel
				if (error == nil) {
					NSTextStorage *text;
					NSLayoutManager *tLayoutMgr;
					NSTextContainer *textContainer;
					NSTextView *textView;
					NSRange  cursorRange;

					text = [self textStorage];
					cursorRange = NSMakeRange(pos1C+skipP1, pos2C+skipP2-pos1C+skipP1);
					for (tLayoutMgr in [text layoutManagers]) {
						for (textContainer in [tLayoutMgr textContainers]) {
							textView = [textContainer textView];
							if (textView) {
								[textView setSelectedRange:cursorRange];
								[textView scrollRangeToVisible:cursorRange]; // 2020-05-13
							}
						}
					}
				}
				handler(error);
			}];
		}];
	}
}

- (NSError *)errorInTextEditDomainWithCode:(NSInteger)errorCode {
    switch (errorCode) {
        case TextEditSaveErrorWritableTypeRequired: {
            NSString *description, *recoverySuggestion;
            /* the document can't be saved in its original format, either because TextEdit cannot write to the format, or TextEdit cannot write documents containing
                attachments to the format. */
            if ([textStorage containsAttachments]) {
                description = NSLocalizedString(@"Convert this document to RTFD format?",
                                                    @"Title of alert panel prompting the user to convert to RTFD.");
                recoverySuggestion = NSLocalizedString(
                                                @"Documents with graphics and attachments will be saved using RTFD (RTF with graphics) format. RTFD documents are not compatible with some applications. Convert anyway?",
                                                @"Contents of alert panel prompting the user to convert to RTFD.");
            } else {
                description = NSLocalizedString(@"Convert this document to RTF format?",
                                @"Title of alert panel prompting the user to convert to RTF.");
                recoverySuggestion = NSLocalizedString(@"This document must be converted to RTF before it can be modified.",
                                        @"Contents of alert panel prompting the user to convert to RTF.");
            }
            return [NSError errorWithDomain:TextEditErrorDomain code:TextEditSaveErrorWritableTypeRequired userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                description, NSLocalizedDescriptionKey,
                recoverySuggestion, NSLocalizedRecoverySuggestionErrorKey,
                [NSArray arrayWithObjects:
                    NSLocalizedString(@"Convert", @"Button choice that allows the user to convert the document."),
                    NSLocalizedString(@"Cancel", @"Button choice that allows the user to cancel."),
                    NSLocalizedString(@"Duplicate", @"Button choice that allows the user to duplicate the document.")
                , nil], NSLocalizedRecoveryOptionsErrorKey,
                self, NSRecoveryAttempterErrorKey,
                nil]];
        }
        case TextEditSaveErrorEncodingInapplicable: {
            NSUInteger enc = [self encodingForSaving];
            if (enc == NoStringEncoding)
				enc = NoStringEncoding;
            return [NSError errorWithDomain:TextEditErrorDomain code:TextEditSaveErrorEncodingInapplicable userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
                        [NSString stringWithFormat:NSLocalizedString(@"This document can no longer be saved using its original %@ encodding.", @"Title of alert panel informing user that the file's string encodding needs to be changed."), [NSString localizedNameOfStringEncoding:enc]], NSLocalizedDescriptionKey,
                        NSLocalizedString(@"Please choose another encodding (such as UTF-8).", @"Subtitle of alert panel informing user that the file's string encodding needs to be changed"), NSLocalizedRecoverySuggestionErrorKey,
                        NSLocalizedString(@"The specified text encodding isn't applicable.",
                            @"Failure reason stating that the text encodding is not applicable."), NSLocalizedFailureReasonErrorKey,
                        [NSArray arrayWithObjects:
                            NSLocalizedString(@"OK", @"OK"),
                            NSLocalizedString(@"Cancel", @"Button choice that allows the user to cancel."), nil], NSLocalizedRecoveryOptionsErrorKey,
                        self, NSRecoveryAttempterErrorKey,
                        nil]];
        }
        
    }
    return nil;
}

- (BOOL)checkAutosavingSafetyAfterChangeAndReturnError:(NSError **)outError {
    BOOL safe = YES;
	// it the document isn't saved, don't complain about limitations of its supposed backing store.
	if ([self fileURL]) {
    	if (![[self writableTypesForSaveOperation:NSSaveAsOperation] containsObject:[self fileType]]) {
	        if (outError)
				*outError = [self errorInTextEditDomainWithCode:TextEditSaveErrorWritableTypeRequired];
	        safe = NO;
	    } else {
	        NSUInteger saveEncoding = NoStringEncoding;
	        if (saveEncoding != NoStringEncoding && ![[textStorage string] canBeConvertedToEncoding:saveEncoding]) {
	            if (outError)
					*outError = [self errorInTextEditDomainWithCode:TextEditSaveErrorEncodingInapplicable];
	            safe = NO;
	        }
	    }
	}
    return safe;
}

- (void)updateChangeCount:(NSDocumentChangeType)change {
    NSError *error;
    
	[super updateChangeCount:change];

    if (change == NSChangeDone || change == NSChangeRedone) {
        // If we don't have a file URL, we can change our backing store type without consulting the user.
        // NSDocument will update the extension of our autosaving location.
        // If we don't do this, we won't be able to store images in autosaved untitled documents.
        if (![self fileURL]) {
            if (![[self writableTypesForSaveOperation:NSSaveAsOperation] containsObject:[self fileType]]) {
                [self setFileType:(NSString *)(kUTTypeCHAT /*kUTTypeText*/)];
            }
        } else if (![self checkAutosavingSafetyAfterChangeAndReturnError:&error]) {
            void (^didRecoverBlock)(BOOL) = ^(BOOL didRecover) {
                if (!didRecover) {
                    if (change == NSChangeDone || change == NSChangeRedone) {
                        [[self undoManager] undo];
                    } else if (change == NSChangeUndone) {
                        [[self undoManager] redo];
                    }
                }
            };
            NSWindow *sheetWindow = [self windowForSheet];
            if (sheetWindow) {
                [self performActivityWithSynchronousWaiting:YES usingBlock:^(void (^activityCompletionHandler)()) {
                [self presentError:error
                        modalForWindow:sheetWindow
                        delegate:self
                        didPresentSelector:@selector(didPresentErrorWithRecovery:block:)
                           contextInfo:Block_copy(^(BOOL didRecover) {
                               if (!didRecover) {
                                   if (change == NSChangeDone || change == NSChangeRedone) {
                                       [[self undoManager] undo];
                                   } else if (change == NSChangeUndone) {
                                       [[self undoManager] redo];
                                   }
                               }
                               activityCompletionHandler();
                           })];
                }];
            } else {
                didRecoverBlock([self presentError:error]);
            }
        }
    }
}

- (NSString *)autosavingFileType {
    if (inDuplicate) {
        if (![[self writableTypesForSaveOperation:NSSaveAsOperation] containsObject:[self fileType]])
            return (NSString *)(kUTTypeCHAT /*kUTTypeText*/);
    }
    return [super autosavingFileType];
}

/* When we duplicate a document, we need to temporarily return the autosaving file type for the
    resultant document.  Unfortunately, the only way to do this from a document subclass appears
    to be to use a boolean indicator.
 */
- (NSDocument *)duplicateAndReturnError:(NSError **)outError {
    NSDocument *result;
    inDuplicate = YES;
    result = [super duplicateAndReturnError:outError];
    inDuplicate = NO;
    return result;
}

- (void)document:(NSDocument *)ignored didSave:(BOOL)didSave block:(void (^)(BOOL))block {
    block(didSave);
    Block_release(block);
}

- (void)didPresentErrorWithRecovery:(BOOL)didRecover block:(void (^)(BOOL))block {
    block(didRecover);
    Block_release(block);
}

/* Returns an object that represents the document to be written to file. 
*/
- (id)fileWrapperOfType:(NSString *)typeName error:(NSError **)outError {
    NSTextStorage *text = [self textStorage];
    NSRange range = NSMakeRange(0, [text length]);
    NSStringEncoding enc;
    
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithObjectsAndKeys:
			[NSValue valueWithSize:[self paperSize]], NSPaperSizeDocumentAttribute,
			[NSNumber numberWithInteger:[self isReadOnly] ? 1 : 0], NSReadOnlyDocumentAttribute,
			[NSNumber numberWithFloat:0.0], NSHyphenationFactorDocumentAttribute,
			[NSNumber numberWithDouble:[[self printInfo] leftMargin]], NSLeftMarginDocumentAttribute,
			[NSNumber numberWithDouble:[[self printInfo] rightMargin]], NSRightMarginDocumentAttribute,
			[NSNumber numberWithDouble:[[self printInfo] bottomMargin]], NSBottomMarginDocumentAttribute,
			[NSNumber numberWithDouble:[[self printInfo] topMargin]], NSTopMarginDocumentAttribute,
			[NSNumber numberWithInteger:0], NSViewModeDocumentAttribute,
			[NSNumber numberWithBool:[self usesDocScreenFonts]], NSUsesScreenFontsDocumentAttribute,
		nil];
    NSString *docType = nil;
    id val = nil; // temporary values
    
    NSSize size = [self viewSize];
    if (!NSEqualSizes(size, NSZeroSize)) {
		[dict setObject:[NSValue valueWithSize:size] forKey:NSViewSizeDocumentAttribute];
    }
    
    // TextEdit knows how to save all these types, including their super-types. It does not know how to save any of their potential subtypes. Hence, the conformance check is the reverse of the usual pattern.
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    // kUTTypePlainText also handles kUTTypeText and has to come before the other types so we will use the least specialized type
    // For example, kUTTypeText is an ancestor of kUTTypeText and kUTTypeRTF but we should use kUTTypeText because kUTTypeText is an ancestor of kUTTypeRTF.
	if ([workspace type:(NSString *)@"public.utf8-plain-text" conformsToType:typeName])
		docType = NSPlainTextDocumentType;
    else if ([workspace type:(NSString *)kUTTypePlainText conformsToType:typeName])
		docType = NSPlainTextDocumentType;
    else
		[NSException raise:NSInvalidArgumentException format:@"%@ is not a recognized document type.", typeName];
    
    if (docType)
		[dict setObject:docType forKey:NSDocumentTypeDocumentAttribute];
    if ((val = [self backgroundColor]))
		[dict setObject:val forKey:NSBackgroundColorDocumentAttribute];
    
    if (docType == NSPlainTextDocumentType) {
        enc = NoStringEncoding;
        if ((currentSaveOperation == NSSaveOperation || currentSaveOperation == NSSaveAsOperation) && (documentEncodingForSaving != NoStringEncoding)) {
            enc = documentEncodingForSaving;
        }
        if (enc == NoStringEncoding)
			enc = [self suggestedDocumentEncoding];
        [dict setObject:[NSNumber numberWithUnsignedInteger:enc] forKey:NSCharacterEncodingDocumentAttribute];
    } else if (docType == NSHTMLTextDocumentType || docType == NSWebArchiveTextDocumentType) {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        NSMutableArray *excludedElements = [NSMutableArray array];
        if (![defaults boolForKey:UseXHTMLDocType])
			[excludedElements addObject:@"XML"];
        if (![defaults boolForKey:UseTransitionalDocType])
			[excludedElements addObjectsFromArray:[NSArray arrayWithObjects:@"APPLET", @"BASEFONT", @"CENTER", @"DIR", @"FONT", @"ISINDEX", @"MENU", @"S", @"STRIKE", @"U", nil]];
        if (![defaults boolForKey:UseEmbeddedCSS]) {
            [excludedElements addObject:@"STYLE"];
            if (![defaults boolForKey:UseInlineCSS]) [excludedElements addObject:@"SPAN"];
        }
        if (![defaults boolForKey:PreserveWhitespace]) {
            [excludedElements addObject:@"Apple-converted-space"];
            [excludedElements addObject:@"Apple-converted-tab"];
            [excludedElements addObject:@"Apple-interchange-newline"];
        }
        [dict setObject:excludedElements forKey:NSExcludedElementsDocumentAttribute];
        [dict setObject:[defaults objectForKey:HTMLEncoding] forKey:NSCharacterEncodingDocumentAttribute];
        [dict setObject:[NSNumber numberWithInteger:2] forKey:NSPrefixSpacesDocumentAttribute];
    }
        
    // Set the text layout orientation for each page
    if ((val = [[[self windowControllers] objectAtIndex:0] layoutOrientationSections])) [dict setObject:val forKey:NSTextLayoutSectionsAttribute];

    NSFileWrapper *result = nil;
    if (docType == NSRTFDTextDocumentType) {
		// We obtain a file wrapper from the text storage for RTFD (to produce a directory),
		// or for true plain-text documents (to write out encodding in extended attributes)
        result = [text fileWrapperFromRange:range documentAttributes:dict error:outError]; // returns NSFileWrapper
    } else {
    	NSData *data = [text dataFromRange:range documentAttributes:dict error:outError]; // returns NSData
	if (data) {
	    result = [[[NSFileWrapper alloc] initRegularFileWithContents:data] autorelease];
	    if (!result && outError) *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileWriteUnknownError userInfo:nil];    // Unlikely, but just in case we should generate an NSError
        }
    }

    return result;
}

- (BOOL)checkAutosavingSafetyAndReturnError:(NSError **)outError {
    BOOL safe = YES;
    if (![super checkAutosavingSafetyAndReturnError:outError])
		return NO;
    if ([self fileURL]) {
        // If the document is converted or lossy but can't be saved in its file type, we will need to save it in a different location or duplicate it anyway.
		// Therefore, we should tell the user that a writable type is required instead.
        if (![[self writableTypesForSaveOperation:NSSaveAsOperation ignoreTemporaryState:YES] containsObject:[self fileType]]) {
            if (outError)
				*outError = [self errorInTextEditDomainWithCode:TextEditSaveErrorWritableTypeRequired];
            safe = NO;
        }
    }
    return safe;
}

/* For plain-text documents, we add our own accessory view for selecting encoddings. The plain text case does not require a format popup.
*/
- (BOOL)shouldRunSavePanelWithAccessoryView {
    return NO;
}

/* If the document is a converted version of a document that existed on disk, set the default directory to the directory in which the source file (converted file) resided at the time
 the document was converted.
*/
- (BOOL)prepareSavePanel:(NSSavePanel *)savePanel {
	[self setEncodingForSaving:[self suggestedDocumentEncoding]];
	[savePanel setAllowedFileTypes:[NSArray arrayWithObject:(NSString *)kUTTypeCHAT]];
	[savePanel setAllowsOtherFileTypes:YES];
    return YES;
}

@end

