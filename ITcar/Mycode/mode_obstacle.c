#include "mode_obstacle.h"
#include "hc_sr04.h"
#include "motor.h"
#include "tim.h"
#include "mpu6050.h"
#include "bluetooth.h"

#define SERVO_RIGHT_CCR   50   // 0.5ms -> 云台向右看
#define SERVO_CENTER_CCR  150  // 1.5ms -> 云台回正看前方
#define SERVO_LEFT_CCR    250  // 2.5ms -> 云台向左看

/**
  * @brief  可被蓝牙模式切换指令随时打断的分片延时
  * @retval 1: 仍处于避障模式, 0: 已被外部切换模式需立即中止
  */
static uint8_t Obstacle_WaitMs(uint32_t ms)
{
    uint32_t elapsed = 0;
    while (elapsed < ms)
    {
        if (Car_Mode != CAR_MODE_OBSTACLE)
        {
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, SERVO_CENTER_CCR);
            Motor_Stop();
            return 0;
        }
        uint32_t step = (ms - elapsed >= 20) ? 20 : (ms - elapsed);
        HAL_Delay(step);
        elapsed += step;
    }
    return (Car_Mode == CAR_MODE_OBSTACLE);
}

/**
  * @brief  SG90 云台超声波探路 + MPU6050 陀螺仪闭环 90° 自主避障逻辑
  */
void Mode_Obstacle_Process(void)
{
    float dist = HCSR04_GetDistance();
    if (Car_Mode != CAR_MODE_OBSTACLE)
    {
        Motor_Stop();
        return;
    }

    // 前方危险区间 (2cm ~ 25cm) 触发停车扫描与闭环转向
    if (dist > 2.0f && dist < 25.0f)
    {
        Motor_Stop();
        if (!Obstacle_WaitMs(200)) return;

        // 云台向右探路
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, SERVO_RIGHT_CCR);
        if (!Obstacle_WaitMs(400)) return;
        float r_dist = HCSR04_GetDistance();

        // 云台向左探路
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, SERVO_LEFT_CCR);
        if (!Obstacle_WaitMs(500)) return;
        float l_dist = HCSR04_GetDistance();

        // 云台回正
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, SERVO_CENTER_CCR);
        if (!Obstacle_WaitMs(200)) return;

        // 选择更宽敞的一侧执行 90° 闭环原地转向（超时测距返回 -1.0f 代表前方空旷无回波）
        float left_eval  = (l_dist < 0.0f) ? 999.0f : l_dist;
        float right_eval = (r_dist < 0.0f) ? 999.0f : r_dist;

        if (left_eval >= right_eval)
        {
            MPU_Turn_Left(90.0f, 450);
        }
        else
        {
            MPU_Turn_Right(90.0f, 450);
        }

        Obstacle_WaitMs(100);
    }
    else
    {
        Motor_Forward(450);
    }
}
