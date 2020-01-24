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
        context.setTransform(1, 0, 0, 1, 0, 0);
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
        context.clearRect(0, 0, context.canvas.width, context.canvas.height);
    },

    js_get_ws_width: function() {
        return Module.canvas.width;
    },

    js_get_ws_height: function() {
        return Module.canvas.height;
    }
});
