#include <stdio.h>
#include <string.h>

static
char *syntax[] = 
  {
  /*
    <?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>
    <gr>
   */
    "arrow x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\"",
    "axes xtick=\"%g\" ytick=\"%g\" xorg=\"%g\" yorg=\"%g\" majorx=\"%d\" majory=\"%d\" ticksize=\"%g\"",
    "axes3d xtick=\"%g\" ytick=\"%g\" ztick=\"%g\" xorg=\"%g\" yorg=\"%g\" zorg=\"%g\" majorx=\"%d\" majory=\"%d\" majorz=\"%d\" ticksize=\"%g\"",
    "cellarray xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\" dimx=\"%d\" dimy=\"%d\" scol=\"%d\" srow=\"%d\" ncol=\"%d\" nrow=\"%d\" color=\"%d*\"",
    "colormap",
    "contour nx=\"%d\" ny=\"%d\" nh=\"%d\" x=\"%d*\" y=\"%d*\" h=\"%d*\" z=\"%d**\" majorh=\"%d\"",
    "drawarc xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\" a1=\"%d\" a2=\"%d\"",
    "drawrect xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\"",
    "fillarc xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\" a1=\"%d\" a2=\"%d\"",
    "fillarea len=\"%d\" x=\"%g*\" y=\"%g*\"",
    "fillrect xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\"",
    "grid xtick=\"%g\" ytick=\"%g\" xorg=\"%g\" yorg=\"%g\" majorx=\"%d\" majory=\"%d\"",
    "herrorbars len=\"%d\" x=\"%g*\" y=\"%g*\" e1=\"%g*\" e2=\"%g*\"",
    "image xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\" width=\"%d\" height=\"%d\" data=\"%d*\" path=\"%s\"",
    "mathtex x=\"%g\" y=\"%g\" text=\"%s\""
    "polyline len=\"%d\" x=\"%g*\" y=\"%g*\"",
    "polymarker len=\"%d\" x=\"%g*\" y=\"%g*\"",
    "setarrowstyle style=\"%d\"",
    "setcharheight height=\"%g\"",
    "setcharup x=\"%g\" y=\"%g\"",
    "setcolormap index=\"%d\"",
    "setfillcolorind color=\"%d\"",
    "setfillind index=\"%d\"",
    "setfillintstyle intstyle=\"%d\"",
    "setfillstyle style=\"%d\"",
    "setlinecolorind color=\"%d\"",
    "setlineind index=\"%d\"",
    "setlinetype type=\"%d\"",
    "setlinewidth width=\"%g\"",
    "setmarkercolorind color=\"%d\"",
    "setmarkerind index=\"%d\"",
    "setmarkersize size=\"%g\"",
    "setmarkertype type=\"%d\"",
    "setscale scale=\"%d\"",
    "settextalign halign=\"%d\" valign=\"%d\"",
    "settextcolorind color=\"%d\"",
    "settextfontprec font=\"%d\" precision=\"%d\"",
    "settextind index=\"%d\"",
    "settextpath path=\"%d\"",
    "setviewport xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\"",
    "setwindow xmin=\"%g\" xmax=\"%g\" ymin=\"%g\" ymax=\"%g\"",
    "spline len=\"%d\" x=\"%g*\" y=\"%g*\" method=\"%d\"",
    "surface nx=\"%d\" ny=\"%d\" x=\"%d*\" y=\"%d*\" z=\"%d**\" option=\"%d\"",
    "text x=\"%g\" y=\"%g\" text=\"%s\"",
    "textex x=\"%g\" y=\"%g\" text=\"%s\"",
    "titles3d xtitle=\"%s\" ytitle=\"%s\" ztitle=\"%s\"",
    "verrorbars len=\"%d\" x=\"%g*\" y=\"%g*\" e1=\"%g*\" e2=\"%g*\"",
  /*
    </gr>
   */
  };

static
int nel = sizeof(syntax) / sizeof(syntax[0]);

static
int binsearch(char *str[], int nel, char *value)
{
  int position, begin = 0, end = nel - 1, cond = 0;
  char *s;
  size_t n = 0;

  while (begin <= end)
    {
      position = (begin + end) / 2;
      s = str[position];
      n = 0;
      while (s[n] && s[n] != ' ')
        n++;
      if ((cond = strncmp(str[position], value, n)) == 0)
        return position;
      else if (cond < 0)
        begin = position + 1;
      else
        end = position - 1;
    }
  return nel;
}

int main(void)
{
  printf("%d\n", binsearch(syntax, nel, "text"));
  printf("%d\n", binsearch(syntax, nel, "axes"));
  printf("%d\n", binsearch(syntax, nel, "axes3d"));
  printf("%d\n", binsearch(syntax, nel, "polymarker"));
  printf("%d\n", binsearch(syntax, nel, "bla"));

  return 0;
}

