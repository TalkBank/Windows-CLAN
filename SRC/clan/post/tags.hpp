/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- tags.hpp	- v1.0.0 - november 1998

	Each kind of ascii tag (label) is associated with a unique integer number.
	The equivalence between tags as a string and tags as a number is memorized in the workspace.
	The onlye kind of tag that is manipulated is integer tag, so array of tags have unique size elements 
	and space is saved in the workspace database.
	
	Tags comes in several kinds (3 main types + bi-tags and tri-tags).

	Tag ASCII name : CLASSNAME[] = an array of char that contains printable names.
	Tag Integer equiv. : TAG = an integer that correspond to an unique printable name.
			(these are smaller and quicker to handle if data files).
	Ambiguous tag : AMBTAG = TAG* = an array of integer where the first number contains the number of
				different tags in the ambiguous tag.

	Bi-Tags (simple) : couple of TAG (grouped into one number) : couple of UNambiguous tags.
	Tri-Tags (simple) : triplet of TAG (grouped into one number) : triplet of UNambiguous tags.

	Bi-Ambiguous-Tag : couple of Ambiguous Tag : one array of integer composed of two 
			arrays AMBTAG one after each other (no separator is necessary because the 
			size of the 2 AMBTAG are known and set).

	Multiple Tag : (multclass)this type is specific to WORD dictionary handling (not used for POST-MOR interface).
		This a multiple tag where each element is accompagnied with its number of occurences.
		The forms is an array of integer in the following form:
		[ <size of the array> <total number of tags> <number of tag-i> <name of tag-i> ... ]
|=========================================================================================================================*/

#ifndef __TAGS_HPP__
#define __TAGS_HPP__

/* general types */
typedef char CLASSNAME ;
typedef Int4  TAG ;
typedef TAG* AMBTAG ;

// tags names handling - hashtable creation and handling.
// Storage of classname equ. to TAG (manipulate classnames).

int  classname_open();		// open classnames in a workspace (to be done each time the workspace is openned)
int  classname_init();			// Initialize classnames in a workspace (one time only)
void classname_save();			// save classnames to workspace
void list_classnames(FILE* out);	// dump all classnames

CLASSNAME* addstar_to_classname(CLASSNAME* n, CLASSNAME* s);

// manipulation of multclass type
int multclass_to_ambtag(int *cn, AMBTAG d);	// conversion between multclass and ambtag
int multclass_find( int* x, int nc );	// find a TAG in a multclass
int number_of_multclass(int *cn);	// returns number of element of a multclass
int size_of_multclass(int *cn);		// returns size in int of a multclass
int weight_of_multclass(int *cn);		// returns the total number of counts of a multclass
int nth_of_multclass( int* c, int i );	// returns nth element in a mulclass
int nthcount_of_multclass( int* c, int i ); // returns the number of occurrences of the nth element of a multclass
void setnthcount_of_multclass( int* c, int i, int v ); // sets the number of occurrences of the nth element of a multclass

// change CLASSNAME to TAG.
TAG  classname_to_tag(const CLASSNAME* c);				// conversion of a CLASSNAME in a TAG
int tag_to_classname(TAG t, char* r);				// conversion of a TAG in a CLASSNAME
TAG  classnames_to_bitag(CLASSNAME* cl1, CLASSNAME* cl2);	// conversion of two classnames in a bitag

// manipulation of ambtags.
int number_of_ambtag(AMBTAG amb);	// number of tags of an AMBTAG
TAG nth_of_ambtag( AMBTAG c, int i );	// nth tag of an ambtag
void print_ambtag(AMBTAG a, FILE* out);	// print an ambtag
void print_ambtag_regular(AMBTAG a, FILE* out, int d = 0);	// another format for printing of ambtag

// manipulation of bitags.
TAG tags_to_bitag(TAG t1, TAG t2); // makes a bitag from two tags

TAG first_of_bitag(TAG bi); // return first element in a bitag
TAG second_of_bitag(TAG bi); // return second element in a bitag
int multclasses_to_biambtag(int* a1, int* a2, AMBTAG res);	// makes an ambiguous-biclass from two ambiguous-classes
void print_multclass(int *bm, FILE* out );	// print a multclass
void print_bimultclass(int *bm, FILE* out);	// print a bimultclass (a couple of multclass)
void print_matrixelement( TAG *me, FILE* out );	// print matrix element

// tools
void	isort(int* ob, int ta);	// integer array sort.

// string decomposition.
int strsplitwithsep(char* str, int start, char* element, char* sep, char* actsep, int maxelement );
int strsplitsepnosep(char* str, int start, char* element, const char* sepskip, const char* sepskipnot, int maxelement );
void stripof(char* s, char* a);

// utilities for spliting CHAT main lines
#define MAXSIZECATEGORY 1024
#define MAXSIZEWORD 256
int split_all_formats(char* line, char** words, char* result, const char* sepsskip, const char* sepsbegin, const char* sepsend, const char* sepskeep, int maxofwords, int maxofresult );
int split_all_formats_CHAT_special(char* line, char** words, char* result, const char* sepsskip, const char* sepsbegin, const char* sepsend, const char* sepskeep, int maxofwords, int maxofresult );

int test_special_plus( char* p_line );	// tests for all special forms begining with +.
int is_chat_punctuation( char *s);	// tests if a word is a chat punctuation.
int is_bullet(char *s); // test if it is a bullet.
int is_empty(char *s);	// test if empry string.
int test_special_repet( char* p_line ); // tests for all special forms for repetition

#endif
