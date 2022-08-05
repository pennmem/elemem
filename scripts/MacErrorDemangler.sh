#! /bin/bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: $(basename $0) <load address> <line address>"
fi

atos -o ../dist/Elemem -l $1 $2  

# How to find the load address of a running Elemem
# sample $(ps ax | grep Elemem | awk '{ print $1; exit }') 1 2>/dev/null | grep "Load Address"

#https://stackoverflow.com/questions/54320054/how-to-use-atos-or-addr2line-on-mac-os-x
#https://gist.github.com/bmatcuk/c55a0dd4f8775a3a2c5a
#https://gist.github.com/jerrykrinock/6701316
#https://stackoverflow.com/questions/10301542/getting-process-base-address-in-mac-osx



