/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "bq76920.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define USB_CONNECT_DELAY_MS        10000U  /**< Wait for USB terminal to connect */
#define TEMP_MODE_SWITCH_DELAY_MS    2000U  /**< Per datasheet: delay after temp source change */
#define ADC_STABILISE_DELAY_MS        250U  /**< Per datasheet: ADC stabilisation after enable */
#define TEMP_SAMPLE_COUNT              10   /**< Number of temperature readings per stability test */
#define TEMP_STABILITY_DIE_TOL_F      0.5f /**< Die temperature max consecutive variation (deg C) */
#define TEMP_STABILITY_NTC_TOL_F      0.3f /**< NTC temperature max consecutive variation (deg C) */
#define TEMP_SAMPLE_INTERVAL_MS       500U  /**< Delay between temperature samples */
#define MAIN_LOOP_DELAY_MS           2000U  /**< Main measurement loop period */
#define ERROR_BLINK_DELAY_MS          100U  /**< Fast LED blink period on fatal error */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  bq76920_handle_t bms = {0};
  int16_t temp_readings[TEMP_SAMPLE_COUNT];
  float max_variation = 0.0f;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  
  HAL_Delay(USB_CONNECT_DELAY_MS);
  printf("\r\n\r\n");
  printf("========================================\r\n");
  printf("BQ76920 BMS Integration Test\r\n");
  printf("========================================\r\n");
  printf("Initializing BQ76920 driver...\r\n");

  if (!bq76920_init(&bms, &hi2c1))
  {
      printf("ERROR: Driver initialization failed!\r\n");
      printf("Check I2C connections and power.\r\n");
      while (1)
      {
          HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
          HAL_Delay(TEMP_SAMPLE_INTERVAL_MS);
      }
  }
  printf("Driver initialized successfully.\r\n");
  printf("ADC Gain: %d uV/LSB, ADC Offset: %d mV\r\n", bms.adc_gain, bms.adc_offset);

  if (!bq76920_enable_adc(&bms))
  {
      printf("ERROR: ADC enable failed!\r\n");
      while (1)
      {
          HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
          HAL_Delay(ERROR_BLINK_DELAY_MS);
      }
  }
  printf("ADC enabled.\r\n");
  printf("Waiting for ADC to stabilize...\r\n");
  HAL_Delay(ADC_STABILISE_DELAY_MS);

  /* ============================================ */
  /* Temperature Stability Test - Die Temperature */
  /* ============================================ */
  printf("\r\n========================================\r\n");
  printf("Testing die temperature (TEMP_SEL=0)\r\n");
  printf("========================================\r\n");

  if (!bq76920_select_temperature_source(&bms, false))
  {
      printf("ERROR: Failed to select die temperature!\r\n");
  }
  else
  {
      HAL_Delay(TEMP_MODE_SWITCH_DELAY_MS);  /* Per datasheet: delay after mode switch */
      
      printf("Reading die temperature %d times...\r\n", TEMP_SAMPLE_COUNT);
      for (int i = 0; i < TEMP_SAMPLE_COUNT; i++)
      {
          if (bq76920_read_temperature(&bms))
          {
              temp_readings[i] = bms.temp_tenths;
              int16_t temp_c = temp_readings[i] / 10;
              int16_t temp_frac = temp_readings[i] % 10;
              if (temp_frac < 0) temp_frac = -temp_frac;
              printf("  Read %d: %d.%d C\r\n", i + 1, temp_c, temp_frac);
          }
          else
          {
              printf("  Read %d: ERROR!\r\n", i + 1);
          }
          HAL_Delay(TEMP_SAMPLE_INTERVAL_MS);
      }
      
      /* Calculate stability (maximum consecutive variation) */
      max_variation = 0.0f;
      for (int i = 1; i < TEMP_SAMPLE_COUNT; i++)
      {
          float variation = (float)(temp_readings[i] - temp_readings[i - 1]) / 10.0f;
          if (variation < 0) variation = -variation;
          if (variation > max_variation) max_variation = variation;
      }
      
      int16_t first_c = temp_readings[0] / 10;
      int16_t first_frac = temp_readings[0] % 10;
      if (first_frac < 0) first_frac = -first_frac;
      int16_t last_c = temp_readings[TEMP_SAMPLE_COUNT - 1] / 10;
      int16_t last_frac = temp_readings[TEMP_SAMPLE_COUNT - 1] % 10;
      if (last_frac < 0) last_frac = -last_frac;
      
      printf("\r\nDie Temperature Results:\r\n");
      printf("  Range: %d.%d C - %d.%d C\r\n", first_c, first_frac, last_c, last_frac);
      printf("  Max consecutive variation: %.2f C\r\n", max_variation);
      
      if (max_variation <= TEMP_STABILITY_DIE_TOL_F)
      {
          printf("  Stability: PASS (<= %.1f C)\r\n", TEMP_STABILITY_DIE_TOL_F);
      }
      else
      {
          printf("  Stability: FAIL (> %.1f C)\r\n", TEMP_STABILITY_DIE_TOL_F);
      }
  }

  /* ============================================ */
  /* Temperature Stability Test - External NTC    */
  /* ============================================ */
  printf("\r\n========================================\r\n");
  printf("Testing external NTC temperature (TEMP_SEL=1)\r\n");
  printf("========================================\r\n");

  if (!bq76920_select_temperature_source(&bms, true))
  {
      printf("ERROR: Failed to select external NTC!\r\n");
  }
  else
  {
      HAL_Delay(TEMP_MODE_SWITCH_DELAY_MS);  /* Per datasheet: delay after mode switch */
      
      printf("Reading external temperature %d times...\r\n", TEMP_SAMPLE_COUNT);
      for (int i = 0; i < TEMP_SAMPLE_COUNT; i++)
      {
          if (bq76920_read_temperature(&bms))
          {
              temp_readings[i] = bms.temp_tenths;
              int16_t temp_c = temp_readings[i] / 10;
              int16_t temp_frac = temp_readings[i] % 10;
              if (temp_frac < 0) temp_frac = -temp_frac;
              printf("  Read %d: %d.%d C\r\n", i + 1, temp_c, temp_frac);
          }
          else
          {
              printf("  Read %d: ERROR!\r\n", i + 1);
          }
          HAL_Delay(TEMP_SAMPLE_INTERVAL_MS);
      }
      
      max_variation = 0.0f;
      for (int i = 1; i < TEMP_SAMPLE_COUNT; i++)
      {
          float variation = (float)(temp_readings[i] - temp_readings[i - 1]) / 10.0f;
          if (variation < 0) variation = -variation;
          if (variation > max_variation) max_variation = variation;
      }
      
      int16_t first_c = temp_readings[0] / 10;
      int16_t first_frac = temp_readings[0] % 10;
      if (first_frac < 0) first_frac = -first_frac;
      int16_t last_c = temp_readings[TEMP_SAMPLE_COUNT - 1] / 10;
      int16_t last_frac = temp_readings[TEMP_SAMPLE_COUNT - 1] % 10;
      if (last_frac < 0) last_frac = -last_frac;
      
      printf("\r\nExternal NTC Results:\r\n");
      printf("  Range: %d.%d C - %d.%d C\r\n", first_c, first_frac, last_c, last_frac);
      printf("  Max consecutive variation: %.2f C\r\n", max_variation);

      if (max_variation <= TEMP_STABILITY_NTC_TOL_F)
      {
          printf("  Stability: PASS (<= %.1f C)\r\n", TEMP_STABILITY_NTC_TOL_F);
      }
      else
      {
          printf("  Stability: FAIL (> %.1f C)\r\n", TEMP_STABILITY_NTC_TOL_F);
      }
  }

  /* Switch back to die temperature for normal operation */
  //bq76920_select_temperature_source(&bms, false);
  
  printf("\r\n========================================\r\n");
  printf("Starting continuous voltage measurements...\r\n");
  printf("========================================\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (bq76920_read_all_voltages(&bms))
    {
        printf("Cell 1: %4d mV  Cell 2: %4d mV  Cell 3: %4d mV Cell 4: %4d mV Cell 5: %4d mV\r\n",
                bms.cell_mV[0], bms.cell_mV[1], bms.cell_mV[2], bms.cell_mV[3], bms.cell_mV[4]);
    }
    else
    {
        printf("ERROR: Failed to read cell voltages!\r\n");
    }

    if (bq76920_read_pack_voltage(&bms))
    {
        printf("Pack:  %4d mV\r\n", bms.pack_mV);
    }
    else
    {
        printf("ERROR: Failed to read pack voltage!\r\n");
    }

    if (bq76920_read_temperature(&bms))
    {
        int16_t temp_c = bms.temp_tenths / 10;
        int16_t temp_frac = bms.temp_tenths % 10;
        if (temp_frac < 0) temp_frac = -temp_frac;
        printf("Temp:  %d.%d C\r\n", temp_c, temp_frac);
    }
    else
    {
        printf("ERROR: Failed to read temperature!\r\n");
    }

    printf("----------------------------------------\r\n");
    HAL_Delay(MAIN_LOOP_DELAY_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */