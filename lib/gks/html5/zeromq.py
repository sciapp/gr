import sys
import zmq
import parser
from struct import unpack

context = zmq.Context()
socket = context.socket(zmq.PULL)
socket.connect("tcp://localhost:5556")

while True:
    len_data = socket.recv()
    length = unpack('i', len_data)[0]
    data = ''
    while len(data) < length:
        chunk = socket.recv()
        if chunk == '':
            break
        data += chunk
    if len(data) != length:
        break
    parser.interp(data)
