
#import "DocumentWinController.h"
#import "Document.h"
#import "IdsController.h"
#import "IntegerFormater.h"
#import "c_clan.h"
#import "check.h"

extern FNType mDirPathName[];

char ids_err_message[ERRMESSAGELEN];

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


static Boolean isPartOfSES(unCH *st, unCH *SES) {
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

static ROLESTYPE *addNewRole(ROLESTYPE *rolep, unCH *role) {
	int i;
	ROLESTYPE *p, *t;

	if ((p=NEW(ROLESTYPE)) == NULL) {
		strcpy(ids_err_message, "+Can't continue: Out of memory.");
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
		strcpy(ids_err_message, "+Can't continue: Out of memory.");
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

static Boolean getRoles(DEPFDEFS *Roles_Ses) {
	int  b, e;
	char pf;
	FILE *fp;
	FNType mDirPathName[FNSize];

	fp = NULL;
/*
	if (!isRefEQZero(global_dfC->fileName)) {
		extractPath(mDirPathName, global_dfC->fileName);
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
			strcpy(ids_err_message, "+Error: Can't do check without depfile.");
			return(false);
		} else {
			if (pathcmp(lib_dir, mDirPathName) != 0) {
				strcpy(lib_dir, mDirPathName);
				WriteCedPreference();
#ifdef _MAC_CODE
//				UpdateWindowNamed(Commands_str);
#endif // _MAC_CODE
			}
			addFilename2Path(mDirPathName, DEPFILE);
			fp = fopen(mDirPathName,"r");
			if (fp == NULL) {
				strcpy(ids_err_message, "+Error: Can't open depfile \"");
				uS.FNType2str(ids_err_message, strlen(ids_err_message), mDirPathName);
				strcat(ids_err_message, "\".");
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
					Roles_Ses->depfileRoles = addNewRole(Roles_Ses->depfileRoles, templine+b);
					if (Roles_Ses->depfileRoles == NULL)
						return(false);
					b = e;
				} else {
					templine[e] = EOS;
					Roles_Ses->depfileRoles = addNewRole(Roles_Ses->depfileRoles, templine+b);
					if (Roles_Ses->depfileRoles == NULL)
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
		strcpy(ids_err_message, "+Can't continue: Out of memory.");
		clean_SESs(SES);
		if (s != NULL)
			*s = '>';
		return(NULL);
	}
	if ((p->st=(unCH *)malloc((strlen(st)+1)*sizeof(unCH))) == NULL) {
		strcpy(ids_err_message, "+Can't continue: Out of memory.");
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

static Boolean getSES(DEPFDEFS *Roles_Ses) {
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
		strcpy(ids_err_message, "+Error: Can't open depfile \"");
		uS.FNType2str(ids_err_message, strlen(ids_err_message), mDirPathName);
		strcat(ids_err_message, "\".");
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
					Roles_Ses->depfileSESe = addNewSES(Roles_Ses->depfileSESe, templine+b+2);
					if (Roles_Ses->depfileSESe == NULL)
						return(false);
				} else if (templine[b] == '@' && templine[b+1] == 's') {
					Roles_Ses->depfileSESs = addNewSES(Roles_Ses->depfileSESs, templine+b+2);
					if (Roles_Ses->depfileSESs == NULL)
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


static IDSTYPE *deleteID(IDSTYPE *rootIDs, NSInteger item) {
	IDSTYPE *p, *pp;

	p = rootIDs;
	pp = rootIDs;
	while (p != NULL) {
		item--;
		if (item < 0)
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

static IDSTYPE *createNewId(IDSTYPE *rootIDs, NSInteger item) {
	int  i;
	char numS[256];
	unCH code[CODESIZE];
	IDSTYPE *p, *oldId;

	if (item >= 0) {
		for (oldId=rootIDs; oldId != NULL; oldId=oldId->next_id) {
			item--;
			if (item < 0)
				break;
		}
	} else
		oldId = NULL;

	for (i=1; i < 1000; i++) {
		if (i < 10) {
			strcpy(code, "SP0");
			uS.sprintf(numS,  "%d", i);
		} else {
			strcpy(code, "SP");
			uS.sprintf(numS,  "%d", i);
		}
		strcat(code, numS);
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

static IDSTYPE *add_to_IDs(IDSTYPE *rootIDs, DEPFDEFS *Roles_Ses, unCH *lang, unCH *corp, unCH *code, unCH *age, unCH *sex, unCH *group, unCH *SES, unCH *role, unCH *educ, unCH*fu, unCH *spn) {
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
		for (se=Roles_Ses->depfileSESe; se != NULL; se = se->nextses) {
			if (isPartOfSES(se->st, SES) == true) {
				strcat(p->SES, se->st);
				break;
			}
		}
		for (ss=Roles_Ses->depfileSESs; ss != NULL; ss=ss->nextses) {
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
		for (rr=Roles_Ses->depfileRoles; rr != NULL; rr=rr->nextrole) {
			if (uS.mStricmp(rr->role, role) == 0) {
				strcpy(p->role, rr->role);
				break;
			}
		}
		if (rr == NULL && spn == NULL && p->spname[0] == EOS) {
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

static IDSTYPE *handleParticipants(IDSTYPE *rootIDs, DEPFDEFS *Roles_Ses, unCH *line) {
	unCH sp[SPEAKERLEN];
	unCH *s, *e, t, wc, tchFound;
	short cnt = 0;

	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	tchFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ',' || isSpace(*line) || *line == '\n' || *line == EOS) {
			wc = ' ';
			e = line;
			for (; *line != EOS && (isSpace(*line) || *line == '\n'); line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					rootIDs = add_to_IDs(rootIDs, Roles_Ses, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL);
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
				for (line++; isSpace(*line) || *line=='\n' || *line==','; line++) {
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
					rootIDs = add_to_IDs(rootIDs, Roles_Ses, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL);
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

static IDSTYPE *handleIDs(IDSTYPE *rootIDs, DEPFDEFS *Roles_Ses, unCH *languages, unCH *line) {
	int t, s = 0, e = 0, cnt;
	unCH sp[SPEAKERLEN];
	unCH word[SPEAKERLEN], *st;

	word[0] = EOS;
	sp[0] = EOS;
	while (line[s] != EOS) {
		if (!isSpace(line[s]))
			break;
		s++;
	}
	if (line[s] == EOS)
		return(rootIDs);
	t = s;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS) {
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
	if (sp[0] == EOS)
		return(rootIDs);
	word[0] = EOS;
	s = t;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS) {
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
			rootIDs = add_to_IDs(rootIDs, Roles_Ses, NULL, NULL, sp, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL);
		} else if (cnt == 7) {	// role
			rootIDs = add_to_IDs(rootIDs, Roles_Ses, NULL, NULL, sp, NULL, NULL, NULL, NULL, word, NULL, NULL, NULL);
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

static void id_AddText(NSTextView *textView, NSTextStorage *text, NSRange charRange, unCH *line) {
	NSString *keys;

	keys = [NSString stringWithCharacters:line length:strlen(line)];
	if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
		[text replaceCharactersInRange:charRange withString:keys];
		[textView didChangeText];
		[[textView undoManager] setActionName:@"IDs"];
	}
}

@implementation IdsController

static IdsController *IdsWindow = nil;

- (id)init {
	IdsWindow = nil;
	return [super initWithWindowNibName:@"Ids"];
}

+(char *)idsDialog:(DocumentWindowController *)docWinController;
{
	char triedProcess;
	unCH languages[IDFIELSSIZE+1];
	IDSTYPE *rootIDs, *IDs;
	DEPFDEFS Roles_Ses;
	NSUInteger pos, len;
	NSTextView *textView;
	NSTextStorage *text;
	NSString *textSt;

	NSLog(@"IdsController: idsDialog\n");
	if (IdsWindow == nil) {
		ids_err_message[0] = EOS;
		Roles_Ses.depfileRoles = NULL;
		Roles_Ses.depfileSESe = NULL;
		Roles_Ses.depfileSESs = NULL;
		if (getRoles(&Roles_Ses) == false) {
			clean_roles(Roles_Ses.depfileRoles);
			clean_SESs(Roles_Ses.depfileSESe);
			clean_SESs(Roles_Ses.depfileSESs);
			return(ids_err_message);
		}
		if (getSES(&Roles_Ses) == false) {
			clean_roles(Roles_Ses.depfileRoles);
			clean_SESs(Roles_Ses.depfileSESe);
			clean_SESs(Roles_Ses.depfileSESs);
			return(ids_err_message);
		}

		textView = [docWinController firstTextView];
		text = [textView textStorage];
		textSt = [text string];
		len = [text length];
		pos = 0;

		languages[0] = EOS;
		rootIDs = NULL;
		sp[0] = EOS;
		ced_line[0] = EOS;
		while (1) {
			if (sp[0] != EOS && ced_line[0] != EOS) {
				triedProcess = FALSE;
				if (uS.partcmp(sp, "@Languages", FALSE, FALSE)) {
					cleanup_lang(ced_line);
					add_to_languages(languages, ced_line, FALSE);
				} else if (uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE)) {
					rootIDs = handleParticipants(rootIDs, &Roles_Ses, ced_line);
					triedProcess = TRUE;
				} else if (uS.partcmp(sp, IDOF, FALSE, FALSE)) {
					rootIDs = handleIDs(rootIDs, &Roles_Ses, languages, ced_line);
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
					rootIDs = add_to_IDs(rootIDs, &Roles_Ses, NULL, NULL, templine2, NULL, NULL, NULL, ced_line, NULL, NULL, NULL, NULL);
					triedProcess = TRUE;
				} else if (uS.patmat(sp,cl_T(EDUCOF))) {
					uS.extractString(templine2, sp, "@Education of ", ':');
					rootIDs = add_to_IDs(rootIDs, NULL, NULL, NULL, templine2, NULL, NULL, NULL, NULL, NULL, ced_line, NULL, NULL);
					triedProcess = TRUE;
				}
				if (triedProcess && rootIDs == NULL) {
					strcpy(ids_err_message, "+Can't continue: Out of memory.");
					clean_roles(Roles_Ses.depfileRoles);
					clean_SESs(Roles_Ses.depfileSESe);
					clean_SESs(Roles_Ses.depfileSESs);
					return(ids_err_message);
				}
			}

			pos = getUtts(pos, len, textSt);
			uS.remblanks(ced_line);

			if (!isSpeaker(sp[0]))
				break;
			if (isMainSpeaker(sp[0]))
				break;
		}
		if (rootIDs == NULL) {
			rootIDs = createNewId(rootIDs, 0);
			if (rootIDs == NULL) {
				strcpy(ids_err_message, "+Can't continue: Out of memory.");
				clean_roles(Roles_Ses.depfileRoles);
				clean_SESs(Roles_Ses.depfileSESe);
				clean_SESs(Roles_Ses.depfileSESs);
				return(ids_err_message);
			}
		} else {
			for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
				if (IDs->language[0] == EOS)
					strcpy(IDs->language, languages);
			}
		}

		IdsWindow = [[IdsController alloc] initWithWindowNibName:@"Ids"];
		IdsWindow->docWinController = docWinController;
		IdsWindow->depfileRoles = Roles_Ses.depfileRoles;
		IdsWindow->depfileSESe = Roles_Ses.depfileSESe;
		IdsWindow->depfileSESs = Roles_Ses.depfileSESs;
		IdsWindow->rootIDs = rootIDs;
//		[IdsWindow showWindow:nil];
		[[docWinController window] beginSheet:[IdsWindow window] completionHandler:nil];
	}
	return(NULL);
}

- (void)createDepfilePopupMenus {
	NSUInteger i;
	NSString *str;
	SESTYPE *ses;
	ROLESTYPE *role;

	[racePopUp insertItemWithTitle:@"Unknown " atIndex:0];
	i = 1;
	for (ses=depfileSESe; ses != NULL; ses=ses->nextses) {
		str = [NSString stringWithCharacters:ses->st length:strlen(ses->st)];
		[racePopUp insertItemWithTitle:str atIndex:i];
		i++;
	}
	[racePopUp selectItemAtIndex:0];

	[sesPopUp insertItemWithTitle:@"UNK " atIndex:0];
	i = 1;
	for (ses=depfileSESs; ses != NULL; ses=ses->nextses) {
		str = [NSString stringWithCharacters:ses->st length:strlen(ses->st)];
		[sesPopUp insertItemWithTitle:str atIndex:i];
		i++;
	}
	[sesPopUp selectItemAtIndex:0];

	[rolePopUp insertItemWithTitle:@"Choose one role" atIndex:0];
	i = 1;
	for (role=depfileRoles; role != NULL; role=role->nextrole) {
		str = [NSString stringWithCharacters:role->role length:strlen(role->role)];
		[rolePopUp insertItemWithTitle:str atIndex:i];
		i++;
	}
	[rolePopUp selectItemAtIndex:0];
}

- (void)createUserPopupMenus {
	NSInteger len;
	NSUInteger i;
	NSString *str;
	IDSTYPE *IDs;

	if ([usersPopUp numberOfItems] > 0)
		[usersPopUp removeAllItems];
	i = 0;
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		str = [NSString stringWithCharacters:IDs->code length:strlen(IDs->code)];
		[usersPopUp insertItemWithTitle:str atIndex:i];
		i++;
	}
	len = [usersPopUp numberOfItems];
	if (spItem < 0 || spItem >= len)
		spItem = 0;
	[usersPopUp selectItemAtIndex:spItem];
}

- (void)fillInIDFields {
	char numS[CODESIZE];
	unCH st[IDFIELSSIZE+1];
	NSInteger item;
	NSString *menuStr;
	NSUInteger len;
	IDSTYPE *p;

	if (spItem < 0 || spItem >= [usersPopUp numberOfItems])
		return;
	item = spItem;
	for (p=rootIDs; p != NULL; p=p->next_id) {
		item--;
		if (item < 0)
			break;
	}
	if (p != NULL) {
		[langField setStringValue:[NSString stringWithCharacters:p->language length:strlen(p->language)]];
		[corpusField setStringValue:[NSString stringWithCharacters:p->corpus length:strlen(p->corpus)]];
		[codeField setStringValue:[NSString stringWithCharacters:p->code length:strlen(p->code)]];

		if (p->age1y != -1)
			uS.sprintf(numS, "%d", p->age1y);
		else
			numS[0] = EOS;
		[ageYearField setStringValue:[NSString stringWithUTF8String:numS]];
		if (p->age1m != -1)
			uS.sprintf(numS, "%d", p->age1m);
		else
			numS[0] = EOS;
		[ageMonthField setStringValue:[NSString stringWithUTF8String:numS]];
		if (p->age1d != -1)
			uS.sprintf(numS, "%d", p->age1d);
		else
			numS[0] = EOS;
		[ageDateField setStringValue:[NSString stringWithUTF8String:numS]];

		if (p->sex == 'm' || p->sex == 'M') {
			maleButton.state = NSControlStateValueOn;
			femaleButton.state = NSControlStateValueOff;
			unknownButton.state = NSControlStateValueOff;
		} else if (p->sex == 'f' || p->sex == 'F') {
			maleButton.state = NSControlStateValueOff;
			femaleButton.state = NSControlStateValueOn;
			unknownButton.state = NSControlStateValueOff;
		} else {
			maleButton.state = NSControlStateValueOff;
			femaleButton.state = NSControlStateValueOff;
			unknownButton.state = NSControlStateValueOn;
		}
		[groupField setStringValue:[NSString stringWithCharacters:p->group length:strlen(p->group)]];

		[racePopUp selectItemAtIndex:0];
		[sesPopUp selectItemAtIndex:0];
		if (p->SES[0] != EOS) {
			for (item=1; item < [racePopUp numberOfItems]; item++) {
				menuStr = [racePopUp itemTitleAtIndex:item];
				len = [menuStr length];
				[menuStr getCharacters:st range:NSMakeRange(0, len)];
				st[len] = EOS;
				if (isPartOfSES(st, p->SES) == true) {
					[racePopUp selectItemAtIndex:item];
					break;
				}
			}
			for (item=1; item < [sesPopUp numberOfItems]; item++) {
				menuStr = [sesPopUp itemTitleAtIndex:item];
				len = [menuStr length];
				[menuStr getCharacters:st range:NSMakeRange(0, len)];
				st[len] = EOS;
				if (isPartOfSES(st, p->SES) == true) {
					[sesPopUp selectItemAtIndex:item];
					break;
				}
			}
		}

		[rolePopUp selectItemAtIndex:0];
		if (p->role[0] != EOS) {
			for (item=1; item < [rolePopUp numberOfItems]; item++) {
				menuStr = [rolePopUp itemTitleAtIndex:item];
				len = [menuStr length];
				[menuStr getCharacters:st range:NSMakeRange(0, len)];
				st[len] = EOS;
				if (uS.mStricmp(p->role, st) == 0) {
					[rolePopUp selectItemAtIndex:item];
					break;
				}
			}
		}

		[educationField setStringValue:[NSString stringWithCharacters:p->education length:strlen(p->education)]];
		[customField setStringValue:[NSString stringWithCharacters:p->custom_field length:strlen(p->custom_field)]];
		[optionalField setStringValue:[NSString stringWithCharacters:p->spname length:strlen(p->spname)]];
	}
}

- (BOOL)saveIDFields:(NSInteger)item {
	char	err_mess[512];
	unCH	numS[CODESIZE];
	unCH	st[IDFIELSSIZE+1];
	NSString *menuStr;
	NSUInteger len;
	IDSTYPE *p;

	if (item < 0 || item >= [usersPopUp numberOfItems])
		return(true);
	for (p=rootIDs; p != NULL; p=p->next_id) {
		item--;
		if (item < 0)
			break;
	}
	if (p != NULL) {
		menuStr = [langField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->language range:NSMakeRange(0, len)];
		p->language[len] = EOS;

		menuStr = [corpusField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->corpus range:NSMakeRange(0, len)];
		p->corpus[len] = EOS;

		menuStr = [codeField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->code range:NSMakeRange(0, len)];
		p->code[len] = EOS;
		uS.uppercasestr(p->code, NULL, 0);

		menuStr = [ageYearField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		if (numS[0] == EOS)
			p->age1y = -1;
		else
			p->age1y = uS.atoi(numS);

		menuStr = [ageMonthField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		if (numS[0] == EOS)
			p->age1m = -1;
		else
			p->age1m = uS.atoi(numS);

		menuStr = [ageDateField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:numS range:NSMakeRange(0, len)];
		numS[len] = EOS;
		if (numS[0] == EOS)
			p->age1d = -1;
		else
			p->age1d = uS.atoi(numS);

		if ([maleButton state] == NSControlStateValueOn) {
			p->sex = 'm';
		} else if ([femaleButton state] == NSControlStateValueOn) {
			p->sex = 'f';
		} else {
			p->sex = 0;
		}

		menuStr = [groupField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->group range:NSMakeRange(0, len)];
		p->group[len] = EOS;

		p->SES[0] = EOS;
		item = [racePopUp indexOfSelectedItem];
		if (item != 0) {
			menuStr = [racePopUp itemTitleAtIndex:item];
			len = [menuStr length];
			[menuStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			strcat(p->SES, st);
		}
		item = [sesPopUp indexOfSelectedItem];
		if (item != 0) {
			menuStr = [sesPopUp itemTitleAtIndex:item];
			len = [menuStr length];
			[menuStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			if (p->SES[0] != EOS)
				strcat(p->SES, ",");
			strcat(p->SES, st);
		}

		item = [rolePopUp indexOfSelectedItem];
		if (item == 0) {
			p->role[0] = EOS;
		} else {
			menuStr = [rolePopUp itemTitleAtIndex:item];
			len = [menuStr length];
			[menuStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			strcpy(p->role, st);
		}

		menuStr = [educationField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->education range:NSMakeRange(0, len)];
		p->education[len] = EOS;

		menuStr = [customField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->custom_field range:NSMakeRange(0, len)];
		p->custom_field[len] = EOS;

		menuStr = [optionalField stringValue];
		len = [menuStr length];
		[menuStr getCharacters:p->spname range:NSMakeRange(0, len)];
		p->spname[len] = EOS;

		if (p->language[0] == EOS) {
			sprintf(err_mess, "Please fill-in \"Language\" field");
			do_warning_sheet(err_mess, [self window]);
			return(false);
		}
		if (p->code[0] == EOS) {
			sprintf(err_mess, "Please fill-in \"Name code\" field");
			do_warning_sheet(err_mess, [self window]);
			return(false);
		}
		if (p->role[0] == EOS) {
			sprintf(err_mess, "Please select a \"Role\"");
			do_warning_sheet(err_mess, [self window]);
			return(false);
		}
		if (p->age1m != -1) {
			if (p->age1m < 0 || p->age1m >= 12) {
				sprintf(err_mess, "Illegal month number: %d, please choose 0 - 11", p->age1m);
				do_warning_sheet(err_mess, [self window]);
				return(false);
			}
			if (p->age1y == -1) {
				sprintf(err_mess, "Please specify a year for age");
				do_warning_sheet(err_mess, [self window]);
				return(false);
			}
		}
		if (p->age1d != -1) {
			if (p->age1d < 0 || p->age1d > 31) {
				sprintf(err_mess, "Illegal date number: %d, please choose 1 - 31", p->age1d);
				do_warning_sheet(err_mess, [self window]);
				return(false);
			}
			if (p->age1y == -1) {
				sprintf(err_mess, "Please specify a year for age");
				do_warning_sheet(err_mess, [self window]);
				return(false);
			}
			if (p->age1m == -1) {
				sprintf(err_mess, "Please specify a month for age");
				do_warning_sheet(err_mess, [self window]);
				return(false);
			}
		}
	}
	return(true);
}

- (void)windowDidLoad {

	NSWindow *window = [self window];
	OnlyIntegerValueFormatter *formatter = [[[OnlyIntegerValueFormatter alloc] init] autorelease];

//	[window setIdentifier:@"Ids"];
	[window setRestorationClass:[self class]];
	[super windowDidLoad];  // It's documented to do nothing, but still a good idea to invoke...
/*
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{//TODO
		[[NSApplication sharedApplication] runModalForWindow:self.window];
	});
*/
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];

	[ageYearField setFormatter:formatter];
	[ageMonthField setFormatter:formatter];
	[ageDateField setFormatter:formatter];

	spItem = 0;
	[self createDepfilePopupMenus];
	[self createUserPopupMenus];
	[self fillInIDFields];
}

- (IBAction)usersPopUpClicked:(id)sender
{
#pragma unused (sender)
	NSInteger item;

	NSLog(@"IdsController: usersPopUpClicked\n");
	item = [usersPopUp indexOfSelectedItem];
	if (item >= 0 && item != spItem) {
		if ([self saveIDFields:spItem]) {
			spItem = item;
			[self fillInIDFields];
		}
		[usersPopUp selectItemAtIndex:spItem];
	}
}

- (IBAction)deleteButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSInteger i;

	NSLog(@"IdsController: deleteButtonClicked\n");

	i = [usersPopUp numberOfItems];
	if (i > 0) {
		rootIDs = deleteID(rootIDs, spItem);
		spItem = 0;
		[self createUserPopupMenus];
		[self fillInIDFields];
	} else {
		[self saveIDFields:spItem];
	}
}

- (IBAction)createButtonClicked:(id)sender;
{
#pragma unused (sender)

	NSLog(@"IdsController: createButtonClicked\n");
	if ([self saveIDFields:spItem]) {
		rootIDs = createNewId(rootIDs, 0);
		spItem = 0;
		[self createUserPopupMenus];
		[self fillInIDFields];
	}
}

- (IBAction)copyButtonClicked:(id)sender;
{
#pragma unused (sender)

	NSLog(@"IdsController: copyButtonClicked\n");
	if ([self saveIDFields:spItem]) {
		rootIDs = createNewId(rootIDs, spItem);
		spItem = 0;
		[self createUserPopupMenus];
		[self fillInIDFields];
	}
}

- (IBAction)langFieldChanged:(id)sender {
	NSLog(@"IdsController: langFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)corpusFieldChanged:(id)sender {
	NSLog(@"IdsController: corpusFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)codeFieldChanged:(id)sender {
	NSLog(@"IdsController: codeFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)ageYearFieldChanged:(id)sender {
	NSLog(@"IdsController: ageYearFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)ageMonthFieldChanged:(id)sender {
	NSLog(@"IdsController: ageMonthFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)ageDateFieldChanged:(id)sender {
	NSLog(@"IdsController: ageDateFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}


- (IBAction)unknownClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"IdsController: unknownClicked\n");
	maleButton.state = NSControlStateValueOff;
	femaleButton.state = NSControlStateValueOff;
	unknownButton.state = NSControlStateValueOn;
}

- (IBAction)maleClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"IdsController: maleClicked\n");
	maleButton.state = NSControlStateValueOn;
	femaleButton.state = NSControlStateValueOff;
	unknownButton.state = NSControlStateValueOff;
}

- (IBAction)femaleClicked:(NSButton *)sender
{
#pragma unused (sender)
	NSLog(@"IdsController: femaleClicked\n");
	maleButton.state = NSControlStateValueOff;
	femaleButton.state = NSControlStateValueOn;
	unknownButton.state = NSControlStateValueOff;
}


- (IBAction)groupFieldChanged:(id)sender {
	NSLog(@"IdsController: groupFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}


- (IBAction)racePopUpClicked:(id)sender
{
#pragma unused (sender)
	NSInteger len;
	NSInteger menuPos;
	NSString *menuStr;

	NSLog(@"IdsController: racePopUpClicked\n");
	menuPos = [sender indexOfSelectedItem];
	if (menuPos == 0)
		return;
	len = [usersPopUp numberOfItems] - 1;
	while (len >= menuPos) {
		menuStr = [usersPopUp itemTitleAtIndex:len];
		//		strcat(curPath, [menuStr UTF8String]);
		len--;
	}
}

- (IBAction)sesPopUpClicked:(id)sender
{
#pragma unused (sender)
	NSInteger len;
	NSInteger menuPos;
	NSString *menuStr;

	NSLog(@"IdsController: sesPopUpClicked\n");
	menuPos = [sender indexOfSelectedItem];
	if (menuPos == 0)
		return;
	len = [usersPopUp numberOfItems] - 1;
	while (len >= menuPos) {
		menuStr = [usersPopUp itemTitleAtIndex:len];
		//		strcat(curPath, [menuStr UTF8String]);
		len--;
	}
}

- (IBAction)rolePopUpClicked:(id)sender
{
#pragma unused (sender)
	NSInteger len;
	NSInteger menuPos;
	NSString *menuStr;

	NSLog(@"IdsController: rolePopUpClicked\n");
	menuPos = [sender indexOfSelectedItem];
	if (menuPos == 0)
		return;
	len = [usersPopUp numberOfItems] - 1;
	while (len >= menuPos) {
		menuStr = [usersPopUp itemTitleAtIndex:len];
		//		strcat(curPath, [menuStr UTF8String]);
		len--;
	}
}


- (IBAction)educationFieldChanged:(id)sender {
	NSLog(@"IdsController: educationFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)customFieldChanged:(id)sender {
	NSLog(@"IdsController: customFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}

- (IBAction)optionalFieldChanged:(id)sender {
	NSLog(@"IdsController: optionalFieldChanged\n");
	if ([@"" isEqual:[sender stringValue]])
		return;
}


- (void)controlTextDidChange:(NSNotification *)notification
{
	BOOL isChange;
	int i;
	unichar st[BUFSIZ];
	NSInteger item;
	NSUInteger len;
	NSString *comStr;
	NSLog(@"IdsController: controlTextDidChange\n");

	if ( [ notification object ] == langField ) {
		comStr = [langField stringValue];
		len = [comStr length];
		if (len < BUFSIZ) {
			[comStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			isChange = false;
			for (i=0; i < len; i++) {
				if (isalpha(st[i])) {
					st[i] = tolower(st[i]);
					isChange = true;
				} else if (st[i] == ',' || st[i] == 0xfffe || st[i] == 0xfeff || st[i] == 0x7f || st[i] == 0x8 || st[i] == 0x9 ||
						   st[i] == 0x1b || st[i] == 0x1c || st[i] == 0x1d || st[i] == 0x1e || st[i] == 0x1f) {
				} else {
					strcpy(st+i, st+i+1);
					isChange = true;
					len--;
				}
			}
			if (isChange)
				[langField setStringValue:[NSString stringWithCharacters:st length:strlen(st)]];
		}
	} else if ( [ notification object ] == corpusField ) {
		comStr = [corpusField stringValue];
	} else if ( [ notification object ] == codeField ) {
		comStr = [codeField stringValue];
		len = [comStr length];
		if (len < BUFSIZ) {
			[comStr getCharacters:st range:NSMakeRange(0, len)];
			st[len] = EOS;
			isChange = false;
			for (i=0; i < len; i++) {
				if (st[i] == '*' || st[i] == ':') {
					strcpy(st+i, st+i+1);
					isChange = true;
					len--;
					do_warning_sheet("Please do not include either '*' or ':' character in code", [self window]);
				} else if (isalpha(st[i])) {
					st[i] = toupper(st[i]);
					isChange = true;
				} else if (isdigit(st[i]) || st[i] == '-' || st[i] == 0xfffe || st[i] == 0xfeff || st[i] == 0x7f || st[i] == 0x8 ||
						   st[i] == 0x9 || st[i] == 0x1b || st[i] == 0x1c || st[i] == 0x1d || st[i] == 0x1e || st[i] == 0x1f) {
				} else {
					strcpy(st+i, st+i+1);
					isChange = true;
					len--;
				}
			}
			if (isChange){
				comStr = [NSString stringWithCharacters:st length:strlen(st)];
				[codeField setStringValue:comStr];
				item = [usersPopUp indexOfSelectedItem];
				[usersPopUp insertItemWithTitle:comStr atIndex:item];
				[usersPopUp removeItemAtIndex:item+1];
				[usersPopUp selectItemAtIndex:item];
				if (spItem != item)
					spItem = item;
			}
		}
	} else if ( [ notification object ] == ageYearField ) {
		comStr = [ageYearField stringValue];
	} else if ( [ notification object ] == ageMonthField ) {
		comStr = [ageMonthField stringValue];
	} else if ( [ notification object ] == ageDateField ) {
		comStr = [ageDateField stringValue];
	} else if ( [ notification object ] == groupField ) {
		comStr = [groupField stringValue];
	} else if ( [ notification object ] == educationField ) {
		comStr = [educationField stringValue];
	} else if ( [ notification object ] == customField ) {
		comStr = [customField stringValue];
	} else if ( [ notification object ] == optionalField ) {
		comStr = [optionalField stringValue];
	} else {
	}

//	strcpy(spareTier3, [comStr UTF8String]);
/*
	int i;
	unichar *bufU;
	NSRange aRange;
	NSText *fieldEditor = [[notification userInfo] objectForKey: @"NSFieldEditor"];
	if (!fieldEditor)
		return;
	NSString *oldString = [fieldEditor string];
	NSRange selectedRange = [fieldEditor selectedRange];
	NSString *newString;

	i = [oldString length];
	bufU = (unichar *)malloc((i*sizeof(unichar))+1);
	if (bufU == NULL)
		return;
	aRange.location = 0;
	aRange.length = i;
	[oldString getCharacters:bufU range:aRange];
	bufU[i] = 0;
*/
}


- (IBAction)cancelButtonClicked:(id)sender;
{
#pragma unused (sender)
	NSLog(@"IdsController: cancelButtonClicked\n");

	[[self window] close];
}

- (IBAction)doneButtonClicked:(id)sender;
{
#pragma unused (sender)
	int i;
	BOOL res;
	NSInteger item, cItem;
	IDSTYPE *IDs;

	NSLog(@"IdsController: doneButtonClicked\n");

	res = [self saveIDFields:spItem];
	if (res == true) {
		cItem = spItem;
		item = 0;
		for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
			if (item != cItem) {
				spItem = item;
				[self fillInIDFields];
				[usersPopUp selectItemAtIndex:spItem];
				res = [self saveIDFields:spItem];
				if (res == false)
					break;
			}
			item++;
		}
	}
	if (res == true) {
		[self updateHeadersInText];
		for (i=0; i < SPEAKERNAMENUM; i++)
			[docWinController AllocSpeakerNames:cl_T("") index:i];
		[docWinController SetUpParticipants:false];
		[[self window] close];
	}
}

/* Reopen the line panel when the app's persistent state is restored.

+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler {
	//    completionHandler([[(Controller *)[NSApp delegate] listBoxController] window], NULL);
}
*/

- (void)windowDidResize:(NSNotification *)notification {// 2020-01-29
#pragma unused (notification)
	NSLog(@"IdsController: windowDidResize\n");
}

- (void)windowWillClose:(NSNotification *)notification {
#pragma unused (notification)

	NSLog(@"IdsController: windowWillClose\n");
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:self.window];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResizeNotification object:self.window];

	clean_ids(rootIDs);
	clean_roles(depfileRoles);
	clean_SESs(depfileSESe);
	clean_SESs(depfileSESs);

//	[[NSApplication sharedApplication] stopModal];
	[[docWinController window] endSheet:[self window]];

	[IdsWindow release];
	IdsWindow = nil;
}

static void replaceSpaceWithUnderline(unCH *s) {
	for (; *s != EOS; s++) {
		if (*s == ' ')
			*s = '_';
	}
}

- (void)updateHeadersInText {
	char numS[CODESIZE];
	unCH languages[IDFIELSSIZE+1];
	IDSTYPE *IDs;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSUInteger len, bPos, ePos, lBeg, lEnd;
	NSRange endRange;
	extern long MAXOUTCOL;

	languages[0] = EOS;
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		if (IDs->language[0] != EOS) {
			add_to_languages(languages, IDs->language, TRUE);
		}
	}

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];

//	[text beginEditing];
//	[text endEditing];
	bPos = 0;

	ePos = getUtts(bPos, len, textSt);
	if (uS.partcmp(sp, "@Begin", FALSE, FALSE)) {
		bPos = ePos;
		ePos = getUtts(bPos, len, textSt);
	} else {
		strcpy(templineW, "@Begin\n");
		id_AddText(textView, text, NSMakeRange(bPos, 0), templineW);
		textSt = [text string];
		len = [text length];
		bPos += strlen(templineW);
		ePos = getUtts(bPos, len, textSt);
	}

	if (uS.partcmp(sp, "@Languages:", FALSE, FALSE)) {
		templineW[0] = EOS;
		endRange = NSMakeRange(bPos, ePos-bPos);
		id_AddText(textView, text, endRange, templineW);
		textSt = [text string];
		len = [text length];
		ePos = getUtts(bPos, len, textSt);
	}
	if (languages[0] == EOS)
		strcpy(languages, "change_me_later");
	strcpy(templineW, "@Languages:	");
	strcat(templineW, languages);
	strcat(templineW, "\n");
	id_AddText(textView, text, NSMakeRange(bPos, 0), templineW);
	textSt = [text string];
	len = [text length];
	bPos += strlen(templineW);
	ePos = getUtts(bPos, len, textSt);

	if (uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE)) {
		templineW[0] = EOS;
		endRange = NSMakeRange(bPos, ePos-bPos);
		id_AddText(textView, text, endRange, templineW);
		textSt = [text string];
		len = [text length];
		ePos = getUtts(bPos, len, textSt);
	}
	lBeg = 0;
	strcpy(templineW, "@Participants:	");
	for (IDs=rootIDs; IDs != NULL; IDs=IDs->next_id) {
		if (IDs->code[0] != EOS) {
			if (IDs != rootIDs) {
				lEnd = strlen(templineW);
				if (lEnd-lBeg >= MAXOUTCOL) {
					lBeg = lEnd;
					strcat(templineW, ",\n\t");
				} else {
					strcat(templineW, ", ");
				}
			}
			strcat(templineW, IDs->code);
			if (IDs->spname[0] != EOS) {
				lEnd = strlen(templineW);
				if (lEnd-lBeg >= MAXOUTCOL) {
					lBeg = lEnd;
					strcat(templineW, "\n\t");
				} else {
					strcat(templineW, " ");
				}
				strcat(templineW, IDs->spname);
			}
			lEnd = strlen(templineW);
			if (lEnd-lBeg >= MAXOUTCOL) {
				lBeg = lEnd;
				strcat(templineW, "\n\t");
			} else {
				strcat(templineW, " ");
			}
			strcat(templineW, IDs->role);
		}
	}
	strcat(templineW, "\n");
	id_AddText(textView, text, NSMakeRange(bPos, 0), templineW);
	textSt = [text string];
	len = [text length];
	bPos += strlen(templineW);
	ePos = getUtts(bPos, len, textSt);


	while (1) {
		if (uS.partcmp(sp, IDOF, FALSE, FALSE)) {
			templineW[0] = EOS;
			endRange = NSMakeRange(bPos, ePos-bPos);
			id_AddText(textView, text, endRange, templineW);
			textSt = [text string];
			len = [text length];
			ePos = getUtts(bPos, len, textSt);
		} else
			break;
	}

	if (uS.partcmp(sp, "@Options:", FALSE, FALSE)) {
		bPos = ePos;
		ePos = getUtts(bPos, len, textSt);
	}

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
			uS.sprintf(numS, "%d", IDs->age1y);
			strcat(templineW, numS);
			strcat(templineW, ";");
		}
		if (IDs->age1m != -1) {
			if (IDs->age1m < 10)
				strcpy(numS, "0");
			else
				numS[0] = EOS;
			uS.sprintf(numS+strlen(numS), "%d", IDs->age1m);
			strcat(templineW, numS);
			strcat(templineW, ".");
		}
		if (IDs->age1d != -1) {
			if (IDs->age1d < 10)
				strcpy(numS, "0");
			else
				numS[0] = EOS;
			uS.sprintf(numS+strlen(numS), "%d", IDs->age1d);
			strcat(templineW, numS);
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
		strcat(templineW, "\n");
		id_AddText(textView, text, NSMakeRange(bPos, 0), templineW);
		textSt = [text string];
		len = [text length];
		bPos += strlen(templineW);
		ePos = getUtts(bPos, len, textSt);
	}

	while (sp[0] != EOS) {
		if (uS.partcmp(sp, "@Languages:", FALSE, FALSE) ||
			uS.partcmp(sp, PARTICIPANTS, FALSE, FALSE) ||
			uS.partcmp(sp, IDOF, FALSE, FALSE) ||
//			uS.partcmp(sp, "@Language of ", FALSE, FALSE) ||
			uS.partcmp(sp, "@Age of ", FALSE, FALSE) ||
			uS.partcmp(sp, "@Sex of ", FALSE, FALSE) ||
			uS.partcmp(sp, "@Group of ", FALSE, FALSE) ||
			uS.partcmp(sp, "@Ses of ", FALSE, FALSE) ||
			uS.partcmp(sp, "@Education of ", FALSE, FALSE)) {
			templineW[0] = EOS;
			endRange = NSMakeRange(bPos, ePos-bPos);
			id_AddText(textView, text, endRange, templineW);
			textSt = [text string];
			len = [text length];
			ePos = getUtts(bPos, len, textSt);
		} else {
			bPos = ePos;
			ePos = getUtts(bPos, len, textSt);
		}

		if (isMainSpeaker(sp[0]))
			break;
	}
}

@end

@implementation IdsController(Delegation)


@end
