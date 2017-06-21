#ifndef _CLAN_GLOBALS_
#define _CLAN_GLOBALS_

//#define MSL_USE_PRECOMPILED_HEADERS 1
//#define USE_PRECOMPILED_MAC_HEADERS 1

#define TARGET_API_MAC_CARBON 1
#define PROGCREATOR 'MCed'

/*
#include <ansi_prefix.mac.h>
*/
#if (__MACH__)
//#include <MacHeadersMacOSX++>
//#include <MSLMacHeadersMacOSX++>
//#include <MSLMacHeadersMach-O++>
//#define _MSL_USING_MW_C_HEADERS 1

//			#include <MSL MacHeadersMach-O.h>




//#include <MSLCarbonPrefix.h>
//#include <ansi_parms.h>
/*
#ifndef _MSL_USING_MW_C_HEADERS
#error "NO _MSL_USING_MW_C_HEADERS"
#else

#if !_MSL_USING_MW_C_HEADERS
#error "_MSL_USING_MW_C_HEADERS == 0"
#else
#error "_MSL_USING_MW_C_HEADERS == 1"
#endif

#endif
*/
	#include <Carbon.h>
	#include <QuickTime.h>

//			#define wchar_t unsigned short

/*
	#ifndef __dead2
	#define	__dead2
	#define	__pure2
	#define	__unused
	#endif
*/
#else
//			#include <MacHeadersCarbon++>
	#include <Carbon.h>
	#include <QuickTime.h>
#endif

#define POSTCODE

#endif /* _CLAN_GLOBALS_ */