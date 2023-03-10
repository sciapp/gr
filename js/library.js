mergeInto(LibraryManager.library, {
    js_stroke: function(n, points, colia, linewidth) {
        points = Module.HEAPF64.subarray(points / 8, points / 8 + n * 2);
        var rgb = Module.HEAPU8.subarray(colia, colia + 4);
        var context = Module.context;
        context.beginPath();
        context.strokeStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + rgb[3] + ")";
        context.lineWidth = linewidth;
        context.moveTo(points[0], points[1]);
        for (var i = 1; i < n; i++) {
            context.lineTo(points[i * 2], points[i * 2 + 1]);
        }
        context.stroke();
    },

    js_pattern_routine: function(n, px, py, colia) {
        colia = Module.HEAPU8.subarray(colia, colia + 64 * 4);
        px = Module.HEAPF64.subarray(px / 8, px / 8 + n);
        py = Module.HEAPF64.subarray(py / 8, py / 8 + n);
        var context = Module.context;
        context.beginPath();
        context.moveTo(px[0], py[0]);
        for (var i = 1; i < n; i++) {
            context.lineTo(px[i], py[i]);
        }
        context.lineTo(px[0], py[0]);
        var imageData = context.createImageData(8, 8);
        imageData.data.set(colia);
        var img = document.createElement('canvas');
        img.width = imageData.width;
        img.height = imageData.height;
        img.getContext("2d").putImageData(imageData, 0, 0);
        var pattern = context.createPattern(img, "repeat");
        context.fillStyle = pattern;
        context.fill("evenodd");
    },

    js_fill_routine: function(n, px, py, colia) {
        var rgba = Module.HEAPU8.subarray(colia, colia + 4);
        px = Module.HEAPF64.subarray(px / 8, px / 8 + n);
        py = Module.HEAPF64.subarray(py / 8, py / 8 + n);
        var context = Module.context;
        context.beginPath();
        context.moveTo(px[0], py[0]);
        for (var i = 1; i < n; i++) {
            context.lineTo(px[i], py[i]);
        }
        context.lineTo(px[0], py[0]);
        context.fillStyle = "rgba(" + rgba[0] + "," + rgba[1] + "," + rgba[2] + "," + rgba[3] + ")";
        context.fill("evenodd");
    },

    js_cellarray: function(x, y, width, height, colia) {
        var context = Module.context;
        context.beginPath();
        colia = Module.HEAPU8.subarray(colia, colia + width * height * 4);
        var imageData = context.createImageData(width, height);
        imageData.data.set(colia);
        var img = document.createElement('canvas');
        img.width = width;
        img.height = height;
        img.getContext("2d").putImageData(imageData, 0, 0);
        context.drawImage(img, x, y);
    },

    js_text: function(x, y, n, chars, height, top, angle, bold, italic, align, valign, font, colia) {
        var context = Module.context;
        var rgb = Module.HEAPU8.subarray(colia, colia + 3);
        context.beginPath();
        context.fillStyle = "rgb(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + ")";
        var strboit = "";
        if (bold && italic) {
            strboit = "bold italic";
        } else if (italic) {
            strboit = "italic";
        } else if (bold) {
            strboit = "bold";
        }
        var fonts = [
            '"Times New Roman", Times, serif',
            'Helvetica, Arial, sans-serif',
            'Courier, monospace',
            'Symbol',
            '"Bookman Old Style", serif',
            '"Century Schoolbook", serif',
            '"Century Gothic", sans-serif',
            '"Palatino Linotype", "Book Antiqua", Palatino, serif'
        ];
        context.font = strboit + " " + height + "px " + fonts[font];
        var valg = 0;
        if (valign == 1) {
            valg = 1.2;
        } else if (valign == 2) {
            valg = 1;
        } else if (valign == 3) {
            valg = 0.5;
        } else if (valign == 5) {
            valg = -0.2;
        }
        context.translate(x, y);
        context.rotate(angle * Math.PI / 180);
        if (align == 1) {
            context.textAlign = "center";
        } else if (align == 2) {
            context.textAlign = "right";
        } else {
            context.textAlign = "left";
        }
        var text = UTF8ToString(chars);
        context.fillText(text, 0, top * context.canvas.height * valg);
        context.setTransform(Module.dpr, 0, 0, Module.dpr, 0, 0);
    },

    js_line_routine: function(n, px, py, linetype, fill, width, rgb) {
        px = Module.HEAPF64.subarray(px / 8, px / 8 + n);
        py = Module.HEAPF64.subarray(py / 8, py / 8 + n);
        rgb = Module.HEAPU8.subarray(rgb, rgb + 4);
        var context = Module.context;
        context.beginPath();
        context.setLineDash(Module.get_dash_list(linetype));
        context.strokeStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + rgb[3] + ")";
        context.fillStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + rgb[3] + ")";
        context.lineWidth = width;
        context.moveTo(px[0], py[0]);
        var nan_found = false;
        for (var i = 1; i < n; i++) {
          if (Number.isNaN(px[i]) && Number.isNaN(py[i])) {
            nan_found = true;
            continue;
          }
          if (nan_found) {
            nan_found = false;
            if (linetype == 0) {
                context.closePath();
            }
            context.moveTo(px[i], py[i]);
          } else {
            context.lineTo(px[i], py[i]);
          }
        }
        if (linetype == 0) {
            context.closePath();
        }
        context.stroke();
        if (fill != 0) {
            context.fill("evenodd");
        }
    },

    js_point: function(x, y, colia) {
        var rgb = Module.HEAPU8.subarray(colia, colia + 3);
        var context = Module.context;
        context.beginPath();
        context.fillStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + 255 + ")";
        context.fillRect(x, y, 1, 1);
    },

    js_line: function(x1, y1, x2, y2, colia) {
        var rgb = Module.HEAPU8.subarray(colia, colia + 3);
        var context = Module.context;
        context.beginPath();
        context.lineWidth = 1;
        context.strokeStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + 255 + ")";
        context.moveTo(x1, y1);
        context.lineTo(x2, y2);
        context.stroke();
    },

    js_circle: function(x, y, r, fill, colia) {
        var rgb = Module.HEAPU8.subarray(colia, colia + 3);
        var context = Module.context;
        context.beginPath();
        context.lineWidth = 1;
        context.fillStyle = "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + 255 + ")";
        context.arc(x, y, r, 0, 2 * Math.PI);
        if (fill == 1) {
            context.fill("evenodd");
        } else {
            context.stroke();
        }
    },

    js_clip_path: function(x, y, width, height) {
        var context = Module.context;
        context.restore();
        context.save();
        context.beginPath();
        context.rect(x, y, width, height);
        context.clip();
    },

    js_reset_clipping: function() {
        var context = Module.context;
        context.restore();
        context.save();
    },

    js_clear: function() {
        var context = Module.context;
        context.restore();
        context.save();
        context.clearRect(0, 0, parseInt(Module.canvas.width / Module.dpr, 10), parseInt(Module.canvas.height / Module.dpr, 10));
    },

    js_get_ws_width: function() {
      return parseInt(Module.canvas.width / Module.dpr, 10);
    },

    js_get_ws_height: function() {
        return parseInt(Module.canvas.height / Module.dpr, 10);
    },

    js_draw_path: function(n, px, py, nc, codes, bcoli, facoli, linewidth, to_DC_) {
        px = Module.HEAPF64.subarray(px / 8, px / 8 + n);
        py = Module.HEAPF64.subarray(py / 8, py / 8 + n);
        codes = Module.HEAPU32.subarray(codes / 4, codes / 4 + nc);
        facoli = Module.HEAPU8.subarray(facoli, facoli + 4);
        bcoli = Module.HEAPU8.subarray(bcoli, bcoli + 4);

        var i, j;
        var x = new Array(3), y = new Array(3), w, h, a1, a2;
        var cur_x = 0, cur_y = 0;
        var start_x = 0, start_y = 0;
        var context = Module.context;

        to_DC = function(n, x, y) {
            var x_ = _malloc(x.length * 8);
            var y_ = _malloc(y.length * 8);
            Module.HEAPF64.set(x, x_ / 8);
            Module.HEAPF64.set(y, y_ / 8);
            dynCall('viii', to_DC_, [n, x_, y_]);
            x__ = Module.HEAPF64.subarray(x_ / 8, x_ / 8 + x.length);
            y__ = Module.HEAPF64.subarray(y_ / 8, y_ / 8 + y.length);
            for(var i = 0; i < n; ++i) {
                x[i] = x__[i];
                y[i] = y__[i];
            }
            _free(x_);
            _free(y_);
        };

        context.beginPath();
        context.strokeStyle = "rgba(" + bcoli[0] + "," + bcoli[1] + "," + bcoli[2] + "," + bcoli[3] + ")";
        context.fillStyle = "rgba(" + facoli[0] + "," + facoli[1] + "," + facoli[2] + "," + facoli[3] + ")";
        context.lineWidth = linewidth;

        j = 0;
        for (i = 0; i < nc; ++i)
          {
            var code = String.fromCharCode(codes[i]);
            switch (code) {
              case 'M':
              case 'm':
                x[0] = px[j];
                y[0] = py[j];
                if (code == 'm') {
                    x[0] += cur_x;
                    y[0] += cur_y;
                }
                cur_x = start_x = x[0];
                cur_y = start_y = y[0];
                to_DC(1, x, y);
                context.moveTo(x[0], y[0]);
                j += 1;
                break;
              case 'L':
              case 'l':
                x[0] = px[j];
                y[0] = py[j];
                if (code == 'l') {
                    x[0] += cur_x;
                    y[0] += cur_y;
                }
                cur_x = x[0];
                cur_y = y[0];
                to_DC(1, x, y);
                context.lineTo(x[0], y[0]);
                j += 1;
                break;
              case 'Q':
              case 'q':
                  x[0] = px[j];
                  y[0] = py[j];
                  if (code == 'q') {
                      x[0] += cur_x;
                      y[0] += cur_y;
                  }
                  x[1] = px[j + 1];
                  y[1] = py[j + 1];
                  if (code == 'q') {
                      x[1] += cur_x;
                      y[1] += cur_y;
                  }
                  cur_x = x[1];
                  cur_y = y[1];
                  to_DC(2, x, y);
                  context.quadraticCurveTo(x[0], y[0], x[1], y[1]);
                  j += 2;
                  break;
              case 'C':
              case 'c':
                  x[0] = px[j];
                  y[0] = py[j];
                  if (code == 'c') {
                      x[0] += cur_x;
                      y[0] += cur_y;
                  }
                  x[1] = px[j + 1];
                  y[1] = py[j + 1];
                  if (code == 'c') {
                      x[1] += cur_x;
                      y[1] += cur_y;
                  }
                  x[2] = px[j + 2];
                  y[2] = py[j + 2];
                  if (code == 'c') {
                      x[2] += cur_x;
                      y[2] += cur_y;
                  }
                  cur_x = x[2];
                  cur_y = y[2];
                  to_DC(3, x, y);
                  context.bezierCurveTo(x[0], y[0], x[1], y[1], x[2], y[2]);
                  j += 3;
                  break;
              case 'A':
              case 'a':
                  {
                      var rx = Math.abs(px[j]);
                      var ry = Math.abs(py[j]);
                      a1 = px[j + 1];
                      a2 = py[j + 1];
                      var cx = cur_x - rx * Math.cos(a1);
                      var cy = cur_y - ry * Math.sin(a1);
                      x[0] = cx - rx;
                      y[0] = cy - ry;
                      x[1] = cx + rx;
                      y[1] = cy + ry;
                      cur_x = cx + rx * Math.cos(a2);
                      cur_y = cy + ry * Math.sin(a2);
                  }
                  to_DC(2, x, y);
                  w = x[1] - x[0];
                  h = y[1] - y[0];
                  var anticlockwise = a1 < a2;
                  // Use negative angles since the canvas uses a swapped y axis
                  context.ellipse(x[0] + 0.5 * w, y[0] + 0.5 * h, Math.abs(w * 0.5), Math.abs(h * 0.5), 0, -a1, -a2, anticlockwise);
                  j += 3;
                  break;
              case 's': /* close and stroke */
                  context.closePath();
                  cur_x = start_x;
                  cur_y = start_y;
                  context.stroke();
                  break;
              case 'S': /* stroke */
                  context.stroke();
                  break;
              case 'F': /* fill (even-odd rule) and stroke */
              case 'G': /* fill (winding rule) and stroke */
                  context.closePath();
                  cur_x = start_x;
                  cur_y = start_y;
                  context.fill(code === "F" ? "evenodd" : "nonzero");
                  context.stroke();
                  break;
              case 'f': /* fill (even-odd rule) */
              case 'g': /* fill (winding rule) */
                  context.closePath();
                  cur_x = start_x;
                  cur_y = start_y;
                  context.fill(code === "f" ? "evenodd" : "nonzero");
                  break;
              case 'Z': /* closepath */
                  context.closePath();
                  cur_x = start_x;
                  cur_y = start_y;
                  break;
              case '\0':
                  break;
              default:
                  console.log("invalid path code ('" + code + "')");
                  return;
            }
        }
    }
});
