#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* ==================== 电机最大占空比 (对应 TIM2 ARR = 999) ==================== */
#define MOTOR_PWM_MAX  999

/* ==================== 前轮 TB6612 驱动板引脚宏 ==================== */
// 前左轮方向 (PA2 / PA3)
#define AIN1_ON()   HAL_GPIO_WritePin(FAIN1_GPIO_Port, FAIN1_Pin, GPIO_PIN_SET)
#define AIN1_OFF()  HAL_GPIO_WritePin(FAIN1_GPIO_Port, FAIN1_Pin, GPIO_PIN_RESET)
#define AIN2_ON()   HAL_GPIO_WritePin(FAIN2_GPIO_Port, FAIN2_Pin, GPIO_PIN_SET)
#define AIN2_OFF()  HAL_GPIO_WritePin(FAIN2_GPIO_Port, FAIN2_Pin, GPIO_PIN_RESET)

// 前右轮方向 (PA4 / PA5)
#define BIN1_ON()   HAL_GPIO_WritePin(FBIN1_GPIO_Port, FBIN1_Pin, GPIO_PIN_SET)
#define BIN1_OFF()  HAL_GPIO_WritePin(FBIN1_GPIO_Port, FBIN1_Pin, GPIO_PIN_RESET)
#define BIN2_ON()   HAL_GPIO_WritePin(FBIN2_GPIO_Port, FBIN2_Pin, GPIO_PIN_SET)
#define BIN2_OFF()  HAL_GPIO_WritePin(FBIN2_GPIO_Port, FBIN2_Pin, GPIO_PIN_RESET)

// 前驱动板 STBY 使能 (PA6)
#define STBY1_ON()  HAL_GPIO_WritePin(FSTBY_GPIO_Port, FSTBY_Pin, GPIO_PIN_SET)
#define STBY1_OFF() HAL_GPIO_WritePin(FSTBY_GPIO_Port, FSTBY_Pin, GPIO_PIN_RESET)

/* ==================== 后轮 TB6612 驱动板引脚宏 ==================== */
// 后左轮方向 (PB12 / PB13)
#define AIN3_ON()   HAL_GPIO_WritePin(RAIN1_GPIO_Port, RAIN1_Pin, GPIO_PIN_SET)
#define AIN3_OFF()  HAL_GPIO_WritePin(RAIN1_GPIO_Port, RAIN1_Pin, GPIO_PIN_RESET)
#define AIN4_ON()   HAL_GPIO_WritePin(RAIN2_GPIO_Port, RAIN2_Pin, GPIO_PIN_SET)
#define AIN4_OFF()  HAL_GPIO_WritePin(RAIN2_GPIO_Port, RAIN2_Pin, GPIO_PIN_RESET)

// 后右轮方向 (PB14 / PB15)
#define BIN3_ON()   HAL_GPIO_WritePin(RBIN1_GPIO_Port, RBIN1_Pin, GPIO_PIN_SET)
#define BIN3_OFF()  HAL_GPIO_WritePin(RBIN1_GPIO_Port, RBIN1_Pin, GPIO_PIN_RESET)
#define BIN4_ON()   HAL_GPIO_WritePin(RBIN2_GPIO_Port, RBIN2_Pin, GPIO_PIN_SET)
#define BIN4_OFF()  HAL_GPIO_WritePin(RBIN2_GPIO_Port, RBIN2_Pin, GPIO_PIN_RESET)

// 后驱动板 STBY 使能 (PB1)
#define STBY2_ON()  HAL_GPIO_WritePin(RSTBY_GPIO_Port, RSTBY_Pin, GPIO_PIN_SET)
#define STBY2_OFF() HAL_GPIO_WritePin(RSTBY_GPIO_Port, RSTBY_Pin, GPIO_PIN_RESET)

/* ==================== 对外函数接口 ==================== */
void Motor_Init(void);
void Motor_Forward(int speed);
void Motor_Forward_Diff(int left_speed, int right_speed);
void Motor_Backward(int speed);
void Motor_Left(int speed);
void Motor_Right(int speed);
void Motor_Stop(void);

#endif /* __MOTOR_H */
