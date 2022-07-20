#include "cu.h"
#include "ced.h"
#include "Clan2Doc.h"
#include "w95_QTReplace.h"
#include "w95AIFF.h"
#include "c_clan.h"
#include "MMedia.h"
#include "MpegDlg.h"
#include "CedDlgs.h"
#include <mmsystem.h>
#include <mmreg.h>

#define SCALE_TIME_LIMIT 100000
#define SCROLLPERCENT 10
#define CSIZE sizeof(char)

#ifndef EOS			/* End Of String marker			 */
#define EOS '\0'
#endif

extern DWORD	walker_pause;

static Size		NewF;
static int		cm, left_lim, ScollValue;
static int		SndPlayLoopCnt;
static char		dir = 0;
static char		isWinSndDonePlaying = 1;
static char		isWinSndPlayAborted;
static long		orgBeg;
static BOOL		isCurSB1;
static BOOL		gBufferDone = false;
static HPSTR	SB1    = NULL;
static HPSTR	SB2    = NULL;
static DWORD	oDwVolume;
static LPWAVEHDR	waveHdr1 = NULL;
static LPWAVEHDR	waveHdr2 = NULL;
static LPWAVEHDR	cWaveHdr, tWaveHdr;

HWAVEOUT	hWaveOut  = NULL;
int		sndCom;
char 	quickTranscribe = FALSE;
char	leftChan = 1;
char	rightChan = 1;

void AdjustSoundScroll(int col, Size right_lim);

Size conv_to_msec_MP3(long num) {
	double res, num2 = 1.0;

		res = (double)num;
		res /= num2;
		res *= 1000.0000;
	return(roundUp(res));
}

Size conv_from_msec_MP3(long num) {
	double res, num2 = 1.0;

		res = (double)num;
		res /= 1000.0000;
		res *= num2;
	return(roundUp(res));
}

Size conv_to_msec_rep(Size num) {
	double res, num2;

	res = (double)num;
	num2 = global_df->SnTr.SNDrate / (double)1000.0000;
	res = res / num2;
	num2 = (double)global_df->SnTr.SNDsample;
	res = res / num2;
	num2 = (double)global_df->SnTr.SNDchan;
	res = res / num2;

	return(roundUp(res));
}

Size conv_from_msec_rep(Size num) {
	double res, num2;

	res = (double)num;
	num2 = global_df->SnTr.SNDrate / (double)1000.0000;
	res = res * num2;
	num2 = (double)global_df->SnTr.SNDsample;
	res = res * num2;
	num2 = (double)global_df->SnTr.SNDchan;
	res = res * num2;
	return(roundUp(res));
}
/*
Value		Stored as
real*4		sign bit, 8-bit exponent, 23-bit mantissa
real*8		sign bit, 11-bit exponent, 52-bit mantissa
real*10		sign bit, 15-bit exponent, 64-bit mantissa

In real*4 and real*8 formats, there is an assumed leading 1 in
the mantissa that is not stored in memory, so the mantissas are
actually 24 or 53 bits, even though only 23 or 52 bits are stored.
The real*10 format actually stores this bit.

The exponents are biased by half of their possible value.
This means you subtract this bias from the stored exponent to get
the actual exponent. If the stored exponent is less than the bias,
it is actually a negative exponent.

The exponents are biased as follows:

Exponent			Biased by
8-bit (real*4)		127
11-bit (real*8)		1023
15-bit (real*10)	16383

These exponents are not powers of ten; they are powers of two. That is,
8-bit stored exponents can be up to 127. The value 2**127 is roughly
equivalent to 10**38, which is the actual limit of real*4.

The mantissa is stored as a binary fraction of the form 1.XXX... .
This fraction has a value greater than or equal to 1 and less than 2.
Note that real numbers are always stored in normalized form; that is,
the mantissa is left-shifted such that the high-order bit of the mantissa
is always 1. Because this bit is always 1, it is assumed (not stored) in
the real*4 and real*8 formats. The binary (not decimal) point is assumed
to be just to the right of the leading 1.

The format, then, for the various sizes is as follows:
BYTE 1	BYTE 2	BYTE 3	BYTE 4	...	BYTE n

real*4	XXXX	XMMM	MMMM
		XXXX	MMMM	MMMM

real*8	SXXX	XXXX	MMMM	...	MMMM
		XXXX	MMMM	MMMM		MMMM

real*10	SXXX	XXXX	1MMM	...	MMMM
		XXXX	XXXX	MMMM		MMMM
	
S represents the sign bit, the X's are the exponent bits, and the M's
are the mantissa bits. Note that the leftmost bit is assumed in real*4 and
real*8 formats, but present as "1" in BYTE 3 of the real*10 format.

To shift the binary point properly, you first un-bias the exponent and then
move the binary point to the right or left the appropriate number of bits.

The following are some examples in real*4 format:

                    SXXX XXXX XMMM MMMM ... MMMM MMMM
2   =  1  * 2**1  = 0100 0000 0000 0000 ... 0000 0000 = 4000 0000


Note the sign bit is zero, and the stored exponent is 128, or 100 0000 0 in
binary, which is 127 plus 1. The stored mantissa is (1.) 000 0000 ... 0000 0000,
which has an implied leading 1 and binary point, so the actual mantissa is one.

-2  = -1  * 2**1  = 1100 0000 0000 0000 ... 0000 0000 = C000 0000

Same as +2 except that the sign bit is set. This is true for all IEEE format
floating-point numbers.

 4  =  1  * 2**2  = 0100 0000 1000 0000 ... 0000 0000 = 4080 0000

Same mantissa, exponent increases by one (biased value is 129, or 100 0000 1
in binary.
*/
struct shortCommonChunk {
	short			numChannels;
	unsigned long	numSampleFrames;
	short			sampleSize;
	extended80		sampleRate;
} sCommonChunk;

static struct mContainerChunk {
	unsigned long	ckID;
	long			ckSize;
	unsigned long	formType;
} formAIFF;

static struct mCommonChunk {
	unsigned long	ckID;
	long			ckSize;
	short			numChannels;
	unsigned long	numSampleFrames;
	short			sampleSize;
	extended80		sampleRate;
} commonChunk;

static struct mSoundDataChunk {
	unsigned long	ckID;
	long			ckSize;
	unsigned long	offset;
	unsigned long	blockSize;
} soundDataChunk;

struct myShortRep {
	char b16:1,b15:1,b14:1,b13:1,b12:1,b11:1,b10:1,b9:1,b8:1,b7:1,b6:1,b5:1,b4:1,b3:1,b2:1,b1:1;
} ;
struct myX80Rep {
	unsigned short exp;
	char b16:1,b15:1,b14:1,b13:1,b12:1,b11:1,b10:1,b9:1,b8:1,b7:1,b6:1,b5:1,b4:1,b3:1,b2:1,b1:1;
	char c16:1,c15:1,c14:1,c13:1,c12:1,c11:1,c10:1,c9:1,c8:1,c7:1,c6:1,c5:1,c4:1,c3:1,c2:1,c1:1;
	char d16:1,d15:1,d14:1,d13:1,d12:1,d11:1,d10:1,d9:1,d8:1,d7:1,d6:1,d5:1,d4:1,d3:1,d2:1,d1:1;
	char e16:1,e15:1,e14:1,e13:1,e12:1,e11:1,e10:1,e9:1,e8:1,e7:1,e6:1,e5:1,e4:1,e3:1,e2:1,e1:1;
} ;
struct myDoubleRep {
	char d16:1,d15:1,d14:1,d13:1,d12:1,d11:1,d10:1,d9:1,d8:1,d7:1,d6:1,d5:1,d4:1,d3:1,d2:1,d1:1;
	char c16:1,c15:1,c14:1,c13:1,c12:1,c11:1,c10:1,c9:1,c8:1,c7:1,c6:1,c5:1,c4:1,c3:1,c2:1,c1:1;
	char b16:1,b15:1,b14:1,b13:1,b12:1,b11:1,b10:1,b9:1,b8:1,b7:1,b6:1,b5:1,b4:1,b3:1,b2:1,b1:1;
	unsigned short exp;
} ;

static void flipLong(long *num) {
	char *s, t0, t1, t2, t3;

	s = (char *)num;
	t0 = s[0];
	t1 = s[1];
	t2 = s[2];
	t3 = s[3];
	s[3] = t0;
	s[2] = t1;
	s[1] = t2;
	s[0] = t3;
}

static void flipShort(short *num) {
	char *s, t0, t1;

	s = (char *)num;
	t0 = s[0];
	t1 = s[1];
	s[1] = t0;
	s[0] = t1;
}

static double x80toDouble(struct myX80Rep *x80) {
	double num;
	unsigned short exp;
	struct myDoubleRep *myDouble;
	struct myShortRep  *myShort;

	exp = x80->exp - 16383;
	exp = exp + 1023;
	exp = exp << 4;
	myShort = (struct myShortRep *)&exp;
	myShort->b1 = 0;
//	myShort->b12 = x80->b1;
	myShort->b13 = x80->b2;
	myShort->b14 = x80->b3;
	myShort->b15 = x80->b4;
	myShort->b16 = x80->b5;

	myDouble = (struct myDoubleRep *)&num;
	myDouble->exp = exp;
	myDouble->b1  = x80->b6;
	myDouble->b2  = x80->b7;
	myDouble->b3  = x80->b8;
	myDouble->b4  = x80->b9;
	myDouble->b5  = x80->b10;
	myDouble->b6  = x80->b11;
	myDouble->b7  = x80->b12;
	myDouble->b8  = x80->b13;
	myDouble->b9  = x80->b14;
	myDouble->b10 = x80->b15;
	myDouble->b11 = x80->b16;
	myDouble->b12 = x80->c1;
	myDouble->b13 = x80->c2;
	myDouble->b14 = x80->c3;
	myDouble->b15 = x80->c4;
	myDouble->b16 = x80->c5;
	myDouble->c1  = x80->c6;
	myDouble->c2  = x80->c7;
	myDouble->c3  = x80->c8;
	myDouble->c4  = x80->c9;
	myDouble->c5  = x80->c10;
	myDouble->c6  = x80->c11;
	myDouble->c7  = x80->c12;
	myDouble->c8  = x80->c13;
	myDouble->c9  = x80->c14;
	myDouble->c10 = x80->c15;
	myDouble->c11 = x80->c16;
	myDouble->c12 = x80->d1;
	myDouble->c13 = x80->d2;
	myDouble->c14 = x80->d3;
	myDouble->c15 = x80->d4;
	myDouble->c16 = x80->d5;
	myDouble->d1  = x80->d6;
	myDouble->d2  = x80->d7;
	myDouble->d3  = x80->d8;
	myDouble->d4  = x80->d9;
	myDouble->d5  = x80->d10;
	myDouble->d6  = x80->d11;
	myDouble->d7  = x80->d12;
	myDouble->d8  = x80->d13;
	myDouble->d9  = x80->d14;
	myDouble->d10 = x80->d15;
	myDouble->d11 = x80->d16;
	myDouble->d12 = x80->e1;
	myDouble->d13 = x80->e2;
	myDouble->d14 = x80->e3;
	myDouble->d15 = x80->e4;
	myDouble->d16 = x80->e5;
	return(num);
}

static void Doubletox80(double *num, struct myX80Rep *x80) {
	unsigned short exp;
	struct myDoubleRep *myDouble;
	struct myShortRep  *myShort;
	
	myDouble = (struct myDoubleRep *)num;
	exp = myDouble->exp;
	x80->b6  = myDouble->b1;
	x80->b7  = myDouble->b2;
	x80->b8  = myDouble->b3;
	x80->b9  = myDouble->b4;
	x80->b10 = myDouble->b5;
	x80->b11 = myDouble->b6;
	x80->b12 = myDouble->b7;
	x80->b13 = myDouble->b8;
	x80->b14 = myDouble->b9;
	x80->b15 = myDouble->b10;
	x80->b16 = myDouble->b11;
	x80->c1  = myDouble->b12;
	x80->c2  = myDouble->b13;
	x80->c3  = myDouble->b14;
	x80->c4  = myDouble->b15;
	x80->c5  = myDouble->b16;
	x80->c6  = myDouble->c1;
	x80->c7  = myDouble->c2;
	x80->c8  = myDouble->c3;
	x80->c9  = myDouble->c4;
	x80->c10 = myDouble->c5;
	x80->c11 = myDouble->c6;
	x80->c12 = myDouble->c7;
	x80->c13 = myDouble->c8;
	x80->c14 = myDouble->c9;
	x80->c15 = myDouble->c10;
	x80->c16 = myDouble->c11;
	x80->d1  = myDouble->c12;
	x80->d2  = myDouble->c13;
	x80->d3  = myDouble->c14;
	x80->d4  = myDouble->c15;
	x80->d5  = myDouble->c16;
	x80->d6  = myDouble->d1;
	x80->d7  = myDouble->d2;
	x80->d8  = myDouble->d3;
	x80->d9  = myDouble->d4;
	x80->d10 = myDouble->d5;
	x80->d11 = myDouble->d6;
	x80->d12 = myDouble->d7;
	x80->d13 = myDouble->d8;
	x80->d14 = myDouble->d9;
	x80->d15 = myDouble->d10;
	x80->d16 = myDouble->d11;
	x80->e1  = myDouble->d12;
	x80->e2  = myDouble->d13;
	x80->e3  = myDouble->d14;
	x80->e4  = myDouble->d15;
	x80->e5  = myDouble->d16;

	myShort = (struct myShortRep *)&exp;
	myShort->b1 = 0;
	x80->b1 = 1; // myShort->b12;
	x80->b2 = myShort->b13;
	x80->b3 = myShort->b14;
	x80->b4 = myShort->b15;
	x80->b5 = myShort->b16;
	exp = exp >> 4;
	exp = exp - 1023;
	x80->exp = exp + 16383;
}

char CheckRateChan(sndInfo *snd, char *errMess) {
	snd->DDsound = TRUE;
	snd->AIFFOffset = 0L;
	snd->errMess = NULL;
	snd->SNDformat = 0L;

	if (snd->SFileType == 'MP3!') {
	} else if (snd->SFileType == 'AIFF' || snd->SFileType == 'AIFC') {
		long count;
		long pos;
		ChunkHeader cHead;
		ContainerChunk hHead;

		snd->DDsound = TRUE;
		snd->SoundFileSize = 0L;
		snd->SNDformat = 0L;
		if (fseek(snd->SoundFPtr, 0, SEEK_SET) != 0) {
			sprintf(errMess, "+Error reading sound file(1): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		count = sizeof(hHead);
		if (fread((char *)&hHead, CSIZE, count, snd->SoundFPtr) != count) {
			sprintf(errMess, "+Error reading sound file: %s. It is either corrupt or mislabeled with incorrect file extension", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		if (hHead.ckID != FORMID) {
			flipLong((long*)&hHead.ckID);
			if (hHead.ckID != FORMID) {
				sprintf(errMess, "+Unknown sound file format: %s", snd->rSoundFile);
				snd->errMess = errMess+1;
				return(0);
			}
			snd->DDsound = TRUE;
		}
		if (snd->DDsound) {
			flipLong(&hHead.ckSize);
			flipLong((long*)&hHead.formType);
		}
		if (hHead.formType != AIFFID) {
			sprintf(errMess, "+Only AIFF format supported: %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		hHead.ckSize += 8;
		pos = count;
		while (pos < hHead.ckSize) {
			if (fseek(snd->SoundFPtr, pos, SEEK_SET) != 0) {
				sprintf(errMess, "+Error reading sound file(3): %s", snd->rSoundFile);
				snd->errMess = errMess+1;
				return(0);
			}
			count = sizeof(cHead);
			if (fread((char *)&cHead, CSIZE, count, snd->SoundFPtr) != count) {
				sprintf(errMess, "+Error reading sound file(4): %s", snd->rSoundFile);
				snd->errMess = errMess+1;
				return(0);
			}
			if (snd->DDsound) {
				flipLong((long*)&cHead.ckID);
				flipLong(&cHead.ckSize);
			}
			if (cHead.ckID == CommonID) {
				count = sizeof(sCommonChunk);
				if (fread((char *)&sCommonChunk, CSIZE, count, snd->SoundFPtr) != count) {
					sprintf(errMess, "+Error reading sound file(5): %s", snd->rSoundFile);
					snd->errMess = errMess+1;
					return(0);
				}
				if (snd->DDsound) {
					flipShort(&sCommonChunk.numChannels);
					flipLong((long*)&sCommonChunk.numSampleFrames);
					flipShort(&sCommonChunk.sampleSize);
					flipShort(&sCommonChunk.sampleRate.exp);
					flipShort((short*)&sCommonChunk.sampleRate.man[0]);
					flipShort((short*)&sCommonChunk.sampleRate.man[1]);
					flipShort((short*)&sCommonChunk.sampleRate.man[2]);
					flipShort((short*)&sCommonChunk.sampleRate.man[3]);
				}
				snd->SNDsample = sCommonChunk.sampleSize;
				if (snd->SNDsample == 8) snd->SNDsample = 1;
				else if (snd->SNDsample == 16) snd->SNDsample = 2;
				else snd->SNDsample = 0;
				snd->SNDchan = sCommonChunk.numChannels;
				snd->SNDrate = x80toDouble((struct myX80Rep *)&sCommonChunk.sampleRate);
			} else if (cHead.ckID == SoundDataID) {
				count = 4;
				if (fread((char *)&snd->AIFFOffset, CSIZE, count, snd->SoundFPtr) != count) {
					sprintf(errMess, "+Error reading sound file(6): %s", snd->rSoundFile);
					snd->errMess = errMess+1;
					return(0);
				}
				if (snd->DDsound)
					flipLong((long*)&snd->AIFFOffset);
				if (snd->AIFFOffset != 0L) {
					sprintf(errMess, "+Not supported type of AIFF: %s", snd->rSoundFile);
					snd->errMess = errMess+1;
					return(0);
				}
				count = 4;
				if (fread((char *)&snd->AIFFOffset, CSIZE, count, snd->SoundFPtr) != count) {
					sprintf(errMess, "+Error reading sound file(7): %s", snd->rSoundFile);
					snd->errMess = errMess+1;
					return(0);
				}
				snd->AIFFOffset = pos + 16;
				snd->SoundFileSize = cHead.ckSize - sizeof(cHead);
			}
			pos += cHead.ckSize + sizeof(cHead);
			if (cHead.ckSize % 2L != 0L)
				pos++;
		}
	} else if (snd->SFileType == 'WAVE') {
		unCH			wrSoundFile[FNSize];
		HMMIO          hmmio;
		MMCKINFO       mmckinfoParent;
		MMCKINFO       mmckinfoSubchunk;
		DWORD          dwFmtSize;
		HANDLE         hFormat;
		WAVEFORMATEX   *pFormat;

		snd->DDsound = FALSE;
		fclose(snd->SoundFPtr);
		u_strcpy(wrSoundFile, snd->rSoundFile, FNSize);
		/* Open the given file for reading using buffered I/O.
		*/
		if (!(hmmio=mmioOpen(wrSoundFile,NULL,MMIO_READ | MMIO_ALLOCBUF))) {
			sprintf(errMess, "+Error reading sound file(1): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		/* Locate a 'RIFF' chunk with a 'WAVE' form type
		* to make sure it's a WAVE file.
		*/
		mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		if (mmioDescend(hmmio, &mmckinfoParent, NULL, MMIO_FINDRIFF)) {
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file: %s. It is either corrupt or mislabeled with incorrect file extension", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		/* Now, find the format chunk (form type 'fmt '). It should be
		* a subchunk of the 'RIFF' parent chunk.
		*/
		mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
		if (mmioDescend(hmmio,&mmckinfoSubchunk,&mmckinfoParent,MMIO_FINDCHUNK)) {
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file(3): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		/* Get the size of the format chunk, allocate and lock memory for it.
		*/
		dwFmtSize = mmckinfoSubchunk.cksize;
		hFormat = LocalAlloc(LMEM_MOVEABLE, LOWORD(dwFmtSize));
		if (!hFormat) {
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file(4): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		pFormat = (WAVEFORMATEX *) LocalLock(hFormat);
		if (!pFormat) {
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file(5): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}		
		/* Read the format chunk.
		*/
		if (mmioRead(hmmio, (HPSTR) pFormat, dwFmtSize) != (LONG) dwFmtSize) {
			LocalUnlock( hFormat );
			LocalFree( hFormat );
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file: %s(6)", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		/* Make sure it's a PCM file.
		*/
		if (pFormat->wFormatTag == WAVE_FORMAT_PCM) {
			snd->SNDformat = 0L; //WAVE_FORMAT_PCM;
		} else if (pFormat->wFormatTag == WAVE_FORMAT_ALAW) {
			snd->SNDformat = WAVE_FORMAT_ALAW;
		} else if (pFormat->wFormatTag == WAVE_FORMAT_MULAW) {
			snd->SNDformat = WAVE_FORMAT_MULAW;
		} else {
			sprintf(errMess, "+Unsupported WAV files format; file: %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		snd->SNDchan = pFormat->nChannels;
		snd->SNDrate = (double)pFormat->nSamplesPerSec;
		snd->SNDsample = pFormat->wBitsPerSample / 8;
		/* Ascend out of the format subchunk.
		*/
		mmioAscend(hmmio, &mmckinfoSubchunk, 0);
		/* Find the data subchunk.
		*/
		mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if (mmioDescend(hmmio,&mmckinfoSubchunk,&mmckinfoParent,MMIO_FINDCHUNK)){
			LocalUnlock( hFormat );
			LocalFree( hFormat );
			mmioClose(hmmio, 0);
			sprintf(errMess, "+Error reading sound file(7): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
		/* Get the size of the data subchunk.
		*/
		snd->SoundFileSize = mmckinfoSubchunk.cksize;
		snd->AIFFOffset = mmioSeek(hmmio,0L,SEEK_CUR); 
		/* We're done with the format header, free it.
		*/
		LocalUnlock( hFormat );
		LocalFree( hFormat );
		/* We're done with the file, close it.
		*/
		mmioClose(hmmio, 0);
		if ((snd->SoundFPtr=fopen(snd->rSoundFile,"rb")) == NULL) {
			sprintf(errMess, "+Error reading sound file(8): %s", snd->rSoundFile);
			snd->errMess = errMess+1;
			return(0);
		}
	} else {
		sprintf(errMess, "+Unsupported audio file format; file: %s", snd->rSoundFile);
		snd->errMess  = errMess+1;
		return(0);
	}

	if (snd->SNDsample != 1 && snd->SNDsample != 2) {
		snd->errMess = "When displaying a wave only 8 or 16 bit sample size supported.";
	}
	if (snd->SNDchan != 1 && snd->SNDchan != 2) {
		snd->errMess = "When displaying a wave only monaural and stereo sounds supported.";
	}
	if ((snd->SNDrate >= 48000.0 && snd->SNDrate < 49000.0) ||
		(snd->SNDrate >= 44000.0 && snd->SNDrate < 45000.0) ||
		(snd->SNDrate >= 32000.0 && snd->SNDrate < 33000.0) ||
		(snd->SNDrate >= 24000.0 && snd->SNDrate < 25000.0) ||
		(snd->SNDrate >= 22000.0 && snd->SNDrate < 23000.0) ||
		(snd->SNDrate >= 16000.0 && snd->SNDrate < 17000.0) ||
		(snd->SNDrate >= 11000.0 && snd->SNDrate < 12000.0) ||
		(snd->SNDrate >=  8000.0 && snd->SNDrate <  9000.0))
		;
	else {
		snd->errMess = "In sonic only 8, 11, 16, 22, 24, 32, 44 and 48kHz rates supported.";
	}
	if (snd->IsSoundOn && snd->errMess != NULL) {
		cChan = -1;
		soundwindow(0);
		snd->IsSoundOn = FALSE;
	} else if (snd->IsSoundOn && cChan != -1 && cChan != snd->SNDchan && !doMixedSTWave) {
		cChan = -1;
		soundwindow(0);
		snd->IsSoundOn = (char)(soundwindow(1) == 0);
		if (snd->IsSoundOn)
			cChan = snd->SNDchan;
	}
	return(1);
}

//lxs-mp3
// #define GETMEDIAINFO
#ifdef GETMEDIAINFO
#include <mfidl.h>
#include <mfapi.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfuuid.lib")
//#pragma comment(lib, "mf.lib")

static long DisplayInfo(LPCWSTR url) {
	HRESULT hr;
	long dur;
	UINT32 num;
	UINT32 width, height;
	UINT32 compressed;
	UINT64 duration, j;
	DWORD i, count;
	bool video;
	GUID major;

	dur = 0L;
	::MFStartup(MF_VERSION);

	CComPtr<IMFSourceResolver> spResolver;
	hr = ::MFCreateSourceResolver(&spResolver);
	MF_OBJECT_TYPE type;
	CComPtr<IUnknown> spUnkSource;
	hr = spResolver->CreateObjectFromURL(url, MF_RESOLUTION_MEDIASOURCE, NULL, &type, &spUnkSource);
	CComQIPtr<IMFMediaSource> spSource(spUnkSource);
	CComPtr<IMFPresentationDescriptor> spDesc;
	hr = spSource->CreatePresentationDescriptor(&spDesc);

	spDesc->GetStreamDescriptorCount(&count);
	for (i=0; i < count; i++) {
		BOOL selected;
		CComPtr<IMFStreamDescriptor> spStreamDesc;
		hr = spDesc->GetStreamDescriptorByIndex(i, &selected, &spStreamDesc);
		if (selected) {
			CComPtr<IMFMediaTypeHandler> spHandler;
			spStreamDesc->GetMediaTypeHandler(&spHandler);
			CComPtr<IMFMediaType> spMediaType;
			spHandler->GetCurrentMediaType(&spMediaType);
			spMediaType->GetMajorType(&major);
			if (major == MFMediaType_Audio)
				video = false;
			else if (major == MFMediaType_Video)
				video = true;
			else
				continue;
			hr = spMediaType->GetUINT32(MF_MT_COMPRESSED, &compressed);
			if (hr == S_OK) {
				//fprintf(stdout, "Compressed: %s\n", (compressed ? "True" : "False"));
			}

			if (video) {
				hr = spMediaType->GetUINT32(MF_MT_AVG_BITRATE, &num);
				if (hr == S_OK) {
					//fprintf(stdout, "Average bitrate: %d Kbps\n", (num >> 10));
				}
				::MFGetAttributeSize(spMediaType, MF_MT_FRAME_SIZE, &width, &height);
				//fprintf(stdout, "Frame size: %d X %d\n", width, height);
				::MFGetAttributeRatio(spMediaType, MF_MT_FRAME_RATE, &width, &height);
				//fprintf(stdout, "Frame rate: %d FPS\n", width / (float)height);
			} else {
				hr = spMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &num);
				if (hr == S_OK) {
					//fprintf(stdout, "Bits/sample: %d\n", num);
				}
				hr = spMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &num);
				if (hr == S_OK) {
					//fprintf(stdout, "# Channels: %d\n", num);
				}
				hr = spMediaType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &num);
				if (hr == S_OK) {
					//fprintf(stdout, "Average bytes/sec: %d\n", num);
				}
				hr = spMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &num);
				if (hr == S_OK) {
					//fprintf(stdout, "Samples/sec: %d\n", num);
				}
			}
		}
		hr = spDesc->GetUINT64(MF_PD_DURATION, &duration);
		if (hr == S_OK) {
			//fprintf(stdout, "Duration: %ld\n", duration);
			j = duration / 10000L;
		} else
			j = 0L;
		if (j > dur)
			dur = (long)j;
	}
	::MFShutdown();
	return(dur);
}
#endif //GETMEDIAINFO
//lxs-mp3

//#include <Mmsystem.h>
//#include <mciapi.h>
//these two headers are already included in the <Windows.h> header
//#pragma comment(lib, "Winmm.lib")

//#include <dshow.h>
//#pragma comment(lib, "strmiids.lib") 


int PlayMovie(movInfo *mvRec, myFInfo *t2, char isJustOpen) {
//	BOOL isMP3;
	int len;
	char *ext;
	unCH *t;
	CString fname;
	CWaitCursor wait;
	char MovieFileName[_MAX_PATH+FILENAME_MAX];
	unCH wDirPathName[FNSize];
	extern long gCurF;

	gCurF = 0L;
	if (PlayingContMovie != 1 && PlayingContMovie != '\003')
		CopyAndFreeMovieData(TRUE);
	strcpy(MovieFileName, mvRec->rMovieFile);
	uS.lowercasestr(MovieFileName, &dFnt, FALSE);
	ext = strrchr(MovieFileName,'.');
	if (ext == NULL) {
		strcat(MovieFileName, ".mov");
		ext = strrchr(MovieFileName,'.');
	}
//	else if (uS.mStricmp(ext, ".mp3") == 0)
//		isMP3 = true;
//	else
//		isMP3 = false;
	u_strcpy(wDirPathName, MovieFileName, FNSize);
	fname = wDirPathName;
	long mode=isPlayS;

//(strcmp(ext, ".mpg") == 0  ||
// strcmp(ext, ".mpeg") == 0 ||
// strcmp(ext, ".aiff") == 0 ||
// strcmp(ext, ".aif") == 0  ||
// strcmp(ext, ".avi") == 0  ||
// strcmp(ext, ".mov") == 0  ||
// strcmp(ext, ".mp4") == 0  ||
// strcmp(ext, ".wmv") == 0)

	if (ext != NULL && true /*(strcmp(ext, ".dat") == 0)*/)
//	if (ext != NULL && strcmp(ext, ".mov") != 0) // .mov played by QuickTime
//	if (ext != NULL && (strcmp(ext, ".dat") == 0)) // Original
	{
		if (!PlayingContMovie || (PBC.walk && PBC.enable))
			;
		else if (PlayingContMovie == '\003') {
			FakeSelectWholeTier(F5Option == EVERY_LINE);
			DrawFakeHilight(1);
		}

/* // NO QT
		if (MovDlg != NULL) {
			if (MovDlg->qt != NULL && !(MovDlg->qt)->IsQTMovieDone()) { //playing
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
			}
			MovDlg->doErase = false;
			MovDlg->OnCancel();
			MovDlg = NULL;
		}
*/
		if (MpegDlg != NULL) {
			if (MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpControler != NULL) { //playing
				(MpegDlg->mpeg)->StopMpegMovie();
				(MpegDlg->mpeg)->isPlaying = 0;
			}
			if (fname.Find('\\') != -1)
				*wDirPathName = EOS;
			else
				u_strcpy(wDirPathName, wd_dir, FNSize);
			strcat(wDirPathName, fname);
			if (wpathcmp(wDirPathName, MpegDlg->pathname)) {
				MpegDlg->doErase = false;
				MpegDlg->OnCancel();
				MpegDlg = NULL;
			}
		}
		if (MpegDlg == NULL) {
			MpegDlg = new CMpegDlg(AfxGetApp()->m_pMainWnd);
			if (fname.Find('\\') != -1)
				MpegDlg->pathname = cl_T("");
			else {
				u_strcpy(wDirPathName, wd_dir, FNSize);
				len = strlen(wd_dir) - 1;
				if (wd_dir[len] != PATHDELIMCHR)
					strcat(wDirPathName, PATHDELIMSTR);
				MpegDlg->pathname = wDirPathName;
			}
			MpegDlg->pathname = MpegDlg->pathname + fname;
			MpegDlg->Create(IDD_MPEG_DIALOG);
			if (!PlayingContMovie || (PBC.walk && PBC.enable))
				AfxGetApp()->m_pMainWnd->SetActiveWindow();
//			dur = (MpegDlg->mpeg)->GetMpegMovieDuration();
		} else {
			if (!PlayingContMovie || (PBC.walk && PBC.enable))
				;
			else
				MpegDlg->SetActiveWindow();
//			dur = (MpegDlg->mpeg)->GetMpegMovieDuration();
//			MpegDlg->m_to = dur;
		}
		if (MpegDlg->mpeg != NULL) {
			(MpegDlg->mpeg)->isPlaying = 0;
			(MpegDlg->mpeg)->m_wmpControler->pause();
		}
		MpegDlg->mvRec = mvRec;
		if (isJustOpen)
			MpegDlg->isJustOpen = TRUE;
		else
			MpegDlg->isJustOpen = FALSE;
		if ((t = strrchr(wDirPathName, '\\')) == NULL)
			t = wDirPathName;
		else
			t++;
		MpegDlg->SetWindowText(t);
		MpegDlg->m_isUserResize = TRUE;
		MpegDlg->MoviePlayLoopCnt = 1;
#ifdef GETMEDIAINFO
		long   lDur;
		double dDur;

		lDur = DisplayInfo(MpegDlg->pathname);
		if (lDur > 0L) {
			dDur = (double)lDur;
			dDur = dDur / 1000;
			MpegDlg->mpeg->SetRawDuration(dDur);
			MpegDlg->mpeg->isMovieDurationSet = TRUE;
			MpegDlg->mpeg->setVolume(100);
		}
#endif //GETMEDIAINFO
		MpegDlg->textDoc = GlobalDoc;
		MpegDlg->textDC = NULL; // GlobalDC;
//		if (isMP3 == true) {
//			unCH st[1024];

//			MpegDlg->m_from -= 843;
//			MpegDlg->m_to -= 1549; // 2021-05-07
//			if (mvRec->MBeg > 843) mvRec->MBeg -= 843;
//			if (mvRec->MEnd > 843) mvRec->MEnd -= 843;

//			MCIERROR res;
//			u_strcpy(st, "open \"C:\\USR\\PC-MP3\\cwsr059_42-m.mp3\" type mpegvideo alias mp3", 1023);
//			res = mciSendString(st, NULL, 0, NULL);
//			u_strcpy(st, "play mp3 from 22004 to 24326", 1023);
//			res = mciSendString(st, NULL, 0, NULL);

/*

			// Visual C++ example
			// For IID_IGraphBuilder, IID_IMediaControl, IID_IMediaEvent

			LONGLONG beg, end;
			// Obviously change this to point to a valid mp3 file.
			const wchar_t* filePath = L"C:\\USR\\PC-MP3\\cwsr059_42-m.mp3";

			IGraphBuilder *pGraph = NULL;
			IMediaControl *pControl = NULL;
			IMediaSeeking *pSeek = NULL;
			IMediaEvent   *pEvent = NULL;

			// Initialize the COM library.
			HRESULT hr = ::CoInitialize(NULL);
			if (FAILED(hr))
			{
				::printf("ERROR - Could not initialize COM library");
				return 0;
			}

			// Create the filter graph manager and query for interfaces.
			hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
				IID_IGraphBuilder, (void **)&pGraph);
			if (FAILED(hr))
			{
				::printf("ERROR - Could not create the Filter Graph Manager.");
				return 0;
			}

			hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
			hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeek);
			hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

			// Build the graph.
			hr = pGraph->RenderFile(filePath, NULL);
			if (SUCCEEDED(hr))
			{
				beg = mvRec->MBeg * 10000;
				end = mvRec->MEnd * 10000;
				const GUID pFormat = TIME_FORMAT_MEDIA_TIME;
				hr = pSeek->SetTimeFormat(&pFormat);
				hr = pSeek->SetPositions(&beg, AM_SEEKING_AbsolutePositioning, &end, AM_SEEKING_AbsolutePositioning);
				if (SUCCEEDED(hr))
				{
					// Run the graph.
					hr = pControl->Run();
					if (SUCCEEDED(hr))
					{
						// Wait for completion.
						long evCode;
						pEvent->WaitForCompletion(INFINITE, &evCode);

						// Note: Do not use INFINITE in a real application, because it
						// can block indefinitely.
					}
				}
			}
			// Clean up in reverse order.
			pEvent->Release();
			pControl->Release();
			pGraph->Release();
			::CoUninitialize();
*/
//		}
		MpegDlg->Play();
	} else {
		PlayingContMovie = FALSE;
		do_warning("Unknown media type.", 0);
//		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW_MOVIE, NULL);
	}
	return(1);
}

void stopMoviePlaying(void) {
	if (MpegDlg != NULL && MpegDlg->mpeg != NULL) {
		if (!(MpegDlg->mpeg)->IsMpegMovieDone()) {
			(MpegDlg->mpeg)->StopMpegMovie();
		}
		(MpegDlg->mpeg)->isPlaying = 0;
		PBC.isPC = 0;
		PBC.walk = 0;
	}
/* // NO QT
	if (MovDlg != NULL) {
		if (!(MovDlg->qt)->IsQTMovieDone()) {
			StopMovie((MovDlg->qt)->theMovie);
		}
		(MovDlg->qt)->isPlaying = 0;
		PBC.isPC = 0;
		PBC.walk = 0;
	}
*/
}

void stopMovieIfPlaying(void) {
	if (MpegDlg != NULL) {
		if (!(MpegDlg->mpeg)->IsMpegMovieDone()) {
			(MpegDlg->mpeg)->StopMpegMovie();
		}
		(MpegDlg->mpeg)->isPlaying = 0;
		MpegDlg->DestroyWindow();
		MpegDlg = NULL;
	}
/* // NO QT
	if (MovDlg != NULL) {
		if (!(MovDlg->qt)->IsQTMovieDone()) {
			StopMovie((MovDlg->qt)->theMovie);
		}
		(MovDlg->qt)->isPlaying = 0;
		MovDlg->DestroyWindow();
		MovDlg = NULL;
	}
*/
}


void replayMovieWithOffset(int offset) {
	long		t;

	if (global_df == NULL)
		return;
	if (MpegDlg != NULL) {
		if (offset < 0) {
			t = (MpegDlg->mpeg)->GetCurTime() - PBC.step_length;
			if (t < 0L)
				t = 0L;
			global_df->MvTr.MBeg = t;
			if (!(MpegDlg->mpeg)->IsMpegMovieDone()) {
				(MpegDlg->mpeg)->StopMpegMovie();
				(MpegDlg->mpeg)->isPlaying = 0;
			}
			if (global_df->MvTr.MBeg >= (MpegDlg->mpeg)->GetMpegMovieDuration()) {
				PBC.isPC = 0;
				PBC.walk = 0;
			} else {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				MpegDlg->m_from = global_df->MvTr.MBeg;
				MpegDlg->m_pos = MpegDlg->m_from;
				MpegDlg->m_to = global_df->MvTr.MEnd;
				MpegDlg->Play();
			}
		} else if (offset > 0) {
			t = (MpegDlg->mpeg)->GetCurTime() + PBC.step_length;
			if (t < 0L)
				t = 0L;
			global_df->MvTr.MBeg = t;
			if (!(MpegDlg->mpeg)->IsMpegMovieDone()) {
				(MpegDlg->mpeg)->StopMpegMovie();
				(MpegDlg->mpeg)->isPlaying = 0;
			}
			if (global_df->MvTr.MBeg >= (MpegDlg->mpeg)->GetMpegMovieDuration()) {
				PBC.isPC = 0;
				PBC.walk = 0;
			} else {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				MpegDlg->m_from = global_df->MvTr.MBeg;
				MpegDlg->m_pos = MpegDlg->m_from;
				MpegDlg->m_to = global_df->MvTr.MEnd;
				MpegDlg->Play();
			}
		}
	}
/* // NO QT
	else if (MovDlg != NULL) {
		if (offset < 0) {
			t = (MovDlg->qt)->GetCurTime() - PBC.step_length;
			if (t < 0L)
				t = 0L;
			global_df->MvTr.MBeg = t;
			if (!(MovDlg->qt)->IsQTMovieDone()) {
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
			}
			if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
				PBC.isPC = 0;
				PBC.walk = 0;
			} else {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				MovDlg->qt->Movie_PreLoad(global_df->MvTr.MBeg);
				MovDlg->m_from=global_df->MvTr.MBeg;
				MovDlg->m_pos = MovDlg->m_from;
				MovDlg->m_to=global_df->MvTr.MEnd;
				MovDlg->Play();
			}
		} else if (offset > 0) {
			t = (MovDlg->qt)->GetCurTime() + PBC.step_length;
			if (t < 0L)
				t = 0L;
			global_df->MvTr.MBeg = t;
			if (!(MovDlg->qt)->IsQTMovieDone()) {
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
			}
			if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
				PBC.isPC = 0;
				PBC.walk = 0;
			} else {
				global_df->MvTr.MEnd = global_df->MvTr.MBeg + PBC.step_length;
				MovDlg->qt->Movie_PreLoad(global_df->MvTr.MBeg);
				MovDlg->m_from=global_df->MvTr.MBeg;
				MovDlg->m_pos = MovDlg->m_from;
				MovDlg->m_to=global_df->MvTr.MEnd;
				MovDlg->Play();
			}
		}
	}
*/
}

void getFileType(FNType *fn, unsigned long *type) {
	FNType *s;

	s = strrchr(fn,'.');
	if (s == NULL) {
		*type = 0L;
		return;
	}
	if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L))
		*type = 'AIFF';
	else if (!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L))
		*type = 'WAVE';
	else if (!uS.FNTypeicmp(s, ".mp3", 0L)) {
		*type = 'MP3!';
	} else if (!uS.FNTypeicmp(s, ".mov", 0L))
		*type = 'MooV';
	else if (!uS.FNTypeicmp(s, ".mp4", 0L))
		*type = 'MooV'; // 'mpg4'
	else if (!uS.FNTypeicmp(s, ".m4v", 0L))
		*type = 'MooV'; // 'm4v'
	else if (!uS.FNTypeicmp(s, ".flv", 0L))
		*type = 'MooV'; // 'flv ';
	else if (!uS.FNTypeicmp(s, ".dv", 0L))
		*type = 'MooV'; // 'dvc!';
	else if (!uS.FNTypeicmp(s, ".dat", 0L)) // vcd file
		*type = 'MPG ';
	else if (!uS.FNTypeicmp(s, ".mpeg", 0L)  || !uS.FNTypeicmp(s, ".mpg", 0L))
		*type = 'MPG ';
	else if (!uS.FNTypeicmp(s, ".avi", 0L))
		*type = 'MooV'; // 'AVI ';
	else if (!uS.FNTypeicmp(s, ".wmv", 0L))
		*type = 'MooV'; // 'wmv '
	else if (!uS.FNTypeicmp(s, ".txt", 0L) || !uS.FNTypeicmp(s, ".cut", 0L) ||
			 !uS.FNTypeicmp(s, ".cha", 0L)  || !uS.FNTypeicmp(s, ".ca", 0L))
		*type = 'TEXT';
	else if (!uS.FNTypeicmp(s, ".pict", 0L))
		*type = 'PICT';
	else if (!uS.FNTypeicmp(s, ".jpg", 0L)  || !uS.FNTypeicmp(s, ".jpeg", 0L))
		*type = 'JPEG';
	else if (!uS.FNTypeicmp(s, ".gif", 0L))
		*type = 'GIFf';
	else
		*type = 0L;
}

void CantFindMedia(char *err_message, unCH *fname, char isOld) {
	FNType *c;
	FNType fileName[FNSize];
	FNType tFileName[FNSize];

	u_strcpy(tFileName, fname, FNSize);
	if (isRefEQZero(global_df->fileName)) {
		sprintf(global_df->err_message, "+If this is a newfile, then please save it to the disk before you try again. Locate the media file \"%s\" and move it to the folder which has this transcript file or to the \"media\" folder below it.", tFileName);
		return;
	}

	strcpy(fileName, global_df->fileName);
	c = strrchr(fileName, PATHDELIMCHR);
	if (c == NULL) {
		if (isOld)
			sprintf(global_df->err_message, "+OLD BULLET FORMAT. Please locate the media file \"%s\" and move it into the folder which has your transcript or the \"media\" folder below the transcript and try again.", tFileName);
		else
			sprintf(global_df->err_message, "+Please locate the media file \"%s\" and move it into the folder which has your transcript or the \"media\" folder below the transcript and try again.", tFileName);
		return;
	}
	*(c+1) = EOS;

	if (isOld)
		sprintf(global_df->err_message, "+OLD BULLET FORMAT. Please locate the media file \"%s\" and move it to the \"%s\" folder or the \"media\" folder inside of it and try again.", tFileName, fileName);
	else
		sprintf(global_df->err_message, "+Please locate the media file \"%s\" and move it to the \"%s\" folder or the \"media\" folder inside of it and try again.", tFileName, fileName);
	if (strlen(global_df->err_message) >= 300) {
		if (isOld)
			sprintf(global_df->err_message, "+OLD BULLET FORMAT. Please locate the media file \"%s\" and move it into the folder which has your transcript or the \"media\" folder below the transcript and try again.", tFileName);
		else
			sprintf(global_df->err_message, "+Please locate the media file \"%s\" and move it into the folder which has your transcript or the \"media\" folder below the transcript and try again.", tFileName);
	}
}

static char isRightMediaFolder(FNType *sfFile, char *err_message) {
	FNType fileName1[FNSize], fileName2[FNSize];

	if (isRefEQZero(global_df->fileName))
		return(TRUE);

	extractPath(fileName2, sfFile);	

	extractPath(fileName1, global_df->fileName);
	if (pathcmp(fileName1, fileName2) == 0)
		return(TRUE);

	addFilename2Path(fileName1, "media");
	if (pathcmp(fileName1, fileName2) == 0)
		return(TRUE);


	if (!isRefEQZero(wd_dir)) {
		strcpy(fileName1, wd_dir);
		if (pathcmp(fileName1, fileName2) == 0)
			return(TRUE);
	}

	if (!isRefEQZero(lib_dir)) {
		strcpy(fileName1, lib_dir);
		if (pathcmp(fileName1, fileName2) == 0)
			return(TRUE);
	}

	strcpy(fileName2, sfFile);
	extractPath(fileName1, global_df->fileName);
	sprintf(global_df->err_message, "+Please locate the media file \"%s\" and move it to the \"%s\" folder or the \"media\" folder inside of it and try again.", fileName2, fileName1);
	if (strlen(global_df->err_message) >= 300)
		sprintf(global_df->err_message, "+Please locate the media file \"%s\" and move it into the folder which has your transcript or the \"media\" folder below the transcript and try again.", fileName2);
	return(FALSE);
}
// select media dialog box BEGIN
/////////////////////////////////////////////////////////////////////////////
// CSelectMediaFile dialog

class CSelectMediaFile : public CDialog
{
// Construction
public:
	CSelectMediaFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_OPENMEDIAFILE };
	//{{AFX_DATA(CSelectMediaFile)
	CListBox	m_SelectMediaFileCTRL;
	FNType		m_File[FNSize];
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMediaFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMediaFile)
	afx_msg void OnDblclkFileList();
	afx_msg void OnSelchangeFileList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CSelectMediaFile dialog

CSelectMediaFile::CSelectMediaFile(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectMediaFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMediaFile)
	//}}AFX_DATA_INIT
}


void CSelectMediaFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMediaFile)
	DDX_Control(pDX, IDC_LIST_OF_MEDIA_FILES, m_SelectMediaFileCTRL);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectMediaFile, CDialog)
	//{{AFX_MSG_MAP(CSelectMediaFile)
	ON_LBN_DBLCLK(IDC_LIST_OF_MEDIA_FILES, OnDblclkFileList)
	ON_LBN_SELCHANGE(IDC_LIST_OF_MEDIA_FILES, OnSelchangeFileList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMediaFile message handlers
static char strictMediaFileMatch(FNType *sTemp) {
	char *s;
	
	s = strrchr(sTemp, '.');
	if (s != NULL) {
		if (!uS.FNTypeicmp(s, ".aiff", 0L) || !uS.FNTypeicmp(s, ".aif", 0L)  ||
			!uS.FNTypeicmp(s, ".wav", 0L)  || !uS.FNTypeicmp(s, ".wave", 0L) ||
			!uS.FNTypeicmp(s, ".mp3", 0L)  ||
			!uS.FNTypeicmp(s, ".mov", 0L)  || !uS.FNTypeicmp(s, ".mp4", 0L)  ||
			!uS.FNTypeicmp(s, ".m4v", 0L)  || !uS.FNTypeicmp(s, ".flv", 0L)  ||
			!uS.FNTypeicmp(s, ".mpeg", 0L) || !uS.FNTypeicmp(s, ".mpg", 0L)  ||
			!uS.FNTypeicmp(s, ".avi", 0L)  || !uS.FNTypeicmp(s, ".dv", 0L)   || 
			!uS.FNTypeicmp(s, ".wmv", 0L)   || !uS.FNTypeicmp(s, ".dat", 0L) // vcd file
			) {
			return(TRUE); /* show in box */
		}
	}
	return(FALSE); /* do not show */
}

BOOL CSelectMediaFile::OnInitDialog() {
	int i;
	char isMediaFolderFound;
	FNType sTemp[FNSize+1];

	CDialog::OnInitDialog();

	SetNewVol(m_File);
	isMediaFolderFound = FALSE;
	i = 1;
	while ((i=Get_Dir(sTemp, i)) != 0) {
		if (sTemp[0] != '.') {
			if (uS.mStricmp(sTemp, "media") == 0)
				isMediaFolderFound = TRUE;
		}
	}
	i = 1;
	while ((i=Get_File(sTemp, i)) != 0) {
		if (sTemp[0] != '.') {
			if (strictMediaFileMatch(sTemp)) {
				u_strcpy(templineW, sTemp, UTTLINELEN);
				m_SelectMediaFileCTRL.AddString(templineW);
			}
		}
	}
	if (isMediaFolderFound) {
		addFilename2Path(m_File, "media");
		strcpy(sTemp, "media\\");
		SetNewVol(m_File);
		i = 1;
		while ((i=Get_File(sTemp+6, i)) != 0) {
			if (sTemp[6] != '.') {
				if (strictMediaFileMatch(sTemp+6)) {
					u_strcpy(templineW, sTemp, UTTLINELEN);
					m_SelectMediaFileCTRL.AddString(templineW);
				}
			}
		}
	}
	m_File[0] = EOS;
	return TRUE;
}

void CSelectMediaFile::OnDblclkFileList() 
{
	int i;

	i = m_SelectMediaFileCTRL.GetCurSel();
	if (i != LB_ERR) {
		m_SelectMediaFileCTRL.GetText(i, templineW);
		u_strcpy(m_File, templineW, UTTLINELEN);
	}
	UpdateData(FALSE);
	EndDialog(IDOK);
}

void CSelectMediaFile::OnSelchangeFileList() 
{
	int i;

	i = m_SelectMediaFileCTRL.GetCurSel();
	if (i != LB_ERR) {
		m_SelectMediaFileCTRL.GetText(i, templineW);
		u_strcpy(m_File, templineW, UTTLINELEN);
	}
	UpdateData(FALSE);
}

char LocateMediaFile(FNType *fname) {
	CSelectMediaFile dlg;

	DrawCursor(1);
	strcpy(dlg.m_File, fname);
	fname[0] = EOS;
	if (dlg.DoModal() != IDOK) {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);
		return(FALSE);
	}
	DrawCursor(0);
	strcpy(fname, dlg.m_File);
	if (fname[0] == EOS)
		return(FALSE);
	strcpy(global_df->err_message, DASHES);		
	DrawCursor(0);
	return(TRUE);
}
// select media dialog box END

char GetNewMediaFile(char isCheckError, char isAllMedia) {
    OPENFILENAME	ofn;
	unsigned long	SFileType;
    unCH			szFile[FILENAME_MAX];
    unCH			*szFilter;
	unCH			wPrompt[256];
	FNType			*c, retFileName[FNSize];

	if (isAllMedia == isAllType) {
		strcpy(wPrompt, "Please locate movie, sound, picture or text file");
		szFilter = _T("Media Files (*.aif,*.aiff,*.wav,*.mp3,*.mpg,*.mpeg,*.dat,*.mov,*.mp4,*.m4v,*.wmv)\0*.aif;*.aiff;*.wav;*.mp3;*.mpg;*.mpeg;*.dat;*.mov;*.mp4;*.m4v;*.wmv\0Picture and text (*.gif,*.jpg,*.jpeg,*.pict,*.txt,*.cut,*.cha,*.ca)\0*.gif;*.jpg;*.jpeg;*.pict;*.txt;*.cut;*.cha;*.ca\0All files (*.*)\0*.*\0\0");
	} else if (isAllMedia == isPictText) {
		strcpy(wPrompt, "Please locate TEXT or JPEG file ONLY");
		szFilter = _T("Sound Files (*.txt,*.cut,*.cdc,*.pict,*.jpg,*.jpeg,*.gif)\0*.txt;*.cut;*.cdc;*.pict;*.jpg;*.jpeg;*.gif\0All files (*.*)\0*.*\0\0");
	} else if (isAllMedia == isAudio) {
		strcpy(wPrompt, "Please locate sound file ONLY");
		szFilter = _T("Sound Files (*.aif,*.aiff,*.wav,*.mp3)\0*.aif;*.aiff;*.wav;*.mp3\0All files (*.*)\0*.*\0\0");
	} else {
		strcpy(wPrompt, "Please locate movie or sound file ONLY");
		szFilter = _T("Media Files (*.aif,*.aiff,*.wav,*.mp3,*.mpg,*.mpeg,*.dat,*.mov,*.mp4,*.m4v,*.wmv)\0*.aif;*.aiff;*.wav;*.mp3;*.mpg;*.mpeg;*.dat;*.mov;*.mp4;*.m4v;*.wmv\0All files (*.*)\0*.*\0\0");
	}

	szFile[0] = EOS;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL; // u_strcpy(wDirPathName, wd_dir, FNSize);
	ofn.lpstrTitle = wPrompt;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetOpenFileName(&ofn) == 0) {
		if (global_df->SnTr.SoundFile[0] != EOS) {
			global_df->SnTr.SoundFile[0] = EOS;
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
			}
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);		
		return(0);
	}
	DrawCursor(0);
	u_strcpy(retFileName, szFile, FNSize);
	getFileType(retFileName, &SFileType);
	if (!isRightMediaFolder(retFileName, global_df->err_message)) {
		if (global_df->SnTr.SoundFile[0] != EOS) {
			global_df->SnTr.SoundFile[0] = EOS;
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
			}
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
		DrawCursor(0);
	} else if (SFileType == 'PICT' || SFileType == 'JPEG' || SFileType == 'GIFf') {
		strcpy(global_df->PcTr.pictFName,retFileName);
		return(isPict);
	} else if (SFileType == 'TEXT') {
		strcpy(global_df->TxTr.textFName,retFileName);
		return(isText);
	} else if (SFileType == 'MooV' || SFileType == 'MPG ') {
		if (isCheckError && !strcmp(retFileName, global_df->MvTr.rMovieFile)) {
			strcpy(global_df->err_message, "+Movie marker is not found at cursor position. Please move text cursor next to the bullet.");
			return(0);
		}
		DrawSoundCursor(0);
		global_df->MvTr.MBeg = 0L;
		global_df->MvTr.MEnd = 0L;
		strcpy(global_df->MvTr.rMovieFile, retFileName);
		if ((c=strrchr(retFileName, PATHDELIMCHR)) != NULL)
			strcpy(retFileName, c+1);
		if ((c=strrchr(retFileName,'.')) != NULL)
			*c = EOS;
		u_strcpy(global_df->MvTr.MovieFile, retFileName, FILENAME_MAX);
		return(isVideo);
	} else if (SFileType == 'AIFF' || SFileType == 'WAVE' || SFileType == 'MP3!') {
		if (isCheckError && global_df->SnTr.SoundFile[0] != EOS && !strcmp(retFileName, global_df->SnTr.rSoundFile)) {
			strcpy(global_df->err_message, "+Sound marker is not found at cursor position. If you are planning to link sound to text then please use Sonic mode \"ESC-0\"");
			return(0);
		}
		if (global_df->SnTr.isMP3) {
			global_df->SnTr.isMP3 = FALSE;
		} else if (global_df->SnTr.SoundFPtr != 0) {
			fclose(global_df->SnTr.SoundFPtr);
			global_df->SnTr.SoundFile[0] = EOS;
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}

		DrawSoundCursor(0);
		global_df->SnTr.BegF = global_df->SnTr.EndF = 0L;
		if (SFileType == 'MP3!') {
			extractFileName(FileName2, retFileName);
			c = strrchr(FileName2, '.');
			if (c != NULL && uS.mStricmp(c, ".mp3") == 0)
				*c = EOS;
			strcpy(FileName1, retFileName);
			c = strrchr(FileName1, '.');
			if (c != NULL && uS.mStricmp(c, ".mp3") == 0)
				*c = EOS;
			strcat(FileName1, ".wav");
			if (global_df->SnTr.SoundFPtr != NULL)
				fclose(global_df->SnTr.SoundFPtr);
			strcpy(global_df->MvTr.rMovieFile, retFileName);
			u_strcpy(global_df->MvTr.MovieFile, FileName2, FILENAME_MAX);
			PlayMovie(&global_df->MvTr, global_df, TRUE);
			SaveSoundMovieAsWAVEFile(retFileName, FileName1);
			if (MpegDlg != NULL)
				MpegDlg->OnCancel();
			if ((global_df->SnTr.SoundFPtr = fopen(FileName1, "rb")) == NULL) {
				return(0);
			}
			global_df->MvTr.rMovieFile[0] = EOS;
			global_df->MvTr.MovieFile[0] = EOS;
			u_strcpy(global_df->SnTr.SoundFile, FileName2, FILENAME_MAX);
			strcpy(retFileName, FileName1);
		} else if ((global_df->SnTr.SoundFPtr=fopen(retFileName, "rb")) == NULL) {
			sprintf(global_df->err_message, "+Can't open sound file: %s. Perhaps it is already opened by other application or in another window.", retFileName);
			global_df->SnTr.SoundFile[0] = EOS;
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
			return(0);
		}
		global_df->SnTr.SFileType = SFileType;
		strcpy(global_df->SnTr.rSoundFile, retFileName);
		if (!CheckRateChan(&global_df->SnTr, global_df->err_message)) {
			global_df->SnTr.SoundFile[0] = EOS;
			global_df->SnTr.rSoundFile[0] = EOS;
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
			} else {
				fclose(global_df->SnTr.SoundFPtr);
				global_df->SnTr.SoundFPtr = 0;
				if (global_df->SoundWin)
					DisposeOfSoundWin();
			}
			return(0);
		}
		if ((c=strrchr(retFileName, PATHDELIMCHR)) != NULL)
			strcpy(retFileName, c+1);
		if ((c=strrchr(retFileName,'.')) != NULL)
			*c = EOS;
		u_strcpy(global_df->SnTr.SoundFile, retFileName, FILENAME_MAX);
		ResetUndos();
		global_df->SnTr.WBegFM = 0L;
		global_df->SnTr.WEndFM = 0L;
		if (global_df->SoundWin)
			DisplayEndF(TRUE);
		return(isAudio);
	} else {
		if (global_df->SnTr.SoundFile[0] != EOS) {
			global_df->SnTr.SoundFile[0] = EOS;
			if (global_df->SnTr.isMP3 == TRUE) {
				global_df->SnTr.isMP3 = FALSE;
			} else if (global_df->SnTr.SoundFPtr != 0) {
				fclose(global_df->SnTr.SoundFPtr);
			}
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		}
	}
	return(0);
}

char GetNewPictFile(FNType *fname) {
	int				len;
    OPENFILENAME	ofn;
	FILE			*fptr;
	FNType			fileName[FNSize];
    unCH			tfname[FILENAME_MAX];
    unCH			szFile[FILENAME_MAX];
    unCH			*szFilter;
	unCH			wDirPathName[FNSize];

	szFilter = _T("Picture Files (*.gif,*.jpeg,*.jpg)\0*.gif;*.jpeg;*.jpg\0All files (*.*)\0*.*\0\0");
	szFile[0] = EOS;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
	if (*fname != EOS) {
		strcpy(tfname, "Please locate picture file: ");
		len = strlen(tfname);
		u_strcpy(tfname+len, fname, FILENAME_MAX-len);
	} else {
		strcpy(tfname, "Please locate picture file");
	}
    ofn.lpstrTitle = tfname;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetOpenFileName(&ofn) == 0) {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);		
		return(0);
	}

	DrawCursor(0);
	DrawSoundCursor(0);
	u_strcpy(fileName, szFile, FNSize);
	if ((fptr=fopen(fileName,"rb")) == NULL) {
		sprintf(global_df->err_message, "+Can't open picture file: %s.", fileName);
		return(0);
	}
	fclose(fptr);
	strcpy(global_df->PcTr.pictFName,fileName);
	return(1);
}

char GetNewTextFile(FNType *fname) {
	int				len;
    OPENFILENAME	ofn;
	FILE			*fptr;
	FNType			fileName[FNSize];
    unCH			tfname[FILENAME_MAX];
    unCH			szFile[FILENAME_MAX];
    unCH			*szFilter;
	unCH			wDirPathName[FNSize];

	szFilter = _T("Text Files (*.txt,*.cut,*.cha,*.ca)\0*.txt;*.cut;*.cha;*.ca\0All files (*.*)\0*.*\0\0");
	szFile[0] = EOS;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
	if (*fname != EOS) {
		strcpy(tfname, "Please locate text file: ");
		len = strlen(tfname);
		u_strcpy(tfname+len, fname, FILENAME_MAX-len);
	} else {
		strcpy(tfname, "Please locate text file");
	}
    ofn.lpstrTitle = tfname;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetOpenFileName(&ofn) == 0) {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);		
		return(0);
	}

	DrawCursor(0);
	DrawSoundCursor(0);
	u_strcpy(fileName, szFile, FNSize);
	if ((fptr=fopen(fileName,"rb")) == NULL) {
		sprintf(global_df->err_message, "+Can't open text file: %s.", fileName);
		return(0);
	}
	fclose(fptr);
	strcpy(global_df->TxTr.textFName,fileName);
	return(1);
}

static char ReadSound8bit(int col, int cm, Size cur) {
	int row, hp1, lp1, hp2, lp2, scale;
	short t;
	double d;
	Size i, count, scale_cnt;

	if (cm == 0) {
		GetSoundWinDim(&row, &cm);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	} else {
		GetSoundWinDim(&row, &hp1);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)hp1 * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	}
	if (cur == 0)
		cur = global_df->SnTr.WBegF;
	scale = ((0xFF / global_df->mainVScale) / row) + 1;
	
	global_df->SnTr.WBegFM = global_df->SnTr.WBegF;
	global_df->SnTr.WEndFM = global_df->SnTr.WEndF;
	scale_cnt = 0L;
	hp1 = 0; lp1 = hp1 - 1;
	hp2 = 0; lp2 = hp2 - 1;

	wdrawcontr(global_df->SoundWin, TRUE);
	while (cur < global_df->SnTr.WEndF && cur < global_df->SnTr.SoundFileSize && col < cm) {
		count = ((Size)SNDBUFSIZE < global_df->SnTr.WEndF-cur ? (Size)SNDBUFSIZE : global_df->SnTr.WEndF-cur);
		if (cur+count >= global_df->SnTr.SoundFileSize)
			count = global_df->SnTr.SoundFileSize - cur;
		if (fseek(global_df->SnTr.SoundFPtr, cur+global_df->SnTr.AIFFOffset, SEEK_SET) != 0)
			return(0);
		if (fread(global_df->SnTr.SWaveBuf, CSIZE, count, global_df->SnTr.SoundFPtr) != count)
			return(0);
		if (global_df->SnTr.SFileType == 'WAVE' && !global_df->SnTr.DDsound) {
			for (i=0; i < count; i++)
				global_df->SnTr.SWaveBuf[i] -= (char)128;
		}
		for (i=0; i < count && col < cm; i++) {
			if (global_df->SnTr.SNDformat == 0L) 
				row = (int)(global_df->SnTr.SWaveBuf[i]) / scale;
			else if (global_df->SnTr.SNDformat == WAVE_FORMAT_MULAW) {
				t = Mulaw2Lin((unsigned char)global_df->SnTr.SWaveBuf[i]);
				d = (double)t;
				d = d * 0.38909912109375 / 100.000000000000;
				t = (short)d;
				row = t / scale;
			} else if (global_df->SnTr.SNDformat == WAVE_FORMAT_ALAW) {
				t = Alaw2Lin((unsigned char)global_df->SnTr.SWaveBuf[i]);
				d = (double)t;
				d = d * 0.38909912109375 / 100.000000000000;
				t = (short)d;
				row = t / scale;
			}	
			if (global_df->SnTr.SNDchan == 1 || doMixedSTWave) {
				if (row > hp1) hp1 = row;
				if (row < lp1) lp1 = row;
			} else {
				if ((i % 2) == 0) {
					if (row > hp1) hp1 = row;
					if (row < lp1) lp1 = row;
				} else {
					if (row > hp2) hp2 = row;
					if (row < lp2) lp2 = row;
				}
			}
			scale_cnt++;
			if (scale_cnt == global_df->scale_row) {
				if (global_df->TopP1 && global_df->BotP1) {
					global_df->TopP1[col] = hp1;
					global_df->BotP1[col] = lp1;
				}
				if (global_df->TopP2 && global_df->BotP2) {
					global_df->TopP2[col] = hp2;
					global_df->BotP2[col] = lp2;
				}
				wdrawdot(global_df->SoundWin, hp1, lp1, hp2, lp2, col);
				col++;
				scale_cnt = 0L;
 				hp1 = 0;
				lp1 = hp1-1;
 				hp2 = 0;
				lp2 = hp2-1;
			}
		}
		cur += (Size)i;
	}
	if (global_df->TopP1 && global_df->BotP1 && cm != 1)
		global_df->TopP1[col] = -1;
	return(1);
}

static char ReadSound16bit(int col, int cm, Size cur) {
	int   row, hp1, lp1, hp2, lp2, scale;
	short *tbuf;
	Size  i, count, scale_cnt;

	if (cm == 0) {
		GetSoundWinDim(&row, &cm);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	} else {
		GetSoundWinDim(&row, &hp1);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)hp1 * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	}
	if (cur == 0)
		cur = global_df->SnTr.WBegF;
	scale = (int)((0xFFFFL / (Size)global_df->mainVScale) / (Size)row) + 1;
	global_df->SnTr.WBegFM = global_df->SnTr.WBegF;
	global_df->SnTr.WEndFM = global_df->SnTr.WEndF;
	scale_cnt = 0L;
	hp1 = 0; lp1 = hp1 - 1;
	hp2 = 0; lp2 = hp2 - 1;

	wdrawcontr(global_df->SoundWin, TRUE);
	while (cur < global_df->SnTr.WEndF && cur < global_df->SnTr.SoundFileSize && col < cm) {
		count = ((Size)SNDBUFSIZE < global_df->SnTr.WEndF-cur ? (Size)SNDBUFSIZE : global_df->SnTr.WEndF-cur);
		if (cur+count >= global_df->SnTr.SoundFileSize) 
			count = global_df->SnTr.SoundFileSize - cur;
		if (fseek(global_df->SnTr.SoundFPtr, cur+global_df->SnTr.AIFFOffset, SEEK_SET) != 0)
			return(0);
		if (fread(global_df->SnTr.SWaveBuf, CSIZE, count, global_df->SnTr.SoundFPtr) != count)
			return(0);
		tbuf = (short *)global_df->SnTr.SWaveBuf;
		count /= 2;
		if (global_df->SnTr.SFileType == 'AIFF' && global_df->SnTr.DDsound) {
			for (i=0; i < count; i++) {
				flipShort(&tbuf[i]);
			}
		}
		if (global_df->SnTr.SFileType != 'WAVE' && global_df->SnTr.SFileType != 'AIFF' && 
								!global_df->SnTr.DDsound) {
			for (i=0; i < count; i++)
				tbuf[i] -= (short)32768;
		}
		for (i=0; i < count && col < cm; i++) {
			row = (int)(tbuf[i]) / scale;
			if (global_df->SnTr.SNDchan == 1 || doMixedSTWave) {
				if (row > hp1) hp1 = row;
				if (row < lp1) lp1 = row;
			} else {
				if ((i % 2) == 0) {
					if (row > hp1) hp1 = row;
					if (row < lp1) lp1 = row;
				} else {
					if (row > hp2) hp2 = row;
					if (row < lp2) lp2 = row;
				}
			}
			scale_cnt++;
			if (scale_cnt == global_df->scale_row) {
				if (global_df->TopP1 && global_df->BotP1) {
					global_df->TopP1[col] = hp1;
					global_df->BotP1[col] = lp1;
				}
				if (global_df->TopP2 && global_df->BotP2) {
					global_df->TopP2[col] = hp2;
					global_df->BotP2[col] = lp2;
				}
				wdrawdot(global_df->SoundWin, hp1, lp1, hp2, lp2, col);
				col++;
				scale_cnt = 0L;
 				hp1 = 0; lp1 = hp1 - 1;
 				hp2 = 0; lp2 = hp2 - 1;
			}
		}
		cur += ((Size)i * 2L);
	}
	if (global_df->TopP1 && global_df->BotP1 && cm != 1)
		global_df->TopP1[col] = -1;
	return(1);
}

void PrintSoundWin(int col, int cm, Size cur) {
	char err_mess[512];
	int rm;
	short winHeight;
	short offset;

	if (global_df->SoundWin == NULL)
		return;
	if (global_df->SnTr.SoundFPtr == NULL && global_df->SnTr.rSoundFile[0] != EOS)
		global_df->SnTr.SoundFPtr = fopen(global_df->SnTr.rSoundFile, "rb");
	if (global_df->SnTr.SoundFPtr == NULL) {
		if (global_df->SoundWin)
			DisposeOfSoundWin();
		sprintf(err_mess, "Internal error occured.");
		do_warning(err_mess, -1);
		return;
	}
	offset = ComputeHeight(global_df->SoundWin,0,1);
	winHeight = ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows);
	if (global_df->SnTr.SNDchan == 1 || doMixedSTWave) {
		global_df->SnTr.SNDWHalfRow = (winHeight / 2) + 1;
		global_df->SnTr.SNDWprint_row1 = global_df->SoundWin->LT_row + offset + winHeight;
	} else {
		global_df->SnTr.SNDWHalfRow = (winHeight / 4) + 1;
		global_df->SnTr.SNDWprint_row1 = global_df->SoundWin->LT_row + offset + (winHeight / 2);
		global_df->SnTr.SNDWprint_row2 = global_df->SoundWin->LT_row + offset + winHeight;
	}
	if (global_df->TopP1 && global_df->BotP1 && global_df->SnTr.WBegFM == global_df->SnTr.WBegF && global_df->SnTr.WEndFM == global_df->SnTr.WEndF) {
		if (global_df->TopP1[0] == -1) {
			if (global_df->SnTr.isMP3 == TRUE) {
			}  else if (global_df->SnTr.SNDsample == 1)
				ReadSound8bit(col, cm, cur);
			else if (global_df->SnTr.SNDsample == 2)
				ReadSound16bit(col, cm, cur);
		} else {
			wdrawcontr(global_df->SoundWin, TRUE);
			GetSoundWinDim(&rm, &cm);
			for (rm=0; global_df->TopP1[rm] != -1 && rm < cm; rm++) {
				if (global_df->TopP2 == NULL || global_df->BotP2 == NULL)
					wdrawdot(global_df->SoundWin, global_df->TopP1[rm], global_df->BotP1[rm], 0, 0, rm);
				else
					wdrawdot(global_df->SoundWin, global_df->TopP1[rm], global_df->BotP1[rm], global_df->TopP2[rm], global_df->BotP2[rm], rm);
			}
			PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
		}
	} else {
		if (global_df->SnTr.isMP3 == TRUE) {
		}  else if (global_df->SnTr.SNDsample == 1)
			ReadSound8bit(col, cm, cur);
		else if (global_df->SnTr.SNDsample == 2)
			ReadSound16bit(col, cm, cur);

		PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
/*
		if (cm == 0) PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
		else if (global_df->err_message != DASHES) {
			strcpy(global_df->err_message, DASHES);
			draw_mid_wm();
		}
*/
	}
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
	wrefresh(global_df->w1);
}

Size ComputeOffset(Size prc) {
	Size Offset;

	Offset = (global_df->SnTr.WEndF-global_df->SnTr.WBegF) / 100L;
	if (Offset == 0) Offset = 1;
	Offset *= prc;
	return(Offset);
}

void DisplayEndF(char all) {
	if ((global_df->SnTr.EndF < global_df->SnTr.WBegF || global_df->SnTr.EndF > global_df->SnTr.WEndF) && global_df->SnTr.EndF != 0 && global_df->SnTr.EndF != global_df->SnTr.BegF) {
		global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.EndF-ComputeOffset(90L), '+');
		global_df->WinChange = FALSE;
		touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
		global_df->WinChange = TRUE;
	} else if ((global_df->SnTr.EndF == 0 || global_df->SnTr.BegF == global_df->SnTr.EndF) && (global_df->SnTr.BegF < global_df->SnTr.WBegF || global_df->SnTr.BegF > global_df->SnTr.WEndF)) {
		global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.BegF-ComputeOffset(15L), '-');
		global_df->WinChange = FALSE;
		touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
		global_df->WinChange = TRUE;
	} else if (all) {
		global_df->WinChange = FALSE;
		touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
		global_df->WinChange = TRUE;
	}
}

static void CheckNewF(Size NewF, char *dir, bool isDrawCursor) {
	Size t1, HalfF;

	if (global_df->SnTr.EndF < global_df->SnTr.BegF && global_df->SnTr.EndF != 0L) {
		t1 = global_df->SnTr.EndF;
		global_df->SnTr.EndF = global_df->SnTr.BegF;
		global_df->SnTr.BegF = t1;
	}

	if (global_df->SnTr.EndF == 0L || global_df->SnTr.BegF == global_df->SnTr.EndF) {
		*dir = 0;
		if (NewF != global_df->SnTr.BegF && isDrawCursor == true)
			DrawSoundCursor(0);
		if (global_df->SnTr.BegF > NewF) {
			global_df->SnTr.EndF = global_df->SnTr.BegF;
			global_df->SnTr.BegF = NewF;
		} else
			global_df->SnTr.EndF = NewF;
		if (isDrawCursor == true)
			DrawSoundCursor(2);
	} else if (NewF > global_df->SnTr.EndF) {
		if (*dir == -1) {
			if (isDrawCursor == true)
				DrawSoundCursor(2);
			t1 = global_df->SnTr.EndF;
		} else
			t1 = global_df->SnTr.BegF;
		global_df->SnTr.BegF = global_df->SnTr.EndF;
		global_df->SnTr.EndF = NewF;
		if (isDrawCursor == true)
			DrawSoundCursor(2);
		global_df->SnTr.BegF = t1;
		*dir = 1;
	} else if (NewF < global_df->SnTr.BegF) {
		if (*dir == 1) {
			if (isDrawCursor == true)
				DrawSoundCursor(2);
			t1 = global_df->SnTr.BegF;
		} else
			t1 = global_df->SnTr.EndF;
		global_df->SnTr.EndF = global_df->SnTr.BegF;
		global_df->SnTr.BegF = NewF; 
		if (isDrawCursor == true)
			DrawSoundCursor(2);
		global_df->SnTr.EndF = t1;
		*dir = -1;
	} else if (NewF > global_df->SnTr.BegF && NewF < global_df->SnTr.EndF && *dir == 1) {
		t1 = global_df->SnTr.BegF;
		global_df->SnTr.BegF = NewF;
		if (isDrawCursor == true)
			DrawSoundCursor(2);
		global_df->SnTr.BegF = t1;
		global_df->SnTr.EndF = NewF;
	} else if (NewF > global_df->SnTr.BegF && NewF < global_df->SnTr.EndF && *dir == -1) {
		t1 = global_df->SnTr.EndF;
		global_df->SnTr.EndF = NewF;
		if (isDrawCursor == true)
			DrawSoundCursor(2);
		global_df->SnTr.BegF = NewF;
		global_df->SnTr.EndF = t1;
	} else {
		HalfF = (global_df->SnTr.EndF - global_df->SnTr.BegF) / 2;
		if (NewF > global_df->SnTr.BegF && NewF < global_df->SnTr.BegF + HalfF) {
			t1 = global_df->SnTr.EndF;
			global_df->SnTr.EndF = NewF;
			if (isDrawCursor == true)
				DrawSoundCursor(2);
			global_df->SnTr.BegF = NewF;
			global_df->SnTr.EndF = t1;
			*dir = -1;
		} else if (NewF > global_df->SnTr.BegF+HalfF && NewF < global_df->SnTr.EndF) {
			t1 = global_df->SnTr.BegF;
			global_df->SnTr.BegF = NewF;
			if (isDrawCursor == true)
				DrawSoundCursor(2);
			global_df->SnTr.BegF = t1;
			global_df->SnTr.EndF = NewF;
			*dir = 1;
		}
	}
}

void delay_mach(DWORD num) {
	DWORD t;
	
	t = GetTickCount() + (num * 16.66);
	do {  
		if (GetTickCount() > t) 
			break;
	} while (1) ;	
}

char SetWaveTimeValue(long time) {
	return(TRUE);
}


char SetCurrSoundTier(void) {
	if (global_df->SoundWin) {
		if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) {
			DrawSoundCursor(0);
			DrawCursor(0);
			addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(global_df->SnTr.EndF));
			return(TRUE);
		}
	}
	return(FALSE);
}

void AdjustSoundScroll(int col, Size right_lim) {
	if (col < left_lim || col > right_lim) {
		BOOL	isStillDown;
		long	t;
		DWORD	tDelay;
		MSG		msg;

		DrawSoundCursor(0);
		NewF = ((Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample) * ScollValue;
		if (col < left_lim) {
			col = left_lim;
			if (NewF >= global_df->SnTr.WBegF) {
				if (global_df->SnTr.WBegF == 0L) 
					return;
				NewF = global_df->SnTr.WBegF;
				ScollValue = NewF / ((Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
				if (ScollValue < 1) ScollValue = 1;
				global_df->SnTr.WEndF = global_df->SnTr.WEndF - global_df->SnTr.WBegF;
				global_df->SnTr.WBegF = 0L;
			} else {
				global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF-NewF, '+');
				global_df->SnTr.WEndF = AlignMultibyteMediaStream(global_df->SnTr.WEndF-NewF, '+');
			}
//			MoveSoundWave(global_df->SoundWin, ScollValue, left_lim, right_lim-ScollValue);
			if (global_df->SnTr.WBegF >= global_df->SnTr.BegF && global_df->SnTr.WBegF <= global_df->SnTr.EndF) {
				t = global_df->SnTr.EndF;
				global_df->SnTr.EndF = global_df->SnTr.WBegF + NewF;
//				DrawSoundCursor(2);
				global_df->SnTr.EndF = t;
			}
			if (global_df->TopP1 && global_df->BotP1) {
				for (t=0; global_df->TopP1[t] != -1; t++) ;
				for (t--; t >= ScollValue; t--) {
					global_df->TopP1[t] = global_df->TopP1[t-ScollValue];
					global_df->BotP1[t] = global_df->BotP1[t-ScollValue];
				}
			}
			if (global_df->TopP2 && global_df->BotP2) {
				for (t=0; global_df->TopP1[t] != -1; t++) ;
				for (t--; t >= ScollValue; t--) {
					global_df->TopP2[t] = global_df->TopP2[t-ScollValue];
					global_df->BotP2[t] = global_df->BotP2[t-ScollValue];
				}
			}
//			PrintSoundWin(0, ScollValue, global_df->SnTr.WBegF);
		} else if (col > right_lim) {
			col = right_lim;
			if (NewF+global_df->SnTr.WEndF < global_df->SnTr.SoundFileSize) {
				global_df->SnTr.WBegF += NewF;
				global_df->SnTr.WEndF += NewF;
			} else {
				if (global_df->SnTr.WBegF == global_df->SnTr.SoundFileSize - (global_df->SnTr.WEndF-global_df->SnTr.WBegF))
					return;
				NewF = global_df->SnTr.SoundFileSize - global_df->SnTr.WEndF;
				ScollValue = NewF / ((Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
				if (ScollValue < 1)
					ScollValue = 1;
				global_df->SnTr.WBegF = global_df->SnTr.SoundFileSize - (global_df->SnTr.WEndF-global_df->SnTr.WBegF);
				global_df->SnTr.WEndF = global_df->SnTr.SoundFileSize;
			}
			global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF, '-');
			global_df->SnTr.WEndF = AlignMultibyteMediaStream(global_df->SnTr.WEndF, '-');
//			MoveSoundWave(global_df->SoundWin, -ScollValue, left_lim+ScollValue, right_lim);
			if (global_df->SnTr.WEndF >= global_df->SnTr.BegF && global_df->SnTr.WEndF <= global_df->SnTr.EndF) {
				t = global_df->SnTr.BegF; global_df->SnTr.BegF = global_df->SnTr.WEndF - NewF;
//				DrawSoundCursor(2);
				global_df->SnTr.BegF = t;
			}
			if (global_df->TopP1 && global_df->BotP1) {
				for (t=0; global_df->TopP1[t+ScollValue] != -1; t++) {
					global_df->TopP1[t] = global_df->TopP1[t+ScollValue];
					global_df->BotP1[t] = global_df->BotP1[t+ScollValue];
				}
			}
			if (global_df->TopP2 && global_df->BotP2) {
				for (t=0; global_df->TopP1[t+ScollValue] != -1; t++) {
					global_df->TopP2[t] = global_df->TopP2[t+ScollValue];
					global_df->BotP2[t] = global_df->BotP2[t+ScollValue];
				}
			}
//			PrintSoundWin(cm-ScollValue, cm, global_df->SnTr.WEndF-NewF);
		}
		NewF = global_df->SnTr.WBegF + ((col-left_lim) * global_df->scale_row * global_df->SnTr.SNDsample);
		NewF = AlignMultibyteMediaStream(NewF, '-');
		CheckNewF(NewF, &dir, false);
		global_df->WinChange = FALSE;
		touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
		global_df->WinChange = TRUE;
		DrawSoundCursor(2);
		isStillDown = TRUE;
		tDelay = GetTickCount() + 100;
		do {
			if (GetTickCount() > tDelay)
				break;
				if (PeekMessage(&msg, AfxGetApp()->m_pMainWnd->m_hWnd, 0, 0, PM_REMOVE)) {
					if (msg.message != WM_LBUTTONDOWN && msg.message != WM_RBUTTONDOWN) {
						isStillDown = FALSE;
					}
				}
		} while (isStillDown) ;
	} else {
		NewF = global_df->SnTr.WBegF + ((col-left_lim) * global_df->scale_row * global_df->SnTr.SNDsample);
		NewF = AlignMultibyteMediaStream(NewF, '-');
		CheckNewF(NewF, &dir, true);
		PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
	}
}

char AdjustSound(int row, int col, int ext, Size right_lim) {
	int rowExtender;

	if (doMixedSTWave)
		rowExtender = 1;
	else
		rowExtender = ((global_df->SnTr.SNDchan - 1) * 5) + 1;
	if (ext == 2) {
		if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) 
			isPlayS = -3;
		else
			isPlayS = -2;
		return(5);
	} else if (col < global_df->SnTr.SNDWccol) {
		if (row == 2+rowExtender) {
			if (SetCurrSoundTier())
				return(2);
		} else if ((row == 0+rowExtender) || (row == 4+rowExtender)) {
			Size ws;
			int  rm, cm;
	
			GetSoundWinDim(&rm, &cm);
			if (row == 4+rowExtender) {
				ws = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
				if (ws < global_df->SnTr.SoundFileSize) {
					ws = global_df->scale_row;
					global_df->scale_row += global_df->scale_row;
					if (conv_to_msec_rep(((Size)cm*(Size)global_df->scale_row*(Size)global_df->SnTr.SNDsample)) > 
							SCALE_TIME_LIMIT) {
						ws = conv_from_msec_rep(SCALE_TIME_LIMIT);
						global_df->scale_row = ws / cm / (Size)global_df->SnTr.SNDsample;
					}
					ws = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
					if (global_df->SnTr.WBegF+ws >= global_df->SnTr.SoundFileSize) {
						global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.SoundFileSize-ws, '+');
					}
					global_df->SnTr.WEndF = global_df->SnTr.WBegF + ws;
				}
			} else if (row == 0+rowExtender) {
				global_df->scale_row -= (global_df->scale_row / 2);
				global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
			}
			ws = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
			PutSoundStats(ws);
			delay_mach(20);
		}
	} else if (col > right_lim) {
		if ((row == 1+rowExtender) || (row == 3+rowExtender)) {
			if (row == 1+rowExtender) leftChan = ((leftChan == 1) ? 0 : 1);
			else rightChan = ((rightChan == 1) ? 0 : 1);
			wdrawcontr(global_df->SoundWin, TRUE);
		} else if ((row == 0+rowExtender) || (row == 4+rowExtender)) {
			if (row == 0+rowExtender) global_df->mainVScale *= 2;
			else global_df->mainVScale /= 2;
			if (global_df->mainVScale < 1) global_df->mainVScale = 1;
			global_df->SnTr.WEndFM = !global_df->SnTr.WEndF;
		}
	} else {
		dir = 0;
		GetSoundWinDim(&left_lim, &cm);
		left_lim = global_df->SnTr.SNDWccol;
		if (col < left_lim || col > right_lim) 
			return(5);
		NewF = global_df->SnTr.WBegF + ((col-left_lim) * global_df->scale_row * global_df->SnTr.SNDsample);
		NewF = AlignMultibyteMediaStream(NewF, '+');
		if (ext != 1) {
			DrawSoundCursor(0);
			global_df->SnTr.BegF = NewF;
			global_df->SnTr.EndF = 0L;
			if (syncAudioAndVideo(global_df->MvTr.MovieFile)) {
				if (MpegDlg != NULL)
					MpegDlg->SetCurPosToValue(conv_to_msec_rep(NewF));
/* // NO QT
				else if (MovDlg != NULL)
					MovDlg->SetCurPosToValue(conv_to_msec_rep(NewF));
*/
			}
		} else
			CheckNewF(NewF, &dir, true);
		PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);

		ScollValue = (cm * SCROLLPERCENT / 100);
		if (ScollValue < 1)
			ScollValue = 1;
		AdjustSoundScroll(col, right_lim);
		return(7);
	}
	return(2);
}

int soundwindow(int TurnOn) {
	int cm, rm, rows;
	Size ws;
	FONTINFO tf;

	if (TurnOn == 0) {
		DisposeOfSoundWin();
		if (MpegDlg != NULL) {
			if (MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->m_wmpControler != NULL) { //playing
				(MpegDlg->mpeg)->StopMpegMovie();
				(MpegDlg->mpeg)->isPlaying = 0;
			}
			MpegDlg->doErase = false;
			MpegDlg->OnCancel();
			MpegDlg = NULL;
		}
/* // NO QT
		if (MovDlg != NULL) {
			if (MovDlg->qt != NULL && !(MovDlg->qt)->IsQTMovieDone()) { //playing
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
			}
			MovDlg->doErase = false;
			MovDlg->OnCancel();
			MovDlg = NULL;
		}
*/
		return(0);
	}

	if (doMixedSTWave)
		rows = 5;
	else
		rows = global_df->SnTr.SNDchan * 5;
	rows += 1;
	cm = rows - COM_WIN_SIZE;
	if (cm > 0) {
		char t;
		global_df->EdWinSize = 0;
		global_df->CodeWinStart = 0;
		global_df->total_num_rows = rows;
		t = global_df->NoCodes;
		global_df->NoCodes = FALSE;
		if (!init_windows(true, 1, false))
			mem_err(TRUE, global_df);
		global_df->NoCodes = t;
		if (COM_WIN_SIZE < rows) {
			RemoveLastUndo();
			strcpy(global_df->err_message, "+Window is not large enough");
			return(1);
		} 
	}

	if ((cm=WindowPageWidth()) < 80)
		cm = 80;
	if ((global_df->SoundWin=newwin(rows, cm, global_df->CodeWinStart, 0, 0)) == NULL) {
		strcpy(global_df->err_message, "+Can't create sound window");
		DisposeOfSoundWin();
		return(1);
	}

	if (GlobalDC != NULL) {
		int cnt, winPixelSize;
		LOGFONT lfFont;
		CFont l_font;
		CSize CharsWidth;
		CFont* pOldFont;

		winPixelSize = global_df->SoundWin->winPixelSize - 1;
		strcpy(tf.FName, "CAfont");
		tf.FSize = -16; //-10;
		tf.CharSet = 0;
		tf.Encod = tf.CharSet;
		tf.FHeight = GetFontHeight(&tf, NULL);
		SetLogfont(&lfFont, &tf, NULL);
		if (l_font.CreateFontIndirect(&lfFont) != 0) {
			pOldFont = GlobalDC->SelectObject(&l_font);
			CharsWidth = GlobalDC->GetTextExtent(_T("IjWfFg"), 6);
			GlobalDC->SelectObject(pOldFont);
			l_font.DeleteObject();
			tf.FHeight = CharsWidth.cy + FLINESPACE;
			cnt = 0;
			if ((rows * tf.FHeight) < winPixelSize) {
				do {
					tf.FSize--;	
					lfFont.lfHeight = tf.FSize;
					l_font.CreateFontIndirect(&lfFont);
					pOldFont = GlobalDC->SelectObject(&l_font);
					CharsWidth = GlobalDC->GetTextExtent(_T("W"), 1);
					GlobalDC->SelectObject(pOldFont);
					l_font.DeleteObject();
					tf.FHeight = CharsWidth.cy + FLINESPACE;
					cnt++;
					if (cnt > 30)
						break;
				} while ((rows * tf.FHeight) <= winPixelSize) ;
				tf.FSize++;
			} else if ((rows * tf.FHeight) > winPixelSize) {
				do {
					tf.FSize++;	
					lfFont.lfHeight = tf.FSize;
					l_font.CreateFontIndirect(&lfFont);
					pOldFont = GlobalDC->SelectObject(&l_font);
					CharsWidth = GlobalDC->GetTextExtent(_T("W"), 1);
					GlobalDC->SelectObject(pOldFont);
					l_font.DeleteObject();
					tf.FHeight = CharsWidth.cy + FLINESPACE;
					cnt++;
					if (cnt > 30)
						break;
				} while ((rows * tf.FHeight) > winPixelSize) ;
			}
			if (cnt <= 30) {
				for (rows--; rows >= 0; rows--) {
					strcpy(global_df->SoundWin->RowFInfo[rows]->FName, tf.FName);
					global_df->SoundWin->RowFInfo[rows]->FSize = tf.FSize;
					global_df->SoundWin->RowFInfo[rows]->CharSet = tf.CharSet;
					global_df->SoundWin->RowFInfo[rows]->FHeight = tf.FHeight;
				}
			}
		}
	}

	global_df->SnTr.SNDWccol =  FontTxtWidth(global_df->SoundWin,0,cl_T("+W"),0,2);
	GetSoundWinDim(&rm, &cm);
	global_df->TopP1 = (int *)malloc((cm+1) * sizeof(int));
	global_df->BotP1 = (int *)malloc((cm+1) * sizeof(int));
	if (global_df->SnTr.SNDchan == 2 && !doMixedSTWave) {
		global_df->TopP2 = (int *)malloc((cm+1) * sizeof(int));
		global_df->BotP2 = (int *)malloc((cm+1) * sizeof(int));
	} else {
		global_df->TopP2 = NULL;
		global_df->BotP2 = NULL;
	}
	global_df->SnTr.WBegFM = 0L;
	global_df->SnTr.WEndFM = 0L;
	ws = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	if (global_df->scale_row <= 0L || ws >= global_df->SnTr.SoundFileSize )
		global_df->scale_row = 128L * global_df->SnTr.SNDsample * global_df->SnTr.SNDchan;
	if (global_df->mainVScale < 1)
		global_df->mainVScale = 1;
	global_df->SnTr.WBegF = 0L;
	global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	ResetUndos();
	wstandend(global_df->SoundWin);
	DisplayEndF(TRUE);
	DrawSoundCursor(1);
	return(0);
}

int ContPlayPause(void) {
	if (PlayingContSound == '\002') {
		global_df->WinChange = FALSE;
		strcpy(global_df->err_message, "-Pausing ...");
		draw_mid_wm();
		global_df->WinChange = TRUE;
	} else if (PlayingContSound == '\003') {
		PlayingContSound = FALSE;
//		DrawCursor(2);
//		ResetSelectFlag(0);
		strcpy(global_df->err_message, "-Aborted.");
		return(TRUE);
	}
	return(FALSE);
}

void show_wave(char *file, long int begin, long int end, int add) {
}

/* Play sound Begin */
#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)GlobalHandle(lp))

#define     GlobalLockPtr(lp)                \
                ((BOOL)GlobalLock(GlobalPtrHandle(lp)))
#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)        \
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)       \
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)                \
                (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define MACBUFSIZE	65536L // 131072L // 65536L // 32768L // 15360L // 10240L 
// MUST be multiple of 256-2
static long		CurF;

static void cleanup(void) {
	// if you add wave instance data, this is a good place to free it.
	cWaveHdr = NULL;
	GlobalFreePtr( waveHdr1 );
	waveHdr1 = NULL;
	GlobalFreePtr( waveHdr2 );
	waveHdr2 = NULL;
	GlobalFreePtr( SB1 );
	SB1 = NULL;
	GlobalFreePtr( SB2 );
	SB2 = NULL;
}

void stopSoundIfPlaying(void) {
	if (hWaveOut != NULL) {
		if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
		waveOutReset(hWaveOut);
		waveOutUnprepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR));
		waveOutUnprepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR));
		cleanup();
		waveOutClose(hWaveOut);
		hWaveOut = NULL;
		isWinSndDonePlaying = 1;
		if (global_df && PBC.walk != 1) {
			DrawSoundCursor(0);
			DrawCursor(0);
			strcpy(global_df->err_message, DASHES);
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
		}
	}
}

int play_sound(long int begin, long int end, int cont) {
	SndPlayLoopCnt = 1;
	if (!PlayBuffer(cont)) {
		PlayingSound = FALSE;
		PlayingContSound = FALSE;
		DrawCursor(0);
		global_df->SnTr.SoundFile[0] = EOS;
		if (global_df->SnTr.isMP3 == TRUE) {
			global_df->SnTr.isMP3 = FALSE;
		} else {
			fclose(global_df->SnTr.SoundFPtr);
		}
		global_df->SnTr.SoundFPtr = 0;
		if (global_df->SoundWin)
			DisposeOfSoundWin();
		return(1);
	}

	if (!PlayingContSound)
		DrawCursor(0);
	return(0);
}

static DWORD ReadSound(HPSTR buf, DWORD max) {
	register long i;
	short  *tShort;
	long	bytesCount;

	bytesCount = my_MIN(max, (DWORD)(global_df->SnTr.EndF-CurF));
	if (CurF < global_df->SnTr.SoundFileSize) {
		if (CurF+bytesCount >= global_df->SnTr.SoundFileSize)
			bytesCount = global_df->SnTr.SoundFileSize - CurF;

		fseek(global_df->SnTr.SoundFPtr, CurF+global_df->SnTr.AIFFOffset, SEEK_SET);
		fread(buf, 1, bytesCount, global_df->SnTr.SoundFPtr);
		if (global_df->SnTr.SFileType == 'AIFF') {
			if (global_df->SnTr.SNDsample == 1) {
				for (i=0; i < bytesCount; i++)
					buf[i] += (char)128;
			} else {
				if (global_df->SnTr.DDsound) {
					for (i=0; i < bytesCount; i+=2) {
						tShort = (short *)&buf[i];
						flipShort(tShort);
					}
				}
			}
		}
	} else
		bytesCount = 0;
	if (bytesCount == 0)
		global_df->SnTr.EndF = CurF;
	CurF += bytesCount;
	return(bytesCount);
}

/*
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwUVar, DWORD dwP1, DWORD dwP2) {
	if (hwo == hWaveOut && hWaveOut != NULL) {
		if (uMsg == WOM_DONE) {
			gBufferDone = true;
		}
	}
}
*/

char checkPCMSound(MSG* pMsg, char skipOnChar, DWORD tickCount) {
	long   CurFP;
	MMTIME pmmt;

	pmmt.wType = TIME_BYTES;
  	waveOutGetPosition(hWaveOut, &pmmt, sizeof(pmmt));
	CurFP = (long)pmmt.u.cb + orgBeg + global_df->SnTr.AIFFOffset;
	checkContinousSndPlayPossition(CurFP);
	if (!PlayingContSound && PBC.enable && PBC.isPC) {
		strcpy(global_df->err_message, "-Playing, press any F5-F6 key to stop");
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
	}
	if (pMsg != NULL) {
		if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN) {
			if (PlayingContSound || !PBC.enable) {
				if (PlayingContSound)
					PlayingContSound = '\003';
				stopSoundIfPlaying();
				isWinSndPlayAborted = 1;
				skipOnChar = TRUE;
			}
		} else if (pMsg->message == WM_KEYDOWN) {
			if (PlayingContSound == '\004') {
				char res;
				global_df->LeaveHighliteOn = TRUE;
				if ((pMsg->wParam == VK_F5 || pMsg->wParam == VK_F1 || pMsg->wParam == VK_F2 || pMsg->wParam == VK_F3) && F5_Offset != 0L) {
					DrawCursor(2);
					DrawSoundCursor(2);
					ResetSelectFlag(0);
					pmmt.wType = TIME_BYTES;
					waveOutGetPosition(hWaveOut, &pmmt, sizeof(pmmt));
					CurFP = (long)pmmt.u.cb + orgBeg + global_df->SnTr.AIFFOffset;
					if (global_df->SnTr.BegF != CurFP && CurFP != 0L) {
						global_df->row_win2 = 0L;
						global_df->col_win2 = -2L;
						global_df->col_chr2 = -2L;
						if ((res = findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
							findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
						SaveUndoState(FALSE);
						addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(CurFP));
						if (FreqCountLimit > 0 && global_df->FreqCount <= FreqCountLimit)
							global_df->FreqCount++;
						global_df->SnTr.BegF = AlignMultibyteMediaStream(CurFP, '-');
						if (global_df->SnTr.IsSoundOn)
							DisplayEndF(FALSE);
						if (GlobalDoc && global_df->DataChanged)
							GlobalDoc->SetModifiedFlag(TRUE);
					}
					selectNextSpeaker();
					strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
					draw_mid_wm();
					strcpy(global_df->err_message, DASHES);
					wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
					wrefresh(global_df->w1);
					DrawSoundCursor(1);
					DrawCursor(1);
					isWinSndDonePlaying = 1;
				} else if (pMsg->wParam == 'i' || pMsg->wParam == 'I' || pMsg->wParam == ' ') {
					DrawCursor(2);
					DrawSoundCursor(2);
					ResetSelectFlag(0);
					pmmt.wType = TIME_BYTES;
					waveOutGetPosition(hWaveOut, &pmmt, sizeof(pmmt));
					CurFP = (long)pmmt.u.cb + orgBeg + global_df->SnTr.AIFFOffset;
					if (global_df->SnTr.BegF != CurFP && CurFP != 0L) {
						global_df->row_win2 = 0L;
						global_df->col_win2 = -2L;
						global_df->col_chr2 = -2L;
						if ((res = findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
							findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
						SaveUndoState(FALSE);
						addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(CurFP));
						if (FreqCountLimit > 0 && global_df->FreqCount <= FreqCountLimit)
							global_df->FreqCount++;
						global_df->SnTr.BegF = AlignMultibyteMediaStream(CurFP, '-');
						if (global_df->SnTr.IsSoundOn)
							DisplayEndF(FALSE);
						if (GlobalDoc && global_df->DataChanged)
							GlobalDoc->SetModifiedFlag(TRUE);
					}
					selectNextSpeaker();
					strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
					draw_mid_wm();
					strcpy(global_df->err_message, DASHES);
					wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
					wrefresh(global_df->w1);
					DrawSoundCursor(1);
					DrawCursor(1);
				}
			} else if (pMsg->wParam >= 32) {
				if (PlayingContSound) {
					PlayingContSound = '\003';
					stopSoundIfPlaying();
					isWinSndPlayAborted = 1;
					skipOnChar = TRUE;
				}
				if (!PBC.enable || (pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F12)) {
					stopSoundIfPlaying();
					isWinSndPlayAborted = 1;
					if (pMsg->wParam == VK_F7 && PBC.enable) {
						isWinSndPlayAborted = 4; // F7
					} else if (pMsg->wParam == VK_F9 && PBC.enable) {
						isWinSndPlayAborted = 5; // F8
					} else
						isWinSndPlayAborted = 1;
					skipOnChar = TRUE;
				}
			}
		}
	}
	if (isWinSndDonePlaying)
		stopSoundIfPlaying();

	if (hWaveOut == NULL) {
		if (!isWinSndPlayAborted && PBC.isPC && PBC.LoopCnt > SndPlayLoopCnt && sndCom != (int)'p') {
			SndPlayLoopCnt++;
			PlayBuffer(sndCom);
			return(skipOnChar);
		}
		if (/*!isWinSndPlayAborted && */PBC.walk && PBC.backspace != PBC.step_length && global_df) {
			long t;
			extern SInt32 sEF;

			SndPlayLoopCnt = 1;
			DrawSoundCursor(0);
			if (isWinSndPlayAborted) {
				if (isWinSndPlayAborted == 4) {
					t = conv_to_msec_rep(CurFP) - PBC.step_length;
					if (t < 0L)
						t = 0L;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				} else if (isWinSndPlayAborted == 5) {
					t = conv_to_msec_rep(CurFP) + PBC.step_length;
					if (t < 0L)
						t = 0L;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				} else {
					t = conv_to_msec_rep(CurFP);
					if (t < 0L)
						goto fin;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				}
			} else if (PBC.backspace > PBC.step_length) {
				t = conv_to_msec_rep(global_df->SnTr.BegF) - (PBC.backspace - PBC.step_length);
				if (t < 0L)
					goto fin;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
					goto fin;
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.EndF) - (PBC.backspace - PBC.step_length);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
					global_df->SnTr.EndF = sEF;
			} else {
				t = conv_to_msec_rep(global_df->SnTr.BegF) + (PBC.step_length - PBC.backspace);
				if (t < 0L)
					goto fin;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
					goto fin;
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.EndF) + (PBC.step_length - PBC.backspace);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
					global_df->SnTr.EndF = sEF;
			}
			SetPBCglobal_df(false, 0L);
			DisplayEndF(TRUE);
			DrawSoundCursor(3);

			walker_pause = 0L;
			if (!isWinSndPlayAborted && PBC.pause_len > 0L && !PlayingContSound) {
				walker_pause = tickCount + PBC.pause_len;
			} else if (isWinSndPlayAborted != 1) {
				PlayBuffer(sndCom);
			}
			return(skipOnChar);
		}
fin:
		if (global_df) {
			if (isWinSndPlayAborted == 1 && PBC.walk)
				strcpy(global_df->err_message, "-Aborted.");
			else if (!PlayingContSound && PBC.enable && PBC.isPC)
				strcpy(global_df->err_message, DASHES);
			else if (global_df->LastCommand == 49) {
				extern long gCurF;

				gCurF = conv_to_msec_rep(CurFP);
				if (gCurF >= 0L) {
					gCurF = AlignMultibyteMediaStream(conv_from_msec_rep(gCurF), '+');
				} else
					gCurF = global_df->SnTr.BegF;
			}
			draw_mid_wm();
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			wrefresh(global_df->w1);
		}
		if (global_df)
			SetPBCglobal_df(false, 0L);
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	return(skipOnChar);
}

char checkPCMPlaybackBuffer(void) {
	if (cWaveHdr == NULL)
		gBufferDone = true;
	else if (cWaveHdr->dwFlags & WHDR_DONE)
		gBufferDone = true;
	if (gBufferDone && hWaveOut != NULL) {
		isCurSB1 = !isCurSB1;
		if (isCurSB1)
			cWaveHdr = waveHdr1;
		else
			cWaveHdr = waveHdr2;

		if (cWaveHdr->dwBufferLength == 0 || isWinSndDonePlaying) {
			isWinSndDonePlaying = 1;
		} else {
			gBufferDone = false;
			if (isCurSB1) {
				tWaveHdr = waveHdr2;
				if (CurF < global_df->SnTr.EndF)
					waveHdr2->dwBufferLength = ReadSound(SB2, MACBUFSIZE);
				else
					waveHdr2->dwBufferLength = 0;
			} else {
				tWaveHdr = waveHdr1;
				if (CurF < global_df->SnTr.EndF) 
					waveHdr1->dwBufferLength = ReadSound(SB1, MACBUFSIZE);
				else
					waveHdr1->dwBufferLength = 0;
			}
			if (tWaveHdr->dwBufferLength > 0) {
				if (waveOutWrite(hWaveOut, tWaveHdr, sizeof(WAVEHDR)) != 0) {
					if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
					waveOutReset(hWaveOut);
					waveOutUnprepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR));
					waveOutUnprepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR));
					cleanup();
					waveOutClose(hWaveOut);
					hWaveOut = NULL;
					strcpy(global_df->err_message, "+Error playing sound.");
					return(0);
				}
			}
		}
	}
	return(1);
}

static char PlayPCMSound(int com) {
	WAVEFORMATEX	pFormat;
	DWORD	tickCount, lastTickCount;

	sndCom = com;
	if (com == 'p' || global_df->SnTr.EndF == 0) {
		strcpy(global_df->err_message, "+Please select sound portion.");
		return(2);
	}
	if (!leftChan && !rightChan) {
		strcpy(global_df->err_message, "+No channel was selected. Please goto \"sonic mode\" and click on 'L' and/or 'R'.");
		return(2);
	}
	if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize) {
		strcpy(global_df->err_message, "+Starting point is beyond end of file.");
		return(2);
	}
	pFormat.wFormatTag = ((global_df->SnTr.SNDformat == 0L) ? 1L : global_df->SnTr.SNDformat);
	pFormat.nChannels = global_df->SnTr.SNDchan;
	pFormat.nSamplesPerSec = (DWORD)global_df->SnTr.SNDrate;
	pFormat.wBitsPerSample = global_df->SnTr.SNDsample * 8;
	pFormat.nBlockAlign = pFormat.nChannels * pFormat.wBitsPerSample / 8;
	pFormat.nAvgBytesPerSec = pFormat.nSamplesPerSec * pFormat.nBlockAlign;
	pFormat.cbSize = 0;

	if (PBC.speed != 100) {
		double t1;

 		t1 = (double)pFormat.nSamplesPerSec;
 		t1 *= (double)PBC.speed;
 		t1 = t1 / 100.0000;
 		pFormat.nSamplesPerSec = (unsigned long)t1;
	}
	if (waveOutOpen(&hWaveOut,WAVE_MAPPER,&pFormat,0,0L,WAVE_FORMAT_QUERY)) {
		strcpy(global_df->err_message, "+Cannot play sound: audio hardware not responding or unavailable.");
		hWaveOut = NULL;
		return(0);
	}
//	if (waveOutOpen(&hWaveOut,WAVE_MAPPER,&pFormat,(UINT)waveOutProc,0L,CALLBACK_FUNCTION )) {
	if (waveOutOpen(&hWaveOut,WAVE_MAPPER,&pFormat,0,0L,CALLBACK_NULL )) {
		strcpy(global_df->err_message, "+Error playing sound.");
		hWaveOut = NULL;
		return(0);
	}
	DWORD dwVolume;
	if (waveOutGetVolume(hWaveOut, &oDwVolume) != MMSYSERR_NOERROR) ;
//	dwVolume = oDwVolume;
	dwVolume = 0xFFFFFFFF;
	if (global_df->SnTr.SNDchan != 1) {
		if (!leftChan)
			dwVolume &= 0xFFFF0000;
		if (!rightChan)
			dwVolume &= 0x0000FFFF;
		if (waveOutSetVolume(hWaveOut, dwVolume) != MMSYSERR_NOERROR) ;
	}
 
	CurF = orgBeg = global_df->SnTr.BegF;
	checkContinousSndPlayPossition(global_df->SnTr.BegF);

	SB1 = (HPSTR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, MACBUFSIZE);
	if (!SB1) {
		strcpy(global_df->err_message, "+Out of memory.");
		return(0);
	}
	SB2 = (HPSTR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, MACBUFSIZE);
	if (!SB2) {
		strcpy(global_df->err_message, "+Out of memory.");
		GlobalFreePtr(SB1);
		SB1 = NULL;
		return(0);
	}
	
	waveHdr1 = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,(DWORD)sizeof(WAVEHDR));
	if (!waveHdr1) {
		GlobalFreePtr(SB1);
		GlobalFreePtr(SB2);
		SB1 = NULL;
		SB2 = NULL;
		strcpy(global_df->err_message, "+Out of memory.");
		return(0);
	}
	waveHdr1->lpData = SB1;
	waveHdr1->dwBufferLength = 0;
	waveHdr1->dwFlags = 0L;
	waveHdr1->dwLoops = 0L;
// waveHdr1->dwUser = (DWORD) lpYourData; // save instance data ptr

	waveHdr2 = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,(DWORD)sizeof(WAVEHDR));
	if (!waveHdr2) {
		GlobalFreePtr(SB1);
		GlobalFreePtr(SB2);
		SB1 = NULL;
		SB2 = NULL;
		if (cWaveHdr == waveHdr1)
			cWaveHdr = NULL;
		GlobalFreePtr( waveHdr1 );
		waveHdr1 = NULL;
		strcpy(global_df->err_message, "+Out of memory.");
		return(0);
	}
	waveHdr2->lpData = SB2;
	waveHdr2->dwBufferLength = 0;
	waveHdr2->dwFlags = 0L;
	waveHdr2->dwLoops = 0L;
	// waveHdr2->dwUser = (DWORD) lpYourData; // save instance data ptr

	if (waveOutPrepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR))) {
		if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
		cleanup();
		waveOutClose(hWaveOut);
		hWaveOut = NULL;
		strcpy(global_df->err_message, "+Out of memory.");
		return(0);
	}
	if (waveOutPrepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR))) {
		if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
		cleanup();
		waveOutClose(hWaveOut);
		hWaveOut = NULL;
		strcpy(global_df->err_message, "+Out of memory.");
		return(0);
	}
/* 07-10-01
	if (PBC.speed != 100) {
		double t1, t2;
		DWORD dwRate;
		MMRESULT res;

 		t1 = (double)0x10000;
 		t1 *= (double)PBC.speed;
 		t1 = t1 / 100.0000;
 		t2 = (double)0x10000;
 		t2 = t2 / t1;
 		t1 = (double)0x10000;
 		t1 *= t2;
 		dwRate = (long)t1;
		if ((res=waveOutSetPlaybackRate(hWaveOut, dwRate)) != MMSYSERR_NOERROR ) {
			if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
			waveOutReset(hWaveOut);
			waveOutUnprepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR));
			waveOutUnprepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR));
			cleanup();
			waveOutClose(hWaveOut);
			hWaveOut = NULL;
			if (res == MMSYSERR_NOTSUPPORTED)
				strcpy(global_df->err_message, "+Change of playback speed is not supported on this machine.");
			else
				strcpy(global_df->err_message, "+Error playing sound.");
			return(0);
		}
 	}
*/
	gBufferDone = false;
	isCurSB1 = true;
	if (isCurSB1)
		cWaveHdr = waveHdr1;
	else
		cWaveHdr = waveHdr2;

	cWaveHdr->dwBufferLength = ReadSound(SB1, MACBUFSIZE);
	if (waveOutWrite(hWaveOut, cWaveHdr, sizeof(WAVEHDR)) != 0) {
		if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
		waveOutReset(hWaveOut);
		waveOutUnprepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR));
		waveOutUnprepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR));
		cleanup();
		waveOutClose(hWaveOut);
		hWaveOut = NULL;
		strcpy(global_df->err_message, "+Error playing sound.");
		return(0);
	}

	if (isCurSB1) {
		tWaveHdr = waveHdr2;
		if (CurF < global_df->SnTr.EndF)
			waveHdr2->dwBufferLength = ReadSound(SB2, MACBUFSIZE);
		else
			waveHdr2->dwBufferLength = 0;
	} else {
		tWaveHdr = waveHdr1;
		if (CurF < global_df->SnTr.EndF) 
			waveHdr1->dwBufferLength = ReadSound(SB1, MACBUFSIZE);
		else
			waveHdr1->dwBufferLength = 0;
	}
	if (tWaveHdr->dwBufferLength > 0) {
		if (waveOutWrite(hWaveOut, tWaveHdr, sizeof(WAVEHDR)) != 0) {
			if (waveOutSetVolume(hWaveOut, oDwVolume) != MMSYSERR_NOERROR) ;
			waveOutReset(hWaveOut);
			waveOutUnprepareHeader(hWaveOut, waveHdr1, sizeof(WAVEHDR));
			waveOutUnprepareHeader(hWaveOut, waveHdr2, sizeof(WAVEHDR));
			cleanup();
			waveOutClose(hWaveOut);
			hWaveOut = NULL;
			strcpy(global_df->err_message, "+Error playing sound.");
			return(0);
		}
	}
	
	if (PlayingContSound)
		isWinSndPlayAborted = 2;
	else
		isWinSndPlayAborted = 0;
	isWinSndDonePlaying = 0;
	if (PlayingContSound || !PBC.isPC) {
		char t = 0;
		MSG msg;
		tickCount = GetTickCount();
		lastTickCount = tickCount;
		while (hWaveOut != NULL) {
			if (tickCount - lastTickCount > 15) {
				if (PeekMessage(&msg, AfxGetApp()->m_pMainWnd->m_hWnd, 0, 0, PM_REMOVE)) {
					t = checkPCMSound(&msg, t, tickCount);
				} else {
					t = checkPCMSound(NULL, t, tickCount);
				}
				lastTickCount = tickCount;
			}
			tickCount = GetTickCount();
			if (!checkPCMPlaybackBuffer())
				return(0);
		}
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	return(1);
}

char PlayBuffer(int com) {
	if (global_df->SnTr.SoundFPtr == NULL && global_df->SnTr.rSoundFile[0] != EOS)
		global_df->SnTr.SoundFPtr = fopen(global_df->SnTr.rSoundFile, "rb");
	if (global_df->SnTr.SoundFPtr == NULL)
		return(FALSE);
	return(PlayPCMSound(com));
}
/* Play sound End */

void WriteAIFFHeader(FILE *fpout, long numBytes, unsigned int frames, short numChannels, short sampleSize, double sampleRate) {
	formAIFF.ckID = 'FORM';
	formAIFF.ckSize = numBytes + sizeof(formAIFF)+sizeof(commonChunk)+sizeof(soundDataChunk) - 16;
	formAIFF.formType = 'AIFF';
//	if (byteOrder == CFByteOrderLittleEndian) {
		flipLong((long*)&formAIFF.ckID);
		flipLong(&formAIFF.ckSize);
		flipLong((long*)&formAIFF.formType);
//	}

	commonChunk.ckID = 'COMM';
	commonChunk.ckSize = 18;
	commonChunk.numChannels = numChannels;
	commonChunk.numSampleFrames = frames;
	commonChunk.sampleSize = sampleSize;

	Doubletox80(&sampleRate, (struct myX80Rep *)&commonChunk.sampleRate);					
//	if (byteOrder == CFByteOrderLittleEndian) {
		flipLong((long*)&commonChunk.ckID);
		flipLong(&commonChunk.ckSize);
		flipShort(&commonChunk.numChannels);
		flipLong((long*)&commonChunk.numSampleFrames);
		flipShort(&commonChunk.sampleSize);
		flipShort(&commonChunk.sampleRate.exp);
		flipShort((short*)&commonChunk.sampleRate.man[0]);
		flipShort((short*)&commonChunk.sampleRate.man[1]);
		flipShort((short*)&commonChunk.sampleRate.man[2]);
		flipShort((short*)&commonChunk.sampleRate.man[3]);
//	}

	soundDataChunk.ckID = 'SSND';
	soundDataChunk.ckSize = numBytes;// 2 * frames * numChannels + 8;
	soundDataChunk.offset = 0;
	soundDataChunk.blockSize = 0;
//	if (byteOrder == CFByteOrderLittleEndian) {
		flipLong((long*)&soundDataChunk.ckID);
		flipLong(&soundDataChunk.ckSize);
		flipLong((long*)&soundDataChunk.offset);
		flipLong((long*)&soundDataChunk.blockSize);
//	}

	fseek(fpout, 0, SEEK_SET);
	// Write the AIFF header, previously initilized
	if (fwrite((char *)&formAIFF, 1, sizeof(formAIFF), fpout) != sizeof(formAIFF)) {
		strcpy(global_df->err_message, "+Error writting sound file.");
		return;
	}
	if (fwrite((char *)&commonChunk, 1, sizeof(commonChunk), fpout) != sizeof(commonChunk)) {
		strcpy(global_df->err_message, "+Error writting sound file.");
		return;
	}
	if (fwrite((char *)&soundDataChunk, 1, sizeof(soundDataChunk), fpout) != sizeof(soundDataChunk)) {
		strcpy(global_df->err_message, "+Error writting sound file.");
		return;
	}
}
//#pragma comment(lib, "mf.lib") 

extern HRESULT CreateWavFile(const WCHAR *sURL, const WCHAR *sOutputFile);


void SaveSoundMovieAsWAVEFile(FNType *inFile, FNType *outFile) {
	HRESULT hr;
	WCHAR wInFile[FNSize], wOutFile[FNSize];

	u_strcpy(wInFile, inFile, FNSize);
	u_strcpy(wOutFile, outFile, FNSize);
	hr = CreateWavFile(wInFile, wOutFile);
}

void SaveSoundClip(sndInfo *SnTr, char isGetNewname) {
}
