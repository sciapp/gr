#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import sys
PY2 = (sys.version_info.major < 3)
if PY2:
    import SocketServer as socketserver
else:
    import socketserver  # pylint: disable=import-error

ETB = b'\027'


class TCPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        data = b''
        while ETB not in data:
            data += self.request.recv(1024).strip()
        print("Received:", data)


def main():
    host, port = "localhost", 8001
    server = socketserver.TCPServer((host, port), TCPHandler)
    server.serve_forever()


if __name__ == '__main__':
    main()
