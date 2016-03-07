#!/bin/ash

GW_IP="192.168.1.1"

ping -q -W1 -c2 ${GW_IP} > /dev/null
 
if [ $? -eq 0 ]; then
    echo "ok" > /tmp/ping_status
    exit 0
else
    echo "fail" > /tmp/ping_status
    exit 1
fi
