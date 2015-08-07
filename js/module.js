var Module = {
    preRun: [],
    postRun: [],
    printErr: function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        if (0) {
            dump(text + '\n');
        } else {
            console.error(text);
        }
    },
    canvas: (function() {
        var canvas = document.getElementById('canvas');

        return canvas;
    })(),
    context: (function() {
        var canvas = document.getElementById('canvas');

        var context = canvas.getContext('2d');
        context.save();

        return context;
    })(),
    setStatus: function(text) {
    },
    totalDependencies: 0,
};

window.onerror = function(event) {
};
