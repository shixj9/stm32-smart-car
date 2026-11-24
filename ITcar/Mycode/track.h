#ifndef __TRACK_H
#define __TRACK_H

#include "main.h"

/* 传感器检测到黑线时的电平状态 (1: 高电平代表黑线) */
#define LINE_RAW_VALUE  1

/* 循迹动作状态码（用于 OLED A:x 实时状态指示） */
typedef enum {
    TRACK_ACT_IDLE  = 0, // 初始化 / 无动作
    TRACK_ACT_PD    = 1, // 直线及常规弯道 PD 闭环循迹
    TRACK_ACT_LEFT  = 2, // 左直角弯锁定转向
    TRACK_ACT_RIGHT = 3, // 右直角弯锁定转向
    TRACK_ACT_STOP  = 4  // 脱离赛道全灭保护停车
} TrackAction_t;

extern uint8_t Track_Show[8];
extern uint8_t Track_Action;

void Track_Init(void);
void Track_Reset(void);
void Track_ReadSensors(uint8_t *sensor_values);
void Mode_Track_Process(void);

#endif /* __TRACK_H */
