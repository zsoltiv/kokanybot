#!/bin/sh

dev="eth0"
port="1338"
address=""

while [ "$address" = "" ]; do
    address="$(kokany-select-ip.sh)"
    echo "address is ${address}"
done

echo "Client ${address}"

ffmpeg -f video4linux2 -fflags nobuffer -input_format mjpeg -video_size 1280x720 -i /dev/front-camera -fflags nobuffer -c:v copy -f mjpeg "udp://${address}:${port}"
