
/*
     File: DocumentController.m
 Abstract: NSDocumentController subclass for TextEdit.
 Required to support transient documents and customized Open panel.
 
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
#import "ced.h"
#import "DocumentController.h"
#import "Document.h"
#import "AppDefaultsKeys.h"
#import "AppErrors.h"

@implementation DocumentController

- (void)awakeFromNib {
	NSLog(@"DocumentController: awakeFromNib\n");
    [self bind:@"autosavingDelay" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values." AutosavingDelay options:nil];
    displayDocumentLock = [[NSLock alloc] init];
}

- (void)dealloc {
    [self unbind:@"autosavingDelay"];
    [displayDocumentLock release];
    [super dealloc];
}

/* Create a new document of the default type and initialize its contents from the pasteboard. 
*/
- (Document *)openDocumentWithContentsOfPasteboard:(NSPasteboard *)pb display:(BOOL)display error:(NSError **)error {
	// Read type and attributed string.
	NSString *pasteboardType = [pb availableTypeFromArray:[NSAttributedString readableTypesForPasteboard:pb]];
	NSData *data = [pb dataForType:pasteboardType];
	NSAttributedString *string = nil;
	NSString *type = nil;

	if (data != nil) {
		NSDictionary *attributes = nil;
		string = [[[NSAttributedString alloc] initWithData:data options:nil documentAttributes:&attributes error:error] autorelease];

		// We only expect to see plain-text, RTF, and RTFD at this point.
		NSString *docType = [attributes objectForKey:NSDocumentTypeDocumentAttribute];
		if ([docType isEqualToString:NSPlainTextDocumentType]) {
			type = (NSString *)kUTTypeCHAT /*kUTTypeText*/;
		} else {
			type = (NSString *)kUTTypeCHAT /*kUTTypeText*/;
		}
	}

	if (string != nil && type != nil) {
		Class docClass = [self documentClassForType:type];

		if (docClass != nil) {
			id doc = [[[docClass alloc] initWithType:type error:error] autorelease];
			if (!doc) return nil; // error has been set

			NSTextStorage *text = [doc textStorage];
			[text replaceCharactersInRange:NSMakeRange(0, [text length]) withAttributedString:string];
			if ([type isEqualToString:(NSString *)kUTTypeCHAT /*kUTTypeText*/])
				[doc applyDefaultTextAttributes:NO];

			[self addDocument:doc];
			[doc updateChangeCount:NSChangeReadOtherContents];

			if (display)
				[self displayDocument:doc];

			return doc;
		}
	}

	// Either we could not read data from pasteboard, or the data was interpreted with a type we don't understand.
	if ((data == nil || (string != nil && type == nil)) && error)
		*error = [NSError errorWithDomain:TextEditErrorDomain code:TextEditOpenDocumentWithSelectionServiceFailed
								 userInfo:[NSDictionary dictionaryWithObjectsAndKeys:NSLocalizedString(@"Service failed. Couldn't open the selection.", @"Title of alert indicating error during 'New Window Containing Selection' service"), NSLocalizedDescriptionKey,
										  NSLocalizedString(@"There might be an internal error or a performance problem, or the source application may be providing text of invalid type in the service request. Please try the operation a second time. If that doesn't work, copy/paste the selection into TextEdit.", @"Recommendation when 'New Window Containing Selection' service fails"),
										  NSLocalizedRecoverySuggestionErrorKey,
										  nil]];

	return nil;
}

/* This method is overridden in order to support transient documents, i.e. the automatic closing of an automatically created untitled document, when a real document is opened.
*/
- (id)openUntitledDocumentAndDisplay:(BOOL)isDisplayDocument error:(NSError **)outError {
	NSLog(@"DocumentController: openUntitledDocumentAndDisplay\n");
	Document *doc = [super openUntitledDocumentAndDisplay:isDisplayDocument error:outError];

//	do_warning("OpenUntitledDocument", 0);  // 2019-12-05

	if (!doc)
		return nil;

	[doc set_wID:DOCWIN]; // 2019-09-06
	if ([[self documents] count] == 1) {
		// Determine whether this document might be a transient one
		// Check if there is a current AppleEvent. If there is, check whether it is an open or reopen event. In that case, the document being created is transient.
//		NSAppleEventDescriptor *evtDesc = [[NSAppleEventManager sharedAppleEventManager] currentAppleEvent];
//		AEEventID evtID = [evtDesc eventID];
	}
	return doc;
}

/* 2019-09-04
- (Document *)transientDocumentToReplace {
	 NSArray *documents = [self documents];
	 Document *transientDoc = nil;
	 return ([documents count] == 1 && [(transientDoc = [documents objectAtIndex:0]) isTransientAndCanBeReplaced]) ? transientDoc : nil;
}
*/

- (void)displayDocument:(NSDocument *)doc {
    // Documents must be displayed on the main thread.
	NSLog(@"DocumentController: displayDocument\n");
    if ([NSThread isMainThread]) {
        [doc makeWindowControllers];
        [doc showWindows];
    } else {
        [self performSelectorOnMainThread:_cmd withObject:doc waitUntilDone:YES];
    }
}

- (void)replaceTransientDocument:(NSArray *)documents {
	// Transient document must be replaced on the main thread, since it may undergo automatic display on the main thread.
	NSLog(@"DocumentController: replaceTransientDocument\n");
	if ([NSThread isMainThread]) {
		NSDocument *transientDoc = [documents objectAtIndex:0], *doc = [documents objectAtIndex:1];
		NSArray *controllersToTransfer = [[transientDoc windowControllers] copy];
		NSEnumerator *controllerEnum = [controllersToTransfer objectEnumerator];
		NSWindowController *controller;

		[controllersToTransfer makeObjectsPerformSelector:@selector(retain)];

		while (controller = [controllerEnum nextObject]) {
			[doc addWindowController:controller];
			[transientDoc removeWindowController:controller];
		}
		[transientDoc close];

		[controllersToTransfer makeObjectsPerformSelector:@selector(release)];
		[controllersToTransfer release];

		// We replaced the value of the transient document with opened document, need to notify accessibility clients.
		for (NSLayoutManager *layoutManager in [[(Document *)doc textStorage] layoutManagers]) {
			for (NSTextContainer *textContainer in [layoutManager textContainers]) {
				NSTextView *textView = [textContainer textView];
				if (textView)
					NSAccessibilityPostNotification(textView, NSAccessibilityValueChangedNotification);
			}
		}

	} else {
		[self performSelectorOnMainThread:_cmd withObject:documents waitUntilDone:YES];
	}
}

/* When a document is opened, check to see whether there is a document that is already open, and whether it is transient.
 If so, transfer the document's window controllers and close the transient document.
 When +[Document canConcurrentlyReadDocumentsOfType:] return YES, this method may be invoked on multiple threads.
 Ensure that only one document replaces the transient document. The transient document must be replaced before any
 other documents are displayed for window cascading to work correctly. To guarantee this, defer all display operations
 until the transient document has been replaced.
*/
// - (void)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler;
- (id)openDocumentWithContentsOfURL:(NSURL *)absoluteURL display:(BOOL)displayDocument error:(NSError **)outError {
	// Don't make NSDocumentController display the NSDocument it creates. Instead,
	// do it later manually to ensure that the transient document has been replaced first.
	unichar ch;
	NSUInteger len, pos, tPos;
	BOOL isIncrementPos;
	NSRange  endRange;
	Document *cDoc = [super openDocumentWithContentsOfURL:absoluteURL display:NO error:outError];
	NSTextStorage *text;
	NSString *textSt;

	if (cDoc == nil)
		return cDoc;
	[cDoc set_wID:DOCWIN]; // 2019-09-06
//2021-07-14 beg
	text = [cDoc textStorage];
	[text beginEditing];
	textSt = [text string];
	pos = 0;
	len = [text length];
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
							endRange = NSMakeRange(pos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:endRange withString:@""];
							tPos -= 2;
							endRange = NSMakeRange(tPos, 2); // (NSUInteger pos, NSUInteger len)
							[text replaceCharactersInRange:endRange withString:@""];
							if (cDoc->pos1C > pos)
								cDoc->pos1C -= 2;
							if (cDoc->pos2C > pos)
								cDoc->pos2C -= 2;
							if (cDoc->pos1C > tPos)
								cDoc->pos1C -= 2;
							if (cDoc->pos2C > tPos)
								cDoc->pos2C -= 2;
							endRange = NSMakeRange(pos, tPos-pos);
							[text addAttribute:NSForegroundColorAttributeName value:[NSColor redColor]
										 range:endRange];
							textSt = [text string];
							len = [text length];
							ch = [textSt characterAtIndex:pos];
							isIncrementPos = false;
							break;
						}
					}
					tPos++;
				}
			} else // 2020-07-27 end
				if (ch == underline_start) { // 2021-07-12 beg
					tPos = pos + 2;
					while (tPos < len) {
						ch = [textSt characterAtIndex:tPos];
						if (ch == '\n')
							break;
						if (ch == ATTMARKER) {
							ch = [textSt characterAtIndex:tPos+1];
							if (ch == underline_end) {
								endRange = NSMakeRange(pos, 2); // (NSUInteger pos, NSUInteger len)
								[text replaceCharactersInRange:endRange withString:@""];
								tPos -= 2;
								endRange = NSMakeRange(tPos, 2); // (NSUInteger pos, NSUInteger len)
								[text replaceCharactersInRange:endRange withString:@""];
								if (cDoc->pos1C > pos)
									cDoc->pos1C -= 2;
								if (cDoc->pos2C > pos)
									cDoc->pos2C -= 2;
								if (cDoc->pos1C > tPos)
									cDoc->pos1C -= 2;
								if (cDoc->pos2C > tPos)
									cDoc->pos2C -= 2;
								endRange = NSMakeRange(pos, tPos-pos);              // @(NSUnderlineStyleThick)
								[text addAttribute:NSUnderlineStyleAttributeName value:@(NSUnderlineStyleSingle)
											 range:endRange];
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
		}  // 2021-07-12 end
		if (isIncrementPos) {
			pos++;
		}
	}
	[text endEditing];
//2021-07-14 end
	if (displayDocument) {
		[displayDocumentLock lock];
/*
		if (deferredDocuments) {
			// Defer displaying this document, because the transient document has not yet been replaced.
			[deferredDocuments addObject:doc];
			[displayDocumentLock unlock];
		} else {
*/
			// The transient document has been replaced, so display the document immediately.
			[displayDocumentLock unlock];
			[self displayDocument:cDoc];
//			}
	}
	return cDoc;
}

- (void)addDocument:(NSDocument *)newDoc {
	int cnt; // 2019-09-04
	Document *firstDoc;
	NSArray *documents = [self documents];
	cnt = [documents count];
	if (cnt == 1) {
		firstDoc = [documents objectAtIndex:0];
	}
	[super addDocument:newDoc];
}

/* Overridden to add an accessory view to the open panel. This method is called for both modal and non-modal invocations.
*/
- (void)beginOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)types completionHandler:(void (^)(NSInteger result))completionHandler {
    [super beginOpenPanel:openPanel forTypes:types completionHandler:^(NSInteger result) {
        if (result == NSOKButton) {
        }
        completionHandler(result);
    }];
    
}

/* The user can change the default document type between Rich and Plain in Preferences. We override
   -defaultType to return the appropriate type string. 
*/
- (NSString *)defaultType { // 2019-08-16
//	return (NSString *)([[NSUserDefaults standardUserDefaults] boolForKey:RichText] ? kUTTypeRTF : kUTTypeCHAT /*kUTTypeText*/);
//	return (NSString *)(kUTTypeUTF8PlainText); // (NSString *)(kUTTypePlainText); // (NSString *)(kUTTypeCHAT /*kUTTypeText*/);
	return (NSString *)(kUTTypeCHAT /*kUTTypeText*/);
}

@end
