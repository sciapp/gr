GR.ready_callbacks = [];
GR.ready = function(callback){
    GR.ready_callbacks.push(callback);
};

Module['onRuntimeInitialized'] = function() {
    GR.ready_callbacks.forEach(function (callback) {
      	callback();
    })
};

function GR() {
    this.opengks = gr_opengks;
    this.closegks = gr_closegks;
    this.inqdspsize = gr_inqdspsize;
    this.openws = gr_openws;
    this.closews = gr_closews;
    this.activatews = gr_activatews;
    this.deactivatews = gr_deactivatews;
    this.clearws = gr_clearws;
    this.updatews = gr_updatews;
    this.polyline = gr_polyline;
    this.polymarker = gr_polymarker;
    this.text = gr_text;
    this.inqtext = gr_inqtext;
    this.fillarea = gr_fillarea;
    this.cellarray = gr_cellarray;
    this.spline = gr_spline;
    this.gridit = gr_gridit;
    this.setlinetype = gr_setlinetype;
    this.inqlinetype = gr_inqlinetype;
    this.setlinewidth = gr_setlinewidth;
    this.inqlinewidth = gr_inqlinewidth;
    this.setlinecolorind = gr_setlinecolorind;
    this.inqlinecolorind = gr_inqlinecolorind;
    this.setmarkertype = gr_setmarkertype;
    this.inqmarkertype = gr_inqmarkertype;
    this.setmarkersize = gr_setmarkersize;
    this.setmarkercolorind = gr_setmarkercolorind;
    this.inqmarkercolorind = gr_inqmarkercolorind;
    this.settextfontprec = gr_settextfontprec;
    this.setcharexpan = gr_setcharexpan;
    this.setcharspace = gr_setcharspace;
    this.settextcolorind = gr_settextcolorind;
    this.setcharheight = gr_setcharheight;
    this.setcharup = gr_setcharup;
    this.settextpath = gr_settextpath;
    this.settextalign = gr_settextalign;
    this.setfillintstyle = gr_setfillintstyle;
    this.setfillstyle = gr_setfillstyle;
    this.setfillcolorind = gr_setfillcolorind;
    this.setcolorrep = gr_setcolorrep;
    this.setscale = gr_setscale;
    this.inqscale = gr_inqscale;
    this.setwindow = gr_setwindow;
    this.inqwindow = gr_inqwindow;
    this.setviewport = gr_setviewport;
    this.selntran = gr_selntran;
    this.setclip = gr_setclip;
    this.setwswindow = gr_setwswindow;
    this.setwsviewport = gr_setwsviewport;
    this.createseg = gr_createseg;
    this.copysegws = gr_copysegws;
    this.redrawsegws = gr_redrawsegws;
    this.setsegtran = gr_setsegtran;
    this.closeseg = gr_closeseg;
    this.emergencyclosegks = gr_emergencyclosegks;
    this.updategks = gr_updategks;
    this.setspace = gr_setspace;
    this.inqspace = gr_inqspace;
    this.textext = gr_textext;
    this.inqtextext = gr_inqtextext;
    this.axes = gr_axes;
    this.grid = gr_grid;
    this.verrorbars = gr_verrorbars;
    this.herrorbars = gr_herrorbars;
    this.polyline3d = gr_polyline3d;
    this.axes3d = gr_axes3d;
    this.titles3d = gr_titles3d;
    this.surface = gr_surface;
    this.contour = gr_contour;
    this.setcolormap = gr_setcolormap;
    this.inqcolormap = gr_inqcolormap;
    this.colormap = gr_colormap;
    this.inqcolor = gr_inqcolor;
    this.inqcolorfromrgb = gr_inqcolorfromrgb;
    this.hsvtorgb = gr_hsvtorgb;
    this.tick = gr_tick;
    this.validaterange = gr_validaterange;
    this.adjustrange = gr_adjustrange;
    this.beginprint = gr_beginprint;
    this.beginprintext = gr_beginprintext;
    this.endprint = gr_endprint;
    this.ndctowc = gr_ndctowc;
    this.wctondc = gr_wctondc;
    this.drawrect = gr_drawrect;
    this.fillrect = gr_fillrect;
    this.drawarc = gr_drawarc;
    this.fillarc = gr_fillarc;
    this.drawpath = gr_drawpath;
    this.setarrowstyle = gr_setarrowstyle;
    this.drawarrow = gr_drawarrow;
    this.readimage = gr_readimage;
    this.drawimage = gr_drawimage;
    this.importgraphics = gr_importgraphics;
    this.setshadow = gr_setshadow;
    this.settransparency = gr_settransparency;
    this.setcoordxform = gr_setcoordxform;
    this.begingraphics = gr_begingraphics;
    this.endgraphics = gr_endgraphics;
    this.mathtex = gr_mathtex;
    this.beginselection = gr_beginselection;
    this.endselection = gr_endselection;
    this.moveselection = gr_moveselection;
    this.resizeselection = gr_resizeselection;
    this.inqbbox = gr_inqbbox;
    this.precision = gr_precision;
    this.setregenflags = gr_setregenflags;
    this.inqregenflags = gr_inqregenflags;
}

floatarray = function(a) {
    var ptr = Module._malloc(a.length * 8);
    var data = Module.HEAPF64.subarray(ptr / 8, ptr / 8 + a.length);
    
    for (i = 0; i < a.length; i++){
	data[i] = a[i];
    }       
    
    return ptr;
}

intarray = function(a) {
    var ptr = Module._malloc(a.length * 4);
    var data = Module.HEAP32.subarray(ptr / 4, ptr / 4 + a.length);
    
    for (i = 0; i < a.length; i++) {
	data[i] = a[i];
    }       
    
    return ptr;
}

uint8array = function(a) {
    var ptr = Module._malloc(a.length + 1);
    a = intArrayFromString(a, true);
    var data = Module.HEAPU8.subarray(ptr, ptr + a.length + 1);

    for (i = 0; i < a.length; i++) {
	data[i] = a[i];
    }
    data[a.length] = 0x00;

    return ptr;
}

freearray = function(ptr) {
    Module._free(ptr);
}

gr_opengks = Module.cwrap('gr_opengks', '', []);

gr_closegks = Module.cwrap('gr_closegks', '', []);

gr_inqdspsize_c = Module.cwrap('gr_inqdspsize', '', ['number', 'number', 'number', 'number', ]);
gr_inqdspsize = function() {
    var _mwidth = Module._malloc(8);
    var mwidth = Module.HEAPF64.subarray(_mwidth / 8, _mwidth / 8 + 1);
    var _mheight = Module._malloc(8);
    var mheight = Module.HEAPF64.subarray(_mheight / 8, _mheight / 8 + 1);
    var _width = Module._malloc(4);
    var width = Module.HEAP32.subarray(_width / 4, _width / 4 + 1);
    var _height = Module._malloc(4);
    var height = Module.HEAP32.subarray(_height / 4, _height / 4 + 1);
    gr_inqdspsize_c(_mwidth, _mheight, _width, _height);
    var result = new Array(4);
    result[0] = mwidth[0];
    result[1] = mheight[0];
    result[2] = width[0];
    result[3] = height[0];
    freearray(_mwidth);
    freearray(_mheight);
    freearray(_width);
    freearray(_height);
    return result;
}

gr_openws_c = Module.cwrap('gr_openws', '', ['number', 'number', 'number', ]);
gr_openws = function(workstation_id, connection, type) {
    _connection = uint8array(connection);
    gr_openws_c(workstation_id, _connection, type);
    freearray(_connection);
}

gr_closews = Module.cwrap('gr_closews', '', ['number', ]);

gr_activatews = Module.cwrap('gr_activatews', '', ['number', ]);

gr_deactivatews = Module.cwrap('gr_deactivatews', '', ['number', ]);

gr_clearws = Module.cwrap('gr_clearws', '', []);

gr_updatews = Module.cwrap('gr_updatews', '', []);

gr_polyline_c = Module.cwrap('gr_polyline', '', ['number', 'number', 'number', ]);
gr_polyline = function(n, x, y) {
    _x = floatarray(x);
    _y = floatarray(y);
    gr_polyline_c(n, _x, _y);
    freearray(_x);
    freearray(_y);
}

gr_polymarker_c = Module.cwrap('gr_polymarker', '', ['number', 'number', 'number', ]);
gr_polymarker = function(n, x, y) {
    _x = floatarray(x);
    _y = floatarray(y);
    gr_polymarker_c(n, _x, _y);
    freearray(_x);
    freearray(_y);
}

gr_text_c = Module.cwrap('gr_text', '', ['number', 'number', 'number', ]);
gr_text = function(x, y, string) {
    _string = uint8array(string);
    gr_text_c(x, y, _string);
    freearray(_string);
}

gr_inqtext_c = Module.cwrap('gr_inqtext', '', ['number', 'number', 'number', 'number', 'number', ]);
gr_inqtext = function(x, y, string) {
    _string = uint8array(string);
    var _tbx = Module._malloc(8);
    var tbx = Module.HEAPF64.subarray(_tbx / 8, _tbx / 8 + 1);
    var _tby = Module._malloc(8);
    var tby = Module.HEAPF64.subarray(_tby / 8, _tby / 8 + 1);
    gr_inqtext_c(x, y, _string, _tbx, _tby);
    var result = new Array(2);
    result[0] = tbx[0];
    result[1] = tby[0];
    freearray(_string);
    freearray(_tbx);
    freearray(_tby);
    return result;
}

gr_fillarea_c = Module.cwrap('gr_fillarea', '', ['number', 'number', 'number', ]);
gr_fillarea = function(n, x, y) {
    _x = floatarray(x);
    _y = floatarray(y);
    gr_fillarea_c(n, _x, _y);
    freearray(_x);
    freearray(_y);
}

gr_cellarray_c = Module.cwrap('gr_cellarray', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);
gr_cellarray = function(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow, color) {
    _color = intarray(color);
    gr_cellarray_c(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow, _color);
    freearray(_color);
}

gr_spline_c = Module.cwrap('gr_spline', '', ['number', 'number', 'number', 'number', 'number', ]);
gr_spline = function(n, px, py, m, method) {
    _px = floatarray(px);
    _py = floatarray(py);
    gr_spline_c(n, _px, _py, m, method);
    freearray(_px);
    freearray(_py);
}


gr_gridit_c = Module.cwrap('gr_gridit', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);
gr_gridit = function(nd, xd, yd, zd, nx, ny) {
    _xd = floatarray(xd);
    _yd = floatarray(yd);
    _zd = floatarray(zd);
    var _x = Module._malloc(nx * 8);
    var x = Module.HEAPF64.subarray(_x / 8, _x / 8 + nx);
    var _y = Module._malloc(ny * 8);
    var y = Module.HEAPF64.subarray(_y / 8, _y / 8 + ny);
    var _z = Module._malloc(nx*ny * 8);
    var z = Module.HEAPF64.subarray(_z / 8, _z / 8 + nx*ny);
    gr_gridit_c(nd, _xd, _yd, _zd, nx, ny, _x, _y, _z);
    var result = new Array(3);
    result[0] = new Float64Array(new ArrayBuffer(nx * 8));
    result[0].set(x);
    result[1] = new Float64Array(new ArrayBuffer(ny * 8));
    result[1].set(y);
    result[2] = new Float64Array(new ArrayBuffer(nx*ny * 8));
    result[2].set(z);
    freearray(_xd);
    freearray(_yd);
    freearray(_zd);
    freearray(_x);
    freearray(_y);
    freearray(_z);
    return result;
}

gr_setlinetype = Module.cwrap('gr_setlinetype', '', ['number', ]);

gr_inqlinetype_c = Module.cwrap('gr_inqlinetype', '', ['number', ]);
gr_inqlinetype = function() {
    var _ltype = Module._malloc(4);
    var ltype = Module.HEAP32.subarray(_ltype / 4, _ltype / 4 + 1);
    gr_inqlinetype_c(_ltype);
    var result = new Array(1);
    result[0] = ltype[0];
    freearray(_ltype);
    return result;
}

gr_setlinewidth = Module.cwrap('gr_setlinewidth', '', ['number', ]);

gr_inqlinewidth_c = Module.cwrap('gr_inqlinewidth', '', ['number', ]);
gr_inqlinewidth = function() {
    var _width = Module._malloc(8);
    var width = Module.HEAPF64.subarray(_width / 8, _width / 8 + 1);
    gr_inqlinewidth_c(_width);
    var result = new Array(1);
    result[0] = width[0];
    freearray(_width);
    return result;
}

gr_setlinecolorind = Module.cwrap('gr_setlinecolorind', '', ['number', ]);

gr_inqlinecolorind_c = Module.cwrap('gr_inqlinecolorind', '', ['number', ]);
gr_inqlinecolorind = function() {
    var _coli = Module._malloc(4);
    var coli = Module.HEAP32.subarray(_coli / 4, _coli / 4 + 1);
    gr_inqlinecolorind_c(_coli);
    var result = new Array(1);
    result[0] = coli[0];
    freearray(_coli);
    return result;
}

gr_setmarkertype = Module.cwrap('gr_setmarkertype', '', ['number', ]);

gr_inqmarkertype_c = Module.cwrap('gr_inqmarkertype', '', ['number', ]);
gr_inqmarkertype = function() {
    var _mtype = Module._malloc(4);
    var mtype = Module.HEAP32.subarray(_mtype / 4, _mtype / 4 + 1);
    gr_inqmarkertype_c(_mtype);
    var result = new Array(1);
    result[0] = mtype[0];
    freearray(_mtype);
    return result;
}

gr_setmarkersize = Module.cwrap('gr_setmarkersize', '', ['number', ]);

gr_setmarkercolorind = Module.cwrap('gr_setmarkercolorind', '', ['number', ]);

gr_inqmarkercolorind_c = Module.cwrap('gr_inqmarkercolorind', '', ['number', ]);
gr_inqmarkercolorind = function() {
    var _coli = Module._malloc(4);
    var coli = Module.HEAP32.subarray(_coli / 4, _coli / 4 + 1);
    gr_inqmarkercolorind_c(_coli);
    var result = new Array(1);
    result[0] = coli[0];
    freearray(_coli);
    return result;
}

gr_settextfontprec = Module.cwrap('gr_settextfontprec', '', ['number', 'number', ]);

gr_setcharexpan = Module.cwrap('gr_setcharexpan', '', ['number', ]);

gr_setcharspace = Module.cwrap('gr_setcharspace', '', ['number', ]);

gr_settextcolorind = Module.cwrap('gr_settextcolorind', '', ['number', ]);

gr_setcharheight = Module.cwrap('gr_setcharheight', '', ['number', ]);

gr_setcharup = Module.cwrap('gr_setcharup', '', ['number', 'number', ]);

gr_settextpath = Module.cwrap('gr_settextpath', '', ['number', ]);

gr_settextalign = Module.cwrap('gr_settextalign', '', ['number', 'number', ]);

gr_setfillintstyle = Module.cwrap('gr_setfillintstyle', '', ['number', ]);

gr_setfillstyle = Module.cwrap('gr_setfillstyle', '', ['number', ]);

gr_setfillcolorind = Module.cwrap('gr_setfillcolorind', '', ['number', ]);

gr_setcolorrep = Module.cwrap('gr_setcolorrep', '', ['number', 'number', 'number', 'number', ]);

gr_setscale = Module.cwrap('gr_setscale', 'number', ['number', ]);

gr_inqscale_c = Module.cwrap('gr_inqscale', '', ['number', ]);
gr_inqscale = function() {
    var _options = Module._malloc(4);
    var options = Module.HEAP32.subarray(_options / 4, _options / 4 + 1);
    gr_inqscale_c(_options);
    var result = new Array(1);
    result[0] = options[0];
    freearray(_options);
    return result;
}

gr_setwindow = Module.cwrap('gr_setwindow', '', ['number', 'number', 'number', 'number', ]);

gr_inqwindow_c = Module.cwrap('gr_inqwindow', '', ['number', 'number', 'number', 'number', ]);
gr_inqwindow = function() {
    var _xmin = Module._malloc(8);
    var xmin = Module.HEAPF64.subarray(_xmin / 8, _xmin / 8 + 1);
    var _xmax = Module._malloc(8);
    var xmax = Module.HEAPF64.subarray(_xmax / 8, _xmax / 8 + 1);
    var _ymin = Module._malloc(8);
    var ymin = Module.HEAPF64.subarray(_ymin / 8, _ymin / 8 + 1);
    var _ymax = Module._malloc(8);
    var ymax = Module.HEAPF64.subarray(_ymax / 8, _ymax / 8 + 1);
    gr_inqwindow_c(_xmin, _xmax, _ymin, _ymax);
    var result = new Array(4);
    result[0] = xmin[0];
    result[1] = xmax[0];
    result[2] = ymin[0];
    result[3] = ymax[0];
    freearray(_xmin);
    freearray(_xmax);
    freearray(_ymin);
    freearray(_ymax);
    return result;
}

gr_setviewport = Module.cwrap('gr_setviewport', '', ['number', 'number', 'number', 'number', ]);

gr_selntran = Module.cwrap('gr_selntran', '', ['number', ]);

gr_setclip = Module.cwrap('gr_setclip', '', ['number', ]);

gr_setwswindow = Module.cwrap('gr_setwswindow', '', ['number', 'number', 'number', 'number', ]);

gr_setwsviewport = Module.cwrap('gr_setwsviewport', '', ['number', 'number', 'number', 'number', ]);

gr_createseg = Module.cwrap('gr_createseg', '', ['number', ]);

gr_copysegws = Module.cwrap('gr_copysegws', '', ['number', ]);

gr_redrawsegws = Module.cwrap('gr_redrawsegws', '', []);

gr_setsegtran = Module.cwrap('gr_setsegtran', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);

gr_closeseg = Module.cwrap('gr_closeseg', '', []);

gr_emergencyclosegks = Module.cwrap('gr_emergencyclosegks', '', []);

gr_updategks = Module.cwrap('gr_updategks', '', []);

gr_setspace = Module.cwrap('gr_setspace', 'number', ['number', 'number', 'number', 'number', ]);

gr_inqspace_c = Module.cwrap('gr_inqspace', '', ['number', 'number', 'number', 'number', ]);
gr_inqspace = function() {
    var _zmin = Module._malloc(8);
    var zmin = Module.HEAPF64.subarray(_zmin / 8, _zmin / 8 + 1);
    var _zmax = Module._malloc(8);
    var zmax = Module.HEAPF64.subarray(_zmax / 8, _zmax / 8 + 1);
    var _rotation = Module._malloc(4);
    var rotation = Module.HEAP32.subarray(_rotation / 4, _rotation / 4 + 1);
    var _tilt = Module._malloc(4);
    var tilt = Module.HEAP32.subarray(_tilt / 4, _tilt / 4 + 1);
    gr_inqspace_c(_zmin, _zmax, _rotation, _tilt);
    var result = new Array(4);
    result[0] = zmin[0];
    result[1] = zmax[0];
    result[2] = rotation[0];
    result[3] = tilt[0];
    freearray(_zmin);
    freearray(_zmax);
    freearray(_rotation);
    freearray(_tilt);
    return result;
}

gr_textext_c = Module.cwrap('gr_textext', 'number', ['number', 'number', 'number', ]);
gr_textext = function(x, y, string) {
    _string = uint8array(string);
    gr_textext_c(x, y, _string);
    freearray(_string);
}

gr_inqtextext_c = Module.cwrap('gr_inqtextext', '', ['number', 'number', 'number', 'number', 'number', ]);
gr_inqtextext = function(x, y, string) {
    _string = uint8array(string);
    var _tbx = Module._malloc(8);
    var tbx = Module.HEAPF64.subarray(_tbx / 8, _tbx / 8 + 1);
    var _tby = Module._malloc(8);
    var tby = Module.HEAPF64.subarray(_tby / 8, _tby / 8 + 1);
    gr_inqtextext_c(x, y, _string, _tbx, _tby);
    var result = new Array(2);
    result[0] = tbx[0];
    result[1] = tby[0];
    freearray(_string);
    freearray(_tbx);
    freearray(_tby);
    return result;
}

gr_axes = Module.cwrap('gr_axes', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', ]);

gr_grid = Module.cwrap('gr_grid', '', ['number', 'number', 'number', 'number', 'number', 'number', ]);

gr_verrorbars_c = Module.cwrap('gr_verrorbars', '', ['number', 'number', 'number', 'number', 'number', ]);
gr_verrorbars = function(n, px, py, e1, e2) {
    _px = floatarray(px);
    _py = floatarray(py);
    _e1 = floatarray(e1);
    _e2 = floatarray(e2);
    gr_verrorbars_c(n, _px, _py, _e1, _e2);
    freearray(_px);
    freearray(_py);
    freearray(_e1);
    freearray(_e2);
}

gr_herrorbars_c = Module.cwrap('gr_herrorbars', '', ['number', 'number', 'number', 'number', 'number', ]);
gr_herrorbars = function(n, px, py, e1, e2) {
    _px = floatarray(px);
    _py = floatarray(py);
    _e1 = floatarray(e1);
    _e2 = floatarray(e2);
    gr_herrorbars_c(n, _px, _py, _e1, _e2);
    freearray(_px);
    freearray(_py);
    freearray(_e1);
    freearray(_e2);
}

gr_polyline3d_c = Module.cwrap('gr_polyline3d', '', ['number', 'number', 'number', 'number', ]);
gr_polyline3d = function(n, px, py, pz) {
    _px = floatarray(px);
    _py = floatarray(py);
    _pz = floatarray(pz);
    gr_polyline3d_c(n, _px, _py, _pz);
    freearray(_px);
    freearray(_py);
    freearray(_pz);
}

gr_axes3d = Module.cwrap('gr_axes3d', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);

gr_titles3d_c = Module.cwrap('gr_titles3d', '', ['number', 'number', 'number', ]);
gr_titles3d = function(x_title, y_title, z_title) {
    _x_title = uint8array(x_title);
    _y_title = uint8array(y_title);
    _z_title = uint8array(z_title);
    gr_titles3d_c(_x_title, _y_title, _z_title);
    freearray(_x_title);
    freearray(_y_title);
    freearray(_z_title);
}

gr_surface_c = Module.cwrap('gr_surface', '', ['number', 'number', 'number', 'number', 'number', 'number', ]);
gr_surface = function(nx, ny, px, py, pz, option) {
    _px = floatarray(px);
    _py = floatarray(py);
    _pz = floatarray(pz);
    gr_surface_c(nx, ny, _px, _py, _pz, option);
    freearray(_px);
    freearray(_py);
    freearray(_pz);
}

gr_contour_c = Module.cwrap('gr_contour', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);
gr_contour = function(nx, ny, nh, px, py, h, pz, major_h) {
    _px = floatarray(px);
    _py = floatarray(py);
    _h = floatarray(h);
    _pz = floatarray(pz);
    gr_contour_c(nx, ny, nh, _px, _py, _h, _pz, major_h);
    freearray(_px);
    freearray(_py);
    freearray(_h);
    freearray(_pz);
}

gr_setcolormap = Module.cwrap('gr_setcolormap', '', ['number', ]);

gr_inqcolormap_c = Module.cwrap('gr_inqcolormap', '', ['number', ]);
gr_inqcolormap = function() {
    var _index = Module._malloc(4);
    var index = Module.HEAP32.subarray(_index / 4, _index / 4 + 1);
    gr_inqcolormap_c(_index);
    var result = new Array(1);
    result[0] = index[0];
    freearray(_index);
    return result;
}

gr_colormap = Module.cwrap('gr_colormap', '', []);

gr_inqcolor_c = Module.cwrap('gr_inqcolor', '', ['number', 'number', ]);
gr_inqcolor = function(color) {
    var _rgb = Module._malloc(4);
    var rgb = Module.HEAP32.subarray(_rgb / 4, _rgb / 4 + 1);
    gr_inqcolor_c(color, _rgb);
    var result = new Array(1);
    result[0] = rgb[0];
    freearray(_rgb);
    return result;
}

gr_inqcolorfromrgb = Module.cwrap('gr_inqcolorfromrgb', 'number', ['number', 'number', 'number', ]);

gr_hsvtorgb_c = Module.cwrap('gr_hsvtorgb', '', ['number', 'number', 'number', 'number', 'number', 'number', ]);
gr_hsvtorgb = function(h, s, v, r, g, b) {
    _r = floatarray(r);
    _g = floatarray(g);
    _b = floatarray(b);
    gr_hsvtorgb_c(h, s, v, _r, _g, _b);
    freearray(_r);
    freearray(_g);
    freearray(_b);
}

gr_tick = Module.cwrap('gr_tick', 'number', ['number', 'number', ]);

gr_validaterange = Module.cwrap('gr_validaterange', 'number', ['number', 'number', ]);

gr_adjustrange_c = Module.cwrap('gr_adjustrange', '', ['number', 'number', ]);
gr_adjustrange = function(amin, amax) {
    _amin = floatarray(amin);
    _amax = floatarray(amax);
    gr_adjustrange_c(_amin, _amax);
    freearray(_amin);
    freearray(_amax);
}

gr_beginprint_c = Module.cwrap('gr_beginprint', '', ['number', ]);
gr_beginprint = function(pathname) {
    _pathname = uint8array(pathname);
    gr_beginprint_c(_pathname);
    freearray(_pathname);
}

gr_beginprintext_c = Module.cwrap('gr_beginprintext', '', ['number', 'number', 'number', 'number', ]);
gr_beginprintext = function(pathname, mode, format, orientation) {
    _pathname = uint8array(pathname);
    _mode = uint8array(mode);
    _format = uint8array(format);
    _orientation = uint8array(orientation);
    gr_beginprintext_c(_pathname, _mode, _format, _orientation);
    freearray(_pathname);
    freearray(_mode);
    freearray(_format);
    freearray(_orientation);
}

gr_endprint = Module.cwrap('gr_endprint', '', []);


gr_ndctowc_c = Module.cwrap('gr_ndctowc', '', ['number', 'number', ]);
gr_ndctowc = function(x, y) {
    var _x = Module._malloc(8);
    var x = Module.HEAPF64.subarray(_x / 8, _x / 8 + 1);
    x[0] = x;
    var _y = Module._malloc(8);
    var y = Module.HEAPF64.subarray(_y / 8, _y / 8 + 1);
    y[0] = y;
    gr_ndctowc_c(_x, _y);
    result = new Array(2);
    result[0] = x[0];
    result[1] = y[0];
    freearray(_x);
    freearray(_y);
    return result;
}


gr_wctondc_c = Module.cwrap('gr_wctondc', '', ['number', 'number', ]);
gr_wctondc = function(x, y) {
    var _x = Module._malloc(8);
    var x = Module.HEAPF64.subarray(_x / 8, _x / 8 + 1);
    x[0] = x;
    var _y = Module._malloc(8);
    var y = Module.HEAPF64.subarray(_y / 8, _y / 8 + 1);
    y[0] = y;
    gr_wctondc_c(_x, _y);
    result = new Array(2);
    result[0] = x[0];
    result[1] = y[0];
    freearray(_x);
    freearray(_y);
    return result;
}

gr_drawrect = Module.cwrap('gr_drawrect', '', ['number', 'number', 'number', 'number', ]);

gr_fillrect = Module.cwrap('gr_fillrect', '', ['number', 'number', 'number', 'number', ]);

gr_drawarc = Module.cwrap('gr_drawarc', '', ['number', 'number', 'number', 'number', 'number', 'number', ]);

gr_fillarc = Module.cwrap('gr_fillarc', '', ['number', 'number', 'number', 'number', 'number', 'number', ]);

gr_drawpath_c = Module.cwrap('gr_drawpath', '', ['number', 'number', 'number', 'number', ]);
gr_drawpath = function(n, vertices, codes, fill) {
    _codes = uint8array(codes);
    gr_drawpath_c(n, _vertices, _codes, fill);
    freearray(_vertices);
    freearray(_codes);
}

gr_setarrowstyle = Module.cwrap('gr_setarrowstyle', '', ['number', ]);

gr_drawarrow = Module.cwrap('gr_drawarrow', '', ['number', 'number', 'number', 'number', ]);

gr_readimage_c = Module.cwrap('gr_readimage', 'number', ['number', 'number', 'number', 'number', ]);
gr_readimage = function(path) {
    _path = uint8array(path);
    var _width = Module._malloc(4);
    var width = Module.HEAP32.subarray(_width / 4, _width / 4 + 1);
    var _height = Module._malloc(4);
    var height = Module.HEAP32.subarray(_height / 4, _height / 4 + 1);
    var _data = Module._malloc(4);
    var data = Module.HEAP32.subarray(_data / 4, _data / 4 + 1);
    gr_readimage_c(_path, _width, _height, _data);
    var result = new Array(3);
    result[0] = width[0];
    result[1] = height[0];
    result[2] = data[0];
    freearray(_path);
    freearray(_width);
    freearray(_height);
    freearray(_data);
    return result;
}

gr_drawimage_c = Module.cwrap('gr_drawimage', '', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', ]);
gr_drawimage = function(xmin, xmax, ymin, ymax, width, height, data, model) {
    _data = intarray(data);
    gr_drawimage_c(xmin, xmax, ymin, ymax, width, height, _data, model);
    freearray(_data);
}

gr_importgraphics_c = Module.cwrap('gr_importgraphics', 'number', ['number', ]);
gr_importgraphics = function(path) {
    _path = uint8array(path);
    gr_importgraphics_c(_path);
    freearray(_path);
}

gr_setshadow = Module.cwrap('gr_setshadow', '', ['number', 'number', 'number', ]);

gr_settransparency = Module.cwrap('gr_settransparency', '', ['number', ]);

gr_setcoordxform = Module.cwrap('gr_setcoordxform', '', ['number', ]);

gr_begingraphics_c = Module.cwrap('gr_begingraphics', '', ['number', ]);
gr_begingraphics = function(path) {
    _path = uint8array(path);
    gr_begingraphics_c(_path);
    freearray(_path);
}

gr_endgraphics = Module.cwrap('gr_endgraphics', '', []);

gr_mathtex_c = Module.cwrap('gr_mathtex', '', ['number', 'number', 'number', ]);
gr_mathtex = function(x, y, string) {
    _string = uint8array(string);
    gr_mathtex_c(x, y, _string);
    freearray(_string);
}

gr_beginselection = Module.cwrap('gr_beginselection', '', ['number', 'number', ]);

gr_endselection = Module.cwrap('gr_endselection', '', []);

gr_moveselection = Module.cwrap('gr_moveselection', '', ['number', 'number', ]);

gr_resizeselection = Module.cwrap('gr_resizeselection', '', ['number', 'number', 'number', ]);

gr_inqbbox_c = Module.cwrap('gr_inqbbox', '', ['number', 'number', 'number', 'number', ]);
gr_inqbbox = function() {
    var _xmin = Module._malloc(8);
    var xmin = Module.HEAPF64.subarray(_xmin / 8, _xmin / 8 + 1);
    var _xmax = Module._malloc(8);
    var xmax = Module.HEAPF64.subarray(_xmax / 8, _xmax / 8 + 1);
    var _ymin = Module._malloc(8);
    var ymin = Module.HEAPF64.subarray(_ymin / 8, _ymin / 8 + 1);
    var _ymax = Module._malloc(8);
    var ymax = Module.HEAPF64.subarray(_ymax / 8, _ymax / 8 + 1);
    gr_inqbbox_c(_xmin, _xmax, _ymin, _ymax);
    var result = new Array(4);
    result[0] = xmin[0];
    result[1] = xmax[0];
    result[2] = ymin[0];
    result[3] = ymax[0];
    freearray(_xmin);
    freearray(_xmax);
    freearray(_ymin);
    freearray(_ymax);
    return result;
}

gr_precision = Module.cwrap('gr_precision', 'number', []);

gr_setregenflags = Module.cwrap('gr_setregenflags', '', ['number', ]);

gr_inqregenflags = Module.cwrap('gr_inqregenflags', 'number', ['number', ]);
