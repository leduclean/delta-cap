# Lib

## The rule

**CubeMX ships the driver. The lib does the init.**

If a lib inits a peripheral, uncheck "Generate Code" in CubeMX
(*Project Manager → Advanced Settings*), **unless the lib doesn't provide `HAL_xxx_MspInit`**. Then keep "Generate Code" and only check
"Do Not Generate Function Call".

| Peripheral | Init by               | Generate Code   |
|------------|-----------------------|-----------------|
| DCMIPP     | `stm32-mw-camera`     | no              |
| I2C2       | BSP `nucleo_bus.c`    | no              |
| DMA2D      | `screenl`             | no              |
| USB        | `uvcl` (CubeMX MSP)   | yes, no call    |

## AI (NPU)

Copied unmodified from [STM32N6-GettingStarted-ObjectDetection](https://github.com/STMicroelectronics/STM32N6-GettingStarted-ObjectDetection)
@ `4240acbeb7febd335cf128ebe56f26790cabbdd3`. Runtime and `Model/` must come from the
same commit, otherwise: "Possible mismatch in ll_aton library used".

| Here                                   | From (ST repo)                                       |
|----------------------------------------|------------------------------------------------------|
| `Lib/stedgeai-lib/{Inc,Npu}`           | `Middlewares/stedgeai-lib` (prebuilt `Lib/` skipped) |
| `Lib/ai-pp-wrapper`                    | `Middlewares/ai-postprocessing-wrapper`              |
| `Lib/stm32-vision-models-pp`            | `Middlewares/stm32-vision-models-postprocessing/lib_vision_models_pp` |
| `Model/`                               | `Model/NUCLEO-N657X0-Q`                              |
| `Model/app_config.h`                   | `Application/NUCLEO-N657X0-Q/Inc` (post-process part only) |
| `Drivers/BSP/STM32N6xx_Nucleo/*xspi*`, `mx25um51245g_conf.h`, `Drivers/BSP/Components/mx25um51245g` | `STM32Cube_FW_N6` (BSP V1.2.0, same as ours) |
| `Drivers/CMSIS/DSP/Include`            | `STM32Cube_FW_N6` (headers only, for `float32_t`)    |
