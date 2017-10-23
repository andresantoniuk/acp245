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

E_INLINE static void _free_msg_prov_upd(acp_msg_prov_upd *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_timestamp(&msg->start_time);
    acp_el_free_timestamp(&msg->end_time);
    acp_el_free_tcu_desc(&msg->tcu_desc);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_prov_upd(e_buff *buff, acp_msg_prov_upd* msg){
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_prov_upd enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    msg->target_app_id = b & 0x7F;
    E_TRACE("...msg_prov_upd target_app_id=%lu.", (unsigned long) msg->target_app_id);
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
    E_TRACE("...msg_prov_upd readed appl_flg=0x%x.", msg->appl_flg);

    msg->ctrl_flg1 = 0x3F & b;
    E_TRACE("...msg_prov_upd readed ctrl_flg1=0x%x.", msg->ctrl_flg1);

    if (msg->ctrl_flg1 & ACP_MSG_PROV_ADDL_FLG_MASK) {
        if (e_buff_read(buff, &b)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
        msg->ctrl_flg2 = b;
        E_TRACE("...msg_prov_upd readed ctrl_flg2=0x%x.", msg->ctrl_flg2);

        if (msg->ctrl_flg2 & ACP_MORE_FLG) {
            E_DBG("...msg_prov_upd skipping additional ctrl_flg2 flags.");
            if ((rc = acp_el_skip_while_flag(buff, ACP_MORE_FLG))) {
                goto exit;
            }
        }
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

    if ((rc = acp_el_read_tcu_desc(buff, &msg->tcu_desc))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

exit:
    E_TRACE("_read_msg_prov_upd exit.");
	return rc;
}

static e_ret _write_msg_prov_upd(e_buff *buff, acp_msg_prov_upd* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_prov_upd enter.");

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

    if (ACP_EL_NOT_PRESENT == msg->tcu_desc.present
            || (rc = acp_el_write_tcu_desc(buff, &msg->tcu_desc))) {
        goto exit;
    }

    if (msg->ctrl_flg1 & ACP_MSG_PROV_VEHICLE_DESC_MASK) {
        if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
                || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
            goto exit;
        }
    }

exit:
    E_TRACE("_write_msg_prov_upd exit.");
	return rc;
}

E_INLINE static void _free_msg_prov_reply(acp_msg_prov_reply *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_error(&msg->error);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_prov_reply(e_buff *buff, acp_msg_prov_reply* msg){
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_prov_reply enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    msg->target_app_id = b & 0x7F;
    E_TRACE("...msg_prov_reply target_app_id=%lu.", (unsigned long) msg->target_app_id);
    if (b & ACP_MORE_FLG) {
        /* TODO: support more than one target_app_id on provision msg */
        E_DBG("more than one provisioned target_app_id, ignoring");
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

exit:
    E_TRACE("_read_msg_prov_reply exit.");
	return rc;
}

static e_ret _write_msg_prov_reply(e_buff *buff, acp_msg_prov_reply* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_prov_reply enter.");

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

    if (e_buff_write(buff, (
                      ((msg->status << 6) & 0xC0)
                    | ((msg->tcu_resp << 4) & 0x30)
                    /* | bits 4..7 reserved = 0 */
                    ))) {
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

exit:
    E_TRACE("_write_msg_prov_reply exit.");
	return rc;
}
