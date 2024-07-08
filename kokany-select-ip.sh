#!/bin/bash

address=""
dev="eth0"

IFS='
'

while [ "$address" = "" ]; do
    for neighbor in $(ip neigh show dev $dev | cut -d' ' -f1 | grep -oE '[^\s]+') ; do
        ping -W 1 -c 1 "$neighbor" &> /dev/null
        if [ "$?" = 0 ] ; then
            address="$neighbor"
            echo "$address"
            exit 0
        fi
    done
done
