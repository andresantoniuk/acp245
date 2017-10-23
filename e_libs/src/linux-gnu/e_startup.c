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

#include "e_startup.h"

#ifdef E_LIBS_STARTUP_ENABLE

#include <sys/types.h>
#include <signal.h>

typedef u8 u_char;
#include <event.h>
#include <stdio.h>

#include "e_port.h"
#include "e_log.h"

/* ISO C forbids conversion of object pointer to function pointer type */
struct _cb {
    e_startup_callback cb;
};

static struct event_base* _eb = NULL;

static void _sigint_cb(int fd, short event, void *arg) {
    struct timeval tv;
    struct _cb *cb;

    E_INFO("Got SIGINT, stopping event loop");

    cb = (struct _cb *) arg;
    cb->cb();

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    event_loopexit(&tv);
}

static void _call_startup_cb(int fd, short event, void *arg) {
    struct _cb *cb;

    E_DBG("Calling startup function");
    cb = (struct _cb *) arg;
    cb->cb();
}

static void _sigabrt_cb(int fd, short event, void *arg) {
    struct timeval tv;

    printf("SIGABRT received\n");

    tv.tv_sec = 0;
    tv.tv_usec = 1;
    event_loopexit(&tv);
}

void e_startup(e_startup_callback cb, e_startup_callback endcb)
{
    struct event sig_int;
    struct event sig_abrt;
    struct timeval tv;
    struct _cb cb_start;
    struct _cb cb_end;

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    E_DBG("Initializing event loop");
    _eb = event_init();
    if (!_eb) {
        e_abort(0, "Event loop init failed");
    }
    cb_start.cb = cb;
    cb_end.cb = endcb;

    event_set(&sig_int, SIGINT, EV_SIGNAL|EV_PERSIST, _sigint_cb,
              &cb_end);
    if(event_add(&sig_int, NULL)) {
        e_abort(0, "Failed registering SIGINT signal");
    }
    event_set(&sig_abrt, SIGABRT, EV_SIGNAL|EV_PERSIST, _sigabrt_cb,
              NULL);
    if(event_add(&sig_abrt, NULL)) {
        E_ERR("Failed registering SIGABRT signal");
    }

    event_once(-1, EV_TIMEOUT, _call_startup_cb, &cb_start, &tv);

    E_DBG("Running event loop");
    event_dispatch();

    E_DBG("Exited event loop");
    event_del(&sig_int);
    event_del(&sig_abrt);
    event_base_free(NULL);
}

#endif /* E_LIBS_STARTUP_ENABLE */

