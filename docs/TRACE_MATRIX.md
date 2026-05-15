# Traceability Matrix
## STM32 BQ76920 Battery Management System

## Requirement to Test Traceability

| Requirement ID | Requirement Summary | Unit Test | Integration Test | System Test | Acceptance Test |
|----------------|---------------------|-----------|------------------|-------------|-----------------|
| <a href="SRS.md#req-func-001" target="_blank">REQ-FUNC-001</a> | Read cell voltage (±5mV) | <a href="tests/unit/UT-VOLT-001.md" target="_blank">UT-VOLT-001</a> | <a href="tests/integration/TC-INT-VOLT-001.md" target="_blank">TC-INT-VOLT-001</a> | <a href="tests/system/TC-SYS-VOLT-001.md" target="_blank">TC-SYS-VOLT-001</a> | HARA validation |
| <a href="SRS.md#req-func-002" target="_blank">REQ-FUNC-002</a> | Read pack current | <a href="tests/unit/UT-CURR-001.md" target="_blank">UT-CURR-001</a> | <a href="tests/integration/TC-INT-CURR-001.md" target="_blank">TC-INT-CURR-001</a> | <a href="tests/system/TC-SYS-CURR-001.md" target="_blank">TC-SYS-CURR-001</a> | HARA validation |
| <a href="SRS.md#req-func-003" target="_blank">REQ-FUNC-003</a> | Read temperature | <a href="tests/unit/UT-TEMP-001.md" target="_blank">UT-TEMP-001</a> | <a href="tests/integration/TC-INT-TEMP-001.md" target="_blank">TC-INT-TEMP-001</a> | <a href="tests/system/TC-SYS-TEMP-001.md" target="_blank">TC-SYS-TEMP-001</a> | HARA validation |
| <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | Overvoltage protection | <a href="tests/unit/UT-PROT-001.md" target="_blank">UT-PROT-001</a> | <a href="tests/integration/TC-INT-OV-001.md" target="_blank">TC-INT-OV-001</a> | <a href="tests/system/TC-SYS-OV-001.md" target="_blank">TC-SYS-OV-001</a> | HARA validation |
| <a href="SRS.md#req-func-005" target="_blank">REQ-FUNC-005</a> | Undervoltage protection | <a href="tests/unit/UT-PROT-002.md" target="_blank">UT-PROT-002</a> | <a href="tests/integration/TC-INT-UV-001.md" target="_blank">TC-INT-UV-001</a> | <a href="tests/system/TC-SYS-UV-001.md" target="_blank">TC-SYS-UV-001</a> | HARA validation |
| <a href="SRS.md#req-func-006" target="_blank">REQ-FUNC-006</a> | Overtemperature protection | <a href="tests/unit/UT-PROT-003.md" target="_blank">UT-PROT-003</a> | <a href="tests/integration/TC-INT-OT-001.md" target="_blank">TC-INT-OT-001</a> | <a href="tests/system/TC-SYS-OT-001.md" target="_blank">TC-SYS-OT-001</a> | HARA validation |
| <a href="SRS.md#ui-02" target="_blank">UI-02</a> | LED heartbeat | N/A | N/A | <a href="tests/system/TC-SYS-LED-001.md" target="_blank">TC-SYS-LED-001</a> | N/A |
| <a href="SRS.md#hwi-03" target="_blank">HWI-03</a> | Charge FET active-high control | N/A | N/A | <a href="tests/system/TC-SYS-FET-001.md" target="_blank">TC-SYS-FET-001</a> | HARA validation |
| <a href="SRS.md#hwi-04" target="_blank">HWI-04</a> | Discharge FET active-high control | N/A | N/A | <a href="tests/system/TC-SYS-FET-002.md" target="_blank">TC-SYS-FET-002</a> | HARA validation |
| <a href="SRS.md#hwi-01" target="_blank">HWI-01</a> | I2C communication | N/A | <a href="tests/integration/TC-INT-I2C-001.md" target="_blank">TC-INT-I2C-001</a> | <a href="tests/system/TC-SYS-I2C-001.md" target="_blank">TC-SYS-I2C-001</a> | HARA validation |
| <a href="SRS.md#perf-01" target="_blank">PERF-01</a> | Cycle time <100ms | N/A | N/A | <a href="tests/system/TC-SYS-PERF-001.md" target="_blank">TC-SYS-PERF-001</a> | N/A |

## Test to Requirement Traceability

| Test ID | Test Level | Verified Requirements | Status |
|---------|------------|----------------------|--------|
| <a href="tests/unit/UT-VOLT-001.md" target="_blank">UT-VOLT-001</a> | Unit | <a href="SRS.md#req-func-001" target="_blank">REQ-FUNC-001</a> | Pass |
| <a href="tests/unit/UT-CURR-001.md" target="_blank">UT-CURR-001</a> | Unit | <a href="SRS.md#req-func-002" target="_blank">REQ-FUNC-002</a> | Not Run |
| <a href="tests/unit/UT-TEMP-001.md" target="_blank">UT-TEMP-001</a> | Unit | <a href="SRS.md#req-func-003" target="_blank">REQ-FUNC-003</a> | Pass |
| <a href="tests/unit/UT-PROT-001.md" target="_blank">UT-PROT-001</a> | Unit | <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | Not Run |
| <a href="tests/unit/UT-PROT-002.md" target="_blank">UT-PROT-002</a> | Unit | <a href="SRS.md#req-func-005" target="_blank">REQ-FUNC-005</a> | Not Run |
| <a href="tests/unit/UT-PROT-003.md" target="_blank">UT-PROT-003</a> | Unit | <a href="SRS.md#req-func-006" target="_blank">REQ-FUNC-006</a> | Not Run |
| <a href="tests/integration/TC-INT-VOLT-001.md" target="_blank">TC-INT-VOLT-001</a> | Integration | <a href="SRS.md#req-func-001" target="_blank">REQ-FUNC-001</a> | Pass |
| <a href="tests/integration/TC-INT-CURR-001.md" target="_blank">TC-INT-CURR-001</a> | Integration | <a href="SRS.md#req-func-002" target="_blank">REQ-FUNC-002</a> | Not Run |
| <a href="tests/integration/TC-INT-TEMP-001.md" target="_blank">TC-INT-TEMP-001</a> | Integration | <a href="SRS.md#req-func-003" target="_blank">REQ-FUNC-003</a> | Pass |
| <a href="tests/integration/TC-INT-OV-001.md" target="_blank">TC-INT-OV-001</a> | Integration | <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | Not Run |
| <a href="tests/integration/TC-INT-UV-001.md" target="_blank">TC-INT-UV-001</a> | Integration | <a href="SRS.md#req-func-005" target="_blank">REQ-FUNC-005</a> | Not Run |
| <a href="tests/integration/TC-INT-OT-001.md" target="_blank">TC-INT-OT-001</a> | Integration | <a href="SRS.md#req-func-006" target="_blank">REQ-FUNC-006</a> | Not Run |
| <a href="tests/integration/TC-INT-I2C-001.md" target="_blank">TC-INT-I2C-001</a> | Integration | <a href="SRS.md#hwi-01" target="_blank">HWI-01</a> | Pass |
| <a href="tests/system/TC-SYS-VOLT-001.md" target="_blank">TC-SYS-VOLT-001</a> | System | <a href="SRS.md#req-func-001" target="_blank">REQ-FUNC-001</a> | Not Run |
| <a href="tests/system/TC-SYS-CURR-001.md" target="_blank">TC-SYS-CURR-001</a> | System | <a href="SRS.md#req-func-002" target="_blank">REQ-FUNC-002</a> | Not Run |
| <a href="tests/system/TC-SYS-TEMP-001.md" target="_blank">TC-SYS-TEMP-001</a> | System | <a href="SRS.md#req-func-003" target="_blank">REQ-FUNC-003</a> | Not Run |
| <a href="tests/system/TC-SYS-OV-001.md" target="_blank">TC-SYS-OV-001</a> | System | <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | Not Run |
| <a href="tests/system/TC-SYS-UV-001.md" target="_blank">TC-SYS-UV-001</a> | System | <a href="SRS.md#req-func-005" target="_blank">REQ-FUNC-005</a> | Not Run |
| <a href="tests/system/TC-SYS-OT-001.md" target="_blank">TC-SYS-OT-001</a> | System | <a href="SRS.md#req-func-006" target="_blank">REQ-FUNC-006</a> | Not Run |
| <a href="tests/system/TC-SYS-FET-001.md" target="_blank">TC-SYS-FET-001</a> | System | <a href="SRS.md#hwi-03" target="_blank">HWI-03</a> | Not Run |
| <a href="tests/system/TC-SYS-FET-002.md" target="_blank">TC-SYS-FET-002</a> | System | <a href="SRS.md#hwi-04" target="_blank">HWI-04</a> | Not Run |
| <a href="tests/system/TC-SYS-I2C-001.md" target="_blank">TC-SYS-I2C-001</a> | System | <a href="SRS.md#hwi-01" target="_blank">HWI-01</a> | Not Run |
| <a href="tests/system/TC-SYS-LED-001.md" target="_blank">TC-SYS-LED-001</a> | System | <a href="SRS.md#ui-02" target="_blank">UI-02</a> | Not Run |
| <a href="tests/system/TC-SYS-PERF-001.md" target="_blank">TC-SYS-PERF-001</a> | System | <a href="SRS.md#perf-01" target="_blank">PERF-01</a> | Not Run |

## Hazard to Requirement Traceability

| Hazard ID | Description | Safety Goal | Requirement | Test |
|-----------|-------------|-------------|-------------|------|
| HZD-001 | Cell overvoltage >4.25V | Prevent overvoltage | <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | <a href="tests/system/TC-SYS-OV-001.md" target="_blank">TC-SYS-OV-001</a> |
| HZD-002 | Cell undervoltage <2.8V | Prevent undervoltage | <a href="SRS.md#req-func-005" target="_blank">REQ-FUNC-005</a> | <a href="tests/system/TC-SYS-UV-001.md" target="_blank">TC-SYS-UV-001</a> |
| HZD-003 | Cell overvoltage >4.35V | Critical overvoltage cutoff | <a href="SRS.md#req-func-004" target="_blank">REQ-FUNC-004</a> | <a href="tests/system/TC-SYS-OV-001.md" target="_blank">TC-SYS-OV-001</a> |
| HZD-004 | Overtemperature >60°C | Prevent overtemperature | <a href="SRS.md#req-func-006" target="_blank">REQ-FUNC-006</a> | <a href="tests/system/TC-SYS-OT-001.md" target="_blank">TC-SYS-OT-001</a> |
| HZD-005 | Overtemperature >80°C (critical) | Critical overtemperature cutoff | <a href="SRS.md#req-func-006">REQ-FUNC-006</a> | <a href="tests/system/TC-SYS-OT-001.md" target="_blank">TC-SYS-OT-001</a> |
| HZD-006 | I2C communication failure | Detect and recover | <a href="SRS.md#hwi-01" target="_blank">HWI-01</a> | <a href="tests/system/TC-SYS-I2C-001.md" target="_blank">TC-SYS-I2C-001</a> |
| HZD-007 | Charge FET fails closed | Monitor FET state | <a href="SRS.md#hwi-03">HWI-03</a> | <a href="tests/system/TC-SYS-FET-001.md" target="_blank">TC-SYS-FET-001</a> |
| HZD-008 | Discharge FET fails closed | Monitor FET state | <a href="SRS.md#hwi-04">HWI-04</a> | <a href="tests/system/TC-SYS-FET-002.md" target="_blank">TC-SYS-FET-002</a> |
| HZD-009 | BQ76920 enters SHIP mode | Verify BQ76920 is awake | <a href="SRS.md#hwi-01">HWI-01</a> | <a href="tests/integration/TC-INT-I2C-001.md" target="_blank">TC-INT-I2C-001</a> |

## Summary

| Test Level | Total Tests | Passed | Failed | Not Run |
|------------|-------------|--------|--------|---------|
| Unit | 6 | 2 | 0 | 4 |
| Integration | 7 | 3 | 0 | 4 |
| System | 11 | 0 | 0 | 11 |
| **Total** | **24** | **5** | **0** | **19** |

## How to Use

- Click any requirement link to open the SRS section in a new tab
- Click any test link to open the test case in a new tab
- Update the test case status when executed
- Update the Status column in the Summary section as tests pass