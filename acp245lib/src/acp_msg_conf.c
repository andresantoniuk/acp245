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

#include "e_port.h"
#include "e_log.h"
#include "e_mem.h"
#include "e_buff.h"

#include "acp_err.h"
#include "acp_el.h"

E_INLINE static void _free_msg_cfg_upd_245(acp_msg_cfg_upd_245 *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_timestamp(&msg->start_time);
    acp_el_free_timestamp(&msg->end_time);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
    acp_el_free_tcu_desc(&msg->tcu_desc);
    acp_el_free_tcu_data(&msg->tcu_data);
}

static e_ret _read_msg_cfg_upd_245(e_buff *buff, acp_msg_cfg_upd_245 *msg){
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_cfg_upd_245 enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->target_app_id = b & 0x7F;
    if (b & ACP_MORE_FLG) {
       if ((rc = acp_el_skip_while_flag(buff, ACP_MORE_FLG))) {
           goto exit;
       }
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->appl_flg = (0xC0 & b) >> 6;
    msg->ctrl_flg1 = 0x3F & b;

    if (msg->ctrl_flg1 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if (e_buff_read(buff, &b)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
        msg->ctrl_flg2 = b;
    }

    if (msg->ctrl_flg2 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if ((rc = acp_el_skip_while_flag(buff, ACP_MSG_PROV_ADDL_FLG_MASK))) {
            goto exit;
        }
    }

    /* reserved */
    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_START_TIME_MASK) {
        if ((rc = acp_el_read_timestamp(buff, &msg->start_time))) {
            goto exit;
        }
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_END_TIME_MASK) {
        if ((rc = acp_el_read_timestamp(buff, &msg->end_time))) {
            goto exit;
        }
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_GRACE_TIME_MASK) {
        if ((rc = acp_el_read_timestamp(buff, &msg->grace_time))) {
            goto exit;
        }
    }

    if (msg->ctrl_flg2 & ACP_MSG_PROV_NUM_SAMPLES_MASK) {
        /* ignore number of samples */
        rc = acp_el_skip_while_flag(buff, ACP_MORE_FLG);
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    if ((rc = acp_el_read_tcu_desc(buff, &msg->tcu_desc))) {
        goto exit;
    }

    if ((rc = acp_el_read_tcu_data(buff, &msg->tcu_data))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("_read_msg_cfg_upd_245 exit.");
	return rc;
}

static e_ret _write_msg_cfg_upd_245(e_buff *buff, acp_msg_cfg_upd_245* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_cfg_upd_245 enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_write(buff, /* more flag = 0 | */ (msg->target_app_id & 0x7F))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff, (
                      (msg->appl_flg << 6) & 0xC0)
                    | (msg->ctrl_flg1 & 0x3F))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if (e_buff_write(buff, /* addl_flg = 0 | */ (msg->ctrl_flg2 & 0x7F))) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
    }

	/* reserved octet */
    if (e_buff_write(buff, 0x00)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

	if (msg->ctrl_flg1 & ACP_MSG_PROV_START_TIME_MASK) {
        if ((rc = acp_el_write_timestamp(buff, &msg->start_time))) {
            goto exit;
        }
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_END_TIME_MASK) {
        if ((rc = acp_el_write_timestamp(buff, &msg->end_time))) {
            goto exit;
        }
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_GRACE_TIME_MASK) {
        if ((rc = acp_el_write_timestamp(buff, &msg->grace_time))) {
            goto exit;
        }
    }

    /* number of samples are ignored (not present in 245) */

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
                || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    if (ACP_EL_NOT_PRESENT == msg->tcu_desc.present
            || (rc = acp_el_write_tcu_desc(buff, &msg->tcu_desc))) {
        goto exit;
    }

    if ((rc = acp_el_write_tcu_data(buff, &msg->tcu_data))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("_write_msg_cfg_upd_245 exit.");
	return rc;

}

E_INLINE static void _free_msg_cfg_reply(acp_msg_cfg_reply *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_error(&msg->error);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_cfg_reply(e_buff *buff, acp_msg_cfg_reply *msg){
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_cfg_reply enter.");

    E_TRACE("remain, %d", e_buff_read_remain(buff));
    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }
    E_TRACE("remain after version, %d", e_buff_read_remain(buff));

    /* reserved octet */
    if (e_buff_skip(buff, 1)) {
        E_TRACE("reserved not available, %d", e_buff_read_remain(buff));
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        E_TRACE("target_app_id not available");
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->target_app_id = b & 0x7F;
    if (b & ACP_MORE_FLG) {
       if ((rc = acp_el_skip_while_flag(buff, ACP_MORE_FLG))) {
           goto exit;
       }
    }

    if (e_buff_read(buff, &b)) {
        E_TRACE("appl_flg not available");
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->appl_flg = (0xC0 & b) >> 6;
    msg->ctrl_flg1 = 0x3F & b;

    if (msg->ctrl_flg1 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if ((rc = acp_el_skip_while_flag(buff, ACP_MSG_PROV_ADDL_FLG_MASK))) {
            goto exit;
        }
    }

    if (e_buff_read(buff, &b)) {
        E_TRACE("status not available");
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->status = (0xC0 & b) >> 6;
    msg->tcu_resp = (0x30 & b) >> 4;
    /* bits 4..7 reserved */

    if ((rc = acp_el_read_error(buff, &msg->error))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    rc = OK;

exit:
    E_TRACE("_read_msg_cfg_reply exit.");
	return rc;
}

static e_ret _write_msg_cfg_reply(e_buff *buff, acp_msg_cfg_reply* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_cfg_reply enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    /* reserved octet */
    if (e_buff_write(buff, 0x0)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff, /* 0 = more flag | */ msg->target_app_id & 0x7F)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                ((msg->appl_flg << 6) & 0xC0)
                | (/* 0 = addl_flag | */msg->ctrl_flg1 & 0x1F))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                ((msg->status << 6) & 0xC0)
                | ((msg->tcu_resp << 4) & 0x30)
                /* | 0 = reserved */)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if ((rc = acp_el_write_error(buff, &msg->error))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
                || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    rc = OK;

exit:
    E_TRACE("_write_msg_cfg_reply exit.");
	return rc;

}

E_INLINE static void _free_msg_cfg_reply_245(acp_msg_cfg_reply_245 *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_tcu_data_error(&msg->error);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_cfg_reply_245(e_buff *buff, acp_msg_cfg_reply_245 *msg){
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_cfg_reply_245 enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->target_app_id = b & 0x7F;
    if (b & ACP_MORE_FLG) {
       if ((rc = acp_el_skip_while_flag(buff, ACP_MORE_FLG))) {
           goto exit;
       }
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->appl_flg = (0xC0 & b) >> 6;
    msg->ctrl_flg1 = 0x3F & b;

    if (msg->ctrl_flg1 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if ((rc = acp_el_skip_while_flag(buff, ACP_MSG_PROV_ADDL_FLG_MASK))) {
            goto exit;
        }
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->status = (0xC0 & b) >> 6;
    E_TRACE("...status_flag1: %u", msg->status);
    msg->tcu_resp = (0x30 & b) >> 4;
    E_TRACE("...tcu_resp: %u", msg->tcu_resp);
    /* bits 4..7 reserved */

    if ((rc = acp_el_read_tcu_data_error(buff, &msg->error))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    rc = OK;

exit:
    E_TRACE("_read_msg_cfg_reply_245 exit.");
	return rc;
}

static e_ret _write_msg_cfg_reply_245(e_buff *buff, acp_msg_cfg_reply_245* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_cfg_reply_245 enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }


    if (e_buff_write(buff, /* 0 = more flag | */ msg->target_app_id & 0x7F)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                ((msg->appl_flg << 6) & 0xC0)
                | (/* 0 = addl_flag | */msg->ctrl_flg1 & 0x1F))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                ((msg->status << 6) & 0xC0)
                | ((msg->tcu_resp << 4) & 0x30)
                /* | 0 = reserved */)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if ((rc = acp_el_write_tcu_data_error(buff, &msg->error))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
                || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

    rc = OK;

exit:
    E_TRACE("_write_msg_cfg_reply_245 exit.");
	return rc;
}

E_INLINE static void _free_msg_cfg_activation(acp_msg_cfg_activation *msg) {
    e_assert( msg != NULL );

    acp_el_free_apn_cfg(&msg->apn_cfg);
    acp_el_free_server_cfg(&msg->server_cfg);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_cfg_activation(e_buff *buff, acp_msg_cfg_activation *msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_cfg_activation enter.");

    if ((rc = acp_el_read_apn_cfg(buff, &msg->apn_cfg))) {
        goto exit;
    }
    if ((rc = acp_el_read_server_cfg(buff, &msg->server_cfg))) {
        goto exit;
    }
    if ((rc = e_buff_read(buff, &msg->ctrl_byte))) {
        goto exit;
    }
    if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }
    rc = OK;
exit:
    E_TRACE("_read_msg_cfg_activation exit.");
    return rc;
}
static e_ret _write_msg_cfg_activation(e_buff *buff, acp_msg_cfg_activation* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_cfg_activation enter.");

    if ((rc = acp_el_write_apn_cfg(buff, &msg->apn_cfg))) {
        goto exit;
    }
    if ((rc = acp_el_write_server_cfg(buff, &msg->server_cfg))) {
        goto exit;
    }
    if ((rc = e_buff_write(buff, msg->ctrl_byte))) {
        goto exit;
    }
    if ((rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }
    rc = OK;
exit:
    E_TRACE("_write_msg_cfg_activation exit.");
    return rc;
}
