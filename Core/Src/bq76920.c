/**
 * @file bq76920.c
 * @brief BQ76920 Battery Monitor Driver - Implementation
 * @author Cameron Burnett
 * @date 2026-05-14
 */

#include "bq76920.h"
#include <stdio.h>
#include <math.h>

/*============================================================================*/
/*                              Private Helpers                               */
/*============================================================================*/

/**
 * @brief Write a single byte to a register
 * @param handle Device handle
 * @param reg Register address 
 * @param data Data to write
 * @return true on success, false on error
 */
static bool write_reg(bq76920_handle_t *handle, uint8_t reg, uint8_t data)
{
    bool success = false;
    
    if (handle != NULL) 
    {
        if (HAL_I2C_Mem_Write(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100) == HAL_OK) 
        {
            success = true;
        }
    }
    
    return success;
}

/**
 * @brief Read a single byte from a register
 * @param handle Device handle
 * @param reg Register address 
 * @param value Pointer to the variable to store the read value
 * @return true on success, false on error
 */
static bool read_reg(bq76920_handle_t *handle, uint8_t reg, uint8_t *data)
{
    bool success = false;
    
    if (handle != NULL && data != NULL) 
    {
        if (HAL_I2C_Mem_Read(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, data, 1, 100) == HAL_OK) 
        {
            success = true;
        }
    }
    
    return success;
}

/**
 * @brief Read a 16-bit register (HI and LO combined)
 * @note BQ76920 auto-increments from HI to LO when reading 2 bytes
 * @param handle Device handle
 * @param reg HI register address (e.g., BQ76920_REG_VC1_HI)
 * @param value Pointer to store combined 16-bit value (HI << 8 | LO)
 * @return true on success, false on error
 */
static bool read_word(bq76920_handle_t *handle, uint8_t reg, uint16_t *value)
{
    bool success = false;
    uint8_t data[2] = {0};
    
    if (handle != NULL && value != NULL) 
    {
        if (HAL_I2C_Mem_Read(handle->hi2c, BQ76920_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, data, 2, 100) == HAL_OK) 
        {
            *value = ((uint16_t)data[0] << 8U) | data[1];
            success = true;
        }
    }
    
    return success;
}

/**
 * @brief Convert raw ADC counts to millivolts for cell readings only
 */
static uint16_t counts_to_millivolts(bq76920_handle_t *handle, uint16_t counts)
{
    int32_t voltage_mv;
    
    // Use the gain and offset from EEPROM for more accurate conversion
    voltage_mv = ((int32_t)counts * (int32_t)handle->adc_gain) / 1000L;
    voltage_mv += (int32_t)handle->adc_offset;
    
    return (uint16_t)voltage_mv;
}

/*============================================================================*/
/*                              Public Functions                              */
/*============================================================================*/

bool bq76920_init(bq76920_handle_t *handle, I2C_HandleTypeDef *hi2c)
{
    bool success = false;
    uint8_t i;
    uint8_t gain1 = 0;
    uint8_t gain2 = 0;
    uint8_t gain5bit = 0;
    uint8_t offset_raw = 0;
    
    if (handle != NULL && hi2c != NULL)
    {
        handle->hi2c = hi2c;
        
        /* Read GAIN from factory EEPROM */
        if (read_reg(handle, 0x50, &gain1) && read_reg(handle, 0x59, &gain2))
        {
            /* Combine ADCGAIN4:3 (0x50 bits 3-2) and ADCGAIN2:0 (0x59 bits 7-5) */
            gain5bit = ((gain1 & 0x0C) << 1) | ((gain2 & 0xE0) >> 5);
            handle->adc_gain = 365 + gain5bit;
        }
        else
        {
            handle->adc_gain = 382;  /* Default fallback */
        }
        
        /* Read OFFSET from factory EEPROM */
        if (read_reg(handle, 0x51, &offset_raw))
        {
            /* OFFSET is stored in 2's complement format */
            handle->adc_offset = (int8_t)offset_raw;
        }
        else
        {
            handle->adc_offset = 0;  /* Default fallback */
        }
        
        /* Clear stored data */
        for (i = 0; i < BQ76920_NUM_CELLS; i++)
        {
            handle->cell_mV[i] = 0;
        }
        handle->pack_mV = 0;
        handle->current_mA = 0;
        handle->temp_tenths = 0;
        
        success = true;
    }
    
    return success;
}

bool bq76920_enable_adc(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0;
    
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
    uint8_t reg = 0;
    
    if (handle != NULL)
    {
        if (read_reg(handle, BQ76920_REG_SYS_CTRL1, &reg))
        {
            if (use_external_ts1)
            {
                reg |= BQ76920_TEMP_SEL;      /* Use external TS1 thermistor */
            }
            else
            {
                reg &= (uint8_t)(~BQ76920_TEMP_SEL);  /* Use internal die temperature */
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
    uint16_t raw = 0;
    uint8_t sys_ctrl1 = 0;
    double vtsx = 0.0;
    double rts = 0.0;
    double ln_r = 0.0;
    double inv_t = 0.0;
    double temp_c = 0.0;
    int32_t vtsx_mv = 0;
    
    if (handle != NULL)
    {
        /* Read current temperature source setting */
        if (read_reg(handle, BQ76920_REG_SYS_CTRL1, &sys_ctrl1))
        {
            /* Read TS1 register for temperature */
            if (read_word(handle, BQ76920_REG_TEMP_HI, &raw))
            {
                //printf("Raw temperature reading: %d\r\n", raw);
                if (sys_ctrl1 & BQ76920_TEMP_SEL)
                {
                    /* External thermistor on TS1 */
                    /* VTSX = counts × 382 µV/LSB = counts × 0.000382 V */
                    vtsx = (double)raw * 0.000382;
                    
                    /* RTS = (10000 × VTSX) ÷ (3.3 – VTSX) */
                    rts = (10000.0 * vtsx) / (3.3 - vtsx);
                    
                    /* Steinhart-Hart for 10k NTC 103AT thermistor */
                    ln_r = log(rts);
                    inv_t = 1.129241e-3 + (2.341077e-4 * ln_r) + (8.77541e-8 * ln_r * ln_r * ln_r);
                    temp_c = (1.0 / inv_t) - 273.15;
                    
                    handle->temp_tenths = (int16_t)(temp_c * 10.0);
                    success = true;
                }
                else
                {
                    /* Die temperature (internal chip temperature) */
                    /* V25 = 1.200V = 1200 mV */
                    /* VTSX = counts × 382 µV/LSB = counts × 0.382 mV */
                    vtsx_mv = ((int32_t)raw * 382) / 1000;
                    
                    /* TEMPDIE = 25° – ((VTSX – V25) ÷ 0.0042) */
                    /* 0.0042 V/°C = 4.2 mV/°C */
                    temp_c = 25.0 - ((double)(vtsx_mv - 1200) / 4.2);
                    
                    handle->temp_tenths = (int16_t)(temp_c * 10.0);
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
    uint8_t regs[BQ76920_NUM_CELLS] = 
    {
        BQ76920_REG_VC1_HI,
        BQ76920_REG_VC2_HI,
        BQ76920_REG_VC3_HI,
        BQ76920_REG_VC4_HI,
        BQ76920_REG_VC5_HI,
    };
    uint8_t i;
    uint16_t raw = 0;
    bool all_read = true;
    
    if (handle != NULL) 
    {
        for (i = 0; i < BQ76920_NUM_CELLS; i++) 
        {
            if (!read_word(handle, regs[i], &raw)) 
            {
                all_read = false;
                break;
            }
            printf("Cell %d raw: %5d, calculated: %4d mV\r\n", 
                   i + 1, raw, counts_to_millivolts(handle, raw));

            handle->cell_mV[i] = counts_to_millivolts(handle, raw);
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
    uint16_t raw = 0;          /* raw will hold the 16-bit value from BAT_HI/LO */
    int32_t voltage_mv = 0;
    
    if (handle != NULL)
    {
        /* Read 16-bit value from BAT_HI (0x2A) and BAT_LO (0x2B) */
        if (read_word(handle, BQ76920_REG_BAT_HI, &raw))
        {
            /* V(BAT) = 4 × GAIN × ADC(bat) + (#Cells × OFFSET) */
            voltage_mv = (4L * (int32_t)raw * (int32_t)handle->adc_gain) / 1000L;
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
    uint16_t raw = 0;
    int16_t counts = 0;
    int32_t temp = 0;
    
    if (handle != NULL && shunt_uOhms != 0) 
    {
        if (read_word(handle, BQ76920_REG_CC_HI, &raw)) 
        {
            /* Convert raw counts to milliamps */
            counts = (int16_t)raw;
            temp = (int32_t)counts * 3815L;
            handle->current_mA = (int16_t)(temp / (10L * (int32_t)shunt_uOhms));
            success = true;
        }
    }
    
    return success;
}

bool bq76920_enable_charge_fet(bq76920_handle_t *handle)
{
    bool success = false;
    uint8_t reg = 0;
    
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
    uint8_t reg = 0;
    
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
    uint8_t reg = 0;
    
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
    uint8_t reg = 0;
    
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