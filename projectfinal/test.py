import os
import subprocess

bench = open("bench.txt", "w")
devnull = open(os.devnull, "w")

proc = subprocess.Popen(['sysbench', '--test=cpu', 'run'], stdout=bench, stderr=devnull)


print(f"pid: {proc.pid}")

with open("/proc/watch","a") as watch:
    watch.write(proc.pid)

# while True:
#     ret = proc.poll()
#     if ret is not None:
#         break

#     with open("/proc/watch", "r") as watch:
#         utime, stime = watch.readline().split(' ')
#         ram = watch.readline()
        
#         print(utime, stime, ram)

#     sleep(0.1)

proc.wait()
bench.close()
    