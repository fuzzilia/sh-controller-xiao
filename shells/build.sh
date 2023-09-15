#! /bin/bash

cd main
rm -rf out
FQBN=Seeeduino:nrf52:xiaonRF52840Sense:softdevice=s140v6,debug=l0
PROPS=`arduino-cli compile -b $FQBN --show-properties`
CPP_FLAGS=`echo "$PROPS" | grep ^compiler.cpp.flags | cut -d = -f 2- | sed -e 's/gnu++11/gnu++17/'`
ORIGINAL_EXTRA_FLAGS=`echo "$PROPS" | grep ^build.extra_flags | cut -d = -f 2-`
arduino-cli compile -b $FQBN \
  --build-property "compiler.cpp.flags=$CPP_FLAGS" \
  --build-property compiler.c.elf.cmd=arm-none-eabi-g++ \
  --build-property "build.extra_flags=$ORIGINAL_EXTRA_FLAGS -DSH_CONTROLLER_PRODUCTION" \
  --build-path out
uf2conv.py -c out/main.ino.hex --out out/sh-controller-nrf52-xiao.uf2 --family 0xADA52840
