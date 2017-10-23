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

#include "e_gprs.h"

#include "e_log.h"

static bool connected = FALSE;

e_ret e_gprs_init(
        const ascii * pincode,
        const ascii * apn,
        const ascii * user,
        const ascii * pass,
        e_gprs_ev_handler handler,
        void *context) {
	E_UNUSED(pincode);
	E_UNUSED(apn);
	E_UNUSED(user);
	E_UNUSED(pass);

	E_DBG("Calling dummy e_gprs_init function");
    handler(E_GPRS_INIT_OK, context);
    connected = TRUE;
    handler(E_GPRS_CONNECTED, context);
    return OK;
}

bool e_gprs_is_connected(void) {
    E_DBG("Calling dummy e_gprs_is_connected function");
    return connected;
}

e_ret e_gprs_set_config(const ascii *apn, const ascii *user, const ascii *pass) {
	E_UNUSED(apn);
	E_UNUSED(user);
	E_UNUSED(pass);

    E_DBG("Calling dummy e_gprs_set_config function");
    return OK;
}

void e_gprs_stop(void) {
    connected = FALSE;
}
