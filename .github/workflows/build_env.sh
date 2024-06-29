#!/bin/bash

set -x
# For gcc
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
#sudo add-apt-repository "deb http://ppa.launchpad.net/ubuntu-toolchain-r/ppa/ubuntu kinetic main" gcc 12 is not available in 20.04...
sudo apt-get install gcc-13 g++-13
sudo apt-get install gcc-14 g++-14

# For clang
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 17 all

# Other deps
sudo apt-get install freeglut3-dev libsdl2-dev libgl-dev libglew-dev libxinerama-dev libxcursor-dev libxi-dev libxrandr-dev libx11-dev libfluidsynth-dev
