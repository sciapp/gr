module jlgr

import gr

export plot

function plot(x, y,
         bgcolor=0,
         viewport=(0.1, 0.95, 0.1, 0.95),
         window=None,
         scale=0,
         grid=true,
         linetype=gr.LINETYPE_SOLID,
         markertype=gr.MARKERTYPE_DOT,
         clear=true,
         update=true)
    if clear
        gr.clearws()
    end
    if window == None
        if scale & gr.OPTION_X_LOG == 0
            xmin, xmax = gr.adjustrange(minimum(x), maximum(x))
        else
            xmin, xmax = (minimum(x), maximum(x))
        end
        if scale & gr.OPTION_Y_LOG == 0
            ymin, ymax = gr.adjustrange(minimum(y), maximum(y))
        else
            ymin, ymax = (minimum(y), maximum(y))
        end
    else
        xmin, xmax, ymin, ymax = window
    end
    if scale & gr.OPTION_X_LOG == 0
        majorx = 5
        xtick = gr.tick(xmin, xmax) / majorx
    else
        xtick = majorx = 1
    end
    if scale & gr.OPTION_Y_LOG == 0
        majory = 5
        ytick = gr.tick(ymin, ymax) / majory
    else
        ytick = majory = 1
    end
    gr.setviewport(viewport[1], viewport[2], viewport[3], viewport[4])
    gr.setwindow(xmin, xmax, ymin, ymax)
    gr.setscale(scale)
    if bgcolor != 0
        gr.setfillintstyle(1)
        gr.setfillcolorind(bgcolor)
        gr.fillrect(xmin, xmax, ymin, ymax)
    end
    charheight = 0.024 * (viewport[4] - viewport[3])
    gr.setcharheight(charheight)
    if grid
         gr.grid(xtick, ytick, xmax, ymax, majorx, majory)
    end
    gr.axes(xtick, ytick, xmin, ymin, majorx, majory, 0.01)
    gr.axes(xtick, ytick, xmax, ymax, -majorx, -majory, -0.01)
    gr.setlinetype(linetype)
    gr.polyline(x, y)
    if markertype != gr.MARKERTYPE_DOT
        gr.setmarkertype(markertype)
        gr.polymarker(x, y)
    end
    if update
        gr.updatews()
    end
end

end
