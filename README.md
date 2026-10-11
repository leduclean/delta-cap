# Delta

## Where you code

**`FSBL/User/` only.**

| File        | Does                                              |
|-------------|---------------------------------------------------|
| `app.c`     | Entry point `app_start()`, creates the tasks. Start here. |
| `camera.c`  | Sensor and pipeline     |
| `display.c` | Frame buffers streamed over USB.                  |
| `console.c` | UART line input, `printf` output.                 |

## Layout

```
Delta.ioc      CubeMX config: pins, clocks, peripherals. Edit with CubeMX → Generate.
FSBL/Core/     generated: main.c, peripheral init, IRQs.
FSBL/User/     our code.
FSBL/USBX/     generated USB glue.
Lib/           ST libs: camera, isp, screenl (USB video), lcd.
Drivers/       ST Hardware Abstraction Layer + CMSIS. Never edit.
Middlewares/   FreeRTOS, USBX. Never edit.
```

FSBL = First Stage Boot Loader. For us it's just "the app".

## Boot flow

`main.c` (HAL, clocks, UART) → FreeRTOS → `app_freertos.c` → `app_start()` → tasks `cli` (console) and `camera` (frames).

## Build

Needs `arm-none-eabi-gcc`, `cmake`, `make`.

```sh
cmake --preset Debug
cmake --build --preset Debug    # → FSBL/build/Delta_FSBL.elf
```

`FSBL/User/` builds with `-Werror`.

Format before committing:

```sh
clang-format -i FSBL/User/Src/*.c FSBL/User/Inc/*.h
```

PS(dev note): I could add a small git hook if you want.

## Run

- Flash: Use STM32CubeIDE for now.
- Console: `picocom -b 115200 /dev/ttyACM0`
- Video: plug the board's user USB port, then `vlc v4l2:///dev/video0`
