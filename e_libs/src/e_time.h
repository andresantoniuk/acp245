/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_time_h_
#define __e_time_h_

#include "e_port.h"

#ifndef HAVE_GETTIMEOFDAY
#ifdef __cplusplus
extern "C" {
#endif
typedef u32 suseconds_t;
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
extern int gettimeofday(struct timeval *tv, struct timezone *tz);
#ifdef __cplusplus
}
#endif
#else
#include <sys/time.h>
#endif
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int e_time_cmp(struct timeval *tv1, struct timeval *tv2);

extern void e_time_add(struct timeval *tv1,
        struct timeval *tv2, struct timeval *r);

extern void e_time_sub(struct timeval *tv1, 
        struct timeval *tv2, struct timeval *r);

/**
 * Returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in
 * seconds.
 * If t is non-NULL, the return value is also stored in the memory pointed to by t.
 *
 * @return On success, the value of time in seconds since the Epoch is returned
 * On error, ((time_t)-1) is returned.
 */
extern time_t e_time_time(time_t *t);

extern time_t e_time_timegm(struct tm* tm);

extern e_ret e_time_gmtime(time_t t, struct tm* tm);

extern e_ret e_time_gmtime_now(struct tm* tm);

extern const ascii* e_time_month_name(u8 month);

#ifdef __cplusplus
}
#endif

#endif
