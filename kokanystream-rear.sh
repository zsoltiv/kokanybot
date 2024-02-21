#!/bin/sh

dev="eth0"
port="1341"
address="$(arp -n -i $dev | tail -n1 | cut -d' ' -f1)"

ffmpeg -fflags nobuffer -f v4l2 -framerate 30 -input_format mjpeg -video_size 1280x720 -i /dev/rear-camera -c:v copy -f mjpeg "udp://${address}:${port}?buffer_size=65536&pkt_size=512"
