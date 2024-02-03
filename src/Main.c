/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : Tamap
 * Version            : V1.0
 * Date               : 2024/01/15
 * Description        : UART2驱动WS281X彩灯功能演示
 *
 * Copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#include "CH58x_common.h"
#include "kb_interface.h"
#include "stimer.h"

/* =========================KEY========================= */

#define KEY_NUM  16
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

/* =========================RGB========================= */

static inf_rgbarr_t *rgb0;

/**
 * @brief Get the rgb function u8 pix
 * @param x phase (period is 1024)
 * @return uint32_t
 */
static uint32_t get_rgbfun_u8(uint16_t x)
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
    static uint16_t x = 0;
    x++;
    if (x >= 1024)
        x = 0;

    ra_pix_t *pix = rgb0->buff;
    for (int i = 0; i < rgb0->led_num; i++)
    {
        if (spiBuff[(KEY_BYTE - 1) - i / 8] & (1U << (i % 8)))
        {
            *pix = (ra_pix_t){.r = 1, .g = 1, .b = 1};
        }
        else
        {
            uint16_t j = x + (i << 4);
            pix->r     = get_rgbfun_u8(j) >> 3;
            pix->g     = get_rgbfun_u8(j + 341) >> 3;
            pix->b     = get_rgbfun_u8(j + 682) >> 3;
        }
        pix++;
    }
}

/* =========================PWR========================= */

static uint8_t pwr_led  = 1;
static uint8_t pwr_chip = 1;
static uint8_t pwr_key  = 1;

/**
 * @brief ^LVCC ^CVCC ^KVCC CGND KGND
 */
static char pwr_pins[5] = "00011";
/**
 * @brief 电源引脚查表
 */
static const uint32_t pwr_table[] = {
    GPIO_Pin_22, /**< LVCC */
    GPIO_Pin_23, /**< CVCC */
    GPIO_Pin_4,  /**< KVCC */
    GPIO_Pin_12, /**< CGND */
    GPIO_Pin_13, /**< KGND */
};

/**
 * @brief 供电控制
 */
static void power_test_update(void)
{
    uint32_t pinh = 0;
    uint32_t pinl = 0;
    int i;
    for (i = 0; i < 5; i++)
    {
        if (pwr_pins[i] == '1')
            pinh |= pwr_table[i];
        else
            pinl |= pwr_table[i];
    }
    GPIOB_SetBits(pinh);
    GPIOB_ResetBits(pinl);
    /* 开关相应的功能 */
    pwr_led  = (pwr_pins[0] == '0' && pwr_pins[3] != '0'); /* LVCC CGND */
    pwr_chip = (pwr_pins[1] == '0' && pwr_pins[3] != '0'); /* CVCC CGND */
    pwr_key  = (pwr_pins[2] == '0' && pwr_pins[4] != '0'); /* KVCC KGND */
}

/**
 * @brief 测试供电开关(低功耗测试)
 */
static void power_test_init(void)
{
    uint32_t pin =
        pwr_table[0] |
        pwr_table[1] |
        pwr_table[2] |
        pwr_table[3] |
        pwr_table[4];
    GPIOB_ModeCfg(pin, GPIO_ModeOut_PP_5mA);
    power_test_update();
}

/**
 * @brief 判断供电开启
 * @return 是否更新 0:1
 */
static int power_test_handle(void)
{
    uint8_t rdata = 0;
    /* 串口控制 */
    while (R8_UART1_RFC)
    {
        rdata = UART1_RecvByte();
        if (rdata > '0' && rdata < '6')
        {
            pwr_pins[rdata - '1'] ^= 0x01;
        }
        else
        {
            pwr_pins[0] = '0';
            pwr_pins[1] = '0';
            pwr_pins[2] = '0';
            pwr_pins[3] = '0';
            pwr_pins[4] = '0';
        }
    }
    if (rdata)
        power_test_update();
    return (rdata != 0);
}

/* =========================APP========================= */

/**
 * @brief 在终端中显示按键状态 推荐使用PUTTY
 * @details 0000
 *          0000
 *          0000
 *          0000
 */
static uint8_t uartBuff[] = "\x1B[0;0H\x1B[J0000\r\n0000\r\n0000\r\n0000\r\n\r\n00000\r\n";

void port_rgbarr_init(void);

static uint8_t t_key_to = 0; /**< stim key timeout */
static void t_key_cb(struct stim *t)
{
    t_key_to = 1;
}
static uint8_t t_rgb_to = 0; /**< stim key timeout */
static void t_rgb_cb(struct stim *t)
{
    t_rgb_to = 1;
}

static void print_message(void)
{
    char *c;
    int i;
    /* 串口排版并发送 */
    c = uartBuff + 9;
    for (i = 0; i < KEY_BYTE; i++)
    {
        uint8_t b = spiBuff[i];
        *c++      = ((b & 0x80) == 0) + '0';
        *c++      = ((b & 0x40) == 0) + '0';
        *c++      = ((b & 0x20) == 0) + '0';
        *c++      = ((b & 0x10) == 0) + '0';
        c += 2;
        *c++ = ((b & 0x08) == 0) + '0';
        *c++ = ((b & 0x04) == 0) + '0';
        *c++ = ((b & 0x02) == 0) + '0';
        *c++ = ((b & 0x01) == 0) + '0';
        c += 2;
    }

    /* 电源排版并发送 */
    c = uartBuff + 35;
    for (i = 0; i < sizeof(pwr_pins); i++)
    {
        c[i] = pwr_pins[i];
    }
    UART1_SendString(uartBuff, sizeof(uartBuff) - 1);
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

    /* 配置串口调试 */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

    /* 配置定时器0 */
    TMR0_TimerInit(FREQ_SYS / 1000);       // 定时时间 1ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
    PFIC_EnableIRQ(TMR0_IRQn);

    /* rgb初始化 */
    port_rgbarr_init();
    rgb0 = kb_rgbarr_get_next(NULL); // 获取默认RGB阵列
    rgb0->init(rgb0);

    /* SPI 0 */
    GPIOA_SetBits(GPIO_Pin_12);
    GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    SPI0_MasterDefInit();
    SPI0_CLKCfg(60);                     // 60M/1M
    SPI0_DataMode(Mode0_HighBitINFront); // SCLK空闲高电平 高位在前
    R8_SPI0_CTRL_MOD &= ~RB_SPI_MOSI_OE; // MOSI不输出

    /* 软件定时器 */
    static stim_t t_key;
    stim_init(&t_key, 1, STIM_FLAG_PERIOD);
    stim_callback(&t_key, t_key_cb);
    stim_start(&t_key);
    static stim_t t_rgb;
    stim_init(&t_rgb, 30, STIM_FLAG_PERIOD);
    stim_callback(&t_rgb, t_rgb_cb);
    stim_start(&t_rgb);

    /* 电源测试 */
    power_test_init();

    while (1)
    {
        uint8_t is_print = 0;
        /* KEY */
        if (t_key_to)
        {
            t_key_to = 0;
            if (pwr_chip) // 如果电源开启
            {
                int i;

                /* 切换buff用于比较按键变化 */
                spiBuff = (spiBuff == keyBuff) ? keyBuff + KEY_BYTE : keyBuff;

                /* 启动SPI接收寄存器数据 */
                GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA); // 输出
                GPIOA_ResetBits(GPIO_Pin_14);
                GPIOA_SetBits(GPIO_Pin_14);
                GPIOA_ResetBits(GPIO_Pin_12);
                SPI0_MasterRecv(spiBuff, KEY_BYTE);
                GPIOA_SetBits(GPIO_Pin_12);

                /* 检查按键变化 */
                uint8_t *c  = keyBuff;
                uint8_t *c1 = keyBuff + KEY_BYTE;
                for (i = 0; i < KEY_BYTE; i++)
                {
                    if (*c++ != *c1++)
                        break;
                }
                /* 如果按键发生变化则更新串口显示 */
                if (i != KEY_BYTE)
                    is_print = 1;
            }
            else
            {
                GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeIN_Floating); // 浮空 避免给LED及相关电路供电
            }
        }
        /* RGB */
        if (t_rgb_to)
        {
            t_rgb_to = 0;
            update_rgb();
            if (pwr_led) // 如果电源开启
            {
                GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // 输出
                rgb0->disp(rgb0, rgb0->buff, rgb0->size);
            }
            else
            {
                GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_Floating); // 浮空 避免给LED及相关电路供电
            }
        }
        /* PWR */
        if (power_test_handle())
            is_print = 1;
        /* PRINT */
        if (is_print)
            print_message();
    }
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void) // TMR0 定时中断
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // 清除中断标志
        stim_handle();                       // 软件定时器
    }
}
