# femap-firmware
This is the firmware for the Fernsprechmobilapparat project presented at the 39th Chaos Communication Congress (39c3). Please see www.femap.net for further information.

## Prerequisites

- CMake ≥ 3.22
- Ninja
- [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (arm-none-eabi-gcc, includes binutils)
- arm-none-eabi-newlib - lightweight C standard library for embedded targets
- [stlink](https://github.com/stlink-org/stlink) - provides st-flash for programming the device

On Arch Linux:
```
sudo pacman -S cmake ninja arm-none-eabi-gcc arm-none-eabi-newlib stlink
```

On Debian/Ubuntu:
```
sudo apt install cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi stlink-tools
```

## Build

```
cmake --preset <Debug|Release>
cmake --build --preset <Debug|Release>
```

Build output lands in `build/Debug/` or `build/Release/`.

```
cmake --build --preset format
```

## Flash

Connect the STM32 via ST-Link, then use the `flash` CMake target (builds first if needed):

```
cmake --preset <Debug|Release>
cmake --build --preset <Debug|Release> --target flash
```
