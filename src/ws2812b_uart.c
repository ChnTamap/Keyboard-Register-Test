/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : Tamap
 * Version            : V1.0
 * Date               : 2024/01/12
 * Description        : UART1驱动WS281X彩灯功能演示
 *
 * Copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#include "CH58x_common.h"

/**
 * 时序选择, 请根据实际系统主频和性能要求选择串口频率和位数
 * 注意: 带'!'的选项并不严格满足时序或比较极限, 但经测试可以运行
 */
#define RGBT2_N61_300ns /*    2:1 34.7k LED/s 中断周期19.2us */
// #define RGBT3_N81_300ns /* (!)3:1 41.7k LED/s 中断周期24us */
// #define RGBT3_N81_333ns /* (!)3:1 37.5k LED/s 中断周期26.64us */
// #define RGBT3_N81_400ns /* (!)3:1 31.3k LED/s 中断周期32us */
// #define RGBT3_E81_300ns /* (!)3:1 37.9k LED/s 中断周期 26.4us */
// #define RGBT3_E81_333ns /* (!)3:1 34.1k LED/s 中断周期29.3us */
// #define RGBT3_E81_400ns /* (!)3:1 28.4k LED/s 中断周期35.2us */

#if defined(RGBT2_N61_300ns)
#define RGB_FSYS CLK_SOURCE_PLL_80MHz
#define RGB_BAUDRATE 3333333
#define BIT_PER_BYTE 2
#define RGB_UART_LCR 0x01 // N61
static const uint8_t bit_table[4] = {
    /* 前位0 低位在前 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
    /* 前位1相同 ... */
};
#elif defined(RGBT3_N81_300ns)
#define RGB_FSYS CLK_SOURCE_PLL_80MHz
#define RGB_BAUDRATE 3333333
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x03 // N81
static const uint8_t bit_table[8] = {
    0x12, // 0 01001000 1
    0x92, // 0 01001001 1
    0x32, // 0 01001100 1
    0xB2, // 0 01001101 1
    0x16, // 0 01101000 1
    0x96, // 0 01101001 1
    0x36, // 0 01101100 1
    0xB6, // 0 01101101 1
};
#elif defined(RGBT3_N81_333ns)
#define RGB_FSYS CLK_SOURCE_PLL_48MHz
#define RGB_BAUDRATE 3000000
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x03 // N81
static const uint8_t bit_table[8] = {
    0x12, // 0 01001000 1
    0x92, // 0 01001001 1
    0x32, // 0 01001100 1
    0xB2, // 0 01001101 1
    0x16, // 0 01101000 1
    0x96, // 0 01101001 1
    0x36, // 0 01101100 1
    0xB6, // 0 01101101 1
};
#elif defined(RGBT3_N81_400ns)
#define RGB_FSYS CLK_SOURCE_PLL_60MHz
#define RGB_BAUDRATE 2500000
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x03 // N81
static const uint8_t bit_table[8] = {
    0x12, // 0 01001000 1
    0x92, // 0 01001001 1
    0x32, // 0 01001100 1
    0xB2, // 0 01001101 1
    0x16, // 0 01101000 1
    0x96, // 0 01101001 1
    0x36, // 0 01101100 1
    0xB6, // 0 01101101 1
};
#elif defined(RGBT3_E81_300ns)
#define RGB_FSYS CLK_SOURCE_PLL_80MHz
#define RGB_BAUDRATE 3333333
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x1B // E81
static const uint8_t bit_table[8] = {
    0x22, // 0 01000100 01
    0x92, // 0 01001001 11
    0x72, // 0 01001110 01
    0x62, // 0 01000110 11
    0x4E, // 0 01110010 01
    0x26, // 0 01100100 11
    0x66, // 0 01100110 01
    0x6E, // 0 01110110 11
};
#elif defined(RGBT3_E81_333ns)
#define RGB_FSYS CLK_SOURCE_PLL_48MHz
#define RGB_BAUDRATE 3000000
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x1B // E81
static const uint8_t bit_table[8] = {
    0x22, // 0 01000100 01
    0x92, // 0 01001001 11
    0x72, // 0 01001110 01
    0x62, // 0 01000110 11
    0x4E, // 0 01110010 01
    0x26, // 0 01100100 11
    0x66, // 0 01100110 01
    0x6E, // 0 01110110 11
};
#elif defined(RGBT3_E81_400ns)
#define RGB_FSYS CLK_SOURCE_PLL_60MHz
#define RGB_BAUDRATE 2500000
#define BIT_PER_BYTE 3
#define RGB_UART_LCR 0x1B // E81
static const uint8_t bit_table[8] = {
    0x22, // 0 01000100 01
    0x92, // 0 01001001 11
    0x72, // 0 01001110 01
    0x62, // 0 01000110 11
    0x4E, // 0 01110010 01
    0x26, // 0 01100100 11
    0x66, // 0 01100110 01
    0x6E, // 0 01110110 11
};
#endif

/**
 * RGB串口数据缓存
 */
#define RGB_NUM 16                         // RGBLED的数量
#define BYTE_PER_RGB (24 / BIT_PER_BYTE)   // X byte/LED
#define BIT_NUM ((BYTE_PER_RGB) * RGB_NUM) // DATA
__attribute__((aligned(4))) uint8_t TxBuff[BIT_NUM + 9];

typedef union
{
#ifdef __BIG_ENDIAN
    struct
    {
        uint8_t x;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } c;
#else
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t x;
    } c;
#endif
    uint32_t dw;
} color_t;
uint16_t x;

static uint32_t get_sin_u8(uint16_t x)
{
    x = x % 1024;
    if (x < 256)
        return x;
    else if (x < 512)
        return (511 - x);
    else
        return 0;
}

/*********************************************************************
 * @fn      update_rgb
 *
 * @brief   更新rgb数据
 *
 * @return  none
 */
void update_rgb(void)
{
    x++;
    if (x >= 1024)
        x = 0;

    uint8_t *b = TxBuff;
    for (int i = 0; i < RGB_NUM; i++)
    {
        uint16_t j = x + (i << 4);
        color_t rgb;
        rgb.c.r = get_sin_u8(j);
        rgb.c.g = get_sin_u8(j + 341);
        rgb.c.b = get_sin_u8(j + 682);
        for (int j = 0; j < BYTE_PER_RGB; j++)
        {
            rgb.dw <<= BIT_PER_BYTE;
#define BIT_MASK ((1 << BIT_PER_BYTE) - 1)
            *b++ = bit_table[rgb.c.x & BIT_MASK];
        }
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main()
{
    SetSysClock(CLK_SOURCE_PLL_80MHz);

    /* 配置串口1：先配置IO口模式，再配置串口 */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART1_DefInit();
    UART1_BaudRateCfg(RGB_BAUDRATE);
    R8_UART1_LCR = RGB_UART_LCR; // 串口数据位

    TxBuff[BIT_NUM - 9] = 0x00;
    while (1)
    {
        update_rgb();

        /* 先发几个数据, 顺便等第一个字节进入起始位 */
        R8_UART1_THR = TxBuff[0];
        R8_UART1_THR = TxBuff[1];
        R8_UART1_THR = TxBuff[2];
        R8_UART1_THR = TxBuff[3];
        /* 起始位及后续的低电平期间, 使能TX以平滑过渡 */
        R8_UART1_IER = RB_IER_TXD_EN;
        GPIOA_SetBits(GPIO_Pin_9);
        /* 输出数据 */
        UART1_SendString(TxBuff + 4, sizeof(TxBuff) - 4);
        /* 倒数第9个字节(后面有8个无效字节在FIFO里面)是无效字节00, 用于过渡低电平 */
        GPIOA_ResetBits(GPIO_Pin_9);
        R8_UART1_IER &= ~RB_IER_TXD_EN;

        DelayMs(30);
    }
}
