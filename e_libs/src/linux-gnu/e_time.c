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

#include <time.h>
#include <errno.h>
#include <string.h>

#include <stdlib.h>

#ifdef HAVE_TIMEGM
#include <time.h>
#endif

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, struct timezone *tz) {
	E_UNUSED(tz);
    tv->tv_sec = time(NULL);
    if (tv->tv_sec == ((time_t)-1)) {
        return -1;
    }
    tv->tv_usec = 0;
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
    return time(t);
}

time_t e_time_timegm(struct tm* tm) {
/* FIXME configure build environment to be able to use POSIX functions here
 * so we can implement timegm without much hassle */
#if defined HAVE_TIMEGM && !defined __STRICT_ANSI__
    return timegm(tm);
#elif defined(WIN_MSVC)
	return _mkgmtime(tm);
#else
    /*  Based on code from:
     *  http://lists.samba.org/archive/samba-technical/2002-November/025737.html
     *  Which was taken from:
     *  Contributed by Roger Beeman <beeman@xxxxxxxxx>, with the help of
     *  Mark Baushke <mdb@xxxxxxxxx> and the rest of the Gurus at CISCO.
     *  Further improved by Roger with assistance from Edward J. Sabol
     *  based on input by Jamie Zawinski.
     */
    time_t tl, tb;
    struct tm *tg;

    tl = mktime (tm);
    if (tl == -1) {
        tm->tm_hour--;
        tl = mktime (tm);
        if (tl == -1)
            return -1; /* can't deal with output from strptime */
        tl += 3600;
    }
    tg = gmtime (&tl);
    tg->tm_isdst = 0;
    tb = mktime (tg);
    if (tb == -1) {
        tg->tm_hour--;
        tb = mktime (tg);
        if (tb == -1)
            return -1; /* can't deal with output from gmtime */
        tb += 3600;
    }
    return (tl - (tb - tl));
#endif
}

e_ret e_time_gmtime(time_t t, struct tm* tm) {
    struct tm* r;
    r = gmtime(&t);
    if (r) {
        *tm = *r;
        return OK;
    } else {
        return ERROR;
    }
}
