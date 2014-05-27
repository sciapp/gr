#!/usr/bin/env python
''' GKS socket daemon '''

import socket, sys
from thread import start_new_thread
from struct import unpack
import gks
import html5

HOST = ''
PORT = 8410

n_dl = 0
ff = 0

functionTable = {
    12: ('polyline', 'iFF'),
    13: ('polymarker', 'iFF'),
    14: ('text', 'ffis'),
    15: ('fillarea', 'iFF'),
    16: ('cellarray', 'ffffiiiI'),
    19: ('set_pline_linetype', 'i'),
    20: ('set_pline_linewidth', 'f'),
    21: ('set_pline_color_index', 'i'),
    23: ('set_pmark_type', 'i'),
    24: ('set_pmark_size', 'f'),
    25: ('set_pmark_color_index', 'i'),
    27: ('set_text_fontprec', 'ii'),
    28: ('set_text_expfac', 'f'),
    29: ('set_text_spacing', 'f'),
    30: ('set_text_color_index', 'i'),
    31: ('set_text_height', 'f'),
    32: ('set_text_upvec', 'ff'),
    33: ('set_text_path', 'i'),
    34: ('set_text_align', 'ii'),
    36: ('set_fill_int_style', 'i'),
    37: ('set_fill_style_index', 'i'),
    38: ('set_fill_color_index', 'i'),
    41: ('set_asf', 'iiiiiiiiiiiii'),
    48: ('set_color_rep', 'ifff'),
    49: ('set_window', 'iffff'),
    50: ('set_viewport', 'iffff'),
    52: ('select_xform', 'i'),
    53: ('set_clipping', 'i'),
   200: ('set_text_slant', 'f'),
   201: ('draw_image', 'ffffiiiI'),
   202: ('set_shadow', 'fff'),
   203: ('set_transparency', 'f'),
   204: ('set_coord_xform', 'ffffff') }


def unpackargs(format, data):
    ''' unpack function arguments and return Python tuple '''
    args = []
    
    for fmt in format:
        if fmt == 'i' or fmt == 'f':    
            args.append(unpack(fmt, data[:4])[0])
            data = data[4:]
            n = args[-1]
        elif fmt == 'I':
            n = args[-2] * args[-1]
            args.append(unpack('%di' % n, data[:n * 4]))
            data = data[n * 4:]
        elif fmt == 'F':
            args.append(unpack('%df' % n, data[:n * 4]))
            data = data[n * 4:]
        elif fmt == 's':
            args.append(unpack('132s', data[:132])[0][:n])
            data = data[132:]
    return args

def unpack_statelist(data):
    state_list = gks.state_list()
    state_list.copy_data(data)
    return state_list

def interp(data):
    ''' interpret display list '''
    global n_dl, ff
    
    length = unpack('i', data[:4])[0]
    state_list = unpack_statelist(data[8:])
    data = data[length:]
    
    process_list = []
    length = unpack('i', data[:4])[0]
    
    offset=0
    while length > 0:
        fctid = unpack('i', data[offset+4:offset+8])[0]
        if functionTable.has_key(fctid):
            process_list.append((functionTable[fctid][0],
                  unpackargs(functionTable[fctid][1], data[offset+8:offset+8+length])))
        offset+=length
        if len(data)-offset < 4:
            break
        length = unpack('i', data[offset:offset+4])[0]
    if not ff:
        ff = gks.open_font() 
    state_list.fontfile = ff
    gks.init_core(state_list)
    
    html5.Html_output(state_list, process_list, n_dl)
    n_dl += 1
