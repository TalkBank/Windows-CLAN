/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 * dcompute.h                                      * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/
#ifndef __VOCD_DCOMPUTE__
#define __VOCD_DCOMPUTE__

#include "vocdefs.h"
#include "wstring.h"

double  ttr_fromseq     (char **token_seq, int tkns_in_seq, int *types);
 
void    d_profile       (double from, double to, double incr,
                         NT_Values * nt_tuples, int v);

double  t_eqn           (double d, int n);

double  d_eqn           (int n, double t);

double  find_min_d      (double d_av, NT_Values *nt_tuples, int no_NTvalues,
                         double *min_ls_val);

double  d_least_sq      (double d, NT_Values * nt_tuples, int no_nt_vals);
 
int     average_ttr     (char **token_seq,int tkns,int n,double *t,double *sd,
                         long no_samples, long flags);

void    segmented_ttr   (char **token_seq, int no_oftokens, int seg_size);

int     d_compute       (VOCDSP *speaker, char **token_seq, int tkns_in_seq, int from, int to,
                         int inc, int no_samples, long flags, FILE *ofp,
                         FNType *fname, double *d_optimum);
 
#endif
