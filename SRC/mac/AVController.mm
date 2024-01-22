#import "DocumentWinController.h"
#import "WalkerController.h"
#import "PictController.h"
#import <CoreMedia/CoreMedia.h>
#import "Controller.h"

extern struct DefWin MovieWinSize;
extern FNType prefsDir[];
extern char wd_dir[];
extern char ced_err_message[];
extern struct PBCs PBC;
extern DocumentWindowController *getDocWinController(NSWindow *win);

AVController *AVMediaPlayer = nil;

@implementation AVController // 2020-02-27

- (id)init
{
	AVMediaPlayer = nil;
	if ((self = [super init])) {
		// initialization
	}
	return self;
//	return [super initWithWindowNibName:@"AVController"];
}

- (void)StopAV {
	NSUInteger j;
	NSRange cursorRange;
	DocumentWindowController *docWinCtrl;
	struct AVInfoNextSeg *tSeg;

	if (isWhatType == isAudio) {
		if (audioTimer != nil) {
			[audioTimer invalidate];
			audioTimer = nil;
		}
		[audioPlayer stop];
		if (playMode == oneBullet || playMode == Walker || playMode == F_five || playMode == ESC_eight) {
			docWinCtrl = getDocWinController(docWindow);
			textView = [docWinCtrl firstTextView];
			cursorRange = [textView selectedRange];
			playMode = stopped;
			j = [docWinCtrl countLineNumbers:cursorRange.location];
			[docWinCtrl setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
		}
	} else {
		playerView.player.rate = 0.0;
//		if (timeObserverToken != nil) {
//			[playerView.player removeTimeObserver:timeObserverToken];
//  	      timeObserverToken = nil;
//		}
	}
	playMode = stopped;
	duration = -1;
	while (nextSegs != nil) {
		tSeg = nextSegs;
		nextSegs = nextSegs->nextSeg;
		free(tSeg);
	}
}

- (void)nextHighlight:(long)endValue {
	char buf[BUFSIZ], *bufEnd, bType;
	char FName[FILENAME_MAX];
	long beg, end;
	unichar ch, bufU[BUFSIZ];
	BOOL isSPFound;
	NSTextStorage *text;
	NSString *textSt;
	NSRange cursorRange;
	NSUInteger pos, j, len, bufLen, prev_pos, jCR, bulletPosBeg, bulletPosEnd;
	DocumentWindowController *docWinCtrl;

	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	pos = cursorRange.location + cursorRange.length;
	prev_pos = pos;

	isSPFound = false;
	while (pos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (ch == 0x2022) {
			docWinCtrl = getDocWinController(docWindow);
			bulletPosBeg = pos;
			bulletPosEnd = [docWinCtrl isLegalBullet:pos bTest:&bType];
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
						extractPath(FileName1, rMovieFile);
						UnicodeToUTF8(bufU, strlen(bufU), (unsigned char *)templineC2, NULL, UTTLINELEN);
						strcpy(FileName2, templineC2);
						[PictController openPict:FileName2 curDocPath:(FNType *)FileName1 docWin:[self window]];
						[docWindow makeKeyAndOrderFront:nil];
						[docWindow setOrderedIndex:0];
					}
				} else {
					if (bType == oldBullet) {
						bulletPosBeg++;
						bulletPosBeg = [docWinCtrl isOldBullet:FName txt:textSt tpos:bulletPosBeg tlen:len];
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
						beg = atol(buf);
						end = atol(bufEnd+1);
						if (beg < endValue) {
							isSPFound = true;
							endContPlay = end;
							if (docWinCtrl->SnTr.IsSoundOn == true) {
								docWinCtrl->SnTr.BegF = [docWinCtrl conv_from_msec_rep:beg];
								docWinCtrl->SnTr.EndF = [docWinCtrl conv_from_msec_rep:end];
								[docWinCtrl DisplayEndF];
							}
							break;
						}
					}
				}
				pos = bulletPosEnd;
			}
		}
		pos++;
	}
	if (isSPFound) {
		j = pos;
		while (j > 0) {	// Run through the whole text in NSTextStorage *text
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
		cursorRange = NSMakeRange(j, 0); // (NSUInteger pos, NSUInteger len)
		while (pos < len) {	// Run through the whole text in NSTextStorage *text
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				cursorRange.length = pos - cursorRange.location; // 2020-12-02
				break;
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
			}
			pos++;
		}
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
	}
}

- (void)currentPlayHeadTime {
	long curTime;
	NSTimeInterval currentTime, endValue;
	struct AVInfoNextSeg *tSeg;

	if (isWhatType == isAudio)
		currentTime = audioPlayer.currentTime * 1000.0000;
	else
		currentTime = CMTimeGetSeconds(playerView.player.currentItem.currentTime) * 1000.0000;
	curTime = (long)currentTime;
	if (playMode == ESC_eight && curTime > endContPlay) {
		[self nextHighlight:curTime];
	}
	endValue = (NSTimeInterval)end;
	if (currentTime >= endValue) {
		if (playMode == Walker) {
			if (PBC.pause_len > 0) {
				playerView.player.rate = 0.0;
				dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
					//Here your non-main thread.
					[NSThread sleepForTimeInterval:PBC.pause_len/1000.0000];
					dispatch_async(dispatch_get_main_queue(), ^{
						duration = -1;
						if (isWhatType == isAudio) {
							if (audioTimer != nil) {
								[audioTimer invalidate];
								audioTimer = nil;
							}
						} else {
							if (timeObserverToken != nil) {
								[playerView.player removeTimeObserver:timeObserverToken];
								timeObserverToken = nil;
							}
						}
						if (LoopCnt > 1) {
							LoopCnt = LoopCnt - 1;
							if (isWhatType == isAudio)
								[self PlayAudio:NO];
							else
								[self PlayVideo];
						} else if (endWalker == -1 || currentTime < endWalker) {
							LoopCnt = PBC.LoopCnt;
							beg = currentTime - PBC.backspace;
							end = beg + PBC.step_length;
							if (endWalker > -1) {
								if (end > endWalker)
									end = endWalker;
							}
							if (isWhatType == isAudio)
								[self PlayAudio:NO];
							else
								[self PlayVideo];
						} else {
							[self StopAV];
						}
					});
				});
			} else {
				playerView.player.rate = 0.0;
				duration = -1;
				if (isWhatType == isAudio) {
					if (audioTimer != nil) {
						[audioTimer invalidate];
						audioTimer = nil;
					}
				} else {
					if (timeObserverToken != nil) {
						[playerView.player removeTimeObserver:timeObserverToken];
						timeObserverToken = nil;
					}
				}
				if (LoopCnt > 1) {
					LoopCnt = LoopCnt - 1;
					if (isWhatType == isAudio)
						[self PlayAudio:NO];
					else
						[self PlayVideo];
				} else if (endWalker == -1 || currentTime < endWalker) {
					LoopCnt = PBC.LoopCnt;
					beg = currentTime - PBC.backspace;
					end = beg + PBC.step_length;
					if (endWalker > -1) {
						if (end > endWalker)
							end = endWalker;
					}
					if (isWhatType == isAudio)
						[self PlayAudio:NO];
					else
						[self PlayVideo];
				} else {
					[self StopAV];
				}
			}
		} else if (playMode == ESC_eight) {
			if (nextSegs == nil)
				[self StopAV];
			else {
				playerView.player.rate = 0.0;
				duration = -1;
				if (isWhatType == isAudio) {
					if (audioTimer != nil) {
						[audioTimer invalidate];
						audioTimer = nil;
					}
				} else {
					if (timeObserverToken != nil) {
						[playerView.player removeTimeObserver:timeObserverToken];
						timeObserverToken = nil;
					}
				}
				beg = nextSegs->beg;
				end = nextSegs->end;
				tSeg = nextSegs;
				nextSegs = nextSegs->nextSeg;
				free(tSeg);
				if (isWhatType == isAudio)
					[self PlayAudio:NO];
				else
					[self PlayVideo];
			}
		} else {
			[self StopAV];
		}
	} else if (currentTime >= duration) {
		[self StopAV];
	}
	NSLog(@"currentTime %ld", curTime);
}

- (void)PlayAudio:(BOOL)isNewFile {
	float rate;
	NSTimeInterval btime;
	NSURL *MovieFileUrl;
	BOOL result;
	NSUInteger j;
	NSRange cursorRange;
	DocumentWindowController *docWinCtrl;

	NSLog(@"AVController: PlayAudio\n");
	if (isNewFile == YES) {
		if (audioPlayer != nil) {
			[audioPlayer release];
			audioPlayer = nil;
		}
		audioPlayer = [AVAudioPlayer alloc];
		MovieFileUrl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:rMovieFile]];
		audioPlayer = [audioPlayer initWithContentsOfURL:MovieFileUrl error:nil];
		result = [audioPlayer prepareToPlay];
	} else
		result = YES;
	if (result == YES) {
		audioPlayer.enableRate = YES;
		if (PBC.speed < 100) {
			rate = (1.0000 * PBC.speed) / 100.0000;
		} else if (PBC.speed > 100) {
			rate = (1.0000 * PBC.speed) / 100.0000;
		} else {
			rate = 1.0;
		}
		audioPlayer.rate = rate;
		duration = audioPlayer.duration * 1000.0000;
		if (beg >= duration) {
			do_warning_sheet("Starting point is beyond end of sound file.", docWindow);
			return;
		}
		if (end == -1L) {
			end = duration;
			if (end <= 0.0) {
				do_warning_sheet("Failed to get media file length", docWindow);
				return;
			}
		}
		if (playMode == oneBullet || playMode == Walker || playMode == F_five || playMode == ESC_eight) {
			docWinCtrl = getDocWinController(docWindow);
			textView = [docWinCtrl firstTextView];
			cursorRange = [textView selectedRange];
			j = [docWinCtrl countLineNumbers:cursorRange.location];
			[docWinCtrl setStatusLine:j extraMess:ced_err_message isUpdateDisplay:YES];
		}
		lastEndBullet = beg;
		btime = beg / 1000.0000;
		audioPlayer.currentTime = btime;
//		result = [audioPlayer playAtTime:btime];
		result = [audioPlayer play];
		if (result == YES) {
			audioTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:AVMediaPlayer selector:@selector(currentPlayHeadTime) userInfo:nil repeats:YES];
		}
	}
}

- (void)PlayVideo {
	float rate;

	NSLog(@"AVController: PlayVideo\n");

	AVPlayerItem *playerCurrentItem;
	playerCurrentItem = playerView.player.currentItem;
	if (PBC.speed < 100 && playerCurrentItem.canPlaySlowForward) {
		rate = (1.0 * PBC.speed) / 100.0000;
	} else if (PBC.speed > 100 && playerCurrentItem.canPlayFastForward) {
		rate = (1.0 * PBC.speed) / 100.0000;
	} else {
		rate = 1.0;
	}
	CMTime cmDuration;
	long lduration;
	cmDuration = playerCurrentItem.duration;
	lduration = CMTimeGetSeconds(cmDuration) * 1000.0000;
	playerView.player.rate = 1.0;
	[playerView.player seekToTime:(CMTime)CMTimeMake((int64_t)beg, 1000) //CMTimeMakeWithSeconds(bef, 1)
				toleranceBefore:(CMTime)kCMTimeZero toleranceAfter:(CMTime)kCMTimeZero];
//              toleranceBefore:(CMTime)kCMTimePositiveInfinity toleranceAfter:(CMTime)kCMTimePositiveInfinity];
	CMTime interval = CMTimeMakeWithSeconds(0.1, NSEC_PER_SEC);
	dispatch_queue_t mainQueue = dispatch_get_main_queue();

	__weak typeof(self) weakSelf = self;
	if (timeObserverToken == nil) {
		lastEndBullet = beg;
		timeObserverToken = [playerView.player addPeriodicTimeObserverForInterval:interval queue:mainQueue
										 usingBlock:^(CMTime time) {
										 #pragma unused (time)
											 // Use weak reference to self
											 if (duration == -1) {
												 Float64 dur = CMTimeGetSeconds(playerView.player.currentItem.duration);
												 duration = dur * 1000.0000;
											 }
											 if (end == -1L) {
												 end = duration;
												 if (end <= 0.0) {
													 do_warning_sheet("Failed to get media file length", docWindow);
													 [playerView.player removeTimeObserver:timeObserverToken];
													 timeObserverToken = nil;
													 return;
												 }
											 }
											 if (end > duration)
												 end = duration;
											 if (duration != -1 && beg >= duration) {
												 do_warning_sheet("Starting point is beyond end of video file.", docWindow);
												 [playerView.player removeTimeObserver:timeObserverToken];
												 timeObserverToken = nil;
												 return;
											 }
											 [weakSelf currentPlayHeadTime];
											 // Update player transport UI
										 }];
	}
	if (duration != -1 && beg >= duration) {
		do_warning_sheet("Starting point is beyond end of video file.", docWindow);
		return;
	}
	[playerView.player playImmediatelyAtRate:rate]; // play];
	if (playMode == oneBullet) { // 2020-12-02
		[docWindow makeKeyAndOrderFront:nil];
		[docWindow setOrderedIndex:0];
	}
}

static BOOL findFullMediaName(AVInfo *AVinfo) {
	int pathLen, nameLen;
	char *rFileName;
	BOOL isFirstTimeAround;

	isFirstTimeAround = true;
	rFileName = AVinfo->mediaFPath;
	pathLen = strlen(rFileName);
tryAgain:
	strcat(rFileName, AVinfo->mediaFName);
	nameLen = strlen(rFileName);
	strcat(rFileName, ".mov");
	AVinfo->isWhatType = isVideo;
	if (access(rFileName, 0)) {
		rFileName[nameLen] = EOS;
		uS.str2FNType(rFileName, strlen(rFileName), ".mp4");
		AVinfo->isWhatType = isVideo;
		if (access(rFileName, 0)) {
			rFileName[nameLen] = EOS;
			uS.str2FNType(rFileName, strlen(rFileName), ".mp3");
			AVinfo->isWhatType = isAudio;
			if (access(rFileName, 0)) {
				rFileName[nameLen] = EOS;
				uS.str2FNType(rFileName, strlen(rFileName), ".wav");
				AVinfo->isWhatType = isAudio;
				if (access(rFileName, 0)) {
					rFileName[nameLen] = EOS;
					uS.str2FNType(rFileName, strlen(rFileName), ".m4v");
					AVinfo->isWhatType = isVideo;
					if (access(rFileName, 0)) {
						rFileName[nameLen] = EOS;
						uS.str2FNType(rFileName, strlen(rFileName), ".aif");
						AVinfo->isWhatType = isAudio;
						if (access(rFileName, 0)) {
							rFileName[nameLen] = EOS;
							uS.str2FNType(rFileName, strlen(rFileName), ".avi");
							AVinfo->isWhatType = isVideo;
							if (access(rFileName, 0)) {
								rFileName[nameLen] = EOS;
								uS.str2FNType(rFileName, strlen(rFileName), ".wmv");
								AVinfo->isWhatType = isVideo;
								if (access(rFileName, 0)) {
									rFileName[nameLen] = EOS;
									uS.str2FNType(rFileName, strlen(rFileName), ".mpg");
									AVinfo->isWhatType = isVideo;
									if (access(rFileName, 0)) {
										rFileName[nameLen] = EOS;
										uS.str2FNType(rFileName, strlen(rFileName), ".aiff");
										AVinfo->isWhatType = isAudio;
										if (access(rFileName, 0)) {
											rFileName[nameLen] = EOS;
											uS.str2FNType(rFileName, strlen(rFileName), ".dv");
											AVinfo->isWhatType = isVideo;
											if (access(rFileName, 0)) {
												if (isFirstTimeAround) {
													isFirstTimeAround = false;
													rFileName[pathLen] = EOS;
													strcat(rFileName, "media/");
													goto tryAgain;
												}
												rFileName[nameLen] = EOS;
												AVinfo->isWhatType = 0;
												return(false);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return(true);
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
						change:(NSDictionary<NSString *,id> *)change context:(void *)context {
	// Only handle observations for the PlayerItemContext
	if (context != &playerItemContext) {
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
		return;
	}

	if ([keyPath isEqualToString:@"status"]) {
		AVPlayerItemStatus status = AVPlayerItemStatusUnknown;
		// Get the status change from the change dictionary
		NSNumber *statusNumber = change[NSKeyValueChangeNewKey];
		if ([statusNumber isKindOfClass:[NSNumber class]]) {
			status = statusNumber.integerValue;
		}
		// Switch over the status
		switch (status) {
			case AVPlayerItemStatusReadyToPlay:
				[self PlayVideo];
				break;
			case AVPlayerItemStatusFailed:
				// Failed. Examine AVPlayerItem.error
				do_warning_sheet("Failed to loading media player", docWindow);
				break;
			case AVPlayerItemStatusUnknown:
				// Not ready
				break;
		}
	}
}

+ (void)createAndPlayAV:(AVInfo *)AVinfo;
{
	char err_mess[ERRMESSAGELEN];
	NSString *nMovieFile;
	AVURLAsset *clip;
	NSKeyValueObservingOptions options;

	strcpy(err_mess, "Can't locate media filename:\n");
	strcat(err_mess, AVinfo->mediaFPath);
	addFilename2Path(err_mess, AVinfo->mediaFName);
	strcat(err_mess, "\n or:\n");
	if (!findFullMediaName(AVinfo)) {
		strcat(err_mess, AVinfo->mediaFPath);
		do_warning_sheet(err_mess, AVinfo->docWindow);
		return;
	}
/*
	if (AVinfo->isWhatType == isAudio) {
		UInt32 thePropertySize;
		ExtAudioFileRef sourceFile = nil;
		AudioStreamBasicDescription srcFormat;
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:AVinfo->mediaFPath]];

		ExtAudioFileOpenURL((__bridge CFURLRef)url, &sourceFile);
		thePropertySize = sizeof(srcFormat); //UInt32(MemoryLayout.stride(ofValue: srcFormat));;
		ExtAudioFileGetProperty(sourceFile, kExtAudioFileProperty_FileDataFormat, &thePropertySize, &srcFormat);
		if (srcFormat.mChannelsPerFrame > 1) {
		}
	}
*/
//	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(interruption:) name:AVAudioSessionInterruptionNotification object:nil];
/*
	- (void)interruption:(NSNotification *)notification {
		NSDictionary *userInfo = notification.userInfo;
		if ([[userInfo objectForKey:AVAudioSessionInterruptionTypeKey] integerValue] == AVAudioSessionInterruptionTypeBegan) {
			NSLog(@"Audio session was interrupted");
			[self.audioPlayer pause];
		} else if ([[userInfo objectForKey:AVAudioSessionInterruptionTypeKey] integerValue] == AVAudioSessionInterruptionTypeEnded) {
			NSLog(@"Audio interruption ended");
			if ([self.audioPlayer prepareToPlay]) {
				NSLog(@"Will resume playing audio");
				[self.audioPlayer play];
			} else {
				NSLog(@"Failed to prepare to play");
			}
		}
	}
*/
	if (AVMediaPlayer == nil) {
		AVMediaPlayer = [[AVController alloc] initWithWindowNibName:@"AVController"];
		strcpy(AVMediaPlayer->rMovieFile, AVinfo->mediaFPath);
		AVMediaPlayer->beg = AVinfo->beg;
		AVMediaPlayer->end = AVinfo->end;
		AVMediaPlayer->lastEndBullet = AVMediaPlayer->beg;
		AVMediaPlayer->audioTimer = nil;
		AVMediaPlayer->timeObserverToken = nil;
		AVMediaPlayer->isPlayerItemObserverSet = NO;
		AVMediaPlayer->LoopCnt = PBC.LoopCnt;
		AVMediaPlayer->duration = -1;
		AVMediaPlayer->isWhatType = AVinfo->isWhatType;
		AVMediaPlayer->playMode = AVinfo->playMode;
		AVMediaPlayer->endWalker = AVinfo->endWalker;
		AVMediaPlayer->endContPlay = AVinfo->endContPlay;
		AVMediaPlayer->textView = AVinfo->textView;
		AVMediaPlayer->docWindow = AVinfo->docWindow;
		AVMediaPlayer->nextSegs = AVinfo->nextSegs;
		AVinfo->nextSegs = nil;
		AVMediaPlayer->audioPlayer = nil;
		if (AVinfo->isWhatType == isAudio) { // 2021-03-07 beg
			[AVMediaPlayer PlayAudio:YES];
		} else {// 2021-03-07 end
			[AVMediaPlayer showWindow:nil];
		}
		NSLog(@"AVController: createAndPlayAV-create AVMediaPlayer\n");
	} else {
		AVMediaPlayer->playerView.player.rate = 0.0;
		if (AVMediaPlayer->audioTimer != nil) {
			[AVMediaPlayer->audioTimer invalidate];
			AVMediaPlayer->audioTimer = nil;
			AVMediaPlayer->duration = -1;
		}
		if (AVMediaPlayer->timeObserverToken != nil) {
			[AVMediaPlayer->playerView.player removeTimeObserver:AVMediaPlayer->timeObserverToken];
			AVMediaPlayer->timeObserverToken = nil;
			AVMediaPlayer->duration = -1;
		}

		if (strcmp(AVinfo->mediaFPath, AVMediaPlayer->rMovieFile) != 0) {
			strcpy(AVMediaPlayer->rMovieFile, AVinfo->mediaFPath);
			AVMediaPlayer->beg = AVinfo->beg;
			AVMediaPlayer->end = AVinfo->end;
			AVMediaPlayer->lastEndBullet = AVMediaPlayer->beg;
			AVMediaPlayer->LoopCnt = PBC.LoopCnt;
			AVMediaPlayer->isWhatType = AVinfo->isWhatType;
			AVMediaPlayer->playMode = AVinfo->playMode;
			AVMediaPlayer->endWalker = AVinfo->endWalker;
			AVMediaPlayer->endContPlay = AVinfo->endContPlay;
			AVMediaPlayer->textView = AVinfo->textView;
			AVMediaPlayer->docWindow = AVinfo->docWindow;
			AVMediaPlayer->nextSegs = AVinfo->nextSegs;
			AVinfo->nextSegs = nil;
			if (AVMediaPlayer->isWhatType == isAudio) { // 2021-03-07 beg
				[AVMediaPlayer PlayAudio:YES];
			} else {// 2021-03-07 end
				nMovieFile = [NSString stringWithUTF8String:AVMediaPlayer->rMovieFile];
				clip = [AVURLAsset assetWithURL:[NSURL fileURLWithPath:nMovieFile]];
				if (clip == nil)
					return;
				if (AVMediaPlayer->isPlayerItemObserverSet == YES) {
					AVMediaPlayer->playerItemContext = 0;
					[AVMediaPlayer->playerItem removeObserver:AVMediaPlayer forKeyPath:@"status" context:&AVMediaPlayer->playerItemContext];
					AVMediaPlayer->isPlayerItemObserverSet = NO;
				}
				AVMediaPlayer->playerItem = [AVPlayerItem playerItemWithAsset:clip];
				options = NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
				[AVMediaPlayer->playerItem addObserver:AVMediaPlayer forKeyPath:@"status" options:options context:&AVMediaPlayer->playerItemContext];
				AVMediaPlayer->isPlayerItemObserverSet = YES;
				[AVMediaPlayer->playerView.player replaceCurrentItemWithPlayerItem:AVMediaPlayer->playerItem];
				[[AVMediaPlayer window] setTitleWithRepresentedFilename:nMovieFile];
				[AVMediaPlayer showWindow:nil];
			}
		} else {
			AVMediaPlayer->isWhatType = AVinfo->isWhatType;
			AVMediaPlayer->playMode = AVinfo->playMode;
			AVMediaPlayer->endWalker = AVinfo->endWalker;
			AVMediaPlayer->endContPlay = AVinfo->endContPlay;
			AVMediaPlayer->textView = AVinfo->textView;
			AVMediaPlayer->docWindow = AVinfo->docWindow;
			AVMediaPlayer->nextSegs = AVinfo->nextSegs;
			AVinfo->nextSegs = nil;
			AVMediaPlayer->beg = AVinfo->beg;
			AVMediaPlayer->end = AVinfo->end;
			AVMediaPlayer->LoopCnt = PBC.LoopCnt;
			if (AVMediaPlayer->isWhatType == isAudio) { // 2021-03-07 beg
				[AVMediaPlayer PlayAudio:NO];
			} else {// 2021-03-07 end
				[AVMediaPlayer showWindow:nil];
				[AVMediaPlayer PlayVideo];
			}
		}
		NSLog(@"AVController: createAndPlayAV-Just showWindow AVMediaPlayer\n");
	}
}

- (void)windowDidLoad {
	AVURLAsset *clip;
	NSKeyValueObservingOptions options;
	NSRect wFrame;
	NSWindow *window = [self window];
	NSString *nMovieFile;

	NSLog(@"AVController: windowDidLoad\n");
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...

	isSettingSize = NO;
	nMovieFile = [NSString stringWithUTF8String:rMovieFile];
	clip = [AVURLAsset assetWithURL:[NSURL fileURLWithPath:nMovieFile]];
	if (clip == nil)
		return;
	playerItemContext = 0;
	playerItem = [AVPlayerItem playerItemWithAsset:clip];
	options = NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
	[playerItem addObserver:self forKeyPath:@"status" options:options context:&playerItemContext];
	AVMediaPlayer->isPlayerItemObserverSet = YES;
//	playerView.player = [AVPlayer playerWithPlayerItem:playerItem];
	playerView.player = [[AVPlayer alloc] init];
	[playerView.player replaceCurrentItemWithPlayerItem:playerItem];

	if (MovieWinSize.height != 0 && MovieWinSize.width != 0) {
		wFrame.origin.y = MovieWinSize.top;
		wFrame.origin.x = MovieWinSize.left;
		wFrame.size.height = MovieWinSize.height;
		wFrame.size.width = MovieWinSize.width;
		[window setFrame:wFrame display:false];
	}
	[window setTitleWithRepresentedFilename:nMovieFile];

//	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
}

/* Reopen the line panel when the app's persistent state is restored.
 */
+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
#pragma unused (identifier, state, completionHandler)
	//	completionHandler([[(Controller *)[NSApp delegate] avController] window], NULL);
}


- (void)dealloc {
	playMode = stopped;
	[super dealloc]; // NSWindowController deallocates all the nib objects
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"AVController: windowWillClose\n");
//	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];

//	playerItemContext = 0;
//	[playerItem removeObserver:self forKeyPath:@"status" context:&playerItemContext];
	playMode = stopped;
	duration = -1;
	playerView.player.rate = 0.0;
	if (audioTimer != nil) {
		[audioTimer invalidate];
		audioTimer = nil;
	}
	if (timeObserverToken != nil) {
		[playerView.player removeTimeObserver:timeObserverToken];
		timeObserverToken = nil;
	}
}

@end


@implementation AVController(Delegation)

- (void)windowWillResize:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)
	NSLog(@"AVController: windowWillResize\n");
}

- (void)windowDidResize:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)
	NSLog(@"AVController: windowDidResize\n");

	if (!isSettingSize) {   // There is potential for recursion, but typically this is prevented in NSWindow which doesn't call this method if the frame doesn't change. However, just in case...
		isSettingSize = YES;
/*
		NSRect wFrame = self.window.frame;
		if (MovieWinSize.top != wFrame.origin.y || MovieWinSize.left != wFrame.origin.x ||
			MovieWinSize.height != wFrame.size.height || MovieWinSize.width != wFrame.size.width) {
			MovieWinSize.top = wFrame.origin.y;
			MovieWinSize.left = wFrame.origin.x;
			MovieWinSize.height = wFrame.size.height;
			MovieWinSize.width = wFrame.size.width;
			WriteCedPreference();
		}
*/
		isSettingSize = NO;
	}
}

- (void)windowDidMove:(NSNotification *)notification {// 2020-03-06
#pragma unused (notification)

	NSLog(@"AVController: windowDidMove\n");
/*
	NSRect wFrame = self.window.frame;
	if (MovieWinSize.top != wFrame.origin.y || MovieWinSize.left != wFrame.origin.x ||
		MovieWinSize.height != wFrame.size.height || MovieWinSize.width != wFrame.size.width) {
		MovieWinSize.top = wFrame.origin.y;
		MovieWinSize.left = wFrame.origin.x;
		MovieWinSize.height = wFrame.size.height;
		MovieWinSize.width = wFrame.size.width;
		WriteCedPreference();
	}
*/
}

@end

