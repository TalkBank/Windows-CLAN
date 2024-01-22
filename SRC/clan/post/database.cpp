/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- database.cpp	- v1.0.0 - november 1998
|=========================================================================================================================*/

#include "system.hpp"

#include "database.hpp"


void define_databasename(FNType *dbn)
{
#ifdef POSTCODE
	FNType *p;
	FNType mFileName[FNSize];

	strcpy(mFileName, mor_lib_dir);
	do {
		p = strrchr(mFileName, PATHDELIMCHR);
		if (p == NULL) {
			strcpy(dbn, "eng.db");
		} else {
			if (*(p+1) == EOS && p != mor_lib_dir) {
				*p = EOS;
			} else {
				strcpy(dbn, p+1);
				strcat(dbn, ".db");
				break;
			}
		}
	} while (p != NULL) ;
#else
	strcpy(dbn, "eng.db");
#endif
}

FNType* fileresolve(FNType* fn, FNType* str)
{
	char *p;

#if defined(UNX)
	if ( access(fn, 0) == 0 )
#elif defined(__STDC__) && !defined(POSTCODE)
	if ( _access( fn, 0 ) != -1 )
#else
	if ( access(fn, 0) == 0 )
#endif
	{
		strcpy( str, fn );
		return str;
	}
	p = getenv("POST");
	if (p) {
		strcpy( str, p );
#ifdef _WIN32
		strcat( str, "\\" );
#else
		strcat( str, "/" );
#endif
		strcat( str, fn );

#if defined(UNX)
		if ( access(str, 0) == 0 )
#elif defined(__STDC__) && !defined(POSTCODE)
		if ( _access( str, 0 ) != -1 )
#else
		if ( access(str, 0) == 0 )
#endif
			return str;
	}
	strcpy( str, fn );
	return str;
}

int create_all( FNType* fn, int inmemory )
{
	msg( "Database creation %s ...\n", fn );
	int rv = wkscreate(fn, inmemory);
	if (rv) rv=classname_init();
	return rv;
}

int open_all( FNType* fn, int RW, int inmemory )
{
//	msg( "Database opening %s ...\n", fn );
	int rv = wksopen(fn, RW, inmemory);
	if (rv) {
		if ((rv=classname_open()) == -1)
			return(0);
	}
	return rv;
}

int save_all()
{
	classname_save();

// address_savedate

	return wkssave();
}

int close_all()
{
	return wksclose();
}

void local_abort ( long32 size, const char* funname )
{
	if (size==0) {
	msg( "\nfatal error: %s\n", funname );
	} else {

	msg( "\nfatal error: memory cannot be allocated [%4.2f] Mo in function %s\n",
		(double)size / (double)Mo, funname );
	msg( "stderr: fatal error: no more core memory in file Stockage.cpp\n");
	msg( "\nstdout: fatal error: no more core memory in file Stockage.cpp\n");
	}
	exit(-1);
}
