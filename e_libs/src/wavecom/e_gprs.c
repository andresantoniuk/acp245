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
#include "e_gprs_wm.h"

#include "adl_global.h"
#include "wip.h"

#include "e_port.h"
#include "e_log.h"
#include "e_errors.h"

/*AT+EGPRS="internet.ctimovil.com.uy","ctigprs","ctigprs999"*/

#define CREG_POLLING_PERIOD (5*10) /* 5 secs */
#define RECONNECT_PERIOD    (5*10) /* 5 secs */

typedef enum _gprs_state {
    _IDLE,
    _DISCONNECTED,
    _CONNECTED,
    _CONFIGURING,
    _STOPPING
} _gprs_state;

static e_gprs_ev_handler _gprs_ev_handler;
static void *_gprs_ev_handler_context;

static ascii gprs_apn[E_GPRS_MAX_PAR_LEN + 1];
static ascii gprs_user[E_GPRS_MAX_PAR_LEN + 1];
static ascii gprs_pass[E_GPRS_MAX_PAR_LEN + 1];
static ascii gsm_pincode[E_GPRS_MAX_PAR_LEN + 1];

static wip_bearer_t bearer = NULL;
static adl_tmr_t *reconnect_timer = NULL;
static adl_tmr_t *poll_creg_timer = NULL;

static _gprs_state _state;

static bool poll_creg_callback(adl_atResponse_t *rsp);
static void poll_creg_cancel_delayed(void);

static void stop_bearer(wip_bearer_t b)
{
    s8 rc;
    e_assert(b != NULL);

    if ((rc = wip_bearerStop(b))) {
        if (WIP_BERR_OK_INPROGRESS == rc) {
            E_DBG("bearerStop failed, another disconnection is in progress");
        } else {
            ERR_LOG(wip_bearerStop, rc);
        }
    }
}

static void close_bearer(wip_bearer_t *b)
{
    s8 rc;

    if (NULL == b || NULL == *b) {
        return;
    }

    E_DBG("Closing GPRS bearer");
    if((rc = wip_bearerClose(*b))) {
        ERR_LOG(wip_bearerClose, rc);
    }

    *b = NULL;
}

static e_ret start_bearer(wip_bearer_t b)
{
	e_ret rc = ERROR;
	s8 r;

    e_assert(b != NULL);

    E_DBG("WIP bearer opened, starting, APN %s, USER %s",
            gprs_apn,
            gprs_user);

	r = wip_bearerSetOpts(b,
		WIP_BOPT_GPRS_APN, 	gprs_apn,
		WIP_BOPT_LOGIN,   	gprs_user,
		WIP_BOPT_PASSWORD,	gprs_pass,
		WIP_BOPT_END);
	if( r != OK ) {
        ERR_LOG(wip_bearerSetOpts, r);
		goto exit;
	}

	r = wip_bearerStart(b);
	if( OK != r && WIP_BERR_OK_INPROGRESS != r) {
        ERR_LOG(wip_bearerStart, r);
    	goto exit;
	}

    E_DBG("WIP bearer started");

    rc = OK;

exit:
	return rc;
}

static void poll_creg(void)
{
	E_STACK("poll_creg enter");

    poll_creg_cancel_delayed();

	if( adl_atCmdCreate("AT+CREG?", FALSE, poll_creg_callback,
                ADL_STR_CREG, NULL) != OK ) {
		E_FATAL("Error creating AT command 'AT+CREG?', stopping");
	}

	E_STACK("poll_creg exit");
}

static void poll_creg_tmr_wrp( u8 ID, /*@null@*/ void * Context )
{
	E_STACK("poll_creg_tmr_wrp enter");
    poll_creg_timer = NULL;
	poll_creg();
	E_STACK("poll_creg_tmr_wrp exit");
}

static void poll_creg_cancel_delayed(void)
{
	E_STACK("poll_creg_cancel_delayed enter");
    if (poll_creg_timer != NULL) {
        s32 res;
        res = adl_tmrUnSubscribe(poll_creg_timer, poll_creg_tmr_wrp,
                ADL_TMR_TYPE_100MS);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        } else {
            poll_creg_timer = NULL;
        }
    }
	E_STACK("poll_creg_cancel_delayed exit");
}

static void poll_creg_delayed(void)
{
    if (poll_creg_timer != NULL) {
        E_DBG("Poll creg already scheduled, ignoring.");
        return;
    }

	poll_creg_timer = adl_tmrSubscribe( FALSE, CREG_POLLING_PERIOD,
			ADL_TMR_TYPE_100MS, poll_creg_tmr_wrp);
	if ((poll_creg_timer == NULL) || (poll_creg_timer < 0)) {
		ERR_LOG(adl_tmrSubscribe, poll_creg_timer);
        poll_creg_timer = NULL;
	}
}

static void reconnect_cancel_delayed(void);

static void reconnect(void)
{
	E_INFO("reconnecting to GPRS");

    reconnect_cancel_delayed();

    if(start_bearer(bearer)) {
        close_bearer(&bearer);
    	poll_creg();
    }
}

static void reconnect_tmr_wrp(u8 id, void* context)
{
    reconnect_timer = NULL;
    reconnect();
}

static void reconnect_cancel_delayed(void)
{
    if (reconnect_timer != NULL) {
        s32 res;
        res = adl_tmrUnSubscribe(reconnect_timer, reconnect_tmr_wrp, 
                ADL_TMR_TYPE_100MS);
        if (res < 0) {
            ERR_LOG(adl_tmrUnSubscribe, res);
        } else {
            reconnect_timer = NULL;
        }
    }
}

static void reconnect_delayed(void)
{
    if (reconnect_timer != NULL) {
        E_DBG("Already reconnecting, ignoring reconnect request.");
        return;
    }

	reconnect_timer = adl_tmrSubscribe(FALSE, 
            RECONNECT_PERIOD, ADL_TMR_TYPE_100MS, 
            reconnect_tmr_wrp);
	if((reconnect_timer == NULL) || (reconnect_timer < 0)) {
		ERR_LOG(adl_tmrSubscribe, reconnect_timer);
        reconnect_timer = NULL;
        /* FIXME: notify of fatal error to callback */
	}
}

static void evh_bearer(wip_bearer_t b, s8 event, void *ctx) 
{
    e_assert(b != NULL);
    e_assert(bearer != NULL);
    e_assert(b == bearer);

	E_STACK("evh_bearer enter");

	switch(event) {
	case WIP_BEV_IP_CONNECTED:
        E_INFO("Connected to GPRS bearer");
        _state = _CONNECTED;
        if (_gprs_ev_handler != NULL) {
		    _gprs_ev_handler(E_GPRS_CONNECTED, _gprs_ev_handler_context);
        }
        break;
	case WIP_BEV_IP_DISCONNECTED:
		E_DBG("GPRS Connection terminated");
        if (_STOPPING != _state) {
            stop_bearer(b);
        }
		break;
	case WIP_BEV_CONN_FAILED:
		E_WARN("Connection failed");
		reconnect_delayed();
		break;
	case WIP_BEV_STOPPED:
		E_INFO("Bearer stopped");
        if (_CONFIGURING == _state) {
            close_bearer(&b);
            bearer = NULL;
            _state = _DISCONNECTED;
            if (_gprs_ev_handler != NULL) {
	    	    _gprs_ev_handler(E_GPRS_DISCONNECTED, _gprs_ev_handler_context);
            }
            poll_creg_delayed();
        } else if (_STOPPING == _state) {
            close_bearer(&b);
            bearer = NULL;
            _state = _DISCONNECTED;
            if (_gprs_ev_handler != NULL) {
	    	    _gprs_ev_handler(E_GPRS_DISCONNECTED, _gprs_ev_handler_context);
            }
        } else {
            _state = _DISCONNECTED;
            if (_gprs_ev_handler != NULL) {
	    	    _gprs_ev_handler(E_GPRS_DISCONNECTED, _gprs_ev_handler_context); 
            }
	    	reconnect_delayed();
        }
		break;
	default:
		E_FATAL("Unknown bearer event: %d", event);
        e_assert(FALSE);
        break;
	}

    E_STACK("evh_bearer exit");
}

static e_ret open_and_start_bearer(wip_bearer_t* b)
{
	e_ret rc = ERROR;
	s8 r;

    e_assert(b != NULL);
    e_assert(*b == NULL);

	E_STACK("open_and_start_bearer");

    reconnect_cancel_delayed();

    *b = NULL;

	r = wip_bearerOpen(b, "GPRS", evh_bearer, NULL);
	if( r != OK) {
        if (WIP_BERR_ALREADY == r) {
            E_DBG("WIP bearer already opened, ignoring.");
        } else {
            *b = NULL;
            ERR_LOG(wip_bearerOpen, r);
    		goto exit;
        }
	}

	e_assert(*b != NULL);

    if((rc = start_bearer(*b))) {
        goto exit;
    }

    rc = OK;

exit:
    if(rc) {
    	if(*b != NULL) {
            close_bearer(b);
        }
    }
	return rc;
}

/* answer to +CREG? */
static bool poll_creg_callback(adl_atResponse_t *rsp)
{
    e_ret rc = ERROR;

    e_assert(bearer == NULL);
    e_assert(rsp != NULL);

	E_STACK("poll_creg_callback enter");

    if (_STOPPING == _state) {
        rc = OK;
        goto exit;
    }

    poll_creg_cancel_delayed();

    /* avoid excesively long responses */
    if (rsp->StrLength < 128) {
        ascii rsp_str[rsp->StrLength];
        s32 rsp_code;
        /* reply is +CREG: <mode>,<stat>[,<lac>,<cid>],
         * AT Guide 11.2*/
    	(void) wm_strGetParameterString(rsp_str, rsp->StrData, 2);
    	rsp_code = (s32) wm_atoi(rsp_str);

        E_DBG("creg reply %ld", ((long)rsp_code));

	    if( (1 == rsp_code)         /* registered, ome network */
                || (5 == rsp_code)  /* registered, roaming */
                ) {
		    E_INFO("Registered on GPRS network, opening bearer");
    		if(!open_and_start_bearer(&bearer)) {
                rc = OK;
		    }
        }
	} else {
        E_WARN("Response string too long: %lu",
                (unsigned long) rsp->StrLength);
    }

exit:
    if(rc) {
    	E_DBG("rechecking GPRS network.");
        poll_creg_delayed();
    }

	E_STACK("poll_creg_callback exit");
	return FALSE;
}

static void evh_sim(u8 event)
{
	E_STACK("evh_sim enter");

	switch( event ) {
	case ADL_SIM_EVENT_FULL_INIT:
		E_DBG("SIM Initialized.");
        if (_gprs_ev_handler) {
    		_gprs_ev_handler(E_GPRS_INIT_OK, _gprs_ev_handler_context);
        }
		poll_creg();
        goto exit;
	case ADL_SIM_EVENT_PIN_OK:
		E_INFO("PIN accepted."); break;
	case ADL_SIM_EVENT_REMOVED:
		E_ERR("SIM Card removed!."); break;
	case ADL_SIM_EVENT_INSERTED:
		E_INFO("SIM Card inserted."); break;
	case ADL_SIM_EVENT_PIN_ERROR:
		E_ERR("Invalid PIN."); break;
	case ADL_SIM_EVENT_PIN_WAIT:
		E_ERR("No PIN provided."); break;
	default:
		E_FATAL("Unknown PIN event (%d).", event);
        e_abort(0, "SIM event failed");
	}

    if (_gprs_ev_handler) {
    	_gprs_ev_handler(E_GPRS_INIT_FAILED, _gprs_ev_handler_context);
    }

exit:
    E_STACK("evh_sim exit");
}


static void at_set_config_handler(adl_atCmdPreParser_t *params)
{
    u8 rc;

    e_assert(params != NULL);

    E_STACK("at_set_config_handler enter");

    rc = OK;
    switch(params->Type) {
    case ADL_CMD_TYPE_PARA:
        /* min params=3, max params = 3*/
        if (3 == params->NbPara) {
            ascii* apn;
            ascii* user;
            ascii* pass;
            apn = ADL_GET_PARAM(params, 0);
            user = ADL_GET_PARAM(params, 1);
            pass = ADL_GET_PARAM(params, 2);
            if (apn != NULL && user != NULL && pass != NULL) {
                if(e_gprs_set_config(apn, user, pass) != OK) {
                    rc = 3;
                }
                /* TODO store GPRS config in flash? */
            } else {
                rc = 4;
            }
        } else {
            rc = 5;
        }
        break;
    case ADL_CMD_TYPE_READ:
        {
        ascii resp[sizeof(gprs_apn) + sizeof(gprs_user) +
            sizeof(gprs_pass) + 25];
        /* careful with length! */
        /*@-bufferoverflowhigh@*/
        wm_sprintf(resp, "\r\n+EGPRS: \"%s\",\"%s\",\"%s\"\r",
                gprs_apn,gprs_user,gprs_pass);
        /*@+bufferoverflowhigh@*/
        (void) adl_atSendResponse (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ), resp );
        }
        goto exit;

    default:
        rc = 6;
    }

    if (!rc) {
        (void) adl_atSendStdResponse (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ), ADL_STR_OK );
    } else {
        (void) adl_atSendStdResponseExt (
                ADL_AT_PORT_TYPE ( params->Port, ADL_AT_RSP ),
                    ADL_STR_CME_ERROR, rc );
    }

exit:
    E_STACK("at_set_config_handler exit");
}

static s8 _adl_gprs_handler(u16 event, u8 cid) {
    E_DBG("GPRS event handler: event=%lu, cid=%lu",
            (unsigned long) event,
            (unsigned long) cid);
    return ADL_GPRS_FORWARD;
}

e_ret e_gprs_set_config(const ascii* apn, const ascii* user, const ascii* pass)
{
    e_assert(apn != NULL);
    e_assert(user != NULL);
    e_assert(pass != NULL);

    if (strlen(apn) > E_GPRS_MAX_PAR_LEN
            || strlen(user) > E_GPRS_MAX_PAR_LEN
            || strlen(pass) > E_GPRS_MAX_PAR_LEN) {
        return ERROR;
    }

    (void) strlcpy(gprs_apn, apn, E_GPRS_MAX_PAR_LEN + 1);
    (void) strlcpy(gprs_user, user, E_GPRS_MAX_PAR_LEN + 1);
    (void) strlcpy(gprs_pass, pass, E_GPRS_MAX_PAR_LEN + 1);

    if (bearer != NULL) {
        _state = _CONFIGURING;
        stop_bearer(bearer);
    }

    return OK;
}

bool e_gprs_is_connected(void) {
    return _CONNECTED == _state;
}

int e_gprs_register_at_configure(void) 
{
    int rc = ERROR;
    s16 res;

    E_STACK("register_at_configure enter");

    res = adl_atCmdSubscribe("AT+EGPRS", at_set_config_handler, 
            (u16) (ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA | 0x33));
    if (res != OK) {
        ERR_LOG(adl_atCmdSubscribe, res);
        goto exit;
    }
    rc = OK;

exit:
    E_STACK("register_at_configure exit");

    return rc;
}

static bool _sim_init = FALSE;

e_ret e_gprs_init_default(e_gprs_ev_handler handler, void *context){
    E_STACK("e_gprs_init_default enter");
    return e_gprs_init(NULL, gprs_apn, gprs_user, gprs_pass, handler, context);
    E_STACK("e_gprs_init_default exit");
}


e_ret e_gprs_init (
        const ascii *pincode,
        const ascii *apn,
        const ascii *user,
        const ascii *pass,
        e_gprs_ev_handler entry_point,
        void *context)
{
	s32 rc;
    s8 res2;

	E_STACK("e_gprs_init enter");

    _state = _DISCONNECTED;

    if ((apn == NULL) || (user == NULL) || (pass == NULL)) {
        E_ERR("GPRS configuration not set");
        return ERROR;
    }

    if(e_gprs_set_config(apn, user, pass)) {
        return ERROR;
    }

    if (E_LOG_IS(DEBUG)) {
        res2 = adl_gprsSubscribe(_adl_gprs_handler);
        if (res2 != OK) {
            ERR_LOG(adl_gprsSubscribe, res2);
        }
    }

	_gprs_ev_handler = entry_point;
    _gprs_ev_handler_context = context;

    if(pincode != NULL && !_sim_init) {
        _sim_init = TRUE;
        (void) strlcpy(gsm_pincode, pincode, E_GPRS_MAX_PAR_LEN + 1);
    	rc = adl_simSubscribe(evh_sim, gsm_pincode);
        if (rc != OK) {
            ERR_LOG(adl_simSubscribe, rc);
            goto exit;
        }
    } else {
        evh_sim(ADL_SIM_EVENT_FULL_INIT);
    }
    rc = OK;
	E_STACK("e_gprs_init exit");

exit:
    return OK;
}

void e_gprs_stop(void) {
	E_STACK("e_gprs_stop enter");

    E_DBG("Stopping GPRS");

    _state = _STOPPING;
    _gprs_ev_handler = NULL;
    _gprs_ev_handler_context = NULL;

    if (bearer) {
        E_DBG("...stopping bearer");
        stop_bearer(bearer);
    }

    if (_sim_init) {
		s32 rc;
        E_DBG("Unsubscribing SIM");
        if((rc = adl_simUnsubscribe(evh_sim)) != OK) {
			ERR_LOG(adl_simUnsubscribe, rc);
		}
        _sim_init = FALSE;
    }

    E_STACK("e_gprs_stop exit");
}
