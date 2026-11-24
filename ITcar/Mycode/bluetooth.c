#include "bluetooth.h"
#include "motor.h"
#include "track.h"

/* 全局变量 */
uint8_t Rx_Buffer = 0;
volatile char Bluetooth_Cmd = 'S';              // 当前遥控运动指令缓存
volatile uint8_t Car_Mode = CAR_MODE_STANDBY;   // 默认上电进入待机模式

/**
  * @brief  开启 USART1 (JDY-31 蓝牙模块, 9600bps) 单字节接收中断
  */
void Bluetooth_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
}

/**
  * @brief  串口接收完成中断回调：解析模式切换指令 ('0'~'3') 与遥控方向指令 ('F'/'B'/'L'/'R'/'S')
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (Rx_Buffer == '0')
        {
            Car_Mode = CAR_MODE_STANDBY;
            Bluetooth_Cmd = 'S';
            Motor_Stop();
        }
        else if (Rx_Buffer == '1')
        {
            Car_Mode = CAR_MODE_REMOTE;
            Bluetooth_Cmd = 'S'; // 清空历史方向缓存，防止切回遥控模式时误动作
            Motor_Stop();
        }
        else if (Rx_Buffer == '2')
        {
            Car_Mode = CAR_MODE_OBSTACLE;
            Bluetooth_Cmd = 'S';
            Motor_Stop();
        }
        else if (Rx_Buffer == '3')
        {
            Car_Mode = CAR_MODE_TRACK;
            Bluetooth_Cmd = 'S';
            Track_Reset();       // 清零上一次循迹遗留的锁定期与微分项
            Motor_Stop();
        }
        else if (Car_Mode == CAR_MODE_REMOTE)
        {
            Bluetooth_Cmd = (char)Rx_Buffer;
        }

        // 重新挂载下一次单字节中断接收
        HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
    }
}
