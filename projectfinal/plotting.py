import matplotlib.pyplot as plt
import numpy as np
from more_itertools import chunked

with open("sum.txt", "r") as f:
    lines = f.readlines()

cpu_rate = []
mem_rate = []
counter = []
for line in lines:
    cpu, mem, cnt = list(map(float, line.strip().split(' ')))
    cpu_rate.append(cpu)
    mem_rate.append(mem)
    counter.append(cnt)

counter = np.asarray(counter) - counter[0]
counter = list(counter)

avg_counter = np.asarray([sum(x) / len(x) for x in chunked(counter, 5)])
avg_cpu_rate = np.asarray([sum(x) / len(x) for x in chunked(cpu_rate, 5)])
avg_mem_rate = np.asarray([sum(x) / len(x) for x in chunked(mem_rate, 5)])

fig, axs = plt.subplots(2, 1)

color = 'tab:red'
axs[0].set_xlabel('time (s)')
axs[0].set_ylabel('CPU usage (%)', color=color)
axs[0].plot(avg_counter, avg_cpu_rate, color=color)
axs[0].tick_params(axis='y', labelcolor=color)


color = 'tab:blue'
# we already handled the x-label with ax1
axs[1].set_ylabel('Memory Freq (MB/s)', color=color)
axs[1].plot(avg_counter, avg_mem_rate / 1024 / 1024, color=color)
axs[1].tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.savefig("sysbench-mem.png")
