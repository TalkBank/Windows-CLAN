#include "cu.h"
#include "ced.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include <wchar.h>
#include "ChatToXML.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 4

#if defined(_MAC_CODE)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */


char errFound;
FILE *errfp;
char uLanguage[5];
char *chatSt;

extern struct tier *defheadtier;
extern char LeaveSlashes;
extern char OverWriteFile;
extern char *options_ext[];
extern char Parans;

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		options_ext[TEMP] = ".xml";
		LeaveSlashes = '\001';
		Parans = 2;
		uLanguage[0] = EOS;
		chatSt = NULL;
	}
}

void usage() {
	printf("Usage: tCHATtoXML [l %s] filename(s)\n",mainflgs());
	puts("+l : specify missing language");
	puts("     For example:");
	puts("       \"-len\" for English");
	puts("       \"-lde\" for German");
	puts("       \"-lja\" for Japanese");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	errFound = FALSE;
	errfp = fopen("errors.txt", "w");
#ifdef _MAC_CODE
	if (errfp != NULL)
		settyp("errors.txt", 'TEXT', the_file_creator.out, FALSE);
#endif
	bmain(argc,argv,NULL);
	if (errFound) {
		fprintf(stderr, "\nSome errors found. ");
		if (errfp != NULL)
			fprintf(stderr, "Please check file \"%s\".\n", "errors.txt");
	}
	if (errfp != NULL)
		fclose(errfp);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'l':
			strcpy(uLanguage, f);
			uS.uppercasestr(uLanguage, dFnt.Encod, C_MBF);
			if (uLanguage[0] == EOS) {
				strncpy(uLanguage, f, 4);
				uLanguage[4] = EOS;
				uS.lowercasestr(uLanguage, dFnt.Encod, C_MBF);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}


void call() {
	long ln;
	int  c;
	short *chatS, *ts, *message;
	char  *tc, UEncoding;

	fclose(fpout);
	fpout = fopen(newfname, "wb");
	fclose(fpin);
	fpin = fopen(oldfname, "rb");

	if (fseek(fpin, 0L, SEEK_END) != 0) {
		fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
		cutt_exit(0);
	}
	ln = ftell(fpin);
	if (fseek(fpin, 0L, SEEK_SET) != 0) {
		fprintf(stderr, "Can't get the size of a file: %s\n", oldfname);
		cutt_exit(0);
	}
	c = 0;
	if (!feof(fpin))
		c = getc(fpin);
	if (c == (int)0xFF || c == (int)0xFE) {
		if (c == (int)0xFF)
			UEncoding = UPC;
		else
			UEncoding = UMAC;
		chatS = (short *)malloc(ln+4);
		if (chatS == NULL) {
			fprintf(stderr, "Out of memory\n");
			cutt_exit(0);
		}
		if (!feof(fpin))
			c = getc(fpin);
#ifdef _WIN32 
		ts = chatS;
		while (!feof(fpin)) {
			c = getwc(fpin);
			if (c == WEOF)
				break;
			*ts++ = (short)c;
		}
		*ts = 0;
#else
		tc = (char *)chatS;
		while (!feof(fpin)) {
			c = getc(fpin);
			*tc++ = (char)c;
		}
		*tc++ = 0;
		*tc = 0;
#endif
	} else {
		UEncoding = UNOT;
		chatS = (short *)malloc((ln+2)*2);
		if (chatS == NULL) {
			fprintf(stderr, "Out of memory\n");
			cutt_exit(0);
		}
		ts = (short *)chatS;
		c = c << 8;
		*ts++ = (short)c;
		while (!feof(fpin)) {
			c = getc(fpin);
			c = c << 8;
			*ts++ = (short)c;
		}
		*ts = 0;
	}
/*
//UDEFAULT
	int errnum;
	if ((errnum=checkChat(chatS, oldfname, UEncoding)) != 0) {
		message = ChatError(errnum, &ln);
		fprintf(stderr, "Error: line %ld:\n", ln);
		for (ln=0L; message[ln] != 0; ln++) {
			tc = (char *)&message[ln];
#ifdef _WIN32 
			fputc(tc[0], stderr);
#else
			fputc(tc[1], stderr);
#endif
		}
		fprintf(stderr, "\n");
		free(message);
		cutt_exit(0);
	}

	tc = transformChatToXML(chatS, oldfname, UEncoding);
	if (UEncoding == UNOT) {
	} else if (UEncoding == UPC) {
		fputs("ÿþ", fpout);
	} else if (UEncoding == UMAC) {
		fputs("þÿ", fpout);
	}
	for (ln=0; 1; ln+=2) {
		if (tc[ln] == 0 && tc[ln+1] == 0)
			break;
		fputc(tc[ln], fpout);
		fputc(tc[ln+1], fpout);
	}
*/

	if ((tc=ChatToXML(chatS, false, oldfname, uLanguage, UEncoding, &errFound, errfp)) == NULL) {
		if (errfp != NULL)
			fclose(errfp);
		message = ChatError(-1, &ln);
		fprintf(stderr, "Error: line %ld:\n", ln);
		for (ln=0L; message[ln] != 0; ln++) {
			tc = (char *)&message[ln];
#ifdef _WIN32 
			fputc(tc[0], stderr);
#else
			fputc(tc[1], stderr);
#endif
		}
		fprintf(stderr, "\n");
		free(message);
		cutt_exit(0);
	} else {
		if (UEncoding == UNOT) {
		} else if (UEncoding == UPC) {
			fputs("ÿþ", fpout);
		} else if (UEncoding == UMAC) {
			fputs("þÿ", fpout);
		}
		for (ln=0; 1; ln+=2) {
			if (tc[ln] == 0 && tc[ln+1] == 0)
				break;
			if (UEncoding == UNOT) {
				fputc(tc[ln+1], fpout);
			} else {
				fputc(tc[ln], fpout);
				fputc(tc[ln+1], fpout);
			}
		}
		free(tc);
	}
}
