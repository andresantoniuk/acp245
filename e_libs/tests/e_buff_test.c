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
#include "e_buff.h"

#include "e_check.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

START_TEST (test_e_buff_displace_fwd)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[520];
    u8 d1[20];
    u8 b;
    u32 i;

    e_buff_wrap(buff, data, sizeof(data));

    for (i = 0; i < sizeof(d1); i++) {
        d1[i] = i;
    }
    e_buff_write_buff(buff, d1, sizeof(d1));

    ARE_EQ_INT(e_buff_get_lim(buff), sizeof(d1));
    ARE_EQ_INT(e_buff_read_remain(buff), sizeof(d1));
    e_buff_displace_fwd(buff, 10, 5);
    ARE_EQ_INT(e_buff_get_lim(buff), sizeof(d1) + 5);
    ARE_EQ_INT(e_buff_read_remain(buff), sizeof(d1) + 5);

    for (i = 0; i < 15; i++) {
        res = e_buff_read(buff, &b);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(i, b);
    }
    for (i = 10; i < 20; i++) {
        res = e_buff_read(buff, &b);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(i, b);
    }

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_compact)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[50];
    u8 d1[20];
    u8 b;
    u32 i;

    e_buff_wrap(buff, data, sizeof(data));

    for (i = 0; i < sizeof(d1); i++) {
        d1[i] = i;
    }
    e_buff_write_buff(buff, d1, sizeof(d1));

    for (i = 0; i < sizeof(d1); i++) {
        ARE_EQ_INT(sizeof(d1) - i, e_buff_read_remain(buff));
        ARE_EQ_INT(0, e_buff_get_pos(buff));
        ARE_EQ_INT(sizeof(d1) -i, e_buff_get_lim(buff));

        res = e_buff_read(buff, &b);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(i, b);
        ARE_EQ_INT(1, e_buff_get_pos(buff));
        ARE_EQ_INT(sizeof(d1) -i, e_buff_get_lim(buff));

        e_buff_compact(buff);
        ARE_EQ_INT(0, e_buff_get_pos(buff));
        ARE_EQ_INT(sizeof(d1) -i - 1, e_buff_get_lim(buff));
    }

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_read_write)
{
    e_ret res;
    u8 data[512];
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 b;
    int i;

    e_buff_wrap(buff, data, sizeof(data));
    ARE_EQ_INT(512, e_buff_capacity(buff));
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    ARE_EQ_INT(512, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(0, e_buff_get_lim(buff));

    e_buff_reset(buff);
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(0, e_buff_get_lim(buff));

    res = e_buff_write(buff, 0x1);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(1, e_buff_read_remain(buff));
    ARE_EQ_INT(511, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(1, e_buff_get_lim(buff));

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x1, b);
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    ARE_EQ_INT(511, e_buff_write_remain(buff));
    ARE_EQ_INT(1, e_buff_get_pos(buff));
    ARE_EQ_INT(1, e_buff_get_lim(buff));

    e_buff_reset(buff);
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    ARE_EQ_INT(512, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(0, e_buff_get_lim(buff));

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(E_BUFF_OVERFLOW, res);
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    ARE_EQ_INT(512, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(0, e_buff_get_lim(buff));

    res = e_buff_write(buff, 0x2);
    ARE_EQ_INT(OK, res);

    res = e_buff_write(buff, 0x3);
    ARE_EQ_INT(OK, res);

    res = e_buff_write(buff, 0x4);
    ARE_EQ_INT(OK, res);

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x2, b);

    res = e_buff_write(buff, 0x5);

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x3, b);

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x4, b);

    res = e_buff_read(buff, &b);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x5, b);

    e_buff_reset(buff);
    for (i = 0; i < 512; i++) {
        res = e_buff_write(buff, (u8)(i % 256));
        ARE_EQ_INT(OK, res);
    }
    ARE_EQ_INT(512, e_buff_read_remain(buff));
    ARE_EQ_INT(0, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(512, e_buff_get_lim(buff));

    res = e_buff_write(buff, (u8)0x1);
    ARE_EQ_INT(E_BUFF_OVERFLOW, res);
    ARE_EQ_INT(512, e_buff_read_remain(buff));
    ARE_EQ_INT(0, e_buff_write_remain(buff));
    ARE_EQ_INT(0, e_buff_get_pos(buff));
    ARE_EQ_INT(512, e_buff_get_lim(buff));

    for (i = 0; i < 512; i++) {
        ARE_EQ_INT((u32)(512-i), e_buff_read_remain(buff));
        res = e_buff_read(buff, &b);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT((u8) (i % 256), b);
    }

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_read_write_u16)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[5];
    u16 d;
    u16 dr;

    e_buff_wrap(buff, data, sizeof(data));
    d = 0xF2CB;
    res = e_buff_write_u16(buff, d);
    ARE_EQ_INT(OK, res);
    res = e_buff_read_u16(buff, &dr);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(d, dr);

    d = 0x0111;
    res = e_buff_write_u16(buff, d);
    ARE_EQ_INT(OK, res);
    res = e_buff_read_u16(buff, &dr);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(d, dr);

    res = e_buff_read_u16(buff, &dr);
    ARE_EQ_INT(E_BUFF_OVERFLOW, res);

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_read_write_u32)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[9];
    u32 d;
    u32 dr;

    e_buff_wrap(buff, data, sizeof(data));
    d = 0xABCDEF00;
    res = e_buff_write_u32(buff, d);
    ARE_EQ_INT(OK, res);
    res = e_buff_read_u32(buff, &dr);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(d, dr);

    d = 0x00FEDCBA;
    res = e_buff_write_u32(buff, d);
    ARE_EQ_INT(OK, res);
    res = e_buff_read_u32(buff, &dr);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(d, dr);

    res = e_buff_read_u32(buff, &dr);
    ARE_EQ_INT(E_BUFF_OVERFLOW, res);

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_read_write_buff)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[512];
    u8 data_2[1024];
    u8 b;
    u32 i;
    u32 j;
    size_t sz = 0;

    e_buff_wrap(buff, data_2, 1024);

    for (j = 0; j < 512; j++) {
        int first;
        sz++;
        first = sz/2;
        for (i = 0; i < sz; i++) {
            data[i] = (i % 256);
        }
        /* use two additional writes */
        res = e_buff_write_buff(buff, data, first);
        ARE_EQ_INT(OK, res);
        res = e_buff_write_buff(buff, data + first, sz  - first);
        ARE_EQ_INT(OK, res);

        ARE_EQ_INT(OK, res);
        NOT_NULL(buff);
        ARE_EQ_INT(1024, e_buff_capacity(buff));
        ARE_EQ_INT(sz, e_buff_read_remain(buff));
        ARE_EQ_INT(0, e_buff_get_pos(buff));
        ARE_EQ_INT(sz, e_buff_get_lim(buff));
        ARE_EQ_INT(1024-sz, e_buff_write_remain(buff));

        for (i = 0; i < sz; i++) {
            ARE_EQ_INT(sz - i, e_buff_read_remain(buff));
            ARE_EQ_INT(i, e_buff_get_pos(buff));

            res = e_buff_read(buff, &b);
            ARE_EQ_INT(OK, res);
            ARE_EQ_INT(i%256, b);
            ARE_EQ_INT(1024, e_buff_capacity(buff));
            ARE_EQ_INT(1024-sz, e_buff_write_remain(buff));
            ARE_EQ_INT(sz, e_buff_get_lim(buff));
        }

        e_buff_reset(buff);
    }

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_read_write_ascii)
{
    e_ret res;
    e_buff buff_s;
    e_buff* buff = &buff_s;
    u8 data[512];
    ascii str[512];

    e_buff_wrap(buff, data, sizeof(data));
    res = e_buff_write_ascii(buff, "FOOBAR");
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(7, e_buff_read_remain(buff));

    res = e_buff_read_ascii(buff, str, 512);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    ARE_EQ_STR("FOOBAR", str);

    e_buff_reset(buff);

    res = e_buff_write_ascii(buff, "FOOBAR");
    ARE_EQ_INT(OK, res);
    res = e_buff_read_ascii(buff, str, 4);
    ARE_EQ_INT(ERROR, res);

    e_buff_set_lim(buff, 508);
    e_buff_set_pos(buff, 508);

    res = e_buff_write_ascii(buff, "FOOB");
    ARE_EQ_INT(E_BUFF_OVERFLOW, res);

    res = e_buff_write_ascii(buff, "FOO");
    ARE_EQ_INT(OK, res);

    res = e_buff_read_ascii(buff, str, 512);
    ARE_EQ_STR("FOO", str);

    e_buff_reset(buff);
    e_buff_set_lim(buff, 508);
    e_buff_set_pos(buff, 508);
    e_buff_write_u32(buff, 0xFFFFFFFF);
    ARE_EQ_INT(0, e_buff_write_remain(buff));
    res = e_buff_read_ascii(buff, str, 512);
    ARE_EQ_INT(ERROR, res);

    e_buff_dealloc(buff);
}
END_TEST

START_TEST (test_e_buff_skip)
{
    e_ret res;
    e_buff buff;
    u8 data[256];
    int i;

    e_buff_wrap(&buff, data, 256);
    for (i = 0; i < 255; i++) {
        res = e_buff_write(&buff, (u8)(i & 0xFF));
        ARE_EQ_INT(OK, res);
    }

    ARE_EQ_INT(255, e_buff_read_remain(&buff));
    ARE_EQ_INT(1, e_buff_write_remain(&buff));

    res = e_buff_skip(&buff, 1);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(254, e_buff_read_remain(&buff));
    ARE_EQ_INT(1, e_buff_write_remain(&buff));

    res = e_buff_skip(&buff, 50);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(254 - 50, e_buff_read_remain(&buff));
    ARE_EQ_INT(1, e_buff_write_remain(&buff));

    e_buff_reset(&buff);
    for (i = 0; i < 255; i++) {
        res = e_buff_write(&buff, (u8)(i & 0xFF));
        ARE_EQ_INT(OK, res);
    }

    ARE_EQ_INT(255, e_buff_read_remain(&buff));
    for (i = 1; i <= 255; i++) {
        res = e_buff_skip(&buff, 1);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(255 - i, e_buff_read_remain(&buff));
        ARE_EQ_INT(1, e_buff_write_remain(&buff));
    }

    e_buff_reset(&buff);
    for (i = 0; i < 255; i++) {
        res = e_buff_write(&buff, (u8)(i & 0xFF));
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(1, e_buff_read_remain(&buff));
        res = e_buff_skip(&buff, 1);
        ARE_EQ_INT(OK, res);
        ARE_EQ_INT(0, e_buff_read_remain(&buff));
        ARE_EQ_INT(255-i, e_buff_write_remain(&buff));
    }

    e_buff_dealloc(&buff);
}
END_TEST

START_TEST (test_e_buff_peek)
{
	e_buff buff;
	u8 data[] = { 0xA1, 0xB2, 0x13, 0x24, 0x35, 0x46, 0xF7, 0xC8, 0xE9,
			0x1A, 0x2B, 0x3C, 0x4D, 0xFE, 0x5F };
	e_ret rc;
	u8 b;

	e_buff_wrap(&buff, data, sizeof(data));
	e_buff_set_lim(&buff, 2);
	rc = e_buff_peek(&buff, 0, &b);
	ARE_EQ_INT(OK, rc);
	ARE_EQ_INT(0xA1, b);

	rc = e_buff_peek(&buff, 1, &b);
	ARE_EQ_INT(OK, rc);
	ARE_EQ_INT(0xB2, b);

	rc = e_buff_peek(&buff, 3, &b);
	ARE_EQ_INT(E_BUFF_OVERFLOW, rc);
}
END_TEST

START_TEST (test_e_buff_read_hex)
{
	e_buff buff;
	u8 data[] = { 0xA1, 0xB2, 0x13, 0x24, 0x35, 0x46, 0xF7, 0xC8, 0xE9,
			0x1A, 0x2B, 0x3C, 0x4D, 0xFE, 0x5F };
	ascii hex[256];
	u32 r;

	e_buff_wrap(&buff, data, sizeof(data));
	e_buff_set_lim(&buff, sizeof(data));
	r = e_buff_read_hex(&buff, hex, 0);
	ARE_EQ_INT(0, r);
	ARE_EQ_INT(15, e_buff_read_remain(&buff));

	r = e_buff_read_hex(&buff, hex, 1);
	ARE_EQ_INT(0, r);
	ARE_EQ_INT(15, e_buff_read_remain(&buff));

	r = e_buff_read_hex(&buff, hex, 2);
	ARE_EQ_INT(0, r);
	ARE_EQ_INT(15, e_buff_read_remain(&buff));

	r = e_buff_read_hex(&buff, hex, 3);
	ARE_EQ_INT(2, r);
	ARE_EQ_INT('\0', hex[2]);
	ARE_EQ_INT(2, strlen(hex));
	ARE_EQ_STR("A1", hex);
	ARE_EQ_INT(14, e_buff_read_remain(&buff));

	r = e_buff_read_hex(&buff, hex, 31);
	ARE_EQ_INT(28, r);
	ARE_EQ_INT('\0', hex[28]);
	ARE_EQ_INT(28, strlen(hex));
	ARE_EQ_STR("B213243546F7C8E91A2B3C4DFE5F", hex);
	ARE_EQ_INT(0, e_buff_read_remain(&buff));
}
END_TEST

START_TEST (test_e_buff_peek_hex)
{
    e_buff buff;
    u8 data[] = { 0xA1, 0xB2, 0x13, 0x24, 0x35, 0x46, 0xF7, 0xC8, 0xE9,
    		0x1A, 0x2B, 0x3C, 0x4D, 0xFE, 0x5F };
	ascii hex[256];
	u32 r;

    e_buff_wrap(&buff, data, sizeof(data));
    e_buff_set_lim(&buff, e_buff_write_remain(&buff));
    r = e_buff_peek_hex(&buff, hex, 0);
    ARE_EQ_INT(0, r);

    r = e_buff_peek_hex(&buff, hex, 1);
    ARE_EQ_INT(0, r);

    r = e_buff_peek_hex(&buff, hex, 2);
    ARE_EQ_INT(0, r);

    r = e_buff_peek_hex(&buff, hex, 3);
    ARE_EQ_INT(2, r);
    ARE_EQ_INT('\0', hex[2]);
    ARE_EQ_INT(2, strlen(hex));
    ARE_EQ_STR("A1", hex);

    r = e_buff_peek_hex(&buff, hex, 31);
    ARE_EQ_INT(30, r);
	ARE_EQ_INT('\0', hex[30]);
	ARE_EQ_INT(30, strlen(hex));
	ARE_EQ_STR("A1B213243546F7C8E91A2B3C4DFE5F", hex);
}
END_TEST

extern int e_buff_test(void);
int e_buff_test(void) {
    return e_check_run_suite("e_buff",
            test_e_buff_read_write,
            test_e_buff_read_write_u16,
            test_e_buff_read_write_u32,
            test_e_buff_read_write_buff,
            test_e_buff_read_write_ascii,
            test_e_buff_compact,
            test_e_buff_displace_fwd,
            test_e_buff_skip,
            test_e_buff_peek,
            test_e_buff_read_hex,
            test_e_buff_peek_hex,
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
	return e_buff_test();
}
#endif
