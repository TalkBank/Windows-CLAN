/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- anaalgo.cpp	- v1.0.0 - november 1998

	This file contains the main analysis procedure.

int analyze_algorithm( AMBTAG* listOfAmbiguousTags, int numberOfTags, AMBTAG* &Result, int* annotation, int style );
	- the input data is an array of ambiguous tags, each is one of the possible tags for the nth word. The tag order
	  in each ambiguous tag is the order of tags in the dictionary. It is important only for rules retrieval.
	- numberOfTags is the size of the input data (number of element of the array)
	- the output result is an  array of ambiguous tags (same kind of data as input data). But the tag order
	  in each ambiguous tag is very significant. The first tag is the most probable one, the second (if any) the 
	  second most probable and so on.
	  NOTE: this parameter is allocated by the function analyze_algorithm and stays allocated until the next call to 	  analyze_algorithm.
	- the parameter 'annotation' is an output parameter. It provides for each word an information about the analysis
	  process performed. Information are : 
		AnaNote_norule		==> no ambiguous rule could be found for this word.
		AnaNote_sizelimit  	==> a computation limit has been reached for this word.
		AnaNote_noresolution	==> no solution could be computed for this word, so the word has still 
					    the original ambiguous tag.
		AnaNote_unknownword	==> the word was unknown.
	- style : indicate to print information for debuging purpose of the POST grammar.
		style = 5 - simple version 
		style = 6 - full version
		style = 7 - debug soft and POST version
	- FILE* out : output file for style >= 5

	The different steps of the analysis are:
		STEP1) retrieve the whole set of ambiguous rules
			(Example: X1|X2 followed by X3|X4|X5 gives X1 followed by X5)
		STEP2) pseudo-recursive algorithm that computed all possible paths
		STEP3) reevaluation of the paths with the 3D matrix (optional)

		STEP3bis) if the STEP2 provides no solution, a replacement algorithm is used. This algorithm which 
			  is less powerfull always returns a solution.

other functions:
int replacement_analyze_algorithm( AMBTAG* A, int n, AMBTAG* &R, int* annotation, int** M ); (STEP3bis)

|=========================================================================================================================*/

#include <time.h>
#include "system.hpp"

// #define out0
// #define out1
// #define out2
// #define out3

#include "database.hpp"

/*===========================================
TOOLS FUNCTIONS used localy in the current file
*/
void	isort(int* ob, int ta);
void	fisort(double* ob, int* co, int ta);
void	fksort(double* ob, int* co, int ta);
int     linrec_analyze_algorithm(int** M, int word_count, int* solutions, int maxnbsol, double* DD );
int replacement_analyze_algorithm( AMBTAG* A, int n, AMBTAG* &R, int* annotation, int** M );

// R is allocated by analyze_algorithm until the next call to analyze_algorithm.
int analyze_algorithm( AMBTAG* A, int n, AMBTAG* &R, int* annotation, int style, FILE* out ) // , int* taglocalwords )
{
	int res;

	if ( n<=2 ) { // no analysis can be performed.
		// compute Result Format:
		char* analyze_buf_r = dynalloc(SizeRespAnalyze);	// size to be checked...
		if ( !analyze_buf_r ) return -1;
		R = (AMBTAG*) analyze_buf_r;			// --
		TAG* Rd = (TAG*)(R+n);			// --

		// skip first word in the sentence because it is a 'pct' for the analysis only.
		R[0] = Rd;
		Rd += 2;
		R[0][0] = 2;
		R[0][1] = A[0][1];

		if ( n<=1 ) return 1;

		R[1] = Rd;
		R[1][0] = A[1][0]+1;
		for (int i=1; i<R[1][0]; i++ ) R[1][i] = A[1][i];
		return n;
	}

// STEP1 -------------------------------------------------------------------------------------------------
// find rules resolutions. Find which bitags correspond to biambtags
	char* analyze_buf = dynalloc(SizeBufAnalyze);	// size to be checked...
	if ( !analyze_buf ) return -1;
	memset ( analyze_buf, 0, SizeBufAnalyze );
	int ** M = (int**)analyze_buf;		// --
	int * Md = (int*)(M+n-1);		// --
	int * MaxMd = (int*)M+(SizeBufAnalyze/sizeof(int));
	int i;

	for (i=0; i<n-1; i++) {
		TAG AA[MaxNbTags];
		int p1 = number_of_ambtag(A[i])+1 ;
		memcpy( &AA[0], A[i], p1*sizeof(TAG) );
		int p2 = number_of_ambtag(A[i+1])+1 ;
		memcpy( &AA[p1], A[i+1], p2*sizeof(TAG) );

#ifdef out2
		msg( ">>>\t%d : ", i );
		print_ambtag( A[i], stdout );
		msg( " --- " );
		print_ambtag( A[i+1], stdout );
		msg( "\n" );
#endif
		int* x = (int*)hashGetII( _rules_hashdic_, AA, (p1+p2)*sizeof(TAG) );
		if (x) {
			M[i] = Md;
			int s = size_of_multclass(x);
			memcpy( Md, x, s*sizeof(int) );
			if ( Md+s >= MaxMd ) return 0;
			Md += s;
		} else {
			annotation [i] |= AnaNote_norule;
			// expand the two ambtags
#ifdef out2
		msg( "!expand! : ");
#endif
			M[i] = Md;
			M[i][0] = (p1-1)*(p2-1)*2+2;
			M[i][1] = (p1-1)*(p2-1);
			for ( int v1=0; v1<p1-1; v1++ )
				for ( int v2=0; v2<p2-1; v2++ ) {
					M[i][2+(v1*(p2-1)+v2)*2] = tags_to_bitag( nth_of_ambtag(A[i],v1), nth_of_ambtag(A[i+1],v2) );
					M[i][2+(v1*(p2-1)+v2)*2+1] = 0;
				}
			if (Md + M[i][0]>=MaxMd) return 0;
			Md += M[i][0];
#ifdef out2
		msg( ">>>\t%d : ", i );
		print_bimultclass( M[i], stdout );
		msg( "\n" );
#endif
		}
	}

#ifdef out2
	msgfile( out, "\tList of entries in the rule dictionary\n");
	for ( i=0; i<n-1; i++ ) {
		msgfile (out, "\t%d : ", i );
		print_bimultclass( M[i], out );
		msgfile (out, "\n" );
	}
#endif

// The elements of the M array are bi_multclasses, i.e. arrays of bitags
// this means that the contents is something like that (but tags are represented as numbers)
	// M[0]      M[1]     M[2]
	// det-s     s-v      det-n
	// prn-v     prn-v    co-co
	// det-co    ...      adj-n
	// ...                ...


	// ACCESS to elements : size of M[i] ==> number_of_multclass(M[i])
	// ACCESS to nth element of M[i] : M[i][x] ==> nth_of_multclass(M[i],x)
	// ACCESS to left-part of the nth element of M[i] : M[i][x]->left ==> first_of_bitag(nth_of_multclass(M[i],x))
	// ACCESS to right-part of the nth element of M[i] : M[i][x]->right ==> second_of_bitag(nth_of_multclass(M[i],x))
	// ACCESS to the weight of the nth element of M[i] : M[i][x]->count ==> nthcount_of_multclass(M[i],x)
if (style == 6) {
	msgfile( out, "\tList of entries in the rule dictionary\n");
	for ( i=0; i<n-1; i++ ) {
		msgfile( out, "\t%d : ", i );
		print_ambtag( A[i], out );
		msgfile( out, " - " );
		print_ambtag( A[i+1], out );
		msgfile( out, " - M[%d]=", i );
		print_bimultclass( M[i], out);
		msgfile( out, "\n" );
	}
}

	if (n>100) {
if (style == 6) {
		msgfile( out, "\nReplacement algorithm (n>100)\n" );
}
		res = replacement_analyze_algorithm( A, n, R, annotation, M );
		if (style == 6) {
			msgfile( out, "\tList of entries (after replacement_analyze_algorithm)\n");
			for ( i=0; i<n-1; i++ ) {
				msgfile( out, "\t%d : ", i );
				print_ambtag( A[i], out );
				msgfile( out, " - " );
				print_ambtag( A[i+1], out );
				msgfile( out, " - M[%d]=", i );
				print_bimultclass( M[i], out);
				msgfile( out, " - R[%d]=", i);
				print_ambtag(R[i],out);
				msgfile(out, "\n" );
			}
		}
		return res;
	}

	int *S = Md;
	double DD [MaxNbSolutions];
	
	int nbsol = linrec_analyze_algorithm(M, n-1, S, (MaxMd-S)/n>MaxNbSolutions?MaxNbSolutions:(MaxMd-S)/n, DD );
//	int nbsol = 0; // to test the replacement algorithm
if (style == 6) {
	msgfile( out, "\n\tNbSol linrec_algo n=%d, nbsol=%d\n", n, nbsol );
	for (int ii=0; ii<nbsol; ii++) {
		msgfile( out, "LR> ");
		msgfile( out, "%f ", DD[ii] );
		msgfile( out, " ");
		msgfile( out, "\n");
	}
	msgfile( out, "\n\tList of entries (after linrec_analyze_algorithm)\n");
	for ( i=0; i<n-1; i++ ) {
		msgfile( out, "\t%d : ", i );
		print_ambtag( A[i], out );
		msgfile( out, " - " );
		print_ambtag( A[i+1], out );
		msgfile( out, " - M[%d]=", i );
		print_bimultclass( M[i], out);
		msgfile(out, "\n" );
	}
}

	if (nbsol<=0||nbsol>MaxNbSolutions) {
if (style == 6) {
		msgfile( out, "\nReplacement algorithm (nbsol<=0||nbsol>MaxNbSolutions)\n" );
}
		res = replacement_analyze_algorithm( A, n, R, annotation, M );
		if (style == 6) {
			msgfile( out, "\tList of entries (after replacement_analyze_algorithm)\n");
			for ( i=0; i<n-1; i++ ) {
				msgfile( out, "\t%d : ", i );
				print_ambtag( A[i], out );
				msgfile( out, " - " );
				print_ambtag( A[i+1], out );
				msgfile( out, " - M[%d]=", i );
				print_bimultclass( M[i], out);
				msgfile( out, " - R[%d]=", i);
				print_ambtag(R[i],out);
				msgfile(out, "\n" );
			}
			msgfile(out, "\n" );
		}
		return res;
	}

	int    ind[MaxNbSolutions];
	for ( i=0; i<nbsol; i++ ) {
		ind[i] = i;
	}

	fisort( DD, ind, nbsol );

	if (style == 6) {
		msgfile (out, "\t%d solutions after binary rules analysis\n", nbsol );
		for ( i=0; i<nbsol; i++ ) {
			msgfile (out, "\t%4d %4.2e ", i+1, DD[i] );
	//		msgfile (out, "\t%4d %4d ", i+1, (S+(n)*ind[i])[0] );
			for ( int j=1; j<n; j++ ) {
				char s2[128];
				tag_to_classname( (S+(n)*ind[i])[j], s2 );
				msgfile (out, "%s ", s2 );
			}
			msgfile (out, "\n" );
		}

		msgfile (out, "\tmatrix information size: %d\n", width_of_matrix() );
	}

	if ( _matrix_hashdic_ != -1 ) {
		int k; /* lxs new fix/addition */
	    // reevaluate using the matrix for the 'n' best equal solutions.

	    if (DD[0] < .000001) { /* case of unknown rules : matrix is necessary */
		for (k=0; k<nbsol; k++) {
			double tottotal = 0.0;
			double tot = 1.0;
			for ( i=1; i<n-1; i++ ) {
				// makes matrix element with 2 elements
				TAG ME[3];
				ME[0] = 2;
				ME[1] = (S+(n)*(k))[i] ;
				ME[2] = (S+(n)*(k))[i+1] ;
				// test if ME exists.
				Int4 r = hashGetIN( _matrix_hashdic_, ME, 3*sizeof(TAG) );
#ifdef out0
msg("-- %d %f ",r,tot);
print_ambtag(ME,stdout);
msg("\n");
#endif
				if ( r == -1 ) {
					tot = 0;
					break ;
				} else
					tot = tot * r ;
			}
#ifdef out0
msg("-end sum- %f %f\n",tot,tottotal);
#endif
			tottotal = tot;
			DD[k] = tottotal;
		}

		for ( i=0; i<nbsol; i++ )
			ind[i] = i;
		fisort( DD, ind, k );

		if ( DD[0] == 0.0 ) {	// second try allowing transitions to be equal to 0.
			for (k=0; k<nbsol; k++) {/* lxs new fix/addition */
				double tottotal = 0.0;
				double tot = 1.0;
				for ( i=1; i<n-1; i++ ) {
					// makes matrix element with 2 elements
					TAG ME[3];
					ME[0] = 2;
					ME[1] = (S+(n)*(k))[i] ;
					ME[2] = (S+(n)*(k))[i+1] ;
					// test if ME exists.
					Int4 r = hashGetIN( _matrix_hashdic_, ME, 3*sizeof(TAG) );
#ifdef out0
msg("-- %d %f ",r,tot);
print_ambtag(ME,stdout);
msg("\n");
#endif
					if ( r != -1 )
						tot = tot * r ;
					// else transition is 0.0, do nothing (i.e. multiply by 1.0)
				}
#ifdef out0
msg("-end sum- %f %f\n",tot,tottotal);
#endif
				tottotal = tot;
				DD[k] = tottotal;
			}

			for ( i=0; i<nbsol; i++ )
				ind[i] = i;
			fisort( DD, ind, k );

		}
	    } else if ( width_of_matrix()>=3 ) { /* case of known rules : matrix is not necessary */
		for (k=0; k<nbsol; k++) {/* lxs new fix/addition */
			double tottotal = 0.0;
			double tot = 1.0;
			tot = 1;
			for ( i=1; i<n-2; i++ ) {
				// makes matrix element with 3 elements
				TAG ME[4];
				ME[0] = 3;
				ME[1] = (S+(n)*(k))[i] ;
				ME[2] = (S+(n)*(k))[i+1] ;
				ME[3] = (S+(n)*(k))[i+2] ;
				// test if ME exists.
				Int4 r = hashGetIN( _matrix_hashdic_, ME, 4*sizeof(TAG) );
				if ( r == -1 ) {
					tot = 0;
					break ;
				} else
					tot = tot * r ;
				}
			tottotal = tot;
			if ( width_of_matrix()>=4 ) {
				tot = 1;
				for ( i=1; i<n-3; i++ ) {
					// makes matrix element with 4 elements
					TAG ME[5];
					ME[0] = 4;
					ME[1] = (S+(n)*(k))[i] ;
					ME[2] = (S+(n)*(k))[i+1] ;
					ME[3] = (S+(n)*(k))[i+2] ;
					ME[4] = (S+(n)*(k))[i+3] ;
					// test if ME exists.
					Int4 r = hashGetIN( _matrix_hashdic_, ME, 5*sizeof(TAG) );
					if ( r == -1 ) {
						tot = 0;
						break ;
					} else
						tot = tot * r ;
				}
				tottotal *= tot;
			}
			if (tot < .000001) {
				// could not find a path using matrix so come back to weights using rules = ignore new sort.
				if (style == 6) {
					msgfile( out, "\tcould not find a path using matrix so using rules weights\n" );
				}
				goto ignore_matrix;
			}
			DD[k] = tottotal;
		}

		for ( i=0; i<nbsol; i++ )
			ind[i] = i;
		fisort( DD, ind, k );
	    }		
	}

	if (style == 6) {
		msgfile (out, "\torder of the %d solutions after matrix analysis\n", nbsol );
		for ( i=0; i<nbsol; i++ ) {
			msgfile (out, "\t%4d %4.2e ", i+1, DD[i] );
	//		msgfile (out, "\t%4d %4d ", i+1, (S+(n)*ind[i])[0] );
			for ( int j=1; j<n; j++ ) {
				char s2[128];
				tag_to_classname( (S+(n)*ind[i])[j], s2 );
				msgfile (out, "%s ", s2 );
			}
			msgfile (out, "\n" );
		}
	}

ignore_matrix:

	// compute Result Format:
	char* analyze_buf_r = dynalloc(SizeRespAnalyze);	// size to be checked...
	R = (AMBTAG*) analyze_buf_r;			// --
	TAG* Rd = (TAG*)(R+n);			// --

	// skip first word in the sentence because it is a 'pct' for the analysis only.
	R[0] = Rd;
	Rd += 2;
	R[0][0] = 2;
	R[0][1] = first_of_bitag(nth_of_multclass(M[0],0)) ;
	int k; /* lxs new fix/addition */

	for ( i=1; i<n; i++ ) {
		R[i] = Rd;
		for (k=0; (k<nbsol && DD[k]>0.0) || (k<1 && DD[0]==0.0) ; k++)
			R[i][k+1] = (S+(n)*(ind[k]))[i] ;
		// skip the repetitions.
		for (int l=1;l<k+1;l++)
			for (int b=l+1;b<k+1;b++)
				if ( R[i][l] == R[i][b] ) 
					R[i][b] = -1;
		R[i][0] = k+1;
		Rd += R[i][0] ;
	}
#ifdef out0
	msg ("n = %d\n", n );
	for ( i=0; i<n; i++ ) {
		print_ambtag(R[i],stdout);
		msg( " " );
	}
	msg( "\n" );
#endif

	return n;
}

/*
 * Main algorithm for analyze.
 * Returns number of solutions
 */

int     linrec_analyze_algorithm(int** M, int word_count, int* solutions, int maxnbsol, double* DD )
{
	int I,maxI=0;
	int	nbsolutions=0;
	int	choices_rn[MaxNbSolutions];

	int	n100=0;
	time_t	td;
	time(&td);

	if (word_count>MaxNbSolutions)
		return 0;	// too large to compute with this algorithm.
	memset(choices_rn,0,sizeof(choices_rn));

	if (word_count <= 1)
		return 0;	// too small to compute with this algorithm.

	I=0; choices_rn[I]=0; choices_rn[I+1]=0;

	while (choices_rn[0] < number_of_multclass(M[0])) {

		while (choices_rn[I+1] < number_of_multclass(M[I+1])) {

			n100++;
			if (n100%50==0) {
				time_t tc;
				time(&tc);
				if ( tc-td > 10 ) {
					// msg( "Canceled because of too much time during analysis\n" );
					return -2; // too much time, use a faster algo.
				}
			}

			if (
				first_of_bitag(nth_of_multclass(M[I+1],choices_rn[I+1]))
				== second_of_bitag(nth_of_multclass(M[I],choices_rn[I]))
				) {
				if ( I == word_count - 2 ) {
					//                              nbsolutions = store_solution(M, word_count, solutions, nbsolutions, choice_rn);
					double dwsol=1.0;
					int i;
					for (i=0;i<word_count;i++) {
						(solutions+nbsolutions*(word_count+1))[i+1] = second_of_bitag(nth_of_multclass(M[i],choices_rn[i])) ;
						dwsol *= (double) nthcount_of_multclass(M[i],choices_rn[i]) /* / (double) weight_of_multclass(M[i]) */ ;
					}
					DD[nbsolutions] = dwsol / (double)word_count;
					// msg ( "%f\n", dwsol );

					for (i=0; i<nbsolutions; i++) {
						for ( int j=0; j<word_count; j++ ) {
							if ( (solutions+nbsolutions*(word_count+1))[j] != (solutions+i*(word_count+1))[j] )
								goto next;
						}
						// this solution already exists
						goto none;
next: ;
					}
					if (nbsolutions>=maxnbsol-1) return 0;
					nbsolutions ++;
none:
					choices_rn[I+1] ++;
				} else {
					I++;
					if (maxI < I) maxI = I;
					choices_rn[I+1] = 0;
					goto suivants;
				}
			} else
				choices_rn[I+1] ++;
		}

		choices_rn[I]++;
		I--;

		if ( I == -1 ) {
			I = 0;
			choices_rn[1] = 0;
		}

suivants: ;
	}

	return nbsolutions;
}

int replacement_analyze_algorithm( AMBTAG* A, int n, AMBTAG* &R, int* annotation, int** M )
{
	int* bag = new int [sizelimitRepAlgo];
	TAG* tbag = new TAG [sizelimitRepAlgo];
	double* fbag = new double [sizelimitRepAlgo];
	int tM[MaxNbTags][25];

#ifdef out3
	{ msg ("AMBTAG n = %d\n", n );
	for ( int i=0; i<n; i++ ) {
		print_ambtag(A[i],stdout);
		msg( " " );
	}
	msg( "\n" ); }
#endif

// STEP3BIS -------------------------------------------------------------------------------------------------

	// find solutions using a simplified analysis schema.
	int i;
	// tag of first word = left tag(s) of M[0]
	for (i=0; i<n-2; i++ ) {
		// compute intersection of right tags of M[i], left tag of M[i+1]
		if (number_of_multclass(M[i])+number_of_multclass(M[i+1])>=sizelimitRepAlgo) {
			annotation [i] |= AnaNote_sizelimit;
			continue;
		}
		int b=0, j, c;
		for ( j=0; j<number_of_multclass(M[i]); j++ )
			if ( second_of_bitag(nth_of_multclass(M[i],j)) != -1 )
				bag[b++] = second_of_bitag(nth_of_multclass(M[i],j));
		// suppress repeted elements in first part of bag.
		isort(bag,b);
		for ( c=0,j=1; j<b; j++ ) {
			if (bag[c] == bag[j]) // identical as previous.
				continue;
			c++;
			if (c!=j) bag[c] = bag[j];
		}
		c++;
		b = c;
		for ( j=0; j<number_of_multclass(M[i+1]); j++ )
			bag[b++] = first_of_bitag(nth_of_multclass(M[i+1],j));
		// suppress repeted elements in second part of bag.
		isort(bag+c,b-c);
		for ( j=c+1; j<b; j++ ) {
			if (bag[c] == bag[j]) // identical as previous.
				continue;
			c++;
			if (c!=j)
				bag[c] = bag[j];
		}
		c++;
		b = c;
		// sort bag.
		isort(bag,b);		
		for ( c=0,j=0; j<b-1; j++ ) {
			if (bag[j] == bag[j+1]) {
				c++;
				j++;
			}
			// if element is present twice, then it belongs to intersection.
		}
#ifdef out3
msg ( "5bag (%d) = ", c );
for ( j=0; j<b ; j++ ) msg( " [%d]=%d,", j, bag[j] );
msg ("\n");
#endif
		if (c==0) {
		// if intersection is empty, then there is no solution within the current trained
		// syntactic rules. This is noted in the annotation tab, and previous tags are left 
		// as it is (no analysis is perform at that point in the utterance)
			annotation [i] |= AnaNote_noresolution;
			annotation [i+1] |= AnaNote_noresolution;
			int k;
			for (k=0; k<number_of_multclass(M[i]); k++ )
				setnthcount_of_multclass( M[i], k, -1 );
			for ( k=0; k<number_of_multclass(M[i+1]); k++ )
				setnthcount_of_multclass( M[i+1], k, -1 );
		} else {
		// suppress all rules than do not belong to intersection.
			int k;
			for ( j=0; j<b; j++ ) {
				if (bag[j] == bag[j+1]) {
					j++; // skip when belongs to intersection.
					continue;
				}
				for (k=0; k<number_of_multclass(M[i]); k++ )
					if (k < 25)
						tM[i][k] = nthcount_of_multclass(M[i],k);
					if ( bag[j] == second_of_bitag(nth_of_multclass(M[i],k)) ) {
						// suppress
						setnthcount_of_multclass( M[i], k, -1 );
					}
				for ( k=0; k<number_of_multclass(M[i+1]); k++ )
					if (k < 25)
						tM[i+1][k] = nthcount_of_multclass(M[i+1],k);
					if ( bag[j] == first_of_bitag(nth_of_multclass(M[i+1],k)) ) {
						// suppress
						setnthcount_of_multclass( M[i+1], k, -1 );
					}
			}
		}

	}
	// tag of last word = right tag(s) of M[n-2]
#ifdef out3
	msg( "AFTER XXXXxxxxxxxxxxxxxxxxxxxxxxxXXXXXXXXXXXXX\n");
	for ( i=0; i<n-1; i++ ) {
		msg ("#%d : %d ", i, M[i][1] );
		print_bimultclass( M[i], stdout );
		char an[256];
		strcpy( an, "" );
		if (annotation[i]&AnaNote_unknownword ) strcat(an, "-unknown-word");
		if (annotation[i]&AnaNote_norule ) strcat(an, "-no-rule");
		if (annotation[i]&AnaNote_noresolution ) strcat(an, "-no-resolution");
		msg( "%s\n", an );
	}
#endif

	// makes ambiguous result and add probabilities.
	// compute PROBABILITIES.... bitags go from 0 to n-2 (n-1 bitags)
	// nthcount_of_multclass(M[i],j) / M[i][1]
	// unless there is no resolutions ...

	char* analyze_buf_r = dynalloc(SizeRespAnalyze);	// size to be checked...
	R = (AMBTAG*) analyze_buf_r;			// --
	TAG* Rd = (TAG*)(R+n);			// --
	int k,j;

	// skip first word in the sentence because it is a 'pct' for the analysis only.
	R[0] = Rd;
	Rd += 2;
	R[0][0] = 1;
	R[0][1] = first_of_bitag(nth_of_multclass(M[0],0)) ;

/*	for ( k=0,j=0; j<number_of_multclass(M[0]); j++ )
		if ( nthcount_of_multclass(M[0],j) != -1 ) {
			fbag[k] = (double)nthcount_of_multclass(M[0],j) / (double)M[0][1] ;
			tbag[k++] = first_of_bitag(nth_of_multclass(M[0],j) ;
		}
	// suppress repeted elements
	fksort( fbag, tbag, k ); // sort on tags and weigths.
	int w;
	for ( w=0,j=1; j<k; j++ ) {
		if (tbag[w] == tbag[j]) // identical as previous.
			continue;
		w++;
		if (w!=j) {
			tbag[w] = tbag[j];
			fbag[w] = fbag[j];
		}
	}
	k = w+1;
	fisort( fbag, tbag, k );
	R[0] = Rd;
	Rd += k + 1;
	R[0][0] = k;
	for (j=0; j<k; j++) {
		R[0][j+1] = tbag[j];
	}
*/

NotResolved:
	int nores = 0;
	for ( i=0; i<n-2; i++ ) {
		if ( annotation [i+1] & AnaNote_noresolution ) {
/* this was in the NewVersion but should have been included from the old version.
			nores = 1;
			R[i+1] = Rd;
			R[i+1][0] = 0;
			R[i+1][1] = 0;
			Rd += 2;
*/
			// put all possible tags
#ifdef out0
	msg ("bcl = %d\n", i );
	print_ambtag(A[i],stdout);
	msg( " " );
	print_ambtag(A[i+1],stdout);
	msg( "\n" );
#endif
//NotResolvedFix1:
			R[i+1] = Rd;
			R[i+1][0] = number_of_ambtag(A[i+1]);
			for (int k=1; k<=R[i+1][0]; k++ )
				R[i+1][k] = A[i+1][k];
			Rd += R[i+1][0]+1;
			continue;
		}
		for ( k=0,j=0; j<number_of_multclass(M[i]); j++ )
			if ( nthcount_of_multclass(M[i],j) != -1 ) {
				fbag[k] = (double)nthcount_of_multclass(M[i],j) / (double)M[i][1] ;
				tbag[k++] = second_of_bitag(nth_of_multclass(M[i],j)) ;
			}
		for ( j=0; j<number_of_multclass(M[i+1]); j++ )
			if ( nthcount_of_multclass(M[i+1],j) != -1 ) {
				fbag[k] = (double)nthcount_of_multclass(M[i+1],j) / (double)M[i+1][1] ;
				tbag[k++] = first_of_bitag(nth_of_multclass(M[i+1],j)) ;
			}
		fksort( fbag, (int*)tbag, k ); // sort on tags and weigths.
		// suppress repeted elements
		int w;
		for ( w=0,j=1; j<k; j++ ) {
			if (tbag[w] == tbag[j]) // identical as previous.
				continue;
			w++;
			if (w!=j) {
				tbag[w] = tbag[j];
				fbag[w] = fbag[j];
			}
		}
		k = w+1;
		fisort( fbag, (int*)tbag, k );
		R[i+1] = Rd;
		Rd += k + 1;
		R[i+1][0] = k;
		for (j=0; j<k; j++) {
			R[i+1][j+1] = tbag[j];
		}
// 2016-10-15 lxs that
		for (j=0; j<k; j++) {
			if (R[i+1][j+1] != 0)
				break;
		}
		if (j == k) {
//		if (R[i+1][1] == 0) {
			for ( j=0; j<number_of_multclass(M[i+1]) && j < 25; j++ )
				setnthcount_of_multclass( M[i+1], j, tM[i+1][j] );
			goto NotResolved;
//			goto NotResolvedFix1;
		}
// 2016-10-15 lxs that
	}
#ifdef out3
	for (i=0; i<n-1; i++) {
		msg("avantavant= ");
		print_ambtag( R[i], stdout );
		msg( "\n" );
	}
#endif
	// last element.
	for ( k=0,j=0; j<number_of_multclass(M[n-2]); j++ )
		if ( nthcount_of_multclass(M[n-2],j) != -1 ) {
			fbag[k] = (double)nthcount_of_multclass(M[n-2],j) / (double)M[n-2][1] ;
			tbag[k++] = second_of_bitag(nth_of_multclass(M[n-2],j)) ;
		}
	if ( k == 0 ) {
/*
		R[n-1] = Rd;
		R[n-1][0] = 0;
		R[n-1][1] = 0;
		Rd += 2;
*/

		// put all possible tags
		R[n-1] = Rd;
		R[n-1][0] = number_of_ambtag(A[n-1]);
		for (int k=1; k<=R[n-1][0]; k++ )
			R[n-1][k] = A[n-1][k];
		Rd += R[n-1][0]+1;

		delete [] bag;
		delete [] tbag;
		delete [] fbag;
#ifdef out3
		msg("avantavantLast [0]\n");
#endif
		return n;
	}
	// suppress repeted elements
	fksort( fbag, (int*)tbag, k ); // sort on tags and weigths.
	int w;
	for ( w=0,j=1; j<k; j++ ) {
		if (tbag[w] == tbag[j]) // identical as previous.
			continue;
		w++;
		if (w!=j) {
			tbag[w] = tbag[j];
			fbag[w] = fbag[j];
		}
	}
	k = w+1;
	fisort( fbag, (int*)tbag, k );
	R[n-1] = Rd;
	Rd += k + 1;
	R[n-1][0] = k;
	for (j=0; j<k; j++) {
		R[n-1][j+1] = tbag[j];
	}

	if ( _matrix_hashdic_ == -1 || nores == 1 ) {
		delete [] bag;
		delete [] tbag;
		delete [] fbag;
		return n;
	}

	/* Other complementary algorithms. - At this stage, old probabilities
	   don't count any more.
	*/

	/*	3D Matrix - there is 'n' element in R.
	 */
#ifdef out3
	for (i=0; i<n; i++) {
		msg("avant= ");
		print_ambtag( R[i], stdout );
		msg( "\n" );
	}
#endif
	for ( i=1 ; i<n ; i++ ) {
		// find for left and right triples when
		// there is at least two choices.
		register int a, b, c, x, cx;
		for ( cx=0, x = 1 ; x < R[i][0]+1 ; x++ )
			if ( R[i][x] != -1 ) cx++;
		if (cx<2) continue;
		// do them in the reverse way an stop when there is just one.
		if ( i-2 >= 0 ) { // enough to the left.
			for ( x = R[i][0] ; x>0 ; x-- ) { // reverse
				if ( R[i][x] == -1 ) continue; 
				for ( a = 1 ; a < R[i-2][0]+1 ; a++ ) {
					if ( R[i-2][a] == -1 ) continue;
					for ( b = 1 ; b < R[i-1][0]+1 ; b++ ) {
						if ( R[i-1][b] == -1 ) continue;
						// makes matrix element with R[i-2][a], R[i-1][b], R[i][x].
						TAG ME[4];
						ME[0] = 3;
						ME[1] = R[i-2][a];
						ME[2] = R[i-1][b];
						ME[3] = R[i][x];
						// test if ME exists.
						Int4 r = hashGetIN( _matrix_hashdic_, ME, 4*sizeof(TAG) );
						if ( r != -1 )
							goto mat3av;
					}
				}
#ifdef out3
{	
	for (int z=0; z<n; z++) {
		print_ambtag( R[z], stdout );
		msg( "\n" );
	}
}
	msg("AV3 %d %d ",i,x);
//	print_ambtag(ME,stdout);
	msg("\n");
#endif
				// if not, suppress the x tag.
				R[i][x] = -1;
				cx--;
				if (cx<2)
					goto stopmatrixprocessing;
mat3av: ;
			}
		}

		if ( i+2 < n ) { // enough to the right.
			for ( x = R[i][0] ; x>0 ; x-- ) { // reverse
				if ( R[i][x] == -1 ) continue;
				for ( a = 1 ; a < R[i+2][0]+1 ; a++ ) {
					if ( R[i+2][a] == -1 ) continue;
					for ( b = 1 ; b < R[i+1][0]+1 ; b++ ) {
						if ( R[i+1][b] == -1 ) continue;
						// makes matrix element with R[i][x], R[i+1][b], R[i+2][a].
						TAG ME[4];
						ME[0] = 3;
						ME[1] = R[i][x];
						ME[2] = R[i+1][b];
						ME[3] = R[i+2][a];
						// test if ME exists.
						Int4 r = hashGetIN( _matrix_hashdic_, ME, 4*sizeof(TAG) );
						if ( r != -1 )
							goto mat3ap;
					}
				}
#ifdef out3
{	
	for (int z=0; z<n; z++) {
		print_ambtag( R[z], stdout );
		msg( "\n" );
	}
}
	msg("AP3 %d %d ",i,x);
//	print_ambtag(ME,stdout);
	msg("\n");
#endif
				// if not, suppress the x tag.
				R[i][x] = -1;
				cx--;
				if (cx<2)
					goto stopmatrixprocessing;
mat3ap: ;
			}
		}

		if ( i-3 >= 0 ) { // enough to the left.
		    for ( x = R[i][0] ; x>0 ; x-- ) { // reverse
			if ( R[i][x] == -1 ) continue;
			for ( a = 1 ; a < R[i-3][0]+1 ; a++ ) {
				if ( R[i-3][a] == -1 ) continue;
				for ( b = 1 ; b < R[i-2][0]+1 ; b++ ) {
					if ( R[i-2][b] == -1 ) continue;
					for ( c = 1 ; c < R[i-1][0]+1 ; c++ ) {
						if ( R[i-1][c] == -1 ) continue;
						// makes matrix element with R[i-3][a], R[i-2][b], R[i-1][c], R[i][x].
						TAG ME[5];
						ME[0] = 4;
						ME[1] = R[i-3][a];
						ME[2] = R[i-2][b];
						ME[3] = R[i-1][c];
						ME[4] = R[i][x];
						// test if ME exists.
						Int4 r = hashGetIN( _matrix_hashdic_, ME, 5*sizeof(TAG) );
						if ( r != -1 )
							goto mat4av;
					}
				}
			}
#ifdef out3
{	
	for (int z=0; z<n; z++) {
		print_ambtag( R[z], stdout );
		msg( "\n" );
	}
}
	msg("AV4 %d %d ",i,x);
//	print_ambtag(ME,stdout);
	msg("\n");
#endif
			// if not, suppress the x tag.
			R[i][x] = -1;
			cx--;
			if (cx<2)
				goto stopmatrixprocessing;
mat4av: ;
		    }
		}

		if ( i+3 < n ) { // enough to the right.
		    for ( x = R[i][0] ; x>0 ; x-- ) { // reverse
			if ( R[i][x] == -1 )
				continue;
			    for ( a = 1 ; a < R[i+3][0]+1 ; a++ ) {
				if ( R[i+3][a] == -1 )
					continue;
				for ( b = 1 ; b < R[i+2][0]+1 ; b++ ) {
					if ( R[i+2][b] == -1 )
						continue;
					for ( c = 1 ; c < R[i+1][0]+1 ; c++ ) {
						if ( R[i+1][c] == -1 )
							continue;
						// makes matrix element with R[i][x], R[i+1][c], R[i+2][b], R[i+3][a].
						TAG ME[5];
						ME[0] = 4;
						ME[1] = R[i][x];
						ME[2] = R[i+1][c];
						ME[3] = R[i+2][b];
						ME[4] = R[i+3][a];
						// test if ME exists.
						Int4 r = hashGetIN( _matrix_hashdic_, ME, 5*sizeof(TAG) );
						if ( r != -1 )
							goto mat4ap;
					}
				}
			}
#ifdef out3
{	
	for (int z=0; z<n; z++) {
		print_ambtag( R[z], stdout );
		msg( "\n" );
	}
}
	msg("AP4 %d %d ",i,x);
//	print_ambtag(ME,stdout);
	msg("\n");
#endif
			// if not, suppress the x tag.
			R[i][x] = -1;
			cx--;
			if (cx<2)
				goto stopmatrixprocessing;
mat4ap: ;
		    }
		}
	}
   stopmatrixprocessing:
#ifdef out3
	for (i=0; i<n; i++) {
		msg("apres= ");
		print_ambtag( R[i], stdout );
		msg( "\n" );
	}
#endif
	for (i=0; i<n; i++) {
		int w;
		for ( w=0,j=1; j<R[i][0]+1; j++ ) {
			if (R[i][j] == -1)
				continue;
			w++;
			if (w!=j)
				R[i][w] = R[i][j];
		}
		R[i][0] = w;
	}
#ifdef out0
	for (i=0; i<n; i++) {
		msg("apres= ");
		print_ambtag( R[i], stdout );
		msg( "\n" );
	}
#endif
	delete [] bag;
	delete [] tbag;
	delete [] fbag;
	return n;
}

/*===================================================|

void fisort(double* array_of_double, int* array_of_int, int size_of_array)

	This function sorts an array of 'double' and and array of 'int'. 
	It uses THE ARAY OF 'DOUBLE' as indices to the sort process.
	At the end of the process, the two arrays are sorted and
	the larger elements are the first ones.
	It is quite efficient (shell sort) and does not use any memory.
|*/
void	fisort(double* ob, int* co, int ta)
{
  	register int gap,i,j;
	double	sw_ob ;
	int	sw_co ;

	for (gap=ta/2; gap>0; gap/=2)
     		for (i=gap; i<ta; i++)
        		for (j=i-gap; j>=0; j-=gap) {
           			if ( ob[j] >= ob[j+gap] )
                			break;
                		sw_ob = ob[j];
                		ob[j] = ob[j+gap];
                		ob[j+gap] = sw_ob;
                		sw_co = co[j];
                		co[j] = co[j+gap];
                		co[j+gap] = sw_co;
			}
}

/*===================================================|

void fksort(double* array_of_double, int* array_of_int, int size_of_array)

	This function sorts an array of 'double' and and array of 'int'. 
	It uses THE ARRAY OF 'INT' as first indices to the sort process and the array of doubles as second indices.
	At the end of the process, the two arrays are sorted.
	It is quite efficient (shell sort) and does not use any memory.
|*/
void	fksort(double* ob, int* co, int ta)
{
  	register int gap,i,j;
	double	sw_ob ;
	int	sw_co ;

	for (gap=ta/2; gap>0; gap/=2)
     		for (i=gap; i<ta; i++)
        		for (j=i-gap; j>=0; j-=gap) {
           			if ( co[j] < co[j+gap] )
                			break;
           			if ( co[j] == co[j+gap]
           			  && ob[j] >= ob[j+gap] )
                			break;
                		sw_ob = ob[j];
                		ob[j] = ob[j+gap];
                		ob[j+gap] = sw_ob;
                		sw_co = co[j];
                		co[j] = co[j+gap];
                		co[j+gap] = sw_co;
			}
}

/*===================================================|

void isort(int* array_of_int, int size_of_array)

	This function sorts an array of 'int'. It is quite efficient (shell sort)
	and does not use any memory.
|*/
void	isort(int* ob, int ta)	// integer array sort.
{
  	register int gap,i,j;
  	int sw;

	for (gap=ta/2; gap>0; gap/=2)
     		for (i=gap; i<ta; i++)
        		for (j=i-gap; j>=0; j-=gap) {
           			if ( ob[j] <= ob[j+gap] )
                			break;
                		sw = ob[j];
                		ob[j] = ob[j+gap];
                		ob[j+gap] = sw;
			}
}
