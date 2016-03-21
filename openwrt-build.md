How to build the openwrt image
==============================

Just run the `openwrt-build.sh` script as a user (not root), tested under
ubuntu 14.04.

First of all, you have to be root without password prompt preferably, here an
example how to add the user `joe` for sudo, those steps have to be done with
the root user:

```
$ export MYUSER="joe"
$ echo "$MYUSER ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$MYUSER
$ chmod 0440 /etc/sudoers.d/$MYUSER
```

This has been tested under ubuntu 14.04, if you want other distributions, look
at the OpenWRT requirements here:

https://wiki.openwrt.org/doc/howto/buildroot.exigence

Then, just run the `openwrt-build.sh` script as a user (not root):

```
./openwrt-build.sh
```

At the end of the process, you should end up with a firmware bin file named
`openwrt-ar71xx-generic-gl-ar150-squashfs-sysupgrade.bin` which
contains glard and all the zmq libs in the `openwrt/bin/ar71xx` directory:

```
zoobab@sabayonx86-64 /home/zoobab/soft/glar150/openwrt/bin/ar71xx [13]$ ls   
md5sums                                                  openwrt-ar71xx-generic-uImage-lzma.bin   openwrt-ar71xx-generic-vmlinux.lzma
openwrt-ar71xx-generic-gl-ar150-squashfs-sysupgrade.bin  openwrt-ar71xx-generic-vmlinux-lzma.elf  packages
openwrt-ar71xx-generic-root.squashfs                     openwrt-ar71xx-generic-vmlinux.bin       sha256sums
openwrt-ar71xx-generic-root.squashfs-64k                 openwrt-ar71xx-generic-vmlinux.elf
openwrt-ar71xx-generic-uImage-gzip.bin                   openwrt-ar71xx-generic-vmlinux.gz
```

You should then upload this file in the default firmware of Gl.inet by going to
the web interface of the device, -> Advanced Settings -> User+passwd -> System
-> Backup/FlashFirmware -> Flash New Firmware Image -> Choose File -> Flash
Image

Do no power up the device, and wait 3 minutes at least.

The device will reboot with a 192.168.1.1 IP address, you should be able to:

a. ping if you put a static IP on your PC (like 192.168.1.2)
b. a DHCP address from your PC

From there, you should be able to reach the default web interface
http://192.168.1.1 or telnet to the device.

Sometimes the previous configuration is not erased, so it will keep its default
IP address (192.168.8.1), so if you want to wipe it out, you should login in
telnet or ssh, and run `firstboot` to erase the previous settings.
