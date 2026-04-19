#!/bin/bash

# This script assumes you have ninja installed. If not please remove -GNinja, and simply invoke this with make.


mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja ..
ln -sf compile_commands.json ../compile_commands.json

ninja
