/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : Tamap
 * Version            : V1.0
 * Date               : 2024/01/09
 * Description        : TMR驱动WS281X彩灯功能演示
 * Copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#include "CH58x_common.h"

static uint32_t get_sin_u8(uint8_t x) {
    if(x < 64)
        return x>>1;
    else if(x < 128)
        return (127-x)>>1;
    else
        return 0;
}

#define RGB_NUM 16
#define BIT_RESET (280*100/125)
#define BIT_NUM (BIT_RESET + 24*RGB_NUM + 1)
__attribute__((aligned(4))) uint32_t PwmBuf[BIT_NUM];
uint32_t rgb;
uint8_t x;

/*********************************************************************
 * @fn      update_rgb
 *
 * @brief   更新rgb数据
 *
 * @return  none
 */
void update_rgb(void) {
    uint32_t *b = PwmBuf+BIT_RESET;
    x++;
    rgb = (get_sin_u8(x)<<16) | (get_sin_u8(x+85)<<8) | (get_sin_u8(x+170));
    for(int i = 0; i < RGB_NUM; i++) {
        uint32_t pix = rgb;
        for(int j = 0; j < 24; j++) {
            *b++ = (pix & 0x00800000)? 45: 15;
            pix <<= 1;
        }
    }
    *b = 0xFF;
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main() {
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置GPIO */
    PWR_UnitModCfg(DISABLE, UNIT_SYS_LSE); // 注意此引脚是LSE晶振引脚，要保证关闭才能使用其他功能
    GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_20mA);

    TMR1_PWMCycleCfg(75); // 60M / 75 = 1/1.25us
    for(int i = 0; i < BIT_RESET; i++)
        PwmBuf[i] = 0;
    update_rgb();

    TMR1_PWMInit(High_Level, PWM_Times_1);
    TMR1_DMACfg(ENABLE, (uint16_t)(uint32_t)&PwmBuf[0], (uint16_t)(uint32_t)&PwmBuf[BIT_NUM], Mode_Single);
    TMR1_PWMEnable();
    TMR1_Enable();

    /* 开启计数溢出中断，计满 BIT_NUM 个周期进入中断 */
    // TMR1_ClearITFlag(TMR1_2_IT_DMA_END);
    // TMR1_ITCfg(ENABLE, TMR1_2_IT_DMA_END);
    // PFIC_EnableIRQ(TMR1_IRQn);

    while(1)
    {
        DelayMs(30);
        update_rgb();
        TMR1_DMACfg(ENABLE, (uint16_t)(uint32_t)&PwmBuf[0], (uint16_t)(uint32_t)&PwmBuf[BIT_NUM], Mode_Single);
        // TMR1_ITCfg(ENABLE, TMR1_2_IT_DMA_END);
    }
}

/*********************************************************************
 * @fn      TMR1_IRQHandler
 *
 * @brief   TMR1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void) // TMR1 定时中断
{
    if(TMR1_GetITFlag(TMR1_2_IT_DMA_END))
    {
        TMR1_ITCfg(DISABLE, TMR1_2_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR1_ClearITFlag(TMR1_2_IT_DMA_END);    // 清除中断标志
    }
}
