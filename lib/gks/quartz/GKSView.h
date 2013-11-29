#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#import "gkscore.h"
#import "gksquartz.h"

typedef struct {
  int fontsize;
  NSString *fontfamily;
} _FontInfo;

@interface GKSView : NSView 
{
  @private
    char *buffer;
    int size;
    int win_id;
    double angle;
    IBOutlet NSBox *extendSavePanelView;
    IBOutlet NSPopUpButton *saveFormatPopUp;
    IBOutlet NSSlider *compressionSlider;
}
- (void) setDisplayList: (id) display_list;
- (void) close;

- (void) setWinID: (int) winid;
- (int) getWinID;

- (void) resize_window;
- (void) set_clip_rect: (int) tnr;

- (void) gks_set_shadow;

- (void) polyline: (int) n : (double *) px : (double *) py;
- (void) draw_marker: (double) xn : (double) yn : (int) mtype : (double) mscale : (int) mcolor : (CGContextRef) context;
- (void) polymarker: (int) n : (double *) px : (double *) py;
- (void) fillarea: (int) n : (double *) px : (double *) py;
- (void) cellarray:
  (double) xmin : (double) xmax : (double) ymin : (double) ymax :
  (int) dx : (int) dy : (int) dimx : (int *) colia : (int) true_color;
  
- (void) text: (double) px : (double) py : (char *) text;
- (_FontInfo) set_font: (int) font;
- (NSString *) stringForText: (const char*)text withFontFamilyID : (int)family;

- (IBAction) keep_on_display: (id) sender;
- (IBAction) rotate: (id) sender;
@end
