/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* drules.c - includes:  */
/* read_drules */
/* apply_drules */

#include "cu.h"
#include "morph.h"

extern long mem_ctr;

/* read_drules */
/* read through drules file */
/* quit if incorrect syntax */
/* or run out of space to store rules */
BOOL
read_drules(FILE *rulefile) {
	extern DRULE_PTR drule_list;
	extern int drules_linenum;
	DRULE_PTR cur_rule;
	CRULE_COND_PTR choose_conds = NULL;
	CRULE_COND_PTR prev_conds = NULL;
	CRULE_COND_PTR next_conds = NULL;
	char line[MAXLINE];
	char keyword[MAX_WORD];
	char *line_ptr;
	int lines_read;

	drules_linenum = lines_read = 0;
	cur_rule = drule_list;

	/* main loop - read through drules file */
	while (get_line(line,MAXLINE,rulefile,&lines_read) != EOF) {  
		/* keep reading in lines from file */
		drules_linenum += lines_read;
		line_ptr = line;

//		print_drules(drule_list);     // development debug only
		if (DEBUG_CLAN & 32) {
			fprintf(debug_fp,"processing drules line# %d:  %s\n",drules_linenum,line);
		}
		line_ptr = get_word(line,keyword);  

		if (strncmp(keyword,"RULENAME",8) == 0) { /* rule header */
			/* get rulename */
			line_ptr = get_word(line_ptr,keyword);  /* keyword = rulename */
			/* set up new rule record */
			if (cur_rule != NULL) {  /* add this rule to end of list */
				if ((cur_rule->next_rule = m_drule_node(keyword)) != NULL)
					cur_rule = cur_rule->next_rule;
				else
					mor_mem_error(FAIL,"can't malloc space for drules, quitting");
			} else { /* set up first drule in list */
				if ((drule_list = m_drule_node(keyword)) != NULL)
					cur_rule = drule_list;
				else
					mor_mem_error(FAIL,"can't malloc space for drules, quitting");
			}
		} else if (strncmp(keyword,"choose",6) == 0) {  /* choose stmts header */
			/* set up choose_list record and pointer */
			if ((cur_rule->choose_list = m_crule_cond_rec()) != NULL)
				choose_conds = cur_rule->choose_list;
			else
				mor_mem_error(FAIL,"can't malloc space for drules, quitting");
		} else if (strncmp(keyword,"when",4) == 0) {  /* when stmts header */
			if ((cur_rule->prev_cond_list = m_crule_cond_rec()) != NULL)
				prev_conds = cur_rule->prev_cond_list;
			else
				mor_mem_error(FAIL,"can't malloc space for drules, quitting");
			if ((cur_rule->next_cond_list = m_crule_cond_rec()) != NULL)
				next_conds = cur_rule->next_cond_list;
			else
				mor_mem_error(FAIL,"can't malloc space for drules, quitting");
		} else if ((strcmp(keyword,"CURSURF") == 0) || (strcmp(keyword,"CURCAT") == 0)) {
			if (get_drule_conds(keyword,line_ptr,choose_conds) == FAIL) {
				fprintf(stderr,"*ERROR* in drules file at line %d\n",drules_linenum);
				return(FAIL);
			}
		} else if ((strcmp(keyword,"PREVSURF") == 0) || (strcmp(keyword,"PREVCAT") == 0)) {
			if (get_drule_conds(keyword,line_ptr,prev_conds) == FAIL) {
				fprintf(stderr,"*ERROR* in drules file at line %d\n",drules_linenum);
				return(FAIL);
			}
		} else if ((strcmp(keyword,"NEXTSURF") == 0) || (strcmp(keyword,"NEXTCAT") == 0)) {
			if (get_drule_conds(keyword,line_ptr,next_conds) == FAIL) {
				fprintf(stderr,"*ERROR* in drules file at line %d\n",drules_linenum);
				return(FAIL);
			}
		} else { /* hope we're looking at some variable declarations */
			if (get_var_decl(keyword,line_ptr) == 0) {
				fprintf(stderr,"*ERROR* in drules file at line %d\n",drules_linenum);
				return(FAIL);
			}
		}
	} /* end while */
	drules_linenum += lines_read;
	return(SUCCEED);
}

BOOL
get_drule_conds(STRING *cond_name, STRING *line, CRULE_COND_PTR cond_ptr) {
	extern int drules_linenum;
	int  hyphenI;
	CRULE_COND_PTR cond_tmp;
	STRING *line_ptr;
	char pattern[MAX_PATLEN];
	FEATTYPE features[MAXCAT];
	int cond_type;

	/* housekeeping - set up pointers */
	line_ptr = line;
	cond_tmp = cond_ptr;   /* get to empty condition record in list */
	while((cond_tmp->cond_type != -1) && (cond_tmp->next_cond != NULL))
		cond_tmp = cond_tmp->next_cond;
	if ((cond_tmp->cond_type != -1) && (cond_tmp->next_cond == NULL)) {
		if ((cond_tmp->next_cond = m_crule_cond_rec()) == NULL)
			mor_mem_error(FAIL,"can't malloc space for drules, quitting");
		else
			cond_tmp = cond_tmp->next_cond;
	}
	if (strcmp(cond_name,"CURSURF") == 0)
		cond_type = CURSURF_CHK;
	else if (strcmp(cond_name,"CURCAT") == 0)
		cond_type = CURCAT_CHK;
	else if (strcmp(cond_name,"PREVSURF") == 0)
		cond_type = PREVSURF_CHK;
	else if (strcmp(cond_name,"PREVCAT") == 0)
		cond_type = PREVCAT_CHK;
	else if (strcmp(cond_name,"NEXTSURF") == 0)
		cond_type = NEXTSURF_CHK;
	else if (strcmp(cond_name,"NEXTCAT") == 0)
		cond_type = NEXTCAT_CHK;
	else
		cond_type = 0;


	/* long clause to process statements accordingly */
	if ((cond_type == CURSURF_CHK) || (cond_type == PREVSURF_CHK) || (cond_type == NEXTSURF_CHK)) {
		line_ptr = skip_equals(line_ptr);
		if (*line_ptr == EOS) {
			fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",drules_linenum);
			return(FAIL);
		}

// 2019-07-30 change dash to 0x2013
		for (hyphenI=0; line_ptr[hyphenI] != EOS; hyphenI++) {
			if (line_ptr[hyphenI] == '-') {
//fprintf(stderr, "%s\n", line_ptr);
				uS.shiftright(line_ptr+hyphenI, 2);
				line_ptr[hyphenI++] = 0xE2;
				line_ptr[hyphenI++] = 0x80;
				line_ptr[hyphenI++] = 0x93;
			}
		}

		/* check if we want negation of pattern */
		if (*line_ptr == '!') {
			cond_type += NOT;
			++line_ptr;
			if (*line_ptr == EOS) {
				fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",drules_linenum);
				return(FAIL);
			}
		}
		/* rest of line contains surface pattern - process & store */
		if (expand_pat(line_ptr,pattern) == FAIL)
			return(FAIL);
		else {
			cond_tmp->op_surf = (STRING *) malloc((size_t) (sizeof(char) * (strlen(pattern)+1)));
			mem_ctr++;
			if (cond_tmp->op_surf!= NULL) {
				cond_tmp->cond_type = cond_type;
				strcpy(cond_tmp->op_surf,pattern);
			} else
				mor_mem_error(FAIL,"can't malloc space for drules, quitting");
		}
	}  else { /* process (series of) checks on features */
		line_ptr = skip_equals(line_ptr);
		if (*line_ptr == EOS) {
			fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",drules_linenum);
			return(FAIL);
		}
		/* rest of line contains conditions on features */

		while ((line_ptr != NULL) && (*line_ptr != EOS)) {
			if (*line_ptr == '!'){   /* NOT */
				cond_type += NOT;
				++line_ptr;
			}
			line_ptr = get_feats(line_ptr,features);
			if (line_ptr != NULL) {  /* enter condition into condition list */
				if (cond_tmp->cond_type != -1) {  /* time for new cond rec */
					if ((cond_tmp->next_cond = m_crule_cond_rec()) == NULL)
						mor_mem_error(FAIL,"can't malloc space for drules, quitting");
					else
						cond_tmp = cond_tmp->next_cond;
				}
				cond_tmp->cond_type = cond_type;
				cond_tmp->op_cat = (FEATTYPE *) malloc((size_t) (sizeof(FEATTYPE) * (featlen(features)+1)));
				mem_ctr++;
				if (cond_tmp->op_cat!= NULL) {
					featcpy(cond_tmp->op_cat,features);
				} else
					mor_mem_error(FAIL,"can't malloc space for drules, quitting");
				if ((line_ptr = strchr(line_ptr,',')) != NULL) { /* more conditions? */
					++line_ptr; /* skip comma */
					while ((*line_ptr == ' ') || (*line_ptr == '\t')) /* skip whitespace */
						++line_ptr;
				}
			} else {
				fprintf(stderr,"*ERROR* unexpected feature found\n");
				fprintf(stderr,"*ERROR* found at line %d.\nALLOCAT%s\n",drules_linenum,line);
				return(FAIL);
			}
		}
	}
	return(SUCCEED);
}


/* ************************************************** */
/*   APPLY DRULES !!                                  */
/* ************************************************** */
BOOL
apply_drules(
     STRING *surf,
     FEATTYPE *cat,
     RESULT_REC_PTR prev_stack,
     int num_prev,
     RESULT_REC_PTR next_stack,
     int num_next)
{
	extern DRULE_PTR drule_list;
	DRULE_PTR cur_rule;
	CRULE_COND_PTR cur_cond;
	int i;
	BOOL res;
	char ln_cat[MAX_WORD];
	char lop_cat[MAX_WORD];
	STRING *p_surf;
	FEATTYPE *p_cat;
	STRING *p_stem;
	STRING *n_surf;
	FEATTYPE *n_cat;
	STRING *n_stem;

	char cat_tmp[MAXCAT];
	char word_tmp[MAX_WORD];
	char ts[256];

	strcpy(cat_tmp,"");
	strcpy(word_tmp,"");

	if (DEBUG_CLAN & 8) {
		fprintf(debug_fp,"\napplying drules\n");
		fprintf(debug_fp," surf: %s\n",surf);
		if (featlen(cat) > 0) {
			fs_decomp(cat,cat_tmp);
			fprintf(debug_fp," cat: %s\n",cat_tmp);
		} else
			fprintf(debug_fp," cat:\n");
	}

	cur_rule = drule_list;
 rule_loop:
	while (cur_rule != NULL) {
		if (DEBUG_CLAN & 8) {
			fprintf(debug_fp,"\ntrying rule %s ... \n",cur_rule->name);
		}
		cur_cond = cur_rule->choose_list;
		while (cur_cond != NULL) {  /* apply conditions on current entry */
			if (DEBUG_CLAN & 8) {
				decode_cond_code(cur_cond->cond_type,word_tmp);
				fprintf(debug_fp,"  condition = %s\n",word_tmp);
			}
			switch(cur_cond->cond_type) {
				case CURSURF_CHK :
					if (match_word(surf,cur_cond->op_surf,0) == SUCCEED){
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
						cur_cond = cur_cond->next_cond;
					} else {
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
						cur_rule = cur_rule->next_rule;
						goto rule_loop;
					}
					break;
				case CURSURF_NOT :
					if (match_word(surf,cur_cond->op_surf,0) == FAIL) {
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
						cur_cond = cur_cond->next_cond;
					} else {
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
						cur_rule = cur_rule->next_rule;
						goto rule_loop;
					}
					break;
				case CURCAT_CHK :
					if (check_fvp(cat,cur_cond->op_cat) == SUCCEED){
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
						cur_cond = cur_cond->next_cond;
					} else {
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
						cur_rule = cur_rule->next_rule;
						goto rule_loop;
					}
					break;
				case CURCAT_NOT :
					if (check_fvp(cat,cur_cond->op_cat) == FAIL){
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
						cur_cond = cur_cond->next_cond;
					} else {
						if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
						cur_rule = cur_rule->next_rule;
						goto rule_loop;
					}
					break;
				default: { /* shouldn't get here */
					;
				}
			}

		}/* end loop on all conditions */
		/* if we're here, we matched on all conditions for current entry */
		/* now make sure we can match previous and next categories */
		if (DEBUG_CLAN & 8) {
			fprintf(debug_fp,"checking conditions on previous word (if any)\n");
		}
		if ((num_prev == 0) && (cur_rule->prev_cond_list->cond_type != -1))
			return(FAIL);
		else if ((num_prev == 0) && (cur_rule->prev_cond_list->cond_type == -1))
			goto next_loop;
		else if ((num_prev > 0) && (cur_rule->prev_cond_list->cond_type != -1)) {
			for (i = num_prev; i > 0; i--) {
				p_surf = prev_stack[i].surface;
				p_cat = prev_stack[i].cat;
				p_stem = prev_stack[i].stem;  /* not using stem yet */
				cur_cond = cur_rule->prev_cond_list;
				while (cur_cond != NULL){
					if (DEBUG_CLAN & 8){
						decode_cond_code(cur_cond->cond_type,word_tmp);
						fprintf(debug_fp,"  condition = %s\n",word_tmp);
					}
					switch(cur_cond->cond_type) {
						case PREVSURF_CHK :
							if (match_word(p_surf,cur_cond->op_surf,0) == SUCCEED) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_prev_loop;
							}
							break;
						case PREVSURF_NOT :
							if (match_word(p_surf,cur_cond->op_surf,0) == FAIL) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_prev_loop;
							}
							break;
						case PREVCAT_CHK :
							if (check_fvp(p_cat,cur_cond->op_cat) == SUCCEED) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_prev_loop;
							}
							break;
						case PREVCAT_NOT :
							if (check_fvp(p_cat,cur_cond->op_cat) == FAIL) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_prev_loop;
							}
							break;
						default: /* shouldn't get here */
							; /* fall through */
					} /* close switch */	  
				}/* end while loop over prev_cond_list */
				if (cur_cond == NULL)
					goto next_loop;  /* matched all conditions-  jump out of for loop */
bottom_prev_loop:
				;
			} /* bottom of for loop */
			/* if we're here, no matches on any words in prev_list */
			cur_rule = cur_rule->next_rule;
			goto rule_loop;
		}  /* matches if ((num_prev > 0) && (cur_rule->prev_cond_list != NULL)) */

next_loop:
		if (DEBUG_CLAN & 8) {
			fprintf(debug_fp,"checking conditions on next word (if any)\n");
		}
		/* no further checks */
		if (cur_rule->next_cond_list->cond_type == -1) {
			return(SUCCEED);
		} else if ((num_next == 0) && (cur_rule->next_cond_list->cond_type != -1)) {
			return(FAIL);
		} else if ((num_next > 0) && (cur_rule->next_cond_list->cond_type != -1)) {
			for (i = num_next; i > 0; i--) {
				n_surf = next_stack[i].surface;
				n_cat = next_stack[i].cat;
				n_stem = next_stack[i].stem;  /* not using stem yet */
				cur_cond = cur_rule->next_cond_list;
				while (cur_cond != NULL) {
					if (DEBUG_CLAN & 8) {
						decode_cond_code(cur_cond->cond_type,word_tmp);
						fprintf(debug_fp,"  condition = %s\n",word_tmp);
					}
					switch(cur_cond->cond_type) {
						case NEXTSURF_CHK :
							if (match_word(n_surf,cur_cond->op_surf,0) == SUCCEED) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_next_loop;
							}
							break;
						case NEXTSURF_NOT :
							if (match_word(n_surf,cur_cond->op_surf,0) == FAIL) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_next_loop;
							}
							break;
						case NEXTCAT_CHK :
							if (check_fvp(n_cat,cur_cond->op_cat) == SUCCEED) {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
								cur_cond = cur_cond->next_cond;
							} else {
								res = false;
								strcpy(ts, "[end]");
								if (get_vname(cur_cond->op_cat,ts,lop_cat)) {
									strcpy(ts, "[end]");
									if (get_vname(n_cat,ts,ln_cat)) {
										if (ln_cat[0] != EOS && lop_cat[0] != EOS) {
											if (uS.patmat(ln_cat, lop_cat))
												res = true;
										}
									}
								}
								if (res) {
									if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
									cur_cond = cur_cond->next_cond;
								} else {
									if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
									goto bottom_next_loop;
								}
							}
							break;
						case NEXTCAT_NOT :
							if (check_fvp(n_cat,cur_cond->op_cat) == FAIL){
								res = true;
								strcpy(ts, "[end]");
								if (get_vname(cur_cond->op_cat,ts,lop_cat)) {
									strcpy(ts, "[end]");
									if (get_vname(n_cat,ts,ln_cat)) {
										if (ln_cat[0] != EOS && lop_cat[0] != EOS) {
											if (uS.patmat(ln_cat, lop_cat))
												res = false;
										}
									}
								}
								if (res) {
									if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition is met\n");
									cur_cond = cur_cond->next_cond;
								} else {
									if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
									goto bottom_next_loop;
								}
							} else {
								if (DEBUG_CLAN & 8) fprintf(debug_fp,"  condition failed\n");
								goto bottom_next_loop;
							}
							break;
						default: /* shouldn't get here */
							; /* fall through */
					} /* close switch */

				}/* end while loop over next_cond_list */
				if (cur_cond == NULL)
				/* matched all conditions */
					return(SUCCEED);
bottom_next_loop:
				;
			} /* bottom of for loop */
		}  /* matches if ((num_next > 0) && (next_cond_list != NULL)) */

		cur_rule = cur_rule->next_rule;  /* keep going through rules */
	}
  return(FAIL);
}

/* ************************************************** */

/* m_drule_node: malloc new rule node & initialize structure
   copy rulename into name slot, set all ptrs to NULL
   returns NULL if can't allocate enough space for node */
DRULE_PTR
m_drule_node(STRING *rulename) {
  DRULE_PTR new_node;

  new_node = NULL;
  new_node = (DRULE_PTR) malloc((size_t) sizeof(DRULE));
  mem_ctr++;
  if (new_node != NULL)
    {
      new_node->name =  (STRING *) malloc((size_t) (sizeof(char) * (strlen(rulename)+1)));
      mem_ctr++;
      if (new_node->name != NULL)
	strcpy(new_node->name,rulename);
      else
	return(NULL);
      new_node->choose_list = NULL;
      new_node->prev_cond_list = NULL;
      new_node->next_cond_list = NULL;
      new_node->next_rule = NULL;

      return(new_node);
    }
  else
    return(NULL);
}

/* ******************** FREE STRUCTURES ********************* */

/* free_drules: free up drule_list */
/* recurse down list */
/* free on way out of recursion stack */
void free_drules(DRULE_PTR rule_p) {
	if (rule_p != NULL){
		free_drules(rule_p->next_rule);
		if (rule_p->name != NULL) free(rule_p->name);
		mem_ctr--;
		free_crule_cond(rule_p->choose_list);
		free_crule_cond(rule_p->prev_cond_list);
		free_crule_cond(rule_p->next_cond_list);
		free(rule_p);
		mem_ctr--;
	}
}

/* ******************** PRINT STRUCTURES ********************* */

/* print drules */
void
print_drules(DRULE_PTR rulelist) {
  DRULE_PTR drule_tmp;
  CRULE_COND_PTR drule_cond_tmp;
  char word_tmp[MAX_WORD];
  int clause_ctr;
  fprintf(debug_fp,"\n*** print drules ***\n");

  drule_tmp = rulelist;
  while (drule_tmp != NULL){
    clause_ctr = 0;
    fprintf(debug_fp,"rulename: %s\n",drule_tmp->name);
    drule_cond_tmp = drule_tmp->choose_list;
    fprintf(debug_fp,"  choose\n");
    while (drule_cond_tmp != NULL){
      decode_cond_code(drule_cond_tmp->cond_type,word_tmp);
      fprintf(debug_fp,"  condition type = %s\n",word_tmp);
	  if (drule_cond_tmp->cond_type == CURSURF_CHK) {
		  if (drule_cond_tmp->op_surf != NULL)
		  	fprintf(debug_fp,"  condition operand 1 = %s\n",drule_cond_tmp->op_surf);
	  } else {
		  if (drule_cond_tmp->op_cat != NULL) {
			  fs_decomp(drule_cond_tmp->op_cat,word_tmp);
			  fprintf(debug_fp,"  condition operand 1 = %s\n",word_tmp);
		  }
	  }
	  drule_cond_tmp = drule_cond_tmp->next_cond;
    }
    fprintf(debug_fp,"  when\n");
    drule_cond_tmp = drule_tmp->prev_cond_list;
    while ((drule_cond_tmp != NULL) &&
	   (drule_cond_tmp->cond_type != -1)){
      decode_cond_code(drule_cond_tmp->cond_type,word_tmp);
      fprintf(debug_fp,"  condition type = %s\n",word_tmp);
	  if ((drule_cond_tmp->cond_type == PREVSURF_CHK) ||
	    (drule_cond_tmp->cond_type == PREVSURF_NOT)){
	    if (drule_cond_tmp->op_surf != NULL)
	  		fprintf(debug_fp,"  condition operand 1 = %s\n",drule_cond_tmp->op_surf);
	  } else {
		if (drule_cond_tmp->op_cat != NULL) {
		  fs_decomp(drule_cond_tmp->op_cat,word_tmp);
		  fprintf(debug_fp,"  condition operand 1 = %s\n",word_tmp);
	    }
	  }
      drule_cond_tmp = drule_cond_tmp->next_cond;
    }
    drule_cond_tmp = drule_tmp->next_cond_list;
    while ((drule_cond_tmp != NULL) &&
	   (drule_cond_tmp->cond_type != -1)){
      decode_cond_code(drule_cond_tmp->cond_type,word_tmp);
      fprintf(debug_fp,"  condition type = %s\n",word_tmp);
	  if ((drule_cond_tmp->cond_type == NEXTSURF_CHK) ||
	    (drule_cond_tmp->cond_type == NEXTSURF_NOT)){
	    if (drule_cond_tmp->op_surf != NULL)
	  		fprintf(debug_fp,"  condition operand 1 = %s\n",drule_cond_tmp->op_surf);
	  } else {
		if (drule_cond_tmp->op_cat != NULL) {
		  fs_decomp(drule_cond_tmp->op_cat,word_tmp);
	 	  fprintf(debug_fp,"  condition operand 1 = %s\n",word_tmp);
	 	}
	  }
      drule_cond_tmp = drule_cond_tmp->next_cond;
    }
    drule_tmp = drule_tmp->next_rule;
  }
}
