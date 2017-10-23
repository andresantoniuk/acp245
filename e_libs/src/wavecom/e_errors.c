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

#include "e_errors.h"

#include "adl_global.h"

#include "e_port.h"
#include "e_log.h"

/**
 * WARNING:
 * This file has very special semantics to avoid code duplication if possible.
 * The e_log E_* macros are redefined to use the file and line provider by 
 * parameter in all of the err_log_* functions. 
 * Therefore, while inside this file, E_* macros can only be used inside an 
 * err_log function which declares a file and line parameter.
 */
#ifdef __e_log_h_MACROS
#undef E_LOG
#define E_LOG(x, ...)		if(e_log_errlevel >= x) \
        { e_log_file(file, line, x, __VA_ARGS__ ); }
#endif

void err_log_adl_tmrUnSubscribe(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_BAD_HDL:
        E_ERR("Invalid handler provided to unsubscribe");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Handler already unsubscribed");
        /* timer unsubscribed in handler */
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_tmrUnSubscribe error: 0x%lx", (long) rc);
    }
}

void err_log_adl_tmrSubscribe(const char* file, const long line, adl_tmr_t* r) 
{
    if(r == NULL) {
        E_ERR("No timers left");
    } else if( (int) r == ADL_RET_ERR_SERVICE_LOCKED) {
        E_ERR("Service locked");
    } else {
        E_FATAL("Unknown adl_tmrSubscribe error");
    }
}

void err_log_adl_tmrSubscribeExt(const char* file, const long line, adl_tmr_t* r) 
{
    if(r == NULL) {
        E_ERR("No timers left");
    } else if( (int) r == ADL_RET_ERR_SERVICE_LOCKED) {
        E_ERR("Service locked");
    } else {
        E_FATAL("Unknown adl_tmrSubscribeExt error");
    }
}

void err_log_adl_ioEventSubscribe(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid param to adl_ioEventSubscribe");
        break;
    case ADL_RET_ERR_NO_MORE_HANDLES:
        E_ERR("No more GPIO subscription handles");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_ioEventSubscribe error: 0x%lx", (long) rc);
    }
}

void err_log_adl_ioSubscribe(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid param to adl_ioSubscribe");
        break;
    case ADL_RET_ERR_DONE:
        /* refer to 3.10.2.6 */
        E_ERR("GPIO subscription error: %ld", (long) rc);
        break;
    case ADL_RET_ERR_NO_MORE_TIMERS:
        E_ERR("No more timers available");
        break;
    case ADL_RET_ERR_NO_MORE_HANDLES:
        E_ERR("No more handlers");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_ioSubscribe error: 0x%lx", (long) rc);
    }
}

void err_log_adl_ioUnsubscribe(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle for adl_ioUnsubscribe");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_ioUnsubscribe error: 0x%x", (long) rc);
    }
}

void err_log_adl_ioWriteSingle(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid param to adl_ioWriteSingle");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle for adl_ioWriteSingle");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("adl_ioWriteSingle failed, One of the required GPIOs \
                was not subscribed as an output");
        break;
    default:
        E_FATAL("Unknown adl_ioWriteSingle error: 0x%x", rc);
    }
}

void err_log_adl_ioReadSingle(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid param to adl_ioReadSingle");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle for adl_ioReadSingle");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("adl_ioReadSingle failed, One of the required GPIOs \
                was not subscribed as an input");
        break;
    default:
        E_FATAL("Unknown adl_ioReadSingle error: 0x%x", rc);
    }
}

void err_log_adl_atCmdSubscribe(const char* file, const long line, s16 rc) 
{
    switch(rc) {
    case ERROR:
        E_ERR("Error subscribing to AT command");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_atCmdSubscribe error: 0x%hx", (short) rc);
    }
}

void err_log_adl_atCmdCreate(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ERROR:
        E_ERR("Error creating AT command");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_atCmdCreate error: 0x%hx", (short) rc);
    }
}

void err_log_adl_atCmdSendExt(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ERROR:
        E_ERR("Error sending AT command");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_atCmdSendExt error: 0x%hx", (short) rc);
    }
}

void err_log_adl_atSendStdResponse(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_atSendStdResponse error: 0x%ld", (long) rc);
    }
}

void err_log_adl_atSendStdResponseExt(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_atSendStdResponse error: 0x%lx", (long) rc);
    }
}

void err_log_adl_fcmSubscribe(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter for adl_fcmSubscribe");
        break;
    case ADL_RET_ERR_ALREADY_SUBSCRIBED:
        E_ERR("Port already subscribed to FCM service");
        break;
    case ADL_RET_ERR_NOT_SUBSCRIBED:
        E_ERR("Master flow not subscribed, slave subscribe failed");
        break;
    case ADL_FCM_RET_ERROR_GSM_GPRS_ALREADY_OPENNED:
        E_ERR("Can perform a GSM/GPRS subscription while the other is already subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("adl_fcmSubscribe failed, port not available");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_fcmSubscribe error: 0x%hx", (short) rc);
    }
}

void err_log_adl_fcmUnsubscribe(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle adl_fcmUnsubscribe");
        break;
    case ADL_RET_ERR_NOT_SUBSCRIBED:
        E_ERR("Flow not subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("adl_fcmUnsubscribe failed, serial link is not in AT mode");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_fcmUnsubscribe error: 0x%hx", (short) rc);
    }
}

void err_log_adl_fcmSendData(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ADL_FCM_RET_OK_WAIT_RESUME:
        E_ERR("Send succeeded, but the flow has no more credit to use");
        break;
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter for adl_fcmSendData");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle for adl_fcmSendData");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Flow is not ready to send data");
        break;
    case ADL_FCM_RET_ERR_WAIT_RESUME:
        E_ERR("The flow has no more credit to use");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_fcmSendData error: 0x%hx", (short) rc);
    }
}

void err_log_adl_fcmSwitchV24State(const char* file, const long line, s8 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter for adl_fcmSwitchV24State");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle for adl_fcmSwitchV24State");
        break;
    case ADL_RET_ERR_BAD_HDL:
        E_ERR("Bad handle for adl_fcmSwitchV24State");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("Unknown adl_fcmSwitchV24State error: 0x%hx", (short) rc);
    }
}

void err_log_adl_rtcGetTime(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter for adl_rtcGetTime");
        break;
    default:
        E_FATAL("Unknown adl_rtcGetTime error: 0x%lx", (long) rc);
    }
}

void err_log_adl_rtcConvertTime(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ERROR:
        E_ERR("adl_rtcConverTime failed, internal error");
        break;
    case ADL_RET_ERR_PARAM:
        E_ERR("ald_rtcConverTime failed, invalid parameter");
        break;
    default:
        E_FATAL("Unknown ald_rtcConverTime error: 0x%lx", (long) rc);
    }
}

void err_log_adl_simSubscribe(const char* file, const long line, s32 rc) 
{
    switch(rc) {
    case ADL_RET_ERR_ALREADY_SUBSCRIBED:
        E_FATAL("SIM Subscribe failed. Service already subscribed for given handler!."); 
        break;
    case ADL_RET_ERR_PARAM:
        E_FATAL("SIM Subscribe failed. NULL handler provided!."); 
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("SIM Subscribe failed. Unknown return value: 0x%lx.", (long) rc); 
    }
}

void err_log_adl_simUnsubscribe(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("SIM Unsubscribe failed. Unknown return value: 0x%lx.", (long) rc);
    }
}


void err_log_adl_flhSubscribe(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Unknown parameter.");
        break;
    case ADL_FLH_RET_ERR_NO_ENOUGH_IDS:
        E_ERR("Not enough flash number IDs.");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_flhWrite failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_adl_flhExist(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle."); 
        break;
    case ADL_FLH_RET_ERR_ID_OUT_OF_RANGE:
        E_FATAL("ID out of range."); 
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_flhRead failed. Unknwon error: 0x%lx", (long) rc); 
        break;
    }
}

void err_log_adl_flhErase(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle."); 
        break;
    case ADL_FLH_RET_ERR_ID_OUT_OF_RANGE:
        E_FATAL("ID out of range."); 
        break;
    case ADL_FLH_RET_ERR_OBJ_NOT_EXIST:
        E_ERR("Object does not exists."); 
        break;
    case ADL_RET_ERR_FATAL:
        E_FATAL("Fatal error ocurred while erasing flash."); 
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_flhRead failed. Unknwon error: 0x%hx", (short) rc); 
        break;
    }
}

void err_log_adl_flhWrite(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_FATAL("Invalid parameter."); 
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle."); 
        break;
    case ADL_FLH_RET_ERR_ID_OUT_OF_RANGE:
        E_FATAL("ID out of range."); 
        break;
    case ADL_RET_ERR_FATAL:
        E_FATAL("Fatal error ocurred on while writing to flash."); 
        break;
    case ADL_FLH_RET_ERR_MEM_FULL:
        E_ERR("Flash memory full."); 
        break;
    case ADL_FLH_RET_ERR_NO_ENOUGH_IDS:
        E_ERR("Not enough flash number IDs."); 
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_flhWrite failed. Unknwon error: 0x%hx", (short) rc); 
        break;
    }
}

void err_log_adl_flhRead(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_FATAL("Invalid parameter."); 
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle."); 
        break;
    case ADL_FLH_RET_ERR_ID_OUT_OF_RANGE:
        E_FATAL("ID out of range."); 
        break;
    case ADL_FLH_RET_ERR_OBJ_NOT_EXIST:
        E_ERR("Object does not exists."); 
        break;
    case ADL_RET_ERR_FATAL:
        E_FATAL("Fatal error ocurred while reading from flash."); 
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_flhRead failed. Unknwon error: 0x%hx", (short) rc); 
        break;
    }
}

void err_log_adl_adSubscribe(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_ALREADY_SUBSCRIBED:
        E_ERR("Cell already subscribed");
        break;
    case ADL_AD_RET_ERR_OVERFLOW:
        E_ERR("Not enough allocated space for cell");
        break;
    case ADL_AD_RET_ERR_NOT_AVAILABLE:
        E_ERR("No A&D space available on the product");
        break;
    case ADL_RET_ERR_PARAM:
        E_FATAL("Bad Cell ID, 0xFFFFFFFF is not a valid cell ID.");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Another undefined size cell is already subscribed and not finalized");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adSubscribe failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adUnsubscribe(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adUnsubscribe failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adEventSubscribe(const char* file, const long line, s32 rc) {
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid handler function");
        break;
    case ADL_RET_ERR_NO_MORE_HANDLES:
        E_ERR("No more handles for A&D event subscriptions");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adEventSubscribe. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adEventUnsubscribe(const char* file, const long line, s32 rc) {
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_NOT_SUBSCRIBED:
        E_ERR("Not subscribed to A&D events");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Format or compact process is running with this handle");
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adEventUnsubscribe. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adWrite(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Cell finalized");
        break;
    case ADL_RET_ERR_OVERFLOW:
        E_ERR("Not enough space for data in cel");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adWrite failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adInfo(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Parameter error");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Cell is not finalized or of undefined size");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adInfo failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adFinalise(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Cell finalized");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adFinalise failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adDelete(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Cell is not finalized or of undefined size");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adDelete failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adInstall(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle not subscribed");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Cell is not finalized");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adInstall failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_adRecompact(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Re-compaction or format already running");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Handle unknown");
        break;
    case ADL_RET_ERR_NOT_SUBSCRIBED:
        E_ERR("Not subscribed to A&D events");
        break;
    case ADL_AD_RET_ERR_NOT_AVAILABLE:
        E_ERR("No A&D space available on the product");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_adRecompact failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_smsSubscribe(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_smsSubscribe failed. Unknown error: 0x%lx", (long) rc);
    }
}
void err_log_adl_smsUnsubscribe(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle");
        break;
    case ADL_RET_ERR_NOT_SUBSCRIBED:
        E_ERR("Not subscribed to SMS service");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Service processing pending SMS");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_smsUnsubscribe failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_smsSend(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Not ready to send SMS (not initialized or send pending)");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_smsSend failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_audioTonePlayExt(const char* file, const long line, s32 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("Invalid parameter");
        break;
    case ADL_RET_ERR_UNKNOWN_HDL:
        E_ERR("Unknown handle");
        break;
    case ADL_RET_ERR_BAD_STATE:
        E_ERR("Not ready to play audio stream, already in use?");
        break;
    case ADL_RET_ERR_BAD_HDL:
        E_ERR("Tone playing not allowed on audio resource");
        break;
    case ADL_RET_ERR_NOT_SUPPORTED:
        E_ERR("Tone playing not available on audio resource");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_audioTonePlayExt failed. Unknown error: 0x%lx", (long) rc);
    }
}

void err_log_adl_gprsSubscribe(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case ADL_RET_ERR_PARAM:
        E_ERR("GPRS subscribe failed, bad parameter");
        break;
    case ADL_RET_ERR_SERVICE_LOCKED:
        E_ERR("Service locked");
        break;
    default:
        E_FATAL("adl_gprsSubscribe failed. Unknown error: 0x%lx", (long) rc);
    }
}
