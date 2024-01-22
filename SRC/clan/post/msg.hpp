/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- msg.hpp	- v1.0.0 - november 1998

this file defines the procedure that permits to make message display platform independent

|=========================================================================================================================*/

#ifndef __MSG_HPP__
#define __MSG_HPP__

void msg( const char *string, ... );     // this function may be called from any program to redirect output
void msgfile( FILE* outfile, const char *string, ... );     // this function may be called from any program to redirect output

#define MaxSizeOfMsg (1024*16)
#endif

