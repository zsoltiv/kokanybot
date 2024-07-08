#!/bin/sh

dev="eth0"
port="1341"
address=""

while [ "$address" = "" ]; do
    address="$(kokany-select-ip.sh)"
done

echo "Client ${address}"

ffmpeg -fflags nobuffer -f v4l2 -framerate 30 -input_format mjpeg -video_size 1280x720 -i /dev/rear-camera -c:v mjpeg -f mjpeg -vf vflip "udp://${address}:${port}?buffer_size=65536&pkt_size=512"
