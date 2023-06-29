#!/bin/sh

dev="wlan0"
port="1338"

address="$(ip -4 -br a show $dev | sed -E 's/\s+/ /g ; s/\/[0-9]+//' | cut -d' ' -f 3)"
card="$(arecord -l | grep card | cut -d' ' -f2 | tr -d ':')"
echo $address

ffmpeg -video_size 1280x720 -i /dev/video0 -f alsa -channels 1 -i hw:$card,0 -c:a aac -c:v h264_v4l2m2m -b:v 3M -f mpegts "tcp://${address}:${port}?listen=1"
