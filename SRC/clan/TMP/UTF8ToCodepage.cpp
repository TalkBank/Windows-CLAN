/**********************************************************************
	"Copyright 2006 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "ced.h"
#ifdef _WIN32 
	#include "stdafx.h"
#else
	#define UINT unsigned int
#endif

#if !defined(UNX)
#define _main utftocp_main
#define call utftocp_call
#define getflag utftocp_getflag
#define init utftocp_init
#define usage utftocp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

extern char LeaveSlashes;
extern char *options_ext[];
extern char Parans;
extern char AddCEXExtension;

static UINT myCodePage;

void init(char f) {
	if (f) {
		myCodePage = 0;
		onlydata = 1;
		stout = FALSE;
		AddCEXExtension = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		options_ext[UTF2CP] = ".cha";
		LeaveSlashes = '\001';
		Parans = 2;
	} else {
		if (myCodePage == 0) {
			fprintf(stderr, "Please specify codepage number with +c option.\n");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("Usage: utf2cp [c %s] filename(s)\n",mainflgs());
	puts("For example: to convert UTF8 to Korean Hangeul, type:");
	puts("     utf2cp -c949 *.txt");
	puts("+cN: specifies codepage number");
	puts("     949 - Korean Unified Hangeul");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = UTF2CP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
#ifdef _MAC_CODE
	fprintf(stderr, "This program works only on Windows machines.\n");
	return;
#elif !defined(_WIN32)
	fprintf(stderr, "This program works only on Windows machines.\n");
	return(0);
#endif
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {
	UINT cp;

	f++;
	switch(*f++) {
		case 'c':
			cp = (UINT)atoi(f);
			if (cp == 949)
				myCodePage = cp;
			else {
				fprintf(stderr, "Unrecognized codepage number.\n");
				cutt_exit(0);
			}
			break;
//		myCodePage = 949; // Korean
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call() {
#ifdef _WIN32 
	fclose(fpin);
	if (stout == FALSE) {
		fclose(fpout);
		fpout = fopen(newfname, "wb");
	}
	fpin = fopen(oldfname, "rb");
	if (fpin == NULL) {
		fprintf(stderr,"Can't open file %s.\n",oldfname);
		cutt_exit(0);
	}
	if (fpout == NULL) {
		fprintf(stderr,"Can't open output file %s.\n",newfname);
		cutt_exit(0);
	}
	unsigned long total;
	if (fseek(fpin, 0L, SEEK_END) != 0) {
		fprintf(stderr, "Error determining the length of a file\n");
		return;
	}
	total = (unsigned long)ftell(fpin);
	if (fseek(fpin, 0L, SEEK_SET) != 0) {
		fprintf(stderr, "Error going to the beginning of a file\n");
		return;
	}
	LPVOID hText;
	hText = LocalAlloc(LMEM_MOVEABLE, total+2L);
	if (hText == NULL) {
		fprintf(stderr, "Out of memory\n");
		return;
	}
	LPSTR pText = (LPSTR)LocalLock(hText);
	LPSTR t;
	ASSERT(pText != NULL);
	t = pText;
	unsigned long ltotal = 0L;
	while (!feof(fpin)) {
		*t++ = getc(fpin);
		ltotal++;
	}
	*t = EOS;
	if (ltotal != total+1) {
		fprintf(stderr, "Internal error, totals not equal.\n");
	}
	unsigned short *puText=NULL;
	int totalw=0;
	totalw=MultiByteToWideChar(CP_UTF8,0,pText,total,NULL,0);
	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, (totalw+1L)*2);
	puText = (unsigned short *)LocalLock(hwText);
	MultiByteToWideChar(CP_UTF8,0,pText,total,puText,totalw);
	LocalUnlock(hText);
	LocalFree(hText);

	int convChars=WideCharToMultiByte(myCodePage,0,puText,totalw,NULL,0,NULL,NULL);
	hText = LocalAlloc(LMEM_MOVEABLE, convChars);
	LPSTR convText = (LPSTR)LocalLock(hText);
	WideCharToMultiByte(myCodePage,0,puText,totalw,convText,convChars,NULL,NULL);
	LocalUnlock(hwText);
	LocalFree(hwText);

	if (myCodePage == 949)
		fprintf(fpout, "%s\tWin95:Arial Unicode MS:-12:129\r\n", FONTHEADER);
	t = convText;
	while (convChars > 0) {
		putc(*t, fpout);
		t++;
		convChars--;
	}

	LocalUnlock(hText);
	LocalFree(hText);
#endif
}
