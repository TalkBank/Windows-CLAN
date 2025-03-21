#include "ced.h"
#include "cu.h"
#include "MMedia.h"

#ifdef _WIN32
	extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);
	extern char isOverRidePicName;
#endif

static void convertToURLText(char *to, char *from) {
	int ito, ifr, i;
	char hex[50];

	ito = 0;
	ifr = 0;
	while (from[ifr] != EOS) {
		if (isalnum(from[ifr]) || from[ifr] == '-' || from[ifr] == '_') {
			to[ito++] = from[ifr++];
		} else if (!strncmp(from+ifr, "&graText=", 9)) {
			for (i=0; i < 9; i++)
				to[ito++] = from[ifr++];
		} else if (!strncmp(from+ifr, "&mainText=", 10)) {
			for (i=0; i < 10; i++)
				to[ito++] = from[ifr++];
		} else { 
			to[ito++] = '%';
			sprintf(hex, "%x", (unsigned char)from[ifr++]);
			to[ito++] = hex[0];
			to[ito++] = hex[1];
		}
	}
	to[ito] = EOS;
}

char ShowGRA(char *graSt, char *morSt) {
	int  i;
	int  j;
	char hf, jpgTag[20], errMess[512];
	FILE *fp;
	ROWS *tr, *grt, *gra, *trn, *mor, *msp;
	extern FNType prefsDir[];

	ChangeCurLineAlways(0);
	msp = NULL;
	grt = NULL;
	gra = NULL;
	trn = NULL;
	mor = NULL;
	tr = global_df->row_txt;
	if (isSpace(tr->line[0])) {
		while (!isSpeaker(tr->line[0]) && !AtTopEnd(tr, global_df->head_text, FALSE)) {
			tr = ToPrevRow(tr, FALSE);
		}
	}
	if (uS.partcmp(tr->line, "%grt:", FALSE, FALSE))
		grt = tr;
	else if (uS.partcmp(tr->line, graSt, FALSE, FALSE))
		gra = tr;
	while (!isMainSpeaker(tr->line[0]) && tr->line[0] != (unCH)'@'  && !AtTopEnd(tr, global_df->head_text, FALSE)) {
		if (uS.partcmp(tr->line, morSt, FALSE, FALSE))
			mor = tr;
		else if (uS.partcmp(tr->line, "%trn:", FALSE, FALSE))
			trn = tr;
		else if (tr->line[0] == '*')
			msp = tr;
		tr = ToPrevRow(tr, FALSE);
	}
	if (tr->line[0] == '*')
		msp = tr;
	tr = global_df->row_txt;
	while (!isMainSpeaker(tr->line[0]) && tr->line[0] != (unCH)'@' && !AtBotEnd(tr, global_df->tail_text, FALSE)) {
		if (uS.partcmp(tr->line, morSt, FALSE, FALSE))
			mor = tr;
		else if (uS.partcmp(tr->line, "%trn:", FALSE, FALSE))
			trn = tr;
		tr = ToNextRow(tr, FALSE);
	}
	if (grt != NULL)
		tr = grt;
	else if (gra != NULL)
		tr = gra;
	else {
		sprintf(errMess, "Can't find %%grt: or %s tier.", graSt);
		do_warning(errMess, -1);
		return(FALSE);
	}
	if (trn == NULL && mor == NULL) {
		sprintf(errMess, "Can't find %%trn: or %s tier.", morSt);
		do_warning(errMess, -1);
		return(FALSE);
	}
	j = 0;
	do {
		i = 0;
		if (!isSpace(tr->line[i])) {
			for (; tr->line[i] != ':' && tr->line[i] != EOS; i++) ;
			if (tr->line[i] == ':')
				for (i++; isSpace(tr->line[i]); i++) ;
		}
		hf = FALSE;
		for (; tr->line[i] != EOS; i++) {
			if (tr->line[i] == HIDEN_C)
				hf = !hf;
			else if (!hf && tr->line[i] != NL_C && tr->line[i] != SNL_C && tr->line[i] != '\n') {
				if (tr->line[i] == '\t')
					templine[j++] = ' ';
				else
					templine[j++] = tr->line[i];
			}
		}
		tr = ToNextRow(tr, FALSE);
	} while (!isSpeaker(tr->line[0]) && !AtBotEnd(tr, global_df->tail_text, FALSE)) ;
	templine[j] = EOS;
	UnicodeToUTF8(templine, j, (unsigned char *)templineC, NULL, UTTLINELEN);
	if (trn != NULL && grt != NULL)
		tr = trn;
	else if (mor != NULL && grt != NULL)
		tr = mor;
	else if (mor != NULL && gra != NULL)
		tr = mor;
	else if (trn != NULL && gra != NULL)
		tr = trn;
	else {
		sprintf(errMess, "Can't find %%trn: or %s tier.", morSt);
		do_warning(errMess, -1);
		return(FALSE);
	}
	j = 0;
	do {
		i = 0;
		if (!isSpace(tr->line[0])) {
			for (; tr->line[i] != ':' && tr->line[i] != EOS; i++) ;
			if (tr->line[i] == ':')
				for (i++; isSpace(tr->line[i]); i++) ;
		}
		hf = FALSE;
		for (; tr->line[i] != EOS; i++) {
			if (tr->line[i] == HIDEN_C)
				hf = !hf;
			else if (!hf && tr->line[i] != NL_C && tr->line[i] != SNL_C && tr->line[i] != '\n') {
				if (tr->line[i] == '\t')
					templine2[j++] = ' ';
				else
					templine2[j++] = tr->line[i];
			}
		}
		tr = ToNextRow(tr, FALSE);
	} while (!isSpeaker(tr->line[0]) && !AtBotEnd(tr, global_df->tail_text, FALSE)) ;
	templine2[j] = EOS;
	UnicodeToUTF8(templine2, j, (unsigned char *)templineC2, NULL, UTTLINELEN);
	if (msp == NULL) {
		strcpy(templineC3, "*NO MAIN SPEAKER FOUND*");
	} else {
		tr = msp;
		j = 0;
		do {
			i = 0;
			if (!isSpace(tr->line[0])) {
				for (; tr->line[i] != ':' && tr->line[i] != EOS; i++) ;
				if (tr->line[i] == ':')
					for (i++; isSpace(tr->line[i]); i++) ;
			}
			hf = FALSE;
			for (; tr->line[i] != EOS; i++) {
				if (tr->line[i] == HIDEN_C)
					hf = !hf;
				else if (!hf && tr->line[i] != NL_C && tr->line[i] != SNL_C && tr->line[i] != '\n') {
					if (tr->line[i] == '\t')
						templine3[j++] = ' ';
					else
						templine3[j++] = tr->line[i];
				}
			}
			tr = ToNextRow(tr, FALSE);
		} while (!isSpeaker(tr->line[0]) && !AtBotEnd(tr, global_df->tail_text, FALSE)) ;
		templine3[j] = EOS;
		UnicodeToUTF8(templine3, j, (unsigned char *)templineC3, NULL, UTTLINELEN);
	}	
	if (!strcmp(global_df->fileName, "newfile.cha"))
		strcpy(global_df->PcTr.pictFName, prefsDir);
	else
		extractPath(global_df->PcTr.pictFName, global_df->fileName);
	addFilename2Path(global_df->PcTr.pictFName, "0graph.jpg");
	strcpy(templineC1, "https://talkbank.org/cgi-bin/morgra2jpg.cgi?morText=");
	i = strlen(templineC1);
	strcpy(templineC4, "https://talkbank.org/cgi-bin/morgra2jpg.cgi?morText=");
	strcat(templineC4, templineC2);
	strcat(templineC4, "&graText=");
	strcat(templineC4, templineC);
	strcat(templineC4, "&mainText=");
	strcat(templineC4, templineC3);
	convertToURLText(templineC1+i, templineC4+i);
#ifdef _MAC_CODE
	if (!DownloadURL(templineC1, 60000L, NULL, 0, global_df->PcTr.pictFName, FALSE, TRUE, "Downloading gra graph...")) {
		do_warning("WEB CONNECTION FAILED", -1);
		return(FALSE);
	}
#endif
#ifdef _WIN32
	u_strcpy(templine1, templineC1, UTTLINELEN);
	u_strcpy(templine, global_df->PcTr.pictFName, UTTLINELEN);
	if (curlURLDownloadToFile(templine1, templine, 60000L) == false) {
		do_warning("WEB CONNECTION FAILED", 0);
		return(FALSE);
	}
	isOverRidePicName = TRUE;
#endif
	if (access(global_df->PcTr.pictFName, 0)) {
		do_warning("ACCESS TO IMAGE FILE FAILED.", -1);
		return(FALSE);
	} else {
		fp = fopen(global_df->PcTr.pictFName, "rb");
		if (fp == NULL) {
			do_warning("ACCESS TO IMAGE FILE FAILED.", -1);
			return(FALSE);
		}
		i = 0;
		while (!feof(fp) && i < 20)
			jpgTag[i++] = getc(fp);
		fclose(fp);
		if (uS.mStrnicmp(jpgTag, "<h1>Software error:", 19) == 0) {
			do_warning("Possibly %mor tier length does not match %gra tier length", -1);
			return(FALSE);
		} else if (uS.mStrnicmp(jpgTag, "<!DOCTYPE", 9) == 0) {
			do_warning("Internal error, image generating script missing on talkbank.org, contact support", -1);
			return(FALSE);
		}
	}
	global_df->PcTr.pictChanged = TRUE;
	DisplayPhoto(global_df->PcTr.pictFName);
	return(TRUE);
}
