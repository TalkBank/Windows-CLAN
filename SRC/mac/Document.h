
/*
     File: Document.h
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

#import <Cocoa/Cocoa.h>

enum {
	NoStringEncoding = 0xFFFFFFFF
};

#define NUMCOLORPOINTS 5
#define cCOLORTEXTLIST struct cctext
cCOLORTEXTLIST {
	char cWordFlag;
	unichar *keyWord;
	unichar *fWord;
	int len;
	int red;
	int green;
	int blue;
	int index;
	long sCol[NUMCOLORPOINTS], eCol[NUMCOLORPOINTS];
	cCOLORTEXTLIST *nextCT;
} ;

@interface Document : NSDocument /*<NSProgressReporting>*/ {
    // Book-keeping
    BOOL setUpPrintInfoDefaults;/* YES the first time -printInfo is called */
    BOOL inDuplicate;
    // Document data
    NSTextStorage *textStorage;	/* The (styled) text content of the document */
    BOOL isReadOnly;			/* The document is locked and should not be modified */
    NSColor *backgroundColor;	/* The color of the document's background */
    NSSize viewSize;			/* The view size, as stored in an RTF document. Can be NSZeroSize */
    BOOL usesDocScreenFonts;	/* The document allows using screen fonts */
	NSFont *docFont;
	CGFloat fontHeight;

    // Information about how the document was created
    NSArray *originalOrientationSections; /* An array of dictionaries. Each describing the text layout orientation for a page */
    
    // Temporary information about how to save the document
    NSStringEncoding documentEncodingForSaving;	 /* NSStringEncoding for saving the document */
    NSSaveOperationType currentSaveOperation;    /* So we can know whether to use documentEncodingForSavingin -fileWrapperOfType:error: */

    // Temporary information about document's desired file type
    NSString *fileTypeToSet;		/* Actual file type determined during a read, and set after the read (which includes revert) is complete. */ 
@public
	long top; // 2020-03-13
	long left;
	long height;
	long width;
	long topC;
	long skipTop;
	long pos1C;
	long skipP1;
	long pos2C;
	long skipP2;
	char filePath[CS_MAX_PATH+FILENAME_MAX]; // 2020-03-12
	char mediaFileName[FILENAME_MAX]; // 2020-03-12
	char pidSt[256]; // 2020-05-08
	cCOLORTEXTLIST *RootColorText;
	unsigned short wID;
	BOOL rawTextInput;
	BOOL ShowPercentOfFile;
	BOOL isCAFont;

}

#define kUTTypeCHAT @"org.talkbank"

/* View size (as it should be saved in a RTF file) */
- (NSFont *)docFont;
- (void)setDocFont:(NSFont *)newDocFont;

/* View size (as it should be saved in a RTF file) */
- (CGFloat)fontHeight;
- (void)setFontHeight:(CGFloat)newFontHeight;

/* Is the document read-only? */
- (BOOL)isReadOnly;
- (void)setReadOnly:(BOOL)flag;

/* Document background color */
- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

/* Encoding of the document chosen when saving */
- (NSUInteger)encodingForSaving;
- (void)setEncodingForSaving:(NSUInteger)SaveEncoding;

/* View size (as it should be saved in a RTF file) */
- (NSSize)viewSize;
- (void)setViewSize:(NSSize)newSize;

/* Attributes */ // This will _copy_ the contents of the NS[Attributed]String ts into the document's textStorage.
- (NSTextStorage *)textStorage;
- (void)setTextStorage:(id)ts;

/* Page-oriented methods */
- (NSSize)paperSize;
- (void)setPaperSize:(NSSize)size;

/* Action methods */
- (IBAction)openSpecialCharacters:(id)sender;
- (IBAction)openKeysList:(id)sender;

- (IBAction)toggleReadOnly:(id)sender;

/* Default text attributes for plain or rich text formats */
- (NSDictionary *)defaultTextAttributes:(BOOL)forRichText;
- (void)applyDefaultTextAttributes:(BOOL)forRichText;

/* Layout orientation sections */
- (NSArray *)originalOrientationSections;
- (void)setOriginalOrientationSections:(NSArray *)array;

/* Screen fonts property */
- (BOOL)usesDocScreenFonts;
- (void)setDocUsesScreenFonts:(BOOL)aFlag;

/* Document window's ID # 2019-09-06 */
- (void)set_wID:(unsigned short )num;
- (unsigned short)get_wID;

extern void RunCommand(NSString *comStr);
extern NSUInteger getUtts(NSUInteger pos, NSUInteger len, NSString *textSt);

@end

@interface ProgressController : NSWindowController <NSLayoutManagerDelegate, NSTextFieldDelegate> {
}
@end
