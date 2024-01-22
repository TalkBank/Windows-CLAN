/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * dcompute.c                                              *
 * -  main functions for computing D_OPTIMUM.              *
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/


#include "cu.h"
#include <math.h>

#include "vocdefs.h"
#include "tokens.h"
#include "dcompute.h"
extern char vocd_show_warnings;

/*
 * ttr_fromseq
 * Compute the type-token ratio (TTR) for a sequence of words.
 * <token_seq>    - the sequence of words
 * <tkns_in_seq>  - the number of tokens in the sequence
 * <types>        - the number of types in the sequence (returned)
 * the TTR is returned, along with the number of types
 *
 */

double ttr_fromseq(char **token_seq, int tkns_in_seq, int *types)
{
   int t=0;
   NODE *root;
   double  ttr;
   int i;
 
   /* assemble the words into types */
   root = tokens_create_node(NO_SUBNODES);
 
   for (i=0;i<tkns_in_seq;i++)
      if (!tokens_member_simple(token_seq[i],root)) {
         if (tokens_insert(root, token_seq[i])==-1) 
            fprintf(stderr,"ttr_fromseq: token insertion failed\n");
         t++;
      }
 
   *types = t;
   ttr = (double ) t / (double ) tkns_in_seq;
 
   return ttr;
}


/*
 * d_profile
 * compute the least squares for a range of values of d
 * <from>     lower value of d
 * <to>       upper value of d
 * <inc>      measure by which we increment d
 * nt_tuples  structure containing <n,t>  values.
 * no_ntvals  the number of <n,t> values in nt_typles.
 *
 */

void d_profile(double from, double to, double incr, NT_Values * nt_tuples, int no_nt_vals)
{
   int j;
   double d;
   NT_Values *nt_ptr;
   double ls;

   for (d=from;d<to;d+=incr) {

      /* compute the least squares value for d */
      nt_ptr = nt_tuples;
      ls=0.0;
      for (j=0;j<no_nt_vals;j++) {
         double t;
         t = t_eqn(d, nt_ptr->N);
         ls+= (nt_ptr->T - t)*(nt_ptr->T - t);
         nt_ptr++;
      }
      printf("<d, ls> = <%f, %f> \n", d, ls);
   }
}



/*
 * t_eqn
 * compute the value of t given d and n
 *
 *   t = D/N . (Sqrt(1 + 2N/D) - 1)
 */
double t_eqn(double d, int n)
{
   double x;

   x = sqrt(1.0 + (2.0 * ((double) n)/d) ) -1.0;
   return ( d/(double) n ) * x ;
}


/*
 * d_eqn;
 * compute the value of D, given n and t.
 */
double d_eqn (int n, double t)
{
   double d;
   double tmp;

   tmp = 1.0 - t;
   if (tmp==0.0)
      return 0;

   d = 0.5* ( (((double) n)*t*t)/(1.0 - t) ) ;

   return d;
}


/*
 * find_min_d
 * Find the value of d that minimises the least squares difference
 * between the t-equation, and a set of <n.t> values.
 * Take the d_av value passed in as the starting value, find the
 * direction of descent, and follow it until the minimum point is found.
 * Assumes that the curve has a single stationary point (the minimum).
 * <d_av>        - the seeded d value.
 * <nt_tuples>   - the <n,t> values.
 * <no_NTvalues> - the number of <n,t> values.
 * <min_ls_val>  - the minimum least squares diference (returned).
 * Returns the value of D for which the least squares difference is
 * a minimum, and the minimum least squares difference.
 *
 */
double find_min_d (double d_av, NT_Values *nt_tuples, int no_NTvalues,
                     double *min_ls_val)
{
   int k;
   double d, diff, prev_d_least_sq, next;
 
   diff = d_least_sq( d_av, nt_tuples, no_NTvalues) -
             d_least_sq( d_av - 0.001, nt_tuples, no_NTvalues);

   if (diff > 0.0 )      /* decrement d val to reach min */
      k = -1;
   else if (diff < 0.0)  /* increment d val to reach min */ 
      k = +1;
   else                  /* diff = 0 ; assume at min */
      k = 0;
 
   if (k==0)
      return d_av;
 
   prev_d_least_sq = d_av;
   for (d=d_av; d>0.0 && d<(2*d_av); d+=(k*0.001)) {
       next = d_least_sq(d, nt_tuples, no_NTvalues);
       if (prev_d_least_sq < next) {
          break;   /* prev is the min value of d */
       }
       prev_d_least_sq = next;
   }
   *min_ls_val = prev_d_least_sq;
 
   return d;
}
 

/*
 * d_least_sq
 * Given a value for d, compute the least square difference between
 * the t equation using this value of d, and a curve defined by a
 * set of <n,t> values.
 * <d>          - the value of d to use in the t equation.
 * <nt_tuples>  - the set of <n,t> values.
 * <no_nt_vals> - the number of <n,t> values in the set.
 * Returns the least square value.
 */
double d_least_sq (double d, NT_Values * nt_tuples, int no_nt_vals)
{
   int j;
   NT_Values *nt_ptr;
   double ls;
   
   nt_ptr = nt_tuples;
   ls = (double) 0.0;
   for (j=0;j<no_nt_vals; j++) {
      double t;
      t = t_eqn(d, nt_ptr->N);
      ls+= (nt_ptr->T - t)* (nt_ptr->T - t);
      nt_ptr++;
   }
   return ls;
}




/*
 * average_ttr
 * Compute the average type-token ratio (TTR) for a token sequence.
 * Calculates the average TTR on the basis of a set of segments.
 * If the function is passed a positive number for <no_samples>, a
 * fixed number of segments is assumed, given by <no_samples>. Otherwise,
 * the number of segments is calculated from the number of tokens and
 * the integer <n>, representing the segment size. The former is used for
 * random selection of tokens for segments, the latter for splitting a
 * sequence of tokens in a sequence of segments. 
 * For random selection of tokens for segments, the TTR can be computed
 * in two basic ways. The first involves the replacement of tokens, once they
 * have been selected. The second involves the non-replacements of tokens,
 * so that a token can only be selected once.
 *
 * token_seq = the sequence of tokens.
 * tkns      = the number of tokens in the sequence.
 * n         = size of segment for which the ttr is to be calculated.
 * t         = the average ttr value.
 * sd        = the standard deviation.
 * no_samples= for random selection of tokens, gives fixed no of segment samples.
 * flags     = used in various ways - in this current case for debugging
 *             info, but we also want it for random info.
 * returns 1 if successful, -1 otherwise.
 *
 */

int average_ttr (char **token_seq,int tkns,int n,double *t,double *sd, long no_samples, long flags)
{
	int      tokens, types=0;
	char   **tkn_ptr;
	char   **rtoken_list = NULL;
	NODE    *root=NULL;
	long     no_segments;
	double   ttr;
	double   std;
	int      i, j, index;
	double  *ttr_list;    
	int     *replacement_list = NULL;

	/* determine the number of segments to be used to calculate the average TTR */
	if ((flags & RANDOM_TTR) && no_samples)
		no_segments = no_samples;
	else
		no_segments = tkns/n;
	/* an array to collect the sequence of ttr values to be averaged. */
	ttr_list = (double *) malloc (no_segments * sizeof(double));
	if (ttr_list == NULL) {
		fprintf(stderr, "no memory for ttr values\n");
		return -1;
	}
	if (flags & RANDOM_TTR) {
		rtoken_list = (char **) malloc (n * sizeof(char*));
		if (rtoken_list == NULL) {
			fprintf(stderr,"average_ttr: no memory\n");
			return -1;
		}
		/* Array to hold information for tokens if using NOREPLACEMENT */
		if (flags & NOREPLACEMENT) {
			replacement_list = (int *) malloc (tkns * sizeof(int));
			if (replacement_list == NULL) {
				fprintf(stderr, "average_ttr: no memory\n");
				return -1;
			}
			memset(replacement_list,0,tkns*sizeof(int));
		}
	}
	for (i=0;i<no_segments;i++) {
		/* Generate the list of tokes that forms the segment */
		if (flags & RANDOM_TTR) {   /* select the segment's tokens at random */
			if (flags & NOREPLACEMENT) {
				for (j=0;j<tkns;j++)
					replacement_list[j]=0;
			}
			for (j=0;j<n;j++)  {
				index = rand() % tkns;
				if (flags & NOREPLACEMENT) { /* no replacement */
					if (replacement_list[index] != 1) { /* select and tag */
						rtoken_list[j] = token_seq[index];
						replacement_list[index] = 1;
					}
					else  { /* decrement and go around again */
						j--;
					}
				}
				else {  /* replacement */
					rtoken_list[j] = token_seq[index];
				}
			}
			tkn_ptr = rtoken_list;
		}
		else    /* the segment's tokens form part of the token sequence */
			tkn_ptr = token_seq + i*n;
		/* For the segments, find the TTR value and store in the ttr_list */
		tokens = n;
		if (root!=NULL)
			tokens_free_tree(root);
		root = tokens_create_node(NO_SUBNODES);
		if (root==NULL) {
			fprintf(stderr,"average_ttr: cannot create list\n");
			free(ttr_list);
			return -1;
		}
		types = 0;
		while (tokens--)  {
			if (!tokens_member_simple(*tkn_ptr,root)) {
				tokens_insert(root, *tkn_ptr);
				types++;
			}
			tkn_ptr++;
		}
		ttr_list[i] = (double) types / (double) n;
	}
	/* calc the average TTR value */
	ttr = 0.0;
	for (i=0;i<no_segments;i++)
		ttr+=ttr_list[i];
	*t = ttr/ (double) no_segments;
	/* calc st.dev. */
	std = 0.0;
	for (i=0; i<no_segments; i++)
		std+=( (ttr_list[i]-*t)*(ttr_list[i]-*t) );
	*sd = sqrt ( (double) (std / (double) no_segments)  ) ;
	free(ttr_list);
	/* tidy up */
	if (flags & RANDOM_TTR) {
		if (flags & NOREPLACEMENT) {
			free(replacement_list);
		}
		free(rtoken_list);
	}
	return 1;
}

/*
 * segmented_ttr
 * This is no longer used, but might be useful again.
 * Compute the TTR for a sequence of segments, given the
 * segments size and a sequence of tokens. The segments are
 * end-to-end along the token sequence.
 * The list of words not used in the calculation, because they
 * do not fit into a complete segments, are also printed out.
 * 
 * <token_seq>     - the sequence of tokens.
 * <no_oftokens>   - number of tokens in the sequence.
 * <seg_size>      - the size of the segments.
 *
 */
void segmented_ttr(char **token_seq, int no_oftokens, int seg_size)
{
   int tokens, types=0;
   char **words;
   NODE *root=NULL;
   int no_ofsegments;
   int left_over;
   double ttr;
   int i;

   /* no of segments */
   no_ofsegments = no_oftokens/seg_size;

   left_over = no_oftokens%seg_size;

   for (i=0;i<no_ofsegments;i++) {
      /* calc the ttr for each segments */
      words = token_seq + i*seg_size;
      tokens = seg_size;

      if (root!=NULL)
         tokens_free_tree(root);
      root = tokens_create_node(NO_SUBNODES);
      if (root==NULL) {
         fprintf(stderr,"segment_ttr: cannot create list\n");
         return;
      }

      types = 0;
      while (tokens--)  {
         if (!tokens_member_simple(*words,root)) {
            tokens_insert(root, *words);
            types++;
         }
         words++;
      }

      /* print the sequence of words */
      tokens = seg_size;
      words = token_seq + i*seg_size;
      printf("Segment(%i) <%i>: ",i, seg_size);
      while (tokens--) {
         printf("%s ",*words++);
      }
      printf("\n");

      ttr = (double) types / (double) seg_size;
      printf("<types,tokens,ttr> = <%i,%i,%f>\n", types, seg_size, ttr);

   }

   /* show info on the remaining unused words */
   if (left_over >0) {
      words = token_seq + no_ofsegments*seg_size;
      printf("left over <%i>:",left_over);
      while (left_over--) {
         printf("%s ",*words++);
      }
      printf("\n");
   }
}


/*
 * d_compute
 * 
 * Compute values of d.
 * if flag is RANDOM, compute three values using a defined set of
 *    samples selected at random, of a particular size.
 * if flag is not RANDOM, compute as with 't' - a single value of d.
 * <token_seq>     - the sequence of tokens.
 * <tkns_in_seq>   - the number of tokens in the sequence.
 * <from>          - the lower value of d.
 * <to>            - the upper value of d.
 * <incr>          - the step size of d.
 * <no_samples>    - the number of segments to be used in the calculation
 *                   of the average value of TTR.
 * <flags>         - various flags to direct the program.
 * <ofp>           - the file to which the data are to be output.
 * <fname>         - the name of the file whose data is being processed.
 * <d_optimum>     - the variable in which the computed d_optimum value is
 *                   returned. Needed for LIMITING_TYPE_TYPE_RATIO.
 */

int d_compute (VOCDSP *speaker, char **token_seq, int tkns_in_seq, int from, int to,
                int incr, int no_samples, long flags, FILE * ofp,
                 FNType *fname, double *d_optimum)
{
	int no_NTvalues;
	NT_Values *nt_tuples = NULL, *nt_ptr;
	D_Values d;
	int N;
//	long pcnt, tcnt;
	char *t;
	double T, SD;
	int discard = 0;
	int i, j;
	double d_av;
	double d_std;
	double d_min, min_ls_val;
	int no_trials;
	DMIN_Values dmin_trials[MAX_TRIALS];
	double dmin_av = 0.0;
	int types;
	double ttr;

	if (flags & RANDOM_TTR)
		no_trials = MAX_TRIALS;
	else
		no_trials = 1;
	if (tkns_in_seq==0) {
		if (vocd_show_warnings) {
			fprintf(stderr,"\n*** File \"%s\": Speaker: \"%s\"\n", fname, speaker->code);
			fprintf(stderr, "vocd: WARNING: Token count is zero\n\n");
		}
//		if (ofp != NULL)
//			fprintf(stderr, "vocd: Terminating ...\n");
		if (ofp != NULL && onlydata == 4) {
			if (speaker->fname != NULL)
				outputStringForExcel(ofp, speaker->fname, 1);
			else {
				char *f;
// ".xls"
				strcpy(FileName2, oldfname);
				f = strrchr(FileName2, PATHDELIMCHR);
				if (f != NULL) {
					f++;
					strcpy(FileName2, f);
				}
				f = strchr(FileName2, '.');
				if (f != NULL)
					*f = EOS;
				if (!combinput || onlydata == 4)
					fprintf(ofp,"%s,", FileName2);
				else
					fprintf(ofp,"%s,", POOLED_FNAME);
			}
			if (speaker->IDs != NULL)
				outputIDForExcel(ofp, speaker->IDs, 1);
			else {
				fprintf(ofp,".,.,%s,.,.,.,.,.,.,.,.,", speaker->code);
			}
			fprintf(ofp,".,.,.,");
			for (j=0;j<no_trials;j++) {
				fprintf(ofp,".,");
			}
			fprintf(ofp,".\n");
		}
		return 3;
	}
	if ( (flags & NOREPLACEMENT) && (tkns_in_seq < to) ) {
		if (vocd_show_warnings) {
			fprintf(stderr,"\n*** File \"%s\": Speaker: \"%s\"\n", fname, speaker->code);
			fprintf(stderr, "vocd: WARNING: Not enough tokens for random sampling without replacement.\n\n");
		}
//		if (ofp != NULL)
//			fprintf(stderr, "vocd: Terminating ...\n");
		if (ofp != NULL && onlydata == 4) {
			if (speaker->fname != NULL)
				outputStringForExcel(ofp, speaker->fname, 1);
			else {
				char *f;
// ".xls"
				strcpy(FileName2, oldfname);
				f = strrchr(FileName2, PATHDELIMCHR);
				if (f != NULL) {
					f++;
					strcpy(FileName2, f);
				}
				f = strchr(FileName2, '.');
				if (f != NULL)
					*f = EOS;
				if (!combinput || onlydata == 4)
					fprintf(ofp,"%s,", FileName2);
				else
					fprintf(ofp,"%s,", POOLED_FNAME);
			}
			if (speaker->IDs != NULL)
				outputIDForExcel(ofp, speaker->IDs, 1);
			else {
				fprintf(ofp,".,.,%s,.,.,.,.,.,.,.,.,", speaker->code);
			}
			fprintf(ofp,".,.,.,");
			for (j=0;j<no_trials;j++) {
				fprintf(ofp,".,");
			}
			fprintf(ofp,".\n");
		}
		return 2;
	}
//	pcnt = no_trials * (to - from + 1);
//	tcnt = pcnt;
	for (j=0; j<no_trials; j++) {
		/* calculate the number of <n,t> values to be returned */
		no_NTvalues = (to-from)/incr + 1;
		nt_tuples = (NT_Values *) malloc (no_NTvalues*sizeof(NT_Values));
		if (nt_tuples==NULL) {
			fprintf(stderr, "NT tupes; no memory\n");
			return -1;
		}
		memset(nt_tuples,0,no_NTvalues*sizeof(NT_Values));
		/* initialise the D data structure */
		d.nt_tuples = nt_tuples;
		d.no_tuples = no_NTvalues;
		d.D   = 0.0;
		d.STD = 0.0;
		/* calculate average TTR values for a range of segment sizes */
		nt_ptr = nt_tuples;
		for (N=from;N<=to;N+=incr) {
/*
			if (pcnt < tcnt) {
				tcnt = tcnt - 1;
#if !defined(CLAN_SRV)
				fprintf(stderr, "\r%ld ", pcnt);
				my_flush_chr();
#endif
			}
			pcnt--;
*/
			/* compute <n,t> values */
			average_ttr (token_seq,tkns_in_seq,N,&T,&SD, no_samples, flags); 
			nt_ptr->N  = N;
			if ((flags & RANDOM_TTR) && no_samples) /* ran. samp.+ fixed size */
				nt_ptr->S = no_samples; 
			else 
				nt_ptr->S  = tkns_in_seq/N;
			nt_ptr->T  = T;
			nt_ptr->SD = SD;
			nt_ptr->D  = d_eqn(N,T);
			nt_ptr++;
		}
		if (ofp != NULL && onlydata == 0) {
			fprintf(ofp, "\ntokens  samples    ttr     st.dev      D\n");
			for (i=0; i<no_NTvalues; i++) {
				fprintf(ofp, "  %-5i   %-5i  %.4f %8.3f %10.3f\n",
						nt_tuples[i].N, nt_tuples[i].S, nt_tuples[i].T,
						nt_tuples[i].SD, nt_tuples[i].D);
			}
		}
		/* calc mean value of d */
		discard = 0;
		d_av    = 0.0;
		for (i=0;i<no_NTvalues;i++) {
			if (nt_tuples[i].D == 0.0) {
				discard++;
				continue;
			}
			d_av+=nt_tuples[i].D;
		}
		d_av = d_av / (double) (no_NTvalues - discard);
		/* calc st.dev. */
		discard = 0;
		d_std = 0.0;
		for (i=0; i<no_NTvalues; i++) {
			if (nt_tuples[i].D == 0.0) {
				discard++;
				continue;
			}
			d_std+=( (nt_tuples[i].D-d_av)*(nt_tuples[i].D-d_av) );
		}
		d_std = sqrt ( d_std/ ( (double) (no_NTvalues-discard))  );
		d.D   = d_av;
		d.STD = d_std;
		if (ofp != NULL && onlydata == 0)
			fprintf(ofp,"\nD: average = %.3f; std dev. = %.3f\n", d.D, d.STD);

		/* try to find the min d value */
		d_min = find_min_d (d_av, nt_tuples, no_NTvalues, &min_ls_val); 
		if (ofp != NULL && onlydata == 0)
			fprintf(ofp,"D_optimum     <%.2f; min least sq val = %.3f> \n", d_min, min_ls_val);
		dmin_trials[j].dmin = d_min;
		dmin_trials[j].mls  = min_ls_val;
		free(nt_tuples);
	} /* end for */
/*
#if !defined(CLAN_SRV)
	fprintf(stderr, "\r          \r");
	my_flush_chr();
#endif
*/
	if (onlydata == 0) {
		if (ofp != NULL) {
			fprintf(ofp,"\n");
			fprintf(ofp,"VOCD RESULTS SUMMARY\n");
			fprintf(ofp,"====================\n");
		}
		ttr = ttr_fromseq (token_seq, tkns_in_seq, &types);
		if (ofp != NULL)
			fprintf(ofp,"   Types,Tokens,TTR:  <%i,%i,%f>\n", types, tkns_in_seq, ttr);
		dmin_av = 0.0;
		if (ofp != NULL)
			fprintf(ofp,"  D_optimum  values:  <");
		for (j=0;j<no_trials;j++) {
			if (ofp != NULL)
				fprintf(ofp,"%.2f", dmin_trials[j].dmin);
			if (ofp != NULL && j<no_trials-1)
				fprintf(ofp,", ");
			dmin_av += dmin_trials[j].dmin ;
		}
		if (ofp != NULL)
			fprintf(ofp,">\n");
		dmin_av = dmin_av/( (double) no_trials );
		if (ofp != NULL) {
			fprintf (ofp,"  D_optimum average:  %.2f\n", dmin_av);
			fprintf (ofp,"\n");
		}
	} else if (onlydata == 2) {
		if (ofp != NULL)
			fprintf(ofp,"File_name:	");
		if (ofp != NULL) {
			t = strchr(fname, '.');
			if (t != NULL) {
				*t = '\0';
				fprintf(ofp,"%s	", fname);
				*t = '.';
			} else
				fprintf(ofp,"%s	", fname);
		}
		ttr = ttr_fromseq (token_seq, tkns_in_seq, &types);
		if (ofp != NULL)
			fprintf(ofp,"Types:	%i	Tokens:	%i	TTR:	%f	", types, tkns_in_seq, ttr);
		dmin_av = 0.0;
		if (ofp != NULL)
			fprintf(ofp,"D_optimum_values:	");
		for (j=0;j<no_trials;j++) {
			if (ofp != NULL)
				fprintf(ofp,"%.2f	", dmin_trials[j].dmin);
			dmin_av += dmin_trials[j].dmin ;
		}
		dmin_av = dmin_av/( (double) no_trials );
		if (ofp != NULL) {
			fprintf(ofp,"D_optimum_average:	%.2f", dmin_av);
			fprintf(ofp,"\n");
		}
	} else if (onlydata == 4) {
		if (ofp != NULL) {
			if (speaker->fname != NULL)
				outputStringForExcel(ofp, speaker->fname, 1);
			else {
				char *f;
// ".xls"
				strcpy(FileName2, oldfname);
				f = strrchr(FileName2, PATHDELIMCHR);
				if (f != NULL) {
					f++;
					strcpy(FileName2, f);
				}
				f = strchr(FileName2, '.');
				if (f != NULL)
					*f = EOS;
				if (!combinput || onlydata == 4)
					fprintf(ofp,"%s,", FileName2);
				else
					fprintf(ofp,"%s,", POOLED_FNAME);
			}
			if (speaker->IDs != NULL)
				outputIDForExcel(ofp, speaker->IDs, 1);
			else {
				fprintf(ofp,".,.,%s,.,.,.,.,.,.,.,.,", speaker->code);
			}
		}
		ttr = ttr_fromseq (token_seq, tkns_in_seq, &types);
		if (ofp != NULL)
			fprintf(ofp,"%i,%i,%.3f,", types, tkns_in_seq, ttr);
		dmin_av = 0.0;
		for (j=0;j<no_trials;j++) {
			if (ofp != NULL)
				fprintf(ofp,"%.2f,", dmin_trials[j].dmin);
			dmin_av += dmin_trials[j].dmin ;
		}
		dmin_av = dmin_av/( (double) no_trials );
		if (ofp != NULL) {
			fprintf(ofp,"%.2f", dmin_av);
			fprintf(ofp,"\n");
		}
	}
	*d_optimum = dmin_av;
	return 1;
}


