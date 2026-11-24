#include "mode_remote.h"
#include "motor.h"

/**
  * @brief  蓝牙遥控模式运动执行器
  * @param  cmd: 串口接收到的运动控制字符 ('F'/'B'/'L'/'R'/'S')
  */
void Mode_Remote_Process(char cmd)
{
    switch (cmd)
    {
        case 'F': Motor_Forward(500);  break; // 前进
        case 'B': Motor_Backward(500); break; // 后退
        case 'L': Motor_Left(200);     break; // 原地左转
        case 'R': Motor_Right(200);    break; // 原地右转
        case 'S': Motor_Stop();        break; // 停车
        default:                       break;
    }
}
