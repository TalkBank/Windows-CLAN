/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* hashc.hpp
	- all these functions are defined in hashc.cpp
	  and make the interface between storage and POST more transparent
	  Internal C++ structure knowledge in not necessary anymore.

	FOR ALL THESE FUNCTIONS, INDIVIDUAL SIZE OF A HASH ELEMENT IS LIMITED 
	TO _max_of_hash_ (40000 bytes) - this limits apply only because of hashget functions.
	changing it need not to recompile database, but a program with a small value should NOT
	use a database created with a large value. Unexpected crashes could occur.

	All used memory is allocated statically in the program static space and freed at the
	end of the program (no memory allocation scheme is necessary).
	
*/

#ifndef __HASHC_HPP__
#define __HASHC_HPP__

#include "maxconst.hpp"

// WorkSpace (only one workspace at a time in the whole software)
int wkscreate (FNType* fn, int inmemory=1);
int wksopen (FNType* fn, int RW=1, int inmemory=1);
int wkssave ();
int wksclose ();

// HashFiles: 'hd' stands for 'hash descriptor'.
// type==0 ==> string of char ended by '\0', type==1 ==> binary data
int hashcreate (const char* fn, int n, int type=0 );	// initial creation. (n=size of hash)
int hashopen (const char* fn, int type=0);		// open an existing hash : return -1 of not OK
int hashprobe (const char* fn);		// test of hash exists : return yes(1) or no(0)
void hashlist();	// list all existing hashes

// deals with hash elements
// lk parameter represents the length of 'k' (key) for binary data
// length of e element is represented by first element of e. e is always an array of int
// Name of Function: 	C = string of char
//			I = array of int
//			N = single int
char* hashGetCC(int hd, const char* k);		// key is char*, returns char*
int hashPutCC(int hd, const char* k, const char* e);	// key is char*, element is char*

Int4* hashGetCI(int hd, const char* k );		// key is char*, returns Int4*
int hashPutCI(int hd, const char* k, Int4* e );	// key is char*, element is Int4*

Int4* hashGetII(int hd, Int4* k , int lk);	// key is Int4*, returns Int4*
int hashPutII(int hd, Int4* k, int lk, Int4* e );	// key is Int4*, element is Int4*

Int4 hashGetCN(int hd, const char* k );		// key is char*, return Int4
int hashPutCN(int hd, const char* k, Int4 n ); 	// key is char*, element is Int4

Int4 hashGetIN(int hd, Int4* k, int lk );		// key is Int4*, return Int4
int hashPutIN(int hd, Int4* k, int lk, Int4 n );	// key is Int4*, element is Int4

// global hash functions:
/* this structure handle the results of an iteration process (of a hashfile) */
struct loopr {
	char* s;	// key of hash: when (char*) is binary data (and not string) length can be computed later.
	Int4* e;	// element of hash.
};
int hashactualsize (int hd);		// number of elements of hash
int hashiteratorinit (int hd);		// (re)starts iteration process
loopr* hashiteratorloopCC (int hd);	// iterate for string data type
loopr* hashiteratorloopCI (int hd);	// iterate for Int4 data type
loopr* hashiteratorloopII (int hd);	// iterate for Int4 data type
loopr* hashiteratorloopCN (int hd);	// iterate for number data type
loopr* hashiteratorloopIN (int hd);	// iterate for number data type
int hashstat(int hd, FILE* out=stdout);	// statistics of hash use
long32 hashsize(int h);			// actual size of a hash

int alloc_and_write_string( char* rule );	// allocate and store a string in the database
void read_string( int adr, char* rule );	// retrieve a string stored in the database
void write_string( int adr, char* rule );	// store a string in the database (already allocated).

#endif

