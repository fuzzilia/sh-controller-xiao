#! /bin/bash

cd main
rm -rf out
FQBN=adafruit:nrf52:itsybitsy52840:softdevice=s140v6,debug=l0,debug_output=serial
PROPS=`arduino-cli compile -b $FQBN --show-properties`
CPP_FLAGS=`echo "$PROPS" | grep ^compiler.cpp.flags | cut -d = -f 2- | sed -e 's/gnu\+\+11/gnu\+\+17/'`
arduino-cli compile -b $FQBN \
  --build-property "compiler.cpp.flags=$CPP_FLAGS" \
  --build-property compiler.c.elf.cmd=arm-none-eabi-g++ \
  --build-path out