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

#include "e_log_wm.h"
#include "e_log.h"

#include "adl_global.h"

static void at_set_level_handler(adl_atCmdPreParser_t *params)
{
    u8 rc;

    E_STACK("at_set_level_handler enter");

    rc = 0;
    switch(params->Type) {
    case ADL_CMD_TYPE_PARA:
        /* min params=1, max params = 1*/
        if (params->NbPara == 1) {
            ascii *p;

            p = ADL_GET_PARAM(params, 0);
            if (p != NULL  && wm_strlen(p) == 1) {
                int level = wm_atoi(p);
                if (level >= 0 && level <= 9) {
                    e_log_set_level(level);
                } else {
                    rc = 3;
                }
            } else {
                rc = 3;
            }
        } else {
            rc = 5;
        }
        break;
    case ADL_CMD_TYPE_READ:
        {
        ascii resp[14];
        e_assert(e_log_get_level() <= 99 && e_log_get_level() >= 0);
        /* 13 chars long max + \0 */
        /*@-bufferoverflowhigh@*/
        wm_sprintf(resp, "\r\n+ELOG: %d\r\n", e_log_get_level());
        /*@+bufferoverflowhigh@*/
        (void) adl_atSendResponse (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ), resp );
        }
        goto exit;
    default:
        rc = 6;
    }

    if (rc == 0) {
        (void) adl_atSendStdResponse (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ),
                ADL_STR_OK );
    } else {
        (void) adl_atSendStdResponseExt (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ),
                ADL_STR_CME_ERROR, rc );
    }

exit:
    E_STACK("at_set_level_handler exit");
    return;
}

int e_log_wm_register_at_set_level(void)
{
    int rc = -1;
    s16 res;

    E_STACK("e_log_register_at_set_level enter");

    res = adl_atCmdSubscribe(
            E_LOG_WM_AT_SET_LEVEL,
            at_set_level_handler,
            (u16) (ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA | 0x11));
    if (res != OK) {
        goto exit;
    }
    rc = 0;

    E_DBG("e_log_register_at_set_level OK");

exit:

    E_STACK("e_log_register_at_set_level exit");

    return rc;
}
