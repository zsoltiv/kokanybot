#!/bin/sh

dev="eth0"
port="1338"
address="$(ip -4 -br a show $dev | sed -E 's/\s+/ /g ; s/\/[0-9]+//' | cut -d' ' -f 3)"

rpicam-vid --width 1296 --height 972 -n --denoise cdn_off -t 0 --listen --flush -o "tcp://${address}:${port}"
