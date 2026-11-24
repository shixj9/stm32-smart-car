#ifndef __HC_SR04_H
#define __HC_SR04_H

#include "main.h"

/* ==================== 超声波输入捕获状态枚举 ==================== */
typedef enum {
    HCSR04_WAIT_RISE = 0, // 等待 Echo 上升沿
    HCSR04_WAIT_FALL = 1, // 等待 Echo 下降沿
    HCSR04_COMPLETE  = 2  // 脉冲宽度捕获完成
} HCSR04_State_t;

/* ==================== 对外函数接口 ==================== */
void  HCSR04_StartTrigger(void);
float HCSR04_GetDistance(void);
void  HCSR04_UpdateAsync(void);
float HCSR04_GetLastDistance(void);

#endif /* __HC_SR04_H */
