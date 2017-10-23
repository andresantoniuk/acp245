/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_timer_h_
#define __e_timer_h_

#include "e_port.h"

#ifdef E_LIBS_TIMER_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

#define E_TIMER_MAX     32

#define E_TIMER_MAX_MILLIS      0xFFFFF
#define E_TIMER_MAX_SECS        0xFFFFFF
#define E_TIMER_MAX_WAIT        (u32)(0xFFFFFF)*1000

#define E_TIMER_STRICT          ((u8)1<<0)
#define E_TIMER_PERIODIC        ((u8)1<<1)

typedef enum e_timer_type {
    E_TIMER_1_SEC,
    E_TIMER_100_MSEC
} e_timer_type;

typedef struct _e_timer *e_timer;

typedef void (*e_timer_handler)(e_timer timer, void *context);

extern e_ret e_timer_create(e_timer *timer, u32 val,
        e_timer_type type, e_timer_handler hndl, void *ctx, u8 flags);

extern bool e_timer_is_periodic(e_timer timer);

extern void e_timer_free(e_timer *timer);

#ifdef __cplusplus
}
#endif

#endif /* E_LIBS_TIMER_ENABLE */

#endif
