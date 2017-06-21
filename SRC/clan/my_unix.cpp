/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#if defined(_MAC_CODE)
	#include "0global.h"
	#include <unistd.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/stat.h>
	#include "wstring.h"
	#include "my_unix.h"
#endif

#if defined(_WIN32)
	#include "ced.h"
	#include "stdafx.h"
	#include "Clan2.h"
#endif

#if defined(_WIN32)
	static wchar_t wDirPathName[FNSize];
#endif

int my_chdir(const FNType *path) {
#ifdef UNX

	return(chdir(path));

#elif defined(_MAC_CODE)

	return(chdir(path));

#elif defined(_WIN32)

	u_strcpy(wDirPathName, path, FNSize);
	return(_wchdir(wDirPathName));

#endif
}

int my_mkdir(const FNType *path, mode_t mode) {
#ifdef UNX

	return(mkdir(path, mode));

#elif defined(_MAC_CODE)

	return(mkdir(path, mode));

#elif defined(_WIN32)

	u_strcpy(wDirPathName, path, FNSize);
	return(_wmkdir(wDirPathName));

#endif
}

int my_access(const FNType *path, int mode) {
#ifdef UNX

	return(access(path, mode));

#elif defined(_MAC_CODE)

	return(access(path, mode));

#elif defined(_WIN32)

	u_strcpy(wDirPathName, path, FNSize);
	return(_waccess(wDirPathName, mode));

#endif
}

int my_unlink(const FNType *path) {
#ifdef UNX

	return(unlink(path));

#elif defined(_MAC_CODE)

	return(unlink(path));

#elif defined(_WIN32)

	u_strcpy(wDirPathName, path, FNSize);
	return(_wunlink(wDirPathName));

#endif
}

int my_rename(const FNType *f1, const FNType *f2) {
#ifdef UNX

	return(rename(f1, f2));

#elif defined(_MAC_CODE)

	return(rename(f1, f2));

#elif defined(_WIN32)
	wchar_t wDirPathName2[FNSize];

	u_strcpy(wDirPathName,  f1, FNSize);
	u_strcpy(wDirPathName2, f2, FNSize);
	return(_wrename(wDirPathName, wDirPathName2));

#endif
}

char *my_getcwd(FNType *path, size_t len) {
#ifdef UNX

	return(getcwd(path, len));

#elif defined(_MAC_CODE)

	return(getcwd(path, len));

#elif defined(_WIN32)

	if (_wgetcwd(wDirPathName, len) == NULL)
		return(NULL);
	u_strcpy(path, wDirPathName, FNSize);
	return(path);

#endif
}

FILE *my_fopen(const FNType *name, const char *mode) {
#ifdef UNX

	return(fopen(name, mode));

#elif defined(_MAC_CODE)
/*
	char *lMode;

	if (mode[0] == 'w')
		lMode = "wb";
	else
		lMode = mode;
*/
	return(fopen(name, mode));

#elif defined(_WIN32)
	int i;
	wchar_t wMode[FNSize];

	if (name[0] == '\\' && name[1] != '\\')
		i = 1;
	else
		i = 0;
	u_strcpy(wDirPathName, name+i, FNSize);
	u_strcpy(wMode, mode, FNSize);
	return(_wfopen(wDirPathName, wMode));

#endif
}
