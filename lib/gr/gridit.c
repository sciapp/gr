#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*#include "gr.h"*/
#include "gridit.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define Integer static int
#define Real    static double

static
int idcldp(int *ndp, double *xd, double *yd, int *ncp, int *ipc)
{
  Integer j1, j2, j3, j4;
  Real x1, y1, r1, r2;
  Integer ip1, ip2, ip3;
  Real dx12, dy12, dx13, dy13;
  Integer jmx, ipc0[25], ncp0, ndp0;
  Real dsq0[25], dsqi;
  Integer ip2mn, ip3mn, nclpt;
  Real dsqmn, dsqmx;

  /* THIS SUBROUTINE SELECTS SEVERAL DATA POINTS THAT ARE CLOSEST */
  /* TO EACH OF THE DATA POINT. */
  /* THE INPUT PARAMETERS ARE */
  /*     NDP = NUMBER OF DATA POINTS, */
  /*     XD,YD = ARRAYS OF DIMENSION NDP CONTAINING THE X AND Y */
  /*           COORDINATES OF THE DATA POINTS, */
  /*     NCP = NUMBER OF DATA POINTS CLOSEST TO EACH DATA */
  /*           POINTS. */
  /* THE OUTPUT PARAMETER IS */
  /*     IPC = INTEGER ARRAY OF DIMENSION NCP*NDP, WHERE THE */
  /*           POINT NUMBERS OF NCP DATA POINTS CLOSEST TO */
  /*           EACH OF THE NDP DATA POINTS ARE TO BE STORED. */
  /* THIS SUBROUTINE ARBITRARILY SETS A RESTRICTION THAT NCP MUST */
  /* NOT EXCEED 25. */

  /* PRELIMINARY PROCESSING */
  ip3mn = 0;
  jmx = 0;
  ndp0 = *ndp;
  ncp0 = *ncp;
  dsqmn = 0.;
  if (ndp0 >= 2) {
    if (ncp0 >= 1 && ncp0 <= 25 && ncp0 < ndp0) {
      /* CALCULATION */
      for (ip1 = 1; ip1 <= ndp0; ++ip1) {
        /* - SELECTS NCP POINTS. */
        x1 = xd[ip1-1];
        y1 = yd[ip1-1];
        j1 = 0;
        dsqmx = 0.;
        for (ip2 = 1; ip2 <= ndp0; ++ip2) {
          if (ip2 != ip1) {
            r1 = xd[ip2-1] - x1;
            r2 = yd[ip2-1] - y1;
            dsqi = r1 * r1 + r2 * r2;
            ++j1;
            dsq0[j1 - 1] = dsqi;
            ipc0[j1 - 1] = ip2;
            if (dsqi > dsqmx) {
              dsqmx = dsqi;
              jmx = j1;
            }
            if (j1 >= ncp0) {
              goto L30;
            }
          }
          /* L20: */
        }
      L30:
        ip2mn = ip2 + 1;
        if (ip2mn <= ndp0) {
          for (ip2 = ip2mn; ip2 <= ndp0; ++ip2) {
            if (ip2 != ip1) {
              r1 = xd[ip2-1] - x1;
              r2 = yd[ip2-1] - y1;
              dsqi = r1 * r1 + r2 * r2;
              if (dsqi < dsqmx) {
                dsq0[jmx - 1] = dsqi;
                ipc0[jmx - 1] = ip2;
                dsqmx = 0.;
                for (j1 = 1; j1 <= ncp0; ++j1) {
                  if (dsq0[j1 - 1] > dsqmx) {
                    dsqmx = dsq0[j1 - 1];
                    jmx = j1;
                  }
                  /* L50: */
                }
              }
            }
            /* L40: */
          }
        }
        /* - CHECKS IF ALL THE NCP+1 POINTS ARE COLLINEAR. */
        ip2 = ipc0[0];
        dx12 = xd[ip2-1] - x1;
        dy12 = yd[ip2-1] - y1;
        for (j3 = 2; j3 <= ncp0; ++j3) {
          ip3 = ipc0[j3 - 1];
          dx13 = xd[ip3-1] - x1;
          dy13 = yd[ip3-1] - y1;
          if ((r1 = dy13 * dx12 - dx13 * dy12, fabs(r1)) > 1e-6f) {
            goto L70;
          }
          /* L60: */
        }
        /* - SEARCHES FOR THE CLOSEST NONCOLLINEAR POINT. */
        nclpt = 0;
        for (ip3 = 1; ip3 <= ndp0; ++ip3) {
          if (ip3 != ip1) {
            for (j4 = 1; j4 <= ncp0; ++j4) {
              if (ip3 == ipc0[j4 - 1]) {
                goto L80;
              }
              /* L90: */
            }
            dx13 = xd[ip3-1] - x1;
            dy13 = yd[ip3-1] - y1;
            if ((r1 = dy13 * dx12 - dx13 * dy12, fabs(r1)) > 1e-6f) {
              r1 = xd[ip3-1] - x1;
              r2 = yd[ip3-1] - y1;
              dsqi = r1 * r1 + r2 * r2;
              if (nclpt != 0) {
                if (dsqi >= dsqmn) {
                  goto L80;
                }
              }
              nclpt = 1;
              dsqmn = dsqi;
              ip3mn = ip3;
            }
          }
        L80:
          ;
        }
        if (nclpt == 0) {
          goto L100;
        } else {
          dsqmx = dsqmn;
          ipc0[jmx - 1] = ip3mn;
        }
        /* - REPLACES THE LOCAL ARRAY FOR THE OUTPUT ARRAY. */
      L70:
        j1 = (ip1 - 1) * ncp0;
        for (j2 = 1; j2 <= ncp0; ++j2) {
          ++j1;
          ipc[j1-1] = ipc0[j2 - 1];
          /* L110: */
        }
        /* L10: */
      }
      return 0;
    L100:
      fprintf(stderr, " ***   ALL COLLINEAR DATA POINTS.\n");
      goto L120;
    }
  }
  /* ERROR EXIT */
  fprintf(stderr, " ***   IMPROPER INPUT PARAMETER VALUE(S).\n");
 L120:
  fprintf(stderr, "   NDP =%5d   NCP =%5d\n"
          " ERROR DETECTED IN ROUTINE   IDCLDP\n", ndp0, ncp0);
  ipc[0] = 0; /*1*/
  return 0;
}

static
int idgrid(double *xd, double *yd, int *nt, int *ipt,
           int *nl, int *ipl, int *nxi, int *nyi, double *xi, double *yi,
           int *ngp, int *igp)
{
  Integer l;
  Real x1, y1, x2, y2, x3, y3, r1, r2;
  Integer il0, nl0, ip1, ip2, it0, ip3, nt0, ixi, iyi;
  Real yii, xii;
  Integer izi;
  Real xmn, ymn, xmx, ymx;
  Integer ngp0, ngp1, ilp1, nxi0, nyi0, il0t3, insd, it0t3;
  Real ximn, yimn, expr, ximx, yimx;
  Integer jigp0, jigp1, jngp0, jngp1, ilp1t3, iximn, iximx, jigp1i, nxinyi;

  /* THIS SUBROUTINE ORGANIZES GRID POINTS FOR SURFACE FITTING BY */
  /* SORTING THEM IN ASCENDING ORDER OF TRIANGLE NUMBERS AND OF THE */
  /* BORDER LINE SEGMENT NUMBER. */
  /* THE INPUT PARAMETERS ARE */
  /*     XD,YD = ARRAYS OF DIMENSION NDP CONTAINING THE X AND Y */
  /*           COORDINATES OF THE DATA POINTS, WHERE NDP IS THE */
  /*           NUMBER OF THE DATA POINTS, */
  /*     NT  = NUMBER OF TRIANGLES, */
  /*     IPT = INTEGER ARRAY OF DIMENSION 3*NT CONTAINING THE */
  /*           POINT NUMBERS OF THE VERTEXES OF THE TRIANGLES, */
  /*     NL  = NUMBER OF BORDER LINE SEGMENTS, */
  /*     IPL = INTEGER ARRAY OF DIMENSION 3*NL CONTAINING THE */
  /*           POINT NUMBERS OF THE END POINTS OF THE BORDER */
  /*           LINE SEGMENTS AND THEIR RESPECTIVE TRIANGLE */
  /*           NUMBERS, */
  /*     NXI = NUMBER OF GRID POINTS IN THE X COORDINATE, */
  /*     NYI = NUMBER OF GRID POINTS IN THE Y COORDINATE, */
  /*     XI,YI = ARRAYS OF DIMENSION NXI AND NYI CONTAINING */
  /*           THE X AND Y COORDINATES OF THE GRID POINTS, */
  /*           RESPECTIVELY. */
  /* THE OUTPUT PARAMETERS ARE */
  /*     NGP = INTEGER ARRAY OF DIMENSION 2*(NT+2*NL) WHERE THE */
  /*           NUMBER OF GRID POINTS THAT BELONG TO EACH OF THE */
  /*           TRIANGLES OR OF THE BORDER LINE SEGMENTS ARE TO */
  /*           BE STORED, */
  /*     IGP = INTEGER ARRAY OF DIMENSION NXI*NYI WHERE THE */
  /*           GRID POINT NUMBERS ARE TO BE STORED IN ASCENDING */
  /*           ORDER OF THE TRIANGLE NUMBER AND THE BORDER LINE */
  /*           SEGMENT NUMBER. */

  nt0 = *nt;
  nl0 = *nl;
  nxi0 = iximn = *nxi;
  nyi0 = *nyi;
  nxinyi = nxi0 * nyi0;
  r1 = xi[0], r2 = xi[nxi0-1];
  ximn = min(r1,r2);
  r1 = xi[0], r2 = xi[nxi0-1];
  ximx = max(r1,r2);
  r1 = yi[0], r2 = yi[nyi0-1];
  yimn = min(r1,r2);
  r1 = yi[0], r2 = yi[nyi0-1];
  yimx = max(r1,r2);
  /* DETERMINES GRID POINTS INSIDE THE DATA AREA. */
  jngp0 = 0;
  jngp1 = 2 * (nt0 + 2 * nl0) + 1;
  jigp0 = 0;
  jigp1 = nxinyi + 1;
  for (it0 = 1; it0 <= nt0; ++it0) {
    ngp0 = 0;
    ngp1 = 0;
    it0t3 = it0 * 3;
    ip1 = ipt[it0t3 - 3];
    ip2 = ipt[it0t3 - 2];
    ip3 = ipt[it0t3 - 1];
    x1 = xd[ip1-1];
    y1 = yd[ip1-1];
    x2 = xd[ip2-1];
    y2 = yd[ip2-1];
    x3 = xd[ip3-1];
    y3 = yd[ip3-1];
    r1 = min(x1,x2);
    xmn = min(r1,x3);
    r1 = max(x1,x2);
    xmx = max(r1,x3);
    r1 = min(y1,y2);
    ymn = min(r1,y3);
    r1 = max(y1,y2);
    ymx = max(r1,y3);
    insd = 0;
    for (ixi = 1; ixi <= nxi0; ++ixi) {
      if (xi[ixi-1] >= xmn && xi[ixi-1] <= xmx) {
        if (insd != 1) {
          insd = 1;
          iximn = ixi;
        }
      } else if (insd != 0) {
        goto L30;
      }
      /* L20: */
    }
    if (insd == 0) {
      goto L40;
    } else {
      iximx = nxi0;
      goto L50;
    }
  L30:
    iximx = ixi - 1;
  L50:
    for (iyi = 1; iyi <= nyi0; ++iyi) {
      yii = yi[iyi-1];
      if (yii >= ymn && yii <= ymx) {
        for (ixi = iximn; ixi <= iximx; ++ixi) {
          xii = xi[ixi-1];
          l = 0;
          expr = (x1 - xii) * (y2 - yii) - (y1 - yii) * (x2 - xii);
          if (expr < 0.) {
            goto L70;
          }
          if (expr == 0.) {
            l = 1;
          }
          expr = (x2 - xii) * (y3 - yii) - (y2 - yii) * (x3 - xii);
          if (expr < 0.) {
            goto L70;
          }
          if (expr == 0.) {
            l = 1;
          }
          expr = (x3 - xii) * (y1 - yii) - (y3 - yii) * (x1 - xii);
          if (expr < 0.) {
            goto L70;
          }
          if (expr == 0.) {
            l = 1;
          }
          izi = nxi0 * (iyi - 1) + ixi;
          if (l == 1) {
            if (jigp1 <= nxinyi) {
              for (jigp1i = jigp1; jigp1i <= nxinyi; ++jigp1i) {
                if (izi == igp[jigp1i-1]) {
                  goto L70;
                }
                /* L140: */
              }
            }
            ++ngp1;
            --jigp1;
            igp[jigp1-1] = izi;
          } else {
            ++ngp0;
            ++jigp0;
            igp[jigp0-1] = izi;
          }
        L70:
          ;
        }
      }
      /* L60: */
    }
  L40:
    ++jngp0;
    ngp[jngp0-1] = ngp0;
    --jngp1;
    ngp[jngp1-1] = ngp1;
    /* L10: */
  }
  /* DETERMINES GRID POINTS OUTSIDE THE DATA AREA. */
  /* - IN SEMI-INFINITE RECTANGULAR AREA. */
  for (il0 = 1; il0 <= nl0; ++il0) {
    ngp0 = 0;
    ngp1 = 0;
    il0t3 = il0 * 3;
    ip1 = ipl[il0t3 - 3];
    ip2 = ipl[il0t3 - 2];
    x1 = xd[ip1-1];
    y1 = yd[ip1-1];
    x2 = xd[ip2-1];
    y2 = yd[ip2-1];
    xmn = ximn;
    xmx = ximx;
    ymn = yimn;
    ymx = yimx;
    if (y2 >= y1) {
      xmn = min(x1,x2);
    }
    if (y2 <= y1) {
      xmx = max(x1,x2);
    }
    if (x2 <= x1) {
      ymn = min(y1,y2);
    }
    if (x2 >= x1) {
      ymx = max(y1,y2);
    }
    insd = 0;
    for (ixi = 1; ixi <= nxi0; ++ixi) {
      if (xi[ixi-1] >= xmn && xi[ixi-1] <= xmx) {
        if (insd != 1) {
          insd = 1;
          iximn = ixi;
        }
      } else if (insd != 0) {
        goto L170;
      }
      /* L160: */
    }
    if (insd == 0) {
      goto L180;
    } else {
      iximx = nxi0;
      goto L190;
    }
  L170:
    iximx = ixi - 1;
  L190:
    for (iyi = 1; iyi <= nyi0; ++iyi) {
      yii = yi[iyi-1];
      if (yii >= ymn && yii <= ymx) {
        for (ixi = iximn; ixi <= iximx; ++ixi) {
          xii = xi[ixi-1];
          l = 0;
          expr = (x1 - xii) * (y2 - yii) - (y1 - yii) * (x2 - xii);
          if (expr > 0.) {
            goto L210;
          }
          if (expr == 0.) {
            l = 1;
          }
          expr = (x2 - x1) * (xii - x1) + (y2 - y1) * (yii - y1);
          if (expr < 0.) {
            goto L210;
          }
          if (expr == 0.) {
            l = 1;
          }
          expr = (x1 - x2) * (xii - x2) + (y1 - y2) * (yii - y2);
          if (expr < 0.) {
            goto L210;
          }
          if (expr == 0.) {
            l = 1;
          }
          izi = nxi0 * (iyi - 1) + ixi;
          if (l == 1) {
            if (jigp1 <= nxinyi) {
              for (jigp1i = jigp1; jigp1i <= nxinyi; ++jigp1i) {
                if (izi == igp[jigp1i-1]) {
                  goto L210;
                }
                /* L280: */
              }
            }
            ++ngp1;
            --jigp1;
            igp[jigp1-1] = izi;
          } else {
            ++ngp0;
            ++jigp0;
            igp[jigp0-1] = izi;
          }
        L210:
          ;
        }
      }
      /* L200: */
    }
  L180:
    ++jngp0;
    ngp[jngp0-1] = ngp0;
    --jngp1;
    ngp[jngp1-1] = ngp1;
    /* - IN SEMI-INFINITE TRIANGULAR AREA. */
    ngp0 = 0;
    ngp1 = 0;
    ilp1 = il0 % nl0 + 1;
    ilp1t3 = ilp1 * 3;
    ip3 = ipl[ilp1t3 - 2];
    x3 = xd[ip3-1];
    y3 = yd[ip3-1];
    xmn = ximn;
    xmx = ximx;
    ymn = yimn;
    ymx = yimx;
    if (y3 >= y2 && y2 >= y1) {
      xmn = x2;
    }
    if (y3 <= y2 && y2 <= y1) {
      xmx = x2;
    }
    if (x3 <= x2 && x2 <= x1) {
      ymn = y2;
    }
    if (x3 >= x2 && x2 >= x1) {
      ymx = y2;
    }
    insd = 0;
    for (ixi = 1; ixi <=nxi0; ++ixi) {
      if (xi[ixi-1] >= xmn && xi[ixi-1] <= xmx) {
        if (insd != 1) {
          insd = 1;
          iximn = ixi;
        }
      } else if (insd != 0) {
        goto L300;
      }
      /* L290: */
    }
    if (insd == 0) {
      goto L310;
    } else {
      iximx = nxi0;
      goto L320;
    }
  L300:
    iximx = ixi - 1;
  L320:
    for (iyi = 1; iyi <= nyi0; ++iyi) {
      yii = yi[iyi-1];
      if (yii >= ymn && yii <= ymx) {
        for (ixi = iximn; ixi <= iximx; ++ixi) {
          xii = xi[ixi-1];
          l = 0;
          expr = (x1 - x2) * (xii - x2) + (y1 - y2) * (yii - y2);
          if (expr > 0.) {
            goto L340;
          }
          if (expr == 0.) {
            l = 1;
          }
          expr = (x3 - x2) * (xii - x2) + (y3 - y2) * (yii - y2);
          if (expr > 0.) {
            goto L340;
          }
          if (expr == 0.) {
            l = 1;
          }
          izi = nxi0 * (iyi - 1) + ixi;
          if (l == 1) {
            if (jigp1 <= nxinyi) {
              for (jigp1i = jigp1; jigp1i <= nxinyi; ++jigp1i) {
                if (izi == igp[jigp1i-1]) {
                  goto L340;
                }
                /* L390: */
              }
            }
            ++ngp1;
            --jigp1;
            igp[jigp1-1] = izi;
          } else {
            ++ngp0;
            ++jigp0;
            igp[jigp0-1] = izi;
          }
        L340:
          ;
        }
      }
      /* L330: */
    }
  L310:
    ++jngp0;
    ngp[jngp0-1] = ngp0;
    --jngp1;
    ngp[jngp1-1] = ngp1;
    /* L150: */
  }
  return 0;
}

static
int idpdrv(int *ndp, double *xd, double *yd, double *zd,
           int *ncp, int *ipc, double *pd)
{
  Real x0, y0, z0;
  Integer ic1, ic2, ip0;
  Real dx1, dy1, dz1, dx2, dy2, dz2, zx0, zy0;
  Integer jpd, ipi;
  Real nmx, nmy, nmz;
  Integer jpd0, ncp0, ndp0;
  Real dzx1, dzy1, dzx2, dzy2;
  Integer jipc;
  Real dnmx, dnmy, dnmz, nmxx, nmxy, nmyx, nmyy;
  Integer jipc0, ic2mn, ncpm1;
  Real dnmxx, dnmxy, dnmyx, dnmyy;

  /* THIS SUBROUTINE ESTIMATES PARTIAL DERIVATIVES OF THE FIRST AND */
  /* SECOND ORDER AT THE DATA POINTS. */
  /* THE INPUT PARAMETERS ARE */
  /*     NDP = NUMBER OF DATA POINTS, */
  /*     XD,YD,ZD = ARRAYS OF DIMENSION NDP CONTAINING THE X, */
  /*           Y, AND Z COORDINATES OF THE DATA POINTS, */
  /*     NCP = NUMBER OF ADDITIONAL DATA POINTS USED FOR ESTI- */
  /*           MATING PARTIAL DERIVATIVES AT EACH DATA POINT, */
  /*     IPC = INTEGER ARRAY OF DIMENSION NCP*NDP CONTAINING */
  /*           THE POINT NUMBERS OF NCP DATA POINTS CLOSEST TO */
  /*           EACH OF THE NDP DATA POINTS. */
  /* THE OUTPUT PARAMETER IS */
  /*     PD  = ARRAY OF DIMENSION 5*NDP, WHERE THE ESTIMATED */
  /*           ZX, ZY, ZXX, ZXY, AND ZYY VALUES AT THE DATA */
  /*           POINTS ARE TO BE STORED. */

  ndp0 = *ndp;
  ncp0 = *ncp;
  ncpm1 = ncp0 - 1;
  /* ESTIMATION OF ZX AND ZY */
  for (ip0 = 1; ip0 <= ndp0; ++ip0) {
    x0 = xd[ip0-1];
    y0 = yd[ip0-1];
    z0 = zd[ip0-1];
    nmx = 0.;
    nmy = 0.;
    nmz = 0.;
    jipc0 = ncp0 * (ip0 - 1);
    for (ic1 = 1; ic1 <= ncpm1; ++ic1) {
      jipc = jipc0 + ic1;
      ipi = ipc[jipc-1];
      dx1 = xd[ipi-1] - x0;
      dy1 = yd[ipi-1] - y0;
      dz1 = zd[ipi-1] - z0;
      ic2mn = ic1 + 1;
      for (ic2 = ic2mn; ic2 <= ncp0; ++ic2) {
        jipc = jipc0 + ic2;
        ipi = ipc[jipc-1];
        dx2 = xd[ipi-1] - x0;
        dy2 = yd[ipi-1] - y0;
        dnmz = dx1 * dy2 - dy1 * dx2;
        if (fabs(dnmz) > 1e-6f) {
          dz2 = zd[ipi-1] - z0;
          dnmx = dy1 * dz2 - dz1 * dy2;
          dnmy = dz1 * dx2 - dx1 * dz2;
          if (dnmz < 0.) {
            dnmx = -dnmx;
            dnmy = -dnmy;
            dnmz = -dnmz;
          }
          nmx += dnmx;
          nmy += dnmy;
          nmz += dnmz;
        }
        /* L30: */
      }
      /* L20: */
    }
    jpd0 = ip0 * 5;
    pd[jpd0 - 5] = -nmx / nmz;
    pd[jpd0 - 4] = -nmy / nmz;
    /* L10: */
  }
  /* ESTIMATION OF ZXX, ZXY, AND ZYY */
  for (ip0 = 1; ip0 <= ndp0; ++ip0) {
    jpd0 += 5;
    x0 = xd[ip0-1];
    jpd0 = ip0 * 5;
    y0 = yd[ip0-1];
    zx0 = pd[jpd0 - 5];
    zy0 = pd[jpd0 - 4];
    nmxx = 0.;
    nmxy = 0.;
    nmyx = 0.;
    nmyy = 0.;
    nmz = 0.;
    jipc0 = ncp0 * (ip0 - 1);
    for (ic1 = 1; ic1 <= ncpm1; ++ic1) {
      jipc = jipc0 + ic1;
      ipi = ipc[jipc-1];
      dx1 = xd[ipi-1] - x0;
      dy1 = yd[ipi-1] - y0;
      jpd = ipi * 5;
      dzx1 = pd[jpd - 5] - zx0;
      dzy1 = pd[jpd - 4] - zy0;
      ic2mn = ic1 + 1;
      for (ic2 = ic2mn; ic2 <= ncp0; ++ic2) {
        jipc = jipc0 + ic2;
        ipi = ipc[jipc-1];
        dx2 = xd[ipi-1] - x0;
        dy2 = yd[ipi-1] - y0;
        dnmz = dx1 * dy2 - dy1 * dx2;
        if (fabs(dnmz) > 1e-6f) {
          jpd = ipi * 5;
          dzx2 = pd[jpd - 5] - zx0;
          dzy2 = pd[jpd - 4] - zy0;
          dnmxx = dy1 * dzx2 - dzx1 * dy2;
          dnmxy = dzx1 * dx2 - dx1 * dzx2;
          dnmyx = dy1 * dzy2 - dzy1 * dy2;
          dnmyy = dzy1 * dx2 - dx1 * dzy2;
          if (dnmz < 0.) {
            dnmxx = -dnmxx;
            dnmxy = -dnmxy;
            dnmyx = -dnmyx;
            dnmyy = -dnmyy;
            dnmz = -dnmz;
          }
          nmxx += dnmxx;
          nmxy += dnmxy;
          nmyx += dnmyx;
          nmyy += dnmyy;
          nmz += dnmz;
        }
        /* L60: */
      }
      /* L50: */
    }
    pd[jpd0 - 3] = -nmxx / nmz;
    pd[jpd0 - 2] = -(nmxy + nmyx) / (nmz * 2.);
    pd[jpd0 - 1] = -nmyy / nmz;
    /* L40: */
  }
  return 0;
}

static
int idptip(double *xd, double *yd, double *zd, int *nt,
           int *ipt, int *nl, int *ipl, double *pdd, int *iti,
           double *xii, double *yii, double *zii, int *itpv)
{
  Real ap = 0., bp = 0., cp = 0., dp = 0., x0 = 0., y0 = 0.;
  Real p5 = 0., p00 = 0., p01 = 0., p02 = 0., p03 = 0., p04 = 0.;
  Real p05 = 0., p10 = 0., p11 = 0., p12 = 0., p13 = 0., p14 = 0.;
  Real p20 = 0., p21 = 0., p22 = 0., p23 = 0., p30 = 0., p31 = 0.;
  Real p32 = 0., p40 = 0., p41 = 0., p50 = 0.;
  Real a, b, c, d;
  Integer i;
  Real u, v, x[3], y[3], z[3], g1, h1, h2, h3, g2, p0, p1, p2, p3, p4;
  Real aa, ab, bb, ad, bc, cc, cd, dd, ac;
  Real pd[15], lu, lv;
  Real zu[3], zv[3], dx, dy;
  Integer il1, il2, it0, idp, jpd, kpd;
  Real dlt;
  Integer ntl;
  Real zuu[3], zuv[3], zvv[3], act2, bdt2, adbc;
  Integer jpdd, jipl, jipt;
  Real csuv, thus, thsv, thuv, thxu;

  /* THIS SUBROUTINE PERFORMS PUNCTUAL INTERPOLATION OR EXTRAPOLA- */
  /* TION, I.E., DETERMINES THE Z VALUE AT A POINT. */
  /* THE INPUT PARAMETERS ARE */
  /*     XD,YD,ZD = ARRAYS OF DIMENSION NDP CONTAINING THE X, */
  /*           Y, AND Z COORDINATES OF THE DATA POINTS, WHERE */
  /*           NDP IS THE NUMBER OF THE DATA POINTS, */
  /*     NT  = NUMBER OF TRIANGLES, */
  /*     IPT = INTEGER ARRAY OF DIMENSION 3*NT CONTAINING THE */
  /*           POINT NUMBERS OF THE VERTEXES OF THE TRIANGLES, */
  /*     NL  = NUMBER OF BORDER LINE SEGMENTS, */
  /*     IPL = INTEGER ARRAY OF DIMENSION 3*NL CONTAINING THE */
  /*           POINT NUMBERS OF THE END POINTS OF THE BORDER */
  /*           LINE SEGMENTS AND THEIR RESPECTIVE TRIANGLE */
  /*           NUMBERS, */
  /*     PDD = ARRAY OF DIMENSION 5*NDP CONTAINING THE PARTIAL */
  /*           DERIVATIVES AT THE DATA POINTS, */
  /*     ITI = TRIANGLE NUMBER OF THE TRIANGLE IN WHICH LIES */
  /*           THE POINT FOR WHICH INTERPOLATION IS TO BE */
  /*           PERFORMED, */
  /*     XII,YII = X AND Y COORDINATES OF THE POINT FOR WHICH */
  /*           INTERPOLATION IS TO BE PERFORMED. */
  /* THE OUTPUT PARAMETER IS */
  /*     ZII = INTERPOLATED Z VALUE. */

  it0 = *iti;
  ntl = *nt + *nl;
  if (it0 <= ntl) {
    /* CALCULATION OF ZII BY INTERPOLATION. */
    /* CHECKS IF THE NECESSARY COEFFICIENTS HAVE BEEN CALCULATED. */
    if (it0 != *itpv) {
      /* LOADS COORDINATE AND PARTIAL DERIVATIVE VALUES AT THE */
      /* VERTEXES. */
      jipt = (it0 - 1) * 3;
      jpd = 0;
      for (i = 1; i <= 3; ++i) {
        ++jipt;
        idp = ipt[jipt-1];
        x[i - 1] = xd[idp-1];
        y[i - 1] = yd[idp-1];
        z[i - 1] = zd[idp-1];
        jpdd = (idp - 1) * 5;
        for (kpd = 1; kpd <= 5; ++kpd) {
          ++jpd;
          ++jpdd;
          pd[jpd - 1] = pdd[jpdd-1];
          /* L20: */
        }
        /* L10: */
      }
      /* DETERMINES THE COEFFICIENTS FOR THE COORDINATE SYSTEM */
      /* TRANSFORMATION FROM THE X-Y SYSTEM TO THE U-V SYSTEM */
      /* AND VICE VERSA. */
      x0 = x[0];
      y0 = y[0];
      a = x[1] - x0;
      b = x[2] - x0;
      c = y[1] - y0;
      d = y[2] - y0;
      ad = a * d;
      bc = b * c;
      dlt = ad - bc;
      ap = d / dlt;
      bp = -b / dlt;
      cp = -c / dlt;
      dp = a / dlt;
      /* CONVERTS THE PARTIAL DERIVATIVES AT THE VERTEXES OF THE */
      /* TRIANGLE FOR THE U-V COORDINATE SYSTEM. */
      aa = a * a;
      act2 = a * 2. * c;
      cc = c * c;
      ab = a * b;
      adbc = ad + bc;
      cd = c * d;
      bb = b * b;
      bdt2 = b * 2. * d;
      dd = d * d;
      for (i = 1; i <= 3; ++i) {
        jpd = i * 5;
        zu[i - 1] = a * pd[jpd - 5] + c * pd[jpd - 4];
        zv[i - 1] = b * pd[jpd - 5] + d * pd[jpd - 4];
        zuu[i - 1] = aa * pd[jpd - 3] + act2 * pd[jpd - 2] + cc * pd[jpd - 1];
        zuv[i - 1] = ab * pd[jpd - 3] + adbc * pd[jpd - 2] + cd * pd[jpd - 1];
        zvv[i - 1] = bb * pd[jpd - 3] + bdt2 * pd[jpd - 2] + dd * pd[jpd - 1];
        /* L30: */
      }
      /* CALCULATES THE COEFFICIENTS OF THE POLYNOMIAL. */
      p00 = z[0];
      p10 = zu[0];
      p01 = zv[0];
      p20 = zuu[0] * .5;
      p11 = zuv[0];
      p02 = zvv[0] * .5;
      h1 = z[1] - p00 - p10 - p20;
      h2 = zu[1] - p10 - zuu[0];
      h3 = zuu[1] - zuu[0];
      p30 = h1 * 10. - h2 * 4. + h3 * .5;
      p40 = h1 * -15. + h2 * 7. - h3;
      p50 = h1 * 6. - h2 * 3. + h3 * .5;
      p5 = p50;
      h1 = z[2] - p00 - p01 - p02;
      h2 = zv[2] - p01 - zvv[0];
      h3 = zvv[2] - zvv[0];
      p03 = h1 * 10. - h2 * 4. + h3 * .5;
      p04 = h1 * -15. + h2 * 7. - h3;
      p05 = h1 * 6. - h2 * 3. + h3 * .5;
      lu = sqrt(aa + cc);
      lv = sqrt(bb + dd);
      thxu = atan2(c, a);
      thuv = atan2(d, b) - thxu;
      csuv = cos(thuv);
      p41 = lv * 5. * csuv / lu * p50;
      p14 = lu * 5. * csuv / lv * p05;
      h1 = zv[1] - p01 - p11 - p41;
      h2 = zuv[1] - p11 - p41 * 4.;
      p21 = h1 * 3. - h2;
      p31 = h1 * -2. + h2;
      h1 = zu[2] - p10 - p11 - p14;
      h2 = zuv[2] - p11 - p14 * 4.;
      p12 = h1 * 3. - h2;
      p13 = h1 * -2. + h2;
      thus = atan2(d - c, b - a) - thxu;
      thsv = thuv - thus;
      aa = sin(thsv) / lu;
      bb = -cos(thsv) / lu;
      cc = sin(thus) / lv;
      dd = cos(thus) / lv;
      ac = aa * cc;
      ad = aa * dd;
      bc = bb * cc;
      g1 = aa * ac * (bc * 3. + ad * 2.);
      g2 = cc * ac * (ad * 3. + bc * 2.);
      h1 = -aa * aa * aa * (aa * 5. * bb * p50 + (bc * 4. + ad) * p41) -
            cc * cc * cc * (cc * 5. * dd * p05 + (ad * 4. + bc) * p14);
      h2 = zvv[1] * .5 - p02 - p12;
      h3 = zuu[2] * .5 - p20 - p21;
      p22 = (g1 * h2 + g2 * h3 - h1) / (g1 + g2);
      p32 = h2 - p22;
      p23 = h3 - p22;
      *itpv = it0;
    }
    /* CONVERTS XII AND YII TO U-V SYSTEM. */
    dx = *xii - x0;
    dy = *yii - y0;
    u = ap * dx + bp * dy;
    v = cp * dx + dp * dy;
    /* EVALUATES THE POLYNOMIAL. */
    p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
    p1 = p10 + v * (p11 + v * (p12 + v * (p13 + v * p14)));
    p2 = p20 + v * (p21 + v * (p22 + v * p23));
    p3 = p30 + v * (p31 + v * p32);
    p4 = p40 + v * p41;
    *zii = p0 + u * (p1 + u * (p2 + u * (p3 + u * (p4 + u * p5))));
  } else {
    il1 = it0 / ntl;
    il2 = it0 - il1 * ntl;
    if (il1 == il2) {
      /* CALCULATION OF ZII BY EXTRAPOLATION IN THE RECTANGLE. */
      /* CHECKS IF THE NECESSARY COEFFICIENTS HAVE BEEN CALCULATED. */
      if (it0 != *itpv) {
        /* LOADS COORDINATE AND PARTIAL DERIVATIVE VALUES AT THE END */
        /* POINTS OF THE BORDER LINE SEGMENT. */
        jipl = (il1 - 1) * 3;
        jpd = 0;
        for (i = 1; i <= 2; ++i) {
          ++jipl;
          idp = ipl[jipl-1];
          x[i - 1] = xd[idp-1];
          y[i - 1] = yd[idp-1];
          z[i - 1] = zd[idp-1];
          jpdd = (idp - 1) * 5;
          for (kpd = 1; kpd <= 5; ++kpd) {
            ++jpd;
            ++jpdd;
            pd[jpd - 1] = pdd[jpdd-1];
            /* L50: */
          }
          /* L40: */
        }
        /* DETERMINES THE COEFFICIENTS FOR THE COORDINATE SYSTEM */
        /* TRANSFORMATION FROM THE X-Y SYSTEM TO THE U-V SYSTEM */
        /* AND VICE VERSA. */
        x0 = x[0];
        y0 = y[0];
        a = y[1] - y[0];
        b = x[1] - x[0];
        c = -b;
        d = a;
        ad = a * d;
        bc = b * c;
        dlt = ad - bc;
        ap = d / dlt;
        bp = -b / dlt;
        cp = -bp;
        dp = ap;
        /* CONVERTS THE PARTIAL DERIVATIVES AT THE END POINTS OF THE */
        /* BORDER LINE SEGMENT FOR THE U-V COORDINATE SYSTEM. */
        aa = a * a;
        act2 = a * 2. * c;
        cc = c * c;
        ab = a * b;
        adbc = ad + bc;
        cd = c * d;
        bb = b * b;
        bdt2 = b * 2. * d;
        dd = d * d;
        for (i = 1; i <= 2; ++i) {
          jpd = i * 5;
          zu[i - 1] = a * pd[jpd - 5] + c * pd[jpd - 4];
          zv[i - 1] = b * pd[jpd - 5] + d * pd[jpd - 4];
          zuu[i - 1] = aa * pd[jpd - 3] + act2 * pd[jpd - 2] + cc * pd[jpd - 1];
          zuv[i - 1] = ab * pd[jpd - 3] + adbc * pd[jpd - 2] + cd * pd[jpd - 1];
          zvv[i - 1] = bb * pd[jpd - 3] + bdt2 * pd[jpd - 2] + dd * pd[jpd - 1];
          /* L60: */
        }
        /* CALCULATES THE COEFFICIENTS OF THE POLYNOMIAL. */
        p00 = z[0];
        p10 = zu[0];
        p01 = zv[0];
        p20 = zuu[0] * .5;
        p11 = zuv[0];
        p02 = zvv[0] * .5;
        h1 = z[1] - p00 - p01 - p02;
        h2 = zv[1] - p01 - zvv[0];
        h3 = zvv[1] - zvv[0];
        p03 = h1 * 10. - h2 * 4. + h3 * .5;
        p04 = h1 * -15. + h2 * 7. - h3;
        p05 = h1 * 6. - h2 * 3. + h3 * .5;
        h1 = zu[1] - p10 - p11;
        h2 = zuv[1] - p11;
        p12 = h1 * 3. - h2;
        p13 = h1 * -2. + h2;
        p21 = 0.;
        p23 = -zuu[1] + zuu[0];
        p22 = p23 * -1.5;
        *itpv = it0;
      }
      /* CONVERTS XII AND YII TO U-V SYSTEM. */
      dx = *xii - x0;
      dy = *yii - y0;
      u = ap * dx + bp * dy;
      v = cp * dx + dp * dy;
      /* EVALUATES THE POLYNOMIAL. */
      p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))))
        ;
      p1 = p10 + v * (p11 + v * (p12 + v * p13));
      p2 = p20 + v * (p21 + v * (p22 + v * p23));
      *zii = p0 + u * (p1 + u * p2);
    } else {
      /* CALCULATION OF ZII BY EXTRAPOLATION IN THE TRIANGLE. */
      /* CHECKS IF THE NECESSARY COEFFICIENTS HAVE BEEN CALCULATED. */
      if (it0 != *itpv) {
        /* LOADS COORDINATE AND PARTIAL DERIVATIVE VALUES AT THE VERTEX */
        /* OF THE TRIANGLE. */
        jipl = il2 * 3 - 2;
        idp = ipl[jipl-1];
        x[0] = xd[idp-1];
        y[0] = yd[idp-1];
        z[0] = zd[idp-1];
        jpdd = (idp - 1) * 5;
        for (kpd = 1; kpd <= 5; ++kpd) {
          ++jpdd;
          pd[kpd - 1] = pdd[jpdd-1];
          /* L70: */
        }
        /* CALCULATES THE COEFFICIENTS OF THE POLYNOMIAL. */
        p00 = z[0];
        p10 = pd[0];
        p01 = pd[1];
        p20 = pd[2] * .5;
        p11 = pd[3];
        p02 = pd[4] * .5;
        *itpv = it0;
      }
      /* CONVERTS XII AND YII TO U-V SYSTEM. */
      u = *xii - x[0];
      v = *yii - y[0];
      /* EVALUATES THE POLYNOMIAL. */
      p0 = p00 + v * (p01 + v * p02);
      p1 = p10 + v * p11;
      *zii = p0 + u * (p1 + u * p20);
    }
  }
  return 0;
}

static
int idxchg(double *x, double *y, int *i1, int *i2, int *i3, int *i4)
{
  Integer ret_val;
  Real r1, r2;
  Real a1sq, b1sq, c1sq, c2sq, a3sq, b2sq, b3sq, a4sq, b4sq, a2sq, c4sq, c3sq;
  Real u1, u2, u3, x1, y1, x2, y2, x3, y3, x4, y4, u4;
  Integer idx;
  Real s1sq, s2sq, s3sq, s4sq;

  /* THIS FUNCTION DETERMINES WHETHER OR NOT THE EXCHANGE OF TWO */
  /* TRIANGLES IS NECESSARY ON THE BASIS OF MAX-MIN-ANGLE CRITERION */
  /* BY C. L. LAWSON. */
  /* THE INPUT PARAMETERS ARE */
  /*     X,Y = ARRAYS CONTAINING THE COORDINATES OF THE DATA */
  /*           POINTS, */
  /*     I1,I2,I3,I4 = POINT NUMBERS OF FOUR POINTS P1, P2, */
  /*           P3, AND P4 THAT FORM A QUADRILATERAL WITH P3 */
  /*           AND P4 CONNECTED DIAGONALLY. */
  /* THIS FUNCTION RETURNS AN INTEGER VALUE 1 (ONE) WHEN AN EX- */
  /* CHANGE IS NECESSARY, AND 0 (ZERO) OTHERWISE. */

  x1 = x[*i1-1];
  y1 = y[*i1-1];
  x2 = x[*i2-1];
  y2 = y[*i2-1];
  x3 = x[*i3-1];
  y3 = y[*i3-1];
  x4 = x[*i4-1];
  y4 = y[*i4-1];
  /* CALCULATION */
  idx = 0;
  u3 = (y2 - y3) * (x1 - x3) - (x2 - x3) * (y1 - y3);
  u4 = (y1 - y4) * (x2 - x4) - (x1 - x4) * (y2 - y4);
  if (u3 * u4 > 0.) {
    u1 = (y3 - y1) * (x4 - x1) - (x3 - x1) * (y4 - y1);
    u2 = (y4 - y2) * (x3 - x2) - (x4 - x2) * (y3 - y2);
    r1 = x1 - x3;
    r2 = y1 - y3;
    a1sq= b3sq = r1 * r1 + r2 * r2;
    r1 = x4 - x1;
    r2 = y4 - y1;
    b1sq= a4sq = r1 * r1 + r2 * r2;
    r1 = x3 - x4;
    r2 = y3 - y4;
    c1sq= c2sq = r1 * r1 + r2 * r2;
    r1 = x2 - x4;
    r2 = y2 - y4;
    a2sq= b4sq = r1 * r1 + r2 * r2;
    r1 = x3 - x2;
    r2 = y3 - y2;
    b2sq= a3sq = r1 * r1 + r2 * r2;
    r1 = x2 - x1;
    r2 = y2 - y1;
    c3sq= c4sq = r1 * r1 + r2 * r2;
    s1sq = u1 * u1 / (c1sq * max(a1sq,b1sq));
    s2sq = u2 * u2 / (c2sq * max(a2sq,b2sq));
    s3sq = u3 * u3 / (c3sq * max(a3sq,b3sq));
    s4sq = u4 * u4 / (c4sq * max(a4sq,b4sq));
    if (min(s1sq,s2sq) < min(s3sq,s4sq)) {
      idx = 1;
    }
  }
  ret_val = idx;
  return ret_val;
}

static
int idtang(int *ndp, double *xd, double *yd, int *nt,
           int *ipt, int *nl, int *ipl, int *iwl, int *iwp, double *wk)
{
  Real ratio = 1e-6f;
  Integer nrep = 100;
  Real x1, y1, ar, r1, r2;
  Integer ip, jp;
  Real dx, dy;
  Integer it, ip1, ip2, jp1, jp2, ip3, nl0, nt0, ilf, jpc;
  Real dx21, dy21;
  Integer nlf, itf[2], nln, nsh, ntf, jwl, its, ndp0, ipl1, ipl2;
  Integer jlt3, ipt1, ipt2, ipt3, nlt3, jwl1, itt3, ntt3, nlfc, ip1p1;
  Real dsq12, armn;
  Integer irep;
  Real dsqi;
  Integer jp2t3, jp3t3, jpmn;
  Real dxmn, dymn, xdmp, ydmp, armx;
  Integer ipti, it1t3, it2t3, jpmx;
  Real dxmx, dymx;
  Integer ndpm1, ilft2, iplj1, iplj2, ipmn1, ipmn2, ipti1, ipti2;
  Integer nlft2, nlnt3, nsht3, itt3r;
  Real dsqmn;
  Integer ntt3p3;
  Real dsqmx;
  Integer jwl1mn;

  /* THIS SUBROUTINE PERFORMS TRIANGULATION.  IT DIVIDES THE X-Y */
  /* PLANE INTO A NUMBER OF TRIANGLES ACCORDING TO GIVEN DATA */
  /* POINTS IN THE PLANE, DETERMINES LINE SEGMENTS THAT FORM THE */
  /* BORDER OF DATA AREA, AND DETERMINES THE TRIANGLE NUMBERS */
  /* CORRESPONDING TO THE BORDER LINE SEGMENTS. */
  /* AT COMPLETION, POINT NUMBERS OF THE VERTEXES OF EACH TRIANGLE */
  /* ARE LISTED COUNTER-CLOCKWISE.  POINT NUMBERS OF THE END POINTS */
  /* OF EACH BORDER LINE SEGMENT ARE LISTED COUNTER-CLOCKWISE, */
  /* LISTING ORDER OF THE LINE SEGMENTS BEING COUNTER-CLOCKWISE. */
  /* THIS SUBROUTINE CALLS THE IDXCHG FUNCTION. */
  /* THE INPUT PARAMETERS ARE */
  /*     NDP = NUMBER OF DATA POINTS, */
  /*     XD  = ARRAY OF DIMENSION NDP CONTAINING THE */
  /*           X COORDINATES OF THE DATA POINTS, */
  /*     YD  = ARRAY OF DIMENSION NDP CONTAINING THE */
  /*           Y COORDINATES OF THE DATA POINTS. */
  /* THE OUTPUT PARAMETERS ARE */
  /*     NT  = NUMBER OF TRIANGLES, */
  /*     IPT = INTEGER ARRAY OF DIMENSION 6*NDP-15, WHERE THE */
  /*           POINT NUMBERS OF THE VERTEXES OF THE (IT)TH */
  /*           TRIANGLE ARE TO BE STORED AS THE (3*IT-2)ND, */
  /*           (3*IT-1)ST, AND (3*IT)TH ELEMENTS, */
  /*           IT=1,2,...,NT, */
  /*     NL  = NUMBER OF BORDER LINE SEGMENTS, */
  /*     IPL = INTEGER ARRAY OF DIMENSION 6*NDP, WHERE THE */
  /*           POINT NUMBERS OF THE END POINTS OF THE (IL)TH */
  /*           BORDER LINE SEGMENT AND ITS RESPECTIVE TRIANGLE */
  /*           NUMBER ARE TO BE STORED AS THE (3*IL-2)ND, */
  /*           (3*IL-1)ST, AND (3*IL)TH ELEMENTS, */
  /*           IL=1,2,..., NL. */
  /* THE OTHER PARAMETERS ARE */
  /*     IWL = INTEGER ARRAY OF DIMENSION 18*NDP USED */
  /*           INTERNALLY AS A WORK AREA, */
  /*     IWP = INTEGER ARRAY OF DIMENSION NDP USED */
  /*           INTERNALLY AS A WORK AREA, */
  /*     WK  = ARRAY OF DIMENSION NDP USED INTERNALLY AS A */
  /*           WORK AREA. */

  /* PRELIMINARY PROCESSING */
  nlnt3 = 0;
  nln = 0;
  ndp0 = *ndp;
  ndpm1 = ndp0 - 1;
  if (ndp0 < 4) {
    /* ERROR EXIT */
    fprintf(stderr, " ***   NDP LESS THAN 4.\n"
            "   NDP =%5d\n", ndp0);
  } else {
    /* DETERMINES THE CLOSEST PAIR OF DATA POINTS AND THEIR MIDPOINT. */
    r1 = xd[1] - xd[0];
    r2 = yd[1] - yd[0];
    dsqmn = r1 * r1 + r2 * r2;
    ipmn1 = 1;
    ipmn2 = 2;
    for (ip1 = 1; ip1 <= ndpm1; ++ip1) {
      x1 = xd[ip1-1];
      y1 = yd[ip1-1];
      ip1p1 = ip1 + 1;
      for (ip2 = ip1p1; ip2 <= ndp0; ++ip2) {
        r1 = xd[ip2-1] - x1;
        r2 = yd[ip2-1] - y1;
        dsqi = r1 * r1 + r2 * r2;
        if (fabs(dsqi) <= 1e-6f) {
          goto L30;
        } else if (dsqi < dsqmn) {
          dsqmn = dsqi;
          ipmn1 = ip1;
          ipmn2 = ip2;
        }
        /* L20: */
      }
      /* L10: */
    }
    dsq12 = dsqmn;
    xdmp = (xd[ipmn1-1] + xd[ipmn2-1]) / 2.;
    ydmp = (yd[ipmn1-1] + yd[ipmn2-1]) / 2.;
    /* SORTS THE OTHER (NDP-2) DATA POINTS IN ASCENDING ORDER OF */
    /* DISTANCE FROM THE MIDPOINT AND STORES THE SORTED DATA POINT */
    /* NUMBERS IN THE IWP ARRAY. */
    jp1 = 2;
    for (ip1 = 1; ip1 <= ndp0; ++ip1) {
      if (ip1 != ipmn1 && ip1 != ipmn2) {
        ++jp1;
        iwp[jp1-1] = ip1;
        r1 = xd[ip1-1] - xdmp;
        r2 = yd[ip1-1] - ydmp;
        wk[jp1-1] = r1 * r1 + r2 * r2;
      }
      /* L40: */
    }
    for (jp1 = 3; jp1 <= ndpm1; ++jp1) {
      dsqmn = wk[jp1-1];
      jpmn = jp1;
      for (jp2 = jp1; jp2 <= ndp0; ++jp2) {
        if (wk[jp2-1] < dsqmn) {
          dsqmn = wk[jp2-1];
          jpmn = jp2;
        }
        /* L60: */
      }
      its = iwp[jp1-1];
      iwp[jp1-1] = iwp[jpmn-1];
      iwp[jpmn-1] = its;
      wk[jpmn-1] = wk[jp1-1];
      /* L50: */
    }
    /* IF NECESSARY, MODIFIES THE ORDERING IN SUCH A WAY THAT THE */
    /* FIRST THREE DATA POINTS ARE NOT COLLINEAR. */
    ar = dsq12 * ratio;
    x1 = xd[ipmn1-1];
    y1 = yd[ipmn1-1];
    dx21 = xd[ipmn2-1] - x1;
    dy21 = yd[ipmn2-1] - y1;
    for (jp = 3; jp <= ndp0; ++jp) {
      ip = iwp[jp-1];
      if ((r1 = (yd[ip-1] - y1) * dx21 - (xd[ip-1] - x1) * dy21,
           fabs(r1)) > ar) {
        goto L80;
      }
      /* L70: */
    }
    fprintf(stderr, " ***   ALL COLLINEAR DATA POINTS.\n"
            "   NDP =%5d\n", ndp0);
    goto L90;
  L80:
    if (jp != 3) {
      jpmx = jp;
      jp = jpmx + 1;
      for (jpc = 4; jpc <= jpmx; ++jpc) {
        --jp;
        iwp[jp-1] = iwp[jp-2];
        /* L100: */
      }
      iwp[2] = ip;
    }
    /* FORMS THE FIRST TRIANGLE.  STORES POINT NUMBERS OF THE VER- */
    /* TEXES OF THE TRIANGLE IN THE IPT ARRAY, AND STORES POINT NUM- */
    /* BERS OF THE BORDER LINE SEGMENTS AND THE TRIANGLE NUMBER IN */
    /* THE IPL ARRAY. */
    ip1 = ipmn1;
    ip2 = ipmn2;
    ip3 = iwp[2];
    if ((yd[ip3-1] - yd[ip1-1]) * (xd[ip2-1] - xd[ip1-1]) -
        (xd[ip3-1] - xd[ip1-1]) * (yd[ip2-1] - yd[ip1-1]) < 0.) {
      ip1 = ipmn2;
      ip2 = ipmn1;
    }
    nt0 = 1;
    ntt3 = 3;
    ipt[0] = ip1;
    ipt[1] = ip2;
    ipt[2] = ip3;
    nl0 = 3;
    nlt3 = 9;
    ipl[0] = ip1;
    ipl[1] = ip2;
    ipl[2] = 1;
    ipl[3] = ip2;
    ipl[4] = ip3;
    ipl[5] = 1;
    ipl[6] = ip3;
    ipl[7] = ip1;
    ipl[8] = 1;
    /* ADDS THE REMAINING (NDP-3) DATA POINTS, ONE BY ONE. */
    for (jp1 = 4; jp1 <= ndp0; ++jp1) {
      ip1 = iwp[jp1-1];
      x1 = xd[ip1-1];
      y1 = yd[ip1-1];
      /* - DETERMINES THE VISIBLE BORDER LINE SEGMENTS. */
      ip2 = ipl[0];
      jpmn = 1;
      dxmn = xd[ip2-1] - x1;
      dymn = yd[ip2-1] - y1;
      r1 = dxmn;
      r2 = dymn;
      dsqmn = r1 * r1 + r2 * r2;
      armn = dsqmn * ratio;
      jpmx = 1;
      dxmx = dxmn;
      dymx = dymn;
      dsqmx = dsqmn;
      armx = armn;
      for (jp2 = 2; jp2 <= nl0; ++jp2) {
        ip2 = ipl[jp2 * 3 - 3];
        dx = xd[ip2-1] - x1;
        dy = yd[ip2-1] - y1;
        ar = dy * dxmn - dx * dymn;
        if (ar <= armn) {
          r1 = dx;
          r2 = dy;
          dsqi = r1 * r1 + r2 * r2;
          if (ar < -armn || dsqi < dsqmn) {
            jpmn = jp2;
            dxmn = dx;
            dymn = dy;
            dsqmn = dsqi;
            armn = dsqmn * ratio;
          }
        }
        ar = dy * dxmx - dx * dymx;
        if (ar >= -armx) {
          r1 = dx;
          r2 = dy;
          dsqi = r1 * r1 + r2 * r2;
          if (ar > armx || dsqi < dsqmx) {
            jpmx = jp2;
            dxmx = dx;
            dymx = dy;
            dsqmx = dsqi;
            armx = dsqmx * ratio;
          }
        }
        /* L120: */
      }
      if (jpmx < jpmn) {
        jpmx += nl0;
      }
      nsh = jpmn - 1;
      if (nsh > 0) {
        /* - SHIFTS (ROTATES) THE IPL ARRAY TO HAVE THE INVISIBLE BORDER */
        /* - LINE SEGMENTS CONTAINED IN THE FIRST PART OF THE IPL ARRAY. */
        nsht3 = nsh * 3;
        for (jp2t3 = 3; jp2t3 <= nsht3; jp2t3 += 3) {
          jp3t3 = jp2t3 + nlt3;
          ipl[jp3t3-3] = ipl[jp2t3-3];
          ipl[jp3t3-2] = ipl[jp2t3-2];
          ipl[jp3t3-1] = ipl[jp2t3-1];
          /* L130: */
        }
        for (jp2t3 = 3; jp2t3 <= nlt3; jp2t3 += 3) {
          jp3t3 = jp2t3 + nsht3;
          ipl[jp2t3-3] = ipl[jp3t3-3];
          ipl[jp2t3-2] = ipl[jp3t3-2];
          ipl[jp2t3-1] = ipl[jp3t3-1];
          /* L140: */
        }
        jpmx -= nsh;
      }
      /* - ADDS TRIANGLES TO THE IPT ARRAY, UPDATES BORDER LINE */
      /* - SEGMENTS IN THE IPL ARRAY, AND SETS FLAGS FOR THE BORDER */
      /* - LINE SEGMENTS TO BE REEXAMINED IN THE IWL ARRAY. */
      jwl = 0;
      for (jp2 = jpmx; jp2 <= nl0; ++jp2) {
        jp2t3 = jp2 * 3;
        ipl1 = ipl[jp2t3-3];
        ipl2 = ipl[jp2t3-2];
        it = ipl[jp2t3-1];
        /* - - ADDS A TRIANGLE TO THE IPT ARRAY. */
        ++nt0;
        ntt3 += 3;
        ipt[ntt3-3] = ipl2;
        ipt[ntt3-2] = ipl1;
        ipt[ntt3-1] = ip1;
        /* - - UPDATES BORDER LINE SEGMENTS IN THE IPL ARRAY. */
        if (jp2 == jpmx) {
          ipl[jp2t3-2] = ip1;
          ipl[jp2t3-1] = nt0;
        }
        if (jp2 == nl0) {
          nln = jpmx + 1;
          nlnt3 = nln * 3;
          ipl[nlnt3-3] = ip1;
          ipl[nlnt3-2] = ipl[0];
          ipl[nlnt3-1] = nt0;
        }
        /* - - DETERMINES THE VERTEX THAT DOES NOT LIE ON THE BORDER */
        /* - - LINE SEGMENTS. */
        itt3 = it * 3;
        ipti = ipt[itt3-3];
        if (ipti == ipl1 || ipti == ipl2) {
          ipti = ipt[itt3-2];
          if (ipti == ipl1 || ipti == ipl2) {
            ipti = ipt[itt3-1];
          }
        }
        /* - - CHECKS IF THE EXCHANGE IS NECESSARY. */
        if (idxchg(&xd[0], &yd[0], &ip1, &ipti, &ipl1, &ipl2) != 0) {
          /* - - MODIFIES THE IPT ARRAY WHEN NECESSARY. */
          ipt[itt3 - 3] = ipti;
          ipt[itt3 - 2] = ipl1;
          ipt[itt3-1] = ip1;
          ipt[ntt3 - 2] = ipti;
          if (jp2 == jpmx) {
            ipl[jp2t3-1] = it;
          }
          if (jp2 == nl0 && ipl[2] == it) {
            ipl[2] = nt0;
          }
          /* - - SETS FLAGS IN THE IWL ARRAY. */
          jwl += 4;
          iwl[jwl-4] = ipl1;
          iwl[jwl-3] = ipti;
          iwl[jwl-2] = ipti;
          iwl[jwl-1] = ipl2;
        }
        /* L150: */
      }
      nl0 = nln;
      nlt3 = nlnt3;
      nlf = jwl / 2;
      if (nlf != 0) {
        /* - IMPROVES TRIANGULATION. */
        ntt3p3 = ntt3 + 3;
        for (irep = 1; irep <= nrep; ++irep) {
          for (ilf = 1; ilf <= nlf; ++ilf) {
            ilft2 = ilf * 2;
            ipl1 = iwl[ilft2-2];
            ipl2 = iwl[ilft2-1];
            /* - - LOCATES IN THE IPT ARRAY TWO TRIANGLES ON BOTH SIDES OF */
            /* - - THE FLAGGED LINE SEGMENT. */
            ntf = 0;
            for (itt3r = 3; itt3r <= ntt3; itt3r += 3) {
              itt3 = ntt3p3 - itt3r;
              ipt1 = ipt[itt3-3];
              ipt2 = ipt[itt3-2];
              ipt3 = ipt[itt3-1];
              if (ipl1 == ipt1 || ipl1 == ipt2 || ipl1 == ipt3)
                {
                  if (ipl2 == ipt1 || ipl2 == ipt2 || ipl2 == ipt3) {
                    ++ntf;
                    itf[ntf - 1] = itt3 / 3;
                    if (ntf == 2) {
                      goto L190;
                    }
                  }
                }
              /* L180: */
            }
            if (ntf < 2) {
              goto L170;
            }
            /* - - DETERMINES THE VERTEXES OF THE TRIANGLES THAT DO NOT LIE */
            /* - - ON THE LINE SEGMENT. */
          L190:
            it1t3 = itf[0] * 3;
            ipti1 = ipt[it1t3-3];
            if (ipti1 == ipl1 || ipti1 == ipl2) {
              ipti1 = ipt[it1t3-2];
              if (ipti1 == ipl1 || ipti1 == ipl2) {
                ipti1 = ipt[it1t3-1];
              }
            }
            it2t3 = itf[1] * 3;
            ipti2 = ipt[it2t3-3];
            if (ipti2 == ipl1 || ipti2 == ipl2) {
              ipti2 = ipt[it2t3-2];
              if (ipti2 == ipl1 || ipti2 == ipl2) {
                ipti2 = ipt[it2t3-1];
              }
            }
            /* - - CHECKS IF THE EXCHANGE IS NECESSARY. */
            if (idxchg(&xd[0], &yd[0], &ipti1, &ipti2, &ipl1, &ipl2) != 0) {
              /* - - MODIFIES THE IPT ARRAY WHEN NECESSARY. */
              ipt[it1t3-3] = ipti1;
              ipt[it1t3-2] = ipti2;
              ipt[it1t3-1] = ipl1;
              ipt[it2t3-3] = ipti2;
              ipt[it2t3-2] = ipti1;
              ipt[it2t3-1] = ipl2;
              /* - - SETS NEW FLAGS. */
              jwl += 8;
              iwl[jwl - 8] = ipl1;
              iwl[jwl - 7] = ipti1;
              iwl[jwl - 6] = ipti1;
              iwl[jwl - 5] = ipl2;
              iwl[jwl - 4] = ipl2;
              iwl[jwl - 3] = ipti2;
              iwl[jwl - 2] = ipti2;
              iwl[jwl - 1] = ipl1;
              for (jlt3 = 3; jlt3 <= nlt3; jlt3 += 3) {
                iplj1 = ipl[jlt3 - 3];
                iplj2 = ipl[jlt3 - 2];
                if ((iplj1 == ipl1 && iplj2 == ipti2) ||
                    (iplj2 == ipl1 && iplj1 == ipti2)) {
                  ipl[jlt3-1] = itf[0];
                }
                if ((iplj1 == ipl2 && iplj2 == ipti1) ||
                    (iplj2 == ipl2 && iplj1 == ipti1)) {
                  ipl[jlt3-1] = itf[1];
                }
                /* L200: */
              }
            }
          L170:
            ;
          }
          nlfc = nlf;
          nlf = jwl / 2;
          if (nlf == nlfc) {
            goto L110;
          } else {
            /* - - RESETS THE IWL ARRAY FOR THE NEXT ROUND. */
            jwl = 0;
            jwl1mn = (nlfc + 1) * 2;
            nlft2 = nlf * 2;
            for (jwl1 = jwl1mn; jwl1 <= nlft2; jwl1 += 2) {
              jwl += 2;
              iwl[jwl - 2] = iwl[jwl1 - 2];
              iwl[jwl-1] = iwl[jwl1-1];
              /* L210: */
            }
            nlf = jwl / 2;
          }
          /* L160: */
        }
      }
    L110:
      ;
    }
    /* REARRANGES THE IPT ARRAY SO THAT THE VERTEXES OF EACH TRIANGLE */
    /* ARE LISTED COUNTER-CLOCKWISE. */
    for (itt3 = 3; itt3 <= ntt3; itt3 += 3) {
      ip1 = ipt[itt3 - 3];
      ip2 = ipt[itt3 - 2];
      ip3 = ipt[itt3-1];
      if ((yd[ip3-1] - yd[ip1-1]) * (xd[ip2-1] - xd[ip1-1]) -
          (xd[ip3-1] - xd[ip1-1]) * (yd[ip2-1] - yd[ip1-1]) < 0.) {
        ipt[itt3 - 3] = ip2;
        ipt[itt3 - 2] = ip1;
      }
      /* L220: */
    }
    *nt = nt0;
    *nl = nl0;
    return 0;
  L30:
    fprintf(stderr, " ***   IDENTICAL DATA POINTS.\n"
            "   NDP =%5d   IP1 =%5d   IP2 =%5d   XD=%g   YD=%g\n",
            ndp0, ip1, ip2, x1, y1);
  }
 L90:
  fprintf(stderr, " ERROR DETECTED IN ROUTINE   IDTANG.\n");
  *nt = 0;
  return 0;
}

static
int idlin(double *xd, double *yd, double *zd, int *nt, int *iwk, double *wk)
{
  Real x1, y1, z1, x2, y2, z2, x3, y3, z3;
  Integer ip1, ip2, ip3, itri, ipoint;

  /* THIS ROUTINE GENERATES THE COORDINATES USED IN A LINEAR INTERPOLATION */
  /* OF THE TRIANGLES CREATED FROM IRREGULARLY DISTRIBUTED DATA. */
  /*   LOOP FOR ALL TRIANGLES */
  /* Parameter adjustments */

  for (itri = 1; itri <= *nt; ++itri) {
    /* GET THE POINTS OF THE TRIANGLE */
    ipoint = (itri - 1) * 3;
    ip1 = iwk[ipoint]-1;
    ip2 = iwk[ipoint + 1]-1;
    ip3 = iwk[ipoint + 2]-1;
    /* GET THE VALUES AT THE TRIANBGLE POINTS */
    x1 = xd[ip1];
    y1 = yd[ip1];
    z1 = zd[ip1];
    x2 = xd[ip2];
    y2 = yd[ip2];
    z2 = zd[ip2];
    x3 = xd[ip3];
    y3 = yd[ip3];
    z3 = zd[ip3];
    /* COMPUTE THE INTERPLOATING COEFICIENTS */
    wk[ipoint] = (y2 - y1) * (z3 - z1) - (y3 - y1) * (z2 - z1);
    wk[ipoint + 1] = (x3 - x1) * (z2 - z1) - (x2 - x1) * (z3 - z1);
    wk[ipoint + 2] = (x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1);
    /* L10: */
  }
  return 0;
}

static
int idlcom(double *x, double *y, double *z, int *itri,
           double *xd, double *yd, double *zd, int *nt, int *iwk, double *wk)
{
  Real x1, y1, z1;
  Integer iv, ipoint;

  /* COMPUTE A Z VALUE FOR A GIVEN X,Y VALUE */
  /* IF OUTSIDE CONVEX HULL DON'T COMPUTE A VALUE */

  if (*itri <= *nt) {
    ipoint = (*itri - 1) * 3;
    iv = iwk[ipoint]-1;
    x1 = *x - xd[iv];
    y1 = *y - yd[iv];
    z1 = zd[iv];
    /* COMPUTE THE Z VALUE */
    *z = (wk[ipoint] * x1 + wk[ipoint + 1] * y1) / wk[ipoint + 2] + z1;
  }
  return 0;
}

void idsfft(int *md, int *ncp, int *ndp, double *xd, double *yd, double *zd,
            int *nxi, int *nyi, double *xi, double *yi, double *zi,
            int *iwk, double *wk)
{
  Integer nl, nt, md0, il1, il2, iti, ixi, izi, iyi, ncp0, ndp0;
  Integer ngp0, ngp1, nxi0, nyi0, jigp, jngp, nngp, itpv;
  Integer jwipc, jwigp, jwipl, ncppv, ndppv, jwngp, jwiwl, jwipt;
  Integer jwiwp, nxipv, nyipv, jig0mn, jig1mn, jig0mx, jig1mx, jwigp0, jwngp0;
  Integer linear;

  /* THIS SUBROUTINE PERFORMS SMOOTH SURFACE FITTING WHEN THE PRO- */
  /* JECTIONS OF THE DATA POINTS IN THE X-Y PLANE ARE IRREGULARLY */
  /* DISTRIBUTED IN THE PLANE. */
  /* THE INPUT PARAMETERS ARE */
  /*     MD  = MODE OF COMPUTATION (MUST BE 1, 2, OR 3), */
  /*         = 1 FOR NEW NCP AND/OR NEW XD-YD, */
  /*         = 2 FOR OLD NCP, OLD XD-YD, NEW XI-YI, */
  /*         = 3 FOR OLD NCP, OLD XD-YD, OLD XI-YI, */
  /*     NCP = NUMBER OF ADDITIONAL DATA POINTS USED FOR ESTI- */
  /*           MATING PARTIAL DERIVATIVES AT EACH DATA POINT */
  /*           (MUST BE 2 OR GREATER, BUT SMALLER THAN NDP), */
  /*     NDP = NUMBER OF DATA POINTS (MUST BE 4 OR GREATER), */
  /*     XD  = ARRAY OF DIMENSION NDP CONTAINING THE X */
  /*           COORDINATES OF THE DATA POINTS, */
  /*     YD  = ARRAY OF DIMENSION NDP CONTAINING THE Y */
  /*           COORDINATES OF THE DATA POINTS, */
  /*     ZD  = ARRAY OF DIMENSION NDP CONTAINING THE Z */
  /*           COORDINATES OF THE DATA POINTS, */
  /*     NXI = NUMBER OF OUTPUT GRID POINTS IN THE X COORDINATE */
  /*           (MUST BE 1 OR GREATER), */
  /*     NYI = NUMBER OF OUTPUT GRID POINTS IN THE Y COORDINATE */
  /*           (MUST BE 1 OR GREATER), */
  /*     XI  = ARRAY OF DIMENSION NXI CONTAINING THE X */
  /*           COORDINATES OF THE OUTPUT GRID POINTS, */
  /*     YI  = ARRAY OF DIMENSION NYI CONTAINING THE Y */
  /*           COORDINATES OF THE OUTPUT GRID POINTS. */
  /* THE OUTPUT PARAMETER IS */
  /*     ZI  = DOUBLY-DIMENSIONED ARRAY OF DIMENSION (NXI,NYI), */
  /*           WHERE THE INTERPOLATED Z VALUES AT THE OUTPUT */
  /*           GRID POINTS ARE TO BE STORED. */
  /* THE OTHER PARAMETERS ARE */
  /*     IWK = INTEGER ARRAY OF DIMENSION */
  /*              MAX0(31,27+NCP)*NDP+NXI*NYI */
  /*           USED INTERNALLY AS A WORK AREA, */
  /*     WK  = ARRAY OF DIMENSION 6*(NDP+1) USED INTERNALLY AS A */
  /*           WORK AREA. */
  /* THE VERY FIRST CALL TO THIS SUBROUTINE AND THE CALL WITH A NEW */
  /* NCP VALUE, A NEW NDP VALUE, AND/OR NEW CONTENTS OF THE XD AND */
  /* YD ARRAYS MUST BE MADE WITH MD=1.  THE CALL WITH MD=2 MUST BE */
  /* PRECEDED BY ANOTHER CALL WITH THE SAME NCP AND NDP VALUES AND */
  /* WITH THE SAME CONTENTS OF THE XD AND YD ARRAYS.  THE CALL WITH */
  /* MD=3 MUST BE PRECEDED BY ANOTHER CALL WITH THE SAME NCP, NDP, */
  /* NXI, AND NYI VALUES AND WITH THE SAME CONTENTS OF THE XD, YD, */
  /* XI, AND YI ARRAYS.  BETWEEN THE CALL WITH MD=2 OR MD=3 AND ITS */
  /* PRECEDING CALL, THE IWK AND WK ARRAYS MUST NOT BE DISTURBED. */
  /* USE OF A VALUE BETWEEN 3 AND 5 (INCLUSIVE) FOR NCP IS RECOM- */
  /* MENDED UNLESS THERE ARE EVIDENCES THAT DICTATE OTHERWISE. */
  /* THIS SUBROUTINE CALLS THE IDCLDP, IDGRID, IDPDRV, IDPTIP, AND */
  /* IDTANG SUBROUTINES. */

  md0 = *md;
  ncp0 = *ncp;
  ndp0 = *ndp;
  nxi0 = *nxi;
  nyi0 = *nyi;
  linear = 0;
  if (*ndp > 100) {
    linear = 1;
  }
  /* ERROR CHECK.  (FOR MD=1,2,3) */
  if (md0 >= 1 && md0 <= 3) {
    if (ncp0 >= 2 && ncp0 < ndp0) {
      if (ndp0 >= 4) {
        if (nxi0 >= 1 && nyi0 >= 1) {
          if (md0 >= 2) {
            ncppv = iwk[0];
            ndppv = iwk[1];
            if (ncp0 != ncppv) {
              goto L10;
            } else if (ndp0 != ndppv) {
              goto L10;
            }
          } else {
            iwk[0] = ncp0;
            iwk[1] = ndp0;
          }
          if (md0 >= 3) {
            nxipv = iwk[2];
            nyipv = iwk[3];
            if (nxi0 != nxipv) {
              goto L10;
            } else if (nyi0 != nyipv) {
              goto L10;
            }
          } else {
            iwk[2] = nxi0;
            iwk[3] = nyi0;
          }
          /* ALLOCATION OF STORAGE AREAS IN THE IWK ARRAY.  (FOR MD=1,2,3) */
          jwipt = 16;
          jwiwl = ndp0 * 6 + 1;
          jwngp0 = jwiwl - 1;
          jwipl = ndp0 * 24 + 1;
          jwiwp = ndp0 * 30 + 1;
          jwipc = ndp0 * 27 + 1;
          jwigp0 = max(31,ncp0 + 27) * ndp0;
          /* TRIANGULATES THE X-Y PLANE.  (FOR MD=1) */
          if (md0 <= 1) {
            idtang(&ndp0, &xd[0], &yd[0], &nt, &iwk[jwipt-1], &nl,
                   &iwk[jwipl-1], &iwk[jwiwl-1], &iwk[jwiwp-1], &wk[0])
              ;
            iwk[4] = nt;
            iwk[5] = nl;
            if (nt == 0) {
              return;
            }
          }
          /* DETERMINES NCP POINTS CLOSEST TO EACH DATA POINT.  (FOR MD=1) */
          if (md0 <= 1) {
            idcldp(&ndp0, &xd[0], &yd[0], &ncp0, &iwk[jwipc-1]);
            if (iwk[jwipc-1] == 0) {
              return;
            }
          }
          /* SORTS OUTPUT GRID POINTS IN ASCENDING ORDER OF THE TRIANGLE */
          /* NUMBER AND THE BORDER LINE SEGMENT NUMBER.  (FOR MD=1,2) */
          if (md0 != 3) {
            idgrid(&xd[0], &yd[0], &nt, &iwk[jwipt-1], &nl, &iwk[jwipl-1],
                   &nxi0, &nyi0, &xi[0], &yi[0], &iwk[jwngp0], &iwk[jwigp0]);
          }
          if (linear) {
            /* FIND THE COEFFICENTS FOR LINER INTERPOLATION OF EACH TRIANGLE */
            idlin(&xd[0], &yd[0], &zd[0], &nt, &iwk[jwipt-1], &wk[0]);
          } else {
            /* ESTIMATES PARTIAL DERIVATIVES AT ALL DATA POINTS. */
            /* (FOR MD=1,2,3) */
            idpdrv(&ndp0, &xd[0], &yd[0], &zd[0], &ncp0, &iwk[jwipc-1], &wk[0]);
          }
          /* INTERPOLATES THE ZI VALUES.  (FOR MD=1,2,3) */
          itpv = 0;
          jig0mx = 0;
          jig1mn = nxi0 * nyi0 + 1;
          nngp = nt + 2 * nl;
          for (jngp = 1; jngp <= nngp; ++jngp) {
            iti = jngp;
            if (jngp > nt) {
              il1 = (jngp - nt + 1) / 2;
              il2 = (jngp - nt + 2) / 2;
              if (il2 > nl) {
                il2 = 1;
              }
              iti = il1 * (nt + nl) + il2;
            }
            jwngp = jwngp0 + jngp;
            ngp0 = iwk[jwngp-1];
            if (ngp0 != 0) {
              jig0mn = jig0mx + 1;
              jig0mx += ngp0;
              for (jigp = jig0mn; jigp <= jig0mx; ++jigp) {
                jwigp = jwigp0 + jigp;
                izi = iwk[jwigp-1];
                iyi = (izi - 1) / nxi0 + 1;
                ixi = izi - nxi0 * (iyi - 1);
                if (linear) {
                  idlcom(&xi[ixi-1], &yi[iyi-1], &zi[izi-1],
                         &iti, &xd[0], &yd[0], &zd[0], &nt,
                         &iwk[jwipt-1], &wk[0]);
                } else {
                  idptip(&xd[0], &yd[0], &zd[0], &nt,
                         &iwk[jwipt-1], &nl, &iwk[jwipl-1], &wk[0],
                         &iti, &xi[ixi-1], &yi[iyi-1], &zi[izi-1],
                         &itpv);
                }
                /* L30: */
              }
            }
            jwngp = jwngp0 + 2 * nngp + 1 - jngp;
            ngp1 = iwk[jwngp-1];
            if (ngp1 != 0) {
              jig1mx = jig1mn - 1;
              jig1mn -= ngp1;
              for (jigp = jig1mn; jigp <= jig1mx; ++jigp) {
                jwigp = jwigp0 + jigp;
                izi = iwk[jwigp-1];
                iyi = (izi - 1) / nxi0 + 1;
                ixi = izi - nxi0 * (iyi - 1);
                if (linear) {
                  idlcom(&xi[ixi-1], &yi[iyi-1], &zi[izi-1],
                         &iti, &xd[0], &yd[0], &zd[0], &nt,
                         &iwk[jwipt-1], &wk[0]);
                } else {
                  idptip(&xd[0], &yd[0], &zd[0], &nt,
                         &iwk[jwipt-1], &nl, &iwk[jwipl-1], &wk[0],
                         &iti, &xi[ixi-1], &yi[iyi-1], &zi[izi-1],
                         &itpv);
                }
                /* L40: */
              }
            }
            /* L20: */
          }
          return;
        }
      }
    }
  }
  /* ERROR EXIT */
 L10:
  fprintf(stderr, " ***   IMPROPER INPUT PARAMETER VALUE(S)."
          "   MD =%4d   NCP =%6d   NDP = %6d   NXI =%6d   NYI =%6d\n",
          md0, ncp0, ndp0, nxi0, nyi0);
}
