make all
rmmod maptest
insmod maptest.ko
./maptest_test
dmesg | tail -10