/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- workspace.hpp	- v1.0.0 - november 1998
	version 0.99 - 21 october 1997
	Handling symbolic addresses of databases in one only file
|=========================================================================================================================*/

#include "system.hpp"

#include "storage.hpp"
#include "workspace.hpp"

void cWorkSpace::Parameters( int memvsfile, long32 szvmem )
{
	if (szvmem < 100000)  szvmem = 100000;
	if (szvmem > 256000000) szvmem = 256000000;

	m_MemVsFile = memvsfile ? vmRealMemory : vmFileMemory;
	m_SzVirtualMemory = szvmem;
}

int cWorkSpace::Create(FNType* s)
{
	m_Reversed = 0;	// by default, the workspace is not upside down

	m_vm.m_sgn[0] = 'W';
	m_vm.m_sgn[1] = 'X';

	m_vm.m_lversionnumber = low_WSP_VERSION_NUMBER;
	m_vm.m_hversionnumber = high_WSP_VERSION_NUMBER;

#if defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
	strcpy(m_vm.m_CreationName, s);
#else
	uS.FNType2str(m_vm.m_CreationName, 0L, s);
#endif
	strcpy( m_vm.m_CreationDate, "00/00/0000" );

	m_vm.m_FirstElement = (address)0;

	/*** virtual memory initialization ***/
	if (!vmCreate( s, m_MemVsFile, m_SzVirtualMemory, m_Reversed, sizeof(cVMWorkSpace) )) return 0;
	/** write one by one each element of the structure **/
	// OLD *** vmWrite( &m_vm, 0l, sizeof(cVMWorkSpace) );
	vmWrite( m_vm.m_sgn, 0L, 2 );
	vmWriteShort ( m_vm.m_lversionnumber, 2L );
	vmWriteShort ( m_vm.m_hversionnumber, (long32)(2+sizeof(short int)) );
	vmWrite ( m_vm.m_CreationDate, (long32)(2+2*sizeof(short int)), 128 );
	vmWrite ( m_vm.m_CreationName, (long32)(2+2*sizeof(short int)+128), 128 );
	vmWriteLong ( m_vm.m_FirstElement, (long32)(2+2*sizeof(short int)+2*128) );

	return 1;
}

int cWorkSpace::Open(FNType* s, int RW)
{
	m_Reversed = 0;	// by default, the workspace is not upside down

	/*** virtual memory initialization from a memory image ***/
	if (!vmOpen( s, m_MemVsFile, 0L, m_Reversed, RW )) return 0;

	/** read version element of the structure **/
	// OLD *** vmRead( &m_vm, 0l, sizeof(cVMWorkSpace) );
	vmRead( m_vm.m_sgn, 0L, 2 );
	m_vm.m_lversionnumber = vmReadShort ( 2L );
	m_vm.m_hversionnumber = vmReadShort ( (long32)(2+sizeof(short int)) );
	vmRead ( m_vm.m_CreationDate, (long32)(2+2*sizeof(short int)), 128 );
	vmRead ( m_vm.m_CreationName, (long32)(2+2*sizeof(short int)+128), 128 );

	if (m_vm.m_sgn[0] != 'W' || m_vm.m_sgn[1] != 'X') {
		msg( "error: this file is not a POST workspace\n" );
		return 0;
	}
	if (m_vm.m_hversionnumber != high_WSP_VERSION_NUMBER ) {
		if ( m_vm.m_hversionnumber != tis_a_reverse_file ) {
			// msg( "%x %x\n", m_vm.m_hversionnumber, tis_a_reverse_file );
			msg( "error: this is an unknown type of workspace\n" );
			return 0;
		}
		// ELSE the workspace was created on a different machine, so every integer should be reversed.
		// msg( "info: this database has been created with a different byte order\n" );

		flipShort( &m_vm.m_hversionnumber );
		flipShort( &m_vm.m_lversionnumber );
		vmSetReversed( 1 );
		m_Reversed = 1;
	}

	if (m_vm.m_lversionnumber != low_WSP_VERSION_NUMBER ) {
		int ov1 = m_vm.m_lversionnumber % 16;
		int ov2 = (m_vm.m_lversionnumber / 16) % 16;
		int ov3 = (m_vm.m_lversionnumber / (16*16)) % 16;
		int nv1 = low_WSP_VERSION_NUMBER % 16;
		int nv2 = (low_WSP_VERSION_NUMBER / 16) % 16;
		int nv3 = (low_WSP_VERSION_NUMBER / (16*16)) % 16;

		msg( "error: the version of the workspace is %d.%d.%d instead of %d.%d.%d\n", ov3, ov2, ov1, nv3, nv2, nv1 );
		return 0;
	}

	/*** read last element of workspace structure ***/
	m_vm.m_FirstElement = vmReadLong ( (long32)(2+2*sizeof(short int)+2*128) );

	/*** ***/
	// msg( "Workspace is %s\n", m_Reversed ? "reversed" : "not reversed" );
	
	return 1;
}

void cWorkSpace::Save()
{
	/** write one by one each element of the structure **/
	vmWrite( m_vm.m_sgn, 0L, 2 );
	vmWriteShort ( m_vm.m_lversionnumber, 2L );
	vmWriteShort ( m_vm.m_hversionnumber, (long32)(2+sizeof(short int)) );
	vmWrite ( m_vm.m_CreationDate, (long32)(2+2*sizeof(short int)), 128 );
	vmWrite ( m_vm.m_CreationName, (long32)(2+2*sizeof(short int)+128), 128 );
	vmWriteLong ( m_vm.m_FirstElement, (long32)(2+2*sizeof(short int)+2*128) );
	vmSave();
}

void cWorkSpace::Close()
{
	/** write one by one each element of the structure **/
	vmWrite( m_vm.m_sgn, 0L, 2 );
	vmWriteShort ( m_vm.m_lversionnumber, 2L );
	vmWriteShort ( m_vm.m_hversionnumber, (long32)(2+sizeof(short int)) );
	vmWrite ( m_vm.m_CreationDate, (long32)(2+2*sizeof(short int)), 128 );
	vmWrite ( m_vm.m_CreationName, (long32)(2+2*sizeof(short int)+128), 128 );
	vmWriteLong ( m_vm.m_FirstElement, (long32)(2+2*sizeof(short int)+2*128) );
	vmClose();
}

int cWorkSpace::CreateNS(const char* extname, address where)
{
	cSymbolicAddress tmp;

	/*** verification si pas de noms doubles ***/
	if ( OpenNS(extname) != (address)0 ) return 0;

	address iadr = vmAllocate( sizeof(cSymbolicAddress) );
	if ( strlen(extname) < MaxNameSA )
		strcpy( tmp.m_Name, extname );
	else {
		strncpy( tmp.m_Name, extname, MaxNameSA -1 );
		tmp.m_Name[MaxNameSA -1] = '\0';
	}
	tmp.m_VirtualAddress = where;
	tmp.m_NextSA = m_vm.m_FirstElement;
	/** write one by one each element of the structure **/
	// OLD *** vmWrite( &tmp, iadr, sizeof(cSymbolicAddress) );
	vmWrite( tmp.m_Name, iadr, MaxNameSA );
	vmWriteLong( tmp.m_VirtualAddress, iadr + (long32)(MaxNameSA) );
	vmWriteLong( tmp.m_NextSA, iadr + (long32)(MaxNameSA+sizeof(long32)) );

	m_vm.m_FirstElement = iadr;
	/** write last element of the structure **/
	vmWriteLong ( m_vm.m_FirstElement, (long32)(2+2*sizeof(short int)+2*128) );
	return 1;
}

address cWorkSpace::OpenNS(const char* extname)
{
	cSymbolicAddress tmp;
	address iadr = m_vm.m_FirstElement;
	while (iadr != (address)0) {
		/** read one by one each element of the structure **/
		// OLD *** vmRead( &tmp, iadr, sizeof(cSymbolicAddress) );
		vmRead( tmp.m_Name, iadr, MaxNameSA );
		tmp.m_VirtualAddress = vmReadLong( iadr + (long32)(MaxNameSA) );
		tmp.m_NextSA = vmReadLong( iadr + (long32)(MaxNameSA+sizeof(long32)) );
		if ( strlen(extname) < MaxNameSA ) {
			if (!strcmp( tmp.m_Name, extname ))
				return tmp.m_VirtualAddress;
		} else {
			if (!strncmp( tmp.m_Name, extname, MaxNameSA -1 ))
				return tmp.m_VirtualAddress;
		}
		iadr = tmp.m_NextSA;	
	}
	return (address)0;
}

void cWorkSpace::ListNS()
{
	cSymbolicAddress tmp;
	address iadr = m_vm.m_FirstElement;
	while (iadr != (address)0) {
		/** read one by one each element of the structure **/
		// OLD *** vmRead( &tmp, iadr, sizeof(cSymbolicAddress) );
		vmRead( tmp.m_Name, iadr, MaxNameSA );
		tmp.m_VirtualAddress = vmReadLong( iadr + (long32)(MaxNameSA) );
		tmp.m_NextSA = vmReadLong( iadr + (long32)(MaxNameSA+sizeof(long32)) );
		fprintf(stderr,"%s %lx %lx\n", tmp.m_Name, tmp.m_VirtualAddress, tmp.m_NextSA);
		iadr = tmp.m_NextSA;	
	}
}

