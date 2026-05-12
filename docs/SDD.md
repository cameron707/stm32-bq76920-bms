# Software Design Description
## STM32 BQ76920 Battery Management System

**Version:** 1.0  
**Prepared by:** Cameron Burnett  
**Date:** 2026-05-05  
**Status:** Draft  

---

## 1. Introduction

### 1.1 Document Purpose
This Software Design Description (SDD) describes the architecture, component decomposition, and detailed design of the STM32 BQ76920 Battery Management System. It bridges the Software Requirements Specification (SRS) and the implementation.

### 1.2 Scope
This document covers:
- System architecture and component decomposition
- Component descriptions and interfaces
- Data flow and control flow
- Design decisions and rationale
- Traceability to requirements

### 1.3 Definitions, Acronyms, and Abbreviations

| Term | Definition |
|------|------------|
| AFE | Analog Front End (BQ76920) |
| BMS | Battery Management System |
| HAL | Hardware Abstraction Layer (STM32Cube) |
| I2C | Inter-Integrated Circuit (communication protocol) |
| SWD | Serial Wire Debug |

### 1.4 References

| Document | Location |
|----------|----------|
| Software Requirements Specification | `docs/SRS.md` |
| Hazard Analysis (HARA) | `docs/HARA.md` |
| Test Plan | `docs/TEST_PLAN.md` |
| BQ76920 Datasheet | Texas Instruments (external) |
| STM32F103 Reference Manual | STMicroelectronics (external) |

### 1.5 Document Overview
This document is organized into eight main sections: System Architecture and Design Constraints (Section 2), Component Design (Section 3), Interface Design (Section 4), Data Design (Section 5), Control Flow Design (Section 6), Design Decisions (Section 7), and Traceability (Section 8).

### 1.6 Context
The STM32 BQ76920 BMS is an embedded system designed to monitor and protect a 3-cell Li-ion battery pack. It operates as a standalone hardware module that interfaces with the battery pack, providing real-time voltage, current, and temperature data while executing safety-critical protection functions such as overvoltage and overtemperature cutoffs.

### 1.7 Summary
This SDD defines the architecture of the BMS software, including the decomposition of functionality into components (Application Logic, BQ76920 Driver, HAL), the data flow from hardware to application, and the control flow for protection logic. It documents key design decisions, rationale for alternatives, and traceability from requirements to design elements.

---

## 2. System Architecture and Design Constraints

### 2.1 System Context

| Component | Type | Description | External Interfaces |
|-----------|------|-------------|---------------------|
| STM32F103 | Microcontroller | Executes firmware; runs application logic, I2C driver, protection logic | I2C, SWD, GPIO (LED) |
| BQ76920 | Analog Front End | Measures cell voltages, pack current, temperature; controls protection FETs | I2C, analog inputs, FET gate drives |
| BatteryPack | 3S Li-ion | 3 cells in series; 11.1V nominal, 12.6V maximum | Analog outputs to BQ76920 |

### 2.2 Block Definition (Logical Architecture)

| Block | Stereotype | Contained Parts | External References |
|-------|------------|-----------------|---------------------|
| STM32F103 | `«device»` | ApplicationLogic, I2CDriver, ProtectionLogic | BQ76920 (via I2C) |
| BQ76920 | `«device»` | (none) | STM32F103 (I2C), BatteryPack (analog) |
| BatteryPack | `«block»` | (none) | BQ76920 (voltage/current/temp sensing) |

### 2.3 Interface Summary

| Interface | Source | Target | Protocol | Speed | Pins |
|-----------|--------|--------|----------|-------|------|
| I2C | STM32F103 | BQ76920 | I2C | 100kHz | PB6 (SCL), PB7 (SDA) |
| SWD | ST-LINK V2 | STM32F103 | SWD | 4MHz | PA13 (SWDIO), PA14 (SWCLK) |
| LED | STM32F103 | PC13 | GPIO | N/A | PC13 |

### 2.4 Design Constraints

| ID | Constraint | Source |
|----|------------|--------|
| DC-01 | I2C communication at 100kHz only | BQ76920 datasheet |
| DC-02 | No dynamic memory allocation (`malloc`) | MISRA C:2012 Rule 21.10 |
| DC-03 | Static analysis required (Cppcheck) | Project quality standard |
| DC-04 | STM32F103 runs at 72MHz max | Hardware limitation |

### 2.5 Assumptions

| ID | Assumption | Impact if False |
|----|------------|-----------------|
| AS-01 | BQ76920 is correctly wired to STM32 via I2C with 4.7kΩ pull-ups | I2C communication fails |
| AS-02 | Battery is 3S Li-ion (11.1V nominal) | Voltage thresholds would be incorrect |
| AS-03 | Shunt resistor = 100 μΩ | Current readings would be scaled incorrectly |

---

## 3. Component Design

### 3.1 Component List

| Component | File | Responsibility | Requirements |
|-----------|------|----------------|--------------|
| Application | `Core/Src/main.c` | Main loop, initialization, protection logic, LED heartbeat | REQ-FUNC-004, REQ-FUNC-005, REQ-FUNC-006 |
| BQ76920 Driver | `Core/Src/bq76920.c`, `Core/Inc/bq76920.h` | I2C communication, register access, data conversion | REQ-FUNC-001, REQ-FUNC-002, REQ-FUNC-003, HWI-01 |
| I2C HAL | `Drivers/STM32F1xx_HAL_Driver/` | Low-level I2C hardware abstraction | HWI-01 |
| GPIO HAL | `Drivers/STM32F1xx_HAL_Driver/` | LED control | UI-02 |

### 3.2 Component Interfaces

#### BQ76920 Driver Public Interface

| Function | Parameters | Return Type | Description |
|----------|------------|-------------|-------------|
| `bq76920_init` | `handle`, `hi2c` | `bool` | Initializes I2C communication with BQ76920 |
| `bq76920_read_voltage` | `handle`, `cell_num` | `uint16_t` | Returns cell voltage in millivolts |
| `bq76920_read_current` | `handle` | `int16_t` | Returns pack current in milliamps |
| `bq76920_read_temperature` | `handle` | `int16_t` | Returns temperature in 0.1°C units |
| `bq76920_enable_charge_fet` | `handle` | `bool` | Enables charge FET |
| `bq76920_disable_charge_fet` | `handle` | `bool` | Disables charge FET |
| `bq76920_enable_discharge_fet` | `handle` | `bool` | Enables discharge FET |
| `bq76920_disable_discharge_fet` | `handle` | `bool` | Disables discharge FET |

#### Application Internal Functions

| Function | Parameters | Return Type | Description | Requirements |
|----------|------------|-------------|-------------|--------------|
| `check_overvoltage` | `cell_voltages[3]` | `void` | Disables charge FET if any cell > 4.25V | REQ-FUNC-004 |
| `check_undervoltage` | `cell_voltages[3]` | `void` | Disables discharge FET if any cell < 2.8V | REQ-FUNC-005 |
| `check_overtemperature` | `temp_tenths` | `void` | Disables both FETs if temperature > 60°C | REQ-FUNC-006 |

---

## 4. Interface Design

### 4.1 Hardware Interface Details

| Interface | Pin | Direction | Protocol | Electrical Spec |
|-----------|-----|-----------|----------|-----------------|
| I2C SCL | PB6 | Output | I2C clock | 100kHz, open-drain, 4.7kΩ pull-up |
| I2C SDA | PB7 | Bidirectional | I2C data | 100kHz, open-drain, 4.7kΩ pull-up |
| SWDIO | PA13 | Bidirectional | SWD data | 4MHz, push-pull |
| SWCLK | PA14 | Input | SWD clock | 4MHz, push-pull |
| LED | PC13 | Output | GPIO | Active low (on = 0V, off = 3.3V) |

### 4.2 I2C Communication Protocol

| Step | Master (STM32) | Slave (BQ76920) |
|------|----------------|-----------------|
| 1 | Send START condition | — |
| 2 | Send 0x10 (address + write) | Send ACK |
| 3 | Send register address (e.g., 0x0C) | Send ACK |
| 4 | Send REPEATED START | — |
| 5 | Send 0x11 (address + read) | Send ACK |
| 6 | Read 2 bytes of data | Send data |
| 7 | Send NACK, then STOP | — |

---

## 5. Data Design

### 5.1 Data Flow

| Step | Source | Action | Destination | Data |
|------|--------|--------|-------------|------|
| 1 | BQ76920 | Measures cell voltages | STM32 (via I2C) | Raw ADC counts (16-bit) |
| 2 | STM32 | Converts counts to millivolts | Application logic | `cell_mV[3]` (uint16_t) |
| 3 | BQ76920 | Measures pack current | STM32 (via I2C) | Raw counts (16-bit) |
| 4 | STM32 | Converts counts to milliamps | Application logic | `current_mA` (int16_t) |
| 5 | BQ76920 | Measures temperature | STM32 (via I2C) | Raw counts (16-bit) |
| 6 | STM32 | Converts counts to 0.1°C | Application logic | `temp_tenths` (int16_t) |
| 7 | Application | Evaluates protection thresholds | FET control logic | Enable/disable flags |
| 8 | Application | Toggles LED | PC13 GPIO | Heartbeat signal |

### 5.2 Key Data Structures

| Variable | Type | Description | Range | Source |
|----------|------|-------------|-------|--------|
| `cell_mV[3]` | `uint16_t` | Cell voltages in millivolts | 2500-4200 | BQ76920 via I2C |
| `current_mA` | `int16_t` | Pack current in milliamps | -10000 to +10000 | BQ76920 via I2C |
| `temp_tenths` | `int16_t` | Temperature in 0.1°C units | -200 to +850 | BQ76920 via I2C |
| `charge_fet_enabled` | `bool` | Charge FET state | true/false | Application decision |
| `discharge_fet_enabled` | `bool` | Discharge FET state | true/false | Application decision |

### 5.3 Data Conversion Algorithms

#### Voltage Conversion

Input: raw_counts (uint16_t from BQ76920 register)
Output: voltage_mV (uint16_t)

Algorithm: voltage_mV = (raw_counts * 3815) / 10000
Rationale: 1 LSB = 381.5 μV = 0.3815 mV

#### Current Conversion

Input: raw_counts (int16_t from BQ76920 register)
Output: current_mA (int16_t)

Algorithm: current_mA = (raw_counts * 3815) / (10 * shunt_uOhms)
Rationale: 1 LSB = 381.5 μV across shunt; shunt = 100 μΩ

#### Temperature Conversion

Input: raw_counts (uint16_t from BQ76920 register)
Output: temp_tenths (int16_t, 0.1°C units)

Algorithm: temp_tenths = raw_counts - 2730
Rationale: 1 LSB = 0.1°K; 2730 = 273.0°C (absolute zero offset)

---

## 6. Control Flow Design

### 6.1 Main Loop Sequence

| Phase | Action | Description |
|-------|--------|-------------|
| 0 | Power-on | System reset, vector table load |
| 1 | HAL Initialization | `HAL_Init()`, clock configuration |
| 2 | Peripheral Initialization | GPIO, I2C, BQ76920 init |
| 3 | ADC Enable | `bq76920_enable_adc()` |
| 4 | FET Enable | `bq76920_enable_charge_fet()`, `bq76920_enable_discharge_fet()` |
| 5 | Main Loop (100ms period) | Read measurements → check protection → toggle LED → delay |

### 6.2 Protection Logic - Overvoltage

| Step | Condition | Action |
|------|-----------|--------|
| 1 | Read cell voltages via I2C | Store in `cell_mV[3]` |
| 2 | For each cell | Compare `cell_mV > 4250` |
| 3 | Overvoltage detected | Call `bq76920_disable_charge_fet()` |
| 4 | Log error | Record overvoltage event |
| 5 | Halt charging | FET stays disabled until voltage falls below 4.2V |

### 6.3 Protection Logic - Undervoltage

| Step | Condition | Action |
|------|-----------|--------|
| 1 | Read cell voltages via I2C | Store in `cell_mV[3]` |
| 2 | For each cell | Compare `cell_mV < 2800` |
| 3 | Undervoltage detected | Call `bq76920_disable_discharge_fet()` |
| 4 | Log error | Record undervoltage event |
| 5 | Halt discharging | FET stays disabled until voltage rises above 3.0V |

### 6.4 Protection Logic - Overtemperature

| Step | Condition | Action |
|------|-----------|--------|
| 1 | Read temperature via I2C | Store in `temp_tenths` |
| 2 | Convert to Celsius | `temp_c = temp_tenths / 10` |
| 3 | Compare `temp_c > 60` | Call `bq76920_disable_charge_fet()` and `bq76920_disable_discharge_fet()` |
| 4 | Log error | Record overtemperature event |
| 5 | Halt operation | FETs stay disabled until temperature falls below 55°C |

## 6.5 State Machine Description

| State | Description | Entry Action | Exit Action | Next State |
|-------|-------------|--------------|-------------|-------------|
| INIT | Initialization after power-on | Configure clocks, GPIO, I2C | Enable ADC | NORMAL |
| NORMAL | Normal operation | Enable FETs | None | FAULT_OV, FAULT_UV, FAULT_OT, SHIP |
| FAULT_OV | Overvoltage fault | Disable charge FET, log error | None | RECOVERY |
| FAULT_UV | Undervoltage fault | Disable discharge FET, log error | None | RECOVERY |
| FAULT_OT | Overtemperature fault | Disable both FETs, log error | None | RECOVERY |
| RECOVERY | Fault recovery check | Verify conditions cleared | Re-enable FETs | NORMAL |
| SHIP_MODE | Low power state | Disable ADC, disable FETs | None | INIT (on wake) |

---

## 7. Design Decisions and Rationale

| Decision | Rationale | Alternatives Rejected |
|----------|-----------|----------------------|
| I2C at 100kHz | BQ76920 maximum supported speed | 400kHz (not supported by BQ76920) |
| Polling instead of interrupts | Simplicity, loop rate sufficient for 100ms protection response | I2C interrupts (unnecessary complexity) |
| Integer math for voltage conversion | Avoid floating point (no FPU on STM32F103), deterministic timing | Floating point (slower, larger code size) |
| `HAL_Delay` for timing | Simple, adequate for 100ms heartbeat | Timer interrupts (overkill for this application) |
| LED on PC13 | Built-in on Blue Pill board, active low | External LED (unnecessary hardware) |
| 2-second startup delay | Gives debugger time to connect before protection logic runs | No delay (debugger can't halt target in time) |

---

## 8. Traceability to Requirements

| Requirement ID | Design Element | Unit Test | Integration Test | System Test |
|----------------|----------------|-----------|------------------|-------------|
| REQ-FUNC-001 | `bq76920_read_voltage()` + counts_to_millivolts() | UT-VOLT-001 | TC-INT-VOLT-001 | TC-SYS-VOLT-001 |
| REQ-FUNC-002 | `bq76920_read_current()` | UT-CURR-001 | TC-INT-CURR-001 | TC-SYS-CURR-001 |
| REQ-FUNC-003 | `bq76920_read_temperature()` | UT-TEMP-001 | TC-INT-TEMP-001 | TC-SYS-TEMP-001 |
| REQ-FUNC-004 | `check_overvoltage()` in main loop | UT-PROT-001 | TC-INT-OV-001 | TC-SYS-OV-001 |
| REQ-FUNC-005 | `check_undervoltage()` in main loop | UT-PROT-002 | TC-INT-UV-001 | TC-SYS-UV-001 |
| REQ-FUNC-006 | `check_overtemperature()` in main loop | UT-PROT-003 | TC-INT-OT-001 | TC-SYS-OT-001 |
| UI-02 | `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` | N/A | N/A | TC-SYS-LED-001 |
| HWI-01 | I2C1 initialization at 100kHz | N/A | TC-INT-I2C-001 | TC-SYS-I2C-001 |
| PERF-01 | Main loop 100ms delay | N/A | N/A | TC-SYS-PERF-001 |

### Test Artifact Locations

| Test Level | Document | Location |
|------------|----------|----------|
| Unit Test | Unit Test Specification | `docs/UNIT_TEST_SPEC.md` |
| Integration Test | Test Specification | `docs/TEST_SPEC.md` |
| System Test | Test Specification | `docs/TEST_SPEC.md` |

---

## 9. Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-05 | Cameron Burnett | Initial SDD creation |

---

*End of Software Design Description*