/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* allo.c - includes:  */
/* read_arules */
/* apply_arules */

#include "cu.h"
#include "morph.h"

extern long mem_ctr;
extern ARULE_FILE_PTR ar_files_list;
extern RESULT_REC *result_list;

static int arules_linenum = 0;

BOOL
markPostARules(void) {
	char isFoundPostRule;
	ARULE_FILE_PTR c_ar_file, c_ar_file_tmp;
	ARULES_PTR ar_rules, rule_tmp;

	for (c_ar_file=ar_files_list; c_ar_file != NULL; c_ar_file=c_ar_file->next_file) {
		for (ar_rules=c_ar_file->arules_list; ar_rules != NULL; ar_rules=ar_rules->next_rule) {
			if (ar_rules->post_rule_name != NULL) {
				isFoundPostRule = FALSE;
				for (c_ar_file_tmp=ar_files_list; c_ar_file_tmp != NULL; c_ar_file_tmp=c_ar_file_tmp->next_file) {
					for (rule_tmp=c_ar_file_tmp->arules_list; rule_tmp != NULL; rule_tmp=rule_tmp->next_rule) {
						if (uS.mStricmp(rule_tmp->name, ar_rules->post_rule_name) == 0) {
							ar_rules->post_rule_ptr = rule_tmp;
//							rule_tmp->isPostRule = TRUE;
							isFoundPostRule = TRUE;
						}
					}
				}
				if (!isFoundPostRule) {
					fprintf(stderr,"\n*ERROR* Can't find rule \"%s\" referenced from \"POST:\" in RULENAME: %s\n\n", ar_rules->post_rule_name, ar_rules->name);
					return(FAIL);
				}
			}
		}
	}
	return(SUCCEED);
}

BOOL
read_arules(FILE *rulefile, FNType *fname) {
	ARULE_FILE_PTR c_ar_file, new_ar_file;
	ARULES_PTR cur_rule;
	ARULE_CLAUSE_PTR cur_clause;
	LEX_ENTRY_PTR cur_lex;
	ALLO_ENTRY_PTR cur_allo;
	char line[MAXLINE];
	char *line_ptr;
	char keyword[MAX_WORD];
	char isPostRuleFile;
	FNType mFileName[FNSize];
	int lines_read;

	if (ar_files_list == NULL) {
		ar_files_list = (ARULE_FILE_PTR) malloc((size_t) sizeof(ARULE_FILE));
		if (ar_files_list == NULL)
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		c_ar_file = ar_files_list;
		c_ar_file->next_file = NULL;
	} else {
		for (c_ar_file=ar_files_list; c_ar_file->next_file != NULL && c_ar_file->next_file->isDefault == FALSE; c_ar_file=c_ar_file->next_file) ;
		new_ar_file = (ARULE_FILE_PTR) malloc((size_t) sizeof(ARULE_FILE));
		if (new_ar_file == NULL)
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		if (ar_files_list->isDefault == TRUE) {
			new_ar_file->next_file = ar_files_list;
			ar_files_list = new_ar_file;
		} else {
			new_ar_file->next_file = c_ar_file->next_file;
			c_ar_file->next_file = new_ar_file;
		}
		c_ar_file = new_ar_file;
	}
//	strcpy(c_ar_file->arfname, fname);
	extractFileName(mFileName, fname);
	isPostRuleFile = (uS.mStricmp(mFileName, "post.cut") == 0);
	c_ar_file->isDefault = FALSE;
	c_ar_file->arules_list = NULL;
	mem_ctr++;
	arules_linenum = lines_read = 0;
	/* rulepackages statement ? */
	/* startrules statement ? */

	cur_rule = c_ar_file->arules_list;
	cur_clause = NULL;
	cur_lex = NULL;
	cur_allo = NULL;

	/* loop to read through arules file */
	while (get_line(line,MAXLINE,rulefile,&lines_read) != EOF) {  
		/* keep reading in lines from file */
		arules_linenum += lines_read;
		line_ptr = line;
		if (DEBUG_CLAN & 128) {
			fprintf(debug_fp,"processing arules line# %d: %s\n",arules_linenum,line);
		}
		line_ptr = get_word(line,keyword);  
		
		if (strncmp(keyword,"RULENAME",8) == 0){  /* start of new rule declaration */
			/* get rulename */
			line_ptr = get_word(line_ptr,keyword);  /* keyword = rulename */
			if (uS.mStricmp(keyword, "default") == 0)
				c_ar_file->isDefault = TRUE;
			/* set up new rule record */
			if (cur_rule != NULL){
				if ((cur_rule->next_rule = m_arule_node(keyword)) != NULL)
					cur_rule = cur_rule->next_rule;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			}
			else{ /* set up first arule in list */
				if ((c_ar_file->arules_list = m_arule_node(keyword)) != NULL)
					cur_rule = c_ar_file->arules_list;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			}
			if (isPostRuleFile || uS.mStrnicmp(cur_rule->name, "post_", 5) == 0)
				cur_rule->isPostRule = TRUE;
			/* set up clause_record as well */
			if ((cur_rule->clauses = m_clause_node()) != NULL)
				cur_clause = cur_rule->clauses;
			else
				mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			cur_lex = NULL;
			cur_allo = NULL;
		} /* end RULENAME: */
		
		else if (strcmp(keyword,"LEX-ENTRY:") == 0){  /* lex_entry header */
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_clause->lex != NULL){   /* this is not first clause in list */
				if ((cur_clause->next_clause = m_clause_node()) != NULL)
					cur_clause = cur_clause->next_clause;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			}
			/* set up lex_entry record and pointer */
			if ((cur_clause->lex = m_lex_node()) != NULL)
				cur_lex = cur_clause->lex;
			else
				mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		} /* end LEX-ENTRY: */
		
		else if (strcmp(keyword,"ALLO:") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_clause->allos == NULL) /* first allo entry for this clause */
				if ((cur_clause->allos = m_allo_node()) != NULL)
					cur_allo = cur_clause->allos;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			else /* next allo for this clause */
				if ((cur_allo->next_allo = m_allo_node()) != NULL)
					cur_allo = cur_allo->next_allo;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		} /* end ALLO: */
		
		else if (strncmp(keyword,"POST",4) == 0){
			if (cur_rule == NULL || cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			/* get post rule name */
			line_ptr = get_word(line_ptr,keyword);  /* keyword = rulename */
			cur_rule->post_rule_name = (STRING *) malloc((size_t) (sizeof(char) * (strlen(keyword)+1)));
			if (cur_rule->post_rule_name != NULL)
				strcpy(cur_rule->post_rule_name,keyword);
			else
				mor_mem_error(FAIL,"can't malloc space for arules post rule name, quitting");
		} /* end RULENAME: */

		else if (strcmp(keyword,"LEXSURF") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_lex == NULL) {
				strcpy(templineC, "entry \"LEX-ENTRY:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (get_lexsurf(line_ptr,cur_lex) == 0) {
				fprintf(stderr,"*ERROR* in arules file at line %d\n",arules_linenum);
				return(FAIL);
			}
		} else if (strcmp(keyword,"LEXCAT") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_lex == NULL) {
				strcpy(templineC, "entry \"LEX-ENTRY:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (get_lexcat(line_ptr,cur_lex) == 0)
				return(FAIL);
		} else if (strcmp(keyword,"ALLOSURF") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_lex == NULL) {
				strcpy(templineC, "entry \"LEX-ENTRY:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (cur_allo == NULL) {
				strcpy(templineC, "entry \"ALLO:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (get_allosurf(line_ptr,cur_lex,cur_allo) == 0)
				return(FAIL);
		} else if (strcmp(keyword,"ALLOCAT") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_allo == NULL) {
				strcpy(templineC, "entry \"ALLO:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (get_allocat(line_ptr,cur_allo) == 0)
				return(FAIL);
		} else if (strcmp(keyword,"ALLOSTEM") == 0){
			if (cur_clause == NULL)
				mor_mem_error(FAIL,"no RULENAME was specified");
			if (cur_allo == NULL) {
				strcpy(templineC, "entry \"ALLO:\" was not specified in RULENAME: ");
				strcat(templineC, cur_rule->name);
				mor_mem_error(FAIL,templineC);
			}
			if (get_allostem(line_ptr,cur_allo) == 0)
				return(FAIL);
		} else { /* variable declarations */
			if (get_var_decl(keyword,line_ptr) == 0)
			{ fprintf(stderr,"*ERROR* in arules file at line %d\n",arules_linenum);
			return(FAIL); }
		}
	} /* end while */
	return(SUCCEED);
}

BOOL
get_lexsurf(STRING *line, LEX_ENTRY_PTR lex_ptr) {
	STRING *line_ptr;
	char pattern[MAX_PATLEN];
	STRING *end_ptr;
	
	pattern[0] = EOS;
	
	line_ptr = line;
	line_ptr = skip_equals(line_ptr);
	if (*line_ptr == EOS)
    { fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
	return(FAIL); }
	
	/* check if want negation of pattern */
	if (*line_ptr == '!'){
		lex_ptr->cond_type = LEXSURF_NOT;
		++line_ptr;
		if (*line_ptr == EOS)
		{ fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
		return(FAIL); }
	}
	else
		lex_ptr->cond_type = LEXSURF_CHK;
	
	/* rest of line contains surface pattern - process & store */
	/* trim trailing blanks? */
	if ((end_ptr = strpbrk(line_ptr," \t\n")) == NULL)
		end_ptr = strchr(line_ptr,EOS);
	*end_ptr = EOS;
	if (expand_pat(line_ptr,pattern)){
		lex_ptr->surf_cond = 
		(char *) malloc((size_t) (sizeof(char) * (strlen(pattern)+1)));
		mem_ctr++;
		if (lex_ptr->surf_cond != NULL)
			strcpy(lex_ptr->surf_cond,pattern);
		else
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
	}
	else return(FAIL);
	
	return(SUCCEED);
}

BOOL
get_lexcat(STRING *line, LEX_ENTRY_PTR lex_ptr) {
	FEATTYPE features[MAX_WORD];
	STRING *line_ptr, *res;
	CAT_COND_PTR cur_cond;
	int cond_type = 0;
	
	cur_cond = NULL;
	features[0] = 0;
	line_ptr = line;
	line_ptr = skip_equals(line_ptr);
	if (*line_ptr == EOS)
    { fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
	return(FAIL); }
	
	/* rest of line contains conditions on category- get and store */
	while ((line_ptr != NULL) && (*line_ptr != EOS)){
		
		if (*line_ptr == '!'){   /* NOT */
			cond_type = LEXCAT_NOT;
			++line_ptr;
		}
		else
			cond_type = LEXCAT_CHK;
		
		line_ptr = get_feats(line_ptr,features);
		if (line_ptr != NULL){  /* enter condition into condition list */
			if (lex_ptr->cat_cond == NULL){
				if ((lex_ptr->cat_cond = m_cat_cond_rec(cond_type,features)) != NULL)
					cur_cond = lex_ptr->cat_cond;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			} else if (cur_cond != NULL) {
				if ((cur_cond->next_cond = m_cat_cond_rec(cond_type,features)) != NULL)
					cur_cond = cur_cond->next_cond;
				else
					mor_mem_error(FAIL,"can't malloc space for arules, quitting");
			}
			if ((res = strchr(line_ptr,',')) != NULL){ /* more conditions? */
				line_ptr = res;
				++line_ptr; /* skip comma */
				line_ptr = skip_blanks(line_ptr);
			} else {
				line_ptr = skip_blanks(line_ptr);
				if (*line_ptr != EOS) {
					fprintf(stderr,"*ERROR* Extra data found at line %d. Try using ',' separator or '%%':\n%s\n",arules_linenum,line_ptr);
					return(FAIL);
				}
			}
		}
	}
	return(SUCCEED);
}

BOOL
get_allosurf(
     STRING *line,
     LEX_ENTRY_PTR lex_ptr,
     ALLO_ENTRY_PTR allo_ptr) 
{
	STRING *line_ptr;
	STRING *end_ptr;
	char pattern[MAX_PATLEN];

	pattern[0] = EOS;
	line_ptr = line;
	line_ptr = skip_equals(line_ptr);
	if (*line_ptr == EOS) {
		fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
		return(FAIL);
	}

	/* trim trailing blanks? */
	if ((end_ptr = strpbrk(line_ptr," \t\n")) == NULL)
		end_ptr = strchr(line_ptr,EOS);
	*end_ptr = EOS;
	/* rest of line contains surface template - process & store */
	if (strncmp(line_ptr,"LEXSURF",7) == 0) {  /* copy lex_surf into record */
		if (lex_ptr->surf_cond != NULL) {
			allo_ptr->surf_template = (STRING *)malloc((size_t)(sizeof(char)*(strlen(lex_ptr->surf_cond)+1)));
			mem_ctr++;
			if (allo_ptr->surf_template != NULL)
				strcpy(allo_ptr->surf_template,lex_ptr->surf_cond);
			else
				mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		}
	} else if (expand_pat(line_ptr,pattern)) {
		allo_ptr->surf_template = (STRING *)malloc((size_t)(sizeof(char)*(strlen(pattern)+1)));
		mem_ctr++;
		if (allo_ptr->surf_template != NULL)
			strcpy(allo_ptr->surf_template,pattern);
		else
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
	} else {
		fprintf(stderr,"*ERROR* in arules file at line %d\n",arules_linenum);
		return(FAIL);
	}

	return(SUCCEED);
}

BOOL
get_allostem(STRING *line, ALLO_ENTRY_PTR allo_ptr) {
	STRING *line_ptr;
	STRING *end_ptr;
	char pattern[MAX_PATLEN];
	
	
	pattern[0] = EOS;
	line_ptr = line;
	line_ptr = skip_equals(line_ptr);
	if (*line_ptr == EOS)
    { fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
	return(FAIL); }
	
	/* trim trailing blanks? */
	if ((end_ptr = strpbrk(line_ptr," \t\n")) == NULL)
		end_ptr = strchr(line_ptr,EOS);
	*end_ptr = EOS;
	/* rest of line contains stem template - process & store */
	if (expand_pat(line_ptr,pattern)){
		allo_ptr->stem_template = (STRING *)malloc((size_t)(sizeof(char)*(strlen(pattern)+1)));
		mem_ctr++;
		if (allo_ptr->stem_template != NULL)
			strcpy(allo_ptr->stem_template,pattern);
		else
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
	}
	else
    { fprintf(stderr,"*ERROR* in arules file at line %d\n",arules_linenum);
	return(FAIL); }
	
	return(SUCCEED);
}

BOOL
get_allocat(STRING *line, ALLO_ENTRY_PTR allo_ptr) {
	STRING *line_ptr, *res;
	CAT_OP_PTR cur_op = NULL;
	char cOperator[MAX_WORD];
	FEATTYPE features[MAX_WORD];
	int op_type = 0;

	cOperator[0] = EOS;
	features[0] = 0;

	line_ptr = line;
	line_ptr = skip_equals(line_ptr);
	if (*line_ptr == EOS) {
		fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",arules_linenum);
		return(FAIL);
	}

  /* rest of line contains conditions on category- get and store */
	while ((line_ptr != NULL) && (*line_ptr != EOS)){
 		line_ptr = get_word(line_ptr,cOperator);     /* get operation */
		if (strncmp(cOperator,"LEXCAT",6) == 0) { 
			op_type = COPY_LEXCAT;
			features[0] = 0;
		} else if (strncmp(cOperator,"ADD",3) == 0) {
			op_type = ADD_CAT;
			line_ptr = skip_blanks(line_ptr);
			line_ptr = get_feats(line_ptr,features);
		} else if (strncmp(cOperator,"DEL",3) == 0) {
			op_type = DEL_CAT;
			line_ptr = skip_blanks(line_ptr);
			if ((line_ptr = get_feat_path(line_ptr,features)) == NULL)
				return(FAIL);
		} else if (strncmp(cOperator,"SET",3) == 0){
			op_type = SET_CAT;
			line_ptr = skip_blanks(line_ptr);
			line_ptr = get_feats(line_ptr,features);
		} else { /* shouldn't get here */
			fprintf(stderr,"*ERROR* in arules file at line %d\n",arules_linenum);
			return(FAIL);
		}
  
  /* set up pointer into rules */
		if (allo_ptr->cat_op == NULL) {
			if ((allo_ptr->cat_op = m_cat_op_rec(op_type,features)) != NULL)
				cur_op = allo_ptr->cat_op;
			else
				mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		} else if (cur_op == NULL) {
			fprintf(stderr,"*ERROR* found at line %d.\n%s\n",arules_linenum,line_ptr);
			fprintf(stderr,"Possibly missing line \"ALLO:\" above\n");
			return(FAIL);
		} else if ((cur_op->next_op = m_cat_op_rec(op_type,features)) != NULL) {
			cur_op = cur_op->next_op;
		} else {
			mor_mem_error(FAIL,"can't malloc space for arules, quitting");
		}
		if (line_ptr == NULL) {
			fprintf(stderr,"*ERROR* found at line %d.\nALLOCAT%s\n",arules_linenum,line);
			return(FAIL);
		} else if ((res = strchr(line_ptr,',')) != NULL) { /* more operations? */
			line_ptr = res;
			++line_ptr; /* skip comma */
			line_ptr = skip_blanks(line_ptr);
		} else {
			line_ptr = skip_blanks(line_ptr);
			if (*line_ptr != EOS) {
				fprintf(stderr,"*ERROR* Extra data found at line %d. Try using ',' separator or '%%':\n%s\n",arules_linenum,line_ptr);
				return(FAIL);
			}
		}
	}
	return(SUCCEED);
}

static int
apply_arules_post(int s, int e, ARULES_PTR rule_tmp) {
	int rt_index;
	STRING *surface;
	ARULE_CLAUSE_PTR clause_tmp;
	FEATTYPE *cat;
	STRING *stem;
	STRING *trans;
	STRING *comp;
	LEX_ENTRY_PTR lex_tmp;
	CAT_COND_PTR cur_cond;
	CAT_OP_PTR cur_op;
	ALLO_ENTRY_PTR allo_tmp;
	char allo_surface[MAX_WORD];
	FEATTYPE allo_cat[MAXLINE];
	char allo_stem[MAX_WORD];
	FEATTYPE fvp_tmp[MAXCAT];
	char isStemMatched, isAlreadyMatched;
	char cat_st[MAXCAT];

	if (rule_tmp == NULL)
		return(e);
	rt_index = e;
	for (; s < e; s++) {
		surface = result_list[s].surface;
		cat = result_list[s].cat;
		stem = result_list[s].stem;
		trans = result_list[s].trans;
		comp = result_list[s].comp;
		isAlreadyMatched = FALSE;
		clause_tmp = rule_tmp->clauses;
clause_loop:
		while (clause_tmp != NULL) {
			lex_tmp = clause_tmp->lex;
			/* check conditions on category first */
			if (lex_tmp->cat_cond != NULL) {
				cur_cond = lex_tmp->cat_cond;
				while (cur_cond != NULL) {
					if (cur_cond->cond_type == LEXCAT_NOT) {
						if (check_fvp(cat,cur_cond->fvp) == FAIL)
							cur_cond = cur_cond->next_cond;
						else
							break;
					} else { /* LEXCAT_CHK */
						if (check_fvp(cat,cur_cond->fvp) == SUCCEED)
							cur_cond = cur_cond->next_cond;
						else
							break;
					}
				}
				if (cur_cond != NULL) { /* didn't meet all conditions */
					clause_tmp = clause_tmp->next_clause; /* try next clause */
					goto clause_loop;
				}
			}
			/* check conditions on surface */
			//lxslxs UTF lxs
			if ((lex_tmp->surf_cond == NULL) ||
				((lex_tmp->cond_type == LEXSURF_CHK) && (match_word(surface,lex_tmp->surf_cond,0) == SUCCEED)) ||
				((lex_tmp->cond_type == LEXSURF_NOT) && (match_word(surface,lex_tmp->surf_cond,0) == FAIL))) {
				/* matched conditions of rule */
				/* apply rule */
				if (lex_tmp->surf_cond != NULL)
					record_match(surface);
				allo_tmp = clause_tmp->allos;
				while (allo_tmp != NULL) {
					/* precaution - clean out slot in stack for allo entry */
					result_list[rt_index].name = NULL;
					result_list[rt_index].clause_type = NULL;
					strcpy(result_list[rt_index].surface,"");
					result_list[rt_index].cat[0] = 0;
					strcpy(result_list[rt_index].stem,stem);  /* from lexicon on disk */
					strcpy(result_list[rt_index].trans,trans);
					strcpy(result_list[rt_index].comp,comp);


					if (lex_tmp->surf_cond != NULL && allo_tmp->surf_template != NULL) {
						if (strchr(lex_tmp->surf_cond, '\\') == NULL && strchr(allo_tmp->surf_template, '\\') != NULL) {
							fprintf(stderr, "\n\nERROR:  LEXSURF has no variable and ALLOSURF does\n");
							fprintf(stderr, "\tIn this case please make \"ALLOSURF\" = to either:\n");
							fprintf(stderr, "\t\t\"LEXSURF\" or string.\n");
							fprintf(stderr, "\t\tOr make \"LEXSURF\" = to the same variable as in \"ALLOSURF\".\n");
							fs_decomp(cat,cat_st);
							fprintf(stderr, "\tsurface=%s\n", surface);
							fprintf(stderr, "\tcat=%s\n", cat_st);
							fprintf(stderr, "\tstem=%s\n", stem);
							fprintf(stderr, "\ttrans=%s\n", trans);
							fprintf(stderr, "\tcomp=%s\n", comp);
							CleanUpAll(TRUE);
							CloseFiles();
							cutt_exit(0);
						}
					}


					/* process surface and stem simultaniously */
					strcpy(allo_surface,"");
					if (allo_tmp->surf_template == NULL){
						strcpy(result_list[rt_index].surface,surface);
					} else if (gen_word(allo_tmp->surf_template,allo_surface)) {
						strcpy(result_list[rt_index].surface,allo_surface);
						/* allo stem = lex surface, if different from allo surface */
						/* unless stem pattern is specified in the allo rule */
						if ((allo_tmp->stem_template == NULL) && (*stem == EOS)
							//							 && (strcmp(allo_surface,surface) != 0)
							)
							strcpy(result_list[rt_index].stem,surface);
					} else
						return(-1);  /* if couldn't generate surface-  will this happen? */
					/* process stem! */
					if (allo_tmp->stem_template != NULL){
						isStemMatched = FALSE;
						if (stem[0] != EOS && lex_tmp->surf_cond != NULL) {
							if (match_word(stem,lex_tmp->surf_cond,0) == SUCCEED) {
								record_match(stem);
								isStemMatched = TRUE;
							} else {
								match_word(surface,lex_tmp->surf_cond,0);
								record_match(surface);
							}
						}

						strcpy(allo_stem,"");
						if (gen_word(allo_tmp->stem_template,allo_stem)){
							strcpy(result_list[rt_index].stem,allo_stem);
						} else
							return(-1);  /* if couldn't generate surface-  will this happen? */

						if (isStemMatched) {
							match_word(surface,lex_tmp->surf_cond,0);
							record_match(surface);
						}
					}
					/* process cat */
					allo_cat[0] = 0;
					if (allo_tmp->cat_op == NULL) {  
						/* allo cat = lex cat */
						result_list[rt_index].name = rule_tmp->name;
						result_list[rt_index].clause_type = lex_tmp;
						featcpy(result_list[rt_index].cat,cat);
					} else { /* apply operations, in order, to cat */
						cur_op = allo_tmp->cat_op;
						while (cur_op != NULL) {
							if (cur_op->op_type == COPY_LEXCAT) {
								featcpy(allo_cat,cat);
							} else if (cur_op->op_type == ADD_CAT) {
								add_fvp(allo_cat,cur_op->fvp);
							} else if (cur_op->op_type == DEL_CAT) {
								if (retrieve_fvp(fvp_tmp,cur_op->fvp,allo_cat) != NULL)
									remove_fvp(allo_cat,fvp_tmp);
							} else if (cur_op->op_type == SET_CAT) {
								add_fvp(allo_cat,cur_op->fvp);
							}
							cur_op = cur_op->next_op;
						}
						result_list[rt_index].name = rule_tmp->name;
						result_list[rt_index].clause_type = lex_tmp;
						featcpy(result_list[rt_index].cat,allo_cat);
					}
					if (lex_tmp->surf_cond != NULL || !isAlreadyMatched) {
						if (++rt_index == MAX_PARSES) {
							fprintf(stderr, "warning-  too many allos generated from entry %s\n",surface);
							return(rt_index);
						}
					}
					allo_tmp = allo_tmp->next_allo;
				}
				//					return(rt_index); /* rule successfully been applied; just first match lxslxs lxs addition  */
				break;  /* Go through all the a-rules and not just 
						 first match as before; lxslxs lxs addition */
			} else /* keep going through rule_list looking to match condition */
				clause_tmp = clause_tmp->next_clause;
		}
	}
	return(rt_index);
}

/* apply_arules - takes an entry from core lexicon
   applies rules from a_rules list to generate
   0 or more entries for run-time lexicon */
/* returns number of allos generated, or -1 if problem found */
int
apply_arules(
    STRING *surface,
    FEATTYPE *cat,
    STRING *stem,
    STRING *trans,
	STRING *comp)
{
	int rt_index = 0, result_rule_s;  /* index into list of allos generated */
	ARULE_FILE_PTR c_ar_file;
	ARULES_PTR rule_tmp;
	ARULE_CLAUSE_PTR clause_tmp;
	LEX_ENTRY_PTR lex_tmp;
	CAT_COND_PTR cur_cond;
	CAT_OP_PTR cur_op;
	ALLO_ENTRY_PTR allo_tmp;
	char allo_surface[MAX_WORD];
	FEATTYPE allo_cat[MAXLINE];
	char allo_stem[MAX_WORD];
	FEATTYPE fvp_tmp[MAXCAT];
	char isPassOneDone, isStemMatched, isAlreadyMatched;
	char cat_st[MAXCAT];

	isAlreadyMatched = FALSE;
	for (c_ar_file=ar_files_list; c_ar_file != NULL; c_ar_file=c_ar_file->next_file) {
		/* walk through allo rule file - 
		if match surface & cat then generate allos */
		isPassOneDone = FALSE;
repeat_rules:
		rule_tmp = c_ar_file->arules_list;
		while (rule_tmp != NULL) {
			result_rule_s = rt_index;
			if (isPassOneDone) {
				if (rule_tmp->name[0] != '2' || rule_tmp->name[1] != '_')
					goto rule_loop;
			} else {
				if (rule_tmp->name[0] == '2' && rule_tmp->name[1] == '_')
					goto rule_loop;
			}
			if (rule_tmp->isPostRule == TRUE) {
				goto rule_loop;
			}
			clause_tmp = rule_tmp->clauses;
clause_loop:
			while (clause_tmp != NULL) {
				lex_tmp = clause_tmp->lex;
				/* check conditions on category first */
				if (lex_tmp->cat_cond != NULL) {
					cur_cond = lex_tmp->cat_cond;
					while (cur_cond != NULL) {
						if (cur_cond->cond_type == LEXCAT_NOT) {
							if (check_fvp(cat,cur_cond->fvp) == FAIL)
								cur_cond = cur_cond->next_cond;
							else
								break;
						} else { /* LEXCAT_CHK */
							if (check_fvp(cat,cur_cond->fvp) == SUCCEED)
								cur_cond = cur_cond->next_cond;
							else
								break;
						}
					}
					if (cur_cond != NULL) { /* didn't meet all conditions */
						clause_tmp = clause_tmp->next_clause; /* try next clause */
						goto clause_loop;
					}
				}
				/* check conditions on surface */
//lxslxs UTF lxs
				if ((lex_tmp->surf_cond == NULL) ||
					((lex_tmp->cond_type == LEXSURF_CHK) && (match_word(surface,lex_tmp->surf_cond,0) == SUCCEED)) ||
					((lex_tmp->cond_type == LEXSURF_NOT) && (match_word(surface,lex_tmp->surf_cond,0) == FAIL))) {
					/* matched conditions of rule */
					/* apply rule */
					if (lex_tmp->surf_cond != NULL)
						record_match(surface);
					allo_tmp = clause_tmp->allos;
					while (allo_tmp != NULL) {
						/* precaution - clean out slot in stack for allo entry */
						result_list[rt_index].name = NULL;
						result_list[rt_index].clause_type = NULL;
						strcpy(result_list[rt_index].surface,"");
						result_list[rt_index].cat[0] = 0;
						strcpy(result_list[rt_index].stem,stem);  /* from lexicon on disk */
						strcpy(result_list[rt_index].trans,trans);
						strcpy(result_list[rt_index].comp,comp);


						if (lex_tmp->surf_cond != NULL && allo_tmp->surf_template != NULL) {
							if (strchr(lex_tmp->surf_cond, '\\') == NULL && strchr(allo_tmp->surf_template, '\\') != NULL) {
								fprintf(stderr, "\n\nERROR:  LEXSURF has no variable and ALLOSURF does\n");
								fprintf(stderr, "\tIn this case please make \"ALLOSURF\" = to either:\n");
								fprintf(stderr, "\t\t\"LEXSURF\" or string.\n");
								fprintf(stderr, "\t\tOr make \"LEXSURF\" = to the same variable as in \"ALLOSURF\".\n");
								fs_decomp(cat,cat_st);
								fprintf(stderr, "\tsurface=%s\n", surface);
								fprintf(stderr, "\tcat=%s\n", cat_st);
								fprintf(stderr, "\tstem=%s\n", stem);
								fprintf(stderr, "\ttrans=%s\n", trans);
								fprintf(stderr, "\tcomp=%s\n", comp);
								CleanUpAll(TRUE);
								CloseFiles();
								cutt_exit(0);
							}
						}


						/* process surface and stem simultaniously */
						strcpy(allo_surface,"");
						if (allo_tmp->surf_template == NULL){
							strcpy(result_list[rt_index].surface,surface);
						} else if (gen_word(allo_tmp->surf_template,allo_surface)) {
							strcpy(result_list[rt_index].surface,allo_surface);
							/* allo stem = lex surface, if different from allo surface */
							/* unless stem pattern is specified in the allo rule */
							if ((allo_tmp->stem_template == NULL) && (*stem == EOS)
							//							 && (strcmp(allo_surface,surface) != 0)
												)
							strcpy(result_list[rt_index].stem,surface);
						} else
							return(-1);  /* if couldn't generate surface-  will this happen? */
						/* process stem! */
						if (allo_tmp->stem_template != NULL){
							isStemMatched = FALSE;
							if (stem[0] != EOS && lex_tmp->surf_cond != NULL) {
								if (match_word(stem,lex_tmp->surf_cond,0) == SUCCEED) {
									record_match(stem);
									isStemMatched = TRUE;
								} else {
									match_word(surface,lex_tmp->surf_cond,0);
									record_match(surface);
								}
							}

							strcpy(allo_stem,"");
							if (gen_word(allo_tmp->stem_template,allo_stem)){
								strcpy(result_list[rt_index].stem,allo_stem);
							} else
								return(-1);  /* if couldn't generate surface-  will this happen? */

							if (isStemMatched) {
								match_word(surface,lex_tmp->surf_cond,0);
								record_match(surface);
							}
						}
						/* process cat */
						allo_cat[0] = 0;
						if (allo_tmp->cat_op == NULL) {  
							/* allo cat = lex cat */
							result_list[rt_index].name = rule_tmp->name;
							result_list[rt_index].clause_type = lex_tmp;
							featcpy(result_list[rt_index].cat,cat);
						} else { /* apply operations, in order, to cat */
							cur_op = allo_tmp->cat_op;
							while (cur_op != NULL) {
								if (cur_op->op_type == COPY_LEXCAT) {
									featcpy(allo_cat,cat);
								} else if (cur_op->op_type == ADD_CAT) {
									add_fvp(allo_cat,cur_op->fvp);
								} else if (cur_op->op_type == DEL_CAT) {
									if (retrieve_fvp(fvp_tmp,cur_op->fvp,allo_cat) != NULL)
										remove_fvp(allo_cat,fvp_tmp);
								} else if (cur_op->op_type == SET_CAT) {
									add_fvp(allo_cat,cur_op->fvp);
								}
								cur_op = cur_op->next_op;
							}
							result_list[rt_index].name = rule_tmp->name;
							result_list[rt_index].clause_type = lex_tmp;
							featcpy(result_list[rt_index].cat,allo_cat);
						}
						if (lex_tmp->surf_cond != NULL || !isAlreadyMatched || isPassOneDone) {
							if (++rt_index == MAX_PARSES) {
								fprintf(stderr, "warning-  too many allos generated from entry %s\n",surface);
								return(rt_index);
							}
						}
						allo_tmp = allo_tmp->next_allo;
					}
//					return(rt_index); /* rule successfully been applied; just first match lxslxs lxs addition  */
					if (isPassOneDone == FALSE) {
						if (rule_tmp->post_rule_name != NULL && rule_tmp->post_rule_ptr != NULL && result_rule_s < rt_index) {
							rt_index = apply_arules_post(result_rule_s, rt_index, rule_tmp->post_rule_ptr);
							result_rule_s = rt_index;
						}
						isPassOneDone = TRUE;
						goto repeat_rules;
					} else	
						break;  /* Go through all the "2_" a-rules and not just 
								first match as before; lxslxs lxs addition */
				} else /* keep going through rule_list looking to match condition */
					clause_tmp = clause_tmp->next_clause;
			}
rule_loop:
			rule_tmp = rule_tmp->next_rule;
		}
		if (isPassOneDone == FALSE) {
			isPassOneDone = TRUE;
			goto repeat_rules;
		}
		if (rt_index > 0 && ar_files_list->next_file != NULL)
			isAlreadyMatched = TRUE;
	}
	return(rt_index);
}

/* m_rule_node: malloc new rule node & initialize structure
   copy rulename into name slot, set all ptrs to NULL
   returns NULL if can't allocate enough space for node */
ARULES_PTR m_arule_node(STRING *rulename) {
	ARULES_PTR new_node;

	new_node = NULL;
	new_node = (ARULES_PTR) malloc((size_t) sizeof(ARULES));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->name = (STRING *) malloc((size_t) (sizeof(char) * (strlen(rulename)+1)));
		mem_ctr++;
		if (new_node->name != NULL)
			strcpy(new_node->name,rulename);
		else
			return(NULL);
		new_node->next_rule = NULL;
		new_node->clauses = NULL;
		new_node->isPostRule = FALSE;
		new_node->post_rule_name = NULL;
		new_node->post_rule_ptr = NULL;
		return(new_node);
    } else
		return(NULL);
}

/* m_arule_clause_node: malloc new clause node & initialize structure
   set all ptrs to NULL
   returns NULL if can't allocate enough space for node */
ARULE_CLAUSE_PTR
m_clause_node()
{
	ARULE_CLAUSE_PTR new_node;

	new_node = NULL;
	new_node = (ARULE_CLAUSE_PTR) malloc((size_t) sizeof(ARULE_CLAUSE));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->lex = NULL;
		new_node->allos = NULL;
		new_node->next_clause = NULL;
		
		return(new_node);
    } else
		return(NULL);
}

/* m_lex_node: malloc new lex node & initialize structure
   set all ptrs to NULL
   returns NULL if can't allocate enough space for node */
LEX_ENTRY_PTR
m_lex_node()
{
	LEX_ENTRY_PTR new_node;

	new_node = NULL;
	new_node = (LEX_ENTRY_PTR) malloc((size_t) sizeof(LEX_ENTRY));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->cond_type = 0;
		new_node->surf_cond = NULL;
		new_node->cat_cond = NULL;
		return(new_node);
    } else
		return(NULL);
}

/* m_allo_node: malloc new allo node & initialize structure
   set all ptrs to NULL
   returns NULL if can't allocate enough space for node */
ALLO_ENTRY_PTR
m_allo_node()
{
	ALLO_ENTRY_PTR new_node;

	new_node = NULL;
	new_node = (ALLO_ENTRY_PTR) malloc((size_t) sizeof(ALLO_ENTRY));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->surf_template = NULL;
		new_node->stem_template = NULL;
		new_node->cat_op = NULL;
		new_node->next_allo = NULL;
		return(new_node);
    } else
		return(NULL);
}

/* m_cat_cond_rec:  malloc new cond node & intialize structure */
/* set pointers to NULL, enter cond string */
CAT_COND_PTR
m_cat_cond_rec(int cond_type, FEATTYPE *cond_fs) {
	CAT_COND_PTR new_node;

	new_node = NULL;
	new_node = (CAT_COND_PTR) malloc((size_t) sizeof(CAT_COND));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->cond_type = cond_type;
		new_node->fvp =  
		(FEATTYPE *) malloc((size_t)(sizeof(FEATTYPE) * (featlen(cond_fs)+1)));
		mem_ctr++;
		if (new_node->fvp != NULL)
			featcpy(new_node->fvp,cond_fs);
		else
			return(NULL);
		new_node->next_cond = NULL;
		return(new_node);
    } else
		return(NULL);
}

/* m_cat_op_rec:  malloc new op node & intialize structure */
/* set pointers to NULL, enter op string */
CAT_OP_PTR
m_cat_op_rec(int op_type, FEATTYPE *op_fs) {
	CAT_OP_PTR new_node;

	new_node = NULL;
	new_node = (CAT_OP_PTR) malloc((size_t) sizeof(CAT_OP));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->op_type = op_type;
		new_node->fvp = (FEATTYPE *) malloc((size_t) (sizeof(FEATTYPE) * (featlen(op_fs)+1)));
		mem_ctr++;
		if (new_node->fvp != NULL)
			featcpy(new_node->fvp,op_fs);
		else
			return(NULL);
		new_node->next_op = NULL;
		return(new_node);
    } else
		return(NULL);
}

/* free_arules: free up arules_list */
/* recurse down list */
/* free on way out of recursion stack */
void
free_ar_files(ARULE_FILE_PTR file_p) {
	if (file_p != NULL){
		free_ar_files(file_p->next_file);
		free_arules(file_p->arules_list);
		free(file_p);
		mem_ctr--;
	}
}

void
free_arules(ARULES_PTR rule_p) {
	if (rule_p != NULL){
		free_arules(rule_p->next_rule);
		if (rule_p->name != NULL)
			free(rule_p->name);
		if (rule_p->post_rule_name != NULL)
			free(rule_p->post_rule_name);
		mem_ctr--;
		free_arule_clauses(rule_p->clauses);
		free(rule_p);
		mem_ctr--;
	}
}

/* free_arule_clauses: free up clauses in a rule */
void free_arule_clauses(ARULE_CLAUSE_PTR clause_p) {
	if (clause_p != NULL){
		free_arule_clauses(clause_p->next_clause);
		free_lex(clause_p->lex);
		free_allos(clause_p->allos);
		free(clause_p);
		mem_ctr--;
	}
}

/* free_lex: free up lex_entry part of allo rule */
void free_lex(LEX_ENTRY_PTR lex_p) {
	if (lex_p != NULL){
		if (lex_p->surf_cond){
			free(lex_p->surf_cond);
			mem_ctr--;
		}
		free_cat_cond(lex_p->cat_cond);
		free(lex_p);
		mem_ctr--;
	}
}

/* free_cat_cond: free up condition list */
void free_cat_cond(CAT_COND_PTR cond_p) {
	if (cond_p != NULL){
		free_cat_cond(cond_p->next_cond);
		if (cond_p->fvp){
			free(cond_p->fvp);
			mem_ctr--;
		}
		free(cond_p);
		mem_ctr--;
	}
}

/* free_allos: free up allos list */
void free_allos(ALLO_ENTRY_PTR allo_p) {
	if (allo_p != NULL){
		free_allos(allo_p->next_allo);
		if (allo_p->surf_template){
			free(allo_p->surf_template);
			mem_ctr--;
		}
		if (allo_p->stem_template){
			free(allo_p->stem_template);
			mem_ctr--;
		}
		free_cat_op(allo_p->cat_op);
		free(allo_p);
		mem_ctr--;
	}
}

/* free_cat_op: free up operations list */
void free_cat_op(CAT_OP_PTR op_p) {
	if (op_p != NULL){
		free_cat_op(op_p->next_op);
		if (op_p->fvp){
			free(op_p->fvp);
			mem_ctr--;
		}
		free(op_p);
		mem_ctr--;
	}
}


/* print a_rules */
void print_arules(ARULE_FILE_PTR ar_files) {
	ARULES_PTR arule_tmp;
	ARULE_CLAUSE_PTR aclause_tmp;
	LEX_ENTRY_PTR lex_tmp;
	ALLO_ENTRY_PTR allo_tmp;
	CAT_COND_PTR cond_tmp;
	CAT_OP_PTR op_tmp;
	char word_tmp[MAX_WORD];
	int clause_ctr;

	while (ar_files != NULL) {
		arule_tmp = ar_files->arules_list;
		while (arule_tmp != NULL){
			fprintf(debug_fp,"rulename: %s\n",arule_tmp->name);
			clause_ctr = 0;
			aclause_tmp = arule_tmp->clauses;
			while (aclause_tmp != NULL){
				fprintf(debug_fp," clause %d\n",++clause_ctr);
				lex_tmp = aclause_tmp->lex;
				fprintf(debug_fp,"  lex-entry surface = %s\n",lex_tmp->surf_cond);
				cond_tmp = lex_tmp->cat_cond;
				while (cond_tmp != NULL){
					decode_cond_code(cond_tmp->cond_type,word_tmp);
					fprintf(debug_fp,"  cat cond type = %s\n",word_tmp);
					fs_decomp(cond_tmp->fvp,word_tmp);
					fprintf(debug_fp,"  feature specification = %s\n",word_tmp);
					cond_tmp = cond_tmp->next_cond;
				}
				allo_tmp = aclause_tmp->allos;
				while (allo_tmp != NULL){
					fprintf(debug_fp,"  allo surface = %s\n",allo_tmp->surf_template);
					op_tmp = allo_tmp->cat_op;
					while (op_tmp != NULL){
						decode_op_code(op_tmp->op_type,word_tmp);
						fprintf(debug_fp,"  cat operation type = %s\n",word_tmp);
						fs_decomp(op_tmp->fvp,word_tmp);
						fprintf(debug_fp,"  feature specification = %s\n",word_tmp);
						op_tmp = op_tmp->next_op;
					}
					fprintf(debug_fp,"  allo stem = %s\n",allo_tmp->stem_template);
					allo_tmp = allo_tmp->next_allo;
				}
				aclause_tmp = aclause_tmp->next_clause;
			}
			arule_tmp = arule_tmp->next_rule;
		}
		ar_files = ar_files->next_file;
	}
}
