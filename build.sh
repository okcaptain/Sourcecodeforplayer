#!/bin/bash

NDK=/home/yogiczy/media3/android-ndk-r27
export NDK

ORIGINAL_PATH=$(pwd)

cd ./av3a_binaural_render/AudioDecoder/av3a_binaural_render/build/linux/
chmod +x ./build_av3a_render.sh
./build_av3a_render.sh

cd "$ORIGINAL_PATH"

cd ./av3adecoder/avs3Decoder/build/aarch64/
chmod +x ./build.sh
./build.sh
