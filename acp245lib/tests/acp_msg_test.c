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

#include "acp_msg.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "e_check.h"
#include "e_buff.h"
#include "e_mem.h"
#include "e_util.h"
#include "e_log.h"

#include "acp_init.h"

#define THRASH_TEST_CNT      (10000)
/* big pool size improves performance of thrash test, set to 1MB */
#define RANDPOOL_SZ         (0xFFFF)
static u8 buff_data[512];
static u8 r_buff_data[512];

typedef struct rand_pool {
    u8 pool[RANDPOOL_SZ];
    u32 i;
} rand_pool;

static void _seed(int seed) {
    FILE* f;
    f = fopen("/dev/urandom","r");
    if (f) {
        fwrite(&seed, sizeof(seed), 1, f);
        fclose(f);
    } else {
        srand(seed);
    }
}

static void _fill(rand_pool *p) {
    FILE* f;
    f = fopen("/dev/urandom","r");
    if (f) {
        fread(p->pool, 1, RANDPOOL_SZ, f);
        fclose(f);
        p->i = 0;
    } else {
        int i;
        for (i = 0; i < RANDPOOL_SZ; i++) {
            p->pool[i] = (u8) (0xFF & rand());
        }
    }
}

static u8 _rand(rand_pool *p)
{
    if (RANDPOOL_SZ == p->i)
    {
        p->i = 0;
    }

    return p->pool[p->i++];
}

#include <stdio.h>

/* check that the code does not faults on trash data */
START_TEST (test_acp_msg_thrash)
{
    e_ret res;
    acp_msg msg;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    unsigned int i;
    /* 6 = max header length, 0xFFFF = max body length */
    u8 data[6 + 0xFFFF];
    rand_pool pool;
    _seed(e_check_get_seed());
    _fill(&pool);
    e_buff_wrap(buff, data, sizeof(data));
    for (i = 0; i < THRASH_TEST_CNT; i++) {
        u32 j;
        u32 wrote;
        u32 lim;
        /* between 0 and 65535 + 6 */
        u32 len;
        if (pool.i >= RANDPOOL_SZ) {
            _fill(&pool);
        }
        len = (_rand(&pool) % 0x10006);
        for (j = 0; j < len; j+=2) {
            int r = _rand(&pool);
            data[j] = (u8) (r & 0xFF);
            data[j+1] = (u8) ((r >> 8) & 0xFF);
        }
        /* focus on valid app_ids */
        data[0] = (_rand(&pool) & 0xC0) | (_rand(&pool) % 13);
        /* focus on valid message type range */
        data[1] = (_rand(&pool) & 0xE0) | (_rand(&pool) % 12);

        e_buff_reset(buff);
        e_buff_set_pos(buff, 0);
        e_buff_set_lim(buff, len);
        res = acp_msg_read(buff, &msg);
        if (OK == res) {
            wrote = e_buff_get_pos(buff);

            e_buff_reset(buff);
            /* 1% chance of not enough room in buffer */
            lim = e_buff_capacity(buff) - ((wrote - 1) + (_rand(&pool) % 100));
            if (lim > e_buff_capacity(buff)) {
                lim = e_buff_capacity(buff);
            }
            e_buff_set_lim(buff, lim);
            res = acp_msg_write(buff, &msg);
            if (res == ACP_MSG_ERR_INCOMPLETE) {
                /* not enough data in buffer, try extending buffer */
                e_buff_reset(buff);
                res = acp_msg_write(buff, &msg);
            }
            if (res != OK) {
                E_LOG_SET_LEVEL(ALL);
                e_buff_set_lim(buff, lim);
                res = acp_msg_write(buff, &msg);
                E_LOG_SET_LEVEL(NONE);
            }
            ARE_EQ_INT(OK, res);
        }
        acp_msg_free(&msg);
    }
}
END_TEST

START_TEST (test_acp_msg_read_write_prov_upd)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_prov_upd body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x01,
        0x01,
        0xB0,
        0x01,
        0x4E
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* target app id */
        0x47,
        /* appl flg1 - ctrl flg1 */
        0xAE,
        /* ctrl flg 2 */
        0x00,
        /* start time */
        0x48, 0xDC, 0x3C, 0x30,
        /* end time, 2008, 01, 26, 16:56:23 */
        0x48, 0xDC, 0x3C, 0x38,
        /* tcu descriptor */
        0x09, 0x00, 0x1, 0x55, 0x45, 0x30, 0x31, 0x32, 0x33, 0x34,
        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_PROVISIONING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_PROV_UPD, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);
    ARE_EQ_INT(1, hdr.msg_prio);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.prov_upd;

    ARE_EQ_INT(2008, body.start_time.year);
    ARE_EQ_INT(3, body.start_time.month);
    ARE_EQ_INT(14, body.start_time.day);
    ARE_EQ_INT(3, body.start_time.hour);
    ARE_EQ_INT(48, body.start_time.minute);
    ARE_EQ_INT(48, body.start_time.second);

    ARE_EQ_INT(2008, body.end_time.year);
    ARE_EQ_INT(3, body.end_time.month);
    ARE_EQ_INT(14, body.end_time.day);
    ARE_EQ_INT(3, body.end_time.hour);
    ARE_EQ_INT(48, body.end_time.minute);
    ARE_EQ_INT(56, body.end_time.second);

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(0x55, body.tcu_desc.device_id);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.tcu_desc.version.id);
    ARE_EQ_STR("01234", body.tcu_desc.version.data.str);

    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));

    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_prov_reply)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_prov_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x01,
        0x03,
        0x30,
        0x3D
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* target app id */
        0x47,
        /* appl flg1 - ctrl flg1 */
        0x02,
        /* status flag - tcu resp flag */
        0x90,
        /* error element */
        0x01, 0x03,
        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_PROVISIONING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_PROV_REPLY, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.prov_reply;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));

    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1)); 
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_cfg_upd_245)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_cfg_upd_245 body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x02,
        0x08,
        0x01,
        0x5C  /*92*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                           /* 17 */
        /* TARGET APPLICATIN ID */
        0x47,
        /* appl flg1 - ctrl flg1 */
        0x2E,
        /* control flag 2*/
        0x01,
        /* reserved */
        0x00,                                               

        /* START TIME */
        /* Year - Month */
        0x34,    /* 2009/03/13 15:00:00  */

        /* month - day - hour */
        0xDA,

        /* hour - min */
        0xF0,
        /* min - sec */
        0x00,
        /* END TIME */
        /* year - month*/
        0x34,    /* 2009/03/14 17:00:00*/

        /* month - day - hour*/
        0xDD,

        /* hour - min */
        0x10,
        /* min - sec */
        0x00,


        /* VEHICLE DESCRIPTOR ELEMENT */                                                        /* 59 */
        0x20, 0x39,    /*lenght 57*/
        0xB1, 0x30, 
        /*VIN Number*/
        0x50, /*IE Identifier = 1 - length = 16 */        
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 
        /*TCU Serial number*/
        0x48, /*IE Identifier = 2 - length = 8*/ 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /*BCD IMEI Number*/
        0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 

        /*BCD SIM CARD ID*/
        0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/ 
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 

        /*Binary format (Auth Key)*/
        0x08, /*IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        
        /* TCU DESCRIPTOR */                                                                    /* 12 */
        /* IE Identifier - more flag - lenght */
        0x05,
        /* reserved */
        0x00,
        /* IE Identifier - more flag - lenght */
        0x01,
        /* Device ID */
        0x03,
        /* IE Identifier - more flag - lenght */
        0x01,
        /* version ID */
        0x04,

        /* TCU DATA ELEMENT */
        /* IE identifier - more flag - length */
        0x05,     /* pagina 74, el ultimo*/
        /* Data type MSB */
        0x00,
        /* Data type LSB */
        0x82,
        /* Lenght data type */
        0x02,
        /* Configuration data */
        0x02,
        0xD7
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_CONFIGURATION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_CFG_UPD_245, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.cfg_upd_245;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));

    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_read_write_cfg_reply)
{
 
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_cfg_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x02,
        0x03,
        0x01,
        0x4A        /*74*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   /* 5 */

        /* MESSAGE FIELDS*/                                                             /* 5 */
        0x00,    
        /*More flag - Target Application ID*/
        0x00,    
        /*ApplFlag1 - ControlFlag1 = 2*/
        0x02,
        /*StatusFlag1 - TCU Response flag - Reserved*/
        0x00,


        /*ERROR ELEMENT*/
        /*ID Identifier - More flag - Length*/
        0x01,
        /*Error code*/
        0x00,

        /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    /*lenght 57*/
        0xB1, 0x30, 
        /*VIN Number*/
        0x50, /*IE Identifier = 1 - length = 16 */        
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 
        /*TCU Serial number*/
        0x48, /*IE Identifier = 2 - length = 8*/ 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /*BCD IMEI Number*/
        0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 
        /*BCD SIM CARD ID*/
        0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/ 
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 

        /*Binary format (Auth Key)*/
        0x08, /*IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
                  };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_CONFIGURATION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.cfg_reply;
    E_DBG("CAR MANUFACTURER ID = \n\n%d\n\n",body.version.car_manufacturer);
    ARE_EQ_INT(0x8, body.version.car_manufacturer);

    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_track_pos)
{

    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_track_pos body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0A,
        0x02,
        0x00,
        0x8F    /*144*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   /* 5 */

        /* TIMESTAMP */
        /* Year - Month */
        0x34,    /* 2009/03/13 15:00:00  */                                             /* 4 */
        /* month - day - hour */
        0xDA,
        /* hour - min */
        0xF0,
        /* min - sec */
        0x00,

        /*LOCATION ELEMENT*/                                                            /* 25 */
        /*IE Identifier - More flag - Length*/
        0x20, 0x3A,  /*LENGTH = 58, more flag = 1; 0x3A = 58*/
        /*CURRENT GPSRawData*/

        /*IE identifier = 0 - More flag - length*/
        0x16, /*LENGTH GPSRawDataElement = 22 */

        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /*LENGTH Area location coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,

        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x03,
        /*Altitude byte 2*/
        0x04,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,
        /*END OF AREA LOCATION CODING*/

        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*PRIOR GPSRawData*/                                                                    /* 35 */
        /*IE identifier = 0 - More flag - length*/
        0x16, /* LENGTH = 22 */
        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /* LENGTH Area Location Coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,
        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x0F,
        /*Altitude byte 2*/
        0xF0,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,

        /*END OF AREA LOCATION CODING*/
        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*Current Dead Reckoning Data*/
        /*IE Identifier = 0 - More flag - Length*/
        0x08,  /*LENGTH = 2*/
        /*Latitude*/
        0x8F,0x8F,0x8F,0x8F,
        /*Longitude*/
        0xF0,0xF0,0xF0,0x70,

        /*Array of Area Location Delta Coding*/
        /*IE Identifier = 0 - More flag - Length*/
        0x02,   /*LENGTH = 2*/
        /*Delta Longitude 1*/
        0x81,
        /*Delta Latitude 1*/
        0x02,
        /*END OF LOCATION ELEMENT*/

        /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    /*lenght 57*/
        0xB1, 0x30,
        /*VIN Number*/
        0x50, /*IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        /*TCU Serial number*/
        0x48, /*IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /*BCD IMEI Number*/
        0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        /*BCD SIM CARD ID*/
        0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        /*Binary format (Auth Key)*/
        0x08, /*IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        /*BREAKDOWN STATUS ELEMENT*/                                                                    /* 11 */
        /*IE Identifier - More flag - Length*/
        0x06,
        /*More flag = 1 - Breakdown Source (First flag)*/
        0x81,
        /*More flag = 1 - Breakdown Source (Second  flag)*/
        0x81,
        /*More flag - Breakdown Source (Third flag)*/
        0x01,
        /*More flag - Breakdown Sensor*/
        0x01,
        /*IE Identifier - More flag - Length*/
        0x01,
        /*Breakdown Data*/
        0x30,


        /*INFORMATION TYPE ELEMENT*/
        /*IE Identifier = 0 - More flag - Length */
        0x03,

        /*Add Flag - Information Type*/
        0x00,
        /*IE Identifier - More flag - Length */
        0x01,
        /*Raw Data*/
        0x30
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_POS, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & (ACP_HDR_MSG_CTRL_16BIT_LEN | ACP_HDR_MSG_CTRL_RESP_EXP | ACP_HDR_MSG_CTRL_DONT_USE_TLV));

    body = msg.data.track_pos;
    E_DBG("CAR MANUFACTURER ID = \n\n%d\n\n",body.version.car_manufacturer);
    ARE_EQ_INT(0x8, body.version.car_manufacturer);

    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));
    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_read_write_cfg_reply_245)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_cfg_reply_245 body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
    0x02,
    0x09,
    0x01,
    0x4E    /*78*/
    };

    u8 body_data1[] = {
    /* version element */
    0x04, 0x08, 0x83, 0x01, 0x03,                                                   /* 5 */
           
    /*MESSAGE FIELDS*/                                                              /* 3 */
    /*More flags*/
    0x00,
    /*Applflag1 - Control flag = 2*/
    0x02,
    /*Status flag1 - TCU Response Flag - Reserved*/
    0x00,

    /*TCU DATA ERROR ELEMENT*/                                                      /* 7 */
    /*IE Identifier - More Flag - Length*/
    0x06,
    /*Data Type MSB*/
    0x00,
    /*Data Type LSB*/
    0x11,
    /*Length Data Type*/
    0x01,
    /*Configuration Data*/
    0x00,
    /*Error Element*/
    /*IE Identifier = 0 - More flag - Length*/
    0x01,
    /*Error code*/
    0x00,


    /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
    0x20, 0x39,    /*lenght 57*/
    0xB1, 0x30,
    /*VIN Number*/
    0x50, /*IE Identifier = 1 - length = 16 */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
    /*TCU Serial number*/
    0x48, /*IE Identifier = 2 - length = 8*/
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    /*BCD IMEI Number*/
    0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
    0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
    /*BCD SIM CARD ID*/
    0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/
    0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
    /*Binary format (Auth Key)*/
    0x08, /*IE = 0 - length = 8*/
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_CONFIGURATION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_CFG_REPLY_245, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);
    body = msg.data.cfg_reply_245;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel); ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_read_write_cfg_activation)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_cfg_activation body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x02,
        0x0A,
        0x00,
        0x5D
    };
    u8 body_data1[] = {
        /* apn_cfg */
        0x1A,
            0x4D,
                0x61, 0x70, 0x6E, 0x2E, 0x74, 0x65, 0x6C, 0x63, 0x6F, 0x2E,0x62, 0x61, 0x72,
            0x44,
                0x75, 0x73, 0x65, 0x72,
            0x46,
                0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72,
        /* server_cfg */
        0x0D,
            0x0F, 0x00, 0x00, 0x01,
            0x0F, 0x00,
            0x0F, 0x01, 0x02, 0x03,
            0x12, 0x34,
            0xAB,
        /* ctrl_byte */
        0xFF,

        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));
    ARE_EQ_INT(0x5D, sizeof(hdr_data1) + sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_CONFIGURATION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_CFG_ACT_245, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x0, hdr.msg_ctrl);

    body = msg.data.cfg_activation;

    ARE_EQ_INT(ACP_EL_PRESENT, body.apn_cfg.present);
    ARE_EQ_STR("apn.telco.bar", body.apn_cfg.address);
    ARE_EQ_STR("user", body.apn_cfg.login);
    ARE_EQ_STR("foobar", body.apn_cfg.password);

    ARE_EQ_INT(ACP_EL_PRESENT, body.server_cfg.present);
    ARE_EQ_INT(0x0F000001, body.server_cfg.server_1);
    ARE_EQ_INT(0x0F00, body.server_cfg.port_1);
    ARE_EQ_INT(0x0F010203, body.server_cfg.server_2);
    ARE_EQ_INT(0x1234, body.server_cfg.port_2);
    ARE_EQ_INT(0xAB, body.server_cfg.proto_id);

    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);


    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

/* example from 1.31.1 */
START_TEST (test_acp_msg_read_write_func_cmd_1_31_1)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_cmd body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x02,
        0x31,
        0x3F
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* control function */
        0x03, 0x0A, 0x00, 0x00,
        /* function command */
        0x02, 0x02, 0x00,
        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_CMD, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_cmd;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(ACP_ENT_ID_IMMOBILIZE, body.ctrl_func.entity_id);
    IS_TRUE(body.ctrl_func.transmit_present);
    ARE_EQ_INT(ACP_EL_TIME_UNIT_SECOND, body.ctrl_func.transmit_unit);
    ARE_EQ_INT(0, body.ctrl_func.transmit_interval);
    ARE_EQ_INT(ACP_FUNC_CMD_ENABLE, body.func_cmd.cmd);
    ARE_EQ_INT(ACP_EL_EMPTY, body.func_cmd.raw_data.present);
    ARE_EQ_INT(0, body.func_cmd.raw_data.data_len);
    IS_NULL(body.func_cmd.raw_data.data);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);


    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);


}
END_TEST

START_TEST (test_acp_msg_read_write_func_cmd_no_transmit_interval)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_cmd body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x02,
        0x31,
        0x3D
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* control function */
        0x01, 0x0A,
        /* function command */
        0x02, 0x02, 0x00,
        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_CMD, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_cmd;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(ACP_ENT_ID_IMMOBILIZE, body.ctrl_func.entity_id);
    IS_FALSE(body.ctrl_func.transmit_present);
    ARE_EQ_INT(ACP_FUNC_CMD_ENABLE, body.func_cmd.cmd);
    ARE_EQ_INT(ACP_EL_EMPTY, body.func_cmd.raw_data.present);
    ARE_EQ_INT(0, body.func_cmd.raw_data.data_len);
    IS_NULL(body.func_cmd.raw_data.data);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);


    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);


}
END_TEST

/* example from VW spec, 13.5 */
START_TEST (test_acp_msg_read_write_func_cmd_vw_13_5)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_cmd body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x02,
        0x11,
        0x1B
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x01,
        /* control function */
        0x01, 0x80,
        /* function command */
        0x01, 0x03,
        /* vehicle descriptor */
        0x0D, 0x80, 0x20, 0x8A, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);

    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_CMD, hdr.type);
    ARE_EQ_INT(1, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_cmd;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(1, body.version.major_soft_rel);

    ARE_EQ_INT(128, body.ctrl_func.entity_id);
    ARE_EQ_INT(ACP_FUNC_CMD_DISABLE, body.func_cmd.cmd);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.func_cmd.raw_data.present);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    ARE_EQ_STR("12345678901234567890", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

/* example from 1.31.2 */
START_TEST (test_acp_msg_read_write_func_status_1_31_2)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_status body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x03,
        0x31,
        0x42
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* control function */
        0x03, 0x0A, 0x00, 0x00,
        /* function command */
        0x03, 0x02, 0x01, 0x20,
        /* error element */
        0x01, 0x00,
        /* vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_STATUS, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_status;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(ACP_ENT_ID_IMMOBILIZE, body.ctrl_func.entity_id);
    IS_TRUE(body.ctrl_func.transmit_present);
    ARE_EQ_INT(ACP_EL_TIME_UNIT_SECOND, body.ctrl_func.transmit_unit);
    ARE_EQ_INT(0, body.ctrl_func.transmit_interval);
    ARE_EQ_INT(ACP_FUNC_STATE_ENABLED, body.func_status.cmd);
    ARE_EQ_INT(ACP_EL_PRESENT, body.func_status.raw_data.present);
    ARE_EQ_INT(0x01, body.func_status.raw_data.data_len);
    ARE_EQ_INT(0x20, body.func_status.raw_data.data[0]);

    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    ARE_EQ_INT(ACP_ERR_OK, body.error.code);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

/* example from VW spec, 13.6 */
START_TEST (test_acp_msg_read_write_func_status_vw_13_6)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_status body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x03,
        0x10,
        0x1D
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x01,
        /* control function */
        0x01, 0x80,
        /* function command */
        0x01, 0x02,
        /* error element */
        0x01, 0x00,
        /* vehicle descriptor */
        0x0D, 0x80, 0x20, 0x8A, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_STATUS, hdr.type);
    ARE_EQ_INT(1, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_status;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(1, body.version.major_soft_rel);

    ARE_EQ_INT(128, body.ctrl_func.entity_id);
    ARE_EQ_INT(ACP_FUNC_STATE_ENABLED, body.func_status.cmd);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.func_status.raw_data.present);

    ARE_EQ_INT(0, body.error.code);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    ARE_EQ_STR("12345678901234567890", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

/* example from VW spec, 13.7 */
START_TEST (test_acp_msg_read_write_func_status_vw_13_7)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_func_status body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x06,
        0x03,
        0x10,
        0x1D
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x01,
        /* control function */
        0x01, 0x80,
        /* function command */
        0x01, 0x01,
        /* error element */
        0x01, 0x03,
        /* vehicle descriptor */
        0x0D, 0x80, 0x20, 0x8A, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_FUNC_STATUS, hdr.type);
    ARE_EQ_INT(1, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.func_status;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(1, body.version.major_soft_rel);

    ARE_EQ_INT(128, body.ctrl_func.entity_id);
    ARE_EQ_INT(ACP_FUNC_STATE_REJECTED, body.func_status.cmd);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.func_status.raw_data.present);

    ARE_EQ_INT(3, body.error.code);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    ARE_EQ_STR("12345678901234567890", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_read_write_track_cmd)
{

    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_track_cmd body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0A,                                                           /*0A*/    
        0x01,                                                           /*01*/
        0x00,                                                           /*02*/                
        0x4C   /*76*/                                                   /*4C*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* control function */
        0x03, 0x01, 0x00, 0x00,
        /* function command */
        0x03, 0x02, 0x01, 0x20,
      

   /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
   0x20, 0x39,    /*lenght 57*/
   0xB1, 0x30,
   /*VIN Number*/
   0x50, /*IE Identifier = 1 - length = 16 */
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
   /*TCU Serial number*/
   0x48, /*IE Identifier = 2 - length = 8*/
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   /*BCD IMEI Number*/
   0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
   0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
   /*BCD SIM CARD ID*/
   0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/
   0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
   /*Binary format (Auth Key)*/
   0x08, /*IE = 0 - length = 8*/
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_CMD, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_16BIT_LEN);

    body = msg.data.track_cmd;
    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(ACP_ENT_ID_VEHICLE_TRACK, body.ctrl_func.entity_id);
    IS_TRUE(body.ctrl_func.transmit_present);
    ARE_EQ_INT(ACP_EL_TIME_UNIT_SECOND, body.ctrl_func.transmit_unit);
    ARE_EQ_INT(0, body.ctrl_func.transmit_interval);

    ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);


    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

/* example from 1.32.1 */
START_TEST (test_acp_msg_read_write_track_pos_1_32_1)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_track_pos body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    int i;
    u8 hdr_data1[] = {
        0x0A,
        0x02,
        0x31,
        0x63        /*99*/
    };
    u8 body_data1[] = {
        /* version element */                                                                       /* 37 */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /* timestamp */
        0x88, 0x42, 0x00, 0x00,
        /* location */
        /*24    16 */
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
            0x00, 0x00,
            0x00,
            0x00,
            0x00,
            0x00,
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
        0x00,

        /* vehicle descriptor */                                                        /* 47 */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        /* BREAKDOWN STATUS ELEMENT */                                                          /* 11 */
        /* IE Identifier - More flag - Length */
        0x06,
        /* More flag = 1 - Breakdown source (first flag)*/
        0x80,
        /* More flag = 1 - Breakdown source (second flag)*/
        0x80,
        /* More flag - Breakdown source (third flag)*/
        0x00,
        /* More flag - Breakdon sensor*/
        0x00,
        /* IE identifier - More flag - Length*/
        0x01,
        /* Breakdown data*/
        0x00,

        /* INFORMATION TYPE ELEMENT */
        /* IE Identifier - More flag - Length*/
        0x03,
        /* Add flag - Information type*/
        0x00,
        /* IE Identifier - More flag - Length*/
        0x01,
        /* Raw data*/
        0x00

    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(95, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_POS, hdr.type);
    ARE_EQ_INT(3, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.track_pos;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_INT(ACP_MORE_FLG, body.location.curr_gps.flg1);
    ARE_EQ_INT(0x45, body.location.curr_gps.flg2);
    ARE_EQ_INT(ACP_LOCATION_SOUTH_WEST, body.location.curr_gps.flg2 & ACP_LOCATION_FLG2_HEAD_MASK);
    ARE_EQ_INT(0, body.location.curr_gps.area_type);
    ARE_EQ_INT(0, body.location.curr_gps.location_type);
    ARE_EQ_INT(-46553300, body.location.curr_gps.lon);
    ARE_EQ_INT(-23488500, body.location.curr_gps.lat);
    ARE_EQ_INT(0x0, body.location.curr_gps.alt);
    ARE_EQ_INT(0x0, body.location.curr_gps.pos_uncert);
    ARE_EQ_INT(0x0, body.location.curr_gps.head_uncert);
    ARE_EQ_INT(0x0, body.location.curr_gps.heading);
    ARE_EQ_INT(ACP_LOCATION_DIST_UNIT_ND, body.location.curr_gps.dist_unit);
    ARE_EQ_INT(ACP_LOCATION_TIME_UNIT_SECONDS, body.location.curr_gps.time_unit);

    /* satellite data */
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 0) {
        ARE_EQ_INT(0x07, body.location.curr_gps.satellites[0]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 1) {
        ARE_EQ_INT(0x05, body.location.curr_gps.satellites[1]);
    }
    if (ACP_EL_GPS_RAW_DATA_SAT_MAX > 2) {
        ARE_EQ_INT(0x03, body.location.curr_gps.satellites[2]);
    }
    for (i = 3; i < ACP_EL_GPS_RAW_DATA_SAT_MAX; i++) {
        ARE_EQ_INT(0x00, body.location.curr_gps.satellites[i]);
    }

    ARE_EQ_INT(0, body.location.prev_gps.area_type);
    ARE_EQ_INT(0, body.location.prev_gps.location_type);
    ARE_EQ_INT(0, body.location.prev_gps.lon);
    ARE_EQ_INT(0, body.location.prev_gps.lat);
    ARE_EQ_INT(0x0, body.location.prev_gps.alt);
    ARE_EQ_INT(0x0, body.location.prev_gps.pos_uncert);
    ARE_EQ_INT(0x0, body.location.prev_gps.head_uncert);
    ARE_EQ_INT(0x0, body.location.prev_gps.heading);
    ARE_EQ_INT(0, body.location.prev_gps.dist_unit);
    ARE_EQ_INT(0, body.location.prev_gps.time_unit);

    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);

    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

/* example from VW spec, 13.8 */
START_TEST (test_acp_msg_read_write_track_pos_vw_13_8)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_track_pos body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    int i;
    u8 hdr_data1[] = {
        0x0A,
        0x02,
        0x11,
        0x31
    };
    u8 body_data1[] = {
        /* version element */                                                                       /* 37 */
        0x04, 0x08, 0x83, 0x01, 0x01,
        /* timestamp */
        0x4C, 0xE8, 0x03, 0x1A,
        /* location */
        /*24    16 */
        0x14,
        /* current gps raw data */
        0x13,
            /* area location code */
            0x11,
            0x00, /* VW example is wrong, FULL MESSAGE indicate this as 0x80,
                     but table says 0x00. 0x00 is better according to the rest of the VW spec. */
            0x00, 0x00,
            /* lon */
            0xFD, 0x39, 0xA7, 0x2C,
            /* lat */
            0xFE, 0x99, 0x98, 0x0C,
            0x02, 0xF9, 0x03, 0x00, 0x06, 0x6E,
            /* satellite data truncated */
            0x30,

        /* vehicle descriptor */
        0x0D, 0x80, 0x20, 0x8A, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90,

        /* breakdown status */
        0x00
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(45, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_POS, hdr.type);
    ARE_EQ_INT(1, hdr.version); /* VW example says 3, example is wrong! */
    ARE_EQ_INT(1, hdr.msg_ctrl);
    ARE_EQ_INT(0, hdr.msg_prio);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.track_pos;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer); /* VW example says 0x82, example is wrong! */
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(1, body.version.major_soft_rel);

    ARE_EQ_INT(2009, body.timestamp.year);
    ARE_EQ_INT(3, body.timestamp.month);
    ARE_EQ_INT(20, body.timestamp.day);
    ARE_EQ_INT(0, body.timestamp.hour);
    ARE_EQ_INT(12, body.timestamp.minute);
    ARE_EQ_INT(26, body.timestamp.second);

    ARE_EQ_INT(0, body.location.curr_gps.flg1);
    ARE_EQ_INT(0, body.location.curr_gps.flg2);
    ARE_EQ_INT(0, body.location.curr_gps.area_type);
    ARE_EQ_INT(0, body.location.curr_gps.location_type);
    ARE_EQ_INT(-46553300, body.location.curr_gps.lon);
    ARE_EQ_INT(-23488500, body.location.curr_gps.lat);
    ARE_EQ_INT(761, body.location.curr_gps.alt);
    ARE_EQ_INT(0x1, body.location.curr_gps.pos_uncert);
    ARE_EQ_INT(0x0, body.location.curr_gps.head_uncert);
    ARE_EQ_INT(0x0, body.location.curr_gps.heading);
    ARE_EQ_INT(ACP_LOCATION_DIST_UNIT_KM, body.location.curr_gps.dist_unit);
    ARE_EQ_INT(ACP_LOCATION_TIME_UNIT_HOURS, body.location.curr_gps.time_unit);
    ARE_EQ_INT(110, body.location.curr_gps.velocity);
    ARE_EQ_INT(3, body.location.curr_gps.satellites_avail);
    ARE_EQ_INT(0, body.location.curr_gps.satellites_cnt);

    /* satellite data */
    for (i = 0; i < ACP_EL_GPS_RAW_DATA_SAT_MAX; i++) {
        ARE_EQ_INT(0, body.location.curr_gps.satellites[i]);
    }

    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.location.prev_gps.present);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.location.dead_reck.present);
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.location.loc_delta.present);

    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    ARE_EQ_STR("12345678901234567890", body.vehicle_desc.iccid);

    ARE_EQ_INT(ACP_EL_EMPTY, body.breakdown_status.present);
    ARE_EQ_INT(0, body.breakdown_status.source_cnt);
    ARE_EQ_INT(0, body.breakdown_status.sensor);
    ARE_EQ_INT(0, body.breakdown_status.data_len);
    IS_NULL(body.breakdown_status.data);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));
    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));
    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_track_reply)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_track_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0A,
        0x03,
        0x01,
        0x0D    /*13*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /*MESSAGE FIELDS*/
        /*Confirmation - Transmit units*/
        0x40, 
        /*E-call control flag2*/
        0x00,
        /*ERROR ELEMENT*/
        /*IE Identifier - More flag - Length*/
        0x01,
        /*Error code*/
        0x00
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(9, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_TRACK_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.track_reply;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

 
    acp_msg_free(&msg); 
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_notif)
{
  
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_notif body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x01,
        0x01,
        0x8F    /*143*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
        /**/
  
        /* TIMESTAMP */
        /* Year - Month */
        0x34,    /* 2009/03/13 15:00:00  */                                             /* 4 */
        /* month - day - hour */
        0xDA,
        /* hour - min */
        0xF0,
        /* min - sec */
        0x00,

        /*LOCATION ELEMENT*/                                                            /* 25 */
        /*IE Identifier - More flag - Length*/
        0x20, 0x3A,  /*LENGTH = 58, more flag = 1; 0x3A = 58*/
        /*CURRENT GPSRawData*/

        /*IE identifier = 0 - More flag - length*/
        0x16, /*LENGTH GPSRawDataElement = 22 */

        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /*LENGTH Area location coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,
        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x03,
        /*Altitude byte 2*/
        0x04,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,
        /*END OF AREA LOCATION CODING*/

        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*PRIOR GPSRawData*/                                                                    /* 35 */
        /*IE identifier = 0 - More flag - length*/
        0x16, /* LENGTH = 22 */
        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /* LENGTH Area Location Coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,
        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x0F,
        /*Altitude byte 2*/
        0xF0,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,

        /*END OF AREA LOCATION CODING*/
        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*Current Dead Reckoning Data*/
        /*IE Identifier = 0 - More flag - Length*/
        0x08,  /*LENGTH = 2*/
        /*Latitude*/
        0x8F,0x8F,0x8F,0x8F,
        /*Longitude*/
        0xF0,0xF0,0xF0,0x70,

        /*Array of Area Location Delta Coding*/
        /*IE Identifier = 0 - More flag - Length*/
        0x02,   /*LENGTH = 2*/
        /*Delta Longitude 1*/
        0x81,
        /*Delta Latitude 1*/
        0x02,
        /*END OF LOCATION ELEMENT*/

        /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    /*lenght 57*/
        0xB1, 0x30,
        /*VIN Number*/
        0x50, /*IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        /*TCU Serial number*/
        0x48, /*IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /*BCD IMEI Number*/
        0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        /*BCD SIM CARD ID*/
        0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        /*Binary format (Auth Key)*/
        0x08, /*IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        /*BREAKDOWN STATUS ELEMENT*/                                                                    /* 11 */
        /*IE Identifier - More flag - Length*/
        0x06,
        /*More flag = 1 - Breakdown Source (First flag)*/
        0x81,
        /*More flag = 1 - Breakdown Source (Second  flag)*/
        0x81,
        /*More flag - Breakdown Source (Third flag)*/
        0x01,
        /*More flag - Breakdown Sensor*/
        0x01,
        /*IE Identifier - More flag - Length*/
        0x01,
        /*Breakdown Data*/
        0x30,


        /*INFORMATION TYPE ELEMENT*/
        /*IE Identifier = 0 - More flag - Length */
        0x03,

        /*Add Flag - Information Type*/
        0x00,
        /*IE Identifier - More flag - Length */
        0x01,
        /*Raw Data*/
        0x30
       
   };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(139, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT( ACP_APP_ID_ALARM , hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_NOTIF, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_notif;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_reply)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x02,
        0x01,
        0x0D    /*13*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,
       /*MESSAGE FIELDS*/
        /*Confirmation - Transmit units*/
        0x40, 
        /*E-call control flag2*/
        0x00,
        /*ERROR ELEMENT*/
        /*IE Identifier - More flag - Length*/
        0x01,
        /*Error code*/
        0x00
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(9, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_reply;

    ARE_EQ_INT(0x8, body.version.car_manufacturer);
    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg); 
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_pos)
{

    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_pos body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x03,
        0x00,
        0x8F    /*144*/
    };
    u8 body_data1[] = {
        /* version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   /* 5 */

        /* TIMESTAMP */
        /* Year - Month */
        0x34,    /* 2009/03/13 15:00:00  */                                             /* 4 */
        /* month - day - hour */
        0xDA,
        /* hour - min */
        0xF0,
        /* min - sec */
        0x00,

        /*LOCATION ELEMENT*/                                                            /* 25 */
        /*IE Identifier - More flag - Length*/
        0x20, 0x3A,  /*LENGTH = 58, more flag = 1; 0x3A = 58*/
        /*CURRENT GPSRawData*/

        /*IE identifier = 0 - More flag - length*/
        0x16, /*LENGTH GPSRawDataElement = 22 */

        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /*LENGTH Area location coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,
        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x03,
        /*Altitude byte 2*/
        0x04,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,
        /*END OF AREA LOCATION CODING*/

        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*PRIOR GPSRawData*/                                                                    /* 35 */
        /*IE identifier = 0 - More flag - length*/
        0x16, /* LENGTH = 22 */
        /*AREA LOCATION CODING*/
        /*IE Identifier = 0 - More flag - Length*/
        0x12, /* LENGTH Area Location Coding = 18*/
        /*More flag - Area Location Status Flag 1*/
        0xA0,
        /*More flag - Area Location Status Flag 2*/
        0x00,
        /*Area type - Location type - reserved*/
        0x00,
        /*More flag - Time difference*/
        0x00,
        /*Longitude byte 1*/
        0x00,
        /*Longitude byte 2*/
        0x01,
        /*Longitude byte 3*/
        0x02,
        /*Longitude byte 4*/
        0x04,
        /*Latitude byte 1*/
        0x10,
        /*Latitude byte 2*/
        0x11,
        /*Latitude byte 3*/
        0x12,
        /*Latitude byte 4*/
        0x13,
        /*Altitude byte 1*/
        0x0F,
        /*Altitude byte 2*/
        0xF0,
        /*  Position uncertany estimate - K/HDOP*/
        0x10,
        /*Heading Uncertainty Estimate - Heading*/
        0x31, /*15 grados*/
        /*Reserved - Distance flag - Time flag*/
        0x00, /*units not definded - seconds*/
        /*Velocity*/
        0x00,

        /*END OF AREA LOCATION CODING*/
        /*Number of satelites - reserved*/
        0x20, /* 2 satelites*/
        /*satelite ID 1 */
        0xFF,
        /*satelite ID 2 */
        0xFE,

        /*Current Dead Reckoning Data*/
        /*IE Identifier = 0 - More flag - Length*/
        0x08,  /*LENGTH = 2*/
        /*Latitude*/
        0x8F,0x8F,0x8F,0x8F,
        /*Longitude*/
        0xF0,0xF0,0xF0,0x70,

        /*Array of Area Location Delta Coding*/
        /*IE Identifier = 0 - More flag - Length*/
        0x02,   /*LENGTH = 2*/
        /*Delta Longitude 1*/
        0x81,
        /*Delta Latitude 1*/
        0x02,
        /*END OF LOCATION ELEMENT*/

        /* VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    /*lenght 57*/
        0xB1, 0x30,
        /*VIN Number*/
        0x50, /*IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        /*TCU Serial number*/
        0x48, /*IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        /*BCD IMEI Number*/
        0x88, /*IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        /*BCD SIM CARD ID*/
        0x8A,/*IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        /*Binary format (Auth Key)*/
        0x08, /*IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        /*BREAKDOWN STATUS ELEMENT*/                                                                    /* 11 */
        /*IE Identifier - More flag - Length*/
        0x06,
        /*More flag = 1 - Breakdown Source (First flag)*/
        0x81,
        /*More flag = 1 - Breakdown Source (Second  flag)*/
        0x81,
        /*More flag - Breakdown Source (Third flag)*/
        0x01,
        /*More flag - Breakdown Sensor*/
        0x01,
        /*IE Identifier - More flag - Length*/
        0x01,
        /*Breakdown Data*/
        0x30,


        /*INFORMATION TYPE ELEMENT*/
        /*IE Identifier = 0 - More flag - Length */
        0x03,

        /*Add Flag - Information Type*/
        0x00,
        /*IE Identifier - More flag - Length */
        0x01,
        /*Raw Data*/
        0x30
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    E_DBG("hdr_size=%lu, body_size=%lu.\n",
            (unsigned long) sizeof(hdr_data1),
            (unsigned long) sizeof(body_data1));
    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_POS, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0x0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & (ACP_HDR_MSG_CTRL_16BIT_LEN | ACP_HDR_MSG_CTRL_RESP_EXP | ACP_HDR_MSG_CTRL_DONT_USE_TLV));

    body = msg.data.alarm_pos;
    E_DBG("CAR MANUFACTURER ID = \n\n%d\n\n",body.version.car_manufacturer);
    ARE_EQ_INT(0x8, body.version.car_manufacturer);

    ARE_EQ_INT(0x83, body.version.tcu_manufacturer);
    ARE_EQ_INT(1, body.version.major_hard_rel);
    ARE_EQ_INT(3, body.version.major_soft_rel);

    ARE_EQ_STR("0123456789ABCDEF", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("01234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("2345678901234567", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);


    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));
    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x04,
        0x01,
        0x33    /*51*/
    };
    u8 body_data1[] = {
        /* vehicle descriptor, len=47 */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(47, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka;
    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka_no_vehicle_desc)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x04,
        0x01,
        0x04
    };
    ARE_EQ_INT(4, sizeof(hdr_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka;
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    IS_NULL(body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka_empty_vehicle_desc)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x04,
        0x01,
        0x05,
        0x00
    };
    ARE_EQ_INT(5, sizeof(hdr_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka;
    ARE_EQ_INT(ACP_EL_EMPTY, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    IS_NULL(body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka_reply)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x05,
        0x00,
        0x33    /*51*/
    };
    u8 body_data1[] = {
        /* vehicle descriptor, len=47 */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));
    ARE_EQ_INT(47, sizeof(body_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka_reply;
    ARE_EQ_INT(ACP_EL_PRESENT, body.vehicle_desc.present);
    ARE_EQ_STR("0123456789ABCDEF0", body.vehicle_desc.vin);
    IS_TRUE(body.vehicle_desc.tcu_serial.present);
    ARE_EQ_INT(ACP_IE_ISO_8859_1, body.vehicle_desc.tcu_serial.id);
    ARE_EQ_STR("1234567", body.vehicle_desc.tcu_serial.data.str);
    ARE_EQ_STR("0456000450", body.vehicle_desc.imei);
    ARE_EQ_STR("01234567890123456789", body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1) + sizeof(body_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(body_data1, r_buff_data + sizeof(hdr_data1), sizeof(body_data1));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka_reply_no_vehicle_desc)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x05,
        0x00,
        0x04
    };

    ARE_EQ_INT(4, sizeof(hdr_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka_reply;
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    IS_NULL(body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_alarm_ka_reply_empty_vehicle_desc)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka_reply body;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x05,
        0x00,
        0x05,
        0x00
    };

    ARE_EQ_INT(5, sizeof(hdr_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;

    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka_reply;
    ARE_EQ_INT(ACP_EL_EMPTY, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    IS_NULL(body.vehicle_desc.iccid);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1), e_buff_read_remain(buff)); 
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_track_pos_bad_1)
{
    e_ret res;
    acp_msg msg;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0A,0x02,0x00,0x45
    };
    u8 body_data1[] = {
        0x04,0x08,0x82,0x01,0x02,0x00,0x00,0x00,0x00,0x17,0x13,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,0x00,0x11,0x81,0x20,0x8D,0x89,0x91,0x45,0x00,0x04,0x00,0x77,0x18,0x65,0x64,0x53,0xE8,0xEA,0x85,0x21,0x43,0x65,0x87,0x09,0x05,0xC0,0x80,0x00,0x00,0x00,0x02,0x00,0x00
    };

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));
    e_buff_write_buff(buff, body_data1, sizeof(body_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(0x8004, res);
    acp_msg_free(&msg);
    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_data_write_data)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    acp_msg_alarm_ka_reply body;
    u8 hdr_data_written[4];
    u32 readed;
    u32 written;
    u8 hdr_data1[] = {
        0x0B,
        0x05,
        0x00,
        0x04
    };
    ARE_EQ_INT(4, sizeof(hdr_data1));

    res = acp_msg_read_data(hdr_data1, 4, &readed, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(4, readed);

    hdr = msg.hdr;
    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type);
    ARE_EQ_INT(0, hdr.version);
    ARE_EQ_INT(0, hdr.msg_ctrl);

    IS_FALSE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    body = msg.data.alarm_ka_reply;
    ARE_EQ_INT(ACP_EL_NOT_PRESENT, body.vehicle_desc.present);
    IS_NULL(body.vehicle_desc.vin);
    IS_FALSE(body.vehicle_desc.tcu_serial.present);
    IS_NULL(body.vehicle_desc.tcu_serial.data.str);
    IS_NULL(body.vehicle_desc.imei);
    IS_NULL(body.vehicle_desc.iccid);

    res = acp_msg_write_data(hdr_data_written, 4, &written, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(4, written);
    ARE_EQ_BINC(hdr_data1, hdr_data_written, 4);

    acp_msg_free(&msg);
}
END_TEST

START_TEST (test_acp_msg_read_write_extended_version_field)
{
    e_ret res;
    acp_msg msg;
    acp_hdr hdr;
    e_buff buff_s;
    e_buff *buff = &buff_s;
    u8 hdr_data1[] = {
        0x0B,
        0x04,
        0xF1, /* version_flag = 1, version == 7 */
        0x50, /* extended version = 20 */
        0x05
    };

    ARE_EQ_INT(5, sizeof(hdr_data1));

    e_buff_wrap(buff, buff_data, sizeof(buff_data));
    e_buff_write_buff(buff, hdr_data1, sizeof(hdr_data1));

    res = acp_msg_read(buff, &msg);
    ARE_EQ_INT(OK, res);

    hdr = msg.hdr;
    ARE_EQ_INT(ACP_APP_ID_ALARM, hdr.app_id);
    IS_FALSE(hdr.test);
    ARE_EQ_INT(ACP_MSG_TYPE_ALARM_KA, hdr.type);
    ARE_EQ_INT(20, hdr.version);
    ARE_EQ_INT(1, hdr.msg_ctrl);

    IS_TRUE(hdr.msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP);

    e_buff_reset(buff);
    ARE_EQ_INT(sizeof(buff_data), e_buff_write_remain(buff));

    res = acp_msg_write(buff, &msg);
    ARE_EQ_INT(OK, res);
    ARE_EQ_INT(sizeof(hdr_data1), e_buff_read_remain(buff));
    e_buff_read_buff(buff, r_buff_data, e_buff_read_remain(buff));
    ARE_EQ_BINC(hdr_data1, r_buff_data, sizeof(hdr_data1));

    acp_msg_free(&msg);

}
END_TEST

START_TEST (test_acp_msg_is_tcu_message)
{
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_PROVISIONING,
                ACP_MSG_TYPE_PROV_REPLY));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_REPLY));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_REPLY_245));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_REMOTE_VEHICLE_FUNCTION,
                ACP_MSG_TYPE_FUNC_STATUS));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_VEHICLE_TRACKING,
                ACP_MSG_TYPE_TRACK_POS));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_NOTIF));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_POS));
    IS_TRUE(acp_msg_is_tcu_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_KA));

    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_PROVISIONING,
                ACP_MSG_TYPE_PROV_UPD));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_UPD_245));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_ACT_245));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_REMOTE_VEHICLE_FUNCTION,
                ACP_MSG_TYPE_FUNC_CMD));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_VEHICLE_TRACKING,
                ACP_MSG_TYPE_TRACK_REPLY));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_REPLY));
    IS_FALSE(acp_msg_is_tcu_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_KA_REPLY));
}
END_TEST

START_TEST (test_acp_msg_is_so_message)
{
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_PROVISIONING,
                ACP_MSG_TYPE_PROV_UPD));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_UPD_245));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_ACT_245));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_REMOTE_VEHICLE_FUNCTION,
                ACP_MSG_TYPE_FUNC_CMD));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_VEHICLE_TRACKING,
                ACP_MSG_TYPE_TRACK_REPLY));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_REPLY));
    IS_TRUE(acp_msg_is_so_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_KA_REPLY));

    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_PROVISIONING,
                ACP_MSG_TYPE_PROV_REPLY));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_REPLY));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_CONFIGURATION,
                ACP_MSG_TYPE_CFG_REPLY_245));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_REMOTE_VEHICLE_FUNCTION,
                ACP_MSG_TYPE_FUNC_STATUS));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_VEHICLE_TRACKING,
                ACP_MSG_TYPE_TRACK_POS));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_NOTIF));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_POS));
    IS_FALSE(acp_msg_is_so_message(
                ACP_APP_ID_ALARM,
                ACP_MSG_TYPE_ALARM_KA));
}
END_TEST

extern int acp_msg_test (void);
int acp_msg_test (void)
{
    e_mem_set(buff_data, 0, sizeof(buff_data));
    e_mem_set(r_buff_data, 0, sizeof(r_buff_data));
    return e_check_run_suite("acp_msg",
        test_acp_msg_thrash,
        test_acp_msg_read_write_prov_upd,
        test_acp_msg_read_write_prov_reply,
        test_acp_msg_read_write_cfg_upd_245,
        test_acp_msg_read_write_cfg_reply,
        test_acp_msg_read_write_cfg_reply_245,
        test_acp_msg_read_write_cfg_activation,
        test_acp_msg_read_write_func_cmd_no_transmit_interval,
        test_acp_msg_read_write_func_cmd_1_31_1,
        test_acp_msg_read_write_func_cmd_vw_13_5,
        test_acp_msg_read_write_func_status_1_31_2,
        test_acp_msg_read_write_func_status_vw_13_6,
        test_acp_msg_read_write_func_status_vw_13_7,
        test_acp_msg_read_write_track_cmd,
        test_acp_msg_read_write_track_pos_1_32_1,
        test_acp_msg_read_write_track_pos_vw_13_8,
        test_acp_msg_read_write_track_pos,
        test_acp_msg_read_write_track_pos_bad_1,
        test_acp_msg_read_write_track_reply,
        test_acp_msg_read_write_alarm_notif,
        test_acp_msg_read_write_alarm_reply,
        test_acp_msg_read_write_alarm_pos,
        test_acp_msg_read_write_alarm_ka,
        test_acp_msg_read_write_alarm_ka_no_vehicle_desc,
        test_acp_msg_read_write_alarm_ka_empty_vehicle_desc,
        test_acp_msg_read_write_alarm_ka_reply,
        test_acp_msg_read_write_alarm_ka_reply_no_vehicle_desc,
        test_acp_msg_read_write_alarm_ka_reply_empty_vehicle_desc,
        test_acp_msg_read_data_write_data,
        test_acp_msg_read_write_extended_version_field,
        test_acp_msg_is_tcu_message,
        test_acp_msg_is_so_message,
        NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
    acp_init_opts("valid_license.sig");
	return acp_msg_test();
}
#endif
