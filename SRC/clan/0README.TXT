CLAN has to be compiled as a 32 bit application.

To compile, first edit makefile with text editor and remove comment character from "CC" and "CFLAGS" lines that correspond to your Unix OS and your compiler.

Second, edit file "common.h", look for keyword "DEPDIR" and change the corresponding file path of second occurence of  "DEPDIR" to full path location where you will install the "lib" folder. To compile CLAN run Unix command "make".

 CLAN is distributed in folder "unix-clan". After you compile CLAN all executable commands will be located in "unix-clan/unix/bin" folder. To run CLAN commands you need to set PATH on your system to "/<from root to unix-clan path>/unix-clan/unix/bin" folder or you can specify the full path to those commands in the "bin" folder at Unix prompt. Unix-clan also includes a "unix-clan/lib" folder. Lib folder has a few sample CHAT files that you can run CLAN commands on.

To test FREQ command, for example, you can specify command at Unix prompt:
	/<from root to unix-clan path>/unix-clan/unix/bin/freq   /<from root to unix-clan path>/unix-clan/lib/samples/sample.cha

To get Usage information for FREQ command type command at Unix prompt:
	/<from root to unix-clan path>/unix-clan/unix/bin/freq

Basically, to use Unix-clan you need to know how to navigate Unix directory structure and how to run Unix executable commands.



***********************************************
The following is very brief description about how to write CLAN programs:

The program has to include file cu.h, which contains the declarations of
useful variables and structures. Cutt libraries provide file pointers to
the input data stream, be it stdin or a file, and to the output data stream,
be it stdout or a file. If the output goes to the file cutt derives the
output file name from the input file name specified by the user. The program
has to contain the following procedures, requested by the cutt:
	usage(): 	briefly describing the usage of the program.
	getflag():	procedure taking care of the local options.
	init():		procedure to initialize variables

Usage procedure should look something like this:

void usage() {
    puts("Freq creates a frequency word count");
    puts("For complete documentation on freq");
    printf("Usage: freq [o s %s] filename(s)\n",mainflgs());
    puts("-o : sort output by descending frequency");
    puts("-sS: word S to search for in a given file");
    mainusage(TRUE);
}

MAINFLGS - contains the list of extended options that could be used by the
program. This list is defined in the cutt library.

mainusage() - is a function in the cutt library which will produce a brief
description of all the extended options.

Getflag procedure should look something like this:

void getflag(f)		/* parses command line to set options */
char *f;
{
    f++;
    switch(*f++) {
        case 'o':
            sort = 1;
            break;
        case 's':
	    addword('i',f);
            break;
        default:
            maingetflag(f-2);
            break;
    }
}

maingetflag(f-2) - procedure is called if the option specified by the user
does not match any of the local option. This procedure will try to match the
given option to one of the extended options. If the math fails the error
message is printed, otherwise the appropriate code in the cutt library is
executed.

Init procedure should look something like this:

void init(first)
char first;
{
    if (first) {
	addword('\0',"+<//>");
	addword('\0',"+</>");
	addword('\0',"+0*");
	addword('\0',"+&*");
	addword('\0',"+xxx");
	addword('\0',"+yyy");
	addword('\0',"+www");
	addword('\0',"+zzz");
    } else {
	total = 0;
	different = 0;
    }
}

This procedure is called by the cutt library once at the beginning with the
argument set to 1, and then it is called once before each data file is
opened with argument set to 0.

The program must have a declaration of the following variables:

	char ext[];		- the extension of the output file
	char lversion[];	- current version number of the program
	char UttlineEqUtterance;- save a copy of the original data, or not
	char chatmode;		- in which mode(s) can the program run

Declaration of ext could look like this:

	char ext[] =  ".frq";

Declaration of lversion could look like this:

	char lversion[] = "3";


UttlineEqUtterance SHOULD be declared either like this:

	char UttlineEqUtterance = TRUE;

or like this:

	char UttlineEqUtterance = FALSE;

When UttlineEqUtterance is set to TRUE the copy of the input data is NOT
kept. Thus if the program has to output the turn, as found in the input
data, the UttlineEqUtterance should be set to FALSE. uttline is a library
variable which points to a working version of the current turn.
utterance->line points to the original version of the current turn if
UttlineEqUtterance is set to FALSE. Otherwise they both point to a working
version of the turn. If the WINDOWS constant is defined the original copy is
always saved.


Declaration of lversion could look like this:

	char chatmode = 1;

If chatmode is set to 0 then the program can work on a non-CHAT data files
only.

If chatmode is set to 1 then the program can work on a non-CHAT files as
well as a CHAT files. The -y option will specify that the program is going
to work on a non-CHAT files. By default the program will assume that the
data files are in a CHAT format. The cutt library routines will perform a
simple test on data files to see if they are indeed in a CHAT format. If the
test fails the user will be advised to use the -y option. The test will
fail if at least one error is found in the first TESTLIM turns.  TESTLIM is
a library local constant, which specifies how many turn should be tested
before declaring the data to be in a CHAT format.

If chatmode is set to 2 then the program can work on a CHAT files only. The
-y option can NOT be used with this program. The program will always assume
that the data files are in a CHAT format. The cutt library routines will
perform a simple test on data files to see if they are indeed in a CHAT
format. If the test fails the user is advised that this program can NOT be
run on a non-CHAT files. The test will fail if at least one error is found
in the first TESTLIM turns.

If chatmode is set to 3 then the program will perform the same way as if the
chatmode is set to 1, except that during the test of the data file, the test
will NOT fail unless there are more than TESTLIM errors are found in the
data file. This is useful for the chstring type of programs, where the data
file is close enough to a complete CHAT format, but it still has few errors.

