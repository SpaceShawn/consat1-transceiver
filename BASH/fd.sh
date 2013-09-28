#!/bin/bash
PORT=/dev/ttyUSB0
exec 3<>$PORT
head -n "1" 0<&3 &
wait_pid=$!

printf $'\x48\x65\x10\x01\x00\x00\x11\x43\x00\x00' > $PORT

cat - 1>&3

wait $wait_pid:
trap="if kill -0 $wait_pid ; then kill -TERM $wait_pid ; fi"
trap "$trap" SIGINT SIGKILL SIGTERM

exec 3>&-
