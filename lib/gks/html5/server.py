import sys
import socket
import bottle

from struct import unpack
from thread import start_new_thread
from gevent import monkey

import parser

monkey.patch_all()

plots = []
pindex = 0

HOST = ''
PORT = 8410

def listen():
    global plots

    sock_stream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock_stream.bind((HOST, PORT))
    except socket.error, msg:
        print('Bind failed: ' + msg[1])
        sys.exit()

    sock_stream.listen(5)

    while True:
        conn, addr = sock_stream.accept()
        while True:
            data = conn.recv(4)
            if not data:
                break
            length = unpack('i', data)[0]
            data = ''
            while len(data) < length:
                chunk = conn.recv(length - len(data))
                if chunk == '':
                    break
                data += chunk
            if len(data) != length:
                break
            plots.append(data.decode('latin-1').encode('utf-8'))
        conn.close()

@bottle.get('/stream')
def sendplot():
    global plots
    bottle.response.content_type  = 'text/event-stream'
    bottle.response.cache_control = 'no-cache'

    # Set client-side auto-reconnect timeout, ms.
    yield 'retry: 100\n\n'

    while True:
        if len(plots) > 0:
            yield 'data: %s\n\n' % plots.pop().replace('\n','')
        else:
            yield ''

@bottle.route('/<filename>')
def send_static(filename='index.html'):
    '''
    return the requested static web page to the web browser
    '''
    return bottle.static_file(filename, root='./')

@bottle.route('/')
def index():
    return bottle.static_file('index.html', root='./')

start_new_thread(listen, ())

bottle.run(server=bottle.GeventServer, host='127.0.0.1', port=8080)
