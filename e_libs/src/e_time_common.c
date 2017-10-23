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

const ascii* e_time_month_name(u8 month) {
    static const ascii* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    return months[(month - 1) % (12)];
}

e_ret e_time_gmtime_now(struct tm* tm) {
    time_t t;

    t = e_time_time(NULL);
    if (t == ((time_t) -1)) {
        return ERROR;
    }
    return e_time_gmtime(t, tm);
}
