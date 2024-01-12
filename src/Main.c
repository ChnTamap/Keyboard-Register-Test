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
 * 2LED位1个串口字节 12 Byte/LED
 * 单位时间300ns 主频80M 波特率3.33M N61
 * 数据操作使用串口FIFO 每次中断写入8字节 中断周期19.2us
 * 128个LED总时间 3.6864 ms 总显存 1536 byte
 */
#define RGBT1_80M_300ns

#ifdef RGBT1_80M_300ns
#define BIT_PER_BYTE 2
static const uint8_t bit_table[4] = {
    /* 前位0 低位在前 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
    /* 前位1相同 ... */
};
#endif

#define RGB_NUM 16                         // RGBLED的数量
#define BYTE_PER_RGB (24 / BIT_PER_BYTE)   // X byte/LED
#define BIT_NUM ((BYTE_PER_RGB) * RGB_NUM) // DATA
__attribute__((aligned(4))) uint8_t TxBuff[BIT_NUM + 8];

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
    UART1_BaudRateCfg(3333333);
    R8_UART1_LCR = 0x01; // RB_LCR_WORD_SZ; 6bit

    TxBuff[BIT_NUM - 8] = 0x00;
    while (1)
    {
        update_rgb();

        R8_UART1_THR = TxBuff[0];
        R8_UART1_THR = TxBuff[1];
        R8_UART1_IER = RB_IER_TXD_EN;
        GPIOA_SetBits(GPIO_Pin_9);
        R8_UART1_THR = TxBuff[2];
        R8_UART1_THR = TxBuff[3];

        UART1_SendString(TxBuff + 4, sizeof(TxBuff) - 4);

        GPIOA_ResetBits(GPIO_Pin_9);
        R8_UART1_IER &= ~RB_IER_TXD_EN;

        DelayMs(30);
    }
}
