# Application makefile

# Command name (Public function 'int <APPNAME>_main(void)' required)
APPNAME = spresense

# Application execute priority (Range: 0 ~ 255, Default: 100)
PRIORITY = 100

# Application stack memory size (Default: 2048) <- really?
STACKSIZE = 8096

# Additional assembler source files (*.S)
ASRCS =

mkfile_path := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Source files of this project
CODE_BIKE_REPO = $(mkfile_path)..
CODE_BIKE_MAIN = $(CODE_BIKE_REPO)/src/main

# The Arduino Spresense Core
SPRESENSE_ARDUINO_CORE = $(CODE_BIKE_REPO)/ext/spresense-arduino-compatible/Arduino15/packages/SPRESENSE
CORE = $(SPRESENSE_ARDUINO_CORE)/hardware/spresense/1.0.0/cores/spresense

# The Arduino Spresense Library
LIBRARIES_SPRESENSE = $(SPRESENSE_ARDUINO_CORE)/hardware/spresense/1.0.0/libraries

# 3rd Party Arduino Libraries
LIBRARIES_ARDUINO = $(CODE_BIKE_REPO)/ext

# Source files of this project
CSRCS += $(CODE_BIKE_MAIN)/fonts/lv_font_notosans_j_16.c
CSRCS += $(CODE_BIKE_MAIN)/fonts/lv_font_notosans_j_16_no_jp.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/battery.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/bluetooth.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/gps.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/heading.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/heart.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/icecream.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/lte.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/mic.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/phone.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/rain.c
CSRCS += $(CODE_BIKE_MAIN)/icons/1bit/sun.c
CSRCS += $(CODE_BIKE_MAIN)/icons/color/heading.c
CXXSRCS += $(CODE_BIKE_MAIN)/antiTheft.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/app_manager.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/audio_pcm_capture.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/audio_ogg.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/audio_module.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/camera_sensor.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/cloud_module.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/cycle_sensor.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/EventManager.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/GlobalStatus_time.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/GlobalStatus.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/gnss_module_fence.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/gnss_module.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/imu.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/lte_module.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/main_app.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/mqtt_module.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/my_log.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/sd_utils.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_appManager.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_audio.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_camera.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_cloud.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_cycle.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_globalStatus.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_gnss.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_imu.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_lte.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_mqtt.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_uibutton.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands_uitft.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/shell_commands.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/ui_button.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/ui_tft_imageConversion.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/ui_tft.cpp
CXXSRCS += $(CODE_BIKE_MAIN)/utils.cpp

# The Arduino Spresense Core
MAINSRC += $(CORE)/main.cpp
CXXSRCS += $(CORE)/HardwareSerial.cpp
CXXSRCS += $(CORE)/IPAddress.cpp
CXXSRCS += $(CORE)/Print.cpp
CXXSRCS += $(CORE)/Stream.cpp
CXXSRCS += $(CORE)/WInterrupt.cpp
CXXSRCS += $(CORE)/WMath.cpp
CXXSRCS += $(CORE)/WString.cpp
CSRCS += $(CORE)/hooks.c
CSRCS += $(CORE)/multi_print.c
CSRCS += $(CORE)/time.c
CSRCS += $(CORE)/timer_utility.c
CSRCS += $(CORE)/wiring_analog.c
CSRCS += $(CORE)/wiring_digital.c
CSRCS += $(CORE)/wiring_pulse.c
CSRCS += $(CORE)/wiring_shift.c
CSRCS += $(CORE)/wiring_tone.c

# The Arduino Spresense Libraries
CXXSRCS += $(LIBRARIES_SPRESENSE)/Camera/Camera.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/EEPROM/src/EEPROM.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/File/src/File.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/GNSS/GNSS.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LowPower/src/LowPower.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTEAccessProvider.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTEClient.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTECore.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTEModemVerification.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTEScanNetworks.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTETLSClient.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/LTEUDP.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/LTE/src/TLSClient.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/MP/src/MP.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/RTC/src/RTC.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/RTC/src/RtcTime.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/SDHCI/src/SDHCI.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/SPI/SPI.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/Storage/src/Storage.cpp
CXXSRCS += $(LIBRARIES_SPRESENSE)/Wire/Wire.cpp

# 3rd Party Arduino Libraries
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_BMP085_Unified/Adafruit_BMP085_U.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_L3GD20_U/Adafruit_L3GD20_U.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_LSM303DLHC/Adafruit_LSM303_U.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_Sensor_Calibration/Adafruit_Sensor_Calibration_EEPROM.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_Sensor_Calibration/Adafruit_Sensor_Calibration_SDFat.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_Sensor_Calibration/Adafruit_Sensor_Calibration.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Adafruit_Sensor/Adafruit_Sensor.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLEAdvertisingData.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLECharacteristic.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLEDescriptor.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLEDevice.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLEService.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLEStringCharacteristic.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/BLETypedCharacteristics.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/local/BLELocalAttribute.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/local/BLELocalCharacteristic.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/local/BLELocalDescriptor.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/local/BLELocalDevice.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/local/BLELocalService.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/remote/BLERemoteAttribute.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/remote/BLERemoteCharacteristic.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/remote/BLERemoteDescriptor.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/remote/BLERemoteDevice.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/remote/BLERemoteService.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/ATT.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/bitDescriptions.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/BLEUuid.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/btct.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/GAP.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/GATT.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/HCI.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/HCICordioTransport.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/HCIUartTransport.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/HCIVirtualTransport.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/keyDistribution.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/ArduinoBLE/src/utility/L2CAPSignaling.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/Arduino-Log/ArduinoLog.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/SimpleSerialShell/src/SimpleSerialShell.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/TFT_eSPI/TFT_eSPI.cpp
CXXSRCS += $(LIBRARIES_ARDUINO)/QZQSM/src/QZQSM.cpp
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_utils.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_txt.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_anim_timeline.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_style.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_style_gen.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_mem.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_fs.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_txt_ap.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_color.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_ll.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_log.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_area.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_lru.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_timer.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_async.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_templ.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_printf.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_tlsf.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_math.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_anim.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_gc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/misc/lv_bidi.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_indev.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_theme.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_disp.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_class.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_refr.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_event.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_draw.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_style_gen.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_style.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_group.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_indev_scroll.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_pos.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_tree.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/core/lv_obj_scroll.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/hal/lv_hal_tick.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/hal/lv_hal_indev.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/hal/lv_hal_disp.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/snapshot/lv_snapshot.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/ime/lv_ime_pinyin.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/msg/lv_msg.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/fragment/lv_fragment.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/fragment/lv_fragment_manager.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/gridnav/lv_gridnav.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/monkey/lv_monkey.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/others/imgfont/lv_imgfont.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/ffmpeg/lv_ffmpeg.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/sjpg/tjpgd.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/sjpg/lv_sjpg.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/freetype/lv_freetype.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/png/lv_png.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/png/lodepng.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/qrcode/lv_qrcode.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/qrcode/qrcodegen.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/rlottie/lv_rlottie.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/fsdrv/lv_fs_stdio.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/fsdrv/lv_fs_posix.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/fsdrv/lv_fs_fatfs.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/fsdrv/lv_fs_win32.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/bmp/lv_bmp.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/gif/lv_gif.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/libs/gif/gifdec.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/layouts/flex/lv_flex.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/layouts/grid/lv_grid.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/lv_extra.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/themes/basic/lv_theme_basic.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/themes/default/lv_theme_default.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/themes/mono/lv_theme_mono.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/calendar/lv_calendar.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/led/lv_led.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/win/lv_win.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/chart/lv_chart.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/spinbox/lv_spinbox.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/spinner/lv_spinner.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/tabview/lv_tabview.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/msgbox/lv_msgbox.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/span/lv_span.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/list/lv_list.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/menu/lv_menu.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/animimg/lv_animimg.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/meter/lv_meter.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/tileview/lv_tileview.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/extra/widgets/keyboard/lv_keyboard.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_28_compressed.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_10.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_14.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_unscii_8.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_40.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_44.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_loader.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_36.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_38.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_32.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_28.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_22.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_26.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_18.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_12.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_fmt_txt.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_16.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_48.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_42.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_46.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_unscii_16.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_34.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_8.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_30.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_20.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_simsun_16_cjk.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_24.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/font/lv_font_montserrat_12_subpx.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_line.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_layer.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_blend.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_img.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_arc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_gradient.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_rect.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_transform.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_letter.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_polygon.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_line.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sw/lv_draw_sw_dither.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_triangle.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_img_decoder.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/pxp/lv_gpu_nxp_pxp_osa.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/pxp/lv_draw_pxp_blend.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/pxp/lv_draw_pxp.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/pxp/lv_gpu_nxp_pxp.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_draw_vglite.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_vglite_buf.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/nxp/vglite/lv_vglite_utils.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/swm341_dma2d/lv_gpu_swm341_dma2d.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_arc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_rect.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_img.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/arm2d/lv_gpu_arm2d.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_layer.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_transform.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_bg.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_rect.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_img.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_utils.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_label.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_polygon.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_arc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_composite.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_line.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_layer.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_mask.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/sdl/lv_draw_sdl.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_img_cache.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_mask.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw_label.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_draw.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/draw/lv_img_buf.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_label.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_roller.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_btn.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_textarea.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_btnmatrix.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_img.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_slider.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_dropdown.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_bar.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_table.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_arc.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_line.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_checkbox.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_switch.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_canvas.c
CSRCS += $(LIBRARIES_ARDUINO)/lvgl/src/widgets/lv_objx_templ.c

FPP = /Users/jens/work/mcu_prj/prj/spresense/references/spresense-sdk/spresense
# Include directories
# The Arduino Spresense Core
_INCLUDES += -I$(FPP)/nuttx/include/libcxx 
_INCLUDES += -I$(FPP)/nuttx/include 
_INCLUDES += -I$(FPP)/nuttx/arch/arm/src/cxd56xx
_INCLUDES += -I$(FPP)/nuttx/arch/arm/src
_INCLUDES += -I$(FPP)/sdk/include 
_INCLUDES += -I$(FPP)/sdk/modules/include 
_INCLUDES += -I$(FPP)/sdk/apps/include 
_INCLUDES += -I$(FPP)/sdk/system/include 
_INCLUDES += -I$(FPP)/externals/include 
_INCLUDES += -I$(FPP)/externals/include/cmsis 
_INCLUDES += -I$(SPRESENSE_ARDUINO_CORE)/hardware/spresense/1.0.0/variants/spresense 
_INCLUDES += -I$(SPRESENSE_ARDUINO_CORE)/hardware/spresense/1.0.0/cores/spresense
# The Arduino Spresense Libraries
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/Audio
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/Camera
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/EEPROM/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/File/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/GNSS
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/LTE/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/LowPower/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/MP/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/RTC/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/SDHCI/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/SPI
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/Storage/src
_INCLUDES += -I$(LIBRARIES_SPRESENSE)/Wire
_INCLUDES += -I$(CODE_BIKE_REPO)/ext/opus/out/include
_INCLUDES += -I$(CODE_BIKE_REPO)/ext/libogg/out/include
_INCLUDES += -I$(CODE_BIKE_REPO)/ext/liboggz/out/include
# 3rd Party Arduino Libraries
_INCLUDES += -I$(LIBRARIES_ARDUINO)
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Adafruit_BMP085_Unified
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Adafruit_L3GD20_U
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Adafruit_LSM303DLHC
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Adafruit_Sensor_Calibration
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Adafruit_Sensor
_INCLUDES += -I$(LIBRARIES_ARDUINO)/ArduinoBLE/src
_INCLUDES += -I$(LIBRARIES_ARDUINO)/ArduinoJson/src
_INCLUDES += -I$(LIBRARIES_ARDUINO)/Arduino-Log
_INCLUDES += -I$(LIBRARIES_ARDUINO)/SimpleSerialShell/src
_INCLUDES += -I$(LIBRARIES_ARDUINO)/TFT_eSPI
_INCLUDES += -I$(LIBRARIES_ARDUINO)/base64_arduino/src
_INCLUDES += -I$(LIBRARIES_ARDUINO)/lvgl/src
_INCLUDES += -I$(LIBRARIES_ARDUINO)/QZQSM/src

#compiler flags taken over from the Arduino build
COMMON_FLAGS += -DARDUINO_BOARD=\"spresense_ast\"
COMMON_FLAGS += -DARDUINO=10816
COMMON_FLAGS += -DARDUINO_ARCH_SPRESENSE
COMMON_FLAGS += -DARDUINO_spresense_ast
COMMON_FLAGS += -DCONFIG_HAVE_DOUBLE
COMMON_FLAGS += -DCONFIG_WCHAR_BUILTIN
COMMON_FLAGS += -DF_CPU=156000000L

#  !!! gnu++11

# set the compiler flags
CFLAGS = $(_INCLUDES) $(COMMON_FLAGS)
CXXFLAGS = $(_INCLUDES) $(COMMON_FLAGS)

# I named my files cpp. Arduino names these cpp too, so I need to align here.
CXXEXT = .cpp

# Taken over from the Arduino build
LDFLAGS += -nodefaultlibs
LDFLAGS += -nostartfiles
LDFLAGS += -gc-sections
LDFLAGS += --defsym=__reserved_ramsize=0x20000

# We need that for the audio sub-system
CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

# This does not work!! So I needed to hack this into the file spresense-nuttx/arch/arm/src/Makefile
EXTRA_LIBS += -lopus
EXTRA_LIBS += -loggz
EXTRA_LIBS += -logg
EXTRA_LIBPATHS += -L$(CODE_BIKE_REPO)/ext/opus/out/lib


include $(SPRESENSE_HOME)/.vscode/application.mk
