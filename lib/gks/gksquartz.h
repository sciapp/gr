typedef struct
  {
    int state;
    int win;
    gks_display_list_t dl;
    double width, height;
    double swidth, sheight;
    double a, b, c, d;
    double window[4], viewport[4];
    CGColorRef rgb[MAX_COLOR];
    int family, capheight;
    double angle;
    CGRect rect[MAX_TNR];
    pthread_t master_thread;
 }
ws_state_list;

@protocol gks_protocol
- (int) GKSQuartzCreateWindow;
- (void) GKSQuartzDraw: (int) win displayList: (id) displayList;
- (int) GKSQuartzIsAlive: (int) win;
- (void) GKSQuartzCloseWindow: (int) win;
@end
