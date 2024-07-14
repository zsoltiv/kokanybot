# Kókánybot

This repository contains the software used by Kókánybot which is being built for the 2024 RoboCup competition.
It's based on the previous year's code, which you can read after doing a `git checkout tags/2023`.

## Packages
```
sudo apt install git make autoconf dnsmasq autoconf-archive libtool pkg-config gcc ffmpeg libi2c-dev libavformat-dev libavutil-dev
sudo systemctl enable dnsmasq
git clone https://github.com/zsoltiv/libhwpwm.git
cd libhwpwm
make
sudo make install
cd
git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
cd libgpiod
./autogen.sh --enable-tools=yes
make
sudo make install
```

Afterwards, build this repo with the Makefile and reboot

## OS

Raspberry Pi OS 64 bit
SSH enabled

### `/boot/firmware/config.txt`

```
dtparam=i2c_arm=on

dtoverlay=kokanystepperctl
dtoverlay=i2c-pwm-pca9685a,addr=0x40
```

### `/etc/modules-load.d/i2c.conf`

```
i2c-dev
```
