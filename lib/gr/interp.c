static
char *syntax[] = 
  {
  /*
    <?xml version='1.0' encoding='ISO-8859-1'?>
    <gr>
   */
    "polyline len='%d' x='%g*' y='%g*'",
    "polymarker len='%d' x='%g*' y='%g*'",
    "text x='%g' y='%g' text='%s'",
    "fillarea len='%d' x='%g*' y='%g*'",
    "cellarray xmin='%g' xmax='%g' ymin='%g' ymax='%g' dimx='%d' dimy='%d' scol='%d' srow='%d' ncol='%d' nrow='%d' color='%d*'",
    "spline len='%d' x='%g*' y='%g*' method='%d'",
    "setlineind index='%d'",
    "setlinetype type='%d'",
    "setlinewidth width='%g'",
    "setlinecolorind color='%d'",
    "setmarkerind index='%d'",
    "setmarkertype type='%d'",
    "setmarkersize size='%g'",
    "setmarkercolorind color='%d'",
    "settextind index='%d'",
    "settextfontprec font='%d' precision='%d'",
    "settextcolorind color='%d'",
    "setcharheight height='%g'",
    "setcharup x='%g' y='%g'",
    "settextpath path='%d'",
    "settextalign halign='%d' valign='%d'",
    "setfillind index='%d'",
    "setfillintstyle intstyle='%d'",
    "setfillstyle style='%d'",
    "setfillcolorind color='%d'",
    "setscale scale='%d'",
    "setwindow xmin='%g' xmax='%g' ymin='%g' ymax='%g'",
    "setviewport xmin='%g' xmax='%g' ymin='%g' ymax='%g'",
    "textex x='%g' y='%g' text='%s'",
    "axes xtick='%g' ytick='%g' xorg='%g' yorg='%g' majorx='%d' majory='%d' ticksize='%g'",
    "grid xtick='%g' ytick='%g' xorg='%g' yorg='%g' majorx='%d' majory='%d'",
    "verrorbars len='%d' x='%g*' y='%g*' e1='%g*' e2='%g*'",
    "herrorbars len='%d' x='%g*' y='%g*' e1='%g*' e2='%g*'",
    "axes3d xtick='%g' ytick='%g' ztick='%g' xorg='%g' yorg='%g' zorg='%g' majorx='%d' majory='%d' majorz='%d' ticksize='%g'",
    "titles3d xtitle='%s' ytitle='%s' ztitle='%s'",
    "surface nx='%d' ny='%d' x='%d*' y='%d*' z='%d**' option='%d'",
    "contour nx='%d' ny='%d' nh='%d' x='%d*' y='%d*' h='%d*' z='%d**' majorh='%d'",
    "setcolormap index='%d'",
    "colormap",
    "drawrect xmin='%g' xmax='%g' ymin='%g' ymax='%g'",
    "fillrect xmin='%g' xmax='%g' ymin='%g' ymax='%g'",
    "drawarc xmin='%g' xmax='%g' ymin='%g' ymax='%g' a1='%d' a2='%d'",
    "fillarc xmin='%g' xmax='%g' ymin='%g' ymax='%g' a1='%d' a2='%d'",
    "setarrowstyle style='%d'",
    "arrow x1='%g' y1='%g' x2='%g' y2='%g'",
    "image xmin='%g' xmax='%g' ymin='%g' ymax='%g' width='%d' height='%d' data='%d*' path='%s'",
    "mathtex x='%g' y='%g' text='%s'"
  /*
    </gr>
   */
  };
