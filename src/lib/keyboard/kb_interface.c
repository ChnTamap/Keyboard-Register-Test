/**
 * @file kb_interface.c
 * @author Tamap
 * @brief 键盘板级驱动接口
 * @version 0.1
 * @date 2024-01-20
 * @details
 * 暂时先用RGB888进行渲染, 然后再转换成对应的数据
 * 后续根据需要再迭代重写
 *
 * @copyright (c) 2024 Tamap
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kb_interface.h"

static inf_rgbarr_t *rgbarr_def = NULL;

/**
 * @brief 注册RGB阵列
 * @param inf rgbarr接口
 */
void kb_rgbarr_register(inf_rgbarr_t *inf)
{
    if (rgbarr_def)
    {
        inf_rgbarr_t *node = rgbarr_def;
        while (node->next)
            node = node->next;
        node->next = inf;
    }
    else
    {
        rgbarr_def = inf;
    }
}

/**
 * @brief 获取下一个RGB阵列
 * @param inf 前一个对象(NULL则表示获取默认对象)
 * @return inf_rgbarr_t* 对象指针
 * @retval NULL 找不到
 */
inf_rgbarr_t *kb_rgbarr_get_next(inf_rgbarr_t *inf)
{
    if (inf)
        return inf->next;
    return rgbarr_def;
}
