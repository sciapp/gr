
#include "gks.h"
#include "gkscore.h"

#import "GKSTerm.h"
#import "GKSView.h"

@implementation GKSTerm

- (void) awakeFromNib
{ 
  int win;

  [[NSNotificationCenter defaultCenter] addObserver:self 
                                        selector:@selector(keepOnDisplay:) 
                                        name:@"GKSViewKeepOnDisplayNotification" object:nil];   
  if (!connection)
    {
      // Deprecated in Mac OS X v10.6
      //connection = [NSConnection defaultConnection];
      connection = [NSConnection new];
      [connection setRootObject:self];
      [connection registerName:@"GKSQuartz"];
      num_windows = 0;
      curr_win_id = 0;
      for (win = 0; win < MAX_WINDOWS; win++)
        window[win] = nil;
    }
}

- (int) GKSQuartzCreateWindow
{
  int win = [self getNextWindowID];
 
  if (win < MAX_WINDOWS)
    {
      curr_win_id = win;
      num_windows++;
      NSRect screenFrame = [[[NSScreen screens] objectAtIndex:0] frame];
      window[win] = [[NSWindow alloc]
                      initWithContentRect: NSMakeRect(NSMinX(screenFrame), NSMaxY(screenFrame) - 500, 500, 500)
                      styleMask: NSTitledWindowMask | NSClosableWindowMask |
                      NSMiniaturizableWindowMask | NSResizableWindowMask
                      backing: NSBackingStoreBuffered defer: NO];
      [window[win] setBackgroundColor: [NSColor colorWithCalibratedWhite: 1 alpha: 1]];
      view[win] = [[GKSView alloc] initWithFrame: NSMakeRect(0, 0, 500, 500)];
      [window[win] setContentView:view[win]];
      [window[win] makeFirstResponder: view[win]];
      [window[win] makeKeyAndOrderFront: nil];
      [window[win] setTitle: @"GKSTerm"];
      [window[win] display];
      
      [view[win] setWinID: win];
      
      cascadingPoint = [window[win] cascadeTopLeftFromPoint: cascadingPoint];
      
      close_window[win] = YES;
      [[NSNotificationCenter defaultCenter] addObserver:self
              selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification
              object:window[win]];
      return win;
    }
  else
    return -1;
}

- (void) windowWillClose:(NSNotification *)notification
{
  int win;

  NSWindow *nswin = [notification object];
  for (win = 0; win < MAX_WINDOWS; win++) {
    if (window[win] != nil && close_window[win] && window[win] == nswin) {
      window[win] = nil;
    }
  }
}

- (int) GKSQuartzIsAlive: (int) win
{
  return window[win] != nil ? 1 : 0;
}

- (void) GKSQuartzDraw: (int) win displayList: (id) displayList
{
  [view[win] setDisplayList: displayList];
}

- (void) GKSQuartzCloseWindow: (int) win
{
  if (close_window[win])
    {
      if (view[win] != nil) {
        [view[win] close];
      }
      if (window[win] != nil) {
        [window[win] close];
      }
    }
  view[win] = nil;
  window[win] = nil;
  
  curr_win_id = win;
}

- (IBAction) cascadeWindows: (id) sender
{
  int i;
  NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
  cascadingPoint = NSMakePoint( NSMinX(screenFrame), NSMaxY(screenFrame) );
      
  for (i = 0; i < num_windows; i++) 
    {
      if (window[i])
        {        
          [self setWindowPos:window[i]];
          [window[i] makeKeyAndOrderFront:self];
        }
    }
}

- (void) setWindowPos: (NSWindow *) plotWindow
{
  cascadingPoint = [plotWindow cascadeTopLeftFromPoint:cascadingPoint];
}

- (void) keepOnDisplay: (NSNotification *) aNotification
{
  GKSView *tmpView = [aNotification object];  
  int win = [tmpView getWinID];
  close_window[win] = NO;
}

- (int) getNextWindowID
{
  /* Search unused window */
  int unused_win_id;
  for (unused_win_id = 0; unused_win_id < MAX_WINDOWS && window[unused_win_id]; unused_win_id++);
  
  /* Either return the index of an unused window or MAX_WINDOWS */
  return unused_win_id;
}

@end
