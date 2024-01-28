/**
 * @file stimer.h
 * @author Tamap
 * @brief Software timer
 * @version 0.1
 * @date 2024-01-25
 *
 * @copyright Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */
#if !defined(__STIMER_H)
#define __STIMER_H

#include <stddef.h>
#include <stdint.h>           // >C99
typedef int32_t stim_tick_t; /**< type of tick */

/** @defgroup STIM_FLAG
 * @{
 */
#define STIM_FLAG_PERIOD  0 /**< period */
#define STIM_FLAG_ONESHOT 1 /**< one shot */
/** @} */

/** @defgroup STIM_STATE
 * @{
 */
#define STIM_STATE_STOP 0 /**< stop */
#define STIM_STATE_RUN  1 /**< run */
/** @} */

struct stim;
typedef void (*stim_cb_t)(struct stim *); /**< callback funciton typedef */

/**
 * @brief software timer object
 */
typedef struct stim
{
    /* ll */
    struct stim *next; /**< link list */
    /* timer */
    stim_tick_t period;  /**< timer period tick */
    stim_tick_t timeout; /**< timeout tick */
    uint8_t flag;        /**< flag */
    uint8_t state;       /**< state */
    /* callback */
    stim_cb_t cb;    /**< callback function of timeout */
    void *user_data; /**< user data */
} stim_t;

/**
 * @brief Call this function per tick
 * for example, in interrupt irq
 */
void stim_handle(void);

/**
 * @brief init software timer and add to link list
 *
 * @param t object of stim
 * @param period period tick
 * @param flag flag @see STIM_FLAG
 */
void stim_init(stim_t *t, stim_tick_t period, uint8_t flag);
/**
 * @brief deinit software timer and remove from link list
 * @param t object of stim
 */
void stim_deinit(stim_t *t);
/**
 * @brief start or restart timer
 * @param t stim
 */
void stim_start(stim_t *t);
/**
 * @brief stop timer
 * @param t stim
 */
void stim_stop(stim_t *t);
/**
 * @brief set period of stim
 * @param t stim
 * @param period period tick
 */
void stim_set_period(stim_t *t, stim_tick_t period);
/**
 * @brief set flag of stim
 * @param t stim
 * @param flag flag @see STIM_FLAG
 */
void stim_set_flag(stim_t *t, stim_tick_t flag);
/**
 * @brief set callback when timer timeout
 * @param t stim
 * @param cb callback function @see stim_cb_t
 */
void stim_callback(stim_t *t, stim_cb_t cb);
/**
 * @brief set user data of stim
 * @param t stim
 * @param user_data user data
 */
void stim_set_data(stim_t *t, void *user_data);
/**
 * @brief get user data of stim
 * @param t stim
 * @return void* : user data
 */
void *stim_get_data(stim_t *t);

#endif // __STIMER_H
