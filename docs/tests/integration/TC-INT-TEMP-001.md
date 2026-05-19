# Test Case: TC-INT-TEMP-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | TC-INT-TEMP-001 |
| Title | I2C + Temperature Read Integration Test |
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
| SRS | REQ-FUNC-003 | REQ-FUNC-003 | `bq76920_read_temperature()` | TC-INT-TEMP-001 |

---

## Statement

The STM32 shall successfully read temperature data from the BQ76920 over I2C, supporting both external NTC thermistor (TEMP_SEL=1) and internal die temperature (TEMP_SEL=0) modes, with stable readings.

## Rationale

Temperature monitoring is essential for battery safety (overtemperature protection). This integration test validates that:
- I2C communication with the BQ76920 temperature hardware works
- The driver correctly reads raw ADC counts from TS1 register
- The conversion to temperature in 0.1°C units produces stable readings
- Both die temperature and external NTC modes function correctly

**Note on Accuracy:** Absolute temperature accuracy cannot be verified without a temperature-controlled chamber or calibrated reference thermometer. This test verifies **stability** (consistent readings) only, not absolute accuracy. The actual temperature values are reasonable estimates but are not certified.

## Acceptance Criteria

| Test | Source | Condition | Expected Result |
|------|--------|-----------|-----------------|
| Die temperature read | TEMP_SEL = 0 | Normal operation | Read returns true, value in 25-65°C range (sanity check) |
| Die temperature stability | TEMP_SEL = 0 | Consecutive reads | Variation < ±0.5°C between readings |
| External NTC read | TEMP_SEL = 1 | Room temperature | Read returns true, value in 20-35°C range (sanity check) |
| External NTC stability | TEMP_SEL = 1 | Consecutive reads | Variation < ±0.3°C between readings |
| I2C communication | Both modes | All transactions | No I2C errors (HAL_OK) |

**Stability Justification:** Consecutive reads should be very stable (±0.5°C maximum). The external NTC is more stable (±0.3°C) because it measures ambient temperature which changes slowly.

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
| TEMP_SEL = 0 | 0 | Select internal die temperature |
| TEMP_SEL = 1 | 1 | Select external TS1 thermistor |
| Sample count | 10 | Multiple readings to verify stability |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Initialize BQ76920 driver and enable ADC | Driver init succeeds, ADC enabled |
| 2 | Set TEMP_SEL = 0 (die temperature) | I2C write succeeds |
| 3 | Wait 2 seconds for mode switch (per datasheet) | Delay completes |
| 4 | Read temperature 10 times consecutively | Each read returns true |
| 5 | Calculate variation between consecutive reads | Variation < ±0.5°C |
| 6 | Set TEMP_SEL = 1 (external TS1 thermistor) | I2C write succeeds |
| 7 | Wait 2 seconds for mode switch | Delay completes |
| 8 | Read temperature 10 times consecutively | Each read returns true |
| 9 | Calculate variation between consecutive reads | Variation < ±0.3°C |

## Expected Results

| Test Case | Expected Result | Actual Result |
|-----------|----------------|---------------|
| Die temperature reads (sanity) | 25°C - 65°C | 30.7°C ✅ |
| Die temperature stability | Variation < ±0.5°C | 0.0°C (perfect) ✅ |
| External NTC reads (sanity) | 20°C - 35°C | 29.5°C ✅ |
| External NTC stability | Variation < ±0.3°C | 0.0°C (perfect) ✅ |
| I2C errors | None | No errors ✅ |

## Post-conditions

| # | Description |
|---|-------------|
| 1 | BQ76920 remains operational |
| 2 | I2C bus idle |
| 3 | Driver temperature handle updated with latest readings |

## Verification Method

| Test | Method | Description |
|------|--------|-------------|
| Die temperature (sanity) | Demonstration | Read temperature with TEMP_SEL=0, verify values are plausible (25-65°C) |
| Die temperature stability | Analysis | Compare consecutive readings, verify variation < ±0.5°C |
| External NTC (sanity) | Demonstration | Read temperature with TEMP_SEL=1, verify values are plausible (20-35°C) |
| External NTC stability | Analysis | Compare consecutive readings, verify variation < ±0.3°C |
| I2C communication | Inspection | Verify all I2C transactions return HAL_OK |
| Mode switch | Demonstration | Verify temperature changes when switching between modes |

**Overall Method:** Test - Integration test executed on STM32 hardware with BQ76920 EVM. Results observed via USB serial output. Stability is verified analytically; absolute accuracy is not certified.

## References

| Document Type | Reference |
|---------------|-----------|
| Requirements | `docs/SRS.md` - REQ-FUNC-003 |
| Design | `docs/SDD.md` - Section 5.3 (Data Conversion Algorithms) |
| BQ76920 Datasheet | Section 8.3.1.1.4 (External Thermistor), Section 8.3.1.1.5 (Die Temperature Monitor) |
| Related Test Cases | UT-TEMP-001, TC-SYS-TEMP-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-15 | Cameron Burnett | Pass | Die temperature: 28.3°C, perfect stability (superseded) |
| 2026-05-19 | Cameron Burnett | Pass | Die temperature: 30.7°C, perfect stability (PASS) |
| 2026-05-19 | Cameron Burnett | Pass | External NTC: 29.5°C, perfect stability (PASS) |
| 2026-05-19 | Cameron Burnett | Pass | I2C communication: No errors (PASS) |
| 2026-05-19 | Cameron Burnett | Pass | Mode switch: Works correctly (PASS) |

## Test Output

```text
========================================
Driver initialized successfully.
ADC Gain: 377 uV/LSB, ADC Offset: 46 mV
ADC enabled.
Waiting for ADC to stabilize...

========================================
Testing die temperature (TEMP_SEL=0)
Reading die temperature 10 times...
  Read 1: 30.7 C
  Read 2: 30.7 C
  Read 3: 30.7 C
  Read 4: 30.7 C
  Read 5: 30.7 C
  Read 6: 30.7 C
  Read 7: 30.7 C
  Read 8: 30.7 C
  Read 9: 30.7 C
  Read 10: 30.7 C

  Range: 30.7 C - 30.7 C
  Max consecutive variation: 0.00 C
  Stability: PASS (<= 0.5 C)

Testing external NTC temperature (TEMP_SEL=1)
Reading external temperature 10 times...
  Read 1: 29.5 C
  Read 2: 29.5 C
  Read 3: 29.5 C
  Read 4: 29.5 C
  Read 5: 29.5 C
  Read 6: 29.5 C
  Read 7: 29.5 C
  Read 8: 29.5 C
  Read 9: 29.5 C
  Read 10: 29.5 C

  Range: 29.5 C - 29.5 C
  Max consecutive variation: 0.00 C
  Stability: PASS (<= 0.3 C)
========================================
```

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-15 | Cameron Burnett | Initial creation |
| 1.1 | 2026-05-19 | Cameron Burnett | Updated actual results: die 30.7°C, NTC 29.5°C (integer LUT driver) |
