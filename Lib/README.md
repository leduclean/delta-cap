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
