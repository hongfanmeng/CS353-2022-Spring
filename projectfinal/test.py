import subprocess
import os
import time
import psutil
import math

benchmark = open("benchmark.txt", "w")
# proc = subprocess.Popen(['sysbench', '--threads=4', 'cpu', 'run'],
#                         stdout=benchmark,
#                         stderr=open(os.devnull, "w"))
proc = subprocess.Popen(['7z', 'b'],
                        stdout=benchmark,
                        stderr=open(os.devnull, "w"))

print(f"PID: {proc.pid}")


def convert_size(size_bytes):
    if size_bytes == 0:
        return "0B"
    size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
    i = int(math.floor(math.log(size_bytes, 1024)))
    p = math.pow(1024, i)
    s = round(size_bytes / p, 2)
    return "%s%s" % (s, size_name[i])


PERIOD = 0.01
lines_list = []
counter_list = []
while proc.poll() is None:
    with open("/proc/watch", "w") as f:
        out = ''
        try:
            for thread in psutil.Process(proc.pid).threads():
                out += str(thread.id) + ' '
        except:
            break
        print(out, file=f, end='')

    with open("/proc/watch", "r") as f:
        lines = f.readlines()

    # record data and time
    lines_list.append(lines.copy())
    counter_list.append(time.perf_counter())

    time.sleep(PERIOD)

origin = ''
data_list = []
couter_list = []
for lines, counter in zip(lines_list, counter_list):
    stat_list = []
    for line in lines:
        data = line.strip()
        # process is exit
        if data.split(' ')[1] == '-1':
            continue
        # pid, user time, system time, memory
        pid, utime, stime, mem = list(map(int, data.split(' ')))
        stat_list.append((pid, utime, stime, mem))
        # write origin data to file
        origin += f'{pid} {utime} {stime} {mem} {counter}\n'
    data_list.append(stat_list)

with open("origin.txt", "w") as f:
    print(origin, file=f)

output = ''
last_counter = None
last_stat_list = None
sum_stat_list = []
for stat_list, counter in zip(data_list, counter_list):
    delta_t = counter - last_counter if last_counter is not None else None
    if delta_t is not None:
        sum_stat_list.append((0, 0, counter))
    for pid, utime, stime, mem in stat_list:
        # calc stat from second
        if delta_t is None:
            continue
        # find old stat with same pid
        old_stat = next((stat for stat in last_stat_list
                         if stat[0] == pid), None)
        # process is killed
        if old_stat is None:
            continue
        # calc cpu rate and mem rate
        delta_t = counter - last_counter
        last_utime = old_stat[1]
        cpu_rate = (utime - last_utime) / (delta_t * 1e7)
        mem_rate = mem / delta_t

        tmp = f"pid: {pid}, cpu: {cpu_rate:6.2f}%, mem: {convert_size(mem_rate)}/s"
        print(tmp)
        output += tmp + '\n'

        sum_stat_list[-1] = tuple(map(sum, zip(
            sum_stat_list[-1], (cpu_rate, mem_rate, 0)
        )))

    if len(sum_stat_list) > 0:
        cpu_rate, mem_rate, _ = sum_stat_list[-1]
        tmp = f"total cpu: {cpu_rate:6.2f}%\n" + \
            f"total mem: {convert_size(mem_rate)}/s"
        print(tmp)
        output += tmp + '\n'

    last_counter = counter
    last_stat_list = stat_list
    print('')
    output += '\n'

with open("output.txt", "w") as f:
    print(output, file=f)

with open("sum.txt", "w") as f:
    for cpu_rate, mem_rate, counter in sum_stat_list:
        print(cpu_rate, mem_rate, counter, file=f)

benchmark.close()
