#!/usr/bin/env python
''' GKS socket daemon '''

import socket
from thread import start_new_thread
from struct import unpack
import gks
import html5

HOST = ''
PORT = 8410

page = 0
fontfile = 0

functionTable = {
    12: ('polyline', 'iDD'),
    13: ('polymarker', 'iDD'),
    14: ('text', 'ddis'),
    15: ('fillarea', 'iDD'),
    16: ('cellarray', 'ddddiiiI'),
    19: ('set_pline_linetype', 'i'),
    20: ('set_pline_linewidth', 'd'),
    21: ('set_pline_color_index', 'i'),
    23: ('set_pmark_type', 'i'),
    24: ('set_pmark_size', 'd'),
    25: ('set_pmark_color_index', 'i'),
    27: ('set_text_fontprec', 'ii'),
    28: ('set_text_expdac', 'd'),
    29: ('set_text_spacing', 'd'),
    30: ('set_text_color_index', 'i'),
    31: ('set_text_height', 'd'),
    32: ('set_text_upvec', 'dd'),
    33: ('set_text_path', 'i'),
    34: ('set_text_align', 'ii'),
    36: ('set_fill_int_style', 'i'),
    37: ('set_fill_style_index', 'i'),
    38: ('set_fill_color_index', 'i'),
    41: ('set_asf', 'iiiiiiiiiiiii'),
    48: ('set_color_rep', 'iddd'),
    49: ('set_window', 'idddd'),
    50: ('set_viewport', 'idddd'),
    52: ('select_xform', 'i'),
    53: ('set_clipping', 'i'),
    200: ('set_text_slant', 'd'),
    201: ('draw_image', 'ddddiiiI'),
    202: ('set_shadow', 'ddd'),
    203: ('set_transparency', 'd'),
    204: ('set_coord_xform', 'dddddd')}


def unpackargs(format, data):
    ''' unpack function arguments and return Python tuple '''
    args = []
    for fmt in format:
        if fmt == 'i':
            args.append(unpack('i', data[:4])[0])
            data = data[4:]
            n = args[-1]
        elif fmt == 'd':
            args.append(unpack('d', data[:8])[0])
            data = data[8:]
            n = args[-1]
        elif fmt == 'I':
            n = args[-2] * args[-1]
            args.append(unpack('%di' % n, data[:n * 4]))
            data = data[n * 4:]
        elif fmt == 'D':
            args.append(unpack('%dd' % n, data[:n * 8]))
            data = data[n * 8:]
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
    global page, fontfile

    length = unpack('i', data[:4])[0]
    state_list = unpack_statelist(data[8:])
    data = data[length:]

    display_list = []
    length = unpack('i', data[:4])[0]

    offset = 0
    while length > 0:
        fctid = unpack('i', data[offset+4:offset+8])[0]
        if fctid in functionTable.keys():
            display_list.append((functionTable[fctid][0],
                                 unpackargs(functionTable[fctid][1],
                                            data[offset+8:offset+8+length])))
        offset += length
        if len(data) - offset < 4:
            break
        length = unpack('i', data[offset:offset+4])[0]

    if not fontfile:
        fontfile = gks.open_font()

    state_list.fontfile = fontfile
    gks.init_core(state_list)

    html5.Html_output(state_list, display_list, page)
    page += 1
