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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "bluetooth.h"
#include "mpu6050.h"
#include "hc_sr04.h"
#include "oled.h"
#include "track.h"
#include "mode_remote.h"
#include "mode_obstacle.h"
#include <stdio.h>
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  Motor_Init();      // 初始化前后 TB6612 电机驱动及 TIM2 四路 PWM
  Track_Init();      // 初始化八路灰度循迹地址引脚
  Bluetooth_Init();  // 开启 USART1 蓝牙单字节接收中断

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);   // 启动 SG90 云台舵机 PWM (PA7, 50Hz)
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1); // 启动 HC-SR04 Echo 输入捕获中断 (PA8)

  uint32_t oled_tick = 0;      // OLED 刷新分频计数 (20 * 10ms = 200ms)
  uint32_t sr04_tick = 0;      // 超声波后台刷新分频计数 (10 * 10ms = 100ms)
  uint8_t mpu_status = 1;
  float distance = -1.0f;
  char oled_buf[20];

  HAL_Delay(1000);             // 上电静置等待，确保 MPU6050 零偏校准时车身稳定

  OLED_Init();
  mpu_status = MPU6050_Init();
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 150); // 云台舵机回正 (1.5ms)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 0. MPU6050 偏航角实时积分更新 */
    if (mpu_status == 0)
    {
      MPU6050_Update_Angle();
    }

    /* 1. 核心模式状态机 */
    switch (Car_Mode)
    {
      case CAR_MODE_STANDBY:  // 0: 待机模式
        Motor_Stop();
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 150);
        break;

      case CAR_MODE_REMOTE:   // 1: 蓝牙遥控模式
        Mode_Remote_Process(Bluetooth_Cmd);
        break;

      case CAR_MODE_OBSTACLE: // 2: 超声波 + 陀螺仪闭环避障模式
        Mode_Obstacle_Process();
        break;

      case CAR_MODE_TRACK:    // 3: 八路灰度 PD 自主循迹模式
        Mode_Track_Process();
        break;

      default:
        Motor_Stop();
        break;
    }

    /* 2. 非阻塞后台超声波测距（非避障模式下每 100ms 触发一次，不阻塞循迹控制周期） */
    sr04_tick++;
    if (sr04_tick >= 10)
    {
      sr04_tick = 0;
      if (Car_Mode != CAR_MODE_OBSTACLE)
      {
        HCSR04_UpdateAsync();
      }
      distance = HCSR04_GetLastDistance();
    }

    /* 3. OLED 状态面板刷新（每 200ms 刷新一次） */
    oled_tick++;
    if (oled_tick >= 20)
    {
      oled_tick = 0;
      distance = HCSR04_GetLastDistance();

      // 非循迹模式下主动采样一次八路灰度，便于调试观察 X1~X8 实时状态
      if (Car_Mode != CAR_MODE_TRACK)
      {
        Track_ReadSensors(Track_Show);
      }

      /*
       * Track_Action (A:x) 状态码说明：
       *   0 = 初始化 / 待机
       *   1 = PD 闭环循迹
       *   2 = 左直角弯锁定转向
       *   3 = 右直角弯锁定转向
       *   4 = 脱离黑线保护停车
       */
      // 第 1 行：循迹动作 A + 当前模式 M + X1/X2
      OLED_ShowString(0, 0, "A:", 16);
      OLED_ShowNum(16, 0, Track_Action, 1, 16);
      OLED_ShowString(32, 0, "M:", 16);
      OLED_ShowNum(48, 0, Car_Mode, 1, 16);
      OLED_ShowString(80, 0, "X12", 16);
      OLED_ShowNum(112, 0, Track_Show[0], 1, 16);
      OLED_ShowNum(120, 0, Track_Show[1], 1, 16);

      // 第 2 行：工作模式名称 + X3/X4
      if (Car_Mode == CAR_MODE_STANDBY)
      {
        OLED_ShowString(0, 2, "Standby     ", 16);
      }
      else if (Car_Mode == CAR_MODE_REMOTE)
      {
        OLED_ShowString(0, 2, "Remote      ", 16);
      }
      else if (Car_Mode == CAR_MODE_OBSTACLE)
      {
        OLED_ShowString(0, 2, "Obstacle    ", 16);
      }
      else if (Car_Mode == CAR_MODE_TRACK)
      {
        OLED_ShowString(0, 2, "Tracking    ", 16);
      }
      else
      {
        OLED_ShowString(0, 2, "Unknown     ", 16);
      }
      OLED_ShowString(80, 2, "X34", 16);
      OLED_ShowNum(112, 2, Track_Show[2], 1, 16);
      OLED_ShowNum(120, 2, Track_Show[3], 1, 16);

      // 第 3 行：偏航角 Yaw + X5/X6
      if (mpu_status == 0)
      {
        snprintf(oled_buf, sizeof(oled_buf), "Y:%5.1f   ", Yaw_Angle);
        OLED_ShowString(0, 4, oled_buf, 16);
      }
      else
      {
        OLED_ShowString(0, 4, "Y:MPU_ERR  ", 16);
      }
      OLED_ShowString(80, 4, "X56", 16);
      OLED_ShowNum(112, 4, Track_Show[4], 1, 16);
      OLED_ShowNum(120, 4, Track_Show[5], 1, 16);

      // 第 4 行：超声波距离 D + X7/X8
      if (distance < 0.0f)
      {
        OLED_ShowString(0, 6, "D:ERR      ", 16);
      }
      else
      {
        snprintf(oled_buf, sizeof(oled_buf), "D:%5.1fcm ", distance);
        OLED_ShowString(0, 6, oled_buf, 16);
      }
      OLED_ShowString(80, 6, "X78", 16);
      OLED_ShowNum(112, 6, Track_Show[6], 1, 16);
      OLED_ShowNum(120, 6, Track_Show[7], 1, 16);
    }

    HAL_Delay(10); // 主循环 10ms 基准节拍
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
