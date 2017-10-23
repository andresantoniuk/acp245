/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_log_wm_h_
#define __e_log_wm_h_

#ifdef __cplusplus
extern "C" {
#endif

#define E_LOG_WM_AT_SET_LEVEL   "AT+ELOG"

extern int e_log_wm_register_at_set_level(void);

#ifdef __cplusplus
}
#endif

#endif /* __e_log_wm_h */
