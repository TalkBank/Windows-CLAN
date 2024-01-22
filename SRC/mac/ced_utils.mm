/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#include "my_ctype.h"
#include <Foundation/NSString.h>

enum {
	myOK=1000,
	myNO,
	myCANCEL
} ;

@implementation NSAlert (Cat) // 2020-01-16

-(NSInteger) runModalSheetForWindow:(NSWindow *)aWindow
{
	[self beginSheetModalForWindow:aWindow completionHandler:^(NSModalResponse returnCode)
	 { [NSApp stopModalWithCode:returnCode]; } ];
	NSInteger modalCode = [NSApp runModalForWindow:[self window]];
	return modalCode;
}

-(NSInteger) runModalSheet {
	NSInteger modalCode;

//	dispatch_async(dispatch_get_main_queue(), ^{
		modalCode = [self runModal];
//	});
/*
	NSWindow *aWindow = [NSApp mainWindow];
	NSInteger modalCode = [NSApp runModalForWindow:aWindow]; // [self runModalSheetForWindow:aWindow];
//	[NSApp stopModal];
*/
	return modalCode;
}

@end


long UnicodeToUTF8(const unCH *UniStr, unsigned long actualStringLength, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	unsigned long ail;
	unsigned long aol;

	actualStringLength *= 2;
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, kTextEncodingUnicodeDefault, utf8Encoding)) != noErr) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		UTF8str[0] = EOS;
		return(err);
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)UniStr, actualStringLength, &ail, (TextPtr)UTF8str, MaxLen-2, &aol)) != noErr) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		UTF8str[aol] = EOS;
		return(err);
	}
	err = TECDisposeConverter(ec);
	UTF8str[aol] = EOS;
	if (ail < actualStringLength) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		return(1L);
	}
	if (actualUT8Len != NULL)
		*actualUT8Len = aol;
	return(0L);
}

long UTF8ToUnicode(unsigned char *UTF8str, unsigned long actualUT8Len, unCH *UniStr, unsigned long *actualUnicodeLength, unsigned long MaxLen) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	unsigned long ail;
	unsigned long aol;

	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, utf8Encoding, kTextEncodingUnicodeDefault)) != noErr) {
		if (actualUnicodeLength != NULL)
			*actualUnicodeLength = 0L;
		UniStr[0] = EOS;
		return(err);
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)UTF8str, actualUT8Len, &ail, (TextPtr)UniStr, (MaxLen*2)-2, &aol)) != noErr) {
		if (actualUnicodeLength != NULL)
			*actualUnicodeLength = 0L;
		aol /= 2;
		UniStr[aol] = EOS;
		return(err);
	}
	err = TECDisposeConverter(ec);
	aol /= 2;
	UniStr[aol] = EOS;
	if (ail < actualUT8Len) {
		if (actualUnicodeLength != NULL)
			*actualUnicodeLength = 0L;
		return(1L);
	}
	if (actualUnicodeLength != NULL)
		*actualUnicodeLength = aol;
	return(0L);
}

long UnicodeToANSI(unCH *UniStr, unsigned long actualStringLength, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding MacRomanEncoding;
	unsigned long ail;
	unsigned long aol;

	actualStringLength *= 2;
	MacRomanEncoding = CreateTextEncoding((long)script, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
	if ((err=TECCreateConverter(&ec, kTextEncodingUnicodeDefault, MacRomanEncoding)) != noErr) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		ANSIstr[0] = EOS;
		return(err);
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)UniStr, actualStringLength, &ail, (TextPtr)ANSIstr, MaxLen-2, &aol)) != noErr) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		ANSIstr[aol] = EOS;
		return(err);
	}
	err = TECDisposeConverter(ec);
	ANSIstr[aol] = EOS;
	if (ail < actualStringLength) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		return(1L);
	}
	if (actualANSILen != NULL)
		*actualANSILen = aol;
	return(0L);
}

long ANSIToUnicode(unsigned char *ANSIstr, unsigned long actualANSILen, unCH *UniStr, unsigned long *actualStringLength, unsigned long MaxLen, short script) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding MacRomanEncoding;
	unsigned long ail;
	unsigned long aol;

	MacRomanEncoding = CreateTextEncoding((long)script, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, kTextEncodingUnicodeDefault)) != noErr) {
		if (err == kTECNoConversionPathErr) {
			for (aol=0; aol < actualANSILen; aol++)
				UniStr[aol] = ANSIstr[aol];
			ail = actualANSILen;
			goto finANSIToUni;
		} else {
			if (actualStringLength != NULL)
				*actualStringLength = 0L;
			UniStr[0] = EOS;
			return(err);
		}
	}
	if ((err=TECConvertText(ec, (ConstTextPtr)ANSIstr, actualANSILen, &ail, (TextPtr)UniStr, MaxLen-2, &aol)) != noErr) {
		aol /= 2;
		if (actualStringLength != NULL)
			*actualStringLength = aol;
		UniStr[aol] = EOS;
		return(err);
	}
	err = TECDisposeConverter(ec);
	aol /= 2;
finANSIToUni:
	UniStr[aol] = EOS;
	if (ail < actualANSILen) {
		if (actualStringLength != NULL)
			*actualStringLength = 0L;
		return(1L);
	}
	if (actualStringLength != NULL)
		*actualStringLength = aol;
	return(0L);
}

long UTF8ToANSI(unsigned char *UTF8str, unsigned long actualUT8Len, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding MacRomanEncoding;
	TextEncoding utf8Encoding;
	unsigned long ail;
	unsigned long aol;

	MacRomanEncoding = CreateTextEncoding((long)script, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, utf8Encoding, MacRomanEncoding)) != noErr) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		ANSIstr[0] = EOS;
		return(err);
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)UTF8str, actualUT8Len, &ail, (TextPtr)ANSIstr, MaxLen-2, &aol)) != noErr) {
		ANSIstr[aol] = EOS;
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		return(err);
	}
	err = TECDisposeConverter(ec);
	ANSIstr[aol] = EOS;
	if (ail < actualUT8Len) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		return(1L);
	}
	if (actualANSILen != NULL)
		*actualANSILen = aol;
	return(0L);
}

long ANSIToUTF8(unsigned char *ANSIstr, unsigned long actualANSILen, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen, short script) {
	OSStatus err;
	TECObjectRef ec;
	TextEncoding MacRomanEncoding;
	TextEncoding utf8Encoding;
	unsigned long ail;
	unsigned long aol;

	MacRomanEncoding = CreateTextEncoding((long)script, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		UTF8str[0] = EOS;
		return(err);
	}

	if ((err=TECConvertText(ec, (ConstTextPtr)ANSIstr, actualANSILen, &ail, (TextPtr)UTF8str, MaxLen-2, &aol)) != noErr) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		UTF8str[aol] = EOS;
		return(err);
	}
	err = TECDisposeConverter(ec);
	UTF8str[aol] = EOS;
	if (ail < actualANSILen) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		return(1L);
	}
	if (actualUT8Len != NULL)
		*actualUT8Len = aol;
	return(0L);
}

short my_FontToScript(short fName, int charSet) {
	extern short ArialUnicodeFOND, SecondDefUniFOND;

	if (ArialUnicodeFOND != 0 && ArialUnicodeFOND == fName)
		return(GetScriptManagerVariable(smKeyScript));
	if (SecondDefUniFOND != 0 && SecondDefUniFOND == fName)
		return(GetScriptManagerVariable(smKeyScript));
//2015 lxs 	return(FontToScript(fName));
	return(0);
}

short my_CharacterByteType(const char *org, short pos, NewFontInfo *finfo) {
	short cType;

	cType = 2;
/*
	for (i=0; i <= pos; i++) {
		if (IsDBCSLeadByteEx(950,(BYTE)org[i]) == 0) {
			DWORD t = GetLastError();
			if (cType == -1) {
				cType = 1;
			} else
				cType = 0;
		 } else
			 cType = -1;
	}
*/
	if (UTF8_IS_SINGLE((unsigned char)org[pos]))
		cType = 0;
	else if (UTF8_IS_LEAD((unsigned char)org[pos]))
		cType = -1;
	else if (UTF8_IS_TRAIL((unsigned char)org[pos])) {
		if (!UTF8_IS_TRAIL((unsigned char)org[pos+1]))
			cType = 1;
		else
			cType = 2;
	}

	return(cType);
}

void do_warning_sheet(const char *str, NSWindow *window) { // 2019-12-05

	if (str == NULL)
		return;

	NSAlert *alert = [[NSAlert alloc] init];
	NSString *strNSStr;
	strNSStr = [NSString stringWithUTF8String:str];
//	[alert setMessageText:@"TESTING WARNING"];
	[alert setMessageText:strNSStr];
	[alert setInformativeText:@"Click OK when ready to continue"];
//	[alert addButtonWithTitle:@"OK"];
//	[alert addButtonWithTitle:@"Cancel"];
	[alert setAlertStyle:NSAlertStyleWarning]; // NSAlertStyleInformational
//	dispatch_async(dispatch_get_main_queue(), ^{
		[alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
			if (returnCode == NSAlertSecondButtonReturn) {
				NSLog(@"Delete was cancelled!");
//				return(returnCode);
			}
			NSLog(@"This project was deleted!");
		}];
//	});
//	[alert release];
}

void do_warning(const char *str, int delay) { // 2019-12-05
	NSString *strNSStr;


	if (str == NULL)
		return;
/*
	NSTextView *accessory = [[NSTextView alloc] initWithFrame:NSMakeRect(0,0,200,15)];
	NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
	NSDictionary *textAttributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
	strNSStr = [NSString stringWithUTF8String:str];
	[accessory insertText:[[NSAttributedString alloc] initWithString:strNSStr attributes:textAttributes]];
	[accessory setEditable:NO];
	[accessory setDrawsBackground:NO];
*/
	NSAlert *alert = [[NSAlert alloc] init];
	strNSStr = [NSString stringWithUTF8String:str];
//	[alert setMessageText:@"TESTING WARNING"];
	[alert setMessageText:strNSStr];
	[alert setInformativeText:@"Click OK when ready to continue"];
//	[alert addButtonWithTitle:@"OK"];
//	[alert addButtonWithTitle:@"Cancel"];
	[alert setAlertStyle:NSAlertStyleWarning];

	dispatch_async(dispatch_get_main_queue(), ^{
		NSInteger NSModalResponse;
		NSModalResponse = [alert runModal];
	});
//	NSWindow *window = [alert window];
//	[window close];
	[alert release];

//	NSInteger NSRunAlertPanel(NSString *title, NSString *msgFormat, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, ...);
//	NSRunAlertPanel(@str, [messageTextField stringValue], @"OK", NULL, NULL);

}


int QueryDialog(const char *str, short id) { // 2020-01-16
	NSInteger	res;
	NSString	*strNSStr;
	NSWindow	*window;

	if (str == NULL)
		return(0);

	window = getWindow(CLANWIN);

	NSAlert *alert = [[NSAlert alloc] init];
	strNSStr = [NSString stringWithUTF8String:str];
	[alert setMessageText:strNSStr];
	if (id == 140) {
		[alert setInformativeText:@"Click Yes, No or Cancel"];
		[alert addButtonWithTitle:@"Yes"];
		[alert addButtonWithTitle:@"No"];
		[alert addButtonWithTitle:@"Cancel"];
	} else if (id == 147) {
		[alert setInformativeText:@"Click Yes or No"];
		[alert addButtonWithTitle:@"Yes"];
		[alert addButtonWithTitle:@"No"];
	} else {
		[alert release];
		return(0);
	}
	[alert setAlertStyle:NSAlertStyleInformational];

//	dispatch_async(dispatch_get_main_queue(), ^{
/*
	i = 0;
		[alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
			if (returnCode == NSAlertSecondButtonReturn) {
				NSLog(@"Delete was cancelled!");
//				return(returnCode);
			}
			NSLog(@"This project was deleted!");
			i = 1;
		}];
*/
//	});

	if (window != nil)
		res = [alert runModalSheetForWindow:window];
	else
		res = [alert runModalSheet];
	[alert release];
	
	if (res == myNO) {
		res = -1;
	} else if (res == myCANCEL) {
		res = 0;
	} else
		res = 1;
	return(res);
}

OSStatus my_FSPathMakeRef(const FNType *path, FSRef *ref) {
	return(FSPathMakeRef((UInt8 *)path, ref, NULL));
}

OSStatus my_FSRefMakePath(const FSRef *ref, FNType *path, UInt32 maxPathSize) {
	return(FSRefMakePath(ref, (UInt8 *)path, maxPathSize));
}

CFStringRef my_CFStringCreateWithBytes(const char *bytes) {
	return(CFStringCreateWithBytes(NULL, (UInt8 *)bytes, strlen(bytes), kCFStringEncodingUTF8, false));
}

void my_CFStringGetBytes(CFStringRef theString, char *buf, CFIndex maxBufLen) {
	CFIndex	len;

	CFStringGetBytes(theString, CFRangeMake(0, CFStringGetLength(theString)), kCFStringEncodingUTF8, 0, false, (UInt8 *)buf, maxBufLen, &len);
	buf[len] = EOS;
}

char getFileDate(FNType *fn, UInt32 *date) {
	FSCatalogInfo catalogInfo;
	FSRef  ref;
	CFAbsoluteTime oCFTime;

	*date = 0L;
	my_FSPathMakeRef(fn, &ref);
	if (FSGetCatalogInfo(&ref, kFSCatInfoContentMod, &catalogInfo, NULL, NULL, NULL) == noErr) {
		if (UCConvertUTCDateTimeToCFAbsoluteTime(&catalogInfo.contentModDate, &oCFTime) == noErr)
			UCConvertCFAbsoluteTimeToSeconds(oCFTime, date);
		return(1);
	} else
		return(0);
}

void setFileDate(FNType *fn, UInt32 date) {
	FSCatalogInfo catalogInfo;
	FSRef  ref;
	CFAbsoluteTime oCFTime;

	my_FSPathMakeRef(fn, &ref);
	if (FSGetCatalogInfo(&ref, kFSCatInfoContentMod, &catalogInfo, NULL, NULL, NULL) == noErr) {
		if (UCConvertSecondsToCFAbsoluteTime(date, &oCFTime) == noErr) {
			if (UCConvertCFAbsoluteTimeToUTCDateTime(oCFTime, &catalogInfo.contentModDate) == noErr)
				FSSetCatalogInfo(&ref, kFSCatInfoContentMod, &catalogInfo);
		}
	}
}

void gettyp(const FNType *fn, long *type, long *creator) {
	FSRef  ref;
	creator_type the_file_type;
	FSCatalogInfo catalogInfo;

	my_FSPathMakeRef(fn, &ref);
	if (FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL) == noErr) {
		the_file_type.in[0] = catalogInfo.finderInfo[0];
		the_file_type.in[1] = catalogInfo.finderInfo[1];
		the_file_type.in[2] = catalogInfo.finderInfo[2];
		the_file_type.in[3] = catalogInfo.finderInfo[3];
		*type = the_file_type.out;
		the_file_type.in[0] = catalogInfo.finderInfo[4];
		the_file_type.in[1] = catalogInfo.finderInfo[5];
		the_file_type.in[2] = catalogInfo.finderInfo[6];
		the_file_type.in[3] = catalogInfo.finderInfo[7];
		*creator = the_file_type.out;
	} else
		return;
}

void settyp(const FNType *fn, long type, long creator, char isForce) {
	FSRef  ref;
	creator_type the_file_type;
	FSCatalogInfo catalogInfo;

	my_FSPathMakeRef(fn, &ref);
	if (FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL) == noErr) {
		if (type || isForce) {
			the_file_type.out = type;
			catalogInfo.finderInfo[0] = the_file_type.in[0];
			catalogInfo.finderInfo[1] = the_file_type.in[1];
			catalogInfo.finderInfo[2] = the_file_type.in[2];
			catalogInfo.finderInfo[3] = the_file_type.in[3];
		}
		if (creator || isForce) {
			the_file_type.out = creator;
			catalogInfo.finderInfo[4] = the_file_type.in[0];
			catalogInfo.finderInfo[5] = the_file_type.in[1];
			catalogInfo.finderInfo[6] = the_file_type.in[2];
			catalogInfo.finderInfo[7] = the_file_type.in[3];
		}
		FSSetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catalogInfo);
	} else
		return;
}

int LocateDir(const char *prompt, FNType *currentDir, char noDefault) {
	char DirSelected = FALSE;
	NSString *path;

	//this gives you a copy of an open file dialogue
	NSOpenPanel* panel = [NSOpenPanel openPanel];

	//set the title of the dialogue window
	panel.title = [NSString stringWithUTF8String:prompt];

	//shoud the user be able to resize the window?
	panel.showsResizeIndicator = YES;

	//should the user see hidden files (for user apps - usually no)
	panel.showsHiddenFiles = NO;

	//can the user select a directory?
	panel.canChooseDirectories = YES;

	//can the user create directories while using the dialogue?
	panel.canCreateDirectories = YES;

	//should the user be able to select multiple files?
	panel.allowsMultipleSelection = NO;

	//an array of file extensions to filter the file list
	panel.allowedFileTypes = @[@"NoOneExtensionAllowed"];

	//this launches the dialogue
	if ([panel runModal] == NSOKButton) {
		path = [panel filename];
		DirSelected = TRUE;
			// Open  the document.
	}

	if (DirSelected) {
		strcpy(currentDir, [path UTF8String]);
		return(TRUE);
	} else
		return(FALSE);
}

#define isFontDefaultUni(x, y) ((x == ArialUnicodeFOND/* || x == SecondDefUniFOND*/) && y == defFontSize())


struct WriteHelp {
	FNType *fn;
#ifdef _WIN32
	unCH Font[LF_FACESIZE];
	long Size;
#else
	short Font, Size;
#endif
	int  charSet;
	char isEndFound;
	char isFontChanged;
	FILE *fpout;
} ;

int  LineNumberDigitSize;
char LineNumberingType;
char  last_cr_char = 0;

char *fgets_ced(char *beg,int size,FILE *fp,unsigned long *ocnt) {
	char isCNT;
	unsigned long cnt;
	extern short DOSdata;
	
	cnt = 0L;
	size--;
	beg[cnt] = (char)getc(fp);
	if (feof(fp)) {
		if (ocnt != NULL)
			*ocnt = cnt;
		return(NULL);
	}
	
	if (last_cr_char == '\r' && beg[cnt] == '\n') {
#if defined(_MAC_CODE)
		if (DOSdata == 0)
			DOSdata = 5;
#endif
	}
#if defined(_WIN32)
	else if (last_cr_char == '\n' && beg[cnt] != '\r') {
		if (DOSdata == 0)
			DOSdata = 5;
	}
#endif
	do {
		isCNT = TRUE;
		if (beg[cnt] == '\r') {
			if (last_cr_char != '\n') {
				beg[cnt] = '\n';
				last_cr_char = '\r';
				break;
			} else {
				isCNT = FALSE;
			}
		} else if (beg[cnt] == '\n') {
			if (last_cr_char != '\r') {
				beg[cnt] = '\n';
				last_cr_char = '\n';
				break;
			} else {
				isCNT = FALSE;
			}
		}
		last_cr_char = 0;
		if (cnt >= size)
			break;
		if (isCNT)
			cnt++;
		beg[cnt] = (char)getc(fp);
		if (feof(fp)) {
			cnt--;
			break;
		}
	} while (1) ;
	cnt++;
	if (beg[0] == '\032' && cnt == 1L)		 
		cnt = 0L;
	beg[cnt] = EOS;
	if (ocnt != NULL)
		*ocnt = cnt;
	return(beg);
}

// check file extensions
char isCHATFile(FNType *fname) {
	register int i, j, k;
	unCH ext[16];
	
	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);
	
	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'A' || iswdigit(ext[2])) )
			return('\001');
		else if (ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'I' || iswdigit(ext[2])) )
			return('\001');
		else if ((ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'C' && ext[1] == 'U' && (ext[2] == 'T' || iswdigit(ext[2])))
				 )
			return('\003');
		else if ((ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'E' && (ext[2] == 'L' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'S' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'I' && (ext[2] == 'X' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'L' && (ext[2] == 'O' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'X' && (ext[2] == 'B' || iswdigit(ext[2]))) ||
				 (ext[0] == 'G' && ext[1] == 'E' && (ext[2] == 'M' || iswdigit(ext[2]))) ||
				 (ext[0] == 'I' && ext[1] == 'D' && (ext[2] == 'N' || iswdigit(ext[2]))) ||
				 (ext[0] == 'I' && ext[1] == 'N' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
				 (ext[0] == 'L' && ext[1] == 'O' && (ext[2] == 'W' || iswdigit(ext[2]))) ||
				 (ext[0] == 'M' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'P' && ext[1] == 'S' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'S' && ext[1] == 'T' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'T' && ext[1] == 'M' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
				 (ext[0] == 'T' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'U' && ext[1] == 'T' && (ext[2] == 'F' || iswdigit(ext[2])))
				 )
			return('\002');
		else if (ext[0] == 'C' && ext[1] == 'E' && ext[2] == 'X') {
			for (i=j-1; i > 0 && fname[i] != '.'; i--) ;
			if (i > 0 && j - i < 14) {
				k = 0;
				for (i++; i < j; i++)
					ext[k++] = towupper(fname[i]);
				ext[k] = EOS;
				if ((ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'E' && (ext[2] == 'L' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'S' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'I' && (ext[2] == 'X' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'L' && (ext[2] == 'O' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'X' && (ext[2] == 'B' || iswdigit(ext[2]))) ||
					(ext[0] == 'G' && ext[1] == 'E' && (ext[2] == 'M' || iswdigit(ext[2]))) ||
					(ext[0] == 'I' && ext[1] == 'D' && (ext[2] == 'N' || iswdigit(ext[2]))) ||
					(ext[0] == 'I' && ext[1] == 'N' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
					(ext[0] == 'L' && ext[1] == 'O' && (ext[2] == 'W' || iswdigit(ext[2]))) ||
					(ext[0] == 'M' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'P' && ext[1] == 'S' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
					(ext[0] == 'S' && ext[1] == 'T' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'T' && ext[1] == 'M' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
					(ext[0] == 'T' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'U' && ext[1] == 'T' && (ext[2] == 'F' || iswdigit(ext[2]))) ||
					!strncmp(ext,"CHIP",4)  || !strncmp(ext,"DATE",4)  || !strncmp(ext,"KWAL",4)  ||
					!strncmp(ext,"SYNC",4)  ||
					!strncmp(ext,"DELIM",5) || !strncmp(ext,"CHSTR",5) || !strncmp(ext,"INDNT",5) ||
					!strncmp(ext,"COMBO",5) || !strncmp(ext,"FIXIT",5) || !strncmp(ext,"TORDR",5) ||
					!strncmp(ext,"MEDIA",5) ||!strncmp(ext,"IPCORE",5) ||
					!strncmp(ext,"LOWCAS",6)|| !strncmp(ext,"FXBLTS",6)|| !strncmp(ext,"CP2UTF",6)||
					!strncmp(ext,"MGRASP",6)|| !strncmp(ext,"PMORTM",6)||
					!strncmp(ext,"COMB",4)
					)
					return('\002');
			} else
				return('\004');
		} else if (ext[0] == 'X' && ext[1] == 'L' && ext[2] == 'S') {
			for (i=j-1; i > 0 && fname[i] != '.'; i--) ;
			if (i > 0 && j - i < 14) {
				k = 0;
				for (i++; i < j; i++)
					ext[k++] = towupper(fname[i]);
				ext[k] = EOS;
				if (!strncmp(ext,"KWAL",4)  || !strncmp(ext,"KIDEVAL",3)|| !strncmp(ext,"MLT",3)  || !strncmp(ext,"MLU",3)   ||
					!strncmp(ext,"OUT",3)   || !strncmp(ext,"MRTBL",5) || !strncmp(ext,"TIMDUR",6) || !strncmp(ext,"VOCD",4) ||
					!strncmp(ext,"WDLEN",5) || !strncmp(ext,"FLUCALC",5) || !strncmp(ext,"SCRIPT",3))
					return('\005');
			}
		}
	}
	return(FALSE);
}

char isCEXFile(FNType *fname) {
	register int j;
	unCH ext[4];
	
	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);
	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'C' && ext[1] == 'E' && ext[2] == 'X')
			return('\001');
		else
			return(FALSE);
	} else
		return(FALSE);
}

char SetChatModeOfDatafileExt(FNType *fname, char isJustCheck) {
	char res;
	extern short DOSdata;
	
	res = isCHATFile(fname);
	if (res == '\002') {
		res = TRUE;
		if (!isJustCheck)
			DOSdata = 0;
	} else if (res == '\003' || res == '\005')
		res = FALSE;
	else if (res == '\004')
		res = TRUE;
	return(res);
}
// end check file extensions

unCH BigWBuf[UTTLINELEN+UTTLINELEN];

unCH *cl_T(const char *st) {
	strcpy(BigWBuf, st);
	return(BigWBuf);
}
/*
char my_isalnum(char c) {
	return((char)isalnum((int)c))
}

char my_isalpha(char c) {
	return((char)isalpha((int)c))
}

char my_isdigit(char c) {
	return((char)isdigit((int)c))
}

char my_isspace(char c) {
	return((char)isspace((int)c))
}

char my_islower(char c) {
	return((char)islower((int)c))
}

char my_isupper(char c) {
	return((char)isupper((int)c))
}

char my_tolower(char c) {
	return((char)tolower((int)c))
}

char my_toupper(char c) {
	return((char)toupper((int)c))
}
*/

/*
int LocateDir(const char *prompt, FNType *currentDir, char noDefault) {
	char				DirSelected;
	int					len;
	FNType				new_path[FNSize];
	FNType 				dirname[FNSize];
	NavReplyRecord		reply;
	OSStatus			anErr = noErr;
	FSRef				ref;
	CFStringRef			CFPrompt,
	CFdirname;
	NavDialogRef		outDialog;
	NavEventUPP			eventProc = NewNavEventUPP(myNavEventProc);
	NavDialogCreationOptions Options;

	DirSelected = FALSE;
	uS.str2FNType(dirname, 0L, "Current Directory-> ");
	strcat(dirname, currentDir);
	CFPrompt = my_CFStringCreateWithBytes(prompt);
	if (CFPrompt == NULL)
		return(FALSE);
	CFdirname= my_CFStringCreateWithBytes(dirname);
	if (CFdirname == NULL) {
		CFRelease(CFPrompt);
		return(FALSE);
	}

	NavGetDefaultDialogCreationOptions(&Options);
	Options.version = kNavDialogCreationOptionsVersion;
	Options.optionFlags = kNavDefaultNavDlogOptions;
	Options.location = SFGwhere;
	Options.clientName = CFSTR("CLAN");
	Options.windowTitle = CFPrompt;
	Options.actionButtonLabel = CFSTR("Select Folder");
	Options.cancelButtonLabel = CFSTR("Cancel");
	//	Options.saveFileName = CFSTR("");
	Options.message = CFdirname;
	Options.preferenceKey = 0;
	//	Options.popupExtension;
	//	Options.modality;
	//	Options.parentWindow;
	anErr = NavCreateChooseFolderDialog(&Options, eventProc, NULL, NULL, &outDialog);
	if (anErr != noErr) {
		CFRelease(CFPrompt);
		CFRelease(CFdirname);
		return(FALSE);
	}

	if (noDefault) {
		if (!isRefEQZero(defaultPath)) {
			AEDesc defaultDir;

			my_FSPathMakeRef(defaultPath, &ref);
			AECreateDesc(typeFSRef, &ref, sizeof(ref),&defaultDir);
			NavCustomControl(outDialog, kNavCtlSetLocation, &defaultDir);
			AEDisposeDesc(&defaultDir);
		}
	} else if (!isRefEQZero(currentDir)) {
		AEDesc defaultDir;

		my_FSPathMakeRef(currentDir, &ref);
		AECreateDesc(typeFSRef, &ref, sizeof(ref),&defaultDir);
		NavCustomControl(outDialog, kNavCtlSetLocation, &defaultDir);
		AEDisposeDesc(&defaultDir);
	}
	anErr = NavDialogRun(outDialog);
	if (anErr != noErr) {
		CFRelease(CFPrompt);
		CFRelease(CFdirname);
		NavDialogDispose(outDialog);
		return(FALSE);
	}
	anErr = NavDialogGetReply(outDialog, &reply);
	if (anErr != noErr) {
		CFRelease(CFPrompt);
		CFRelease(CFdirname);
		NavDialogDispose(outDialog);
		return(FALSE);
	}
	if (reply.validRecord) {
		long count;
		anErr = AECountItems(&(reply.selection),&count);
		if (anErr == noErr) {
			long index;
			AEKeyword theKeyword;
			DescType actualType;
			Size actualSize;

			for (index=1; index <=count; index++) {
				anErr = AEGetNthPtr(&(reply.selection),index,typeFSRef,&theKeyword,&actualType,&ref,sizeof(ref),&actualSize);
				if (anErr ==noErr) {
					DirSelected = TRUE;
					my_FSRefMakePath(&ref, new_path, FNSize);
					if (noDefault) {
						strcpy(defaultPath, new_path);
						len = strlen(defaultPath);
						if (defaultPath[len-1] != PATHDELIMCHR)
							uS.str2FNType(defaultPath, len, PATHDELIMSTR);
					}
				}
			}
		}
	}
	//Dispose of NavReplyRecord,resources,descriptors
	NavDisposeReply(&reply);
	NavDialogDispose(outDialog);
	CFRelease(CFPrompt);
	CFRelease(CFdirname);

	if (DirSelected) {
		FinishMainLoop();
		strcpy(currentDir, new_path);
		return(TRUE);
	} else
		return(FALSE);
}
*/
