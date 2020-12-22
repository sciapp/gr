import Foundation

setenv("GKS_DOUBLE_BUF", "1", 1)

var x = linspace(x1: -3, x2: 3, n: 49)
var y = linspace(x1: -3, x2: 3, n: 49)
var z = peaks()

setwindow(xmin: -3, xmax: 3, ymin: -3, ymax: 3)
if setspace(zmin: -6, zmax: 6, rotation: 30, tilt: 60) == 0 {
    surface(px: x, py: y, pz: z)
}

let (mw, mh, w, h) = inqdspsize()
print(mw, mh, w, h)

settextfontprec(font: 232, precision: 3)
axes(x_tick: 0.5, y_tick: 0.5, x_org: -3, y_org: -3, major_x: 2, major_y: 2, tick_size: -0.01)
mathtex(x: 0.25, y: 0.25, string: "E=mc^2")

text(x: 0.25, y: 0.85, string: "Hello World")
let (tbx, tby) = inqtext(x: 0.25, y: 0.85, string: "Hello World")
fillarea(x: tbx, y: tby)

x = linspace(x1: -3, x2: 3)
y = x
for i in 0..<x.count {
    y[i] = sin(.pi * x[i])
}
polyline(x: x, y: y)
let color = Array<Int32>(0...79)
cellarray(xmin: 1, xmax: 3, ymin: -2.5, ymax: -0.5, dimx: 8, dimy: 10, color: color)
updatews()

if (readLine() == nil) {
  exit(EXIT_SUCCESS)
}
