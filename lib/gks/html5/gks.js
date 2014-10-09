factor = 1

var es = new EventSource("stream");
var Buffers = [document.getElementById("html-canvas"), document.getElementById("html-canvas2")];
var DrawingBuffer = 0;
var canvas, c;

es.onmessage = function (e) {
    canvas= Buffers[DrawingBuffer];
    c = canvas.getContext('2d');

    clear_canvas(c);
    eval(e.data);

    Buffers[1 - DrawingBuffer].style.visibility='hidden';
    Buffers[DrawingBuffer].style.visibility='visible';

    DrawingBuffer= 1 - DrawingBuffer;
};

function set_dashes(ctx, dashes) {
    user_agent = navigator.userAgent;
    if (user_agent.indexOf("Firefox")!=-1) {
        if (dashes[0] != 0) {
            ctx.mozDash = dashes;
        } else {
            ctx.mozDash = [];
        }
    }else{
        if (!ctx.setLineDash) {
            ctx.setLineDash = function () {}
        }
        ctx.setLineDash(dashes);
    }
}

function key_pressed(event) {
    if (event.keyCode == 43 || event.which == 43) { // + pressed
        zoom_in();
    } else if (event.keyCode == 45 || event.which == 45) { // - pressed 
        zoom_out();
    } else if (event.keyCode == 48 || event.which == 48) { // 0 pressed
        reset_zoom();
    }
}

function zoom_in(){
    c.save();
    factor = factor + 0.2;
    c.canvas.width  = base_width*factor;
    c.canvas.height  = base_height*factor;
    
    clear_canvas();
    c.scale(factor,factor);
    draw();
    
    c.restore();
}

function zoom_out(){
    c.save();
    if ((factor - 0.2) > 0.01) {
        factor = factor - 0.2;
    }
    c.canvas.width  = base_width*factor;
    c.canvas.height  = base_height*factor;
        
    clear_canvas();
    c.scale(factor, factor);
    draw();
        
    c.restore();
}

function reset_zoom(){
    factor = 1;
        
    c.canvas.width  = base_width;
    c.canvas.height  = base_height;
    clear_canvas();
    draw();
}

function clear_canvas(c){
    c.restore();
    c.save();
    c.beginPath();
    c.rect(0, 0, canvas.width, canvas.height);
    c.clip();
    c.clearRect(0, 0, canvas.width, canvas.height);
}

document.onkeypress = key_pressed;
