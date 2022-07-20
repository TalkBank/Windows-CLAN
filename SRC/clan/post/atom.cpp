/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*
	atom.cpp
	--------
	version 0.99 - 21 octobre 1997
*/

#include "system.hpp"

#include "atom.hpp"

/*** tool functions for hash and comparison functions ***/

/*
 * Hash function for words.
 */

inline long32 funHashWord(unsigned char *key, int lkey, long32 hsize)
{
 	register int i;
  	long32 ret=1L;

	for (i=0;i<lkey;i++)
		ret += labs(ret*(long32)key[i])+i;

	ret  = labs(ret);

	ret %= hsize;

	ret  = labs(ret);

	return ret;
}


/*
 * Tests the bit <bnum> in <bstring>.
 */

inline int funBitTest(unsigned char *a, int bnum)
{
	unsigned char t;

	a += ( bnum >> 3 );
	t = *a;
	return t & (unsigned char)(0x80 >> (bnum & 7));
}

/*
 * Hash function for numbers.
 */

inline long32 funHashNumber(unsigned char *key, int lkey, long32 hsize)
{
 	register int i;
  	static long32 prem[10]={2L,3L,5L,7L,11L,13L,17L,19L,23L,29L};
  	long32 ret=1L;

	for (i=0;i<lkey;i++)
		ret += labs(ret*(long32)(key[i]+2)*prem[i%10]+(long32)i);

	ret = labs(ret);

	ret %= hsize;

	ret  = labs(ret);

/*
for ( int j = 0; j < lkey ; j++ ) {
	char s[128];
	tag_to_classname(key[j],s);
	msg( " %s", s );
}
msg (" = %ld\n", ret );
*/
	return ret;
}

static long32 FHASH_Atom( unsigned char * W, long32 hsize )
{
	Int4 wl = ((cAtom*)W)->WordLength() ;
	return funHashWord( ((cAtom*)W)->Str(), wl, hsize );
}

static int FCMP_Atom( unsigned char * W1, unsigned char * W2 )
{
	return !strcmp( (char*)((cAtom*)W1)->Str(), (char*)((cAtom*)W2)->Str() );
}

static long32 FHASH_AtomL( unsigned char * W, long32 hsize )
{
	Int4 wl = ((cAtom*)W)->WordLength() ;
	Int4* I4 = (Int4*)(((cAtom*)W)->Str()) ;
	int i;
	if (vmIsReversed()) for (i = 0; i < wl/sizeof(Int4) ; i++) {
		flipLong(&I4[i]);
	}
	long32 r = funHashNumber( ((cAtom*)W)->Str(), wl, hsize );
	if (vmIsReversed()) for (i = 0; i < wl/sizeof(Int4) ; i++) {
		flipLong(&I4[i]);
	}
	return r;
}

static int FCMP_AtomL( unsigned char * W1, unsigned char * W2 )
{
	Int4 wl1 = ((cAtom*)W1)->WordLength() ;
	Int4 wl2 = ((cAtom*)W2)->WordLength() ;
	if ( wl1 != wl2 ) return 0;
	return !memcmp( (char*)((cAtom*)W1)->Str(), (char*)((cAtom*)W2)->Str(), wl1 );
}

static void FREAD_AtomL( void* adr, address vmadr, int nbbytes )
{
	vmRead( adr, vmadr, nbbytes );
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
		Int4 *I = (Int4*)a->Str(), lI = Align4(a->WordLength())/sizeof(Int4);
		for (int i=0; i<lI; i++)
			flipLong( &(I[i]) );
	}
}

static void FWRITE_AtomL( void* adr, address vmadr, int nbbytes )
{
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		Int4 *I = (Int4*)a->Str(), lI = Align4(a->WordLength())/sizeof(Int4);
		for (int i=0; i<lI; i++)
			flipLong( &(I[i]) );
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
	}
	vmWrite( adr, vmadr, nbbytes );
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
		Int4 *I = (Int4*)a->Str(), lI = Align4(a->WordLength())/sizeof(Int4);
		for (int i=0; i<lI; i++)
			flipLong( &(I[i]) );
	}
}

static void FREAD_Atom( void* adr, address vmadr, int nbbytes )
{
	vmRead( adr, vmadr, nbbytes );
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
	}
}

static void FWRITE_Atom( void* adr, address vmadr, int nbbytes )
{
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
	}
	vmWrite( adr, vmadr, nbbytes );
	if ( vmIsReversed() ) {
		cAtom* a = (cAtom*) adr;
		flipLong( &(a->m_WordLength) );
		flipLong( &(a->m_Prop1) );
		flipLong( &(a->m_Prop2) );
		flipLong( &(a->m_Prop3) );
	}
}

address cHashAtom::Create( long32 sizeBB, int type )
{
	type_key = type;
	if (type == 1) {
		SetFCH( FHASH_AtomL );
		SetFCE( FCMP_AtomL );
		SetReadRWE( FREAD_AtomL );
		SetWriteRWE( FWRITE_AtomL );
	} else {
		SetFCH( FHASH_Atom );
		SetFCE( FCMP_Atom );
		SetReadRWE( FREAD_Atom );
		SetWriteRWE( FWRITE_Atom );
	}
	return cHashBasic::Create( sizeBB, type );
}

int cHashAtom::Open( address vmadr, int type )
{
	type_key = type;
	if (type == 1) {
		SetFCH( FHASH_AtomL );
		SetFCE( FCMP_AtomL );
		SetReadRWE( FREAD_AtomL );
		SetWriteRWE( FWRITE_AtomL );
	} else {
		SetFCH( FHASH_Atom );
		SetFCE( FCMP_Atom );
		SetReadRWE( FREAD_Atom );
		SetWriteRWE( FWRITE_Atom );
	}
	return cHashBasic::Open( vmadr, type );
}

address cHashAtom::AddI( cAtom* a )
{
	if (type_key != 1) {
		msg( "Fatal error - key incompatible with function ! (AddI)\n" );
		exit(1);
	} 

	int al = a->UsedLength() ;

	return cHashBasic::Add( (unsigned char *)a, al );
}

cAtom* cHashAtom::LookI( cAtom* a ) /* look for one element (atom) */
{
	if (type_key != 1) {
		msg( "Fatal error - key incompatible with function ! (LookI)\n" );
		exit(1);
	} 

	return (cAtom*)cHashBasic::Look( (unsigned char *) a );
}

address cHashAtom::LookI_AddressOf( cAtom* a ) /* look for one element (atom) */
{
	if (type_key != 1) {
		msg( "Fatal error - key incompatible with function ! (LookI_AddressOf)\n" );
		exit(1);
	} 

	return cHashBasic::LookAddressOf( (unsigned char *) a );
}

address cHashAtom::AddC( cAtom* a )
{
	if (type_key != 0) {
		msg( "Fatal error - key incompatible with function ! (AddC)\n" );
		exit(1);
	} 

	int al = a->UsedLength() ;

	return cHashBasic::Add( (unsigned char *)a, al );
}

cAtom* cHashAtom::LookC( cAtom* a ) /* look for one element (atom) */
{
	if (type_key != 0) {
		msg( "Fatal error - key incompatible with function ! (LookC)\n" );
		exit(1);
	} 

	return (cAtom*)cHashBasic::Look( (unsigned char *) a );
}

address cHashAtom::LookC_AddressOf( cAtom* a ) /* look for one element (atom) */
{
	if (type_key != 0) {
		msg( "Fatal error - key incompatible with function ! (LookC_AddressOf)\n" );
		exit(1);
	} 

	return cHashBasic::LookAddressOf( (unsigned char *) a );
}

long32 cAtom::NbOcc()
{
	return Property3();
}
/*
void cAtom::vmRead(address vadr, cAtom* mptr)
{
	// READ the 4 first long32 int of the Atom.
	mptr->m_WordLength = vmReadLong( vadr );
	mptr->m_Prop1 = vmReadLong( vadr + (long32)(sizeof(long32 int)) );
	mptr->m_Prop2 = vmReadLong( vadr + (long32)(2*sizeof(long32 int)) );
	mptr->m_Prop3 = vmReadLong( vadr + (long32)(3*sizeof(long32 int)) );
	// READ the name of the atom.
	::vmRead ( ((unsigned char*)mptr) + (long32)(4*sizeof(long32 int)),
		vadr + (long32)(4*sizeof(long32 int)), m_WordLength );
}
*/
cAtom* cHashAtom::GiveAtom(int firsttime) /* for Dump and for iteration */
{
	cAtom* a = (cAtom*)cHashBasic::Give(firsttime);
	if (firsttime) return 0;
	return a;
}

void cHashAtom::ReWriteHead( cAtom* mptr, address vadr )
{
	// WRITE the 4 first long32 int of the Atom.
	vmWriteLong( mptr->m_WordLength, vadr );
	vmWriteLong( mptr->m_Prop1, vadr + (long32)(sizeof(long32)) );
	vmWriteLong( mptr->m_Prop2, vadr + (long32)(2*sizeof(long32)) );
	vmWriteLong( mptr->m_Prop3, vadr + (long32)(3*sizeof(long32)) );
}
