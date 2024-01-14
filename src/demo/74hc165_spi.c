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

__attribute__((aligned(4))) UINT8 spiBuffrev[16];

void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

uint8_t uartBuff[] = "\x1B[0;0H\x1B[J0000\r\n0000\r\n0000\r\n0000\r\n";

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

    /* 配置串口调试 */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

    /* SPI 0 */
    GPIOA_SetBits(GPIO_Pin_12);
    GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    SPI0_MasterDefInit();
    SPI0_CLKCfg(60);                     // 60M/1M
    SPI0_DataMode(Mode0_HighBitINFront); // SCLK空闲高电平 高位在前
    R8_SPI0_CTRL_MOD &= ~RB_SPI_MOSI_OE; // MOSI不输出

    while (1)
    {
        GPIOA_ResetBits(GPIO_Pin_14);
        GPIOA_SetBits(GPIO_Pin_14);
        GPIOA_ResetBits(GPIO_Pin_12);
        SPI0_MasterRecv(spiBuffrev, 2);
        GPIOA_SetBits(GPIO_Pin_12);

        uint8_t *c = uartBuff + 9;
        for (int i = 0; i < 2; i++)
        {
            uint8_t b = spiBuffrev[i];
            *c++ = ((b & 0x80) == 0) + '0';
            *c++ = ((b & 0x40) == 0) + '0';
            *c++ = ((b & 0x20) == 0) + '0';
            *c++ = ((b & 0x10) == 0) + '0';
            c += 2;
            *c++ = ((b & 0x08) == 0) + '0';
            *c++ = ((b & 0x04) == 0) + '0';
            *c++ = ((b & 0x02) == 0) + '0';
            *c++ = ((b & 0x01) == 0) + '0';
            c += 2;
        }
        *c = 0;
        UART1_SendString(uartBuff, sizeof(uartBuff) - 1);

        DelayMs(30);
    }
}