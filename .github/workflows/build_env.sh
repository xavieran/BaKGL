#!/bin/bash

# For gcc
sudo add-apt-repository ppa:ubuntu-toolchain-r/ppa
#sudo add-apt-repository "deb http://ppa.launchpad.net/ubuntu-toolchain-r/ppa/ubuntu kinetic main" gcc 12 is not available in 20.04...
sudo apt-get install g++-11

# For clang
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 16 all

# Other deps
sudo apt-get install freeglut3-dev libsdl2-dev libgl-dev libglew-dev libxinerama-dev libxcursor-dev libxi-dev libxrandr-dev libx11-dev libfluidsynth-dev
