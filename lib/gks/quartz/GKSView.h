#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#import "gkscore.h"
#import "gksquartz.h"

@interface GKSView : NSView 
{
  @private
    char *buffer;
    int size;
    int win_id;
    float angle;
    IBOutlet NSBox *extendSavePanelView;
    IBOutlet NSPopUpButton *saveFormatPopUp;
    IBOutlet NSSlider *compressionSlider;
}
- (void) setDisplayList: (id) display_list;
- (void) close;
- (void) clear;

- (void) setWinID: (int) winid;
- (int) getWinID;

- (void) resize_window;
- (void) set_clip_rect: (int) tnr;

- (void) gks_set_shadow;

- (void) polyline: (int) n: (float *) px: (float *) py;
- (void) draw_marker: (float) xn: (float) yn: (int) mtype: (float) mscale: (int) mcolor: (CGContextRef) context;
- (void) polymarker: (int) n: (float *) px: (float *) py;
- (void) fillarea: (int) n: (float *) px: (float *) py;
- (void) cellarray:
  (float) xmin: (float) xmax: (float) ymin: (float) ymax:
  (int) dx: (int) dy: (int) dimx: (int *) colia: (int) true_color;
  
- (void) text: (float) px: (float) py: (char *) text;
- (NSFont *) set_font: (int) font;

- (IBAction) keep_on_display: (id) sender;
- (IBAction) rotate: (id) sender;
@end
