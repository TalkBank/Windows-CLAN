
/*
     File: DocumentWindowController.m
 Abstract: Document's main window controller object for TextEdit.
 
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

#import "DocumentWinController.h"
#import "IdsController.h"
#import "Document.h"
#import "MultiplePageView.h"
#import "AppDefaultsKeys.h"
#import "AppMisc.h"
#import "AppErrors.h"
#import "CommandsController.h"
#import "UserStrController.h"
#import "WalkerController.h"
#import "SelectF5Controller.h"
#import "SelectMediaController.h"
#import "PictController.h"
#import "check.h"

@interface DocumentWindowController(Private)

- (void)setDocument:(Document *)doc; // Overridden with more specific type. Expects Document instance.

- (void)setupInitialTextViewSharedState;
- (void)setupTextViewForDocument;
- (void)setupPagesViewForLayoutOrientation:(NSTextLayoutOrientation)orientation;

- (void)autosaveIfNeededThenToggleRich;

- (void)showRulerDelayed:(BOOL)flag;

- (void)addPage;
- (void)removePage;

- (NSTextView *)firstTextView;

- (void)printInfoUpdated;

- (void)resizeWindowForViewSize:(NSSize)size;
- (void)setPagesForce:(BOOL)force;

@end

@interface LowerTextView(Public)

- (void)drawWaveForm:(DocumentWindowController *)docWinCtrl;

@end


extern DocumentWindowController *getDocWinController(NSWindow *win);
extern NSFont *defUniFont;
extern NSFont *defCAFont;

extern int MaxNumOfCodes;

extern BOOL DefWindowDims;
extern BOOL isCursorPosRestore;

extern char ced_version[];
extern char ced_err_message[];
extern FNType prefsDir[];


extern struct PBCs PBC;

extern struct DefWin ClanOutSize; // 2020-05-14
extern struct DefWin SpCharsWinSize;// 2020-05-14
extern struct DefWin defWinSize; // 2020-05-15

char doMixedSTWave;

@implementation DocumentWindowController

- (id)init {
	int i;
    if (self = [super initWithWindowNibName:@"DocumentWindow"]) {
		layoutMgr = [[NSLayoutManager allocWithZone:[self zone]] init];
		[layoutMgr setDelegate:self];
		[layoutMgr setAllowsNonContiguousLayout:YES];
		docWinEventMonitor = nil;
		isAddingChar = false;
		textStOrig = NULL;
		isF1_2Key = 0;
		isESCKey = 0;
		cursorKeyDir = 0;
		ShowParags = NO;
		self.lineNumberView = nil;

		for (i=0; i < SPEAKERNAMENUM; i++)
			SpeakerNames[i][0] = EOS;

		SnTr.StatusLineType = 0;
		SnTr.mediaFPath[0] = EOS;
		SnTr.VScale = 0;
		SnTr.HScale = 0L;
		SnTr.WaveWidth = 0;
		SnTr.SoundFileSize = 0L;
		SnTr.TopP1 = NULL;
		SnTr.TopP2 = NULL;
    }
    return self;
}

- (void)dealloc {
    if ([self document])
		[self setDocument:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
	if (docWinEventMonitor) {
		[NSEvent removeMonitor:docWinEventMonitor];
		docWinEventMonitor = nil;
	}
    [[self firstTextView] removeObserver:self forKeyPath:@"backgroundColor"];
    [[scrollView verticalScroller] removeObserver:self forKeyPath:@"scrollerStyle"];
    [layoutMgr release];
    
    [self showRulerDelayed:NO];
    
    [super dealloc]; // NSWindowController deallocates all the nib objects
}

/* This method can be called in three different situations (number three is a special TextEdit case):
	1) When the window controller is created and set up with a new or opened document. (!oldDoc && doc)
	2) When the document is closed, and the controller is about to be destroyed (oldDoc && !doc)
	3) When the window controller is assigned to another document (a document has been opened and it takes the place of an automatically-created window).  In that case this method is called twice.  First as #2 above, second as #1.
 
   The window can be visible or hidden at the time of the message.
*/
- (void)setDocument:(Document *)doc {
	NSLog(@"DocumentWinController: setDocument\n");
	Document *oldDoc = [[self document] retain];

	if (oldDoc) {
		[[self firstTextView] unbind:@"editable"];
	}
	[super setDocument:doc];
	if (doc) {
		[[self firstTextView] bind:@"editable" toObject:self withKeyPath:@"document.readOnly" options:[NSDictionary dictionaryWithObject:NSNegateBooleanTransformerName forKey:NSValueTransformerNameBindingOption]];
	}
	if (oldDoc != doc) {
		if (oldDoc) {
			/* Remove layout manager from the old Document's text storage. No need to retain as we already own the object. */
			[[oldDoc textStorage] removeLayoutManager:layoutMgr];

			[oldDoc removeObserver:self forKeyPath:@"printInfo"];
			[oldDoc removeObserver:self forKeyPath:@"viewSize"];
		}

		if (doc) {
			[[doc textStorage] addLayoutManager:layoutMgr];

			if ([self isWindowLoaded]) {
				[self setPagesForce:NO];
				[self setupInitialTextViewSharedState];
				[self setupWindowForDocument];
				[[doc undoManager] removeAllActions];
			}

			[doc addObserver:self forKeyPath:@"printInfo" options:0 context:NULL];
			[doc addObserver:self forKeyPath:@"viewSize" options:0 context:NULL];
		}
	}

	[oldDoc release];
}

- (void)breakUndoCoalescing {
	[[self firstTextView] breakUndoCoalescing];
}

- (NSLayoutManager *)myLayoutManager {
	return layoutMgr;
}

- (NSTextView *)firstTextView {
	return [[self myLayoutManager] firstTextView];
}

-(NSUInteger)countTotalNumberOfLines {
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger cursorLine, index, len;
	unichar uc;

	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [textSt length];
	if (len == 0)
		return(1);
	cursorLine = 0;
	for (index=0; index < len; cursorLine++) {
		index = NSMaxRange([textSt lineRangeForRange:NSMakeRange(index, 0)]);
	}
	uc = [textSt characterAtIndex:(index-1)];
	if (uc == '\n')
		cursorLine++;
	return(cursorLine);
}

-(NSUInteger)countLineNumbers:(NSUInteger)cursorPos {
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	unichar uc;
	NSUInteger cursorLine, index;

	if (cursorPos == 0) {
		cursorLine = 1;
	} else {
		textView = [self firstTextView];
		text = [textView textStorage];
		textSt = [text string];
		cursorLine = 0;
		for (index=0; index < cursorPos; cursorLine++) {
			index = NSMaxRange([textSt lineRangeForRange:NSMakeRange(index, 0)]);
		}
		if (index == cursorPos) {
			uc = [textSt characterAtIndex:(cursorPos-1)];
			if (uc == '\n')
				cursorLine++;
		}
	}
	return(cursorLine);
}
/*
-(void)textChangeFont:(id)sender
{
	Document *cDoc = [self document];
	NSFont *oldFont, *newFont;

	oldFont = [cDoc docFont];
//	newFont = [[NSFontManager sharedFontManager] selectedFont];
//	newFont = [sender selectedFont];
	newFont = [sender convertFont:oldFont];
//	[cDoc setDocFont:newFont];
}
*/

- (void)setupInitialTextViewSharedState {
	NSTextView *textView = [self firstTextView];

	[textView setUsesFontPanel:YES];
	[textView setUsesFindBar:YES];
//	[textView setUsesFindPanel: YES];
	[textView setIncrementalSearchingEnabled:YES];
	[textView setDelegate:self];
	[textView setAllowsUndo:YES];
	[textView setAllowsDocumentBackgroundColorChange:YES];
	[textView setIdentifier:@"First Text View"];

	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

	// Some settings are not enabled for plain text docs if the default "SubstitutionsEnabledInRichTextOnly" is set to YES.
	// There is no UI at this stage for this preference.
	BOOL substitutionsOK = ![defaults boolForKey:SubstitutionsEnabledInRichTextOnly];
	[textView setContinuousSpellCheckingEnabled:[defaults boolForKey:CheckSpellingAsYouType]];
	[textView setGrammarCheckingEnabled:[defaults boolForKey:CheckGrammarWithSpelling]];
	[textView setAutomaticSpellingCorrectionEnabled:substitutionsOK && [defaults boolForKey:CorrectSpellingAutomatically]];
	[textView setSmartInsertDeleteEnabled:[defaults boolForKey:SmartCopyPaste]];
	[textView setAutomaticQuoteSubstitutionEnabled:substitutionsOK && [defaults boolForKey:SmartQuotes]];
	[textView setAutomaticDashSubstitutionEnabled:substitutionsOK && [defaults boolForKey:SmartDashes]];
	[textView setAutomaticLinkDetectionEnabled:[defaults boolForKey:SmartLinks]];
	[textView setAutomaticDataDetectionEnabled:[defaults boolForKey:DataDetectors]];
	[textView setAutomaticTextReplacementEnabled:substitutionsOK && [defaults boolForKey:TextReplacement]];
/*
	NSFontManager *fontManager = [NSFontManager sharedFontManager];
	[fontManager setAction:@selector(textChangeFont:)];
*/
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
#pragma unused (change, context)
	if (object == [self firstTextView]) {
		if ([keyPath isEqualToString:@"backgroundColor"]) {
			[[self document] setBackgroundColor:[[self firstTextView] backgroundColor]];
		}
	} else if (object == [scrollView verticalScroller]) {
		if ([keyPath isEqualToString:@"scrollerStyle"]) {
			[self invalidateRestorableState];
			NSSize size = [[self document] viewSize];
			if (!NSEqualSizes(size, NSZeroSize)) {
				[self resizeWindowForViewSize:size];
			}
		}
	} else if (object == [self document]) {
		if ([keyPath isEqualToString:@"printInfo"]) {
			[self printInfoUpdated];
		} else if ([keyPath isEqualToString:@"viewSize"]) {
			if (!isSettingSize) {
				NSSize size = [[self document] viewSize];
				if (!NSEqualSizes(size, NSZeroSize)) {
					[self resizeWindowForViewSize:size];
				}
			}
		}
	}
}

- (void)setupTextViewForDocument {
	NSLog(@"DocumentWinController: setupTextViewForDocument\n");
	Document *doc = [self document];
	NSTextView *view = [self firstTextView];
	NSArray *sections = [doc originalOrientationSections];
	NSTextLayoutOrientation orientation = NSTextLayoutOrientationHorizontal;

	if (doc)
		[view setTypingAttributes:[doc defaultTextAttributes:NO]];

	[view setRichText:NO];
	[view setUsesRuler:NO];	// If NO, this correctly gets rid of the ruler if it was up
	[view setUsesInspectorBar:NO];
	if (rulerIsBeingDisplayed)
		[self showRulerDelayed:NO];	// Cancel delayed ruler request
	[view setImportsGraphics:NO];
	[view setBackgroundColor:[doc backgroundColor]];
	[[view layoutManager] setUsesScreenFonts:[doc usesDocScreenFonts]];

	// process the initial container
	if ([sections count] > 0) {
		for (NSDictionary *dict in sections) {
			id rangeValue = [dict objectForKey:NSTextLayoutSectionRange];

			if (!rangeValue || NSLocationInRange(0, [rangeValue rangeValue])) {
				orientation = NSTextLayoutOrientationVertical;
				[[self firstTextView] setLayoutOrientation:orientation];
				break;
			}
		}
	}
}

- (void)printInfoUpdated {
}

/* Method to lazily display ruler. Call with YES to display, NO to cancel display; this method doesn't remove the ruler. 
*/
- (void)showRulerDelayed:(BOOL)flag {
    if (!flag && rulerIsBeingDisplayed) {
        [[self class] cancelPreviousPerformRequestsWithTarget:self selector:@selector(showRuler:) object:self];
    } else if (flag && !rulerIsBeingDisplayed) {
        [self performSelector:@selector(showRuler:) withObject:self afterDelay:0.0];
    }
    rulerIsBeingDisplayed = flag;
}

- (void)showRuler:(id)obj {
    if (rulerIsBeingDisplayed && !obj) [self showRulerDelayed:NO];	// Cancel outstanding request, if not coming from the delayed request
    if ([[NSUserDefaults standardUserDefaults] boolForKey:ShowRuler]) [[self firstTextView] setRulerVisible:YES];
}

/* Used when converting to plain text
*/
- (void)removeAttachments {
	NSTextStorage *attrString = [[self document] textStorage];
	NSTextView *view = [self firstTextView];
	NSUInteger loc = 0;
	NSUInteger end = [attrString length];
	[attrString beginEditing];
	while (loc < end) {	/* Run through the string in terms of attachment runs */
		NSRange attachmentRange;	/* Attachment attribute run */
		NSTextAttachment *attachment = [attrString attribute:NSAttachmentAttributeName atIndex:loc longestEffectiveRange:&attachmentRange inRange:NSMakeRange(loc, end-loc)];
		if (attachment) {	/* If there is an attachment and it is on an attachment character, remove the character */
			unichar ch = [[attrString string] characterAtIndex:loc];
			if (ch == NSAttachmentCharacter) {
				if ([view shouldChangeTextInRange:NSMakeRange(loc, 1) replacementString:@""]) {
					[attrString replaceCharactersInRange:NSMakeRange(loc, 1) withString:@""];
					[view didChangeText];
				}
				end = [attrString length];	/* New length */
			} else
				loc++;	/* Just skip over the current character... */
		}
		else loc = NSMaxRange(attachmentRange);
	}
	[attrString endEditing];
}

/* This method implements panel-based "attach" functionality. Note that as-is, it's set to accept all files; however, by setting allowed types on the open panel it can be restricted to images, etc.
*/

- (void)presenterDidPresent:(BOOL)inDidSucceed soContinue:(void (^)(BOOL didSucceed))inContinuerCopy {
    inContinuerCopy(inDidSucceed);
    Block_release(inContinuerCopy);
}

- (void)configureTypingAttributesAndDefaultParagraphStyleForTextView:(NSTextView *)view {
	NSLog(@"DocumentWinController: configureTypingAttributesAndDefaultParagraphStyleForTextView\n");
    Document *doc = [self document];
    BOOL rich = NO;
    NSDictionary *textAttributes = [doc defaultTextAttributes:rich];
    NSParagraphStyle *paragraphStyle = [textAttributes objectForKey:NSParagraphStyleAttributeName];

    [view setTypingAttributes:textAttributes];
    [view setDefaultParagraphStyle:paragraphStyle];
}

- (NSUInteger)numberOfPages {
    return 1;
}

- (void)updateTextViewGeometry {
    MultiplePageView *pagesView = [scrollView documentView];

    [[[self myLayoutManager] textContainers] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		if (*stop) {
		}
        [[obj textView] setFrame:[pagesView documentRectForPageNumber:idx]];
    }];
}

- (void)setupPagesViewForLayoutOrientation:(NSTextLayoutOrientation)orientation {
    MultiplePageView *pagesView = [scrollView documentView];
    
    [pagesView setLayoutOrientation:orientation];
    [[self firstTextView] setLayoutOrientation:orientation];
    [self updateTextViewGeometry];
    
    [scrollView setHasHorizontalRuler:(NSTextLayoutOrientationHorizontal == orientation) ? YES : NO];
    [scrollView setHasVerticalRuler:(NSTextLayoutOrientationHorizontal == orientation) ? NO : YES];
}

- (void)addPage {
    NSZone *zone = [self zone];
    NSUInteger numberOfPages = [self numberOfPages];
    MultiplePageView *pagesView = [scrollView documentView];
    
    NSSize textSize = [pagesView documentSizeInPage];
    NSTextContainer *textContainer = [[NSTextContainer allocWithZone:zone] initWithContainerSize:textSize];
    NSTextView *textView;
    NSUInteger orientation = [pagesView layoutOrientation];
    NSRect visibleRect = [pagesView visibleRect];
    CGFloat originalWidth = NSWidth([pagesView bounds]);

    [textContainer setWidthTracksTextView:YES];// 2020-04-14
    [textContainer setHeightTracksTextView:YES];

    [pagesView setNumberOfPages:numberOfPages + 1];
    if (NSTextLayoutOrientationVertical == [pagesView layoutOrientation]) {
        visibleRect.origin.x += (NSWidth([pagesView bounds]) - originalWidth);
        [pagesView scrollRectToVisible:visibleRect];
    }

    textView = [[NSTextView allocWithZone:zone] initWithFrame:[pagesView documentRectForPageNumber:numberOfPages] textContainer:textContainer];
    [textView setLayoutOrientation:orientation];
    [textView setHorizontallyResizable:NO];
    [textView setVerticallyResizable:NO];

    if (NSTextLayoutOrientationVertical == orientation) { // Adjust the initial container size
        textSize = NSMakeSize(textSize.height, textSize.width); // Translate size
        [textContainer setContainerSize:textSize];
    }
    [self configureTypingAttributesAndDefaultParagraphStyleForTextView:textView];
    [pagesView addSubview:textView];
    [[self myLayoutManager] addTextContainer:textContainer];

    [textView release];
    [textContainer release];
}

- (void)removePage {
    NSUInteger numberOfPages = [self numberOfPages];
    NSArray *textContainers = [[self myLayoutManager] textContainers];
    NSTextContainer *lastContainer = [textContainers objectAtIndex:[textContainers count] - 1];
    MultiplePageView *pagesView = [scrollView documentView];
    
    [pagesView setNumberOfPages:numberOfPages - 1];
    [[lastContainer textView] removeFromSuperview];
    [[lastContainer layoutManager] removeTextContainerAtIndex:[textContainers count] - 1];
}

- (NSView *)documentView {
    return [scrollView documentView];
}

- (NSUInteger)isOldBullet:(char *)fn txt:(NSString *)textSt tpos:(NSUInteger)pos tlen:(NSUInteger)len {
	BOOL isSkipNextChar;
	unichar ch, bufU[BUFSIZ];
	NSUInteger tpos, i;

	tpos = pos;
	ch = [textSt characterAtIndex:tpos];
	if (ch == '%') {
		tpos++;
		ch = [textSt characterAtIndex:tpos];
		if (ch == 's' || ch == 'S') {
			tpos++;
			ch = [textSt characterAtIndex:tpos];
			if (ch == 'n' || ch == 'N') {
				tpos++;
				ch = [textSt characterAtIndex:tpos];
				if (ch == 'd' || ch == 'D') {
					tpos++;
					ch = [textSt characterAtIndex:tpos];
					if (ch == ':') {
						tpos++;
						ch = [textSt characterAtIndex:tpos];
						if (ch == '"')
							tpos++;
						i = 0;
						bufU[0] = EOS;
						isSkipNextChar = false;
						while (tpos < len) { // Run through the whole text in NSTextStorage *text
							ch = [textSt characterAtIndex:tpos];
							if (ch == 0x2022 || ch == '\n') {
								fn[0] = EOS;
								return(pos);
							}
							if (ch == '"') {
								tpos++;
								ch = [textSt characterAtIndex:tpos];
								if (ch == '_')
									tpos++;
								bufU[i] = EOS;
								UnicodeToUTF8(bufU,strlen(bufU),(unsigned char *)fn,NULL,FILENAME_MAX);
								return(tpos);
							}
							if (ch == ATTMARKER)
								isSkipNextChar = true;
							else if (isSkipNextChar == true)
								isSkipNextChar = false;
							else
								bufU[i++] = ch;
							tpos++;
						}
						fn[0] = EOS;
					}
				}
			}
		}
	}
	return(pos);
}

- (BOOL)getCurrentMediaName:(NSString *)textSt MaxLen:(NSUInteger)len showErr:(BOOL)isShowErr{ // 2020-09-23
	NSUInteger i, suSti, pos, tPos;
	unichar ch, lastCh, suSt[256];
	NSString *subSt;
	Document *cDoc;

	cDoc = [self document];
	if (cDoc == nil)
		return(false);
	cDoc->mediaFileName[0] = EOS;
	pos = 0;
	suSti = 0;
	lastCh = '\n';
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
					for (tPos=pos+MEDIAHEADERLEN; tPos < len; tPos++) {
						ch = [textSt characterAtIndex:tPos];
						if (ch != ' ' && ch != '\t')
							break;
					}
					if (tPos >= len) {
						if (isShowErr == true)
							do_warning_sheet("@Media: header is missing media filename", [self window]);
						return(false);
					}
					i = tPos;
					for (; tPos < len; tPos++) {
						ch = [textSt characterAtIndex:tPos];
						if (ch == ',' || isSpace(ch) || ch == '\n')
							break;
					}
					if (tPos == i) {
						if (isShowErr == true)
							do_warning_sheet("@Media: header is missing media filename", [self window]);
						return(false);
					}
					subSt = [textSt substringWithRange:NSMakeRange(i, tPos-i)];
					if ([subSt length] < FILENAME_MAX) {
						strcpy(cDoc->mediaFileName, [subSt UTF8String]);
					}
				}
			}
		} else if (ch == '*' && lastCh == '\n')
			break;
		lastCh = ch;
		pos++;
	}
	if (cDoc->mediaFileName[0] != EOS)
		return(true);
	if (isShowErr == true)
		do_warning_sheet("Can't find @Media: header", [self window]);
	return(false);
}

- (BOOL) isStopMovie {
	if (AVMediaPlayer == nil)
		return(false);
	if (AVMediaPlayer->playMode != stopped && AVMediaPlayer->playMode != Walker)
		return(true);
	return(false);
}

// 2019-10-20 beg
static unichar CharToSpChar(unsigned short key, unsigned short num) { // CA CHARS
	if (num == 1) {
		if (key == 0xF700) { 				 // up-arrow
			return(0x2191); // 0xe2 86 91
		} else if (key == 0xF701) {			 // down-arrow
			return(0x2193); // 0xe2 86 93
		} else if (key == '1') {			 // rise to high
			return(0x21D7); // 0xe2 87 97
		} else if (key == '2') {			 // rise to mid
			return(0x2197); // 0xe2 86 97
		} else if (key == '3') {			 // level
			return(0x2192); // 0xe2 86 92
		} else if (key == '4') {			 // fall to mid
			return(0x2198); // 0xe2 86 98
		} else if (key == '5') {			 // fall to low
			return(0x21D8); // 0xe2 87 98
		} else if (key == '6') {			 // unmarked ending
			return(0x221E); // 0xe2 88 9e
		} else if (key == '+') {			 // continuation - wavy triple-line equals sign
			return(0x224B); // 0xe2 89 8b
		} else if (key == '.') {			 // inhalation - raised period
			return(0x2219); // 0xe2 88 99
		} else if (key == '=') {			 // latching
			return(0x2248); // 0xe2 89 88
		} else if (key == 'u' || key == 'U'){// uptake
			return(0x2261); // 0xe2 89 88
		} else if (key == '[') {			 // raised [
			return(0x2308); // 0xe2 8c 88
		} else if (key == ']') {			 // raised ]
			return(0x2309); // 0xe2 8c 89
		} else if (key == '{') {			 // lowered [
			return(0x230A); // 0xe2 8c 8a
		} else if (key == '}') {			 // lowered ]
			return(0x230B); // 0xe2 8c 8b
		} else if (key == 0xF703) {			 // faster right-arrow
			return(0x2206); // 0xe2 88 86
		} else if (key == 0xF702) {			 // slower left-arrow
			return(0x2207); // 0xe2 88 87
		} else if (key == '*'){				 // creaky
			return(0x204E); // 0xe2 81 8e
		} else if (key == '/') {			 // unsure
			return(0x2047); // 0xe2 81 87
		} else if (key == '0') {			 // softer
			return(0x00B0); // 0xc2 b0
		} else if (key == ')') {			 // louder
			return(0x25C9); // 0xe2 97 89
		} else if (key == 'd'){				 // low pitch - low bar
			return(0x2581); // 0xe2 96 81
		} else if (key == 'h') {			 // high pitch - high bar
			return(0x2594); // 0xe2 96 94
		} else if (key == 'l' || key == 'L'){// smile voice
			return(0x263A); // 0xe2 98 ba
		} else if (key == 'b'){				 // breathy-voice -♋- NOT CA
			return(0x264b); // 0xe2 99 8b
		} else if (key == 'w' || key == 'W'){// whisper
			return(0x222C); // 0xe2 88 ac
		} else if (key == 'y' || key == 'Y'){// yawn
			return(0x03AB); // ce ab
		} else if (key == 's' || key == 'S'){// singing
			return(0x222E); // 0xe2 88 ae
		} else if (key == 'p' || key == 'P'){// precise
			return(0x00A7); // c2 a7
		} else if (key == 'n' || key == 'N'){// constriction
			return(0x223E); // 0xe2 88 be
		} else if (key == 'r' || key == 'R'){// pitch reset
			return(0x21BB); // 0xe2 86 bb
		} else if (key == 'c' || key == 'C'){// laugh in a word
			return(0x1F29); // 0xe1 bc a9
		} else
			return(0);
	} else if (num == 2) {
		if (key == 'H') {					 // raised h - NOT CA
			return(0x02B0); // ca b0
		} else if (key == ',') {			 // dot diacritic - NOT CA
			return(0x0323); // cc a3
		} else if (key == '<') {			 // Group start marker - NOT CA
			return(0x2039); // 0xe2 80 B9
		} else if (key == '>') {			 // Group end marker - NOT CA
			return(0x203A); // 0xe2 80 BA
		} else if (key == 't' || key == 'T'){// Tag or sentence final particle; „ - NOT CA
			return(0x201E); // 0xe2 80 9E
		} else if (key == 'v' || key == 'V'){// Vocative or summons - ‡ - NOT CA
			return(0x2021); // 0xe2 80 A1
		} else if (key == '-'){				 // Stress - ̄ - NOT CA
			return(0x0304); // cc 84
		} else if (key == 'q'){				 // Glottal stop - ʔ - NOT CA
			return(0x0294); // ca 94
		} else if (key == 'Q'){				 // Hebrew glottal - ʕ - NOT CA
			return(0x0295); // ca 95
		} else if (key == ';'){				 // caron - ̌- NOT CA
			return(0x030C); // cc 8c
		} else if (key == '1'){				 // raised stroke - NOT CA
			return(0x02C8); // cb 88
		} else if (key == '2'){				 // lowered stroke - NOT CA
			return(0x02CC); // cb 8c
		} else if (key == '{'){				 // sign group start marker - NOT CA
			return(0x3014); // 0xe3 80 94
		} else if (key == '}'){				 // sign group end marker - NOT CA
			return(0x3015); // 0xe3 80 95
		} else if (key == 'm'){				 // %pho missing word -…- NOT CA
			return(0x2026); // 0xe2 80 a6
		} else if (key == '_'){				 // Uderline - NOT CA
			return(0x0332); // 0xe2 80 a6
		} else if (key == '\''){			 // open quote “ - NOT CA
			return(0x201C); // 0xe2 80 9c - NOTCA_OPEN_QUOTE
		} else if (key == '"'){				 // close quote ” - NOT CA
			return(0x201D); // 0xe2 80 9d - NOTCA_CLOSE_QUOTE
		} else if (key == '='){				 // crossed equal ≠ - NOT CA
			return(0x2260); // 0xe2 89 a0 - NOTCA_CROSSED_EQUAL
		} else if (key == '/'){				 // left arrow with circle ↫ - NOT CA
			return(0x21AB); // 0xe2 86 ab - NOTCA_LEFT_ARROW_CIRCLE
		} else if (key == ':'){				 // colon for long vowels ː - NOT CA
			return(0x02D0); // 0xe2 CB 90 - NOTCA_VOWELS_COLON
		} else
			return(0);
	} else
		return(0);
}
// 2019-10-20 end

#define EVENTSINPUT

- (void)setPagesForce:(BOOL)force {
	NSTextLayoutOrientation orientation = NSTextLayoutOrientationHorizontal;
	NSZone *zone = [self zone];

	NSLog(@"DocumentWinController: setPagesForce\n");

	if (!force)
		return;

	[[self firstTextView] removeObserver:self forKeyPath:@"backgroundColor"];
	[[self firstTextView] unbind:@"editable"];

	if ([self firstTextView]) {
		orientation = [[self firstTextView] layoutOrientation];
	} else {
		NSArray *sections = [[self document] originalOrientationSections];

		if (([sections count] > 0) && (NSTextLayoutOrientationVertical == [[[sections objectAtIndex:0] objectForKey:NSTextLayoutSectionOrientation] unsignedIntegerValue]))
			orientation = NSTextLayoutOrientationVertical;
	}

	NSSize size = [scrollView contentSize];
	NSTextContainer *textContainer;
	NSTextView *textView;

//		textContainer = [[NSTextContainer allocWithZone:zone] initWithContainerSize:NSMakeSize(size.width, CGFLOAT_MAX)];
	textContainer = [[NSTextContainer allocWithZone:zone] initWithContainerSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)]; // 2020-04-14
	textView = [[NSTextView allocWithZone:zone] initWithFrame:NSMakeRect(0.0, 0.0, size.width, size.height) textContainer:textContainer];
	// Insert the single container as the first container in the layout manager before removing the existing pages in order to preserve the shared view state.
	[[self myLayoutManager] insertTextContainer:textContainer atIndex:0];

	if ([[scrollView documentView] isKindOfClass:[MultiplePageView class]]) {
		NSArray *textContainers = [[self myLayoutManager] textContainers];
		NSUInteger cnt = [textContainers count];
		while (cnt-- > 1) {
			[[self myLayoutManager] removeTextContainerAtIndex:cnt];
		}
	}

	[textContainer setWidthTracksTextView:YES]; // 2021-09-05 YES // 2020-04-14 NO
	[textContainer setHeightTracksTextView:NO];		/* Not really necessary */
	[textView setHorizontallyResizable:YES];			/* Not really necessary */
	[textView setVerticallyResizable:YES];
	[textView setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable]; // 2020-01-27 always keep it ONLY NSViewHeightSizable
	[textView setMinSize:size];	/* Not really necessary; will be adjusted by the autoresizing... */
	[textView setMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];	/* Will be adjusted by the autoresizing... */
	[self configureTypingAttributesAndDefaultParagraphStyleForTextView:textView];

	[textView setLayoutOrientation:orientation]; // this configures the above settings

	/* The next line should cause the multiple page view and everything else to go away */
	[scrollView setDocumentView:textView];

	[scrollView setHasVerticalScroller:YES];
	[scrollView setHasHorizontalScroller:YES];

	[textView release];
	[textContainer release];

	// Show the selected region
	[[self firstTextView] scrollRangeToVisible:[[self firstTextView] selectedRange]];

	[scrollView setHasHorizontalRuler:((orientation == NSTextLayoutOrientationHorizontal) ? NO : NO)];
//	[scrollView setHasHorizontalRuler:((orientation == NSTextLayoutOrientationHorizontal) ? YES : NO)];
	[scrollView setHasVerticalRuler:((orientation == NSTextLayoutOrientationHorizontal) ? NO : YES)];

	[[self firstTextView] addObserver:self forKeyPath:@"backgroundColor" options:0 context:NULL];
	[[self firstTextView] bind:@"editable" toObject:self withKeyPath:@"document.readOnly" options:[NSDictionary dictionaryWithObject:NSNegateBooleanTransformerName forKey:NSValueTransformerNameBindingOption]];

	[[scrollView window] makeFirstResponder:[self firstTextView]];
	[[scrollView window] setInitialFirstResponder:[self firstTextView]];	// So focus won't be stolen (2934918)

	// 2019-10-20 beg
	if (docWinEventMonitor == nil) {
		docWinEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask|NSRightMouseDownMask|NSOtherMouseDownMask|NSKeyDownMask handler:^(NSEvent *incomingEvent) {
			char buf[BUFSIZ], *bufEnd;
			unichar bufU[BUFSIZ];
			BOOL isSPFound, isSetHighlight;
			NSEvent *result = incomingEvent;
			NSWindow *targetWindowForEvent = [incomingEvent window];
			NSString *keys;
			NSEventModifierFlags modFlags;
			NSRange charRange, cursorRange;
			NSRect wFrame = [[self window] frame];
			unichar ch;
			NSTextStorage *text;
			NSTextView *textView;
			NSString *textSt;
			NSUInteger pos, j, len, bufLen;
			long currentTimeL;
			Float64 currentTimeD;
			Document *cDoc;

			modFlags = [incomingEvent modifierFlags]; // (modFlags & NSAlternateKeyMask)
// NSEventModifierFlagCapsLock, NSEventModifierFlagShift, NSEventModifierFlagControl,
// NSEventModifierFlagOption, NSEventModifierFlagCommand, NSEventModifierFlagNumericPad
// NSEventModifierFlagHelp, NSEventModifierFlagFunction, NSEventModifierFlagDeviceIndependentFlagsMask

			if (TrimTierMode == NO)
				textStOrig = NULL;
			isAddingChar = false;
			if (targetWindowForEvent != [self window]) {
				j = 0L;
			} else if ([incomingEvent type] == NSEventTypeKeyDown /*NSKeyDown*/) {
//				NSRange charRangeOut;
				unsigned short keyCode;
				extern char isMORXiMode;
				extern char morTestCom[];
				extern char morXiExec(void);

				cursorKeyDir = 0;
				keyCode = [incomingEvent keyCode];
				keys = [incomingEvent characters];
				ch = 0;
				
				// keyCode 123 - left arrow
				// keyCode 124 - right arrow
				// keyCode 125 - down arrow
				// keyCode 126 - up arrow
				// keyCode 96  - F5
				// keyCode 97  - F6
				// keyCode 98  - F7
				// keyCode 100 - F8
				// keyCode 101 - F9
				// keyCode 53  - Escape
				// keyCode 36  - Enter
				// close window - [[self window] close];
// https://developer.apple.com/documentation/appkit/nsevent/1535851-function-key_unicodes

				cDoc = [self document];
				textView = [self firstTextView];
				text = [textView textStorage];
				textSt = [text string];
				cursorRange = [textView selectedRange];
				len = [text length];
				if (EditorMode == false) {
					if ([self isStopMovie]) {
						[AVMediaPlayer StopAV];
						result = nil; // Don't process the event
					} else if (keyCode == 36) { // CR
						GetCursorCode(13, self);
						result = nil; // Don't process the event
					} else if (keyCode == 8 && (modFlags & NSEventModifierFlagControl)) { // ^C
						GetCursorCode(13, self);
						result = nil; // Don't process the event
					} else if (keyCode == 17 && (modFlags & NSEventModifierFlagControl)) { // ^T
						EndCurTierGotoNext(13, self);
						result = nil; // Don't process the event
					} else if (keyCode == 123) { // 2019-11-21 2020-05-07 - left arrow
						MoveCodeCursorLeft(self);
						result = nil; // Don't process the event
						isF1_2Key = 0;
					} else if (keyCode == 124) { // 2019-11-21 2020-05-07 - right arrow
						MoveCodeCursorRight(self);
						result = nil; // Don't process the event
						isF1_2Key = 0;
					} else if (keyCode == 125) { // - down arrow
						MoveCodeCursorDown(self);
						result = nil; // Don't process the event
						isF1_2Key = 0;
					} else if (keyCode == 126) { // - up arrow
						MoveCodeCursorUp(self);
						result = nil; // Don't process the event
						isF1_2Key = 0;
					} else if ([keys length] == 1) {
						ch = [keys characterAtIndex:0];
						if (isESCKey != 0) { // 2019-11-08 2020-05-07
							isF1_2Key = 0;
							isESCKey = 0;
							if (ch == 'a' || ch == 'A') {
								[self ExpandBullets:self];
							} else if (ch == 'e' || ch == 'E') {
								[self EditMode:self];
							} else if (ch == 'm' || ch == 'M') {
								[self ChatModeSet:self];
							} else if (ch == 's' || ch == 'S') {
								GetFakeCursorCode(1, self);
							} else if (ch == 'c' || ch == 'C') {
								ToTopLevel(13, self);
							} else {
								do_warning_sheet("Please finish coding current line first.", [self window]);
							}
							result = nil; // Don't process the event
						} else if (keyCode == 53) {
							NSResponder *firstResponder = [[self window] firstResponder];
							textView = [self firstTextView];
							if (firstResponder == textView) {
								isESCKey = 1;
								isF1_2Key = 0;
								result = nil; // Don't process the event
							}
						} else if (modFlags & NSEventModifierFlagCommand) {
							if (ch == 'q' || ch == 'Q' || ch == 'w' || ch == 'W') {
							} else {
								do_warning_sheet("Please finish coding current line first.", [self window]);
								result = nil; // Don't process the event
							}
						} else {
							do_warning_sheet("Please finish coding current line first.", [self window]);
							result = nil; // Don't process the event
						}
					} else {
						do_warning_sheet("Please finish coding current line first.", [self window]);
						result = nil; // Don't process the event
					}
				} else if (cDoc != nil && cDoc->wID == CLANWIN && isMORXiMode && keyCode == 36) { // 2020-08-12 2020-08-13
					pos = cursorRange.location;
					if (pos >= len)
						pos = len - 1;
					if (pos > 0) {
						ch = [textSt characterAtIndex:pos];
						if (ch == '\n')
							pos--;
					}
					while (pos > 0) {	// Run through the whole text in NSTextStorage *text
						ch = [textSt characterAtIndex:pos];
						if (ch == '\n') {
							pos++;
							break;
						}
						pos--;
					}
					for (j=0; pos < len; j++) {	// Run through the whole text in NSTextStorage *text
						ch = [textSt characterAtIndex:pos];
						templineW1[j] = ch;
						if (ch == '\n') {
							break;
						}
						pos++;
					}
					templineW1[j] = EOS;
					if (uS.partcmp(templineW1, "mor (:h help)>", FALSE, TRUE)) {
						strcpy(templineW1, templineW1+strlen("mor (:h help)>"));
					}
					uS.remFrontAndBackBlanks(templineW1);
					UnicodeToUTF8(templineW1, j, (unsigned char *)morTestCom, NULL, UTTLINELEN);
					if (pos < len)
						OutputToScreen(templineW1);
					[[textView undoManager] removeAllActions];
					if (morXiExec()) {
						[self window].documentEdited = NO;
						[cDoc updateChangeCount:NSChangeCleared];
//						[[cDoc undoManager] removeAllActions];
						if (commandsWindow != NULL)
							[commandsWindow showWindow:nil];
					}
					isF1_2Key = 0;
					result = nil; // Don't process the event
				} else if (keyCode == 123 && isF1_2Key == 0) { // 2019-11-21 2020-05-07 - left arrow
					if (modFlags & NSCommandKeyMask) {
						[self MoveMediaEndLeft];
						result = nil; // Don't process the event
					} else if (modFlags & NSControlKeyMask) {
						[self MoveMediaBegLeft];
						result = nil; // Don't process the event
					} else {
						cursorKeyDir = -1;
					}
				} else if (keyCode == 124 && isF1_2Key == 0) { // 2019-11-21 2020-05-07 - right arrow
					if (modFlags & NSCommandKeyMask) {
						[self MoveMediaEndRight];
						result = nil; // Don't process the event
					} else if (modFlags & NSControlKeyMask) {
						[self MoveMediaBegRight];
						result = nil; // Don't process the event
					} else {
						cursorKeyDir = 1;
					}
				} else if (keyCode == 125 && isF1_2Key == 0) { // - down arrow
				} else if (keyCode == 126 && isF1_2Key == 0) { // - up arrow
				} else if (keyCode == 49 && AVMediaPlayer != nil && AVMediaPlayer->playMode == F_five) { // F5 space bar 2020-08-19 beg
					BOOL skipToSP;

					if (AVMediaPlayer->isWhatType == isAudio) { // 2021-03-07 beg
						currentTimeD = AVMediaPlayer->audioPlayer.currentTime;
					} else {
						currentTimeD = CMTimeGetSeconds(AVMediaPlayer->playerView.player.currentItem.currentTime);
					}
					currentTimeD = currentTimeD * 1000.000;
					currentTimeL = (long)currentTimeD;
					sprintf(buf, " %s%ld_%ld%s", "•", AVMediaPlayer->lastEndBullet, currentTimeL, "•");
					AVMediaPlayer->lastEndBullet = currentTimeL;
					pos = cursorRange.location + cursorRange.length;

					if (pos >= len)
						pos = len - 1;
					while (pos > 0) {	// Run through the whole text in NSTextStorage *text
						ch = [textSt characterAtIndex:pos];
						if (ch == '\n') {
//							pos--;
							break;
						}
						pos--;
					}
					charRange = NSMakeRange(pos, 0); // (NSUInteger pos, NSUInteger len)

					j = pos;
					isSPFound = false;
					isSetHighlight = false;
					if (j > 0) {
						ch = [textSt characterAtIndex:j];
						if (ch == '\n') {
							j--;
						}
					}
					while (j > 0) {	// Run through the whole text in NSTextStorage *text
						ch = [textSt characterAtIndex:j];
						if (ch == '\n') {
							if (j+1 < len) {
								ch = [textSt characterAtIndex:j+1];
								if (F5Option == EVERY_LINE && isSpace(ch)) {
									j++;
									break;
								} else if (ch == '*') {
									j++;
									break;
								}
							}
						} else if (ch == 0x2022 && j > 0) {
							bufLen = j;
							j--;
							ch = [textSt characterAtIndex:j];
							if (isdigit(ch)) {
								do {
									j--;
									ch = [textSt characterAtIndex:j];
								} while ((isdigit(ch) || ch =='_') && j > 0);
								if (ch == 0x2022) {
									if (j > 0) {
										ch = [textSt characterAtIndex:j-1];
										if (ch == ' ') {
											j--;
										}
									}
									charRange.location = j;
									charRange.length = bufLen - j + 1;
									break;
								}
							}
						}
						j--;
					}

					ch = [textSt characterAtIndex:charRange.location];
					if (ch == '\n' && charRange.location > 0 && charRange.length == 0) {
						if (charRange.location + 1 == len)
							charRange.location--;
					}

					keys = [NSString stringWithUTF8String:buf];
					if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
						[text replaceCharactersInRange:charRange withString:keys];
						len = [text length];
						[textView didChangeText];
						[[textView undoManager] setActionName:@"Bullet"];
					}

					isSPFound = false;
					isSetHighlight = false;
					skipToSP = false;
					bufU[0] = EOS;
					while (pos < len) {	// Run through the whole text in NSTextStorage *text
						ch = [textSt characterAtIndex:pos];
						if (ch == '\n') {
							if (pos+1 < len) {
								ch = [textSt characterAtIndex:pos+1];
								if (F5Option == EVERY_LINE && isSpace(ch) && !skipToSP) {
									pos++;
									cursorRange.location = pos;
									isSPFound = true;
									break;
								} else if (ch == '*') {
									pos++;
									cursorRange.location = pos;
									isSPFound = true;
									break;
								} else if (pos+5 < len) {
									charRange.location = pos + 1;
									charRange.length = 4;
									[textSt getCharacters:bufU range:charRange];
									bufU[4] = EOS;
									if (strcmp(bufU, "@End") == 0) { // 2020-08-24
										pos++;
										break;
									} else if (ch == '%' || ch == '@') {
										skipToSP = true;
									}
								} else if (ch == '%' || ch == '@') {
									skipToSP = true;
								}
							}
						}
						pos++;
					}
					if (isSPFound) {
						while (pos < len) {	// Run through the whole text in NSTextStorage *text
							ch = [textSt characterAtIndex:pos];
							if (ch == '\n') {
								if (F5Option == EVERY_LINE) {
									cursorRange.length = pos - cursorRange.location;
									isSetHighlight = true;
									break;
								} else if (pos+1 < len) {
									ch = [textSt characterAtIndex:pos+1];
									if (ch == '*' || ch == '%' || ch == '@') {
										cursorRange.length = pos + 1 - cursorRange.location;
										isSetHighlight = true;
										break;
									}
								} else if (pos+1 >= len) {
									cursorRange.length = pos - cursorRange.location;
									isSetHighlight = true;
									break;
								}
							}
							pos++;
						}
					} else if (pos >= len || strcmp(bufU, "@End") == 0) { // 2020-08-24 beg
						keys = [NSString stringWithUTF8String:"*:	\n"];
						charRange.location = pos;
						charRange.length = 0;
						if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
							[text replaceCharactersInRange:charRange withString:keys];
							len = [text length];
							if (pos < len) {
								cursorRange.location = pos;
								cursorRange.length = 4;
								isSetHighlight = true;
							}
							[textView didChangeText];
							[[textView undoManager] setActionName:@"EmptyTier"];
						}
					}  // 2020-08-24 end
					if (isSetHighlight) {
						[textView setSelectedRange:cursorRange];
						[textView scrollRangeToVisible:cursorRange];
					}
					isF1_2Key = 0;
					result = nil; // Don't process the event
// 2020-08-19 end
				} else if (keyCode == 96) { // 2020-08-18 beg F5
					if ([self isStopMovie]) {
						[AVMediaPlayer StopAV];
						result = nil; // Don't process the event
					} else
						[self TranscribeSoundOrMovie:self];
					result = nil; // Don't process the event
// 2020-08-18 end
				} else if (keyCode == 97) { // F6 2020-09-22 beg
					if (AVMediaPlayer != nil && AVMediaPlayer->playMode != stopped) {
						[AVMediaPlayer StopAV];
						result = nil; // Don't process the event
					} else if (WalkerControllerWindow == nil) {
						do_warning_sheet("Please open Walker Controller first", [self window]);
					} else if (cDoc != nil) {
						strcpy(theAVinfo.mediaFPath, cDoc->filePath);
						bufEnd = strrchr(theAVinfo.mediaFPath, '/');
						if (bufEnd != NULL) {
							*(bufEnd+1) = EOS;
						}
						if (AVMediaPlayer != nil) {
							if (AVMediaPlayer->isWhatType == isAudio) { // 2021-03-07 beg
								currentTimeD = AVMediaPlayer->audioPlayer.currentTime;
							} else {
								currentTimeD = CMTimeGetSeconds(AVMediaPlayer->playerView.player.currentItem.currentTime);
							}
							currentTimeD = currentTimeD * 1000.000;
							currentTimeL = (long)currentTimeD;
						} else
							currentTimeL = 0L;
						theAVinfo.mediaFName[0] = EOS;
						theAVinfo.beg = currentTimeL;
						theAVinfo.endWalker = -1;
						pos = cursorRange.location;
						if (pos >= len)
							pos = len - 1;
						if (pos > 0) {
							ch = [textSt characterAtIndex:pos];
							if (ch == '\n')
								pos--;
						}
						isSPFound = false;
						while (pos > 0) {	// Run through the whole text in NSTextStorage *text
							ch = [textSt characterAtIndex:pos];
							if (ch == '\n') {
								if (pos+1 < len) {
									ch = [textSt characterAtIndex:pos+1];
									if (ch == '*') {
										pos++;
										cursorRange.location = pos;
										isSPFound = true;
										break;
									}
								}
							}
							pos--;
						}
						if (isSPFound == true) {
							while (pos < len) {	// Run through the whole text in NSTextStorage *text
								ch = [textSt characterAtIndex:pos];
								if (ch == '\n') {
									if (pos+1 < len) {
										ch = [textSt characterAtIndex:pos+1];
										if (ch == '*' || ch == '%' || ch == '@') {
											break;
										}
									} else if (pos+1 >= len) {
										break;
									}
								} else if (ch == 0x2022 && pos+1 < len) {
									pos++;
									pos = [self isOldBullet:theAVinfo.mediaFName txt:textSt tpos:pos tlen:len];
									ch = [textSt characterAtIndex:pos];
									if (isdigit(ch)) {
										bufLen = 0L;
										do {
											buf[bufLen++] = (char)ch;
											pos++;
											ch = [textSt characterAtIndex:pos];
										} while ((isdigit(ch) || ch =='_') && pos < len);
										if (ch == 0x2022) {
											buf[bufLen] = EOS;
											bufEnd = strchr(buf, '_');
											if (buf[0] != EOS && bufEnd != NULL) {
												theAVinfo.beg = atol(buf);
												theAVinfo.endWalker = atol(bufEnd+1);
											}
										}
									} else
										pos--;
								}
								pos++;
							}
						}
						if (theAVinfo.mediaFName[0] == EOS) {
							if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
								result = nil;
								return result;
							}
							strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
						}
						theAVinfo.end = theAVinfo.beg + PBC.step_length;
						if (theAVinfo.endWalker > -1) {
							if (theAVinfo.end > theAVinfo.endWalker)
								theAVinfo.end = theAVinfo.endWalker;
						}
						if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
							theAVinfo.playMode = Walker;
							theAVinfo.textView = textView;
							theAVinfo.docWindow = [self window];
							theAVinfo.nextSegs = nil;
							[AVController createAndPlayAV:&theAVinfo];
							[cDoc showWindows];
						}
					}
					result = nil; // Don't process the event
				} else if (keyCode == 98) { // F7
					if (WalkerControllerWindow == nil) {
						do_warning_sheet("Please open Walker Controller first", [self window]);
					} else if (AVMediaPlayer == nil) {
						do_warning_sheet("Please play media first", [self window]);
					} else if (cDoc != nil) {
						strcpy(theAVinfo.mediaFPath, cDoc->filePath);
						bufEnd = strrchr(theAVinfo.mediaFPath, '/');
						if (bufEnd != NULL) {
							*(bufEnd+1) = EOS;
						}
						if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
							result = nil;
							return result;
						}
						strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
						theAVinfo.beg = AVMediaPlayer->beg - PBC.step_length;
						theAVinfo.end = theAVinfo.beg + PBC.step_length;
						if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
							theAVinfo.playMode = Walker;
							theAVinfo.textView = textView;
							theAVinfo.docWindow = [self window];
							theAVinfo.nextSegs = nil;
							[AVController createAndPlayAV:&theAVinfo];
							[cDoc showWindows];
						}
					}
					result = nil; // Don't process the event
				} else if (keyCode == 100) { // F8
					if (WalkerControllerWindow == nil) {
						do_warning_sheet("Please open Walker Controller first", [self window]);
					} else if (AVMediaPlayer == nil) {
						do_warning_sheet("Please play media first", [self window]);
					} else if (cDoc != nil) {
						strcpy(theAVinfo.mediaFPath, cDoc->filePath);
						bufEnd = strrchr(theAVinfo.mediaFPath, '/');
						if (bufEnd != NULL) {
							*(bufEnd+1) = EOS;
						}
						if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
							result = nil;
							return result;
						}
						strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
						theAVinfo.beg = AVMediaPlayer->beg;
						theAVinfo.end = AVMediaPlayer->end;
						if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
							theAVinfo.playMode = Walker;
							theAVinfo.textView = textView;
							theAVinfo.docWindow = [self window];
							theAVinfo.nextSegs = nil;
							[AVController createAndPlayAV:&theAVinfo];
							[cDoc showWindows];
						}
					}
					result = nil; // Don't process the event
				} else if (keyCode == 101) { // F9
					if (WalkerControllerWindow == nil) {
						do_warning_sheet("Please open Walker Controller first", [self window]);
					} else if (AVMediaPlayer == nil) {
						do_warning_sheet("Please play media first", [self window]);
					} else if (cDoc != nil) {
						strcpy(theAVinfo.mediaFPath, cDoc->filePath);
						bufEnd = strrchr(theAVinfo.mediaFPath, '/');
						if (bufEnd != NULL) {
							*(bufEnd+1) = EOS;
						}
						if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
							result = nil;
							return result;
						}
						strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
						theAVinfo.beg = AVMediaPlayer->end;
						theAVinfo.end = theAVinfo.beg + PBC.step_length;
						if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
							theAVinfo.playMode = Walker;
							theAVinfo.textView = textView;
							theAVinfo.docWindow = [self window];
							theAVinfo.nextSegs = nil;
							[AVController createAndPlayAV:&theAVinfo];
							[cDoc showWindows];
						}
					}
					result = nil; // Don't process the event
// 2020-09-22 end
				} else if ([keys length] == 1) {
					ch = [keys characterAtIndex:0];
					if ([self isStopMovie]) {
						[AVMediaPlayer StopAV];
						result = nil; // Don't process the event
					} else if (isESCKey != 0) { // 2019-11-08 2020-05-07
						isF1_2Key = 0;
						isESCKey = 0;
						if (ch == '8') { // 2020-08-27 beg
							[self ContinuousPlayback:self]; // 2020-09-01
							result = nil; // Don't process the event
							// 2020-08-27 end
						} else if (ch == '9') { // 2021-08-03 beg
							[self ContinuousSkipPausePlayback:self]; // 2021-08-03
							result = nil; // Don't process the event
							// 2021-08-03 end
						} else if (ch == 'l' || ch == 'L') {// 2020-08-28 beg
							[self CheckOpenedFile:self];
							result = nil; // Don't process the event
// 2020-08-28 end
						} else if (ch == 'a' || ch == 'A') {
							[self ExpandBullets:self];
						} else if (ch == 'e' || ch == 'E') {
							[self EditMode:self];
						} else if (ch == 'm' || ch == 'M') {
							[self ChatModeSet:self];
						}  else if (ch == 's' || ch == 'S') {
							GetFakeCursorCode(1, self);
						} else if (ch == 'c' || ch == 'C') {
							ToTopLevel(13, self);
						} else if (ch == '0') {
							[self SonicMode:self];
						} else if (ch == '3' || ch == '"') {
							GetNewCodes(13, self);
						} else if (ch == 't' || ch == 'T') {
							[self SetNextTierName:self];
						} else if (ch == '2') {
							MorDisambiguate(self);
						} else if (ch == 'p' || ch == 'P') {
//							if (cDoc->ShowPercentOfFile)
//								cDoc->ShowPercentOfFile = false;
//							else
//								cDoc->ShowPercentOfFile = true;
//							j = [self countLineNumbers:cursorRange.location];
//							[self setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
						}
						result = nil; // Don't process the event
					} else if (isF1_2Key != 0) {
						ch = CharToSpChar(ch, isF1_2Key);
						isF1_2Key = 0;
						isESCKey = 0;
						if (ch != 0) {
							keys = [NSString stringWithFormat:@"%C", ch];
							result = [NSEvent keyEventWithType:[incomingEvent type]
													  location:[incomingEvent locationInWindow]
												 modifierFlags:[incomingEvent modifierFlags]
													 timestamp:[incomingEvent timestamp]
												  windowNumber:[incomingEvent windowNumber]
													   context:nil
													characters:keys
								   charactersIgnoringModifiers:keys
													 isARepeat:(BOOL)NO // [incomingEvent ARepeat]
													   keyCode:0
									  ];
						}
					} else if (keyCode == 53) {
						NSResponder *firstResponder = [[self window] firstResponder];
						textView = [self firstTextView];
						if (firstResponder == textView) {
							isESCKey = 1;
							isF1_2Key = 0;
							result = nil; // Don't process the event
						}
					} else if (ch == NSF1FunctionKey) {
						isF1_2Key = 1;
						isESCKey = 0;
						result = nil; // Don't process the event
					} else if (ch == NSF2FunctionKey) {
						isF1_2Key = 2;
						result = nil; // Don't process the event
						isESCKey = 0;
					} else {
						if ((modFlags & NSCommandKeyMask) == 0 && ch > 0x20) { // 2021-07-14
							isAddingChar = true;
						}
						isESCKey = 0;
						isF1_2Key = 0;
					}
				} else if ([self isStopMovie]) {
					[AVMediaPlayer StopAV];
					result = nil; // Don't process the event
				} else {
					isF1_2Key = 0;
				}
			} else if ([incomingEvent type] == NSEventTypeLeftMouseDown /*NSLeftMouseDown*/) {
				BOOL isBulletFound, isBulletInfoFound;
				NSInteger clickCount;
				NSUInteger pressedMouseButtons, chrIndex, bulletIndex;
				NSPoint mouseWinLoc, mouseTextLoc;

				cDoc = [self document];
				textView = [self firstTextView];
				text = [textView textStorage];
				pressedMouseButtons = [NSEvent pressedMouseButtons]; // 1 << 0 left mouse, 1 << 1 right mouse, 1 << n, n >=2 other mouse.
				mouseWinLoc = [incomingEvent locationInWindow];
				mouseTextLoc = [textView convertPoint:mouseWinLoc fromView:nil];
				if ([self isStopMovie]) {
					[AVMediaPlayer StopAV];
					result = nil; // Don't process the event
				} else if (EditorMode == false) {
					if (mouseWinLoc.y > wFrame.size.height - 20) {
					} else if (mouseWinLoc.y > (lowerView->lowerFontHeight * lowerView->rowsNum)) {
						do_warning_sheet("Please finish coding current line first.", [self window]);
						result = nil; // Don't process the event
					}
				} else if (mouseTextLoc.x >= 0 && mouseTextLoc.y >= 0) {
					chrIndex = [textView characterIndexForInsertionAtPoint:mouseTextLoc];
					textSt = [text string]; // 2019-10-25
					len = [textSt length];
					if (chrIndex < [textSt length]) { // 2019-10-25
						if (modFlags & NSCommandKeyMask) {
							[self findAndPlayTargetBullet:chrIndex isKeyPress:NO];
						} else if ((clickCount=[incomingEvent clickCount]) == 3) { // 2020-07-31 beg
							if (SnTr.IsSoundOn == true) {
								theAVinfo.mediaFPath[0] = EOS;
								theAVinfo.mediaFName[0] = EOS;
								theAVinfo.beg = 0L;
								theAVinfo.end = 0L;
								isBulletFound = false;
								isBulletInfoFound = false;
tryAgainSonicMode:
								for (j=chrIndex; j < len; j++) { // 2020-03-11
									ch = [textSt characterAtIndex:j];
									if (ch == '\n') {
										break;
									} else if (ch == 0x2022 && j+1 < len) {
										isBulletFound = true;
										bulletIndex = j;
										j++;
										j = [self isOldBullet:theAVinfo.mediaFName txt:textSt tpos:j tlen:len];
										ch = [textSt characterAtIndex:j];
										if (isdigit(ch)) {
											bufLen = 0L;
											do {
												buf[bufLen++] = (char)ch;
												j++;
												ch = [textSt characterAtIndex:j];
											} while ((isdigit(ch) || ch =='_') && j < len);
											if (ch == 0x2022) {
												buf[bufLen] = EOS;
												bufEnd = strchr(buf, '_');
												if (buf[0] != EOS && bufEnd != NULL) {
													theAVinfo.beg = atol(buf);
													theAVinfo.end = atol(bufEnd+1);
													strcpy(theAVinfo.mediaFPath, cDoc->filePath);
													bufEnd = strrchr(theAVinfo.mediaFPath, '/');
													if (bufEnd != NULL) {
														*(bufEnd+1) = EOS;
													}
													if (theAVinfo.mediaFName[0] == EOS) {
														if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
															result = nil;
															return result;
														}
														strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
													}
													isBulletInfoFound = true;
													break;
												}
											}
										} else
											j--;
									}
								}
								if (ch == '\n' && j >= chrIndex && !isBulletFound) {
									while (true) {
										if (j == 0)
											break;
										j--;
										ch = [textSt characterAtIndex:j];
										if (ch == 0x2022) {
											isBulletFound = true;
											bulletIndex = j;
											break;
										} else if (ch == '\n')
											break;
									}
								}
								if (isBulletFound && !isBulletInfoFound) {
									isBulletInfoFound = true;
									j = bulletIndex;
									if (j > 0)
										j--;
									ch = [textSt characterAtIndex:j];
									if (isdigit(ch)) {
										while ((isdigit(ch) || ch =='_')) {
											if (j == 0)
												break;
											j--;
											ch = [textSt characterAtIndex:j];
										}
										if (ch == 0x2022) {
											chrIndex = j;
											goto tryAgainSonicMode;
										}
									}
								}
								if (theAVinfo.beg < theAVinfo.end && theAVinfo.end != 0L) {
									SnTr.BegF = [self conv_from_msec_rep:theAVinfo.beg];
									SnTr.EndF = [self conv_from_msec_rep:theAVinfo.end];
									[self DisplayEndF];
								}
							} else {
								for (pos=chrIndex; pos > 0; pos--) {
									ch = [textSt characterAtIndex:pos];
									if (ch == '\n') {
										if (pos < chrIndex)
											pos++;
										break;
									}
								}
								for (j=chrIndex; j < len; j++) {
									ch = [textSt characterAtIndex:j];
									if (ch == '\n') {
										break;
									}
								}
								charRange = NSMakeRange(pos, j-pos);
								[textSt getCharacters:templine4 range:charRange];
								templine4[charRange.length] = EOS;
								if (uS.mStrnicmp(templine4, "%gra:", 5) == 0 || uS.mStrnicmp(templine4, "%grt:", 5) == 0) {
									[self ShowGRA:(NSUInteger)chrIndex];
								} else {
									extractPath(templineC4, cDoc->filePath);
									if (FindFileLine(FALSE, templineC4, templine4)) {
									}
								}
							}
						} else if ([cDoc get_wID] == SpChrWIN && (clickCount=[incomingEvent clickCount]) == 2) { // 2021-03-28 beg
							BOOL isSpChrFound = false;

							if (gLastTextWinController == nil) {
								do_warning_sheet("Please click on the target text window first", [self window]);
							} else {
								chrIndex = [textView characterIndexForInsertionAtPoint:mouseTextLoc];
								pos = chrIndex;
								if (pos > 0) {
									ch = [textSt characterAtIndex:pos];
									if (ch == '\n' && pos >= chrIndex) {
										pos--;
									}
								}
								for (; pos > 0; pos--) {
									ch = [textSt characterAtIndex:pos];
									if (ch == '\n') {
										if (pos < chrIndex) {
											pos++;
											ch = [textSt characterAtIndex:pos];
											textView = [gLastTextWinController firstTextView];
											text = [textView textStorage];
											cursorRange = [textView selectedRange];
											templineW[0] = ch;
											templineW[1] = EOS;
											isSpChrFound = true;
										}
										break;
									}
								}
								if (pos == 0) {
									if (pos <= chrIndex && pos < [text length]) {
										ch = [textSt characterAtIndex:pos];
										textView = [gLastTextWinController firstTextView];
										text = [textView textStorage];
										cursorRange = [textView selectedRange];
										templineW[0] = ch;
										templineW[1] = EOS;
										isSpChrFound = true;
									}
								}
								if (isSpChrFound == true) {
									cDoc = [gLastTextWinController document];
									if ([text length] == 0) {
										NSString *spChar;
										NSAttributedString *myAttSt;
										NSMutableDictionary *textAttributes;

										spChar = [NSString stringWithCharacters:templineW length:strlen(templineW)]; // NSUnicodeStringEncoding
										textAttributes = [[NSMutableDictionary alloc] initWithCapacity:2];
										[textAttributes setObject:[cDoc docFont] forKey:NSFontAttributeName];
										myAttSt = [[NSAttributedString alloc] initWithString:spChar attributes:textAttributes];
										[text beginEditing];
										[text appendAttributedString:myAttSt];
										[text endEditing];
										[myAttSt release];
										[textAttributes release];
										[cDoc showWindows];
									} else {
										keys = [NSString stringWithCharacters:templineW length:strlen(templineW)];
										if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
											[text replaceCharactersInRange:cursorRange withString:keys];
											[textView didChangeText];
											[[textView undoManager] setActionName:@"SpChars"];
											[cDoc showWindows];
										}
									}
								}
							}
						} // 2021-03-28 end
					}
				} else
					j = 0L;
				j = 0L;
/*
				NSUInteger buttonNumber, characterIndex1, characterIndex2, characterIndex3;
				NSPoint mouseScreenLocation, mouseWinLoc, mouseTextLoc1, mouseTextLoc2, mouseTextLoc2;

				buttonNumber = [incomingEvent buttonNumber];
 				mouseScreenLocation = [NSEvent mouseLocation];
 				mouseWinLoc = [[self window] mouseLocationOutsideOfEventStream];

				mouseTextLoc1 = [textView convertPoint:mouseWinLoc toView:textView];
				characterIndex1 = [textView characterIndexForInsertionAtPoint:mouseTextLoc1];
//				mouseTextLoc2 = [textView convertPoint:mouseWinLoc fromView:textView];
				characterIndex2 = [textView characterIndexForInsertionAtPoint:mouseTextLoc2];
				mouseTextLoc3 = [textView convertPoint:mouseWinLoc fromView:nil];
				characterIndex3 = [textView characterIndexForInsertionAtPoint:mouseTextLoc3];
*/
			}
			return result;
		}];
	}
// 2019-10-20 end
// 2020-09-10 beg
	int i;
	unsigned short wID;
	Document *cDoc;
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN && ChatMode == YES) {
		for (i=0; i < SPEAKERNAMENUM; i++)
			[self AllocSpeakerNames:cl_T("") index:i];
		[self SetUpParticipants:false];
	}
// 2020-09-10 end
}

/* We override these pair of methods so we can stash away the scrollerStyle, since we want to preserve the size
 of the document (rather than the size of the window).
 */
- (void)restoreStateWithCoder:(NSCoder *)coder {
    [super restoreStateWithCoder:coder];
    if ([coder containsValueForKey:@"scrollerStyle"]) {
        NSScrollerStyle previousScrollerStyle = [coder decodeIntegerForKey:@"scrollerStyle"];
        if (previousScrollerStyle != [NSScroller preferredScrollerStyle]) {
            // When we encoded the frame, the window was sized for this saved style. The preferred scroller style has since changed. Given our current frame and the style it had applied, compute how big the view must have been, and then resize ourselves to make the view that size.
            NSSize scrollViewSize = [scrollView frame].size;
            NSSize previousViewSize = [[scrollView class] contentSizeForFrameSize:scrollViewSize horizontalScrollerClass:[scrollView hasHorizontalScroller] ? [NSScroller class] : Nil verticalScrollerClass:[scrollView hasVerticalScroller] ? [NSScroller class] : Nil borderType:[scrollView borderType] controlSize:NSRegularControlSize scrollerStyle:previousScrollerStyle];
            previousViewSize.width -= (defaultTextPadding() * 2.0);
            [self resizeWindowForViewSize:previousViewSize];
        }
    }
}

- (void)encodeRestorableStateWithCoder:(NSCoder *)coder {
    [super encodeRestorableStateWithCoder:coder];
    // Normally you would just encode things that changed; however, since the only invalidation we do is for scrollerStyle, this approach is fine for now.
    [coder encodeInteger:[NSScroller preferredScrollerStyle] forKey:@"scrollerStyle"];
}

- (void)resizeWindowForViewSize:(NSSize)size {
	id scrollBar;
	BOOL hasHScroller, hasVScroller;
	CGFloat tFontHeight = [[self document] fontHeight];
	NSWindow *window = [self window];
	NSRect origWindowFrame = [window frame];
	NSScrollerStyle scrollerStyle;
	NSRect scrollViewRect = [[window contentView] frame];
	size.width += (defaultTextPadding() * 2.0);
	scrollerStyle = [NSScroller preferredScrollerStyle];
	scrollBar = [scrollView class];
	hasHScroller = [scrollView hasHorizontalScroller];
	hasVScroller = [scrollView hasVerticalScroller];
	scrollViewRect.size = [scrollBar frameSizeForContentSize:size
					 horizontalScrollerClass:hasHScroller ? [NSScroller class] : Nil
					   verticalScrollerClass: hasVScroller? [NSScroller class] : Nil
								  borderType:[scrollView borderType]
								 controlSize:NSRegularControlSize
							   scrollerStyle:scrollerStyle];
	NSRect newFrame = [window frameRectForContentRect:scrollViewRect];
	newFrame.size.height += (tFontHeight * [lowerView getRowsCnt]);
	newFrame.origin = NSMakePoint(origWindowFrame.origin.x, NSMaxY(origWindowFrame) - newFrame.size.height);
	[window setFrame:newFrame display:false];
}

- (void)setupWindowForDocument {
	NSLog(@"DocumentWinController: setupWindowForDocument\n");
	Document *cDoc = [self document];
	NSSize viewSize = [cDoc viewSize];
	unsigned short wID = [cDoc get_wID]; // 2020-05-19

	[self setupTextViewForDocument];

	if (!NSEqualSizes(viewSize, NSZeroSize)) { // Document has a custom view size that should be used
		[self resizeWindowForViewSize:viewSize];
	} else { // Set the window size from defaults...
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSInteger windowHeight = [defaults integerForKey:WindowHeight];
		NSInteger windowWidth = [defaults integerForKey:WindowWidth];
		NSFont *tDocFont;
//		windowHeight = 10;
//		windowWidth = 20;

		tDocFont = [cDoc docFont];
		NSSize size;
		size.height = [cDoc fontHeight];
		size.height = ceil(size.height * windowHeight);
		size.width = [@"x" sizeWithAttributes:[NSDictionary dictionaryWithObject:tDocFont forKey:NSFontAttributeName]].width;
		if (size.width == 0.0)
			size.width = [@" " sizeWithAttributes:[NSDictionary dictionaryWithObject:tDocFont forKey:NSFontAttributeName]].width; /* try for space width */
		if (size.width == 0.0)
			size.width = [tDocFont maximumAdvancement].width; /* or max width */
		size.width  = ceil(size.width * windowWidth);
		[self resizeWindowForViewSize:size];
	}
	// 2020-05-09 beg
	NSTextView *textView = [self firstTextView]; // [docView isFlipped];
	NSTextStorage *text;
	NSRange cursorRange;
	NSUInteger cursorLine;

	if (isCursorPosRestore && wID == DOCWIN) { // 2020-05-19
		cursorRange = NSMakeRange(cDoc->pos1C+cDoc->skipP1, cDoc->pos2C+cDoc->skipP2-cDoc->pos1C+cDoc->skipP1);
//		cursorRange = [textView selectedRange];
		text = [textView textStorage];
		cursorLine = [text length];
		if (cursorRange.location+cursorRange.length > cursorLine || cursorLine == 0) {
			cDoc->topC = 0L;
			cDoc->skipTop = 0L;
			cDoc->pos1C = 0L;
			cDoc->skipP1 = 0L;
			cDoc->pos2C = 0L;
			cDoc->skipP2 = 0L;
			cursorLine = 1;
		} else {
			dispatch_async(dispatch_get_main_queue(), ^{
				[textView setSelectedRange:cursorRange];
				[textView scrollRangeToVisible:cursorRange]; // 2020-05-13
			});
			cursorLine = [self countLineNumbers:cursorRange.location];
		}
	} else
		cursorLine = 1;
	ced_err_message[0] = EOS;
	[self setStatusLine:cursorLine extraMess:ced_err_message isUpdateDisplay:YES];
// 2020-05-09 end

/*
 	NSRange theTextRange;
	NSRect layoutRect;
	NSPoint containerOrigin;
	NSPoint cursorPoint;
	NSRect visibleRect = [textView visibleRect];

	[textView scrollRectToVisible:visibleRect];
	visibleRect = [scrollView visibleRect];

	theTextRange = [[textView layoutManager] glyphRangeForCharacterRange:cursorRange actualCharacterRange:NULL];
	layoutRect = [[textView layoutManager] boundingRectForGlyphRange:theTextRange inTextContainer:[textView textContainer]];
	containerOrigin = [textView textContainerOrigin];
	cursorPoint.x = layoutRect.origin.x + containerOrigin.x;
	cursorPoint.y = layoutRect.origin.y + containerOrigin.y;
	[textView scrollPoint:cursorPoint];

	NSRect rect;
	NSSize tSize;
	rect = [scrollView documentVisibleRect];
	rect = [scrollView visibleRect];
	tSize = [scrollView contentSize];

	NSRect mainScreenFrame = [[NSScreen mainScreen] frame];
	layoutRect = [[textView layoutManager] boundingRectForGlyphRange:theTextRange inTextContainer:[textView textContainer]];
//	layoutRect = [[textView layoutManager] boundingRectForGlyphRange:[textView selectedRange] inTextContainer:[textView textContainer]];
	cursorPoint=NSMakePoint(0,layoutRect.origin.y+layoutRect.size.height+(2*scrolling)-[[NSScreen mainScreen] frame].size.height);
//	[textView scrollPoint:cursorPoint];
*/
}

- (void)setStatusLine:(NSUInteger)lineNumber extraMess:(char *)mess isUpdateDisplay:(BOOL)isDisplay { // 2020-05-06
	char status[2048+2];
	Document *cDoc = [self document];

	if (cDoc == nil)
		return;
	
	if (ced_version[0] == EOS)
		strcpy(status, "CLAN ");
	else
		strcpy(status, ced_version);

	if (EditorMode)
		strcat(status, "[E][");
	else
		strcat(status, "[C][");
	if (!ChatMode) {
		strcat(status, "TEXT");
	} else {
		strcat(status, "CHAT");
	}
	if (cDoc->rawTextInput) {
		strcat(status, "-RAW");
	}
	strcat(status, "] ");

//	if (cDoc.documentEdited)
//		strcat(status, "* ");
//	else
//		strcat(status, "  ");

	if (cDoc->ShowPercentOfFile) {
		sprintf(status+strlen(status), "%ld%%", (lineNumber * 100L) / [self countTotalNumberOfLines]);
	} else {
		sprintf(status+strlen(status), "%lu", lineNumber);
	}
	if (mess[0] != EOS) {
		if (mess[0] == '+')
			do_warning_sheet(mess+1, [self window]);
		else if (mess[0] == '-')
			sprintf(status+strlen(status), ": %s", mess+1);
		else
			sprintf(status+strlen(status), ": %s", mess);
		mess[0] = EOS;
	} else {
		if (AVMediaPlayer != nil && AVMediaPlayer->isWhatType == isAudio && AVMediaPlayer->audioPlayer != nil) {
			if (AVMediaPlayer->playMode == oneBullet || AVMediaPlayer->playMode == ESC_eight)
				strcat(status, ": Press any KEY or click mouse to stop");
			else if (AVMediaPlayer->playMode == Walker)
				strcat(status, ": Playing, press any F5-F6 key to stop");
			else if (AVMediaPlayer->playMode == F_five)
				strcat(status, ": Transcribing, click mouse to stop");
		}
		SnTr.StatusLineType = 0;
	}

//	sprintf(status, "TEST %d", lineNumber);
	[lowerView setInfoString:status];
	if (isDisplay == YES)
		[lowerView setNeedsDisplay:YES];
}

- (void)setSliderMinNum:(double)min maxNum:(double)max {
	LowerSlider.minValue = min;
	LowerSlider.maxValue = max;
}

- (void)setSliderHidden:(BOOL)val {
	LowerSlider.hidden = val;
}

- (void)windowDidLoad {
	NSRect wFrame;
	NSFont *font = nil;
	CGFloat newFontHeight;
	Document *cDoc = [self document];
	NSWindow *window = [self window];
	unsigned short wID;

	NSLog(@"DocumentWinController: windowDidLoad\n");
    [super windowDidLoad];

	ChatMode = YES;
	TrimTierMode = NO;
	if (cDoc != NULL) {
		if (cDoc->filePath[0] != EOS) {
			ChatMode = SetChatModeOfDatafileExt(cDoc->filePath, FALSE);
		}
	}
	isSettingSize = NO;
	EditorMode = true; // 2020-09-24
	[self setSliderHidden:YES];
	[self setSliderMinNum:(double)0 maxNum:(double)0];
	SnTr.IsSoundOn = false;
	SnTr.IsPlayMedia = false;
	SnTr.isUpdateSlider = true;
	isDisambiguateAgain = false;

	wID = [cDoc get_wID];
	self.shouldCascadeWindows = NO;
	if ([cDoc docFont] == nil) {
		if (cDoc->isCAFont)
			[cDoc setDocFont:defCAFont];
		else
			[cDoc setDocFont:defUniFont];
	}
	font = [cDoc docFont];

	newFontHeight = [[self myLayoutManager] defaultLineHeightForFont:font];
	[cDoc setFontHeight:newFontHeight];

	[lowerView setRowsCnt:2]; // 2020-09-24 beg
	[lowerView setLowerFont:font];
	[lowerView setLowerFontHeight:newFontHeight]; // 2020-09-24 end

	// This creates the first text view
    [self setPagesForce:YES];
    
    // This sets it up
    [self setupInitialTextViewSharedState];

	if (wID == CLANWIN) {// 2020-05-14
		if (ClanOutSize.height != 0 || ClanOutSize.width != 0) {
			wFrame.origin.y = ClanOutSize.top;
			wFrame.origin.x = ClanOutSize.left;
			wFrame.size.height = ClanOutSize.height;
			wFrame.size.width = ClanOutSize.width;
			[window setFrame:wFrame display:false];
		}
	} else if (wID == SpChrWIN) {// 2020-05-14
		if (SpCharsWinSize.height != 0 || SpCharsWinSize.width != 0) {
			wFrame.origin.y = SpCharsWinSize.top;
			wFrame.origin.x = SpCharsWinSize.left;
			wFrame.size.height = SpCharsWinSize.height;
			wFrame.size.width = SpCharsWinSize.width;
			[window setFrame:wFrame display:false];
		}
	} else if (DefWindowDims == false && cDoc->height != 0 && cDoc->width != 0) {// 2020-03-13
		wFrame.origin.y = cDoc->top;
		wFrame.origin.x = cDoc->left;
		wFrame.size.height = cDoc->height;
		wFrame.size.width = cDoc->width;
		[window setFrame:wFrame display:false];
	} else if (defWinSize.height != 0 || defWinSize.width != 0) {
		wFrame.origin.y = defWinSize.top;
		wFrame.origin.x = defWinSize.left;
		wFrame.size.height = defWinSize.height;
		wFrame.size.width = defWinSize.width;
		[window setFrame:wFrame display:false];
	}
    // This makes sure the window's UI (including text view shared state) is updated to reflect the document
    [self setupWindowForDocument];  // 2020-01-27
    
    [[scrollView verticalScroller] addObserver:self forKeyPath:@"scrollerStyle" options:0 context:NULL];
    [[[self document] undoManager] removeAllActions];

// 2020-09-18 beg
	if (isChatLineNums) {
		if (self.lineNumberView == nil) {
			self.lineNumberView = [[NoodleLineNumberView alloc] initWithScrollView:scrollView];
			[scrollView setVerticalRulerView:self.lineNumberView];
			[scrollView setHasVerticalRuler:YES];
			[scrollView setHasHorizontalRuler:NO];
		}
		[scrollView setRulersVisible:YES];
	}
// 2020-09-18 beg

// 2020-09-25 beg
	NoCodes = true;
	OldCode[0] = EOS;
	cod_fname = NULL;
	EnteredCode = '\0';
	MakeBackupFile = false;
	FreqCountLimit = 0;
	CodingScheme = '\0';
	PriorityCodes = '\001';
	RootCodesArr = NULL;
// 2020-09-25 end
}

// 2020-05-12 beg
- (NSString *)windowTitleForDocumentDisplayName:(NSString *)displayName {
	Document *cDoc = [self document];
	NSString *newPath;
	unsigned short wID;
	char fpath[FNSize];

	if (cDoc == nil)
		return(displayName);
	wID = [cDoc get_wID];
	if (wID == SpChrWIN)
		displayName = @"Special Characters";
	else if (wID == KeysLWIN)
		displayName = @"Commands and Shortcuts";
	else if (wID == CLANWIN)
		displayName = @"CLAN Output";
	else if (wID == DOCWIN) {
		if (cDoc->filePath[0] == EOS) {
			if ([[cDoc fileURL] getFileSystemRepresentation:fpath maxLength:FNSize] == YES)
				strcpy(cDoc->filePath, fpath);
		}
		if (cDoc->filePath[0] != EOS) {
			newPath = [NSString stringWithUTF8String:cDoc->filePath];
			return(newPath);
		}
	}
	return(displayName);
}
// 2020-05-12 end

- (void)setDocumentEdited:(BOOL)edited {
    [super setDocumentEdited:edited];
    if (edited)
		[[self document] setOriginalOrientationSections:nil];
}

/* Layout orientation sections */
- (NSArray *)layoutOrientationSections {
    NSArray *textContainers = [layoutMgr textContainers];
    NSMutableArray *sections = nil;
    NSUInteger layoutOrientation = 0; // horizontal
    NSRange range = NSMakeRange(0, 0);
    
    for (NSTextContainer *container in textContainers) {
        NSUInteger newOrientation = [container layoutOrientation];
        
        if (newOrientation != layoutOrientation) {
            if (range.length > 0) {
                if (!sections) sections = [NSMutableArray arrayWithCapacity:0];
                
                [sections addObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInteger:layoutOrientation], NSTextLayoutSectionOrientation, [NSValue valueWithRange:range], NSTextLayoutSectionRange, nil]];
                
                range.length = 0;
            }
            
            layoutOrientation = newOrientation;
        }
        
        if (layoutOrientation > 0) {
            NSRange containerRange = [layoutMgr characterRangeForGlyphRange:[layoutMgr glyphRangeForTextContainer:container] actualGlyphRange:NULL];
            
            if (range.length == 0) {
                range = containerRange;
            } else {
                range.length = NSMaxRange(containerRange) - range.location;
            }
        }
    }
    
    if (range.length > 0) {
        if (!sections) sections = [NSMutableArray arrayWithCapacity:0];
        
        [sections addObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInteger:layoutOrientation], NSTextLayoutSectionOrientation, [NSValue valueWithRange:range], NSTextLayoutSectionRange, nil]];
    }
    
    return sections;
}

// 2019-11-13 2020-05-07 beg
- (BOOL)isWithinHidenRange:(NSString *)utf16CodeUnits index:(NSUInteger)textI length:(NSUInteger)len {
	NSUInteger i;
	BOOL hideBullet;
	unichar ch;

	if (ShowParags == YES)
		return NO ;
	if (textI >= len)
		return NO ;
	i = textI;
	while (YES) {
		ch = [utf16CodeUnits characterAtIndex:i];
		if (ch == '\n') {
			if (i == textI)
				return NO;
			else
				break;
		}
		if (i == 0)
			break;
		i--;
	}
	if (ch == '\n')
		i++;
	hideBullet = NO;
	for (; i < len; i++) {
		ch = [utf16CodeUnits characterAtIndex:i];
		if (i == textI)
			break;
		if (ch == 0x2022) {
			if (hideBullet == YES)
				hideBullet = NO;
			else
				hideBullet = YES;
		}
	}
	return hideBullet;
}
// 2019-11-13 2020-05-07 end

- (BOOL)isShouldBeHiden:(NSString *)utf16CodeUnits index:(NSUInteger)textI length:(NSUInteger)len {
	NSUInteger i;
	unichar ch;

	if (ShowParags == YES)
		return NO ;
	for (i=textI; i < len; i++) {
		ch = [utf16CodeUnits characterAtIndex:i];
		if (ch == '\n') {
			return NO;
		} else if (ch == 0x2022) {
			return YES;
		}
	}
	return NO;
}

- (IBAction)ContinuousPlayback:(id)sender { // 2020-09-01 beg
#pragma unused (sender)
	char buf[BUFSIZ], *bufEnd, fpath[FNSize], bType;
	char FName[FILENAME_MAX], oldFName[FILENAME_MAX];
	BOOL isSPFound, isSetHighlight, isSetFirstHighlight, isOldStyleBullets;
	NSRange cursorRange;
	unichar ch, bufU[BUFSIZ];
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger pos, j, len, bufLen, bulletPosBeg, bulletPosEnd;
	Document *cDoc;
	struct AVInfoNextSeg *cSeg;

	isOldStyleBullets = false;
	FName[0] = EOS;
	oldFName[0] = EOS;
	cDoc = [self document];
	if (cDoc->filePath[0] == EOS) {
		if ([[cDoc fileURL] getFileSystemRepresentation:fpath maxLength:FNSize] == YES)
			strcpy(cDoc->filePath, fpath);
		if (cDoc->filePath[0] == EOS) {
			do_warning_sheet("Can't determine document's file path", [self window]);
			return;
		}
	}
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];

	pos = cursorRange.location;
	if (pos >= len)
		pos = len - 1;
	isSPFound = false;
	isSetHighlight = false;
	isSetFirstHighlight = false;
	j = pos;
	if (j > 0) {
		ch = [textSt characterAtIndex:j];
		if (ch == '\n')
			j--;
	}
	while (j > 0) {
		ch = [textSt characterAtIndex:j];
		if (ch == '\n' && j+1 < len) {
			ch = [textSt characterAtIndex:j+1];
			if (ch == '*') {
				j++;
				cursorRange.location = j;
				isSPFound = true;
				pos = j;
				break;
			}
		}
		j--;
	}
	if (isSPFound == false && pos < len) {
		ch = [textSt characterAtIndex:pos+1];
		if (ch == '*') {
			cursorRange.location = pos+1;
			isSPFound = true;
		} else {
			while (pos < len) {	// Run through the whole text in NSTextStorage *text
				ch = [textSt characterAtIndex:pos];
				if (ch == 0x2022) {
					bulletPosBeg = pos;
					bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
					if (bulletPosBeg < bulletPosEnd) {
						if (bType == picBullet) {
							bufLen = 0L;
							for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
								ch = [textSt characterAtIndex:pos];
								if (ch != '"')
									bufU[bufLen++] = ch; 
							}
							bufU[bufLen] = EOS;
							if (bufU[0] != EOS) {
								extractPath(FileName1, cDoc->filePath);
								UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
								strcpy(FileName2, templineC2);
								[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
							}
						}
						pos = bulletPosEnd;
					}
				} else if (ch == '\n' || pos == 0) {
					if (pos+1 < len) {
						ch = [textSt characterAtIndex:pos+1];
						if (ch == '*') {
							pos++;
							cursorRange.location = pos;
							isSPFound = true;
							break;
						}
					}
				}
				pos++;
			}
		}
	}
	theAVinfo.beg = -1L;
	theAVinfo.end = -1L;
	theAVinfo.nextSegs = nil; // 2021-07-30
	cSeg = nil;
	if (isSPFound) {
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				if (isSetHighlight) {
					if (isSetFirstHighlight == false) {
						isSetFirstHighlight = true;
						cursorRange.length = pos - cursorRange.location; // 2020-12-02
					}
				}
/*
				if (pos+1 < len) {
					ch = [textSt characterAtIndex:pos+1];
					if (ch == '*' || ch == '%' || ch == '@') {
						cursorRange.length = pos + 1 - cursorRange.location;
						break;
					}
				} else if (pos+1 >= len) {
					cursorRange.length = pos - cursorRange.location;
					break;
				}
*/
			} else if (ch == 0x2022) {
				bulletPosBeg = pos;
				bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
				if (bulletPosBeg < bulletPosEnd) {
					if (bType == picBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							if (ch != '"')
								bufU[bufLen++] = ch; 
						}
						bufU[bufLen] = EOS;
						if (bufU[0] != EOS) {
							extractPath(FileName1, cDoc->filePath);
							UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
							strcpy(FileName2, templineC2);
							[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
						}
					} else {
						if (bType == oldBullet) {
							do_warning_sheet("Continuous playback does not work with old style bullets", [self window]);
							return;
//							bulletPosBeg++;
//							bulletPosBeg = [self isOldBullet:FName txt:textSt tpos:bulletPosBeg tlen:len];
//							bulletPosBeg--;
//							isOldStyleBullets = true;
						}
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							if (theAVinfo.beg == -1) {
								if (FName[0] != EOS) {
									strcpy(theAVinfo.mediaFName, FName);
									strcpy(oldFName, FName);
								}
								theAVinfo.beg = atol(buf);
								theAVinfo.endContPlay = atol(bufEnd+1);
								theAVinfo.end = atol(bufEnd+1);
							} else if (strchr(buf, '-') != NULL || uS.mStricmp(oldFName, FName)) {
								if (theAVinfo.nextSegs == nil) {
									theAVinfo.nextSegs = NEW(struct AVInfoNextSeg);
									cSeg = theAVinfo.nextSegs;
								} else {
									for (cSeg=theAVinfo.nextSegs; cSeg->nextSeg != nil; cSeg=cSeg->nextSeg) ;
									cSeg->nextSeg = NEW(struct AVInfoNextSeg);
									cSeg = cSeg->nextSeg;
								}
								if (cSeg == nil) {
									break;
								}
								cSeg->nextSeg = nil;
								if (FName[0] != EOS) {
									strcpy(cSeg->mediaFName, FName);
									strcpy(oldFName, FName);
								}
								cSeg->beg = atol(buf);
								cSeg->end = atol(bufEnd+1);
							} else {
								if (cSeg == nil)
									theAVinfo.end = atol(bufEnd+1);
								else
									cSeg->end = atol(bufEnd+1);
							}
							isSetHighlight = true;
						}
					}
					pos = bulletPosEnd;
				}
			}
			pos++;
		}
	}
	if (isSetHighlight && cDoc != NULL) {
		if (isOldStyleBullets == false) {
			if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
				return;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
		}
		strcpy(theAVinfo.mediaFPath, cDoc->filePath);
		bufEnd = strrchr(theAVinfo.mediaFPath, '/');
		if (bufEnd != NULL) {
			*(bufEnd+1) = EOS;
		}
		if (theAVinfo.beg >= 0L || theAVinfo.end > 0L) {
			if (SnTr.IsSoundOn == true) {
				SnTr.BegF = [self conv_from_msec_rep:theAVinfo.beg];
				SnTr.EndF = [self conv_from_msec_rep:theAVinfo.endContPlay];
				[self DisplayEndF];
			}
			[textView setSelectedRange:cursorRange];
			[textView scrollRangeToVisible:cursorRange];
			theAVinfo.playMode = ESC_eight;
			theAVinfo.textView = textView;
			theAVinfo.docWindow = [self window];
			[AVController createAndPlayAV:&theAVinfo];
			[cDoc showWindows];
		}
	}
	isF1_2Key = 0;
} // 2020-09-01 end

- (IBAction)ContinuousSkipPausePlayback:(id)sender { // 2021-08-03 beg
#pragma unused (sender)
	char buf[BUFSIZ], *bufEnd, fpath[FNSize], bType;
	char FName[FILENAME_MAX], oldFName[FILENAME_MAX];
	BOOL isSPFound, isSetHighlight, isSetFirstHighlight, isOldStyleBullets;
	NSRange cursorRange;
	unichar ch, bufU[BUFSIZ];
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger pos, j, len, bufLen, bulletPosBeg, bulletPosEnd;
	Document *cDoc;
	struct AVInfoNextSeg *cSeg;
	
	isOldStyleBullets = false;
	FName[0] = EOS;
	oldFName[0] = EOS;
	cDoc = [self document];
	if (cDoc->filePath[0] == EOS) {
		if ([[cDoc fileURL] getFileSystemRepresentation:fpath maxLength:FNSize] == YES)
			strcpy(cDoc->filePath, fpath);
		if (cDoc->filePath[0] == EOS) {
			do_warning_sheet("Can't determine document's file path", [self window]);
			return;
		}
	}
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	
	pos = cursorRange.location;
	if (pos >= len)
		pos = len - 1;
	isSPFound = false;
	isSetHighlight = false;
	isSetFirstHighlight = false;
	j = pos;
	if (j > 0) {
		ch = [textSt characterAtIndex:j];
		if (ch == '\n')
			j--;
	}
	while (j > 0) {
		ch = [textSt characterAtIndex:j];
		if (ch == '\n' && j+1 < len) {
			ch = [textSt characterAtIndex:j+1];
			if (ch == '*') {
				j++;
				cursorRange.location = j;
				isSPFound = true;
				pos = j;
				break;
			}
		}
		j--;
	}
	if (isSPFound == false && pos < len) {
		ch = [textSt characterAtIndex:pos+1];
		if (ch == '*') {
			cursorRange.location = pos+1;
			isSPFound = true;
		} else {
			while (pos < len) {	// Run through the whole text in NSTextStorage *text
				ch = [textSt characterAtIndex:pos];
				if (ch == 0x2022) {
					bulletPosBeg = pos;
					bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
					if (bulletPosBeg < bulletPosEnd) {
						if (bType == picBullet) {
							bufLen = 0L;
							for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
								ch = [textSt characterAtIndex:pos];
								if (ch != '"')
									bufU[bufLen++] = ch; 
							}
							bufU[bufLen] = EOS;
							if (bufU[0] != EOS) {
								extractPath(FileName1, cDoc->filePath);
								UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
								strcpy(FileName2, templineC2);
								[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
							}
						}
						pos = bulletPosEnd;
					}
				} else if (ch == '\n' || pos == 0) {
					if (pos+1 < len) {
						ch = [textSt characterAtIndex:pos+1];
						if (ch == '*') {
							pos++;
							cursorRange.location = pos;
							isSPFound = true;
							break;
						}
					}
				}
				pos++;
			}
		}
	}
	theAVinfo.beg = -1L;
	theAVinfo.end = -1L;
	theAVinfo.nextSegs = nil; // 2021-07-30
	cSeg = nil;
	if (isSPFound) {
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				if (isSetHighlight) {
					if (isSetFirstHighlight == false) {
						isSetFirstHighlight = true;
						cursorRange.length = pos - cursorRange.location; // 2020-12-02
					}
				}
			} else if (ch == 0x2022) {
				bulletPosBeg = pos;
				bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
				if (bulletPosBeg < bulletPosEnd) {
					if (bType == picBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							if (ch != '"')
								bufU[bufLen++] = ch; 
						}
						bufU[bufLen] = EOS;
						if (bufU[0] != EOS) {
							extractPath(FileName1, cDoc->filePath);
							UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
							strcpy(FileName2, templineC2);
							[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
						}
					} else {
						if (bType == oldBullet) {
							do_warning_sheet("Continuous playback does not work with old style bullets", [self window]);
							return;
//							bulletPosBeg++;
//							bulletPosBeg = [self isOldBullet:FName txt:textSt tpos:bulletPosBeg tlen:len];
//							bulletPosBeg--;
//							isOldStyleBullets = true;
						}
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							if (theAVinfo.beg == -1) {
								if (FName[0] != EOS)
									strcpy(theAVinfo.mediaFName, FName);
								theAVinfo.beg = atol(buf);
								theAVinfo.endContPlay = atol(bufEnd+1);
								theAVinfo.end = atol(bufEnd+1);
							} else {
								if (theAVinfo.nextSegs == nil) {
									theAVinfo.nextSegs = NEW(struct AVInfoNextSeg);
									cSeg = theAVinfo.nextSegs;
								} else {
									for (cSeg=theAVinfo.nextSegs; cSeg->nextSeg != nil; cSeg=cSeg->nextSeg) ;
									cSeg->nextSeg = NEW(struct AVInfoNextSeg);
									cSeg = cSeg->nextSeg;
								}
								if (cSeg == nil) {
									break;
								}
								cSeg->nextSeg = nil;
								if (FName[0] != EOS) {
									strcpy(cSeg->mediaFName, FName);
									strcpy(oldFName, FName);
								}
								cSeg->beg = atol(buf);
								cSeg->end = atol(bufEnd+1);
							}
							isSetHighlight = true;
						}
					}
					pos = bulletPosEnd;
				}
			}
			pos++;
		}
	}
	if (isSetHighlight && cDoc != NULL) {
		if (isOldStyleBullets == false) {
			if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
				return;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
		}
		strcpy(theAVinfo.mediaFPath, cDoc->filePath);
		bufEnd = strrchr(theAVinfo.mediaFPath, '/');
		if (bufEnd != NULL) {
			*(bufEnd+1) = EOS;
		}
		if (theAVinfo.beg >= 0L || theAVinfo.end > 0L) {
			if (SnTr.IsSoundOn == true) {
				SnTr.BegF = [self conv_from_msec_rep:theAVinfo.beg];
				SnTr.EndF = [self conv_from_msec_rep:theAVinfo.endContPlay];
				[self DisplayEndF];
			}
			[textView setSelectedRange:cursorRange];
			[textView scrollRangeToVisible:cursorRange];
			theAVinfo.playMode = ESC_eight;
			theAVinfo.textView = textView;
			theAVinfo.docWindow = [self window];
			[AVController createAndPlayAV:&theAVinfo];
			[cDoc showWindows];
		}
	}
	isF1_2Key = 0;
} // 2021-08-03 end

- (NSUInteger)isLegalBullet:(NSUInteger)pos bTest:(char *)bType {
	unichar ch, bufU[BUFSIZ];
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger bulletPosEnd, len;

	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	pos++;
	if (pos >= len)
		return(0);
	*bType = newBullet;
	bulletPosEnd = 0;
	if (pos+5 < len) {
		[textSt getCharacters:bufU range:NSMakeRange(pos, 5)];
		bufU[5] = EOS;
		if (uS.mStricmp(bufU, "%pic:") == 0)
			*bType = picBullet;
		else if (uS.mStricmp(bufU, "%snd:") == 0)
			*bType = oldBullet;
	}
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			break;
		} else if (ch == 0x2022) {
			bulletPosEnd = pos;
			break;
		} else if ((*bType == picBullet || *bType==oldBullet) && isSpace(ch)) {
			break;
		} else if (*bType==newBullet && !isdigit(ch) && ch != '_' && ch != '-') {
			break;
		}
	}
	return(bulletPosEnd);
}

- (void)findAndPlayTargetBullet:(NSUInteger)pos isKeyPress:(BOOL)isKeyboard { // 2021-05-08 beg
	char buf[BUFSIZ], *bufEnd, bType;
	unichar ch, bufU[BUFSIZ];
	BOOL isBulletInfoFound;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger chrIndex, bulletPosBeg, bulletPosEnd, len, bufLen;
	Document *cDoc;

	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	theAVinfo.mediaFPath[0] = EOS;
	theAVinfo.mediaFName[0] = EOS;
	theAVinfo.beg = 0L;
	theAVinfo.end = 0L;
	isBulletInfoFound = false;
	buf[0] = EOS;
	bufU[0] = EOS;
	chrIndex = pos;
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
			chrIndex--;
		}
	}
	while (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1)
				return;
			pos++;
			break;
		}
		pos--;
	}
	if (pos == 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1)
				return;
			pos++;
		}
	}
	
	bType = newBullet;
	bulletPosBeg = 0;
	bulletPosEnd = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (bulletPosBeg < bulletPosEnd) {
				if (bType == picBullet) {
					bufLen = 0L;
					for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
						ch = [textSt characterAtIndex:pos];
						if (ch != '"')
							bufU[bufLen++] = ch; 
					}
					bufU[bufLen] = EOS;
					isBulletInfoFound = true;
					break;
				} else {
					if (bType == oldBullet) {
						bulletPosBeg++;
						bulletPosBeg = [self isOldBullet:theAVinfo.mediaFName txt:textSt tpos:bulletPosBeg tlen:len];
						bulletPosBeg--;
					}
					bufLen = 0L;
					for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
						ch = [textSt characterAtIndex:pos];
						buf[bufLen++] = (char)ch;
					}
					buf[bufLen] = EOS;
					bufEnd = strchr(buf, '_');
					if (buf[0] != EOS && bufEnd != NULL) {
						theAVinfo.beg = atol(buf);
						theAVinfo.end = atol(bufEnd+1);
						isBulletInfoFound = true;
						break;
					}
				}
			} else {
				if (isKeyboard == YES) {
					if (pos+1 < len) {
						ch = [textSt characterAtIndex:pos+1];
						if (ch == '*') {
							do_warning_sheet("Can't find a right bullet on this line", [self window]);
							return;
						}
					} else {
						do_warning_sheet("Can't find a right bullet on this line", [self window]);
						return;
					}
				} else {
					do_warning_sheet("Can't find a right bullet on this line", [self window]);
					return;
				}
			}
		} else if (ch == 0x2022) {
			bulletPosBeg = pos;
			bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
			if (bulletPosBeg < bulletPosEnd) {
				if (chrIndex <= bulletPosEnd) {
					if (bType == picBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+6; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							if (ch != '"')
								bufU[bufLen++] = ch; 
						}
						bufU[bufLen] = EOS;
						isBulletInfoFound = true;
						break;
					} else {
						if (bType == oldBullet) {
							bulletPosBeg++;
							bulletPosBeg = [self isOldBullet:theAVinfo.mediaFName txt:textSt tpos:bulletPosBeg tlen:len];
							bulletPosBeg--;
						}
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							theAVinfo.beg = atol(buf);
							theAVinfo.end = atol(bufEnd+1);
							isBulletInfoFound = true;
							break;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
	}

	if (isBulletInfoFound) {
		if (bType == picBullet) {
			if (bufU[0] != EOS) {
				if (cDoc->filePath[0] == EOS)
					strcpy(FileName1, prefsDir);
				else
					extractPath(FileName1, cDoc->filePath);
				UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
				strcpy(FileName2, templineC2);
				[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
			} else
				do_warning_sheet("%pic: bullet is missing picture filename", [self window]);
		} else {
			if (theAVinfo.mediaFName[0] == EOS) {
				if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false)
					return;
				strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
			}
			if (cDoc->filePath[0] != EOS) {
				strcpy(theAVinfo.mediaFPath, cDoc->filePath);
				bufEnd = strrchr(theAVinfo.mediaFPath, '/');
				if (bufEnd != NULL) {
					*(bufEnd+1) = EOS;
				}
				if (theAVinfo.beg >= 0L || theAVinfo.end > 0L) {
					theAVinfo.playMode = oneBullet;
					theAVinfo.textView = textView;
					theAVinfo.docWindow = [self window];
					theAVinfo.nextSegs = nil;
					[AVController createAndPlayAV:&theAVinfo];
				}
			} else
				do_warning_sheet("Can't determine document's file path", [self window]);
		}
	}
	isF1_2Key = 0;
} // 2021-05-08 end

- (IBAction)PlayBulletMedia:(id)sender { // 2020-09-01 beg
#pragma unused (sender)
	NSTextView *textView;
	NSTextStorage *text;
	NSRange cursorRange;
	NSUInteger chrIndex, len;
	
	textView = [self firstTextView];
	text = [textView textStorage];
	len = [text length];
	cursorRange = [textView selectedRange];
	chrIndex = cursorRange.location;
	if (chrIndex >= len)
		chrIndex = len - 1;
	[self findAndPlayTargetBullet:chrIndex isKeyPress:YES];
} // 2020-09-01 eng

- (IBAction)TranscribeSoundOrMovie:(id)sender { // 2020-08-18 beg // 2020-09-01 beg
#pragma unused (sender)
	char buf[BUFSIZ], *bufEnd;
	char fpath[FNSize];
	unichar bufU[BUFSIZ];
	BOOL isSPFound, isSetHighlight;
	NSString *keys;
	NSRange charRange, cursorRange;
	unichar ch;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger pos, j, len, bufLen;
	Document *cDoc;

	cDoc = [self document];
	if (cDoc->filePath[0] == EOS) {
		if ([[cDoc fileURL] getFileSystemRepresentation:fpath maxLength:FNSize] == YES)
			strcpy(cDoc->filePath, fpath);
		if (cDoc->filePath[0] == EOS) {
			do_warning_sheet("Can't determine document's file path", [self window]);
			return;
		}
	}
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];

	pos = cursorRange.location;
	if (pos >= len)
		pos = len - 1;
	isSPFound = false;
	isSetHighlight = false;
	if (F5Option == EVERY_LINE && pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n' && pos > 0) {
			pos--;
		}
	}
	if (len == 0) {
		do_warning_sheet("@Media: header is missing media filename", [self window]);
		return;
	}
	while (pos > 0) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
				if (F5Option == EVERY_LINE && isSpace(ch)) {
					pos++;
					cursorRange.location = pos;
					isSPFound = true;
					break;
				} else if (ch == '*') {
					pos++;
					cursorRange.location = pos;
					isSPFound = true;
					break;
				}
			}
		}
		pos--;
	}
	theAVinfo.beg = -1L;
	theAVinfo.end = -1L;
	if (isSPFound == true) {
		j = pos;
		while (j > 0) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:j];
			if (ch == 0x2022 && j > 0) {
				j--;
				ch = [textSt characterAtIndex:j];
				if (isdigit(ch)) {
					do {
						j--;
						ch = [textSt characterAtIndex:j];
					} while ((isdigit(ch) || ch =='_') && j > 0);
					if (ch == 0x2022) {
						bufLen = 0L;
						j++;
						ch = [textSt characterAtIndex:j];
						while (ch != 0x2022 && j < len) {
							buf[bufLen++] = (char)ch;
							j++;
							ch = [textSt characterAtIndex:j];
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							theAVinfo.beg = atol(bufEnd+1);
						}
						break;
					}
				}
			}
			j--;
		}
	} else {
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				if (pos+1 < len) {
					ch = [textSt characterAtIndex:pos+1];
					if (ch == '*') {
						pos++;
						cursorRange.location = pos;
						isSPFound = true;
						break;
					} else if (pos+5 < len) {
						charRange.location = pos + 1; // 2020-08-31
						charRange.length = 4;
						[textSt getCharacters:bufU range:charRange];
						bufU[4] = EOS;
						if (strcmp(bufU, "@End") == 0 && isSPFound == false) { // 2020-08-31
							pos++;
							break;
						}
					}
				}
			}
			pos++;
		}
	}
	if (isSPFound == true) {
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				if (F5Option == EVERY_LINE) {
					cursorRange.length = pos - cursorRange.location;
					isSetHighlight = true;
					break;
				} else if (pos+1 < len) {
					ch = [textSt characterAtIndex:pos+1];
					if (ch == '*' || ch == '%' || ch == '@') {
						cursorRange.length = pos + 1 - cursorRange.location;
						isSetHighlight = true;
						break;
					}
				} else if (pos+1 >= len) {
					cursorRange.length = pos - cursorRange.location;
					isSetHighlight = true;
					break;
				}
			} else if (ch == 0x2022 && pos+1 < len) {
				pos++;
				ch = [textSt characterAtIndex:pos];
				if (isdigit(ch)) {
					bufLen = 0L;
					do {
						buf[bufLen++] = (char)ch;
						pos++;
						ch = [textSt characterAtIndex:pos];
					} while ((isdigit(ch) || ch =='_') && pos < len);
					if (ch == 0x2022) {
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							theAVinfo.beg = atol(buf);
						}
					}
				} else
					pos--;
			}
			pos++;
		}
	}
	if (isSetHighlight && cDoc != NULL) {
		if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
			return;
		}
		if (cDoc->filePath[0] != EOS) {
			strcpy(theAVinfo.mediaFPath, cDoc->filePath);
			bufEnd = strrchr(theAVinfo.mediaFPath, '/');
			if (bufEnd != NULL) {
				*(bufEnd+1) = EOS;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
			if (theAVinfo.beg != -1L) {
/* 2020-09-01
			} else if (AVMediaPlayer != nil) {
				strcpy(spareTier1, theAVinfo.mediaFPath);
				addFilename2Path(spareTier1, theAVinfo.mediaFName);
				if (uS.partwcmp(spareTier1, AVMediaPlayer->rMovieFile) == 0)
					theAVinfo.beg = AVMediaPlayer->lastEndBullet;
				else
					theAVinfo.beg = 0L;
*/
			} else
				theAVinfo.beg = 0L;
			theAVinfo.end = -1L;
			if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
				[textView setSelectedRange:cursorRange];
				[textView scrollRangeToVisible:cursorRange];
				theAVinfo.playMode = F_five;
				theAVinfo.textView = textView;
				theAVinfo.docWindow = [self window];
				theAVinfo.nextSegs = nil;
				[AVController createAndPlayAV:&theAVinfo];
				[cDoc showWindows];
			} else
				do_warning_sheet("Can't find correct begin and end bullet time", [self window]);
		} else
			do_warning_sheet("Missing media filename or data filename", [self window]);
	} else if (pos >= len || strcmp(bufU, "@End") == 0) { // 2020-08-31 beg
		keys = [NSString stringWithUTF8String:"*:	\n"];
		charRange.location = pos;
		charRange.length = 0;
		if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
			[text replaceCharactersInRange:charRange withString:keys];
			len = [text length];
			if (pos < len) {
				cursorRange.location = pos;
				cursorRange.length = 4;
				isSetHighlight = true;
			}
			[textView didChangeText];
			[[textView undoManager] setActionName:@"EmptyTier"];
		}
		if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
			return;
		}
		if (cDoc != NULL && cDoc->filePath[0] != EOS) {
			strcpy(theAVinfo.mediaFPath, cDoc->filePath);
			bufEnd = strrchr(theAVinfo.mediaFPath, '/');
			if (bufEnd != NULL) {
				*(bufEnd+1) = EOS;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
			theAVinfo.beg = 0L;
			theAVinfo.end = -1L;
			if (theAVinfo.beg != 0L || theAVinfo.end != 0L) {
				[textView setSelectedRange:cursorRange];
				[textView scrollRangeToVisible:cursorRange];
				theAVinfo.playMode = F_five;
				theAVinfo.textView = textView;
				theAVinfo.docWindow = [self window];
				theAVinfo.nextSegs = nil;
				[AVController createAndPlayAV:&theAVinfo];
				[cDoc showWindows];
			}
		} else
			do_warning_sheet("Can't find correct media filename or data filename", [self window]);
	} else
		do_warning_sheet("Can't find correct media filename or data filename", [self window]);
	// 2020-08-31 end
	isF1_2Key = 0;
} // 2020-08-18 end // 2020-09-01 eng

- (IBAction)CheckOpenedFile:(id)sender { // 2020-08-28 beg // 2020-09-01 beg
#pragma unused (sender)
	NSRange cursorRange;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger j;
	Document *cDoc;
	extern NSRange ced_cursorRange;
	extern void CheckC(NSTextView *textView, char *filePath, BOOL isMedia);

	if (ChatMode == NO) {
		do_warning_sheet("Illegal in TEXT mode.", [self window]);
		return;
	}
	cDoc = [self document];
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	j = [text length];
	[[textView undoManager] removeAllActions];
	ced_err_message[0] = EOS;
	[self getCurrentMediaName:textSt MaxLen:j showErr:false];
	CheckC(textView, cDoc->filePath, cDoc->mediaFileName[0] != EOS);
	if (ced_err_message[0] == '+')
		do_warning_sheet(ced_err_message+1, [textView window]);
	else if (ced_err_message[0] != EOS) {
		cursorRange = [textView selectedRange];
		j = [self countLineNumbers:cursorRange.location];
		[self setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
	}
} // 2020-08-28 end // 2020-09-01 eng

- (IBAction)ExpandBullets:(id)sender { // 2020-08-18 beg // 2020-09-01 beg
#pragma unused (sender)
	NSRange charRange;
	NSTextStorage *text;
	NSTextView *textView;
	Document *cDoc;
//	NSLayoutManager *tLayoutMgr;

//	NSRange theTextRange;
//	NSRect layoutRect;
//	NSPoint containerOrigin;
//	NSPoint cursorPoint;

	if (ShowParags == YES)
		ShowParags = NO;
	else
		ShowParags = YES;
	cDoc = [self document];
	textView = [self firstTextView];
	text = [textView textStorage];
	charRange = NSMakeRange(0, [text length]);

// 2019-11-26 beg
//[[self window] disableCursorRects];
//NSRange cursorRange = [textView selectedRange];

	[text beginEditing];
	[text edited:NSTextStorageEditedAttributes range:charRange changeInLength:0];
	[text endEditing];

//	for (tLayoutMgr in [text layoutManagers]) {


//								dispatch_async(dispatch_get_main_queue(), ^{
//								NSRange charRangeOut;

// 2020-05-28
//charRange.location = cursorRange.location;
//charRange.length = [text length] - charRange.location;
//[tLayoutMgr processEditingForTextStorage:text edited:NSTextStorageEditedCharacters range:charRange changeInLength:0 invalidatedRange:charRange];
//charRange.location = 0;
//charRange.length = cursorRange.location;
//[tLayoutMgr processEditingForTextStorage:text edited:NSTextStorageEditedCharacters range:charRange changeInLength:0 invalidatedRange:charRange];

//cursorRange.length = cursorRange.location;
//cursorRange.location = 0;
//charRange.location = 0;
//charRange.length = cursorRange.location;
//cursorRange.length = 0;
//cursorRange.location = 0;
//[text beginEditing];
//[tLayoutMgr processEditingForTextStorage:text edited:NSTextStorageEditedAttributes|NSTextStorageEditedCharacters range:charRange changeInLength:0 invalidatedRange:charRange];
//[text endEditing];
//[tLayoutMgr ensureLayoutForCharacterRange:charRange];
//[tLayoutMgr invalidateDisplayForGlyphRange:charRange];
//[tLayoutMgr invalidateDisplayForCharacterRange:charRange];


//								[tLayoutMgr invalidateGlyphsForCharacterRange:charRange changeInLength:0 actualCharacterRange:&charRangeOut];
//								[tLayoutMgr invalidateLayoutForCharacterRange:charRange actualCharacterRange:&charRangeOut];
//								[tLayoutMgr invalidateLayoutForCharacterRange:charRange isSoft:YES actualCharacterRange:&charRangeOut];
//								[tLayoutMgr invalidateDisplayForGlyphRange:charRange];


//								});

//	}
//[textView didChangeText];
//[textView display];
//[textView setSelectedRange:cursorRange];
//[textView scrollRangeToVisible:cursorRange];
//dispatch_async(dispatch_get_main_queue(), ^{
//	[textView scrollRangeToVisible:cursorRange];
//});
//[textView setNeedsDisplay:YES];
//[tLayoutMgr ensureLayoutForCharacterRange:charRange];
//[tLayoutMgr ensureLayoutForCharacterRange:charRangeOut];
/*
 NSArray *textContainers = [tLayoutMgr textContainers];
 for (NSTextContainer *container in textContainers) {
 	[tLayoutMgr ensureLayoutForTextContainer:container];
 }
 */

/*
 theTextRange = [tLayoutMgr glyphRangeForCharacterRange:cursorRange actualCharacterRange:NULL];
 layoutRect = [tLayoutMgr boundingRectForGlyphRange:theTextRange inTextContainer:[textView textContainer]];
 containerOrigin = [textView textContainerOrigin];
 cursorPoint.x = layoutRect.origin.x + containerOrigin.x;
 cursorPoint.y = layoutRect.origin.y + containerOrigin.y - 50;
 [textView scrollPoint:cursorPoint];
 */
//BOOL res = [textView scrollRectToVisible:layoutRect];

//dispatch_async(dispatch_get_main_queue(), ^{
//[[self window] invalidateCursorRectsForView:textView];
//});

//[[self window] enableCursorRects];

// 2019-11-26 end
//[textView setLayoutOrientation:NSTextLayoutOrientationHorizontal];
//[textView setNeedsLayout];

//[textView setNeedsDisplay:YES];
//[textView display];
/*
 							dispatch_async(dispatch_get_main_queue(), ^{
 //								NSValue *cursorValue = [[textView selectedRanges] objectAtIndex:0];
 //								cursorRange = [cursorValue rangeValue];

								 cursorRange = [textView selectedRange];
								 [textView scrollRangeToVisible:cursorRange]; // 2020-05-13
							 });
 */

//[[self window] makeKeyAndOrderFront:self];
//[self.window setOrderedIndex:0];
//[[self window] invalidateCursorRectsForView:textView];
//[[self window] enableCursorRects];
//[NSApp activateIgnoringOtherApps:YES];
//[[self window] setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces|NSWindowCollectionBehaviorTransient];
//[[self window] setLevel:NSPopUpMenuWindowLevel];

} // 2020-08-18 end // 2020-09-01 eng

- (IBAction)DefineIDs:(id)sender { // 2020-09-04 beg
#pragma unused (sender)
	char *err;
	unsigned short wID;
	Document *cDoc;
	NSTextView *textView;

	NSLog(@"DocumentWinController: DefineIDs\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		textView = [self firstTextView];
		err = [IdsController idsDialog:self];
		if (err != NULL) {
			if (err[0] == '+' || err[0] == '-')
				do_warning_sheet(err+1, [self window]);
			else
				do_warning_sheet(err, [self window]);
		}
	} else {
		do_warning_sheet("\"ID headers\" menu only works with CHAT text", [self window]);
	}
} // 2020-09-04 end


- (void)SetUpSpeakers:(unCH *)line { // 2020-09-10 beg
	int i = 0;
	unCH *s, *e, t, wc;
	short cnt = 0;

	for (; *line && *line != ':'; line++) ;
	if (*line == EOS)
		return;
	for (line++; *line && isSpace(*line); line++) ;
	s = line;
	while (*s) {
		if (*line == ',' || isSpace(*line) || *line == '\n' || *line == EOS) {
			wc = ' ';
			e = line;
			for (; isSpace(*line) || *line == NL_C || *line == '\n'; line++) ;
			if (*line != ',' && *line != EOS) line--;
			else wc = ',';
			if (s != e) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || (wc == ',' && cnt != 0)) {
				} else if (cnt == 0) {
					strcpy(ced_line, "*");
					strcat(ced_line, s);
					strcat(ced_line, ":\t");
					*e = t;
					[self AllocSpeakerNames:ced_line index:i];
					i++;
					if (i >= SPEAKERNAMENUM)
						return;
				}
				*e = t;
				if (wc == ',') {
					cnt = -1;
				}
				if (*line) {
					for (line++; isSpace(*line) || *line=='\n' || *line==','; line++) {
						if (*line == ',') {
							cnt = -1;
						}
					}
				}
			} else {
				for (line=e; *line; line++) ;
			}
			cnt++;
			s = line;
		} else line++;
	}
} // 2020-09-10 end

- (void)SetUpParticipants:(BOOL)isShowErr { // 2020-09-10 beg
	BOOL found = false;
	NSUInteger pos, len;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;

	NSLog(@"DocumentWinController: SetUpParticipants\n");

	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	pos = 0;
/*
	while (1) {
		pos = getUtts(pos, len, textSt);
		uS.remblanks(ced_line);
		if (!isSpeaker(sp[0]))
			break;
		if (isMainSpeaker(sp[0]))
			break;
		if (uS.partcmp(sp, DEPENDENT, FALSE, FALSE)) {
			found = true;
			ced_SetUpDependents(templine)
			return;
		}
	}
*/
	while (1) {
		pos = getUtts(pos, len, textSt);
		uS.remblanks(ced_line);
		if (!isSpeaker(sp[0]))
			break;
		if (isMainSpeaker(sp[0]))
			break;
		if (uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE)) {
			found = true;
			[self SetUpSpeakers:templine];
		}
	}
	if (isShowErr && !found) {
		do_warning_sheet("Can't locate '@Participants:' tier", [self window]);
	}
} // 2020-09-10 beg

- (void)AllocSpeakerNames:(unCH *)st index:(int)i { // 2020-09-10 beg
	NSLog(@"DocumentWinController: AllocSpeakerNames\n");

	if (*st != EOS) {
		strncpy(SpeakerNames[i], st, SPEAKERNAMELEN);
		SpeakerNames[i][SPEAKERNAMELEN] = EOS;
	} else if (SpeakerNames[i][0] != EOS) {
		SpeakerNames[i][0] = EOS;
	}
} // 2020-09-10 end

- (IBAction)UpdateTiersSpeakers:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	int i;
	unsigned short wID;
	Document *cDoc;
	
	NSLog(@"DocumentWinController: UpdateTiersSpeakers\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		for (i=0; i < SPEAKERNAMENUM; i++)
			[self AllocSpeakerNames:cl_T("") index:i];
		[self SetUpParticipants:true];
	} else {
		do_warning_sheet("\"Update\" menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)UnderlineText:(id)sender { // 2021-07-15 beg
#pragma unused (sender)
	unsigned short wID;
	Document *cDoc;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;
	id isUnderlined;
	NSInteger val;

	NSLog(@"DocumentWinController: UnderlineText\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		textView = [self firstTextView];
		text = [textView textStorage];
		cursorRange = [[[textView selectedRanges] objectAtIndex:0] rangeValue];
		if (cursorRange.length == 0) {
			do_warning_sheet("Please highlight some word first", [self window]);
		} else {
			[text beginEditing];
			isUnderlined = [text attribute:NSUnderlineStyleAttributeName atIndex:cursorRange.location
							effectiveRange:nil];
			val = [isUnderlined integerValue];
			if (val != 0) {
				[text addAttribute:NSUnderlineStyleAttributeName value:@(NSUnderlineStyleNone)
							 range:cursorRange];
			} else {
				[text addAttribute:NSUnderlineStyleAttributeName value:@(NSUnderlineStyleSingle)
							 range:cursorRange];
			}
			[text endEditing];
		}
	} else {
		do_warning_sheet("\"Underline\" menu only works with CHAT text", [self window]);
	}
} // 2021-07-15 end

- (NSRange)findRightPos:(NSTextStorage *)text view:(NSTextView *)textView {
	unichar ch;
	NSUInteger pos, tPos, len;
	NSString *keys;
	NSString *textSt;
	NSRange cursorRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	pos = cursorRange.location;
	if (pos == len) {
		cursorRange.location = pos;
		cursorRange.length = 0;
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
		return(cursorRange);
	} else {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n' && pos > 0)
			pos--;
	}
	while (true) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n' || pos == 0) {
			if (ch == '\n') {
				pos++;
				ch = [textSt characterAtIndex:pos];
			}
			if (ch == '*') {
				tPos = pos;
				while (tPos < len) {
					ch = [textSt characterAtIndex:tPos];
					tPos++;
					if (ch == ':' || ch == '\n') {
						break;
					}
				}
				if (ch == ':') {
					tPos++;
					while (tPos < len) {
						ch = [textSt characterAtIndex:tPos];
						tPos++;
						if (!isSpace(ch)) {
							cursorRange.location = pos;
							cursorRange.length = tPos-pos-1;
							keys = [NSString stringWithUTF8String:""];
							if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
								[text replaceCharactersInRange:cursorRange withString:keys];
								[textView didChangeText];
								[[textView undoManager] setActionName:@"SpCode"];
#ifdef DEBUGCODE
if (cursorRange.location+40 >= [text length]) dl = [text length]-cursorRange.location; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(cursorRange.location, dl)]; bufU[dl] = EOS; // lxs debug
#endif
								cursorRange.length = 0;
								[textView setSelectedRange:cursorRange];
								[textView scrollRangeToVisible:cursorRange];
							} else
								cursorRange.length = 0;
							[textView setSelectedRange:cursorRange];
							[textView scrollRangeToVisible:cursorRange];
							return(cursorRange);
						}
					}
				} else {
					cursorRange.location = pos;
					cursorRange.length = 0;
				}
			} else {
				cursorRange.location = pos;
				cursorRange.length = 0;
			}
			break;
		}
		pos--;
	}
	[textView setSelectedRange:cursorRange];
	[textView scrollRangeToVisible:cursorRange];
	return(cursorRange);
}

- (IBAction)InsertTiersSpOne:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpOne\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[0]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpTwo:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpTwo\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[1]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpThree:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpThree\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[2]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpFour:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpFour\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[3]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpFive:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpFive\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[4]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpSix:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpSix\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[5]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpSeven:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpSeven\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[6]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpEight:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpEight\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[7]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpNine:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpNine\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[8]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)InsertTiersSpTen:(id)sender { // 2020-09-10 beg
#pragma unused (sender)
	unichar bufU[BUFSIZ];
	unsigned short wID;
	Document *cDoc;
	NSString *keys;
	NSTextStorage *text;
	NSTextView *textView;
	NSRange cursorRange;

	NSLog(@"DocumentWinController: InsertTiersSpTen\n");
	cDoc = [self document];
	wID = [cDoc get_wID];
	if (wID == DOCWIN) {
		strcpy(bufU, SpeakerNames[9]);
		if (bufU[0] != EOS) {
			textView = [self firstTextView];
			text = [textView textStorage];
			cursorRange = [self findRightPos:text view:textView];
			keys = [NSString stringWithCharacters:bufU length:strlen(bufU)];
			if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
				[text replaceCharactersInRange:cursorRange withString:keys];
				[textView didChangeText];
				[[textView undoManager] setActionName:@"SpCode"];
			}
		} else {
			do_warning_sheet("No speaker code specified.\nPlease, Tiers->Update if necessary.", [self window]);
		}
	} else {
		do_warning_sheet("Insert Speaker codes menu only works with CHAT text", [self window]);
	}
} // 2020-09-10 end

- (IBAction)showHideLineNumbers:(id)sender // 2020-09-18 beg
{
#pragma unused (sender)
	if (!isChatLineNums) {
		if (self.lineNumberView == nil) {
			self.lineNumberView = [[NoodleLineNumberView alloc] initWithScrollView:scrollView];
			[scrollView setVerticalRulerView:self.lineNumberView];
			[scrollView setHasVerticalRuler:YES];
			[scrollView setHasHorizontalRuler:NO];
		}
		[scrollView setRulersVisible:YES];
		isChatLineNums = TRUE;
	} else {
		[scrollView setRulersVisible:NO];
		isChatLineNums = FALSE;
	}
	WriteCedPreference();
} // 2020-09-18 end

- (IBAction)EditMode:(id)sender // 2020-09-24 beg
{
#pragma unused (sender)
	NSTextView *textView;
	NSRange cursorRange;

	if (SnTr.IsSoundOn == true) {
		do_warning_sheet("Illegal in sonic mode", [self window]);
		return;
	}
	isDisambiguateAgain = false;
	textView = [self firstTextView];
	cursorRange = [textView selectedRange];
	if (EditorMode) {
		EditorMode = false;
		[lowerView setRowsCnt:6];
		if (NoCodes) {
			if (!GetNewCodes(-1, self)) {
				do_warning_sheet("NO CODES DEFINED", [self window]);
			}
			if (NoCodes)
				return;
		}
	} else {
		ToTopLevel(-1, self);
		EditorMode = true;
		[lowerView setRowsCnt:2];
	}
	[self setupWindowForDocument];
	dispatch_async(dispatch_get_main_queue(), ^{
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
	});
	if (!EditorMode) {
		EndCodeArr = DisplayCodes(self);
//		DisplayCursor(COM_WIN_SIZE);
	}
	EndCodeArr = DisplayCodes(self);

} // 2020-09-24 end

- (IBAction)DisambiguateMorTier:(id)sender // 2020-10-09 beg
{
#pragma unused (sender)
	MorDisambiguate(self);
} // 2020-10-09 end

- (IBAction)SetNextTierName:(id)sender
{
	unCH tNextTierName[BUFSIZ];

#pragma unused (sender)
	strcpy(tNextTierName, NextTierName);
	[UserStrController userStrDialog:self label:@"New Speaker code:" str:tNextTierName max:512];
	uS.remFrontAndBackBlanks(tNextTierName);
	uS.uppercasestr(tNextTierName, &dFnt, C_MBF);
	if (tNextTierName[0] != EOS) {
		if (!isSpeaker(tNextTierName[0]))
			strcpy(NextTierName, "*");
		else
			NextTierName[0] = EOS;
		strcat(NextTierName, tNextTierName);
	}
}

-(BOOL)convertACCtoWAV:(NSURL*)url outputURL:(NSURL*)outputURL
{
	OSStatus finalError = noErr, error;
	ExtAudioFileRef sourceFile = nil;
	ExtAudioFileRef destinationFile = nil;
	UInt32 thePropertySize, numFrames;
	AudioBufferList fillBufList;
	const UInt32 bufferByteSize = 32768;
	UInt8 srcBuffer[bufferByteSize];// = [UInt8](repeating: 0, count: 32768)

	AudioStreamBasicDescription srcFormat;
	AudioStreamBasicDescription dstFormat;

	ExtAudioFileOpenURL((__bridge CFURLRef)url, &sourceFile);

	thePropertySize = sizeof(srcFormat); //UInt32(MemoryLayout.stride(ofValue: srcFormat));;
	ExtAudioFileGetProperty(sourceFile, kExtAudioFileProperty_FileDataFormat, &thePropertySize, &srcFormat);

	dstFormat.mSampleRate = srcFormat.mSampleRate;  //Set sample rate
	dstFormat.mFormatID = kAudioFormatLinearPCM;
	dstFormat.mChannelsPerFrame = 1;
	dstFormat.mBitsPerChannel = 16;
	dstFormat.mBytesPerPacket = 2 * dstFormat.mChannelsPerFrame;
	dstFormat.mBytesPerFrame = 2 * dstFormat.mChannelsPerFrame;
	dstFormat.mFramesPerPacket = 1;
	dstFormat.mFormatFlags = kLinearPCMFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;

	// Create destination file
	error = ExtAudioFileCreateWithURL(
									  (__bridge CFURLRef)outputURL,
									  kAudioFileWAVEType,
									  &dstFormat,
									  nil,
									  kAudioFileFlags_EraseFile,
									  &destinationFile);
//	NSLog(@"Error 1 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
	if (error != noErr) {
		error = ExtAudioFileDispose(destinationFile);
//		NSLog(@"Error 6.1 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		error = ExtAudioFileDispose(sourceFile);
//		NSLog(@"Error 7.1 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		return(false);
	}
	error = ExtAudioFileSetProperty(sourceFile,
									kExtAudioFileProperty_ClientDataFormat,
									thePropertySize,
									&dstFormat);
//	NSLog(@"Error 2 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
	if (error != noErr) {
		error = ExtAudioFileDispose(destinationFile);
//		NSLog(@"Error 6.2 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		error = ExtAudioFileDispose(sourceFile);
//		NSLog(@"Error 7.2 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		return(false);
	}
	error = ExtAudioFileSetProperty(destinationFile,
									kExtAudioFileProperty_ClientDataFormat,
									thePropertySize,
									&dstFormat);
//	NSLog(@"Error 3 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
	if (error != noErr) {
		error = ExtAudioFileDispose(destinationFile);
//		NSLog(@"Error 6.3 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		error = ExtAudioFileDispose(sourceFile);
//		NSLog(@"Error 7.3 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		return(false);
	}

	memset(srcBuffer, 0, bufferByteSize);
	unsigned long sourceFrameOffset = 0;

	while (true) {
		fillBufList.mNumberBuffers = 1;
		fillBufList.mBuffers[0].mNumberChannels = 2;
		fillBufList.mBuffers[0].mDataByteSize = bufferByteSize;
		fillBufList.mBuffers[0].mData = &srcBuffer;
		numFrames = 0;
		if(dstFormat.mBytesPerFrame > 0){
			numFrames = bufferByteSize / dstFormat.mBytesPerFrame;
		}
		error = ExtAudioFileRead(sourceFile, &numFrames, &fillBufList);
//		NSLog(@"Error 4 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		if (error != noErr)
			break;
		if (numFrames == 0) {
			error = noErr;
			break;
		}
		sourceFrameOffset += numFrames;
		error = ExtAudioFileWrite(destinationFile, numFrames, &fillBufList);
//		NSLog(@"Error 5 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
		if (error != noErr)
			break;
	}

	if (error != noErr)
		finalError = error;
	error = ExtAudioFileDispose(destinationFile);
//	NSLog(@"Error 6 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
	if (error != noErr)
		finalError = error;
	error = ExtAudioFileDispose(sourceFile);
//	NSLog(@"Error 7 in convertAudio: %d - %@", error, [NSError errorWithDomain:NSOSStatusErrorDomain code:error userInfo:nil].description);
	if (error != noErr)
		finalError = error;
	if (finalError == noErr)
		return(true);
	else
		return(false);
}

- (void)sendToSoundAnalyzer { // 2020-12-03 beg
	double dBeg, dEnd;
	const char *err = NULL;
	extern const char *sendpraat (void *display, const char *programName, long timeOut, const char *text);

	dBeg = theAVinfo.beg;
	dEnd = theAVinfo.end;
	dBeg /= 1000;
	dEnd /= 1000;
	sprintf(templineC, "Open long sound file... %s\n", theAVinfo.mediaFPath);
//	.mov		sprintf(templineC, "Read from file... %s\n", fname);
	sprintf(templineC+strlen(templineC), "Rename... temp\n");
	sprintf(templineC+strlen(templineC), "Extract part... %lf %lf yes\n", dBeg, dEnd);
//	.mov		sprintf(templineC+strlen(templineC), "Extract part... %lf %lf Rectangular 1.0 yes\n", dBeg, dEnd);
	sprintf(templineC+strlen(templineC), "Rename... %s\n", templineC2);
	sprintf(templineC+strlen(templineC), "select LongSound temp\n");
	sprintf(templineC+strlen(templineC), "Remove");
	err = sendpraat((void *)NULL, "Praat", 0L, templineC);
	if (err != NULL) {
		if (*err == '-')
			do_warning_sheet(err+1, [self window]);
		else
			do_warning_sheet(err, [self window]);
	}
}// 2020-12-03 end

- (void)updateExportDisplay {
	if (isExportDone == true) {
		[exportProgressBarTimer invalidate];
	} else {
		if (exportSession.progress > .99) {
			[exportProgressBarTimer invalidate];
		} else
			[progInd setDoubleValue:(double)exportSession.progress];
	}
}

- (void)exportToWave:(BOOL)isSend { // 2020-12-02 beg
	FNType	*ext;
	NSWindow *win = [self window];
	NSURL *inURL, *outURL;
	AVURLAsset* asset;
	extern FNType noMorFName[];

	strcpy(noMorFName, theAVinfo.mediaFPath);
	strcpy(FileName1, theAVinfo.mediaFPath);
	ext = strrchr(FileName1, '.');
	if (ext != NULL)
		uS.str2FNType(ext, 0L, ".m4a");
	else
		uS.str2FNType(FileName1, strlen(FileName1), ".m4a");
	strcpy(FileName2, FileName1);
	ext = strrchr(FileName2, '.');
	if (ext != NULL)
		uS.str2FNType(ext, 0L, ".wav");
	if (!access(FileName2, 0)) {
		if (isSend == true) {
			strcpy(theAVinfo.mediaFPath, FileName2);
			[self sendToSoundAnalyzer];
		} else {
			strcpy(theAVinfo.mediaFPath, FileName2);
			strcpy(theAVinfo.mediaFName, noMorFName);
			[self soundwindow];
		}
	} else {

		FNType *FNameOffset;
		NSRect wFrame = [[self window] frame];
		wFrame.size.width = wFrame.size.width / 5 * 4;
		wFrame.size.height =  60;

		progSheet = [[NSWindow alloc] initWithContentRect:wFrame
												styleMask:NSTitledWindowMask
												  backing:NSBackingStoreBuffered
													defer:YES];
		contentView = [[NSView alloc] initWithFrame:wFrame];

		progInd = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(wFrame.origin.x, wFrame.size.height-50, wFrame.size.width, 20)];
		[progInd setIndeterminate:NO];
		[progInd setDoubleValue:0.f];
		[progInd startAnimation:self];
		progInd.minValue = 0.0;
		progInd.maxValue = 1.0;

		label = [[NSTextField alloc] initWithFrame:NSMakeRect(wFrame.origin.x, wFrame.size.height-30, wFrame.size.width, 22)];
		FNameOffset = strrchr(noMorFName, '/');
		if (FNameOffset != NULL)
			FNameOffset++;
		else
			FNameOffset = noMorFName;
		sprintf(templineC4, "Exporting audio from file: %s", FNameOffset);
		[label setStringValue:[NSString stringWithUTF8String:templineC4]];
		[label setBezeled:NO];
		[label setDrawsBackground:NO];
		[label setEditable:NO];
		[label setSelectable:NO];

		[contentView addSubview:progInd];
		[contentView addSubview:label];
		[progSheet setContentView:contentView];

//		[NSApp beginSheet:progSheet modalForWindow:self.window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
		[win beginSheet:progSheet completionHandler:nil];
		[progSheet makeKeyAndOrderFront:self];

		inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:theAVinfo.mediaFPath]];
		asset = [AVURLAsset URLAssetWithURL:inURL options:nil];
		exportSession = [[AVAssetExportSession alloc] initWithAsset:asset presetName:AVAssetExportPresetAppleM4A]; //AVAssetExportPresetPassthrough];

//		NSArray<AVFileType> *supportedFileTypes = exportSession.supportedFileTypes;
//		NSArray<NSString *> *allExportPresets = [AVAssetExportSession exportPresetsCompatibleWithAsset:asset];

		outURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:FileName1]];
		exportSession.outputURL = outURL;
		exportSession.outputFileType = AVFileTypeAppleM4A; // AVFileTypeWAVE;
		CMTime duration = [asset duration];
		exportSession.timeRange = CMTimeRangeMake(kCMTimeZero, duration);
		[[NSFileManager defaultManager] removeItemAtURL:outURL error:nil];
		isExportDone = false;
		exportProgressBarTimer = [NSTimer scheduledTimerWithTimeInterval:.1 target:self selector:@selector(updateExportDisplay) userInfo:nil repeats:YES];
		[exportSession exportAsynchronouslyWithCompletionHandler:^(void) {
			BOOL res;
			NSString *error;
			NSURL *inURL, *outURL;

			switch (exportSession.status) {
				case AVAssetExportSessionStatusCompleted:
				{
					NSLog(@"Completed exporting!");
					inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:FileName1]];
					outURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:FileName2]];
					res = [self convertACCtoWAV:inURL outputURL:outURL];

					dispatch_async(dispatch_get_main_queue(), ^{
						isExportDone = true;
						[progInd setIndeterminate:YES];
						[NSApp endSheet:progSheet];
						[progSheet orderOut:self];

						[progSheet release];
						[contentView release];
						[progInd release];
						[label release];
						progSheet = nil;
						contentView = nil;
						progInd = nil;
						label = nil;
					});

					if (res == true) {
						[exportSession release];
						[[NSFileManager defaultManager] removeItemAtURL:inURL error:nil];
						if (isSend == true) {
							strcpy(theAVinfo.mediaFPath, FileName2);
							[self sendToSoundAnalyzer];
						} else {
							strcpy(theAVinfo.mediaFPath, FileName2);
							strcpy(theAVinfo.mediaFName, noMorFName);
							dispatch_async(dispatch_get_main_queue(), ^{
								[self soundwindow];
							});
						}
					}
					break;
				}
				case AVAssetExportSessionStatusFailed:
				case AVAssetExportSessionStatusUnknown:
				case AVAssetExportSessionStatusWaiting:
				case AVAssetExportSessionStatusCancelled:
				{
					dispatch_async(dispatch_get_main_queue(), ^{
						isExportDone = true;
						[progInd setIndeterminate:YES];
						[NSApp endSheet:progSheet];
						[progSheet orderOut:self];
						[progSheet release];
						[contentView release];
						[progInd release];
						[label release];
						progSheet = nil;
						contentView = nil;
						progInd = nil;
						label = nil;
					});

					error = exportSession.error.description;
					[exportSession release];
//					NSLog(@"FAILURE: %@\n", error);
					do_warning_sheet("Failed to extract audio WAV from video media", [self window]);
					break;
				}
			};
		}];
	}
} // 2020-12-02 end

- (void)findMediaFile:(BOOL)isSend { // 2020-12-02 beg
	int pathLen, nameLen;
	char *rFileName;
	BOOL isFirstTimeAround;

	isFirstTimeAround = true;
	rFileName = theAVinfo.mediaFPath;
	pathLen = strlen(rFileName);
tryAgain:
	strcat(rFileName, theAVinfo.mediaFName);
	nameLen = strlen(rFileName);
	strcat(rFileName, ".mov");
	if (access(rFileName, 0)) {
		rFileName[nameLen] = EOS;
		uS.str2FNType(rFileName, strlen(rFileName), ".mp4");
		if (access(rFileName, 0)) {
			rFileName[nameLen] = EOS;
			uS.str2FNType(rFileName, strlen(rFileName), ".wav");
			if (access(rFileName, 0)) {
				rFileName[nameLen] = EOS;
				uS.str2FNType(rFileName, strlen(rFileName), ".aif");
				if (access(rFileName, 0)) {
					rFileName[nameLen] = EOS;
					uS.str2FNType(rFileName, strlen(rFileName), ".mp3");
					if (access(rFileName, 0)) {
						rFileName[nameLen] = EOS;
						uS.str2FNType(rFileName, strlen(rFileName), ".m4v");
						if (access(rFileName, 0)) {
							rFileName[nameLen] = EOS;
							uS.str2FNType(rFileName, strlen(rFileName), ".avi");
							if (access(rFileName, 0)) {
								rFileName[nameLen] = EOS;
								uS.str2FNType(rFileName, strlen(rFileName), ".wmv");
								if (access(rFileName, 0)) {
									rFileName[nameLen] = EOS;
									uS.str2FNType(rFileName, strlen(rFileName), ".mpg");
									if (access(rFileName, 0)) {
										rFileName[nameLen] = EOS;
										uS.str2FNType(rFileName, strlen(rFileName), ".aiff");
										if (access(rFileName, 0)) {
											rFileName[nameLen] = EOS;
											uS.str2FNType(rFileName, strlen(rFileName), ".dv");
											if (access(rFileName, 0)) {
												if (isFirstTimeAround) {
													isFirstTimeAround = false;
													rFileName[pathLen] = EOS;
													strcat(rFileName, "media/");
													goto tryAgain;
												}
												rFileName[nameLen] = EOS;
												do_warning_sheet("Can't locate media file or recognize media format", [self window]);
											} else {
												[self exportToWave:isSend];
											}
										} else {
											if (isSend == true)
												[self sendToSoundAnalyzer];
											else
												[self soundwindow];
										}
									} else {
										[self exportToWave:isSend];
									}
								} else {
									[self exportToWave:isSend];
								}
							} else {
								[self exportToWave:isSend];
							}
						} else {
							[self exportToWave:isSend];
						}
					} else {
						if (isSend == true)
							[self sendToSoundAnalyzer];
						else
							[self soundwindow];
					}
				} else {
					if (isSend == true)
						[self sendToSoundAnalyzer];
					else
						[self soundwindow];
				}
			} else {
				if (isSend == true)
					[self sendToSoundAnalyzer];
				else
					[self soundwindow];
			}
		} else {
			[self exportToWave:isSend];
		}
	} else {
		[self exportToWave:isSend];
	}
} // 2020-12-02 beg

static void remAllBlanks(unCH *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == '_'; i++) ;
	if (i > 0)
		strcpy(st, st+i);
	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '_' || st[i] == NL_C || st[i] == SNL_C)) i--;
	st[i+1] = EOS;
}

- (void)getTierText:(NSString *)textSt Index:(NSUInteger)pos MaxLen:(NSUInteger)len MaxSize:(int)max {
	int  j;
	unichar ch;
	char hf, spf;

	ch = 0;
	while (pos > 0) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
				if (ch == '*' || ch == '%' || ch == '@') {
					pos++;
					break;
				}
			}
		}
		pos--;
	}
	j = 0;
	if (ch == '*' || ch == '%' || ch == '@') {
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
			if (ch == ':') {
				pos++;
				break;
			} else if (ch == '\n') {
				break;
			}
			pos++;
		}
		hf = FALSE;
		spf = FALSE;
		while (j < max-2 && pos < len) {
			ch = [textSt characterAtIndex:pos];
			if (ch == '*' || ch == '%' || ch == '@')
				break;
			if (ch == 0x2022)
				hf = !hf;
			else if (!hf) {
				if (isSpace(ch) || ch == '\n' || ch == '<' || ch == '>') {
					if (!spf)
						templine2[j++] = '_';
					spf = TRUE;
				} else {
					templine2[j++] = ch;
					spf = FALSE;
				}
			}
			pos++;
		}
	}
	templine2[j] = EOS;
	remAllBlanks(templine2);
	UnicodeToUTF8(templine2, j, (unsigned char *)templineC2, NULL, BUFSIZ);
}

-(BOOL)isFindBullet:(NSString *)textSt Index:(NSUInteger *)cPos MaxLen:(NSUInteger)len AVInformation:(struct AVInfo *)AVinfo {
	char buf[BUFSIZ], *bufEnd;
	unichar ch;
	NSUInteger pos, bufLen;
	BOOL isFoundAV;

	pos = *cPos;
	ch = [textSt characterAtIndex:pos];
	if (ch == '\n') {
		pos--;
	}
	isFoundAV = false;
	AVinfo->beg = -1L;
	AVinfo->end = -1L;

	while (pos > 0) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			break;
		} else if (ch == 0x2022) {
			pos--;
			ch = [textSt characterAtIndex:pos];
			if (isdigit(ch)) {
				do {
					pos--;
					ch = [textSt characterAtIndex:pos];
				} while ((isdigit(ch) || ch== '_') && pos > 0);
				if (ch == '"' && pos > 0) {
					do {
						pos--;
						ch = [textSt characterAtIndex:pos];
					} while (ch != 0x2022 && pos > 0);
				}
				if (ch == 0x2022) {
					bufLen = 0L;
					pos++;
					pos = [self isOldBullet:theAVinfo.mediaFName txt:textSt tpos:pos tlen:len];
					ch = [textSt characterAtIndex:pos];
					while (ch != 0x2022 && pos < len) {
						buf[bufLen++] = (char)ch;
						pos++;
						ch = [textSt characterAtIndex:pos];
					}
					buf[bufLen] = EOS;
					bufEnd = strchr(buf, '_');
					if (buf[0] != EOS && bufEnd != NULL) {
						AVinfo->beg = atol(buf);
						AVinfo->end = atol(bufEnd+1);
						isFoundAV = true;
						break;
					}
				}
			}
		}
		if (pos == 0)
			break;
		pos--;
	}
	if (isFoundAV == false) {
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				break;
			} else if (ch == 0x2022 && pos+1 < len) {
				pos++;
				pos = [self isOldBullet:AVinfo->mediaFName txt:textSt tpos:pos tlen:len];
				ch = [textSt characterAtIndex:pos];
				if (isdigit(ch)) {
					bufLen = 0L;
					do {
						buf[bufLen++] = (char)ch;
						pos++;
						ch = [textSt characterAtIndex:pos];
					} while ((isdigit(ch) || ch =='_') && pos < len);
					if (ch == 0x2022) {
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							AVinfo->beg = atol(buf);
							AVinfo->end = atol(bufEnd+1);
							isFoundAV = true;
							break;
						}
					}
				} else
					pos--;
			}
			pos++;
		}
	}
	*cPos = pos;
	return(isFoundAV);
}

- (IBAction)getTextAndSendToSoundAnalyzer:(id)sender { // 2020-12-02 beg
#pragma unused (sender)
	char *bufEnd;
	BOOL isFoundAV;
	NSRange cursorRange;
	unichar ch;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger pos, len;
	Document *cDoc;

	cDoc = [self document];
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];

	pos = cursorRange.location;
	if (pos >= len)
		pos = len - 1;
	ch = [textSt characterAtIndex:pos];
	if (ch == '\n') {
		pos--;
	}
	theAVinfo.mediaFName[0] = EOS;
	isFoundAV = [self isFindBullet:textSt Index:&pos MaxLen:len AVInformation:&theAVinfo];
	if (isFoundAV && cDoc != NULL) {
		if (theAVinfo.mediaFName[0] == EOS) {
			if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
				return;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
		}
		if (cDoc->filePath[0] != EOS) {
			[self getTierText:textSt Index:pos MaxLen:len MaxSize:40];
			strcpy(theAVinfo.mediaFPath, cDoc->filePath);
			bufEnd = strrchr(theAVinfo.mediaFPath, '/');
			if (bufEnd != NULL) {
				*(bufEnd+1) = EOS;
			}
			[self findMediaFile:true];
		} else
			do_warning_sheet("Can't determine document's file path", [self window]);
	} else
		do_warning_sheet("Media marker not found at text cursor position. Place text cursor right after the bullet.", [self window]);

	isF1_2Key = 0;
} // 2020-12-02 end

- (IBAction)SelectF5Option:(id)sender { // 2020-12-09 beg
#pragma unused (sender)
	[SelectF5Controller SelectF5Dialog:[self window]];
} // // 2020-12-09 end

- (IBAction)ChatModeSet:(id)sender { // 2020-12-20 beg
#pragma unused (sender)
	char tmes[2];
	NSTextView *textView;
	NSValue *cursorValue;
	NSRange cursorRange;
	NSUInteger cursorPos, cursorLine;

	if (ChatMode == YES) {
		ChatMode = NO;
		do_warning_sheet("TEXT mode is on", [self window]);
	} else {
		ChatMode = YES;
		do_warning_sheet("CHAT mode is on", [self window]);
	}
	textView = [self firstTextView];
	cursorValue = [[textView selectedRanges] objectAtIndex:0];
	cursorRange = [cursorValue rangeValue];
	cursorPos = cursorRange.location;
	cursorLine = [self countLineNumbers:cursorPos];
	tmes[0] = EOS;
	[self setStatusLine:cursorLine extraMess:tmes isUpdateDisplay:YES];
} // 2020-12-20 end

- (IBAction)TrimTiers:(id)sender { // 2022-06-27 beg
#pragma unused (sender)
	char tmes[2];
	BOOL isSPos;
	unichar ch;

//	NSTextStorage *text;
	NSString *textStNew;
	NSTextView *textView;

	NSRange posRange;
	NSValue *cursorValue;
	NSUInteger len, pos, sPos, cursorPos, cursorLine;
	Document *cDoc = [self document];

	textView = [self firstTextView];

	[cDoc toggleReadOnly:cDoc];
	if (TrimTierMode == YES) {
		TrimTierMode = NO;
		do_warning_sheet("Editor mode is on", [self window]);
		[textView setString:textStOrig];
		textStOrig = NULL;
		[textView setSelectedRange:cursorRangeOrig];
		[textView scrollRangeToVisible:cursorRangeOrig];
		cursorPos = cursorRangeOrig.location;
		cursorLine = [self countLineNumbers:cursorPos];
		tmes[0] = EOS;
		[self setStatusLine:cursorLine extraMess:tmes isUpdateDisplay:YES];
		textView.needsDisplay = YES;
		[textView display];
		[cDoc showWindows];
		return;
	} else {
		TrimTierMode = YES;
		do_warning_sheet("Read only mode is on", [self window]);
		textStNew = [textView string];
		textStOrig = [textStNew mutableCopy];
		cursorValue = [[textView selectedRanges] objectAtIndex:0];
		cursorRangeOrig = [cursorValue rangeValue];
		len = [textStNew length];
		pos = 0;
		isSPos = NO;
		while (pos < len) { // get speaker code in variable "suSt"
			ch = [textStNew characterAtIndex:pos];
			if (ch == '\n' && pos < len - 1) {
				if (isSPos == YES) {
					posRange = NSMakeRange(sPos, (pos - sPos) + 1);
					textStNew = [textStNew stringByReplacingCharactersInRange:posRange withString:@""];
					len = [textStNew length];
					pos = sPos;
				} else
					pos++;
				ch = [textStNew characterAtIndex:pos];
				if (ch == '%') {
					sPos = pos;
					isSPos = YES;
				} else if (ch == '*' || ch == '@')
					isSPos = NO;
			} else if (pos == 0 && ch == '%') {
				sPos = pos;
				isSPos = YES;
				pos++;
			} else
				pos++;
		}
		if (isSPos == YES) {
			posRange = NSMakeRange(sPos, len - sPos);
			textStNew = [textStNew stringByReplacingCharactersInRange:posRange withString:@""];
			len = [textStNew length];
			pos = sPos;
		}
		[textView setString:textStNew];
		textView.needsDisplay = YES;
		[textView display];
		[cDoc showWindows];
	}
//	len = [textStNew length];
//	len = [textStOrig length];
} // 2022-06-27 end

static void SelectMedia_AddText(NSTextView *textView, NSTextStorage *text, NSRange charRange, unCH *line) {
	NSString *keys;

	keys = [NSString stringWithCharacters:line length:strlen(line)];
	if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
		[text replaceCharactersInRange:charRange withString:keys];
		[textView didChangeText];
		[[textView undoManager] setActionName:@"Media"];
	}
}

- (IBAction)SelectMediaFile:(id)sender { // 2021-02-15 beg
#pragma unused (sender)
	FNType mediaFName[FNSize], *s;
	char tmes[2], isWhatType;
	NSTextStorage *text;
	NSString *textSt;
	NSTextView *textView;
	NSValue *cursorValue;
	NSRange cursorRange;
	NSUInteger cursorPos, cursorLine;
	NSUInteger len, bPos, ePos;
	NSRange endRange;
	Document *cDoc = [self document];

	[SelectMediaController SelectMediaDialog:self str:mediaFName];

	if (mediaFName[0] == EOS)
		return;
	isWhatType = 0;
	s = strrchr(mediaFName, '.');
	if (s != NULL) {
		if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L) ||
			!uS.FNTypeicmp(s, ".wav", 0L) || !uS.FNTypeicmp(s, ".wave", 0L) ||
			!uS.FNTypeicmp(s, ".mp3", 0L)) {
			isWhatType = isAudio;
		} else if (!uS.FNTypeicmp(s, ".mov", 0L) || !uS.FNTypeicmp(s, ".mp4", 0L) ||
				   !uS.FNTypeicmp(s, ".m4v", 0L) || !uS.FNTypeicmp(s, ".flv", 0L) ||
				   !uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L) ||
				   !uS.FNTypeicmp(s, ".avi", 0L) || !uS.FNTypeicmp(s, ".dv", 0L) ||
				   !uS.FNTypeicmp(s, ".dat", 0L)) {
			isWhatType = isVideo;
		}
		*s = EOS;
	}
	s = strrchr(mediaFName, ' ');
	if (s != NULL) {
		do_warning_sheet("Selected file name has a SPACE character in it!!", [self window]);
		return;
	}
	s = strrchr(mediaFName, '\t');
	if (s != NULL) {
		do_warning_sheet("Selected file name has a TAB character in it!!", [self window]);
		return;
	}
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];

	bPos = 0;
	ePos = getUtts(bPos, len, textSt);
	while (1) {
		if (uS.partcmp(sp, "@Begin", FALSE, FALSE) || uS.partcmp(sp, "@Languages:", FALSE, FALSE)   ||
			uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE) || uS.partcmp(sp, "@Options:", FALSE, FALSE) ||
			uS.partcmp(sp, IDOF, FALSE, FALSE)) {
			bPos = ePos;
			ePos = getUtts(bPos, len, textSt);
		} else if (uS.partcmp(sp, "@Media:", FALSE, FALSE)) {
			templineW[0] = EOS;
			endRange = NSMakeRange(bPos, ePos-bPos);
			len = [text length];
			SelectMedia_AddText(textView, text, endRange, templineW);
			textSt = [text string];
			len = [text length];
			ePos = getUtts(bPos, len, textSt);
		} else
			break;
	}

	strcpy(templineC, "@Media:	");
	strcat(templineC, mediaFName);
	if (isWhatType == isAudio)
		strcat(templineC, ", audio");
	else if (isWhatType == isVideo)
		strcat(templineC, ", video");
	strcat(templineC, "\n");
	u_strcpy(templineW, templineC, UTTLINELEN);
	len = [text length];
	SelectMedia_AddText(textView, text, NSMakeRange(bPos, 0), templineW);
	if (len == 0) {
		if ([cDoc docFont] == nil) {
			if (cDoc->isCAFont)
				[cDoc setDocFont:defCAFont];
			else
				[cDoc setDocFont:defUniFont];
		}
		text.font = [cDoc docFont];
	}
	textSt = [text string];
	len = [text length];
	bPos += strlen(templineW);
	ePos = getUtts(bPos, len, textSt);


	cursorValue = [[textView selectedRanges] objectAtIndex:0];
	cursorRange = [cursorValue rangeValue];
	cursorPos = cursorRange.location;
	cursorLine = [self countLineNumbers:cursorPos];
	tmes[0] = EOS;
	[self setStatusLine:cursorLine extraMess:tmes isUpdateDisplay:YES];
} // 2021-02-15 end

- (BOOL)move_cursor_to_bullet:(long)mtime { // 2021-04-27 beg
	char buf[BUFSIZ], *bufEnd;
	long beg, end;
	unichar ch;
	BOOL isSPFound;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSRange cursorRange;
	NSUInteger pos, j, len, bufLen, prev_pos, jCR;

	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	pos = 0; // cursorRange.location + cursorRange.length;
	prev_pos = pos;

	if (mtime == 0L)
		mtime = 1L;
	isSPFound = false;
	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022 && pos+1 < len) {
			pos++;
			ch = [textSt characterAtIndex:pos];
			if (isdigit(ch)) {
				bufLen = 0L;
				do {
					buf[bufLen++] = (char)ch;
					pos++;
					ch = [textSt characterAtIndex:pos];
				} while ((isdigit(ch) || ch =='_') && pos < len);
				if (ch == 0x2022) {
					buf[bufLen] = EOS;
					bufEnd = strchr(buf, '_');
					if (buf[0] != EOS && bufEnd != NULL) {
						beg = atol(buf);
						end = atol(bufEnd+1);
						if (beg <= mtime && end >= mtime) {
							isSPFound = true;
							break;
						} else {
							if (beg > mtime || end > mtime) {
//								isSPFound = true;
								break;
							}
						}
					}
				}
			} else
				pos--;
		}
		pos++;
	}
	if (isSPFound) {
		j = pos;
		while (j > 0) {
			ch = [textSt characterAtIndex:j];
			if (ch == '\n') {
				jCR = j;
				if (j+1 < len) {
					ch = [textSt characterAtIndex:j+1];
					if (ch == '*' || ch == '%' || ch == '@') {
						j++;
						break;
					}
				}
			}
			if (j < prev_pos) {
				j = jCR + 1;
				break;
			}
			j--;
		}
		cursorRange = NSMakeRange(j, 0);
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				cursorRange.length = pos - cursorRange.location;
				break;
			}
			pos++;
		}
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
	}
	return(isSPFound);
} // 2021-04-27 end

// char move_cursor(long mtime, unCH *file, char isSetBE, char isSnd)
- (IBAction)SoundToTextSync:(id)sender { // 2021-02-15 beg
#pragma unused (sender)
	long mtime;
//	FNType *pFName;

	if (SnTr.IsSoundOn) {
		if (SnTr.BegF != SnTr.EndF && SnTr.EndF != 0L)
			mtime = [self conv_to_msec_rep:(SnTr.BegF + ((SnTr.EndF - SnTr.BegF) / 2L))];
		else
			mtime = [self conv_to_msec_rep:SnTr.BegF];
//		if (syncAudioAndVideo(global_df->MvTr.MovieFile))
//			pFName = global_df->MvTr.MovieFile;
//		else
//			pFName = SnTr.mediaFPath;
		if ([self move_cursor_to_bullet:mtime] == false)
			do_warning_sheet("Can't find sound position in a text.", [self window]);
	} else {
		do_warning_sheet("Please start sonic mode first", [self window]);
	}
} // 2021-04-27 end
/*
static pascal Boolean MyMediaFileFilter(AEDesc *theItem,void *info,void *callBackUD,NavFilterModes filterMode) {
#pragma unused(callBackUD, filterMode)

	NavFileOrFolderInfo *FDinfo;

	if (theItem->descriptorType != typeFSRef)
		return true;

	FDinfo = (NavFileOrFolderInfo *)info;
	if (FDinfo->isFolder)
		return true;


	if (NewMediaFileTypes == isAudio) {
		if (FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFF' || // AIFF
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFC' || // AIFC
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'WAVE' || // WAVE
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'jB1 ' || // SoundEdit 16
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Sd2f' || // SoundDesignerII
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mp3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG3' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Mp3 ' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3 ')   // mp3
				return true; // show in box
	} else if (NewMediaFileTypes == isVideo) {
		if (FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MooV' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mpg4' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'flv ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AVI ')
				return true; // show in box
	} else if (NewMediaFileTypes == isAllType) {
		if (FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFF' || // AIFF
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFC' || // AIFC
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'WAVE' || // WAVE
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'jB1 ' || // SoundEdit 16
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Sd2f' || // SoundDesignerII
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mp3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG3' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Mp3 ' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3 ' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MooV' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mpg4' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'flv ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AVI ')
				return true; // show in box
	} else if (NewMediaFileTypes == isPictText) {
		if (FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'TEXT' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'PICT' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'JPEG' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'GIFf')
				return true; // show in box
	} else {
		if (FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFF' || // AIFF
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AIFC' || // AIFC
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'WAVE' || // WAVE
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'jB1 ' || // SoundEdit 16
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Sd2f' || // SoundDesignerII
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mp3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3!' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG3' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'Mp3 ' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MP3 ' || // mp3
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MooV' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'mpg4' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'flv ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'MPG ' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'AVI ' ||
			//			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'TEXT' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'PICT' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'JPEG' ||
			FDinfo->fileAndFolder.fileInfo.finderInfo.fdType == 'GIFf')
				return true; // show in box
	}

	if (theItem->descriptorType == typeFSRef) {
		FSRef ref;
		FNType fileName[FNSize];
		FNType *s;

		AEGetDescData(theItem, &ref, sizeof(ref));
		my_FSRefMakePath(&ref, fileName, FNSize);
		uS.lowercasestr(fileName, &dFnt, FALSE);
		s = strrchr(fileName, '.');
		if (s != NULL) {
			if (NewMediaFileTypes == isAudio) {
				if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L)  ||
					!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L) ||
					!uS.FNTypeicmp(s, ".mp3", 0L) 		// vcd file
					) {
						return true; // show in box
				}
			} else if (NewMediaFileTypes == isVideo) {
				if (!uS.FNTypeicmp(s, ".mov", 0L)  || !uS.FNTypeicmp(s, ".mp4", 0L)  ||
					!uS.FNTypeicmp(s, ".m4v", 0L)  || !uS.FNTypeicmp(s, ".flv", 0L)  ||
					!uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L)  ||
					!uS.FNTypeicmp(s, ".avi", 0L)  || !uS.FNTypeicmp(s, ".dv", 0L)   ||
					!uS.FNTypeicmp(s, ".dat", 0L) 		// vcd file
					) {
						return true; // show in box
				}
			} else if (NewMediaFileTypes == isAllType) {
				if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L)  ||
					!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L) ||
					!uS.FNTypeicmp(s, ".mp3", 0L)  ||
					!uS.FNTypeicmp(s, ".mov", 0L)  || !uS.FNTypeicmp(s, ".mp4", 0L)  ||
					!uS.FNTypeicmp(s, ".m4v", 0L)  || !uS.FNTypeicmp(s, ".flv", 0L)  ||
					!uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L)  ||
					!uS.FNTypeicmp(s, ".avi", 0L)  || !uS.FNTypeicmp(s, ".dv", 0L)   ||
					!uS.FNTypeicmp(s, ".dat", 0L) 		// vcd file
					) {
						return true; // show in box
				}
			} else if (NewMediaFileTypes == isPictText) {
				if (!uS.FNTypeicmp(s, ".txt", 0L)  || !uS.FNTypeicmp(s, ".cut", 0L)  ||
					!uS.FNTypeicmp(s, ".cha", 0L)  || !uS.FNTypeicmp(s, ".cdc", 0L)  ||
					!uS.FNTypeicmp(s, ".pict", 0L) ||
					!uS.FNTypeicmp(s, ".jpg", 0L)  || !uS.FNTypeicmp(s, ".jpeg", 0L) ||
					!uS.FNTypeicmp(s, ".gif", 0L)
					) {
						return true; // show in box
				}
			} else {
				if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L)  ||
					!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L) ||
					!uS.FNTypeicmp(s, ".mp3", 0L)  ||
					!uS.FNTypeicmp(s, ".mov", 0L)  || !uS.FNTypeicmp(s, ".mp4", 0L)  ||
					!uS.FNTypeicmp(s, ".m4v", 0L)  || !uS.FNTypeicmp(s, ".flv", 0L)  ||
					!uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L)  ||
					!uS.FNTypeicmp(s, ".avi", 0L)  || !uS.FNTypeicmp(s, ".dv", 0L)   ||
					!uS.FNTypeicmp(s, ".txt", 0L)  || !uS.FNTypeicmp(s, ".cut", 0L)  ||
					!uS.FNTypeicmp(s, ".cha", 0L)  || !uS.FNTypeicmp(s, ".cdc", 0L)  ||
					!uS.FNTypeicmp(s, ".pict", 0L) ||
					!uS.FNTypeicmp(s, ".jpg", 0L)  || !uS.FNTypeicmp(s, ".jpeg", 0L) ||
					!uS.FNTypeicmp(s, ".gif", 0L)  ||
					!uS.FNTypeicmp(s, ".dat", 0L) 		// vcd file
					) {
						return true; // show in box
				}
			}
		}
		return false; // do not show
	} else
		return true;
}

char GetNewMediaFile(char isCheckError, char whatMediaType) {
	FNType		*c, retFileName[FNSize];
	OSType		resType;
	Boolean		good;
	const char	*prompt;

	NewMediaFileTypes = whatMediaType;
	if (whatMediaType == 0) {
		prompt = "Please locate movie, sound, picture or text file";
	} else if (whatMediaType == isPictText) {
		prompt = "Please locate TEXT or JPEG file ONLY";
	} else if (whatMediaType == isAudio) {
		prompt = "Please locate sound file ONLY";
	} else {
		prompt = "Please locate movie or sound file ONLY";
	}

	retFileName[0] = EOS;
	if (myNavGetFile(prompt, -1, nil, (NavObjectFilterProcPtr)MyMediaFileFilter, retFileName)) {
		good = true;
		getFileType(retFileName, &resType);
	} else {
		good = false;
		resType = 0;
	}
	if (good) {
		if (!isRightMediaFolder(retFileName, global_df->err_message)) {
			if (pSnTr->SoundFile[0] != EOS) {
				pSnTr->SoundFile[0] = EOS;
				if (pSnTr->isMP3 == TRUE) {
					pSnTr->isMP3 = FALSE;
					if (pSnTr->mp3.hSys7SoundData)
						DisposeHandle(pSnTr->mp3.hSys7SoundData);
					pSnTr->mp3.theSoundMedia = NULL;
					pSnTr->mp3.hSys7SoundData = NULL;
				} else {
					fclose(pSnTr->SoundFPtr);
				}
				pSnTr->SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
			}
		} else if (resType == 'PICT' || resType == 'JPEG' || resType == 'GIFf') {
			strcpy(global_df->PcTr.pictFName,retFileName);
			return(isPict);
		} else if (resType == 'TEXT') {
			strcpy(global_df->TxTr.textFName,retFileName);
			return(isText);
		} else if (resType == 'MooV' || resType == 'mpg4' || resType == 'flv ' || resType == 'MPG ' || resType == 'AVI ' || resType == 'dvc!') {
			if (isCheckError && !strcmp(retFileName, global_df->MvTr.rMovieFile)) {
				strcpy(global_df->err_message, "+Movie marker is not found at cursor position. Please move text cursor next to the bullet.");
				return(0);
			}
			global_df->MvTr.MBeg = 0L;
			global_df->MvTr.MEnd = 0L;
			strcpy(global_df->MvTr.rMovieFile, retFileName);
			if ((c=strrchr(retFileName, PATHDELIMCHR)) != NULL)
				strcpy(retFileName, c+1);
			if ((c=strrchr(retFileName,'.')) != NULL)
				*c = EOS;
			u_strcpy(global_df->MvTr.MovieFile, retFileName, FILENAME_MAX);
			return(isVideo);
		} else if (resType == 'AIFF'  || resType == 'AIFC' || resType == 'WAVE' || resType == 'MP3!') {
			if (isCheckError && pSnTr->SoundFile[0] != EOS && !strcmp(retFileName, pSnTr->rSoundFile)) {
				strcpy(global_df->err_message, "+Sound marker is not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"ESC-0\"");
				return(0);
			}
			if (pSnTr->isMP3 == TRUE) {
				pSnTr->isMP3 = FALSE;
				if (pSnTr->mp3.hSys7SoundData)
					DisposeHandle(pSnTr->mp3.hSys7SoundData);
				pSnTr->mp3.theSoundMedia = NULL;
				pSnTr->mp3.hSys7SoundData = NULL;
			} else if (pSnTr->SoundFPtr != 0) {
				fclose(pSnTr->SoundFPtr);
				pSnTr->SoundFile[0] = EOS;
				pSnTr->SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
			}
			DrawSoundCursor(0);
			pSnTr->BegF = 0L;
			pSnTr->EndF = 0L;
			SetPBCglobal_df(false, 0L);
			pSnTr->SFileType = resType;
			if (pSnTr->SFileType == 'MP3!') {
				if (GetMovieMedia(retFileName,&pSnTr->mp3.theSoundMedia,&pSnTr->mp3.hSys7SoundData) != noErr) {
					sprintf(global_df->err_message, "+Can't open sound file: %s. Perhaps it is already opened by other application or in another window.", retFileName);
					pSnTr->SoundFile[0] = EOS;
					pSnTr->SoundFPtr = 0;
					if (global_df->SoundWin)
						DisposeOfSoundWin();
					return(0);
				}
				pSnTr->isMP3 = TRUE;
			} else if ((pSnTr->SoundFPtr=fopen(retFileName, "rb")) == NULL) {
				sprintf(global_df->err_message, "+Can't open sound file: %s. Perhaps it is already opened by other application or in another window.", retFileName);
				pSnTr->SoundFile[0] = EOS;
				pSnTr->SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
				return(0);
			}
			strcpy(pSnTr->rSoundFile,retFileName);
			if (!CheckRateChan(&global_df->SnTr, global_df->err_message)) {
				pSnTr->SoundFile[0] = EOS;
				if (pSnTr->isMP3 == TRUE) {
					pSnTr->isMP3 = FALSE;
					if (pSnTr->mp3.hSys7SoundData)
						DisposeHandle(pSnTr->mp3.hSys7SoundData);
					pSnTr->mp3.theSoundMedia = NULL;
					pSnTr->mp3.hSys7SoundData = NULL;
				} else {
					fclose(pSnTr->SoundFPtr);
					pSnTr->SoundFPtr = 0;
					if (global_df->SoundWin)
						DisposeOfSoundWin();
				}
				return(0);
			}
			if ((c=strrchr(retFileName, PATHDELIMCHR)) != NULL)
				strcpy(retFileName, c+1);
			if ((c=strrchr(retFileName,'.')) != NULL)
				*c = EOS;
			u_strcpy(pSnTr->SoundFile, retFileName, FILENAME_MAX);
			ResetUndos();
			pSnTr->WBegFM = 0L;
			pSnTr->WEndFM = 0L;
			return(isAudio);
		} else {
			if (pSnTr->SoundFile[0] != EOS) {
				pSnTr->SoundFile[0] = EOS;
				if (pSnTr->isMP3 == TRUE) {
					pSnTr->isMP3 = FALSE;
					if (pSnTr->mp3.hSys7SoundData)
						DisposeHandle(pSnTr->mp3.hSys7SoundData);
					pSnTr->mp3.theSoundMedia = NULL;
					pSnTr->mp3.hSys7SoundData = NULL;
				} else {
					fclose(pSnTr->SoundFPtr);
				}
				pSnTr->SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
			}
			strcpy(global_df->err_message, DASHES);
		}
	} else {
		if (pSnTr->SoundFile[0] != EOS) {
			pSnTr->SoundFile[0] = EOS;
			if (pSnTr->isMP3 == TRUE) {
				pSnTr->isMP3 = FALSE;
				if (pSnTr->mp3.hSys7SoundData)
					DisposeHandle(pSnTr->mp3.hSys7SoundData);
				pSnTr->mp3.theSoundMedia = NULL;
				pSnTr->mp3.hSys7SoundData = NULL;
			} else {
				fclose(pSnTr->SoundFPtr);
			}
			pSnTr->SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
		strcpy(global_df->err_message, DASHES);
	}
	return(0);
}
*/
- (IBAction)InsertBulletIntoText:(id)sender {
#pragma unused (sender)
	unsigned short wID;
	Document *cDoc;
	cDoc = [self document];
	wID = [cDoc get_wID];

	if (wID != DOCWIN) {
		do_warning_sheet("This command only works in text window", [self window]);
	} else if (SnTr.IsSoundOn == true) {
		if (SnTr.BegF != SnTr.EndF && SnTr.EndF != 0L) {
			[self addBulletsToText:SOUNDTIER begin:[self conv_to_msec_rep:SnTr.BegF] end:[self conv_to_msec_rep:SnTr.EndF]];
		}
	} else {
		do_warning_sheet("This command only works in Sonic Mode", [self window]);
/*
		char ret;
		FNType mFileName[FILENAME_MAX];
		unCH   wFileName[FILENAME_MAX];
		FNType *c;
		if ((ret=GetNewMediaFile(TRUE, whatMediaType))) {
			if (ret == isAudio) { // sound
			} else if (ret == isVideo) { // movie
			} else if (ret == isPict) { // picture
				DrawCursor(0);
				extractFileName(mFileName, global_df->PcTr.pictFName);
				if ((c=strrchr(mFileName,'.')) != NULL)
					*c = EOS;
				u_strcpy(wFileName, mFileName, FILENAME_MAX);
				addBulletsToText(PICTTIER, wFileName, 0L, 0L);
				return(TRUE);
			} else if (ret == isText) { // text
				DrawCursor(0);
				extractFileName(mFileName, global_df->TxTr.textFName);
				if ((c=strrchr(mFileName,'.')) != NULL)
					*c = EOS;
				u_strcpy(wFileName, mFileName, FILENAME_MAX);
				addBulletsToText(TEXTTIER, wFileName, 0L, 0L);
				return(TRUE);
			}
		}
*/
	}
}

- (void)MoveMediaBegRight {
	char buf[BUFSIZ], *bufEnd, bType;
	long beg, end;
	unichar ch;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt, *keys;
	NSUInteger pos, bulletPosBeg, bulletPosEnd, len, bufLen;
	Document *cDoc;
	NSRange charRange, cursorRange;

	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;

	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
		}
	}
	if (pos == 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1) {
				do_warning_sheet("Can't find a right bullet on this line", [self window]);
				return;
			}
			pos++;
		}
	}
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022) {
			if (pos < len-1) {
				ch = [textSt characterAtIndex:pos+1];
			}
			if (!isdigit(ch) && ch != '%') {
				pos--;
				bType = newBullet;
				while (pos > 0) {
					ch = [textSt characterAtIndex:pos];
					if (ch == '\n') {
						do_warning_sheet("Can't find a right bullet on this line", [self window]);
						return;
					} else if (ch == 0x2022) {
						bulletPosBeg = pos;
						bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
						if (bulletPosBeg < bulletPosEnd && pos <= bulletPosEnd && bType == newBullet)
							break;
					}
					pos--;
				}
			}
		}
	}
	bType = newBullet;
	bulletPosBeg = 0;
	bulletPosEnd = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			do_warning_sheet("Can't find a right bullet on this line", [self window]);
			return;
		} else if (ch == 0x2022) {
			bulletPosBeg = pos;
			bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
			if (bulletPosBeg < bulletPosEnd) {
				if (pos <= bulletPosEnd) {
					if (bType == newBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							beg = atol(buf) + 25L;
							end = atol(bufEnd+1);
							if (beg > end)
								beg = end - 1;
							charRange.location = bulletPosBeg;
							charRange.length = bulletPosEnd - bulletPosBeg + 1;
							if (bulletPosBeg > 0)
								ch = [textSt characterAtIndex:bulletPosBeg-1];
							else
								ch = ' ';
							if (!isSpace(ch))
								sprintf(buf, " %s%ld_%ld%s", "•", beg, end, "•");
							else
								sprintf(buf, "%s%ld_%ld%s", "•", beg, end, "•");
							keys = [NSString stringWithUTF8String:buf];
							if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
								[text replaceCharactersInRange:charRange withString:keys];
								len = [text length];
								[textView didChangeText];
								[[textView undoManager] setActionName:@"Bullet"];
							}
							return;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
	}
	do_warning_sheet("Can't find a right bullet on this line", [self window]);
}

- (void)MoveMediaBegLeft {
	char buf[BUFSIZ], *bufEnd, bType;
	long beg, end;
	unichar ch;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt, *keys;
	NSUInteger pos, bulletPosBeg, bulletPosEnd, len, bufLen;
	Document *cDoc;
	NSRange charRange, cursorRange;
	
	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;
	
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
		}
	}
	if (pos == 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1) {
				do_warning_sheet("Can't find a right bullet on this line", [self window]);
				return;
			}
			pos++;
		}
	}
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022) {
			if (pos < len-1) {
				ch = [textSt characterAtIndex:pos+1];
			}
			if (!isdigit(ch) && ch != '%') {
				pos--;
				bType = newBullet;
				while (pos > 0) {
					ch = [textSt characterAtIndex:pos];
					if (ch == '\n') {
						do_warning_sheet("Can't find a right bullet on this line", [self window]);
						return;
					} else if (ch == 0x2022) {
						bulletPosBeg = pos;
						bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
						if (bulletPosBeg < bulletPosEnd && pos <= bulletPosEnd && bType == newBullet)
							break;
					}
					pos--;
				}
			}
		}
	}
	bType = newBullet;
	bulletPosBeg = 0;
	bulletPosEnd = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			do_warning_sheet("Can't find a right bullet on this line", [self window]);
			return;
		} else if (ch == 0x2022) {
			bulletPosBeg = pos;
			bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
			if (bulletPosBeg < bulletPosEnd) {
				if (pos <= bulletPosEnd) {
					if (bType == newBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							beg = atol(buf) - 25L;
							end = atol(bufEnd+1);
							if (beg < 0L)
								beg = 0L;
							charRange.location = bulletPosBeg;
							charRange.length = bulletPosEnd - bulletPosBeg + 1;
							if (bulletPosBeg > 0)
								ch = [textSt characterAtIndex:bulletPosBeg-1];
							else
								ch = ' ';
							if (!isSpace(ch))
								sprintf(buf, " %s%ld_%ld%s", "•", beg, end, "•");
							else
								sprintf(buf, "%s%ld_%ld%s", "•", beg, end, "•");
							keys = [NSString stringWithUTF8String:buf];
							if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
								[text replaceCharactersInRange:charRange withString:keys];
								len = [text length];
								[textView didChangeText];
								[[textView undoManager] setActionName:@"Bullet"];
							}
							return;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
	}
	do_warning_sheet("Can't find a right bullet on this line", [self window]);
}

- (void)MoveMediaEndRight {
	char buf[BUFSIZ], *bufEnd, bType;
	long beg, end;
	unichar ch;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt, *keys;
	NSUInteger pos, bulletPosBeg, bulletPosEnd, len, bufLen;
	Document *cDoc;
	NSRange charRange, cursorRange;
	
	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;
	
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
		}
	}
	if (pos == 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1) {
				do_warning_sheet("Can't find a right bullet on this line", [self window]);
				return;
			}
			pos++;
		}
	}
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022) {
			if (pos < len-1) {
				ch = [textSt characterAtIndex:pos+1];
			}
			if (!isdigit(ch) && ch != '%') {
				pos--;
				bType = newBullet;
				while (pos > 0) {
					ch = [textSt characterAtIndex:pos];
					if (ch == '\n') {
						do_warning_sheet("Can't find a right bullet on this line", [self window]);
						return;
					} else if (ch == 0x2022) {
						bulletPosBeg = pos;
						bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
						if (bulletPosBeg < bulletPosEnd && pos <= bulletPosEnd && bType == newBullet)
							break;
					}
					pos--;
				}
			}
		}
	}
	bType = newBullet;
	bulletPosBeg = 0;
	bulletPosEnd = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			do_warning_sheet("Can't find a right bullet on this line", [self window]);
			return;
		} else if (ch == 0x2022) {
			bulletPosBeg = pos;
			bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
			if (bulletPosBeg < bulletPosEnd) {
				if (pos <= bulletPosEnd) {
					if (bType == newBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							beg = atol(buf);
							end = atol(bufEnd+1) + 25L;
							charRange.location = bulletPosBeg;
							charRange.length = bulletPosEnd - bulletPosBeg + 1;
							if (bulletPosBeg > 0)
								ch = [textSt characterAtIndex:bulletPosBeg-1];
							else
								ch = ' ';
							if (!isSpace(ch))
								sprintf(buf, " %s%ld_%ld%s", "•", beg, end, "•");
							else
								sprintf(buf, "%s%ld_%ld%s", "•", beg, end, "•");
							keys = [NSString stringWithUTF8String:buf];
							if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
								[text replaceCharactersInRange:charRange withString:keys];
								len = [text length];
								[textView didChangeText];
								[[textView undoManager] setActionName:@"Bullet"];
							}
							return;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
	}
	do_warning_sheet("Can't find a right bullet on this line", [self window]);
}

- (void)MoveMediaEndLeft {
	char buf[BUFSIZ], *bufEnd, bType;
	long beg, end;
	unichar ch;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt, *keys;
	NSUInteger pos, bulletPosBeg, bulletPosEnd, len, bufLen;
	Document *cDoc;
	NSRange charRange, cursorRange;
	
	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;
	
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
		}
	}
	if (pos == 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos >= len-1) {
				do_warning_sheet("Can't find a right bullet on this line", [self window]);
				return;
			}
			pos++;
		}
	}
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022) {
			if (pos < len-1) {
				ch = [textSt characterAtIndex:pos+1];
			}
			if (!isdigit(ch) && ch != '%') {
				pos--;
				bType = newBullet;
				while (pos > 0) {
					ch = [textSt characterAtIndex:pos];
					if (ch == '\n') {
						do_warning_sheet("Can't find a right bullet on this line", [self window]);
						return;
					} else if (ch == 0x2022) {
						bulletPosBeg = pos;
						bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
						if (bulletPosBeg < bulletPosEnd && pos <= bulletPosEnd && bType == newBullet)
							break;
					}
					pos--;
				}
			}
		}
	}
	bType = newBullet;
	bulletPosBeg = 0;
	bulletPosEnd = 0;
	for (; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			do_warning_sheet("Can't find a right bullet on this line", [self window]);
			return;
		} else if (ch == 0x2022) {
			bulletPosBeg = pos;
			bulletPosEnd = [self isLegalBullet:pos bTest:&bType];
			if (bulletPosBeg < bulletPosEnd) {
				if (pos <= bulletPosEnd) {
					if (bType == newBullet) {
						bufLen = 0L;
						for (pos=bulletPosBeg+1; pos < bulletPosEnd; pos++) {
							ch = [textSt characterAtIndex:pos];
							buf[bufLen++] = (char)ch;
						}
						buf[bufLen] = EOS;
						bufEnd = strchr(buf, '_');
						if (buf[0] != EOS && bufEnd != NULL) {
							beg = atol(buf);
							end = atol(bufEnd+1) - 25L;
							if (beg >= end)
								end = beg + 1;
							charRange.location = bulletPosBeg;
							charRange.length = bulletPosEnd - bulletPosBeg + 1;
							if (bulletPosBeg > 0)
								ch = [textSt characterAtIndex:bulletPosBeg-1];
							else
								ch = ' ';
							if (!isSpace(ch))
								sprintf(buf, " %s%ld_%ld%s", "•", beg, end, "•");
							else
								sprintf(buf, "%s%ld_%ld%s", "•", beg, end, "•");
							keys = [NSString stringWithUTF8String:buf];
							if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
								[text replaceCharactersInRange:charRange withString:keys];
								len = [text length];
								[textView didChangeText];
								[[textView undoManager] setActionName:@"Bullet"];
							}
							return;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
	}
	do_warning_sheet("Can't find a right bullet on this line", [self window]);
}


#ifdef _WIN32
extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);
extern char isOverRidePicName;
#endif

static void convertToURLText(char *to, char *from) {
	int ito, ifr, i;
	char hex[50];
	
	ito = 0;
	ifr = 0;
	while (from[ifr] != EOS) {
		if (isalnum(from[ifr]) || from[ifr] == '-' || from[ifr] == '_') {
			to[ito++] = from[ifr++];
		} else if (!strncmp(from+ifr, "&graText=", 9)) {
			for (i=0; i < 9; i++)
				to[ito++] = from[ifr++];
		} else if (!strncmp(from+ifr, "&mainText=", 10)) {
			for (i=0; i < 10; i++)
				to[ito++] = from[ifr++];
		} else { 
			to[ito++] = '%';
			sprintf(hex, "%x", (unsigned char)from[ifr++]);
//			if (hex[1] == EOS) {
//				to[ito++] = hex[0];
//			} else {
				to[ito++] = hex[0];
				to[ito++] = hex[1];
//			}
		}
	}
	to[ito] = EOS;
}


- (void)fillup_templine:(NSString *)textSt curPos:(NSUInteger)pos length:(NSUInteger)len tier:(unCH *)line {
	BOOL hf;
	int j;
	unichar ch;
	
	hf = false;
	j = 0;
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (j < UTTLINELEN) {
			if (ch == 0x2022)
				hf = !hf;
			if (hf == false && ch != '\n') {
				if (ch == '\t')
					line[j++] = ' ';
				else
					line[j++] = ch;
			}
		}
		if (ch == '\n') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
				if (ch == '*' || ch == '%' || ch == '@') {
					pos++;
					break;
				}
			}
		}
		pos++;
	}
	line[j] = EOS;
	for (j=0; line[j] != EOS && line[j] != ':'; j++) ;
	if (line[j] == ':') {
		for (j++; line[j] == ' '; j++) ;
		strcpy(line, line+j);
	}
}

- (void)ShowGRA:(NSUInteger)chrIndex{
	char jpgTag[20];
	FILE *fp;
	unichar ch;
	unichar code[BUFSIZ];
	Document *cDoc;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger pos, i, len;
	BOOL graf, grtf, morf, trnf;
	NSUInteger gra, grt, mor, trn;
	
	cDoc = [self document];
	if (cDoc == nil)
		return;
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	templineC3[0] = EOS;
	graf = false;
	grtf = false;
	morf = false;
	trnf = false;
	for (pos=chrIndex; pos > 0; pos--) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			ch = [textSt characterAtIndex:pos+1];
			if (ch == (unCH)'%') {
				if (pos+6 >= len) {
					break;
				} else {
					[textSt getCharacters:code range:NSMakeRange(pos+1, 5)];
					code[5] = EOS;
					if (uS.mStricmp(code, "%gra:") == 0) {
						gra = pos+1;
						graf = true;
					} else if (uS.mStricmp(code, "%grt:") == 0) {
						grt = pos+1;
						grtf = true;
					} else if (uS.mStricmp(code, "%mor:") == 0) {
						mor = pos+1;
						morf = true;
					} else if (uS.mStricmp(code, "%trn:") == 0){
						trn = pos+1;
						trnf = true;
					}
				}
			} else if (isMainSpeaker(ch)) {
				[self fillup_templine:textSt curPos:pos+1 length:len tier:templine4];
				UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC3, NULL, UTTLINELEN);
				break;
			}
		}
	}
	if (pos == 0 && templineC3[0] == EOS) {
		ch = [textSt characterAtIndex:pos];
		if (ch == (unCH)'%') {
			if (pos+5 < len) {
				[textSt getCharacters:code range:NSMakeRange(pos, 5)];
				code[5] = EOS;
				if (uS.mStricmp(code, "%gra:") == 0) {
					gra = pos;
					graf = true;
				} else if (uS.mStricmp(code, "%grt:") == 0) {
					grt = pos;
					grtf = true;
				} else if (uS.mStricmp(code, "%mor:") == 0) {
					mor = pos;
					morf = true;
				} else if (uS.mStricmp(code, "%trn:") == 0){
					trn = pos;
					trnf = true;
				}
			}
		} else if (isMainSpeaker(ch)) {
			[self fillup_templine:textSt curPos:pos length:len tier:templine4];
			UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC3, NULL, UTTLINELEN);
		}
	}
	for (pos=chrIndex; pos < len; pos++) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			ch = [textSt characterAtIndex:pos+1];
			if (ch == (unCH)'%') {
				pos++;
				if (pos+5 >= len) {
					break;
				} else {
					[textSt getCharacters:code range:NSMakeRange(pos, 5)];
					code[5] = EOS;
					if (uS.mStricmp(code, "%gra:") == 0) {
						gra = pos;
						graf = true;
					} else if (uS.mStricmp(code, "%grt:") == 0) {
						grt = pos;
						grtf = true;
					} else if (uS.mStricmp(code, "%mor:") == 0) {
						mor = pos;
						morf = true;
					} else if (uS.mStricmp(code, "%trn:") == 0){
						trn = pos;
						trnf = true;
					}
				}
			} else if (isMainSpeaker(ch) || ch == (unCH)'@') {
				break;
			}
		}
	}
	if (grtf) {
		[self fillup_templine:textSt curPos:grt length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC, NULL, UTTLINELEN);
	} else if (graf) {
		[self fillup_templine:textSt curPos:gra length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC, NULL, UTTLINELEN);
	} else {
		do_warning_sheet("Can't find %grt or %gra tier.", [self window]);
		return;
	}
	if (trnf == false && morf == false) {
		do_warning_sheet("Can't find %trn or %mor tier.", [self window]);
		return;
	}
	if (trnf && grtf) {
		[self fillup_templine:textSt curPos:trn length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC2, NULL, UTTLINELEN);
	} else if (morf && grtf) {
		[self fillup_templine:textSt curPos:mor length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC2, NULL, UTTLINELEN);
	} else if (morf && graf) {
		[self fillup_templine:textSt curPos:mor length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC2, NULL, UTTLINELEN);
	} else if (trnf && graf) {
		[self fillup_templine:textSt curPos:trn length:len tier:templine4];
		UnicodeToUTF8(templine4, strlen(templine4), (unsigned char *)templineC2, NULL, UTTLINELEN);
	} else {
		do_warning_sheet("Can't find %trn or %mor tier.", [self window]);
		return;
	}
	if (templineC3[0] == EOS) {
		strcpy(templineC3, "*NO MAIN SPEAKER FOUND*");
	}	
	strcpy(templineC1, "https://talkbank.org/cgi-bin/morgra2jpg.cgi?morText=");
	i = strlen(templineC1);
	strcpy(templineC4, "https://talkbank.org/cgi-bin/morgra2jpg.cgi?morText=");
	strcat(templineC4, templineC2);
	strcat(templineC4, "&graText=");
	strcat(templineC4, templineC);
	strcat(templineC4, "&mainText=");
	strcat(templineC4, templineC3);
	convertToURLText(templineC1+i, templineC4+i);
	if (cDoc->filePath[0] != EOS)
		strcpy(FileName1, prefsDir);
	else
		extractPath(FileName1, cDoc->filePath);
	addFilename2Path(FileName1, "0graph.jpg");
	if (!DownloadURL(templineC1, 60000L, NULL, 0, FileName1, FALSE, TRUE, "Downloading gra graph...")) {
		do_warning("WEB CONNECTION FAILED", -1);
		return;
	}
	if (access(FileName1, 0)) {
		do_warning_sheet("ACCESS TO IMAGE FILE FAILED.", [self window]);
		return;
	} else {
		fp = fopen(FileName1, "rb");
		if (fp == NULL) {
			do_warning_sheet("ACCESS TO IMAGE FILE FAILED.", [self window]);
			return;
		}
		i = 0;
		jpgTag[0] = EOS;
		while (!feof(fp) && i < 20)
			jpgTag[i++] = getc(fp);
		fclose(fp);
		if (uS.mStrnicmp(jpgTag, "<h1>Software error:", 19) == 0) {
			do_warning("Possibly %mor tier length does not match %gra tier length", -1);
			do_warning_sheet("Possibly %mor tier length does not match %gra tier length", [self window]);
			return;
		} else if (uS.mStrnicmp(jpgTag, "<!DOCTYPE", 9) == 0) {
			do_warning_sheet("Internal error, image generating script missing on talkbank.org, contact support", [self window]);
			return;
		}
	}
	[PictController openPict:nil curDocPath:(FNType *)FileName1 docWin:[self window]];
}

- (void)blinkCursorLine {
	NSUInteger pos, sc, ec;
	unichar ch;
	NSRange cursorRange;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt, *stChars;

	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	if (cursorRange.length == 0)
		return;
	pos = cursorRange.location;
	if (pos >= [text length])
		return;
	sc = cursorRange.location;
	ec = cursorRange.location + cursorRange.length;
	if (ec - sc > 25) {
		do_warning_sheet("Please select < 25 characters", [self window]);
		return;
	}
	stChars = [NSString stringWithUTF8String:""];
//	stChar = [NSString string];
	for (; sc < ec; sc++) {
		ch = [textSt characterAtIndex:sc];
		stChars = [stChars stringByAppendingString:@"| "];
		stChars = [stChars stringByAppendingString:[NSString stringWithFormat:@"%C", ch]];
//		stChars = [stChars stringByAppendingString:[NSString stringWithFormat:@"=0x%x", ch]];
		stChars = [stChars stringByAppendingFormat:@"=0x%x", ch];
	}
	stChars = [stChars stringByAppendingString:@"|"];
	[stChars getCString:templineC maxLength:UTTLINELEN encoding:NSUTF8StringEncoding];
	do_warning_sheet(templineC, [self window]);
}

@end

@implementation DocumentWindowController(SonicModeCode)

- (IBAction)lowerSliderChanged:(id)sender {
#pragma unused (sender)
	NSInteger pos;

	if (SnTr.IsSoundOn == true) {
		pos = LowerSlider.integerValue;
		if (pos <= SnTr.SoundFileSize - SnTr.WaveWidth) {
			SnTr.WBegF = pos;
			SnTr.WEndF = SnTr.WBegF + SnTr.WaveWidth;
			[lowerView setNeedsDisplay:YES];
		}
	}
}

- (void)addBulletsToText:(const char *)tagName begin:(long)begin end:(long)end {
#pragma unused (tagName)
	char buf[BUFSIZ];
	unichar ch;
	NSRange cursorRange;
	NSUInteger j, pos, len, bufLen;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSString *keys;
	NSRange charRange;

	sprintf(buf, " %s%ld_%ld%s", "•", begin, end, "•");
	textView = [self firstTextView];
	text = [textView textStorage];
	textSt = [text string];
 	cursorRange = [textView selectedRange];
	len = [text length];

	pos = cursorRange.location + cursorRange.length;
	if (pos >= len)
		pos = len - 1;
	charRange = NSMakeRange(pos, 0); // (NSUInteger pos, NSUInteger len)
	j = pos;
	if (j > 0) {
		ch = [textSt characterAtIndex:j];
		if ((isdigit(ch) || ch =='_') && j < len) {
			do {
				j++;
				ch = [textSt characterAtIndex:j];
			} while ((isdigit(ch) || ch =='_') && j < len);
		} else if (ch == 0x2022 && j < len) {
			ch = [textSt characterAtIndex:j+1];
			if ((isdigit(ch) || ch =='_') && j < len) {
				do {
					j++;
					ch = [textSt characterAtIndex:j];
				} while ((isdigit(ch) || ch =='_') && j < len);
			}
		} else if (j > 0) {
			ch = [textSt characterAtIndex:j-1];
			if (ch == 0x2022 && j > 0)
				j--;
			else if (isSpace(ch)) {
				strcpy(buf, buf+1);
			}
		}
	}
	ch = [textSt characterAtIndex:j];
	if (ch == 0x2022 && j > 0) {
		bufLen = j;
		j--;
		ch = [textSt characterAtIndex:j];
		if (isdigit(ch)) {
			do {
				j--;
				ch = [textSt characterAtIndex:j];
			} while ((isdigit(ch) || ch =='_') && j > 0);
			if (ch == 0x2022) {
				if (j > 0) {
					ch = [textSt characterAtIndex:j-1];
					if (ch == ' ') {
						j--;
					}
				}
				charRange.location = j;
				charRange.length = bufLen - j + 1;
			}
		}
	} else if (!isSpace(ch)) {
		strcat(buf, " ");
	}

	ch = [textSt characterAtIndex:charRange.location];
	if (ch == '\n' && charRange.location > 0 && charRange.length == 0) {
		if (charRange.location + 1 == len)
			charRange.location--;
	}

	keys = [NSString stringWithUTF8String:buf];
	if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
		[text replaceCharactersInRange:charRange withString:keys];
		len = [text length];
		[textView didChangeText];
		[[textView undoManager] setActionName:@"Bullet"];
	}
}

- (long)roundUp:(double)num {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}

- (long)conv_to_msec_rep:(long)num {
	double res, num2;

	res = (double)num;
	num2 = SnTr.HScale;
	res = res * num2;
	num2 = SnTr.SNDrate / (double)1000.0000;
	res = res / num2;
//	num2 = (double)SnTr.SNDsample;
//	res = res / num2;
	num2 = (double)SnTr.SNDchan;
	res = res / num2;

	return([self roundUp:res]);
}

- (long)conv_from_msec_rep:(double)num {
	double res, num2;

	res = num;
	num2 = SnTr.SNDrate / (double)1000.0000;
	res = res * num2;
//	num2 = (double)SnTr.SNDsample;
//	res = res * num2;
	num2 = (double)SnTr.SNDchan;
	res = res * num2;
	num2 = SnTr.HScale;
	res = res / num2;

	return([self roundUp:res]);
}

- (void)DisposeOfSoundData {
	[self setSliderHidden:YES];
	[self setSliderMinNum:(double)0 maxNum:(double)0];
	SnTr.IsSoundOn = false;
	SnTr.IsPlayMedia = false;
	SnTr.isUpdateSlider = true;
	if (SnTr.TopP1)
		free(SnTr.TopP1);
	if (SnTr.TopP2)
		free(SnTr.TopP2);
	SnTr.StatusLineType = 0;
	SnTr.mediaFPath[0] = EOS;
	SnTr.VScale = 0;
	SnTr.HScale = 0L;
	SnTr.WaveWidth = 0;
	SnTr.SoundFileSize = 0L;
	SnTr.TopP1 = NULL;
	SnTr.TopP2 = NULL;
}

- (long)ComputeOffset:(long)prc {
	long Offset;

	Offset = (SnTr.WEndF-SnTr.WBegF) / 100L;
	if (Offset == 0)
		Offset = 1;
	Offset *= prc;
	return(Offset);
}
/*
#define maxloop 1000
- (IBAction)startTask:(id)sender {
#pragma unused (sender)
	// Prepare sheet and show it...
	NSRect sheetRect = NSMakeRect(0, 0, 400, 114);

	NSWindow *progSheet = [[NSWindow alloc] initWithContentRect:sheetRect
													  styleMask:NSTitledWindowMask
														backing:NSBackingStoreBuffered
														  defer:YES];

	NSView *contentView = [[NSView alloc] initWithFrame:sheetRect];

	NSProgressIndicator *progInd = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(143, 74, 239, 20)];

	NSTextField *label = [[NSTextField alloc] initWithFrame:NSMakeRect(145, 48, 235, 22)];

	NSButton *cancelButton = [[NSButton alloc] initWithFrame:NSMakeRect(304, 12, 82, 32)];
	cancelButton.bezelStyle = NSRoundedBezelStyle;
	cancelButton.title = @"Cancel";
	cancelButton.action = @selector(cancelTask:);
	cancelButton.target = self;

	[contentView addSubview:progInd];
	[contentView addSubview:label];
	[contentView addSubview:cancelButton];

	[progSheet setContentView:contentView];


	[NSApp beginSheet:progSheet
	   modalForWindow:self.window
		modalDelegate:nil
	   didEndSelector:NULL
		  contextInfo:NULL];

	[progSheet makeKeyAndOrderFront:self];

	[progInd setIndeterminate:NO];
	[progInd setDoubleValue:0.f];
	[progInd startAnimation:self];


	// Start computation using GCD...

	dispatch_async(dispatch_get_global_queue(0, 0), ^{

		for (int i = 0; i < maxloop; i++) {

			[NSThread sleepForTimeInterval:0.01];

			if (breakLoop)
				break;

			// Update the progress bar which is in the sheet:
			dispatch_async(dispatch_get_main_queue(), ^{
				[progInd setDoubleValue: (double)i/maxloop * 100];
			});
		}


		// Calculation finished, remove sheet on main thread

		dispatch_async(dispatch_get_main_queue(), ^{
			[progInd setIndeterminate:YES];

			[NSApp endSheet:progSheet];
			[progSheet orderOut:self];
		});
	});
}

- (IBAction)cancelTask:(id)sender {
#pragma unused (sender)
	NSLog(@"Cancelling");
	breakLoop = YES;
}
*/
- (BOOL)readMedia:(BOOL)isFirtTime {
	BOOL isShowProgressBar;
	FNType *FNameOffset;
	NSWindow *win = [self window];
	NSRect wFrame = [win frame];
	wFrame.size.width = wFrame.size.width / 5 * 4;
	wFrame.size.height =  60;
	contentView = nil;
	label = nil;
	progInd = nil;
	progSheet = nil;
	if (SnTr.SoundFileSize > 300000)
		isShowProgressBar = true;
	else
		isShowProgressBar = false;

	if (isShowProgressBar) {
		progSheet = [[NSWindow alloc] initWithContentRect:wFrame
												styleMask:NSTitledWindowMask
												  backing:NSBackingStoreBuffered
													defer:YES
					 ];
		contentView = [[NSView alloc] initWithFrame:wFrame];

		progInd = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(wFrame.origin.x, wFrame.size.height-50, wFrame.size.width, 20)];
		[progInd setIndeterminate:NO];
		[progInd setDoubleValue:0.f];
		[progInd startAnimation:self];

		label = [[NSTextField alloc] initWithFrame:NSMakeRect(wFrame.origin.x, wFrame.size.height-30, wFrame.size.width, 22)];
		FNameOffset = strrchr(SnTr.mediaFPath, '/');
		if (FNameOffset != NULL)
			FNameOffset++;
		else
			FNameOffset = SnTr.mediaFPath;
		sprintf(templineC4, "Reading file: %s", FNameOffset);
		[label setStringValue:[NSString stringWithUTF8String:templineC4]];
		[label setBezeled:NO];
		[label setDrawsBackground:NO];
		[label setEditable:NO];
		[label setSelectable:NO];

		[contentView addSubview:progInd];
		[contentView addSubview:label];
		[progSheet setContentView:contentView];

//		[NSApp beginSheet:progSheet modalForWindow:self.window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
		[win beginSheet:progSheet completionHandler:nil];
		[progSheet makeKeyAndOrderFront:self];
	}

	dispatch_async(dispatch_get_global_queue(0, 0), ^{
		BOOL isRead;
		UInt32 i, pixelCnt, sampleCount, progress, oldProgress;
		Float32 val, hp1, hp2, winHeight;
//		Float32 scale;
		NSInteger sampleTally, samplesPerPixel;
		NSError * error = nil;
		AVAssetReader *reader;

		AVURLAsset *songAsset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:SnTr.mediaFPath]] options:nil];
		reader = [[AVAssetReader alloc] initWithAsset:songAsset error:&error];
		AVAssetTrack *songTrack = [songAsset.tracks objectAtIndex:0];

		NSDictionary* outputSettingsDict = [[NSDictionary alloc] initWithObjectsAndKeys:
											[NSNumber numberWithInt:kAudioFormatLinearPCM],AVFormatIDKey,
//											[NSNumber numberWithInt:44100.0],AVSampleRateKey, // Not Supported
//2021-04-25										[NSNumber numberWithInt:8000.0],AVSampleRateKey, // Not Supported
//											[NSNumber numberWithInt: 2],AVNumberOfChannelsKey,// Not Supported
											[NSNumber numberWithInt:16],AVLinearPCMBitDepthKey,
											[NSNumber numberWithBool:NO],AVLinearPCMIsBigEndianKey,
											[NSNumber numberWithBool:NO],AVLinearPCMIsFloatKey,
											[NSNumber numberWithBool:NO],AVLinearPCMIsNonInterleaved,
											nil];
		AVAssetReaderTrackOutput* output = [[AVAssetReaderTrackOutput alloc] initWithTrack:songTrack outputSettings:outputSettingsDict];
		[reader addOutput:output];
		[output release];

		UInt32 bytesPerSample = 2 * SnTr.SNDchan;
//		reader.timeRange = CMTimeRangeMake(CMTimeMake((int64_t)[self conv_to_msec_rep:SnTr.WBegF], 1000), kCMTimePositiveInfinity);
		reader.timeRange = CMTimeRangeMake(kCMTimeZero, [songAsset duration]);
		// CMTimeRangeMake(kCMTimeZero, [songAsset duration])
		[reader startReading];

		sampleTally = 0;
		samplesPerPixel = SnTr.HScale / SnTr.SNDchan;

		oldProgress = 0;
		winHeight = ([lowerView getRowsCnt] * lowerView->lowerFontHeight) / (Float32)SnTr.SNDchan;
//		scale = (int)((0xFFFFL / SnTr.VScale) / winHeight) + 1;
		pixelCnt = 0;
		hp1 = 0.0;
		hp2 = 0.0;
		isRead = false;
		while (reader.status == AVAssetReaderStatusReading) {
			AVAssetReaderTrackOutput * trackOutput = (AVAssetReaderTrackOutput *)[reader.outputs objectAtIndex:0];
			CMSampleBufferRef sampleBufferRef = [trackOutput copyNextSampleBuffer];
			if (sampleBufferRef) {
				CMBlockBufferRef blockBufferRef = CMSampleBufferGetDataBuffer(sampleBufferRef);
				size_t length = CMBlockBufferGetDataLength(blockBufferRef);
				NSMutableData * data = [NSMutableData dataWithLength:length];
				CMBlockBufferCopyDataBytes(blockBufferRef, 0, length, data.mutableBytes);
				SInt16 *samples = (SInt16 *)data.mutableBytes;
				sampleCount = length / bytesPerSample;
				for (i=0; i < sampleCount ; i++) {
					val = (Float32)*samples++;
//					val = val / scale;
					if (val > hp1)
						hp1 = val;
					if (SnTr.SNDchan==2) {
						val = (Float32)*samples++;
//						val = val / scale;
						if (val > hp2)
							hp2 = val;
					}
					sampleTally++;
					if (sampleTally > samplesPerPixel) {
						SnTr.TopP1[pixelCnt] = hp1;
						if (SnTr.SNDchan == 2) {
							if (!doMixedSTWave)
								SnTr.TopP2[pixelCnt] = hp2;
							else if (hp2 > hp1)
								SnTr.TopP1[pixelCnt] = hp2;
						}
						sampleTally = 0;
						hp1 = 0.0;
						hp2 = 0.0;
						pixelCnt++;
						if (pixelCnt >= SnTr.SoundFileSize)
							break;
					}
					progress = (UInt32)((double)pixelCnt/SnTr.SoundFileSize * 100.0000);
					if (progress > oldProgress && isShowProgressBar) {
						oldProgress = progress;
						// Update the progress bar which is in the sheet:
						dispatch_async(dispatch_get_main_queue(), ^{
							[progInd setDoubleValue:(double)progress];
						});
					}
				}
				CMSampleBufferInvalidate(sampleBufferRef);
				CFRelease(sampleBufferRef);
				if (pixelCnt >= SnTr.SoundFileSize)
					break;
			}
		}

		if (isShowProgressBar) {
			dispatch_async(dispatch_get_main_queue(), ^{
				[progInd setIndeterminate:YES];
				[NSApp endSheet:progSheet];
				[progSheet orderOut:self];

				[progSheet release];
				[contentView release];
				[progInd release];
				[label release];
				progSheet = nil;
				contentView = nil;
				progInd = nil;
				label = nil;
			});
		}

		if (pixelCnt < SnTr.SoundFileSize && sampleTally > 0) {
			SnTr.TopP1[pixelCnt] = hp1;
			if (SnTr.SNDchan == 2) {
				if (!doMixedSTWave)
					SnTr.TopP2[pixelCnt] = hp2;
				else if (hp2 > hp1)
					SnTr.TopP1[pixelCnt] = hp2;
			}
			pixelCnt++;
		}
		for (i=pixelCnt; i < SnTr.SoundFileSize; i++) {
			SnTr.TopP1[i] = -1;
		}
		if (SnTr.SoundFileSize > pixelCnt)
			SnTr.SoundFileSize = pixelCnt;

		if (reader.status == AVAssetReaderStatusFailed || reader.status == AVAssetReaderStatusUnknown){
			do_warning_sheet("Failed to read media file.", win);
		} else if (pixelCnt >= SnTr.SoundFileSize || reader.status == AVAssetReaderStatusCompleted) {
			isRead = true;
			dispatch_async(dispatch_get_main_queue(), ^{
				[self setSliderMinNum:(double)0 maxNum:(double)(SnTr.SoundFileSize - SnTr.WaveWidth)];
			});
		}
		[reader release];
		if (isFirtTime) {
			if (isRead) {
				if (SnTr.EndF >= SnTr.SoundFileSize)
					SnTr.EndF = SnTr.SoundFileSize;
				if (SnTr.BegF >= SnTr.SoundFileSize) {
					do_warning_sheet("The begin bullet time is greater than media file.", win);
					SnTr.BegF = SnTr.EndF - 1;
				}
				[self DisplayEndF];
				dispatch_async(dispatch_get_main_queue(), ^{
					NSTextView *textView;
					NSRange cursorRange;

					[self setSliderHidden:NO];
					SnTr.IsSoundOn = true;
					[self setupWindowForDocument];
					textView = [self firstTextView];
					cursorRange = [textView selectedRange];
					[textView setSelectedRange:cursorRange];
					[textView scrollRangeToVisible:cursorRange];
				});
			} else {
				[lowerView setRowsCnt:2];
			}
		} else {
			[self DisplayEndF];
			[lowerView setNeedsDisplay:YES];
		}
	});
	return(true);
}

- (void)DisplayEndF {
	if ((SnTr.EndF < SnTr.WBegF || SnTr.EndF > SnTr.WEndF) && SnTr.EndF != 0 && SnTr.EndF != SnTr.BegF) {
		SnTr.WBegF = (SnTr.EndF - [self ComputeOffset:90L]);
		if (SnTr.WBegF < 1)
			SnTr.WBegF = 1;
		SnTr.WEndF = SnTr.WBegF + SnTr.WaveWidth;
	} else if ((SnTr.EndF == 0 || SnTr.BegF == SnTr.EndF) && (SnTr.BegF < SnTr.WBegF || SnTr.BegF > SnTr.WEndF)) {
		SnTr.WBegF = SnTr.BegF - [self ComputeOffset:15L];
		if (SnTr.WBegF < 1)
			SnTr.WBegF = 1;
		SnTr.WEndF = SnTr.WBegF + SnTr.WaveWidth;
	}
	if (SnTr.WEndF > SnTr.SoundFileSize && SnTr.SoundFileSize - SnTr.WaveWidth >= 0) {
		SnTr.WEndF = SnTr.SoundFileSize;
		SnTr.WBegF = SnTr.WEndF - SnTr.WaveWidth;
	}
}

- (UInt32)ControlWidth:(LowerTextView *)lowerView {
	UInt32 maxWidth;
	NSString *string;
	NSFont *font;
	CGSize HVWidth;
	NSDictionary *attsW;

	font = lowerView->lowerWinFont;
	if (font == nil) {
		return(0);
	}
	attsW = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor whiteColor], NSLigatureAttributeName:@0};
	if (attsW == nil) {
		return(0);
	}
	maxWidth = 0;
	string = @"+H";
	HVWidth = [string sizeWithAttributes:attsW];
	if (HVWidth.width > maxWidth) {
		maxWidth = HVWidth.width;
	}
	string = @"S";
	HVWidth = [string sizeWithAttributes:attsW];
	if (HVWidth.width > maxWidth) {
		maxWidth = HVWidth.width;
	}
	string = @"-H";
	HVWidth = [string sizeWithAttributes:attsW];
	if (HVWidth.width > maxWidth) {
		maxWidth = HVWidth.width;
	}
	string = @"+V";
	HVWidth = [string sizeWithAttributes:attsW];
	if (HVWidth.width > maxWidth) {
		maxWidth = HVWidth.width;
	}
	string = @"-V";
	HVWidth = [string sizeWithAttributes:attsW];
	if (HVWidth.width > maxWidth) {
		maxWidth = HVWidth.width;
	}

	return(maxWidth);
}

- (void)soundwindow {
	CGFloat fwidth;
	long rows;
	UInt32 i;
	NSUInteger curLowerViewRows;
	AVURLAsset *songAsset;
	AVAssetTrack *songTrack;
	NSArray* formatDesc;
	CMAudioFormatDescriptionRef item;
	const AudioStreamBasicDescription* fmtDesc;
	NSRect lowerViewFrame = [lowerView frame];

	SnTr.ctrlWidth = [self ControlWidth:lowerView];
	fwidth = lowerViewFrame.size.width - (SnTr.ctrlWidth * 2);
	SnTr.WaveWidth = (UInt32)fwidth;

	strcpy(SnTr.mediaFPath, theAVinfo.mediaFPath);
	songAsset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:SnTr.mediaFPath]] options:nil];
	songTrack = [songAsset.tracks objectAtIndex:0];

	formatDesc = songTrack.formatDescriptions;
	for (i=0; i < [formatDesc count]; i++) {
		item = (CMAudioFormatDescriptionRef)[formatDesc objectAtIndex:i];
		fmtDesc = CMAudioFormatDescriptionGetStreamBasicDescription(item);
		if (fmtDesc) {
			SnTr.SNDformat = fmtDesc->mFormatID;
			SnTr.SNDrate = fmtDesc->mSampleRate; // 8000.0; //2021-04-25
			SnTr.mSampleRate = fmtDesc->mSampleRate;
			SnTr.SNDchan = fmtDesc->mChannelsPerFrame;
			SnTr.SNDsample = 2;
			SnTr.mBitsPerChannel = fmtDesc->mBitsPerChannel;
			NSLog(@"channels:%u, bytes/packet: %u, sampleRate %f",fmtDesc->mChannelsPerFrame, fmtDesc->mBytesPerPacket,fmtDesc->mSampleRate);
		}
	}

	curLowerViewRows = [lowerView getRowsCnt];
	if (doMixedSTWave) {
		rows = 5;
	} else {
		rows = SnTr.SNDchan * 5;
	}
	rows += 2;
	[lowerView setRowsCnt:rows];

	if (SnTr.HScale <= 0L) {
		SnTr.HScale = 128L * SnTr.SNDsample * SnTr.SNDchan; //2021-04-25
	}

	CMTime duration = [songAsset duration];
	double tdv = (double)duration.value;
	double tds = (double)duration.timescale;
	SnTr.SoundFileSize = [self conv_from_msec_rep:((tdv / tds) * 1000.0000)];

	SnTr.TopP1 = (Float32 *)malloc((SnTr.SoundFileSize+1) * sizeof(Float32));
	if (SnTr.SNDchan == 2 && !doMixedSTWave)
		SnTr.TopP2 = (Float32 *)malloc((SnTr.SoundFileSize+1) * sizeof(Float32));
	else
		SnTr.TopP2 = NULL;

	if (SnTr.WaveWidth >= (UInt32)SnTr.SoundFileSize) {
		SnTr.HScale = 128L * SnTr.SNDsample * SnTr.SNDchan; //2021-04-25
	}

	SnTr.WBegF = 0L;
	SnTr.WEndF = SnTr.WBegF + SnTr.WaveWidth;
	if (SnTr.VScale < 1)
		SnTr.VScale = 1;


	if (theAVinfo.beg == -1) {
		SnTr.BegF = 0L;
	} else {
		SnTr.BegF = [self conv_from_msec_rep:theAVinfo.beg];
	}
	if (theAVinfo.end == -1) {
		SnTr.EndF = 0L;
	} else {
		SnTr.EndF = [self conv_from_msec_rep:theAVinfo.end];
	}

	[self readMedia:true];
}

- (IBAction)SonicMode:(id)sender { // 2020-11-13 beg
#pragma unused (sender)
	char *bufEnd;
	unichar ch;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSRange cursorRange;
	NSUInteger pos, len;
	Document *cDoc;
	BOOL isFoundAV;

	textView = [self firstTextView];
	if (SnTr.IsSoundOn == false) {
		cDoc = [self document];
		if (cDoc == nil)
			return;
		if (EditorMode == false) {
			do_warning_sheet("Illegal in coder mode", [self window]);
			return;
		} else if ([cDoc isReadOnly]) {
			do_warning_sheet("Illegal in this window", [self window]);
			return;
		}
		text = [textView textStorage];
		textSt = [text string];
		cursorRange = [textView selectedRange];
		len = [text length];

		pos = cursorRange.location;
		if (pos >= len)
			pos = len - 1;
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos--;
		}
		theAVinfo.mediaFName[0] = EOS;
		isFoundAV = [self isFindBullet:textSt Index:&pos MaxLen:len AVInformation:&theAVinfo];
		if (theAVinfo.mediaFName[0] == EOS) {
			if ([self getCurrentMediaName:textSt MaxLen:len showErr:true] == false) {
				return;
			}
			strcpy(theAVinfo.mediaFName, cDoc->mediaFileName);
		}
		if (cDoc->filePath[0] != EOS) {
			strcpy(theAVinfo.mediaFPath, cDoc->filePath);
			bufEnd = strrchr(theAVinfo.mediaFPath, '/');
			if (bufEnd != NULL) {
				*(bufEnd+1) = EOS;
			}
			[self findMediaFile:false];
		} else
			do_warning_sheet("Can't determine document's file path", [self window]);
	} else {
		[self DisposeOfSoundData];
		[lowerView setRowsCnt:2];
		[self setupWindowForDocument];
		cursorRange = [textView selectedRange];
		dispatch_async(dispatch_get_main_queue(), ^{
			[textView setSelectedRange:cursorRange];
			[textView scrollRangeToVisible:cursorRange];
		});
	}
} // 2020-11-13 end

@end

@implementation DocumentWindowController(Delegation)

/* Window delegation messages */

- (void)windowDidExpose:(NSNotification *)notification {
#pragma unused (notification)

}

- (void)windowDidBecomeKey:(NSNotification *)notification {
#pragma unused (notification)
	Document *cDoc = [self document];
	if ([cDoc get_wID] == DOCWIN) {
		gLastTextWinController = self;
	}
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
#pragma unused (notification)
	Document *cDoc = [self document];
	if ([cDoc get_wID] == DOCWIN) {
		gLastTextWinController = self;
	}
}

- (void)windowDidUpdate:(NSNotification *)notification {
#pragma unused (notification)

}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)defaultFrame {
#pragma unused (window)
	return defaultFrame;
}

- (void)windowWillResize:(NSNotification *)notification {
#pragma unused (notification)
	if (!isSettingSize) {   // There is potential for recursion, but typically this is prevented in NSWindow which doesn't call this method if the frame doesn't change. However, just in case...
		isSettingSize = YES;
		NSSize viewSize = [[scrollView class] contentSizeForFrameSize:[scrollView frame].size hasHorizontalScroller:[scrollView hasHorizontalScroller] hasVerticalScroller:[scrollView hasVerticalScroller] borderType:[scrollView borderType]];

		viewSize.width -= (defaultTextPadding() * 2.0);
		[[self document] setViewSize:viewSize];
		isSettingSize = NO;
	}
}

- (void)windowDidResize:(NSNotification *)notification {
#pragma unused (notification)
	id scrollBar;
	BOOL hasHScroller, hasVScroller;
	long lowerRowsOffset;
	CGFloat tFontHeight = [[self document] fontHeight];
	CGFloat titleBarHeight;
	NSScrollerStyle scrollerStyle;
	NSRect scrollFrame, lowerFrame;
	NSSize viewSize;
	NSWindow *window = [self window];
	NSRect wFrame = [window frame];
	Document *cDoc = [self document];
	unsigned short wID;

	wID = [cDoc get_wID];
	if (wID == CLANWIN) {// 2020-05-14
		if (ClanOutSize.top != wFrame.origin.y || ClanOutSize.left != wFrame.origin.x ||
			ClanOutSize.height != wFrame.size.height || ClanOutSize.width != wFrame.size.width) {
			ClanOutSize.top = wFrame.origin.y;
			ClanOutSize.left = wFrame.origin.x;
			ClanOutSize.height = wFrame.size.height;
			ClanOutSize.width = wFrame.size.width;
			WriteCedPreference();
		}
	} else if (wID == SpChrWIN) {// 2020-05-14
		if (SpCharsWinSize.top != wFrame.origin.y || SpCharsWinSize.left != wFrame.origin.x ||
			SpCharsWinSize.height != wFrame.size.height || SpCharsWinSize.width != wFrame.size.width) {
			SpCharsWinSize.top = wFrame.origin.y;
			SpCharsWinSize.left = wFrame.origin.x;
			SpCharsWinSize.height = wFrame.size.height;
			SpCharsWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
	} else if ([cDoc fileURL] == nil && wID != CLANWIN && wID != SpChrWIN && wID != KeysLWIN) {
		if (defWinSize.top != wFrame.origin.y || defWinSize.left != wFrame.origin.x ||
			defWinSize.height != wFrame.size.height || defWinSize.width != wFrame.size.width) {
			lowerRowsOffset = (long)(lowerView->lowerFontHeight * (lowerView->rowsNum-2));
			defWinSize.top = wFrame.origin.y;
			defWinSize.left = wFrame.origin.x;
			defWinSize.height = wFrame.size.height;
			defWinSize.width = wFrame.size.width;
			if (lowerView->rowsNum > 2) {
				defWinSize.height = defWinSize.height - lowerRowsOffset;
				defWinSize.top = defWinSize.top + lowerRowsOffset;
			}
			WriteCedPreference();
		}
	} else {
		cDoc->top = wFrame.origin.y;
		cDoc->left = wFrame.origin.x;
		cDoc->height = wFrame.size.height;
		cDoc->width = wFrame.size.width;
	}
	if (!isSettingSize) {   // There is potential for recursion, but typically this is prevented in NSWindow which doesn't call this method if the frame doesn't change. However, just in case...
		isSettingSize = YES;
		scrollBar = [scrollView class];
		hasHScroller = [scrollView hasHorizontalScroller];
		hasVScroller = [scrollView hasVerticalScroller];
		scrollerStyle = [NSScroller preferredScrollerStyle];
		scrollFrame = [scrollView frame];
		scrollFrame.origin.y = (tFontHeight * [lowerView getRowsCnt]);
		titleBarHeight = wFrame.size.height - [window contentRectForFrameRect:wFrame].size.height;
		scrollFrame.size.height = wFrame.size.height - (tFontHeight * [lowerView getRowsCnt]) - titleBarHeight;
		[scrollView setFrameOrigin:scrollFrame.origin];
		[scrollView setFrameSize:scrollFrame.size];
//		scrollFrame = [scrollView frame];
		viewSize = [scrollBar contentSizeForFrameSize:scrollFrame.size
					horizontalScrollerClass:hasHScroller ? [NSScroller class] : Nil
					verticalScrollerClass: hasVScroller? [NSScroller class] : Nil
					borderType:[scrollView borderType]
					controlSize:NSRegularControlSize
					scrollerStyle:scrollerStyle];
//		viewSize = [scrollBar contentSizeForFrameSize:scrollFrame.size hasHorizontalScroller:hasHScroller hasVerticalScroller:hasVScroller borderType:[scrollView borderType]];
		viewSize.width -= (defaultTextPadding() * 2.0);

		[cDoc setViewSize:viewSize];

//		lowerFrame = [lowerView frame];
		lowerFrame.origin.y = 0;
		lowerFrame.origin.x = 0;
		lowerFrame.size.height = (tFontHeight * [lowerView getRowsCnt]);
		lowerFrame.size.width = wFrame.size.width;
		[lowerView setFrameOrigin:lowerFrame.origin];
		[lowerView setFrameSize:lowerFrame.size];
		if (SnTr.IsSoundOn == true) {
			SnTr.WaveWidth = (UInt32)(lowerFrame.size.width - (SnTr.ctrlWidth * 2));
			SnTr.WEndF = SnTr.WBegF + SnTr.WaveWidth;
			[self DisplayEndF];
			[lowerView setNeedsDisplay:YES];
		}
		isSettingSize = NO;
	}
}

- (void)windowDidMove:(NSNotification *)notification {
#pragma unused (notification)
	NSRect wFrame = [[self window] frame];
	Document *cDoc = [self document];
	long lowerRowsOffset;
	unsigned short wID;

	wID = [cDoc get_wID];
	if (wID == CLANWIN) {// 2020-05-14
		if (ClanOutSize.top != wFrame.origin.y || ClanOutSize.left != wFrame.origin.x ||
			ClanOutSize.height != wFrame.size.height || ClanOutSize.width != wFrame.size.width) {
			ClanOutSize.top = wFrame.origin.y;
			ClanOutSize.left = wFrame.origin.x;
			ClanOutSize.height = wFrame.size.height;
			ClanOutSize.width = wFrame.size.width;
			WriteCedPreference();
		}
	} else if (wID == SpChrWIN) {// 2020-05-14
		if (SpCharsWinSize.top != wFrame.origin.y || SpCharsWinSize.left != wFrame.origin.x ||
			SpCharsWinSize.height != wFrame.size.height || SpCharsWinSize.width != wFrame.size.width) {
			SpCharsWinSize.top = wFrame.origin.y;
			SpCharsWinSize.left = wFrame.origin.x;
			SpCharsWinSize.height = wFrame.size.height;
			SpCharsWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
	} else if ([cDoc fileURL] == nil && wID != CLANWIN && wID != SpChrWIN && wID != KeysLWIN) {
		if (defWinSize.top != wFrame.origin.y || defWinSize.left != wFrame.origin.x ||
			defWinSize.height != wFrame.size.height || defWinSize.width != wFrame.size.width) {
			lowerRowsOffset = (long)(lowerView->lowerFontHeight * (lowerView->rowsNum-2));
			defWinSize.top = wFrame.origin.y;
			defWinSize.left = wFrame.origin.x;
			defWinSize.height = wFrame.size.height;
			defWinSize.width = wFrame.size.width;
			if (lowerView->rowsNum > 2) {
				defWinSize.height = defWinSize.height - lowerRowsOffset;
				defWinSize.top = defWinSize.top + lowerRowsOffset;
			}
			WriteCedPreference();
		}
	} else {
		cDoc->top = wFrame.origin.y;
		cDoc->left = wFrame.origin.x;
		cDoc->height = wFrame.size.height;
		cDoc->width = wFrame.size.width;
	}
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)
	NSLog(@"DocumentWinController: windowWillClose\n");
	[self DisposeOfSoundData];
	if (self == gLastTextWinController)
		gLastTextWinController = nil;
	if (docWinEventMonitor) {
		[NSEvent removeMonitor:docWinEventMonitor];
		docWinEventMonitor = nil;
	}
	FreeCodesMem(self);
}

/* Text view delegation messages */
- (void)textViewDidChangeSelection:(NSNotification *)notification {
#pragma unused (notification)
// 2020-05-06 2020-05-07 beg
	NSValue *cursorValue;
	NSRange endRange, cursorRange;
	NSFont *font, *oldFont;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;
	NSUInteger cursorPos, nCursorPoint, cursorLine;
	NSUInteger len, indexG;
	NSLayoutManager *tLayoutMgr;
	NSGlyphProperty prop;
	CGFloat oldFontHeight, newFontHeight;
	Document *cDoc;

	textView = [self firstTextView];
	if (textView == nil)
		return;
	text = [textView textStorage];
	if (text == nil)
		return;
	len = [text length];
	textSt = [text string];
	cursorValue = [[textView selectedRanges] objectAtIndex:0];
	cursorRange = [cursorValue rangeValue];
	cursorPos = cursorRange.location;
/*
	cursorLine = 1;
	for (index=0; index < cursorPos; index++) {
		if ([textSt characterAtIndex:index] == '\n')
			cursorLine++;
	}
*/

	id isUnderlined; // 2021-07-14 beg
	NSInteger val;
	NSColor *color;
	if (cursorPos > 1 && isAddingChar) {
		endRange = NSMakeRange(cursorPos-1, 1);              // @(NSUnderlineStyleThick)
		isUnderlined = [text attribute:NSUnderlineStyleAttributeName
							   atIndex:cursorPos-2 effectiveRange:nil];
		val = [isUnderlined integerValue];
		if (val != 0) {
			[text addAttribute:NSUnderlineStyleAttributeName value:@(NSUnderlineStyleSingle)
						 range:endRange];
		}
		color = [text attribute:NSForegroundColorAttributeName 
						atIndex:cursorPos-2 effectiveRange:nil];
		if (color == [NSColor redColor]) {
			[text addAttribute:NSForegroundColorAttributeName value:[NSColor redColor] range:endRange];
		}

	} // 2021-07-14 end
	isAddingChar = false;
	if (cursorPos > 0) {
		if (ShowParags == NO && cursorRange.length == 0 &&
			(cursorPos < len || cursorKeyDir < 0)) {// 2020-05-07
			for (tLayoutMgr in [text layoutManagers]) {
				nCursorPoint = cursorPos;
				while (YES) {
//					uc = [textSt characterAtIndex:(nCursorPoint)];
					indexG = [tLayoutMgr glyphIndexForCharacterAtIndex:nCursorPoint];
					prop = [tLayoutMgr propertyForGlyphAtIndex:indexG];
					if (prop == NSGlyphPropertyNull) {
						if (cursorKeyDir < 0)
							nCursorPoint--;
						else
							nCursorPoint++;
					} else {
						break;
					}
				}
				if (nCursorPoint != cursorPos) {
					cursorRange.location = nCursorPoint;
					[textView setSelectedRange:cursorRange];
				}
			}
		}
	}
	cursorLine = [self countLineNumbers:cursorPos];
	[self setStatusLine:cursorLine extraMess:ced_err_message isUpdateDisplay:YES];
	ced_err_message[0] = EOS;

	cDoc = [self document];
	if (cDoc == nil)
		return;
	oldFontHeight = [cDoc fontHeight]; // 2020-09-24 beg
	if ([cDoc docFont] == nil) {
		if (cDoc->isCAFont)
			[cDoc setDocFont:defCAFont];
		else
			[cDoc setDocFont:defUniFont];
	}
	font = [cDoc docFont];
	text = [cDoc textStorage];
	if (text != nil) {// 2021-03-04
		oldFont = text.font;
		if (oldFont == nil || oldFont != font)
			text.font = font;
	}
	newFontHeight = [[self myLayoutManager] defaultLineHeightForFont:font];
	if (oldFontHeight != newFontHeight) {
		[cDoc setDocFont:font];
		[cDoc setFontHeight:newFontHeight];
		[lowerView setLowerFont:font];
		[lowerView setLowerFontHeight:newFontHeight];

		[self setupWindowForDocument];
	} // 2020-09-24 end

// 2020-05-06 2020-05-07 end
}

/* Layout manager delegation message */

- (NSUInteger)layoutManager:(NSLayoutManager *)lLayoutManager
	   shouldGenerateGlyphs:(const CGGlyph *)glyphs
				 properties:(const NSGlyphProperty *)props
		   characterIndexes:(const NSUInteger *)charIndexes
					   font:(NSFont *)aFont
			  forGlyphRange:(NSRange)glyphRange {
	BOOL hideBullet;
	unichar ch;
	NSString *utf16CodeUnits;
	NSUInteger layoutI, textI, len;
	NSTextStorage *myTextStorage;
	NSGlyphProperty modProps[glyphRange.length];

	// First, make sure we'll be able to access the NSTextStorage.
	myTextStorage = [lLayoutManager textStorage];
	if (myTextStorage == nil) {
		NSLog(@"No textStorage was associated to this layoutManager");
	}
	// Access the characters.
	utf16CodeUnits = [myTextStorage string];
	len = [utf16CodeUnits length];
	textI = charIndexes[0]; // glyphRange.location;
	hideBullet = [self isWithinHidenRange:utf16CodeUnits index:textI length:len];
	for (layoutI=0; layoutI < glyphRange.length && textI < len; layoutI++) {
		textI = charIndexes[layoutI]; // glyphRange.location;
		ch = [utf16CodeUnits characterAtIndex:textI];
		textI++;

		if (ShowParags == YES) { // 2019-11-08 2019-11-21 2019-11-26 2020-05-07
			modProps[layoutI] = props[layoutI];
		} else {
			if (hideBullet == YES) {
				modProps[layoutI] = NSGlyphPropertyNull;
			} else {
				modProps[layoutI] = props[layoutI];
			}
			if (ch == '\n') {
				if (hideBullet == YES)
					hideBullet = NO;
			} else if (ch == 0x2022) {
				if (hideBullet == YES)
					hideBullet = NO;
				else
					hideBullet = [self isShouldBeHiden:utf16CodeUnits index:textI length:len];
			}
		}
	}

//	[lLayoutManager setGlyphs:glyphs properties:props characterIndexes:charIndexes font:aFont forGlyphRange:glyphRange];
	[lLayoutManager setGlyphs:glyphs properties:modProps characterIndexes:charIndexes font:aFont forGlyphRange:glyphRange];

	return glyphRange.length; // 0 - for default
}

-(void)windowDidChangeOcclusionState:(NSNotification *)notification {
	int j;
	NSWindow *win = notification.object;

	if (win.occlusionState & NSWindowOcclusionStateVisible) {
		j = 0;
	} else {
		j = 0;
	}
}

- (void)layoutManager:(NSLayoutManager *)layoutManager didCompleteLayoutForTextContainer:(NSTextContainer *)textContainer atEnd:(BOOL)layoutFinishedFlag {
#pragma unused (layoutManager, textContainer, layoutFinishedFlag)
	BOOL j;
//	NSWindow *win;
//	NSTextView *textView;

	j = YES;
//	win = [self window];
//	[win disableScreenUpdatesUntilFlush];
//	[win displayIfNeeded];
//	win.viewsNeedDisplay = YES;
//	[win display];

//	textView = [textContainer textView];
//	[textView setNeedsDisplay:YES];
//	[textView displayIfNeeded];
//	[textView layoutSubtreeIfNeeded];
//	textView.needsDisplay = YES;
//	[textView display];

//	j = [win viewsNeedDisplay];
//	j = [win allowsConcurrentViewDrawing];

//	CALayer *layer = textView.layer;
//	[layer setNeedsDisplay];
//	[layer displayIfNeeded];

}

- (void)textViewDidChangeTypingAttributes:(NSNotification *)notification { // 2020-09-24 beg
	Document *cDoc = [self document];
	NSTextStorage *text;
	NSFont *font, *oldFont;
	CGFloat oldFontHeight, newFontHeight;
#pragma unused (notification)

	if (cDoc == nil)
		return;
	oldFontHeight = [cDoc fontHeight];
	text = [cDoc textStorage];
//	if (text == nil)
//		return;
//	font = [text font];
//	if (font == nil)
//		return;
	if ([cDoc docFont] == nil) {
		if (cDoc->isCAFont)
			[cDoc setDocFont:defCAFont];
		else
			[cDoc setDocFont:defUniFont];
	}
	font = [cDoc docFont];
	if (text != nil) {// 2021-03-04
		oldFont = text.font;
		if (oldFont == nil || oldFont != font)
			text.font = font;
	}
	newFontHeight = [[self myLayoutManager] defaultLineHeightForFont:font];
	if (oldFontHeight != newFontHeight) {
		[cDoc setDocFont:font];
		[cDoc setFontHeight:newFontHeight];
		[lowerView setLowerFont:font];
		[lowerView setLowerFontHeight:newFontHeight];

		[self setupWindowForDocument];
	}
}// 2020-09-24 end

- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link atIndex:(NSUInteger)charIndex {
#pragma unused (charIndex, textView)
	NSURL *linkURL = nil;

	if ([link isKindOfClass:[NSURL class]]) {	// Handle NSURL links
		linkURL = link;
	} else if ([link isKindOfClass:[NSString class]]) {	// Handle NSString links
		linkURL = [NSURL URLWithString:link relativeToURL:[[self document] fileURL]];
	}
	if (linkURL) {
		NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
		if ([linkURL isFileURL]) {
			NSError *error;
			if (![linkURL checkResourceIsReachableAndReturnError:&error]) {	// To be able to present an error panel, see if the file is reachable
				[[self document] performActivityWithSynchronousWaiting:YES usingBlock:^(void (^activityCompletionHandler)(void)) {
					[[self window] presentError:error modalForWindow:[self window] delegate:self didPresentSelector:@selector(presenterDidPresent:soContinue:) contextInfo:Block_copy(^(BOOL didSucceed) {
						if (didSucceed) {
						}
						activityCompletionHandler();
					})];
				}];
				return YES;
			} else {
				// Special case: We want to open text types in TextEdit, as presumably that is what was desired
				NSString *typeIdentifier = nil;
				if ([linkURL getResourceValue:&typeIdentifier forKey:NSURLTypeIdentifierKey error:NULL] && typeIdentifier) {
					BOOL openInTextEdit = NO;
					for (NSString *textTypeIdentifier in [NSAttributedString textTypes]) {
						if ([workspace type:typeIdentifier conformsToType:textTypeIdentifier]) {
							openInTextEdit = YES;
							break;
						}
					}
					if (openInTextEdit) {
						if ([[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:linkURL display:YES error:NULL]) return YES;
					}
				}
				// Other file URLs are displayed in Finder
				[workspace activateFileViewerSelectingURLs:[NSArray arrayWithObject:linkURL]];
				return YES;
			}
		} else {
			// Other URLs are simply opened
			if ([workspace openURL:linkURL]) return YES;
		}
	}

	// We only get here on failure... Because we beep, we return YES to indicate "success", so the text system does no further processing.
	NSBeep();
    return YES;
}

- (NSURL *)textView:(NSTextView *)textView URLForContentsOfTextAttachment:(NSTextAttachment *)textAttachment atIndex:(NSUInteger)charIndex {
#pragma unused (charIndex, textView)
    NSURL *attachmentURL = nil;
    NSString *name = [[textAttachment fileWrapper] filename];

    if (name) {
        Document *document = [self document];
        NSURL *docURL = [document fileURL];

        if (!docURL) docURL = [document autosavedContentsFileURL];

        if (docURL && [docURL isFileURL])
			attachmentURL = [docURL URLByAppendingPathComponent:name];
    }
    
    return attachmentURL;
}

- (NSArray *)textView:(NSTextView *)view writablePasteboardTypesForCell:(id <NSTextAttachmentCell>)cell atIndex:(NSUInteger)charIndex {
#pragma unused (charIndex, view)
	NSString *name = [[[cell attachment] fileWrapper] filename];
	NSURL *docURL = [[self document] fileURL];
	return (docURL && [docURL isFileURL] && name) ? [NSArray arrayWithObject:NSFilenamesPboardType] : nil;
}

- (BOOL)textView:(NSTextView *)view writeCell:(id <NSTextAttachmentCell>)cell atIndex:(NSUInteger)charIndex toPasteboard:(NSPasteboard *)pboard type:(NSString *)type {
#pragma unused (charIndex, view)
	NSString *name = [[[cell attachment] fileWrapper] filename];
	NSURL *docURL = [[self document] fileURL];
	if ([type isEqualToString:NSFilenamesPboardType] && name && [docURL isFileURL]) {
		NSString *docPath = [docURL path];
		NSString *pathToAttachment = [docPath stringByAppendingPathComponent:name];
		if (pathToAttachment) {
			[pboard setPropertyList:[NSArray arrayWithObject:pathToAttachment] forType:NSFilenamesPboardType];
			return YES;
		}
	}
	return NO;
}

@end


@implementation DocumentWindowController(NSMenuValidation)

- (BOOL)validateMenuItem:(NSMenuItem *)aCell {
	NSString *title = nil;
	SEL action = [aCell action];

	if (action == @selector(InsertTiersSpOne:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[0] length:strlen(SpeakerNames[0])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpTwo:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[1] length:strlen(SpeakerNames[1])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpThree:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[2] length:strlen(SpeakerNames[2])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpFour:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[3] length:strlen(SpeakerNames[3])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpFive:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[4] length:strlen(SpeakerNames[4])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpSix:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[5] length:strlen(SpeakerNames[5])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpSeven:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[6] length:strlen(SpeakerNames[6])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpEight:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[7] length:strlen(SpeakerNames[7])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpNine:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[8] length:strlen(SpeakerNames[8])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(InsertTiersSpTen:)) { // 2020-09-10
		title = NSLocalizedString([NSString stringWithCharacters:SpeakerNames[9] length:strlen(SpeakerNames[9])],
								  @"Speaker codes");
		[aCell setTitle:title];
		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(ExpandBullets:)) { // 2020-09-01
		if (ShowParags == YES)
			title = NSLocalizedString(@"Hide bullets {Esc-a}", @"Menu item hides bullets");
		else
			title = NSLocalizedString(@"Expand bullets {Esc-a}", @"Menu item expends bullets");

		[aCell setTitle:title];

		if ([[self document] isReadOnly])
			return NO;
	} else if (action == @selector(ChatModeSet:)) { // 2020-12-20
		if (ChatMode == YES)
			[aCell setState:NSControlStateValueOn];
		else
			[aCell setState:NSControlStateValueOff];
		return YES;
	} else if (action == @selector(TrimTiers:)) { // 2020-12-20
		if (TrimTierMode == YES)
			title = NSLocalizedString(@"Show dependent tiers", @"Menu item shows dependent tiers");
		else
			title = NSLocalizedString(@"Hide dependent tiers", @"Menu item hides dependent tiers");
		[aCell setTitle:title];
		return YES;
	} else if (action == @selector(showHideLineNumbers:)) { // 2020-09-18
		if (isChatLineNums)
			[aCell setState:NSControlStateValueOn];
		else
			[aCell setState:NSControlStateValueOff];
		return YES;
	} else if (action == @selector(EditMode:)) { // 2020-09-24
		if (EditorMode)
			[aCell setState:NSControlStateValueOff];
		else
			[aCell setState:NSControlStateValueOn];
		return YES;
	} else if (action == @selector(SonicMode:)) { // 2020-11-13
		if (EditorMode)
			[aCell setState:NSControlStateValueOff];
		else
			[aCell setState:NSControlStateValueOn];
		return YES;
	}

	return YES;
}
/*
- (NSMenu *)textView:(NSTextView *)view menu:(NSMenu *)menu forEvent:(NSEvent *)event atIndex:(NSUInteger)charIndex {
    // Removing layout orientation menu item in multipage mode for enforcing the document-wide setting
	return menu;
}
*/
@end

@implementation LowerTextView // 2020-04-28

enum {
	noDir = 0,
	leftDir,
	rightDir
} ;

- (IBAction)lowerSlide:(id)sender {
#pragma unused (sender)
}

- (void)setLowerFont:(NSFont *)newLowerFont {// 2020-04-28
	lowerWinFont = newLowerFont;
}

- (void)setLowerFontHeight:(CGFloat)newFontHeight { // 2020-09-24
	lowerFontHeight = newFontHeight;
}

- (void)setInfoString:(char *)newInfoString {// 2020-04-28
//	u_strcpy(lowerInfoString, newInfoString, 2048);
	strcpy(lowerInfoString, newInfoString);
}

- (char *)getInfoString {// 2020-06-08
	return lowerInfoString;
}

- (void)setRowsCnt:(NSUInteger)newRowsCnt {  // 2020-09-24
	rowsNum = newRowsCnt;
}

- (NSUInteger)getRowsCnt { // 2020-09-24
	return(rowsNum);
}

- (void)CheckNewF:(sndInfoC *)pSnTr newCol:(long)NewF direction:(char *)dir {
	if (pSnTr->EndF == 0L || *dir == noDir) {
		if (NewF > pSnTr->BegF) {
			*dir = rightDir;
			pSnTr->EndF = NewF;
		} else if (NewF < pSnTr->BegF) {
			*dir = leftDir;
			pSnTr->EndF = pSnTr->BegF;
			pSnTr->BegF = NewF;
		}
	} else if (*dir == leftDir) {
		if (NewF == pSnTr->EndF) {
			pSnTr->BegF = NewF;
			pSnTr->EndF = 0L;
			*dir = noDir;
		} else if (NewF < pSnTr->EndF) {
			pSnTr->BegF = NewF;
		} else {
			pSnTr->BegF = pSnTr->EndF;
			pSnTr->EndF = NewF;
			*dir = rightDir;
		}
	} else if (*dir == rightDir) {
		if (NewF == pSnTr->BegF) {
			pSnTr->BegF = NewF;
			pSnTr->EndF = 0L;
			*dir = noDir;
		} else if (NewF > pSnTr->BegF) {
			pSnTr->EndF = NewF;
		} else {
			pSnTr->EndF = pSnTr->BegF;
			pSnTr->BegF = NewF;
			*dir = leftDir;
		}
	}
}

- (void)PutSoundStats:(DocumentWindowController *)docWinCtrl winDur:(long)ws {
	const char *mtype;
	long cb, ce, cbm, cbsm, cem, cesm;
	sndInfoC *pSnTr;
	NSUInteger j;
	NSTextView *textView;
	NSRange cursorRange;

	pSnTr = &docWinCtrl->SnTr;
	if (pSnTr->StatusLineType == 0) {
		return;
	}
	if (pSnTr->StatusLineType == 2) {
		FNType mFileName[FNSize];

		extractFileName(mFileName, pSnTr->mediaFPath);
		if (pSnTr->SNDformat == kAudioFormatMPEGLayer3)
			mtype = "MP3";
		else if (pSnTr->SNDformat == kAudioFormatULaw)
			mtype = "mu-law";
		else if (pSnTr->SNDformat == kAudioFormatALaw)
			mtype = "a-law";
		else if (pSnTr->SNDformat == kAudioFormatLinearPCM)
			mtype = "pcm";
		else
			mtype = "unknown";
		if (pSnTr->SNDformat == kAudioFormatMPEGLayer3) {
			sprintf(ced_err_message, "-\"%s\": %ldHz., %s., %s",
					mFileName, (long)pSnTr->mSampleRate,
					((pSnTr->SNDchan == 1) ? "mono" : "stereo"), mtype);
		} else {
			sprintf(ced_err_message, "-\"%s\": %d bits, %ldHz., %s., %s",
					mFileName, pSnTr->mBitsPerChannel, (long)pSnTr->mSampleRate,
					((pSnTr->SNDchan == 1) ? "mono" : "stereo"), mtype);
		}
	} else {
		if (pSnTr->BegF == pSnTr->EndF)
			pSnTr->EndF = 0L;
		cb = [docWinCtrl conv_to_msec_rep:pSnTr->WBegF];
		ce = pSnTr->WBegF + ws;
		if (ce > pSnTr->SoundFileSize)
			ce = pSnTr->SoundFileSize;
		ce = [docWinCtrl conv_to_msec_rep:ce];
		sprintf(ced_err_message, "-W ");

		if (ce/1000/60 > 0) {
			cbm = cb / 1000 / 60;
			cbsm = (cb / 1000) - (cbm * 60);
			cem = ce / 1000 / 60;
			cesm = (ce / 1000) - (cem * 60);
//			if (cbm != cem || cbsm != cesm) {
//				sprintf(ced_err_message+strlen(ced_err_message), "%ld:%ld-%ld:%ld; ", cbm, cbsm, cem, cesm);
//			} else {
			cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
			ce = ce - (cem * 1000 * 60) - (cesm * 1000);
			sprintf(ced_err_message+strlen(ced_err_message), "%ld:%ld.%s%s%ld-%ld:%ld.%s%s%ld; ",
					cbm, cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb,
					cem, cesm, ((ce < 100)? "0" : ""), ((ce < 10)? "0" : ""), ce);
//			}
		} else if (ce / 1000 > 0) {
			cbsm = cb / 1000;
			cesm = ce / 1000;
//			if (cbsm != cesm)
//				sprintf(ced_err_message+strlen(ced_err_message),"%lds-%lds; ",cbsm,cesm);
//			else {
			cb = cb - (cbsm * 1000);
			ce = ce - (cesm * 1000);
			sprintf(ced_err_message+strlen(ced_err_message), "%ld.%s%s%ld-%ld.%s%s%ld; ",
					cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb,
					cesm, ((ce < 100)? "0" : ""), ((ce < 10)? "0" : ""), ce);
//			}
		} else {
			sprintf(ced_err_message+strlen(ced_err_message), "%ldMs-%ldMs; ", cb, ce);
		}

		if (pSnTr->StatusLineType == 1) {
			if (pSnTr->EndF != 0L) {
				cb = [docWinCtrl conv_to_msec_rep:pSnTr->EndF] - [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
				cbm = cb / 1000 / 60;
				cbsm = (cb / 1000) - (cbm * 60);
				cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
				sprintf(ced_err_message+strlen(ced_err_message), "D 00:%s%ld:%s%ld.%s%s%ld; ",
						(cbm < 10 ? "0" : ""), cbm, (cbsm < 10 ? "0" : ""), cbsm,
						((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
			} else
				sprintf(ced_err_message+strlen(ced_err_message), "D 00:00:00.000; ");
		}

		cb = [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
		strcat(ced_err_message, "C at ");
		if (cb/1000/60 > 0) {
			cbm = cb / 1000 / 60;
			cbsm = (cb / 1000) - (cbm * 60);
			cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
			sprintf(ced_err_message+strlen(ced_err_message), "%ld:%ld.%s%s%ld",
					cbm, cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
			cb = [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
			sprintf(ced_err_message+strlen(ced_err_message), "(%ld)", cb);
		} else if (cb / 1000 > 0) {
			cbsm = cb / 1000;
			cb = cb - (cbsm * 1000);
			sprintf(ced_err_message+strlen(ced_err_message), "%ld.%s%s%lds",
					cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
			cb = [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
			sprintf(ced_err_message+strlen(ced_err_message), "(%ld)", cb);
		} else {
			sprintf(ced_err_message+strlen(ced_err_message), "%ldMs", cb);
		}
	}
	textView = [docWinCtrl firstTextView];
	cursorRange = [textView selectedRange];
	j = [docWinCtrl countLineNumbers:cursorRange.location];
	[docWinCtrl setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
}

- (void)mouseDown:(NSEvent*)incomingEvent {// 2020-05-06
	int index;
	char *fname, *s;
	const char *mtype;
	BOOL isMatched;
	NSInteger clickCount;
	NSUInteger j;
	UInt32 ctrlWidth;
	NSPoint mouseLoc;
	NSWindow *win = [self window];
	NSRect wFrame = [self frame];
	NSRange cursorRange;
	CGRect cBox;
	NSEventModifierFlags modFlags;
	DocumentWindowController *docWinCtrl;
	NSTextView *textView;
	sndInfoC *pSnTr;
	struct AVInfo AVinfo;

	modFlags = [incomingEvent modifierFlags]; // (modFlags & NSAlternateKeyMask)
		// NSEventModifierFlagShift, NSEventModifierFlagControl, NSEventModifierFlagOption, NSEventModifierFlagCommand
	docWinCtrl = getDocWinController(win);
	pSnTr = &docWinCtrl->SnTr;
	mouseLoc = [incomingEvent locationInWindow];
	if (rowsNum > 2) { // 2020-09-24
		if (pSnTr->IsSoundOn == true) {
			ctrlWidth = pSnTr->ctrlWidth;
			if (modFlags & NSEventModifierFlagCommand) {
				if (pSnTr->BegF < pSnTr->EndF && pSnTr->EndF > 0L) {
					extractPath(AVinfo.mediaFPath, pSnTr->mediaFPath);
					extractFileName(AVinfo.mediaFName, pSnTr->mediaFPath);
					s = strrchr(AVinfo.mediaFName, '.');
					if (s != NULL)
						*s = EOS;
					AVinfo.beg = [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
					AVinfo.end = [docWinCtrl conv_to_msec_rep:pSnTr->EndF];
					AVinfo.playMode = oneBullet;
					AVinfo.textView = [docWinCtrl firstTextView];
					AVinfo.docWindow = [docWinCtrl window];
					AVinfo.nextSegs = nil;
					[AVController createAndPlayAV:&AVinfo];
				}
				return;
			} else if (mouseLoc.y >= lowerFontHeight * (rowsNum - 1) && mouseLoc.y <= lowerFontHeight * rowsNum) { // Status Bar
				pSnTr->StatusLineType = (++pSnTr->StatusLineType) % 3;
				if (pSnTr->StatusLineType == 0) {
					textView = [docWinCtrl firstTextView];
					cursorRange = [textView selectedRange];
					j = [docWinCtrl countLineNumbers:cursorRange.location];
					[docWinCtrl setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
				} else
					[self PutSoundStats:docWinCtrl winDur:(pSnTr->WEndF - pSnTr->WBegF)];
			} else if (mouseLoc.x >= 0 && mouseLoc.x <= ctrlWidth &&
				mouseLoc.y >= lowerFontHeight * 5 && mouseLoc.y <= lowerFontHeight * 6) { // +H
				pSnTr->SoundFileSize = [docWinCtrl conv_to_msec_rep:(pSnTr->SoundFileSize)];
				pSnTr->WBegF = [docWinCtrl conv_to_msec_rep:(pSnTr->WBegF)];
				pSnTr->BegF = [docWinCtrl conv_to_msec_rep:(pSnTr->BegF)];
				pSnTr->EndF = [docWinCtrl conv_to_msec_rep:(pSnTr->EndF)];
				pSnTr->HScale -= (pSnTr->HScale / 2);
				if (pSnTr->HScale <= 0)
					pSnTr->HScale = 1;
				pSnTr->SoundFileSize = [docWinCtrl conv_from_msec_rep:(pSnTr->SoundFileSize)];
				if (pSnTr->TopP1 != NULL)
					free(pSnTr->TopP1);
				if (pSnTr->TopP2 != NULL)
					free(pSnTr->TopP2);
				pSnTr->TopP1 = (Float32 *)malloc((pSnTr->SoundFileSize+1) * sizeof(Float32));
				if (pSnTr->SNDchan == 2 && !doMixedSTWave)
					pSnTr->TopP2 = (Float32 *)malloc((pSnTr->SoundFileSize+1) * sizeof(Float32));
				else
					pSnTr->TopP2 = NULL;
				pSnTr->WBegF = [docWinCtrl conv_from_msec_rep:(pSnTr->WBegF)];
				pSnTr->BegF = [docWinCtrl conv_from_msec_rep:(pSnTr->BegF)];
				pSnTr->EndF = [docWinCtrl conv_from_msec_rep:(pSnTr->EndF)];
				pSnTr->WEndF = pSnTr->WBegF + pSnTr->WaveWidth;
				[docWinCtrl readMedia:false];
			} else if (mouseLoc.x >= 0 && mouseLoc.x <= ctrlWidth &&
					   mouseLoc.y >= lowerFontHeight * 3 && mouseLoc.y <= lowerFontHeight * 4) { // S
				[docWinCtrl InsertBulletIntoText:self];
			} else if (mouseLoc.x >= 0 && mouseLoc.x <= ctrlWidth &&
					   mouseLoc.y >= ScrollBarHeight && mouseLoc.y <= lowerFontHeight * 2) { // -H
				if (pSnTr->WaveWidth < pSnTr->SoundFileSize) {
					pSnTr->SoundFileSize = [docWinCtrl conv_to_msec_rep:(pSnTr->SoundFileSize)];
					pSnTr->WBegF = [docWinCtrl conv_to_msec_rep:(pSnTr->WBegF)];
					pSnTr->BegF = [docWinCtrl conv_to_msec_rep:(pSnTr->BegF)];
					pSnTr->EndF = [docWinCtrl conv_to_msec_rep:(pSnTr->EndF)];
					pSnTr->HScale += pSnTr->HScale;
					if (pSnTr->HScale <= 0)
						pSnTr->HScale = 1;
					pSnTr->SoundFileSize = [docWinCtrl conv_from_msec_rep:(pSnTr->SoundFileSize)];
					if (pSnTr->TopP1 != NULL)
						free(pSnTr->TopP1);
					if (pSnTr->TopP2 != NULL)
						free(pSnTr->TopP2);
					pSnTr->TopP1 = (Float32 *)malloc((pSnTr->SoundFileSize+1) * sizeof(Float32));
					if (pSnTr->SNDchan == 2 && !doMixedSTWave)
						pSnTr->TopP2 = (Float32 *)malloc((pSnTr->SoundFileSize+1) * sizeof(Float32));
					else
						pSnTr->TopP2 = NULL;
					pSnTr->WBegF = [docWinCtrl conv_from_msec_rep:(pSnTr->WBegF)];
					pSnTr->BegF = [docWinCtrl conv_from_msec_rep:(pSnTr->BegF)];
					pSnTr->EndF = [docWinCtrl conv_from_msec_rep:(pSnTr->EndF)];
					pSnTr->WEndF = pSnTr->WBegF + pSnTr->WaveWidth;
					[docWinCtrl readMedia:false];
				}
			} else if (mouseLoc.x >= wFrame.size.width - ctrlWidth && mouseLoc.x <= wFrame.size.width &&
					   mouseLoc.y >= lowerFontHeight * 5 && mouseLoc.y <= lowerFontHeight * 6) { // +V
				pSnTr->VScale *= 2;
				if (pSnTr->VScale < 1)
					pSnTr->VScale = 1;
				[docWinCtrl DisplayEndF];
				[self setNeedsDisplay:YES];
			} else if (mouseLoc.x >= wFrame.size.width - ctrlWidth && mouseLoc.x <= wFrame.size.width &&
					   mouseLoc.y >= ScrollBarHeight && mouseLoc.y <= lowerFontHeight * 2) { // -V
				pSnTr->VScale /= 2;
				if (pSnTr->VScale < 1)
					pSnTr->VScale = 1;
				[docWinCtrl DisplayEndF];
				[self setNeedsDisplay:YES];
			} else {
				BOOL leftSide, rightSide;
				long NewF, HalfF;
				char dir;
				NSPoint gmouseLoc;
				CGEventRef globalEvent, left_drag_event;
				NSUInteger mouseButtonMask;


/*
mouseLoc.x = ctrlWidth;
mouseLoc.y = lowerFontHeight;
[NSEvent mouseEventWithType:(NSEventType)NSEventTypeLeftMouseDragged
	location:(NSPoint)mouseLoc
	modifierFlags:(NSEventModifierFlags)0
	timestamp:(NSTimeInterval)0.0
	windowNumber:(NSInteger)wNum
	context:(NSGraphicsContext *)nil
	eventNumber:(NSInteger)NSIntegerMin
	clickCount:(NSInteger)0
	pressure:(float)0.0
];

typedef NS_ENUM(NSUInteger, NSEventType) { // various types of events
	 NSEventTypeLeftMouseDown             = 1,
	 NSEventTypeLeftMouseUp               = 2,
	 NSEventTypeRightMouseDown            = 3,
	 NSEventTypeRightMouseUp              = 4,
	 NSEventTypeMouseMoved                = 5,
	 NSEventTypeLeftMouseDragged          = 6,
	 NSEventTypeRightMouseDragged         = 7,
	 NSEventTypeMouseEntered              = 8,
	 NSEventTypeMouseExited               = 9,
	 NSEventTypeKeyDown                   = 10,
	 NSEventTypeKeyUp                     = 11,
	 NSEventTypeFlagsChanged              = 12,
	 NSEventTypeAppKitDefined             = 13,
	 NSEventTypeSystemDefined             = 14,
	 NSEventTypeApplicationDefined        = 15,
	 NSEventTypePeriodic                  = 16,
	 NSEventTypeCursorUpdate              = 17,
	 NSEventTypeScrollWheel               = 22,
	 NSEventTypeTabletPoint               = 23,
	 NSEventTypeTabletProximity           = 24,
	 NSEventTypeOtherMouseDown            = 25,
	 NSEventTypeOtherMouseUp              = 26,
	 NSEventTypeOtherMouseDragged         = 27,
};

pSnTr->WaveWidth
[NSThread sleepForTimeInterval:0.05];
usleep(10000);

mouseButtonMask = [NSEvent pressedMouseButtons];
BOOL leftMouseButtonDown = (mouseButtonMask & (1 << 0)) != 0;
BOOL rightMouseButtonDown = (mouseButtonMask & (1 << 1)) != 0;
*/
				if (mouseLoc.x < ctrlWidth || mouseLoc.x > wFrame.size.width - ctrlWidth)
					return;
				dir = noDir;
				NewF = pSnTr->WBegF + ((mouseLoc.x-ctrlWidth));
				if (modFlags & NSEventModifierFlagShift) {
					if (pSnTr->BegF <= pSnTr->EndF) {
						HalfF = pSnTr->BegF + ((pSnTr->EndF - pSnTr->BegF) / 2);
						if (NewF >= HalfF)
							dir = rightDir;
						else
							dir = leftDir;
					}
				} else {
					pSnTr->BegF = NewF;
					pSnTr->EndF = 0L;
				}
				[self CheckNewF:pSnTr newCol:NewF direction:&dir];
				[self setNeedsDisplay:YES];
				pSnTr->IsPlayMedia = true;
				mouseButtonMask = [NSEvent pressedMouseButtons];
				leftSide = false;
				rightSide = false;
				while ((mouseButtonMask & (1 << 0)) == 1) {
					// NSEventMaskLeftMouseUp | NSEventMaskLeftMouseDragged | NSEventMaskLeftMouseDown
					incomingEvent = [[docWinCtrl window] nextEventMatchingMask:NSEventMaskAny];
					mouseButtonMask = [NSEvent pressedMouseButtons];
					if ((mouseButtonMask & (1 << 0)) == 0) {
						pSnTr->IsPlayMedia = true;
						break;
					}
					ctrlWidth = pSnTr->ctrlWidth;
					if (incomingEvent.type == NSEventTypeLeftMouseDragged && incomingEvent.window == win) {
						mouseLoc = [self convertPoint:[incomingEvent locationInWindow] fromView:nil];
						if (mouseLoc.x < ctrlWidth/* && pSnTr->BegF <= pSnTr->WBegF*/) {
							leftSide = true;
						} else if (mouseLoc.x > pSnTr->WaveWidth + ctrlWidth/* && pSnTr->EndF >= pSnTr->WEndF*/) {
							rightSide = true;
						} else {
							leftSide = false;
							rightSide = false;
							NewF = pSnTr->WBegF + ((mouseLoc.x-ctrlWidth));
							[self CheckNewF:pSnTr newCol:NewF direction:&dir];
						}
					}
					if (leftSide) {
						NewF = 2;
						if (pSnTr->WBegF-NewF < 0L) {
							pSnTr->WBegF = 0L;
							pSnTr->WEndF = pSnTr->WBegF + pSnTr->WaveWidth;
						} else {
							pSnTr->WBegF -= NewF;
							pSnTr->WEndF -= NewF;
						}
						NewF = pSnTr->WBegF;
						[self CheckNewF:pSnTr newCol:NewF direction:&dir];

						globalEvent = CGEventCreate(nil);
						gmouseLoc = CGEventGetLocation(globalEvent);
						CFRelease(globalEvent);
						left_drag_event = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDragged, gmouseLoc, kCGMouseButtonLeft);
						CGEventPost(kCGHIDEventTap, left_drag_event);
						CFRelease(left_drag_event);
						if (pSnTr->SNDchan == 1)
							[NSThread sleepForTimeInterval:0.001];
						else
							[NSThread sleepForTimeInterval:0.001];
					} else if (rightSide) {
						NewF = 2;
						if (pSnTr->WEndF+NewF > pSnTr->SoundFileSize) {
							pSnTr->WEndF = pSnTr->SoundFileSize;
							pSnTr->WBegF = pSnTr->WEndF - pSnTr->WaveWidth;
						} else {
							pSnTr->WBegF += NewF;
							pSnTr->WEndF += NewF;
						}
						NewF = pSnTr->WEndF;
						[self CheckNewF:pSnTr newCol:NewF direction:&dir];

						globalEvent = CGEventCreate(nil);
						gmouseLoc = CGEventGetLocation(globalEvent);
						CFRelease(globalEvent);
						left_drag_event = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDragged, gmouseLoc, kCGMouseButtonLeft);
						CGEventPost(kCGHIDEventTap, left_drag_event);
						CFRelease(left_drag_event);
						if (pSnTr->SNDchan == 1)
							[NSThread sleepForTimeInterval:0.001];
						else
							[NSThread sleepForTimeInterval:0.001];
					}
					[self setNeedsDisplay:YES];
					[self PutSoundStats:docWinCtrl winDur:(pSnTr->WEndF - pSnTr->WBegF)];
					pSnTr->IsPlayMedia = true;
				}
				if (pSnTr->IsPlayMedia == true) {
					pSnTr->IsPlayMedia = false;
					if (pSnTr->BegF < pSnTr->EndF && pSnTr->EndF > 0L) {
						extractPath(AVinfo.mediaFPath, pSnTr->mediaFPath);
						extractFileName(AVinfo.mediaFName, pSnTr->mediaFPath);
						s = strrchr(AVinfo.mediaFName, '.');
						if (s != NULL)
							*s = EOS;
						AVinfo.beg = [docWinCtrl conv_to_msec_rep:pSnTr->BegF];
						AVinfo.end = [docWinCtrl conv_to_msec_rep:pSnTr->EndF];
						AVinfo.playMode = oneBullet;
						AVinfo.textView = [docWinCtrl firstTextView];
						AVinfo.docWindow = [docWinCtrl window];
						AVinfo.nextSegs = nil;
						[AVController createAndPlayAV:&AVinfo];
					}
				}
			}
			[self PutSoundStats:docWinCtrl winDur:(pSnTr->WEndF - pSnTr->WBegF)];
		} else if (docWinCtrl->EditorMode == false) {
			isMatched = false;
			if (docWinCtrl->StartCodeArr != NULL) {
				for (index=0; index < MaxNumOfCodes; index++) {
					if (docWinCtrl->StartCodeArr[index] == NULL) {
						break;
					}
					cBox = docWinCtrl->StartCodeArr[index]->stringRect;
					if (mouseLoc.x >= cBox.origin.x && mouseLoc.x <= cBox.origin.x + cBox.size.width &&
						mouseLoc.y >= cBox.origin.y && mouseLoc.y <= cBox.origin.y + cBox.size.height) {
						isMatched = true;
						break;
					}
				}

				clickCount = [incomingEvent clickCount];
				if (isMatched) {
					if (clickCount == 1) {
						docWinCtrl->CursorCodeArr = index;
						[docWinCtrl->lowerView setNeedsDisplay:YES];
					} else if (clickCount == 2) {
						docWinCtrl->CursorCodeArr = index;
						GetCursorCode(13, docWinCtrl);
					}
				}
			}
		}
	}
	if (AVMediaPlayer != nil && pSnTr->IsSoundOn == false) {
		if (AVMediaPlayer->isWhatType == isAudio && AVMediaPlayer->audioPlayer != nil) {
			if (AVMediaPlayer->rMovieFile[0] != EOS) {
				UInt32 thePropertySize;
				ExtAudioFileRef sourceFile = nil;
				AudioStreamBasicDescription srcFormat;
				NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:AVMediaPlayer->rMovieFile]];

				ExtAudioFileOpenURL((__bridge CFURLRef)url, &sourceFile);
				thePropertySize = sizeof(srcFormat); //UInt32(MemoryLayout.stride(ofValue: srcFormat));;
				ExtAudioFileGetProperty(sourceFile, kExtAudioFileProperty_FileDataFormat, &thePropertySize, &srcFormat);

				fname = strrchr(AVMediaPlayer->rMovieFile, '/');
				if (fname == NULL)
					fname = AVMediaPlayer->rMovieFile;
				else
					fname++;
				if (srcFormat.mFormatID == kAudioFormatMPEGLayer3)
					mtype = "MP3";
				else if (srcFormat.mFormatID == kAudioFormatULaw)
					mtype = "mu-law";
				else if (srcFormat.mFormatID == kAudioFormatALaw)
					mtype = "a-law";
				else if (srcFormat.mFormatID == kAudioFormatLinearPCM)
					mtype = "pcm";
				else
					mtype = "unknown";
				if (srcFormat.mFormatID == kAudioFormatMPEGLayer3) {
					sprintf(ced_err_message, "-\"%s\": %ldHz., %s., %s",
							fname, (long)srcFormat.mSampleRate,
							((srcFormat.mChannelsPerFrame == 1) ? "mono" : "stereo"), mtype);
				} else {
					sprintf(ced_err_message, "-\"%s\": %d bits, %ldHz., %s., %s",
							fname, srcFormat.mBitsPerChannel, (long)srcFormat.mSampleRate,
							((srcFormat.mChannelsPerFrame == 1) ? "mono" : "stereo"), mtype);
				}
				docWinCtrl = getDocWinController(win);
				textView = [docWinCtrl firstTextView];
				cursorRange = [textView selectedRange];
				j = [docWinCtrl countLineNumbers:cursorRange.location];
				[docWinCtrl setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
			}
		}
	} else if (rowsNum <= 2 && pSnTr->IsSoundOn == false) {
		if (mouseLoc.y >= lowerFontHeight * (rowsNum - 1) && mouseLoc.y <= lowerFontHeight * rowsNum) {
			// Status Bar
			[docWinCtrl blinkCursorLine];
		}
	}
}

- (void)rightMouseDown:(NSEvent*)incomingEvent{// 2020-05-06
	NSInteger clickCount;
	NSPoint mouseLoc;
	NSWindow *win = [self window];
	NSRect wFrame;
	wFrame = [win frame];

	clickCount = [incomingEvent clickCount];
	mouseLoc = [incomingEvent locationInWindow];
}

- (id)initWithFrame:(NSRect)frame {
	int i;
	self = [super initWithFrame:frame];
	if (self) {
		i = 0;
	}
	return self;
}

-(CGFloat)maxSize:(DocumentWindowController *)docWinCtrl atts:(NSDictionary *)attsW {
	int index, row;
	BOOL done;
	unCH *code;
	CGFloat max;
	NSUInteger wsize;
	NSString *string;
	CGSize  stringSize;

	wsize = rowsNum - 2;
	if (wsize == 0)
		return(0.0);
	max = 0.0;
	index = 0;
	done = false;
	while (done == false) {
		for (row=0; row < wsize; row++) {
			if (docWinCtrl->StartCodeArr[index] == NULL) {
				done = true;
				break;
			}
			code = docWinCtrl->StartCodeArr[index]->code;
			string = [NSString stringWithCharacters:code length:strlen(code)];
			if (string == nil)
				return(max);
			stringSize = [string sizeWithAttributes:attsW];
			if (stringSize.width > max)
				max = stringSize.width;
			index++;
		}
	}
	return(max);
}

// sonic mode beg

- (void)DrawSoundCursor:(DocumentWindowController *)docWinCtrl part:(BOOL)isFirstCall {
	CGFloat winWidth;
	CGFloat winHeight;
	CGRect  rect;
	sndInfoC *pSnTr;
	pSnTr = &docWinCtrl->SnTr;
	CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

	pSnTr = &docWinCtrl->SnTr;

	if (pSnTr->EndF > pSnTr->SoundFileSize) {
		if (pSnTr->BegF == pSnTr->EndF) {
			pSnTr->EndF = 0L;
			pSnTr->BegF = pSnTr->SoundFileSize;
		} else
			pSnTr->EndF = pSnTr->SoundFileSize;
	}
	winHeight = lowerFontHeight * (rowsNum - 2);
	if (pSnTr->WEndF >= pSnTr->SoundFileSize)
		pSnTr->WEndF  = pSnTr->SoundFileSize;

	if (isFirstCall == true && (pSnTr->BegF > pSnTr->WBegF || pSnTr->EndF < pSnTr->WEndF)) {
		rect.origin.y = ScrollBarHeight;
		rect.origin.x = pSnTr->ctrlWidth;
		rect.size = CGSizeMake(docWinCtrl->SnTr.WaveWidth, winHeight);

		CGContextSetFillColorWithColor(context, [NSColor whiteColor].CGColor);
		CGContextSetLineWidth(context, 1.0);
		CGContextSetAlpha(context,1.0);
		CGContextFillRect(context, rect);
	}
	if (pSnTr->BegF != pSnTr->EndF && pSnTr->EndF != 0L) {
		if (isFirstCall == true) {
//			if (docWinCtrl->SnTr.IsSoundOn && PlayingContSound == TRUE && docWinCtrl->SnTr.contPlayBeg != 0L && docWinCtrl->SnTr.contPlayEnd != 0L) {
//				pSnTr->Beg = docWinCtrl->SnTr.contPlayBeg;
//				pSnTr->End = docWinCtrl->SnTr.contPlayEnd;
//			} else {
//				pSnTr->Beg = pSnTr->BegF;
//				pSnTr->End = pSnTr->EndF;
//			}
			rect.origin.y = ScrollBarHeight;
			if (pSnTr->BegF < pSnTr->WBegF)
				rect.origin.x = pSnTr->ctrlWidth;
			else
				rect.origin.x = pSnTr->ctrlWidth + (pSnTr->BegF - pSnTr->WBegF);
			if (pSnTr->EndF > pSnTr->WEndF) {
				winWidth = (CGFloat)pSnTr->WEndF - pSnTr->BegF;
				if (winWidth > (CGFloat)pSnTr->WaveWidth)
					winWidth = (CGFloat)pSnTr->WaveWidth;
				if (winWidth < 0.0)
					winWidth = 0.0;
				rect.size = CGSizeMake(winWidth, winHeight);
			} else {
				if (pSnTr->WBegF > pSnTr->BegF)
					winWidth = (CGFloat)(pSnTr->EndF - pSnTr->WBegF);
				else
					winWidth = (CGFloat)(pSnTr->EndF - pSnTr->BegF);
				if (winWidth > (CGFloat)pSnTr->WaveWidth) {
					winWidth = (CGFloat)pSnTr->WEndF - pSnTr->BegF;
					if (winWidth > (CGFloat)pSnTr->WaveWidth)
						winWidth = (CGFloat)pSnTr->WaveWidth;
				}
				if (winWidth < 0.0)
					winWidth = 0.0;
				rect.size = CGSizeMake(winWidth, winHeight);
			}
			if (winWidth > 0.0) {
				CGContextSetFillColorWithColor(context, [NSColor selectedTextBackgroundColor].CGColor);
				CGContextSetLineWidth(context, 1.0);
				CGContextSetAlpha(context,1.0);
				CGContextFillRect(context, rect);
			}
		}
	} else {
		if (isFirstCall == false && pSnTr->BegF >= pSnTr->WBegF && pSnTr->BegF <= pSnTr->WEndF) {
			rect.origin.y = ScrollBarHeight;
			rect.origin.x = pSnTr->ctrlWidth + (pSnTr->BegF - pSnTr->WBegF);
			rect.size = CGSizeMake(2.0, winHeight);

			CGContextSetFillColorWithColor(context, [NSColor redColor].CGColor);
			CGContextSetLineWidth(context, 1.0);
			CGContextSetAlpha(context,1.0);
			CGContextFillRect(context, rect);
		}
	}
}

- (void)drawWaveForm:(DocumentWindowController *)docWinCtrl {
	NSFont *font;
	NSString *string;
	UInt32 colCnt, TopPCnt, ctrlWidth, lSNDchan;
	Float32 topPixel, botPixel, diff;
	CGRect rect;
	CGFloat scale, winHeight;
	CGPoint textPosition;
	NSDictionary *attsW;
	NSRect wFrame = [self frame];
	CGColorRef leftcolor = [[NSColor blackColor] CGColor];
	CGColorRef rightcolor = [[NSColor blackColor] CGColor];
//	CGContextRef context = [NSGraphicsContext currentContext].CGContext; //UIGraphicsGetCurrentContext();
	CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

	font = lowerWinFont;
	if (font == nil) {
		return;
	}
	attsW = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor whiteColor], NSLigatureAttributeName:@0};
	if (attsW == nil) {
		return;
	}

	ctrlWidth = docWinCtrl->SnTr.ctrlWidth;

	string = @"+H";
	textPosition = CGPointMake(0, (lowerFontHeight * 4)+ScrollBarHeight); // x, y
	rect = CGRectMake(textPosition.x, textPosition.y, ctrlWidth, lowerFontHeight-1); // x, y, width, height
	[[NSColor blackColor] set];
	NSRectFill(rect);
	[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsW];

	string = @"S";
	textPosition = CGPointMake(0, (lowerFontHeight * 2)+ScrollBarHeight);
	rect = CGRectMake(textPosition.x, textPosition.y, ctrlWidth, lowerFontHeight-1);
	[[NSColor blackColor] set];
	NSRectFill(rect);
	[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsW];

	string = @"-H";
	textPosition = CGPointMake(0, ScrollBarHeight);
	rect = CGRectMake(textPosition.x, textPosition.y, ctrlWidth, lowerFontHeight-1);
	[[NSColor blackColor] set];
	NSRectFill(rect);
	[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsW];

	string = @"+V";
	textPosition = CGPointMake(wFrame.size.width-ctrlWidth, (lowerFontHeight * 4)+ScrollBarHeight);
	rect = CGRectMake(textPosition.x, textPosition.y, ctrlWidth, lowerFontHeight-1);
	[[NSColor blackColor] set];
	NSRectFill(rect);
	[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsW];

	string = @"-V";
	textPosition = CGPointMake(wFrame.size.width-ctrlWidth, ScrollBarHeight);
	rect = CGRectMake(textPosition.x, textPosition.y, ctrlWidth, lowerFontHeight-1);
	[[NSColor blackColor] set];
	NSRectFill(rect);
	[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsW];

	if (!doMixedSTWave)
		lSNDchan = docWinCtrl->SnTr.SNDchan;
	else
		lSNDchan = 1;

	winHeight = ((rowsNum * lowerFontHeight) - ScrollBarHeight) / (CGFloat)lSNDchan;
	scale = (int)((0xFFFFL / docWinCtrl->SnTr.VScale) / winHeight) + 1;

	winHeight = (lowerFontHeight * (rowsNum - 1)) - ScrollBarHeight;
	CGContextSetFillColorWithColor(context, [NSColor whiteColor].CGColor);
	CGContextSetAlpha(context,1.0);
	rect.size = CGSizeMake(docWinCtrl->SnTr.WaveWidth, winHeight);
	rect.origin.x = ctrlWidth;
	rect.origin.y = 0;
	CGContextFillRect(context, rect);
	CGContextSetLineWidth(context, 1.0);

	[self DrawSoundCursor:docWinCtrl part:true];

	float halfGraphHeight = (winHeight / 2) / (float)lSNDchan;
	float centerLeft = halfGraphHeight+ScrollBarHeight;
	float centerRight = (halfGraphHeight*3)+ScrollBarHeight ;

	colCnt = ctrlWidth;
	for (TopPCnt=docWinCtrl->SnTr.WBegF; TopPCnt < docWinCtrl->SnTr.SoundFileSize && colCnt < docWinCtrl->SnTr.WaveWidth+ctrlWidth; TopPCnt++) {
		topPixel = centerLeft + (docWinCtrl->SnTr.TopP1[TopPCnt] / scale);
		if (topPixel > centerLeft + halfGraphHeight)
			topPixel = centerLeft + halfGraphHeight;
		botPixel = centerLeft - (docWinCtrl->SnTr.TopP1[TopPCnt] / scale);
		if (botPixel < ScrollBarHeight)
			botPixel = ScrollBarHeight;
		if (topPixel > botPixel)
			diff = topPixel - botPixel;
		else
			diff = botPixel - topPixel;
		if (diff > 0.0 && diff < 1.0) {
			topPixel += 1.0;
//			botPixel -= 1.0;
		}
		CGContextMoveToPoint(context, colCnt, botPixel);
		CGContextAddLineToPoint(context, colCnt, topPixel);
		CGContextSetStrokeColorWithColor(context, leftcolor);
		CGContextStrokePath(context);

		if (lSNDchan == 2) {
			topPixel = centerRight + (docWinCtrl->SnTr.TopP2[TopPCnt] / scale);
			if (topPixel > centerRight + halfGraphHeight)
				topPixel = centerRight + halfGraphHeight;
			botPixel = centerRight - (docWinCtrl->SnTr.TopP2[TopPCnt] / scale);
			if (botPixel < centerRight - halfGraphHeight)
				botPixel = centerRight - halfGraphHeight;
			if (topPixel > botPixel)
				diff = topPixel - botPixel;
			else
				diff = botPixel - topPixel;
			if (diff > 0.0 && diff < 1.0) {
				topPixel += 1.0;
//				botPixel -= 1.0;
			}
			CGContextMoveToPoint(context, colCnt, botPixel);
			CGContextAddLineToPoint(context, colCnt, topPixel);
			CGContextSetStrokeColorWithColor(context, rightcolor);
			CGContextStrokePath(context);
		}
		colCnt++;
	}

	[self DrawSoundCursor:docWinCtrl part:false];

}
// sonic mode end

- (void)drawRect:(NSRect)rect { // 2020-04-28 form "TextArcCocoaSmall"
	int index, row;
	unCH *code;
	CGFloat col, max;
	NSUInteger wsize;
	NSWindow *win = [self window];
	NSRect wFrame = [win frame];
	NSFont *font;
	NSString *string;
	CGRect stringRect;
	CGSize  stringSize;
	CGPoint textPosition;
	NSDictionary *attsW, *attsB;
	DocumentWindowController *docWinCtrl;
	// Initialize the text matrix to a known value

	CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
	CGContextSetTextMatrix(context, CGAffineTransformIdentity);

// Draw a white background
	[[NSColor whiteColor] set];
 	NSRectFill(rect);
	CGContextSaveGState(context);

//	CGContextSetTextPosition(context, 0, lowerFontHeight);
//	CGColorRef color = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 0.0);
//	CGColorRef color = CGColorCreateSRGB(0.0, 0.0, 0.0, 0.0);


	font = lowerWinFont;
	if (font == nil) {
		return;
	}

	string = [NSString stringWithUTF8String:[self getInfoString]];
	if (string == nil) {
		return;
	}
	string = [string stringByReplacingOccurrencesOfString:@"\r\r" withString:@"NEWLINE"];
	string = [string stringByReplacingOccurrencesOfString:@"\r" withString:@"\\n"];
	string = [string stringByReplacingOccurrencesOfString:@"\n" withString:@"\\n"];
	textPosition = CGPointMake(0.0, lowerFontHeight * (rowsNum - 1));

	attsW = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor whiteColor], NSLigatureAttributeName:@0};
	if (attsW == nil) {
		return;
	}
	attsB = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor blackColor], NSLigatureAttributeName:@0};
	if (attsB == nil) {
		return;
	}

	stringSize = [string sizeWithAttributes:attsW];
	stringRect = CGRectMake(textPosition.x, textPosition.y, stringSize.width, lowerFontHeight);

	[[NSColor blackColor] set];
	NSRectFill(stringRect);
//	textPosition.y += 1.0;
	[string drawAtPoint:textPosition withAttributes:attsW];
//	[string drawInRect:stringRect withAttributes:attsW];

	if (rowsNum > 2) {
		docWinCtrl = getDocWinController(win);
		if (docWinCtrl->SnTr.IsSoundOn == true) {
			docWinCtrl->LowerSlider.integerValue = docWinCtrl->SnTr.WBegF;
			[self drawWaveForm:docWinCtrl];
		} else if (docWinCtrl->EditorMode == false) {
			wsize = rowsNum - 2;

			if (docWinCtrl->SnTr.IsSoundOn == true)
				return;
			
			max = [self maxSize:docWinCtrl atts:attsW];

			for (index=0; index < MaxNumOfCodes; index++) {
				if (docWinCtrl->StartCodeArr[index] == NULL) {
					break;
				}
				docWinCtrl->StartCodeArr[index]->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
			}
			col = 0.0;
			index = 0;
			while (true) {
				if (col > wFrame.size.width)
					return;
				for (row=0; row < wsize; row++) {
					if (docWinCtrl->StartCodeArr[index] == NULL) {
						return;
					}
					if (docWinCtrl->EditorMode)
						index++;
					else {
						code = docWinCtrl->StartCodeArr[index]->code;
						string = [NSString stringWithCharacters:code length:strlen(code)];
						if (string == nil) {
							return;
						}
						if (index == docWinCtrl->CursorCodeArr) {
	//						stringSize = [string sizeWithAttributes:attsW];
							textPosition = CGPointMake(col, lowerFontHeight * (rowsNum-2-row));
							stringRect = CGRectMake(textPosition.x, textPosition.y, max, lowerFontHeight-1);
							[[NSColor blackColor] set]; //[[NSColor selectedTextBackgroundColor] set];
							NSRectFill(stringRect);
							[string drawAtPoint:textPosition withAttributes:attsW]; //[string drawAtPoint:textPosition withAttributes:attsB];
	//						[string drawInRect:stringRect withAttributes:attsW];
						} else {
	//						stringSize = [string sizeWithAttributes:attsB];
							textPosition = CGPointMake(col, lowerFontHeight * (rowsNum-2-row));
							stringRect = CGRectMake(textPosition.x, textPosition.y, max, lowerFontHeight-1);
							[[NSColor whiteColor] set];
							NSRectFill(stringRect);
							[string drawAtPoint:textPosition withAttributes:attsB];
	//						[string drawInRect:stringRect withAttributes:attsB];
						}
						docWinCtrl->StartCodeArr[index]->stringRect = stringRect;
						index++;
					}
				}
				col = col + max + 10.0;
			}
		}
	}

//	[string drawAtPoint:textPosition withAttributes:atts];

	CGContextRestoreGState(context);
}

-(CODES **)DisplayCodes:(DocumentWindowController *)docWinCtrl {
	int index, row;
	CGFloat col, max;
	NSUInteger wsize;
	NSWindow *win = [self window];
	NSRect wFrame = [win frame];
	NSFont *font;
	NSDictionary *attsW, *attsB;

	index = 0;
	font = lowerWinFont;
	if (font == nil)
		return(docWinCtrl->EndCodeArr);
	attsW = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor whiteColor], NSLigatureAttributeName:@0};
	if (attsW == nil)
		return(docWinCtrl->EndCodeArr);
	attsB = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor blackColor], NSLigatureAttributeName:@0};
	if (attsB == nil)
		return(docWinCtrl->EndCodeArr);
//	[string drawAtPoint:textPosition withAttributes:atts];


	wsize = docWinCtrl->lowerView->rowsNum - 1;

	if (docWinCtrl->SnTr.IsSoundOn == true)
		return(docWinCtrl->EndCodeArr);

	max = [self maxSize:docWinCtrl atts:attsW];
	if (max <= 0.0)
		return(docWinCtrl->EndCodeArr);

	col = 0.0;
	index = 0;
	while (true) {
		if (col > wFrame.size.width)
			return(docWinCtrl->StartCodeArr+index);
		for (row=0; row < wsize; row++) {
			if (docWinCtrl->StartCodeArr[index] == NULL) {
				return(docWinCtrl->StartCodeArr+index);
			}
			index++;
		}
		col = col + max + 10.0;
	}
	return(docWinCtrl->StartCodeArr+index);
}

-(void)FindLastPage:(DocumentWindowController *)docWinCtrl {
	int index, row;
	CGFloat col, max;
	NSUInteger wsize;
	NSWindow *win = [self window];
	NSRect wFrame = [win frame];
	NSFont *font;
	NSDictionary *attsW;

	if (docWinCtrl->StartCodeArr == docWinCtrl->RootCodesArr + 1) {
		docWinCtrl->CursorCodeArr = 0;
		return;
	}

	font = lowerWinFont;
	if (font == nil)
		return;
	attsW = @{NSFontAttributeName:font, NSForegroundColorAttributeName:[NSColor whiteColor], NSLigatureAttributeName:@0};
	if (attsW == nil)
		return;

	wsize = docWinCtrl->lowerView->rowsNum - 1;

	docWinCtrl->StartCodeArr--;
	max = [self maxSize:docWinCtrl atts:attsW];
	col = 0.0;
	index = 0;
	while (true) {
		if (col > wFrame.size.width) {
			docWinCtrl->StartCodeArr++;
			docWinCtrl->CursorCodeArr = index - 1;
			return;
		}
		for (row=0; row < wsize; row++) {
			if (docWinCtrl->StartCodeArr == docWinCtrl->RootCodesArr + 1) {
				docWinCtrl->CursorCodeArr = index;
				return;
			}
			index++;
			docWinCtrl->StartCodeArr--;
			max = [self maxSize:docWinCtrl atts:attsW];
		}
		col = col + max + 10.0;
	}
}

@end


@implementation ScalingScrollView

- (void)awakeFromNib {
	NSLog(@"ScalingScrollView: awakeFromNib\n");
	[super awakeFromNib];
	[self setAllowsMagnification:NO];
	[self setMaxMagnification:16.0];
	[self setMinMagnification:0.25];
}
@end

@implementation NSTextView (TextEditAdditions)

/* This method causes the text to be laid out in the foreground (approximately) up to the
   indicated character index.  Note that since we are adding a category on a system framework,
   we are prefixing the method with "textEdit" to greatly reduce chance of any naming conflict.
*/
- (void)textEditDoForegroundLayoutToCharacterIndex:(NSUInteger)loc {
    NSUInteger len;
	NSTextStorage *text;
	NSLayoutManager *tLayoutMgr;
	NSRange glyphRange;

	text = [self textStorage];
	if (loc > 0 && (len = [text length]) > 0) {
        if (loc >= len)
			loc = len - 1;
        /* Find out which glyph index the desired character index corresponds to */
		for (tLayoutMgr in [text layoutManagers]) {		
			glyphRange = [tLayoutMgr glyphRangeForCharacterRange:NSMakeRange(loc, 1) actualCharacterRange:NULL];
			if (glyphRange.location > 0) {
				/* Now cause layout by asking a question which has to determine where the glyph is */
				(void)[tLayoutMgr textContainerForGlyphAtIndex:glyphRange.location - 1 effectiveRange:NULL];
			}
		}
    }
}

@end


