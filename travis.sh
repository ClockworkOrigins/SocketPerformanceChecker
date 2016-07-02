#!/usr/bin/env sh
set -evx
env | sort

mkdir build || true
mkdir build/$SPC_TARGET || true
cd build/$SPC_TARGET
cmake -DWITH_M2ETIS=$SPC_M2ETIS -DCMAKE_BUILD_TYPE=$SPC_BUILD_TYPE ../..
make
