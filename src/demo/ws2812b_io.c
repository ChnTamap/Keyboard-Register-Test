/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : Li
 * Version            : V1.0
 * Date               : 2022/09/07
 * Description         : IO驱动WS281X彩灯功能演示
 * Copyright (c) 2022 Guangzhou Liandun Electronic Technology Co., LTD.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#include "CH58x_common.h"

#define SNUM       4//灯珠数量

//缓存区
unsigned char buf[SNUM][3]={
0
};
//发0码
void Set0Code(void) {
    GPIOB_SetBits(GPIO_Pin_7);
    __nop();
    __nop();
    GPIOB_ResetBits(GPIO_Pin_7);
    //       NOP();
}
//发1码
void Set1Code(void) {
    GPIOB_SetBits(GPIO_Pin_7);
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    GPIOB_ResetBits(GPIO_Pin_7);
}
//发一个像素
void SendOnePix(unsigned char *ptr) {
    unsigned char i, j;
    unsigned char temp;

    for (j = 0; j < 3; j++) {
        temp = ptr[j];
        for (i = 0; i < 8; i++) {
            if (temp & 0x80)        //从高位开始发送
                    {
                Set1Code();
            } else                //发送“0”码
            {
                Set0Code();
            }
            temp = (temp << 1);      //左移位
        }
    }
}
//发一帧数据
void SendOneFrame(unsigned char *ptr) {
    uint16_t k;

    //  PWM_RGB=0;//发送帧复位信号
    //  DelayMs(1);

    for (k = 0; k < SNUM; k++)              //发送一帧数据，SNUM是板子LED的个数
            {
        SendOnePix(&ptr[(3 * k)]);
    }

    GPIOB_ResetBits(GPIO_Pin_7);              //发送帧复位信号

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
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_20mA);  // PB7 - PWM9


    while(1)
    {
        int i;
        //Red
        for (i = 0; i < SNUM; ++i) {
            buf[i][0]=0x80;
            buf[i][1]=0x00;
            buf[i][2]=0x00;
        }
        SendOneFrame(buf[0]);
        DelayMs(1000);
        //Green
        for (i = 0; i < SNUM; ++i) {
            buf[i][0]=0x00;
            buf[i][1]=0x80;
            buf[i][2]=0x00;
        }
        SendOneFrame(buf[0]);
        DelayMs(1000);
        //Blue
        for (i = 0; i < SNUM; ++i) {
            buf[i][0]=0x00;
            buf[i][1]=0x00;
            buf[i][2]=0x80;
        }
        SendOneFrame(buf[0]);
        DelayMs(1000);
    }
}

