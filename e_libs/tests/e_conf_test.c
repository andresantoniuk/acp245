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
#include "e_conf.h"

#include "e_check.h"
#include "e_log.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef E_LIBS_CONF_ENABLE

START_TEST (test_e_conf_open)
{
	e_conf* conf = NULL;
	int rc;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);
	NOT_NULL(conf);
	e_conf_free(&conf);

	rc = e_conf_open(&conf, CASE_PATH("must.fail_not.exists"));
	IS_NOT_OK(rc);
	IS_NULL(conf);

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_bad.conf"));
	IS_NOT_OK(rc);
	IS_NULL(conf);
}
END_TEST

START_TEST (test_e_conf_close)
{
	int rc;
	e_conf* conf = NULL;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

	e_conf_free(&conf);
	IS_NULL(conf);
}
END_TEST

START_TEST (test_e_conf_def_str)
{
	e_conf* conf = NULL;
	int rc;
	const char* v;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

	v = e_conf_str(conf, "section", "def1");
	IS_NULL(v);

	v = e_conf_def_str("section", "def1");
	IS_NULL(v);

    e_conf_def_set(CASE_PATH("e_conf_test_def_ok.conf"));
	v = e_conf_str(conf, "section", "def1");
	IS_NULL(v);

	v = e_conf_def_str("section", "def1");
	ARE_EQ_STR("val1", v);

	e_conf_free(&conf);
	e_conf_def_free();
}
END_TEST

START_TEST (test_e_conf_bool)
{
	e_conf* conf = NULL;
	int rc;
    int v = -1;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

    v = -1;
    e_conf_bool(conf, "foo", "bar", &v);
	IS_TRUE(v);

    v = -1;
    e_conf_bool(conf, "foo", "bar2", &v);
	IS_TRUE(v);

    v = -1;
    e_conf_bool(conf, "foo", "bar3", &v);
	IS_TRUE(v);

    v = -1;
    e_conf_bool(conf, "foo", "bar4", &v);
	IS_FALSE(v);

	e_conf_free(&conf);
}
END_TEST

START_TEST (test_e_conf_str_c)
{
	e_conf* conf = NULL;
	int rc;
	char* v1;
	char* v2;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

	v1 = e_conf_str_c(conf, "section", "option");
	ARE_EQ_STR("value", v1);

	v2 = e_conf_str_c(conf, "section", "option");
	ARE_EQ_STR("value", v2);

    IS_FALSE((v1 == v2));
    free(v1);
    free(v2);

    e_conf_free(&conf);
}
END_TEST

START_TEST (test_e_conf_str)
{
	e_conf* conf = NULL;
	int rc;
	const char* v;
	const char* v1;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

	v = e_conf_str(conf, "section", "option");
	ARE_EQ_STR("value", v);

	v1 = e_conf_str(conf, "section", "option");
	ARE_EQ_STR("value", v1);
    IS_TRUE((v == v1));

	v = e_conf_str(conf, "foo", "bar");
	ARE_EQ_STR("1", v);

	v = e_conf_str(conf, "foo", "baz");
	ARE_EQ_STR("foobar", v);

	v = e_conf_str(conf, "section", "option2");
	ARE_EQ_STR("value value2", v);

	v = e_conf_str(conf, "section", "option3");
	ARE_EQ_STR("value3", v);

	v = e_conf_str(conf, "section", "option4");
	ARE_EQ_STR("u=1 u = 2", v);


	v = e_conf_str(conf, "section", "long5");
	ARE_EQ_STR("a=b c = d  e=f gggg xxxxxxxxxxx=yyyyyyyyyy zzzzzzzzzzzzzzzzzzzzzzzzz", v);

	v = e_conf_str(conf, "section", "verylong");
	ARE_EQ_INT(8193, strlen(v));

	e_conf_free(&conf);
}
END_TEST

START_TEST (test_e_conf_int)
{
	int rc;
	u32 v;
	e_conf* conf = NULL;

	rc = e_conf_open(&conf, CASE_PATH("e_conf_test_ok.conf"));
	IS_OK(rc);

	rc = e_conf_int(conf, "foo", "bar", &v);
	IS_OK(e_conf_int(conf, "foo", "bar", &v));

	ARE_EQ_INT(1, v);

	e_conf_free(&conf);
}
END_TEST

#endif /* E_LIBS_CONF_ENABLE */

extern int e_conf_test(void);
int e_conf_test (void) {
    return e_check_run_suite("e_conf",
#ifdef E_LIBS_CONF_ENABLE
            test_e_conf_open,
            test_e_conf_close,
            test_e_conf_int,
            test_e_conf_str,
            test_e_conf_str_c,
            test_e_conf_bool,
            test_e_conf_def_str,
#endif
            NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
	return e_conf_test();
}
#endif
