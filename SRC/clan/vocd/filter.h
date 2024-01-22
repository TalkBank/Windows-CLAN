/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 * filter.h                                        * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/
#ifndef __VOCD_FILTER__
#define __VOCD_FILTER__


#include "vocdefs.h"
#include "tokens.h"
#include "speaker.h"


int   filter_enclosed        (Line *line, int c1, int c2, long flags);
int   filter_angle           (Line *line, int c1, int c2, long flags);

//void  filter_parentheses     (VOCDSPs *speakers, long flags);

void  filter_exclude         (Line *line, NODE *exclude);
void  filter_terminators     (Line *line, NODE *terminators);

//void  filter_split_at        (VOCDSPs *speakers, char * string);
//void  filter_split_before    (VOCDSPs *speakers, char * string);
//void  filter_split_after     (VOCDSPs *speakers, char * string);
//void  filter_split_tail      (VOCDSPs *speakers, char * string);

//void  filter_lowercase       (VOCDSPs *speakers);
void  filter_capitalisation  (VOCDSP *speaker);

int   filter_overlapping     (Line *line, long flags );


#endif
