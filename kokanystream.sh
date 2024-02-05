#!/bin/sh

dev="eth0"
port="1338"

address="$(ip -4 -br a show $dev | sed -E 's/\s+/ /g ; s/\/[0-9]+//' | cut -d' ' -f 3)"
echo $address

ffmpeg -fflags nobuffer -framerate 30 -input_format mjpeg -video_size 1280x720 -i /dev/video0 -c:v copy -f mjpeg "tcp://${address}:${port}?listen=1"
