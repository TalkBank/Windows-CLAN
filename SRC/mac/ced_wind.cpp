#include "ced.h"
#include "my_ctype.h"
//2015 lxs #include "MMedia.h"

MACWINDOWS *RootWind;

#ifdef _UNICODE
#define DEFSYSFSIZE 9

static char fontName[256];
static short fontID = 0;
static short fontSize = 0;

char *defFontName(void) {
	if (fontID == 0 || strcmp(fontName, defUniFontName) || fontSize != defUniFontSize) {
		fontSize = defUniFontSize;
		strcpy(fontName, defUniFontName);
		if (!GetFontNumber(fontName, &fontID)) {
			fontSize = DEFSYSFSIZE;
			fontID = 4;
			strcpy(fontName, "Monaco");
		}
	}
	return(fontName);
}

short defFontID(void) {
	if (fontID == 0 || strcmp(fontName, defUniFontName) || fontSize != defUniFontSize) {
		fontSize = defUniFontSize;
		strcpy(fontName, defUniFontName);
		if (!GetFontNumber(fontName, &fontID)) {
			fontSize = DEFSYSFSIZE;
			fontID = 4;
			strcpy(fontName, "Monaco");
		}
	}
	return(fontID);
}

short defFontSize(void) {
	if (fontID == 0 || strcmp(fontName, defUniFontName) || fontSize != defUniFontSize) {
		fontSize = defUniFontSize;
		strcpy(fontName, defUniFontName);
		if (!GetFontNumber(fontName, &fontID)) {
			fontSize = DEFSYSFSIZE;
			fontID = 4;
			strcpy(fontName, "Monaco");
		}
	}
	return(fontSize);
}
#endif

/* 2015 lxs 
 short GetFontHeight(FONTINFO *fontInfo, NewFontInfo *finfo, WindowPtr wind) {
	short	  scriptId;
	FontInfo  fi;
	GrafPtr   oldPort;

	GetPort(&oldPort);
	SetPortWindowPort(wind);
	if (fontInfo != NULL) {
		TextFont(fontInfo->FName);
		TextSize(fontInfo->FSize);
	} else if (finfo != NULL) {
		TextFont(finfo->fontId);
		TextSize((short)finfo->fontSize);
	}
	GetFontInfo(&fi);
	scriptId = FontScript();
	SetPort(oldPort);
#ifndef _UNICODE
	if (scriptId == smJapanese)
		return(fi.ascent+fi.descent+fi.leading+FLINESPACE+1);
	else
#endif
		return(fi.ascent+fi.descent+fi.leading+FLINESPACE);
}

void GetWindTopLeft(WindowPtr in_wind, short *left, short *top) {
	WindowPtr	wind;
	GrafPtr		port;
	Point		pt;
	Rect		box;
		
	if (!in_wind) {
		GetPort(&port);
		wind = GetWindowFromPort(port);
	} else {
		port = NULL;
		wind = in_wind;
	}

	if (IsWindowVisible(wind)) {	
		// Use window's content region (faster than method below):
		
#if (TARGET_API_MAC_CARBON == 1)
		RgnHandle ioWinRgn = NewRgn();
		GetWindowRegion(wind, kWindowContentRgn, ioWinRgn);
		GetRegionBounds(ioWinRgn, &box);
		DisposeRgn(ioWinRgn);
#else
		box = (**((WindowPeek)wind)->contRgn).rgnBBox;	
#endif
		*left = box.left;	
		*top = box.top;	
		return;	
	}	
		
	// Set port and use LocalToGlobal:
	if (in_wind) {	
		GetPort(&port);	
		SetPortWindowPort(wind);	
	}	
	
	pt.h = pt.v = 0;	
	LocalToGlobal(&pt);	
		
	*left = pt.h;	
	*top = pt.v;	
		
	if (in_wind)	
		SetPort(port);
}	

char isClosedAllWindows(void) {
	return(RootWind == NULL);
}
*/
WindowPtr FindAWindowNamed(FNType *name) {
	MACWINDOWS *twind;
	
	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (!strcmp(twind->windRec->wname, name))
			return(twind->wind);
	}
	return(NULL);
}

WindowPtr FindAWindowProc(WindowProcRec *windProc) {
	MACWINDOWS *twind;
	
	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (twind->windRec == windProc)
			return(twind->wind);
	}
	return(NULL);
}

WindowPtr FindAWindowID(short id) {
	MACWINDOWS *twind;
	
	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (twind->windRec->id == id)
			return(twind->wind);
	}
	return(NULL);
}

extern WindowProcRec *FindAProcRecID(short id);

WindowProcRec *FindAProcRecID(short id) {
	MACWINDOWS *twind;
	
	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (twind->windRec->id == id)
			return(twind->windRec);
	}
	return(NULL);
}

WindowProcRec *WindowProcs(WindowPtr wind) {
	MACWINDOWS *twind;
	
	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (twind->wind == wind)
			return(twind->windRec);
	}
	return(NULL);
}

myFInfo *WindowFromGlobal_df(myFInfo *df) {
	MACWINDOWS *twind;

	if (df == NULL)
		return(NULL);

	for (twind=RootWind; twind != NULL; twind=twind->nextWind) {
		if (twind->windRec->FileInfo == df) 
			return(df);
	}
	return(NULL);
}
