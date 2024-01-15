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

#define KEY_NUM 16
#define KEY_BYTE (KEY_NUM / 8)
__attribute__((aligned(4))) UINT8 keyBuff[KEY_BYTE * 2];
static uint8_t *spiBuff = keyBuff;

void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/**
 * @brief 在终端中显示按键状态 推荐使用PUTTY
 * @details 0000
 *          0000
 *          0000
 *          0000
 */
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
        int i;
        DelayMs(5); // Delay

        /* 切换buff用于比较按键变化 */
        spiBuff = (spiBuff == keyBuff) ? keyBuff + KEY_BYTE : keyBuff;

        /* 启动SPI接收寄存器数据 */
        GPIOA_ResetBits(GPIO_Pin_14);
        GPIOA_SetBits(GPIO_Pin_14);
        GPIOA_ResetBits(GPIO_Pin_12);
        SPI0_MasterRecv(spiBuff, KEY_BYTE);
        GPIOA_SetBits(GPIO_Pin_12);

        /* 检查按键变化 */
        uint8_t *c = keyBuff;
        uint8_t *c1 = keyBuff + KEY_BYTE;
        for (i = 0; i < KEY_BYTE; i++)
        {
            if (*c++ != *c1++)
                break;
        }
        /* 如果按键发生变化则更新串口显示 */
        if (i == KEY_BYTE)
            continue;

        /* 串口排版并发送 */
        c = uartBuff + 9;
        for (i = 0; i < KEY_BYTE; i++)
        {
            uint8_t b = spiBuff[i];
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
    }
}