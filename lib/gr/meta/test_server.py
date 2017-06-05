#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import SocketServer


ETB = b'\027'


class TCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        data = b''
        while ETB not in data:
            data += self.request.recv(1024).strip()
        print("Received:", data)


def main():
    host, port = "localhost", 8001
    server = SocketServer.TCPServer((host, port), TCPHandler)
    server.serve_forever()


if __name__ == '__main__':
    main()
