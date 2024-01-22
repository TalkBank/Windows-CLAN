

#import <Cocoa/Cocoa.h>

#define CODESIZE	20
#define IDFIELSSIZE 256

#define IDSTYPE struct IDSs
struct IDSs {
	unCH language[IDFIELSSIZE+1];	// @Languages:, @Language of #:
	unCH corpus[IDFIELSSIZE+1];		// such as: MacWhinney, Bates, Sachs, etc...
	unCH code[CODESIZE+1];			// @Participants: *CHI
	short age1y;					// @Age of #:
	short age1m;					// @Age of #:
	short age1d;					// @Age of #:
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

#define SESTYPE struct SES_type
struct SES_type {
	unCH *st;
	SESTYPE *nextses;
} ;

#define DEPFDEFS struct DepfileDefs
struct DepfileDefs {
	ROLESTYPE *depfileRoles;
	SESTYPE *depfileSESe;
	SESTYPE *depfileSESs;
} ;

@interface IdsController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate> {
	IBOutlet NSPopUpButton *usersPopUp;

	IBOutlet NSTextField *langField;
	IBOutlet NSTextField *corpusField;
	IBOutlet NSTextField *codeField;
	IBOutlet NSTextField *ageYearField;
	IBOutlet NSTextField *ageMonthField;
	IBOutlet NSTextField *ageDateField;

	IBOutlet NSButton *unknownButton;
	IBOutlet NSButton *maleButton;
	IBOutlet NSButton *femaleButton;

	IBOutlet NSTextField *groupField;

	IBOutlet NSPopUpButton *racePopUp;
	IBOutlet NSPopUpButton *sesPopUp;
	IBOutlet NSPopUpButton *rolePopUp;

	IBOutlet NSTextField *educationField;
	IBOutlet NSTextField *customField;
	IBOutlet NSTextField *optionalField;

	DocumentWindowController *docWinController;

	NSInteger spItem;
	IDSTYPE *rootIDs;
	ROLESTYPE *depfileRoles;
	SESTYPE *depfileSESe;
	SESTYPE *depfileSESs;
}

- (IBAction)usersPopUpClicked:(id)sender;

- (IBAction)deleteButtonClicked:(id)sender;
- (IBAction)createButtonClicked:(id)sender;
- (IBAction)copyButtonClicked:(id)sender;

- (IBAction)langFieldChanged:(id)sender;
- (IBAction)corpusFieldChanged:(id)sender;
- (IBAction)codeFieldChanged:(id)sender;
- (IBAction)ageYearFieldChanged:(id)sender;
- (IBAction)ageMonthFieldChanged:(id)sender;
- (IBAction)ageDateFieldChanged:(id)sender;

- (IBAction)unknownClicked:(id)sender;
- (IBAction)maleClicked:(id)sender;
- (IBAction)femaleClicked:(id)sender;

- (IBAction)groupFieldChanged:(id)sender;

- (IBAction)racePopUpClicked:(id)sender;
- (IBAction)sesPopUpClicked:(id)sender;
- (IBAction)rolePopUpClicked:(id)sender;

- (IBAction)educationFieldChanged:(id)sender;
- (IBAction)customFieldChanged:(id)sender;
- (IBAction)optionalFieldChanged:(id)sender;

- (IBAction)cancelButtonClicked:(id)sender;
- (IBAction)doneButtonClicked:(id)sender;

+(char *)idsDialog:(DocumentWindowController *)docWinController;

@end

