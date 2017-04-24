import Foundation

func char(str: NSString) ->UnsafeMutablePointer<Int8> {
  var buffer: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.alloc(132)
  str.getCString(buffer, maxLength: 132, encoding: NSISOLatin1StringEncoding)
  return buffer
}

public func opengks() {
  gr_opengks()
}

public func closegks() {
  gr_closegks()
}

public func inqdspsize() -> NSArray {
  var mwidth: CDouble = CDouble()
  var mheight: CDouble = CDouble()
  var width: CInt = CInt()
  var height: CInt = CInt()
  gr_inqdspsize(&mwidth, &mheight, &width, &height)
  return [Double(mwidth), Double(mheight), Int(width), Int(height)]
}

public func openws(workstation_id: Int, connection: String, workstation_type: Int) {
  gr_openws(CInt(workstation_id), char(connection), CInt(workstation_type))
}

public func closews(workstation_id: Int) {
  gr_closews(CInt(workstation_id))
}

public func activatews(workstation_id: Int) {
  gr_activatews(CInt(workstation_id))
}

public func deactivatews(worksation_id: Int) {
  gr_deactivatews(CInt(worksation_id))
}

public func clearws() {
  gr_clearws()
}

public func updatews() {
  gr_updatews()
}

func _assertEqualLength(lists: NSArray...) -> Int{
  for elem in lists[1..<lists.count] {
    assert(elem.count == lists[0].count, "Sequences must have same length.")
  }
  return lists[0].count
}

public func polyline(x: [Double], y: [Double]) {
  var n = _assertEqualLength(x, y)
  gr_polyline(CInt(n), UnsafeMutablePointer<Double>(x), UnsafeMutablePointer<Double>(y))
}

public func polymarker(x: [Double], y: [Double]) {
  var n = _assertEqualLength(x, y)
  gr_polymarker(CInt(n), UnsafeMutablePointer<Double>(x), UnsafeMutablePointer<Double>(y))
}

public func text(x: Double, y: Double, string: String) {
  gr_text(CDouble(x), CDouble(y), char(string))
}

public func inqtext(x: Double, y: Double, string: String) -> Array<Array<Double>> {
  var tbx = [CDouble](count: 4, repeatedValue: 0.0)
  var tby = [CDouble](count: 4, repeatedValue: 0.0)
  gr_inqtext(CDouble(x), CDouble(y), char(string), &tbx, &tby)
  return [[tbx[0], tbx[1], tbx[2], tbx[3]], [tby[0], tby[1], tby[2], tby[3]]]
}

public func fillarea(x: [Double], y: [Double]) {
  var n = _assertEqualLength(x, y)
  gr_fillarea(CInt(n), UnsafeMutablePointer<Double>(x), UnsafeMutablePointer<Double>(y))
}

public func cellarray(xmin: Double, xmax: Double, ymin: Double, ymax: Double, dimx: Int, dimy: Int, color:[Int32]) {
  gr_cellarray(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax), CInt(dimx), CInt(dimy), CInt(1), CInt(1), CInt(dimx), CInt(dimy), UnsafeMutablePointer<Int32>(color))
}

public func spline(px: [Double], py: [Double], m: Int, method: Int) {
  var n = _assertEqualLength(px, py)
  gr_spline(CInt(n), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), CInt(m), CInt(method))
}

public func gridit(xd: [Double], yd: [Double], zd: [Double], nx: Int, ny: Int) -> Array<Array<Double>>{
  var nd = _assertEqualLength(xd, yd, zd)
  var x = [CDouble](count: nx, repeatedValue: 0.0)
  var y = [CDouble](count: ny, repeatedValue: 0.0)
  var z = [CDouble](count: nx * ny, repeatedValue: 0.0)
  gr_gridit(CInt(nd), UnsafeMutablePointer<Double>(xd), UnsafeMutablePointer<Double>(yd), UnsafeMutablePointer<Double>(zd), CInt(nx), CInt(ny), &x, &y, &z)
  return [x, y, z]
}

public func setlinetype(style: Int) {
  gr_setlinetype(CInt(style))
}

public func inqlinetype() -> Int {
  var ltype = CInt()
  gr_inqlinetype(&ltype)
  return Int(ltype)
}

public func setlinewidth(width: Double) {
  gr_setlinewidth(CDouble(width))
}

public func setlinecolorind(color: Int) {
  gr_setlinecolorind(CInt(color))
}

public func inqlinecolorind() -> Int {
  var coli = CInt()
  gr_inqlinecolorind(&coli)
  return Int(coli)
}

public func setmarkertype(style: Int) {
  gr_setmarkertype(CInt(style))
}

public func inqmarkertype() -> Int {
  var mtype = CInt()
  gr_inqmarkertype(&mtype)
  return Int(mtype)
}

public func setmarkersize(size: Double) {
  gr_setmarkersize(CDouble(size))
}

public func setmarkercolorind(color: Int) {
  gr_setmarkercolorind(CInt(color))
}

public func inqmarkercolorind() -> Int {
  var coli = CInt()
  gr_inqmarkercolorind(&coli)
  return Int(coli)
}

public func setfontprec(font: Int, precision: Int) {
  gr_settextfontprec(CInt(font), CInt(precision))
}

public func settextfontprec(font: Int, precision: Int) {
  setfontprec(font, precision)
}

public func setcharexpan(factor: Double) {
  gr_setcharexpan(CDouble(factor))
}

public func setcharspace(spacing: Double) {
  gr_setcharspace(CDouble(spacing))
}

public func settextcolorind(color: Int) {
  gr_settextcolorind(CInt(color))
}

public func setcharheight(height: Double) {
  gr_setcharheight(CDouble(height))
}

public func setcharup(ux: Double, uy: Double) {
  gr_setcharup(CDouble(ux), CDouble(uy))
}

public func settextpath(path: Int) {
  gr_settextpath(CInt(path))
}

public func settextalign(horizontal: Int, vertical: Int) {
  gr_settextalign(CInt(horizontal), CInt(vertical))
}

public func setfillintstyle(style: Int) {
  gr_setfillintstyle(CInt(style))
}

public func setfillstyle(index: Int) {
  gr_setfillstyle(CInt(index))
}

public func setfillcolorind(color: Int) {
  gr_setfillcolorind(CInt(color))
}

public func setcolorrep(index: Int, red: Double, green: Double, blue: Double) {
  gr_setcolorrep(CInt(index), CDouble(red), CDouble(green), CDouble(blue))
}

public func setscale(options: Int) -> Int {
  return Int(gr_setscale(CInt(options)))
}

public func inqscale() -> Int {
  var options = CInt()
  gr_inqscale(&options)
  return Int(options)
}

public func setwindow(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_setwindow(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

public func inqwindow() -> Array<Double> {
  var xmin = CDouble()
  var xmax = CDouble()
  var ymin = CDouble()
  var ymax = CDouble()
  gr_inqwindow(&xmin, &xmax, &ymin, &ymax)
  return [Double(xmin), Double(xmax), Double(ymin), Double(ymax)]
}

public func setviewport(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_setviewport(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

public func inqviewport() -> Array<Double> {
  var xmin = CDouble()
  var xmax = CDouble()
  var ymin = CDouble()
  var ymax = CDouble()
  gr_inqviewport(&xmin, &xmax, &ymin, &ymax)
  return [Double(xmin), Double(xmax), Double(ymin), Double(ymax)]
}

public func selntran(transform: Int) {
  gr_selntran(CInt(transform))
}

public func setclip(indicator: Int) {
  gr_setclip(CInt(indicator))
}

public func setwswindow(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_setwswindow(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

public func setwsviewport(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_setwsviewport(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

func createseg(segment: Int) {
  gr_createseg(CInt(segment))
}

func copysegws(segment: Int) {
  gr_copysegws(CInt(segment))
}

func redrawsegws() {
  gr_redrawsegws()
}

public func setsegtran(segment: Int, fx: Double, fy: Double, transx: Double, transy: Double, phi: Double, scalex: Double, scaley: Double) {
  gr_setsegtran(CInt(segment), CDouble(fx), CDouble(fy), CDouble(transx), CDouble(transy), CDouble(phi), CDouble(scalex), CDouble(scaley))
}

public func closesegks() {
  gr_closegks()
}

public func emergencyclosegks() {
  gr_emergencyclosegks()
}

public func updategks() {
  gr_updategks()
}

public func setspace(zmin: Double, zmax: Double, rotation: Int, tilt: Int) -> Int {
  return Int(gr_setspace(CDouble(zmin), CDouble(zmax), CInt(rotation), CInt(tilt)))
}

public func inqspace() -> NSArray {
  var zmin = CDouble()
  var zmax = CDouble()
  var rotation = CInt()
  var tilt = CInt()
  gr_inqspace(&zmin, &zmax, &rotation, &tilt)
  return [Double(zmin), Double(zmax), Int(rotation), Int(tilt)]
}

public func textext(x: Double, y: Double, string: String) {
  gr_textext(CDouble(x), CDouble(y), char(string))
}

public func inqtextext(x: Double, y: Double, string: String) -> Array<Array<Double>> {
  var tbx = [CDouble](count: 4, repeatedValue: 0.0)
  var tby = [CDouble](count: 4, repeatedValue: 0.0)
  gr_inqtextext(CDouble(x), CDouble(y), char(string), &tbx, &tby)
  return [[tbx[0], tbx[1], tbx[2], tbx[3]], [tby[0], tby[1], tby[2], tby[3]]]
}

/*func axeslbl(x_tick: Double, y_tick: Double, x_org: Double, y_org: Double, major_x: Int, major_y: Int, tick_size: Doble, fpx=0, fpy=0) {
if fpx is None:
fpx = 0
if fpy is None:
fpy = 0

cfpx = _axeslbl_callback(fpx)
cfpy = _axeslbl_callback(fpy)
__gr.gr_axeslbl(c_double(x_tick), c_double(y_tick),
c_double(x_org), c_double(y_org),
c_int(major_x), c_int(major_y), c_double(tick_size),
cfpx, cfpy)
}*/

public func axes(x_tick: Double, y_tick: Double, x_org: Double, y_org: Double, major_x: Int, major_y: Int, tick_size: Double) {
  gr_axes(CDouble(x_tick), CDouble(y_tick), CDouble(x_org), CDouble(y_org), CInt(major_x), CInt(major_y), CDouble(tick_size))
}

public func grid(x_tick: Double, y_tick: Double, x_org: Double, y_org: Double, major_x: Int, major_y: Int) {
  gr_grid(CDouble(x_tick), CDouble(y_tick), CDouble(x_org), CDouble(y_org), CInt(major_x), CInt(major_y))
}

public func verrorbars(px: [Double], py: [Double], e1: [Double], e2: [Double]) {
  var n = _assertEqualLength(px, py, e1, e2)
  gr_verrorbars(CInt(n), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(e1), UnsafeMutablePointer<Double>(e2))
}

public func herrorbars(px: [Double], py: [Double], e1: [Double], e2: [Double]) {
  var n = _assertEqualLength(px, py, e1, e2)
  gr_herrorbars(CInt(n), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(e1), UnsafeMutablePointer<Double>(e2))
}

public func polyline3d(px: [Double], py: [Double], pz: [Double]) {
  var n = _assertEqualLength(px, py, pz)
  gr_polyline3d(CInt(n), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(pz))
}

public func polymarker3d(px: [Double], py: [Double], pz: [Double]) {
  var n = _assertEqualLength(px, py, pz)
  gr_polymarker3d(CInt(n), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(pz))
}

public func axes3d(x_tick: Double, y_tick: Double, z_tick: Double, x_org: Double, y_org: Double, z_org: Double, major_x: Int, major_y: Int, major_z: Int, tick_size: Double) {
  gr_axes3d(CDouble(x_tick), CDouble(y_tick), CDouble(z_tick), CDouble(x_org), CDouble(y_org), CDouble(z_org), CInt(major_x), CInt(major_y), CInt(major_z), CDouble(tick_size))
}

public func titles3d(x_title: String, y_title: String, z_title: String) {
  gr_titles3d(char(x_title), char(y_title), char(z_title))
}

public func surface(px: [Double], py: [Double], pz: [[Double]], option: Int) {
  var nx = px.count
  var ny = py.count
  var nz = pz.count
  assert((nz == nx && pz[0].count == ny), "Sequences have incorrect length.")
  gr_surface(CInt(nx), CInt(ny), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(pz), CInt(option))
}

public func surface(px: [Double], py: [Double], pz: [Double], option: Int) {
  var nx = px.count
  var ny = py.count
  var nz = pz.count
  assert(nz == nx * ny, "Sequences have incorrect length.")
  gr_surface(CInt(nx), CInt(ny), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(pz), CInt(option))
}

public func contour(px: [Double], py: [Double], h: [Double], pz: [[Double]], major_h: Int) {
  var nx = px.count
  var ny = py.count
  var nz = pz.count
  var nh = h.count
  assert((nz == nx && pz[0].count == ny), "Sequences have incorrect length.")
  gr_contour(CInt(nx), CInt(ny), CInt(nh), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(h), UnsafeMutablePointer<Double>(pz), CInt(major_h))
}

public func contour(px: [Double], py: [Double], h: [Double], pz: [Double], major_h: Int) {
  var nx = px.count
  var ny = py.count
  var nz = pz.count
  var nh = h.count
  assert(nz == nx * ny, "Sequences have incorrect length.")
  gr_contour(CInt(nx), CInt(ny), CInt(nh), UnsafeMutablePointer<Double>(px), UnsafeMutablePointer<Double>(py), UnsafeMutablePointer<Double>(h), UnsafeMutablePointer<Double>(pz), CInt(major_h))
}

public func hexbin(x: [Double], y: [Double], nbins: Int) {
  var n = _assertEqualLength(x, y)
  return Int(gr_hexbin(CInt(n), UnsafeMutablePointer<Double>(x), UnsafeMutablePointer<Double>(y), CInt(nbins)))
}

public func setcolormap(index: Int) {
  gr_setcolormap(CInt(index))
}

public func colorbar() -> Int {
  return Int(gr_colorbar())
}

public func inqcolor(color: Int) -> Int {
  var rgb = CInt()
  gr_inqcolor(CInt(color), &rgb)
  return Int(rgb)
}

public func inqcolorfromrgb(red: CDouble, green: Double, blue: Double) -> Int {
  return Int(gr_inqcolorfromrgb(CDouble(red),CDouble(green),CDouble(blue)))
}

public func tick(amin: Double, amax: Double) -> Double {
  var res = Double()
  var _amin = CDouble(amin)
  var _amax = CDouble(amax)
  res = gr_tick(_amin, _amax)
  return Double(res)
}

public func beginprint(pathname: String) {
  gr_beginprint(char(pathname))
}

public func beginprintext(pathname: String, mode: String, fmt: String, orientation: String) {
  gr_beginprintext(char(pathname), char(mode), char(fmt), char(orientation))
}

public func endprint() {
  gr_endprint()
}

func ndctowc(x: Int, y: Int) -> [Double] {
  var _x = CDouble(x)
  var _y = CDouble(y)
  gr_ndctowc(&_x, &_y)
  return [Double(_x), Double(_y)]
}

func wctondc(x: Int, y: Int) -> [Double] {
  var _x = CDouble(x)
  var _y = CDouble(y)
  gr_wctondc(&_x, &_y)
  return [Double(_x), Double(_y)]
}

public func drawrect(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_drawrect(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

public func fillrect(xmin: Double, xmax: Double, ymin: Double, ymax: Double) {
  gr_fillrect(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax))
}

public func drawarc(xmin: Double, xmax: Double, ymin: Double, ymax: Double, a1: Int, a2: Int) {
  gr_drawarc(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax), CInt(a1), CInt(a2))
}

public func fillarc(xmin: Double, xmax: Double, ymin: Double, ymax: Double, a1: Int, a2: Int) {
  gr_fillarc(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax), CInt(a1), CInt(a2))
}

public func drawpath(points: [[Double]], codes: [Int], fill: Int) {
  var n = _assertEqualLength(points, codes)
  var _points = [vertex_t]()
  for vec in points {
    var _vec = vertex_t(x: vec[0], y: vec[1])
    _points.append(_vec)
  }
  var _codes = [UInt8]()
  for elem in codes {
    _codes.append(UInt8(elem))
  }
  gr_drawpath(CInt(n), UnsafeMutablePointer<vertex_t>(_points), UnsafeMutablePointer<UInt8>(_codes), CInt(fill))
}

public func setarrowstyle(style: Int) {
  gr_setarrowstyle(CInt(style))
}

public func setarrowsize(size: Double) {
  gr_setarrowsize(CDouble(size))
}

public func drawarrow(x1: Double, y1: Double, x2: Double, y2: Double) {
  gr_drawarrow(CDouble(x1), CDouble(y1), CDouble(x2), CDouble(y2))
}

public func readimage(path: String) -> (width: Int, height: Int, data: UnsafeMutablePointer<CInt>) {
  var width = CInt()
  var height = CInt()
  var _data = UnsafeMutablePointer<CInt>()
  gr_readimage(char(path), &width, &height, &_data)
  
  return (Int(width), Int(height), _data)
}

public func drawimage(xmin: Double, xmax: Double, ymin: Double, ymax: Double, width: Int, height: Int, data: [Int], model: Int) {
  gr_drawimage(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax), CInt(width), CInt(height), UnsafeMutablePointer<Int32>(data), CInt(model))
}

public func drawimage(xmin: Double, xmax: Double, ymin: Double, ymax: Double, width: Int, height: Int, data: UnsafeMutablePointer<Int32>, model: Int) {
  gr_drawimage(CDouble(xmin), CDouble(xmax), CDouble(ymin), CDouble(ymax), CInt(width), CInt(height), UnsafeMutablePointer<Int32>(data), CInt(model))
}

public func importgraphics(path: String) {
  gr_importgraphics(char(path))
}

public func setshadow(offsetx: Double, offsety: Double, blur: Double) {
  gr_setshadow(CDouble(offsetx), CDouble(offsety), CDouble(blur))
}

public func settransparency(alpha: Double) {
  gr_settransparency(CDouble(alpha))
}

public func setcoordxform(mat: [[Double]]) {
  var _mat = [(mat[0][0],mat[0][1]),(mat[1][0],mat[1][1]),(mat[2][0],mat[2][1])]
  gr_setcoordxform(&_mat)
}

public func begingraphics(path: String) {
  gr_begingraphics(char(path))
}

public func endgraphics(){
  gr_endgraphics()
}

public func mathtex(x: Double, y: Double, string: String) {
  gr_mathtex(CDouble(x), CDouble(y), char(string))
}

public func endselection() {
  gr_endselection()
}

public func moveselection(x: Double, y: Double) {
  gr_moveselection(CDouble(x), CDouble(y))
}

public func resizeselection(kind: Int, x: Double, y: Double) {
  gr_resizeselection(CInt(kind), CDouble(x), CDouble(y))
}

public func inqbbox() -> [Double] {
  var xmin = CDouble()
  var xmax = CDouble()
  var ymin = CDouble()
  var ymax = CDouble()
  gr_inqbbox(&xmin, &xmax, &ymin, &ymax)
  return [Double(xmin), Double(xmax), Double(ymin), Double(ymax)]
}

let ASF_BUNDLED = 0
let ASF_INDIVIDUAL = 1

let NOCLIP = 0
let CLIP = 1

let COORDINATES_WC = 0
let COORDINATES_NDC = 1

let INTSTYLE_HOLLOW = 0
let INTSTYLE_SOLID = 1
let INTSTYLE_PATTERN = 2
let INTSTYLE_HATCH = 3

let TEXT_HALIGN_NORMAL = 0
let TEXT_HALIGN_LEFT = 1
let TEXT_HALIGN_CENTER = 2
let TEXT_HALIGN_RIGHT = 3
let TEXT_VALIGN_NORMAL = 0
let TEXT_VALIGN_TOP = 1
let TEXT_VALIGN_CAP = 2
let TEXT_VALIGN_HALF = 3
let TEXT_VALIGN_BASE = 4
let TEXT_VALIGN_BOTTOM = 5

let TEXT_PATH_RIGHT = 0
let TEXT_PATH_LEFT = 1
let TEXT_PATH_UP = 2
let TEXT_PATH_DOWN = 3

let TEXT_PRECISION_STRING = 0
let TEXT_PRECISION_CHAR = 1
let TEXT_PRECISION_STROKE = 2

let LINETYPE_SOLID = 1
let LINETYPE_DASHED = 2
let LINETYPE_DOTTED = 3
let LINETYPE_DASHED_DOTTED = 4
let LINETYPE_DASH_2_DOT = -1
let LINETYPE_DASH_3_DOT = -2
let LINETYPE_LONG_DASH = -3
let LINETYPE_LONG_SHORT_DASH = -4
let LINETYPE_SPACED_DASH = -5
let LINETYPE_SPACED_DOT = -6
let LINETYPE_DOUBLE_DOT = -7
let LINETYPE_TRIPLE_DOT = -8

let MARKERTYPE_DOT = 1
let MARKERTYPE_PLUS = 2
let MARKERTYPE_ASTERISK = 3
let MARKERTYPE_CIRCLE = 4
let MARKERTYPE_DIAGONAL_CROSS = 5
public let MARKERTYPE_SOLID_CIRCLE = -1
let MARKERTYPE_TRIANGLE_UP = -2
let MARKERTYPE_SOLID_TRI_UP = -3
let MARKERTYPE_TRIANGLE_DOWN = -4
let MARKERTYPE_SOLID_TRI_DOWN = -5
let MARKERTYPE_SQUARE = -6
let MARKERTYPE_SOLID_SQUARE = -7
let MARKERTYPE_BOWTIE = -8
let MARKERTYPE_SOLID_BOWTIE = -9
let MARKERTYPE_HOURGLASS = -10
let MARKERTYPE_SOLID_HGLASS = -11
let MARKERTYPE_DIAMOND = -12
let MARKERTYPE_SOLID_DIAMOND = -13
let MARKERTYPE_STAR = -14
let MARKERTYPE_SOLID_STAR = -15
let MARKERTYPE_TRI_UP_DOWN = -16
let MARKERTYPE_SOLID_TRI_RIGHT = -17
let MARKERTYPE_SOLID_TRI_LEFT = -18
let MARKERTYPE_HOLLOW_PLUS = -19
let MARKERTYPE_SOLID_PLUS = -20
let MARKERTYPE_PENTAGON = -21
let MARKERTYPE_HEXAGON = -22
let MARKERTYPE_HEPTAGON = -23
let MARKERTYPE_OCTAGON = -24
let MARKERTYPE_STAR_4 = -25
let MARKERTYPE_STAR_5 = -26
let MARKERTYPE_STAR_6 = -27
let MARKERTYPE_STAR_7 = -28
let MARKERTYPE_STAR_8 = -29
let MARKERTYPE_VLINE = -30
let MARKERTYPE_HLINE = -31
let MARKERTYPE_OMARK = -32

let OPTION_X_LOG = 1
let OPTION_Y_LOG = 2
let OPTION_Z_LOG = 4
let OPTION_FLIP_X = 8
let OPTION_FLIP_Y = 16
let OPTION_FLIP_Z = 32

let OPTION_LINES = 0
let OPTION_MESH = 1
let OPTION_FILLED_MESH = 2
let OPTION_Z_SHADED_MESH = 3
let OPTION_COLORED_MESH = 4
let OPTION_CELL_ARRAY = 5
let OPTION_SHADED_MESH = 6

let COLORMAP_UNIFORM = 0
let COLORMAP_TEMPERATURE = 1
let COLORMAP_GRAYSCALE = 2
let COLORMAP_GLOWING = 3
let COLORMAP_RAINBOWLIKE = 4
let COLORMAP_GEOLOGIC = 5
let COLORMAP_GREENSCALE = 6
let COLORMAP_CYANSCALE = 7
let COLORMAP_BLUESCALE = 8
let COLORMAP_MAGENTASCALE = 9
let COLORMAP_REDSCALE = 10
let COLORMAP_FLAME = 11
let COLORMAP_BROWNSCALE = 12
let COLORMAP_PILATUS = 13
let COLORMAP_AUTUMN = 14
let COLORMAP_BONE = 15
let COLORMAP_COOL = 16
let COLORMAP_COPPER = 17
let COLORMAP_GRAY = 18
let COLORMAP_HOT = 19
let COLORMAP_HSV = 20
let COLORMAP_JET = 21
let COLORMAP_PINK = 22
let COLORMAP_SPECTRAL = 23
let COLORMAP_SPRING = 24
let COLORMAP_SUMMER = 25
let COLORMAP_WINTER = 26
let COLORMAP_GIST_EARTH = 27
let COLORMAP_GIST_HEAT = 28
let COLORMAP_GIST_NCAR = 29
let COLORMAP_GIST_RAINBOW = 30
let COLORMAP_GIST_STERN = 31
let COLORMAP_AFMHOT = 32
let COLORMAP_BRG = 33
let COLORMAP_BWR = 34
let COLORMAP_COOLWARM = 35
let COLORMAP_CMRMAP = 36
let COLORMAP_CUBEHELIX = 37
let COLORMAP_GNUPLOT = 38
let COLORMAP_GNUPLOT2 = 39
let COLORMAP_OCEAN = 40
let COLORMAP_RAINBOW = 41
let COLORMAP_SEISMIC = 42
let COLORMAP_TERRAIN = 43
let COLORMAP_VIRIDIS = 44
let COLORMAP_INFERNO = 45
let COLORMAP_PLASMA = 46
let COLORMAP_MAGMA = 47

let FONT_TIMES_ROMAN = 101
let FONT_TIMES_ITALIC = 102
let FONT_TIMES_BOLD = 103
let FONT_TIMES_BOLDITALIC = 104
let FONT_HELVETICA = 105
let FONT_HELVETICA_OBLIQUE = 106
let FONT_HELVETICA_BOLD = 107
let FONT_HELVETICA_BOLDOBLIQUE = 108
let FONT_COURIER = 109
let FONT_COURIER_OBLIQUE = 110
let FONT_COURIER_BOLD = 111
let FONT_COURIER_BOLDOBLIQUE = 112
let FONT_SYMBOL = 113
let FONT_BOOKMAN_LIGHT = 114
let FONT_BOOKMAN_LIGHTITALIC = 115
let FONT_BOOKMAN_DEMI = 116
let FONT_BOOKMAN_DEMIITALIC = 117
let FONT_NEWCENTURYSCHLBK_ROMAN = 118
let FONT_NEWCENTURYSCHLBK_ITALIC = 119
let FONT_NEWCENTURYSCHLBK_BOLD = 120
let FONT_NEWCENTURYSCHLBK_BOLDITALIC = 121
let FONT_AVANTGARDE_BOOK = 122
let FONT_AVANTGARDE_BOOKOBLIQUE = 123
let FONT_AVANTGARDE_DEMI = 124
let FONT_AVANTGARDE_DEMIOBLIQUE = 125
let FONT_PALATINO_ROMAN = 126
let FONT_PALATINO_ITALIC = 127
let FONT_PALATINO_BOLD = 128
let FONT_PALATINO_BOLDITALIC = 129
let FONT_ZAPFCHANCERY_MEDIUMITALIC = 130
let FONT_ZAPFDINGBATS = 131

let PRINT_PS = "ps"
let PRINT_EPS = "eps"
let PRINT_PDF = "pdf"
let PRINT_BMP = "bmp"
let PRINT_JPEG = "jpeg"
let PRINT_JPG = "jpg"
let PRINT_PNG = "png"
let PRINT_TIFF = "tiff"
let PRINT_TIF = "tif"
let PRINT_FIG = "fig"
let PRINT_SVG = "svg"
let PRINT_WMF = "wmf"
