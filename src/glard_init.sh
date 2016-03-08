#! /bin/sh

GLARD_UP=0
GW_IP="192.168.43.1"

while true; do
    #   If wireless network is up, start glard
    ping -q -W1 -c2 ${GW_IP} > /dev/null
    if [ $? -eq 0 ]; then
        if [ $GLARD_UP -eq 0 ]; then
            GLARD_UP=1
            blink 2 0.2
            screen -d -L -m /usr/bin/glard -v
        fi
    else
        #   Stop glard if necessary
        if [ $GLARD_UP -eq 1 ]; then
            kill -9 `pgrep glard`
            GLARD_UP=0
            #   flash red LED here (error)...
        else
            blink 1 0.2
        fi
    fi
    sleep 1
done
