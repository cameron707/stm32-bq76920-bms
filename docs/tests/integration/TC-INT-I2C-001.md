# Test Case: TC-INT-I2C-001

## Document Information

| Field | Value |
|-------|-------|
| Test Case ID | TC-INT-I2C-001 |
| Title | I2C Communication Integration Test |
| Test Level | Integration |
| Priority | High |
| Status | Pass |
| Version | 1.0 |
| Date | 2026-05-14 |
| Author | Cameron Burnett |

---

## Traceability

| Source Artifact | Source ID | Requirement ID | Design Element | Test ID |
|-----------------|-----------|----------------|----------------|---------|
| SRS | HWI-01 | HWI-01 | `MX_I2C1_Init()`, `HAL_I2C_IsDeviceReady()`, `HAL_I2C_Mem_Read()` | TC-INT-I2C-001 |

---

## Statement

The STM32 shall successfully communicate with the BQ76920 over I2C at 100kHz, verify device presence, and read the SYS_STAT register.

## Rationale

I2C communication is the foundation of all BMS monitoring and protection functions. Without reliable I2C, voltage, current, and temperature data cannot be read, and protection FETs cannot be controlled. This test validates the physical I2C connection and basic driver functionality before proceeding to measurement tests.

## Acceptance Criteria

- BQ76920 detected at I2C address 0x08
- `HAL_I2C_IsDeviceReady()` returns HAL_OK
- Successful read of SYS_STAT register (0x00)
- Register value printed via USB printf
- No I2C timeout or bus errors

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

## Test Data

| Data Element | Value | Purpose |
|--------------|-------|---------|
| I2C address | 0x08 (7-bit) | BQ76920 slave address |
| SYS_STAT register | 0x00 | Register to read for communication test |

## Test Steps

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Apply 18V power to BQ76920 EVM (J2/J3) | Orange LED illuminates |
| 2 | Press SW1 (BOOT) button on EVM | BQ76920 exits SHIP mode |
| 3 | Power STM32 Blue Pill and flash firmware | USB serial monitor shows output |
| 4 | `HAL_I2C_IsDeviceReady()` called with address 0x08 | Returns HAL_OK |
| 5 | `HAL_I2C_Mem_Read()` called to read SYS_STAT (0x00) | Returns HAL_OK |
| 6 | SYS_STAT value printed via printf | Value displayed in serial terminal |

## Expected Results

- I2C device detected at address 0x08
- SYS_STAT register read successfully
- printf output shows: "Device found at address 0x08" and "SYS_STAT = 0xXX"
- No I2C timeout or bus errors

## Post-conditions

| # | Description |
|---|-------------|
| 1 | I2C bus remains functional for subsequent tests |
| 2 | BQ76920 remains operational (not in SHIP mode) |
| 3 | STM32 continues normal operation |

## Verification Method

Test

## References

| Document Type | Reference |
|---------------|-----------|
| Requirements | `docs/SRS.md` - HWI-01 |
| Design | `docs/SDD.md` - Section 2.3 (Interface Summary), Section 4.2 (I2C Protocol) |
| Related Test Cases | TC-INT-VOLT-001, TC-INT-CURR-001, TC-INT-TEMP-001 |

## Test Execution Log

| Date | Executed By | Result | Notes |
|------|-------------|--------|-------|
| 2026-05-14 | Cameron Burnett | Pass | BQ76920 detected at address 0x08. SYS_STAT register read successfully. Output: "Device found at address 0x08" and "SYS_STAT = 0x00". USB printf confirmed communication. |

## Change History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-05-14 | Cameron Burnett | Initial creation - test passed |