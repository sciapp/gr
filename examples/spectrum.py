#!/usr/bin/env python
# -*- animation -*-
"""
Sample microphone input and display power spectrum
"""

from __future__ import print_function

import pyaudio
import numpy
from scipy import signal
import time
import gr

FS=44100       # Sampling frequency
SAMPLES=4096

mic = None
def get_spectrum():
    global mic
    if mic is None:
        pa = pyaudio.PyAudio()
        mic = pa.open(format=pyaudio.paInt16, channels=1, rate=FS,
                      input=True, frames_per_buffer=SAMPLES)
    amplitudes = numpy.fromstring(mic.read(SAMPLES), dtype=numpy.short)
    return abs(numpy.fft.fft(amplitudes / 32768.0))[:SAMPLES/2]

def parabolic(x, f, i):
    xe = 1/2. * (f[i-1] - f[i+1]) / (f[i-1] - 2 * f[i] + f[i+1]) + x
    ye = f[i] - 1/4. * (f[i-1] - f[i+1]) * (xe - x)
    return xe, ye

f = [FS/float(SAMPLES)*t for t in range(0, SAMPLES//2)]

gr.setviewport(0.1, 0.95, 0.1, 0.95)
gr.setwindow(50, 25000, 0, 100)
gr.setscale(1)

start = time.time()

while time.time() - start < 10:
    try:
        power = get_spectrum()
        peakind = signal.find_peaks_cwt(power, numpy.array([5]))
    except (IOError):
        continue

    gr.clearws()
    gr.setlinewidth(1)
    gr.setlinecolorind(1)
    gr.grid(1, 5, 50, 0, 1, 2)
    gr.axes(1, 5, 50, 0, 1, 2, -0.008)
    gr.setcharheight(0.020)
    gr.text(0.15, 0.965, '100Hz')
    gr.text(0.47, 0.965, '1kHz')
    gr.text(0.79, 0.965, '10kHz')
    gr.setlinecolorind(4)
    gr.polyline(f[1:], power[1:])
    for p in peakind:
        if power[p] > 10:
            gr.setlinewidth(2)
            gr.setlinecolorind(2)
            xe, ye = parabolic(f[p], power, p)
            print(xe, ye)
            gr.polyline([xe] * 2, [0, ye])
    gr.updatews()

