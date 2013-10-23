from ctypes import *
from constants import *

line_callback = None
fill_callback = None

fill_prototype = CFUNCTYPE(c_void_p, c_int, POINTER(c_float), POINTER(c_float), c_int)
line_prototype = CFUNCTYPE(c_void_p, c_int, POINTER(c_float), POINTER(c_float), c_int, c_int)

_gks = CDLL('/usr/local/gr/lib/libGKS.so')

def set_line_callback(lrc):
    global line_callback
    line_callback = line_prototype(lrc)
    
def set_fill_callback(frc):
    global fill_callback
    fill_callback = fill_prototype(frc)

class state_list(Structure):
    _fields_ = [
        ('lindex', c_int),
        ('ltype', c_int),
        ('lwidth', c_float),
        ('plcoli', c_int),
        ('mindex', c_int),
        ('mtype', c_int),
        ('mszsc', c_float),
        ('pmcoli', c_int),
        ('tindex', c_int),
        ('txfont', c_int),
        ('txprec', c_int),
        ('chxp', c_float),
        ('chsp', c_float),
        ('txcoli', c_int),
        ('chh', c_float),
        ('chup', c_float * 2),
        ('txp', c_int),
        ('txal', c_int * 2),
        ('findex', c_int),
        ('ints', c_int),
        ('styli', c_int),
        ('facoli', c_int),
        ('window', (c_float * 4) * MAX_TNR),
        ('viewport', (c_float * 4) * MAX_TNR),
        ('cntnr', c_int),
        ('clip', c_int),
        ('opsg', c_int),
        ('mat', (c_float * 2) * 3),
        ('asf', c_int * 13),
        ('wiss', c_int),
        ('version', c_int),
        ('fontfile', c_int),
        ('txslant', c_float),
        ('shoff', c_float * 2),
        ('blur', c_float),
        ('alpha', c_float),
        ('a', c_float * MAX_TNR),
        ('b', c_float * MAX_TNR),
        ('c', c_float * MAX_TNR), 
        ('d', c_float * MAX_TNR) 
    ]
    
    def sizeof(self):
        return sizeof(self)
    
    def copy_data(self, bytes):
        memmove(addressof(self), bytes, sizeof(self))

_gks.gks_set_dev_xform.argtypes = (POINTER(state_list), POINTER(c_float), POINTER(c_float))
def set_dev_xform(s_list, window, viewport):
    
    c_window = (c_float*len(window))()
    for i in range(len(window)):
        c_window[i] = c_float(window[i])
        
    c_viewport = (c_float*len(viewport))()
    for i in range(len(viewport)):
        c_viewport[i] = c_float(viewport[i])
    
    _gks.gks_set_dev_xform(s_list, cast(c_window,POINTER(c_float)), cast(c_viewport,POINTER(c_float)))

_gks.gks_inq_pattern_array.argtypes = (c_int, c_int * 33)
def inq_pattern_array(index):
    global _gks
    pattern = (c_int * 33)()
    _gks.gks_inq_pattern_array(c_int(index), pattern)
    n = pattern[0]
    return [i for i in pattern[1:n+1]]

_gks.gks_inq_rgb.argtypes = (c_int, POINTER(c_float), POINTER(c_float), POINTER(c_float))
def inq_rgb(color_index):
    global _gks
    color = [c_float(0) for i in range(3)]
    _gks.gks_inq_rgb(c_int(color_index), *map(byref,color))
    return [c.value for c in color]

_gks.gks_get_dash_list.argtypes = (c_int, c_float, c_int * 10)
def get_dash_list(ltype, scale):
    global _gks
    
    lt = c_int * 10
    l = lt()
    _gks.gks_get_dash_list(c_int(ltype), c_float(scale), l)
    
    l = [i for i in l[1:] if i != 0]
    return l

_gks.gks_init_core.argtypes = (POINTER(state_list),)
def init_core(list):
    global _gks

    _gks.gks_init_core(byref(list))
   
_gks.gks_open_font.argtypes = []
def open_font():
    global _gks
    
    fd = _gks.gks_open_font()
    return fd

_gks.gks_emul_text.argtypes = (c_float, c_float, c_int, c_char_p, line_prototype, fill_prototype)
def emul_text(x_pos, y_pos, text):
    _gks.gks_emul_text(c_float(x_pos), c_float(y_pos), c_int(len(text)), c_char_p(text), line_callback, fill_callback)
    
_gks.gks_set_norm_xform.argtypes = (c_int, POINTER(c_float), POINTER(c_float))
def set_norm_xform(tnr, window, viewport):
    c_window = (c_float*len(window))()
    for i in range(len(window)):
        c_window[i] = c_float(window[i])
        
    c_viewport = (c_float*len(viewport))()
    for i in range(len(viewport)):
        c_viewport[i] = c_float(viewport[i])
        
    _gks.gks_set_norm_xform(c_int(tnr), cast(c_window,POINTER(c_float)), cast(c_viewport, POINTER(c_float)))
