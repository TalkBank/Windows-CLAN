
/*
     File: DocumentWindowController.h
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

#import <Cocoa/Cocoa.h>
#import "ced.h"
#import "NoodleLineNumberView.h"
#import "NoodleLineNumberMarker.h"
#import "AVController.h"

#ifdef _CLAN_DEBUG
#define DEBUGCODE
#endif

#define sndInfoC struct SOUNDINFOC
struct SOUNDINFOC {
	BOOL  IsSoundOn;
	BOOL  IsPlayMedia;
	BOOL  isUpdateSlider;
	char  StatusLineType;
	UInt32 WaveWidth;
	Float32 *TopP1;
	Float32 *TopP2;
	UInt32 VScale;
	NSInteger HScale;
	long  BegF, EndF;
	long  WBegF, WEndF;
	long  SoundFileSize;
	UInt32 ctrlWidth;
	AudioFormatID SNDformat;
	UInt32 SNDsample;
	UInt32 SNDchan;
	Float64 SNDrate;
	Float64 mSampleRate; //2021-04-25
	UInt32 mBitsPerChannel;
	FNType mediaFPath[CS_MAX_PATH+FILENAME_MAX];

} ;


@interface ScalingScrollView : NSScrollView

@end

@interface LowerTextView : NSView { // 2020-04-28
	char lowerInfoString[2048+2];
@public
	NSFont *lowerWinFont;
	CGFloat lowerFontHeight;
	NSUInteger rowsNum; // 2020-09-24
}

- (void)setLowerFont:(NSFont *)newFont;// 2020-04-28
- (void)setLowerFontHeight:(CGFloat)newFontHeight; // 2020-09-24

- (void)setInfoString:(char *)newInfoString;// 2020-04-28
- (char *)getInfoString;// 2020-06-08

- (void)mouseDown:(NSEvent*)theEvent; // 2020-05-06
- (void)rightMouseDown:(NSEvent*)theEvent; // 2020-05-06

- (void)setRowsCnt:(NSUInteger)newRowsCnt; // 2020-09-24
- (NSUInteger)getRowsCnt; // 2020-09-24

@end

#define picBullet 2
#define oldBullet 1
#define newBullet 0

#define SPEAKERNAMENUM 10
#define SPEAKERNAMELEN 128

@interface DocumentWindowController : NSWindowController <NSLayoutManagerDelegate, NSTextViewDelegate> {
	IBOutlet ScalingScrollView *scrollView;
	NSLayoutManager *layoutMgr;
	BOOL rulerIsBeingDisplayed;
	BOOL isSettingSize;
	BOOL ShowParags;
	NSString *textStOrig;
	NSRange cursorRangeOrig;

	BOOL isAddingChar; // 2021-07-14
	char isF1_2Key;// 2019-10-20
	char isESCKey; // 2019-11-08
	char cursorKeyDir;
	id docWinEventMonitor;// 2019-10-20

	unCH SpeakerNames[SPEAKERNAMENUM][SPEAKERNAMELEN+1]; // 2020-09-10

	struct AVInfo theAVinfo;

// progress bar variables beg
	BOOL isExportDone;
	NSTimer *exportProgressBarTimer;
	NSView *contentView;
	NSTextField *label;
	NSProgressIndicator *progInd;
	NSWindow *progSheet;
	AVAssetExportSession *exportSession;
// progress bar variables end

@public
	IBOutlet NSSlider *LowerSlider;
	IBOutlet LowerTextView *lowerView;
	BOOL ChatMode;
	BOOL TrimTierMode;

	// sonic mode beg
	sndInfoC SnTr;
	// sonic mode end

	// coder mode beg
	BOOL EditorMode;
	BOOL NoCodes;
	BOOL MakeBackupFile;
	BOOL CodingScheme;
	BOOL PriorityCodes;
	BOOL EnteredCode;
	unCH OldCode[SPEAKERLEN];
	FNType  *cod_fname;
	int  FreqCountLimit;
	int  CursorCodeArr;
	CODES *RootCodes, *CurCode;
	CODES **StartCodeArr, **EndCodeArr, **RootCodesArr;
	CODES *FirstCodeOfPrevLevel, *CurFirstCodeOfPrevLevel;
	// coder mode end

	// Disambiguate MOR beg
	BOOL isDisambiguateAgain;
	// Disambiguate MOR end
}

@property (retain)  NoodleLineNumberView *lineNumberView;

// Convenience initializer. Loads the correct nib automatically.
- (id)init;

- (NSUInteger)numberOfPages;

- (NSView *)documentView;

- (void)breakUndoCoalescing;

- (void)setupTextViewForDocument;

- (NSTextView *)firstTextView;

/* Layout orientation sections */
- (NSArray *)layoutOrientationSections;

- (IBAction)ContinuousPlayback:(id)sender; // 2020-09-01

- (IBAction)ContinuousSkipPausePlayback:(id)sender; // 2021-08-03

- (IBAction)PlayBulletMedia:(id)sender; // 2020-09-01

- (IBAction)TranscribeSoundOrMovie:(id)sender; // 2020-09-01

- (IBAction)ExpandBullets:(id)sender; // 2020-09-01

- (IBAction)CheckOpenedFile:(id)sender; // 2020-09-01

- (IBAction)DefineIDs:(id)sender; // 2020-09-01

- (void)SetUpParticipants:(BOOL)isShowErr; // 2020-09-10 beg
- (void)AllocSpeakerNames:(unCH *)st index:(int)sp; // 2020-09-10
- (IBAction)UpdateTiersSpeakers:(id)sender; // 2020-09-10

- (IBAction)UnderlineText:(id)sender; // 2021-07-15

- (IBAction)EditMode:(id)sender;

- (IBAction)DisambiguateMorTier:(id)sender;

- (void)setupWindowForDocument;

- (NSLayoutManager *)myLayoutManager;

- (NSUInteger)countLineNumbers:(NSUInteger)cursorPos;

- (void)setStatusLine:(NSUInteger)lineNumber extraMess:(char *)mess isUpdateDisplay:(BOOL)isDisplay;

- (void)findMediaFile:(BOOL)isSend;

- (BOOL)getCurrentMediaName:(NSString *)textSt MaxLen:(NSUInteger)len showErr:(BOOL)isShowErr;

- (BOOL)isFindBullet:(NSString *)textSt Index:(NSUInteger *)cPos MaxLen:(NSUInteger)len AVInformation:(struct AVInfo *)AVinfo;

- (IBAction)InsertBulletIntoText:(id)sender;

- (IBAction)SoundToTextSync:(id)sender;

- (NSUInteger)isOldBullet:(char *)fn txt:(NSString *)textSt tpos:(NSUInteger)pos tlen:(NSUInteger)len;

- (NSUInteger)isLegalBullet:(NSUInteger)pos bTest:(char *)bType;

@end

@interface DocumentWindowController(SonicModeCode)

- (IBAction)lowerSliderChanged:(id)sender;

- (void)DisposeOfSoundData;

- (void)DisplayEndF;

- (void)soundwindow;

- (IBAction)SonicMode:(id)sender;

- (void)addBulletsToText:(const char *)tagName begin:(long)begin end:(long)end;

- (long)conv_to_msec_rep:(long)num;

- (long)conv_from_msec_rep:(double)num;

@end

// coder mode beg
BOOL init_codes(const FNType *fname, char *fontName, DocumentWindowController *docWinController);
extern int FindRightCode(int disp, DocumentWindowController *docWinController);
extern int GetNewCodes(int i, DocumentWindowController *docWinController);
extern int ToTopLevel(int i, DocumentWindowController *docWinController);
extern int GetCursorCode(int i, DocumentWindowController *docWinController);
extern int GetFakeCursorCode(int i, DocumentWindowController *docWinController);
extern int EndCurTierGotoNext(int i, DocumentWindowController *docWinController);
extern int MoveCodeCursorUp(DocumentWindowController *docWinController);
extern int MoveCodeCursorDown(DocumentWindowController *docWinController);
extern int MoveCodeCursorLeft(DocumentWindowController *docWinController);
extern int MoveCodeCursorRight(DocumentWindowController *docWinController);
extern void FreeCodesMem(DocumentWindowController *docWinController);
extern void MoveToSubcode(DocumentWindowController *docWinController);
extern void MapArrToList(CODES *CurCode, DocumentWindowController *docWinController);
extern void AddCodeTier(unCH *code, char real_code, NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController);
extern CODES **DisplayCodes(DocumentWindowController *docWinController);
// coder mode end
// Disambiguate MOR beg
extern int MorDisambiguate(DocumentWindowController *docWinController);
// Disambiguate MOR end

extern DocumentWindowController *gLastTextWinController;

#define ScrollBarHeight 20
