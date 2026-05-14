# Test Plan: STM32 BQ76920 Battery Management System

## ISO/IEC/IEEE 29119-3 Compliant

---

## Document Information

| Field | Value |
|-------|-------|
| **Test Plan ID** | TP-BMS-001 |
| **Version** | 1.0 |
| **Date** | 2026-05-03 |
| **Author** | Cameron Burnett |
| **Status** | Draft |
| **Document Classification** | Internal |

---

## 1. Introduction

### 1.1 Background

The STM32 BQ76920 Battery Management System (BMS) is designed to monitor and protect a 3-cell Li-ion battery pack. This test plan defines the strategy for verifying that the system meets its functional, safety, and performance requirements.

### 1.2 Scope

This test plan covers the following test levels:

- **Static Analysis** – Code quality, MISRA/CERT C compliance (Cppcheck)
- **Integration Testing** – I2C communication, voltage/current measurement chain, protection logic integration
- **System Testing** – Complete BMS functionality with real battery pack
- **Safety (Fault Injection)** – Overvoltage, undervoltage, I2C timeout responses

### 1.3 Out of Scope

- Environmental testing (temperature, humidity, vibration)
- EMI/EMC compliance testing
- Production/acceptance testing

### 1.4 References

| Document | Location |
|----------|----------|
| Software Requirements Specification | `docs/SRS.md` |
| Software Design Description | `docs/SDD.md` |
| Hazard Analysis (HARA) | `docs/HARA.md` |
| BQ76920 Datasheet | Texas Instruments (external) |
| STM32F103 Datasheet | STMicroelectronics (external) |

### 1.5 Definitions and Acronyms

| Term | Definition |
|------|------------|
| BMS | Battery Management System |
| DUT | Device Under Test |
| FET | Field Effect Transistor (charge/discharge switch) |
| HIL | Hardware-in-the-Loop |
| I2C | Inter-Integrated Circuit (communication protocol) |
| PASS/FAIL | Binary test outcome |
| SRS | Software Requirements Specification |
| SWD | Serial Wire Debug |

---

## 2. Test Items

The following artifacts will be tested:

| Item | Description | Location |
|------|-------------|----------|
| `bq76920.c` | BQ76920 driver source code | `Core/Src/` |
| `bq76920.h` | BQ76920 driver header | `Core/Inc/` |
| `main.c` | Application logic and protection | `Core/Src/` |
| STM32F103 target | Hardware platform | Blue Pill board |
| BQ76920 EVM | Battery monitor hardware | Evaluation board |

---

## 3. Features to Be Tested

**Note:** Feature IDs below map to SRS requirements as follows:

| Feature ID | SRS Requirement |
|------------|-----------------|
| FUN-VOLT-01 | REQ-FUNC-001 |
| FUN-CURR-01 | REQ-FUNC-002 |
| FUN-TEMP-01 | REQ-FUNC-003 |
| FUN-I2C-01 | HWI-01 |
| SAFE-OV-01 | REQ-FUNC-004 |
| SAFE-UV-01 | REQ-FUNC-005 |
| SAFE-OT-01 | REQ-FUNC-006 |
| PERF-01 | PERF-01 |

| Feature ID | Description | Priority | Test Coverage |
|------------|-------------|----------|---------------|
| FUN-VOLT-01 | Read individual cell voltages | High | 100% of voltage range (2.5V-4.2V) |
| FUN-CURR-01 | Read pack current via shunt | High | -10A to +10A range |
| FUN-TEMP-01 | Read temperature | Medium | -20°C to +60°C |
| FUN-I2C-01 | I2C communication at 100kHz | High | All register reads/writes |
| SAFE-OV-01 | Overvoltage protection | High | Trip at 4.25V |
| SAFE-UV-01 | Undervoltage protection | High | Trip at 2.8V |
| SAFE-OT-01 | Overtemperature protection | Medium | Trip at 60°C |
| PERF-01 | Measurement cycle time | Low | < 100ms |

---

## 4. Features Not to Be Tested

| Feature | Reason for Exclusion |
|---------|----------------------|
| Cell balancing | Hardware not populated on BQ76920EVM |
| Coulomb counting | Future feature (planned for v2) |
| WiFi telemetry | ESP8266 integration planned for v2 |
| CAN communication | Not supported on Blue Pill hardware |

---

## 5. Test Approach and Strategy

### 5.1 Test Levels

| Level | What It Tests | Method | Tool/Environment |
|-------|---------------|--------|------------------|
| **Static Analysis** | Code quality, MISRA/CERT C compliance, potential bugs | Automated source code inspection | Cppcheck on host PC |
| **Integration** | I2C communication, voltage/current reading chain, protection logic | Manual execution with debugger | STM32 + BQ76920 on bench |
| **System** | Complete BMS behavior with real battery | Manual execution with real load | STM32 + BQ76920 + 3S battery pack |
| **Safety (Fault Injection)** | Overvoltage, undervoltage, I2C timeout responses | Simulate fault conditions | STM32 + BQ76920 + battery simulator |

### 5.2 Test Techniques

| Technique | Definition | Application in BMS Project |
|-----------|------------|---------------------------|
| **Black box** | Testing based on inputs and outputs without knowing internal code | Apply 3.7V to BQ76920 cell input → verify STM32 reports 3.7V ±5mV |
| **White box** | Testing based on internal code structure | Force `cell_mV > 4200` condition → verify overvoltage protection branch executes |
| **Failure mode** | Testing system response to faults | Disconnect I2C during operation → verify FETs disable and error flag sets |

### 5.3 Pass/Fail Criteria

A test is considered **PASSED** if:
- The actual result matches the expected result
- No unexpected errors or crashes occur
- All safety requirements are satisfied

A test is considered **FAILED** if:
- The actual result deviates from expected
- Any requirement is not met
- The system enters an undefined state
- A safety violation occurs

### 5.4 Test Environment

| Component | Specification |
|-----------|---------------|
| **Hardware** | STM32F103C8T6 "Blue Pill", BQ76920EVM, ST-LINK V2 |
| **Software** | ARM GCC 14.3.1, OpenOCD, VS Code |
| **Power supply** | Adjustable DC supply (0-20V) for battery simulation with inbuilt 5v USB for STM32 |
| **Measurement** | Oscilloscope (I2C signals), Multimeter (voltages) |
| **Debug interface** | SWD via ST-LINK V2 |

### 5.5 Test Data

| Test Data | Source | Purpose |
|-----------|--------|---------|
| Simulated cell voltages | Potentiometer divider | 0-5V range for testing |
| Simulated current | Precision shunt resistor | ±10A range |
| Simulated temperature | Variable resistor (NTC emulation) | -20°C to +60°C |
| Real battery | 3S 18650 pack | Final validation |

### 5.6 Test Tools

| Tool | Purpose |
|------|---------|
| Cppcheck | Static analysis for MISRA/CERT C |
| OpenOCD | Flashing and debugging |
| Saleae Logic Analyzer (or clone) | I2C bus protocol analysis |
| Oscilloscope | Signal integrity validation |
| Multimeter | Voltage verification |

---

## 6. Test Deliverables

| Deliverable | Format | Due Date |
|-------------|--------|----------|
| Test Plan | `docs/TEST_PLAN.md` | Before testing begins |
| Test Specifications | `docs/TEST_SPEC.md` | Before testing begins |
| Test Logs | `test_logs/` directory | During testing |
| Test Report | `docs/TEST_REPORT.md` | After testing completes |
| Defect Log | `docs/DEFECT_LOG.md` | During testing |
| Traceability Matrix | `docs/TRACE_MATRIX.md` | After testing |

---

## 7. Test Schedule

| Phase | Activities | Duration | Target Date | Status |
|-------|------------|----------|-------------|--------|
| **Phase 1** | I2C communication validation (BQ76920 detection) | 1 hour | Day 1 | ✅ Complete |
| **Phase 2** | Voltage measurement accuracy testing | 1 hour | Day 1 | ⏳ Pending |
| **Phase 3** | Current measurement validation | 1 hour | Day 2 | ⏳ Pending |
| **Phase 4** | Temperature measurement validation | 1 hour | Day 2 | ⏳ Pending |
| **Phase 5** | Protection feature testing (OV, UV, OT) | 2 hours | Day 2 | ⏳ Pending |
| **Phase 6** | Integration testing with real 3S battery pack | 2 hours | Day 2 | ⏳ Pending |
| **Phase 7** | Static analysis (cppcheck) | 1 hour | Day 2 | ⏳ Pending |
| **Phase 8** | Documentation and test report | 1 hour | Day 2 | ⏳ Pending |

**Total estimated time:** ~10 hours

---

## 8. Resource Requirements

### 8.1 Hardware

| Item | Quantity | Status |
|------|----------|--------|
| STM32F103 Blue Pill | 1 | Available |
| BQ76920EVM | 1 | Available |
| ST-LINK V2 | 1 | Available |
| 3S 18650 Battery Pack | 1 | Available |
| Oscilloscope | 1 | Unavailable (will use logic analyzer for I2C, multimeter for DC voltages) |
| Multimeter | 1 | Available |

### 8.2 Software

| Item | Status |
|------|--------|
| ARM GCC Toolchain | Available |
| VS Code | Available |
| OpenOCD | Available |
| Cppcheck | Available |

---

## 9. Risks and Contingencies

| Risk ID | Description | Likelihood | Severity | Mitigation |
|---------|-------------|------------|----------|------------|
| R-01 | BQ76920 not detected on I2C bus | Medium | High | Verify pull-up resistors, check wiring, test with I2C scanner |
| R-02 | ST-LINK fails to halt target | Medium | High | Use BOOT0 method for flashing (documented in README) |
| R-03 | Real battery safety risk during testing | Low | Critical | Use battery simulator first, only use real battery for final validation |
| R-04 | Test schedule slips | Medium | Low | Prioritize high-risk features first |

---

## 10. Test Management and Approvals

### 10.1 Test Team

| Role | Name | Responsibilities |
|------|------|------------------|
| Tester/Developer | Cameron Burnett | Execute all tests, document results, fix defects |

### 10.2 Test Reporting

| Report | Frequency | Recipient |
|--------|-----------|-----------|
| Test execution log | Daily | GitHub issues |
| Test Summary Report | At completion | GitHub README |

### 10.3 Approvals

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Author | Cameron Burnett | | |
| Reviewer | (Peer review) | | |

---

## 11. Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-03 | Cameron Burnett | Initial test plan creation |

---

## Appendix A: Test Environment Setup

### A.1 Hardware Setup

- PC (VS Code) → USB → ST-LINK V2 → SWD → STM32F103 (Blue Pill)
- STM32F103 → I2C (PB6/PB7) → BQ76920EVM
- BQ76920EVM → 3S Battery Pack

### A.2 Software Setup

1. Clone repository
2. Run `cmake --build build/Debug`
3. Flash using VS Code
4. Run tests via test scripts

---

## Appendix B: Traceability Matrix Example

| Requirement ID | Test Case ID | Test Type | Status |
|----------------|--------------|-----------|--------|
| REQ-FUNC-001 | UT-VOLT-001 | Unit | Not Run |
| REQ-FUNC-001 | TC-INT-VOLT-001 | Integration | Not Run |
| REQ-FUNC-001 | TC-SYS-VOLT-001 | System | Not Run |
| REQ-FUNC-004 | UT-PROT-001 | Unit | Not Run |
| REQ-FUNC-004 | TC-INT-OV-001 | Integration | Not Run |
| REQ-FUNC-004 | TC-SYS-OV-001 | System | Not Run |

---

*End of Test Plan*