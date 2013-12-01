#!/bin/bash
stty -F /dev/ttyUSB0 raw speed cs8 -ignpar cread clocal -cstopb -echo
printf $'\x48\x65\x10\x01\x00\x00\x11\x43\x00\x00' > /dev/ttyUSB0
while [ 1 ]; do
    cat /dev/ttyUSB0
done
