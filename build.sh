#!/bin/bash

# This script assumes you have ninja installed. If not please remove -GNinja, and simply invoke this with make.

OFFLINE_FLAGS=""

DEPS_DIR=$HOME/Code/deps

for arg in "$@"; do
    case "$arg" in
        --offline|-o)
            OFFLINE_FLAGS=" \
    -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
    -DFETCHCONTENT_SOURCE_DIR_GLM=$DEPS_DIR/glm \
    -DFETCHCONTENT_SOURCE_DIR_GLFW=$DEPS_DIR/glfw \
    -DFETCHCONTENT_SOURCE_DIR_LUA=$DEPS_DIR/lua \
    -DFETCHCONTENT_SOURCE_DIR_LUABRIDGE3=$DEPS_DIR/LuaBridge3 \
    -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=$DEPS_DIR/googletest \
    -DFETCHCONTENT_SOURCE_DIR_AUDIOCODECS=$DEPS_DIR/AudioCodecs \
    -DFETCHCONTENT_SOURCE_DIR_SDLMIXERX=$DEPS_DIR/SDL-Mixer-X \
    "
            ;;
        *)
            echo "Unknown option: $arg" >&2
            exit 1
            ;;
    esac
done

mkdir -p build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja $OFFLINE_FLAGS ..
ln -sf $(pwd)/compile_commands.json $(pwd)/../compile_commands.json

ninja
