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

        return canvas_id;
    }),
    printErr: function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        if (0) {
            dump(text + '\n');
        } else {
            console.error(text);
        }
    },
    canvas: null,
    context: null,
    setStatus: function(text) {
    },
    totalDependencies: 0,
};

window.onerror = function(event) {
};
