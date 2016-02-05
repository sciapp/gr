
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Foundation/NSFileManager.h>
#import <AppKit/AppKit.h>

#include <pthread.h>

#include "gks.h"
#include "gkscore.h"
#include "gksquartz.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifdef _WIN32

#include <windows.h>
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif
  
#else
  
#ifdef __cplusplus
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT
#endif
  
#endif
  
DLLEXPORT void gks_quartzplugin(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr);
  
#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

static
gks_state_list_t *gkss;

id plugin;
NSLock *mutex;

int num_windows = 0;


@interface wss_wrapper:NSObject
{
  ws_state_list *wss;
}

@property ws_state_list *wss;
@end

@implementation wss_wrapper
@synthesize wss;
@end

@interface gks_quartz_thread : NSObject
+ (void) update: (id) param;
@end

@implementation gks_quartz_thread
+ (void) update: (id) param
{
  int didDie = 0;
  wss_wrapper *wrapper = (wss_wrapper *)param;
  ws_state_list *wss = [wrapper wss];
  [wrapper release];

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  while (!didDie && wss != NULL)
  {
      [mutex lock];
      if (wss->inactivity_counter == 3)
	{
          [wss->displayList initWithBytesNoCopy: wss->dl.buffer
                       length: wss->dl.nbytes freeWhenDone: NO];
          @try
            {
              [plugin GKSQuartzDraw: wss->win displayList: wss->displayList];
	      wss->inactivity_counter = -1;
            }
          @catch (NSException *e)
            {
              didDie = 1;
            }
        }
      if (wss->inactivity_counter >= 0)
        wss->inactivity_counter++;
      @try
        {
          if ([plugin GKSQuartzIsAlive: wss->win] == 0)
            {
              /* This process should die when the user closes the last window */
              if (!wss->closed_by_api) {
                bool all_dead = YES;
                int win;
                for (win = 0; all_dead && win < MAX_WINDOWS; win++) {
                  all_dead = [plugin GKSQuartzIsAlive: win] == 0;
                }
                if (all_dead) {
                  pthread_kill(wss->master_thread, SIGTERM);
                }
              }
              didDie = 1;
            }
        }
      @catch (NSException *e)
        {
          pthread_kill(wss->master_thread, SIGTERM);
          didDie = 1;
        }

    if (didDie) {
      wss->thread_alive = NO;
    }
    [mutex unlock];

    usleep(100000);
    }
  [pool drain];
}
@end

static
BOOL gks_terminal(void)
{
  NSURL *url;
  OSStatus status;
  BOOL isDir;

  NSFileManager *fm = [[NSFileManager alloc] init];
  NSString *grdir = [[[NSProcessInfo processInfo]
                      environment]objectForKey:@"GRDIR"];
  if ( grdir == NULL )
    grdir = [NSString stringWithUTF8String:GRDIR];

  NSString *path = [NSString stringWithFormat:@"%@/Applications/GKSTerm.app",
                    grdir];
  if ( ! ([fm fileExistsAtPath:path isDirectory:&isDir] && isDir) )
    path = [[NSString stringWithFormat:@"%@/GKSTerm.app",
             grdir] stringByStandardizingPath];
  if ( [fm fileExistsAtPath:path isDirectory:&isDir] && isDir )
  {
     url = [NSURL fileURLWithPath: path];
     status = LSOpenCFURLRef((CFURLRef) url, NULL);
     return (status == noErr);
  }
  NSLog(@"Could not locate GKSTerm.app.");
  return false;
}

void gks_quartzplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  
  ws_state_list *wss = (ws_state_list *) *ptr;
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  switch (fctid)
    {
    case OPEN_WS:
      gkss = (gks_state_list_t *) *ptr;      

      wss = (ws_state_list *) calloc(1, sizeof(ws_state_list));
      wss->displayList = [[NSData alloc] initWithBytesNoCopy: wss
                                    length: sizeof(ws_state_list)
                                    freeWhenDone: NO];
      if (plugin == nil) {
        plugin = [NSConnection rootProxyForConnectionWithRegisteredName:
                  @"GKSQuartz" host: nil];
      }
      if (mutex == nil) {
        mutex = [[NSLock alloc] init];
      }
      wss->master_thread = pthread_self();

      if (plugin == nil)
        {
          if (!gks_terminal())
            {
               NSLog(@"Launching GKSTerm failed.");          
               exit(-1);
            }
          else
            {
              int counter = 10;
              while (--counter && !plugin)
                {
                  [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1.0]];
                  plugin = [NSConnection rootProxyForConnectionWithRegisteredName:
                            @"GKSQuartz" host: nil];
                }            
            }
        }

        wss->win = [plugin GKSQuartzCreateWindow];
        num_windows++;

      if (plugin)
        {
          wss_wrapper *wrapper = [wss_wrapper alloc];
          [wrapper init];
          wrapper.wss = wss;
          wss->thread_alive = YES;
          wss->closed_by_api = NO;
          [NSThread detachNewThreadSelector: @selector(update:) toTarget:[gks_quartz_thread class] withObject:wrapper];
          [plugin setProtocolForProxy: @protocol(gks_protocol)];
        }

      *ptr = wss;

      CGSize size = CGDisplayScreenSize(CGMainDisplayID());
      r1[0] = 0.001 * size.width;
      r2[0] = 0.001 * size.height;
      ia[0] = (int) NSMaxX([[[NSScreen screens] objectAtIndex:0] frame]);
      ia[1] = (int) NSMaxY([[[NSScreen screens] objectAtIndex:0] frame]);
      break;
  
    case CLOSE_WS:
        
        [mutex lock];
        wss->closed_by_api = YES;
        [mutex unlock];
        @try
      {
        [plugin GKSQuartzCloseWindow: wss->win];
        num_windows--;
      }
        @catch (NSException *e)
      {
        ;
      }
        
        [mutex lock];
        while (wss->thread_alive) {
          [mutex unlock];
          usleep(100000);
          [mutex lock];
        }
        [mutex unlock];
        
      if (num_windows == 0) {
        [mutex release];
        mutex = nil;
        [plugin release];
        plugin = nil;
      }
      [wss->displayList release];
      free(wss);
      wss = NULL;
      break;

      case 6:
      break;

      case UPDATE_WS:
      if (ia[1] == GKS_K_PERFORM_FLAG)
        {
          [mutex lock];
          [wss->displayList initWithBytesNoCopy: wss->dl.buffer
                       length: wss->dl.nbytes freeWhenDone: NO];
          @try
            {
              [plugin GKSQuartzDraw: wss->win displayList: wss->displayList];
              wss->inactivity_counter = -1;
            }
          @catch (NSException *e)
            {
              ;
            }
          [mutex unlock];
        }
      break;

    case POLYLINE:
    case POLYMARKER:
    case TEXT:
    case FILLAREA:
    case CELLARRAY:
    case DRAW_IMAGE:
      [mutex lock];
      wss->inactivity_counter = 0;
      [mutex unlock];
      break;
    }

  if (wss != NULL)
    gks_dl_write_item(&wss->dl,
      fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, gkss);
  
  [pool drain];
}
