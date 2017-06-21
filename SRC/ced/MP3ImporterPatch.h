/*
	File:		MP3ImporterPatch.h
	
	Author:		QuickTime Engineering

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
					
	$Log: MP3ImporterPatch.h,v $
	Revision 1.2  2004/03/16 16:39:15  jkc
	Capture the MP3 importer and tell it to prefer VBR sample layout (2828781).
	
	Revision 1.1  2004/03/12 23:39:17  jkc
	First time
	
*/

#ifdef _MAC_CODE

#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>

#endif

#ifdef _WIN32 

#include "QTML.h"
#include "Movies.h"

#endif

extern "C"
{

extern void MP3ImporterPatch_DoRegister();

extern Component gQuickTimeMP3ImporterComponent;

}
