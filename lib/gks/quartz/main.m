#import <Cocoa/Cocoa.h>

#import "GKSTerm.h"

int main (int argc, const char *argv[])
{   
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  int retVal = NSApplicationMain(argc, (const char **) argv);
  [pool release];
  return retVal;
}
