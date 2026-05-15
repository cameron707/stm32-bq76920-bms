# Test Case: TC-INT-VOLT-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | TC-INT-VOLT-001 |
| Title | I2C + Voltage Read Integration Test |
| Test Level | Integration |
| Priority | High |
| Status | Pass |
| Version | 1.0 |
| Date | 2026-05-15 |
| Author | Cameron Burnett |

---

## Traceability

| Source Artifact | Source ID | Requirement ID | Design Element | Test ID |
|-----------------|-----------|----------------|----------------|---------|
| SRS | REQ-FUNC-001 | REQ-FUNC-001 | `bq76920_read_all_voltages()` | TC-INT-VOLT-001 |

---

## Statement

The STM32 shall successfully read individual cell voltages from the BQ76920 over I2C and convert the raw ADC counts to millivolts with ±5mV accuracy across the operating range (2.5V-4.2V per cell).

## Rationale

Accurate cell voltage monitoring is the foundation of all BMS protection features including overvoltage and undervoltage cutoffs. This integration test validates that the I2C communication works correctly with the BQ76920 and that the driver correctly converts raw ADC counts to voltage readings.

## Acceptance Criteria

| Test | Condition | Expected Result |
|------|-----------|-----------------|
| Cell 1 voltage read | Normal operation | Read returns true, value in 2500-4200 mV range |
| Cell 2 voltage read | Normal operation | Read returns true, value in 2500-4200 mV range |
| Cell 3 voltage read | Normal operation | Read returns true, value in 2500-4200 mV range |
| Cell 4 voltage read | Normal operation | Read returns true, value in 2500-4200 mV range |
| Cell 5 voltage read | Normal operation | Read returns true, value in 2500-4200 mV range |
| Pack voltage read | Normal operation | Read returns true, value approximates sum of cells |
| I2C communication | All transactions | No I2C errors (HAL_OK) |
| Reading stability | Multiple reads | Values stable (±10 mV) |

## Preconditions

| # | Description |
|---|-------------|
| 1 | BQ76920 EVM powered with 18V on J2 (BAT+) and J3 (BAT-) |
| 2 | Orange LED on EVM illuminated (cell simulator active) |
| 3 | STM32 Blue Pill powered via USB |
| 4 | I2C connections: PB6 (SCL) to BQ76920 SCL, PB7 (SDA) to BQ76920 SDA |
| 5 | 4.7kΩ pull-up resistors installed on both SDA and SCL lines to 3.3V |
| 6 | SW1 (BOOT) button on EVM pressed after power-up to wake BQ76920 from SHIP mode |
| 7 | USB serial monitor connected to view printf output |
| 8 | Driver initialized and ADC enabled |

## Test Data

| Data Element | Value | Purpose |
|--------------|-------|---------|
| Cell voltage range | 2500-4200 mV | Normal Li-ion operating range |
| Cell simulator | 18V input | Produces ~3600 mV per cell |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Initialize BQ76920 driver and enable ADC | Driver init succeeds, ADC enabled |
| 2 | Read all cell voltages via `bq76920_read_all_voltages()` | Function returns true |
| 3 | Verify each cell voltage | All cells between 2500-4200 mV |
| 4 | Calculate pack voltage from cell sum | Pack voltage ≈ sum of individual cells |
| 5 | Repeat readings 5 times | Values remain stable (±10 mV) |

## Expected Results

| Test Case | Expected Result | Actual Result |
|-----------|----------------|---------------|
| Cell 1 reads | 2500-4200 mV | 3619-3622 mV ✅ |
| Cell 2 reads | 2500-4200 mV | 3606-3609 mV ✅ |
| Cell 3 reads | 2500-4200 mV | 3611-3613 mV ✅ |
| Cell 4 reads | 2500-4200 mV | 3615-3617 mV ✅ |
| Cell 5 reads | 2500-4200 mV | 3613-3616 mV ✅ |
| Pack voltage | ≈ sum of cells | 18066-18078 mV (5 × ~3613 mV) ✅ |
| I2C errors | None | No errors ✅ |
| Reading stability | ±10 mV | Stable ✅ |

## Post-conditions

| # | Description |
|---|-------------|
| 1 | BQ76920 remains operational |
| 2 | I2C bus idle |
| 3 | Driver cell voltage array updated with latest readings |

## Verification Method

| Test | Method | Description |
|------|--------|-------------|
| Cell voltage readings | Demonstration | Read voltages with TEMP_SEL=0, verify values are plausible (2500-4200 mV) |
| Pack voltage | Analysis | Verify pack voltage equals sum of individual cells |
| I2C communication | Inspection | Verify all I2C transactions return HAL_OK |
| Reading stability | Analysis | Compare consecutive readings, verify stability |

**Overall Method:** Test - Integration test executed on STM32 hardware with BQ76920 EVM. Results observed via USB serial output.

## References

| Document Type | Reference |
|---------------|-----------|
| Requirements | `docs/SRS.md` - REQ-FUNC-001 |
| Design | `docs/SDD.md` - Section 5.3 (Voltage Conversion) |
| BQ76920 Datasheet | Section 7.3.1.1.4 (14-Bit Cell Voltage ADC) |
| Related Test Cases | UT-VOLT-001, TC-SYS-VOLT-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-15 | Cameron Burnett | Pass | Cell 1: 3622 mV (PASS) |
| 2026-05-15 | Cameron Burnett | Pass | Cell 2: 3609 mV (PASS) |
| 2026-05-15 | Cameron Burnett | Pass | Cell 3: 3613 mV (PASS) |
| 2026-05-15 | Cameron Burnett | Pass | Cell 4: 3617 mV (PASS) |
| 2026-05-15 | Cameron Burnett | Pass | Cell 5: 3615 mV (PASS) |
| 2026-05-15 | Cameron Burnett | Pass | Pack: 18078 mV (PASS) |

## Test Output

```text
========================================
BQ76920 BMS Integration Test
========================================
Driver initialized successfully.
ADC Gain: 377 uV/LSB, ADC Offset: 46 mV
ADC enabled.
Waiting for ADC to stabilize...

Cell 1: 3622 mV  Cell 2: 3609 mV  Cell 3: 3613 mV  Cell 4: 3617 mV  Cell 5: 3615 mV
Pack:  18078 mV

Cell 1: 3622 mV  Cell 2: 3609 mV  Cell 3: 3613 mV  Cell 4: 3617 mV  Cell 5: 3616 mV
Pack:  18078 mV

Cell 1: 3622 mV  Cell 2: 3609 mV  Cell 3: 3613 mV  Cell 4: 3617 mV  Cell 5: 3615 mV
Pack:  18078 mV

----------------------------------------
```

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-15 | Cameron Burnett | Initial creation - test passed
