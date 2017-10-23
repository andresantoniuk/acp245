/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_gprs_h_
#define __e_gprs_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E_GPRS_MAX_PAR_LEN  64

typedef enum {
    E_GPRS_INIT_OK,
    E_GPRS_INIT_FAILED,
    E_GPRS_CONNECTED,
    E_GPRS_DISCONNECTED
} e_gprs_event;

typedef void (*e_gprs_ev_handler) (e_gprs_event, void*);

extern bool e_gprs_is_connected(void);

extern e_ret e_gprs_set_config(
        const ascii *apn, const ascii *user, const ascii *pass);

extern e_ret e_gprs_init_default(e_gprs_ev_handler handler, void *context);

extern e_ret e_gprs_init(
        const ascii *pincode,
        const ascii *apn, const ascii *user, const ascii *pass,
        e_gprs_ev_handler handler,
        void *context);


extern void e_gprs_stop(void);

#ifdef __cplusplus
}
#endif

#endif
