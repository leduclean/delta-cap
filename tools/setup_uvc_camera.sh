#!/bin/bash
function make_comment() {
    srcs=$1
    to_disable=$2
    for fsrc in $srcs; do
        echo "comment file $fsrc"
        for text in $to_disable; do
            echo "text $text"
            sedarg="s/^[[:space:]]*${text}/\/\/${text}\/\/_DISABLED/g"
            echo "$sedarg"
            sed -e "$sedarg" -i $fsrc
        done
    done
}
function make_disable() {
    srcs=$1
    to_disable=$2
    for fsrc in $srcs; do
        echo "disable file $fsrc"
        for text in $to_disable; do
            echo "text $text"
            sedarg="s/[[:space:]]${text}(/ ${text}_DISABLED(/g"
            echo "$sedarg"
            sed -e "$sedarg" -i $fsrc
        done
    done
}

srcs="./Appli/Core/Src/stm32n6xx_it.c
./FSBL/Core/Src/stm32n6xx_it.c"
to_disable="USB1_OTG_HS_IRQHandler DMA2D_IRQHandler"
make_disable "$srcs" "$to_disable"
srcs="./FSBL/Core/Src/usb_otg.c"
to_disable="HAL_PCD_MspInit"
make_disable "$srcs" "$to_disable"
srcs="./Appli/USBX/App/app_usbx.c
./FSBL/USBX/App/app_usbx.c
"
to_disable="_ux_utility_interrupt_disable _ux_utility_interrupt_restore"
make_disable "$srcs" "$to_disable"

to_comment="MX_USB1_OTG_HS_PCD_Init
  MX_USBX_Init
  MX_DMA2D_Init
"
make_comment "./FSBL/Core/Src/main.c" "$to_comment"

rm ./Appli/USBX/App/ux_user.h ./FSBL/USBX/App/ux_user.h
