/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// change date to current: <oai:datestamp>2008-04-04</oai:datestamp>

#define CHAT_MODE 0
#include "cu.h"
#include "c_curses.h"
#include <time.h>

#ifdef UNX
	#define RGBColor int
#endif

#if !defined(UNX)
#define _main olac_main
#define call olac_call
#define getflag olac_getflag
#define init olac_init
#define usage olac_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#define DICNAME "changes.cut"

enum {
	isChildes=500,
	isTalkbank,
	isOther
} ;

extern char OverWriteFile;
extern char Preserve_dir;
extern char isRecursive;

static void printStaticInfo(char isPrefix);

static int whichData;
static char curr_date[30];

FILE *tFP;

static void getCurTime(char *st) {
	time_t timer;
	struct tm *timeS;
	time(&timer);
	timeS = localtime(&timer);
	sprintf(curr_date, "%d-%s%d-%s%d", timeS->tm_year+1900,
			((timeS->tm_mon+1) < 10 ? "0":""), timeS->tm_mon+1,
			((timeS->tm_mday) < 10 ? "0":""), timeS->tm_mday);
}

void init(char f) {
	if (f) {
		curr_date[0] = EOS;
		OverWriteFile = TRUE;
		AddCEXExtension = "";
		stout = TRUE;
		combinput = TRUE;
		onlydata = 1;
		whichData = isChildes;
		isRecursive = TRUE;
	} else if (curr_date[0] == EOS) {
		getCurTime(curr_date);
	}
}

void usage() {
	printf("Usage: olac [d c t re] filename(s)\n");
	puts("+dS: specify date in YYYY-MM-DD.");
	puts("+c : working on childes database.");
	puts("+t : working on talkbank database.");
	puts("+re: run program recursively on all sub-directories.");
	puts("\nExample:");
	puts("    Working dir must be set to \"/web/childes/data-orig/\" on childes.talkbank.org");
	puts("\tolac -childes +d2010-10-06 -re 0metadata.cdc");
	puts("    Working dir must be set to \"/TalkBank/data-orig/\" on talkbank.org");
	puts("\tolac -talkbank +d2010-10-06 -re 0metadata.cdc");
	puts("    Working dir must be set to \"/TalkBank/resources/metamaker/form/metadata\" for other");
	puts("\tolac -other +d2010-10-06 -re 0metadata-*.cdc\n");
	puts("CHANGE DATE \"2010-10-06\", IN +d OPTION, TO CURRENT DATE");
	cutt_exit(0);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = OLAC_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	tFP = NULL;
	bmain(argc,argv,NULL);
	if (whichData == isChildes)
		fprintf(stderr, "Output file <%s>\n\n", "childes.xml");
	else if (whichData == isTalkbank)
		fprintf(stderr, "Output file <%s>\n\n", "talkbank.xml");
	else
		fprintf(stderr, "Output file <%s>\n\n", "other.xml");
	if (tFP != NULL) {
		printStaticInfo(FALSE);
		fclose(tFP);
	}
	printf("%c%c    Make sure that \"<sampleIdentifier>\" matches anyone \"<oai:identifier>\"%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
}

static char isHeadTerm(char *id, int i) {
	if (id[i] != ':')
		return(FALSE);
	id[i] = EOS;
	if (!strcmp(id, "Title")) {
	} else if (!strcmp(id, "Creator")) {
	} else if (!strcmp(id, "Subject")) {
	} else if (!strcmp(id, "Subject.olac:language")) {
	} else if (!strcmp(id, "Subject.olac:linguistic-field")) {
	} else if (!strcmp(id, "Subject.childes:participant")) {
	} else if (!strcmp(id, "Description")) {
	} else if (!strcmp(id, "Publisher")) {
	} else if (!strcmp(id, "Contributor")) {
	} else if (!strcmp(id, "accessRights")) {
	} else if (!strcmp(id, "Date")) {
	} else if (!strcmp(id, "Type")) {
	} else if (!strcmp(id, "Type.olac:linguistic-type")) {
	} else if (!strcmp(id, "Type.olac:discourse-type")) {
	} else if (!strcmp(id, "Format")) {
	} else if (!strcmp(id, "Identifier")) {
	} else if (!strcmp(id, "Language")) {
	} else if (!strcmp(id, "Relation")) {
	} else if (!strcmp(id, "Coverage")) {
	} else if (!strcmp(id, "Rights")) {
	} else if (!strcmp(id, "IMDI_Genre")) {
	} else if (!strcmp(id, "IMDI_Interactivity")) {
	} else if (!strcmp(id, "IMDI_PlanningType")) {
	} else if (!strcmp(id, "IMDI_Involvement")) {
	} else if (!strcmp(id, "IMDI_SocialContext")) {
	} else if (!strcmp(id, "IMDI_EventStructure")) {
	} else if (!strcmp(id, "IMDI_Channel")) {
	} else if (!strcmp(id, "IMDI_Task")) {
	} else if (!strcmp(id, "IMDI_Modalities")) {
	} else if (!strcmp(id, "IMDI_Subject")) {
	} else if (!strcmp(id, "IMDI_EthnicGroup")) {
	} else if (!strcmp(id, "IMDI_RecordingConditions")) {
	} else if (!strcmp(id, "IMDI_AccessAvailability")) {
	} else if (!strcmp(id, "IMDI_Continent")) {
	} else if (!strcmp(id, "IMDI_Country")) {
	} else if (!strcmp(id, "IMDI_WrittenResourceSubType")) {
	} else if (!strcmp(id, "IMDI_ProjectDescription")) {
	} else if (!strcmp(id, "IMDI_MediaFileDescription")) {
	} else {
		id[i] = ':';
		return(FALSE);
	}
	id[i] = ':';
	return(TRUE);
}

static void Choices(const char *h) {
	if (!strcmp(h, "Subject")) {
		fprintf(fpout,"Subject:\n");
		fprintf(fpout,"Subject.olac:language:\n");
		fprintf(fpout,"Subject.olac:linguistic-field:\n");
		fprintf(fpout,"Subject.childes:participant:\n");
	} else if (!strcmp(h, "Type")) {
		fprintf(fpout,"Type:\n");
		fprintf(fpout,"Type.olac:linguistic-type:\n");
		fprintf(fpout,"Type.olac:discourse-type:\n");
	} else {
		fprintf(fpout,"Contributor:\n");
		fprintf(fpout,"accessRights:\n");
		fprintf(fpout,"Coverage:\n");
		fprintf(fpout,"Creator:\n");
		fprintf(fpout,"Date:\n");
		fprintf(fpout,"Description:\n");
		fprintf(fpout,"Format:\n");
		fprintf(fpout,"Identifier:\n");
		fprintf(fpout,"Language:\n");
		fprintf(fpout,"Publisher:\n");
		fprintf(fpout,"Relation:\n");
		fprintf(fpout,"Rights:\n");
		fprintf(fpout,"Subject:\n");
		fprintf(fpout,"Subject.olac:language:\n");
		fprintf(fpout,"Subject.olac:linguistic-field:\n");
		fprintf(fpout,"Subject.childes:participant:\n");
		fprintf(fpout,"Title:\n");
		fprintf(fpout,"Type:\n");
		fprintf(fpout,"Type.olac:linguistic-type:\n");
		fprintf(fpout,"Type.olac:discourse-type:\n");
	}
	fprintf(fpout,"\n");
}

static void printOut(char *h, char *l, char *lang) {
	int  i;
	char *subH, *tl;

	subH = strchr(h, '.');
	if (subH != NULL) {
		*subH = EOS;
		subH++;
	}
	uS.remFrontAndBackBlanks(l);
	if (!strcmp(h, "Title")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<title>%s</title>\n", l);
	} else if (!strcmp(h, "Creator")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<creator>%s</creator>\n", l);
	} else if (!strcmp(h, "Contributor")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<contributor>%s</contributor>\n", l);
	} else if (!strcmp(h, "accessRights")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<dcterms:accessRights>%s</dcterms:accessRights>\n", l);
	} else if (!strcmp(h, "Subject")) {
		if (l[0] != EOS) {
			if (subH == NULL) {
				fprintf(tFP, "\t\t\t\t<subject>%s</subject>\n", l);
			} else {
				if (!strcmp(subH, "olac:language")) {
					strcpy(lang, l);
					if (l[0] != EOS)
						tl = l;
					else
						tl = NULL;
					if (tl != NULL) {
						subH = tl;
						do {
							subH = strchr(tl, ',');
							if (subH != NULL) {
								*subH = EOS;
								fprintf(tFP, "\t\t\t\t<subject xsi:type=\"olac:language\" olac:code=\"%s\"/>\n", tl);
								tl = subH + 1;
								while (isSpace(*tl))
									tl++;
							} else {
								fprintf(tFP, "\t\t\t\t<subject xsi:type=\"olac:language\" olac:code=\"%s\"/>\n", tl);
								break;
							}
						} while (*tl != EOS) ;
					}
				} else if (!strcmp(subH, "olac:linguistic-field")) {
					fprintf(tFP, "\t\t\t\t<subject xsi:type=\"olac:linguistic-field\" olac:code=\"%s\"/>\n", l);
				} else if (!strcmp(subH, "childes:participant")) {
					for (i=0; l[i] != EOS; i++) {
						if (l[i] == ',')
							strcpy(l+i, l+i+1);
					}
					fprintf(tFP, "\t\t\t\t<subject xsi:type=\"childes:participant\" %s/>\n", l);
				} else {
					fprintf(fpout,"*** File \"%s\": line %ld. Illegal field \"%s.%s\" found.\n", oldfname, lineno, h, subH);
					Choices(h);
				}
			}
		}
	} else if (!strcmp(h, "Description")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<description>%s</description>\n", l);
	} else if (!strcmp(h, "Publisher")) {
		if (l[0] != EOS)
			fprintf(tFP, "\t\t\t\t<publisher>%s</publisher>\n", l);
	} else if (!strcmp(h, "Date")) {
		if (l[0] != EOS)
//			fprintf(tFP, "\t\t\t\t<date xsi:type=\"W3CDTF\">%s</date>\n", l);
			fprintf(tFP, "\t\t\t\t<date>%s</date>\n", l);
	} else if (!strcmp(h, "Type")) {
		if (l[0] != EOS) {
			if (subH == NULL) {
				if (uS.mStricmp(l, "text") == 0)
					fprintf(tFP, "\t\t\t\t<type xsi:type=\"dcterms:DCMIType\">Text</type>\n");
				else if (uS.mStricmp(l, "sound") == 0)
					fprintf(tFP, "\t\t\t\t<type xsi:type=\"dcterms:DCMIType\">Sound</type>\n");
				else if (uS.mStricmp(l, "movingimage") == 0)
					fprintf(tFP, "\t\t\t\t<type xsi:type=\"dcterms:DCMIType\">MovingImage</type>\n");
				else
					fprintf(tFP, "\t\t\t\t<type>%s</type>\n", l);
			} else {
				if (!strcmp(subH, "olac:linguistic-type")) {
					fprintf(tFP, "\t\t\t\t<type xsi:type=\"olac:linguistic-type\" olac:code=\"%s\"/>\n", l);
				} else if (!strcmp(subH, "olac:discourse-type")) {
					fprintf(tFP, "\t\t\t\t<type xsi:type=\"olac:discourse-type\" olac:code=\"%s\"/>\n", l);
				} else {
					fprintf(fpout,"*** File \"%s\": line %ld. Illegal field \"%s.%s\" found.\n", oldfname, lineno, h, subH);
					Choices(h);
				}
			}
		}
	} else if (!strcmp(h, "Format")) {
	} else if (!strcmp(h, "Identifier")) {
		for (i=0; isSpace(l[i]); i++) ;
		if (l[i] != EOS) {
			fprintf(tFP, "\t\t\t\t<identifier>%s</identifier>\n", l+i);
		}

		i = strlen(wd_dir);
		if (oldfname[i] == PATHDELIMCHR)
			i++;
		uS.FNType2str(templineC1, 0L, oldfname+i);
		for (i=strlen(templineC1)-1; i >= 0 && templineC1[i] != PATHDELIMCHR; i--) ;
		if (templineC1[i] == PATHDELIMCHR && uS.mStrnicmp(templineC1+i+1, "0metadata-", 10) != 0) {
			templineC1[i] = EOS;
			for (i=0; templineC1[i] != EOS; i++) {
				if (templineC1[i] == PATHDELIMCHR)
					templineC1[i] = '/';
			}
			if (whichData == isChildes)
				sprintf(templineC, "http://childes.talkbank.org/data-xml/%s.zip", templineC1);
			else if (whichData == isTalkbank)
				sprintf(templineC, "http://talkbank.org/data-xml/%s.zip", templineC1);
			else
				sprintf(templineC, "http://talkbank.org/data-xml/%s.zip", templineC1);
			fprintf(tFP, "\t\t\t\t<identifier xsi:type=\"dcterms:URI\">%s</identifier>\n", templineC);
			if (whichData == isChildes)
				sprintf(templineC, "http://childes.talkbank.org/data/%s.zip", templineC1);
			else if (whichData == isTalkbank)
				sprintf(templineC, "http://talkbank.org/data/%s.zip", templineC1);
			else
				sprintf(templineC, "http://talkbank.org/data/%s.zip", templineC1);
			fprintf(tFP, "\t\t\t\t<identifier xsi:type=\"dcterms:URI\">%s</identifier>\n", templineC);
		}
	} else if (!strcmp(h, "Language")) {
		if (l[0] != EOS)
			tl = l;
		else if (lang[0] != EOS)
			tl = lang;
		else
			tl = NULL;
		if (tl != NULL) {
			subH = tl;
			do {
				subH = strchr(tl, ',');
				if (subH != NULL) {
					*subH = EOS;
					fprintf(tFP, "\t\t\t\t<language xsi:type=\"olac:language\" olac:code=\"%s\"/>\n", tl);
					tl = subH + 1;
					while (isSpace(*tl))
						tl++;
				} else {
					fprintf(tFP, "\t\t\t\t<language xsi:type=\"olac:language\" olac:code=\"%s\"/>\n", tl);
					break;
				}
			} while (*tl != EOS) ;
		}
	} else if (!strcmp(h, "Relation")) {
/*
		if (l[0] != EOS) {
			fprintf(tFP, "\t\t\t\t<relation xsi:type=\"dcterms:URI\">%s</relation>\n", l);
		} else {
			i = strlen(wd_dir);
			strcpy(templineC1, oldfname+i);
			for (i=strlen(templineC1)-1; i >= 0 && templineC1[i] != PATHDELIMCHR; i--) ;
			if (templineC1[i] == PATHDELIMCHR && uS.mStrnicmp(templineC1+i+1, "0metadata-", 10) != 0) {
				templineC1[i] = EOS;
				for (i=0; templineC1[i] != EOS; i++) {
					if (templineC1[i] == PATHDELIMCHR)
						templineC1[i] = '/';
				}
				if (whichData == isChildes)
					sprintf(templineC, "http://childes.talkbank.org/data-xml/%s.zip", templineC1);
				else if (whichData == isTalkbank)
					sprintf(templineC, "http://talkbank.org/data-xml/%s.zip", templineC1);
				else
					sprintf(templineC, "http://talkbank.org/data-xml/%s.zip", templineC1);
				fprintf(tFP, "\t\t\t\t<relation xsi:type=\"dcterms:URI\">%s</relation>\n", templineC);
				if (whichData == isChildes)
					sprintf(templineC, "http://childes.talkbank.org/data/%s.zip", templineC1);
				else if (whichData == isTalkbank)
					sprintf(templineC, "http://talkbank.org/data/%s.zip", templineC1);
				else
					sprintf(templineC, "http://talkbank.org/data/%s.zip", templineC1);
				fprintf(tFP, "\t\t\t\t<relation xsi:type=\"dcterms:URI\">%s</relation>\n", templineC);
			}
		}
*/
	} else if (!strcmp(h, "Coverage")) {
	} else if (!strcmp(h, "Rights")) {
	} else if (!strcmp(h, "IMDI_Genre")) {
	} else if (!strcmp(h, "IMDI_Interactivity")) {
	} else if (!strcmp(h, "IMDI_PlanningType")) {
	} else if (!strcmp(h, "IMDI_Involvement")) {
	} else if (!strcmp(h, "IMDI_SocialContext")) {
	} else if (!strcmp(h, "IMDI_EventStructure")) {
	} else if (!strcmp(h, "IMDI_Channel")) {
	} else if (!strcmp(h, "IMDI_Task")) {
	} else if (!strcmp(h, "IMDI_Modalities")) {
	} else if (!strcmp(h, "IMDI_Subject")) {
	} else if (!strcmp(h, "IMDI_EthnicGroup")) {
	} else if (!strcmp(h, "IMDI_RecordingConditions")) {
	} else if (!strcmp(h, "IMDI_AccessAvailability")) {
	} else if (!strcmp(h, "IMDI_Continent")) {
	} else if (!strcmp(h, "IMDI_Country")) {
	} else if (!strcmp(h, "IMDI_WrittenResourceSubType")) {
	} else if (!strcmp(h, "IMDI_ProjectDescription")) {
	} else if (!strcmp(h, "IMDI_MediaFileDescription")) {
	} else if (h[0] != EOS) {
		fprintf(fpout,"*** File \"%s\": line %ld. Illegal field \"%s\" found.\n", oldfname, lineno, h);
		Choices("");
	}
}

void call() {
	long i;
	char lang[256];

	if (tFP == NULL) {
		if (Preserve_dir)
			strcpy(FileName1, wd_dir);
		else
			strcpy(FileName1, od_dir);
		if (whichData == isChildes)
			addFilename2Path(FileName1, "childes.xml");
		else if (whichData == isTalkbank)
			addFilename2Path(FileName1, "talkbank.xml");
		else
			addFilename2Path(FileName1, "other.xml");
		tFP = fopen(FileName1,"w");
		if (tFP == NULL) {
			fprintf(stderr, "Can't open output file \"%s\".\n", FileName1);
			cutt_exit(0);
		}
		i = strlen(wd_dir);
		uS.FNType2str(templineC1, 0L, oldfname+i);
		for (i=strlen(templineC1)-1; i >= 0 && templineC1[i] != PATHDELIMCHR; i--) ;
		if (templineC1[i] == PATHDELIMCHR) {
			if (uS.mStrnicmp(templineC1+i+1, "0metadata-", 10) == 0) {
				strcpy(templineC1, templineC1+i+11);
				for (i=0; templineC1[i] != EOS && templineC1[i] != '.'; i++) ;
				if (templineC1[i] == '.')
					templineC1[i] = EOS;
			} else {
				templineC1[i] = EOS;
				if (templineC1[0] == PATHDELIMCHR)
					strcpy(templineC1, templineC1+1);
			}
			for (i=0; templineC1[i] != EOS; i++) {
				if (templineC1[i] == PATHDELIMCHR)
					templineC1[i] = '-';
			}
		} else {
			if (templineC1[0] == PATHDELIMCHR)
				strcpy(templineC1, templineC1+1);
		}
		printStaticInfo(TRUE);
	}

	lang[0] = EOS;
	utterance->speaker[0] = EOS;
	utterance->line[0] = EOS;
	fprintf(tFP, "    <oai:record>\n");
	fprintf(tFP, "        <oai:header>\n");
	i = strlen(wd_dir);
	uS.FNType2str(templineC1, 0L, oldfname+i);
	for (i=strlen(templineC1)-1; i >= 0 && templineC1[i] != PATHDELIMCHR; i--) ;
	if (templineC1[i] == PATHDELIMCHR) {
		if (uS.mStrnicmp(templineC1+i+1, "0metadata-", 10) == 0) {
			strcpy(templineC1, templineC1+i+11);
			for (i=0; templineC1[i] != EOS && templineC1[i] != '.'; i++) ;
			if (templineC1[i] == '.')
				templineC1[i] = EOS;
		} else {
			templineC1[i] = EOS;
			if (templineC1[0] == PATHDELIMCHR)
				strcpy(templineC1, templineC1+1);
		}
		for (i=0; templineC1[i] != EOS; i++) {
			if (templineC1[i] == PATHDELIMCHR)
				templineC1[i] = '-';
		}
		if (whichData == isChildes)
			fprintf(tFP, "			<oai:identifier>oai:childes.talkbank.org:%s</oai:identifier>\n", templineC1);
		else if (whichData == isTalkbank)
			fprintf(tFP, "			<oai:identifier>oai:talkbank.org:%s</oai:identifier>\n", templineC1);
		else
			fprintf(tFP, "			<oai:identifier>oai:independent.talkbank.org:%s</oai:identifier>\n", templineC1);
	} else {
		if (templineC1[0] == PATHDELIMCHR)
			strcpy(templineC1, templineC1+1);
		if (whichData == isChildes)
			fprintf(tFP, "			<oai:identifier>oai:childes.talkbank.org:%s</oai:identifier>\n", templineC1);
		else if (whichData == isTalkbank)
			fprintf(tFP, "			<oai:identifier>oai:talkbank.org:%s</oai:identifier>\n", templineC1);
		else
			fprintf(tFP, "			<oai:identifier>oai:independent.talkbank.org:%s</oai:identifier>\n", templineC1);
	}

	fprintf(tFP, "\
			<oai:datestamp>%s</oai:datestamp>\n\
        </oai:header>\n\
        <oai:metadata>\n", curr_date);
    fprintf(tFP, "\t\t\t<olac:olac xmlns=\"http://purl.org/dc/elements/1.1/\">\n");

	rewind(fpin);
	utterance->speaker[0] = EOS;
	while (fgets_cr(uttline, UTTLINELEN, fpin)) {
		if (uS.isUTF8(uttline) || uS.partcmp(uttline, FONTHEADER, FALSE, FALSE))
			continue;
		if (!strcmp(uttline,"\n"))
			continue;
		if (uttline[0] == '%')
			break;
		uS.remblanks(uttline);
		for (i=0; uttline[i] != EOS && !isHeadTerm(uttline, i); i++) ;
		if (uttline[i] == ':') {
			printOut(utterance->speaker, utterance->line, lang);
			uttline[i] = EOS;
			strcpy(utterance->speaker, uttline);
			for (i=i+1; isSpace(uttline[i]); i++) ;
			strcpy(utterance->line, uttline+i);
		} else {
			strcat(utterance->line, uttline);
		}
	}
	if (utterance->speaker[0] != EOS)
		printOut(utterance->speaker, utterance->line, lang);
    fprintf(tFP, "\t\t\t</olac:olac>\n");
	fprintf(tFP, "        </oai:metadata>\n");
	fprintf(tFP, "    </oai:record>\n\n");
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				whichData = isChildes;
				break;
		case 't':
				whichData = isTalkbank;
				break;
		case 'o':
				whichData = isOther;
				break;
		case 'd':
				strncpy(curr_date, f, 29);
				curr_date[29] = EOS;
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

static void printStaticInfo(char isPrefix) {
	if (isPrefix) {
		fprintf(tFP, "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<Repository xmlns=\"http://www.openarchives.org/OAI/2.0/static-repository\" \n\
            xmlns:oai=\"http://www.openarchives.org/OAI/2.0/\" \n\
            xmlns:olac=\"http://www.language-archives.org/OLAC/1.1/\"\n");
		if (whichData == isChildes)
			fprintf(tFP, "            xmlns:childes=\"http://childes.talkbank.org/\"\n");
		else if (whichData == isTalkbank)
			fprintf(tFP, "            xmlns:talkbank=\"http://talkbank.org/\"\n");
		else
			fprintf(tFP, "            xmlns:talkbank=\"http://talkbank.org/\"\n");
		fprintf(tFP, "\
            xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n\
            xmlns:dcterms=\"http://purl.org/dc/terms/\"\n\
            xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \n\
            xsi:schemaLocation=\"http://www.openarchives.org/OAI/2.0/static-repository \n\
                                http://www.language-archives.org/OLAC/1.1/static-repository.xsd\n\
                                http://www.language-archives.org/OLAC/1.1/\n\
                                http://www.language-archives.org/OLAC/1.1/olac.xsd\n");
		if (whichData == isChildes) {
			fprintf(tFP, "\
                                http://childes.talkbank.org/\n\
                                http://childes.talkbank.org/childes.xsd\n");
		} else if (whichData == isTalkbank) {
			fprintf(tFP, "\
                                http://talkbank.org/\n\
                                http://talkbank.org/talkbank.xsd\n");
		} else {
			fprintf(tFP, "\
                                http://talkbank.org/\n\
                                http://talkbank.org/talkbank.xsd\n");
		}
		fprintf(tFP, "\
                                http://purl.org/dc/elements/1.1/\n\
                                http://www.language-archives.org/OLAC/1.1/dc.xsd\n\
                                http://purl.org/dc/terms/\n\
                                http://www.language-archives.org/OLAC/1.1/dcterms.xsd\">\n\
\n\
  <!-- This document is valid according to both of the following schemas:\n\
       http://www.openarchives.org/OAI/2.0/static-repository.xsd\n\
       http://www.language-archives.org/OLAC/1.1/static-repository.xsd\n\
  -->\n\
\n\
  <Identify>\n");
		if (whichData == isChildes) {
			fprintf(tFP, "    <oai:repositoryName>CHILDES Data repository</oai:repositoryName>\n");
			fprintf(tFP, "    <oai:baseURL>http://childes.talkbank.org/childes.xml</oai:baseURL>\n");
		} else if (whichData == isTalkbank) {
			fprintf(tFP, "    <oai:repositoryName>TALKBANK Data repository</oai:repositoryName>\n");
			fprintf(tFP, "    <oai:baseURL>http://talkbank.org/talkbank.xml</oai:baseURL>\n");
		} else {
			fprintf(tFP, "    <oai:repositoryName>Independent Corpora</oai:repositoryName>\n");
			fprintf(tFP, "    <oai:baseURL>http://talkbank.org/resources/metamaker/other.xml</oai:baseURL>\n");
		}
		fprintf(tFP, "\
    <oai:protocolVersion>2.0</oai:protocolVersion>\n\
    <oai:adminEmail>macw@cmu.edu</oai:adminEmail>\n\
    <oai:earliestDatestamp>2002-09-19</oai:earliestDatestamp>\n\
    <oai:deletedRecord>no</oai:deletedRecord>\n\
    <oai:granularity>YYYY-MM-DD</oai:granularity>\n\
    <oai:description>\n\
      <oai-identifier xmlns=\"http://www.openarchives.org/OAI/2.0/oai-identifier\"\n\
         xsi:schemaLocation=\"http://www.openarchives.org/OAI/2.0/oai-identifier http://www.language-archives.org/OLAC/1.1/oai-identifier.xsd\">\n\
        <scheme>oai</scheme>\n");
		if (whichData == isChildes) {
			fprintf(tFP, "\
        <repositoryIdentifier>childes.talkbank.org</repositoryIdentifier>\n\
        <delimiter>:</delimiter>\n\
        <sampleIdentifier>oai:childes.talkbank.org:%s</sampleIdentifier>\n", templineC1);
        } else if (whichData == isTalkbank) {
			fprintf(tFP, "\
        <repositoryIdentifier>talkbank.org</repositoryIdentifier>\n\
        <delimiter>:</delimiter>\n\
        <sampleIdentifier>oai:talkbank.org:%s</sampleIdentifier>\n", templineC1);
		} else {
			fprintf(tFP, "\
		<repositoryIdentifier>independent.talkbank.org</repositoryIdentifier>\n\
		<delimiter>:</delimiter>\n\
		<sampleIdentifier>oai:independent.talkbank.org:%s</sampleIdentifier>\n", templineC1);
		}
		fprintf(tFP, "\
      </oai-identifier>\n\
    </oai:description>\n\
\n\
    <oai:description>\n\
      <olac-archive\n\
          currentAsOf=\"%s\"\n\
          type=\"institutional\"\n\
          xmlns=\"http://www.language-archives.org/OLAC/1.1/olac-archive\"\n\
          xsi:schemaLocation=\"http://www.language-archives.org/OLAC/1.1/olac-archive http://www.language-archives.org/OLAC/1.1/olac-archive.xsd\">\n",
		curr_date);
		if (whichData == isChildes)
			fprintf(tFP, "        <archiveURL>http://childes.talkbank.org/</archiveURL>\n");
		else if (whichData == isTalkbank)
			fprintf(tFP, "        <archiveURL>http://talkbank.org/</archiveURL>\n");
		else
			fprintf(tFP, "        <archiveURL>http://talkbank.org/resources/metamaker/</archiveURL>\n");
		fprintf(tFP, "\
        <participant name=\"Brian MacWhinney\" role=\"Organizer\" email=\"macw@cmu.edu\"/>\n\
        <institution>Carnegie Mellon University</institution>\n\
        <institutionURL>http://www.cmu.edu</institutionURL>\n\
        <shortLocation>Pittsburgh, USA</shortLocation>\n\
        <location>5000 Forbes ave., Pittsburgh, PA 15213, USA</location>\n");
		if (whichData == isOther)
			fprintf(tFP, "        <synopsis>This is a service for personalized corpus announcement provided by the TalkBank server. These corpora that are not yet contributed to other major repositories. The corpora will include a wide range of materials of all types.</synopsis>\n");
		else
			fprintf(tFP, "        <synopsis>%s is an international system for the exchange of transcripts of communicative interactions linked to audio or video media.</synopsis>\n", ((whichData == isChildes) ? "CHILDES" : "TalkBank"));
		fprintf(tFP, "        <access>public.</access>\n");
		if (whichData != isOther)
			fprintf(tFP, "        <archivalSubmissionPolicy>CHILDES and TalkBank happily accept archival submissions of transcript corpora of conversational interactions.  Interactions involving young children are included in CHILDES.  Interactions involving older children and adults are included in TalkBank.  Corpora are further divided into collections by language and interest areas (aphasia, bilingualism, classroom, legal, etc.).  All corpora must eventually be entered in CHAT format and must pass through XML validation of this format.  In many cases, we are willing to work with contributors to reformat their data into CHAT.  Because this formatting can involve significant work on our part, we place a priority on contributions that include audio or video materials.  However, we are interested in including non-linked materials and materials without media when the corpora themselves are intrinsically interesting.  Guidelines for contribution can be found at http://talkbank.org/share.</archivalSubmissionPolicy>\n");
//		else
//			fprintf(tFP, "        <archivalSubmissionPolicy>This database is used to allow researchers to declare the existence of corpora that are not yet contributed to other major repositories. The overall database will include a wide range of materials of all types.</archivalSubmissionPolicy>\n");
		fprintf(tFP, "\
      </olac-archive>\n\
    </oai:description>\n\
  </Identify>\n\
\n\
  <ListMetadataFormats>\n\
    <oai:metadataFormat>\n\
      <oai:metadataPrefix>olac</oai:metadataPrefix>\n\
      <oai:schema>http://www.language-archives.org/OLAC/1.1/olac.xsd</oai:schema>\n\
      <oai:metadataNamespace>http://www.language-archives.org/OLAC/1.1/</oai:metadataNamespace>\n\
    </oai:metadataFormat>\n\
  </ListMetadataFormats>\n\
  <ListRecords metadataPrefix=\"olac\">\n\n\n");
	} else {
	  fprintf(tFP, "\n\
  </ListRecords>\n\
</Repository>\n");
	}
}
