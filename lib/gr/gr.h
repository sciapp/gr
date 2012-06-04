#ifdef _WIN32

#define HAVE_BOOLEAN

#include <windows.h>	/* required for all Windows applications */
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

#else

#ifdef __cplusplus
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT
#endif

#endif

DLLEXPORT void gr_opengks(void);
DLLEXPORT void gr_closegks(void);
DLLEXPORT void gr_inqdspsize(float *, float *, int *, int *);
DLLEXPORT void gr_openws(int, char *, int);
DLLEXPORT void gr_closews(int);
DLLEXPORT void gr_activatews(int);
DLLEXPORT void gr_deactivatews(int);
DLLEXPORT void gr_clearws(void);
DLLEXPORT void gr_updatews(void);
DLLEXPORT void gr_polyline(int, float *, float *);
DLLEXPORT void gr_polymarker(int, float *, float *);
DLLEXPORT void gr_text(float, float, char *);
DLLEXPORT void gr_fillarea(int, float *, float *);
DLLEXPORT void gr_cellarray(
  float, float, float, float, int, int, int, int, int, int, int *);
DLLEXPORT void gr_spline(int, float *, float *, int, int);
DLLEXPORT void gr_setasf(int *);
DLLEXPORT void gr_setlineind(int);
DLLEXPORT void gr_setlinetype(int);
DLLEXPORT void gr_setlinewidth(float);
DLLEXPORT void gr_setlinecolorind(int);
DLLEXPORT void gr_setmarkerind(int);
DLLEXPORT void gr_setmarkertype(int);
DLLEXPORT void gr_setmarkersize(float);
DLLEXPORT void gr_setmarkercolorind(int);
DLLEXPORT void gr_settextind(int);
DLLEXPORT void gr_settextfontprec(int, int);
DLLEXPORT void gr_setcharexpan(float);
DLLEXPORT void gr_setcharspace(float);
DLLEXPORT void gr_settextcolorind(int);
DLLEXPORT void gr_setcharheight(float);
DLLEXPORT void gr_setcharup(float, float);
DLLEXPORT void gr_settextpath(int);
DLLEXPORT void gr_settextalign(int, int);
DLLEXPORT void gr_setfillind(int);
DLLEXPORT void gr_setfillintstyle(int);
DLLEXPORT void gr_setfillstyle(int);
DLLEXPORT void gr_setfillcolorind(int);
DLLEXPORT void gr_setcolorrep(int, float, float, float);
DLLEXPORT void gr_setwindow(float, float, float, float);
DLLEXPORT void gr_inqwindow(float *, float *, float *, float *);
DLLEXPORT void gr_setviewport(float, float, float, float);
DLLEXPORT void gr_selntran(int);
DLLEXPORT void gr_setclip(int);
DLLEXPORT void gr_setwswindow(float, float, float, float);
DLLEXPORT void gr_setwsviewport(float, float, float, float);
DLLEXPORT void gr_createseg(int);
DLLEXPORT void gr_copysegws(int);
DLLEXPORT void gr_redrawsegws(void);
DLLEXPORT void gr_setsegtran(
  int, float, float, float, float, float, float, float);
DLLEXPORT void gr_closeseg(void);
DLLEXPORT void gr_emergencyclosegks(void);
DLLEXPORT void gr_updategks(void);
DLLEXPORT int gr_setspace(float, float, int, int);
DLLEXPORT void gr_inqspace(float *, float *, int *, int *);
DLLEXPORT int gr_setscale(int);
DLLEXPORT void gr_inqscale(int *);
DLLEXPORT int gr_textext(float, float, char *);
DLLEXPORT void gr_inqtextext(float, float, char *, float *, float *);
DLLEXPORT void gr_axes(float, float, float, float, int, int, float);
DLLEXPORT void gr_grid(float, float, float, float, int, int);
DLLEXPORT void gr_verrorbars(int, float *, float *, float *, float *);
DLLEXPORT void gr_herrorbars(int, float *, float *, float *, float *);
DLLEXPORT void gr_polyline3d(int, float *, float *, float *);
DLLEXPORT void gr_axes3d(
  float, float, float, float, float, float, int, int, int, float);
DLLEXPORT void gr_titles3d(char *, char *, char *);
DLLEXPORT void gr_surface(int, int, float *, float *, float *, int);
DLLEXPORT void gr_contour(
  int, int, int, float *, float *, float *, float *, int);
DLLEXPORT void gr_setcolormap(int);
DLLEXPORT void gr_colormap(void);
DLLEXPORT float gr_tick(float, float);
DLLEXPORT void gr_adjustrange(float *, float *);
DLLEXPORT void gr_beginprint(char *);
DLLEXPORT void gr_beginprintext(char *, char *, char *, char *);
DLLEXPORT void gr_endprint(void);
DLLEXPORT int gr_openstream(char *path);
DLLEXPORT void gr_writestream(char *string, ...);
DLLEXPORT void gr_flushstream(int);
DLLEXPORT void gr_closestream(void);
DLLEXPORT void gr_ndctowc(float *, float *);
DLLEXPORT void gr_wctondc(float *, float *);
DLLEXPORT void gr_drawrect(float, float, float, float);
DLLEXPORT void gr_fillrect(float, float, float, float);
DLLEXPORT void gr_drawarc(float, float, float, float, int, int);
DLLEXPORT void gr_fillarc(float, float, float, float, int, int);
DLLEXPORT void gr_setarrowstyle(int);
DLLEXPORT void gr_drawarrow(float, float, float, float);
DLLEXPORT int gr_readimage(char *, int *, int *, int **);
DLLEXPORT void gr_drawimage(float, float, float, float, int, int, int *);
DLLEXPORT void gr_setshadow(float, float, float);
DLLEXPORT void gr_settransparency(float);
DLLEXPORT void gr_setcoordxform(float [3][2]);
DLLEXPORT void gr_begingraphics(char *);
DLLEXPORT void gr_endgraphics(void);
DLLEXPORT void gr_mathtex(float, float, char *);
DLLEXPORT void gr_beginselection(int, int);
DLLEXPORT void gr_endselection(void);
DLLEXPORT void gr_moveselection(float, float);
DLLEXPORT void gr_resizeselection(int, float, float);
DLLEXPORT void gr_inqbbox(float *, float *, float *, float *);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif
