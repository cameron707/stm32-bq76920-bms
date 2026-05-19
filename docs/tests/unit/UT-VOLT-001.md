# Test Case: UT-VOLT-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | UT-VOLT-001 |
| Title | Voltage Conversion Function Unit Test |
| Test Level | Unit |
| Priority | High |
| Status | Pass |
| Version | 1.1 |
| Date | 2026-05-19 |
| Author | Cameron Burnett |

---

## Traceability

| Source Artifact | Source ID | Requirement ID | Design Element | Test ID |
|-----------------|-----------|----------------|----------------|---------|
| SRS | REQ-FUNC-001 | REQ-FUNC-001 | `counts_to_millivolts()` | UT-VOLT-001 |

---

## Statement

The `counts_to_millivolts()` function shall convert raw ADC counts from the BQ76920 to millivolts using the formula `mV = (counts × GAIN) / 1000 + OFFSET`, where GAIN is in µV/LSB and OFFSET is in mV.

## Rationale

Accurate voltage conversion is the foundation of all BMS protection features including overvoltage and undervoltage cutoffs. This unit test verifies the conversion formula in isolation on the host PC before hardware integration, ensuring the mathematical conversion is correct regardless of hardware variations.

## Test Scope

This is a **unit test** that verifies only the mathematical conversion function. It does **not** test:
- I2C communication with the BQ76920
- Reading GAIN and OFFSET from EEPROM
- Hardware accuracy (ADC, voltage dividers)

Those factors are verified in integration tests (TC-INT-VOLT-001) and system tests (TC-SYS-VOLT-001).

## Acceptance Criteria

| Test | Input (counts) | GAIN (µV/LSB) | OFFSET (mV) | Expected Output (mV) | Tolerance | Justification |
|------|----------------|---------------|-------------|---------------------|-----------|---------------|
| Zero input | 0 | 382 | 0 | 0 | ±0 mV | Mathematical identity |
| Nominal 1000 mV | 2621 | 382 | 0 | 1000 | ±5 mV | Integer math rounding |
| Nominal 2000 mV | 5242 | 382 | 0 | 2000 | ±5 mV | Integer math rounding |
| Nominal 3000 mV | 7863 | 382 | 0 | 3000 | ±5 mV | Integer math rounding |
| Nominal 4000 mV | 10484 | 382 | 0 | 4000 | ±5 mV | Integer math rounding |
| Extended range | 13107 | 382 | 0 | 5000 | ±10 mV | Exceeds Li-ion range (testing only) |
| Hardware Cell 1 | 9459 | 377 | 46 | 3612 | ±1 mV | Session 1 chip calibration |
| Hardware Cell 2 | 9423 | 377 | 46 | 3598 | ±1 mV | Session 1 chip calibration |
| Hardware Cell 3 | 9434 | 377 | 46 | 3602 | ±1 mV | Session 1 chip calibration |
| Hardware Cell 1 S2 | 9446 | 377 | 46 | 3607 | ±1 mV | Session 2 chip calibration |
| Hardware Cell 2 S2 | 9410 | 377 | 46 | 3593 | ±1 mV | Session 2 chip calibration |
| Hardware Cell 3 S2 | 9422 | 377 | 46 | 3598 | ±1 mV | Session 2 chip calibration |
| Hardware Cell 4 S2 | 9433 | 377 | 46 | 3602 | ±1 mV | Session 2 chip calibration |
| Hardware Cell 5 S2 | 9426 | 377 | 46 | 3599 | ±1 mV | Session 2 chip calibration |

**Tolerance Justification:**
- **±5 mV for nominal tests:** Accounts for integer math rounding from the formula `(counts × GAIN) / 1000`. The maximum rounding error across the 14-bit ADC range is approximately ±6 mV.
- **±10 mV for extended range:** The 5000 mV test exceeds normal Li-ion operating range (2500-4200 mV). A wider tolerance is acceptable as this is outside the BMS normal operating region.
- **±1 mV for hardware tests:** Uses the actual factory-calibrated GAIN and OFFSET from the specific chip. The conversion should be exact.

## Preconditions

| # | Description |
|---|-------------|
| 1 | Test compiled on host PC (not on STM32 hardware) |
| 2 | Conversion formula extracted from `bq76920.c` |
| 3 | C compiler with integer math support |
| 4 | Test vectors derived from datasheet and actual hardware readings |

## Test Data

| Data Element | Value | Purpose |
|--------------|-------|---------|
| Zero input counts | 0 | Verify zero output |
| Nominal counts (1000 mV) | 2621 | Verify conversion at 1000 mV |
| Nominal counts (2000 mV) | 5242 | Verify conversion at 2000 mV |
| Nominal counts (3000 mV) | 7863 | Verify conversion at 3000 mV |
| Nominal counts (4000 mV) | 10484 | Verify conversion at 4000 mV |
| Extended range counts | 13107 | Verify conversion at 5000 mV (testing only) |
| Hardware Cell 1 counts | 9459 | Verify with actual chip GAIN=377, OFFSET=46 |
| Hardware Cell 2 counts | 9423 | Verify with actual chip GAIN=377, OFFSET=46 |
| Hardware Cell 3 counts | 9434 | Verify with actual chip GAIN=377, OFFSET=46 |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Compile `ut_volt_001.c` on host PC | Compilation succeeds with no errors |
| 2 | Run `./ut_volt_001.exe` | Program executes without crashes |
| 3 | Verify zero input | 0 counts → 0 mV |
| 4 | Verify nominal conversions | Each nominal test passes within ±5 mV tolerance |
| 5 | Verify extended range | 5000 mV test passes within ±10 mV tolerance |
| 6 | Verify hardware-calibrated conversions | 9459, 9423, 9434 counts produce exact expected mV values with GAIN=377, OFFSET=46 |

## Expected Results

All test cases pass within specified tolerances:

| Test | Actual Result | Expected | Difference | Within Tolerance? |
|------|---------------|----------|------------|-------------------|
| Zero input | 0 mV | 0 mV | 0 mV | Yes |
| 2621 counts | 1001 mV | 1000 mV | +1 mV | Yes (±5 mV) |
| 5242 counts | 2002 mV | 2000 mV | +2 mV | Yes (±5 mV) |
| 7863 counts | 3003 mV | 3000 mV | +3 mV | Yes (±5 mV) |
| 10484 counts | 4004 mV | 4000 mV | +4 mV | Yes (±5 mV) |
| 13107 counts | 5006 mV | 5000 mV | +6 mV | Yes (±10 mV) |
| Cell 1 (9459) | 3612 mV | 3612 mV | 0 mV | Yes (±1 mV) |
| Cell 2 (9423) | 3598 mV | 3598 mV | 0 mV | Yes (±1 mV) |
| Cell 3 (9434) | 3602 mV | 3602 mV | 0 mV | Yes (±1 mV) |

The small differences in nominal tests (1-4 mV) are due to integer math rounding and are within the specified tolerance. The hardware-calibrated tests match exactly.

## Post-conditions

| # | Description |
|---|-------------|
| 1 | No memory leaks during test execution |
| 2 | Test output saved to `test/ut_volt_001_output.txt` for documentation |
| 3 | All test resources released |

## Verification Method

| Test | Method | Description |
|------|--------|-------------|
| Zero input | Analysis | Mathematical verification of identity |
| Nominal conversions | Analysis | Mathematical verification with integer math rounding tolerance |
| Extended range | Analysis | Mathematical verification at upper ADC range |
| Hardware-calibrated | Analysis | Verification with actual chip calibration values |

**Overall Method:** Test - Unit test executed on host PC with compiled C code. Results compared against expected values calculated from the conversion formula.

## References

| Document Type | Reference |
|---------------|-----------|
| Requirements | `docs/SRS.md` - REQ-FUNC-001 |
| Design | `docs/SDD.md` - Section 5.3 (Voltage Conversion) |
| BQ76920 Datasheet | Section 7.3.1.1.4 (14-Bit Cell Voltage ADC) |
| Related Test Cases | TC-INT-VOLT-001, TC-SYS-VOLT-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-15 | Cameron Burnett | Pass | Tests 1–9: 9/9 passed (session 1 hardware vectors, 3 cells) |
| 2026-05-19 | Cameron Burnett | Pass | Tests 10–14 added: 14/14 passed (session 2 — all 5 cells) |

## Test Output

```text
--------------------------------------------------
UT-VOLT-001: Voltage Conversion Unit Test
--------------------------------------------------

Test 1: Zero input test
  Input: counts=    0, gain=382 uV/LSB, offset=   0 mV
  Result:    0 mV, Expected:    0 mV, Diff: 0 mV
  Result: PASS

Test 2: Nominal: 2621 counts -> 1000 mV (+-5 mV tolerance for integer math)
  Input: counts= 2621, gain=382 uV/LSB, offset=   0 mV
  Result: 1001 mV, Expected: 1000 mV, Diff: 1 mV
  Result: PASS

Test 3: Nominal: 5242 counts -> 2000 mV (+-5 mV tolerance)
  Input: counts= 5242, gain=382 uV/LSB, offset=   0 mV
  Result: 2002 mV, Expected: 2000 mV, Diff: 2 mV
  Result: PASS

Test 4: Nominal: 7863 counts -> 3000 mV (+-5 mV tolerance)
  Input: counts= 7863, gain=382 uV/LSB, offset=   0 mV
  Result: 3003 mV, Expected: 3000 mV, Diff: 3 mV
  Result: PASS

Test 5: Nominal: 10484 counts -> 4000 mV (+-5 mV tolerance)
  Input: counts=10484, gain=382 uV/LSB, offset=   0 mV
  Result: 4004 mV, Expected: 4000 mV, Diff: 4 mV
  Result: PASS

Test 6: Nominal: 13107 counts -> 5000 mV (+-10 mV tolerance, exceeds normal Li-ion range)
  Input: counts=13107, gain=382 uV/LSB, offset=   0 mV
  Result: 5006 mV, Expected: 5000 mV, Diff: 6 mV
  Result: PASS

Test 7: Actual hardware: Cell 1 (9459 counts -> 3612 mV)
  Input: counts= 9459, gain=377 uV/LSB, offset=  46 mV
  Result: 3612 mV, Expected: 3612 mV, Diff: 0 mV
  Result: PASS

Test 8: Actual hardware: Cell 2 (9423 counts -> 3598 mV)
  Input: counts= 9423, gain=377 uV/LSB, offset=  46 mV
  Result: 3598 mV, Expected: 3598 mV, Diff: 0 mV
  Result: PASS

Test 9: Actual hardware: Cell 3 (9434 counts -> 3602 mV)
  Input: counts= 9434, gain=377 uV/LSB, offset=  46 mV
  Result: 3602 mV, Expected: 3602 mV, Diff: 0 mV
  Result: PASS

Test 10: Actual hardware: Cell 1 session 2 (9446 counts -> 3607 mV)
  Input: counts= 9446, gain=377 uV/LSB, offset=  46 mV
  Result: 3607 mV, Expected: 3607 mV, Diff: 0 mV
  Result: PASS

Test 11: Actual hardware: Cell 2 session 2 (9410 counts -> 3593 mV)
  Input: counts= 9410, gain=377 uV/LSB, offset=  46 mV
  Result: 3593 mV, Expected: 3593 mV, Diff: 0 mV
  Result: PASS

Test 12: Actual hardware: Cell 3 session 2 (9422 counts -> 3598 mV)
  Input: counts= 9422, gain=377 uV/LSB, offset=  46 mV
  Result: 3598 mV, Expected: 3598 mV, Diff: 0 mV
  Result: PASS

Test 13: Actual hardware: Cell 4 session 2 (9433 counts -> 3602 mV)
  Input: counts= 9433, gain=377 uV/LSB, offset=  46 mV
  Result: 3602 mV, Expected: 3602 mV, Diff: 0 mV
  Result: PASS

Test 14: Actual hardware: Cell 5 session 2 (9426 counts -> 3599 mV)
  Input: counts= 9426, gain=377 uV/LSB, offset=  46 mV
  Result: 3599 mV, Expected: 3599 mV, Diff: 0 mV
  Result: PASS

--------------------------------------------------
Test Summary: 14 passed, 0 failed
--------------------------------------------------
```

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-15 | Cameron Burnett | Initial creation — 9 test vectors, 3 hardware cells |
| 1.1 | 2026-05-19 | Cameron Burnett | Added 5 session 2 hardware vectors (all 5 cells); fixed %ld format warning |