# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Bare-metal embedded firmware for the **Fernsprechmobilapparat (FEMAP)** — a rotary telephone connected to a mobile phone via Bluetooth. Targets STM32L476RETx (Cortex-M4).

## Build Commands

```bash
# Configure + build Debug
cmake --preset Debug && cmake --build --preset Debug

# Configure + build Release
cmake --preset Release && cmake --build --preset Release

# Format code (runs clang-format on own sources)
cmake --build --preset format

# Flash to device
st-flash --connect-under-reset write build/Debug/femap_firmware.bin 0x08000000
```

Build output lands in `build/Debug/` or `build/Release/`. The ELF and `.bin` files are generated there, along with `compile_commands.json` for IDE/clangd integration.

No automated tests — testing is done via hardware with GDB (`.gdbinit` connects to `localhost:4242` for st-link).

## Code Style

LLVM-based clang-format with Allman braces and 4-space indentation (see `.clang-format`, `.editorconfig`). Run the `format` CMake target before committing.

STM32CubeMX generated code is protected by `/* USER CODE BEGIN */` / `/* USER CODE END */` markers. Do not modify code outside those markers — it will be overwritten by CubeMX. Own source files are `Core/Src/ringer.c`, `Core/Src/tone.c`, `Core/Src/main.c` (user sections), and their headers in `Core/Inc/`.

## Architecture

The firmware is a state machine (`phone_state_t`: IDLE → DIALTONE → ACTIVE_CALL / INCOMING_CALL / ERROR) driven by hardware events.

**`Core/Src/main.c`** — Application core: state machine, Bluetooth AT command handling over LPUART1, UART relay for audio (USART2), hook switch (`GU`/PC3) and rotary dial (`NSA`/PC6, `NSI`/PC7) GPIO reading, power management (STOP0 sleep when idle).

**`Core/Src/ringer.c`** — Electromechanical bell control. Drives an H-bridge via TIM1/TIM2/TIM8 PWM with DMA-fed 25 Hz sine wave sample tables (`pwm_25hz_pos[]`, `pwm_25hz_neg[]`, 320 samples each). High-voltage rail enabled via `HV_EN`/PB13.

**`Core/Src/tone.c`** — Dial/busy tone synthesis via DAC1. TIM4 clocks DAC samples; TIM3 ARR selects tone type (ARR=1 for dial tone at 425 Hz, ARR=479 for congestion/busy tone). 4096-sample sine table fed through OPAMP2 to the handset.

**Bluetooth flow** — The Bluetooth module is managed entirely via AT commands. `hfpstat_t` tracks HFP connection state; `hfpaudio_t` tracks SCO audio. Voice is routed through `VOICE_EN`/PB2. Incoming characters from the BT module trigger UART RX interrupts and are accumulated into a line buffer for parsing.

**Rotary dial** — `read_rotary()` counts NSI pulse edges; a pause longer than the inter-digit timeout terminates the digit. Digit 10 maps to `0`.

**Peripheral allocation summary:**
| Peripheral | Use |
|---|---|
| DAC1 | Tone output |
| TIM3 | DAC tone type selector |
| TIM4 | DAC sample clock |
| TIM1 | H-bridge NMOS PWM (DMA) |
| TIM2 | H-bridge duty cycle |
| TIM8 | H-bridge PMOS |
| TIM15 | Timing/debounce |
| I2C2 | MP2722 charger IC |
| USART2 | Serial relay to BT module (audio path) |
| LPUART1 | BT AT command interface |
| OPAMP2 | Analog signal conditioning for tone output |
