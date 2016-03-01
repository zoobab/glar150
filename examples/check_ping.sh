#!/bin/sh
 
# -q quiet
# -c nb of pings to perform

GW_IP="192.168.1.1"
LED="/sys/devices/platform/leds-gpio/leds/gl_ar150:wlan/brightness"

ping -q -c3 ${GW_IP} > /dev/null
 
if [ $? -eq 0 ]
then
    echo "ok"
else
    # blink the RED LED of the GL-150
    while true; do echo 1 > ${LED} && sleep 0.2 && echo 0 > ${LED} && sleep 0.2 ; done
fi
