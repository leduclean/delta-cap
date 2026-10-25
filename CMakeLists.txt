cmake_minimum_required(VERSION 3.22)
include(ExternalProject)

# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Set the project name
set(CMAKE_PROJECT_NAME Delta)

# Core project settings
project(${CMAKE_PROJECT_NAME})

# Include external project file
include("mx-generated.cmake")

file(GLOB ST_TOOL_DIRS $ENV{HOME}/st/*/plugins/*/tools/bin)
find_program(ARM_GDB arm-none-eabi-gdb HINTS ${ST_TOOL_DIRS})
find_program(STLINK_GDBSERVER ST-LINK_gdbserver HINTS ${ST_TOOL_DIRS})
find_program(STM32_PROGRAMMER STM32_Programmer_CLI HINTS ${ST_TOOL_DIRS})
cmake_path(GET STM32_PROGRAMMER PARENT_PATH STM32_PROGRAMMER_DIR)

set(FSBL_ELF ${CMAKE_SOURCE_DIR}/FSBL/build/Delta_FSBL.elf)
set(GDB_PORT 61234)
set(GDB_LOAD
    -ex
    "target extended-remote :${GDB_PORT}"
    -ex
    "monitor reset"
    -ex
    "load"
)

add_custom_target(
    info
    COMMAND
        ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/info.cmake
    COMMENT "Available targets"
)

add_custom_target(
    gdbserver
    COMMAND
        ${STLINK_GDBSERVER} --port-number ${GDB_PORT} --log-level 1 --swd
        # access port 1: the Cortex-M55 (AP0 is the system AP)
        --apid 1
        # connect to the device under reset
        --initialize-reset
        # keep running after gdb disconnects
        --persistent
        # libs to drive the ST-Link
        --stm32cubeprogrammer-path ${STM32_PROGRAMMER_DIR}
    USES_TERMINAL
)

add_custom_target(
    load
    COMMAND
        ${ARM_GDB} ${FSBL_ELF} --quiet --batch ${GDB_LOAD} --ex
        "detach" # Leave the console but let the program run
    DEPENDS
        Delta_FSBL
    USES_TERMINAL
)

add_custom_target(
    debug
    COMMAND
        ${ARM_GDB} ${FSBL_ELF} ${GDB_LOAD} -ex "break main" -ex "continue"
)

add_custom_target(
    flash-model
    COMMAND
        ${STM32_PROGRAMMER} --connect port=SWD mode=HOTPLUG ap=1 --extload
        ${STM32_PROGRAMMER_DIR}/ExternalLoader/MX25UM51245G_STM32N6570-NUCLEO.stldr
        --write ${CMAKE_SOURCE_DIR}/Model/network_rel.hex --verify
        -hardRst # no long form
    USES_TERMINAL
)
