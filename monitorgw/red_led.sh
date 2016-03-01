#!/bin/sh

REDLED="/sys/devices/platform/leds-gpio/leds/gl_ar150:wlan/brightness"

while true; do
    grep -q "ok" /tmp/ping_status
    if [ $? -eq 0 ]; then
    	echo "0" > ${REDLED}
    else
	echo "1" > ${REDLED}
    fi
sleep 1 
done
