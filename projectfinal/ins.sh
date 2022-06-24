make clean > /dev/null
make all > /dev/null
rmmod watch.ko || true
insmod watch.ko
make clean
