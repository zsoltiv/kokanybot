#!/bin/bash

# generate #define directives for Raspberry Pi pins (BCM)

IFS='
'

for line in $(gpioinfo | sed '/gpiochip.*$/d') ; do
    name="$(echo $line | cut -f2 -d'"')"
    value="$(echo $line | grep -E -o '[0-9]+:' | tr -d ':')"
    echo "#define ${name} ${value}"
done
