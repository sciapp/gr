'''
Simulation demo: movie of growing particles on substrate
'''

import os, sys
import numpy
import gr
from gr.pygr import imshow
import math
from bornagain import *

Nframes = 50

radius = 1
height = 4
distance = 5

# ----------------------------------
# describe sample and run simulation
# ----------------------------------
def RunSimulation():
    # defining materials
    mAir = HomogeneousMaterial("Air", 0.0, 0.0)
    mSubstrate = HomogeneousMaterial("Substrate", 6e-6, 2e-8)
    mParticle = HomogeneousMaterial("Particle", 6e-4, 2e-8)

    # collection of particles
    cylinder_ff = FormFactorCylinder(radius, height)
    cylinder = Particle(mParticle, cylinder_ff)
    particle_layout = ParticleLayout()
    particle_layout.addParticle(cylinder)

    # interference function
    interference = InterferenceFunctionRadialParaCrystal(distance)
    pdf = FTDistribution1DGauss(3 * nanometer)
    interference.setProbabilityDistribution(pdf)
    particle_layout.addInterferenceFunction(interference)

    # air layer with particles and substrate form multi layer
    air_layer = Layer(mAir)
    air_layer.addLayout(particle_layout)
    substrate_layer = Layer(mSubstrate)
    multi_layer = MultiLayer()
    multi_layer.addLayer(air_layer)
    multi_layer.addLayer(substrate_layer)

    # build and run experiment
    simulation = GISASSimulation()
    simulation.setDetectorParameters(100, -4.0 * degree, 4.0 * degree, 100, 0.0 * degree, 8.0 * degree)
    simulation.setBeamParameters(1.0 * angstrom, 0.2 * degree, 0.0 * degree)
    simulation.setSample(multi_layer)
    simulation.runSimulation()
    # intensity data
    return simulation.getIntensityData().getArray()


def SetParameters(i):
    global radius
    global height
    global distance
    radius = (1. + (3.0/Nframes)*i) * nanometer
    height = (1. + (4.0/Nframes)*i) * nanometer
    distance = (10. - (1.0/Nframes)*i) * nanometer

#-------------------------------------------------------------
# main()
#-------------------------------------------------------------
if __name__ == '__main__':
    for i in range(Nframes):
        SetParameters(i)
        result = RunSimulation() + 1 # for log scale
        result = numpy.log10(result)
        imshow(result, cmap=gr.COLORMAP_PILATUS)
