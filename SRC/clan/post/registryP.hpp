/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef _regyP_h_
#define _regyP_h_

#include "memory.hpp"
#include "sysdep.hpp"
#include "darray.hpp"

#include "registry.hpp"

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

/* private internal representation of the table */
typedef struct RegistryRecord_st {
  VOIDP name;
  VOIDP obj;
  struct RegistryRecord_st *next;
} RegistryRecord;

#define DEFAULT_HT_SIZE 97	/* This should be prime */

/* The Registry representation */
typedef struct Registry_st {
  unsigned int ht_size;
  RegistryRecord **hash_table;	/* First record of directory */
  Registry_CompareFunc comp_fun; /* Comparison function */
  Registry_HashFunc hash_fun;	/* Hash function */
  unsigned int record_count;	/* Number of records in the registry */
} Registry_rep;

/* private traversal routine used to implement Registry_fetch_contents() */
static NORET add_to_darrays(VOIDP, VOIDP, VOIDP);

/* used when calling add_to_darrays() within Registry_fetch_contents() */
struct darray_pair {
  Darray key_darray;
  Darray value_darray;
};

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

#define raise(p_to_rep) ((Registry)p_to_rep)
#define lower(obj) ((Registry_rep *)obj)
#define create() ((Registry_rep *) Memory_allocate(sizeof (Registry_rep)))
#define destroy(p_to_rep) (Memory_free((VOIDP)p_to_rep))

#endif
