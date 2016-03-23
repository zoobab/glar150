#!/bin/bash

set -x

if [[ $(id -u) -eq 0 ]] ; then
    echo "Please do not run as root!";
    exit 1;
fi

DEBIAN_FRONTEND=noninteractive sudo apt-get update -y -q
DEBIAN_FRONTEND=noninteractive sudo apt-get install -y -q --force-yes build-essential subversion git-core libncurses5-dev zlib1g-dev gawk flex quilt libssl-dev xsltproc libxml-parser-perl mercurial bzr ecj cvs unzip wget

git clone https://github.com/mirrors/openwrt.git
cd openwrt/
git checkout 4eba46dc3a80529146329a5f28629429d6fb3cd5

echo "src-git zmq https://github.com/zoobab/openwrt-zmq-packages.git" >> feeds.conf.default
uniq feeds.conf.default > feeds.conf.default
./scripts/feeds update zmq
./scripts/feeds install -p zmq
./scripts/feeds update -a
./scripts/feeds install -a

# "=y" means in the firmware image, "=m" means as an external *.ipk package
echo "CONFIG_TARGET_ar71xx_generic_GL-AR150=y" > .config
echo "CONFIG_PACKAGE_glard=y" >> .config
make defconfig
make 
