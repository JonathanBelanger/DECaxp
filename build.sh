#! /usr/bin/bash

[[ ! -d build ]] && mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
cd ..
exit 0
