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

#include "e_client.h"

#ifdef E_LIBS_NET_ENABLE

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WIN32
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

typedef u8 u_char;
#include <event.h>
#include <evutil.h>

#include "e_log.h"
#include "e_port.h"
#include "e_buff.h"
#include "e_mem.h"

#define _MAX_EVENTS 5

#define _SK_DISCONNECTING    1
#define _SK_CONNECTING       2
#define _SK_CONNECTED        3
#define _SK_CLOSED           4
#define _SK_FREEING          5

#define _SIGINT             SIGUSR1

struct _e_client
{
    e_client_type type;

	struct sockaddr_in peer_addr;
	int sk;

    int state;
    short events;

    u16 local_port;

    e_client_event_handler event_handler;
    void* event_handler_arg;

    struct event sock_event;
    struct event tmr_event;
};

static int
_get_addr(const char *addr, u32 *ipaddr)
{
	struct hostent *h;

	assert( addr != NULL );

	h = gethostbyname(addr);
	if( !h ) {
		return -1;
	}

	(void) e_mem_cpy(ipaddr, h->h_addr_list[0], 4);
	return 0;
}

static void _switch_state(e_client cli, int next_state) {
    E_DBG("e_client state from %d to %d", cli->state, next_state);
    cli->state = next_state;
}

static void _close_socket(int fd, short ev, void *arg) {
    e_client cli;

    cli = (e_client) arg;

    e_assert(_SK_DISCONNECTING == cli->state || _SK_FREEING == cli->state);

    switch(cli->state) {
        case _SK_DISCONNECTING:
            _switch_state(cli, _SK_CLOSED);
            E_TRACE("e_client event handler for disconnect");
            cli->event_handler(cli->event_handler_arg, E_CLIENT_DISCONNECT);
            break;
        case _SK_FREEING:
            e_mem_free(cli);
            break;
        default:
            e_assert(FALSE);
    }
}

static void _schedule_close(e_client cli) {
    /* emulate async disconnection */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    event_once(-1, EV_TIMEOUT, _close_socket, cli, &tv);
}

void e_client_close(e_client cli) {

    E_TRACE("e_client_close enter");
    if (!e_client_is_open(cli) || _SK_DISCONNECTING == cli->state) {
        return;
    }

    _switch_state(cli, _SK_DISCONNECTING);

    if(event_del(&cli->sock_event)) {
        E_DBG("event_del failed for sock event");
    }
    if(event_del(&cli->tmr_event)) {
        E_DBG("event_del failed for timer event");
    }
    if (cli->sk != 0) {
        (void) close(cli->sk);
    }
    _schedule_close(cli);

    E_TRACE("e_client_close exit");
}

void e_client_free(e_client* cli) {
    if (cli == NULL || *cli == NULL) {
        return;
    }

    if (e_client_is_open(*cli)) {
        e_client_close(*cli);
        _switch_state(*cli, _SK_FREEING);
    } else {
        e_mem_free(*cli);
    }

    *cli = NULL;
}

static void _handle_event(int fd, short ev, void *arg);

static void _add_sock_event(e_client cli)
{
    event_set(&cli->sock_event, cli->sk, cli->events, _handle_event, cli);
    if(event_add(&cli->sock_event, NULL)) {
        E_ERR("Failed registering for socket events, closing connecting: %s", strerror(errno));
        e_client_close(cli);
    }
}

static void _try_connect(e_client cli) {
    if (connect(cli->sk, (struct sockaddr *) &(cli->peer_addr), (int)sizeof(cli->peer_addr))) {
#ifdef WIN32
        int tmp_error = WSAGetLastError();
        if (tmp_error != WSAEWOULDBLOCK &&
                tmp_error != WSAEINPROGRESS) {
#else
        if (EINPROGRESS != errno) {
#endif
            E_DBG("e_client connect delayed");
            cli->events |= EV_WRITE;
            _add_sock_event(cli);
        } else {
            E_ERR("Can't connect: %s\n", strerror(errno));
            e_client_close(cli);
        }
    } else {
        E_DBG("e_client connected");
        _switch_state(cli, _SK_CONNECTED);
        cli->events |= EV_READ;
        _add_sock_event(cli);
        cli->event_handler(cli->event_handler_arg, E_CLIENT_CONNECT);
    }
}

static void _handle_event(int fd, short ev, void *arg) {
    e_client cli;

    cli = (e_client) arg;
    e_assert(_SK_CONNECTING == cli->state || _SK_CONNECTED == cli->state);

    E_DBG("e_client I/O event");

    switch(cli->state) {
        case _SK_CONNECTING:
            _try_connect(cli);
            break;
        case _SK_CONNECTED:
            if(EV_READ & ev) {
                cli->events |= EV_READ;
                _add_sock_event(cli);
                cli->event_handler(cli->event_handler_arg, E_CLIENT_READ);
            } else if (EV_WRITE & ev) {
                cli->events |= EV_READ;
                cli->events &= ~EV_WRITE;
                _add_sock_event(cli);
                cli->event_handler(cli->event_handler_arg, E_CLIENT_WRITE);
            } else {
                e_assert(FALSE);
            }
            break;
        default:
            e_assert(FALSE);
    }

    return;
}

static void _connect(int fd, short ev, void *arg) {
    e_ret rc = ERROR;
    e_client cli = (e_client) arg;

    E_DBG("e_client connecting");

    /* bind to local port if needed */
    if (cli->local_port > 0) {
        struct sockaddr_in bind_addr;
        (void) e_mem_set(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_addr.s_addr = INADDR_ANY;
        bind_addr.sin_port = htons(cli->local_port);
        if (bind(cli->sk, (struct sockaddr*) &bind_addr, sizeof(bind_addr))) {
            E_ERR("Can't bind to local port: %s\n", strerror(errno));
            goto exit;
        }
        E_DBG("Socket bound to local port: %d", ntohs(bind_addr.sin_port));
    }

    /* set non blocking */
    if (evutil_make_socket_nonblocking(cli->sk)) {
        E_FATAL("Can't set to nonblocking: %s\n", strerror(errno));
        goto exit;
    }

    _try_connect(cli);

    rc = OK;

exit:
    if (rc) {
        e_client_close(cli);
    }
}

static e_ret _get_ip_port(e_client_addr *addr, u32 *ip, u16 *port) {
    e_assert (ip != NULL);
    e_assert (port != NULL);
    e_assert (addr == NULL || ((addr->is_str && addr->host.str != NULL) || (addr->host.ip > 0)));

    if (addr != NULL) {
        *port = addr->port;
        if (addr->is_str) {
            if(_get_addr(addr->host.str, ip)) {
    		    E_ERR("Failed to get address of %s\n", addr->host.str);
                return E_CLIENT_BAD_ADDR;
            }
        } else {
            *ip = addr->host.ip;
        }
    } else {
        *ip = 0;
        *port = 0;
    }

    return OK;
}

static s32 _write_to(e_client cli, u8* buff, u32 size,  struct sockaddr *addr, socklen_t len) {
    ssize_t written;

    e_assert(cli != NULL);
    e_assert(buff != NULL);

    if (!e_client_is_connected(cli)) {
        return ERROR;
    }

    if (E_CLIENT_TCP == cli->type) {
#ifdef WIN32
        written = send(cli->sk, (char*)buff, size, 0);
#else
        written = send(cli->sk, buff, size, 0);
#endif
    } else if (NULL == addr) {
#ifdef WIN32
        written = sendto(cli->sk, (char*) buff, size, 0, (struct sockaddr*) &cli->peer_addr, sizeof(cli->peer_addr));
#else
        written = sendto(cli->sk, buff, size, 0, (struct sockaddr*) &cli->peer_addr, sizeof(cli->peer_addr));
#endif
    } else {
#ifdef WIN32
        written = sendto(cli->sk, (char*) buff, size, 0, addr, len);
#else
        written = sendto(cli->sk, buff, size, 0, addr, len);
#endif
    }

    if (written < 0) {
#ifdef WIN32
        int tmp_error = WSAGetLastError();
        if (tmp_error != WSAEWOULDBLOCK && tmp_error != WSAEINPROGRESS) {
#else
        if ((EAGAIN != errno) && (EWOULDBLOCK != errno) && (EINPROGRESS != errno)) {
#endif
            return E_CLIENT_ERROR;
        } else {
            cli->events |= EV_WRITE;
            _add_sock_event(cli);
            return E_CLIENT_EAGAIN;
        }
    }
    return (s32) written;
}

s32 e_client_write(e_client cli, u8* buff, u32 size) {
    e_assert(cli != NULL);
    e_assert(buff != NULL);
    e_assert(cli->type == E_CLIENT_TCP || cli->peer_addr.sin_port != 0);

    return _write_to(cli, buff, size, (struct sockaddr *) &cli->peer_addr, sizeof(cli->peer_addr));
}

s32 e_client_write_buff(e_client cli, e_buff *buff) {
    s32 w;
    u32 sz;

    e_assert(cli != NULL);
    e_assert(buff != NULL);

    sz = e_buff_read_remain(buff);
    /* internal access to buff to avoid additional copy... */
    w = e_client_write(cli, buff->data + e_buff_get_pos(buff), sz);
    if (w > 0) {
        if (((u32)w) == sz) {
            e_buff_reset(buff);
        } else {
            e_ret rc;
            /* skip written data */
            rc = e_buff_skip(buff, w);
            e_assert(OK == rc);
        }
    }
    return w;
}

s32 e_client_read(e_client cli, u8* buff, u32 size) {
    ssize_t readed;

    E_TRACE("e_client_read enter");

    e_assert(cli != NULL);
    e_assert(buff != NULL);

    if (!e_client_is_connected(cli)) {
        return E_CLIENT_ERROR;
    }

#ifdef WIN32
    readed = recv(cli->sk, (char*) buff, size, 0);
#else
    readed = recv(cli->sk, buff, size, MSG_DONTWAIT);
#endif
    if (readed == 0) {
        E_DBG("Peer closed the connection");
        e_client_close(cli);
    } else if (readed < 0) {
#ifdef WIN32
        int tmp_error = WSAGetLastError();
        if (tmp_error != WSAEWOULDBLOCK && tmp_error != WSAEINPROGRESS) {
#else
        if ((EAGAIN != errno) && (EWOULDBLOCK != errno) && (EINPROGRESS != errno)) {
#endif
            return E_CLIENT_ERROR;
        } else {
            return E_CLIENT_EAGAIN;
        }
    }

    E_TRACE("e_client_read exit");

    return (s32) readed;
}

s32 e_client_read_buff(e_client cli, e_buff *buff) {
    s32 r;
    u32 lim;

    e_assert(cli != NULL);
    e_assert(buff != NULL);

    if (e_buff_get_pos(buff) > 0) {
        /* make room for data */
        e_buff_compact(buff);
    }

    lim = e_buff_get_lim(buff);
    r = e_client_read(cli, buff->data + lim, e_buff_write_remain(buff));
    if (r > 0) {
        e_buff_set_lim(buff, lim + r);
    }
    return r;
}

bool e_client_is_open(e_client cli) {
    return _SK_CLOSED != cli->state && _SK_FREEING != cli->state;
}

bool e_client_is_connected(e_client cli) {
    return _SK_CONNECTED == cli->state;
}

e_ret e_client_create(e_client* cli_ret, e_client_event_handler event_handler, void* event_handler_arg) {
    e_ret rc;
    e_client cli;

    assert( cli_ret != NULL );
    assert( *cli_ret == NULL );

    E_TRACE("e_client_create enter");

    cli = (e_client) e_mem_malloc(sizeof(struct _e_client));
    if (!cli) {
        E_LOG_NOMEM;
        rc = E_CLIENT_ERROR;
        goto exit;
    }

    (void) e_mem_set(cli, 0, sizeof(struct _e_client));
    cli->state = _SK_CLOSED;
    cli->event_handler = event_handler;
    cli->event_handler_arg = event_handler_arg;
    rc = OK;

exit:
    if (rc) {
        e_mem_free(cli);
        *cli_ret = NULL;
    } else {
        *cli_ret = cli;
    }

    E_TRACE("e_client_create exit");

    e_assert(rc != OK || *cli_ret != NULL);
    return rc;
}

e_ret e_client_open(e_client cli, e_client_type type, e_client_addr *peer, u16 local_port) {
    e_ret rc;
    u32 peer_ip;
    u16 peer_port;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    assert( cli != NULL );
	assert( type == E_CLIENT_UDP || peer != NULL );
    e_assert( !e_client_is_open(cli) );

    E_TRACE("e_client_open enter");

    if ( _get_ip_port(peer, &peer_ip, &peer_port)) {
        rc = E_CLIENT_BAD_ADDR;
        goto exit;
    }

    cli->type = type;
    cli->local_port = local_port;

    if (E_CLIENT_TCP == type) {
    	cli->sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (E_CLIENT_UDP == type) {
    	cli->sk = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

	if( -1 == cli->sk ) {
		E_ERR("Can't create client socket\n");
		rc = E_CLIENT_ERROR;
        goto exit;
	}

    (void) e_mem_set(&cli->peer_addr, 0, sizeof(cli->peer_addr));
    if (peer_ip != 0) {
	    cli->peer_addr.sin_family = AF_INET;
        cli->peer_addr.sin_addr.s_addr = peer_ip;
    	cli->peer_addr.sin_port = htons(peer_port);
    }

    _switch_state(cli, _SK_CONNECTING);

    evtimer_set(&cli->tmr_event, _connect, cli);
    evtimer_add(&cli->tmr_event, &tv);

    rc = OK;

exit:
    if (rc) {
        if (cli->sk) {
            close(cli->sk);
        }
    }

    E_TRACE("e_client_open exit");
    return rc;
}

ascii* e_client_addr_get_host(e_client_addr *addr) {
    if (addr->is_str) {
        return e_strdup(addr->host.str);
    } else {
        struct in_addr i_addr;
        ascii *host;

        e_mem_set(&i_addr, 0, sizeof(struct in_addr));
        i_addr.s_addr = addr->host.ip;

        host = inet_ntoa(i_addr);
        return e_strdup(host);
    }
}

#endif /* E_LIBS_NET_ENABLE */

