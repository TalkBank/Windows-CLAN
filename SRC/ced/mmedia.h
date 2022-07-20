
#ifndef MMEDIA
#define MMEDIA


#define MOVEINCREMENT 25
#define	my_MIN(x,y)		((x) < (y) ? (x) : (y))
#define	my_MAX(x,y)		((x) < (y) ? (y) : (x))

extern "C"
{

struct PBCs {
	char isPC;
	char walk;
	char enable;
	int  LoopCnt, speed;
	long backspace,
		 step_length,
		 pause_len,
		 cur_pos,
		 total_len;
} ;

#if defined(_MAC_CODE)
// constants for selecting StopApplication phase
enum {
	kStopAppPhase_BeforeDestroyWindows 		= 1L << 0,		// movie windows not yet torn down
	kStopAppPhase_AfterDestroyWindows		= 1L << 1,		// movie windows already torn down
	kStopAppPhase_BothPhases				= kStopAppPhase_BeforeDestroyWindows | kStopAppPhase_AfterDestroyWindows
};
extern char  isMovieAvialable;
extern short movieActive;

extern int  ShowThumbnails(MovieTNInfo *orgPicts, myFInfo *df, long totalPicts);
extern void ResizeMovie(WindowPtr MvWindow, MovieInfo *theMovie, short w, short isRedo);
extern void ResizePicture(WindowPtr PictWindow, short w, short h);
extern void doMovieUndo(WindowProcRec *windProc);
extern void doMovieRedo(WindowProcRec *windProc);
extern void drawFakeHilight(char isOn, WindowPtr win);
extern void saveMovieUndo(WindowPtr win, short selBeg, short selEnd, char isFixRedo);
extern void setMovieCursor(WindowPtr win);
extern void setMovieCursorToValue(long tTime, myFInfo *df);
extern void UpdatePBCDialog(WindowPtr wind);
extern void freeThumbnails(MovieTNInfo *Picts);
#ifndef _COCOA_APP
extern void SaveSoundMovieAsWAVEFile(Movie tMovie, FNType *outFile);
extern char isMovieStreaming(Movie MvMovie);
#endif
extern char isPBCWindowOpen(void);
extern char SetCurrSoundTier(WindowProcRec *windProc, char whatMediaType);
extern char SetWaveTimeValue(WindowProcRec *windProc, long timeBeg, long timeEnd);
extern short whichActive(WindowPtr win);
extern MovieInfo *Movie_Open(movInfo *mvRec);
#endif // _MAC_CODE

#if defined(_WIN32)
extern char checkPCMSound(MSG* pMsg, char skipOnChar, DWORD tickCount);
extern char checkPCMPlaybackBuffer(void);
extern char SetCurrSoundTier(void);
extern char isTimeCodeZero(DVTimeCode *tc);
extern void cpTimeCode(DVTimeCode *dest, DVTimeCode *src);
extern void cpStrToTimeCode(char *st, DVTimeCode *tc);
extern void setTimeCodeToTime(DVTimeCode *dest, UInt8 h, UInt8 m, UInt8 s, UInt8 f);
extern void setTimeCodeToZero(DVTimeCode *dest);
extern void stopSoundIfPlaying(void);
//extern void SaveSoundMovieAsWAVEFile(Movie tMovie, FNType *outFile);
extern void SaveSoundMovieAsWAVEFile(FNType *inFile, FNType *outFile);
#endif /* _WIN32 */

extern char  MovieReady;
extern short cChan;
extern short MVWinWidth, MVWinZoom;
extern long F5_Offset;
extern struct PBCs PBC;

extern int  soundwindow(int TurnOn);
extern int  play_sound(long int begin, long int end, int cont);
extern int  PlaySound(sndInfo *sndRec, int cont);
extern int  PlayMovie(movInfo *mvRec, myFInfo *df, char isJustOpen);
extern int  DisplayPhoto(FNType *fname);

extern long ComputeOffset(long prc);
extern long conv_to_msec_rep(long num);
extern long conv_from_msec_rep(long num);
extern long conv_to_msec_mov(long num, long ts);
extern long conv_from_msec_mov(long num, long ts);
extern long conv_to_msec_MP3(long num);
extern long conv_from_msec_MP3(long num);
extern long conv_to_msec_MP3_pure(sndInfo *SnTr, long num);
extern long conv_from_msec_rep_pure(sndInfo *SnTr, long num);
extern long conv_to_msec_rep_pure(sndInfo *SnTr, long num);

extern char GetNewMediaFile(char isCheckError, char isAllMedia);
extern char findStartMediaTag(char isMove, char isCAMode);
extern char PlayBuffer(int com);
extern char CheckRateChan(sndInfo *snd, char *errMess);
extern char move_cursor(long mtime, unCH *file, char isSetBE, char isSnd);
extern char FindSndInfoAndCopyIt(FNType *fname, long *beg, long *end);
extern char sameKeyPressed(int key);
extern char FakeSelectWholeTier(char isCAMode);
extern char GetNewPictFile(FNType *fname);
extern char GetNewTextFile(FNType *fname);
extern char findMediaTiers(void);
extern char syncAudioAndVideo(unCH *buf);
extern char findSoundFileInWd(sndInfo *tSound, char *errMess);
extern char findMovieFileInWd(movInfo *tMovie);
extern char mOpenMovieFile(movInfo *tMovie);
extern char LocateMediaFile(FNType *fname);

extern void CleanMediaName(unCH *mediaFileName);
extern void getFileType(FNType *fn, unsigned long *type);
extern void CantFindMedia(char *err_message, unCH *fname, char isOld);
extern void checkContinousSndPlayPossition(long CurFP);
extern void selectNextSpeaker(void);
extern void SelectMediaFile(void);
extern void findEndOfSpeakerTier(char goin, char isCAMode);
extern void CopyAndFreeMovieData(char all);
extern void DisposeOfSoundWin(void);
extern void addBulletsToText(const char *, const unCH *, long, long);
extern void show_wave(char *file, long begin, long end, int add);
extern void SaveSoundClip(sndInfo *SnTr, char isGetNewname);
extern void stopMoviePlaying(void);
extern void replayMovieWithOffset(int offset);
extern void WriteAIFFHeader(FILE *fpout, long numBytes, unsigned int frames, short numChannels, short sampleSize, double sampleRate);
extern unsigned char Lin2Alaw(short pcm_val);
extern unsigned char Lin2Mulaw(short pcm_val);
extern unsigned char alaw2ulaw(unsigned char aval);
extern unsigned char ulaw2alaw(unsigned char uval);
extern short Alaw2Lin(unsigned char a_val);
extern short Mulaw2Lin(unsigned char u_val);
}

#endif // MMEDIA
