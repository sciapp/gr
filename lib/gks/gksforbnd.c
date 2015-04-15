
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gks.h"
#include "gksforbnd.h"

#if defined (_WIN32) && !defined (__GNUC__)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

static
double *x = NULL, *y = NULL;

static
int max_points = 0;

static
void gksrealloc(int n)
{
  if (n > max_points)
    {
      x = (double *) realloc(x, sizeof(double) * n);
      y = (double *) realloc(y, sizeof(double) * n);
      max_points = n;
    }
}

void STDCALL GOPKS(int *errfil, int *buffer)
{
  gks_open_gks(2);
}

void STDCALL GCLKS(void)
{
  gks_close_gks();
}

void STDCALL GOPWK(int *wkid, int *conid, int *wtype)
{
  static char s[32];

  if (*wtype >= 210 && *wtype <= 212 && (unsigned int) *conid >= 100 + 100)
    {
      sprintf(s, "GKS_CONID=%p", (void *) conid);
      putenv(s);

      gks_open_ws(*wkid, GKS_K_CONID_DEFAULT, 213);
    }
  else if (*wtype < 301 && *conid != 0)
    {
      sprintf(s, "GKS_CONID=");
      putenv(s);
      sprintf(s, "!%d", *conid);

      gks_open_ws(*wkid, s, *wtype);
    }
  else
    gks_open_ws(*wkid, NULL, *wtype);
}

void STDCALL GCLWK(int *wkid)
{
  gks_close_ws(*wkid);
}

void STDCALL GACWK(int *wkid)
{
  gks_activate_ws(*wkid);
}

void STDCALL GDAWK(int *wkid)
{
  gks_deactivate_ws(*wkid);
}

void STDCALL GCLRWK(int *wkid, int *cofl)
{
  gks_clear_ws(*wkid, *cofl);
}

void STDCALL GRSGWK(int *wkid)
{
  gks_redraw_seg_on_ws(*wkid);
}

void STDCALL GUWK(int *wkid, int *regfl)
{
  gks_update_ws(*wkid, *regfl);
}

void STDCALL GSDS(int *wkid, int *defmo, int *regmo)
{
  gks_set_deferral_state(*wkid, *defmo, *regmo);
}

void STDCALL GESC(
  int *funid, int *dimidr, int *idr, int *maxodr, int *lenodr, int *odr)
{
  gks_escape(*funid, *dimidr, idr, *maxodr, lenodr, odr);
}

void STDCALL GMSG(int *wkid, char *chars, unsigned short len)
{
  char s[256];
  int n = len;

  if (n > 255)
    n = 255;
  strncpy(s, chars, n);
  s[n] = '\0';

  gks_message(*wkid, s);
}

void STDCALL GPL(int *n, float *pxa, float *pya)
{
  int i;

  gksrealloc(*n);
  for (i = 0; i < *n; i++)
    {
      x[i] = pxa[i];
      y[i] = pya[i];
    }
  gks_polyline(*n, x, y);
}

void STDCALL GPM(int *n, float *pxa, float *pya)
{
  int i;

  gksrealloc(*n);
  for (i = 0; i < *n; i++)
    {
      x[i] = pxa[i];
      y[i] = pya[i];
    }
  gks_polymarker(*n, x, y);
}

void STDCALL GTX(float *px, float *py, char *chars, unsigned short len)
{
  double _x = (double) *px, _y = (double) *py;
  char s[256];
  int n = len;

  if (n > 255)
    n = 255;
  strncpy(s, chars, n);
  s[n] = '\0';
 
  gks_text(_x, _y, s);
}

void STDCALL GTXS(
  float *px, float *py, int *nchars, char *chars, unsigned short len)
{
  double _x = (double) *px, _y = (double) *py;
  char s[256];
  int n = *nchars;

  if (n > 255)
    n = 255;
  strncpy(s, chars, n);
  s[n] = '\0';
 
  gks_text(_x, _y, s);
}

void STDCALL GFA(int *n, float *pxa, float *pya)
{
  int i;

  gksrealloc(*n);
  for (i = 0; i < *n; i++)
    {
      x[i] = pxa[i];
      y[i] = pya[i];
    }
  gks_fillarea(*n, x, y);
}

void STDCALL GCA(
  float *qx, float *qy, float *rx, float *ry,
  int *dimx, int *dimy, int *scol, int *srow, int *ncol, int *nrow, int *colia)
{
  double _qx = (double) *qx, _qy = (double) *qy;
  double _rx = (double) *rx, _ry = (double) *ry;

  gks_cellarray(_qx, _qy, _rx, _ry,
		*dimx, *dimy, *scol, *srow, *ncol, *nrow, colia);
}

void STDCALL GSPLI(int *index)
{
  gks_set_pline_index(*index);
}

void STDCALL GSLN(int *ltype)
{
  gks_set_pline_linetype(*ltype);
}

void STDCALL GSLWSC(float *lwidth)
{
  double _lwidth = (double) *lwidth;

  gks_set_pline_linewidth(_lwidth);
}

void STDCALL GSPLCI(int *index)
{
  gks_set_pline_color_index(*index);
}

void STDCALL GSPMI(int *coli)
{
  gks_set_pmark_index(*coli);
}

void STDCALL GSMK(int *mtype)
{
  gks_set_pmark_type(*mtype);
}

void STDCALL GSMKSC(float *mszsc)
{
  double _mszsc = (double) *mszsc;

  gks_set_pmark_size(_mszsc);
}

void STDCALL GSPMCI(int *coli)
{
  gks_set_pmark_color_index(*coli);
}

void STDCALL GSTXI(int *index)
{
  gks_set_text_index(*index);
}

void STDCALL GSTXFP(int *font, int *prec)
{
  gks_set_text_fontprec(*font, *prec);
}

void STDCALL GSCHXP(float *chxp)
{
  double _chxp = (double) *chxp;

  gks_set_text_expfac(_chxp);
}

void STDCALL GSCHSP(float *chsp)
{
  double _chsp = (double) *chsp;

  gks_set_text_spacing(_chsp);
}

void STDCALL GSTXCI(int *coli)
{
  gks_set_text_color_index(*coli);
}

void STDCALL GSCHH(float *chh)
{
  double _chh = (double) *chh;

  gks_set_text_height(_chh);
}

void STDCALL GSCHUP(float *chux, float *chuy)
{
  double _chux = (double) *chux, _chuy = (double) *chuy;

  gks_set_text_upvec(_chux, _chuy);
}

void STDCALL GSTXP(int *txp)
{
  gks_set_text_path(*txp);
}

void STDCALL GSTXAL(int *txalh, int *txalv)
{
  gks_set_text_align(*txalh, *txalv);
}

void STDCALL GSFAI(int *index)
{
  gks_set_fill_index(*index);
}

void STDCALL GSFAIS(int *ints)
{
  gks_set_fill_int_style(*ints);
}

void STDCALL GSFASI(int *styli)
{
  gks_set_fill_style_index(*styli);
}

void STDCALL GSFACI(int *coli)
{
  gks_set_fill_color_index(*coli);
}

void STDCALL GSASF(int *flag)
{
  gks_set_asf(flag);
}

void STDCALL GSCR(int *wkid, int *index, float *red, float *green, float *blue)
{
  double _red = (double) *red, _green = (double) *green, _blue = (double) *blue;

  gks_set_color_rep(*wkid, *index, _red, _green, _blue);
}

void STDCALL GSWN(int *tnr, float *xmin, float *xmax, float *ymin, float *ymax)
{
  double _xmin = (double) *xmin, _xmax = (double) *xmax;
  double _ymin = (double) *ymin, _ymax = (double) *ymax;

  gks_set_window(*tnr, _xmin, _xmax, _ymin, _ymax);
}

void STDCALL GSVP(int *tnr, float *xmin, float *xmax, float *ymin, float *ymax)
{
  double _xmin = (double) *xmin, _xmax = (double) *xmax;
  double _ymin = (double) *ymin, _ymax = (double) *ymax;

  gks_set_viewport(*tnr, _xmin, _xmax, _ymin, _ymax);
}

void STDCALL GSELNT(int *tnr)
{
  gks_select_xform(*tnr);
}

void STDCALL GSCLIP(int *clsw)
{
  gks_set_clipping(*clsw);
}

void STDCALL GSWKWN(
  int *wkid, float *xmin, float *xmax, float *ymin, float *ymax)
{
  double _xmin = (double) *xmin, _xmax = (double) *xmax;
  double _ymin = (double) *ymin, _ymax = (double) *ymax;

  gks_set_ws_window(*wkid, _xmin, _xmax, _ymin, _ymax);
}

void STDCALL GSWKVP(
  int *wkid, float *xmin, float *xmax, float *ymin, float *ymax)
{
  double _xmin = (double) *xmin, _xmax = (double) *xmax;
  double _ymin = (double) *ymin, _ymax = (double) *ymax;

  gks_set_ws_viewport(*wkid, _xmin, _xmax, _ymin, _ymax);
}

void STDCALL GCRSG(int *segn)
{
  gks_create_seg(*segn);
}

void STDCALL GCLSG(void)
{
  gks_close_seg();
}

void STDCALL GDSG(int *segn)
{
  gks_delete_seg(*segn);
}

void STDCALL GASGWK(int *wkid, int *segn)
{
  gks_assoc_seg_with_ws(*wkid, *segn);
}

void STDCALL GCSGWK(int *wkid, int *segn)
{
  gks_copy_seg_to_ws(*wkid, *segn);
}

void STDCALL GSSGT(int *segn, float mat[3][2])
{
  int i, j;
  double _mat[3][2];

  for (i = 0; i < 3; i++)
    for (j = 0; j < 2; j++)
       _mat[i][j] = mat[i][j];

  gks_set_seg_xform(*segn, _mat);
}

void STDCALL GINLC(
  int *wkid, int *lcdnr, int *tnr, float *px, float *py, int *pet,
  float *xmin, float *xmax, float *ymin, float *ymax, int *ldr, char *datrec,
  unsigned short len)
{
  double _x = (double) *px, _y = (double) *py;
  double _xmin = (double) *xmin, _xmax = (double) *xmax;
  double _ymin = (double) *ymin, _ymax = (double) *ymax;

  gks_initialize_locator(*wkid, *lcdnr, *tnr, _x, _y, *pet,
			 _xmin, _xmax, _ymin, _ymax, *ldr, datrec);
}

void STDCALL GRQLC(
  int *wkid, int *lcdnr, int *stat, int *tnr, float *px, float *py)
{
  double _px, _py;

  gks_request_locator(*wkid, *lcdnr, stat, tnr, &_px, &_py);
  *px = _px;
  *py = _py;
}

void STDCALL GRQSK(
  int *wkid, int *skdnr, int *n,
  int *stat, int *tnr, int *np, float *pxa, float *pya)
{
  int i;
  
  gksrealloc(*n);
  gks_request_stroke(*wkid, *skdnr, *n, stat, tnr, np, x, y);
  for (i = 0; i < *np; i++)
    {
      pxa[i] = x[i];
      pya[i] = y[i];
    }
}

void STDCALL GRQCH(int *wkid, int *chdnr, int *stat, int *chnr)
{
  gks_request_choice(*wkid, *chdnr, stat, chnr);
}

void STDCALL GRQST(
  int *wkid, int *stdnr, int *stat, int *lostr, char *str, unsigned short len)
{
  gks_request_string(*wkid, *stdnr, stat, lostr, str);
}

void STDCALL GRDITM(
  int *wkid, int *lenidr, int *maxodr, char *odr, unsigned short len)
{
  gks_read_item(*wkid, *lenidr, *maxodr, odr);
}

void STDCALL GGTITM(int *wkid, int *type, int *lenodr)
{
  gks_get_item(*wkid, type, lenodr);
}

void STDCALL GIITM(
  int *type, int *lenidr, int *dimidr, char *idr, unsigned short len)
{
  gks_interpret_item(*type, *lenidr, *dimidr, idr);
}

void STDCALL GEVTM(
  float *fx, float *fy, float *transx, float *transy, float *phi,
  float *scalex, float *scaley, int *coord, float mat[3][2])
{
  double _fx = (double) *fx, _fy = (double) *fy;
  double _transx = (double) *transx, _transy = (double) *transy;
  double _phi = (double) *phi;
  double _scalex = (double) *scalex, _scaley = (double) *scaley;
  int i, j;
  double _mat[3][2];

  gks_eval_xform_matrix(
    _fx, _fy, _transx, _transy, _phi, _scalex, _scaley, *coord, _mat);

  for (i = 0; i < 3; i++)
    for (j = 0; j < 2; j++)
       mat[i][j] = _mat[i][j];
}

void STDCALL GQOPS(int *opsta)
{
  gks_inq_operating_state(opsta);
}

void STDCALL GQLVKS(int *errind, int *lev)
{
  gks_inq_level(errind, lev);
}

void STDCALL GQEWK(int *n, int *errind, int *number, int *wtype)
{
  gks_inq_wstype(*n, errind, number, wtype);
}

void STDCALL GQMNTN(int *errind, int *maxtnr)
{
  gks_inq_max_xform(errind, maxtnr);
}

void STDCALL GQOPWK(int *n, int *errind, int *ol, int *wkid)
{
  gks_inq_open_ws(*n, errind, ol, wkid);
}

void STDCALL GQACWK(int *n, int *errind, int *ol, int *wkid)
{
  gks_inq_active_ws(*n, errind, ol, wkid);
}

void STDCALL GQSGWK(int *wkid, int *n, int *errind, int *ol, int *segn)
{
  gks_inq_segn_ws(*wkid, *n, errind, ol, segn);
}

void STDCALL GQLN(int *errind, int *ltype)
{
  gks_inq_pline_linetype(errind, ltype);
}

void STDCALL GQLWSC(int *errind, float *lwidth)
{
  double _lwidth;
  
  gks_inq_pline_linewidth(errind, &_lwidth);
  *lwidth = (float) _lwidth;
}

void STDCALL GQPLCI(int *errind, int *coli)
{
  gks_inq_pline_color_index(errind, coli);
}

void STDCALL GQMK(int *errind, int *mtype)
{
  gks_inq_pmark_type(errind, mtype);
}

void STDCALL GQMKSC(int *errind, float *mszsc)
{
  double _mszsc;
  
  gks_inq_pmark_size(errind, &_mszsc);
  *mszsc = (float) _mszsc;
}

void STDCALL GQPMCI(int *errind, int *coli)
{
  gks_inq_pmark_color_index(errind, coli);
}

void STDCALL GQTXFP(int *errind, int *font, int *prec)
{
  gks_inq_text_fontprec(errind, font, prec);
}

void STDCALL GQCHXP(int *errind, float *chxp)
{
  double _chxp;

  gks_inq_text_expfac(errind, &_chxp);
  *chxp = _chxp;
}

void STDCALL GQCHSP(int *errind, float *chsp)
{
  double _chsp;
  
  gks_inq_text_spacing(errind, &_chsp);
  *chsp = _chsp;
}

void STDCALL GQTXCI(int *errind, int *coli)
{
  gks_inq_text_color_index(errind, coli);
}

void STDCALL GQCHH(int *errind, float *chh)
{
  double _chh;
  
  gks_inq_text_height(errind, &_chh);
  *chh = _chh;
}

void STDCALL GQCHUP(int *errind, float *chux, float *chuy)
{
  double _chux, _chuy;
  
  gks_inq_text_upvec(errind, &_chux, &_chuy);
  *chux = _chux;
  *chuy = _chuy;
}

void STDCALL GQTXP(int *errind, int *txp)
{
  gks_inq_text_path(errind, txp);
}

void STDCALL GQTXAL(int *errind, int *txalh, int *txalv)
{
  gks_inq_text_align(errind, txalh, txalv);
}

void STDCALL GQFAIS(int *errind, int *ints)
{
  gks_inq_fill_int_style(errind, ints);
}

void STDCALL GQFASI(int *errind, int *styli)
{
  gks_inq_fill_style_index(errind, styli);
}

void STDCALL GQFACI(int *errind, int *coli)
{
  gks_inq_fill_color_index(errind, coli);
}

void STDCALL GQOPSG(int *errind, int *segn)
{
  gks_inq_open_segn(errind, segn);
}

void STDCALL GQCNTN(int *errind, int *tnr)
{
  gks_inq_current_xformno(errind, tnr);
}

void STDCALL GQNT(int *tnr, int *errind, float *wn, float *vp)
{
  double _wn[4], _vp[4];
  int i;

  gks_inq_xform(*tnr, errind, _wn, _vp);
  for (i = 0; i < 4; i++)
    {
      wn[i] = (float) _wn[i];
      vp[i] = (float) _vp[i];
    }
}

void STDCALL GQCLIP(int *errind, int *clsw, float *clrt)
{
  double _clrt[4];
  int i;

  gks_inq_clip(errind, clsw, _clrt);
  for (i = 0; i < 4; i++)
    clrt[i] = (float) _clrt[i];
}

void STDCALL GQWKC(int *wkid, int *errind, int *conid, int *wtype)
{
  gks_inq_ws_conntype(*wkid, errind, conid, wtype);
}

void STDCALL GQWKCA(int *wtype, int *errind, int *wscat)
{
  gks_inq_ws_category(*wtype, errind, wscat);
}

#if defined (_WIN32) && !defined (__GNUC__)
void STDCALL GQTXX(
  int *wkid, float *px, float *py, char *str, unsigned short len,
  int *errind, float *cpx, float *cpy, float *tx, float *ty)
#else
void STDCALL GQTXX(
  int *wkid, float *px, float *py, char *str,
  int *errind, float *cpx, float *cpy, float *tx, float *ty,
  unsigned short len)
#endif
{
  double _x = (double) *px, _y = (double) *py;
  char s[256];
  int i, n = len;
  double _cpx, _cpy, _tx[4], _ty[4];

  if (n > 255)
    n = 255;
  strncpy(s, str, n);
  s[n] = '\0';

  gks_inq_text_extent(*wkid, _x, _y, s, errind, &_cpx, &_cpy, _tx, _ty);
  *cpx = _cpx;
  *cpy = _cpy;
  for (i = 0; i < 4; i++)
    {
      tx[i] = (float) _tx[i];
      ty[i] = (float) _ty[i];
    }
}

#if defined (_WIN32) && !defined (__GNUC__)
void STDCALL GQTXXS(
  int *wkid, float *px, float *py, int *nchars, char *str, unsigned short len,
  int *errind, float *cpx, float *cpy, float *tx, float *ty)
#else
void STDCALL GQTXXS(
  int *wkid, float *px, float *py, int *nchars, char *str,
  int *errind, float *cpx, float *cpy, float *tx, float *ty,
  unsigned short len)
#endif
{
  double _x = (double) *px, _y = (double) *py;
  char s[256];
  int i, n = *nchars;
  double _cpx, _cpy, _tx[4], _ty[4];

  if (n > 255)
    n = 255;
  strncpy(s, str, n);
  s[n] = '\0';

  gks_inq_text_extent(*wkid, _x, _y, s, errind, &_cpx, &_cpy, _tx, _ty);
  *cpx = _cpx;
  *cpy = _cpy;
  for (i = 0; i < 4; i++)
    {
      tx[i] = (float) _tx[i];
      ty[i] = (float) _ty[i];
    }
}

void STDCALL GQDSP(
  int *wtype, int *errind, int *dcunit, float *rx, float *ry, int *lx, int *ly)
{
  double _rx, _ry;
  
  gks_inq_max_ds_size(*wtype, errind, dcunit, &_rx, &_ry, lx, ly);
  *rx = _rx;
  *ry = _ry;
}

void STDCALL GECLKS(void)
{
  gks_emergency_close();
}
