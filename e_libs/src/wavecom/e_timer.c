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

#include "adl_global.h"

#include "e_port.h"
#include "e_mem.h"
#include "e_errors.h"

/* UGLY: wrapper functions are too expensive */

struct _e_timer {
    adl_tmr_t *tmr;
    e_timer_handler hndl;
    void * ctx;
    bool periodic;
    bool running;
};
static bool _init = FALSE;
static struct _e_timer _timers[E_TIMER_MAX];

static void _tmr_hdnl(u8 id, void *ctx) {
    e_timer tmr = (e_timer) ctx;
    e_assert(tmr != NULL);
    e_assert(tmr >= &_timers[0] && tmr <= &_timers[E_TIMER_MAX-1]);

    E_TRACE("Running timer: %p %p", tmr->hndl, tmr->ctx);
    tmr->running = TRUE;
    tmr->hndl(tmr, tmr->ctx);
    tmr->running = FALSE;

    if (!tmr->periodic) {
        E_TRACE("timer released after running: %p %p", tmr->hndl, tmr->ctx);
        (void) e_mem_set(tmr, 0, sizeof(struct _e_timer));
    }
}

e_ret e_timer_create(e_timer *timer, u32 val, e_timer_type type,
        e_timer_handler hndl, void *ctx, u8 flags) {
    u8 i;

    e_assert(timer != NULL);

    *timer = NULL;

    if (!_init) {
        _init = TRUE;
        (void) e_mem_set(_timers, 0, sizeof(_timers));
    }

    for (i = 0; i < E_TIMER_MAX; i++) {
        if (NULL == _timers[i].tmr) {
            break;
        }
    }

    if (E_TIMER_MAX == i) {
        E_DBG("no more e_timers");
        return ERROR;
    }

    if (E_TIMER_1_SEC == type) {
        val *= 10;
    }

    E_TRACE("Adding timer %d of %lu ms. periodic=%u, strict=%u",
            (int) i,
            (unsigned long) val * 100,
            (unsigned int) (flags & E_TIMER_PERIODIC) != 0,
            (unsigned int) (flags & E_TIMER_STRICT) != 0);

    _timers[i].hndl = hndl;
    _timers[i].ctx = ctx;
    _timers[i].periodic = (flags & E_TIMER_PERIODIC) != 0;
    _timers[i].running = FALSE;

    _timers[i].tmr = adl_tmrSubscribeExt(
            _timers[i].periodic ?
                ADL_TMR_CYCLIC_OPT_ON_EXPIRATION :
                ADL_TMR_CYCLIC_OPT_NONE,
            val == 0 ? (u32) 1 : val, val == 0 ? ADL_TMR_TYPE_TICK : ADL_TMR_TYPE_100MS,
            _tmr_hdnl, &_timers[i], ((flags & E_TIMER_STRICT)!=0));

    if ((NULL == _timers[i].tmr) || (_timers[i].tmr < 0)) {
        ERR_LOG(adl_tmrSubscribeExt, _timers[i].tmr);
        (void) e_mem_set(&_timers[i], 0, sizeof(struct _e_timer));
        return ERROR;
    }

    *timer = &_timers[i];
    return OK;
}

bool e_timer_is_periodic(e_timer timer) {
    e_assert(timer != NULL);

    return timer->periodic;
}

void e_timer_free(e_timer *timer) {
    s32 rc;
    if (timer == NULL || *timer == NULL) {
        return;
    }
    e_assert((*timer)->tmr != NULL);
    e_assert((*timer) >= &_timers[0] && (*timer) <= &_timers[E_TIMER_MAX-1]);

    if((*timer)->running){
        /* this condition can be meet if e_timer_free
         * is called from inside the timer handler
         */
        if ((*timer)->periodic) {
            if((rc = adl_tmrUnSubscribe((*timer)->tmr, _tmr_hdnl,
                            ADL_TMR_TYPE_100MS)) < 0) {
                ERR_LOG(adl_tmrUnSubscribe, rc);
            }
            /* clear periodic so memory it's freed after running */
            (*timer)->periodic = FALSE;
        } /* else, will be discarded after running */
    } else {
        E_TRACE("Canceling timer %p", (*timer)->hndl);
        if((rc = adl_tmrUnSubscribe((*timer)->tmr, _tmr_hdnl,
                        ADL_TMR_TYPE_100MS)) < 0) {
            ERR_LOG(adl_tmrUnSubscribe, rc);
        }
        (void) e_mem_set((*timer), 0, sizeof(struct _e_timer));
    }

    *timer = NULL;
}
