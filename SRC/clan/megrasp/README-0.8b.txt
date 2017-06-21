MEGRASP -- The Maximum Entropy GR Parser for CHILDES transcripts
Version 0.8a

Kenji Sagae

lastname [at] usc [dot] edu
University of Southern California

July 2009

Based on GRASP, aka childesparser, developed by
Kenji Sagae 
at Carnegie Mellon University

Includes code from SSMaxEnt, written by Yoshimasa Tsuruoka 
at the University of Tokyo (now at the University of 
Manchester).

----------------------------------------------------------------

QUICK START

1. Unpack the tar.gz or .zip archive (see below).

2. Find the executable for your platform.  For example, the
   i386 Linux executable is called megrasp-linux.

   If an executable for your plataform is not available,
   compile (see below).

3. Assuming you have a file named test.cha, you can run
   % ./megrasp test.cha > output.cha
   to create an output file called output.cha.
   
   This uses a model provided with the distribution,
   megrasp.mod.  If option -t is used and a new model
   name is not specified, the provided model will be
   overwritten.  To retrain the parser, see command line
   options below.

----------------------------------------------------------------

UNPACKING

% gunzip megrasp0.8b.tar.gz
% tar xvf megrasp0.8b.tar

COMPILING

% make megrasp

This produces an executable called megrasp.
This has only been tested on Linux, but it
should work fine on cygwin and MacOSX.

USAGE

% ./megrasp [-g str] [-G str] [-p str] [-m str] [-L int] [-e]
	    [-t [-i int] [-c float]] inputfile

inputfile may be a filename (file must be in CHAT format, 
with part-of-speech tags produced by MOR and POST), or a 
sequence of filenames, or a pattern using wildcards (for 
example: *.cha).  Use the -o option (see below) only if 
using a single file name.

If inputfile is omitted, the parser reads input from STDIN
and writes output to STDOUT.  If inputfile is specified, 
the parser creates a new output file with the same name as
the input file with the additional extension .txt (unless 
the -o option is used to set the name of the output file).

Command line options:

-g str:	prefix for output lines (default: xgra)
	For example, if the option '-g abc' is given to the parser,
	output lines will begin with %abc: followed by a tab character
	(the default is to use the standard %xgra:). 

-G str:	prefix for GR training lines (default: xgrt)
	GR training lines have the same format as output lines, but
	are assumed to be correct.

-p str:	prefix for POS lines, must be three characters (default: mor)
	Other choices that make sense here include trn and pos.
	Ambiguous tags are NOT supported.

-m str:	model name (default: megrasp.mod).	

-o str: output file name.  If this option is not used, the output
	file name is the input file name with a ".txt" appended to
	it (if input is being taken from STDIN, output is written
	to STDOUT).

-L int: number of characters before output line wraps (default: 80)
	If L is set to zero, line-wrapping is turned off, and
	line breaks never interrupt output lines.

-e:	evaluate accuracy (input file must contain gold-standard GRs)

-t: training mode (parser runs in parse mode by default)
	Use this option only to train the parser using a file that
	contains GR training lines (usually %xgrt:).  When the parser
	is used with the -t option, it writes warnings to STDERR, 
	which can be captured to a file like this:
	./megrasp -t myfile.cha >& log.txt

The next two options are used in training mode only (with the -t option).
If the short descriptions don't make sense, it's best to leave those
alone, or see Kazama & Tsujii (EMNLP 2003).

-i int:	number of iterations for training the ME model (default: 300)
-c float: inequality parameter for training the ME model with
	inequality constraints (default: 1.0).


---------------------------

Changes

10 JUL 2009, v0.8a: changed default tier names (xgrt, xgra, mor)
10 JUL 2009, v0.8a: changed default output file name (add .txt)
17 MAR 2008, v0.7a: added evaluation option (-e)
19 NOV 2007, v0.7: made the "previous action" feature part of the parser state.

