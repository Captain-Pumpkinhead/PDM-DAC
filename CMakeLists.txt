cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(dac_pdm_test)
pico_sdk_init()

add_executable(dac_pdm_test dac_pdm_test.cpp)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(dac_pdm_test pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(dac_pdm_test 1)
pico_enable_stdio_uart(dac_pdm_test 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(dac_pdm_test)

# add url via pico_set_program_url
#example_auto_set_url(dac_pdm_test)

