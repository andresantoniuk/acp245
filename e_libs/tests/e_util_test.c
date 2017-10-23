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
#include "e_util.h"

#include "e_check.h"
#include "e_mem.h"
#include "e_time.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

START_TEST (test_e_std_date_now)
{
	char* d;

	d = e_std_date_now();
	NOT_NULL(d);
}
END_TEST

START_TEST (test_e_std_date)
{
	char* d;
	struct tm t;
	time_t t_arg;

	memset(&t, 0, sizeof(struct tm));
	t.tm_year = 100;
	t.tm_mon = 1;
	t.tm_mday = 2;
	t.tm_hour = 13;
	t.tm_min = 45;
	t.tm_sec = 52;

	t_arg = e_time_timegm(&t);

	d = e_std_date(&t_arg);
	NOT_NULL(d);
	ARE_EQ_STR("2000-02-02 13:45:52", d);
}
END_TEST

START_TEST (test_e_time_timegm)
{
    struct tm t;
    time_t t_arg;

	memset(&t, 0, sizeof(struct tm));
    t.tm_year = 109;
    t.tm_mon = 8;
    t.tm_mday = 17;
    t.tm_hour = 19;
    t.tm_min = 7;
    t.tm_sec = 11;

    t_arg = e_time_timegm(&t);
    ARE_EQ_INT(1253214431, t_arg);
}
END_TEST

START_TEST (test_e_file_access)
{
	int rc;
	rc = e_file_access(CASE_PATH("e_util_file_access"));
	IS_TRUE(rc);

	rc = e_file_access(CASE_PATH("e_util_file_access_foobar"));
	IS_FALSE(rc);
}
END_TEST

START_TEST (test_e_str_strip)
{
	const char* s;
	char* st;

	s = "FOOBAR";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("FOOBAR", st);
    e_mem_free(st);

	s = " FOOBAR ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("FOOBAR", st);
    e_mem_free(st);

	s = " FOOBAR";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("FOOBAR", st);
    e_mem_free(st);

	s = "FOOBAR ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("FOOBAR", st);
    e_mem_free(st);

	s = " FOOBAR ";
	st = e_str_strip(s, strlen(s) - 1);
	ARE_EQ_STR("FOOBAR", st);
    e_mem_free(st);

	s = " FOOBAR ";
	st = e_str_strip(s, strlen(s) - 2);
	ARE_EQ_STR("FOOBA", st);
    e_mem_free(st);

	s = " F ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("F", st);
    e_mem_free(st);

	s = " F";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("F", st);
    e_mem_free(st);

	s = "F ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("F", st);
    e_mem_free(st);

	s = "F";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("F", st);
    e_mem_free(st);

	s = "  ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("", st);
    e_mem_free(st);

	s = " ";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("", st);
    e_mem_free(st);

	s = "";
	st = e_str_strip(s, strlen(s));
	ARE_EQ_STR("", st);
    e_mem_free(st);

}
END_TEST

START_TEST (test_e_util_to_hex)
{
	unsigned char buf[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xFF };
	char* hex;

	hex = e_util_to_hex(buf, sizeof(buf));
	ARE_EQ_STR("000102030405ff", hex);
	e_mem_free(hex);

	hex = e_util_to_hex(buf, 0);
	ARE_EQ_STR("", hex);
	e_mem_free(hex);

	hex = e_util_to_hex(buf, 1);
	ARE_EQ_STR("00", hex);
	e_mem_free(hex);

	hex = e_util_to_hex(buf, 2);
	ARE_EQ_STR("0001", hex);
	e_mem_free(hex);
}
END_TEST

START_TEST (test_e_util_to_bin)
{
	u8 buf[7];
	u8 expected_buf[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xFF };
    const ascii *hexstr = "000102030405ff";
    char *hex;

	ARE_EQ_INT(OK, e_util_to_bin(buf, hexstr, 7));
    ARE_EQ_BINC(expected_buf, buf, 7);

    hex = e_util_to_hex(buf, 7);
    ARE_EQ_STR(hexstr, hex);
    e_mem_free(hex);
}
END_TEST

extern int e_util_test(void);
int e_util_test(void) {
    return e_check_run_suite("e_util",
            test_e_file_access,
            test_e_str_strip,
            test_e_time_timegm,
            test_e_std_date,
            test_e_std_date_now,
            test_e_util_to_hex,
            test_e_util_to_bin,
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
	return e_util_test();
}
#endif
