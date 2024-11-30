#!/bin/bash

set -x
# For gcc
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt-get install gcc-14 g++-14

# For clang
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18 all

# Other deps
sudo apt-get install freeglut3-dev libsdl2-dev libgl-dev libglew-dev libxinerama-dev libxcursor-dev libxi-dev libxrandr-dev libx11-dev libfluidsynth-dev ninja-build
