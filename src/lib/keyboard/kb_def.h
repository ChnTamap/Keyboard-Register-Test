/**
 * @file kb_def.h
 * @author Tamap
 * @brief 变量类型等定义
 * @version 0.1
 * @date 2024-01-21
 *
 * @copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 */
#if !defined(__KB_DEF_H)
#define __KB_DEF_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief RGB矩阵像素类型: RGBA888
 */
struct ra_rgba888
{
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef struct ra_rgba888 ra_pix_t; /**< 定义像素类型 */

#endif // __KB_DEF_H
