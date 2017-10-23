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
#include "e_syslog.h"

#include "e_check.h"
#include "e_port.h"
#include "e_log.h"
#include "e_timer.h"

#ifdef E_LIBS_NET_ENABLE

#include <unistd.h>
typedef u8 u_char;
#include <event.h>
#include <errno.h>
#include <string.h>

static u8 _cnt;

static void _handler_stop(e_timer tmr, void *ctx) {
    event_loopexit(0);
}

static void _handler_syslog(e_timer tmr, void *ctx) {
    e_ret rc;
    rc = e_syslog(E_SYSLOG_ERROR, "foo");
    ARE_EQ_INT(OK, rc);
    _cnt--;
    if (!_cnt) {
        e_timer_free(&tmr);
        e_syslog_closelog();
    }
}

START_TEST (test_e_syslog)
{
    e_ret rc;
    e_timer tmr;

    event_init();
    E_LOG_SET_LEVEL(TRACE);
    rc = e_syslog_openlog("test", E_SYSLOG_LOCAL_3, "127.0.0.1", 514, 0);
    ARE_EQ_INT(OK, rc);

    _cnt = 5;
    rc = e_timer_create(&tmr, 1, E_TIMER_100_MSEC, _handler_syslog, NULL, E_TIMER_PERIODIC);

    rc = e_timer_create(&tmr, 10, E_TIMER_100_MSEC, _handler_stop, NULL, 0);

    event_dispatch();

    ARE_EQ_INT(0, _cnt);
}
END_TEST

#endif /* E_LIBS_NET_ENABLE */

extern int e_syslog_test (void);
int e_syslog_test (void) {
    return e_check_run_suite("e_syslog",
#ifdef E_LIBS_NET_ENABLE
            test_e_syslog,
#endif /* E_LIBS_NET_ENABLE */
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
	return e_syslog_test();
}
#endif
