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

# Seed for randomization
torch.manual_seed(time.time())

# Instance of model
model = Model()

# Load ALL Data
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

# Train Test Split
X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.2)

# Convert to tensors
X_train = torch.FloatTensor(X_train)
X_test = torch.FloatTensor(X_test)
Y_train = torch.FloatTensor(Y_train)
Y_test = torch.FloatTensor(Y_test)

# Loss Function
criterion = nn.MSELoss()

# Optimizer and learning rate
optimizer = torch.optim.Adam(model.parameters(), lr=0.1)

# TRAIN
epochs = 500
losses = []
for i in range(epochs):
    Y_pred = model.forward(X_train)
    loss = criterion(Y_pred, Y_train)
    losses.append(loss.detach().numpy())
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

# RESULTS
print("Final loss: ",losses[-1])

fig, axs = plt.subplots(2)
fig.suptitle('Training Results')

axs[0].plot(losses)
axs[0].set(xlabel='Epoch', ylabel='Loss (logscale)')
axs[0].set_yscale('log')

testN = 1000
point = [1000, 0]
testY = [point[0],]

point = np.array(point)
point = torch.FloatTensor(point)
with torch.no_grad():
    for i in range(testN):
        point = model.forward(point)
        testY.append(point[0])

axs[1].plot(testY)
axs[1].set(xlabel='Step', ylabel='Position')

plt.tight_layout()
plt.show()
