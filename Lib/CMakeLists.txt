link_libraries(stm32cubemx)

file(
    GLOB_RECURSE CAMERA_SRC
    CONFIGURE_DEPENDS
    stm32-mw-camera/*.c
    isp/*.c
)
# The camera BSP bus owns I2C2 (sensor). CubeMX provides the HAL I2C driver
# (MX_I2C2_Init is static and not called), but doesn't build the BSP bus file,
# copied by hand from STM32Cube_FW_N6 V1.4.1.
list(
    APPEND CAMERA_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/BSP/STM32N6xx_Nucleo/stm32n6xx_nucleo_bus.c
)
add_library(camera STATIC ${CAMERA_SRC})
target_include_directories(
    camera
    PUBLIC
        stm32-mw-camera
        stm32-mw-camera/sensors
        stm32-mw-camera/sensors/imx335
        isp
)
target_compile_definitions(camera PUBLIC STM32N6570_NUCLEO_REV)
target_link_libraries(camera PUBLIC m)

file(
    GLOB LCD_SRC
    CONFIGURE_DEPENDS
    lcd/*.c
    fonts/*.c
)
add_library(lcd STATIC ${LCD_SRC})
target_include_directories(
    lcd
    PUBLIC
        lcd
        fonts
)
target_link_libraries(lcd PUBLIC stm32cubemx)

file(GLOB_RECURSE SCREENL_SRC CONFIGURE_DEPENDS screenl/*.c)

add_library(screenl STATIC ${SCREENL_SRC})
target_include_directories(
    screenl
    PUBLIC
        screenl/Inc
        screenl/uvcl/Inc
        screenl/uvcl/Src
        screenl/uvcl/Src/usbx
)
target_compile_definitions(
    screenl
    PUBLIC
        SCR_LIB_USE_UVCL
        SCR_LIB_USE_FREERTOS
        UVCL_USBX_USE_FREERTOS
        UVC_LIB_USE_USBX
        USBL_PACKET_PER_MICRO_FRAME=3
)
target_link_libraries(screenl PUBLIC lcd)

#TODO: Clean This lib declaration

# NPU runtime + compiled model.
# Source list taken from the ST GettingStarted-ObjectDetection Makefile.
set(LL_ATON stedgeai-lib/Npu/ll_aton)
add_library(
    ai
    STATIC
    EXCLUDE_FROM_ALL # built only once something links it
    # Low level driver
    ${LL_ATON}/ll_aton.c
    # NPU aton runtime
    ${LL_ATON}/ll_aton_runtime.c
    ${LL_ATON}/ll_aton_cipher.c
    ${LL_ATON}/ll_aton_util.c
    ${LL_ATON}/ll_aton_lib.c
    ${LL_ATON}/ll_aton_osal_freertos.c
    ${LL_ATON}/ll_aton_reloc_network.c
    # Software kernels
    ${LL_ATON}/ll_sw_float.c
    ${LL_ATON}/ll_sw_integer.c
    ${LL_ATON}/ll_aton_lib_sw_operators.c
    stedgeai-lib/Npu/Devices/STM32N6xx/npu_cache.c
    stedgeai-lib/Npu/Devices/STM32N6xx/mcu_cache.c
    # External NOR flash on xSPI2 (holds the weights), copied by hand like nucleo_bus.c
    ../Drivers/BSP/STM32N6xx_Nucleo/stm32n6xx_nucleo_xspi.c
    ../Drivers/BSP/Components/mx25um51245g/mx25um51245g.c
)
target_include_directories(
    ai
    PUBLIC
        stedgeai-lib/Inc
        ${LL_ATON}
        stedgeai-lib/Npu/Devices/STM32N6xx
        ../Model
        ../Drivers/BSP/Components/mx25um51245g
)
target_compile_definitions(
    ai
    PUBLIC
        LL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6
        # ai task blocks on a semaphore given by the NPU IRQ
        LL_ATON_OSAL=LL_ATON_OSAL_FREERTOS
        # a single network
        APP_HAS_PARALLEL_NETWORKS=0
        LL_ATON_RT_MODE=LL_ATON_RT_ASYNC
        LL_ATON_RT_RELOC
        LL_ATON_EB_DBG_INFO
        LL_ATON_SW_FALLBACK
)
target_link_libraries(ai PUBLIC m) # m: exp() in ll_aton_lib.c softmax

file(
    GLOB_RECURSE AI_PP_SRC
    CONFIGURE_DEPENDS
    stm32-vision-models-pp/Src/*.c
    ai-pp-wrapper/*.c
)
add_library(ai-pp STATIC EXCLUDE_FROM_ALL ${AI_PP_SRC})
target_include_directories(
    ai-pp
    PUBLIC
        stm32-vision-models-pp/Inc
        ai-pp-wrapper
        ../Drivers/CMSIS/DSP/Include # arm_math.h
)
target_link_libraries(
    ai-pp
    PUBLIC
        ai
        m
)
target_compile_options(ai-pp PRIVATE -O2)
target_compile_options(ai PRIVATE -O2)

file(GLOB STEPPER_SRC CONFIGURE_DEPENDS stepper-ihm02/*.c)
add_library(stepper STATIC ${STEPPER_SRC})
target_include_directories(stepper PUBLIC stepper-ihm02/)
