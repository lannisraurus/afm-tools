import torch
import torch.nn as nn
import torch.nn.functional as nnf
import time
import numpy as np
from sklearn.model_selection import train_test_split

class Model(nn.Module):
    # Input Layer (x,v) --> H1(N) --> ... --> (x,v)
    def __init__(self, inputFeatures=2, h1=8, h2=9, outFeatures=2):
        super().__init__() # Initialize parent class
        # Define layers
        self.fc1 = nn.Linear(inputFeatures, h1)
        self.fc2 = nn.Linear(h1,h2)
        self.out = nn.Linear(h2,outFeatures)
    
    # Forwarding
    def forward(self, x):
        x = nnf.relu(self.fc1(x))
        x = nnf.relu(self.fc2(x))
        x = self.out(x)
        return x

# Seed for randomization
torch.manual_seed(time.time())

# Instance of model
model = Model()

# Load Training Data
file = open('training_data.txt','r')
X = [] # Input Features
Y = [] # Output Features
for line in file:
    xy = line.strip().split(';')
    xs = xy[0].split(' ')
    ys = xy[1].split(' ')
    xArr = []
    yArr = []
    for x in xs:
        if len(x) > 0:
            xArr.append(float(x))
    for y in ys:
        if len(y) > 0:
            yArr.append(float(y))
    X.append(xArr)
    Y.append(yArr)

# 
