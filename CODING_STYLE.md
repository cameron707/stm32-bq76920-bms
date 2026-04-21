# STM32 BMS Coding Standards

## MISRA C:2012 & CERT C Embedded Subset

| Project | STM32 BQ76920 Battery Management System |
|---------|------------------------------------------|
| Version | 1.0 |
| Language | C11 (GNU Embedded Extensions) |
| Target | STM32F103C8T6 (ARM Cortex-M3) |
| Standards | MISRA C:2012 (AMD 3), CERT C (Embedded Subset), STM32 HAL |

---

## 1. Guiding Principles

- Safety First – Undefined behavior is unacceptable in a BMS.
- Readability over cleverness – Write for the next engineer debugging at 3 AM.
- Const Correctness – Immutable data must be marked const.
- Single Exit Point – Functions should have one clear return path (MISRA Rule 15.5).

---

## 2. File & Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | snake_case | bq76920_driver.c |
| Public Functions | module_action_object() | bq76920_read_voltage() |
| Static Functions | action_object() | calculate_crc() |
| Variables | snake_case | cell_voltage_raw |
| Global Variables | g_snake_case | g_i2c_handle |
| Constants | kSnakeCase | kMaxCellVoltageMv |
| Macros | UPPER_SNAKE_CASE | BQ76920_I2C_TIMEOUT_MS |
| Typedefs | snake_case_t | bq76920_handle_t |

---

## 3. MISRA C:2012 Core Rules

### Rule 15.5: Single Exit Point

[GOOD EXAMPLE]
bool bq76920_write_reg(const handle_t *h, uint8_t reg, uint8_t val) {
    bool success = false;
    if (h != NULL) {
        if (i2c_write(h->addr, reg, val)) {
            success = true;
        }
    }
    return success;
}

[BAD EXAMPLE]
bool bad_write(handle_t *h, uint8_t reg, uint8_t val) {
    if (h == NULL) return false;
    return i2c_write(h->addr, reg, val);
}

### Rule 8.13: Const Correctness

[GOOD EXAMPLE]
bool read_voltage(const handle_t *h, uint16_t *result);

[BAD EXAMPLE]
bool read_voltage(handle_t *h, uint16_t *result);

### Rule 10.1: No Implicit Conversions

[GOOD EXAMPLE]
uint16_t voltage = ((uint16_t)raw_high << 8) | raw_low;

[BAD EXAMPLE]
uint16_t voltage = (raw_high << 8) | raw_low;

### Rule 21.10: No Dynamic Allocation

[GOOD EXAMPLE]
uint16_t cell_voltages[5];

[BAD EXAMPLE]
uint16_t *voltages = malloc(5 * sizeof(uint16_t));

---

## 4. CERT C Embedded Subset (7 Core Rules)

| Rule | Description |
|------|-------------|
| DCL37-C | No reserved identifiers (leading underscores) |
| EXP33-C | Check pointers before dereferencing |
| EXP34-C | Initialize variables at declaration |
| INT30-C | Prevent unsigned integer wrap |
| INT31-C | Check integer conversions for overflow |
| ARR30-C | Bounds-check array indices |
| MSC37-C | Non-void functions must return a value |

[EXAMPLE: EXP33-C + DCL37-C]
static bool read_i2c_byte(const bq76920_handle_t *handle,
                           uint8_t reg,
                           uint8_t *out) {
    if (handle == NULL || out == NULL) {
        return false;
    }
    return true;
}

[EXAMPLE: ARR30-C]
if (cell_index < BQ76920_MAX_CELLS) {
    voltages[cell_index] = raw_value;
}

---

## 5. Type Usage & Error Handling

Recommended Types:
- uint8_t, int8_t (instead of unsigned char)
- uint16_t, int16_t (instead of unsigned short)
- uint32_t, int32_t (instead of unsigned long)
- bool (instead of uint8_t as boolean)

Error Codes:
typedef enum {
    BQ76920_OK = 0,
    BQ76920_ERR_I2C = -1,
    BQ76920_ERR_PARAM = -2
} bq76920_error_e;

---

## 6. Documentation Format

/**
 * @brief Read cell voltage from BQ76920 AFE
 * @param[in] handle Pointer to device handle
 * @param[out] voltage_mv Pointer to store voltage
 * @return true on success, false on error
 */
bool bq76920_read_voltage(const bq76920_handle_t *handle,
                          uint16_t *voltage_mv);

---

## 7. Git Commit Convention

- feat: add BQ76920 cell voltage reading
- fix: correct I2C address shift
- docs: add coding style guide
- refactor: extract I2C retry logic

---

## 8. Compliance Checklist

- No compiler warnings (-Wall -Wextra -Wpedantic)
- All read-only pointers are const
- Single return per function
- No implicit integer conversions
- No dynamic allocation
- No leading underscores
- All pointers checked for NULL
- Variables initialized at declaration
- Array indices bounds-checked

---

## 9. References

1. MISRA C:2012 – Amendment 3 (2025)
2. CERT C Coding Standard – Embedded Systems Subset
3. STM32 HAL Driver User Manual (UM1725)

---

End of Document