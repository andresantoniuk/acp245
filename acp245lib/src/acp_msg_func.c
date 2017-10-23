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

E_INLINE static void _free_msg_func_cmd(acp_msg_func_cmd *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_ctrl_func(&msg->ctrl_func);
    acp_el_free_func_cmd(&msg->func_cmd);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_func_cmd(e_buff *buff, acp_msg_func_cmd* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_func_cmd enter.");

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
    E_TRACE("_read_msg_func_cmd exit.");
	return rc;
}

static e_ret _write_msg_func_cmd(e_buff *buff, acp_msg_func_cmd* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_func_cmd enter.");

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
    E_TRACE("_write_msg_func_cmd exit.");
	return rc;
}

E_INLINE static void _free_msg_func_status(acp_msg_func_status *msg) {
    e_assert( msg != NULL );

    acp_el_free_version(&msg->version);
    acp_el_free_ctrl_func(&msg->ctrl_func);
    acp_el_free_func_cmd(&msg->func_status);
    acp_el_free_error(&msg->error);
    acp_el_free_vehicle_desc(&msg->vehicle_desc);
}

static e_ret _read_msg_func_status(e_buff *buff, acp_msg_func_status* msg){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_read_msg_func_status enter.");

    if ((rc = acp_el_read_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_read_ctrl_func(buff, &msg->ctrl_func))) {
        goto exit;
    }

    if ((rc = acp_el_read_func_cmd(buff, &msg->func_status))) {
        goto exit;
    }

    if ((rc = acp_el_read_error(buff, &msg->error))) {
        goto exit;
    }

    if ((rc = acp_el_read_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

exit:
    E_TRACE("_read_msg_func_status exit.");
	return rc;
}

static e_ret _write_msg_func_status(e_buff *buff, acp_msg_func_status* msg) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    E_TRACE("_write_msg_func_status enter.");

    if ((rc = acp_el_write_version(buff, &msg->version))) {
        goto exit;
    }

    if ((rc = acp_el_write_ctrl_func(buff, &msg->ctrl_func))) {
        goto exit;
    }

    if ((rc = acp_el_write_func_cmd(buff, &msg->func_status))) {
        goto exit;
    }

    if ((rc = acp_el_write_error(buff, &msg->error))) {
        goto exit;
    }

    if (ACP_EL_NOT_PRESENT == msg->vehicle_desc.present
            || (rc = acp_el_write_vehicle_desc(buff, &msg->vehicle_desc))) {
        goto exit;
    }

exit:
    E_TRACE("_write_msg_func_status exit.");
	return rc;
}
