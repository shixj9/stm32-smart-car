#include "oled.h"
#include "oledfont.h"

static void OLED_Delay(void)
{
    __IO uint32_t delay = 30;
    while (delay--);
}

static void OLED_I2C_Start(void)
{
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    OLED_Delay();
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);
    OLED_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    OLED_Delay();
}

static void OLED_I2C_Stop(void)
{
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    OLED_Delay();
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    OLED_Delay();
}

static void OLED_I2C_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, (byte & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        byte <<= 1;
        OLED_Delay();
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
        OLED_Delay();
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
        OLED_Delay();
    }
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
    OLED_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    OLED_Delay();
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    OLED_Delay();
}

static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_I2C_Start();
    OLED_I2C_WriteByte(OLED_ADDRESS);
    OLED_I2C_WriteByte(0x00);
    OLED_I2C_WriteByte(cmd);
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    OLED_I2C_WriteByte(OLED_ADDRESS);
    OLED_I2C_WriteByte(0x40);
    OLED_I2C_WriteByte(data);
    OLED_I2C_Stop();
}

static void OLED_SetPos(uint8_t x, uint8_t y)
{
    OLED_WriteCmd(0xb0 + y);
    OLED_WriteCmd(((x & 0xf0) >> 4) | 0x10);
    OLED_WriteCmd((x & 0x0f) | 0x01);
}

void OLED_Clear(void)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteCmd(0xb0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for (uint8_t n = 0; n < 128; n++)
        {
            OLED_WriteData(0);
        }
    }
}

void OLED_Init(void)
{
    HAL_Delay(200);

    OLED_WriteCmd(0xAE); // 关闭显示
    OLED_WriteCmd(0x00); // 设置低列地址
    OLED_WriteCmd(0x10); // 设置高列地址
    OLED_WriteCmd(0x40); // 设置起始行地址
    OLED_WriteCmd(0xB0); // 设置页地址
    OLED_WriteCmd(0x81); // 对比度设置
    OLED_WriteCmd(0xFF); // 亮度最大
    OLED_WriteCmd(0xA1); // 设置左右方向
    OLED_WriteCmd(0xA6); // 正常显示
    OLED_WriteCmd(0xA8); // 设置多路复用率 (1 to 64)
    OLED_WriteCmd(0x3F); // 1/64 duty
    OLED_WriteCmd(0xC8); // 设置上下扫描方向
    OLED_WriteCmd(0xD3); // 设置显示偏移
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xD5); // 设置时钟分频因子
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xD9); // 设置预充电周期
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDA); // 设置 COM 引脚硬件配置
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xDB); // 设置 VCOMH 电压倍率
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D); // 电荷泵设置
    OLED_WriteCmd(0x14); // 开启电荷泵
    OLED_WriteCmd(0xAF); // 开启显示

    OLED_Clear();
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size)
{
    uint8_t c = (uint8_t)(chr - ' ');

    if (size == 16)
    {
        OLED_SetPos(x, y);
        for (uint8_t i = 0; i < 8; i++)
        {
            OLED_WriteData(F8X16[c * 16 + i]);
        }

        OLED_SetPos(x, y + 1);
        for (uint8_t i = 0; i < 8; i++)
        {
            OLED_WriteData(F8X16[c * 16 + i + 8]);
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size)
{
    uint8_t j = 0;
    while (chr[j] != '\0')
    {
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
        OLED_ShowChar(x, y, chr[j], size);
        x += 8;
        j++;
    }
}

static uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    for (uint8_t t = 0; t < len; t++)
    {
        uint8_t temp = (uint8_t)((num / OLED_Pow(10, len - 1 - t)) % 10);
        OLED_ShowChar(x + 8 * t, y, (char)(temp + '0'), size);
    }
}
