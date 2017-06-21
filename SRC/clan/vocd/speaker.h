/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 *  speaker.h                                      * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/
#ifndef __VOCD_SPEAKER__
#define __VOCD_SPEAKER__

extern "C"
{
	extern VOCDSPs *speakers;
	extern VOCDSPs *denom_speakers;

	extern VOCDSP  *speaker_add_speaker       (VOCDSPs ** s, char *name, char *IDs, char isTemp, char isUsed);
//	extern int     speaker_exclude_speaker    (VOCDSPs ** s, char *name, char isTemp);
	extern int     speaker_add_line           (VOCDSP * speaker, char *pt, long lineno);
	extern int     speaker_append_to_line     (VOCDSP * speaker, char *pt );

//	extern int     speaker_add_dependent_tier (Tier **d, char *description) ;
//	extern void    speaker_fprint_tiers       (FILE *ofp, VOCDSPs *speakers, Tier *dependents);
//	extern void	   speaker_free_up_tier	      (Code *c);
//	extern Code *  speaker_insert_tier        (Code *c, char *name);
	extern int     speaker_create_sequence    (VOCDSP *speaker, char ***token_seq);
//	extern void    speaker_utterances         (VOCDSPs *speakers, FILE *ofp);
	extern void    speaker_free_line          (Line *line);

//	extern int     speaker_parse_file         ( FILE * fp, VOCDSPs **speakers, Tier * dependent_tiers);

//	extern void    speaker_print_speaker      (VOCDSP *speaker );
	extern void    speaker_print_line         (int n, char **w);
	extern void    speaker_fprint_speaker     (FILE *ofp, VOCDSP *speaker );
	extern void    speaker_fprint_line        (FILE *ofp, int n, char **w);

	extern int     speaker_tokencount         (VOCDSP *speaker);
	extern int     speaker_linecount          (VOCDSP *speaker);
	extern double  speaker_ttr                (VOCDSP *speaker, int *typs, int *tkns );

//	extern VOCDSPs * speaker_copy_speaker     (VOCDSPs *original);
	extern VOCDSPs * speaker_free_up_speakers (VOCDSPs *p, char isAll);
	extern void    clean_speakers			  (VOCDSPs *Spk_pt);
}

#endif
