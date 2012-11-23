#!/usr/bin/env python

import pyaudio
import numpy
import gr
from pygr import *

FS=44100		# Sampling frequency
SAMPLES=1000

mic = None
def get_audio_data():
    global mic
    if mic is None:
        pa = pyaudio.PyAudio()
        mic = pa.open(format=pyaudio.paInt16, channels=1, rate=FS,
                      input=True, frames_per_buffer=SAMPLES)
    amplitudes = numpy.fromstring(mic.read(SAMPLES), dtype=numpy.short)
    return amplitudes / 32768.

while (True):
    try:
        amplitudes = get_audio_data()
    except (IOError):
        continue
    plot(range(SAMPLES), amplitudes, window=[0, SAMPLES, -1, 1])

