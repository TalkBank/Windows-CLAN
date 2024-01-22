#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVPlayerView.h>

enum {
	stopped = 1,
	oneBullet,
	F_five,
	ESC_eight,
	Walker
} ;

struct AVInfoNextSeg {
	char mediaFName[CS_MAX_PATH+FILENAME_MAX];
	long beg, end;
	struct AVInfoNextSeg *nextSeg;
} ;

struct AVInfo {
	char mediaFPath[CS_MAX_PATH+FILENAME_MAX]; // 2020-03-12
	char mediaFName[CS_MAX_PATH+FILENAME_MAX];  // 2020-03-12
	char isWhatType; // isAudio isVideo
	long beg, end;
	char playMode;
	long endWalker;
	long endContPlay;
	NSWindow *docWindow;
	NSTextView *textView;
	struct AVInfoNextSeg *nextSegs;
} ;

@interface AVController : NSWindowController <NSLayoutManagerDelegate>  {
	long duration, LoopCnt;
	BOOL isSettingSize;
	NSTextView *textView;
	NSWindow *docWindow;
@public
	long beg, end, endContPlay;
	char rMovieFile[CS_MAX_PATH+FILENAME_MAX];
	char isWhatType; // isAudio isVideo
	struct AVInfoNextSeg *nextSegs;
	char playMode;
	long endWalker;
	long lastEndBullet;
	BOOL isPlayerItemObserverSet;
	void *playerItemContext;
	AVAudioPlayer *audioPlayer;
	AVPlayerItem *playerItem;
	IBOutlet AVPlayerView *playerView;
	id timeObserverToken;
	NSTimer *audioTimer;
}

+(void)createAndPlayAV:(AVInfo *)AVinfo;

- (void)StopAV;

@end

extern AVController *AVMediaPlayer;
