#!/bin/bash

origin_directory=$PWD
script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

build_directory="$script_directory/../build"

mkdir -p $build_directory
cd $build_directory

cmake ..
cmake --build . --parallel 10

cd $origin_directory
