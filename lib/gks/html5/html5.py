import sys
from math import sqrt, pi, atan2
import zlib
import struct
import gks
from constants import *

fonts = [
    "Times New Roman", "Arial", "Courier", "Open Symbol",
    "Bookman Old Style", "Century Schoolbook", "Century Gothic",
    "Book Antiqua"
]

capheights = [
    0.662, 0.660, 0.681, 0.662,
    0.729, 0.729, 0.729, 0.729,
    0.583, 0.583, 0.583, 0.583,
    0.667,
    0.681, 0.681, 0.681, 0.681,
    0.722, 0.722, 0.722, 0.722,
    0.739, 0.739, 0.739, 0.739,
    0.694, 0.693, 0.683, 0.683
]

font_map = [
    22, 9, 5, 14, 18, 26, 13, 1,
    24, 11, 7, 16, 20, 28, 13, 3,
    23, 10, 6, 15, 19, 27, 13, 2,
    25, 12, 8, 17, 21, 29, 13, 4
]

predef_styli = [1, 1, 1, 2, 3]
predef_ints = [0, 1, 3, 3, 3]

a = [1.0 for i in range(MAX_TNR)]
b = [0.0 for i in range(MAX_TNR)]
c = [1.0 for i in range(MAX_TNR)]
d = [0.0 for i in range(MAX_TNR)]


class ws_state_list(object):
    def __init__(self):
        self.state = None
        self.a = None
        self.b = None
        self.c = None
        self.d = None
        self.width = None
        self.height = None
        self.window = None
        self.viewport = None
        self.rgb = None
        self.strokeStyle = None
        self.fillStyle = None
        self.lineWidth = None
        self.dashes = None
        self.valign = None
        self.halign = None
        self.font = None
        self.rect = None
        self.clip_rect = None


class Html_output(object):
    def __init__(self, gks_state_list, data, n):
        if n >= 250:
            print("Maximum number of HTML output files reached")
            sys.exit(0)
        if n > 1:
            self.filename = 'gks-%d.html' % (n)
        else:
            self.filename = 'gks.html'
        self.title = 'GKS'
        self.gkss = gks_state_list
        self.p = ws_state_list()
        self.file = open(self.filename, 'w')
        self.p.width = 500
        self.p.height = 500
        self.p.window = [0.0, 1.0, 0.0, 1.0]
        self.p.viewport = [self.p.width * MWIDTH / WIDTH, 0.,
                           self.p.height * MHEIGHT / HEIGHT]
        self.p.rgb = [[] for i in range(MAX_COLOR)]
        self.p.dashes = []
        self.indentation = 0
        self.p.rect = [() for i in range(MAX_TNR)]
        self.p.clip_rect = ()
        self.transparency = 1.0

        self.set_xform()
        self.init_norm_xform()
        self.init_colors()

        if not self.file:
            print("Can't open HTML output file")

        self.write('<!DOCTYPE html>\n')
        self.write('<html>\n')
        self.write('  <head>\n')
        self.write('    <meta charset="utf-8" />\n')
        self.write('    <title>{0}</title>\n'.format(self.title))
        self.write('  </head>\n')
        self.write('  <script src="modernizr.js"></script>\n')
        self.write('  <body>\n')
        self.write(
            '    <canvas id="html-canvas" width="{0}" height="{1}" tabindex="1" style="outline-style:none;"></canvas>\n'.format(
                self.p.width, self.p.height))
        self.write('\n')
        self.write('<script src="gks.js"></script>\n')
        self.write('<script type="text/javascript">\n')
        self.indentation = 1
        self.init_canvas()

        gks.set_fill_callback(self.fill_routine)
        gks.set_line_callback(self.line_routine)

        self.write('base_width = canvas.width;\n')
        self.write('base_height = canvas.height;\n')

        self.write('function draw() {\n')

        self.footer = '''  }
  draw();
</script>
<div id="buttons"></div>
<script>
    if (Modernizr.touch) {
        var container = document.getElementById("buttons");
            container.innerHTML = "<table border='0'><tr><td><button onclick='zoom_in();'>Zoom In</button></td><td><button onclick='zoom_out();'>Zoom Out</button></td><td><button onclick='reset_zoom();'>Reset</button></td></tr></table>";
        }
</script>
</body>
</html>
'''
        for d in data:
            getattr(self, d[0])(*d[1])

        self.file.write(self.footer)
        self.file.close()

    def text(self, xst, yst, n, text):

        tx_font = self.gkss.txfont if self.gkss.asf[6] else predef_font[self.gkss.tindex - 1]
        tx_prec = self.gkss.txprec if self.gkss.asf[6] else predef_prec[self.gkss.tindex - 1]
        tx_color = self.gkss.txcoli if self.gkss.asf[9] else 1
        color = self.p.rgb[tx_color]
        color.append(self.transparency)

        if self.gkss.version > 4:
            ln_width = round(self.p.height / 500.0)
            if ln_width < 1:
                ln_width = 1
        else:
            ln_width = 1
        if ln_width < 1:
            ln_width = 1
        if self.p.fillStyle != color:
            self.p.fillStyle = color
            self.write('c.fillStyle="rgba({0},{1},{2},{3})";\n'.format(*color))
        if self.p.strokeStyle != color:
            self.p.strokeStyle = color
            self.write('c.strokeStyle="rgba({0},{1},{2},{3})";\n'.format(*color))
        if tx_prec == GKS_K_TEXT_PRECISION_STRING:
            self.set_font(tx_font)
            (x, y) = self.WC_to_NDC(xst, yst, self.gkss.cntnr)
            (x, y) = self.seg_xform(x, y)
            self.text_routine(x, y, n, text)
        else:
            gks.emul_text(xst, yst, text)

    def text_routine(self, x, y, nchars, text):
        (xs, ys) = self.NDC_to_DC(x, y)

        halign = self.gkss.txal[0]
        valign = self.gkss.txal[1]

        if self.p.valign != valign:
            self.p.valign = valign
            if valign == GKS_K_TEXT_VALIGN_NORMAL:
                self.write('c.textBaseline="alphabetic";\n')
            elif valign == GKS_K_TEXT_VALIGN_TOP:
                self.write('c.textBaseline="top";\n')
            elif valign == GKS_K_TEXT_VALIGN_CAP:
                self.write('c.textBaseline="hanging";\n')
            elif valign == GKS_K_TEXT_VALIGN_HALF:
                self.write('c.textBaseline="middle";\n')
            elif valign == GKS_K_TEXT_VALIGN_BASE:
                self.write('c.textBaseline="alphabetic";\n')
            elif valign == GKS_K_TEXT_VALIGN_BOTTOM:
                self.write('c.textBaseline="bottom";\n')

        if self.p.halign != halign:
            if halign == GKS_K_TEXT_HALIGN_NORMAL:
                self.write('c.textAlign="left";\n')
            elif halign == GKS_K_TEXT_HALIGN_LEFT:
                self.write('c.textAlign="left";\n')
            elif halign == GKS_K_TEXT_HALIGN_CENTER:
                self.write('c.textAlign="center";\n')
            elif halign == GKS_K_TEXT_HALIGN_RIGHT:
                self.write('c.textAlign="right";\n')

        if self.p.alpha > 0:
            self.write('c.save();\n')
            self.write('c.translate({0}, {1});\n'.format(xs, ys))
            self.write('c.rotate({0});\n'.format(-self.p.alpha))
            self.write('c.fillText("{0}", 0, 0);\n'.format(text))
            self.write('c.restore();\n')
        else:
            self.write('c.fillText("{0}", {1}, {2});\n'.format(text, xs, ys))

    def cellarray(self, xmin, xmax, ymin, ymax, dx, dy, dimx, colia):
        self.image_routine(xmin, xmax, ymin, ymax, dx, dy, dimx, colia, False)

    def draw_image(self, xmin, xmax, ymin, ymax, dx, dy, dimx, colia):
        self.image_routine(xmin, xmax, ymin, ymax, dx, dy, dimx, colia, True)

    def write_png(self, buf, width, height):
        width_byte_4 = width * 4
        raw_data = b"".join(b'\x00' + bytes(bytearray(buf[span:span + width_byte_4])) for span in range(0, height * width * 4, width_byte_4))
        def png_pack(png_tag, data):
            chunk_head = png_tag + data
            return struct.pack("!I", len(data)) + chunk_head + struct.pack("!I", 0xFFFFFFFF & zlib.crc32(chunk_head))
        return b"".join([
            b'\x89PNG\r\n\x1a\n',
            png_pack(b'IHDR', struct.pack("!2I5B", width, height, 8, 6, 0, 0, 0)),
            png_pack(b'IDAT', zlib.compress(raw_data, 9)),
            png_pack(b'IEND', b'')])

    def image_routine(self, xmin, xmax, ymin, ymax, dx, dy, dimx, colia, true_color):
        (x1, y1) = self.WC_to_NDC(xmin, ymax, self.gkss.cntnr)
        (x1, y1) = self.seg_xform(x1, y1)
        (ix1, iy1) = self.NDC_to_DC(x1, y1)

        (x2, y2) = self.WC_to_NDC(xmax, ymin, self.gkss.cntnr)
        (x2, y2) = self.seg_xform(x2, y2)
        (ix2, iy2) = self.NDC_to_DC(x2, y2)

        pix_buf = []

        ix1 = int(ix1)
        iy1 = int(iy1)
        ix2 = int(ix2)
        iy2 = int(iy2)

        width = abs(ix2 - ix1)
        height = abs(iy2 - iy1)
        if width == 0 or height == 0:
            return

        x = min(ix1, ix2)
        y = min(iy1, iy2)

        swapx = ix1 > ix2
        swapy = iy1 < iy2

        for j in range(height):
            iy = dy * j / height
            if swapy:
                iy = dy - 1 - iy
            for i in range(width):
                ix = dx * i / width
                if swapx:
                    ix = dx - 1 - ix
                if not true_color:
                    ci = colia[iy * dimx + ix]
                    (red, green, blue) = self.p.rgb[ci][:3]
                    alpha = int(self.transparency * 255)
                else:
                    rgb = colia[iy * dimx + ix]
                    red = (rgb & 0xff)
                    green = (rgb & 0xff00) >> 8
                    blue = (rgb & 0xff0000) >> 16
                    alpha = (rgb & 0xff000000) >> 24

                pix_buf.append(red)
                pix_buf.append(green)
                pix_buf.append(blue)
                pix_buf.append(alpha)

        png = self.write_png(pix_buf, width, height)

        import base64
        enc_png = base64.b64encode(png)
        data_uri = 'data:image/png;base64, {0}'.format(enc_png)
        self.write('var imageObj = new Image();\n')
        self.write('imageObj.onload = function() {\n')
        self.indentation += 1
        self.write('c.drawImage(this, {0}, {1});\n'.format(x, y))

        self.footer = (self.indentation - 1) * '  ' + 'imageObj.src = "{0}";\n'.format(data_uri) + self.footer
        self.footer = (self.indentation - 1) * '  ' + '};\n' + self.footer

    def fillarea(self, n, px, py):
        fl_color = self.gkss.facoli if self.gkss.asf[12] else 1
        ln_width = max(1, round(self.p.height / 500.0)) if self.gkss.version > 4 else 1
        if self.p.lineWidth != ln_width:
            self.p.lineWidth = ln_width
            self.write('c.lineWidth = {0};\n'.format(ln_width))
        color = self.p.rgb[fl_color]
        color.append(self.transparency)
        if self.p.fillStyle != color:
            self.p.fillStyle = color
            self.write('c.fillStyle="rgba({0},{1},{2},{3})";\n'.format(*color))
        self.fill_routine(n, px, py, self.gkss.cntnr)

    def fill_routine(self, n, px, py, tnr):
        self.write('c.beginPath();\n')

        if self.p.dashes != []:
            self.p.dashes = []
            self.write('set_dashes(c, []);\n')

        (xn, yn) = self.WC_to_NDC(px[0], py[0], self.gkss.cntnr)
        (xn, yn) = self.seg_xform(xn, yn)
        (xd, yd) = self.NDC_to_DC(xn, yn)
        self.write('c.moveTo({0}, {1});\n'.format(xd, yd))
        for i in range(1, n):
            (xn, yn) = self.WC_to_NDC(px[i], py[i], self.gkss.cntnr)
            (xn, yn) = self.seg_xform(xn, yn)
            (xd, yd) = self.NDC_to_DC(xn, yn)
            self.write('c.lineTo({0}, {1});\n'.format(xd, yd))
        self.write('c.closePath();\n')

        fl_inter = self.gkss.ints if self.gkss.asf[10] else predef_ints[self.gkss.findex - 1]
        if fl_inter == GKS_K_INTSTYLE_PATTERN or fl_inter == GKS_K_INTSTYLE_HATCH:
            fl_style = self.gkss.styli if self.gkss.asf[11] else predef_styli[self.gkss.findex - 1]
            if fl_inter == GKS_K_INTSTYLE_HATCH:
                fl_style += HATCH_STYLE
            if fl_style >= PATTERNS:
                fl_style = 1
            pattern = gks.inq_pattern_array(fl_style)
            self.write('var pcan = document.createElement("canvas");\n')
            self.write('pcan.width = 8;\n')
            self.write('pcan.height = {0};\n'.format(len(pattern)))
            self.write('var pctx = pcan.getContext("2d");\n')
            if self.p.fillStyle != [0, 0, 0]:
                self.write('c.fillStyle="rgba({0},{1},{2},{3})";\n'.format(*self.p.fillStyle))
            for j in range(len(pattern)):
                for i in range(8):
                    a = (1 << i) & pattern[j]
                    if not a:
                        self.write(
                            'pctx.rect({0}, {1}, 1, 1);\n'.format((i + 7) % 8, (j + (len(pattern) - 1)) % len(pattern)))
            self.write('pctx.fill();\n')
            self.write('var pattern = c.createPattern(pcan, "repeat");\n')
            self.write('c.fillStyle = pattern;\n')
            self.write('c.fill();\n')
        elif fl_inter == GKS_K_INTSTYLE_SOLID:
            self.write('c.fill();\n')
        else:
            self.write('c.stroke();\n')

    def set_window(self, tnr, xmin, xmax, ymin, ymax):
        self.gkss.window[tnr][0] = xmin
        self.gkss.window[tnr][1] = xmax
        self.gkss.window[tnr][2] = ymin
        self.gkss.window[tnr][3] = ymax

        self.set_xform()
        self.set_norm_xform(tnr, self.gkss.window[tnr], self.gkss.viewport[tnr])

    def set_viewport(self, tnr, xmin, xmax, ymin, ymax):
        self.gkss.viewport[tnr][0] = xmin
        self.gkss.viewport[tnr][1] = xmax
        self.gkss.viewport[tnr][2] = ymin
        self.gkss.viewport[tnr][3] = ymax

        self.set_xform()
        self.set_norm_xform(tnr, self.gkss.window[tnr], self.gkss.viewport[tnr])
        if tnr == self.gkss.cntnr:
            self.set_clip_rect(tnr)

    def set_asf(self, *asf):
        for i in range(13):
            self.gkss.asf[i] = asf[i]

    def polyline(self, n, px, py):
        ln_width = self.gkss.lwidth if self.gkss.asf[1] else 1
        ln_type = self.gkss.ltype if self.gkss.asf[0] else self.gkss.lindex
        ln_color = self.gkss.plcoli if self.gkss.asf[2] else 1

        if self.gkss.version > 4:
            ln_width *= self.p.height / 500.0
        if ln_color <= 0 or ln_color >= MAX_COLOR:
            ln_color = 1
        ln_width = max(1, round(ln_width))

        color = self.p.rgb[ln_color]
        color.append(self.transparency)
        if self.p.strokeStyle != color:
            self.p.strokeStyle = color
            self.write('c.strokeStyle="rgba({0},{1},{2},{3})";\n'.format(*color))
        if self.p.lineWidth != ln_width:
            self.p.lineWidth = ln_width
            self.write('c.lineWidth = {0};\n'.format(ln_width))

        self.write('c.beginPath();\n')
        dashes = gks.get_dash_list(ln_type, ln_width)
        if self.p.dashes != dashes:
            self.p.dashes = dashes
            self.write('set_dashes(c, {0});\n'.format(str(dashes)))

        (xn, yn) = self.WC_to_NDC(px[0], py[0], self.gkss.cntnr)
        (xn, yn) = self.seg_xform(xn, yn)
        (xd, yd) = self.NDC_to_DC(xn, yn)
        self.write('c.moveTo({0}, {1});\n'.format(xd, yd))
        for i in range(1, n):
            (xn, yn) = self.WC_to_NDC(px[i], py[i], self.gkss.cntnr)
            (xn, yn) = self.seg_xform(xn, yn)
            (xd, yd) = self.NDC_to_DC(xn, yn)
            self.write('c.lineTo({0}, {1});\n'.format(xd, yd))
        self.write('c.stroke();\n')

    def line_routine(self, n, px, py, ltype, tnr):
        (xn, yn) = self.WC_to_NDC(px[0], py[0], self.gkss.cntnr)
        (xn, yn) = self.seg_xform(xn, yn)
        (xd, yd) = self.NDC_to_DC(xn, yn)
        self.write('c.beginPath();\n')
        self.write('c.moveTo({0}, {1});\n'.format(xd, yd))
        for i in range(1, n):
            (xn, yn) = self.WC_to_NDC(px[i], py[i], self.gkss.cntnr)
            (xn, yn) = self.seg_xform(xn, yn)
            (xd, yd) = self.NDC_to_DC(xn, yn)
            self.write('c.lineTo({0}, {1});\n'.format(xd, yd))
        self.write('c.stroke();\n')

    def init_canvas(self):
        self.write('var canvas=document.getElementById("html-canvas");\n')
        self.write('var c=canvas.getContext("2d");\n')

    def draw_point(self, x, y):
        self.write('c.fillRect({0}, {1}, 1, 1);\n'.format(x, y))

    def draw_line(self, x1, y1, x2, y2):
        self.write('c.beginPath();\n')
        self.write('c.moveTo({0}, {1});\n'.format(x1, y1))
        self.write('c.lineTo({0}, {1});\n'.format(x2, y2))
        self.write('c.stroke();\n')

    def draw_marker(self, xn, yn, mtype, mscale, mcolor):
        marker = [
            [5, 9, -4, 7, 4, 7, 7, 4, 7, -4,  # omark
             4, -7, -4, -7, -7, -4, -7, 4,
             -4, 7, 3, 9, -4, 7, 4, 7, 7, 4,
             7, -4, 4, -7, -4, -7, -7, -4,
             -7, 4, -4, 7, 0],
            [5, 13, -2, 8, 2, 8, 2, 2, 8, 2,  # hollow plus
             8, -2, 2, -2, 2, -8, -2, -8,
             -2, -2, -8, -2, -8, 2, -2, 2,
             -2, 8, 3, 13, -2, 8, 2, 8,
             2, 2, 8, 2, 8, -2, 2, -2, 2, -8,
             -2, -8, -2, -2, -8, -2, -8, 2,
             -2, 2, -2, 8, 0],
            [4, 4, -8, 0, 4, 7, 4, -7,  # solid triangle right
             -8, 0, 0],
            [4, 4, 8, 0, -4, -7, -4, 7,  # solid triangle left
             8, 0, 0],
            [5, 4, 0, 8, 7, -4, -7, -4, 0, 8,  # triangle up down
             5, 4, 0, -8, -7, 4, 7, 4, 0, -8,
             3, 4, 0, 8, 7, -4, -7, -4, 0, 8,
             3, 4, 0, -8, -7, 4, 7, 4, 0, -8,
             0],
            [4, 11, 0, 9, 2, 2, 9, 3, 3, -1,  # solid star
             6, -8, 0, -3, -6, -8, -3, -1,
             -9, 3, -2, 2, 0, 9, 0],
            [5, 11, 0, 9, 2, 2, 9, 3, 3, -1,  # hollow star
             6, -8, 0, -3, -6, -8, -3, -1,
             -9, 3, -2, 2, 0, 9,
             3, 11, 0, 9, 2, 2, 9, 3, 3, -1,
             6, -8, 0, -3, -6, -8, -3, -1,
             -9, 3, -2, 2, 0, 9, 0],
            [4, 5, 0, 9, 9, 0, 0, -9, -9, 0,  # solid diamond
             0, 9, 0],
            [5, 5, 0, 9, 9, 0, 0, -9, -9, 0,  # hollow diamond
             0, 9, 3, 5, 0, 9, 9, 0, 0, -9,
             -9, 0, 0, 9, 0],
            [4, 5, 9, 9, -9, -9, 9, -9, -9, 9,  # solid hourglass
             9, 9, 0],
            [5, 5, 9, 9, -9, -9, 9, -9, -9, 9,  # hollow hourglass
             9, 9, 3, 5, 9, 9, -9, -9, 9, -9,
             -9, 9, 9, 9, 0],
            [4, 5, 9, 9, 9, -9, -9, 9, -9, -9,  # solid bowtie
             9, 9, 0],
            [5, 5, 9, 9, 9, -9, -9, 9, -9, -9,  # hollow bowtie
             9, 9, 3, 5, 9, 9, 9, -9, -9, 9,
             -9, -9, 9, 9, 0],
            [4, 5, 9, 9, 9, -9, -9, -9, -9, 9,  # solid square
             9, 9, 0],
            [5, 5, 9, 9, 9, -9, -9, -9, -9, 9,  # hollow square
             9, 9, 3, 5, 9, 9, 9, -9, -9, -9,
             -9, 9, 9, 9, 0],
            [4, 4, -9, 9, 9, 9, 0, -9, -9, 9,  # solid triangle down
             0],
            [5, 4, -9, 9, 9, 9, 0, -9, -9, 9,  # hollow triangle down
             3, 4, -9, 9, 9, 9, 0, -9, -9, 9,
             0],
            [4, 4, 0, 9, 9, -9, -9, -9, 0, 9,  # solid triangle up
             0],
            [5, 4, 0, 9, 9, -9, -9, -9, 0, 9,  # hollow triangle up
             3, 4, 0, 9, 9, -9, -9, -9, 0, 9, 0],
            [7, 0, 360, 0],  # solid circle
            [0],  # not used
            [1, 0],  # dot
            [2, 0, 0, 0, 9, 2, 0, 0, 9, 0,  # plus
             2, 0, 0, 0, -9, 2, 0, 0, -9, 0,
             0],
            [2, 0, 0, 0, 9, 2, 0, 0, 9, 3,  # asterisk
             2, 0, 0, 6, -9, 2, 0, 0, -6, -9,
             2, 0, 0, -9, 3, 0],
            [8, 0, 360, 6, 0, 360, 0],  # circle
            [2, 0, 0, 9, 9, 2, 0, 0, 9, -9,  # diagonal cross
             2, 0, 0, -9, -9, 2, 0, 0, -9, 9, 0]]

        if self.gkss.version > 4:
            mscale *= self.p.height / 500.0
        r = int(3 * mscale)
        scale = mscale / 3.0

        xr = r
        yr = 0
        (xr, yr) = self.seg_xform_rel(xr, yr)
        r = round(sqrt(xr * xr + yr * yr))

        (x, y) = self.NDC_to_DC(xn, yn)

        pc = 0
        mtype = mtype + 20 if (2 * r > 1) else 21
        while (True):
            op = marker[mtype][pc]
            if op == 1:  # point
                self.draw_point(x, y)
            elif op == 2:  # line
                x1 = scale * marker[mtype][pc + 1]
                y1 = scale * marker[mtype][pc + 2]
                (x1, y1) = self.seg_xform_rel(x1, y1)

                x2 = scale * marker[mtype][pc + 2 + 1]
                y2 = scale * marker[mtype][pc + 2 + 2]
                (x2, y2) = self.seg_xform_rel(x2, y2)

                self.draw_line(x - x1, y - y1, x - x2, y - y2)
                pc += 4

            elif op in (3, 4, 5):  # 3 - polygon   4 - filled polygon   5 - hollow polygon
                xr = scale * marker[mtype][pc + 2]
                yr = scale * marker[mtype][pc + 3]
                (xr, yr) = self.seg_xform_rel(xr, yr)

                self.write('c.beginPath();\n')
                self.write('c.moveTo({0}, {1});\n'.format(x - xr, y - yr))

                for i in range(1, marker[mtype][pc + 1]):
                    xr = scale * marker[mtype][pc + 2 + 2 * i]
                    yr = scale * marker[mtype][pc + 3 + 2 * i]

                    (xr, yr) = self.seg_xform_rel(xr, yr)
                    self.write('c.lineTo({0}, {1});\n'.format(x - xr, y - yr))

                self.write('c.closePath();\n')

                if not op == 3:
                    if op == 5:
                        self.write('c.fillStyle = "#FFFFFF";\n')
                    else:
                        self.write('c.fillStyle = "#000000";\n')
                    self.write('c.fill();\n')
                    self.write('c.fillStyle = "#000000";\n')
                else:
                    self.write('c.stroke();\n')

                pc += 1 + 2 * marker[mtype][pc + 1]
            elif op in (6, 7, 8):  # 6 - arc   7 - filled arc   8 - hollow arc
                center = (x, y)

                start_angle = marker[mtype][pc + 1] * pi / 180
                end_angle = marker[mtype][pc + 2] * pi / 180

                self.write('c.beginPath();\n')
                self.draw_arc(x, y, r, start_angle, end_angle)
                self.write('c.closePath();\n')
                if not op == 6:
                    if op == 8:
                        self.write('c.fillStyle = "#FFFFFF";\n')
                    self.write('c.fill();\n')
                    self.write('c.fillStyle = "#000000";\n')
                pc += 2
            pc += 1
            if op == 0:
                break

    def set_fill_style_index(self, index):
        self.gkss.styli = index

    def set_fill_int_style(self, istyle):
        self.gkss.ints = istyle

    def set_fill_color_index(self, idx):
        self.gkss.facoli = idx

    def set_pline_linewidth(self, lwidth):
        self.gkss.lwidth = lwidth

    def set_pline_color_index(self, idx):
        self.gkss.plcoli = idx

    def set_pline_linetype(self, ltype):
        self.gkss.ltype = ltype

    def set_pmark_type(self, type):
        self.gkss.mtype = type

    def set_pmark_size(self, f):
        self.gkss.mszsc = f

    def set_pmark_color_index(self, idx):
        self.gkss.pmcoli = idx

    def draw_arc(self, x, y, r, start_angle, end_angle):
        self.write('c.beginPath();\n')
        self.write('c.arc({0},{1},{2},{3}, {4}, true);\n'.format(x, y, r, start_angle, end_angle))
        self.write('c.stroke();\n')

    def polymarker(self, n, px, py):
        if self.p.dashes != []:
            self.p.dashes = []
            self.write('set_dashes(c, []);\n')
        mk_type = self.gkss.mtype if self.gkss.asf[3] else self.gkss.mindex
        mk_size = self.gkss.mszsc if self.gkss.asf[4] else 1
        mk_color = self.gkss.pmcoli if self.gkss.asf[5] else 1
        color = self.p.rgb[mk_color]
        color.append(self.transparency)

        if self.p.fillStyle != color:
            self.p.fillStyle = color
            self.write('c.fillStyle="rgba({0},{1},{2},{3})";\n'.format(*color))
        if self.p.strokeStyle != color:
            self.p.strokeStyle = color
            self.write('c.strokeStyle="rgba({0},{1},{2},{3})";\n'.format(*color))

        ln_width = max(1, round(self.p.height / 500.0)) if self.gkss.version > 4 else 1
        if self.p.lineWidth != ln_width:
            self.p.lineWidth = ln_width
            self.write('c.lineWidth = {0};\n'.format(ln_width))

        for i in range(len(px)):
            (x, y) = self.WC_to_NDC(px[i], py[i], self.gkss.cntnr)
            (x, y) = self.seg_xform(x, y)
            self.draw_marker(x, y, mk_type, mk_size, 1)

    def seg_xform_rel(self, x, y):
        xx = x * self.gkss.mat[0][0] + y * self.gkss.mat[0][1]
        y = x * self.gkss.mat[1][0] + y * self.gkss.mat[1][1]
        return (xx, y)

    def seg_xform(self, x, y):
        xx = x * self.gkss.mat[0][0] + y * self.gkss.mat[0][1] + self.gkss.mat[2][0]
        y = x * self.gkss.mat[1][0] + y * self.gkss.mat[1][1] + self.gkss.mat[2][1]
        return (xx, y)

    def set_xform(self):
        self.p.a = (self.p.width - 1) / (self.p.window[1] - self.p.window[0])
        self.p.b = -self.p.window[0] * self.p.a
        self.p.c = (self.p.height - 1) / (self.p.window[2] - self.p.window[3])
        self.p.d = self.p.height - 1 - self.p.window[2] * self.p.c

    def set_norm_xform(self, tnr, wn, vp):
        a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0])
        b[tnr] = vp[0] - wn[0] * a[tnr]
        c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2])
        d[tnr] = vp[2] - wn[2] * c[tnr]

        (xp1, yp1) = self.NDC_to_DC(vp[0], vp[3])
        (xp2, yp2) = self.NDC_to_DC(vp[1], vp[2])

        self.p.rect[tnr] = ((xp1, yp1), (xp2, yp2))

    def set_text_fontprec(self, font, prec):
        self.gkss.txfont = font
        self.gkss.txprec = prec

    def set_text_expfac(self, exp_fac):
        self.gkss.chxp = exp_fac

    def set_text_spacing(self, sp):
        self.gkss.chsp = sp

    def set_text_color_index(self, index):
        self.gkss.txcoli = index

    def set_text_height(self, h):
        self.gkss.chh = h

    def set_text_upvec(self, x, y):
        self.gkss.chup[0] = x
        self.gkss.chup[1] = y

    def set_text_path(self, path):
        self.gkss.txp = path

    def set_text_align(self, i, j):
        self.gkss.txal[0] = i
        self.gkss.txal[1] = j

    def set_text_slant(self, slant):
        self.gkss.txslant = slant

    def set_clipping(self, clip):
        self.gkss.clip = clip
        self.set_clip_rect(self.gkss.cntnr)

    def set_clip_rect(self, tnr):
        if self.gkss.clip == GKS_K_CLIP and self.p.clip_rect == self.p.rect[tnr]:
            return

        if self.p.clip_rect:
            self.write('c.restore();\n')

        if self.gkss.clip == GKS_K_CLIP:
            self.p.clip_rect = self.p.rect[tnr]
            self.write('c.save();\n')
            self.write('c.beginPath();\n')
            self.write('c.rect({}, {}, {}, {});\n'.format(self.p.clip_rect[0][0], self.p.clip_rect[0][1],
                                                          self.p.clip_rect[1][0], self.p.clip_rect[1][1]))
            self.write('c.clip();\n')
        else:
            self.p.clip_rect = ()

    def set_font(self, font):

        font = abs(font)
        if font >= 101 and font <= 129:
            font -= 100
        elif font >= 1 and font <= 32:
            font = font_map[font - 1]
        else:
            font = 9

        (ux, uy) = self.WC_to_NDC_rel(self.gkss.chup[0], self.gkss.chup[1], self.gkss.cntnr)
        (ux, uy) = self.seg_xform_rel(ux, uy)
        self.p.alpha = -atan2(ux, uy)
        if self.p.alpha < 0:
            self.p.alpha += 2 * pi

        scale = sqrt(self.gkss.chup[0] * self.gkss.chup[0] + self.gkss.chup[1] * self.gkss.chup[1])
        ux = self.gkss.chup[0] / scale * self.gkss.chh
        uy = self.gkss.chup[1] / scale * self.gkss.chh
        (ux, uy) = self.WC_to_NDC_rel(ux, uy, self.gkss.cntnr)

        width = 0
        height = sqrt(ux * ux + uy * uy)
        (width, height) = self.seg_xform_rel(width, height)

        height = sqrt(width * width + height * height)
        capheight = round(height * (abs(self.p.c) + 1))
        self.p.capheight = round(capheight)

        fontNum = font - 1
        size = round(self.p.capheight / capheights[fontNum])

        if font > 13:
            font += 3
        self.p.family = (font - 1) / 4
        bold = 0 if (font % 4 == 1 or font % 4 == 2) else 1
        italic = (font % 4 == 2 or font % 4 == 0)

        font_str = ''

        if bold:
            font_str += 'bold '
        if italic:
            font_str += 'italic '

        font_str += str(size) + 'px '
        font_str += fonts[self.p.family]

        if self.p.font != font_str:
            self.p.font = font_str
            self.write('c.font = "{0}";\n'.format(font_str))

    def set_color_rep(self, color, red, green, blue):
        if color >= 0 and color < MAX_COLOR:
            self.p.rgb[color] = [int(255 * red), int(255 * green), int(255 * blue), 255]

    def set_transparency(self, val):
        self.write('c.globalAlpha = {0};'.format(val))
        self.transparency = val

    def init_colors(self):
        for color in range(MAX_COLOR):
            (red, green, blue) = gks.inq_rgb(color)
            self.set_color_rep(color, red, green, blue)

    def init_norm_xform(self):
        for tnr in range(MAX_TNR):
            self.set_norm_xform(tnr, self.gkss.window[tnr], self.gkss.viewport[tnr])

    def WC_to_NDC(self, xw, yw, tnr):
        xn = a[tnr] * xw + b[tnr]
        yn = c[tnr] * yw + d[tnr]
        return (xn, yn)

    def WC_to_NDC_rel(self, xw, yw, tnr):
        xn = a[tnr] * xw
        yn = c[tnr] * yw
        return (xn, yn)

    def NDC_to_DC(self, xn, yn):
        xd = self.p.a * xn + self.p.b
        yd = self.p.c * yn + self.p.d
        return (xd, yd)

    def DC_to_NDC(self, xd, yd):
        xn = (xd - self.p.b) / self.p.a
        yn = (yd - self.p.d) / self.p.c
        return (xn, yn)

    def select_xform(self, tnr):
        self.gkss.cntnr = tnr
        self.set_clip_rect(tnr)

    def write(self, s):
        self.file.write(self.indentation * '  ' + s)
