/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* crules.c - includes:  */
/* read_crules */
/* apply_crules */

#include "cu.h"
#include "morph.h"

/* states used while reading through file of crules */
#define INIT_STMTS 0
#define COND_STMTS 1
#define OP_STMTS 2

extern void out_of_mem(void);
extern long mem_ctr;

/* read_crules */
/* read through crules file */
/* quit if incorrect syntax */
/* or run out of space to store rules */
BOOL
read_crules(FILE *rulefile) {
	extern CRULE_PTR crule_list;
	extern int crules_linenum;
	CRULE_PTR cur_rule;
	CRULE_CLAUSE_PTR cur_clause = NULL;
	CRULE_COND_PTR cur_cond = NULL;
	CRULE_OP_PTR cur_op = NULL;
	char line[MAXLINE];
	char keyword[MAX_WORD];
	char *line_ptr;
	int lines_read;
	int STATE = INIT_STMTS;

	crules_linenum = lines_read = 0;

	cur_rule = crule_list;

	/* main loop - read through crules file */
	while (get_line(line,MAXLINE,rulefile,&lines_read) != EOF) {  
		/* keep reading in lines from file */
		crules_linenum += lines_read;
		line_ptr = line;

		/* print_crules(crule_list);     */ // development debug only
		if (DEBUG_CLAN & 64) {
			fprintf(debug_fp,"processing crules line# %d:  %s\n",crules_linenum,line);
		}
		line_ptr = get_word(line,keyword);  

		if (strncmp(keyword,"RULENAME",8) == 0) { /* rule header */
			/* get rulename */
			STATE = INIT_STMTS;
			line_ptr = get_word(line_ptr,keyword);  /* keyword = rulename */
			/* set up new rule record */
			if (cur_rule != NULL) {  /* add this rule to end of list */
				if ((cur_rule->next_rule = m_crule_node(keyword)) != NULL)
					cur_rule = cur_rule->next_rule;
				else
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			} else { /* set up first crule in list */
				if ((crule_list = m_crule_node(keyword)) != NULL)
					cur_rule = crule_list;
				else
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			}
			/* set up clause_record as well */
			if ((cur_rule->clauses = m_crule_clause_node()) != NULL)
				cur_clause = cur_rule->clauses;
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");

			/* get next statement immediately - must be "ruletype" stmt */
			if (get_line(line,MAXLINE,rulefile,&lines_read) == EOF) {
				fprintf(stderr,"*ERROR* unexpected end of crules file\n");
				return(FAIL);
			} else {  /* process RULETYPE line */
				crules_linenum += lines_read;
				line_ptr = get_word(line,keyword);
				if (strncmp(keyword,"CTYPE",5) != 0) {
					fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
					return(FAIL);
				} else if ((line_ptr = get_word(line_ptr,keyword)) == NULL) {
					fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
					return(FAIL);
				} else {
					if (strcmp(keyword,"START") == 0) {
						cur_rule->rule_type = START_RULE;
						put_startrule(cur_rule);
					} else if (strcmp(keyword,"END") == 0) {
						cur_rule->rule_type = END_RULE;
						put_endrule(cur_rule);
//					} else if (strcmp(keyword,"DONT_ADD_NEXTSURF") == 0) {
//						cur_rule->rule_type = DONT_ADD_NEXTSURF;
					} else if (strlen(keyword) == 1) {
						cur_rule->rule_type = (int) keyword[0];
					} else {  /* ruletype no good */
						fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
						return(FAIL);
					}
				}
			}/* end processing of "ruletype" stmt */
		} else if (strncmp(keyword,"if",2) == 0) {  /* condition stmts header */
			STATE = COND_STMTS;
			if (cur_clause->conditions != NULL) {  /* not first clause in rule */
				if ((cur_clause->next_clause = m_crule_clause_node()) != NULL)
					cur_clause = cur_clause->next_clause;    /* set up new clause rec */
				else
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			}
			/* set up crule_cond record and pointer */
			if ((cur_clause->conditions = m_crule_cond_rec()) != NULL)
				cur_cond = cur_clause->conditions;
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		} else if (strncmp(keyword,"then",4) == 0) {  /* opertation stmts header */
			STATE = OP_STMTS;
			if ((cur_clause->operations = m_crule_op_rec()) != NULL)
				cur_op = cur_clause->operations;
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		} else if ((strcmp(keyword,"STARTSURF") == 0) ||/* process all condition statements together */
					(strcmp(keyword,"NEXTSURF") == 0) ||
					(strcmp(keyword,"STARTCAT") == 0) ||
					(strcmp(keyword,"NEXTCAT") == 0)  ||
					(strcmp(keyword,"MATCHCAT") == 0)) { /* condition statements */
			if (STATE != COND_STMTS) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
			if (get_crule_conds(keyword,line_ptr,cur_cond) == FAIL) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
		} else if (strcmp(keyword,"RESULTCAT") == 0) { /* process operation statements */
			if (STATE != OP_STMTS) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
			if (get_crule_ops(line_ptr,cur_op) == FAIL) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
		} else if (strncmp(keyword,"RULEPACKAGE",11) == 0) {
			if (put_rulepack(line_ptr,cur_clause) == FAIL) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
		} else { /* hope we're looking at some variable declarations */
			if (STATE != INIT_STMTS) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
			if (get_var_decl(keyword,line_ptr) == 0) {
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}
		}
	} /* end while */
	crules_linenum += lines_read;
	check_rulepacks();
	return(SUCCEED);
}

BOOL get_crule_conds(STRING *cond_name, STRING *line, CRULE_COND_PTR cond_ptr) {
	extern int crules_linenum;
	int  hyphenI;
	CRULE_COND_PTR cond_tmp;
	STRING *line_ptr, *tline_ptr;
	char pattern[MAX_PATLEN];
	FEATTYPE features[MAXCAT];
	FEATTYPE feat1[MAX_WORD];
	int cond_type;
	BOOL isNot = FALSE;

	/* housekeeping - set up pointers */
	line_ptr = line;
	cond_tmp = cond_ptr;   /* get to empty condition record in list */
	while((cond_tmp->cond_type != -1) && (cond_tmp->next_cond != NULL))
		cond_tmp = cond_tmp->next_cond;
	if ((cond_tmp->cond_type != -1) && (cond_tmp->next_cond == NULL)) {
		if ((cond_tmp->next_cond = m_crule_cond_rec()) == NULL)
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		else
			cond_tmp = cond_tmp->next_cond;
	}
	if (strcmp(cond_name,"STARTSURF") == 0)
		cond_type = STARTSURF_CHK;
	else if (strcmp(cond_name,"NEXTSURF") == 0)
		cond_type = NEXTSURF_CHK;
	else if (strcmp(cond_name,"STARTCAT") == 0)
		cond_type = STARTCAT_CHK;
	else if (strcmp(cond_name,"NEXTCAT") == 0)
		cond_type = NEXTCAT_CHK;
	else if (strcmp(cond_name,"MATCHCAT") == 0)
		cond_type = MATCHCAT;
	else
		cond_type = 0;

	/* long clause to process statements accordingly */
	if ((cond_type == STARTSURF_CHK) || (cond_type == NEXTSURF_CHK)) {
		line_ptr = skip_equals(line_ptr);
		if (*line_ptr == EOS) {
			fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",crules_linenum);
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
				fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",crules_linenum);
				return(FAIL);
			}
		}

		/* rest of line contains surface pattern - process & store */
		if (expand_pat(line_ptr,pattern) == FAIL)
			return(FAIL);
		else {
			cond_tmp->op_surf= (char *) malloc((size_t) (sizeof(char) * (strlen(pattern)+1)));
			mem_ctr++;
			if (cond_tmp->op_surf!= NULL) {
				cond_tmp->cond_type = cond_type;
				strcpy(cond_tmp->op_surf,pattern);
			} else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
	} else if (cond_type == MATCHCAT) {
		line_ptr = skip_blanks(line_ptr);
		if (*line_ptr == EOS) {
			fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",crules_linenum);
			return(FAIL);
		}
		if ((line_ptr = get_feat_path(line_ptr,feat1)) == NULL) {
			fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
			return(FAIL);
		}
		cond_tmp->cond_type = cond_type;/* got path- store in cond rec */
		cond_tmp->op_cat= (FEATTYPE *) malloc((size_t) (sizeof(FEATTYPE) * (featlen(feat1)+1)));
		mem_ctr++;
		if (cond_tmp->op_cat != NULL) {
	 		featcpy(cond_tmp->op_cat,feat1);
		} else
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
	} else { /* end of match value stmt !; process (series of) checks on features */
		line_ptr = skip_equals(line_ptr);
		if (*line_ptr == EOS) {
			fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",crules_linenum);
			return(FAIL);
		}
		/* rest of line contains conditions on features */
		while ((line_ptr != NULL) && (*line_ptr != EOS)) {
			if (*line_ptr == '!') {   /* NOT */
				isNot = TRUE;
				++line_ptr;
			}
			line_ptr = get_feats(line_ptr,features);
			if (line_ptr != NULL) {  /* enter condition into condition list */
				if (cond_tmp->cond_type != -1) {  /* time for new cond rec */
					if ((cond_tmp->next_cond = m_crule_cond_rec()) == NULL)
						mor_mem_error(FAIL,"can't malloc space for crules, quitting");
					else
						cond_tmp = cond_tmp->next_cond;
				}
				if (isNot) {
					cond_tmp->cond_type = cond_type + NOT;
					isNot = FALSE;
				} else {
					cond_tmp->cond_type = cond_type;
				}
				cond_tmp->op_cat = (FEATTYPE *) malloc((size_t)(sizeof(FEATTYPE)*(featlen(features)+1)));
				mem_ctr++;
				if (cond_tmp->op_cat!= NULL) {
					featcpy(cond_tmp->op_cat,features);
				} else
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			} else {
				fprintf(stderr,"*ERROR* bad feature specification\n");
				fprintf(stderr,"*ERROR* found at line %d.\nALLOCAT%s\n",crules_linenum,line);
				return(FAIL);
			}
			tline_ptr = line_ptr;
			if ((line_ptr = strchr(line_ptr,',')) != NULL) { /* more conditions? */
				++line_ptr; /* skip comma */
				while ((*line_ptr == ' ') || (*line_ptr == '\t')) /* skip whitespace */
					++line_ptr;
			} else {
				while (*tline_ptr == ' ' || *tline_ptr == '\t' || *tline_ptr == '\n')
					tline_ptr++;
				if (*tline_ptr && *tline_ptr != '%') {
					fprintf(stderr,"*ERROR* features must be separated with a comma.\n");
					return(FAIL);
				}
			}
		}
	}
	return(SUCCEED);
}


/* get_crule_ops */
/* need only process RESULTCAT statements */
BOOL get_crule_ops(STRING *line, CRULE_OP_PTR op_ptr) {
	extern int crules_linenum;
	STRING *line_ptr, *tline_ptr;
	CRULE_OP_PTR op_tmp;
	char cOperator[MAX_WORD];
	FEATTYPE features[MAXCAT];
	int op_type = 0;

	/* housekeeping - set up pointers */
	line_ptr = line;
	op_tmp = op_ptr;   /* get to empty operation record in list */
	while((op_tmp->op_type != -1) && (op_tmp->next_op != NULL))
		op_tmp = op_tmp->next_op;
	if ((op_tmp->op_type != -1) && (op_tmp->next_op == NULL)) {
		if ((op_tmp->next_op = m_crule_op_rec()) == NULL)
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		else
		op_tmp = op_tmp->next_op;
	}
	line_ptr = skip_equals(line_ptr);

	if (*line_ptr == EOS) {
		fprintf(stderr,"*ERROR* unexpected end of line at line %d\n",crules_linenum);
		return(FAIL);
	}

	/* rest of line contains operations on fs- get and store */
	while ((line_ptr != NULL) && (*line_ptr != EOS)) {
		line_ptr = get_word(line_ptr,cOperator);     /* get operation */
		if (cOperator[0] != EOS) {
			if (strncmp(cOperator,"STARTCAT",8) == 0) { 
				op_type = COPY_STARTCAT; 	/* copy entire cat or just set of features? */
				line_ptr = skip_blanks(line_ptr);
				if ((*line_ptr == ',') || (*line_ptr == EOS))
					features[0] = 0;
				else {
					op_type = ADD_START_FEAT;
					if ((line_ptr = get_feat_path(line_ptr,features)) == NULL)
						return(FAIL);
				}
			} else if (strncmp(cOperator,"NEXTCAT",7) == 0) { 
				op_type = COPY_NEXTCAT; /* copy entire cat or just set of features? */
				line_ptr = skip_blanks(line_ptr);
				if ((*line_ptr == ',') || (*line_ptr == EOS))
					features[0] = 0;
				else {
					op_type = ADD_NEXT_FEAT;
					if ((line_ptr = get_feat_path(line_ptr,features)) == NULL)
						return(FAIL);
				}
			} else if (strncmp(cOperator,"ADD",3) == 0) {
				op_type = ADD_CAT;
				line_ptr = skip_blanks(line_ptr);
				if ((line_ptr = get_feats(line_ptr,features)) == NULL) {
					fprintf(stderr,"*ERROR* found at line %d.\nALLOCAT%s\n",crules_linenum,line);
					return(FAIL);
				}
			} else if (strncmp(cOperator,"DEL",3) == 0) {
				op_type = DEL_CAT;
				line_ptr = skip_blanks(line_ptr);
				if ((line_ptr = get_feat_path(line_ptr,features)) == NULL)
					return(FAIL);
			} else if (strncmp(cOperator,"SET",3) == 0) {
				op_type = SET_CAT;
				line_ptr = skip_blanks(line_ptr);
				if ((line_ptr = get_feats(line_ptr,features)) == NULL) {
					fprintf(stderr,"*ERROR* found at line %d.\nALLOCAT%s\n",crules_linenum,line);
					return(FAIL);
				}
			} else { /* shouldn't get here */
				fprintf(stderr,"*ERROR* in crules file at line %d\n",crules_linenum);
				return(FAIL);
			}

			/* set up pointer into rules */
			if (op_tmp->op_type != -1) {  /* time for new op rec */
				if ((op_tmp->next_op = m_crule_op_rec()) == NULL)
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
				else
					op_tmp = op_tmp->next_op;
			}
			/* stick in the data */
			op_tmp->op_type = op_type;
			op_tmp->data = (FEATTYPE *) malloc((size_t) (sizeof(FEATTYPE) * (featlen(features)+1)));
			mem_ctr++;
			if (op_tmp->data != NULL)
				featcpy(op_tmp->data,features);
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
		tline_ptr = line_ptr;
		if ((line_ptr = strchr(line_ptr,',')) != NULL) { /* more operations? */
			++line_ptr; /* skip comma */
			line_ptr = skip_blanks(line_ptr);
		} else {
			while (*tline_ptr == ' ' || *tline_ptr == '\t' || *tline_ptr == '\n')
				tline_ptr++;
			if (*tline_ptr && *tline_ptr != '%') {
				fprintf(stderr,"*ERROR* operators must be separated with a comma.\n");
				return(FAIL);
			}
		}
	}
	return(SUCCEED);
}

BOOL
put_startrule(CRULE_PTR rule_ptr) {
	extern RULEPACK_PTR startrules_list;
	RULEPACK_PTR rp_tmp;

	if (startrules_list == NULL) { /* first startrule found */
		if ((startrules_list = m_rulepack_rec()) != NULL) {
			startrules_list->rule_ptr = rule_ptr;
			startrules_list->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rule_ptr->name)+1)));
			mem_ctr++;
			if (startrules_list->rulename != NULL)
				strcpy(startrules_list->rulename,rule_ptr->name);
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
		else  /* malloc failed */
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
	}
	else {  /* add to end of startrules list */
		rp_tmp = startrules_list;
		while (rp_tmp->next_rule != NULL)
			rp_tmp = rp_tmp->next_rule;
		if ((rp_tmp->next_rule = m_rulepack_rec()) != NULL) {
			rp_tmp = rp_tmp->next_rule;
			rp_tmp->rule_ptr = rule_ptr;
			rp_tmp->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rule_ptr->name)+1)));
			mem_ctr++;
			if (rp_tmp->rulename != NULL)
				strcpy(rp_tmp->rulename,rule_ptr->name);
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
		else  /* malloc failed */
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
	}
	return(SUCCEED);
}

BOOL
put_rulepack(STRING *line_ptr, CRULE_CLAUSE_PTR clause_ptr) {
	RULEPACK_PTR rp_tmp;
	STRING *line_tmp;
	char rulename[MAX_WORD];
	int index;

	rp_tmp = NULL;

	if ((line_tmp = skip_equals(line_ptr)) == NULL)
		return(FAIL);
	/* special case; emtpy rulepack */
	if (strncmp(line_tmp,"{}",2) == 0)  
		return(SUCCEED);  /* leave rulepackages slot NULL */
	if (*line_tmp == '{') {   /* open of rp set */
		line_tmp++;
		line_tmp = skip_blanks(line_tmp);
	}
	else
		return(FAIL);
	while ((line_tmp != NULL) && (*line_tmp != EOS)) {
		if ((line_tmp = get_word(line_tmp,rulename)) != NULL) { /* got rulename */
			/* trim trailing '}' - is this necessary? */
			index = strlen(rulename);
			if (rulename[index] == '}')
				rulename[index] = EOS;
			/* store name in rulepackages list */
			if (rp_tmp == NULL) { /* first rulepakage in list */
				if ((clause_ptr->rulepackages = m_rulepack_rec()) != NULL) {
					clause_ptr->rulepackages->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rulename)+1)));
					mem_ctr++;
					if (clause_ptr->rulepackages->rulename != NULL)
						strcpy(clause_ptr->rulepackages->rulename,rulename);
					else
						mor_mem_error(FAIL,"can't malloc space for crules, quitting");
					rp_tmp = clause_ptr->rulepackages;  /* all set for next rulename */
				}
				else  /* malloc failed */
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			}
			else {  /* add to end of list */
				if ((rp_tmp->next_rule = m_rulepack_rec()) != NULL) {
					rp_tmp = rp_tmp->next_rule;
					rp_tmp->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rulename)+1)));
					mem_ctr++;
					if (rp_tmp->rulename != NULL)
						strcpy(rp_tmp->rulename,rulename);
					else
						mor_mem_error(FAIL,"can't malloc space for crules, quitting");
				}
				else  /* malloc failed */
					mor_mem_error(FAIL,"can't malloc space for crules, quitting");
			}
			/* more rulenames in this package? */
			if ((line_tmp = strchr(line_tmp,',')) != NULL)
				++line_tmp;
		}
	}
	return(SUCCEED);
}

BOOL
put_endrule(CRULE_PTR rule_ptr) {
	extern RULEPACK_PTR endrules_list;
	RULEPACK_PTR rp_tmp;

	if (endrules_list == NULL) { /* first endrule found */
		if ((endrules_list = m_rulepack_rec()) != NULL) {
			endrules_list->rule_ptr = rule_ptr;
			endrules_list->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rule_ptr->name)+1)));
			mem_ctr++;
			if (endrules_list->rulename != NULL)
				strcpy(endrules_list->rulename,rule_ptr->name);
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
		else  /* malloc failed */
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
	}
	else {  /* add to end of endrules list */
		rp_tmp = endrules_list;
		while (rp_tmp->next_rule != NULL)
			rp_tmp = rp_tmp->next_rule;
		if ((rp_tmp->next_rule = m_rulepack_rec()) != NULL) {
			rp_tmp = rp_tmp->next_rule;
			rp_tmp->rule_ptr = rule_ptr;
			rp_tmp->rulename = (STRING *)malloc((size_t)(sizeof(char)*(strlen(rule_ptr->name)+1)));
			mem_ctr++;
			if (rp_tmp->rulename != NULL)
				strcpy(rp_tmp->rulename,rule_ptr->name);
			else
				mor_mem_error(FAIL,"can't malloc space for crules, quitting");
		}
		else  /* malloc failed */
			mor_mem_error(FAIL,"can't malloc space for crules, quitting");
	}
	return(SUCCEED);
}

/* check_rulepacks : 
   make sure that rules referenced in "RULEPAKAGES" stmt
   have been defined */
/* prints warnings if rule not found somewhere in rulelist */
void
check_rulepacks()
{
	extern CRULE_PTR crule_list;
	CRULE_PTR rule_tmp;
	CRULE_CLAUSE_PTR clause_tmp;
	CRULE_PTR search_tmp;
	CRULE_COND_PTR cond_tmp;
	RULEPACK_PTR rp_tmp;

	rule_tmp = crule_list;
	while (rule_tmp != NULL) {
		clause_tmp = rule_tmp->clauses;
		while (clause_tmp != NULL) {
			cond_tmp = clause_tmp->conditions;
			while (cond_tmp != NULL) {
				if (cond_tmp->cond_type == -1) {
					fprintf(stderr,"\n*ERROR* in crule \"%s\"\n", rule_tmp->name);
					fprintf(stderr,"   Missing conditional statement, such as NEXTCAT or NEXTSURF, on line below \"if\"\n");
					break;
				}
				cond_tmp = cond_tmp->next_cond;
			}
			rp_tmp = clause_tmp->rulepackages;
			while (rp_tmp != NULL) {
				search_tmp = crule_list;
				while ((search_tmp != NULL) && (strcmp(rp_tmp->rulename,search_tmp->name) != 0))
					search_tmp = search_tmp->next_rule;
				if (search_tmp != NULL)
					rp_tmp->rule_ptr = search_tmp;
				else
					fprintf(stderr,"warning:  crule \"%s\" referenced but not defined\n", rp_tmp->rulename);
				rp_tmp = rp_tmp->next_rule;
			}
			clause_tmp = clause_tmp->next_clause;
		}
		rule_tmp = rule_tmp->next_rule;
	}
}

void free_lex_lett(LEX_PTR lex, int num) {
    register int i;

    for (i=0; i < num; i++) {
		if (lex[i].entries) {
			lex[i].entries = free_entries(lex[i].entries);
		}
		if (lex[i].num_letters > -1)
			free_lex_lett(lex[i].next_lett, lex[i].num_letters);
    }
    free(lex);
}

static void add_to_result(STRING *word, FEATTYPE *cat, STRING *parse, STRING *trans, STRING *comp) {
	int i;
	extern RESULT_REC *result_list;
	extern int result_index;
/* lxs lxslxs remove duplicates dup */

	if (result_index < MAX_PARSES-1) {
		if (apply_endrules(word,cat,parse) == SUCCEED) {
			for (i=1; i <= result_index; i++) {
				if (featcmp(result_list[i].cat, cat)	== 0 && 
					strcmp(result_list[i].surface,word)	== 0 &&
					strcmp(result_list[i].stem,parse)	== 0 &&
					strcmp(result_list[i].trans,trans)	== 0 &&
					strcmp(result_list[i].comp,comp)	== 0)
					break;
			}
			if (i > result_index) {
				result_index++;
				strcpy(result_list[result_index].surface,word);
				featcpy(result_list[result_index].cat,cat);
				strcpy(result_list[result_index].stem,parse);
				strcpy(result_list[result_index].trans,trans);
				strcpy(result_list[result_index].comp,comp);
			}
		}
	} else {
		fprintf(stderr, "\nanalyze_word: too many parses, parse stack overflow\n");
		CleanUpAll(TRUE);
		CloseFiles();
		cutt_exit(0);
	}
}



/***********************************************************************
 * analyze word                                                         *
 * parse a word into its constituent morphemes                          *
 ************************************************************************/
static void do_word_analyses(
							 STRING *word,
							 FEATTYPE *cat,
							 STRING *parse,
							 STRING *trans,
							 STRING *comp,
							 STRING *rest,
							 RULEPACK_PTR rp,
							 int ind)
{
	extern TRIE_PTR trie_root;
	TRIE_PTR trie_p;
	ELIST_PTR entry_p;
	STRING *rest_p;
	STRING *newrest_p;
	char start[MAX_WORD];
	char next[MAX_WORD];
	FEATTYPE start_cat[MAXCAT];
	FEATTYPE next_cat[MAXCAT];
	char start_parse[MAX_WORD];
	char next_stem[MAX_WORD];
	char start_trans[MAX_WORD];
	char next_trans[MAX_WORD];
	char start_comp[MAX_WORD];
	char next_comp[MAX_WORD];
	char fs_tmp[MAXCAT];
	unsigned char utf8Char[7];
	int UTF8i;
	int newind, i;
	int letter_idx;

	if (DEBUG_CLAN & 4) {
		fprintf(debug_fp,"\nenter analyze_word\n");
		fprintf(debug_fp,"word: %s\n",word);
		if (featlen(cat) > 0) {
			fs_decomp(cat,fs_tmp);
			fprintf(debug_fp,"cat: %s\n",fs_tmp);
		} else {
			int i;
			fprintf(debug_fp,"cat: ");
			for (i=0; cat[i] != 0; i++)
				fprintf(debug_fp,"%c",(char)cat[i]);
			fprintf(debug_fp,"\n");
		}
		fprintf(debug_fp,"parse: %s\n",parse);
		fprintf(debug_fp,"rest: %s\n",rest);
	}

	if (*rest == EOS) {  /* success! (via recursion) */
		add_to_result(word, cat, parse, trans, comp);
		return;  /* recursion bottoms out here */
	}
	/* otherwise, keep on parsing word */
	trie_p = trie_root;
	rest_p = rest;

	UTF8i = 0;
	while ((trie_p != NULL) && (*rest_p != EOS)) {
		if ((letter_idx = find_letter(trie_p,*rest_p)) < 0)
			trie_p = NULL;
		else {  /* matched on a letter - see if we've got an entire morpheme */
			trie_p = trie_p->letters[letter_idx].t_node;
			if (DEBUG_CLAN & 4) {
				utf8Char[UTF8i] = (unsigned char)*rest_p;
				if (UTF8_IS_SINGLE(utf8Char[UTF8i]) || UTF8_IS_LEAD((unsigned char)*(rest_p+1)) || UTF8_IS_SINGLE((unsigned char)*(rest_p+1)) || *(rest_p+1) == EOS) {
					utf8Char[UTF8i+1] = EOS;
					for (i = 0; i < ind; i++)
						fprintf(debug_fp," ");
					fprintf(debug_fp,"%s\n",utf8Char);
					UTF8i = 0;
					++ind;
				} else if (UTF8i < 5)
					UTF8i++;
			}
			if (trie_p->entries != NULL) {  	/* found a morpheme */
				newrest_p = rest_p+1; /* anticipating success */
				newind = ind+1;
				/* set up variable to be passed to c_rules */
				strncpy(start,word,rest-word);
				start[rest-word] = EOS;
				strncpy(next,rest,rest_p-rest+1);
				next[rest_p-rest+1] = EOS;
				featcpy(start_cat,cat);
				strcpy(start_parse,parse);
				strcpy(start_trans,trans);
				strcpy(start_comp,comp);

				entry_p = trie_p->entries;
				while (entry_p != NULL) {
					/* more variables */
					featcpy(next_cat,entry_p->entry);
					if (entry_p->stem != NULL)
						strcpy(next_stem,entry_p->stem);
					else
						strcpy(next_stem,next);

					if (entry_p->trans != NULL)
						strcpy(next_trans,entry_p->trans);
					else
						next_trans[0] = EOS;

					if (entry_p->comp != NULL)
						strcpy(next_comp,entry_p->comp);
					else
						next_comp[0] = EOS;

					concat_word(word,rest,start,start_cat,start_parse,start_trans,start_comp,next,next_cat,next_stem,next_trans,next_comp,rp,newrest_p,newind);
					entry_p = entry_p->next_elist;
				}
			} /* end if (trie_p->entries != NULL) */

			++rest_p;
		}  /* end else */
	} /* end while loop */
	return;  /* quiet failure */
}

void
analyze_word(
			 STRING *word,
			 FEATTYPE *cat,
			 STRING *parse,
			 STRING *trans,
			 STRING *comp,
			 STRING *rest,
			 RULEPACK_PTR rp,
			 int ind,
			 char doDef)
{
	char *hyphenI;
	char lcat[MAX_WORD], ts[256];
	FEATTYPE n_comp_feat[256];
	extern int result_index;
	extern RESULT_REC *result_list;
    extern BOOL doCapitals;

// 2019-07-30 change dash to 0x2013
	hyphenI = strchr(word, '-');
	while (hyphenI != NULL) {
//fprintf(stderr, "%s\n", word);
		uS.shiftright(hyphenI, 2);
		*hyphenI++ = 0xE2;
		*hyphenI++ = 0x80;
		*hyphenI   = 0x93;
		hyphenI = strchr(word, '-');
	}

	do_word_analyses(word, cat, parse, trans, comp, rest, rp, ind);
	/* lxs lxslxs *+* */
	if (doDef && result_index == 0 && uS.patmat(word,"_*+_*") && doCapitals) {
		strcpy(ts, "[scat]");
		if (get_vname(cat,ts,lcat) && !uS.patmat(lcat,"_*+_*")) {
			strcpy(ts, "[scat n+n+n]");
			get_feats(ts, n_comp_feat); // THIS ONE MUST BE LAST IN A LIST
			if (++result_index < MAX_PARSES) {
				strcpy(result_list[result_index].surface,word);
				featcpy(result_list[result_index].cat,n_comp_feat);
				strcpy(result_list[result_index].stem,word);
				strcpy(result_list[result_index].trans,trans);
				strcpy(result_list[result_index].comp,comp);
			}
		}
	}
}

static BOOL isExcludeLetterFromDebug(char *st) {
	int i;
	const char *debugLetters = "abcefghjklmnopqrtuvwxz";

	if (st[0] == EOS || st[1] != EOS)
		return(FALSE);
	for (i=0; debugLetters[i] != EOS; i++) {
		if (debugLetters[i] == st[0])
			return(TRUE);
	}
	return(FALSE);
}

/***********************************************************************
* concat word                                                          *
* parse a word into its constituent morphemes                          *
************************************************************************/
void
concat_word(
	STRING *word,
	STRING *rest,
    STRING *s_surf,
    FEATTYPE *s_cat,
    STRING *s_parse,
	STRING *s_trans,
	STRING *s_comp,
    STRING *n_surf,
    FEATTYPE *n_cat,
    STRING *n_stem,
	STRING *n_trans,
	STRING *n_comp,
    RULEPACK_PTR rp,
	STRING *newrest_p,
	int    newind) {

	RULEPACK_PTR cur_rp;
	CRULE_PTR cur_rule;
	CRULE_CLAUSE_PTR cur_clause;
	CRULE_COND_PTR cur_cond;
	CRULE_OP_PTR cur_op;
	char r_surf[MAX_WORD];
	FEATTYPE r_cat[MAXCAT];
	char r_parse[MAX_WORD];
	char r_trans[MAX_WORD];
	char r_comp[MAX_WORD];
	FEATTYPE fvp_tmp[MAXCAT];
	char fs_tmp[MAX_WORD];
	char word_tmp[MAX_WORD];
	char concat_char[2];
	int clause_ctr;
	BOOL isExcludeFromDebug;
	FEATTYPE *NextCatt = NULL;
	PARSE_REC parse_stack;

	isExcludeFromDebug = isExcludeLetterFromDebug(n_surf);
	if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
		fprintf(debug_fp,"\napplying c rules\n");
		fprintf(debug_fp," word: %s\n",word);
		fprintf(debug_fp," rest: %s\n",rest);
		fprintf(debug_fp," start: %s\n",s_surf);
		if (featlen(s_cat) > 0) {
			fs_decomp(s_cat,fs_tmp);
			fprintf(debug_fp," start cat: %s\n",fs_tmp);
		} else {
			int i;
			fprintf(debug_fp," start cat: ");
			for (i=0; s_cat[i] != 0; i++)
				fprintf(debug_fp,"%c",(char)s_cat[i]);
			fprintf(debug_fp,"\n");
		}
		fprintf(debug_fp," current parse: %s\n",s_parse);
		fprintf(debug_fp," next: %s\n",n_surf);
		if (featlen(n_cat) > 0) {
			fs_decomp(n_cat,fs_tmp);
			fprintf(debug_fp," next cat: %s\n",fs_tmp);
		} else {
			int i;
			fprintf(debug_fp," next cat: ");
			for (i=0; n_cat[i] != 0; i++)
				fprintf(debug_fp,"%c",(char)n_cat[i]);
			fprintf(debug_fp,"\n");
		}
		fprintf(debug_fp," next stem: %s\n",n_stem);
	}

	strcpy(r_surf,s_surf);
	strcat(r_surf,n_surf);  /* concatenation at its finest */

	cur_rp = rp;  /* walk through rulepack */
	while (cur_rp != NULL) {
		if (cur_rp->rule_ptr != NULL) {
			cur_rule = cur_rp->rule_ptr;
			if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
				fprintf(debug_fp,"\ntrying rule %s ... \n",cur_rule->name);
// lxs 2014-02-05
				fprintf(debug_fp,"        word: %s\n",word);
				fprintf(debug_fp,"        rest: %s\n",rest);
				fprintf(debug_fp,"        start: %s\n",s_surf);
				if (featlen(s_cat) > 0) {
					fs_decomp(s_cat,fs_tmp);
					fprintf(debug_fp,"        start cat: %s\n",fs_tmp);
				} else {
					int i;
					fprintf(debug_fp,"        start cat: ");
					for (i=0; s_cat[i] != 0; i++)
						fprintf(debug_fp,"%c",(char)s_cat[i]);
					fprintf(debug_fp,"\n");
				}
				fprintf(debug_fp,"        current parse: %s\n",s_parse);
				fprintf(debug_fp,"        next: %s\n",n_surf);
				if (featlen(n_cat) > 0) {
					fs_decomp(n_cat,fs_tmp);
					fprintf(debug_fp,"        next cat: %s\n",fs_tmp);
				} else {
					int i;
					fprintf(debug_fp,"        next cat: ");
					for (i=0; n_cat[i] != 0; i++)
						fprintf(debug_fp,"%c",(char)n_cat[i]);
					fprintf(debug_fp,"\n");
				}
				fprintf(debug_fp,"        next stem: %s\n",n_stem);
// lxs 2014-02-05
			}
			clause_ctr = 0;
			cur_clause = cur_rule->clauses;
clause_loop:
			if (cur_clause != NULL) {
				clause_ctr++;
				if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
					fprintf(debug_fp," trying clause/ if-then %d\n",clause_ctr);
				}

				cur_cond = cur_clause->conditions;
				while (cur_cond != NULL) {  /* apply conditions on categories */
					if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
						int i;
						decode_cond_code(cur_cond->cond_type,word_tmp);
						fprintf(debug_fp,"  condition = %s ",word_tmp);
						if (cur_cond->cond_type != STARTSURF_CHK && cur_cond->cond_type != NEXTSURF_CHK && 
							cur_cond->cond_type != STARTSURF_NOT && cur_cond->cond_type != NEXTSURF_NOT &&
							cur_cond->cond_type != -1) {
							fs_decomp(cur_cond->op_cat,word_tmp);
							if (*word_tmp == '{')
								strcpy(word_tmp, word_tmp+1);
							i = strlen(word_tmp) - 1;
							if (word_tmp[i] == '}')
								word_tmp[i] = EOS;
						} else
							strcpy(word_tmp, cur_cond->op_surf);
						fprintf(debug_fp,"{%s}\n",word_tmp);
					}

					switch(cur_cond->cond_type) {
						case STARTSURF_CHK :
							if (match_word(s_surf,cur_cond->op_surf,0) == SUCCEED) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_cond = cur_cond->next_cond;
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case NEXTSURF_CHK :
							if (match_word(n_surf,cur_cond->op_surf,0) == SUCCEED) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case STARTSURF_NOT :
							if (match_word(s_surf,cur_cond->op_surf,0) == FAIL) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_cond = cur_cond->next_cond;
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case NEXTSURF_NOT :
							if (match_word(n_surf,cur_cond->op_surf,0) == FAIL) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case STARTCAT_CHK :
							if (check_fvp(s_cat,cur_cond->op_cat) == SUCCEED) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else{
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case NEXTCAT_CHK :
							NextCatt = cur_cond->op_cat;
							if (check_fvp(n_cat,cur_cond->op_cat) == SUCCEED) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case STARTCAT_NOT :
							if (check_fvp(s_cat,cur_cond->op_cat) == FAIL) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else{
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case NEXTCAT_NOT :
							if (check_fvp(n_cat,cur_cond->op_cat) == FAIL) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						case MATCHCAT :
							if (match_fvp(s_cat,n_cat,cur_cond->op_cat) == SUCCEED) {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition is met\n");
								}
								cur_cond = cur_cond->next_cond;
							} else {
								if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
									fprintf(debug_fp,"  condition failed\n");
								}
								cur_clause = cur_clause->next_clause;
								goto clause_loop;
							}
							break;
						default: { /* shouldn't get here */
							cur_clause = cur_clause->next_clause;
							goto clause_loop; 
						}
					}
				}
				/* end loop on all conditions */
				/* if we're here, we matched on all conditions for a rule */
				/* process cat */
				r_cat[0] = 0;
				if (cur_clause->operations != NULL) {  /* what if it's null? */
					cur_op = cur_clause->operations;
					while (cur_op != NULL) {
						if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
							decode_op_code(cur_op->op_type,word_tmp);
							fprintf(debug_fp,"  operation = %s",word_tmp);
							if (cur_op->data != NULL) {
								fs_decomp(cur_op->data,word_tmp);
								if (word_tmp[0] == EOS)
									fprintf(debug_fp,"\n");
								else
									fprintf(debug_fp," %s\n",word_tmp);
							} else
								fprintf(debug_fp,"\n");
							fs_decomp(r_cat,word_tmp);
							fprintf(debug_fp,"   current result cat = %s\n",word_tmp);
						}
						switch(cur_op->op_type) {
							case COPY_STARTCAT :
								featcat(r_cat,s_cat);
								break;
							case COPY_NEXTCAT :
								featcat(r_cat,n_cat);
								break;
							case ADD_CAT :
							case SET_CAT :
								add_fvp(r_cat,cur_op->data);
								break;
							case DEL_CAT :
								if (retrieve_fvp(fvp_tmp,cur_op->data,r_cat) != NULL)
									remove_fvp(r_cat,fvp_tmp);
//                              else  // let rule succeed !
//                                  return; // rule fails if operation can't be done!
								break;
							case ADD_START_FEAT :
								if (retrieve_fvp(fvp_tmp,cur_op->data,s_cat) != NULL)
									add_fvp(r_cat,fvp_tmp);
// comment the whole else for RESULTCAT = NEXTCAT, STARTCAT [gen]
/*
								else {
									if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
										int fvp_type;
										fs_decomp(cur_op->data,word_tmp);
										if (*word_tmp == '{')
											strcpy(word_tmp, word_tmp+1);
										fvp_type = strlen(word_tmp) - 1;
										if (word_tmp[fvp_type] == '}')
											word_tmp[fvp_type] = EOS;
										fs_decomp(s_cat,fs_tmp);
										fprintf(debug_fp, "   Missing \"%s\" from \"STARTCAT: %s\" of \"%s\".\n",word_tmp,fs_tmp,s_surf);
										fprintf(debug_fp, "   operation failed\n");
									}
									cur_clause = cur_clause->next_clause;
									goto clause_loop; 
// lxslxs STARTCAT [comp]
//									return;  // rule fails if operation can't be done!
								}
*/
								break;
							case ADD_NEXT_FEAT :
								if (retrieve_fvp(fvp_tmp,cur_op->data,n_cat) != NULL)
									add_fvp(r_cat,fvp_tmp);
// comment the whole else for RESULTCAT = STARTCAT, NEXTCAT [gen]
								else {
									Error_fvp(cur_op->data, NextCatt, n_surf, s_surf, s_cat, n_cat, s_parse, n_stem, cur_rule, clause_ctr, cur_cond, cur_op, r_cat);
									if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
										fprintf(debug_fp,"  condition failed\n");
									}
									return;  // rule fails if operation can't be done!
								}
								break;
						}
						cur_op = cur_op->next_op;
					}
				}
				/* now process stem */
				strcpy(r_parse,s_parse);
				if (cur_rule->rule_type == START_RULE) {
					if (strlen(r_parse) > 0)
						strcat(r_parse,"+");
					strcat(r_parse,n_stem);
//				} else if (cur_rule->rule_type == DONT_ADD_NEXTSURF) {
				} else {
					concat_char[0] = (char)cur_rule->rule_type;
					concat_char[1] = EOS;
					strcat(r_parse,concat_char);
					strcat(r_parse,n_stem);
				}

				strcpy(r_trans, s_trans);
				if (strlen(s_trans) > 0 && strlen(n_trans) > 0)
					strcat(r_trans, "_");
				strcat(r_trans, n_trans);

				strcpy(r_comp, s_comp);
				if (strlen(s_comp) > 0 && strlen(n_comp) > 0)
					strcat(r_comp, "_");
				strcat(r_comp, n_comp);

				/* add this to parse stack */
				featcpy(parse_stack.cat,r_cat);
				strcpy(parse_stack.parse,r_parse);
				strcpy(parse_stack.trans,r_trans);
				strcpy(parse_stack.comp,r_comp);
				parse_stack.rp  = cur_clause->rulepackages;
				if (DEBUG_CLAN & 4 && !isExcludeFromDebug) {
					fprintf(debug_fp,"%s succeeded!\n",cur_rule->name);
					if (featlen(r_cat) > 0) {
						fs_decomp(r_cat,fs_tmp);
						fprintf(debug_fp," result cat: %s\n",fs_tmp);
					} else {
						int i;
						fprintf(debug_fp," result cat: ");
						for (i=0; r_cat[i] != 0; i++)
							fprintf(debug_fp,"%c",(char)r_cat[i]);
						fprintf(debug_fp,"\n");
					}
					fprintf(debug_fp," current parse: %s\n",r_parse);
				}
				/* recurse iteratively on successful concatenations */
				do_word_analyses(word,parse_stack.cat,parse_stack.parse,parse_stack.trans,parse_stack.comp,newrest_p,parse_stack.rp,newind);
			}  /* end loop through clauses in a rule */
		}
		cur_rp = cur_rp->next_rule; /* look at next rule in rulepackage */
	}  /* end loop through rules in rulepackage */
	return;
}

/* apply_endrules -  make sure that word is actual word- */
/* most of this code is identical to analyze_word */
/* differences :  apply_endrules only operates on cat info */
BOOL
apply_endrules(STRING *surf, FEATTYPE *cat, STRING *stem)
{
	extern RULEPACK_PTR endrules_list;
	RULEPACK_PTR cur_rp;
	CRULE_PTR cur_rule;
	CRULE_OP_PTR cur_op;
	CRULE_CLAUSE_PTR cur_clause;
	CRULE_COND_PTR cur_cond;
	char surf_tmp[MAX_WORD];
	FEATTYPE cat_tmp[MAXCAT];
	char stem_tmp[MAX_WORD];
	FEATTYPE fvp_tmp[MAXCAT];
	char fs_tmp[MAXCAT];
	char word_tmp[MAX_WORD];
	int clause_ctr;
	FEATTYPE r_cat[MAXCAT];
	
	r_cat[0] = EOS;
	
	strcpy(surf_tmp,surf);
	featcpy(cat_tmp,cat);
	strcpy(stem_tmp,stem);
	
	if (DEBUG_CLAN & 4) {
		fprintf(debug_fp,"\napplying end rules\n");
		fprintf(debug_fp," surf: %s\n",surf_tmp);
		if (featlen(cat_tmp) > 0) {
			fs_decomp(cat_tmp,fs_tmp);
			fprintf(debug_fp," cat: %s\n",fs_tmp);
		}
		else {
			int i;
			fprintf(debug_fp," cat: ");
			for (i=0; cat_tmp[i] != 0; i++)
				fprintf(debug_fp,"%c",(char)cat_tmp[i]);
			fprintf(debug_fp,"\n");
		}
		fprintf(debug_fp," parse: %s\n",stem_tmp);
	}
	
	cur_rp = endrules_list;  /* walk through endrules */
	while (cur_rp != NULL) {
		cur_rule = cur_rp->rule_ptr;
		if (DEBUG_CLAN & 4) {
			fprintf(debug_fp,"\ntrying rule %s ... \n",cur_rule->name);
		}
		clause_ctr = 0;
		cur_clause = cur_rule->clauses;
	clause_loop:
		if (cur_clause != NULL) {
			if (DEBUG_CLAN & 4) { fprintf(debug_fp," trying clause/ if-then %d\n",++clause_ctr);}
			cur_cond = cur_clause->conditions;
			while (cur_cond != NULL) {  /* apply conditions on categories */
				if (DEBUG_CLAN & 4) {
					decode_cond_code(cur_cond->cond_type,word_tmp);
					fprintf(debug_fp,"  condition = %s\n",word_tmp);
				}
				switch(cur_cond->cond_type) {
					case STARTSURF_CHK :
						if (match_word(surf,cur_cond->op_surf,0) == SUCCEED) {
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition is met\n");}
							cur_cond = cur_cond->next_cond;
						}
						else{
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition failed\n");}
							cur_clause = cur_clause->next_clause;
							goto clause_loop;
						}
						break;
					case STARTSURF_NOT :
						if (match_word(surf,cur_cond->op_surf,0) == FAIL) {
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition is met\n");}
							cur_cond = cur_cond->next_cond;
						}
						else{
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition failed\n");}
							cur_clause = cur_clause->next_clause;
							goto clause_loop;
						}
						break;
					case STARTCAT_CHK :
						if (check_fvp(cat,cur_cond->op_cat) == SUCCEED) {
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition is met\n");}
							cur_cond = cur_cond->next_cond;
						}
						else{
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition failed\n");}
							cur_clause = cur_clause->next_clause;
							goto clause_loop;
						}
						break;
					case STARTCAT_NOT :
						if (check_fvp(cat,cur_cond->op_cat) == FAIL) {
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition is met\n");}
							cur_cond = cur_cond->next_cond;
						}
						else{
							if (DEBUG_CLAN & 4) {fprintf(debug_fp,"  condition failed\n");}
							cur_clause = cur_clause->next_clause;
							goto clause_loop;
						}
						break;
					default: { /* shouldn't get here */
						cur_clause = cur_clause->next_clause;
						goto clause_loop; 
					}
				}
			}/* end loop on all conditions */
			if (cur_clause != NULL) {
				/* if we're here, we matched on all conditions for a rule */
				/* process cat */
				r_cat[0] = 0;
				if (cur_clause->operations != NULL) {  /* what if it's null? */
					cur_op = cur_clause->operations;
					while (cur_op != NULL) {
						if (DEBUG_CLAN & 4) {
							decode_op_code(cur_op->op_type,word_tmp);
							fprintf(debug_fp,"  operation = %s",word_tmp);
							if (cur_op->data != NULL) {
								fs_decomp(cur_op->data,word_tmp);
								if (word_tmp[0] == EOS)
									fprintf(debug_fp,"\n");
								else
									fprintf(debug_fp," %s\n",word_tmp);
							} else
								fprintf(debug_fp,"\n");
							fs_decomp(r_cat,word_tmp);
							fprintf(debug_fp,"   current result cat = %s\n",word_tmp);
						}
						switch(cur_op->op_type) {
							case COPY_STARTCAT :
								featcat(r_cat,cat);
								break;
							case ADD_CAT :
							case SET_CAT :
								add_fvp(r_cat,cur_op->data);
								break;
							case DEL_CAT :
								if (retrieve_fvp(fvp_tmp,cur_op->data,r_cat) != NULL)
									remove_fvp(r_cat,fvp_tmp);
								break;
							case ADD_START_FEAT :
								if (retrieve_fvp(fvp_tmp,cur_op->data,cat) != NULL)
									add_fvp(r_cat,fvp_tmp);
								else
									return(FAIL);  /* rule fails if operation can't be done! */
								break;
						}
						cur_op = cur_op->next_op;
					}
				}
				
				/* if we're here, processed string properly, update category */
				featcpy(cat,r_cat);
				return(SUCCEED);
			}
		}/* end loop through clauses in a rule */
		cur_rp = cur_rp->next_rule; /* look at next rule in rulepackage */
	}/* end loop through rules in rulepackage */
	return(FAIL);
}

/* m_crule_node: malloc new rule node & initialize structure
 copy rulename into name slot, set all ptrs to NULL
 returns NULL if can't allocate enough space for node */
CRULE_PTR
m_crule_node(STRING *rulename) {
	CRULE_PTR new_node;

	new_node = NULL;
	new_node = (CRULE_PTR) malloc((size_t) sizeof(CRULE));
	mem_ctr++;
	if (new_node != NULL)
    {
		new_node->name =  (STRING *) malloc((size_t) (sizeof(char) * (strlen(rulename)+1)));
		mem_ctr++;
		if (new_node->name != NULL)
			strcpy(new_node->name,rulename);
		else
			return(NULL);
		new_node->rule_type = -1;
		new_node->clauses = NULL;
		new_node->next_rule = NULL;
		return(new_node);
    }
	else
		return(NULL);
}

/* m_crule_clause_node: malloc new clause node & initialize structure
 set all ptrs to NULL
 returns NULL if can't allocate enough space for node */
CRULE_CLAUSE_PTR
m_crule_clause_node()
{
	CRULE_CLAUSE_PTR new_node;

	new_node = NULL;
	new_node = (CRULE_CLAUSE_PTR) malloc((size_t) sizeof(CRULE_CLAUSE));
	mem_ctr++;
	if (new_node != NULL)
    {
		new_node->conditions = NULL;
		new_node->operations = NULL;
		new_node->rulepackages = NULL;
		new_node->next_clause = NULL;
		return(new_node);
    }
	else
		return(NULL);
}

/* m_crule_cond_rec:  malloc new cond node & intialize structure */
CRULE_COND_PTR
m_crule_cond_rec()
{
	CRULE_COND_PTR new_node;

	new_node = NULL;
	new_node = (CRULE_COND_PTR) malloc((size_t) sizeof(CRULE_COND));
	mem_ctr++;
	if (new_node != NULL)
    {
		new_node->cond_type = -1;
		new_node->next_cond = NULL;
		new_node->op_cat = NULL;  
		new_node->op_surf = NULL;  
		return(new_node);
    }
	else
		return(NULL);
}

/* m_crule_op_rec:  malloc new op node & intialize structure */
CRULE_OP_PTR
m_crule_op_rec()
{
	CRULE_OP_PTR new_node;

	new_node = NULL;
	new_node = (CRULE_OP_PTR) malloc((size_t) sizeof(CRULE_OP));
	mem_ctr++;
	if (new_node != NULL)
    {
		new_node->op_type = -1;
		new_node->next_op = NULL;
		new_node->data =  NULL;
		return(new_node);
    }
	else
		return(NULL);
}


/* m_rulepack_rec:  malloc new rulepack node & intialize structure */
RULEPACK_PTR
m_rulepack_rec()
{
	RULEPACK_PTR new_node;

	new_node = NULL;
	new_node = (RULEPACK_PTR) malloc((size_t) sizeof(RULEPACK));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->rulename = NULL;
		new_node->rule_ptr = NULL;
		new_node->next_rule =  NULL;
		return(new_node);
    } else
		return(NULL);
}

/* ******************** FREE STRUCTURES ********************* */

/* free_crules: free up crules_list */
/* recurse down list */
/* free on way out of recursion stack */
void free_crules(CRULE_PTR rule_p) {
	if (rule_p != NULL) {
		free_crules(rule_p->next_rule);
		if (rule_p->name != NULL) free(rule_p->name);
		mem_ctr--;
		if (rule_p->clauses != NULL) free_crule_clauses(rule_p->clauses);
		free(rule_p);
		mem_ctr--;
	}
}

/* free_crule_clauses: free up clauses in a rule */
void free_crule_clauses(CRULE_CLAUSE_PTR clause_p) {
	if (clause_p != NULL) {
		free_crule_clauses(clause_p->next_clause);
		free_crule_cond(clause_p->conditions);
		free_crule_op(clause_p->operations);
		free_rulepack(clause_p->rulepackages);
		free(clause_p);
		mem_ctr--;
	}
}

/* free_crule_cond: free up condition list */
void free_crule_cond(CRULE_COND_PTR cond_p) {
	if (cond_p != NULL) {
		free_crule_cond(cond_p->next_cond);
		if (cond_p->op_cat) {
			free(cond_p->op_cat);
			mem_ctr--;
		}
		if (cond_p->op_surf) {
			free(cond_p->op_surf);
			mem_ctr--;
		}
		free(cond_p);
		mem_ctr--;
	}
}

/* free_crule_op: free up operations list */
void free_crule_op(CRULE_OP_PTR op_p) {
	if (op_p != NULL) {
		free_crule_op(op_p->next_op);
		if (op_p->data) {
			free(op_p->data);
			mem_ctr--;
		}
		free(op_p);
		mem_ctr--;
	}
}

/* free rulepackages */
void free_rulepack(RULEPACK_PTR rulepack_p) {
	if (rulepack_p != NULL) {
		free_rulepack(rulepack_p->next_rule);
		if (rulepack_p->rulename != NULL) free(rulepack_p->rulename);
		mem_ctr--;
		free(rulepack_p);
		mem_ctr--;
	}
}

/* ******************** PRINT STRUCTURES ********************* */

/* print crules */
void print_crules(CRULE_PTR rulelist) {
	CRULE_PTR crule_tmp;
	CRULE_CLAUSE_PTR cclause_tmp;
	CRULE_COND_PTR crule_cond_tmp;
	CRULE_OP_PTR crule_op_tmp;
	char word_tmp[MAX_WORD];
	int clause_ctr;

	fprintf(debug_fp,"\n*** print crules ***\n");
	crule_tmp = rulelist;
	while (crule_tmp != NULL) {
		clause_ctr = 0;
		fprintf(debug_fp,"rulename: %s\n",crule_tmp->name);
		cclause_tmp = crule_tmp->clauses;
		while (cclause_tmp != NULL) {
			fprintf(debug_fp,"  clause %d\n",++clause_ctr);
			crule_cond_tmp = cclause_tmp->conditions;
			while (crule_cond_tmp != NULL) {
				decode_cond_code(crule_cond_tmp->cond_type,word_tmp);
				fprintf(debug_fp,"  condition type = %s\n",word_tmp);
				if (crule_cond_tmp->op_surf != NULL) {
					if ((crule_cond_tmp->cond_type == STARTSURF_CHK) ||
						(crule_cond_tmp->cond_type == NEXTSURF_CHK) ||
						(crule_cond_tmp->cond_type == STARTSURF_NOT) ||
						(crule_cond_tmp->cond_type == NEXTSURF_NOT)) {
						fprintf(debug_fp,"  condition operand 1 = %s\n",crule_cond_tmp->op_surf);
					}
				}
				if (crule_cond_tmp->op_cat != NULL) {
					if  (crule_cond_tmp->cond_type == MATCHCAT) {
						feat_path_decomp(crule_cond_tmp->op_cat,word_tmp);
						fprintf(debug_fp,"  condition operand 1 = %s\n",word_tmp);
					}
					else {
						fs_decomp(crule_cond_tmp->op_cat,word_tmp);
						fprintf(debug_fp,"  condition operand 1 = %s\n",word_tmp);
					}
				}
				crule_cond_tmp = crule_cond_tmp->next_cond;
			}
			/* get and print operations */
			crule_op_tmp = cclause_tmp->operations;
			while (crule_op_tmp != NULL) {
				decode_op_code(crule_op_tmp->op_type,word_tmp);
				fprintf(debug_fp,"  operation type = %s\n",word_tmp);
				if (crule_op_tmp->data != NULL) {
					if ((crule_op_tmp->op_type == ADD_CAT) ||
						(crule_op_tmp->op_type == SET_CAT))
						fs_decomp(crule_op_tmp->data,word_tmp);
					else
						feat_path_decomp(crule_op_tmp->data,word_tmp);
					fprintf(debug_fp,"  operand = %s\n",word_tmp);
				}
				crule_op_tmp = crule_op_tmp->next_op;
			}
			print_rulepacks(cclause_tmp->rulepackages);
			cclause_tmp = cclause_tmp->next_clause;
		}
		crule_tmp = crule_tmp->next_rule;
	}
}

void
print_rulepacks(RULEPACK_PTR rp_ptr) {
	RULEPACK_PTR rp_tmp;

	fprintf(debug_fp,"rulepackages:\n");
	rp_tmp = rp_ptr;
	while (rp_tmp != NULL) {
		fprintf(debug_fp,"\t%s\n",rp_tmp->rulename);
		rp_tmp = rp_tmp->next_rule;
	}
}
