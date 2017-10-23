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

E_INLINE static void _free_msg_track_cmd(acp_msg_track_cmd *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_ctrl_func(&msg->ctrl_func);
    acp_el_free_func_cmd(&msg->func_cmd);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_track_cmd(e_buff *buff, acp_msg_track_cmd* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_track_cmd enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_read_ctrl_func(buff, &msg->ctrl_func))) {
        goto exit;
    }

    if ((rc = acp_el_read_func_cmd(buff, &msg->func_cmd))) {
        goto exit;
    }

    if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

exit:
    E_TRACE("_read_msg_track_cmd exit.");
	return rc;
}
static e_ret _write_msg_track_cmd(e_buff *buff, acp_msg_track_cmd* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_track_cmd enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_write_ctrl_func(buff, &msg->ctrl_func))) {
        goto exit;
    }

    if ((rc = acp_el_write_func_cmd(buff, &msg->func_cmd))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
            || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

exit:
    E_TRACE("_write_msg_track_cmd exit.");
	return rc;
}

E_INLINE static void _free_msg_track_pos(acp_msg_track_pos *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_timestamp(&msg->timestamp);
    acp_el_free_location(&msg->location);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
    acp_el_free_breakdown_status(&msg->breakdown_status);
    acp_el_free_info_type(&msg->info_type);
}

static e_ret _read_msg_track_pos(e_buff *buff, acp_msg_track_pos* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_track_pos enter.");


    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_read_timestamp(buff, &msg->timestamp))) {
        goto exit;
    }

    if ((rc = acp_el_read_location(buff, &msg->location))) {
        goto exit;
    }

    if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

    if ((rc = acp_el_read_breakdown_status(buff, &msg->breakdown_status))) {
        goto exit;
    }

    if ((rc = acp_el_read_info_type(buff, &msg->info_type))) {
        goto exit;
    }

exit:
    E_TRACE("_read_msg_track_pos exit.");
	return rc;
}

static e_ret _write_msg_track_pos(e_buff *buff, acp_msg_track_pos* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_track_pos enter.");


    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_write_timestamp(buff, &msg->timestamp))) {
        goto exit;
    }

    if ((rc = acp_el_write_location(buff, &msg->location))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
            || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

    if ((rc = acp_el_write_breakdown_status(buff, &msg->breakdown_status))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == msg->info_type.present
            || (rc = acp_el_write_info_type(buff, &msg->info_type))) {
        goto exit;
    }

exit:
    E_TRACE("_wrote_msg_track_pos exit.");
	return rc;
}

static E_INLINE void _free_msg_track_reply(acp_msg_track_reply *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_error(&msg->error);
}

static e_ret _read_msg_track_reply(e_buff *buff, acp_msg_track_reply *msg) {
    e_ret rc;
    u8 b;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_track_reply enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->confirmation = (b & 0xF0) >> 4;
    msg->transmit_unit = (b & 0x0F);

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    msg->ctrl_flg = b;

    if ((rc = acp_el_read_error(buff, &msg->error))) {
        goto exit;
    }
    /* skip the rest of the reply message */

exit:
    E_TRACE("_read_msg_track_reply exit.");
	return rc;
}

static e_ret _write_msg_track_reply(e_buff *buff, acp_msg_track_reply* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_track_reply enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    if (e_buff_write(buff, ((msg->confirmation << 4) & 0xF0)
                | ((msg->transmit_unit & 0x0F)))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff, /* 0 = addl flg | */ (msg->ctrl_flg & 0x7F))) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if ((rc = acp_el_write_error(buff, &msg->error))) {
        goto exit;
    }

exit:
    E_TRACE("_write_msg_track_reply exit.");
	return rc;
}
