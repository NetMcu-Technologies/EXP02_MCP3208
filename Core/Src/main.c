/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright Diplom Ingenieurin Djamila Boethfueer Boucetta.
  * All rights reserved...............
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcp3208.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static char     uart_buf[128];
static uint32_t measure_count = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void     App_Init(void);
static void     App_MeasureAndTransmit(void);
static uint16_t App_ReadInternalADC(void);
static float    App_ConvertADCtoVoltage(uint16_t raw_value, uint16_t max_value, float vref);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t App_ReadInternalADC(void)
{
    extern ADC_HandleTypeDef hadc1;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
        return 0xFFFFU;

    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return 0xFFFFU;
    }

    uint16_t value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
}

static float App_ConvertADCtoVoltage(uint16_t raw_value, uint16_t max_value, float vref)
{
    if (max_value == 0U) return 0.0f;
    return (float)raw_value * (vref / (float)max_value);
}

static void App_PrintSeparator(void)
{
    extern UART_HandleTypeDef huart2;
    const char *sep = "----------------------------------------\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)sep, (uint16_t)strlen(sep), 100U);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(500);   /* 500ms warten bis MCP3208 stabil */
  App_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    App_MeasureAndTransmit();
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void App_Init(void)
{
    extern UART_HandleTypeDef huart2;
    extern ADC_HandleTypeDef  hadc1;

    MCP3208_Init();

    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
        Error_Handler();

    const char *welcome =
        "\r\n"
        "========================================\r\n"
        "  EXP 02: MCP3208 + Interner ADC       \r\n"
        "  Nucleo-L476RG  |  HAL-Bibliothek     \r\n"
        "========================================\r\n"
        "  Format: [N] EXT(CH0): raw / V         \r\n"
        "               INT(PA0): raw / V         \r\n"
        "               Diff: mV                  \r\n"
        "========================================\r\n\r\n";

    HAL_UART_Transmit(&huart2, (uint8_t *)welcome, (uint16_t)strlen(welcome), 1000U);
}

static void App_MeasureAndTransmit(void)
{
    extern UART_HandleTypeDef huart2;

    measure_count++;

    /* Externer ADC: MCP3208 Kanal 0 */
    MCP3208_Result_t ext_result;
    MCP3208_Status_t ext_status = MCP3208_ReadVoltage(7U, &ext_result);

    /* Interner ADC: STM32 ADC1 / PA0 */
    uint16_t int_raw  = App_ReadInternalADC();
    float    int_volt = App_ConvertADCtoVoltage(int_raw, APP_ADC_MAX_VALUE, APP_ADC_VREF_V);

    /* Differenz in Millivolt */
    float diff_mv = 0.0f;
    if (ext_status == MCP3208_OK && int_raw != 0xFFFFU)
        diff_mv = (ext_result.voltage - int_volt) * 1000.0f;

    /* UART-Ausgabe */
    int len = snprintf(uart_buf, sizeof(uart_buf),
                       "[%4lu] Messung:\r\n", (unsigned long)measure_count);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, (uint16_t)len, 100U);

    if (ext_status == MCP3208_OK)
        len = snprintf(uart_buf, sizeof(uart_buf),
                       "  EXT MCP3208 CH0 : %4u / %.4f V\r\n",
                       ext_result.raw, (double)ext_result.voltage);
    else
        len = snprintf(uart_buf, sizeof(uart_buf),
                       "  EXT MCP3208 CH0 : FEHLER\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, (uint16_t)len, 100U);

    if (int_raw != 0xFFFFU)
        len = snprintf(uart_buf, sizeof(uart_buf),
                       "  INT STM32  PA0  : %4u / %.4f V\r\n",
                       int_raw, (double)int_volt);
    else
        len = snprintf(uart_buf, sizeof(uart_buf),
                       "  INT STM32  PA0  : FEHLER\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, (uint16_t)len, 100U);

    if (ext_status == MCP3208_OK && int_raw != 0xFFFFU)
    {
        len = snprintf(uart_buf, sizeof(uart_buf),
                       "  Differenz (E-I) : %+.2f mV\r\n", (double)diff_mv);
        HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, (uint16_t)len, 100U);
    }

    App_PrintSeparator();
    HAL_Delay(APP_MEASURE_INTERVAL_MS);
}
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

#ifdef  USE_FULL_ASSERT
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
