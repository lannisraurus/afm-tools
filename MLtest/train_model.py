import torch
import torch.nn as nn
import torch.nn.functional as nnf
import time
import numpy as np
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt

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

# Msg
print("\n*** TRAINING MODEL ***")
print("This programme will train upon ODE data to learn how to evolve specific physical systems.\n")

# Seed for randomization
print(">>> Setting seed...")
torch.manual_seed(10)

# Instance of model
print(">>> Initializing model...")
model = Model()

# Load ALL Data
print(">>> Loading data from training_data.txt...")
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

for x in X:
    x = np.array(x)
for y in Y:
    y = np.array(y)

X = np.array(X)
Y = np.array(Y)
print(">>> Done loading data!")

# Train Test Split
print(">>> Splitting data into parts...")
X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.2, random_state=11)

# Convert to tensors
X_train = torch.FloatTensor(X_train)
X_test = torch.FloatTensor(X_test)
Y_train = torch.FloatTensor(Y_train)
Y_test = torch.FloatTensor(Y_test)

# Loss Function
print(">>> Setting model criterion and loss function...")
criterion = nn.MSELoss()

# Optimizer and learning rate
optimizer = torch.optim.Adam(model.parameters(), lr=0.1)

# TRAIN
epochs = 5000
print(">>> Training for ",epochs," epochs.")
losses = []
for i in range(epochs):
    Y_pred = model.forward(X_train)
    loss = criterion(Y_pred, Y_train)
    losses.append(loss.detach().numpy())
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

# RESULTS
print(">>> TRAINING COMPLETE! Final loss was: ",losses[-1])
print("\n*** INTERACTIVE MODE ***")
print("Feel free to try initial conditions; enter an invalid input to exit.\n")

fig, axs = plt.subplots(2)
fig.suptitle('Training Results ('+str(epochs)+' epochs)')

axs[0].plot(losses)
axs[0].set(xlabel='Epoch', ylabel='Loss (logscale)')
axs[0].set_yscale('log')

testN = 2000
point = [500, 0]
testY = [point[0],]

pointT = np.array(point)
pointT = torch.FloatTensor(point)

def testEvolve(Y, N, init):
    point = init
    Y.clear()
    Y.append(init[0])
    with torch.no_grad():
        for i in range(N):
            point = model.forward(point)
            Y.append(point[0])

testEvolve(testY, testN, pointT)

axs[1].plot(testY, label=("x0 v0 = "+str(point[0]) +' '+str(point[1])))
axs[1].legend(loc="upper right")
axs[1].set(xlabel='Time Step', ylabel='Position')

plt.tight_layout()
plt.show(block = False)

while True:
    initTest = input(">>> Please insert an initial condition (x vx): ")
    try:
        initTestSplit = initTest.split()
        point = [float(initTestSplit[0]), float(initTestSplit[1])]
        point = np.array(point)
        point = torch.FloatTensor(point)
        testEvolve(testY, testN, point)
        axs[1].plot(testY, label=("x0 v0 = "+initTest))
        axs[1].legend(loc="upper right")
        print(">>> Done! Check the graph :)")
        plt.show(block = False)
    except:
        print(">>> Invalid input! Exiting...")
        break
