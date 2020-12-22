import Foundation

setenv("GKS_DOUBLE_BUF", "1", 1)

let (mw, mh, w, h) = inqdspsize()
print(mw, mh, w, h)

settextfontprec(font: 232, precision: 3)
axes(x_tick: 0.1, y_tick: 0.1, x_org: 0, y_org: 0, major_x: 2, major_y: 2, tick_size: -0.01)
mathtex(x: 0.5, y: 0.5, string: "E=mc^2")

text(x: 0.25, y: 0.25, string: "Hello World")
let (tbx, tby) = inqtext(x: 0.25, y: 0.25, string: "Hello World")
fillarea(x: tbx, y: tby)

let x = Array(stride(from: 0.0, through: 1.0, by: 0.01))
var y = x
for i in 0...100 {
    y[i] = sin(.pi * x[i])
}
polyline(x: x, y: y)
let color = Array<Int32>(0...79)
cellarray(xmin: 0.45, xmax: 0.75, ymin: 0.05, ymax: 0.35, dimx: 8, dimy: 10, color: color)
updatews()

if (readLine() == nil) {
  exit(EXIT_SUCCESS)
}
