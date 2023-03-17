mkdir -p ../ext

# checkout the external libraries
cd ../ext
git clone https://github.com/sonydevworld/spresense-arduino-compatible.git
cd spresense-arduino-compatible && git checkout tags/v2.6.0 && cd -

git clone https://github.com/adafruit/Adafruit_BMP085_Unified.git
cd Adafruit_BMP085_Unified && git checkout tags/1.1.1 && cd -

git clone https://github.com/adafruit/Adafruit_L3GD20_U.git
cd Adafruit_L3GD20_U && git checkout tags/2.0.1 && cd -

git clone https://github.com/adafruit/Adafruit_LSM303DLHC.git
cd Adafruit_LSM303DLHC && git checkout tags/1.0.4 && cd -

git clone https://github.com/adafruit/Adafruit_Sensor_Calibration.git
cd Adafruit_Sensor_Calibration && git checkout tags/1.1.3 && cd -

git clone https://github.com/arduino-libraries/ArduinoBLE.git
cd ArduinoBLE && git checkout b40f618 && cd -

git clone https://github.com/lvgl/lvgl.git
cd lvgl && git checkout tags/v8.3.5 && cd -

git clone https://github.com/philj404/SimpleSerialShell.git
cd SimpleSerialShell && git checkout 14b80b0 && cd -

git clone https://github.com/Bodmer/TFT_eSPI.git
cd TFT_eSPI && git checkout tags/v2.5.0 && cd -

git clone https://github.com/baggio63446333/QZQSM.git
cd QZQSM && git checkout b774320 && cd -

git clone https://github.com/thijse/Arduino-Log.git
cd Arduino-Log && git checkout 3f4fcf5 && cd -

git clone https://github.com/bblanchon/ArduinoJson.git
cd ArduinoJson && git checkout tags/v6.20.0 && cd -

git clone https://github.com/adafruit/Adafruit_Sensor.git
cd Adafruit_Sensor && git checkout tags/1.1.7 && cd -

git clone https://github.com/Densaugeo/base64_arduino.git
cd base64_arduino && git checkout ac168f5 && cd -

# apply patches
cd Adafruit_LSM303DLHC && git apply ../../patches/Adafruit_LSM303DLHC.patch && cd -
cd Adafruit_Sensor_Calibration && git apply ../../patches/Adafruit_Sensor_Calibration.patch && cd -
cd ArduinoBLE && git apply ../../patches/ArduinoBLE.patch && cd -
cd ArduinoJson && git apply ../../patches/ArduinoJson.patch && cd -
cd TFT_eSPI && git apply ../../patches/TFT_eSPI.patch && cd -
cd lvgl && git apply ../../patches/lvgl.patch && cd -
cd spresense-arduino-compatible && git apply ../../patches/spresense-arduino-compatible.patch && cd -
cp ../patches/lv_conf.h .


source ~/spresenseenv/setup

export LDFLAGS='--specs=nosys.specs'
export CFLAGS=' -MMD -fno-builtin -mabi=aapcs -Wall -Os -fno-strict-aliasing -fno-strength-reduce -fomit-frame-pointer -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -pipe -gdwarf-3 -ffunction-sections -fdata-sections -ggdb'
export CPPFLAGS=' -D_FORTIFY_SOURCE=0 -Wa,-mimplicit-it=thumb'

git clone https://gitlab.xiph.org/xiph/opus.git
cd opus && git checkout tags/v1.3.1 && cd -

cd opus
./autogen.sh
./configure --host=arm-none-eabi --prefix=$PWD/out --disable-rtcd --enable-fixed-point --disable-float-api --enable-float-approx --disable-doc --disable-extra-programs
sed -i 's/#define OPUS_ARM_MAY_HAVE_NEON_INTR 1//g' config.h 
make clean && make -j && make install
cd ..


git clone https://github.com/gcp/libogg.git

cd libogg
./autogen.sh
./configure --host=arm-none-eabi --prefix=$PWD/out
make clean && make -j && make install
export PKG_CONFIG_PATH=$PWD/out/lib/pkgconfig
cd ..


git clone https://github.com/kfish/liboggz.git

sed -i 's/    if ((bytes = read (fileno(oggz->file), buf, n)) == 0) {/    if ((bytes = fread (fileno(oggz->file), 1, buf, n)) == 0) {/g' liboggz/src/liboggz/oggz_io.c
sed -i 's/  return read (fileno(f), buf, n);/  return fread (fileno(f), 1, buf, n);/g' liboggz/src/examples/read-io.c

cd liboggz
./autogen.sh
sed -i 's/SUBDIRS = liboggz tools tests examples/SUBDIRS = liboggz tests examples/g' src/Makefile.in 
./configure --host=arm-none-eabi --prefix=$PWD/out --disable-examples
make clean && make -j && make install
