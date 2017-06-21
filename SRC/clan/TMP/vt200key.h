/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "my_ctype.h"

#ifdef STRICT_PROTO
int VT200key(char c, WINDOW *w);
#else
int VT200key();
#endif

#define	ESC	27
 
#ifndef VMS
#define CSI	155
#define SS3	143
#else
#define CSI	-101
#define SS3	-113
#endif
 
/* Editing Keys (CSI) */
#define	KEY_FIND		255+1
#define	KEY_INSERT_HERE		255+2
#define	KEY_REMOVE		255+3
#define	KEY_SELECT		255+4
#define	KEY_PREV_SCREEN		255+5
#define	KEY_NEXT_SCREEN		255+6
 
/* Cursor Control Keys (CSI || SS3) */
#define	KEY_ARROW_UP		255+'A'
#define	KEY_ARROW_DOWN		255+'B'
#define	KEY_ARROW_RIGHT		255+'C'
#define	KEY_ARROW_LEFT		255+'D'
 
/* Auxiliary Keypad Keys (SS3) */
#define	KEY_KEYPAD_0		255+'p'
#define	KEY_KEYPAD_1		255+'q'
#define	KEY_KEYPAD_2		255+'r'
#define	KEY_KEYPAD_3		255+'s'
#define	KEY_KEYPAD_4		255+'t'
#define	KEY_KEYPAD_5		255+'u'
#define	KEY_KEYPAD_6		255+'v'
#define	KEY_KEYPAD_7		255+'w'
#define	KEY_KEYPAD_8		255+'x'
#define	KEY_KEYPAD_9		255+'y'
 
#define	KEY_KEYPAD_MINUS	255+'m'
#define	KEY_KEYPAD_COMMA	255+'l'
#define	KEY_KEYPAD_PERIOD	255+'n'
#define	KEY_KEYPAD_ENTER	255+'M'
 
#define	KEY_KEYPAD_PF1		255+'P'
#define	KEY_KEYPAD_PF2		255+'Q'
#define	KEY_KEYPAD_PF3		255+'R'
#define	KEY_KEYPAD_PF4		255+'S'
 
/* Top Row Function Keys (CSI) */
 
#define	KEY_F6			255+17
#define	KEY_F7			255+18
#define	KEY_F8			255+19
#define	KEY_F9			255+20
#define	KEY_F10			255+21
#define	KEY_F11			255+23
#define	KEY_F12			255+24
#define	KEY_F13			255+25
#define	KEY_F14			255+26
#define	KEY_HELP		255+28
#define	KEY_DO			255+29
#define	KEY_F17			255+31
#define	KEY_F18			255+32
#define	KEY_F19			255+33
#define	KEY_F20 		255+34

#ifdef VT200KEY
int VT200key(int c, WINDOW *win)
{
     int cn[5], oc;
     int  rc, fn, i;
     
     rc= -1;
     oc=c;
 
     if (c==ESC) {
     	cn[++rc]=wgetch(win);
        switch (cn[rc])
        {
        case '[': c = CSI;
                  break;
        case 'O': c = SS3;
                  break;
        default:  goto nosuchkey;
        }
     }
     	
     switch(c)
     {
     case SS3:	cn[++rc]=wgetch(win);
     		switch(cn[rc]) 
                {
     		case 'A': 	return(KEY_ARROW_UP);
     		case 'B':	return(KEY_ARROW_DOWN);
     		case 'C':	return(KEY_ARROW_RIGHT);
     		case 'D':	return(KEY_ARROW_LEFT);
 
     		case 'p':	return(KEY_KEYPAD_0);
     		case 'q':	return(KEY_KEYPAD_1);
     		case 'r':	return(KEY_KEYPAD_2);
     		case 's':	return(KEY_KEYPAD_3);
     		case 't':	return(KEY_KEYPAD_4);
     		case 'u':	return(KEY_KEYPAD_5);
     		case 'v':	return(KEY_KEYPAD_6);
     		case 'w':	return(KEY_KEYPAD_7);
     		case 'x':	return(KEY_KEYPAD_8);
     		case 'y':	return(KEY_KEYPAD_9);
 
     		case 'm':	return(KEY_KEYPAD_MINUS);
     		case 'l':	return(KEY_KEYPAD_COMMA);
     		case 'n':	return(KEY_KEYPAD_PERIOD);
     		case 'M':	return(KEY_KEYPAD_ENTER);
 
     		case 'P':	return(KEY_KEYPAD_PF1);
     		case 'Q':	return(KEY_KEYPAD_PF2);
     		case 'R':	return(KEY_KEYPAD_PF3);
     		case 'S':	return(KEY_KEYPAD_PF4);
 
		default:	goto nosuchkey;
     		}
     case CSI:	cn[++rc]=wgetch(win);
                switch(cn[rc])
                {
     		case 'A': 	return(KEY_ARROW_UP);
     		case 'B':	return(KEY_ARROW_DOWN);
     		case 'C':	return(KEY_ARROW_RIGHT);
     		case 'D':	return(KEY_ARROW_LEFT);
		default:	if (isdigit(cn[rc]))	fn = cn[rc] - '0';
				else 			goto nosuchkey;
		}
		cn[++rc]=wgetch(win);
		if (isdigit(cn[rc])) { 
			fn = fn*10 + cn[rc] - '0';
			cn[++rc]=wgetch(win);
		}
		if (cn[rc]=='~') switch(fn)
			{
			case 1:	return(KEY_FIND);
			case 2:	return(KEY_INSERT_HERE);
			case 3:	return(KEY_REMOVE);
			case 4:	return(KEY_SELECT);
			case 5:	return(KEY_PREV_SCREEN);
			case 6:	return(KEY_NEXT_SCREEN);
			
			case 17:	return(KEY_F6);
			case 18:	return(KEY_F7);
			case 19:	return(KEY_F8);
			case 20:	return(KEY_F9);
			case 21:	return(KEY_F10);
			case 23:	return(KEY_F11);
			case 24:	return(KEY_F12);
			case 25:	return(KEY_F13);
			case 26:	return(KEY_F14);
			case 28:	return(KEY_HELP);
			case 29:	return(KEY_DO);
			case 31:	return(KEY_F17);
			case 32:	return(KEY_F18);
			case 33:	return(KEY_F19);
			case 34:	return(KEY_F20);
			default:	goto nosuchkey;
			}
		else goto nosuchkey;
   default:	return(oc);
   }
 
nosuchkey:
		for (i=0; i<=rc; i++) ungetc(cn[i],stdin);
		return(oc);
}
 
#if     VMS
#include        <stsdef.h>
#include        <ssdef.h>
#include        <descrip.h>
#include        <iodef.h>
#include        <ttdef.h>
#include	<tt2def.h>
 
#define NIBUF   128                     /* Input buffer size            */
#define NOBUF   1024                    /* MM says bug buffers win!     */
#define EFN     0                       /* Event flag                   */
 
char    obuf[NOBUF];                    /* Output buffer                */
int     nobuf;                  /* # of bytes in above    */
char    ibuf[NIBUF];                    /* Input buffer          */
int     nibuf;                  /* # of bytes in above  */
int     ibufi;                  /* Read index                   */
int     oldmode[3];                     /* Old TTY mode bits            */
int     newmode[3];                     /* New TTY mode bits            */
short   iochan;                  /* TTY I/O channel             */
 
/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw.
 */
ttopen()
{
        struct  dsc$descriptor  idsc;
        struct  dsc$descriptor  odsc;
        char    oname[40];
        int     iosb[2];
        int     status;
 
        odsc.dsc$a_pointer = "TT";
        odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
        odsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        odsc.dsc$b_class        = DSC$K_CLASS_S;
        idsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        idsc.dsc$b_class        = DSC$K_CLASS_S;
        do {
                idsc.dsc$a_pointer = odsc.dsc$a_pointer;
                idsc.dsc$w_length  = odsc.dsc$w_length;
                odsc.dsc$a_pointer = &oname[0];
                odsc.dsc$w_length  = sizeof(oname);
                status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
                if (status!=SS$_NORMAL && status!=SS$_NOTRAN)
                        exit(status);
                if (oname[0] == 0x1B) {
                        odsc.dsc$a_pointer += 4;
                        odsc.dsc$w_length  -= 4;
                }
        } while (status == SS$_NORMAL);
        status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
        if (status != SS$_NORMAL)
                exit(status);
        status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        newmode[0] = oldmode[0];
        newmode[1] = oldmode[1] | TT$M_NOECHO;
        newmode[1] &= ~(TT$M_TTSYNC|TT$M_HOSTSYNC);
        newmode[2] = oldmode[2] | TT2$M_PASTHRU;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
 
}
 
/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 */
ttclose()
{
        int     status;
        int     iosb[1];
 
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                 oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        status = SYS$DASSGN(iochan);
        if (status != SS$_NORMAL)
                exit(status);
}
#endif  /* VMS */

#endif /* VT200KEY */
