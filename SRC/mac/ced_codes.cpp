
#import "DocumentWinController.h"
#import "UserStrController.h"


// The elements of RootCodes tree point to the memory allocated for
// coresponding code within this list. It is used to save space, i.e. space,
// for each code string is allocated only once.
#define CODESLIST struct code_slist
CODESLIST
{
	unCH *cod;
	CODESLIST *next_code;
} *RootCodeList;

extern char ced_err_message[];

int  MaxNumOfCodes;
char isLastCode;
char DisTier[51];

@interface LowerTextView(Public)
-(CODES **)DisplayCodes:(DocumentWindowController *)docWinController;
-(void)FindLastPage:(DocumentWindowController *)docWinController;
@end


static void code_getflag(char *f, DocumentWindowController *docWinController) {
	f++;
	switch(*f++) {
		case 'b': // +bN: set number of commands and words before auto-save"
			docWinController->FreqCountLimit = atoi(f);
			break;
		case 'd': // +d: create backup file; -d: do NOT create backup file
			if (*(f-2) == '+')
				docWinController->MakeBackupFile = true;
			else
				docWinController->MakeBackupFile = false;
			break;
		case 'f': // +fS: specify fixed-width font S (example: +fCAfont)
			break;
		case 'l': // +lN: re-order codes (0 = do not change order, 1 = advance to top, 2 = advance one step)
			docWinController->PriorityCodes = (char)atoi(f);
			if (docWinController->PriorityCodes > '\002') {
				docWinController->PriorityCodes = '\002';
			}
			break;
		case 's': // +sN: progam will make identical copies of codes across branches
			if (*f == '0')
				docWinController->CodingScheme = '\0';
			else
				docWinController->CodingScheme = '\001';
			break;
		case 't': // "+tS: set next speaker name to S")
			if (*f) {
				int i;
				if (*f != '*')
					strcpy(NextTierName, cl_T("*"));
				else
					NextTierName[0] = EOS;
				strcat(NextTierName, f);
				uS.uppercasestr(NextTierName, &dFnt, C_MBF);
				for (i=0; !isSpace(NextTierName[i]) && NextTierName[i] != '\n' && NextTierName[i]!= EOS; i++) ;
				NextTierName[i] = EOS;
			}
			break;
		case 'x': // "+xS: disambiguate tier S (default %s)" =  DisTier
			if (*f) {
				strncpy(DisTier, f, 50);
				DisTier[50] = EOS;
				uS.uppercasestr(DisTier, &dFnt, C_MBF);
			}
			break;
		default:
			break;
	}
}

static void FreeCL(void) {
	CODESLIST *t;

	while (RootCodeList != NULL) {
		t = RootCodeList;
		RootCodeList = RootCodeList->next_code;
		if (t->cod != NULL)
 			free(t->cod);
		free(t);
	}
}

static void FreeRC(DocumentWindowController *docWinController) {
	CODES *trcn, *trc;

	while (docWinController->RootCodes != NULL) {
		trcn = docWinController->RootCodes->NextCode;
		while (trcn != NULL) {
			trc = trcn;
			trcn = trcn->NextCode;
			free(trc);
		}
		trc = docWinController->RootCodes;
		docWinController->RootCodes = docWinController->RootCodes->subcodes;
		free(trc);
	}
}

void FreeCodesMem(DocumentWindowController *docWinController) {
	if (docWinController->RootCodesArr)
		free(docWinController->RootCodesArr);
	FreeCL();
	FreeRC(docWinController);
	docWinController->RootCodes = NULL;
	docWinController->RootCodesArr = NULL;
	RootCodeList = NULL;
}

static NSUInteger BeginningOfLine(NSUInteger pos, NSString *textSt) {
	unichar ch;

//	if (pos >= len)
//		pos = len - 1;
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n')
			pos--;
	}
	while (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
		pos--;
	}
	return(pos);
}

static NSUInteger EndOfLine(NSUInteger pos, NSString *textSt, NSUInteger len) {
	unichar ch;

	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			pos++;
			break;
		}
		pos++;
	}
	return(pos);
}

static void GetCurCode(NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	unichar ch;
	NSUInteger i, tPos, len;
	NSTextStorage *text;
	NSString *textSt;
	NSRange cursorRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	if (textView == nil) {
		textView = [docWinController firstTextView];
		cursorRange = [textView selectedRange];
		pos = cursorRange.location;
	}
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];

	*sp = EOS;
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	pos = BeginningOfLine(pos, textSt);
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	while (true) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
		if (isSpeaker(ch)) {
			i = 0;
			tPos = pos;
			while (tPos < len) {
				ch = [textSt characterAtIndex:tPos];
				sp[i++] = ch;
				tPos++;
				if (ch == ':' || ch == '\n') {
					break;
				}
			}
			sp[i] = EOS;
			break;
		}
		if (pos == 0)
			break;
		pos--;
	}
}

CODES **DisplayCodes(DocumentWindowController *docWinController) {
	CODES **res;

	res = [docWinController->lowerView DisplayCodes:docWinController];
	[docWinController->lowerView setNeedsDisplay:YES];
	return(res);
}

int ToTopLevel(int i, DocumentWindowController *docWinController) {
	if (docWinController->EditorMode && i != -1) {
//		do_warning_sheet("Illegal", [docWinController window]);
		NSBeep();
		return(9);
	}
	if (docWinController->RootCodesArr[0] != docWinController->CurCode && i != -1) {
		docWinController->FirstCodeOfPrevLevel = docWinController->CurFirstCodeOfPrevLevel;
		MapArrToList(docWinController->CurCode, docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else if (docWinController->CurCode != docWinController->RootCodes) {
		docWinController->EnteredCode = '\0';
		docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodes;
		docWinController->CurCode = docWinController->RootCodes;
		docWinController->FirstCodeOfPrevLevel = docWinController->RootCodes;
		MapArrToList(docWinController->CurCode, docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
		GetCurCode(nil, 0, docWinController);
		FindRightCode(1, docWinController);
	}
	return(9);
}

static char isUNDCode(unCH *code, NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	extern UserStrController *UserStrWindow;

	if (strncmp(code, "?|", 2) == 0) {
		strcpy(templine3, code);
		[UserStrController userStrDialog:docWinController label:@"Make change and/or press Done:" str:templine3 max:UTTLINELEN];
		if (templine3[0] == EOS)
			strcpy(templine3, code);
		AddCodeTier(templine3, TRUE, textView, pos, docWinController);
		return(TRUE);
	}
	return(FALSE);
}

static void PositionCursor(NSTextView *textView, NSUInteger pos) {
	unichar ch;
	NSUInteger tPos, len;
	NSTextStorage *text;
	NSString *textSt;
	NSRange charRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	tPos = BeginningOfLine(pos, textSt);
#ifdef DEBUGCODE
if (tPos+40 >= len) dl = len-tPos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(tPos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	if (tPos < len) {
		ch = [textSt characterAtIndex:tPos];
		if (ch == '%') {
			while (tPos < len) {
				ch = [textSt characterAtIndex:tPos];
#ifdef DEBUGCODE
if (tPos+40 >= len) dl = len-tPos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(tPos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				if (ch == ':' || ch == '\n') {
					break;
				}
				if (tPos == pos)
					pos++;
				tPos++;
			}
			if (ch == ':') {
				if (tPos == pos)
					pos++;
				tPos++;
				while (tPos < len) {
					ch = [textSt characterAtIndex:tPos];
#ifdef DEBUGCODE
if (tPos+40 >= len) dl = len-tPos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(tPos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
					if (!isSpace(ch)) {
						charRange = NSMakeRange(pos, 0);
						[textView setSelectedRange:charRange];
						[textView scrollRangeToVisible:charRange];
						break;
					}
					if (tPos == pos)
						pos++;
					tPos++;
				}
			}
		}
	}
}

void AddCodeTier(unCH *code, char real_code, NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	BOOL BlankLine;
	unichar ch, lastCh;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
	NSString *keys;
	NSRange charRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	if (real_code == '\002') {
		pos = BeginningOfLine(pos, textSt);
		BlankLine = true;
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				charRange = NSMakeRange(pos, 0);
				[textView setSelectedRange:charRange];
				[textView scrollRangeToVisible:charRange];
				break;
			} else if (!isSpace(ch)) {
				BlankLine = false;
				charRange = NSMakeRange(pos, 0);
				[textView setSelectedRange:charRange];
				[textView scrollRangeToVisible:charRange];
				break;
			}
			pos++;
		}
		strcpy(templineW, code);
		len = strlen(templineW);
		if (!BlankLine) {
			strcat(templineW, "\n");
		}
	} else if (docWinController->CurCode == docWinController->RootCodes && docWinController->ChatMode && (docWinController->cod_fname != NULL || !real_code)) {
		pos = BeginningOfLine(pos, textSt);
		lastCh = '\n';
		ch = '\n';
		while (pos < len) {
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			ch = [textSt characterAtIndex:pos];
			if (lastCh == '\n' && !isSpace(ch)) {
				charRange = NSMakeRange(pos, 0);
				[textView setSelectedRange:charRange];
				[textView scrollRangeToVisible:charRange];
				break;
			}
			lastCh = ch;
			pos++;
		}
		if (ch == '\n')
			BlankLine = true;
		else
			BlankLine = false;
//		FindMidWindow();
		strcpy(templineW, code);
		len = strlen(templineW);
		if (!BlankLine) {
			strcat(templineW, "\n");
		}
	} else {
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		strcpy(templineW, code);
		len = strlen(templineW);
	}
	charRange = NSMakeRange(pos, 0);
	keys = [NSString stringWithCharacters:templineW length:strlen(templineW)];
	if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
		[text replaceCharactersInRange:charRange withString:keys];
		[textView didChangeText];
		[[textView undoManager] setActionName:@"Coder"];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		charRange = NSMakeRange(pos+len, 0);
		[textView setSelectedRange:charRange];
		[textView scrollRangeToVisible:charRange];
	}
	if (real_code == TRUE)
		MoveToSubcode(docWinController);
}

static int FindTextCodeLine(unCH *code, NSTextView *textView, NSUInteger pos) {
	unichar ch;
	unichar bufU[BUFSIZ];
	NSUInteger len, codeLen;
	NSTextStorage *text;
	NSString *textSt;
	NSRange charRange;
#ifdef DEBUGCODE
NSUInteger dl;
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	codeLen = strlen(code);
	*sp = EOS;
	bufU[0] = EOS;
	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n') {
			if (pos+1 < len) {
				ch = [textSt characterAtIndex:pos+1];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				if (isMainSpeaker(ch)) {
					break;
				} else if (pos+codeLen < len) {
					charRange.location = pos + 1;
					charRange.length = codeLen;
					[textSt getCharacters:bufU range:charRange];
					bufU[codeLen] = EOS;
					if (strcmp(bufU, code) == 0) { // 2020-08-24
						strcpy(sp, code);
						charRange.length = 0;
						[textView setSelectedRange:charRange];
						[textView scrollRangeToVisible:charRange];
						return(TRUE);
					}
				}
			}
		}
		pos++;
	}
	return(FALSE);
}


static BOOL DeleteHighlight(NSTextView *textView, NSTextStorage *text, NSUInteger *pos) {
	NSString *keys;
	NSRange cursorRange;
	NSString *textSt;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	textSt = [text string];
	cursorRange = [textView selectedRange];
	if (cursorRange.length == 0)
		return(true);
	keys = [NSString stringWithUTF8String:""];
	if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
		[text replaceCharactersInRange:cursorRange withString:keys];
		[textView didChangeText];
		[[textView undoManager] setActionName:@"Coder"];
#ifdef DEBUGCODE
if (cursorRange.location+40 >= [text length]) dl = [text length]-cursorRange.location; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(cursorRange.location, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		cursorRange.length = 0;
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
		*pos = cursorRange.location;
	}
	return(true);
}

int GetCursorCode(int i, DocumentWindowController *docWinController) {
	int  cd = -1;
	BOOL isSpaceFound = false;
	unCH *s;
	unichar ch, lastCh;
	NSUInteger pos, len, codeLen;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSRange cursorRange;
	unichar bufU[BUFSIZ];
#ifdef DEBUGCODE
NSUInteger dl;
#endif


	if (docWinController->EditorMode && i != -1) {
//		do_warning_sheet("Illegal", [docWinController window]);
		NSBeep();
		*sp = EOS;
		return(39);
	}

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	pos = cursorRange.location;

	if (docWinController->CurCode != docWinController->RootCodes || !docWinController->ChatMode || docWinController->cod_fname == NULL) {
		if (DeleteHighlight(textView, text, &pos)) {
			len = [text length];
			if (pos < len && pos > 0) {
#ifdef DEBUGCODE
if (pos-1+40 >= len) dl = len-pos-1; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos-1, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				ch = [textSt characterAtIndex:pos-1];
				isSpaceFound = isSpace(ch);
			} else
				isSpaceFound = false;
			s = docWinController->StartCodeArr[docWinController->CursorCodeArr]->code;
			if (!isSpaceFound) {
				if (!isUNDCode(s, textView, pos, docWinController))
					AddCodeTier(s, TRUE, textView, pos, docWinController);
			} else {
				if (*s == ' ') {
					if (!isUNDCode(s+1, textView, pos, docWinController))
						AddCodeTier(s+1, TRUE, textView, pos, docWinController);
				} else {
					if (!isUNDCode(s, textView, pos, docWinController))
						AddCodeTier(s, TRUE, textView, pos, docWinController);
				}
			}
			if (docWinController->cod_fname == NULL) {
				docWinController->isDisambiguateAgain = true;
			}
		}
	} else {
		if (pos == 0 && len == 0)
			sp[0] = EOS;
		else
			GetCurCode(textView, pos, docWinController);
		if (!uS.partcmp(docWinController->StartCodeArr[docWinController->CursorCodeArr]->code, sp, FALSE, TRUE) || *sp == EOS) {
			strcpy(templine1, docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
			uS.remFrontAndBackBlanks(templine1);
			if (*sp == '@') {
				pos = BeginningOfLine(pos, textSt);
				lastCh = '\n';
				ch = '\n';
				while (pos < len) {
					ch = [textSt characterAtIndex:pos];
					if (lastCh == '\n' && isMainSpeaker(ch)) {
						codeLen = strlen(NextTierName);
						cursorRange.location = pos;
						cursorRange.length = codeLen;
						[textSt getCharacters:bufU range:cursorRange]; bufU[codeLen] = EOS;
						if (NextTierName[0] == EOS || uS.mStricmp(bufU, NextTierName) == 0) {
							pos = EndOfLine(pos, textSt, len);
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
							cursorRange = NSMakeRange(pos, 0);
							[textView setSelectedRange:cursorRange];
							[textView scrollRangeToVisible:cursorRange];
							break;
						}
					}
					lastCh = ch;
					pos++;
				}
				if (pos >= len) {
					do_warning_sheet("Can't find right speaker tier.", [docWinController window]);
					*sp = EOS;
					return(39);
				}
			}
			if (!FindTextCodeLine(templine1, textView, pos)) {
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				ch = [textSt characterAtIndex:pos];
				if (ch != '%')
					pos = EndOfLine(pos, textSt, len);
				AddCodeTier(docWinController->StartCodeArr[docWinController->CursorCodeArr]->code, TRUE, textView, pos, docWinController);
				cd = 0;
			} else {
				docWinController->EnteredCode = '\001';
				strcpy(spareTier1, "Please put text cursor at desired place on the \"");
				u_strcpy(spareTier1+strlen(spareTier1), templine1, UTTLINELEN);
				strcat(spareTier1, "\" tier");
				do_warning_sheet(spareTier1, [docWinController window]);
			}
		} else {
			PositionCursor(textView, pos);
			MoveToSubcode(docWinController);
		}
	}
	*sp = EOS;
	return(39);
}

static void FindLastPage(DocumentWindowController *docWinController) {
	[docWinController->lowerView FindLastPage:docWinController];
	[docWinController->lowerView setNeedsDisplay:YES];
}

int MoveCodeCursorUp(DocumentWindowController *docWinController) {
	if (docWinController->StartCodeArr+docWinController->CursorCodeArr == docWinController->RootCodesArr + 1) {
		for (; *docWinController->StartCodeArr != NULL; docWinController->StartCodeArr++) ;
		FindLastPage(docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else if (docWinController->CursorCodeArr == 0) {
		FindLastPage(docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else {
		docWinController->CursorCodeArr--;
	}
	[docWinController->lowerView setNeedsDisplay:YES];
	if (docWinController->CurCode == docWinController->RootCodes)
		strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	*sp = EOS;
	return(42);
}

int MoveCodeCursorDown(DocumentWindowController *docWinController) {
	docWinController->CursorCodeArr++;
	if (docWinController->StartCodeArr[docWinController->CursorCodeArr] == docWinController->EndCodeArr[0]) {
		if (docWinController->EndCodeArr[0] == NULL)
			docWinController->StartCodeArr = docWinController->RootCodesArr + 1;
		else
			docWinController->StartCodeArr = docWinController->EndCodeArr;
		docWinController->CursorCodeArr = 0;
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else
		[docWinController->lowerView setNeedsDisplay:YES];
	if (docWinController->CurCode == docWinController->RootCodes)
		strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	*sp = EOS;
	return(43);
}

int MoveCodeCursorLeft(DocumentWindowController *docWinController) {
	int i, j;
	NSUInteger wsize;

	wsize = docWinController->lowerView->rowsNum - 1;
	if (docWinController->CursorCodeArr < wsize) {
		i = docWinController->CursorCodeArr;
		j = 0;
		do {
			i = i + wsize;
			for (; docWinController->StartCodeArr[j] != docWinController->EndCodeArr[0] && j < i; j++) ;
			if (docWinController->StartCodeArr[j] != docWinController->EndCodeArr[0])
				docWinController->CursorCodeArr = j;
			if (docWinController->StartCodeArr[j] == docWinController->EndCodeArr[0])
				break;
		} while (1) ;
	} else {
		docWinController->CursorCodeArr = docWinController->CursorCodeArr - wsize;
	}
	if (docWinController->StartCodeArr[docWinController->CursorCodeArr] == docWinController->EndCodeArr[0]) {
		if (docWinController->EndCodeArr[0] == NULL)
			docWinController->StartCodeArr = docWinController->RootCodesArr + 1;
		else
			docWinController->StartCodeArr = docWinController->EndCodeArr;
		docWinController->CursorCodeArr = 0;
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else
		[docWinController->lowerView setNeedsDisplay:YES];
	if (docWinController->CurCode == docWinController->RootCodes)
		strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	*sp = EOS;
	return(70);
}

int MoveCodeCursorRight(DocumentWindowController *docWinController) {
	int i;
	NSUInteger wsize;

	wsize = docWinController->lowerView->rowsNum - 1;
	docWinController->CursorCodeArr = docWinController->CursorCodeArr + wsize;
	for (i=0; docWinController->StartCodeArr[i] != docWinController->EndCodeArr[0] && i < docWinController->CursorCodeArr; i++) ;
	if (i < docWinController->CursorCodeArr || docWinController->StartCodeArr[i] == docWinController->EndCodeArr[0]) {
		i = docWinController->CursorCodeArr;
		docWinController->CursorCodeArr = i - ((i / wsize) * wsize);
	}
	if (docWinController->StartCodeArr[docWinController->CursorCodeArr] == docWinController->EndCodeArr[0]) {
		if (docWinController->EndCodeArr[0] == NULL)
			docWinController->StartCodeArr = docWinController->RootCodesArr + 1;
		else
			docWinController->StartCodeArr = docWinController->EndCodeArr;
		docWinController->CursorCodeArr = 0;
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else
		[docWinController->lowerView setNeedsDisplay:YES];
	if (docWinController->CurCode == docWinController->RootCodes)
		strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	*sp = EOS;
	return(77);
}

int GetFakeCursorCode(int i, DocumentWindowController *docWinController) {
	NSUInteger pos, len;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSRange cursorRange;

	if (docWinController->EditorMode && i != -1) {
//		do_warning_sheet("Illegal", [docWinController window]);
		NSBeep();
		*sp = EOS;
		return(40);
	}

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	len = [text length];
	pos = cursorRange.location;

	if (docWinController->CurCode == docWinController->RootCodes) {
		GetCurCode(textView, pos, docWinController);
		if (!FindRightCode(0, docWinController))
			return(40);
		PositionCursor(textView, pos);
	}
	if (docWinController->StartCodeArr[docWinController->CursorCodeArr]->subcodes != NULL) {
		if (docWinController->CurCode == docWinController->RootCodes) {
			docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodesArr[0]->subcodes;
			docWinController->CurCode = docWinController->StartCodeArr[docWinController->CursorCodeArr];
		}
		docWinController->FirstCodeOfPrevLevel = docWinController->RootCodesArr[0]->subcodes;
		MapArrToList(docWinController->StartCodeArr[docWinController->CursorCodeArr], docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else if (docWinController->RootCodesArr[0] != docWinController->CurCode) {
		docWinController->FirstCodeOfPrevLevel = docWinController->CurFirstCodeOfPrevLevel;
		MapArrToList(docWinController->CurCode, docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	}
	*sp = EOS;
	return(40);
}

static unCH *AddCodeToList(unCH *s) {
	short cnt = 2;
	CODESLIST *nc;

	if (RootCodeList == NULL) {
		nc = NEW(CODESLIST);
		if (nc == NULL)
			return(NULL);
		RootCodeList = nc;
	} else {
		for (nc=RootCodeList; 1; nc=nc->next_code) {
			if (strcmp(nc->cod,s) == 0)
				return(nc->cod);
			cnt++;
			if (nc->next_code == NULL) {
				nc->next_code = NEW(CODESLIST);
				if (nc->next_code == NULL)
					return(NULL);
				nc = nc->next_code;
				break;
			}
		}
	}
	nc->cod = (unCH *)malloc((strlen(s)+1)*sizeof(unCH));
	if (nc->cod == NULL)
		return(NULL);
	strcpy(nc->cod,s);
	nc->next_code = NULL;
	return(nc->cod);
}

static NSUInteger GetNextMorString(NSUInteger pos, unCH *st, DocumentWindowController *docWinController) {
	int i = 0, j;
	unichar ch;
	NSUInteger len;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSRange cursorRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	if (isLastCode)
		return(-1);

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	if (pos == -1) {
		cursorRange = [textView selectedRange];
		pos = cursorRange.location;
		while (pos > 0) {
			ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			if (ch == '\n' || isSpace(ch)) {
				pos++;
				break;
			}
			pos--;
		}
	}
	st[i] = EOS;
	if (pos <= len) {
		if (pos == len) {
			isLastCode = TRUE;
			strcpy(st, "?|");
			if (*templine3 != EOS)
				strcat(st, templine3);
			return(pos);
		} else {
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n' || isSpace(ch)) {
				isLastCode = TRUE;
				strcpy(st, "?|");
				if (*templine3 != EOS)
					strcat(st, templine3);
				return(pos);
			}
		}
	}
	if (pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '^') {
			pos++;
		}
	}
	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n' || ch == '^' || isSpace(ch)) {
			break;
		}
		st[i++] = ch;
		pos++;
	}
	st[i] = EOS;
	if (*templine3 == EOS) {
		for (i=0; st[i] && st[i] != '|'; i++) ;
		if (st[i])
			i++;
		for (j=0; st[i] && isalpha(st[i]); i++, j++) {
			templine3[j] = st[i];
		}
		templine3[j] = EOS;
	}
	return(pos);
}

static CODES *FillInCodesString(NSUInteger pos, int LastLevel, BOOL *isEnd, DocumentWindowController *docWinController) {
	int ThisLevel, offset, LocMaxNumOfCodes = 1;
	CODES *RootCode, *CurCode, *LastSubcode;

	LastSubcode = NULL;
	ThisLevel = LastLevel;
	RootCode = NEW(CODES);
	if (RootCode == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(NULL);
	}
	if (ced_line[ThisLevel] == '"')
		offset = 1;
	else
		offset = 0;
	RootCode->code = AddCodeToList(ced_line+ThisLevel+offset);
	if (RootCode->code == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(NULL);
	}
	RootCode->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
	RootCode->subcodes = NULL;
	RootCode->NextCode = NULL;
	CurCode = RootCode;
	while ((pos=GetNextMorString(pos,ced_line,docWinController)) != -1) {
		do {
			for (offset=0; isSpace(ced_line[offset]) || ced_line[offset] == '\n'; offset++) ;
			if (ced_line[offset] != EOS)
				break;
			if ((pos=GetNextMorString(pos,ced_line,docWinController)) == -1)
				goto contfillstr;
		} while (1) ;
		for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) ;
		if (ThisLevel > LastLevel) {
			if (ThisLevel > LastLevel+1) {
				do_warning_sheet("More than one space indentation between two codes", [docWinController window]);
				return(NULL);
			}
			CurCode->subcodes = FillInCodesString(pos,ThisLevel,isEnd,docWinController);
			if (CurCode->subcodes == NULL)
				return(NULL);
			LastSubcode = CurCode->subcodes;
			if (*isEnd)
				break;
			for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) ;
		} else if (*isEnd)
			break;

		if (ThisLevel < LastLevel) {
			if (LocMaxNumOfCodes>MaxNumOfCodes)
				MaxNumOfCodes=LocMaxNumOfCodes;
			return(RootCode);
		}
		LocMaxNumOfCodes++;
		CurCode->NextCode = NEW(CODES);
		if (CurCode->NextCode == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(NULL);
		}
		CurCode = CurCode->NextCode;
		if (ced_line[ThisLevel] == '"')
			offset = 1;
		else
			offset = 0;
		CurCode->code = AddCodeToList(ced_line+ThisLevel+offset);
		if (CurCode->code == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(NULL);
		}
		if (docWinController->CodingScheme == '\001')
			CurCode->subcodes = LastSubcode;
		else
			CurCode->subcodes = NULL;
		CurCode->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
		CurCode->NextCode = NULL;
	}
contfillstr:
	*isEnd = TRUE;
	if (LocMaxNumOfCodes > MaxNumOfCodes)
		MaxNumOfCodes = LocMaxNumOfCodes;
	return(RootCode);
}

static int init_codes_string(NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	BOOL isEnd;

	RootCodeList = NULL;
	if ((docWinController->RootCodes=NEW(CODES)) == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		docWinController->NoCodes = TRUE;
		return(0);
	}
	docWinController->RootCodes->code = NULL;
	docWinController->RootCodes->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
	docWinController->RootCodes->NextCode = NULL;
	pos = GetNextMorString(-1, ced_line, docWinController);
	
	if (pos == -1) {
		strcpy(ced_line,"    ");
		docWinController->NoCodes = TRUE;
		MaxNumOfCodes = 1;
		docWinController->RootCodes->subcodes = NEW(CODES);
		if (docWinController->RootCodes->subcodes == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			docWinController->NoCodes = TRUE;
			return(0);
		}
		docWinController->RootCodes->subcodes->code = AddCodeToList(ced_line);
		if (docWinController->RootCodes->subcodes->code == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(NULL);
		}
		docWinController->RootCodes->subcodes->subcodes = NULL;
		docWinController->RootCodes->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
		docWinController->RootCodes->subcodes->NextCode = NULL;
		MaxNumOfCodes++;
		docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodes;
		docWinController->CurCode = docWinController->RootCodes;
		docWinController->EditorMode = TRUE;
	} else {
		docWinController->NoCodes = FALSE;
		MaxNumOfCodes = 0;
		isEnd = false;
		docWinController->RootCodes->subcodes = FillInCodesString(pos,0,&isEnd,docWinController);
		if (docWinController->RootCodes->subcodes == NULL)
			docWinController->NoCodes = TRUE;
		MaxNumOfCodes++;
		docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodes;
		docWinController->CurCode = docWinController->RootCodes;
	}
	MaxNumOfCodes++;
	docWinController->RootCodesArr = (CODES **)malloc(sizeof(CODES *) * MaxNumOfCodes);
	if (docWinController->RootCodesArr == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		docWinController->NoCodes = TRUE;
		return(0);
	}
	docWinController->FirstCodeOfPrevLevel = docWinController->RootCodes;
	MapArrToList(docWinController->RootCodes, docWinController);
	strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	return(1);
}

static void HiLightMatch(NSUInteger pos, NSTextView *textView) {
	unichar ch;
	NSUInteger tPos, len;
	NSTextStorage *text;
	NSString *textSt;
	NSRange charRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	tPos = pos;
	while (tPos > 0) {
		ch = [textSt characterAtIndex:tPos];
#ifdef DEBUGCODE
if (tPos+40 >= len) dl = len-tPos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(tPos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n' || isSpace(ch)) {
			tPos++;
			break;
		}
		tPos--;
	}
	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n' || isSpace(ch)) {
			break;
		}
		pos++;
	}
	if (tPos < pos) {
#ifdef DEBUGCODE
[textSt getCharacters:bufU range:NSMakeRange(tPos, pos-tPos)]; bufU[pos-tPos] = EOS; // lxs debug
#endif
		charRange = NSMakeRange(tPos, pos-tPos);
		dispatch_async(dispatch_get_main_queue(), ^{
			[textView setSelectedRange:charRange];
			[textView scrollRangeToVisible:charRange];
		});
	}
}

static NSUInteger FindNextGivenTier(NSTextView *textView, NSUInteger pos, char *st) {
	unichar ch, lastCh;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
	const char *p;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	lastCh = '\n';
	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
		if (lastCh == '\n' && ch == st[0]) {
			p = st;
			while (pos < len) {
				ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				if (ch >= 'a' && ch <= 'z') {
					if (toupper(ch) != (unCH)*p)
						break;
				} else if (ch != *p)
					break;
				p++;
				pos++;
			}
			if (*p == ':') {
				if (ch == '\n')
					break;
			} else if (*p == EOS)
				break;
		}
		lastCh = ch;
		pos++;
	}
	return(pos);
}

static char isMatchCaret(NSTextView *textView, NSUInteger pos) {
	unichar ch;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
	NSRange charRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	if (pos < len) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '^') {
			pos++;
			if (pos < len) {
				ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				if (isalnum(ch)) {
					charRange = NSMakeRange(pos, 0);
					[textView setSelectedRange:charRange];
					[textView scrollRangeToVisible:charRange];
					return(1);
				}
			}
		}
	}
	return(0);
}

static int CaretMatched(NSTextView *textView, NSUInteger pos) {
	unichar ch;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];

	while (pos < len) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n')
			break;
		if (isMatchCaret(textView, pos))
			return(1);
		pos++;
	}
	return(0);
}

static BOOL FindCaretOnMor(NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	unichar ch;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
#ifdef DEBUGCODE
	if (pos+40 >= len) dl = len-pos; else dl = 40;
	[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	GetCurCode(textView, pos, docWinController);
	uS.uppercasestr(sp, &dFnt, FALSE);
	if (uS.partcmp(sp, DisTier, FALSE, FALSE)) {
		if (CaretMatched(textView, pos)) {
			GetCurCode(textView, pos, docWinController);
			FindRightCode(1, docWinController);
			return(true);
		} else {
			while (pos < len) {
				ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				if (ch == '\n') {
					pos++;
					break;
				}
				pos++;
			}
		}
	}
	if (pos < len) {
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (isSpeaker(ch))
			pos = FindNextGivenTier(textView, pos, DisTier);
	}
	while (pos < len) {
		if (CaretMatched(textView, pos)) {
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			GetCurCode(textView, pos, docWinController);
			FindRightCode(1, docWinController);
			return(true);
		}
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			if (ch == '\n') {
				pos++;
				break;
			}
			pos++;
		}
		if (pos < len) {
			ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			if (isSpeaker(ch))
				pos = FindNextGivenTier(textView, pos, DisTier);
		}
	}
	return(false);
}

static void getSpTierText(NSUInteger pos, NSTextView *textView) {
	int j;
	BOOL hcf;
	unichar ch, lastCh;
	NSUInteger len;
	NSTextStorage *text;
	NSString *textSt;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	j = 0;
	templine2[0] = EOS;
	if (pos > 0) {
		ch = [textSt characterAtIndex:pos];
		if (ch == '\n')
			pos--;
	}
	if (pos < len)
		lastCh = [textSt characterAtIndex:pos];
	else
		lastCh = 0;
	while (true) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (ch == '\n') {
			if (isMainSpeaker(lastCh))
				break;
		}
		lastCh = ch;
		if (pos == 0)
			break;
		pos--;
	}
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	if (!isMainSpeaker(lastCh))
		return;
	while (true) {
		hcf = false;


		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			if (ch == '\n') {
				break;
			}
			if (ch == 0x2022 || hcf) {
				if (ch == 0x2022)
					hcf = !hcf;
			} else if (ch == '\t')
				templine2[j++] = ' ';
			else
				templine2[j++] = ch;
			pos++;
		}
		if (pos >= len)
			break;
		pos++;
		if (pos >= len)
			break;
		ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		if (isSpeaker(ch))
			break;
	}
	templine2[j] = EOS;
}

int MorDisambiguate(DocumentWindowController *docWinController) {
	NSUInteger pos;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSRange cursorRange;

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;
	if (!FindCaretOnMor(textView, pos, docWinController)) {
		if (!docWinController->EditorMode)
 			[docWinController EditMode:docWinController];
		do_warning_sheet("Done.", [docWinController window]);
	} else {
		cursorRange = [textView selectedRange];
		pos = cursorRange.location;

		*ced_line = EOS;
		if (docWinController->cod_fname)
			free(docWinController->cod_fname);
		docWinController->cod_fname = NULL;
		FreeCodesMem(docWinController);
		isLastCode = FALSE;
		*templine3 = EOS;
		if (init_codes_string(textView, pos, docWinController)) {
			if (docWinController->EditorMode)
				[docWinController EditMode:docWinController];
			else
				docWinController->EndCodeArr = DisplayCodes(docWinController);
			HiLightMatch(pos, textView);
			getSpTierText(pos, textView);
			if (templine2[0] != EOS) {
				u_strcpy(ced_err_message, templine2, ERRMESSAGELEN-5);
			}
		}
	}
	return(69);
}

static void DeleteCodeIfEmpty(NSTextView *textView, NSUInteger pos, DocumentWindowController *docWinController) {
	BOOL found = false;
	unichar ch;
	NSString *keys;
	NSUInteger cnt, len;
	NSTextStorage *text;
	NSString *textSt;
	NSRange cursorRange;
#ifdef DEBUGCODE
NSUInteger dl;
unichar bufU[BUFSIZ];
#endif

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	pos = BeginningOfLine(pos, textSt);
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
	cnt = 0;
	ch = [textSt characterAtIndex:pos];
	cursorRange = NSMakeRange(pos, 0);
	if (ch == '%') {
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			cnt++;
			if (ch == ':' || ch == '\n')
				break;
			pos++;
		}
		if (ch == ':') {
			pos++;
			while (pos < len) {
				ch = [textSt characterAtIndex:pos];
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
				cnt++;
				if (ch == '\n')
					break;
				else if (!isSpace(ch)) {
					found = true;
					break;
				}
				pos++;
			}
		}
	} else
 		found = true;
	if (!found) {
		cursorRange.length = cnt;
#ifdef DEBUGCODE
if (cursorRange.length < BUFSIZ) // lxs debug
[textSt getCharacters:bufU range:cursorRange]; bufU[cursorRange.length] = EOS; // lxs debug
#endif
		keys = [NSString stringWithUTF8String:""];
		if ([textView shouldChangeTextInRange:cursorRange replacementString:keys]) {
			[text replaceCharactersInRange:cursorRange withString:keys];
			[textView didChangeText];
			[[textView undoManager] setActionName:@"Coder"];
#ifdef DEBUGCODE
if (cursorRange.location+40 >= [text length]) dl = [text length]-cursorRange.location; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(cursorRange.location, dl)]; bufU[dl] = EOS; // lxs debug
#endif
			cursorRange.length = 0;
			[textView setSelectedRange:cursorRange];
			[textView scrollRangeToVisible:cursorRange];
		}
	}
}

int EndCurTierGotoNext(int i, DocumentWindowController *docWinController) {
	BOOL found = false;
	unichar ch;
	unichar bufU[BUFSIZ];
	NSUInteger pos, len, codeLen;
	NSTextStorage *text;
	NSTextView *textView;
	NSString *textSt;
	NSRange cursorRange;
#ifdef DEBUGCODE
NSUInteger dl;
#endif

	if (docWinController->EditorMode && i != -1) {
		do_warning_sheet("This command works in coder mode only.", [docWinController window]);
		return(8);
	}

	textView = [docWinController firstTextView];
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	cursorRange = [textView selectedRange];
	pos = cursorRange.location;

	if (docWinController->CurCode != docWinController->RootCodes) {
		ToTopLevel(-1, docWinController);
		DeleteCodeIfEmpty(textView, pos, docWinController);
		docWinController->EnteredCode = '\0';
		cursorRange = [textView selectedRange];
		pos = cursorRange.location;
	}
	codeLen = strlen(NextTierName);
	if (docWinController->EnteredCode != '\001') {
		bufU[0] = EOS;
		while (pos < len) {
			ch = [textSt characterAtIndex:pos];
			if (ch == '\n') {
				if (pos+1 < len) {
					ch = [textSt characterAtIndex:pos+1];
#ifdef DEBUGCODE
if (pos+1+40 >= len) dl = len-pos+1; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos+1, dl)]; bufU[dl] = EOS; // lxs debug
#endif
					if (isMainSpeaker(ch)) {
						cursorRange.location = pos + 1;
						cursorRange.length = codeLen;
						if (NextTierName[0] == EOS) {
							found = true;
							pos++;
							break;
						}
						[textSt getCharacters:bufU range:cursorRange];
						bufU[codeLen] = EOS;
						if (uS.mStricmp(bufU, NextTierName) == 0) {
							found = true;
							pos++;
							break;
						}
					}
				}
			}
			pos++;
		}
	} else {
		cursorRange.location = pos;
		found = true;
		docWinController->EnteredCode = '\0';
	}
	if (found) {
		pos = EndOfLine(pos, textSt, len);
		cursorRange = NSMakeRange(pos, 0);
#ifdef DEBUGCODE
if (pos+40 >= len) dl = len-pos; else dl = 40;
[textSt getCharacters:bufU range:NSMakeRange(pos, dl)]; bufU[dl] = EOS; // lxs debug
#endif
		[textView setSelectedRange:cursorRange];
		[textView scrollRangeToVisible:cursorRange];
		GetCursorCode(i, docWinController);
	}
	return(8);
}

int FindRightCode(int disp, DocumentWindowController *docWinController) {
	register int i;

	if (docWinController->EditorMode)
		return(0);
	if (docWinController->NoCodes)
		return(0);
	if (docWinController->CurCode != docWinController->RootCodes)
		return(0);
	if (*sp == EOS) {
		if (disp)
			strcpy(sp, docWinController->OldCode);
		else {
			do_warning_sheet("Not pointing to a code tier!", [docWinController window]);
			return(0);
		}
	} else if (*sp == '*' || *sp == '@') {
		*sp = EOS;
		if (!disp)
			do_warning_sheet("Not pointing to a code tier!", [docWinController window]);
		return(0);
	}
	for (i=1; docWinController->RootCodesArr[i] != NULL; i++) {
		if (uS.partcmp(docWinController->RootCodesArr[i]->code, sp, FALSE, TRUE)) {
			if (docWinController->RootCodesArr[i] != docWinController->StartCodeArr[docWinController->CursorCodeArr]) {
				if (docWinController->StartCodeArr <= docWinController->RootCodesArr+i && docWinController->RootCodesArr+i <= docWinController->EndCodeArr) {
					if (disp && !docWinController->EditorMode) {
						docWinController->CursorCodeArr = i - (int)(docWinController->StartCodeArr-docWinController->RootCodesArr);
					} else
						docWinController->CursorCodeArr = i-(int)(docWinController->StartCodeArr-docWinController->RootCodesArr);
				} else {
					docWinController->StartCodeArr = docWinController->RootCodesArr + i;
					docWinController->CursorCodeArr = 0;
					if (disp)
						docWinController->EndCodeArr = DisplayCodes(docWinController);
				}
			}
			*sp = EOS;
			return(1);
		}
	}
	if (docWinController->cod_fname != NULL)
		do_warning_sheet("Undefined code tier!", [docWinController window]);
	*sp = EOS;
	return(0);
}

void MoveToSubcode(DocumentWindowController *docWinController) {
	CODES *tc, *OldSubCode;

	if (docWinController->PriorityCodes == '\001') {
		if (docWinController->StartCodeArr+docWinController->CursorCodeArr != docWinController->RootCodesArr+1) {
			(*(docWinController->StartCodeArr+(docWinController->CursorCodeArr-1)))->NextCode = 
											docWinController->StartCodeArr[docWinController->CursorCodeArr]->NextCode;
			docWinController->StartCodeArr[docWinController->CursorCodeArr]->NextCode = docWinController->RootCodesArr[0]->subcodes;
			OldSubCode = docWinController->RootCodesArr[0]->subcodes;
			docWinController->RootCodesArr[0]->subcodes = docWinController->StartCodeArr[docWinController->CursorCodeArr];
			for (tc=docWinController->FirstCodeOfPrevLevel; tc; tc=tc->NextCode) {
				if (tc->subcodes == OldSubCode)
					tc->subcodes = docWinController->StartCodeArr[docWinController->CursorCodeArr];
			}
		}
	} else if (docWinController->PriorityCodes == '\002') {
		if (docWinController->StartCodeArr+docWinController->CursorCodeArr != docWinController->RootCodesArr+1) {
			(*(docWinController->StartCodeArr+docWinController->CursorCodeArr-1))->NextCode = 
											docWinController->StartCodeArr[docWinController->CursorCodeArr]->NextCode;
			if (docWinController->StartCodeArr+docWinController->CursorCodeArr-1 == docWinController->RootCodesArr+1) {
				docWinController->StartCodeArr[docWinController->CursorCodeArr]->NextCode =
												docWinController->RootCodesArr[0]->subcodes;
				OldSubCode = docWinController->RootCodesArr[0]->subcodes;
				docWinController->RootCodesArr[0]->subcodes = docWinController->StartCodeArr[docWinController->CursorCodeArr];
				for (tc=docWinController->FirstCodeOfPrevLevel; tc; tc=tc->NextCode) {
					if (tc->subcodes == OldSubCode)
						tc->subcodes = docWinController->StartCodeArr[docWinController->CursorCodeArr];
				}
			} else {
				docWinController->StartCodeArr[docWinController->CursorCodeArr]->NextCode =
												(*(docWinController->StartCodeArr+docWinController->CursorCodeArr-2))->NextCode;
				(*(docWinController->StartCodeArr+docWinController->CursorCodeArr-2))->NextCode = 
												docWinController->StartCodeArr[docWinController->CursorCodeArr];
			}
		}
	}
	if (docWinController->StartCodeArr[docWinController->CursorCodeArr]->subcodes != NULL) {
		if (docWinController->CurCode == docWinController->RootCodes) {
			docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodesArr[0]->subcodes;
			docWinController->CurCode = docWinController->StartCodeArr[docWinController->CursorCodeArr];
		}
		docWinController->FirstCodeOfPrevLevel = docWinController->RootCodesArr[0]->subcodes;
		MapArrToList(docWinController->StartCodeArr[docWinController->CursorCodeArr], docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	} else if (docWinController->RootCodesArr[0] != docWinController->CurCode || docWinController->PriorityCodes) {
		docWinController->FirstCodeOfPrevLevel = docWinController->CurFirstCodeOfPrevLevel;
		MapArrToList(docWinController->CurCode, docWinController);
		docWinController->EndCodeArr = DisplayCodes(docWinController);
	}
}

static void replaceSpaces(unCH *code, int ThisLevel) {
	int  i, end;

	i = ThisLevel;
	if (code[i] == '"') {
		for (i++; isSpace(code[i]); i++) ;
	}
	if ((ThisLevel == 1 && code[i] == '$') || (ThisLevel > 1 && code[i] == ':' && i == ThisLevel)) {
		end = strlen(code) - 1;
		while (end >= 0 && (isSpace(code[end]) || code[end] == '\n' || code[end] == '\r'))
			end--;
		end++;
		for (; code[i] != EOS && i < end; i++) {
			if (isSpace(code[i]))
				code[i] = '_';
		}
	}
}

 static CODES *FillInCodes(FILE *fp, int LastLevel, int *end, long *ln, DocumentWindowController *docWinController) {
	unsigned long cnt;
	int ThisLevel, offset, LocMaxNumOfCodes = 1;
	CODES *RootCode, *CurCode, *LastSubcode;

	LastSubcode = NULL;
	ThisLevel = LastLevel;
	RootCode = NEW(CODES);
	if (RootCode == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(NULL);
	}

	if (ced_line[ThisLevel] == '"')
		offset = 1;
	else
		offset = 0;
	RootCode->code = AddCodeToList(ced_line+ThisLevel+offset);
	if (RootCode->code == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(NULL);
	}
	RootCode->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
	RootCode->subcodes = NULL;
	RootCode->NextCode = NULL;
	CurCode = RootCode;
	while (fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) {
		for (ThisLevel=0; isSpace(ced_lineC[ThisLevel]); ThisLevel++) {
			if (ced_lineC[ThisLevel] == '\t') {
				sprintf(ced_lineC, "ERROR: Tab character is found on line %ld in codes file.", *ln);
				do_warning_sheet(ced_lineC, [docWinController window]);
				return(NULL);
			}
		}
		if (ThisLevel == 1 && ced_lineC[ThisLevel] == '"' && ced_lineC[ThisLevel+1] == '$') {
			uS.shiftright(ced_lineC+ThisLevel+1, 1);
			ced_lineC[ThisLevel+1] = ' ';
			cnt++;
		}
		UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		(*ln)++;
		do {
			for (offset=0; isSpace(ced_line[offset]) || ced_line[offset] == '\n'; offset++) ;
			if (ced_line[offset] != EOS)
				break;
			(*ln)++;
			if (!fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt))
				goto contfill;
			UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		} while (1) ;
		offset = strlen(ced_line) - 1;
		ced_line[offset] = EOS;
		for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) {
			if (ced_line[ThisLevel] == '\t') {
				sprintf(ced_lineC, "ERROR: Tab character is found on line %ld in codes file.", *ln);
				do_warning_sheet(ced_lineC, [docWinController window]);
				return(NULL);
			}
		}
		replaceSpaces(ced_line, ThisLevel);

//printf("ced_line=%s; ThisLevel=%d;LastLevel=%d\n",ced_line,ThisLevel,LastLevel);

		if (ThisLevel > LastLevel) {
			if (ThisLevel > LastLevel+1) {
				sprintf(ced_lineC, "ERROR: Line %ld is indented by more than one space relative to previous line in codes file.", *ln);
				do_warning_sheet(ced_lineC, [docWinController window]);
				return(NULL);
			}
			if ((CurCode->subcodes=FillInCodes(fp,ThisLevel,end,ln,docWinController))== NULL)
				return(NULL);
			LastSubcode = CurCode->subcodes;
			if (*end)
				break;
			for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) {
				if (ced_line[ThisLevel] == '\t') {
					sprintf(ced_lineC, "ERROR: Tab character is found on line %ld in codes file.", *ln);
					do_warning_sheet(ced_lineC, [docWinController window]);
					return(NULL);
				}
			}
		} else if (*end)
			break;

		if (ThisLevel < LastLevel) {
			if(LocMaxNumOfCodes>MaxNumOfCodes) MaxNumOfCodes=LocMaxNumOfCodes;
			return(RootCode);
		}
		LocMaxNumOfCodes++;
		CurCode->NextCode = NEW(CODES);
		if (CurCode->NextCode == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(NULL);
		}
		CurCode = CurCode->NextCode;
		if (ced_line[ThisLevel] == '"')
			offset = 1;
		else
			offset = 0;
		CurCode->code = AddCodeToList(ced_line+ThisLevel+offset);
		if (CurCode->code == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(NULL);
		}
		if (docWinController->CodingScheme == '\001')
			CurCode->subcodes = LastSubcode;
		else
			CurCode->subcodes = NULL;
		CurCode->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
		CurCode->NextCode = NULL;
	}
contfill:
	*end = TRUE;
	if (LocMaxNumOfCodes > MaxNumOfCodes)
		MaxNumOfCodes = LocMaxNumOfCodes;
	return(RootCode);
}

void MapArrToList(CODES *CurCode, DocumentWindowController *docWinController) {
	int i = 0;

	docWinController->RootCodesArr[i++] = CurCode;
	if (CurCode != NULL) {
		for (CurCode=CurCode->subcodes; CurCode!= NULL; CurCode=CurCode->NextCode)
			docWinController->RootCodesArr[i++] = CurCode;
	}
	docWinController->RootCodesArr[i] = NULL;
	docWinController->StartCodeArr = docWinController->RootCodesArr + 1;
	docWinController->CursorCodeArr = 0;
}

// +bN: set number of commands and word before auto-save
// -d : do NOT create backup file
// +lN: re-order codes (1 = advance to top, 2 = advance one step)
// +sN: progam will make identical copies of codes across branches
// +tS: set next speaker name to S
// +xS: disambiguate tier S (default %s)", DisTier - "%MOR:"

BOOL init_codes(const FNType *fname, char *fontName, DocumentWindowController *docWinController) {
	int end;
	unsigned long cnt;
	char *res;
	long ln;
	FILE *fp;

	RootCodeList = NULL;
	if ((docWinController->RootCodes=NEW(CODES)) == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(false);
	}
	docWinController->RootCodes->code = NULL;
	docWinController->RootCodes->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
	docWinController->RootCodes->NextCode = NULL;

	fp = NULL;
	if (fname != NULL && *fname)
		fp = fopen(fname,"r");
	else
		fname = NULL;
	if (fp != NULL) {
		last_cr_char = 0;
		while ((res=fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) != NULL) {
			if (!uS.isUTF8(ced_lineC) && !uS.isInvisibleHeader(ced_lineC) &&
				strncmp(ced_lineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) != 0 &&
				ced_lineC[0] != ';' && ced_lineC[0] != '#' && ced_lineC[0] != '\n' && ced_lineC[0] != EOS)
				break;
		}
		if (res != NULL) {
			if (*ced_lineC == '\\') {
				end = 1;
				do {
					while (isSpace(ced_lineC[end]))
						end++;
					if (ced_lineC[end] == '-' || ced_lineC[end] == '+') {
						if (fontName != NULL) {
							if (ced_lineC[end+1] == 'f') {
								strcpy(fontName, ced_lineC+end+2);
								uS.remblanks(fontName);
							}
						}
						code_getflag(ced_lineC+end, docWinController);
					}
					while (!isSpace(ced_lineC[end]) && ced_lineC[end])
						end++;
				} while (ced_lineC[end]) ;
			}
		}
		fclose(fp);
	}
	if (fname == NULL || (fp=fopen(fname,"rb")) == NULL) {
		do_warning_sheet("Can't open codes file", [docWinController window]);
NoCodesFound:
		strcpy(ced_line,"	");
		docWinController->NoCodes = TRUE;
		MaxNumOfCodes = 1;
		docWinController->RootCodes->subcodes = NEW(CODES);
		if (docWinController->RootCodes->subcodes == NULL) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(false);
		}
		docWinController->RootCodes->subcodes->code = AddCodeToList(ced_line);
		if (docWinController->RootCodes->subcodes->code == nil) {
			do_warning_sheet("Out of memory!", [docWinController window]);
			return(false);
		}
		docWinController->RootCodes->subcodes->stringRect = CGRectMake(0.0, 0.0, 0.0, 0.0);
		docWinController->RootCodes->subcodes->subcodes = NULL;
		docWinController->RootCodes->subcodes->NextCode = NULL;
		MaxNumOfCodes++;
		docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodes;
		docWinController->CurCode = docWinController->RootCodes;
		docWinController->EditorMode = TRUE;
		goto codes_fin;
	} else
		docWinController->NoCodes = FALSE;

	MaxNumOfCodes = 0;
	last_cr_char = 0;
	ln = 0;
	while ((res=fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) != NULL) {
		ln++;
		if (!uS.isUTF8(ced_lineC) && !uS.isInvisibleHeader(ced_lineC) &&
			strncmp(ced_lineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) != 0 &&
			ced_lineC[0] != ';' && ced_lineC[0] != '#' && ced_lineC[0] != '\n' && ced_lineC[0] != EOS)
			break;
	}
	if (res != NULL) {
		UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		if (*ced_line == '\\') {
			while (1) {
				if (fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt) == NULL) {
					fclose(fp);
					goto NoCodesFound;
				} else {
					if (ced_lineC[0] != ';' && ced_lineC[0] != '#') {
						UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
						for (end=0; isSpace(ced_line[end]); end++) ;
						if (ced_line[end] != EOS && ced_line[end] != '\n')
							break;
					}
				}
			}
		}
		end = strlen(ced_line) - 1;
		ced_line[end] = EOS;
		if (end > 0) {
			if (ced_line[end-1] != '\t')
				strcat(ced_line, "\t");
		}
		for (end=0; isSpace(ced_line[end]); end++) ;
		if (end != 0) {
			do_warning_sheet("Wrong code format - top line.", [docWinController window]);
			fclose(fp);
			goto NoCodesFound;
		}
		end = FALSE;
		if ((docWinController->RootCodes->subcodes=FillInCodes(fp,0,&end,&ln,docWinController)) == NULL) {
			fclose(fp);
			goto NoCodesFound;
		}
	} else
		docWinController->RootCodes->subcodes = NULL;

	MaxNumOfCodes++;
	docWinController->CurFirstCodeOfPrevLevel = docWinController->RootCodes;
	docWinController->CurCode = docWinController->RootCodes;
	fclose(fp);
codes_fin:
	MaxNumOfCodes++;
	docWinController->RootCodesArr = (CODES **)malloc(sizeof(CODES *) * MaxNumOfCodes);
	if (docWinController->RootCodesArr == NULL) {
		do_warning_sheet("Out of memory!", [docWinController window]);
		return(false);
	}
	docWinController->FirstCodeOfPrevLevel = docWinController->RootCodes;
	MapArrToList(docWinController->RootCodes, docWinController);
	strcpy(docWinController->OldCode,docWinController->StartCodeArr[docWinController->CursorCodeArr]->code);
	return(true);
}

int GetNewCodes(int i, DocumentWindowController *docWinController) {
	const char *filename;
	char fontName[512];
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];

	// Create the File Open Dialog class.
	[openDlg setCanChooseFiles:YES];
	[openDlg setAllowsMultipleSelection:NO];
	[openDlg setCanChooseDirectories:NO];

	// Display the dialog. If the OK button was pressed, process the files.
	if ( [openDlg runModal] == NSModalResponseOK ) {
		// Get an array containing the full filenames of all
		// files and directories selected.
		NSArray* urls = [openDlg URLs];

		filename = nil;
		// Loop through all the files and process them.
		for(i=0; i < [urls count]; i++) {
			filename = [[[urls objectAtIndex:i] path] UTF8String];
		}
		if (filename == nil) {
			do_warning_sheet("Can't open codes file", [docWinController window]);
			if (i > 0)
				return(67);
			else
				return(0);
		}
	} else {
		if (i > 0)
			return(67);
		else
			return(0);
	}
	if (docWinController->cod_fname)
		free(docWinController->cod_fname);
	docWinController->cod_fname = (FNType *)malloc((strlen(filename)+1)*sizeof(FNType));
	if (docWinController->cod_fname == NULL) {
		if (i > 0)
			return(67);
		else
			return(0);
	}
	strcpy(docWinController->cod_fname, filename);
	FreeCodesMem(docWinController);
	fontName[0] = EOS;
	if (init_codes(docWinController->cod_fname, fontName, docWinController)) {
		if (fontName[0] != EOS) {
/* set specific font for coder window
			FName = 0;
			if (GetFontNumber(fontName, &FName)) {
				if (docWinController != NULL)
					w = docWinController->w2;
				else
					w = NULL;
				if (w != NULL) {
					for (i=0; i < w->num_rows; i++) {
						w->RowFInfo[i]->FName = FName;
					}
				}
			}
*/
		}
	}
	return(67);
}

