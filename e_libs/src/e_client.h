/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_client_h_
#define __e_client_h_

#include "e_port.h"
#include "e_splint_macros.h"

#include "e_buff.h"

#ifdef E_LIBS_NET_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _e_client* e_client;

/* TODO: use a addr struct instead of different functions for string/int 
 * hosts */
/* TODO: receive an e_buff instead of a u8* */

typedef enum e_client_event {
    E_CLIENT_CONNECT,
    E_CLIENT_WRITE,
    E_CLIENT_READ,
    E_CLIENT_DISCONNECT
} e_client_event;

typedef enum e_client_type {
    E_CLIENT_TCP,
    E_CLIENT_UDP
} e_client_type;

typedef struct e_client_addr {
    u16 port;
    bool is_str;
    union {
        ascii *str;
        u32 ip;
    } host;
} e_client_addr;

#define E_CLIENT_ERROR         -1
#define E_CLIENT_EAGAIN        -2
#define E_CLIENT_BAD_ADDR      -3

/**
 * Event handler callback prototype.
 * @param arg the argument given when calling e_client_create.
 * @param event the event.
 */
typedef void (*e_client_event_handler) (void* arg, e_client_event event);

/**
 * Creates a new network client.
 * @param cli_ret a pointer to the client that will be created.
 * @param event_handler a handler of client events.
 * @param event_handler_arg an argument to pass to the event handler whenever
 * there's an event.
 * @return OK if the client was created successfuly.
 *         E_CLIENT_ERROR if there was an error creating the client.
 */
extern e_ret e_client_create(SPL_CONS_P e_client* cli_ret,
        e_client_event_handler event_handler, void* event_handler_arg);

/**
 * Open a connection to the given host.
 *
 * This function will begin to perform a connection to the host. Once the
 * connection is established, the event handler will receive a E_CLIENT_CONNECT
 * event.
 *
 * If the connection fails, it will receive a E_CLIENT_DISCONNECT event.
 *
 * @param cli_the client.
 * @param type the type of connection.
 * @param addr the address of the host.
 * @param local_port the port from where to start the connection.
 * @return OK if the connection is being performed.
 *         E_CLIENT_ERROR if there's an error starting the connection.
 *         E_CLIENT_BAD_ADDR if there was an error processing the address
 *         of the host.
 */
extern e_ret e_client_open(e_client cli,
        e_client_type type, e_client_addr *addr, u16 local_port);

extern s32 e_client_write(e_client cli, u8* buff, u32 size);

extern s32 e_client_write_buff(e_client cli, e_buff *buff);

extern s32 e_client_read(e_client cli, u8* buff, u32 size);

extern s32 e_client_read_buff(e_client cli, e_buff *buff);

extern void e_client_close(e_client cli);

extern bool e_client_is_open(e_client cli);

extern bool e_client_is_connected(e_client cli);

extern void e_client_free(SPL_DEST_P e_client* cli_ret);

extern e_client_addr *e_client_addr_host(const ascii* host, u16 port);

extern e_client_addr *e_client_addr_ip(u32 host, u16 port);

extern e_client_addr *e_client_addr_dup(e_client_addr *addr);

extern e_ret e_client_addr_cpy(e_client_addr *dst, e_client_addr *src);

extern ascii* e_client_addr_get_host(e_client_addr *addr);

extern bool e_client_addr_equals(e_client_addr *a1, e_client_addr *a2);

extern void e_client_addr_free(e_client_addr **addr);

#ifdef __cplusplus
}
#endif

#endif /* E_LIBS_NET_ENABLE */

#endif
