/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- storage.cpp	- v1.0.0 - november 1998
	version 0.99 - 21 october 1997
	Memory Handling
|=========================================================================================================================*/

#include "system.hpp"

#include "storage.hpp"
#include "database.hpp"

#ifdef POSTCODE
//extern "C"
//{
//	extern void myjmp(int jval);
//}
#endif

/*** there is several possibilites of work in virtual memory
     1- real memoire
     2- disk memoire
     3- hardware virtual memory (if possible)
 ***/

/* this is for error recovery */
jmp_buf mark;              /* Address for long32 jump to jump to */

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

struct vmStructure {
	int vmType;	/* type of memoire (1,2, or 3 above) */
	int vmRW;	/* allowed to be saved to disk */
	int vmReversed; /* is this a reversed database */

	long32 vmCurrentSize;
	long32 vmCurrentAllocatedSize;

	/* real memory */

	unsigned char* mRealMemory;

	/* disk memory */

	FILE* mFileMemory;
	FNType  mFileMemoryName[FNSize];
};

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

static vmStructure VM = { 0, 0, 0, 0L, 0L, 0, 0, {'\0'} } ;	/* global access to virtual memory */

static void testfilememory( FILE* f, long32 e, const char *s )
{
	if (e<0) {
		msg ( "cannot extend file with %ld bytes\n", e );
		local_abort( VM.vmCurrentAllocatedSize, s );
	}
/*	char m[1024];	
	while ( e > 1024 ) {
		if ( fwrite( m, 1024, 1, f ) !=1 ) {
			perror( "error on write in test file memory" );
			exit(-1);
		}
		e -= 1024;
	}
*/
}

int vmCreate( FNType* vmexternalname, int vmtype, long32 vmavesize, int reversed, long32 vminitsize )
{
	/** global variable updated **/
	VM.vmReversed = reversed;	// is this a reversed database.
	VM.vmType = vmtype;
	VM.vmCurrentSize = 0;
	VM.vmCurrentAllocatedSize = 0;
	VM.vmRW = 1;
	strcpy( VM.mFileMemoryName, vmexternalname );
	

	switch ( VM.vmType ) {
		case vmRealMemory:
// msg ("m= %d\n", vmavesize );
			VM.mRealMemory = new unsigned char [vmavesize];
			if (VM.vmRW==1) msg ( "\nwarning: %ld bytes allocated (%4.2fMo)\n", vmavesize, (double)vmavesize / (double)(Mo) );
			if (!VM.mRealMemory) local_abort(vmavesize, "vmInitialize");
			VM.vmCurrentAllocatedSize = vmavesize;
			VM.vmCurrentSize = vminitsize;
			return 1;
		case vmFileMemory:
			VM.mFileMemory = fopen( VM.mFileMemoryName, "w+b" );
#ifdef POSTCODE
#ifdef _MAC_CODE
			settyp(VM.mFileMemoryName, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
#endif			
			VM.vmCurrentAllocatedSize = vmavesize;
			VM.vmCurrentSize = vminitsize;
			if (VM.vmRW==1) msg ( "\nwarning: %ld bytes of file memory allocated and tested (%4.2fMo)\n", vmavesize, (double)vmavesize / (double)(Mo) );
			testfilememory( VM.mFileMemory, vmavesize-vminitsize, "vmInitialize(case2:FileMemory)" );
			return 1;
	}

	return 0;
}

int vmOpen(  FNType* vmexternalname, int vmtype, long32 vmavesize, int reversed, int RW )
{
	/** global variable updated **/
	VM.vmReversed = reversed;	// is this a reversed database.
	VM.vmType = vmtype;
	VM.vmCurrentSize = 0;
	VM.vmCurrentAllocatedSize = 0;
	VM.vmRW = RW;
	strcpy( VM.mFileMemoryName, vmexternalname );

	FILE *f;
#if defined(POSTCODE) || defined(CLANEXTENDED)
	FNType mFileName[FNSize], tFileName[FILENAME_MAX];
#endif

	switch ( VM.vmType ) {
		case vmRealMemory:
#if defined(POSTCODE) || defined(CLANEXTENDED)
			f = OpenMorLib(VM.mFileMemoryName,"rb",FALSE,FALSE,mFileName);
			if (f == NULL) {
				f = OpenGenLib(VM.mFileMemoryName,"rb",FALSE,FALSE,mFileName);
				if (f != NULL)
					fprintf(stderr, "    Using database file:      %s.\n", mFileName);
				else {
					define_databasename(tFileName);
					f = OpenMorLib(tFileName,"rb",FALSE,FALSE,mFileName);
					if (f == NULL) {
						f = OpenGenLib(tFileName,"rb",FALSE,FALSE,mFileName);
						if (f != NULL)
							fprintf(stderr, "    Using database file:      %s.\n", mFileName);
					} else
						fprintf(stderr, "    Using database file:      %s.\n", mFileName);
				}
			} else
				fprintf(stderr, "    Using database file:      %s.\n", mFileName);
#else
			f= fopen( VM.mFileMemoryName, "rb" );
			if (f) {
				fprintf(stderr, "    Using database file:      %s.\n", VM.mFileMemoryName);
			}
#endif
			if (f) {
				fseek( f, 0L, SEEK_END );
				VM.vmCurrentSize=ftell(f);
				if (vmavesize==0L)
					vmavesize = VM.vmCurrentSize + SizeSupplement;
				else if (VM.vmCurrentSize + SizeSupplement>vmavesize)
					vmavesize = VM.vmCurrentSize + SizeSupplement;
// msg ("m= %d\n", vmavesize );
				VM.mRealMemory = new unsigned char [vmavesize];
				if (!VM.mRealMemory) local_abort(vmavesize, "vmInitializeFromOld");
				VM.vmCurrentAllocatedSize = vmavesize;
				fseek( f, 0L, SEEK_SET );
				fread( VM.mRealMemory, VM.vmCurrentSize, 1, f );
				fclose( f );
				return 1;
			}
			msg ( "cannot open \"%s\".\n***Please make sure that any file with \".db\" in \"mor lib\" folder is renamed to %s\n", VM.mFileMemoryName, VM.mFileMemoryName);
			msg ( "a workspace has to be created\n" );
			return 0;
			/**
			 VM.mRealMemory = new unsigned char [vmavesize];
			 if (!VM.mRealMemory) local_abort(vmavesize, "vmInitializeFromOld(2)");
			 VM.vmCurrentAllocatedSize = vmavesize;
			 **/
			// return 1;
		case vmFileMemory:

#if defined(POSTCODE) || defined(CLANEXTENDED)
			VM.mFileMemory=OpenMorLib(VM.mFileMemoryName,"r+b",FALSE,FALSE,mFileName);
			if (VM.mFileMemory == NULL)
				VM.mFileMemory= OpenGenLib(VM.mFileMemoryName,"r+b",FALSE,FALSE,mFileName);
#else
			VM.mFileMemory=fopen( VM.mFileMemoryName, "r+b" );
#endif
			if (VM.mFileMemory == NULL)
			{
				msg ( "cannot open \"%s\".\n***Please make sure that any file with \".db\" in \"mor lib\" folder is renamed to %s\n", VM.mFileMemoryName, VM.mFileMemoryName);
				msg ( "a workspace should be created\n" );
				return 0;
			}
			fseek(VM.mFileMemory,0L,SEEK_END);
			VM.vmCurrentSize = ftell(VM.mFileMemory);
			VM.vmCurrentAllocatedSize = vmavesize>VM.vmCurrentSize ?vmavesize :VM.vmCurrentSize ;
			return 1;
	}

	return 0;
}

void vmSave()
{
#if defined(POSTCODE) || defined(CLANEXTENDED)
	FNType mFileName[FNSize];
#endif

	switch ( VM.vmType ) {
		case vmRealMemory:
			{
			  FILE* f= fopen( VM.mFileMemoryName, "wb" );
#ifdef POSTCODE
#ifdef _MAC_CODE
			settyp(VM.mFileMemoryName, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
#endif			
			  fwrite( VM.mRealMemory, VM.vmCurrentSize, 1, f );
			  fclose( f );
			}
			return;
		case vmFileMemory:
			fclose( VM.mFileMemory );
			
#if defined(POSTCODE) || defined(CLANEXTENDED)

			VM.mFileMemory=OpenMorLib(VM.mFileMemoryName,"r+b",FALSE,FALSE,mFileName);
			if (VM.mFileMemory == NULL)
				VM.mFileMemory= OpenGenLib(VM.mFileMemoryName,"r+b",FALSE,FALSE,mFileName);
#else
			VM.mFileMemory=fopen( VM.mFileMemoryName, "r+b" );
#endif
			if (VM.mFileMemory == NULL)
			{
				msg ( "cannot reopen %s\n", VM.mFileMemoryName );
				msg ( "very fatal error: please stop the program\n" );
				return;
			}
	}
}

void vmClose()
{
	switch ( VM.vmType ) {
		case vmRealMemory:
			if ( VM.mRealMemory ) delete [] VM.mRealMemory;
			VM.mRealMemory = 0;
			return;
		case vmFileMemory:
			fclose( VM.mFileMemory );
			return;
	}
}

address vmAllocate( int nbbytes )
{
	long32 old_size;

	address returnvalue;
	switch ( VM.vmType ) {
		case vmRealMemory:
			returnvalue = VM.vmCurrentSize;
			VM.vmCurrentSize += nbbytes;
			if (VM.vmCurrentSize > VM.vmCurrentAllocatedSize) {
				do
					VM.vmCurrentAllocatedSize += _IncrSizeMV_ * Ko;
				while (VM.vmCurrentSize > VM.vmCurrentAllocatedSize);
				if (VM.vmRW==1) msg ( "warning: real memory has been extended to [%4.2f] Mo\n",
					(double)VM.vmCurrentAllocatedSize / (double)(Mo) );

// msg ("m= %d\n", VM.vmCurrentAllocatedSize );
				unsigned char* newmem = new unsigned char [VM.vmCurrentAllocatedSize];
				if ( !newmem ) local_abort( VM.vmCurrentAllocatedSize, "vmAllocate" );
				memcpy( newmem, VM.mRealMemory, VM.vmCurrentSize - nbbytes );
				delete [] VM.mRealMemory;
				VM.mRealMemory = newmem;
			}
			return returnvalue;
		case vmFileMemory:
			returnvalue = VM.vmCurrentSize;
			VM.vmCurrentSize += nbbytes;
			if (VM.vmCurrentSize > VM.vmCurrentAllocatedSize) {
				old_size = VM.vmCurrentSize - nbbytes ;
				do {
					VM.vmCurrentAllocatedSize += Mo;
				} while (VM.vmCurrentSize > VM.vmCurrentAllocatedSize);

				if (VM.vmRW==1) msg ( "warning: file memory has been extended to [%4.2f] Mo\n",
					(double)VM.vmCurrentAllocatedSize / (double)(Mo) );
				testfilememory( VM.mFileMemory, VM.vmCurrentAllocatedSize - old_size, "vmAllocate(case2:FileMemory)" );
			}
			return returnvalue;
	}

	return NILMEMORY;
}

int vmIsReversed()
{
	return VM.vmReversed ;
}

void vmSetReversed(int reversed)
{
	VM.vmReversed = reversed;
}

void flipLong(long32 *num) {
	unsigned char *s, t0, t1, t2, t3;

	s = (unsigned char *)num;
	t0 = s[0];
	t1 = s[1];
	t2 = s[2];
	t3 = s[3];
	s[3] = t0;
	s[2] = t1;
	s[1] = t2;
	s[0] = t3;
}

void flipShort(short *num) {
	unsigned char *s, t0, t1;

	s = (unsigned char *)num;
	t0 = s[0];
	t1 = s[1];
	s[1] = t0;
	s[0] = t1;
}

short int vmReadShort( address vmadr )
{
	short int v;
	// the following should be a compilation time condition.
	if (2 != sizeof(short int)) {
		msg( "non-compatible \"short int\" size, it is \"%d\", but should be \"2\"\n", sizeof(short int));
		exit(1);
	}
	if ( VM.vmReversed == 0 ) { // not reversed
		vmRead(&v, vmadr, 2);
	} else {
		unsigned char  st[2];	// intermediate value.
		unsigned char* sv = (unsigned char *)&v;
		vmRead(st, vmadr, 2);
		sv[1] = st[0];
		sv[0] = st[1];
	}
	return v;
}

int vmReadInt( address vmadr )
{
	// the following should be a compilation time condition.
	if (4 == sizeof(int)) {
		return vmReadLong(vmadr);
	} else {
		msg( "non-compatible \"int\" size, it is \"%d\", but should be \"4\"\n", sizeof(int));
		exit(1);
	}
}

long32 vmReadLong( address vmadr )
{
	long32 v;
	// the following should be a compilation time condition.
	if (4 != sizeof(long32)) {
		msg( "non-compatible \"long32\" size, it is \"%d\", but should be \"4\"\n", sizeof(long32));
		exit(1);
	}
	if ( VM.vmReversed == 0 ) { // not reversed
		vmRead(&v, vmadr, 4);
	} else {
		unsigned char  st[4];	// intermediate value.
		unsigned char* sv = (unsigned char *)&v;
		vmRead(st, vmadr, 4);
		sv[3] = st[0];
		sv[2] = st[1];
		sv[1] = st[2];
		sv[0] = st[3];
	}
	return v;
}

double vmReadDouble( address vmadr )
{
	double v = .0;
	msg( "double flip/flap not implemented\n" );
	exit(1);
	return v;
}

void	vmRead( void* adr, address vmadr, int nbbytes )
{
	unsigned long32 Uvmadr, Unbbytes, totalSize;

	switch ( VM.vmType ) {
		case vmRealMemory:
			Uvmadr = vmadr;
			Unbbytes = nbbytes;
			totalSize = VM.vmCurrentAllocatedSize;
			if (Uvmadr+Unbbytes > totalSize) {
				fprintf(stderr, "\n    Post database is corrupted.\n    Please re-build it with \"posttrain +c\" command and you traning data set.\n\n");
				exit(0);
			} else
				memcpy( adr, VM.mRealMemory+vmadr, nbbytes );
			return;
		case vmFileMemory:
			fseek( VM.mFileMemory, vmadr, SEEK_SET );
			fread( adr, nbbytes, 1, VM.mFileMemory );
			return;
	}
}

void	vmWriteShort( short int v, address vmadr )
{
	// the following should be a compilation time condition.
	if (2 != sizeof(short int)) {
		msg( "non-compatible \"short int\" size, it is \"%d\", but should be \"2\"\n", sizeof(short int));
		exit(1);
	}
	if ( VM.vmReversed == 0 ) { // not reversed
		vmWrite(&v, vmadr, 2);
	} else {
		unsigned char  st[2];	// intermediate value.
		unsigned char* sv = (unsigned char *)&v;
		st[1] = sv[0];
		st[0] = sv[1];
		vmWrite(st, vmadr, 2);
	}
}

void	vmWriteInt( int v, address vmadr )
{
	// the following should be a compilation time condition.
	if (4 == sizeof(int)) {
		vmWriteLong(v, vmadr);
	} else {
		msg( "non-compatible \"int\" size, it is \"%d\", but should be \"4\"\n", sizeof(int));
		exit(1);
	}
}

void	vmWriteLong( long32 v, address vmadr )
{
	// the following should be a compilation time condition.
	if (4 != sizeof(long32)) {
		msg( "non-compatible \"long32\" size, it is \"%d\", but should be \"4\"\n", sizeof(long32));
		exit(1);
	}
	if ( VM.vmReversed == 0 ) { // not reversed
		vmWrite(&v, vmadr, 4);
	} else {
		unsigned char  st[4];	// intermediate value.
		unsigned char* sv = (unsigned char *)&v;
		st[3] = sv[0];
		st[2] = sv[1];
		st[1] = sv[2];
		st[0] = sv[3];
		vmWrite(st, vmadr, 4);
	}
}

void	vmWriteDouble( double v, address vmadr )
{
	msg( "double flip/flap not implemented\n" );
	exit(1);
}

void	vmWrite( const void* adr, address vmadr, int nbbytes )
{
	unsigned long32 Uvmadr, Unbbytes, totalSize;

	switch ( VM.vmType ) {
		case vmRealMemory:
			Uvmadr = vmadr;
			Unbbytes = nbbytes;
			totalSize = VM.vmCurrentAllocatedSize;
			if (Uvmadr+Unbbytes > totalSize) {
				fprintf(stderr, "\n    Post database is corrupted.\n    Please re-build it with \"posttrain +c\" command and you traning data set.\n\n");
				exit(0);
			} else
				memcpy( VM.mRealMemory+vmadr, adr, nbbytes );
			return;
		case vmFileMemory:
			fseek( VM.mFileMemory, vmadr, SEEK_SET );
			if ( fwrite( adr, nbbytes, 1, VM.mFileMemory ) !=1 ) {
				perror( "error on vmWrite" );
				exit(-1);
			}
			return;
	}
}

/*** Management of a local pool of memory ***/
static char* _dyn_memory = 0;
static unsigned long32 _dyn_current = 0;
static unsigned long32 _dyn_total_size = 0;

char* dynalloc(unsigned size)
{
	size = (((size-1)/4)+1)*4;
	if ( !_dyn_memory ) {
		_dyn_total_size = SizeDynAlloc ;
// msg ("m= %d\n", _dyn_total_size );
#ifdef POSTCODE
		_dyn_memory = (char*) malloc(_dyn_total_size);
		if (_dyn_memory == NULL) {
#ifndef UNX
			do_warning("Out of memory. Please increase program memory size to a minimum of 15 Mb. The program will quit after this message.", 0);
			myjmp(1999);
#else
			fprintf(stderr, "Out of memory. Please increase program memory size to a minimum of 15 Mb. The program will quit after this message.");
			exit(0);
#endif
		}
#else
		_dyn_memory = (char*) new char [ _dyn_total_size ];
#endif
	}
	if ( _dyn_current + size > _dyn_total_size ) {
		msg( " Not enough memory allocated (%d allocated)\n", _dyn_total_size );
		msg( " Increase program size\n" );
		exit(-1);
//		longjmp( mark, -1 );
//		return 0;
	}
	char* rv = _dyn_memory + _dyn_current ;
	_dyn_current += size;
	return rv;
}

void  dynreset()
{
//	msg( "Memory used (in dynalloc) : %d\n", _dyn_current );
	_dyn_current = 0;
}

void  dynfreeall()
{
	_dyn_current = 0;
	_dyn_total_size = 0;
	if ( _dyn_memory ) {
#ifdef POSTCODE
		free(_dyn_memory);
#else
		delete [] _dyn_memory ;
#endif
	}
	_dyn_memory = 0;
}
