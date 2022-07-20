/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* HashBsc.hpp
	- HASH-ACCESS -
	- Creation of a hash table in virtual memory
	- 5 may 1996
*/

#ifndef __HASHBSC_HPP__
#define __HASHBSC_HPP__

#include "storage.hpp"

const address NOCELL = -1L;
const address ENDCELL = -2L;
const address NOSPACELEFT = -3L;

const int MaxHashKeySize = 1024;

#define HASHBSC_VERSION_NUMBER (2*1024 + 0*1024*1024 + 0*1024*1024*1024)
/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

struct cHashHeader {
	long32	mVersionNumber;		/* version number */
	long32	mSizeBackbone;		/* number of elements in direct access */
	long32	mNumberOfItems;		/* number of items of hash */
	long32	mNumberOfCrashes;	/* number of hits of hash */
};

struct cCellBB {
	address data;
	address next;
	long32	size;
};

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

	/* FCE := function_compare_elements */
typedef	int (*FCE) ( unsigned char* element1, unsigned char* element2 );
	/* FCH := function_compute_hashkey */
typedef	long32 (*FCH) ( unsigned char* element, long32 hashsize );
	/* RWE := function_read/write_element */
typedef void (*RWE) ( void* adr, address vmadr, int nbbytes );

class cHashBasic {
	address	mVirtualAddress;	/* virtual address of hash begining (entete+backbone) */
	cHashHeader mHeader;		/* pointer to header */
	unsigned char mPad[MaxHashKeySize]; /* buffer for searching */
	FCE	mCompareElements;	/* comparison function between 2 elements */
	FCH	mComputeHashkey;	/* computing function of hash key for 1 element */
	RWE	mReadElement;		/* read an element (key) and reverse if necessary */
	RWE	mWriteElement;		/* write an element (key) and reverse if necessary */
	void	UpdateHeader();		/* update in virtual memory of header */
  public:
		cHashBasic () { mCompareElements=0; mComputeHashkey=0; }
		~cHashBasic () {}
virtual address	Create( long32 sizeBB, int type=0 );				/* initial creation in virtual memory */
virtual int	Open( address vmadr, int type );				/* initial opening in vitual memory */
virtual address Add( unsigned char* element, int sizeofelement );  	/* add 1 element */
virtual unsigned char* Look( unsigned char* element );			/* look for 1 element */
virtual address LookAddressOf( unsigned char* element );		/* look for 1 element */
	void	SetFCE(FCE function) { mCompareElements=function; } /* choice of comparison between elements */
	void	SetFCH(FCH function) { mComputeHashkey=function; } /* choix of hash function */
	void	SetReadRWE(RWE function) { mReadElement = function; } /* choix of read function */
	void	SetWriteRWE(RWE function) { mWriteElement = function; } /* choix of write function */
	void	Stat(FILE* f);
	unsigned char* Give(int firsttime); /* for Dump and for iteration */
	void	Dump( FILE* f, void (*fun)(FILE* f1, unsigned char* c) );
	long32	ActualSize() {return mHeader.mNumberOfItems;};
};

#endif /* __HASHBSC_HPP__ */
