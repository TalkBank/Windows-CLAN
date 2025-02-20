/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef _Memory_h_
#define _Memory_h_

#include <stddef.h>

#include "sysdep.hpp"

#if defined(POSTCODE) || defined(_MAC_CODE)
#if __LP64__
	#define long32 int
#else
	#define long32 long
#endif

extern "C"
{
#endif


extern VOIDP Memory_allocate(size_t);
extern VOIDP Memory_reallocate(VOIDP, size_t);
extern NORET Memory_free(VOIDP);
extern long32 Memory_unfreed_bytes(NOARGS);

#if defined(POSTCODE) || defined(_MAC_CODE)
}
#endif

#endif
