/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 * args.h                                          * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/
#ifndef __VOCD_ARGS__
#define __VOCD_ARGS__

#include "wstring.h"
/* parse_args:
 * function for parsing a line into a set of words
 * returns no. of words and words returned in argv
 */

int   parse_args   (char * buf, char ***argv);

void  truncate_cha (FNType *fname);

#endif
