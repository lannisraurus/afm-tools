"""
João Camacho, 2025

This programme generates a data-set for a simple 1-dimensional mass-spring system.
The data is structured as follows:

t x

where t represents an instance of time, and x a position.

"""

####################################### Imports

import src.ODESolver as ode
import src.phaseSpaceSampler as smp

####################################### Main Programme

########### Parameters
# Sampling Parameters
N = 10000       # Number of data-points

# Simulation Parameters
dt = 0.001      # Time Step for Simulation

m0 = 1          # Mass
k0 = 1          # Spring Constant

# File parameters
fileName = 'massSpringData.csv'

########### Data Generation

solver = ode.ODESolver()
sampler = smp.phaseSpaceSampler()

samples = sampler.latinHypercubeSampler( [[0,0],[-20,20],[-20,20]], 1000, [1,40,40] )

odeStartSample = [[sample[0],sample[1:]] for sample in samples]
odeFinalSample = [ solver.setSystem(ode.massSpringSystem, odeStart, [m0, k0]).eulerSolverStep(dt) for odeStart in odeStartSample]

file = open("training_data.txt",mode='w')
for index, start in enumerate(odeStartSample):

    for var in start[1]:
        file.write(str(var)+' ')
    file.write('; ')
    for var in odeFinalSample[index][1]:
        file.write(str(var)+' ')
    file.write('\n')

file.close()