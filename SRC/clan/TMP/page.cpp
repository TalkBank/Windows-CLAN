char lversion[] = "(03/14/94)";

#include <stdio.h>
#ifdef MSC
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <io.h>
#include <bios.h>
#endif

extern void getflag(char *);
extern void work(int,char *);
extern int  fgets_cr(char *beg, int size, FILE *fp);

FILE *fpin;
char buf[BUFSIZ], lastcr;

void usage(void) {		/* print proper usage and exit */
    puts("page displays data one screen full at the time");
    puts("Usage: page [h ?] filename(s)");
    puts("+h : prints this help message.");
    puts("+? : prints this help message.");
    exit(1);
}

int fgets_cr(char *buf, int size, FILE *fp) {
    register int i = 1;
    
    size--;
    *buf = (char)getc(fp);
    if (feof(fp)) return(-1);
    do {
	if (*buf == '\r') {
	    *buf++ = '\n';
	    break;
	} else if (*buf == '\n') {
	    buf++;
	    break;
	}
	i++;
	buf++;
	if (i >= size) break;
	*buf = (char)getc(fp);
	if (feof(fp)) break;
    } while (1) ;
    *buf = '\0';
    return(i);
}

void call(void) {
    register int c;
    register int len;
    register int count;

    count = 1;
    while ((len=fgets_cr(buf,BUFSIZ,fpin)) > -1) {
	if (count % 24 == 0) {
#ifdef MSC
	    while (kbhit()) getch();
#endif
	    fputs("- More -", stderr);
	    do {
#ifdef MSC
		c = getch();
#endif
		if (c == 'q' || c == 'Q') exit(0);
		if (c == '\r') count = 23;
		else if (c == ' ') count = 1;
		else if (c == 'n' || c == 'N') {
		    fputs("\r        \r", stderr);
		    return;
		} else count = 24;
	    } while (count == 24) ;
	    fputs("\r        \r", stderr);
	}
	for (c=0; c < len; c++)
	    fputc(buf[c], stdout);
	if (buf[len-1] == '\n') lastcr = 1;
	else lastcr = 0;
	count += (len / 80) + 1;
    }
}

void getflag(char *f) {
	f++;
	switch(*f++)
	{
		case '?':
		case 'h':
			usage();
			break;
		default:
			fprintf(stderr,"Invalid flag: %s\n", f-2);
			exit(1);
	}
}

void work(int num, char *st) {
    register int c;

    if (num > 1) {
#ifdef MSC
	while (kbhit()) getch();
	if (!lastcr) putc('\n', stderr);
	fputs("press any key", stderr);
	c = getch();
	fputs("\r             \n", stderr);
	if (c == 'q' || c == 'Q') exit(0);
#endif
#ifdef VMS
	fputs("press RETURN key", stderr);
	getc(stdin);
#endif
    }
    if (fpin != stdin) {
#ifndef VMS
	if (access(st,0)) {
	    fprintf(stderr,
		"Can't locate file \"%s\" in a specified directory.\n\n", st);
	    return;
	}
#endif
	if ((fpin=fopen(st,"r")) == NULL) {
	    fprintf(stderr,"Can't open file %s.\n\n",st);
	    return;
	}
	fprintf(stderr,"******* From file <%s>\n",st);
	call();
	fclose(fpin);
    } else call();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    int i;

#ifdef VMS
#include <descrip.h>
#include <ssdef.h>
#include <rmsdef.h>
    int j;
    static $DESCRIPTOR(ff,"                                                                                                    \0");
    static $DESCRIPTOR(fnn,"                                                                                                    \0");
    static unsigned int context = 0;
#endif

    fpin = NULL;
#ifdef MSC
    if (!isatty(fileno(stdin))) fpin = stdin;
#endif
#ifdef UNX
    if (gtty(fileno(stdin), &otty) != 0) fpin = stdin;
#endif

    if (fpin == stdin) {
	work(1,"Stdin");
	exit(0);
    } else if (argc == 1) {
	usage();
    } else {
	if (argv[1][0] == '-' || argv[1][0] == '+') {
	    if (argv[1][1] == 'v') {
		if (argv[1][2] == '\0') {
		    printf("Version V%s; ",lversion);
#ifdef VMS
		    printf("VMS.\n");
#else
#ifdef MSC
		    printf("IBM.\n");
#else
#ifdef MAC
#ifdef THINK_C
		    printf("MAC using THINK C 6.0.\n");
#else
		    printf("MAC.\n");
#endif
#else
		    printf("Unix.\n");
#endif
#endif
#endif
		    exit(0);
		}
	    }
	}
    }

    lastcr = 1;
    for (i=1; i < argc; i++) {
	if (*argv[i] == '-') {
	    getflag(argv[i]);
	    continue;
	}
#ifdef VMS
	ff.dsc$w_length = strlen(argv[i]);
	ff.dsc$a_pointer = argv[i];
	while(LIB$FIND_FILE (&ff,&fnn,&context,0,0,0,0)!=RMS$_NMF) {
	    for (j=strlen(fnn.dsc$a_pointer)-1; 
		 j >= 0 && fnn.dsc$a_pointer[j] == ' '; j--) ;
	    fnn.dsc$a_pointer[j+1] = '\0';
	    work(i,fnn.dsc$a_pointer);
	}
	LIB$FIND_FILE_END (&context);
#else
	work(i,argv[i]);
#endif
    }
}
