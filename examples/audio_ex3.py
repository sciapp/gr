#!/usr/bin/env python
# -*- animation -*-
"""
Play an audio file and display spectrogram in realtime
"""

import os, wave, pyaudio
import numpy as np
import gr
import gr3

FS=44100       # Sampling frequency
SAMPLES = 2048

wf = wave.open(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                            'Monty_Python.wav'), 'rb')
pa = pyaudio.PyAudio()
stream = pa.open(format=pa.get_format_from_width(wf.getsampwidth()),
                 channels=wf.getnchannels(), rate=wf.getframerate(), output=True)

spectrum = np.zeros((256, 64), dtype=float)
t = -63
dt = float(SAMPLES) / FS
df = FS / float(SAMPLES) / 2 / 2

data = wf.readframes(SAMPLES)
while data != '' and len(data) == SAMPLES * wf.getsampwidth():
    stream.write(data)
    amplitudes = np.fromstring(data, dtype=np.short)
    power = abs(np.fft.fft(amplitudes / 32768.0))[:SAMPLES/2]

    gr.clearws()
    spectrum[:, 63] = power[:256]
    spectrum = np.roll(spectrum, 1)
    gr.setcolormap(-113)
    gr.setviewport(0.05, 0.95, 0.1, 1)
    gr.setwindow(t * dt, (t + 63) * dt, 0, df)
    gr.setscale(gr.OPTION_FLIP_X)
    gr.setspace(0, 256, 30, 80)
    gr3.surface((t + np.arange(64)) * dt, np.linspace(0, df, 256), spectrum, 4)
    gr.setscale(0)
    gr.axes3d(0.2, 0.2, 0, (t + 63) * dt, 0, 0, 5, 5, 0, -0.01)
    gr.titles3d('t [s]', 'f [kHz]', '')
    gr.updatews()

    data = wf.readframes(SAMPLES)
    t += 1
