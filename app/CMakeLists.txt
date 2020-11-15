# SPDX-License-Identifier: Apache-2.0

set(BOARD beirdo_battery_joule_thief_v1)
set(ZEPHYR_EXTRA_MODULES ${CMAKE_SOURCE_DIR}/lib/zephyr-adafruit-ssd1306)

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(beirdo-battery-joule-thief-v1-app)

include_directories(BEFORE SYSTEM include)

# Add your source file to the "app" target. This must come after
# find_package(Zephyr) which defines the target.

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/gpios.c)
target_sources(app PRIVATE src/adcs.c)
target_sources(app PRIVATE src/charge-counters.c)
target_sources(app PRIVATE src/input-batteries.c)
target_sources(app PRIVATE src/charger.c)
target_sources(app PRIVATE src/display.c)
