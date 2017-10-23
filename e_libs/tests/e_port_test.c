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
#include "e_port.h"

#include "e_check.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

START_TEST (test_strlcat)
{
    char dst[5];
    memset(dst, 0, sizeof(dst));

	ARE_EQ_STR("", dst);

    strlcat(dst, "1234", sizeof(dst));
	ARE_EQ_STR("1234", dst);

    dst[0] = '\0';
    strlcat(dst, "abcdefg", sizeof(dst));
	ARE_EQ_STR("abcd", dst);

    strlcat(dst, "543", sizeof(dst));
	ARE_EQ_STR("abcd", dst);

    dst[0] = '\0';
    strlcat(dst, "543", sizeof(dst));
	ARE_EQ_STR("543", dst);
}
END_TEST

START_TEST (test_strlcpy)
{
    char dst[5];
    memset(dst, 0, sizeof(dst));

	ARE_EQ_STR("", dst);

    strlcpy(dst, "1234567", sizeof(dst));
	ARE_EQ_STR("1234", dst);

    strlcpy(dst, "543", sizeof(dst));
	ARE_EQ_STR("543", dst);

    dst[0] = '\0';
    strlcpy(dst, "542", sizeof(dst));
	ARE_EQ_STR("542", dst);
}
END_TEST

extern int e_port_test (void);
int e_port_test (void) {
    return e_check_run_suite("e_port",
            test_strlcat,
            test_strlcpy,
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
	return e_port_test();
}
#endif
