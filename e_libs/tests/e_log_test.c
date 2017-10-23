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
#include "e_log.h"

#include "e_check.h"
#include "e_mem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef E_LIBS_LOG_ENABLE
START_TEST (test_e_log_get_level)
{
    E_LOG_SET_LEVEL(ERR);
    ARE_EQ_INT(E_LOG_ERR, e_log_get_level());
}
END_TEST

START_TEST (test_E_ERR)
{
    char* err;
    E_LOG_SET_LEVEL(ERR);
    E_LOG_SET_FACILITY(BUFFER);
    E_ERR("BARFOO %s %d", "bar", 20);
    err = e_log_get_err();
    IS_IN_STR(err, "BARFOO bar 20");
    e_mem_free(err);
}
END_TEST

START_TEST (test_e_log_file)
{
    char* err;
    E_LOG_SET_LEVEL(ERR);
    E_LOG_SET_FACILITY(BUFFER);
    e_log_file("barfoo.c", 20, E_LOG_ERR, "FOOBAR %s %d", "foo", 10);
    err = e_log_get_err();
    IS_IN_STR(err, "[ERRO] barfoo.c:20 - FOOBAR foo 10");
    e_mem_free(err);
}
END_TEST

START_TEST (test_e_log_set_facility)
{
    ARE_EQ_INT(0, e_log_set_facility(FACILITY_CONSOLE));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CONSOLE);

    ARE_EQ_INT(0, e_log_set_facility(FACILITY_CUSTOM));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CUSTOM);

    ARE_EQ_INT(0, e_log_set_facility(FACILITY_BUFFER));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_BUFFER);
}
END_TEST

static int _level;
static ascii _msg[512];
static void _handler(int level, ascii* msg) {
    _level = level;
    strlcpy(_msg, msg, 512);
}

START_TEST (test_e_log_set_custom_handler)
{
    _level = 0;
    e_mem_set(_msg, 0, sizeof(_msg));

    E_LOG_SET_LEVEL(ERR);
    E_LOG_SET_FACILITY(CUSTOM);
    e_log_file("barfoo.c", 20, E_LOG_ERR, "FOOBAR %s %d", "foo", 10);
    ARE_EQ_INT(0, _level);
    ARE_EQ_INT(0, strlen(_msg));

    e_log_set_custom_handler(_handler);
    e_log_file("barfoo.c", 20, E_LOG_ERR, "FOOBAR %s %d", "foo", 10);

    ARE_EQ_INT(E_LOG_ERR, _level);
    IS_IN_STR(_msg, "[ERRO] barfoo.c:20 - FOOBAR foo 10");
}
END_TEST

START_TEST (test_e_log_add_facility)
{
    char* err;
    e_log_set_facility(FACILITY_NONE);
    ARE_EQ_INT(e_log_get_facility(), FACILITY_NONE);

    ARE_EQ_INT(0, e_log_add_facility(FACILITY_CONSOLE));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CONSOLE);

    ARE_EQ_INT(0, e_log_add_facility(FACILITY_CUSTOM));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CONSOLE | FACILITY_CUSTOM);

    ARE_EQ_INT(0, e_log_add_facility(FACILITY_BUFFER));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CONSOLE | FACILITY_CUSTOM | FACILITY_BUFFER);

    e_log_set_facility(FACILITY_NONE);
    ARE_EQ_INT(e_log_get_facility(), FACILITY_NONE);

    /* test logging to multiple facilities */
    _level = 0;
    e_mem_set(_msg, 0, sizeof(_msg));

    ARE_EQ_INT(0, e_log_add_facility(FACILITY_CUSTOM | FACILITY_BUFFER));
    ARE_EQ_INT(e_log_get_facility(), FACILITY_CUSTOM | FACILITY_BUFFER);

    E_LOG_SET_LEVEL(ERR);
    e_log_set_custom_handler(_handler);
    e_log_file("barfoo.c", 20, E_LOG_ERR, "FOOBAR %s %d", "foo", 10);

    err = e_log_get_err();
    IS_IN_STR(err, "[ERRO] barfoo.c:20 - FOOBAR foo 10");
    e_mem_free(err);

    ARE_EQ_INT(E_LOG_ERR, _level);
    IS_IN_STR(_msg, "[ERRO] barfoo.c:20 - FOOBAR foo 10");
}
END_TEST

#endif /* E_LIBS_LOG_ENABLE */

extern int e_log_test (void);
int e_log_test (void) {
    return e_check_run_suite("e_log",
#ifdef E_LIBS_LOG_ENABLE
            test_E_ERR,
            test_e_log_file,
            test_e_log_set_facility,
            test_e_log_set_custom_handler,
            test_e_log_add_facility,
            test_e_log_get_level,
#endif
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
    return e_log_test();
}
#endif
