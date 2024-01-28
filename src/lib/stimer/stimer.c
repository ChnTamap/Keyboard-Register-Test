/**
 * @file stimer.c
 * @author Tamap
 * @brief Software timer
 * @version 0.1
 * @date 2024-01-25
 *
 * @copyright Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */

#include "stimer.h"

static stim_t *stim_ll_def       = NULL; /**< default link list */
static stim_tick_t stim_tick_def = 0;    /**< default tick counter */

void stim_handle(void)
{
    stim_tick_def++; // Add tick counter
    /* Handle software timer */
    for (stim_t *t = stim_ll_def; t; t = t->next)
    {
        /* When timer is running and timeout */
        if (t->state == STIM_STATE_RUN && (t->timeout - stim_tick_def) <= 0)
        {
            /* Call callback function */
            if (t->cb)
                t->cb(t);
            /* set next timeout tick or stop timer*/
            if (t->flag == STIM_FLAG_ONESHOT)
                t->state = STIM_STATE_STOP;
            else
                t->timeout = stim_tick_def + t->period;
        }
    }
}

void stim_init(stim_t *t, stim_tick_t period, uint8_t flag)
{
    /* Init */
    t->period    = period;
    t->flag      = flag;
    t->cb        = NULL;
    t->user_data = NULL;
    t->state     = STIM_STATE_STOP;
    /* Add to link list */
    t->next     = stim_ll_def;
    stim_ll_def = t;
}
void stim_deinit(stim_t *t)
{
    /* Remove from link list */
    stim_t **n = &stim_ll_def;
    while (*n && *n != t)
        n = &(*n)->next;
    if (*n == t)
    {
        *n      = t->next;
        t->next = NULL;
    }
}
void stim_start(stim_t *t)
{
    t->timeout = stim_tick_def + t->period;
    t->state   = STIM_STATE_RUN;
}
void stim_stop(stim_t *t)
{
    t->state = STIM_STATE_STOP;
}
void stim_set_period(stim_t *t, stim_tick_t period)
{
    t->period = period;
}
void stim_set_flag(stim_t *t, stim_tick_t flag)
{
    t->flag = flag;
}
void stim_callback(stim_t *t, stim_cb_t cb)
{
    t->cb = cb;
}
void stim_set_data(stim_t *t, void *user_data)
{
    t->user_data = user_data;
}
void *stim_get_data(stim_t *t)
{
    return t->user_data;
}
