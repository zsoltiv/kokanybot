#!/bin/sh

dev="eth0"
port="1338"
address="$(nmap -sn '192.168.69.*' | grep -oE '192\.168\.69\.1[0-9]')"

rpicam-vid --codec h264 -n --denoise cdn_off -t 0 --inline --flush -o "udp://${address}:${port}"
