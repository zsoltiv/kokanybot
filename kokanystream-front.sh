#!/bin/sh

dev="eth0"
port="1338"
address=""

while [ "$address" = "" ]; do
    address="$(ip neigh show dev $dev | cut -d' ' -f1 | grep -oE '[^\s]+')"
done

echo "Client ${address}"

ffmpeg -f video4linux2 -fflags nobuffer -input_format mjpeg -video_size 1280x720 -i /dev/front-camera -fflags nobuffer -c:v copy -f mjpeg "udp://${address}:${port}"
