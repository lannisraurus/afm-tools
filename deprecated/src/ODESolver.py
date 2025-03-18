"""
João Camacho, 2025

This file contains a class - ODESolver - which transforms a set of starting
conditions and master equations into a description of the system in time.

It also contains user defined systems for example systems.

"""
####################################### ODESolver

# A class for ODE solvers.
class ODESolver:

    ############ Initializers
    # setSystem takes an ODEPoint object, a list of [t,[positions, velocities, angles, ...]]
    # It also takes a masterEquations object, which is simply a list of functions governing each ODE,
    # and a params object, which is a list of constant system parameters (mass, temperature, etc.)
    def setSystem(self, masterEquations, ODEPoint, params):
        self.masterEquations = masterEquations
        self.ODEPoint = ODEPoint
        self.params = params
        return self

    # Initializing with defined system
    def __init__(self, masterEquations, ODEPoint, params):
        self.setSystem(masterEquations, ODEPoint, params)
    
    # Initializing empty system
    def __init__(self):
        self.setSystem([], [0, []], [])

    ############ Steppers
    # Simple Euler Solver Method. Returns next iteration of an ODEPoint
    def eulerSolverStep(self, dt):
        point = self.ODEPoint[1].copy()
        for index, equation in enumerate(self.masterEquations):
            point[index] = self.ODEPoint[1][index] + equation(self.ODEPoint[1], self.params)*dt
        return [self.ODEPoint[0] + dt, point]


    ############ Solvers
    # Simple Euler Solver Method. Returns a list of ODEPoints
    def eulerSolver(self, dt, T):
        
        # Setup.
        systemSolution = [self.ODEPoint,]   # Solution: list of ODEPoints 
        
        # Solve System.
        while systemSolution[-1][0] <= T+self.ODEPoint[0]:
            systemSolution.append(self.eulerSolverStep(dt))

        # Finished - return solution.
        return systemSolution

####################################### ODE Example Systems

# Simple 1D mass spring system - Hook's law
# point[0]: position    point[1]: velocity
# params[0]: mass       params[1]: spring constant
massSpringSystem = [
    lambda point, parms: ( point[1] ),
    lambda point, params: ( -(params[1]/params[0])*point[0] )
]