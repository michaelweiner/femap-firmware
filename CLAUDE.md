# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Bare-metal embedded firmware for the **Fernsprechmobilapparat (FeMAp)** — a rotary telephone connected to a mobile phone via Bluetooth. Targets STM32L476RETx (Cortex-M4).

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

STM32CubeMX generated code is protected by `/* USER CODE BEGIN */` / `/* USER CODE END */` markers. Do not modify code outside those markers — it will be overwritten by CubeMX. Own source files live in `Core/Src/` and `Core/Inc/`:

- `main.c` / `main.h` — CubeMX scaffold + user init calls
- `phone_fsm.c/h` — phone state machine
- `bt_hfp.c/h` — Bluetooth HFP AT command handling
- `rotary.c/h` — rotary dial reading
- `uart_debug.c/h` — debug UART TX FIFO + stdio hook
- `ringer.c/h` — electromechanical bell
- `tone.c/h` — dial/busy tone synthesis

## Architecture

The firmware is a state machine (`phone_state_t`: IDLE → DIALTONE → ACTIVE_CALL / INCOMING_CALL / ERROR) driven by hardware events.

**`Core/Src/main.c`** — CubeMX-generated HAL init. Calls `bt_hfp_init()`, `uart_debug_init()`, `phone_fsm_init()`, and `init_rotary()` in the user init sections, then enters the `phone_fsm_process()` loop. Power management (STOP0 sleep) is handled inside `phone_fsm_process()`.

**`Core/Src/phone_fsm.c`** — Phone state machine. `phone_fsm_process()` is called in the main loop; reads hook switch (`GU`/PC3), delegates to `bt_hfp_process()`, and drives state transitions between IDLE, DIALTONE, ACTIVE_CALL, INCOMING_CALL, and ERROR. Also manages STOP0 sleep entry when idle and all UART paths are quiet.

**`Core/Src/bt_hfp.c`** — Bluetooth HFP module over LPUART1. Installs custom ISRs for AT command RX (line-buffered, `+HFPSTAT=`/`+HFPAUDIO=` parsing) and audio relay RX. Exposes `bt_hfp_get_stat()`, `bt_hfp_get_audio()`, `bt_hfp_audio_changed()`, and `bt_hfp_uart_done()`. Voice is routed through `VOICE_EN`/PB2.

**`Core/Src/rotary.c`** — Rotary dial. `init_rotary()` takes TIM15. `read_rotary()` counts NSI (PC7) pulse edges; a pause > 3000 timer ticks terminates the digit sequence. `count_to_ascii()` converts raw pulse counts to digit characters (digit 10 → `'0'`).

**`Core/Src/uart_debug.c`** — Debug output and BT relay TX. Implements a ring-buffer TX FIFO for LPUART1 (debug) and USART2 (BT relay). Hooks newlib's `__io_putchar` so `printf`/`puts`/`iprintf` output goes to the debug UART. `uart_debug_relay_to_bt()` forwards bytes from the relay RX ISR to the BT module.

**`Core/Src/ringer.c`** — Electromechanical bell control. Drives an H-bridge via TIM1/TIM2/TIM8 PWM with DMA-fed 25 Hz sine wave sample tables (`pwm_25hz_pos[]`, `pwm_25hz_neg[]`, 320 samples each). High-voltage rail enabled via `HV_EN`/PB13.

**`Core/Src/tone.c`** — Dial/busy tone synthesis via DAC1. TIM4 clocks DAC samples; TIM3 ARR selects tone type (ARR=1 for dial tone at 425 Hz, ARR=479 for congestion/busy tone). 4096-sample sine table fed through OPAMP2 to the handset.

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
| USART2 | Serial relay to BT module as well as debug outputs |
| LPUART1 | BT AT command interface |
| OPAMP2 | Analog signal conditioning for tone output |
