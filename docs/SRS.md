# Software Requirements Specification
## STM32 BQ76920 Battery Management System

**Version:** 1.0  
**Prepared by:** Cameron Burnett  
**Date:** 2026-05-03  
**Status:** Draft

---

## Table of Contents
<!-- TOC -->
* [1. Introduction](#1-introduction)
    * [1.1 Document Purpose](#11-document-purpose)
    * [1.2 Product Scope](#12-product-scope)
    * [1.3 Definitions, Acronyms, and Abbreviations](#13-definitions-acronyms-and-abbreviations)
    * [1.4 References](#14-references)
    * [1.5 Document Overview](#15-document-overview)
* [2. Product Overview](#2-product-overview)
    * [2.1 Product Perspective](#21-product-perspective)
    * [2.2 Product Functions](#22-product-functions)
    * [2.3 Product Constraints](#23-product-constraints)
    * [2.4 User Characteristics](#24-user-characteristics)
    * [2.5 Assumptions and Dependencies](#25-assumptions-and-dependencies)
    * [2.6 Apportioning of Requirements](#26-apportioning-of-requirements)
* [3. Requirements](#3-requirements)
    * [3.1 External Interfaces](#31-external-interfaces)
    * [3.2 Functional](#32-functional)
    * [3.3 Quality of Service](#33-quality-of-service)
    * [3.4 Compliance](#34-compliance)
    * [3.5 Design and Implementation](#35-design-and-implementation)
* [4. Verification](#4-verification)
* [5. Appendixes](#5-appendixes)
<!-- TOC -->

---

## Revision History

| Name | Date | Reason For Changes | Version |
|------|------|--------------------|---------|
| Cameron Burnett | 2026-05-03 | Initial creation | 1.0 |
| | | | |
| | | | |

---

## 1. Introduction

### 1.1 Document Purpose

This Software Requirements Specification (SRS) documents the functional, performance, safety, and interface requirements for the STM32 BQ76920 Battery Management System (BMS). The primary audiences for this document are:

- **Developer (Cameron Burnett):** For implementation guidance and acceptance criteria
- **Portfolio Reviewers (Employers):** To demonstrate professional engineering practices
- **Future Maintainers:** To understand system requirements and design rationale

This SRS defines **what** the system must do, not **how** it will be implemented.

### 1.2 Product Scope

The STM32 BQ76920 Battery Management System is a 3-cell Li-ion battery monitor and protection system built around an STM32F103 microcontroller and Texas Instruments BQ76920 analog front-end.

**Primary capabilities include:**
- Individual cell voltage monitoring (±5mV accuracy)
- Pack current measurement via external shunt
- Temperature monitoring
- Overvoltage/undervoltage/overtemperature protection
- I2C communication between STM32 and BQ76920

**Inclusions:**
- STM32F103 "Blue Pill" as main MCU
- BQ76920EVM evaluation board as AFE
- I2C communication at 100kHz
- SWD debugging via ST-LINK V2

**Exclusions (phase 1):**
- Cell balancing (hardware not populated on BQ76920EVM)
- Coulomb counting
- WiFi telemetry
- CAN communication

### 1.3 Definitions, Acronyms, and Abbreviations

| Term | Definition |
|------|------------|
| AFE | Analog Front End - BQ76920 battery monitor IC |
| BMS | Battery Management System |
| DUT | Device Under Test |
| FET | Field Effect Transistor (charge/discharge switch) |
| I2C | Inter-Integrated Circuit (communication protocol) |
| MCU | Microcontroller Unit (STM32F103) |
| SRS | Software Requirements Specification |
| SWD | Serial Wire Debug (programming/debug interface) |

### 1.4 References

| Document | Location | Type |
|----------|----------|------|
| BQ76920 Datasheet (SLUSBZ2E) | Texas Instruments | Normative |
| STM32F103 Datasheet (DS5319) | STMicroelectronics | Normative |
| Test Plan | `docs/TEST_PLAN.md` | Informative |
| Hazard Analysis (HARA) | `docs/HARA.md` | Informative |

### 1.5 Document Overview

Section 2 provides product background and context. Section 3 contains all requirements organized by category (interfaces, functional, quality of service, compliance, design). Section 4 describes verification methods. Section 5 contains appendixes.

---

## 2. Product Overview

### 2.1 Product Perspective

The BMS is a standalone embedded system that monitors a 3-cell Li-ion battery pack. It is a new product designed for portable electronics and small battery applications. The system provides voltage, current, and temperature data to a user via debug UART (future debugging) and controls external FETs for battery protection.

**External Interfaces:**
- I2C to BQ76920 AFE
- SWD to ST-LINK V2 debugger
- GPIO to charge/discharge FETs
- UART (optional debug output)

### 2.2 Product Functions

The BMS provides the following major functional areas:

- **Cell voltage monitoring:** Read voltages of 3 series cells via BQ76920
- **Current monitoring:** Measure pack current via external shunt
- **Temperature monitoring:** Measure battery temperature via NTC thermistor
- **Protection logic:** Disable charge/discharge FETs on fault conditions
- **I2C communication:** Reliable communication between STM32 and BQ76920
- **Static analysis:** MISRA/CERT C compliance verification via Cppcheck

### 2.3 Product Constraints

| ID | Constraint | Source |
|----|------------|--------|
| CON-01 | I2C communication speed fixed at 100kHz (BQ76920 maximum) | BQ76920 datasheet |
| CON-02 | MCU operating voltage: 3.3V | STM32F103 specification |
| CON-03 | Programming language: C11 with GNU extensions | Project decision |
| CON-04 | Static analysis required: MISRA C:2012, CERT C | Quality standard |
| CON-05 | No dynamic memory allocation (malloc) | MISRA Rule 21.10 |

### 2.4 User Characteristics

| User Class | Description | Expertise |
|------------|-------------|-----------|
| Developer | Implements and tests the BMS | High (embedded C, STM32) |
| Portfolio Reviewer | Evaluates project quality | Medium (software engineering) |

### 2.5 Assumptions and Dependencies

**Assumptions:**

| ID | Assumption | Impact if False |
|----|------------|-----------------|
| ASM-01 | BQ76920EVM is correctly wired to STM32 via I2C | No communication, BMS non-functional |
| ASM-02 | 4.7kΩ pull-up resistors installed on I2C lines | I2C bus errors, unreliable communication |
| ASM-03 | ST-LINK V2 clone can communicate with target | Cannot flash or debug |

**Dependencies:**

| ID | Dependency | Owner |
|----|------------|-------|
| DEP-01 | ARM GCC toolchain | Open source |
| DEP-02 | STM32Cube HAL drivers | STMicroelectronics |
| DEP-03 | Cppcheck static analyzer | Open source |

### 2.6 Apportioning of Requirements

| Phase | Requirement ID | Description |
|-------|----------------|-------------|
| Phase 1 (current) | REQ-FUNC-001 | Cell voltage monitoring |
| Phase 1 (current) | REQ-FUNC-002 | Pack current monitoring |
| Phase 1 (current) | REQ-FUNC-003 | Temperature monitoring |
| Phase 1 (current) | REQ-FUNC-004 | Overvoltage protection |
| Phase 1 (current) | REQ-FUNC-005 | Undervoltage protection |
| Phase 1 (current) | REQ-FUNC-006 | Overtemperature protection |
| Phase 1 (current) | HWI-01 | I2C communication |
| Phase 1 (current) | PERF-01 | Cycle time <100ms |
| Phase 2 (future) | (To be defined) | Cell balancing, Coulomb counting, WiFi telemetry |

---

## 3. Requirements

### 3.1 External Interfaces

#### 3.1.1 User Interfaces

| ID | Requirement | Priority |
|----|-------------|----------|
| <a id="ui-01"></a>UI-01 | Debug UART output shall display cell voltages, current, and temperature when enabled | Low |
| <a id="ui-02"></a>UI-02 | LED on PC13 shall blink at 1Hz to indicate system is operational (heartbeat) | Medium |

#### 3.1.2 Hardware Interfaces

| ID | Requirement | Priority |
|----|-------------|----------|
| <a id="hwi-01"></a>HWI-01 | I2C1 on PB6 (SCL) and PB7 (SDA) shall communicate with BQ76920 at 100kHz | High |
| <a id="hwi-02"></a>HWI-02 | SWD interface on PA13 (SWDIO) and PA14 (SWCLK) shall support debugging and flashing | High |
| <a id="hwi-03"></a>HWI-03 | GPIO pin controlling charge FET shall be active-high (enable when high) | High |
| <a id="hwi-04"></a>HWI-04 | GPIO pin controlling discharge FET shall be active-high (enable when high) | High |

#### 3.1.3 Software Interfaces

<a id="swi-01"></a><a id="swi-02"></a><a id="swi-03"></a><a id="swi-04"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| SWI-01 | BQ76920 driver shall provide `bq76920_read_voltage(cell)` function | High |
| SWI-02 | BQ76920 driver shall provide `bq76920_read_current()` function | High |
| SWI-03 | BQ76920 driver shall provide `bq76920_read_temperature()` function | High |
| SWI-04 | BQ76920 driver shall provide `bq76920_enable_fets()` and `bq76920_disable_fets()` | High |

### 3.2 Functional

The following functional requirements apply to the BMS system:

| ID | Requirement | Priority |
|----|-------------|----------|
| <a id="req-func-001"></a>REQ-FUNC-001 | System shall measure each cell voltage via BQ76920 I2C with ±5mV accuracy (2.5V-4.2V range) | High |
| <a id="req-func-002"></a>REQ-FUNC-002 | System shall measure pack current via external shunt with ±10A range | High |
| <a id="req-func-003"></a>REQ-FUNC-003 | System shall measure battery temperature via BQ76920 internal temperature sensor | Medium |
| <a id="req-func-004"></a>REQ-FUNC-004 | System shall disable charge FET when any cell exceeds 4.25V | High |
| <a id="req-func-005"></a>REQ-FUNC-005 | System shall disable discharge FET when any cell falls below 2.8V | High |
| <a id="req-func-006"></a>REQ-FUNC-006 | System shall disable both FETs when temperature exceeds 60°C | Medium |

### 3.3 Quality of Service

#### 3.3.1 Performance

| ID | Requirement | Target | Priority |
|----|-------------|--------|----------|
| <a id="perf-01"></a>PERF-01 | Complete measurement cycle (all cell voltages + current + temperature) | < 100ms | Medium |
| PERF-02 | I2C register read operation | < 10ms | Medium |
| PERF-03 | Protection fault response time (detection to FET disable) | < 10ms | High |

#### 3.3.2 Security

<a id="sec-01"></a><a id="sec-02"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| SEC-01 | No sensitive data (keys, passwords) stored in firmware | High |
| SEC-02 | SWD debug interface shall remain enabled for development (no readout protection in debug builds) | Medium |

#### 3.3.3 Reliability

<a id="rel-01"></a><a id="rel-02"></a><a id="rel-03"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| REL-01 | I2C communication shall include timeout mechanism (100ms) to prevent bus lock | High |
| REL-02 | System shall recover from I2C bus errors without watchdog reset | Medium |
| REL-03 | Protection logic shall be evaluated at least once per main loop iteration | High |

### 3.4 Compliance

<a id="cmp-01"></a><a id="cmp-02"></a><a id="cmp-03"></a><a id="cmp-04"></a>

| ID | Requirement | Standard | Priority |
|----|-------------|----------|----------|
| CMP-01 | Code shall comply with MISRA C:2012 rules as enforced by Cppcheck | MISRA C:2012 | High |
| CMP-02 | Code shall comply with CERT C rules as enforced by Cppcheck | CERT C | High |
| CMP-03 | Static analysis shall pass with zero violations (suppressions documented) | Project quality | High |
| CMP-04 | Coding style shall follow `CODING_STYLE.md` in project root | Internal | Medium |

### 3.5 Design and Implementation

#### 3.5.1 Installation

<a id="ins-01"></a><a id="ins-02"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| INS-01 | Firmware shall be flashable via ST-LINK V2 using OpenOCD | High |
| INS-02 | BOOT0 jumper method shall be documented for ST-LINK clone recovery | Medium |

#### 3.5.2 Build and Delivery

<a id="bld-01"></a><a id="bld-02"></a><a id="bld-03"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| BLD-01 | Project shall build with CMake and ARM GCC toolchain | High |
| BLD-02 | Build shall produce .elf, .bin, and .hex output files | Medium |
| BLD-03 | CI build (GitHub Actions) shall run cppcheck automatically | Low (future) |

#### 3.5.3 Maintainability

<a id="mnt-01"></a><a id="mnt-02"></a><a id="mnt-03"></a>

| ID | Requirement | Priority |
|----|-------------|----------|
| MNT-01 | Code shall be documented with Doxygen-style comments for public functions | Medium |
| MNT-02 | HAL driver files shall not be modified (to preserve CubeMX compatibility) | High |
| MNT-03 | Suppressions in `cppcheck_suppressions.txt` shall be documented with rationale | Medium |

---

## 4. Verification

Verification confirms that each requirement has been implemented correctly and functions as specified. Following the V-Model, each requirement is verified at multiple test levels:

| Test Level | Definition | Environment | Responsibility |
|------------|------------|-------------|----------------|
| **Unit Test** | Validates individual functions in isolation (e.g., voltage conversion formula) | Host PC (cross-compiled or native) | Developer |
| **Integration Test** | Validates that modules work together (e.g., I2C communication + voltage read) | STM32 + BQ76920 on bench | Developer |
| **System Test** | Validates complete system behavior (e.g., overvoltage protection with real battery) | STM32 + BQ76920 + battery pack | Developer |

### Verification Summary

| Requirement | Description | Unit Test | Integration Test | System Test | Status |
|-------------|-------------|-----------|------------------|-------------|--------|
| <a href="#req-func-001">REQ-FUNC-001</a> | Read cell voltage (±5mV accuracy) | UT-VOLT-001: Conversion formula accuracy | TC-INT-VOLT-001: I2C + voltage read on hardware | TC-SYS-VOLT-001: Full measurement with real battery | Not Run |
| <a href="#req-func-002">REQ-FUNC-002</a> | Read pack current (±10A range) | UT-CURR-001: Conversion formula accuracy | TC-INT-CURR-001: I2C + current read on hardware | TC-SYS-CURR-001: Full measurement with real battery | Not Run |
| <a href="#req-func-003">REQ-FUNC-003</a> | Read temperature (±1°C) | UT-TEMP-001: Conversion formula accuracy | TC-INT-TEMP-001: I2C + temp read on hardware | TC-SYS-TEMP-001: Full measurement with real battery | Not Run |
| <a href="#req-func-004">REQ-FUNC-004</a> | Overvoltage protection (4.25V cutoff) | UT-PROT-001: Detection logic | TC-INT-OV-001: Detection + FET control integration | TC-SYS-OV-001: Full protection with real battery | Not Run |
| <a href="#req-func-005">REQ-FUNC-005</a> | Undervoltage protection (2.8V cutoff) | UT-PROT-002: Detection logic | TC-INT-UV-001: Detection + FET control integration | TC-SYS-UV-001: Full protection with real battery | Not Run |
| <a href="#req-func-006">REQ-FUNC-006</a> | Overtemperature protection (60°C cutoff) | UT-PROT-003: Detection logic | TC-INT-OT-001: Detection + FET control integration | TC-SYS-OT-001: Full protection with real battery | Not Run |
| <a href="#hwi-01">HWI-01</a> | I2C communication at 100kHz | N/A | TC-INT-I2C-001: I2C bus scan and register read | TC-SYS-I2C-001: Reliable comms with real battery | Not Run |
| <a href="#ui-02">UI-02</a> | LED heartbeat (1Hz) | N/A | N/A | TC-SYS-LED-001: Visual confirmation of LED blink | Not Run |
| <a href="#perf-01">PERF-01</a> | Cycle time <100ms | N/A | N/A | TC-SYS-PERF-001: Timing measurement with oscilloscope | Not Run |
| <a href="#cmp-01">CMP-01</a> | MISRA C:2012 compliance | N/A | N/A | N/A | Pass (Cppcheck) |
| <a href="#cmp-02">CMP-02</a> | CERT C compliance | N/A | N/A | N/A | Pass (Cppcheck) |

### Status Legend

| Status | Meaning |
|--------|---------|
| Pass | All tests for this requirement have passed |
| Fail | One or more tests for this requirement have failed |
| Not Run | Tests have not been executed yet |

### Test Artifact Locations

| Test Level | Document | Location |
|------------|----------|----------|
| Unit Test | Unit Test Specification | `docs/UNIT_TEST_SPEC.md` |
| Integration Test | Test Specification | `docs/TEST_SPEC.md` |
| System Test | Test Specification | `docs/TEST_SPEC.md` |

### How to Use This Section

1. Each row shows one requirement and its verification strategy
2. The "Description" column explains what the requirement does
3. Each test cell describes what that specific test verifies
4. Execute tests in order: Unit → Integration → System
5. Update the Status column as tests pass
6. Click requirement IDs to jump to their full description in Section 3

---

## 5. Appendixes

### Appendix A: Traceability Matrix

Traceability matrix stored in `docs/TRACE_MATRIX.md`.

| Requirement ID | Source (Backward Trace) | Test Case ID (Forward Trace) | Status |
|----------------|-------------------------|------------------------------|--------|
| <a href="#req-func-001">REQ-FUNC-001</a> | HARA (overvoltage risk) | <a href="tests/unit/UT-VOLT-001.md">UT-VOLT-001</a> | Not Run |
| <a href="#req-func-004">REQ-FUNC-004</a> | HARA (overvoltage risk) | <a href="tests/unit/UT-PROT-001.md">UT-PROT-001</a> | Not Run |
| <a href="#req-func-005">REQ-FUNC-005</a> | HARA (undervoltage risk) | <a href="tests/unit/UT-PROT-002.md">UT-PROT-002</a> | Not Run |
| <a href="#req-func-006">REQ-FUNC-006</a> | HARA (overtemperature risk) | <a href="tests/unit/UT-PROT-003.md">UT-PROT-003</a> | Not Run |

### Appendix B: Requirement ID Schema

| Prefix | Meaning |
|--------|---------|
| REQ-FUNC-XXX | Functional requirement |
| REQ-SAFE-XXX | Safety requirement |
| REQ-INT-XXX | Interface requirement |
| REQ-PERF-XXX | Performance requirement |
| REQ-CMP-XXX | Compliance requirement |

---

*End of Software Requirements Specification*