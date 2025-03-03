"""
João Camacho, 2025

This programme generates a data-set for a simple 1-dimensional mass-spring system.
The data is structured as follows:

t x

where t represents an instance of time, and x a position.

"""

####################################### Imports

import matplotlib.pyplot as plt
from ODESolver import *

####################################### Main Programme

########### Parameters
# Sampling Parameters
N = 10          # Number of data-points

# Simulation Parameters
dt = 0.001      # Time Step for Simulation

m0 = 1          # Mass
k0 = 1          # Spring Constant

# File parameters
fileName = 'massSpringData.csv'

########### Data Generation

solver = ODESolver()
