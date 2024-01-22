/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 2

#include "cu.h"
#include <math.h>

#if !defined(UNX)
#define _main chains_main
#define call chains_call
#define getflag chains_getflag
#define init chains_init
#define usage chains_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

extern char tct;
extern struct tier *defheadtier;

#define CODECHAR '$'
#define CODES_ORDER_FILE "codes_ord.cut"
#define CHAIN_CLAUSES struct chains_speakers
#define CHAIN_SPEAKERS struct chains_speakers
#define CHAIN_UTTS  struct chains_utterances
#define CHAIN_CODES struct chains_codes

CHAIN_SPEAKERS {
	char *sp;
	CHAIN_SPEAKERS *nextsp;
} ;

CHAIN_CODES {
	char fn;
	char *code;
	char chain_break;
	char total_chain_break;
	float num_chains, all_sp_total_len, total_len, total_lensqr;
	unsigned int total_num_chains;
	unsigned int min, max, cur;
	CHAIN_UTTS *root_utt, *utt;
	CHAIN_CODES *nextcode;
} ;

CHAIN_UTTS {
	char spnum;
	char *st;
	unsigned int utt_line;
	unsigned int lineno;
	CHAIN_UTTS *nextutt;
} ;

static int  chains_CodeTierGiven;
static char chains_onlydata;
static char sec_code[SPEAKERLEN];
static unsigned int utt_line;
static unsigned int llineno;
static CHAIN_SPEAKERS *root_sp;
static CHAIN_CODES *root_code;
static CHAIN_CLAUSES *root_clause;

void usage() {
	puts("CHAINS .");
	printf("Usage: chains [cS dN %s] filename(s)\n", mainflgs());
	puts("+cS: look for clause marker S or markers listed in file @S");
	puts("+d : changes zeroes to spaces in output");
	puts("+d1: same as +d, plus displays every input line in output");
	mainusage(TRUE);
}

static void chains_free_sp(CHAIN_SPEAKERS *p) {
	CHAIN_SPEAKERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		free(t->sp);
		free(t);
	}
}

static void chains_free_utt(CHAIN_UTTS *p) {
	CHAIN_UTTS *t;

	while (p != NULL) {
		t = p;
		p = p->nextutt;
		if (t->st) free(t->st);
		free(t);
	}
}

static void chains_free_codes(CHAIN_CODES *p) {
	CHAIN_CODES *t;

	while (p != NULL) {
		t = p;
		p = p->nextcode;
		chains_free_utt(t->root_utt);
		free(t->code);
		free(t);
	}
}

static void chains_free_clause(CHAIN_SPEAKERS *p) {
	CHAIN_SPEAKERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		if (t->sp != NULL)
			free(t->sp);
		free(t);
	}
}

static void chains_exit(int i) {
	if (i != 1) {
		chains_free_clause(root_clause);
		chains_free_sp(root_sp);
		chains_free_codes(root_code);
	}
	cutt_exit(i);
}

static CHAIN_CODES *chains_create_code(char *code) {
	CHAIN_CODES *tc;

	if ((tc=NEW(CHAIN_CODES)) == NULL)
		out_of_mem();
	if ((tc->code=(char *)malloc(strlen(code)+1)) == NULL)
		out_of_mem();
	strcpy(tc->code,code);
	tc->chain_break = TRUE;
	tc->total_chain_break = TRUE;
	tc->num_chains  = (float)0.0;
	tc->total_len   = (float)0.0;
	tc->total_lensqr= (float)0.0;
	tc->all_sp_total_len = (float)0.0;
	tc->total_num_chains = (unsigned int)0;
	tc->min         = (unsigned int)65535;
	tc->max         = 0;
	tc->cur         = 0;
	tc->utt         = NULL;
	tc->root_utt    = NULL;
	return(tc);
}

static void ReadCodesOrder() {
	FILE *fp;
	char code[256];
	register int lp;
	CHAIN_CODES *tcode = NULL;
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(CODES_ORDER_FILE,"r",TRUE,TRUE,mFileName)) == NULL)
		return;
	while (fgets_cr(code, 255, fp)) {
		if (uS.isUTF8(code) || uS.isInvisibleHeader(code))
			continue;
		lp = strlen(code) - 1;
		if (code[lp] == '\n')
			code[lp] = EOS;
		if (!nomap)
			uS.lowercasestr(code, &dFnt, C_MBF);
		if (root_code == NULL) {
			tcode = chains_create_code(code);
			root_code = tcode;
		} else if (tcode != NULL) {
			tcode->nextcode = chains_create_code(code);
			tcode = tcode->nextcode;
		}
		tcode->nextcode = NULL;
		tcode->fn = FALSE;
	}
	fclose(fp);
}

void init(char f) {
	if (f) {
		utt_line = 0;
		root_sp = NULL;
		root_code = NULL;
		root_clause = NULL;
		chains_onlydata = FALSE;
		LocalTierSelect = TRUE;
		RemPercWildCard = FALSE;
		tct = TRUE;
		sec_code[0] = EOS;
		chains_CodeTierGiven = 0;
		FilterTier = 0;
		ReadCodesOrder();
	} else {
		if (chains_CodeTierGiven == 0) {
			fprintf(stderr,"Please specify a code tier with \"+t\" option.\n");
			chains_exit(0);
		}
		if (!combinput) {
			CHAIN_CODES *p;

			chains_free_sp(root_sp);
			for (p=root_code; p != NULL; p=p->nextcode) {
				if (p->root_utt != NULL) {
					chains_free_utt(p->root_utt);
					p->root_utt = NULL;
				}
				p->fn = FALSE;
			}
			utt_line = 0;
			root_sp = NULL;
		}
	}
}

static int chains_isany(CHAIN_CODES *tc) {
	while (tc) {
		if (tc->fn)
			return(TRUE);
		tc = tc->nextcode;
	}
	return(FALSE);
}

static void chains_pr_stats(const char *pat, const char *st) {
	float avg;
	CHAIN_CODES *tc;

	if (*st == '\001')
    	fprintf(fpout,pat,st+1);
	else
    	fprintf(fpout,pat,st);
	fprintf(fpout,"\t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			tc = tc->nextcode;
			if (tc == NULL)
				break;
		} else {
			fprintf(fpout,"%s\t", tc->code);
			tc = tc->nextcode;
			if (!chains_isany(tc))
				break;
		}
	}
	putc('\n', fpout);
	putc('\n', fpout);
	fprintf(fpout,"# chains\t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			if (tc->nextcode == NULL)
				break;
			tc = tc->nextcode;
		} else {
			fprintf(fpout,"%.0f", tc->num_chains);
			if (!chains_isany(tc->nextcode))
				break;
			putc('\t', fpout);
			tc = tc->nextcode;
		}
	}
	putc('\n', fpout);
	fprintf(fpout,"Avg leng\t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			if (tc->nextcode == NULL)
				break;
			tc = tc->nextcode;
		} else {
			if (!tc->chain_break) {
				tc->total_len += (float)tc->cur;
				tc->total_lensqr = tc->total_lensqr + (tc->cur * tc->cur);
			}
			if (*st == '\001')
				tc->all_sp_total_len = tc->total_len;
			if (tc->num_chains <= (float)0.0)
				avg = (float)0.0;
			else
				avg = tc->total_len / tc->num_chains;
			fprintf(fpout,"%.2f", avg);
			if (!chains_isany(tc->nextcode))
				break;
			putc('\t', fpout);
			tc = tc->nextcode;
		}
	}
	putc('\n', fpout);
	fprintf(fpout,"Std dev \t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			if (tc->nextcode == NULL)
				break;
			tc = tc->nextcode;
		} else {
			if (tc->num_chains*(tc->num_chains-1)==(float)0.0)
				avg=(float)0.0;
			else {
				if (tc->num_chains <= (float)0.0)
					avg = (float)0.0;
				else
					avg = tc->total_len / tc->num_chains;
				avg= (float)sqrt((tc->total_lensqr/tc->num_chains)-(avg*avg));
			}
			fprintf(fpout,"%.2f", avg);
			if (!chains_isany(tc->nextcode))
				break;
			putc('\t', fpout);
			tc = tc->nextcode;
		}
	}
	putc('\n', fpout);
	fprintf(fpout,"Min leng\t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			if (tc->nextcode == NULL)
				break;
			tc = tc->nextcode;
		} else {
			if (tc->min > tc->cur && tc->cur > 0)
				tc->min = tc->cur;
			if (tc->min == (unsigned int)65535)
				fprintf(fpout,"0");
			else
				fprintf(fpout,"%u", tc->min);
			if (!chains_isany(tc->nextcode))
				break;
			putc('\t', fpout);
			tc = tc->nextcode;
		}
	}
	putc('\n', fpout);
	fprintf(fpout,"Max leng\t");
	tc = root_code;
	while (1) {
		if (tc->fn == FALSE) {
			if (tc->nextcode == NULL) break;
			tc = tc->nextcode;
		} else {
			if (tc->max < tc->cur && tc->cur > 0)
				tc->max = tc->cur;
			fprintf(fpout,"%u", tc->max);
			if (!chains_isany(tc->nextcode))
				break;
			putc('\t', fpout);
			tc = tc->nextcode;
		}
	}
	putc('\n', fpout);
	if (*st != '\001') {
		fprintf(fpout,"SP Part.\t");
		tc = root_code;
		while (1) {
			if (tc->fn == FALSE) {
				if (tc->nextcode == NULL)
					break;
				tc = tc->nextcode;
			} else {
				fprintf(fpout,"%u", tc->total_num_chains);
				if (!chains_isany(tc->nextcode))
					break;
				putc('\t', fpout);
				tc = tc->nextcode;
			}
		}
		putc('\n', fpout);

		fprintf(fpout,"SP/Total\t");
		tc = root_code;
		while (1) {
			if (tc->fn == FALSE) {
				if (tc->nextcode == NULL)
					break;
				tc = tc->nextcode;
			} else {
				if (tc->all_sp_total_len <= (float)0.0)
					avg = (float)0.0;
				else
					avg = tc->total_len / tc->all_sp_total_len;
				fprintf(fpout,"%.2f", avg);
				if (!chains_isany(tc->nextcode))
					break;
				putc('\t', fpout);
				tc = tc->nextcode;
			}
		}
		putc('\n', fpout);
	}

/* reset loop */
    tc = root_code;
    while (1) {
		if (tc->fn == FALSE) {
			tc = tc->nextcode;
			if (tc == NULL)
				break;
		} else {
			tc->utt = tc->root_utt;
			tc->chain_break = TRUE;
			tc->total_chain_break = TRUE;
			tc->num_chains = (float)0.0; tc->total_len = (float)0.0;
			tc->total_num_chains = (unsigned int)0;
			tc->total_lensqr = (float)0.0;
			tc->min = (unsigned int)65535; tc->max = 0; tc->cur = 0;
			tc = tc->nextcode;
			if (!chains_isany(tc))
				break;
		}
    }
}

static void chains_pr_result(void) {
	int ti;
	int cnt = 0;
	int cnt_items = 0;
	char  fontName[256];
	CHAIN_CODES *tc;
	CHAIN_SPEAKERS *tsp;
	unsigned int tutt_line;
	unsigned int lno = 0;
	unsigned int tlno;


	if (stout) {
		strcpy(fontName, "CAfont:13:7\n");
		cutt_SetNewFont(fontName, EOS);
	} else
		fprintf(fpout,"%s	CAfont:13:7\n", FONTHEADER);

	if (root_sp != NULL) {
		fprintf(fpout,"Speaker markers:  %d=%s", ++cnt, root_sp->sp);
		for (tsp=root_sp->nextsp; tsp != NULL; tsp=tsp->nextsp)
			fprintf(fpout,", %d=%s", ++cnt, tsp->sp);
		putc('\n', fpout);
		putc('\n', fpout);
	}
	cnt_items = 0;
	if (chains_isany(root_code)) {
		tc = root_code;
		while (1) {
			if (tc->fn == FALSE) {
				tc = tc->nextcode;
				if (tc == NULL)
					break;
			} else {
				cnt_items++;
				tc->utt = tc->root_utt;
				fprintf(fpout,"%s\t", tc->code);
				tc = tc->nextcode;
				if (!chains_isany(tc))
					break;
			}
		}
		fprintf(fpout,"line #");
		putc('\n', fpout);
	}
	tlno = 1;
	tutt_line = 0;
	while (tutt_line < utt_line) {
		cnt = FALSE;
		tutt_line = tutt_line + 1;

		/* print all the missing lines */
		if (chains_onlydata == 2) {
			for (tc=root_code; tc != NULL; tc=tc->nextcode) {
				if (tc->fn && tc->utt != NULL) {
					if (tc->utt->utt_line == tutt_line) {
						for (lno=tc->utt->lineno; tlno < lno; tlno++) {
							for (ti=0; ti < cnt_items-1; ti++)
								putc('\t', fpout);
							fprintf(fpout, "\t%5u\n", tlno);
						}
					}
				}
			}
		}

		for (tc=root_code; tc != NULL; tc=tc->nextcode) {
			if (tc->fn == FALSE)
				continue;
			if (tc->utt != NULL) {
				if (tc->utt->utt_line == tutt_line) {
					cnt = TRUE;
					lno = tc->utt->lineno;
					tlno = lno + 1;
					fprintf(fpout,"%d", tc->utt->spnum);
					if (tc->utt->st != NULL) {
						fprintf(fpout," %s", tc->utt->st+1);
					}
					/*		    if (chains_isany(tc->nextcode)) */
					if (strlen(tc->code) > 7)
						putc('\t', fpout);
					putc('\t', fpout);
					tc->cur++;
					if (tc->utt->st != NULL) {
						for (ti=1; tc->utt->st[ti]; ti++) {
							if (tc->utt->st[ti] == ',')
								tc->cur++;
						}
					}
					if (tc->chain_break) {
						tc->chain_break = FALSE;
						tc->num_chains++;
					}
					tc->utt = tc->utt->nextutt;
				} else {
					if (chains_onlydata) {
						cnt = TRUE;
						fprintf(fpout," ");
					} else if (cnt_items != 1) {
						cnt = TRUE;
						fprintf(fpout,"0");
					}
					if (strlen(tc->code) > 7)
						putc('\t', fpout);
					putc('\t', fpout);
					/*		    if (chains_isany(tc->nextcode)) */
					if (!tc->chain_break) {
						tc->chain_break = TRUE;
						tc->total_len += (float)tc->cur;
						tc->total_lensqr = tc->total_lensqr+(tc->cur*tc->cur);
						if (tc->min > tc->cur)
							tc->min = tc->cur;
						if (tc->max < tc->cur)
							tc->max = tc->cur;
						tc->cur = 0;
					}
				}
			} else {
				if (chains_onlydata) {
					cnt = TRUE;
					fprintf(fpout," ");
				} else if (cnt_items != 1) {
					cnt = TRUE;
					fprintf(fpout,"0");
				}
				if (strlen(tc->code) > 7)
					putc('\t', fpout);
				putc('\t', fpout);
				/*		if (chains_isany(tc->nextcode)) */
				if (!tc->chain_break) {
					tc->chain_break = TRUE;
					tc->total_len += (float)tc->cur;
					tc->total_lensqr = tc->total_lensqr+(tc->cur*tc->cur);
					if (tc->min > tc->cur)
						tc->min = tc->cur;
					if (tc->max < tc->cur)
						tc->max = tc->cur;
					tc->cur = 0;
				}
			}
		}
		if (cnt)
			fprintf(fpout, "%5u\n", lno);
	}
	if (chains_isany(root_code)) {
		cnt = 0;
		chains_pr_stats("\n%s speakers:\n", "\001ALL");
		for (tsp=root_sp; tsp != NULL; tsp=tsp->nextsp) {
			++cnt;
			tutt_line = 0;
			while (tutt_line < utt_line) {
				tutt_line = tutt_line + 1;
				for (tc=root_code; tc != NULL; tc=tc->nextcode) {
					if (tc->fn == FALSE) 
						continue;
					if (tc->utt != NULL) {
						if (tc->utt->utt_line == tutt_line) {
							if (cnt == tc->utt->spnum) {
								tc->cur++;
								if (tc->utt->st != NULL) {
									for (ti=1; tc->utt->st[ti]; ti++) {
										if (tc->utt->st[ti] == ',')
											tc->cur++;
									}
								}
								if (tc->chain_break) {
									tc->chain_break = FALSE;
									tc->num_chains++;
								}
								if (tc->total_chain_break) {
									tc->total_chain_break = FALSE;
									tc->total_num_chains++;
								}
							} else if (!tc->chain_break) {
								tc->chain_break = TRUE;
								tc->total_len += (float)tc->cur;
								tc->total_lensqr = tc->total_lensqr+(tc->cur*tc->cur);
								if (tc->min > tc->cur)
									tc->min = tc->cur;
								if (tc->max < tc->cur)
									tc->max = tc->cur;
								tc->cur = 0;
							}
							tc->utt = tc->utt->nextutt;
						} else {
							tc->total_chain_break = TRUE;
							if (!tc->chain_break) {
								tc->chain_break = TRUE;
								tc->total_len += (float)tc->cur;
								tc->total_lensqr = tc->total_lensqr+(tc->cur*tc->cur);
								if (tc->min > tc->cur)
									tc->min = tc->cur;
								if (tc->max < tc->cur)
									tc->max = tc->cur;
								tc->cur = 0;
							}
						}
					} else {
						tc->total_chain_break = TRUE;
						if (!tc->chain_break) {
							tc->chain_break = TRUE;
							tc->total_len += (float)tc->cur;
							tc->total_lensqr = tc->total_lensqr+(tc->cur*tc->cur);
							if (tc->min > tc->cur)
								tc->min = tc->cur;
							if (tc->max < tc->cur)
								tc->max = tc->cur;
							tc->cur = 0;
						}
					}
				}
			}
			chains_pr_stats("\nSpeakers %s:\n", tsp->sp);
		}
	}
}

static int get_speaker(char *spname) {
	int cnt = 0;
	CHAIN_SPEAKERS *tsp;

	if (root_sp == NULL) {
		if ((root_sp=NEW(CHAIN_SPEAKERS)) == NULL)
			out_of_mem();
		tsp = root_sp;
		cnt = 1;
	} else {
		for (tsp=root_sp; 1; tsp=tsp->nextsp) {
			cnt++;
			if (!strcmp(spname,tsp->sp))
				return(cnt);
			if (tsp->nextsp == NULL)
				break;
		}
		cnt++;
		if ((tsp->nextsp=NEW(CHAIN_SPEAKERS)) == NULL)
			out_of_mem();
		tsp = tsp->nextsp;
	}
	if ((tsp->sp=(char *)malloc(strlen(spname)+1)) == NULL)
		out_of_mem();
	strcpy(tsp->sp,spname);
	tsp->nextsp = NULL;
	return(cnt);
}

static CHAIN_CODES *get_code(char *code) {
	int res;
	CHAIN_CODES *tcode, *t, *tt;

	if (root_code == NULL) {
		tcode = chains_create_code(code);
		tcode->nextcode = NULL;
		tcode->fn = TRUE;
		root_code = tcode;
	} else if ((res=strcmp(code, root_code->code)) < 0) {
		tcode = root_code->nextcode;
		while (tcode != NULL) {
			if (strcmp(code,tcode->code) == 0) {
				tcode->fn = TRUE;
				return(tcode);
			}
			tcode = tcode->nextcode;
		}
		tcode = chains_create_code(code);
		tcode->nextcode = root_code;
		tcode->fn = TRUE;
		root_code = tcode;
	} else if (res == 0) {
		tcode = root_code;
		tcode->fn = TRUE;
	} else {
		t  = root_code;
		tt = root_code->nextcode;
		while (tt != NULL) {
			if ((res=strcmp(code, tt->code)) < 0) {
				tcode = tt->nextcode;
				while (tcode != NULL) {
					if (strcmp(code,tcode->code) == 0) {
						tcode->fn = TRUE;
						return(tcode);
					}
					tcode = tcode->nextcode;
				}
				break;
			} else if (res == 0) {
				tt->fn = TRUE;
				return(tt);
			}
			t = tt;
			tt = tt->nextcode;
		}
		tcode = chains_create_code(code);
		tcode->fn = TRUE;
		if (tt == NULL) {
			t->nextcode = tcode;
			tcode->nextcode = NULL;
		} else {
			t->nextcode = tcode;
			tcode->nextcode = tt;
		}
	}
	return(tcode);
}

static void add_code(char *code, char *diff, int sp,
				unsigned int flineno, unsigned int utt_line) {
	char *t;
	register int len;
	CHAIN_CODES *tcode;

	tcode = get_code(code);
	if (tcode->root_utt == NULL) {
		if ((tcode->root_utt=NEW(CHAIN_UTTS)) == NULL)
			out_of_mem();
		tcode->utt = tcode->root_utt;
	} else if (utt_line == tcode->utt->utt_line) {
		if (tcode->utt->st == NULL && *diff == EOS) {
			fprintf(stderr,"On tier starting on line %u;\n", flineno);
			fprintf(stderr,"Code \"%s\" is assigned twice to the same clause!\n",code);
			fprintf(stderr,"Possibly code delimiter \".\" is missing.\n");
			chains_exit(1);
		} else if (tcode->utt->st == NULL) {
			len = strlen(diff) + 2;
			tcode->utt->st = (char *)malloc(len);
			if (tcode->utt->st == NULL)
				out_of_mem();
			tcode->utt->st[0] = (char)len;
			strcpy(tcode->utt->st+1, diff);
		} else {
			len = strlen(diff) + 2 + (int)tcode->utt->st[0];
			t = tcode->utt->st;
			tcode->utt->st = (char *)malloc(len);
			if (tcode->utt->st == NULL)
				out_of_mem();
			tcode->utt->st[0] = (char)len;
			strcpy(tcode->utt->st+1, t+1);
			strcat(tcode->utt->st+1, ", ");
			strcat(tcode->utt->st+1, diff);
			free(t);
		}
		return;
	} else {
		if ((tcode->utt->nextutt=NEW(CHAIN_UTTS)) == NULL)
			out_of_mem();
		tcode->utt = tcode->utt->nextutt;
	}
	tcode->utt->spnum = (char)sp;
	if (*diff) {
		len = strlen(diff) + 2;
		tcode->utt->st = (char *)malloc(len);
		if (tcode->utt->st == NULL)
			out_of_mem();
		tcode->utt->st[0] = (char)len;
		strcpy(tcode->utt->st+1, diff);
	} else
		tcode->utt->st = NULL;
	tcode->utt->utt_line = utt_line;
	tcode->utt->lineno = llineno;
	tcode->utt->nextutt = NULL;
}

static void make_diff(char *diff, char *ow, char *tw) {
	char *tt;

	for (; *tw; ow++) {
		if (*tw == '\001') {
			for (tt=tw; *tt == '\001'; tt++, ow++)
				*diff++ = *ow;
			strcpy(tw, tt);
		} else
			tw++;
	}
	*diff = EOS;
}

static int IsClauseMarker(char *s) {
	CHAIN_CLAUSES *tc;

	for (tc=root_clause; tc; tc=tc->nextsp) {
		if (!strcmp(s, tc->sp))
			return(TRUE);
	}
	return(FALSE);
}

static int chains_getword(char *code, char *word, int i) {
	register char sq;

	while ((*word=code[i++]) != EOS && uS.isskip(code,i-1,&dFnt,MBF) && *word != '[') ;
	if (*word == EOS)
		return(FALSE);
	if (*word == '[')
		sq = TRUE;
	else
		sq = FALSE;
	while ((*++word=code[i++]) != EOS && (!uS.isskip(code,i-1,&dFnt,MBF) || sq)) {
		if (*word == ']') {
			*++word = EOS;
			return(i);
		}
	}
	if (!*word || *word == '[')
		i--;
	else
		*word = EOS;
	return(i);
}

static void AddSecondTierData(int beg, int end, char *code_two,
				unsigned int slineno, unsigned int utt_line) {
	char *t;
	register int i;
	register int j;
	char found = FALSE;
	CHAIN_UTTS  *tu;
	CHAIN_CODES *tcode;

	for (tcode=root_code; tcode != NULL; tcode=tcode->nextcode) {
		if (tcode->fn == FALSE)
			continue;
		for (tu=tcode->root_utt; tu != NULL; tu=tu->nextutt) {
			if (utt_line == tu->utt_line) {
				found = TRUE;
				if (tu->st == NULL) {
					for (i=beg, j=0; i < end && code_two[i] != EOS; i++)
						j++;
					j += 2;
					tu->st = (char *)malloc(j);
					if (tu->st == NULL)
						out_of_mem();
					tu->st[0] = (char)j;
					for (i=beg, j=1; i < end && code_two[i] != EOS; i++, j++)
						tu->st[j]= code_two[i];
					tu->st[j] = EOS;
				} else {
				j = 2 + (int)tu->st[0];
					for (i=beg; i < end && code_two[i] != EOS; i++, j++) ;
					t = tu->st;
					tu->st = (char *)malloc(j);
					if (tu->st == NULL)
						out_of_mem();
					tu->st[0] = (char)j;
					strcpy(tu->st+1, t+1);
					strcat(tu->st+1, "  ");
// more efficient   j = (int)t[0] + 1;
					j = strlen(tu->st+1) + 1;
					for (i=beg; i < end && code_two[i] != EOS; i++, j++)
						tu->st[j]= code_two[i];
					tu->st[j] = EOS;
					free(t);
				}
				break;
			}
		}
	}
	if (!found) {
		uS.lowercasestr(utterance->speaker, &dFnt, MBF);
		fprintf(stderr,"On tier starting on line %u;\n", slineno);
		fprintf(stderr,"Data on second code tier refers to nonexistent unit:\n\t%s\t%s",sec_code,code_two);
		chains_exit(0);
	}
}

static void analyse_codes(char *code_one, char *code_two, char *temp_buf,
		    unsigned int flineno, unsigned int slineno, int speaker,
		    unsigned int *l_offset, int *utt_offset) {
	register int  i;
	register char fn;
	unsigned int  sec_l_offset;
	int  clause_count;
	char next_clause;
	char clauses[30];
	char org_word[SPEAKERLEN], diff[SPEAKERLEN];

	for (i=0; i < 30; i++)
		clauses[i] = FALSE;
	if (code_one[0]) {
		clause_count = 0;
		for (i=0; uS.isskip(code_one,i,&dFnt,MBF) && code_one[i] != EOS; i++) {
			if (code_one[i] == '.')
				clauses[clause_count++] = FALSE;
		}
		next_clause = TRUE;
		while ((i=chains_getword(code_one,temp_buf,i))) {
			if (next_clause) {
				clauses[clause_count++] = TRUE;
				(*l_offset) = (*l_offset) + 1;
				(*utt_offset) = (*utt_offset) - 1;
				next_clause = FALSE;
			}
			if (!nomap)
				uS.lowercasestr(temp_buf, &dFnt, MBF);
			strcpy(org_word, temp_buf);
			if (*temp_buf == CODECHAR && exclude(temp_buf)) {
				make_diff(diff, org_word, temp_buf);
				add_code(temp_buf,diff,speaker,flineno,utt_line+(*l_offset));
			}
			for (i--; uS.isskip(code_one,i,&dFnt,MBF) && code_one[i] != EOS; i++) {
				if (code_one[i] == '.') {
					if (next_clause)
						clauses[clause_count++] = FALSE;
					next_clause = TRUE;
				}
			}
		}
		if (*utt_offset < 0) {
			fprintf(stderr,"On tier starting on line %u; ", flineno);
			fprintf(stderr,"More codes than clauses on code line:\n");
			fprintf(stderr, "%s", code_one);
			chains_exit(0);
		}
	}

	if (code_two[0]) {
		i = 0;
		checktier(sec_code);
		clause_count = 0;
		sec_l_offset = 1;
		while (1) {
			fn = FALSE;
			while (uS.isskip(code_two,i,&dFnt,MBF) && code_two[i]!='.' && code_two[i])
				i++;
			for (speaker=i; code_two[i] != '.' && code_two[i]; i++) {
				if (!uS.isskip(code_two,i,&dFnt,MBF))
					fn = TRUE;
			}
			if (!fn)
				break;
			if (speaker != i) {
				if (!clauses[clause_count]) {
					uS.lowercasestr(utterance->speaker, &dFnt, MBF);
					fprintf(stderr,"On line %d;\n", slineno);
					fprintf(stderr,"Data on second code tier refers to ");
					fprintf(stderr,"nonexistent unit on first tier:\n");
					fprintf(stderr,"\t%s\t%s", sec_code, code_two);
					chains_exit(0);
				}
				AddSecondTierData(speaker,i,code_two,
				slineno,utt_line+sec_l_offset);
			}
			if (code_two[i] == '.') {
				if (clauses[clause_count++])
					sec_l_offset++;
				i++;
				if (!code_two[i])
					break;
			}
		}
	}
}

void call() {
	register int i;
	register int j;
	int speaker;
	unsigned int l_offset;
	unsigned int flineno = 0;
	unsigned int slineno = 0;
	int utt_offset;
	char temp_buf[SPEAKERLEN];

	speaker = 0;
	*temp_buf = EOS;
	utt_offset = 0;
	l_offset = 0;
	llineno = 0;
	spareTier1[0] = EOS; spareTier2[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
		if (checktier(utterance->speaker) || *utterance->speaker == '*') {
			fprintf(fpout, "sp=%s; uttline=%s", utterance->speaker, uttline);
			if (uttline[strlen(uttline)-1] != '\n') putc('\n', fpout);
		}
*/
#if !defined(CLAN_SRV)
		if (lineno % 250 == 0)
			fprintf(stderr,"\r%ld ", lineno);
#endif
		uS.uppercasestr(utterance->speaker, &dFnt, MBF);
		if (*utterance->speaker == '*') {
			analyse_codes(spareTier1,spareTier2,temp_buf,flineno,slineno,speaker,&l_offset,&utt_offset);
			spareTier1[0] = EOS; spareTier2[0] = EOS;
			llineno = (unsigned int)lineno;
			i = 0;
			utt_offset = 0;
			utt_line = utt_line + l_offset;
			l_offset = 0;
			if (root_clause) {
				while ((i=getword(utterance->speaker, uttline, temp_buf, NULL, i))) {
					if (IsClauseMarker(temp_buf))
						utt_offset = utt_offset + 1;
				}
			} else {
				while (uttline[i]) {
					if (uS.IsUtteranceDel(uttline, i)) {
						do {
							i++;
						} while (uS.IsUtteranceDel(uttline, i)) ;
						utt_offset = utt_offset + 1;
					} else i++;
				}
				if (utt_offset == 0 && uttline[0])
					utt_offset = 1;
			}
			if (checktier(utterance->speaker)) {
				strcpy(temp_buf,utterance->speaker);
				for (i=strlen(temp_buf); temp_buf[i] != ':' && i > 0; i--) ;
				temp_buf[i] = EOS;
				speaker = get_speaker(temp_buf);
			} else
				speaker = 0;
		} else if (uS.partcmp(utterance->speaker, sec_code, FALSE, TRUE) && *sec_code) {
			slineno = (unsigned int)lineno;
			for (i=0, j=0; uttline[i]; i++) {
				if (uttline[i] == ',') {
					if (uttline[i+1] != ' ' && uttline[i+1] != EOS)
						spareTier2[j++] = ' ';
				} else if (uttline[i] != '\n') {
					spareTier2[j++] = uttline[i];
				}
			}
			spareTier2[j] = EOS;
		} else if (speaker && checktier(utterance->speaker)) {
			flineno = (unsigned int)lineno;
			strcpy(spareTier1, uttline);
		}
	}
	fprintf(stderr,"\n");
	analyse_codes(spareTier1,spareTier2,temp_buf,flineno,slineno,speaker,&l_offset,&utt_offset);
	utt_line = utt_line + l_offset;
	if (!combinput)
		chains_pr_result();
}

static void AddClause(char *s) {
	CHAIN_CLAUSES *tc;

	if ((tc=NEW(CHAIN_CLAUSES)) == NULL)
		out_of_mem();
	if ((tc->sp=(char *)malloc(strlen(s)+1)) == NULL)
		out_of_mem();
	strcpy(tc->sp, s);
	tc->nextsp = root_clause;
	root_clause = tc;
}

static void AddClauseFromFile(FNType *fname) {
	FILE *fp;
	char wd[256];
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the delemeter files:\n\t\"%s\", \"%s\"\n",fname,mFileName);
		chains_exit(0);
	}
	while (fgets_cr(wd, 255, fp)) {
		if (uS.isUTF8(wd) || uS.isInvisibleHeader(wd))
			continue;
		AddClause(wd);
	}
	fclose(fp);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'd':
			chains_onlydata = (char)(atoi(getfarg(f,f1,i))+1);
			if (chains_onlydata > 2) {
				fprintf(stderr,"+d levels are 0-1.\n");
				cutt_exit(0);
			}
			break;
		case 'c':
			if (!*f) {
				fprintf(stderr,"Specify clause delemeters after +c option.\n");
				chains_exit(0);
			}
			if (*f == '@') {
				f++;
				AddClauseFromFile(f);
			} else {
				if (*f == '\\')
					f++;
				AddClause(f);
			}
			break;
		case 't':
			if (*f == '%' && *(f-2) == '+') {
				if (chains_CodeTierGiven == 1) {
					strcpy(sec_code, f);
					uS.uppercasestr(sec_code, &dFnt, C_MBF);
				}
				chains_CodeTierGiven++;
			}
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = CHAINS;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,chains_pr_result);
	chains_free_clause(root_clause);
	chains_free_sp(root_sp);
	chains_free_codes(root_code);
}
