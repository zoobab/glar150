#! /bin/sh

GLARD_UP=0
GW_IP="192.168.43.1"

while true; do
    #   If wireless network is up, start glard
    ping -q -W1 -c2 ${GW_IP} > /dev/null
    if [ $? -eq 0 -a $GLARD_UP -eq 0 ]; then
        GLARD_UP=1
        screen -d -L -m /usr/bin/glard -v
    else
        #   Stop glard if necessary
        if [ $GLARD_UP -eq 1 ]; then
            kill -9 `pgrep glard`
            GLARD_UP=0
            #   flash red LED here (error)...
        else
            #   flash green LED here (waiting)...
            true
        fi
    fi
    sleep 2
done
