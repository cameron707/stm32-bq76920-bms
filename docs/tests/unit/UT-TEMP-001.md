# Test Case: UT-TEMP-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | UT-TEMP-001 |
| Title | Temperature Conversion Function Unit Test |
| Test Level | Unit |
| Priority | High |
| Status | Pass |
| Version | 1.0 |
| Date | 2026-05-15 |
| Author | Cameron Burnett |

---

## Traceability

| Source Artifact | Source ID | Requirement ID | Design Element | Test ID |
|-----------------|-----------|----------------|----------------|---------|
| SRS | REQ-FUNC-003 | REQ-FUNC-003 | `bq76920_read_temperature()` | UT-TEMP-001 |

---

## Statement

The `bq76920_read_temperature()` function shall convert raw ADC counts from the BQ76920 TS1 register to temperature in 0.1°C units, supporting both external NTC thermistor (TEMP_SEL=1) and internal die temperature (TEMP_SEL=0) modes.

## Rationale

Accurate temperature monitoring is essential for battery safety and protection features. This unit test verifies the conversion formulas in isolation on the host PC before hardware integration, ensuring both external thermistor and die temperature calculations are mathematically correct.

## Acceptance Criteria

- External NTC conversion: 4072 counts → 27.6°C (±0.5°C tolerance)
- Die temperature at V25 calibration: 3141 counts → 25.0°C (±0.5°C tolerance)
- Die temperature at 3400 counts: 3400 counts → 1.3°C (±0.5°C tolerance)
- All test cases pass within specified tolerance

**Tolerance Justification:** ±0.5°C accounts for floating-point to integer rounding, integer math precision, and GAIN value variations (382 nominal vs actual factory-calibrated value).

## Preconditions

| # | Description |
|---|-------------|
| 1 | Test compiled on host PC (not on STM32 hardware) |
| 2 | Conversion formulas extracted from `bq76920.c` |
| 3 | Math library available (for log() and pow()) |
| 4 | Test vectors from datasheet or known theoretical values |

## Test Data

| Data Element | Value | Purpose |
|--------------|-------|---------|
| External NTC raw counts | 4072 | Test external thermistor conversion |
| Die temperature raw counts (V25) | 3141 | Test die temperature at calibration point |
| Die temperature raw counts | 3400 | Test die temperature at typical operating point |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Compile `ut_temp_001.c` on host PC with math library | Compilation succeeds with no errors |
| 2 | Run `./ut_temp_001.exe` | Program executes without crashes |
| 3 | Verify external NTC conversion | 4072 counts → 27.6°C ±0.5°C |
| 4 | Verify die temperature at V25 | 3141 counts → 25.0°C ±0.5°C |
| 5 | Verify die temperature at 3400 counts | 3400 counts → 1.3°C ±0.5°C |

## Expected Results

All test cases pass within the specified ±0.5°C tolerance:
- External NTC: 27.6°C (0.0°C difference from expected)
- Die V25: 25.2°C (0.2°C difference from expected)
- Die 3400: 1.6°C (0.3°C difference from expected)

The small differences (0.2-0.3°C) are due to:
- Use of 382 µV/LSB nominal GAIN vs actual factory-calibrated GAIN (377 µV/LSB)
- Integer division rounding in the conversion function
- These differences are mathematically expected and acceptable for a unit test

## Post-conditions

| # | Description |
|---|-------------|
| 1 | No memory leaks during test execution |
| 2 | Test output saved to `test/ut_temp_001_output.txt` for documentation |
| 3 | All test resources released |

## Verification Method

| Test | Method | Description |
|------|--------|-------------|
| External NTC conversion | Analysis | Mathematical verification of Steinhart-Hart equation using known input/output pairs |
| Die temperature V25 | Analysis | Mathematical verification of datasheet calibration formula |
| Die temperature 3400 | Analysis | Mathematical verification of linear temperature coefficient |

**Overall Method:** Test - Unit test executed on host PC with compiled C code. Results compared against expected values calculated from datasheet formulas.

## References

| Document Type | Reference |
|---------------|-----------|
| Requirements | `docs/SRS.md` - REQ-FUNC-003 |
| Design | `docs/SDD.md` - Section 5.3 (Data Conversion Algorithms) |
| BQ76920 Datasheet | Section 8.3.1.1.4 (External Thermistor), Section 8.3.1.1.5 (Die Temperature Monitor) |
| Related Test Cases | TC-INT-TEMP-001, TC-SYS-TEMP-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-15 | Cameron Burnett | Pass | External NTC: 27.6°C |
| 2026-05-15 | Cameron Burnett | Pass | Die V25: 25.2°C (within 0.5°C) |
| 2026-05-15 | Cameron Burnett | Pass | Die 3400: 1.6°C (within 0.5°C) |

## Test Output

```text
----------------------------------------
UT-TEMP-001: Temperature Conversion Unit Test
----------------------------------------

Test 1: External NTC conversion
--------------------------------
  Raw: 4072 -> Result: 27.6C (Expected: 27.6C)
  Result: PASS

Test 2: Die temperature at V25 (1.200V)
----------------------------------------
  Raw: 3141 -> Result: 25.2C (Expected: 25.0C)
  Result: PASS (within tolerance)

Test 3: Die temperature at 3400 counts
-----------------------------------------------
  Raw: 3400 -> Result: 1.6C
  Result: PASS (within tolerance)

----------------------------------------
Test Summary: 3 passed, 0 failed
----------------------------------------
```

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-15 | Cameron Burnett | Initial creation - test passed