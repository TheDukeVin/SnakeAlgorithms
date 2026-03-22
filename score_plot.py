
import numpy as np;
import matplotlib.pyplot as plt

with open("score.out", 'r') as f:
    lines = f.readlines()
    arrs = [np.array(l.split(' ')[:-1]).astype(float) for l in lines]

for arr in arrs:
    plt.plot(arr, color='b', alpha=0.2)

endpoint_x = np.array([len(arr) for arr in arrs])
endpoint_y = np.array([arr[-1] for arr in arrs])
plt.scatter(endpoint_x, endpoint_y, color='orange', s=5)

print(endpoint_x)
print(endpoint_y)

plt.savefig('time_by_size')
