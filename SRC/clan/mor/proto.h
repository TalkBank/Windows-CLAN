/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


extern "C"
{
/* ******************** mor prototypes ********************** */
/* *********************************************************** */
extern void hello_arrays(void);
extern void bye_arrays(void);
extern void CleanUpAll(char all);
extern void CloseFiles(void);
/* *******************  arules.c ***************** */
BOOL
markPostARules(void);
BOOL
read_arules(FILE *rulefile, FNType *fname);
BOOL
get_lexsurf(STRING *line,LEX_ENTRY_PTR lex_ptr);
BOOL
get_lexcat(STRING *line,LEX_ENTRY_PTR lex_ptr);
BOOL
get_allosurf(STRING *line,LEX_ENTRY_PTR lex_ptr,ALLO_ENTRY_PTR allo_ptr);
BOOL
get_allocat(STRING *line,ALLO_ENTRY_PTR allo_ptr);
BOOL
get_allostem(STRING *line,ALLO_ENTRY_PTR allo_ptr);
int
apply_arules(STRING *surface,FEATTYPE *cat,STRING *stem,STRING *trans,STRING *comp);
ARULES_PTR
m_arule_node(STRING *rulename);
ARULE_CLAUSE_PTR
m_clause_node(void);
LEX_ENTRY_PTR
m_lex_node(void);
ALLO_ENTRY_PTR
m_allo_node(void);
CAT_COND_PTR
m_cat_cond_rec(int cond_type,FEATTYPE *cond_fs);
CAT_OP_PTR
m_cat_op_rec(int op_type,FEATTYPE *op_fs);
void
free_ar_files(ARULE_FILE_PTR file_p);
void
free_arules(ARULES_PTR rule_p);
void
free_arule_clauses(ARULE_CLAUSE_PTR clause_p);
void
free_lex(LEX_ENTRY_PTR lex_p);
void
free_cat_cond(CAT_COND_PTR cond_p);
void
free_allos(ALLO_ENTRY_PTR allo_p);
void
free_cat_op(CAT_OP_PTR op_p);
void
print_arules(ARULE_FILE_PTR ar_files);
/* *******************  crules.c ***************** */
int
read_crules(FILE *rulefile);
BOOL
get_crule_conds(STRING *cond_name,STRING *line,CRULE_COND_PTR cond_ptr);
BOOL
get_crule_ops(STRING *line,CRULE_OP_PTR op_ptr);
BOOL
put_startrule(CRULE_PTR rule_ptr);
BOOL
put_rulepack(STRING *line_ptr,CRULE_CLAUSE_PTR clause_ptr);
BOOL
put_endrule(CRULE_PTR rule_ptr);
void
check_rulepacks(void);
void
analyze_word(STRING *word,FEATTYPE *cat,STRING *parse,STRING *trans,STRING *comp,STRING *rest,RULEPACK_PTR rp,int ind, char doDef);
void free_lex_lett(LEX_PTR lex, int num);
void concat_word(STRING *,STRING *,STRING *,FEATTYPE *,STRING *,STRING *,STRING *,STRING *,FEATTYPE *,STRING *,STRING *,STRING *,RULEPACK_PTR,STRING *,int);
BOOL
apply_endrules(STRING *surf, FEATTYPE *cat, STRING *stem);
CRULE_PTR
m_crule_node(STRING *rulename);
CRULE_CLAUSE_PTR
m_crule_clause_node(void);
CRULE_COND_PTR
m_crule_cond_rec(void);
CRULE_OP_PTR
m_crule_op_rec(void);
RULEPACK_PTR
m_rulepack_rec(void);
void
free_crules(CRULE_PTR rule_p);
void
free_crule_clauses(CRULE_CLAUSE_PTR clause_p);
void
free_crule_cond(CRULE_COND_PTR cond_p);
void
free_crule_op(CRULE_OP_PTR op_p);
void
free_rulepack(RULEPACK_PTR rulepack_p);
void
print_crules(CRULE_PTR rulelist);
void
print_rulepacks(RULEPACK_PTR rp_ptr);
/* *******************  drules.c ***************** */
BOOL
read_drules(FILE *rulefile);
BOOL
get_drule_conds(STRING *cond_name,STRING *line,CRULE_COND_PTR cond_ptr);
BOOL
apply_drules(STRING *surf,FEATTYPE *cat,
	     RESULT_REC_PTR prev_stack,int num_prev,
	     RESULT_REC_PTR next_stack,int num_next);
DRULE_PTR
m_drule_node(STRING *rulename);
void
free_drules(DRULE_PTR rule_p);
void
print_drules(DRULE_PTR rulelist);
/* *********************** lextrie.c ************************* */
void
delete_entry(TRIE_PTR node, FEATTYPE *entry);
int
process_lex_entry(STRING *entry, char isComp, FNType *mFileName, long ln);
TRIE_PTR
m_trie_node(void);
LETTER_PTR
m_letter_node(int letter);
ELIST_PTR 
m_entry_node(FEATTYPE *entry, const STRING *stem, const STRING *trans, const STRING *comp);
int
add_entry( const char *org_word,TRIE_PTR node,FEATTYPE *entry,const STRING *stem,const STRING *trans,const STRING *comp,char isTempItem);
void
enter_word(const STRING *word,FEATTYPE *entry,const STRING *stem,const STRING *trans,const STRING *comp,char isTempItem);
void
delete_word(STRING *word,FEATTYPE *entry);
ELIST_PTR 
lookup(STRING *word);
//25-5-99 long print_trie(TRIE_PTR root);
ELIST_PTR
free_entries(ELIST_PTR entry_list);
void
free_trie(TRIE_PTR root);
int
find_letter(TRIE_PTR cur_node,int letter);
int
add_letter(TRIE_PTR cur_node, int letter);
FEAT_PTR
access_feat(FEAT_PTR *root_node_ptr, STRING *name);
/* *******************  morlib.c ***************** */
FEATTYPE *
retrieve_fvp(FEATTYPE *fvp, FEATTYPE *path, FEATTYPE *cat);
void
Error_fvp(FEATTYPE *data, FEATTYPE *NextCatRule, STRING *item,
		STRING *s_surf, FEATTYPE *s_cat, FEATTYPE *n_cat, STRING *s_parse, STRING *n_stem,
		CRULE_PTR cur_rule, int clause_ctr, CRULE_COND_PTR cur_cond, CRULE_OP_PTR cur_op, FEATTYPE *r_cat);
BOOL
legal_char(char c);
void
mor_mem_error(int code, const STRING *message);
void
mem_usage(void);
int
get_line(STRING *line,int max_len,FILE *infile,int *lines_read);
STRING *get_word(STRING *line,STRING *keyword);
STRING *get_lex_word(STRING *line,STRING *keyword);
STRING *get_pattern(STRING *line,STRING *pattern);
STRING *get_cat(STRING *line,STRING *cat);
STRING *get_feats(STRING *line, FEATTYPE *feat);
BOOL
get_var_decl(STRING *var,STRING *rest);
STRING *get_feat_path(STRING *line,FEATTYPE *feat);
BOOL
get_vname(FEATTYPE *fs,STRING *fname,STRING *vname);
STRING *skip_blanks(STRING *line);
STRING *skip_equals(STRING *line);
void
decode_cond_code(int code,STRING *code_name);
void
decode_op_code(int code,STRING *code_name);

// lxs new features type supporting functions
FEATTYPE *
featrchr(FEATTYPE *ft, char c);
FEATTYPE *
featchr(FEATTYPE *ft, char c);
void
featncpy(FEATTYPE *c_fs,FEATTYPE *tmp_str, long num);
void
featcpy(FEATTYPE *c_fs,FEATTYPE *tmp_str);
void
featcat(FEATTYPE *c_fs,FEATTYPE *tmp_str);
int
featncmp(FEATTYPE *f1, FEATTYPE *f2, long num);
int
featcmp(FEATTYPE *f1, FEATTYPE *f2);
int
partfeatcmp(FEATTYPE *full,FEATTYPE *part);
long
featlen(FEATTYPE *features);
/* ******************* (was patmat.c) ***************** */
void
test_pattern(STRING *cur_text,STRING *pat,STRING **stack_ptr);
BOOL
match_word(STRING *word,STRING *pat,int index);
void
record_match(STRING *text);
void
match_naive(STRING *text,STRING *pat);
BOOL
gen_word(STRING *pat,STRING *output);
BOOL
expand_pat(STRING *in_pat,STRING *out_pat);
BOOL
store_pat(STRING *var_name,STRING *var_pat);
/* ***************************** (was fs.c) *********************** */
BOOL
fs_comp(STRING *big_fs, FEATTYPE *c_fs);
FEAT_PTR
m_feat_node(STRING *name);
FEATTYPE
get_feat_code(STRING *name);
STRING *get_feat_name(int code);
BOOL
fs_decomp(FEATTYPE *c_fs,STRING *big_fs);
BOOL
feat_path_decomp(FEATTYPE *c_path,STRING *big_path);
BOOL
check_fvp(FEATTYPE *cat,FEATTYPE *fvp);
BOOL
add_fvp(FEATTYPE *cat,FEATTYPE *fvp);
BOOL
remove_fvp(FEATTYPE *cat,FEATTYPE *fvp);
BOOL
match_fvp(FEATTYPE *cat_1, FEATTYPE *cat_2, FEATTYPE *path);
void
free_values(VAL_PTR root);
void
free_feats(FEAT_PTR root);
/* *********************************************************** */
}
