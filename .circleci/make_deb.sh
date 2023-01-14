#!/bin/bash

mkdir -p deb_linux/usr/bin
cp ../build/FlipperAM deb_linux/usr/bin/flipperam
dpkg-deb --build deb_linux && mv deb_linux.deb ../build/flipperam.deb
