/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* maxconst.hpp
   - definition of various maximal constants for storage.
*/

#ifndef __MAXCONST_HPP__
#define __MAXCONST_HPP__

#define maxSizeOfHashInternal (10000L*sizeof(int))  /* max size of a structure in hashfile */

#define maxSizeOfInSent 39000L /* max size of sentence in input */
#define maxWordsInSentence 500L	/* max length of sentence */
// this corresponds to an average size of 70 characters per word.

#define maxSizeBA 64000L	/* max space to store ambiguous tags */

#define VMFILE_DATAFILENAME "vmfile.dat"
#define _IncrSizeMV_ 500L

#define SizeDynAlloc (1000L*Ko)
#define SizeBufAnalyze (196L*Ko)
#define SizeRespAnalyze (80L*Ko)
#define AbsoluteMaxNbTags 65536L
#define HashInitMaxNbTags 4096L
#define LocalWordMaxNbTags 640L
#define MaxNbTags 512L // 2048L
#define MaxNbSubtags 512L
#define SizeSupplement 2048L
#define MaxNbSolutions 2048L
#define sizelimitRepAlgo 2000L
#define MaxNbWords 4096L
#define MaxSizeOfString 2048L
#define MaxSzTag 512L
#define MaxSizeAffix 40L
#define morCodeInfoLength 64L

extern char* analyze_buf_r;
extern char* analyze_buf;

#endif
