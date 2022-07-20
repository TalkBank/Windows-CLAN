/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// Rules.cpp
// 6 novembre 1997
// generation of rules

#include "system.hpp"
#include "database.hpp"

// #define DBG8

//;-------------------------------------------------------------------------------------------;
//; general functions on rules
//;-------------------------------------------------------------------------------------------;

int _rules_hashdic_ = -1 ;

int open_rules_dic( const char* rules_filename, int readonly )
{
	if ( hashprobe( rules_filename ) == 1 ) {
		_rules_hashdic_ = hashopen( rules_filename, 1 );	// type 1 for binary data
	} else {
		if ( readonly ) {
			msg ( "rules dictionary %s should exist ...\n", rules_filename );
			return 0;
		}
		msg ( "creation of rules dictionary %s ...\n", rules_filename );
		_rules_hashdic_ = hashcreate( rules_filename, _rules_hash_entries_ , 1 ); // type 1 for binary data
	//	classname_open();
	}
	return _rules_hashdic_ == -1 ? 0 : 1 ;
}

//;-------------------------------------------------------------------------------------------;
//; inserting a rule from a CHAT file
//;-------------------------------------------------------------------------------------------;

char* insert_rule( mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2 )
{
	AMBTAG at = (AMBTAG)dynalloc(MaxNbTags);
	int* at1 = (int*)dynalloc((AMT1->nA*2+2)*sizeof(int));
	int* at2 = (int*)dynalloc((AMT2->nA*2+2)*sizeof(int));
	int i;
	
#ifdef DBG8
	print_mortag(stdout,MT1);
	msg(" * ");
	print_mortag(stdout,MT2);
	msg(" ==> ");
	print_ambmortag(stdout,AMT1);
	msg(" * ");
	print_ambmortag(stdout,AMT2);
	msg( "\n" );
#endif

	mortag *rmt1=0, *rmt2=0;
	for (i=0; i<AMT1->nA; i++ )
		if ( is_in( &(AMT1->A[i]), MT1 ) ) {
			rmt1 = &(AMT1->A[i]) ;
			break;
		}
	for ( i=0; i<AMT2->nA; i++ )
		if ( is_in( &(AMT2->A[i]), MT2 ) ) {
			rmt2 = &(AMT2->A[i]) ;
			break;
		}
	if ( !rmt1 || !rmt2 ) return 0;

#ifdef DBG8
	msg( "  ***\n" );
#endif

	ambmortag_to_multclass(AMT1,at1);
	ambmortag_to_multclass(AMT2,at2);
	TAG biresult = classnames_to_bitag(mortag_to_string(rmt1),mortag_to_string(rmt2)) ;

	int atsz=multclasses_to_biambtag( at1, at2, at );
	add_rules_dic( at, atsz, biresult );

#ifdef DBG8
	msg( "   =====\n" );
#endif

	// add rules for unknown words if cl2 has a class that can be an unknown word
	char *t2 = mortag_to_string(rmt2) ;
	if (t2 == 0)
		return 0;
	char *p = strchr( t2, '-' );
	if ( p ) *p = '\0';
	for ( i=0; i<number_of_multclass(_words_classnames_default_); i++ ) {
		char str[128];
		tag_to_classname( nth_of_multclass(_words_classnames_default_,i), str );
		if ( !strcmp( t2, str ) ) {
			atsz=multclasses_to_biambtag( _words_classnames_default_, at2, at);
			add_rules_dic( at, atsz, biresult );
			atsz=multclasses_to_biambtag( at1, _words_classnames_default_, at);
			add_rules_dic( at, atsz, biresult );
			break;
		}
	}
	return mortag_to_string(rmt2);
}


//;-------------------------------------------------------------------------------------------;
//; add a tag to a multclass and a dictionary (if not already present) - 
//;-------------------------------------------------------------------------------------------;

int* multclass_addone( int* x, int f )
{
	x[1]++;
	x[f+1]++;
	return x;
}

int* multclass_extend( int* x, int nc )
{
	int i, k;

	for (i=2 ; i<x[0] ; i+= 2 )
		if ( x[i] >= nc ) break;
	for (k=x[0]-1; k>=i; k--) x[k+2] = x[k];
	x[0] += 2;
	x[1] ++;
	x[i] = nc;
	x[i+1] = 1;
	return x;
}

//;-------------------------------------------------------------------------------------------;
//; inserting a matrix(n) from a CHAT file
//;-------------------------------------------------------------------------------------------;

void insert_matrix_2( mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2 )
{
	int i;

	mortag *rmt1=0, *rmt2=0;
	for (i=0; i<AMT1->nA; i++ )
		if ( is_in( &(AMT1->A[i]), MT1 ) ) {
			rmt1 = &(AMT1->A[i]) ;
			break;
		}
	for ( i=0; i<AMT2->nA; i++ )
		if ( is_in( &(AMT2->A[i]), MT2 ) ) {
			rmt2 = &(AMT2->A[i]) ;
			break;
		}
	if ( !rmt1 || !rmt2 ) return;

	TAG t[3];

	// generate an array of tag
	// the size of the array is '2'.
	t[0] = 2 ;
	t[1] = classname_to_tag(mortag_to_string(rmt1)) ;
	t[2] = classname_to_tag(mortag_to_string(rmt2)) ;

	add_matrix_dic( t, 3 );
}

void insert_matrix_3( mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2, mortag* MT3, ambmortag* AMT3 )
{
	int i;

	mortag *rmt1=0, *rmt2=0, *rmt3=0;
	for (i=0; i<AMT1->nA; i++ )
		if ( is_in( &(AMT1->A[i]), MT1 ) ) {
			rmt1 = &(AMT1->A[i]) ;
			break;
		}
	for ( i=0; i<AMT2->nA; i++ )
		if ( is_in( &(AMT2->A[i]), MT2 ) ) {
			rmt2 = &(AMT2->A[i]) ;
			break;
		}
	for ( i=0; i<AMT3->nA; i++ )
		if ( is_in( &(AMT3->A[i]), MT3 ) ) {
			rmt3 = &(AMT3->A[i]) ;
			break;
		}
	if ( !rmt1 || !rmt2 || !rmt3 ) return;

	TAG t[4];

	// generate an array of tag
	// the size of the array is '3'.
	t[0] = 3 ;
	t[1] = classname_to_tag(mortag_to_string(rmt1)) ;
	t[2] = classname_to_tag(mortag_to_string(rmt2)) ;
	t[3] = classname_to_tag(mortag_to_string(rmt3)) ;

	add_matrix_dic( t, 4 );
}

void insert_matrix_4(	mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2,
			mortag* MT3, ambmortag* AMT3, mortag* MT4, ambmortag* AMT4 )
{
	int i;

	mortag *rmt1=0, *rmt2=0, *rmt3=0, *rmt4=0;
	for (i=0; i<AMT1->nA; i++ )
		if ( is_in( &(AMT1->A[i]), MT1 ) ) {
			rmt1 = &(AMT1->A[i]) ;
			break;
		}
	for ( i=0; i<AMT2->nA; i++ )
		if ( is_in( &(AMT2->A[i]), MT2 ) ) {
			rmt2 = &(AMT2->A[i]) ;
			break;
		}
	for ( i=0; i<AMT3->nA; i++ )
		if ( is_in( &(AMT3->A[i]), MT3 ) ) {
			rmt3 = &(AMT3->A[i]) ;
			break;
		}
	for ( i=0; i<AMT4->nA; i++ )
		if ( is_in( &(AMT4->A[i]), MT4 ) ) {
			rmt4 = &(AMT4->A[i]) ;
			break;
		}
	if ( !rmt1 || !rmt2 || !rmt3 || !rmt4 ) return;

	TAG t[5];

	// generate an array of tag
	// the size of the array is '4'.
	t[0] = 4 ;
	t[1] = classname_to_tag(mortag_to_string(rmt1)) ;
	t[2] = classname_to_tag(mortag_to_string(rmt2)) ;
	t[3] = classname_to_tag(mortag_to_string(rmt3)) ;
	t[4] = classname_to_tag(mortag_to_string(rmt4)) ;

	add_matrix_dic( t, 5 );
}

void add_rules_dic(AMBTAG at, int atsz, TAG nc)
{
	int* x = (int*)hashGetII( _rules_hashdic_, at, atsz*sizeof(TAG) );
	if (x) {
//msg ("exists\n");
		// exists - either add the tag, either increment the tag number of occurrences
		int f = multclass_find( x, nc );
		if (f == -1)
			// the class do not exist for the word
			hashPutII( _rules_hashdic_, at, atsz*sizeof(TAG), (Int4*)multclass_extend( x, nc ) );
		else
			// the class already exists for the word
			hashPutII( _rules_hashdic_, at, atsz*sizeof(TAG), (Int4*)multclass_addone( x, f ) );
	} else {
//msg ("do not exists\n");
		// do not exist. Create an element
		int v[4] = { 4, 1, nc, 1 };
		hashPutII( _rules_hashdic_, at, atsz*sizeof(TAG), (Int4*)v );
	}
}

void list_rules(FILE* out)
{
	int j;
	int totalsize = 0;
	int totalsizenoslack = 0;

//	hashstat( _rules_hashdic_, out );
	hashiteratorinit( _rules_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopII( _rules_hashdic_ ))) {
#ifdef _short_v0
		AMBTAG a = (AMBTAG)(AMBTAG)l->s;
		for (j = 0; j < number_of_ambtag(a) ; j++ ) {
			msgfile( out, " %3d", nth_of_ambtag(a,j) );
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, " (%s)", s );
		}
		msgfile( out," * ");
		a += number_of_ambtag(a) + 1;
		for ( j = 0; j < number_of_ambtag(a) ; j++ ) {
			msgfile( out, " %3d", nth_of_ambtag(a,j) );
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, " (%s)", s );
		}

		msgfile( out," ==> ");
		print_bimultclass( l->e, out );
		msgfile( out, "\n" );
#else
		AMBTAG a = (AMBTAG)(AMBTAG)l->s;
		for (j = 0; j < number_of_ambtag(a) ; j++ ) {
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, "%s", s );
			if (j!=number_of_ambtag(a)-1) msgfile( out, "/" );
		}
		msgfile( out," * ");
		a += number_of_ambtag(a) + 1;
		for ( j = 0; j < number_of_ambtag(a) ; j++ ) {
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, "%s", s );
			if (j!=number_of_ambtag(a)-1) msgfile( out, "/" );
		}

		msgfile( out," ==> ");
		print_bimultclass( (int*)l->e, out );
		msgfile( out, "\n" );
		totalsizenoslack += (((((l->e[0] * sizeof(Int4))-1) / 32)+1)*32);
//		totalsizenoslack += number_of_ambtag(a) * sizeof(Int4);
		totalsize += l->e[0] * sizeof(Int4);
//		totalsize += number_of_ambtag(a) * sizeof(Int4);
#endif
	}
	msgfile( out, "\n" );
	hashstat( _rules_hashdic_, out );
//	msgfile( out, "total size = %d\n", totalsize );
//	msgfile( out, "total size (no slack) = %d\n", totalsizenoslack );
}

//;-------------------------------------------------------------------------------------------;
//; utilitary commands for matrix
//;-------------------------------------------------------------------------------------------;

int _matrix_hashdic_ = -1 ;
static Int4 WdOfMat[2] = { 1, -1 };

void init_rules(void) {
	_matrix_hashdic_ = -1 ;
}

int width_of_matrix()
{
	if ( _matrix_hashdic_ == -1 ) return 0;
	int e = hashGetIN( _matrix_hashdic_, (Int4*)WdOfMat, 2*sizeof(Int4) );
	if (e==-1) return 0;
	else return e;
}

void set_width_of_matrix(int w)
{
	if ( _matrix_hashdic_ == -1 ) return;
	hashPutIN( _matrix_hashdic_, (Int4*)WdOfMat, 2*sizeof(Int4), w );
}

int open_matrix_dic( const char* matrix_filename, int readonly )
{
	if ( hashprobe( matrix_filename ) == 1 ) {
		_matrix_hashdic_ = hashopen( matrix_filename, 1 );
	} else {
		if ( readonly ) {
			// msg ( "matrix dictionary %s should exist ...\n", matrix_filename );
			return 0;
		}
		msg ( "creation of a matrix dictionary %s ...\n", matrix_filename );
		_matrix_hashdic_ = hashcreate( matrix_filename, _matrix_hash_entries_, 1 );
	}
	return _matrix_hashdic_ == -1 ? 0 : 1 ;
}

void add_matrix_dic( TAG* m, int s )
{
	int e = hashGetIN( _matrix_hashdic_, m, s*sizeof(TAG) );
	if (e == -1)
		hashPutIN( _matrix_hashdic_, m, s*sizeof(TAG), 1 );
	else
		hashPutIN( _matrix_hashdic_, m, s*sizeof(TAG), e+1 );
}

void list_matrix(FILE* out)
{
	int wdmat = 0;
	hashiteratorinit( _matrix_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopIN( _matrix_hashdic_ ))) {
		int* t = (int*)l->s ;
		if (t[0] == 1) {
			wdmat = l->e[0];
			continue;
		}
		msgfile( out, "%d ", t[0] );
		for ( int j = 0; j < t[0] ; j++ ) {
			char s[128];
			tag_to_classname(t[j+1],s);
			msgfile( out, " %s", s );
		}
		msgfile( out, " => %d\n", l->e[0] );
	}
	msgfile( out, "\n" );
	msgfile( out, "Width of matrix => %d\n", wdmat );
	hashstat( _matrix_hashdic_, out );
}

//;-------------------------------------------------------------------------------------------;
//; utilitary commands for localword
//;-------------------------------------------------------------------------------------------;

int _localword_hashdic_ = -1 ;

int open_localword_dic( const char* filename, int readonly )
{
	if ( hashprobe( filename ) == 1 ) {
		_localword_hashdic_ = hashopen( filename, 0 );
	} else {
		if ( readonly ) {
			// msg ( "localword dictionary %s should exist ...\n", filename );
			return 0;
		}
		msg ( "creation of a local word dictionary %s ...\n", filename );
		_localword_hashdic_ = hashcreate( filename, _localwords_hash_entries_, 0 );
	}
	return _localword_hashdic_ == -1 ? 0 : 1 ;
}

void add_local_word( char* ams, int wt, int nbt )
{
	int* e = (int*)hashGetCI( _localword_hashdic_, ams );
	if (e) {
// msg ("/!!!%s!!!%d-%d_",ams,e[0],wt);for (int iiii=0;iiii<e[0]-1;iiii++) msg("%d,",e[iiii+1]); msg("\n");
		e[wt+1] ++;
		hashPutCI( _localword_hashdic_, ams, (Int4*)e );
	} else {
		int E[MaxNbTags];
		E[0] = nbt+1;
		for (int i=0;i<nbt;i++) E[i+1] = 0;
		E[wt+1] = 1;
// msg ("!!!%s!!!%d-%d_",ams,E[0],wt);for (int iiii=0;iiii<E[0]-1;iiii++) msg("%d,",E[iiii+1]); msg("\n");
		hashPutCI( _localword_hashdic_, ams, (Int4*)E );
	}
}

int get_local_word( char* ams )
{
	int* e = (int*)hashGetCI( _localword_hashdic_, ams );
	if (e) {
		// find tag most used for this word.
		int m = 0, w = 0 ;
		for (int i=0;i<e[0]-1;i++)
			if (e[i+1] > m) {
				m = e[i+1];
				w = i;
			}
		// find the corresponding TAG
//		ambmortag a ;
//		make_ambmortag( ams, &a );
//		return classname_to_tag( mortag_to_string( filter_mortag( &a.A[w] ) ) );

		return w;
		
	} else {
		return -1;
	}
}

void list_localword(FILE* out)
{
	int totalsize = 0;
	int totalsizenoslack = 0;
// 2006-02-20	int wdmat = 0;
	hashiteratorinit( _localword_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopCI( _localword_hashdic_ ))) {
		msgfile( out, "%s", l->s );
		msgfile( out, " => %d [", l->e[0]-1 );
		for (int i=0; i<(l->e[0]-2) ; i++) 
			msgfile( out, "%d,", l->e[i+1] );
		msgfile( out, "%d]\n", l->e[l->e[0]-1] );
		totalsizenoslack += (((((l->e[0] * sizeof(Int4))-1) / 32)+1)*32);
		totalsize += l->e[0] * sizeof(Int4);
	}
	msgfile( out, "\n" );
	hashstat( _localword_hashdic_, out );
//	msgfile( out, "total size = %d\n", totalsize );
//	msgfile( out, "total size (no slack) = %d\n", totalsizenoslack );
}


//;-------------------------------------------------------------------------------------------;
//; utilitary commands for word
//;-------------------------------------------------------------------------------------------;

int _word_hashdic_ = -1 ;

int open_word_dic( const char* filename, int readonly )
{
	if ( hashprobe( filename ) == 1 ) {
		_word_hashdic_ = hashopen( filename, 0 );
	} else {
		if ( readonly ) {
			// msg ( "word dictionary %s should exist ...\n", filename );
			return 0;
		}
		msg ( "creation of a word dictionary %s ...\n", filename );
		_word_hashdic_ = hashcreate( filename, _words_hash_entries_, 0 );
	}
	return _word_hashdic_ == -1 ? 0 : 1 ;
}

static void update_nth_freq( char* oldfreq, char* newfreq, int nth )
{
	char repl[MAXSIZECATEGORY];
	// split oldfreq and update nth part: recreate new element in old freq
	char* s = strtok( oldfreq, "|" );
	int i = 0;
	repl[0] = '\0';
	if (!s) { // no element at all should not happen
		msg("serious error: update_nth_freq: bad data ?");
		return;
	}
	while ( s ) {
		if (i == nth) {
			int nfo = atoi(newfreq);
			int fo = atoi(s);
			sprintf( repl + strlen(repl), "%d", fo+nfo);
		} else
			strcat( repl, s );
		s = strtok( NULL, "|" );
		i++;
		if (s) // not the end
			strcat( repl, "|" );
	}
	strcpy( oldfreq, repl );
}

static void update_all_freq( char* oldfreq, char* newfreq )
{
	char repl[MAXSIZECATEGORY];
	// split oldfreq and update nth part: recreate new element in old freq
	char* s = strtok( oldfreq, "|" );
	int nfo = atoi(newfreq);
	repl[0] = '\0';
	if (!s) { // no element at all should not happen
		msg("serious error: update_nth_freq: bad data ?");
		return;
	}
	while ( s ) {
		int fo = atoi(s);
		sprintf( repl + strlen(repl), "%d", fo+nfo);
		s = strtok( NULL, "|" );
		if (s) // not the end
			strcat( repl, "|" );
	}	
}

static int nb_cat(char* cat)	// tell how many categories there are in an ambiguous categories
{
	char* p = strchr(cat,'^');
	int n = 1;
	while (p) {
		n++;
		p++;
		p = strchr(p,'^');
	}
	return n;
}

char* get_nth_cat(char* cat, int nth, char* d)	// copy the nth category in the words categories
{
	char* md[32];
	char copy[MAXSIZECATEGORY];
	strcpy(copy, cat);
	if (32 <= nth) {
		msg( "Too many categories (%d)\n", nth);
		d[0] = '\0';
		return d;
	}
	split_with(copy, md, "^", 32);
	strcpy( d, md[nth]);
	return d;
}

char* get_nth_elt(char* elt, int nth, char* d)	// copy the nth element in the phonology and frequencies
{
	char* md[32];
	char copy[MAXSIZECATEGORY];
	strcpy(copy, elt);
	if (32 <= nth) {
		msg( "Too many elements (%d)\n", nth);
		d[0] = '\0';
		return d;
	}
	split_with(copy, md, "|", 32);
	strcpy( d, md[nth]);
	return d;
}

// for add_word_dic, some arguments can be optional: then they are given as NULL or empty string
// add_singleword_dic do the job. It looks for a couple w + ncat and add info only to this entry.
// if ncat is NULL and frequencies are not null, frequencies are added to all cat of the word entry
void add_singleword_dic( char* w, char* ncat, char* npho, char* nf1, char* nf2, char* nf3)
{
	char* e = hashGetCC( _word_hashdic_, w );
	char ne[MAXSIZECATEGORY];
	if (e) {
//msg ("a/%s/%s/%s/%s/%s/%s/\n", w, ncat, npho, nf1, nf2, nf3);
		// extract information from old entry
		char cat[MAXSIZECATEGORY];
		char pho[MAXSIZECATEGORY];
		char f1[MAXSIZECATEGORY];
		char f2[MAXSIZECATEGORY];
		char f3[MAXSIZECATEGORY];
		cat_from_word(e, cat);	// gets category
		phono_from_word(e, pho);	// gets phonology
		freq1_from_word(e, f1);	// gets frequency 1 (lemma form frequency)
		freq2_from_word(e, f2);	// gets frequency 2 (basic form frequency)
		freq3_from_word(e, f3);	// gets frequency 3 (local corpus frequency)
//msg ("b/%s/%s/%s/%s/%s/%s/\n", w, cat, pho, f1, f2, f3);
		if (!ncat || ncat[0]=='\0') {
			// add only frequencies information
			if (nf1 && nf1[0]!='\0')
				update_all_freq(f1,nf1);
			if (nf2 && nf2[0]!='\0')
				update_all_freq(f2,nf2);
			if (nf3 && nf3[0]!='\0')
				update_all_freq(f3,nf3);
			create_parts_of_word( ne, cat, pho, f1, f2, f3 );
//msg ("c(frq only)/%s/%s/%s/%s/%s/%s/\n", w, cat, pho, f1, f2, f3);
			hashPutCC( _word_hashdic_, w, ne );
			return;
		}

		// look for the new category in the old entry
		int i, n = nb_cat(cat);
		for ( i=0; i<n; i++ ) {
			char ict[MAXSIZECATEGORY];
			get_nth_cat( cat, i, ict );
			if ( !strcmp( ict, ncat ) ) { // found! Now update & to be changed into inclusion for prefixes suffixes purposes
				// add frequencies only to the nth element
				if (nf1 && nf1[0]!='\0')
					update_nth_freq(f1,nf1,i);
				if (nf2 && nf2[0]!='\0')
					update_nth_freq(f2,nf2,i);
				if (nf3 && nf3[0]!='\0')
					update_nth_freq(f3,nf3,i);
				create_parts_of_word( ne, cat, pho, f1, f2, f3 );
//msg ("c(update frq)/%d/%s/%s/%s/%s/%s/%s/%s/%s/%s/\n", i, w, cat, pho, f1, f2, f3, nf1, nf2, nf3 );
				hashPutCC( _word_hashdic_, w, ne );
				return;
			}
		}

		// this is a new category for this entry! Add at the end		
		if ( strlen(ne) + strlen(e) + 6 > MAXSIZECATEGORY ) {
			msg( "Max category size reached for /%s/%s/\n", ne, e );
			return;
		}
		strcat(cat, "^"); strcat(cat, ncat);
		strcat(pho, "|"); strcat(pho, npho);
		strcat(f1, "|"); strcat(f1, nf1);
		strcat(f2, "|"); strcat(f2, nf2);
		strcat(f3, "|"); strcat(f3, nf3);
		create_parts_of_word( ne, cat, pho, f1, f2, f3 );
//msg ("c(new entry)/%s/%s!%s!%s!%s!%s/%s!%s!%s!%s!%s/\n",w,cat,pho,f1,f2,f3,ncat,npho,nf1,nf2,nf3);
		hashPutCC( _word_hashdic_, w, ne );
	} else {
		// this is creation of a full new entry
		char xcat[MAXSIZECATEGORY];
		char xpho[MAXSIZECATEGORY];
		char xf1[MAXSIZECATEGORY];
		char xf2[MAXSIZECATEGORY];
		char xf3[MAXSIZECATEGORY];
		strcpy( xcat, ncat?ncat:"" );
		strcpy( xpho, npho?npho:"" );
		strcpy( xf1, nf1?nf1:"" );
		strcpy( xf2, nf2?nf2:"" );
		strcpy( xf3, nf3?nf3:"" );
		create_parts_of_word( ne, xcat, xpho, xf1, xf2, xf3 );
		hashPutCC( _word_hashdic_, w, ne );
	}
}

void add_word_dic( char* w, char* ncat, char* npho, char* nf1, char* nf2, char* nf3)
{
	// slit ncat between ambiguous forms
	char* s = strtok( ncat, "^" );
	while ( s ) {
		add_singleword_dic( w, s, npho, nf1, nf2, nf3);
		s = strtok( NULL, "^" );
	}	
}

char* get_word_dic( char* w, char* c )
{
	char* e = hashGetCC( _word_hashdic_, w );
	if (e) {
		strcpy( c, e );
		return c;
	} else {
		return (char*)0;
	}
}

static char* insert_pho(char* cat, char* pho, char* d, const char* si, const char* st)
{
	char* p = strchr(cat, '|');
	if (!p) {
		strcpy( d, cat );
		strcat( d, si );
		strcat( d, pho );
		strcat( d, st );
		return d;
	}
	char* q1 = strchr(p, MorTagComplementsIrregular );
	char* q2 = strchr(p, MorTagComplementsRegular );
	if (!q1 && !q2) {
		strcpy( d, cat );
		strcat( d, si );
		strcat( d, pho );
		strcat( d, st );
		return d;
	}
	if (!q1 && q2)
		q1 = q2;
	if (q1 && q2 && q2<q1)
		q1 = q2;
	strcpy(d,cat);
	d[q1-cat] = '\0';
	strcat( d, si );
	strcat( d, pho );
	strcat( d, st );
	strcat( d, cat+(q1-cat) );
	return d;
}

char* catpho_from_word(char* word, char* to)	// gets category and phonology
{
	char allct[MAXSIZECATEGORY];
	char allph[MAXSIZECATEGORY];

	cat_from_word(word, allct);
	phono_from_word(word, allph);
	to[0] = '\0';

	int i, n = nb_cat(allct);
// msg( "%d/%s/%s/\n", n, allct, allph );
	for ( i=0; i<n; i++ ) {
		if (i>0) strcat(to,"^");
		char ct[MAXSIZECATEGORY];
		char ph[MAXSIZECATEGORY];
// msg( "!!!%s!!!%s!!!\n", allct, allph );
		get_nth_cat( allct, i, ct );
		get_nth_elt( allph, i, ph );
//char* rr = to+strlen(to);
		insert_pho(ct, ph, to+strlen(to), "(", ")" );
//msg( "[%s][%s]/%s/\n", ct, ph, rr );
	}
	return to;
}

char* cat_from_word_form(char* word, char* form, char* to)	// gets category + original form
{
	char allct[MAXSIZECATEGORY];

	cat_from_word(word, allct);
	//use form instead of allph in phono_from_word(word, allph);
	to[0] = '\0';

	int i, n = nb_cat(allct);
	for ( i=0; i<n; i++ ) {
		if (i>0) strcat(to,"^");
		char ct[MAXSIZECATEGORY];
		get_nth_cat( allct, i, ct );
		//get_nth_elt( allph, i, ph );
		insert_pho(ct, form, to+strlen(to), "{", "}" );
	}
	return to;
}

char* catpho_from_word_form(char* word, char* form, char* to)	// gets category + original form + phono
{
	char allct[MAXSIZECATEGORY];
	char allph[MAXSIZECATEGORY];
	char fp[MAXSIZECATEGORY];

	cat_from_word(word, allct);
	//use form in addition to allph
	phono_from_word(word, allph);
	to[0] = '\0';

	int i, n = nb_cat(allct);
	for ( i=0; i<n; i++ ) {
		if (i>0) strcat(to,"^");
		char ct[MAXSIZECATEGORY];
		char ph[MAXSIZECATEGORY];
		get_nth_cat( allct, i, ct );
		get_nth_elt( allph, i, ph );
		strcpy(fp, form );
		strcat(fp, "}(");
		strcat(fp, ph);
		insert_pho(ct, fp, to+strlen(to), "{", ")" );
	}
	return to;
}

void list_word(FILE* out)
{
	int totalsize = 0;
	hashiteratorinit( _word_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopCC( _word_hashdic_ ))) {
		msgfile( out, "%s", l->s );
//		msgfile( out, " => %s\n", (char*)l->e );
		char info[MAXSIZECATEGORY];
		cat_from_word((char*)l->e, info);
		msgfile( out, " => %s", info );
		phono_from_word((char*)l->e, info);
		msgfile( out, " [%s]", info );
		freq1_from_word((char*)l->e, info);
		msgfile( out, " %s", info );
		freq2_from_word((char*)l->e, info);
		msgfile( out, " %s", info );
		freq3_from_word((char*)l->e, info);
		msgfile( out, " %s\n", info );
		totalsize ++;
	}
	msgfile( out, "\n" );
	hashstat( _word_hashdic_, out );
	msgfile( out, "total size = %d\n", totalsize );
}

char* create_parts_of_word(char* d, char* s1, char* s2, char* n3, char* n4, char* n5 )
{
	sprintf( d, "%s\x1%s\x2%s\x3%s\x4%s", s1, s2, n3, n4, n5 );
	return d;
}

char* cat_from_word(char* word, char* d)	// gets category
{
	char* p = strchr(word, '\x1');
	if (!p) { strcpy(d,word); return d; }
	strncpy(d, word, p-word);
	d[p-word] = '\0';
	return d;
}

char* phono_from_word(char* word, char* d)	// gets phonology
{
	char* p1 = strchr(word, '\x1');
	char* p2 = strchr(word, '\x2');
	if (!p1 || !p2 || p2 < p1) { strcpy(d,word); return d; }
	strncpy(d, p1+1, p2-p1-1);
	d[p2-p1-1] = '\0';
	return d;
}

char* freq1_from_word(char* word, char* d)	// gets frequency 1 (lemma form frequency)
{
	char* p1 = strchr(word, '\x2');
	char* p2 = strchr(word, '\x3');
	if (!p1 || !p2 || p2 < p1) { strcpy(d,word); return d; }
	strncpy(d, p1+1, p2-p1-1);
	d[p2-p1-1] = '\0';
	return d;
}

char* freq2_from_word(char* word, char* d)	// gets frequency 2 (basic form frequency)
{
	char* p1 = strchr(word, '\x3');
	char* p2 = strchr(word, '\x4');
	if (!p1 || !p2 || p2 < p1) { strcpy(d,word); return d; }
	strncpy(d, p1+1, p2-p1-1);
	d[p2-p1-1] = '\0';
	return d;
}

char* freq3_from_word(char* word, char* d)	// gets frequency 3 (local corpus frequency)
{
	char* p = strchr(word, '\x4');
	if (!p) { strcpy(d,word); return d; }
	strcpy(d, p+1);
	return d;
}

