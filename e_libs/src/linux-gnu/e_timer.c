/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_timer.h"

#include "e_port.h"
#include "e_log.h"
#include "e_time.h"
#include "e_mem.h"

#ifdef E_LIBS_TIMER_ENABLE

#include <errno.h>
#include <string.h>
#include <stdlib.h>

typedef u8 u_char;
#include <event.h>

#define _PERIODIC 0x01
#define _ENABLED  0x02
#define _RUNNING  0x04
#define _FREED    0x08

#define _MAGIC  0x3F0C
struct _e_timer {
    u16 magic;
    u8 idx;
    u8 flags;

    struct timeval delay;
    void *ctx;
    e_timer_handler hndl;
    struct event event;
};
struct _e_timer _timers[E_TIMER_MAX];

static void _timer_release(e_timer *timer) {
    _timers[(*timer)->idx].flags = 0;
    _timers[(*timer)->idx].magic = 0;
    (void) e_mem_set(&_timers[(*timer)->idx], 0, sizeof(struct _e_timer));
    *timer = NULL;
}

static void _timer_handler(int fd, short event, void *arg)
{
    e_timer tmr;

    e_assert(arg != NULL);
    e_assert(_MAGIC == ((e_timer)arg)->magic);
    e_assert(((e_timer)arg)->flags & _ENABLED);

    tmr = (e_timer) arg;

    if (event & EV_TIMEOUT) {
        tmr->flags |= _RUNNING;
        tmr->hndl(tmr, tmr->ctx);
        tmr->flags &= ~_RUNNING;
        if (tmr->flags & _FREED) {
            _timer_release(&tmr);
        } else {
            if (tmr->flags & _PERIODIC) {
                evtimer_set(&tmr->event, _timer_handler, tmr);
                if(evtimer_add(&tmr->event, &tmr->delay)) {
                    E_FATAL("Failed registering periodic timer: %s", strerror(errno));
                    e_assert(FALSE);
                }
            } else {
                _timer_release(&tmr);
            }
        }
    } else {
        E_DBG("Signal event, release timer");
        _timer_release(&tmr);
    }
}

e_ret e_timer_create(e_timer *timer, u32 val, e_timer_type type, e_timer_handler hndl, void *ctx, u8 flags) {
    int i;
    e_ret rc;

    e_assert(timer != NULL);
    e_assert(((type == E_TIMER_1_SEC) && (val <= E_TIMER_MAX_SECS)) ||
            ((type == E_TIMER_100_MSEC) && (val <= E_TIMER_MAX_MILLIS)));

    *timer = NULL;
    rc = ERROR;

    for (i = 0; i < E_TIMER_MAX; i++) {
        if (!(_timers[i].flags & _ENABLED)) {
            *timer = &_timers[i];
            (*timer)->magic = _MAGIC;
            (*timer)->idx = i;
            _timers[i].flags = 0;
            if (flags & E_TIMER_PERIODIC) {
                (*timer)->flags |= _PERIODIC;
            }
            if (0 == val) {
                (*timer)->delay.tv_sec = 0;
                (*timer)->delay.tv_usec = 1;
            } else if (E_TIMER_1_SEC == type) {
                (*timer)->delay.tv_sec = val;
                (*timer)->delay.tv_usec = 0;
            } else if (E_TIMER_100_MSEC == type) {
                (*timer)->delay.tv_sec = val / 10;
                (*timer)->delay.tv_usec = (val % 10) * 10000;
            }
            (*timer)->ctx = ctx;
            (*timer)->hndl = hndl;
            (*timer)->flags |= _ENABLED;

            evtimer_set(&(*timer)->event, _timer_handler, (*timer));
            if(evtimer_add(&(*timer)->event, &(*timer)->delay)) {
                (*timer)->flags &= ~_ENABLED;
                E_ERR("Failed registering timer");
                e_assert(FALSE);
                goto exit;
            }

            E_DBG("Added timer, delay=%lu.%lu",
                    (unsigned long) (*timer)->delay.tv_sec,
                    (unsigned long) (*timer)->delay.tv_usec);
            break;
        }
    }

    if (NULL == *timer) {
        goto exit;
    }

    rc = OK;
exit:
    return rc;
}

bool e_timer_is_periodic(e_timer timer) {
    e_assert(timer != NULL);

    return (timer->flags & _PERIODIC);
}

void e_timer_free(e_timer *timer) {
    if(timer == NULL || *timer == NULL) {
        return;
    }
    if ((*timer)->flags & _RUNNING) {
        (*timer)->flags |= _FREED;
        *timer = NULL;
    } else {
        if(evtimer_del(&(*timer)->event)) {
            E_DBG("Failed deleting timer event");
            e_assert(FALSE);
        }
        _timer_release(timer);
    }

    e_assert(*timer == NULL);
}

#endif /* E_LIBS_TIMER_ENABLE */

