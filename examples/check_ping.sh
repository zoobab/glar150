#!/bin/ash

GW_IP="192.168.2.132"

ping -q -W2 -c2 ${GW_IP} > /dev/null
 
if [ $? -eq 0 ]; then
    echo "ok" > /tmp/ping_status
    exit 0
else
    echo "fail" > /tmp/ping_status
    exit 1
fi
