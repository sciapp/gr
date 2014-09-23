from ctypes import *
from constants import *

line_callback = None
fill_callback = None

fill_prototype = CFUNCTYPE(
    c_void_p, c_int, POINTER(c_double), POINTER(c_double), c_int)
line_prototype = CFUNCTYPE(
    c_void_p, c_int, POINTER(c_double), POINTER(c_double), c_int, c_int)

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
        ('lwidth', c_double),
        ('plcoli', c_int),
        ('mindex', c_int),
        ('mtype', c_int),
        ('mszsc', c_double),
        ('pmcoli', c_int),
        ('tindex', c_int),
        ('txfont', c_int),
        ('txprec', c_int),
        ('chxp', c_double),
        ('chsp', c_double),
        ('txcoli', c_int),
        ('chh', c_double),
        ('chup', c_double * 2),
        ('txp', c_int),
        ('txal', c_int * 2),
        ('findex', c_int),
        ('ints', c_int),
        ('styli', c_int),
        ('facoli', c_int),
        ('window', (c_double * 4) * MAX_TNR),
        ('viewport', (c_double * 4) * MAX_TNR),
        ('cntnr', c_int),
        ('clip', c_int),
        ('opsg', c_int),
        ('mat', (c_double * 2) * 3),
        ('asf', c_int * 13),
        ('wiss', c_int),
        ('version', c_int),
        ('fontfile', c_int),
        ('txslant', c_double),
        ('shoff', c_double * 2),
        ('blur', c_double),
        ('alpha', c_double),
        ('a', c_double * MAX_TNR),
        ('b', c_double * MAX_TNR),
        ('c', c_double * MAX_TNR),
        ('d', c_double * MAX_TNR)
    ]

    def sizeof(self):
        return sizeof(self)

    def copy_data(self, bytes):
        memmove(addressof(self), bytes, sizeof(self))

_gks.gks_set_dev_xform.argtypes = (POINTER(state_list), POINTER(c_double),
                                   POINTER(c_double))
_gks.gks_inq_pattern_array.argtypes = (c_int, c_int * 33)
_gks.gks_inq_rgb.argtypes = (
    c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double))
_gks.gks_get_dash_list.argtypes = (c_int, c_double, c_int * 10)
_gks.gks_init_core.argtypes = (POINTER(state_list),)
_gks.gks_open_font.argtypes = []
_gks.gks_emul_text.argtypes = (
    c_double, c_double, c_int, c_char_p, line_prototype, fill_prototype)
_gks.gks_set_norm_xform.argtypes = (
    c_int, POINTER(c_double), POINTER(c_double))


def set_dev_xform(s_list, window, viewport):

    c_window = (c_double*len(window))()
    for i in range(len(window)):
        c_window[i] = c_double(window[i])

    c_viewport = (c_double*len(viewport))()
    for i in range(len(viewport)):
        c_viewport[i] = c_double(viewport[i])

    _gks.gks_set_dev_xform(s_list,
                           cast(c_window, POINTER(c_double)),
                           cast(c_viewport, POINTER(c_double)))


def inq_pattern_array(index):
    global _gks
    pattern = (c_int * 33)()
    _gks.gks_inq_pattern_array(c_int(index), pattern)
    n = pattern[0]
    return [i for i in pattern[1:n+1]]


def inq_rgb(color_index):
    global _gks
    color = [c_double(0) for i in range(3)]
    _gks.gks_inq_rgb(c_int(color_index), *map(byref, color))
    return [c.value for c in color]


def get_dash_list(ltype, scale):
    global _gks

    lt = c_int * 10
    l = lt()
    _gks.gks_get_dash_list(c_int(ltype), c_double(scale), l)

    l = [i for i in l[1:] if i != 0]
    return l


def init_core(list):
    global _gks

    _gks.gks_init_core(byref(list))


def open_font():
    global _gks

    fd = _gks.gks_open_font()
    return fd


def emul_text(x_pos, y_pos, text):
    _gks.gks_emul_text(c_double(x_pos), c_double(y_pos),
                       c_int(len(text)), c_char_p(text),
                       line_callback, fill_callback)


def set_norm_xform(tnr, window, viewport):
    c_window = (c_double*len(window))()
    for i in range(len(window)):
        c_window[i] = c_double(window[i])

    c_viewport = (c_double*len(viewport))()
    for i in range(len(viewport)):
        c_viewport[i] = c_double(viewport[i])

    _gks.gks_set_norm_xform(c_int(tnr),
                            cast(c_window, POINTER(c_double)),
                            cast(c_viewport, POINTER(c_double)))
