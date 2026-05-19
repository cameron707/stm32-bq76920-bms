# Hazard Analysis and Risk Assessment (HARA)

## STM32 BQ76920 Battery Management System

> **Note:** This HARA follows ISO 26262-3 methodology for hazard identification and ASIL determination. ASIL is calculated per Table 4 based on Severity (S), Exposure (E), and Controllability (C). Where a QM classification is assigned, the rationale explicitly documents why no additional safety requirement is needed — typically because an independent hardware protection layer remains active regardless of software or communication state.

---

## Hazard Identification and Classification

| ID | Scenario | Operational Situation | Sev | Exp | Cont | ASIL | Safety Goal | Notes |
|----|----------|----------------------|-----|-----|------|------|-------------|-------|
| HZD-001 | Cell overvoltage during charging (>4.25V) | Battery charging, any condition | S2 | E3 | C2 | ASIL B | Prevent any cell voltage from exceeding 4.25V | BQ76920 has hardware overvoltage protection. Software adds redundancy. |
| HZD-002 | Cell undervoltage during discharging (<2.8V) | Battery discharging, normal operation | S1 | E3 | C1 | ASIL A | Prevent any cell voltage from falling below 2.8V | Permanent cell damage possible but not immediate safety risk. |
| HZD-003 | Cell overvoltage during charging (>4.35V - critical) | Battery charging, high current | S3 | E2 | C3 | ASIL C | Prevent any cell voltage from exceeding 4.35V | Immediate fire/explosion risk. Need redundant protection. |
| HZD-004 | Overtemperature during operation (>60°C) | Continuous operation, high load | S2 | E3 | C2 | ASIL B | Prevent battery temperature from exceeding 60°C | Thermal runaway threshold typically 80°C. 60°C is warning zone. |
| HZD-005 | Overtemperature during operation (>80°C - critical) | Continuous operation, high load | S3 | E2 | C3 | ASIL C | Prevent battery temperature from exceeding 80°C | Thermal runaway imminent. Need immediate cutoff. |
| HZD-006 | I2C communication failure between STM32 and BQ76920 | Normal operation, any condition | S1 | E3 | C2 | QM | Detect I2C failure and log fault within 100ms | QM justified: BQ76920 hardware protection (OV, UV, OT, SCD) operates via independent analog comparators and remains active regardless of I2C state. STM32 software layer provides additional monitoring only. Hardware layer alone is sufficient to prevent hazardous outcomes. |
| HZD-007 | Charge FET fails closed (stuck ON) | Charging operation | S3 | E1 | C3 | ASIL A | Provide independent hardware monitoring of FET state | Hard to detect. Consider watchdog or secondary cutoff. E1 reduces exposure, lowering ASIL. |
| HZD-008 | Discharge FET fails closed (stuck ON) | Discharging operation | S2 | E1 | C3 | ASIL A | Provide independent hardware monitoring of FET state | Less critical than charge FET failure. E1 reduces exposure. |
| HZD-009 | BQ76920 enters SHIP mode (low power) | Normal operation, idle state | S1 | E2 | C2 | QM | Periodically verify BQ76920 is awake and communicating | QM justified: BQ76920 hardware protection remains active even approaching SHIP mode transition. Software shall periodically ping BQ76920 to confirm operational state. If SHIP mode is entered inadvertently, hardware OV/UV/OT protection is lost — software detection and recovery mitigates this within the QM quality management framework. |

---

## Severity, Exposure, Controllability Reference

### Severity (S0-S3)

| Level | Meaning | BMS Example |
|-------|---------|-------------|
| S0 | No injuries | Cosmetic battery damage |
| S1 | Minor injuries | Battery swelling, smoke |
| S2 | Severe injuries | Fire, minor burns |
| S3 | Life-threatening or fatal | Explosion, major fire |

### Exposure (E0-E4)

| Level | Meaning | BMS Example |
|-------|---------|-------------|
| E0 | Incredibly unlikely | Multiple simultaneous failures |
| E1 | Very low probability | Failure once per device lifetime |
| E2 | Low probability | Failure possible once per user |
| E3 | Medium probability | Failure possible during each use cycle |
| E4 | High probability | Failure likely during each charge |

### Controllability (C0-C3)

| Level | Meaning | BMS Example |
|-------|---------|-------------|
| C0 | Controllable by user | User can unplug battery |
| C1 | Simple to control | User can press reset button |
| C2 | Difficult to control | User may not notice issue |
| C3 | Uncontrollable | Automatic cutoff required |

---

## ASIL Determination

ASIL (Automotive Safety Integrity Level) is determined per ISO 26262-3 Table 4 based on the combination of Severity (S), Exposure (E), and Controllability (C).

| Hazard ID | Severity (S) | Exposure (E) | Controllability (C) | ASIL | Rationale |
|-----------|--------------|--------------|---------------------|------|-----------|
| HZD-001 | S2 | E3 | C2 | ASIL B | S2 (severe injury) + E3 (medium probability) + C2 (difficult to control) → ASIL B per ISO 26262-3 Table 4 |
| HZD-002 | S1 | E3 | C1 | ASIL A | S1 (minor injury) + E3 (medium probability) + C1 (simple to control) → ASIL A |
| HZD-003 | S3 | E2 | C3 | ASIL C | S3 (life-threatening) + E2 (low probability) + C3 (uncontrollable) → ASIL C |
| HZD-004 | S2 | E3 | C2 | ASIL B | S2 (severe injury) + E3 (medium probability) + C2 (difficult to control) → ASIL B |
| HZD-005 | S3 | E2 | C3 | ASIL C | S3 (life-threatening) + E2 (low probability) + C3 (uncontrollable) → ASIL C |
| HZD-006 | S1 | E3 | C2 | QM | S1 (minor injury) → QM per ISO 26262-3 Table 4. Further justified by independent hardware protection: BQ76920 analog comparators monitor OV/UV/OT/SCD continuously and will trip FETs regardless of I2C state. Software layer is supplementary. |
| HZD-007 | S3 | E1 | C3 | ASIL A | S3 (life-threatening) + E1 (very low probability) + C3 (uncontrollable) → ASIL A (exposure reduces ASIL) |
| HZD-008 | S2 | E1 | C3 | ASIL A | S2 (severe injury) + E1 (very low probability) + C3 (uncontrollable) → ASIL A |
| HZD-009 | S1 | E2 | C2 | QM | S1 (minor injury) → QM per ISO 26262-3 Table 4. Software shall implement periodic keep-alive polling to detect SHIP mode entry. Recovery action: re-initialise BQ76920 via I2C boot sequence. |

### ASIL Levels Reference

| Rating | Meaning | BMS Application |
|--------|---------|-----------------|
| QM | Quality Management (no safety requirement) | I2C communication monitoring, SHIP mode detection |
| ASIL A | Minor injury possible | Undervoltage protection, FET monitoring |
| ASIL B | Severe injury possible | Overvoltage protection, overtemperature warning |
| ASIL C | Life-threatening possible | Critical overvoltage cutoff, critical overtemperature cutoff |
| ASIL D | Fatal possible | Multiple redundant cutoffs (not assigned in this analysis) |

---

## Traceability to Requirements

| Hazard ID | Safety Goal | SRS Requirement ID | Verification |
|-----------|-------------|--------------------|--------------|
| HZD-001 | Prevent cell voltage exceeding 4.25V | <a href="SRS.md#req-func-004">REQ-FUNC-004</a> | TC-SYS-OV-001 |
| HZD-002 | Prevent cell voltage falling below 2.8V | <a href="SRS.md#req-func-005">REQ-FUNC-005</a> | TC-SYS-UV-001 |
| HZD-003 | Prevent cell voltage exceeding 4.35V | <a href="SRS.md#req-func-004">REQ-FUNC-004</a> | TC-SYS-OV-001 |
| HZD-004 | Prevent battery temperature exceeding 60°C | <a href="SRS.md#req-func-006">REQ-FUNC-006</a> | TC-SYS-OT-001 |
| HZD-005 | Prevent battery temperature exceeding 80°C | <a href="SRS.md#req-func-006">REQ-FUNC-006</a> | TC-SYS-OT-001 |
| HZD-006 | Detect I2C failure, log fault, rely on hardware protection | <a href="SRS.md#hwi-01">HWI-01</a> | TC-SYS-I2C-001 |
| HZD-007 | Monitor charge FET state | <a href="SRS.md#hwi-03">HWI-03</a> | TC-SYS-FET-001 |
| HZD-008 | Monitor discharge FET state | <a href="SRS.md#hwi-04">HWI-04</a> | TC-SYS-FET-002 |
| HZD-009 | Verify BQ76920 is awake | <a href="SRS.md#hwi-01">HWI-01</a> | TC-INT-I2C-001 |

---

## Summary

| ASIL Level | Hazards | Safety Goals |
|------------|---------|--------------|
| QM | HZD-006, HZD-009 | 2 |
| ASIL A | HZD-002, HZD-007, HZD-008 | 3 |
| ASIL B | HZD-001, HZD-004 | 2 |
| ASIL C | HZD-003, HZD-005 | 2 |
| ASIL D | None | 0 |

---

*End of Hazard Analysis and Risk Assessment*