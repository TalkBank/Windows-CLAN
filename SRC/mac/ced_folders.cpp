#include "ced.h"
#include "c_clan.h"
#import "ListBoxController.h"

extern FNType CLAN_Folder_str[FNSize];

#define NUMFOLDERS 15
FNType  *wd_folders[NUMFOLDERS]; // 1
FNType  *mor_folders[NUMFOLDERS];// 2

void initFoldersList(void) {
	register short i;

	for (i=0; i < NUMFOLDERS; i++)
		wd_folders[i] = NULL;
	for (i=0; i < NUMFOLDERS; i++)
		mor_folders[i] = NULL;
}

void set_folders(int which, FNType *text) {
	register int i;

	if (text == NULL)
		return;

	if (which == 1) {
		for (i=0; i < NUMFOLDERS; i++) {
			if (wd_folders[i] == NULL) {
				wd_folders[i] = (FNType *)malloc((strlen(text)+1)*sizeof(FNType));
				if (wd_folders[i] != NULL)
					strcpy(wd_folders[i], text);
				return;
			}
		}
	} else if (which == 2) {
		for (i=0; i < NUMFOLDERS; i++) {
			if (mor_folders[i] == NULL) {
				mor_folders[i] = (FNType *)malloc((strlen(text)+1)*sizeof(FNType));
				if (mor_folders[i] != NULL)
					strcpy(mor_folders[i], text);
				return;
			}
		}
	}
}

void addToFolders(int which, FNType *dir) {
	register int i;
	FNType **cfolders;

	if (dir[0] == EOS)
		return;

	if (which == 1)
		cfolders = wd_folders;
	else if (which == 2)
		cfolders = mor_folders;
	else
		return;
	for (i=0; i < NUMFOLDERS; i++) {
		if (cfolders[i] == NULL)
			break;
		if (strcmp(cfolders[i], dir) == 0)
			return;
	}
	if (i >= NUMFOLDERS) {
		free(cfolders[0]);
		for (i=0; i < NUMFOLDERS-1; i++)
			cfolders[i] = cfolders[i+1];
		cfolders[i] = NULL;
	}
	cfolders[i] = (FNType *)malloc((strlen(dir)+1)*sizeof(FNType));
	if (cfolders[i] != NULL)
		strcpy(cfolders[i], dir);
	WriteClanPreference();
}

void write_folders_2011_2013(FILE *fp) {
	register int i;

	for (i=0; i < NUMFOLDERS; i++) {
		if (wd_folders[i] != NULL)
			fprintf(fp, "%d=%s\n", 2011, wd_folders[i]);
	}
	for (i=0; i < NUMFOLDERS; i++) {
		if (mor_folders[i] != NULL)
			fprintf(fp, "%d=%s\n", 2012, mor_folders[i]);
	}
}

void delFromFolderList(int index, int which) {
	int i;

	if (which == wdFolders) {
		if (index < 0 || index >= NUMFOLDERS)
			return;
		if (wd_folders[index] != NULL)
			free(wd_folders[index]);
		for (i=index; i < NUMFOLDERS-1; i++) {
			wd_folders[i] = wd_folders[i+1];
		}
		WriteClanPreference();
	} else if (which == mlFolders) {
		if (index < 0 || index >= NUMFOLDERS)
			return;
		if (mor_folders[index] != NULL)
			free(mor_folders[index]);
		for (i=index; i < NUMFOLDERS-1; i++) {
			mor_folders[i] = mor_folders[i+1];
		}
		WriteClanPreference();
	}
}

NSDictionary *get_next_obj_folder(NSUInteger *i, int which) {
	NSDictionary *addedObject;
	NSString *str;

	if (which == wdFolders) {
		while (*i < NUMFOLDERS) {
			if (wd_folders[*i] == NULL) {
				*i = *i + 1;
			} else {
				str = [NSString stringWithUTF8String:wd_folders[*i]];
				addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
				return(addedObject);
			}
		}
	} else if (which == mlFolders) {
		while (*i < NUMFOLDERS) {
			if (mor_folders[*i] == NULL) {
				*i = *i + 1;
			} else {
				str = [NSString stringWithUTF8String:mor_folders[*i]];
				addedObject = [NSDictionary dictionaryWithObjectsAndKeys:str, @"lines", nil];
				return(addedObject);
			}
		}
	}
	return(nil);
}

char *getFolderAtIndex(long clickedRow, int which) {
	if (clickedRow >= 0 && clickedRow < NUMFOLDERS) {
		if (which == wdFolders) {
			if (wd_folders[clickedRow] != NULL) {
				return(wd_folders[clickedRow]);
			}
		} else if (which == mlFolders) {
			if (mor_folders[clickedRow] != NULL) {
				return(mor_folders[clickedRow]);
			}
		}
	}
	return(nil);
}
