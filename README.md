### Project setup

This project is based on SW4STM32 which can be found
[Here](https://www.st.com/en/development-tools/sw4stm32.html)

It has been based on this IDE as it facilitate the compilation/integration to a
STM32 board. It is based on [openOCD](http://openocd.org/) for uploading the
binary to the board.

To import this project, clone the repository then:
`File > Import... > General > Existing Project into Workspace.`
It should point to the folder containing the `.cproject` and `.project`.

The project has been created as follow:
- File > Create new C Project 
- Select Ac6 MCU STM32 with Ac6 Compiler
- Select Serie STM32L4 and board B-L475SE-IOT01A1
- Use Hardware Abstraction Layer (HAL Firmware)
- Select third party FreeRTOS

Under `Project Settings > C++ Build > Settings > MCU Settings`, the following
parameters were set:
- Floating Point Hardware: fpv4-sp-d16
- Floating Point ABI: Hard
- Instruction Set: Thumb

### Serial through Putty

The UART configured within the STM32 for log output is based on:
- 115200 baud rate.
- 8 Data bit
- 1 Stop bit
- No parity bit
- No flow control

Configure the Terminal to have `Local Echo` and `Local Line Editing` options.
Configure the Terminal to have "implicit CR in every LF"

### Additional information

The storage uses [SPIFFS](https://github.com/pellepl/spiffs)for storing the
values. This Filesystem has bufferization disabled.

The socket uses the ES WIFI ism4336 device from the board.
See [documentation](https://www.inventeksys.com/ism4336-m3g-l44-e-embedded-serial-to-wifi-module/).

Configuration for FreeRTOS:
- Under `Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang`, heap4.c is
  the only source file not excluded from the build.
- The default value in FreeRTOS for the dynamic allocation has been increased.
  (Under `inc/FreeRTOSConfig.h`, variable `configTOTAL_HEAP_SIZE`)