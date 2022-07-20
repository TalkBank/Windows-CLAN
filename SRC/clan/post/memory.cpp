/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include <stdio.h>
#include "memory.hpp"
#include "sysdep.hpp"


#ifndef MEMORY_LEAK_COUNT
long32 Memory_unfreed_bytes(NOARGS)
{
  return 0;
}
#else
static long32 bytes_unfreed;
long32 Memory_unfreed_bytes(NOARGS)
{
  return bytes_unfreed;
}

#endif

#ifdef MEMORY_CHECK_POINTERS

#define MEMORY_REGISTRY_HT_SIZE 997

#include "registry.h"
#include "bool.h"
static Registry valid_blocks = NULL;
static int dont_check = 0;

static NORET mem_add_to_rgy( VOIDP temp , size_t bytes )
{
  dont_check = 1;
  assert(Registry_add(valid_blocks, temp, (VOIDP)bytes)==Bool_TRUE);
#ifdef MEMORY_LEAK_COUNT
  bytes_unfreed += bytes;
#endif
  dont_check = 0;
}

static NORET mem_remove_from_rgy( VOIDP temp )
{
  long32 num_bytes;

  dont_check = 1;
  num_bytes = (long32)Registry_get(valid_blocks, temp);
  assert(num_bytes != 0);
#ifdef MEMORY_LEAK_COUNT
  bytes_unfreed -= num_bytes;
#endif
  assert(Registry_remove(valid_blocks, temp));
  dont_check = 0;
}

static int mem_rgy_initialized(NOARGS)
{
  if (valid_blocks == (Registry)NULL) {
    dont_check = 1;
    valid_blocks = Registry_create(Registry_ptrcmp,
				   Registry_ptrhash);
    Registry_size_hint(valid_blocks, (unsigned int)MEMORY_REGISTRY_HT_SIZE);
    dont_check = 0;
  }
  return 1;
}

#endif

VOIDP Memory_allocate( size_t n )
{
  VOIDP temp;

  temp = (VOIDP)malloc(n);

#ifdef MEMORY_CHECK_POINTERS
  assert(dont_check ||mem_rgy_initialized());
#endif


  if (temp == NULL) {
    fprintf(stderr, "Fatal error allocating %d bytes", (int)n);
    abort();
  }
#ifdef MEMORY_CHECK_POINTERS
  assert(dont_check || (mem_add_to_rgy(temp,n),1));
#endif
  return temp;
}

VOIDP Memory_reallocate( VOIDP ptr, size_t n )
{
  VOIDP temp;

#ifdef MEMORY_CHECK_POINTERS
  assert(dont_check ||mem_rgy_initialized());
  assert(dont_check ||(mem_remove_from_rgy(ptr),1));
#endif

  temp = (VOIDP)realloc(ptr, n);
  if (temp == NULL) {
    fprintf(stderr, "Fatal error allocating %d bytes", (int)n);
    abort();
  }
#ifdef MEMORY_CHECK_POINTERS
  assert(dont_check ||(mem_add_to_rgy(temp,n),1));
#endif
  return temp;
}

NORET Memory_free( VOIDP ptr )
{
#ifdef MEMORY_CHECK_POINTERS
  assert(dont_check ||mem_rgy_initialized());
  assert(dont_check ||(mem_remove_from_rgy(ptr),1));
#endif

  if (ptr != NULL)
    free(ptr);
}
