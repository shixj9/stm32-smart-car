#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "main.h"
#include "usart.h"

/* 小车全局工作模式定义 */
typedef enum {
    CAR_MODE_STANDBY  = 0, // 待机停止模式
    CAR_MODE_REMOTE   = 1, // 蓝牙手动遥控模式
    CAR_MODE_OBSTACLE = 2, // 超声波 + 陀螺仪自主避障模式
    CAR_MODE_TRACK    = 3  // 八路灰度自主循迹模式
} CarMode_t;

/* 全局状态变量声明 */
extern uint8_t Rx_Buffer;
extern volatile uint8_t Car_Mode;
extern volatile char Bluetooth_Cmd;

/* 函数声明 */
void Bluetooth_Init(void);

#endif /* __BLUETOOTH_H */
