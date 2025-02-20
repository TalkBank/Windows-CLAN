/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

const char *feats[220];
static int featSize; 

static void init_feats(void) {
	int i;

	i=0;
	feats[i++] = "0";
	feats[i++] = "1";
	feats[i++] = "2";
	feats[i++] = "3";
	feats[i++] = "4";
	feats[i++] = "Abe";
	feats[i++] = "Abl";
	feats[i++] = "Abs";
	feats[i++] = "Abv";
	feats[i++] = "Acc";
	feats[i++] = "Act";
	feats[i++] = "Add";
	feats[i++] = "Ade";
	feats[i++] = "All";
	feats[i++] = "Anim";
	feats[i++] = "Antip";
	feats[i++] = "Arch";
	feats[i++] = "Art";
	feats[i++] = "Aug";
	feats[i++] = "Aux";
	feats[i++] = "Bantu1";
	feats[i++] = "Bantu2";
	feats[i++] = "Bantu3";
	feats[i++] = "Bantu4";
	feats[i++] = "Bantu5";
	feats[i++] = "Bantu6";
	feats[i++] = "Bantu7";
	feats[i++] = "Bantu8";
	feats[i++] = "Bantu9";
	feats[i++] = "Bantu10";
	feats[i++] = "Bantu11";
	feats[i++] = "Bantu12";
	feats[i++] = "Bantu13";
	feats[i++] = "Bantu14";
	feats[i++] = "Bantu15";
	feats[i++] = "Bantu16";
	feats[i++] = "Bantu17";
	feats[i++] = "Bantu18";
	feats[i++] = "Bantu19";
	feats[i++] = "Bantu20";
	feats[i++] = "Bantu21";
	feats[i++] = "Bantu22";
	feats[i++] = "Bantu23";
	feats[i++] = "Bel";
	feats[i++] = "Ben";
	feats[i++] = "Bfoc";
	feats[i++] = "Brck";
	feats[i++] = "Card";
	feats[i++] = "Cau";
	feats[i++] = "Circ";
	feats[i++] = "Clf";
	feats[i++] = "Cmp";
	feats[i++] = "Cnd";
	feats[i++] = "Cns";
	feats[i++] = "Coll";
	feats[i++] = "Colo";
	feats[i++] = "Com";
	feats[i++] = "Combi";
	feats[i++] = "Comm";
	feats[i++] = "Cons";
	feats[i++] = "Conv";
	feats[i++] = "Cop";
	feats[i++] = "Count";
	feats[i++] = "Dash";
	feats[i++] = "Dat";
	feats[i++] = "Def";
	feats[i++] = "Deg";
	feats[i++] = "Del";
	feats[i++] = "Dem";
	feats[i++] = "Des";
	feats[i++] = "Digit";
	feats[i++] = "Dim";
	feats[i++] = "Dir";
	feats[i++] = "Dis";
	feats[i++] = "Dist";
	feats[i++] = "Ditr";
	feats[i++] = "Dual";
	feats[i++] = "Ech";
	feats[i++] = "Ela";
	feats[i++] = "Elev";
	feats[i++] = "Elip";
	feats[i++] = "Emp";
	feats[i++] = "Equ";
	feats[i++] = "Erg";
	feats[i++] = "Ess";
	feats[i++] = "Even";
	feats[i++] = "Ex";
	feats[i++] = "Exc";
	feats[i++] = "Excl";
	feats[i++] = "Expr";
	feats[i++] = "Fem";
	feats[i++] = "Fh";
	feats[i++] = "Fin";
	feats[i++] = "Form";
	feats[i++] = "Frac";
	feats[i++] = "Fut";
	feats[i++] = "Gdv";
	feats[i++] = "Gen";
	feats[i++] = "Geo";
	feats[i++] = "Ger";
	feats[i++] = "Giv";
	feats[i++] = "Grpa";
	feats[i++] = "Grpl";
	feats[i++] = "Hab";
	feats[i++] = "Hum";
	feats[i++] = "Humb";
	feats[i++] = "Ill";
	feats[i++] = "Imp";
	feats[i++] = "In";
	feats[i++] = "Inan";
	feats[i++] = "Ind";
	feats[i++] = "Indir";
	feats[i++] = "Ine";
	feats[i++] = "Inf";
	feats[i++] = "Infm";
	feats[i++] = "Ini";
	feats[i++] = "Ins";
	feats[i++] = "Int";
	feats[i++] = "Intr";
	feats[i++] = "Inv";
	feats[i++] = "Irr";
	feats[i++] = "Iter";
	feats[i++] = "Jus";
	feats[i++] = "Lat";
	feats[i++] = "Lfoc";
	feats[i++] = "Light";
	feats[i++] = "Loc";
	feats[i++] = "Man";
	feats[i++] = "Masc";
	feats[i++] = "Med";
	feats[i++] = "Mid";
	feats[i++] = "Mod";
	feats[i++] = "Mult";
	feats[i++] = "Nat";
	feats[i++] = "Nec";
	feats[i++] = "Neg";
	feats[i++] = "Neut";
	feats[i++] = "Nfh";
	feats[i++] = "Nhum";
	feats[i++] = "Nom";
	feats[i++] = "Npr";
	feats[i++] = "Nvis";
	feats[i++] = "Oper";
	feats[i++] = "Opt";
	feats[i++] = "Ord";
	feats[i++] = "Oth";
	feats[i++] = "Par";
	feats[i++] = "Part";
	feats[i++] = "Pass";
	feats[i++] = "Past";
	feats[i++] = "Pat";
	feats[i++] = "Pauc";
	feats[i++] = "Per";
	feats[i++] = "Perf";
	feats[i++] = "Peri";
	feats[i++] = "Plur";
	feats[i++] = "Pos";
	feats[i++] = "Post";
	feats[i++] = "Pot";
	feats[i++] = "Pqp";
	feats[i++] = "Pre";
	feats[i++] = "Pred";
	feats[i++] = "Prep";
	feats[i++] = "Pres";
	feats[i++] = "Pro";
	feats[i++] = "Prog";
	feats[i++] = "Prosp";
	feats[i++] = "Prox";
	feats[i++] = "Prp";
	feats[i++] = "Prs";
	feats[i++] = "Ptan";
	feats[i++] = "Qest";
	feats[i++] = "Qot";
	feats[i++] = "Quasi";
	feats[i++] = "Quot";
	feats[i++] = "Range";
	feats[i++] = "Rare";
	feats[i++] = "Rcp";
	feats[i++] = "Rdp";
	feats[i++] = "Rel";
	feats[i++] = "Remt";
	feats[i++] = "Roman";
	feats[i++] = "Sbe";
	feats[i++] = "Sbl";
	feats[i++] = "Semi";
	feats[i++] = "Sets";
	feats[i++] = "Sing";
	feats[i++] = "Slng";
	feats[i++] = "Slsh";
	feats[i++] = "Spec";
	feats[i++] = "Spl";
	feats[i++] = "Sub";
	feats[i++] = "Sur";
	feats[i++] = "Tem";
	feats[i++] = "Ter";
	feats[i++] = "Tim";
	feats[i++] = "Tot";
	feats[i++] = "Tra";
	feats[i++] = "Tran";
	feats[i++] = "Tri";
	feats[i++] = "Vbp";
	feats[i++] = "Vnoun";
	feats[i++] = "Voc";
	feats[i++] = "Vrnc";
	feats[i++] = "Vulg";
	feats[i++] = "Wol1";
	feats[i++] = "Wol2";
	feats[i++] = "Wol3";
	feats[i++] = "Wol4";
	feats[i++] = "Wol5";
	feats[i++] = "Wol6";
	feats[i++] = "Wol7";
	feats[i++] = "Wol8";
	feats[i++] = "Wol9";
	feats[i++] = "Wol10";
	feats[i++] = "Wol11";
	feats[i++] = "Wol12";
	feats[i++] = "Word";
	feats[i++] = "Yes";
	featSize = i;
}

void init(char f) {
    if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		init_feats();
   }
}

void usage()			/* print proper usage and exit */
{
	puts("TEMP replace on %mor tier - in lemma with u2013, replace & with -");
    printf("Usage: temp [%s] filename(s)\n",mainflgs());
    mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
    OnlydataLimit = 0;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,NULL);
}
	
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char isFeat(char *line) {
	int i, len;
//	int j, oi;

	for (i=0; i < featSize; i++) {
		len = strlen(feats[i]);
		if (uS.mStrnicmp(line+1, feats[i], len) == 0) {
			len++;
			if (line[len] == '-' || line[len] == '&' || line[len] == '\n' || isSpace(line[len]) || line[len] == EOS)
				return(TRUE);
/*
			else {
				oi = len;
				for (j=0; j < featSize; j++) {
					len = strlen(feats[j]);
					if (strncmp(line+oi, feats[j], len) == 0) {
						if (line[oi+len] == '-' || line[oi+len] == '&' || line[oi+len] == '\n' || isSpace(line[oi+len]) || line[oi+len] == EOS)
							return(TRUE);
					}
				}
			}
 */
		}
	}
	return(FALSE);
}

void call() {
	int i;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"%mor",FALSE,FALSE)) {
			for (i=0; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '-') {
					if (!isFeat(utterance->line+i)) {
						fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(stderr, "%s\n", utterance->line+i);
						att_shiftright(utterance->line+i, utterance->attLine+i, 2);
						utterance->line[i++] = 0xE2;
						utterance->line[i++] = 0x80;
						utterance->line[i]   = 0x93;
					}
				} else if (utterance->line[i] == '&') {
					utterance->line[i]   = '-';
				}
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
