#!/bin/bash
sudo pkill test
sudo pkill test1
gcc test.c -o test -pthread
taskset -c 0 nice -n 1 ./test &
taskset -c 0 nice -n 5 ./test &