#!/usr/bin/env python3

import math
import gr
from base64 import b64encode
from gr import pygr


GRAPHICS_FILENAME = "gr.xml.base64"


def create_example_graphics_export():
    x = [2 * math.pi * i / 100 for i in range(100)]
    y = [math.sin(c) for c in x]
    gr.begingraphics(GRAPHICS_FILENAME)
    pygr.plot(x, y)
    gr.endgraphics()
    with open(GRAPHICS_FILENAME, "rb") as f:
        graphics_data = f.read()
    base64_encoded_graphics_data = b64encode(graphics_data)
    with open(GRAPHICS_FILENAME, "wb") as f:
        f.write(base64_encoded_graphics_data)


def main():
    create_example_graphics_export()


if __name__ == "__main__":
    main()
