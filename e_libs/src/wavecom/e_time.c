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

#include "e_time.h"

#include "adl_global.h"

#include "e_port.h"
#include "e_errors.h"
#include "e_log.h"

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, struct timezone *tz) {
    s32 res;
    adl_rtcTime_t  time;
    adl_rtcTimeStamp_t ts;

    if ((res = adl_rtcGetTime(&time)) != OK) {
        ERR_LOG(adl_rtcGetTime, res);
        return -1;
    }

    if ((res = adl_rtcConvertTime( &time, &ts, ADL_RTC_CONVERT_TO_TIMESTAMP))) {
        ERR_LOG(adl_rtcConvertTime, res);
        return -1;
    }
    tv->tv_sec = ts.TimeStamp;
    tv->tv_usec = ts.SecondFracPart*ADL_RTC_SECOND_FRACPART_STEP/1000000;
    return 0;
}
#endif

extern int e_time_cmp(struct timeval *tv1, struct timeval *tv2) {
    if (tv1->tv_sec == tv2->tv_sec && tv1->tv_usec == tv2->tv_usec) {
        return 0;
    }
    if (tv1->tv_sec < tv2->tv_sec) {
        return -1;
    }
    if (tv1->tv_sec > tv2->tv_sec) {
        return 1;
    }
    return tv1->tv_usec < tv2->tv_usec ? -1 : 1;
}

void e_time_add(struct timeval *tv1, struct timeval *tv2, struct timeval *r) {
    u32 usecs;
    r->tv_sec = tv1->tv_sec + tv2->tv_sec;
    usecs = tv1->tv_usec + tv2->tv_usec;
    if (usecs > 1000000) {
        r->tv_sec++;
        r->tv_usec = usecs - 1000000;
    } else {
        r->tv_usec = usecs;
    }
}

void e_time_sub(struct timeval *tv1, struct timeval *tv2, struct timeval *r) {
    r->tv_sec = tv1->tv_sec - tv2->tv_sec;
    if (tv1->tv_usec < tv2->tv_usec) {
        r->tv_sec--;
        r->tv_usec = (1000000) - (tv2->tv_usec - tv1->tv_usec);
    } else {
        r->tv_usec = tv1->tv_usec - tv2->tv_usec;
    }
}

time_t e_time_time(time_t *t) {
    struct timeval ts;
    if(gettimeofday(&ts, NULL)) {
        return ((time_t)-1);
    } else {
        return ts.tv_sec;
    }
}

e_ret e_time_gmtime(time_t time, struct tm* tm) {
    e_ret rc = ERROR;
    s32 res;
    adl_rtcTime_t t;
    adl_rtcTimeStamp_t ts;

    e_assert(tm != NULL);

    ts.TimeStamp = time;

    if ((res = adl_rtcConvertTime(&t, &ts, ADL_RTC_CONVERT_FROM_TIMESTAMP))) {
        ERR_LOG(adl_rtcConvertTime, res);
        goto exit;
    }

    (void) e_mem_set(tm, 0, sizeof(struct tm));

    /*@+matchanyintegral@*/
    tm->tm_year = (t.Year - 1900);
    tm->tm_mon = (t.Month - 1);
    tm->tm_mday = t.Day;
    tm->tm_wday = (t.WeekDay - 1);
    tm->tm_hour = t.Hour;
    tm->tm_min = t.Minute;
    tm->tm_sec = t.Second;
    /*@-matchanyintegral@*/

    rc = OK;

exit:
    return OK;
}

time_t e_time_timegm(struct tm* tm) {
    s32 res;
    adl_rtcTime_t  t;
    adl_rtcTimeStamp_t ts;

    (void) e_mem_set(&t, 0, sizeof(adl_rtcTime_t));

    t.Year = (u16) (tm->tm_year + 1900);
    t.Month = (u8) (tm->tm_mon + 1);
    t.Day = (u8) (tm->tm_mday);
    /* FIXME check if WeekDay starts on Sunday...*/
    t.WeekDay = (u8) (tm->tm_wday + 1);
    t.Hour = (u8) (tm->tm_hour);
    t.Minute = (u8) (tm->tm_min);
    /* FIXME skipping leap seconds... */
    t.Second = (u8) (tm->tm_sec % 60);

    if ((res = adl_rtcConvertTime( &t, &ts, ADL_RTC_CONVERT_TO_TIMESTAMP))) {
        ERR_LOG(adl_rtcConvertTime, res);
        return (time_t)-1;
    }

    return (time_t) ts.TimeStamp;
}
