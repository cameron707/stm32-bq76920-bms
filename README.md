# STM32 BQ76920 Battery Management System

## Overview

A Battery Management System (BMS) for 5-cell Li-ion and LiFePo battery packs using the STM32F103 microcontroller and Texas Instruments BQ76920 battery monitor IC.

## Documentation

This project follows a V-Model development process with complete traceability from requirements to tests.

| Document | Description | Standard |
|----------|-------------|----------|
| [Test Plan](docs/TEST_PLAN.md) | **Start here** - Overall test strategy and schedule | ISO/IEC/IEEE 29119-3 |
| [Requirements (SRS)](docs/SRS.md) | Functional, safety, and interface requirements | ISO/IEC/IEEE 29148 |
| [Design (SDD)](docs/SDD.md) | Software architecture and component design | IEEE 1016 |
| [Hazard Analysis (HARA)](docs/HARA.md) | Safety risk assessment | ISO 26262 |
| [Traceability Matrix](docs/TRACE_MATRIX.md) | Requirements → Tests traceability | ASPICE / ISO 26262 |
| [Coding Style](docs/CODING_STYLE.md) | Coding standards | MISRA C:2012, CERT C |
| [Process Description](docs/PROCESS.md) | V-Model development process | ISO 15288 |

## Standards Compliance

| Standard | Scope | Evidence |
|----------|-------|----------|
| **ISO 26262** | Functional safety (ASIL B) | HARA document, safety traceability |
| **MISRA C:2012** | Embedded C coding safety | Coding style guide, Cppcheck analysis |
| **CERT C** | Secure C coding | Coding style guide, Cppcheck analysis |
| **ISO/IEC/IEEE 29148** | Requirements engineering | SRS document |
| **ISO/IEC/IEEE 29119-3** | Software testing | Test plan, test specifications |
| **ASPICE** | Automotive software process | Traceability matrix, V-Model process |

## Current Features

- Li-ion only for initial proof of concept
- I2C communication between STM32F103 and BQ76920
- Read individual cell voltages (±5mV accuracy).
- Passive cell balancing (~17mA using BQ76920EVM default configuration)
- Overvoltage/undervoltage protection (hardware-based via BQ76920)

## Hardware

| Component | Model | Description |
|-----------|-------|-------------|
| **MCU** | STM32F103C8T6 | ARM Cortex-M3 "Blue Pill" |
| **Battery Monitor** | Texas Instruments BQ76920 | AFE + protection FETs |
| **Communication** | I2C | PB6 (SCL), PB7 (SDA) |
| **Debugger** | ST-LINK V2 | SWD programming/debugging |

## Pin Connections

| BQ76920 Pin | STM32 Pin | Description |
|-------------|-----------|-------------|
| SDA | PB7 | I2C Data (with 4.7kΩ pull-up) |
| SCL | PB6 | I2C Clock (with 4.7kΩ pull-up) |
| ALERT | PA0 | Interrupt output (faults, measurements ready) - with 500kΩ-1MΩ pull-down |
| REGOUT | 3.3V | Regulator Output |

## Getting Started

1. Review the [Test Plan](docs/TEST_PLAN.md) for overall strategy
2. Build the project using CMake and ARM GCC
3. Flash using ST-LINK V2 (see README for clone recovery notes)
4. Run tests in order: Unit → Integration → System

## Planned Features

- Extend safety checks to allow for LiFePo
- Coulomb counting for current monitoring
- ESP8266 WiFi module for cloud telemetry
- MQTT publishing to Alibaba Cloud
- FreeRTOS task management (optional)
- Altium PCB design (custom board) with active cell balancing

## Author

**Cameron Burnett**

[![GitHub](https://img.shields.io/badge/GitHub-cameron707-181717?style=flat&logo=github)](https://github.com/cameron707)

## License

MIT