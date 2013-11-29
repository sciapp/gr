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
 }
ws_state_list;

@protocol gks_protocol
- (int) GKSQuartzCreateWindow;
- (void) GKSQuartzDraw: (int) win displayList: (id) displayList;
- (void) GKSQuartzCloseWindow: (int) win;
@end
