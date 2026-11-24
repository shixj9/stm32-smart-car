#include "track.h"
#include "motor.h"
#include <math.h>

uint8_t Track_Show[8] = {0};
uint8_t Track_Action = TRACK_ACT_IDLE;

/* PD 控制器参数与速度基准 */
static const float kp = 55.0f;
static const float kd = 15.0f;
static const int base_speed = 150;
static const int max_speed  = 500;

/* 八路灰度传感器位置权重（左负右正：X1 ~ X8） */
static const float sensor_weights[8] = {-5.0f, -3.5f, -2.0f, -0.8f, 0.8f, 2.0f, 3.5f, 5.0f};

static float last_error = 0.0f;
static uint16_t lock_timer = 0;

/**
  * @brief  复位循迹控制器内部状态（模式切换进入循迹时调用）
  */
void Track_Reset(void)
{
    last_error = 0.0f;
    lock_timer = 0;
    Track_Action = TRACK_ACT_IDLE;
}

/**
  * @brief  初始化八路灰度循迹模块地址线及状态变量
  */
void Track_Init(void)
{
    HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, GPIO_PIN_RESET); // PB3 -> AD0
    HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, GPIO_PIN_RESET); // PB4 -> AD1
    HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, GPIO_PIN_RESET); // PB5 -> AD2
    Track_Reset();
}

/**
  * @brief  通过 AD0~AD2 (PB3~PB5) 3-8 译码轮询读取 OUT (PB6) 八路灰度状态
  * @param  sensor_values: 长度为 8 的数组，依次对应 X1 ~ X8
  */
void Track_ReadSensors(uint8_t *sensor_values)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, (i & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, (i & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, (i & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        for (volatile int delay = 0; delay < 10; delay++); // 等待多路复用器通道稳定
        sensor_values[i] = (HAL_GPIO_ReadPin(OUT_GPIO_Port, OUT_Pin) == GPIO_PIN_SET) ? 1 : 0;
    }
}

/**
  * @brief  八路灰度自主循迹主处理函数（主循环 10ms 周期调用）
  */
void Mode_Track_Process(void)
{
    Track_ReadSensors(Track_Show);

    /* 1. 直角弯防误触状态锁：锁定期间维持大差速转弯，车头回正后提前解锁 */
    if (lock_timer > 0)
    {
        lock_timer--;

        // 转弯执行超过约 60ms 后，若中间传感器 X4 或 X5 重新压线则提前解锁交还 PD 微调
        if (lock_timer < 20)
        {
            if (Track_Show[3] == LINE_RAW_VALUE || Track_Show[4] == LINE_RAW_VALUE)
            {
                lock_timer = 0;
                last_error = 0.0f;
                return;
            }
        }

        if (Track_Action == TRACK_ACT_LEFT)
        {
            Motor_Forward_Diff(-100, 425);
        }
        else if (Track_Action == TRACK_ACT_RIGHT)
        {
            Motor_Forward_Diff(425, -100);
        }
        return;
    }

    /* 2. 脱离赛道保护：八路全未检测到黑线时立即停车 */
    uint8_t all_dark = 1;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (Track_Show[i] == LINE_RAW_VALUE)
        {
            all_dark = 0;
            break;
        }
    }
    if (all_dark)
    {
        Motor_Stop();
        Track_Action = TRACK_ACT_STOP;
        return;
    }

    /* 3. 直角弯道特征识别（单侧连续三路同时触发） */
    if ((Track_Show[0] == LINE_RAW_VALUE && Track_Show[1] == LINE_RAW_VALUE && Track_Show[2] == LINE_RAW_VALUE) ||
        (Track_Show[1] == LINE_RAW_VALUE && Track_Show[2] == LINE_RAW_VALUE && Track_Show[3] == LINE_RAW_VALUE))
    {
        Track_Action = TRACK_ACT_LEFT;
        lock_timer = 26;
        Motor_Forward_Diff(-100, 425);
        return;
    }

    if ((Track_Show[5] == LINE_RAW_VALUE && Track_Show[6] == LINE_RAW_VALUE && Track_Show[7] == LINE_RAW_VALUE) ||
        (Track_Show[4] == LINE_RAW_VALUE && Track_Show[5] == LINE_RAW_VALUE && Track_Show[6] == LINE_RAW_VALUE))
    {
        Track_Action = TRACK_ACT_RIGHT;
        lock_timer = 26;
        Motor_Forward_Diff(425, -100);
        return;
    }

    /* 4. 直线与普通弧形弯加权偏差 PD 闭环修正 */
    float weighted_sum = 0.0f;
    int active_sensors = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (Track_Show[i] == LINE_RAW_VALUE)
        {
            weighted_sum += sensor_weights[i];
            active_sensors++;
        }
    }

    if (active_sensors > 0)
    {
        float error = weighted_sum / (float)active_sensors;

        if (fabsf(error) < 0.4f)
        {
            error = 0.0f;
        }

        float pid_output = (kp * error) + (kd * (error - last_error));
        last_error = error;

        int left_speed  = base_speed + (int)pid_output;
        int right_speed = base_speed - (int)pid_output;

        if (left_speed > max_speed)  left_speed = max_speed;
        if (left_speed < 0)          left_speed = 0;
        if (right_speed > max_speed) right_speed = max_speed;
        if (right_speed < 0)         right_speed = 0;

        Motor_Forward_Diff(left_speed, right_speed);
        Track_Action = TRACK_ACT_PD;
    }
}
