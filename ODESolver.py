"""
João Camacho, 2025

This file contains a class - ODESolver - which transforms a set of starting
conditions and master equations into a description of the system in time.

"""
####################################### ODESolver

# Generic interface class for ODE solvers.
class ODESolver:
    # Solvers take an ODEPoint object, a list of [t,[positions, velocities, angles, ...]]
    # Solvers also take a masterEquations object, which is simply a list of functions governing each ODE.
    # Each equation in the masterEquations object should take as a first parameter the 

    ############ Steppers
    # Simple Euler Solver Method. Returns next iteration of an ODEPoint
    def eulerSolverStep(self, dt, masterEquations, ODEPoint, params):
        point = ODEPoint[1].copy()
        for index, equation in enumerate(masterEquations):
            point[index] = ODEPoint[1][index] + equation(ODEPoint[1], params)*dt
        return [ODEPoint[0] + dt, point]


    ############ Solvers
    # Simple Euler Solver Method. Returns a list of ODEPoints
    def eulerSolver(self, dt, T, masterEquations, startODEPoint, params):
        
        # Setup.
        systemSolution = [startODEPoint,]   # Solution: list of ODEPoints 
        
        # Solve System.
        while systemSolution[-1][0] <= T+startODEPoint[0]:
            systemSolution.append(self.eulerSolverStep(dt, masterEquations, systemSolution[-1], params))

        # Finished - return solution.
        return systemSolution

####################################### ODE Systems

# point[0]: position    point[1]: velocity
# params[0]: mass       params[1]: spring constant
massSpringSystem = [
    lambda point, parms: ( point[1] ),
    lambda point, params: ( -(params[1]/params[0])*point[0] )
]