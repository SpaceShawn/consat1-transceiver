#!/bin/bash
valid_device=0;

until [  $valid_device = 1 ]; do
    echo "Enter the device name, e.g. ttyS0";
    read device_id;
    device="/dev/$device_id";
    echo "Intending to use $device, is this correct? (Y/n)";
    read correct;
    if [ "$correct" = "Y" ] ; then
        echo "Configuring tty $device ...";
        stty -F $device raw speed cs8 -ignpar cread clocal -cstopb -echo
        #stty -F $PORT raw speed 9600 cs8 -ignpar -cstopb -echo
        valid_device=1;
    else    
        echo "Ditching $device, try again!";
    fi
done 

he100_listen () {
    while [ 1 ]; do
       cat $device; #dump whatever is on the device
       #READ=`dd if=$device count=1`; echo $READ; #dump one character at a time
       echo "Listening on $device ..."; 
       sleep 1;
       trap sigint_handler 2 
    done   
}

he100_noop () {
    echo "Transmitting noop on $device";
    printf $'\x48\x65\x10\x01\x00\x00\x11\x43' > $device;
    he100_listen;
}

he100_transmit () {
    echo "Transmitting message on $device";
    printf $'\x48\x65\x10\x03\x00\x05\x18\x4e\x48\x65\x6c\x6c\x6f\xf5\x8c' > $device;
    he100_listen;
}

he100_transmit_continuously () {
     while [ 1 ]; do
        echo "Transmitting message on $device ...";
        printf $'\x48\x65\x10\x03\x00\x05\x18\x4e\x48\x65\x6c\x6c\x6f\xf5\x8c' > $device;
        trap sigint_handler 2
    done       
    he100_listen;
}

sigint_handler () {
    echo " >> Caught SIGINT, breaking current loop"
    break
}

cleanup () {
    echo "No cleanup..."
}

running=1;
until [ $running = 0 ]; do
    echo "OPTIONS:"
    echo "Enter 0 to execute noop"
    echo "Enter 1 to listen continuously"
    echo "Enter 2 to transmit message"
    echo "Enter 3 to transmit continuously"
    echo "Enter q to quit"
    read reply;
    case "$reply" in 
        0)  echo "Sending noop and listening for response"; he100_noop; 
            ;;
        1)  echo "Listening continuously"; he100_listen;
            ;;
        2)  echo "Transmitting message"; he100_transmit;
            ;;
        3)  echo "Transmitting continuously"; he100_transmit_continuously;
            ;;
        q)  running=0;
            echo "Exiting...";
            cleanup
            exit 1;
            ;;
    esac
done
cleanup
exit 1
