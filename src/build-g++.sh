#!/bin/bash
echo "Compiling fsb.cc with g++"
g++ -m32 -O2 -s -o fsb fsb.cc -luser32
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi
echo "Compilation successful. fsb created."
