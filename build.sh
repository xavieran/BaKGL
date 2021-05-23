#!/bin/bash


mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja ..
ln -s compile_commands.json ../compile_commands.json

ninja
