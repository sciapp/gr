factor = 1

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

function clear_canvas(){
    c.clearRect(0, 0, c.canvas.width, c.canvas.height);
}

document.onkeypress = key_pressed;
