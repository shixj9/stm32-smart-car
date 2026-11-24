#include "mpu6050.h"
#include "motor.h"
#include "bluetooth.h"
#include <math.h>

static int32_t GyroZ_Offset = 0;
static uint32_t Gyro_LastTick = 0;
static uint8_t MPU_Ready = 0;

float Yaw_Angle = 0.0f;

/**
  * @brief  软件模拟 I2C 微秒级延时（72MHz 主频）
  */
static void I2C_Delay(void)
{
    __IO uint32_t delay = 30;
    while (delay--);
}

static void I2C_Start(void)
{
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);
    I2C_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    I2C_Delay();
}

static void I2C_Stop(void)
{
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    I2C_Delay();
}

static uint8_t I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, (byte & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        byte <<= 1;
        I2C_Delay();
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
        I2C_Delay();
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
        I2C_Delay();
    }

    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    I2C_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    uint8_t ack = (uint8_t)HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    I2C_Delay();
    return ack;
}

static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t byte = 0;
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    I2C_Delay();

    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
        I2C_Delay();
        if (HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin) == GPIO_PIN_SET)
        {
            byte |= 0x01;
        }
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
        I2C_Delay();
    }

    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, ack ? GPIO_PIN_SET : GPIO_PIN_RESET);
    I2C_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    I2C_Delay();
    return byte;
}

void MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    I2C_Start();
    I2C_SendByte(MPU6050_ADDR_WRITE);
    I2C_SendByte(reg);
    I2C_SendByte(data);
    I2C_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t res;
    I2C_Start();
    I2C_SendByte(MPU6050_ADDR_WRITE);
    I2C_SendByte(reg);

    I2C_Start();
    I2C_SendByte(MPU6050_ADDR_READ);
    res = I2C_ReadByte(1);
    I2C_Stop();
    return res;
}

/**
  * @brief  初始化 MPU6050 并执行上电静止零偏校准（采样约 1s，期间小车需保持静止）
  * @retval 0: 成功, 1: 器件未响应或 ID 不匹配
  */
uint8_t MPU6050_Init(void)
{
    MPU_Ready = 0;
    HAL_Delay(100);

    uint8_t id = MPU6050_ReadReg(MPU_WHO_AM_I_REG);
    if (id != 0x68)
    {
        return 1;
    }

    MPU6050_WriteReg(MPU_PWR_MGMT_1_REG, 0x00); // 解除休眠
    HAL_Delay(5);
    MPU6050_WriteReg(MPU_GYRO_CFG_REG, 0x18);   // 量程 ±2000 deg/s (16.4 LSB/(deg/s))
    HAL_Delay(5);

    int32_t sum = 0;
    for (int i = 0; i < 200; i++)
    {
        sum += MPU6050_Get_GyroZ();
        HAL_Delay(5);
    }
    GyroZ_Offset = sum / 200;

    Yaw_Angle = 0.0f;
    Gyro_LastTick = HAL_GetTick();
    MPU_Ready = 1;
    return 0;
}

uint8_t MPU6050_IsReady(void)
{
    return MPU_Ready;
}

int16_t MPU6050_Get_GyroZ(void)
{
    uint8_t head, tail;
    I2C_Start();
    I2C_SendByte(MPU6050_ADDR_WRITE);
    I2C_SendByte(MPU_GYRO_ZOUT_H_REG);

    I2C_Start();
    I2C_SendByte(MPU6050_ADDR_READ);
    head = I2C_ReadByte(0);
    tail = I2C_ReadByte(1);
    I2C_Stop();

    return (int16_t)((head << 8) | tail);
}

/**
  * @brief  根据真实时间间隔离散积分更新偏航角 Yaw_Angle
  */
void MPU6050_Update_Angle(void)
{
    if (!MPU_Ready)
    {
        return;
    }

    uint32_t now_tick = HAL_GetTick();
    float dt = (float)(now_tick - Gyro_LastTick) / 1000.0f;
    Gyro_LastTick = now_tick;

    // 过滤刚初始化或阻塞流程恢复后的异常时间步长
    if (dt <= 0.0f || dt > 0.2f)
    {
        return;
    }

    int16_t raw_z = MPU6050_Get_GyroZ() - (int16_t)GyroZ_Offset;

    // 静止死区滤波，抑制陀螺仪零漂
    if (raw_z > -15 && raw_z < 15)
    {
        raw_z = 0;
    }

    float gyro_z_speed = (float)raw_z / 16.4f;
    Yaw_Angle += gyro_z_speed * dt;

    if (Yaw_Angle > 180.0f)  Yaw_Angle -= 360.0f;
    if (Yaw_Angle < -180.0f) Yaw_Angle += 360.0f;
}

/**
  * @brief  原地闭环左转指定角度（支持蓝牙模式切换随时中断退出）
  */
void MPU_Turn_Left(float angle, uint16_t speed)
{
    Motor_Left(speed);

    // 若陀螺仪未就绪，退化为约 460ms 定时转向兜底
    if (!MPU_Ready)
    {
        for (uint8_t i = 0; i < 23 && Car_Mode == CAR_MODE_OBSTACLE; i++)
        {
            HAL_Delay(20);
        }
        Motor_Stop();
        return;
    }

    float target_turned = 0.0f;
    float last_angle = Yaw_Angle;
    uint16_t safety_counter = 0;

    while (target_turned < angle && safety_counter < 200)
    {
        if (Car_Mode != CAR_MODE_OBSTACLE)
        {
            break;
        }

        safety_counter++;
        MPU6050_Update_Angle();

        float diff = Yaw_Angle - last_angle;
        if (diff < -180.0f) diff += 360.0f;
        if (diff > 180.0f)  diff -= 360.0f;

        target_turned += fabsf(diff);
        last_angle = Yaw_Angle;

        HAL_Delay(20);
    }

    Motor_Stop();
}

/**
  * @brief  原地闭环右转指定角度（支持蓝牙模式切换随时中断退出）
  */
void MPU_Turn_Right(float angle, uint16_t speed)
{
    Motor_Right(speed);

    if (!MPU_Ready)
    {
        for (uint8_t i = 0; i < 23 && Car_Mode == CAR_MODE_OBSTACLE; i++)
        {
            HAL_Delay(20);
        }
        Motor_Stop();
        return;
    }

    float target_turned = 0.0f;
    float last_angle = Yaw_Angle;
    uint16_t safety_counter = 0;

    while (target_turned < angle && safety_counter < 200)
    {
        if (Car_Mode != CAR_MODE_OBSTACLE)
        {
            break;
        }

        safety_counter++;
        MPU6050_Update_Angle();

        float diff = last_angle - Yaw_Angle;
        if (diff < -180.0f) diff += 360.0f;
        if (diff > 180.0f)  diff -= 360.0f;

        target_turned += fabsf(diff);
        last_angle = Yaw_Angle;

        HAL_Delay(20);
    }

    Motor_Stop();
}
