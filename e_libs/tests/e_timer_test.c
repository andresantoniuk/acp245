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

#include "e_check.h"
#include "e_time.h"
#include "e_port.h"

#ifdef E_LIBS_TIMER_ENABLE

#include <unistd.h>
typedef u8 u_char;
#include <event.h>
#include <errno.h>
#include <string.h>

static struct timeval _ts;
static u8 _cnt;
static u8 _target;

static void _handler(e_timer tmr, void *ctx) {
    struct timeval now;
    struct timeval diff;
    E_DBG("Handling");

    e_assert(_cnt <= _target);

    E_DBG("Getting time of day");
    if(gettimeofday(&now, NULL)) {
        E_ERR("gettimeofday failed");
    }

    E_DBG("Now: %lu,%lu", (unsigned long) now.tv_sec, (unsigned long) now.tv_usec);
    e_assert(now.tv_sec > 0);
    e_assert(50 == (*((u8*)ctx)));

    _cnt++;

    if (_cnt == _target) {
        E_DBG("Freeing");
        e_timer_free(&tmr);
        E_DBG("Freed '%d'", tmr == NULL);
        e_assert(tmr == NULL);
        event_loopexit(0);
    }

    diff.tv_usec = ((100 * _cnt) % 1000)*1000;
    diff.tv_sec = (100 * _cnt)/1000;

    e_assert(e_time_cmp(&diff, &now) < 0);
}

START_TEST (test_e_timer)
{
    e_ret rc;
    e_timer tmr;
    u8 ctx;
    ctx = 50;

    E_LOG_SET_LEVEL(NONE);

    event_init();

    if(gettimeofday(&_ts, NULL)) {
        FAIL("gettimeofday failed");
    }

    rc = e_timer_create(&tmr, 10000, E_TIMER_100_MSEC, _handler, &ctx, 0);
    NOT_NULL(tmr);
    ARE_EQ_INT(OK, rc);
    e_timer_free(&tmr);
    IS_NULL(tmr);

    _cnt = 0;
    _target = 1;
    rc = e_timer_create(&tmr, 1, E_TIMER_100_MSEC, _handler, &ctx, 0);
    NOT_NULL(tmr);
    ARE_EQ_INT(OK, rc);
    ARE_EQ_INT(0, _cnt);

    event_dispatch();

    _cnt = 0;
    _target = 5;
    rc = e_timer_create(&tmr, 1, E_TIMER_100_MSEC, _handler, &ctx, E_TIMER_PERIODIC);
    NOT_NULL(tmr);
    ARE_EQ_INT(OK, rc);
    ARE_EQ_INT(0, _cnt);

    event_dispatch();
}
END_TEST

#endif /* E_LIBS_TIMER_ENABLE */

extern int e_timer_test (void);
int e_timer_test (void) {
    return e_check_run_suite("e_timer",
#ifdef E_LIBS_TIMER_ENABLE
            test_e_timer,
#endif /* E_LIBS_TIMER_ENABLE */
            NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
	return e_timer_test();
}
#endif
