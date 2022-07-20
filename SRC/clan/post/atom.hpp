/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*
	atom.hpp
	--------
	version 0.99 - 21 octobre 1997
	------------
	hash table handling of character strings
*/

#ifndef __ATOM_HPP__
#define __ATOM_HPP__

/* atoms are made of a string of characters (and its length) and 
   three integer values (4 bytes integers)
*/

#include "hashbsc.hpp"

inline  int Align4(int v) { return v > 0 ? ((v-1)/4+1)*4 : 0 ; }

const int stringatommaxsize = 2048;
#define sizeof_header_cAtom (sizeof(Int4)*4)
class cAtom {
	// attention a modifer la taille ci-dessus: sizeof_header_cAtom
  public:
	Int4  m_WordLength;
	Int4  m_Prop1;
	Int4  m_Prop2;
	Int4  m_Prop3;
	unsigned char m_Word[stringatommaxsize];
  public:
	cAtom() { m_WordLength=0; m_Prop1 = m_Prop2 = m_Prop3 = 1789; }
	cAtom(unsigned char* str) { Set(str); }
	cAtom(unsigned char* str,int l) { Set(str,l); }
	~cAtom() {}
	unsigned char* Str() { return (unsigned char *)m_Word; }
	void  Set(unsigned char* str)
	{
		strcpy( (char*)m_Word, (char*)str );
		m_WordLength=strlen((char*)str) + 1;
	}
	void  Set(unsigned char* str, int l)
	{
		memcpy( (char*)m_Word, (char*)str, l );
		m_WordLength=l;
	}
	unsigned char  Char(int n) { return m_Word[n]; }
	int   UsedLength()	{ return Align4((int)m_WordLength) + sizeof_header_cAtom; }
	int   WordLength()	{ return (int)m_WordLength; }

	long32  Property1() { return (long32)m_Prop1; }
	void  SetProperty1(long32 v) { m_Prop1 = (Int4)v; }

	long32  Property2() { return (long32)m_Prop2; }
	void  SetProperty2(long32 v) { m_Prop2 = (Int4)v; }

	long32  Property3() { return (long32)m_Prop3; }
	void  SetProperty3(long32 v) { m_Prop3 = (Int4)v; }

	long32 NbOcc();
//	void vmRead( address vadr, cAtom* mptr );
};

class cHashAtom : public cHashBasic {
	int	type_key;	// 1 Int4 key, 0 char key
  public:
	address Create( long32 sizeBB, int type=0 );
	int Open( address vmadr, int type=0 );
	address AddC( cAtom* a );  /* add one element (atom) */
	address AddI( cAtom* a );  /* add one element (atom) */
	void ReWriteHead( cAtom* a, address p ); /* rewrite an atom after modifications */
	cAtom* LookC( cAtom* a ); /* look for one element (atom) */
	cAtom* LookI( cAtom* a ); /* look for one element (atom) */
	address LookC_AddressOf( cAtom* a ); /* look for one element (atom) */
	address LookI_AddressOf( cAtom* a ); /* look for one element (atom) */
	cAtom* GiveAtom(int firsttime); /* for Dump and for iteration */
};

#endif /* __ATOM_HPP__ */

