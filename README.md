# STM32 BQ76920 Battery Management System

## Overview

A Battery Management System (BMS) for 5-cell Li-ion battery packs using the STM32F103 microcontroller and Texas Instruments BQ76920 battery monitor IC.

## Current Features

- 

## Hardware

- **MCU:** STM32F103C8T6 (ARM Cortex-M3)
- **Battery Monitor:** Texas Instruments BQ76920
- **Communication:** I2C (PB6 - SCL, PB7 - SDA)
- **Debugger:** ST-LINK V2

## Pin Connections

| BQ76920 Pin | STM32 Pin | Description |
|-------------|-----------|-------------|
| SDA | PB7 | I2C Data |
| SCL | PB6 | I2C Clock |
| ALERT | PA0 | Alert Interrupt |
| REGOUT | 3.3V | Regulator Output |

## Getting Started

Please refer to additional detailed documents for running this project yourself.


## Repository Structure
├── Core/ # Main source code
│ ├── Src/ # C source files
│ └── Inc/ # Header files
├── Drivers/ # STM32 HAL drivers
└── bq76920.ioc # STM32CubeMX configuration


## Core Features (Already Planned):

· 3S Li-ion battery monitoring using BQ76920 AFE
· I2C communication between STM32F103 and BQ76920
· Read individual cell voltages (±5mV accuracy)
· Passive cell balancing (50mA via internal FETs)
· Overvoltage/undervoltage protection (hardware-based via BQ76920)
· Coulomb counting for current monitoring
· ESP8266 WiFi module for cloud telemetry
· MQTT publishing to Alibaba Cloud
· Real-time battery status dashboard

## Hardware:

· STM32F103C8T6 "Blue Pill" main MCU
· BQ76920EVM evaluation board (AFE + protection FETs)
· 3S 18650 Li-ion battery pack (12.6V nominal)
· ESP8266-01S for WiFi connectivity
· TP4056-based 12.6V balance charger

## Technical Highlights:

· DMA for efficient ADC sampling (future expansion)
· FreeRTOS task management (optional, version 2)
· SWD debugging via ST-LINK V2
· Altium PCB design (version 2 - custom board)

## Author

Cameron Burnett - [GitHub](https://github.com/cameron707)

## License

MIT