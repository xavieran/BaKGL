#!/bin/bash

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo add-apt-repository ppa:savoury1/llvm-defaults-13

sudo apt-get update

sudo apt-get install llvm-toolchain-13 g++-11 freeglut3-dev libsdl-dev libsdl-mixer1.2-dev libsdl-sound1.2-dev libgl-dev libglew-dev libxinerama-dev libxcursor-dev libxi-dev libxrandr-dev libx11-dev
