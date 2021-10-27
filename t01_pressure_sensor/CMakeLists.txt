

cmake_minimum_required(VERSION 3.20.0)

# set board variable to thingy:52
set(BOARD "thingy52_nrf52832")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(my_fancy_demo)

target_sources(app PRIVATE src/main.c)
