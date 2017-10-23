/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "acp245_config.h"

#include "acp_el.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "e_check.h"
#include "e_buff.h"
#include "e_util.h"
#include "e_mem.h"

#include "acp_init.h"

#define MIN(x,y)    ((x) < (y) ? (x): (y))
#define ARE_EQ_BUFF_BIN(x, y)   { u8 __d_buff_bin[sizeof(x)];e_ret __rc;e_mem_set(__d_buff_bin, 0, sizeof(__d_buff_bin));ARE_EQ_INT(sizeof(x), e_buff_read_remain(y));__rc = e_buff_read_buff(y, __d_buff_bin, sizeof(__d_buff_bin));ARE_EQ_INT(OK, __rc);ARE_EQ_BINC(x, __d_buff_bin, sizeof(__d_buff_bin));}

static u8 buff_data[512];

START_TEST (test_acp_el_read_write_version_empty)
{
    acp_el_version el;
    e_ret res;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x00,
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_EMPTY, el.present);

    /* test write */
    e_buff_reset(buff);
    res = acp_el_write_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_version(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_version_not_present)
{
    acp_el_version el;
    e_ret res;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    /* test read */
    e_buff_reset(buff);
    res = acp_el_read_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, el.present);

    /* test write */
    e_buff_reset(buff);
    res = acp_el_write_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0, e_buff_read_remain(buff));
    acp_el_free_version(&el);
}
END_TEST

/* example from 3.1 */
START_TEST (test_acp_el_read_write_version_3_1)
{
    acp_el_version el;
    e_ret res;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x04,
        0x08,
        0x83,
        0x01,
        0x03,
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(8, el.car_manufacturer);
    ARE_EQ_INT(131, el.tcu_manufacturer);
    ARE_EQ_INT(1, el.major_hard_rel);
    ARE_EQ_INT(3, el.major_soft_rel);

    /* test write */
    e_buff_reset(buff);
    res = acp_el_write_version(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_version(&el);
}
END_TEST

/* example from 3.2 */
START_TEST (test_acp_el_read_write_timestamp_3_2)
{
    e_ret res;
    acp_el_timestamp el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = { 0x48, 0xDC, 0x3C, 0x38 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_timestamp(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(2008, el.year);
    ARE_EQ_INT(3, el.month);
    ARE_EQ_INT(14, el.day);
    ARE_EQ_INT(3, el.hour);
    ARE_EQ_INT(48, el.minute);
    ARE_EQ_INT(56, el.second);

    /* test write */
    e_buff_reset(buff);
    acp_el_write_timestamp(buff, &el);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_timestamp(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_timestamp_month)
{
    int i;
    e_ret res;
    acp_el_timestamp el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = { 0x49, 0xDC, 0x3C, 0x38 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_timestamp(buff, &el);

    /* check invalid date values */
    el.year = 2054;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);

    el.year = 1989;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);

    el.year = 1990;

    el.month = 13;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);
    el.month = 3;

    el.hour = 24;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);
    el.hour = 1;

    el.minute = 60;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);
    el.minute = 1;

    el.second = 60;
    res = acp_el_write_timestamp(buff, &el);
    ARE_EQ_INT(ACP_MSG_ERR_BAD_FORMAT, res);

    /* check valid ranges */

    for (i = 0; i < 0x3F; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.year = 1990 + i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(1990 + i, el.year);
        acp_el_free_timestamp(&el);
    }
    for (i = 0; i < 12; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.month = i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(i, el.month);
        acp_el_free_timestamp(&el);
    }
    for (i = 0; i < 31; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.day = i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(i, el.day);
        acp_el_free_timestamp(&el);
    }
    for (i = 0; i < 23; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.hour = i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(i, el.hour);
        acp_el_free_timestamp(&el);
    }
    for (i = 0; i < 59; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.minute = i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(i, el.minute);
        acp_el_free_timestamp(&el);
    }
    for (i = 0; i < 59; i++) {
        e_buff_reset(buff);
        e_buff_write_buff(buff, el1, sizeof(el1));
        res = acp_el_read_timestamp(buff, &el);
        el.second = i;

        e_buff_reset(buff);
        res = acp_el_write_timestamp(buff, &el);
        ARE_EQ_INT(OK, res);
        acp_el_read_timestamp(buff, &el);
        ARE_EQ_INT(i, el.second);
        acp_el_free_timestamp(&el);
    }
}
END_TEST

START_TEST (test_acp_el_read_write_tcu_desc)
{
    e_ret res;
    acp_el_tcu_desc el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x05,
        0xFF,
        0x01,
        0x55,
        0x01,
        0x44
    };
    u8 el2[] = {
        0x09,
        0x00,
        0x01,
        0x55,
        0x45,
        0x30, 0x31, 0x32, 0x33, 0x34,
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test el1 (int version) */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_tcu_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_PRESENT, el.present);
    ARE_EQ_INT(0x55, el.device_id);
    ARE_EQ_INT(ACP_IE_BINARY, el.version.id);
    ARE_EQ_INT(1, el.version.len);
    ARE_EQ_INT(0x44, el.version.data.bin[0]);
    acp_el_free_tcu_desc(&el);

    /* test el2 (string version) */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el2, sizeof(el2));
    res = acp_el_read_tcu_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_PRESENT, el.present);
    ARE_EQ_INT(0x55, el.device_id);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, el.version.id);
    ARE_EQ_STR("01234", el.version.data.str);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_tcu_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el2, buff);
    acp_el_free_tcu_desc(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_tcu_desc_empty)
{
    e_ret res;
    acp_el_tcu_desc el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x00,
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    e_mem_set(&el, 0, sizeof(el));
    res = acp_el_read_tcu_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_EMPTY, el.present);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_tcu_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_tcu_desc(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_vehicle_desc)
{
    e_ret res;
    acp_el_vehicle_desc el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    (void) e_mem_set(&el, 0, sizeof(acp_el_vehicle_desc));
    res = acp_el_read_vehicle_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_STR("0123456789ABCDEF0", el.vin);
    IS_TRUE(el.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, el.tcu_serial.id);
    ARE_EQ_STR("1234567", el.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", el.imei);
    ARE_EQ_STR("01234567890123456789", el.iccid);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_vehicle_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_vehicle_desc(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_vehicle_desc_bin_tcu_serial)
{
    e_ret res;
    acp_el_vehicle_desc el;
    e_buff buff_s;
    e_buff *buff = &buff_s;

    u8 el1[] = {
        0x20, 0x2D, 0xB1, 0x20,
        0x51,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x07,
            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* should match tcu_serial */
        0x85,
            0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A,
            0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
    };
    u8 tcu_serial[] = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    (void) e_mem_set(&el, 0, sizeof(acp_el_vehicle_desc));
    res = acp_el_read_vehicle_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_STR("0123456789ABCDEF0", el.vin);
    IS_TRUE(el.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_BINARY, el.tcu_serial.id);
    ARE_EQ_INT(7, el.tcu_serial.len);
    ARE_EQ_BIN(tcu_serial, el.tcu_serial.data.bin, el.tcu_serial.len);
    ARE_EQ_STR("0456000450", el.imei);
    ARE_EQ_STR("01234567890123456789", el.iccid);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_vehicle_desc(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_vehicle_desc(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_error)
{
    e_ret res;
    acp_el_error el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x01, 0x55 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_error(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x55, el.code);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_error(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_error(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_ctrl_func)
{
    e_ret res;
    acp_el_ctrl_func el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x03, 0xCA, 0x0B, 0x10 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_ctrl_func(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0xCA, el.entity_id);
    ARE_EQ_INT(0x0B, el.transmit_unit);
    ARE_EQ_INT(0x10, el.transmit_interval);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_ctrl_func(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_ctrl_func(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_func_cmd)
{
    e_ret res;
    acp_el_func_cmd el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x04, 0x0B, 0x02, 0x01, 0x02 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_func_cmd(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x0B, el.cmd);
    ARE_EQ_INT(ACP_EL_PRESENT, el.raw_data.present);
    ARE_EQ_INT(0x02, el.raw_data.data_len);
    ARE_EQ_INT(0x01, el.raw_data.data[0]);
    ARE_EQ_INT(0x02, el.raw_data.data[1]);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_func_cmd(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_func_cmd(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_location_not_present)
{
    e_ret res;
    acp_el_location el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    int i;
    u8 el1[] = {
        0x18,
        /* current gps raw data */
        0x17,
            /* area location code */
            0x12, 0x80, 0x45, 0x00, 0x00,
            /* lon */
            0xFD, 0x39, 0xA7, 0x2C,
            /* lat */
            0xFE, 0x99, 0x98, 0x0C,
            /* alt */
            0x0C, 0x0F,
            /* pos uncert */
            0x02,
            /* heading */
            0x22,
            0x07,
            0x04,
            /* satellites */
            0x30,
            0x07,
            0x05,
            0x03,
        /* followin elements not present */
        /* prior gps raw data */
        /* current dead reckoning data */
        /* area location delta code */
    };

    e_mem_set(&el, 0, sizeof(el));
    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_location(buff, &el);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(ACP_EL_PRESENT, el.curr_gps.present);
    ARE_EQ_INT(ACP_MORE_FLG, el.curr_gps.flg1);
    ARE_EQ_INT(0x45, el.curr_gps.flg2);
    ARE_EQ_INT(ACP_LOCATION_SOUTH_WEST, el.curr_gps.flg2 & ACP_LOCATION_FLG2_HEAD_MASK);
    ARE_EQ_INT(0, el.curr_gps.area_type);
    ARE_EQ_INT(0, el.curr_gps.location_type);
    ARE_EQ_INT(-46553300, el.curr_gps.lon);
    ARE_EQ_INT(-23488500, el.curr_gps.lat);
    ARE_EQ_INT(0x0C0F, el.curr_gps.alt);
    ARE_EQ_INT(0x1, el.curr_gps.pos_uncert);
    ARE_EQ_INT(0x1, el.curr_gps.head_uncert);
    ARE_EQ_INT(0x2, el.curr_gps.heading);
    ARE_EQ_INT(1, el.curr_gps.dist_unit);
    ARE_EQ_INT(3, el.curr_gps.time_unit);

    /* satellite data */
    ARE_EQ_INT(3, el.curr_gps.satellites_avail);
    ARE_EQ_INT(MIN(3, ACP_EL_GPS_RAW_DATA_SAT_MAX), el.curr_gps.satellites_cnt);
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 0) {
        ARE_EQ_INT(0x07, el.curr_gps.satellites[0]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 1) {
        ARE_EQ_INT(0x05, el.curr_gps.satellites[1]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 2) {
        ARE_EQ_INT(0x03, el.curr_gps.satellites[2]);
    }
    for (i = 3; i < ACP_EL_GPS_RAW_DATA_SAT_MAX; i++) {
        ARE_EQ_INT(0x00, el.curr_gps.satellites[i]);
    }

    ARE_EQ_INT(ACP_EL_NOT_PRESENT, el.prev_gps.present);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, el.dead_reck.present);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, el.loc_delta.present);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_location(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_location(&el);
}
END_TEST
START_TEST (test_acp_el_read_write_location)
{
    e_ret res;
    acp_el_location el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    int i;
    u8 el1[] = {
        0x1B,
        /* current gps raw data */
        0x17,
            /* area location code */
            0x12, 0x80, 0x45, 0x00, 0x00,
            /* lon */
            0xFD, 0x39, 0xA7, 0x2C,
            /* lat */
            0xFE, 0x99, 0x98, 0x0C,
            /* alt */
            0x0C, 0x0F,
            /* pos uncert */
            0x02,
            /* heading */
            0x22,
            0x07,
            0x04,
            /* satellites */
            0x30,
            0x07,
            0x05,
            0x03,
        /* prior gps raw data */
        0x00,
        /* current dead reckoning data */
        0x00,
        /* area location delta code */
        0x00
    };

    e_mem_set(&el, 0, sizeof(el));
    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_location(buff, &el);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(ACP_EL_PRESENT, el.curr_gps.present);
    ARE_EQ_INT(ACP_MORE_FLG, el.curr_gps.flg1);
    ARE_EQ_INT(0x45, el.curr_gps.flg2);
    ARE_EQ_INT(ACP_LOCATION_SOUTH_WEST,
            el.curr_gps.flg2 & ACP_LOCATION_FLG2_HEAD_MASK);
    ARE_EQ_INT(0, el.curr_gps.area_type);
    ARE_EQ_INT(0, el.curr_gps.location_type);
    ARE_EQ_INT(-46553300, el.curr_gps.lon);
    ARE_EQ_INT(-23488500, el.curr_gps.lat);
    ARE_EQ_INT(0x0C0F, el.curr_gps.alt);
    ARE_EQ_INT(0x1, el.curr_gps.pos_uncert);
    ARE_EQ_INT(0x1, el.curr_gps.head_uncert);
    ARE_EQ_INT(0x2, el.curr_gps.heading);
    ARE_EQ_INT(1, el.curr_gps.dist_unit);
    ARE_EQ_INT(3, el.curr_gps.time_unit);

    /* satellite data */
    ARE_EQ_INT(3, el.curr_gps.satellites_avail);
    ARE_EQ_INT(MIN(3, ACP_EL_GPS_RAW_DATA_SAT_MAX), el.curr_gps.satellites_cnt);
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 0) {
        ARE_EQ_INT(0x07, el.curr_gps.satellites[0]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 1) {
        ARE_EQ_INT(0x05, el.curr_gps.satellites[1]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 2) {
        ARE_EQ_INT(0x03, el.curr_gps.satellites[2]);
    }
    for (i = 3; i < ACP_EL_GPS_RAW_DATA_SAT_MAX; i++) {
        ARE_EQ_INT(0x00, el.curr_gps.satellites[i]);
    }

    ARE_EQ_INT(ACP_EL_EMPTY, el.prev_gps.present);
    ARE_EQ_INT(0, el.prev_gps.area_type);
    ARE_EQ_INT(0, el.prev_gps.location_type);
    ARE_EQ_INT(0, el.prev_gps.lon);
    ARE_EQ_INT(0x0, el.prev_gps.lat);
    ARE_EQ_INT(0x0, el.prev_gps.alt);
    ARE_EQ_INT(0x0, el.prev_gps.pos_uncert);
    ARE_EQ_INT(0x0, el.prev_gps.head_uncert);
    ARE_EQ_INT(0x0, el.prev_gps.heading);
    ARE_EQ_INT(0, el.prev_gps.dist_unit);
    ARE_EQ_INT(0, el.prev_gps.time_unit);

    ARE_EQ_INT(ACP_EL_EMPTY, el.dead_reck.present);
    ARE_EQ_INT(ACP_EL_EMPTY, el.dead_reck.present);
    ARE_EQ_INT(ACP_EL_EMPTY, el.loc_delta.present);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_location(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_location(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_loc_delta)
{
    e_ret res;
    acp_el_loc_delta el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = {
        0x03,
        0x81,
        0x82,
        0x03
    };
    u8 el1_rep[] = {
        0x04,
        0x81,
        0x82,
        0x83,
        0x00
    };

    e_mem_set(&el, 0, sizeof(el));
    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_loc_delta(buff, &el);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(2, el.delta_cnt);
    ARE_EQ_INT(1, el.delta[0].lon);
    ARE_EQ_INT(2, el.delta[0].lat);
    ARE_EQ_INT(3, el.delta[1].lon);
    ARE_EQ_INT(0, el.delta[1].lat);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_loc_delta(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1_rep, buff);
    acp_el_free_loc_delta(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_breakdown_status)
{
    e_ret res;
    acp_el_breakdown_status el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x07, 0x81, 0x82, 0x03, 0x04, 0x02, 0x01, 0x02 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_breakdown_status(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(ACP_EL_PRESENT, el.present);
    ARE_EQ_INT(3, el.source_cnt);
    ARE_EQ_INT(0x01, el.source[0]);
    ARE_EQ_INT(0x02, el.source[1]);
    ARE_EQ_INT(0x03, el.source[2]);
    ARE_EQ_INT(0x04, el.sensor);
    ARE_EQ_INT(0x02, el.data_len);
    ARE_EQ_INT(0x01, el.data[0]);
    ARE_EQ_INT(0x02, el.data[1]);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_breakdown_status(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_breakdown_status(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_breakdown_status_with_addl_flags)
{
    e_ret res;
    acp_el_breakdown_status el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x08, 0xc0, 0xf0, 0x82, 0x81, 0x82, 0x83, 0x04, 0x00 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_mem_set(&el, 0, sizeof(el));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_breakdown_status(buff, &el);
    ARE_EQ_INT(OK, res);

    /* otherwise test case must be changed... */
    ARE_EQ_INT(ACP_EL_PRESENT, el.present);
    ARE_EQ_INT(5, ACP_EL_BREAKDOWN_STATUS_MAX_SOURCE);
    ARE_EQ_INT(5, el.source_cnt);
    ARE_EQ_INT(0x40, el.source[0]);
    ARE_EQ_INT(0x70, el.source[1]);
    ARE_EQ_INT(0x02, el.source[2]);
    ARE_EQ_INT(0x01, el.source[3]);
    ARE_EQ_INT(0x02, el.source[4]);
    /* 0x3 will be skipped */
    ARE_EQ_INT(0x00, el.sensor);
    ARE_EQ_INT(0x00, el.data_len);
    ARE_EQ_INT(NULL, el.data);

    /* test write element */
    {
        /* we include the data field with 0 length */
        u8 el_w[] = { 0x07, 0xc0, 0xf0, 0x82, 0x81, 0x02, 0x00, 0x00};
        e_buff_reset(buff);
        res = acp_el_write_breakdown_status(buff, &el);
        ARE_EQ_INT(OK, res);
        ARE_EQ_BUFF_BIN(el_w, buff);
    }
    acp_el_free_breakdown_status(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_info_type)
{
    e_ret res;
    acp_el_info_type el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x04, 0x32, 0x02, 0x01, 0x02 };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_info_type(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x32, el.type);
    ARE_EQ_INT(0x02, el.data_len);
    ARE_EQ_INT(0x01, el.data[0]);
    ARE_EQ_INT(0x02, el.data[1]);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_info_type(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_info_type(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_tcu_data)
{
    e_ret res;
    acp_el_tcu_data el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x10,
                 0x00, 0x01, 0x03, 0x01, 0x02, 0x03,
                 0xAB, 0xCD, 0x01, 0x84,
                 0xFF, 0x00, 0x03, 0xFF, 0xFE, 0xFD
            };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_tcu_data(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(3, el.items_cnt);
    ARE_EQ_INT(0x0001, el.items[0].type);
    ARE_EQ_INT(3, el.items[0].data_len);
    ARE_EQ_INT(0x01, el.items[0].data[0]);
    ARE_EQ_INT(0x02, el.items[0].data[1]);
    ARE_EQ_INT(0x03, el.items[0].data[2]);

    ARE_EQ_INT(0xABCD, el.items[1].type);
    ARE_EQ_INT(1, el.items[1].data_len);
    ARE_EQ_INT(0x84, el.items[1].data[0]);

    ARE_EQ_INT(0xFF00, el.items[2].type);
    ARE_EQ_INT(3, el.items[2].data_len);
    ARE_EQ_INT(0xFF, el.items[2].data[0]);
    ARE_EQ_INT(0xFE, el.items[2].data[1]);
    ARE_EQ_INT(0xFD, el.items[2].data[2]);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_tcu_data(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_tcu_data(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_tcu_data_error)
{
    e_ret res;
    acp_el_tcu_data_error el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = { 0x16,
                 0x00, 0x01, 0x03, 0x01, 0x02, 0x03, 0x01, 0x00,
                 0xAB, 0xCD, 0x01, 0x84, 0x01, 0x01,
                 0xFF, 0x00, 0x03, 0xFF, 0xFE, 0xFD, 0x01, 0xFF
            };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_tcu_data_error(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(3, el.items_cnt);
    ARE_EQ_INT(0x0001, el.items[0].type);
    ARE_EQ_INT(3, el.items[0].data_len);
    ARE_EQ_INT(0x01, el.items[0].data[0]);
    ARE_EQ_INT(0x02, el.items[0].data[1]);
    ARE_EQ_INT(0x03, el.items[0].data[2]);
    ARE_EQ_INT(0x00, el.items[0].error.code);

    ARE_EQ_INT(0xABCD, el.items[1].type);
    ARE_EQ_INT(1, el.items[1].data_len);
    ARE_EQ_INT(0x84, el.items[1].data[0]);
    ARE_EQ_INT(0x01, el.items[1].error.code);

    ARE_EQ_INT(0xFF00, el.items[2].type);
    ARE_EQ_INT(3, el.items[2].data_len);
    ARE_EQ_INT(0xFF, el.items[2].data[0]);
    ARE_EQ_INT(0xFE, el.items[2].data[1]);
    ARE_EQ_INT(0xFD, el.items[2].data[2]);
    ARE_EQ_INT(0xFF, el.items[2].error.code);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_tcu_data_error(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_tcu_data_error(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_apn_cfg)
{
    e_ret res;
    acp_el_apn_cfg el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = {
        0x1A,
            0x4D,
                0x61, 0x70, 0x6E, 0x2E, 0x74, 0x65, 0x6C, 0x63, 0x6F, 0x2E,0x62, 0x61, 0x72,
            0x44,
                0x75, 0x73, 0x65, 0x72,
            0x46,
                0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72
    };
    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_apn_cfg(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_STR("apn.telco.bar", el.address);
    ARE_EQ_STR("user", el.login);
    ARE_EQ_STR("foobar", el.password);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_apn_cfg(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_apn_cfg(&el);
}
END_TEST

START_TEST (test_acp_el_read_write_server_cfg)
{
    e_ret res;
    acp_el_server_cfg el;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 el1[] = {
        0x0D,
            0x0F, 0x00, 0x00, 0x01,
            0x0F, 0x00,
            0x0F, 0x01, 0x02, 0x03,
            0x12, 0x34,
            0xFF
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));

    /* test read */
    e_buff_reset(buff);
    e_buff_write_buff(buff, el1, sizeof(el1));
    res = acp_el_read_server_cfg(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(0x0F000001, el.server_1);
    ARE_EQ_INT(0x0F00, el.port_1);
    ARE_EQ_INT(0x0F010203, el.server_2);
    ARE_EQ_INT(0x1234, el.port_2);
    ARE_EQ_INT(0xFF, el.proto_id);

    /* test write element */
    e_buff_reset(buff);
    res = acp_el_write_server_cfg(buff, &el);
    ARE_EQ_INT(OK, res);
    ARE_EQ_BUFF_BIN(el1, buff);
    acp_el_free_server_cfg(&el);
}
END_TEST

START_TEST (test_acp_el_skip_while_flag)
{
    e_ret res;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 flg1 = 0x1;
    u8 flg2 = 0x2;
    int i;
    u8 ver_el1[10];
    u8 len = sizeof(ver_el1);
    u8 len_flg = sizeof(ver_el1)/2;
    u8 len_nflg = sizeof(ver_el1) - len_flg;

    e_buff_alloc(buff, len * 2);

    e_mem_set(ver_el1, 0, len);
    for (i = 0; i < len_flg; i++) {
        ver_el1[i] = flg1 | (rand() & 0xFF);
    }
    e_buff_write_buff(buff, ver_el1, len);
    res = acp_el_skip_while_flag(buff, flg1);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT((u32) (len_nflg - 1), e_buff_read_remain(buff));

    e_buff_reset(buff);

    /* check with different flag */
    e_mem_set(ver_el1, 0, len);
    for (i = 0; i < len_flg; i++) {
        ver_el1[i] = flg2 | (rand() & 0xFF);
    }
    e_buff_write_buff(buff, ver_el1, len);
    res = acp_el_skip_while_flag(buff, flg2);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT((u32)(len_nflg -1), e_buff_read_remain(buff));

    e_buff_reset(buff);

    /* check with unsufficient data */
    e_mem_set(ver_el1, 0, len);
    for (i = 0; i < len; i++) {
        ver_el1[i] = flg1 | (rand() & 0xFF);
    }
    e_buff_write_buff(buff, ver_el1, len);
    res = acp_el_skip_while_flag(buff, flg1);
    ARE_EQ_INT(ACP_MSG_ERR_INCOMPLETE, res);
    ARE_EQ_INT(0, e_buff_read_remain(buff));

    e_buff_dealloc(buff);
}
END_TEST

extern int acp_el_test (void);
int acp_el_test (void)
{
    acp_init_opts("valid_license.sig");
    return e_check_run_suite("acp_el",
		test_acp_el_read_write_location,
		test_acp_el_read_write_location_not_present,
        test_acp_el_read_write_version_empty,
        test_acp_el_read_write_version_not_present,
        test_acp_el_read_write_version_3_1,
		test_acp_el_read_write_timestamp_3_2,
		test_acp_el_read_write_timestamp_month,
		test_acp_el_read_write_tcu_desc,
		test_acp_el_read_write_tcu_desc_empty,
		test_acp_el_read_write_vehicle_desc,
        test_acp_el_read_write_vehicle_desc_bin_tcu_serial,
		test_acp_el_read_write_error,
		test_acp_el_read_write_ctrl_func,
		test_acp_el_read_write_func_cmd,
        test_acp_el_read_write_loc_delta,
		test_acp_el_read_write_breakdown_status,
		test_acp_el_read_write_breakdown_status_with_addl_flags,
		test_acp_el_read_write_info_type,
		test_acp_el_read_write_tcu_data,
		test_acp_el_read_write_tcu_data_error,
		test_acp_el_read_write_apn_cfg,
		test_acp_el_read_write_server_cfg,
        test_acp_el_skip_while_flag,
        NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
	return acp_el_test();
}
#endif
