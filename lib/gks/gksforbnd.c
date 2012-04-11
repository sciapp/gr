
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
  gks_polyline(*n, pxa, pya);
}

void STDCALL GPM(int *n, float *pxa, float *pya)
{
  gks_polymarker(*n, pxa, pya);
}

void STDCALL GTX(float *px, float *py, char *chars, unsigned short len)
{
  char s[256];
  int n = len;

  if (n > 255)
    n = 255;
  strncpy(s, chars, n);
  s[n] = '\0';
 
  gks_text(*px, *py, s);
}

void STDCALL GTXS(
  float *px, float *py, int *nchars, char *chars, unsigned short len)
{
  char s[256];
  int n = *nchars;

  if (n > 255)
    n = 255;
  strncpy(s, chars, n);
  s[n] = '\0';
 
  gks_text(*px, *py, s);
}

void STDCALL GFA(int *n, float *pxa, float *pya)
{
  gks_fillarea(*n, pxa, pya);
}

void STDCALL GCA(
  float *xmin, float *xmax, float *ymin, float *ymax,
  int *dimx, int *dimy, int *scol, int *srow, int *ncol, int *nrow, int *colia)
{
  gks_cellarray(*xmin, *xmax, *ymin, *ymax,
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
  gks_set_pline_linewidth(*lwidth);
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
  gks_set_pmark_size(*mszsc);
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
  gks_set_text_expfac(*chxp);
}

void STDCALL GSCHSP(float *chsp)
{
  gks_set_text_spacing(*chsp);
}

void STDCALL GSTXCI(int *coli)
{
  gks_set_text_color_index(*coli);
}

void STDCALL GSCHH(float *chh)
{
  gks_set_text_height(*chh);
}

void STDCALL GSCHUP(float *chux, float *chuy)
{
  gks_set_text_upvec(*chux, *chuy);
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
  gks_set_color_rep(*wkid, *index, *red, *green, *blue);
}

void STDCALL GSWN(int *tnr, float *xmin, float *xmax, float *ymin, float *ymax)
{
  gks_set_window(*tnr, *xmin, *xmax, *ymin, *ymax);
}

void STDCALL GSVP(int *tnr, float *xmin, float *xmax, float *ymin, float *ymax)
{
  gks_set_viewport(*tnr, *xmin, *xmax, *ymin, *ymax);
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
  gks_set_ws_window(*wkid, *xmin, *xmax, *ymin, *ymax);
}

void STDCALL GSWKVP(
  int *wkid, float *xmin, float *xmax, float *ymin, float *ymax)
{
  gks_set_ws_viewport(*wkid, *xmin, *xmax, *ymin, *ymax);
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
  gks_set_seg_xform(*segn, mat);
}

void STDCALL GINLC(
  int *wkid, int *lcdnr, int *tnr, float *px, float *py, int *pet,
  float *xmin, float *xmax, float *ymin, float *ymax, int *ldr, char *datrec,
  unsigned short len)
{
  gks_initialize_locator(*wkid, *lcdnr, *tnr, *px, *py, *pet,
			 *xmin, *xmax, *ymin, *ymax, *ldr, datrec);
}

void STDCALL GRQLC(
  int *wkid, int *lcdnr, int *stat, int *tnr, float *px, float *py)
{
  gks_request_locator(*wkid, *lcdnr, stat, tnr, px, py);
}

void STDCALL GRQSK(
  int *wkid, int *skdnr, int *n,
  int *stat, int *tnr, int *np, float *pxa, float *pya)
{
  gks_request_stroke(*wkid, *skdnr, *n, stat, tnr, np, pxa, pya);
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
  gks_eval_xform_matrix(
    *fx, *fy, *transx, *transy, *phi, *scalex, *scaley, *coord, mat);
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
  gks_inq_pline_linewidth(errind, lwidth);
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
  gks_inq_pmark_size(errind, mszsc);
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
  gks_inq_text_expfac(errind, chxp);
}

void STDCALL GQCHSP(int *errind, float *chsp)
{
  gks_inq_text_spacing(errind, chsp);
}

void STDCALL GQTXCI(int *errind, int *coli)
{
  gks_inq_text_color_index(errind, coli);
}

void STDCALL GQCHH(int *errind, float *chh)
{
  gks_inq_text_height(errind, chh);
}

void STDCALL GQCHUP(int *errind, float *chux, float *chuy)
{
  gks_inq_text_upvec(errind, chux, chuy);
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
  gks_inq_xform(*tnr, errind, wn, vp);
}

void STDCALL GQCLIP(int *errind, int *clsw, float *clrt)
{
  gks_inq_clip(errind, clsw, clrt);
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
  char s[256];
  int n = len;

  if (n > 255)
    n = 255;
  strncpy(s, str, n);
  s[n] = '\0';

  gks_inq_text_extent(*wkid, *px, *py, s, errind, cpx, cpy, tx, ty);
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
  char s[256];
  int n = *nchars;

  if (n > 255)
    n = 255;
  strncpy(s, str, n);
  s[n] = '\0';

  gks_inq_text_extent(*wkid, *px, *py, s, errind, cpx, cpy, tx, ty);
}

void STDCALL GQDSP(
  int *wtype, int *errind, int *dcunit, float *rx, float *ry, int *lx, int *ly)
{
  gks_inq_max_ds_size(*wtype, errind, dcunit, rx, ry, lx, ly);
}

void STDCALL GECLKS(void)
{
  gks_emergency_close();
}
