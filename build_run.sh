#!/bin/bash

mkdir -p build
cd build
cmake .. 

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

cmake --build . -- -j4

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

./redsim