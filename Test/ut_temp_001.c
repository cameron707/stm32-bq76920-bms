/**
 * @file ut_temp_001.c
 * @brief Unit test for BQ76920 temperature conversion
 * @author Cameron Burnett
 * @date 2026-05-15
 * 
 * Compilation: gcc ut_temp_001.c -lm -o ut_temp_001
 * Run: ./ut_temp_001
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/*============================================================================*/
/*                              Test Constants                                */
/*============================================================================*/

#define BQ76920_ADC_GAIN_UV_PER_LSB    382U   /**< 382 uV/LSB */
#define BQ76920_V25_MV                 1200   /**< 1.200V = 1200 mV */
#define BQ76920_TEMP_COEFF_MV_PER_C    4.2    /**< 4.2 mV/C */

/*============================================================================*/
/*                              Helper Functions                              */
/*============================================================================*/

static int16_t abs_tenths(int16_t value)
{
    return (value < 0) ? -value : value;
}

/*============================================================================*/
/*                              Test Functions                                */
/*============================================================================*/

/**
 * @brief Convert raw ADC counts to temperature (external NTC)
 * @param raw Raw ADC counts from TS1 register
 * @return Temperature in 0.1C units
 */
static int16_t external_ntc_to_temperature(uint16_t raw)
{
    /* VTSX = counts × 382 uV/LSB = counts × 0.000382 V */
    double vtsx = (double)raw * 0.000382;
    
    /* RTS = (10000 × VTSX) ÷ (3.3 – VTSX) */
    double rts = (10000.0 * vtsx) / (3.3 - vtsx);
    
    /* Steinhart-Hart for 10k NTC 103AT thermistor */
    /* Coefficients: A = 1.129241e-3, B = 2.341077e-4, C = 8.77541e-8 */
    double ln_r = log(rts);
    double inv_t = 1.129241e-3 + (2.341077e-4 * ln_r) + (8.77541e-8 * ln_r * ln_r * ln_r);
    double temp_c = (1.0 / inv_t) - 273.15;
    
    return (int16_t)(temp_c * 10.0);
}

/**
 * @brief Convert raw ADC counts to temperature (die temperature)
 * @param raw Raw ADC counts from TS1 register
 * @return Temperature in 0.1C units
 */
static int16_t die_temperature_to_tenths(uint16_t raw)
{
    /* VTSX = counts × 382 uV/LSB = counts × 0.382 mV */
    int32_t vtsx_mv = ((int32_t)raw * BQ76920_ADC_GAIN_UV_PER_LSB) / 1000;
    
    /* TEMPDIE = 25 – ((VTSX – V25) ÷ 0.0042) */
    double temp_c = 25.0 - ((double)(vtsx_mv - BQ76920_V25_MV) / BQ76920_TEMP_COEFF_MV_PER_C);
    
    return (int16_t)(temp_c * 10.0);
}

/*============================================================================*/
/*                              Main Test Entry                               */
/*============================================================================*/

int main(void)
{
    int passed = 0;
    int failed = 0;
    
    printf("========================================\n");
    printf("UT-TEMP-001: Temperature Conversion Unit Test\n");
    printf("========================================\n\n");
    
    /* Test 1: External NTC - Known value from actual hardware */
    printf("Test 1: External NTC conversion\n");
    printf("--------------------------------\n");
    
    /* From your actual output: raw=4072 -> 27.6C */
    int16_t result = external_ntc_to_temperature(4072);
    int16_t expected = 276;  /* 27.6C */
    
    printf("  Raw: 4072 -> Result: %d.%dC (Expected: 27.6C)\n",
           result / 10, abs_tenths(result % 10));
    
    if (result >= 270 && result <= 280)
    {
        printf("  Result: PASS (within expected range)\n\n");
        passed++;
    }
    else
    {
        printf("  Result: FAIL (out of range)\n\n");
        failed++;
    }
    
    /* Test 2: Die temperature - V25 calibration point */
    printf("Test 2: Die temperature at V25 (1.200V)\n");
    printf("----------------------------------------\n");
    
    /* V25 = 1.200V = 3141 counts (1200mV / 0.382mV) */
    result = die_temperature_to_tenths(3141);
    expected = 250;  /* 25.0C */
    
    printf("  Raw: 3141 -> Result: %d.%dC (Expected: 25.0C)\n",
           result / 10, abs_tenths(result % 10));
    
    if (result >= 248 && result <= 252)
    {
        printf("  Result: PASS\n\n");
        passed++;
    }
    else
    {
        printf("  Result: FAIL\n\n");
        failed++;
    }
    
    /* Test 3: Die temperature - typical operating range */
    printf("Test 3: Die temperature at 3400 counts\n");
    printf("-----------------------------------------------\n");
    
    result = die_temperature_to_tenths(3400);
    
    printf("  Raw: 3400 -> Result: %d.%dC\n",
           result / 10, abs_tenths(result % 10));
    
    if (result >= 10 && result <= 20)
    {
        printf("  Result: PASS\n\n");
        passed++;
    }
    else
    {
        printf("  Result: FAIL (expected ~1.3C)\n\n");
        failed++;
    }
    
    /* Summary */
    printf("========================================\n");
    printf("Test Summary: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");
    
    return (failed == 0) ? 0 : 1;
}