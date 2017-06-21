/*
	File:		MP3ImporterPatchDispatch.h
	
	Description: Component that captures QuickTime's MP3 importer and makes it do smart things

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
					
	$Log: MP3ImporterPatchDispatch.h,v $
	Revision 1.1  2004/03/12 23:39:17  jkc
	First time
	
*/

	ComponentComment ("Dispatch file for MP3ImporterPatch")

	ComponentSelectorOffset (2)

	ComponentRangeCount (1)
	ComponentRangeShift (7)
	ComponentRangeMask	(7F)

	ComponentStorageType (Ptr)
	ComponentDelegateByteOffset (4)

	ComponentRangeBegin (0)
		StdComponentCall (Close)
		StdComponentCall (Open)
	ComponentRangeEnd (0)
