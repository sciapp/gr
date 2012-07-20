#define MAX_COLOR 980

typedef struct
  {
    int state;
    int win;
    gks_display_list_t dl;
    float width, height;
    float swidth, sheight;
    float a, b, c, d;
    float window[4], viewport[4];
    CGColorRef rgb[MAX_COLOR];
    int family, capheight;
    float angle;
    CGRect rect[MAX_TNR];
 }
ws_state_list;

@protocol gks_protocol
- (int) GKSQuartzCreateWindow;
- (void) GKSQuartzDraw: (int) win displayList: (id) displayList;
- (void) GKSQuartzCloseWindow: (int) win;
@end
