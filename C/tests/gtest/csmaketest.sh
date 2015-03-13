#! /bin/bash
if [ -z "$BASH_VERSION" ]; then exec bash "$0" "$@"; fi;
# csmake.sh
# Copyright (C) 2014 spaceconcordia <spaceconcordia@mustang>
#
# Distributed under terms of the MIT license.
#
# colors: echo -e "${red}Text${NC}"
NC='\e[0m';black='\e[0;30m';darkgrey='\e[1;30m';blue='\e[0;34m';lightblue='\e[1;34m';green='\e[0;32m';lightgreen='\e[1;32m';cyan='\e[0;36m';lightcyan='\e[1;36m';red='\e[0;31m';lightred='\e[1;31m';purple='\e[0;35m';lightpurple='\e[1;35m';orange='\e[0;33m';yellow='\e[1;33m';lightgrey='\e[0;37m';yellow='\e[1;37m';

declare -a SysReqs=('dialog' 'whiptail')
for item in ${SysReqs[*]}; do command -v $item >/dev/null 2>&1 || { echo >&2 "I require $item but it's not installed.  Aborting."; exit 1; }; done

confirm () { #https://stackoverflow.com/a/3232082
    read -r -p "${1:-[y/N]} [y/N] " response
    case $response in
        [yY][eE][sS]|[yY]) 
            true
            ;;
        *)
            false
            ;;
    esac
}

fail () {
    echo -e "${red}$1 ABORTING...${NC}" 
    exit 1
}
quit () {
    echo -e "$1 exiting..."
    exit 0
}

make-tests () {
    if $1 == "Q6" ; then
        if [ -f "/usr/local/lib/mbgcc/bin/microblazeel-xilinx-linux-gnu-c++" ] ; then
            make clean
            make buildQ6
        fi
    else
        make clean
        make buildBin
    fi
}

make-clean-gtest () {
    confirm "Would you like to run the live test (requires radio to be present and active)?" && live=1
    make clean
    if make-tests ; then
        $1 ./he100_lib_testPC
        [ $live ] && sudo ./he100_live_radio_testPC
        return 0
    fi;
}

for arg in "$@"; do
    case $arg in
        "test")
            make-clean-gtest    
        ;;
        "gdb")
            make-clean-gtest gdb
    esac
done

CURRENT_DIR="${PWD##*/}"
if [ "$CURRENT_DIR" != "gtest" ]; then fail "This script must be run from HE100_lib/C, not $CURRENT_DIR"; fi;

usage="usage: csmaketest.sh [options] "
if [ $# -eq 0 ]; then 
    echo "No arguments supplied... $usage"; 
    echo "Running default conditions..."
    make-clean-gtest
fi
