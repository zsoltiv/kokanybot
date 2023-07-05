#!/bin/sh

dev="wlan0"
port="1338"

address="$(ip -4 -br a show $dev | sed -E 's/\s+/ /g ; s/\/[0-9]+//' | cut -d' ' -f 3)"
echo $address

ffmpeg -video_size 1280x720 -i /dev/video0 -c:v h264_v4l2m2m -b:v 3M -f mpegts "tcp://${address}:${port}?listen=1"
