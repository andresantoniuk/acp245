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

#ifdef E_LIBS_NET_ENABLE

static int _level = E_SYSLOG_ERROR;

void e_syslog_set_level(int level) {
    _level = level;
}

#ifdef HAVE_SYSLOG
#include <syslog.h>
static bool _init = FALSE;

e_ret e_syslog_openlog(const char *ident, int option, int facility) {
    openlog(ident, LOG_CONS, facility);
    return OK;
}

void e_syslog_closelog(void) {
    closelog();
}

e_ret e_syslog(int prio, const char* str) {
    if (prio <= _level) {
        syslog(prio, str);
    }
    return OK;
}

e_ret e_syslog_set_config(const ascii *host, const u16 port, const u16 local_port) {
    return OK;
}
#else

#include "e_client.h"
#include "e_buff.h"
#include "e_log.h"
#include "e_mem.h"
#include "e_time.h"
#include "e_timer.h"

static u8 _data[E_SYSLOG_BUFSIZ];
static e_buff _buff;
static e_client _client = NULL;
static ascii *_ident = NULL;
static e_client_addr *_addr = NULL;
static u16 _local_port;
static int _facility;
/*static const ascii *_hostname = "e_libs";*/
static bool _logging = FALSE;

typedef enum {
    _CONNECTING,
    _CONNECTED,
    _DISCONNECTING,
    _DISCONNECTED,
    _CLOSED
} _states;
static _states _state = _CLOSED;
static e_timer _connect_timer = NULL;
static e_timer _write_pending_timer = NULL;

static u16 _connect_timeout = 1;

static void _connect(void)
{
    if (_state == _CONNECTED || _state == _CONNECTING || _state == _DISCONNECTING) {
        return;
    }

    e_assert(_addr != NULL);
    e_assert(_client != NULL);

    if(e_client_open(_client, E_CLIENT_UDP, _addr, _local_port)) {
        _state = _DISCONNECTED;
        E_WARN("Connection to syslog server failed");
    } else if (e_client_is_connected(_client)) {
        _state = _CONNECTED;
    } else {
        _state = _CONNECTING;
    }
}

static void _connect_wrp(e_timer tmr, void *ctx)
{
    _logging = TRUE;
    _connect_timer = NULL;
    _connect();
    _logging = FALSE;
}

static void _cancel_connect(void)
{
    if (_connect_timer) {
        e_timer_free(&_connect_timer);
    }
}

static void _schedule_connect(void)
{
    e_assert(_state != _CLOSED);

    if (_connect_timer != NULL) {
        return;
    }

    if (e_timer_create(&_connect_timer, _connect_timeout, E_TIMER_100_MSEC,
                _connect_wrp, NULL, 0)) {
        E_ERR("syslog connect timer failed");
        e_assert(_connect_timer == NULL);
    }
    e_assert(_connect_timer != NULL);
}

static void _schedule_write_pending(void);

static e_ret _write_pending(void)
{
    int cnt;
    u32 len;
    e_ret rc = ERROR;
    ascii str[E_SYSLOG_MAX_LEN + 1];
    u32 start;

    e_assert(_state == _CONNECTED);

    /* don't write more than 5 messages in a row... UDP may be
     * syncrhonous */
    cnt = 20;
    while(
            e_buff_read_remain(&_buff) &&
            e_client_is_connected(_client) &&
            cnt--) {
        s32 w;

        start = e_buff_get_pos(&_buff);

        if(e_buff_read_ascii(&_buff, str, E_SYSLOG_MAX_LEN + 1)) {
            goto exit;
        }

        len = (u32) e_strlen(str);
        w = e_client_write(_client, e_ptr_ascii_to_u8(str, len), len);
        if (w > 0) {
            /* UDP transport, w should be == len */
            if (((u32) w) != len) {
                E_ERR("syslog wrote a partial UDP packet");
                goto exit;
            }
        } else if (E_CLIENT_ERROR == w) {
            E_WARN("syslog write failed, closing connection");
            _state = _DISCONNECTING;
            e_client_close(_client);
            goto exit;
        } else { /* w == E_CLIENT_EAGAIN, wait for buffer */
            break;
        }
    }
    e_buff_compact(&_buff);
    rc = OK;

    if (e_buff_read_remain(&_buff)) {
        _schedule_write_pending();
    }

exit:
    if (rc) {
        /* rollback buffer to last sent message */
        e_buff_set_pos(&_buff, start);
    }
    return rc;
}

static void _write_pending_wrp(e_timer tmr, void *ctx) {
    _logging = TRUE;
    _write_pending_timer = NULL;
    if (_state == _CONNECTED) {
        (void) _write_pending();
    }
    _logging = FALSE;
}

static void _schedule_write_pending(void)
{
    _logging = TRUE;
    if (_write_pending_timer != NULL) {
        return;
    }

    if (e_timer_create(&_write_pending_timer, 3, E_TIMER_100_MSEC,
                _write_pending_wrp, NULL, 0)) {
        E_ERR("syslog write_pending timer failed");
        e_assert(_write_pending_timer == NULL);
    }
    e_assert(_write_pending_timer != NULL);
    _logging = FALSE;
}

static void _event_handler(void* arg, e_client_event event) {
    e_client client;
    client = (e_client) arg;

    _logging = TRUE;
    switch(event) {
        case E_CLIENT_CONNECT:
            _connect_timeout = 1;
            E_INFO("connected to syslog server");
            _state = _CONNECTED;
            if (e_buff_read_remain(&_buff)) {
                _schedule_write_pending();
            }
            break;
        case E_CLIENT_WRITE:
            (void) _write_pending();
            break;
        case E_CLIENT_READ:
            break;
        case E_CLIENT_DISCONNECT:
            E_INFO("disconnected from syslog server");
            if (_CLOSED == _state) {
                E_DBG("freeing client");
                e_client_free(&client);
            } else {
                if (_CONNECTING == _state) {
                    if (_connect_timeout < 512) {
                        _connect_timeout *= 2;
                    }
                    E_DBG("Syslog connect timeout: %lu", (unsigned long) _connect_timeout);
                }
                _state = _DISCONNECTED;
                _schedule_connect();
            }
            break;
        default:
            E_FATAL("Unknown syslog client event");
    }
    _logging = FALSE;
}

e_ret e_syslog_openlog(const char *ident, int facility,
        const ascii *host, const u16 port, const u16 local_port) {
    e_ret rc = ERROR;

    _logging = TRUE;
    E_STACK("e_syslog_openlog enter");

    if (_state != _CLOSED) {
        E_DBG("Syslog already opened");
        _logging = FALSE;
        return ERROR;
    }

    if (_ident) {
        e_mem_free(_ident);
        _ident = NULL;
    }
    _ident = e_strdup(ident);
    if(!_ident) {
        goto exit;
    }
    _facility = facility;

    _addr = e_client_addr_host(host, port);
    if (!_addr) {
        E_DBG("Error getting address");
        goto exit;
    }
    _local_port = local_port;

    E_DBG("syslog server: %s:%lu local port %lu",
            host,
            (unsigned long) port,
            (unsigned long) local_port);

    (void) e_mem_set(&_data, 0, sizeof(_data));
    (void) e_mem_set(&_buff, 0, sizeof(e_buff));
    e_buff_wrap(&_buff, _data, E_SYSLOG_BUFSIZ);
    if(e_client_create(&_client, _event_handler, _client)) {
        E_DBG("Error creating syslog client");
        goto exit;
    }

    _state = _DISCONNECTED;
    _schedule_connect();

    rc = OK;

exit:
    if(rc) {
        e_syslog_closelog();
    }

    E_STACK("e_syslog_openlog exit");

    _logging = FALSE;
    return rc;
}

void e_syslog_closelog(void) {
    _logging = TRUE;
    E_STACK("e_syslog_closelog enter");

    _state = _CLOSED;

    _cancel_connect();

    if(_client) {
        e_client_close(_client);
        /* client will be freed on DISCONNECT event */
        _client = NULL;
    }
    if (_ident) {
        e_mem_free(_ident);
        _ident = NULL;
    }
    if (_addr) {
        e_client_addr_free(&_addr);
        e_assert(_addr == NULL);
    }

    E_STACK("e_syslog_closelog exit");
    _logging = FALSE;
}

e_ret e_syslog(int prio, const char* str) {
    e_ret rc = ERROR;

    if (_logging) {
        return ERROR;
    }
    _logging = TRUE;

    if (_CLOSED == _state) {
        goto exit;
    }

    if (prio > _level) {
        rc = OK;
        goto exit;
    }

    #if 0
    ascii line[E_SYSLOG_MAX_LEN + 128];
    int wrote;
    u16 pri_part;
    time_t t;
    struct tm tm;
    if (e_strlen(str) > E_SYSLOG_MAX_LEN) {
        goto exit;
    }
    pri_part = (_facility * 8 + prio);
    if (pri_part > 999) {
        goto exit;
    }
    t = e_time_time(NULL);
    if(e_time_gmtime(t, &tm)) {
        goto exit;
    }

    wrote = e_sprintf(
            line, "<%d> %s %.2d %.2d:%.2d:%.2d %s %s",
            pri_part,
            e_time_month_name(tm.tm_mon + 1),
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            _hostname,
            str
            );
    if(e_buff_write_ascii(&_buff, line)) {
        E_WARN("out of buffer while writing to syslog");
        goto exit;
    }
    e_assert(wrote < (E_SYSLOG_MAX_LEN + 128));
    #else
    if(e_buff_write_ascii(&_buff, str)) {
        E_WARN("out of buffer while writing to syslog, resetting buffer");
        e_buff_reset(&_buff);
        goto exit;
    }
    #endif

    switch(_state) {
        case _CONNECTED:
            /* return as fast as possible...*/
            _schedule_write_pending();
            break;
        case _DISCONNECTED:
            _schedule_connect();
            break;
        case _CONNECTING:
            break;
        case _DISCONNECTING:
            break;
        default:
            e_assert(FALSE);
    }

    rc = OK;
exit:
    _logging = FALSE;
    return rc;
}

/* mark as unused in case logging is disabled */
UNUSED_FUNC static void _e_log_custom_handler(int level, ascii *str) {
    switch(level) {
        case E_LOG_FATAL:
            (void) e_syslog(E_SYSLOG_EMERG, str);
            break;
        case E_LOG_ERR:
            (void) e_syslog(E_SYSLOG_ERROR, str);
            break;
        case E_LOG_WARN:
            (void) e_syslog(E_SYSLOG_WARN, str);
            break;
        case E_LOG_INFO:
        case E_LOG_ALL:
            (void) e_syslog(E_SYSLOG_NOTICE, str);
            break;
        case E_LOG_PROG:
            (void) e_syslog(E_SYSLOG_INFO, str);
            break;
        case E_LOG_DEBUG:
        case E_LOG_TRACE:
        case E_LOG_MEM:
            (void) e_syslog(E_SYSLOG_DEBUG, str);
            break;
        default:
            (void) e_syslog(E_SYSLOG_INFO, str);
            break;
    }
}

void e_syslog_set_as_e_log(void) {
    E_DBG("setting syslog as e_log handler");
    E_LOG_ADD_FACILITY(CUSTOM);
    E_LOG_SET_CUSTOM_HANDLER(_e_log_custom_handler);
}
#endif /* HAS_SYSLOG */

#endif /* E_LIBS_NET_ENABLE */
