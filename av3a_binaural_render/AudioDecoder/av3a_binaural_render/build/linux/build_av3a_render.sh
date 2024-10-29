#!/bin/bash

NDK=/home/yogiczy/media3/android-ndk-r27
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64

#需要在当前目录下新建temp文件夹
export TMPDIR=temp

function build_android {
echo "Compiling av3a_binaural_render for $CPU"

mkdir -p ../../../../bin/{arm64-v8a,armeabi-v7a,x86,x86_64}

#make -f av3a_binaural_render_android.mk clean
make -f av3a_binaural_render_android.mk clean
make -f av3a_binaural_render_android.mk

make -f av3a_binaural_render_android_arm7.mk clean
make -f av3a_binaural_render_android_arm7.mk

make -f av3a_binaural_render_android_x86.mk clean
make -f av3a_binaural_render_android_x86.mk

make -f av3a_binaural_render_android_x86_64.mk clean
make -f av3a_binaural_render_android_x86_64.mk

echo "The Compilation of av3a_binaural_render for $CPU is completed"

mkdir -p ../../../../../ffmpeg/ffmpeg-6.1/android-libs
cp -r ../../../../bin/* ../../../../../ffmpeg/ffmpeg-6.1/android-libs/
}


build_android


