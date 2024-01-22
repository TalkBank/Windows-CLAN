#include "ced.h"
#include "ids.h"
#include "check.h"
#ifdef _WIN32
	#include "w95_commands.h"
#endif


static void clean_roles(ROLESTYPE *p) {
	ROLESTYPE *t;

	while (p != NULL) {
		t = p;
		p = p->nextrole;
		if (t->role)
			free(t->role);
		free(t);
	}
}

static void clean_SESs(SESTYPE *p) {
	SESTYPE *t;

	while (p != NULL) {
		t = p;
		p = p->nextses;
		if (t->st)
			free(t->st);
		free(t);
	}
}

static void clean_ids(IDSTYPE *p) {
	IDSTYPE *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_id;
		free(t);
	}
}

static ROLESTYPE *addNewRole(ROLESTYPE *rolep, unCH *role) {
	int i;
	ROLESTYPE *p, *t;

	if ((p=NEW(ROLESTYPE)) == NULL) {
		strcpy(global_df->err_message, "+Can't continue: Out of memory.");
		clean_roles(rolep);
		return(NULL);
	}
	for (i=0; role[i] != EOS; ) {
		if (role[i] == '\\')
			strcpy(role+i, role+i+1);
		else
			i++;
	}
	if ((p->role=(unCH *)malloc((strlen(role)+1)*sizeof(unCH))) == NULL) {
		strcpy(global_df->err_message, "+Can't continue: Out of memory.");
		free(p);
		clean_roles(rolep);
		return(NULL);
	}
	strcpy(p->role, role);
	p->nextrole = NULL;
	if (rolep == NULL) {
		rolep = p;
	} else {
		for (t=rolep; t->nextrole != NULL; t=t->nextrole) ;
		t->nextrole = p;
	}
	return(rolep);
}

static Boolean getRoles(DEPFDEFS *RolesSes) {
	int  b, e;
	char pf;
	FILE *fp;
	FNType mDirPathName[FNSize];

	fp = NULL;
	if (!isRefEQZero(global_df->fileName)) {
		extractPath(mDirPathName, global_df->fileName);
		addFilename2Path(mDirPathName, DEPFILE);
		fp = fopen(mDirPathName,"r");
	}
/*
	 if (fp == NULL) {
		 strcpy(mDirPathName, wd_dir);
		 addFilename2Path(mDirPathName, DEPFILE);
		 fp = fopen(mDirPathName,"r");
	 }
*/	
	if (fp == NULL) {
		strcpy(mDirPathName, lib_dir);
		addFilename2Path(mDirPathName, DEPFILE);
		fp = fopen(mDirPathName,"r");
	}
	if (fp == NULL) {
		strcpy(mDirPathName, lib_dir);
		if (!LocateDir("Please locate library directory with depfile",mDirPathName,false)) {
			strcpy(global_df->err_message, "+Error: Can't do check without depfile.");
			return(false);
		} else {
			if (pathcmp(lib_dir, mDirPathName) != 0) {
				strcpy(lib_dir, mDirPathName);
				WriteCedPreference();
#ifdef _MAC_CODE
				UpdateWindowNamed(Commands_str);
#endif // _MAC_CODE
#ifdef _WIN32
				if (clanDlg != NULL) {
					u_strcpy(clanDlg->t_st, lib_dir, FNSize);
					AdjustName(clanDlg->lib_st, clanDlg->t_st, 39);
					clanDlg->m_LibSt = clanDlg->lib_st;
					clanDlg->UpdateData(FALSE);
				}
#endif // _WIN32
			}
			addFilename2Path(mDirPathName, DEPFILE);
			fp = fopen(mDirPathName,"r");
			if (fp == NULL) {
				strcpy(global_df->err_message, "+Error: Can't open depfile \"");
				uS.FNType2str(global_df->err_message, strlen(global_df->err_message), mDirPathName);
				strcat(global_df->err_message, "\".");
				return(false);
			}
		}
	}
	pf = FALSE;
	last_cr_char = 0;
	while (fgets_ced(templineC, UTTLINELEN, fp, NULL) != NULL) {
		if (templineC[0] == '#')
			continue;
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		if (!strcmp(templineC,"\n"))
			continue;
		if (uS.partcmp(templineC,PARTICIPANTS,FALSE,FALSE)) {
			pf = TRUE;
			templineC[0] = ' ';
			for (b=0; templineC[b] != ':' && templineC[b] != EOS; b++) ;
			if (templineC[b] == ':')
				b++;
		} else
			b = 0;
		if (pf && isSpeaker(templineC[0]))
			break;
		if (pf && isSpace(templineC[0])) {
			u_strcpy(templine, templineC+b, UTTLINELEN);
			for (b=0; isSpace(templine[b]) || templine[b] == '\n' || templine[b] == '\r'; b++) ;
			while (templine[b]) {
				for (e=b; !isSpace(templine[e]) && templine[e] != '\n' && templine[e] != '\r' && templine[e] != EOS; e++) ;
				if (templine[e] == EOS) {
					RolesSes->rootRoles = addNewRole(RolesSes->rootRoles, templine+b);
					if (RolesSes->rootRoles == NULL)
						return(false);
					b = e;
				} else {
					templine[e] = EOS;
					RolesSes->rootRoles = addNewRole(RolesSes->rootRoles, templine+b);
					if (RolesSes->rootRoles == NULL)
						return(false);
					b = e + 1;
				}
				for (; isSpace(templine[b]) || templine[b] == '\n' || templine[b] == '\r'; b++) ;
			}	
		}
	}
	fclose(fp);
	return(true);
}

static SESTYPE *addNewSES(SESTYPE *SES, unCH *st) {
	unCH *s;
	SESTYPE *p, *t;

	if (*st == '<') {
		st++;
		s = strrchr(st, '>');
		if (s != NULL)
			*s = EOS;
	} else
		s = NULL;
	if ((p=NEW(SESTYPE)) == NULL) {
		strcpy(global_df->err_message, "+Can't continue: Out of memory.");
		clean_SESs(SES);
		if (s != NULL)
			*s = '>';
		return(NULL);
	}
	if ((p->st=(unCH *)malloc((strlen(st)+1)*sizeof(unCH))) == NULL) {
		strcpy(global_df->err_message, "+Can't continue: Out of memory.");
		free(p);
		clean_SESs(SES);
		if (s != NULL)
			*s = '>';
		return(NULL);
	}
	strcpy(p->st, st);
	p->nextses = NULL;
	if (SES == NULL) {
		SES = p;
	} else {
		for (t=SES; t->nextses != NULL; t=t->nextses) ;
		t->nextses = p;
	}
	if (s != NULL)
		*s = '>';
	return(SES);
}

static Boolean getSES(DEPFDEFS *RolesSes) {
	int  b, e;
	char idf;
	unCH t;
	FILE *fp;
	FNType mDirPathName[FNSize];

	fp = NULL;
	strcpy(mDirPathName, lib_dir);
	addFilename2Path(mDirPathName, DEPFILE);
	fp = fopen(mDirPathName, "r");
	if (fp == NULL) {
		strcpy(global_df->err_message, "+Error: Can't open depfile \"");
		uS.FNType2str(global_df->err_message, strlen(global_df->err_message), mDirPathName);
		strcat(global_df->err_message, "\".");
		return(false);
	}
	idf = FALSE;
	last_cr_char = 0;
	while (fgets_ced(templineC, UTTLINELEN, fp, NULL) != NULL) {
		if (templineC[0] == '#')
			continue;
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		if (!strcmp(templineC, "\n"))
			continue;
		if (uS.partcmp(templineC, IDOF, FALSE, FALSE)) {
			idf = TRUE;
			templineC[0] = ' ';
			for (b = 0; templineC[b] != ':' && templineC[b] != EOS; b++);
			if (templineC[b] == ':')
				b++;
		} else
			b = 0;
		if (idf && isSpeaker(templineC[0]))
			break;
		if (idf && isSpace(templineC[0])) {
			u_strcpy(templine, templineC + b, UTTLINELEN);
			for (b=0; isSpace(templine[b]) || templine[b] == '\n' || templine[b] == '\r'; b++);
			while (templine[b]) {
				for (e=b; !isSpace(templine[e]) && templine[e] != '\n' && templine[e] != '\r' && templine[e] != EOS; e++);
				t = templine[e];
				templine[e] = EOS;
				if (templine[b] == '@' && templine[b+1] == 'e') {
					RolesSes->rootSESe = addNewSES(RolesSes->rootSESe, templine+b+2);
					if (RolesSes->rootSESe == NULL)
						return(false);
				} else if (templine[b] == '@' && templine[b+1] == 's') {
					RolesSes->rootSESs = addNewSES(RolesSes->rootSESs, templine+b+2);
					if (RolesSes->rootSESs == NULL)
						return(false);
				}
				templine[e] = t;
				if (t == EOS)
					break;
				else
					b = e + 1;
				for (; isSpace(templine[b]) || templine[b] == '\n' || templine[b] == '\r'; b++);
			}
		}
	}
	fclose(fp);
	return(true);
}

IDSTYPE *deleteID(IDSTYPE *rootIDs, UInt16 item) {
	IDSTYPE *p, *pp;
	
	p = rootIDs;
	pp = rootIDs;
	while (p != NULL) {
		item--;
		if (item <= 0)
			break;
		pp = p;
		p = p->next_id;
	}
	if (p != NULL) {
		if (p == rootIDs) {
			rootIDs = rootIDs->next_id;
			free(p);
		} else {
			pp->next_id = p->next_id;
			free(p);
		}
	}
	return(rootIDs);
}

IDSTYPE *createNewId(IDSTYPE *rootIDs, int item) {
	int  i;
	unCH code[CODESIZE];
	IDSTYPE *p, *oldId;

	if (item > 0) {
		for (oldId=rootIDs; oldId != NULL; oldId=oldId->next_id) {
			item--;
			if (item <= 0)
				break;
		}
	} else
		oldId = NULL;

	for (i=1; i < 1000; i++) {
		if (i < 10)
			uS.sprintf(code,  cl_T("SP0%d"), i);
		else
			uS.sprintf(code,  cl_T("SP%d"), i);
		for (p=rootIDs; p != NULL; p=p->next_id) {
			if (uS.mStricmp(p->code, code) == 0)
				break;
		}
		if (p == NULL)
			break;
	}

	if ((p=NEW(IDSTYPE)) == NULL)
		return(rootIDs);
	if (oldId == NULL) {
		p->language[0] = EOS;
		p->corpus[0] = EOS;
		strcpy(p->code, code);
		p->age1y = -1;
		p->age1m = -1;
		p->age1d = -1;
		p->sex = 0;
		p->group[0] = EOS;
		p->SES[0] = EOS;
		p->role[0] = EOS;
		p->education[0] = EOS;
		p->custom_field[0] = EOS;
		p->spname[0] = EOS;
	} else {
		strcpy(p->language, oldId->language);
		strcpy(p->corpus, oldId->corpus);
		strcpy(p->code, code);
		p->age1y = oldId->age1y;
		p->age1m = oldId->age1m;
		p->age1d = oldId->age1d;
		p->sex = oldId->sex;
		strcpy(p->group, oldId->group);
		strcpy(p->SES, oldId->SES);
		strcpy(p->role, oldId->role);
		strcpy(p->education, oldId->education);
		strcpy(p->custom_field, oldId->custom_field);
		strcpy(p->spname, oldId->spname);
	}
	p->next_id = rootIDs;
	rootIDs = p;
	return(rootIDs);
}

static void cleanup_lang(unCH *lang) {
	while (*lang) {
		if (isSpace(*lang) || *lang < (unCH)' ')
			strcpy(lang, lang+1);
		else if (!uS.mStrnicmp(lang, "legacy,", 7))
			strcpy(lang, lang+7);
		else if (!uS.mStrnicmp(lang, "legacy", 6))
			strcpy(lang, lang+6);
		else if (!uS.mStrnicmp(lang, "ca,", 3))
			strcpy(lang, lang+3);
		else if (!uS.mStrnicmp(lang, "ca", 2))
			strcpy(lang, lang+2);
		else
			lang++;
	}
	if (*(lang-1) == ',')
		strcpy(lang-1, lang);
}

static void add_to_languages(unCH *languages, unCH *line, char isAddSpaces) {
	unCH *e, lang[IDFIELSSIZE];
	unCH *bl;

	while (*line != EOS) {
		e = strchr(line, ',');
		if (e != NULL) {
			*e = EOS;
			strcpy(lang, line);
			*e = ',';
			line = e + 1;
			for (; isSpace(*line); line++) ;
		} else {
			strcpy(lang, line);
			line = line + strlen(line);
		}
		for (bl=languages; *bl != EOS;) {
			e = strchr(bl, ',');
			if (e != NULL) {
				*e = EOS;
				if (uS.mStricmp(lang, bl) == 0) {
					*e = ',';
					break;
				}
				*e = ',';
				bl = e + 1;
				for (; isSpace(*bl); bl++) ;
			} else {
				if (uS.mStricmp(lang, bl) == 0)
					break;
				bl = bl + strlen(bl);
			}
		}
		if (languages[0] == EOS) {
			strcat(languages, lang);
		} else if (*bl == EOS) {
			if (isAddSpaces)
				strcat(languages, ", ");
			else
				strcat(languages, ",");
			strcat(languages, lang);
		}
	}
}

static void parseAge(IDSTYPE *p, unCH *age) {
	unCH *e;

	if (p == NULL)
		return;
	while (*age != EOS) {
		for (; isSpace(*age); age++) ;
		for (e=age; *e != ';' && *e != '.' && *e != '-' && *e != EOS; e++) ;
		if (*e == ';') {
			*e = EOS;
			p->age1y = uS.atoi(age);
			*e = ';';
			age = e + 1;
		} else if (*e == '.') {
			*e = EOS;
			p->age1m = uS.atoi(age);
			*e = '.';
			age = e + 1;
		} else if (*e == EOS) {
			p->age1d = uS.atoi(age);
			break;
		} 
	}
}

static IDSTYPE *add_to_IDs(IDSTYPE *rootIDs, DEPFDEFS *RolesSes, unCH *lang, unCH *corp, unCH *code, unCH *age, unCH *sex, unCH *group, unCH *SES, unCH *role, unCH *educ, unCH*fu, unCH *spn) {
	IDSTYPE *p;

	if (code == NULL)
		return(rootIDs);
	uS.remblanks(code);
	uS.uppercasestr(code, NULL, 0);
	if (rootIDs == NULL) {
		if ((rootIDs=NEW(IDSTYPE)) == NULL)
			return(NULL);
		p = rootIDs;
		p->language[0] = EOS;
		p->corpus[0] = EOS;
		p->code[0] = EOS;
		p->age1y = -1;
		p->age1m = -1;
		p->age1d = -1;
		p->sex = 0;
		p->group[0] = EOS;
		p->SES[0] = EOS;
		p->role[0] = EOS;
		p->education[0] = EOS;
		p->custom_field[0] = EOS;
		p->spname[0] = EOS;
		p->next_id = NULL;
	} else {
		for (p=rootIDs; p->next_id != NULL; p=p->next_id) {
			if (uS.mStricmp(p->code, code) == 0)
				break;
		}
		if (uS.mStricmp(p->code, code) != 0) {
			p->next_id = NEW(IDSTYPE);
			p = p->next_id;
			if (p == NULL) {
				clean_ids(rootIDs);
				return(NULL);
			}
			p->language[0] = EOS;
			p->corpus[0] = EOS;
			p->code[0] = EOS;
			p->age1y = -1;
			p->age1m = -1;
			p->age1d = -1;
			p->sex = 0;
			p->group[0] = EOS;
			p->SES[0] = EOS;
			p->role[0] = EOS;
			p->education[0] = EOS;
			p->custom_field[0] = EOS;
			p->spname[0] = EOS;
			p->next_id = NULL;
		}
	}
	if (p->code[0] == EOS) {
		strncpy(p->code, code, CODESIZE);
		p->code[CODESIZE] = EOS;
	}
	if (lang != NULL) {
		uS.remblanks(lang);
		strncpy(p->language, lang, IDFIELSSIZE);
		p->language[IDFIELSSIZE] = EOS;
	}
	if (corp != NULL) {
		uS.remblanks(corp);
		strncpy(p->corpus, corp, IDFIELSSIZE);
		p->corpus[IDFIELSSIZE] = EOS;
	}
	if (age != NULL) {
		parseAge(p, age);
	}
	if (sex != NULL) {
		if (sex[0] == 'm' || sex[0] == 'M')
			p->sex = 'm';
		else if (sex[0] == 'f' || sex[0] == 'F')
			p->sex = 'f';
		else
			p->sex = 0;
	}
	if (group != NULL) {
		uS.remblanks(group);
		strncpy(p->group, group, IDFIELSSIZE);
		p->group[IDFIELSSIZE] = EOS;
	}
	if (SES != NULL) {
		SESTYPE *se, *ss;

		uS.remblanks(SES);
		p->SES[0] = EOS;
		for (se=RolesSes->rootSESe; se != NULL; se = se->nextses) {
			if (isPartOfSES(se->st, SES) == true) {
				strcat(p->SES, se->st);
				break;
			}
		}
		for (ss=RolesSes->rootSESs; ss != NULL; ss=ss->nextses) {
			if (isPartOfSES(ss->st, SES) == true) {
				if (p->SES[0] != EOS)
					strcat(p->SES, ",");
				strcat(p->SES, ss->st);
				break;
			}
		}
	}
	if (role != NULL) {
		ROLESTYPE *rr;

		uS.remblanks(role);
		p->role[0] = EOS;
		for (rr=RolesSes->rootRoles; rr != NULL; rr=rr->nextrole) {
			if (uS.mStricmp(rr->role, role) == 0) {
				strcpy(p->role, rr->role);
				break;
			}
		}
		if (rr == NULL && spn == NULL) {
			strncpy(p->spname, role, IDFIELSSIZE);
			p->spname[IDFIELSSIZE] = EOS;
		}
	}
	if (educ != NULL) {
		uS.remblanks(educ);
		strncpy(p->education, educ, IDFIELSSIZE);
		p->education[IDFIELSSIZE] = EOS;
	}
	if (fu != NULL) {
		uS.remblanks(fu);
		strncpy(p->custom_field, fu, IDFIELSSIZE);
		p->custom_field[IDFIELSSIZE] = EOS;
	}
	if (spn != NULL) {
		uS.remblanks(spn);
		strncpy(p->spname, spn, IDFIELSSIZE);
		p->spname[IDFIELSSIZE] = EOS;
	}
	return(rootIDs);
}

static IDSTYPE *handleParticipants(IDSTYPE *rootIDs, DEPFDEFS *RolesSes, unCH *line) {
	unCH sp[SPEAKERLEN];
	unCH *s, *e, t, wc, tchFound;
	short cnt = 0;
	
	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	tchFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ',' || isSpace(*line) || *line == '\n' || *line == NL_C || *line == SNL_C || *line == EOS) {
			wc = ' ';
			e = line;
			for (; *line != EOS && (isSpace(*line) || *line == '\n' || *line == NL_C || *line == SNL_C); line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					rootIDs = add_to_IDs(rootIDs, RolesSes, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
				} else if (cnt == 1) {
					rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, s);
					if (rootIDs == NULL)
						return(NULL);
				} else if (cnt == 0) {
					strcpy(sp, s);
					rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
				}
				*e = t;
				if (wc == ',') {
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; isSpace(*line) || *line=='\n' || *line == NL_C || *line == SNL_C || *line==','; line++) {
					if (*line == ',') {
						cnt = -1;
						sp[0] = EOS;
					}
				}
			} else {
				for (line=e; *line; line++) {
				}
				if (cnt != 0) {
					t = *e;
					*e = EOS;
					rootIDs = add_to_IDs(rootIDs, RolesSes, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
					*e = t;
				}
				for (line=e; *line; line++) {
				}
			}
			if (cnt == 2) {
				cnt = 0;
				sp[0] = EOS;
			} else
				cnt++;
			s = line;
		} else
			line++;
	}
	return(rootIDs);
}


static IDSTYPE *handleIDs(IDSTYPE *rootIDs, DEPFDEFS *RolesSes, unCH *languages, unCH *line) {
	int t, s = 0, e = 0, cnt;
	unCH sp[SPEAKERLEN];
	unCH word[SPEAKERLEN], *st;
	
	word[0] = EOS;
	sp[0] = EOS;
	while (line[s] != EOS && line[s] != NL_C && line[s] != SNL_C) {
		if (!isSpace(line[s]))
			break;
		s++;
	}
	if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
		return(rootIDs);
	t = s;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS || *st == NL_C || *st == SNL_C)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS && *st != NL_C && *st != SNL_C) {
			e++;
			if (line[e-1] == '|')
				break;
		}
		*st = EOS;
		if (cnt == 0) {			// language
		} else if (cnt == 1) {	// corpus
		} else if (cnt == 2) {	// code
			strcpy(sp, word);
			break;
		}
		s = e;
		cnt++;
	}
	if (sp[0] == EOS || sp[0] == NL_C || sp[0] == SNL_C)
		return(rootIDs);
	word[0] = EOS;
	s = t;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS || *st == NL_C || *st == SNL_C)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS && *st != NL_C && *st != SNL_C) {
			e++;
			if (line[e-1] == '|')
				break;
		}
		*st = EOS;
		if (cnt == 0) {			// language
			cleanup_lang(word);
			add_to_languages(languages, word, FALSE);
			rootIDs = add_to_IDs(rootIDs, NULL, word, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 1) {	// corpus
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, word, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 2) {	// code
			
		} else if (cnt == 3) {	// age
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, word, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 4) {	// sex
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, word, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 5) {	// group
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, NULL, word, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 6) {	// SES
			rootIDs = add_to_IDs(rootIDs, RolesSes, NULL, NULL, sp, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL);
		} else if (cnt == 7) {	// role
			rootIDs = add_to_IDs(rootIDs, RolesSes, NULL, NULL, sp, NULL, NULL, NULL, NULL, word, NULL, NULL, NULL);
		} else if (cnt == 8) {	// education
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, word, NULL, NULL);
		} else if (cnt == 9) {	// file unique ID
			rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, word, NULL);
		}
		s = e;
		if (rootIDs == NULL)
			return(NULL);
		cnt++;
	}
	
	return(rootIDs);
}

static void id_AddText(unCH *s, long len) {
	if (global_df->UndoList->NextUndo) {
		if (global_df->UndoList->key != INSTRPT)
			global_df->UndoList->key = INSTKEY;
	}
	ChangeCurLine();

	if (global_df->col_txt == global_df->tail_row && global_df->col_txt->prev_char->c == NL_C) {
		global_df->UndoList->key = MOVEKEY;
		global_df->col_chr--;
		global_df->col_txt = global_df->col_txt->prev_char;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		SaveUndoState(FALSE);
		global_df->UndoList->key = INSTRPT;
	}
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	global_df->col_txt = global_df->col_txt->prev_char;
	AddString(s, len, TRUE);
	global_df->col_txt = global_df->col_txt->next_char;
}

static void replaceSpaceWithUnderline(unCH *s) {
	for (; *s != EOS; s++) {
		if (*s == ' ')
			*s = '_';
	}
}

static void updateHeadersInText(IDSTYPE *rootIDs) {
	char isRightTierFound;
	long len;
	unCH languages[IDFIELSSIZE+1];
	IDSTYPE *IDs;

	languages[0] = EOS;
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		if (IDs->language[0] != EOS) {
			add_to_languages(languages, IDs->language, TRUE);
		}
	}
	BeginningOfFile(-1);
	if (uS.partcmp(global_df->row_txt->line, "@Begin", FALSE, FALSE))
		MoveDown(-1);
	else {
		strcpy(templineW, "@Begin");
		len = strlen(templineW);
		templineW[len++] = NL_C;
		id_AddText(templineW, len);
	}
	isRightTierFound = FALSE;
	while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
		ChangeCurLineAlways(0);
		if (uS.partcmp(global_df->row_txt->line, "@Languages:", FALSE, FALSE)) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
			isRightTierFound = TRUE;
		} else if (isRightTierFound && isSpace(global_df->row_txt->line[0])) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
		} else {
			break;
		}
	}
	if (languages[0] == EOS)
		strcpy(languages, "change_me_later");
	strcpy(templineW, "@Languages:	");
	strcat(templineW, languages);
	len = strlen(templineW);
	templineW[len++] = NL_C;
	id_AddText(templineW, len);
	isRightTierFound = FALSE;
	while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
		ChangeCurLineAlways(0);
		if (uS.partcmp(global_df->row_txt->line, PARTICIPANTS, FALSE, FALSE)) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
			isRightTierFound = TRUE;
		} else if (isRightTierFound && isSpace(global_df->row_txt->line[0])) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
		} else {
			break;
		}
	}
	strcpy(templineW, "@Participants:	");
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		if (IDs->code[0] != EOS) {
			if (IDs != rootIDs)
				strcat(templineW, ", ");
			strcat(templineW, IDs->code);
			if (IDs->spname[0] != EOS) {
				strcat(templineW, " ");
				strcat(templineW, IDs->spname);
			}
			strcat(templineW, " ");
			strcat(templineW, IDs->role);
		}
	}
	len = strlen(templineW);
	templineW[len++] = NL_C;
	id_AddText(templineW, len);
	isRightTierFound = FALSE;
	while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
		ChangeCurLineAlways(0);
		if (uS.partcmp(global_df->row_txt->line, IDOF, FALSE, FALSE)) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
			isRightTierFound = TRUE;
		} else if (isRightTierFound && isSpace(global_df->row_txt->line[0])) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
		} else {
			break;
		}
	}
	if (uS.partcmp(global_df->row_txt->line, "@Options:", FALSE, FALSE))
		MoveDown(-1);
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		strcpy(templineW, "@ID:	");
		len = strlen(templineW);
		add_to_languages(templineW+len, IDs->language, TRUE);
		strcat(templineW, "|");
		if (IDs->corpus[0] == EOS)
			strcat(templineW, "change_corpus_later");
		else
			strcat(templineW, IDs->corpus);
		strcat(templineW, "|");
		strcat(templineW, IDs->code);
		strcat(templineW, "|");
		templineW1[0] = EOS;
		if (IDs->age1y != -1) {
			uS.sprintf(templineW1, cl_T("%d;"), IDs->age1y);
			strcat(templineW, templineW1);
		}
		if (IDs->age1m != -1) {
			if (IDs->age1m < 10)
				uS.sprintf(templineW1, cl_T("0%d."), IDs->age1m);
			else
				uS.sprintf(templineW1, cl_T("%d."), IDs->age1m);
			strcat(templineW, templineW1);
		}
		if (IDs->age1d != -1) {
			if (IDs->age1d < 10)
				uS.sprintf(templineW1, cl_T("0%d"), IDs->age1d);
			else
				uS.sprintf(templineW1, cl_T("%d"), IDs->age1d);
			strcat(templineW, templineW1);
		}
		strcat(templineW, "|");
		if (IDs->sex == 'm')
			strcat(templineW, "male|");
		else if (IDs->sex == 'f')
			strcat(templineW, "female|");
		else 
			strcat(templineW, "|");
		replaceSpaceWithUnderline(IDs->group);
		strcat(templineW, IDs->group);
		strcat(templineW, "|");
		replaceSpaceWithUnderline(IDs->SES);
		strcat(templineW, IDs->SES);
		strcat(templineW, "|");
		strcat(templineW, IDs->role);
		strcat(templineW, "|");
		replaceSpaceWithUnderline(IDs->education);
		strcat(templineW, IDs->education);
		strcat(templineW, "|");
		replaceSpaceWithUnderline(IDs->custom_field);
		strcat(templineW, IDs->custom_field);
		strcat(templineW, "|");
		len = strlen(templineW);
		templineW[len++] = NL_C;
		id_AddText(templineW, len);
	}
	isRightTierFound = FALSE;
	global_df->redisplay = 0;
	while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
		ChangeCurLineAlways(0);
		if (uS.partcmp(global_df->row_txt->line, "@Languages:", FALSE, FALSE)	||
			uS.partcmp(global_df->row_txt->line, PARTICIPANTS, FALSE, FALSE)	||
			uS.partcmp(global_df->row_txt->line, IDOF, FALSE, FALSE)			||
//			uS.partcmp(global_df->row_txt->line, "@Language of ", FALSE, FALSE)	||
			uS.partcmp(global_df->row_txt->line, "@Age of ", FALSE, FALSE)		||
			uS.partcmp(global_df->row_txt->line, "@Sex of ", FALSE, FALSE)		||
			uS.partcmp(global_df->row_txt->line, "@Group of ", FALSE, FALSE)	||
			uS.partcmp(global_df->row_txt->line, "@Ses of ", FALSE, FALSE)		||
			uS.partcmp(global_df->row_txt->line, "@Education of ", FALSE, FALSE)) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
			isRightTierFound = TRUE;
		} else if (isRightTierFound && isSpace(global_df->row_txt->line[0])) {
			while (global_df->col_txt != global_df->tail_row)
				DeleteNextChar(-1);
			if (global_df->cur_line->next_row != global_df->tail_text)
				DeleteNextChar(-1);
		} else {
			isRightTierFound = FALSE;
			MoveDown(-1);
		}
		if (isMainSpeaker(global_df->row_txt->line[0]))
			break;
	}
	ResetUndos();
	global_df->redisplay = 1;
	DisplayTextWindow(NULL, 1);
}

static void id_remblanks(unCH *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == NL_C || st[i] == SNL_C))
		i--;
	st[i+1] = EOS;
}

int setIDs(int c) {
	register int i;
	char triedProcess;
	unCH languages[IDFIELSSIZE+1];
	ROWS *tt;
	IDSTYPE *rootIDs, *IDs;
	DEPFDEFS RolesSes;

	if (global_df == NULL)
		return(66);
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(66);
	}
	RolesSes.rootRoles = NULL;
	RolesSes.rootSESe = NULL;
	RolesSes.rootSESs = NULL;
	if (getRoles(&RolesSes) == false) {
		clean_roles(RolesSes.rootRoles);
		clean_SESs(RolesSes.rootSESe);
		clean_SESs(RolesSes.rootSESs);
		draw_mid_wm();
		return(66);
	}
	if (getSES(&RolesSes) == false) {
		clean_roles(RolesSes.rootRoles);
		clean_SESs(RolesSes.rootSESe);
		clean_SESs(RolesSes.rootSESs);
		draw_mid_wm();
		return(66);
	}
	languages[0] = EOS;
	rootIDs = NULL;
	sp[0] = EOS;
	ced_line[0] = EOS;
	ChangeCurLineAlways(0);
	tt = global_df->head_text->next_row;
	while (1) {
		if (sp[0] != EOS && ced_line[0] != EOS) {
			triedProcess = FALSE;
			if (uS.partcmp(sp, "@Languages", FALSE, FALSE)) {
				cleanup_lang(ced_line);
				add_to_languages(languages, ced_line, FALSE);
			} else if (uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE)) {
				rootIDs = handleParticipants(rootIDs, &RolesSes, ced_line);
				triedProcess = TRUE;
			} else if (uS.partcmp(sp, IDOF, FALSE, FALSE)) {
				rootIDs = handleIDs(rootIDs, &RolesSes, languages, ced_line);
				triedProcess = TRUE;
			} else if (uS.patmat(sp,cl_T("@Language of *"))) {
/*
				 cleanup_lang(ced_line);
				 add_to_languages(languages, ced_line, FALSE);
				 uS.extractString(templine2, sp, "@Language of ", ':');
				 rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, ced_line, NULL, templine2, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				 triedProcess = TRUE;
*/
			} else if (uS.patmat(sp,cl_T(AGEOF))) {
				uS.extractString(templine2, sp, "@Age of ", ':');
				rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, templine2, ced_line, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				triedProcess = TRUE;
			} else if (uS.patmat(sp,cl_T(SEXOF))) {
				uS.extractString(templine2, sp, "@Sex of ", ':');
				rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, templine2, NULL, ced_line, NULL, NULL, NULL, NULL, NULL, NULL);
				triedProcess = TRUE;
			} else if (uS.patmat(sp,cl_T(GROUPOF))) {
				uS.extractString(templine2, sp, "@Group of ", ':');
				rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, templine2, NULL, NULL, ced_line, NULL, NULL, NULL, NULL, NULL);
				triedProcess = TRUE;
			} else if (uS.patmat(sp,cl_T(SESOF))) {
				uS.extractString(templine2, sp, "@Ses of ", ':');
				rootIDs = add_to_IDs(rootIDs, &RolesSes, NULL, NULL, templine2, NULL, NULL, NULL, ced_line, NULL, NULL, NULL, NULL);
				triedProcess = TRUE;
			} else if (uS.patmat(sp,cl_T(EDUCOF))) {
				uS.extractString(templine2, sp, "@Education of ", ':');
				rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, templine2, NULL, NULL, NULL, NULL, NULL, ced_line, NULL, NULL);
				triedProcess = TRUE;
			}
			if (triedProcess && rootIDs == NULL) {
				strcpy(global_df->err_message, "+Can't continue: Out of memory.");
				clean_roles(RolesSes.rootRoles);
				clean_SESs(RolesSes.rootSESe);
				clean_SESs(RolesSes.rootSESs);
				draw_mid_wm();
				return(66);
			}	
		}
		if (tt == global_df->tail_text)
			break;
		if (isSpeaker(tt->line[0])) {
			if (isMainSpeaker(tt->line[0]))
				break;
			for (i=0; tt->line[i] && tt->line[i] != ':' && i < SPEAKERLEN-2; i++)
				sp[i] = tt->line[i];
			if (tt->line[i] == ':') {
				sp[i] = ':';
				i++;
			}
			sp[i] = EOS;
			for (; isSpace(tt->line[i]); i++) ;
			if (tt->line[i] != EOS) {
				strcpy(ced_line, tt->line+i);
				id_remblanks(ced_line);
			} else
				ced_line[0] = EOS;
		} else if (strlen(ced_line)+strlen(tt->line) < UTTLINELEN-10) {
			strcat(ced_line, tt->line);
			id_remblanks(ced_line);
		}
		tt = tt->next_row;
	}
	if (rootIDs == NULL) {
		rootIDs = createNewId(rootIDs, 0);
		if (rootIDs == NULL) {
			strcpy(global_df->err_message, "+Can't continue: Out of memory.");
			clean_roles(RolesSes.rootRoles);
			clean_SESs(RolesSes.rootSESe);
			clean_SESs(RolesSes.rootSESs);
			draw_mid_wm();
			return(66);
		}
	} else {
		for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
			if (IDs->language[0] == EOS)
				strcpy(IDs->language, languages);
		}
	}
	if (IDDialog(&rootIDs, &RolesSes)) {
		updateHeadersInText(rootIDs);
		for (i=0; i < 10; i++)
			AllocSpeakerNames(cl_T(""), i);
		SetUpParticipants();
#ifdef _MAC_CODE
		ChangeSpeakerMenuItem();
#endif
	}
	clean_ids(rootIDs);
	clean_roles(RolesSes.rootRoles);
	clean_SESs(RolesSes.rootSESe);
	clean_SESs(RolesSes.rootSESs);
	return(66);
}

Boolean isPartOfSES(unCH *st, unCH *SES) {
	int i, s;
	unCH word[IDFIELSSIZE + 1];

	s = 0;
	while (1) {
		i = 0;
		if (SES[s] == EOS) {
			word[0] = EOS;
			break;
		}
		while (SES[s] == ',')
			s++;
		while ((word[i]=SES[s]) != EOS && word[i] != ',') {
			i++;
			s++;
		}
		word[i] = EOS;
		uS.remFrontAndBackBlanks(word);
		if (strcmp(word, st) == 0)
			return(true);
	}
	return(false);
}
