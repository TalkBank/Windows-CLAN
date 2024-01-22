/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- system.hpp	- v1.0.0 - november 1998
-- this file contains all the required system include files.
|=========================================================================================================================*/

#ifndef __system_hpp__
#define __system_hpp__

#include "longdef.hpp"

#if defined(POSTCODE) || defined(_MAC_CODE)
	#include "cu.h"

	#define fgets fgets_cr
	#define _stricmp uS.mStricmp
	#define _strnicmp uS.mStrnicmp
	#define sprintf uS.sprintf
#else

	#include <stdio.h>
  #if !defined(MacOsx) && !defined(UNX)
	#include <io.h>
  #endif
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>

  #ifndef WINDOWS_VC9
	#define _stricmp mStricmp
	#define _strnicmp mStrnicmp
  #endif

  #if !defined(_MAX_PATH)
	#define _MAX_PATH	2048
  #endif
	#define FNSize _MAX_PATH+FILENAME_MAX
	#define FNType char

	#define EOS '\0'
#endif /* defined(POSTCODE) || defined(_MAC_CODE) */

#if defined(UNX)
#include <ctype.h>
#include <unistd.h>

#define FONTHEADER		"@Font:"
#define WINDOWSINFO		"@Window:"
#define CKEYWORDHEADER	"@Color words:"
#define PIDHEADER		"@PID:"
#define UTF8HEADER		"@UTF8"

#define stricmp mStricmp
#define strnicmp mStrnicmp

extern int mStrnicmp(const char *st1, const char *st2, size_t len);
extern int mStricmp(const char *st1, const char *st2);

#endif

#include <setjmp.h>


#include "msg.hpp"


#ifdef CLANEXTENDED
#include "ClanCommands.h"
#endif

typedef long32 Int4;
typedef short int Int2;

#define MaxFnSize 256

extern const char *strchrConst(const char *s, int c);

#endif
