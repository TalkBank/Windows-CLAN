/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- mortags.cpp	- v1.0.0 - november 1998

   library of functions used to compute the tag format of mor and clan files.

|=========================================================================================================================*/
#include "system.hpp"

#include "maxconst.hpp"

#include "tags.hpp"
#include "mortags.hpp"
#include "storage.hpp"
#include "database.hpp"
#include "hashc.hpp"

char Separator1 = Separator1default;
char Separator2 = Separator2default;


const char *strchrConst(const char *src, int c) {
	for (; *src != c && *src != EOS; src++) ;
	if (*src == EOS)
		return(NULL);
	else
		return(src);
}

static int compare( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
   return _stricmp( * ( char** ) arg1, * ( char** ) arg2 );
}

void sort_and_pack_in_string( char** S1, int n, char* st1, int maxlen, const char* sep )
{
	int i,c;

	/* Sort using Quicksort algorithm: */
	qsort( (void *)S1, (size_t)n, sizeof( char * ), compare );
	for ( c=0,i=1; i<n; i++ ) {
		if ( !strcmp(S1[c], S1[i]) ) // identical as previous.
			continue;
		c++;
		if (c!=i) S1[c] = S1[i];
	}
	c++;

	int len = 0;
	st1[0] = '\0';
	len += strlen(S1[0]);
	if ( len > maxlen ) return;
	strcat( st1, S1[0] );
	for (i=1;i<c;i++) {
		len += strlen(S1[i]+1);
		if ( len > maxlen ) return;
		strcat(st1, sep );
		strcat(st1, S1[i] );
	}
}

// #define debug_split
// #define dbg_xml 1
// #define dbg_xml0 1

/*
*** int split_multiple_mortag(char* initial_string, char** &st, int withtagsuffix);
*** 	This function tries to split mortags of the following kind (square brackets are for clarity sake, 
*** 		there are not part of the actual computer representation):
*** 	(1) ambiguous example with *joker* inserted as necessary
*** 	[a$b~c~d^e~f^k^x$b] ==> [a^e^k^x] [b^f^k:joker^b] [c^f:joker^k:joker^b:joker] [d^f:joker^k:joker^b:joker] 
*** 	(2) non-ambiguous example
*** 	[a$b$c~d~e~f] ==> [a] [b] [c] [d] [e] [f]

*** ALTERNATIVE: do not split but create compound tag

	(1) ambiguous example 
 	[a$b~c~d^e~f^k^x$b] ==> [a:b:c:d^e:f^x:b] 
 	(2) non-ambiguous example
 	[a$b$c~d~e~f] ==> [a:b:c:d:e:f]
*/
int make_multi_categ(char* initial_string, char* &st )
{
#ifdef dbg_xml
	msg("initial: %s\n", initial_string );
#endif
	int i;
	char*  W[MaxNbTags];
	char*  TR[MaxNbTags];
	// int    N[MaxNbTags];

	if ( initial_string[0] == '+' )
		return make_multi_categ_single( initial_string, st );

	char* savestring = dynalloc( strlen(initial_string)+1 );
	strcpy( savestring, initial_string );
	int n = split_with( savestring, W, "^", MaxNbTags );

#ifdef dbg_xml
	msg("(split in) %d\n", n );
#endif
	if (n==1) {
		return make_multi_categ_single( initial_string, st );
	}

#ifdef dbg_xml
	msg("(split ambiguous in) %d\n", n );
#endif

	// from [a$b~c~d^e~f^k^x$b] contructs (in TR) [multi:a:b:c:d] [multi:e:f] [k] [multi:x:b]
	for (i=0;i<n;i++) {

#ifdef dbg_xml
	msg("split[%d]: %s\n", i, W[i] );
#endif

		make_multi_categ_single( W[i], TR[i] );
	}

#ifdef dbg_xml
	for (i=0;i<n;i++) {
		msg("[%d] !%s!\n", i, TR[i]?TR[i]:W[i] );
	}
#endif

	int l = 0;
	for (i=0;i<n;i++) {
			l += strlen(TR[i]?TR[i]:W[i]) + 1;
	}

	// concatene multiple categories
	st = (char*)dynalloc( l + 1);
	strcpy( st, TR[0]?TR[0]:W[0] );
	for (i=1;i<n;i++) {
		strcat( st, "^" );
		strcat( st, TR[i]?TR[i]:W[i] );
	}

#ifdef dbg_xml
	msg("%s\n", st );
#endif

	return 1;
}

static int split_with_b(char* line, char** words, char* res, const char* seps, const char* sepskeep, int maxofwords, int maxofres )
{
	char element [2048];
	int i = strsplitsepnosep( line, 0, element, seps, sepskeep, 1024 );
	int n = 0;
	while ( i != -1 ) {
		if ( strlen(element) >= maxofres ) return n;

		strcpy( res, element );
		words[n++] = res;
		res += strlen(element)+1;
		maxofres -= strlen(element)+1;

		if (n>=maxofwords) return n;

		i = strsplitsepnosep( line, i, element, seps, sepskeep, 1024 );

	}
	return n;
}

static char* mortag_to_string_filtered_in( mortag* mt, char* storage )
{
	int k;
	if ( !mt || !mt->MT ) {
		strcpy( storage, "0" );
		return storage;
	}
	storage[0] = '\0';
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] == MorTagComplementsPrefix && filter_ok(mt->SP[k]) ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( storage, mt->SP[k] );
			strcat( storage , s ) ;
		}
	}
	strcat( storage, mt->MT );
	if ( mt->GR && filter_ok(mt->GR)) {
		strcat( storage , "|" ) ;
		strcat( storage, mt->GR );
	}
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] != MorTagComplementsPrefix && filter_ok(mt->SP[k]) ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( storage , s ) ;
			strcat( storage, mt->SP[k] );
		}
	}
	return storage;
}

/*
int split_multiple_mortag_single(char* initial_string, char** &st, int withtagsuffix);
	This function splits mortags of the following kind (square brackets are for clarity sake, 
		there are not part of the actual computer representation):
	[a$b$c~d~e~f] ==> [multi:a:b:c:d:e:f]
	Note: The tag is ALWAYS non-ambiguous
*/
int make_multi_categ_single(char* initial_string, char* &st)
{
	char *parts[MaxParts], *prefs;
	char new_string[2048];
	int n = split_with_b(initial_string, parts, new_string, " ", TagGroups, MaxParts, sizeof(new_string) );
	if (n==1) {
		st = (char*)0;
		return 0;
	}

#ifdef dbg_xml0
	for (int i=0;i<n;i++) {
		msg( "[%d]=/%s/\n", i, parts[i] );
	}
#endif

	/* each even element is a separator */
	
	int s=0;
	for (int i=0; i<n ; i+=2) {
		s += (strlen(parts[i]) * 2) + 1;
	}

	st = (char*)dynalloc( (s+1+strlen(MultiCategory))*sizeof(char) );
	st[0] = EOS;
// lxs change 2016-10-13 grandmother's
	prefs = NULL;
	if (n > 1) {
		prefs = strrchr(parts[0], MorTagComplementsPrefix);
//		if (prefs != NULL) {
//			*prefs = EOS;
//			strcat(st, parts[0]);
//			strcat(st, "#");
//			*prefs = '#';
//		}
	}
// lxs change 2016-10-13 grandmother's
	strcat(st, MultiCategory );
	
	// first copy all category names, including filtered suffixes
	// int make_mortag(char* str, mortag* mt)
	// char* mortag_to_string_filtered( mortag* mt, char* storage )
	for (int i=0;i<n;i+=2) {
		mortag mt;
		char str1[MaxSzTag];
		char str2[MaxSzTag];
		if (i == 0 && prefs != NULL)
			strcpy(str1, prefs+1);
		else
			strcpy(str1, parts[i]);
		make_mortag( str1, &mt);
		mortag_to_string_filtered_in( &mt, str2 );
		char* p = strchr( str2, '|' );
		if (p)
			*p = '-';
		// strcpy( str2, mortag_to_string( &mt ) );
#ifdef dbg_xml0
		if (i == 0 && prefs != NULL)
			msg("(%s) (%s)\n", prefs+1, str2 );
		else
			msg("(%s) (%s)\n", parts[i], str2 );
#endif
		strcat( st, "/" );
		strcat( st, str2 );
	}
#ifdef dbg_xml0
	msg("!!%s!!\n", st);
#endif
	
	// next copy all rest of words
	strcat(st, "|+" );
	strcat( st, parts[0] );
	for (int i=2;i<n;i+=2) {
		strcat( st, "\001" ); // compund fix 2010-12-17 "+"
		strcat( st, parts[i-1] );
		strcat( st, parts[i] );
	}
#ifdef dbg_xml0
	msg("    @@%s@@\n", st);
#endif
	/**** version that does not keep information about separators
	
	char *parts[MaxParts];
	int n = split_with(initial_string, parts, TagGroups, MaxParts );
	if (n==1) {
		st = (char*)0;
		return 0;
	}
	
	int s=0;
	for (int i=0; i<n ; i++) {
		s += strlen(parts[i])*2+1;
	}
			
	st = (char*)dynalloc( (s+1+strlen(MultiCategory))*sizeof(char) );
	strcpy(st, MultiCategory );
	
	// first copy all category names
	for (int i=0;i<n;i++) {
		char* p = strchr( parts[i], '|' );
		int ll;
		if (p)
			ll = p-parts[i];
		else
			ll = strlen(parts[i]);
		strcat( st, "/" );
		int e = strlen(st);
		strncpy(st+e, parts[i], ll );
		st[e+ll] = '\0';
	}
	
	// next copy all rest of words
	strcat(st, "|" );
	for (int i=0;i<n;i++) {
		strcat( st, "+" );
		strcat( st, parts[i] );
	}
	****/
	
	return 1;
}

/*
int split_mortag(char* initial_string, char* &maintag, char** subparts, int* subpartstype, char* &genericroot, int maxofsubparts, char* &translation)
	Extracts different parts of a tag generated by MOR
	The syntax for mortag is the following:
	        'maintag'|'genericroot'&-subpart1&-subpart2&-subpart3...
	     or 'maintag'&-subpart1&-subpart2&-subpart3...

	The parameters passed by value (&), maintag, genericroot, translation return a value.
	// previous altMT suppressed replaced by translation
	The char pointers to the different subparts will be returned using the subparts parameter.

	The subparts parameter point to an array already allocated with a size given by the 'maxofsubparts' parameter
	The subpartstype parameter point to an array already allocated with a size given by the 'maxofsubparts' parameter
 
	The function returns the number of subparts. In subpartstype, it is indicated whether the suffix is non-fusional '-' or fusional '&'
	If the string is empty (no word at all), the function returns -1.

	NO MEMORY ALLOCATION IS PERFORMED. The original string (initial_string) is used as memory storage.
	AFTER THIS FUNCTION, THE PARAMETER initial_string cannot be used any more by the calling function.
	It is up to the calling function to made a copy of the parameter before the call in order to used it later.
*/

static int findsep(char* str, const char* seps )
{
	int s;
	for (s = 0; str[s] ; s++ ) {
		if ( strchrConst( seps, str[s] ) ) break;
	}
	if ( str[s] )
		return s;
	else
		return -1;
}

int split_mortag(char* mortag, char* &maintag, char** subparts, char* subpartstype, char* &genericroot, int maxofsp, char* &translation)
{
	int n = 0;
	translation= 0;

	// find translation:

	int t = findsep( mortag, MorTagTranslation );
	if (t!=-1) {
		mortag[t] = '\0';
		translation = mortag + t + 1;
	}

	char* p2 = strchr( mortag, MorTagMainSeparator );
	if (p2==0) p2 = strchr( mortag, MorTagComplementsRegular );
	if (p2==0) p2 = strchr( mortag, MorTagComplementsIrregular );

	if (!p2) {
		// this is the case of tags that contains only a TAG and maybe prefixes
		genericroot = 0;
		// find prefixes.
		char* pref[32];
//		int np = split_with(mortag, pref, MorTagComplementsPrefixes, 32 );
		int np = split_Prefixes(mortag, pref, 32 );
		if (np>1) {
			int i;
			for (i=0;i<np-1;i++) {
				subpartstype[n] = MorTagComplementsPrefix;
				subparts[n++] = pref[i];
			}
			maintag = pref[i];
		} else
			maintag = mortag;
		return n;
	}

	char sepa = *p2;	// remember wich sepa we found.
	*p2 = '\0';
	p2++;	// gets the rest of the line after the sepa
	maintag = mortag;

	if ( p2[0] == '\0' ) {
		genericroot = 0;
		return 0;
	} 
	{
		// first find prefixes.
		char* pref[32];
		int i;
//		int np = split_with(maintag, pref, MorTagComplementsPrefixes, 32 );
		int np = split_Prefixes(maintag, pref, 32 );
		if (np>1) {
			for (i=0;i<np-1;i++) {
				subpartstype[n] = MorTagComplementsPrefix;
				subparts[n++] = pref[i];
			}
			maintag = pref[i];
		}

		if (sepa == MorTagMainSeparator) {
			genericroot = p2;
		} else {
			genericroot = 0;
			subpartstype[n] = sepa;
			subparts[n++] = p2;
			if (n>=maxofsp) return n;
		}

		if ( p2[0] == MorTagCompoundWord ) {
			int l = strlen( p2 );
			for (;l>0;l--)
				if (p2[l] == MorTagMainSeparator || p2[l] == MorTagCompoundWord)
					break;
			if (l>0) {
				i = findsep( p2+l, MorTagComplements );
				if ( i != -1) i += l;
			} else
				i = -1;
		} else
			i = findsep( p2, MorTagComplements );

		if ( i == -1 ) {
			return n;
		} else {
			sepa = p2[i]; 
			p2[i] = '\0';
			p2 += i+1;
			while ( (i = findsep( p2, MorTagComplements )) != -1 ) {
				subpartstype[n] = sepa;
				subparts[n++] = p2;
				if (n>=maxofsp) return n;
				sepa = p2[i];
				p2[i] = '\0';
				p2 += i+1;
			}
			subpartstype[n] = sepa;
			subparts[n++] = p2;
			return n;
		}
	}
}

/*
int split_with(char* line, char** words, char* seps, int maxofwords)
	This function splits the parameter 'line' into words
	The char pointers to the different words will be returned using the words parameter.
	The words parameter point to an array already allocated with a size given by the 'maxofwords' parameter
 
	The function returns the number of words.

	'seps' is a list of characters to be used as separators.

	NO MEMORY ALLOCATION IS PERFORMED. The original string (line) is used as memory storage.
	AFTER THIS FUNCTION, THE PARAMETER initial_string cannot be used any more by the calling function.
	It is up to the calling function to made a copy of the parameter before the call in order to used it later.
*/
/*	In this function separators are lost from the original string after segmentation
*/
int split_with(char* line, char** words, const char* seps, int maxofwords)
{
	char* s = strtok( line, seps );
	int n = 0;
	while ( s ) {
		words[n++] = s;
		if (n>=maxofwords) return n;
		s = strtok( NULL, seps );
	}
	return n;
}

// lxs change 2016-10-13 grandmother's
int split_Prefixes(char* line, char** words, int maxofwords)
{
	int n = 0;
	char *s;

	s = strtok( line, MorTagComplementsPrefixes );
	while ( s ) {
		words[n++] = s;
		if (n>=maxofwords)
			return n;
		s = strtok( NULL, MorTagComplementsPrefixes );
	}
	return n;
}
// lxs change 2016-10-13 grandmother's

/*	In this function separators from the parameter 'seps' are lost from the original string after segmentation
	and separators from the parameters sepskeep remain in the original string after segmentation.
	The output string can be longer than the original string, as a NULL character is added for
	each separator to be kept. For this reason, a copy of the original is done in string parameter 'res'.
*/
int split_with_a(char* line, char** words, char* res, const char* seps, const char* sepskeepatend, const char* sepskeep, int maxofwords, int maxofres )
{
	char sk[129];
	if ( sepskeep == 0 )
		sk [0] = '\0';
	else {
		if (strlen(sepskeep) < 127)
			strcpy( sk, sepskeep );
		else {
			strncpy( sk, sepskeep, 127 );
			sk[128] = '\0';
		}
	}
	char skae[129];
	if ( sepskeepatend == 0 )
		skae [0] = '\0';
	else {
		if (strlen(sepskeepatend) < 127)
			strcpy( skae, sepskeepatend );
		else {
			strncpy( skae, sepskeepatend, 127 );
			skae[128] = '\0';
		}
	}
	int i = strlen( line );
	if ( i > 0 && strchr( skae, line[i-1] ) ) {
		int e = i;
		// add a white space before the end of the line.
		do {
			i--;
		} while ( i> 0 && strchr( skae, line[i-1] ) );
		while ( e >= i ) {
			line [e+1] = line[e];
			e--;
		}
		line[i] = ' ';
	}
	char element [2048];
	i = strsplitsepnosep( line, 0, element, seps, sk, 1024 );
	int n = 0;
	while ( i != -1 ) {
		if ( strlen(element) >= maxofres ) return n;

		strcpy( res, element );
		words[n++] = res;
		res += strlen(element)+1;
		maxofres -= strlen(element)+1;

		if (n>=maxofwords) return n;

		i = strsplitsepnosep( line, i, element, seps, sk, 1024 );

	}
	return n;
}

/*
int make_mortag(char* str, mortag* mt)
	This function creates a mortag in memory using the dynalloc allocation scheme.
	'str' is the original string 
	The result parameter 'mt' is already allocated. Only the 'SP' field of the 'mt' structure will be allocated.
	Returns 1 if OK, 0 if no memory is available.
*/
int make_mortag(char* str, mortag* mt)
{
#ifdef debug_split
	msg("MOR /%s/ ==> ", str );
#endif

	char* sp[32];
	char  spt[32];
	mt->nSP = split_mortag( str, mt->MT, sp, spt, mt->GR, 32, mt->translation );
	if (mt->nSP == -1) {
		mt->MT = dynalloc(4);
		if (!mt->MT) return 0;
		strcpy( mt->MT, "000" );	// the 'str' string was empty.
		mt->nSP = 0;
		mt->GR = 0;
		return 1;
	}

	if (mt->nSP == 0) {
		mt->SP = 0;
		mt->nSP = 0;
#ifdef debug_split
		print_mortag_2(stdout, mt);
		msg("\n");
#endif
		return 1;
	}

	// The sp variable is copied into the resulting 'mt' structure (with allocation of new space)
	mt->SP = (char**)dynalloc( mt->nSP * sizeof(char*) );
	mt->SPt = (char*)dynalloc( mt->nSP * sizeof(char) );
	if ( !mt->SP ) return 0; // no memory.
	for ( int k=0; k<mt->nSP ; k++) {
		mt->SP[k] = dynalloc( strlen(sp[k])+1 );
		if ( !mt->SP[k] ) return 0; // no memory.
		strcpy( mt->SP[k], sp[k] );
		mt->SPt[k] = spt[k];
	}
#ifdef debug_split
	print_mortag_2(stdout, mt);
	msg(" {%s}\n", mortag_to_string(mt) );
#endif
	return 1;
}

/*
int make_mortag_from_heap(char* str, mortag* mt)
	This function creates a mortag in memory using the new/delete allocation scheme.
	'str' is the original string 
	The result parameter 'mt' is already allocated. 
	The 'SP' field of the 'mt' structure will be allocated, as well as the MT field.
	Returns 1 if OK, 0 if no memory is available.
*/
int make_mortag_from_heap(char* str, mortag* mt)
{
#ifdef debug_split
	msg("MOR /%s/ ==> ", str );
#endif

	char* sp[32];
	char  spt[32];
	char  *lMT;
	char  *ltranslation;
	char  *lGR;
	mt->nSP = split_mortag( str, lMT, sp, spt, lGR, 32, ltranslation );
	if (mt->nSP == -1) {
		mt->MT = new char[4];
		if (!mt->MT) return 0;
		strcpy( mt->MT, "000" );	// the 'str' string was empty.
		mt->nSP = 0;
		mt->GR = 0;
		return 1;
	}

	if (mt->nSP == 0) {
		mt->MT = new char [strlen(lMT)+1];
		strcpy( mt->MT, lMT );
		if (ltranslation) {
			mt->translation = new char [strlen(ltranslation)+1];
			strcpy( mt->translation, ltranslation );
		}
		if (lGR) {
			mt->GR = new char [strlen(lGR)+1];
			strcpy( mt->GR, lGR );
		}
		mt->SP = 0;
		mt->nSP = 0;
#ifdef debug_split
		print_mortag_2(stdout, mt);
		msg("\n");
#endif
		return 1;
	}

	// The sp variable is copied into the resulting 'mt' structure (with allocation of new space)
	mt->SP = new char* [mt->nSP];
	mt->SPt = new char [mt->nSP];
	if ( !mt->SP ) return 0; // no memory.
	int k;
	for ( k=0; k<mt->nSP ; k++) {
		mt->SP[k] = new char [strlen(sp[k])+1];
		if ( !mt->SP[k] ) return 0; // no memory.
		strcpy( mt->SP[k], sp[k] );
		mt->SPt[k] = spt[k];
	}
	mt->MT = new char [strlen(lMT)+1];
	strcpy( mt->MT, lMT );
	if (ltranslation) {
		mt->translation = new char [strlen(ltranslation)+1];
		strcpy( mt->translation, ltranslation );
	}
	if (lGR) {
		mt->GR = new char [strlen(lGR)+1];
		strcpy( mt->GR, lGR );
	}
#ifdef debug_split
	print_mortag_2(stdout, mt);
	msg(" {%s}\n", mortag_to_string(mt) );
#endif
	return 1;
}

void delete_mortag_from_heap(mortag* mt)
{
	if (mt->MT) delete mt->MT;
	if (mt->translation) delete mt->translation;
	if (mt->GR) delete mt->GR;
	if (mt->SPt) delete mt->SPt;
	if (mt->SP) {
		for (int i=0;i<mt->nSP;i++)
			if (mt->SP[i]) delete mt->SP[i];
		delete mt->SP;
	}
}

/*
mortag* heap_copy_of(mortag* mt)
{
	mortag* al = new mortag;
	if (mt->MT) {
		al->MT = new char[strlen(mt->MT)+1];
		strcpy( al->MT, mt->MT );
	}
	if (mt->translation) {
		al->translation = new char[strlen(mt->translation)+1];
		strcpy( al->translation, mt->translation );
	}
	if (mt->GR) {
		al->GR = new char[strlen(mt->GR)+1];
		strcpy( al->GR, mt->GR );
	}
	if (mt->nSP) {
		al->nSP = mt->nSP;
		al->SP = new char* [mt->nSP];
		al->SPt = new char [mt->nSP];
		memcpy( al->SPt, mt->SPt, mt->nSP );
		int k;
		for (k=0;k<=mt->nSP;k++) {
			if (mt->SP[k]) {
				al->SP[k] = new char[strlen(mt->SP[k])+1];
				strcpy( al->SP[k], mt->SP[k] );
			}
		}
	}
	return al;
}
*/

static char** gbin_filter_ok=0 ;	// list of sub-tag names for to keep for analysis
static int gbin_filter_nb=0 ;	// size of the above list

static int _hashdic_filter_in_ = -1;

static char** gbout_filter_ok=0 ;	// list of sub-tag names for to keep for output
static int gbout_filter_nb=0 ;	// size of the above list

static int gbin_filter_readonly = 0; // if equal 1 adding sub-tag names for input is forbidden.

void init_mortags(void) {
	gbin_filter_readonly = 0;
	gbin_filter_ok=0 ;	// list of sub-tag names for to keep for analysis
	gbin_filter_nb=0 ;	// size of the above list
	_hashdic_filter_in_ = -1;
	gbout_filter_ok=0 ;	// list of sub-tag names for to keep for output
	gbout_filter_nb=0 ;	// size of the above list
}

// get list of sub-tags to kept for input or output from an external file
void gb_filter_init_from_file(FNType* gb_filter_name, char** &gb_filter_ok, int &gb_filter_nb, int put_in_memory, char** gb_not=0, int gb_not_nb=0);
void gb_filter_init_from_file(FNType* gb_filter_name, char** &gb_filter_ok, int &gb_filter_nb, int put_in_memory, char** gb_not, int gb_not_nb)
{
	FILE* f = 0;
	if ( gb_filter_name ) {
		f = fopen( gb_filter_name, "r" );
		if (!f) {
#ifdef POSTCODE
			FNType mFileName[FNSize];
			f=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, mFileName);
			if (f == NULL) {
				f= OpenGenLib(gb_filter_name,"r",FALSE,FALSE, mFileName);
				if (f != NULL)
					fprintf(stderr, "    Using tags file: %s.\n\n", mFileName);
			} else
				fprintf(stderr, "    Using tags file: %s.\n\n", mFileName);
			if (f == NULL) {
#endif
			gb_filter_ok = (char**)1; // anything different from NULL
			gb_filter_nb = 0;
			return;
#ifdef POSTCODE
			}
#endif
		}
	} else {
		gb_filter_ok = (char**)1; // anything different from NULL
		gb_filter_nb = 0;
		return;
	}

	char line[256];
// msg("m= %d\n", sizeof(char*) * MaxNbSubtags );
	gb_filter_ok = new char * [MaxNbSubtags];	// max nb of sub-tags == MaxNbSubtags.
	while ( fgets(line, 256, f) ) {
		char w[256];
		int n;

		if (strncmp(line, UTF8HEADER, 5) == 0 || strncmp(line, WINDOWSINFO, 8) == 0 || strncmp(line, FONTHEADER, 6) == 0)
			continue;
		n = sscanf( line, "%s", w );
		if (w[0] == MorTagComment ) continue;
		if (w[0] == '\0' ) continue;
		if (n < 1) continue;

		if ( gb_not ) {
			int found = 0;
			for (int kk=0; kk<gb_not_nb; kk++) {
				if ( !strcmp( w, gb_not[kk] ) ) {
					found = 1;
					break;
				}
			}
			if ( found == 1 ) continue;
		}
// msg("m= %d\n", strlen(w)+1 );
		gb_filter_ok[gb_filter_nb] = new char [strlen(w)+1];
		strcpy( gb_filter_ok[gb_filter_nb], w );
		gb_filter_nb ++;
		if (gb_filter_nb == MaxNbSubtags) break;
	}
	fclose(f);

	if ( ! put_in_memory ) return;
	// Now create and write to memory.

	_hashdic_filter_in_ = hashcreate( "_filter_in_", MaxSizeAffix );

	char xn[MaxSizeAffix];

	for ( int i = 0; i< gb_filter_nb; i++ ) {
		sprintf (xn, "e%d", i );
		hashPutCC( _hashdic_filter_in_, xn, gb_filter_ok[i] );
	}

	sprintf( xn, "%d", gb_filter_nb );
	hashPutCC( _hashdic_filter_in_, "nb_filters", xn );
	
}

// get list of sub-tags to kept for input or output from the database
int gb_filter_init_from_memory(char** &gb_filter_ok, int &gb_filter_nb);
int gb_filter_init_from_memory(char** &gb_filter_ok, int &gb_filter_nb) // should be used always with gbin_...
{
	if ( hashprobe( "_filter_in_" ) != 1 ) return 0;

	// Now read from memory.

	_hashdic_filter_in_ = hashopen( "_filter_in_" );

	char* x = hashGetCC( _hashdic_filter_in_, "nb_filters" );

	if (!x) return 0;

	sscanf( x, "%d", &gb_filter_nb );

	gb_filter_ok = new char * [MaxNbSubtags];	// max nb of sub-tags == MaxNbSubtags.

	for ( int i = 0; i< gb_filter_nb; i++ ) {
		char xn[MaxSizeAffix];
		sprintf (xn, "e%d", i );
		x = hashGetCC( _hashdic_filter_in_, xn );

		gb_filter_ok[i] = new char [strlen(x)+1];
		strcpy( gb_filter_ok[i], x );
	}

	return 1;
}

void gbin_filter_initialize()
{
	if ( !gbin_filter_ok ) {
		_hashdic_filter_in_ = hashcreate( "_filter_in_", MaxSizeAffix );
		gbin_filter_ok = new char * [MaxNbSubtags];	// max nb of sub-tags == MaxNbSubtags.
		gbin_filter_nb = 0;
	}
}

static void gbin_filter_addelement(char* e)
{
	char xn[MaxSizeAffix];
	if (gbin_filter_readonly == 1) return;
	if (!gbin_filter_ok) {
		msg( "Warning: parameter +e used in POSTTRAIN incompatible with current database\n");
		return;
	}

	if (gbin_filter_nb == MaxNbSubtags) return;

	if (strlen(e)>=MaxSizeAffix) e[MaxSizeAffix-1] = '\0';
	gbin_filter_ok[gbin_filter_nb] = new char [strlen(e)+1];
	strcpy( gbin_filter_ok[gbin_filter_nb], e );

	sprintf (xn, "e%d", gbin_filter_nb );
	hashPutCC( _hashdic_filter_in_, xn, e );

	gbin_filter_nb ++;
	sprintf( xn, "%d", gbin_filter_nb );
	hashPutCC( _hashdic_filter_in_, "nb_filters", xn );
}

// free list of sub-tags 
void gb_filter_free(char** &gb_filter_ok, int &gb_filter_nb);
void gb_filter_free(char** &gb_filter_ok, int &gb_filter_nb)
{
	if (gb_filter_ok != (char**)0 && gb_filter_ok != (char**)1) {
		for ( int i = 0; i<gb_filter_nb; i++ )
			delete [] gb_filter_ok[i] ;
		gb_filter_nb = 0;
		delete [] gb_filter_ok ;
		gb_filter_ok = (char **)0;
	}
}

void list_filter(FILE* out)
{
	if ( _hashdic_filter_in_ == -1 ) return;
	msgfile( out, "number of suffixes filters: %ld\n", gbin_filter_nb );
	for ( int i = 0; i< gbin_filter_nb; i++ ) {
		msgfile( out, "filter[%d] = {%s}\n", i+1, gbin_filter_ok[i] );
	} 
}

void gbin_filter_create(FNType* filename)
{
	gb_filter_init_from_file(filename, gbin_filter_ok, gbin_filter_nb, 1);
}

int gbin_filter_open()
{
	gbin_filter_readonly = 1;
	return gb_filter_init_from_memory(gbin_filter_ok, gbin_filter_nb);
}

void gbin_filter_free()
{
	gb_filter_free(gbin_filter_ok, gbin_filter_nb);
}

void gbout_filter_create(FNType* filename)
{
	gb_filter_init_from_file(filename, gbout_filter_ok, gbout_filter_nb, 0, gbin_filter_ok, gbin_filter_nb);
}

void gbout_filter_free()
{
	gb_filter_free(gbout_filter_ok, gbout_filter_nb);
}


// Test if a string correspond to a subclass tag (and so should be kept in MORTAGS). ONLY for input.
int filter_ok( char* sp )
{
	int i;
//	if (!gb_filter_ok)
//		gb_filter_init();

	if (gbin_filter_nb==0) return 0;
	for (i=0;i<gbin_filter_nb;i++) {
		if ( !_stricmp( sp, gbin_filter_ok[i] ) )
			return 1;
	}
	return 0;
}

// Test if a string correspond to a subclass tag (and so should be kept for OUTPUT). ONLY for output.
int filter_ok_out( char* sp )
{
	if (gbout_filter_nb==0) return 0;
	for (int i=0;i<gbout_filter_nb;i++)
		if ( !_stricmp( sp, gbout_filter_ok[i] ) ) return 1;
	
	return 0;
}

char* mortag_to_string_filtered( mortag* mt, char* storage )
{
	int k;
	if ( !mt || !mt->MT ) {
		strcpy( storage, "0" );
		return storage;
	}
	storage[0] = '\0';
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] == MorTagComplementsPrefix && filter_ok_out(mt->SP[k]) ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( storage, mt->SP[k] );
			strcat( storage , s ) ;
		}
	}
	strcat( storage, mt->MT );
	if ( mt->GR && filter_ok_out(mt->GR)) {
		strcat( storage , "|" ) ;
		strcat( storage, mt->GR );
	}
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] != MorTagComplementsPrefix && filter_ok_out(mt->SP[k]) ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( storage , s ) ;
			strcat( storage, mt->SP[k] );
		}
	}
	return storage;
}

int is_eq_filtered( mortag* a, mortag* b)
{
	char sa[MaxSzTag];
	char sb[MaxSzTag];
#ifdef DBG0
	msgfile( stderr, "is_eq_filtered " );
	print_mortag_2( stderr, a );
	msgfile( stderr, " *** " );
	print_mortag_2( stderr, b );
	msgfile( stderr, "\n" );
#endif
	mortag_to_string_filtered(a,sa);
	mortag_to_string_filtered(b,sb);
	return !strcmp(sa,sb) ? 1 : 0 ;
}

void filter_add_all_affixes_of_mortag( mortag* mt )
{
	int k;
	if ( !mt || !mt->MT ) {
		return;
	}
	if ( is_pct(mt) ) {
		return;
	}
//	if ( !strncmp( MultiCategory, mt->MT, strlen(MultiCategory) ) ) return;

	for (k=0; k<mt->nSP ; k++) {
		if ( !filter_ok( mt->SP[k] ) ) {
			gbin_filter_addelement( mt->SP[k] );
		}
	}
}

void filter_add_all_affixes( ambmortag* amt )
{
	int k;
	for (k=0; k<amt->nA ; k++)
		filter_add_all_affixes_of_mortag( &amt->A[k] );
}

mortag* filter_mortag( mortag* mt )
{
	int k, w;
	if ( !mt || !mt->MT ) {
		return mt;
	}
	if ( is_pct(mt) ) {
		char* r = dynalloc( 4 );
		if ( !r ) return 0; // no memory.
		strcpy( r, "pct" );
		mt->MT = r;
		mt->SP = 0;
		mt->nSP = 0;
		return mt;
	}

	if ( !( mt->GR && filter_ok( mt->GR ) ) ) {
		mt->GR = 0;
	}
	for (k=0, w=0; k<mt->nSP ; k++) {
		if ( filter_ok( mt->SP[k] ) ) {
			mt->SP[w] = mt->SP[k];
			mt->SPt[w] = mt->SPt[k];
			w++;
		}
	}
	mt->nSP = w;
	return mt;
}

mortag* filter_mortag_keep_mainword( mortag* mt )
{
	int k, w;
	if ( !mt || !mt->MT ) {
		return mt;
	}
	if ( is_pct(mt) ) {
		char* r = dynalloc( 4 );
		if ( !r ) return 0; // no memory.
		strcpy( r, "pct" );
		mt->MT = r;
		mt->SP = 0;
		mt->nSP = 0;
		return mt;
	}

	for (k=0, w=0; k<mt->nSP ; k++) {
		if ( filter_ok( mt->SP[k] ) ) {
			mt->SP[w] = mt->SP[k];
			mt->SPt[w] = mt->SPt[k];
			w++;
		}
	}
	mt->nSP = w;
	return mt;
}

int mortags_cmp( mortag* mt1, mortag* mt2 )
{
	if ( !mt1 || !mt1->MT ) {
		if ( !mt2 || !mt2->MT )
			return 0;
		else
			return -1;
	}
	if ( !mt2 || !mt2->MT ) {
		return 1;
	}

	int t = _stricmp( mt1->MT, mt2->MT );
	if ( t != 0 ) return t;

	t = mt1->nSP - mt2->nSP;
	if ( t != 0 ) return t;

	for ( int k=0; k<mt1->nSP ; k++) {
		t = _stricmp( mt1->SP[k], mt2->SP[k] );
		if ( t != 0 ) return t;
	}

	if ( !mt1->GR ) {
		if ( !mt2->GR )
			return 0;
		else
			return -1;
	}
	if ( !mt2->GR ) {
		return 1;
	}

	t = _stricmp( mt1->GR, mt2->GR );
	return t;
}

char* mortag_to_string( mortag* mt )
{
	int k;
	char temp[MaxSizeOfString];
	if ( !mt || !mt->MT ) {
		char* r = dynalloc( 2 );
		if ( !r ) return 0; // no memory.
		strcpy( r, "0" );
		return r;
	}
	temp[0] = '\0';
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] == MorTagComplementsPrefix ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( temp, mt->SP[k] );
			strcat( temp , s ) ;
		}
	}
	strcat( temp, mt->MT );
	if ( mt->GR ) {
		strcat( temp , "|" ) ;
		strcat( temp, mt->GR );
	}
	for ( k=0; k<mt->nSP ; k++) {
		if ( mt->SPt[k] != MorTagComplementsPrefix ) {
			char s[2];
			s[0] = mt->SPt[k];
			s[1] = '\0';
			strcat( temp , s ) ;
			strcat( temp, mt->SP[k] );
		}
	}

	char* r = dynalloc( strlen(temp)+1 );
	if ( !r ) return 0; // no memory.
	strcpy( r, temp );
	return r;
}

// this function creates an ambiguous mortag in memory using the dynalloc allocation scheme
int make_ambmortag(char* str, ambmortag* amt)
{
	char* temp[MaxNbTags];
	if ( str[0] == '+' ) {
		amt->nA = 1;
		temp[0] = str;
	} else
		amt->nA = split_with( str, temp, AmbiguitySeparator, MaxNbTags );
	amt->A = (mortag*)dynalloc( amt->nA * sizeof(mortag) );
	if ( !amt->A ) return 0;	// no memory.
	for ( int j = 0; j < amt->nA ; j++ )
		if ( !make_mortag( temp[j], &amt->A[j]) ) return 0; // no memory.
	return 1;
}

int* ambmortag_to_multclass(ambmortag* amt, int* ma)
{
	// first pack equal values.
	int temp[MaxNbTags];
	int i;
	if (amt->nA > MaxNbTags) {
		// ??? warning (too many tags).
		ma[0] = 0;
		return ma;
	}
	for (i=0; i<amt->nA; i++)
		temp[i] = classname_to_tag(mortag_to_string(&amt->A[i]));
	isort ( temp, amt->nA );
	int w;
	for ( w=0,i=1; i<amt->nA; i++ ) {
		if (temp[w] == temp[i]) // identical as previous.
			continue;
		w++;
		if (w!=i)
			temp[w] = temp[i];
	}
	w++;

	ma[0] = w*2 +2 ;
	ma[1] = 0;
	for (i=0; i<w; i++) {
		ma[2+i*2] = temp[i];
		ma[2+i*2+1] = 0;
	}
	return ma;
}

// print functions
void print_mortag_2( FILE* out, mortag* mt )
{
	msgfile( out, "/%s/ [%s]", mt->MT?mt->MT:"", mt->GR?mt->GR:"" );
	if (mt->SPt) {
		for ( int k=0; k< mt->nSP; k++)
			msgfile( out, " (%s%c)", mt->SP[k], mt->SPt[k] );
	} else {
		for ( int k=0; k< mt->nSP; k++)
			msgfile( out, " (%s)", mt->SP[k] );
	}
	if (mt->translation) msgfile( out, "=%s", mt->translation);
}

void print_ambmortag_2( FILE* out, ambmortag* amt )
{
	for ( int j = 0; j < amt->nA ; j++ ) {
		msgfile( out, "{%d} ", j+1 );
		print_mortag( out, &amt->A[j] );
		if (j!=amt->nA-1) msgfile( out, " " );
	}
}

// print functions
void print_mortag( FILE* out, mortag* mt )
{
	msgfile( out, "%s", mortag_to_string(mt) );
}

void print_ambmortag( FILE* out, ambmortag* amt )
{
	int  j;

	msgfile( out, "(%d ", amt->nA );
	for ( j = 0; j < amt->nA ; j++ ) {
		msgfile( out, "{%d} ", j+1 );
		print_mortag( out, &amt->A[j] );
		if (j!=amt->nA-1)
			msgfile( out, " " );
	}
	msgfile( out, " )" );
}

int is_unknown_word( mortag* mt )
{
	return ( !_stricmp(mt->MT,"?") && mt->GR && _stricmp(mt->GR,"?") ) ? 1 : 0 ;
}

int is_pct( mortag* mt )
{
	// look if tag name is ok.
	#define allpcts "~#{[|`\\^@]}=+)_-('\"&<>,;:!?./ßµ%£$"

	if ( strspn( mt->MT, allpcts ) != strlen(mt->MT) ) return 0;
	
	if ( !mt->GR ) return 1;
	
	// there is a generic root, must be OK also.
	if ( strspn( mt->GR, allpcts ) != strlen(mt->GR) ) return 0;

	return 1;
}

int is_in( mortag* a, mortag* b ) // is b in a (like strstr, strchr, ...
{
	int i, j;
	if ( _stricmp( a->MT, b->MT ) ) return 0;
	for (i=0; i<b->nSP; i++ ) {
		for (j=0; j<a->nSP ; j++ ) {
			if ( !_stricmp( b->SP[i], a->SP[j] ) )
				break;
		}
		if ( j==a->nSP ) return 0;
	}
	return 1;
}

int is_in_plus_gr( mortag* a, mortag* b ) // is b in a (like strstr, strchr, ...
{
	int i, j;
	if ( _stricmp( a->MT, b->MT ) ) return 0;
	for (i=0; i<b->nSP; i++ ) {
		for (j=0; j<a->nSP ; j++ ) {
			if ( !_stricmp( b->SP[i], a->SP[j] ) )
				break;
		}
		if ( j==a->nSP ) {
			if ( a->GR == 0 || _stricmp( b->SP[i], a->GR ) )
				return 0;
		}
	}
	return 1;
}

int is_eq( mortag* a, mortag* b ) // is b in a (like strstr, strchr, ...
{
	if ( _stricmp( a->MT, b->MT ) ) return 0;
	if ( a->nSP != b->nSP ) return 0;
	if ( a->GR || b->GR ) {
		if ( a->GR && !b->GR ) return 0;
		if ( !a->GR && b->GR ) return 0;
		if ( _stricmp( a->GR, b->GR ) ) return 0;
	}
	for ( int i=0; i<b->nSP; i++ ) {
			if ( _stricmp( b->SP[i], a->SP[i] ) )
				return 0;
	}
	return 1;
}

int is_eq_no_GR( mortag* a, mortag* b ) // is b in a (like strstr, strchr, ...
{
	if ( _stricmp( a->MT, b->MT ) ) return 0;
	if ( a->nSP != b->nSP ) return 0;
	for ( int i=0; i<b->nSP; i++ ) {
			if ( _stricmp( b->SP[i], a->SP[i] ) )
				return 0;
	}
	return 1;
}

int where_in_amb( ambmortag* a, mortag* b )
{
	int i;

	for (i=0; i<a->nA ; i++ )
		if ( is_eq_no_GR( &(a->A[i]), b ) )
			return i;
	return -1;
}

int is_in_amb( ambmortag* a, mortag* b )
{
	int i, ni;

	for (i=0, ni=0; i<a->nA ; i++ )
		if ( is_eq( &(a->A[i]), b ) )
			ni ++;
	return ni;
}

int filter_and_pack_ambmortag( ambmortag* a )
{
	int i,w;
	for ( i=0; i<a->nA ; i++ ) {
		if ( strncmp( a->A[i].MT, MultiCategory, strlen(MultiCategory) ) )
			filter_mortag( &a->A[i] );
		else {
			a->A[i].GR=0;
			a->A[i].nSP=0;
		}
	}
	qsort( a->A, a->nA, sizeof(mortag), (int ( * )(const void *elem1, const void *elem2 ))mortags_cmp );
	for ( i=1,w=0; i<a->nA ; i++ ) {
		if ( mortags_cmp( &a->A[i],&a->A[w]) != 0 ) {
			w++;
			if (w!=i)
				a->A[w] = a->A[i];
		}
	}
	a->nA = w+1;
	return a->nA;
}

int filter_and_pack_ambmortag_keep_mainword( ambmortag* a )
{
	int i,w;
	for ( i=0; i<a->nA ; i++ )
		filter_mortag_keep_mainword( &a->A[i] );
	qsort( a->A, a->nA, sizeof(mortag), (int ( * )(const void *elem1, const void *elem2 ))mortags_cmp );
	for ( i=1,w=0; i<a->nA ; i++ ) {
		if ( mortags_cmp( &a->A[i],&a->A[w]) != 0 ) {
			w++;
			if (w!=i)
				a->A[w] = a->A[i];
		}
	}
	a->nA = w+1;
	return a->nA;
}

int prepare_ambtagword( ambmortag* a, mortag* m, char* result )
{
	filter_and_pack_ambmortag_keep_mainword(a);
	int w = where_in_amb(a,m);
	if (w == -1) return -1;
	char* s = mortag_to_string( &a->A[0] );
//	strcpy( result, "@@" );
	strcpy( result, s );
	for (int i=1; i<a->nA; i++) {
		s = mortag_to_string( &a->A[i] );
		strcat( result, "^" );
		strcat( result, s );
	}
	return w;
}

void prepare_ambtagword( ambmortag* a, char* result )
{
	filter_and_pack_ambmortag_keep_mainword(a);
	char* s = mortag_to_string( &a->A[0] );
//	strcpy( result, "@@" );
	strcpy( result, s );
	for (int i=1; i<a->nA; i++) {
		s = mortag_to_string( &a->A[i] );
		strcat( result, "^" );
		strcat( result, s );
	}
}

// for the two functions, the first argument is handled in the same way than the first argument of strtok.
// in the first call, it is the full content of the string to be decomposed.
// during subsequent calls, if it is NULL, then the previous string is again decomposed.

// the second argument element gets the result of the decomposition (element by element, one for each call to the function)

// the two functions return NULL when they arrive to end of the string to be decomposed 

int strsplitwithsep(char* str, int start, char* element, char* sep, char* actsep, int maxelement );
// the sep argument contains the separators to be used.
// the actsep argument will receive the actual separator (occuring before the element).

int strsplitwithsep(char* MAIN, int start, char* element, char* sep, char* actsep, int maxelement )
{
	int s, i;
	if ( start < 0 ) return -2 ; // error call

	// first get the separator

	for (s = 0; MAIN[start+s] ; s++ ) {
		if ( !strchr( sep, MAIN[start+s] ) ) break;
	}
	if (s==0)
		actsep[0] = '\0';
	else {
		strncpy( actsep, MAIN+start, s );
		actsep[s] = '\0';
	}

	if ( !MAIN[start+s] ) // end of the string
		return -1;

	// then get the word

	for (i = 1; MAIN[start+i+s] ; i++ ) {
		if ( strchr( sep, MAIN[start+i+s] ) ) break;
	}
	if (i >= maxelement) {
		strncpy( element, MAIN+start+s, maxelement-1 );
		element[maxelement-1] = '\0';
	} else {
		strncpy( element, MAIN+start+s, i );
		element[i] = '\0';
	}
	return start+s+i;
}

int strsplitsepnosep(char* MAIN, int start, char* element, const char* sepskip, const char* sepskipnot, int maxelement )
{
	int s, i;
	if ( start < 0 ) return -2 ; // error call

	// first get the separator : could only be a "sepskip"

	for (s = 0; MAIN[start+s] ; s++ ) {
		if ( !strchrConst( sepskip, MAIN[start+s] ) ) break;
	}

	if ( !MAIN[start+s] ) // end of the string
		return -1;

	// then get the word : could be composed either of elements of sepskipnot, or other (but not sepskip, of course)

	if ( strchrConst( sepskipnot, MAIN[start+s] ) ) { // make element from characters in "sepskipnot"
		for (i=1; MAIN[start+i+s] ; i++ ) {
			if ( !strchrConst( sepskipnot, MAIN[start+i+s] ) ) break;
		}
		strncpy( element, MAIN+start+s, i );
		element[i] = '\0';
		return start+s+i;
	}

	for (i=1; MAIN[start+i+s] ; i++ ) {
		if ( strchrConst( sepskip, MAIN[start+i+s] ) || strchrConst( sepskipnot, MAIN[start+i+s] ) ) break;
	}
	if (i >= maxelement) {
		strncpy( element, MAIN+start+s, maxelement-1 );
		element[maxelement-1] = '\0';
	} else {
		strncpy( element, MAIN+start+s, i );
		element[i] = '\0';
	}

	return start+s+i;
}

/**** STRIP OFF ALL ELEMENTS THAT ARE NOT PASSED ON TO THE %MOR: LINE ***/

/*

+" -> rien
+". -> .
+"/. -> .
++ -> rien
+, -> rien
+... -> .
+...[+ -> .
+/. -> .
+/.[+ -> .
+//. -> .
+//.> -> .
+//.[+ -> .
+//? -> .
+/? -> .

[/]
[//]
[///]

*/

static void stripchatcode(char* s, char* a)
{
	a[0] = '\0';
	int na = 0;
	int cp = 1;
	for ( int i = 0 ; i < strlen(s) ; i++ ) {
		char x = s[i];
		if ( x == '<' || x == '>' ||/* x == ',' ||*/ x == ';' )
			continue;
		if ( x == '#' || x == '@' ) {
			while ( s[i] && !strchrConst(" \t\r\n", s[i]) ) i++;
			continue;
		}
		if ( x == '[' )  {
			cp --;
			continue;
		}
		if ( x == ']' )  {
			cp ++;
			continue;
		}
		if ( cp<1 )
			continue;

//		if ( x == '-' || x] == '\'' ) x = ' ';

		if ( strchrConst (".?!", x ) ) {
			a[na++] = ' ';
			a[na++] = x;
		} else
			a[na++] = x;

//		if ( strchr (".?!", x ) ) continue;	// strip silent code
//		a[na++] = x;
	}
	a[na++] = '\0';
}

int split_original_line( int nMorW, char* OrigLine, char** OrigW, char* buffer, int maxlen )
{
// #define DBGsol
#ifdef DBGsol
msg("1] %s\n", OrigLine);
#endif

	// FIRST SPLIT TO PROCESS INFORMATIONS ABOUT [] <> +
	char temporary[MaxNbWords];
	// add ' ' before [ and < and after ] and >
	int dest = 0, i;
	for ( i = 0 ; OrigLine[i] != '\0' ; i++ ) {
		if ( OrigLine[i] == '[' || OrigLine[i] == '<' ) {
			temporary[dest++] = ' ';
			temporary[dest++] = OrigLine[i];
		} else if ( OrigLine[i] == ']' || OrigLine[i] == '>' ) {
			temporary[dest++] = OrigLine[i];
			temporary[dest++] = ' ';
		} else {
			temporary[dest++] = OrigLine[i];
		}
	}
	temporary[dest] = '\0';
	
	int ne = split_with(temporary, OrigW, " \t\n\r", MaxNbWords );

	for ( i=1; i<ne; i++ ) {
		if ( OrigW[i][0] == '+' || OrigW[i][0] == '0' ) {
			if ( strchr( OrigW[i], '.' ) )
				strcpy( OrigW[i], "." );
			else
				OrigW[i][0] = '\0';
		} else if ( !strcmp( OrigW[i], "[///]" ) ) {
			for (int k=1; k<=i; k++ ) {
				OrigW[k][0] = '\0';
			}
		} else if ( !strcmp( OrigW[i], "[/]" ) || !strcmp( OrigW[i], "[//]" ) ) {
			if (i>1 && OrigW[i-1][strlen(OrigW[i-1])-1] == '>') {
				int w = i-1;
				do {
					if ( strchr( OrigW[w], '<' ) ) break;
					w--;
				} while (w>1);
				for (int k=w; k<=i; k++ )
					OrigW[k][0] = '\0';
			} else {
				if (i>1) OrigW[i-1][0] = '\0';
				OrigW[i][0] = '\0';
			}
		}
	}

	// MAKES A NEW COPY OF LINE
	char temporary2[MaxNbWords];
	temporary2[0] = '\0';
	for (i=0; i<ne; i++) {
		if ( OrigW[i][0] != '\0' ) {
			strcat( temporary2, OrigW[i] );
			strcat( temporary2, " " );
		}
	}
	// cleans all other []<>
	stripchatcode( temporary2, temporary );

#ifdef DBGsol
msg("2] %s\n", temporary2);
msg("2] %s\n", temporary);
#endif

	// NOW SPLITS THE LINE
	// there are two ways to split the line: with ' or without '
	int nOrig = split_with_a( temporary, OrigW, buffer, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, maxlen );
	for (i=0; i<nOrig; i++) {
		if ( strchrConst( non_internal_pcts, OrigW[i][0] ) ) {
			for ( int j=i; j<nOrig-1; j++ )
				OrigW[j] = OrigW[j+1];
			nOrig --;
		}
	}
	if (nOrig != nMorW) {
		nOrig = split_with_a( temporary, OrigW, buffer, WhiteSpaces, PunctuationsAtEnd, "'", MaxNbWords, maxlen );
		for (i=0; i<nOrig; i++) {
			if ( strchrConst( non_internal_pcts, OrigW[i][0] ) ) {
				for ( int j=i; j<nOrig-1; j++ )
					OrigW[j] = OrigW[j+1];
				nOrig --;
			}
		}
	}

#ifdef DBGsol
msg("3]");
if (nOrig == nMorW) msg( "DÈcoupe OK.\n" ); else msg( "Erreur DÈcoupe.\n" );
for (i=0; i<nOrig; i++)
	msg( " %s", OrigW[i] );
msg("\n");
#endif
	return nOrig;
}
