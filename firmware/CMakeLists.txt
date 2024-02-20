cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(cozy-extrude)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(lib/pwm_fan)
add_subdirectory(lib/bme280)

add_executable(${PROJECT_NAME}
        cozy-extrude.c
        cmds.c cmds.h
        heater.c heater.h
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

target_link_libraries(${PROJECT_NAME} pwm_fan bme280 pico_stdlib)
