#!/usr/bin/env python
# -*- no-plot -*-
"""
Sample microphone input and display spectrogram
"""

import pyaudio
import numpy as np
import time
import gr
import gr3

FS=44100       # Sampling frequency
SAMPLES=1024

mic = None
def get_spectrum():
    global mic
    if mic is None:
        pa = pyaudio.PyAudio()
        mic = pa.open(format=pyaudio.paInt16, channels=1, rate=FS,
                      input=True, frames_per_buffer=SAMPLES)
    amplitudes = np.fromstring(mic.read(SAMPLES), dtype=np.short)
    return abs(np.fft.fft(amplitudes / 32768.0))[:SAMPLES/2]

spectrum = np.zeros((256, 256), dtype=float)
t = -255
dt = float(SAMPLES) / FS
df = FS / float(SAMPLES) / 2 / 2

start = time.time()

while time.time() - start < 10:
    try:
        power = get_spectrum()
    except (IOError):
        continue

    gr.clearws()
    spectrum[:, 255] = power[:256]
    spectrum = np.roll(spectrum, 1)
    gr.setcolormap(-113)
    gr.setviewport(0.05, 0.95, 0.1, 1)
    gr.setwindow(t * dt, (t + 255) * dt, 0, df)
    gr.setscale(gr.OPTION_FLIP_X)
    gr.setspace(0, 200, 30, 80)
    gr3.surface((t + np.arange(256)) * dt, np.linspace(0, df, 256), spectrum, 4)
    gr.setscale(0)
    gr.axes3d(0.2, 0.2, 0, (t + 255) * dt, 0, 0, 5, 5, 0, -0.01)
    gr.titles3d('t [s]', 'f [kHz]', '')
    gr.updatews()

    t += 1
