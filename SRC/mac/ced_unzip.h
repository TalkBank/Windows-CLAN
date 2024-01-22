#ifndef UNZIP_ZIPS
#define UNZIP_ZIPS

#import <Cocoa/Cocoa.h>
// 2020-07-24 beg
@interface downLoadController : NSObject ;

- (IBAction)morGrammarEng:(id)sender;
- (IBAction)morGrammarYue:(id)sender;
- (IBAction)morGrammarZho:(id)sender;
- (IBAction)morGrammarDan:(id)sender;
- (IBAction)morGrammarNld:(id)sender;
- (IBAction)morGrammarFra:(id)sender;
- (IBAction)morGrammarDeu:(id)sender;
- (IBAction)morGrammarHeb:(id)sender;
- (IBAction)morGrammarIta:(id)sender;
- (IBAction)morGrammarJpn:(id)sender;
- (IBAction)morGrammarSpa:(id)sender;


@end
// 2020-07-24 end

extern char UnZipFolder(FNType *zipFName, FNType *destination);
extern void GetMORGrammar(const char *grammar, size_t fileSize);
extern void GetKidevalDB(const char *database, size_t fileSize);

#endif /* UNZIP_ZIPS */
