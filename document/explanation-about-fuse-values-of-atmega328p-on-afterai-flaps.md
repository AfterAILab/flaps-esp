---
title: Explanation about Fuse Values of ATmega328P on AfterAI Flaps
createdAt : 2024-08-29T19:41:15+09:00
updatedAt: 2024-08-29T19:43:11+09:00
---
# Explanation about Fuse Values of ATmega328P on AfterAI Flaps

Refer to 27.2 Fuse Bits of [the datasheet of ATmega328P](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf).
## Low Fuse Settings
Clock source: Low Power Crystal Oscillator (Y1: 16kHz crystal)
Start-up time: Slowly rising power
Clock output: No
Divide clock by 8: No

| Low Fuse Byte | Bit No. | Description          | Value |
| ------------- | ------- | -------------------- | ----- |
| CKDIV8        | 7       | Divide clock by 8    | 1     |
| SKOUT         | 6       | Clock output         | 1     |
| SUT1          | 5       | Select start-up time | 1     |
| SUT0          | 4       | Select start-up time | 1     |
| CKSEL3        | 3       | Select clock source  | 1     |
| CKSEL2        | 2       | Select clock source  | 1     |
| CKSEL1        | 1       | Select clock source  | 1     |
| CKSEL0        | 0       | Select clock source  | 1     |

## High Fuse Settings

| High Fuse Byte | Bit No. | Description                                       | Value |
| -------------- | ------- | ------------------------------------------------- | ----- |
| RSTDISBL       | 7       | External reset disable                            | 1     |
| DWEN           | 6       | debugWIRE enable                                  | 1     |
| SPIEN          | 5       | Enable serial program and data downloading        | 0     |
| WDTON          | 4       | Watchdog timer always On                          | 1     |
| EESAVE         | 3       | EEPROM memory is preserved through the chip erase | 1     |
| BOOTSZ1        | 2       | Select boot size                                  | 1     |
| BOOTSZ0        | 1       | Select boot size                                  | 1     |
| BOOTRST        | 0       | Select reset vector                               | 0     |
## Extended Fuse Settings

| Extended Fuse Byte | Bit No. | Description             | Value |
| ------------------ | ------- | ----------------------- | ----- |
| -                  | 7       | -                       | 1     |
| -                  | 6       | -                       | 1     |
| -                  | 5       | -                       | 1     |
| -                  | 4       | -                       | 1     |
| -                  | 3       | -                       | 1     |
| -                  | 2       | -                       | 1     |
| -                  | 1       | -                       | 1     |
| SELFPRGEN          | 0       | Self programming enable | 1     |
