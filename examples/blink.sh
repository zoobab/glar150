#!/bin/sh
# Blink a LED every second

GPNR="1"
SLEEP="1"

# export the GPIO
echo 1 > /sys/class/gpio/export

# Initalize the GPIO direction
echo out > /sys/class/gpio/gpio${GPNR}/direction

while true; do
echo 1 > /sys/class/gpio/gpio${GPNR}/value
sleep $SLEEP
echo 0 > /sys/class/gpio/gpio${GPNR}/value
sleep 
done
