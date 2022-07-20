/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- workspace.hpp	- v1.0.0 - november 1998
	version 0.99 - 21 october 1997
	Handling symbolic addresses of databases in one only file
|=========================================================================================================================*/

#ifndef __WORKSPACE_HPP__
#define __WORKSPACE_HPP__

#define low_WSP_VERSION_NUMBER  (0x0121)
#define high_WSP_VERSION_NUMBER (0x7755)
#define tis_a_reverse_file (0x5577)

const int MaxNameSA = 512;

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

/*** storage of data names with their virtual addresses ***/
struct cSymbolicAddress {
	char m_Name[MaxNameSA];
	address m_VirtualAddress;
	address m_NextSA;
};

/*
	HEADER of FILE
	bloc with pointers to descriptions.
*/

struct cVMWorkSpace {
	char	m_sgn[2];
	short int m_lversionnumber;
	short int m_hversionnumber;
	/** **/
	char	m_CreationDate[128];
	char	m_CreationName[128];
	/** **/
	address m_FirstElement;
};
/*** FOLLOWED BY DATA ***/

struct cWorkSpace {
	int	m_Reversed;	// If equal to 1, the current workspace has been created on a PC (little endian) 
				// and is runing on a Mac (big endian), or this opposite. If 0, same machine.

	cVMWorkSpace m_vm;	// virtual memory

	int	m_MemVsFile;
	long32	m_SzVirtualMemory;

	/** **/

	cWorkSpace()
	{
		m_MemVsFile = vmRealMemory;
		m_SzVirtualMemory = 500000L;
	}

	/** basic functions **/
	void Parameters( int memvsfile, long32 szvmem );
	int  Create(FNType* extern_name);
	int  Open(FNType* extern_name, int RW=1);
	void Close();
	void Save();

	/** functions of symbolic names handling **/
	int  CreateNS(const char* nomsymbolique, address nsvm);
	address OpenNS(const char* nomsymbolique);
	void ListNS();
};

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

#endif /* __WORKSPACE_HPP__ */
