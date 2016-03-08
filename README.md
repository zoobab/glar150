# Glar150

This is a demo for the GL-AR150 OpenWRT router, henceforth called the "bot". We show how to use the ZeroMQ Zyre library to control clusters of intelligent devices (henceforth also called "bots").

The demo shows how to control the three LED lights, and an external high-intensity LED lamp powered by 3V GPIO.

We use a cluster of bots, each with its own external lamp, and battery pack. The cluster joins a well-known WiFi network, and then self-discovers and interconnects using Zyre.

Each bot starts by connecting to the Zyre network and joining the `glar150` group. The bots then talk to each other and to a controller, to demonstrate their capabilities.
 
 
