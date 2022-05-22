make all
rmmod mtest
insmod mtest.ko
./mtest_test
dmesg | tail -20