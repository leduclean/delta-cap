cmake_minimum_required(VERSION 3.22)
# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default build type is Debug.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(
    "Build type: "
    ${CMAKE_BUILD_TYPE}
)

# Compiler and linked options
set(STM32_MCU_FLAGS "-mcpu=cortex-m55 -mfpu=fpv5-d16 -mfloat-abi=hard -mcmse ")
add_compile_definitions(USE_FULL_ASSERT)
set(STM32_LINKER_SCRIPT STM32N657XX_AXISRAM2_fsbl.ld)

# Must stay after the STM32_* variables: project() loads the toolchain file,
# which reads them to build the compiler and linker flags.
set(CMAKE_PROJECT_NAME Delta_FSBL)
project(
    ${CMAKE_PROJECT_NAME}
    LANGUAGES
        C
        ASM
)

add_executable(${CMAKE_PROJECT_NAME})

include("mx-generated.cmake")

# Lib first: User sets properties on its targets
add_subdirectory(../Lib ${CMAKE_BINARY_DIR}/Lib)

# Our code: a separate lib so the strict flags don't hit CubeMX and Lib sources
file(GLOB_RECURSE USER_SRC CONFIGURE_DEPENDS User/Src/*.c)
add_library(user STATIC ${USER_SRC})
target_include_directories(user PUBLIC User/Inc)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/model.cmake)
target_compile_definitions(user PRIVATE AI_MODEL_ADDR=${AI_MODEL_ADDR})
target_link_libraries(
    user
    PUBLIC
        camera
        lcd
        screenl
        stm32cubemx
        ai-pp
        stepper
)

# Linux like flags
set_target_properties(
    user
    PROPERTIES
        C_STANDARD
            23
)
target_compile_options(
    user
    PRIVATE
        -Wall
        -Wextra
        -Wundef
        -Wvla
        -Wshadow
        -Wdouble-promotion
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wformat=2
        -Wcast-align
        -Wnull-dereference
        -fno-strict-aliasing
        -Werror
)

# External lib are not compiled with our flags
set_target_properties(
    stm32cubemx
    camera
    lcd
    screenl
    ai
    ai-pp
    stepper
    PROPERTIES
        SYSTEM
            TRUE
)

target_link_libraries(${CMAKE_PROJECT_NAME} user)
