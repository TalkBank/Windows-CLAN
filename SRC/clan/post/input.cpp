/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* input.cpp
	- input from different types of data:
	- vertical texts.
	- raw texts.
	- CLAN & MOR files.
*/

#include "system.hpp"

#include "database.hpp"

#define sizeOfLine 1024

char rightSpTier = 0;
char isReportConllError = FALSE;

static struct ConllTable *conllRoot = NULL;

#if defined(UNX)

#include <ctype.h>

#ifndef EOS			/* End Of String marker			 */
	#define EOS '\0'
#endif
#define SPEAKERLEN 1024		 /* max len of the code part of the turn */
#define UTTLINELEN 18000L		 /* max len of the line part of the turn */

#define UTF8_IS_SINGLE(uchar) (((uchar)&0x80)==0)
#define UTF8_IS_LEAD(uchar) ((unsigned char)((uchar)-0xc0)<0x3e)
#define UTF8_IS_TRAIL(uchar) (((uchar)&0xc0)==0x80)

char templineC [UTTLINELEN+2];	/* used to be: templine  */
char templineC1[UTTLINELEN+2];	/* used to be: templine1 */
char templineC2[UTTLINELEN+2];	/* used to be: templine2 */

static void out_of_mem(void) {
	fputs("ERROR: Out of memory.\n",stderr);
	exit(1);
}

void remFrontAndBackBlanks(char *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n'; i++) ;
	if (i > 0)
		strcpy(st, st+i);
	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '\r'))
		i--;
	st[i+1] = EOS;
}

#endif

#ifdef POST_UNICODE
static char* fgets_unicode( char* buffer, int sz, FILE* f )
{
	int c;
	for ( int nr = 0; nr < sz-1 ; nr++ ) {
		//		if ( fread(buffer+nr,1,1,f) != 1 ) {
		if ( (buffer[nr] = fgetc(f)) == EOF ) {
			if (nr==0) return NULL;
			buffer[nr] = '\0';
			return buffer;
		}
		if ( buffer[nr] == 0xD || buffer[nr] == 0xA ) { /* end of line */
			c = fgetc_cr(f);
			if ( c != 0xD && c != 0xA ) ungetc(c,f);
			buffer[nr+1] = '\0';
			return buffer;
		}
	}
	buffer[nr] = '\0';
	return buffer;
}
#endif

#ifdef POSTCODE

static char post_sp[SPEAKERLEN];

static void extractSpeaker(char *post_sp, char *tier) {
	int i;

	for (i=0; i < SPEAKERLEN-2 && tier[i] != ':'; i++) ;
	if (tier[i] == ':') {
		i++;
	}
	strncpy(post_sp, tier, i);
	post_sp[i] = '\0';
}

#else

// #define _stricmp mStricmp
// #define _strnicmp mStrnicmp

int mStrnicmp(const char *st1, const char *st2, size_t len) {
	for (; _toupper((unsigned char)*st1) == _toupper((unsigned char)*st2) && *st2 != '\0' && len > 0L; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0L)
		return(0);
	else if (_toupper((unsigned char)*st1) > _toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

int mStricmp(const char *st1, const char *st2) {
	for (; _toupper((unsigned char)*st1) == _toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (_toupper((unsigned char)*st1) > _toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

static void sp_mod(char *s1, char *s2) {
	if (s1 != s2) {
		while (s1 != s2)
			*s1++ = '\001';
	}
}

static int patmat(char *s, const char *pat) {
	register int j, k;
	int n, m, t, l;
	char *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == '_') {
			if (isspace(s[j]))
				return(FALSE);
			if (s[j] == EOS)
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; // any character
			else {
				if (s[j+1] == EOS && pat[k+1] == '*' && pat[k+2] == EOS)
					return(TRUE);
				else if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {	// wildcard
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else {
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS) {
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (isspace(s[m]) || s[m] == EOS) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					if (pat[n+1] == '%' && pat[n+2] == '%')
						break;
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		} else if (pat[k] == '%') {		  // wildcard
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\') k++;
				if ((t=j - 1) < 0) t = 0;
			} else {
				if (pat[k] == '\\') k++;
				t = j;
			}
f2:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					sp_mod(s+t,s+j);
					return(TRUE);
				} else {
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS) {
						sp_mod(s+t,s+j);
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*' || pat[n] == '%') break;
				else if (pat[n] == '_') {
					if (isspace(s[m])) {
						j++;
						goto f2;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n]) return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f2;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f2;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f2;
			}
			sp_mod(s+t,s+j);
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			} else
				return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) {
			while (lf != s)
				*lf++ = ' ';
		}
		return(TRUE);
	} else
		return(FALSE);
}

static char isUTF8(char *str) {
	if (str[0] == (char)0xef && str[1] == (char)0xbb && str[2] == (char)0xbf && 
		str[3] == '@' && str[4] == 'U' && str[5] == 'T' && str[6] == 'F' && str[7] == '8')
		return(TRUE);
	else if (str[0] == '@' && str[1] == 'U' && str[2] == 'T' && str[3] == 'F' && str[4] == '8')
		return(TRUE);
	else
		return(FALSE);
}

static int partcmp(char *st1, const char *st2, char pat_match, char isCaseSenc) { // st1- full, st2- part
	if (pat_match) {
		int i;
		if (*st1 == *st2) {
			for (i=strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			if (i < 0)
				i++;
			if (st1[i] == ':')
				st1[i] = EOS;
			else
				st1[i+1] = EOS;
			return(patmat(st1+1, st2+1));
		} else
			return(FALSE);
	}
	if (isCaseSenc) {
		for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	} else {
		for (; toupper((unsigned char)*st1) == toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	}
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}

#endif

static struct ConllTable *freeConllList(struct ConllTable *p) {
	struct ConllTable *t;
	
	while (p) {
		t = p;
		p = p->next;
		if (t->from != NULL)
			free(t->from);
		if (t->to != NULL)
			free(t->to);
		free(t);
	}
	return(NULL);
}

static struct ConllTable *addToConll(struct ConllTable *root, char *from, char *to) {
	struct ConllTable *t;
	
	if (root == NULL) {
		root = NEW(struct ConllTable);
		t = root;
	} else {
		for (t=root; t->next != NULL; t=t->next) ;
		t->next = NEW(struct ConllTable);
		t = t->next;
	}
	if (t == NULL) {
		out_of_mem();
	}
	t->next = NULL;
	if ((t->from=(char *)malloc(strlen(from)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(t->from, from);
	if ((t->to=(char *)malloc(strlen(to)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(t->to, to);
	return(root);
}

void freeConll(void) {
	conllRoot = freeConllList(conllRoot);
}

char isConllSpecified(void) {
	if (conllRoot != NULL)
		return(TRUE);
	else
		return(FALSE);
}

void readConllFile(FNType *dbname) {
	char *to;
	FILE *fp;
	FNType conllFN[128];
	FNType mFileName[FNSize];

	conllRoot = NULL;
#ifdef POSTCODE
	strcpy(conllFN, "conll.cut");
	if ((fp=OpenMorLib(conllFN, "r", TRUE, FALSE, mFileName)) == NULL) {
		fprintf(stderr, "    NOT Using conll file\n");
		return;
	}
#else
	strcpy(mFileName, dbname);
	to = strrchr(mFileName, '/');
	if (to != NULL) {
		*(to+1) = EOS;
		strcat(mFileName, "conll.cut");
	} else {
		to = strrchr(mFileName, '\\');
		if (to != NULL) {
			*(to+1) = EOS;
			strcat(mFileName, "conll.cut");
		} else {
			fprintf(stderr, "    NOT Using conll file\n");
			return;
		}
	}
	if ((fp=fopen(mFileName, "r")) == NULL) {
		fprintf(stderr, "    NOT Using conll file: %s\n", mFileName);
		return;
	}
#endif
	fprintf(stderr, "    Using conll file:      %s.\n", mFileName);
	while (fgets(templineC, UTTLINELEN, fp)) {
#ifdef POSTCODE
		uS.remFrontAndBackBlanks(templineC);
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
#else
		remFrontAndBackBlanks(templineC);
		if (isUTF8(templineC) || partcmp(templineC,FONTHEADER,0,0) || partcmp(templineC,WINDOWSINFO,0,0) ||
			partcmp(templineC,CKEYWORDHEADER,0,0))
#endif
			continue;
		if (templineC[0] == '%' || templineC[0] == '#' || templineC[0] == EOS)
			continue;
		for (to=templineC; !isSpace(*to) && *to != EOS; to++) ;
		if (*to != EOS) {
			*to = EOS;
			for (to++; isSpace(*to); to++) ;
			if (*to != EOS) {
				conllRoot = addToConll(conllRoot, templineC, to);
			}
		}
	}
	fclose(fp);
}

void init_conll(void) {
	isReportConllError = TRUE;
	conllRoot = NULL;
}

void freeConvertTier(struct SimpleTier *p) {
	if (p == NULL)
		return;
	if (p->nextChoice != NULL)
		freeConvertTier(p->nextChoice);
	if (p->nextW != NULL)
		freeConvertTier(p->nextW);
	if (p->org != NULL)
		free(p->org);
	if (p->ow != NULL)
		free(p->ow);
	if (p->sw != NULL)
		free(p->sw);
	free(p);
}

static void getConllTranslation(char *elem, char *trans, FNType* fn, char *oItem) {
	char *s;
	struct ConllTable *p;

	if (strchr(elem, '|') ==  NULL)
		strcpy(trans, elem);
	else {
		for (p=conllRoot; p != NULL; p=p->next) {
#ifdef POSTCODE
			if (uS.patmat(elem, p->from))
#else
			if (patmat(elem, p->from))
#endif
										  {
				strcpy(trans, p->to);
				if (strcmp(p->from, "*") == 0) {
					if (isReportConllError) {
						fprintf(stderr, "*** ERROR 1: In file \"%s\"\n", fn);
						fprintf(stderr, "  in item:    %s\n", oItem);
						fprintf(stderr, "  Can't find conversion for: %s\n", elem);
					}
				}
				return;
			}
		}
		strcpy(trans, elem);
		s = strchr(trans, '#');
		if (s != NULL)
			strcpy(trans, s+1);
		s = strchr(trans, '|');
		if (s != NULL)
			*s = EOS;
		if (isReportConllError) {
			fprintf(stderr, "*** ERROR 1: In file \"%s\"\n", fn);
			fprintf(stderr, "  in item:    %s\n", oItem);
			fprintf(stderr, "  Can't find conversion for: %s\n", elem);
		}
	}
}

static void initElem(struct SimpleTier *t, char *elem, char isClitic, FNType* fn, char *line, char *oItem) {
	char trans[BUFSIZ], *s;

	if (t == NULL) {
		out_of_mem();
	}
	t->nextW = NULL;
	t->nextChoice = NULL;
	t->org = NULL;
	t->ow = NULL;
	t->sw = NULL;
	t->isCliticChr = isClitic;
	if (isClitic)
		t->org = NULL;
	else {
		if ((t->org=(char *)malloc(strlen(oItem)+1)) == NULL) {
			out_of_mem();
		}
		strcpy(t->org, oItem);
	}
	if ((t->ow=(char *)malloc(strlen(elem)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(t->ow, elem);
	s = elem;
	while ((s=strchr(s, '#')) != NULL) {
		strcpy(elem, s+1);
		s = elem;
	}
	getConllTranslation(elem, trans, fn, oItem);
	if ((t->sw=(char *)malloc(strlen(trans)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(t->sw, trans);
}

SimpleTier *convertTier(char *T, FNType* fn, char *isConllError) {
	int i, tierNameEnd, c, cCnt, maxcCnt;
	char oItem[BUFSIZ], item[BUFSIZ], *si, *sc, *alt, *clt, isClitic, b, e;
	struct SimpleTier *convMorT, *t, *sItem, *tc;

	for (tierNameEnd=0; T[tierNameEnd] != ':' && T[tierNameEnd] != EOS; tierNameEnd++) ;
	if (T[tierNameEnd] == EOS)
		return(NULL);
	for (tierNameEnd++; isSpace(T[tierNameEnd]); tierNameEnd++) ;
	if (T[tierNameEnd] == EOS)
		return(NULL);
	convMorT = NULL;
	strcpy(templineC, T+tierNameEnd);
	i = 0;
	while ((i=getNextMorItem(templineC, oItem, NULL, i))) {
#ifdef POSTCODE
		uS.remFrontAndBackBlanks(oItem);
#else
		remFrontAndBackBlanks(oItem);
#endif
		if (oItem[0] != EOS) {
			strcpy(item, oItem);
			sItem = NULL;
			maxcCnt = 0;
			si = item;
			do {
				alt = strchr(si, '^');
				if (alt != NULL)
					*alt = EOS;
				cCnt = 0;
				sc = si;
				b = '\0';
				while (1) {
					for (clt=sc; *clt != EOS && *clt != '$' && *clt != '~'; clt++) ;
					e = *clt;
					if (e != EOS)
						*clt = EOS;
					if (b == '~')
						isClitic = '~';
					else if (e == '$')
						isClitic = '$';
					else
						isClitic = '\0';
					if (convMorT == NULL) {
						convMorT = NEW(struct SimpleTier);
						initElem(convMorT, sc, isClitic, fn, templineC, oItem);
						t = convMorT;
					} else {
						if (sItem == NULL) {
							for (t=convMorT; t->nextW != NULL; t=t->nextW) ;
							t->nextW = NEW(struct SimpleTier);
							t = t->nextW;
							initElem(t, sc, isClitic, fn, templineC, oItem);
						} else {
							if (si != item && cCnt > maxcCnt) {
								fprintf(stderr, "*** ERROR 2: In file \"%s\"\n", fn);
								fprintf(stderr, "on line:    %s", templineC);
								fprintf(stderr, "in item:    %s\n", oItem);
								fprintf(stderr, "Some alternatives have more clitic elements than others\n");
								*isConllError = TRUE;
								freeConvertTier(convMorT);
								return(NULL);
							}
							c = cCnt;
							for (t=sItem; t->nextW != NULL; t=t->nextW) {
								if (c <= 0)
									break;
								c--;
							}
							if (si == item) {
								t->nextW = NEW(struct SimpleTier);
								t = t->nextW;
								initElem(t, sc, isClitic, fn, templineC, oItem);
							} else if (c > 0) {
								fprintf(stderr, "*** ERROR 2: In file \"%s\"\n", fn);
								fprintf(stderr, "on line:    %s", templineC);
								fprintf(stderr, "in item:    %s\n", oItem);
								fprintf(stderr, "Some alternatives have more clitic elements than others\n");
								*isConllError = TRUE;
								freeConvertTier(convMorT);
								return(NULL);
							} else {
#ifdef POSTCODE
								for (; t->nextChoice != NULL; t=t->nextChoice) {
									if (uS.mStricmp(sc, t->ow) == 0)
										break;
								}
								if (uS.mStricmp(sc, t->ow) != 0) {
									t->nextChoice = NEW(struct SimpleTier);
									t = t->nextChoice;
									initElem(t, sc, isClitic, fn, templineC, oItem);
								}
#else
								for (; t->nextChoice != NULL; t=t->nextChoice) {
									if (mStricmp(sc, t->ow) == 0)
										break;
								}
								if (mStricmp(sc, t->ow) != 0) {
									t->nextChoice = NEW(struct SimpleTier);
									t = t->nextChoice;
									initElem(t, sc, isClitic, fn, templineC, oItem);
								}
#endif
							}
							
						}
					}
					if (sItem == NULL)
						sItem = t;
					if (e != EOS) {
						b = e;
						sc = clt + 1;
						cCnt++;
					} else
						break;
				}
				if (si == item)
					maxcCnt = cCnt;
				else if (maxcCnt != cCnt) {
					fprintf(stderr, "*** ERROR 2: In file \"%s\"\n", fn);
					fprintf(stderr, "on line:    %s", templineC);
					fprintf(stderr, "in item:    %s\n", oItem);
					fprintf(stderr, "Some alternatives have more clitic elements than others\n");
					*isConllError = TRUE;
					freeConvertTier(convMorT);
					return(NULL);
				}
				if (alt != NULL)
					si = alt + 1;
			} while (alt != NULL) ;
			struct SimpleTier *tct;
			for (t=sItem; t != NULL; t=t->nextW) {
				for (tc=t; tc != NULL; tc=tc->nextChoice) {
					for (tct=tc->nextChoice; tct != NULL; tct=tct->nextChoice) {
						if (strcmp(tc->sw, tct->sw) == 0) {
							fprintf(stderr, "*** ERROR 3: In file \"%s\"\n", fn);
//							fprintf(stderr, "on line:    %s", templineC);
							fprintf(stderr, "in item:    %s\n", oItem);
							fprintf(stderr, "Both %%mor items \"%s\" and \"%s\" result in \"%s\"\n", tc->ow, tct->ow, tc->sw);
//							*isConllError = TRUE;
//							freeConvertTier(convMorT);
//							return(NULL);
						}
					}
				}
			}
		}
	}
	T[tierNameEnd] = EOS;
	for (t=convMorT; t != NULL; t=t->nextW) {
		for (tc=t; tc != NULL; tc=tc->nextChoice) {
			strcat(T, tc->sw);
			if (tc->nextChoice != NULL)
				strcat(T, "^");
		}
		if (t->nextW != NULL)
			strcat(T, " ");
	}
	strcat(T, "\n");
	return(convMorT);
}

static void printChars(char *st, char needCR, FILE *fp) {
	char lastCR = FALSE;

	while (*st) {
		if (*st == '\n')
			lastCR = TRUE;
		else
			lastCR = FALSE;
		putc(*st, fp);
		st++;
	}
	if (!lastCR && needCR) {
		putc('\n',fp);
	}
}

static char isBreakChar(char c) {
	if (c == ' ' || c == '\t' || c == '\n' || c == '\b' || c == '\r')
		return(TRUE);
	else
		return(FALSE);
}

void printclean(FILE *out, const char *spO, char *line) {
	int  i, j, TabSize;
	char sp[SPEAKERLEN];
	long32 colnum;
	long32 splen = 0;
	char *pos, *tpos, *s, first = TRUE, sb = FALSE, isLineSpace;

	TabSize = 8;
	strcpy(templineC, line);
	line = templineC;
	if (spO == NULL) {
		j = 0;
		for (i=0; templineC[i] != ':' && templineC[i] != EOS; i++)
			sp[j++] = templineC[i];
		if (templineC[i] == ':') {
			sp[j++] = templineC[i];
			for (i++; isSpace(templineC[i]); i++) ;
			strcpy(templineC, templineC+i);
		}
		sp[j] = EOS;
	} else
		strcpy(sp, spO);
	for (colnum=0; sp[colnum] != EOS; colnum++) {
		if (sp[colnum] == '\t')
			splen = ((splen / TabSize) + 1) * TabSize;
		else
			splen++;
	}
	printChars(sp, FALSE, out);
	if (colnum != 0 && !isSpace(sp[colnum-1]) && line[0] != EOS && line[0] != '\n' && line[0] != '\r') {
		putc('\t',out);
		splen = ((splen / TabSize) + 1) * TabSize;
	}
	if (isSpace(*line))
		*line = ' ';
	colnum = splen;
	tpos = line;
	isLineSpace = *line;
	while (*line != EOS) {
		pos = line;
		colnum++;
		if (*line == '[')
			sb = TRUE;
		while ((!isBreakChar(*line) || sb) && *line != EOS) {
			line++;
			if (((*line == '\n' || *line == '\r') && sb) || *line == '\t') *line = ' ';
			else if (*line == '[') sb = TRUE;
			else if (*line == ']') sb = FALSE;
			if ((UTF8_IS_SINGLE((unsigned char)*line) || UTF8_IS_LEAD((unsigned char)*line)))
				colnum++;
		}
		isLineSpace = *line;
		if (*line != EOS) {
			*line++ = EOS;
		}
		if (colnum > 76L) {
			if (first)
				first = FALSE;
			else {
				while (isSpace(*tpos)) {
					tpos++;
				}
				if (*tpos == '\n' || *tpos == '\r') {
					tpos++;
				}
				putc('\n',out);
				while (isSpace(*tpos)) {
					tpos++;
				}
				putc('\t',out);
				colnum = splen;
				for (s=pos; *s != EOS; s++) {
					if ((UTF8_IS_SINGLE((unsigned char)*s) || UTF8_IS_LEAD((unsigned char)*s)))
						colnum++;
				}
			}
		} else if (!first) {
			while (isSpace(*tpos) || *tpos == '\n' || *tpos == '\r') {
				tpos++;
			}
			putc(' ',out);
		} else
			first = FALSE;
		if (pos != line)
			printChars(pos, FALSE, out);
		if (isLineSpace != EOS) {
			line--;
			*line = isLineSpace;
		}
		tpos = line;
		while (isSpace(*line) || *line == '\n' || *line == '\r') {
			line++;
		}
	}
	putc('\n',out);
}

int getNextMorItem(char *line, char *oItem, int *morI, int i) {
	char *word;

	word = oItem;
	if (line[i] == EOS)
		return(0);
	while ((*word=line[i]) != EOS && isBreakChar(line[i]))
		i++;
	if (*word == EOS)
		return(0);
	if (morI != NULL)
		*morI = i - 1;
	while ((*++word=line[++i]) != EOS && line[i] != EOS && !isBreakChar(line[i])) {
	}
	*word = EOS;
	return(i);
}

#define nbIOs	12	/* max number of files */
/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/
struct inputObject {
	FILE* in;
	int   fmt;
	char* buffer;
	char  linebuffer[sizeOfLine];
} IOs[nbIOs];

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/
int	init_input( FNType* filename, const char* type )	// return 1 if OK, 0 if not
{
	int fmt;
	if ( !_stricmp( type, VTT ) )
		fmt = fVTT;
	else if ( !_stricmp( type, VTA ) )
		fmt = fVTA;
	else if ( !_stricmp( type, MOR ) )
		fmt = fMOR;
	else
		return -1;
#ifdef POST_UNICODE
	FILE* in = fopen( filename, "rb" );
#else
	FILE* in = fopen( filename, "r" );
#endif

	if (!in)
		return -1;

	// find a free IOs.
	for ( int id=0; id<nbIOs; id++ ) {
		if ( !IOs[id].in ) {
			IOs[id].in = in;
			IOs[id].fmt = fmt;
			IOs[id].buffer = new char [maxSizeOfInSent];
			if ( fmt==fMOR ) {
#ifdef POST_UNICODE
				if ( !fgets_unicode( IOs[id].linebuffer, sizeOfLine, IOs[id].in ) )
#else
				if ( !fgets( IOs[id].linebuffer, sizeOfLine, IOs[id].in ) )
#endif
					IOs[id].linebuffer[0] = '\0';
			} else
				IOs[id].linebuffer[0] = '\0';
			return id;
		}
	}
	return -1;
}

/*
int	get_input( int ID, char** &data )		// return number of words (or lines).
{
	if ( !IOs[ID].in ) return 0;
	
	data = (char**)IOs[ID].buffer;
	char* freedata = IOs[ID].buffer ;
	int pfreedata =  sizeof(char*) * maxWordsInSentence ;
	int  n = 0; // current word count.
	switch( IOs[ID].fmt ) {
		case fVTT:
		case fVTA:
			if ( IOs[ID].linebuffer[0] != '\0' ) {
				data[0] = freedata+pfreedata;
				strcpy( freedata+pfreedata, IOs[ID].linebuffer );
				pfreedata += strlen(IOs[ID].linebuffer) + 1;
				n = 1;
			} else {
				data[0] = freedata+pfreedata;
				strcpy( freedata+pfreedata, fVTT ? "." : ". pct" );
				pfreedata += strlen( ". pct")+1;
				n = 1;
			}
			while ( n<maxWordsInSentence ) {
				if ( feof(IOs[ID].in) ) return 0;
#ifdef POST_UNICODE
				if ( ! fgets_unicode( IOs[ID].linebuffer, sizeOfLine, IOs[ID].in ) )
#else
				if ( ! fgets( IOs[ID].linebuffer, sizeOfLine, IOs[ID].in ) )
#endif
					return n;
				int i = strlen(IOs[ID].linebuffer)-1;
				while (i>=0 && (IOs[ID].linebuffer[i] == '\n' || IOs[ID].linebuffer[i] == '\r')) i--;
				IOs[ID].linebuffer[i+1] = '\0';
				if (i>=0) {
					if ( pfreedata + strlen(IOs[ID].linebuffer) + 1 > maxSizeOfInSent )
						return n;
					data[n] = freedata+pfreedata;
					strcpy( freedata+pfreedata, IOs[ID].linebuffer );
					pfreedata += strlen(IOs[ID].linebuffer) + 1;
					n ++;
				}
				char w[MaxSizeOfString];
				sscanf( IOs[ID].linebuffer, "%s", w );
				if (  !strcmp( w, "." ) 
				   || !strcmp( w, "!" ) 
				   || !strcmp( w, "?" ) ) {
					IOs[ID].linebuffer[0] = '\0';
					return n;
				}
			}
			return n;
		default:
			return 0;
	}
	return n;
}
*/

void	close_input( int ID ) {
	if ( !IOs[ID].in ) return;
	delete [] IOs[ID].buffer;
	fclose( IOs[ID].in );
	IOs[ID].in = 0;
}

void strip_of_brackets( char* line )
{
	char *p1, *p2;
	do {
		p1 = strchr(line, '[');
		p2 = strchr(line, ']');
		if (p2 != NULL)
			p2 = p2 + 1;
		if (p1!=NULL && p2!=NULL && p1<p2) {
			line = p1;
			while (*p2 != '\0')
				*p1++ = *p2++;
			*p1 = '\0';
		}
	} while (p1!=NULL && p2!=NULL);
}

char* strip_of_one_bracket( char* line, char* tag, int mltag )
{
	char* p1 = strchr(line, '[');
	char* p2 = strchr(line, ']');
	*tag = '\0';
	if (p1==NULL || p2==NULL || p1>p2) return NULL;
	char* t = tag;
	char* p = p1;
	while ( p<=p2 ) {
		if (mltag>0) *t++ = *p++;
		mltag--;
	}
	*t = '\0';
	p2++;
	while (*p2 != '\0')
		*p1++ = *p2++;
	*p1 = '\0';
	return tag;
}

int get_type_of_tier( char* tier, char* name )
{
	switch( tier[0] ) {
		case '@':
			return TTgeneral;
		case '%':
			if ( !_strnicmp( &tier[1], "mor", 3 ) )
				return TTmor;
			if ( !_strnicmp( &tier[1], "xmor", 4 ) )
				return TTmor;
			if ( !_strnicmp( &tier[1], "trn", 3 ) )
				return TTtrn;
			if ( !_strnicmp( &tier[1], "wrd", 3 ) )
				return TTwrd;
			if ( !_strnicmp( &tier[1], "pos", 3 ) )
				return TTpos;
			if ( !_strnicmp( &tier[1], "cnl", 3 ) )
				return TTcnl;
			if ( !_strnicmp( &tier[1], "xcnl", 4 ) )
				return TTcnl;
			return TTdependant;
		case '*':
			if ( name ) {
				int lt = strlen( tier ) - 1 ;
				int ln = strlen( name );
				int lm = ln<lt ? ln : lt ; 
				if ( !_strnicmp( &tier[1], name, lm ) )
					return TTthisname;
			}
			return TTname;
		
		default:
			return TTunknown;
	}
	return TTunknown;
}

#ifdef POSTCODE
//2011-01-13
static char isRightPostCode(char *tier) {
	int i, j, k, res;
	char code[SPEAKERLEN];
	extern int  ScopNWds;
	extern char PostCodeMode;

	res = 0;
	for (i=0; tier[i] != EOS; i++) {
		if (tier[i] == '[' && (tier[i+1] == '-' || tier[i+1] == '+') && tier[i+2] == ' ') {
			k = 0;
			for (j=i+1; tier[j] != EOS && tier[j] != ']';) {
				if (isSpace(tier[j])) {
					code[k++] = ' ';
					for (j++; isSpace(tier[j]) && tier[j] != EOS; j++) ;
					if (tier[j] == EOS)
						break;
				} else
					code[k++] = tier[j++];
			}
			code[k] = EOS;
			uS.lowercasestr(code, &dFnt, MBF);
			res = isExcludePostcode(code);
			if (res == 4 && (PostCodeMode == 'i' || PostCodeMode == 'I'))
				return(TRUE);
			if (res == 5 && PostCodeMode == 'e')
				return(FALSE);
		}
	}
	if (ScopNWds > 1 && (PostCodeMode == 'i' || PostCodeMode == 'I'))
		return(FALSE);
	else
		return(TRUE);
}
#endif

// get a tier in a CHAT file. returns number of characters read. 0 if end of file.
int get_tier(int ID, char* &tier, int &typeoftier, long32 &ln)
{
	if ( !IOs[ID].in ) return 0;
	
	tier = (char*)IOs[ID].buffer;
	int  n = 0; // current char count.

	switch( IOs[ID].fmt ) {
		case fMOR:
			if ( IOs[ID].linebuffer[0] != '\0' ) {
				strcpy( tier, (char*)IOs[ID].linebuffer );
				n = strlen((char*)IOs[ID].linebuffer);
				IOs[ID].linebuffer[0] = '\0';
			} else {
				strcpy( tier, "" );
				n = 0;
			}
			if ( feof(IOs[ID].in) ) {
				if (n!=0) {
#ifdef POSTCODE
					extractSpeaker(post_sp, tier);
					if ((checktier(post_sp) && post_sp[0] == '*') || post_sp[0] == '@')
						rightSpTier = TRUE;
					else if (post_sp[0] == '*')
						rightSpTier = FALSE;
					if (rightSpTier && post_sp[0] == '*' && !isRightPostCode(tier))
						rightSpTier = FALSE;
					if (!rightSpTier)
						typeoftier = TTgeneral;
					else
#endif
						typeoftier = get_type_of_tier( tier );
				}
				return n;
			}
#ifdef POST_UNICODE
			while ( fgets_unicode( IOs[ID].linebuffer, sizeOfLine, IOs[ID].in ) ) {
#else
			while ( fgets( IOs[ID].linebuffer, sizeOfLine, IOs[ID].in ) ) {
#endif

// printf( "[%d][%c] {{{%s}}}\n", IOs[ID].linebuffer[0], IOs[ID].linebuffer[0], IOs[ID].linebuffer );
#ifdef POSTCODE
				if (!uS.isInvisibleHeader(IOs[ID].linebuffer) && !uS.isUTF8(IOs[ID].linebuffer))
#else
				if (!partcmp(IOs[ID].linebuffer, PIDHEADER, FALSE, TRUE) && !partcmp(IOs[ID].linebuffer, CKEYWORDHEADER, FALSE, TRUE) &&
					!partcmp(IOs[ID].linebuffer, WINDOWSINFO, FALSE, TRUE) && !partcmp(IOs[ID].linebuffer, FONTHEADER, FALSE, TRUE) &&
					!isUTF8(IOs[ID].linebuffer))
#endif
					ln++;

				if ( IOs[ID].linebuffer[0] == '@' || IOs[ID].linebuffer[0] == '*' || IOs[ID].linebuffer[0] == '%' ) {
#ifdef POSTCODE
					extractSpeaker(post_sp, tier);
					if ((checktier(post_sp) && post_sp[0] == '*') || post_sp[0] == '@')
						rightSpTier = TRUE;
					else if (post_sp[0] == '*')
						rightSpTier = FALSE;
					if (rightSpTier && post_sp[0] == '*' && !isRightPostCode(tier))
						rightSpTier = FALSE;
					if (!rightSpTier)
						typeoftier = TTgeneral;
					else
#endif
						typeoftier = get_type_of_tier( tier );
					return n;
				}
				strcat( tier, (char*)IOs[ID].linebuffer );
				n += strlen((char*)IOs[ID].linebuffer);
			}
			if (n!=0) {
#ifdef POSTCODE
				extractSpeaker(post_sp, tier);
				if ((checktier(post_sp) && post_sp[0] == '*') || post_sp[0] == '@')
					rightSpTier = TRUE;
				else if (post_sp[0] == '*')
					rightSpTier = FALSE;
				if (rightSpTier && post_sp[0] == '*' && !isRightPostCode(tier))
					rightSpTier = FALSE;
				if (!rightSpTier)
					typeoftier = TTgeneral;
				else
#endif
					typeoftier = get_type_of_tier( tier );
			}
			IOs[ID].linebuffer[0] = '\0';
			return n;
		default:
			return 0;
	}
	return n;
}

