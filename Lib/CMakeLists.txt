file(GLOB_RECURSE CAMERA_SRC CONFIGURE_DEPENDS
    stm32-mw-camera/*.c
    isp/*.c
)
# The camera BSP bus owns I2C2 (sensor). CubeMX provides the HAL I2C driver
# (MX_I2C2_Init is static and not called), but doesn't build the BSP bus file,
# copied by hand from STM32Cube_FW_N6 V1.4.1.
list(APPEND CAMERA_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/BSP/STM32N6xx_Nucleo/stm32n6xx_nucleo_bus.c
)
add_library(camera STATIC ${CAMERA_SRC})
target_include_directories(camera PUBLIC
    stm32-mw-camera
    stm32-mw-camera/sensors
    stm32-mw-camera/sensors/imx335
    isp
)
target_compile_definitions(camera PUBLIC STM32N6570_NUCLEO_REV)
target_link_libraries(camera PUBLIC stm32cubemx m)


file(GLOB LCD_SRC CONFIGURE_DEPENDS lcd/*.c fonts/*.c)
add_library(lcd STATIC ${LCD_SRC})
target_include_directories(lcd PUBLIC lcd fonts)
target_link_libraries(lcd PUBLIC stm32cubemx)


file(GLOB_RECURSE SCREENL_SRC CONFIGURE_DEPENDS screenl/*.c)

add_library(screenl STATIC ${SCREENL_SRC})
target_include_directories(screenl PUBLIC
    screenl/Inc
    screenl/uvcl/Inc
    screenl/uvcl/Src
    screenl/uvcl/Src/usbx
)
target_compile_definitions(screenl PUBLIC
    SCR_LIB_USE_UVCL
    SCR_LIB_USE_FREERTOS
    UVCL_USBX_USE_FREERTOS
    UVC_LIB_USE_USBX
    USBL_PACKET_PER_MICRO_FRAME=3
)
target_link_libraries(screenl PUBLIC lcd stm32cubemx)
