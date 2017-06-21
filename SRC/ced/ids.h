/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef IDsh
#define IDsh

extern "C"
{

#define CODESIZE	20
#define IDFIELSSIZE 100

#define IDSTYPE struct IDSs
struct IDSs {
	unCH language[IDFIELSSIZE+1];	// @Languages:, @Language of #:
	unCH corpus[IDFIELSSIZE+1];		// such as: MacWhinney, Bates, Sachs, etc...
	unCH code[CODESIZE+1];			// @Participants: *CHI
	char ageAbout;					// @Sex of #:
	short age1y;					// @Age of #:
	short age1m;					// @Age of #:
	short age1d;					// @Age of #:
	short age2y;					// @Age of #:
	short age2m;					// @Age of #:
	short age2d;					// @Age of #:
	char sex;						// @Sex of #:
	unCH group[IDFIELSSIZE+1];		// @Group of #:
	unCH SES[IDFIELSSIZE+1];		// @Ses of #:
	unCH role[IDFIELSSIZE+1];		// @Participants: Target_Child
	unCH education[IDFIELSSIZE+1];	// @Education of #:
	unCH custom_field[IDFIELSSIZE+1];// file name or other unique ID
	unCH spname[IDFIELSSIZE+1];		// @Participants: Jane
	struct IDSs *next_id;
} ;

#define ROLESTYPE struct Roles
struct Roles {
	unCH *role;
	ROLESTYPE *nextrole;
} ;

extern int setIDs(int i);
extern char IDDialog(IDSTYPE **rootIDs, ROLESTYPE *rootRoles);
extern IDSTYPE *createNewId(IDSTYPE *rootIDs, int item);
extern IDSTYPE *deleteID(IDSTYPE *rootIDs, int item);

}

#endif /* IDsh */
