unit gks;

interface

{$linklib GKS}

  type
     Pdouble  = ^double;
     Plongint  = ^longint;

  const

  { default connection identifier }
     GKS_K_CONID_DEFAULT = '';

  { default workstation type }
     GKS_K_WSTYPE_DEFAULT = 0;

  { aspect source flag }
     GKS_K_ASF_BUNDLED = 0;
     GKS_K_ASF_INDIVIDUAL = 1;

  { clear control flag }
     GKS_K_CLEAR_CONDITIONALLY = 0;
     GKS_K_CLEAR_ALWAYS = 1;

  { clipping indicator }
     GKS_K_NOCLIP = 0;
     GKS_K_CLIP = 1;

  { coordinate switch }
     GKS_K_COORDINATES_WC = 0;
     GKS_K_COORDINATES_NDC = 1;

  { device coordinate units }
     GKS_K_METERS = 0;
     GKS_K_OTHER_UNITS = 1;

  { fill area interior style }
     GKS_K_INTSTYLE_HOLLOW = 0;
     GKS_K_INTSTYLE_SOLID = 1;
     GKS_K_INTSTYLE_PATTERN = 2;
     GKS_K_INTSTYLE_HATCH = 3;

  { input device status }
     GKS_K_STATUS_NONE = 0;
     GKS_K_STATUS_OK = 1;

  { level of GKS }
     GKS_K_LEVEL_0A = 0;
     GKS_K_LEVEL_0B = 1;
     GKS_K_LEVEL_0C = 2;
     GKS_K_LEVEL_1A = 3;
     GKS_K_LEVEL_1B = 4;
     GKS_K_LEVEL_1C = 5;
     GKS_K_LEVEL_2A = 6;
     GKS_K_LEVEL_2B = 7;
     GKS_K_LEVEL_2C = 8;

  { operating state value }
     GKS_K_GKCL = 0;
     GKS_K_GKOP = 1;
     GKS_K_WSOP = 2;
     GKS_K_WSAC = 3;
     GKS_K_SGOP = 4;

  { regeneration flag }
     GKS_K_POSTPONE_FLAG = 0;
     GKS_K_PERFORM_FLAG = 1;

  { text alignment horizontal }
     GKS_K_TEXT_HALIGN_NORMAL = 0;
     GKS_K_TEXT_HALIGN_LEFT = 1;
     GKS_K_TEXT_HALIGN_CENTER = 2;
     GKS_K_TEXT_HALIGN_RIGHT = 3;

  { text alignment vertical }
     GKS_K_TEXT_VALIGN_NORMAL = 0;
     GKS_K_TEXT_VALIGN_TOP = 1;
     GKS_K_TEXT_VALIGN_CAP = 2;
     GKS_K_TEXT_VALIGN_HALF = 3;
     GKS_K_TEXT_VALIGN_BASE = 4;
     GKS_K_TEXT_VALIGN_BOTTOM = 5;

  { text path }
     GKS_K_TEXT_PATH_RIGHT = 0;
     GKS_K_TEXT_PATH_LEFT = 1;
     GKS_K_TEXT_PATH_UP = 2;
     GKS_K_TEXT_PATH_DOWN = 3;

  { text precision }
     GKS_K_TEXT_PRECISION_STRING = 0;
     GKS_K_TEXT_PRECISION_CHAR = 1;
     GKS_K_TEXT_PRECISION_STROKE = 2;

  { workstation category }
     GKS_K_WSCAT_OUTPUT = 0;
     GKS_K_WSCAT_INPUT = 1;
     GKS_K_WSCAT_OUTIN = 2;
     GKS_K_WSCAT_WISS = 3;
     GKS_K_WSCAT_MO = 4;
     GKS_K_WSCAT_MI = 5;

  { workstation state }
     GKS_K_WS_INACTIVE = 0;
     GKS_K_WS_ACTIVE = 1;

  { standard linetypes }
     GKS_K_LINETYPE_SOLID = 1;
     GKS_K_LINETYPE_DASHED = 2;
     GKS_K_LINETYPE_DOTTED = 3;
     GKS_K_LINETYPE_DASHED_DOTTED = 4;

  { GKS specific linetypes }
     GKS_K_LINETYPE_DASH_2_DOT = -(1);
     GKS_K_LINETYPE_DASH_3_DOT = -(2);
     GKS_K_LINETYPE_LONG_DASH = -(3);
     GKS_K_LINETYPE_LONG_SHORT_DASH = -(4);
     GKS_K_LINETYPE_SPACED_DASH = -(5);
     GKS_K_LINETYPE_SPACED_DOT = -(6);
     GKS_K_LINETYPE_DOUBLE_DOT = -(7);
     GKS_K_LINETYPE_TRIPLE_DOT = -(8);

  { standard markertypes }
     GKS_K_MARKERTYPE_DOT = 1;
     GKS_K_MARKERTYPE_PLUS = 2;
     GKS_K_MARKERTYPE_ASTERISK = 3;
     GKS_K_MARKERTYPE_CIRCLE = 4;
     GKS_K_MARKERTYPE_DIAGONAL_CROSS = 5;

  { GKS specific markertypes }
     GKS_K_MARKERTYPE_SOLID_CIRCLE = -(1);
     GKS_K_MARKERTYPE_TRIANGLE_UP = -(2);
     GKS_K_MARKERTYPE_SOLID_TRI_UP = -(3);
     GKS_K_MARKERTYPE_TRIANGLE_DOWN = -(4);
     GKS_K_MARKERTYPE_SOLID_TRI_DOWN = -(5);
     GKS_K_MARKERTYPE_SQUARE = -(6);
     GKS_K_MARKERTYPE_SOLID_SQUARE = -(7);
     GKS_K_MARKERTYPE_BOWTIE = -(8);
     GKS_K_MARKERTYPE_SOLID_BOWTIE = -(9);
     GKS_K_MARKERTYPE_HOURGLASS = -(10);
     GKS_K_MARKERTYPE_SOLID_HGLASS = -(11);
     GKS_K_MARKERTYPE_DIAMOND = -(12);
     GKS_K_MARKERTYPE_SOLID_DIAMOND = -(13);
     GKS_K_MARKERTYPE_STAR = -(14);
     GKS_K_MARKERTYPE_SOLID_STAR = -(15);
     GKS_K_MARKERTYPE_TRI_UP_DOWN = -(16);
     GKS_K_MARKERTYPE_SOLID_TRI_RIGHT = -(17);
     GKS_K_MARKERTYPE_SOLID_TRI_LEFT = -(18);
     GKS_K_MARKERTYPE_HOLLOW_PLUS = -(19);
     GKS_K_MARKERTYPE_SOLID_PLUS = -(20);
     GKS_K_MARKERTYPE_PENTAGON = -(21);
     GKS_K_MARKERTYPE_HEXAGON = -(22);
     GKS_K_MARKERTYPE_HEPTAGON = -(23);
     GKS_K_MARKERTYPE_OCTAGON = -(24);
     GKS_K_MARKERTYPE_STAR_4 = -(25);
     GKS_K_MARKERTYPE_STAR_5 = -(26);
     GKS_K_MARKERTYPE_STAR_6 = -(27);
     GKS_K_MARKERTYPE_STAR_7 = -(28);
     GKS_K_MARKERTYPE_STAR_8 = -(29);
     GKS_K_MARKERTYPE_VLINE = -(30);
     GKS_K_MARKERTYPE_HLINE = -(31);
     GKS_K_MARKERTYPE_OMARK = -(32);

  { GKS error codes }
     GKS_K_NO_ERROR = 0;
     GKS_K_ERROR = 1;

  { GKS function prototypes }

  procedure gks_open_gks(errfil : longint); cdecl; external 'GKS';
  procedure gks_close_gks; cdecl; external 'GKS';
  procedure gks_open_ws(
    wkid : longint; conid : ansistring; wtype : longint); cdecl; external 'GKS';
  procedure gks_close_ws(wkid : longint); cdecl; external 'GKS';
  procedure gks_activate_ws(wkid : longint); cdecl; external 'GKS';
  procedure gks_deactivate_ws(wkid : longint); cdecl; external 'GKS';
  procedure gks_clear_ws(wkid : longint; cofl : longint); cdecl; external 'GKS';
  procedure gks_redraw_seg_on_ws(wkid : longint); cdecl; external 'GKS';
  procedure gks_update_ws(
    wkid : longint; regfl : longint); cdecl; external 'GKS';
  procedure gks_set_deferral_state(
    wkid : longint; defmo : longint; regmo : longint); cdecl; external 'GKS';
  procedure gks_escape(
    funid : longint; dimidr : longint; idr : Plongint; maxodr : longint;
    lenodr : Plongint; odr : Plongint); cdecl; external 'GKS';
  procedure gks_message(
    wkid : longint; message : ansistring); cdecl; external 'GKS';

  procedure gks_polyline(
    n : longint; pxa, pya : array of double); cdecl; external 'GKS';
  procedure gks_polymarker(
    n : longint; pxa, pya : array of double); cdecl; external 'GKS';
  procedure gks_text(
    px : double; py : double; str : ansistring); cdecl; external 'GKS';
  procedure gks_fillarea(
    n : longint; pxa, pya : array of double); cdecl; external 'GKS';
  procedure gks_cellarray(
    xmin, xmax, ymin, ymax : double;
    dimx : longint; dimy : longint; scol : longint; srow : longint;
    ncol : longint; nrow : longint; colia : array of longint);
    cdecl; external 'GKS';

  procedure gks_set_pline_index(index : longint); cdecl; external 'GKS';
  procedure gks_set_pline_linetype(ltype : longint); cdecl; external 'GKS';
  procedure gks_set_pline_linewidth(lwidth : double); cdecl; external 'GKS';
  procedure gks_set_pline_color_index(coli : longint); cdecl; external 'GKS';
  procedure gks_set_pmark_index(index : longint); cdecl; external 'GKS';
  procedure gks_set_pmark_type(mtype : longint); cdecl; external 'GKS';
  procedure gks_set_pmark_size(mszsc : double); cdecl; external 'GKS';
  procedure gks_set_pmark_color_index(coli : longint); cdecl; external 'GKS';
  procedure gks_set_text_index(index : longint); cdecl; external 'GKS';
  procedure gks_set_text_fontprec(font : longint; prec : longint);
     cdecl; external 'GKS';
  procedure gks_set_text_expfac(chxp : double); cdecl; external 'GKS';
  procedure gks_set_text_spacing(chsp : double); cdecl; external 'GKS';
  procedure gks_set_text_color_index(coli : longint); cdecl; external 'GKS';
  procedure gks_set_text_height(chh : double); cdecl; external 'GKS';
  procedure gks_set_text_upvec(chux : double; chuy : double);
     cdecl; external 'GKS';
  procedure gks_set_text_path(txp : longint); cdecl; external 'GKS';
  procedure gks_set_text_align(txalh : longint; txalv : longint);
    cdecl; external 'GKS';
  procedure gks_set_fill_index(index : longint); cdecl; external 'GKS';
  procedure gks_set_fill_int_style(ints : longint); cdecl; external 'GKS';
  procedure gks_set_fill_style_index(styli : longint); cdecl; external 'GKS';
  procedure gks_set_fill_color_index(coli : longint); cdecl; external 'GKS';
  procedure gks_set_asf(flag : array of longint); cdecl; external 'GKS';
  procedure gks_set_color_rep(
    wkid : longint; index : longint; red, green, blue : double);
    cdecl; external 'GKS';

  procedure gks_set_window(tnr : longint; xmin, xmax, ymin, ymax : double);
    cdecl; external 'GKS';
  procedure gks_set_viewport(tnr : longint; xmin, xmax, ymin, ymax : double);
    cdecl; external 'GKS';
  procedure gks_select_xform(tnr : longint); cdecl; external 'GKS';
  procedure gks_set_clipping(clsw : longint); cdecl; external 'GKS';
  procedure gks_set_ws_window(wkid : longint; xmin, xmax, ymin, ymax : double);
    cdecl; external 'GKS';
  procedure gks_set_ws_viewport(
    wkid : longint; xmin, xmax, ymin, ymax : double); cdecl; external 'GKS';

  procedure gks_create_seg(segn : longint); cdecl; external 'GKS';
  procedure gks_close_seg; cdecl; external 'GKS';
  procedure gks_delete_seg(segn : longint); cdecl; external 'GKS';
  procedure gks_assoc_seg_with_ws(
    wkid : longint; segn : longint); cdecl; external 'GKS';
  procedure gks_copy_seg_to_ws(
    wkid : longint; segn : longint); cdecl; external 'GKS';
  procedure gks_set_seg_xform(
    segn : longint; mat : Pdouble); cdecl; external 'GKS';

  procedure gks_initialize_locator(
    wkid : longint; lcdnr : longint; tnr : longint; px, py : double;
    pet : longint; xmin, xmax, ymin, ymax : double; ldr : longint;
    datrec : ansistring); cdecl; external 'GKS';
  procedure gks_request_locator(
    wkid : longint; lcdnr : longint; var stat, tnr : longint;
    var px, py : double); cdecl; external 'GKS';
  procedure gks_request_stroke(
    wkid : longint; skdnr : longint; n : longint; stat : Plongint;
    tnr : Plongint; np : Plongint; pxa, pya : array of double);
    cdecl; external 'GKS';
  procedure gks_request_choice(
    wkid : longint; chdnr : longint; stat : Plongint; chnr : Plongint);
    cdecl; external 'GKS';
  procedure gks_request_string(
    wkid : longint; stdnr : longint; stat : Plongint;
    lostr : Plongint; str : ansistring); cdecl; external 'GKS';

  procedure gks_read_item(
    wkid : longint; lenidr : longint; maxodr : longint; odr : ansistring);
    cdecl; external 'GKS';
  procedure gks_get_item(wkid : longint; _type : Plongint; lenodr : Plongint);
    cdecl; external 'GKS';
  procedure gks_interpret_item(
    _type : longint; lenidr : longint; dimidr : longint; idr : ansistring);
    cdecl; external 'GKS';
  procedure gks_eval_xform_matrix(
    fx, fy : double; transx, transy : double; phi : double;
    scalex, scaley : double; coord : longint; tran : Pdouble);
    cdecl; external 'GKS';

  procedure gks_inq_operating_state(opsta : Plongint); cdecl; external 'GKS';
  procedure gks_inq_level(var errind, lev : longint); cdecl; external 'GKS';
  procedure gks_inq_wstype(
    n : longint; var errind, number, wtype : longint) cdecl; external 'GKS';
  procedure gks_inq_max_xform(var errind, maxtnr : longint);
    cdecl; external 'GKS';
  procedure gks_inq_open_ws(
    n : longint; var errind, ol, wkid : longint); cdecl; external 'GKS';
  procedure gks_inq_active_ws(
    n : longint; var errind, ol, wkid : longint); cdecl; external 'GKS';
  procedure gks_inq_segn_ws(
    wkid : longint; n : longint; var errind, ol, segn : longint);
    cdecl; external 'GKS';
  procedure gks_inq_pline_linetype(var errind, ltype : longint);
    cdecl; external 'GKS';
  procedure gks_inq_pline_linewidth(var errind : longint; var lwidth : double);
    cdecl; external 'GKS';
  procedure gks_inq_pline_color_index(var errind, coli : longint);
    cdecl; external 'GKS';
  procedure gks_inq_pmark_type(var errind, mtype : longint);
    cdecl; external 'GKS';
  procedure gks_inq_pmark_size(var errind : longint; var mszsc : double);
    cdecl; external 'GKS';
  procedure gks_inq_pmark_color_index(var errind, coli : longint);
    cdecl; external 'GKS';
  procedure gks_inq_text_fontprec(var errind, font, prec : longint);
    cdecl; external 'GKS';
  procedure gks_inq_text_expfac(var errind : longint; var chxp : double);
    cdecl; external 'GKS';
  procedure gks_inq_text_spacing(var errind : longint; var chsp : double);
    cdecl; external 'GKS';
  procedure gks_inq_text_color_index(var errind, coli : longint);
    cdecl; external 'GKS';
  procedure gks_inq_text_height(var errind : longint; var chh : double);
    cdecl; external 'GKS';
  procedure gks_inq_text_upvec(var errind : longint; var chux, chuy : double);
    cdecl; external 'GKS';
  procedure gks_inq_text_path(var errind, txp : longint); cdecl; external 'GKS';
  procedure gks_inq_text_align(var errind, txalh, txalv : longint);
    cdecl; external 'GKS';
  procedure gks_inq_fill_int_style(var errind, ints : longint);
    cdecl; external 'GKS';
  procedure gks_inq_fill_style_index(var errind, styli : longint);
    cdecl; external 'GKS';
  procedure gks_inq_fill_color_index(var errind, coli : longint);
    cdecl; external 'GKS';
  procedure gks_inq_open_segn(var errind, segn : longint);
    cdecl; external 'GKS';
  procedure gks_inq_current_xformno(var errind, tnr : longint);
    cdecl; external 'GKS';
  procedure gks_inq_xform(
    tnr : longint; var errind : longint; var wn, vp : array of double);
    cdecl; external 'GKS';
  procedure gks_inq_clip(
    var errind, clsw : longint; var clrt : array of double);
    cdecl; external 'GKS';
  procedure gks_inq_ws_conntype(
    wkid : longint; var errind, conid, wtype : longint); cdecl; external 'GKS';
  procedure gks_inq_ws_category(
    wkid : longint; var errind, wscat : longint); cdecl; external 'GKS';
  procedure gks_inq_text_extent(
    wkid : longint; px, py : double; str : ansistring; var errind : longint;
    var cpx, cpy : double; var tx, ty : array of double); cdecl; external 'GKS';
  procedure gks_inq_max_ds_size(
    wtype : longint; var errind, dcunit : longint; var rx, ry : double;
    var lx, ly : longint); cdecl; external 'GKS';

  procedure gks_emergency_close; cdecl; external 'GKS';

  procedure gks_set_text_slant(slant : double); cdecl; external 'GKS';
  procedure gks_draw_image(
    x, y, scalex, scaley : double; width, height : longint; data : Plongint);
    cdecl; external 'GKS';
  procedure gks_set_shadow(offsetx, offsety, blur : double);
    cdecl; external 'GKS';
  procedure gks_set_transparency(alpha : double); cdecl; external 'GKS';
  procedure gks_set_coord_xform(mat : Pdouble); cdecl; external 'GKS';

  procedure gks_begin_selection(index, kind : longint); cdecl; external 'GKS';
  procedure gks_end_selection; cdecl; external 'GKS';
  procedure gks_move_selection(x : double; y : double); cdecl; external 'GKS';
  procedure gks_resize_selection(
    kind : longint; x : double; y : double); cdecl; external 'GKS';

  procedure gks_inq_text_slant(var errind : longint; var slant : double); cdecl;
    external 'GKS';

implementation

end.
