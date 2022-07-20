/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

/* protyped (03/07/90) DC */

#include "cu.h"

#if !defined(UNX)
#define _main dist_main
#define call dist_call
#define getflag dist_getflag
#define init dist_init
#define usage dist_usage
#endif

#define pr_result dist_pr_result
#define IS_WIN_MODE FALSE
#include "mul.h" 

void free();

/* Routine declarations for the "tree" package.				    */

typedef char generic; /* generic * is just char *  ok? */

typedef generic *key_type;	/* The real key and data types are defined, */
typedef generic *data_type;	/* allocated and handled by the caller.	    */

typedef generic *tree_type;	/* A reference to a tree.		    */

/* The user program must define a key compare function and call "tinit"	    */
/* with the key compare function as an argument.			    */
/* The key compare routine is used to form a partial ordering of possible   */
/* keys.  It should return (-1, 0, 1) as indicated by the following code:   */
/*									    */
/*		int key_compare(key_type a, key_type b) {		    */
/*			if (*a < *b) return -1;				    */
/*			if (*a = *b) return 0;				    */
/*			if (*a > *b) return 1;				    */
/*		}							    */
/* Binary tree routines.  Written February 2, 1988 - Bruce Adams   */

/* This module implements a generic symmetric order binary tree handler.    */
/* The current implementation does simple binary trees.  No attempt is made */
/* to produce balanced trees.						    */

/* Note: "tdelete" has not been implemented yet.			    */

#define prdebug(x)	/* printf x */

typedef struct real_tree_struct *real_tree_type;
typedef struct node_struct *node_type;

typedef int (*func_ptr)(key_type, key_type);
struct real_tree_struct {
    node_type root;				/* Pointer to tree root.    */
} ;

struct node_struct {
    key_type key;		/* Pointer to the caller's key.		    */
    data_type data;		/* Pointer to caller's data.		    */
    node_type left;
    node_type right;
} ;

/* #define prdebug(x)	*/ /* printf x */

#define MAX_IDS		32
#define MAX_ID_LEN	8

static tree_type word_tree;		/* tree for counting.		    */
static int line_count;			/* current turn count, or line count*/

static char break_char;		/* user controlled flag.  (-bC)	    */
static int count_multiple;	/* user controlled flag.  (-g)	    */
static int break_only;		/* user controlled flag.  (-o)	    */
static int code_tiers_only;	/* user controlled flag.  (-m)	    */
static int weighted_averages;	/* user controlled flag.  (-w)	    */
static int no_main;		/* user controlled flag.  (-t*ANY)  */

enum kind_enum { simple, complex, first_part, last_part } ;
typedef enum kind_enum kind_type;

struct word_counts_struct {
    int occurrences;
    int first_seen;	/* line number */
    int last_seen;	/* line number */
    kind_type kind;
    char *word;
} ;

typedef struct word_counts_struct *word_counts;


#define max(a, b)	(((a) > (b)) ? (a) : (b))
#define min(a, b)	(((a) < (b)) ? (a) : (b))

#define null_string(s)	(*(s) == EOS)	/* Is this string empty?	    */

/* Create a new tree, and return a reference to it to the user.		    */

/* ******************** dist prototypes ********************** */
tree_type tnew(void);
static node_type find_node(real_tree_type tree, key_type key, node_type *cur);
data_type tfind(tree_type tree, key_type key);
int tadd(tree_type tree, key_type key, data_type data);
static void real_twalk(node_type node, void (*func) (key_type, data_type));
void twalk(tree_type tree, void (*func) (key_type, data_type));
void real_tgarbage (node_type node);
void tgarbage (tree_type tree);
void count_word(char *word, int line_count, kind_type kind);
char *lstrchr(char *s, int c);
void check_word(char *word, int line_count);
void dist(void);
void pr_data_line(char *key, word_counts data);
void pr_item(char *key, word_counts data);
void pr_result(void);
/* *********************************************************** */

tree_type tnew(void) {
    real_tree_type t = (real_tree_type) malloc(sizeof(struct real_tree_struct));

    if (t == NULL) {
	perror("Fatal error in \"tree.c\":'tnew'.  'malloc' failed");
	cutt_exit(1);
    }

    t->root = NULL;	/* The tree itself starts out empty.		    */

    return (tree_type) t;
}

/* Find the node in the tree with the given key.  NULL means no such node.  */
/* The return value is the node in the tree which is the parent of the	    */
/* found node, or if the key wasn't found, parent is the last node visited. */
static node_type find_node(real_tree_type tree,key_type key,node_type *cur) {
    int eq;
    node_type parent = NULL;
    prdebug(("find_node(-, '%s', -)\n", key));

    *cur = tree->root;		/* Start at the root.			    */

    while (*cur != NULL) {	/* While the current node exists.	    */

	prdebug(("looping... (*cur)->key = '%s'\n", (*cur)->key));

	eq = strcmp(key, (*cur)->key);

	if (eq < 0) {			/* key < cur->key		    */
	    parent = *cur;		/* Remember this node.		    */
	    *cur = (*cur)->left;	/* Move to left child.		    */
	} else if (eq > 0) {		/* key > cur->key		    */
	    parent = *cur;		/* Remember this node.		    */
	    *cur = (*cur)->right;	/* Move to right child.		    */
	} else {			/* key == cur->key		    */
	    return parent;
	}
    }
    
    return parent;		/* Didn't find key.  (That is *cur = NULL.) */
}

/* Return the data portion of a node for the user.			    */
data_type tfind(
		tree_type tree, /* The tree to look in.		    */
		key_type key)   /* The key to look for.		    */
{
    node_type node;

    find_node((real_tree_type) tree, key, &node);

    if (node == NULL) return NULL;	/* No such node.		    */

    return node->data;			/* Return data from requested node. */
}


/* Add a node to the tree for the user.					    */
/* ("data" may be "NULL".)	    */

int tadd(
	 tree_type tree,		/* The tree to add into.	    */
	 key_type key, 		/* The data for the new node	    */
	 data_type data) 	/* The key of the new node.	    */
{
    int eq;
    node_type node;
    node_type parent;

    parent = find_node((real_tree_type) tree, key, &node);
    if (node != NULL) return(TRUE);		/* node already exists	    */

	/* Create and initialize the new node.				    */
    node = (node_type) malloc(sizeof(struct node_struct));
    if (node == NULL) {
	perror("Fatal error in \"tree.c\":'tadd'.  'malloc' failed");
	cutt_exit(1);
    }
    node->key = key;
    node->data = data;
    node->left = NULL;
    node->right = NULL;

    if (parent == NULL) {			/* Must be an empty tree.   */
	((real_tree_type) tree)->root = node;	/* Make tree be this node.  */
	return(FALSE);				/* Successful return.	    */
    }
		/* splice the node into the tree */

    eq = strcmp(node->key, parent->key);

    if (eq < 0) {
	parent->left = node;
	return(FALSE);
    } else if (eq > 0) {
	parent->right = node;
	return(FALSE);
    }
	free(node);
    fprintf(stderr, "Fatal error in \"tree.c\":tadd.  Corrupt tree.  Same key in node?\n");
    cutt_exit(1);				/* Must be VERY confused!   */
    return(FALSE);
}


static void real_twalk(node_type node, void (*func)(key_type, data_type)) {

    if (node == NULL) return;		/* Avoid infinite recursion!	    */

    real_twalk(node->left, func);	/* Walk through "less than" nodes.  */
    ((func_ptr)(*func)) (node->key, node->data);	/* Call user function for this node.*/
    real_twalk(node->right, func);	/* Do "greater than" nodes.	    */
}


/* Walk the tree in "smallest" to "largest" order (as defined by the user's */
/* key compare function.)  Call the user supplied function at each node.    */
void twalk(tree_type tree, void (*func) (key_type, data_type)) {

    real_twalk(((real_tree_type) tree)->root, func);
}


void real_tgarbage (node_type node) {

    if (node == NULL) return;		/* Avoid infinite recursion!	    */

/* Garbage collect this node's subtrees first.				    */
    real_tgarbage(node->left);
    real_tgarbage(node->right);

    free(node->key);	/* for "key", call it.		    */
    free((generic *) node);		/* Free this node's memory.	    */
}


/* Garbage collect a tree and, optionally, all of the attached data.	    */
void tgarbage (tree_type tree) {

/* Garbage collect the contents of the tree.				    */
    real_tgarbage(((real_tree_type) tree)->root);

    free(tree);		/* Free the memory used by the tree header.	    */
}

void count_word(char *word, int line_count, kind_type kind) {
    word_counts p = (word_counts) tfind(word_tree, (key_type) word);

    prdebug(("count_word('%s', %d, -); p==NULL:%d\n", word, line_count, p==NULL));

    if (p != NULL) {			/* if this word has been seen before*/
/* If counting multiple occurrences or word not already seen on this line.   */
	if (count_multiple || p->last_seen < line_count) {
	    p->occurrences ++;			/* count the word	    */
	    p->last_seen = line_count;		/* reset "last_seen"	    */
	}
    } else {				/* haven't seen this word before    */
	/* create space for it - THIS IS A HACK! */
	p = (word_counts) malloc(sizeof(struct word_counts_struct));
	if (p == NULL) {
	    perror("Fatal error in 'count_word'.  'malloc' failed");
	    cutt_exit(1);			/* failure!  punt		    */
	}
	p->word = (char *)malloc(strlen(word)+1);
	if (p->word == NULL) {
	    perror("Fatal error in 'count_word'.  'malloc' failed");
	    free(p);
	    cutt_exit(1);			/* failure!  punt		    */
	}
	p->occurrences = 1;		/* have seen it once now	    */
	p->first_seen = line_count;	/* first seen here		    */
	p->last_seen = line_count;	/* last seen here		    */
	p->kind = kind;			/* save kind info		    */
	strcpy(p->word, word);		/* save the word itself (as key)    */
	if (tadd(word_tree, (key_type) p->word, (data_type) p)) {	/* add it to tree	    */
	    fprintf(stderr, "Fatal error - couldn't add word \"%s\" into tree!\n", word);
	    cutt_exit(1);			/* failure!  punt		    */
	}
    }
}

char *lstrchr(char *s, int c) {
    for (; *s != (char)c && *s; s++) ;
    if (*s == (char)c) return(s);
    else return(NULL);
}

void check_word(char *word, int line_count) {
    int len;
    char subword[BUFSIZ];
    char *break_point = lstrchr(word, break_char);

    prdebug(("check_word('%s', %d)\n", word, line_count));

    if (break_point == NULL) {			/* No break char in word.   */
	if (!break_only) {			/* If wanted,		    */
	    count_word(word, line_count, simple);	/* count this word. */
	}
    } else {					/* Found break char.	    */
	count_word(word, line_count, complex);	/* Count "foo:bar" as item. */

	if (break_char) {			/* If breakdown wanted...   */
	    *break_point = EOS;			/* Blast the break char.    */
	    strcpy(subword, word);		/* Copy out subword "foo".  */
	    *break_point = break_char;		/* Put break char back.	    */
	    len = strlen(subword);
	    if (len != 0) {			/* If subword is "", ignore.*/
		subword[++len] = break_char;	/* tack the break char on   */
		subword[++len] = ' ';		/* and a space.		    */
		subword[++len] = EOS;		/* and make it a string.    */
		count_word(subword, line_count, first_part);
	    }

	    strcpy(subword, break_point);	/* Copy out subword ":bar". */
	    if (!null_string(subword)) {	/* If subword is "", ignore.*/
		strcat(subword, " ");		/* Make it unique.	    */
		count_word(subword, line_count, last_part);
	    }
	}
    }
}


void dist() {
    int i;
    char word[BUFSIZ];

    prdebug(("dist()\n"));
    while (getwholeutter()) {
		if (!chatmode || no_main || *utterance->speaker == '*') {
			line_count++;
		}
		if (!chatmode || *utterance->speaker=='%' || (*utterance->speaker=='*' && !code_tiers_only)) {
			for (i=getword(utterance->speaker, uttline, word, NULL, 0); i; i=getword(utterance->speaker, uttline, word, NULL, i)) {
				check_word(word, line_count);
			}
		}
    }
}


void pr_data_line(char *key, word_counts data) {
    prdebug(("pr_data_line('%s', -)\n", key));
    fprintf(fpout, "%-20s%6d%9d",
	key,
	data->occurrences,
	data->first_seen);
/* Only print 'last_seen' and average distance if they are interesting.	    */
    if (!onlydata && data->occurrences == 1) {	/* boring		    */
	fprintf(fpout, "\n");
    } else {					/* interesting		    */
	fprintf(fpout, "%9d%15.4f\n",
	    data->last_seen,
	    (float) (data->last_seen - data->first_seen)
		/ max(1, data->occurrences));
    }
}


/* Print out a node.  No fancy stuff is done if 'onlydata' is true	    */
void pr_item(char *key, word_counts data) {
    static int line_skipped	= TRUE;
    static int have_first_part	= FALSE;
    static int group_count	= 0;
    static int real_group_count	= 0;
    static int total_occurrences	= 0;
    static int total_distance	= 0;
    static word_counts hold_first_part	= NULL;
    char str[BUFSIZ];

    prdebug(("pr_item('%s', -)\n", key));

    if (onlydata) {
	pr_data_line(key, data);
	return;
    }

    if (have_first_part && data->kind != complex) {	/* Print totals??   */
	if (group_count > 1) {				/* only when > 1    */
	    if (weighted_averages && real_group_count > 1) {
		strcpy(str, hold_first_part->word);
		strcat(str," (weighted average of distances)");
		fprintf(fpout, "%-49s%10.4f\n",
		    str,
		    (float) total_distance / max(1, total_occurrences));
	    }
	    strcpy(str, hold_first_part->word);
	    strcat(str, " (any)");
	    pr_data_line(str, hold_first_part);
	}
	if (!line_skipped) {
	    fprintf(fpout, "\n");			/* skip a line	    */
	    line_skipped = TRUE;
	}
	total_occurrences = 0;
	total_distance = 0;
	have_first_part = FALSE;
	group_count = 0;
	real_group_count = 0;
    }

    switch (data->kind) {
	case complex:
	    if (have_first_part) {
		group_count ++;
		if (data->last_seen - data->first_seen > 0) {
		    real_group_count ++;
		    total_occurrences += data->occurrences;
		    total_distance += data->last_seen - data->first_seen;
		}
	    }
	case simple:				/* NOTE: no "break" here.   */
	    pr_data_line(key, data);
	    line_skipped = FALSE;
	    break;

	case first_part:
	    hold_first_part = data;	/* This stuff is printed later.	    */
	    have_first_part = TRUE;
	    if (!line_skipped) {
		fprintf(fpout, "\n");			/* skip a line	    */
		line_skipped = TRUE;
	    }
	    break;

	case last_part:
/* Skip line at the beginning of the last part.				    */
	    if (!line_skipped) {
		fprintf(fpout, "\n");			/* skip a line	    */
		line_skipped = TRUE;
	    }
	    strcpy(str, key);
	    strcat(str, " (any)");
	    pr_data_line(str, data);
	    break;
    }
}


void pr_result(void) {
    static char format[64] = "%-17s%10s%9s%9s%14s\n";	/* constant	*/

    prdebug(("pr_result()\n"));

    if (!onlydata) {
		if (chatmode) {
			fprintf(fpout, "There were %d turns.\n\n\n", line_count);
		} else {
			fprintf(fpout, "There were %d lines.\n\n\n", line_count);
		}

		fprintf(fpout, format,
			"",
			"Occurrence",
			"First ",
			"Last  ",
			"Average ");

		fprintf(fpout, format,
			"Word",
			"Count",
			"Occurs",
			"Occurs",
			"Distance");

		fprintf(fpout, "-----------------------------------------------------------\n");
    }

	/* Print out the data collected in the tree.			    */
    twalk(word_tree, (void (*) (key_type, data_type)) pr_item);
}


void usage()			/* print proper usage and exit */
{
    printf("DIST computes the average distance between words or codes.\n");
    printf("Usage: dist [bC g o %s] filename(s)\n", mainflgs());
    printf("+bC: break apart words at the character C.\n");
    printf("+g : count only one occurrence of each word for each turn.\n");
    printf("+o : only consider words which contain the character C given in +bC.\n");
    mainusage(TRUE);
}


void getflag(char *f, char *f1, int *i) {
    char *tf;

    prdebug(("getflag('%s')\n", f));

    f++;
    switch(*f++) {
	case 'b':	/* Break down words which contain the given char.   */
	    tf = getfarg(f,f1,i);
	    if (*tf == EOS) {
	       fprintf(stderr,"Argument is expected with option \"%s\"\n",f-2);
	       cutt_exit(0);
	    }
	    break_char = *tf;
	    break;

	case 'o':	/* Only look at words which contain the -bX char. */
	    break_only = TRUE;
	    no_arg_option(f);
	    break;

	case 'g':	/* Count only one occurrence of each word per line. */
	    count_multiple = FALSE;
	    no_arg_option(f);
	    break;

	case 't':	/* Notice if any main utterance lines are stripped, */
	    tf = getfarg(f,f1,i);
	    if (*tf == EOS || *tf == '*') {
			no_main = TRUE;		/* Don't try to count utterances.   */
	    }
	    maingetflag(f-2,f1,i);
	    break;

	default:
	    maingetflag(f-2,f1,i);
	    break;
    }
}


void call() {

    prdebug(("call()\n"));

	currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);

    dist();

    if (!combinput) pr_result();
}


void init(char first) {
    if (first) {
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
	    addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		maininitwords();
		mor_initwords();
    } else
    	prdebug(("init()\n"));

    if (!combinput || first) {
		prdebug(("inter()\n"));
		/* Lazy garbage collection.  Only garbage collect if needed.	    */
		if (word_tree) {		/* If there is an old tree around,  */
			tgarbage(word_tree);	/* garbage collect it.	    */
		}
		/* Cast the standard "strcmp" as taking two "void *" args.	    */
		word_tree = tnew();
		line_count = 0;
    }
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    word_tree = NULL;
    break_char = EOS;		/* user controlled flag.  (-bC)	    */
    count_multiple = TRUE;	/* user controlled flag.  (-g)	    */
    break_only = FALSE;		/* user controlled flag.  (-o)	    */
    code_tiers_only = FALSE;	/* user controlled flag.  (-m)	    */
    weighted_averages = FALSE;	/* user controlled flag.  (-w)	    */
    no_main = FALSE;		/* user controlled flag.  (-t*ANY)  */
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = DIST;
    chatmode = CHAT_MODE;
    OnlydataLimit = 1;
    UttlineEqUtterance = TRUE;

    bmain(argc,argv,pr_result);

    if (word_tree) {			/* If there is a tree around,	    */
	tgarbage(word_tree);	/* garbage collect it.	    */
    }
}
