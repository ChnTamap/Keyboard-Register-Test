/**
 * @file kb_interface.h
 * @author Tamap
 * @brief 键盘板级驱动接口
 * @version 0.1
 * @date 2024-01-20
 *
 * @copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 */

#if !defined(__KB_INTERFACE_H)
#define __KB_INTERFACE_H

#include "kb_def.h"

/**
 * @brief RGB阵列接口类
 */
struct inf_rgbarr
{
    /* 链表 */
    struct inf_rgbarr *next; /**< 链表的下一个 */
    /* RGB布局 */
    uint16_t led_num; /**< 布局相关参数, 暂时用一个LED数量代替 */
    /* 缓冲区 */
    ra_pix_t *buff; /**< 缓冲区: 由应用层渲染出结果, 再由接口层转为外设数据 */
    uint32_t size;  /**< 缓冲区可用大小 */
    /* 函数接口 */
    void (*init)(struct inf_rgbarr *inf);                                /**< 初始化 */
    void (*disp)(struct inf_rgbarr *inf, ra_pix_t *buff, uint32_t size); /**< 显示 */
    /* 回调函数 */
    // void (*finish)(struct inf_rgbarr *inf); /**< 发送完成回调 */
};
typedef struct inf_rgbarr inf_rgbarr_t;

/**
 * @brief 注册RGB阵列
 * @param inf rgbarr接口
 */
void kb_rgbarr_register(inf_rgbarr_t *inf);

/**
 * @brief 获取下一个RGB阵列
 * @param inf 前一个对象(NULL则表示获取默认对象)
 * @return inf_rgbarr_t* 对象指针
 * @retval NULL 找不到
 */
inf_rgbarr_t *kb_rgbarr_get_next(inf_rgbarr_t *inf);

#endif // __KB_INTERFACE_H
