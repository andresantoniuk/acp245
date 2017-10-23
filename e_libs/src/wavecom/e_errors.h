/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef _e_errors_h_
#define _e_errors_h_

#include "e_port.h"
#include "adl_global.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_LOG(func, rc)   err_log_##func(__FILE__, __LINE__, rc)

#define ERR_LOG_DEC(name, res) extern void err_log_##name(const char* file, const long line, res);

ERR_LOG_DEC(adl_tmrSubscribe, adl_tmr_t* r)

ERR_LOG_DEC(adl_tmrSubscribeExt, adl_tmr_t* r)

ERR_LOG_DEC(adl_tmrUnSubscribe, s32 rc)

ERR_LOG_DEC(adl_ioSubscribe, s32 rc)

ERR_LOG_DEC(adl_ioUnsubscribe, s32 rc)

ERR_LOG_DEC(adl_ioEventSubscribe, s32 rc)

ERR_LOG_DEC(adl_ioWriteSingle, s32 rc)

ERR_LOG_DEC(adl_ioReadSingle, s32 rc)

ERR_LOG_DEC(adl_atCmdSubscribe, s16 rc)

ERR_LOG_DEC(adl_atCmdCreate, s8 rc)

ERR_LOG_DEC(adl_atCmdSendExt, s8 rc)

ERR_LOG_DEC(adl_atSendStdResponse, s32 rc)

ERR_LOG_DEC(adl_atSendStdResponseExt, s32 rc)

ERR_LOG_DEC(adl_fcmSubscribe, s8 rc)

ERR_LOG_DEC(adl_fcmUnsubscribe, s8 rc)

ERR_LOG_DEC(adl_fcmSendData, s8 rc)

ERR_LOG_DEC(adl_fcmSwitchV24State, s8 rc)

ERR_LOG_DEC(adl_rtcGetTime, s32 rc)

ERR_LOG_DEC(adl_rtcConvertTime, s32 rc)

ERR_LOG_DEC(adl_simSubscribe, s32 rc)

ERR_LOG_DEC(adl_simUnsubscribe, s32 rc)

ERR_LOG_DEC(adl_flhSubscribe, s8 rc)

ERR_LOG_DEC(adl_flhExist, s32 rc)

ERR_LOG_DEC(adl_flhErase, s8 rc)

ERR_LOG_DEC(adl_flhWrite, s8 rc)

ERR_LOG_DEC(adl_flhRead, s8 rc)

ERR_LOG_DEC(adl_adSubscribe, s32 rc)

ERR_LOG_DEC(adl_adUnsubscribe, s32 rc)

ERR_LOG_DEC(adl_adEventSubscribe, s32 rc)

ERR_LOG_DEC(adl_adEventUnsubscribe, s32 rc)

ERR_LOG_DEC(adl_adWrite, s32 rc)

ERR_LOG_DEC(adl_adInfo, s32 rc)

ERR_LOG_DEC(adl_adFinalise, s32 rc)

ERR_LOG_DEC(adl_adDelete, s32 rc)

ERR_LOG_DEC(adl_adInstall, s32 rc)

ERR_LOG_DEC(adl_adRecompact, s32 rc)

ERR_LOG_DEC(adl_smsSubscribe, s8 rc)

ERR_LOG_DEC(adl_smsUnsubscribe, s8 rc)

ERR_LOG_DEC(adl_smsSend, s8 rc)

ERR_LOG_DEC(adl_audioTonePlayExt, s32 rc)

ERR_LOG_DEC(adl_gprsSubscribe, s8 rc)

#ifdef __cplusplus
}
#endif

#endif
