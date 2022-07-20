/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef _darrayP_h_
#define _darrayP_h_

#include "sysdep.hpp"
#include "memory.hpp"

#include "darray.hpp"

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

typedef struct st_Darray {
  unsigned length;
  VOIDP *storage;
  unsigned storage_offset;
  unsigned storage_length;
} Darray_rep;

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

#define draise(p_to_rep) ((Darray)p_to_rep)
#define dlower(obj) ((Darray_rep *)obj)
#define dcreate() ((Darray_rep *)Memory_allocate(sizeof(Darray_rep)))
/*#define destroy(p_to_rep) (Memory_free(p_to_rep))*/

#define MAX_GROW_STEP 100	/* Defines the maximum amount to increase
				   storage_length when storage is full.
				   A call to Darray_hint can cause a larger
				   grow to occur; this only applies to 
				   "on-demand grows" */

enum grow_direction {GROW_HIGH, GROW_LOW};

static NORET grow(Darray_rep *, enum grow_direction, unsigned);

/* Invariant conditions:
 * 
 * Each array component (object) is stored in a void pointer.
 *
 * [storage_length] >= 1
 * 
 * [storage] is pointer to a a heap object large enough for
 * [storage_length] pointers to void,
 *
 * [storage_length] >= [length]
 * 
 * The n-th element (index=n-1) of the dynamic array is located at 
 * address [storage]+[storage_offset]+n-1.
 *
 */ 
#endif /* ifndef _darrayP_h_ */
