#include "ced.h"
#include "c_clan.h"
#ifdef _WIN32 
	#include "CedDlgs.h"
#endif
#ifdef _MAC_CODE
	#include <cstdarg>
#endif

static long inIndex = 0L; 
FNType *StdInWindow = NULL;
const char *StdInErrMessage = NULL;
const char *StdDoneMessage  = NULL;
static char temp_print_str[UTTLINELEN+4];

static int print_str_cnt = 0;
static char print_str[UTTLINELEN+4];
static unCH uPrint_str[UTTLINELEN*4];
static unCH input_str[UTTLINELEN+6];


static void addToSTDOutput(char chr, char *st) {
	long len, sLen;

	if (st == NULL) {
		if (chr == '\n' || print_str_cnt >= UTTLINELEN) {
			print_str[print_str_cnt++] = chr;
			print_str[print_str_cnt] = EOS;
			UTF8ToUnicode((unsigned char *)print_str, print_str_cnt, uPrint_str, NULL, UTTLINELEN*4);
			print_str_cnt = 0;
			OutputToScreen(uPrint_str);
		} else {
			print_str[print_str_cnt++] = chr;
			print_str[print_str_cnt] = EOS;
		}
	} else {
		len = 0L;
repeatAdding:
		sLen = len;
		for (; st[len] != '\n' && st[len] != EOS && print_str_cnt < UTTLINELEN; len++) {
			print_str[print_str_cnt++] = st[len];
		}

		if (st[len] == '\n' || print_str_cnt >= UTTLINELEN) {
			if (st[len] == '\n') {
				print_str[print_str_cnt++] = st[len];
				len++;
			}
			print_str[print_str_cnt] = EOS;
			UTF8ToUnicode((unsigned char *)print_str, print_str_cnt, uPrint_str, NULL, UTTLINELEN*4);
			print_str_cnt = 0;
			OutputToScreen(uPrint_str);
			goto repeatAdding;
		} else {
			print_str[print_str_cnt] = EOS;
		}
	}
}

/*
char *my_gets(char *st) {
	*st = EOS;
	if (isKillProgram)
		return(st);

	my_flush_chr();
#ifdef _WIN32 
	CStdIn dlg;
	dlg.m_StdInput = _T("");
	if (dlg.DoModal() != IDOK)
		strcpy(st, ":q");
	else
		u_strcpy(st, dlg.m_StdInput, 1024);
	strcpy(input_str, dlg.m_StdInput);
	strcat(input_str, "\n");
	OutputToScreen(input_str);
#else
//	do_warning("gets(string) is not working.", 0);
	StdInWindow = global_df->fileName;
	DoStdInput(st);
	StdInWindow = NULL;
	StdInErrMessage = NULL;
	StdDoneMessage  = NULL;
#endif
	return(st);
}
*/

int my_getc(FILE *fp) {
	int c;

	if (isKillProgram)
		return(EOF);

	my_flush_chr();
	if (fp == stdin) {
		if (pipe_in->fp != NULL)
			c = getc(pipe_in->fp);
		else if (redirect_in.fp != NULL)
			c = getc(redirect_in.fp);
		else {
			if (inIndex == 0L) {
#ifdef _WIN32 
				CStdIn dlg;
				dlg.m_StdInput = _T("");
				if (dlg.DoModal() == IDOK)
					strcpy(input_str, dlg.m_StdInput);
				else
					input_str[0] = EOS;
				strcat(input_str, "\n");
				OutputToScreen(input_str);
				for (c=0; input_str[c] != (int)EOF && input_str[c] != (int)EOS &&
						  input_str[c] != (int)NL_C && input_str[c] != (int)'\n'; c++) {
					if (input_str[c] == NL_C)
						input_str[c] = '\n';
				}
				if (input_str[c] == NL_C)
					input_str[c] = '\n';
#else
				StdInWindow = global_df->fileName;
#ifndef _COCOA_APP
				DoCharStdInput(input_str);
#endif
				StdInWindow = NULL;
				StdInErrMessage = NULL;
				StdDoneMessage  = NULL;
#endif
				c = (int)input_str[inIndex++];
				if (c == (int)'\n' || c == (int)EOF || c == (int)EOS) {
					inIndex = 0L;
					if (c == (int)EOF || c == (int)EOS)
						c = (int)'\n';
				}
			} else {
				c = (int)input_str[inIndex++];
				if (c == (int)'\n' || c == (int)EOF || c == (int)EOS) {
					inIndex = 0L;
					if (c == (int)EOF || c == (int)EOS)
						c = (int)'\n';
				}
			}
		}
	} else
		c = getc(fp);
	return(c);
}

void my_flush_chr(void) {
	if (print_str_cnt > 0) {
		UTF8ToUnicode((unsigned char *)print_str, print_str_cnt, uPrint_str, NULL, UTTLINELEN*4);
		print_str_cnt = 0;
		OutputToScreen(uPrint_str);
	}
}

int my_feof(FILE *fp) {
	if (isKillProgram)
		return(TRUE);

	if (fp == stdin) {
		if (pipe_in->fp != NULL)
			return(feof(pipe_in->fp));
		else if (redirect_in.fp != NULL)
			return(feof(redirect_in.fp));
		else
			return(feof(fp));
	} else
		return(feof(fp));
}

int  my_isatty(FILE *fp) {
	if (fp == stdin) {
		if (pipe_in->fp != NULL || redirect_in.fp != NULL)
			return(0);
		else
			return(1);
	} else {
		if (pipe_out->fp != NULL || redirect_out.fp != NULL)
			return(0);
		else
			return(1);
	}
}

void my_rewind(FILE *fp) {
	if (isKillProgram) 
		return;

	if (fp == stdin) {
		if (pipe_in->fp != NULL)
			rewind(pipe_in->fp);
		else if (redirect_in.fp != NULL)
			rewind(redirect_in.fp);
		else
			rewind(fp);
	} else
		rewind(fp);
}

void my_fprintf(FILE * file, const char * format, ...) {
	va_list args;

	if (isKillProgram) 
		return;

	va_start(args, format); 	// prepare the arguments
	if (file == stdout) {
		if (pipe_out->fp != NULL)
			vfprintf(pipe_out->fp, format, args);
		else if (redirect_out.fp == NULL) {
			vsprintf(temp_print_str, format, args);
			addToSTDOutput(0, temp_print_str);
		} else
			vfprintf(redirect_out.fp, format, args);
	} else if (file == stderr) {
		if (redirect_out.fp == NULL || !redirect_out.all) {
			vsprintf(temp_print_str, format, args);
			addToSTDOutput(0, temp_print_str);
		} else
			vfprintf(redirect_out.fp, format, args);
	} else {
		if (isUnixCRs) {
			long i;
			vsprintf(temp_print_str, format, args);
			for (i=0L; temp_print_str[i]; i++) {
				if (temp_print_str[i] == '\n')
					temp_print_str[i] = 0x0A;
			}
			fputs(temp_print_str, file);
		} else
			vfprintf(file, format, args);
	}
	va_end(args);				// clean the stack
}

void my_printf(const char * format, ...) {
	va_list args;

	if (isKillProgram) 
		return;

	va_start(args, format); 	// prepare the arguments
	if (pipe_out->fp != NULL)
		vfprintf(pipe_out->fp, format, args);
	else if (redirect_out.fp == NULL) {
		vsprintf(temp_print_str, format, args);
		addToSTDOutput(0, temp_print_str);
	} else
		vfprintf(redirect_out.fp, format, args);
	va_end(args);				// clean the stack
}

void my_fputs(const char *format, FILE *file) {
	if (isKillProgram) 
		return;

	if (file == stdout) {
		if (pipe_out->fp != NULL)
			fputs(format, pipe_out->fp);
		else if (redirect_out.fp == NULL) {
			strcpy(temp_print_str, format);
			addToSTDOutput(0, temp_print_str);
		} else
			fputs(format, redirect_out.fp);
	} else if (file == stderr) {
		if (redirect_out.fp == NULL || !redirect_out.all) {
			strcpy(temp_print_str, format);
			addToSTDOutput(0, temp_print_str);
		} else
			fputs(format, redirect_out.fp);
	} else {
		if (isUnixCRs) {
			long i;
			strcpy(temp_print_str, format);
			for (i=0L; temp_print_str[i]; i++) {
				if (temp_print_str[i] == '\n')
					temp_print_str[i] = 0x0A;
			}
			fputs(temp_print_str, file);
		} else
			fputs(format, file);
	}
}

void my_puts(const char * format) {
	if (isKillProgram) 
		return;

	strcpy(temp_print_str, format);
	strcat(temp_print_str, "\n");
	if (pipe_out->fp != NULL)
		fputs(temp_print_str, pipe_out->fp);
	else if (redirect_out.fp == NULL) {
		addToSTDOutput(0, temp_print_str);
	} else
		fputs(temp_print_str, redirect_out.fp);
}

void my_putc(const char format, FILE *file) {
	if (isKillProgram) 
		return;

	if (file == stdout) {
		if (pipe_out->fp != NULL)
			putc(format, pipe_out->fp);
		else if (redirect_out.fp == NULL) {
 			addToSTDOutput(format, NULL);
		} else
			putc(format, redirect_out.fp);
	} else if (file == stderr) {
		if (redirect_out.fp == NULL || !redirect_out.all) {
 			addToSTDOutput(format, NULL);
		} else
			putc(format, redirect_out.fp);
	} else {
		if (isUnixCRs) {
			if (format == '\n')
				putc(0x0A, file);
			else
				putc(format, file);
		} else
			putc(format, file);
	}
}

void my_putchar(const char format) {
	if (isKillProgram) 
		return;

	if (pipe_out->fp != NULL)
		putc(format, pipe_out->fp);
	else if (redirect_out.fp == NULL) {
 		addToSTDOutput(format, NULL);
	} else
		putc(format, redirect_out.fp);
}
