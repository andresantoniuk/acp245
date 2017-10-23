/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef _e_errors_wip_h_
#define _e_errors_wip_h_

#include "e_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void err_log_wip_bearerOpen(const char* file, const long line, s8 rc);

extern void err_log_wip_bearerStart(const char* file, const long line, s8 rc);

extern void err_log_wip_bearerSetOpts(const char* file, const long line, s8 rc);

extern void err_log_wip_bearerStop(const char* file, const long line, s8 rc);

extern void err_log_wip_bearerClose(const char* file, const long line, s8 rc);

extern void err_log_wip_setOpts(const char* file, const long line, int rc);

extern void err_log_wip_close(const char* file, const long line, int rc);

extern void err_log_wip_shutdown(const char* file, const long line, int rc);

extern void err_log_wip_write(const char* file, const long line, int rc);

extern void err_log_wip_read(const char* file, const long line, int rc);

#ifdef __cplusplus
}
#endif

#endif
