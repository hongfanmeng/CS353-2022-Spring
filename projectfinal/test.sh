sysbench --test=cpu run > /dev/null 2>/dev/null & 
echo $! > /proc/watch
echo PID: $!
sleep 1
cat /proc/watch
sleep 1
cat /proc/watch
sleep 1
cat /proc/watch
sleep 1
cat /proc/watch