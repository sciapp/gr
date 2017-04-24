program demo(input, output);

uses
  gks;

var
  asf : array[1..13] of longint;
  x, y : array[1..10] of double;
  cpx, cpy : double;
  tx, ty : array[1..4] of double;
  i, j, k : longint;
  colia : array[1..80] of longint;
  inp_status, tnr, errind : longint;

begin

  for i := 1 to 13 do
    asf[i] := GKS_K_ASF_INDIVIDUAL;

  for i := 1 to 80 do
    colia[i] := i - 1;

  gks_open_gks(6);
  gks_set_asf(asf);
  if ARGC > 1 then
    begin
    gks_open_ws(1, GKS_K_CONID_DEFAULT, 5);
    gks_activate_ws(1);
    gks_create_seg(1);
    k := 10;
    end
  else
    begin
    gks_open_ws(2, GKS_K_CONID_DEFAULT, GKS_K_WSTYPE_DEFAULT);
    gks_activate_ws(2);
    k := 1;
    end;
  gks_set_text_fontprec(12, GKS_K_TEXT_PRECISION_STROKE);

  { Text }
  gks_set_text_height(0.02);
  gks_set_text_fontprec(-1, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.7, 'Font-1');
  gks_set_text_fontprec(-2, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.65, 'Font-2');
  gks_set_text_fontprec(-3, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.6, 'Font-3');
  gks_set_text_fontprec(-4, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.55, 'Font-4');
  gks_set_text_fontprec(-5, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.5, 'Font-5');
  gks_set_text_fontprec(-6, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.45, 'Font-6');
  gks_set_text_fontprec(-7, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.4, 'Font-7');
  gks_set_text_fontprec(-8, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.35, 'Font-8');
  gks_set_text_fontprec(-9, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.3, 'Font-9');
  gks_set_text_fontprec(-10, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.25, 'Font-10');
  gks_set_text_fontprec(-11, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.2, 'Font-11');
  gks_set_text_fontprec(-12, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.7, 'Font-12');
  gks_set_text_fontprec(-13, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.65, 'Font-13');
  gks_set_text_fontprec(-14, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.6, 'Font-14');
  gks_set_text_fontprec(-15, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.55, 'Font-15');
  gks_set_text_fontprec(-16, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.5, 'Font-16');
  gks_set_text_fontprec(-17, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.45, 'Font-17');
  gks_set_text_fontprec(-18, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.4, 'Font-18');
  gks_set_text_fontprec(-19, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.35, 'Font-19');
  gks_set_text_fontprec(-20, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.3, 'Font-20');
  gks_set_text_fontprec(-21, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.25, 'Font-21');
  gks_set_text_fontprec(-22, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.2, 'Font-22');
  gks_set_text_fontprec(-23, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.6, 0.15, 'Font-23');
  gks_set_text_fontprec(-24, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.8, 0.15, 'Font-24');

  { Colors }
  gks_set_text_fontprec(-5, GKS_K_TEXT_PRECISION_STROKE);
  gks_set_text_color_index(0);
  gks_text(0.45, 0.35, 'White');
  gks_set_text_color_index(1);
  gks_text(0.45, 0.32, 'Black');
  gks_set_text_color_index(2);
  gks_text(0.45, 0.29, 'Red');
  gks_set_text_color_index(3);
  gks_text(0.45, 0.26, 'Green');
  gks_set_text_color_index(4);
  gks_text(0.45, 0.23, 'Blue');
  gks_set_text_color_index(5);
  gks_text(0.45, 0.20, 'Cyan');
  gks_set_text_color_index(6);
  gks_text(0.45, 0.17, 'Yellow');
  gks_set_text_color_index(7);
  gks_text(0.45, 0.14, 'Magenta');

  { Linetypes }
  x[1] := 0.10;
  x[2] := 0.26;
  y[1] := 0.95;
  y[2] := y[1];

  for j := -8 to 4 do
    if j <> 0 then
      begin
      gks_set_pline_linetype(j);
      y[1] := y[1] - 0.05;
      y[2] := y[1];
      gks_polyline(2, x, y);
      end;

  { Markertypes }
  gks_set_pmark_size(3.5);

  x[1] := 0.25;
  y[1] := 0.95;
  for j := -32 to -21 do
    begin
    gks_set_pmark_type(j);
    x[1] := x[1] + 0.06;
    gks_polymarker(1, x, y);
    end;

  x[1] := 0.25;
  y[1] := 0.875;
  for j := -20 to -9 do
    begin
    gks_set_pmark_type(j);
    x[1] := x[1] + 0.06;
    gks_polymarker(1, x, y);
    end;

  x[1] := 0.25;
  y[1] := 0.8;
  for j := -8 to 4 do
    if j <> 0 then
      begin
      gks_set_pmark_type(j);
      x[1] := x[1] + 0.06;
      gks_polymarker(1, x, y);
      end;

  { Fill Areas }
  x[1] := 0.02;
  x[2] := 0.12;
  x[3] := 0.12;
  x[4] := 0.02;
  x[5] := x[1];
  y[1] := 0.02;
  y[2] := 0.02;
  y[3] := 0.12;
  y[4] := 0.12;
  y[5] := y[1];

  gks_set_fill_style_index(4);
  for j := 0 to 3 do
    begin
    for i := 1 to 5 do
      begin
      x[i] := x[i] + 0.1;
      y[i] := y[i] + 0.1;
      end;
    gks_set_fill_int_style(j);
    gks_fillarea(5, x, y);
    end;

  { Patterns }
  x[1] := 0.02;
  x[2] := 0.07;
  x[3] := 0.07;
  x[4] := 0.02;
  x[5] := x[1];
  y[1] := 0.2;
  y[2] := 0.2;
  y[3] := 0.25;
  y[4] := 0.25;
  y[5] := y[1];

  gks_set_fill_int_style(2);
  for j := 4 to 15 do
    begin
    for i := 1 to 5 do
      y[i] := y[i] + 0.06;
    gks_set_fill_style_index(j - 3);
    gks_fillarea(5, x, y);
    end;

  { Cell Array }
  gks_cellarray(0.332, 0.75, 0.532, 0.55, 8, 10, 1, 1, 8, 10, colia);

  { More Text }
  gks_set_text_height(0.08);
  gks_set_text_align(2, 3);
  gks_set_text_color_index(1);
  gks_set_text_fontprec(12, GKS_K_TEXT_PRECISION_STROKE);
  gks_text(0.5, 0.05, 'Hello World');

  gks_inq_text_extent(2, 0.5, 0.05, 'Hello World', errind, cpx, cpy, tx, ty);
  gks_set_fill_int_style(0);
  gks_fillarea(4, tx, ty);

  gks_set_text_height(0.024);
  gks_set_text_align(1, 1);
  gks_set_text_fontprec(3, GKS_K_TEXT_PRECISION_STRING);
  gks_set_text_upvec(-1, 0);
  gks_set_text_color_index(200);
  gks_text(0.05, 0.15, 'up');
  gks_set_text_upvec(0, -1);
  gks_set_text_color_index(400);
  gks_text(0.05, 0.15, 'left');
  gks_set_text_upvec(1, 0);
  gks_set_text_color_index(600);
  gks_text(0.05, 0.15, 'down');
  gks_set_text_upvec(0, 1);
  gks_set_text_color_index(800);
  gks_text(0.05, 0.15, 'right');

  if ARGC > 1 then
    begin
    gks_close_seg();
    gks_deactivate_ws(1);
    gks_open_ws(2, GKS_K_CONID_DEFAULT, GKS_K_WSTYPE_DEFAULT);
    gks_activate_ws(2);

    while k > 0 do
      begin
      k := k - 1;
      gks_clear_ws(2, GKS_K_CLEAR_ALWAYS);
      gks_copy_seg_to_ws(2, 1);
      gks_update_ws(2, GKS_K_POSTPONE_FLAG);
      writeln(k);
      end;
    end
  else
    begin
    gks_update_ws(2, GKS_K_POSTPONE_FLAG);
    gks_set_pmark_type(2);
    gks_request_locator(2, 1, inp_status, tnr, x[1], y[1]);
    while inp_status = 1 do
      begin
      gks_polymarker(1, x, y);
      gks_request_locator(2, 1, inp_status, tnr, x[1], y[1]);
      end;

    gks_emergency_close();

    writeln(x[1], y[1]);
    end;
end.
