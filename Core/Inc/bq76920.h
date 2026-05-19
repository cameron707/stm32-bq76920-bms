/**
 * @file bq76920.h
 * @brief BQ76920 Battery Monitor Driver - Public Interface
 * @author Cameron Burnett
 * @date 2026-05-14
 */

#ifndef BQ76920_H
#define BQ76920_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/*============================================================================*/
/*                              Constants                                     */
/*============================================================================*/

#define BQ76920_I2C_ADDR_7BIT       0x08U         /**< 7-bit I2C address */
#define BQ76920_I2C_ADDR_8BIT       (0x08U << 1U) /**< 8-bit I2C address for HAL (left-shifted for R/W bit) */

/*============================================================================*/
/*                              Register Addresses                            */
/*============================================================================*/

/* System registers */
#define BQ76920_REG_SYS_STAT        0x00U   /**< System status */
#define BQ76920_REG_CELLBAL1        0x01U   /**< Cell balancing 1 */
#define BQ76920_REG_CELLBAL2        0x02U   /**< Cell balancing 2 */
#define BQ76920_REG_CELLBAL3        0x03U   /**< Cell balancing 3 */
#define BQ76920_REG_SYS_CTRL1       0x04U   /**< System control 1 */
#define BQ76920_REG_SYS_CTRL2       0x05U   /**< System control 2 */

/* Protection registers */
#define BQ76920_REG_PROTECT1        0x06U   /**< Protection 1 */
#define BQ76920_REG_PROTECT2        0x07U   /**< Protection 2 */
#define BQ76920_REG_PROTECT3        0x08U   /**< Protection 3 */
#define BQ76920_REG_OV_TRIP         0x09U   /**< Overvoltage trip threshold */
#define BQ76920_REG_UV_TRIP         0x0AU   /**< Undervoltage trip threshold */

/* Configuration registers */
#define BQ76920_REG_CC_CFG          0x0BU   /**< Coulomb counter config */

/* Cell voltage registers (5 cells for BQ76920) */
#define BQ76920_REG_VC1_HI          0x0CU   /**< Cell 1 voltage MSB */
#define BQ76920_REG_VC1_LO          0x0DU   /**< Cell 1 voltage LSB */
#define BQ76920_REG_VC2_HI          0x0EU   /**< Cell 2 voltage MSB */
#define BQ76920_REG_VC2_LO          0x0FU   /**< Cell 2 voltage LSB */
#define BQ76920_REG_VC3_HI          0x10U   /**< Cell 3 voltage MSB */
#define BQ76920_REG_VC3_LO          0x11U   /**< Cell 3 voltage LSB */
#define BQ76920_REG_VC4_HI          0x12U   /**< Cell 4 voltage MSB */
#define BQ76920_REG_VC4_LO          0x13U   /**< Cell 4 voltage LSB */
#define BQ76920_REG_VC5_HI          0x14U   /**< Cell 5 voltage MSB */
#define BQ76920_REG_VC5_LO          0x15U   /**< Cell 5 voltage LSB */

/* Pack voltage registers */
#define BQ76920_REG_BAT_HI          0x2AU   /**< Pack voltage MSB */
#define BQ76920_REG_BAT_LO          0x2BU   /**< Pack voltage LSB */

/* Thermistor registers (TS1 used for temperature) */
#define BQ76920_REG_TS1_HI          0x2CU   /**< Thermistor 1 voltage MSB */
#define BQ76920_REG_TS1_LO          0x2DU   /**< Thermistor 1 voltage LSB */

/* Coulomb counter registers */
#define BQ76920_REG_CC_HI           0x32U   /**< Coulomb counter MSB */
#define BQ76920_REG_CC_LO           0x33U   /**< Coulomb counter LSB */

/* Factory trim registers (read-only, do not modify) */
#define BQ76920_REG_ADCGAIN1        0x50U   /**< ADC gain 1 */
#define BQ76920_REG_ADCOFFSET       0x51U   /**< ADC offset */
#define BQ76920_REG_ADCGAIN2        0x59U   /**< ADC gain 2 */

/* Temperature alias (die temperature via TS1 when no external thermistor) */
#define BQ76920_REG_TEMP_HI         BQ76920_REG_TS1_HI
#define BQ76920_REG_TEMP_LO         BQ76920_REG_TS1_LO

/*============================================================================*/
/*                              Bit Masks                                     */
/*============================================================================*/

/* SYS_CTRL1 bits */
#define BQ76920_ADC_EN              0x10U   /**< ADC enable (binary 00010000) */
#define BQ76920_TEMP_SEL            0x08U   /**< Temperature select (1 = external TS1, 0 = die) (binary 00001000) */

/* SYS_CTRL2 bits */
#define BQ76920_DSG_ON              0x02U   /**< Discharge FET on (binary 00000010) */
#define BQ76920_CHG_ON              0x01U   /**< Charge FET on (binary 00000001) */

/* Number of cells in the configuration */
#define BQ76920_NUM_CELLS           5U

/*============================================================================*/
/*                              Types                                         */
/*============================================================================*/

/**
 * @brief BQ76920 device handle
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;                       /**< I2C handle */
    uint16_t cell_mV[BQ76920_NUM_CELLS];           /**< Last read cell voltages (mV) */
    uint16_t pack_mV;                              /**< Last read pack voltage (mV) */
    int16_t current_mA;                            /**< Last read current (mA) */
    int16_t temp_tenths;                           /**< Last read temperature (0.1°C) */
    uint16_t adc_gain;                             /**< ADC gain in uV/LSB (factory calibrated, range 365-396) */
    int8_t adc_offset;                             /**< ADC offset in mV (factory calibrated, range -128 to +127) */
} bq76920_handle_t;

/*============================================================================*/
/*                              Public Functions                              */
/*============================================================================*/

/**
 * @brief Initialize BQ76920 device
 * @param handle Device handle
 * @param hi2c I2C handle
 * @return true on success, false on error
 */
bool bq76920_init(bq76920_handle_t *handle, I2C_HandleTypeDef *hi2c);

/**
 * @brief Enable ADC (required for measurements)
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_enable_adc(bq76920_handle_t *handle);

/**
 * @brief Select temperature source (die or external NTC)
 * @param handle Device handle
 * @param use_external_ts1 true for external TS1 thermistor, false for die temperature
 * @return true on success, false on error
 */
bool bq76920_select_temperature_source(bq76920_handle_t *handle, bool use_external_ts1);

/**
 * @brief Read all cell voltages
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_read_all_voltages(bq76920_handle_t *handle);

/**
 * @brief Read pack voltage
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_read_pack_voltage(bq76920_handle_t *handle);

/**
 * @brief Read pack current
 * @param handle Device handle
 * @param shunt_uOhms Shunt resistor value in microOhms
 * @return true on success, false on error
 */
bool bq76920_read_current(bq76920_handle_t *handle, uint32_t shunt_uOhms);

/**
 * @brief Read temperature (automatically uses selected source: die or external)
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_read_temperature(bq76920_handle_t *handle);

/**
 * @brief Enable charge FET
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_enable_charge_fet(bq76920_handle_t *handle);

/**
 * @brief Disable charge FET
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_disable_charge_fet(bq76920_handle_t *handle);

/**
 * @brief Enable discharge FET
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_enable_discharge_fet(bq76920_handle_t *handle);

/**
 * @brief Disable discharge FET
 * @param handle Device handle
 * @return true on success, false on error
 */
bool bq76920_disable_discharge_fet(bq76920_handle_t *handle);

#endif /* BQ76920_H */