#!/bin/sh
DELAY=$2
PIN=$1
if [ $# -lt 1 ]; then
  PIN=56
fi

if [ $# -lt 2 ]; then
  DELAY=.1
fi

echo $PIN > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio$PIN/direction

COUNTER=0
while [ $COUNTER -lt 100 ]; do
	sleep $DELAY
	echo 0 > /sys/class/gpio/gpio$PIN/value
	sleep $DELAY
	echo 1 > /sys/class/gpio/gpio$PIN/value
	let COUNTER=COUNTER+1
done
echo $PIN > /sys/class/gpio/unexport

