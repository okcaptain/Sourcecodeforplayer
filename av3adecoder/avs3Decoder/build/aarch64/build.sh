#!/bin/bash

mkdir -p ../../../bin/{arm64-v8a,armeabi-v7a,x86,x86_64}

make -f ./AVS3AudioDec_arm_android.mk clean
make -f ./AVS3AudioDec_arm_android.mk

make -f ./AVS3AudioDec_arm7_android.mk clean
make -f ./AVS3AudioDec_arm7_android.mk

make -f ./AVS3AudioDec_x86_android.mk clean
make -f ./AVS3AudioDec_x86_android.mk

make -f ./AVS3AudioDec_x86-64_android.mk clean
make -f ./AVS3AudioDec_x86-64_android.mk

mkdir -p ../../../../ffmpeg/ffmpeg-6.1/android-libs
cp -r ../../../bin/* ../../../../ffmpeg/ffmpeg-6.1/android-libs/
