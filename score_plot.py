
import numpy as np;
import matplotlib.pyplot as plt

with open("score.out", 'r') as f:
    lines = f.readlines()
    arrs = [np.array(l.split(' ')[:-1]).astype(float) for l in lines]

all_x = arrs
all_y = [np.arange(len(arr)) for arr in arrs]

for x, y in zip(all_x, all_y):
    plt.plot(x, y, color='b', alpha=0.2)

endpoint_x = np.array([arr[-1] for arr in all_x])
endpoint_y = np.array([arr[-1] for arr in all_y])
plt.scatter(endpoint_x, endpoint_y, color='orange', s=5)

print(endpoint_x)
print(endpoint_y)

plt.grid()
plt.xlabel('Time')
plt.ylabel('Size')

plt.savefig('size_over_time')

times = []
for i in range(max(endpoint_y)):
    times.append(np.array([arrs[x][i+1] - arrs[x][i] for x in range(len(arrs)) if len(arrs[x]) > i+1]).mean())

cum_times = np.array(times).cumsum()

plt.figure()
window_size = 5
cutoff = 700

area = 900
hamRateLen = 200


rolling_avg = (cum_times[window_size:] - cum_times[:-window_size])/window_size
plt.plot(rolling_avg)
plt.vlines([cutoff, cutoff], 0, max(rolling_avg), colors='red')
plt.plot([area-hamRateLen, area], [hamRateLen/2, 0], color='green')
plt.savefig('avg_times')