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
#include "stm32f1xx_hal.h"

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
#define FPWMA_Pin GPIO_PIN_0
#define FPWMA_GPIO_Port GPIOA
#define FPWMB_Pin GPIO_PIN_1
#define FPWMB_GPIO_Port GPIOA
#define FAIN1_Pin GPIO_PIN_2
#define FAIN1_GPIO_Port GPIOA
#define FAIN2_Pin GPIO_PIN_3
#define FAIN2_GPIO_Port GPIOA
#define FBIN1_Pin GPIO_PIN_4
#define FBIN1_GPIO_Port GPIOA
#define FBIN2_Pin GPIO_PIN_5
#define FBIN2_GPIO_Port GPIOA
#define FSTBY_Pin GPIO_PIN_6
#define FSTBY_GPIO_Port GPIOA
#define sg90_Pin GPIO_PIN_7
#define sg90_GPIO_Port GPIOA
#define Trig_Pin GPIO_PIN_0
#define Trig_GPIO_Port GPIOB
#define RSTBY_Pin GPIO_PIN_1
#define RSTBY_GPIO_Port GPIOB
#define RPWMA_Pin GPIO_PIN_10
#define RPWMA_GPIO_Port GPIOB
#define RPWMB_Pin GPIO_PIN_11
#define RPWMB_GPIO_Port GPIOB
#define RAIN1_Pin GPIO_PIN_12
#define RAIN1_GPIO_Port GPIOB
#define RAIN2_Pin GPIO_PIN_13
#define RAIN2_GPIO_Port GPIOB
#define RBIN1_Pin GPIO_PIN_14
#define RBIN1_GPIO_Port GPIOB
#define RBIN2_Pin GPIO_PIN_15
#define RBIN2_GPIO_Port GPIOB
#define Echo_Pin GPIO_PIN_8
#define Echo_GPIO_Port GPIOA
#define AD0_Pin GPIO_PIN_3
#define AD0_GPIO_Port GPIOB
#define AD1_Pin GPIO_PIN_4
#define AD1_GPIO_Port GPIOB
#define AD2_Pin GPIO_PIN_5
#define AD2_GPIO_Port GPIOB
#define OUT_Pin GPIO_PIN_6
#define OUT_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_8
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_9
#define SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
