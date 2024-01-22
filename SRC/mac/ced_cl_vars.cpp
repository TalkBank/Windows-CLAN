#include "cu.h"
#include "ced.h"

char isUnixCRs;

short TabSize = 8;
short DOSdata;

unCH  sp[SPEAKERLEN+1];
char  spC[SPEAKERLEN+1];
unCH  ced_line[UTTLINELEN+1]; // found uttlinelen
char  ced_lineC[UTTLINELEN+1];

char ced_version[32];

NewFontInfo dFnt, oFnt;
myFInfo *global_df;

FNType lib_dir[FNSize];
FNType mor_lib_dir[FNSize];
FNType wd_dir[FNSize]; // working directory
FNType od_dir[FNSize]; // output directory
