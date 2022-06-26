from re import sub
import subprocess
import os
import time
import psutil

benchmark = open("benchmark.txt", "w")
# proc = subprocess.Popen(['sysbench', '--threads=4', 'cpu', 'run'],
#                         stdout=benchmark,
#                         stderr=open(os.devnull, "w"))
proc = subprocess.Popen(['7z', 'b'],
                        stdout=benchmark,
                        stderr=open(os.devnull, "w"))

print(f"PID: {proc.pid}")

PERIOD = 0.5
old_data_list = None
output = ''
while proc.poll() is None:
    with open("/proc/watch", "w") as f:
        out = ''
        for thread in psutil.Process(proc.pid).threads():
            out += str(thread.id) + ' '

        print(out, file=f, end='')

    with open("/proc/watch", "r") as f:
        lines = f.readlines()

    data_list = []
    for line in lines:
        data = line.strip()
        # process is exit
        if data.split(' ')[1] == '-1':
            continue
        # user time (ns)
        pid, utime, stime, mem = list(map(int, data.split(' ')))
        data_list.append((pid, utime, stime, mem))
        # print(pid, utime, stime, mem)

    for pid, utime, stime, mem in data_list:
        if old_data_list is not None:
            old_data = next(
                (data for data in old_data_list if data[0] == pid), None)
            if old_data is None:
                continue
            _, last_utime, last_stime, _ = old_data
            cpu_rate = (utime - last_utime) / \
                (PERIOD * 1e7)
            print(f"pid: {pid:5d}, cpu: {cpu_rate:6.2f}%, mem: {mem}B")

            output += f"pid: {pid:5d}, cpu: {cpu_rate:6.2f}%, mem: {mem}B\n"

    old_data_list = data_list.copy()

    print('')

    time.sleep(PERIOD)

benchmark.close()

with open("output.txt", "w") as f:
    print(output, file=f)
