/**
 * @file bms.h
 * @brief BMS State Machine - Public Interface
 * @author Cameron Burnett
 * @date 2026-05-18
 */

#ifndef BMS_H
#define BMS_H

#include <stdbool.h>
#include <stdint.h>
#include "bq76920.h"

/*============================================================================*/
/*                              Fault Bitmask Constants                       */
/*============================================================================*/

typedef uint16_t bms_fault_mask_t;

#define BMS_FAULT_NONE  ((bms_fault_mask_t)0x00U) /**< No fault active */
#define BMS_FAULT_OV    ((bms_fault_mask_t)0x01U) /**< Overvoltage fault */
#define BMS_FAULT_UV    ((bms_fault_mask_t)0x02U) /**< Undervoltage fault */
#define BMS_FAULT_OT    ((bms_fault_mask_t)0x04U) /**< Overtemperature fault */

/*============================================================================*/
/*                              Protection Thresholds                         */
/*============================================================================*/

/** Overvoltage trip threshold (4.25V). Charge FET disabled above this. */
#define BMS_OV_THRESHOLD_MV         ((uint16_t)4250U)

/** Undervoltage trip threshold (2.8V). Discharge FET disabled below this. */
#define BMS_UV_THRESHOLD_MV         ((uint16_t)2800U)

/** Overtemperature trip threshold (60.0 deg C in 0.1 deg C units).
 *  Both FETs disabled above this. int16_t to match bq76920_handle_t.temp_tenths. */
#define BMS_OT_THRESHOLD_TENTHS     ((int16_t)600)

/** Overvoltage recovery threshold (4.20V). Charge FET re-enabled below this. */
#define BMS_OV_RECOVERY_MV          ((uint16_t)4200U)

/** Undervoltage recovery threshold (3.0V). Discharge FET re-enabled above this. */
#define BMS_UV_RECOVERY_MV          ((uint16_t)3000U)

/** Overtemperature recovery threshold (55.0 deg C). FETs re-enabled below this. */
#define BMS_OT_RECOVERY_TENTHS      ((int16_t)550)

/*============================================================================*/
/*                              Timing Constants                              */
/*============================================================================*/

/** Measurement cycle period in milliseconds. */
#define BMS_MEASUREMENT_INTERVAL_MS ((uint32_t)2000U)

/** Delay before retrying after an I2C error (milliseconds). */
#define BMS_ERROR_RECOVERY_MS       ((uint32_t)1000U)

/** Maximum consecutive I2C errors before the fault mask is set persistently. */
#define BMS_MAX_I2C_ERRORS          ((uint8_t)3U)

/** Convenience alias: total number of cells. */
#define BMS_MAX_CELLS               BQ76920_NUM_CELLS

/*============================================================================*/
/*                              Enums                                         */
/*============================================================================*/

/**
 * @brief Temperature source selector.
 */
typedef enum
{
    BMS_TEMP_DIE,           /**< Internal die temperature (TEMP_SEL=0) */
    BMS_TEMP_EXTERNAL_NTC   /**< External NTC thermistor (TEMP_SEL=1) */
} bms_temp_source_t;

/**
 * @brief BMS operating states.
 *
 * Normal flow: INIT -> IDLE -> MEASURE -> CHECK_FAULTS -> IDLE -> ...
 * Error flow:  any state -> ERROR -> IDLE (after BMS_ERROR_RECOVERY_MS)
 */
typedef enum
{
    BMS_STATE_INIT,         /**< One-shot initialisation announcement */
    BMS_STATE_IDLE,         /**< Waiting for next measurement interval */
    BMS_STATE_MEASURE,      /**< Reading cell voltages, pack voltage, temperature */
    BMS_STATE_CHECK_FAULTS, /**< Evaluating protection thresholds, controlling FETs */
    BMS_STATE_ERROR         /**< I2C communication failure — attempting recovery */
} bms_state_t;

/*============================================================================*/
/*                              Handle                                        */
/*============================================================================*/

/**
 * @brief BMS system handle — all runtime state in one struct.
 *
 * @note last_print_ms is provided for the application layer to track when
 *       to call bms_print_status(). It is not used internally by bms.c.
 */
typedef struct
{
    bms_state_t       state;                      /**< Current operating state */
    bq76920_handle_t  bq;                         /**< BQ76920 device handle */
    bms_temp_source_t temp_source;                /**< Active temperature source */

    uint32_t last_measurement_ms;                 /**< Tick of last measurement cycle start */
    uint32_t last_print_ms;                       /**< Tick of last status print (app-managed) */

    uint8_t  i2c_error_count;                     /**< Consecutive I2C read failures */

    /* Active fault bitmask — combination of BMS_FAULT_* bits */
    bms_fault_mask_t fault_mask;

    /* Overvoltage tracking (one entry per cell simultaneously in OV) */
    uint8_t  ov_cell_count;                       /**< Number of cells in overvoltage */
    uint8_t  ov_cells[BMS_MAX_CELLS];             /**< Zero-based cell indices */
    uint16_t ov_values[BMS_MAX_CELLS];            /**< Measured voltages (mV) */

    /* Undervoltage tracking */
    uint8_t  uv_cell_count;                       /**< Number of cells in undervoltage */
    uint8_t  uv_cells[BMS_MAX_CELLS];             /**< Zero-based cell indices */
    uint16_t uv_values[BMS_MAX_CELLS];            /**< Measured voltages (mV) */

    /* Overtemperature tracking */
    int16_t  ot_value;                            /**< Temperature at fault time (0.1 deg C) */
} bms_handle_t;

/*============================================================================*/
/*                              Public Functions                              */
/*============================================================================*/

/**
 * @brief Initialise the BMS system.
 *
 * Initialises the BQ76920 driver, enables ADC, selects die temperature source,
 * enables FETs, and sets initial state to BMS_STATE_INIT.
 *
 * @param[in,out] handle  BMS handle to initialise
 * @param[in]     hi2c    I2C peripheral handle
 * @return true on success, false if BQ76920 initialisation or configuration fails
 */
bool bms_init(bms_handle_t *handle, I2C_HandleTypeDef *hi2c);

/**
 * @brief Run one tick of the BMS state machine.
 *
 * Call this in the main loop on every iteration. The function is non-blocking;
 * it advances the state machine by one state per call and returns immediately.
 *
 * @param[in,out] handle  BMS handle
 */
void bms_run(bms_handle_t *handle);

/**
 * @brief Print a full BMS status report to the USB CDC terminal.
 *
 * Prints state, fault summary, individual cell voltages, pack voltage,
 * and temperature. Intended to be called periodically by the application.
 *
 * @param[in] handle  BMS handle (read-only)
 */
void bms_print_status(const bms_handle_t *handle);

#endif /* BMS_H */
