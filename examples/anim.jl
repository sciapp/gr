using PyPlot

x = [0:0.01:2*pi]

tic()
line, = plot(x, sin(x))
for i = 1:200
    line[:set_ydata](sin(x + i / 10.0))
    draw()
end

fps_gr = int(200 / toc())
println("fps  (PyPlot): ", fps_gr)

using jlgr

tic()
for i = 1:200
    plot(x, sin(x + i / 10.0))
end

fps_gr = int(200 / toc())
println("fps  (GR): ", fps_gr)

