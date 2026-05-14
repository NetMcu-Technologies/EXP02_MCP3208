/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCP3208_CS_Pin GPIO_PIN_6
#define MCP3208_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* ---- [USER-CODE] Applikations-Konstanten --------------------------------- */

/**
 * @brief  UART-Baudrate für Debug-Output über ST-Link Virtual COM Port.
 *         Muss mit der Einstellung im PC-Terminal (z. B. PuTTY) übereinstimmen.
 */
#define APP_UART_BAUDRATE        115200U

/**
 * @brief  Wartezeit zwischen zwei Messzyklen in Millisekunden.
 *         500 ms = 2 Messungen pro Sekunde.
 */
#define APP_MEASURE_INTERVAL_MS  500U

/**
 * @brief  Referenzspannung des STM32-internen ADC in Volt.
 *         Auf dem Nucleo-Board liegt VDDA typisch bei 3,3 V.
 */
#define APP_ADC_VREF_V           3.3f

/**
 * @brief  Auflösung des internen STM32 ADC (12-bit → 4095 Stufen).
 */
#define APP_ADC_MAX_VALUE        4095U

/* USER CODE END Private defines */

/* ---- [CUBEMX] Exportierte Funktionen ------------------------------------- */
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
