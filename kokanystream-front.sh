#!/bin/sh

dev="eth0"
port="1338"
address=""

while [ "$address" = "" ]; do
    address="$(ip neigh show dev $dev | cut -d' ' -f1 | grep -oE '[^\s]+')"
done

echo "Client ${address}"

rpicam-vid --codec h264 --width 1296 --height 972 -n --denoise cdn_off -t 0 --inline --flush -o "udp://${address}:${port}"
