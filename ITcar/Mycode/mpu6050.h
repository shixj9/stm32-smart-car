#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"

/* MPU6050 I2C 8位从机地址（AD0 接地时 7位地址为 0x68） */
#define MPU6050_ADDR_WRITE  0xD0
#define MPU6050_ADDR_READ   0xD1

/* MPU6050 核心寄存器地址 */
#define MPU_PWR_MGMT_1_REG  0x6B   // 电源管理寄存器 1
#define MPU_GYRO_CFG_REG    0x1B   // 陀螺仪量程配置寄存器
#define MPU_GYRO_ZOUT_H_REG 0x47   // Z 轴角速度高 8 位
#define MPU_GYRO_ZOUT_L_REG 0x48   // Z 轴角速度低 8 位
#define MPU_WHO_AM_I_REG    0x75   // 器件 ID 寄存器 (0x68)

/* 当前累计偏航角（单位：度，范围 -180.0 ~ 180.0） */
extern float Yaw_Angle;

/* 对外公共接口 */
uint8_t MPU6050_Init(void);
uint8_t MPU6050_IsReady(void);
uint8_t MPU6050_ReadReg(uint8_t reg);
void    MPU6050_WriteReg(uint8_t reg, uint8_t data);
int16_t MPU6050_Get_GyroZ(void);
void    MPU6050_Update_Angle(void);
void    MPU_Turn_Left(float angle, uint16_t speed);
void    MPU_Turn_Right(float angle, uint16_t speed);

#endif /* __MPU6050_H */
