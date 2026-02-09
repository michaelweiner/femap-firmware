# femap-firmware
This is the firmware for the Fernsprechmobilapparat project presented at the 39th Chaos Communication Congress (39c3). Please see www.femap.net for further infornation.

## How to flash
`make; st-flash --connect-under-reset write build/femap_firmware.bin 0x08000000`
