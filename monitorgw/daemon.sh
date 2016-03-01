#!/bin/sh
while true; do /root/blink/examples/check_ping.sh ; sleep 4; done &
while true; do /root/blink/examples/red_led.sh ; done &
