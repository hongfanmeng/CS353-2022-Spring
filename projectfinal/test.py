import subprocess
import os
import time

benchmark = open("benchmark.txt", "w")
proc = subprocess.Popen(['sysbench', '--test=cpu', 'run'], 
                        stdout=benchmark,
                        stderr=open(os.devnull, "w"))

print(f"PID: {proc.pid}")

with open("/proc/watch", "w") as f:
    print(proc.pid, file=f)

# while process not end
first = True
last_utime = last_stime = last_mem = 0
PERIOD = 0.5
while proc.poll() is None:

    with open("/proc/watch", "r") as f:
        data = f.readline().strip()
        if data == '-1':
            break
        # user time (ns)
        utime, stime, mem = list(map(int, data.split(' ')))
        # print(utime, stime, mem)

    cpu_rate = (utime - last_utime) / (PERIOD * 1e9 * int(os.sysconf('SC_CLK_TCK')))

    if first is True:
        first = False
    else:
        print(f"cpu: {cpu_rate: .2f}%, mem: {mem * 4}B")
        
    last_utime, last_stime, last_mem = utime, stime, mem

    time.sleep(PERIOD)

benchmark.close()