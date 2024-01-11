/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : Tamap
 * Version            : V1.0
 * Date               : 2024/01/10
 * Description        : SPI0驱动WS281X彩灯功能演示
 * Copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#include "CH58x_common.h"

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

#define RGB_NUM 16
#define PART_NUM RGB_NUM
#define BIT_NUM (12 * PART_NUM) // DATA
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

#if 0
static const uint8_t bit_table[8] = {
    /* 前位0 低位在前 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
    /* 前位1相同 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
};
#else
static const uint8_t bit_table[8] = {
    /* 前位0 低位在前 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
    /* 前位1相同 */
    0x04, // 0 001000 1
    0x24, // 0 001001 1
    0x06, // 0 011000 1
    0x26, // 0 011001 1
};
#endif

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
    for (int i = 0; i < PART_NUM; i++)
    {
        uint16_t j = x + (i << 4);
        color_t rgb;
        rgb.c.r = get_sin_u8(j);
        rgb.c.g = get_sin_u8(j + 341);
        rgb.c.b = get_sin_u8(j + 682);
        for (int j = 0; j < 12; j++)
        {
            rgb.dw <<= 2;
            *b++ = bit_table[rgb.c.x & 0x03];
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
