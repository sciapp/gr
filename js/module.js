var Module = {
    preRun: [],
    postRun: [],
    set_canvas: (function(canvas_id) {
        this.canvas;

        if (typeof canvas_id == "undefined") {
            console.log("gr.js: no canvas name given. Will use default canvas with id 'canvas'");

            canvas_id = "canvas";
        }
        this.canvas = document.getElementById(canvas_id);

        if (this.canvas == null) {
            console.log("gr.js: will auto create canvas object");
            var tempHtml = '<canvas id="' + canvas_id + '" width="500" height="500"></canvas>';
            var tempElem = document.createElement('canvas');
            tempElem.innerHTML = tempHtml;

            document.getElementsByTagName('body')[0].appendChild(tempElem.firstChild);
        }

        this.canvas = document.getElementById(canvas_id);
        this.context = this.canvas.getContext('2d');
        this.context.save();

        this.set_dpr();

        return canvas_id;
    }),
    printErr: function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        if (0) {
            dump(text + '\n');
        } else {
            console.warn(text);
        }
    },
    set_dpr: function() {
      this.dpr = window.devicePixelRatio || 1;
      if (this.canvas.style.height == '' || this.canvas.style.width == '' || parseInt(this.canvas.style.width, 10) * this.dpr != this.canvas.width || parseInt(this.canvas.style.height, 10) * this.dpr != this.canvas.height) {
        this.canvas.style.width = this.canvas.width + "px";
        this.canvas.style.height = this.canvas.height + "px";
      }
      this.canvas.width = parseInt(this.canvas.style.width, 10) * this.dpr;
      this.canvas.height = parseInt(this.canvas.style.height, 10) * this.dpr;
      this.context.setTransform(this.dpr, 0, 0, this.dpr, 0, 0);
    },
    canvas: null,
    context: null,
    dpr: 1,
    setStatus: function(text) {},
    totalDependencies: 0,
    get_dash_list: function(linetype) {
        var list = Module._malloc(10 * 16);
        this.ccall('gks_get_dash_list', '', ['number', 'number', 'number'], [linetype, 1.0, list]);
        var result = [];
        var len = Module.getValue(list, 'i16');
        for (var i = 1; i < len + 1; i++) {
            result.push(Module.getValue(list + i * 4, 'i16'));
        }
        Module._free(list);
        return result;
    }
};

window.onerror = function(event) {};
