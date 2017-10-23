/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_syslog_h_
#define __e_syslog_h_

#include "e_port.h"

#ifdef E_LIBS_NET_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

#define E_SYSLOG_BUFSIZ   4092
#define E_SYSLOG_MAX_LEN   384

/* Defined on http://tools.ietf.org/html/rfc3164 */
#define E_SYSLOG_USER       1
#define E_SYSLOG_DAEMON     3
#define E_SYSLOG_AUTH       4
#define E_SYSLOG_SECURITY   10
#define E_SYSLOG_LOCAL_0    16
#define E_SYSLOG_LOCAL_2    17
#define E_SYSLOG_LOCAL_3    18

#define E_SYSLOG_EMERG      0
#define E_SYSLOG_ALERT      1
#define E_SYSLOG_CRIT       2
#define E_SYSLOG_ERROR      3
#define E_SYSLOG_WARN       4
#define E_SYSLOG_NOTICE     5
#define E_SYSLOG_INFO       6
#define E_SYSLOG_DEBUG      7

extern e_ret e_syslog_openlog(const char *ident, int facility,
        const ascii *host, const u16 port, const u16 local_port);

extern e_ret e_syslog(int prio, const char* str);

extern void e_syslog_closelog(void);

extern void e_syslog_set_level(int level);

extern void e_syslog_set_as_e_log(void);

#ifdef __cplusplus
}
#endif

#endif /* E_LIBS_NET_ENABLE */

#endif
