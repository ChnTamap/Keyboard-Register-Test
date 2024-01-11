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
#define BIT_NUM (12 * RGB_NUM)
__attribute__((aligned(4))) uint8_t spiBuff[BIT_NUM];

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

static const uint8_t bit_table[4] = {0x88, 0x8C, 0xC8, 0xCC};

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

    color_t rgb;
    rgb.c.r = get_sin_u8(x);
    rgb.c.g = get_sin_u8(x + 341);
    rgb.c.b = get_sin_u8(x + 682);

    uint8_t *b = spiBuff;
    for (int i = 0; i < RGB_NUM; i++)
    {
        color_t pix = rgb;
        for (int j = 0; j < 12; j++)
        {
            pix.dw <<= 2;
            *b++ = bit_table[pix.c.x & 0x03];
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
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* SPI 0 */
    GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    SPI0_MasterDefInit();
    SPI0_CLKCfg(18); // 1/8bit=9 1/4bit=18 1.2us/bit SPI:3.33M
    SPI0_DataMode(Mode0_HighBitINFront);

    while (1)
    {
        SPI0_MasterDMATrans(spiBuff, BIT_NUM);
        update_rgb();
        DelayMs(30);
    }
}
