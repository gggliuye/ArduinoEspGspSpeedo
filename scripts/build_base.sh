#!/usr/bin/env bash
set -e

# export PATH=~/Arduino/bin:$PATH

MODULE=""
if [ $# -lt 2 ]; then
    echo "please set chip, port and module name"
    echo "  example :  ./scripts/build_base.sh esp32c3 ttyACM0/ttyUSB0 ESP32CAN"
    echo "  monitor :  ./scripts/build_base.sh esp32c3 ttyUSB0"
    exit 1
else
    CHIP=$1
    PORT=$2
    MODULE=$3
    echo "Build chip : " $CHIP
    echo "Build module : " $MODULE
    echo "Build port : " $PORT
fi

if [ "$MODULE" = "" ] ; then
    arduino-cli monitor -p /dev/${PORT} -c baudrate=115200 -b esp32:esp32:$CHIP
    exit 1
fi

# BOARD_OPTIONS=""
BOARD_OPTIONS="--board-options CDCOnBoot=cdc"
# if [ "$CHIP" = esp32cam ] ; then
#     BOARD_OPTIONS=""
# fi
echo "Build options : " $BOARD_OPTIONS

./scripts/install_libs.sh

echo "==============================="
echo "=========== COMPILE ==========="
echo "==============================="

mkdir -p build/$MODULE

arduino-cli compile \
--fqbn esp32:esp32:$CHIP \
--log \
--libraries libraries,submodules \
--output-dir build/$MODULE \
--build-cache-path build-cache/$MODULE \
${BOARD_OPTIONS} \
$MODULE


echo "==============================="
echo "======== UPLOAD COM ==========="
echo "==============================="
arduino-cli upload \
--fqbn esp32:esp32:$CHIP \
--port /dev/${PORT} \
--input-dir build/$MODULE \
--verbose \
${BOARD_OPTIONS} \
$MODULE

# /home/yeliu/.arduino15/packages/esp32/tools/esptool_py/4.9.dev3/esptool \
# --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
# write_flash 0x10000 "./build/can/ESP32CAN.ino.bin"

echo "==============================="
echo "=========== MONITOR ==========="
echo "==============================="

arduino-cli monitor -p /dev/${PORT} -c baudrate=115200 -b esp32:esp32:$CHIP
