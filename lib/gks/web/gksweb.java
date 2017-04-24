import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.applet.Applet;
import java.awt.Frame;
import java.awt.Image;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.BasicStroke;
import java.awt.Stroke;
import java.awt.Paint;
import java.awt.geom.Arc2D;
import java.awt.TexturePaint;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;

public class gksweb extends Applet
{
  public static final long serialVersionUID = 5L;

  static final int GKS_K_CLIP = 1;

  static final int GKS_K_INTSTYLE_HOLLOW = 0;
  static final int GKS_K_INTSTYLE_SOLID = 1;
  static final int GKS_K_INTSTYLE_PATTERN = 2;
  static final int GKS_K_INTSTYLE_HATCH = 3;

  static final int GKS_K_TEXT_PATH_RIGHT = 0;
  static final int GKS_K_TEXT_PATH_LEFT = 1;
  static final int GKS_K_TEXT_PATH_UP = 2;
  static final int GKS_K_TEXT_PATH_DOWN = 3;

  static final int GKS_K_TEXT_HALIGN_NORMAL = 0;
  static final int GKS_K_TEXT_HALIGN_LEFT = 1;
  static final int GKS_K_TEXT_HALIGN_CENTER = 2;
  static final int GKS_K_TEXT_HALIGN_RIGHT = 3;

  static final int GKS_K_TEXT_VALIGN_NORMAL = 0;
  static final int GKS_K_TEXT_VALIGN_TOP = 1;
  static final int GKS_K_TEXT_VALIGN_CAP = 2;
  static final int GKS_K_TEXT_VALIGN_HALF = 3;
  static final int GKS_K_TEXT_VALIGN_BASE = 4;
  static final int GKS_K_TEXT_VALIGN_BOTTOM = 5;

  static final int GKS_K_TEXT_PRECISION_STRING = 0;
  static final int GKS_K_TEXT_PRECISION_CHAR = 1;
  static final int GKS_K_TEXT_PRECISION_STROKE = 2;

  static final int GRALGKS = 3;
  static final int GLIGKS = 4;
  static final int GKS5 = 5;

  static final double FEPS = 1.0E-09;

  private static class GKSStateList
  {
    int lindex;
    int ltype;
    double lwidth;
    int plcoli;
    int mindex;
    int mtype;
    double mszsc;
    int pmcoli;
    int tindex;
    int txfont;
    int txprec;
    double chxp;
    double chsp;
    int txcoli;
    double chh;
    double[] chup = {0, 1};
    int txp;
    int[] txal = {0, 0};
    int findex;
    int ints;
    int styli;
    int facoli;
    double[][] window = new double[9][4];
    double[][] viewport = new double[9][4];
    int cntnr;
    int clip;
    int opsg;
    double[][] mat = {{1, 0}, {0, 1}, {0, 0}};
    int[] asf = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int wiss, version;
    int fontfile;
    double txslant;
    double shoff[] = {0, 0};
    double blur;
    double alpha;
    double[] a = new double[9];
    double[] b = new double[9];
    double[] c = new double[9];
    double[] d = new double[9];
  }

  private static GKSStateList sl;

  private static class WSStateList
  {
    double a, b, c, d;
    int width = 0, height = 0;
    double[] window = new double[4];
    double[] viewport = new double[4];
    int[][] rect = new int[9][4];
    double cxl, cxr, cyb, cyt;
    double angle;
    double alpha;
    int capheight;
    int family;
  }

  private static WSStateList ws;
  private static Frame frame;
  private static Graphics g;
  private static Image offscreen;
  private static byte[] buf;

  private static class Window
  {
    private static void init(int width, int height)
    {
      frame = new Frame("GKSweb");
      frame.setSize(width, height);
      frame.setBackground(Color.white);
      frame.setVisible(true);
      offscreen = frame.createImage(width, height);
      g = offscreen.getGraphics();
    }
  }

  private static Window win;

  private static int nint(double a)
  {
    return (int) Math.round(a);
  }

  private static int[] NDCtoDC(double xn, double yn)
  {
    int xd, yd;
    xd = nint(ws.a * (xn) + ws.b);
    yd = nint(ws.c * (yn) + ws.d);
    return new int[] {xd, yd};
  }

  private static double[] WCtoNDC(double xw, double yw, int tnr)
  {
    double xn, yn;
    xn = sl.a[tnr] * xw + sl.b[tnr];
    yn = sl.c[tnr] * yw + sl.d[tnr];
    return new double[] {xn, yn};
  }

  private static double[] WCtoNDCrel(double xw, double yw, int tnr)
  {
    double xn, yn;
    xn = sl.a[tnr] * xw;
    yn = sl.c[tnr] * yw;
    return new double[] {xn, yn};
  }

  private static double[] charXform(double xrel, double yrel)
  {
     double xx;
     xx   = (double)(Math.cos(ws.alpha) * (xrel) - Math.sin(ws.alpha) * (yrel));
     yrel = (double)(Math.sin(ws.alpha) * (xrel) + Math.cos(ws.alpha) * (yrel));
     xrel = xx;
     return new double[] {xrel, yrel};
  }

  private static void initNormXform()
  {
    for (int tnr = 0; tnr < 9; tnr++)
      setNormXform(tnr);
  }

  private static void setXform()
  {
    ws.a = (ws.width - 1) / (ws.window[1] - ws.window[0]);
    ws.b = -ws.window[0] * ws.a;
    ws.c = (ws.height - 1) / (ws.window[2] - ws.window[3]);
    ws.d = ws.height - 1 - ws.window[2] * ws.c;
  }

  private static void setNormXform(int tnr)
  {
    int xl, xr, yb, yt;
    int[] xy;

    sl.a[tnr] = (sl.viewport[tnr][1] - sl.viewport[tnr][0]) /
                (sl.window[tnr][1] - sl.window[tnr][0]);
    sl.b[tnr] = sl.viewport[tnr][0] - (sl.window[tnr][0] * sl.a[tnr]);
    sl.c[tnr] = (sl.viewport[tnr][3] - sl.viewport[tnr][2]) /
                (sl.window[tnr][3] - sl.window[tnr][2]);
    sl.d[tnr] = sl.viewport[tnr][2] - (sl.window[tnr][2] * sl.c[tnr]);

    xy = NDCtoDC(sl.viewport[tnr][0], sl.viewport[tnr][3]);
    xl = xy[0];
    yb = xy[1];

    xy = NDCtoDC(sl.viewport[tnr][1], sl.viewport[tnr][2]);
    xr = xy[0];
    yt = xy[1];

    ws.rect[tnr] = new int[] {xl, yb, xr, yt};
  }

  private static double[] segXform(double x, double y)
  {
    double xx;

    xx = x * sl.mat[0][0] + y * sl.mat[0][1] + sl.mat[2][0];
    y  = x * sl.mat[1][0] + y * sl.mat[1][1] + sl.mat[2][1];
    x  = xx;

    double result[] = {x, y};
    return result;
  }

  private static double[] segXformRel(double x, double y)
  {
    double xx;

    xx = x * sl.mat[0][0] + y * sl.mat[0][1];
    y  = x * sl.mat[1][0] + y * sl.mat[1][1];
    x  = xx;

    double result[] = {x,y};
    return result;
  }

  private static void setClipRect(int state)
  {
    int tnr;
    Graphics2D g2 = (Graphics2D) g;

    if (state == 1 && sl.clip == GKS_K_CLIP)
      tnr = sl.cntnr;
    else
      tnr = 0;

    g2.setClip(ws.rect[tnr][0],
               ws.rect[tnr][1],
               ws.rect[tnr][2] - ws.rect[tnr][0],
               ws.rect[tnr][3] - ws.rect[tnr][1]);
  }

  private static void clear()
  {
    if (sl.cntnr != 0)
      setClipRect(0);
    g.clearRect(0, 0, ws.width, ws.height); 
    if (sl.cntnr != 0)
      setClipRect(1);
  }

  private static void setDevXform(double[] window, double[] viewport)
  {
    int tnr;

    tnr = (sl.clip == GKS_K_CLIP) ? sl.cntnr : 0;
    if (sl.clip == GKS_K_CLIP)
      {
        ws.cxl = Math.max(sl.viewport[tnr][0], window[0]);
        ws.cxr = Math.min(sl.viewport[tnr][1], window[1]);
        ws.cyb = Math.max(sl.viewport[tnr][2], window[2]);
        ws.cyt = Math.min(sl.viewport[tnr][3], window[3]);
      }
    else
      {
        ws.cxl = window[0];
        ws.cxr = window[1];
        ws.cyb = window[2];
        ws.cyt = window[3];
      }

    ws.cxl -= FEPS;
    ws.cxr += FEPS;
    ws.cyb -= FEPS;
    ws.cyt += FEPS;
  }

  private static double bx = 1, by = 0, ux = 0, uy = 1;
  private static double sin_f = 0, cos_f = 1;

  private static double[] chrXform(double x, double y, int size)
  {
    double xn, yn;

    xn = x / size;
    yn = y / size;

    xn = cos_f * xn - sin_f * yn;
    yn = cos_f * yn;

    x = (double) (bx * xn + ux * yn);
    y = (double) (by * xn + uy * yn);

    return new double[] {x,y};
  }

  private static void setChrXform()
  {
    int tnr;
    double chux, chuy, scale, chh, chxp, slant;
    double rad;
    double[] xy;

    tnr = sl.cntnr;

    chux = sl.chup[0];
    chuy = sl.chup[1];
    chh = sl.chh;
    chxp = sl.chxp;
    slant = sl.txslant;

    /* scale to normalize the up vector */
    scale = (double) Math.sqrt(chux * chux + chuy * chuy);
    chux /= scale;
    chuy /= scale;

    /* compute character up vector */
    ux = chux * chh;
    uy = chuy * chh;

    /* normalize character up vector */
    xy = WCtoNDCrel((double) ux, (double) uy, tnr);
    ux = xy[0];
    uy = xy[1];

    /* compute character base vector (right angle to up vector) */
    bx = chuy * chh;
    by = -chux * chh;

    /* normalize character base vector */
    xy = WCtoNDCrel((double) bx, (double) by, tnr);
    bx = xy[0];
    by = xy[1];

    bx *= chxp;
    by *= chxp;

    rad = - slant / 180.0 * Math.PI;

    cos_f = (double) Math.cos(rad);
    sin_f = (double) Math.sin(rad);
  }

  static final int[] rgba =
  {
    0xffffff, 0x000000, 0xff0000, 0x00ff00, 0x0000ff, 0x00ffff,
    0xffff00, 0xff00ff, 0x1f1fdf, 0x2f1fdf, 0x3f1fdf, 0x4f1fdf,
    0x5f1fdf, 0x6f1fdf, 0x7f1fdf, 0x8f1fdf, 0x9f1fdf, 0xaf1fdf,
    0xbf1fdf, 0xcf1fdf, 0xdf1fdf, 0xdf1fcf, 0xdf1fbf, 0xdf1faf,
    0xdf1f9f, 0xdf1f8f, 0xdf1f7f, 0xdf1f6f, 0xdf1f5f, 0xdf1f4f,
    0xdf1f3f, 0xdf1f2f, 0xdf1f1f, 0xdf2f1f, 0xdf3f1f, 0xdf4f1f,
    0xdf5f1f, 0xdf6f1f, 0xdf7f1f, 0xdf8f1f, 0xdf9f1f, 0xdfaf1f,
    0xdfbf1f, 0xdfcf1f, 0xdfdf1f, 0xcfdf1f, 0xbfdf1f, 0xafdf1f,
    0x9fdf1f, 0x8fdf1f, 0x7fdf1f, 0x6fdf1f, 0x5fdf1f, 0x4fdf1f,
    0x3fdf1f, 0x2fdf1f, 0x1fdf1f, 0x1fdf2f, 0x1fdf3f, 0x1fdf4f,
    0x1fdf5f, 0x1fdf6f, 0x1fdf7f, 0x1fdf8f, 0x1fdf9f, 0x1fdfaf,
    0x1fdfbf, 0x1fdfcf, 0x1fdfdf, 0x1fcfdf, 0x1fbfdf, 0x1fafdf,
    0x1f9fdf, 0x1f8fdf, 0x1f7fdf, 0x1f6fdf, 0x1f5fdf, 0x1f4fdf,
    0x1f3fdf, 0x1f2fdf, 0x444444, 0x545454, 0x666666, 0x777777,
    0x878787, 0x999999, 0xaaaaaa, 0xbababa, 0xcccccc, 0xdddddd,
    0xededed, 0xffffff, 0x0c3359, 0x113b77, 0x153f94, 0x193fb2,
    0x1d3bd0, 0x2e3de1, 0x4c4ce5, 0x746ae9, 0x9987ed, 0xb8a5f2,
    0xd4c3f6, 0xebe1fa, 0x330c59, 0x4c1177, 0x6a1594, 0x8c19b2,
    0xb21dd0, 0xd22ee1, 0xe54ce5, 0xe96adf, 0xed87dd, 0xf2a5df,
    0xf6c3e5, 0xfae1f0, 0x590c33, 0x77113d, 0x94153f, 0xb2193f,
    0xd01d3b, 0xe12e3d, 0xe54c4c, 0xe9746a, 0xed9987, 0xf2b8a5,
    0xf6d4c3, 0xfaebe1, 0x59330c, 0x774c11, 0x946a15, 0xb28c19,
    0xd0b21d, 0xe1d22e, 0xe5e54c, 0xdfe96a, 0xdded87, 0xdff2a5,
    0xe5f6c3, 0xf0fae1, 0x33590c, 0x3b7711, 0x419415, 0x3fb219,
    0x3bd01d, 0x3de12e, 0x4ce54c, 0x6ae974, 0x87ed99, 0xa5f2b8,
    0xc3f6d4, 0xe1faeb, 0x0c5933, 0x11774c, 0x15946a, 0x19b28c,
    0x1dd0b2, 0x2ee1d2, 0x4ce5e5, 0x6adfe9, 0x87dded, 0xa5dff2,
    0xc3e5f6, 0xe1f0fa, 0x18186f, 0x000080, 0x6494ec, 0x483c8b,
    0x6a59cc, 0x7b67ed, 0x836fff, 0x0000cc, 0x4069e1, 0x1d8fff,
    0x00beff, 0x86cdeb, 0x86cdfa, 0x4581b4, 0xb0c3de, 0xacd8e6,
    0xb0dfe6, 0xafeded, 0x00cdd0, 0x48d0cc, 0x40dfd0, 0xdfffff,
    0x5e9da0, 0x66ccaa, 0x7effd4, 0x006400, 0x546a2e, 0x8fbc8f,
    0x2e8b57, 0x3cb370, 0x20b1aa, 0x98fa98, 0x00ff7e, 0x7cfb00,
    0x7eff00, 0x00fa99, 0xacff2e, 0x32cc32, 0x99cc32, 0x218b21,
    0x6a8e23, 0xbdb66a, 0xf0e68b, 0xede7aa, 0xfafad1, 0xffffdf,
    0xffd600, 0xeddd81, 0xd9a520, 0xb8860a, 0xbc8f8f, 0xcc5c5c,
    0x8b4512, 0xa0522d, 0xcc853f, 0xdeb886, 0xf5f5db, 0xf5deb3,
    0xf4a360, 0xd1b48b, 0xd1691d, 0xb12121, 0xa52929, 0xe89579,
    0xfa8072, 0xffa079, 0xffa500, 0xff8b00, 0xff7e4f, 0xf08080,
    0xff6246, 0xff4500, 0xff69b4, 0xff1393, 0xffbfcb, 0xffb5c1,
    0xda6f93, 0xb02f60, 0xc71585, 0xd0208f, 0xed81ed, 0xdda0dd,
    0xd96fd5, 0xb954d3, 0x9932cc, 0x9400d3, 0x8a2ae2, 0xa020f0,
    0x936fda, 0xd8bed8, 0x3f3f3f, 0x7f7f7f, 0xbfbfbf, 0x0000ff,
    0x0400ff, 0x0900ff, 0x0d00ff, 0x1200ff, 0x1700ff, 0x1b00ff,
    0x2000ff, 0x2500ff, 0x2900ff, 0x2e00ff, 0x3300ff, 0x3700ff,
    0x3c00ff, 0x4000ff, 0x4500ff, 0x4a00ff, 0x4e00ff, 0x5300ff,
    0x5800ff, 0x5c00ff, 0x6100ff, 0x6600ff, 0x6a00ff, 0x6f00ff,
    0x7300ff, 0x7800ff, 0x7d00ff, 0x8100ff, 0x8600ff, 0x8b00ff,
    0x8f00ff, 0x9400ff, 0x9900ff, 0x9d00ff, 0xa200ff, 0xa600ff,
    0xab00ff, 0xb000ff, 0xb400ff, 0xb900ff, 0xbe00ff, 0xc200ff,
    0xc700ff, 0xcc00ff, 0xd000ff, 0xd500ff, 0xd900ff, 0xde00ff,
    0xe300ff, 0xe700ff, 0xec00ff, 0xf100ff, 0xf500ff, 0xfa00ff,
    0xff00ff, 0xff00fa, 0xff00f5, 0xff00f1, 0xff00ec, 0xff00e7,
    0xff00e3, 0xff00de, 0xff00d9, 0xff00d5, 0xff00d0, 0xff00cc,
    0xff00c7, 0xff00c2, 0xff00be, 0xff00b9, 0xff00b4, 0xff00b0,
    0xff00ab, 0xff00a6, 0xff00a2, 0xff009d, 0xff0099, 0xff0094,
    0xff008f, 0xff008b, 0xff0086, 0xff0081, 0xff007d, 0xff0078,
    0xff0073, 0xff006f, 0xff006a, 0xff0066, 0xff0061, 0xff005c,
    0xff0058, 0xff0053, 0xff004e, 0xff004a, 0xff0045, 0xff0040,
    0xff003c, 0xff0037, 0xff0033, 0xff002e, 0xff0029, 0xff0025,
    0xff0020, 0xff001b, 0xff0017, 0xff0012, 0xff000d, 0xff0009,
    0xff0004, 0xff0000, 0xff0400, 0xff0900, 0xff0d00, 0xff1200,
    0xff1700, 0xff1b00, 0xff2000, 0xff2500, 0xff2900, 0xff2e00,
    0xff3300, 0xff3700, 0xff3c00, 0xff4000, 0xff4500, 0xff4a00,
    0xff4e00, 0xff5300, 0xff5800, 0xff5c00, 0xff6100, 0xff6600,
    0xff6a00, 0xff6f00, 0xff7300, 0xff7800, 0xff7d00, 0xff8100,
    0xff8600, 0xff8b00, 0xff8f00, 0xff9400, 0xff9900, 0xff9d00,
    0xffa200, 0xffa600, 0xffab00, 0xffb000, 0xffb400, 0xffb900,
    0xffbe00, 0xffc200, 0xffc700, 0xffcc00, 0xffd000, 0xffd500,
    0xffd900, 0xffde00, 0xffe300, 0xffe700, 0xffec00, 0xfff100,
    0xfff500, 0xfffa00, 0xffff00, 0xfaff00, 0xf5ff00, 0xf1ff00,
    0xecff00, 0xe7ff00, 0xe3ff00, 0xdeff00, 0xd9ff00, 0xd5ff00,
    0xd0ff00, 0xccff00, 0xc7ff00, 0xc2ff00, 0xbeff00, 0xb9ff00,
    0xb4ff00, 0xb0ff00, 0xabff00, 0xa6ff00, 0xa2ff00, 0x9dff00,
    0x99ff00, 0x94ff00, 0x8fff00, 0x8bff00, 0x86ff00, 0x81ff00,
    0x7dff00, 0x78ff00, 0x73ff00, 0x6fff00, 0x6aff00, 0x66ff00,
    0x61ff00, 0x5cff00, 0x58ff00, 0x53ff00, 0x4eff00, 0x4aff00,
    0x45ff00, 0x40ff00, 0x3cff00, 0x37ff00, 0x33ff00, 0x2eff00,
    0x29ff00, 0x25ff00, 0x20ff00, 0x1bff00, 0x17ff00, 0x12ff00,
    0x0dff00, 0x09ff00, 0x04ff00, 0x00ff00, 0x00ff04, 0x00ff09,
    0x00ff0d, 0x00ff12, 0x00ff17, 0x00ff1b, 0x00ff20, 0x00ff25,
    0x00ff29, 0x00ff2e, 0x00ff33, 0x00ff37, 0x00ff3c, 0x00ff40,
    0x00ff45, 0x00ff4a, 0x00ff4e, 0x00ff53, 0x00ff58, 0x00ff5c,
    0x00ff61, 0x00ff66, 0x00ff6a, 0x00ff6f, 0x00ff73, 0x00ff78,
    0x00ff7d, 0x00ff81, 0x00ff86, 0x00ff8b, 0x00ff8f, 0x00ff94,
    0x00ff99, 0x00ff9d, 0x00ffa2, 0x00ffa6, 0x00ffab, 0x00ffb0,
    0x00ffb4, 0x00ffb9, 0x00ffbe, 0x00ffc2, 0x00ffc7, 0x00ffcc,
    0x00ffd0, 0x00ffd5, 0x00ffd9, 0x00ffde, 0x00ffe3, 0x00ffe7,
    0x00ffec, 0x00fff1, 0x00fff5, 0x00fffa, 0x00ffff, 0x00faff,
    0x00f5ff, 0x00f1ff, 0x00ecff, 0x00e7ff, 0x00e3ff, 0x00deff,
    0x00d9ff, 0x00d5ff, 0x00d0ff, 0x00ccff, 0x00c7ff, 0x00c2ff,
    0x00beff, 0x00b9ff, 0x00b4ff, 0x00b0ff, 0x00abff, 0x00a6ff,
    0x00a2ff, 0x009dff, 0x0099ff, 0x0094ff, 0x008fff, 0x008bff,
    0x0086ff, 0x0081ff, 0x007dff, 0x0078ff, 0x0073ff, 0x006fff,
    0x006aff, 0x0066ff, 0x0061ff, 0x005cff, 0x0058ff, 0x0053ff,
    0x004eff, 0x004aff, 0x0045ff, 0x0040ff, 0x003cff, 0x0037ff,
    0x0033ff, 0x002eff, 0x0029ff, 0x0025ff, 0x0020ff, 0x001bff,
    0x0017ff, 0x0012ff, 0x000dff, 0x0009ff, 0x0004ff, 0x0000ff,
    0x2d2d2d, 0x313131, 0x353535, 0x393939, 0x3d3d3d, 0x404040,
    0x444444, 0x484848, 0x4c4c4c, 0x505050, 0x535353, 0x575757,
    0x5b5b5b, 0x5f5f5f, 0x636363, 0x666666, 0x6a6a6a, 0x6e6e6e,
    0x727272, 0x767676, 0x797979, 0x7d7d7d, 0x818181, 0x858585,
    0x898989, 0x8c8c8c, 0x909090, 0x949494, 0x989898, 0x9c9c9c,
    0x9f9f9f, 0xa3a3a3, 0xa7a7a7, 0xababab, 0xafafaf, 0xb2b2b2,
    0xb6b6b6, 0xbababa, 0xbebebe, 0xc2c2c2, 0xc5c5c5, 0xc9c9c9,
    0xcdcdcd, 0xd1d1d1, 0xd5d5d5, 0xd8d8d8, 0xdcdcdc, 0xe0e0e0,
    0xe4e4e4, 0xe8e8e8, 0xebebeb, 0xefefef, 0xf3f3f3, 0xf7f7f7,
    0xfbfbfb, 0xffffff, 0x00002d, 0x000034, 0x00003b, 0x000042,
    0x000049, 0x000050, 0x000057, 0x00005e, 0x000065, 0x00006c,
    0x000073, 0x00007a, 0x000081, 0x000088, 0x00008f, 0x000096,
    0x00009d, 0x0000a4, 0x0000ab, 0x0000b2, 0x0000b9, 0x0000c0,
    0x0000c7, 0x0000ce, 0x0000d5, 0x0000dc, 0x0000e3, 0x0000ea,
    0x0808ff, 0x1111ff, 0x1919ff, 0x2121ff, 0x2a2aff, 0x3333ff,
    0x3b3bff, 0x4444ff, 0x4c4cff, 0x5454ff, 0x5d5dff, 0x6666ff,
    0x6e6eff, 0x7777ff, 0x7f7fff, 0x8787ff, 0x9090ff, 0x9999ff,
    0xa1a1ff, 0xaaaaff, 0xb2b2ff, 0xbabaff, 0xc3c3ff, 0xccccff,
    0xd4d4ff, 0xddddff, 0xe5e5ff, 0xededff, 0x2d002d, 0x340034,
    0x3b003b, 0x420042, 0x490049, 0x500050, 0x570057, 0x5e005e,
    0x650065, 0x6c006c, 0x730073, 0x7a007a, 0x810081, 0x880088,
    0x8f008f, 0x960096, 0x9d009d, 0xa400a4, 0xab00ab, 0xb200b2,
    0xb900b9, 0xc000c0, 0xc700c7, 0xce00ce, 0xd500d5, 0xdc00dc,
    0xe300e3, 0xea00ea, 0xff08ff, 0xff11ff, 0xff19ff, 0xff21ff,
    0xff2aff, 0xff33ff, 0xff3bff, 0xff44ff, 0xff4cff, 0xff54ff,
    0xff5dff, 0xff66ff, 0xff6eff, 0xff77ff, 0xff7fff, 0xff87ff,
    0xff90ff, 0xff99ff, 0xffa1ff, 0xffaaff, 0xffb2ff, 0xffbaff,
    0xffc3ff, 0xffccff, 0xffd4ff, 0xffddff, 0xffe5ff, 0xffedff,
    0x2d0000, 0x340000, 0x3b0000, 0x420000, 0x490000, 0x500000,
    0x570000, 0x5e0000, 0x650000, 0x6c0000, 0x730000, 0x7a0000,
    0x810000, 0x880000, 0x8f0000, 0x960000, 0x9d0000, 0xa40000,
    0xab0000, 0xb20000, 0xb90000, 0xc00000, 0xc70000, 0xce0000,
    0xd50000, 0xdc0000, 0xe30000, 0xea0000, 0xff0808, 0xff1111,
    0xff1919, 0xff2121, 0xff2a2a, 0xff3333, 0xff3b3b, 0xff4444,
    0xff4c4c, 0xff5454, 0xff5d5d, 0xff6666, 0xff6e6e, 0xff7777,
    0xff7f7f, 0xff8787, 0xff9090, 0xff9999, 0xffa1a1, 0xffaaaa,
    0xffb2b2, 0xffbaba, 0xffc3c3, 0xffcccc, 0xffd4d4, 0xffdddd,
    0xffe5e5, 0xffeded, 0x2d2d00, 0x343400, 0x3b3b00, 0x424200,
    0x494900, 0x505000, 0x575700, 0x5e5e00, 0x656500, 0x6c6c00,
    0x737300, 0x7a7a00, 0x818100, 0x888800, 0x8f8f00, 0x969600,
    0x9d9d00, 0xa4a400, 0xabab00, 0xb2b200, 0xb9b900, 0xc0c000,
    0xc7c700, 0xcece00, 0xd5d500, 0xdcdc00, 0xe3e300, 0xeaea00,
    0xffff08, 0xffff11, 0xffff19, 0xffff21, 0xffff2a, 0xffff33,
    0xffff3b, 0xffff44, 0xffff4c, 0xffff54, 0xffff5d, 0xffff66,
    0xffff6e, 0xffff77, 0xffff7f, 0xffff87, 0xffff90, 0xffff99,
    0xffffa1, 0xffffaa, 0xffffb2, 0xffffba, 0xffffc3, 0xffffcc,
    0xffffd4, 0xffffdd, 0xffffe5, 0xffffed, 0x002d00, 0x003400,
    0x003b00, 0x004200, 0x004900, 0x005000, 0x005700, 0x005e00,
    0x006500, 0x006c00, 0x007300, 0x007a00, 0x008100, 0x008800,
    0x008f00, 0x009600, 0x009d00, 0x00a400, 0x00ab00, 0x00b200,
    0x00b900, 0x00c000, 0x00c700, 0x00ce00, 0x00d500, 0x00dc00,
    0x00e300, 0x00ea00, 0x08ff08, 0x11ff11, 0x19ff19, 0x21ff21,
    0x2aff2a, 0x33ff33, 0x3bff3b, 0x44ff44, 0x4cff4c, 0x54ff54,
    0x5dff5d, 0x66ff66, 0x6eff6e, 0x77ff77, 0x7fff7f, 0x87ff87,
    0x90ff90, 0x99ff99, 0xa1ffa1, 0xaaffaa, 0xb2ffb2, 0xbaffba,
    0xc3ffc3, 0xccffcc, 0xd4ffd4, 0xddffdd, 0xe5ffe5, 0xedffed,
    0x002d2d, 0x003434, 0x003b3b, 0x004242, 0x004949, 0x005050,
    0x005757, 0x005e5e, 0x006565, 0x006c6c, 0x007373, 0x007a7a,
    0x008181, 0x008888, 0x008f8f, 0x009696, 0x009d9d, 0x00a4a4,
    0x00abab, 0x00b2b2, 0x00b9b9, 0x00c0c0, 0x00c7c7, 0x00cece,
    0x00d5d5, 0x00dcdc, 0x00e3e3, 0x00eaea, 0x08ffff, 0x11ffff,
    0x19ffff, 0x21ffff, 0x2affff, 0x33ffff, 0x3bffff, 0x44ffff,
    0x4cffff, 0x54ffff, 0x5dffff, 0x66ffff, 0x6effff, 0x77ffff,
    0x7fffff, 0x87ffff, 0x90ffff, 0x99ffff, 0xa1ffff, 0xaaffff,
    0xb2ffff, 0xbaffff, 0xc3ffff, 0xccffff, 0xd4ffff, 0xddffff,
    0xe5ffff, 0xedffff
  };

  private static void setColorRep(int color, double red, double green, double blue)
  {
    if (color >= 0 && color < rgba.length)
      rgba[color] = (nint(  red * 255) << 16) +
                    (nint(green * 255) << 8) +
                    (nint( blue * 255));
  }

  static final int[][] dash_table =
  {
    { 8,  4, 2, 4, 2, 4, 2, 4, 6, 0 },
    { 6,  4, 2, 4, 2, 4, 6, 0, 0, 0 },
    { 4,  4, 2, 4, 6, 0, 0, 0, 0, 0 },
    { 8,  3, 2, 3, 2, 3, 2, 3, 6, 0 },
    { 6,  3, 2, 3, 2, 3, 6, 0, 0, 0 },
    { 4,  3, 2, 3, 6, 0, 0, 0, 0, 0 },
    { 8,  3, 2, 3, 2, 3, 2, 3, 4, 0 },
    { 6,  3, 2, 3, 2, 3, 4, 0, 0, 0 },
    { 4,  3, 2, 3, 4, 0, 0, 0, 0, 0 },
    { 2,  1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  1, 2, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  1, 6, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  1, 8, 0, 0, 0, 0, 0, 0, 0 },
    { 6,  1, 3, 1, 3, 1, 6, 0, 0, 0 },
    { 4,  1, 3, 1, 6, 0, 0, 0, 0, 0 },
    { 8,  6, 2, 1, 2, 1, 2, 1, 2, 0 },
    { 6,  6, 2, 1, 2, 1, 2, 0, 0, 0 },
    { 4,  6, 2, 1, 2, 0, 0, 0, 0, 0 },
    { 4,  9, 3, 5, 3, 0, 0, 0, 0, 0 },
    { 2,  9, 3, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  5, 5, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  5, 3, 0, 0, 0, 0, 0, 0, 0 },
    { 6,  1, 4, 1, 4, 1, 8, 0, 0, 0 },
    { 4,  1, 4, 1, 8, 0, 0, 0, 0, 0 },
    { 2,  1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  8, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 4, 16, 5, 8, 5, 0, 0, 0, 0, 0 },
    { 2, 16, 5, 0, 0, 0, 0, 0, 0, 0 },
    { 8,  8, 4, 1, 4, 1, 4, 1, 4, 0 },
    { 6,  8, 4, 1, 4, 1, 4, 0, 0, 0 },
    { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  8, 5, 0, 0, 0, 0, 0, 0, 0 },
    { 2,  1, 2, 0, 0, 0, 0, 0, 0, 0 },
    { 4,  8, 4, 1, 4, 0, 0, 0, 0, 0 }
  };

  private static float[] getDashList(int ltype, float scale)
  {
    int i, len;
    float value;
    float[] list;

    if (scale < 1)
      scale = 1;

    len = dash_table[ltype + 30][0];
    list = new float[len];
    for (i = 1; i <= len; i++)
      {
        value = dash_table[ltype + 30][i] * scale;
        list[i - 1] = nint(value);
      }

    return list;
  }

  private static void emulPolyline(int n, double[] px, double[] py,
                                   int ltype, double lwidth, int tnr)
  {
    int[] xpoints, ypoints;
    float[] dashList;
    Stroke savedStroke;
    BasicStroke stroke;
    double[] xy;
    int[] ixy;

    xpoints = new int[n];
    ypoints = new int[n];

    dashList = getDashList(ltype, (float) lwidth);

    Graphics2D g2 = (Graphics2D) g;
    savedStroke = g2.getStroke();
    if (ltype != 0 && ltype != 1)
      stroke = new BasicStroke((float) lwidth, BasicStroke.CAP_BUTT,
                               BasicStroke.JOIN_MITER, 1, dashList, 0);
    else
      stroke = new BasicStroke((float) lwidth);
    g2.setStroke(stroke);

    for (int i = 0; i < n; i++)
      {
        xy = WCtoNDC(px[i], py[i], tnr);
        xy = segXform(xy[0], xy[1]);
        ixy = NDCtoDC(xy[0], xy[1]);
        xpoints[i] = ixy[0];
        ypoints[i] = ixy[1];
      }
    g.drawPolyline(xpoints, ypoints, n);

    g2.setStroke(savedStroke);
  }

  private static void setColor(int c)
  {
    Color color = new Color(rgba[c]);
    g.setColor(color);
  }

  private static void polyline(int n, double[] px, double[] py)
  {
    int ln_type, ln_color;
    double ln_width;
    int width;

    ln_type  = sl.asf[0] != 0 ? sl.ltype  : sl.lindex;
    ln_width = sl.asf[1] != 0 ? sl.lwidth : 1;
    ln_color = sl.asf[2] != 0 ? sl.plcoli : 1;

    if (sl.version > 4)
      width = nint(ln_width * (ws.width + ws.height) * 0.001);
    else
      width = nint(ln_width);

    if (width < 1)
      width = 1;
    if (ln_color < 0 || ln_color >= rgba.length)
      ln_color = 1;

    setColor(ln_color);
    setDevXform(ws.window, ws.viewport);

    emulPolyline(n, px, py, ln_type, ln_width, sl.cntnr);
  }

  static final int[][] marker =
  {
    /* omark */
    { 5, 9, -400, 700, 400, 700, 700, 400, 700, -400, 400, -700, -400, -700,
      -700, -400, -700, 400, -400, 700,
      3, 9, -400, 700, 400, 700, 700, 400, 700, -400, 400, -700, -400, -700,
      -700, -400, -700, 400, -400, 700, 0 },
    /* hline */
    { 2, -1000, 0, 1000, 0, 0 },
    /* vline */
    { 2, 0, -1000, 0, 1000, 0 },
    /* star8 */
    { 4, 16,  0, -1000, 153, -370, 707, -707, 370, -153, 1000, 0, 370, 153,
      707, 707, 153, 370, 0, 1000, -153, 370, -707, 707, -370, 153, -1000, 0,
      -370, -153, -707, -707, -153, -370, 0 },
    /* star7 */
    { 4, 14,  0, 1000, 174, 360, 782, 623, 390, 89, 975, -223, 313, -249,
      434, -901, 0, -400, -434, -901, -313, -249, -975, -223, -390, 89,
      -782, 623, -174, 360, 0 },
    /* star6 */
    { 4, 12,  0, -1000, 200, -346, 866, -500, 400, 0, 866, 500, 200, 346, 0,
      1000, -200, 346, -866, 500, -400, 0, -866, -500, -200, -346, 0 },
    /* star5 */
    { 4, 10,  0, 1000, 235, 324, 951, 309, 380, -124, 588, -809, 0, -400,
      -588, -809, -380, -124, -951, 309, -235, 324, 0 },
    /* star4 */
    { 4, 8,  0, -1000, 283, -283, 1000, 0, 283, 283, 0, 1000, -283, 283,
      -1000, 0, -283, -283, 0 },
    /* octagon */
    { 4, 8,  0, -1000, 707, -707, 1000, 0, 707, 707, 0, 1000, -707, 707,
      -1000, 0, -707, -707, 0 },
    /* heptagon */
    { 4, 7,  0, 1000, 782, 623, 975, -223, 434, -901, -434, -901,
      -975, -223, -782, 623, 0 },
    /* hexagon */
    { 4, 6,  0, -1000, 866, -500, 866, 500, 0, 1000, -866, 500, -866, -500,
      0 },
    /* pentagon */
    { 4, 5,  0, 1000, 951, 309, 588, -809, -588, -809, -951, 309, 0 },
    /* omark */
    { 5, 9, -400, 700, 400, 700, 700, 400, 700, -400, 400, -700, -400, -700,
      -700, -400, -700, 400, -400, 700,
      3, 9, -400, 700, 400, 700, 700, 400, 700, -400, 400, -700, -400, -700,
      -700, -400, -700, 400, -400, 700, 0 },
    /* hollow plus */
    { 5, 13, -200, 800, 200, 800, 200, 200, 800, 200, 800, -200, 200, -200,
      200, -800, -200, -800, -200, -200, -800, -200, -800, 200, -200, 200,
      -200, 800,
      3, 13, -200, 800, 200, 800, 200, 200, 800, 200, 800, -200, 200, -200,
      200, -800, -200, -800, -200, -200, -800, -200, -800, 200, -200, 200,
      -200, 800, 0 },
    /* solid triangle right */
    { 4, 4, -800, 0, 400, 700, 400, -700, -800, 0, 0 },
    /* solid triangle left */
    { 4, 4, 800, 0, -400, -700, -400, 700, 800, 0, 0 },
    /* triangle up down */
    { 5, 4, 0, 800, 700, -400, -700, -400, 0, 800,
      5, 4, 0, -800, -700, 400, 700, 400, 0, -800,
      3, 4, 0, 800, 700, -400, -700, -400, 0, 800,
      3, 4, 0, -800, -700, 400, 700, 400, 0, -800, 0 },
    /* solid star */
    { 4, 11, 0, 900, 200, 200, 900, 300, 300, -100, 600, -800, 0, -300,
      -600, -800, -300, -100, -900, 300, -200, 200, 0, 900, 0 },
    /* hollow star */
    { 5, 11, 0, 900, 200, 200, 900, 300, 300, -100, 600, -800, 0, -300,
      -600, -800, -300, -100, -900, 300, -200, 200, 0, 900,
      3, 11, 0, 900, 200, 200, 900, 300, 300, -100, 600, -800, 0, -300,
      -600, -800, -300, -100, -900, 300, -200, 200, 0, 900, 0 },
    /* solid diamond */
    { 4, 5, 0, 1000, 1000, 0, 0, -1000, -1000, 0, 0, 1000, 0},
    /* hollow diamond */
    { 5, 5, 0, 1000, 1000, 0, 0, -1000, -1000, 0, 0, 1000,
      3, 5, 0, 1000, 1000, 0, 0, -1000, -1000, 0, 0, 1000, 0 },
    /* solid hourglass */
    { 4, 5, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000, 1000, 0 },
    /* hollow hourglass */
    { 5, 5, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000, 1000,
      3, 5, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000, 1000, 0 },
    /* solid bowtie */
    { 4, 5, 1000, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000, 0 },
    /* hollow bowtie */
    { 5, 5, 1000, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000,
      3, 5, 1000, 1000, 1000, -1000, -1000, 1000, -1000, -1000, 1000, 1000, 0 },
    /* solid square */
    { 4, 5, 1000, 1000, 1000, -1000, -1000, -1000, -1000, 1000, 1000, 1000, 0 },
    /* hollow square */
    { 5, 5, 1000, 1000, 1000, -1000, -1000, -1000, -1000, 1000, 1000, 1000,
      3, 5, 1000, 1000, 1000, -1000, -1000, -1000, -1000, 1000, 1000, 1000, 0 },
    /* solid triangle down */
    { 4, 4, -1000, 1000, 1000, 1000, 0, -1000, -1000, 1000, 0 },
    /* hollow triangle down */
    { 5, 4, -1000, 1000, 1000, 1000, 0, -1000, -1000, 1000,
      3, 4, -1000, 1000, 1000, 1000, 0, -1000, -1000, 1000, 0 },
    /* solid triangle up */
    { 4, 4, 0, 1000, 1000, -1000, -1000, -1000, 0, 1000, 0 },
    /* hollow triangle up */
    { 5, 4, 0, 1000, 1000, -1000, -1000, -1000, 0, 1000,
      3, 4, 0, 1000, 1000, -1000, -1000, -1000, 0, 1000, 0 },
    /* solid circle */
    { 7, 0, 360, 0 },
    /* not used */
    { 0 },
    /* dot */
    { 1, 0 },
    /* plus */
    { 2, 0, 0, 0, 1000, 2, 0, 0, 1000, 0, 2, 0, 0, 0, -1000,
      2, 0, 0, -1000, 0, 0 },
    /* asterisk */
    { 2, 0, 0, 0, 1000, 2, 0, 0, 1000, 300,
      2, 0, 0, 600, -1000, 2, 0, 0, -600, -1000,
      2, 0, 0, -1000, 300, 0 },
    /* circle */
    { 8, 6, 0 },
    /* diagonal cross */
    { 2, 0, 0, 1000, 1000, 2, 0, 0, 1000, -1000,
      2, 0, 0, -1000, -1000, 2, 0, 0, -1000, 1000, 0 }
  };

  private static void drawMarker(double[] ndc, int mtype, double mscale,
                                 int mcolor)
  {
    int r, d, i;
    int pc, op;
    double scale, xr, yr;
    double[] xy;
    int[] ixy;
    int[][] p;

    if (sl.version > 4)
      mscale *= (ws.width + ws.height) * 0.001;

    r = (int) (3 * mscale);
    d = 2 * r;
    scale = 0.01 * mscale / 3.0;

    xr = r;
    yr = 0;
    xy = segXformRel(xr,yr);
    xr = xy[0];
    yr = xy[1];
    r = nint(Math.sqrt(xr * xr + yr * yr));

    ixy = NDCtoDC(ndc[0], ndc[1]);

    pc = 0;
    mtype = (d > 1) ? mtype + 32 : 33;

    do
      {
        op = marker[mtype][pc];
        switch (op)
          {
          case 1:		/* point */
            g.drawLine(ixy[0], ixy[1], ixy[0], ixy[1]);
            break;

          case 2:		/* line */
            p = new int[2][2];
            for (i = 0; i < 2; i++)
              {
                xr = scale * marker[mtype][pc + 2 * i + 1];
                yr = scale * marker[mtype][pc + 2 * i + 2];
                xy = segXformRel(xr, yr);
                p[0][i] = nint(ixy[0] - xy[0]);
                p[1][i] = nint(ixy[1] - xy[1]);
              }
            g.drawPolyline(p[0], p[1], 2);
            pc += 4;
            break;

          case 3:		/* polygon */
            p = new int[2][marker[mtype][pc + 1]];
            for (i = 0; i < marker[mtype][pc + 1]; i++)
              {
                xr = scale * marker[mtype][pc + 2 + 2 * i];
                yr = scale * marker[mtype][pc + 3 + 2 * i];
                xy = segXformRel(xr, yr);
                p[0][i] = nint(ixy[0] - xy[0]);
                p[1][i] = nint(ixy[1] - xy[1]);
              }
            g.drawPolyline(p[0], p[1], marker[mtype][pc + 1]);
            pc += (1 + 2 * marker[mtype][pc + 1]);
            break;

          case 4:		/* filled polygon */
          case 5:               /* hollow polygon */
            p = new int[2][marker[mtype][pc + 1]];
            for (i = 0; i < marker[mtype][pc + 1]; i++)
              {
                xr = scale * marker[mtype][pc + 2 + 2 * i];
                yr = -scale * marker[mtype][pc + 3 + 2 * i];
                xy = segXformRel(xr, yr);
                p[0][i] = nint(ixy[0] - xy[0]);
                p[1][i] = nint(ixy[1] + xy[1]);
              }
            g.drawPolyline(p[0], p[1], marker[mtype][pc + 1]);
            if (op == 5)
              setColor(0);
            g.fillPolygon(p[0], p[1], marker[mtype][pc + 1]);
            pc += (1 + 2 * marker[mtype][pc + 1]);
            if (op == 5)
              setColor(mcolor);
            break;

          case 6:		/* arc */
            g.drawArc(ixy[0] - r, ixy[1] - r, d, d,
                      marker[mtype][pc + 1] * 16, marker[mtype][pc + 2] * 16);
            pc += 2;
            break;

          case 7:		/* filled arc */
          case 8:		/* hollow arc */
            Arc2D chord = new Arc2D.Float(ixy[0] - r, ixy[1] - r, d, d,
                                          marker[mtype][pc + 1] * 16,
                                          marker[mtype][pc + 2] * 16,
                                          Arc2D.CHORD);
            Graphics2D g2 = (Graphics2D) g;
            g2.draw(chord);
            if (op == 8)
              setColor(0);
            g2.fill(chord);
            pc += 2;
            if (op == 8)
              setColor(mcolor);
            break;
          }
        pc++;
      }
    while (op != 0);
  }

  private static void markerRoutine(int n, double[] px, double[] py,
                                    int mtype, double mscale, int mcolor)
  {
    double[] clrt = sl.viewport[sl.cntnr];
    int i, draw;
    double[] ndc;

    for (i = 0; i < n; i++)
      {
        ndc = WCtoNDC(px[i], py[i], sl.cntnr);
        ndc = segXform(ndc[0], ndc[1]);

        if (sl.clip == GKS_K_CLIP)
          {
            if (ndc[0] >= clrt[0] && ndc[0] <= clrt[1] &&
                ndc[1] >= clrt[2] && ndc[1] <= clrt[3])
              draw = 1;
            else
              draw = 0;
          }
        else
          draw = 1;

        if (draw != 0)
          drawMarker(ndc, mtype, mscale, mcolor);
      }
  }

  private static void polymarker(int n,double[] px, double[] py)
  {
    int mk_type, mk_color, ln_width;
    double mk_size;

    mk_type = sl.asf[3] != 0 ? sl.mtype : sl.mindex;
    mk_size = sl.asf[4] != 0 ? sl.mszsc : 1;
    mk_color = sl.asf[5] != 0 ? sl.pmcoli : 1;

    if (sl.version > 4)
      {
        ln_width = nint((ws.width + ws.height) * 0.001);
        if (ln_width < 1)
          ln_width = 1;
      }
    else
      ln_width = 1;

    setColor(mk_color);
    markerRoutine(n, px, py, mk_type, mk_size, mk_color);
  }

  static final int[][] pattern =
  {
    {  4,   0,   0,   0,   0,   0,   0,   0,   0 },
    {  4, 255, 255, 187, 255,   0,   0,   0,   0 },
    {  4, 238, 255, 187, 255,   0,   0,   0,   0 },
    {  4, 187, 238, 187, 255,   0,   0,   0,   0 },
    {  4, 119, 221, 119, 221,   0,   0,   0,   0 },
    {  4, 119, 170, 119, 221,   0,   0,   0,   0 },
    {  4,  85, 170,  85, 187,   0,   0,   0,   0 },
    {  4,  85, 170,  85, 170,   0,   0,   0,   0 },
    {  4, 136,  85, 136,  34,   0,   0,   0,   0 },
    {  4, 136,  34, 136,  34,   0,   0,   0,   0 },
    {  4,   0,  68,   0,  17,   0,   0,   0,   0 },
    {  4,   0,  34,   0,   0,   0,   0,   0,   0 },
    {  8,   0,   0,   0,   0,   0,   0,   0,   0 },
    {  8, 128,   0,   8,   0, 128,   0,   8,   0 },
    {  8,  34, 136,  34, 136,  34, 136,  34, 136 },
    {  8,  85, 170,  85, 170,  85, 170,  85, 170 },
    {  8, 170,   0, 170,   0, 170,   0, 170,   0 },
    {  8,  85,  85,  85,  85,  85,  85,  85,  85 },
    {  8,  17,  34,  68, 136,  17,  34,  68, 136 },
    {  8, 119, 119, 119, 119, 119, 119, 119, 119 },
    {  8,  78, 207, 252, 228,  39,  63, 243, 114 },
    {  8, 127, 239, 253, 223, 254, 247, 191, 251 },
    {  8,   0, 119, 119, 119,   0, 119, 119, 119 },
    {  8,   0, 127, 127, 127,   0, 247, 247, 247 },
    {  8, 127, 255, 255, 255, 255, 255, 255, 255 },
    {  8, 127, 191, 223, 255, 253, 251, 247, 255 },
    {  8, 125, 187, 198, 187, 125, 254, 254, 254 },
    {  8,   7, 139, 221, 184, 112, 232, 221, 142 },
    {  8, 170,  95, 191, 191, 170, 245, 251, 251 },
    {  8, 223, 175, 119, 119, 119, 119, 250, 253 },
    {  8,  64, 255,  64,  64,  79,  79,  79,  79 },
    {  8, 127, 255, 247, 255, 127, 255, 247, 255 },
    {  8, 119, 255, 221, 255, 119, 255, 221, 255 },
    {  8, 119, 221, 119, 221, 119, 221, 119, 221 },
    {  8,  85, 255,  85, 255,  85, 255,  85, 255 },
    {  8,   0, 255,   0, 255,   0, 255,   0, 255 },
    {  8, 238, 221, 187, 119, 238, 221, 187, 119 },
    {  8,   0, 255, 255, 255,   0, 255, 255, 255 },
    {  8, 254, 253, 251, 247, 239, 223, 191, 127 },
    {  8,  85, 255, 127, 255, 119, 255, 127, 255 },
    {  8,   0, 127, 127, 127, 127, 127, 127, 127 },
    {  8, 247, 227, 221,  62, 127, 254, 253, 251 },
    {  8, 119, 235, 221, 190, 119, 255,  85, 255 },
    {  8, 191,  95, 255, 255, 251, 245, 255, 255 },
    {  8, 252, 123, 183, 207, 243, 253, 254, 254 },
    {  8, 127, 127, 190, 193, 247, 247, 235,  28 },
    {  8, 239, 223, 171,  85,   0, 253, 251, 247 },
    {  8, 136, 118, 112, 112, 136, 103,   7,   7 },
    {  8, 255, 247, 235, 213, 170, 213, 235, 247 },
    {  8, 255, 247, 235, 213, 170, 213, 235, 247 },
    {  8, 127, 255, 255, 255, 255, 255, 255, 255 },
    {  8, 127, 255, 255, 255, 247, 255, 255, 255 },
    {  8, 119, 255, 255, 255, 247, 255, 255, 255 },
    {  8, 119, 255, 255, 255, 119, 255, 255, 255 },
    {  8, 119, 255, 223, 255, 119, 255, 255, 255 },
    {  8, 119, 255, 223, 255, 119, 255, 253, 255 },
    {  8, 119, 255, 221, 255, 119, 255, 253, 255 },
    {  8, 119, 255, 221, 255, 119, 255, 221, 255 },
    {  8,  87, 255, 221, 255, 119, 255, 221, 255 },
    {  8,  87, 255, 221, 255, 117, 255, 221, 255 },
    {  8,  85, 255, 221, 255, 117, 255, 221, 255 },
    {  8,  85, 255, 221, 255,  85, 255, 221, 255 },
    {  8,  85, 255,  93, 255,  85, 255, 221, 255 },
    {  8,  85, 255,  93, 255,  85, 255, 213, 255 },
    {  8,  85, 255,  85, 255,  85, 255, 213, 255 },
    {  8,  85, 255,  85, 255,  85, 255,  85, 255 },
    {  8,  85, 191,  85, 255,  85, 255,  85, 255 },
    {  8,  85, 191,  85, 255,  85, 251,  85, 255 },
    {  8,  85, 187,  85, 255,  85, 251,  85, 255 },
    {  8,  85, 187,  85, 255,  85, 187,  85, 255 },
    {  8,  85, 187,  85, 239,  85, 187,  85, 255 },
    {  8,  85, 187,  85, 239,  85, 187,  85, 254 },
    {  8,  85, 187,  85, 238,  85, 187,  85, 254 },
    {  8,  85, 187,  85, 238,  85, 187,  85, 238 },
    {  8,  85, 171,  85, 238,  85, 187,  85, 238 },
    {  8,  85, 171,  85, 238,  85, 186,  85, 238 },
    {  8,  85, 170,  85, 238,  85, 186,  85, 238 },
    {  8,  85, 170,  85, 238,  85, 170,  85, 238 },
    {  8,  85, 170,  85, 174,  85, 170,  85, 238 },
    {  8,  85, 170,  85, 174,  85, 170,  85, 234 },
    {  8,  85, 170,  85, 170,  85, 170,  85, 234 },
    {  8,  85, 170,  85, 170,  85, 170,  85, 170 },
    {  8,  21, 170,  85, 170,  85, 170,  85, 170 },
    {  8,  21, 170,  85, 170,  81, 170,  85, 170 },
    {  8,  17, 170,  85, 170,  81, 170,  85, 170 },
    {  8,  17, 170,  85, 170,  17, 170,  85, 170 },
    {  8,  17, 170,  69, 170,  17, 170,  85, 170 },
    {  8,  17, 170,  69, 170,  17, 170,  84, 170 },
    {  8,  17, 170,  68, 170,  17, 170,  84, 170 },
    {  8,  17, 170,  68, 170,  17, 170,  68, 170 },
    {  8,   1, 170,  68, 170,  17, 170,  68, 170 },
    {  8,   1, 170,  68, 170,  16, 170,  68, 170 },
    {  8,   0, 170,  68, 170,  16, 170,  68, 170 },
    {  8,   0, 170,  68, 170,   0, 170,  68, 170 },
    {  8,   0, 170,   4, 170,   0, 170,  68, 170 },
    {  8,   0, 170,   4, 170,   0, 170,  64, 170 },
    {  8,   0, 170,   0, 170,   0, 170,  64, 170 },
    {  8,   0, 170,   0, 170,   0, 170,   0, 170 },
    {  8,   0,  42,   0, 170,   0, 170,   0, 170 },
    {  8,   0,  42,   0, 170,   0, 162,   0, 170 },
    {  8,   0,  34,   0, 170,   0, 162,   0, 170 },
    {  8,   0,  34,   0, 170,   0,  34,   0, 170 },
    {  8,   0,  34,   0, 138,   0,  34,   0, 170 },
    {  8,   0,  34,   0, 138,   0,  34,   0, 168 },
    {  8,   0,  34,   0, 136,   0,  34,   0, 168 },
    {  8,   0,  34,   0, 136,   0,  34,   0, 136 },
    {  8,   0,   2,   0, 136,   0,  34,   0, 136 },
    {  8,   0,   2,   0, 136,   0,  32,   0, 136 },
    {  8,   0,   0,   0, 136,   0,  32,   0, 136 },
    {  8, 119, 119, 119, 119, 119, 119, 119, 119 },
    {  8,   0, 255, 255, 255,   0, 255, 255, 255 },
    {  8, 119, 187, 221, 238, 119, 187, 221, 238 },
    {  8, 238, 221, 187, 119, 238, 221, 187, 119 },
    {  8,   0, 119, 119, 119,   0, 119, 119, 119 },
    {  8, 126, 189, 219, 231, 231, 219, 189, 126 },
    {  8, 247, 247, 247, 247, 247, 247, 247, 247 },
    {  8, 255, 255, 255, 255,   0, 255, 255, 255 },
    {  8, 127, 191, 223, 239, 247, 251, 253, 254 },
    {  8, 254, 253, 251, 247, 239, 223, 191, 127 },
    {  8, 247, 247, 247, 247,   0, 247, 247, 247 }
  };
   
  private static float[][] getPattern(int style)
  {
    int j, k, l, numseg, len, bit, onoff;
    float [][] brush;
    float [] seglen = new float[32];

    len = pattern[style][0];
    brush = new float[len][];

    for (j = 0; j < len; j++)
      {
        l = numseg = 0;
        onoff = pattern[style][j + 1] & 0x01;
        if (onoff == 1)
          seglen[numseg++] = 0.0;
        for (k = 0; k < len; k++)
        {
          bit = (pattern[style][j + 1] >> k) & 0x01;
          if (bit == onoff)
            l += 1;
          else
          {
            seglen[numseg++] = (float) l;
            onoff = 1 - onoff;
            l = 1;
          }        
        }
        if (l != 0)
          seglen[numseg++] = (float) l;

        brush[j] = new float[numseg];
        for (k = 0; k < numseg; k++)
          brush[j][k] = seglen[k];
      }

    return brush;
  }

  private static void fillRoutine(int n, double[] px, double[] py,
                                  int tnr, int fl_inter, int fl_style)
  {
    int i;
    double[] xy;
    int[] ixy;
    int[][] p = new int[2][n];
    float[][] dashList;
    BufferedImage bi;
    int w, h;
    Graphics2D texture;
    BasicStroke stroke;

    for (i = 0; i < n; i++)
      {
        xy = WCtoNDC(px[i], py[i], tnr);
        xy = segXform(xy[0], xy[1]);
        ixy = NDCtoDC(xy[0], xy[1]);
        p[0][i] = ixy[0];
        p[1][i] = ixy[1];
      }

    switch (fl_inter)
      {
      case GKS_K_INTSTYLE_SOLID:
        g.fillPolygon(p[0], p[1], n);
        g.drawPolyline(p[0], p[1], n);
        break;

      case GKS_K_INTSTYLE_HATCH:
      case GKS_K_INTSTYLE_PATTERN:
        Paint paint;

        Graphics2D g2 = (Graphics2D) g;
        Paint savedPaint = g2.getPaint();

        dashList = getPattern(fl_style);
        w = h = dashList.length;
        bi = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
        texture = bi.createGraphics();
        texture.setColor(g.getColor());

        for (h = 0; h < w; h++)
          {
            stroke = new BasicStroke(1, BasicStroke.CAP_BUTT,
                                     BasicStroke.JOIN_MITER, 1, dashList[h], 0);
            texture.setStroke(stroke);
            texture.drawLine(0, h, w, h);
          }

        paint = new TexturePaint(bi, new Rectangle2D.Double(-1, -1,
                                 bi.getWidth(), bi.getWidth()));
        g2.setPaint(paint);

        g2.fillPolygon(p[0], p[1], n);

        g2.setPaint(savedPaint);
        break;
      }
  }

  private static void fillareaLineRoutine(int n, double[] px, double[] py,
                                          int linetype, int tnr)
  {
    int i, x0, y0, xi, yi, xim1, yim1;
    int[] ixy;
    double[] xy;
    int[][] p;
    int npoints;

    xy = WCtoNDC(px[0], py[0], tnr);
    xy = segXform(xy[0], xy[1]);
    ixy = NDCtoDC(xy[0], xy[1]);
    x0 = ixy[0];
    y0 = ixy[1];

    p = new int[2][n + 1];

    npoints = 0;
    p[0][npoints] = x0;
    p[1][npoints] = y0;
    npoints++;

    xim1 = x0;
    yim1 = y0;
    for (i = 1; i < n; i++)
      {
        xy = WCtoNDC(px[i], py[i], tnr);
        xy = segXform(xy[0], xy[1]);
        ixy = NDCtoDC(xy[0], xy[1]);
        xi = ixy[0];
        yi = ixy[1];
        if (i == 1 || xi != xim1 || yi != yim1)
          {
            p[0][npoints] = xi;
            p[1][npoints] = yi;
            npoints++;
            xim1 = xi;
            yim1 = yi;
          }
      }
    if (linetype == 0)
      {
        p[0][npoints] = x0;
        p[1][npoints] = y0;
        npoints++;
      }
    g.drawPolyline(p[0], p[1], npoints);
  }

  static final int[] predef_ints = { 0, 1, 3, 3, 3 };
  static final int[] predef_styli = { 1, 1, 1, 2, 3 };

  private static void fillarea(int n, double[] px, double[] py)
  {
    int fl_inter, fl_style, fl_color, ln_width;
    Stroke savedStroke;

    fl_inter = sl.asf[10] != 0 ? sl.ints : predef_ints[sl.findex - 1];
    fl_style = sl.asf[11] != 0 ? sl.styli : predef_styli[sl.findex - 1];
    fl_color = sl.asf[12] != 0 ? sl.facoli : 1;

    if (sl.version > 4)
      {
        ln_width = nint((ws.width + ws.height) * 0.001);
        if (ln_width < 1)
          ln_width = 1;
      }
    else
      ln_width = 1;

    if (ln_width < 1)
      ln_width = 1;

    if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
      {
        Graphics2D g2 = (Graphics2D) g;
        savedStroke = g2.getStroke();
        g2.setStroke(new BasicStroke(ln_width));

        setColor(fl_color);
        fillareaLineRoutine(n, px, py, 0, sl.cntnr);

        g2.setStroke(savedStroke);
      }
    else if (fl_inter == GKS_K_INTSTYLE_SOLID)
      {
        setColor(fl_color);
        fillRoutine(n, px, py, sl.cntnr, fl_inter, 0);
      }
    else if (fl_inter == GKS_K_INTSTYLE_PATTERN ||
             fl_inter == GKS_K_INTSTYLE_HATCH)
      {
        if (fl_inter == GKS_K_INTSTYLE_HATCH)
          fl_style += 108;
        if (fl_style >= 120)
          fl_style = 1;

        fillRoutine(n, px, py, sl.cntnr, fl_inter, fl_style);
      }
  } 

  static final int[] symbol2utf = {
       0,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    18,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,  8704,    35,  8707,    37,    38,  8715,
      40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,
    8773,   913,   914,   935,   916,   917,   934,   915,
     919,   921,   977,   922,   923,   924,   925,   927,
     928,   920,   929,   931,   932,   933,   962,   937,
     926,   936,   918,    91,  8756,    93,  8869,    95,
    8254,   945,   946,   967,   948,   949,   966,   947,
     951,   953,   981,   954,   955,   956,   957,   959,
     960,   952,   961,   963,   964,   965,   982,   969,
     958,   968,   950,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,
     160,   978,  8242,  8804,  8260,  8734,   402,  9827,
    9830,  9829,  9824,  8596,  8592,  8593,  8594,  8595,
     176,   177,  8243,  8805,   215,  8733,  8706,  8226,
     247,  8800,  8801,  8776,  8230,  9116,  9135,  8629,
    8501,  8465,  8476,  8472,  8855,  8853,  8709,  8745,
    8746,  8835,  8839,  8836,  8834,  8838,  8712,  8713,
    8736,  8711,   174,   169,  8482,  8719,  8730,   183,
     172,  8743,  8744,  8660,  8656,  8657,  8658,  8659,
    9674, 12296,   174,   169,  8482,  8721,  9115,  9116,
    9117,  9121,  9116,  9123,  9127,  9128,  9129,  9116,
     240, 12297,  8747,  9127,  9116,  9133,  9131,  9130,
    9120,  9124,  9130,  9126,  9131,  9132,  9133,   255
  };

  private static double[] xfac = {0.0, 0.0, -0.5, -1.0};
  private static double[] yfac = {0.0, -1.2, -1.0, -0.5, 0, 0.2};

  private static void textRoutine(double x, double y, String text)
  {
    int i, ch, xstart, ystart, width;
    double xrel, yrel, ax, ay;
    int[] ixy;
    double[] xy;
    String t = "";

    Graphics2D g2 = (Graphics2D) g;
    FontMetrics fm = g2.getFontMetrics(g2.getFont());

    for (i = 0; i < text.length(); ++i)
      {
        ch = text.charAt(i);
        if (ch < 0)
          ch += 256;
        if (ws.family == 3)
          ch = symbol2utf[ch];
        t += String.valueOf((char) ch);
      }
    text = t;

    ixy = NDCtoDC(x, y);
    xstart = ixy[0];
    ystart = ixy[1];

    width = fm.stringWidth(text);
    xrel = width * xfac[sl.txal[0]];
    yrel = ws.capheight * yfac[sl.txal[1]];

    xy = charXform(xrel, yrel);
    ax = xy[0];
    ay = xy[1];
    xstart += (int)ax;
    ystart -= (int)ay;

    if (Math.abs(ws.angle) > FEPS)
      {
        FontRenderContext frc = g2.getFontRenderContext();
        GlyphVector gv = g2.getFont().createGlyphVector(frc, text);

        g2.translate(xstart, ystart);
        g2.rotate(Math.toRadians(360 - ws.angle));

        g2.drawGlyphVector(gv, 0, 0);

        g2.rotate(Math.toRadians(ws.angle - 360));
        g2.translate(-xstart, -ystart);
      }
    else
      g.drawString(text, xstart, ystart);
  }

  static final int map[] = {
    22,  9,  5, 14, 18, 26, 13,  1,
    24, 11,  7, 16, 20, 28, 13,  3,
    23, 10,  6, 15, 19, 27, 13,  2,
    25, 12,  8, 17, 21, 29, 13,  4
  };

  static final double[] capheights = {
    0.662, 0.660, 0.681, 0.662,
    0.729, 0.729, 0.729, 0.729,
    0.583, 0.583, 0.633, 0.633,
    0.667,
    0.681, 0.681, 0.681, 0.681,
    0.722, 0.722, 0.722, 0.722,
    0.739, 0.739, 0.739, 0.739,
    0.694, 0.693, 0.683, 0.683
  };

  static final String[] fonts = {
    "Times New Roman", "Arial", "Courier", "OpenSymbol",
    "Bookman Old Style", "Century Schoolbook", "Century Gothic", "Book Antiqua"
  };

  private static void setFont(int font)
  {
    double scale;
    double ux, uy;
    double[] xy;
    int fontNum, size;
    int style = Font.PLAIN;
    double width, height, capheight;

    font = Math.abs(font);
    if (font >= 101 && font <= 129)
      font -= 100;
    else if (font >= 1 && font <= 32)
      font = map[font - 1];
    else
      font = 9;

    xy = WCtoNDCrel(sl.chup[0], sl.chup[1], sl.cntnr);
    xy = segXformRel(xy[0], xy[1]);

    ws.alpha = -Math.atan2(xy[0], xy[1]);
    ws.angle = ws.alpha * 180 / Math.PI;
    if (ws.angle < 0) ws.angle += 360;

    scale = Math.sqrt(sl.chup[0] * sl.chup[0] + sl.chup[1] * sl.chup[1]);
    ux = (double) (sl.chup[0] / scale * sl.chh);
    uy = (double) (sl.chup[1] / scale * sl.chh);
    xy = WCtoNDCrel(ux, uy, sl.cntnr);
    ux = xy[0];
    uy = xy[1];

    width = 0;
    height = (double) Math.sqrt(ux * ux + uy * uy);
    xy = segXformRel(width, height);
    width = xy[0];
    height = xy[1];

    height = (double) Math.sqrt(width * width + height * height);
    capheight = nint(height * Math.abs(ws.c) + 1);
    ws.capheight = nint(capheight);

    fontNum = font - 1;
    size = nint(ws.capheight / capheights[fontNum]);

    if (font > 13)
      font += 3;

    ws.family = (font - 1) / 4;
    style += (font % 4 == 1 || font % 4 == 2) ? 0 : Font.BOLD;
    style += (font % 4 == 2 || font % 4 == 0) ? Font.ITALIC : 0;

    g.setFont(new Font(fonts[ws.family], style, size));
  }

  private static void emulText(double px, double py, int nchars, String text)
  {
    int[] xfac = { 1, -1, 0, 0 };
    int[] yfac = { 0, 0, 1, -1 };

    int i;
    int tnr, font, prec, alh, alv, path;
    double xn, yn, chsp, ax, ay, spacex, spacey;
    int[] txx, size, bottom, base, cap, top;
    int space;
    char[] chars = text.toCharArray();
    double[] xy;

    tnr = sl.cntnr;
    xy = WCtoNDC(px, py, tnr);
    xn = xy[0];
    yn = xy[1];

    txx = new int[1];
    size = new int[1];
    bottom = new int[1];
    base = new int[1];
    cap = new int[1];
    top = new int[1];

    font = sl.txfont;
    prec = sl.txprec;
    if (prec != GKS_K_TEXT_PRECISION_STROKE)
      font = mapFont(font);

    setChrXform();
    inqTextExtent(chars, nchars, font, prec, 
                  txx, size, bottom, base, cap, top);

    chsp = sl.chsp;
    space = nint(chsp * size[0]);
    txx[0] += nchars * space;

    alh = sl.txal[0];
    alv = sl.txal[1];
    path = sl.txp;

    if (path == GKS_K_TEXT_PATH_UP || path == GKS_K_TEXT_PATH_DOWN)
      txx[0] = size[0];

    switch (alh)
      {
      case GKS_K_TEXT_HALIGN_CENTER:
        ax = -0.5 * txx[0];
        break;
      case GKS_K_TEXT_HALIGN_RIGHT:
        ax = -txx[0];
        break;
      default:
        ax = 0;
      }

    if (path == GKS_K_TEXT_PATH_LEFT)
      {
        inqTextExtent(chars, 1, font, prec, 
                      txx, size, bottom, base, cap, top);
        ax = -ax - txx[0];
      }

    switch (alv)
      {
      case GKS_K_TEXT_VALIGN_TOP:
        ay = base[0] - top[0];
        break;
      case GKS_K_TEXT_VALIGN_CAP:
        ay = base[0] - cap[0];
        break;
      case GKS_K_TEXT_VALIGN_HALF:
        ay = (base[0] - cap[0]) * 0.5;
        break;
      case GKS_K_TEXT_VALIGN_BOTTOM:
        ay = base[0] - bottom[0];
        break;
      default:
        ay = 0;
      }

    xy = chrXform(ax, ay, size[0]);
    ax = xy[0];
    ay = xy[1];

    xn += ax;
    yn += ay;

    for (i = 0; i < nchars; i++)
      {
        char[] ch = new char[1];

        ch[0] = chars[i];
        inqTextExtent(ch, 1, font, prec, txx, size, bottom, base, cap, top);

        spacex = (txx[0] + space) * xfac[path];
        spacey = (top[0] - bottom[0] + space) * yfac[path];

        xy = chrXform(spacex, spacey, size[0]);
        spacex = xy[0];
        spacey = xy[1];

        drawCharacter(xn, yn, chars[i], font, 0);

        xn += spacex;
        yn += spacey;
      }
  }

  private static class strokeData
  {
    int left;
    int right;
    int size;
    int bottom;
    int base;
    int cap;
    int top;
    int length;
    int[][] coord = new int[124][2];
  }

  private static void inqTextExtent(
    char[] chars, int nchars, int font, int prec, 
    int[] txx, int[] size, int[] bottom, int[] base, int[] cap, int[] top)
  {
    int i;

    strokeData s = new strokeData();

    txx[0] = 0;
    if (nchars > 0)
      {
        if (prec != GKS_K_TEXT_PRECISION_STRING)
          {
            for (i = 0; i < nchars; i++)
              {
                lookupFont(sl.version, font, chars[i], s);
                if (chars[i] != ' ')
                  txx[0] += s.right - s.left;
                else
                  txx[0] += s.size / 2;
              }
          }
        else
          {
            for (i = 0; i < nchars; i++)
              {
                lookupAFM(font, chars[i], s);
                txx[0] += s.right - s.left;
              }
          }
      }
    else
      {
        if (prec != GKS_K_TEXT_PRECISION_STRING)
          lookupFont(sl.version, font, ' ', s);
        else
          lookupAFM(font, ' ', s);
      }

    size[0] = s.size;
    bottom[0] = s.bottom;
    base[0] = s.base;
    cap[0] = s.cap;
    top[0] = s.top;
  }

  private static void drawCharacter(double x, double y, char chr,
                                    int font, int flag)
  {
    int i;
    int xmin, xmax, ymin, ymax, xc, yc, n;
    double xn, yn;
    double[] px, py;
    int[] xpoints, ypoints;
    double[] window, viewport;
    double mszsc;
    double scalex = 0, scaley = 0, center = 0, half = 0;
    int[] dcp;
    double[] xy;
    strokeData s = new strokeData();

    window = new double[4];
    viewport = new double[4];

    lookupFont(sl.version, font, chr, s);

    px = new double[64];
    py = new double[64];
    xpoints = new int[64];
    ypoints = new int[64];

    if (flag != 0)
      {
        xmin = 127;
        xmax = 0;
        ymin = 127;
        ymax = 0;

        for (i = 0; i < s.length; i++)
          {
            xc = s.coord[i][0];
            if (xc > 127)
              xc = xc - 256;
            else if (xc < 0)
              xc = -xc;
            yc = s.coord[i][1];
            if (xc < xmin)
              xmin =  xc;
            else if (xc > xmax)
              xmax = xc;
            if (yc < ymin)
              ymin =  yc;
            else if (yc > ymax)
              ymax = yc;
          }
        if (xmax <= xmin)
          {
            xmin = s.left;
            xmax = s.right;
          }
        if (ymax <= ymin)
          {
            ymin = s.base;
            ymax = s.cap;
          }

        mszsc = sl.mszsc;
        scalex = 0.001 * mszsc / (xmax - xmin);
        scaley = 0.001 * mszsc / (ymax - ymin);
        scalex *= (ws.window[1] - ws.window[0]) /
                  (ws.viewport[1] - ws.viewport[0]);
        scaley *= (ws.window[3] - ws.window[2]) /
                  (ws.viewport[3] - ws.viewport[2]);

        center = 0.5 * (xmin + xmax);
        half = 0.5 * (ymin + ymax);
      }

    n = 0;

    for (i = 0; i < s.length; i++)
      {
        xc = s.coord[i][0];
        if (xc > 127)
          xc = xc - 256;
        yc = s.coord[i][1];

        if (xc < 0)
          {
            if (n > 1)
              {
                if (font == -51)
                  {
                    if (n > 2)
                      {
                        for (int k = 0; k < n; k++)
                          {
                            dcp = NDCtoDC(px[k], py[k]);
                            xpoints[k] = dcp[0];
                            ypoints[k] = dcp[1];
                          }
                        g.fillPolygon(xpoints, ypoints, n);
                      }
                  }

                for (int k = 0; k < n; k++)
                  {
                    dcp = NDCtoDC(px[k], py[k]);
                    xpoints[k] = dcp[0];
                    ypoints[k] = dcp[1];
                  }
                g.drawPolyline(xpoints, ypoints, n);

                n = 0;
              }
            xc = -xc;
          }

        if (flag != 0)
          {
            xn = scalex * (xc - center);
            yn = scaley * (yc - half);
          }
        else
          {
            if (s.left == s.right)
              xc += s.size / 2;

            xn = xc - s.left;
            yn = yc - s.base;

            xy = chrXform(xn, yn, s.size);
            xn = xy[0];
            yn = xy[1];
          }

        px[n] = x + xn;
        py[n] = y + yn;
        n++;
      }

    if (n > 1)
      {
        if (font == -51)
          {
            if (n > 2)
              {
                for (int k = 0; k < n; k++)
                  {
                    dcp = NDCtoDC(px[k], py[k]);
                    xpoints[k] = dcp[0];
                    ypoints[k] = dcp[1];
                  }
                g.fillPolygon(xpoints, ypoints, n);
              }
          }

        for (int k = 0; k < n; k++)
          {
            dcp = NDCtoDC(px[k], py[k]);
            xpoints[k] = dcp[0];
            ypoints[k] = dcp[1];
          }

        g.drawPolyline(xpoints, ypoints, n);
      }
  }

  private static byte[] fontarray = null;

  private static void lookupFont(int version, int font, int chr, strokeData s)
  {

  /*  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 */
    int[] map = {
      1,18, 1, 6,12, 3, 8,11, 4, 7,10, 2,13,14, 5, 9,15,16,17,20,21,19,22,23 };
    int[] gksgralmap = {
      1,12, 6, 9, 8,11, 5,13,18,17,19, 1, 4, 7,24, 1, 1, 1, 1, 1, 1, 1,23,24 };
    int[] s_map = {
      4, 4, 4, 4, 4, 7, 7, 7,10,10,10, 7, 7, 7, 4, 4, 7, 7, 7, 4, 4, 4, 4, 4 };
    int[] german = {
      196, 214, 220, 228, 246, 252, 223, 171, 187, 183, 169 };
    char[] ansi = {
      'A', 'O', 'U', 'a', 'o', 'u', 'b', '<', '>', '.', '@' };
    char[] greek = {
      'j', 'o', 'q', 'u', 'v', 'w', 'y', 'J', 'O', 'Q', 'U', 'V', 'W', 'Y' };
    char[] g_map = {
      ' ', 'w', ' ', 'o', 'y', 'v', 'q', ' ', 'W', ' ', 'O', 'Y', 'V', 'Q' };

    int umlaut, sharp_s, offset;
    int i;

    umlaut = 0;
    sharp_s = 0;

    if (chr < 0)
      chr += 256;

    if (chr >= 127)
      {
        for (i = 0; i <= 10; i++)
          {
            if (chr == german[i])
              {
                chr = ansi[i];
                if (i < 6)
                  umlaut = 1;
                else if (i == 6)
                  sharp_s = 1;
              }
          }
      }
    if (chr < ' ' || chr >= 127)
      chr = ' ';

    font = Math.abs(font) % 100;
    if (font == 51)
      font = 23;
    else if (font > 23)
      font = 1;

    if (chr == '_')
      {
        if (font < 20)
          font = 23;
      }
    else if (sharp_s != 0)
      {
        if (font != 23)
          font = s_map[font - 1];
        else
          chr = 126;
      }
    else if (version == GRALGKS)
      {
        if (font == 13 || font == 14)
          {
            for (i = 0; i < 14; i++)
              {
                if (chr == greek[i])
                  {
                    chr = g_map[i];
                    break;
                  }
              }
          }
        font = gksgralmap[font - 1];
      }

    chr -= ' ';
    offset = ((map[font - 1] - 1) * 95 + chr) * 256;

    s.left = buf[offset + 0];
    s.right = buf[offset + 1];
    s.size = buf[offset + 2];
    s.bottom = buf[offset + 3];
    s.base = buf[offset + 4];
    s.cap = buf[offset + 5];
    s.top = buf[offset + 6];
    s.length = buf[offset + 7];
    for (int j = 0; j < s.coord.length; j++)
      for (int k = 0; k < s.coord[0].length; k++)
          s.coord[j][k] = buf[offset + 8 + j*s.coord[0].length + k];

    if (umlaut != 0 && (s.length < 120 - 20))
      s.length += 10;
  }

  static final int[] roman = { 3, 12, 16, 11 };
  static final int[] greek = { 4, 7, 10, 7 };

  private static int mapFont(int font)
  {
    int family, type;

    font = Math.abs(font);
    family = ((font - 1) % 8) + 1;
    type = (font - 1) / 8;
    if (type > 3)
      type = 3;

    if (family != 7)
      font = roman[type];
    else
      font = greek[type];

    return font;
  }

  static final int psmap[] =
  {
    22,  9,  5, 14,
    18, 26, 13,  1,
    24, 11,  7, 16,
    20, 28, 13,  3,
    23, 10,  6, 15,
    19, 27, 13,  2,
    25, 12,  8, 17,
    21, 29, 13,  4
  };

  static final int[] descenders =
  {
    -217, -206, -210, -203,
    -219, -219, -219, -219,
    -207, -207, -257, -257,
    -205,
    -228, -212, -212, -213,
    -205, -205, -205, -205,
    -192, -192, -185, -185,
    -283, -276, -258, -271,
    -248, -205
  };

  static final int[] caps =
  {
    662,  660,  681,  662,
    729,  729,  729,  729,
    583,  583,  633,  633,
    667,
    681,  681,  681,  681,
    722,  722,  722,  722,
    739,  739,  739,  739,
    694,  693,  683,  683,
    587,  692
  };

  private static void lookupAFM(int font, int chr, strokeData s)
  {
    int psfont;

    psfont = Math.abs(font) - 1;
    if (chr < 0)
      chr += 256;
    if (chr == '-')
      chr = '+';

    if (psfont >= 100 && psfont <= 130)
      psfont -= 100;
    else if (psfont >= 0 && psfont <= 31)
      psfont = psmap[psfont] - 1;
    else
      psfont = 8;

    s.left = 0;
    s.right = afm.widths[psfont][chr % 256];
    s.size = caps[psfont];
    s.bottom = descenders[psfont];
    s.base = 0;
    s.cap = s.size;
    s.top = s.cap + 120;
  }

  static final int[] predef_font = { 1, 1, 1, -2, -3, -4 };
  static final int[] predef_prec = { 0, 1, 2, 2, 2, 2 };

  private static void text(double px, double py, String text)
  {
    int tx_font, tx_prec, tx_color, ln_width;
    double[] xy;

    tx_font  = sl.asf[6] != 0 ? sl.txfont : predef_font[sl.tindex - 1];
    tx_prec  = sl.asf[6] != 0 ? sl.txprec : predef_prec[sl.tindex - 1];
    tx_color = sl.asf[9] != 0 ? sl.txcoli : 1;

    if (sl.version > 4)
      {
        ln_width = nint((ws.width + ws.height) * 0.001);
        if (ln_width < 1)
          ln_width = 1;
      }
    else
      ln_width = 1;

    if (ln_width < 1)
      ln_width = 1;

    setColor(tx_color);

    if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
      {
        setFont(tx_font);

        xy = WCtoNDC(px, py, sl.cntnr);
        xy = segXform(xy[0], xy[1]);
        textRoutine(xy[0], xy[1], text);
      }
    else
      emulText(px, py, text.length(), text);
  }

  private static void cellarray(double xmin, double xmax, double ymin, double ymax,
                                int dx, int dy, int dimx, int[] colia,
                                int true_color)
  {
    int ix1, ix2, iy1, iy2;
    int x, y, width, height;
    int i, j, ix, iy, ind, rgb;
    int swapx, swapy;
    BufferedImage bi;
    double[] xy;
    int[] ixy;

    xy = WCtoNDC(xmin, ymax, sl.cntnr);
    xy = segXform(xy[0], xy[1]);
    ixy = NDCtoDC(xy[0], xy[1]);
    ix1 = ixy[0];
    iy1 = ixy[1];

    xy = WCtoNDC(xmax, ymin, sl.cntnr);
    xy = segXform(xy[0], xy[1]);
    ixy = NDCtoDC(xy[0], xy[1]);
    ix2 = ixy[0];
    iy2 = ixy[1];

    width = Math.abs(ix2 - ix1);
    height = Math.abs(iy2 - iy1);
    if (width == 0 || height == 0) return;
    x = Math.min(ix1, ix2);
    y = Math.min(iy1, iy2);

    swapx = (ix1 > ix2) ? 1 : 0;
    swapy = (iy1 < iy2) ? 1 : 0;

    bi = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);

    for (j = 0; j < height; j++)
      {
        iy = dy * j / height;
        if (swapy != 0)
          iy = dy - 1 - iy;
        for (i = 0; i < width; i++)
          {
            ix = dx * i / width;
            if (swapx != 0)
              ix = dx - 1 - ix;
            if (true_color == 0)
              {
                ind = colia[iy * dimx + ix];
                if (ind >= 0 && ind < rgba.length)
                  rgb = rgba[ind];
                else
                  rgb = rgba[0];
              }
            else
              rgb = swap(colia[iy * dimx + ix]) >> 8;
            bi.setRGB(i, j, rgb);
          }
      }
    g.drawImage(bi, x, y, null);
  }

  private static int sp;
  private static byte[] s;

  private static byte RESOLVE_byte()
  {
    return s[sp++];
  }

  private static int RESOLVE_int()
  {
    int arg = ((s[sp] & 0xff)) + ((s[sp+1] & 0xff) << 8) +
              ((s[sp+2] & 0xff) << 16) + ((s[sp+3] & 0xff) << 24);
    sp += 4;
    return arg;
  }

  private static double RESOLVE_double()
  {
    int nbytes = 4;
    int arg = 0;
    int tmp = sp;

    for (int shiftBy = 0; shiftBy < 32; shiftBy += 8)
      arg |= (s[tmp++] & 0xff) << shiftBy;
    sp += nbytes;
    return Float.intBitsToFloat(arg);
  }

  private static void RESOLVE_GKSStateList()
  {
    sl.lindex = RESOLVE_int();
    sl.ltype = RESOLVE_int();
    sl.lwidth = RESOLVE_double();
    sl.plcoli = RESOLVE_int();
    sl.mindex = RESOLVE_int();
    sl.mtype = RESOLVE_int();
    sl.mszsc = RESOLVE_double();
    sl.pmcoli = RESOLVE_int();
    sl.tindex = RESOLVE_int();
    sl.txfont = RESOLVE_int();
    sl.txprec = RESOLVE_int();
    sl.chxp = RESOLVE_double();
    sl.chsp = RESOLVE_double();
    sl.txcoli = RESOLVE_int();
    sl.chh = RESOLVE_double();
    for (int i = 0; i < 2; i++)
      sl.chup[i] = RESOLVE_double();
    sl.txp = RESOLVE_int();
    for (int i = 0; i < 2; i++)
      sl.txal[i] = RESOLVE_int();
    sl.findex = RESOLVE_int();
    sl.ints = RESOLVE_int();
    sl.styli = RESOLVE_int();
    sl.facoli = RESOLVE_int();

    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 4; j++)
        sl.window[i][j] = RESOLVE_double();
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 4; j++)
        sl.viewport[i][j] = RESOLVE_double();
    sl.cntnr = RESOLVE_int();
    sl.clip = RESOLVE_int();

    sl.opsg = RESOLVE_int();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 2; j++)
        sl.mat[i][j] = RESOLVE_double();

    for (int i = 0; i < 13; i++)
      sl.asf[i] = RESOLVE_int();

    sl.wiss = RESOLVE_int();
    sl.version = RESOLVE_int();
    sl.fontfile = RESOLVE_int();

    sl.txslant = RESOLVE_double();
    for (int i = 0; i < 2; i++)
      sl.shoff[i] = RESOLVE_double();
    sl.blur = RESOLVE_double();
    sl.alpha = RESOLVE_double();

    for (int i = 0; i < 9; i++)
      sl.a[i] = RESOLVE_double();
    for (int i = 0; i < 9; i++)
      sl.b[i] = RESOLVE_double();
    for (int i = 0; i < 9; i++)
      sl.c[i] = RESOLVE_double();
    for (int i = 0; i < 9; i++)
      sl.d[i] = RESOLVE_double();
  }

  private static void interp(byte[] str)
  {
    sp = 0;
    int len, f;
    int[] i_arr = null;
    int dx = 0;
    int dy = 0;
    int dimx = 0;
    int sx = 0;
    int sy = 0;
    int len_c_arr = 0;
    double[] f_arr_1 = null;
    double[] f_arr_2 = null;
    byte[] c_arr = null;
    int i;

    s = str;

    while (sp < s.length)
      {
        len = RESOLVE_int();
        f = RESOLVE_int();

        switch (f)
          {
          case 2:
            RESOLVE_GKSStateList();
            break;

          case 3:       /* close ws */
            break;

          case 12:      /* polyline */
          case 13:      /* polymarker */
          case 15:      /* fill area */
            i_arr = new int[1];
            i_arr[0] = RESOLVE_int();
            f_arr_1 = new double[i_arr[0]];
            f_arr_2 = new double[i_arr[0]];
            for (i = 0; i < f_arr_1.length; i++)
              f_arr_1[i] = RESOLVE_double();
            for (i = 0; i < f_arr_2.length; i++)
              f_arr_2[i] = RESOLVE_double();
            break;

          case 14:      /* text */
            f_arr_1 = new double[1];
            f_arr_2 = new double[1];
            f_arr_1[0] = RESOLVE_double();
            f_arr_2[0] = RESOLVE_double();
            len_c_arr = RESOLVE_int();
            c_arr = new byte[132];
            for (i = 0; i < c_arr.length; i++)
              c_arr[i] = RESOLVE_byte();
            break;

          case 16:      /* cell array */
          case 201:     /* draw image */
            f_arr_1 = new double[2];
            f_arr_2 = new double[2];
            f_arr_1[0] = RESOLVE_double();
            f_arr_1[1] = RESOLVE_double();
            f_arr_2[0] = RESOLVE_double();
            f_arr_2[1] = RESOLVE_double();
            dx = RESOLVE_int();
            dy = RESOLVE_int();
            dimx = RESOLVE_int();
            i_arr = new int[dimx * dy];
            for (i = 0; i < i_arr.length; i++)
              i_arr[i] = RESOLVE_int();
            break;

          case 19:      /* set linetype */
          case 21:      /* set polyline color index */
          case 23:      /* set markertype */
          case 25:      /* set polymarker color index */
          case 30:      /* set text color index */
          case 33:      /* set text path */
          case 36:      /* set fillarea interior style */
          case 37:      /* set fillarea style index */
          case 38:      /* set fillarea color index */
          case 52:      /* select normalization transformation */
          case 53:      /* set clipping indicator */
            i_arr = new int[1];
            i_arr[0] = RESOLVE_int();
            break;

          case 27:      /* set text font and precision */
          case 34:      /* set text alignment */
            i_arr = new int[2];
            i_arr[0] = RESOLVE_int();
            i_arr[1] = RESOLVE_int();
            break;

          case 20:      /* set linewidth scale factor */
          case 24:      /* set marker size scale factor */
          case 28:      /* set character expansion factor */
          case 29:      /* set character spacing */
          case 31:      /* set character height */
          case 200:     /* set text slant */
          case 203:     /* set transparency */
            f_arr_1 = new double[1];
            f_arr_1[0] = RESOLVE_double();
            break;

          case 32:      /* set character up vector */
            f_arr_1 = new double[1];
            f_arr_2 = new double[1];
            f_arr_1[0] = RESOLVE_double();
            f_arr_2[0] = RESOLVE_double();
            break;

          case 41:      /* set aspect source flags */
            i_arr = new int[13];
            for (i = 0; i < i_arr.length; i++)
              i_arr[0] = RESOLVE_int();
            break;

          case 48:      /* set color representation */
            i_arr = new int[1];
            i_arr[0] = RESOLVE_int();
            f_arr_1 = new double[3];
            f_arr_1[0] = RESOLVE_double();
            f_arr_1[1] = RESOLVE_double();
            f_arr_1[2] = RESOLVE_double();
            break;

          case 49:      /* set window */
          case 50:      /* set viewport */
          case 54:      /* set workstation window */
          case 55:      /* set workstation viewport */
            i_arr = new int[1];
            f_arr_1 = new double[2];
            f_arr_2 = new double[2];
            i_arr[0] = RESOLVE_int();
            f_arr_1[0] = RESOLVE_double();
            f_arr_1[1] = RESOLVE_double();
            f_arr_2[0] = RESOLVE_double();
            f_arr_2[1] = RESOLVE_double();
            break;

          default:
            System.err.printf("Display list corrupted (len = %d, fctid = %d, sp = %d)\n", len, f,sp);
            System.exit(1);
          }

        switch (f)
          {
          case 2:
            ws.width = ws.height = 500;

            ws.window[0] = ws.window[2] = 0.0;
            ws.window[1] = ws.window[3] = 1.0;

            ws.viewport[0] = ws.viewport[2] = 0.0;
            ws.viewport[1] = ws.width * 0.254 / 1024;
            ws.viewport[3] = ws.height * 0.1905 / 768;

            setXform();
            initNormXform();

            clear();
            break;

          case 12:
            polyline(i_arr[0], f_arr_1, f_arr_2);
            break;

          case 13:
            polymarker(i_arr[0], f_arr_1, f_arr_2);
            break;

          case 14:
            text(f_arr_1[0], f_arr_2[0],
               new String(c_arr, 0, len_c_arr, Charset.forName("ISO-8859-1")));
            break;

          case 15:
            fillarea(i_arr[0], f_arr_1, f_arr_2);
            break;

          case 16:
          case 201:
            int true_color = f == 201 ? 1 : 0;
            cellarray(f_arr_1[0], f_arr_1[1], f_arr_2[0], f_arr_2[1],dx, dy, dimx, i_arr, true_color);
            break;

          case 19:
            sl.ltype = i_arr[0];
            break;
          case 20:
            sl.lwidth = f_arr_1[0];
            break;
          case 21:
            sl.plcoli = i_arr[0];
            break;
          case 23:
            sl.mtype = i_arr[0];
            break;
          case 24:
            sl.mszsc = f_arr_1[0];
            break;
          case 25:
            sl.pmcoli = i_arr[0];
            break;
          case 27:
            sl.txfont = i_arr[0];
            sl.txprec = i_arr[1];
            break;
          case 28:
            sl.chxp = f_arr_1[0];
            break;
          case 29:
            sl.chsp = f_arr_1[0];
            break;
          case 30:
            sl.txcoli = i_arr[0];
            break;
          case 31:
            sl.chh = f_arr_1[0];
            break;
          case 32:
            sl.chup[0] = f_arr_1[0];
            sl.chup[1] = f_arr_2[0];
            break;
          case 33:
            sl.txp = i_arr[0];
            break;
          case 34:
            sl.txal[0] = i_arr[0];
            sl.txal[1] = i_arr[1];
            break;
          case 36:
            sl.ints = i_arr[0];
            break;
          case 37:
            sl.styli = i_arr[0];
            break;
          case 38:
            sl.facoli = i_arr[0];
            break;

          case 48:
            setColorRep(i_arr[0], f_arr_1[0], f_arr_1[1], f_arr_1[2]);
            break;

          case 49:
            sl.window[i_arr[0]][0] = f_arr_1[0];
            sl.window[i_arr[0]][1] = f_arr_1[1];
            sl.window[i_arr[0]][2] = f_arr_2[0];
            sl.window[i_arr[0]][3] = f_arr_2[1];
            setNormXform(i_arr[0]);
            setClipRect(1);
            break;

          case 50:
            sl.viewport[i_arr[0]][0] = f_arr_1[0];
            sl.viewport[i_arr[0]][1] = f_arr_1[1];
            sl.viewport[i_arr[0]][2] = f_arr_2[0];
            sl.viewport[i_arr[0]][3] = f_arr_2[1];
            setNormXform(i_arr[0]);
            setClipRect(1);
            break;

          case 52:
            sl.cntnr = i_arr[0];
            setClipRect(1);
            break;

          case 53:
            sl.clip = i_arr[0];
            setClipRect(1);
            break;

          case 54:
            ws.window[0] = f_arr_1[0];
            ws.window[1] = f_arr_1[1];
            ws.window[2] = f_arr_2[0];
            ws.window[3] = f_arr_2[1];

            setXform();
            initNormXform();
            break;

          case 55:
            ws.viewport[0] = f_arr_1[0];
            ws.viewport[1] = f_arr_1[1];
            ws.viewport[2] = f_arr_2[0];
            ws.viewport[3] = f_arr_2[1];

            setXform();
            initNormXform();
            break;

          case 200:
            sl.txslant = f_arr_1[0];
            break;

          case 202:
            sl.shoff[0] = f_arr_1[0];
            sl.shoff[1] = f_arr_1[1];
            sl.blur = f_arr_1[2];
            break;

          case 203:
            sl.alpha = f_arr_1[0];
            break;

          case 204:
            sl.mat[0][0] = f_arr_1[0];
            sl.mat[0][1] = f_arr_1[1];
            sl.mat[1][0] = f_arr_1[2];
            sl.mat[1][1] = f_arr_1[3];
            sl.mat[2][0] = f_arr_1[4];
            sl.mat[2][1] = f_arr_1[5];
            break;
          }
      }
  }

  private static int swap(int value)
  {
    int b1 = (value >>  0) & 0xff;
    int b2 = (value >>  8) & 0xff;
    int b3 = (value >> 16) & 0xff;
    int b4 = (value >> 24) & 0xff;

    return b1 << 24 | b2 << 16 | b3 << 8 | b4 << 0;
  }

  public static void listen()
  {
    ServerSocket s = null;
    Socket c = null;
    DataInputStream is = null;
    int nbyte = 0;
    boolean done = false;
    DataInputStream stream;
    Graphics gr;

    try
      {
        s = new ServerSocket(8410);
        c = s.accept();
        is = new DataInputStream(c.getInputStream());
      }
    catch (SecurityException e)
      {
        System.err.println("Socket security exception");
      }
    catch (IOException ioe)
      {
        System.err.println("Socket I/O exception");
      }

    if (s != null && c != null && is != null)
      {
        sl = new GKSStateList();
        ws = new WSStateList();

        win = new Window();
        win.init(500, 500);

        try {
          stream = new DataInputStream(
                   gksweb.class.getResourceAsStream("gksfont.dat"));
          buf = new byte[stream.available()];
          stream.readFully(buf);
          stream.close();
        } catch (Exception e) {
          System.err.println("Error reading gksfont.dat.");
        }

        while (!done)
          {
            try
              {
                nbyte = swap(is.readInt());
                byte[] x = new byte[nbyte];
                is.readFully(x);
                interp(x);
                gr = frame.getGraphics();
                gr.drawImage(offscreen, 0, 0, null);
              }
            catch (IOException e)
              {
                System.err.println("Socket I/O error");
                done = true;
              }
          }
      }
  }

  public void init()
  {
    listen();
  }

  public static void main(String[] args)
  {
    listen();
  }
}

