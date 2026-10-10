file(GLOB_RECURSE CAMERA_SRC CONFIGURE_DEPENDS
    stm32-mw-camera/*.c
    isp/*.c
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
    screenl/uvcl/Inc/usbx
    screenl/uvcl/Src
    screenl/uvcl/Src/usbx
)
target_compile_definitions(screenl PUBLIC
    SCR_LIB_USE_UVCL
    UVC_LIB_USE_USBX
    USBL_PACKET_PER_MICRO_FRAME=3
)
target_link_libraries(screenl PUBLIC lcd stm32cubemx)
