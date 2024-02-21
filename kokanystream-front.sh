#!/bin/sh

dev="eth0"
port="1338"
address="$(arp -n -i $dev | tail -n1 | cut -d' ' -f1)"

rpicam-vid -n --denoise cdn_off -t 0 --inline --flush -o "udp://${address}:${port}"
