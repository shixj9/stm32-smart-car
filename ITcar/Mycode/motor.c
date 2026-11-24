#include "motor.h"
#include "tim.h"

/**
  * @brief  无符号速度限幅 (0 ~ MOTOR_PWM_MAX)，防止负值写入 CCR 寄存器导致满占空比溢出
  */
static uint16_t Speed_Clamp_Unsigned(int speed)
{
    if (speed > MOTOR_PWM_MAX) return MOTOR_PWM_MAX;
    if (speed < 0)             return 0;
    return (uint16_t)speed;
}

/**
  * @brief  有符号差速限幅 (-MOTOR_PWM_MAX ~ MOTOR_PWM_MAX)
  */
static int Speed_Clamp_Signed(int speed)
{
    if (speed > MOTOR_PWM_MAX)  return MOTOR_PWM_MAX;
    if (speed < -MOTOR_PWM_MAX) return -MOTOR_PWM_MAX;
    return speed;
}

/**
  * @brief  初始化前后两块 TB6612 驱动板并启动 TIM2 四路 PWM
  */
void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // PA0  -> 前左轮 PWMA
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); // PA1  -> 前右轮 PWMB
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // PB10 -> 后左轮 PWMA (TIM2 部分重映射)
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); // PB11 -> 后右轮 PWMB (TIM2 部分重映射)

    STBY1_ON();
    STBY2_ON();

    Motor_Stop();
}

/**
  * @brief  四轮同步前进
  * @note   前后轮物理接线极性镜像：前轮正转为 IN1=1/IN2=0，后轮正转为 IN1=0/IN2=1
  * @param  speed: 前进速度 (0 ~ 999)
  */
void Motor_Forward(int speed)
{
    uint16_t pwm = Speed_Clamp_Unsigned(speed);

    AIN1_ON();  AIN2_OFF(); // 前左轮前进
    BIN1_ON();  BIN2_OFF(); // 前右轮前进
    AIN3_OFF(); AIN4_ON();  // 后左轮前进
    BIN3_OFF(); BIN4_ON();  // 后右轮前进

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm);
}

/**
  * @brief  左右轮独立差速控制（支持正数前进、负数反转）
  * @param  left_speed:  左侧前后轮速度 (-999 ~ 999)
  * @param  right_speed: 右侧前后轮速度 (-999 ~ 999)
  */
void Motor_Forward_Diff(int left_speed, int right_speed)
{
    left_speed  = Speed_Clamp_Signed(left_speed);
    right_speed = Speed_Clamp_Signed(right_speed);

    if (left_speed >= 0)
    {
        AIN1_ON();  AIN2_OFF();
        AIN3_OFF(); AIN4_ON();
    }
    else
    {
        AIN1_OFF(); AIN2_ON();
        AIN3_ON();  AIN4_OFF();
        left_speed = -left_speed;
    }

    if (right_speed >= 0)
    {
        BIN1_ON();  BIN2_OFF();
        BIN3_OFF(); BIN4_ON();
    }
    else
    {
        BIN1_OFF(); BIN2_ON();
        BIN3_ON();  BIN4_OFF();
        right_speed = -right_speed;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint16_t)left_speed);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)right_speed);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)left_speed);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, (uint16_t)right_speed);
}

/**
  * @brief  四轮同步后退
  * @param  speed: 后退速度 (0 ~ 999)
  */
void Motor_Backward(int speed)
{
    uint16_t pwm = Speed_Clamp_Unsigned(speed);

    AIN1_OFF(); AIN2_ON();  // 前左轮后退
    BIN1_OFF(); BIN2_ON();  // 前右轮后退
    AIN3_ON();  AIN4_OFF(); // 后左轮后退
    BIN3_ON();  BIN4_OFF(); // 后右轮后退

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm);
}

/**
  * @brief  原地差速左转（左轮后退，右轮前进）
  * @param  speed: 转向速度 (0 ~ 999)
  */
void Motor_Left(int speed)
{
    uint16_t pwm = Speed_Clamp_Unsigned(speed);

    AIN1_OFF(); AIN2_ON();  // 前左轮后退
    BIN1_ON();  BIN2_OFF(); // 前右轮前进
    AIN3_ON();  AIN4_OFF(); // 后左轮后退
    BIN3_OFF(); BIN4_ON();  // 后右轮前进

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm);
}

/**
  * @brief  原地差速右转（左轮前进，右轮后退）
  * @param  speed: 转向速度 (0 ~ 999)
  */
void Motor_Right(int speed)
{
    uint16_t pwm = Speed_Clamp_Unsigned(speed);

    AIN1_ON();  AIN2_OFF(); // 前左轮前进
    BIN1_OFF(); BIN2_ON();  // 前右轮后退
    AIN3_OFF(); AIN4_ON();  // 后左轮前进
    BIN3_ON();  BIN4_OFF(); // 后右轮后退

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm);
}

/**
  * @brief  停止电机输出
  */
void Motor_Stop(void)
{
    AIN1_OFF(); AIN2_OFF();
    BIN1_OFF(); BIN2_OFF();
    AIN3_OFF(); AIN4_OFF();
    BIN3_OFF(); BIN4_OFF();

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
}
