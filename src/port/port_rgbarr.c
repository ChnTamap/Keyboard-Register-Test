/**
 * @file port_rgbarr.c
 * @author Tamap
 * @brief RGB阵列接口驱动程序(WS2812B)
 * @version 0.1
 * @date 2024-01-23
 *
 * @copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kb_interface.h> /* 接口头文件 */
#include "CH58x_common.h" /* 芯片头文件 */

/* =====================参数选择====================== */
/* 此处应作为一个CH58x的通用驱动文件存在, 暂时直接写在文件内 */
/**
 * 时序选择, 请根据实际系统主频和性能要求选择串口频率和位数
 * 注意: 带'!'的选项并不严格满足时序或比较极限, 但经测试可以运行
 */
// #define RGBT2_N61_300ns /*    2:1 34.7k LED/s 中断周期19.2us */
// #define RGBT3_N81_300ns /* (!)3:1 41.7k LED/s 中断周期24us */
// #define RGBT3_N81_333ns /* (!)3:1 37.5k LED/s 中断周期26.64us */
// #define RGBT3_N81_400ns /* (!)3:1 31.3k LED/s 中断周期32us */
// #define RGBT3_E81_300ns /* (!)3:1 37.9k LED/s 中断周期 26.4us */
// #define RGBT3_E81_333ns /* (!)3:1 34.1k LED/s 中断周期29.3us */
#define RGBT3_E81_400ns /* (!)3:1 28.4k LED/s 中断周期35.2us */

#if defined(RGBT2_N61_300ns)
#define RGB_FSYS     CLK_SOURCE_PLL_80MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_80MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_48MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_60MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_80MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_48MHz
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
#define RGB_FSYS     CLK_SOURCE_PLL_60MHz
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

/* ========================布局======================== */
/* 此处有布局相关的包含或定义...> */
/* 暂时用这些宏定义简单定义 */
#define RGB_NUM      16                         // RGBLED的数量
#define BYTE_PER_RGB (24 / BIT_PER_BYTE)        // X byte/LED
#define BIT_NUM      ((BYTE_PER_RGB) * RGB_NUM) // DATA
__attribute__((aligned(4))) uint8_t TxBuff[BIT_NUM + 1];

/* ========================程序======================== */

/**
 * @brief 板级rgb驱动程序接口, 继承自 @ref inf_rgbarr_t
 */
typedef struct inf_rgbarr_uartit
{
    inf_rgbarr_t inf; /**< 接口类 */
    /* Var */
    uint8_t *txbuf;  /**< 原始Buffer: 将后半部分给接口, 避免转换时覆盖有用数据*/
    uint8_t *tx_ptr; /**< 发送指针 */
    uint16_t tx_len; /**< 剩余发送数量 */
    /* GPIO */
    /* UARTx */
} inf_rgbarr_uartit_t;

/**
 * @brief 从inf指针获取uartit指针
 */
#define get_uartit_form_inf(_p) ((inf_rgbarr_uartit_t *)_p)
// #define get_uartit_form_inf(_p) ((inf_rgbarr_uartit_t *)((uintptr_t)(_p) - (uintptr_t)((inf_rgbarr_uartit_t *)0)->inf))

/**
 * @brief WS2812B像素联合体
 */
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

/**
 * @brief   GPIOx端口引脚输出置高/低
 *
 * @param   reg     - R32_Px_OUT/CLR
 * @param   pin     - PA0-PA15
 */
#define GPIOx_WriteBits(reg, pin) (*(reg) |= pin)

/**
 * @brief 初始化接口函数
 */
static void rgb_init(inf_rgbarr_t *inf)
{
    // inf_rgbarr_uartit_t *_inf = get_uartit_form_inf(inf);

    // 判断当前系统时钟是否能准确分频, 否则打印警告信息
    // uint32_t sysclk = GetSysClock();
    /* 配置串口1：先配置IO口模式，再配置串口 */
    GPIOB_SetBits(GPIO_Pin_7);
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART0_DefInit();
    UART0_BaudRateCfg(RGB_BAUDRATE);
    R8_UART0_LCR = RGB_UART_LCR; // 串口数据位
    PFIC_EnableIRQ(UART0_IRQn);
}

/**
 * @brief 发送RGB数据接口函数
 */
static void rgb_disp(inf_rgbarr_t *inf, ra_pix_t *buff, uint32_t size)
{
    inf_rgbarr_uartit_t *_inf = get_uartit_form_inf(inf);

    /* 转换数据 rgb888->ws2812b(uartit) */
    ra_pix_t *pix = inf->buff;
    uint8_t *b    = _inf->txbuf;
    for (int i = 0; i < RGB_NUM; i++)
    {
        /* 转换为WS2812格式 */
        color_t rgb = {.c = {.g = pix->g, .b = pix->b, .r = pix->r}};
        for (int j = 0; j < BYTE_PER_RGB; j++)
        {
            rgb.dw <<= BIT_PER_BYTE;
#define BIT_MASK ((1 << BIT_PER_BYTE) - 1)
            *b++ = bit_table[rgb.c.x & BIT_MASK];
        }
        /* 下一个像素 */
        pix++;
    }

    /* 发送 */
    _inf->txbuf[BIT_NUM] = 0x00;
    b                    = _inf->txbuf;
    /* 先发几个数据, 顺便等第一个字节进入起始位 */
    R8_UART0_THR = *b++;
    GPIOB_SetBits(GPIO_Pin_8); // Debug
    R8_UART0_THR = *b++;
    /* 起始位及后续的低电平期间, 使能TX以平滑过渡 */
    R8_UART0_IER = RB_IER_TXD_EN;
    GPIOB_SetBits(GPIO_Pin_7);
    R8_UART0_THR = *b++;
    R8_UART0_THR = *b++;
    R8_UART0_THR = *b++;
    R8_UART0_THR = *b++;
    R8_UART0_THR = *b++;
    R8_UART0_THR = *b++;
    _inf->tx_len = sizeof(TxBuff) - 8;
    _inf->tx_ptr = b;
    UART0_INTCfg(ENABLE, RB_IER_THR_EMPTY);
}

#define BUFSZ (RGB_NUM * sizeof(ra_pix_t)) /**< 定义给应用层的Buff大小 */
/**
 * @brief 驱动层rgbarr结构体
 */
static inf_rgbarr_uartit_t rgb = {
    .inf = {
        // .布局 = ...,
        .led_num = RGB_NUM,
        .buff    = (ra_pix_t *)(TxBuff + (sizeof(TxBuff) - BUFSZ)),
        .size    = BUFSZ,
        .init    = rgb_init,
        .disp    = rgb_disp,
    },
    .txbuf = TxBuff,
};

/**
 * @brief RGB阵列接口驱动初始化函数, 此函数应当在"未确定"初始化被调用
 */
void port_rgbarr_init(void)
{
    SetSysClock(RGB_FSYS);        // TODO: 不再设置系统时钟, 改为判断系统时钟是否能准确生成波特率
    kb_rgbarr_register(&rgb.inf); // 注册
}

/**
 * @fn rgb_isr_send
 * @brief RGB发送缓冲区空中断处理函数
 */
static void rgb_isr_send(inf_rgbarr_uartit_t *_inf)
{
    if (_inf->tx_len == 0)
    {
        /* 最后一个字节是无效字节00, 用于过渡低电平 */
        GPIOB_ResetBits(GPIO_Pin_7);
        GPIOB_ResetBits(GPIO_Pin_8); // Debug
        R8_UART0_IER &= ~RB_IER_TXD_EN;
        UART0_INTCfg(DISABLE, RB_IER_THR_EMPTY);
        return;
    }
    int n = (_inf->tx_len > 7) ? 8 : _inf->tx_len;
    switch (n)
    {
    case 8: R8_UART0_THR = *(_inf->tx_ptr++);
    case 7: R8_UART0_THR = *(_inf->tx_ptr++);
    case 6: R8_UART0_THR = *(_inf->tx_ptr++);
    case 5: R8_UART0_THR = *(_inf->tx_ptr++);
    case 4: R8_UART0_THR = *(_inf->tx_ptr++);
    case 3: R8_UART0_THR = *(_inf->tx_ptr++);
    case 2: R8_UART0_THR = *(_inf->tx_ptr++);
    case 1: R8_UART0_THR = *(_inf->tx_ptr++);
    default: break;
    }
    _inf->tx_len -= n;
    GPIOB_SetBits(GPIO_Pin_8); // Debug
}

/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    GPIOB_ResetBits(GPIO_Pin_8); // Debug
    switch (UART0_GetITFlag())
    {
    case UART_II_LINE_STAT: // 线路状态错误
        break;

    case UART_II_RECV_RDY: // 数据达到设置触发点
        break;

    case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
        break;

    case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
        rgb_isr_send(&rgb);
        break;

    case UART_II_MODEM_CHG: // 只支持串口0
        break;

    default:
        break;
    }
}