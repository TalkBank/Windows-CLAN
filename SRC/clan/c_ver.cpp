/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// The format of Version has to be exactly like this:
//		"V DD-MMM-YYYY HH:MM"

int  CLANVersion = 3; // CHANGE NUMBER IN FILE "ChiMain/web/childes/clan/CLAN_VERSION.txt"
char VERSION[] = "V 20-Feb-2025 11:00"; //"ChiMain/web/childes/clan/index.html"
// ON MAC CHANGE "Bundle versions string, short" IN FILE "Info.plist"
// ON PC CHANGE RESOURCE VERSION AND IN INSTALLER DISTRIBUTION VERSION

// Also change file "ChiMain/web/childes/clan/index.html"

#if !defined(UNX) && !defined(_MAC_CODE) && !defined(_WIN32)
#error "Please read top of the 0README.TXT file for instructions."
#endif
