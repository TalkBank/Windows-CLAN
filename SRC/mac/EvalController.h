
#import <Cocoa/Cocoa.h>

@interface EvalController : NSWindowController <NSLayoutManagerDelegate, NSTableViewDelegate>
{
	IBOutlet NSButton *sp_oneCH;
	IBOutlet NSButton *sp_twoCH;
	IBOutlet NSButton *sp_threeCH;
	IBOutlet NSButton *sp_fourCH;
	IBOutlet NSButton *sp_fiveCH;
	IBOutlet NSButton *sp_sixCH;
	IBOutlet NSButton *sp_sevenCH;
	IBOutlet NSButton *sp_eightCH;

	IBOutlet NSButton *AnomicCH;
	IBOutlet NSButton *GlobalCH;
	IBOutlet NSButton *ControlCH;
	IBOutlet NSButton *BrocaCH;
	IBOutlet NSButton *WernickeCH;
	IBOutlet NSButton *ConductionCH;
	IBOutlet NSButton *TransSensoryCH;
	IBOutlet NSButton *TransMotorCH;
	IBOutlet NSButton *NotAphasicByWABCH;

	IBOutlet NSTextField *AgeRangeField;

	IBOutlet NSButton *maleCH;
	IBOutlet NSButton *femaleCH;

	IBOutlet NSButton *SpeechCH;
	IBOutlet NSButton *CatCH;
	IBOutlet NSButton *FloodCH;
	IBOutlet NSButton *CinderellaCH;
	IBOutlet NSButton *UmbrellaCH;
	IBOutlet NSButton *SandwichCH;
	IBOutlet NSButton *Important_EventCH;
	IBOutlet NSButton *StrokeCH;
	IBOutlet NSButton *WindowCH;
}

- (IBAction)sp_oneClicked:(id)sender;
- (IBAction)sp_twoClicked:(id)sender;
- (IBAction)sp_threeClicked:(id)sender;
- (IBAction)sp_fourClicked:(id)sender;
- (IBAction)sp_fiveClicked:(id)sender;
- (IBAction)sp_sixClicked:(id)sender;
- (IBAction)sp_sevenClicked:(id)sender;
- (IBAction)sp_eightClicked:(id)sender;

- (IBAction)DeselectDatabaseClicked:(id)sender;

- (IBAction)AnomicClicked:(id)sender;
- (IBAction)GlobalClicked:(id)sender;
- (IBAction)ControlClicked:(id)sender;
- (IBAction)BrocaClicked:(id)sender;
- (IBAction)WernickeClicked:(id)sender;
- (IBAction)ConductionClicked:(id)sender;
- (IBAction)TransSensoryClicked:(id)sender;
- (IBAction)TransMotorClicked:(id)sender;
- (IBAction)NotAphasicByWABClicked:(id)sender;

- (IBAction)FluentClicked:(id)sender;
- (IBAction)NonFluentClicked:(id)sender;
- (IBAction)AllAphasiaClicked:(id)sender;

- (IBAction)maleClicked:(id)sender;
- (IBAction)femaleClicked:(id)sender;

- (IBAction)DeselectAllGemsClicked:(id)sender;
- (IBAction)SelectAllGemsClicked:(id)sender;

- (IBAction)SpeechClicked:(id)sender;
- (IBAction)CatClicked:(id)sender;
- (IBAction)FloodClicked:(id)sender;
- (IBAction)CinderellaClicked:(id)sender;
- (IBAction)UmbrellaClicked:(id)sender;
- (IBAction)SandwichClicked:(id)sender;
- (IBAction)Important_EventClicked:(id)sender;
- (IBAction)StrokeClicked:(id)sender;
- (IBAction)WindowClicked:(id)sender;

- (IBAction)OKClicked:(id)sender;
- (IBAction)CancelClicked:(id)sender;

+ (const char *)EvalDialog;

@end
