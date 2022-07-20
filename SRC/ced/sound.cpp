/* Natural Pauses during continuos playback of audio media
if	
	%media:"BOYS85"_8719_9346
	%media:"BOYS85"_9400_9938
Then there is a pause of 54 between them

if
	%media:"BOYS85"_8719_9346-
	%media:"BOYS85"_9400_9938
Then there is no pause between them at all
*/

#include "cu.h"
#include "ced.h"
#include "my_ctype.h"
#if defined(__MACH__)
	#include "mp3.h"
	#include <QuickTimeComponents.h>
#endif
#include "MMedia.h"
#ifdef _WIN32
	#include "w95_QTReplace.h"
	#include "MpegDlg.h"
#endif

extern char SelectWholeTier(char isCAMode);

static char isDisplayWave = 1;
static char isSkipPause = FALSE;

static unCH sFN[FILENAME_MAX];
static long sBF;
long sEF;
long gCurF;

char isPlayAudioFirst = 1, isPlayAudioMenuSet = 0;
char doMixedSTWave;
short cChan = -1;
short MVWinWidth, MVWinZoom;

long F5_Offset;

/* sound commands begin */
long AlignMultibyteMediaStream(long num, char dir) {
	if (num < 0L)
		num = 0L;
	if (num >= global_df->SnTr.SoundFileSize) 
		num = global_df->SnTr.SoundFileSize;
	if (global_df->SnTr.SNDsample == 2) {
		if (num % 2 != 0) {
			if (dir == '+') {
				num++;
				if (num >= global_df->SnTr.SoundFileSize) 
					num = global_df->SnTr.SoundFileSize;
			} else {
				num--;
				if (num < 0L)
					num = 0L;
			}
		}
		if (global_df->SnTr.SNDchan == 2 && (num / 2) % 2 != 0) {
			if (dir == '+') {
				num += 2;
				if (num >= global_df->SnTr.SoundFileSize) 
					num = global_df->SnTr.SoundFileSize;
			} else {
				num -= 2;
				if (num < 0L)
					num = 0L;
			}
		}
	} else if (global_df->SnTr.SNDchan == 2 && num % 2 != 0) {
		if (dir == '+') {
			num++;
			if (num >= global_df->SnTr.SoundFileSize) 
				num = global_df->SnTr.SoundFileSize;
		} else {
			num--;
			if (num < 0L)
				num = 0L;
		}
	}
	return(num);
}

static void PreserveStateAndShowPict(void) {
	myFInfo *saveGlobal_df;
#if defined(_MAC_CODE)
	WindowPtr wind;
#endif

	saveGlobal_df = global_df;
#if defined(_MAC_CODE)
	wind = FrontWindow();
#endif
	if (*global_df->PcTr.pictFName != EOS)
		DisplayPhoto(global_df->PcTr.pictFName);
#if defined(_MAC_CODE)
	changeCurrentWindow(NULL, wind, true);
#endif
#ifdef _WIN32
	AfxGetApp()->m_pMainWnd->BringWindowToTop();
#endif
	global_df = saveGlobal_df;
}

static void PreserveStateAndShowText(void) {
	myFInfo *saveGlobal_df;
#if defined(_MAC_CODE)
	WindowPtr wind;
#endif

	saveGlobal_df = global_df;
#if defined(_MAC_CODE)
	wind = FrontWindow();
#endif
	if (*global_df->TxTr.textFName != EOS)
		DisplayText(global_df->TxTr.textFName);
#if defined(_MAC_CODE)
	changeCurrentWindow(NULL, wind, true);
#endif
#ifdef _WIN32
	AfxGetApp()->m_pMainWnd->BringWindowToTop();
#endif
	global_df = saveGlobal_df;
}

// BEGIN Open media files listed in bullets
static char InGivenDirOpenPictFile(FNType *pictFName) {
	int len;

	len = strlen(pictFName);
	uS.str2FNType(pictFName, strlen(pictFName), ".gif");
	if (access(pictFName, 0)) {
		pictFName[len] = EOS;
		uS.str2FNType(pictFName, strlen(pictFName), ".pict");
		if (access(pictFName, 0)) {
			pictFName[len] = EOS;
			uS.str2FNType(pictFName, strlen(pictFName), ".jpeg");
			if (access(pictFName, 0)) {
				pictFName[len] = EOS;
				uS.str2FNType(pictFName, strlen(pictFName), ".jpg");
				if (access(pictFName, 0)) {
					return(FALSE);
				}
			}
		}
	}

	return(TRUE);
}

static char mOpenPictFile(void) {
	FNType *p;
	FNType mFileName[FNSize];
	FNType tFileName[FNSize];

	extractFileName(mFileName, global_df->PcTr.pictFName);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;

	extractPath(global_df->PcTr.pictFName, global_df->fileName);
	strcat(global_df->PcTr.pictFName, mFileName);
	if (InGivenDirOpenPictFile(global_df->PcTr.pictFName))
		return(1);

	extractPath(global_df->PcTr.pictFName, global_df->fileName);
	addFilename2Path(global_df->PcTr.pictFName, "media");
	addFilename2Path(global_df->PcTr.pictFName, mFileName);
	if (InGivenDirOpenPictFile(global_df->PcTr.pictFName))
		return(1);

	strcpy(global_df->PcTr.pictFName, wd_dir);
	addFilename2Path(global_df->PcTr.pictFName, mFileName);
	if (InGivenDirOpenPictFile(global_df->PcTr.pictFName))
		return(1);

	strcpy(global_df->PcTr.pictFName, lib_dir);
	addFilename2Path(global_df->PcTr.pictFName, mFileName);
	if (InGivenDirOpenPictFile(global_df->PcTr.pictFName))
		return(1);

	extractFileName(tFileName, global_df->fileName);
	p = strrchr(tFileName, '.');
	if (p != NULL)
		*p = EOS;
	extractPath(global_df->PcTr.pictFName, global_df->fileName);
	addFilename2Path(global_df->PcTr.pictFName, tFileName);
	addFilename2Path(global_df->PcTr.pictFName, mFileName);
	if (InGivenDirOpenPictFile(global_df->PcTr.pictFName))
		return(1);

	if (GetNewPictFile(global_df->PcTr.pictFName))
		return(1);
	return(0);
}

static char InGivenDirOpenTextFile(FNType *textFName) {
	int len;

	len = strlen(textFName);
	uS.str2FNType(textFName, strlen(textFName), ".txt");
	if (access(textFName, 0)) {
		textFName[len] = EOS;
		uS.str2FNType(textFName, strlen(textFName), ".cut");
		if (access(textFName, 0)) {
			textFName[len] = EOS;
			uS.str2FNType(textFName, strlen(textFName), ".cha");
			if (access(textFName, 0)) {
				textFName[len] = EOS;
				uS.str2FNType(textFName, strlen(textFName), ".cdc");
				if (access(textFName, 0)) {
					return(FALSE);
				}
			}
		}
	}
	return(TRUE);
}

static char mOpenTextFile(void) {
	FNType *p;
	FNType mFileName[FNSize];

	extractFileName(mFileName, global_df->TxTr.textFName);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;

	extractPath(global_df->TxTr.textFName, global_df->fileName);
	strcat(global_df->TxTr.textFName, mFileName);
	if (InGivenDirOpenTextFile(global_df->TxTr.textFName))
		return(1);

	extractPath(global_df->TxTr.textFName, global_df->fileName);
	addFilename2Path(global_df->TxTr.textFName, "media");
	addFilename2Path(global_df->TxTr.textFName, mFileName);
	if (InGivenDirOpenTextFile(global_df->TxTr.textFName))
		return(1);

	strcpy(global_df->TxTr.textFName, wd_dir);
	addFilename2Path(global_df->TxTr.textFName, mFileName);
	if (InGivenDirOpenTextFile(global_df->TxTr.textFName))
		return(1);

	strcpy(global_df->TxTr.textFName, lib_dir);
	addFilename2Path(global_df->TxTr.textFName, mFileName);
	if (InGivenDirOpenTextFile(global_df->TxTr.textFName))
		return(1);

	extractPath(global_df->TxTr.textFName, global_df->fileName);
	strcat(global_df->TxTr.textFName, mFileName);
	if (GetNewTextFile(global_df->TxTr.textFName))
		return(1);

	return(0);
}

static char InGivenDirOpenMovieFile(FNType *movieFName, movInfo *tMovie) {
	int len;
	FNType *c;

	len = strlen(movieFName);
	uS.str2FNType(movieFName, strlen(movieFName), ".mov");
	if (access(movieFName, 0)) {
		movieFName[len] = EOS;
		uS.str2FNType(movieFName, strlen(movieFName), ".mp4");
		if (access(movieFName, 0)) {
			movieFName[len] = EOS;
			uS.str2FNType(movieFName, strlen(movieFName), ".mpg");
			if (access(movieFName, 0)) {
				movieFName[len] = EOS;
				uS.str2FNType(movieFName, strlen(movieFName), ".m4v");
				if (access(movieFName, 0)) {
					movieFName[len] = EOS;
					uS.str2FNType(movieFName, strlen(movieFName), ".avi");
					if (access(movieFName, 0)) {
						movieFName[len] = EOS;
						uS.str2FNType(movieFName, strlen(movieFName), ".dv");
						if (access(movieFName, 0)) {
#ifdef _WIN32
/*
							movieFName[len] = EOS;
							uS.str2FNType(movieFName, strlen(movieFName), ".wmv");
							if (access(movieFName, 0)) {
								movieFName[len] = EOS;
								uS.str2FNType(movieFName, strlen(movieFName), ".mp3");
								if (access(movieFName, 0)) {
									return(FALSE);
								}
							}
*/
							movieFName[len] = EOS;
							uS.str2FNType(movieFName, strlen(movieFName), ".wmv");
							if (access(movieFName, 0)) {
								return(FALSE);
							}
#else
							return(FALSE);
#endif
						}
					}
				}
			}
		}
	}
	strcpy(tMovie->rMovieFile, movieFName);
	if ((c=strrchr(movieFName, PATHDELIMCHR)) != NULL)
		strcpy(movieFName, c+1);
	if ((c=strrchr(movieFName,'.')) != NULL)
		*c = EOS;
	u_strcpy(tMovie->MovieFile, movieFName, FILENAME_MAX);
	return(TRUE);
}

char mOpenMovieFile(movInfo *tMovie) {
	FNType *p;
	FNType mFileName[FILENAME_MAX];
	FNType movieFName[FNSize];

	u_strcpy(mFileName, tMovie->MovieFile, FILENAME_MAX);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;
	if (global_df != NULL)
		extractPath(movieFName, global_df->fileName);
	else
		strcpy(movieFName, tMovie->rMovieFile);
	strcat(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);
	if (global_df != NULL)
		extractPath(movieFName, global_df->fileName);
	else
		strcpy(movieFName, tMovie->rMovieFile);
	addFilename2Path(movieFName, "media");
	addFilename2Path(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);
	strcpy(movieFName, wd_dir);
	addFilename2Path(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);
	strcpy(movieFName, lib_dir);
	addFilename2Path(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);
	return(1);
}

char findMovieFileInWd(movInfo *tMovie) {
	FNType *p;
	FNType mFileName[FILENAME_MAX];
	FNType movieFName[FNSize];

	u_strcpy(mFileName, tMovie->MovieFile, FILENAME_MAX);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;

	strcpy(movieFName, wd_dir);
	addFilename2Path(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);

	strcpy(movieFName, wd_dir);
	addFilename2Path(movieFName, "media");
	addFilename2Path(movieFName, mFileName);
	if (InGivenDirOpenMovieFile(movieFName, tMovie))
		return(0);

	return(1);
}

static char InGivenDirOpenSoundFile(FNType *rFileName, sndInfo *tSound) {
	int len;

	len = strlen(rFileName);
	uS.str2FNType(rFileName, strlen(rFileName), ".aiff");
	if ((tSound->SoundFPtr=fopen(rFileName, "rb")) == NULL) {
		rFileName[len] = EOS;
		uS.str2FNType(rFileName, strlen(rFileName), ".aif");
		if ((tSound->SoundFPtr=fopen(rFileName, "rb")) == NULL) {
			rFileName[len] = EOS;
			uS.str2FNType(rFileName, strlen(rFileName), ".wav");
			if ((tSound->SoundFPtr=fopen(rFileName, "rb")) == NULL) {
				rFileName[len] = EOS;
				uS.str2FNType(rFileName, strlen(rFileName), ".mp3");
#ifdef _WIN32
				if (access(rFileName, 0)) {
					return(FALSE);
				} else {
//					return(FALSE);
					FNType *s;
					extractFileName(FileName2, rFileName);
					s = strrchr(FileName2, '.');
					if (s != NULL && uS.mStricmp(s, ".mp3") == 0)
						*s = EOS;
					strcpy(FileName1, rFileName);
					s = strrchr(FileName1, '.');
					if (s != NULL && uS.mStricmp(s, ".mp3") == 0)
						*s = EOS;
					strcat(FileName1, ".wav");
					if (global_df->SnTr.SoundFPtr != NULL)
						fclose(global_df->SnTr.SoundFPtr);

					strcpy(global_df->MvTr.rMovieFile, rFileName);
					u_strcpy(global_df->MvTr.MovieFile, FileName2, FILENAME_MAX);
					PlayMovie(&global_df->MvTr, global_df, TRUE);
					SaveSoundMovieAsWAVEFile(rFileName, FileName1);
					if (MpegDlg != NULL)
						MpegDlg->OnCancel();
					if ((tSound->SoundFPtr = fopen(FileName1, "rb")) == NULL) {
						return(FALSE);
					}
					global_df->MvTr.rMovieFile[0] = EOS;
					global_df->MvTr.MovieFile[0] = EOS;
					u_strcpy(global_df->SnTr.SoundFile, FileName2, FILENAME_MAX);
					strcpy(rFileName, FileName1);
				}
#else
				if (GetMovieMedia(rFileName, &tSound->mp3.theSoundMedia, &tSound->mp3.hSys7SoundData) != noErr) {
					return(FALSE);
				} else {
					tSound->isMP3 = TRUE;
				}
#endif
			}
		}
	}
	strcpy(tSound->rSoundFile, rFileName);
	return(TRUE);
}

static char mOpenSoundFile(void) {
	char isOpened;
	FNType *p;
	FNType mFileName[FILENAME_MAX];
	FNType rFileName[FNSize];

	global_df->SnTr.SoundFPtr = 0;
	u_strcpy(mFileName, global_df->SnTr.SoundFile, FILENAME_MAX);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;
	extractPath(rFileName, global_df->fileName);
	strcat(rFileName, mFileName);
	if ((isOpened=InGivenDirOpenSoundFile(rFileName, &global_df->SnTr)) == FALSE) {
		extractPath(rFileName, global_df->fileName);
		addFilename2Path(rFileName, "media");
		addFilename2Path(rFileName, mFileName);
		if ((isOpened=InGivenDirOpenSoundFile(rFileName, &global_df->SnTr)) == FALSE) {
			strcpy(rFileName, wd_dir);
			addFilename2Path(rFileName, mFileName);
			if ((isOpened=InGivenDirOpenSoundFile(rFileName, &global_df->SnTr)) == FALSE) {
				strcpy(rFileName, lib_dir);
				addFilename2Path(rFileName, mFileName);
				isOpened = InGivenDirOpenSoundFile(rFileName, &global_df->SnTr);
			}
		}
	}

	if (isOpened) {
		getFileType(rFileName, &global_df->SnTr.SFileType);
		if (global_df->SnTr.SFileType == 'MP3!' && global_df->SnTr.isMP3 != TRUE) {
			if (global_df->SnTr.SoundFPtr != NULL)
				fclose(global_df->SnTr.SoundFPtr);
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
			global_df->SnTr.isMP3 = TRUE;
#ifdef _MAC_CODE
			if (GetMovieMedia(rFileName, &global_df->SnTr.mp3.theSoundMedia, &global_df->SnTr.mp3.hSys7SoundData) != noErr) {
				sprintf(global_df->err_message, "+Can't open sound file: %s", mFileName);
				return(0);
			}
#endif
		}
	} else {
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
		global_df->SnTr.SoundFile[0] = EOS;
		global_df->SnTr.SoundFPtr = 0;
		if (global_df->SoundWin)
			DisposeOfSoundWin();
		return(0);
	}

	if (global_df->SnTr.SoundFPtr == NULL && global_df->SnTr.isMP3 != TRUE) {
		sprintf(global_df->err_message, "+Can't open sound file: %s", mFileName);
		global_df->SnTr.SoundFile[0] = EOS;
		global_df->SnTr.SoundFPtr = 0;
		if (global_df->SoundWin)
			DisposeOfSoundWin();
		return(0);
	}

	if (!CheckRateChan(&global_df->SnTr, global_df->err_message)) {
		if (global_df->SnTr.isMP3 == TRUE) {
			global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
			if (global_df->SnTr.mp3.hSys7SoundData)
				DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
			global_df->SnTr.mp3.theSoundMedia = NULL;
			global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
		} else {
			if (global_df->SnTr.SoundFPtr != NULL)
				fclose(global_df->SnTr.SoundFPtr);
		}
		global_df->SnTr.SoundFile[0] = EOS;
		global_df->SnTr.SoundFPtr = 0;
		if (global_df->SoundWin)
			DisposeOfSoundWin();
		return(0);
	}
	return(1);
}

char findSoundFileInWd(sndInfo *tSound, char *errMess) {
	char isOpened;
	FNType *p;
	FNType mFileName[FILENAME_MAX];
	FNType rFileName[FNSize];

	tSound->SoundFPtr = 0;
	u_strcpy(mFileName, tSound->SoundFile, FILENAME_MAX);
	p = strrchr(mFileName, '.');
	if (p != NULL)
		*p = EOS;
	strcpy(rFileName, wd_dir);
	addFilename2Path(rFileName, mFileName);
	if ((isOpened=InGivenDirOpenSoundFile(rFileName, tSound)) == FALSE) {
		strcpy(rFileName, wd_dir);
		addFilename2Path(rFileName, "media");
		addFilename2Path(rFileName, mFileName);
		isOpened = InGivenDirOpenSoundFile(rFileName, tSound);
	}

	if (isOpened) {
		getFileType(rFileName, &tSound->SFileType);
		if (tSound->SFileType == 'MP3!' && tSound->isMP3 != TRUE) {
			if (tSound->SoundFPtr != NULL)
				fclose(tSound->SoundFPtr);
			tSound->SoundFPtr = 0;
			tSound->isMP3 = TRUE;
#ifdef _MAC_CODE
			if (GetMovieMedia(rFileName, &tSound->mp3.theSoundMedia, &tSound->mp3.hSys7SoundData) != noErr) {
				return(0);
			}
#endif
		}
	} else {
		tSound->SoundFile[0] = EOS;
		tSound->SoundFPtr = 0;
		return(0);
	}

	if (tSound->SoundFPtr == NULL && tSound->isMP3 != TRUE) {
		tSound->SoundFile[0] = EOS;
		return(0);
	}

	if (!CheckRateChan(tSound, errMess)) {
		if (tSound->isMP3 == TRUE) {
			tSound->isMP3 = FALSE;
#ifdef _MAC_CODE
			if (tSound->mp3.hSys7SoundData)
				DisposeHandle(tSound->mp3.hSys7SoundData);
			tSound->mp3.theSoundMedia = NULL;
			tSound->mp3.hSys7SoundData = NULL;
#endif
		} else {
			if (tSound->SoundFPtr != NULL)
				fclose(tSound->SoundFPtr);
		}
		tSound->SoundFile[0] = EOS;
		tSound->SoundFPtr = 0;
		return(0);
	}
	return(1);
}
// END Open media files listed in bullets


void PutSoundStats(Size ws) {
	long cb, ce, cbm, cbsm, cem, cesm;

	if (global_df == NULL)
		return;
	if (global_df->SnTr.dtype == 0) {
		global_df->WinChange = FALSE;
		draw_mid_wm();
		global_df->WinChange = TRUE;
		return;
	}
	if (global_df->SnTr.dtype == 2) {
		FNType mFileName[FNSize];

		extractFileName(mFileName, global_df->SnTr.rSoundFile);
		global_df->WinChange = FALSE;
		sprintf(global_df->err_message, "-\"%s\": %d bits, %ldHz., %s., %s", 
					mFileName, global_df->SnTr.SNDsample*8, 
					(long)global_df->SnTr.SNDrate, 
					((global_df->SnTr.SNDchan == 1) ? "mono" : "stereo"), 
					((global_df->SnTr.isMP3 == TRUE) ? "MP3" : 
					    ((global_df->SnTr.SNDformat == kULawCompression) ? "mu-law" : 
						    ((global_df->SnTr.SNDformat == kALawCompression) ? "a-law" : "pcm"))));
		draw_mid_wm();
		global_df->WinChange = TRUE;
		return;
	}
 
	if (global_df->SnTr.BegF == global_df->SnTr.EndF)
		global_df->SnTr.EndF = 0L;
	global_df->WinChange = FALSE;
	cb = conv_to_msec_rep(global_df->SnTr.WBegF);
	ce = global_df->SnTr.WBegF + ws;
	if (ce > global_df->SnTr.SoundFileSize)
		ce = global_df->SnTr.SoundFileSize;
	ce = conv_to_msec_rep(ce);
	sprintf(global_df->err_message, "-W ");

	if (ce/1000/60 > 0) {
		cbm = cb / 1000 / 60;
		cbsm = (cb / 1000) - (cbm * 60);
		cem = ce / 1000 / 60;
		cesm = (ce / 1000) - (cem * 60);
//		if (cbm != cem || cbsm != cesm) {
//			sprintf(global_df->err_message+strlen(global_df->err_message), "%ld:%ld-%ld:%ld; ", cbm, cbsm, cem, cesm);
//		} else {
			cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
			ce = ce - (cem * 1000 * 60) - (cesm * 1000);
			sprintf(global_df->err_message+strlen(global_df->err_message), "%ld:%ld.%s%s%ld-%ld:%ld.%s%s%ld; ", 
												cbm, cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb, 
												cem, cesm, ((ce < 100)? "0" : ""), ((ce < 10)? "0" : ""), ce);
//		}
	} else if (ce / 1000 > 0) {
		cbsm = cb / 1000;
		cesm = ce / 1000;
//		if (cbsm != cesm)
//			sprintf(global_df->err_message+strlen(global_df->err_message),"%lds-%lds; ",cbsm,cesm);
//		else {
			cb = cb - (cbsm * 1000);
			ce = ce - (cesm * 1000);
			sprintf(global_df->err_message+strlen(global_df->err_message), "%ld.%s%s%ld-%ld.%s%s%ld; ",
												cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb,
												cesm, ((ce < 100)? "0" : ""), ((ce < 10)? "0" : ""), ce);
//		}
	} else {
		sprintf(global_df->err_message+strlen(global_df->err_message), "%ldMs-%ldMs; ", cb, ce);
	}

	if (global_df->SnTr.dtype == 1) {
		if (global_df->SnTr.EndF != 0L) {
			cb = conv_to_msec_rep(global_df->SnTr.EndF) - conv_to_msec_rep(global_df->SnTr.BegF);
			cbm = cb / 1000 / 60;
			cbsm = (cb / 1000) - (cbm * 60);
			cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
			sprintf(global_df->err_message+strlen(global_df->err_message), "D 00:%s%ld:%s%ld.%s%s%ld; ", 
					(cbm < 10 ? "0" : ""), cbm, (cbsm < 10 ? "0" : ""), cbsm, 
					((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
		} else
			sprintf(global_df->err_message+strlen(global_df->err_message), "D 00:00:00.000; ");
	}

	cb = conv_to_msec_rep(global_df->SnTr.BegF);
	strcat(global_df->err_message, "C at ");
	if (cb/1000/60 > 0) {
		cbm = cb / 1000 / 60;
		cbsm = (cb / 1000) - (cbm * 60);
		cb = cb - (cbm * 1000 * 60) - (cbsm * 1000);
		sprintf(global_df->err_message+strlen(global_df->err_message), "%ld:%ld.%s%s%ld", 
				cbm, cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
		cb = conv_to_msec_rep(global_df->SnTr.BegF);
		sprintf(global_df->err_message+strlen(global_df->err_message), "(%ld)", cb);
	} else if (cb / 1000 > 0) {
		cbsm = cb / 1000;
		cb = cb - (cbsm * 1000);
		sprintf(global_df->err_message+strlen(global_df->err_message), "%ld.%s%s%lds",
				cbsm, ((cb < 100)? "0" : ""), ((cb < 10)? "0" : ""), cb);
		cb = conv_to_msec_rep(global_df->SnTr.BegF);
		sprintf(global_df->err_message+strlen(global_df->err_message), "(%ld)", cb);
	} else {
		sprintf(global_df->err_message+strlen(global_df->err_message), "%ldMs", cb);
	}
	draw_mid_wm();
	global_df->WinChange = TRUE;
}

void DisposeOfSoundWin(void) {
	if (global_df->SoundWin) {
		DrawSoundCursor(0);
		wdrawcontr(global_df->SoundWin, FALSE);
		delwin(global_df->SoundWin);
		ResetUndos();
		if (global_df->TopP1) free(global_df->TopP1);
		if (global_df->BotP1) free(global_df->BotP1);
		if (global_df->TopP2) free(global_df->TopP2);
		if (global_df->BotP2) free(global_df->BotP2);
		global_df->TopP1 = NULL;
		global_df->BotP1 = NULL;
		global_df->TopP2 = NULL;
		global_df->BotP2 = NULL;
		global_df->SnTr.WBegFM = 0L;
		global_df->SnTr.WEndFM = 0L;
	}
	touchwin(global_df->w2); wrefresh(global_df->w2);
	global_df->SoundWin = NULL;
	global_df->SnTr.IsSoundOn = FALSE;
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif
	global_df->EdWinSize = 0;
	global_df->CodeWinStart = 0;
	global_df->total_num_rows = (global_df->NoCodes ? 1 : 5);
	if (!init_windows(true, 1, false))
	    mem_err(TRUE, global_df);
}

void CleanMediaName(unCH *tMediaFileName) {
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

char syncAudioAndVideo(unCH *buf) {
	if (global_df->SoundWin == NULL)
		return(FALSE);
	strcpy(ced_line, buf);
	if (global_df->SnTr.IsSoundOn && uS.mStrnicmp(global_df->SnTr.SoundFile, ced_line, strlen(ced_line)) == 0)
		;
	else
#ifdef _MAC_CODE
	if (theMovie == NULL || theMovie->MvMovie == NULL) {
		return(FALSE);
	}
#elif _WIN32
/* // NO QT
	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->theMovie != NULL) {
	} else 
*/
	if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpPlayer != NULL) {
	} else {
		return(FALSE);
	}
#endif
//	strcat(ced_line, "-clanw");
	strcat(ced_line, ".wav");
	if (uS.mStricmp(buf, global_df->SnTr.SoundFile) == 0 || uS.mStricmp(ced_line, global_df->SnTr.SoundFile) == 0)
		return(TRUE);
	else
		return(FALSE);
}

char findMediaTiers(void) {
	ROWS *p;

	global_df->mediaFileName[0] = EOS;
	global_df->isErrorFindPict = FALSE;
	p = global_df->head_text->next_row;
	if (p == global_df->tail_text)
		return(FALSE);

	while (1) {
		if (global_df->cur_line == p)
			ChangeCurLineAlways(0);

		if (isMainSpeaker(*p->line))
			break;
		if (uS.partcmp(p->line, MEDIAHEADER, FALSE, FALSE)) {
			strncpy(global_df->mediaFileName, p->line+strlen(MEDIAHEADER), FILENAME_MAX-1);
			global_df->mediaFileName[FILENAME_MAX-1] = EOS;
			CleanMediaName(global_df->mediaFileName);
			return(TRUE);
		}
		if (AtBotEnd(p, global_df->tail_text, TRUE))
			break;
		p = p->next_row;
	}
	return(FALSE);
}

void SelectMediaFile(void) {
	char isRightTierFound, *s, isMediaFound, isWhatType;
	long len;

	if (isRefEQZero(global_df->fileName)) {
		do_warning("Please save your data file on hard drive first.", 0);
		return;
	}
	extractPath(FileName1, global_df->fileName);
	if (!LocateMediaFile(FileName1))
		return;	
	extractFileName(FileName2, FileName1);
	isWhatType = 0;
	s = strrchr(FileName2, '.');
	if (s != NULL) {
#ifdef _WIN32
		if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L)  ||
			!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L) ||
			!uS.FNTypeicmp(s, ".mp3", 0L)) {
			isWhatType = isAudio;
		} else if (!uS.FNTypeicmp(s, ".mov", 0L)  || !uS.FNTypeicmp(s, ".mp4", 0L)  ||
				   !uS.FNTypeicmp(s, ".m4v", 0L)  || !uS.FNTypeicmp(s, ".flv", 0L)  ||
				   !uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L)  ||
				   !uS.FNTypeicmp(s, ".avi", 0L)  || !uS.FNTypeicmp(s, ".dv", 0L)   || 
				   !uS.FNTypeicmp(s, ".dat", 0L)  || !uS.FNTypeicmp(s, ".wmv", 0L)) {
			isWhatType = isVideo;
		}
#else
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
#endif
		*s = EOS;
	}
	s = strrchr(FileName2, ' ');
	if (s != NULL) {
		do_warning("Selected file name has a SPACE character in it!!", 0);
		return;
	}
	s = strrchr(FileName2, '\t');
	if (s != NULL) {
		do_warning("Selected file name has a TAB character in it!!", 0);
		return;
	}
	BeginningOfFile(-1);
	isMediaFound = FALSE;
	isRightTierFound = FALSE;
	while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
		ChangeCurLineAlways(0);
		if (uS.partcmp(global_df->row_txt->line, "@Media:", FALSE, FALSE)) {
			isMediaFound = TRUE;
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
			isRightTierFound = TRUE;
		} else if (isRightTierFound && isSpace(global_df->row_txt->line[0])) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
		} else if (isMediaFound || global_df->row_txt->line[0] == '*') {
			break;
		} else
			MoveDown(-1);
	}
	BeginningOfFile(-1);
	if (uS.partcmp(global_df->row_txt->line, "@Begin", FALSE, FALSE)) {
		do {
			MoveDown(-1);
		} while (isSpace(global_df->row_txt->line[0]) && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE)) ;
	}
	if (uS.partcmp(global_df->row_txt->line, "@Languages:", FALSE, FALSE)) {
		do {
			MoveDown(-1);
		} while (isSpace(global_df->row_txt->line[0]) && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE)) ;
	}
	if (uS.partcmp(global_df->row_txt->line, "@Participants:", FALSE, FALSE)) {
		do {
			MoveDown(-1);
		} while (isSpace(global_df->row_txt->line[0]) && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE)) ;
	}
	if (uS.partcmp(global_df->row_txt->line, "@Options:", FALSE, FALSE)) {
		do {
			MoveDown(-1);
		} while (isSpace(global_df->row_txt->line[0]) && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE)) ;
	}
	if (uS.partcmp(global_df->row_txt->line, "@ID:", FALSE, FALSE)) {
		do {
			MoveDown(-1);
			if (AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE))
				break;
		} while (uS.partcmp(global_df->row_txt->line, "@ID:", FALSE, FALSE)) ;
	}
	if (uS.partcmp(global_df->row_txt->line, "L1 of ", FALSE, FALSE) ||
		uS.partcmp(global_df->row_txt->line, "@Birth of ", FALSE, FALSE) ||
		uS.partcmp(global_df->row_txt->line, "@Birthplace of ", FALSE, FALSE)) {
		do {
			MoveDown(-1);
		} while ((isSpace(global_df->row_txt->line[0]) ||
				  uS.partcmp(global_df->row_txt->line, "L1 of ", FALSE, FALSE) ||
				  uS.partcmp(global_df->row_txt->line, "@Birth of ", FALSE, FALSE) ||
				  uS.partcmp(global_df->row_txt->line, "@Birthplace of ", FALSE, FALSE))
				 && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE)) ;
	}
	strcpy(templineC, "@Media:	");
	strcat(templineC, FileName2);
	if (isWhatType == isAudio)
		strcat(templineC, ", audio");
	else if (isWhatType == isVideo)
		strcat(templineC, ", video");
	u_strcpy(templineW, templineC, UTTLINELEN);
	len = strlen(templineW);
	templineW[len++] = NL_C;
	templineW[len] = EOS;
	if (global_df->UndoList->NextUndo) {
		if (global_df->UndoList->key != INSTRPT)
			global_df->UndoList->key = INSTKEY;
	}
	ChangeCurLine();
	if (global_df->col_txt == global_df->tail_row && global_df->col_txt->prev_char->c == NL_C) {
		global_df->UndoList->key = MOVEKEY;
		global_df->col_chr--;
		global_df->col_txt = global_df->col_txt->prev_char;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		SaveUndoState(FALSE);
		global_df->UndoList->key = INSTRPT;
	}
	//	TRUE_CHECK_ID1(global_df->row_txt->flag);
	//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	global_df->col_txt = global_df->col_txt->prev_char;
	AddString(templineW, len, TRUE);
	global_df->col_txt = global_df->col_txt->next_char;
	ResetUndos();
	global_df->redisplay = 1;
	DisplayTextWindow(NULL, 1);	
}

static char FindTextInfo(void) {
	register int i;
	unCH buf[BUFSIZ];
	LINE *tl;

	if (global_df->col_txt->c != HIDEN_C)
		tl = global_df->head_row->next_char;
	else
		tl = global_df->col_txt->next_char;
	i = 0;
	while (tl != global_df->tail_row && TEXTTIER[i]) {
		if (tl->c != TEXTTIER[i])
			break;
		i++;
		tl = tl->next_char;
	}
	if (TEXTTIER[i] != EOS)
		return(0);
	while (tl != global_df->tail_row && (isSpace(tl->c) || tl->c == '_'))
		tl = tl->next_char;
	if (tl == global_df->tail_row)
		return(0);
	if (tl->c != '"')
		return(0);

	tl = tl->next_char;
	if (tl == global_df->tail_row)
		return(0);
	for (i=0; tl != global_df->tail_row && tl->c != '"'; tl = tl->next_char)
		buf[i++] = tl->c;
	buf[i] = EOS;
	UnicodeToUTF8(buf, i, (unsigned char *)global_df->TxTr.textFName, NULL, FNSize);
	if (!mOpenTextFile())
		return(-1);
	global_df->TxTr.textChanged = TRUE;
	return(1);
}

static char FindPictInfo(char isAlwaysFind) {
	register int i;
	unCH buf[BUFSIZ];
	LINE *tl;

	if (global_df->col_txt->c != HIDEN_C)
		tl = global_df->head_row->next_char;
	else
		tl = global_df->col_txt->next_char;
	i = 0;
	while (tl != global_df->tail_row && PICTTIER[i]) {
		if (tl->c != PICTTIER[i])
			break;
		i++;
		tl = tl->next_char;
	}
	if (PICTTIER[i] != EOS)
		return(0);
	while (tl != global_df->tail_row && (isSpace(tl->c) || tl->c == '_'))
		tl = tl->next_char;
	if (tl == global_df->tail_row)
		return(0);
	if (tl->c != '"')
		return(0);

	tl = tl->next_char;
	if (tl == global_df->tail_row)
		return(0);
	for (i=0; tl != global_df->tail_row && tl->c != '"'; tl = tl->next_char)
		buf[i++] = tl->c;
	buf[i] = EOS;
	if (isAlwaysFind || !global_df->isErrorFindPict) {
		UnicodeToUTF8(buf, i, (unsigned char *)global_df->PcTr.pictFName, NULL, FNSize);
		if (!mOpenPictFile()) {
			global_df->isErrorFindPict = TRUE;
			return(-1);
		}
	}
	global_df->PcTr.pictChanged = TRUE;
	return(1);
}

static char FindMediaInfoInLine(unCH *line, int index, char mediaType, unCH *MediaFileName, char *bf, char isLocateMedia) {
	register int i;
	long beg, end;
	char nf, isWhatType = 0, isDashFound, isOldBullet;
	unCH buf[FILENAME_MAX];
	movInfo tempMovie;
	movInfo *tMovie;

	if (line[index] == HIDEN_C)
		index++;

	if (is_unCH_digit(line[index])) {
		if (global_df->mediaFileName[0] == EOS) {
			sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
			return(0);
		}
	} else if (!uS.partwcmp(line+index, SOUNDTIER) && !uS.partwcmp(line+index, REMOVEMOVIETAG))
		return(0);
	else
		index += 5;

	buf[0] = EOS;
	if (is_unCH_digit(line[index])) {
		if (global_df->mediaFileName[0] != EOS) {
			strcpy(buf, global_df->mediaFileName);
		}
		isOldBullet = FALSE;
	} else {
		for (; line[index] && (isSpace(line[index]) || line[index] == '_'); index++) ;
		if (line[index] != '"')
			return(0);
		index++;
		if (line[index] == EOS)
			return(0);
		for (i=0; line[index] && line[index] != '"' && line[index] != HIDEN_C; index++)
			buf[i++] = line[index];
		buf[i] = EOS;
		if (line[index] != '"')
			return(0);
		isOldBullet = TRUE;
	}
	if (line[index] == EOS || line[index] == HIDEN_C)
		return(0);

	if (isPlayAudioFirst)
		nf = strcmp(buf, global_df->SnTr.SoundFile);
	else
		nf = TRUE;
	if (nf) {
		if (global_df->SnTr.SoundFile[0] != EOS && !syncAudioAndVideo(buf)) {
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
				if (global_df->SnTr.mp3.hSys7SoundData)
					DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
				global_df->SnTr.mp3.theSoundMedia = NULL;
				global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
			}
			global_df->SnTr.SoundFile[0] = EOS;
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
	} else
		isWhatType = isAudio;

	if (nf) {
		if (isPlayAudioFirst) {
			strcpy(global_df->SnTr.SoundFile, buf);
			if (!mOpenSoundFile()) {
				strcpy(tempMovie.MovieFile, buf);
				strcpy(tempMovie.rMovieFile, global_df->MvTr.rMovieFile);
				if (mOpenMovieFile(&tempMovie) != 0) {
					if (!isLocateMedia && MediaFileName != NULL && strcmp(MediaFileName, tempMovie.MovieFile) == 0)
						;
					else {
						if (global_df->SnTr.errMess != NULL) {
							do_warning(global_df->SnTr.errMess, 0);
							global_df->err_message[0] = EOS;
							global_df->SnTr.errMess = NULL;
						} else
							CantFindMedia(global_df->err_message, tempMovie.MovieFile, isOldBullet);
						if (!isLocateMedia) {
							if (MediaFileName != NULL)
								strcpy(MediaFileName, tempMovie.MovieFile);
							return(0);
						} else
							return(-1);
					}
				} else
					isWhatType = isVideo;
			} else
				isWhatType = isAudio;
		} else {
			strcpy(tempMovie.MovieFile, buf);
			strcpy(tempMovie.rMovieFile, global_df->MvTr.rMovieFile);
			if (mOpenMovieFile(&tempMovie) != 0) {
				strcpy(global_df->SnTr.SoundFile, buf);
				if (!mOpenSoundFile()) {
					if (!isLocateMedia && MediaFileName != NULL && strcmp(MediaFileName, buf) == 0)
						;
					else {
						if (global_df->SnTr.errMess != NULL) {
							do_warning(global_df->SnTr.errMess, 0);
							global_df->err_message[0] = EOS;
							global_df->SnTr.errMess = NULL;
						} else
							CantFindMedia(global_df->err_message, buf, isOldBullet);
						if (!isLocateMedia) {
							if (MediaFileName != NULL)
								strcpy(MediaFileName, buf);
							return(0);
						} else
							return(-1);
					}
				} else
					isWhatType = isAudio;
			} else
				isWhatType = isVideo;
		}
	}

	if (mediaType != isWhatType)
		return(0);

	for (i=index; line[i] && line[i] != HIDEN_C; i++) ;
	if (line[i] == HIDEN_C) {
		for (i--; isSpace(line[i]) && i > 0; i--) ;
	}
	if (line[i] == '-' || isSkipPause)
		isDashFound = TRUE;
	else
		isDashFound = FALSE;

	if (isWhatType == isVideo) {
		if (global_df->cpMvTr == NULL) {
			tMovie = &global_df->MvTr;
		} else {
			for (tMovie=global_df->cpMvTr; tMovie->nMovie != NULL; tMovie=tMovie->nMovie) ;
		}
		if (MediaFileName == NULL || *MediaFileName == EOS) {
			if (MediaFileName != NULL)
				strcpy(MediaFileName, tempMovie.MovieFile);
			strcpy(tMovie->MovieFile, tempMovie.MovieFile);
			strcpy(tMovie->rMovieFile, tempMovie.rMovieFile);
		} else if (uS.mStricmp(tempMovie.MovieFile, MediaFileName) || isDashFound) {
			if (global_df->cpMvTr == NULL) {
				global_df->cpMvTr = NEW(movInfo);
				tMovie = global_df->cpMvTr;
			} else {
				tMovie->nMovie = NEW(movInfo);
				tMovie = tMovie->nMovie;
			}
			if (tMovie == NULL)
				return(-1);
			tMovie->nMovie = NULL;
			if (MediaFileName != NULL)
				strcpy(MediaFileName, tempMovie.MovieFile);
			strcpy(tMovie->MovieFile, tempMovie.MovieFile);
			strcpy(tMovie->rMovieFile, tempMovie.rMovieFile);
			if (bf != NULL)
				*bf = FALSE;
		}
	} else
		tMovie = NULL;
	for (; line[index] && !is_unCH_digit(line[index]) && line[index] != HIDEN_C; index++) ;
	if (!is_unCH_digit(line[index]))
		return(-1);
	for (i=0; line[index] && is_unCH_digit(line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	beg = uS.atol(buf);
	for (; line[index] && !is_unCH_digit(line[index]) && line[index] != HIDEN_C; index++) ;
	if (!is_unCH_digit(line[index]))
		return(-1);
	for (i=0; line[index] && is_unCH_digit(line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	end = uS.atol(buf);
	if (isWhatType == isVideo) {
		if (bf != NULL && *bf == FALSE) {
			tMovie->MBeg = beg;
			*bf = TRUE;
		}
		tMovie->MEnd = end;
	 	tMovie->nMovie = NULL;
	} else {
		DrawSoundCursor(0);
		global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(beg), '-');
		SetPBCglobal_df(false, 0L);

		global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(end), '-');
		if (global_df->SnTr.IsSoundOn && isDisplayWave && (!PlayingContSound || PlayingContSound == '\004')) {
			DisplayEndF(FALSE);
			if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
				SaveUndoState(FALSE);
		}
		if (isDashFound)
			global_df->SnTr.isNaturalPause = FALSE;
		else
			global_df->SnTr.isNaturalPause = TRUE;
	}
	return(1);
}

static char FindSndInfo(char *isWhatType) {
	register int i;
	char isOldBullet;
	unCH buf[BUFSIZ];
	LINE *tl;
	int nf;

	findMediaTiers();
	if (global_df->col_txt->c != HIDEN_C)
		return(-1);

	tl = global_df->col_txt->next_char;
	if (tl == global_df->tail_row || (tl->c != '%' && !is_unCH_digit(tl->c))) {
		tl = global_df->col_txt->prev_char;
		while (tl != global_df->head_row && tl->c != HIDEN_C)
			tl = tl->prev_char;
		tl = tl->next_char;
	}

	buf[0] = EOS;
	if (is_unCH_digit(tl->c)) {
		if (global_df->mediaFileName[0] == EOS) {
			sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
			return(-1);
		}
		if (global_df->mediaFileName[0] != EOS) {
			strcpy(buf, global_df->mediaFileName);
		}
		isOldBullet = FALSE;
	} else {
		i = 0;
		while (tl != global_df->tail_row && tl->c != ':' && tl->c != HIDEN_C) {
			buf[i++] = tl->c;
			tl = tl->next_char;
		}
		if (tl->c == ':') {
			buf[i++] = tl->c;
			tl = tl->next_char;
		}
		buf[i] = EOS;
		if (!uS.partwcmp(buf, SOUNDTIER) && !uS.partwcmp(buf, REMOVEMOVIETAG)) 
			return(-1);

		while (tl != global_df->tail_row && (isSpace(tl->c) || tl->c == '_'))
			tl = tl->next_char;
		if (tl == global_df->tail_row || tl->c != '"') {
			sprintf(global_df->err_message, "+%s tier is corrupted, missing '\"' around file name.", SOUNDTIER);
			return(-1);
		}
		tl = tl->next_char;
		if (tl == global_df->tail_row || tl->c == HIDEN_C)
			return(-1);
		for (i=0; tl != global_df->tail_row && tl->c != '"' && tl->c != HIDEN_C; tl = tl->next_char)
			buf[i++] = tl->c;
		buf[i] = EOS;
		if (tl == global_df->tail_row || tl->c != '"') {
			sprintf(global_df->err_message, "+%s tier is corrupted, missing '\"' around file name.", SOUNDTIER);
			return(-1);
		}
		isOldBullet = TRUE;
	}

	uS.lowercasestr(buf, &dFnt, FALSE);
	if (syncAudioAndVideo(buf))
		nf = -1;
	else if (isPlayAudioFirst) {
		if (strcmp(buf, global_df->SnTr.SoundFile))
			nf = 1;
		else
			nf = 0;
	} else
		nf = 1;

	if (nf > 0) {
		if (global_df->SnTr.SoundFile[0] != EOS && !syncAudioAndVideo(buf)) {
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
				if (global_df->SnTr.mp3.hSys7SoundData)
					DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
				global_df->SnTr.mp3.theSoundMedia = NULL;
				global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
			}
			global_df->SnTr.SoundFile[0] = EOS;
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
	} else
		*isWhatType = isAudio;
	if (nf > 0) {
		if (isPlayAudioFirst) {
			strcpy(global_df->SnTr.SoundFile, buf);
			if (!mOpenSoundFile()) {
				strcpy(global_df->MvTr.MovieFile, buf);
				if (mOpenMovieFile(&global_df->MvTr) != 0) {
					if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					} else
						CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, isOldBullet);
					global_df->MvTr.MovieFile[0] = EOS;
					return(-2);
				} else {
					*isWhatType = isVideo;
				}
			} else {
				global_df->MvTr.MovieFile[0] = EOS;
				*isWhatType = isAudio;
			}
		} else {
			strcpy(global_df->MvTr.MovieFile, buf);
			if (mOpenMovieFile(&global_df->MvTr) != 0) {
				strcpy(global_df->SnTr.SoundFile, buf);
				if (!mOpenSoundFile()) {
					if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					} else
						CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, isOldBullet);
					return(-2);
				} else {
					global_df->MvTr.MovieFile[0] = EOS;
					*isWhatType = isAudio;
				}
			} else {
				*isWhatType = isVideo;
			}
		}
	}

	while (tl != global_df->tail_row && !is_unCH_digit(tl->c) && tl->c != HIDEN_C)
		tl = tl->next_char;
	if (tl == global_df->tail_row || !is_unCH_digit(tl->c)) {
		return(-1);
	}

	for (i=0; tl != global_df->tail_row && is_unCH_digit(tl->c); tl = tl->next_char)
		buf[i++] = tl->c;
	buf[i] = EOS;
	DrawSoundCursor(0);
	if (*isWhatType == isVideo || nf == -1)
		global_df->MvTr.MBeg = uS.atol(buf);

	if (*isWhatType == isAudio)
		global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(uS.atol(buf)), '-');

	if (*isWhatType == isVideo || nf == -1)
		global_df->MvTr.MEnd = 0L;

	if (*isWhatType == isAudio) {
		global_df->SnTr.EndF = 0L;
		global_df->SnTr.isNaturalPause = TRUE;
		SetPBCglobal_df(false, 0L);
	}

	while (tl != global_df->tail_row && !is_unCH_digit(tl->c) && tl->c != HIDEN_C)
		tl = tl->next_char;
	if (tl == global_df->tail_row || !is_unCH_digit(tl->c)) {
		return(-1);
	}

	for (i=0; tl != global_df->tail_row && is_unCH_digit(tl->c); tl = tl->next_char)
		buf[i++] = tl->c;
	buf[i] = EOS;
	if (*isWhatType == isVideo || nf == -1)
		global_df->MvTr.MEnd = uS.atol(buf);

	if (*isWhatType == isAudio) {
		global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(uS.atol(buf)), '-');
		if (global_df->SnTr.IsSoundOn && isDisplayWave && (!PlayingContSound || PlayingContSound == '\004')) {
			DisplayEndF(FALSE);
			if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
				SaveUndoState(FALSE);
		}
	}

	if (*isWhatType == isAudio) {
		if ((tl->c == '-' && tl != global_df->tail_row) || isSkipPause)
			global_df->SnTr.isNaturalPause = FALSE;
		else
			global_df->SnTr.isNaturalPause = TRUE;
	}

	if (nf == 0)
		return(1);
	else
		return(2);
}

char FindSndInfoAndCopyIt(FNType *fname, long *beg, long *end) {
	char isWhatType;

	fname[0] = EOS;
	FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER));
	if (FindSndInfo(&isWhatType) <= 0) {
		return(FALSE);
	} else {
		if (isWhatType == isVideo && sendMessageTargetApp == PRAAT) {
			FNType	*ext;
#ifdef _MAC_CODE
			Movie	cMovie;
			WindowProcRec *windProc;

			windProc = WindowProcs(FindAWindowNamed(Movie_sound_str));
			if (windProc && windProc->id == 500 && theMovie != NULL)
				cMovie = theMovie->MvMovie;
			else
				return(FALSE);
#endif
#ifdef _WIN32
			if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpPlayer != NULL) {
			} else {
				return(FALSE);
			}
#endif

			strcpy(FileName1, global_df->MvTr.rMovieFile);
			ext = strrchr(FileName1, '.');
			if (ext != NULL)
				uS.str2FNType(ext, 0L, ".wav");
			else
				uS.str2FNType(FileName1, strlen(FileName1), ".wav");
			if (access(FileName1, 0))
#ifdef _MAC_CODE
				SaveSoundMovieAsWAVEFile(cMovie, FileName1);
#else //_WIN32
				SaveSoundMovieAsWAVEFile(global_df->MvTr.rMovieFile, FileName1);
#endif
			strcat(fname, FileName1);
			*beg = global_df->MvTr.MBeg;
			*end = global_df->MvTr.MEnd;
		} else {
			strcat(fname, global_df->SnTr.rSoundFile);
			*beg = conv_to_msec_rep(global_df->SnTr.BegF);
			*end = conv_to_msec_rep(global_df->SnTr.EndF);
		}
		return(TRUE);
	}
}

void CopyAndFreeMovieData(char all) {
	movInfo *tMovie;

	if (global_df->cpMvTr == NULL)
		return;

	if (!all) {
		strcpy(global_df->MvTr.MovieFile, global_df->cpMvTr->MovieFile);
		strcpy(global_df->MvTr.rMovieFile, global_df->cpMvTr->rMovieFile);
		global_df->MvTr.MBeg = global_df->cpMvTr->MBeg;
		global_df->MvTr.MEnd = global_df->cpMvTr->MEnd;
	}
	do {
		tMovie = global_df->cpMvTr;
		global_df->cpMvTr = global_df->cpMvTr->nMovie;
		free(tMovie);
	} while (all && global_df->cpMvTr != NULL) ;
}

int SoundMode(int i) {
	char isWhatType;
	char isMovieDialog;
#ifdef _MAC_CODE // NO QT
	Movie cMovie;
#endif
	long beg = 0L, end = 0L;
#ifdef _MAC_CODE
	WindowPtr win;
#endif

	cChan = -1;
	if (global_df == NULL)
		return(63);
	if (global_df->SnTr.IsSoundOn) {
		soundwindow(0);
		global_df->SnTr.IsSoundOn = FALSE;
		return(63);
	}
	if (!global_df->EditorMode || global_df->RowLimit) {
		RemoveLastUndo();
		if (global_df->RowLimit)
			strcpy(global_df->err_message, "+Illegal in this window.");
		else
			strcpy(global_df->err_message, "+Illegal in coder mode.");
		return(63);
	}
	if (i <= 0)
		draw_mid_wm();
	ChangeCurLineAlways(0);
	if (i != 2) {
		isMovieDialog = FALSE;
		FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER));
#ifdef _MAC_CODE
		cMovie = NULL;
		if (theMovie != NULL && theMovie->MvMovie != NULL) {
			if (theMovie->isPlaying) {
				stopMoviePlaying();
			}
			if (global_df->MvTr.MovieFile[0] != EOS && global_df->MvTr.rMovieFile[0] != EOS) {
				isMovieDialog = TRUE;
				cMovie = theMovie->MvMovie;
			}
		}
#elif _WIN32
/* // NO QT
		cMovie = NULL;
		if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->theMovie != NULL) {
			if (!(MovDlg->qt)->IsQTMovieDone()) {
				stopMoviePlaying();
			}
			if (global_df->MvTr.MovieFile[0] != EOS && global_df->MvTr.rMovieFile[0] != EOS) {
				isMovieDialog = TRUE;
				cMovie = (MovDlg->qt)->theMovie;
			}
		}
*/
		if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpPlayer != NULL) {
			if (!(MpegDlg->mpeg)->IsMpegMovieDone()) {
				stopMoviePlaying();
			}
			if (global_df->MvTr.MovieFile[0] != EOS && global_df->MvTr.rMovieFile[0] != EOS) {
				isMovieDialog = TRUE;
// NO QT				cMovie = NULL;
			}
		}
#endif
		ced_line[0] = EOS;
		if (isMovieDialog) {
			beg = global_df->MvTr.MBeg;
			end = global_df->MvTr.MEnd;
			strcpy(ced_line, global_df->MvTr.MovieFile);
		}
		isWhatType = 0;
		if ((FindSndInfo(&isWhatType) <= 0)) {
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
			if (isMovieDialog) {
				isWhatType = isVideo;
				global_df->err_message[0] = EOS;
			} else if (global_df->SnTr.errMess == NULL) {
				isWhatType = 0;
				if (global_df->mediaFileName[0] != EOS) {
					if (isPlayAudioFirst) {
						strcpy(global_df->SnTr.SoundFile, global_df->mediaFileName);
						if (!mOpenSoundFile()) {
							strcpy(global_df->MvTr.MovieFile, global_df->mediaFileName);
							if (mOpenMovieFile(&global_df->MvTr) == 0) {
								global_df->MvTr.MBeg = 0L;
								global_df->MvTr.MEnd = 0L;
								isWhatType = isVideo;
							} else {
								if (global_df->SnTr.errMess != NULL) {
									do_warning(global_df->SnTr.errMess, 0);
									global_df->err_message[0] = EOS;
									global_df->SnTr.errMess = NULL;
								} else
									CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, FALSE);
								return(63);
							}
						} else {
							global_df->SnTr.BegF = 0L;
							global_df->SnTr.EndF = 0L;
							global_df->SnTr.WBegFM = 0L;
							global_df->SnTr.WEndFM = 0L;
							isWhatType = isAudio;
						}
					} else {
						strcpy(global_df->MvTr.MovieFile, global_df->mediaFileName);
						if (mOpenMovieFile(&global_df->MvTr) != 0) {
							strcpy(global_df->SnTr.SoundFile, global_df->mediaFileName);
							if (mOpenSoundFile()) {
								global_df->SnTr.BegF = 0L;
								global_df->SnTr.EndF = 0L;
								global_df->SnTr.WBegFM = 0L;
								global_df->SnTr.WEndFM = 0L;
								isWhatType = isAudio;
							} else {
								if (global_df->SnTr.errMess != NULL) {
									do_warning(global_df->SnTr.errMess, 0);
									global_df->err_message[0] = EOS;
									global_df->SnTr.errMess = NULL;
								} else
									CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, FALSE);
								return(63);
							}
						} else {
							global_df->MvTr.MBeg = 0L;
							global_df->MvTr.MEnd = 0L;
							isWhatType = isVideo;
						}
					}
				}
				if (isWhatType == 0) {
					if ((isWhatType=GetNewMediaFile(FALSE, isAllType)) == 0) {
						RemoveLastUndo();
						return(63);
					}
				}
				if (isWhatType == isVideo) {
					PlayMovie(&global_df->MvTr, global_df, TRUE);
#ifdef _MAC_CODE
					win = FindAWindowNamed(Movie_sound_str);
					theMovie->df = WindowFromGlobal_df(theMovie->df);
					if (win == NULL || theMovie == NULL || theMovie->df == NULL) {
						global_df->err_message[0] = EOS;
						do_warning("Please open movie window first with F5 or F4 function", 0);
						return(63);
					}
					changeCurrentWindow(win, theMovie->df->wind, true);
					if (theMovie != NULL && theMovie->MvMovie != NULL) {
						isMovieDialog = TRUE;
						cMovie = theMovie->MvMovie;
					}
#elif _WIN32
					if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpPlayer != NULL) {
						isMovieDialog = TRUE;
					}
#endif
					isWhatType = isVideo;
					if (isMovieDialog) {
						beg = global_df->MvTr.MBeg;
						end = global_df->MvTr.MEnd;
						strcpy(ced_line, global_df->MvTr.MovieFile);
					} else {
						global_df->err_message[0] = EOS;
						do_warning("Please open movie window first with F5 or F4 function", 0);
						return(63);
					}
				}
			}
		}
		if (isWhatType == isVideo && global_df->SnTr.errMess == NULL) {
			if (!isMovieDialog) {
				PlayMovie(&global_df->MvTr, global_df, TRUE);
#ifdef _MAC_CODE
				win = FindAWindowNamed(Movie_sound_str);
				theMovie->df = WindowFromGlobal_df(theMovie->df);
				if (win == NULL || theMovie == NULL || theMovie->df == NULL) {
					global_df->err_message[0] = EOS;
					do_warning("Please open movie window first with F5 or F4 function", 0);
					return(63);
				}
				changeCurrentWindow(win, theMovie->df->wind, true);
				if (theMovie != NULL && theMovie->MvMovie != NULL) {
					isMovieDialog = TRUE;
					cMovie = theMovie->MvMovie;
				}
#elif _WIN32
				if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpPlayer != NULL) {
					isMovieDialog = TRUE;
				}
#endif
				if (isMovieDialog) {
					beg = global_df->MvTr.MBeg;
					end = global_df->MvTr.MEnd;
					strcpy(ced_line, global_df->MvTr.MovieFile);
				} else {
					global_df->err_message[0] = EOS;
					do_warning("Please open movie window first with F5 or F4 function", 0);
					return(63);
				}
			}
			if (end != 0L && !strcmp(ced_line, global_df->MvTr.MovieFile)) {
				global_df->MvTr.MBeg = beg;
				global_df->MvTr.MEnd = end;
			}
			strcpy(ced_line, global_df->MvTr.MovieFile);
//			strcat(ced_line, "-clanw");
			strcat(ced_line, ".wav");
			strcpy(global_df->SnTr.SoundFile, ced_line);
			if (!mOpenSoundFile()) {
#ifdef _MAC_CODE
				if (isMovieStreaming(cMovie)) {
					if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					} else {
						global_df->err_message[0] = EOS;
						do_warning("Can't extract wave file from streaming movie.", 0);
					}
					return(63);
				}
#endif
				extractPath(FileName1, global_df->MvTr.rMovieFile);
				u_strcpy(ced_lineC, ced_line, UTTLINELEN);
				addFilename2Path(FileName1, ced_lineC);
				if (access(FileName1, 0))
#ifdef _MAC_CODE
					SaveSoundMovieAsWAVEFile(cMovie, FileName1);
#else //_WIN32
					SaveSoundMovieAsWAVEFile(global_df->MvTr.rMovieFile, FileName1);
#endif
				strcpy(global_df->SnTr.SoundFile, ced_line);
				if (!mOpenSoundFile()) {
					if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					} else {
						global_df->err_message[0] = EOS;
						do_warning("Can't extract or open wave file from movie file.", 0);
					}
					return(63);
				}
			}
			global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(global_df->MvTr.MBeg), '+');
			global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(global_df->MvTr.MEnd), '+');
		}
	}
	if (global_df->SnTr.errMess != NULL) {
		do_warning(global_df->SnTr.errMess, 0);
		global_df->err_message[0] = EOS;
		global_df->SnTr.errMess = NULL;
	} else {
		global_df->SnTr.IsSoundOn = (char)(soundwindow(1) == 0);
		if (global_df->SnTr.IsSoundOn) {
			cChan = global_df->SnTr.SNDchan;
		}
	}
	return(63);
}

static char isRightBullet(unCH *line) {
	long i;

	for (i=0; line[i]; i++) {
		if (line[i] == HIDEN_C) {
			i++;
			if (is_unCH_digit(line[i]))
				return(TRUE);
			if (uS.partwcmp(line+i, SOUNDTIER) || uS.partwcmp(line+i, REMOVEMOVIETAG))
				return(TRUE);
		}
	}
	return(FALSE);
}

struct sound_clips {
	unCH	SoundFile[FILENAME_MAX];
	FNType  rSoundFile[FNSize];
	long	BegF, EndF;
	char	isNaturalPause;
	struct sound_clips *nextClip;
} ;

static void freeClips(struct sound_clips *p) {
	struct sound_clips *t;

	while (p != NULL) {
		t = p;
		p = p->nextClip;
		free(t);
	}
}

static struct sound_clips *AddToSoundClip(struct sound_clips *root) {
	char isJoined;
	struct sound_clips *t;

	if (root == NULL) {
		root = NEW(struct sound_clips);
		if (root == NULL)
			return(NULL);
		t = root;
		isJoined = FALSE;
		t->EndF = 0L;
	} else {
		for (t=root; t->nextClip != NULL; t=t->nextClip) ;
		if (!global_df->SnTr.isNaturalPause || uS.mStricmp(t->rSoundFile, global_df->SnTr.rSoundFile)) {
			t->nextClip = NEW(struct sound_clips);
			if (t->nextClip == NULL) {
				freeClips(root);
				return(NULL);
			}
			t = t->nextClip;
			isJoined = FALSE;
			t->EndF = 0L;
		} else {
			isJoined = TRUE;
		}
	}
	t->nextClip = NULL;
	if (!isJoined) {
		t->BegF = global_df->SnTr.BegF;
		strcpy(t->SoundFile, global_df->SnTr.SoundFile);
		strcpy(t->rSoundFile, global_df->SnTr.rSoundFile);
	}
	if (global_df->SnTr.EndF > t->EndF)
		t->EndF = global_df->SnTr.EndF;
	t->isNaturalPause = global_df->SnTr.isNaturalPause;
	return(root);
}

void checkContinousSndPlayPossition(long CurFP) {
	char ret;
	long tBeg, tEnd;

	SetPBCglobal_df(false, CurFP);
	if (PlayingContSound && PlayingContSound != '\004') {
		if (CurFP < global_df->SnTr.dBegF || CurFP > global_df->SnTr.dEndF) {
			DrawCursor(0);
			DrawSoundCursor(0);
			ret = move_cursor(conv_to_msec_rep(CurFP), global_df->SnTr.SoundFile, FALSE, TRUE);
			if (ret == FALSE || ret == '\003') {
				global_df->row_win2 = 0L;
				global_df->col_win2 = -2L;
				global_df->col_chr2 = -2L;
//				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
//				wrefresh(global_df->w1);
			} else if (ret == '\002') {
			} else {
				if (global_df->SnTr.IsSoundOn && isDisplayWave) {
					tBeg = global_df->SnTr.BegF;
					global_df->SnTr.BegF = global_df->SnTr.dBegF;
					tEnd = global_df->SnTr.EndF;
					global_df->SnTr.EndF = global_df->SnTr.dEndF;
					DisplayEndF(FALSE);
					if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
						SaveUndoState(FALSE);
					global_df->SnTr.BegF = tBeg;
					global_df->SnTr.EndF = tEnd;
				}
			}
			ret = global_df->ScrollBar;
			global_df->ScrollBar = '\255';
			SetScrollControl();
			global_df->ScrollBar = ret;
			DrawCursor(1);
			DrawSoundCursor(1);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			draw_mid_wm();
			global_df->RdW = global_df->w1;
		}
		if (global_df->row_win2 || global_df->col_win2 != -2)
			global_df->LeaveHighliteOn = TRUE;
		else
			global_df->LeaveHighliteOn = FALSE;
	}
}

static void PlayContSound() {
	char res, sndfound, snd_row_found, hasChanged, isNonMainSpeakerFound;
	unCH SoundFileName[FILENAME_MAX];
	int  i;
	ROWS *curRow;
	struct sound_clips *sClips, *t;

	global_df->SnTr.contPlayBeg = 0L;
	global_df->SnTr.contPlayEnd = 0L;
	PlayingContSound = TRUE;
	PlayingContMovie = FALSE;
	sClips = NULL;
	ChangeCurLineAlways(0);
	sndfound = FALSE;
	snd_row_found = FALSE;
	isKillProgram = 0;
	curRow = global_df->row_txt;
	isNonMainSpeakerFound = FALSE;
	while (1) {
		if (!isNonMainSpeakerFound && curRow != global_df->row_txt && isRightBullet(curRow->line)) {
			curRow = ToNextRow(curRow, FALSE);
			snd_row_found = TRUE;
			break;
		}
		if (*curRow->line == '%' || *curRow->line == '@')
			isNonMainSpeakerFound = TRUE;
		if (isMainSpeaker(*curRow->line)) {
			snd_row_found = TRUE;
			break;
		}
		if (AtTopEnd(curRow, global_df->head_text, FALSE))
			break;
		curRow = ToPrevRow(curRow, FALSE);
	}
	while (1) {
		if (snd_row_found) {
			for (i=0; curRow->line[i]; i++) {
				if (curRow->line[i] == HIDEN_C) {
					res = FindMediaInfoInLine(curRow->line, i, isAudio, SoundFileName, NULL, FALSE);
					if (res == -1) {
						PlayingContSound = FALSE;
						freeClips(sClips);
						strcpy(global_df->err_message, "+Media bullet is corrupted. Please run check.");
						return;
					} else if (res == 1) {
						sndfound = TRUE;
						sClips = AddToSoundClip(sClips);
						snd_row_found = FALSE;
						break;
					}
					for (i++; curRow->line[i] != EOS && curRow->line[i] != HIDEN_C; i++) ;
					if (curRow->line[i] != HIDEN_C)
						i--;
				}
			}
		}
		curRow = ToNextRow(curRow, FALSE);
		if (isMainSpeaker(*curRow->line))
			snd_row_found = TRUE;
		if (FoundEndHTier(curRow, FALSE) || AtBotEnd(curRow, global_df->tail_text, FALSE))
			break;
	}
	
//AfxGetApp()->m_pMainWnd->MessageBox("before strcmp", "", MB_ICONWARNING);
	if (sClips != NULL) {
		*global_df->PcTr.pictFName = EOS;
		global_df->PcTr.pictChanged = FALSE;
		*global_df->TxTr.textFName = EOS;
		global_df->TxTr.textChanged = FALSE;
		hasChanged = uS.mStricmp(sClips->rSoundFile, global_df->SnTr.rSoundFile);
//AfxGetApp()->m_pMainWnd->MessageBox("after strcmp", "", MB_ICONWARNING);
		while (sClips != NULL) {
			if (hasChanged) {
				if (global_df->SnTr.SoundFile[0] != EOS) {
					if (global_df->SnTr.isMP3 == TRUE) {
						global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
						if (global_df->SnTr.mp3.hSys7SoundData)
							DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
						global_df->SnTr.mp3.theSoundMedia = NULL;
						global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
					} else if (global_df->SnTr.SoundFPtr != 0) {
						fclose(global_df->SnTr.SoundFPtr);
					}
					global_df->SnTr.SoundFPtr = 0;
					if (global_df->SoundWin)
						DisposeOfSoundWin();
				}
			}
			strcpy(global_df->SnTr.SoundFile, sClips->SoundFile);
			strcpy(global_df->SnTr.rSoundFile, sClips->rSoundFile);
			global_df->SnTr.dBegF = sClips->EndF;
			global_df->SnTr.dEndF = sClips->EndF;
			global_df->SnTr.BegF = sClips->BegF;
			global_df->SnTr.EndF = sClips->EndF;
			global_df->SnTr.isNaturalPause = sClips->isNaturalPause;
			SetPBCglobal_df(false, 0L);
			if (hasChanged) {
				if (!mOpenSoundFile()) {
					DrawCursor(0);
					PlayingContSound = FALSE;
					freeClips(sClips);
					if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					}
					return;
				}
			}
			PlayingContSound = TRUE;
			strcpy(global_df->err_message, DASHES);
			global_df->SnTr.SSource = 1;
			if (PlaySound(&global_df->SnTr, (int)'r')) {
				DrawCursor(0);
				DrawSoundCursor(0);
				PlayingContSound = FALSE;
				global_df->SnTr.contPlayBeg = 0L;
				global_df->SnTr.contPlayEnd = 0L;
				freeClips(sClips);
				return;
			}
			global_df->RdW = global_df->w1;
			if (ContPlayPause()) {
				DrawCursor(0);
				DrawSoundCursor(0);
				PlayingContSound = FALSE;
				global_df->SnTr.contPlayBeg = 0L;
				global_df->SnTr.contPlayEnd = 0L;
				freeClips(sClips);
				return;
			}
			t = sClips;
			sClips = sClips->nextClip;
			if (sClips != NULL) {
				hasChanged = uS.mStricmp(t->rSoundFile, sClips->rSoundFile);
			}
			free(t);
		}
	}
	DrawSoundCursor(0);
	PlayingContSound = FALSE;
	global_df->SnTr.contPlayBeg = 0L;
	global_df->SnTr.contPlayEnd = 0L;
	if (sndfound) {
		strcpy(global_df->err_message, "-Done.");
	} else
		strcpy(global_df->err_message, "+Sound marker not found at cursor position");
	freeClips(sClips);
}

static void PlayContMovie(void) {
	int  i;
	char mvfound, bf, res, isNonMainSpeakerFound;
	unCH MovieFileName[FILENAME_MAX];
	long cMBeg, cMEnd;
	ROWS *curRow;

	*global_df->PcTr.pictFName = EOS;
	global_df->PcTr.pictChanged = FALSE;
	*global_df->TxTr.textFName = EOS;
	global_df->TxTr.textChanged = FALSE;
	ChangeCurLineAlways(0);
	global_df->MvTr.MBeg = 0L;
	global_df->MvTr.MEnd = 0L;
	cMBeg = cMEnd = 0L;
	bf = FALSE;
	mvfound = FALSE;
	curRow = global_df->row_txt;
	*MovieFileName = EOS;
	isNonMainSpeakerFound = FALSE;
	CopyAndFreeMovieData(TRUE);
	while (1) {
		if (!isNonMainSpeakerFound && curRow != global_df->row_txt && isRightBullet(curRow->line)) {
			curRow = ToNextRow(curRow, FALSE);
			break;
		}
		if (*curRow->line == '%' || *curRow->line == '@')
			isNonMainSpeakerFound = TRUE;
		if (isMainSpeaker(*curRow->line)) {
			break;
		}
		if (AtTopEnd(curRow, global_df->head_text, FALSE))
			break;
		curRow = ToPrevRow(curRow, FALSE);
	}
	while (!AtBotEnd(curRow, global_df->tail_text, FALSE)) {
		for (i=0; curRow->line[i]; i++) {
			if (curRow->line[i] == HIDEN_C) {
				res = FindMediaInfoInLine(curRow->line, i, isVideo, MovieFileName, &bf, FALSE);
				if (res == -1) {
					strcpy(global_df->err_message, "+Media bullet is corrupted. Please run check.");
					return;
				} else if (res == 1) {
					if (global_df->MvTr.MBeg != 0L)
						cMBeg = global_df->MvTr.MBeg;
					if (global_df->MvTr.MEnd != 0L)
						cMEnd = global_df->MvTr.MEnd;
				}
				for (i++; curRow->line[i] != EOS && curRow->line[i] != HIDEN_C; i++) ;
				if (curRow->line[i] != HIDEN_C)
					i--;
			}
		}
		curRow = ToNextRow(curRow, FALSE);
	}
	if (curRow != global_df->tail_text) {
		for (i=0; curRow->line[i]; i++) {
			if (curRow->line[i] == HIDEN_C) {
				res = FindMediaInfoInLine(curRow->line, i, isVideo, MovieFileName, &bf, FALSE);
				if (res == -1) {
					strcpy(global_df->err_message, "+Media bullet is corrupted. Please run check.");
					return;
				}
			}
		}
		if (global_df->MvTr.MBeg != 0L)
			cMBeg = global_df->MvTr.MBeg;
		if (global_df->MvTr.MEnd != 0L)
			cMEnd = global_df->MvTr.MEnd;
	}
	global_df->MvTr.MBeg = cMBeg;
	global_df->MvTr.MEnd = cMEnd;
	mvfound = (global_df->MvTr.MEnd != 0L);

	if (mvfound) {
		PlayingContMovie = '\001';
		PlayMovie(&global_df->MvTr, global_df, FALSE);
	} else {
		PlayingContMovie = FALSE;
		strcpy(global_df->err_message, "+Movie marker not found at cursor position or below or bullets are corrupted");
	}
}

int PlayContMedia(int i) {
	int  c;
	char isWhatType, isOldBullet = FALSE;
	char type;
	ROWS *p;

	isSkipPause = FALSE;
	ChangeCurLineAlways(0);
	findMediaTiers();
	p = global_df->row_txt;
	while (1) {
		if (isMainSpeaker(*p->line)) {
			break;
		}
		if (AtTopEnd(p, global_df->head_text, FALSE))
			break;
		p = ToPrevRow(p, FALSE);
	}
	i = 0;
	type = 0;
	while (!AtBotEnd(p, global_df->tail_text, FALSE)) {
		for (i=0; p->line[i]; i++) {
			if (p->line[i] == HIDEN_C) {
				i++;
				if (is_unCH_digit(p->line[i])) {
					if (global_df->mediaFileName[0] != EOS) {
						strcpy(sp, SOUNDTIER);
						type = 1;
						break;
					} else {
						sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
						return(71);
					}
				} else {
					c = 0;
					for (; p->line[i] && p->line[i] != HIDEN_C && p->line[i] != ':'; i++) {
						if (c >= SPEAKERLEN)
							break;
						sp[c++] = p->line[i];
					}
					if (p->line[i] == ':') {
						sp[c] = EOS;
						if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
							type = 1;
							break;
						}
					}
				}
				for (; p->line[i] && p->line[i] != HIDEN_C; i++) ;
				if (p->line[i] == EOS)
					break;
			}
		}
		if (type != 0)
			break;
		p = ToNextRow(p, FALSE);
	}
	if (type == 1) {
		for (i++; p->line[i] != EOS && p->line[i] != HIDEN_C && (isSpace(p->line[i]) || p->line[i] == '_'); i++) ;
		sp[0] = EOS;
		isWhatType = 0;
		if (is_unCH_digit(p->line[i])) {
			if (global_df->mediaFileName[0] != EOS) {
				strcpy(sp, global_df->mediaFileName);
			} else {
				sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
				return(71);
			}
			isOldBullet = FALSE;
		} else if (p->line[i] == '"') {
			for (i++, c=0; p->line[i] != EOS && p->line[i] != '"'; i++, c++)
				sp[c] = p->line[i];
			sp[c] = EOS;
			if (global_df->mediaFileName[0] != EOS) {
				strcpy(global_df->err_message, "+Old format bullets found. Please close this data file and run \"fixbullets\" command on it.");
				return(71);
			}
			isOldBullet = TRUE;
		}
		if (sp[0] != EOS) {
			uS.lowercasestr(sp, &dFnt, FALSE);
			if (isPlayAudioFirst) {
				strcpy(global_df->SnTr.SoundFile, sp);
				if (!mOpenSoundFile()) {
					strcpy(global_df->MvTr.MovieFile, sp);
					if (mOpenMovieFile(&global_df->MvTr) == 0)
						isWhatType = isVideo;
					else if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					}
				} else
					isWhatType = isAudio;
			} else {
				strcpy(global_df->MvTr.MovieFile, sp);
				if (mOpenMovieFile(&global_df->MvTr) != 0) {
					strcpy(global_df->SnTr.SoundFile, sp);
					if (mOpenSoundFile())
						isWhatType = isAudio;
					else if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					}
				} else
					isWhatType = isVideo;
			}
		}
		if (isWhatType == 0) {
			if (sp[0] == EOS)
				strcpy(global_df->err_message, "+Bullet under a cursor must be corrupted. Please point to a bullet associated with movie or sound and try again.");
			else
				CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, isOldBullet);
		} else if (isWhatType == isAudio) {
			PlayContSound();
		} else if (isWhatType == isVideo) {
			PlayContMovie();
		}
	} else
		strcpy(global_df->err_message, "+Can't find any bullets under a cursor. Please point to a bullet associated with movie, sound and try again.");

	return(71);
}

int PlayContSkipMedia(int i) {
	int  c;
	char isWhatType, isOldBullet = FALSE;
	char type;
	ROWS *p;

	isSkipPause = TRUE;
	ChangeCurLineAlways(0);
	findMediaTiers();
	p = global_df->row_txt;
	while (1) {
		if (isMainSpeaker(*p->line)) {
			break;
		}
		if (AtTopEnd(p, global_df->head_text, FALSE))
			break;
		p = ToPrevRow(p, FALSE);
	}
	i = 0;
	type = 0;
	while (!AtBotEnd(p, global_df->tail_text, FALSE)) {
		for (i=0; p->line[i]; i++) {
			if (p->line[i] == HIDEN_C) {
				i++;
				if (is_unCH_digit(p->line[i])) {
					if (global_df->mediaFileName[0] != EOS) {
						strcpy(sp, SOUNDTIER);
						type = 1;
						break;
					} else {
						sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
						isSkipPause = FALSE;
						return(97);
					}
				} else {
					c = 0;
					for (; p->line[i] && p->line[i] != HIDEN_C && p->line[i] != ':'; i++) {
						if (c >= SPEAKERLEN)
							break;
						sp[c++] = p->line[i];
					}
					if (p->line[i] == ':') {
						sp[c] = EOS;
						if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
							type = 1;
							break;
						}
					}
				}
				for (; p->line[i] && p->line[i] != HIDEN_C; i++) ;
				if (p->line[i] == EOS)
					break;
			}
		}
		if (type != 0)
			break;
		p = ToNextRow(p, FALSE);
	}
	if (type == 1) {
		for (i++; p->line[i] != EOS && p->line[i] != HIDEN_C && (isSpace(p->line[i]) || p->line[i] == '_'); i++) ;
		sp[0] = EOS;
		isWhatType = 0;
		if (is_unCH_digit(p->line[i])) {
			if (global_df->mediaFileName[0] != EOS) {
				strcpy(sp, global_df->mediaFileName);
			} else {
				sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
				isSkipPause = FALSE;
				return(97);
			}
			isOldBullet = FALSE;
		} else if (p->line[i] == '"') {
			for (i++, c=0; p->line[i] != EOS && p->line[i] != '"'; i++, c++)
				sp[c] = p->line[i];
			sp[c] = EOS;
			isOldBullet = TRUE;
		}
		if (sp[0] != EOS) {
			uS.lowercasestr(sp, &dFnt, FALSE);
			if (isPlayAudioFirst) {
				strcpy(global_df->SnTr.SoundFile, sp);
				if (!mOpenSoundFile()) {
					strcpy(global_df->MvTr.MovieFile, sp);
					if (mOpenMovieFile(&global_df->MvTr) == 0)
						isWhatType = isVideo;
					else if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					}
				} else
					isWhatType = isAudio;
			} else {
				strcpy(global_df->MvTr.MovieFile, sp);
				if (mOpenMovieFile(&global_df->MvTr) != 0) {
					strcpy(global_df->SnTr.SoundFile, sp);
					if (mOpenSoundFile())
						isWhatType = isAudio;
					else if (global_df->SnTr.errMess != NULL) {
						do_warning(global_df->SnTr.errMess, 0);
						global_df->err_message[0] = EOS;
						global_df->SnTr.errMess = NULL;
					}
				} else
					isWhatType = isVideo;
			}
		}
		if (isWhatType == 0) {
			if (sp[0] == EOS)
				strcpy(global_df->err_message, "+Bullet under a cursor must be corrupted. Please point to a bullet associated with movie or sound and try again.");
			else
				CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, isOldBullet);
		} else if (isWhatType == isAudio) {
			PlayContSound();
		} else if (isWhatType == isVideo) {
			PlayContMovie();
		}
	} else
		strcpy(global_df->err_message, "+Can't find any bullets under a cursor. Please point to a bullet associated with movie, sound and try again.");
	isSkipPause = FALSE;
	return(97);
}

/* sound commands end */
/*
static char isOnebullet(unCH *line) {
	long i, bulletCnt;

	bulletCnt = 0L;
	i = 0L;
	while (line[i] != EOS) {
		if (line[i] == HIDEN_C) {
			i++;
			if (is_unCH_digit(line[i]))
				bulletCnt++;
			else if (uS.partwcmp(line+i, SOUNDTIER) || uS.partwcmp(line+i, REMOVEMOVIETAG))
				bulletCnt++;
			while (line[i] && line[i] != HIDEN_C)
				i++;
			if (line[i] == HIDEN_C)
				i++;
		} else
			i++;
	}
	if (bulletCnt > 1L)
		return(FALSE);
	else
		return(TRUE);
}
*/
char FakeSelectWholeTier(char isEveryLine) {
	char res;
	ROWS *tr1, *tr2;

	if (global_df == NULL)
		return(FALSE);
#ifdef _MAC_CODE
	res = global_df->err_message[0];
	global_df->err_message[0] = EOS;
	global_df->WinChange = FALSE;
	draw_mid_wm();
	global_df->WinChange = TRUE;
	global_df->err_message[0] = res;
#endif
	SetScrollControl();
	ChangeCurLineAlways(0);
	DisplayRow(FALSE);
	global_df->fake_row_win  = global_df->row_win;
	global_df->fake_col_win  = 0L;
	global_df->fake_row_win2 = 0L;
	global_df->fake_col_win2 = 0L;
	tr1 = global_df->row_txt;
	tr2 = tr1;
	if (isEveryLine) {
		tr2 = ToNextRow(tr2, FALSE);
		global_df->fake_row_win2++;
	} else {
		while (!AtTopEnd(tr1, global_df->head_text, FALSE)) {
			if (tr1 != global_df->row_txt && isRightBullet(tr1->line)) {
				tr1 = ToNextRow(tr1, FALSE);
				global_df->fake_row_win++;
				global_df->fake_row_win2--;
				break;
			}
			if (isSpeaker(tr1->line[0]))
				break;
			if (global_df->fake_row_win <= 0L)
				break;
			tr1 = ToPrevRow(tr1, FALSE);
			global_df->fake_row_win--;
			global_df->fake_row_win2++;
		}
		while (!AtBotEnd(tr2, global_df->tail_text, FALSE)) {
			if (isRightBullet(tr2->line)) {
				tr2 = ToNextRow(tr2, FALSE);
				global_df->fake_row_win2++;
				break;
			}
			tr2 = ToNextRow(tr2, FALSE);
			global_df->fake_row_win2++;
			if (isSpeaker(tr2->line[0]))
				break;
		}
	}
	if (tr1 == tr2) {
		if (strlen(tr1->line) == 0/* || (tr1->line[0] == '*' && tr1->line[1] == ':')*/) {
			global_df->fake_row_win2 = 0L;
			global_df->fake_col_win2 = -2L;
			res = FALSE;
		} else {
			global_df->fake_col_win2 = ComColWin(FALSE, tr1->line, strlen(tr1->line));
			res = TRUE;
		}
	} else
		res = TRUE;
	return(res);
}

char SelectWholeTier(char isCAMode) {
	char res, isAddLineno;
	ROWS *tr;

	if (PlayingContMovie) {
		global_df->col_chr  = 0L;
		global_df->col_win  = 0L;
		global_df->row_win2 = 0L;
		global_df->col_chr2  = -2L;
		global_df->col_win2 = -2L;
		return(FakeSelectWholeTier(isCAMode));
	}
	ChangeCurLineAlways(0);
	DisplayRow(FALSE);
	global_df->col_chr  = 0L;
	global_df->col_win  = 0L;
	global_df->row_win2 = 0L;
	global_df->col_chr2  = -2L;
	global_df->col_win2 = -2L;
	tr = global_df->row_txt;
	if (isCAMode) {
		tr = ToNextRow(tr, FALSE);
		global_df->row_win2++;
	} else {
		while (!AtTopEnd(global_df->row_txt, global_df->head_text, FALSE)) {
			if (tr != global_df->row_txt && isRightBullet(global_df->row_txt->line)) {
				if (isNL_CFound(global_df->row_txt))
					isAddLineno = TRUE;
				else
					isAddLineno = FALSE;
				global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
				if (isAddLineno)
					global_df->lineno++;
				global_df->wLineno++;
				global_df->row_win++;
				global_df->row_win2--;
				break;
			}
			if (isSpeaker(global_df->row_txt->line[0]))
				break;
			if (global_df->row_win <= 0L)
				break;
			global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
			if (isNL_CFound(global_df->row_txt))
				global_df->lineno--;
			global_df->wLineno--;
			global_df->row_win--;
			global_df->row_win2++;
		}
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
		if (!AtBotEnd(tr, global_df->tail_text, FALSE)) {
			tr = ToNextRow(tr, FALSE);
			global_df->row_win2++;
		}
	}
	if (tr == global_df->row_txt) {
		if (strlen(tr->line) == 0/* || (tr->line[0] == '*' && tr->line[1] == ':')*/) {
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			global_df->LeaveHighliteOn = FALSE;
			res = FALSE;
		} else {
			global_df->col_chr2 = strlen(tr->line);
			global_df->col_win2 = ComColWin(FALSE, tr->line, global_df->col_chr2);
			global_df->LeaveHighliteOn = TRUE;
			res = TRUE;
		}
	} else {
		global_df->LeaveHighliteOn = TRUE;
		res = TRUE;
	}
	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt = global_df->head_row->next_char;
	return(res);
}

void selectNextSpeaker() {
	int  i;
	unCH s[5];
	char res, spFound, isNonMainSpeakerFound, isEndHFound;
	ROWS *tr;

//F5 mode function
	isNonMainSpeakerFound = FALSE;
	spFound = FALSE;
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	ChangeCurLineAlways(0);
	if (isSpeaker(*global_df->row_txt->line))
		spFound = TRUE;
	tr = global_df->row_txt;
	if (!FoundEndHTier(global_df->row_txt, FALSE) && !AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
		while (1) {
			MoveDown(1);
			if (isSpeaker(*global_df->row_txt->line))
				spFound = TRUE;
			if (isMainSpeaker(*global_df->row_txt->line)) {
				break;
			}
			if (F5Option == EVERY_LINE) {
				if (*global_df->row_txt->line == '%' || *global_df->row_txt->line == '@') {
					isNonMainSpeakerFound = TRUE;
				} else if (!isNonMainSpeakerFound)
					break;
			}
			if (FoundEndHTier(global_df->row_txt, FALSE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
				break;
		}
	}
	if (F5Option == EVERY_LINE && !isNonMainSpeakerFound && tr != global_df->row_txt) {
		if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
			findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
		res = SelectWholeTier(F5Option == EVERY_LINE);
	} else if (isMainSpeaker(*global_df->row_txt->line) && tr != global_df->row_txt) {
		if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
			findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
		res = SelectWholeTier(F5Option == EVERY_LINE);
	} else
		res = 0;
	if (!res) {
		EndOfLine(-1);
		i = 0;
		isEndHFound = FALSE;
		if (global_df->row_txt->line[0] != EOS && global_df->row_txt->line[0] != NL_C) {
			s[i++] = NL_C;
			if (FoundEndHTier(global_df->row_txt, FALSE)) {
				MoveUp(1);
				EndOfLine(-1);
				isEndHFound = TRUE;
			}
		}
		if (F5Option != EVERY_LINE || isEndHFound || FoundEndHTier(global_df->row_txt, FALSE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
			s[i++] = '*';
			s[i++] = ':';
		}
		s[i++] = '\t';
		s[i] = EOS;
		SaveUndoState(FALSE);
		AddText(s, EOS, 1, strlen(s));
		if (spFound || PlayingContMovie || PlayingContSound == '\004') {
			ChangeCurLineAlways(0);
			SelectWholeTier(F5Option == EVERY_LINE);
		} else {
			global_df->fake_row_win2 = 0L;
			global_df->fake_col_win2 = -2L;
		}
	}
}

char findStartMediaTag(char isMove, char isCAMode) {
	char fnd = FALSE, isNonMainSpeakerFound, isTopWinChanged;
	char tRedisplay;
	ROWS *st_row_txt, *tr;
	long old_col_win = global_df->col_win;
	long old_col_chr = global_df->col_chr;
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	ROWS *old_row_txt = global_df->row_txt;
	ROWS *old_top_win = global_df->top_win;

	ChangeCurLineAlways(0);
	isNonMainSpeakerFound = FALSE;
	if (isCAMode) {
		tr = global_df->row_txt;
		if (*tr->line == '%' || *tr->line == '@') {
			isNonMainSpeakerFound = TRUE;
		}
		while (!AtTopEnd(tr, global_df->head_text, FALSE)) {
			if (isMainSpeaker(*tr->line))
				break;
			if (*tr->line == '%' || *tr->line == '@') {
				isNonMainSpeakerFound = TRUE;
				break;
			}
			tr = ToPrevRow(tr, FALSE);
		}
	}
	if (isNonMainSpeakerFound || !isCAMode) {
		st_row_txt = NULL;
		while (1) {
			if (isMainSpeaker(*global_df->row_txt->line)) { 
				st_row_txt = global_df->row_txt;
				break;
			}
			if (AtTopEnd(global_df->row_txt, global_df->head_text, FALSE))
				break;
			MoveUp(1);
		}
		while (1) {
			if (st_row_txt) {
				if (FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER))) {
					if (!isMove) {
						DisplayTextWindow(NULL, 1);
						GetCurCode();
						FindRightCode(1);
					}
					return(TRUE);
				}
				fnd = '\002';
			}
			tr = global_df->row_txt;
			if (!isMove) {
				tRedisplay = global_df->redisplay;
				global_df->redisplay = FALSE;
				MoveDown(1);
				global_df->redisplay = tRedisplay;
			} else
				MoveDown(1);
			if (tr == global_df->row_txt) {
				fnd = '\003';
				EndOfLine(-1);
				break;
			}
			if (isMainSpeaker(*global_df->row_txt->line)) {
				if (st_row_txt != NULL)
					break;
				st_row_txt = global_df->row_txt;
			}
			if (FoundEndHTier(global_df->row_txt, FALSE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
				break;
		}
		if (isCAMode && st_row_txt != NULL) {
			while (1) {
				MoveUp(1);
				if (FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER))) {
					MoveDown(1);
					break;
				}
				if (isMainSpeaker(*global_df->row_txt->line)) { 
					break;
				}
				if (AtTopEnd(global_df->row_txt, global_df->head_text, FALSE))
					break;
			}
		}
	} else if (isCAMode) {
		if (FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER))) {
			if (!isMove) {
				DisplayTextWindow(NULL, 1);
				GetCurCode();
				FindRightCode(1);
			}
			return(TRUE);
		} else {
			if (isMove) {
				EndOfLine(-1);
				return(fnd);
			}
		}
	}
	if (!isMove) {
		isTopWinChanged = (global_df->top_win != old_top_win);
		global_df->row_txt = old_row_txt;
		global_df->top_win = old_top_win;
		global_df->row_win = old_row_win;
		global_df->col_win = old_col_win;
		global_df->col_chr = old_col_chr;
		global_df->lineno  = old_lineno;
		global_df->wLineno = old_wLineno;
		if (global_df->row_txt == global_df->cur_line) {
			long j;
			for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
				global_df->col_txt = global_df->col_txt->next_char;
		}
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		if (isTopWinChanged)
			DisplayTextWindow(NULL, 1);
//		wrefresh(global_df->w1);
	}
	return(fnd);
}

void findEndOfSpeakerTier(char goin, char isCAMode) {
	char isNonMainSpeakerFound;
	ROWS *tr;

	ChangeCurLineAlways(0);
	if (goin == '\003')
		return;
	isNonMainSpeakerFound = FALSE;
	if (isCAMode) {
		tr = global_df->row_txt;
		if (*tr->line == '%' || *tr->line == '@') {
			isNonMainSpeakerFound = TRUE;
		}
		while (!AtTopEnd(tr, global_df->head_text, FALSE)) {
			if (isMainSpeaker(*tr->line))
				break;
			if (*tr->line == '%' || *tr->line == '@') {
				isNonMainSpeakerFound = TRUE;
				break;
			}
			tr = ToPrevRow(tr, FALSE);
		}
	}
	if (isCAMode && isNonMainSpeakerFound) {
		if (!FoundEndHTier(global_df->row_txt, FALSE)) {
			while (1) {
				MoveDown(1);
				if (FoundEndHTier(global_df->row_txt, FALSE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
					break;
				}
				if (isSpeaker(*global_df->row_txt->line)) {
					MoveLeft(-1);
					break;
				}
			}
		}
	} else if (!isCAMode && (goin == '\002' || isMainSpeaker(*global_df->row_txt->line))) {
		while (1) {
			MoveUp(1);
			if (isMainSpeaker(*global_df->row_txt->line)) { 
				break;
			}
			if (AtTopEnd(global_df->row_txt, global_df->head_text, FALSE))
				break;
		}
		while (1) {
			MoveDown(1);
			if (isSpeaker(*global_df->row_txt->line)) {
				MoveLeft(-1);
				break;
			}
			if (FoundEndHTier(global_df->row_txt, FALSE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
				break;
		}
	} else {
		EndOfLine(-1);
	}
}

int QuickTrinscribeMedia(int i) {
	int  c;
	char type, res, isNonMainSpeakerFound;
	char isWhatType;
	ROWS *p;

#ifdef _MAC_CODE
	if (theMovie != NULL && theMovie->isPlaying && PBC.enable && PBC.walk) {
		stopMoviePlaying();
		return(89);
	}
#elif _WIN32
/* // NO QT
	if (MovDlg != NULL && !(MovDlg->qt)->IsQTMovieDone() && PBC.enable && PBC.walk) {
		stopMoviePlaying();
		return(89);
	}
*/
	if (MpegDlg != NULL && !(MpegDlg->mpeg)->IsMpegMovieDone() && PBC.enable && PBC.walk) {
		stopMoviePlaying();
		return(89);
	}
#endif
	ChangeCurLineAlways(0);
	if (!findMediaTiers()) {
		sprintf(global_df->err_message, "+Please add \"%s\" tier with media file name to headers section at the top of the file.", MEDIAHEADER);
		return(89);
	}
	type = 0;
	if (GetCurHidenCode(FALSE, NULL)) {
		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			type = 1;
		}
	}
	res = FALSE;
	isNonMainSpeakerFound = FALSE;
	p = global_df->row_txt;
	while (1) {
		if (isMainSpeaker(*p->line)) {
			res = TRUE;
			break;
		}
		if (AtTopEnd(p, global_df->head_text, FALSE))
			break;
		p = ToPrevRow(p, FALSE);
	}
	if (F5Option == EVERY_LINE && !isNonMainSpeakerFound) {
		if (res) {
			p = global_df->row_txt;
			for (i=0; p->line[i]; i++) {
				if (p->line[i] == HIDEN_C) {
					i++;
					if (is_unCH_digit(p->line[i])) {
						if (global_df->mediaFileName[0] != EOS) {
							strcpy(sp, SOUNDTIER);
							type = 1;
							break;
						} else {
							sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
							return(89);
						}
					} else {
						c = 0;
						for (; p->line[i] && p->line[i] != HIDEN_C && p->line[i] != ':'; i++) {
							if (c >= SPEAKERLEN)
								break;
							sp[c++] = p->line[i];
						}
						if (p->line[i] == ':') {
							sp[c] = EOS;
							if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
								type = 1;
								break;
							}
						}
					}
					for (; p->line[i] && p->line[i] != HIDEN_C; i++) ;
					if (p->line[i] == EOS)
						break;
				}
			}
		} else if (type != 0) {
			if (type == 1)
				type = 2; // sound
		}
	} else {
		if (isNonMainSpeakerFound) {
			p = global_df->row_txt;
			while (1) {
				if (isMainSpeaker(*p->line)) {
					break;
				}
				if (AtTopEnd(p, global_df->head_text, FALSE))
					break;
				p = ToPrevRow(p, FALSE);
			}
		}
		if (isMainSpeaker(*p->line) || type == 0) {
			while (1) {
				for (i=0; p->line[i]; i++) {
					if (p->line[i] == HIDEN_C) {
						i++;
						if (is_unCH_digit(p->line[i])) {
							if (global_df->mediaFileName[0] != EOS) {
								strcpy(sp, SOUNDTIER);
								type = 1;
								break;
							} else {
								sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
								return(89);
							}
						} else {
							c = 0;
							for (; p->line[i] && p->line[i] != HIDEN_C && p->line[i] != ':'; i++) {
								if (c >= SPEAKERLEN)
									break;
								sp[c++] = p->line[i];
							}
							if (p->line[i] == ':') {
								sp[c] = EOS;
								if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
									type = 1;
									break;
								}
							}
						}
						for (; p->line[i] && p->line[i] != HIDEN_C; i++) ;
						if (p->line[i] == EOS)
							break;
					}
				}
				if (AtBotEnd(p, global_df->tail_text, FALSE))
					break;
				p = ToNextRow(p, FALSE);
				if (isMainSpeaker(*p->line)) {
					if (!AtTopEnd(p, global_df->head_text, FALSE))
						p = ToPrevRow(p, FALSE);
					break;
				}
			}
		} else if (type != 0) {
			if (type == 1)
				type = 2; // sound
		}
	}

	if (type == 0) {
		while (1) {
			for (i=0; p->line[i]; i++) {
				if (p->line[i] == HIDEN_C) {
					i++;
					if (is_unCH_digit(p->line[i])) {
						if (global_df->mediaFileName[0] != EOS) {
							strcpy(sp, SOUNDTIER);
							if (global_df->SnTr.SoundFile[0] != EOS || global_df->MvTr.MovieFile[0] != EOS) {
								type = 3;
								break;
							}
						} else {
							sprintf(global_df->err_message, "+Missing %s tier with media file name in headers section at the top of the file", MEDIAHEADER);
							return(89);
						}
					} else {
						c = 0;
						for (; p->line[i] && p->line[i] != HIDEN_C && p->line[i] != ':'; i++) {
							if (c >= SPEAKERLEN)
								break;
							sp[c++] = p->line[i];
						}
						if (p->line[i] == ':') {
							sp[c] = EOS;
							if ((uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) && 
								  (global_df->SnTr.SoundFile[0] != EOS || global_df->MvTr.MovieFile[0] != EOS)) {
								type = 3;
								break;
							}
						}
					}
					for (; p->line[i] && p->line[i] != HIDEN_C; i++) ;
					if (p->line[i] == EOS)
						break;
				}
			}
			if (type != 0)
				break;
			if (AtTopEnd(p, global_df->head_text, FALSE))
				break;
			p = ToPrevRow(p, FALSE);
		}
	}
	if (type == 1) { // sound
		if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) == TRUE) {
			if (FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER)) && (FindSndInfo(&isWhatType) > 0)) {
				if (isWhatType == isVideo) {
					PlayingContMovie = '\003';
					SelectWholeTier(F5Option == EVERY_LINE);
					i = PlayMovie(&global_df->MvTr, global_df, FALSE);
				} else if (isWhatType == isAudio) {
					PlayingContSound = '\004';
					global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
					strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
					draw_mid_wm();
					strcpy(global_df->err_message, DASHES);
					SelectWholeTier(F5Option == EVERY_LINE);
					DrawSoundCursor(1);
					DrawCursor(1);
					global_df->SnTr.SSource = 1;
					if (PlaySound(&global_df->SnTr, (int)'r'))
						return(89);
					PlayingContSound = FALSE;
					global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
				}
			} else {
				return(89);
			}
		} else {
			findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
			type = 0;
		}
	} else if (type == 2) { // sound
		if (FindSndInfo(&isWhatType) > 0) {
			MoveDown(1);
			if (isWhatType == isVideo) {
				PlayingContMovie = '\003';
				DrawCursor(1);
				i = PlayMovie(&global_df->MvTr, global_df, FALSE);
			} else if (isWhatType == isAudio) {
				PlayingContSound = '\004';
				global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
				strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
				draw_mid_wm();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				strcpy(global_df->err_message, DASHES);
				DrawSoundCursor(1);
				DrawCursor(1);
				global_df->SnTr.SSource = 1;
				if (PlaySound(&global_df->SnTr, (int)'r'))
					return(89);
				PlayingContSound = FALSE;
				global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
			}
		} else {
			findEndOfSpeakerTier(FALSE, F5Option == EVERY_LINE);
			type = 0;
		}
	} else if (type == 3) {
		if (global_df->SnTr.SoundFile[0] != EOS)
			type = isAudio;
#ifdef _MAC_CODE
		else if (MovieReady && theMovie != NULL && theMovie->df == global_df)
			type = isVideo;
#elif _WIN32
		else if (MpegDlg != NULL)
			type = isVideo;
#endif
		else
			type = 0;
		if (type == isVideo) {
			PlayingContMovie = '\003';
			if (global_df->MvTr.MEnd != 1 && global_df->MvTr.MEnd != 0)
				global_df->MvTr.MBeg = global_df->MvTr.MEnd;
			if (global_df->MvTr.MBeg == 1)
				global_df->MvTr.MBeg = 0;
			global_df->MvTr.MEnd = 1;
			SelectWholeTier(F5Option == EVERY_LINE);
			i = PlayMovie(&global_df->MvTr, global_df, FALSE);
		} else if (type == isAudio) {
			PlayingContSound = '\004';
			if (global_df->SnTr.SSource != 2) {
				if (global_df->SnTr.EndF != global_df->SnTr.SoundFileSize)
					global_df->SnTr.BegF = AlignMultibyteMediaStream(global_df->SnTr.EndF, '-');
			}
			if (global_df->SnTr.BegF == global_df->SnTr.SoundFileSize)
				global_df->SnTr.BegF = 0L;
			global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
			SetPBCglobal_df(false, 0L);
			strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
			draw_mid_wm();
			strcpy(global_df->err_message, DASHES);
			SelectWholeTier(F5Option == EVERY_LINE);
			DrawSoundCursor(1);
			DrawCursor(1);
//			global_df->SnTr.SSource = 1;
			if (PlaySound(&global_df->SnTr, (int)'r'))
				return(89);
			PlayingContSound = FALSE;
			global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
		}
	}

	if (type == 0) {
		if (global_df->mediaFileName[0] == EOS) {
			sprintf(global_df->err_message, "+Please add \"%s\" tier with media file name to headers section at the top of the file.", MEDIAHEADER);
			return(89);
		} else {
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
				if (global_df->SnTr.mp3.hSys7SoundData)
					DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
				global_df->SnTr.mp3.theSoundMedia = NULL;
				global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
				global_df->SnTr.SoundFile[0] = EOS;
				global_df->SnTr.SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
			}
			if (isPlayAudioFirst) {
				strcpy(global_df->SnTr.SoundFile, global_df->mediaFileName);
				if (!mOpenSoundFile()) {
					strcpy(global_df->MvTr.MovieFile, global_df->mediaFileName);
					if (mOpenMovieFile(&global_df->MvTr) != 0) {
						if (global_df->SnTr.errMess != NULL) {
							do_warning(global_df->SnTr.errMess, 0);
							global_df->err_message[0] = EOS;
							global_df->SnTr.errMess = NULL;
						} else
							CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, FALSE);
						type = 0;
					} else
						type = isVideo;
				} else
					type = isAudio;
			} else {
				strcpy(global_df->MvTr.MovieFile, global_df->mediaFileName);
				if (mOpenMovieFile(&global_df->MvTr) != 0) {
					strcpy(global_df->SnTr.SoundFile, global_df->mediaFileName);
					if (!mOpenSoundFile()) {
						if (global_df->SnTr.errMess != NULL) {
							do_warning(global_df->SnTr.errMess, 0);
							global_df->err_message[0] = EOS;
							global_df->SnTr.errMess = NULL;
						} else
							CantFindMedia(global_df->err_message, global_df->MvTr.MovieFile, FALSE);
						type = 0;
					} else
						type = isAudio;
				} else
					type = isVideo;
			}
		}

		if (type != 0) {
#ifdef _WIN32
			extern char quickTranscribe;

			if (GlobalDoc) {
				quickTranscribe = type;
				GlobalDoc->UpdateAllViews(NULL, 0L, NULL);
			}
#endif // _WIN32
#ifdef _MAC_CODE
			char isSpeakerFound;

			if (type == isAudio || type == isVideo) { // sound or movie
				if (type == isAudio) {
					PlayingContSound = '\004';
					global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
					strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
					draw_mid_wm();
					strcpy(global_df->err_message, DASHES);
				} else
					PlayingContMovie = '\003';

				if (isMainSpeaker(*global_df->row_txt->line))
					isSpeakerFound = TRUE;
				else
					isSpeakerFound = FALSE;
				if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
					findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
				if (global_df->row_txt->line[0] == '\t' && (global_df->row_txt->line[1] == EOS || global_df->row_txt->line[1] == NL_C)) { 
					SaveUndoState(FALSE);
					global_df->redisplay = 0;
					DeletePrevChar(1);
					global_df->redisplay = 1;
				}
				if (!isSpeakerFound) {
					if (!isMainSpeaker(*global_df->row_txt->line) && (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE) || FoundEndHTier(global_df->row_txt, FALSE))) {
						if (global_df->row_txt->line[0] != EOS && global_df->row_txt->line[0] != NL_C) { 
							SaveUndoState(FALSE);
							if (FoundEndHTier(global_df->row_txt, FALSE)) {
								MoveUp(1);
							}
							EndOfLine(-1);
							NewLine(-1);
						}
					}
					if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C) { 
						SaveUndoState(FALSE);
						AddText(cl_T("*:\t"), EOS, 1, strlen("*:\t"));
					}
				} else if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C) { 
					MoveUp(1);
				}
				SelectWholeTier(F5Option == EVERY_LINE);
				if (type == isAudio) {
					DrawSoundCursor(1);
					DrawCursor(1);
					global_df->SnTr.SSource = 1;
					if (PlaySound(&global_df->SnTr, (int)'r'))
						return(89);
					PlayingContSound = FALSE;
					global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
				} else
					i = PlayMovie(&global_df->MvTr, global_df, FALSE);
			} else if (type == isPict) { // picture
				DisplayPhoto(global_df->PcTr.pictFName);
			} else if (type == isText) { // text
				DisplayText(global_df->TxTr.textFName);
			} else {
				strcpy(global_df->err_message, "+Unsupported media type.");
			}
#endif // _MAC_CODE
		}
	}
	return(89);
}

char RSoundPlay(int c, char isPreserveTimes) {
	int  i;
	unCH tFNs[FILENAME_MAX], tFNm[FILENAME_MAX];
	long tBFs, tBFm, tEFs, tEFm;
	char isWhatType;

	if (isPreserveTimes)
		isDisplayWave = 0;
	i = (int)global_df->DataChanged;
	if (c >= 0) {
		if (!FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
			strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
			if (isPreserveTimes)
				isDisplayWave = 1;
			return(0);
		} else
			ChangeCurLine();
	} else
		ChangeCurLine();
	if (!i)
		global_df->DataChanged = '\0';

	strcpy(tFNs, global_df->SnTr.SoundFile);
	tBFs = global_df->SnTr.BegF;
	tEFs = global_df->SnTr.EndF;
	strcpy(tFNm, global_df->MvTr.MovieFile);
	tBFm = global_df->MvTr.MBeg;
	tEFm = global_df->MvTr.MEnd;

	isWhatType = 0;
	if (FindSndInfo(&isWhatType) <= 0) {
		if (isPreserveTimes) {
			isDisplayWave = 1;
			global_df->SnTr.BegF = tBFs;
			global_df->SnTr.EndF = tEFs;
			global_df->MvTr.MBeg = tBFm;
			global_df->MvTr.MEnd = tEFm;
		}
		return(0);
	}
	if (c == -2) {
		if (isPreserveTimes) {
			isDisplayWave = 1;
			global_df->SnTr.BegF = tBFs;
			global_df->SnTr.EndF = tEFs;
			global_df->MvTr.MBeg = tBFm;
			global_df->MvTr.MEnd = tEFm;
		}
		return(isWhatType);
	}

	if (isWhatType == isVideo) {
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
		if (global_df->LastCommand == 49 && !strcmp(tFNm, global_df->MvTr.MovieFile) &&
			tBFm == global_df->MvTr.MBeg && tEFm == global_df->MvTr.MEnd &&
			gCurF >= global_df->MvTr.MBeg && gCurF < global_df->MvTr.MEnd-100 && PBC.enable)
			global_df->MvTr.MBeg = gCurF;
		i = PlayMovie(&global_df->MvTr, global_df, FALSE);
		if (isPreserveTimes) {
			isDisplayWave = 1;
			global_df->SnTr.BegF = tBFs;
			global_df->SnTr.EndF = tEFs;
			global_df->MvTr.MBeg = tBFm;
			global_df->MvTr.MEnd = tEFm;
		}
		return(isWhatType);
	} else {
		if (global_df->LastCommand == 49 && !strcmp(tFNs, global_df->SnTr.SoundFile) &&
			tBFs == global_df->SnTr.BegF && tEFs == global_df->SnTr.EndF &&
			gCurF >= global_df->SnTr.BegF && gCurF < global_df->SnTr.EndF && PBC.enable)
			global_df->SnTr.BegF = gCurF;
		if (c >= 0) 
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
//		strcpy(global_df->err_message, DASHES);
		if (PlaySound(&global_df->SnTr, (int)'r')) {
			if (isPreserveTimes) {
				isDisplayWave = 1;
				global_df->SnTr.BegF = tBFs;
				global_df->SnTr.EndF = tEFs;
				global_df->MvTr.MBeg = tBFm;
				global_df->MvTr.MEnd = tEFm;
			}
			if (syncAudioAndVideo(global_df->MvTr.MovieFile))
				isWhatType = isVideo;
			return(isWhatType);
		}
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
		DrawSoundCursor(1);
	}
	if (isPreserveTimes) {
		isDisplayWave = 1;
		global_df->SnTr.BegF = tBFs;
		global_df->SnTr.EndF = tEFs;
		global_df->MvTr.MBeg = tBFm;
		global_df->MvTr.MEnd = tEFm;
	}
	return(isWhatType);
}

void ExecSoundCom(int c) {
	char isTopWinChanged;
	long old_col_win;
	long old_col_chr;
	long old_row_win;
	long old_col_win2;
	long old_col_chr2;
	long old_row_win2;
	long old_lineno;
	long old_wLineno;
	long old_LeftCol;
	ROWS *old_row_txt;
	ROWS *old_top_win;

	if (global_df != NULL) {
		old_col_win = global_df->col_win;
		old_col_chr = global_df->col_chr;
		old_row_win = global_df->row_win;
		old_col_win2 = global_df->col_win2;
		old_col_chr2 = global_df->col_chr2;
		old_row_win2 = global_df->row_win2;
		old_lineno  = global_df->lineno;
		old_wLineno = global_df->wLineno;
		old_LeftCol = global_df->LeftCol;
		old_row_txt = global_df->row_txt;
		old_top_win = global_df->top_win;
	} else {
		old_col_win = NULL;
		old_col_chr = NULL;
		old_row_win = NULL;
		old_col_win2 = NULL;
		old_col_chr2 = NULL;
		old_row_win2 = NULL;
		old_lineno  = NULL;
		old_wLineno = NULL;
		old_LeftCol = NULL;
		old_row_txt = NULL;
		old_top_win = NULL;
	}
	DrawCursor(0);
	findMediaTiers();
	if (c == -2 || c == -3) { // SOUND ONLY
		RemoveLastUndo();
#if defined(_WIN32)
		if (syncAudioAndVideo(global_df->MvTr.MovieFile) && global_df->SnTr.dtype != 0) {
		} else {
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			draw_mid_wm();
		}
#else
		strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
		draw_mid_wm();
#endif
#if defined(_MAC_CODE) || defined(_WIN32)
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
#else
		wmove(global_df->w1, (int)global_df->row_win, (int)(global_df->col_win-global_df->LeftCol));
#endif
		wrefresh(global_df->w1);
//		strcpy(global_df->err_message, DASHES);
		global_df->SnTr.SSource = 2;
		if (c == -2)
			PlaySound(&global_df->SnTr, (int)'p');
		else
			PlaySound(&global_df->SnTr, (int)'r');
	} else if (c == -4 || c == -6 || c == -7) {
		int i;
		char tType = 0;
		myFInfo *saveGlobal_df;

		saveGlobal_df = global_df;
		if (!saveGlobal_df->EditorMode) {
			SaveUndoState(FALSE);
		}

		i = (int)global_df->DataChanged;
		FindAnyBulletsOnLine();
		if (!i)
			global_df->DataChanged = '\0';
		if (!GetCurHidenCode(FALSE, NULL))
			GetCurCode();
		if (uS.mStricmp(sp, SOUNDTIER) == 0 || uS.mStricmp(sp, REMOVEMOVIETAG) == 0) {
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			global_df->SnTr.SSource = 1;
			tType = RSoundPlay((c == -6), (c == -7));
		} else if (uS.mStricmp(sp, PICTTIER) == 0) {
			tType = isPict;
			ShowPicture(c == -6);
		} else if (uS.mStricmp(sp, TEXTTIER) == 0) {
			tType = isText;
			ShowTextFile(c == -6);
		} else
			strcpy(global_df->err_message, "+Can't find any bullets under a cursor. Please point to a bullet associated with movie, sound or picture and try again.");

		if (!saveGlobal_df->EditorMode) {
			if (global_df == NULL) {
				global_df = saveGlobal_df;
				if (tType == isVideo)
					DrawFakeHilight(0);
				Undo(-1);
				if (tType == isVideo)
					DrawFakeHilight(1);
				global_df = NULL;
			} else
				Undo(-1);
		}
/* 2012-03-08
		if (global_df == NULL && saveGlobal_df != NULL) {
			global_df = saveGlobal_df;
			isTopWinChanged = (global_df->top_win != old_top_win);
			global_df->col_win = old_col_win;
			global_df->col_chr = old_col_chr;
			global_df->row_win = old_row_win;
			global_df->col_win2 = old_col_win2;
			global_df->col_chr2 = old_col_chr2;
			global_df->row_win2 = old_row_win2;
			global_df->lineno  = old_lineno;
			global_df->wLineno = old_wLineno;
			global_df->LeftCol = old_LeftCol;
			global_df->row_txt = old_row_txt;
			global_df->top_win = old_top_win;
			if (global_df->row_txt == global_df->cur_line) {
				long j;
				for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
					global_df->col_txt = global_df->col_txt->next_char;
			}
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			if (isTopWinChanged)
				DisplayTextWindow(NULL, 1);
			global_df = NULL;
		}
*/ 
	}
	if (global_df != NULL) {
		isTopWinChanged = (global_df->top_win != old_top_win);
		global_df->col_win = old_col_win;
		global_df->col_chr = old_col_chr;
		global_df->row_win = old_row_win;
		global_df->col_win2 = old_col_win2;
		global_df->col_chr2 = old_col_chr2;
		global_df->row_win2 = old_row_win2;
		global_df->lineno  = old_lineno;
		global_df->wLineno = old_wLineno;
		global_df->LeftCol = old_LeftCol;
		global_df->row_txt = old_row_txt;
		global_df->top_win = old_top_win;
		if (global_df->row_txt == global_df->cur_line) {
			long j;
			for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
				global_df->col_txt = global_df->col_txt->next_char;
		}
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		if (isTopWinChanged)
			DisplayTextWindow(NULL, 1);
	}
	DrawCursor(1);
}

char issoundwin() {
	if (!global_df->SnTr.IsSoundOn)
		return(FALSE);
	touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
	return(TRUE);
}

int PlayMedia(int c) {
	int  i;
	char isTopWinChanged;
	long old_col_win = global_df->col_win;
	long old_col_chr = global_df->col_chr;
	long old_row_win = global_df->row_win;
	long old_col_win2 = global_df->col_win2;
	long old_col_chr2 = global_df->col_chr2;
	long old_row_win2 = global_df->row_win2;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	long old_LeftCol = global_df->LeftCol;
	ROWS *old_row_txt = global_df->row_txt;
	ROWS *old_top_win = global_df->top_win;
	Boolean done;

	DrawCursor(0);
	i = (int)global_df->DataChanged;
	done = false;
	do {
		FindAnyBulletsOnLine();
		if (global_df->row_txt == global_df->cur_line)
			done = (global_df->col_txt->c == HIDEN_C);
		else
			done = (global_df->row_txt->line[global_df->col_chr] == HIDEN_C);
		if (!done) {
			if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
				isTopWinChanged = (global_df->top_win != old_top_win);
				global_df->col_win = old_col_win;
				global_df->col_chr = old_col_chr;
				global_df->row_win = old_row_win;
				global_df->col_win2 = old_col_win2;
				global_df->col_chr2 = old_col_chr2;
				global_df->row_win2 = old_row_win2;
				global_df->lineno  = old_lineno;
				global_df->wLineno = old_wLineno;
				global_df->LeftCol = old_LeftCol;
				global_df->row_txt = old_row_txt;
				global_df->top_win = old_top_win;
				if (global_df->row_txt == global_df->cur_line) {
					long j;
					for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				if (isTopWinChanged)
					DisplayTextWindow(NULL, 1);
				done = true;
			} else {
				MoveDown(-1);
				if (global_df->row_txt == global_df->cur_line) {
					if (global_df->head_row->next_char != global_df->tail_row) {
						if (isSpeaker(global_df->head_row->next_char->c)) {
							isTopWinChanged = (global_df->top_win != old_top_win);
							global_df->col_win = old_col_win;
							global_df->col_chr = old_col_chr;
							global_df->row_win = old_row_win;
							global_df->col_win2 = old_col_win2;
							global_df->col_chr2 = old_col_chr2;
							global_df->row_win2 = old_row_win2;
							global_df->lineno  = old_lineno;
							global_df->wLineno = old_wLineno;
							global_df->LeftCol = old_LeftCol;
							global_df->row_txt = old_row_txt;
							global_df->top_win = old_top_win;
							if (global_df->row_txt == global_df->cur_line) {
								long j;
								for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
									global_df->col_txt = global_df->col_txt->next_char;
							}
							wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
							if (isTopWinChanged)
								DisplayTextWindow(NULL, 1);
							done = true;
						}
					}
				} else {
					if (isSpeaker(global_df->row_txt->line[0])) {
						isTopWinChanged = (global_df->top_win != old_top_win);
						global_df->col_win = old_col_win;
						global_df->col_chr = old_col_chr;
						global_df->row_win = old_row_win;
						global_df->col_win2 = old_col_win2;
						global_df->col_chr2 = old_col_chr2;
						global_df->row_win2 = old_row_win2;
						global_df->lineno  = old_lineno;
						global_df->wLineno = old_wLineno;
						global_df->LeftCol = old_LeftCol;
						global_df->row_txt = old_row_txt;
						global_df->top_win = old_top_win;
						if (global_df->row_txt == global_df->cur_line) {
							long j;
							for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
								global_df->col_txt = global_df->col_txt->next_char;
						}
						wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
						if (isTopWinChanged)
							DisplayTextWindow(NULL, 1);
						done = true;
					}
				}
			}
		}
	} while (!done) ;
	ChangeCurLine();
	if (!i)
		global_df->DataChanged = '\0';
	if (!GetCurHidenCode(FALSE, NULL))
		GetCurCode();

	if (uS.mStricmp(sp, SOUNDTIER) == 0 || uS.mStricmp(sp, REMOVEMOVIETAG) == 0) {
		strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
		global_df->SnTr.SSource = 1;
 		global_df->LastCommand = 49;
		RSoundPlay(!global_df->EditorMode, (global_df->SoundWin != NULL));
	} else if (uS.mStricmp(sp, PICTTIER) == 0) {
		global_df->LastCommand = 49;
		ShowPicture(!global_df->EditorMode);
	} else if (uS.mStricmp(sp, TEXTTIER) == 0) {
		global_df->LastCommand = 49;
		ShowTextFile(!global_df->EditorMode);
	} else if (global_df->SnTr.IsSoundOn) {
		if (global_df->SnTr.BegF > global_df->SnTr.EndF || global_df->SnTr.BegF == global_df->SnTr.EndF) {
			strcpy(global_df->err_message, "-Playing all, press any KEY or click mouse to stop");
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
			global_df->SnTr.SSource = 2;
			PlaySound(&global_df->SnTr, (int)'p');
		} else {
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
			global_df->SnTr.SSource = 2;
			PlaySound(&global_df->SnTr, (int)'r');
		}
	} else {
		strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
/*
		if ((ret=getNewMediaFile(TRUE, isAllType))) {
			if (ret == isAudio) { // sound
				global_df->SnTr.BegF = 0L;
				global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
				global_df->SnTr.SSource = 1;
				SetPBCglobal_df(false, 0L);
				global_df->LastCommand = 49;
				PlaySound(&global_df->SnTr, (int)'r');
			} else if (ret == isVideo) { // movie
				global_df->MvTr.MBeg = 0L;
				global_df->MvTr.MEnd = 0L;
				global_df->LastCommand = 49;
				i = PlayMovie(&global_df->MvTr, global_df, FALSE);
			} else if (ret == isPict) { // picture
				global_df->LastCommand = 49;
				DisplayPhoto(global_df->PcTr.pictFName);
			} else if (ret == isText) { // text
				global_df->LastCommand = 49;
				DisplayText(global_df->TxTr.textFName);
			} else {
				strcpy(global_df->err_message, "+Unsupported media type.");
			}
		}
*/
	}
	DrawCursor(1);
	return(49);
}


int PlayStepForward(int c) {
	int  i;
	char samePoint, ret, type, isTopWinChanged;
	unCH sfn[FILENAME_MAX], mfn[FILENAME_MAX];
	char isWhatType;
	long sBegF, sEndF, mBegF, mEndF;
	long t;
	long old_col_win = global_df->col_win;
	long old_col_chr = global_df->col_chr;
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	long old_LeftCol = global_df->LeftCol;
	ROWS *old_row_txt = global_df->row_txt;
	ROWS *old_top_win = global_df->top_win;

	if (!PBC.enable) {
		return(PlayMedia(c));
	}

#ifdef _MAC_CODE
	if (theMovie != NULL && theMovie->isPlaying && PBC.enable && PBC.walk) {
		replayMovieWithOffset(1);
		return(81);
	}
#elif _WIN32
	if (MpegDlg != NULL && !(MpegDlg->mpeg)->IsMpegMovieDone() && PBC.enable && PBC.walk) {
		replayMovieWithOffset(1);
		return(81);
	}
#endif
	PBC.isPC = 1;
	i = (int)global_df->DataChanged;
	ChangeCurLine();
	if (!i)
		global_df->DataChanged = '\0';

	if (global_df->SnTr.SoundFile[0] != EOS)
		type = isAudio;
#ifdef _MAC_CODE
	else if (MovieReady && theMovie != NULL && theMovie->df == global_df)
		type = isVideo;
#elif _WIN32
/* // NO QT
	else if (MovDlg != NULL)
		type = isVideo;
*/
	else if (MpegDlg != NULL)
		type = isVideo;
#endif
	else
		type = 0;

	samePoint = (type != 0);

	if (samePoint) {
		if (!GetCurHidenCode(FALSE, NULL))
			GetCurCode();
		if (uS.mStricmp(sp, SOUNDTIER) == 0 || uS.mStricmp(sp, REMOVEMOVIETAG) == 0) {
			strcpy(sfn, global_df->SnTr.SoundFile);
			sBegF = global_df->SnTr.BegF;
			sEndF = global_df->SnTr.EndF;

			strcpy(mfn, global_df->MvTr.MovieFile);
			mBegF = global_df->MvTr.MBeg;
			mEndF = global_df->MvTr.MEnd;

			ret = FindSndInfo(&isWhatType);
			if (ret > 0) {
				type = isWhatType;
				if (isWhatType == isAudio) {
					if (uS.mStricmp(sFN,global_df->SnTr.SoundFile) || sBF!=global_df->SnTr.BegF || sEF!=global_df->SnTr.EndF)
						samePoint = FALSE;
				} else if (isWhatType == isVideo) {
					if (uS.mStricmp(sFN,global_df->MvTr.MovieFile) || sBF!=global_df->MvTr.MBeg || sEF!=global_df->MvTr.MEnd)
						samePoint = FALSE;
				}
			} else if (ret == -2)
				samePoint = FALSE;

			if (ret != 2 && type == isAudio) {
				strcpy(global_df->SnTr.SoundFile, sfn);
				global_df->SnTr.BegF = sBegF;
				global_df->SnTr.EndF = sEndF;
				SetPBCglobal_df(false, 0L);
			} else if ((ret == -2 || samePoint) && type == isVideo) {
				strcpy(global_df->MvTr.MovieFile, mfn);
				global_df->MvTr.MBeg = mBegF;
				global_df->MvTr.MEnd = mEndF;
				SetPBCglobal_df(false, 0L);
			}
		}
	}

	if (samePoint) {
		if (type == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
			type = isVideo;
			global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
			global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
		}
		if (type == isAudio) {
			DrawSoundCursor(0);
			if (global_df->SnTr.BegF >= global_df->SnTr.EndF) {
				global_df->SnTr.EndF = global_df->SnTr.BegF + AlignMultibyteMediaStream(conv_from_msec_rep(PBC.step_length), '+');
			} else if (PBC.backspace > PBC.step_length) {
				t = conv_to_msec_rep(global_df->SnTr.BegF) + (PBC.backspace - PBC.step_length);
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				t = conv_to_msec_rep(global_df->SnTr.EndF) + (PBC.backspace - PBC.step_length);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			} else {
				t = conv_to_msec_rep(global_df->SnTr.BegF) + (PBC.step_length - PBC.backspace);
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				t = conv_to_msec_rep(global_df->SnTr.EndF) + (PBC.step_length - PBC.backspace);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			}
			t = conv_from_msec_rep(PBC.step_length);
			if (global_df->SnTr.EndF - global_df->SnTr.BegF < t) {
				global_df->SnTr.EndF = global_df->SnTr.BegF + AlignMultibyteMediaStream(t, '+');
			}
			SetPBCglobal_df(false, 0L);
			DisplayEndF(TRUE);
			DrawSoundCursor(3);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			global_df->SnTr.SSource = 1;
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
//			strcpy(global_df->err_message, DASHES);
			PlaySound(&global_df->SnTr, (int)'r');
		} else if (type == isVideo) {
			if (global_df->MvTr.MBeg >= global_df->MvTr.MEnd) {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
			} else if (PBC.backspace > PBC.step_length) {
				global_df->MvTr.MBeg = global_df->MvTr.MBeg + (PBC.backspace - PBC.step_length);
				global_df->MvTr.MEnd = global_df->MvTr.MEnd + (PBC.backspace - PBC.step_length);
			} else {
				global_df->MvTr.MBeg = global_df->MvTr.MBeg + (PBC.step_length - PBC.backspace);
				global_df->MvTr.MEnd = global_df->MvTr.MEnd + (PBC.step_length - PBC.backspace);
			}
			if (global_df->MvTr.MEnd - global_df->MvTr.MBeg < PBC.step_length) {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
			}
			SetPBCglobal_df(false, 0L);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
			PlayMovie(&global_df->MvTr,global_df, FALSE);
		}
	} else {
		if (global_df->SnTr.SoundFile[0] != EOS && global_df->SnTr.IsSoundOn && !syncAudioAndVideo(global_df->MvTr.MovieFile)) {
			strcpy(sFN, global_df->SnTr.SoundFile);
			sBF = global_df->SnTr.BegF;
			sEF = global_df->SnTr.EndF;
			DrawSoundCursor(0);
			global_df->SnTr.EndF = AlignMultibyteMediaStream(global_df->SnTr.BegF+conv_from_msec_rep(PBC.step_length), '+');
			DisplayEndF(TRUE);
			DrawSoundCursor(3);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			global_df->SnTr.SSource = 1;
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
//			strcpy(global_df->err_message, DASHES);
			PlaySound(&global_df->SnTr, (int)'r');
		} else if (findStartMediaTag(FALSE, F5Option == EVERY_LINE) == TRUE) {
			if (!FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
				strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
			} else {
				if (FindSndInfo(&isWhatType) > 0) {
					isTopWinChanged = (global_df->top_win != old_top_win);
					global_df->row_txt = old_row_txt;
					global_df->top_win = old_top_win;
					global_df->row_win = old_row_win;
					global_df->col_win = old_col_win;
					global_df->col_chr = old_col_chr;
					global_df->lineno  = old_lineno;
					global_df->wLineno = old_wLineno;
					global_df->LeftCol = old_LeftCol;
					if (global_df->row_txt == global_df->cur_line) {
						long j;
						for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					if (isTopWinChanged)
						DisplayTextWindow(NULL, 1);

					if (isWhatType == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
						isWhatType = isVideo;
						global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
						global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
					}
					if (isWhatType == isAudio) {
						strcpy(sFN, global_df->SnTr.SoundFile);
						sBF = global_df->SnTr.BegF;
						sEF = global_df->SnTr.EndF;
						DrawSoundCursor(0);
						global_df->SnTr.EndF = AlignMultibyteMediaStream(global_df->SnTr.BegF+conv_from_msec_rep(PBC.step_length), '+');
						DisplayEndF(TRUE);
						DrawSoundCursor(3);
						global_df->SnTr.SSource = 1;
					} else {
						strcpy(sFN, global_df->MvTr.MovieFile);
						sBF = global_df->MvTr.MBeg;
						sEF = global_df->MvTr.MEnd;
						global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
					}
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					if (isWhatType == isAudio)
						PlaySound(&global_df->SnTr, (int)'r');
					else
						PlayMovie(&global_df->MvTr,global_df, FALSE);
					type = isWhatType;
				}
			}
		} else {
			strcpy(global_df->err_message, "+Please choose media file in Walker Controller with \"Open Media\" button.");
			return(81);

/*
			if ((ret=getNewMediaFile(TRUE, isAllType))) {
				if (ret == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
					ret = isVideo;
					global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
				}
				if (ret == isAudio) { // sound
					strcpy(sFN, global_df->SnTr.SoundFile);
					sBF = global_df->SnTr.BegF;
					sEF = global_df->SnTr.EndF;
					DrawSoundCursor(0);
					t = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
					DisplayEndF(TRUE);
					DrawSoundCursor(3);
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					global_df->SnTr.SSource = 1;
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					PlaySound(&global_df->SnTr, (int)'r');
					type = isAudio;
				} else if (ret == isVideo) { // movie
					strcpy(sFN, global_df->MvTr.MovieFile);
					sBF = global_df->MvTr.MBeg;
					sEF = global_df->MvTr.MEnd;
					global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
					PlayMovie(&global_df->MvTr,global_df, FALSE);
					type = isVideo;
				} else {
					strcpy(global_df->err_message, "+Unsupported media type.");
				}
			}
*/
		}
	}
#ifndef _WIN32
	if (type != 2)
		PBC.isPC = 0;
#endif
	return(81);
}

int PlayStepBackward(int c) {
	int  i;
	char samePoint, ret, type, isTopWinChanged;
	unCH sfn[FILENAME_MAX], mfn[FILENAME_MAX];
	char isWhatType;
	long sBegF, sEndF, mBegF, mEndF;
	long t;
	long old_col_win = global_df->col_win;
	long old_col_chr = global_df->col_chr;
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	long old_LeftCol = global_df->LeftCol;
	ROWS *old_row_txt = global_df->row_txt;
	ROWS *old_top_win = global_df->top_win;

	if (!PBC.enable) {
// 18-07-2008		return(SelectWordColor());
		strcpy(global_df->err_message, "+Please enable Walker Controller first.");
		return(83);
	}

#ifdef _MAC_CODE
	if (theMovie != NULL && theMovie->isPlaying && PBC.enable && PBC.walk) {
		replayMovieWithOffset(-1);
		return(83);
	}
#elif _WIN32
	if (MpegDlg != NULL && !(MpegDlg->mpeg)->IsMpegMovieDone() && PBC.enable && PBC.walk) {
		replayMovieWithOffset(-1);
		return(83);
	}
#endif
	PBC.isPC = 1;
	i = (int)global_df->DataChanged;
	ChangeCurLine();
	if (!i)
		global_df->DataChanged = '\0';

	if (global_df->SnTr.SoundFile[0] != EOS)
		type = isAudio;
#ifdef _MAC_CODE
	else if (MovieReady && theMovie != NULL && theMovie->df == global_df)
		type = isVideo;
#elif _WIN32
/* // NO QT
	else if (MovDlg != NULL)
		type = isVideo;
*/
	else if (MpegDlg != NULL)
		type = isVideo;
#endif
	else
		type = 0;

	samePoint = (type != 0);

	if (samePoint) {
		if (!GetCurHidenCode(FALSE, NULL))
			GetCurCode();
		if (uS.mStricmp(sp, SOUNDTIER) == 0 || uS.mStricmp(sp, REMOVEMOVIETAG) == 0) {
			strcpy(sfn, global_df->SnTr.SoundFile);
			sBegF = global_df->SnTr.BegF;
			sEndF = global_df->SnTr.EndF;

			strcpy(mfn, global_df->MvTr.MovieFile);
			mBegF = global_df->MvTr.MBeg;
			mEndF = global_df->MvTr.MEnd;

			ret = FindSndInfo(&isWhatType);
			if (ret > 0) {
				type = isWhatType;
				if (isWhatType == isAudio) {
					if (uS.mStricmp(sFN,global_df->SnTr.SoundFile) || sBF!=global_df->SnTr.BegF || sEF!=global_df->SnTr.EndF)
						samePoint = FALSE;
				} else if (isWhatType == isVideo) {
					if (uS.mStricmp(sFN,global_df->MvTr.MovieFile) || sBF!=global_df->MvTr.MBeg || sEF!=global_df->MvTr.MEnd)
						samePoint = FALSE;
				}
			} else if (ret == -2)
				samePoint = FALSE;

			if (ret != 2 && type == isAudio) {
				strcpy(global_df->SnTr.SoundFile, sfn);
				global_df->SnTr.BegF = sBegF;
				global_df->SnTr.EndF = sEndF;
				SetPBCglobal_df(false, 0L);
			} else if ((ret == -2 || samePoint) && type == isVideo) {
				strcpy(global_df->MvTr.MovieFile, mfn);
				global_df->MvTr.MBeg = mBegF;
				global_df->MvTr.MEnd = mEndF;
				SetPBCglobal_df(false, 0L);
			}
		}
	}

	if (samePoint) {
		if (type == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
			type = isVideo;
			global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
			global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
		}
		if (type == isAudio) {
			DrawSoundCursor(0);
			if (global_df->SnTr.BegF >= global_df->SnTr.EndF) {
				global_df->SnTr.EndF = global_df->SnTr.BegF + AlignMultibyteMediaStream(conv_from_msec_rep(PBC.step_length), '+');
			} else if (PBC.backspace > PBC.step_length) {
				t = conv_to_msec_rep(global_df->SnTr.BegF) - (PBC.backspace - PBC.step_length);
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				t = conv_to_msec_rep(global_df->SnTr.EndF) - (PBC.backspace - PBC.step_length);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
			} else {
				t = conv_to_msec_rep(global_df->SnTr.BegF) - (PBC.step_length - PBC.backspace);
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				t = conv_to_msec_rep(global_df->SnTr.EndF) - (PBC.step_length - PBC.backspace);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
			}
			if (global_df->SnTr.BegF < 0L) {
				global_df->SnTr.BegF = 0L;
				global_df->SnTr.EndF = global_df->SnTr.BegF + AlignMultibyteMediaStream(conv_from_msec_rep(PBC.step_length), '+');
			}
			t = conv_from_msec_rep(PBC.step_length);
			if (global_df->SnTr.EndF - global_df->SnTr.BegF < t) {
				global_df->SnTr.EndF = global_df->SnTr.BegF + AlignMultibyteMediaStream(t, '+');
			}
			SetPBCglobal_df(false, 0L);
			DisplayEndF(TRUE);
			DrawSoundCursor(3);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			global_df->SnTr.SSource = 1;
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
//			strcpy(global_df->err_message, DASHES);
			PlaySound(&global_df->SnTr, (int)'r');
		} else if (type == isVideo) {
			if (global_df->MvTr.MBeg >= global_df->MvTr.MEnd) {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
			} else if (PBC.backspace > PBC.step_length) {
				global_df->MvTr.MBeg = global_df->MvTr.MBeg - (PBC.backspace - PBC.step_length);
				global_df->MvTr.MEnd = global_df->MvTr.MEnd - (PBC.backspace - PBC.step_length);
			} else {
				global_df->MvTr.MBeg = global_df->MvTr.MBeg - (PBC.step_length - PBC.backspace);
				global_df->MvTr.MEnd = global_df->MvTr.MEnd - (PBC.step_length - PBC.backspace);
			}
			if (global_df->MvTr.MBeg < 0L) {
				global_df->MvTr.MBeg = 0L;
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
			}
			if (global_df->MvTr.MEnd - global_df->MvTr.MBeg < PBC.step_length) {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
			}
			SetPBCglobal_df(false, 0L);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
			PlayMovie(&global_df->MvTr,global_df, FALSE);
		}
	} else {
		if (global_df->SnTr.SoundFile[0] != EOS && global_df->SnTr.IsSoundOn && !syncAudioAndVideo(global_df->MvTr.MovieFile)) {
			strcpy(sFN, global_df->SnTr.SoundFile);
			sBF = global_df->SnTr.BegF;
			sEF = global_df->SnTr.EndF;
			DrawSoundCursor(0);
			global_df->SnTr.EndF = AlignMultibyteMediaStream(global_df->SnTr.BegF+conv_from_msec_rep(PBC.step_length), '+');
			DisplayEndF(TRUE);
			DrawSoundCursor(3);
			strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
			global_df->SnTr.SSource = 1;
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
//			strcpy(global_df->err_message, DASHES);
			PlaySound(&global_df->SnTr, (int)'r');
		} else if (findStartMediaTag(FALSE, F5Option == EVERY_LINE) == TRUE) {
			if (!FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
				strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
			} else {
				if (FindSndInfo(&isWhatType) > 0) {
					isTopWinChanged = (global_df->top_win != old_top_win);
					global_df->row_txt = old_row_txt;
					global_df->top_win = old_top_win;
					global_df->row_win = old_row_win;
					global_df->col_win = old_col_win;
					global_df->col_chr = old_col_chr;
					global_df->lineno  = old_lineno;
					global_df->wLineno = old_wLineno;
					global_df->LeftCol = old_LeftCol;
					if (global_df->row_txt == global_df->cur_line) {
						long j;
						for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					if (isTopWinChanged)
						DisplayTextWindow(NULL, 1);

					if (isWhatType == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
						isWhatType = isVideo;
						global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
						global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
					}
					if (isWhatType == isAudio) {
						strcpy(sFN, global_df->SnTr.SoundFile);
						sBF = global_df->SnTr.BegF;
						sEF = global_df->SnTr.EndF;
						DrawSoundCursor(0);
						t = conv_to_msec_rep(global_df->SnTr.BegF);
						global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
						DisplayEndF(TRUE);
						DrawSoundCursor(3);
						global_df->SnTr.SSource = 1;
					} else {
						strcpy(sFN, global_df->MvTr.MovieFile);
						sBF = global_df->MvTr.MBeg;
						sEF = global_df->MvTr.MEnd;
						global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
					}
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					if (isWhatType == isAudio)
						PlaySound(&global_df->SnTr, (int)'r');
					else
						PlayMovie(&global_df->MvTr,global_df, FALSE);
					type = isWhatType;
				}
			}
		} else {
			strcpy(global_df->err_message, "+Please choose media file in Walker Controller with \"Open Media\" button.");
			return(83);
/*
			if ((ret=getNewMediaFile(TRUE, isAllType))) {
				if (ret == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
					ret = isVideo;
					global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
				}
				if (ret == isAudio) { // sound
					strcpy(sFN, global_df->SnTr.SoundFile);
					sBF = global_df->SnTr.BegF;
					sEF = global_df->SnTr.EndF;
					DrawSoundCursor(0);
					t = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
					DisplayEndF(TRUE);
					DrawSoundCursor(3);
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					global_df->SnTr.SSource = 1;
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					PlaySound(&global_df->SnTr, (int)'r');
					type = 1;
				} else if (ret == isVideo) { // movie
					strcpy(sFN, global_df->MvTr.MovieFile);
					sBF = global_df->MvTr.MBeg;
					sEF = global_df->MvTr.MEnd;
					global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
					PlayMovie(&global_df->MvTr,global_df, FALSE);
					type = 2;
				} else {
					strcpy(global_df->err_message, "+Unsupported media type.");
				}
			}
 */
		}
	}
#ifndef _WIN32
	if (type != 2)
		PBC.isPC = 0;
#endif
	return(83);
}

int ShowPicture(int c) {
	int i;

	global_df->isErrorFindPict = FALSE;
	i = (int)global_df->DataChanged;
	if (c >= 0) {
		if (!FindTextCodeLine(cl_T(PICTTIER), NULL)) {
			strcpy(global_df->err_message, "+Picture marker is not found at cursor position");
			return(1);
		}
		ChangeCurLine();
	} else
		ChangeCurLine();

	if (!i)
		global_df->DataChanged = '\0';

	if ((i=FindPictInfo(TRUE)) == 1) {
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
		DisplayPhoto(global_df->PcTr.pictFName);
	} else if (i == 0) {
		sprintf(global_df->err_message, "+%s tier is corrupted or missing.", PICTTIER);
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
	}
	return(1);
}

int ShowTextFile(int c) {
	int i;

	i = (int)global_df->DataChanged;
	if (c >= 0) {
		if (!FindTextCodeLine(cl_T(TEXTTIER), NULL)) {
			strcpy(global_df->err_message, "+Text file marker is not found at cursor position");
			return(1);
		}
		ChangeCurLine();
	} else
		ChangeCurLine();

	if (!i)
		global_df->DataChanged = '\0';

	if ((i=FindTextInfo()) == 1) {
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
		DisplayText(global_df->TxTr.textFName);
	} else if (i == 0) {
		sprintf(global_df->err_message, "+%s tier is corrupted or missing.", TEXTTIER);
		if (c > 0) {
			SaveUndoState(FALSE);
			Undo(-1);
		}
	}
	return(1);
}

int PlaySound(sndInfo *sndRec, int cont) {
	long begin, end;

	if (syncAudioAndVideo(global_df->MvTr.MovieFile)) {
		global_df->MvTr.MBeg = conv_to_msec_rep(sndRec->BegF);
		global_df->MvTr.MEnd = conv_to_msec_rep(sndRec->EndF);
		PlayMovie(&global_df->MvTr, global_df, FALSE);
		return(0);
	} else 
#ifdef _WIN32
			if (global_df->SnTr.isMP3 == TRUE) {
		global_df->MvTr.MBeg = conv_to_msec_rep(sndRec->BegF);
		global_df->MvTr.MEnd = conv_to_msec_rep(sndRec->EndF);
		strcpy(global_df->MvTr.rMovieFile, sndRec->rSoundFile);
		PlayMovie(&global_df->MvTr, global_df, FALSE);
		return(0);
	} else
#endif
			{
		sndRec->BegF = AlignMultibyteMediaStream(sndRec->BegF, '+');
		begin = sndRec->BegF;
		end = sndRec->EndF;
		if (begin >= sndRec->SoundFileSize) {
			PlayingSound = FALSE;
			PlayingContSound = FALSE;
			strcpy(global_df->err_message, "+Starting point is beyond end of sound file.");
			return(1);
		} else if (begin > end || (begin == end && cont != (int)'p')) {
			PlayingSound = FALSE;
			PlayingContSound = FALSE;
			sprintf(global_df->err_message, "+Begin value of selection is greater or equal to end value.");
			return(1);
		} else
			return(play_sound(begin, end, cont));
	}
}

static char isInsertTab(void) {
	LINE *t;
	char stats;

	stats = 0;
	for (t=global_df->head_row->next_char; t != global_df->col_txt && t != global_df->tail_row; t=t->next_char) {
		if (t->c == '*' && t->prev_char == global_df->head_row && stats == 0)
			stats = 1;
		if (t->c == ':' && stats == 1)
			stats = 2;
		if (t->c == '\t' || t->c == ' ')
			stats = 0;
	}
	if (stats == 2)
		return(TRUE);
	else if (global_df->col_txt->prev_char == global_df->head_row)
		return(TRUE);
	else
		return(FALSE);
}

void addBulletsToText(const char *tagName, const unCH *file, long begin, long end) {
	char isSpaceAdded;
	char foundTier;
	long len;
	char st[256];
	unCH uSt[2048];
	LINE *tl;

	if (global_df == NULL)
		return;
	if (!global_df->EditorMode) {
		do_warning("Please exit Coders mode first.", 0);
		return;
	}
	SaveUndoState(FALSE);
	ChangeCurLine();

	if ((PlayingContSound == '\004' || PlayingContMovie == '\003') && F5_Offset > 0L) {
		if (begin < end - F5_Offset)
			begin = end - F5_Offset;
		if (begin < 0L)
			begin = 0L;
	}

	if (isInsertTab())
		uSt[0] = '\t';
	else
		uSt[0] = ' ';
	uSt[1] = HIDEN_C;
	uSt[2] = EOS;
	if (tagName[1] == 's') {
/* 2009-02-26
		if (global_df->mediaFileName[0] == EOS) {
			strcat(uSt, tagName);
			strcat(uSt, "\"");
			strcat(uSt, file);
			strcat(uSt, "\"_");
		}
*/
		uS.sprintf(st, "%ld_%ld", begin, end);
		strcat(uSt, st);
	} else {
		strcat(uSt, tagName);
		strcat(uSt, "\"");
		strcat(uSt, file);
		strcat(uSt, "\"");
	}
	len = strlen(uSt);
	uSt[len++] = HIDEN_C;
	uSt[len] = EOS;
	uS.lowercasestr(uSt, &dFnt, FALSE);
	if (global_df->col_txt->c != ' ' && global_df->col_txt->c != EOS &&
		global_df->col_txt->c != NL_C && global_df->col_txt->c != SNL_C &&
		global_df->col_txt->c != HIDEN_C) {
		isSpaceAdded = TRUE;
		strcat(uSt, " ");
	} else
		isSpaceAdded = FALSE;

	foundTier = FindTextCodeLine(cl_T(SOUNDTIER), NULL);
	if (global_df->col_txt->c == HIDEN_C && foundTier) {
		if (global_df->col_txt->prev_char == global_df->head_row || isSpace(global_df->col_txt->prev_char->c))
			strcpy(uSt, uSt+1);
		global_df->col_chr2 = global_df->col_chr+1;
		for (tl=global_df->col_txt->next_char; tl->c != HIDEN_C && tl != global_df->tail_row; tl=tl->next_char)
			global_df->col_chr2++;
		if (tl->c == HIDEN_C)
			global_df->col_chr2++;
		global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
		DeleteChank(1);
		if (global_df->UndoList->key != INSTKEY)
			SaveUndoState(FALSE);
	} else if (global_df->col_txt->prev_char != global_df->tail_row && isSpace(global_df->col_txt->prev_char->c))
		strcpy(uSt, uSt+1);

	if (FreqCountLimit > 0 && global_df->FreqCount <= FreqCountLimit)
		global_df->FreqCount++;
	global_df->gAtt = 0;
	AddText(NULL, uSt[0], -2, 1L);
	if (global_df->UndoList->key != INSTKEY)
		SaveUndoState(FALSE);
	AddText(uSt+1, EOS, -2, (long)strlen(uSt+1));
	PosAndDispl();
	global_df->LastCommand = -2;
	if (isSpaceAdded) {
		ChangeCurLineAlways(0);
		MoveLeft(-1);
		PosAndDispl();
		ChangeCurLineAlways(0);
	}
}

static char MatchBulletTime(ROWS *tRow, long mtime, unCH *file, unCH *nFName, long *Beg, long *End, char isSetBE, char isSnd, char pic) {
	register int i;
	unCH *line;
	char m;
	long beg = 0L, end = 0L;
	LINE *tl;

	*Beg = beg;
	*End = end;
	if (tRow == global_df->cur_line) {
		for (tl=global_df->head_row->next_char; tl != global_df->tail_row; tl=tl->next_char) {
			if (tl->c == HIDEN_C) {
				tl = tl->next_char;
				if (is_unCH_digit(tl->c) && global_df->mediaFileName[0] != EOS) {
					strcpy(sp, SOUNDTIER);
				} else {
					for (i=0; tl!= global_df->tail_row && tl->c!= ':' && tl->c!= HIDEN_C; tl=tl->next_char)
						sp[i++] = tl->c;
					if (tl == global_df->tail_row)
						return(FALSE);
					if (tl->c!= ':') {
						sp[i++] = tl->c;
						tl = tl->next_char;
						sp[i] = EOS;
					} else
						strcpy(sp, "NON_MEDIA_BULLET");
				}
				if ((PlayingContMovie && PlayingContMovie != '\003') ||
					(PlayingContSound && PlayingContSound != '\004')) {
					if (pic && global_df->PcTr.pictChanged == FALSE) {
						if (uS.partcmp(sp, PICTTIER, FALSE, FALSE)) {
							m = global_df->DataChanged;
							if (GetCurHidenCode(TRUE, cl_T(PICTTIER))) {
								FindPictInfo(FALSE);
							}
							if (!m)
								global_df->DataChanged = '\0';
						}
					}
					if (pic && global_df->TxTr.textChanged == FALSE) {
						if (uS.partcmp(sp, TEXTTIER, FALSE, FALSE)) {
							m = global_df->DataChanged;
							if (GetCurHidenCode(TRUE, cl_T(TEXTTIER)))
								FindTextInfo();
							if (!m)
								global_df->DataChanged = '\0';
						}
					}
				}
				if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
					while (tl != global_df->tail_row && (isSpace(tl->c) || tl->c == '_')) 
						tl = tl->next_char;
					if (tl == global_df->tail_row)
						return(FALSE);
					if (is_unCH_digit(tl->c) && global_df->mediaFileName[0] != EOS) {
						strcpy(sp, global_df->mediaFileName);
					} else {
						if (tl->c != '"')
							return(FALSE);
						tl = tl->next_char;
						if (tl == global_df->tail_row)
							return(FALSE);
						for (i=0; tl != global_df->tail_row && tl->c != '"'; tl = tl->next_char)
							sp[i++] = tl->c;
						sp[i] = EOS;
					}
					if (nFName != NULL)
						strcpy(nFName, sp);
					if (uS.mStricmp(sp, file) != 0)
						return(FALSE);
					while (tl != global_df->tail_row && !is_unCH_digit(tl->c))
						tl = tl->next_char;
					if (tl == global_df->tail_row)
						return(FALSE);
					for (i=0; tl != global_df->tail_row && is_unCH_digit(tl->c); tl = tl->next_char)
						sp[i++] = tl->c;
					sp[i] = EOS;
					beg = uS.atol(sp);
					if (beg == 0L)
						beg = 1L;
					while (tl != global_df->tail_row && !is_unCH_digit(tl->c))
						tl = tl->next_char;
					if (tl == global_df->tail_row)
						return(FALSE);
					for (i=0; tl != global_df->tail_row && is_unCH_digit(tl->c); tl = tl->next_char)
						sp[i++] = tl->c;
					sp[i] = EOS;
					end = uS.atol(sp);
					*Beg = beg;
					*End = end;
					if (mtime >= beg && mtime < end) {
						if (isSnd) {
							beg = AlignMultibyteMediaStream(conv_from_msec_rep(beg), '-');
							end = AlignMultibyteMediaStream(conv_from_msec_rep(end), '-');
							if (isSetBE) {
								global_df->SnTr.BegF = beg;
								global_df->SnTr.EndF = end;
								SetPBCglobal_df(false, 0L);
							} else {
								global_df->SnTr.dBegF = beg;
								global_df->SnTr.dEndF = end;
								if (global_df->SoundWin && PlayingContSound == TRUE) {
									global_df->SnTr.contPlayBeg = beg;
									global_df->SnTr.contPlayEnd = end;
								}
							}
						}
						if (!isSnd && isSetBE) {
							global_df->MvTr.MBeg = beg;
							global_df->MvTr.MEnd = end;
						}
						return(TRUE);
					}
				}
				for (; tl!= global_df->tail_row && tl->c!= HIDEN_C; tl=tl->next_char) ;
				if (tl == global_df->tail_row)
					return(FALSE);
			}
		}
	} else {
		for (line=tRow->line; *line; line++) {
			if (*line == HIDEN_C) {
				line++;
				if (is_unCH_digit(*line) && global_df->mediaFileName[0] != EOS) {
					strcpy(sp, SOUNDTIER);
				} else {
					for (i=0; *line && *line != ':' && *line != HIDEN_C; line++)
						sp[i++] = *line;
					if (*line == EOS)
						return(FALSE);
					if (*line == ':') {
						sp[i++] = *line;
						line++;
						sp[i] = EOS;
					} else
						strcpy(sp, "NON_MEDIA_BULLET");
				}
				if ((PlayingContMovie && PlayingContMovie != '\003') ||
					(PlayingContSound && PlayingContSound != '\004')) {
					if (pic && global_df->PcTr.pictChanged == FALSE) {
						if (uS.partcmp(sp, PICTTIER, FALSE, FALSE)) {
							m = global_df->DataChanged;
							if (GetCurHidenCode(TRUE, cl_T(PICTTIER))) {
								FindPictInfo(FALSE);
							}
							if (!m)
								global_df->DataChanged = '\0';
						}
					}
					if (pic && global_df->TxTr.textChanged == FALSE) {
						if (uS.partcmp(sp, TEXTTIER, FALSE, FALSE)) {
							m = global_df->DataChanged;
							if (GetCurHidenCode(TRUE, cl_T(TEXTTIER)))
								FindTextInfo();
							if (!m)
								global_df->DataChanged = '\0';
						}
					}
				}
				if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
					while (*line && (isSpace(*line) || *line == '_'))
						line++;
					if (is_unCH_digit(*line) && global_df->mediaFileName[0] != EOS) {
						strcpy(sp, global_df->mediaFileName);
					} else {
						if (*line != '"')
							return(FALSE);
						line++;
						if (*line == EOS)
							return(FALSE);
						for (i=0; *line && *line != '"'; line++)
							sp[i++] = *line;
						sp[i] = EOS;
					}
					if (nFName != NULL)
						strcpy(nFName, sp);
					if (uS.mStricmp(sp, file) != 0)
						return(FALSE);
					while (*line && !is_unCH_digit(*line))
						line++;
					if (*line == EOS)
						return(FALSE);
					for (i=0; *line && is_unCH_digit(*line); line++)
						sp[i++] = *line;
					sp[i] = EOS;
					beg = uS.atol(sp);
					if (beg == 0L)
						beg = 1L;
					while (*line && !is_unCH_digit(*line))
						line++;
					if (*line == EOS)
						return(FALSE);
					for (i=0; *line && is_unCH_digit(*line); line++)
						sp[i++] = *line;
					sp[i] = EOS;
					end = uS.atol(sp);
					*Beg = beg;
					*End = end;
					if (mtime >= beg && mtime < end) {
						if (isSnd) {
							beg = AlignMultibyteMediaStream(conv_from_msec_rep(beg), '-');
							end = AlignMultibyteMediaStream(conv_from_msec_rep(end), '-');
							if (isSetBE) {
								global_df->SnTr.BegF = beg;
								global_df->SnTr.EndF = end;
								SetPBCglobal_df(false, 0L);
							} else {
								global_df->SnTr.dBegF = beg;
								global_df->SnTr.dEndF = end;
								if (global_df->SoundWin && PlayingContSound == TRUE) {
									global_df->SnTr.contPlayBeg = beg;
									global_df->SnTr.contPlayEnd = end;
								}
							}
						}
						if (!isSnd && isSetBE) {
							global_df->MvTr.MBeg = beg;
							global_df->MvTr.MEnd = end;
						}
						return(TRUE);
					}
				}
				for (; *line && *line != HIDEN_C; line++) ;
				if (*line == EOS)
					return(FALSE);
			}
		}
	}
	return(FALSE);
}

char move_cursor(long mtime, unCH *file, char isSetBE, char isSnd) {
	unCH nFName[FNSize];
	FNType old_pictFName[FNSize];
	char old_pictChanged = global_df->PcTr.pictChanged;
	FNType old_textFName[FNSize];
	char old_textChanged = global_df->TxTr.textChanged, isAddLineno;
	long old_col_win = global_df->col_win;
	long old_col_chr = global_df->col_chr;
	long old_row_win = global_df->row_win;
	long old_lineno = global_df->lineno;
	long old_wLineno= global_df->wLineno;
	long Beg, End, lastBeg, lastEnd;
	ROWS *old_row_txt = global_df->row_txt;
	ROWS *old_top_win = global_df->top_win;

	if (mtime == 0L)
		mtime = 1L;
	global_df->isRedrawTextWindow = FALSE;
	lastBeg = 0L;
	lastEnd = 0L;
	strcpy(old_pictFName, global_df->PcTr.pictFName);
	strcpy(old_textFName, global_df->TxTr.textFName);
	if (MatchBulletTime(global_df->row_txt, mtime, file, nFName, &Beg, &End, isSetBE, isSnd, TRUE)) {
		if (global_df->PcTr.pictChanged) {
			PreserveStateAndShowPict();
			global_df->PcTr.pictChanged = FALSE;
		}
		if (global_df->TxTr.textChanged) {
			PreserveStateAndShowText();
			global_df->TxTr.textChanged = FALSE;
		}
		global_df->isRedrawTextWindow = TRUE;
		SelectWholeTier(FALSE);
		return TRUE;
	}
	lastBeg = Beg;
	lastEnd = End;

	if (global_df->row_txt->prev_row != global_df->head_text) {
		if (isNL_CFound(global_df->row_txt->prev_row))
			global_df->lineno++;
	} else
		global_df->lineno++;
	global_df->wLineno++;
	if (global_df->row_win < (long)global_df->EdWinSize)
		global_df->row_win++;
	global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
	while (global_df->row_txt != global_df->tail_text) {
		if (MatchBulletTime(global_df->row_txt, mtime, file, nFName, &Beg, &End, isSetBE, isSnd, TRUE)) {
			if (global_df->PcTr.pictChanged) {
				PreserveStateAndShowPict();
				global_df->PcTr.pictChanged = FALSE;
			}
			if (global_df->TxTr.textChanged) {
				PreserveStateAndShowText();
				global_df->TxTr.textChanged = FALSE;
			}
			global_df->isRedrawTextWindow = TRUE;
			SelectWholeTier(FALSE);
			return TRUE;
		}
		if (lastBeg == 0L)
			lastBeg = Beg;
		if (lastEnd == 0L)
			lastEnd = End;
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		if (global_df->row_win < (long)global_df->EdWinSize)
			global_df->row_win++;
	}
	
	strcpy(global_df->PcTr.pictFName, old_pictFName);
	global_df->PcTr.pictChanged = old_pictChanged;
	strcpy(global_df->TxTr.textFName, old_textFName);
	global_df->TxTr.textChanged = old_textChanged;
	global_df->row_txt = old_row_txt;
	global_df->top_win = old_top_win;
	global_df->row_win = old_row_win;
	global_df->col_win = old_col_win;
	global_df->col_chr = old_col_chr;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;

	if (isNL_CFound(global_df->row_txt))
		global_df->lineno--;
	global_df->wLineno--;
	if (global_df->row_win >= 0L)
		global_df->row_win--;
	global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
	while (global_df->row_txt != global_df->head_text) {
		if (MatchBulletTime(global_df->row_txt, mtime, file, nFName, &Beg, &End, isSetBE, isSnd, FALSE)) {
			global_df->isRedrawTextWindow = TRUE;
			SelectWholeTier(FALSE);
			return TRUE;
		}
		if (lastBeg == 0L)
			lastBeg = Beg;
		if (lastEnd == 0L)
			lastEnd = End;
		global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno--;
		global_df->wLineno--;
		if (global_df->row_win >= 0L)
			global_df->row_win--;
	}
	global_df->row_txt = old_row_txt;
	global_df->top_win = old_top_win;
	global_df->row_win = old_row_win;
	global_df->col_win = old_col_win;
	global_df->col_chr = old_col_chr;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;

	global_df->LeaveHighliteOn = FALSE;

	if (isNL_CFound(global_df->row_txt))
		global_df->lineno--;
	global_df->wLineno--;
	if (global_df->row_win >= 0L)
		global_df->row_win--;
	global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
	while (global_df->row_txt != global_df->head_text) {
		MatchBulletTime(global_df->row_txt, mtime, file, nFName, &Beg, &End, isSetBE, isSnd, FALSE);
		if (Beg != 0 && End != 0) {
			if (mtime >= End && mtime <= lastBeg && uS.mStricmp(nFName, file) == 0) {
				global_df->row_win2 = 0L;
				global_df->col_win2 = -2L;
				global_df->col_chr2 = -2L;
				while (!AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE) && !isSpeaker(global_df->row_txt->line[0])) {
					if (isNL_CFound(global_df->row_txt))
						isAddLineno = TRUE;
					else
						isAddLineno = FALSE;
					global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
					if (isAddLineno)
						global_df->lineno++;
					global_df->wLineno++;
					if (global_df->row_win < (long)global_df->EdWinSize)
						global_df->row_win++;
				}
				global_df->isRedrawTextWindow = TRUE;
				DisplayRow(FALSE);
				global_df->col_chr  = 0L;
				global_df->col_win  = 0L;
				if (global_df->row_txt == global_df->cur_line)
					global_df->col_txt = global_df->head_row->next_char;
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				if (isSnd && !isSetBE) {
					global_df->SnTr.dBegF = AlignMultibyteMediaStream(conv_from_msec_rep(End), '-');
					global_df->SnTr.dEndF = AlignMultibyteMediaStream(conv_from_msec_rep(lastBeg), '-');
				} else if (!isSnd && isSetBE) {
					global_df->MvTr.MBeg = End;
					global_df->MvTr.MEnd = lastBeg;
				}
				return '\002';
			}
			lastBeg = Beg;
		}
		global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno--;
		global_df->wLineno--;
		if (global_df->row_win >= 0L)
			global_df->row_win--;
	}
	global_df->row_txt = old_row_txt;
	global_df->top_win = old_top_win;
	global_df->row_win = old_row_win;
	global_df->col_win = old_col_win;
	global_df->col_chr = old_col_chr;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;
	if (mtime <= lastBeg && lastBeg != 0) {
		sprintf(global_df->err_message, "+Current position is before the first media tier.");
		global_df->isRedrawTextWindow = TRUE;
		return '\003';
	}

	if (global_df->row_txt->prev_row != global_df->head_text) {
		if (isNL_CFound(global_df->row_txt->prev_row))
			global_df->lineno++;
	} else
		global_df->lineno++;
	global_df->wLineno++;
	if (global_df->row_win < (long)global_df->EdWinSize)
		global_df->row_win++;
	global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
	while (global_df->row_txt != global_df->tail_text) {
		MatchBulletTime(global_df->row_txt, mtime, file, nFName, &Beg, &End, isSetBE, isSnd, TRUE);
		if (Beg != 0 && End != 0) {
			if (mtime <= Beg && mtime >= lastEnd && uS.mStricmp(nFName, file) == 0) {
				global_df->row_win2 = 0L;
				global_df->col_win2 = -2L;
				global_df->col_chr2 = -2L;
				while (!AtBotEnd(global_df->row_txt,global_df->head_text,FALSE) && !isSpeaker(global_df->row_txt->line[0])) {
					global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
					if (isNL_CFound(global_df->row_txt))
						global_df->lineno--;
					global_df->wLineno--;
					if (global_df->row_win >= 0L && global_df->row_win < (long)global_df->EdWinSize)
						global_df->row_win--;
				}
				global_df->isRedrawTextWindow = TRUE;
				DisplayRow(FALSE);
				global_df->col_chr  = 0L;
				global_df->col_win  = 0L;
				if (global_df->row_txt == global_df->cur_line)
					global_df->col_txt = global_df->head_row->next_char;
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				if (isSnd && !isSetBE) {
					global_df->SnTr.dBegF = AlignMultibyteMediaStream(conv_from_msec_rep(lastEnd), '-');
					global_df->SnTr.dEndF = AlignMultibyteMediaStream(conv_from_msec_rep(Beg), '-');
				} else if (!isSnd && isSetBE) {
					global_df->MvTr.MBeg = lastEnd;
					global_df->MvTr.MEnd = Beg;
				}
				return '\002';
			}
			lastEnd = End;
		}
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		if (global_df->row_win < (long)global_df->EdWinSize)
			global_df->row_win++;
	}
	strcpy(global_df->PcTr.pictFName, old_pictFName);
	global_df->PcTr.pictChanged = old_pictChanged;
	strcpy(global_df->TxTr.textFName, old_textFName);
	global_df->TxTr.textChanged = old_textChanged;
	global_df->row_txt = old_row_txt;
	global_df->top_win = old_top_win;
	global_df->row_win = old_row_win;
	global_df->col_win = old_col_win;
	global_df->col_chr = old_col_chr;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;
	if (mtime >= lastEnd && lastEnd != 0) {
		sprintf(global_df->err_message, "+Current position is after the last media tier.");
		global_df->isRedrawTextWindow = TRUE;
		return '\003';
	}
	global_df->isRedrawTextWindow = TRUE;
	RemoveLastUndo();
	return FALSE;
}

int SoundToTextSync(int c) {
	long mtime;
	unCH *pFName;

	if (global_df->SoundWin) {
		DrawSoundCursor(0);
		if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
			mtime = conv_to_msec_rep(global_df->SnTr.BegF + ((global_df->SnTr.EndF - global_df->SnTr.BegF) / 2L));
		else
			mtime = conv_to_msec_rep(global_df->SnTr.BegF);
		if (syncAudioAndVideo(global_df->MvTr.MovieFile))
			pFName = global_df->MvTr.MovieFile;
		else
			pFName = global_df->SnTr.SoundFile;
		if (!move_cursor(mtime, pFName, TRUE, TRUE))
			strcpy(global_df->err_message, "+Can't find sound position in a text.");
	} else {
		RemoveLastUndo();
		strcpy(global_df->err_message, "+Please, start sonic mode first");
	}
	return(50);
}

int MovieToTextSync(int c) {
	long mtime;

	DrawSoundCursor(0);
	if (global_df->MvTr.MBeg != global_df->MvTr.MEnd && global_df->MvTr.MEnd != 0L)
		mtime = conv_to_msec_rep(global_df->MvTr.MBeg + ((global_df->MvTr.MEnd - global_df->MvTr.MBeg) / 2L));
	else
		mtime = conv_to_msec_rep(global_df->MvTr.MBeg);
	if (!move_cursor(mtime, global_df->MvTr.MovieFile, FALSE, FALSE))
		strcpy(global_df->err_message, "+Can't find movie position in a text.");
	return(88);
}

#if defined(_MAC_CODE)
static void cpAndCleanUpThumbText(unCH *text, MovieTNInfo *t, int i, int max) {
	int  j;
	char hf;

	hf = FALSE;
	for (j=0; j < max-2 && text[i] != EOS; i++) {
		if (text[i] == HIDEN_C)
			hf = !hf;
		else if (!hf)
			t->text[j++] = text[i];
	}
	t->text[j] = EOS;
	t->textLen = strlen(t->text);
	while (TextWidthInPix(t->text, 0L, t->textLen, &t->Font, 0) > ThumbnailsHight) {
		t->textLen--;
	}
}

static void getFirstTierText(ROWS *tr, MovieTNInfo *t, int max) {
	int  i;

	while (!isSpeaker(tr->line[0]) && !AtTopEnd(tr, global_df->tail_text, FALSE))
		tr = ToPrevRow(tr, FALSE);
	for (i=0; tr->line[i] != ':' && tr->line[i] != EOS; i++) ;
	if (tr->line[i] == ':')
		for (i++; isSpace(tr->line[i]); i++) ;
	copyFontInfo(&t->Font, &tr->Font, FALSE);
	cpAndCleanUpThumbText(tr->line, t, i, max);
}
#endif // _MAC_CODE

int MovieThumbNails(int i) {
#if defined(_MAC_CODE)
	char		bf, res;
	unCH		tTf[50];
	long		freqCnt;
	long		cPict;
	long		totalPicts;
	ROWS		*curRow;
	FONTINFO	tFont;
	MovieTNInfo *Picts, *t = NULL;

	DrawMouseCursor(2);
	DrawSoundCursor(0);
	ChangeCurLineAlways(0);
	totalPicts = 0L;
	curRow = ToNextRow(global_df->head_text, FALSE);
	while (!AtBotEnd(curRow, global_df->tail_text, FALSE)) {
		bf = FALSE;
		for (i=0; curRow->line[i]; i++) {
			if (curRow->line[i] == HIDEN_C) {
				res = FindMediaInfoInLine(curRow->line, i, isVideo, NULL, &bf, TRUE);
				if (res == -1) {
					DrawMouseCursor(1);
					strcpy(global_df->err_message, "+Media bullet is corrupted. Please run check.");
					return(80);
				} else if (res == 1) {
					if (bf)
						break;
				}
			}
		}
		if (bf) {
			totalPicts++;
			curRow = ToNextRow(curRow, FALSE);
			while (global_df->ChatMode && !isMainSpeaker(curRow->line[0]) && !AtBotEnd(curRow, global_df->tail_text, FALSE))
				curRow = ToNextRow(curRow, FALSE);
		} else
			curRow = ToNextRow(curRow, FALSE);
	}
	DrawMouseCursor(1);
	freqCnt = DoGetFreqCount(totalPicts);
	if (freqCnt == 0L) {
		DrawMouseCursor(1);
		return(80);
	}		
	DrawMouseCursor(2);
	tTf[0] = EOS;
	cPict = 0L;
	totalPicts = 0L;
	Picts = NULL;
	curRow = ToNextRow(global_df->head_text, FALSE);
	while (!AtBotEnd(curRow, global_df->tail_text, FALSE)) {
		bf = FALSE;
		if (curRow->line[0]=='@' && (curRow->line[1]=='T' || curRow->line[1]=='t') && !is_unCH_alnum(curRow->line[2]) && freqCnt==-1L) {
			for (i=2; curRow->line[i] == ':' || isSpace(curRow->line[i]); i++) ;
			strncpy(tTf, curRow->line+i, 49);
			tTf[49] = EOS;
			if (tTf[0] == EOS || tTf[0] == NL_C)
				strcpy(tTf, "lXvSX1962XvLMs");
			copyFontInfo(&tFont, &curRow->Font, FALSE);
			curRow = ToNextRow(curRow, FALSE);
			continue;
		}
		for (i=0; curRow->line[i]; i++) {
			if (curRow->line[i] == HIDEN_C) {
				res = FindMediaInfoInLine(curRow->line, i, isVideo, NULL, &bf, TRUE);
				if (res == -1) {
					DrawMouseCursor(1);
					strcpy(global_df->err_message, "+Media bullet is corrupted. Please run check.");
					return(80);
				} else if (res == 1) {
					if (bf)
						break;
				}
			}
		}
		if (bf) {
			cPict++;
			if ((freqCnt != -1L && (cPict % freqCnt) == 0) || tTf[0] != EOS) {
				if (strcmp(tTf, "lXvSX1962XvLMs") == 0)
					tTf[0] = EOS;
				totalPicts++;
				if (Picts == NULL) {
					Picts = NEW(MovieTNInfo);
					t = Picts;
				} else if (t != NULL) {
					t->nextPict = NEW(MovieTNInfo);
					t = t->nextPict;
				}
				if (t == NULL) {
					freeThumbnails(Picts);
					strcpy(global_df->err_message, "+Out of memory.");
					DrawMouseCursor(1);
					return(80);
				}
				if (tTf[0] == EOS)
					getFirstTierText(curRow, t, 50);
				else {
					copyFontInfo(&t->Font, &tFont, FALSE);
					cpAndCleanUpThumbText(tTf, t, 0, 50);
				}
				uS.remFrontAndBackBlanks(t->text);
				tTf[0] = EOS;
				t->orgFName = (unCH *)malloc((strlen(global_df->MvTr.MovieFile)+1) * sizeof(unCH));
				if (t->orgFName == NULL) {
					t->fName = NULL;
					t->pict = NULL;
					t->nextPict = NULL;
					freeThumbnails(Picts);
					strcpy(global_df->err_message, "+Out of memory.");
					DrawMouseCursor(1);
					return(80);
				}
				t->fName = (FNType *)malloc((strlen(global_df->MvTr.rMovieFile)+1) * sizeof(FNType));
				if (t->fName == NULL) {
					t->pict = NULL;
					t->nextPict = NULL;
					freeThumbnails(Picts);
					strcpy(global_df->err_message, "+Out of memory.");
					DrawMouseCursor(1);
					return(80);
				}
				strcpy(t->orgFName, global_df->MvTr.MovieFile);
				strcpy(t->fName, global_df->MvTr.rMovieFile);
				t->MBeg = global_df->MvTr.MBeg;
				t->MEnd = global_df->MvTr.MEnd;
				t->row = 0;
				t->pict = NULL;
				t->nextPict = NULL;
			}
			curRow = ToNextRow(curRow, FALSE);
			while (global_df->ChatMode && !isMainSpeaker(curRow->line[0]) &&
						curRow->line[0] != '@' && !AtBotEnd(curRow, global_df->tail_text, FALSE))
				curRow = ToNextRow(curRow, FALSE);
		} else
			curRow = ToNextRow(curRow, FALSE);
	}
	DrawMouseCursor(1);
	if (Picts != NULL) {
		ShowThumbnails(Picts, global_df, totalPicts);
	}
#endif // _MAC_CODE
	return(80);
}

int FindPrevMediaTier(int c) {
	int i;
	char isWhatType;

	if (PBC.enable != 0) {
		char samePoint, type, isTopWinChanged;
		long t;
		long old_col_win = global_df->col_win;
		long old_col_chr = global_df->col_chr;
		long old_row_win = global_df->row_win;
		long old_lineno  = global_df->lineno;
		long old_wLineno = global_df->wLineno;
		long old_LeftCol = global_df->LeftCol;
		ROWS *old_row_txt = global_df->row_txt;
		ROWS *old_top_win = global_df->top_win;

#ifdef _MAC_CODE
		if (theMovie != NULL && theMovie->isPlaying && PBC.enable && PBC.walk) {
			stopMoviePlaying();
			return(96);
		}
#elif _WIN32
/* // NO QT
		if (MovDlg != NULL && !(MovDlg->qt)->IsQTMovieDone() && PBC.enable && PBC.walk) {
			stopMoviePlaying();
			return(96);
		}
*/
		if (MpegDlg != NULL && !(MpegDlg->mpeg)->IsMpegMovieDone() && PBC.enable && PBC.walk) {
			stopMoviePlaying();
			return(96);
		}
#endif
//		global_df->isRedrawTextWindow = FALSE;
		PBC.isPC = 1;
		PBC.walk = 1;
		i = (int)global_df->DataChanged;
		ChangeCurLine();
		if (!i)
			global_df->DataChanged = '\0';

		if (global_df->SnTr.SoundFile[0] != EOS)
			type = isAudio;
#ifdef _MAC_CODE
		else if (MovieReady && theMovie != NULL && theMovie->df == global_df)
			type = isVideo;
#elif _WIN32
/* // NO QT
		else if (MovDlg != NULL)
			type = isVideo;
*/
		else if (MpegDlg != NULL)
			type = isVideo;
#endif
		else
			type = 0;

		samePoint = (type != 0);

		if (findStartMediaTag(FALSE, F5Option == EVERY_LINE) == TRUE) {
			if (!FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
				strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
			} else {
				if (FindSndInfo(&isWhatType) > 0) {
					PBC.walk = 2;
					isTopWinChanged = (global_df->top_win != old_top_win);
					global_df->row_txt = old_row_txt;
					global_df->top_win = old_top_win;
					global_df->row_win = old_row_win;
					global_df->col_win = old_col_win;
					global_df->col_chr = old_col_chr;
					global_df->lineno  = old_lineno;
					global_df->wLineno = old_wLineno;
					global_df->LeftCol = old_LeftCol;
					if (global_df->row_txt == global_df->cur_line) {
						long j;
						for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					if (isTopWinChanged)
						DisplayTextWindow(NULL, 1);

					if (isWhatType == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
						isWhatType = isVideo;
						global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
						global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
					}
					if (isWhatType == isAudio) {
						strcpy(sFN, global_df->SnTr.SoundFile);
						sBF = global_df->SnTr.BegF;
						sEF = global_df->SnTr.EndF;
						DrawSoundCursor(0);
						t = conv_to_msec_rep(global_df->SnTr.BegF);
						t = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
						if (t < global_df->SnTr.EndF)
							global_df->SnTr.EndF = t;
						DisplayEndF(TRUE);
						DrawSoundCursor(3);
						global_df->SnTr.SSource = 1;
					} else {
						strcpy(sFN, global_df->MvTr.MovieFile);
						sBF = global_df->MvTr.MBeg;
						sEF = global_df->MvTr.MEnd;
						t = global_df->MvTr.MBeg + PBC.step_length;
						if (t < global_df->MvTr.MEnd)
							global_df->MvTr.MEnd = t;
					}
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					if (isWhatType == isAudio)
						PlaySound(&global_df->SnTr, (int)'r');
					else
						PlayMovie(&global_df->MvTr,global_df, FALSE);
					type = isWhatType;
				}
			}
		} else if (samePoint) {
			isTopWinChanged = (global_df->top_win != old_top_win);
			global_df->row_txt = old_row_txt;
			global_df->top_win = old_top_win;
			global_df->row_win = old_row_win;
			global_df->col_win = old_col_win;
			global_df->col_chr = old_col_chr;
			global_df->lineno  = old_lineno;
			global_df->wLineno = old_wLineno;
			global_df->LeftCol = old_LeftCol;
			if (global_df->row_txt == global_df->cur_line) {
				long j;
				for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
					global_df->col_txt = global_df->col_txt->next_char;
			}
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			if (isTopWinChanged)
				DisplayTextWindow(NULL, 1);
			if (type == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
				type = isVideo;
				global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
				global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
			}
			if (type == isAudio) {
				DrawSoundCursor(0);
				t = conv_to_msec_rep(global_df->SnTr.BegF);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
				SetPBCglobal_df(false, 0L);
				DisplayEndF(TRUE);
				DrawSoundCursor(3);
				strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
				global_df->SnTr.SSource = 1;
				draw_mid_wm();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
//				strcpy(global_df->err_message, DASHES);
				PlaySound(&global_df->SnTr, (int)'r');
			} else if (type == isVideo) {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				SetPBCglobal_df(false, 0L);
				strcpy(global_df->err_message, "-Playing, press any F5-F6 key to stop");
				draw_mid_wm();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				PlayMovie(&global_df->MvTr,global_df, FALSE);
			}
		} else {
			strcpy(global_df->err_message, "+Please choose media file in Walker Controller with \"Open Media\" button.");
			return(96);
		}
#ifndef _WIN32
		if (type != isVideo) {
			PBC.walk = 0;
			PBC.isPC = 0;
		}
#endif
		return(96);
	} else {
		return(PlayContMedia(c));
	}
}

int FindNextMediaTier(int c) {
	int  i;
	char isWhatType;

	if (PBC.enable != 0) {
		char ret, type, isTopWinChanged;
		char samePoint;
		unCH sfn[FILENAME_MAX], mfn[FILENAME_MAX];
		long sBegF, sEndF, mBegF, mEndF;
		long t;
		long old_col_win = global_df->col_win;
		long old_col_chr = global_df->col_chr;
		long old_row_win = global_df->row_win;
		long old_lineno  = global_df->lineno;
		long old_wLineno = global_df->wLineno;
		long old_LeftCol = global_df->LeftCol;
		ROWS *old_row_txt = global_df->row_txt;
		ROWS *old_top_win = global_df->top_win;
		Boolean done;

		i = (int)global_df->DataChanged;
		done = false;
		do {
			FindAnyBulletsOnLine();
			if (global_df->row_txt == global_df->cur_line)
				done = (global_df->col_txt->c == HIDEN_C);
			else
				done = (global_df->row_txt->line[global_df->col_chr] == HIDEN_C);
			if (!done) {
				if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
					isTopWinChanged = (global_df->top_win != old_top_win);
					global_df->row_txt = old_row_txt;
					global_df->top_win = old_top_win;
					global_df->row_win = old_row_win;
					global_df->col_win = old_col_win;
					global_df->col_chr = old_col_chr;
					global_df->lineno  = old_lineno;
					global_df->wLineno = old_wLineno;
					global_df->LeftCol = old_LeftCol;
					if (global_df->row_txt == global_df->cur_line) {
						long j;
						for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					if (isTopWinChanged)
						DisplayTextWindow(NULL, 1);
					done = true;
				} else {
					MoveDown(-1);
					if (global_df->row_txt == global_df->cur_line) {
						if (global_df->head_row->next_char != global_df->tail_row) {
							if (isSpeaker(global_df->head_row->next_char->c)) {
								isTopWinChanged = (global_df->top_win != old_top_win);
								global_df->row_txt = old_row_txt;
								global_df->top_win = old_top_win;
								global_df->row_win = old_row_win;
								global_df->col_win = old_col_win;
								global_df->col_chr = old_col_chr;
								global_df->lineno  = old_lineno;
								global_df->wLineno = old_wLineno;
								global_df->LeftCol = old_LeftCol;
								if (global_df->row_txt == global_df->cur_line) {
									long j;
									for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
										global_df->col_txt = global_df->col_txt->next_char;
								}
								wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
								if (isTopWinChanged)
									DisplayTextWindow(NULL, 1);
								done = true;
							}
						}
					} else {
						if (isSpeaker(global_df->row_txt->line[0])) {
							isTopWinChanged = (global_df->top_win != old_top_win);
							global_df->row_txt = old_row_txt;
							global_df->top_win = old_top_win;
							global_df->row_win = old_row_win;
							global_df->col_win = old_col_win;
							global_df->col_chr = old_col_chr;
							global_df->lineno  = old_lineno;
							global_df->wLineno = old_wLineno;
							global_df->LeftCol = old_LeftCol;
							if (global_df->row_txt == global_df->cur_line) {
								long j;
								for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
									global_df->col_txt = global_df->col_txt->next_char;
							}
							wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
							if (isTopWinChanged)
								DisplayTextWindow(NULL, 1);
							done = true;
						}
					}
				}
			}
		} while (!done) ;
		ChangeCurLine();
		if (!i)
			global_df->DataChanged = '\0';
		if (!GetCurHidenCode(FALSE, NULL))
			GetCurCode();

		if (global_df->SnTr.SoundFile[0] != EOS)
			type = isAudio;
#ifdef _MAC_CODE
		else if (MovieReady && theMovie != NULL && theMovie->df == global_df)
			type = isVideo;
#elif _WIN32
/* // NO QT
		else if (MovDlg != NULL)
			type = isVideo;
*/
		else if (MpegDlg != NULL)
			type = isVideo;
#endif
		else
			type = 0;

		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			PBC.isPC = 1;
			samePoint = (type != 0);

			if (samePoint) {
				strcpy(sfn, global_df->SnTr.SoundFile);
				sBegF = global_df->SnTr.BegF;
				sEndF = global_df->SnTr.EndF;

				strcpy(mfn, global_df->MvTr.MovieFile);
				mBegF = global_df->MvTr.MBeg;
				mEndF = global_df->MvTr.MEnd;

				ret = FindSndInfo(&isWhatType);
				if (ret > 0) {
					type = isWhatType;
					if (isWhatType == isAudio) {
						if (uS.mStricmp(sFN,global_df->SnTr.SoundFile) || sBF!=global_df->SnTr.BegF || sEF!=global_df->SnTr.EndF)
							samePoint = FALSE;
					} else if (isWhatType == isVideo) {
						if (uS.mStricmp(sFN,global_df->MvTr.MovieFile) || sBF!=global_df->MvTr.MBeg || sEF!=global_df->MvTr.MEnd)
							samePoint = FALSE;
					}
				} else if (ret == -2)
					samePoint = FALSE;

				if (ret != 2 && type == isAudio) {
					strcpy(global_df->SnTr.SoundFile, sfn);
					global_df->SnTr.BegF = sBegF;
					global_df->SnTr.EndF = sEndF;
					SetPBCglobal_df(false, 0L);
				} else if ((ret == -2 || samePoint) && type == isVideo) {
					strcpy(global_df->MvTr.MovieFile, mfn);
					global_df->MvTr.MBeg = mBegF;
					global_df->MvTr.MEnd = mEndF;
					SetPBCglobal_df(false, 0L);
				}
			}

			if (samePoint) {
				isTopWinChanged = (global_df->top_win != old_top_win);
				global_df->row_txt = old_row_txt;
				global_df->top_win = old_top_win;
				global_df->row_win = old_row_win;
				global_df->col_win = old_col_win;
				global_df->col_chr = old_col_chr;
				global_df->lineno  = old_lineno;
				global_df->wLineno = old_wLineno;
				global_df->LeftCol = old_LeftCol;
				if (global_df->row_txt == global_df->cur_line) {
					long j;
					for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				if (isTopWinChanged)
					DisplayTextWindow(NULL, 1);

				if (type == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
					type = isVideo;
					global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
				}
				if (type == isAudio) {
					if (c == 12+7 || c == 12+4) {
						PlaySound(&global_df->SnTr, (int)'p');
					} else {
						DrawSoundCursor(0);
						t = conv_to_msec_rep(global_df->SnTr.BegF);
						global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
						DisplayEndF(TRUE);
						DrawSoundCursor(3);
						strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
						global_df->SnTr.SSource = 1;
						draw_mid_wm();
						wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
						wrefresh(global_df->w1);
//						strcpy(global_df->err_message, DASHES);
						PlaySound(&global_df->SnTr, (int)'r');
					}
				} else if (type == isVideo) {
					global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					PlayMovie(&global_df->MvTr,global_df, FALSE);
				}
			} else {
				if (findStartMediaTag(FALSE, F5Option == EVERY_LINE) == TRUE) {
					if (!FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
						strcpy(global_df->err_message, "+Media marker not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"F5\"");
					} else {
						if (FindSndInfo(&isWhatType) > 0) {
							isTopWinChanged = (global_df->top_win != old_top_win);
							global_df->row_txt = old_row_txt;
							global_df->top_win = old_top_win;
							global_df->row_win = old_row_win;
							global_df->col_win = old_col_win;
							global_df->col_chr = old_col_chr;
							global_df->lineno  = old_lineno;
							global_df->wLineno = old_wLineno;
							global_df->LeftCol = old_LeftCol;
							if (global_df->row_txt == global_df->cur_line) {
								long j;
								for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
									global_df->col_txt = global_df->col_txt->next_char;
							}
							wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
							if (isTopWinChanged)
								DisplayTextWindow(NULL, 1);

							if (isWhatType == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
								isWhatType = isVideo;
								global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
								global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
							}
							if (isWhatType == isAudio) {
								strcpy(sFN, global_df->SnTr.SoundFile);
								sBF = global_df->SnTr.BegF;
								sEF = global_df->SnTr.EndF;
								DrawSoundCursor(0);
								t = conv_to_msec_rep(global_df->SnTr.BegF);
								global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
								DisplayEndF(TRUE);
								DrawSoundCursor(3);
								global_df->SnTr.SSource = 1;
							} else {
								strcpy(sFN, global_df->MvTr.MovieFile);
								sBF = global_df->MvTr.MBeg;
								sEF = global_df->MvTr.MEnd;
								global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
							}
							strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
							draw_mid_wm();
							wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
							wrefresh(global_df->w1);
//							strcpy(global_df->err_message, DASHES);
							if (isWhatType == isAudio) {
								if (c == 12+7 || c == 12+4) {
									PlaySound(&global_df->SnTr, (int)'p');
								} else {
									PlaySound(&global_df->SnTr, (int)'r');
								}
							} else
								PlayMovie(&global_df->MvTr,global_df, FALSE);
							type = isWhatType;
						}
					}
				}
			}
#ifndef _WIN32
			if (type != isVideo)
				PBC.isPC = 0;
#endif
		} else {
			long t;

			PBC.isPC = 1;
			if (type == isAudio && syncAudioAndVideo(global_df->MvTr.MovieFile)) {
				type = isVideo;
				global_df->MvTr.MBeg = conv_to_msec_rep(global_df->SnTr.BegF);
				global_df->MvTr.MEnd = conv_to_msec_rep(global_df->SnTr.EndF);
			}
			if (type == isAudio) {
				isTopWinChanged = (global_df->top_win != old_top_win);
				global_df->row_txt = old_row_txt;
				global_df->top_win = old_top_win;
				global_df->row_win = old_row_win;
				global_df->col_win = old_col_win;
				global_df->col_chr = old_col_chr;
				global_df->lineno  = old_lineno;
				global_df->wLineno = old_wLineno;
				global_df->LeftCol = old_LeftCol;
				if (global_df->row_txt == global_df->cur_line) {
					long j;
					for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				if (isTopWinChanged)
					DisplayTextWindow(NULL, 1);
				if (c == 12+7 || c == 12+4) {
					PlaySound(&global_df->SnTr, (int)'p');
				} else {
					DrawSoundCursor(0);
					t = conv_to_msec_rep(global_df->SnTr.BegF);
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t+PBC.step_length), '+');
					DisplayEndF(TRUE);
					DrawSoundCursor(3);
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					global_df->SnTr.SSource = 1;
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
//					strcpy(global_df->err_message, DASHES);
					PlaySound(&global_df->SnTr, (int)'r');
				}
			} else if (type == isVideo) {
				isTopWinChanged = (global_df->top_win != old_top_win);
				global_df->row_txt = old_row_txt;
				global_df->top_win = old_top_win;
				global_df->row_win = old_row_win;
				global_df->col_win = old_col_win;
				global_df->col_chr = old_col_chr;
				global_df->lineno  = old_lineno;
				global_df->wLineno = old_wLineno;
				global_df->LeftCol = old_LeftCol;
				if (global_df->row_txt == global_df->cur_line) {
					long j;
					for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				if (isTopWinChanged)
					DisplayTextWindow(NULL, 1);
				strcpy(sFN, global_df->MvTr.MovieFile);
				sBF = global_df->MvTr.MBeg;
				sEF = global_df->MvTr.MEnd;
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
				draw_mid_wm();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				PlayMovie(&global_df->MvTr,global_df, FALSE);
			} else {
				strcpy(global_df->err_message, "+Please choose media file in Walker Controller with \"Open Media\" button.");
				return(91);
			}
#ifndef _WIN32
			if (type != 2)
				PBC.isPC = 0;
#endif
		}
	} else {
		if (GetCurHidenCode(FALSE, NULL)) {
			if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
				if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
					MoveDown(1);
				while (!isMainSpeaker(*global_df->row_txt->line) && !AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
					MoveDown(1);
				}
			}
		}
		if (findStartMediaTag(FALSE, F5Option == EVERY_LINE) == TRUE) {
			if (FindTextCodeLine(cl_T(SOUNDTIER), cl_T(SOUNDTIER)) && (FindSndInfo(&isWhatType) > 0)) {
				if (isWhatType == isAudio) {
					strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
					draw_mid_wm();
//					strcpy(global_df->err_message, DASHES);
					DrawSoundCursor(1);
					DrawCursor(1);
					global_df->SnTr.SSource = 2;
					if (PlaySound(&global_df->SnTr, (int)'r'))
						return(91);
					global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
				} else
					i = PlayMovie(&global_df->MvTr, global_df, FALSE);
			}
		}
	}
	return(91);
}

int MoveMediaBegRight(int i) {
	char isWhatType;
	long tb, te;

	ChangeCurLine();
	FindAnyBulletsOnLine();
	if (GetCurHidenCode(TRUE, NULL)) {
		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			global_df->err_message[0] = EOS;
			if (FindSndInfo(&isWhatType) > 0 ) {
				if (isWhatType == isAudio) {
					tb = conv_to_msec_rep(global_df->SnTr.BegF);
					te = conv_to_msec_rep(global_df->SnTr.EndF);
					tb += MOVEINCREMENT;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(tb), '-');
					SetPBCglobal_df(false, 0L);
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, tb, te);
					if (!sameKeyPressed(94)) {
/*
						strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
						draw_mid_wm();
//						strcpy(global_df->err_message, DASHES);
*/
						DrawSoundCursor(3);
						DrawCursor(1);
						global_df->SnTr.SSource = 2;
						if (PlaySound(&global_df->SnTr, (int)'r'))
							return(94);
						global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
					} else {
						DrawSoundCursor(3);
						DrawCursor(1);
					}
				} else {
					global_df->MvTr.MBeg += MOVEINCREMENT;
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
					if (!sameKeyPressed(92)) {
					} else {
						DrawCursor(1);
					}
				}
			} else {
				if (global_df->err_message[0] == EOS)
					strcpy(global_df->err_message, "+Corrupted sound tag.");
			}
		} else {
			strcpy(global_df->err_message, "+Unsupported media tag.");
		}
	} else
		strcpy(global_df->err_message, "+Please position text cursor next to a bullet");
	return(94);
}

int MoveMediaBegLeft(int i) {
	char isWhatType;
	long tb, te;

	ChangeCurLine();
	FindAnyBulletsOnLine();
	if (GetCurHidenCode(TRUE, NULL)) {
		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			global_df->err_message[0] = EOS;
			if (FindSndInfo(&isWhatType) > 0) {
				if (isWhatType == isAudio) {
					tb = conv_to_msec_rep(global_df->SnTr.BegF);
					te = conv_to_msec_rep(global_df->SnTr.EndF);
					tb -= MOVEINCREMENT;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(tb), '-');
					SetPBCglobal_df(false, 0L);
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, tb, te);
					if (!sameKeyPressed(95)) {
/*
						strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
						draw_mid_wm();
//						strcpy(global_df->err_message, DASHES);
*/
						DrawSoundCursor(3);
						DrawCursor(1);
						global_df->SnTr.SSource = 2;
						if (PlaySound(&global_df->SnTr, (int)'r'))
							return(95);
						global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
					} else {
						DrawSoundCursor(3);
						DrawCursor(1);
					}
				} else {
					global_df->MvTr.MBeg -= MOVEINCREMENT;
					if (global_df->MvTr.MBeg < 0L)
						global_df->MvTr.MBeg = 0L;
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
					if (!sameKeyPressed(92)) {
					} else {
						DrawCursor(1);
					}
				}
			} else {
				if (global_df->err_message[0] == EOS)
					strcpy(global_df->err_message, "+Corrupted sound tag.");
			}
		} else {
			strcpy(global_df->err_message, "+Unsupported media tag.");
		}
	} else
		strcpy(global_df->err_message, "+Please position text cursor next to a bullet");
	return(95);
}

int MoveMediaEndRight(int i) {
	char isWhatType;
	long tb, te;

	ChangeCurLine();
	FindAnyBulletsOnLine();
	if (GetCurHidenCode(TRUE, NULL)) {
		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			global_df->err_message[0] = EOS;
			if (FindSndInfo(&isWhatType) > 0) {
				if (isWhatType == isAudio) {
					tb = conv_to_msec_rep(global_df->SnTr.BegF);
					te = conv_to_msec_rep(global_df->SnTr.EndF);
					te += MOVEINCREMENT;
					global_df->SnTr.EndF = conv_from_msec_rep(te);
					if (global_df->SnTr.EndF > global_df->SnTr.SoundFileSize)
						global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, tb, te);
					if (!sameKeyPressed(92)) {
/*
						strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
						draw_mid_wm();
//						strcpy(global_df->err_message, DASHES);
*/
						DrawSoundCursor(3);
						DrawCursor(1);
						global_df->SnTr.SSource = 2;
						if (PlaySound(&global_df->SnTr, (int)'r'))
							return(92);
						global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
					} else {
						DrawSoundCursor(3);
						DrawCursor(1);
					}
				} else {
					global_df->MvTr.MEnd += MOVEINCREMENT;
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
					if (!sameKeyPressed(92)) {
					} else {
						DrawCursor(1);
					}
				}
			} else {
				if (global_df->err_message[0] == EOS)
					strcpy(global_df->err_message, "+Corrupted sound tag.");
			}
		} else {
			strcpy(global_df->err_message, "+Unsupported media tag.");
		}
	} else
		strcpy(global_df->err_message, "+Please position text cursor next to a bullet");
	return(92);
}

int MoveMediaEndLeft(int i) {
	char isWhatType;
	long tb, te;

	ChangeCurLine();
	FindAnyBulletsOnLine();
	if (GetCurHidenCode(TRUE, NULL)) {
		if (uS.partcmp(sp, SOUNDTIER, FALSE, FALSE) || uS.partcmp(sp, REMOVEMOVIETAG, FALSE, FALSE)) {
			global_df->err_message[0] = EOS;
			if (FindSndInfo(&isWhatType) > 0) {
				if (isWhatType == isAudio) {
					tb = conv_to_msec_rep(global_df->SnTr.BegF);
					te = conv_to_msec_rep(global_df->SnTr.EndF);
					te -= MOVEINCREMENT;
					global_df->SnTr.EndF = conv_from_msec_rep(te);
					if (global_df->SnTr.EndF < 0L)
						global_df->SnTr.EndF = 0L;
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, tb, te);
					if (!sameKeyPressed(93)) {
/*
						strcpy(global_df->err_message, "-Press any KEY or click mouse to stop");
						draw_mid_wm();
//						strcpy(global_df->err_message, DASHES);
*/
						DrawSoundCursor(3);
						DrawCursor(1);
						global_df->SnTr.SSource = 2;
						if (PlaySound(&global_df->SnTr, (int)'r'))
							return(93);
						global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
					} else {
						DrawSoundCursor(3);
						DrawCursor(1);
					}
				} else {
					global_df->MvTr.MEnd -= MOVEINCREMENT;
					if (global_df->MvTr.MEnd < 0L)
						global_df->MvTr.MEnd = 0L;
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
					if (!sameKeyPressed(92)) {
					} else {
						DrawCursor(1);
					}
				}
			} else {
				if (global_df->err_message[0] == EOS)
					strcpy(global_df->err_message, "+Corrupted sound tag.");
			}
		} else {
			strcpy(global_df->err_message, "+Unsupported media tag.");
		}
	} else
		strcpy(global_df->err_message, "+Please position text cursor next to a bullet");
	return(93);
}
