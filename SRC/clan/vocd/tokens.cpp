/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * tokens.c                                                *
 * -  functions for managing tokens.                       *
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
#include "cu.h"
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "vocdefs.h"
#include "args.h"
#include "tokens.h"

static  int tokens_present    (int c, NODE *n);
static void tokens_print_word (NODE *n, char *w, int len );
#ifdef TOKENS_DEBUG
static void tokens_print_node (NODE *n ); /* useful for debugging */
#endif

/********************************************************
 * functions                                            *
 ********************************************************/

/*
 * tokens_freadlist:
 *
 * read a list of tokens from a file and enter into a
 * token tree.
 *
 * returns 0 on success.
 */

int tokens_freadlist(FNType * fname, NODE *root)
{
   FILE *fp;
   char pt[MAX_LINE];
   char ** argv;
   int argc;

   if (root == NULL) {
      fprintf(stderr,"tokens_freadlist: undefined list \n");
      return -1;
   }

#if defined(_MAC_CODE) || defined(_WIN32)
	FNType mFileName[FNSize];
// lxs-checkpc

	fp = OpenGenLib(fname,"r",TRUE,FALSE,mFileName);
	if (fp==NULL) {
		fprintf(stderr,"Cannot open either one of files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		return -1;
	}
#else
	fp = fopen(fname, "r");
	if (fp==NULL) {
		fprintf(stderr, "fopen: cannot open %s\n",fname);
		return -1;
	}
#endif
   while (1) {  /* read words */
      if (fgets(pt, MAX_LINE,fp)==NULL)
         break;

      if ((int) strlen(pt)>0) {
         /* parse the line */
         argc = parse_args(pt, &argv);

         while (argc--) {
            tokens_insert(root, *argv);
            argv++;
         }
      }
   }
   fclose(fp);
   return 0;
}

/*
 * tokens_create_node:
 *
 * create a new node, with no_subnodes subnodes
 *
 */
NODE * tokens_create_node( int no_subnodes )
{
   NODE *new_node;

   new_node = (NODE *) malloc (sizeof(NODE));
   if (new_node==NULL)
      return 0;
   memset(new_node, 0, sizeof(NODE));
   new_node->subs = (node *) malloc (sizeof(node)*no_subnodes);
   if (new_node->subs==NULL) {
      free(new_node);
      return 0;
   }
   memset(new_node->subs, 0, sizeof(node)*no_subnodes);
   new_node->used  = 0;
   new_node->avail = no_subnodes;
   return new_node;
}

/*
 * tokens_insert:
 *
 * insert a word into a list of words
 * returns -1 if fail, 0 otherwise;
 */
int tokens_insert ( NODE * root, const char *word )
{
   const char *pt;
   int i,j, len;
   NODE *r;
   node *parent = NULL;

   if (root == NULL) {
      fprintf(stderr,"undefined list \n");
      return -1;
   }
   if (word == NULL) {
      fprintf(stderr,"tokens_insert: null token\n");
      return -1;
   }
   r  = root;
   pt = word;
   len = strlen(word) + 1;
   for (j=0;j<len;j++)  {
      if (r==NULL) {     /* create a new child node */
         r = tokens_create_node( NO_SUBNODES );
         if (r==NULL)
            return -1;
         parent->child = r;
      }
      for (i=0;i<r->used;i++) {      /* first char present ? */
          if (r->subs[i].ch==*pt)
             break;
      }
      if (i==r->used) {   /* not present, add it */
         if (r->avail) {   /* space available */
            r->subs[i].ch = *pt;
            r->used++;
            r->avail--;
         }
         else { /* no space available */
            node *np;
            int newsize;

            newsize = ((r->used/NO_SUBNODES) + 1 ) * NO_SUBNODES;
            np = (node *) malloc (sizeof(node)*newsize);
            if (np==NULL) {
               fprintf(stderr, "no memory\n");
               return -1;
            }
            memset(np, 0, sizeof(node)*newsize);
            memcpy(np, r->subs, r->used*sizeof(node));
            free(r->subs);
            r->subs = np;
            r->avail = newsize - r->used;
            r->subs[i].ch = *pt;
            r->used++;
            r->avail--;
         }
      }
      parent = &r->subs[i];
      r = r->subs[i].child;
      pt++;
   }
   return 0;
}

/*
 * tokens_present:
 *
 * test for the presence of a char at a node.
 * returns -1 if fail, the location of the char otherwise;
 */
static int tokens_present(int c, NODE *n)
{
   int i;

   if (n==NULL)
      return -1;
   for (i=0; i<n->used; i++ )
      if (n->subs[i].ch == c)
         break;
   if (i == n->used)
      return -1;
   return i;
}


/*
 * tokens_member_simple:
 *
 * a simple character for character test for
 * membership - no wildcard matching
 * returns 1 if success, 0 if failure.
 */
int tokens_member_simple(char * word, NODE * root)
{
   int slen,i, pos;
   NODE *node;

   node = root;
   slen = strlen(word) + 1;
   pos = -1;
   for (i=0; i<slen; i++) {
      pos = tokens_present(word[i],node);
      if (pos==-1)
         return 0;
      node = node->subs[pos].child;
   }
   /* if get here, string has been matched */
   return 1;
}

/*
 * tokens_member:
 *
 * test for membership of a token in a word list.
 *
 * returns 1 if success, 0 if failure.
 */
int tokens_member(char * s, NODE * root)
{
	int i, j, slen;
	NODE *node;
	int pos;
	int filter = 0;
	int wild = 0, rep_wild = 0;
	int target, top=0;
	int k;
	/* MOR matching */
	int wild_index = -1;
	NODE *wild_node = NULL;

	if ( (s==NULL) || (root==NULL) )
		return 0;
	node = root;
	slen = strlen(s) + 1;
	for (k=0; k<root->used; k++) {
		target = root->subs[k].ch;
		if ( target==s[0] || target=='_' || target=='*' || target=='%' ) {
			node = root;
			pos=k; top=1;
			for (i=0;i<slen;i++) {
				if (!wild && !rep_wild) {
					if (!top)
						pos = tokens_present(s[i],node);
					if (pos != -1)
						if (s[i]!='*') /* don't literal match - for mor tier */
							if (node->subs[pos].ch==s[i])  {
								node = node->subs[pos].child;
								if (top) top=0;
								continue;
							}
					if (!top)
						pos = tokens_present('_',node);             /* '_' */
					if (pos != -1) {
						if ( node->subs[pos].ch=='_') {
							node = node->subs[pos].child;
							if (top) top=0;
							continue;
						}
					}
					if (!top)
						pos = tokens_present('*',node);             /* '*' */
					if (pos != -1) {
						if (node->subs[pos].ch=='*') {
							int fwd_pos;
							fwd_pos = tokens_present(s[i],node->subs[pos].child);
							if (fwd_pos != -1) {   /* 0 matched items */
								/* MOR matching */
								wild_index = i;
								wild_node = node->subs[pos].child;
								node = node->subs[pos].child->subs[fwd_pos].child;
								if (top) top=0;
								continue;
							} else {                 /* 1 or more matched items */
								node = node->subs[pos].child;  /* '*'subtree */
								wild++;
								if (top) top=0;
								continue;
							}
						}
					}
					if (!top)
						pos = tokens_present('%',node);
					if (pos != -1) {
						if ( node->subs[pos].ch=='%') {
							int fwd_pos;
							
							/* test for xx%x first */
							fwd_pos = tokens_present(s[i],node->subs[pos].child);
							if (fwd_pos != -1) {   /* 0 matched items */
								node = node->subs[pos].child->subs[fwd_pos].child;
								if (top) top=0;
								continue;
							} else if ((fwd_pos=tokens_present('%',node->subs[pos].child))!=-1) {
								/* %%, ?0 matched items */
								int fwd_pos2;
								
								node = node->subs[pos].child;
								fwd_pos2 = tokens_present(s[i],node->subs[fwd_pos].child);
								if (fwd_pos2 != -1) {   /* 0 matched items */
									node=node->subs[fwd_pos].child->subs[fwd_pos2].child;
									if (i>0) { s[i-1]='%'; filter++; }
									if (top) top=0;
									continue;
								} else {
									rep_wild++;
									s[i] = '%';
									if (i>0)
										s[i-1]='%';
									filter++;
									if (top) top=0;
									continue;
								}
							} else { /* one or more matched items */
								node = node->subs[pos].child;
								rep_wild++;
								s[i] = '%';
								filter++;
								if (top) top=0;
								continue;
							}
						}
					}
				}  /* if (!wild) */
				if (wild) {   /* * + one or more matched items */
					pos = tokens_present(s[i],node);
					if (pos != -1) {
						/* MOR matching */
						wild_index = i;
						wild_node = node;
						node = node->subs[pos].child;
						wild--;
					}
					continue;
				}
				if (rep_wild) {   /* % or %% + one or more matched items */
					pos = tokens_present(s[i],node);
					if (pos != -1) {
						node = node->subs[pos].child;
						filter++;
						rep_wild--;
						continue;
					}
					pos = tokens_present('%',node);
					if (pos != -1) {
						int fwd_pos;
						fwd_pos = tokens_present(s[i],node->subs[pos].child);
						if (fwd_pos != -1) {
							node = node->subs[pos].child->subs[fwd_pos].child;
							rep_wild--;
							continue;
						}
						s[i] = '%';
						filter++;
						continue;
					}
					s[i] = '%';
					continue;
				}
				/* MOR matching */
				if (i<slen) { /* match failed, can we redo a wildcard match */
					if (wild_node!=NULL && wild_index!=-1) {
						i=wild_index; /* NB: changing the value of loop index */
						node = wild_node;
						wild_node = NULL;
						wild = 1;
						wild_index = -1;
						continue; /* next time around loop, i = wild_index+1 */
						/* i.e. step over the previous matching char */
					}
				}
				break;
			} /* for */
			if (i<slen)           /* word not matched */
				continue;
			if (node != NULL)     /* pattern not completed */
				continue;
			if (!filter)
				return 1;
			else {
				for (i=0,j=0;i<slen+1;i++) {
					if (s[i]!='%')
						s[j++]=s[i];
				}
				return 1;
			}
		} /*  end outer if statement  */
	} /* end outer for */
	return 0;                  /* no top-level match */
}


#ifdef TOKENS_DEBUG
/*
 * tokens_print_node:
 *
 * print the contents of a node.
 *
 */
static void tokens_print_node (NODE * n)
{
   int i;

   printf("### NODE <%i, %i> : ", n->used, n->avail);
   if (n->used==0) {
      printf("empty\n");
      return;
   }

   for (i=0;i<n->used;i++) {
      printf("%c", n->subs[i].ch);
   }
   printf("\n");
}

#endif

/*
 * tokens_free_tree:
 * free all the elements of a tree, starting from the root node.
 *
 */

void tokens_free_tree ( NODE *n)
{
   int i;

	if (n != NULL) {
		for (i=0;i<n->used;i++) {
		  if (n->subs[i].child!=NULL )
		     tokens_free_tree(n->subs[i].child);
		}
		free(n->subs);
		free(n);
		n=NULL;
	}
}


/*
 * tokens_print_tree:
 *
 * print a tree, starting from the root node
 *
 */

void tokens_print_tree(NODE *n, char *title)
{
   int i;
   char word[64];

   if (n==NULL) {
      printf("WORD LIST(%s): EMPTY\n", title);
      return;
   }

   printf("WORD LIST(%s): ...\n",title);

   for (i=0;i<n->used; i++) {
      word[0]=n->subs[i].ch;
      tokens_print_word(n->subs[i].child,word,1);
   }
   printf("\nEND WORD LIST.\n");
}


/*
 * tokens_print_word:
 *
 * print a word from the word list.
 *
 */

static
void tokens_print_word ( NODE *n, char *w, int len)
{
   int i;
   if (n == NULL) {
      printf("%s ", w);
      return;
   }

   for (i=0;i<n->used;i++) {

      w[len]=n->subs[i].ch;

      if (n->subs[i].ch == '\0')
         printf("%s ", w);
      else {
         tokens_print_word(n->subs[i].child,w,len+1);
      }
   }

}
