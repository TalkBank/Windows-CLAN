/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// BrillAnx.cpp
//;-------------------------------------------------------------------------------------------;
//; utilitary commands for brill's rules
//;-------------------------------------------------------------------------------------------;

#include "system.hpp"
#include "database.hpp"

int _brillword_hashdic_ = -1 ;

int open_brillword_dic( const char* filename, int readonly )
{
	if ( hashprobe( filename ) == 1 ) {
		_brillword_hashdic_ = hashopen( filename, 0 );
	} else {
		if ( readonly ) {
			// msg ( "brillword dictionary %s should exist ...\n", filename );
			return 0;
		}
		msg ( "creation of a word dictionary for brill's rules%s ...\n", filename );
		_brillword_hashdic_ = hashcreate( filename, _brillwords_hash_entries_, 0 );
	}
	return _brillword_hashdic_ == -1 ? 0 : 1 ;
}

void add_brillword( char* ams, TAG t )
{
	int* e = (int*)hashGetCI( _brillword_hashdic_, ams );
	if (e) {
		// find the tag
		for (int i=0;i<e[0]-1;i++)
			if (e[i+1] == t)
				return;
		int* s = (int*)dynalloc( sizeof(int)*(e[0]+1) );
		memcpy( s+2, e+1, sizeof(int)*(e[0]-1) );
		s[0] = e[0]+1;
		s[1] = t;
		hashPutCI( _brillword_hashdic_, ams, (Int4*)s );
	} else {
		int E[2];
		E[0] = 2;
		E[1] = t;
		hashPutCI( _brillword_hashdic_, ams, (Int4*)E );
	}
}

int is_in_WORDS( char* ams )
{
	int* e = (int*)hashGetCI( _brillword_hashdic_, ams );
	if (e)
		return 1;
	else
		return 0;
}

int is_in_SEENTAGGING( char* ams )
{
	char w[64];
	char c[64];
	sscanf(ams, "%s%s", w, c );
	TAG t = classname_to_tag(c);
	int* e = (int*)hashGetCI( _brillword_hashdic_, w );
	if (e) {
		// find the tag
		for (int i=0;i<e[0]-1;i++)
			if (e[i+1] == t)
				return 1;
	}
	return 0;
}

void list_brillword(FILE* out)
{
	int totalsize = 0;
	int totalsizenoslack = 0;
	hashiteratorinit( _brillword_hashdic_ );
	loopr* l;
	while ((l=hashiteratorloopCI( _brillword_hashdic_ ))) {
		msgfile( out, "%s", l->s );
		msgfile( out, " => %d ", l->e[0]-1 );
		for (int i=0; i<(l->e[0]-1) ; i++) {
			char s[64];
			tag_to_classname(l->e[i+1],s);
			msgfile( out, " %s", s );
		}
		msgfile( out, "\n" );
		totalsizenoslack += (((((l->e[0] * sizeof(Int4))-1) / 32)+1)*32);
		totalsize += l->e[0] * sizeof(Int4);
	}
	msgfile( out, "\n" );
	hashstat( _brillword_hashdic_, out );
//	msgfile( out, "total size = %d\n", totalsize );
//	msgfile( out, "total size (no slack) = %d\n", totalsizenoslack );
}

int _brillrule_hashdic_ = -1 ;

int open_brillrule_dic( const char* filename, int readonly )
{
	if ( hashprobe( filename ) == 1 ) {
		_brillrule_hashdic_ = hashopen( filename, 0 );
	} else {
		if ( readonly ) {
			// msg ( "brillrule dictionary %s should exist ...\n", filename );
			return 0;
		}
		msg ( "creation of a rule dictionary for brill's rules%s ...\n", filename );
		_brillrule_hashdic_ = hashcreate( filename, _brillrules_hash_entries_, 0 );
	}
	return _brillrule_hashdic_ == -1 ? 0 : 1 ;
}

#define BRILLNBRULES "Number of rules"

int nb_brillrules()
{
	long* rec = hashGetCI( _brillrule_hashdic_, BRILLNBRULES );
	if (!rec)
		return 0;
	else
		return rec[1];
}

int onemore_brillrule()
{
	int n = nb_brillrules()+1;
	long rec[2];
	rec[0] = 2;
	rec[1] = n;
	hashPutCI( _brillrule_hashdic_, BRILLNBRULES, rec );
	return n;
}

int add_brillrule( char* word, char* tag, char* new_rule, int at_number )
{
//msg( "add %s %s [%s] %d\n", word, tag, new_rule, at_number );
	char rulename[128];
	sprintf( rulename, "(%s) (%s)", word, tag );

	long* old_rec = hashGetCI( _brillrule_hashdic_, rulename );
	if (!old_rec) {	// first rule to add.
		char num[10];
		if (at_number == 0)
			at_number = onemore_brillrule();
		sprintf( num, " %d", at_number );
		strcat( new_rule, num );

		int p = alloc_and_write_string( new_rule );
		long rec[2];
		rec[0] = 2;
		rec[1] = p;
		hashPutCI( _brillrule_hashdic_, rulename, rec );

		// add reference to number.
		sprintf( num, "%d", at_number );
		rec[0] = 2;
		rec[1] = p;
		hashPutCI( _brillrule_hashdic_, num, rec );
		
		return 1;
	}
	int lnr = strlen(new_rule);
	new_rule[lnr] = ' ';
	char old_rule[128];
	int i;
	for ( i = 1; i<old_rec[0]; i++ ) {
		read_string( old_rec[i], old_rule);  
		if ( strlen( old_rule ) > lnr && !strncmp( new_rule, old_rule, lnr+1 ) ) { // rule already there
			new_rule[lnr] = '\0';
			return 1;
		}
	}

	// not found.
	// alloc and copy for old 
	long* new_rec = new long [(old_rec[0]+1)*sizeof(long)];
	memcpy( new_rec, old_rec, old_rec[0]*sizeof(long) );

	// modify for new at end
	if ( at_number == 0 )
		at_number = onemore_brillrule();
	int p;
	sprintf( new_rule+lnr, " %d", at_number );
	new_rec[ new_rec[0] ] = p = alloc_and_write_string( new_rule );
	new_rec[0] ++;

	// save
	hashPutCI( _brillrule_hashdic_, rulename, new_rec );
	delete [] new_rec;

	// add reference to number.
	char strnum[24];
	sprintf( strnum, "%d", at_number );
	long short_rec[2];
	short_rec[0] = 2;
	short_rec[1] = p;
	hashPutCI( _brillrule_hashdic_, strnum, short_rec );

	return 1;
}

static void replace_number( int after, int before )
{
	char str[24];
	sprintf( str, "%d", before );
	long* _rec = hashGetCI( _brillrule_hashdic_, str );
	if ( ! _rec ) return;
	if ( _rec[1] == -1 ) return;
	long oldp = _rec[1];

	// changes the number in the rule.
	char tr[MAXSIZEBRILLRULE];
	read_string( oldp, tr );
	char* lb = strrchr(tr, ' ');
	if (lb) {
		sprintf( lb+1, "%d", after );
		write_string( oldp, tr );
	}

	// changes the reference to the rule.
	sprintf( str, "%d", after );
	_rec = hashGetCI( _brillrule_hashdic_, str );
	if ( ! _rec ) return;
	_rec[1] = oldp;
	hashPutCI( _brillrule_hashdic_, str, _rec );
}

int add_brillrule_at_number( int num, char* word, char* tag, char* rule )
{
// msg( "addn %d %s %s [%s]\n", num, word, tag, rule );

	int i;
	int nbr = nb_brillrules();
	if ( num > nbr )
		return add_brillrule( word, tag, rule );
	
	// find a free spot.
	char tr[MAXSIZEBRILLRULE];
	for (i = num; i <= nbr ; i++ ) {
		if ( !get_brillrule_by_number(i,tr) ) break;
	}

	if ( i == nbr+1 ) { // no free spot
		nbr++; // free spot will exist and will be last number.
		onemore_brillrule();
		// add a rule entry to nbr.
		char strnum[24];
		sprintf( strnum, "%d", nbr );
		long short_rec[2];
		short_rec[0] = 2;
		short_rec[1] = -1;
		hashPutCI( _brillrule_hashdic_, strnum, short_rec );
	}

	// now shift all the rules
	while (i>num) {
		replace_number(i,i-1);
		i--;
	}

	add_brillrule( word, tag, rule, num );

	return 1;
}

int suppress_brillrule( char* word, char* tag, char* bad_rule, int full )
{
	char rulename[128];
	sprintf( rulename, "(%s) (%s)", word, tag );

	long* old_rec = hashGetCI( _brillrule_hashdic_, rulename );
	if (!old_rec)
		return 0; // rules does not exist

	int lnr = strlen(bad_rule);
	bad_rule[lnr] = ' ';
	char old_rule[128];
	int i;
	for ( i = 1; i<old_rec[0]; i++ ) {
		read_string( old_rec[i], old_rule);
		if ( !strncmp( bad_rule, old_rule, lnr+1 ) ) { // rule is there
			bad_rule[lnr] = '\0';
			for (int k=i; k<old_rec[0]-1; k++)
				old_rec[k] = old_rec[k+1];
			old_rec[0] --;
			hashPutCI( _brillrule_hashdic_, rulename, old_rec ); // replace entry.
			if (full) {
				// suppress reference to number.
				int n = atoi( old_rule+lnr+1 );
				suppress_brillrule_by_number( n, 0 );
			}
			return 1;
		}
	}
	bad_rule[lnr] = '\0';
	return 0; // rules does not exist
}

int suppress_brillrule_by_number( int n, int full )
{
	char str[24];
	sprintf( str, "%d", n );
	long* _rec = hashGetCI( _brillrule_hashdic_, str );
	if ( ! _rec ) return 0;
	if ( _rec[1] == -1 ) return 0;
	int prev = _rec[1];
	_rec[1] = -1;
	hashPutCI( _brillrule_hashdic_, str, _rec ); // replace entry.

	if (!full) return 1;

	char tr[MAXSIZEBRILLRULE];
	read_string( prev, tr );
	char word[128], tag[128];
	sscanf( tr, "%s %s", word, tag );
	char* lb = strrchr(tr, ' ');
	if (lb) *lb = '\0';

	suppress_brillrule( word, tag, tr, 0 );
	return 1;
}

long* get_brillrule(const char* word, char* tag, char* ctn, int& _nrec)
{
	char rulename[128];
	sprintf( rulename, "(%s) (%s)", word, tag );
	long* _rec = hashGetCI( _brillrule_hashdic_, rulename );
	_nrec = -1;
	if (!_rec || _rec[0] <1)
		return 0;
	else {
		long* _store = new long [_rec[0]*sizeof(long)];
		memcpy( _store, _rec, _rec[0]*sizeof(long) );
		_nrec = 2;
		read_string( _rec[1], ctn );
		return _store;
	}
}

char* get_next_brillrule(char* ctn, int& _nrec, long* _rec)
{
	if ( _nrec >= _rec[0] ) return 0;
	read_string( _rec[ _nrec ], ctn );
	_nrec ++;
	return ctn;
}

char* get_brillrule_by_number(int number, char* ctn)
{
	char str[24];
	sprintf( str, "%d", number );
	long* _rec = hashGetCI( _brillrule_hashdic_, str );
	if ( ! _rec ) return 0;
	if ( _rec[1] == -1 ) return 0;
	if ( !ctn ) return (char*)1;	// test if number exists only
	read_string( _rec[1], ctn );
	return ctn;
}

void list_brillrule(FILE* out, int style)
{
	if ( style == 2 ) {
		int nb=nb_brillrules();
		for (int n=1; n<=nb; n++) {
			char rr[256];
			if ( !get_brillrule_by_number(n,rr) ) continue;
			msgfile( out, "/%d/\t[%s]\n", n, rr );
		}
	} else {
		hashiteratorinit( _brillrule_hashdic_ );
		loopr* l=0;
		while ((l=hashiteratorloopCI( _brillrule_hashdic_ ))) {
			if ( !strcmp(BRILLNBRULES, l->s) ) continue;
			if ( l->s[0] != '(' ) continue;
			char rr[256];
			for (int i=1;i<l->e[0];i++) {
				read_string( l->e[i], rr );
				//msgfile( out, "%s", l->s );
				msgfile( out, "[%s]\n", rr );
			}
		}
	}
	msgfile( out, "\n" );
	hashstat( _brillrule_hashdic_, out );
}
