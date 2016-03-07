#!/bin/sh

PINGFILE="/tmp/ping_status"

while true; do
    if [ -f "${PINGFILE}" ]; then
    	grep -q "ok" ${PINGFILE}
    	if [ $? -eq 0 ]; then
		screen -d -L -m /usr/bin/blink_shell -i wlan0 -v
		exit 0
    	else
		sleep 3
    	fi
    fi
done
