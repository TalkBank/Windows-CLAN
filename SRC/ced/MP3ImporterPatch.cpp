/*
	File:		MP3ImporterPatch.c
	
	Description: Component that captures QuickTime's MP3 importer and makes it do smart things

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
					
	$Log: MP3ImporterPatch.c,v $
	Revision 1.2  2004/03/16 16:39:15  jkc
	Capture the MP3 importer and tell it to prefer VBR sample layout (2828781).
	
	Revision 1.1  2004/03/12 23:39:17  jkc
	First time
	
*/
// #include "MPEnv.h"


#ifdef _MAC_CODE

#include <QuickTime/ImageCompression.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include "Components.h"
#include "MP3ImporterPatch.h"

#endif

#ifdef _WIN32 

#include <ImageCompression.h>
#include "QTML.h"
#include "Components.h"
#include "Movies.h"
#include "Gestalt.h"

#include "MP3ImporterPatch.h"

#if defined(_MSC_VER)	
	#pragma warning (disable: 4068)		// ignore unknown pragmas
#endif

#endif

// Per instance globals
typedef struct
{
	ComponentInstance		self;
	ComponentInstance		delegate;
} MP3ImporterPatchGlobalsRecord, *MP3ImporterPatchGlobals;

/************************************************************************************/
// Setup required for ComponentDispatchHelper.c

// StdComponent Prototypes
#define CALLCOMPONENT_BASENAME()		MP3ImporterPatch_
#define	CALLCOMPONENT_GLOBALS()			MP3ImporterPatchGlobals storage

#ifdef _MAC_CODE
#include <CoreServices/Components.k.h>
#endif

#ifdef _WIN32 
#include "Components.k.h"
#endif

// Component Prototypes
#define	MOVIEIMPORT_BASENAME()			CALLCOMPONENT_BASENAME()
#define	MOVIEIMPORT_GLOBALS()			CALLCOMPONENT_GLOBALS()

#ifdef _MAC_CODE
#include <QuickTime/QuickTimeComponents.k.h>
#endif

#ifdef _WIN32 
#include "QuickTimeComponents.k.h"
#endif

// Defines for dispatch helper
#define COMPONENT_DISPATCH_FILE			"MP3ImporterPatchDispatch.h"	// describes what to dispatch
#define	GET_DELEGATE_COMPONENT()		(storage->delegate)
#define COMPONENT_UPP_SELECT_ROOT()		MovieImport		// root for Type's UPP_PREFIX and SELECT_PREFIX		

#ifdef _MAC_CODE
#include <QuickTime/ComponentDispatchHelper.c>	// Make the dispatcher and canDo
#endif

#ifdef _WIN32 
#include "ComponentDispatchHelper.c"
#endif

#pragma mark -


Component gQuickTimeMP3ImporterComponent;

// Movie importer property class, which is currently private to QuickTime
enum {
	kPrivateDefFor_PropertyClass_MovieImporter = 'eat '
};

// Movie importer property class property IDs, which are currently private to QuickTime
enum {
	kPrivateDefFor_MovieImporterPropertyID_PreferFixedCompression = 'pfc ',
	kPrivateDefFor_MovieImporterPropertyID_PreferVariableCompression = 'pvc '
};


void MP3ImporterPatch_DoRegister()
{
    Component qtMP3Importer;
	ComponentRoutineUPP mp3ComponentUPP;
	ComponentDescription cd;
    Component MP3ImporterPatchComponent;
    
    // don't do this more than once
    if (NULL != gQuickTimeMP3ImporterComponent) return;
    
    // find the QT MP3 importer
    cd.componentType = MovieImportType;
    cd.componentSubType = FOUR_CHAR_CODE('Mp3 ');
	cd.componentManufacturer = 'soun';
	cd.componentFlags =  0;		// any flags
	cd.componentFlagsMask = 0;
    
    qtMP3Importer = FindNextComponent (NULL, &cd);
    if (NULL == qtMP3Importer) return;

    // now get all the correct flag settings of the QT MP3 importer, so that
    // we can register with all the same flags
    if (noErr != GetComponentInfo(qtMP3Importer, &cd, NULL, NULL, NULL)) return;
	
    // now create a UPP for our component's entrypoint and register it as the same type of component
	mp3ComponentUPP = NewComponentRoutineUPP((ComponentRoutineProcPtr)MP3ImporterPatch_ComponentDispatch);
    if (NULL == mp3ComponentUPP) return;

	MP3ImporterPatchComponent = RegisterComponent(&cd, mp3ComponentUPP, 0, NULL, NULL, NULL);
    if (NULL == MP3ImporterPatchComponent) return;
    
    // Capture the real MP3 importer.
    // A special private reference to the now-captured component is returned.
    gQuickTimeMP3ImporterComponent = CaptureComponent(qtMP3Importer, MP3ImporterPatchComponent);
    
    // now whenever anyone wants to instantiate QuickTime's MP3 importer (within this process, anyway),
    // they'll get our component instead. Bwahahahaha, etc.
    
}


#pragma mark-


/************************************************************************************/
// Component functions


// Component Open Request - Required
pascal ComponentResult MP3ImporterPatch_Open(MP3ImporterPatchGlobals glob, ComponentInstance self)
{
	ComponentResult		 err;
    Boolean				 trueVar;
    
    // whenever we're opened, open an instance of the real MP3 importer,
    // configure it to behave as we wish,
    // and set ourselves up to delegate all calls upon us to it

    // if our registration set-up didn't complete, fail utterly
    if (NULL == gQuickTimeMP3ImporterComponent) return paramErr;

	// Allocate memory for our globals, set them up and inform the component
	// manager that we've done so
	glob = (MP3ImporterPatchGlobals)NewPtrClear(sizeof(MP3ImporterPatchGlobalsRecord));
	if ((err = MemError())) goto bail;
	
	SetComponentInstanceStorage(self, (Handle)glob);
	glob->self	 = self;
	
	// Open and target an instance of QT's MP3 importer
	err	= OpenAComponent(gQuickTimeMP3ImporterComponent, &glob->delegate);
	if (err) goto bail;

	// Set ourselves as the MP3 importer instance's target
	CallComponentTarget(glob->delegate, self);
    
    // now configure our delegate importer to behave in a non-default manner.
    // This is the sole reason for all these shenanigans.
    
    // If a version of QuickTime prior to 6.4 is running, the following call will have no effect.
    // Build note: you'll need to build on a machine with the 6.4 version of the QT framework
    // installed for this to compile and link. Panther includes QuickTime 6.4.
    trueVar = true;
    (void)QTSetComponentProperty(glob->delegate, kPrivateDefFor_PropertyClass_MovieImporter,
        kPrivateDefFor_MovieImporterPropertyID_PreferVariableCompression,
        sizeof(Boolean), &trueVar);

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult MP3ImporterPatch_Close(MP3ImporterPatchGlobals glob, ComponentInstance self)
{
    // close down our delegate and dispose of our storage
	if (glob)
	{
		if (glob->delegate)
			CloseComponent(glob->delegate);
		DisposePtr((Ptr)glob);
	}
	return noErr;
}
