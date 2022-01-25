cmake_minimum_required(VERSION 3.20.0)

# set board variable to thingy:52
set(BOARD "thingy52_nrf52832")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ble_sensor)

target_include_directories(app PRIVATE inc)

target_sources(app PRIVATE src/sensors.c)
target_sources(app PRIVATE src/bt_sensor_service.c)
target_sources(app PRIVATE src/rtc.c)
target_sources(app PRIVATE src/main.c)