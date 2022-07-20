/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- hashc.cpp	- v1.0.0 - november 1998
|=========================================================================================================================*/

#include "system.hpp"

#include "atom.hpp"
#include "workspace.hpp"
#include "hashc.hpp"

/*
	- the following functions deal with HASHFILES
*/

/* many descriptors because file are not closed till the end of the program */
#define NBDESCRIPTORS 1024

static cHashAtom *descripteurs_h = 0;
static int n_descripteurs_h = NBDESCRIPTORS;

static char *s_mshi = 0 ;

// static int* hashcgetRaw(int h, char* k, int lk);
// static int  hashcputRaw(int hd, char* k, char* e, int le, int lk=-1); // put the element 'e' (array of char of length le) with key 'k'

/*
	- the 4 following functions deal with WORKSPACE
*/
static cWorkSpace WKS;
int wkscreate (FNType* s, int inmemory)
{
	if ( !descripteurs_h ) {
		s_mshi = new char [maxSizeOfHashInternal+1024];
		descripteurs_h = new cHashAtom [ NBDESCRIPTORS ];
		if ( !descripteurs_h ) {
			msg( "fatal error: no more core memory in file hashc.cpp\n");
			exit( -2 );
		}
	}
	if (inmemory==0)
		WKS.Parameters( 0, 500000L ); // database in file, not in memory
	return WKS.Create(s);
}

int wksopen (FNType* s, int RW, int inmemory)
{
	if ( !descripteurs_h ) {
		s_mshi = new char [maxSizeOfHashInternal+1024];
		descripteurs_h = new cHashAtom [ NBDESCRIPTORS ];
		if ( !descripteurs_h ) {
			msg( "fatal error: no more core memory in file hashc.cpp\n");
			exit( -2 );
		}
	}
	if (inmemory==0)
		WKS.Parameters( 0, 500000L ); // database in file, not in memory
	return WKS.Open(s, RW);
}

int wkssave ()
{
	WKS.Save();
	return 1;
}

int wksclose ()
{
	WKS.Close();
	if (descripteurs_h != 0)
		delete [] descripteurs_h ;
	descripteurs_h = 0;
	if (s_mshi != 0)
		delete [] s_mshi;
	s_mshi = 0;
	n_descripteurs_h = NBDESCRIPTORS; // lxs1234
	return 1;
}

static int lire_hashvalueof( address a, char* v, int szmax ) // read raw data
{
	do {
		if (szmax <= 32) return 0;
		vmRead( v, a, 32 );
		szmax -= 28;
		a = *((address*)(v+28));
		if ( vmIsReversed() ) {
			flipLong( &a );
		}
		v += 28;
	} while (a != (address)0);
	return 1;
}

static address ecrire_hashvalueof( address a, const char* v, int t ) // write raw data
{
	address va;
	if (a!=0) {
//msg("a!=0 a[%d] a+28[%d] t[%d]\n", a, *((address*)(v+28)), t );
		if (t<=28) {
			vmWrite( v, a, t );
			return a;
		} else {
			vmWrite( v, a, 28 );
			// OLD *** vmRead( &va, a+28, sizeof(address) );
			va = vmReadLong( a+28 );
			if (va==(address)0) {
				va = ecrire_hashvalueof(va,v+28,t-28);
				// OLD *** vmWrite( &va, a+28, sizeof(address) );
				vmWriteLong( va, a+28 );
			} else
				ecrire_hashvalueof(va,v+28,t-28);
			return a;
		}
	} else {
//fprintf(stderr,"a==0 a[%d] t[%d]\n", a, t );
		a = vmAllocate(32);
		if (t<=28) {
			vmWrite( v, a, t );
			va = (address)0;
			// OLD *** vmWrite( &va, a+28, sizeof(address) );
			vmWriteLong( va, a+28 );
			return a;
		} else {
			vmWrite( v, a, 28 );
			va = ecrire_hashvalueof(0,v+28,t-28);
			// OLD *** vmWrite( &va, a+28, sizeof(address) );
			vmWriteLong( va, a+28 );
			return a;
		}
	}
}

static char* hashcgetRawC(int h, const char* k) // key is char* , data is raw
{
	cAtom a, *p=0;

	a.Set((unsigned char*)k);
	p = descripteurs_h[h].LookC(&a);
	if (!p)
		return (char*)0;
	else {
		if ( lire_hashvalueof( p->Property1(), s_mshi, maxSizeOfHashInternal ) )
			return (char*)(s_mshi);
		else
			return (char*)0;
	}
}

static char* hashcgetRawI(int h, Int4* k, int lk) // key is int* , data is raw
{
	cAtom a, *p=0;

	a.Set((unsigned char*)k, lk);
	p = descripteurs_h[h].LookI(&a);
	if (!p)
		return (char*)0;
	else {
		if ( lire_hashvalueof( p->Property1(), s_mshi, maxSizeOfHashInternal ) )
			return (char*)(s_mshi);
		else
			return (char*)0;
	}
}

static int hashcputRawC(int h, const char* k, const char* e, int le) // raw data storage - key is char* - byte reversal is dealt with in Look()
{
	if (le>maxSizeOfHashInternal) return 0;

	cAtom a, *p=0;

	a.Set((unsigned char*)k);
	p = descripteurs_h[h].LookC(&a);
	if (!p) {
		a.SetProperty1(ecrire_hashvalueof( 0, e, le ));
		a.SetProperty2(0);
		a.SetProperty3(0);
		descripteurs_h[h].AddC(&a);
	} else
		ecrire_hashvalueof( p->Property1(), e, le );
	return 1;
}

static int hashcputRawI(int h, Int4* k, int lk, char* e, int le) // raw data storage - key is int* - byte reversal is dealt with in Look()
{
	if (le>maxSizeOfHashInternal) return 0;

	cAtom a, *p=0;

	a.Set((unsigned char*)k, lk);
	p = descripteurs_h[h].LookI(&a);
	if (!p) {
		a.SetProperty1(ecrire_hashvalueof( 0, e, le ));
		a.SetProperty2(0);
		a.SetProperty3(0);
		descripteurs_h[h].AddI(&a);
	} else
		ecrire_hashvalueof( p->Property1(), e, le );
	return 1;
}

char* hashGetCC(int h, const char* k)
{
	return hashcgetRawC(h, k);
}

Int4* hashGetCI(int h, const char* k)
{
	Int4* e = (Int4*)hashcgetRawC(h, k);

	if ( e && vmIsReversed() ) {
		flipLong( &(e[0]) );
		for (int i=1; i<e[0]; i++)
			flipLong( &(e[i]) );
	}

	return e;
}

Int4* hashGetII(int h, Int4* k, int lk)
{
	Int4* e = (Int4*)hashcgetRawI(h, k, lk);

	if ( e && vmIsReversed() ) {
		flipLong( &(e[0]) );
		for (int i=1; i<e[0]; i++)
			flipLong( &(e[i]) );
	}

	return e;
}

int hashPutCC(int h, const char* k, const char* e)
{
	int le = strlen(e)+1;
	return hashcputRawC(h, k, e, le);
}

int hashPutCI(int h, const char* k, Int4* e)
{
	int le = (e)[0]*sizeof(Int4);
	if (le>=((maxSizeOfHashInternal/sizeof(int))-10)) {
		msg( "**************************************************\n");
		msg( "***  BEWARE : SIZE >= %d   *************\n", (maxSizeOfHashInternal/sizeof(int))-10 );
		msg( "**************************************************\n");
		exit(0);
	}

	if ( vmIsReversed() ) {
		for (int i=1; i<e[0]; i++)
			flipLong( &(e[i]) );
		flipLong( &(e[0]) );
	}

	return hashcputRawC(h, k, (char*)e, le);
}

int hashPutII(int h, Int4* k, int lk, Int4* e)
{
	int le = (e)[0]*sizeof(Int4);
	if (le>=((maxSizeOfHashInternal/sizeof(int))-10)) {
		msg( "**************************************************\n");
		msg( "***  BEWARE : SIZE >= %d   *************\n", (maxSizeOfHashInternal/sizeof(int))-10 );
		msg( "**************************************************\n");
		exit(0);
	}

	if ( vmIsReversed() ) {
		for (int i=1; i<e[0]; i++)
			flipLong( &(e[i]) );
		flipLong( &(e[0]) );
	}

	return hashcputRawI(h, k, lk, (char*)e, le);
}

Int4 hashGetCN(int h, const char* k)
{
	cAtom a, *p=0;

	a.Set((unsigned char*)k);
	p = descripteurs_h[h].LookC(&a);
	if (!p)
		return (Int4)-1;
	else {
		return (Int4)p->Property1();
	}
}

int hashPutCN(int h, const char* k, Int4 nl)
{
	cAtom a;

	a.Set((unsigned char*)k);
	address p = descripteurs_h[h].LookC_AddressOf(&a);
	if (p==NOCELL) {
		a.SetProperty1(nl);
		a.SetProperty2(0);
		a.SetProperty3(0);
		descripteurs_h[h].AddC(&a);
	} else {
		a.SetProperty1(nl);
		descripteurs_h[h].ReWriteHead(&a, p); // vmWrite(&a, p, a.UsedLength());
	}
	return 1;
}

Int4 hashGetIN(int h, Int4* k, int lk)
{
	cAtom a, *p=0;

	a.Set((unsigned char*)k,lk);
	p = descripteurs_h[h].LookI(&a);
	if (!p)
		return (int)-1;
	else {
		return (Int4)p->Property1();
	}
}

int hashPutIN(int h, Int4* k, int lk, Int4 nl)
{
	cAtom a;

	a.Set((unsigned char*)k,lk);
	address p = descripteurs_h[h].LookI_AddressOf(&a);
	if (p==NOCELL) {
		a.SetProperty1(nl);
		a.SetProperty2(0);
		a.SetProperty3(0);
		descripteurs_h[h].AddI(&a);
	} else {
		a.SetProperty1(nl);
		descripteurs_h[h].ReWriteHead(&a, p); // vmWrite(&a, p, a.UsedLength());
	}
	return 1;
}

/*** local functions to set up hash coding ***/

int hashcreate(const char* fn, int s, int type)
{
	if (n_descripteurs_h<=0)
		return -1;
	n_descripteurs_h--;
	address va = descripteurs_h[n_descripteurs_h].Create((long32)s,type);
	WKS.CreateNS( fn, va );
	return n_descripteurs_h;
}

int hashopen (const char* fn, int type)
{
	if (n_descripteurs_h<=0)
		return -1;
	n_descripteurs_h--;
	address va = WKS.OpenNS( fn );
	if (va==(address)0) return -1;
	if ( !descripteurs_h[n_descripteurs_h].Open( va, type )) return -1;
	return n_descripteurs_h;
}

int hashprobe (const char* fn)
{
	if (WKS.OpenNS(fn)!=(address)0) return 1; else return 0;
}

void hashlist()
{
	WKS.ListNS();
}

int hashactualsize (int h)
{
	return (int)descripteurs_h[h].ActualSize();
	return 1;
}

int hashiteratorinit (int h)
{
	descripteurs_h[h].GiveAtom(1);
	return 1;
}

loopr* hashiteratorloopCC (int h)
{
	static loopr L;
	static char *s_mshl = 0 ;
	if (!s_mshl) s_mshl = new char [maxSizeOfHashInternal+1024];
	cAtom *a = descripteurs_h[h].GiveAtom(0);
	if (!a) {
		delete [] s_mshl;
		s_mshl = 0;
		return (loopr*)0;
	}
	L.s = s_mshl;
	memcpy( L.s, a->Str(), a->WordLength() );
	L.e = (Int4*)(s_mshl+a->UsedLength()+1);
	if ( lire_hashvalueof( a->Property1(), (char*)L.e, maxSizeOfHashInternal-(a->UsedLength()+1) ) )
		return &L;
	else
		return (loopr*)0;
}

loopr* hashiteratorloopCI (int h)
{
	static loopr L;
	static char *s_mshl = 0 ;
	if (!s_mshl) s_mshl = new char [maxSizeOfHashInternal+1024];
	cAtom *a = descripteurs_h[h].GiveAtom(0);
	if (!a) {
		delete [] s_mshl;
		s_mshl = 0;
		return (loopr*)0;
	}
	L.s = s_mshl;
	memcpy( L.s, a->Str(), a->WordLength() );
	L.e = (Int4*)(s_mshl+a->UsedLength()+1);
	if ( lire_hashvalueof( a->Property1(), (char*)L.e, maxSizeOfHashInternal-(a->UsedLength()+1) ) ) {
		if ( vmIsReversed() ) {
			flipLong( &(L.e[0]) );
			int lI = L.e[0];
			for (int i=1; i<lI; i++)
				flipLong( &(L.e[i]) );
		}
		return &L;
	} else
		return (loopr*)0;
}

loopr* hashiteratorloopII (int h)
{
	static loopr L;
	static char *s_mshl = 0 ;
	if (!s_mshl) s_mshl = new char [maxSizeOfHashInternal+1024];
	cAtom *a = descripteurs_h[h].GiveAtom(0);
	if (!a) {
		delete [] s_mshl;
		s_mshl = 0;
		return (loopr*)0;
	}
	L.s = s_mshl;
	memcpy( L.s, a->Str(), a->WordLength() );

	L.e = (Int4*)(s_mshl+a->UsedLength()+1);
	if ( lire_hashvalueof( a->Property1(), (char*)L.e, maxSizeOfHashInternal-(a->UsedLength()+1) ) ) {
		if ( vmIsReversed() ) {
			flipLong( &(L.e[0]) );
			int lI = L.e[0];
			for (int i=1; i<lI; i++)
				flipLong( &(L.e[i]) );
		}
		return &L;
	} else
		return (loopr*)0;
}

loopr* hashiteratorloopCN (int h)
{
	static loopr L;
	static char *s_mshl = 0 ;
	if (!s_mshl) s_mshl = new char [maxSizeOfHashInternal+1024];
	cAtom *a = descripteurs_h[h].GiveAtom(0);
	if (!a) {
		delete [] s_mshl;
		s_mshl = 0;
		return (loopr*)0;
	}
	L.s = s_mshl;
	memcpy( L.s, a->Str(), a->WordLength() );
	L.e = (Int4*)(s_mshl+a->UsedLength()+1);
	
	L.e[0] = (int)(a->Property1());
	L.e[1] = (int)0;
	return &L;
}

loopr* hashiteratorloopIN (int h)
{
	static loopr L;
	static char *s_mshl = 0 ;
	if (!s_mshl) s_mshl = new char [maxSizeOfHashInternal+1024];
	cAtom *a = descripteurs_h[h].GiveAtom(0);
	if (!a) {
		delete [] s_mshl;
		s_mshl = 0;
		return (loopr*)0;
	}
	L.s = s_mshl;
	memcpy( L.s, a->Str(), a->WordLength() );
	L.e = (Int4*)(s_mshl+a->UsedLength()+1);

	L.e[0] = (int)(a->Property1());
	L.e[1] = (int)0;
	return &L;
}

int hashstat(int h, FILE* out)
{
	descripteurs_h[h].Stat(out);
	return 1;
}

long32 hashsize(int h)
{
	return descripteurs_h[h].ActualSize();
}

int alloc_and_write_string( char* rule )
{
	Int4 p = vmAllocate( strlen(rule) + 1 + sizeof(Int4) );
	vmWriteLong( strlen(rule) + 1, p );
	vmWrite( rule, p+sizeof(Int4), strlen(rule) + 1 );
	return (int)p;
}

void read_string( int adr, char* rule )
{
	int l = (int)vmReadLong( adr );
	vmRead( rule, adr+sizeof(Int4), l );
}

void write_string( int adr, char* rule )
{
	vmWriteLong( strlen(rule) + 1, adr );
	vmWrite( rule, adr+sizeof(Int4), strlen(rule) + 1 );
}
