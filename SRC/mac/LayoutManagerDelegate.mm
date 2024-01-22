//  AppDelegate.mm
#import <Cocoa/Cocoa.h>

@interface LayoutManager : NSLayoutManager <NSLayoutManagerDelegate>
@end

@interface LayoutManager ()
@end

@implementation LayoutManager

- (NSUInteger)layoutManager:(NSLayoutManager *)layoutManager
	   shouldGenerateGlyphs:(const CGGlyph *)glyphs
				 properties:(const NSGlyphProperty *)props
		   characterIndexes:(const NSUInteger *)charIndexes
					   font:(NSFont *)aFont
			  forGlyphRange:(NSRange)glyphRange {
	
	return glyphRange.length;
}
@end
