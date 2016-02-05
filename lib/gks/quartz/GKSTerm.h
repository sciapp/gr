
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import "gkscore.h"
#import "gksquartz.h"

#import "GKSView.h"

@interface GKSTerm : NSObject <gks_protocol> 
{
  @private 
    NSConnection *connection;
    int num_windows;
    int curr_win_id;
    bool close_window[MAX_WINDOWS];
    NSWindow *window[MAX_WINDOWS];
    GKSView *view[MAX_WINDOWS];
    NSPoint cascadingPoint;
}
- (IBAction) cascadeWindows: (id) sender;
- (void) setWindowPos: (NSWindow *) plotWindow;
- (void) keepOnDisplay: (NSNotification *) aNotification;
- (int) getNextWindowID;
- (void) windowWillClose:(NSNotification *)notification;
@end
