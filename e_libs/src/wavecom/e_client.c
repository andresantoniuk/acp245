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

#include "adl_global.h"
#include "wip.h"

#include "e_log.h"
#include "e_port.h"
#include "e_mem.h"
#include "e_buff.h"
#include "e_errors.h"
#include "e_errors_wip.h"

typedef enum _client_state {
    _SK_CONNECTING,
    _SK_CONNECTED,
    _SK_CLOSED
} _client_state;

typedef struct _addr {
    ascii* host;
    u16 port;
} _addr;

struct _e_client
{
    e_client_type type;

	_addr peer_addr;
    wip_channel_t sk;

    _client_state state;
    u16 local_port;

    e_client_event_handler event_handler;
    void* event_handler_arg;
};

static void _switch_state(e_client cli, int next_state) {
    E_DBG("e_client state from %d to %d", cli->state, next_state);
    cli->state = next_state;
}

void e_client_close(e_client cli) {
    int rc;
    E_STACK("e_client_close enter");

    _switch_state(cli, _SK_CLOSED);

    if (cli->sk) {
        if((rc = wip_close(cli->sk))) {
            ERR_LOG(wip_close, rc);
        }
	    cli->sk = NULL;
        E_DBG("Notifying client handler of disconnect");
        cli->event_handler(cli->event_handler_arg, E_CLIENT_DISCONNECT);
    }

    E_STACK("e_client_close exit");
}

void e_client_free(/*@only@*/ e_client* cli) {
    if (cli == NULL || *cli == NULL) {
        return;
    }

    e_client_close(*cli);
    e_mem_free((*cli)->peer_addr.host);
    (*cli)->peer_addr.host = NULL;

    e_mem_free(*cli);
    *cli = NULL;
}

static void _handle_wip_event( wip_event_t *ev, void *ctx) 
{
	e_client cli;

	e_assert( ev != NULL );
    e_assert( ctx != NULL );

	E_STACK("handle_wip_event enter");

	cli = (e_client) ctx;

	switch( ev->kind) {
		case WIP_CEV_OPEN:
            _switch_state(cli, _SK_CONNECTED);

            E_TRACE("_handle_wip_event handler for connect");
            cli->event_handler(cli->event_handler_arg, E_CLIENT_CONNECT);
			break;
        case WIP_CEV_PEER_CLOSE:
			E_DBG("socket closed by peer");
            e_client_close(cli);
			break;
		case WIP_CEV_ERROR:
			E_DBG("socket error, closing");
            e_client_close(cli);
			break;
		case WIP_CEV_READ:
			E_TRACE("read available on socket.");
            cli->event_handler(cli->event_handler_arg, E_CLIENT_READ);
			break;
		case WIP_CEV_WRITE:
			E_TRACE("write available on socket.");
            cli->event_handler(cli->event_handler_arg, E_CLIENT_WRITE);
			break;
		default:
			E_FATAL("Invalid WIP event: 0x%x", ev->kind);
            e_client_close(cli);
	}

	E_STACK("handle_wip_event exit");

}

s32 e_client_write(e_client cli, u8* buff, u32 size) {
    int written;

    e_assert(cli != NULL);
    e_assert(buff != NULL);
    e_assert(cli->peer_addr.port != 0);

    E_STACK("e_client_write enter");

    if (!e_client_is_connected(cli)) {
        return E_CLIENT_ERROR;
    }

    if (0 == size) {
        return OK;
    }

    written = wip_write(cli->sk, buff, size);

    E_TRACE("wip_write wrote=%d of %lu",
            written,
            (long unsigned) size);

    if (written < 0) {
        ERR_LOG(wip_write, written);
        written = E_CLIENT_ERROR;
    } else if (written == 0) {
        written = E_CLIENT_EAGAIN;
    }

    E_TRACE("...wrote %d", written);

    E_STACK("e_client_write exit");

    return (s32) written;
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
    int readed;

    e_assert(cli != NULL);
    e_assert(buff != NULL);

    E_STACK("e_client_read enter");

    if (!e_client_is_connected(cli)) {
        return E_CLIENT_ERROR;
    }

    readed = wip_read(cli->sk, buff, size);
    if (readed < 0) {
        ERR_LOG(wip_read, readed);
        readed = E_CLIENT_ERROR;
    } else if (readed == 0) {
        readed = E_CLIENT_EAGAIN;
    }

    E_TRACE("...readed %d", readed);

    E_STACK("e_client_read exit");

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

bool e_client_is_connected(e_client cli) {
    return e_client_is_open(cli) && (wip_getState(cli->sk) == WIP_CSTATE_READY);
}

bool e_client_is_open(e_client cli) {
    e_assert(cli != NULL);

    return cli->state != _SK_CLOSED;
}

e_ret e_client_create(e_client* cli_ret,
        e_client_event_handler event_handler, void* event_handler_arg) {
    e_ret rc = E_CLIENT_ERROR;
    e_client cli = NULL;

    e_assert( cli_ret != NULL );
    e_assert( *cli_ret == NULL );
    e_assert( event_handler != NULL || event_handler_arg == NULL );

    E_STACK("e_client_create enter");

    cli = (e_client) e_mem_malloc(sizeof(struct _e_client));
    if (!cli) {
        goto exit;
    }
    (void) e_mem_set(cli, 0, sizeof(struct _e_client));

    cli->state = _SK_CLOSED;
    cli->event_handler = event_handler;
    cli->event_handler_arg = event_handler_arg;

    rc = OK;

exit:
    *cli_ret = cli;
    if (rc) {
        e_client_free(cli_ret);
    }

    E_STACK("e_client_create exit");

    e_assert(rc != OK || *cli_ret != NULL);
    return rc;
}

e_ret e_client_open(e_client cli,
        e_client_type type,
        e_client_addr *addr, u16 local_port) {

    e_ret rc = E_CLIENT_ERROR;

    e_assert( cli != NULL );
    e_assert( cli->sk == NULL );
	e_assert( addr != NULL );
    e_assert( !e_client_is_open(cli) );

    E_STACK("e_client_open enter");

    cli->type = type;
    cli->local_port = local_port;

    cli->peer_addr.port = addr->port;
    cli->peer_addr.host = e_client_addr_get_host(addr);
    if (!cli->peer_addr.host) {
        goto exit;
    }

    _switch_state(cli, _SK_CONNECTING);
    if (cli->type == E_CLIENT_TCP) {
        E_DBG("Opening TCP connection to %s:%lu",
                cli->peer_addr.host,
                (unsigned long) cli->peer_addr.port);
	    cli->sk = wip_TCPClientCreateOpts(
                cli->peer_addr.host,
                cli->peer_addr.port,
                _handle_wip_event,
                cli,
                WIP_COPT_PORT, local_port,
                WIP_COPT_NODELAY, TRUE,
                WIP_COPT_RCV_BUFSIZE, 2048,
                WIP_COPT_SND_BUFSIZE, 2048,
                WIP_COPT_KEEPALIVE, 0,
                WIP_COPT_END);
    } else if (type == E_CLIENT_UDP) {
        E_DBG("Opening UDP connection to %s:%lu",
                cli->peer_addr.host,
                (unsigned long) cli->peer_addr.port);
       cli->sk = wip_UDPCreateOpts(
                _handle_wip_event,
                cli,
                WIP_COPT_PEER_STRADDR, cli->peer_addr.host,
                WIP_COPT_PEER_PORT, cli->peer_addr.port,
                WIP_COPT_PORT, local_port,
                WIP_COPT_END);
    }

    if (!cli->sk) {
        E_ERR("Error creating WIP channel");
        goto exit;
    }

    rc = OK;

exit:
    if (rc) {
        e_client_close(cli);
    }

    E_STACK("e_client_open exit");

    e_assert((rc != OK) || cli->sk);
    return rc;
}

#define _IP_BUFF_LEN ((3*4 + 3 + 1))
ascii* e_client_addr_get_host(e_client_addr *addr) {
    if (addr->is_str) {
        return e_strdup(addr->host.str);
    } else {
        ascii *host;
        host = e_mem_malloc(_IP_BUFF_LEN * sizeof(ascii));
        if(!host) {
            return NULL;
        }
        if(!wip_inet_ntoa(addr->host.ip, host, _IP_BUFF_LEN)) {
            return NULL;
        } else {
            return host;
        }
    }
}
