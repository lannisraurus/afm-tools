"""
João Camacho, 2025

This file contains a class - phaseSpaceSampler - for various sampling methods of data.

"""

# Imports

import random
import itertools

# Generic Interface Class for data sampling.
class phaseSpaceSampler:
# Sampler functions take at least:
# - a list of ranges (2-dim tuples or lists)
# - N: number of sample points to generate
# - Other parameters are dependent on the sampling method.

    # If there are d dimensions (len(ranges)=d), rangeDivisions is an n dimensional list.
    # each value in rangeDivisions states how many cells are in each dimension.
    def latinHypercubeSampler(self, ranges, N, rangeDivisions):
        
        # Error handling
        if len(ranges) != len(rangeDivisions):
            print("Error in "+self.latinHypercubeSampler.__name__+": ranges and rangeDivisions lists must be of equal size!")
            return []
        for ran in ranges:
            if len(ran) != 2:
                print("Error in "+self.latinHypercubeSampler.__name__+": ranges must be a list of 2-dim tuples/list!")
                return []
        for divs in rangeDivisions:
            if divs <= 0 or type(divs) != int:
                print("Error in "+self.latinHypercubeSampler.__name__+": rangeDivisions' elements must be real integers!")
                return []
        if N <= 0 or type(N) != int:
            print("Error in "+self.latinHypercubeSampler.__name__+": N must be a positive integer!")
            return []
            
        # Stratification algorithm
        stratas = []
        steps = []
        for index, ran in enumerate(ranges):

            steps.append(float(ran[1]-ran[0])/float(rangeDivisions[index]))

            stratas.append([])
            count = 1
            stratas[index].append(ran[0])
            while count < rangeDivisions[index]:
                stratas[index].append(stratas[index][-1] + steps[index])
                count += 1

        cells = [cell for cell in itertools.product(*stratas)]
        sampleCells = []

        # Sample cell extraction algorithm, without repetition, as long as N < len(cells)
        if len(cells) > N:
            sampleCells += random.sample(cells, N)
        else:
            K = N
            while K > len(cells):
                K -= len(cells)
                sampleCells += cells
            sampleCells += random.sample(cells, K)
        
        # Point extraction algorithm
        samplePoints = []
        for cellIndex, cell in enumerate(sampleCells):
            samplePoints.append([])
            for dimensionIndex, dimension in enumerate(cell):
                samplePoints[cellIndex].append( random.uniform(dimension, dimension+steps[dimensionIndex]) )
        
        # Finish - return
        return samplePoints


        
        

        