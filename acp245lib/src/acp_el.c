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

#include "acp_err.h"
#include "acp_ie.h"
#include "e_buff.h"
#include "e_log.h"
#include "e_mem.h"

#include "acp_el_tcu_data.c"
#include "acp_el_tcu_data_error.c"

#define MIN(x,y)    ((x) < (y) ? (x): (y))
#define MAX(x,y)    ((x) > (y) ? (x): (y))

#define COORD_SIZE  4

/* ACP_IE_LEN(acp_el, len_macro)
 * Returns the length of the IE element that represents the given element.
 * If the element is not present, the IE has length 0, otherwise, the length
 * is at least 1 to represent the IE header.
 * @param acp_el an element that has a 'present' field.
 * @param len_macro a macro to calculate the length of the acp_el element.
 */
#define ACP_IE_LEN(acp_el, len_macro) ((acp_el).present == ACP_EL_NOT_PRESENT ? 0 : acp_ie_get_len(len_macro(&(acp_el))))

static e_ret _read_coord(e_buff *buff, s32 *v) {
    u8 b[4];

    if (e_buff_read_remain(buff) < sizeof(b)) {
        return ACP_MSG_ERR_INCOMPLETE;
    }

    e_buff_read_buff(buff, b, sizeof(b));
    *v =  ((b[0] & 0xFF) << 24);
    *v |= ((b[1] & 0xFF) << 16);
    *v |= ((b[2] & 0xFF) << 8);
    *v |= ((b[3] & 0xFF));

    return OK;
}

static e_ret _write_coord(e_buff *buff, s32 v) {
    u8 b[4];

    b[0] = (((u32)v) >> 24) & 0xFF;
    b[1] = (((u32)v) >> 16) & 0xFF;
    b[2] = (((u32)v) >> 8) & 0xFF;
    b[3] = (((u32)v)) & 0xFF;

    return e_buff_write_buff(buff, b, sizeof(b));
}

static e_ret _read_ie_hdr_and_set_presence(e_buff *buff, acp_ie *ie, acp_el_presence *present) {
    e_ret rc;

    if (e_buff_read_remain(buff) == 0) {
        E_TRACE("...ie not present.");
        *present = ACP_EL_NOT_PRESENT;
        rc = OK;
    } else {
        rc = acp_ie_read_exp(buff, ie, ACP_IE_BINARY);
        if (rc) {
            E_TRACE("...ie failed, rem=%lu err=%x.",
                    (unsigned long) e_buff_read_remain(buff),
                    rc);
        } else if(IE_EXIST(*ie)) {
            E_TRACE("...ie present, len=%lu.", (unsigned long) IE_LEN(*ie));
            *present = ACP_EL_PRESENT;
        } else {
            E_TRACE("...ie empty.");
            *present = ACP_EL_EMPTY;
        }
    }
    return rc;
}

e_ret acp_el_skip_while_flag_sz(e_buff *buff, u8 flg_msk, u32 sz) {
    u8 b;

    e_assert(sz >= 1);

    for (;;) {
        if (e_buff_read(buff, &b)) {
            return ACP_MSG_ERR_INCOMPLETE;
        }
        if (sz > 1) {
            if (e_buff_skip(buff, (sz-1))) {
                return ACP_MSG_ERR_INCOMPLETE;
            }
        }
        E_TRACE("Readed skipped flag byte=%x, skiped %lu more bytes.", b, (unsigned long) sz);
        if (!(b & flg_msk)) {
            return OK;
        }
    } ;
}

e_ret acp_el_skip_while_flag(e_buff *buff, u8 flg_msk) {
    return acp_el_skip_while_flag_sz(buff, flg_msk, 1);
}

/*
 * Message Elements Functions
 */

void acp_el_free_version(acp_el_version *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

e_ret acp_el_read_version(e_buff *buff, acp_el_version *el) {
    acp_ie ie;
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_version enter.");
    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    /* if read fails, version element is truncated, which is OK */
    if(e_buff_read(&ie.data, &el->car_manufacturer)) {
        goto exit;
    }
    if(e_buff_read(&ie.data, &el->tcu_manufacturer)) {
        goto exit;
    }
    if(e_buff_read(&ie.data, &el->major_hard_rel)) {
        goto exit;
    }
    if(e_buff_read(&ie.data, &el->major_soft_rel)) {
        goto exit;
    }

    E_TRACE("...el_version car_m=%lu tcu_m=%lu hv=%lu sv=%lu.", 
            (unsigned long) el->car_manufacturer,
            (unsigned long) el->tcu_manufacturer,
            (unsigned long) el->major_hard_rel,
            (unsigned long) el->major_soft_rel
            );

    rc = OK;

exit:
    E_TRACE("acp_el_read_version exit.");
    return rc;
}

e_ret acp_el_write_version(e_buff *buff, acp_el_version *el) {
    e_ret rc;
    u8 b[4];

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_version enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    b[0] = el->car_manufacturer;
    b[1] = el->tcu_manufacturer;
    b[2] = el->major_hard_rel;
    b[3] = el->major_soft_rel;

    if ((rc = acp_ie_write_bin(buff, b, sizeof(b)))) {
        goto exit;
    }

exit:
    E_TRACE("acp_el_write_version exit.");
    return rc;
}

void acp_el_free_timestamp(acp_el_timestamp *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

e_ret acp_el_read_timestamp(e_buff *buff, acp_el_timestamp *el) {
    e_ret rc = OK;
    u8 b[4];

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_timestamp enter, remain: %lu.", 
            (unsigned long) e_buff_read_remain(buff));

    if (e_buff_read_buff(buff, b, sizeof(b))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    el->year = 1990 + ((b[0] & 0xFC) >> 2);
    el->month = ((b[0] & 0x03) << 2) | ((b[1] & 0xC0) >> 6);
    el->day = (b[1] & 0x3E) >> 1;
    el->hour = ((b[1] & 0x01) << 4) | (b[2] & 0xF0) >> 4;
    el->minute = ((b[2] & 0x0F) << 2) | ((b[3] & 0xC0) >> 6);
    el->second = (b[3] & 0x3F);

    if ((el->month > 12) || (el->day > 31) || (el->hour > 23) || (el->minute > 59) || (el->second > 59)) {
        rc =  ACP_MSG_ERR_BAD_FORMAT;
        goto exit;
    }
    E_TRACE("...timestamp=%lu %lu %lu %lu %lu %lu.", 
            (unsigned long) el->year, 
            (unsigned long) el->month, 
            (unsigned long) el->day, 
            (unsigned long) el->hour, 
            (unsigned long) el->minute, 
            (unsigned long) el->second);

exit:
    E_TRACE("acp_el_read_timestamp exit.");
    return rc;
}

e_ret acp_el_write_timestamp(e_buff *buff, acp_el_timestamp *el) {
    e_ret rc = OK;
    u8 b[4];

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_timestamp enter.");

    if ((el->year != 0 && el->year < 1990) ||
            (el->year > 2053) ||
            (el->month > 12) ||
            (el->day > 31) ||
            (el->hour > 23) ||
            (el->minute > 59) ||
            (el->second > 59)) {
        E_DBG("Invalid date fields:"
                "year=%lu month=%lu day=%lu hour=%lu min=%lu sec=%lu",
            (unsigned long) el->year,
            (unsigned long) el->month,
            (unsigned long) el->day,
            (unsigned long) el->hour,
            (unsigned long) el->minute,
            (unsigned long) el->second
            );
        rc =  ACP_MSG_ERR_BAD_FORMAT;
        goto exit;
    }

    if (el->year == 0) {
        b[0] = ((el->month >> 2) & 0x03);
    } else {
        b[0] = (((el->year - 1990) << 2) & 0xFC)
                | ((el->month >> 2) & 0x03);
    }

    b[1] = ((el->month << 6) & 0xC0)
            | ((el->day << 1) & 0x3E)
            | ((el->hour >> 4) & 0x01);
    b[2] = ((el->hour << 4) & 0xF0)
            | ((el->minute >> 2) & 0x0F);
    b[3] = ((el->minute & 0x03) << 6)
            | ((el->second & 0x3F));

    if (e_buff_write_buff(buff, b, 4)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

exit:
    E_TRACE("acp_el_write_timestamp exit.");
    return rc;
}

void acp_el_free_tcu_desc(acp_el_tcu_desc *el) {
    e_assert( el != NULL );

    acp_ie_free_any(&el->version);

}

e_ret acp_el_read_tcu_desc(e_buff *buff, acp_el_tcu_desc *el) {
    e_ret rc;
    acp_ie ie;
    u8 b;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_tcu_desc enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    /* skip reserved */
    if(e_buff_read(&ie.data, &b)) {
        goto exit;
    }
    E_TRACE("...el_read_tcu_desc reserved=%lu.", 
            (unsigned long) b);

    if ((rc = acp_ie_read_byte(&ie.data, &el->device_id))) {
        goto exit;
    }
    E_TRACE("...el_read_tcu_desc device_id=%lu.",
            (unsigned long) el->device_id);

    if ((rc = acp_ie_read_any(&ie.data, &el->version))) {
        goto exit;
    }

    if (IE_REMAIN(ie)) {
        E_DBG("Ignoring addl version el of tcu_desc: %lu", (unsigned long) IE_REMAIN(ie));
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_tcu_desc exit.");
	return rc;
}

e_ret acp_el_write_tcu_desc(e_buff *buff, acp_el_tcu_desc *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_tcu_desc enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
                 1 + /* reserved */
                 acp_ie_get_len(1) + /* device_id */
                 acp_ie_get_any_len(&el->version)
            ))) {
        goto exit;
    }

    e_buff_write(buff, 0x0);

    if ((rc = acp_ie_write_byte(buff, el->device_id))) {
        goto exit;
    }

    if ((rc = acp_ie_write_any(buff, &el->version))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_tcu_desc exit.");
	return rc;
}

void acp_el_free_vehicle_desc(acp_el_vehicle_desc *el) {
    e_assert( el != NULL );

    e_mem_free(el->vin);
    el->vin = NULL;

    acp_ie_free_any(&el->tcu_serial);

    e_mem_free(el->license_plate);
    el->license_plate = NULL;

    e_mem_free(el->vehicle_color);
    el->vehicle_color = NULL;

    e_mem_free(el->vehicle_model);
    el->vehicle_model = NULL;

    e_mem_free(el->imei);
    el->imei = NULL;

    e_mem_free(el->iccid);
    el->iccid = NULL;

    e_mem_free(el->auth_key);
    el->auth_key = NULL;
    el->auth_key_len = 0;
}

e_ret acp_el_read_vehicle_desc(e_buff *buff, acp_el_vehicle_desc *el){
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_vehicle_desc enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    e_buff_read(&ie.data, &el->flg1);
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG_ADDL_FLG)) {
        E_TRACE("...el_vehicle_desc reading addl_flg");
        e_buff_read(&ie.data, &el->flg2);
    } else {
        el->flg2 = 0;
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG_ADDL_FLG)) {
        E_TRACE("...el_vehicle_desc reading more addl_flgs");
        if ((rc = acp_el_skip_while_flag(&ie.data, ACP_VEHICLE_DESC_FLG_ADDL_FLG))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LANG)) {
        E_TRACE("...el_vehicle_desc reading lang");
        e_buff_read(&ie.data, &el->lang);
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_MODEL_YEAR)) {
        E_TRACE("...el_vehicle_desc reading year");
        e_buff_read(&ie.data, &el->model_year);
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VIN)) {
        E_TRACE("...el_vehicle_desc reading vin");
        if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->vin))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_TCU_SERIAL)) {
        E_TRACE("...el_vehicle_desc reading serial");
        if ((rc = acp_ie_read_any(&ie.data, &el->tcu_serial))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LICENSE_PLATE)) {
        E_TRACE("...el_vehicle_desc reading license plate");
        if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->license_plate))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_COLOR)) {
        E_TRACE("...el_vehicle_desc reading vehicle color");
        if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->vehicle_color))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_MODEL)) {
        E_TRACE("...el_vehicle_desc reading vehicle model");
        if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->vehicle_model))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_IMEI)) {
        E_TRACE("...el_vehicle_desc reading imei");
        if ((rc = acp_ie_read_bcd(&ie.data, &el->imei))) {
            goto exit;
        }
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_SIM_CARD_ID)) {
        E_TRACE("...el_vehicle_desc reading iccid");
        if ((rc = acp_ie_read_bcd(&ie.data, &el->iccid))) {
            goto exit;
        }
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_AUTH_KEY)) {
        u32 len = 0;
        E_TRACE("...el_vehicle_desc reading auth key");
        if ((rc = acp_ie_read_bin(&ie.data, &el->auth_key, &len))) {
            goto exit;
        }
        if (len > 255) {
            rc = ACP_MSG_ERR_BAD_LENGTH;
            goto exit;
        }
        el->auth_key_len = (u8) len;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_vehicle_desc exit.");
	return rc;
}

e_ret acp_el_write_vehicle_desc(e_buff *buff, acp_el_vehicle_desc *el){
    u32 ie_size;
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_vehicle_desc enter, rem=%lu", (unsigned long) e_buff_write_remain(buff));

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    ie_size = 1 /* flg1 */;

    if (el->flg1 & ACP_VEHICLE_DESC_FLG_ADDL_FLG) {
        ie_size++;
        E_TRACE("acp_el_write_vehicle_desc addl ie_size=%lu", (unsigned long) ie_size);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LANG)) {
        ie_size++;
        E_TRACE("acp_el_write_vehicle_desc lang ie_size=%lu", (unsigned long) ie_size);
    }
    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_MODEL_YEAR)) {
        ie_size++;
        E_TRACE("acp_el_write_vehicle_desc year ie_size=%lu", (unsigned long) ie_size);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VIN)) {
        ie_size += acp_ie_get_iso_8859_1_len(el->vin);
        E_TRACE("acp_el_write_vehicle_desc vin ie_size=%lu, %lu",
                (unsigned long) ie_size,
                (unsigned long) (el->vin != NULL ? e_strlen(el->vin) : 0));
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_TCU_SERIAL)) {
        ie_size += acp_ie_get_any_len(&el->tcu_serial);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LICENSE_PLATE)) {
        ie_size += acp_ie_get_iso_8859_1_len(el->license_plate);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_COLOR)) {
        ie_size += acp_ie_get_iso_8859_1_len(el->vehicle_color);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_MODEL)) {
        ie_size += acp_ie_get_iso_8859_1_len(el->vehicle_model);
    }
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_IMEI)) {
        ie_size += acp_ie_get_bcd_len(el->imei);
    }
    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_SIM_CARD_ID)) {
        ie_size += acp_ie_get_bcd_len(el->iccid);
    }
    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_AUTH_KEY)) {
        ie_size += acp_ie_get_len(el->auth_key_len);
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, ie_size))) {
        goto exit;
    }
    E_TRACE("acp_el_write_vehicle_desc ie_size=%lu", (unsigned long) ie_size);

    e_buff_write(buff, el->flg1);
    E_TRACE("...el_vehicle_desc write flg1=%x", el->flg1);
    if ((el->flg1 & ACP_VEHICLE_DESC_FLG_ADDL_FLG)) {
        e_buff_write(buff, el->flg2);
        E_TRACE("...el_vehicle_desc write flg2=%x", el->flg2);
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG_ADDL_FLG)) {
        E_DBG("Skipped additional flags.");
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LANG)) {
        e_buff_write(buff, el->lang);
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_MODEL_YEAR)) {
        e_buff_write(buff, el->model_year);
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VIN)) {
        if ((rc = acp_ie_write_iso_8859_1(buff, el->vin))) {
            goto exit;
        }
        E_TRACE("...el_vehicle_desc write vin");
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_TCU_SERIAL)) {
        if ((rc = acp_ie_write_any(buff, &el->tcu_serial))) {
            goto exit;
        }
        E_TRACE("...el_vehicle_desc write tcu serial");
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_LICENSE_PLATE)) {
        if ((rc = acp_ie_write_iso_8859_1(buff, el->license_plate))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_COLOR)) {
        if ((rc = acp_ie_write_iso_8859_1(buff, el->vehicle_color))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_VEHICLE_MODEL)) {
        if ((rc = acp_ie_write_iso_8859_1(buff, el->vehicle_model))) {
            goto exit;
        }
    }

    if ((el->flg1 & ACP_VEHICLE_DESC_FLG1_IMEI)) {
        if ((rc = acp_ie_write_bcd(buff, el->imei))) {
            goto exit;
        }
        E_TRACE("...el_vehicle_desc write IMEI");
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_SIM_CARD_ID)) {
        if ((rc = acp_ie_write_bcd(buff, el->iccid))) {
            goto exit;
        }
        E_TRACE("...el_vehicle_desc write ICCID");
    }

    if ((el->flg2 & ACP_VEHICLE_DESC_FLG2_AUTH_KEY)) {
        if ((rc = acp_ie_write_bin(buff, el->auth_key, el->auth_key_len))) {
            goto exit;
        }
        E_TRACE("...el_vehicle_desc write auth key");
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_vehicle_desc exit, rem=%lu", (unsigned long) e_buff_write_remain(buff));
	return rc;
}

void acp_el_free_error(acp_el_error *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

e_ret acp_el_read_error(e_buff *buff, acp_el_error *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_error enter.");

    if (!e_buff_read_remain(buff)) {
        E_DBG("... ie not present");
        rc = OK;
        goto exit;
    }

    rc = acp_ie_read(buff, &ie);
    if (rc || !IE_EXIST(ie)) {
        goto exit;
    }

    if (ACP_IE_BINARY == ie.id) {
        e_buff_read(&ie.data, &el->code);
        E_TRACE("...readed error code, byte=0x%x", el->code);
    } else {
        E_DBG("Error message skipped.");
    }

    /* skip the rest of the IE */
    rc = OK;

exit:
    E_TRACE("acp_el_read_error exit.");
	return rc;
}

e_ret acp_el_write_error(e_buff *buff, acp_el_error *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_error enter.");

    rc = acp_ie_write_byte(buff, el->code);

    E_TRACE("acp_el_write_error exit.");
    return rc;
}

u32 acp_el_size_error(/*@unused@*/ acp_el_error *el) {
	UNUSED(el);
    return acp_ie_get_len(1);
}

void acp_el_free_ctrl_func(acp_el_ctrl_func *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

e_ret acp_el_read_ctrl_func(e_buff *buff, acp_el_ctrl_func *el) {
    e_ret rc;
    acp_ie ie;
    u8 b;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_ctrl_func enter.");

    if (!e_buff_read_remain(buff)) {
        E_DBG("... ie not present");
        rc = OK;
        goto exit;
    }

    rc = acp_ie_read(buff, &ie);
    if (rc) {
        goto exit;
    }

    E_TRACE("...el_ctrl_func len=%lu.", (unsigned long) IE_LEN(ie));

    if(e_buff_read(&ie.data, &b)) {
        goto exit;
    }
    el->entity_id = b;
    E_TRACE("...readed entity_id, byte=0x%x", b);

    if(e_buff_read(&ie.data, &b)) {
        el->transmit_present = FALSE;
        goto exit;
    }
    el->transmit_present = TRUE;

    el->transmit_unit = b;
    /* bits 7..4 reserved */
    el->transmit_unit &= 0x0F;
    E_TRACE("...readed transmit unit, byte=0x%x", b);

    if(e_buff_read(&ie.data, &el->transmit_interval)) {
        /* TODO: what does it means if this field is not present?
         * At this time, we leave it on the default value
         * but accept the message */
        goto exit;
    }
    E_TRACE("...readed transmit interval, byte=0x%x", el->transmit_interval);

    rc = OK;

exit:
    E_TRACE("acp_el_read_ctrl_func exit.");
	return rc;
}

e_ret acp_el_write_ctrl_func(e_buff *buff, acp_el_ctrl_func *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_ctrl_func enter.");

    rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 
            el->transmit_present ? 3 : 1);
    if (rc) {
        goto exit;
    }

    e_buff_write(buff, el->entity_id);
    if (el->transmit_present) {
        /* bits 7..4 reserved */
        e_buff_write(buff, el->transmit_unit & 0x0F);
        e_buff_write(buff, el->transmit_interval);
    }
    rc = OK;

exit:
    E_TRACE("acp_el_write_ctrl_func exit.");
	return rc;
}

#define RAW_DATA_SIZE(el) ((el)->present == ACP_EL_PRESENT ? ((el)->data_len) : 0)

void acp_el_free_raw_data(acp_el_raw_data *el) {
    e_assert( el != NULL );

    e_mem_free(el->data);
    el->data = NULL;
    el->data_len = 0;
}

e_ret acp_el_read_raw_data(e_buff *buff, acp_el_raw_data *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_raw_data enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    el->data = e_mem_malloc(IE_REMAIN(ie));
    if (!el->data) {
        rc = ERROR;
        goto exit;
    }
    el->data_len = IE_REMAIN(ie);
    if ((rc = e_buff_read_buff(&ie.data, el->data, el->data_len))) {
        e_mem_free(el->data);
        el->data = NULL;
        el->data_len = 0;
        goto exit;
    }

    E_TRACE("...readed raw data, len=%ld", (long) el->data_len);

    rc = OK;

exit:
    E_TRACE("acp_el_read_raw_data exit.");
	return rc;
}

e_ret acp_el_write_raw_data(e_buff *buff, acp_el_raw_data *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_raw_data enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if((rc = acp_ie_write_bin(buff, el->data, el->data_len))) {
        goto exit;
    }
    rc = OK;
exit:
    E_TRACE("acp_el_write_raw_data exit.");
	return rc;
}

void acp_el_free_func_cmd(acp_el_func_cmd *el) {
    e_assert( el != NULL );

    acp_el_free_raw_data(&el->raw_data);
}

e_ret acp_el_read_func_cmd(e_buff *buff, acp_el_func_cmd *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_func_cmd enter.");

    if ((rc = acp_ie_read_exp(buff, &ie, ACP_IE_BINARY))) {
        goto exit;
    }

    E_TRACE("...el_func_cmd len=%lu.", (unsigned long) IE_LEN(ie));

    if (e_buff_read(&ie.data, &el->cmd)) {
        goto exit;
    }
    E_TRACE("...readed cmd, byte=0x%x", el->cmd);

    if ((rc = acp_el_read_raw_data(&ie.data, &el->raw_data))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_func_cmd exit.");
	return rc;
}

e_ret acp_el_write_func_cmd(e_buff *buff, acp_el_func_cmd *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_func_cmd enter.");

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
                    1 + /* cmd */
                    ACP_IE_LEN(el->raw_data, RAW_DATA_SIZE)))) {
        goto exit;
    }

    if ((rc = e_buff_write(buff, el->cmd))) {
        goto exit;
    }

    if ((rc = acp_el_write_raw_data(buff, &el->raw_data))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_func_cmd exit.");
	return rc;
}

void acp_el_free_loc_delta(acp_el_loc_delta *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

#define LOCATION_DELTA_SIZE(el) ((el)->present == ACP_EL_PRESENT ? ((el)->delta_cnt * 2) : 0)

e_ret acp_el_read_loc_delta(e_buff *buff, acp_el_loc_delta *el) {
    e_ret rc;
    acp_ie ie;
    u8 b;
    u8 i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_loc_delta enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    b = ACP_MORE_FLG;
    for (i = 0
            ; (i < ACP_EL_LOC_DELTA_MAX)
            && (b & ACP_MORE_FLG)
            ; i++) {

        if(e_buff_read(&ie.data, &b)) {
            rc = ACP_MSG_ERR_BAD_LENGTH;
            goto exit;
        }
        el->delta_cnt++;
        el->delta[i].lon = b & 0x7F;

        if (b & ACP_MORE_FLG) {
            if(e_buff_read(&ie.data, &b)) {
                rc = ACP_MSG_ERR_BAD_LENGTH;
                goto exit;
            }
            el->delta[i].lat = b & 0x7F;
        }
    }

    if (b & ACP_MORE_FLG) {
        if (i == ACP_EL_LOC_DELTA_MAX) {
            /* there were more elements but no room to store it, unsupported */
            /* FIXME: Skip the next elements instead of returning unsupported! */
            rc = ACP_MSG_ERR_UNSUPPORTED;
        } else {
            /* last octet had the more flag set, but the IE length didnt allow for
             * an additional element */
            rc = ACP_MSG_ERR_BAD_LENGTH;
        }
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_loc_delta exit.");
    return rc;
}

e_ret acp_el_write_loc_delta(e_buff *buff, acp_el_loc_delta *el) {
    e_ret rc;
    u8 i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_loc_delta enter.");

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, LOCATION_DELTA_SIZE(el)))) {
        goto exit;
    }

    /* OPT: we can avoid sending the last byte if the lat is 0 */
    for (i = 0; i < el->delta_cnt; i++) {
        if((e_buff_write(buff,
                        ACP_MORE_FLG
                        | (el->delta[i].lon & 0x7F)))) {
            rc = ACP_MSG_ERR_INCOMPLETE;
        }
        if((e_buff_write(buff,
                        (i == el->delta_cnt - 1 ? 0 : ACP_MORE_FLG)
                        | (el->delta[i].lat & 0x7F)))) {
            rc = ACP_MSG_ERR_INCOMPLETE;
        }
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_loc_delta exit.");
    return rc;
}

void acp_el_free_dead_reck(acp_el_dead_reck *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

#define DEAD_RECK_SIZE(el) ((el)->present == ACP_EL_PRESENT ? (COORD_SIZE * 2) : 0)

e_ret acp_el_read_dead_reck(e_buff *buff, acp_el_dead_reck *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_dead_reck enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    if ((rc = _read_coord(&ie.data, &el->lat))) {
        goto exit;
    }
    E_TRACE("...readed lat=%ld", (long) el->lat);

    if ((rc = _read_coord(&ie.data, &el->lon))) {
        goto exit;
    }
    E_TRACE("...readed lon=%ld", (long) el->lon);

    rc = OK;

exit:
    E_TRACE("acp_el_read_dead_reck exit.");
    return rc;
}

e_ret acp_el_write_dead_reck(e_buff *buff, acp_el_dead_reck *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_dead_reck enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, DEAD_RECK_SIZE(el)))) {
        goto exit;
    }

    if ((rc = _write_coord(buff, el->lat))) {
        goto exit;
    }

    if ((rc = _write_coord(buff, el->lon))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_dead_reck exit.");
    return rc;
}

void acp_el_free_location_coding(acp_el_gps_raw_data *el) {
    E_UNUSED(el);
    e_assert( el != NULL );
}

#define LOC_CODING_SIZE(el) ((el)->present == ACP_EL_PRESENT ? (\
            1 + /* flg1 */ \
            (((el)->flg1 & ACP_MORE_FLG) || ((el)->flg2 != 0)? 1 :0) + /* flg2 */ \
            1 + /* time area and location type */ \
            1 + /* time diff */ \
            COORD_SIZE * 2 + /* lat lon */ \
            2 + /* alt */ \
            1 + /* uncert */ \
            1 + /* heading */ \
            1 + /* units */ \
            1   /* velocity */) : 0)

e_ret acp_el_read_location_coding(e_buff *buff, acp_el_gps_raw_data *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_location_coding enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    /* fields are optional, depending on element length ... */

    if (IE_REMAIN(ie)) {
        e_buff_read(&ie.data, &el->flg1);

        E_TRACE("...readed flag 1 byte=%x", el->flg1);
    }

    if (IE_REMAIN(ie) && (el->flg1 & ACP_MORE_FLG)) {
        e_buff_read(&ie.data, &el->flg2);

        E_TRACE("...readed flag 2 byte=%x", el->flg2);

        if (el->flg2 & ACP_MORE_FLG) {
            if ((rc = acp_el_skip_while_flag(&ie.data, ACP_MORE_FLG))) {
                goto exit;
            }
        }
    }

    if (IE_REMAIN(ie)) {
        u8 b;

        e_buff_read(&ie.data, &b);
        el->area_type = (b & 0xE0) >> 5;
        el->location_type = (b & 0x1C) >> 2;
        /* bit 0..1 reserved */

        E_TRACE("...readed time area and location type byte=%x", b);
    }

    if (IE_REMAIN(ie)) {
        u8 b;

        e_buff_read(&ie.data, &b);

        el->time_diff = b & 0x7F;
        E_TRACE("...readed time diff byte=%x", b);

        if (b & ACP_MORE_FLG) {
            if ((rc = acp_el_skip_while_flag(&ie.data, ACP_MORE_FLG))) {
                goto exit;
            }
        }
    }

    if (IE_REMAIN(ie)) {
        if ((rc = _read_coord(&ie.data, &el->lon))) {
            goto exit;
        }
        E_TRACE("...readed lon=%ld", (long) el->lon);
    }

    if (IE_REMAIN(ie)) {
        if ((rc = _read_coord(&ie.data, &el->lat))) {
            goto exit;
        }
        E_TRACE("...readed lat=%ld", (long) el->lat);
    }

    if (IE_REMAIN(ie)) {
        if (e_buff_read_u16(&ie.data, &el->alt)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }

        E_TRACE("...readed alt bytes=0x%lx", (unsigned long) el->alt);
    }

    if (IE_REMAIN(ie)) {
        u8 b;

        e_buff_read(&ie.data, &b);
        el->pos_uncert = (b & 0xFE) >> 1;
        el->hdop = (b & 0x1) != 0;

        E_TRACE("...readed uncert byte=0x%x", b);
    }

    if (IE_REMAIN(ie)) {
        u8 b;

        E_TRACE("...reading heading");

        e_buff_read(&ie.data, &b);
        el->head_uncert = (b & 0xE0) >> 5;
        el->heading = (b & 0x1F);

        E_TRACE("...readed heading byte=0x%x", b);
    }

    if (IE_REMAIN(ie)) {
        u8 b;

        E_TRACE("...reading units");

        e_buff_read(&ie.data, &b);
        /* bit 0..4 reserved */
        el->dist_unit = (b & 0x0C) >> 2;
        el->time_unit = (b & 0x03);

        E_TRACE("...readed units byte=0x%x", b);
    }

    if (IE_REMAIN(ie)) {
        E_TRACE("...reading velocity");

        e_buff_read(&ie.data, &el->velocity);

        E_TRACE("...readed velocity byte=0x%x", el->velocity);
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_location_coding exit.");
    return rc;
}

e_ret acp_el_write_location_coding(e_buff *buff, acp_el_gps_raw_data *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_location_coding enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, LOC_CODING_SIZE(el)))) {
        goto exit;
    }
    /* FIXME: to allow including just one flag, we are honoring el->flg1
     * more flag. However, this is an special case since the library normally
     * abstracts more flags away trying to decide by itself if the more flag
     * should be set or not based on the contents of the message...
     */
    if ((el->flg1 & ACP_MORE_FLG) || el->flg2 != 0) {
        e_buff_write(buff, ACP_MORE_FLG | (el->flg1 & 0x7F));
        e_buff_write(buff, ~ACP_MORE_FLG & (el->flg2 & 0x7F));
    } else {
        e_buff_write(buff, /* more flag = 0 */ (el->flg1 & 0x7F));
    }
    e_buff_write(buff, ((el->area_type << 5) & 0xE0)
            | ((el->location_type << 2) & 0x1C));
            /* bit 0..1 reserved */

    e_buff_write(buff, ~ACP_MORE_FLG & (el->time_diff & 0x7F));
    _write_coord(buff, el->lon);
    _write_coord(buff, el->lat);
    e_buff_write(buff, (el->alt >> 8) & 0xFF);
    e_buff_write(buff, (el->alt) & 0xFF);
    e_buff_write(buff, ((el->pos_uncert << 1) & 0xFE) | (el->hdop ? 0x1 : 0x0));

    e_buff_write(buff, ((el->head_uncert << 5) & 0xE0)
            | ((el->heading & 0x1F)));

    e_buff_write(buff, ((el->dist_unit << 2) & 0x0C)
            | ((el->time_unit & 0x03)));
        /* bit 0..4 reserved */

    e_buff_write(buff, el->velocity & 0xFF);

    rc = OK;

exit:
    E_TRACE("acp_el_write_location_coding exit.");
    return rc;
}

#define GPS_RAW_DATA_SIZE(el) ((el)->present == ACP_EL_PRESENT ? (acp_ie_get_len(LOC_CODING_SIZE(el)) + 1 /* sattelite cnt */ + MIN((el)->satellites_cnt,ACP_EL_GPS_RAW_DATA_SAT_MAX)) : 0)

void acp_el_free_gps_raw_data(acp_el_gps_raw_data *el) {
    e_assert( el != NULL );

    acp_el_free_location_coding(el);
}

e_ret acp_el_read_gps_raw_data(e_buff *buff, acp_el_gps_raw_data *el) {
    e_ret rc;
    acp_ie ie;
    u8 b;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_gps_raw_data enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    /* location_coding is merged with gps_raw_data in this impl */
    if ((rc = acp_el_read_location_coding(&ie.data, el))) {
        goto exit;
    }

    if (e_buff_read(&ie.data, &b)) {
        E_DBG("Number of satellites not available, not enough length.");
        goto exit;
    }

    /* bit 4..7 reserved */
    el->satellites_avail = (b&0xF0) >> 4;
    E_TRACE("...readed satellites avail, byte=0x%x, cnt=%lu",
            b, (unsigned long) el->satellites_avail);

    if (E_LOG_IS(DEBUG)) {
        if (IE_REMAIN(ie) < el->satellites_avail) {
            E_DBG("Some satellite IDs are not included, "
                  "satellites_cnt=%lu satellites=%lu.",
                    (unsigned long) el->satellites_avail,
                    (unsigned long) IE_REMAIN(ie));
        }
    }

    for(el->satellites_cnt = 0;
            el->satellites_cnt < MIN(el->satellites_avail,
                                     ACP_EL_GPS_RAW_DATA_SAT_MAX);
            el->satellites_cnt++) {
        if(e_buff_read(&ie.data, &el->satellites[el->satellites_cnt])) {
            /* truncated */
            goto exit;
        }
        E_TRACE("...readed satellite 0x%x", el->satellites[el->satellites_cnt]);
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_gps_raw_data exit.");
    return rc;
}

e_ret acp_el_write_gps_raw_data(e_buff *buff, acp_el_gps_raw_data *el) {
    e_ret rc;
    u8 i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_gps_raw_data enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, GPS_RAW_DATA_SIZE(el)))) {
        goto exit;
    }

    if ((rc = acp_el_write_location_coding(buff, el))) {
        goto exit;
    }

    /* bit 4..7 reserved */
    e_buff_write(buff, (el->satellites_avail << 4) & 0xF0);

    for(i = 0; i < MIN(el->satellites_cnt, ACP_EL_GPS_RAW_DATA_SAT_MAX); i++) {
        e_buff_write(buff, el->satellites[i] & 0xFF);
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_gps_raw_data exit.");
    return rc;
}

void acp_el_free_location(acp_el_location *el) {
    e_assert( el != NULL );

    acp_el_free_gps_raw_data(&el->curr_gps);
    acp_el_free_gps_raw_data(&el->prev_gps);
    acp_el_free_dead_reck(&el->dead_reck);
    acp_el_free_loc_delta(&el->loc_delta);
}

e_ret acp_el_read_location(e_buff *buff, acp_el_location *el){
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_location enter.");

    if (!e_buff_read_remain(buff)) {
        E_DBG("... ie not present");
        rc = OK;
        goto exit;
    }

    rc = acp_ie_read_exp(buff, &ie, ACP_IE_BINARY);
    if (rc) {
        goto exit;
    }
    E_TRACE("...el_location len=%lu.", (unsigned long) IE_LEN(ie));

    if (!IE_REMAIN(ie)) {
        goto exit;
    }
    if ((rc = acp_el_read_gps_raw_data(&ie.data, &el->curr_gps))) {
        goto exit;
    }

    if (!IE_REMAIN(ie)) {
        goto exit;
    }
    if ((rc = acp_el_read_gps_raw_data(&ie.data, &el->prev_gps))) {
        goto exit;
    }

    if (!IE_REMAIN(ie)) {
        goto exit;
    }
    if ((rc = acp_el_read_dead_reck(&ie.data, &el->dead_reck))) {
        goto exit;
    }

    if (!IE_REMAIN(ie)) {
        goto exit;
    }
    if ((rc = acp_el_read_loc_delta(&ie.data, &el->loc_delta))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_location exit.");
	return rc;
}

e_ret acp_el_write_location(e_buff *buff, acp_el_location *el){
    e_ret rc;
    acp_el_presence p;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_location enter.");

    rc = OK;

    p = el->curr_gps.present;
    if (ACP_EL_NOT_PRESENT == p
            && ((p = el->prev_gps.present) != ACP_EL_NOT_PRESENT)) {
        E_DBG("curr_gps not present but prev_gps present");
        rc = ACP_MSG_ERR_BAD_FORMAT;
    }
    if (ACP_EL_NOT_PRESENT == p
            && ((p = el->dead_reck.present) != ACP_EL_NOT_PRESENT)) {
        E_DBG("prev_gps not present but dead_reck present");
        rc = ACP_MSG_ERR_BAD_FORMAT;
    }
    if (ACP_EL_NOT_PRESENT == p
            && ((p = el->loc_delta.present) != ACP_EL_NOT_PRESENT)) {
        E_DBG("dead_reck not present but loc_delta present");
        rc = ACP_MSG_ERR_BAD_FORMAT;
    }

    if (rc) {
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
            ACP_IE_LEN(el->curr_gps, GPS_RAW_DATA_SIZE) +
            ACP_IE_LEN(el->prev_gps, GPS_RAW_DATA_SIZE) +
            ACP_IE_LEN(el->dead_reck, DEAD_RECK_SIZE) +
            ACP_IE_LEN(el->loc_delta, LOCATION_DELTA_SIZE)))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == el->curr_gps.present
            || (rc = acp_el_write_gps_raw_data(buff, &el->curr_gps))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == el->prev_gps.present
            || (rc = acp_el_write_gps_raw_data(buff, &el->prev_gps))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == el->dead_reck.present
            || (rc = acp_el_write_dead_reck(buff, &el->dead_reck))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == el->loc_delta.present
            || (rc = acp_el_write_loc_delta(buff, &el->loc_delta))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_location exit.");
	return rc;
}

void acp_el_free_breakdown_status(acp_el_breakdown_status *el) {
    e_assert( el != NULL );

    e_mem_free(el->data);
    el->data = NULL;
    el->data_len = 0;
}

e_ret acp_el_read_breakdown_status(e_buff *buff, acp_el_breakdown_status *el){
    e_ret rc;
    acp_ie ie;
    u8 b;
    u8 i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_breakdown_status enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    i = 0;
    do {
        if(e_buff_read(&ie.data, &b)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
        E_TRACE("...readed source[%d] byte=0x%x", i, b);
        el->source[i] = b & 0x7F;
    } while ((++i < ACP_EL_BREAKDOWN_STATUS_MAX_SOURCE)
            && (b & ACP_MORE_FLG));
    el->source_cnt = MAX(ACP_EL_BREAKDOWN_STATUS_MIN_SOURCE, i);

    if (b & ACP_MORE_FLG) {
        E_DBG("...skipping extra source fields");
        if ((rc = acp_el_skip_while_flag(&ie.data, ACP_MORE_FLG))) {
            goto exit;
        }
    }

    if (e_buff_read(&ie.data, &b)) {
        goto exit;
    }
    E_TRACE("...readed sensor byte=0x%x", b);
    el->sensor = b & 0x7F;

    if (b & ACP_MORE_FLG) {
        if ((rc = acp_el_skip_while_flag(&ie.data, ACP_MORE_FLG))) {
            goto exit;
        }
    }

    if (!IE_REMAIN(ie)
            || (rc = acp_ie_read_bin(&ie.data, &el->data, &el->data_len))) {
        goto exit;
    }
    E_TRACE("...readed binary data, len=%ld", (long) el->data_len);

    rc = OK;

exit:
    E_TRACE("acp_el_read_breakdown_status exit.");
	return rc;
}

#if ACP_EL_BREAKDOWN_STATUS_MIN_SOURCE < 1
#error At least one breakdown status source field must be included.
#endif

e_ret acp_el_write_breakdown_status(e_buff *buff, acp_el_breakdown_status *el) {
    e_ret rc;
    int i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_breakdown_status enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    el->source_cnt = MAX(ACP_EL_BREAKDOWN_STATUS_MIN_SOURCE, el->source_cnt);

    if (el->source[el->source_cnt - 1] & ACP_MORE_FLG) {
        E_DBG("...last source field must not have the more flag set");
        rc = ACP_MSG_ERR_BAD_FORMAT;
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
                    el->source_cnt   /* size of source fields */
                    + 1 /* size of breakdown source */
                    + acp_ie_get_len(el->data_len)))) {
        goto exit;
    }

    for (i = 0; i < el->source_cnt - 1; i++) {
        e_buff_write(buff, ACP_MORE_FLG | el->source[i]);
    }
    e_buff_write(buff, ~ACP_MORE_FLG & el->source[el->source_cnt - 1]);

    e_buff_write(buff, ~ACP_MORE_FLG & el->sensor);

    if((rc = acp_ie_write_bin(buff, el->data, el->data_len))) {
        goto exit;
    }

    rc = OK;
exit:
    E_TRACE("acp_el_write_breakdown_status exit.");
    return rc;
}

void acp_el_free_info_type(acp_el_info_type *el) {
    e_assert( el != NULL );

    e_mem_free(el->data);
    el->data = NULL;
    el->data_len = 0;
}

e_ret acp_el_read_info_type(e_buff *buff, acp_el_info_type *el){
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_info_type enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    e_buff_read(&ie.data, &el->type);
    E_TRACE("...readed info type, byte=0x%x", el->type);

    if (!IE_REMAIN(ie)
            || (rc = acp_ie_read_bin(&ie.data, &el->data, &el->data_len))) {
        goto exit;
    }
    E_TRACE("...readed binary data, len=%ld", (long) el->data_len);

    rc = OK;

exit:
    E_TRACE("acp_el_read_info_type exit.");
	return rc;
}

e_ret acp_el_write_info_type(e_buff *buff, acp_el_info_type *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_info_type enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
                               1 + /* type */
                               acp_ie_get_len(el->data_len)))) {
        goto exit;
    }

    e_buff_write(buff, /* 0x0 = addl flg | */ (el->type & 0x7F));

    if((rc = acp_ie_write_bin(buff, el->data, el->data_len))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_info_type exit.");
    return rc;
}

void acp_el_free_apn_cfg(acp_el_apn_cfg *el) {
    e_mem_free(el->address);
    el->address = NULL;
    e_mem_free(el->login);
    el->login = NULL;
    e_mem_free(el->password);
    el->password = NULL;
}
extern e_ret acp_el_read_apn_cfg(e_buff *buff, acp_el_apn_cfg *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_apn_cfg enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->address))) {
        goto exit;
    }
    if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->login))) {
        goto exit;
    }
    if ((rc = acp_ie_read_iso_8859_1(&ie.data, &el->password))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_apn_cfg exit.");
	return rc;
}
e_ret acp_el_write_apn_cfg(e_buff *buff, acp_el_apn_cfg *el) {
    u32 ie_size;
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_apn_cfg enter, rem=%lu", (unsigned long) e_buff_write_remain(buff));

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    ie_size = 0;
    ie_size += acp_ie_get_iso_8859_1_len(el->address);
    ie_size += acp_ie_get_iso_8859_1_len(el->login);
    ie_size += acp_ie_get_iso_8859_1_len(el->password);

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, ie_size))) {
        goto exit;
    }
    E_TRACE("acp_el_write_apn_cfg ie_size=%lu", (unsigned long) ie_size);

    if ((rc = acp_ie_write_iso_8859_1(buff, el->address))) {
        goto exit;
    }
    E_TRACE("...acp_el_apn_cfg write address");
    if ((rc = acp_ie_write_iso_8859_1(buff, el->login))) {
        goto exit;
    }
    E_TRACE("...acp_el_apn_cfg write address");
    if ((rc = acp_ie_write_iso_8859_1(buff, el->password))) {
        goto exit;
    }
    E_TRACE("...acp_el_apn_cfg write password");

    rc = OK;

exit:
    E_TRACE("acp_el_write_apn_cfg exit, rem=%lu", (unsigned long) e_buff_write_remain(buff));
	return rc;
}

void acp_el_free_server_cfg(acp_el_server_cfg *el) {
	UNUSED(el);
}
e_ret acp_el_read_server_cfg(e_buff *buff, acp_el_server_cfg *el) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_server_cfg enter.");

    rc = _read_ie_hdr_and_set_presence(buff, &ie, &el->present);
    if (rc || ACP_EL_PRESENT != el->present) {
        goto exit;
    }

    if((rc = e_buff_read_u32(&ie.data, &el->server_1))) {
        goto exit;
    }
    if((rc = e_buff_read_u16(&ie.data, &el->port_1))) {
        goto exit;
    }
    if((rc = e_buff_read_u32(&ie.data, &el->server_2))) {
        goto exit;
    }
    if((rc = e_buff_read_u16(&ie.data, &el->port_2))) {
        goto exit;
    }
    if((rc = e_buff_read(&ie.data, &el->proto_id))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_server_cfg exit.");
	return rc;
}
e_ret acp_el_write_server_cfg(e_buff *buff, acp_el_server_cfg *el) {
    e_ret rc;

    E_TRACE("acp_el_write_server_cfg enter.");

    if (ACP_EL_NOT_PRESENT == el->present) {
        E_TRACE("...not present");
        rc = OK;
        goto exit;
    } else if (ACP_EL_EMPTY == el->present) {
        rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 0);
        goto exit;
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY,
                    4 + /* server_1 */
                    2 + /* port_1 */
                    4 + /* server_2 */
                    2 + /* port_2 */
                    1 /* proto_id */
                    ))) {
        goto exit;
    }

    if((rc = e_buff_write_u32(buff, el->server_1))) {
        goto exit;
    }
    if((rc = e_buff_write_u16(buff, el->port_1))) {
        goto exit;
    }
    if((rc = e_buff_write_u32(buff, el->server_2))) {
        goto exit;
    }
    if((rc = e_buff_write_u16(buff, el->port_2))) {
        goto exit;
    }
    if((rc = e_buff_write(buff, el->proto_id))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_server_cfg exit.");
	return rc;
}
