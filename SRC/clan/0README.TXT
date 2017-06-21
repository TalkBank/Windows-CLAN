CLAN has to be compiled as a 32 bit application.

To compile, first edit makefile with text editor and remove comment character from "CC" and "CFLAGS" lines that correspond to your Unix OS and your compiler.

Second, edit file "common.h", look for keyword "DEPDIR" and change the corresponding file path of second occurence of  "DEPDIR" to full path location where you will install the "lib" folder.

 CLAN is distributed in folder "unix-clan". After you compile CLAN all executable commands will be located in "unix-clan/unix/bin" folder. To run CLAN commands you need to set PATH on your system to "/<from root to unix-clan path>/unix-clan/unix/bin" folder or you can specify the full path to those commands in the "bin" folder at Unix prompt. Unix-clan also includes a "unix-clan/lib" folder. Lib folder has a few sample CHAT files that you can run CLAN commands on.

To test FREQ command, for example, you can specify command at Unix prompt:
	/<from root to unix-clan path>/unix-clan/unix/bin/freq   /<from root to unix-clan path>/unix-clan/lib/samples/sample.cha

To get Usage information for FREQ command type command at Unix prompt:
	/<from root to unix-clan path>/unix-clan/unix/bin/freq

Basically, to use Unix-clan you need to know how to navigate Unix directory structure and how to run Unix executable commands.



***********************************************
The following is very brief description about how to write CLAN programs:

The program has to include file cu.h, which contains the declarations of
}