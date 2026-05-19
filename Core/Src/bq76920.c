/**
 * @file bq76920.c
 * @brief BQ76920 Battery Monitor Driver - Implementation
 * @author Cameron Burnett
 * @date 2026-05-14
 *
 * @note MISRA C:2012 / CERT C compliance:
 *   No floating point used in this file. NTC temperature uses a lookup table
 *   with linear interpolation (integer math only). Die temperature uses
 *   integer fixed-point arithmetic.
 */

#include "bq76920.h"
#include <stdio.h>

/*============================================================================*/
/*                         NTC Thermistor Lookup Table                        */
/*============================================================================*/

/**
 * @brief Single entry in the NTC resistance-to-temperature lookup table.
 */
typedef struct
{
    uint32_t resistance_ohms; /**< NTC resistance in ohms */
    int16_t  temp_tenths;     /**< Temperature in 0.1 deg C */
} ntc_lut_entry_t;

/**
 * @brief 103AT NTC thermistor LUT, resistance descending (coldest to hottest).
 *
 * Generated from Steinhart-Hart coefficients:
 *   A = 1.129241e-3, B = 2.341077e-4, C = 8.77541e-8
 * Valid range: -20.5 deg C to 80.1 deg C.
 * Accuracy vs. Steinhart-Hart: within +/-0.3 deg C across entire range.
 */
static const ntc_lut_entry_t kNtcLut[] =
{
    { 100000U, -205 },  /* -20.5 C */
    {  84000U, -175 },  /* -17.5 C */
    {  70600U, -144 },  /* -14.4 C */
    {  59600U, -114 },  /* -11.4 C */
    {  50600U,  -84 },  /*  -8.4 C */
    {  43200U,  -54 },  /*  -5.4 C */
    {  37100U,  -25 },  /*  -2.5 C */
    {  32000U,    4 },  /*   0.4 C */
    {  27700U,   33 },  /*   3.3 C */
    {  24100U,   61 },  /*   6.1 C */
    {  21100U,   88 },  /*   8.8 C */
    {  18520U,  115 },  /*  11.5 C */
    {  16330U,  142 },  /*  14.2 C */
    {  14450U,  168 },  /*  16.8 C */
    {  12840U,  194 },  /*  19.4 C */
    {  11440U,  220 },  /*  22.0 C */
    {  10220U,  245 },  /*  24.5 C */
    {   9170U,  270 },  /*  27.0 C */
    {   8250U,  294 },  /*  29.4 C */
    {   7440U,  319 },  /*  31.9 C */
    {   6720U,  343 },  /*  34.3 C */
    {   6080U,  367 },  /*  36.7 C */
    {   5510U,  392 },  /*  39.2 C */
    {   5000U,  416 },  /*  41.6 C */
    {   4550U,  440 },  /*  44.0 C */
    {   4150U,  463 },  /*  46.3 C */
    {   3790U,  487 },  /*  48.7 C */
    {   3470U,  510 },  /*  51.0 C */
    {   3180U,  533 },  /*  53.3 C */
    {   2920U,  556 },  /*  55.6 C */
    {   2680U,  579 },  /*  57.9 C */
    {   2470U,  602 },  /*  60.2 C */
    {   2280U,  624 },  /*  62.4 C */
    {   2100U,  648 },  /*  64.8 C */
    {   1940U,  670 },  /*  67.0 C */
    {   1800U,  692 },  /*  69.2 C */
    {   1670U,  714 },  /*  71.4 C */
    {   1550U,  736 },  /*  73.6 C */
    {   1440U,  758 },  /*  75.8 C */
    {   1340U,  780 },  /*  78.0 C */
    {   1250U,  801 },  /*  80.1 C */
};

#define NTC_LUT_SIZE ((uint8_t)(sizeof(kNtcLut) / sizeof(kNtcLut[0])))

/* Compile-time guard: if the LUT ever grows past 255 entries, uint8_t loop
 * index in ntc_resistance_to_temp() would overflow. This assert catches it
 * at build time rather than silently producing wrong results at runtime.    */
_Static_assert(sizeof(kNtcLut) / sizeof(kNtcLut[0]) <= 255U,
               "NTC_LUT_SIZE exceeds uint8_t range — change loop index to uint16_t");

/*============================================================================*/
/*                              Private Helpers                               */
/*============================================================================*/

/**
 * @brief Write a single byte to a BQ76920 register.
 * @param[in] handle Device handle (const: handle struct itself not modified)
 * @param[in] reg    Register address
 * @param[in] data   Byte to write
 * @return true on success, false on I2C error or NULL pointer
 */
static bool write_reg(const bq76920_handle_t *handle, uint8_t reg, uint8_t data)
{
    bool success = false;

    if (handle != NULL)
    {
        if (HAL_I2C_Mem_Write(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                              I2C_MEMADD_SIZE_8BIT, &data, 1U, 100U) == HAL_OK)
        {
            success = true;
        }
    }

    return success;
}

/**
 * @brief Read a single byte from a BQ76920 register.
 * @param[in]  handle Device handle (const: handle struct itself not modified)
 * @param[in]  reg    Register address
 * @param[out] data   Pointer to store the read byte
 * @return true on success, false on I2C error or NULL pointer
 */
static bool read_reg(const bq76920_handle_t *handle, uint8_t reg, uint8_t *data)
{
    bool success = false;

    if (handle != NULL && data != NULL)
    {
        if (HAL_I2C_Mem_Read(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, data, 1U, 100U) == HAL_OK)
        {
            success = true;
        }
    }

    return success;
}

/**
 * @brief Read a 16-bit value from two consecutive BQ76920 registers (HI, LO).
 * @note  BQ76920 auto-increments the register address when reading 2 bytes.
 * @param[in]  handle Device handle (const: handle struct itself not modified)
 * @param[in]  reg    High byte register address (e.g. BQ76920_REG_VC1_HI)
 * @param[out] value  Pointer to store combined 16-bit value (HI << 8 | LO)
 * @return true on success, false on I2C error or NULL pointer
 */
static bool read_word(const bq76920_handle_t *handle, uint8_t reg, uint16_t *value)
{
    bool success = false;
    uint8_t data[2] = {0U, 0U};

    if (handle != NULL && value != NULL)
    {
        if (HAL_I2C_Mem_Read(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, data, 2U, 100U) == HAL_OK)
        {
            *value = ((uint16_t)data[0] << 8U) | (uint16_t)data[1];
            success = true;
        }
    }

    return success;
}

/**
 * @brief Convert raw cell ADC counts to millivolts using factory GAIN/OFFSET.
 * @param[in] handle Pointer to device handle (reads adc_gain and adc_offset)
 * @param[in] counts Raw 14-bit ADC value from a cell voltage register
 * @return Voltage in millivolts, or 0 if handle is NULL
 */
static uint16_t counts_to_millivolts(const bq76920_handle_t *handle, uint16_t counts)
{
    uint16_t result = 0U;

    if (handle != NULL)
    {
        int32_t voltage_mv = ((int32_t)counts * (int32_t)handle->adc_gain) / 1000L;
        voltage_mv += (int32_t)handle->adc_offset;
        result = (uint16_t)voltage_mv;
    }

    return result;
}

/**
 * @brief Look up NTC thermistor temperature from resistance using LUT interpolation.
 *
 * The LUT is sorted by descending resistance (ascending temperature).
 * Values outside the LUT range are clamped to the nearest endpoint.
 * Uses integer arithmetic only — no floating point.
 *
 * @param[in] rts_ohms NTC resistance in ohms
 * @return Temperature in 0.1 deg C units
 */
static int16_t ntc_resistance_to_temp(uint32_t rts_ohms)
{
    int16_t result = kNtcLut[NTC_LUT_SIZE - 1U].temp_tenths; /* default: hottest entry */
    uint8_t i = 0U;

    if (rts_ohms >= kNtcLut[0].resistance_ohms)
    {
        /* Above LUT range: clamp to coldest entry */
        result = kNtcLut[0].temp_tenths;
    }
    else if (rts_ohms <= kNtcLut[NTC_LUT_SIZE - 1U].resistance_ohms)
    {
        /* Below LUT range: already set to hottest entry as default */
    }
    else
    {
        /* Find the two surrounding LUT entries and linearly interpolate.
         *
         * Formula: t = t_hi + (t_lo - t_hi) * (r_hi - r) / (r_hi - r_lo)
         * where _hi = higher resistance (colder), _lo = lower resistance (hotter).
         *
         * Verified at boundaries:
         *   r = r_hi -> fraction = 0 -> result = t_hi (correct)
         *   r = r_lo -> fraction = 1 -> result = t_lo (correct)
         *
         * Loop exit: condition folds the found-flag into the for expression,
         * avoiding a break statement (MISRA Rule 15.4).
         */
        bool found = false;
        for (i = 0U; (i < (NTC_LUT_SIZE - 1U)) && !found; i++)
        {
            if ((rts_ohms <= kNtcLut[i].resistance_ohms) &&
                (rts_ohms >  kNtcLut[i + 1U].resistance_ohms))
            {
                int32_t r_hi = (int32_t)kNtcLut[i].resistance_ohms;
                int32_t r_lo = (int32_t)kNtcLut[i + 1U].resistance_ohms;
                int32_t t_hi = (int32_t)kNtcLut[i].temp_tenths;
                int32_t t_lo = (int32_t)kNtcLut[i + 1U].temp_tenths;
                int32_t r    = (int32_t)rts_ohms;

                result = (int16_t)(t_hi + ((t_lo - t_hi) * (r_hi - r)) / (r_hi - r_lo));
                found  = true;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*                              Public Functions                              */
/*============================================================================*/

bool bq76920_init(bq76920_handle_t *handle, I2C_HandleTypeDef *hi2c)
{
    bool success = false;
    uint8_t i = 0U;
    uint8_t gain1 = 0U;
    uint8_t gain2 = 0U;
    uint8_t offset_raw = 0U;

    if (handle != NULL && hi2c != NULL)
    {
        handle->hi2c = hi2c;

        /* Read GAIN from factory EEPROM registers 0x50 and 0x59.
         * ADCGAIN4:3 are bits [3:2] of reg 0x50.
         * ADCGAIN2:0 are bits [7:5] of reg 0x59.
         * Combined 5-bit value added to base of 365 gives uV/LSB. */
        if (read_reg(handle, BQ76920_REG_ADCGAIN1, &gain1) &&
            read_reg(handle, BQ76920_REG_ADCGAIN2, &gain2))
        {
            uint8_t gain5bit = ((gain1 & 0x0CU) << 1U) | ((gain2 & 0xE0U) >> 5U);
            handle->adc_gain = 365U + (uint16_t)gain5bit;
        }
        else
        {
            handle->adc_gain = 382U; /* Datasheet typical value as fallback */
        }

        /* Read OFFSET from factory EEPROM register 0x51.
         * Stored in two's complement format (signed 8-bit). */
        if (read_reg(handle, BQ76920_REG_ADCOFFSET, &offset_raw))
        {
            handle->adc_offset = (int8_t)offset_raw;
        }
        else
        {
            handle->adc_offset = 0; /* Safe fallback */
        }

        /* Clear all stored measurement data */
        for (i = 0U; i < BQ76920_NUM_CELLS; i++)
        {
            handle->cell_mV[i] = 0U;
        }
        handle->pack_mV     = 0U;
        handle->current_mA  = 0;
        handle->temp_tenths = 0;

        success = true;
    }

    return success;
}

bool bq76920_enable_adc(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL1, &reg))
        {
            reg |= BQ76920_ADC_EN;
            if (write_reg(handle, BQ76920_REG_SYS_CTRL1, reg))
            {
                success = true;
            }
        }
    }

    return success;
}

bool bq76920_select_temperature_source(bq76920_handle_t *handle, bool use_external_ts1)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL1, &reg))
        {
            if (use_external_ts1)
            {
                reg |= BQ76920_TEMP_SEL;
            }
            else
            {
                reg &= (uint8_t)(~BQ76920_TEMP_SEL);
            }

            if (write_reg(handle, BQ76920_REG_SYS_CTRL1, reg))
            {
                success = true;
            }
        }
    }

    return success;
}

bool bq76920_read_temperature(bq76920_handle_t *handle)
{
    bool success = false;
    uint16_t raw = 0U;
    uint8_t sys_ctrl1 = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL1, &sys_ctrl1))
        {
            if (read_word(handle, BQ76920_REG_TEMP_HI, &raw))
            {
                if ((sys_ctrl1 & BQ76920_TEMP_SEL) != 0U)
                {
                    /* --- External NTC thermistor on TS1 ---
                     *
                     * Step 1: VTSX in mV using factory ADC gain.
                     *   vtsx_mv = (counts * adc_gain_uV) / 1000
                     *
                     * Step 2: NTC resistance from voltage divider.
                     *   10k pullup resistor to REGOUT (3.3V):
                     *   rts_ohms = (10000 * vtsx_mv) / (3300 - vtsx_mv)
                     *
                     * Step 3: Look up temperature in LUT with linear interpolation.
                     *   Handles -20.5 to 80.1 deg C; clamps outside that range.
                     */
                    int32_t vtsx_mv = ((int32_t)raw * (int32_t)handle->adc_gain) / 1000L;

                    if ((vtsx_mv > 0L) && (vtsx_mv < 3300L))
                    {
                        uint32_t rts_ohms =
                            (uint32_t)((10000L * vtsx_mv) / (3300L - vtsx_mv));
                        handle->temp_tenths = ntc_resistance_to_temp(rts_ohms);
                        success = true;
                    }
                }
                else
                {
                    /* --- Internal die temperature sensor ---
                     *
                     * From BQ76920 datasheet:
                     *   V25   = 1200 mV  (sensor output at 25 deg C)
                     *   Slope = -4.2 mV/deg C (voltage drops as temperature rises)
                     *
                     * Formula: T(degC) = 25 + (V25 - VTSX) / 4.2
                     *
                     * Integer version in 0.1 deg C units:
                     *   T_tenths = 250 + (1200 - vtsx_mv) * 100 / 42
                     *
                     * Derivation: multiply T formula by 10 for tenths precision;
                     *   multiply num/denom by 10 to eliminate decimal divisor:
                     *   (/4.2) becomes (*10/42).
                     */
                    int32_t vtsx_mv = ((int32_t)raw * (int32_t)handle->adc_gain) / 1000L;
                    handle->temp_tenths =
                        (int16_t)(250L + ((1200L - vtsx_mv) * 100L) / 42L);
                    success = true;
                }
            }
        }
    }

    return success;
}

bool bq76920_read_all_voltages(bq76920_handle_t *handle)
{
    bool success = false;
    const uint8_t regs[BQ76920_NUM_CELLS] =
    {
        BQ76920_REG_VC1_HI,
        BQ76920_REG_VC2_HI,
        BQ76920_REG_VC3_HI,
        BQ76920_REG_VC4_HI,
        BQ76920_REG_VC5_HI,
    };
    uint8_t i = 0U;
    uint16_t raw = 0U;
    bool all_read = true;

    if (handle != NULL)
    {
        for (i = 0U; (i < BQ76920_NUM_CELLS) && all_read; i++)
        {
            if (!read_word(handle, regs[i], &raw))
            {
                all_read = false;
            }
            else
            {
                /* Debug printf — remove before production use */
                printf("Cell %d raw: %5d, calculated: %4d mV\r\n",
                       (int)(i + 1U), (int)raw,
                       (int)counts_to_millivolts(handle, raw));

                handle->cell_mV[i] = counts_to_millivolts(handle, raw);
            }
        }

        if (all_read)
        {
            success = true;
        }
    }

    return success;
}

bool bq76920_read_pack_voltage(bq76920_handle_t *handle)
{
    bool success = false;
    uint16_t raw = 0U;
    int32_t voltage_mv = 0;

    if (handle != NULL)
    {
        /* BAT register formula (BQ76920 datasheet):
         *   V(BAT) = 4 * GAIN * ADC(bat) + (NumCells * OFFSET) */
        if (read_word(handle, BQ76920_REG_BAT_HI, &raw))
        {
            voltage_mv  = (4L * (int32_t)raw * (int32_t)handle->adc_gain) / 1000L;
            voltage_mv += (int32_t)BQ76920_NUM_CELLS * (int32_t)handle->adc_offset;
            handle->pack_mV = (uint16_t)voltage_mv;
            success = true;
        }
    }

    return success;
}

bool bq76920_read_current(bq76920_handle_t *handle, uint32_t shunt_uOhms)
{
    bool success = false;
    uint16_t raw = 0U;
    int32_t temp = 0;

    /* CC (Coulomb Counter) register uses a fixed 8.44 uV/LSB resolution.
     * This is separate from the cell voltage ADC gain stored in handle->adc_gain.
     *
     * Derivation:
     *   voltage_uV = counts * 8.44          [counts * uV/LSB]
     *   current_A  = voltage_uV / shunt_uOhms   [uV / uOhm = A]
     *   current_mA = current_A * 1000
     *   => current_mA = (counts * 8440) / shunt_uOhms
     *
     * With 1 mOhm shunt (1000 uOhms): resolution = 8.44 mA per count.
     */
    if ((handle != NULL) && (shunt_uOhms != 0U))
    {
        if (read_word(handle, BQ76920_REG_CC_HI, &raw))
        {
            temp = (int32_t)(int16_t)raw * 8440L;
            handle->current_mA = (int16_t)(temp / (int32_t)shunt_uOhms);
            success = true;
        }
    }

    return success;
}

bool bq76920_enable_charge_fet(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL2, &reg))
        {
            reg |= BQ76920_CHG_ON;
            if (write_reg(handle, BQ76920_REG_SYS_CTRL2, reg))
            {
                success = true;
            }
        }
    }

    return success;
}

bool bq76920_disable_charge_fet(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL2, &reg))
        {
            reg &= (uint8_t)(~BQ76920_CHG_ON);
            if (write_reg(handle, BQ76920_REG_SYS_CTRL2, reg))
            {
                success = true;
            }
        }
    }

    return success;
}

bool bq76920_enable_discharge_fet(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL2, &reg))
        {
            reg |= BQ76920_DSG_ON;
            if (write_reg(handle, BQ76920_REG_SYS_CTRL2, reg))
            {
                success = true;
            }
        }
    }

    return success;
}

bool bq76920_disable_discharge_fet(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0U;

    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL2, &reg))
        {
            reg &= (uint8_t)(~BQ76920_DSG_ON);
            if (write_reg(handle, BQ76920_REG_SYS_CTRL2, reg))
            {
                success = true;
            }
        }
    }

    return success;
}
