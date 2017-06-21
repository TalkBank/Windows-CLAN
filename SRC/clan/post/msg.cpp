/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- msg.cpp	- v1.0.0 - november 1998

this file contains the procedure that permits to make message display platform independent

|=========================================================================================================================*/

#include "system.hpp"
#ifdef POSTCODE
	#include "cu.h"
	#ifdef _WIN32 
		#include "MSUtil.h"
	#endif
#endif
#include "msg.hpp"

#ifdef CLANEXTENDED
static FILE* clan_output = NULL;
static char morlib_path[1024] = "";
static char lib_path[1024] = "";

void set_morlib(char* name)
{
	strcpy( morlib_path, name );
}

void set_lib(char* name)
{
	strcpy( lib_path, name );
}

// implementation de 
FILE* OpenMorLib(char* filename, char* flags, int f1, int f2, char* comment )
{
	char fn[1024];
	strcpy(fn, morlib_path);
	strcat(fn, "\\");
	strcat(fn, filename);
	FILE* r = fopen(fn, flags);
	if (!r) {
//		msg( "error on opening file" );
		if (comment) msg( comment, filename );
	}
	return r;
}

FILE* OpenGenLib(char* filename, char* flags, int f1, int f2, char* comment )
{
	char fn[1024];
	strcpy(fn, lib_path);
	strcat(fn, "\\");
	strcat(fn, filename);
	FILE* r = fopen(fn, flags);
	if (!r) {
//		msg( "error on opening file" );
		if (comment) msg( comment, filename );
	}
	return r;
}

int	open_clan_output(char* name)
{
	clan_output = fopen(name,"w");
	return (clan_output) ? 1 : 0 ;
}

void close_clan_output()
{
	if (clan_output) fclose(clan_output);
}

void flush_clan_output()
{
	if (clan_output) fflush(clan_output);
}

void helpall_main( int argc, char** argv)
{
	msg("command:");
	for (int i=0;i<argc;i++) msg(" %s", argv[i]);
	msg("\n");
	msg("List of all extended commands\n");
	msg("\n");
	msg("COMPOUND = format file according to standard typographic \n");
	msg("           convention and check for all compound words\n");
	msg("CONSTR = list all constructions (from a description file - input is POSTed file)\n");
	msg("POST = morphosyntactic analysis (input is MORed file)\n");
	msg("POSTTRAIN = training of syntactic rules (from a reference tagged text)\n");
	msg("POSTLIST = list contents of POST database\n");
	msg("POSTMODRULES = allows to add manual rules to automatic analysis\n");
	msg("\n");
}

#endif

void msg( const char *string, ... )     // this function may be called from any program to redirect output 
				  // and is always used to console output
{
	char szmess[MaxSizeOfMsg];
#if !defined(POSTCODE) && !defined(UNX)
	char *q = (char *)(&string) + sizeof(char *);
	vsprintf(szmess, string, q);
#else
	va_list args;

	va_start(args, string); 	// prepare the arguments
	vsprintf(szmess, string, args);
	va_end(args);				// clean the stack
//	vsprintf(szmess, string, va_start(string));
#endif

#ifdef CLANEXTENDED
	fprintf( clan_output, "%s", szmess );
#else
	printf( "%s", szmess );
#endif
}

void msgfile( FILE* outfile, const char *string, ... )     // this function may be called from any program to redirect output
				  // and is used either to console output or to text file output
{
	char szmess[MaxSizeOfMsg];
#if !defined(POSTCODE) && !defined(UNX)
	char *q = (char *)(&string) + sizeof(char *);
	vsprintf(szmess, string, q);
#else
	int t;
	va_list args;

	va_start(args, string); 	// prepare the arguments
	t = vsprintf(szmess, string, args);
	va_end(args);				// clean the stack
#endif

	if ( outfile == stdout || outfile == stderr )
#ifdef CLANEXTENDED
		fprintf( clan_output, "%s", szmess );
#else
		printf( "%s", szmess );
#endif
	else
		fprintf( outfile, "%s", szmess );
}
