/**
 * @file ut_volt_001.c
 * @brief Unit test for BQ76920 voltage conversion
 * @author Cameron Burnett
 * @date 2026-05-15
 * 
 * Compilation: gcc ut_volt_001.c -lm -o ut_volt_001
 * Run: ./ut_volt_001
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*============================================================================*/
/*                              Test Constants                                */
/*============================================================================*/

#define BQ76920_ADC_GAIN_NOMINAL_UV_PER_LSB    382U   /* 382 uV/LSB nominal */
#define BQ76920_VOLTAGE_DIVISOR                1000U  /* Convert uV to mV */

/* Default fallback values if GAIN/OFFSET cannot be read */
#define BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB    382U
#define BQ76920_DEFAULT_ADC_OFFSET_MV          0

/* Test tolerance in mV (for integer math rounding) */
#define TEST_TOLERANCE_MV                      1U

/*============================================================================*/
/*                              Test Functions                                */
/*============================================================================*/

/**
 * @brief Convert raw ADC counts to millivolts
 * @param counts Raw ADC count (16-bit)
 * @param adc_gain ADC gain in uV/LSB (from EEPROM, typically 365-396)
 * @param adc_offset ADC offset in mV (from EEPROM, -128 to +127)
 * @return Voltage in millivolts
 */
static uint16_t counts_to_millivolts(uint16_t counts, uint16_t adc_gain, int8_t adc_offset)
{
    int32_t voltage_mv;
    
    /* V = GAIN × counts + OFFSET */
    /* GAIN is in uV/LSB, so divide by 1000 to get mV */
    voltage_mv = ((int32_t)counts * (int32_t)adc_gain) / 1000L;
    voltage_mv += (int32_t)adc_offset;
    
    return (uint16_t)voltage_mv;
}

/*============================================================================*/
/*                              Test Vectors                                  */
/*============================================================================*/

/**
 * Test vector structure
 */
typedef struct {
    uint16_t counts;           /* Raw ADC counts */
    uint16_t adc_gain;         /* ADC gain in uV/LSB */
    int8_t adc_offset;         /* ADC offset in mV */
    uint16_t expected_mv;      /* Expected voltage in mV */
    uint16_t tolerance_mv;     /* Allowed tolerance */
    const char* description;   /* Test description */
} test_vector_t;

/**
 * Test vectors derived from:
 * - Datasheet examples
 * - Actual hardware readings with GAIN=377, OFFSET=46
 * - Nominal values with GAIN=382, OFFSET=0
 * 
 * Tolerance: ±5 mV for nominal tests (accounts for integer math rounding)
 * Tolerance: ±1 mV for hardware tests (exact values)
 */
static const test_vector_t test_vectors[] = {
    {
        .counts = 0,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 0,
        .tolerance_mv = 1,
        .description = "Zero input test"
    },
    {
        .counts = 2621,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 1000,
        .tolerance_mv = 5,
        .description = "Nominal: 2621 counts -> 1000 mV (±5 mV tolerance for integer math)"
    },
    {
        .counts = 5242,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 2000,
        .tolerance_mv = 5,
        .description = "Nominal: 5242 counts -> 2000 mV (±5 mV tolerance)"
    },
    {
        .counts = 7863,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 3000,
        .tolerance_mv = 5,
        .description = "Nominal: 7863 counts -> 3000 mV (±5 mV tolerance)"
    },
    {
        .counts = 10484,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 4000,
        .tolerance_mv = 5,
        .description = "Nominal: 10484 counts -> 4000 mV (±5 mV tolerance)"
    },
    {
        .counts = 13107,
        .adc_gain = BQ76920_DEFAULT_ADC_GAIN_UV_PER_LSB,
        .adc_offset = BQ76920_DEFAULT_ADC_OFFSET_MV,
        .expected_mv = 5000,
        .tolerance_mv = 10,
        .description = "Nominal: 13107 counts -> 5000 mV (±10 mV tolerance, exceeds normal Li-ion range)"
    },
    {
        .counts = 9459,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3612,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 1 (9459 counts -> 3612 mV)"
    },
    {
        .counts = 9423,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3598,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 2 (9423 counts -> 3598 mV)"
    },
    {
        .counts = 9434,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3602,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 3 (9434 counts -> 3602 mV)"
    },
    {
        .counts = 9446,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3607,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 1 session 2 (9446 counts -> 3607 mV)"
    },
    {
        .counts = 9410,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3593,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 2 session 2 (9410 counts -> 3593 mV)"
    },
    {
        .counts = 9422,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3598,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 3 session 2 (9422 counts -> 3598 mV)"
    },
    {
        .counts = 9433,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3602,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 4 session 2 (9433 counts -> 3602 mV)"
    },
    {
        .counts = 9426,
        .adc_gain = 377,
        .adc_offset = 46,
        .expected_mv = 3599,
        .tolerance_mv = 1,
        .description = "Actual hardware: Cell 5 session 2 (9426 counts -> 3599 mV)"
    }
};

/*============================================================================*/
/*                              Main Test Entry                               */
/*============================================================================*/

int main(void)
{
    int passed = 0;
    int failed = 0;
    int num_tests = sizeof(test_vectors) / sizeof(test_vectors[0]);
    
    printf("--------------------------------------------------\n");
    printf("UT-VOLT-001: Voltage Conversion Unit Test\n");
    printf("--------------------------------------------------\n\n");
    
    for (int i = 0; i < num_tests; i++)
    {
        const test_vector_t* tv = &test_vectors[i];
        uint16_t result = counts_to_millivolts(tv->counts, tv->adc_gain, tv->adc_offset);
        int32_t diff = (int32_t)result - (int32_t)tv->expected_mv;
        if (diff < 0) diff = -diff;
        
        printf("Test %d: %s\n", i + 1, tv->description);
        printf("  Input: counts=%5u, gain=%3u uV/LSB, offset=%4d mV\n",
               tv->counts, tv->adc_gain, tv->adc_offset);
        printf("  Result: %4u mV, Expected: %4u mV, Diff: %d mV\n",
               result, tv->expected_mv, (int)diff);
        
        if (diff <= tv->tolerance_mv)
        {
            printf("  Result: PASS\n\n");
            passed++;
        }
        else
        {
            printf("  Result: FAIL (exceeds tolerance of %u mV)\n\n", tv->tolerance_mv);
            failed++;
        }
    }
    
    printf("--------------------------------------------------\n");
    printf("Test Summary: %d passed, %d failed\n", passed, failed);
    printf("--------------------------------------------------\n");
    
    return (failed == 0) ? 0 : 1;
}