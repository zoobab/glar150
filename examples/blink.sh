#!/bin/sh
# Blink a LED every second

GPNR="1"
SLEEP="1"
GPPATH="/sys/class/gpio"
GPIODIR="${GPPATH}/gpio${GPNR}"

# export the GPIO pin
if [ -d "${GPIODIR}" ]
then
    echo "${GPIODIR} found"
else
    echo "${GPIODIR} not found, creating it"
    echo ${GPNR} > ${GPPATH}/export
fi

# Initalize the GPIO direction
echo out > ${GPIODIR}/direction

# Loop
while true; do
    echo 1 > ${GPIODIR}/value
    sleep $SLEEP
    echo 0 > ${GPIODIR}/value
    sleep $SLEEP
done
