# Kókánybot

This repository contains the software used by Kókánybot which is being built for the 2024 RoboCup competition.
It's based on the previous year's code, which you can read after doing a `git checkout tags/2023`.

## Packages
```
sudo apt install git make autoconf autoconf-archive gcc gpiod libgpiod-dev ffmpeg libi2c-dev libavformat-dev libavutil-dev rpicam-apps
```

## OS

Raspberry Pi OS 64 bit
SSH enabled

### `/boot/firmware/config.txt`

```
dtparam=i2c1=on

dtoverlay=kokanystepperctl
dtoverlay=i2c-pwm-pca9685a,addr=0x40
```
