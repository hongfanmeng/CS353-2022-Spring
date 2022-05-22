#!/bin/bash
bash run1.sh
sudo pkill test1
gcc test1.c -o test1 -pthread
taskset -c 0 sudo ./test1 &