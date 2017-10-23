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

#include "adl_global.h"

#include "e_log.h"

#define E_WM_AT_START   "AT+ESTART"

static bool _start;
static bool _cfg_done;
static adl_InitType_e init_type;
static e_startup_entry_point cfg_point = NULL;
static e_startup_entry_point entry_point;

static void at_start_handler(adl_atCmdPreParser_t *params)
{
    u8 rc;

    E_STACK("at_start_handler enter");

    rc = 0;
    switch(params->Type) {
    case ADL_CMD_TYPE_PARA:
        /* min params=1, max params = 1*/
        if (params->NbPara == 1) {
            ascii *p;

            p = ADL_GET_PARAM(params, 0);
            if (p != NULL  && wm_strlen(p) == 1) {
                int i = wm_atoi(p);
                if (i >= 0 && i <= 1) {
                    _start = i;
                } else {
                    rc = 3;
                }
            } else {
                rc = 4;
            }
        } else {
            rc = 5;
        }
        break;
    case ADL_CMD_TYPE_READ:
        {
        ascii resp[16];
        /* 15 chars long max + \0 */
        /*@-bufferoverflowhigh@*/   
        wm_sprintf(resp, "\r\n+ESTART: %d\r\n", _cfg_done);
        /*@+bufferoverflowhigh@*/
        (void) adl_atSendResponse (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ), resp );
        }
        goto exit;
    default:
        rc = 6;
    }

    switch(_start){
    case TRUE:
        E_DBG("at_start_handler MANUAL START");
        entry_point();
        break;
    case FALSE:
        if(_cfg_done == FALSE){
            _cfg_done = TRUE;
            E_DBG("at_start_handler RUNNING CONFIGURATION");
            cfg_point();
        }else{
            E_DBG("at_start_handler CONFIGURATION ALREADY DONE");
            rc = 7;
        }
        break;
    default:
        rc = 8;
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
    E_STACK("at_start_handler exit");
    return;
}

int e_startup_wm_register_at_start(void)
{
    int rc = -1;
    s16 res;

    E_STACK("e_startup_wm_register_at_start enter");

    res = adl_atCmdSubscribe(
            E_WM_AT_START,
            at_start_handler,
            (u16) (ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA | 0x11));
    if (res != OK) {
        goto exit;
    }
    rc = 0;

    E_DBG("e_startup_wm_register_at_start OK");

exit:

    E_STACK("e_startup_wm_register_at_start exit");

    return rc;
}

static bool error_handler(u16 type, ascii * info)
{
    /* use ADL_AT_RSP to avoid buffering */
	(void) adl_atSendResponse(ADL_AT_RSP, "\r\nFATAL ERROR\r\n");
    {
        static ascii buf[20];
        /*@-bufferoverflowhigh@*/
        wm_sprintf(buf, "CODE %lu.\r\n", (unsigned long) type);
        /*@+bufferoverflowhigh@*/
    	(void) adl_atSendResponse(ADL_AT_RSP, buf);
    	(void) adl_atSendResponse(ADL_AT_RSP, info);
    	(void) adl_atSendResponse(ADL_AT_RSP, "\r\n");
    }

	return TRUE;
}

static void log_init_cause(adl_InitType_e InitType)
{
	switch( InitType ) {
    case ADL_INIT_REBOOT_FROM_EXCEPTION:
        E_INFO("********EXCEPTION OCURRED - DEVICE REBOOTED ********");
        break;
    case ADL_INIT_POWER_ON:
        E_INFO("Device powering on.");
        break;
    case ADL_INIT_DOWNLOAD_SUCCESS:
        E_INFO("Update succeeded.");
        break;
    case ADL_INIT_DOWNLOAD_ERROR:
        E_INFO("Update failed.");
        break;
    case ADL_INIT_RTC:
        E_INFO("RTC Alarm.");
        break;
    default:
        E_ERR("Unknown Init Type.");
        break;
	}
}

static void _auto_startup_handler(u8 ID, void *context) {
    E_STACK("_startup_handler enter");

    if(_start == TRUE){
        if(cfg_point){
            E_DBG("_startup_handler AUTO CONFIGURATION");
            cfg_point();
        }
        E_DBG("_startup_handler AUTO START");
        entry_point();
    }else{
        E_DBG("_startup_handler STAY IN CONFIGURE MODE");
    }

    E_STACK("_startup_handler exit");
}

static void _print_diag(u8 ID, void *context) {
    adl_memInfo_t mem_info;

    (void) e_mem_set(&mem_info, 0, sizeof(mem_info));
    if(adl_memGetInfo(&mem_info) != OK) {
        E_ERR("Error getting memory info.");
    } else {
        E_DBG("Memory: TS=%lu SS=%lu HS=%lu GS=%lu",
                (unsigned long) mem_info.TotalSize,
                (unsigned long) mem_info.StackSize,
                (unsigned long) mem_info.HeapSize,
                (unsigned long) mem_info.GlobalSize);
    }
}

adl_InitType_e e_startup_get_init_type(void) {
    return init_type;
}

void e_startup_main ( adl_InitType_e it, e_startup_entry_point ep, u32 delay) {
    init_type = it;
    entry_point = ep;

    (void) adl_atSendResponse(ADL_AT_UNS, "Starting...\r\n");
	if (adl_errSubscribe( error_handler )  < 0) {
        adl_errHalt(0, "Error subscribe failed");
        return;
	}

    log_init_cause(init_type);

    if(E_LOG_IS(DEBUG)) {
        _print_diag(0, NULL);
        /*
        if(adl_tmrSubscribe(TRUE, 10*10, ADL_TMR_TYPE_100MS,
                    _print_diag) < 0) {
            adl_errHalt(0, "diagnostic handler timer failed.");
        }
        */
    }

    if (delay) {
        if(adl_tmrSubscribe(FALSE, delay*10, ADL_TMR_TYPE_100MS,
                    _auto_startup_handler) < 0) {
            adl_errHalt(0, "startup handler timer failed.");
        }
    }

    E_STACK("e_startup_main exit.");
}

void e_startup_cfg ( adl_InitType_e it, e_startup_entry_point cfg, e_startup_entry_point ep, u32 delay) {
    init_type = it;
    cfg_point = cfg;
    entry_point = ep;

    (void) adl_atSendResponse(ADL_AT_UNS, "Starting...\r\n");
	if (adl_errSubscribe( error_handler )  < 0) {
        adl_errHalt(0, "Error subscribe failed");
        return;
	}

    log_init_cause(init_type);

    if(E_LOG_IS(DEBUG)) {
        _print_diag(0, NULL);
        /*
        if(adl_tmrSubscribe(TRUE, 10*10, ADL_TMR_TYPE_100MS,
                    _print_diag) < 0) {
            adl_errHalt(0, "diagnostic handler timer failed.");
        }
        */
    }

    /* by default: AUTO START */
    _start = TRUE;
    _cfg_done = FALSE;
    /* register AT+ECFG=<TRUE/FALSE> command */
    if(e_startup_wm_register_at_start()  < 0) {
        adl_errHalt(0, "Failed registering for ESTART AT command");
    }

    if (delay) {
        if(adl_tmrSubscribe(FALSE, delay*10, ADL_TMR_TYPE_100MS,
                    _auto_startup_handler) < 0) {
            adl_errHalt(0, "auto startup handler timer failed.");
        }
    }

    E_STACK("e_startup_main exit.");
}
