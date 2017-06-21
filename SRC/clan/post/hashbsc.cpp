/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* hashbsc.cpp
	- basic hash functions
*/

#include "system.hpp"

#include "hashbsc.hpp"

#define sz_cHashHeader 16
#define sz_cCellBB sizeof(long)*3

/*
 * Add an item to an hashfile system.
 */

void cHashBasic::UpdateHeader()
{
	// Write Header element by element.
	// OLD *** vmWrite( &mHeader, mVirtualAddress, sz_cHashHeader );
	vmWriteLong( mHeader.mVersionNumber, mVirtualAddress );		/* version number */
	vmWriteLong( mHeader.mSizeBackbone, mVirtualAddress + (long)(sizeof(long)) );	/* number of elements in direct access */
	vmWriteLong( mHeader.mNumberOfItems, mVirtualAddress + (long)(2*sizeof(long)) );/* number of items of hash */
	vmWriteLong( mHeader.mNumberOfCrashes, mVirtualAddress + (long)(3*sizeof(long)) );/* number of hits of hash */
}

address cHashBasic::Create( long sizeBB )
{
	mVirtualAddress = vmAllocate( sz_cHashHeader + sz_cCellBB * sizeBB );

	mHeader.mVersionNumber = HASHBSC_VERSION_NUMBER;
	mHeader.mSizeBackbone = sizeBB;
	mHeader.mNumberOfItems = 0L;
	mHeader.mNumberOfCrashes = 0L;
	UpdateHeader();

	cCellBB bb;
	bb.data = NOCELL;
	bb.next = NOCELL;
	bb.size = 0;
	for ( long n=0l; n<sizeBB; n++ ) {
// if (n%100000==0) msg( ".%ld",n/1000);
		// Write all element of structure
		// OLD *** vmWrite( &bb, mVirtualAddress + sz_cHashHeader + n * sz_cCellBB, sz_cCellBB );
		vmWriteLong( bb.data, mVirtualAddress + sz_cHashHeader + n * sz_cCellBB );
		vmWriteLong( bb.next, mVirtualAddress + sz_cHashHeader + n * sz_cCellBB + sizeof(long) );
		vmWriteLong( bb.size, mVirtualAddress + sz_cHashHeader + n * sz_cCellBB + 2 * sizeof(long) );
	}
	return mVirtualAddress;
}

int cHashBasic::Open( address vmadr )
{
	mVirtualAddress = vmadr;
	// OLD *** vmRead( &mHeader, mVirtualAddress, sz_cHashHeader );
	mHeader.mVersionNumber = vmReadLong( mVirtualAddress );		/* version number */
	mHeader.mSizeBackbone = vmReadLong( mVirtualAddress + (long)(sizeof(long)) );	/* number of elements in direct access */
	mHeader.mNumberOfItems = vmReadLong( mVirtualAddress + (long)(2*sizeof(long)) );/* number of items of hash */
	mHeader.mNumberOfCrashes = vmReadLong( mVirtualAddress + (long)(3*sizeof(long)) );/* number of hits of hash */

	if ( mHeader.mVersionNumber < HASHBSC_VERSION_NUMBER ) {
		msg( "The version of the POST database file is older than the version of the CLAN software.\nPlease upgrade your POST database file.\n" );
		return 0;
	} else if ( mHeader.mVersionNumber > HASHBSC_VERSION_NUMBER ) {
		msg( "The version of the CLAN software is older than the version of the POST database file.\nPlease upgrade your CLAN software\n" );
		return 0;
	}
	return 1;
}

address cHashBasic::Add( unsigned char* element, int sizeofelement )
{
  cCellBB cl;
  address pt;

	pt = (*mComputeHashkey)( element, mHeader.mSizeBackbone ) ;
	pt = pt*sz_cCellBB + sz_cHashHeader + mVirtualAddress;
	for (;;) {
		// OLD *** vmRead( &cl, pt, sz_cCellBB );
		cl.data = vmReadLong( pt );
		cl.next = vmReadLong( pt + sizeof(long) );
		cl.size = vmReadLong( pt + 2 * sizeof(long) );
		if (cl.next==NOCELL) { /* Entry was empty */
			cl.next= ENDCELL;
			cl.data= vmAllocate(sizeofelement);
			cl.size= sizeofelement;
			// OLD *** vmWrite( element, cl.data, sizeofelement );
			(*mWriteElement)( element, cl.data, sizeofelement );
			// OLD *** vmWrite( &cl, pt, sz_cCellBB );
			vmWriteLong( cl.data, pt );
			vmWriteLong( cl.next, pt + sizeof(long) );
			vmWriteLong( cl.size, pt + 2 * sizeof(long) );

			mHeader.mNumberOfItems++;
			UpdateHeader();
			return cl.data;		// virtual address
		}

		// OLD *** vmRead( mPad, cl.data, cl.size /*MaxHashKeySize*/ );
		(*mReadElement)( mPad, cl.data, cl.size /*MaxHashKeySize*/ );

		if ( (*mCompareElements)( element, mPad ) ) {
			/* Item is here, return it's location */
			return cl.data;		// virtual address
		}

		if (cl.next==ENDCELL) {
			/* modify current cell in order to point further */
			cl.next = vmAllocate(sz_cCellBB);
			// OLD *** vmWrite( &cl, pt, sz_cCellBB );
			// vmWriteLong( cl.data, pt );
			vmWriteLong( cl.next, pt + sizeof(long) );
			// vmWriteLong( cl.size, pt + 2 * sizeof(long) );
			pt = cl.next;  /* address of next cell */

			/* generate next cell */
			cl.next= ENDCELL;
			cl.data= vmAllocate(sizeofelement);
			cl.size= sizeofelement;
			// OLD *** vmWrite( element, cl.data, sizeofelement );
			(*mWriteElement)( element, cl.data, sizeofelement );
			// OLD *** vmWrite( &cl, pt, sz_cCellBB );
			vmWriteLong( cl.data, pt );
			vmWriteLong( cl.next, pt + sizeof(long) );
			vmWriteLong( cl.size, pt + 2 * sizeof(long) );

			mHeader.mNumberOfItems++;
			mHeader.mNumberOfCrashes++;
			UpdateHeader();
			return cl.data;		// virtual address
		}
		pt=cl.next;
	}
	return NOSPACELEFT;	// never reached
}

unsigned char* cHashBasic::Look( unsigned char* element )
{
  cCellBB cl;
  address pt;

	pt = (*mComputeHashkey)( element, mHeader.mSizeBackbone ) ;
	pt = pt*sz_cCellBB + sz_cHashHeader + mVirtualAddress;
	for (;;) {
		// OLD *** vmRead( &cl, pt, sz_cCellBB );
		cl.data = vmReadLong( pt );
		cl.next = vmReadLong( pt + sizeof(long) );
		cl.size = vmReadLong( pt + 2 * sizeof(long) );
		if (cl.next==NOCELL)
			return 0;

		/* there's a cell: read the item */
		// OLD *** vmRead( mPad, cl.data, cl.size );
		(*mReadElement)( mPad, cl.data, cl.size );
		if ( (*mCompareElements)( element, mPad ) )
			return mPad;

		if (cl.next==ENDCELL)
			return 0;
		pt=cl.next;
	}
	return 0;
}

address cHashBasic::LookAddressOf( unsigned char* element )
{
  cCellBB cl;
  address pt;

	pt = (*mComputeHashkey)( element, mHeader.mSizeBackbone ) ;
	pt = pt*sz_cCellBB + sz_cHashHeader + mVirtualAddress;
	for (;;) {
		// OLD *** vmRead( &cl, pt, sz_cCellBB );
		cl.data = vmReadLong( pt );
		cl.next = vmReadLong( pt + sizeof(long) );
		cl.size = vmReadLong( pt + 2 * sizeof(long) );
		if (cl.next==NOCELL) return NOCELL;

		/* there's a cell: read the item */
		// OLD *** vmRead( mPad, cl.data, cl.size );
		(*mReadElement)( mPad, cl.data, cl.size );
		if ( (*mCompareElements)( element, mPad ) )
			return cl.data;	// virtual address

		if (cl.next==ENDCELL) return NOCELL;
		pt=cl.next;
	}
	return 0;
}

void cHashBasic::Stat(FILE* f)
{ double coll;
	if (mHeader.mNumberOfItems!=0)
		coll=(double)mHeader.mNumberOfCrashes/(double)mHeader.mNumberOfItems*(double)100;
	else	coll=(double)0.0;
	msgfile(f,"# of items : %ld\n", ActualSize() );
	msgfile(f,"# of slots : %ld\n", mHeader.mSizeBackbone);
	msgfile(f,"# of crash : %ld (%2.2f%%)\n", mHeader.mNumberOfCrashes, coll);
}

/*
 * Gives in turn each indexed item.
 * Return:
 *	- pointer to indexed item;
 */

unsigned char* cHashBasic::Give(int firsttime)
{
	static address p,l;
	static cCellBB cl;

	if (firsttime) {
		p=(long)sz_cHashHeader+mVirtualAddress;
		l=0L;
		return 0;
	}
	if (l!=0L) goto crash;
 loop:	
	if ( p >= (address)mHeader.mSizeBackbone * (address)sz_cCellBB
		  + (address)sz_cHashHeader
		  + mVirtualAddress )
		return 0;
	// OLD *** vmRead( &cl, p, sz_cCellBB );
	cl.next = vmReadLong( p + sizeof(long) );
	if (cl.next==NOCELL) { p+=sz_cCellBB; goto loop; }
	cl.data = vmReadLong( p );
	cl.size = vmReadLong( p + 2 * sizeof(long) );
	l=cl.data;
 crash:
	// OLD *** vmRead( mPad, l, cl.size /*MaxHashKeySize*/ );
	(*mReadElement)( mPad, l, cl.size /*MaxHashKeySize*/ );
	if (cl.next==ENDCELL) { l=0L; p+=sz_cCellBB; }
	else {
		// OLD *** vmRead( &cl, cl.next, sz_cCellBB );
		cl.data = vmReadLong( cl.next );
		cl.size = vmReadLong( cl.next + 2 * sizeof(long) );
		cl.next = vmReadLong( cl.next + sizeof(long) );
		l=cl.data;
	}
	return mPad;
}

void cHashBasic::Dump( FILE* f, void (*fun)(FILE* f1, unsigned char* c) )
{
	unsigned char* p;

	Give(1);
	for (;;) {
		p = Give(0);
		if (!p) break;
		(*fun)(f,p);
	}
}
