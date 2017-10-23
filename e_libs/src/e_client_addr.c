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

#include "e_port.h"
#include "e_mem.h"

#ifdef E_LIBS_NET_ENABLE

e_client_addr *e_client_addr_host(const ascii* host, u16 port) {
    e_client_addr *d;
    d = e_mem_malloc(sizeof(e_client_addr));
    if(!d) {
        return NULL;
    }
    d->port = port;
    d->is_str = TRUE;
    d->host.str = e_strdup(host);
    if (!d->host.str) {
        e_mem_free(d);
        return NULL;
    }
    return d;
}

e_client_addr *e_client_addr_ip(u32 ip, u16 port) {
    e_client_addr *d;
    d = e_mem_malloc(sizeof(e_client_addr));
    if(!d) {
        return NULL;
    }
    d->port = port;
    d->is_str = FALSE;
    d->host.ip = ip;
    return d;
}

e_ret e_client_addr_cpy(e_client_addr *dst, e_client_addr *src) {
    e_assert(dst != NULL);
    e_assert(src != NULL);

    dst->port = src->port;
    dst->is_str = src->is_str;
    if (src->is_str) {
        dst->host.str = e_strdup(src->host.str);
        if (!dst->host.str) {
            return ERROR;
        }
    } else {
        dst->host.ip = src->host.ip;
    }
    return OK;
}

e_client_addr *e_client_addr_dup(e_client_addr *addr) {
    e_client_addr *d;
    d = e_mem_malloc(sizeof(e_client_addr));
    if(!d) {
        return NULL;
    }
    d->is_str = addr->is_str;
    if (d->is_str) {
        d->host.str = e_strdup(addr->host.str);
        if(!d->host.str) {
            e_mem_free(d);
            return NULL;
        }
    } else {
        d->host.ip = addr->host.ip;
    }
    return d;
}

bool e_client_addr_equals(e_client_addr *a1, e_client_addr *a2) {
    if (a1->is_str != a2->is_str) {
        return FALSE;
    }
    if (a1->is_str) {
        return a1->port == a2->port && 
            (strcmp(a1->host.str, a2->host.str) == 0);
    } else {
        return a1->port == a2->port && 
            a1->host.ip == a2->host.ip;
    }
}

void e_client_addr_free(e_client_addr **addr) {
    if ((*addr)->is_str) {
        e_mem_free((*addr)->host.str);
        (*addr)->host.str = NULL;
    }
    e_mem_free((*addr));
    *addr = NULL;
}

#endif /* E_LIBS_NET_ENABLE */

