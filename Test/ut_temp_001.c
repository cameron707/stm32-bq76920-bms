/**
 * @file ut_temp_001.c
 * @brief Unit test for BQ76920 temperature conversion
 * @author Cameron Burnett
 * @date 2026-05-19
 *
 * Tests the integer-only temperature conversion algorithms used in bq76920.c:
 *   - External NTC: integer voltage divider + LUT with linear interpolation
 *   - Die temperature: integer fixed-point formula
 *
 * No floating point, no math.h — matches the production driver exactly.
 *
 * Compilation: gcc ut_temp_001.c -o ut_temp_001
 * Run:         ./ut_temp_001
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/*                    NTC LUT (copied from bq76920.c)                         */
/*                                                                            */
/* Source of truth is bq76920.c. If the LUT changes there, update here too.  */
/*============================================================================*/

typedef struct
{
    uint32_t resistance_ohms;
    int16_t  temp_tenths;
} ntc_lut_entry_t;

static const ntc_lut_entry_t kNtcLut[] =
{
    { 100000U, -205 },  { 84000U, -175 },  { 70600U, -144 },
    {  59600U, -114 },  { 50600U,  -84 },  { 43200U,  -54 },
    {  37100U,  -25 },  { 32000U,    4 },  { 27700U,   33 },
    {  24100U,   61 },  { 21100U,   88 },  { 18520U,  115 },
    {  16330U,  142 },  { 14450U,  168 },  { 12840U,  194 },
    {  11440U,  220 },  { 10220U,  245 },  {  9170U,  270 },
    {   8250U,  294 },  {  7440U,  319 },  {  6720U,  343 },
    {   6080U,  367 },  {  5510U,  392 },  {  5000U,  416 },
    {   4550U,  440 },  {  4150U,  463 },  {  3790U,  487 },
    {   3470U,  510 },  {  3180U,  533 },  {  2920U,  556 },
    {   2680U,  579 },  {  2470U,  602 },  {  2280U,  624 },
    {   2100U,  648 },  {  1940U,  670 },  {  1800U,  692 },
    {   1670U,  714 },  {  1550U,  736 },  {  1440U,  758 },
    {   1340U,  780 },  {  1250U,  801 },
};

#define NTC_LUT_SIZE ((uint8_t)(sizeof(kNtcLut) / sizeof(kNtcLut[0])))

/*============================================================================*/
/*              Conversion Functions (copied from bq76920.c)                  */
/*============================================================================*/

/**
 * @brief Look up NTC temperature from resistance using LUT + linear interpolation.
 *        Copied verbatim from bq76920.c to ensure unit test matches production code.
 */
static int16_t ntc_resistance_to_temp(uint32_t rts_ohms)
{
    int16_t result = kNtcLut[NTC_LUT_SIZE - 1U].temp_tenths;
    uint8_t i = 0U;
    bool found = false;

    if (rts_ohms >= kNtcLut[0].resistance_ohms)
    {
        result = kNtcLut[0].temp_tenths;
    }
    else if (rts_ohms <= kNtcLut[NTC_LUT_SIZE - 1U].resistance_ohms)
    {
        /* already set to hottest entry */
    }
    else
    {
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

/**
 * @brief Convert raw ADC counts to NTC temperature using integer voltage divider + LUT.
 *        Mirrors the TEMP_SEL=1 path of bq76920_read_temperature().
 * @param raw   Raw 14-bit ADC counts from TS1 register
 * @param gain  ADC gain in uV/LSB (from factory EEPROM, typically 365-396)
 * @param out   Output temperature in 0.1 deg C
 * @return true if conversion valid, false if vtsx out of range
 */
static bool ntc_counts_to_tenths(uint16_t raw, uint16_t gain, int16_t *out)
{
    bool valid = false;
    int32_t vtsx_mv = ((int32_t)raw * (int32_t)gain) / 1000L;

    if ((vtsx_mv > 0L) && (vtsx_mv < 3300L))
    {
        uint32_t rts_ohms = (uint32_t)((10000L * vtsx_mv) / (3300L - vtsx_mv));
        *out = ntc_resistance_to_temp(rts_ohms);
        valid = true;
    }

    return valid;
}

/**
 * @brief Convert raw ADC counts to die temperature using integer fixed-point.
 *        Mirrors the TEMP_SEL=0 path of bq76920_read_temperature().
 *
 * Formula: T_tenths = 250 + (1200 - vtsx_mv) * 100 / 42
 * Derived from: T(C) = 25 + (V25 - VTSX) / 4.2  (BQ76920 datasheet)
 *
 * @param raw   Raw ADC counts from TS1 register
 * @param gain  ADC gain in uV/LSB
 * @return Temperature in 0.1 deg C
 */
static int16_t die_counts_to_tenths(uint16_t raw, uint16_t gain)
{
    int32_t vtsx_mv = ((int32_t)raw * (int32_t)gain) / 1000L;
    return (int16_t)(250L + ((1200L - vtsx_mv) * 100L) / 42L);
}

/*============================================================================*/
/*                              Test Runner                                   */
/*============================================================================*/

static int passed = 0;
static int failed = 0;

static void check(const char *desc, int16_t result, int16_t expected, int16_t tol)
{
    int16_t diff = result - expected;
    if (diff < 0) diff = (int16_t)-diff;

    printf("  %s\n", desc);
    printf("    Result: %d.%d C  Expected: %d.%d C  Diff: %d.%d C  Tol: +/-%d.%d C\n",
           result / 10, (result < 0 ? -result : result) % 10,
           expected / 10, (expected < 0 ? -expected : expected) % 10,
           diff / 10, diff % 10,
           tol / 10, tol % 10);

    if (diff <= tol)
    {
        printf("    PASS\n\n");
        passed++;
    }
    else
    {
        printf("    FAIL\n\n");
        failed++;
    }
}

/*============================================================================*/
/*                              Main Test Entry                               */
/*============================================================================*/

int main(void)
{
    int16_t result = 0;
    bool valid = false;

    printf("==================================================\n");
    printf("UT-TEMP-001: Temperature Conversion Unit Test\n");
    printf("==================================================\n\n");

    /* ------------------------------------------------------------------ */
    printf("--- NTC Thermistor Tests (integer LUT + interpolation) ---\n\n");
    /* ------------------------------------------------------------------ */

    /* Test 1: Exact LUT match at entry index 16 */
    result = ntc_resistance_to_temp(10220U);
    check("T1: rts=10220 ohm -> exact LUT match (24.5 C)",
          result, 245, 0);

    /* Test 2: Exact LUT match at entry index 17 */
    result = ntc_resistance_to_temp(9170U);
    check("T2: rts=9170 ohm -> exact LUT match (27.0 C)",
          result, 270, 0);

    /* Test 3: Linear interpolation between entries 17 and 18
     * rts=8710, r_hi=9170(270), r_lo=8250(294)
     * t = 270 + (294-270)*(9170-8710)/(9170-8250) = 270 + 24*460/920 = 282 */
    result = ntc_resistance_to_temp(8710U);
    check("T3: rts=8710 ohm -> interpolated (28.2 C)",
          result, 282, 2);

    /* Test 4: From actual hardware raw counts (ADC Gain=377 uV/LSB)
     * raw=4072 -> vtsx=1535mV -> rts=8696 ohm -> LUT gives 28.2 C
     * Steinhart-Hart reference gives 27.6 C; LUT accuracy within 0.6 C */
    valid = ntc_counts_to_tenths(4072U, 377U, &result);
    if (valid)
    {
        check("T4: raw=4072, gain=377 -> hardware vector (LUT: 28.2C, S-H ref: 27.6C)",
              result, 282, 10);
    }
    else
    {
        printf("  T4: FAIL (ntc_counts_to_tenths returned invalid)\n\n");
        failed++;
    }

    /* Test 5: Above LUT range -> clamp to coldest entry (-20.5 C) */
    result = ntc_resistance_to_temp(150000U);
    check("T5: rts=150000 ohm -> above range, clamp to -20.5 C",
          result, -205, 0);

    /* Test 6: Below LUT range -> clamp to hottest entry (80.1 C) */
    result = ntc_resistance_to_temp(800U);
    check("T6: rts=800 ohm -> below range, clamp to 80.1 C",
          result, 801, 0);

    /* ------------------------------------------------------------------ */
    printf("--- Die Temperature Tests (integer fixed-point) ---\n\n");
    /* ------------------------------------------------------------------ */

    /* Test 7: V25 calibration point, actual chip gain=377
     * vtsx_mv = (3183*377)/1000 = 1199mV (not exactly 1200 due to integer div)
     * result = 250 + (1200-1199)*100/42 = 252 -> 25.2 C (within 0.3 C of ideal 25.0 C) */
    result = die_counts_to_tenths(3183U, 377U);
    check("T7: raw=3183, gain=377 -> near V25 calibration (~25.0 C)",
          result, 250, 3);

    /* Test 8: Actual hardware reading (30.7 C)
     * raw=3120, gain=377 -> vtsx=1176mV -> 250+(24*100)/42 = 307 -> 30.7 C */
    result = die_counts_to_tenths(3120U, 377U);
    check("T8: raw=3120, gain=377 -> actual hardware reading (30.7 C)",
          result, 307, 0);

    /* Test 9: Colder die temperature
     * raw=3050, gain=377 -> vtsx=1149mV -> 250+(51*100)/42 = 371 -> 37.1 C */
    result = die_counts_to_tenths(3050U, 377U);
    check("T9: raw=3050, gain=377 -> colder die (37.1 C)",
          result, 371, 0);

    /* ------------------------------------------------------------------ */
    printf("==================================================\n");
    printf("Test Summary: %d passed, %d failed\n", passed, failed);
    printf("==================================================\n");

    return (failed == 0) ? 0 : 1;
}
