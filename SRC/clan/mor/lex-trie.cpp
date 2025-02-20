/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/*****************************************************************************
* SPACE EFFICIENT TRIE                                                       *
* This program handles the lexicon used by the morphological analyzer        *
*                                                                            *
* lex-trie.c - store lexicon in a trie structure                             *
* optimal arrangement of trie is series of binary trees                      *
* (each binary tree is height balanced according to AVL algorithm)           *
*                                                                            *
* process_lex_entry (was in buildlex.c)                                      *
* builds runtime lexicon from core lexicon using allo-rules                  *
* SPACE EFFICIENT TRIE                                                       *
*****************************************************************************/

#include "cu.h"
#include "morph.h"
#include <stdlib.h>

extern FILE *fpout;
extern long mem_ctr;
extern char w_temp_flag[];

ELIST_PTR w_temp_entry;

static char *getStem(char *e_ptr, char *stem, char *surface) {
	char *stem_start;
	int i, j, stem_len;

	stem_start = ++e_ptr;
	do {
		e_ptr = strchr(e_ptr,'"');
		if (e_ptr == NULL)
			break;
		else if (*(e_ptr-1) != '\\')
			break;
		e_ptr++;
	} while (1) ;
	if (e_ptr != NULL) {
		stem_len = e_ptr - stem_start;
		for (i=0, j=0; i < stem_len; i++) {
			if (isSpace(stem_start[i])) {
				fprintf(stderr, "\nlex-entry error: space found in \"stem\" specification for entry %s\n\n", surface);
				return(NULL);
			} else if (stem_start[i] != '\\' || stem_start[i+1] != '"') {
				stem[j++] = stem_start[i];
			}
		}
		stem[j] = EOS;
	} else {
		fprintf(stderr, "\nlex-entry error: bad stem specification for entry %s\n\n", surface);
		return(NULL);
	}
	e_ptr = skip_blanks(++e_ptr);
	return(e_ptr);
}

static char *getTranslation(char *e_ptr, char *trans, char *surface) {
	char *trans_start;
	int trans_len;

	trans_start = ++e_ptr;
	e_ptr = strchr(e_ptr,'=');
	if (e_ptr != NULL) {
		trans_len = e_ptr - trans_start;
		strncpy(trans,trans_start,trans_len);
		trans[trans_len] = EOS;
		for (trans_len=0; trans[trans_len] != EOS; trans_len++) {
			if (trans[trans_len] == ',')
				trans[trans_len] = '_';
		}
	} else {
		fprintf(stderr,	"\nlex-entry error: bad word translation specification for entry %s\n\n",	surface);
		return(NULL);
	}
	e_ptr = skip_blanks(++e_ptr);
	return(e_ptr);
}

static void copyStem2comp(char *comp, char *surface, char *stem) {
	int i;

	if (stem[0] != EOS)
		strcpy(comp, stem);
	else
		strcpy(comp, surface);
	i = 0;
	while (surface[i] != EOS) {
		if (surface[i] == '+')
			strcpy(surface+i, surface+i+1);
		else
			i++;
	}
}

/* parse entry into surface and cat(s) */
/* apply allo rules to each surface cat combo */
int
process_lex_entry(STRING *entry, char isComp, FNType *fname, long ln) {
	extern long rt_entry_ctr;
	extern char DEBUGWORD;
	extern RESULT_REC *result_list;

	char surface[MAX_WORD];
	char cat[MAX_WORD];
	char stem[MAX_WORD];
	char trans[MAX_WORD];
	char comp[MAX_WORD];

	CAT_COND_PTR cur_cond;
	FEATTYPE cat_comp[MAXCAT];
	char *e_ptr;
	int num_allos = 0, i, j;

	e_ptr = entry;
//  e_ptr = get_word(e_ptr,surface);
	if (e_ptr == NULL) {
		fprintf(stderr,"\n\n*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr, "lex-entry error: bad entry specified: %s\n", entry);
		return(-1);
	}
	e_ptr = get_lex_word(e_ptr,surface);
	if (e_ptr == NULL) {
		fprintf(stderr,"\n\n*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr, "lex-entry error: bad entry specified: %s\n", entry);
		return(-1);
	}
	e_ptr = skip_blanks(e_ptr);
	if (DEBUGWORD) {
		if (wdptr == NULL || exclude(surface))
			DEBUG_CLAN = DEBUG_CLAN | 256;
		else
			DEBUG_CLAN = DEBUG_CLAN & 0xfeff;
	}
	while(*e_ptr != EOS){  /* processing loop - for each cat entry */
		cat[0] = EOS;
		cat_comp[0] = EOS;
		stem[0] = EOS;
		trans[0] = EOS;
		comp[0] = EOS;
		e_ptr = get_cat(e_ptr,cat);
		if (e_ptr == NULL) {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
			fprintf(stderr, "lex-entry error: bad category specification for entry: %s\n", surface);
//			fprintf(stderr, "Plase make sure that entries appear in this order {[ ]} \" \" = =:\n\t%s\n\n", entry);
			return(-1);
		}
		e_ptr = skip_blanks(e_ptr);
		if (*e_ptr == '=') {
			if ((e_ptr=getTranslation(e_ptr, trans, surface)) == NULL) // get trans - strip off equals
				return(-1);
			if (*e_ptr == '"') {
				if ((e_ptr=getStem(e_ptr, stem, surface)) == NULL) // get stem - strip off quotes
					return(-1);
			}
		} else if (*e_ptr == '"') {
			if ((e_ptr=getStem(e_ptr, stem, surface)) == NULL) // get stem - strip off quotes
				return(-1);
			if (*e_ptr == '=') {
				if ((e_ptr=getTranslation(e_ptr, trans, surface)) == NULL) // get trans - strip off equals
					return(-1);
			}
		}
		if (isComp) {
			if (stem[0] != EOS && strchr(stem,'+') == NULL) {
				fprintf(stderr,"\n\n*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Warning: Item \"%s\" doesn't have compound symbol \"+\" for entry:\n", stem);
				fprintf(stderr, "%s\n\n", entry);
			}
			copyStem2comp(comp, surface, stem);
		}
		if (DEBUG_CLAN & 2 || DEBUG_CLAN & 256){
			fprintf(debug_fp,"\nlex-entry surface: %s        cat: %s\n",surface,cat);
		}
		if (fs_comp(cat,cat_comp) == FAIL){  /* convert cat to internal rep */
			fprintf(stderr,"\n");
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
			fprintf(stderr, "lex-entry error: bad category specification for entry: %s\n\n", surface);
			return(-1);
		}
		/* apply allo rules, stick words in lexicon */
		num_allos = apply_arules(surface,cat_comp,stem,trans,comp);
		if (num_allos > 0){  
			for (i = 0; i < num_allos; i++){	
				/* daignostic details */
				if (DEBUG_CLAN & 2 || DEBUG_CLAN & 256) {
					if (result_list[i].name != NULL)
						fprintf(debug_fp,"allo %d RULENAME: %s\n",i+1,result_list[i].name);
					else
						fprintf(debug_fp,"allo %d RULENAME: NULL NAME\n",i+1);
					if (result_list[i].clause_type != NULL) {
						cur_cond = result_list[i].clause_type->cat_cond;
						if (cur_cond != NULL) {
							fprintf(debug_fp,"allo %d LEXCAT: ",i+1);
							while (cur_cond != NULL) {
								fs_decomp(cur_cond->fvp,cat); /* make cat info readable */
								if (cat[0] == '{') {
									strcpy(cat, cat+1);
									j = strlen(cat) - 1;
									if (cat[j] == '}')
										cat[j] = EOS;
								}
								if (cur_cond->cond_type == LEXCAT_NOT) {
									fprintf(debug_fp, "!%s", cat);
								} else { /* LEXCAT_CHK */
									fprintf(debug_fp, "%s", cat);
								}
								cur_cond = cur_cond->next_cond;
								if (cur_cond != NULL)
									fprintf(debug_fp,", ");
							}
							putc('\n', debug_fp);
						}
					}
					fprintf(debug_fp,"allo %d surface: %s\n",i+1,result_list[i].surface);
					fs_decomp(result_list[i].cat,cat); /* make cat info readable */
					fprintf(debug_fp,"allo %d cat: %s\n",i+1,cat);
//2009-05-27		  fprintf(debug_fp,"allo %d stem: %s\n",i+1,result_list[i].stem);
//2009-05-27		  fprintf(debug_fp,"allo %d translation: %s\n",i+1,result_list[i].trans);
//2009-05-27		  fprintf(debug_fp,"allo %d compound: %s\n",i+1,result_list[i].comp);
					putc('\n', debug_fp);
				} else {
					if (rt_entry_ctr % 100L == 0){
						fprintf(stderr, "\rLoading lexicon: %ld  ", rt_entry_ctr);
						fflush(stderr);

					}
				}  /* end diag details */
				rt_entry_ctr++;
				enter_word(result_list[i].surface,  /* put allos into */
						   result_list[i].cat,      /* run-time lexicon */
						   result_list[i].stem,
						   result_list[i].trans,
						   result_list[i].comp, FALSE);
			}
		} else if (num_allos == 0){
			if (DEBUG_CLAN & 2 || DEBUG_CLAN & 256)
				fprintf(debug_fp,"\nno a_rules applied\n");
			else
				fprintf(stderr,"\nWARNING: no a_rules applied to: %s.\n\n", entry);
		} else {
			if (DEBUG_CLAN & 2 || DEBUG_CLAN & 256)
				fprintf(debug_fp,"\na_rules failed\n");
			else
				fprintf(stderr, "\nWARNING: no a_rules applied to: %s.\n\n", entry);
		}
	}
	return(num_allos);
}

void
delete_word(STRING *word,FEATTYPE *entry) {
	extern TRIE_PTR trie_root;
	TRIE_PTR cur_node, t_node, t2_node;
	int i, t_num_letters, temp_flag_cnt = 0;

	if (trie_root == NULL || *word == EOS) {
		return;
	}
	cur_node = trie_root;
	t_node = NULL;

	while(*word != EOS){
		t_num_letters = cur_node->num_letters;
		for (i = 0; i < cur_node->num_letters; i++) {
			if (cur_node->letters[i].letter == *word) {
				t2_node = cur_node;
				cur_node = cur_node->letters[i].t_node;
				if (w_temp_flag[temp_flag_cnt-1] && t_node != NULL) {
					free(t_node);
					t_node = NULL;
				}
				if (i == t2_node->num_letters-1 && w_temp_flag[temp_flag_cnt]) {
					t2_node->num_letters = t2_node->num_letters - 1;
					t_node = t2_node->letters[i].t_node;
				}
				break;
			}
		}
		if (i >= t_num_letters)
			return;
		word++;
		temp_flag_cnt++;
	}
	delete_entry(cur_node,entry);
	if (w_temp_flag[temp_flag_cnt-1] && t_node != NULL) {
		free(t_node);
	}
}

/* enter_word: add word into trie structure */
/* will exit program if can't allocate enough space for word */
/* int enter_word(word,entry,stem); */
void
enter_word(const STRING *word, FEATTYPE *entry, const STRING *stem, const STRING *trans, const STRING *comp, char isTempItem) {
	extern TRIE_PTR trie_root;
	const char *org_word;
	TRIE_PTR cur_node;
	int l_index, temp_flag_cnt = 0;

	org_word = word;
	if (trie_root == NULL) {
		if ((trie_root = m_trie_node()) == NULL){
			mor_mem_error(-2,"can't malloc enough space for lexicon, quitting");
		}
	}
	cur_node = trie_root;
	
	while(*word != EOS){
		if ((l_index = find_letter(cur_node,*word)) < 0){
			if ((l_index = add_letter(cur_node,*word)) < 0){
				mor_mem_error(-2,"can't malloc enough space for lexicon, quitting");
			} else if (isTempItem)
				w_temp_flag[temp_flag_cnt] = TRUE;
		} else if (isTempItem)
			w_temp_flag[temp_flag_cnt] = FALSE;
		cur_node = cur_node->letters[l_index].t_node;
		word++;
		temp_flag_cnt++;
	}
	add_entry(org_word,cur_node,entry,stem,trans,comp,isTempItem);
}

/* find_letter:  goes through letter array */
/* returns array index */
int
find_letter(TRIE_PTR cur_node, int letter) {
	int i;
	for (i = 0; i < cur_node->num_letters; i++) {
		if (cur_node->letters[i].letter == letter)
			return(i);
	}
	return(-1);
}

/* add_letter:  adds letter to letter array */
/* returns index of new entry */
int
add_letter(TRIE_PTR cur_node, int letter) {
	extern long letter_ctr;   /* number of letter nodes */
	int i;
	LETTER_PTR new_list;

	cur_node->num_letters = (cur_node->num_letters) + 1;
	letter_ctr++;

	new_list = (LETTER_REC *)malloc((size_t)(cur_node->num_letters*(size_t)sizeof(LETTER_REC)));
	mem_ctr++;
	if (new_list != NULL) {
		for (i=0; i < (cur_node->num_letters)-1; i++){
			new_list[i].letter = cur_node->letters[i].letter;
			new_list[i].t_node = cur_node->letters[i].t_node;
		}

		new_list[i].letter  = letter;
		if ((new_list[i].t_node = m_trie_node()) == NULL){
			mor_mem_error(-2,"can't malloc enough space for lexicon, quitting");
		}
	} else {
		i = 0;
		mor_mem_error(-2,"can't malloc enough space for lexicon, quitting");
	}
	if (cur_node->letters){
		free(cur_node->letters);
		//    mem_ctr--;
	}
	cur_node->letters = new_list;
	return(i);
}

void delete_entry(TRIE_PTR node, FEATTYPE *entry) {
	ELIST_PTR entry_p;
	ELIST_PTR prev_p;
	extern long trie_node_ctr;
	
	if (node->entries != NULL) {
		entry_p = node->entries;
		while (entry_p != NULL){
			prev_p = entry_p;
			entry_p = entry_p->next_elist;
			if (prev_p->entry) free(prev_p->entry);
			if (prev_p->stem)  free(prev_p->stem);
			if (prev_p->trans)  free(prev_p->trans);
			if (prev_p->comp)  free(prev_p->comp);
			free(prev_p);
		}
		trie_node_ctr--;
	}
	node->entries = w_temp_entry;
}

int       /* should be BOOL? */
add_entry(const char *org_word, TRIE_PTR node, FEATTYPE *entry, const STRING *stem, const STRING *trans, const STRING *comp, char isTempItem) {
	ELIST_PTR entry_p;
	ELIST_PTR prev_p;
	
	if (isTempItem) {
		w_temp_entry = node->entries;
		node->entries = NULL;
	}
	if (node->entries != NULL) /* already have an entry for this node */
	{
		entry_p = node->entries;
		prev_p = entry_p;
		while (entry_p != NULL){
			prev_p = entry_p;
			/* check if we've already got an entry here */
			if (featcmp(entry_p->entry,entry) == 0){
				if (entry_p->stem == NULL && (stem == NULL || strlen(stem) == 0) &&
					entry_p->trans == NULL && (trans == NULL || strlen(trans) == 0) &&
					entry_p->comp == NULL && (comp == NULL || strlen(comp) == 0))
					return(0);
				if (entry_p->stem != NULL && stem != NULL && strcmp(entry_p->stem,stem) == 0 &&
					entry_p->trans != NULL && trans != NULL && strcmp(entry_p->trans,trans) == 0 &&
					entry_p->comp != NULL && comp != NULL && strcmp(entry_p->comp,comp) == 0)
					return(0);
			}
/*
			if () {
				if (entry_p->stem != NULL && stem != NULL && !strcmp(entry_p->stem, stem)) {
					if ((entry_p->comp == NULL && comp != NULL && strlen(comp) > 0) ||
						(entry_p->comp != NULL && strlen(entry_p->comp) > 0) && (comp == NULL || strlen(comp) == 0)) {
						char cat1[MAXCAT];
						char cat2[MAXCAT];
						char isShowOrg;
						if (comp_fp == NULL) {
#ifndef UNX
							if (WD_Not_Eq_OD)
								strcpy(FileName1, od_dir);
							else
								strcpy(FileName1, wd_dir);
#else
							strcpy(FileName1, wd_dir);
#endif
							addFilename2Path(FileName1, "compounds.udx.cex");
							comp_fp = fopen(FileName1, "w");
#ifdef _MAC_CODE
							if (comp_fp != NULL)
								settyp(FileName1, 'TEXT', the_file_creator.out, FALSE);
#endif
						}
						if (comp_fp != NULL) {
							fs_decomp(entry_p->entry,cat1);
							fs_decomp(entry,cat2);
							fprintf(comp_fp, "*********************************************\n");
							isShowOrg = FALSE;
							if (entry_p->comp == NULL) {
								if (comp == NULL || strlen(comp) == 0)
									isShowOrg = !uS.mStricmp(entry_p->stem, stem);
								else
									isShowOrg = !uS.mStricmp(entry_p->stem, comp);
							} else {
								if (comp == NULL || strlen(comp) == 0)
									isShowOrg = !uS.mStricmp(entry_p->comp, stem);
								else
									isShowOrg = !uS.mStricmp(entry_p->comp, comp);
							}
							if (entry_p->comp == NULL)
								fprintf(comp_fp, "1= %s", entry_p->stem);
							else
								fprintf(comp_fp, "1= %s", entry_p->comp);
							if (isShowOrg)
								fprintf(comp_fp, " (%s)", org_word);
							fprintf(comp_fp, " %s", cat1);
							fprintf(comp_fp, " %s\n", (entry_p->trans != NULL ? entry_p->trans : ""));
							if (comp == NULL || strlen(comp) == 0)
								fprintf(comp_fp, "2= %s", stem);
							else
								fprintf(comp_fp, "2= %s", comp);
							if (isShowOrg)
								fprintf(comp_fp, " (%s)", org_word);
							fprintf(comp_fp, " %s", cat2);
							fprintf(comp_fp, " %s\n", (trans != NULL ? trans : ""));
						}
					}
				}
			}
 */
			entry_p = entry_p->next_elist;
		}
		/* if we're here it's not in the list */
		if ((prev_p->next_elist = m_entry_node(entry,stem,trans,comp)) == NULL)
			mor_mem_error(-2,"can't malloc space for lexical entry, quitting");
	}
	else {
		if ((node->entries = m_entry_node(entry,stem,trans,comp)) == NULL)
			mor_mem_error(-2,"can't malloc space for lexical entry, quitting");
	}
	return(0);
}

	
/* lookup:  find word in trie structure
   returns pointer to entry for that word or
   returns NULL if word is not found in trie */
ELIST_PTR 
lookup(STRING *word) {
  extern TRIE_PTR trie_root;
  TRIE_PTR trie_p;
  STRING *rest_p;
  int letter_idx;

  trie_p = trie_root;
  rest_p = word;

  while ((trie_p != NULL) && (*rest_p != EOS)){
    if ((letter_idx = find_letter(trie_p,*rest_p)) < 0)
      trie_p = NULL;
    else {  /* matched on a letter - see if we've got an entire morpheme */
      trie_p = trie_p->letters[letter_idx].t_node;
      rest_p++;
    }
  }
  if ((*rest_p == EOS) && (trie_p != NULL))
    return(trie_p->entries);
  else
    return(NULL);
}

/* m_letter_node: malloc new letter_rec node & initialize structure
   returns NULL if can't allocate enough space for node */
LETTER_PTR
m_letter_node(int letter) {
  LETTER_PTR new_node;
  extern long letter_ctr;

  new_node = NULL;
  new_node = (LETTER_PTR) malloc((size_t) sizeof(LETTER_REC));
  mem_ctr++;
  if (new_node != NULL)
    {
      new_node->letter = letter;
      new_node->t_node = NULL;

      letter_ctr++;
      return(new_node);
    }
  else
    return(NULL);
}

/* m_trie_node: malloc new trie node & initialize structure
   set letter field to letter, all other fields to NULL
   returns NULL if can't allocate enough space for node */
TRIE_PTR
m_trie_node()
{
  TRIE_PTR new_node;
  extern long trie_node_ctr;

  new_node = NULL;
  new_node = (TRIE_PTR) malloc((size_t) sizeof(TRIE_NODE));
  mem_ctr++;
  if (new_node != NULL)
    {
      new_node->num_letters = 0;
      new_node->letters = NULL;
      new_node->entries = NULL;

      trie_node_ctr++;
      return(new_node);
    }
  else
    return(NULL);
}

/* m_entry_node: malloc new entry list node & initialize structure
   malloc space to store entry string, set ptr next_elist to NULL
   returns NULL if can't allocate enough space for entry */
ELIST_PTR 
m_entry_node(FEATTYPE *entry, const STRING *stem, const STRING *trans, const STRING *comp) {
	ELIST_PTR new_entry;
	extern long trie_entry_ctr;

	new_entry = NULL;
	new_entry = (ELIST_PTR) malloc((size_t)sizeof(ENTRY_LIST));
	mem_ctr++;
	if (new_entry != NULL)
    {
		new_entry->entry = NULL;
		new_entry->stem = NULL;
		new_entry->trans = NULL;
		new_entry->comp = NULL;
		
		new_entry->entry =  (FEATTYPE *)malloc((size_t) (sizeof(FEATTYPE) * (featlen(entry)+1)));
		mem_ctr++;
		trie_entry_ctr++;
		if (new_entry->entry != NULL){
			new_entry->next_elist = NULL;
			featcpy(new_entry->entry,entry);
			
			if (strlen(stem) != 0) {
				new_entry->stem = (STRING *)malloc((size_t) (sizeof(char) * (strlen(stem)+1)));
				mem_ctr++;
				if (new_entry->stem == NULL)
					return(NULL);
				strcpy(new_entry->stem,stem);
			}
			if (strlen(trans) != 0) {
				new_entry->trans = (STRING *)malloc((size_t) (sizeof(char) * (strlen(trans)+1)));
				mem_ctr++;
				if (new_entry->trans == NULL)
					return(NULL);
				strcpy(new_entry->trans,trans);
			}
			if (strlen(comp) != 0) {
				new_entry->comp = (STRING *)malloc((size_t) (sizeof(char) * (strlen(comp)+1)));
				mem_ctr++;
				if (new_entry->comp == NULL)
					return(NULL);
				strcpy(new_entry->comp,comp);
			}
			return(new_entry);
		}
		else
			return(NULL);
    }
	else
		return(NULL);
}

#define NUMLETT 35
/* free_entries:  free up space allocated for lexical entries */
ELIST_PTR free_entries(ELIST_PTR entry_list) {
	if (entry_list->next_elist != NULL)
		entry_list->next_elist = free_entries(entry_list->next_elist);
	if (entry_list->entry != NULL)
		free(entry_list->entry);
	mem_ctr--;
	if (entry_list->stem){
		free(entry_list->stem);
		mem_ctr--;
	}
	if (entry_list->trans){
		free(entry_list->trans);
		mem_ctr--;
	}
	if (entry_list->comp){
		free(entry_list->comp);
		mem_ctr--;
	}
	free(entry_list);
	mem_ctr--;
	return(NULL);
}		  

/* free_trie:  free up space allocated for letter trie */
void free_trie(TRIE_PTR root) {
	int i;

	if (root != NULL){
		if (root->entries != NULL)
			root->entries = free_entries(root->entries);
		for (i = 0; i < root->num_letters; i++){
			free_trie(root->letters[i].t_node);
		}
		if (root->letters){
			free(root->letters);
			mem_ctr--;
		}
		free(root);
		mem_ctr--;
	}
}

/* ************************************************************************* */
/* one function to maintain height-balanced binary trees using AVL algorithm */
/* ************************************************************************* */


/* access_feat: locate or add a feature to the feature tables
   return NULL if can't allocate space for a new node
   make sure that tree is balanced w/respect to left and right branches
   AVL algorithm - keep track of difference between height of subtrees
   when height differs by 2 rearrange nodes to rebalance tree */
FEAT_PTR
access_feat(FEAT_PTR *root_node_ptr, STRING *name) {
  FEAT_PTR ancstr, ante_ancstr, point, ante_point;
  FEAT_PTR new_node, a, b, c, tmp, subtree;  /* passel o' necessary pointers */
  int d = 0;  /* d keeps track of dirction of imbalance - left or right */
  a = b = c = tmp = subtree = NULL;
    
  /* find point of insertion */
  ancstr = point = *root_node_ptr;
  ante_ancstr = ante_point = NULL;

  while (point != NULL)
    { if (point->bf != 0)  /* bf != 0, beginning of unbalanced subtree */
	{ ancstr = point; 
	  ante_ancstr = ante_point; }
      if (strcmp(name,point->name) < 0)
	{ ante_point = point;
	  point = point->left; }
      else if (strcmp(name,point->name) > 0)
	{ ante_point = point;
	  point = point->right; }
      else 
	return(point);  /* feature is in feature table - return ptr to node */
    }

  /* feature not in table - insert, rebalance */
  if (!(new_node = m_feat_node(name))) /* make new node */
    { /* failure - m_feat_node returned NULL */
	  mor_mem_error(-2,"access_feat: can't malloc more space for feature table, quitting");
    }

  if (strcmp(name,ante_point->name) < 0) /* insert */
    ante_point->left = new_node;
  else
    ante_point->right = new_node;
    
  /* rebalance! (lots of bookkeeping to be done) */
  /* which subtree needs balanced? */
  a = ancstr;
  if (strcmp(name,a->name) < 0)
    { d = 1; 
      b = tmp = a->left; }
  else
    { d = -1;
      b = tmp = a->right; }

  /* mark intervening nodes as unbalanced too (max 1 node) */
  /* note b == tmp in first iteration */
 while (tmp != new_node)
    if (strcmp(name,tmp->name) < 0)
      { tmp->bf = 1; 
	tmp = tmp->left; }
    else
      { tmp->bf = -1; 
	tmp = tmp->right; }
  
  /* is tree unbalanced? */
  if (a->bf == 0) /* was a balanced tree */
    { a->bf = d;  /* now off by one, can't rebalance */
      return(new_node); }
  if ((a->bf + d) == 0)  /* was off by one */
    { a->bf = 0;     /* just got evened up, tree is fine */
      return(new_node); }

  /* tree is now off by 2 - time for rebalancing */
  if (d == 1) /* left imbalance */
    {
      if (b->bf == 1)  /* imbalance config  LL */
	{ /* fprintf(debug_fp,"rebalance config LL\n"); */ // debug
	  a->left = b->right; /* subtle? way of nulling out left child of a? */
	  b->right = a;     /* a moved into right node of b */
	  a->bf = 0;     /* every node is balanced */
	  b->bf = 0; 
	  subtree = b;  /* root of new subtree */
	}
      else  /* b type LR */
	{ /* LR rotation - seriously fancy pointers */
	  /* fprintf(debug_fp,"rebalance config LR\n"); */ // debug

	  c = b->right;  /* this node will become root of subtree */
	  /* rearrange children of a and b, sever appropriate links */
	  if (c->left != NULL)
	    b->right = c->left;  /* left child of c becomes right child of b */
	  else
	    b->right = NULL;
	  if (c->right != NULL)
	    a->left = c->right;  /* right child of c becomes left child of a */
	  else
	    a->left = NULL;
	  c->left = NULL;
	  c->right = NULL;
	  /* balance nodes around c */
	  c->left = b;   
	  c->right = a;

	  /* readjust balance factors on a,b,c  */
	  if (c->bf == 1)       /* c had left child - became right child of b */
	    { a->bf = -1;       /* a was balanced - lost left child */
	      b->bf = 0;}        /* b (remains) balanced */
	  else if (c->bf == -1) /* c had right child - became left child of a */
	    { a->bf = 0;        /* a remains balanced */
	      b->bf = 1;}         /* b lost right child */
	  else
	    { a->bf = 0;
	      b->bf = 0;}

	  c->bf = 0;      /* c is globally balanced */
	  subtree = c;    /* c is root of newly balanced subtree */
	}  /* end LR rotation */
    }
  else /* right imbalance */
    {
      if (b->bf == -1)  /* b type RR */
	{ /* RR rotation */
	  /* fprintf(debug_fp,"rebalance config RR\n"); */ // debug
	  a->right = b->left;  /* subtle? way of nulling out right child of a? */
	  b->left = a;
	  a->bf = 0;     /* every node is balanced */
	  b->bf = 0; 
	  subtree = b;  /* root of new subtree */
	}
      else  /* b type RL */
	{ /* RL rotation - seriously fancy pointers */
	  /* fprintf(debug_fp,"rebalance config RL\n"); */ // debug

	  c = b->left;  /* this node will become new subtree root */
	  /* rearrange children of a and b, sever appropriate links */
	  if (c->left != NULL)
	    a->right = c->left;  /* left child of c becomse right child of a */
	  else
	    a->right = NULL;
	  if (c->right != NULL)
	    b->left = c->right; /* right child of c becomse left child of b */
	  else
	    b->left = NULL;
	  c->left = NULL;
	  c->right = NULL;
	  /* balance nodes around c */
	  c->left = a;
	  c->right = b;   

	  /* readjust balance factors on a,b,c  */
	  if (c->bf == 1)        /* c had left child - became right child of a*/
	    { a->bf = 0;         /* a remains balanced */
	      b->bf = -1; }      /* b loses left child */
	  else if (c->bf == -1) /* c had right child - (became left child of b)*/
	    { a->bf = 1;        /* a loses right child - becomes unbalanced */
	      b->bf = 0; }      /* b (remains) balanced */
	  else     
	    { a->bf = 0;
	      b->bf = 0; }
	  c->bf = 0;      /* c is balanced */
	  subtree = c;    /* c is root of newly balanced subtree */
	}  /* end LR rotation */
    }

  /* insert balanced subtree into tree */
  if (ante_ancstr == NULL)
    *root_node_ptr = subtree;
  else if (ante_ancstr->left == ancstr)
    ante_ancstr->left = subtree;
  else if (ante_ancstr->right == ancstr)
    ante_ancstr->right = subtree;

  return(new_node);
}
