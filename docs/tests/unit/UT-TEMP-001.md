# Test Case: UT-TEMP-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | UT-TEMP-001 |
| Title | Temperature Conversion Function Unit Test |
| Test Level | Unit |
| Priority | High |
| Status | Pass |
| Version | 2.0 |
| Date | 2026-05-19 |
| Author | Cameron Burnett |

---

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-15 | Cameron Burnett | Initial — Steinhart-Hart floating-point implementation |
| 2.0 | 2026-05-19 | Cameron Burnett | Rewritten for integer LUT approach; added boundary and interpolation tests; removed math.h dependency |

---

## Traceability

| Source Artifact | Source ID | Requirement ID | Design Element | Test ID |
|-----------------|-----------|----------------|----------------|---------|
| SRS | REQ-FUNC-003 | REQ-FUNC-003 | `bq76920_read_temperature()` | UT-TEMP-001 |

---

## Statement

The `bq76920_read_temperature()` function shall convert raw ADC counts from the BQ76920 TS1 register to temperature in 0.1°C units, supporting both external NTC thermistor (TEMP_SEL=1) and internal die temperature (TEMP_SEL=0) modes, using integer arithmetic only (no floating point, no math library).

## Rationale

Accurate temperature monitoring is essential for battery safety (overtemperature protection). This unit test verifies both conversion algorithms in isolation on the host PC before hardware integration.

**v2.0 rationale:** The original Steinhart-Hart floating-point NTC calculation was replaced with an integer LUT + linear interpolation approach (MISRA compliance, no FPU on STM32F103). The die temperature formula was also converted to integer fixed-point. This version tests both new algorithms. The LUT was generated from the same Steinhart-Hart coefficients, so results are consistent within ±1°C across the full operating range.

## Test Scope

This is a **unit test** that verifies only the mathematical conversion functions. It does **not** test:
- I2C communication with the BQ76920
- Reading ADC GAIN and OFFSET from device EEPROM (gain is supplied directly as a test parameter)
- Absolute hardware accuracy

Those factors are verified in TC-INT-TEMP-001 and TC-SYS-TEMP-001.

## Acceptance Criteria

### NTC Thermistor Tests (TEMP_SEL=1)

| ID | Input | Expected | Tolerance | Justification |
|----|-------|----------|-----------|---------------|
| T1 | rts=10220 Ω | 24.5°C | ±0.0°C | Direct LUT entry — must be exact |
| T2 | rts=9170 Ω | 27.0°C | ±0.0°C | Direct LUT entry — must be exact |
| T3 | rts=8710 Ω | 28.2°C | ±0.2°C | Interpolated — integer rounding |
| T4 | raw=4072, gain=377 | 28.2°C | ±1.0°C | LUT vs Steinhart-Hart ref (27.6°C) within 0.6°C |
| T5 | rts=150000 Ω | -20.5°C | ±0.0°C | Above range — must clamp exactly |
| T6 | rts=800 Ω | 80.1°C | ±0.0°C | Below range — must clamp exactly |

### Die Temperature Tests (TEMP_SEL=0)

| ID | Input | Expected | Tolerance | Justification |
|----|-------|----------|-----------|---------------|
| T7 | raw=3183, gain=377 | 25.0°C | ±0.3°C | Integer div: 3183×377/1000=1199mV (not exactly 1200) |
| T8 | raw=3120, gain=377 | 30.7°C | ±0.0°C | Derived from actual hardware output — exact |
| T9 | raw=3050, gain=377 | 37.1°C | ±0.0°C | Analytically verified — exact |

## Preconditions

| # | Description |
|---|-------------|
| 1 | Test compiled on host PC (not on STM32 hardware) |
| 2 | LUT and conversion functions copied verbatim from `bq76920.c` — if driver changes, update test |
| 3 | Standard C compiler with `<stdint.h>` and `<stdbool.h>` — no math library required |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Compile: `gcc ut_temp_001.c -o ut_temp_001` | No errors or warnings |
| 2 | Run: `./ut_temp_001` | Executes without crashes |
| 3–8 | NTC LUT tests T1–T6 | All pass within tolerance |
| 9–11 | Die temperature tests T7–T9 | All pass within tolerance |

## Expected Results

| Test | Result | Expected | Diff | Pass? |
|------|--------|----------|------|-------|
| T1: rts=10220 | 24.5°C | 24.5°C | 0.0°C | Yes |
| T2: rts=9170 | 27.0°C | 27.0°C | 0.0°C | Yes |
| T3: rts=8710 | 28.2°C | 28.2°C | 0.0°C | Yes (±0.2°C) |
| T4: raw=4072, gain=377 | 28.2°C | 28.2°C | 0.0°C | Yes (±1.0°C) |
| T5: rts=150000 | -20.5°C | -20.5°C | 0.0°C | Yes |
| T6: rts=800 | 80.1°C | 80.1°C | 0.0°C | Yes |
| T7: raw=3183, gain=377 | 25.2°C | 25.0°C | 0.2°C | Yes (±0.3°C) |
| T8: raw=3120, gain=377 | 30.7°C | 30.7°C | 0.0°C | Yes |
| T9: raw=3050, gain=377 | 37.1°C | 37.1°C | 0.0°C | Yes |

## Post-conditions

| # | Description |
|---|-------------|
| 1 | Test output saved to `tests/unit/ut_temp_001_output.txt` |

## References

| Document | Reference |
|----------|-----------|
| Requirements | `docs/SRS.md` — REQ-FUNC-003 |
| Design | `docs/SDD.md` — Section 5.3 (Temperature Conversion) |
| BQ76920 Datasheet | Section 8.3.1.1.4 (External Thermistor), 8.3.1.1.5 (Die Temperature) |
| Related Tests | TC-INT-TEMP-001, TC-SYS-TEMP-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-15 | Cameron Burnett | Pass (v1.0) | Steinhart-Hart floating-point — superseded by v2.0 |
| 2026-05-19 | Cameron Burnett | Pass (v2.0) | Integer LUT — 9/9 passed, 0 failed |

## Test Output

```text
==================================================
UT-TEMP-001: Temperature Conversion Unit Test
==================================================

--- NTC Thermistor Tests (integer LUT + interpolation) ---

  T1: rts=10220 ohm -> exact LUT match (24.5 C)
    Result: 24.5 C  Expected: 24.5 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

  T2: rts=9170 ohm -> exact LUT match (27.0 C)
    Result: 27.0 C  Expected: 27.0 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

  T3: rts=8710 ohm -> interpolated (28.2 C)
    Result: 28.2 C  Expected: 28.2 C  Diff: 0.0 C  Tol: +/-0.2 C
    PASS

  T4: raw=4072, gain=377 -> hardware vector (LUT: 28.2C, S-H ref: 27.6C)
    Result: 28.2 C  Expected: 28.2 C  Diff: 0.0 C  Tol: +/-1.0 C
    PASS

  T5: rts=150000 ohm -> above range, clamp to -20.5 C
    Result: -20.5 C  Expected: -20.5 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

  T6: rts=800 ohm -> below range, clamp to 80.1 C
    Result: 80.1 C  Expected: 80.1 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

--- Die Temperature Tests (integer fixed-point) ---

  T7: raw=3183, gain=377 -> near V25 calibration (~25.0 C)
    Result: 25.2 C  Expected: 25.0 C  Diff: 0.2 C  Tol: +/-0.3 C
    PASS

  T8: raw=3120, gain=377 -> actual hardware reading (30.7 C)
    Result: 30.7 C  Expected: 30.7 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

  T9: raw=3050, gain=377 -> colder die (37.1 C)
    Result: 37.1 C  Expected: 37.1 C  Diff: 0.0 C  Tol: +/-0.0 C
    PASS

==================================================
Test Summary: 9 passed, 0 failed
==================================================
```
