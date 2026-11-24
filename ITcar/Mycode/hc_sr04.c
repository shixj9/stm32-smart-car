#include "hc_sr04.h"
#include "tim.h"

static volatile HCSR04_State_t Capture_Flag = HCSR04_WAIT_RISE;
static uint16_t Rise_Val = 0;          // 上升沿捕获计数值 (1us/tick)
static uint16_t Fall_Val = 0;          // 下降沿捕获计数值 (1us/tick)
static volatile uint32_t Echo_Time = 0;// 高电平持续微秒数
static volatile float Last_Distance = -1.0f;
static uint8_t Async_Triggered = 0;

/**
  * @brief  控制 Trig 引脚 (PB0) 输出 >=10us 高电平触发脉冲
  */
void HCSR04_StartTrigger(void)
{
    Capture_Flag = HCSR04_WAIT_RISE;
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);

    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
    __IO uint32_t delay = 15 * 8; // 72MHz 下使用 volatile 防止编译器高优化等级裁剪延时
    while (delay--);
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
}

/**
  * @brief  阻塞式测距（用于避障模式下停车左右探路，带 30ms 超时保护）
  * @retval 距离值（单位 cm），超时返回 -1.0f
  */
float HCSR04_GetDistance(void)
{
    uint32_t timeout = 0;
    Async_Triggered = 0;

    HCSR04_StartTrigger();

    while (Capture_Flag != HCSR04_COMPLETE)
    {
        timeout++;
        HAL_Delay(1);
        if (timeout > 30)
        {
            Last_Distance = -1.0f;
            return -1.0f;
        }
    }

    Last_Distance = (float)Echo_Time / 58.0f;
    return Last_Distance;
}

/**
  * @brief  非阻塞后台测距刷新（供主循环定时调用，不阻塞循迹 PD 节拍）
  * @note   结算上一轮捕获结果后立即发射下一轮触发脉冲（建议间隔 >= 60ms）
  */
void HCSR04_UpdateAsync(void)
{
    if (Async_Triggered)
    {
        if (Capture_Flag == HCSR04_COMPLETE)
        {
            Last_Distance = (float)Echo_Time / 58.0f;
        }
        else
        {
            Last_Distance = -1.0f;
        }
    }

    HCSR04_StartTrigger();
    Async_Triggered = 1;
}

/**
  * @brief  获取最近一次有效测距缓存值（单位 cm）
  */
float HCSR04_GetLastDistance(void)
{
    return Last_Distance;
}

/**
  * @brief  TIM1_CH1 (PA8) 输入捕获中断回调，测量 Echo 高电平脉宽
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        if (Capture_Flag == HCSR04_WAIT_RISE)
        {
            Rise_Val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
            Capture_Flag = HCSR04_WAIT_FALL;
        }
        else if (Capture_Flag == HCSR04_WAIT_FALL)
        {
            Fall_Val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (Fall_Val >= Rise_Val)
            {
                Echo_Time = Fall_Val - Rise_Val;
            }
            else
            {
                Echo_Time = (65535U - Rise_Val) + Fall_Val + 1U;
            }

            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            Capture_Flag = HCSR04_COMPLETE;
        }
    }
}
