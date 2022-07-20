/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- tags.cpp	- v1.0.0 - november 1998

	Basic handling of TAGS data:
	1) conversion between memory data and file data.
	2) manipulation of the different TAG types and conversion from one type to another.

	General tag handling
	1 - add and retrieve a tag in database.
	2 - make ambiguous tags.
	3 - intersection of tags.
	4 - reunion of tags.

	Relation with MOR
	5 - transform MOR tags into part-of-speech tags
	6 - filter MOR tags with part-of-speech tags

	The different kind so TAG type are (see tags.hpp for a definition of their implementation)

	CLASSNAME = a tag as an ascii string
	TAG = a tag as an integer
	AMBTAG = TAG* = an array of TAG representing an ambiguous classes
	multclass = int* = an array of TAG, but each tag is associated with an occurence figure

|=========================================================================================================================*/

#include "system.hpp"

#include "database.hpp"

static int _class_hashdic_ = -1;
static int _class_hashdic_inverse_ = -1;
static int _nb_class_hashdic_ = 0;

void init_tags(void) {
	_class_hashdic_ = -1;
	_class_hashdic_inverse_ = -1;
	_nb_class_hashdic_ = 0;
}

//;-------------------------------------------------------------------------------------------;
//; Functions that allow to store and retrieve general information in the database
//;-------------------------------------------------------------------------------------------;

void put_info_into_database(char* tag, char* info)
{
	hashPutCC ( _class_hashdic_inverse_, tag, info );
}

int get_info_from_database(char* tag, char* info)
{
	char* p = hashGetCC ( _class_hashdic_inverse_, tag );
	if (!p)
		return 0;
	else {
		strcpy(info, p);
		return 1;
	}
}

//;-------------------------------------------------------------------------------------------;
//; Definition of functions that handle tags and classname
//;-------------------------------------------------------------------------------------------;

void classname_save()
{
	// nothing yet.
}

char _words_classname_pct_ [] = "pct" ;
/** this must be changed and made language dependent **/
const char *_words_classnames_def_noms_ [] = {
	"unk", "adj", "adv", "co", "n:prop", "v:pp", "n", "v", "v:inf", "v:prog", 0
};

int _words_classnames_default_ [2*sizeof(_words_classnames_def_noms_)+2*sizeof(int)] ;

static void _wdef_init() {
	const char **p;

	classname_to_tag( _words_classname_pct_ );	// always set the same tag for 'pct' in any database (useful for Brill's rules).
	_words_classnames_default_ [0] = 2;
	_words_classnames_default_ [1] = 0;
	for ( p = &_words_classnames_def_noms_[0]; *p ; p++ ) {
		int i = _words_classnames_default_ [0];
		_words_classnames_default_[i] = classname_to_tag(*p);
		_words_classnames_default_[i+1] = 1;
		_words_classnames_default_ [0] += 2;
		_words_classnames_default_ [1] ++;
	}
}

int classname_init()
{
	_nb_class_hashdic_ = 0; // lxs1234
	_class_hashdic_ = hashcreate( "classname.equivalences", HashInitMaxNbTags );
	_class_hashdic_inverse_ = hashcreate( "classname.equivalences.inverse", HashInitMaxNbTags );

	_wdef_init() ;
	return _class_hashdic_ ;
}

int classname_open()
{
	_nb_class_hashdic_ = 0; // lxs1234
	if ( (_class_hashdic_ = hashopen( "classname.equivalences" )) == -1 ) {
		msg( "cannot open classname.equivalences\n" );
		return -1;
	}
	if ( (_class_hashdic_inverse_ = hashopen( "classname.equivalences.inverse" )) == -1 ) {
		msg( "cannot open classname.equivalences.inverse\n" );
		return -1;
	}
	_nb_class_hashdic_ = hashGetCN ( _class_hashdic_, "*classname-list-nb*" );
	_wdef_init() ;
	return _class_hashdic_ ;
}

long32 tags_number()
{
	return hashsize( _class_hashdic_ );
}

void list_classnames(FILE* out)
{
	hashstat( _class_hashdic_, out );
	hashiteratorinit( _class_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopCN( _class_hashdic_ ))) {
		msgfile( out, "%s %d\n", l->s, l->e[0] );
	}
	msgfile( out, "number of classes: %d\n", _nb_class_hashdic_ );
}

/*-------------------------------------------------------------------
	functions that handle TAG (and classnames)
  -------------------------------------------------------------------*/

TAG classname_to_tag(const char* c)
{
	int nc = hashGetCN( _class_hashdic_, c );
	if (nc == -1) {
		/* TEST if MAXIMUM NUMBER of TAGs is reached
		*/
		if (_nb_class_hashdic_ >= AbsoluteMaxNbTags ) {// lxs1234
			local_abort ( 0, "maximum number of tags reached (AbsoluteMaxNbTags)" );	// general ABORT function
											// no way back
		}

		_nb_class_hashdic_ ++;
		hashPutCN ( _class_hashdic_, c, _nb_class_hashdic_ );
		char xx[64];
		sprintf( xx, "%d", _nb_class_hashdic_ );
		hashPutCC ( _class_hashdic_inverse_, xx, c );
		hashPutCN ( _class_hashdic_, "*classname-list-nb*", _nb_class_hashdic_ );
		return _nb_class_hashdic_ ;
	} else
		return nc;
}

int tag_to_classname(TAG t, char* r)
{
	char xx[64];
	sprintf( xx, "%ld", t );// lxs1234
	char *s = hashGetCC( _class_hashdic_inverse_, xx );
	if (s) {
		strcpy( r, s );
		return 1;
	} else {
		strcpy( r, "none" );
		return 0;
	}
}

/*-------------------------------------------------------------------
	functions that handle multclass (for tag or bitag)
  -------------------------------------------------------------------*/

int multclass_find( int* x, int nc )
{
	for (int i=2 ; i<x[0] ; i+= 2 )
		if ( x[i] == nc ) return i;
	return -1;
}


int multclass_to_ambtag(int *cn, AMBTAG d)
{
	d[0] = (cn[0] - 2) / 2;
	int k = 1;
	for (int i=2; i<cn[0]; i+=2) d[k++] = cn[i];
	return d[0]+1; // returns the equivalent of size_of_ambtag(d)
}

int size_of_multclass(int *cn)
{
	return cn[0];
}

int weight_of_multclass(int *cn)		// returns the total number of counts of a multclass
{
	return cn[1];
}

int number_of_multclass(int *cn)
{
	return (cn[0] - 2) / 2;
}

int nth_of_multclass( int* c, int i )
{
	return c[i*2+2];	// returns the nth tag in a multclass
}

int nthcount_of_multclass( int* c, int i )
{
	return c[i*2+2+1];	// returns the nth tag in a multclass
}

void setnthcount_of_multclass( int* c, int i, int v )
{
	c[i*2+2+1] = v;	// set the nth tag in a multclass
}

/*-------------------------------------------------------------------
	functions that handle AMBTAG
  -------------------------------------------------------------------*/

int number_of_ambtag( AMBTAG c )
{
	return c[0];	// returns the number of tags.
}

TAG nth_of_ambtag( AMBTAG c, int i )
{
	return c[i+1];	// returns the nth tag in an ambtag
}

int multclasses_to_biambtag(int* w1, int* w2, AMBTAG D)
{
	int nxt = multclass_to_ambtag(w1,D);
	return multclass_to_ambtag(w2,D+nxt) + nxt; // returns size of biambtag
	
}

void print_ambtag(AMBTAG a, FILE* out)
{
	msgfile( out, "[%d", number_of_ambtag(a) );
	for ( int j = 0; j < number_of_ambtag(a) ; j++ ) {
		msgfile( out, ",%d", nth_of_ambtag(a,j) );
		if ( nth_of_ambtag(a,j) == -1 )
			msgfile( out, " xm " );
		else {
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, "-%s", s );
		}
	}
	msgfile( out, "]" );
}

void print_ambtag_regular(AMBTAG a, FILE* out, int d)
{
	for ( int j = 0; j < number_of_ambtag(a) + (d); j++ ) {
		if ( nth_of_ambtag(a,j) != -1 ) {
			char s[128];
			tag_to_classname(nth_of_ambtag(a,j),s);
			msgfile( out, "%s,", s );
		}
	}
}

/*-------------------------------------------------------------------
	functions that handle bitag
  -------------------------------------------------------------------*/
TAG classnames_to_bitag(CLASSNAME* cl1, CLASSNAME* cl2)	// makes a bitag from to string classname.
{
	return tags_to_bitag(classname_to_tag(cl1), classname_to_tag(cl2));
}

TAG tags_to_bitag(TAG t1, TAG t2) // makes a bitag from two tags
{
	return t1 * AbsoluteMaxNbTags + t2 ;
}

TAG first_of_bitag(TAG bi) // return first element in a bitag
{
	return  bi / AbsoluteMaxNbTags;
}

TAG second_of_bitag(TAG bi) // return second element in a bitag
{
	return  bi % AbsoluteMaxNbTags;
}

void print_multclass(int *bm, FILE* out )
{
	msgfile( out, " #%d# ", number_of_multclass(bm) );
	for ( int j=0; j<number_of_multclass(bm); j++ ) {
		msgfile( out, " %d (%d) ", nth_of_multclass(bm,j), nthcount_of_multclass(bm,j) );
		char s[128];
		tag_to_classname(nth_of_multclass(bm,j),s);
		msgfile( out, "[%d-%s] /", nth_of_multclass(bm,j), s );
	}
}

void print_bimultclass(int *bm, FILE* out )
{
	msgfile( out, " %d /", number_of_multclass(bm) );
	for ( int j=0; j<number_of_multclass(bm); j++ ) {
		if (nth_of_multclass(bm,j)>_nb_class_hashdic_)
			msgfile( out, " [nth:~, nth_cnt:%d] ", nthcount_of_multclass(bm,j) );
		else
			msgfile( out, " [nth:%d, nth_cnt:%d] ", nth_of_multclass(bm,j), nthcount_of_multclass(bm,j) );
//		msgfile( out, " (%d) ", nthcount_of_multclass(bm,j) );
		char s1[128];
		tag_to_classname(first_of_bitag(nth_of_multclass(bm,j)),s1);
		char s2[128];
		tag_to_classname(second_of_bitag(nth_of_multclass(bm,j)),s2);
		msgfile( out, "[1st:%d-%s, 2nd:%d-%s] /", first_of_bitag(nth_of_multclass(bm,j)), s1, second_of_bitag(nth_of_multclass(bm,j)), s2 );
//		msgfile( out, "%s,%s /", s1, s2 );
	}
}

// ******

CLASSNAME* addstar_to_classname(CLASSNAME* n, CLASSNAME* s)
	// add the a symbol (*) the name of a tag.
{
	strcpy((char*)s,(char*)n);
	strcat((char*)s,"*");
	return s;
}

void print_matrixelement( TAG *me, FILE* out )
{
	int* t = (int*)me ;
	if (t[0] < 2 || t[0] > 5) {
		msgfile( out, "bizarre matrix element size => %d\n", t[0] );
		return;
	}
	for ( int j = 0; j < t[0] ; j++ ) {
		char s[128];
		tag_to_classname(t[j+1],s);
		msgfile( out, "%s ", s );
	}
	msgfile( out, "\n" );
}
