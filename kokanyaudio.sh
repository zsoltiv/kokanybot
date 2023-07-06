#!/bin/sh

dev="eth0"
port="1340"

address="$(ip -4 -br a show $dev | sed -E 's/\s+/ /g ; s/\/[0-9]+//' | cut -d' ' -f 3)"
card="$(arecord -l | grep card | cut -d' ' -f2 | tr -d ':')"
echo $address

ffmpeg -f alsa -channels 1 -i hw:$card,0 -c:a aac -f mpegts "tcp://${address}:${port}?listen=1"
