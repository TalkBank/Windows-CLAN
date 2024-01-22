/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* morlib.c - basic library functions used by parser */

#include "cu.h"
#include "morph.h"

/* FS-PARSE STATES */

#define PARSE_FEAT 0
#define PARSE_VAL  1

/* FS-TYPES */

#define ATOMIC 0
#define OR 1
#define LIST 2
#define STR 3

extern long mem_ctr;
extern FILE *lex;
extern FILE *fpout;
extern int feat_node_ctr;
extern void cutt_exit(int i);
extern void out_of_mem(void);

/* legal name-  made up of legal characters */
/* set up this alphabet here */

/* remember!  update N_EXCHARS if updating exchar_tab!! */
char punc_chars[] = 
{ '{', '}', '[', ']', ',', '\t', '\n', ' ', '"', COMMENT_CHAR, EOS};   /* EOS? */
#define N_PUNC 11

/* legal chars == anything other than punctuation chars */
BOOL
legal_char(char c) {
	int i;
	for (i=0; i<N_PUNC; i++){
		if (c == punc_chars[i])
			return (FALSE);
	}
	/* if here, it's OK */
	return (TRUE);
}

/* mor_mem_error:  invoked when can't malloc more memory */
/* free up malloc'ed space, print message, exit      */
void
mor_mem_error(int code, const STRING *message) {
//  extern FEAT_PTR *feat_codes;
	CleanUpAll(TRUE);
	CloseFiles();
/* lxs 24-04-97
	for (i=0; i<feat_node_ctr; i++) {
		if (feat_codes[i])
		free(feat_codes[i]);
	}
*/
	fprintf(stderr,"\nmemory allocation so far:\n");
	mem_usage();
	fprintf(stderr, " (not counting records allocated for arules and crules)\n");
	fprintf(stderr,"\nERROR:\n%s\n",message);
	cutt_exit(0);
}
void
mem_usage()
{
	extern long trie_node_ctr;
	extern long letter_ctr;   /* number of letter nodes */
	extern long trie_entry_ctr;
	extern int val_node_ctr;  /* number of value nodes */
	
	fprintf(stderr,"memory allocation so far:\n");
	fprintf(stderr, " allocated %ld nodes and %ld letters for lexicon\n",
			trie_node_ctr,letter_ctr);
	fprintf(stderr, " allocated %ld entries for lexicon\n",trie_entry_ctr);
	fprintf(stderr, " allocated %d records for feature table\n",feat_node_ctr);
	fprintf(stderr, " allocated %d records for value tables\n",val_node_ctr);
}

static void removeMorTierSpaces(char *line) {
	int i;

	for (i=0; line[i] != EOS && line[i] != '\n'; i++) {
		if (line[i] == '|' && i > 0 && isSpace(line[i-1])) {
			strcpy(line+i-1, line+i);
			i -= 2;
		}
		if (line[i] == '|' && isSpace(line[i+1])) {
			strcpy(line+i+1, line+i+2);
			i--;
		}
	}
}

static void removeSpacesFromVars(char *line) {
	int  i, sqf;

	sqf = 0;
	for (i=0; line[i] != EOS && line[i] != '\n' && line[i] != '%'; i++) {
		if (line[i] == '[')
			sqf++;
		else if (line[i] == ']' && sqf > 0)
			sqf--;
		else if (isSpace(line[i]) && sqf > 0) {
			strcpy(line+i, line+i+1);
			i--;
		}
	}
}

/* get_line - read a statement (or entry) from the input file via fgets */
/* 1 statement line can be many textfile lines */
/* strip newline character, trim trailing blanks, comments */
/* arguments: 
   line - place to store entire statement read
   max_len - size of line
   infile - where to read from
   lines_read - how many textfile lines were read */
/* return EOF when end of file reached */
/* returns -2 if problems encountered */
/* else returns 0 */  /* probably should get this down to EOF, SUCCEED, FAIL */
int
get_line(STRING *line, int max_len, FILE *infile, int *lines_read) {
	char inln[MAXLINE], *s;
	STRING *line_ptr;
	STRING *tmp_index;
	BOOL cont = FALSE;
	
	*lines_read = 0;
	line[0] = EOS;
	line_ptr = line;

readline:
	if (fgets_cr(inln,MAXLINE,infile) == NULL){   /* EOF reached */
		if (cont == TRUE) {
			ERROR_MOR("get_line: file ends with continuation line",-2,"");
		} else
			return(EOF);
	} else {    /* process line */
		removeMorTierSpaces(inln);
		/* strip blank lines and comment lines */
		for (s=inln; *s == ' ' || *s == '\n' || *s == '\t'; s++) ;
		if (uS.isUTF8(inln) || uS.isInvisibleHeader(inln))
			goto readline;
		(*lines_read)++;
		if (*s == EOS || *s == COMMENT_CHAR)
			goto readline;
		if ((strlen(line) + strlen(inln) + 3) <= (unsigned)max_len) {
			/* statement fits entry */
			cont = FALSE;  /* reset cont, if need be */
			strcat(line,inln);
			line_ptr = strchr(line,'\n');  /* goto end of line */
			if (line_ptr == NULL)
				line_ptr = line + strlen(line);
			else
				*line_ptr = EOS; /* over-write newline */
			while (*(line_ptr-1) == ' ')
				line_ptr--;
			*line_ptr = EOS; /* over-write blanks */
			
			if ((strlen(line) > 0) && (*(--line_ptr) == CONTINUE_CHAR)) {  /* check if more to follow */
				cont = TRUE;
				*line_ptr = ' ';
			}
			
			if ((tmp_index = strchr(line,COMMENT_CHAR)) != NULL) {
				*tmp_index = EOS;
				line_ptr = --tmp_index;
			}
			
			if (cont == TRUE)
				goto readline;   /* keep reading in lines */
			else
				return(0);
		} else{  /* statement too big */ /* this shouldn't happen */
			/* skip remaining lines in statement, if any, return -2 */
			fprintf(stderr, "\nCrules line is too long:\n");
			fprintf(stderr, "%s\n", line);
			CleanUpAll(TRUE);
			CloseFiles();
			cutt_exit(0);
			return(0);
		}
	}
}

/* get_word - parse word off line */
/* return NULL if problems encountered */
/* else return place in string after word */
STRING *get_word(STRING *line, STRING *keyword) {
	char *word_beg, *word_end;
	int tmp_len;
	
	keyword[0] = EOS;
	
	word_beg = line;
	if (*word_beg  == EOS)
		return(NULL);
	else {
		word_beg = skip_blanks(word_beg);
		word_end = word_beg;
		while ((*(++word_end) != EOS) && (legal_char(*word_end) == TRUE))
			;
		tmp_len = word_end - word_beg;
		strncpy(keyword,word_beg,tmp_len);
		keyword[tmp_len] = EOS;
		return(word_end);
	}
}

/* get_word - parse word off line */
/* return NULL if problems encountered */
/* else return place in string after word */
STRING *get_lex_word(STRING *line, STRING *keyword) {
	char *word_beg, *word_end, qt;
	int tmp_len;
	
	keyword[0] = EOS;
	
	if (*line == '"') {
		qt = TRUE;
		word_beg = line+1;
	} else {
		qt = FALSE;
		word_beg = line;
	}
	if (*word_beg  == EOS)
		return(NULL);
	else {
		if (qt) {
			word_end = word_beg;
			while ((*(++word_end) != EOS) && *word_end != '"') ;
		} else {
			word_beg = skip_blanks(word_beg);
			word_end = word_beg;
			while ((*(++word_end) != EOS) && (legal_char(*word_end) == TRUE)) ;
		}
		tmp_len = word_end - word_beg;
		strncpy(keyword,word_beg,tmp_len);
		keyword[tmp_len] = EOS;
		if (qt && *word_end == '"')
			word_end++;
		return(word_end);
	}
}

/* get_pattern - parse pattern off input line */
/* return NULL if problems encountered */
/* else return place in string after word */
STRING *get_pattern(STRING *line, STRING *pattern) {
	char *word_beg, *word_end;
	int tmp_len;

	pattern[0] = EOS;
	word_beg = line;
	if (*word_beg  == EOS)
		return(NULL);
	else{
		word_beg = skip_blanks(word_beg);
		word_end = word_beg;
		while ((*word_end != ' ') && (*word_end != '\t') && (*word_end != '\n') && (*word_end != EOS))
			++word_end;
		tmp_len = word_end - word_beg;
		if (tmp_len > MAX_PATLEN) {
			fprintf(stderr,"ERROR: pattern too long: %s",line);
			return(NULL);
		} else {
			strncpy(pattern,word_beg,tmp_len);
			pattern[tmp_len] = EOS;
			return(word_end);
		}
	}
}

STRING *get_cat(STRING *line, STRING *cat) {
	char *cat_beg, *cat_end;
	int tmp_len;
	int num_braces = 0;

	cat[0] = EOS;
	cat_beg = line;
	if (*cat_beg  == EOS)
		return(NULL);
	else{
		cat_beg = skip_blanks(cat_beg);
		cat_end = cat_beg;
		
		if (*cat_beg != '{') /* not beginning of cat spec */
			return(NULL);
		else
			++num_braces;
		
		while (*++cat_end != EOS) {
			if (*cat_end == '{')
				++num_braces;
			else if (*cat_end == '}') {
				--num_braces;
				if (num_braces == 0) {
					tmp_len = cat_end - cat_beg + 1;
					strncpy(cat,cat_beg,tmp_len);
					cat[tmp_len] = EOS;
					return(++cat_end);
				}
			}
		} /* end while */
		return(NULL);  /* shouldn't get to this point */
	}
}

/* get_feats parses feat val pair off of line */
/* feat val pair must be in form: [feat val] */
STRING *get_feats(STRING *line, FEATTYPE *feat) {
	char *feat_beg, *feat_end;
	char tmp_feat[MAXCAT];  /* place to put uncompressed string */
	int tmp_len;
	int num_brackets = 0;

	feat[0] = 0;
	feat_beg = line;
	if (*feat_beg  == EOS) {
		fprintf(stderr,"*ERROR* no feature value pair specified\n");
		return(NULL);
	} else {
		feat_beg = skip_blanks(feat_beg);
		feat_end = feat_beg;

		if (*feat_beg != '[') {/* not beginning of feature value pair */
			fprintf(stderr,"*ERROR* no beginning of feature value pair specified: %s\n", line);
			return(NULL);
		} else
			++num_brackets;
		while (*++feat_end != EOS){
			if (*feat_end == '[')
				++num_brackets;
			else if (*feat_end == ']') {
				--num_brackets;
				if (num_brackets == 0) {
					/* get the feature value pair */
					tmp_len = feat_end - feat_beg + 1;
					if (tmp_len >= MAXCAT) {
						fprintf(stderr,"*ERROR* Too many catigories:\n");
						fprintf(stderr,"%s\n", line);
						return(NULL);
					}
					strncpy(tmp_feat,feat_beg,tmp_len);
					tmp_feat[tmp_len] = EOS;
					/* compress it */ /* fs_comp does some syntax checking */
					if (fs_comp(tmp_feat,feat) == SUCCEED)
						return(++feat_end);
					else {
						fprintf(stderr,"*ERROR* can't find specified feature value pair: %s\n", line);
						return(NULL);
					}
				}
			}
		} /* end while */
		fprintf(stderr,"*ERROR* Internal error: %s\n", line);
		return(NULL);  /* shouldn't get to this point */
	}
}

BOOL
get_var_decl(STRING *var, STRING *rest) {
	char var_name[MAX_VARNAME];
	char pattern[MAX_PATLEN];
	STRING *tmp_ptr;
	tmp_ptr = rest;

	if (strlen(var) >= MAX_VARNAME) {
		fprintf(stderr,"\nVariable name is longer than %d characters\n", MAX_VARNAME-1);
		return(FAIL);
	}
	strcpy(var_name,var);
	tmp_ptr = skip_equals(tmp_ptr);
	removeSpacesFromVars(tmp_ptr);
	if (get_pattern(tmp_ptr,pattern) == NULL)
		return(FAIL);
	if (store_pat(var_name,pattern) != 1)
		return(FAIL);
	else
		return(SUCCEED);
}

/* get_feat_path -  get specification of feature name (or path?) */
STRING *get_feat_path(STRING *line, FEATTYPE *feat) {
	FEATTYPE tmp_path[MAX_WORD];
	char tmp_feat[MAX_WORD];
	char *line_ptr;
	FEATTYPE *path_ptr;

	line_ptr = line;
	path_ptr = tmp_path;
	if (*line_ptr  == EOS)
		return(NULL);
	else{
		if (*line_ptr != '[') /* not beginning of feature value pair */
			return(NULL);
		++line_ptr;
		while (*line_ptr != ']'){
			line_ptr = get_word(line_ptr,tmp_feat);
			*path_ptr++ = 'f';
			*path_ptr++ = get_feat_code(tmp_feat);
			line_ptr = skip_blanks(line_ptr);
			if (*line_ptr == EOS)
				return(NULL);
		}
		if (*line_ptr == ']'){
			*path_ptr = EOS;
			featcpy(feat,tmp_path);
			return(++line_ptr);
		}
	}
	return(NULL);
}

/* get_fvp */
/* pull out first feature value pair in a set */
/* copy substring into first_fvp */
/* return pointer to point after end of first_fvp */
static FEATTYPE *
get_fvp(FEATTYPE *first_fvp, int *fvp_type, FEATTYPE *fvp_set) {
	FEATTYPE *f_ptr;
	int fvp_len;

	f_ptr = fvp_set;
	if (*f_ptr == EOS)
		return(NULL);
	while (*f_ptr == (FEATTYPE)'f')
		f_ptr += 2;
	switch(*f_ptr){
		case (FEATTYPE)'v':
			*fvp_type = ATOMIC;
			f_ptr += 2;
			break;
		case (FEATTYPE)'o':
			*fvp_type = OR;
			while (*(f_ptr) == 'o')
				f_ptr += 2;
			break;
		case (FEATTYPE)'l':
			*fvp_type = LIST;
			while (*(f_ptr) == 'l')
				f_ptr += 2;
			break;
		case (FEATTYPE)'"':
			*fvp_type = STR;
			f_ptr++;
			f_ptr = (featrchr(f_ptr,'"'));
			f_ptr++;
			break;
		default:
			break; /* shouldn't get here */
	}
	fvp_len = f_ptr - fvp_set; // lxs lxslxs big check
	featncpy(first_fvp,fvp_set,fvp_len);
	first_fvp[fvp_len] = 0;
	return(f_ptr);
}

/* take path (feature name) and find values from fs cat */
/* copy into string fvp */
/* return pointer to start of next fvp in fs, or NULL, if not found */
FEATTYPE *
retrieve_fvp(FEATTYPE *fvp, FEATTYPE *path, FEATTYPE *cat) {
	FEATTYPE *fvp_ptr;
	FEATTYPE fvp_tmp[MAXCAT];
	int fvp_type;

	fvp_ptr = cat;
	while ((fvp_ptr = get_fvp(fvp_tmp,&fvp_type,fvp_ptr)) != NULL){
		if (featncmp(fvp_tmp,path,featlen(path)) == 0){
			featcpy(fvp,fvp_tmp);
			return(fvp_ptr);
		}
	}
	return(NULL);
}

/* get value for feature from feature structure, uncompressed */
/* don't use this for complex path names */
BOOL
get_vname(FEATTYPE *fs, STRING *featname, STRING *valname) {
	FEATTYPE fvp_tmp[MAX_WORD];
	FEATTYPE path_tmp[MAX_WORD];
	char big_tmp[MAX_WORD];
	int val_end;
	STRING *tmp_ptr;

	valname[0] = EOS;
	get_feat_path(featname, path_tmp);
	if (retrieve_fvp(fvp_tmp,path_tmp,fs) != NULL) {
		fs_decomp(fvp_tmp,big_tmp);
		/* returns : "{[scat x]}" */
		tmp_ptr = strchr(big_tmp,' ');
		tmp_ptr++;
		strcpy(valname,tmp_ptr);
		/* strip off close bracket, brace */
		val_end = strlen(valname) - 2;
		valname[val_end] = EOS;
		return(true);
	}
	return(false);
}

STRING *skip_blanks(STRING *line) {
  STRING *tmp_ptr;
  tmp_ptr = line;

  while ((*tmp_ptr == ' ') || (*tmp_ptr == '\t'))
    tmp_ptr++;
  return(tmp_ptr);
}

STRING *skip_equals(STRING *line) {
  STRING *tmp_ptr;
  tmp_ptr = line;

  while ((*tmp_ptr == ' ') || (*tmp_ptr == '\t') || (*tmp_ptr == '='))
    tmp_ptr++;
  return(tmp_ptr);
}

void
decode_cond_code(int code, STRING *code_name) {
  switch(code){
  case LEXSURF_CHK:
    strcpy(code_name,"CHECK LEXSURF");
    break;
  case LEXSURF_NOT:
    strcpy(code_name,"CHECK LEXSURF NOT");
    break;
  case LEXCAT_CHK:
    strcpy(code_name,"CHECK LEXCAT");
    break;
  case LEXCAT_NOT:
    strcpy(code_name,"CHECK LEXCAT NOT");
    break;
  case STARTSURF_CHK:
    strcpy(code_name,"CHECK STARTSURF");
    break;
  case NEXTSURF_CHK:
    strcpy(code_name,"CHECK NEXTSURF");
    break;
  case STARTSURF_NOT:
    strcpy(code_name,"CHECK STARTSURF NOT");
    break;
  case NEXTSURF_NOT:
    strcpy(code_name,"CHECK NEXTSURF NOT");
    break;
  case STARTCAT_CHK:
    strcpy(code_name,"CHECK STARTCAT");
    break;
  case NEXTCAT_CHK:
    strcpy(code_name,"CHECK NEXTCAT");
    break;
  case STARTCAT_NOT:
    strcpy(code_name,"CHECK STARTCAT NOT");
    break;
  case NEXTCAT_NOT:
    strcpy(code_name,"CHECK NEXTCAT NOT");
    break;
  case MATCHCAT:
    strcpy(code_name,"MATCHCAT");
    break;
  case PREVSURF_CHK:
    strcpy(code_name,"CHECK PREVSURF");
    break;
  case PREVCAT_CHK:
    strcpy(code_name,"CHECK PREVCAT");
    break;
  case PREVSURF_NOT:
    strcpy(code_name,"CHECK PREVSURF NOT");
    break;
  case PREVCAT_NOT:
    strcpy(code_name,"CHECK PREVCAT NOT");
    break;
  case CURSURF_CHK:
    strcpy(code_name,"CHECK CURSURF");
    break;
  case CURCAT_CHK:
    strcpy(code_name,"CHECK CURCAT");
    break;
  case CURSURF_NOT:
    strcpy(code_name,"CHECK CURSURF NOT");
    break;
  case CURCAT_NOT:
    strcpy(code_name,"CHECK CURCAT NOT");
    break;
  default:
    strcpy(code_name,"\?\?");
    break;
  }
}

void
decode_op_code(int code, STRING *code_name) {
  switch(code){
  case COPY_LEXCAT:
    strcpy(code_name,"COPY LEXCAT");
    break;
  case COPY_STARTCAT:
    strcpy(code_name,"COPY STARTCAT");
    break;
  case COPY_NEXTCAT:
    strcpy(code_name,"COPY NEXTCAT");
    break;
  case ADD_CAT:
    strcpy(code_name,"ADD");
    break;
  case DEL_CAT:
    strcpy(code_name,"DEL");
    break;
  case SET_CAT:
    strcpy(code_name,"SET");
    break;
  case ADD_START_FEAT:
    strcpy(code_name,"ADD STARTCAT");
    break;
  case ADD_NEXT_FEAT:
    strcpy(code_name,"ADD NEXTCAT");
    break;
  default:
    strcpy(code_name,"\?\?");
    break;
  }

}
/* ************************************************************************** */
/* patmat.c - library of pattern matching functions                           */
/* ************************************************************************** */

/* test_pattern - match pattern against text substring */
/* success = reaching end of pattern - record endpoints of pattern */
/* calls itself recursively (easy way to handle wildcards) */
/* recognizes:   "." (match anything once) */
/*           ".*" (match anything 0 or more times) */
/*           "[xyz]"  (match one of a set) */
/*           "[^xyz]" (match complement of a set) */
/* note- only records end of pattern in string */
/* must keep track of beginning of match elsewhere */

//lxslxs UTF lxs

static char isSingleChar(STRING *st) {
	st++;
	if (*st != EOS) {
//		if (dFnt.isUTF) {
			short res;

			while (*st != EOS) {
				res = my_CharacterByteType(st, 0, &dFnt);
				if (res == 0 || res == -1)
					return(FALSE);
				st++;
			}
//		} else
//			return(FALSE);
	}
	return(TRUE);
}

static char isMatchedChar(STRING *st1, STRING *st2) {
	if (*st1 == *st2) {
//		if (dFnt.isUTF) {
			short res1 = 0, res2 = 0;

			st1++;
			st2++;
			while (*st1 != EOS && *st2 != EOS) {
				res1 = my_CharacterByteType(st1, 0, &dFnt);
				res2 = my_CharacterByteType(st2, 0, &dFnt);

				if (res1 == 0 || res1 == -1)
					break;
				if (res2 == 0 || res2 == -1)
					break;

				if (*st1 != *st2)
					return(FALSE);

				st1++;
				st2++;
			}
			if ((res1 == 0 || res1 == -1) && (res2 == 0 || res2 == -1))
				return(TRUE);
			if (*st1 == EOS && *st2 != EOS) {
				res2 = my_CharacterByteType(st2, 0, &dFnt);
				if (res2 == 0 || res2 == -1)
					return(TRUE);
			}
			if (*st1 != EOS && *st2 == EOS) {
				res1 = my_CharacterByteType(st1, 0, &dFnt);
				if (res1 == 0 || res1 == -1)
					return(TRUE);
			}
			if (*st1 == EOS && *st2 == EOS)
				return(TRUE);
//		} else
//			return(TRUE);
	}
	return(FALSE);
}

static STRING *advanceChar(STRING *st) {
	st++;
//	if (dFnt.isUTF) {
		short res;

		while (*st != EOS) {
			res = my_CharacterByteType(st, 0, &dFnt);
			if (res == 0 || res == -1)
				break;
			st++;
		}
//	}
	return(st);
}

void
test_pattern(STRING *cur_text, STRING *pat, STRING **stack_ptr) {
    /* begin point in stack for matches on this pattern */
	extern STRING **next_match;
	extern STRING **match_stack;
	STRING **stack_tmp;
	STRING *s_tmp;

	if (*pat == EOS) { /* reached end of pattern */
		/* record endpoints (only if they are unique) */
		for (stack_tmp = stack_ptr; stack_tmp < next_match; stack_tmp++) {
			if (*stack_tmp == cur_text)
				break;
		}
		/* not in list- add & increment next_match */
		if (stack_tmp == next_match) {
			if (next_match == &match_stack[MAX_MATCHES-1]) {
/* NLD nld
				int i;
				fprintf(stderr, "cur_text=%s; pat=%s;\n", cur_text, pat);
				for (i=0; i < MAX_MATCHES; i++)
					fprintf(stderr, "match_stack[%d]=%s;\n", i, match_stack[i]);
*/
				fprintf(stderr, "\nprogram error: pattern match subsystem stack overflow\n");
				CleanUpAll(TRUE);
				CloseFiles();
				cutt_exit(0);
			} else
				*next_match++ = cur_text;
		}
	} else if (*cur_text != EOS){  /* more to match */
		/* match 0 or more chars */
		if ((*pat == '.') && (*(pat+1) == '*')){
			/* wildcard matches 0 char-  advance pattern ptr, don't advance text */
			pat = pat+2;
			test_pattern(cur_text,pat,stack_ptr);  
			/* wildcard matches 1 char-  advance both ptrs 1 */
			cur_text = advanceChar(cur_text);
			test_pattern(cur_text,pat,stack_ptr); /* pat advanced above */
//			test_pattern(++cur_text,pat,stack_ptr); /* pat advanced above */
			/* keep matching on wildcard-  advance cur_text, leave pat alone */
			pat = pat-2;
			test_pattern(cur_text,pat,stack_ptr); /* undo pat ptr incr */
		} else if (*pat == '.') { /* match any one char */

			cur_text = advanceChar(cur_text);
			pat = advanceChar(pat);
			test_pattern(cur_text,pat,stack_ptr);


//			test_pattern(++cur_text,++pat,stack_ptr);
		} else if ((*pat == '[') && (*(pat+1) == '^')) { /* match complement of a set */
//			s_tmp = pat++; /* move onto '^' */
			s_tmp = pat; /* move onto '^' */
			pat = advanceChar(pat);
			s_tmp = advanceChar(s_tmp);
			while ((*s_tmp != ']') && (*s_tmp != EOS)) { /* walk through set */
				if (isMatchedChar(cur_text, s_tmp)) 
//				if (*cur_text == *s_tmp) 
		  			break;
		  		s_tmp = advanceChar(s_tmp);
		  	}
			if (*s_tmp == ']') { /* reached end of choice - didn't match */
				cur_text = advanceChar(cur_text);
				s_tmp = advanceChar(s_tmp);
				test_pattern(cur_text,s_tmp,stack_ptr);
			}
//				test_pattern(++cur_text,++s_tmp,stack_ptr);
			else if (*s_tmp == EOS)
				fprintf(stderr,"ERROR: bad pattern specification: %s\n",pat);
		} else if (*pat == '[') { /* match one of a set */
			s_tmp = pat;
			s_tmp = advanceChar(s_tmp);
			while ((*s_tmp != ']') && (*s_tmp != EOS)) {
				if (isMatchedChar(cur_text, s_tmp)) 
//				if (*cur_text == *s_tmp) 
					break;
				s_tmp = advanceChar(s_tmp);
			}
			if (!((*s_tmp == ']') || (*s_tmp == EOS))) { /* matched somewhere in set */
				while ((*++s_tmp != ']') && (*s_tmp != EOS))
		  			;
				if (*s_tmp == ']') {/* end of choice */
					cur_text = advanceChar(cur_text);
					s_tmp = advanceChar(s_tmp);
					test_pattern(cur_text,s_tmp,stack_ptr);
				}
//					test_pattern(++cur_text,++s_tmp,stack_ptr);
			} else if (*s_tmp == EOS)
				fprintf(stderr,"ERROR: bad pattern specification: %s\n",pat);
		} else if (isMatchedChar(pat, cur_text)) { /* match characters */
			cur_text = advanceChar(cur_text);
			pat = advanceChar(pat);
			test_pattern(cur_text,pat,stack_ptr);  /* tail recursion */
//			test_pattern(++cur_text,++pat,stack_ptr);  /* tail recursion */
		}
	}
}

/* match_pat - match text string against pattern */
/* pattern may be disjunction of several patterns */
/* disjunction symbol "|" */
/* pattern may contain metacharacters */
static void
match_pat(STRING *text, STRING *pat, STRING **stack_ptr) {
	STRING *pat_p;
	STRING *endpat_p;
	char sub_pat[MAX_PATLEN];
	int sub_len;

	pat_p = pat;
	while (pat_p != NULL) {
		endpat_p = (strchr(pat_p,'|'));
		if (endpat_p){
			/* grab subpattern - match text to subpattern */
			sub_len = endpat_p - pat_p;
			strncpy(sub_pat,pat_p,sub_len);
			sub_pat[sub_len] = EOS;
			test_pattern(text,sub_pat,stack_ptr);
			pat_p = ++endpat_p;  /* skip over '|' in string */
		} else{ 
			test_pattern(text,pat_p,stack_ptr);
			pat_p = endpat_p;
		}
	}
}

/* match_word - match a word against a pattern */
/* pattern may contain bracked substrings (variable instantiations) */
/* returns 1 if word matches pattern, otherwise returns 0 */
BOOL
match_word(STRING *word, STRING *pat, int index) {
	extern STRING **match_stack;
	extern STRING **next_match;
	extern SEG_REC *word_segments;
	STRING **stack_beg, **stack_end, **tmp;
	STRING *seg_beg, *seg_end, *seg_mid, *word_p;
	char segment[MAX_PATLEN];
	int seg_len, label, var_index, i, res;

	if (index == 0) {  /* initailize workspaces on first time through */
		next_match = &match_stack[0];
		/* initialize workspace used to analyze word surface */
		for (i = 0; ((i < MAX_SEGS) && (word_segments[i].index != EMPTY)) ; i++) {
			word_segments[i].index = EMPTY;
		}
	}
	if ((*word == EOS) && (*pat == EOS))
		return(SUCCEED);
	else if ((*word == EOS) || (*pat == EOS))
		return(FAIL);
	else { /* match word against pattern */
		/* process next segment in pattern */
		seg_beg = pat;

		/* if pattern begins with '\' we're processing a variable */
		if (*seg_beg == '\\') {
			label = TRUE;
			++seg_beg;
			var_index = *seg_beg - '0';
			++seg_beg;
		} else {
			label = FALSE;
			var_index = -1;
		}
		seg_end = (strchr(seg_beg,'\\'));
		if (seg_end) {
			/* grab subpattern - match text to subpattern */
			if ((seg_mid=strchr(seg_beg,'<')) != NULL && seg_mid < seg_end)
				seg_len = seg_mid - seg_beg;
			else
				seg_len = seg_end - seg_beg;
			strncpy(segment,seg_beg,seg_len);
			segment[seg_len] = EOS;
			/* skip over close pattern */
			if (label)
				seg_beg = ++seg_end;
			else
				seg_beg = seg_end;
		} else { /* processing rest of string */
			strcpy(segment,seg_beg);
			estr[0] = EOS;
			seg_beg = estr;
		}

		/* keep track of all matches found for this pattern */
		stack_beg = next_match;
		match_pat(word,segment,stack_beg);
		stack_end = next_match;

		// debug
/*
		if (1) {
			char tmp_str[MAX_PATLEN];
			int tmp_len;
			printf("word: %s\tpattern: %s\n",word,segment);
			for (tmp = stack_end; tmp-- > stack_beg;){
				if (dFnt.isUTF) {
					while (tmp >= stack_beg) {
						res = my_CharacterByteType(*tmp, 0, &dFnt);
						if (res == 0 || res == -1)
							break;
						tmp--;
					}
					if (tmp < stack_beg)
						break;
				}
				tmp_len = *tmp - word;
				strncpy(tmp_str,word,tmp_len);
				tmp_str[tmp_len] = EOS;
				printf("pattern matched with %s\n",tmp_str);
			}
		}
*/
		// end debug

		if (stack_beg == stack_end)  
			return(FAIL);  /* couldn't find a match for this pattern */
		else { /* keep going from here */
			for (tmp = stack_end; tmp-- > stack_beg;) {
//				if (dFnt.isUTF) {
					while (tmp >= stack_beg) {
						res = my_CharacterByteType(*tmp, 0, &dFnt);
						if (res == 0 || res == -1)
							break;
						tmp--;
					}
					if (tmp < stack_beg)
						break;
//				}
				/* anticipate success for this match - record what was found */
				if (index <= MAX_SEGS) {
					word_segments[index].end = *tmp;
					word_segments[index].index = var_index;
					/* put word pointer past pattern matched */
					word_p = *tmp;
					/* recurse */
					if (match_word(word_p,seg_beg,index+1))
						return(SUCCEED);
				} else {
				  fprintf(stderr,"\nerror in pattern matching subsystem: ");
				  fprintf(stderr,"too many sub-patterns in a word\n");
				  return (FAIL);
				}
		  }
		  /* if we made it to this point, couldn't match past this point */
		  return (FAIL);
		}
	}
}

static short FindIndex(char *pat_p, char *str) {
	STRING *endpat_p;
	char sub_pat[MAX_PATLEN], *s;
	int  sub_len, chr_cnt;
	short index = 0;

	while (pat_p != NULL) {
		endpat_p = (strchr(pat_p,'|'));
		if (endpat_p) {
			/* grab subpattern - match text to subpattern */
			sub_len = endpat_p - pat_p;
			strncpy(sub_pat,pat_p,sub_len);
			sub_pat[sub_len] = EOS;
			if (sub_pat[0] == '[' && sub_pat[1] != '^') {
				chr_cnt = 0;
				for (s=sub_pat+1; *s != ']';) {
					if (isMatchedChar(s, str) && isSingleChar(str))
						return(chr_cnt); // ((short)(s-sub_pat-1));
					s = advanceChar(s);
					chr_cnt++;
				}
			} else if (strcmp(sub_pat, str) == 0)
				return(index);
			pat_p = ++endpat_p;  /* skip over '|' in string */
		} else {
			if (pat_p[0] == '[' && pat_p[1] != '^') {
				chr_cnt = 0;
				for (s=pat_p+1; *s != ']';) {
					if (isMatchedChar(s, str) && isSingleChar(str))
						return(chr_cnt); // ((short)(s-pat_p-1));
					s = advanceChar(s);
					chr_cnt++;
				}
			} else if (strcmp(pat_p, str) == 0)
				return(index);
			pat_p = endpat_p;
		}
		index++;
	}
	return(-1);
}

 
/* record match goes through array word_segments */
/* if segment matched corresponds to some variable */
/* then that substring is copied into the var_tab */
void record_match(STRING *text) {
	extern SEG_REC *word_segments;
	extern VAR_REC *var_tab;
	STRING *tmp_start;
	char tmp_str[MAX_STRLEN];
	int tmp_len, index,i;

//lxslxs UTF lxs
	tmp_start = text;
	for(i=0 ;word_segments[i].index != EMPTY; i++) {
		if (word_segments[i].index >= 0) {
			/* grab segment */
			tmp_len = word_segments[i].end - tmp_start;
			strncpy(tmp_str,tmp_start,tmp_len);
			tmp_str[tmp_len] = EOS;
			index = word_segments[i].index;
			strcpy(var_tab[index].var_inst,tmp_str);
			var_tab[index].var_index = FindIndex(var_tab[index].var_pat, tmp_str);
		}
		tmp_start = word_segments[i].end;
	}
}


static void AddToWordIndex(STRING *word, STRING *pat_p, STRING *seg_mid, short index) {
	STRING *endpat_p;
	char sub_pat[MAX_PATLEN], *s;
	int  sub_len, i, res;

	endpat_p = strchr(pat_p,'|');
	if (endpat_p == NULL || endpat_p >= seg_mid) {
		if (pat_p[0] == '[' && pat_p[1] != '^') {
			for (s=pat_p+1, i=0; index > 0 && s[i] != ']'; i++) {
//				if (dFnt.isUTF) {
					res = my_CharacterByteType(s, i, &dFnt);
					if (res == 0 || res == -1)
						index--;
//				} else
//					index--;
			}
//			if (dFnt.isUTF) {
				while (s[i] != ']') {
					res = my_CharacterByteType(s, i, &dFnt);
					if (res == 0 || res == -1)
						break;
					i++;
				}
//			}
			s = s + i;
			if (*s != ']') {
				sub_len = strlen(word);
				word[sub_len++] = *s;
//				if (dFnt.isUTF) {
					for (i=1; s[i] != ']'; i++) {
						res = my_CharacterByteType(s, i, &dFnt);
						if (res == 0 || res == -1)
							break;
						else
							word[sub_len++] = s[i];
					}
					word[sub_len] = EOS;
//				} else
//					word[sub_len] = EOS;
			}
		} else if (index == 0 && pat_p[0] != '[')
			strncat(word, pat_p, seg_mid-pat_p);
	} else {
		while (pat_p != NULL && pat_p < seg_mid) {
			endpat_p = strchr(pat_p,'|');
			if (endpat_p && endpat_p < seg_mid) {
				/* grab subpattern - match text to subpattern */
				sub_len = endpat_p - pat_p;
				strncpy(sub_pat,pat_p,sub_len);
				sub_pat[sub_len] = EOS;
				if (index == 0) {
					strcat(word, sub_pat);
					return;
				}
				pat_p = endpat_p + 1;  /* skip over '|' in string */
			} else {
				if (index == 0) {
					strncat(word, pat_p, seg_mid-pat_p);
					return;
				}
				pat_p = NULL;
			}
			index--;
		}
	}
}

/* gen_word generates a word from an input pattern */
/* if that input pattern contains labeled variables */
/* then those variables are replaced by the string */
/* that most recently was matched against that pattern */
BOOL gen_word(STRING *pat, STRING *word) {
	extern VAR_REC *var_tab;
	char *pat_p, *tmp_p, *seg_end, *seg_mid;
	int index;

	pat_p = pat;
	while (pat_p != NULL) {
		/* if segment begins with '\' we've got a variable */
		if (*pat_p == '\\') {
			index = *++pat_p - '0';
			pat_p = advanceChar(pat_p);
			seg_end = strchr(pat_p,'\\');
			if ((seg_mid=strchr(pat_p,'<')) != NULL && seg_mid < seg_end) 
				index = *(seg_mid+1) - '0';
			if (seg_mid != NULL && seg_mid < seg_end && var_tab[index].var_index > -1) {
				AddToWordIndex(word, pat_p, seg_mid, var_tab[index].var_index);
			} else if (TRUE /*var_tab[index].var_inst*/)  /* pattern expands to something */
				strcat(word,var_tab[index].var_inst);
			else {
				fprintf(stderr,"gen word: *** error *** pattern not instantiated\n");
				CleanUpAll(TRUE);
				CloseFiles();
				cutt_exit(0);
			}
			pat_p = seg_end + 1;
		} else {  /* append pattern as is to string */
			seg_end = (strchr(pat_p,'\\'));
			if (seg_end) {  /* more later, just get substring */
				tmp_p = (strchr(word, EOS));
				while (pat_p < seg_end)
					*tmp_p++ = *pat_p++;
				*tmp_p = EOS;
			} else { /* append rest of pattern to word */
				strcat(word,pat_p);
				pat_p = EOS;
			}
		}
	}
	return(SUCCEED);
}

/* expand_pat - takes pattern containing variables */
/* builds expanded pattern by substituting var_pat */
/* for var_names */
BOOL
expand_pat(STRING *in_pat, STRING *out_pat) {
	extern VAR_REC *var_tab;
	char *var_beg, *var_end;
	char var_name[MAX_VARNAME];
	char delim_beg[3];
	int i, outlen;

	*out_pat = EOS;
	outlen = 0;

	while (*in_pat != EOS){
		/* '$' marks variable, get var name */
		if (*in_pat == '$' || *in_pat == '<') {
			char vm = *in_pat;
			if (*++in_pat == '('){ /* long var_name */
				var_beg = in_pat;
				if ((var_end = strchr(var_beg,')')) != NULL){
					strncpy(var_name,var_beg,var_end-var_beg+1);
					var_name[var_end-var_beg+1] = EOS;
				} else { ERROR_MOR("expand_var: bad variable name", FAIL, in_pat); }
				in_pat = ++var_end;
			}
			else{
				*var_name = *in_pat++;
				var_name[1] = EOS;
			}
			/* got name, now match in in table */
			for (i = 0; i < MAX_NUMVAR; i++) {
				if (strcmp(var_name,var_tab[i].var_name) == 0)
					break;
			}
			if (i < MAX_NUMVAR){
				/* make sure we've got enough space */
				if ((outlen + strlen(var_tab[i].var_pat)) < MAX_PATLEN) {
					outlen = outlen + strlen(var_tab[i].var_pat);
					if (vm == '<') delim_beg[0] = '<';
					else delim_beg[0] = '\\';
					delim_beg[1] = (char)(i+'0');
					delim_beg[2] = EOS;
					strcat(out_pat,delim_beg);
					strcat(out_pat,var_tab[i].var_pat);
					if (*in_pat != '<') strcat(out_pat,"\\");
				}
				else {ERROR_MOR("expand_pat: pattern expansion too big",FAIL,"");}
			}
			else {ERROR_MOR("expand_pat: variable not defined",FAIL,var_name);}
		}
		else{  /* copy it on over */
			while ((*in_pat != '$') && (*in_pat != '<') && (*in_pat != EOS)){
				outlen = strlen(out_pat);
				out_pat[outlen] = *in_pat;
				out_pat[outlen+1] = EOS;
				in_pat++;
			}
		}
	} /* end_while */
	return(SUCCEED);
}

/* store_pat -  takes variable name */
/* looks it up in var_table */
/* enters string (assume string doesn't contain any variables itself) */
BOOL
store_pat(STRING *var_name, STRING *var_pat) {
	extern VAR_REC *var_tab;
	int i;

	for (i = 0; i < MAX_NUMVAR; i++){
		if (strcmp(var_tab[i].var_name,"") == 0)
			break;
		if (strcmp(var_name,var_tab[i].var_name) == 0)
			break;
	}
	if (i < MAX_NUMVAR){
		strcpy(var_tab[i].var_name,var_name);
		strcpy(var_tab[i].var_pat,var_pat);
		return(SUCCEED);
	}
	else
	{ERROR_MOR("store_pat: variable table overflow",FAIL,"");}
}

/* ************************************************************************** */
/* fs.c - library functions which handle feature-value pairs                  */
/* ************************************************************************** */


/* m_val_node: malloc new value node & initialize structure
   set name field to name, all other fields to NULL
   returns NULL if can't allocate enough space for node */
static VAL_PTR m_val_node(STRING *name, int number) {
	VAL_PTR new_node;
	extern int val_node_ctr;

	new_node = NULL;
	new_node = (VAL_PTR) malloc((size_t)sizeof(VAL_NODE));
	mem_ctr++;
	if (new_node != NULL) {
		new_node->code = number;
		new_node->name = (char*)malloc((size_t)(sizeof(char)*(strlen(name)+1)));
		mem_ctr++;
		if (new_node->name != NULL)
			strcpy(new_node->name,name);
		else
			return(NULL);
		new_node->next = NULL;
		val_node_ctr++;
		return(new_node);
	} else
		return(NULL);
}
/* 2009-10-13
static char isWildCharFound(char *w) {
	for (; *w; w++) {
		if (*w == '*')
			return(TRUE);
	}
	return(FALSE);
}
*/
static int get_val_code(STRING *val_name, int feat_index) {
	extern FEAT_PTR *feat_codes;
	FEAT_PTR feat_entry;
	VAL_PTR val_tmp;
	int val_code = 1;  /* set at 1 since we don't seach 0 length list */

	feat_entry = feat_codes[feat_index-1];
	if (feat_entry->values == NULL) { /* first value found for this feature */
		feat_entry->values = m_val_node(val_name,1);
		if (feat_entry->values == NULL)
			mor_mem_error(FAIL,"out of memory space, quitting");
		else
			return(1);  /* index of item in list */
	} else { /* search through list for match on val_name */
		val_tmp = feat_entry->values;
		while (val_tmp->next != NULL) {
			++val_code;
			if (strcmp(val_name,val_tmp->name) == 0) 
				return(val_tmp->code);  /* found it in list, return code */
			val_tmp = val_tmp->next;
		}
		/* make sure the last one in the list isn't it */
		if (strcmp(val_name,val_tmp->name) == 0) 
			return(val_tmp->code);  /* found it in list, return code */

		++val_code;  /* count last in list as well */
		if (val_code >= MAX_VALS) {
			sprintf(templineC, "\n\nERROR: Too many feature values; limit is %d\n", MAX_VALS);
			mor_mem_error(FAIL,templineC);
		}
		if (val_tmp->next == NULL) { 
			/* not in list, can still take more values, add it */
			if ((val_tmp->next = m_val_node(val_name,val_code)) == NULL)
				mor_mem_error(FAIL,"out of memory space, quitting");
			else
				return(val_code);
		}
	}
	return(0);
}
/* 2009-10-13
static char featurecmp(char *s, char *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS) 
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				else if (pat[n] == '_' && s[m] != EOS) ;
				else if (s[m] != pat[n]) {
				   	j++;
				   	goto f1;
				}
			}
		}
		if (s[j] != pat[k]) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (pat[k] == s[j])
		return(1);
	else if (s[j] == EOS && pat[k] == ':' && pat[k+1] == '*')
		return(1);
	else
		return(0);
}
*/
// *:* German
/* fs_comp: translate fs into compressed string */
/* fails if fs contains mismatched or misplaced {}'s or []'s (fs syntax error) */
/* fails if fs_tables get filled (feature table overflow) */
/* fails if strange character found in string (illegal string) */
BOOL fs_comp(STRING *big_fs, FEATTYPE *c_fs  /* compressed fs */) {
	STRING *p;
	int brace_ctr = 0;  /* keep track of {}'s */
	int bracket_ctr = 0;  /* keep track of []'s */
	int type = ATOMIC;              /* ATOMIC OR or LIST */
	int state = PARSE_FEAT;         /* state in parse */
	FEATTYPE tmp_str[MAXCAT];
	char token[MAX_WORD];
	int feat_stack[20];
	int tmp_cnt = 0;
	FEATTYPE *tmp_p;
	int feat_code = 0, val_code;
	int i;
	int num_vals = 0;
//	extern FEAT_PTR *feat_codes;

	c_fs[0] = EOS;
	if (strlen(big_fs) == 0)
		return (SUCCEED);  /* don't bother to do anything with empty string */
	p = big_fs;
	tmp_p = tmp_str;
parse_loop:
	while (*p != EOS) {
		switch (*p) {
			case '{' :
				state = PARSE_FEAT;
				brace_ctr++;
				p++; 
				break;
			case '[' :
				if (state == PARSE_VAL) {
					ERROR_MOR("bad feature structure spec: state == PARSE_VAL",FAIL,big_fs);
				}
				bracket_ctr++;
				p++;
				break;
			case '}' :
				state = PARSE_FEAT;  /* necessary? */
				brace_ctr--;
				p++; 
				break;
			case ']' :
				if ((state == PARSE_VAL) && (num_vals == 0)) {
					ERROR_MOR("bad feature structure spec: state == PARSE_VAL",FAIL,big_fs);
				}
				state = PARSE_FEAT;  /* necessary? */
				num_vals = 0;
				bracket_ctr--;
				p++; 
				break;
			case '(':
				if (state != PARSE_VAL) {
					ERROR_MOR("bad feature structure spec: state != PARSE_VAL",FAIL,big_fs);
				}
				type = LIST;
				p++;
				break;
			case ')':
				if (state != PARSE_VAL) {
					ERROR_MOR("bad feature structure spec: state != PARSE_VAL",FAIL,big_fs);
				}
				state = PARSE_FEAT;  /* necessary? */
				p++;
				break;
			/* skip whitespace & tabs */
			case ' ':
				p++;
				break;
			case '\t':
				p++;
				break;
			default :   /* either "OR", string value, or feat/val names */
				if (strncmp("OR ",p,3) == 0) {	/* keyword "OR" */
					/* set proper flags, go to top of parse loop */
					if (state != PARSE_VAL) {
						ERROR_MOR("bad feature structure spec: state != PARSE_VAL",FAIL,big_fs);
					}
					type = OR;
					p = p + 3;
					goto parse_loop;
				} else if (*p == '\"') {  /* string value */
					type = STR;
					p++;
					p = get_word(p,token);/* get whole string now */
					if (*p != '\"') {
						ERROR_MOR("bad feature structure spec: *p != '\"'",FAIL,big_fs);
					}
					p++;
				} else if (legal_char(*p) == FALSE) /* unexpected stuff */
					return(FAIL);
				else { /* feat/val names */
					p = get_word(p,token);/* get entire token */
				}
				/* process token */
				if (state == PARSE_FEAT) { /* push feature onto stack */
					if ((feat_code=get_feat_code(token)) != 0xffff) {
						if (bracket_ctr > 20)
							ERROR_MOR("Too many stacks: default: state == PARSE_FEAT",FAIL,big_fs);
						if (bracket_ctr <= 0)
							ERROR_MOR("bad feature structure spec: default: state == PARSE_FEAT",FAIL,big_fs);
						feat_stack[bracket_ctr-1] = feat_code;
					} else {
						ERROR_MOR("feature table overflow",FAIL,"");
					}
					state = PARSE_VAL;
					type = ATOMIC;  /* anticipate simple feat-val pair */
				} else if (state == PARSE_VAL) {
					num_vals++;
					if ((num_vals > 1) && /* write out feature-value pair(s) */
									((type == STR) || (type == ATOMIC))) {
						ERROR_MOR("bad feature structure spec",FAIL,big_fs);
					}
					if (num_vals == 1) {/* write out complete path name */
						for (i=0; i < bracket_ctr; i++) {
							*(tmp_p++) = 'f';
							*(tmp_p++) = (FEATTYPE) feat_stack[i];
							tmp_cnt += 2;
						}
					}
					if (type == STR) {
						*(tmp_p++) = (FEATTYPE)'"';
						for (i=0; token[i] != EOS; i++) {
							*(tmp_p++) = (FEATTYPE)token[i];
						}
						*(tmp_p++) = '"';
						tmp_cnt += 2 + i;
					} else {
/* 2009-10-13
						if (isWildCharFound(token)) {
							VAL_PTR val_tmp;
							char wMatchFound = 0, *s;

							if (feat_codes[feat_code-1]->values != NULL) {
								val_tmp = feat_codes[feat_code-1]->values;
								while (val_tmp != NULL) {
									if (featurecmp(val_tmp->name, token)) {
										switch (type) {
											case ATOMIC: 
												*(tmp_p++) = 'v';
												break;
											case OR: 
												*(tmp_p++) = 'o';
												break;
											case LIST: 
												*(tmp_p++) = 'l';
												break;
											default:
//10-5-00										ERROR_MOR("bad feature structure spec",FAIL,big_fs);
												break;
										}
										*(tmp_p++) = (char) val_tmp->code;
										tmp_cnt += 2;
										wMatchFound++;
									}
									if (wMatchFound > 1 && type == ATOMIC) {
										type = OR;
										for (s=tmp_str; s < tmp_p; s++) {
											if (*s == 'v')
												*s = 'o';
										}
									}
									if (tmp_cnt >= MAXCAT-1) {
										sprintf(templineC3, "Internal error: Max length of value strings exceeded %d", MAXCAT-1);
										ERROR_MOR(templineC3,FAIL,big_fs);
									}
									val_tmp = val_tmp->next;
								}
								if (wMatchFound == 0) {
									sprintf(templineC3, "no feature structure matched: %s", token);
									ERROR_MOR(templineC3,FAIL,big_fs);
								}
							} else {
								ERROR_MOR("no feature structures found",FAIL,big_fs);
							}
						} else
*/
						if ((val_code = get_val_code(token,feat_code)) < 1) {
							ERROR_MOR("bad feature structure spec: val_code < 1",FAIL,big_fs);
						} else {
							switch (type) {
								case ATOMIC: 
									*(tmp_p++) = 'v';
									break;
								case OR: 
									*(tmp_p++) = 'o';
									break;
								case LIST: 
									*(tmp_p++) = 'l';
									break;
								default:
									ERROR_MOR("bad feature structure spec: type == default",FAIL,big_fs);
									break;
							}
							*(tmp_p++) = val_code;
							tmp_cnt += 2;
						}
					}
					if ((type == ATOMIC) || (type == STR))
						state = PARSE_FEAT;  /* necessary? */
				}
				break;
		}/* end switch */
	} /* end_while */
	if (bracket_ctr != 0) {
		ERROR_MOR("bad feature structure spec: bracket_ctr != 0",FAIL,big_fs);
	} else {
		*(tmp_p++) = EOS;
		featcpy(c_fs,tmp_str);
		return (SUCCEED);
	}
}

/* m_feat_node: malloc new feature node & initialize structure
   set name field to name, all other fields to NULL
   returns NULL if can't allocate enough space for node */
FEAT_PTR
m_feat_node(STRING *name) {
  FEAT_PTR new_node;
  extern FEAT_PTR *feat_codes;

  if (feat_node_ctr < MAX_FEATS) {
      new_node = NULL;
      new_node = (FEAT_PTR) malloc((size_t) sizeof(FEAT_NODE));
      mem_ctr++;
      if (new_node != NULL) {
		  new_node->bf = 0;
		  new_node->code = feat_node_ctr + 1;
		  new_node->name = 
		    (char*)malloc((size_t)(sizeof(char)*(strlen(name)+1)));
		  if (new_node->name != NULL)
		      strcpy(new_node->name,name);
		  else {
			  sprintf(templineC, "\n\nERROROut of memory\n");
			  mor_mem_error(FAIL,templineC);
		  }
		  mem_ctr++;
		  new_node->left = NULL;
		  new_node->right = NULL;
		  new_node->values = NULL;
		  feat_node_ctr++;  /* increment number of features */
		  feat_codes[feat_node_ctr-1] = new_node;  /* set up pointer in code array */
		  return(new_node);
		} else {
		  sprintf(templineC, "\n\nERROROut of memory\n");
		  mor_mem_error(FAIL,templineC);
		}
    } else {
		sprintf(templineC, "\n\nERROR: Too many feature values; limit is %d\n", MAX_FEATS);
		mor_mem_error(FAIL,templineC);
    }
  return(NULL);
}

FEATTYPE
get_feat_code(STRING *name) {
  extern FEAT_PTR features;
  FEAT_PTR feat_entry;

  if (features != NULL)
    feat_entry = access_feat(&features,name);
  else
    {  /* first feature encountered becomes root of tree */
      features = m_feat_node(name);
      feat_entry = features;
    }

  if (feat_entry == NULL)
    return(0xffff);
  else
    return(feat_entry->code);
}

STRING *get_feat_name(int code) {
  extern FEAT_PTR *feat_codes;

  if (feat_codes[code-1] != NULL)
    return (feat_codes[code-1]->name);
  else
    return(NULL);
}

/* get_val_name: get the name of a value, based on the code */
/* where code = position in list */
/* find the nth element in the linked list */
/* return the name stored in that entry */
/* or NULL if the element doesn't exist */ 
static STRING *get_val_name(VAL_PTR root_node_ptr, int code) {
	VAL_PTR val_tmp;
	int index = 1;

	val_tmp = root_node_ptr;
	while ((val_tmp != NULL) && (index++ < code))
		val_tmp = val_tmp->next;

	if (val_tmp == NULL)
		return(NULL);
	else
		return(val_tmp->name);
} 

/* fs_decomp: translate compressed fs into something readable */
BOOL
fs_decomp(FEATTYPE *c_fs, STRING *big_fs) {
	FEATTYPE *c_ptr,  *s_beg;
	extern FEAT_PTR *feat_codes;
	FEAT_PTR feat_tmp = NULL;
	STRING *name_tmp;
	int STATE = 0;  /* keeps track of parse */
	int depth = 0;
	char tmp_str[MAX_WORD];
	int i;
	
	c_ptr = c_fs;
	big_fs[0] = EOS;
	tmp_str[0] = EOS;
	
	/* debug  fprintf(debug_fp,"fsdecomp: short =  %s\tlong = %s\n",c_fs,big_fs); */
	if (featlen(c_fs) == 0)
		return (SUCCEED);  /* don't bother to do anything with empty string */
	
	while (*c_ptr != EOS){
		switch(*c_ptr){
			case 'f':
				if ((STATE == 0) || (STATE == 1)){
					strcat(big_fs,"{[");
					depth++;  /* record nesting */
					STATE = 1; 
				}
				else if (STATE == 2){  /* close of atomic or string feature */
					while (depth > 1){  /* close nested features */
						strcat(big_fs,"}] ");
						depth--;
					}
					strcat(big_fs," [");
					STATE = 1; 
				}
				else if (STATE == 3){  /* close of an OR */
					strcat(big_fs,"] [");
					STATE = 1; 
				}
				else if (STATE == 4){  /* close of a list */
					strcat(big_fs,")] [");
					STATE = 1; 
				}
				else 
					return(FAIL); /* shouldn't happen */
				
				++c_ptr; /* now pointing at f_code */
				if ((feat_tmp = feat_codes[((int)*c_ptr)-1]) != NULL){
					strcat(big_fs,feat_tmp->name);
					strcat(big_fs," ");
				}
				else
					return(FAIL); /* bad code- this shouldn't happen */
				
				++c_ptr;
				break;
				
			case 'v':
				if (STATE != 1)
					return(FAIL); /* shouldn't happen */
				
				++c_ptr; /* now pointing at v_code */
				if ((name_tmp = get_val_name(feat_tmp->values,(int)*c_ptr)) != NULL)
					strcat(big_fs,name_tmp);
				else
					return(FAIL);  /* bad code? */
				
				strcat(big_fs,"]");
				STATE = 2;
				++c_ptr;
				break;
				
			case 'o':
				if (STATE == 1){  /* if first in OR series, print OR */
					strcat(big_fs,"OR");
					STATE = 3;
				}
				if (STATE != 3)
					return(FAIL); /* shouldn't happen */
				
				++c_ptr; /* now pointing at v_code */
				if ((name_tmp = get_val_name(feat_tmp->values,(int)*c_ptr)) != NULL){
					strcat(big_fs," ");
					strcat(big_fs,name_tmp);
				}
				else
					return(FAIL);  /* bad code? */
				++c_ptr;
				break;
				
			case 'l':
				if (STATE == 1){  /* if first in LIST, print open paren */
					strcat(big_fs,"(");
					STATE = 4;
				}
				if (STATE != 4)
					return(FAIL); /* shouldn't happen */
				else {
					++c_ptr; /* now pointing at v_code */
					if ((name_tmp = get_val_name(feat_tmp->values,(int)*c_ptr)) != NULL){
						strcat(big_fs," ");
						strcat(big_fs,name_tmp);
					}
					else
						return(FAIL);  /* bad code? */
					++c_ptr;
					break;
				}
				
			case '"':
				if (STATE != 1)
					return(FAIL); /* shouldn't happen */
				
				/* get string from c_fs */
				s_beg = c_ptr++;
				while (*++c_ptr != '"') ;
				++c_ptr;
				for (i=0; s_beg != c_ptr; s_beg++) {
					tmp_str[i++] = (char)*s_beg;
				}
				tmp_str[i] = EOS;
				strcat(big_fs,tmp_str);// lxs lxslxs big check
				strcat(big_fs,"]");
				STATE = 2;
				break;
				
			default: /* shouldn't get here */
				int i;
				fprintf(debug_fp,"\nfs_decomp: Illegal symbol found: ");
				for (i=0; c_ptr[i] != 0; i++)
					fprintf(debug_fp,"%c",(char)c_ptr[i]);
				fprintf(debug_fp," in string: ");
				for (i=0; c_fs[i] != 0; i++)
					fprintf(debug_fp,"%c",(char)c_fs[i]);
				fprintf(debug_fp,".\n");
				return(FAIL);
		}
	} /* end while */
	
	if (STATE == 3)
		strcat(big_fs,"]");
	else if (STATE == 4)
		strcat(big_fs,")]");
	
	strcat(big_fs,"}");  /* put on closing curly bracket */
	/* debug  fprintf(debug_fp,"fsdecomp: short =  %s\tlong = %s\n",c_fs,big_fs); */
	return(SUCCEED);
}

/* feat_path_decomp: translate compressed path into something readable */
BOOL
feat_path_decomp(FEATTYPE *c_path,  /* compressed pathname */ STRING *big_path) {
  FEATTYPE *c_ptr;
  extern FEAT_PTR *feat_codes;
  FEAT_PTR feat_tmp;

  c_ptr = c_path;
  big_path[0] = EOS;

  if (featlen(c_path) == 0)
    return (SUCCEED);  /* don't bother to do anything with empty string */

  strcat(big_path,"[");
  while (*c_ptr != 0){
    ++c_ptr; /* now pointing at f_code */
    if ((feat_tmp = feat_codes[((int)*c_ptr)-1]) != NULL){
      strcat(big_path,feat_tmp->name);
      strcat(big_path," ");
    }
    else
      return(FAIL); /* bad code- this shouldn't happen */
    
    if (*(++c_ptr) != 0)
      ++c_ptr;
  }
  strcat(big_path,"]");
  return(SUCCEED);
}


/* ********** operations on (compressed) feature structures ************* */

/* match_atom */
static BOOL
match_atom(FEATTYPE *cat, FEATTYPE *fvp) {
  FEATTYPE *c_ptr;
  FEATTYPE *fvp_ptr;
  FEATTYPE val_code = 0;
  int f_ctr = 0;

  /* match features */

  while (cat[f_ctr] == (FEATTYPE)'f')
    f_ctr += 2;
  
  if (featncmp(cat,fvp,f_ctr) != 0)
    return(FAIL);

  c_ptr = &cat[f_ctr];
  fvp_ptr = &fvp[f_ctr];

  /* is cat ATOMIC or OR ? */
  if (*c_ptr == (FEATTYPE)'v'){
    if (*++c_ptr == *++fvp_ptr)
      return(SUCCEED);
  } else  /* (*c_ptr = 'o') match value against series of ors */
  	val_code = *++fvp_ptr;
  ++c_ptr;
  while (*c_ptr != EOS){
    if (val_code == *c_ptr)
      return(SUCCEED);
    else {
      c_ptr++;
      if (*c_ptr == (FEATTYPE)'f')
		break;
      if (*c_ptr != EOS)
        c_ptr++;
    }
  }
  return(FAIL);  /* if we got this far, no match */
}

/* match_or */
static BOOL
match_or(FEATTYPE *cat, FEATTYPE *fvp) {
  FEATTYPE *c_ptr;
  FEATTYPE *fvp_ptr;
  FEATTYPE val_code;
  int f_ctr = 0;

  /* match features */

  while (cat[f_ctr] == 'f')
    f_ctr += 2;
  
  if (featncmp(cat,fvp,f_ctr) != 0)
    return(FAIL);

  /* match value against series of ors */

  c_ptr = &cat[f_ctr];
  fvp_ptr = &fvp[f_ctr];
  ++fvp_ptr; /* point to value */

  val_code = *++c_ptr;
  while (*fvp_ptr != EOS){
    if (val_code == *fvp_ptr)
      return(SUCCEED);
    else {
      if (*(++fvp_ptr) != EOS)
        ++fvp_ptr;
    }
  }
  return(FAIL);  /* if we got this far, no match */
}


/* match_list */
static BOOL
match_list(FEATTYPE *cat, FEATTYPE *fvp) {
  FEATTYPE *c_ptr;
  FEATTYPE *fvp_ptr;
  int f_ctr = 0;
  int cat_listlen = 0;
  int fvp_listlen = 0;

  /* match features */

  while (cat[f_ctr] == (FEATTYPE)'f')
    f_ctr += 2;
  
  if (featncmp(cat,fvp,f_ctr) != 0)
    return(FAIL);

  c_ptr = &cat[f_ctr];
  fvp_ptr = &fvp[f_ctr];

  while (*(fvp_ptr+fvp_listlen) == (FEATTYPE)'l')
    fvp_listlen += 2;

  while (*(c_ptr+cat_listlen) == (FEATTYPE)'l')
    cat_listlen += 2;

  if (cat_listlen != fvp_listlen)
    return(FAIL);

  if (featncmp(c_ptr,fvp_ptr,fvp_listlen) == 0)
    return (1);
  else
    return(FAIL);
}

/* check_fvp */
/* check if feature-value pair is present in category entry */
/* returns 1 if fvp matched, else 0 */
BOOL check_fvp(FEATTYPE *cat, FEATTYPE *fvp) {
	FEATTYPE *fvp_ptr;
	FEATTYPE *cat_tmp;
	FEATTYPE cur_fvp[MAXCAT];
	int cur_typ = 0;

	fvp_ptr = fvp;

	/* assume fvp is in disjunctive normal form */
	/* loop through all parts of the conjunct */

loop:
	fvp_ptr = get_fvp(cur_fvp,&cur_typ,fvp_ptr);

	/* match cur_fvp against cat */
	cat_tmp = cat;
	while (cat_tmp != NULL){
		switch(cur_typ){
			case ATOMIC:
				if (match_atom(cat_tmp,cur_fvp))
					goto found;
				else
					cat_tmp = featchr(++cat_tmp,'f');
				break;
			case OR:
				if (match_or(cat_tmp,cur_fvp))
					goto found;
				else
					cat_tmp = featchr(++cat_tmp,'f');
				break;
			case LIST:
				if (match_list(cat_tmp,cur_fvp))
					goto found;
				else
					cat_tmp = featchr(++cat_tmp,'f');
				break;
			default:  /* everything else is a straight match */
				if (featncmp(cat_tmp,cur_fvp,featlen(cur_fvp)) == 0)
					goto found;
				else
					cat_tmp = featchr(++cat_tmp,'f');
				break;
		}
	}
	if (cat_tmp == NULL)
		return(FAIL); 
found:
	if (*fvp_ptr != EOS)
		goto loop;

	return(SUCCEED);  /* if we're here, we've matched */
}


/* add_fvp */
/* add_fvp to category entry */
BOOL add_fvp(FEATTYPE *cat, FEATTYPE *fvp) {
	if ((featlen(cat) > 0) && (check_fvp(cat,fvp) == SUCCEED))
		return(SUCCEED);
	else{
		featcat(cat,fvp);
		return(SUCCEED);
	}
}

/* remove_fvp */
/* remove feature-value pair if it is present in category entry */
/* always succeeds ... if it's there, remove it, if it's not, no problem */
BOOL
remove_fvp(FEATTYPE *cat, FEATTYPE *fvp) {
  FEATTYPE *fvp_ptr;
  FEATTYPE*cat_tmp;
  FEATTYPE *post_fvp;
  FEATTYPE cur_fvp[MAXCAT];
  FEATTYPE misc_fvp[MAXCAT];
  int cur_type, tmp_type;

  fvp_ptr = fvp;

  /* assume fvp is in disjunctive normal form */
  /* loop through all parts of the conjunct */

  while ((fvp_ptr != NULL) && (*fvp_ptr != EOS)){
    fvp_ptr = get_fvp(cur_fvp,&cur_type,fvp);

    /* remove cur_fvp from cat */
    cat_tmp = cat;
    while ((cat_tmp != NULL) && (*cat_tmp != EOS)){
      if (featncmp(cat_tmp,cur_fvp,featlen(cur_fvp)) == 0){
	/* copy over substring */
	post_fvp = cat_tmp + featlen(cur_fvp);
	featcpy(cat_tmp,post_fvp);
      }
      cat_tmp = get_fvp(misc_fvp,&tmp_type,cat_tmp);
    }
  }  
  return(SUCCEED);  /* if we're here, we did ok */
}

/* match_fvp */ // 2012-07 2012-07-21 MATCHCAT
// TO UNDO REMOVE ANYTHING WITH    isPathSpecified
BOOL
match_fvp(FEATTYPE *cat_1, FEATTYPE *cat_2, FEATTYPE *path) {
	int found, matched, tot;
	char isPathSpecified; // 2012-07 2012-07-21 MATCHCAT
	FEATTYPE fvp[MAXCAT];
	FEATTYPE *cat_ptr;

	if (path == NULL || path[0] == 0 || path[1] == 0) // 2012-07 2012-07-21 MATCHCAT
		isPathSpecified = FALSE; // 2012-07 2012-07-21 MATCHCAT
	else // 2012-07 2012-07-21 MATCHCAT
		isPathSpecified = TRUE; // 2012-07 2012-07-21 MATCHCAT
	found = 0;
	matched = 0;
	cat_ptr = cat_1;
	while ((cat_ptr = retrieve_fvp(fvp,path,cat_ptr)) != NULL) {
/*
strcpy(templineC, "fvp=");
fs_decomp(fvp,templineC+strlen(templineC));
strcat(templineC, "; cat_1=");
fs_decomp(cat_2,templineC+strlen(templineC));
*/
		found++;
		if (check_fvp(cat_2,fvp) == SUCCEED)
			matched++;
	}
	if (matched != found || (found == 0 && isPathSpecified)) // 2012-07 2012-07-21 MATCHCAT
		return(FAIL);
	tot = found;
	found = 0;  
	matched = 0;
	cat_ptr = cat_2;
	while ((cat_ptr = retrieve_fvp(fvp,path,cat_ptr)) != NULL) {
/*
strcpy(templineC, "fvp=");
fs_decomp(fvp,templineC+strlen(templineC));
strcat(templineC, "; cat_2=");
fs_decomp(cat_2,templineC+strlen(templineC));
*/
		found++;
		if (check_fvp(cat_1,fvp) == SUCCEED)
			matched++;
	}
	if (matched != found || found != tot || (found == 0 && isPathSpecified)) // 2012-07 2012-07-21 MATCHCAT
		return(FAIL);
	else
		return(SUCCEED);
}
/*
void getError_fvp(char *word_tmp, FEATTYPE *data, FEATTYPE *StartCatt) {
	FEATTYPE *fvp_ptr;
	FEATTYPE fvp_tmp[MAXCAT];
	int fvp_type;

	if (StartCatt == NULL) {
		fs_decomp(data,word_tmp);
	} else {
		*word_tmp = EOS;
		fvp_ptr = StartCatt;
		while ((fvp_ptr = get_fvp(fvp_tmp,&fvp_type,fvp_ptr)) != NULL){
			if (featncmp(fvp_tmp,data,featlen(data)) == 0){
				fs_decomp(fvp_tmp,word_tmp);
				break;
			}
		}
		if (*word_tmp == EOS)
			fs_decomp(data,word_tmp);
	}
	if (*word_tmp == '{')
		strcpy(word_tmp, word_tmp+1);
	fvp_type = strlen(word_tmp) - 1;
	if (word_tmp[fvp_type] == '}')
		word_tmp[fvp_type] = EOS;
}
*/
void Error_fvp(FEATTYPE *data, FEATTYPE *NextCatRule, STRING *item, 
			   STRING *s_surf, FEATTYPE *s_cat, FEATTYPE *n_cat, STRING *s_parse, STRING *n_stem,
			   CRULE_PTR cur_rule, int clause_ctr, CRULE_COND_PTR cur_cond, CRULE_OP_PTR cur_op, FEATTYPE *r_cat) {
	FEATTYPE *fvp_ptr;
	FEATTYPE fvp_tmp[MAXCAT];
	char fs_tmp[MAXCAT];
	char word_tmp[MAX_WORD];
	int fvp_type;
	extern char debugName[];

	if (NextCatRule == NULL) {
		fs_decomp(data,word_tmp);
	} else {
		*word_tmp = EOS;
		fvp_ptr = NextCatRule;
		while ((fvp_ptr = get_fvp(fvp_tmp,&fvp_type,fvp_ptr)) != NULL){
			if (featncmp(fvp_tmp,data,featlen(data)) == 0){
				fs_decomp(fvp_tmp,word_tmp);
				break;
			}
		}
		if (*word_tmp == EOS) fs_decomp(data,word_tmp);
	}
	if (*word_tmp == '{') strcpy(word_tmp, word_tmp+1);
	fvp_type = strlen(word_tmp) - 1;
	if (word_tmp[fvp_type] == '}') word_tmp[fvp_type] = EOS;
	fprintf(stderr, "Failed: Missing \"%s\" in the \"next cat:\" list below for item \"%s\" following word \"%s\".\n",word_tmp,item,s_surf);
	if (debug_fp != stdout) {
		fprintf(stderr, "For further explanation, please see this error message in file \"%s\"\n", debugName);
	} else {
		fprintf(stderr,"applying c rules\n");
		fprintf(stderr," start: %s\n",s_surf);
		if (featlen(s_cat) > 0){
			fs_decomp(s_cat,fs_tmp);
			fprintf(stderr," start cat: %s\n",fs_tmp);
		} else {
			int i;
			fprintf(stderr," start cat: ");
			for (i=0; s_cat[i] != 0; i++)
				fprintf(stderr,"%c",(char)s_cat[i]);
			fprintf(stderr,"\n");
		}
		fprintf(stderr," current parse: %s\n",s_parse);
		fprintf(stderr," next: %s\n", item);
		if (featlen(n_cat) > 0){
			fs_decomp(n_cat,fs_tmp);
			fprintf(stderr," next cat: %s\n",fs_tmp);
		} else {
			int i;
			fprintf(stderr," next cat: ");
			for (i=0; n_cat[i] != 0; i++)
				fprintf(stderr,"%c",(char)n_cat[i]);
			fprintf(stderr,"\n");
		}
		fprintf(stderr," next stem: %s\n",n_stem);
		fprintf(stderr,"trying rule %s ... \n",cur_rule->name);
		fprintf(stderr," trying clause/ if-then %d\n",clause_ctr);
		decode_op_code(cur_op->op_type,word_tmp);
		fprintf(stderr,"  operation = %s",word_tmp);
		if (cur_op->data != NULL) {
			fs_decomp(cur_op->data,word_tmp);
			if (word_tmp[0] == EOS)
				fprintf(stderr,"\n");
			else
				fprintf(stderr," %s\n",word_tmp);
		} else
			fprintf(stderr,"\n");
		fs_decomp(r_cat,word_tmp);
		fprintf(stderr,"   current result cat = %s\n\n",word_tmp);
	}
}

/* ****************** utilities ******************************************** */
/* free_values: free up linked lists of values names & codes */
void
free_values(VAL_PTR entry) {
	if (entry->next != NULL) free_values(entry->next);
	if (entry->name != NULL) free(entry->name);
	mem_ctr--;
	free(entry);
	mem_ctr--;
}		  

/* free_feat - free up tables for features & their associated values */
void free_feats(FEAT_PTR root) {
//  extern FEAT_PTR *feat_codes;
	
	if (root != NULL) {
		free_feats(root->right);
		free_feats(root->left);
		if (root->values != NULL) free_values(root->values);
		if (root->name != NULL) free(root->name);
		mem_ctr--;
/* lxs 24-04-97
		for (i=0; i<feat_node_ctr; i++) {
			if (root == feat_codes[i])
				feat_codes[i] = NULL;
		}
*/
		free(root);
		mem_ctr--;
    }
}

/* Functions that handle new FEATTYPE */

FEATTYPE *featrchr(FEATTYPE *ft, char c) {
	FEATTYPE item, *p;

	for (p=ft; *p != 0; p++) ;
	if (p == ft)
		return(NULL);
	item = (FEATTYPE)c;
	for (--p; p != ft && *p != item; p--) ;
	if (*p == item)
		return(p);
	else
		return(NULL);
}

FEATTYPE *featchr(FEATTYPE *ft, char c) {
	FEATTYPE item;

	item = (FEATTYPE)c;
	for (; *ft != 0 && *ft != item; ft++) ;
	if (*ft == 0)
		return(NULL);
	else
		return(ft);
}

void featncpy(FEATTYPE *c_fs, FEATTYPE *tmp_str, long num) {
	for (; num > 0; num--)
		*(c_fs++) = *(tmp_str++);
	*c_fs = 0;
}

void featcpy(FEATTYPE *c_fs, FEATTYPE *tmp_str) {
	while ((*(c_fs++)=*(tmp_str++)) != 0) ;
}

void featcat(FEATTYPE *c_fs,FEATTYPE *tmp_str) {
	for (; *c_fs != 0; c_fs++) ;
	while ((*(c_fs++)=*(tmp_str++)) != 0) ;
}

int featncmp(FEATTYPE *f1, FEATTYPE *f2, long num) {
	for (; *f1 == *f2 && num > 0; f1++, f2++, num--) ;
	if (num == 0)
		return(0);
	else if (*f1 < *f2)
		return(-1);
	else
		return(1);
}

int featcmp(FEATTYPE *f1, FEATTYPE *f2) {
	for (; *f1 == *f2 && *f1 != 0 && *f2 != 0; f1++, f2++) ;
	if (*f1 == *f2)
		return(0);
	else if (*f1 < *f2)
		return(-1);
	else
		return(1);
}

int partfeatcmp(FEATTYPE *full,FEATTYPE *part) {
	for (; *full == *part && *part != 0; full++, part++) ;
	return(!*part);
}

long featlen(FEATTYPE *features) {
	long i;

	if (features == NULL)
		return(0);
	for (i=0; features[i] != 0; i++) ;
	return(i);
}
