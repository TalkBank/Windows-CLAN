/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***************************************************
 * tokens.h                                        * 
 *                                                 *
 * VOCD Version 1.0, August 1999                   *
 * G T McKee, The University of Reading, UK.       *
 ***************************************************/
#ifndef __VOCD_TOKENS__
#define __VOCD_TOKENS__

#include "vocdefs.h"
#include "wstring.h"


#define NO_SUBNODES  8   /* starting value       */

struct NODE;

typedef struct node {
   char         ch;      /* char stored at node   */
   struct NODE *child;   /* sub-tree - next char  */
} node;

typedef struct NODE {
   int    used;          /* no of subnodes used   */
   int    avail;         /* no of subnodes free   */
   node  *subs;          /* array of subnodes     */
} NODE;


int   tokens_freadlist      (FNType *fname, NODE *root);
int   tokens_member_simple  (char * word, NODE * root);
int   tokens_member         (char * word, NODE * root);
NODE* tokens_create_node    (int no_subnodes );
void  tokens_free_tree      (NODE *n);
int   tokens_insert         (NODE * root, const char *word );
void  tokens_print_tree     (NODE *n, char *title );


#endif
