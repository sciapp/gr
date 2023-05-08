var Module = {
    preRun: [],
    postRun: [],
    set_canvas: (function(canvas_id) {
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
        let _dpr = window.devicePixelRatio || 1;
        if (!this.original_canvas_size) {
            /* Store the initial size of the canvas. JSTerm uses style properties to set canvas width,
            *  plain gr.js sets width and height of the canvas directly so clientWidth and clientHeight
            *  are used. */
            if (this.canvas.style.width && this.canvas.style.height) {
                this.original_canvas_size = [
                    parseInt(this.canvas.style.width, 10),
                    parseInt(this.canvas.style.height, 10)
                ];
            } else {
                this.original_canvas_size = [
                    parseInt(this.canvas.clientWidth, 10),
                    parseInt(this.canvas.clientHeight, 10)
                ];
            }
        }
        this.dpr = _dpr;
        /* JSTerm uses multiple overlay canvases and replaces `this.canvas`. Therefore, the `dpr` must be
        *  set and compared for each individual canvas. */
        if (!(this.canvas.id in this.dpr_per_canvas) || this.dpr !== this.dpr_per_canvas[this.canvas.id]) {
            /* Set the size in memory (https://developer.mozilla.org/en-US/docs/Web/API/Window/devicePixelRatio#correcting_resolution_in_a_canvas) */
            this.canvas.width = this.original_canvas_size[0] * _dpr;
            this.canvas.height = this.original_canvas_size[1] * _dpr;
            this.context.setTransform(_dpr, 0, 0, _dpr, 0, 0);
            /* Set the display size if not already set */
            if (!(this.canvas.style.width && this.canvas.style.height)) {
                this.canvas.style.width = this.original_canvas_size[0] + "px";
                this.canvas.style.height = this.original_canvas_size[1] + "px";
            }
            this.dpr_per_canvas[this.canvas.id] = this.dpr;
        }
    },
    canvas: null,
    context: null,
    dpr: 1,
    dpr_per_canvas: [],
    original_canvas_size: null,
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
