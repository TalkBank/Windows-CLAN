#include "ced.h"
#include "cu.h"
#include "ced_unzip.h"
#import "ListBoxController.h"
#include <sys/stat.h>
#include <dirent.h>


extern FNType home_dir[];
extern FNType prefsDir[];

// 2020-07-24 beg
@implementation downLoadController

/*
 English - eng
 Cantonese - yue
 Chinese - zho
 Danish - dan
 Dutch - nld
 French - fra
 German - deu
 Hebrew - heb
 Italian - ita
 Japanese - jpn
 Spanish - spa
*/
- (IBAction)morGrammarEng:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("eng", 603000L);
}

- (IBAction)morGrammarYue:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("yue",363000L);
}

- (IBAction)morGrammarZho:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("zho", 416000L);
}

- (IBAction)morGrammarDan:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("dan", 589000L);
}

- (IBAction)morGrammarNld:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("nld", 100000L);
}

- (IBAction)morGrammarFra:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("fra", 346000L);
}

- (IBAction)morGrammarDeu:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("deu", 1700000L);
}

- (IBAction)morGrammarHeb:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("heb",1100000L);
}

- (IBAction)morGrammarIta:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("ita", 282000L);
}

- (IBAction)morGrammarJpn:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("jpn",1000000L);
}

- (IBAction)morGrammarSpa:(id)sender
{
#pragma unused (sender)
	GetMORGrammar("spa", 399000L);
}
@end
// 2020-07-24 end

char UnZipFolder(FNType *zipFName, FNType *destination) {
	strcpy(templineC1, "unzip -o \"");
	strcat(templineC1, zipFName);
	strcat(templineC1, "\" -d \"");
	strcat(templineC1, destination);
	strcat(templineC1, "\"");
	if (system(templineC1) == 0)
		return(TRUE);
	else
		return(FALSE);
}

// DEL directory BEG
/*
struct mor_FileList {
	FNType fname[FNSize];
	struct mor_FileList *next_file;
};
typedef struct mor_FileList mor_FileList;

static void free_morFileList(mor_FileList *p) {
	mor_FileList *t;

	while (p != NULL) {
		t = p;
		p = p->next_file;
		free(t);
	}
}

static mor_FileList *mor_addToFileList(mor_FileList *root_file, const FNType *fname) {
	mor_FileList *tF;
	extern char *MEMPROT;

	tF = NEW(mor_FileList);
	if (tF == NULL) {
		return(NULL);
	}
	if (root_file == NULL) {
		root_file = tF;
		tF->next_file = NULL;
	} else {
		tF->next_file = root_file;
		root_file = tF;
	}
	strcpy(tF->fname, fname);
	return(root_file);
}

static char del_dir(FNType *path) {
	int index;
	FNType fname[FILENAME_MAX];
	int  dPos;
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
	mor_FileList *tFile;
	mor_FileList *root_file;

	dPos = strlen(path);
	if (SetNewVol(path))
		return(TRUE);
	root_file = NULL;
	index = 1;
	while ((index = Get_File(fname, index)) != 0) {
		root_file = mor_addToFileList(root_file, fname);
		if (root_file == NULL)
			return(FALSE);
	}

	for (tFile = root_file; tFile != NULL; tFile = tFile->next_file) {
		unlink(tFile->fname);
	}
	free_morFileList(root_file);

	if ((cDIR=opendir(".")) != NULL) {
		while ((dp=readdir(cDIR)) != NULL) {
			if (stat(dp->d_name, &sb) == 0) {
				if (!S_ISDIR(sb.st_mode)) {
					continue;
				}
			} else
				continue;
			if (dp->d_name[0] == '.')
				continue;
			addFilename2Path(path, dp->d_name);
			uS.str2FNType(path, strlen(path), PATHDELIMSTR);
			if (!del_dir(path)) {
				path[dPos] = EOS;
				closedir(cDIR);
				return(FALSE);
			}
			path[dPos] = EOS;
			SetNewVol(path);
		}
		closedir(cDIR);
	}
	return(TRUE);
}
*/
// DEL directory END


char DownloadURL(char *url, unsigned long maxSize, char *memBuf, int memBufLen, char *fname, char isRunIt, char isRawUrl, const char *mess) {
	strcpy(templineC4, "curl \"");
	strcat(templineC4, url);
	strcat(templineC4, "\" -s -o \"");
	strcat(templineC4, fname);
	strcat(templineC4, "\"");
	if (!system(templineC4)) {
		return(TRUE);
	}
	//	strcpy(curlCommand, "curl https://talkbank.org/morgrams/eng.zip -s -o eng.zip");
	return(FALSE);
}

// MOR Grammar BEG
static bool DownloadGrammar(const char *grName, char *destName, size_t fileSize) {
	int  len, i;
	char *s;
	char URLPath[512];
	FILE *fp;

	FSRef  tRef;

	if (FSFindFolder(kOnSystemDisk,kDesktopFolderType,kDontCreateFolder,&tRef) == noErr) {
		my_FSRefMakePath(&tRef, destName, FNSize);
		addFilename2Path(destName, "MOR");
		if (access(destName, 0)) {
			if (my_mkdir(destName, S_IRWXU|S_IRGRP|S_IROTH))
				strcpy(destName, prefsDir);
		}
	} else
//		strcpy(destName, home_dir);
		strcpy(destName, prefsDir);
	len = strlen(destName);
	addFilename2Path(destName, grName);
	strcat(destName, ".zip");
	fp = fopen(destName, "w");
	if (fp == NULL) {
		strcpy(destName, mor_lib_dir);
		i = strlen(destName) - 1;
		if (destName[i] == PATHDELIMCHR)
			destName[i] = EOS;
		s = strrchr(destName, PATHDELIMCHR);
		if (s != NULL) {
			*(s + 1) = EOS;
			len = strlen(destName);
			addFilename2Path(destName, grName);
			strcat(destName, ".zip");
			fp = fopen(destName, "w");
		}
	}
	if (fp != NULL) {
		fclose(fp);
		unlink(destName);
		strcpy(URLPath, "https://talkbank.org/morgrams/");
		strcat(URLPath, grName);
		strcat(URLPath, ".zip");
		if (DownloadURL(URLPath, fileSize, NULL, 0, destName, FALSE, TRUE, "Downloading MOR Grammar...")) {
			destName[len] = EOS;
			return(TRUE);
		} else {
			unlink(destName);
		}
	}
	destName[len] = EOS;
	return(FALSE);
}

void GetMORGrammar(const char *grammar, size_t fileSize) {
	int  i;
	FNType zipFName[FNSize];

	if (DownloadGrammar(grammar, DirPathName, fileSize) == true) {
// delete old grammar location first
		strcpy(FileName1, home_dir);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, ".zip");
		unlink(FileName1);
		strcpy(FileName1, "rm -rf \"");
		strcat(FileName1, home_dir);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, "\"");
		system(FileName1);
// delete old grammar location first

		i = strlen(DirPathName) - 1;
		// BEG empty current MOR directory
		strcpy(FileName1, "rm -rf \"");
		strcat(FileName1, DirPathName);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, "\"");
		system(FileName1);
/*
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, PATHDELIMSTR);
		del_dir(FileName1);
		SetNewVol(wd_dir);
*/
		// END empty current MOR directory
		strcpy(zipFName, DirPathName);
		addFilename2Path(zipFName, grammar);
		strcat(zipFName, ".zip");
		if (UnZipFolder(zipFName, DirPathName)) {
			unlink(zipFName);
			strcpy(zipFName, DirPathName);
			addFilename2Path(zipFName, grammar);
			SetWdLibFolder(zipFName, mlFolders);
		} else {
			do_warning("Can't unzip grammar archive. Please try to download and install it by hand.", 0);
		}
	} else {
		do_warning("Can't download MOR grammar. Please try to download and install it by hand.", 0);
	}
}
// MOR Grammar END

// KIDEVAL DB BEG
/*
static bool getPreinstalledDatabase(const char *database, char *zipFName) {
	char isFoundZip;

	isFoundZip = FALSE;
	strcpy(zipFName, home_dir);
	addFilename2Path(zipFName, "lib/kideval");
	addFilename2Path(zipFName, database);
	strcat(zipFName, ".zip");
	if (access(zipFName, 0) == 0)
		isFoundZip = TRUE;
	else {
		strcpy(zipFName, lib_dir);
		addFilename2Path(zipFName, "kideval");
		addFilename2Path(zipFName, database);
		strcat(zipFName, ".zip");
		if (access(zipFName, 0) == 0)
			isFoundZip = TRUE;
	}
	if (isFoundZip) {
		strcpy(FileName1, prefsDir);
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		strcpy(FileName1, prefsDir);
		if (UnZipFolder(zipFName, FileName1)) {
//			unlink(zipFName);
// DOWNLOAD DB begin
//			strcpy(zipFName, prefsDir);
//			addFilename2Path(zipFName, database);
//			strcat(zipFName, ".1");
//			unlink(zipFName);
//			addFilename2Path(FileName1, database);
//			if (rename(FileName1, zipFName) == 0)
//				return(TRUE);
// DOWNLOAD DB end
		} else {
			do_warning("Can't unzip grammar archive. Please try to download and install it by hand.", 0);
		}
	}
	return(FALSE);
}
*/
/* DOWNLOAD DB
static void compareDatesAndChooseOne(const char *database) {
	char dateOne[256], *s;
	FILE *fp;

	dateOne[0] = EOS;
	strcpy(FileName1, prefsDir);
	addFilename2Path(FileName1, database);
	strcat(FileName1, ".1");
	if ((fp=fopen(FileName1, "r")) != NULL) {
		if (fgets_cr(templineC, UTTLINELEN, fp)) {
			strncpy(dateOne, templineC+3, 256);
			dateOne[255] = EOS;
			s = strchr(dateOne, ',');
			if (s != NULL)
				*s = EOS;
		}
		fclose(fp);
	}
	if (dateOne[0] != EOS) {
		strcpy(FileName2, prefsDir);
		addFilename2Path(FileName2, database);
		templineC[0] = EOS;
		if ((fp=fopen(FileName2, "r")) != NULL) {
			if (fgets_cr(templineC, UTTLINELEN, fp)) {
				strcpy(templineC, templineC+3);
				s = strchr(templineC, ',');
				if (s != NULL)
					*s = EOS;
			}
			fclose(fp);
		}
		if (templineC[0] != EOS) {
			if (strcmp(dateOne, templineC) > 0) {
				unlink(FileName2);
				rename(FileName1, FileName2);
			} else {
				unlink(FileName1);
			}
		}
	}
}

static bool DownloadDatabase(const char *database, char *destName, size_t fileSize) {
	int  len;
	char URLPath[512];
	FILE *fp;

	strcpy(destName, prefsDir);
	len = strlen(destName);
	addFilename2Path(destName, database);
	strcat(destName, ".zip");
	fp = fopen(destName, "w");
	if (fp != NULL) {
		fclose(fp);
		unlink(destName);
		strcpy(URLPath, "https://talkbank.org/clan/kideval-db/");
		strcat(URLPath, database);
		strcat(URLPath, ".zip");
		if (DownloadURL(URLPath, fileSize, NULL, 0, destName, FALSE, TRUE, "Downloading KIDEVAL database...")) {
			destName[len] = EOS;
			return(TRUE);
		} else {
			unlink(destName);
		}
	}
	destName[len] = EOS;
	return(FALSE);
}
*/
void GetKidevalDB(const char *database, size_t fileSize) {
//	char res;
//	FNType zipFName[FNSize];

	do_warning("Done.", 0);
//	res = getPreinstalledDatabase(database, zipFName);

/* DOWNLOAD DB
	if (DownloadDatabase(database, DirPathName, fileSize) == true) {
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, "kideval");
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, "eng_kideval_db.cut");
		unlink(FileName1);
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		strcpy(zipFName, DirPathName);
		addFilename2Path(zipFName, database);
		strcat(zipFName, ".zip");
		if (UnZipFolder(zipFName, DirPathName)) {
			unlink(zipFName);
			if (res)
				compareDatesAndChooseOne(database);
		} else {
			do_warning("Can't unzip grammar archive. Please try to download and install it by hand.", 0);
		}
	} else if (res) {
		strcpy(FileName2, prefsDir);
		addFilename2Path(FileName2, database);
		if (access(FileName2, 0) == 0) {
			do_warning("Can't download KIDEVAL database. Please try later.", 0);
		} else {
			strcpy(FileName1, prefsDir);
			addFilename2Path(FileName1, database);
			strcat(FileName1, ".1");
			rename(FileName1, FileName2);
		}
	} else {
		do_warning("Can't download KIDEVAL database. Please try later.", 0);
	}
*/
}
// KIDEVAL DB END
