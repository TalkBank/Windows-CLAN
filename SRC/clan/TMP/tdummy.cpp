#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 1

void init(char f) { fprintf(stderr,"Internal error with PLUG-IN\n"); }
void usage(void) { fprintf(stderr,"Internal error with PLUG-IN\n"); }
void getflag(char *f, char *f1, int *i) { fprintf(stderr,"Internal error with PLUG-IN\n"); }
void call(void) { fprintf(stderr,"Internal error with PLUG-IN\n"); }

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
/*
	// unix argv extender: *.* if bmain is not called
   if ((argc=expandArgv(argc, argv)) == 0) {
       exit(0);
   }
*/
 	fprintf(stderr,"Unknown command: %s\n",argv[0]);
	show_info(1);
}
