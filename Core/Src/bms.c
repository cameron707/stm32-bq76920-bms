/**
 * @file bms.c
 * @brief BMS State Machine - Implementation
 * @author Cameron Burnett
 * @date 2026-05-18
 */

#include "bms.h"
#include <stdio.h>

/*============================================================================*/
/*                              Private Helpers                               */
/*============================================================================*/

/**
 * @brief Print a temperature value as "DD.D C" handling negative fractions.
 * @param[in] tenths  Temperature in 0.1 deg C units
 */
static void print_temp(int16_t tenths)
{
    int16_t whole = tenths / 10;
    int16_t frac  = tenths % 10;
    if (frac < 0)
    {
        frac = (int16_t)-frac;
    }
    printf("%d.%d C", (int)whole, (int)frac);
}

/**
 * @brief Read all measurements from the BQ76920 in one pass.
 *
 * Reads cell voltages, pack voltage, and temperature. All three must
 * succeed for the function to return true.
 *
 * @param[in,out] handle  BMS handle
 * @return true if all reads succeeded, false on any I2C failure
 */
static bool read_all_measurements(bms_handle_t *handle)
{
    bool success = false;

    if (handle != NULL)
    {
        if (bq76920_read_all_voltages(&handle->bq)  &&
            bq76920_read_pack_voltage(&handle->bq)  &&
            bq76920_read_temperature(&handle->bq))
        {
            success = true;
        }
    }

    return success;
}

/**
 * @brief Reset all fault tracking arrays to zero.
 * @param[in,out] handle  BMS handle
 */
static void clear_fault_tracking(bms_handle_t *handle)
{
    uint8_t i = 0U;

    if (handle != NULL)
    {
        handle->ov_cell_count = 0U;
        handle->uv_cell_count = 0U;
        handle->ot_value      = 0;

        for (i = 0U; i < BMS_MAX_CELLS; i++)
        {
            handle->ov_cells[i]  = 0U;
            handle->ov_values[i] = 0U;
            handle->uv_cells[i]  = 0U;
            handle->uv_values[i] = 0U;
        }
    }
}

/**
 * @brief Evaluate all protection thresholds and update FET states.
 *
 * Uses hysteresis: if a fault is already active, the recovery threshold is
 * applied instead of the trip threshold, preventing chattering at the boundary.
 *
 * FET control return values are checked; a failed write is logged but does not
 * change the state machine (the BQ76920 hardware layer remains active).
 *
 * @param[in,out] handle  BMS handle
 */
static void check_faults(bms_handle_t *handle)
{
    uint8_t i = 0U;
    bms_fault_mask_t new_fault_mask = BMS_FAULT_NONE;
    uint16_t ov_threshold = 0U;
    uint16_t uv_threshold = 0U;
    int16_t  ot_threshold = 0;

    /* Per-cell tracking — local copies, committed to handle at end */
    uint8_t  local_ov_cells[BMS_MAX_CELLS]  = {0U};
    uint16_t local_ov_values[BMS_MAX_CELLS] = {0U};
    uint8_t  local_uv_cells[BMS_MAX_CELLS]  = {0U};
    uint16_t local_uv_values[BMS_MAX_CELLS] = {0U};
    uint8_t  local_ov_count = 0U;
    uint8_t  local_uv_count = 0U;
    int16_t  local_ot_value = 0;
    bool     ot_detected    = false;

    if (handle != NULL)
    {
        /* Select thresholds — trip or recovery depending on existing fault state */
        ov_threshold = ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE) ?
                       BMS_OV_RECOVERY_MV : BMS_OV_THRESHOLD_MV;

        uv_threshold = ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE) ?
                       BMS_UV_RECOVERY_MV : BMS_UV_THRESHOLD_MV;

        ot_threshold = ((handle->fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE) ?
                       BMS_OT_RECOVERY_TENTHS : BMS_OT_THRESHOLD_TENTHS;

        /* Check each cell for OV and UV */
        for (i = 0U; i < BMS_MAX_CELLS; i++)
        {
            if (handle->bq.cell_mV[i] > ov_threshold)
            {
                new_fault_mask |= BMS_FAULT_OV;
                if (local_ov_count < BMS_MAX_CELLS)
                {
                    local_ov_cells[local_ov_count]  = i;
                    local_ov_values[local_ov_count] = handle->bq.cell_mV[i];
                    local_ov_count++;
                }
            }

            if (handle->bq.cell_mV[i] < uv_threshold)
            {
                new_fault_mask |= BMS_FAULT_UV;
                if (local_uv_count < BMS_MAX_CELLS)
                {
                    local_uv_cells[local_uv_count]  = i;
                    local_uv_values[local_uv_count] = handle->bq.cell_mV[i];
                    local_uv_count++;
                }
            }
        }

        /* Overtemperature check — both thresholds are int16_t, no sign mismatch */
        if (handle->bq.temp_tenths > ot_threshold)
        {
            new_fault_mask |= BMS_FAULT_OT;
            local_ot_value = handle->bq.temp_tenths;
            ot_detected    = true;
        }

        /* Commit fault state to handle */
        handle->fault_mask    = new_fault_mask;
        handle->ov_cell_count = local_ov_count;
        handle->uv_cell_count = local_uv_count;

        for (i = 0U; i < local_ov_count; i++)
        {
            handle->ov_cells[i]  = local_ov_cells[i];
            handle->ov_values[i] = local_ov_values[i];
        }

        for (i = 0U; i < local_uv_count; i++)
        {
            handle->uv_cells[i]  = local_uv_cells[i];
            handle->uv_values[i] = local_uv_values[i];
        }

        if (ot_detected)
        {
            handle->ot_value = local_ot_value;
        }

        /* Apply FET controls.
         * OT takes priority: both FETs off regardless of OV/UV state.
         * OV: charge FET off.
         * UV: discharge FET off.
         * Failed FET writes are logged — hardware OV/UV/OT protection remains active. */
        if ((new_fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE)
        {
            if (!bq76920_disable_charge_fet(&handle->bq))
            {
                printf("BMS: WARNING - charge FET disable failed\r\n");
            }
            if (!bq76920_disable_discharge_fet(&handle->bq))
            {
                printf("BMS: WARNING - discharge FET disable failed\r\n");
            }
        }
        else
        {
            if ((new_fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE)
            {
                if (!bq76920_disable_charge_fet(&handle->bq))
                {
                    printf("BMS: WARNING - charge FET disable failed\r\n");
                }
            }
            else
            {
                if (!bq76920_enable_charge_fet(&handle->bq))
                {
                    printf("BMS: WARNING - charge FET enable failed\r\n");
                }
            }

            if ((new_fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE)
            {
                if (!bq76920_disable_discharge_fet(&handle->bq))
                {
                    printf("BMS: WARNING - discharge FET disable failed\r\n");
                }
            }
            else
            {
                if (!bq76920_enable_discharge_fet(&handle->bq))
                {
                    printf("BMS: WARNING - discharge FET enable failed\r\n");
                }
            }
        }
    }
}

/**
 * @brief Print active fault details to the terminal.
 *
 * Only called when fault_mask != BMS_FAULT_NONE.
 *
 * @param[in] handle  BMS handle (read-only)
 */
static void print_faults(const bms_handle_t *handle)
{
    uint8_t i = 0U;

    if (handle != NULL && handle->fault_mask != BMS_FAULT_NONE)
    {
        printf("\r\n*** FAULTS DETECTED ***\r\n");

        if ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE)
        {
            printf("  OVERVOLTAGE (%d cell(s)):\r\n", (int)handle->ov_cell_count);
            for (i = 0U; i < handle->ov_cell_count; i++)
            {
                printf("    Cell %d: %d mV (threshold: %d mV)\r\n",
                       (int)(handle->ov_cells[i] + 1U),
                       (int)handle->ov_values[i],
                       (int)BMS_OV_THRESHOLD_MV);
            }
        }

        if ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE)
        {
            printf("  UNDERVOLTAGE (%d cell(s)):\r\n", (int)handle->uv_cell_count);
            for (i = 0U; i < handle->uv_cell_count; i++)
            {
                printf("    Cell %d: %d mV (threshold: %d mV)\r\n",
                       (int)(handle->uv_cells[i] + 1U),
                       (int)handle->uv_values[i],
                       (int)BMS_UV_THRESHOLD_MV);
            }
        }

        if ((handle->fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE)
        {
            printf("  OVERTEMPERATURE: ");
            print_temp(handle->ot_value);
            printf(" (threshold: ");
            print_temp(BMS_OT_THRESHOLD_TENTHS);
            printf(")\r\n");
        }

        printf("  FET States: ");
        if ((handle->fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE)
        {
            printf("CHG=OFF, DSG=OFF (overtemperature)\r\n");
        }
        else
        {
            printf("CHG=%s, DSG=%s\r\n",
                   ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE) ? "OFF" : "ON",
                   ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE) ? "OFF" : "ON");
        }
    }
}

/*============================================================================*/
/*                              Public Functions                              */
/*============================================================================*/

bool bms_init(bms_handle_t *handle, I2C_HandleTypeDef *hi2c)
{
    bool success = false;

    if (handle != NULL && hi2c != NULL)
    {
        if (bq76920_init(&handle->bq, hi2c))
        {
            if (bq76920_enable_adc(&handle->bq))
            {
                /* Select die temperature as initial source.
                 * Failure here means we cannot confirm temperature source —
                 * thermal protection may be unreliable, so fail init. */
                if (bq76920_select_temperature_source(&handle->bq, false))
                {
                    handle->temp_source = BMS_TEMP_DIE;

                    /* Enable FETs for normal operation */
                    if (bq76920_enable_charge_fet(&handle->bq) &&
                        bq76920_enable_discharge_fet(&handle->bq))
                    {
                        /* Initialise all runtime fields */
                        handle->state                = BMS_STATE_INIT;
                        handle->last_measurement_ms  = 0U;
                        handle->last_print_ms        = 0U;
                        handle->i2c_error_count      = 0U;
                        handle->fault_mask           = BMS_FAULT_NONE;

                        clear_fault_tracking(handle);

                        success = true;
                    }
                }
            }
        }
    }

    return success;
}

void bms_run(bms_handle_t *handle)
{
    uint32_t current_ms = 0U;

    if (handle != NULL)
    {
        current_ms = HAL_GetTick();

        switch (handle->state)
        {
            case BMS_STATE_INIT:
                printf("BMS: Initialisation complete. ADC gain: %d uV/LSB, offset: %d mV\r\n",
                       (int)handle->bq.adc_gain, (int)handle->bq.adc_offset);
                handle->state               = BMS_STATE_IDLE;
                handle->last_measurement_ms = current_ms;
                break;

            case BMS_STATE_IDLE:
                if ((current_ms - handle->last_measurement_ms) >= BMS_MEASUREMENT_INTERVAL_MS)
                {
                    handle->state = BMS_STATE_MEASURE;
                }
                break;

            case BMS_STATE_MEASURE:
                if (read_all_measurements(handle))
                {
                    handle->i2c_error_count = 0U;
                    handle->state           = BMS_STATE_CHECK_FAULTS;
                }
                else
                {
                    handle->i2c_error_count++;
                    printf("BMS: I2C read failed (consecutive errors: %d)\r\n",
                           (int)handle->i2c_error_count);

                    if (handle->i2c_error_count >= BMS_MAX_I2C_ERRORS)
                    {
                        printf("BMS: I2C bus persistently unavailable — entering error state\r\n");
                    }

                    handle->state               = BMS_STATE_ERROR;
                    handle->last_measurement_ms = current_ms;
                }
                break;

            case BMS_STATE_CHECK_FAULTS:
                check_faults(handle);

                if (handle->fault_mask != BMS_FAULT_NONE)
                {
                    print_faults(handle);
                }

                handle->state               = BMS_STATE_IDLE;
                handle->last_measurement_ms = current_ms;
                break;

            case BMS_STATE_ERROR:
                if ((current_ms - handle->last_measurement_ms) >= BMS_ERROR_RECOVERY_MS)
                {
                    printf("BMS: Attempting I2C recovery...\r\n");
                    handle->state               = BMS_STATE_IDLE;
                    handle->last_measurement_ms = current_ms;
                }
                break;

            default:
                /* Defensive: unknown state — reset to IDLE */
                handle->state = BMS_STATE_IDLE;
                break;
        }
    }
}

void bms_print_status(const bms_handle_t *handle)
{
    uint8_t i = 0U;
    uint8_t j = 0U;
    bool    fault_marked = false;

    if (handle != NULL)
    {
        printf("========================================\r\n");
        printf("BMS Status\r\n");
        printf("========================================\r\n");

        /* Operating state */
        printf("State: ");
        switch (handle->state)
        {
            case BMS_STATE_INIT:         printf("INIT\r\n");          break;
            case BMS_STATE_IDLE:         printf("IDLE\r\n");          break;
            case BMS_STATE_MEASURE:      printf("MEASURE\r\n");       break;
            case BMS_STATE_CHECK_FAULTS: printf("CHECK_FAULTS\r\n");  break;
            case BMS_STATE_ERROR:        printf("ERROR\r\n");         break;
            default:                     printf("UNKNOWN\r\n");       break;
        }

        /* I2C error counter */
        if (handle->i2c_error_count > 0U)
        {
            printf("I2C errors (consecutive): %d\r\n", (int)handle->i2c_error_count);
        }

        /* Fault summary */
        if (handle->fault_mask == BMS_FAULT_NONE)
        {
            printf("Faults: NONE\r\n");
        }
        else
        {
            printf("Faults:");
            if ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE) { printf(" OV"); }
            if ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE) { printf(" UV"); }
            if ((handle->fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE) { printf(" OT"); }
            printf("\r\n");

            if ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE)
            {
                printf("  Overvoltage cells: %d\r\n", (int)handle->ov_cell_count);
                for (i = 0U; i < handle->ov_cell_count; i++)
                {
                    printf("    Cell %d: %d mV\r\n",
                           (int)(handle->ov_cells[i] + 1U),
                           (int)handle->ov_values[i]);
                }
            }

            if ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE)
            {
                printf("  Undervoltage cells: %d\r\n", (int)handle->uv_cell_count);
                for (i = 0U; i < handle->uv_cell_count; i++)
                {
                    printf("    Cell %d: %d mV\r\n",
                           (int)(handle->uv_cells[i] + 1U),
                           (int)handle->uv_values[i]);
                }
            }

            if ((handle->fault_mask & BMS_FAULT_OT) != BMS_FAULT_NONE)
            {
                printf("  Overtemperature: ");
                print_temp(handle->ot_value);
                printf("\r\n");
            }
        }

        /* Cell voltages with inline fault markers */
        printf("\r\nCell Voltages:\r\n");
        for (i = 0U; i < BMS_MAX_CELLS; i++)
        {
            printf("  Cell %d: %4d mV", (int)(i + 1U), (int)handle->bq.cell_mV[i]);

            fault_marked = false;

            if (!fault_marked && ((handle->fault_mask & BMS_FAULT_OV) != BMS_FAULT_NONE))
            {
                for (j = 0U; (j < handle->ov_cell_count) && !fault_marked; j++)
                {
                    if (handle->ov_cells[j] == i)
                    {
                        printf(" *OV");
                        fault_marked = true;
                    }
                }
            }

            if (!fault_marked && ((handle->fault_mask & BMS_FAULT_UV) != BMS_FAULT_NONE))
            {
                for (j = 0U; (j < handle->uv_cell_count) && !fault_marked; j++)
                {
                    if (handle->uv_cells[j] == i)
                    {
                        printf(" *UV");
                        fault_marked = true;
                    }
                }
            }

            printf("\r\n");
        }

        /* Pack voltage */
        printf("\r\nPack:  %d mV\r\n", (int)handle->bq.pack_mV);

        /* Temperature */
        printf("Temp:  ");
        print_temp(handle->bq.temp_tenths);
        printf(" (%s)\r\n",
               (handle->temp_source == BMS_TEMP_DIE) ? "Die" : "NTC");

        printf("========================================\r\n");
    }
}
